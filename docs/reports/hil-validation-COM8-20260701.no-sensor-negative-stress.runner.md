# LDC1614 HIL Run - Compact Summary

Overall status: `PASS`
Timestamp UTC: `2026-07-01T14:30:11+00:00`
Profile: `arduino`
Fixture: `no-sensor`
Port: `COM8`
Board: `ESP32-S2 COM8 with LDC1614 at 0x2A, no LC sensor attached`
Library version: `2.0.0`
Firmware version: `2.0.0`
Runner git commit: `6c88592`
Runner git status: `clean`
Repeat command set: `10`
Base command count: `20`
Evidence type: `hardware_hil`
Hardware attached: `True`

## Summary

Total commands executed: `200`
Passed: `200`
Non-pass: `0`
Timeouts: `0`

## Firmware Identity

Full version: `2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)`
Build timestamp: `2026-07-01 16:27:00`
Firmware git commit: `42feb3b`
Firmware git status: `clean`

## Device Identity

Address: `0x2A`
MANUFACTURER_ID: `None`
DEVICE_ID: `None`

## Compaction Note

Per-command serial output and full repeated transcript were omitted to keep the release artifact reviewable. Command counts, per-base-command outcomes, metadata, firmware identity, and non-pass details are retained.

## Base Command Coverage

| # | Command | Runs | Pass | Non-pass | Timeouts | Status | Reasons |
| ---: | --- | ---: | ---: | ---: | ---: | --- | --- |
| 1 | `version` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 2 | `init` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 3 | `wake` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 4 | `rcount 0 0x0123` | 10 | 10 | 0 | 0 | PASS | matched configured expected-failure token: BUSY (10) |
| 5 | `sleep` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 6 | `rcount 4 0x0123` | 10 | 10 | 0 | 0 | PASS | matched configured expected-failure token: INVALID_PARAM (10) |
| 7 | `settle 4 0x0011` | 10 | 10 | 0 | 0 | PASS | matched configured expected-failure token: INVALID_PARAM (10) |
| 8 | `clkdiv 4 2 3` | 10 | 10 | 0 | 0 | PASS | matched configured expected-failure token: INVALID_PARAM (10) |
| 9 | `offset 4 0x0010` | 10 | 10 | 0 | 0 | PASS | matched configured expected-failure token: INVALID_PARAM (10) |
| 10 | `idrive 4 5` | 10 | 10 | 0 | 0 | PASS | matched configured expected-failure token: INVALID_PARAM (10) |
| 11 | `rcount 0 0x0004` | 10 | 10 | 0 | 0 | PASS | matched configured expected-failure token: INVALID_PARAM (10) |
| 12 | `clkdiv 0 0 1` | 10 | 10 | 0 | 0 | PASS | matched configured expected-failure token: INVALID_PARAM (10) |
| 13 | `clkdiv 0 1 0` | 10 | 10 | 0 | 0 | PASS | matched configured expected-failure token: INVALID_PARAM (10) |
| 14 | `idrive 0 32` | 10 | 10 | 0 | 0 | PASS | matched configured expected-failure token: INVALID_PARAM (10) |
| 15 | `errcfg 0x0002` | 10 | 10 | 0 | 0 | PASS | matched configured expected-failure token: INVALID_PARAM (10) |
| 16 | `highcurrent 1` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 17 | `autoscan 2` | 10 | 10 | 0 | 0 | PASS | matched configured expected-failure token: INVALID_PARAM (10) |
| 18 | `highcurrent 0` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 19 | `cfg` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 20 | `sleep` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
