#!/usr/bin/env python3
from __future__ import annotations

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]

MANDATORY_COMMANDS = [
    "help",
    "version",
    "scan",
    "read",
    "readblocking",
    "sample",
    "sampleage",
    "drdy",
    "status",
    "status_raw",
    "freq",
    "timing",
    "init",
    "begin",
    "end",
    "sleep",
    "wake",
    "reset",
    "resetreapply",
    "cfg",
    "settings",
    "snapshot",
    "channels",
    "activech",
    "single",
    "autoscan",
    "deglitch",
    "errcfg",
    "intb",
    "refclk",
    "activate",
    "rpoverride",
    "autoamp",
    "highcurrent",
    "rcount",
    "settle",
    "clkdiv",
    "offset",
    "idrive",
    "initidrive",
    "reg",
    "wreg",
    "drv",
    "online",
    "id",
    "probe",
    "recover",
    "verbose",
    "stress",
    "stress_mix",
    "demo",
    "selftest",
]

REQUIRED_IDF_TOKENS = [
    '#include "driver/i2c_master.h"',
    "i2c_new_master_bus",
    "i2c_master_bus_add_device",
    "i2c_master_probe",
    "Ldc1614Cli.h",
    "ldc1614IdfI2cWrite",
    "ldc1614IdfI2cWriteRead",
    "ldc1614IdfBusReset",
    "ldc1614IdfNowMs",
    "ldc1614IdfYield",
    "MASK_ERRCFG_DRDY_2INT",
    "MASK_ERRCFG_UR_ERR2INT",
    "MASK_ERRCFG_OR_ERR2INT",
    "MASK_ERRCFG_WD_ERR2INT",
    "MASK_ERRCFG_AH_ERR2INT",
    "MASK_ERRCFG_AL_ERR2INT",
]

FORBIDDEN_IDF_TOKENS = [
    "driver/" + "i2c.h",
    "i2c_cmd_link",
    "i2c_driver_install",
    "minimal " + "IDF",
    "minimal " + "ESP-IDF",
]


def fail(msg: str) -> None:
    print(f"IDF example contract FAILED: {msg}")
    raise SystemExit(1)


def read(path: pathlib.Path) -> str:
    if not path.exists():
        fail(f"missing file: {path.as_posix()}")
    return path.read_text(encoding="utf-8", errors="replace")


def main() -> int:
    shared_cli = ROOT / "examples" / "common" / "Ldc1614Cli.cpp"
    idf_main = ROOT / "examples" / "esp_idf" / "basic" / "main" / "main.cpp"
    idf_cmake = ROOT / "examples" / "esp_idf" / "basic" / "main" / "CMakeLists.txt"
    idf_transport = (
        ROOT / "examples" / "esp_idf" / "basic" / "main" / "Ldc1614IdfI2cTransport.cpp"
    )

    shared_text = read(shared_cli)
    idf_text = read(idf_main)
    cmake_text = read(idf_cmake)
    transport_text = read(idf_transport)
    idf_all = "\n".join([idf_text, cmake_text, transport_text])

    for command in MANDATORY_COMMANDS:
        if re.search(rf"\b{re.escape(command)}\b", shared_text) is None:
            fail(f"mandatory shared command missing: {command}")

    for token in REQUIRED_IDF_TOKENS:
        if token not in idf_all:
            fail(f"required ESP-IDF token missing: {token}")

    if "../../../common/Ldc1614Cli.cpp" not in cmake_text:
        fail("ESP-IDF CMake must compile the shared CLI implementation")

    for token in FORBIDDEN_IDF_TOKENS:
        if token in idf_all:
            fail(f"forbidden ESP-IDF token present: {token}")

    print("IDF example contract PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
