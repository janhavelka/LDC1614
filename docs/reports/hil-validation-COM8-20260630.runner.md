# LDC1614 HIL Run

Overall status: `NOT_RUN`
Timestamp UTC: `2026-06-30T14:46:43+00:00`
Git commit: `e96f117`
Git status: `dirty`
Library version: `1.0.0`
Firmware version: `unknown`
Profile: `arduino`
Port: `COM8`
Baud: `115200`
Expected address: `0x2A`
Expected channel count: `4`
Operator: `Codex`
Board: `ESP32-S2FH4 on COM8; LDC1614/LDC1612 fixture not confirmed`
Dry run: `False`
Serial port requested: `True`
Hardware attached: `False`
Evidence type: `serial_not_run`
Startup delay: `1.0` s
Startup read elapsed: `4.016` s
Command timeout: `4.0` s
Idle gap: `0.35` s
Notes: ESP32-S2 upload succeeded; bounded runner executed after target correction

## Commands

| # | Command | Status | Elapsed s | Reason |
| ---: | --- | --- | ---: | --- |
| 1 | `help` | `FAIL` | 4.000 | command response timed out |
| 2 | `version` | `FAIL` | 4.000 | command response timed out |
| 3 | `scan` | `FAIL` | 4.000 | command response timed out |
| 4 | `probe` | `FAIL` | 4.015 | command response timed out |
| 5 | `id` | `FAIL` | 4.000 | command response timed out |
| 6 | `drv` | `FAIL` | 4.016 | command response timed out |
| 7 | `cfg` | `FAIL` | 4.000 | command response timed out |
| 8 | `status` | `FAIL` | 4.000 | command response timed out |
| 9 | `sleep` | `FAIL` | 4.000 | command response timed out |
| 10 | `wake` | `FAIL` | 4.000 | command response timed out |
| 11 | `drdy` | `FAIL` | 4.016 | command response timed out |
| 12 | `read` | `FAIL` | 4.000 | command response timed out |
| 13 | `readfresh` | `FAIL` | 4.000 | command response timed out |
| 14 | `readstaged 0x01 8 1` | `FAIL` | 4.000 | command response timed out |
| 15 | `recover` | `FAIL` | 4.000 | command response timed out |
| 16 | `timing 0 43000000` | `FAIL` | 4.000 | command response timed out |
| 17 | `selftest` | `FAIL` | 4.000 | command response timed out |

## Skipped Optional Tests

| Test | Reason |
| --- | --- |
| - | No optional tests requested |

## Sample Rate

Status: `NOT_RUN`
Reason: sample-rate benchmark was not requested
Channel: `0`
Requested count: `0`
Observed count: `None`
Failures: `None`
Elapsed s: `0.000`
Effective Hz: `None`

## Stress

Status: `NOT_RUN`
Reason: stress was not requested
Requested count: `10`
Success count: `None`
Failure count: `None`
Elapsed s: `0.000`
Effective Hz: `None`

## Soak

Status: `NOT_RUN`
Reason: long soak was not requested
Requested duration s: `0`
Elapsed s: `0.000`
Failure count: `None`
Recovery count: `None`
Reset count: `None`
Worst latency s: `None`

## Transcript

```text
### startup

### command 1: help

### command 2: version

### command 3: scan

### command 4: probe

### command 5: id

### command 6: drv

### command 7: cfg

### command 8: status

### command 9: sleep

### command 10: wake

### command 11: drdy

### command 12: read

### command 13: readfresh

### command 14: readstaged 0x01 8 1

### command 15: recover

### command 16: timing 0 43000000

### command 17: selftest

```
