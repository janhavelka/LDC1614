#!/usr/bin/env python3
"""Host-side tests for the conservative LDC1614 HIL runner."""

from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import ldc1614_hil_runner as runner


class HilRunnerParserTests(unittest.TestCase):
    def test_default_arduino_commands_cover_safe_diagnostics(self) -> None:
        commands = runner.default_commands("arduino")

        self.assertIn("version", commands)
        self.assertIn("scan", commands)
        self.assertIn("probe", commands)
        self.assertIn("cfg", commands)
        self.assertIn("drv", commands)
        self.assertIn("progress", commands)
        self.assertIn("timing 0x01", commands)
        self.assertFalse(any(command.startswith("acquire") for command in commands))

    def test_no_sensor_fixture_excludes_conversion_checks(self) -> None:
        commands = runner.default_commands("arduino", "no-sensor")

        self.assertIn("progress", commands)
        self.assertIn("reg 0x7E", commands)
        self.assertIn("reg 0x7F", commands)
        self.assertNotIn("drdy", commands)
        self.assertFalse(any(command.startswith("acquire") for command in commands))
        self.assertNotIn("samplerate 0 10", commands)

    def test_default_idf_commands_cover_supported_safe_diagnostics(self) -> None:
        commands = runner.default_commands("idf")

        self.assertIn("version", commands)
        self.assertIn("probe", commands)
        self.assertIn("cfg", commands)
        self.assertIn("drv", commands)
        self.assertIn("progress", commands)
        self.assertIn("timing 0x01", commands)
        self.assertNotIn("scan", commands)

    def test_serial_line_defaults_match_known_esp32s2_fixture(self) -> None:
        args = runner.parse_args([])
        self.assertEqual("on", args.serial_dtr)
        self.assertEqual("off", args.serial_rts)

    def test_classifier_accepts_common_informational_outputs(self) -> None:
        cases = (
            ("version", "version: 1.0.0 firmware_git=abcdef1 firmware_status=clean\n> "),
            ("scan", "I2C device at 0x2A\nscan complete found=1 probes=126\nstatus: code=0\n> "),
            ("settings", "address=0x2a variant=2 variantChannels=4 selected=0x01 timeoutMs=10\n> "),
            ("status", "STATUS observed=1 raw=0x0000 drdy=0 unread=0x00 errCh=4\n> "),
            ("health", "bound=1 applied=APPLIED_SLEEPING revision=1\n> "),
            ("progress", "active=0 operation=0 transfers=0/0\n> "),
            ("reg 0x7E", "reg 0x7E = 0x5449\n> "),
        )

        for command, output in cases:
            with self.subTest(command=command):
                status, reason = runner.classify_command(command, output, False)
                self.assertEqual("PASS", status, reason)

    def test_classifier_requires_ok_for_probe(self) -> None:
        status, reason = runner.classify_command(
            "probe", "MANUFACTURER_ID=0x5449 DEVICE_ID=0x3055 status: code=0\n> ", False
        )

        self.assertEqual("PASS", status, reason)

    def test_classifier_detects_failure_tokens(self) -> None:
        failure_outputs = (
            "status: I2C_TIMEOUT code=7\n> ",
            "DEVICE_NOT_FOUND\n> ",
            "not bound\n> ",
            "unknown command: health\n> ",
            "read failed\n> ",
            "errors=2\n> ",
            "[FAIL] readRegister16(DEV_ID)\nstatus: code=0\n> ",
            "Selftest result: pass=4 fail=1 skip=0\nstatus: code=0\n> ",
            "Demo result: ready=1/5 read_ok=4 read_fail=1\nstatus: code=0\n> ",
            "Value: 0x1234 expected=0x5449 match=NO\nstatus: code=0\n> ",
            "ch0 raw=0x0000000 errUR=1 errOR=0 errWD=0 errAmp=0\nstatus: code=0\n> ",
            "raw=0x2140 drdy=1 err=1 ur=1 or=0 wd=0 ah=0 al=0 zc=0\nstatus: code=0\n> ",
            "\x1b[31m[ERR: UR=1 OR=0 WD=0 AE=0]\x1b[0m\nstatus: code=0\n> ",
        )

        for output in failure_outputs:
            with self.subTest(output=output):
                status, reason = runner.classify_command("probe", output, False)
                self.assertEqual("FAIL", status, reason)

    def test_classifier_accepts_terminal_operation_result(self) -> None:
        status, reason = runner.classify_command(
            "custom-acquire",
            (
                "result operation=42 kind=4 outcome=SUCCESS effects=0x01\n"
                "status: code=0 detail=0 msg=OK\n"
                "batch selected=0x01 valid=0x01 fresh=0x01\n> "
            ),
            False,
        )

        self.assertEqual("PASS", status, reason)

    def test_classifier_supports_configured_tokens(self) -> None:
        expected = runner.compile_token_patterns(["fixture ready"])
        failure = runner.compile_token_patterns(["fixture unsafe"])
        expected_failure = runner.compile_token_patterns(["INVALID_PARAM"])

        status, _ = runner.classify_command(
            "fixture", "fixture ready\n> ", False, expected_patterns=expected
        )
        self.assertEqual("PASS", status)

        status, _ = runner.classify_command(
            "fixture", "fixture unsafe\n> ", False, failure_patterns=failure
        )
        self.assertEqual("FAIL", status)

        status, _ = runner.classify_command(
            "negative",
            "status: INVALID_PARAM code=5\n> ",
            False,
            expected_failure_patterns=expected_failure,
        )
        self.assertEqual("PASS", status)

    def test_classifier_rejects_benign_but_unstructured_output(self) -> None:
        status, reason = runner.classify_command(
            "health", "error counters: 0\nlast error: never\n> ", False
        )

        self.assertEqual("FAIL", status, reason)

    def test_classifier_fails_empty_or_timed_out_response(self) -> None:
        status, _ = runner.classify_command("version", "", False)
        self.assertEqual("FAIL", status)

        status, _ = runner.classify_command("version", "version: 1.0.0", True)
        self.assertEqual("FAIL", status)

    def test_classifier_uses_unknown_for_ambiguous_output(self) -> None:
        status, reason = runner.classify_command("custom", "fixture text\n> ", False)

        self.assertEqual("UNKNOWN", status, reason)

    def test_overall_status_never_passes_without_transcript(self) -> None:
        command_results = [{"status": "PASS", "command": "version"}]

        self.assertEqual("NOT_RUN", runner.overall_status(command_results, None, ""))
        self.assertEqual("PASS", runner.overall_status(command_results, None, "version: 1.0.0\n> "))
        self.assertEqual("NOT_RUN", runner.overall_status(command_results, None, "### startup\n"))
        self.assertEqual(
            "UNKNOWN",
            runner.overall_status([{"status": "UNKNOWN", "command": "x"}], None, "x\n> "),
        )

    def test_expectation_checks_fail_reported_address_or_channel_mismatch(self) -> None:
        args = runner.parse_args([
            "--address", "0x2A", "--channel-count", "4",
            "--expected-firmware-commit", "abcdef1",
        ])
        command_results = [{"index": 1, "command": "cfg", "status": "PASS", "reason": "ok"}]

        runner.append_expectation_results(
            args,
            command_results,
            "address=0x2b variantChannels=2 timeoutMs=10 "
            "firmware_git=abcdef1 firmware_status=clean\n> ",
        )

        self.assertEqual("FAIL", command_results[1]["status"])
        self.assertEqual("expect-address", command_results[1]["command"])
        self.assertEqual("FAIL", command_results[2]["status"])
        self.assertEqual("expect-channel-count", command_results[2]["command"])

    def test_expectation_checks_require_all_target_facts_and_match_commit(self) -> None:
        args = runner.parse_args([
            "--address", "0x2A", "--channel-count", "4",
            "--expected-firmware-commit", "abcdef1",
        ])
        results = [{"index": 1, "command": "version", "status": "PASS", "reason": "ok"}]
        runner.append_expectation_results(
            args, results, "firmware_git=1234567 firmware_status=dirty\n> "
        )
        failures = {item["command"] for item in results if item["status"] == "FAIL"}
        self.assertEqual(
            {"expect-address", "expect-channel-count", "expect-firmware-commit",
             "expect-clean-firmware"},
            failures,
        )

        passing = [{"index": 1, "command": "cfg", "status": "PASS", "reason": "ok"}]
        runner.append_expectation_results(
            args,
            passing,
            "address=0x2a variantChannels=4 firmware_git=abcdef123 "
            "firmware_status=clean\n> ",
        )
        self.assertEqual(1, len(passing))

    def test_no_port_writes_not_run_artifacts(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            json_out = Path(temp_dir) / "hil.json"
            markdown_out = Path(temp_dir) / "hil.md"

            exit_code = runner.main(
                [
                    "--json-out",
                    str(json_out),
                    "--markdown-out",
                    str(markdown_out),
                    "--quiet",
                ]
            )

            self.assertEqual(0, exit_code)
            result = json.loads(json_out.read_text(encoding="utf-8"))
            markdown = markdown_out.read_text(encoding="utf-8")

            self.assertEqual("NOT_RUN", result["overall_status"])
            self.assertEqual("serial port was not supplied", result["not_run_reason"])
            self.assertFalse(result["hardware_attached"])
            self.assertEqual("no_hardware_audit", result["evidence_type"])
            self.assertEqual("", result["transcript"])
            self.assertEqual([], result["command_results"])
            self.assertIn("Overall status: `NOT_RUN`", markdown)
            self.assertIn("Evidence type: `no_hardware_audit`", markdown)
            self.assertIn("No serial command transcript captured", markdown)

    def test_dry_run_lists_bounded_not_run_command_rows(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            json_out = Path(temp_dir) / "hil.json"
            markdown_out = Path(temp_dir) / "hil.md"

            exit_code = runner.main(
                [
                    "--dry-run",
                    "--json-out",
                    str(json_out),
                    "--markdown-out",
                    str(markdown_out),
                    "--quiet",
                    "--sample-rate-count",
                    "50",
                ]
            )

            self.assertEqual(0, exit_code)
            result = json.loads(json_out.read_text(encoding="utf-8"))
            markdown = markdown_out.read_text(encoding="utf-8")

            self.assertEqual("NOT_RUN", result["overall_status"])
            self.assertEqual("no_hardware_audit", result["evidence_type"])
            self.assertGreater(len(result["command_results"]), 0)
            self.assertNotIn("samplerate 0 50", result["commands"])
            self.assertEqual(
                "sample_rate_benchmark",
                result["skipped_optional_tests"][0]["name"],
            )
            self.assertEqual("NOT_RUN", result["sample_rate"]["status"])
            self.assertTrue(
                all(item["status"] == "NOT_RUN" for item in result["command_results"])
            )
            self.assertIn("| # | Command | Status | Elapsed s | Reason |", markdown)
            self.assertIn("## Sample Rate", markdown)

    def test_sample_rate_idf_is_skipped_without_parallel_framework(self) -> None:
        args = runner.parse_args(["--profile", "idf", "--dry-run", "--sample-rate-count", "10"])
        result = runner.make_result(args)

        self.assertNotIn("samplerate 0 10", result["commands"])
        self.assertEqual("sample_rate_benchmark", result["skipped_optional_tests"][0]["name"])

    def test_no_sensor_fixture_skips_optional_conversion_benchmarks(self) -> None:
        args = runner.parse_args(
            [
                "--fixture",
                "no-sensor",
                "--dry-run",
                "--include-stress",
                "--sample-rate-count",
                "10",
            ]
        )
        result = runner.make_result(args)

        skipped = {item["name"]: item["reason"] for item in result["skipped_optional_tests"]}
        self.assertIn("stress", skipped)
        self.assertIn("sample_rate_benchmark", skipped)
        self.assertNotIn("stress 10", result["commands"])
        self.assertNotIn("samplerate 0 10", result["commands"])
        self.assertEqual("no-sensor", result["fixture"])

    def test_repeat_command_set_expands_selected_commands(self) -> None:
        args = runner.parse_args(
            [
                "--dry-run",
                "--skip-default-commands",
                "--command",
                "version",
                "--command",
                "cfg",
                "--repeat-command-set",
                "3",
            ]
        )
        result = runner.make_result(args)

        self.assertEqual(["version", "cfg", "version", "cfg", "version", "cfg"],
                         result["commands"])
        self.assertEqual(2, result["base_command_count"])
        self.assertEqual(3, result["repeat_command_set"])
        self.assertEqual(6, len(result["command_results"]))

    def test_sample_rate_summary_parses_gated_samplerate_command(self) -> None:
        args = runner.parse_args(["--sample-rate-count", "50"])
        command_results = [
            {
                "command": "samplerate 0 50",
                "status": "PASS",
                "elapsed_s": 1.25,
                "output": (
                    "SampleRate result: requested=50 ok=50 fail=0 "
                    "elapsed_ms=1250 hz=40.000 worst_ms=4 "
                    "first_raw=0x0000001 last_raw=0x0000002\n> "
                ),
            }
        ]

        summary = runner.summarize_sample_rate(args, command_results)

        self.assertEqual("PASS", summary["status"])
        self.assertEqual(50, summary["observed_count"])
        self.assertEqual(0, summary["failure_count"])
        self.assertEqual(40.0, summary["effective_hz"])

    def test_serial_open_failure_is_not_hardware_hil_evidence(self) -> None:
        args = runner.parse_args(["--port", "COM404"])
        original = runner.run_serial_commands

        def raise_open_failure(_args, _commands):
            raise RuntimeError("could not open port")

        try:
            runner.run_serial_commands = raise_open_failure
            result = runner.make_result(args)
        finally:
            runner.run_serial_commands = original

        self.assertEqual("NOT_RUN", result["overall_status"])
        self.assertFalse(result["hardware_attached"])
        self.assertEqual("serial_not_run", result["evidence_type"])

    def test_parse_args_rejects_unbounded_timeouts_and_stress(self) -> None:
        with self.assertRaises(SystemExit):
            runner.parse_args(["--command-timeout-s", "-1"])
        with self.assertRaises(SystemExit):
            runner.parse_args(["--stress-count", str(runner.MAX_STRESS_COUNT + 1)])
        with self.assertRaises(SystemExit):
            runner.parse_args(["--address", "0x29"])
        with self.assertRaises(SystemExit):
            runner.parse_args(["--expected-firmware-commit", "not-a-sha"])

    def test_unknown_verification_result_exits_nonzero(self) -> None:
        original_make = runner.make_result
        original_write = runner.write_outputs
        try:
            runner.make_result = lambda _args: {"overall_status": "UNKNOWN"}
            runner.write_outputs = lambda _args, _result: None
            self.assertEqual(3, runner.main(["--quiet"]))
        finally:
            runner.make_result = original_make
            runner.write_outputs = original_write

    def test_parser_self_test_passes(self) -> None:
        self.assertEqual(0, runner.main(["--parser-self-test", "--quiet"]))

    def test_no_port_require_run_exits_nonzero(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            json_out = Path(temp_dir) / "hil.json"

            exit_code = runner.main(["--json-out", str(json_out), "--quiet", "--require-run"])

            self.assertEqual(2, exit_code)
            result = json.loads(json_out.read_text(encoding="utf-8"))
            self.assertEqual("NOT_RUN", result["overall_status"])


if __name__ == "__main__":
    unittest.main()
