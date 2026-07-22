# LDC1614 HIL Run

Overall status: `FAIL`
Timestamp UTC: `2026-07-22T17:18:20+00:00`
Host checkout Git commit: `ac710c1`
Host checkout Git status: `clean`
Library version: `3.0.0`
Firmware version: `3.0.0`
Firmware-reported Git commit: `ac710c1`
Firmware-reported Git status: `clean`
Expected firmware Git commit: `ac710c1`
Profile: `arduino`
Fixture: `no-sensor`
Port: `COM8`
Baud: `115200`
Expected address: `0x2A`
Expected channel count: `4`
Operator: `Codex`
Board: `ESP32-S2 COM8; LDC1614 0x2A; no LC sensor; shared OLED 0x3C`
Dry run: `False`
Serial port requested: `True`
Serial DTR/RTS: `on` / `off`
Hardware attached: `True`
Evidence type: `hardware_hil`
Startup delay: `2.0` s
Startup read elapsed: `10.016` s
Command timeout: `10.0` s
Idle gap: `0.02` s
Repeat command set: `25`
Base command count: `8`
Notes: Repeated post-NACK application-owned bus recovery without MCU reset

## Commands

| # | Command | Status | Elapsed s | Reason |
| ---: | --- | --- | ---: | --- |
| 1 | `version` | `PASS` | 0.015 | all command-specific evidence parsed |
| 2 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 3 | `scan` | `PASS` | 0.031 | all command-specific evidence parsed |
| 4 | `busrecover` | `PASS` | 0.016 | status output indicates OK |
| 5 | `init` | `FAIL` | 0.047 | matched failure pattern: code=[1-9][0-9]* |
| 6 | `probe` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |
| 7 | `reg 0x7E` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 8 | `reg 0x7F` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 9 | `version` | `PASS` | 0.016 | all command-specific evidence parsed |
| 10 | `cfg` | `PASS` | 0.015 | all command-specific evidence parsed |
| 11 | `scan` | `PASS` | 0.032 | all command-specific evidence parsed |
| 12 | `busrecover` | `PASS` | 0.015 | status output indicates OK |
| 13 | `init` | `FAIL` | 0.063 | matched failure pattern: code=[1-9][0-9]* |
| 14 | `probe` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |
| 15 | `reg 0x7E` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 16 | `reg 0x7F` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 17 | `version` | `PASS` | 0.016 | all command-specific evidence parsed |
| 18 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 19 | `scan` | `PASS` | 0.031 | all command-specific evidence parsed |
| 20 | `busrecover` | `PASS` | 0.015 | status output indicates OK |
| 21 | `init` | `FAIL` | 0.047 | matched failure pattern: code=[1-9][0-9]* |
| 22 | `probe` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 23 | `reg 0x7E` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 24 | `reg 0x7F` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 25 | `version` | `PASS` | 0.015 | all command-specific evidence parsed |
| 26 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 27 | `scan` | `PASS` | 0.016 | all command-specific evidence parsed |
| 28 | `busrecover` | `PASS` | 0.031 | status output indicates OK |
| 29 | `init` | `FAIL` | 0.062 | matched failure pattern: code=[1-9][0-9]* |
| 30 | `probe` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 31 | `reg 0x7E` | `PASS` | 0.016 | all command-specific evidence parsed |
| 32 | `reg 0x7F` | `PASS` | 0.031 | all command-specific evidence parsed |
| 33 | `version` | `PASS` | 0.016 | all command-specific evidence parsed |
| 34 | `cfg` | `PASS` | 0.015 | all command-specific evidence parsed |
| 35 | `scan` | `PASS` | 0.031 | all command-specific evidence parsed |
| 36 | `busrecover` | `PASS` | 0.016 | status output indicates OK |
| 37 | `init` | `FAIL` | 0.047 | matched failure pattern: code=[1-9][0-9]* |
| 38 | `probe` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 39 | `reg 0x7E` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |
| 40 | `reg 0x7F` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 41 | `version` | `PASS` | 0.016 | all command-specific evidence parsed |
| 42 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 43 | `scan` | `PASS` | 0.015 | all command-specific evidence parsed |
| 44 | `busrecover` | `PASS` | 0.032 | status output indicates OK |
| 45 | `init` | `FAIL` | 0.062 | matched failure pattern: code=[1-9][0-9]* |
| 46 | `probe` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 47 | `reg 0x7E` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |
| 48 | `reg 0x7F` | `FAIL` | 0.032 | matched failure pattern: code=[1-9][0-9]* |
| 49 | `version` | `PASS` | 0.015 | all command-specific evidence parsed |
| 50 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 51 | `scan` | `PASS` | 0.031 | all command-specific evidence parsed |
| 52 | `busrecover` | `PASS` | 0.016 | status output indicates OK |
| 53 | `init` | `FAIL` | 0.062 | matched failure pattern: code=[1-9][0-9]* |
| 54 | `probe` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 55 | `reg 0x7E` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |
| 56 | `reg 0x7F` | `FAIL` | 0.032 | matched failure pattern: code=[1-9][0-9]* |
| 57 | `version` | `PASS` | 0.015 | all command-specific evidence parsed |
| 58 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 59 | `scan` | `PASS` | 0.031 | all command-specific evidence parsed |
| 60 | `busrecover` | `PASS` | 0.016 | status output indicates OK |
| 61 | `init` | `PASS` | 0.047 | all command-specific evidence parsed |
| 62 | `probe` | `PASS` | 0.015 | all command-specific evidence parsed |
| 63 | `reg 0x7E` | `PASS` | 0.016 | all command-specific evidence parsed |
| 64 | `reg 0x7F` | `PASS` | 0.031 | all command-specific evidence parsed |
| 65 | `version` | `PASS` | 0.016 | all command-specific evidence parsed |
| 66 | `cfg` | `PASS` | 0.015 | all command-specific evidence parsed |
| 67 | `scan` | `PASS` | 0.016 | all command-specific evidence parsed |
| 68 | `busrecover` | `PASS` | 0.031 | status output indicates OK |
| 69 | `init` | `PASS` | 0.063 | all command-specific evidence parsed |
| 70 | `probe` | `PASS` | 0.015 | all command-specific evidence parsed |
| 71 | `reg 0x7E` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 72 | `reg 0x7F` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 73 | `version` | `PASS` | 0.016 | all command-specific evidence parsed |
| 74 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 75 | `scan` | `PASS` | 0.031 | all command-specific evidence parsed |
| 76 | `busrecover` | `PASS` | 0.015 | status output indicates OK |
| 77 | `init` | `FAIL` | 0.047 | matched failure pattern: code=[1-9][0-9]* |
| 78 | `probe` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 79 | `reg 0x7E` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 80 | `reg 0x7F` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |
| 81 | `version` | `PASS` | 0.031 | all command-specific evidence parsed |
| 82 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 83 | `scan` | `PASS` | 0.016 | all command-specific evidence parsed |
| 84 | `busrecover` | `PASS` | 0.031 | status output indicates OK |
| 85 | `init` | `FAIL` | 0.062 | matched failure pattern: code=[1-9][0-9]* |
| 86 | `probe` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 87 | `reg 0x7E` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 88 | `reg 0x7F` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 89 | `version` | `PASS` | 0.016 | all command-specific evidence parsed |
| 90 | `cfg` | `PASS` | 0.015 | all command-specific evidence parsed |
| 91 | `scan` | `PASS` | 0.031 | all command-specific evidence parsed |
| 92 | `busrecover` | `PASS` | 0.016 | status output indicates OK |
| 93 | `init` | `PASS` | 0.063 | all command-specific evidence parsed |
| 94 | `probe` | `PASS` | 0.015 | all command-specific evidence parsed |
| 95 | `reg 0x7E` | `PASS` | 0.016 | all command-specific evidence parsed |
| 96 | `reg 0x7F` | `PASS` | 0.031 | all command-specific evidence parsed |
| 97 | `version` | `PASS` | 0.016 | all command-specific evidence parsed |
| 98 | `cfg` | `PASS` | 0.015 | all command-specific evidence parsed |
| 99 | `scan` | `PASS` | 0.032 | all command-specific evidence parsed |
| 100 | `busrecover` | `PASS` | 0.015 | status output indicates OK |
| 101 | `init` | `PASS` | 0.047 | all command-specific evidence parsed |
| 102 | `probe` | `PASS` | 0.016 | all command-specific evidence parsed |
| 103 | `reg 0x7E` | `PASS` | 0.015 | all command-specific evidence parsed |
| 104 | `reg 0x7F` | `PASS` | 0.032 | all command-specific evidence parsed |
| 105 | `version` | `PASS` | 0.015 | all command-specific evidence parsed |
| 106 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 107 | `scan` | `PASS` | 0.031 | all command-specific evidence parsed |
| 108 | `busrecover` | `PASS` | 0.016 | status output indicates OK |
| 109 | `init` | `FAIL` | 0.062 | matched failure pattern: code=[1-9][0-9]* |
| 110 | `probe` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 111 | `reg 0x7E` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |
| 112 | `reg 0x7F` | `FAIL` | 0.032 | matched failure pattern: code=[1-9][0-9]* |
| 113 | `version` | `PASS` | 0.015 | all command-specific evidence parsed |
| 114 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 115 | `scan` | `PASS` | 0.031 | all command-specific evidence parsed |
| 116 | `busrecover` | `PASS` | 0.016 | status output indicates OK |
| 117 | `init` | `FAIL` | 0.047 | matched failure pattern: code=[1-9][0-9]* |
| 118 | `probe` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |
| 119 | `reg 0x7E` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 120 | `reg 0x7F` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 121 | `version` | `PASS` | 0.031 | all command-specific evidence parsed |
| 122 | `cfg` | `PASS` | 0.015 | all command-specific evidence parsed |
| 123 | `scan` | `PASS` | 0.016 | all command-specific evidence parsed |
| 124 | `busrecover` | `PASS` | 0.031 | status output indicates OK |
| 125 | `init` | `PASS` | 0.063 | all command-specific evidence parsed |
| 126 | `probe` | `PASS` | 0.015 | all command-specific evidence parsed |
| 127 | `reg 0x7E` | `PASS` | 0.016 | all command-specific evidence parsed |
| 128 | `reg 0x7F` | `PASS` | 0.031 | all command-specific evidence parsed |
| 129 | `version` | `PASS` | 0.016 | all command-specific evidence parsed |
| 130 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 131 | `scan` | `PASS` | 0.031 | all command-specific evidence parsed |
| 132 | `busrecover` | `PASS` | 0.015 | status output indicates OK |
| 133 | `init` | `PASS` | 0.047 | all command-specific evidence parsed |
| 134 | `probe` | `PASS` | 0.016 | all command-specific evidence parsed |
| 135 | `reg 0x7E` | `PASS` | 0.016 | all command-specific evidence parsed |
| 136 | `reg 0x7F` | `PASS` | 0.015 | all command-specific evidence parsed |
| 137 | `version` | `PASS` | 0.031 | all command-specific evidence parsed |
| 138 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 139 | `scan` | `PASS` | 0.016 | all command-specific evidence parsed |
| 140 | `busrecover` | `PASS` | 0.031 | status output indicates OK |
| 141 | `init` | `FAIL` | 0.062 | matched failure pattern: code=[1-9][0-9]* |
| 142 | `probe` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 143 | `reg 0x7E` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 144 | `reg 0x7F` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 145 | `version` | `PASS` | 0.016 | all command-specific evidence parsed |
| 146 | `cfg` | `PASS` | 0.015 | all command-specific evidence parsed |
| 147 | `scan` | `PASS` | 0.031 | all command-specific evidence parsed |
| 148 | `busrecover` | `PASS` | 0.016 | status output indicates OK |
| 149 | `init` | `FAIL` | 0.063 | matched failure pattern: code=[1-9][0-9]* |
| 150 | `probe` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |
| 151 | `reg 0x7E` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 152 | `reg 0x7F` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 153 | `version` | `PASS` | 0.016 | all command-specific evidence parsed |
| 154 | `cfg` | `PASS` | 0.015 | all command-specific evidence parsed |
| 155 | `scan` | `PASS` | 0.032 | all command-specific evidence parsed |
| 156 | `busrecover` | `PASS` | 0.015 | status output indicates OK |
| 157 | `init` | `PASS` | 0.047 | all command-specific evidence parsed |
| 158 | `probe` | `PASS` | 0.016 | all command-specific evidence parsed |
| 159 | `reg 0x7E` | `PASS` | 0.015 | all command-specific evidence parsed |
| 160 | `reg 0x7F` | `PASS` | 0.032 | all command-specific evidence parsed |
| 161 | `version` | `PASS` | 0.015 | all command-specific evidence parsed |
| 162 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 163 | `scan` | `PASS` | 0.031 | all command-specific evidence parsed |
| 164 | `busrecover` | `PASS` | 0.016 | status output indicates OK |
| 165 | `init` | `PASS` | 0.062 | all command-specific evidence parsed |
| 166 | `probe` | `PASS` | 0.016 | all command-specific evidence parsed |
| 167 | `reg 0x7E` | `PASS` | 0.015 | all command-specific evidence parsed |
| 168 | `reg 0x7F` | `PASS` | 0.032 | all command-specific evidence parsed |
| 169 | `version` | `PASS` | 0.015 | all command-specific evidence parsed |
| 170 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 171 | `scan` | `PASS` | 0.031 | all command-specific evidence parsed |
| 172 | `busrecover` | `PASS` | 0.016 | status output indicates OK |
| 173 | `init` | `FAIL` | 0.047 | matched failure pattern: code=[1-9][0-9]* |
| 174 | `probe` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |
| 175 | `reg 0x7E` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 176 | `reg 0x7F` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 177 | `version` | `PASS` | 0.031 | all command-specific evidence parsed |
| 178 | `cfg` | `PASS` | 0.015 | all command-specific evidence parsed |
| 179 | `scan` | `PASS` | 0.016 | all command-specific evidence parsed |
| 180 | `busrecover` | `PASS` | 0.031 | status output indicates OK |
| 181 | `init` | `FAIL` | 0.063 | matched failure pattern: code=[1-9][0-9]* |
| 182 | `probe` | `FAIL` | 0.015 | matched failure pattern: code=[1-9][0-9]* |
| 183 | `reg 0x7E` | `FAIL` | 0.016 | matched failure pattern: code=[1-9][0-9]* |
| 184 | `reg 0x7F` | `FAIL` | 0.031 | matched failure pattern: code=[1-9][0-9]* |
| 185 | `version` | `PASS` | 0.016 | all command-specific evidence parsed |
| 186 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 187 | `scan` | `PASS` | 0.015 | all command-specific evidence parsed |
| 188 | `busrecover` | `PASS` | 0.031 | status output indicates OK |
| 189 | `init` | `FAIL` | 0.063 | matched failure pattern: code=[1-9][0-9]* |
| 190 | `probe` | `PASS` | 0.016 | all command-specific evidence parsed |
| 191 | `reg 0x7E` | `PASS` | 0.015 | all command-specific evidence parsed |
| 192 | `reg 0x7F` | `PASS` | 0.031 | all command-specific evidence parsed |
| 193 | `version` | `PASS` | 0.016 | all command-specific evidence parsed |
| 194 | `cfg` | `PASS` | 0.016 | all command-specific evidence parsed |
| 195 | `scan` | `PASS` | 0.031 | all command-specific evidence parsed |
| 196 | `busrecover` | `PASS` | 0.016 | status output indicates OK |
| 197 | `init` | `PASS` | 0.046 | all command-specific evidence parsed |
| 198 | `probe` | `PASS` | 0.016 | all command-specific evidence parsed |
| 199 | `reg 0x7E` | `PASS` | 0.016 | all command-specific evidence parsed |
| 200 | `reg 0x7F` | `PASS` | 0.015 | all command-specific evidence parsed |

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

