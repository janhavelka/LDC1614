#!/usr/bin/env python3
from __future__ import annotations

import pathlib
import re
import sys

from ldc1614_cli_contract import (
    ANSI,
    COMMAND_SPECS,
    STALE_COMMANDS,
    compare_cpp_command_specs,
    parse_cpp_command_specs,
    validate_contract,
)

ROOT = pathlib.Path(__file__).resolve().parents[1]

REQUIRED_IDF_TOKENS = (
    '#include "driver/i2c_master.h"', "i2c_new_master_bus",
    "i2c_master_bus_add_device", "i2c_master_probe",
    "i2c_master_bus_rm_device", "i2c_del_master_bus", "esp32_i2c::recover",
    "esp32_i2c::recover(*context)",
    "Ldc1614IdfCli", "esp32_i2c::write", "esp32_i2c::writeRead",
    "esp32_i2c::intbAsserted", "esp32_i2c::uptimeMs", ".bind(",
    ".startInitialize(", ".poll(", ".takeResult(", "select(",
    "readConsoleChar", "GPIO_PULLUP_DISABLE", "push-pull", "ErrorReporting::all",
)
FORBIDDEN_IDF_PATTERNS = (
    (r'#\s*include\s*[<"]Arduino\.h[>"]', "Arduino.h"),
    (r'#\s*include\s*[<"]Wire\.h[>"]', "Wire.h"),
    (r'#\s*include\s*<string>', "<string>"),
    (r"\bstd::string\b", "std::string"), (r"\bString\b", "Arduino String"),
    (r"\bSerial\b", "Serial"), (r"\bTwoWire\b", "TwoWire"),
    (r"\bArduinoCompat\b", "ArduinoCompat"),
    (r"\bIdfArduinoCompat\b", "IdfArduinoCompat"),
    (r'#\s*include\s*[<"]driver/i2c\.h[>"]', "legacy driver/i2c.h"),
    (r"\bi2c_cmd_link\b", "legacy i2c_cmd_link"),
    (r"\bi2c_driver_install\b", "legacy i2c_driver_install"),
    (r"\bgetchar\s*\(", "blocking getchar"),
    (r"\bESP_ERROR_CHECK\s*\(", "abort-style ESP_ERROR_CHECK"),
    (r"\bmillis\s*\(", "Arduino-like millis"),
    (r"\bdelay\s*\(", "Arduino-like delay"),
    (r"\byield\s*\(", "Arduino-like yield"),
)


def fail(message: str) -> None:
    print(f"IDF example contract FAILED: {message}")
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
    keys = {"SRCS", "INCLUDE_DIRS", "PRIV_INCLUDE_DIRS", "REQUIRES", "PRIV_REQUIRES", "EMBED_FILES", "EMBED_TXTFILES"}
    parsed: dict[str, list[pathlib.Path]] = {key: [] for key in ("SRCS", "INCLUDE_DIRS", "PRIV_INCLUDE_DIRS")}
    active: str | None = None
    for raw in re.findall(r'"[^"]+"|\S+', match.group(1)):
        token = raw.strip().strip('"')
        if token in keys:
            active = token if token in parsed else None
        elif active is not None:
            parsed[active].append((cmake_path.parent / token).resolve())
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


def require(source: str, pattern: str, message: str) -> None:
    if re.search(pattern, source) is None:
        fail(message)


def require_ansi(source: str, name: str, value: str) -> None:
    numeric = value.removeprefix("\x1b")
    if re.search(rf"(?:\\x1[bB]|\\033){re.escape(numeric)}", source) is None:
        fail(f"native CLI missing exact ANSI {name} sequence {value!r}")


