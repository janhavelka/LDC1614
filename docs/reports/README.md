# Retained HIL Evidence

This directory preserves transcript-bearing hardware evidence. It is an
evidence archive, not a release certificate; current acceptance gaps are listed
in [Validation status](../VALIDATION_STATUS.md).

## Structured runs

The five `hil-validation-COM8-20260722-*.runner.json` files are historical
post-v3 negative transport-regression runs. Each JSON file contains the complete
startup and command transcript plus machine-readable command results and target
firmware identity. Generated `.runner.md` copies are intentionally not retained.

Fixture: ESP32-S2 on COM8, LDC1614 at `0x2A`, a second responding device at
`0x3C`, and no LC sensor. Sensor-condition flags were permitted; transport,
identity, timeout, and nonzero-status failures were not.

| Firmware | Stack / recovery attempt | Retained observation |
|---|---|---|
| `a11fcb4` | pioarduino `54.03.20`, 400 kHz | Combined identity reads intermittently returned length mismatch, zero/`0xFFFF`, and `ESP_ERR_INVALID_STATE` after a scan found `0x2A` and `0x3C`. |
| `bf44cf1` | pioarduino `53.03.13`, NACK-heavy startup scan | 24 of 39 smoke commands failed after combined reads latched at zero length; the MCU and CLI remained responsive. |
| `ac710c1` | Wire teardown/rebegin after each scan | All 200 commands ran, but 16 of 25 following initialization reads failed despite reported recovery success. |
| `05a71d7` | Shared new-master delete/recreate | 20 of 25 following initialization reads failed with explicit `ESP_ERR_INVALID_STATE`; scans also produced false devices or timeouts. |
| `3036579` | ESP-IDF 5.3 public bus reset | 87 of 200 commands failed, including 19 of 25 initialization attempts; bus reset did not repair the stale state. |

This comparison selected an upgrade containing Espressif's earlier NACK
reporting fix (`459b75f`) instead of another core-library workaround. It did
not close the separate persistent-invalid-state problem tracked by
[Espressif issue #14030](https://github.com/espressif/esp-idf/issues/14030).

Compact 2026-07-01 v2 artifacts had no raw transcript and did not exercise the
v3 ownership contract, so they are available only through Git history.

## Raw serial transcripts

`transcripts/20260722/` contains eight raw serial captures recovered from the
old PlatformIO work directory before build-output cleanup:

- `hil-identity-characterization.serial.txt`
- `hil-inplace-identity-25x.serial.txt`
- `hil-post-failure-scan.serial.txt`
- `hil-transport-debug-100k.serial.txt`
- `hil-transport-debug-400k.serial.txt`
- `hil-transport-debug-400k-no-scan.serial.txt`
- `hil-transport-debug-400k-pio53.serial.txt`
- `hil-transport-debug-400k-pio53-cold.serial.txt`

These captures are historical debug evidence, including failures and dirty
firmware runs. They are not positive exact-release acceptance evidence.

## 2026-08-03 pioarduino 55.03.311 run

`20260803/` retains the exact clean-firmware diagnostics and the failed
one-hour invocation for ESP32-S2 COM8, LDC1614 at `0x2A`, a second device at
`0x3C`, and no LC sensor:

- `preflight.*`: 5/5 commands passed on clean `1263ab1`.
- `full-matrix.*`: 41/41 commands passed.
- `full-matrix-25x.*`: 1,025/1,025 command results passed.
- `hil-one-hour.*`: completed the requested 3,600-second duration, but base command 26
  (`resetreapply`) failed at the first identity read with backend detail 259
  (`ESP_ERR_INVALID_STATE`). The old runner incorrectly continued into 3,204
  meaningless soak cycles; its 9,674 subsequent failures are retained as
  negative evidence, not counted as an accepted soak.
- `postfix-matrix.*`: clean `fd5ccb3` firmware with an experimental 2 ms
  post-reset guard reproduced the same failure, disproving that delay as a
  corrective fix.

The runner now gates soak entry on a complete passing base matrix, preserves
partial serial evidence on exceptions, and counts only complete cycles. A
physical power cycle is required before the next controlled cold-start gate.
Temporary dirty-firmware investigations are intentionally excluded from this
curated evidence bundle and are not used to support acceptance conclusions.

Positive acceptance still requires a clean firmware identity match, the full
no-sensor matrix, and a bounded soak with no reset or ambiguous response.
Sensor conversion, INTB, SD, `0x2B`, LDC1612, coil behavior, and ESP32-S3
product-owner behavior remain outside this archive.