### command 1: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 2: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 3: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 4: busrecover
status: code=0 detail=0 msg=OK
>
### command 5: init
scheduled operation=2 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=2 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 6: probe
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 7: reg 0x7E
reg 0x7E = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 8: reg 0x7F
reg 0x7F = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 9: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 10: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 11: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 12: busrecover
status: code=0 detail=0 msg=OK
>
### command 13: init
scheduled operation=3 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=3 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 14: probe
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 15: reg 0x7E
reg 0x7E = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 16: reg 0x7F
reg 0x7F = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 17: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 18: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 19: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 20: busrecover
status: code=0 detail=0 msg=OK
>
### command 21: init
scheduled operation=4 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=4 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 22: probe
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 23: reg 0x7E
reg 0x7E = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 24: reg 0x7F
reg 0x7F = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 25: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 26: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 27: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 28: busrecover
status: code=0 detail=0 msg=OK
>
### command 29: init
scheduled operation=5 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=5 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 30: probe
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 31: reg 0x7E
reg 0x7E = 0x5449
status: code=0 detail=0 msg=OK
>
### command 32: reg 0x7F
reg 0x7F = 0xFFFF
status: code=0 detail=0 msg=OK
>
### command 33: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 34: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 35: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 36: busrecover
status: code=0 detail=0 msg=OK
>
### command 37: init
scheduled operation=6 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=6 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 38: probe
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 39: reg 0x7E
reg 0x7E = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 40: reg 0x7F
reg 0x7F = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 41: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 42: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 43: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 44: busrecover
status: code=0 detail=0 msg=OK
>
### command 45: init
scheduled operation=7 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=7 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 46: probe
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 47: reg 0x7E
reg 0x7E = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 48: reg 0x7F
reg 0x7F = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 49: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 50: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 51: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 52: busrecover
status: code=0 detail=0 msg=OK
>
### command 53: init
scheduled operation=8 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=8 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 54: probe
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 55: reg 0x7E
reg 0x7E = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 56: reg 0x7F
reg 0x7F = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 57: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 58: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 59: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 60: busrecover
status: code=0 detail=0 msg=OK
>
### command 61: init
scheduled operation=9 reset=0
status: code=5 detail=0 msg=Operation scheduled
> result operation=9 kind=1 outcome=SUCCESS effects=0x00 configRevision=1 phase=11 reg=0x1A channel=255 transfers=26/26
status: code=0 detail=0 msg=OK

