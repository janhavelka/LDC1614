#!/usr/bin/env python3
"""Host-only tests for the CLI contract and conservative HIL evidence runner."""

from __future__ import annotations

import contextlib
import io
import json
import sys
import tempfile
import types
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import ldc1614_cli_contract as contract
import check_cli_contract as cli_checker
import ldc1614_hil_runner as runner


def sync_output(command: str, body: str = "") -> str:
    return (
        body
        + f"CLI result: command={command} outcome=SUCCESS code=0 detail=0 msg=OK\n> "
    )


def async_output(command: str, session: int, body: str = "") -> str:
    return (
        f"CLI scheduled: command={command} session={session}\n"
        + body
        + f"CLI result: command={command} session={session} "
        "outcome=SUCCESS code=0\n> "
    )


def version_output(commit: str = "abcdef1") -> str:
    return (
        f"version=3.0.0 firmware_git={commit} firmware_status=clean "
        "build_timestamp=test\n> "
    )


def cfg_output() -> str:
    return (
        "Configuration: desired revision=1 applied=APPLIED_ACTIVE staged_dirty=0\n"
        "cfg label=desired address=0x2A variant=LDC1614 variant_channels=4 "
        "selected=0x01 mode=single "
        "ref_source=external ref_hz=40000000 tolerance_ppm=100\n"
        "deglitch=10 activation=full timeout_ms=10 rp_override=0 "
        "auto_amplitude=0 high_current=0 intb_config=1 "
        "error_reporting=0xF8FD revision=1 applied=APPLIED_ACTIVE\n> "
    )


def probe_output(session: int = 9) -> str:
    return async_output(
        "probe", session,
        "manufacturer_id=0x5449 device_id=0x3055 match=YES code=0\n",
    )


def status_output() -> str:
    return (
        "STATUS=0x2840 observed=1 raw=0x2840 drdy=1 unread=0x01 "
        "err_ch=0 ur=1 or=0 wd=0 ah=0 al=0 zc=0\n"
        "  Status: OK (code=0, detail=0)\n  Message: OK\n> "
    )


def drv_output() -> str:
    return (
        "state bound=1 applied=APPLIED_ACTIVE profile_dirty=0 session_kind=NONE "
        "active=0 pending_result=0\n"
        "drv bound=1 applied=APPLIED_ACTIVE revision=1 active=0 "
        "result_available=0 attempts=10 success=10 failures=0 last_code=0\n"
        "transport attempts=10 success=10 failures=0\n> "
    )


def base_golden_outputs() -> dict[str, str]:
    status_clean = (
        "STATUS=0x0040 observed=1 raw=0x0040 drdy=1 unread=0x00 err_ch=0 "
        "ur=0 or=0 wd=0 ah=0 al=0 zc=0\n"
        "  Status: OK (code=0, detail=0)\n> "
    )
    batch = (
        "batch type=SEQUENTIAL_READOUT selected=0x01 valid=0x01 fresh=0x01 "
        "error=0x00 overrun=0x00 revision=1 completed_ms=100 simultaneous=0\n"
        "status_before=0x0040 observed=1 raw=0x0040 drdy=1 unread=0x01 "
        "err_ch=0 ur=0 or=0 wd=0 ah=0 al=0 zc=0\n"
        "status_after=0x0000 observed=1 raw=0x0000 drdy=0 unread=0x00 "
        "err_ch=0 ur=0 or=0 wd=0 ah=0 al=0 zc=0\n"
    )
    generic = lambda command: f"CLI result: command={command} outcome=SUCCESS code=0\n> "
    outputs = {
        "help": "=== LDC1614 CLI ===\n[Common]\ncommand_count=64\n> ",
        "color off": "color enabled=0\n> ",
        "color on": "color enabled=1\n> ",
        "verbose 1": "verbose enabled=1\n> ",
        "verbose 0": "verbose enabled=0\n> ",
        "version": version_output(),
        "scan": async_output("scan", 1, "scan complete found=1 probes=112 code=0\n"),
        "busrecover confirm": generic("busrecover"),
        "init": async_output("init", 0),
        "apply": async_output("apply", 0),
        "resetreapply confirm": async_output("resetreapply", 0),
        "probe": probe_output(2),
        "reg 0x7E": "register=0x7E value=0x5449 code=0\n> ",
        "reg 0x7F": "register=0x7F value=0x3055 code=0\n> ",
        "status": status_clean,
        "status_raw": "status_raw=0x0040 code=0\n> ",
        "ready": (
            "ready=1 code=0\nready_status=0x0040 observed=1 raw=0x0040 "
            "drdy=1 unread=0x00 err_ch=0 ur=0 or=0 wd=0 ah=0 al=0 zc=0\n> "
        ),
        "cfg": cfg_output(),
        "job": (
            "job active=0 operation=0 kind=NONE phase=NONE transfers=0 maximum=0 "
            "requested=0x00 completed=0x00 deadline_ms=0 effects=0x00 revision=1\n"
            "session active=0 id=0 kind=NONE phase=0 completed=0/0 pass=0 fail=0 skip=0\n> "
        ),
        "result": (
            "Operation result: operation=7 kind=INITIALIZE outcome=SUCCESS effects=0x00 "
            "revision=1 completed_ms=99 phase=COMPLETE reg=0x00 channel=0 "
            "transfers=20 maximum=20 code=0 detail=0 msg=OK\n> "
        ),
        "state": (
            "state bound=1 applied=APPLIED_ACTIVE profile_dirty=0 session_kind=NONE "
            "active=0 pending_result=0\n> "
        ),
        "profile show": "Configuration: staged revision=1\ncfg label=staged address=0x2A\n> ",
        "profile validate": (
            "profile field=all channel=all dirty=0 valid=1 outcome=VALID code=0 "
            "i2c_attempts=0\n> "
        ),
        "profile discard": (
            "CLI preview: field=profile dirty=0 valid=unknown channel=none "
            "outcome=DISCARDED code=0 i2c_attempts=0\n> "
        ),
        "dump config": async_output(
            "dump", 3, "dump complete scope=config count=23 failures=0\n"
        ),
        "dump all confirm": async_output(
            "dump", 4, "dump complete scope=all count=36 failures=0\n"
        ),
        "verify": async_output(
            "verify", 5,
            "verify complete checked=22 matched=22 mismatched=0 read_failures=0\n",
        ),
        "wake": generic("wake"),
        "selftest": async_output(
            "selftest", 6,
            "selftest identity=completed config=completed status=completed helpers=completed\n"
            "Selftest result: pass=8 fail=0 skip=1\n",
        ),
        "read 0x01": async_output("read", 0, batch),
        "sleep": generic("sleep"),
        "decode status 0x0040": (
            "decode kind=status decoded_status=0x0040 observed=1 raw=0x0040 "
            "drdy=1 unread=0x00 err_ch=0 ur=0 or=0 wd=0 ah=0 al=0 zc=0\n> "
        ),
        "decode data 0x0000 0x0000": (
            "decode kind=data raw=0x0000 msb=0x0000 lsb=0x0000 count=0x0000000 "
            "quality=0x0002 quality_names=STALE\n> "
        ),
        "freq 0 0x0100000": "channel=0 raw=0x0100000 frequency_hz=156250.000000 code=0\n> ",
        "timing 0x01": (
            "mask=0x01 wake_settle_us=1000 conversion_us=1000 "
            "sequential_frame_us=2000 acquisition_transfers=4 code=0\n> "
        ),
        "driveua 0": "code=0 microamps=16\n> ",
        "wreg 0x1A 0x3481 confirm": "register=0x1A value=0x3481 code=0\n> ",
        "invalidate confirm": generic("invalidate"),
        "end": generic("end"),
        "bind": "CLI result: command=bind outcome=SUCCESS code=0 detail=0 msg=OK\n> ",
        "cancel": "CLI result: command=cancel outcome=SUCCESS code=0 session=0\n> ",
        "drv": drv_output(),
    }
    return outputs


