#!/usr/bin/env python3
"""Conservative serial HIL runner for LDC1614 diagnostic firmware.

The runner never reports PASS when no serial hardware is supplied. Without a
port it emits a NOT_RUN artifact. Optional fault/soak/address checks are gated
behind flags because they require board/operator control.
"""

from __future__ import annotations

import argparse
import json
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

ARDUINO_DEFAULT_COMMANDS = [
    "help",
    "version",
    "scan",
    "probe",
    "id",
    "drv",
    "cfg",
    "status",
    "sleep",
    "wake",
    "drdy",
    "read",
    "readfresh",
    "readstaged 0x01 8 1",
    "recover",
    "timing 0 43000000",
    "selftest",
]

IDF_DEFAULT_COMMANDS = [
    "help",
    "version",
    "probe",
    "drv",
    "cfg",
    "status",
    "sleep",
    "wake",
    "ready",
    "read",
    "readall",
    "recover",
    "timing 0 43000000",
    "selftest",
]

INFO_COMMANDS = {
    "help",
    "version",
    "scan",
    "id",
    "drv",
    "cfg",
    "config",
    "settings",
    "state",
    "health",
    "drdy",
    "ready",
    "timing",
}
FAIL_PATTERNS = [
    re.compile(pattern, re.IGNORECASE)
    for pattern in (
        r"\bunknown command\b",
        r"\bDEVICE_NOT_FOUND\b",
        r"\bI2C_(?:ERROR|TIMEOUT|NACK_ADDR|NACK_DATA|BUS)\b",
        r"\bINVALID_(?:CONFIG|PARAM)\b",
        r"\bNOT_INITIALIZED\b",
        r"\bTIMEOUT\b",
        r"\bBUSY\b",
        r"\b(?:begin|init|probe|read|write|recover|selftest|command)\s+failed\b",
        r"\[FAIL\]",
        r"\[ERR:",
        r"\bmatch=(?:\x1b\[[0-9;]*m)*NO\b",
        r"\bfail=(?:\x1b\[[0-9;]*m)*[1-9][0-9]*\b",
        r"\bread_fail=(?:\x1b\[[0-9;]*m)*[1-9][0-9]*\b",
        r"\bconfig_readback_failures=(?:\x1b\[[0-9;]*m)*[1-9][0-9]*\b",
        r"\bfailed\s*[:=]\s*[1-9][0-9]*\b",
        r"\berrors?\s*[:=]\s*[1-9][0-9]*\b",
        r"\berr=1\b",
        r"\b(?:errUR|errOR|errWD|errAmp|ur|or|wd|ah|al|zc)=1\b",
        r"not online",
        r"not initialized",
        r"code=[1-9][0-9]*",
    )
]
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
        r"\bchannel(?:\s+count|s)?\s*[=:]\s*(\d+)",
    )
]


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


def default_commands(profile: str) -> List[str]:
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

    for pattern in FAIL_PATTERNS:
        if pattern.search(output):
            return "FAIL", f"matched failure pattern: {pattern.pattern}"

    name = command_name(command)
    if name in INFO_COMMANDS:
        return "PASS", "informational command responded without failure pattern"

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
    if expected_address is not None and actual_address is not None and actual_address != expected_address:
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
    if actual_channel_count is not None and actual_channel_count != expected_channel_count:
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


def ensure_wake_command(commands: List[str]) -> None:
    if not any(command_name(command) == "wake" for command in commands):
        commands.append("wake")


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
        summary["status"] = "UNKNOWN"
        summary["reason"] = "stress command result was not captured"
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
        summary["status"] = "UNKNOWN"
        summary["reason"] = "sample-rate command result was not captured"
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