### command 62: probe
MANUFACTURER_ID=0x5449 DEVICE_ID=0x3055
status: code=0 detail=0 msg=OK
>
### command 63: reg 0x7E
reg 0x7E = 0x5449
status: code=0 detail=0 msg=OK
>
### command 64: reg 0x7F
reg 0x7F = 0x3055
status: code=0 detail=0 msg=OK
>
### command 65: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 66: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 67: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 68: busrecover
status: code=0 detail=0 msg=OK
>
### command 69: init
scheduled operation=10 reset=0
status: code=5 detail=0 msg=Operation scheduled
> result operation=10 kind=1 outcome=SUCCESS effects=0x00 configRevision=1 phase=11 reg=0x1A channel=255 transfers=26/26
status: code=0 detail=0 msg=OK

### command 70: probe
MANUFACTURER_ID=0x5449 DEVICE_ID=0x3055
status: code=0 detail=0 msg=OK
>
### command 71: reg 0x7E
reg 0x7E = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 72: reg 0x7F
reg 0x7F = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 73: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 74: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 75: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 76: busrecover
status: code=0 detail=0 msg=OK
>
### command 77: init
scheduled operation=11 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=11 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 78: probe
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 79: reg 0x7E
reg 0x7E = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 80: reg 0x7F
reg 0x7F = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 81: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 82: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 83: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 84: busrecover
status: code=0 detail=0 msg=OK
>
### command 85: init
scheduled operation=12 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=12 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 86: probe
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 87: reg 0x7E
reg 0x7E = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 88: reg 0x7F
reg 0x7F = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 89: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 90: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 91: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 92: busrecover
status: code=0 detail=0 msg=OK
>
### command 93: init
scheduled operation=13 reset=0
status: code=5 detail=0 msg=Operation scheduled
> result operation=13 kind=1 outcome=SUCCESS effects=0x00 configRevision=1 phase=11 reg=0x1A channel=255 transfers=26/26
status: code=0 detail=0 msg=OK