def config_golden_output(command: str) -> str:
    tokens = command.split()
    name = tokens[0]
    if command == "profile reset":
        return ("CLI preview: field=profile dirty=1 valid=unknown channel=none "
                "outcome=RESET code=0 i2c_attempts=0\n> ")
    if command == "profile validate":
        return ("profile field=all channel=all dirty=1 valid=1 outcome=VALID "
                "code=0 i2c_attempts=0\n> ")
    if command == "profile discard":
        return ("CLI preview: field=profile dirty=0 valid=unknown channel=none "
                "outcome=DISCARDED code=0 i2c_attempts=0\n> ")
    if command == "drv":
        return drv_output()
    if name == "mode":
        primary = (f"mode=single channel={tokens[2]} count=1" if tokens[1] == "single"
                   else f"mode=seq channel=none count={tokens[2]}")
        field = "mode"
    elif name == "refclk":
        primary = f"source={tokens[1]} hz={tokens[2]} ppm={tokens[3]}"
        field = "refclk"
    elif name == "deglitch":
        primary, field = f"mhz={tokens[1]}", "deglitch"
    elif name == "activation":
        primary, field = f"mode={tokens[1]}", "activation"
    elif name == "timeout":
        primary, field = f"timeout_ms={tokens[1]}", "timeout"
    elif name in ("rp", "autoamp", "highcurrent", "intbconfig"):
        primary, field = f"enabled={tokens[1]}", name
    elif name == "errors":
        enabled = "0" if tokens[1] == "none" else "1"
        prefix = "errors " if tokens[1] == "show" else ""
        primary = prefix + " ".join(
            f"{field_name}={enabled}" for field_name in
            ("ur", "or", "wd", "ah", "al", "zc", "drdy")
        )
        if tokens[1] == "show":
            return primary + "\n> "
        field = "errors"
    elif name == "error":
        primary, field = f"field={tokens[1]} enabled={tokens[2]}", tokens[1]
    elif name in ("rcount", "settle", "findiv", "frefdiv", "offset"):
        primary, field = f"channel={tokens[1]} value={tokens[2]}", name
    else:
        raise AssertionError(f"no config golden for {command}")
    return (
        primary + "\n" +
        f"CLI preview: field={field} dirty=1 valid=unknown channel=none "
        "outcome=STAGED code=0 i2c_attempts=0\n> "
    )


def cpp_contract_source() -> str:
    sections = {
        "Common": "COMMON",
        "Lifecycle": "LIFECYCLE",
        "Measurements": "MEASUREMENTS",
        "Configuration": "CONFIGURATION",
        "Registers and helpers": "REGISTERS_AND_HELPERS",
        "Diagnostics": "DIAGNOSTICS",
    }
    rows = []
    for spec in contract.COMMAND_SPECS:
        aliases = " ".join(spec.aliases)
        synopses = "\\n".join(spec.synopses)
        evidence = " ".join(spec.output_keys)
        rows.append(
            "  {CommandId::%s, \"%s\", \"%s\", \"%s\", \"description\", "
            "HelpSection::%s, CommandSafety::%s, ExecutionKind::%s, "
            "FixtureRequirement::%s, \"%s\"},"
            % (
                spec.command_id, spec.canonical, aliases, synopses,
                sections[spec.section], spec.safety, spec.execution,
                spec.fixture, evidence,
            )
        )
    return "static constexpr CommandSpec COMMAND_SPECS[] = {\n" + "\n".join(rows) + "\n};\n"


