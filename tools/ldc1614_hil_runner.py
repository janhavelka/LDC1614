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
import re
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Tuple

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

ARDUINO_DEFAULT_COMMANDS = [
    "help",
    "version",
    "scan",
    "busrecover",
    "init",
    "probe",
    "drv",
    "cfg",
    "progress",
    "status",
    "sleep",
    "wake",
    "drdy",
    "timing 0x01",
    "selftest",
]

ARDUINO_NO_SENSOR_COMMANDS = [
    "help",
    "version",
    "scan",
    "busrecover",
    "init",
    "probe",
    "drv",
    "cfg",
    "progress",
    "status",
    "reg 0x7E",
    "reg 0x7F",
    "reg 0x19",
    "reg 0x1A",
    "reg 0x1B",
    "sleep",
    "wake",
    "sleep",
    "initidrive 0",
    "cfg",
    "timing 0x01",
    "selftest",
    "sleep",
    "init",
    "apply",
    "resetreapply",
    "wake",
    "acquire 0x01",
    "sleep",
    "cancel",
    "invalidate",
    "drv",
    "init",
    "wreg 0x1B 0x0209",
    "init",
    "end",
    "bind",
    "init",
    "freq 0 0x01000000",
    "drv",
    "sleep",
]

IDF_DEFAULT_COMMANDS = [
    "help",
    "version",
    "scan",
    "busrecover",
    "init",
    "probe",
    "drv",
    "cfg",
    "progress",
    "status",
    "sleep",
    "wake",
    "ready",
    "timing 0x01",
    "selftest",
]

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

ASYNC_COMMAND_NAMES = ("init", "apply", "resetreapply", "acquire", "read", "readall")
SCHEDULED_OPERATION_PATTERN = re.compile(
    r"\bscheduled(?:\s+acquire)?\s+operation=(\d+)\b", re.IGNORECASE
)

