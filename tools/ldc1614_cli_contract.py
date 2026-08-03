#!/usr/bin/env python3
"""Host-only source of truth for the diagnostic CLI surface.

This module is intentionally not included by either firmware.  The Arduino and
native ESP-IDF examples each keep a local fixed-size C++ command table; the
repository checkers compare those tables with this independent contract.
"""

from __future__ import annotations

from dataclasses import dataclass
import re
from typing import Dict, List, Tuple

HELP_SYNOPSIS_WIDTH = 32
SECTION_ORDER = (
    "Common",
    "Lifecycle",
    "Measurements",
    "Configuration",
    "Registers and helpers",
    "Diagnostics",
)

EXECUTION_CLASSES = (
    "CACHE_ONLY",
    "PURE",
    "ONE_TRANSFER",
    "CORE_JOB",
    "CLI_JOB",
    "OWNER_BUS",
    "LIFECYCLE",
)
SAFETY_CLASSES = (
    "SAFE",
    "CONFIRM_MUTATION",
    "CONFIRM_DESTRUCTIVE_READ",
    "BUILD_PROFILE_ONLY",
)
FIXTURE_CLASSES = (
    "ANY",
    "NO_SENSOR_OK",
    "SENSOR_REQUIRED",
    "INTB_WIRED",
    "SD_WIRED",
    "DRIVE_TUNING",
)

ANSI = {
    "reset": "\x1b[0m",
    "red": "\x1b[31m",
    "green": "\x1b[32m",
    "yellow": "\x1b[33m",
    "cyan": "\x1b[36m",
}


@dataclass(frozen=True)
class CommandSpec:
    command_id: str
    section: str
    canonical: str
    aliases: Tuple[str, ...]
    synopses: Tuple[str, ...]
    execution: str
    safety: str
    fixture: str
    output_keys: Tuple[str, ...]


@dataclass(frozen=True)
class ParsedCppSpec:
    command_id: str
    canonical: str
    aliases: Tuple[str, ...]
    synopses: Tuple[str, ...]
    description: str
    section: str
    safety: str
    execution: str
    fixture: str
    output_keys: Tuple[str, ...]


def _spec(
    command_id: str,
    section: str,
    canonical: str,
    aliases: Tuple[str, ...],
    synopses: Tuple[str, ...],
    execution: str,
    safety: str = "SAFE",
    fixture: str = "ANY",
    output_keys: Tuple[str, ...] = ("command", "outcome", "code"),
) -> CommandSpec:
    return CommandSpec(
        command_id, section, canonical, aliases, synopses, execution, safety,
        fixture, output_keys,
    )


