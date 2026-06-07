#!/usr/bin/env python3
from __future__ import annotations

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]

IDF_COMMANDS = [
    "help",
    "version",
    "probe",
    "status",
    "drv",
    "cfg",
    "read",
    "readall",
    "ready",
    "recover",
    "timing",
    "selftest",
]

REQUIRED_IDF_TOKENS = [
    '#include "driver/i2c_master.h"',
    "i2c_new_master_bus",
    "i2c_master_bus_add_device",
    "Ldc1614IdfCli",
    "ldc1614IdfI2cWrite",
    "ldc1614IdfI2cWriteRead",
    "ldc1614IdfBusReset",
    "ldc1614IdfNowMs",
    "ldc1614IdfYield",
    "xSemaphoreCreateMutex",
    "xSemaphoreTake",
    "xSemaphoreGive",
    "select(",
    "readConsoleChar",
    "GPIO_PULLUP_DISABLE",
    "push-pull",
    "MASK_ERRCFG_DRDY_2INT",
    "MASK_ERRCFG_UR_ERR2INT",
    "MASK_ERRCFG_OR_ERR2INT",
    "MASK_ERRCFG_WD_ERR2INT",
    "MASK_ERRCFG_AH_ERR2INT",
    "MASK_ERRCFG_AL_ERR2INT",
]

FORBIDDEN_IDF_PATTERNS = [
    (re.compile(r'#\s*include\s*[<"]Arduino\.h[>"]'), "Arduino.h"),
    (re.compile(r'#\s*include\s*[<"]Wire\.h[>"]'), "Wire.h"),
    (re.compile(r'#\s*include\s*<string>'), "<string>"),
    (re.compile(r"\bstd::string\b"), "std::string"),
    (re.compile(r"\bString\b"), "Arduino String"),
    (re.compile(r"\bSerial\b"), "Serial"),
    (re.compile(r"\bTwoWire\b"), "TwoWire"),
    (re.compile(r"\bArduinoCompat\b"), "ArduinoCompat"),
    (re.compile(r"\bIdfArduinoCompat\b"), "IdfArduinoCompat"),
    (re.compile(r'#\s*include\s*[<"]driver/i2c\.h[>"]'), "legacy driver/i2c.h"),
    (re.compile(r"\bi2c_cmd_link\b"), "legacy i2c_cmd_link"),
    (re.compile(r"\bi2c_driver_install\b"), "legacy i2c_driver_install"),
    (re.compile(r"\bgetchar\s*\("), "blocking getchar"),
    (re.compile(r"\bESP_ERROR_CHECK\s*\("), "abort-style ESP_ERROR_CHECK"),
    (re.compile(r"\bmillis\s*\("), "Arduino-like millis"),
    (re.compile(r"\bdelay\s*\("), "Arduino-like delay"),
    (re.compile(r"\byield\s*\("), "Arduino-like yield"),
]


def fail(msg: str) -> None:
    print(f"IDF example contract FAILED: {msg}")
    raise SystemExit(1)


def read(path: pathlib.Path) -> str:
    if not path.exists():
        fail(f"missing file: {path.as_posix()}")
    return path.read_text(encoding="utf-8", errors="replace")


def parse_idf_register_lists(cmake_path: pathlib.Path) -> dict[str, list[pathlib.Path]]:
    text = read(cmake_path)
    match = re.search(r"idf_component_register\s*\((.*?)\)", text, re.S)
    if not match:
        fail("missing idf_component_register in ESP-IDF main CMakeLists.txt")

    body = match.group(1)
    keys = {
        "SRCS",
        "INCLUDE_DIRS",
        "PRIV_INCLUDE_DIRS",
        "INCLUDE_DIRS",
        "REQUIRES",
        "PRIV_REQUIRES",
        "EMBED_FILES",
        "EMBED_TXTFILES",
    }
    path_keys = {"SRCS", "INCLUDE_DIRS", "PRIV_INCLUDE_DIRS"}
    parsed: dict[str, list[pathlib.Path]] = {key: [] for key in path_keys}
    active_key: str | None = None
    for raw in re.findall(r'"[^"]+"|\S+', body):
        token = raw.strip().strip('"')
        if token in keys:
            active_key = token if token in path_keys else None
            continue
        if active_key is not None:
            parsed[active_key].append((cmake_path.parent / token).resolve())

    if not parsed["SRCS"]:
        fail("ESP-IDF CMake SRCS list is empty")
    return parsed


