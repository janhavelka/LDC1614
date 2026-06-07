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

ARDUINO_DEFAULT_COMMANDS = [
    "help",
    "version",
    "scan",
    "probe",
    "id",
    "drv",
    "cfg",
    "status",
    "drdy",
    "read",
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
    "ready",
    "read",
    "readall",
    "recover",
    "timing 0 43000000",
    "selftest",
]

INFO_COMMANDS = {"help", "version", "scan", "drv", "cfg", "timing"}
FAIL_PATTERNS = [
    re.compile(pattern, re.IGNORECASE)
    for pattern in (
        r"\bDEVICE_NOT_FOUND\b",
        r"\bI2C_(?:ERROR|TIMEOUT|NACK_ADDR|NACK_DATA|BUS)\b",
        r"\bINVALID_(?:CONFIG|PARAM)\b",
        r"\bNOT_INITIALIZED\b",
        r"\bTIMEOUT\b",
        r"\bBUSY\b",
        r"\bfailed\b",
        r"\berror\b",
        r"not online",
        r"not initialized",
        r"code=[1-9][0-9]*",
    )
]
OK_PATTERNS = [
    re.compile(pattern, re.IGNORECASE)
    for pattern in (r"\bOK\b", r"code=0", r"status:\s*0")
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


def classify_command(command: str, output: str, timed_out: bool) -> Tuple[str, str]:
    if timed_out:
        return "FAIL", "command response timed out"
    if not output.strip():
        return "FAIL", "no response captured"

    for pattern in FAIL_PATTERNS:
        if pattern.search(output):
            return "FAIL", f"matched failure pattern: {pattern.pattern}"

    name = command_name(command)
    if name in INFO_COMMANDS:
        return "PASS", "informational command responded without failure pattern"

    for pattern in OK_PATTERNS:
        if pattern.search(output):
            return "PASS", "status output indicates OK"

    return "REVIEW", "no explicit failure found, but no OK status was parsed"


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


def run_serial_commands(args: argparse.Namespace, commands: List[str]) -> Tuple[List[Dict[str, object]], str, str]:
    try:
        import serial  # type: ignore[import-not-found]
    except Exception as exc:
        raise RuntimeError(f"pyserial is required for serial HIL runs: {exc}") from exc

    prompt_patterns = [r">\s*$", r"ldc1614-idf>\s*$"]
    transcript_parts: List[str] = []
    results: List[Dict[str, object]] = []

    with serial.Serial(args.port, args.baud, timeout=0.05, write_timeout=args.write_timeout_s) as ser:
        time.sleep(args.startup_delay_s)
        startup, _ = read_available(
            ser,
            time.monotonic() + args.command_timeout_s,
            args.idle_gap_s,
            prompt_patterns,
        )
        transcript_parts.append("### startup\n" + startup)

        for index, command in enumerate(commands, start=1):
            ser.write((command + "\n").encode("utf-8"))
            ser.flush()
            output, timed_out = read_available(
                ser,
                time.monotonic() + args.command_timeout_s,
                args.idle_gap_s,
                prompt_patterns,
            )
            status, reason = classify_command(command, output, timed_out)
            transcript_parts.append(f"### command {index}: {command}\n{output}")
            results.append(
                {
                    "index": index,
                    "command": command,
                    "status": status,
                    "reason": reason,
                    "timed_out": timed_out,
                    "output": output,
                }
            )

    full_transcript = "\n".join(transcript_parts)
    firmware_version = "unknown"
    version_match = re.search(r"(?:version|library version):\s*([^\r\n]+)", full_transcript, re.IGNORECASE)
    if version_match:
        firmware_version = version_match.group(1).strip()
    return results, full_transcript, firmware_version


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
            commands.append(f"stress {args.stress_count}")
        else:
            skipped.append(
                {
                    "name": "stress",
                    "reason": "IDF diagnostic CLI has no stress command",
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


def overall_status(command_results: List[Dict[str, object]], not_run_reason: Optional[str]) -> str:
    if not_run_reason:
        return "NOT_RUN"
    if not command_results:
        return "NOT_RUN"
    if any(result["status"] == "FAIL" for result in command_results):
        return "FAIL"
    if any(result["status"] == "REVIEW" for result in command_results):
        return "REVIEW"
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

    if not args.port:
        not_run_reason = "serial port was not supplied"
    else:
        try:
            command_results, transcript, firmware_version = run_serial_commands(args, commands)
        except Exception as exc:
            not_run_reason = str(exc)

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
        "commands": commands,
        "command_results": command_results,
        "skipped_optional_tests": skipped,
        "not_run_reason": not_run_reason,
        "transcript": transcript,
    }
    result["overall_status"] = overall_status(command_results, not_run_reason)
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
    ]
    if result.get("not_run_reason"):
        lines.append(f"Not-run reason: `{result['not_run_reason']}`")
    if result.get("notes"):
        lines.append(f"Notes: {result['notes']}")

    lines.extend(["", "## Commands", "", "| # | Command | Status | Reason |", "| ---: | --- | --- | --- |"])
    for item in result["command_results"]:
        lines.append(
            f"| {item['index']} | `{item['command']}` | `{item['status']}` | {item['reason']} |"
        )
    if not result["command_results"]:
        lines.append("| - | - | `NOT_RUN` | No serial command transcript captured |")

    lines.extend(["", "## Skipped Optional Tests", "", "| Test | Reason |", "| --- | --- |"])
    skipped = result["skipped_optional_tests"]
    if skipped:
        for item in skipped:
            lines.append(f"| `{item['name']}` | {item['reason']} |")
    else:
        lines.append("| - | No optional tests requested |")

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
    parser.add_argument("--json-out", default="")
    parser.add_argument("--markdown-out", default="")
    parser.add_argument("--quiet", action="store_true")
    parser.add_argument("--require-run", action="store_true", help="Exit nonzero when result is NOT_RUN")
    return parser.parse_args(argv)


def main(argv: List[str]) -> int:
    args = parse_args(argv)
    result = make_result(args)
    write_outputs(args, result)

    if args.require_run and result["overall_status"] == "NOT_RUN":
        return 2
    if result["overall_status"] == "FAIL":
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
