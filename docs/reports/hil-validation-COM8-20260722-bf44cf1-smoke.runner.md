# LDC1614 HIL Run

Overall status: `FAIL`
Timestamp UTC: `2026-07-22T17:00:43+00:00`
Host checkout Git commit: `bf44cf1`
Host checkout Git status: `clean`
Library version: `3.0.0`
Firmware version: `3.0.0`
Firmware-reported Git commit: `bf44cf1`
Firmware-reported Git status: `clean`
Expected firmware Git commit: `bf44cf1`
Profile: `arduino`
Fixture: `no-sensor`
Port: `COM8`
Baud: `115200`
Expected address: `0x2A`
Expected channel count: `4`
Operator: `Codex`
Board: `ESP32-S2 COM8; LDC1614 at 0x2A; no LC sensor; shared OLED at 0x3C`
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
Base command count: `39`
Notes: Clean post-v3 candidate smoke on pioarduino 53.03.13 at 400 kHz

## Commands

| # | Command | Status | Elapsed s | Reason |
| ---: | --- | --- | ---: | --- |
| 1 | `help` | `PASS` | 0.016 | all command-specific evidence parsed |
| 2 | `version` | `PASS` | 0.031 | all command-specific evidence parsed |
| 3 | `scan` | `PASS` | 0.016 | all command-specific evidence parsed |
| 4 | `probe` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 5 | `drv` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 6 | `cfg` | `PASS` | 0.015 | all command-specific evidence parsed |
| 7 | `progress` | `PASS` | 0.016 | all command-specific evidence parsed |
| 8 | `status` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 9 | `reg 0x7E` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 10 | `reg 0x7F` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 11 | `reg 0x19` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 12 | `reg 0x1A` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |
| 13 | `reg 0x1B` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 14 | `sleep` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 15 | `wake` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 16 | `sleep` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 17 | `initidrive 0` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |
| 18 | `cfg` | `PASS` | 0.031 | all command-specific evidence parsed |
| 19 | `timing 0x01` | `PASS` | 0.016 | all command-specific evidence parsed |
| 20 | `selftest` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 21 | `sleep` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 22 | `init` | `PASS` | 0.391 | all command-specific evidence parsed |
| 23 | `apply` | `PASS` | 0.375 | all command-specific evidence parsed |
| 24 | `resetreapply` | `FAIL` | 0.390 | matched failure pattern: code=[1-9][0-9]* |
| 25 | `wake` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 26 | `acquire 0x01` | `FAIL` | 0.016 | missing scheduled or terminal operation evidence |
| 27 | `sleep` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 28 | `cancel` | `PASS` | 0.031 | status output indicates OK |
| 29 | `invalidate` | `PASS` | 0.016 | status output indicates OK |
| 30 | `drv` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |
| 31 | `init` | `FAIL` | 0.391 | matched failure pattern: code=[1-9][0-9]* |
| 32 | `wreg 0x1B 0x0209` | `PASS` | 0.015 | status output indicates OK |
| 33 | `init` | `FAIL` | 0.391 | matched failure pattern: code=[1-9][0-9]* |
| 34 | `end` | `PASS` | 0.016 | status output indicates OK |
| 35 | `bind` | `PASS` | 0.031 | status output indicates OK |
| 36 | `init` | `FAIL` | 0.375 | matched failure pattern: code=[1-9][0-9]* |
| 37 | `freq 0 0x01000000` | `PASS` | 0.031 | all command-specific evidence parsed |
| 38 | `drv` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 39 | `sleep` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |

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
version: 3.0.0 firmware_git=bf44cf1 firmware_status=clean build_timestamp=2026-07-22 18:38:17
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
transport attempts=2 success=0 failures=2
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 6: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 7: progress
active=0 operation=0 kind=0 phase=0 transfers=0/0 requested=0x00 completed=0x00 deadlineMs=0
>
### command 8: status
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 9: reg 0x7E
reg 0x7E = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 10: reg 0x7F
reg 0x7F = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 11: reg 0x19
reg 0x19 = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 12: reg 0x1A
reg 0x1A = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 13: reg 0x1B
reg 0x1B = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
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
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 18: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 19: timing 0x01
wakeSettleUs=479 conversionUs=1133 sequentialFrameUs=1613 acquisitionTransfers=4
status: code=0 detail=0 msg=OK
>
### command 20: selftest
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 21: sleep
status: code=13 detail=0 msg=Applied configuration is not trusted
>
### command 22: init
scheduled operation=2 reset=0
status: code=5 detail=0 msg=Operation scheduled
> result operation=2 kind=1 outcome=SUCCESS effects=0x00 configRevision=1 phase=11 reg=0x1A channel=255 transfers=26/26
status: code=0 detail=0 msg=OK

### command 23: apply
scheduled operation=3 kind=apply
status: code=5 detail=0 msg=Operation scheduled
> result operation=3 kind=2 outcome=SUCCESS effects=0x00 configRevision=1 phase=11 reg=0x1A channel=255 transfers=24/24
status: code=0 detail=0 msg=OK

### command 24: resetreapply
scheduled operation=4 reset=1
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=4 kind=3 outcome=FAILED effects=0x02 configRevision=1 phase=1 reg=0x7E channel=255 transfers=2/27
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x02
status: code=14 detail=0 msg=I2C read length mismatch

### command 25: wake
status: code=13 detail=0 msg=Applied configuration is not trusted
>
### command 26: acquire 0x01
scheduled acquire operation=5 mask=0x01
status: code=13 detail=0 msg=Acquisition requires verified active configuration
>
### command 27: sleep
status: code=13 detail=0 msg=Applied configuration is not trusted
>
### command 28: cancel
status: code=0 detail=0 msg=OK
>
### command 29: invalidate
status: code=0 detail=0 msg=OK
>
### command 30: drv
bound=1 applied=UNKNOWN configRevision=1 active=0 resultAvailable=0
transport attempts=62 success=51 failures=11
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 31: init
scheduled operation=6 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=6 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 32: wreg 0x1B 0x0209
status: code=0 detail=0 msg=OK
>
### command 33: init
scheduled operation=7 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=7 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 34: end
status: code=0 detail=0 msg=OK
>
### command 35: bind
status: code=0 detail=0 msg=OK
>
### command 36: init
scheduled operation=8 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=8 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 37: freq 0 0x01000000
frequencyHz=2687500.000000
status: code=0 detail=0 msg=OK
>
### command 38: drv
bound=1 applied=UNKNOWN configRevision=1 active=0 resultAvailable=0
transport attempts=1 success=0 failures=1
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 39: sleep
status: code=13 detail=0 msg=Applied configuration is not trusted
>
```