COMMAND_SPECS = (
    _spec("HELP", "Common", "help", ("?",), ("help / ?",), "CACHE_ONLY", output_keys=("command_count",)),
    _spec("VERSION", "Common", "version", ("ver",), ("version / ver",), "CACHE_ONLY", output_keys=("version", "firmware_git", "firmware_status", "build_timestamp")),
    _spec("COLOR", "Common", "color", (), ("color [on|off]",), "CACHE_ONLY", output_keys=("enabled",)),
    _spec("VERBOSE", "Common", "verbose", (), ("verbose [0|1]",), "CACHE_ONLY", output_keys=("enabled",)),

    _spec("BIND", "Lifecycle", "bind", (), ("bind",), "CACHE_ONLY", output_keys=("command", "outcome", "code", "detail", "msg")),
    _spec("END", "Lifecycle", "end", (), ("end",), "LIFECYCLE"),
    _spec("INIT", "Lifecycle", "init", (), ("init [deadline_ms]",), "CORE_JOB", output_keys=("command", "session", "deadline_ms", "outcome", "code")),
    _spec("APPLY", "Lifecycle", "apply", (), ("apply [deadline_ms]",), "CORE_JOB", output_keys=("command", "session", "deadline_ms", "outcome", "code")),
    _spec("RESET_REAPPLY", "Lifecycle", "resetreapply", (), ("resetreapply [ms] confirm",), "CORE_JOB", "CONFIRM_MUTATION", output_keys=("command", "session", "deadline_ms", "outcome", "code")),
    _spec("SLEEP", "Lifecycle", "sleep", (), ("sleep",), "ONE_TRANSFER"),
    _spec("WAKE", "Lifecycle", "wake", (), ("wake",), "ONE_TRANSFER"),
    _spec("CANCEL", "Lifecycle", "cancel", ("stop",), ("cancel / stop",), "CACHE_ONLY", output_keys=("command", "outcome", "code", "session")),
    _spec("JOB", "Lifecycle", "job", ("progress",), ("job / progress",), "CACHE_ONLY", output_keys=("active", "session", "kind", "phase", "transfers", "maximum", "requested", "completed", "deadline_ms")),
    _spec("RESULT", "Lifecycle", "result", (), ("result",), "CACHE_ONLY", output_keys=("command", "session", "kind", "outcome", "effects", "revision", "phase", "reg", "channel", "transfers", "maximum", "code", "detail", "msg")),
    _spec("INVALIDATE", "Lifecycle", "invalidate", (), ("invalidate confirm",), "CACHE_ONLY", "CONFIRM_MUTATION"),
    _spec("BUS_RECOVER", "Lifecycle", "busrecover", (), ("busrecover confirm",), "OWNER_BUS", "CONFIRM_MUTATION"),

    _spec("READ", "Measurements", "read", ("acquire",), ("read / acquire [mask]",), "CORE_JOB", fixture="NO_SENSOR_OK", output_keys=("command", "session", "mask", "outcome", "code", "selected", "valid", "fresh", "error", "overrun", "revision", "completed_ms", "status_before", "status_after")),
    _spec("LAST", "Measurements", "last", (), ("last [channel]",), "CACHE_ONLY", fixture="SENSOR_REQUIRED", output_keys=("channel", "raw", "quality", "msb", "lsb")),
    _spec("WATCH", "Measurements", "watch", (), ("watch <mask> <count> [ms]",), "CLI_JOB", fixture="SENSOR_REQUIRED", output_keys=("command", "session", "requested", "completed", "failed", "elapsed_ms", "outcome", "code")),
    _spec("SAMPLE_RATE", "Measurements", "samplerate", (), ("samplerate <channel> <count>",), "CLI_JOB", fixture="SENSOR_REQUIRED", output_keys=("command", "session", "requested", "ok", "fail", "elapsed_ms", "hz", "outcome", "code")),
    _spec("READY", "Measurements", "ready", ("drdy",), ("ready / drdy",), "ONE_TRANSFER", fixture="NO_SENSOR_OK", output_keys=("ready", "code")),
    _spec("STATUS", "Measurements", "status", (), ("status",), "ONE_TRANSFER", fixture="NO_SENSOR_OK", output_keys=("observed", "raw", "drdy", "unread", "err_ch", "ur", "or", "wd", "ah", "al", "zc", "code")),
    _spec("STATUS_RAW", "Measurements", "status_raw", (), ("status_raw",), "ONE_TRANSFER", fixture="NO_SENSOR_OK", output_keys=("raw", "code")),
    _spec("INTB", "Measurements", "intb", (), ("intb",), "PURE", fixture="INTB_WIRED", output_keys=("asserted", "code")),
    _spec("INIT_DRIVE", "Measurements", "initdrive", (), ("initdrive <channel>",), "ONE_TRANSFER", fixture="DRIVE_TUNING", output_keys=("channel", "init_drive_code", "code")),

    _spec("CFG", "Configuration", "cfg", ("settings",), ("cfg / settings",), "CACHE_ONLY", output_keys=("address", "variant", "variant_channels", "selected", "mode", "ref_source", "ref_hz", "tolerance_ppm", "deglitch", "activation", "timeout_ms", "rp_override", "auto_amplitude", "high_current", "intb_config", "error_reporting", "revision", "applied")),
    _spec("PROFILE", "Configuration", "profile", (), ("profile show|reset|discard", "profile validate", "profile commit confirm"), "CACHE_ONLY", "CONFIRM_MUTATION", output_keys=("dirty", "valid", "field", "channel", "outcome", "code")),
    _spec("ADDR", "Configuration", "addr", (), ("addr",), "CACHE_ONLY", "BUILD_PROFILE_ONLY", output_keys=("address",)),
    _spec("VARIANT", "Configuration", "variant", (), ("variant",), "CACHE_ONLY", "BUILD_PROFILE_ONLY", output_keys=("variant", "variant_channels")),
    _spec("MODE", "Configuration", "mode", (), ("mode <single ch|seq count>",), "CACHE_ONLY", output_keys=("mode", "channel", "count")),
    _spec("REFCLK", "Configuration", "refclk", (), ("refclk <src> <hz> <ppm>",), "CACHE_ONLY", output_keys=("source", "hz", "ppm")),
    _spec("DEGLITCH", "Configuration", "deglitch", (), ("deglitch <1|3|10|33>",), "CACHE_ONLY", output_keys=("mhz",)),
    _spec("ACTIVATION", "Configuration", "activation", (), ("activation <full|low>",), "CACHE_ONLY", output_keys=("mode",)),
    _spec("TIMEOUT", "Configuration", "timeout", (), ("timeout <ms>",), "CACHE_ONLY", output_keys=("timeout_ms",)),
    _spec("RP", "Configuration", "rp", (), ("rp <0|1>",), "CACHE_ONLY", output_keys=("enabled",)),
    _spec("AUTOAMP", "Configuration", "autoamp", (), ("autoamp <0|1>",), "CACHE_ONLY", output_keys=("enabled",)),
    _spec("HIGH_CURRENT", "Configuration", "highcurrent", (), ("highcurrent <0|1>",), "CACHE_ONLY", output_keys=("enabled",)),
    _spec("INTB_CONFIG", "Configuration", "intbconfig", (), ("intbconfig <0|1>",), "CACHE_ONLY", output_keys=("enabled",)),
    _spec("ERRORS", "Configuration", "errors", (), ("errors [show|all|none]",), "CACHE_ONLY", output_keys=("ur", "or", "wd", "ah", "al", "zc", "drdy")),
    _spec("ERROR", "Configuration", "error", (), ("error <field> <0|1>",), "CACHE_ONLY", output_keys=("field", "enabled")),
    _spec("RCOUNT", "Configuration", "rcount", (), ("rcount <ch> <value>",), "CACHE_ONLY", output_keys=("channel", "value")),
    _spec("SETTLE", "Configuration", "settle", (), ("settle <ch> <value>",), "CACHE_ONLY", output_keys=("channel", "value")),
    _spec("FIN_DIV", "Configuration", "findiv", (), ("findiv <ch> <value>",), "CACHE_ONLY", output_keys=("channel", "value")),
    _spec("FREF_DIV", "Configuration", "frefdiv", (), ("frefdiv <ch> <value>",), "CACHE_ONLY", output_keys=("channel", "value")),
    _spec("OFFSET", "Configuration", "offset", (), ("offset <ch> <value>",), "CACHE_ONLY", output_keys=("channel", "value")),
    _spec("DRIVE", "Configuration", "drive", (), ("drive <ch> <code>",), "CACHE_ONLY", fixture="DRIVE_TUNING", output_keys=("channel", "code")),
    _spec("SENSOR_BOUNDS", "Configuration", "sensorbounds", (), ("sensorbounds <ch> <lo> <hi>",), "CACHE_ONLY", fixture="SENSOR_REQUIRED", output_keys=("channel", "low_hz", "high_hz")),

    _spec("PROBE", "Registers and helpers", "probe", ("id",), ("probe / id",), "CLI_JOB", fixture="NO_SENSOR_OK", output_keys=("manufacturer_id", "device_id", "match", "code")),
    _spec("SCAN", "Registers and helpers", "scan", (), ("scan",), "OWNER_BUS", output_keys=("found", "probes", "code")),
    _spec("DUMP", "Registers and helpers", "dump", (), ("dump config", "dump all confirm"), "CLI_JOB", "CONFIRM_DESTRUCTIVE_READ", "NO_SENSOR_OK", ("scope", "register", "value", "count", "code")),
    _spec("VERIFY", "Registers and helpers", "verify", (), ("verify",), "CLI_JOB", fixture="NO_SENSOR_OK", output_keys=("checked", "matched", "mismatched", "read_failures", "outcome", "code")),
    _spec("REG", "Registers and helpers", "reg", ("rreg",), ("reg / rreg <addr> [confirm]",), "ONE_TRANSFER", "CONFIRM_DESTRUCTIVE_READ", "NO_SENSOR_OK", ("register", "value", "code")),
    _spec("WREG", "Registers and helpers", "wreg", (), ("wreg <addr> <value> confirm",), "ONE_TRANSFER", "CONFIRM_MUTATION", "NO_SENSOR_OK", ("register", "value", "code")),
    _spec("DECODE", "Registers and helpers", "decode", (), ("decode status <raw16>", "decode data <msb> <lsb>"), "PURE", output_keys=("kind", "raw", "drdy", "unread", "err_ch", "ur", "or", "wd", "ah", "al", "zc", "count", "quality")),
    _spec("FREQ", "Registers and helpers", "freq", (), ("freq <channel> <raw28>",), "PURE", output_keys=("channel", "raw", "frequency_hz", "code")),
    _spec("TIMING", "Registers and helpers", "timing", (), ("timing [mask]",), "PURE", output_keys=("mask", "wake_settle_us", "conversion_us", "sequential_frame_us", "acquisition_transfers", "code")),
    _spec("DRIVE_UA", "Registers and helpers", "driveua", (), ("driveua <code>",), "PURE", output_keys=("code", "microamps")),

    _spec("DRIVER", "Diagnostics", "drv", ("health",), ("drv / health",), "CACHE_ONLY", output_keys=("bound", "applied", "revision", "active", "result_available", "attempts", "success", "failures", "last_code")),
    _spec("STATE", "Diagnostics", "state", (), ("state",), "CACHE_ONLY", output_keys=("bound", "applied", "profile_dirty", "session_kind", "active", "pending_result")),
    _spec("SELFTEST", "Diagnostics", "selftest", (), ("selftest",), "CLI_JOB", fixture="NO_SENSOR_OK", output_keys=("command", "session", "identity", "config", "status", "helpers", "pass", "fail", "skip", "outcome", "code")),
    _spec("STRESS", "Diagnostics", "stress", (), ("stress <count> [mask]",), "CLI_JOB", fixture="NO_SENSOR_OK", output_keys=("command", "session", "requested", "ok", "fail", "elapsed_ms", "hz", "outcome", "code")),
    _spec("STRESS_MIX", "Diagnostics", "stress_mix", (), ("stress_mix <n> [mask] confirm",), "CLI_JOB", "CONFIRM_MUTATION", "NO_SENSOR_OK", ("command", "session", "requested", "ok", "fail", "elapsed_ms", "outcome", "code")),
    _spec("SOAK", "Diagnostics", "soak", (), ("soak <seconds> [mask]",), "CLI_JOB", fixture="NO_SENSOR_OK", output_keys=("command", "session", "seconds", "cycles", "ok", "fail", "elapsed_ms", "outcome", "code")),
    _spec("SD", "Diagnostics", "sd", (), ("sd status|assert|release confirm",), "LIFECYCLE", "CONFIRM_MUTATION", "SD_WIRED", ("state", "outcome", "code")),
)