def path_is_within(path: pathlib.Path, parent: pathlib.Path) -> bool:
    try:
        path.resolve().relative_to(parent.resolve())
        return True
    except ValueError:
        return False


def rel(path: pathlib.Path) -> str:
    return path.resolve().relative_to(ROOT).as_posix()


def main() -> int:
    idf_main = ROOT / "examples" / "esp_idf" / "basic" / "main" / "main.cpp"
    idf_cmake = ROOT / "examples" / "esp_idf" / "basic" / "main" / "CMakeLists.txt"
    idf_cli = ROOT / "examples" / "esp_idf" / "basic" / "main" / "Ldc1614IdfCli.cpp"
    idf_main_dir = idf_main.parent.resolve()
    examples_common = (ROOT / "examples" / "common").resolve()
    arduino_bringup = (ROOT / "examples" / "01_basic_bringup_cli").resolve()

    cmake_text = read(idf_cmake)
    cmake_lists = parse_idf_register_lists(idf_cmake)
    compiled_paths = cmake_lists["SRCS"]
    for path in compiled_paths:
        if not path_is_within(path, idf_main_dir):
            fail(f"ESP-IDF compiled source must stay under examples/esp_idf/basic/main: {rel(path)}")
        if path_is_within(path, examples_common) or path_is_within(path, arduino_bringup):
            fail(f"ESP-IDF compiled source must not use Arduino/common example path: {rel(path)}")

    for key in ("INCLUDE_DIRS", "PRIV_INCLUDE_DIRS"):
        for path in cmake_lists[key]:
            if path_is_within(path, examples_common) or path_is_within(path, arduino_bringup):
                fail(f"ESP-IDF {key} must not include Arduino/common example path: {rel(path)}")

    header_paths = sorted(set(idf_main.parent.glob("*.h")) | set(idf_main.parent.glob("*.hpp")))
    scan_paths = compiled_paths + header_paths

    for path in scan_paths:
        if not path.exists():
            fail(f"compiled ESP-IDF path missing: {path.as_posix()}")
        path_text = read(path)
        for pattern, label in FORBIDDEN_IDF_PATTERNS:
            if pattern.search(path_text):
                fail(f"forbidden ESP-IDF token present in {rel(path)}: {label}")

    idf_all = "\n".join(read(path) for path in scan_paths) + "\n" + cmake_text

    for token in REQUIRED_IDF_TOKENS:
        if token not in idf_all:
            fail(f"required ESP-IDF token missing: {token}")

    if "../../../common/Ldc1614Cli.cpp" in cmake_text or "Ldc1614Cli.h" in idf_all:
        fail("ESP-IDF example must not compile or include the shared Arduino/common CLI")

    compiled_rels = {rel(path) for path in compiled_paths}
    expected_sources = {
        "examples/esp_idf/basic/main/main.cpp",
        "examples/esp_idf/basic/main/Ldc1614IdfI2cTransport.cpp",
        "examples/esp_idf/basic/main/Ldc1614IdfCli.cpp",
    }
    missing_sources = sorted(expected_sources - compiled_rels)
    if missing_sources:
        fail(f"ESP-IDF CMake missing expected sources: {', '.join(missing_sources)}")

    for command in IDF_COMMANDS:
        if re.search(rf"\b{re.escape(command)}\b", read(idf_cli)) is None:
            fail(f"mandatory IDF diagnostic command missing: {command}")

    for stale in ("ArduinoCompat.cpp", "Arduino.h", "Wire.h", "IdfArduinoCompat.h"):
        if (idf_main.parent / stale).exists():
            fail(f"stale compatibility file remains: {stale}")

    print("IDF example contract PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