def check_native_cli(source: str) -> None:
    errors = compare_cpp_command_specs(source, "native ESP-IDF CLI")
    if errors:
        fail("\n  - ".join(("firmware command-table parity errors:", *errors)))
    if source.count("COMMAND_SPECS") < 2:
        fail("native COMMAND_SPECS must drive lookup/help")
    require(source, r"\bswitch\s*\([^)]*(?:(?:\.|->)id|CommandId)",
            "native dispatch must switch on a table-resolved CommandId")
    for stale in STALE_COMMANDS:
        if re.search(rf'"{re.escape(stale)}"', source):
            fail(f"stale native command token remains: {stale}")
    for color, escape in ANSI.items():
        require_ansi(source, color, escape)
    require(source, r"\bcolorEnabled\b|\b_colorEnabled\b",
            "native CLI must allow ANSI colors to be disabled")
    require(source, r"\b(?:requireConfirm(?:ation)?|isConfirm(?:ed)?)\b",
            "native mutations/destructive reads need confirmation")
    require(source, r"\bparseUnsigned\b", "native CLI must use bounded numeric parsing")
    require(source, r"end\s*==\s*text|end\s*==\s*token",
            "native numeric parsing must reject an empty conversion")
    require(source, r"\*end\s*!=\s*'\\0'", "native numeric parsing must reject trailing input")
    require(source, r"parsed\s*>\s*maxValue|maxValue\s*<\s*parsed",
            "native numeric parsing must enforce upper bounds")
    require(source, r"char\s+\w+\s*\[\s*(?:64|96|128|160|192|256)\s*\]",
            "native CLI must use fixed-capacity input")
    require(source, r"\b(?:rejectExtraArguments|noMoreArguments|requireEnd)\b",
            "native CLI must reject unexpected extra arguments")
    require(source, r"OperationOwner::COMMAND", "native CLI must identify core command ownership")
    require(source, r"OperationOwner::SESSION", "native CLI must identify CLI session ownership")
    require(source, r"SessionKind::SELFTEST|SessionKind::SELF_TEST",
            "native CLI must expose a distinct selftest session")
    require(source, r"\.poll\s*\([^,]+,\s*1U?\s*\)",
            "native service must advance the core with poll(now, 1)")
    for envelope in ("CLI scheduled:", "CLI result:", "CLI preview:"):
        if envelope not in source:
            fail(f"native CLI missing stable envelope: {envelope}")
    for field in (
        "address=", "variant=", "variant_channels=", "selected=", "mode=",
        "ref_source=", "ref_hz=", "tolerance_ppm=", "deglitch=",
        "activation=", "timeout_ms=", "rp_override=", "auto_amplitude=",
        "high_current=", "intb_config=", "error_reporting=", "revision=",
        "applied=", "active=", "session=", "operation=", "kind=", "phase=",
        "transfers=", "requested=", "completed=", "deadline_ms=", "outcome=",
        "effects=", "reg=", "channel=", "code=", "detail=", "msg=",
        "valid=", "fresh=", "error=", "overrun=", "completed_ms=",
    ):
        if field not in source:
            fail(f"native CLI missing required cfg/progress/result/batch field {field!r}")
    for label in ("status_before", "status_after"):
        require(
            source,
            rf'printDeviceStatus\s*\([^;]*"{label}"\s*\)',
            f"native batch output must expose {label}= evidence",
        )
    ids = {spec.canonical: spec.command_id for spec in parse_cpp_command_specs(source)}
    if ids.get("probe") == ids.get("selftest"):
        fail("native probe and selftest must be distinct")


def main() -> int:
    contract_errors = validate_contract()
    if contract_errors:
        fail("invalid host contract: " + "; ".join(contract_errors))

    main_dir = (ROOT / "examples" / "esp_idf" / "basic" / "main").resolve()
    idf_main = main_dir / "main.cpp"
    idf_cmake = main_dir / "CMakeLists.txt"
    idf_cli = main_dir / "Ldc1614IdfCli.cpp"
    common = (ROOT / "examples" / "common").resolve()
    arduino = (ROOT / "examples" / "01_basic_bringup_cli").resolve()
    transport_dir = (ROOT / "examples" / "esp32").resolve()
    transport_source = transport_dir / "I2cMasterTransport.cpp"
    transport_header = transport_dir / "I2cMasterTransport.h"

    cmake_text = read(idf_cmake)
    cmake_lists = parse_idf_register_lists(idf_cmake)
    compiled = cmake_lists["SRCS"]
    for path in compiled:
        if not path_is_within(path, main_dir) and path != transport_source:
            fail(f"compiled source escapes native main directory: {rel(path)}")
        if path_is_within(path, common) or path_is_within(path, arduino):
            fail(f"native build must not compile Arduino/common source: {rel(path)}")
    for key in ("INCLUDE_DIRS", "PRIV_INCLUDE_DIRS"):
        for path in cmake_lists[key]:
            if (path_is_within(path, common) or path_is_within(path, arduino)) and path != transport_dir:
                fail(f"native {key} must not include Arduino/common path: {rel(path)}")

    headers = sorted(set(main_dir.glob("*.h")) | set(main_dir.glob("*.hpp")))
    scan_paths = compiled + headers + [transport_header]
    for path in scan_paths:
        source = read(path)
        for pattern, label in FORBIDDEN_IDF_PATTERNS:
            if re.search(pattern, source):
                fail(f"forbidden native token in {rel(path)}: {label}")
    all_native = "\n".join(read(path) for path in scan_paths) + "\n" + cmake_text
    for token in REQUIRED_IDF_TOKENS:
        if token not in all_native:
            fail(f"required native token missing: {token}")
    if "../../../common/Ldc1614Cli.cpp" in cmake_text or "Ldc1614Cli.h" in all_native:
        fail("native example must not compile/include the Arduino/common CLI")

    expected = {
        "examples/esp_idf/basic/main/main.cpp",
        "examples/esp32/I2cMasterTransport.cpp",
        "examples/esp_idf/basic/main/Ldc1614IdfCli.cpp",
    }
    missing = sorted(expected - {rel(path) for path in compiled})
    if missing:
        fail("CMake missing expected native sources: " + ", ".join(missing))
    for stale in ("ArduinoCompat.cpp", "Arduino.h", "Wire.h", "IdfArduinoCompat.h"):
        if (main_dir / stale).exists():
            fail(f"stale compatibility file remains: {stale}")

    check_native_cli(all_native)
    arduino_specs = parse_cpp_command_specs(read(common / "Ldc1614Cli.cpp"))
    native_specs = parse_cpp_command_specs(read(idf_cli))
    if len(arduino_specs) != len(native_specs):
        fail("Arduino/native command tables differ in length")
    for arduino_spec, native_spec in zip(arduino_specs, native_specs):
        if arduino_spec.description != native_spec.description:
            fail(
                "Arduino/native help description differs for "
                f"{arduino_spec.canonical!r}"
            )
    print(f"IDF example contract PASSED ({len(COMMAND_SPECS)} commands)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