COMMAND_BY_NAME: Dict[str, CommandSpec] = {}
for _command in COMMAND_SPECS:
    for _name in (_command.canonical, *_command.aliases):
        if _name in COMMAND_BY_NAME:
            raise RuntimeError(f"duplicate CLI token in host contract: {_name}")
        COMMAND_BY_NAME[_name] = _command

STALE_COMMANDS = ("begin", "sync", "readall", "initidrive")

# Automatic HIL groups.  Confirmations are explicit so unattended runs cannot
# accidentally exercise a mutation after a firmware prompt/grammar change.
NO_SENSOR_COMMANDS = (
    "help", "color off", "help", "color on", "verbose 1", "verbose 0",
    "version",
    "scan", "busrecover confirm", "init",
    "apply", "resetreapply confirm",
    "probe", "reg 0x7E", "reg 0x7F",
    "status", "status_raw", "ready",
    "cfg", "job", "result", "state",
    "profile show", "profile validate", "profile discard",
    "dump config", "dump all confirm", "verify",
    "wake", "selftest", "read 0x01", "sleep", "wake",
    "decode status 0x0040", "decode data 0x0000 0x0000",
    "freq 0 0x0100000", "timing 0x01", "driveua 0",
    "wreg 0x1A 0x3481 confirm", "init",
    "invalidate confirm", "init",
    "end", "bind", "init",
    "cancel", "wake", "drv",
)
SENSOR_COMMANDS = (
    "help", "version", "scan", "busrecover confirm", "init", "probe", "drv",
    "cfg", "job", "result", "status", "ready", "sleep", "wake",
    "timing 0x01", "verify", "selftest",
)
OPTIONAL_COMMAND_GROUPS = {
    "stress": (
        "stress {count} 0x01",
        "stress_mix {count} 0x01 confirm",
        "soak 2 0x01",
    ),
    "sample_rate": ("samplerate {channel} {count}",),
    "intb": ("intb",),
    "sd": ("sd status",),
    "drive_tuning": ("initdrive 0", "driveua 0"),
}
OPTIONAL_GROUP_FIXTURES = {
    "stress": {"NO_SENSOR_OK"},
    "sample_rate": {"SENSOR_REQUIRED"},
    "intb": {"INTB_WIRED"},
    "sd": {"SD_WIRED"},
    "drive_tuning": {"ANY", "DRIVE_TUNING"},
}
SAFE_COMPOUND_BRANCHES = {
    ("dump", "dump config"),
    ("profile", "profile show"),
    ("profile", "profile reset"),
    ("profile", "profile validate"),
    ("profile", "profile discard"),
    ("reg", "reg 0x7E"),
    ("reg", "reg 0x7F"),
    ("sd", "sd status"),
}

