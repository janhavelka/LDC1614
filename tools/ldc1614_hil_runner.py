#!/usr/bin/env python3
"""Conservative serial HIL runner for LDC1614 diagnostic firmware.

The runner never reports PASS when no serial hardware is supplied. Without a
port it emits a NOT_RUN artifact. Optional fault/address checks remain gated
for board/operator control; the Arduino no-sensor profile also provides an
explicit-duration bounded soak.
"""

from __future__ import annotations

import argparse
import json
import math
import os
import re
import subprocess
import sys
import time
from contextlib import ExitStack
from datetime import datetime, timezone
from pathlib import Path
from typing import Dict, Iterable, List, Optional, TextIO, Tuple

from ldc1614_cli_contract import (
    COMMAND_BY_NAME,
    COMMAND_SPECS,
    NO_SENSOR_COMMANDS as CONTRACT_NO_SENSOR_COMMANDS,
    OPTIONAL_COMMAND_GROUPS,
    SECTION_ORDER,
    SENSOR_COMMANDS as CONTRACT_SENSOR_COMMANDS,
    validate_contract,
)

ROOT = Path(__file__).resolve().parent.parent
MAX_SAMPLE_RATE_COUNT = 5000
MAX_STRESS_COUNT = 100000
MAX_STARTUP_DELAY_S = 30.0
MAX_COMMAND_TIMEOUT_S = 60.0
MAX_WRITE_TIMEOUT_S = 10.0
MAX_IDLE_GAP_S = 10.0
MAX_COMMAND_SET_REPETITIONS = 100
MAX_SOAK_DURATION_S = 24 * 60 * 60
MAX_SOAK_CYCLE_DELAY_S = 60.0
SERIAL_LINE_STATES = ("on", "off", "unchanged")
HELP_SECTION_LABELS = {
    "Common": "Common",
    "Lifecycle": "Lifecycle and owner jobs",
    "Measurements": "Measurements",
    "Configuration": "Staged configuration",
    "Registers and helpers": "Registers and helpers",
    "Diagnostics": "Driver diagnostics",
}
LDC1614_CONFIG_REGISTERS = frozenset((*range(0x08, 0x18), 0x19, 0x1A, 0x1B,
                                      *range(0x1E, 0x22)))
LDC1614_ALL_REGISTERS = frozenset((*range(0x00, 0x18), 0x18, 0x19, 0x1A, 0x1B,
                                   0x1C, *range(0x1E, 0x22), 0x7E, 0x7F))
LDC1612_UNAVAILABLE_REGISTERS = frozenset((
    0x04, 0x05, 0x06, 0x07, 0x0A, 0x0B, 0x0E, 0x0F, 0x12, 0x13,
    0x16, 0x17, 0x20, 0x21,
))
LDC1612_CONFIG_REGISTERS = (
    LDC1614_CONFIG_REGISTERS - LDC1612_UNAVAILABLE_REGISTERS
)
LDC1612_ALL_REGISTERS = (
    LDC1614_ALL_REGISTERS - LDC1612_UNAVAILABLE_REGISTERS
)


class SerialRunFailure(RuntimeError):
    """Serial failure with all evidence captured before the exception."""

    def __init__(
        self,
        message: str,
        results: List[Dict[str, object]],
        transcript: str,
        startup_elapsed_s: float,
        soak: Optional[Dict[str, object]],
        exception_type: str = "RuntimeError",
        serial_opened: bool = True,
    ) -> None:
        super().__init__(message)
        self.results = results
        self.transcript = transcript
        self.startup_elapsed_s = startup_elapsed_s
        self.soak = soak
        self.exception_type = exception_type
        self.serial_opened = serial_opened


class TranscriptJournal:
    """Incremental raw transcript with an in-memory mirror for JSON evidence."""

    def __init__(self, path: str) -> None:
        self.path = Path(path) if path else None
        self.parts: List[str] = []
        self._stream: Optional[TextIO] = None

    def open(self) -> None:
        if self.path is not None:
            self._stream = self.path.open("w", encoding="utf-8", newline="\n")

    def append(self, section: str) -> None:
        self.parts.append(section)
        if self._stream is None:
            return
        if self._stream.tell() != 0:
            self._stream.write("\n")
        self._stream.write(section)
        self._stream.flush()
        os.fsync(self._stream.fileno())

    def text(self) -> str:
        return "\n".join(self.parts)

    def close(self) -> None:
        if self._stream is not None:
            self._stream.close()
            self._stream = None

ARDUINO_DEFAULT_COMMANDS = list(CONTRACT_SENSOR_COMMANDS)
ARDUINO_NO_SENSOR_COMMANDS = list(CONTRACT_NO_SENSOR_COMMANDS)
IDF_DEFAULT_COMMANDS = list(CONTRACT_SENSOR_COMMANDS)

# A bounded no-sensor soak exercises target identity, chip identity, a
# destructive STATUS observation, two bounded CONFIG writes, and cached
# driver state. Each cycle ends awake even when the requested duration expires
# while the cycle is running.
NO_SENSOR_SOAK_COMMANDS = [
    "version",
    "probe",
    "status",
    "sleep",
    "wake",
    "busrecover confirm",
    "state",
    "init",
    "wake",
    "probe",
    "drv",
]

# Safe, cache-only configuration coverage modeled after the sibling I2C HIL
# runners.  Values are staged but never committed; the final discard and
# driver snapshot prove that the live hardware profile was not changed.
CONFIG_MATRIX_COMMON_COMMANDS = (
    "profile reset",
    "mode single 0", "mode seq 2",
    "refclk internal 40000000 100", "refclk external 40000000 100",
    "deglitch 1", "deglitch 3", "deglitch 10", "deglitch 33",
    "activation full", "activation low",
    "timeout 1", "timeout 4294967295",
    "rp 0", "rp 1", "autoamp 0", "autoamp 1",
    "highcurrent 0", "highcurrent 1", "intbconfig 0", "intbconfig 1",
    "errors show", "errors none", "errors all",
)
ERROR_ROUTE_NAMES = (
    "data-under", "data-over", "data-watchdog", "data-amplitude-high",
    "data-amplitude-low", "status-under", "status-over", "status-watchdog",
    "status-amplitude-high", "status-amplitude-low", "status-zero-count",
    "data-ready",
)
INVALID_INPUT_COMMANDS = (
    "help help extra", "version extra", "init 0", "resetreapply",
    "read 0", "mode single 4", "mode seq 1",
    "refclk external 0 100", "refclk external 40000000 1000000",
    "deglitch 2", "activation invalid", "timeout 0",
    "rp 2", "autoamp 2", "highcurrent 2", "intbconfig 2",
    "errors invalid", "error invalid 1", "error data-under 2",
    "rcount 0 4", "rcount 0 65536", "settle 0 65536", "findiv 0 0", "findiv 0 16",
    "frefdiv 0 0", "frefdiv 0 1024", "offset 0 65536",
    "reg 0x80", "wreg 0x1A 0x3481", "decode status 0x10000",
    "decode data 0x10000 0", "freq 4 0", "freq 0 0x10000000",
    "timing 0", "driveua 32",
)

ASYNC_EXECUTION_CLASSES = frozenset(("CORE_JOB", "CLI_JOB"))
ASYNC_CANONICAL_COMMANDS = frozenset(
    spec.canonical
    for spec in COMMAND_SPECS
    if spec.execution in ASYNC_EXECUTION_CLASSES
)
IMMEDIATE_RESULT_COMMANDS = frozenset(
    ("bind", "end", "sleep", "wake", "cancel", "invalidate", "busrecover")
)
SCHEDULED_OPERATION_PATTERN = re.compile(
    r"\bCLI scheduled:\s*command=([a-z_]+)\s+session=(\d+)\b",
    re.IGNORECASE,
)
TERMINAL_RESULT_PATTERN = re.compile(
    r"\bCLI result:\s*command=([a-z_]+)\s+session=(\d+)\b[^\r\n]*"
    r"\boutcome=([A-Z_]+)\b[^\r\n]*\bcode=(\d+)\b",
    re.IGNORECASE,
)
FIRMWARE_VERSION_PATTERN = re.compile(
    r"\b(?:version|library version)\s*[=:]\s*(\d+\.\d+\.\d+)\b",
    re.IGNORECASE,
)