### command 94: probe
MANUFACTURER_ID=0x5449 DEVICE_ID=0x3055
status: code=0 detail=0 msg=OK
>
### command 95: reg 0x7E
reg 0x7E = 0x5449
status: code=0 detail=0 msg=OK
>
### command 96: reg 0x7F
reg 0x7F = 0x3055
status: code=0 detail=0 msg=OK
>
### command 97: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 98: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 99: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 100: busrecover
status: code=0 detail=0 msg=OK
>
### command 101: init
scheduled operation=14 reset=0
status: code=5 detail=0 msg=Operation scheduled
> result operation=14 kind=1 outcome=SUCCESS effects=0x00 configRevision=1 phase=11 reg=0x1A channel=255 transfers=26/26
status: code=0 detail=0 msg=OK

### command 102: probe
MANUFACTURER_ID=0x5449 DEVICE_ID=0x3055
status: code=0 detail=0 msg=OK
>
### command 103: reg 0x7E
reg 0x7E = 0x5449
status: code=0 detail=0 msg=OK
>
### command 104: reg 0x7F
reg 0x7F = 0x3055
status: code=0 detail=0 msg=OK
>
### command 105: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 106: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 107: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 108: busrecover
status: code=0 detail=0 msg=OK
>
### command 109: init
scheduled operation=15 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=15 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 110: probe
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 111: reg 0x7E
reg 0x7E = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 112: reg 0x7F
reg 0x7F = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 113: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 114: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 115: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 116: busrecover
status: code=0 detail=0 msg=OK
>
### command 117: init
scheduled operation=16 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=16 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 118: probe
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 119: reg 0x7E
reg 0x7E = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 120: reg 0x7F
reg 0x7F = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 121: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 122: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 123: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 124: busrecover
status: code=0 detail=0 msg=OK
>
### command 125: init
scheduled operation=17 reset=0
status: code=5 detail=0 msg=Operation scheduled
> result operation=17 kind=1 outcome=SUCCESS effects=0x00 configRevision=1 phase=11 reg=0x1A channel=255 transfers=26/26
status: code=0 detail=0 msg=OK

