# LDC1614 HIL Run

Overall status: `FAIL`
Timestamp UTC: `2026-07-22T13:59:09+00:00`
Host checkout Git commit: `a11fcb4`
Host checkout Git status: `dirty`
Library version: `3.0.0`
Firmware version: `3.0.0`
Firmware-reported Git commit: `a11fcb4`
Firmware-reported Git status: `clean`
Expected firmware Git commit: `a11fcb4c497de691c514a2a841a1fa1e94d47979`
Profile: `arduino`
Fixture: `no-sensor`
Port: `COM8`
Baud: `115200`
Expected address: `0x2A`
Expected channel count: `4`
Operator: `Codex`
Board: `ESP32-S2 COM8 with LDC1614 at 0x2A, no LC sensor attached`
Dry run: `False`
Serial port requested: `True`
Serial DTR/RTS: `on` / `off`
Hardware attached: `True`
Evidence type: `hardware_hil`
Startup delay: `1.0` s
Startup read elapsed: `4.000` s
Command timeout: `4.0` s
Idle gap: `0.35` s
Repeat command set: `1`
Base command count: `21`
Notes: v3.0.0 post-release no-sensor smoke before one-hour soak

## Commands

| # | Command | Status | Elapsed s | Reason |
| ---: | --- | --- | ---: | --- |
| 1 | `help` | `PASS` | 0.031 | all command-specific evidence parsed |
| 2 | `version` | `PASS` | 0.016 | all command-specific evidence parsed |
| 3 | `scan` | `PASS` | 0.016 | all command-specific evidence parsed |
| 4 | `probe` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 5 | `drv` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |
| 6 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 7 | `progress` | `PASS` | 0.016 | all command-specific evidence parsed |
| 8 | `status` | `PASS` | 0.031 | all command-specific evidence parsed |
| 9 | `reg 0x7E` | `PASS` | 0.016 | all command-specific evidence parsed |
| 10 | `reg 0x7F` | `PASS` | 0.015 | all command-specific evidence parsed |
| 11 | `reg 0x19` | `PASS` | 0.031 | all command-specific evidence parsed |
| 12 | `reg 0x1A` | `PASS` | 0.016 | all command-specific evidence parsed |
| 13 | `reg 0x1B` | `PASS` | 0.016 | all command-specific evidence parsed |
| 14 | `sleep` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 15 | `wake` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 16 | `sleep` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |
| 17 | `initidrive 0` | `PASS` | 0.031 | all command-specific evidence parsed |
| 18 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 19 | `timing 0x01` | `PASS` | 0.016 | all command-specific evidence parsed |
| 20 | `selftest` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 21 | `sleep` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |

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
Cycles: `None`
Commands: `None`
Failure count: `None`
Unknown count: `None`
Recovery count: `None`
Reset count: `None`
Worst latency s: `None`

## Transcript

```text
### startup

### command 1: help
Owner-driven jobs: init, apply, resetreapply, acquire [mask], cancel, progress
One-transfer controls: status, ready/drdy, sleep, wake, initdrive <ch>
Diagnostics: version, scan, probe/id, drv/state, cfg/settings, reg, wreg
Pure helpers: timing [mask], freq <ch> <raw28>
Lifecycle: bind, invalidate, end; results print automatically from service()
>
### command 2: version
version: 3.0.0 firmware_git=a11fcb4 firmware_status=clean build_timestamp=2026-07-22 15:34:34
>
### command 3: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=126
status: code=0 detail=0 msg=OK
>
### command 4: probe
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 5: drv
bound=1 applied=UNKNOWN configRevision=1 active=0 resultAvailable=0
transport attempts=3 success=1 failures=2
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 6: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 7: progress
active=0 operation=0 kind=0 phase=0 transfers=0/0 requested=0x00 completed=0x00 deadlineMs=0
>
### command 8: status
STATUS observed=1 raw=0x0000 drdy=0 unread=0x00 errCh=255 UR=0 OR=0 WD=0 AH=0 AL=0 ZC=0
status: code=0 detail=0 msg=OK
>
### command 9: reg 0x7E
reg 0x7E = 0x5449
status: code=0 detail=0 msg=OK
>
### command 10: reg 0x7F
reg 0x7F = 0x3055
status: code=0 detail=0 msg=OK
>
### command 11: reg 0x19
reg 0x19 = 0x00F9
status: code=0 detail=0 msg=OK
>
### command 12: reg 0x1A
reg 0x1A = 0x3401
status: code=0 detail=0 msg=OK
>
### command 13: reg 0x1B
reg 0x1B = 0x020D
status: code=0 detail=0 msg=OK
>
### command 14: sleep
status: code=13 detail=0 msg=Applied configuration is not trusted
>
### command 15: wake
status: code=13 detail=0 msg=Applied configuration is not trusted
>
### command 16: sleep
status: code=13 detail=0 msg=Applied configuration is not trusted
>
### command 17: initidrive 0
channel=0 initDriveCode=0
status: code=0 detail=0 msg=OK
>
### command 18: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 19: timing 0x01
wakeSettleUs=479 conversionUs=1133 sequentialFrameUs=1613 acquisitionTransfers=4
status: code=0 detail=0 msg=OK
>
### command 20: selftest
MANUFACTURER_ID=0xFFFF DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 21: sleep
status: code=13 detail=0 msg=Applied configuration is not trusted
>
```
