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
        )

        for output in failure_outputs:
            with self.subTest(output=output):
                status, reason = runner.classify_command("probe", output, False)
                self.assertEqual("FAIL", status, reason)

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
            self.assertEqual("", result["transcript"])
            self.assertEqual([], result["command_results"])
            self.assertIn("Overall status: `NOT_RUN`", markdown)
            self.assertIn("No serial command transcript captured", markdown)

    def test_no_port_require_run_exits_nonzero(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            json_out = Path(temp_dir) / "hil.json"

            exit_code = runner.main(["--json-out", str(json_out), "--quiet", "--require-run"])

            self.assertEqual(2, exit_code)
            result = json.loads(json_out.read_text(encoding="utf-8"))
            self.assertEqual("NOT_RUN", result["overall_status"])


if __name__ == "__main__":
    unittest.main()
