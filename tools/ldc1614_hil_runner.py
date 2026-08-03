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
    "help extra", "version extra", "init 0", "resetreapply",
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
) | frozenset(("scan",))
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
        re.compile(r"===\s+LDC1614 CLI\s+===", re.IGNORECASE),
        re.compile(rf"\bcommand_count={len(COMMAND_SPECS)}\b", re.IGNORECASE),
    ),
    "version": (
        FIRMWARE_VERSION_PATTERN,
        re.compile(r"\bfirmware_git=[0-9a-f]{7,40}\b", re.IGNORECASE),
        re.compile(r"\bfirmware_status=clean\b", re.IGNORECASE),
        re.compile(r"\bbuild_timestamp=\S+", re.IGNORECASE),
    ),
    "color": (re.compile(r"\bcolor enabled=[01]\b", re.IGNORECASE),),
    "verbose": (re.compile(r"\bverbose enabled=[01]\b", re.IGNORECASE),),
    "scan": (
        re.compile(r"\bscan complete found=\d+ probes=112\b", re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
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
        re.compile(r"\bwake_settle_us=\d+\b.*\bconversion_us=\d+\b", re.IGNORECASE),
    ),
    "freq": (
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
    "errors": (re.compile(r"\b(?:errors\s+)?ur=[01]\s+or=[01]\s+wd=[01]\s+ah=[01]\s+al=[01]\s+zc=[01]\s+drdy=[01]\b", re.IGNORECASE),),
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
SAMPLE_LINE_PATTERN = re.compile(r"\bSample\s+\d+\s*/\s*\d+\b", re.IGNORECASE)
SAMPLE_FAIL_PATTERN = re.compile(r"\bSample\s+\d+\s*/\s*\d+\s+failed\b", re.IGNORECASE)
STRESS_RESULT_PATTERN = re.compile(
    r"Stress results:\s*(\d+)\s+ok,\s*(\d+)\s+failed",
    re.IGNORECASE,
)
STRESS_DETAIL_PATTERN = re.compile(
    r"Stress result:\s*requested=(\d+)\s+ok=(\d+)\s+fail=(\d+)"
    r"\s+elapsed_ms=(\d+)(?:\s+hz=([0-9.]+))?",
    re.IGNORECASE,
)
SAMPLE_RATE_RESULT_PATTERN = re.compile(
    r"SampleRate result:\s*requested=(\d+)\s+ok=(\d+)\s+fail=(\d+)"
    r"\s+elapsed_ms=(\d+)\s+hz=([0-9.]+)",
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


def classify_command(
    command: str,
    output: str,
    timed_out: bool,
    expected_patterns: Optional[List[re.Pattern[str]]] = None,
    failure_patterns: Optional[List[re.Pattern[str]]] = None,
    expected_failure_patterns: Optional[List[re.Pattern[str]]] = None,
    fixture: str = "default",
) -> Tuple[str, str]:
    if timed_out:
        return "FAIL", "command response timed out"
    if not output.strip():
        return "FAIL", "no response captured"

    parsed_output = strip_ansi(output)

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
        if "=== LDC1614" in str(result.get("output", "")):
            return "unexpected firmware restart banner occurred during the base matrix"

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
    rate_match = SAMPLE_RATE_RESULT_PATTERN.search(output)
    if rate_match:
        observed_count = int(rate_match.group(2))
        failure_count = int(rate_match.group(3))
        elapsed_s = int(rate_match.group(4)) / 1000.0
        summary["elapsed_s"] = elapsed_s
        summary["observed_count"] = observed_count
        summary["failure_count"] = failure_count
        summary["effective_hz"] = float(rate_match.group(5))
        summary["status"] = "FAIL" if failure_count > 0 or status == "FAIL" else "PASS"
        summary["reason"] = (
            "sample-rate command reported failures"
            if failure_count > 0
            else "sample-rate summary parsed"
        )
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
    if command_name(command) != "drv" or result.get("status") != "PASS":
        return result
    if re.search(
        r"\bbound=1\b.*\bapplied=APPLIED_ACTIVE\b",
        str(result.get("output", "")),
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
                    TERMINAL_RESULT_PATTERN.search(output) is None):
                session = re.escape(scheduled.group(2))
                completion, completion_timed_out = read_available(
                    ser,
                    command_deadline,
                    args.idle_gap_s,
                    (rf"CLI result:.*session={session}.*(?:\r?\n|$)",),
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
                    if "=== LDC1614" in output:
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
        commands.extend(INVALID_INPUT_COMMANDS)

    skipped.append(
        {
            "name": "address_0x2B",
            "reason": (
                "NOT_RUN: requested, but address is a build/fixture fact; rebuild and "
                "rewire for 0x2B"
                if args.include_address_0x2b else
                "NOT_RUN: requires a separately rebuilt and rewired 0x2B fixture"
            ),
        }
    )
    skipped.append(
        {
            "name": "variant_LDC1612",
            "reason": "NOT_RUN: requires a separately rebuilt LDC1612 fixture with its complete two-channel profile",
        }
    )

    if args.include_stress:
        commands.append("wake")
        commands.extend(optional_group_commands("stress", count=args.stress_count))
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

    for name, enabled, requirement in (
        ("sd_shutdown_wake", args.include_sd, "wired SD control and power-state observation"),
        ("intb_observation", args.include_intb, "wired INTB and independent pin observation"),
        ("unplug_replug", args.include_unplug, "operator-controlled unplug/replug fixture"),
        ("stuck_bus", args.include_stuck_bus, "operator-controlled stuck-bus fault fixture"),
        ("drive_current_tuning", args.include_drive_tuning, "an approved coil and drive-current tuning fixture"),
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
    if "busrecover confirm" not in default_commands("arduino"):
        failures.append("arduino default commands missing explicit bus recovery")
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
    if "scan" not in default_commands("idf"):
        failures.append("idf default commands missing scan")
    if "busrecover confirm" not in default_commands("idf"):
        failures.append("idf default commands missing explicit bus recovery")
    if "wake" not in default_commands("idf"):
        failures.append("idf default commands missing wake")

    status, _ = classify_command(
        "version",
        "version=1.0.0 firmware_git=abcdef1 firmware_status=clean "
        "build_timestamp=test\n> ",
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