### command 126: probe
MANUFACTURER_ID=0x5449 DEVICE_ID=0x3055
status: code=0 detail=0 msg=OK
>
### command 127: reg 0x7E
reg 0x7E = 0x5449
status: code=0 detail=0 msg=OK
>
### command 128: reg 0x7F
reg 0x7F = 0x3055
status: code=0 detail=0 msg=OK
>
### command 129: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 130: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 131: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 132: busrecover
status: code=0 detail=0 msg=OK
>
### command 133: init
scheduled operation=18 reset=0
status: code=5 detail=0 msg=Operation scheduled
> result operation=18 kind=1 outcome=SUCCESS effects=0x00 configRevision=1 phase=11 reg=0x1A channel=255 transfers=26/26
status: code=0 detail=0 msg=OK

### command 134: probe
MANUFACTURER_ID=0x5449 DEVICE_ID=0x3055
status: code=0 detail=0 msg=OK
>
### command 135: reg 0x7E
reg 0x7E = 0x5449
status: code=0 detail=0 msg=OK
>
### command 136: reg 0x7F
reg 0x7F = 0x3055
status: code=0 detail=0 msg=OK
>
### command 137: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 138: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 139: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 140: busrecover
status: code=0 detail=0 msg=OK
>
### command 141: init
scheduled operation=19 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=19 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 142: probe
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 143: reg 0x7E
reg 0x7E = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 144: reg 0x7F
reg 0x7F = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 145: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 146: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 147: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 148: busrecover
status: code=0 detail=0 msg=OK
>
### command 149: init
scheduled operation=20 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=20 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 150: probe
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 151: reg 0x7E
reg 0x7E = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 152: reg 0x7F
reg 0x7F = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 153: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 154: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 155: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 156: busrecover
status: code=0 detail=0 msg=OK
>
### command 157: init
scheduled operation=21 reset=0
status: code=5 detail=0 msg=Operation scheduled
> result operation=21 kind=1 outcome=SUCCESS effects=0x00 configRevision=1 phase=11 reg=0x1A channel=255 transfers=26/26
status: code=0 detail=0 msg=OK