# Every hardware-trust invalidation in the unattended no-sensor matrix is
# immediately followed by the complete recovery sequence it requires.  These
# tuples are also asserted by validate_contract() so a later command-list edit
# cannot silently weaken the HIL lifecycle coverage.
NO_SENSOR_REQUIRED_SUBSEQUENCES = (
    ("scan", "busrecover confirm", "init"),
    ("wreg 0x1A 0x3481 confirm", "init"),
    ("invalidate confirm", "init"),
    ("end", "bind", "init"),
    ("wake", "selftest", "read 0x01"),
    ("cancel", "wake", "drv"),
)

_CPP_SPEC_PATTERN = re.compile(
    r"\{\s*CommandId::(?P<id>[A-Z0-9_]+)\s*,\s*"
    r'"(?P<name>(?:\\.|[^"\\])*)"\s*,\s*'
    r'"(?P<aliases>(?:\\.|[^"\\])*)"\s*,\s*'
    r'"(?P<synopsis>(?:\\.|[^"\\])*)"\s*,\s*'
    r'"(?P<description>(?:\\.|[^"\\])*)"\s*,\s*'
    r"HelpSection::(?P<section>[A-Z0-9_]+)\s*,\s*"
    r"CommandSafety::(?P<safety>[A-Z0-9_]+)\s*,\s*"
    r"ExecutionKind::(?P<execution>[A-Z0-9_]+)\s*,\s*"
    r"FixtureRequirement::(?P<fixture>[A-Z0-9_]+)\s*,\s*"
    r'"(?P<evidence>(?:\\.|[^"\\])*)"\s*\}',
    re.DOTALL,
)


