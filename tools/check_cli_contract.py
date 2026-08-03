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
REQUIRED_COMMON = ("BoardConfig.h", "Ldc1614Cli.h", "Ldc1614Cli.cpp")


def fail(message: str) -> None:
    print(f"CLI contract FAILED: {message}")
    raise SystemExit(1)


def read(path: pathlib.Path, label: str) -> str:
    if not path.exists():
        fail(f"missing {label}: {path.as_posix()}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(source: str, pattern: str, message: str, flags: int = 0) -> None:
    if re.search(pattern, source, flags) is None:
        fail(message)


def require_ansi(source: str, name: str, value: str) -> None:
    numeric = value.removeprefix("\x1b")
    encoded = rf"(?:\\x1[bB]|\\033){re.escape(numeric)}"
    if re.search(encoded, source) is None:
        fail(f"Arduino CLI missing exact ANSI {name} sequence {value!r}")


def require_bounded_unsigned_parser(source: str) -> None:
    parser = re.search(
        r"\bbool\s+parseUnsigned\s*\(\s*const\s+char\s*\*\s*\w+\s*,\s*"
        r"uint64_t\s+(?P<bound>[A-Za-z_]\w*)\s*,[^)]*\)\s*\{"
        r"(?P<body>.*?)\n\}",
        source,
        re.DOTALL,
    )
    if parser is None:
        fail("Arduino CLI must define bounded numeric parsing")

    bound = re.escape(parser.group("bound"))
    if re.search(
        rf"\bparsed\s*>\s*{bound}\b|\b{bound}\s*<\s*parsed\b",
        parser.group("body"),
    ) is None:
        fail("Arduino numeric parsing must enforce its caller-provided upper bound")


def check_table_and_behavior(source: str) -> None:
    errors = compare_cpp_command_specs(source, "Arduino CLI")
    if errors:
        fail("\n  - ".join(("firmware command-table parity errors:", *errors)))

    if source.count("COMMAND_SPECS") < 2:
        fail("Arduino COMMAND_SPECS must drive lookup/help, not be an unused declaration")
    require(
        source,
        r"(?:\bswitch\s*\([^)]*(?:(?:\.|->)id|CommandId)|"
        r"\bhandleCommand\s*\(\s*id\s*,)",
        "Arduino dispatch must use the table-resolved CommandId",
    )
    require(source, r"for\s*\([^)]*COMMAND_SPECS|COMMAND_SPECS\s*\[",
            "Arduino help/lookup must iterate the command table")

    for stale in STALE_COMMANDS:
        if re.search(rf'"{re.escape(stale)}"', source):
            fail(f"stale Arduino command token remains: {stale}")

    for color, escape in ANSI.items():
        require_ansi(source, color, escape)
    require(source, r"\bcolorEnabled\b|\b_colorEnabled\b",
            "Arduino CLI must allow ANSI colors to be disabled")

    require(source, r"\b(?:requireConfirm(?:ation)?|isConfirm(?:ed)?)\b",
            "Arduino mutations/destructive reads need an explicit confirmation guard")
    require_bounded_unsigned_parser(source)
    require(source, r"end\s*==\s*text|end\s*==\s*token",
            "Arduino numeric parsing must reject an empty conversion")
    require(source, r"\*end\s*!=\s*'\\0'",
            "Arduino numeric parsing must reject trailing characters")
    require(source, r"char\s+\w+\s*\[\s*(?:(?:64|96|128|160|192|256)|MAX_LINE_LENGTH\s*\+\s*1U?)\s*\]",
            "Arduino CLI input must remain fixed-capacity")
    require(source, r"\b(?:rejectExtraArguments|noMoreArguments|requireEnd)\b",
            "Arduino CLI must reject unexpected extra arguments")

    require(source, r"OperationOwner::COMMAND", "Arduino CLI must identify core command ownership")
    require(source, r"OperationOwner::SESSION", "Arduino CLI must identify CLI session ownership")
    require(source, r"SessionKind::SELFTEST|SessionKind::SELF_TEST",
            "Arduino CLI must expose a distinct selftest session")
    require(source, r"\.poll\s*\([^,]+,\s*1U?\s*\)",
            "Arduino cooperative service must call poll(now, 1)")
    for envelope in ("CLI scheduled:", "CLI result:", "CLI preview:"):
        if envelope not in source:
            fail(f"Arduino CLI missing stable envelope: {envelope}")

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
            fail(f"Arduino CLI missing required cfg/progress/result/batch field {field!r}")
    for label in ("status_before", "status_after"):
        require(
            source,
            rf'printDeviceStatus\s*\([^;]*"{label}"\s*\)',
            f"Arduino batch output must expose {label}= evidence",
        )

    parsed_ids = {spec.canonical: spec.command_id for spec in parse_cpp_command_specs(source)}
    if parsed_ids.get("probe") == parsed_ids.get("selftest"):
        fail("Arduino probe and selftest must have distinct command IDs/implementations")


def main() -> int:
    contract_errors = validate_contract()
    if contract_errors:
        fail("invalid host contract: " + "; ".join(contract_errors))

    common_dir = ROOT / "examples" / "common"
    bringup_main = ROOT / "examples" / "01_basic_bringup_cli" / "main.cpp"
    idf_main = ROOT / "examples" / "esp_idf" / "basic" / "main" / "main.cpp"
    idf_cmake = ROOT / "examples" / "esp_idf" / "basic" / "main" / "CMakeLists.txt"
    shared_cli = common_dir / "Ldc1614Cli.cpp"
    transport = ROOT / "examples" / "esp32" / "I2cMasterTransport.cpp"
    version_script = ROOT / "scripts" / "generate_version.py"
    gitignore = ROOT / ".gitignore"

    for name in REQUIRED_COMMON:
        read(common_dir / name, f"common helper {name}")
    arduino_text = read(bringup_main, "bringup CLI example")
    idf_text = read(idf_main, "ESP-IDF CLI example")
    idf_cmake_text = read(idf_cmake, "ESP-IDF example build definition")
    cli_text = read(shared_cli, "shared Arduino CLI implementation")
    cli_header_text = read(common_dir / "Ldc1614Cli.h", "shared Arduino CLI header")
    transport_text = read(transport, "shared ESP32 example transport")
    version_text = read(version_script, "version generator")
    gitignore_text = read(gitignore, "Git ignore rules")

    for deprecated in ("00_smoke_boot", "03_feature_walkthrough"):
        if (ROOT / "examples" / deprecated).exists():
            fail(f"forbidden deprecated example remains: examples/{deprecated}")

    if "Ldc1614Cli.h" not in arduino_text:
        fail("Arduino example must use shared Ldc1614Cli.h")
    if re.search(r"\b(?:Wire|TwoWire)\b", arduino_text):
        fail("Arduino example must use the explicit ESP-IDF new-master owner, not Wire")
    if "esp32_i2c::writeRead" not in arduino_text:
        fail("Arduino example must inject the shared ESP32 new-master transport")
    if "Ldc1614Cli.h" in idf_text:
        fail("ESP-IDF example must use its native fixed-buffer CLI")
    if "I2cProbeResult::NACK" not in arduino_text:
        fail("Arduino scan adapter must distinguish normal address NACK")
    if "scan complete found=" not in cli_text or "I2C scan probe failed" not in cli_text:
        fail("Arduino scan must expose bounded completion and transport failure")
    if "i2cRecover" not in cli_text or "Owner reinitialized I2C bus" not in cli_text:
        fail("Arduino CLI must expose explicit owner-controlled bus recovery")
    if "i2c_master_transmit_receive" not in transport_text or "clampTimeoutMs(timeoutMs)" not in transport_text:
        fail("combined write-read must use one bounded new-master transaction")
    for token in (
        "i2c_master_bus_rm_device", "i2c_del_master_bus", "reopen(context)",
        "const esp_err_t probeError = i2c_master_probe",
        "context.bus, context.address, clampTimeoutMs(timeoutMs)",
    ):
        if token not in transport_text:
            fail("owner recovery must rebuild and verify the owned bus/device lifecycle")

    for label, source in (("version generator", version_text), ("IDF build", idf_cmake_text)):
        if "--untracked-files=all" not in source or "--untracked-files=no" in source:
            fail(f"{label} must include untracked source in firmware cleanliness")
    if "status_result.returncode != 0" not in version_text:
        fail("version generation must not report clean when git status fails")
    no_args = version_text.find("if not args:")
    sync = version_text.find("_sync_outputs(project_root", no_args)
    identity = version_text.find("_append_build_metadata_defines(namespace, project_root)", no_args)
    if no_args < 0 or sync < 0 or identity < 0 or sync > identity:
        fail("generated headers must be synchronized before build identity is captured")
    if "LDC1614_GIT_STATUS_RESULT" not in idf_cmake_text:
        fail("ESP-IDF firmware must not report clean when git status fails")
    if "examples/esp_idf/basic/build/" not in gitignore_text:
        fail("generated ESP-IDF output must be ignored")

    style_text = read(common_dir / "CliStyle.h", "shared CLI style contract")
    check_table_and_behavior(cli_text + "\n" + cli_header_text + "\n" + style_text)
    print(f"CLI contract PASSED ({len(COMMAND_SPECS)} commands)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
