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
    idf_cmake = ROOT / "examples" / "esp_idf" / "basic" / "main" / "CMakeLists.txt"
    idf_cli = ROOT / "examples" / "esp_idf" / "basic" / "main" / "Ldc1614IdfCli.cpp"
    shared_cli = common_dir / "Ldc1614Cli.cpp"
    transport = common_dir / "I2cTransport.h"
    version_script = ROOT / "scripts" / "generate_version.py"
    gitignore = ROOT / ".gitignore"

    ensure_exists(common_dir, "common example directory")
    ensure_exists(bringup_main, "bringup CLI example")
    ensure_exists(idf_main, "ESP-IDF CLI example")
    ensure_exists(idf_cmake, "ESP-IDF example build definition")
    ensure_exists(idf_cli, "ESP-IDF diagnostic CLI implementation")
    ensure_exists(shared_cli, "shared CLI implementation")
    ensure_exists(transport, "Arduino example transport")
    ensure_exists(version_script, "version generator")
    ensure_exists(gitignore, "Git ignore rules")

    ensure_missing(ROOT / "examples" / "00_smoke_boot", "deprecated example 00_smoke_boot")
    ensure_missing(
        ROOT / "examples" / "03_feature_walkthrough",
        "deprecated example 03_feature_walkthrough",
    )

    for name in REQUIRED_COMMON:
        ensure_exists(common_dir / name, f"common helper {name}")

    arduino_text = bringup_main.read_text(encoding="utf-8", errors="replace")
    idf_text = idf_main.read_text(encoding="utf-8", errors="replace")
    idf_cmake_text = idf_cmake.read_text(encoding="utf-8", errors="replace")
    idf_cli_text = idf_cli.read_text(encoding="utf-8", errors="replace")
    text = shared_cli.read_text(encoding="utf-8", errors="replace")
    transport_text = transport.read_text(encoding="utf-8", errors="replace")
    version_text = version_script.read_text(encoding="utf-8", errors="replace")
    gitignore_text = gitignore.read_text(encoding="utf-8", errors="replace")

    if "Ldc1614Cli.h" not in arduino_text:
        fail("Arduino example must use shared Ldc1614Cli.h")
    if "Ldc1614Cli.h" in idf_text:
        fail("ESP-IDF example must use native fixed-buffer CLI, not shared Ldc1614Cli.h")
    if "I2cProbeResult::NACK" not in arduino_text:
        fail("Arduino scan adapter must distinguish normal address NACK")
    if "scan complete found=" not in text or "I2C scan probe failed" not in text:
        fail("Arduino scan must expose bounded completion and transport failure")
    if (
        "esp_timer_get_time()" not in transport_text
        or "wire->setTimeOut(remainingMs);" not in transport_text
    ):
        fail("Arduino combined write-read must share one callback timeout budget")

    for source_name, source_text in (
        ("PlatformIO version generator", version_text),
        ("ESP-IDF build definition", idf_cmake_text),
    ):
        if "--untracked-files=all" not in source_text or "--untracked-files=no" in source_text:
            fail(f"{source_name} must include untracked source in firmware cleanliness")
    if "status_result.returncode != 0" not in version_text:
        fail("PlatformIO firmware status must not report clean when git status fails")
    no_args = version_text.find("if not args:")
    sync = version_text.find("_sync_outputs(project_root", no_args)
    identity = version_text.find("_append_build_metadata_defines(namespace, project_root)", no_args)
    if no_args < 0 or sync < 0 or identity < 0 or sync > identity:
        fail("PlatformIO must synchronize generated headers before capturing build identity")
    if "LDC1614_GIT_STATUS_RESULT" not in idf_cmake_text:
        fail("ESP-IDF firmware status must not report clean when git status fails")
    if "examples/esp_idf/basic/build/" not in gitignore_text:
        fail("generated ESP-IDF build output must not dirty firmware source identity")

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