def _decode_cpp_string(value: str) -> str:
    return (value.replace(r"\n", "\n").replace(r"\t", "\t")
            .replace(r'\"', '"').replace(r"\\", "\\"))


def parse_cpp_command_specs(source: str) -> Tuple[ParsedCppSpec, ...]:
    """Parse the deliberately simple firmware-local COMMAND_SPECS table."""
    table = re.search(
        r"\bCOMMAND_SPECS\s*\[\s*\]\s*=\s*\{(?P<body>.*?)\n\s*\};",
        source,
        re.DOTALL,
    )
    if table is None:
        return ()
    parsed: List[ParsedCppSpec] = []
    for match in _CPP_SPEC_PATTERN.finditer(table.group("body")):
        aliases = tuple(_decode_cpp_string(match.group("aliases")).split())
        synopsis_text = _decode_cpp_string(match.group("synopsis"))
        synopses = tuple(line for line in synopsis_text.splitlines() if line)
        evidence = tuple(_decode_cpp_string(match.group("evidence")).split())
        parsed.append(
            ParsedCppSpec(
                match.group("id"),
                _decode_cpp_string(match.group("name")),
                aliases,
                synopses,
                _decode_cpp_string(match.group("description")),
                match.group("section"),
                match.group("safety"),
                match.group("execution"),
                match.group("fixture"),
                evidence,
            )
        )
    return tuple(parsed)