def summarize_soak(args: argparse.Namespace) -> Dict[str, object]:
    if not args.include_long_soak:
        return make_not_run_summary("long soak was not requested")
    return {
        "status": "NOT_RUN",
        "reason": "manual/fixture evidence required; no safe automatic firmware command is defined",
        "requested_duration_s": 8 * 60 * 60,
        "elapsed_s": 0.0,
        "command_counts": {},
        "failure_count": None,
        "recovery_count": None,
        "reset_count": None,
        "worst_latency_s": None,
    }


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
) -> Tuple[List[Dict[str, object]], str, str, float]:
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

    with serial.Serial(args.port, args.baud, timeout=0.05, write_timeout=args.write_timeout_s) as ser:
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

        for index, command in enumerate(commands, start=1):
            command_start = time.monotonic()
            ser.write((command + "\n").encode("utf-8"))
            ser.flush()
            output, timed_out = read_available(
                ser,
                time.monotonic() + args.command_timeout_s,
                args.idle_gap_s,
                prompt_patterns,
            )
            elapsed_s = time.monotonic() - command_start
            status, reason = classify_command(
                command,
                output,
                timed_out,
                expected_patterns,
                failure_patterns,
                expected_failure_patterns,
            )
            transcript_parts.append(f"### command {index}: {command}\n{output}")
            if args.verbose:
                print(f"[{index}/{len(commands)}] {command}: {status} ({elapsed_s:.3f}s)")
            results.append(
                {
                    "index": index,
                    "command": command,
                    "status": status,
                    "reason": reason,
                    "timed_out": timed_out,
                    "elapsed_s": elapsed_s,
                    "output": output,
                }
            )

    full_transcript = "\n".join(transcript_parts)
    firmware_version = "unknown"
    version_match = re.search(r"(?:version|library version):\s*([^\r\n]+)", full_transcript, re.IGNORECASE)
    if version_match:
        firmware_version = version_match.group(1).strip()
    return results, full_transcript, firmware_version, startup_elapsed_s