### command 158: probe
MANUFACTURER_ID=0x5449 DEVICE_ID=0x3055
status: code=0 detail=0 msg=OK
>
### command 159: reg 0x7E
reg 0x7E = 0x5449
status: code=0 detail=0 msg=OK
>
### command 160: reg 0x7F
reg 0x7F = 0x3055
status: code=0 detail=0 msg=OK
>
### command 161: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 162: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 163: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 164: busrecover
status: code=0 detail=0 msg=OK
>
### command 165: init
scheduled operation=22 reset=0
status: code=5 detail=0 msg=Operation scheduled
> result operation=22 kind=1 outcome=SUCCESS effects=0x00 configRevision=1 phase=11 reg=0x1A channel=255 transfers=26/26
status: code=0 detail=0 msg=OK

### command 166: probe
MANUFACTURER_ID=0x5449 DEVICE_ID=0x3055
status: code=0 detail=0 msg=OK
>
### command 167: reg 0x7E
reg 0x7E = 0x5449
status: code=0 detail=0 msg=OK
>
### command 168: reg 0x7F
reg 0x7F = 0x3055
status: code=0 detail=0 msg=OK
>
### command 169: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 170: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 171: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 172: busrecover
status: code=0 detail=0 msg=OK
>
### command 173: init
scheduled operation=23 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=23 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 174: probe
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 175: reg 0x7E
reg 0x7E = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 176: reg 0x7F
reg 0x7F = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 177: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 178: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 179: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 180: busrecover
status: code=0 detail=0 msg=OK
>
### command 181: init
scheduled operation=24 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=24 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 182: probe
MANUFACTURER_ID=0x0000 DEVICE_ID=0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 183: reg 0x7E
reg 0x7E = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 184: reg 0x7F
reg 0x7F = 0x0000
status: code=14 detail=0 msg=I2C read length mismatch
>
### command 185: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 186: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 187: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 188: busrecover
status: code=0 detail=0 msg=OK
>
### command 189: init
scheduled operation=25 reset=0
status: code=5 detail=0 msg=Operation scheduled
> status: code=14 detail=0 msg=I2C read length mismatch
result operation=25 kind=1 outcome=FAILED effects=0x00 configRevision=1 phase=1 reg=0x7E channel=255 transfers=1/26
status: code=14 detail=0 msg=I2C read length mismatch
configFault phase=1 reg=0x7E channel=255 effects=0x00
status: code=14 detail=0 msg=I2C read length mismatch