def compare_cpp_command_specs(source: str, label: str) -> Tuple[str, ...]:
    """Return actionable parity failures for one firmware-local table."""
    actual = parse_cpp_command_specs(source)
    if not actual:
        return (
            f"{label}: missing or unparsable static constexpr COMMAND_SPECS[]; "
            "expected CommandId/name/aliases/synopsis/description/section/"
            "safety/execution/fixture/evidence fields",
        )
    errors: List[str] = []
    if len(actual) != len(COMMAND_SPECS):
        errors.append(
            f"{label}: COMMAND_SPECS has {len(actual)} rows; "
            f"host contract has {len(COMMAND_SPECS)}"
        )
    section_names = {
        "Common": "COMMON",
        "Lifecycle": "LIFECYCLE",
        "Measurements": "MEASUREMENTS",
        "Configuration": "CONFIGURATION",
        "Registers and helpers": "REGISTERS_AND_HELPERS",
        "Diagnostics": "DIAGNOSTICS",
    }
    for index, expected in enumerate(COMMAND_SPECS):
        if index >= len(actual):
            errors.append(f"{label}: missing command row {expected.canonical!r}")
            continue
        observed = actual[index]
        checks = (
            ("command id", observed.command_id, expected.command_id),
            ("canonical", observed.canonical, expected.canonical),
            ("aliases", observed.aliases, expected.aliases),
            ("synopses", observed.synopses, expected.synopses),
            ("section", observed.section, section_names[expected.section]),
            ("safety", observed.safety, expected.safety),
            ("execution", observed.execution, expected.execution),
            ("fixture", observed.fixture, expected.fixture),
            ("evidence keys", observed.output_keys, expected.output_keys),
        )
        for field, got, wanted in checks:
            if got != wanted:
                errors.append(
                    f"{label}: row {index + 1} {expected.canonical!r} {field} "
                    f"is {got!r}; expected {wanted!r}"
                )
        if not observed.description.strip():
            errors.append(f"{label}: {expected.canonical!r} description is empty")
    for extra in actual[len(COMMAND_SPECS):]:
        errors.append(f"{label}: unexpected command row {extra.canonical!r}")
    return tuple(errors)