class CliManifestTests(unittest.TestCase):
    def test_manifest_is_complete_ordered_and_bounded(self) -> None:
        self.assertEqual((), contract.validate_contract())
        self.assertEqual(64, len(contract.COMMAND_SPECS))
        self.assertEqual("help", contract.COMMAND_SPECS[0].canonical)
        self.assertEqual("sd", contract.COMMAND_SPECS[-1].canonical)
        self.assertTrue(
            all(len(line) <= contract.HELP_SYNOPSIS_WIDTH
                for spec in contract.COMMAND_SPECS for line in spec.synopses)
        )

    def test_aliases_and_stale_tokens_are_explicit(self) -> None:
        expected = {
            "?": "help", "ver": "version", "acquire": "read",
            "progress": "job", "drdy": "ready", "settings": "cfg",
            "rreg": "reg", "health": "drv",
        }
        self.assertEqual(
            expected,
            {alias: contract.COMMAND_BY_NAME[alias].canonical for alias in expected},
        )
        self.assertEqual(("begin", "sync", "readall", "initidrive"), contract.STALE_COMMANDS)

    def test_cpp_table_parser_compares_all_metadata(self) -> None:
        source = cpp_contract_source()
        self.assertEqual(64, len(contract.parse_cpp_command_specs(source)))
        self.assertEqual((), contract.compare_cpp_command_specs(source, "fixture"))

        drifted = source.replace("CommandId::JOB", "CommandId::PROGRESS", 1)
        errors = contract.compare_cpp_command_specs(drifted, "fixture")
        self.assertTrue(any("command id" in error and "JOB" in error for error in errors))

    def test_hil_groups_cover_both_profiles_without_stale_commands(self) -> None:
        for profile in ("arduino", "idf"):
            sensor = runner.default_commands(profile, "default")
            no_sensor = runner.default_commands(profile, "no-sensor")
            self.assertIn("busrecover confirm", sensor)
            self.assertIn("job", sensor)
            self.assertIn("selftest", sensor)
            self.assertIn("read 0x01", no_sensor)
            self.assertIn("ready", no_sensor)
            sleep_index = no_sensor.index("sleep")
            wake_after_sleep = no_sensor.index("wake", sleep_index + 1)
            self.assertLess(no_sensor.index("init"), sleep_index)
            self.assertLess(sleep_index, wake_after_sleep)
            self.assertEqual("drv", no_sensor[-1])
            for stale in contract.STALE_COMMANDS:
                self.assertFalse(any(runner.command_name(item) == stale for item in no_sensor))

    def test_no_sensor_recovery_fences_are_exact_and_ordered(self) -> None:
        commands = contract.NO_SENSOR_COMMANDS
        self.assertEqual(49, len(commands))
        for sequence in contract.NO_SENSOR_REQUIRED_SUBSEQUENCES:
            width = len(sequence)
            self.assertTrue(any(
                commands[index:index + width] == sequence
                for index in range(len(commands) - width + 1)
            ), sequence)
        for invalidator in (
            "busrecover confirm", "wreg 0x1A 0x3481 confirm",
            "invalidate confirm", "end",
        ):
            index = commands.index(invalidator)
            expected_next = "bind" if invalidator == "end" else "init"
            self.assertEqual(expected_next, commands[index + 1])
        self.assertEqual(("cancel", "wake", "drv"), commands[-3:])

    def test_sensor_initialization_is_activated_before_readiness_checks(self) -> None:
        commands = contract.SENSOR_COMMANDS
        for sequence in contract.SENSOR_REQUIRED_SUBSEQUENCES:
            width = len(sequence)
            self.assertTrue(any(
                commands[index:index + width] == sequence
                for index in range(len(commands) - width + 1)
            ), sequence)
        self.assertLess(commands.index("wake"), commands.index("ready"))

    def test_optional_groups_are_fixture_and_safety_validated(self) -> None:
        self.assertEqual((), contract.validate_contract())
        self.assertEqual({"NO_SENSOR_OK"}, contract.OPTIONAL_GROUP_FIXTURES["stress"])
        self.assertEqual({"SENSOR_REQUIRED"}, contract.OPTIONAL_GROUP_FIXTURES["sample_rate"])
        self.assertIn(("sd", "sd status"), contract.SAFE_COMPOUND_BRANCHES)


