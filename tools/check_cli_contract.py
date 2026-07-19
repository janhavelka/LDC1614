#!/usr/bin/env python3
from __future__ import annotations

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]

REQUIRED_COMMON = [
    "BoardConfig.h",
    "I2cTransport.h",
    "Ldc1614Cli.h",
    "Ldc1614Cli.cpp",
]

MANDATORY_COMMANDS = [
    "help",
    "version",
    "scan",
    "probe",
    "bind",
    "init",
    "apply",
    "resetreapply",
    "acquire",
    "cancel",
    "progress",
    "drv",
    "cfg",
    "status",
    "drdy",
    "sleep",
    "wake",
    "reg",
    "wreg",
    "invalidate",
    "timing",
    "freq",
    "selftest",
]

IDF_DIAGNOSTIC_COMMANDS = [
    "help",
    "version",
    "probe",
    "bind",
    "init",
    "apply",
    "resetreapply",
    "acquire",
    "cancel",
    "progress",
    "status",
    "drv",
    "cfg",
    "read",
    "ready",
    "sleep",
    "wake",
    "invalidate",
    "timing",
    "freq",
    "selftest",
]


def fail(msg: str) -> None:
    print(f"CLI contract FAILED: {msg}")
    raise SystemExit(1)


def ensure_exists(path: pathlib.Path, label: str) -> None:
    if not path.exists():
        fail(f"missing {label}: {path.as_posix()}")


def ensure_missing(path: pathlib.Path, label: str) -> None:
    if path.exists():
        fail(f"forbidden {label} still present: {path.as_posix()}")


def main() -> int:
    common_dir = ROOT / "examples" / "common"
    bringup_main = ROOT / "examples" / "01_basic_bringup_cli" / "main.cpp"
    idf_main = ROOT / "examples" / "esp_idf" / "basic" / "main" / "main.cpp"
    idf_cli = ROOT / "examples" / "esp_idf" / "basic" / "main" / "Ldc1614IdfCli.cpp"
    shared_cli = common_dir / "Ldc1614Cli.cpp"

    ensure_exists(common_dir, "common example directory")
    ensure_exists(bringup_main, "bringup CLI example")
    ensure_exists(idf_main, "ESP-IDF CLI example")
    ensure_exists(idf_cli, "ESP-IDF diagnostic CLI implementation")
    ensure_exists(shared_cli, "shared CLI implementation")

    ensure_missing(ROOT / "examples" / "00_smoke_boot", "deprecated example 00_smoke_boot")
    ensure_missing(
        ROOT / "examples" / "03_feature_walkthrough",
        "deprecated example 03_feature_walkthrough",
    )

    for name in REQUIRED_COMMON:
        ensure_exists(common_dir / name, f"common helper {name}")

    arduino_text = bringup_main.read_text(encoding="utf-8", errors="replace")
    idf_text = idf_main.read_text(encoding="utf-8", errors="replace")
    idf_cli_text = idf_cli.read_text(encoding="utf-8", errors="replace")
    text = shared_cli.read_text(encoding="utf-8", errors="replace")

    if "Ldc1614Cli.h" not in arduino_text:
        fail("Arduino example must use shared Ldc1614Cli.h")
    if "Ldc1614Cli.h" in idf_text:
        fail("ESP-IDF example must use native fixed-buffer CLI, not shared Ldc1614Cli.h")
    if "I2cProbeResult::NACK" not in arduino_text:
        fail("Arduino scan adapter must distinguish normal address NACK")
    if "scan complete found=" not in text or "I2C scan probe failed" not in text:
        fail("Arduino scan must expose bounded completion and transport failure")

    for cmd in MANDATORY_COMMANDS:
        if re.search(rf"\b{re.escape(cmd)}\b", text) is None:
            fail(f"mandatory command '{cmd}' missing in {shared_cli.as_posix()}")

    for cmd in IDF_DIAGNOSTIC_COMMANDS:
        if re.search(rf"\b{re.escape(cmd)}\b", idf_cli_text) is None:
            fail(f"mandatory IDF diagnostic command '{cmd}' missing in {idf_cli.as_posix()}")

    if re.search(r"\bcfg\b", text) is None and re.search(r"\bsettings\b", text) is None:
        fail("either 'cfg' or 'settings' command must be present")

    print("CLI contract PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