def validate_contract() -> Tuple[str, ...]:
    errors = []
    ids = set()
    section_positions = {name: index for index, name in enumerate(SECTION_ORDER)}
    previous_section = -1
    for spec in COMMAND_SPECS:
        if spec.command_id in ids:
            errors.append(f"duplicate command id: {spec.command_id}")
        ids.add(spec.command_id)
        if spec.section not in section_positions:
            errors.append(f"unknown section for {spec.canonical}: {spec.section}")
        else:
            position = section_positions[spec.section]
            if position < previous_section:
                errors.append(f"section order regressed at {spec.canonical}")
            previous_section = position
        if spec.execution not in EXECUTION_CLASSES:
            errors.append(f"unknown execution class for {spec.canonical}")
        if spec.safety not in SAFETY_CLASSES:
            errors.append(f"unknown safety class for {spec.canonical}")
        if spec.fixture not in FIXTURE_CLASSES:
            errors.append(f"unknown fixture class for {spec.canonical}")
        for synopsis in spec.synopses:
            if len(synopsis) > HELP_SYNOPSIS_WIDTH:
                errors.append(
                    f"synopsis exceeds {HELP_SYNOPSIS_WIDTH} columns: {synopsis!r}"
                )
        if not spec.output_keys:
            errors.append(f"empty output-key contract for {spec.canonical}")
    for group_name, commands, allowed_fixtures in (
        ("no-sensor", NO_SENSOR_COMMANDS, {"ANY", "NO_SENSOR_OK"}),
        ("sensor", SENSOR_COMMANDS, {"ANY", "NO_SENSOR_OK", "SENSOR_REQUIRED"}),
    ):
        for command in commands:
            name = command.split(" ", 1)[0]
            spec = COMMAND_BY_NAME.get(name)
            if spec is None:
                errors.append(f"{group_name} HIL group has unknown command: {command}")
                continue
            if name != spec.canonical:
                errors.append(f"{group_name} HIL group uses alias instead of canonical: {command}")
            if spec.fixture not in allowed_fixtures:
                errors.append(
                    f"{group_name} HIL group cannot run {command}: fixture={spec.fixture}"
                )
            confirmation_required = spec.safety.startswith("CONFIRM_")
            safe_branch = (name, command) in SAFE_COMPOUND_BRANCHES
            if (confirmation_required and "confirm" not in command.split()
                    and not safe_branch):
                errors.append(
                    f"{group_name} HIL group lacks explicit confirmation: {command}"
                )
            if spec.safety == "BUILD_PROFILE_ONLY":
                errors.append(
                    f"{group_name} HIL group contains gated command: {command}"
                )
    for required in NO_SENSOR_REQUIRED_SUBSEQUENCES:
        width = len(required)
        if not any(
            NO_SENSOR_COMMANDS[index:index + width] == required
            for index in range(len(NO_SENSOR_COMMANDS) - width + 1)
        ):
            errors.append(
                "no-sensor HIL group missing required ordered sequence: "
                + " -> ".join(required)
            )
    if not NO_SENSOR_COMMANDS or NO_SENSOR_COMMANDS[-1] != "drv":
        errors.append("no-sensor HIL group must finish with drv state evidence")
    if set(OPTIONAL_COMMAND_GROUPS) != set(OPTIONAL_GROUP_FIXTURES):
        errors.append("optional command groups and fixture contracts differ")
    for group_name, commands in OPTIONAL_COMMAND_GROUPS.items():
        allowed_fixtures = OPTIONAL_GROUP_FIXTURES.get(group_name, set())
        if not commands:
            errors.append(f"optional group is empty: {group_name}")
        for template in commands:
            rendered = template.format(count=1, channel=0)
            name = rendered.split(" ", 1)[0]
            spec = COMMAND_BY_NAME.get(name)
            if spec is None:
                errors.append(f"optional group {group_name} has unknown command: {template}")
                continue
            if name != spec.canonical:
                errors.append(
                    f"optional group {group_name} uses alias instead of canonical: {template}"
                )
            if spec.fixture not in allowed_fixtures:
                errors.append(
                    f"optional group {group_name} cannot run {template}: "
                    f"fixture={spec.fixture}"
                )
            confirmation_required = spec.safety.startswith("CONFIRM_")
            safe_branch = (name, rendered) in SAFE_COMPOUND_BRANCHES
            if confirmation_required and "confirm" not in rendered.split() and not safe_branch:
                errors.append(
                    f"optional group {group_name} lacks confirmation: {template}"
                )
            if spec.safety == "BUILD_PROFILE_ONLY":
                errors.append(
                    f"optional group {group_name} contains forbidden safety class: {template}"
                )
    return tuple(errors)


if __name__ == "__main__":
    problems = validate_contract()
    if problems:
        raise SystemExit("\n".join(problems))
    print(f"CLI host contract PASSED ({len(COMMAND_SPECS)} commands)")