### command 190: probe
MANUFACTURER_ID=0x5449 DEVICE_ID=0x3055
status: code=0 detail=0 msg=OK
>
### command 191: reg 0x7E
reg 0x7E = 0x5449
status: code=0 detail=0 msg=OK
>
### command 192: reg 0x7F
reg 0x7F = 0x3055
status: code=0 detail=0 msg=OK
>
### command 193: version
version: 3.0.0 firmware_git=ac710c1 firmware_status=clean build_timestamp=2026-07-22 19:17:03
>
### command 194: cfg
address=0x2A variant=2 variantChannels=4 selected=0x01 mode=1 refHz=43000000 tolerancePpm=200000 timeoutMs=50
>
### command 195: scan
I2C device at 0x2A
I2C device at 0x3C
scan complete found=2 probes=112
status: code=0 detail=0 msg=OK
>
### command 196: busrecover
status: code=0 detail=0 msg=OK
>
### command 197: init
scheduled operation=26 reset=0
status: code=5 detail=0 msg=Operation scheduled
> result operation=26 kind=1 outcome=SUCCESS effects=0x00 configRevision=1 phase=11 reg=0x1A channel=255 transfers=26/26
status: code=0 detail=0 msg=OK

### command 198: probe
MANUFACTURER_ID=0x5449 DEVICE_ID=0x3055
status: code=0 detail=0 msg=OK
>
### command 199: reg 0x7E
reg 0x7E = 0x5449
status: code=0 detail=0 msg=OK
>
### command 200: reg 0x7F
reg 0x7F = 0x3055
status: code=0 detail=0 msg=OK
>
```