class ClassifierTests(unittest.TestCase):
    def test_informational_commands_do_not_require_fake_result_envelopes(self) -> None:
        status, reason = runner.classify_command("ver", version_output(), False)
        self.assertEqual("PASS", status, reason)
        status, _ = runner.classify_command(
            "version", "version=3.0.0 firmware_git=abcdef1 firmware_status=clean\n", False
        )
        self.assertEqual("FAIL", status)
        self.assertEqual("PASS", runner.classify_command("cfg", cfg_output(), False)[0])

    def test_complete_frozen_no_sensor_golden_matrix_classifies(self) -> None:
        outputs = base_golden_outputs()
        self.assertEqual(set(contract.NO_SENSOR_COMMANDS), set(outputs))
        for command in contract.NO_SENSOR_COMMANDS:
            with self.subTest(command=command):
                status, reason = runner.classify_command(
                    command, outputs[command], False, fixture="no-sensor"
                )
                self.assertEqual("PASS", status, reason)

    def test_help_and_result_golden_fields_are_not_optional(self) -> None:
        outputs = base_golden_outputs()
        self.assertEqual(
            "FAIL",
            runner.classify_command(
                "help", outputs["help"].replace("command_count=64\n", ""), False
            )[0],
        )
        self.assertEqual(
            "FAIL",
            runner.classify_command(
                "result", outputs["result"].replace("maximum=20 ", ""), False
            )[0],
        )

    def test_async_session_envelopes_must_match(self) -> None:
        output = async_output("init", 42)
        between = output.split("CLI scheduled:", 1)[1].split("CLI result:", 1)[0]
        self.assertNotIn(">", between)
        self.assertEqual(1, output.count("> "))
        self.assertEqual("PASS", runner.classify_command("init", output, False)[0])
        self.assertEqual(
            "FAIL",
            runner.classify_command("init", output.replace("session=42 outcome", "session=43 outcome"), False)[0],
        )
        self.assertEqual(
            "FAIL",
            runner.classify_command("init", output.replace("outcome=SUCCESS", "outcome=FAILED"), False)[0],
        )

    def test_probe_and_selftest_require_distinct_evidence(self) -> None:
        probe = probe_output()
        self.assertEqual("PASS", runner.classify_command("probe", probe, False)[0])
        self.assertEqual("FAIL", runner.classify_command("selftest", probe, False)[0])
        selftest = async_output(
            "selftest", 9, "Selftest result: pass=8 fail=0 skip=1\n"
        )
        self.assertEqual("PASS", runner.classify_command("selftest", selftest, False)[0])

    def test_complete_cfg_and_progress_evidence(self) -> None:
        self.assertEqual("PASS", runner.classify_command("cfg", cfg_output(), False)[0])
        progress = sync_output(
            "job",
            "job active=0 operation=0 kind=NONE phase=NONE transfers=0 maximum=0 "
            "requested=0x00 completed=0x00 deadline_ms=0\n"
            "session active=0 id=0 kind=NONE phase=0 completed=0/0\n",
        )
        self.assertEqual("PASS", runner.classify_command("progress", progress, False)[0])

    def test_no_sensor_flags_are_narrowly_tolerated(self) -> None:
        output = status_output()
        self.assertEqual("FAIL", runner.classify_command("status", output, False)[0])
        self.assertEqual(
            "PASS", runner.classify_command("status", output, False, fixture="no-sensor")[0]
        )
        broken = output + "I2C_TIMEOUT code=7\n"
        self.assertEqual(
            "FAIL", runner.classify_command("status", broken, False, fixture="no-sensor")[0]
        )

    def test_ansi_status_and_no_sensor_quality_counters_are_parsed(self) -> None:
        colored_status = status_output().replace(
            "Status: OK", "Status: \x1b[32mOK\x1b[0m"
        )
        self.assertEqual(
            "PASS",
            runner.classify_command(
                "status", colored_status, False, fixture="no-sensor"
            )[0],
        )
        stress = async_output(
            "stress", 17,
            "Stress results: 25 ok, 0 failed\n"
            "Stress result: requested=25 ok=25 fail=0 elapsed_ms=100 hz=250.0\n"
            "session_channel=0 selected=25 valid=0 fresh=25 error=25 "
            "overrun=3 bounds_fail=25\n",
        )
        self.assertEqual(
            "PASS",
            runner.classify_command(
                "stress 25 0x01", stress, False, fixture="no-sensor"
            )[0],
        )
        self.assertEqual(
            "FAIL", runner.classify_command("stress 25 0x01", stress, False)[0]
        )

    def test_timeouts_and_explicit_failure_tokens_never_pass(self) -> None:
        self.assertEqual("FAIL", runner.classify_command("version", version_output(), True)[0])
        self.assertEqual(
            "FAIL", runner.classify_command("version", version_output() + "[FAIL]\n", False)[0]
        )

    def test_version_parser_accepts_frozen_equals_and_legacy_colon(self) -> None:
        self.assertEqual("3.0.0", runner.firmware_version_from_transcript(version_output()))
        self.assertEqual(
            "3.0.0",
            runner.firmware_version_from_transcript("library version: 3.0.0\n"),
        )
        self.assertEqual("unknown", runner.firmware_version_from_transcript("version unknown"))

    def test_invalid_input_matrix_requires_exact_usage_and_no_job_admission(self) -> None:
        self.assertIn("rcount 0 4", runner.INVALID_INPUT_COMMANDS)
        self.assertIn("rcount 0 65536", runner.INVALID_INPUT_COMMANDS)
        self.assertNotIn("rcount 0 255", runner.INVALID_INPUT_COMMANDS)
        for command, pattern in runner.INVALID_INPUT_EVIDENCE_PATTERNS.items():
            synopsis = contract.COMMAND_BY_NAME[command.split(" ", 1)[0]].synopses[0]
            output = f"[E] usage: {synopsis}\n> "
            with self.subTest(command=command):
                self.assertIsNotNone(pattern.search(output))
                self.assertEqual("PASS", runner.classify_command(command, output, False)[0])
                self.assertEqual(
                    "FAIL",
                    runner.classify_command(
                        command,
                        output.replace(synopsis, "wrong") +
                        f"CLI scheduled: command={command.split()[0]} session=1\n",
                        False,
                    )[0],
                )

    def test_gated_command_formats_parse_but_unavailable_sd_does_not_pass(self) -> None:
        gated = {
            "addr": "address=0x2A build_profile_only=1\n> ",
            "variant": "variant=LDC1614 variant_channels=4 build_profile_only=1\n> ",
            "intb": "intb asserted=1 code=0\n> ",
            "initdrive 0": "channel=0 init_drive_code=12 microamps=96 code=0\n> ",
            "drive 0 12": (
                "channel=0 code=12\nCLI preview: field=drive dirty=1 valid=unknown "
                "channel=none outcome=STAGED code=0 i2c_attempts=0\n> "
            ),
            "sensorbounds 0 1000 2000": (
                "channel=0 low_hz=1000 high_hz=2000\n"
                "CLI preview: field=sensorbounds dirty=1 valid=unknown channel=none "
                "outcome=STAGED code=0 i2c_attempts=0\n> "
            ),
            "sd status": "sd state=released outcome=SUCCESS code=0\n> ",
        }
        for command, output in gated.items():
            with self.subTest(command=command):
                status, reason = runner.classify_command(command, output, False)
                self.assertEqual("PASS", status, reason)
        self.assertEqual(
            "FAIL",
            runner.classify_command(
                "sd status", "sd state=unavailable outcome=SKIP code=0\n> ", False
            )[0],
        )

    def test_sensor_and_commit_output_contracts_are_exact(self) -> None:
        batch = (
            "batch type=SEQUENTIAL_READOUT selected=0x01 valid=0x01 fresh=0x01 "
            "error=0x00 overrun=0x00 revision=1 completed_ms=10 simultaneous=0\n"
            "sample channel=0 msb=0x0001 lsb=0x0002 raw=0x0010002 raw28=0x0010002 "
            "quality=0x0001 quality_names=FRESH frequency_hz=1000.0 bounds=PASS\n> "
        )
        outputs = {
            "last 0": batch,
            "watch 0x01 2": async_output(
                "watch", 20,
                "Watch results: requested=2 completed=2 failed=0 elapsed_ms=20\n",
            ),
            "samplerate 0 2": async_output(
                "samplerate", 21,
                "SampleRate result: requested=2 ok=2 fail=0 elapsed_ms=20 "
                "hz=100.000000 ready_checks=2 ready_status_raw=0x0040\n",
            ),
            "profile commit confirm": (
                "profile_commit=COMMITTED config_revision=2 applied=APPLIED_SLEEPING "
                "i2c_attempts=0\n  Status: OK (code=0, detail=0)\n> "
            ),
        }
        for command, output in outputs.items():
            with self.subTest(command=command):
                status, reason = runner.classify_command(command, output, False)
                self.assertEqual("PASS", status, reason)


class CheckerRobustnessTests(unittest.TestCase):
    def test_bounded_parser_checker_captures_semantic_parameter_name(self) -> None:
        valid = """bool parseUnsigned(const char* text, uint64_t ceiling, uint64_t& value) {
  uint64_t parsed = 0;
  if (text == nullptr || parsed > ceiling) return false;
  value = parsed;
  return true;
}
"""
        cli_checker.require_bounded_unsigned_parser(valid)
        invalid = valid.replace("parsed > ceiling", "parsed > UINT64_MAX")
        with contextlib.redirect_stdout(io.StringIO()), self.assertRaises(SystemExit):
            cli_checker.require_bounded_unsigned_parser(invalid)


