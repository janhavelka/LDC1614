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
        self.assertIn("cfg", commands)  # cached settings snapshot
        self.assertIn("drv", commands)  # driver health snapshot

    def test_default_idf_commands_cover_supported_safe_diagnostics(self) -> None:
        commands = runner.default_commands("idf")

        self.assertIn("version", commands)
        self.assertIn("probe", commands)
        self.assertIn("cfg", commands)
        self.assertIn("drv", commands)
        self.assertNotIn("scan", commands)

    def test_classifier_accepts_common_informational_outputs(self) -> None:
        cases = (
            ("version", "version: 1.0.0\n> "),
            ("scan", "I2C scan complete\n0x2A\n> "),
            ("settings", "addr=0x2a channels=4 timeoutMs=10\n> "),
            ("health", "state=READY initialized=1 online=1 dirty=0\n> "),
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
            "not initialized\n> ",
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

    def test_classifier_allows_benign_error_words_without_failure_tokens(self) -> None:
        status, reason = runner.classify_command(
            "health", "error counters: 0\nlast error: never\n> ", False
        )

        self.assertEqual("PASS", status, reason)

    def test_classifier_fails_empty_or_timed_out_response(self) -> None:
        status, _ = runner.classify_command("version", "", False)
        self.assertEqual("FAIL", status)

        status, _ = runner.classify_command("version", "version: 1.0.0", True)
        self.assertEqual("FAIL", status)

    def test_overall_status_never_passes_without_transcript(self) -> None:
        command_results = [{"status": "PASS", "command": "version"}]

        self.assertEqual("NOT_RUN", runner.overall_status(command_results, None, ""))
        self.assertEqual("PASS", runner.overall_status(command_results, None, "version: 1.0.0\n> "))

    def test_expectation_checks_fail_reported_address_or_channel_mismatch(self) -> None:
        args = runner.parse_args(["--address", "0x2A", "--channel-count", "4"])
        command_results = [{"index": 1, "command": "cfg", "status": "PASS", "reason": "ok"}]

        runner.append_expectation_results(
            args,
            command_results,
            "addr=0x2b channels=2 timeoutMs=10\n> ",
        )

        self.assertEqual("FAIL", command_results[1]["status"])
        self.assertEqual("expect-address", command_results[1]["command"])
        self.assertEqual("FAIL", command_results[2]["status"])
        self.assertEqual("expect-channel-count", command_results[2]["command"])

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
            self.assertIn("read 0 50", result["commands"])
            self.assertTrue(
                all(item["status"] == "NOT_RUN" for item in result["command_results"])
            )
            self.assertIn("| # | Command | Status | Elapsed s | Reason |", markdown)

    def test_sample_rate_idf_is_skipped_without_parallel_framework(self) -> None:
        args = runner.parse_args(["--profile", "idf", "--dry-run", "--sample-rate-count", "10"])
        result = runner.make_result(args)

        self.assertNotIn("read 0 10", result["commands"])
        self.assertEqual("sample_rate_benchmark", result["skipped_optional_tests"][0]["name"])

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
