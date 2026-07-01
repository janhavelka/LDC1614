# LDC1614 HIL Run - Compact Summary

Overall status: `PASS`
Timestamp UTC: `2026-07-01T14:29:03+00:00`
Profile: `arduino`
Fixture: `no-sensor`
Port: `COM8`
Board: `ESP32-S2 COM8 with LDC1614 at 0x2A, no LC sensor attached`
Library version: `2.0.0`
Firmware version: `2.0.0`
Runner git commit: `42feb3b`
Runner git status: `clean`
Repeat command set: `10`
Base command count: `101`
Evidence type: `hardware_hil`
Hardware attached: `True`

## Summary

Total commands executed: `1010`
Passed: `1010`
Non-pass: `0`
Timeouts: `0`

## Firmware Identity

Full version: `2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)`
Build timestamp: `2026-07-01 16:27:00`
Firmware git commit: `42feb3b`
Firmware git status: `clean`

## Device Identity

Address: `0x2A`
MANUFACTURER_ID: `0x5449`
DEVICE_ID: `0x3055`

## Compaction Note

Per-command serial output and full repeated transcript were omitted to keep the release artifact reviewable. Command counts, per-base-command outcomes, metadata, firmware identity, and non-pass details are retained.

## Base Command Coverage

| # | Command | Runs | Pass | Non-pass | Timeouts | Status | Reasons |
| ---: | --- | ---: | ---: | ---: | ---: | --- | --- |
| 1 | `help` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 2 | `version` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 3 | `init` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 4 | `scan` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 5 | `probeaddr 0x2A` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 6 | `probe` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 7 | `id` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 8 | `drv` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 9 | `state` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 10 | `online` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 11 | `cfg` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 12 | `snapshot` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 13 | `channels` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 14 | `activech` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 15 | `status` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 16 | `status_raw` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 17 | `rawreg 0x7E` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 18 | `rawreg 0x7F` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 19 | `reg 0x7E` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 20 | `reg 0x7F` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 21 | `reg 0x19` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 22 | `reg 0x1A` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 23 | `reg 0x1B` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 24 | `sleep` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 25 | `wake` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 26 | `sleep` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 27 | `single 0` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 28 | `rcount 0 0x0123` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 29 | `settle 0 0x0011` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 30 | `clkdiv 0 2 3` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 31 | `offset 0 0x0010` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 32 | `idrive 0 5` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 33 | `rcount 1 0x0124` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 34 | `settle 1 0x0012` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 35 | `clkdiv 1 2 4` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 36 | `offset 1 0x0011` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 37 | `idrive 1 6` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 38 | `rcount 2 0x0125` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 39 | `settle 2 0x0013` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 40 | `clkdiv 2 2 5` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 41 | `offset 2 0x0012` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 42 | `idrive 2 7` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 43 | `rcount 3 0x0126` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 44 | `settle 3 0x0014` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 45 | `clkdiv 3 2 6` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 46 | `offset 3 0x0013` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 47 | `idrive 3 8` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 48 | `initidrive 0` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 49 | `initidrive 1` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 50 | `initidrive 2` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 51 | `initidrive 3` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 52 | `cfg` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 53 | `activech 1` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 54 | `activech 2` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 55 | `activech 3` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 56 | `single 0` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 57 | `single 1` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 58 | `single 2` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 59 | `single 3` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 60 | `single 0` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 61 | `autoscan 2` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 62 | `autoscan 3` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 63 | `autoscan 4` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 64 | `single 0` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 65 | `deglitch 1` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 66 | `deglitch 3` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 67 | `deglitch 10` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 68 | `deglitch 33` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 69 | `errcfg 0x0000` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 70 | `errcfg 0x00F9` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 71 | `errcfg` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 72 | `intb 0` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 73 | `intb 1` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 74 | `intb 0` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 75 | `refclk ext` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 76 | `refclk int` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 77 | `activate low` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 78 | `activate full` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 79 | `rpoverride 0` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 80 | `rpoverride 1` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 81 | `autoamp 1` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 82 | `autoamp 0` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 83 | `highcurrent 1` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 84 | `highcurrent 0` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 85 | `cfg` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 86 | `wreg 0x19 0x00F9` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 87 | `sync` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 88 | `cfg` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 89 | `rawwreg 0x19 0x00F9` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 90 | `cfg` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 91 | `resetreapply` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 92 | `cfg` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 93 | `recover` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 94 | `timing 0 43000000` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 95 | `timing 1 43000000` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 96 | `timing 2 43000000` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 97 | `timing 3 43000000` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 98 | `reset` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
| 99 | `init` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 100 | `cfg` | 10 | 10 | 0 | 0 | PASS | informational command responded without failure pattern (10) |
| 101 | `sleep` | 10 | 10 | 0 | 0 | PASS | status output indicates OK (10) |