COMMAND_EVIDENCE_PATTERNS = {
    "help": (re.compile(r"(?:\bOwner-driven jobs\b|\bjobs:)", re.IGNORECASE),),
    "version": (
        re.compile(r"\bversion:\s*\d+\.\d+\.\d+\b", re.IGNORECASE),
        re.compile(r"\bfirmware_git=[0-9a-f]{7,40}\b", re.IGNORECASE),
        re.compile(r"\bfirmware_status=clean\b", re.IGNORECASE),
    ),
    "scan": (
        re.compile(r"\bscan complete found=\d+ probes=112\b", re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
    ),
    "probe": (
        re.compile(r"\bMANUFACTURER_ID=0x5449\b", re.IGNORECASE),
        re.compile(r"\bDEVICE_ID=0x3055\b", re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
    ),
    "id": (
        re.compile(r"\bMANUFACTURER_ID=0x5449\b", re.IGNORECASE),
        re.compile(r"\bDEVICE_ID=0x3055\b", re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
    ),
    "selftest": (
        re.compile(r"\bMANUFACTURER_ID=0x5449\b", re.IGNORECASE),
        re.compile(r"\bDEVICE_ID=0x3055\b", re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
    ),
    "drv": (re.compile(r"\bbound=[01]\b.*\bapplied=", re.IGNORECASE),),
    "state": (re.compile(r"\bbound=[01]\b.*\bapplied=", re.IGNORECASE),),
    "health": (re.compile(r"\bbound=[01]\b.*\bapplied=", re.IGNORECASE),),
    "cfg": (
        re.compile(r"\baddress=0x[0-9a-f]{2}\b", re.IGNORECASE),
        re.compile(r"\bvariantChannels=(?:2|4)\b", re.IGNORECASE),
    ),
    "config": (
        re.compile(r"\baddress=0x[0-9a-f]{2}\b", re.IGNORECASE),
        re.compile(r"\bvariantChannels=(?:2|4)\b", re.IGNORECASE),
    ),
    "settings": (
        re.compile(r"\baddress=0x[0-9a-f]{2}\b", re.IGNORECASE),
        re.compile(r"\bvariantChannels=(?:2|4)\b", re.IGNORECASE),
    ),
    "progress": (
        re.compile(r"\bactive=[01]\b.*\boperation=\d+\b.*\btransfers=\d+/\d+\b",
                   re.IGNORECASE),
    ),
    "status": (
        re.compile(r"\bSTATUS observed=[01]\b.*\braw=0x[0-9a-f]{4}\b",
                   re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
    ),
    "status_raw": (
        re.compile(r"\bSTATUS observed=[01]\b.*\braw=0x[0-9a-f]{4}\b",
                   re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
    ),
    "drdy": (
        re.compile(r"\bready=[01]\b", re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
    ),
    "ready": (
        re.compile(r"\bready=[01]\b", re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
    ),
    "timing": (
        re.compile(r"\bwakeSettleUs=\d+\b.*\bconversionUs=\d+\b", re.IGNORECASE),
    ),
    "freq": (
        re.compile(r"\bfrequencyHz=[0-9.]+\b", re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
    ),
    "init": (
        re.compile(r"\bresult operation=\d+\b.*\boutcome=SUCCESS\b", re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
    ),
    "apply": (
        re.compile(r"\bresult operation=\d+\b.*\boutcome=SUCCESS\b", re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
    ),
    "resetreapply": (
        re.compile(r"\bresult operation=\d+\b.*\boutcome=SUCCESS\b", re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
    ),
    "acquire": (
        re.compile(r"\bresult operation=\d+\b.*\boutcome=SUCCESS\b", re.IGNORECASE),
        re.compile(r"\bbatch selected=0x[0-9a-f]{2}\b.*\bvalid=0x[0-9a-f]{2}\b",
                   re.IGNORECASE),
        re.compile(r"\bcode=0\b", re.IGNORECASE),
    ),
    "initidrive": (
        re.compile(r"\bchannel=\d+\s+initDriveCode=\d+\b", re.IGNORECASE),
    ),
    "initdrive": (
        re.compile(r"\bchannel=\d+\s+initDriveCode=\d+\b", re.IGNORECASE),
    ),
    "reg": (re.compile(r"\breg 0x[0-9a-f]+\s*=\s*0x[0-9a-f]{4}\b",
                       re.IGNORECASE),),
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
        r"\berrors?\s*[:=]\s*[1-9][0-9]*\b",
        r"not bound",
        r"code=[1-9][0-9]*",
    )
]
SENSOR_CONDITION_PATTERNS = [
    re.compile(pattern, re.IGNORECASE)
    for pattern in (
        r"\berr=1\b",
        r"\b(?:errUR|errOR|errWD|errAmp|ur|or|wd|ah|al|zc)=1\b",
    )
]
NO_SENSOR_CONDITION_COMMANDS = (
    "status", "status_raw", "ready", "drdy", "acquire", "read", "readall"
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
        r"\b(?:variantChannels|channel\s+count)\s*[=:]\s*(\d+)",
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
    if fixture == "no-sensor" and profile == "arduino":
        return list(ARDUINO_NO_SENSOR_COMMANDS)
    if profile == "idf":
        return list(IDF_DEFAULT_COMMANDS)
    return list(ARDUINO_DEFAULT_COMMANDS)


def command_name(command: str) -> str:
    return command.strip().split(" ", 1)[0].lower()


def compile_token_patterns(tokens: Iterable[str]) -> List[re.Pattern[str]]:
    return [
        re.compile(re.escape(token), re.IGNORECASE)
        for token in tokens
        if token.strip()
    ]


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

    for pattern in failure_patterns or []:
        if pattern.search(output):
            return "FAIL", f"matched configured failure token: {pattern.pattern}"

    for pattern in expected_failure_patterns or []:
        if pattern.search(output):
            return "PASS", f"matched configured expected-failure token: {pattern.pattern}"

    name = command_name(command)
    failure_scan_output = output
    if name in ASYNC_COMMAND_NAMES:
        scheduled = SCHEDULED_OPERATION_PATTERN.search(output)
        terminal = re.search(
            r"\bresult operation=(\d+)\b[^\r\n]*\boutcome=([A-Z_]+)\b",
            output,
            re.IGNORECASE,
        )
        if scheduled is None or terminal is None:
            return "FAIL", "missing scheduled or terminal operation evidence"
        if scheduled.group(1) != terminal.group(1):
            return "FAIL", "scheduled and terminal operation IDs differ"
        failure_scan_output = re.sub(
            r"\bstatus:\s*code=5\b[^\r\n]*", "", output,
            flags=re.IGNORECASE,
        )
    for pattern in FAIL_PATTERNS:
        if pattern.search(failure_scan_output):
            return "FAIL", f"matched failure pattern: {pattern.pattern}"

    sensor_condition_observed = any(
        pattern.search(output) for pattern in SENSOR_CONDITION_PATTERNS
    )
    sensor_condition_expected = (
        fixture == "no-sensor" and name in NO_SENSOR_CONDITION_COMMANDS
    )
    if sensor_condition_observed and not sensor_condition_expected:
        return "FAIL", "sensor-condition fault flag was asserted"

    required_evidence = COMMAND_EVIDENCE_PATTERNS.get(name)
    if required_evidence is not None:
        missing = [pattern.pattern for pattern in required_evidence
                   if pattern.search(output) is None]
        if missing:
            return "FAIL", "missing command-specific evidence: " + ", ".join(missing)
        if sensor_condition_expected and sensor_condition_observed:
            return "PASS", "command evidence parsed; sensor-condition flags expected for no-sensor fixture"
        return "PASS", "all command-specific evidence parsed"

    for pattern in OK_PATTERNS:
        if pattern.search(output):
            return "PASS", "status output indicates OK"

    for pattern in expected_patterns or []:
        if pattern.search(output):
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


def ensure_wake_command(commands: List[str]) -> None:
    if not any(command_name(command) == "wake" for command in commands):
        commands.append("wake")


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
    if args.profile != "arduino":
        summary["reason"] = "IDF diagnostic CLI has no stress command"
        return summary

    command = f"stress {args.stress_count}"
    result = find_command_result(command_results, command)
    if result is None:
        summary["reason"] = "v3 diagnostic CLIs expose no stress command"
        return summary

    summary["elapsed_s"] = float(result.get("elapsed_s", 0.0))
    status = str(result.get("status", "UNKNOWN"))
    if status == "NOT_RUN":
        summary["reason"] = str(result.get("reason", "stress command was not run"))
        return summary

    output = strip_ansi(str(result.get("output", "")))
    match = STRESS_RESULT_PATTERN.search(output)
    if not match:
        summary["status"] = "FAIL" if status == "FAIL" else "UNKNOWN"
        summary["reason"] = (
            str(result.get("reason", "stress command failed"))
            if status == "FAIL"
            else "stress summary counts were not parsed"
        )
        return summary

    ok_count = int(match.group(1))
    fail_count = int(match.group(2))
    elapsed_s = float(summary["elapsed_s"])
    summary["success_count"] = ok_count
    summary["failure_count"] = fail_count
    summary["effective_hz"] = (ok_count / elapsed_s) if elapsed_s > 0.0 else None
    summary["status"] = "FAIL" if fail_count > 0 or status == "FAIL" else "PASS"
    summary["reason"] = "stress command reported failures" if fail_count > 0 else "stress summary parsed"
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
    if args.profile != "arduino":
        summary["reason"] = "IDF diagnostic CLI has no counted read command"
        return summary

    command = f"samplerate {args.sample_rate_channel} {args.sample_rate_count}"
    result = find_command_result(command_results, command)
    if result is None:
        summary["reason"] = "v3 diagnostic CLIs expose no counted acquisition command"
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
    try:
        import serial  # type: ignore[import-not-found]
    except Exception as exc:
        raise RuntimeError(f"pyserial is required for serial HIL runs: {exc}") from exc

    prompt_patterns = [r">\s*$", r"ldc1614-idf>\s*$"]
    expected_patterns = compile_token_patterns(args.expect_token)
    failure_patterns = compile_token_patterns(args.failure_token)
    expected_failure_patterns = compile_token_patterns(args.expected_failure_token)
    transcript_parts: List[str] = []
    results: List[Dict[str, object]] = []
    startup_elapsed_s = 0.0
    soak_summary: Optional[Dict[str, object]] = None

    with serial.Serial(
        args.port,
        args.baud,
        timeout=0.05,
        write_timeout=args.write_timeout_s,
        rtscts=False,
        dsrdtr=False,
    ) as ser:
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
        transcript_parts.append("### startup\n" + startup)

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
            if (name in ASYNC_COMMAND_NAMES and scheduled is not None and
                    re.search(r"\bstatus:\s*code=5\b", output,
                              re.IGNORECASE) is not None and
                    re.search(
                        rf"\bresult operation={re.escape(scheduled.group(1))}\b",
                        output,
                        re.IGNORECASE,
                    ) is None):
                completion, completion_timed_out = read_available(
                    ser,
                    command_deadline,
                    args.idle_gap_s,
                    (),
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
                expected_failure_patterns,
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
            result = execute(command)
            result["index"] = index
            transcript_parts.append(
                f"### command {index}: {command}\n{result['output']}"
            )
            if args.verbose:
                print(
                    f"[{index}/{len(commands)}] {command}: {result['status']} "
                    f"({float(result['elapsed_s']):.3f}s)"
                )
            results.append(result)

        if (args.include_long_soak and args.soak_duration_s > 0.0 and
                args.fixture == "no-sensor" and args.profile == "arduino"):
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
                cycle_count += 1
                for command in NO_SENSOR_SOAK_COMMANDS:
                    command_count += 1
                    result = enforce_soak_invariant(command, execute(command))
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
                                "cycle": cycle_count,
                                "command": command,
                                "status": status,
                                "reason": result["reason"],
                                "output": output,
                            }
                        )
                    transcript_parts.append(
                        f"### soak cycle {cycle_count} command {command_count}: "
                        f"{command}\n{output}"
                    )

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
                "requested_duration_s": args.soak_duration_s,
                "elapsed_s": elapsed_s,
                "cycle_count": cycle_count,
                "command_count": command_count,
                "command_counts": command_counts,
                "failure_count": failure_count,
                "unknown_count": unknown_count,
                "reset_count": reset_count,
                "worst_latency_s": worst_latency_s,
                "non_pass_details": non_pass_details,
            }

    full_transcript = "\n".join(transcript_parts)
    firmware_version = "unknown"
    version_match = re.search(r"(?:version|library version):\s*(\d+\.\d+\.\d+)",
                              full_transcript, re.IGNORECASE)
    if version_match:
        firmware_version = version_match.group(1).strip()
    return results, full_transcript, firmware_version, startup_elapsed_s, soak_summary


def add_optional_commands(args: argparse.Namespace, commands: List[str], skipped: List[Dict[str, str]]) -> None:
    if args.include_address_0x2b:
        skipped.append(
            {
                "name": "address_0x2B",
                "reason": "rebuild with the explicit 0x2B profile; runtime address changes are not exposed",
            }
        )

    if args.include_stress:
        skipped.append(
            {
                "name": "stress",
                "reason": "v3 diagnostic CLIs expose no production-cadence stress command",
            }
        )

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
            skipped.append(
                {
                    "name": "sample_rate_benchmark",
                    "reason": "v3 diagnostic CLIs expose no production-cadence counted acquisition command",
                }
            )

    for name, enabled in (
        ("sd_shutdown_wake", args.include_sd),
        ("intb_observation", args.include_intb),
        ("unplug_replug", args.include_unplug),
        ("stuck_bus", args.include_stuck_bus),
        ("drive_current_tuning", args.include_drive_tuning),
    ):
        if enabled:
            skipped.append(
                {
                    "name": name,
                    "reason": "manual/fixture evidence required; no safe automatic firmware command is defined",
                }
            )

    if args.include_long_soak and (
            args.soak_duration_s <= 0.0 or args.fixture != "no-sensor" or
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

    if args.dry_run:
        not_run_reason = "dry-run requested; no serial commands were sent"
        command_results = not_run_command_results(commands, not_run_reason)
    elif not args.port:
        not_run_reason = "serial port was not supplied"
    else:
        try:
            (command_results, transcript, firmware_version, startup_elapsed_s,
             observed_soak) = run_serial_commands(args, commands)
            append_expectation_results(args, command_results, transcript)
        except Exception as exc:
            not_run_reason = str(exc)

    if (args.port and not args.dry_run and not commands and not not_run_reason and
            not transcript_payload(transcript)):
        not_run_reason = "no serial startup transcript payload captured"

    has_transcript = bool(transcript_payload(transcript)) and not args.dry_run and not not_run_reason
    if has_transcript:
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
        "expect_tokens": args.expect_token,
        "failure_tokens": args.failure_token,
        "expected_failure_tokens": args.expected_failure_token,
        "sample_rate_count": args.sample_rate_count,
        "sample_rate_channel": args.sample_rate_channel,
        "soak_duration_s": args.soak_duration_s,
        "soak_cycle_delay_s": args.soak_cycle_delay_s,
        "commands": commands,
        "command_results": command_results,
        "skipped_optional_tests": skipped,
        "not_run_reason": not_run_reason,
        "transcript": transcript,
    }
    result["stress"] = summarize_stress(args, command_results)
    result["sample_rate"] = summarize_sample_rate(args, command_results)
    result["soak"] = summarize_soak(args, observed_soak)
    result["overall_status"] = overall_status(command_results, not_run_reason, transcript)
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
    ]
    if result.get("not_run_reason"):
        lines.append(f"Not-run reason: `{result['not_run_reason']}`")
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
    parser.add_argument("--skip-default-commands", action="store_true")
    parser.add_argument("--command", action="append", default=[], help="Additional command to send")
    parser.add_argument("--include-address-0x2b", action="store_true")
    parser.add_argument("--include-stress", action="store_true")
    parser.add_argument("--stress-count", type=int, default=10)
    parser.add_argument("--include-sd", action="store_true")
    parser.add_argument("--include-intb", action="store_true")
    parser.add_argument("--include-unplug", action="store_true")
    parser.add_argument("--include-stuck-bus", action="store_true")
    parser.add_argument("--include-long-soak", action="store_true")
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
                        help=f"Arduino profile only: append a bounded counted read (1..{MAX_SAMPLE_RATE_COUNT})")
    parser.add_argument("--sample-rate-channel", type=int, default=0)
    parser.add_argument("--json-out", default="")
    parser.add_argument("--markdown-out", default="")
    parser.add_argument("--raw-transcript-out", default="")
    parser.add_argument("--quiet", action="store_true")
    parser.add_argument("--require-run", action="store_true", help="Exit nonzero when result is NOT_RUN")
    args = parser.parse_args(argv)

    if args.fixture == "no-sensor" and args.profile != "arduino":
        parser.error("--fixture no-sensor is currently supported only by the Arduino CLI profile")
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
    if args.soak_duration_s < 0.0 or args.soak_duration_s > MAX_SOAK_DURATION_S:
        parser.error(f"--soak-duration-s must be 0..{MAX_SOAK_DURATION_S}")
    if args.soak_cycle_delay_s < 0.0 or args.soak_cycle_delay_s > MAX_SOAK_CYCLE_DELAY_S:
        parser.error(f"--soak-cycle-delay-s must be 0..{MAX_SOAK_CYCLE_DELAY_S}")
    if args.soak_duration_s > 0.0 and not args.include_long_soak:
        parser.error("--soak-duration-s requires --include-long-soak")
    return args


def parser_self_test() -> Tuple[bool, List[str]]:
    failures: List[str] = []
    if "version" not in default_commands("arduino"):
        failures.append("arduino default commands missing version")
    if "busrecover" not in default_commands("arduino"):
        failures.append("arduino default commands missing explicit bus recovery")
    if "drdy" in default_commands("arduino", "no-sensor"):
        failures.append("no-sensor commands must exclude DRDY")
    if "acquire 0x01" not in default_commands("arduino", "no-sensor"):
        failures.append("no-sensor commands missing status-aware acquisition")
    if "progress" not in default_commands("arduino", "no-sensor"):
        failures.append("no-sensor commands missing cooperative progress snapshot")
    if "wake" not in default_commands("arduino"):
        failures.append("arduino default commands missing wake")
    if "ready" not in default_commands("idf"):
        failures.append("idf default commands missing ready")
    if "scan" not in default_commands("idf"):
        failures.append("idf default commands missing scan")
    if "busrecover" not in default_commands("idf"):
        failures.append("idf default commands missing explicit bus recovery")
    if "wake" not in default_commands("idf"):
        failures.append("idf default commands missing wake")

    status, _ = classify_command(
        "version",
        "version: 1.0.0 firmware_git=abcdef1 firmware_status=clean\n> ",
        False,
    )
    if status != "PASS":
        failures.append("version informational output did not pass")

    status, _ = classify_command(
        "probe",
        "MANUFACTURER_ID=0x5449 DEVICE_ID=0x3055\nstatus: code=0\n> ",
        False,
    )
    if status != "PASS":
        failures.append("exact probe identity did not pass")

    status, _ = classify_command("probe", "status: code=0\n> ", False)
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
        "invalid-channel",
        "status: INVALID_PARAM code=5\n> ",
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