COMMAND_EVIDENCE_PATTERNS = {
    "help": (
        re.compile(r"(?:===\s+LDC1614 CLI\s+===|\bhelp command=)", re.IGNORECASE),
        re.compile(rf"\bcommand_count={len(COMMAND_SPECS)}\b", re.IGNORECASE),
    ),
    "version": (
        FIRMWARE_VERSION_PATTERN,
        re.compile(r"\bfirmware_git=[0-9a-f]{7,40}\b", re.IGNORECASE),
        re.compile(r"\bfirmware_status=clean\b", re.IGNORECASE),
        re.compile(r"\bbuild_timestamp=\S+", re.IGNORECASE),
        re.compile(r"\bplatform=\S+\s+framework=\S+\s+framework_version=\S+\s+"
                   r"idf_version=\S+\s+target=\S+\s+i2c_backend=\S+\s+"
                   r"frequency_hz=\d+\b", re.IGNORECASE),
    ),
    "color": (re.compile(r"\bcolor enabled=[01]\b", re.IGNORECASE),),
    "verbose": (re.compile(r"\bverbose enabled=[01]\b", re.IGNORECASE),),
    "bus": (
        re.compile(
            r"\bbus open=[01]\s+backend=\S+\s+port=-?\d+\s+sda=-?\d+\s+"
            r"scl=-?\d+\s+sda_level=-?\d+\s+scl_level=-?\d+\s+"
            r"address=0x[0-9a-f]{2}\s+frequency_hz=\d+\s+timeout_ms=\d+\b",
            re.IGNORECASE,
        ),
    ),
    "busfreq": (
        re.compile(
            r"\bbusfreq previous_hz=\d+\s+requested_hz=\d+\s+active_hz=\d+\s+"
            r"reinitialized=[01]\s+outcome=SUCCESS\s+code=0\b",
            re.IGNORECASE,
        ),
    ),
    "discover": (
        re.compile(
            r"\bdiscover tested=2\s+responding=\d+\s+matched=\d+\s+"
            r"mismatched=\d+\s+failed=\d+\s+variant=UNKNOWN\s+code=0\b",
            re.IGNORECASE,
        ),
    ),
    "probe": (
        re.compile(r"\bmanufacturer_id=0x5449\b", re.IGNORECASE),
        re.compile(r"\bdevice_id=0x3055\b", re.IGNORECASE),
        re.compile(r"\bmatch=YES\b", re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
    ),
    "id": (
        re.compile(r"\bMANUFACTURER_ID=0x5449\b", re.IGNORECASE),
        re.compile(r"\bDEVICE_ID=0x3055\b", re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
    ),
    "selftest": (
        re.compile(r"\bSelftest result:\s*pass=\d+\s+fail=0\s+skip=\d+\b", re.IGNORECASE),
        re.compile(r"\bCLI result:\s*command=selftest\s+session=\d+\b[^\r\n]*\boutcome=SUCCESS\b[^\r\n]*\bcode=0\b", re.IGNORECASE),
    ),
    "stress": (
        re.compile(
            r"\bStress result:\s*requested=\d+\s+ok=\d+\s+fail=0\s+"
            r"elapsed_ms=\d+\s+hz=[0-9.]+\b",
            re.IGNORECASE,
        ),
    ),
    "stress_mix": (
        re.compile(
            r"\bStressMix results:\s*requested=\d+\s+ok=\d+\s+fail=0\s+elapsed_ms=\d+\b",
            re.IGNORECASE,
        ),
    ),
    "stress_id": (
        re.compile(
            r"\bIdentityStress result:\s*requested=\d+\s+completed=\d+\s+"
            r"ok=\d+\s+fail=0\s+first_failure_iteration=-1\s+"
            r"elapsed_ms=\d+\s+hz=[0-9.]+\b",
            re.IGNORECASE,
        ),
    ),
    "stress_reset": (
        re.compile(
            r"\bResetStress result:\s*requested=\d+\s+completed=\d+\s+"
            r"ok=\d+\s+fail=0\s+first_failure_iteration=-1\s+elapsed_ms=\d+\b",
            re.IGNORECASE,
        ),
    ),
    "stress_busfreq": (
        re.compile(
            r"\bBusFrequencyStress result:\s*requested=\d+\s+completed=\d+\s+"
            r"ok=\d+\s+fail=0\s+initial_hz=\d+\s+active_hz=\d+\s+"
            r"restored_hz=\d+\s+restore_code=0\s+first_failure_iteration=-1\s+"
            r"elapsed_ms=\d+\b",
            re.IGNORECASE,
        ),
    ),
    "soak": (
        re.compile(
            r"\bSoak results:\s*seconds=\d+\s+cycles=\d+\s+ok=\d+\s+fail=0\s+elapsed_ms=\d+\b",
            re.IGNORECASE,
        ),
    ),
    "watch": (
        re.compile(
            r"\bWatch results:\s*requested=\d+\s+completed=\d+\s+failed=0\s+elapsed_ms=\d+\b",
            re.IGNORECASE,
        ),
    ),
    "samplerate": (
        re.compile(
            r"\bSampleRate result:\s*requested=\d+\s+ok=\d+\s+fail=0\s+"
            r"elapsed_ms=\d+\s+hz=[0-9.]+\s+ready_checks=\d+\s+"
            r"ready_status_raw=0x[0-9a-f]{4}\b",
            re.IGNORECASE,
        ),
    ),
    "verify": (
        re.compile(
            r"\bverify complete\s+checked=\d+\s+matched=\d+\s+"
            r"mismatched=0\s+read_failures=0\b",
            re.IGNORECASE,
        ),
    ),
    "drv": (
        re.compile(
            r"\bdrv bound=[01]\s+applied=\S+\s+revision=\d+\s+active=[01]\s+"
            r"result_available=[01]\b",
            re.IGNORECASE,
        ),
        re.compile(r"\btransport attempts=\d+\s+success=\d+\s+failures=\d+\b", re.IGNORECASE),
    ),
    "state": (
        re.compile(
            r"\bstate bound=[01]\s+applied=\S+\s+profile_dirty=[01]\s+"
            r"session_kind=\S+\s+active=[01]\s+pending_result=[01]\b",
            re.IGNORECASE,
        ),
    ),
    "cfg": (
        re.compile(r"\bcfg label=(?:desired|staged)\b", re.IGNORECASE),
        re.compile(r"\baddress=0x[0-9a-f]{2}\b", re.IGNORECASE),
        re.compile(r"\bvariant_channels=(?:2|4)\b", re.IGNORECASE),
        re.compile(r"\bselected=0x[0-9a-f]{2}\b", re.IGNORECASE),
        re.compile(r"\bmode=", re.IGNORECASE),
        re.compile(r"\bref_source=", re.IGNORECASE),
        re.compile(r"\bref_hz=\d+\b", re.IGNORECASE),
        re.compile(r"\btolerance_ppm=\d+\b", re.IGNORECASE),
        re.compile(r"\bdeglitch=", re.IGNORECASE),
        re.compile(r"\bactivation=", re.IGNORECASE),
        re.compile(r"\btimeout_ms=\d+\b", re.IGNORECASE),
        re.compile(r"\brp_override=[01]\b", re.IGNORECASE),
        re.compile(r"\bauto_amplitude=[01]\b", re.IGNORECASE),
        re.compile(r"\bhigh_current=[01]\b", re.IGNORECASE),
        re.compile(r"\bintb_config=[01]\b", re.IGNORECASE),
        re.compile(r"\berror_reporting=", re.IGNORECASE),
        re.compile(r"\brevision=\d+\b", re.IGNORECASE),
        re.compile(r"\bapplied=", re.IGNORECASE),
    ),
    "job": (
        re.compile(r"\bjob active=[01]\b.*\boperation=\d+\b.*\btransfers=\d+\b.*\bmaximum=\d+\b",
                   re.IGNORECASE),
        re.compile(r"\bsession active=[01]\b.*\bid=\d+\b.*\bcompleted=\d+/\d+\b", re.IGNORECASE),
    ),
    "result": (
        re.compile(
            r"\bOperation result:\s*operation=\d+\s+kind=\S+\s+outcome=\S+\s+"
            r"effects=0x[0-9a-f]{2}\b.*\btransfers=\d+\s+maximum=\d+\s+code=0\b",
            re.IGNORECASE,
        ),
    ),
    "status": (
        re.compile(r"\bSTATUS=0x[0-9a-f]{4}\s+observed=[01]\s+raw=0x[0-9a-f]{4}\b",
                   re.IGNORECASE),
        re.compile(r"\bStatus:\s*OK\b[^\r\n]*\bcode=0\b", re.IGNORECASE),
    ),
    "status_raw": (
        re.compile(r"\bstatus_raw=0x[0-9a-f]{4}\s+code=0\b", re.IGNORECASE),
    ),
    "ready": (
        re.compile(r"\bready=[01]\s+code=0\b", re.IGNORECASE),
        re.compile(r"\bready_status=0x[0-9a-f]{4}\s+observed=[01]\b", re.IGNORECASE),
    ),
    "timing": (
        re.compile(r"\bprofile=(?:desired|staged)\b", re.IGNORECASE),
        re.compile(r"\bwake_settle_us=\d+\b.*\bconversion_us=\d+\b", re.IGNORECASE),
    ),
    "freq": (
        re.compile(r"\bprofile=(?:desired|staged)\b", re.IGNORECASE),
        re.compile(r"\bfrequency_hz=[0-9.]+\b", re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
    ),
    "read": (
        re.compile(
            r"\bbatch type=SEQUENTIAL_READOUT\s+selected=0x[0-9a-f]{2}\s+"
            r"valid=0x[0-9a-f]{2}\s+fresh=0x[0-9a-f]{2}\s+"
            r"error=0x[0-9a-f]{2}\s+overrun=0x[0-9a-f]{2}\b.*\bsimultaneous=0\b",
            re.IGNORECASE,
        ),
        re.compile(r"\bstatus_before=0x[0-9a-f]{4}\s+observed=[01]\b", re.IGNORECASE),
        re.compile(r"\bstatus_after=0x[0-9a-f]{4}\s+observed=[01]\b", re.IGNORECASE),
    ),
    "last": (
        re.compile(
            r"\bbatch type=SEQUENTIAL_READOUT\s+selected=0x[0-9a-f]{2}\s+"
            r"valid=0x[0-9a-f]{2}\s+fresh=0x[0-9a-f]{2}\s+"
            r"error=0x[0-9a-f]{2}\s+overrun=0x[0-9a-f]{2}\b.*\bsimultaneous=0\b",
            re.IGNORECASE,
        ),
        re.compile(r"\bsample channel=\d+\s+msb=0x[0-9a-f]{4}\s+lsb=0x[0-9a-f]{4}\b", re.IGNORECASE),
    ),
    "intb": (re.compile(r"\bintb asserted=[01]\s+code=0\b", re.IGNORECASE),),
    "initdrive": (
        re.compile(r"\bchannel=\d+\s+init_drive_code=\d+\b", re.IGNORECASE),
    ),
    "reg": (
        re.compile(r"\bregister=0x[0-9a-f]{2}\s+value=0x[0-9a-f]{4}\s+code=0\b", re.IGNORECASE),
    ),
    "wreg": (
        re.compile(r"\bregister=0x[0-9a-f]{2}\s+value=0x[0-9a-f]{4}\s+code=0\b", re.IGNORECASE),
    ),
    "decode": (re.compile(r"\bdecode kind=(?:status|data)\b", re.IGNORECASE),),
    "driveua": (re.compile(r"\bcode=\d+\s+microamps=\d+\b", re.IGNORECASE),),
    "mode": (re.compile(r"\bmode=(?:single|seq)\s+channel=(?:none|\d+)\s+count=\d+\b", re.IGNORECASE),),
    "refclk": (re.compile(r"\bsource=(?:internal|external)\s+hz=\d+\s+ppm=\d+\b", re.IGNORECASE),),
    "deglitch": (re.compile(r"\bmhz=(?:1|3|10|33)\b", re.IGNORECASE),),
    "activation": (re.compile(r"\bmode=(?:full|low)\b", re.IGNORECASE),),
    "timeout": (re.compile(r"\btimeout_ms=\d+\b", re.IGNORECASE),),
    "rp": (re.compile(r"\benabled=[01]\b", re.IGNORECASE),),
    "autoamp": (re.compile(r"\benabled=[01]\b", re.IGNORECASE),),
    "highcurrent": (re.compile(r"\benabled=[01]\b", re.IGNORECASE),),
    "intbconfig": (re.compile(r"\benabled=[01]\b", re.IGNORECASE),),
    "errors": (re.compile(
        r"\b(?:errors\s+)?data_under=[01]\s+data_over=[01]\s+"
        r"data_watchdog=[01]\s+data_amplitude_high=[01]\s+"
        r"data_amplitude_low=[01]\s+status_under=[01]\s+status_over=[01]\s+"
        r"status_watchdog=[01]\s+status_amplitude_high=[01]\s+"
        r"status_amplitude_low=[01]\s+status_zero_count=[01]\s+"
        r"data_ready=[01]\s+encoded=0x[0-9a-f]{4}\b",
        re.IGNORECASE,
    ),),
    "diag": (
        re.compile(
            r"\bdiag platform=\S+\s+framework=\S+\s+framework_version=\S+\s+"
            r"idf_version=\S+\s+target=\S+\s+frequency_hz=\d+\s+bound=[01]\s+"
            r"applied=\S+\s+revision=\d+\s+profile_dirty=[01]\s+active=[01]\s+"
            r"pending_result=[01]\s+attempts=\d+\s+success=\d+\s+failures=\d+\s+"
            r"last_code=\d+\s+outcome=SUCCESS\s+code=0\b",
            re.IGNORECASE,
        ),
    ),
    "xfer": (
        re.compile(
            r"\b(?:xfer write=|xfer_assert expected_write=)",
            re.IGNORECASE,
        ),
    ),
    "error": (re.compile(r"\bfield=[a-z-]+\s+enabled=[01]\b", re.IGNORECASE),),
    "rcount": (re.compile(r"\bchannel=\d+\s+value=\d+\b", re.IGNORECASE),),
    "settle": (re.compile(r"\bchannel=\d+\s+value=\d+\b", re.IGNORECASE),),
    "findiv": (re.compile(r"\bchannel=\d+\s+value=\d+\b", re.IGNORECASE),),
    "frefdiv": (re.compile(r"\bchannel=\d+\s+value=\d+\b", re.IGNORECASE),),
    "offset": (re.compile(r"\bchannel=\d+\s+value=\d+\b", re.IGNORECASE),),
    "drive": (re.compile(r"\bchannel=\d+\s+code=\d+\b", re.IGNORECASE),),
    "sensorbounds": (
        re.compile(r"\bchannel=\d+\s+low_hz=\d+\s+high_hz=\d+\b", re.IGNORECASE),
    ),
    "addr": (re.compile(r"\baddress=0x(?:2a|2b)\s+build_profile_only=1\b", re.IGNORECASE),),
    "variant": (
        re.compile(r"\bvariant=LDC161[24]\s+variant_channels=(?:2|4)\s+build_profile_only=1\b", re.IGNORECASE),
    ),
    "sd": (
        re.compile(r"\bsd state=(?:asserted|released)\s+outcome=SUCCESS\s+code=0\b", re.IGNORECASE),
    ),
}
COMMAND_BRANCH_EVIDENCE_PATTERNS = {
    "profile show": (
        re.compile(r"\bConfiguration:\s+staged\b", re.IGNORECASE),
        re.compile(r"\bcfg label=staged\b", re.IGNORECASE),
    ),
    "profile validate": (
        re.compile(
            r"\bprofile field=all channel=all dirty=[01] valid=1 "
            r"outcome=VALID code=0 i2c_attempts=0\b",
            re.IGNORECASE,
        ),
    ),
    "profile discard": (
        re.compile(
            r"\bCLI preview:\s*field=profile dirty=0 valid=unknown channel=none "
            r"outcome=DISCARDED code=0 i2c_attempts=0\b",
            re.IGNORECASE,
        ),
    ),
    "profile reset": (
        re.compile(
            r"\bCLI preview:\s*field=profile dirty=1 valid=unknown channel=none "
            r"outcome=RESET code=0 i2c_attempts=0\b",
            re.IGNORECASE,
        ),
    ),
    "profile commit confirm": (
        re.compile(
            r"\bprofile_commit=(?:COMMITTED|UNCHANGED)\s+config_revision=\d+\s+"
            r"applied=\S+\s+i2c_attempts=0\b",
            re.IGNORECASE,
        ),
    ),
    "dump config": (re.compile(r"\bdump complete scope=config count=\d+ failures=0\b", re.IGNORECASE),),
    "dump all confirm": (re.compile(r"\bdump complete scope=all count=\d+ failures=0\b", re.IGNORECASE),),
    "decode status 0x0040": (
        re.compile(r"\bdecode kind=status\s+decoded_status=0x0040\s+observed=1\b", re.IGNORECASE),
    ),
    "decode data 0x0000 0x0000": (
        re.compile(
            r"\bdecode kind=data raw=0x0000 msb=0x0000 lsb=0x0000 "
            r"count=0x0000000 quality=0x[0-9a-f]{4} quality_names=",
            re.IGNORECASE,
        ),
    ),
    "reg 0x7e": (re.compile(r"\bregister=0x7e\s+value=0x5449\s+code=0\b", re.IGNORECASE),),
    "reg 0x7f": (re.compile(r"\bregister=0x7f\s+value=0x3055\s+code=0\b", re.IGNORECASE),),
    "wreg 0x1a 0x3481 confirm": (
        re.compile(r"\bregister=0x1a\s+value=0x3481\s+code=0\b", re.IGNORECASE),
    ),
}
INVALID_INPUT_EVIDENCE_PATTERNS = {
    command.lower(): re.compile(
        r"\busage:\s*" + re.escape(
            COMMAND_BY_NAME[command.split(" ", 1)[0]].synopses[0]
        ) + r"\s*(?:\r?$)",
        re.IGNORECASE | re.MULTILINE,
    )
    for command in INVALID_INPUT_COMMANDS
}
FAIL_PATTERNS = [
    re.compile(pattern, re.IGNORECASE)
    for pattern in (
        r"\bunknown command\b",
        r"\bDEVICE_NOT_FOUND\b",
        r"\bI2C_(?:ERROR|TIMEOUT|NACK_ADDR|NACK_DATA|BUS)\b",
        r"\bINVALID_(?:CONFIG|PARAM)\b",
        r"\bNOT_BOUND\b",
        r"\bBUSY\b",
        r"\b(?:bind|init|probe|read|write|selftest|command)\s+failed\b",
        r"\[FAIL\]",
        r"\[ERR:",
        r"\bmatch=(?:\x1b\[[0-9;]*m)*NO\b",
        r"\bfail=(?:\x1b\[[0-9;]*m)*[1-9][0-9]*\b",
        r"\bread_fail=(?:\x1b\[[0-9;]*m)*[1-9][0-9]*\b",
        r"\bconfig_readback_failures=(?:\x1b\[[0-9;]*m)*[1-9][0-9]*\b",
        r"\bfailed\s*[:=]\s*[1-9][0-9]*\b",
        r"not bound",
        r"\bCLI result:[^\r\n]*\bcode=[1-9][0-9]*\b",
        r"\bStatus:[^\r\n]*\bcode=[1-9][0-9]*\b",
        r"\b(?:first_failure|last_failure)\b[^\r\n]*\bcode=[1-9][0-9]*\b",
    )
]
SENSOR_CONDITION_PATTERNS = [
    re.compile(pattern, re.IGNORECASE)
    for pattern in (
        r"\berr=1\b",
        r"\b(?:errUR|errOR|errWD|errAmp|ur|or|wd|ah|al|zc)=1\b",
        r"\b(?:error|overrun|bounds_fail)=(?:0x0*[1-9a-f][0-9a-f]*|[1-9][0-9]*)\b",
    )
]
NO_SENSOR_CONDITION_COMMANDS = (
    "status", "status_raw", "ready", "drdy", "acquire", "read", "stress",
    "stress_mix", "soak",
)
OK_PATTERNS = [
    re.compile(pattern, re.IGNORECASE)
    for pattern in (
        r"\bOK\b",
        r"code=0",
        r"status:\s*0",
        r"\bCh\d+:\s*raw=0x[0-9a-f]+",
        r"\bch\d+\s+raw=0x[0-9a-f]+",
        r"\bReadFresh result:",
        r"\bReadStaged result:",
        r"\bSampleRate result:.*\bfail=(?:\x1b\[[0-9;]*m)*0\b",
        r"\bStress results:.*\b0\s+failed\b",
    )
]
ANSI_PATTERN = re.compile(r"\x1b\[[0-9;]*m")
PROMPT_BOUNDARY_PATTERN = re.compile(
    r"(?:^|\r?\n)(?:>|ldc1614-idf>)\s*\Z",
    re.IGNORECASE,
)
SAMPLE_LINE_PATTERN = re.compile(r"\bSample\s+\d+\s*/\s*\d+\b", re.IGNORECASE)
SAMPLE_FAIL_PATTERN = re.compile(r"\bSample\s+\d+\s*/\s*\d+\s+failed\b", re.IGNORECASE)
STRESS_RESULT_PATTERN = re.compile(
    r"Stress results:\s*(\d+)\s+ok,\s*(\d+)\s+failed",
    re.IGNORECASE,
)
STRESS_DETAIL_PATTERN = re.compile(
    r"Stress result:\s*requested=(\d+)\s+ok=(\d+)\s+fail=(\d+)"
    r"\s+elapsed_ms=(\d+)(?:\s+hz=(\d+(?:\.\d+)?))?",
    re.IGNORECASE,
)
STRESS_MIX_RESULT_PATTERN = re.compile(
    r"StressMix results:\s*requested=(\d+)\s+ok=(\d+)\s+fail=(\d+)"
    r"\s+elapsed_ms=(\d+)",
    re.IGNORECASE,
)
WATCH_RESULT_PATTERN = re.compile(
    r"Watch results:\s*requested=(\d+)\s+completed=(\d+)\s+failed=(\d+)"
    r"\s+elapsed_ms=(\d+)",
    re.IGNORECASE,
)
SAMPLE_RATE_RESULT_PATTERN = re.compile(
    r"SampleRate result:\s*requested=(\d+)\s+ok=(\d+)\s+fail=(\d+)"
    r"\s+elapsed_ms=(\d+)\s+hz=(\d+(?:\.\d+)?)"
    r"(?:\s+ready_checks=(\d+)\s+ready_status_raw=(0x[0-9a-f]{4}))?",
    re.IGNORECASE,
)
SOAK_SESSION_RESULT_PATTERN = re.compile(
    r"Soak results:\s*seconds=(\d+)\s+cycles=(\d+)\s+ok=(\d+)\s+fail=(\d+)"
    r"\s+elapsed_ms=(\d+)",
    re.IGNORECASE,
)
DISCOVER_RESULT_PATTERN = re.compile(
    r"discover tested=(\d+)\s+responding=(\d+)\s+matched=(\d+)\s+"
    r"mismatched=(\d+)\s+failed=(\d+)\s+variant=UNKNOWN\s+code=(\d+)",
    re.IGNORECASE,
)
DISCOVER_ADDRESS_PATTERN = re.compile(
    r"^discover address=0x([0-9a-f]{2})\s+strap=(ADDR_GND|ADDR_VDD)\s+"
    r"manufacturer_id=(0x[0-9a-f]{4}|unavailable)\s+"
    r"device_id=(0x[0-9a-f]{4}|unavailable)\s+match=([01])\s+"
    r"variant=UNKNOWN\s+code=(\d+)\s+detail=(-?\d+)\s*$",
    re.IGNORECASE | re.MULTILINE,
)
DUMP_RESULT_PATTERN = re.compile(
    r"dump complete\s+scope=(config|all)\s+count=(\d+)\s+failures=(\d+)",
    re.IGNORECASE,
)
DUMP_RECORD_PATTERN = re.compile(
    r"^(dump_config|dump_all)\s+register=0x([0-9a-f]{2})\s+name=(\S+)\s+"
    r"access=(R|RW)\s+destructive=([01])\s+value=0x[0-9a-f]{4}\s+"
    r"reserved_valid=([01])\s*$",
    re.IGNORECASE | re.MULTILINE,
)
VERIFY_RESULT_PATTERN = re.compile(
    r"verify complete\s+checked=(\d+)\s+matched=(\d+)\s+"
    r"mismatched=(\d+)\s+read_failures=(\d+)",
    re.IGNORECASE,
)
SELFTEST_RESULT_PATTERN = re.compile(
    r"Selftest result:\s*pass=(\d+)\s+fail=(\d+)\s+skip=(\d+)",
    re.IGNORECASE,
)
IDENTITY_STRESS_RESULT_PATTERN = re.compile(
    r"IdentityStress result:\s*requested=(\d+)\s+completed=(\d+)\s+"
    r"ok=(\d+)\s+fail=(\d+)\s+first_failure_iteration=(-?\d+)\s+"
    r"elapsed_ms=(\d+)\s+hz=(\d+(?:\.\d+)?)",
    re.IGNORECASE,
)
RESET_STRESS_RESULT_PATTERN = re.compile(
    r"ResetStress result:\s*requested=(\d+)\s+completed=(\d+)\s+"
    r"ok=(\d+)\s+fail=(\d+)\s+first_failure_iteration=(-?\d+)\s+"
    r"elapsed_ms=(\d+)",
    re.IGNORECASE,
)
BUS_FREQUENCY_STRESS_RESULT_PATTERN = re.compile(
    r"BusFrequencyStress result:\s*requested=(\d+)\s+completed=(\d+)\s+"
    r"ok=(\d+)\s+fail=(\d+)\s+initial_hz=(\d+)\s+active_hz=(\d+)\s+"
    r"restored_hz=(\d+)\s+restore_code=(\d+)\s+"
    r"first_failure_iteration=(-?\d+)\s+elapsed_ms=(\d+)",
    re.IGNORECASE,
)
BUS_FREQUENCY_RESULT_PATTERN = re.compile(
    r"busfreq previous_hz=(\d+)\s+requested_hz=(\d+)\s+active_hz=(\d+)\s+"
    r"reinitialized=([01])\s+outcome=(SUCCESS|FAILED)\s+code=(\d+)",
    re.IGNORECASE,
)
BUS_INFO_PATTERN = re.compile(
    r"bus open=([01])\s+backend=(\S+)\s+port=(-?\d+)\s+sda=(-?\d+)\s+"
    r"scl=(-?\d+)\s+sda_level=(-?\d+)\s+scl_level=(-?\d+)\s+"
    r"address=(0x[0-9a-f]{2})\s+frequency_hz=(\d+)\s+timeout_ms=(\d+)",
    re.IGNORECASE,
)
XFER_STATS_PATTERN = re.compile(
    r"xfer write=(\d+)\s+write_read=(\d+)\s+discover=(\d+)\s+total=(\d+)\s+"
    r"failures=(\d+)\s+last_code=(\d+)\s+outcome=(SUCCESS|FAILED)\s+code=(\d+)",
    re.IGNORECASE,
)
XFER_ASSERT_PATTERN = re.compile(
    r"xfer_assert expected_write=(\d+)\s+actual_write=(\d+)\s+"
    r"expected_write_read=(\d+)\s+actual_write_read=(\d+)\s+"
    r"expected_discover=(\d+)\s+actual_discover=(\d+)\s+"
    r"expected_total=(\d+)\s+actual_total=(\d+)\s+"
    r"outcome=(SUCCESS|FAILED)\s+code=(\d+)",
    re.IGNORECASE,
)
CFG_HEADER_PATTERN = re.compile(
    r"^cfg label=(desired|staged)\s+address=0x([0-9a-f]{2})\s+"
    r"variant=(LDC1612|LDC1614)\s+variant_channels=(2|4)\s+"
    r"selected=0x([0-9a-f]{2})\s+mode=(\S+)\s+ref_source=(\S+)\s+"
    r"ref_hz=(\d+)\s+tolerance_ppm=(\d+)\s+deglitch=(\d+)\s+"
    r"activation=(\S+)\s+timeout_ms=(\d+)\s+rp_override=([01])\s+"
    r"auto_amplitude=([01])\s+high_current=([01])\s+intb_config=([01])\s+"
    r"error_reporting=0x([0-9a-f]{4})\s+revision=(\d+)\s+applied=(\S+)\s*$",
    re.IGNORECASE | re.MULTILINE,
)
CFG_CHANNEL_PATTERN = re.compile(
    r"^\s+channel=(\d+)\s+selected=[01]\s+rcount=\d+\s+"
    r"settle_count=\d+\s+fin_divider=\d+\s+fref_divider=\d+\s+"
    r"offset=\d+\s+drive_code=\d+\s+drive_ua=(?:invalid:)?\d+\s+"
    r"sensor_min_hz=\d+\s+sensor_max_hz=\d+\s*$",
    re.IGNORECASE | re.MULTILINE,
)
CFG_ERROR_PATTERN = re.compile(
    r"^\s+error_config=0x[0-9a-f]{4}\s+data_under=[01]\s+data_over=[01]\s+"
    r"data_watchdog=[01]\s+data_amplitude_high=[01]\s+"
    r"data_amplitude_low=[01]\s+status_under=[01]\s+status_over=[01]\s+"
    r"status_watchdog=[01]\s+status_amplitude_high=[01]\s+"
    r"status_amplitude_low=[01]\s+status_zero_count=[01]\s+data_ready=[01]\s*$",
    re.IGNORECASE | re.MULTILINE,
)
DRIVER_RESULT_PATTERN = re.compile(
    r"drv bound=([01])\s+applied=(\S+)\s+revision=(\d+)\s+active=([01])\s+"
    r"result_available=([01])\s+attempts=(\d+)\s+success=(\d+)\s+"
    r"failures=(\d+)\s+last_code=(\d+)",
    re.IGNORECASE,
)
TRANSPORT_RESULT_PATTERN = re.compile(
    r"transport attempts=(\d+)\s+success=(\d+)\s+failures=(\d+)",
    re.IGNORECASE,
)
ADDRESS_PATTERNS = [
    re.compile(pattern, re.IGNORECASE)
    for pattern in (
        r"\b(?:i2c\s+)?addr(?:ess)?\s*[=:]\s*(0x[0-9a-f]+|\d+)",
    )
]
CHANNEL_COUNT_PATTERNS = [
    re.compile(pattern, re.IGNORECASE)
    for pattern in (
        r"\b(?:variant_channels|channel_count)\s*[=:]\s*(\d+)",
    )
]
FIRMWARE_GIT_PATTERN = re.compile(r"\bfirmware_git=([0-9a-f]{7,40}|unknown)\b",
                                  re.IGNORECASE)
FIRMWARE_STATUS_PATTERN = re.compile(r"\bfirmware_status=(clean|dirty|unknown)\b",
                                     re.IGNORECASE)
RUNTIME_VERSION_PATTERN = re.compile(
    r"\bplatform=(\S+)\s+framework=(\S+)\s+framework_version=(\S+)\s+"
    r"idf_version=(\S+)\s+target=(\S+)\s+i2c_backend=(\S+)\s+"
    r"frequency_hz=(\d+)\b",
    re.IGNORECASE,
)
FIRMWARE_STARTUP_BANNER_PATTERN = re.compile(
    r"===\s*LDC1614\s+(?:Arduino|Native ESP-IDF)\s+"
    r"Diagnostic Bring-up Example\s*===",
    re.IGNORECASE,
)


def git_value(args: List[str], default: str = "unknown") -> str:
    try:
        result = subprocess.run(
            ["git", *args],
            cwd=ROOT,
            capture_output=True,
            text=True,
            timeout=3,
            check=False,
        )
    except Exception:
        return default
    value = result.stdout.strip()
    return value if result.returncode == 0 and value else default


def load_library_version() -> str:
    try:
        data = json.loads((ROOT / "library.json").read_text(encoding="utf-8"))
        return str(data.get("version", "unknown"))
    except Exception:
        return "unknown"


def timestamp_utc() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def default_commands(profile: str, fixture: str = "default") -> List[str]:
    if fixture == "no-sensor":
        return list(ARDUINO_NO_SENSOR_COMMANDS)
    if profile == "idf":
        return list(IDF_DEFAULT_COMMANDS)
    return list(ARDUINO_DEFAULT_COMMANDS)


def optional_group_commands(group: str, **values: int) -> List[str]:
    problems = validate_contract()
    if problems:
        raise RuntimeError("invalid CLI host contract: " + "; ".join(problems))
    if group not in OPTIONAL_COMMAND_GROUPS:
        raise ValueError(f"unknown optional command group: {group}")
    return [template.format(**values) for template in OPTIONAL_COMMAND_GROUPS[group]]


def command_name(command: str) -> str:
    return command.strip().split(" ", 1)[0].lower()


def canonical_command_name(command: str) -> str:
    name = command_name(command)
    return COMMAND_BY_NAME[name].canonical if name in COMMAND_BY_NAME else name


def command_is_async(command: str) -> bool:
    return canonical_command_name(command) in ASYNC_CANONICAL_COMMANDS


def normalized_command(command: str) -> str:
    return " ".join(command.strip().lower().split())


def firmware_version_from_transcript(transcript: str) -> str:
    match = FIRMWARE_VERSION_PATTERN.search(transcript)
    return match.group(1) if match is not None else "unknown"


def configuration_matrix_commands(channel_count: int) -> List[str]:
    commands = list(CONFIG_MATRIX_COMMON_COMMANDS)
    if channel_count == 4:
        commands.extend(("mode seq 3", "mode seq 4"))
    for route in ERROR_ROUTE_NAMES:
        commands.append(f"error {route} 1")
    for channel in range(channel_count):
        commands.extend((
            f"rcount {channel} 5", f"rcount {channel} 65535",
            f"settle {channel} 0", f"settle {channel} 65535",
            f"findiv {channel} 1", f"findiv {channel} 15",
            f"frefdiv {channel} 1", f"frefdiv {channel} 1023",
            f"offset {channel} 0", f"offset {channel} 65535",
        ))
    commands.extend(("profile reset", "profile validate", "profile discard", "drv"))
    return commands


def compile_token_patterns(tokens: Iterable[str]) -> List[re.Pattern[str]]:
    return [
        re.compile(re.escape(token), re.IGNORECASE)
        for token in tokens
        if token.strip()
    ]


def compile_scoped_expected_failures(
    entries: Iterable[str],
) -> Dict[str, List[re.Pattern[str]]]:
    scoped: Dict[str, List[re.Pattern[str]]] = {}
    for entry in entries:
        command, separator, token = entry.partition("=")
        normalized = normalized_command(command)
        if not separator or not normalized or not token.strip():
            raise ValueError("expected COMMAND=TOKEN")
        scoped.setdefault(normalized, []).extend(compile_token_patterns((token,)))
    return scoped


def command_argument_uint(command: str, index: int) -> Optional[int]:
    """Return one unsigned CLI argument, or None for malformed input."""
    tokens = command.strip().split()
    if index >= len(tokens):
        return None
    parsed = parse_int_token(tokens[index])
    return parsed if parsed is not None and parsed >= 0 else None


def unique_match(
    pattern: re.Pattern[str], output: str, label: str,
) -> Tuple[Optional[re.Match[str]], Optional[str]]:
    matches = list(pattern.finditer(output))
    if len(matches) != 1:
        return None, f"expected exactly one {label}; observed {len(matches)}"
    return matches[0], None


def reported_rate_mismatch(
    completed: int, elapsed_ms: int, reported_hz: float,
) -> bool:
    if (completed <= 0 or elapsed_ms <= 0 or
            not math.isfinite(reported_hz) or reported_hz <= 0.0):
        return True
    expected_hz = completed * 1000.0 / elapsed_ms
    return not math.isclose(
        reported_hz, expected_hz, rel_tol=1.0e-6, abs_tol=1.0e-6
    )


def expected_register_descriptor(address: int) -> Tuple[str, str, int]:
    if 0x00 <= address <= 0x07:
        channel = address // 2
        suffix = "MSB" if address % 2 == 0 else "LSB"
        return f"DATA{channel}_{suffix}", "R", 1
    ranged_names = (
        (0x08, 0x0B, "RCOUNT"),
        (0x0C, 0x0F, "OFFSET"),
        (0x10, 0x13, "SETTLECOUNT"),
        (0x14, 0x17, "CLOCK_DIVIDERS"),
        (0x1E, 0x21, "DRIVE_CURRENT"),
    )
    for first, last, prefix in ranged_names:
        if first <= address <= last:
            return f"{prefix}{address - first}", "RW", 0
    fixed = {
        0x18: ("STATUS", "R", 1),
        0x19: ("ERROR_CONFIG", "RW", 0),
        0x1A: ("CONFIG", "RW", 0),
        0x1B: ("MUX_CONFIG", "RW", 0),
        0x1C: ("RESET_DEV", "RW", 0),
        0x7E: ("MANUFACTURER_ID", "R", 0),
        0x7F: ("DEVICE_ID", "R", 0),
    }
    return fixed.get(address, ("UNMAPPED", "-", 0))


def command_semantic_failure(
    command: str, output: str, profile: str = "arduino",
) -> Optional[str]:
    """Reject internally inconsistent success summaries.

    Shape-only regular expressions are not acceptance evidence for counted
    diagnostics: the firmware's requested/completed counters must reconcile
    with the command that the runner actually sent.
    """
    canonical = canonical_command_name(command)
    tokens = normalized_command(command).split()

    if canonical in ("color", "verbose"):
        match, failure = unique_match(
            re.compile(rf"\b{canonical} enabled=([01])\b", re.IGNORECASE),
            output,
            f"{canonical} state",
        )
        if failure is not None:
            return failure
        if len(tokens) == 2:
            expected = {
                "color": {"on": 1, "off": 0},
                "verbose": {"1": 1, "0": 0},
            }[canonical].get(tokens[1])
            if expected is None or int(match.group(1)) != expected:
                return f"{canonical} state does not match the requested value"
        return None

    simple_setting_patterns = {
        "deglitch": (r"\bmhz=(\d+)\b", (1,)),
        "activation": (r"\bmode=(\S+)\b", (1,)),
        "timeout": (r"\btimeout_ms=(\d+)\b", (1,)),
        "rp": (r"\benabled=([01])\b", (1,)),
        "autoamp": (r"\benabled=([01])\b", (1,)),
        "highcurrent": (r"\benabled=([01])\b", (1,)),
        "intbconfig": (r"\benabled=([01])\b", (1,)),
        "error": (r"\bfield=(\S+)\s+enabled=([01])\b", (1, 2)),
        "rcount": (r"\bchannel=(\d+)\s+value=(\d+)\b", (1, 2)),
        "settle": (r"\bchannel=(\d+)\s+value=(\d+)\b", (1, 2)),
        "findiv": (r"\bchannel=(\d+)\s+value=(\d+)\b", (1, 2)),
        "frefdiv": (r"\bchannel=(\d+)\s+value=(\d+)\b", (1, 2)),
        "offset": (r"\bchannel=(\d+)\s+value=(\d+)\b", (1, 2)),
        "drive": (r"\bchannel=(\d+)\s+code=(\d+)\b", (1, 2)),
        "sensorbounds": (
            r"\bchannel=(\d+)\s+low_hz=(\d+)\s+high_hz=(\d+)\b",
            (1, 2, 3),
        ),
    }
    if canonical in simple_setting_patterns:
        pattern_text, argument_indices = simple_setting_patterns[canonical]
        match, failure = unique_match(
            re.compile(pattern_text, re.IGNORECASE), output,
            f"{canonical} setting result",
        )
        if failure is not None:
            return failure
        assert match is not None
        if len(tokens) != len(argument_indices) + 1:
            return f"runner could not parse requested {canonical} arguments"
        for group_index, argument_index in enumerate(argument_indices, start=1):
            reported = match.group(group_index).lower()
            requested = tokens[argument_index].lower()
            reported_number = parse_int_token(reported)
            requested_number = parse_int_token(requested)
            if (reported_number != requested_number if requested_number is not None
                    else reported != requested):
                return f"{canonical} output does not match the requested value"
        return None

    if canonical == "mode":
        if len(tokens) != 3:
            return "runner could not parse requested mode"
        if tokens[1] == "single":
            pattern = re.compile(
                r"\bmode=single\s+channel=(\d+)\s+count=1\b", re.IGNORECASE
            )
        else:
            pattern = re.compile(
                r"\bmode=seq\s+channel=none\s+count=(\d+)\b", re.IGNORECASE
            )
        match, failure = unique_match(pattern, output, "mode setting result")
        if failure is not None:
            return failure
        assert match is not None
        if parse_int_token(tokens[2]) != int(match.group(1)):
            return "mode output does not match the requested mode"
        return None

    if canonical == "refclk":
        match, failure = unique_match(
            re.compile(
                r"\bsource=(internal|external)\s+hz=(\d+)\s+ppm=(\d+)\b",
                re.IGNORECASE,
            ),
            output,
            "reference-clock setting result",
        )
        if failure is not None:
            return failure
        assert match is not None
        if (len(tokens) != 4 or match.group(1).lower() != tokens[1] or
                int(match.group(2)) != parse_int_token(tokens[2]) or
                int(match.group(3)) != parse_int_token(tokens[3])):
            return "reference-clock output does not match the requested values"
        return None

    if canonical == "errors" and len(tokens) == 2 and tokens[1] in ("all", "none"):
        match, failure = unique_match(
            re.compile(
                r"\berrors\s+" + r"\s+".join(
                    rf"{field}=([01])" for field in (
                        "data_under", "data_over", "data_watchdog",
                        "data_amplitude_high", "data_amplitude_low",
                        "status_under", "status_over", "status_watchdog",
                        "status_amplitude_high", "status_amplitude_low",
                        "status_zero_count", "data_ready",
                    )
                ) + r"\s+encoded=0x([0-9a-f]{4})\b",
                re.IGNORECASE,
            ),
            output,
            "error-route setting result",
        )
        if failure is not None:
            return failure
        assert match is not None
        expected_bit = 1 if tokens[1] == "all" else 0
        expected_encoded = 0xF8FD if expected_bit else 0
        if (any(int(match.group(index)) != expected_bit for index in range(1, 13)) or
                int(match.group(13), 16) != expected_encoded):
            return "error-route output does not match the requested preset"
        return None

    if canonical == "initdrive":
        match = re.search(r"\bchannel=(\d+)\s+init_drive_code=", output,
                          re.IGNORECASE)
        if (match is None or len(tokens) != 2 or
                int(match.group(1)) != parse_int_token(tokens[1])):
            return "initial-drive output does not match the requested channel"
        return None

    if canonical == "driveua":
        match = re.search(r"\bcode=(\d+)\s+microamps=", output, re.IGNORECASE)
        if (match is None or len(tokens) != 2 or
                int(match.group(1)) != parse_int_token(tokens[1])):
            return "drive-current output does not match the requested code"
        return None

    if canonical in ("reg", "wreg"):
        match = re.search(
            rf"(?:\b{canonical}\s+)?register=0x([0-9a-f]{{2}})\b[^\r\n]*"
            + (r"\bvalue=0x([0-9a-f]{4})\b" if canonical == "wreg" else ""),
            output,
            re.IGNORECASE,
        )
        minimum_tokens = 3 if canonical == "wreg" else 2
        if (match is None or len(tokens) < minimum_tokens or
                int(match.group(1), 16) != parse_int_token(tokens[1])):
            return f"{canonical} output does not match the requested register"
        if (canonical == "wreg" and
                int(match.group(2), 16) != parse_int_token(tokens[2])):
            return "wreg output does not match the requested value"
        return None

    if canonical == "decode":
        if len(tokens) == 3 and tokens[1] == "status":
            match = re.search(
                r"\bdecode kind=status\s+decoded_status=0x([0-9a-f]{4})\b",
                output, re.IGNORECASE,
            )
            if (match is None or
                    int(match.group(1), 16) != parse_int_token(tokens[2])):
                return "decoded status does not match the requested raw value"
            return None
        if len(tokens) == 4 and tokens[1] == "data":
            match = re.search(
                r"\bdecode kind=data\s+raw=0x([0-9a-f]{4})\s+"
                r"msb=0x([0-9a-f]{4})\s+lsb=0x([0-9a-f]{4})\b",
                output, re.IGNORECASE,
            )
            if (match is None or int(match.group(1), 16) != parse_int_token(tokens[2]) or
                    int(match.group(2), 16) != parse_int_token(tokens[2]) or
                    int(match.group(3), 16) != parse_int_token(tokens[3])):
                return "decoded data does not match the requested MSB/LSB values"
            return None
        return "runner could not parse requested decode arguments"

    if canonical == "freq" and len(tokens) == 4:
        match = re.search(
            r"\bprofile=(desired|staged)\s+channel=(\d+)\s+"
            r"raw=0x([0-9a-f]{7})\s+frequency_hz=", output, re.IGNORECASE,
        )
        if (match is None or match.group(1).lower() != tokens[1] or
                int(match.group(2)) != parse_int_token(tokens[2]) or
                int(match.group(3), 16) != parse_int_token(tokens[3])):
            return "frequency output does not match the requested profile/channel/raw value"
        return None

    if canonical == "timing" and len(tokens) == 3:
        match = re.search(
            r"\bprofile=(desired|staged)\s+mask=0x([0-9a-f]{2})\b",
            output, re.IGNORECASE,
        )
        if (match is None or match.group(1).lower() != tokens[1] or
                int(match.group(2), 16) != parse_int_token(tokens[2])):
            return "timing output does not match the requested profile/mask"
        return None

    if canonical == "sd" and len(tokens) == 3:
        match = re.search(r"\bsd state=(asserted|released)\b", output,
                          re.IGNORECASE)
        expected = "asserted" if tokens[1] == "assert" else "released"
        if match is None or match.group(1).lower() != expected:
            return "SD output does not match the requested pin action"
        return None

    if canonical == "read" and len(tokens) == 2:
        match = re.search(
            r"\bbatch type=SEQUENTIAL_READOUT\s+selected=0x([0-9a-f]{2})\b",
            output, re.IGNORECASE,
        )
        if match is None or int(match.group(1), 16) != parse_int_token(tokens[1]):
            return "acquisition batch does not match the requested channel mask"
        return None

    if canonical == "help":
        count_matches = re.findall(
            rf"\bcommand_count={len(COMMAND_SPECS)}\b", output, re.IGNORECASE
        )
        if len(count_matches) != 1:
            return "help did not report exactly one current command count"
        if len(tokens) == 1:
            sections = re.findall(r"^\[([^\]\r\n]+)\]\s*$", output, re.MULTILINE)
            expected_sections = tuple(
                HELP_SECTION_LABELS[section] for section in SECTION_ORDER
            )
            if tuple(sections) != expected_sections:
                return "help sections are missing, duplicated, or out of order"
            for spec in COMMAND_SPECS:
                for synopsis in spec.synopses:
                    row_pattern = re.compile(
                        r"^\s{2}" + re.escape(synopsis) + r"\s+-\s+",
                        re.IGNORECASE | re.MULTILINE,
                    )
                    matches = row_pattern.findall(output)
                    if len(matches) != 1:
                        return (
                            f"help synopsis {synopsis!r} appeared "
                            f"{len(matches)} times"
                        )
            return None
        if len(tokens) != 2 or tokens[1] not in COMMAND_BY_NAME:
            return "runner could not resolve the requested help topic"
        spec = COMMAND_BY_NAME[tokens[1]]
        detail = re.search(
            r"\bhelp command=(\S+)\s+aliases=(\S+)\s+section=(.*?)\s+"
            r"execution=(\S+)\s+safety=(\S+)\s+fixture=(\S+)\s+"
            r"busy_allowed=([01])\s+evidence=([^\r\n]+)",
            output,
            re.IGNORECASE,
        )
        if detail is None:
            return "detailed help metadata is incomplete"
        expected_aliases = " ".join(spec.aliases) if spec.aliases else "none"
        if (detail.group(1).lower() != spec.canonical or
                detail.group(2).lower() != expected_aliases.lower() or
                detail.group(3).lower() != HELP_SECTION_LABELS[spec.section].lower() or
                detail.group(4).upper() != spec.execution or
                detail.group(5).upper() != spec.safety or
                detail.group(6).upper() != spec.fixture):
            return "detailed help metadata does not match the host contract"
        for synopsis in spec.synopses:
            if synopsis not in output:
                return f"detailed help omitted synopsis {synopsis!r}"
        return None

    if canonical == "cfg":
        headers = list(CFG_HEADER_PATTERN.finditer(output))
        if len(headers) != 2 or [match.group(1).lower() for match in headers] != [
                "desired", "staged"]:
            return "cfg did not emit exactly one desired and one staged profile"
        binding_facts = [
            (header.group(2).lower(), header.group(3).upper(), int(header.group(4)))
            for header in headers
        ]
        if binding_facts[0] != binding_facts[1]:
            return "desired and staged profiles disagree on immutable binding facts"
        channel_count = int(headers[0].group(4))
        if int(headers[1].group(4)) != channel_count:
            return "desired and staged profiles disagree on physical variant size"
        expected_variant = "LDC1612" if channel_count == 2 else "LDC1614"
        expected_physical_mask = (1 << channel_count) - 1
        for index, header in enumerate(headers):
            block_end = headers[index + 1].start() if index + 1 < len(headers) else len(output)
            block = output[header.end():block_end]
            required_lines = (
                r"^\s+binding address=0x[0-9a-f]{2}\s+variant=LDC161[24]\s+"
                r"physical_mask=0x[0-9a-f]{2}\s+write_callback=1\s+read_callback=1\s*$",
                r"^\s+conversion selected_mask=0x[0-9a-f]{2}\s+mode=\S+\s+"
                r"active_channel=\d+\s+rr_sequence=\d+\s*$",
                r"^\s+reference_clock source=\S+\s+frequency_hz=\d+\s+"
                r"tolerance_ppm=\d+\s*$",
                r"^\s+options timeout_ms=\d+\s+deglitch=\d+\s+activation=\S+\s+"
                r"rp_override=[01]\s+auto_amplitude=[01]\s+high_current=[01]\s+"
                r"intb_enabled=[01]\s+intb_callback=[01]\s*$",
            )
            if any(re.search(pattern, block, re.IGNORECASE | re.MULTILINE) is None
                   for pattern in required_lines):
                return "cfg profile block omitted binding/conversion/clock/options"
            address = int(header.group(2), 16)
            selected_mask = int(header.group(5), 16)
            if (header.group(3).upper() != expected_variant or
                    address not in (0x2A, 0x2B) or selected_mask == 0 or
                    selected_mask & ~expected_physical_mask):
                return "cfg header contains inconsistent variant/address/channel facts"

            binding = re.search(
                r"^\s+binding address=0x([0-9a-f]{2})\s+variant=(LDC161[24])\s+"
                r"physical_mask=0x([0-9a-f]{2})\s+write_callback=1\s+"
                r"read_callback=1\s*$",
                block, re.IGNORECASE | re.MULTILINE,
            )
            conversion = re.search(
                r"^\s+conversion selected_mask=0x([0-9a-f]{2})\s+mode=(\S+)\s+"
                r"active_channel=(\d+)\s+rr_sequence=(\d+)\s*$",
                block, re.IGNORECASE | re.MULTILINE,
            )
            clock = re.search(
                r"^\s+reference_clock source=(\S+)\s+frequency_hz=(\d+)\s+"
                r"tolerance_ppm=(\d+)\s*$",
                block, re.IGNORECASE | re.MULTILINE,
            )
            options = re.search(
                r"^\s+options timeout_ms=(\d+)\s+deglitch=(\d+)\s+"
                r"activation=(\S+)\s+rp_override=([01])\s+auto_amplitude=([01])\s+"
                r"high_current=([01])\s+intb_enabled=([01])\s+intb_callback=[01]\s*$",
                block, re.IGNORECASE | re.MULTILINE,
            )
            assert binding is not None and conversion is not None
            assert clock is not None and options is not None
            if (int(binding.group(1), 16) != address or
                    binding.group(2).upper() != expected_variant or
                    int(binding.group(3), 16) != expected_physical_mask or
                    int(conversion.group(1), 16) != selected_mask or
                    conversion.group(2).lower() != header.group(6).lower() or
                    clock.group(1).lower() != header.group(7).lower() or
                    int(clock.group(2)) != int(header.group(8)) or
                    int(clock.group(3)) != int(header.group(9)) or
                    int(options.group(1)) != int(header.group(12)) or
                    int(options.group(2)) != int(header.group(10)) or
                    options.group(3).lower() != header.group(11).lower() or
                    tuple(int(options.group(group)) for group in range(4, 8)) !=
                    tuple(int(header.group(group)) for group in range(13, 17))):
                return "cfg detail lines disagree with their profile header"

            channel_rows = re.findall(
                r"^\s+channel=(\d+)\s+selected=([01])\s+", block,
                re.IGNORECASE | re.MULTILINE,
            )
            channels = [int(value) for value in CFG_CHANNEL_PATTERN.findall(block)]
            if channels != list(range(channel_count)):
                return "cfg profile block omitted or duplicated physical channels"
            if (len(channel_rows) != channel_count or
                    any(int(selected) != ((selected_mask >> int(channel)) & 1)
                        for channel, selected in channel_rows)):
                return "cfg channel selection rows disagree with the selected mask"
            error_match = CFG_ERROR_PATTERN.search(block)
            if error_match is None or len(CFG_ERROR_PATTERN.findall(block)) != 1:
                return "cfg profile block omitted complete error-route evidence"
            error_value = re.search(r"^\s+error_config=0x([0-9a-f]{4})\b", block,
                                    re.IGNORECASE | re.MULTILINE)
            if (error_value is None or
                    int(error_value.group(1), 16) != int(header.group(17), 16)):
                return "cfg error-route detail disagrees with its profile header"
        return None

    if canonical == "diag":
        match, failure = unique_match(
            re.compile(
                r"\bdiag platform=(\S+)\s+framework=(\S+)\s+"
                r"framework_version=(\S+)\s+idf_version=(\S+)\s+target=(\S+)\s+"
                r"frequency_hz=(\d+)\s+bound=([01])\s+applied=(\S+)\s+"
                r"revision=(\d+)\s+profile_dirty=([01])\s+active=([01])\s+"
                r"pending_result=([01])\s+attempts=(\d+)\s+success=(\d+)\s+"
                r"failures=(\d+)\s+last_code=(\d+)\s+outcome=SUCCESS\s+code=0\b",
                re.IGNORECASE,
            ),
            output,
            "diagnostic summary",
        )
        if failure is not None:
            return failure
        assert match is not None
        frequency_hz = int(match.group(6))
        attempts, successes, failures, last_code = (
            int(match.group(index)) for index in range(13, 17)
        )
        if (any(match.group(index).lower() in ("", "unknown")
                for index in (1, 2, 3, 4, 5)) or
                not 10000 <= frequency_hz <= 400000):
            return "diagnostic summary omitted usable platform/bus provenance"
        if successes + failures != attempts:
            return "diagnostic transport counters do not reconcile"
        if attempts == 0 and (failures != 0 or last_code != 0):
            return "empty diagnostic transport baseline retained stale failure evidence"
        if failures == 0 and last_code != 0:
            return "successful diagnostic transport history retained a failure status"
        return None

    if canonical == "job":
        job, failure = unique_match(
            re.compile(
                r"\bjob active=([01])\s+operation=(\d+)\s+kind=(\S+)\s+"
                r"phase=(\S+)\s+transfers=(\d+)\s+maximum=(\d+)\s+"
                r"requested=0x([0-9a-f]{2})\s+completed=0x([0-9a-f]{2})\s+"
                r"deadline_ms=(\d+)", re.IGNORECASE,
            ),
            output,
            "core job progress",
        )
        if failure is not None:
            return failure
        session, failure = unique_match(
            re.compile(
                r"\bsession active=([01])\s+id=(\d+)\s+kind=(\S+)\s+"
                r"phase=(\d+)\s+completed=(\d+)/(\d+)", re.IGNORECASE,
            ),
            output,
            "CLI session progress",
        )
        if failure is not None:
            return failure
        assert job is not None and session is not None
        transfers, maximum = int(job.group(5)), int(job.group(6))
        requested, completed = int(job.group(7), 16), int(job.group(8), 16)
        session_completed, session_total = int(session.group(5)), int(session.group(6))
        if (transfers > maximum or completed & ~requested or
                session_completed > session_total or
                int(job.group(1)) != int(session.group(1))):
            return "job/session progress violates bounded cooperative invariants"
        return None

    if canonical == "result":
        match = re.search(
            r"\bOperation result:\s*operation=\d+\s+kind=\S+\s+outcome=\S+\s+"
            r"effects=0x[0-9a-f]{2}\b[^\r\n]*\btransfers=(\d+)\s+"
            r"maximum=(\d+)\s+code=0\b", output, re.IGNORECASE,
        )
        if match is None or int(match.group(1)) > int(match.group(2)):
            return "terminal result violates its fixed transfer bound"
        return None

    if canonical == "drv":
        driver, failure = unique_match(DRIVER_RESULT_PATTERN, output, "driver summary")
        if failure is not None:
            return failure
        transport, failure = unique_match(
            TRANSPORT_RESULT_PATTERN, output, "transport summary"
        )
        if failure is not None:
            return failure
        assert driver is not None and transport is not None
        driver_counts = tuple(int(driver.group(index)) for index in range(6, 9))
        transport_counts = tuple(int(transport.group(index)) for index in range(1, 4))
        if driver_counts != transport_counts:
            return "driver and transport counters disagree"
        attempts, successes, failures = driver_counts
        if successes + failures != attempts:
            return "driver transport counters do not reconcile"
        config_faults = re.findall(
            r"^config_fault valid=[01]\s+job=\S+\s+phase=\S+\s+"
            r"register=0x[0-9a-f]{2}\s+channel=\d+\s+effects=0x[0-9a-f]{2}\s+"
            r"effects_names=\S+\s*$",
            output,
            re.IGNORECASE | re.MULTILINE,
        )
        if len(config_faults) != 1:
            return "driver summary omitted structured configuration-fault provenance"
        return None

    if canonical == "discover":
        match, failure = unique_match(
            DISCOVER_RESULT_PATTERN, output, "discovery summary"
        )
        if failure is not None:
            return failure
        assert match is not None
        tested, responding, matched, mismatched, failed, code = (
            int(match.group(index)) for index in range(1, 7)
        )
        records = list(DISCOVER_ADDRESS_PATTERN.finditer(output))
        if tested != 2 or code != 0 or len(records) != 2:
            return "discovery did not report exactly two qualified address tests"
        record_addresses = [int(record.group(1), 16) for record in records]
        if set(record_addresses) != {0x2A, 0x2B}:
            return "discovery records were not unique 0x2A/0x2B tests"
        observed_responding = 0
        observed_matched = 0
        observed_mismatched = 0
        observed_failed = 0
        for record in records:
            address = int(record.group(1), 16)
            strap = record.group(2).upper()
            manufacturer = record.group(3).lower()
            device = record.group(4).lower()
            record_match = int(record.group(5))
            record_code = int(record.group(6))
            record_detail = int(record.group(7))
            expected_strap = "ADDR_GND" if address == 0x2A else "ADDR_VDD"
            if strap != expected_strap:
                return "discovery address strap label is inconsistent"
            if manufacturer != "unavailable":
                observed_responding += 1
            if manufacturer == "unavailable" or device == "unavailable":
                observed_failed += 1
                if record_match != 0 or record_code == 0:
                    return "failed discovery record reported match or code=0"
                if record_code != 14:
                    return (
                        "absent discovery address was not retained as the generic "
                        "I2C_ERROR status"
                    )
                if profile == "arduino" and record_detail != 259:
                    return (
                        "pioarduino 55.03.311 absent-address discovery did not "
                        "retain ESP-IDF 5.5.5 raw detail 259"
                    )
            elif record_match != 0:
                observed_matched += 1
                if (manufacturer != "0x5449" or device != "0x3055" or
                        record_code != 0):
                    return "matched discovery record did not contain exact LDC IDs"
            else:
                observed_mismatched += 1
                if record_code != 0:
                    return "identity-mismatch discovery record reported read failure"
        if (responding, matched, mismatched, failed) != (
                observed_responding, observed_matched,
                observed_mismatched, observed_failed):
            return "discovery summary counters do not reconcile with address records"
        if (matched == 0 or mismatched != 0 or
                matched + mismatched + failed != tested):
            return "discovery did not cleanly qualify the supported LDC addresses"
        return None

    if canonical == "busfreq":
        match, failure = unique_match(
            BUS_FREQUENCY_RESULT_PATTERN, output, "bus-frequency result"
        )
        if failure is not None:
            return failure
        assert match is not None
        previous, reported_requested, active = (
            int(match.group(index)) for index in range(1, 4)
        )
        reinitialized = int(match.group(4))
        outcome, code = match.group(5).upper(), int(match.group(6))
        tokens = normalized_command(command).split()
        requested = previous if len(tokens) == 1 else command_argument_uint(command, 1)
        if requested is None or reported_requested != requested:
            return "bus-frequency requested value does not match the command"
        if not 10000 <= active <= 400000 or active != requested:
            return "bus-frequency callback did not report the requested active rate"
        if outcome != "SUCCESS" or code != 0 or reinitialized != 0:
            return "bus-frequency query/change did not report its explicit success contract"
        return None

    if canonical == "bus":
        match, failure = unique_match(BUS_INFO_PATTERN, output, "bus summary")
        if failure is not None:
            return failure
        assert match is not None
        open_state = int(match.group(1))
        backend = match.group(2).lower()
        port, sda, scl, sda_level, scl_level = (
            int(match.group(index)) for index in range(3, 8)
        )
        address = int(match.group(8), 16)
        frequency_hz, timeout_ms = int(match.group(9)), int(match.group(10))
        if (open_state != 1 or backend in ("", "unknown") or port < 0 or
                sda < 0 or scl < 0 or sda_level not in (0, 1) or
                scl_level not in (0, 1) or address not in (0x2A, 0x2B) or
                not 10000 <= frequency_hz <= 400000 or timeout_ms == 0):
            return "bus summary did not report a usable bounded owner bus"
        return None

    if canonical == "xfer" and normalized_command(command) in (
            "xfer stats", "xfer reset"):
        match, failure = unique_match(XFER_STATS_PATTERN, output, "xfer summary")
        if failure is not None:
            return failure
        assert match is not None
        writes, write_reads, discoveries, total, failures, last_code = (
            int(match.group(index)) for index in range(1, 7)
        )
        if total != writes + write_reads + discoveries or failures > total:
            return "xfer counters do not reconcile"
        if total == 0 and (failures != 0 or last_code != 0):
            return "empty xfer baseline retained stale failure evidence"
        if match.group(7).upper() != "SUCCESS" or int(match.group(8)) != 0:
            return "xfer summary did not report SUCCESS/code=0"
        return None

    if canonical == "xfer" and normalized_command(command).startswith(
            "xfer assert "):
        match, failure = unique_match(XFER_ASSERT_PATTERN, output, "xfer assertion")
        if failure is not None:
            return failure
        tokens = normalized_command(command).split()
        if len(tokens) != 6:
            return "runner could not parse xfer assertion arguments"
        requested = [command_argument_uint(command, index) for index in range(2, 6)]
        if any(value is None for value in requested):
            return "runner could not parse xfer assertion arguments"
        assert match is not None
        expected = [int(match.group(index)) for index in (1, 3, 5, 7)]
        actual = [int(match.group(index)) for index in (2, 4, 6, 8)]
        if expected != requested or actual != requested:
            return "xfer assertion output does not match requested counters"
        if requested[3] != sum(requested[:3]):
            return "xfer assertion requested an inconsistent total"
        if match.group(9).upper() != "SUCCESS" or int(match.group(10)) != 0:
            return "xfer assertion did not report SUCCESS/code=0"
        return None

    if canonical == "dump":
        match, failure = unique_match(DUMP_RESULT_PATTERN, output, "dump summary")
        if failure is not None:
            return failure
        assert match is not None
        expected_scope = "all" if normalized_command(command) == "dump all confirm" else "config"
        scope, count, failures = match.group(1).lower(), int(match.group(2)), int(match.group(3))
        if scope != expected_scope:
            return f"dump scope {scope} does not match requested scope {expected_scope}"
        if count == 0 or failures != 0:
            return "dump summary reported no reads or one or more failures"
        records = list(DUMP_RECORD_PATTERN.finditer(output))
        expected_record_scope = "dump_all" if scope == "all" else "dump_config"
        addresses = [int(record.group(2), 16) for record in records]
        if (len(records) != count or len(set(addresses)) != count or
                any(record.group(1).lower() != expected_record_scope
                    for record in records)):
            return "dump records do not reconcile with scope/count"
        expected_sets = (
            (LDC1614_ALL_REGISTERS, LDC1612_ALL_REGISTERS)
            if scope == "all"
            else (LDC1614_CONFIG_REGISTERS, LDC1612_CONFIG_REGISTERS)
        )
        if frozenset(addresses) not in expected_sets:
            return "dump register set is not valid for LDC1614 or LDC1612"
        if any(record.group(3).upper() == "UNMAPPED" or record.group(6) != "1"
               for record in records):
            return "dump reported an unknown register or invalid reserved bits"
        for record in records:
            expected_name, expected_access, expected_destructive = (
                expected_register_descriptor(int(record.group(2), 16))
            )
            if (record.group(3).upper() != expected_name or
                    record.group(4).upper() != expected_access or
                    int(record.group(5)) != expected_destructive):
                return "dump register descriptor metadata is inconsistent"
        return None

    if canonical == "verify":
        match, failure = unique_match(VERIFY_RESULT_PATTERN, output, "verify summary")
        if failure is not None:
            return failure
        assert match is not None
        checked, matched, mismatched, read_failures = (
            int(match.group(index)) for index in range(1, 5)
        )
        if checked not in (13, 23):
            return "verify summary did not cover the complete variant register set"
        if matched + mismatched + read_failures != checked:
            return "verify counters do not reconcile with checked count"
        if mismatched != 0 or read_failures != 0:
            return "verify summary reported mismatches or read failures"
        return None

    if canonical == "selftest":
        match, failure = unique_match(SELFTEST_RESULT_PATTERN, output, "selftest summary")
        if failure is not None:
            return failure
        assert match is not None
        passed, failed, skipped = (int(match.group(index)) for index in range(1, 4))
        if passed < 8 or skipped > 2:
            return "selftest summary did not execute the maintained check set"
        if failed != 0:
            return "selftest summary reported one or more failures"
        return None

    if canonical in ("stress_id", "stress_reset", "stress_busfreq"):
        pattern = {
            "stress_id": IDENTITY_STRESS_RESULT_PATTERN,
            "stress_reset": RESET_STRESS_RESULT_PATTERN,
            "stress_busfreq": BUS_FREQUENCY_STRESS_RESULT_PATTERN,
        }[canonical]
        match, failure = unique_match(pattern, output, f"{canonical} summary")
        if failure is not None:
            return failure
        requested = command_argument_uint(command, 1)
        if requested is None:
            return f"runner could not parse requested {canonical} count"
        assert match is not None
        reported, completed, succeeded, failed = (
            int(match.group(index)) for index in range(1, 5)
        )
        if reported != requested or completed != reported:
            return f"{canonical} requested/completed counts do not reconcile"
        if succeeded + failed != completed or failed != 0 or succeeded != reported:
            return f"{canonical} success/failure counts do not reconcile"
        if canonical == "stress_id":
            if int(match.group(5)) != -1:
                return "stress_id reported a first failure on a successful run"
            if reported_rate_mismatch(
                    completed, int(match.group(6)), float(match.group(7))):
                return "stress_id rate does not reconcile with count and elapsed time"
        elif canonical == "stress_reset":
            if int(match.group(5)) != -1:
                return "stress_reset reported a first failure on a successful run"
        else:
            initial_hz = int(match.group(5))
            active_hz = int(match.group(6))
            restored_hz = int(match.group(7))
            restore_code = int(match.group(8))
            first_failure = int(match.group(9))
            if (not 10000 <= initial_hz <= 400000 or
                    active_hz != initial_hz or restored_hz != initial_hz or
                    restore_code != 0 or first_failure != -1):
                return "stress_busfreq did not restore and reinitialize its initial rate"
        return None

    counted_patterns = {
        "stress": (STRESS_DETAIL_PATTERN, 1, "stress"),
        "stress_mix": (STRESS_MIX_RESULT_PATTERN, 1, "stress_mix"),
        "watch": (WATCH_RESULT_PATTERN, 2, "watch"),
        "samplerate": (SAMPLE_RATE_RESULT_PATTERN, 2, "samplerate"),
    }
    if canonical in counted_patterns:
        pattern, argument_index, label = counted_patterns[canonical]
        requested = command_argument_uint(command, argument_index)
        match, failure = unique_match(pattern, output, f"{label} summary")
        if failure is not None:
            return failure
        if requested is None:
            return f"runner could not parse requested {label} count"
        assert match is not None
        reported, succeeded, failed = (
            int(match.group(index)) for index in range(1, 4)
        )
        if reported != requested:
            return (
                f"{label} reported requested={reported}, but command requested "
                f"{requested}"
            )
        if succeeded + failed != reported:
            return f"{label} success/failure counters do not reconcile"
        if failed != 0 or succeeded != reported:
            return f"{label} did not complete every requested operation successfully"
        if canonical in ("stress", "samplerate"):
            if match.group(5) is None:
                return f"{label} rate evidence is missing"
            elapsed_ms = int(match.group(4))
            reported_hz = float(match.group(5))
            if reported_rate_mismatch(succeeded, elapsed_ms, reported_hz):
                return f"{label} rate does not reconcile with count and elapsed time"
        if canonical == "samplerate":
            if match.group(6) is None or match.group(7) is None:
                return "samplerate readiness evidence is missing"
            ready_checks = int(match.group(6))
            ready_status = int(match.group(7), 16)
            if ready_checks < reported or (ready_status & 0x0040) == 0:
                return "samplerate readiness evidence does not reconcile"
        return None

    if canonical == "soak":
        requested_seconds = command_argument_uint(command, 1)
        match, failure = unique_match(
            SOAK_SESSION_RESULT_PATTERN, output, "soak summary"
        )
        if failure is not None:
            return failure
        if requested_seconds is None:
            return "runner could not parse requested soak duration"
        assert match is not None
        seconds, cycles, succeeded, failed, elapsed_ms = (
            int(match.group(index)) for index in range(1, 6)
        )
        if seconds != requested_seconds:
            return (
                f"soak reported seconds={seconds}, but command requested "
                f"{requested_seconds}"
            )
        if cycles == 0 or succeeded + failed != cycles:
            return "soak cycle counters do not reconcile or no cycle completed"
        if failed != 0 or succeeded != cycles:
            return "soak reported one or more failed cycles"
        if elapsed_ms < seconds * 1000:
            return "soak elapsed time is shorter than the requested duration"
        return None

    return None


def classify_command(
    command: str,
    output: str,
    timed_out: bool,
    expected_patterns: Optional[List[re.Pattern[str]]] = None,
    failure_patterns: Optional[List[re.Pattern[str]]] = None,
    expected_failure_patterns: Optional[List[re.Pattern[str]]] = None,
    fixture: str = "default",
    profile: str = "arduino",
) -> Tuple[str, str]:
    if timed_out:
        return "FAIL", "command response timed out"
    if not output.strip():
        return "FAIL", "no response captured"

    parsed_output = strip_ansi(output)
    if PROMPT_BOUNDARY_PATTERN.search(parsed_output) is None:
        return "FAIL", "command response did not end at a complete CLI prompt boundary"

    for pattern in failure_patterns or []:
        if pattern.search(output):
            return "FAIL", f"matched configured failure token: {pattern.pattern}"

    expected_failure_match = next(
        (
            pattern
            for pattern in expected_failure_patterns or []
            if pattern.search(parsed_output)
        ),
        None,
    )

    invalid_pattern = INVALID_INPUT_EVIDENCE_PATTERNS.get(normalized_command(command))
    if invalid_pattern is not None:
        if invalid_pattern.search(parsed_output) is None:
            return "FAIL", "invalid input did not emit its exact usage contract"
        if (SCHEDULED_OPERATION_PATTERN.search(parsed_output) is not None or
                TERMINAL_RESULT_PATTERN.search(parsed_output) is not None):
            return "FAIL", "invalid input unexpectedly admitted asynchronous work"
        return "PASS", "invalid input was rejected with its exact usage contract"

    name = command_name(command)
    canonical = canonical_command_name(command)
    failure_scan_output = parsed_output
    if canonical == "discover":
        # One supported strap address is normally absent. Discovery-specific
        # reconciliation below decides whether that count is acceptable.
        failure_scan_output = re.sub(
            r"\bfailed\s*=\s*\d+\b", "failed=0", failure_scan_output,
            flags=re.IGNORECASE,
        )
    if command_is_async(command):
        scheduled_matches = list(SCHEDULED_OPERATION_PATTERN.finditer(parsed_output))
        terminal_matches = list(TERMINAL_RESULT_PATTERN.finditer(parsed_output))
        if len(scheduled_matches) != 1 or len(terminal_matches) != 1:
            return "FAIL", "missing scheduled or terminal CLI session envelope"
        scheduled = scheduled_matches[0]
        terminal = terminal_matches[0]
        if scheduled.group(1).lower() != canonical or terminal.group(1).lower() != canonical:
            return "FAIL", "scheduled/result command does not match the requested command"
        if scheduled.group(2) != terminal.group(2):
            return "FAIL", "scheduled and terminal session IDs differ"
        if terminal.group(3).upper() != "SUCCESS" or int(terminal.group(4)) != 0:
            if expected_failure_match is not None:
                return (
                    "PASS",
                    "correlated failed operation matched configured "
                    f"expected-failure token: {expected_failure_match.pattern}",
                )
            return "FAIL", "terminal CLI session did not report SUCCESS/code=0"
    elif canonical in IMMEDIATE_RESULT_COMMANDS:
        result_envelopes = re.findall(
            rf"\bCLI result:\s*command={re.escape(canonical)}\b[^\r\n]*"
            r"\boutcome=([A-Z_]+)\b[^\r\n]*\bcode=(\d+)\b",
            parsed_output,
            re.IGNORECASE,
        )
        if len(result_envelopes) != 1:
            return "FAIL", "missing stable CLI result envelope"
        if result_envelopes[0][0].upper() != "SUCCESS" or int(result_envelopes[0][1]) != 0:
            if expected_failure_match is not None:
                return (
                    "PASS",
                    "failed immediate operation matched configured "
                    f"expected-failure token: {expected_failure_match.pattern}",
                )
            return "FAIL", "CLI result did not report SUCCESS/code=0"
    for pattern in FAIL_PATTERNS:
        if pattern.search(failure_scan_output):
            return "FAIL", f"matched failure pattern: {pattern.pattern}"

    sensor_condition_observed = (
        canonical in NO_SENSOR_CONDITION_COMMANDS and
        any(pattern.search(parsed_output) for pattern in SENSOR_CONDITION_PATTERNS)
    )
    sensor_condition_expected = (
        fixture == "no-sensor" and canonical in NO_SENSOR_CONDITION_COMMANDS
    )
    if sensor_condition_observed and not sensor_condition_expected:
        return "FAIL", "sensor-condition fault flag was asserted"

    required_evidence = COMMAND_EVIDENCE_PATTERNS.get(canonical, ())
    branch_evidence = COMMAND_BRANCH_EVIDENCE_PATTERNS.get(
        normalized_command(command), ()
    )
    required_evidence = (*required_evidence, *branch_evidence)
    if required_evidence:
        missing = [pattern.pattern for pattern in required_evidence
                   if pattern.search(parsed_output) is None]
        if missing:
            return "FAIL", "missing command-specific evidence: " + ", ".join(missing)
        semantic_failure = command_semantic_failure(command, parsed_output, profile)
        if semantic_failure is not None:
            return "FAIL", semantic_failure
        if sensor_condition_expected and sensor_condition_observed:
            return "PASS", "command evidence parsed; sensor-condition flags expected for no-sensor fixture"
        return "PASS", "all command-specific evidence parsed"

    for pattern in OK_PATTERNS:
        if pattern.search(parsed_output):
            return "PASS", "status output indicates OK"

    for pattern in expected_patterns or []:
        if pattern.search(parsed_output):
            return "PASS", f"matched configured expected token: {pattern.pattern}"

    return "UNKNOWN", "no explicit failure found, but no OK status was parsed"


def parse_int_token(value: object) -> Optional[int]:
    try:
        return int(str(value).strip(), 0)
    except (TypeError, ValueError):
        return None


def first_transcript_int(patterns: List[re.Pattern[str]], transcript: str) -> Optional[int]:
    for pattern in patterns:
        match = pattern.search(transcript)
        if match:
            return parse_int_token(match.group(1))
    return None


def append_expectation_results(
    args: argparse.Namespace,
    command_results: List[Dict[str, object]],
    transcript: str,
) -> None:
    if not transcript.strip():
        return

    expected_address = parse_int_token(args.address)
    actual_address = first_transcript_int(ADDRESS_PATTERNS, transcript)
    if expected_address is not None and actual_address is None:
        command_results.append(
            {
                "index": len(command_results) + 1,
                "command": "expect-address",
                "status": "FAIL",
                "reason": "configured I2C address was not reported by target firmware",
            }
        )
    elif expected_address is not None and actual_address != expected_address:
        command_results.append(
            {
                "index": len(command_results) + 1,
                "command": "expect-address",
                "status": "FAIL",
                "reason": (
                    f"transcript address 0x{actual_address:02X} "
                    f"does not match expected 0x{expected_address:02X}"
                ),
            }
        )

    expected_channel_count = int(args.channel_count)
    actual_channel_count = first_transcript_int(CHANNEL_COUNT_PATTERNS, transcript)
    if actual_channel_count is None:
        command_results.append(
            {
                "index": len(command_results) + 1,
                "command": "expect-channel-count",
                "status": "FAIL",
                "reason": "configured variant channel count was not reported by target firmware",
            }
        )
    elif actual_channel_count != expected_channel_count:
        command_results.append(
            {
                "index": len(command_results) + 1,
                "command": "expect-channel-count",
                "status": "FAIL",
                "reason": (
                    f"transcript channel count {actual_channel_count} "
                    f"does not match expected {expected_channel_count}"
                ),
            }
        )

    expected_commit = (args.expected_firmware_commit or
                       git_value(["rev-parse", "--short", "HEAD"])).lower()
    firmware_match = FIRMWARE_GIT_PATTERN.search(transcript)
    actual_commit = firmware_match.group(1).lower() if firmware_match else ""
    commit_matches = (actual_commit != "unknown" and expected_commit != "unknown" and
                      (actual_commit.startswith(expected_commit) or
                       expected_commit.startswith(actual_commit)))
    if not commit_matches:
        command_results.append(
            {
                "index": len(command_results) + 1,
                "command": "expect-firmware-commit",
                "status": "FAIL",
                "reason": (
                    "target firmware commit was not reported"
                    if not actual_commit
                    else f"target firmware commit {actual_commit} does not match {expected_commit}"
                ),
            }
        )

    firmware_status = FIRMWARE_STATUS_PATTERN.search(transcript)
    if firmware_status is None or firmware_status.group(1).lower() != "clean":
        command_results.append(
            {
                "index": len(command_results) + 1,
                "command": "expect-clean-firmware",
                "status": "FAIL",
                "reason": "target firmware did not report a clean source revision",
            }
        )

    runtime = RUNTIME_VERSION_PATTERN.search(transcript)
    if runtime is None:
        command_results.append(
            {
                "index": len(command_results) + 1,
                "command": "expect-runtime-version",
                "status": "FAIL",
                "reason": "target firmware did not report runtime stack provenance",
            }
        )
    else:
        platform, framework, framework_version, idf_version, target, backend, frequency = (
            runtime.group(index) for index in range(1, 8)
        )
        mismatches: List[str] = []
        if args.profile == "arduino":
            if platform.lower() != "pioarduino-55.03.311":
                mismatches.append(f"platform={platform}")
            if framework.lower() != "arduino":
                mismatches.append(f"framework={framework}")
            if framework_version.lower().lstrip("v") != "3.3.11":
                mismatches.append(f"framework_version={framework_version}")
            if idf_version.lower().lstrip("v") != "5.5.5":
                mismatches.append(f"idf_version={idf_version}")
        else:
            if platform.lower() != "esp-idf-native":
                mismatches.append(f"platform={platform}")
            if framework.lower() != "esp-idf":
                mismatches.append(f"framework={framework}")
            if not args.expected_idf_version:
                mismatches.append("expected_idf_version=missing")
            elif idf_version.lower().lstrip("v") != args.expected_idf_version.lower().lstrip("v"):
                mismatches.append(f"idf_version={idf_version}")
        if target.lower() != args.expected_target.lower():
            mismatches.append(f"target={target}")
        if backend.lower() != "esp-idf-new-master":
            mismatches.append(f"i2c_backend={backend}")
        frequency_hz = int(frequency)
        if frequency_hz < 10000 or frequency_hz > 400000:
            mismatches.append(f"frequency_hz={frequency_hz}")
        if mismatches:
            command_results.append(
                {
                    "index": len(command_results) + 1,
                    "command": "expect-runtime-version",
                    "status": "FAIL",
                    "reason": "runtime stack mismatch: " + ", ".join(mismatches),
                }
            )

    if args.port and not args.dry_run:
        for field in ("operator", "board"):
            if not str(getattr(args, field)).strip():
                command_results.append(
                    {
                        "index": len(command_results) + 1,
                        "command": f"expect-{field}",
                        "status": "FAIL",
                        "reason": f"--{field} is required for a real HIL run",
                    }
                )


def append_matrix_consistency_results(
    command_results: List[Dict[str, object]],
) -> None:
    """Add failures for relationships that span more than one command."""
    help_outputs = [
        strip_ansi(str(result.get("output", "")))
        for result in command_results
        if normalized_command(str(result.get("command", ""))) == "help"
        and str(result.get("status", "UNKNOWN")) == "PASS"
    ]
    if len(help_outputs) >= 2 and any(
            output != help_outputs[0] for output in help_outputs[1:]):
        command_results.append(
            {
                "index": len(command_results) + 1,
                "command": "expect-color-stable-help",
                "status": "FAIL",
                "reason": (
                    "ANSI-stripped help changed between color-enabled and "
                    "color-disabled output"
                ),
            }
        )

    normalized = [
        normalized_command(str(result.get("command", "")))
        for result in command_results
    ]
    assertion = "xfer assert 0 0 0 0"
    if assertion in normalized:
        assertion_index = len(normalized) - 1 - normalized[::-1].index(assertion)
        if assertion_index + 1 >= len(command_results) or normalized[assertion_index + 1] != "state":
            command_results.append(
                {
                    "index": len(command_results) + 1,
                    "command": "expect-invalid-input-state",
                    "status": "FAIL",
                    "reason": "invalid-input transfer fence was not followed by state evidence",
                }
            )
        else:
            state_result = command_results[assertion_index + 1]
            state_output = strip_ansi(str(state_result.get("output", "")))
            clean_state = re.search(
                r"\bstate bound=1\s+applied=APPLIED_ACTIVE\s+profile_dirty=0\s+"
                r"session_kind=NONE\s+active=0\s+pending_result=0\b",
                state_output,
                re.IGNORECASE,
            )
            if str(state_result.get("status", "UNKNOWN")) != "PASS" or clean_state is None:
                command_results.append(
                    {
                        "index": len(command_results) + 1,
                        "command": "expect-invalid-input-state",
                        "status": "FAIL",
                        "reason": (
                            "invalid-input matrix changed staged/applied state or "
                            "left work active"
                        ),
                    }
                )

def base_acceptance_failure(
    args: argparse.Namespace,
    commands: List[str],
    command_results: List[Dict[str, object]],
    transcript: str,
) -> Optional[str]:
    """Return why a long soak prerequisite failed, or None when safe to run."""
    if not commands:
        return "base command matrix is empty"
    if len(command_results) != len(commands):
        return "base command matrix did not produce one result per command"

    for result in command_results:
        if str(result.get("status", "UNKNOWN")) != "PASS":
            return (
                f"base command {result.get('command', '<unknown>')} "
                f"was {result.get('status', 'UNKNOWN')}"
            )

    for result in command_results:
        if has_firmware_startup_banner(str(result.get("output", ""))):
            return "unexpected firmware restart banner occurred during the base matrix"

    normalized = [normalized_command(command) for command in commands]
    if args.fixture == "no-sensor" and "busrecover confirm" in normalized:
        required_recovery = [
            "busrecover confirm", "state", "init", "wake", "probe", "drv",
        ]
        recovery_index = next(
            (
                index for index in range(len(normalized) - len(required_recovery) + 1)
                if normalized[index:index + len(required_recovery)] == required_recovery
            ),
            None,
        )
        if recovery_index is None:
            return "base matrix omitted controller reconstruction and full requalification"
        state_output = str(command_results[recovery_index + 1].get("output", ""))
        if re.search(
            r"\bstate bound=1\s+applied=UNKNOWN\s+profile_dirty=0\b",
            state_output,
            re.IGNORECASE,
        ) is None:
            return "controller recovery did not visibly invalidate applied device state"

    final_driver = command_results[-1] if command_results else None
    final_output = str(final_driver.get("output", "")) if final_driver else ""
    required_driver_state = (
        r"\bbound=1\b",
        r"\bapplied=APPLIED_ACTIVE\b",
        r"\bprofile_dirty=0\b",
        r"\bactive=0\b",
        r"\bresult_available=0\b",
    )
    if (final_driver is None or
            command_name(str(final_driver.get("command", ""))) != "drv" or
            any(re.search(pattern, final_output, re.IGNORECASE) is None
                for pattern in required_driver_state)):
        return "base matrix did not finish with verified APPLIED_ACTIVE state"

    expectation_preview = [dict(result) for result in command_results]
    append_expectation_results(args, expectation_preview, transcript)
    if len(expectation_preview) != len(command_results):
        first_failure = expectation_preview[len(command_results)]
        return str(first_failure.get("reason", "base transcript expectation failed"))
    return None


def has_firmware_startup_banner(text: str) -> bool:
    """Return whether target output contains a maintained firmware boot banner."""
    return FIRMWARE_STARTUP_BANNER_PATTERN.search(ANSI_PATTERN.sub("", text)) is not None


def repeat_commands(commands: List[str], repeat_count: int) -> List[str]:
    if repeat_count <= 1:
        return list(commands)
    repeated: List[str] = []
    for _ in range(repeat_count):
        repeated.extend(commands)
    return repeated


def strip_ansi(text: str) -> str:
    return ANSI_PATTERN.sub("", text)


def transcript_payload(transcript: str) -> str:
    lines = [
        line for line in transcript.splitlines()
        if not line.startswith("### ")
    ]
    return "\n".join(lines).strip()


def find_command_result(command_results: List[Dict[str, object]],
                        command: str) -> Optional[Dict[str, object]]:
    for result in command_results:
        if str(result.get("command", "")) == command:
            return result
    return None


def make_not_run_summary(reason: str) -> Dict[str, object]:
    return {
        "status": "NOT_RUN",
        "reason": reason,
        "elapsed_s": 0.0,
    }


def apply_serial_line_state(ser: object, name: str, value: str) -> None:
    if value == "unchanged":
        return
    setattr(ser, name, value == "on")


def summarize_stress(args: argparse.Namespace,
                     command_results: List[Dict[str, object]]) -> Dict[str, object]:
    summary: Dict[str, object] = {
        "requested": bool(args.include_stress),
        "count": int(args.stress_count),
        "status": "NOT_RUN",
        "reason": "stress was not requested",
        "success_count": None,
        "failure_count": None,
        "elapsed_s": 0.0,
        "effective_hz": None,
    }
    if not args.include_stress:
        return summary

    command = f"stress {args.stress_count} 0x01"
    result = find_command_result(command_results, command)
    if result is None:
        summary["reason"] = "requested stress command was not executed"
        return summary

    summary["elapsed_s"] = float(result.get("elapsed_s", 0.0))
    status = str(result.get("status", "UNKNOWN"))
    if status == "NOT_RUN":
        summary["reason"] = str(result.get("reason", "stress command was not run"))
        return summary

    output = strip_ansi(str(result.get("output", "")))
    detail_match = STRESS_DETAIL_PATTERN.search(output)
    legacy_match = STRESS_RESULT_PATTERN.search(output)
    if detail_match is None and legacy_match is None:
        summary["status"] = "FAIL" if status == "FAIL" else "UNKNOWN"
        summary["reason"] = (
            str(result.get("reason", "stress command failed"))
            if status == "FAIL"
            else "stress summary counts were not parsed"
        )
        return summary

    if detail_match is not None:
        requested_count = int(detail_match.group(1))
        ok_count = int(detail_match.group(2))
        fail_count = int(detail_match.group(3))
        elapsed_s = int(detail_match.group(4)) / 1000.0
        reported_hz = (
            float(detail_match.group(5))
            if detail_match.group(5) is not None else None
        )
        summary["elapsed_s"] = elapsed_s
    else:
        assert legacy_match is not None
        ok_count = int(legacy_match.group(1))
        fail_count = int(legacy_match.group(2))
        requested_count = ok_count + fail_count
        elapsed_s = float(summary["elapsed_s"])
        reported_hz = None
    summary["success_count"] = ok_count
    summary["failure_count"] = fail_count
    summary["effective_hz"] = (
        reported_hz if reported_hz is not None
        else ((ok_count / elapsed_s) if elapsed_s > 0.0 else None)
    )
    incomplete = requested_count != args.stress_count or ok_count + fail_count != requested_count
    summary["status"] = "FAIL" if fail_count > 0 or status == "FAIL" or incomplete else "PASS"
    summary["reason"] = (
        "stress summary count mismatch" if incomplete
        else ("stress command reported failures" if fail_count > 0 else "stress summary parsed")
    )
    return summary


def summarize_sample_rate(args: argparse.Namespace,
                          command_results: List[Dict[str, object]]) -> Dict[str, object]:
    summary: Dict[str, object] = {
        "requested": args.sample_rate_count != 0,
        "channel": int(args.sample_rate_channel),
        "requested_count": int(args.sample_rate_count),
        "status": "NOT_RUN",
        "reason": "sample-rate benchmark was not requested",
        "observed_count": None,
        "failure_count": None,
        "elapsed_s": 0.0,
        "effective_hz": None,
    }
    if args.sample_rate_count == 0:
        return summary
    command = f"samplerate {args.sample_rate_channel} {args.sample_rate_count}"
    result = find_command_result(command_results, command)
    if result is None:
        summary["reason"] = "requested samplerate command was not executed"
        return summary

    summary["elapsed_s"] = float(result.get("elapsed_s", 0.0))
    status = str(result.get("status", "UNKNOWN"))
    if status == "NOT_RUN":
        summary["reason"] = str(result.get("reason", "sample-rate command was not run"))
        return summary

    output = strip_ansi(str(result.get("output", "")))
    rate_matches = list(SAMPLE_RATE_RESULT_PATTERN.finditer(output))
    if len(rate_matches) == 1:
        rate_match = rate_matches[0]
        reported_count = int(rate_match.group(1))
        observed_count = int(rate_match.group(2))
        failure_count = int(rate_match.group(3))
        elapsed_s = int(rate_match.group(4)) / 1000.0
        summary["elapsed_s"] = elapsed_s
        summary["observed_count"] = observed_count
        summary["failure_count"] = failure_count
        summary["effective_hz"] = float(rate_match.group(5))
        incomplete = (
            reported_count != args.sample_rate_count
            or observed_count + failure_count != reported_count
        )
        summary["status"] = (
            "FAIL" if failure_count > 0 or status == "FAIL" or incomplete
            else "PASS"
        )
        summary["reason"] = (
            "sample-rate summary count mismatch" if incomplete
            else (
                "sample-rate command reported failures"
                if failure_count > 0 else "sample-rate summary parsed"
            )
        )
        return summary

    if len(rate_matches) > 1:
        summary["status"] = "FAIL"
        summary["reason"] = "multiple sample-rate summaries were observed"
        return summary

    observed_count = len(SAMPLE_LINE_PATTERN.findall(output))
    failure_count = len(SAMPLE_FAIL_PATTERN.findall(output))
    elapsed_s = float(summary["elapsed_s"])
    summary["observed_count"] = observed_count
    summary["failure_count"] = failure_count

    if status == "FAIL":
        summary["status"] = "FAIL"
        summary["reason"] = str(result.get("reason", "sample-rate command failed"))
        return summary
    if observed_count == args.sample_rate_count and failure_count == 0 and elapsed_s > 0.0:
        summary["status"] = "PASS"
        summary["reason"] = "all requested samples were observed"
        summary["effective_hz"] = observed_count / elapsed_s
        return summary

    summary["status"] = "UNKNOWN"
    summary["reason"] = "sample-rate summary was incomplete"
    return summary


def summarize_soak(
    args: argparse.Namespace,
    observed: Optional[Dict[str, object]] = None,
) -> Dict[str, object]:
    if not args.include_long_soak:
        return make_not_run_summary("long soak was not requested")
    if args.fixture != "no-sensor" or args.profile != "arduino":
        return {
            "status": "NOT_RUN",
            "reason": "automatic soak is defined only for the Arduino no-sensor fixture",
            "requested_duration_s": args.soak_duration_s,
            "elapsed_s": 0.0,
        }
    if args.soak_duration_s <= 0.0:
        return {
            "status": "NOT_RUN",
            "reason": "--soak-duration-s must be supplied to run the bounded soak",
            "requested_duration_s": 0.0,
            "elapsed_s": 0.0,
        }
    if observed is not None:
        return observed
    return {
        "status": "NOT_RUN",
        "reason": "serial soak did not start",
        "requested_duration_s": args.soak_duration_s,
        "elapsed_s": 0.0,
        "command_counts": {},
        "failure_count": None,
        "unknown_count": None,
        "reset_count": None,
        "worst_latency_s": None,
    }


def soak_outcome(
    cycle_count: int,
    failure_count: int,
    unknown_count: int,
    reset_count: int,
) -> Tuple[str, str]:
    if cycle_count == 0:
        return "FAIL", "no complete soak cycle executed"
    if failure_count > 0:
        return "FAIL", "one or more soak commands failed"
    if reset_count > 0:
        return "FAIL", "unexpected firmware reset/banner observed during soak"
    if unknown_count > 0:
        return "UNKNOWN", "one or more soak commands had ambiguous output"
    return "PASS", "requested duration completed with all commands passing"


def enforce_soak_invariant(
    command: str,
    result: Dict[str, object],
) -> Dict[str, object]:
    if result.get("status") != "PASS":
        return result
    output = str(result.get("output", ""))
    if command_name(command) == "state":
        if re.search(
            r"\bstate bound=1\s+applied=UNKNOWN\s+profile_dirty=0\b",
            output,
            re.IGNORECASE,
        ) is not None:
            return result
        failed = dict(result)
        failed["status"] = "FAIL"
        failed["reason"] = "soak recovery did not invalidate applied device state"
        return failed
    if command_name(command) != "drv":
        return result
    if re.search(
        r"\bbound=1\b.*\bapplied=APPLIED_ACTIVE\b",
        output,
        re.IGNORECASE | re.DOTALL,
    ) is not None:
        return result
    failed = dict(result)
    failed["status"] = "FAIL"
    failed["reason"] = "soak cycle did not end bound with applied state APPLIED_ACTIVE"
    return failed


def read_available(ser, deadline: float, idle_gap_s: float, prompt_patterns: Iterable[str]) -> Tuple[str, bool]:
    chunks: List[str] = []
    last_rx = time.monotonic()
    timed_out = False
    prompt_res = [re.compile(pattern) for pattern in prompt_patterns]

    while True:
        now = time.monotonic()
        if now >= deadline:
            timed_out = True
            break

        waiting = getattr(ser, "in_waiting", 0)
        if waiting:
            raw = ser.read(waiting)
            text = raw.decode("utf-8", errors="replace")
            chunks.append(text)
            last_rx = now
            joined = "".join(chunks)
            if any(pattern.search(joined) for pattern in prompt_res):
                break
            continue

        if chunks and (now - last_rx) >= idle_gap_s:
            break
        time.sleep(0.02)

    return "".join(chunks), timed_out


def not_run_command_results(commands: List[str], reason: str) -> List[Dict[str, object]]:
    return [
        {
            "index": index,
            "command": command,
            "status": "NOT_RUN",
            "reason": reason,
            "timed_out": False,
            "elapsed_s": 0.0,
            "output": "",
        }
        for index, command in enumerate(commands, start=1)
    ]


def run_serial_commands(
    args: argparse.Namespace,
    commands: List[str],
) -> Tuple[List[Dict[str, object]], str, str, float, Optional[Dict[str, object]]]:
    journal = TranscriptJournal(args.raw_transcript_out)
    try:
        journal.open()
    except Exception as exc:
        raise SerialRunFailure(
            f"could not open raw transcript journal: {exc}", [], "", 0.0,
            None, type(exc).__name__, False,
        ) from exc
    try:
        import serial  # type: ignore[import-not-found]
    except Exception as exc:
        message = f"pyserial is required for serial HIL runs: {exc}"
        journal.append(f"### runner exception before serial open\n{type(exc).__name__}: {message}\n")
        journal.close()
        raise SerialRunFailure(
            message, [], journal.text(), 0.0, None, type(exc).__name__, False,
        ) from exc

    prompt_patterns = [r">\s*$", r"ldc1614-idf>\s*$"]
    expected_patterns = compile_token_patterns(args.expect_token)
    failure_patterns = compile_token_patterns(args.failure_token)
    expected_failure_patterns = compile_token_patterns(args.expected_failure_token)
    scoped_expected_failures = compile_scoped_expected_failures(
        args.expected_failure
    )
    transcript_parts = journal.parts
    results: List[Dict[str, object]] = []
    startup_elapsed_s = 0.0
    soak_summary: Optional[Dict[str, object]] = None
    capture = journal.append
    stack = ExitStack()
    serial_opened = False
    run_exception: Optional[BaseException] = None
    try:
        ser = stack.enter_context(serial.Serial(
            args.port,
            args.baud,
            timeout=0.05,
            write_timeout=args.write_timeout_s,
            rtscts=False,
            dsrdtr=False,
        ))
        serial_opened = True
        apply_serial_line_state(ser, "dtr", args.serial_dtr)
        apply_serial_line_state(ser, "rts", args.serial_rts)
        time.sleep(args.startup_delay_s)
        startup_start = time.monotonic()
        startup, _ = read_available(
            ser,
            time.monotonic() + args.command_timeout_s,
            args.idle_gap_s,
            prompt_patterns,
        )
        startup_elapsed_s = time.monotonic() - startup_start
        capture("### startup\n" + startup)

        def execute(command: str) -> Dict[str, object]:
            command_start = time.monotonic()
            command_deadline = command_start + args.command_timeout_s
            ser.write((command + "\n").encode("utf-8"))
            ser.flush()
            output, timed_out = read_available(
                ser,
                command_deadline,
                args.idle_gap_s,
                prompt_patterns,
            )
            name = command_name(command)
            scheduled = SCHEDULED_OPERATION_PATTERN.search(output)
            if (command_is_async(command) and scheduled is not None and
                    PROMPT_BOUNDARY_PATTERN.search(strip_ansi(output)) is None):
                completion, completion_timed_out = read_available(
                    ser,
                    command_deadline,
                    args.idle_gap_s,
                    prompt_patterns,
                )
                output += completion
                timed_out = timed_out or completion_timed_out
            elapsed_s = time.monotonic() - command_start
            status, reason = classify_command(
                command,
                output,
                timed_out,
                expected_patterns,
                failure_patterns,
                expected_failure_patterns + scoped_expected_failures.get(
                    normalized_command(command), []
                ),
                args.fixture,
                args.profile,
            )
            return {
                "command": command,
                "status": status,
                "reason": reason,
                "timed_out": timed_out,
                "elapsed_s": elapsed_s,
                "output": output,
            }

        for index, command in enumerate(commands, start=1):
            try:
                result = execute(command)
            except Exception as exc:
                capture(
                    f"### runner exception during command {index}: {command}\n"
                    f"{type(exc).__name__}: {exc}\n"
                )
                raise SerialRunFailure(
                    str(exc), list(results), "\n".join(transcript_parts),
                    startup_elapsed_s, soak_summary, type(exc).__name__,
                    serial_opened,
                ) from exc
            result["index"] = index
            capture(
                f"### command {index}: {command}\n{result['output']}"
            )
            if args.verbose:
                print(
                    f"[{index}/{len(commands)}] {command}: {result['status']} "
                    f"({float(result['elapsed_s']):.3f}s)"
                )
            results.append(result)
            if result["status"] != "PASS":
                reason = (
                    f"not sent after base command {index} ({command}) "
                    f"was {result['status']}"
                )
                capture(
                    f"### runner fail-fast after command {index}: {command}\n"
                    f"{reason}\n"
                )
                for remaining_index, remaining_command in enumerate(
                    commands[index:], start=index + 1
                ):
                    results.append(
                        {
                            "index": remaining_index,
                            "command": remaining_command,
                            "status": "NOT_RUN",
                            "reason": reason,
                            "timed_out": False,
                            "elapsed_s": 0.0,
                            "output": "",
                        }
                    )
                break

        soak_requested = (
            args.include_long_soak and args.soak_duration_s > 0.0 and
            args.fixture == "no-sensor" and args.profile == "arduino"
        )
        gate_failure = base_acceptance_failure(
            args, commands, results, "\n".join(transcript_parts)
        ) if soak_requested else None
        if soak_requested and gate_failure is not None:
            soak_summary = {
                "status": "FAIL",
                "reason": f"soak not started: {gate_failure}",
                "started": False,
                "requested_duration_s": args.soak_duration_s,
                "elapsed_s": 0.0,
                "cycle_count": 0,
                "incomplete_cycle": None,
                "command_count": 0,
                "command_counts": {},
                "failure_count": 0,
                "unknown_count": 0,
                "reset_count": 0,
                "worst_latency_s": 0.0,
                "non_pass_details": [],
            }
        elif soak_requested:
            soak_start = time.monotonic()
            soak_deadline = soak_start + args.soak_duration_s
            cycle_count = 0
            command_count = 0
            failure_count = 0
            unknown_count = 0
            reset_count = 0
            worst_latency_s = 0.0
            command_counts: Dict[str, Dict[str, int]] = {
                command: {"PASS": 0, "FAIL": 0, "UNKNOWN": 0}
                for command in NO_SENSOR_SOAK_COMMANDS
            }
            non_pass_details: List[Dict[str, object]] = []

            while cycle_count == 0 or time.monotonic() < soak_deadline:
                current_cycle = cycle_count + 1
                for command in NO_SENSOR_SOAK_COMMANDS:
                    command_count += 1
                    try:
                        result = enforce_soak_invariant(command, execute(command))
                    except Exception as exc:
                        elapsed_s = time.monotonic() - soak_start
                        soak_summary = {
                            "status": "FAIL",
                            "reason": f"runner exception during soak: {exc}",
                            "started": True,
                            "requested_duration_s": args.soak_duration_s,
                            "elapsed_s": elapsed_s,
                            "cycle_count": cycle_count,
                            "incomplete_cycle": current_cycle,
                            "command_count": command_count - 1,
                            "command_counts": command_counts,
                            "failure_count": failure_count,
                            "unknown_count": unknown_count,
                            "reset_count": reset_count,
                            "worst_latency_s": worst_latency_s,
                            "non_pass_details": non_pass_details,
                        }
                        capture(
                            f"### runner exception during soak cycle "
                            f"{current_cycle}: {command}\n"
                            f"{type(exc).__name__}: {exc}\n"
                        )
                        raise SerialRunFailure(
                            str(exc), list(results),
                            "\n".join(transcript_parts), startup_elapsed_s,
                            soak_summary, type(exc).__name__, serial_opened,
                        ) from exc
                    status = str(result["status"])
                    command_counts[command][status] += 1
                    latency_s = float(result["elapsed_s"])
                    worst_latency_s = max(worst_latency_s, latency_s)
                    output = str(result["output"])
                    if has_firmware_startup_banner(output):
                        reset_count += 1
                    if status == "FAIL":
                        failure_count += 1
                    elif status == "UNKNOWN":
                        unknown_count += 1
                    if status != "PASS" and len(non_pass_details) < 20:
                        non_pass_details.append(
                            {
                                "cycle": current_cycle,
                                "command": command,
                                "status": status,
                                "reason": result["reason"],
                                "output": output,
                            }
                        )
                    capture(
                        f"### soak cycle {current_cycle} command {command_count}: "
                        f"{command}\n{output}"
                    )

                cycle_count = current_cycle

                if args.verbose:
                    elapsed_s = time.monotonic() - soak_start
                    print(
                        f"[soak cycle {cycle_count}] elapsed={elapsed_s:.1f}s "
                        f"fail={failure_count} unknown={unknown_count}"
                    )
                remaining_s = soak_deadline - time.monotonic()
                if remaining_s > 0.0 and args.soak_cycle_delay_s > 0.0:
                    time.sleep(min(args.soak_cycle_delay_s, remaining_s))

            elapsed_s = time.monotonic() - soak_start
            soak_status, soak_reason = soak_outcome(
                cycle_count, failure_count, unknown_count, reset_count
            )
            soak_summary = {
                "status": soak_status,
                "reason": soak_reason,
                "started": True,
                "requested_duration_s": args.soak_duration_s,
                "elapsed_s": elapsed_s,
                "cycle_count": cycle_count,
                "incomplete_cycle": None,
                "command_count": command_count,
                "command_counts": command_counts,
                "failure_count": failure_count,
                "unknown_count": unknown_count,
                "reset_count": reset_count,
                "worst_latency_s": worst_latency_s,
                "non_pass_details": non_pass_details,
            }

    except BaseException as exc:
        run_exception = exc

    try:
        stack.close()
    except BaseException as close_exc:
        if run_exception is None:
            run_exception = close_exc
        else:
            run_exception = RuntimeError(
                f"{type(run_exception).__name__}: {run_exception}; "
                f"serial close failed: {type(close_exc).__name__}: {close_exc}"
            )

    if run_exception is not None:
        if isinstance(run_exception, SerialRunFailure):
            journal.close()
            raise run_exception
        capture(
            "### runner exception during serial run/close\n"
            f"{type(run_exception).__name__}: {run_exception}\n"
        )
        failure = SerialRunFailure(
            str(run_exception), list(results), journal.text(), startup_elapsed_s,
            soak_summary, type(run_exception).__name__, serial_opened,
        )
        journal.close()
        raise failure from run_exception

    journal.close()

    full_transcript = "\n".join(transcript_parts)
    firmware_version = firmware_version_from_transcript(full_transcript)
    return results, full_transcript, firmware_version, startup_elapsed_s, soak_summary


def add_optional_commands(args: argparse.Namespace, commands: List[str], skipped: List[Dict[str, str]]) -> None:
    if args.include_config_matrix:
        commands.extend(configuration_matrix_commands(args.channel_count))

    if args.include_invalid_inputs:
        commands.append("xfer reset")
        commands.extend(INVALID_INPUT_COMMANDS)
        commands.extend(("xfer assert 0 0 0 0", "state"))

    if parse_int_token(args.address) != 0x2B:
        skipped.append(
            {
                "name": "address_0x2B",
                "reason": (
                    "NOT_RUN: requested, but address is a build/fixture fact; "
                    "rebuild and rewire for 0x2B"
                    if args.include_address_0x2b else
                    "NOT_RUN: requires a separately rebuilt and rewired 0x2B fixture"
                ),
            }
        )
    if args.channel_count != 2:
        skipped.append(
            {
                "name": "variant_LDC1612",
                "reason": (
                    "NOT_RUN: requires a separately rebuilt LDC1612 fixture "
                    "with its complete two-channel profile"
                ),
            }
        )

    if args.include_stress:
        commands.append("wake")
        commands.extend(optional_group_commands("stress", count=args.stress_count))
        commands.append("drv")

    if args.include_reset_stress:
        commands.append("wake")
        commands.extend(optional_group_commands(
            "reset_stress", count=args.stress_count
        ))
        commands.append("drv")

    if args.include_busfreq_stress:
        commands.append("wake")
        commands.extend(optional_group_commands(
            "busfreq_stress", count=args.stress_count
        ))
        commands.append("drv")

    if args.sample_rate_count != 0:
        if args.fixture == "no-sensor":
            skipped.append(
                {
                    "name": "sample_rate_benchmark",
                    "reason": "no-sensor fixture excludes DRDY-gated sample-rate checks",
                }
            )
        elif args.sample_rate_count < 0 or args.sample_rate_count > MAX_SAMPLE_RATE_COUNT:
            skipped.append(
                {
                    "name": "sample_rate_benchmark",
                    "reason": f"--sample-rate-count must be 1..{MAX_SAMPLE_RATE_COUNT}",
                }
            )
        elif args.sample_rate_channel < 0 or args.sample_rate_channel > 3:
            skipped.append(
                {
                    "name": "sample_rate_benchmark",
                    "reason": "--sample-rate-channel must be 0..3",
                }
            )
        else:
            commands.append("wake")
            commands.extend(optional_group_commands(
                "sample_rate", channel=args.sample_rate_channel,
                count=args.sample_rate_count,
            ))
            commands.append("drv")

    if args.include_sd:
        commands.extend((
            "sd status", "sd assert confirm", "sd status",
            "sd release confirm", "sd status", "init", "wake", "drv",
        ))
    if args.include_intb:
        commands.extend(optional_group_commands("intb"))
    if args.include_drive_tuning:
        commands.extend(optional_group_commands("drive_tuning"))

    for name, enabled, requirement in (
        ("sd_shutdown_wake", args.include_sd,
         "wired SD control and power-state observation"),
        ("intb_observation", args.include_intb,
         "wired INTB and independent pin observation"),
        ("drive_current_tuning", args.include_drive_tuning,
         "an approved coil and drive-current tuning fixture"),
    ):
        if not enabled:
            skipped.append(
                {"name": name, "reason": f"NOT_RUN: requires {requirement}"}
            )

    for name, enabled, requirement in (
        ("unplug_replug", args.include_unplug, "operator-controlled unplug/replug fixture"),
        ("stuck_bus", args.include_stuck_bus, "operator-controlled stuck-bus fault fixture"),
    ):
        skipped.append(
            {
                "name": name,
                "reason": (
                    f"NOT_RUN: requested, but {requirement} is required"
                    if enabled else f"NOT_RUN: requires {requirement}"
                ),
            }
        )

    if args.fixture == "no-sensor":
        no_sensor_gates = [
            {
                "name": "sensor_measurement_quality",
                "reason": "NOT_RUN: requires a characterized resonant sensor fixture",
            },
            {
                "name": "sample_rate_benchmark",
                "reason": "NOT_RUN: requires DRDY and valid conversions from a sensor fixture",
            },
            {
                "name": "cached_last_sample",
                "reason": "NOT_RUN: last is SENSOR_REQUIRED; no-sensor acquisition is only fault-path evidence",
            },
        ]
        existing_gate_names = {item["name"] for item in skipped}
        skipped.extend(
            item for item in no_sensor_gates
            if item["name"] not in existing_gate_names
        )
    skipped.append(
        {
            "name": "active_job_cancellation",
            "reason": (
                "NOT_RUN: the unattended matrix proves idle cancel plus scheduled/terminal job correlation; "
                "active cancellation requires an interactive timing fixture"
            ),
        }
    )

    if args.include_long_soak:
        if (args.soak_duration_s <= 0.0 or args.fixture != "no-sensor" or
                args.profile != "arduino"):
            skipped.append(
                {
                    "name": "long_soak",
                    "reason": (
                        "automatic soak requires the Arduino no-sensor fixture "
                        "and an explicit positive --soak-duration-s"
                    ),
                }
            )
        else:
            commands.extend(("wake", "drv"))


def overall_status(command_results: List[Dict[str, object]],
                   not_run_reason: Optional[str],
                   transcript: str = "") -> str:
    if not_run_reason:
        return "NOT_RUN"
    if not command_results:
        return "NOT_RUN"
    if not transcript_payload(transcript):
        return "NOT_RUN"
    if any(result["status"] == "FAIL" for result in command_results):
        return "FAIL"
    if any(result["status"] in ("UNKNOWN", "REVIEW") for result in command_results):
        return "UNKNOWN"
    return "PASS"


def combine_overall_and_soak(base_status: str, soak_status: str) -> str:
    if base_status == "FAIL" or soak_status == "FAIL":
        return "FAIL"
    if base_status == "UNKNOWN" or soak_status == "UNKNOWN":
        return "UNKNOWN"
    return base_status


def combine_requested_gate_status(
    base_status: str, gate_status: str, requested: bool,
) -> str:
    """Fold an explicitly requested optional gate into the run verdict."""
    if not requested:
        return base_status
    if base_status == "FAIL" or gate_status == "FAIL":
        return "FAIL"
    if base_status == "NOT_RUN":
        return "NOT_RUN"
    if base_status == "UNKNOWN" or gate_status in ("UNKNOWN", "NOT_RUN"):
        return "UNKNOWN"
    return base_status


def make_result(args: argparse.Namespace) -> Dict[str, object]:
    skipped: List[Dict[str, str]] = []
    commands = [] if args.skip_default_commands else default_commands(args.profile, args.fixture)
    commands.extend(args.command)
    add_optional_commands(args, commands, skipped)
    base_command_count = len(commands)
    commands = repeat_commands(commands, args.repeat_command_set)

    not_run_reason: Optional[str] = None
    command_results: List[Dict[str, object]] = []
    transcript = ""
    firmware_version = "unknown"
    startup_elapsed_s = 0.0
    observed_soak: Optional[Dict[str, object]] = None
    serial_failure: Optional[Dict[str, str]] = None
    serial_opened_evidence = False

    if args.dry_run:
        not_run_reason = "dry-run requested; no serial commands were sent"
        command_results = not_run_command_results(commands, not_run_reason)
    elif not args.port:
        not_run_reason = "serial port was not supplied"
    else:
        try:
            (command_results, transcript, firmware_version, startup_elapsed_s,
             observed_soak) = run_serial_commands(args, commands)
            serial_opened_evidence = True
        except SerialRunFailure as exc:
            command_results = exc.results
            transcript = exc.transcript
            startup_elapsed_s = exc.startup_elapsed_s
            observed_soak = exc.soak
            serial_opened_evidence = exc.serial_opened
            serial_failure = {
                "type": exc.exception_type,
                "message": str(exc),
            }
            firmware_version = firmware_version_from_transcript(transcript)
            command_results.append(
                {
                    "index": len(command_results) + 1,
                    "command": "runner-exception",
                    "status": "FAIL",
                    "reason": f"serial run aborted: {type(exc).__name__}: {exc}",
                    "timed_out": False,
                    "elapsed_s": 0.0,
                    "output": "",
                }
            )
        except Exception as exc:
            serial_failure = {"type": type(exc).__name__, "message": str(exc)}
            transcript = (
                "### runner exception outside serial capture\n"
                f"{type(exc).__name__}: {exc}\n"
            )
            command_results.append(
                {
                    "index": 1,
                    "command": "runner-exception",
                    "status": "FAIL",
                    "reason": f"serial run aborted: {type(exc).__name__}: {exc}",
                    "timed_out": False,
                    "elapsed_s": 0.0,
                    "output": "",
                }
            )

        if serial_failure is None and transcript_payload(transcript):
            append_matrix_consistency_results(command_results)
            append_expectation_results(args, command_results, transcript)

    if (args.port and not args.dry_run and not commands and not not_run_reason and
            not transcript_payload(transcript)):
        not_run_reason = "no serial startup transcript payload captured"

    has_transcript = (
        serial_opened_evidence and bool(transcript_payload(transcript))
        and not args.dry_run and not not_run_reason
    )
    if serial_failure is not None:
        evidence_type = "serial_failure"
    elif has_transcript:
        evidence_type = "hardware_hil"
    elif args.dry_run or not args.port:
        evidence_type = "no_hardware_audit"
    else:
        evidence_type = "serial_not_run"

    host_git_commit = git_value(["rev-parse", "--short", "HEAD"])
    target_commit_match = FIRMWARE_GIT_PATTERN.search(transcript)
    target_status_match = FIRMWARE_STATUS_PATTERN.search(transcript)
    result: Dict[str, object] = {
        "tool": "ldc1614_hil_runner",
        "timestamp_utc": timestamp_utc(),
        "host_git_commit": host_git_commit,
        "host_git_status": "dirty" if git_value(["status", "--porcelain"], "") else "clean",
        "library_version": load_library_version(),
        "firmware_version": firmware_version,
        "firmware_git_commit": (target_commit_match.group(1).lower()
                                if target_commit_match else "unknown"),
        "firmware_git_status": (target_status_match.group(1).lower()
                                if target_status_match else "unknown"),
        "expected_firmware_commit": args.expected_firmware_commit or host_git_commit,
        "profile": args.profile,
        "fixture": args.fixture,
        "port": args.port or "",
        "baud": args.baud,
        "expected_address": args.address,
        "expected_channel_count": args.channel_count,
        "expected_target": args.expected_target,
        "expected_idf_version": (
            "5.5.5" if args.profile == "arduino" else args.expected_idf_version
        ),
        "operator": args.operator,
        "board": args.board,
        "notes": args.note,
        "dry_run": args.dry_run,
        "serial_port_requested": bool(args.port),
        "serial_dtr": args.serial_dtr,
        "serial_rts": args.serial_rts,
        "hardware_attached": has_transcript,
        "evidence_type": evidence_type,
        "startup_delay_s": args.startup_delay_s,
        "startup_elapsed_s": startup_elapsed_s,
        "command_timeout_s": args.command_timeout_s,
        "write_timeout_s": args.write_timeout_s,
        "idle_gap_s": args.idle_gap_s,
        "repeat_command_set": args.repeat_command_set,
        "base_command_count": base_command_count,
        "base_matrix_scope": (
            "custom_reduced" if args.skip_default_commands else "default"
        ),
        "default_matrix_included": not args.skip_default_commands,
        "expect_tokens": args.expect_token,
        "failure_tokens": args.failure_token,
        "expected_failure_tokens": args.expected_failure_token,
        "scoped_expected_failures": args.expected_failure,
        "sample_rate_count": args.sample_rate_count,
        "sample_rate_channel": args.sample_rate_channel,
        "soak_duration_s": args.soak_duration_s,
        "soak_cycle_delay_s": args.soak_cycle_delay_s,
        "commands": commands,
        "command_results": command_results,
        "skipped_optional_tests": skipped,
        "not_run_reason": not_run_reason,
        "serial_failure": serial_failure,
        "transcript": transcript,
    }
    result["stress"] = summarize_stress(args, command_results)
    result["sample_rate"] = summarize_sample_rate(args, command_results)
    result["soak"] = summarize_soak(args, observed_soak)
    result["soak"]["base_matrix_scope"] = result["base_matrix_scope"]
    result["soak"]["default_matrix_included"] = result[
        "default_matrix_included"
    ]
    result["overall_status"] = overall_status(command_results, not_run_reason, transcript)
    if serial_failure is not None:
        result["overall_status"] = "FAIL"
    soak_status = str(result["soak"].get("status", "NOT_RUN"))
    result["overall_status"] = combine_overall_and_soak(
        str(result["overall_status"]), soak_status
    )
    result["overall_status"] = combine_requested_gate_status(
        str(result["overall_status"]), soak_status,
        bool(args.include_long_soak),
    )
    result["overall_status"] = combine_requested_gate_status(
        str(result["overall_status"]),
        str(result["stress"].get("status", "NOT_RUN")),
        bool(args.include_stress),
    )
    result["overall_status"] = combine_requested_gate_status(
        str(result["overall_status"]),
        str(result["sample_rate"].get("status", "NOT_RUN")),
        args.sample_rate_count != 0,
    )
    requested_manual_gate = any((
        args.include_address_0x2b and parse_int_token(args.address) != 0x2B,
        args.include_unplug,
        args.include_stuck_bus,
    ))
    result["overall_status"] = combine_requested_gate_status(
        str(result["overall_status"]), "NOT_RUN", requested_manual_gate
    )
    return result


def render_markdown(result: Dict[str, object]) -> str:
    lines = [
        "# LDC1614 HIL Run",
        "",
        f"Overall status: `{result['overall_status']}`",
        f"Timestamp UTC: `{result['timestamp_utc']}`",
        f"Host checkout Git commit: `{result['host_git_commit']}`",
        f"Host checkout Git status: `{result['host_git_status']}`",
        f"Library version: `{result['library_version']}`",
        f"Firmware version: `{result['firmware_version']}`",
        f"Firmware-reported Git commit: `{result['firmware_git_commit']}`",
        f"Firmware-reported Git status: `{result['firmware_git_status']}`",
        f"Expected firmware Git commit: `{result['expected_firmware_commit']}`",
        f"Profile: `{result['profile']}`",
        f"Fixture: `{result.get('fixture', 'default')}`",
        f"Port: `{result['port']}`",
        f"Baud: `{result['baud']}`",
        f"Expected address: `{result['expected_address']}`",
        f"Expected channel count: `{result['expected_channel_count']}`",
        f"Operator: `{result['operator']}`",
        f"Board: `{result['board']}`",
        f"Dry run: `{result['dry_run']}`",
        f"Serial port requested: `{result['serial_port_requested']}`",
        f"Serial DTR/RTS: `{result.get('serial_dtr', 'on')}` / `{result.get('serial_rts', 'off')}`",
        f"Hardware attached: `{result['hardware_attached']}`",
        f"Evidence type: `{result['evidence_type']}`",
        f"Startup delay: `{result['startup_delay_s']}` s",
        f"Startup read elapsed: `{float(result['startup_elapsed_s']):.3f}` s",
        f"Command timeout: `{result['command_timeout_s']}` s",
        f"Idle gap: `{result['idle_gap_s']}` s",
        f"Repeat command set: `{result.get('repeat_command_set', 1)}`",
        f"Base command count: `{result.get('base_command_count', len(result.get('commands', [])))}`",
        f"Base matrix scope: `{result.get('base_matrix_scope', 'default')}`",
        f"Default matrix included: `{result.get('default_matrix_included', True)}`",
    ]
    if result.get("not_run_reason"):
        lines.append(f"Not-run reason: `{result['not_run_reason']}`")
    if result.get("serial_failure"):
        failure = result["serial_failure"]
        lines.append(
            f"Serial failure: `{failure.get('type', 'Exception')}: "
            f"{failure.get('message', '')}`"
        )
    if result.get("notes"):
        lines.append(f"Notes: {result['notes']}")

    lines.extend(["", "## Commands", "", "| # | Command | Status | Elapsed s | Reason |", "| ---: | --- | --- | ---: | --- |"])
    for item in result["command_results"]:
        lines.append(
            f"| {item['index']} | `{item['command']}` | `{item['status']}` | "
            f"{float(item.get('elapsed_s', 0.0)):.3f} | {item['reason']} |"
        )
    if not result["command_results"]:
        lines.append("| - | - | `NOT_RUN` | 0.000 | No serial command transcript captured |")

    lines.extend(["", "## Skipped Optional Tests", "", "| Test | Reason |", "| --- | --- |"])
    skipped = result["skipped_optional_tests"]
    if skipped:
        for item in skipped:
            lines.append(f"| `{item['name']}` | {item['reason']} |")
    else:
        lines.append("| - | No optional tests requested |")

    sample = result.get("sample_rate", {})
    lines.extend([
        "",
        "## Sample Rate",
        "",
        f"Status: `{sample.get('status', 'NOT_RUN')}`",
        f"Reason: {sample.get('reason', '')}",
        f"Channel: `{sample.get('channel', '')}`",
        f"Requested count: `{sample.get('requested_count', '')}`",
        f"Observed count: `{sample.get('observed_count', '')}`",
        f"Failures: `{sample.get('failure_count', '')}`",
        f"Elapsed s: `{float(sample.get('elapsed_s', 0.0)):.3f}`",
        f"Effective Hz: `{sample.get('effective_hz', None)}`",
    ])

    stress = result.get("stress", {})
    lines.extend([
        "",
        "## Stress",
        "",
        f"Status: `{stress.get('status', 'NOT_RUN')}`",
        f"Reason: {stress.get('reason', '')}",
        f"Requested count: `{stress.get('count', '')}`",
        f"Success count: `{stress.get('success_count', '')}`",
        f"Failure count: `{stress.get('failure_count', '')}`",
        f"Elapsed s: `{float(stress.get('elapsed_s', 0.0)):.3f}`",
        f"Effective Hz: `{stress.get('effective_hz', None)}`",
    ])

    soak = result.get("soak", {})
    lines.extend([
        "",
        "## Soak",
        "",
        f"Status: `{soak.get('status', 'NOT_RUN')}`",
        f"Reason: {soak.get('reason', '')}",
        f"Base matrix scope: `{soak.get('base_matrix_scope', 'default')}`",
        f"Default matrix included: `{soak.get('default_matrix_included', True)}`",
        f"Requested duration s: `{soak.get('requested_duration_s', 0)}`",
        f"Elapsed s: `{float(soak.get('elapsed_s', 0.0)):.3f}`",
        f"Cycles: `{soak.get('cycle_count', None)}`",
        f"Commands: `{soak.get('command_count', None)}`",
        f"Failure count: `{soak.get('failure_count', None)}`",
        f"Unknown count: `{soak.get('unknown_count', None)}`",
        f"Reset count: `{soak.get('reset_count', None)}`",
        f"Worst latency s: `{soak.get('worst_latency_s', None)}`",
    ])

    transcript = "\n".join(
        line.rstrip() for line in str(result.get("transcript", "")).splitlines()
    )
    lines.extend(["", "## Transcript", "", "```text", transcript, "```", ""])
    return "\n".join(lines)


def write_outputs(args: argparse.Namespace, result: Dict[str, object]) -> None:
    json_text = json.dumps(result, indent=2, sort_keys=True) + "\n"
    markdown_text = render_markdown(result)

    if args.json_out:
        Path(args.json_out).write_text(json_text, encoding="utf-8", newline="\n")
    if args.markdown_out:
        Path(args.markdown_out).write_text(markdown_text, encoding="utf-8", newline="\n")
    if args.raw_transcript_out:
        Path(args.raw_transcript_out).write_text(
            str(result.get("transcript", "")), encoding="utf-8", newline="\n"
        )

    if not args.quiet:
        print(markdown_text)


def parse_args(argv: List[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--profile", choices=("arduino", "idf"), default="arduino")
    parser.add_argument(
        "--fixture",
        choices=("default", "no-sensor"),
        default="default",
        help="Fixture-specific default command matrix",
    )
    parser.add_argument("--port", default="", help="Serial port. Omit to produce NOT_RUN.")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--address", default="0x2A")
    parser.add_argument("--channel-count", type=int, default=4)
    parser.add_argument(
        "--expected-target", choices=("esp32s2", "esp32s3"), default="esp32s2",
        help="Exact firmware-reported build target required for HIL acceptance",
    )
    parser.add_argument(
        "--expected-idf-version", default="",
        help="Exact native ESP-IDF version; Arduino 55.03.311 is fixed to 5.5.5",
    )
    parser.add_argument(
        "--expected-firmware-commit",
        default="",
        help="Expected flashed Git SHA/prefix; defaults to the host checkout HEAD",
    )
    parser.add_argument("--operator", default="")
    parser.add_argument("--board", default="")
    parser.add_argument("--note", default="")
    parser.add_argument("--startup-delay-s", type=float, default=1.0)
    parser.add_argument("--command-timeout-s", type=float, default=4.0)
    parser.add_argument("--write-timeout-s", type=float, default=1.0)
    parser.add_argument("--idle-gap-s", type=float, default=0.35)
    parser.add_argument("--serial-dtr", choices=SERIAL_LINE_STATES, default="on",
                        help="Serial DTR line state after opening the port")
    parser.add_argument("--serial-rts", choices=SERIAL_LINE_STATES, default="off",
                        help="Serial RTS line state after opening the port")
    parser.add_argument("--dry-run", action="store_true", help="List planned commands without opening serial")
    parser.add_argument("--parser-self-test", action="store_true", help="Run built-in parser/classifier checks")
    parser.add_argument("--verbose", action="store_true", help="Print per-command progress during serial runs")
    parser.add_argument("--repeat-command-set", type=int, default=1,
                        help=f"Repeat the selected command set 1..{MAX_COMMAND_SET_REPETITIONS} times")
    parser.add_argument("--expect-token", action="append", default=[], help="Additional token that can classify output as PASS")
    parser.add_argument("--failure-token", action="append", default=[], help="Additional token that classifies output as FAIL")
    parser.add_argument("--expected-failure-token", action="append", default=[],
                        help="Token that classifies an intentional negative-test error as PASS")
    parser.add_argument(
        "--expected-failure", action="append", default=[], metavar="COMMAND=TOKEN",
        help="Expected-failure token scoped to one normalized command",
    )
    parser.add_argument("--skip-default-commands", action="store_true")
    parser.add_argument("--command", action="append", default=[], help="Additional command to send")
    parser.add_argument("--include-address-0x2b", action="store_true")
    parser.add_argument(
        "--include-config-matrix", action="store_true",
        help="Append exhaustive cache-only legal setting/boundary coverage and discard it",
    )
    parser.add_argument(
        "--include-invalid-inputs", action="store_true",
        help="Append safe numeric/enum/confirmation rejection coverage",
    )
    parser.add_argument("--include-stress", action="store_true")
    parser.add_argument(
        "--include-reset-stress", action="store_true",
        help="Append confirmed bounded reset/reapply stress",
    )
    parser.add_argument(
        "--include-busfreq-stress", action="store_true",
        help="Append confirmed bounded 100/400 kHz switch/restore stress",
    )
    parser.add_argument("--stress-count", type=int, default=10)
    parser.add_argument("--include-sd", action="store_true")
    parser.add_argument("--include-intb", action="store_true")
    parser.add_argument("--include-unplug", action="store_true")
    parser.add_argument("--include-stuck-bus", action="store_true")
    parser.add_argument("--include-long-soak", action="store_true")
    parser.add_argument(
        "--allow-reduced-soak-gate", action="store_true",
        help="Explicitly permit a labeled custom reduced base gate before soak",
    )
    parser.add_argument(
        "--soak-duration-s",
        type=float,
        default=0.0,
        help=f"Bounded Arduino no-sensor soak duration, >0..{MAX_SOAK_DURATION_S}",
    )
    parser.add_argument(
        "--soak-cycle-delay-s",
        type=float,
        default=1.0,
        help=f"Delay between complete soak cycles, 0..{MAX_SOAK_CYCLE_DELAY_S}",
    )
    parser.add_argument("--include-drive-tuning", action="store_true")
    parser.add_argument("--sample-rate-count", type=int, default=0,
                        help=f"Sensor fixture: append a bounded counted read on either profile (1..{MAX_SAMPLE_RATE_COUNT})")
    parser.add_argument("--sample-rate-channel", type=int, default=0)
    parser.add_argument("--json-out", default="")
    parser.add_argument("--markdown-out", default="")
    parser.add_argument("--raw-transcript-out", default="")
    parser.add_argument("--quiet", action="store_true")
    parser.add_argument("--require-run", action="store_true", help="Exit nonzero when result is NOT_RUN")
    args = parser.parse_args(argv)

    if args.baud <= 0:
        parser.error("--baud must be > 0")
    if args.channel_count not in (2, 4):
        parser.error("--channel-count must be 2 or 4")
    if parse_int_token(args.address) not in (0x2A, 0x2B):
        parser.error("--address must be 0x2A or 0x2B")
    if (args.expected_firmware_commit and
            re.fullmatch(r"[0-9a-fA-F]{7,40}", args.expected_firmware_commit) is None):
        parser.error("--expected-firmware-commit must be a 7..40 digit hexadecimal SHA")
    timing_values = {
        "--startup-delay-s": args.startup_delay_s,
        "--command-timeout-s": args.command_timeout_s,
        "--write-timeout-s": args.write_timeout_s,
        "--idle-gap-s": args.idle_gap_s,
        "--soak-duration-s": args.soak_duration_s,
        "--soak-cycle-delay-s": args.soak_cycle_delay_s,
    }
    for option, value in timing_values.items():
        if not math.isfinite(value):
            parser.error(f"{option} must be finite")
    if args.startup_delay_s < 0.0 or args.startup_delay_s > MAX_STARTUP_DELAY_S:
        parser.error(f"--startup-delay-s must be 0..{MAX_STARTUP_DELAY_S}")
    if args.command_timeout_s <= 0.0 or args.command_timeout_s > MAX_COMMAND_TIMEOUT_S:
        parser.error(f"--command-timeout-s must be >0..{MAX_COMMAND_TIMEOUT_S}")
    if args.write_timeout_s <= 0.0 or args.write_timeout_s > MAX_WRITE_TIMEOUT_S:
        parser.error(f"--write-timeout-s must be >0..{MAX_WRITE_TIMEOUT_S}")
    if args.idle_gap_s <= 0.0 or args.idle_gap_s > MAX_IDLE_GAP_S:
        parser.error(f"--idle-gap-s must be >0..{MAX_IDLE_GAP_S}")
    if args.repeat_command_set < 1 or args.repeat_command_set > MAX_COMMAND_SET_REPETITIONS:
        parser.error(f"--repeat-command-set must be 1..{MAX_COMMAND_SET_REPETITIONS}")
    if args.stress_count < 1 or args.stress_count > MAX_STRESS_COUNT:
        parser.error(f"--stress-count must be 1..{MAX_STRESS_COUNT}")
    if args.sample_rate_count < 0 or args.sample_rate_count > MAX_SAMPLE_RATE_COUNT:
        parser.error(f"--sample-rate-count must be 0..{MAX_SAMPLE_RATE_COUNT}")
    if args.sample_rate_channel < 0 or args.sample_rate_channel > 3:
        parser.error("--sample-rate-channel must be 0..3")
    if (args.sample_rate_count > 0 and
            args.sample_rate_channel >= args.channel_count):
        parser.error(
            "--sample-rate-channel must be less than --channel-count when "
            "sample-rate benchmarking is requested"
        )
    if args.soak_duration_s < 0.0 or args.soak_duration_s > MAX_SOAK_DURATION_S:
        parser.error(f"--soak-duration-s must be 0..{MAX_SOAK_DURATION_S}")
    if args.soak_cycle_delay_s < 0.0 or args.soak_cycle_delay_s > MAX_SOAK_CYCLE_DELAY_S:
        parser.error(f"--soak-cycle-delay-s must be 0..{MAX_SOAK_CYCLE_DELAY_S}")
    if args.soak_duration_s > 0.0 and not args.include_long_soak:
        parser.error("--soak-duration-s requires --include-long-soak")
    if (args.skip_default_commands and args.include_long_soak and
            not args.allow_reduced_soak_gate):
        parser.error(
            "--skip-default-commands with --include-long-soak requires "
            "--allow-reduced-soak-gate"
        )
    try:
        compile_scoped_expected_failures(args.expected_failure)
    except ValueError as exc:
        parser.error(f"--expected-failure {exc}")
    return args


def parser_self_test() -> Tuple[bool, List[str]]:
    failures: List[str] = []
    if "version" not in default_commands("arduino"):
        failures.append("arduino default commands missing version")
    if "discover" not in default_commands("arduino"):
        failures.append("arduino default commands missing qualified discovery")
    if "drdy" in default_commands("arduino", "no-sensor"):
        failures.append("no-sensor commands must exclude DRDY")
    if "read 0x01" not in default_commands("arduino", "no-sensor"):
        failures.append("no-sensor commands missing status-aware acquisition")
    if "job" not in default_commands("arduino", "no-sensor"):
        failures.append("no-sensor commands missing cooperative progress snapshot")
    if "wake" not in default_commands("arduino"):
        failures.append("arduino default commands missing wake")
    if "ready" not in default_commands("idf"):
        failures.append("idf default commands missing ready")
    if "discover" not in default_commands("idf"):
        failures.append("idf default commands missing qualified discovery")
    if "wake" not in default_commands("idf"):
        failures.append("idf default commands missing wake")

    status, _ = classify_command(
        "version",
        "version=1.0.0 firmware_git=abcdef1 firmware_status=clean "
        "build_timestamp=test platform=pioarduino-55.03.311 "
        "framework=arduino framework_version=3.3.11 idf_version=v5.5.5 "
        "target=esp32s2 i2c_backend=esp-idf-new-master frequency_hz=400000\n> ",
        False,
    )
    if status != "PASS":
        failures.append("version informational output did not pass")

    status, _ = classify_command(
        "probe",
        "CLI scheduled: command=probe session=7\n"
        "manufacturer_id=0x5449 device_id=0x3055 match=YES\n"
        "CLI result: command=probe session=7 outcome=SUCCESS code=0\n> ",
        False,
    )
    if status != "PASS":
        failures.append("exact probe identity did not pass")

    status, _ = classify_command(
        "probe",
        "CLI scheduled: command=probe session=7\n"
        "CLI result: command=probe session=7 outcome=SUCCESS code=0\n> ",
        False,
    )
    if status != "FAIL":
        failures.append("probe without exact identity did not fail")

    status, _ = classify_command("probe", "status: I2C_TIMEOUT code=7\n> ", False)
    if status != "FAIL":
        failures.append("I2C_TIMEOUT output did not fail")

    status, _ = classify_command(
        "custom",
        "fixture ready\n> ",
        False,
        expected_patterns=compile_token_patterns(["fixture ready"]),
    )
    if status != "PASS":
        failures.append("configured expected token did not pass")

    status, _ = classify_command(
        "custom",
        "fixture unsafe\n> ",
        False,
        failure_patterns=compile_token_patterns(["fixture unsafe"]),
    )
    if status != "FAIL":
        failures.append("configured failure token did not fail")

    status, _ = classify_command(
        "init",
        "CLI scheduled: command=init session=0\n"
        "status: INVALID_PARAM code=5\n"
        "CLI result: command=init session=0 outcome=FAILED code=5\n> ",
        False,
        expected_failure_patterns=compile_token_patterns(["INVALID_PARAM"]),
    )
    if status != "PASS":
        failures.append("configured expected-failure token did not pass")

    dry_args = parse_args(["--dry-run"])
    dry_result = make_result(dry_args)
    if dry_result["overall_status"] != "NOT_RUN" or not dry_result["command_results"]:
        failures.append("dry-run result did not produce NOT_RUN command rows")

    return not failures, failures


def main(argv: List[str]) -> int:
    args = parse_args(argv)
    if args.parser_self_test:
        ok, failures = parser_self_test()
        if not args.quiet:
            if ok:
                print("parser self-test PASSED")
            else:
                print("parser self-test FAILED")
                for failure in failures:
                    print(f"- {failure}")
        return 0 if ok else 1

    result = make_result(args)
    write_outputs(args, result)

    if args.require_run and result["overall_status"] == "NOT_RUN":
        return 2
    if result["overall_status"] == "FAIL":
        return 1
    if result["overall_status"] == "UNKNOWN":
        return 3
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