class MatrixAndSummaryTests(unittest.TestCase):
    def test_config_matrix_covers_every_safe_setting_and_restores_cache(self) -> None:
        commands = runner.configuration_matrix_commands(4)
        for command in (
            "mode single 0", "mode seq 2", "mode seq 3", "mode seq 4",
            "refclk internal 40000000 100", "refclk external 40000000 100",
            "deglitch 1", "deglitch 3", "deglitch 10", "deglitch 33",
            "activation full", "activation low", "timeout 1", "timeout 4294967295",
            "rp 0", "rp 1", "autoamp 0", "autoamp 1",
            "highcurrent 0", "highcurrent 1", "intbconfig 0", "intbconfig 1",
            "errors show", "errors none", "errors all",
        ):
            self.assertIn(command, commands)
        for route in runner.ERROR_ROUTE_NAMES:
            self.assertIn(f"error {route} 1", commands)
        for channel in range(4):
            for command in (
                f"rcount {channel} 5", f"rcount {channel} 65535",
                f"settle {channel} 0", f"settle {channel} 65535",
                f"findiv {channel} 1", f"findiv {channel} 15",
                f"frefdiv {channel} 1", f"frefdiv {channel} 1023",
                f"offset {channel} 0", f"offset {channel} 65535",
            ):
                self.assertIn(command, commands)
        self.assertEqual(
            ["profile reset", "profile validate", "profile discard", "drv"],
            commands[-4:],
        )
        for command in commands:
            with self.subTest(golden=command):
                output = config_golden_output(command)
                status, reason = runner.classify_command(
                    command, output, False, fixture="no-sensor"
                )
                self.assertEqual("PASS", status, reason)

    def test_config_and_invalid_input_categories_are_explicit_opt_ins(self) -> None:
        result = runner.make_result(runner.parse_args([
            "--dry-run", "--fixture", "no-sensor", "--include-config-matrix",
            "--include-invalid-inputs",
        ]))
        self.assertIn("mode seq 4", result["commands"])
        self.assertIn("driveua 32", result["commands"])
        matrix_end = result["commands"].index("drv", len(contract.NO_SENSOR_COMMANDS))
        self.assertEqual("profile discard", result["commands"][matrix_end - 1])

    def test_physical_fixture_gates_are_always_visible_as_not_run(self) -> None:
        result = runner.make_result(runner.parse_args([
            "--dry-run", "--fixture", "no-sensor",
        ]))
        skipped = {item["name"]: item["reason"] for item in result["skipped_optional_tests"]}
        for name in (
            "address_0x2B", "variant_LDC1612", "sensor_measurement_quality",
            "sample_rate_benchmark", "cached_last_sample", "intb_observation",
            "sd_shutdown_wake", "drive_current_tuning", "active_job_cancellation",
        ):
            self.assertIn(name, skipped)
            self.assertIn("NOT_RUN", skipped[name])

    def test_stress_is_scheduled_for_both_profiles(self) -> None:
        for profile in ("arduino", "idf"):
            args = runner.parse_args([
                "--profile", profile, "--dry-run", "--include-stress", "--stress-count", "25"
            ])
            result = runner.make_result(args)
            expected = [
                "stress 25 0x01", "stress_mix 25 0x01 confirm", "soak 2 0x01",
            ]
            stress_index = result["commands"].index("stress 25 0x01")
            self.assertEqual("wake", result["commands"][stress_index - 1])
            self.assertEqual(expected, result["commands"][stress_index:stress_index + 3])
            self.assertEqual("drv", result["commands"][stress_index + 3])
            self.assertFalse(any(item["name"] == "stress" for item in result["skipped_optional_tests"]))

            stress_outputs = {
                expected[0]: async_output(
                    "stress", 10,
                    "Stress results: 25 ok, 0 failed\n"
                    "Stress result: requested=25 ok=25 fail=0 elapsed_ms=100 hz=250.000000\n",
                ),
                expected[1]: async_output(
                    "stress_mix", 11,
                    "StressMix results: requested=25 ok=25 fail=0 elapsed_ms=250\n",
                ),
                expected[2]: async_output(
                    "soak", 12,
                    "Soak results: seconds=2 cycles=2 ok=2 fail=0 elapsed_ms=2000\n",
                ),
            }
            for command, output in stress_outputs.items():
                status, reason = runner.classify_command(
                    command, output, False, fixture="no-sensor"
                )
                self.assertEqual("PASS", status, reason)

    def test_samplerate_runs_on_sensor_for_both_profiles_and_is_gated_without_sensor(self) -> None:
        for profile in ("arduino", "idf"):
            sensor = runner.make_result(runner.parse_args([
                "--profile", profile, "--dry-run", "--sample-rate-count", "50"
            ]))
            self.assertIn("samplerate 0 50", sensor["commands"])
            sample_index = sensor["commands"].index("samplerate 0 50")
            self.assertEqual("wake", sensor["commands"][sample_index - 1])
            self.assertEqual("drv", sensor["commands"][sample_index + 1])
            no_sensor = runner.make_result(runner.parse_args([
                "--profile", profile, "--fixture", "no-sensor", "--dry-run",
                "--sample-rate-count", "50",
            ]))
            self.assertNotIn("samplerate 0 50", no_sensor["commands"])
            self.assertIn("sample_rate_benchmark", {
                item["name"] for item in no_sensor["skipped_optional_tests"]
            })

    def test_stress_and_samplerate_summaries_parse_counts(self) -> None:
        stress_args = runner.parse_args(["--include-stress", "--stress-count", "10"])
        stress = runner.summarize_stress(stress_args, [{
            "command": "stress 10 0x01", "status": "PASS", "elapsed_s": 0.5,
            "output": "Stress results: 10 ok, 0 failed\n",
        }])
        self.assertEqual("PASS", stress["status"])
        self.assertEqual(10, stress["success_count"])

        sample_args = runner.parse_args(["--sample-rate-count", "50"])
        sample = runner.summarize_sample_rate(sample_args, [{
            "command": "samplerate 0 50", "status": "PASS", "elapsed_s": 1.25,
            "output": "SampleRate result: requested=50 ok=50 fail=0 "
                      "elapsed_ms=1250 hz=40.000\n",
        }])
        self.assertEqual("PASS", sample["status"])
        self.assertEqual(40.0, sample["effective_hz"])

    def test_repeat_and_dry_run_remain_bounded_not_run(self) -> None:
        args = runner.parse_args([
            "--dry-run", "--skip-default-commands", "--command", "version",
            "--command", "cfg", "--repeat-command-set", "3",
        ])
        result = runner.make_result(args)
        self.assertEqual(["version", "cfg"] * 3, result["commands"])
        self.assertEqual("NOT_RUN", result["overall_status"])
        self.assertTrue(all(item["status"] == "NOT_RUN" for item in result["command_results"]))

    def test_no_port_artifact_does_not_claim_hardware(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "result.json"
            self.assertEqual(0, runner.main(["--json-out", str(path), "--quiet"]))
            result = json.loads(path.read_text(encoding="utf-8"))
            self.assertEqual("NOT_RUN", result["overall_status"])
            self.assertFalse(result["hardware_attached"])
            self.assertEqual("no_hardware_audit", result["evidence_type"])


class FakeSerialBase:
    def __init__(self, *_args, **_kwargs) -> None:
        self.buffer = bytearray(b"> ")
        self.pending = bytearray()
        self.dtr = False
        self.rts = False

    def __enter__(self):
        return self

    def __exit__(self, *_args):
        return False

    @property
    def in_waiting(self) -> int:
        if not self.buffer and self.pending:
            self.buffer.extend(self.pending)
            self.pending.clear()
        return len(self.buffer)

    def read(self, length: int) -> bytes:
        data = bytes(self.buffer[:length])
        del self.buffer[:length]
        return data

    def flush(self) -> None:
        return None


class SerialExecutionAndDurabilityTests(unittest.TestCase):
    def install_serial(self, serial_class):
        previous = sys.modules.get("serial")
        sys.modules["serial"] = types.SimpleNamespace(Serial=serial_class)
        self.addCleanup(
            lambda: sys.modules.pop("serial", None)
            if previous is None else sys.modules.__setitem__("serial", previous)
        )

    def test_serial_async_waits_for_matching_result(self) -> None:
        class FakeSerial(FakeSerialBase):
            def write(self, payload: bytes) -> int:
                self.buffer.extend(b"CLI scheduled: command=init session=42\n")
                self.pending.extend(
                    b"CLI result: command=init session=42 outcome=SUCCESS "
                    b"code=0 detail=0 msg=OK\n> "
                )
                return len(payload)

        self.install_serial(FakeSerial)
        args = runner.parse_args([
            "--port", "FAKE", "--startup-delay-s", "0", "--idle-gap-s", "0.01",
        ])
        results, _, _, _, _ = runner.run_serial_commands(args, ["init"])
        self.assertEqual("PASS", results[0]["status"], results[0]["reason"])

    def test_soak_base_gate_rejects_incomplete_or_ambiguous_evidence(self) -> None:
        args = runner.parse_args([
            "--port", "FAKE", "--address", "0x2A", "--channel-count", "4",
            "--expected-firmware-commit", "abcdef1",
            "--operator", "test", "--board", "fake",
        ])
        commands = ["version", "cfg", "drv"]
        outputs = [
            version_output("abcdef1"),
            cfg_output(),
            drv_output(),
        ]
        results = [
            {"command": command, "status": "PASS", "output": output}
            for command, output in zip(commands, outputs)
        ]
        transcript = "\n".join(outputs)
        self.assertIsNone(
            runner.base_acceptance_failure(args, commands, results, transcript)
        )
        default_commands = runner.default_commands("arduino", "no-sensor")
        golden_outputs = base_golden_outputs()
        default_results = [
            {"command": command, "status": "PASS", "output": golden_outputs[command]}
            for command in default_commands
        ]
        self.assertIsNone(
            runner.base_acceptance_failure(
                args, default_commands, default_results,
                "\n".join(golden_outputs[command] for command in default_commands),
            )
        )

        cases = [
            ("empty", [], [], transcript, "empty"),
            ("result-count", commands, results[:-1], transcript, "one result"),
            (
                "unknown", commands,
                [results[0], {**results[1], "status": "UNKNOWN"}, results[2]],
                transcript, "was UNKNOWN",
            ),
            (
                "active-job", commands,
                [results[0], results[1],
                 {**results[2], "output": outputs[2].replace("active=0", "active=1")}],
                transcript, "APPLIED_ACTIVE",
            ),
            (
                "dirty-profile", commands,
                [results[0], results[1],
                 {**results[2], "output": outputs[2].replace(
                     "profile_dirty=0", "profile_dirty=1"
                 )}],
                transcript, "APPLIED_ACTIVE",
            ),
        ]
        for name, candidate_commands, candidate_results, candidate_transcript, reason in cases:
            with self.subTest(name=name):
                failure = runner.base_acceptance_failure(
                    args, candidate_commands, candidate_results, candidate_transcript
                )
                self.assertIsNotNone(failure)
                self.assertIn(reason, failure)

        boot_banners = (
            "=== LDC1614 Arduino Diagnostic Bring-up Example ===",
            "=== LDC1614 Native ESP-IDF Diagnostic Bring-up Example ===",
        )
        for banner in boot_banners:
            with self.subTest(banner=banner):
                restarted = [
                    results[0], {**results[1], "output": f"{banner}\n"}, results[2]
                ]
                failure = runner.base_acceptance_failure(
                    args, commands, restarted, transcript
                )
                self.assertIsNotNone(failure)
                self.assertIn("restart banner", failure)

        expectation_cases = [
            ("address", "address", "0x2B", "address"),
            ("channels", "channel_count", 2, "channel count"),
            ("commit", "expected_firmware_commit", "abcdef2", "commit"),
        ]
        for name, field, value, reason in expectation_cases:
            with self.subTest(name=name):
                original = getattr(args, field)
                setattr(args, field, value)
                try:
                    failure = runner.base_acceptance_failure(
                        args, commands, results, transcript
                    )
                finally:
                    setattr(args, field, original)
                self.assertIsNotNone(failure)
                self.assertIn(reason, failure)

    def test_failed_base_matrix_prevents_soak_commands(self) -> None:
        class FakeSerial(FakeSerialBase):
            writes = []

            def write(self, payload: bytes) -> int:
                command = payload.decode().strip()
                type(self).writes.append(command)
                outputs = {
                    "version": version_output(),
                    "cfg": (
                        "CLI result: command=cfg outcome=FAILED code=18 "
                        "detail=259 msg=I2C invalid state\n> "
                    ),
                    "drv": drv_output(),
                }
                self.buffer.extend(outputs[command].encode())
                return len(payload)

        self.install_serial(FakeSerial)
        args = runner.parse_args([
            "--port", "FAKE", "--profile", "arduino", "--fixture", "no-sensor",
            "--startup-delay-s", "0", "--idle-gap-s", "0.01",
            "--include-long-soak", "--soak-duration-s", "0.1",
        ])
        results, transcript, _, _, soak = runner.run_serial_commands(
            args, ["version", "cfg", "drv"]
        )
        self.assertEqual(["version", "cfg"], FakeSerial.writes)
        self.assertEqual("FAIL", results[1]["status"])
        self.assertEqual("NOT_RUN", results[2]["status"])
        self.assertIn("cfg", results[2]["reason"])
        self.assertIn("runner fail-fast after command 2: cfg", transcript)
        self.assertIsNotNone(soak)
        self.assertFalse(soak["started"])
        self.assertEqual(0, soak["cycle_count"])

    def test_correlated_expected_failure_does_not_trigger_base_fail_fast(self) -> None:
        class FakeSerial(FakeSerialBase):
            writes = []

            def write(self, payload: bytes) -> int:
                command = payload.decode().strip()
                type(self).writes.append(command)
                outputs = {
                    "init": async_output(
                        "init", 1, "expected-nack I2C_NACK_ADDR code=5\n"
                    ).replace("outcome=SUCCESS code=0", "outcome=FAILED code=5"),
                    "cfg": cfg_output(),
                }
                self.buffer.extend(outputs[command].encode())
                return len(payload)

        self.install_serial(FakeSerial)
        args = runner.parse_args([
            "--port", "FAKE", "--startup-delay-s", "0", "--idle-gap-s", "0.01",
            "--expected-failure-token", "expected-nack",
        ])
        results, _, _, _, _ = runner.run_serial_commands(args, ["init", "cfg"])
        self.assertEqual(["init", "cfg"], FakeSerial.writes)
        self.assertEqual(["PASS", "PASS"], [result["status"] for result in results])

        malformed, _ = runner.classify_command(
            "version",
            "expected-nack I2C_NACK_ADDR code=5\n> ",
            False,
            expected_failure_patterns=runner.compile_token_patterns(["expected-nack"]),
        )
        self.assertEqual("FAIL", malformed)

    def test_expected_failure_can_be_scoped_to_one_command(self) -> None:
        class FakeSerial(FakeSerialBase):
            writes = []

            def write(self, payload: bytes) -> int:
                command = payload.decode().strip()
                type(self).writes.append(command)
                canonical = runner.canonical_command_name(command)
                failed = async_output(
                    canonical, 1, "detail=259 I2C_BUS code=18\n"
                ).replace("outcome=SUCCESS code=0", "outcome=FAILED code=18")
                self.buffer.extend(failed.encode())
                return len(payload)

        self.install_serial(FakeSerial)
        args = runner.parse_args([
            "--port", "FAKE", "--startup-delay-s", "0", "--idle-gap-s", "0.01",
            "--expected-failure", "resetreapply confirm=detail=259",
        ])
        results, _, _, _, _ = runner.run_serial_commands(
            args, ["resetreapply confirm", "init"]
        )
        self.assertEqual(["resetreapply confirm", "init"], FakeSerial.writes)
        self.assertEqual("PASS", results[0]["status"])
        self.assertEqual("FAIL", results[1]["status"])

        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            runner.parse_args(["--expected-failure", "missing-token"])

    def test_missing_base_expectation_prevents_soak_commands(self) -> None:
        commit = runner.git_value(["rev-parse", "--short", "HEAD"])

        class FakeSerial(FakeSerialBase):
            writes = []

            def write(self, payload: bytes) -> int:
                command = payload.decode().strip()
                type(self).writes.append(command)
                outputs = {
                    "version": version_output(commit),
                    "cfg": cfg_output(),
                    "drv": drv_output(),
                }
                self.buffer.extend(outputs[command].encode())
                return len(payload)

        self.install_serial(FakeSerial)
        args = runner.parse_args([
            "--port", "FAKE", "--profile", "arduino", "--fixture", "no-sensor",
            "--startup-delay-s", "0", "--idle-gap-s", "0.01",
            "--expected-firmware-commit", commit, "--board", "fake",
            "--include-long-soak", "--soak-duration-s", "0.1",
        ])
        _, _, _, _, soak = runner.run_serial_commands(
            args, ["version", "cfg", "drv"]
        )
        self.assertEqual(["version", "cfg", "drv"], FakeSerial.writes)
        self.assertIsNotNone(soak)
        self.assertFalse(soak["started"])
        self.assertIn("operator", soak["reason"])

    def test_exact_firmware_boot_banners_are_counted_during_soak(self) -> None:
        commit = runner.git_value(["rev-parse", "--short", "HEAD"])
        boot_banners = (
            "=== LDC1614 Arduino Diagnostic Bring-up Example ===",
            "=== LDC1614 Native ESP-IDF Diagnostic Bring-up Example ===",
        )

        for boot_banner in boot_banners:
            with self.subTest(banner=boot_banner):
                class FakeSerial(FakeSerialBase):
                    version_writes = 0

                    def write(self, payload: bytes) -> int:
                        command = payload.decode().strip()
                        if command == "version":
                            type(self).version_writes += 1
                            output = version_output(commit)
                            if type(self).version_writes > 1:
                                output = f"{boot_banner}\n{output}"
                        else:
                            outputs = {
                                "cfg": cfg_output(),
                                "drv": drv_output(),
                                "probe": probe_output(),
                                "status": status_output(),
                                "sleep": sync_output("sleep"),
                                "wake": sync_output("wake"),
                            }
                            output = outputs[command]
                        self.buffer.extend(output.encode())
                        return len(payload)

                self.install_serial(FakeSerial)
                args = runner.parse_args([
                    "--port", "FAKE", "--profile", "arduino",
                    "--fixture", "no-sensor", "--startup-delay-s", "0",
                    "--idle-gap-s", "0.01", "--expected-firmware-commit", commit,
                    "--operator", "test", "--board", "fake",
                    "--include-long-soak", "--soak-duration-s", "0.01",
                ])
                _, _, _, _, soak = runner.run_serial_commands(
                    args, ["version", "cfg", "drv"]
                )
                self.assertIsNotNone(soak)
                self.assertTrue(soak["started"])
                self.assertEqual("FAIL", soak["status"])
                self.assertEqual(soak["cycle_count"], soak["reset_count"])
                self.assertGreater(soak["reset_count"], 0)

    def test_mid_soak_exception_counts_no_partial_cycle(self) -> None:
        commit = runner.git_value(["rev-parse", "--short", "HEAD"])

        class FakeSerial(FakeSerialBase):
            writes = []

            def write(self, payload: bytes) -> int:
                command = payload.decode().strip()
                type(self).writes.append(command)
                if command == "status":
                    raise OSError("simulated mid-cycle disconnect")
                outputs = {
                    "version": version_output(commit),
                    "cfg": cfg_output(),
                    "drv": drv_output(),
                    "probe": probe_output(),
                }
                self.buffer.extend(outputs[command].encode())
                return len(payload)

        self.install_serial(FakeSerial)
        args = runner.parse_args([
            "--port", "FAKE", "--profile", "arduino", "--fixture", "no-sensor",
            "--startup-delay-s", "0", "--idle-gap-s", "0.01",
            "--expected-firmware-commit", commit,
            "--operator", "test", "--board", "fake",
            "--include-long-soak", "--soak-duration-s", "0.2",
            "--soak-cycle-delay-s", "0",
        ])
        with self.assertRaises(runner.SerialRunFailure) as captured:
            runner.run_serial_commands(args, ["version", "cfg", "drv"])
        soak = captured.exception.soak
        self.assertIsNotNone(soak)
        self.assertTrue(soak["started"])
        self.assertEqual(0, soak["cycle_count"])
        self.assertEqual(1, soak["incomplete_cycle"])
        self.assertEqual(
            ["version", "cfg", "drv", "version", "probe", "status"],
            FakeSerial.writes,
        )

    def test_mid_command_exception_keeps_raw_and_writes_fail_json(self) -> None:
        class FakeSerial(FakeSerialBase):
            writes = 0

            def write(self, payload: bytes) -> int:
                type(self).writes += 1
                if type(self).writes == 1:
                    self.buffer.extend(version_output().encode())
                    return len(payload)
                raise OSError("simulated mid-run disconnect")

        self.install_serial(FakeSerial)
        with tempfile.TemporaryDirectory() as temp:
            raw = Path(temp) / "raw.txt"
            result_path = Path(temp) / "result.json"
            exit_code = runner.main([
                "--port", "FAKE", "--startup-delay-s", "0", "--idle-gap-s", "0.01",
                "--skip-default-commands", "--command", "version", "--command", "cfg",
                "--raw-transcript-out", str(raw), "--json-out", str(result_path), "--quiet",
            ])
            result = json.loads(result_path.read_text(encoding="utf-8"))
            raw_text = raw.read_text(encoding="utf-8")
        self.assertEqual(1, exit_code)
        self.assertEqual("FAIL", result["overall_status"])
        self.assertEqual("serial_failure", result["evidence_type"])
        self.assertEqual("OSError", result["serial_failure"]["type"])
        self.assertEqual("3.0.0", result["firmware_version"])
        self.assertIn("### command 1: version", raw_text)
        self.assertIn("simulated mid-run disconnect", raw_text)

    def test_serial_context_exit_after_soak_keeps_complete_evidence_and_fails(self) -> None:
        commit = runner.git_value(["rev-parse", "--short", "HEAD"])

        class CloseFailSerial(FakeSerialBase):
            def __exit__(self, *_args):
                raise OSError("simulated close failure after soak")

            def write(self, payload: bytes) -> int:
                command = payload.decode().strip()
                outputs = {
                    "version": version_output(commit),
                    "cfg": cfg_output(),
                    "probe": probe_output(),
                    "status": status_output(),
                    "sleep": sync_output("sleep"),
                    "wake": sync_output("wake"),
                    "drv": drv_output(),
                }
                self.buffer.extend(outputs[command].encode())
                return len(payload)

        self.install_serial(CloseFailSerial)
        with tempfile.TemporaryDirectory() as temp:
            raw = Path(temp) / "raw.txt"
            result_path = Path(temp) / "result.json"
            exit_code = runner.main([
                "--port", "FAKE", "--profile", "arduino", "--fixture", "no-sensor",
                "--startup-delay-s", "0", "--idle-gap-s", "0.01",
                "--skip-default-commands", "--command", "version", "--command", "cfg",
                "--command", "drv",
                "--allow-reduced-soak-gate",
                "--expected-firmware-commit", commit, "--operator", "test", "--board", "fake",
                "--include-long-soak", "--soak-duration-s", "0.02",
                "--soak-cycle-delay-s", "0", "--raw-transcript-out", str(raw),
                "--json-out", str(result_path), "--quiet",
            ])
            result = json.loads(result_path.read_text(encoding="utf-8"))
            raw_text = raw.read_text(encoding="utf-8")
        self.assertEqual(1, exit_code)
        self.assertEqual("FAIL", result["overall_status"])
        self.assertEqual("serial_failure", result["evidence_type"])
        self.assertEqual("PASS", result["soak"]["status"])
        self.assertEqual(["wake", "drv"], result["commands"][-2:])
        self.assertGreaterEqual(result["soak"]["cycle_count"], 1)
        self.assertEqual(
            result["soak"]["cycle_count"] * len(runner.NO_SENSOR_SOAK_COMMANDS),
            result["soak"]["command_count"],
        )
        for command in runner.NO_SENSOR_SOAK_COMMANDS:
            self.assertEqual(
                result["soak"]["cycle_count"],
                result["soak"]["command_counts"][command]["PASS"],
            )
        self.assertIn("### soak cycle", raw_text)
        self.assertIn("simulated close failure after soak", raw_text)

    def test_serial_open_failure_is_explicit_fail_not_not_run(self) -> None:
        class OpenFailSerial:
            def __init__(self, *_args, **_kwargs):
                raise OSError("could not open port")

        self.install_serial(OpenFailSerial)
        args = runner.parse_args(["--port", "FAKE"])
        result = runner.make_result(args)
        self.assertEqual("FAIL", result["overall_status"])
        self.assertFalse(result["hardware_attached"])
        self.assertEqual("serial_failure", result["evidence_type"])
        self.assertEqual("OSError", result["serial_failure"]["type"])


class BoundsAndSelfTestTests(unittest.TestCase):
    def assert_parse_fails(self, arguments) -> None:
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            runner.parse_args(arguments)

    def test_parse_args_rejects_unbounded_values(self) -> None:
        for option in (
            "--startup-delay-s", "--command-timeout-s", "--write-timeout-s",
            "--idle-gap-s", "--soak-duration-s", "--soak-cycle-delay-s",
        ):
            with self.subTest(option=option):
                self.assert_parse_fails([option, "nan"])
        self.assert_parse_fails(["--stress-count", str(runner.MAX_STRESS_COUNT + 1)])
        self.assert_parse_fails(["--sample-rate-count", "1", "--sample-rate-channel", "4"])
        self.assert_parse_fails(["--address", "0x29"])

    def test_reduced_soak_requires_explicit_scope_acknowledgement(self) -> None:
        arguments = [
            "--dry-run", "--fixture", "no-sensor", "--skip-default-commands",
            "--command", "version", "--include-long-soak",
            "--soak-duration-s", "1",
        ]
        self.assert_parse_fails(arguments)
        args = runner.parse_args(arguments + ["--allow-reduced-soak-gate"])
        result = runner.make_result(args)
        self.assertEqual("custom_reduced", result["base_matrix_scope"])
        self.assertFalse(result["default_matrix_included"])
        self.assertEqual("custom_reduced", result["soak"]["base_matrix_scope"])
        self.assertIn("Base matrix scope: `custom_reduced`", runner.render_markdown(result))

    def test_parser_self_test_passes(self) -> None:
        self.assertEqual(0, runner.main(["--parser-self-test", "--quiet"]))

    def test_unknown_and_require_run_exit_codes(self) -> None:
        original_make = runner.make_result
        original_write = runner.write_outputs
        try:
            runner.make_result = lambda _args: {"overall_status": "UNKNOWN"}
            runner.write_outputs = lambda _args, _result: None
            self.assertEqual(3, runner.main(["--quiet"]))
        finally:
            runner.make_result = original_make
            runner.write_outputs = original_write
        self.assertEqual(2, runner.main(["--quiet", "--require-run"]))


if __name__ == "__main__":
    unittest.main()