def add_optional_commands(args: argparse.Namespace, commands: List[str], skipped: List[Dict[str, str]]) -> None:
    if args.include_address_0x2b:
        if args.profile == "arduino":
            commands.append("probeaddr 0x2B")
        else:
            skipped.append(
                {
                    "name": "address_0x2B",
                    "reason": "IDF diagnostic CLI has no probeaddr command",
                }
            )

    if args.include_stress:
        if args.profile == "arduino":
            ensure_wake_command(commands)
            commands.append(f"stress {args.stress_count}")
        else:
            skipped.append(
                {
                    "name": "stress",
                    "reason": "IDF diagnostic CLI has no stress command",
                }
            )

    if args.sample_rate_count != 0:
        if args.sample_rate_count < 0 or args.sample_rate_count > MAX_SAMPLE_RATE_COUNT:
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
        elif args.profile == "arduino":
            ensure_wake_command(commands)
            commands.append(f"samplerate {args.sample_rate_channel} {args.sample_rate_count}")
        else:
            skipped.append(
                {
                    "name": "sample_rate_benchmark",
                    "reason": "IDF diagnostic CLI has no counted read command",
                }
            )

    for name, enabled in (
        ("sd_shutdown_wake", args.include_sd),
        ("intb_observation", args.include_intb),
        ("unplug_replug", args.include_unplug),
        ("stuck_bus", args.include_stuck_bus),
        ("long_soak", args.include_long_soak),
        ("drive_current_tuning", args.include_drive_tuning),
    ):
        if enabled:
            skipped.append(
                {
                    "name": name,
                    "reason": "manual/fixture evidence required; no safe automatic firmware command is defined",
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


def make_result(args: argparse.Namespace) -> Dict[str, object]:
    skipped: List[Dict[str, str]] = []
    commands = [] if args.skip_default_commands else default_commands(args.profile)
    commands.extend(args.command)
    add_optional_commands(args, commands, skipped)

    not_run_reason: Optional[str] = None
    command_results: List[Dict[str, object]] = []
    transcript = ""
    firmware_version = "unknown"
    startup_elapsed_s = 0.0

    if args.dry_run:
        not_run_reason = "dry-run requested; no serial commands were sent"
        command_results = not_run_command_results(commands, not_run_reason)
    elif not args.port:
        not_run_reason = "serial port was not supplied"
    else:
        try:
            command_results, transcript, firmware_version, startup_elapsed_s = run_serial_commands(
                args,
                commands,
            )
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

    result: Dict[str, object] = {
        "tool": "ldc1614_hil_runner",
        "timestamp_utc": timestamp_utc(),
        "git_commit": git_value(["rev-parse", "--short", "HEAD"]),
        "git_status": "dirty" if git_value(["status", "--porcelain"], "") else "clean",
        "library_version": load_library_version(),
        "firmware_version": firmware_version,
        "profile": args.profile,
        "port": args.port or "",
        "baud": args.baud,
        "expected_address": args.address,
        "expected_channel_count": args.channel_count,
        "operator": args.operator,
        "board": args.board,
        "notes": args.note,
        "dry_run": args.dry_run,
        "serial_port_requested": bool(args.port),
        "hardware_attached": has_transcript,
        "evidence_type": evidence_type,
        "startup_delay_s": args.startup_delay_s,
        "startup_elapsed_s": startup_elapsed_s,
        "command_timeout_s": args.command_timeout_s,
        "write_timeout_s": args.write_timeout_s,
        "idle_gap_s": args.idle_gap_s,
        "expect_tokens": args.expect_token,
        "failure_tokens": args.failure_token,
        "expected_failure_tokens": args.expected_failure_token,
        "sample_rate_count": args.sample_rate_count,
        "sample_rate_channel": args.sample_rate_channel,
        "commands": commands,
        "command_results": command_results,
        "skipped_optional_tests": skipped,
        "not_run_reason": not_run_reason,
        "transcript": transcript,
    }
    result["overall_status"] = overall_status(command_results, not_run_reason, transcript)
    result["stress"] = summarize_stress(args, command_results)
    result["sample_rate"] = summarize_sample_rate(args, command_results)
    result["soak"] = summarize_soak(args)
    return result


def render_markdown(result: Dict[str, object]) -> str:
    lines = [
        "# LDC1614 HIL Run",
        "",
        f"Overall status: `{result['overall_status']}`",
        f"Timestamp UTC: `{result['timestamp_utc']}`",
        f"Git commit: `{result['git_commit']}`",
        f"Git status: `{result['git_status']}`",
        f"Library version: `{result['library_version']}`",
        f"Firmware version: `{result['firmware_version']}`",
        f"Profile: `{result['profile']}`",
        f"Port: `{result['port']}`",
        f"Baud: `{result['baud']}`",
        f"Expected address: `{result['expected_address']}`",
        f"Expected channel count: `{result['expected_channel_count']}`",
        f"Operator: `{result['operator']}`",
        f"Board: `{result['board']}`",
        f"Dry run: `{result['dry_run']}`",
        f"Serial port requested: `{result['serial_port_requested']}`",
        f"Hardware attached: `{result['hardware_attached']}`",
        f"Evidence type: `{result['evidence_type']}`",
        f"Startup delay: `{result['startup_delay_s']}` s",
        f"Startup read elapsed: `{float(result['startup_elapsed_s']):.3f}` s",
        f"Command timeout: `{result['command_timeout_s']}` s",
        f"Idle gap: `{result['idle_gap_s']}` s",
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
        f"Failure count: `{soak.get('failure_count', None)}`",
        f"Recovery count: `{soak.get('recovery_count', None)}`",
        f"Reset count: `{soak.get('reset_count', None)}`",
        f"Worst latency s: `{soak.get('worst_latency_s', None)}`",
    ])

    lines.extend(["", "## Transcript", "", "```text", str(result.get("transcript", "")), "```", ""])
    return "\n".join(lines)


def write_outputs(args: argparse.Namespace, result: Dict[str, object]) -> None:
    json_text = json.dumps(result, indent=2, sort_keys=True) + "\n"
    markdown_text = render_markdown(result)

    if args.json_out:
        Path(args.json_out).write_text(json_text, encoding="utf-8", newline="\n")
    if args.markdown_out:
        Path(args.markdown_out).write_text(markdown_text, encoding="utf-8", newline="\n")

    if not args.quiet:
        print(markdown_text)


def parse_args(argv: List[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--profile", choices=("arduino", "idf"), default="arduino")
    parser.add_argument("--port", default="", help="Serial port. Omit to produce NOT_RUN.")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--address", default="0x2A")
    parser.add_argument("--channel-count", type=int, default=4)
    parser.add_argument("--operator", default="")
    parser.add_argument("--board", default="")
    parser.add_argument("--note", default="")
    parser.add_argument("--startup-delay-s", type=float, default=1.0)
    parser.add_argument("--command-timeout-s", type=float, default=4.0)
    parser.add_argument("--write-timeout-s", type=float, default=1.0)
    parser.add_argument("--idle-gap-s", type=float, default=0.35)
    parser.add_argument("--dry-run", action="store_true", help="List planned commands without opening serial")
    parser.add_argument("--parser-self-test", action="store_true", help="Run built-in parser/classifier checks")
    parser.add_argument("--verbose", action="store_true", help="Print per-command progress during serial runs")
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
    parser.add_argument("--include-drive-tuning", action="store_true")
    parser.add_argument("--sample-rate-count", type=int, default=0,
                        help=f"Arduino profile only: append a bounded counted read (1..{MAX_SAMPLE_RATE_COUNT})")
    parser.add_argument("--sample-rate-channel", type=int, default=0)
    parser.add_argument("--json-out", default="")
    parser.add_argument("--markdown-out", default="")
    parser.add_argument("--quiet", action="store_true")
    parser.add_argument("--require-run", action="store_true", help="Exit nonzero when result is NOT_RUN")
    args = parser.parse_args(argv)

    if args.baud <= 0:
        parser.error("--baud must be > 0")
    if args.channel_count not in (2, 4):
        parser.error("--channel-count must be 2 or 4")
    if args.startup_delay_s < 0.0 or args.startup_delay_s > MAX_STARTUP_DELAY_S:
        parser.error(f"--startup-delay-s must be 0..{MAX_STARTUP_DELAY_S}")
    if args.command_timeout_s <= 0.0 or args.command_timeout_s > MAX_COMMAND_TIMEOUT_S:
        parser.error(f"--command-timeout-s must be >0..{MAX_COMMAND_TIMEOUT_S}")
    if args.write_timeout_s <= 0.0 or args.write_timeout_s > MAX_WRITE_TIMEOUT_S:
        parser.error(f"--write-timeout-s must be >0..{MAX_WRITE_TIMEOUT_S}")
    if args.idle_gap_s <= 0.0 or args.idle_gap_s > MAX_IDLE_GAP_S:
        parser.error(f"--idle-gap-s must be >0..{MAX_IDLE_GAP_S}")
    if args.stress_count < 1 or args.stress_count > MAX_STRESS_COUNT:
        parser.error(f"--stress-count must be 1..{MAX_STRESS_COUNT}")
    return args


def parser_self_test() -> Tuple[bool, List[str]]:
    failures: List[str] = []
    if "version" not in default_commands("arduino"):
        failures.append("arduino default commands missing version")
    if "wake" not in default_commands("arduino"):
        failures.append("arduino default commands missing wake before reads")
    if "ready" not in default_commands("idf"):
        failures.append("idf default commands missing ready")
    if "wake" not in default_commands("idf"):
        failures.append("idf default commands missing wake before reads")

    status, _ = classify_command("version", "version: 1.0.0\n> ", False)
    if status != "PASS":
        failures.append("version informational output did not pass")

    status, _ = classify_command("probe", "status: code=0\n> ", False)
    if status != "PASS":
        failures.append("code=0 output did not pass")

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
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
