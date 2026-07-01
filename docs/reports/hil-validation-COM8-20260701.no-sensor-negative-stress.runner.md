# LDC1614 HIL Run

Overall status: `PASS`
Timestamp UTC: `2026-07-01T14:30:11+00:00`
Git commit: `6c88592`
Git status: `clean`
Library version: `2.0.0`
Firmware version: `2.0.0`
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
Startup read elapsed: `6.000` s
Command timeout: `6.0` s
Idle gap: `0.35` s
Repeat command set: `10`
Base command count: `20`
Notes: Release v2.0.0 no-sensor negative HIL stress: version plus BUSY/INVALID_PARAM command set repeated 10 times; conversion checks excluded

## Commands

| # | Command | Status | Elapsed s | Reason |
| ---: | --- | --- | ---: | --- |
| 1 | `version` | `PASS` | 0.015 | informational command responded without failure pattern |
| 2 | `init` | `PASS` | 0.032 | informational command responded without failure pattern |
| 3 | `wake` | `PASS` | 0.015 | status output indicates OK |
| 4 | `rcount 0 0x0123` | `PASS` | 0.016 | matched configured expected-failure token: BUSY |
| 5 | `sleep` | `PASS` | 0.031 | status output indicates OK |
| 6 | `rcount 4 0x0123` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 7 | `settle 4 0x0011` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 8 | `clkdiv 4 2 3` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 9 | `offset 4 0x0010` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 10 | `idrive 4 5` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 11 | `rcount 0 0x0004` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 12 | `clkdiv 0 0 1` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 13 | `clkdiv 0 1 0` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 14 | `idrive 0 32` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 15 | `errcfg 0x0002` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 16 | `highcurrent 1` | `PASS` | 0.016 | status output indicates OK |
| 17 | `autoscan 2` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 18 | `highcurrent 0` | `PASS` | 0.031 | status output indicates OK |
| 19 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 20 | `sleep` | `PASS` | 0.031 | status output indicates OK |
| 21 | `version` | `PASS` | 0.016 | informational command responded without failure pattern |
| 22 | `init` | `PASS` | 0.016 | informational command responded without failure pattern |
| 23 | `wake` | `PASS` | 0.031 | status output indicates OK |
| 24 | `rcount 0 0x0123` | `PASS` | 0.015 | matched configured expected-failure token: BUSY |
| 25 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 26 | `rcount 4 0x0123` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 27 | `settle 4 0x0011` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 28 | `clkdiv 4 2 3` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 29 | `offset 4 0x0010` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 30 | `idrive 4 5` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 31 | `rcount 0 0x0004` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 32 | `clkdiv 0 0 1` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 33 | `clkdiv 0 1 0` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 34 | `idrive 0 32` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 35 | `errcfg 0x0002` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 36 | `highcurrent 1` | `PASS` | 0.015 | status output indicates OK |
| 37 | `autoscan 2` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 38 | `highcurrent 0` | `PASS` | 0.016 | status output indicates OK |
| 39 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 40 | `sleep` | `PASS` | 0.015 | status output indicates OK |
| 41 | `version` | `PASS` | 0.031 | informational command responded without failure pattern |
| 42 | `init` | `PASS` | 0.016 | informational command responded without failure pattern |
| 43 | `wake` | `PASS` | 0.016 | status output indicates OK |
| 44 | `rcount 0 0x0123` | `PASS` | 0.031 | matched configured expected-failure token: BUSY |
| 45 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 46 | `rcount 4 0x0123` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 47 | `settle 4 0x0011` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 48 | `clkdiv 4 2 3` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 49 | `offset 4 0x0010` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 50 | `idrive 4 5` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 51 | `rcount 0 0x0004` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 52 | `clkdiv 0 0 1` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 53 | `clkdiv 0 1 0` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 54 | `idrive 0 32` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 55 | `errcfg 0x0002` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 56 | `highcurrent 1` | `PASS` | 0.015 | status output indicates OK |
| 57 | `autoscan 2` | `PASS` | 0.032 | matched configured expected-failure token: INVALID_PARAM |
| 58 | `highcurrent 0` | `PASS` | 0.015 | status output indicates OK |
| 59 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 60 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 61 | `version` | `PASS` | 0.015 | informational command responded without failure pattern |
| 62 | `init` | `PASS` | 0.032 | informational command responded without failure pattern |
| 63 | `wake` | `PASS` | 0.015 | status output indicates OK |
| 64 | `rcount 0 0x0123` | `PASS` | 0.016 | matched configured expected-failure token: BUSY |
| 65 | `sleep` | `PASS` | 0.031 | status output indicates OK |
| 66 | `rcount 4 0x0123` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 67 | `settle 4 0x0011` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 68 | `clkdiv 4 2 3` | `PASS` | 0.032 | matched configured expected-failure token: INVALID_PARAM |
| 69 | `offset 4 0x0010` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 70 | `idrive 4 5` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 71 | `rcount 0 0x0004` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 72 | `clkdiv 0 0 1` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 73 | `clkdiv 0 1 0` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 74 | `idrive 0 32` | `PASS` | 0.032 | matched configured expected-failure token: INVALID_PARAM |
| 75 | `errcfg 0x0002` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 76 | `highcurrent 1` | `PASS` | 0.016 | status output indicates OK |
| 77 | `autoscan 2` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 78 | `highcurrent 0` | `PASS` | 0.032 | status output indicates OK |
| 79 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 80 | `sleep` | `PASS` | 0.031 | status output indicates OK |
| 81 | `version` | `PASS` | 0.016 | informational command responded without failure pattern |
| 82 | `init` | `PASS` | 0.015 | informational command responded without failure pattern |
| 83 | `wake` | `PASS` | 0.032 | status output indicates OK |
| 84 | `rcount 0 0x0123` | `PASS` | 0.015 | matched configured expected-failure token: BUSY |
| 85 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 86 | `rcount 4 0x0123` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 87 | `settle 4 0x0011` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 88 | `clkdiv 4 2 3` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 89 | `offset 4 0x0010` | `PASS` | 0.032 | matched configured expected-failure token: INVALID_PARAM |
| 90 | `idrive 4 5` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 91 | `rcount 0 0x0004` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 92 | `clkdiv 0 0 1` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 93 | `clkdiv 0 1 0` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 94 | `idrive 0 32` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 95 | `errcfg 0x0002` | `PASS` | 0.032 | matched configured expected-failure token: INVALID_PARAM |
| 96 | `highcurrent 1` | `PASS` | 0.015 | status output indicates OK |
| 97 | `autoscan 2` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 98 | `highcurrent 0` | `PASS` | 0.016 | status output indicates OK |
| 99 | `cfg` | `PASS` | 0.046 | informational command responded without failure pattern |
| 100 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 101 | `version` | `PASS` | 0.031 | informational command responded without failure pattern |
| 102 | `init` | `PASS` | 0.016 | informational command responded without failure pattern |
| 103 | `wake` | `PASS` | 0.016 | status output indicates OK |
| 104 | `rcount 0 0x0123` | `PASS` | 0.031 | matched configured expected-failure token: BUSY |
| 105 | `sleep` | `PASS` | 0.015 | status output indicates OK |
| 106 | `rcount 4 0x0123` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 107 | `settle 4 0x0011` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 108 | `clkdiv 4 2 3` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 109 | `offset 4 0x0010` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 110 | `idrive 4 5` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 111 | `rcount 0 0x0004` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 112 | `clkdiv 0 0 1` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 113 | `clkdiv 0 1 0` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 114 | `idrive 0 32` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 115 | `errcfg 0x0002` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 116 | `highcurrent 1` | `PASS` | 0.031 | status output indicates OK |
| 117 | `autoscan 2` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 118 | `highcurrent 0` | `PASS` | 0.016 | status output indicates OK |
| 119 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 120 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 121 | `version` | `PASS` | 0.015 | informational command responded without failure pattern |
| 122 | `init` | `PASS` | 0.031 | informational command responded without failure pattern |
| 123 | `wake` | `PASS` | 0.016 | status output indicates OK |
| 124 | `rcount 0 0x0123` | `PASS` | 0.016 | matched configured expected-failure token: BUSY |
| 125 | `sleep` | `PASS` | 0.031 | status output indicates OK |
| 126 | `rcount 4 0x0123` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 127 | `settle 4 0x0011` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 128 | `clkdiv 4 2 3` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 129 | `offset 4 0x0010` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 130 | `idrive 4 5` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 131 | `rcount 0 0x0004` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 132 | `clkdiv 0 0 1` | `PASS` | 0.032 | matched configured expected-failure token: INVALID_PARAM |
| 133 | `clkdiv 0 1 0` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 134 | `idrive 0 32` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 135 | `errcfg 0x0002` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 136 | `highcurrent 1` | `PASS` | 0.016 | status output indicates OK |
| 137 | `autoscan 2` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 138 | `highcurrent 0` | `PASS` | 0.032 | status output indicates OK |
| 139 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 140 | `sleep` | `PASS` | 0.031 | status output indicates OK |
| 141 | `version` | `PASS` | 0.016 | informational command responded without failure pattern |
| 142 | `init` | `PASS` | 0.015 | informational command responded without failure pattern |
| 143 | `wake` | `PASS` | 0.032 | status output indicates OK |
| 144 | `rcount 0 0x0123` | `PASS` | 0.015 | matched configured expected-failure token: BUSY |
| 145 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 146 | `rcount 4 0x0123` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 147 | `settle 4 0x0011` | `PASS` | 0.032 | matched configured expected-failure token: INVALID_PARAM |
| 148 | `clkdiv 4 2 3` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 149 | `offset 4 0x0010` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 150 | `idrive 4 5` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 151 | `rcount 0 0x0004` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 152 | `clkdiv 0 0 1` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 153 | `clkdiv 0 1 0` | `PASS` | 0.032 | matched configured expected-failure token: INVALID_PARAM |
| 154 | `idrive 0 32` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 155 | `errcfg 0x0002` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 156 | `highcurrent 1` | `PASS` | 0.031 | status output indicates OK |
| 157 | `autoscan 2` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 158 | `highcurrent 0` | `PASS` | 0.015 | status output indicates OK |
| 159 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 160 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 161 | `version` | `PASS` | 0.031 | informational command responded without failure pattern |
| 162 | `init` | `PASS` | 0.016 | informational command responded without failure pattern |
| 163 | `wake` | `PASS` | 0.015 | status output indicates OK |
| 164 | `rcount 0 0x0123` | `PASS` | 0.016 | matched configured expected-failure token: BUSY |
| 165 | `sleep` | `PASS` | 0.031 | status output indicates OK |
| 166 | `rcount 4 0x0123` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 167 | `settle 4 0x0011` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 168 | `clkdiv 4 2 3` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 169 | `offset 4 0x0010` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 170 | `idrive 4 5` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 171 | `rcount 0 0x0004` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 172 | `clkdiv 0 0 1` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 173 | `clkdiv 0 1 0` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 174 | `idrive 0 32` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 175 | `errcfg 0x0002` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 176 | `highcurrent 1` | `PASS` | 0.016 | status output indicates OK |
| 177 | `autoscan 2` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 178 | `highcurrent 0` | `PASS` | 0.016 | status output indicates OK |
| 179 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 180 | `sleep` | `PASS` | 0.031 | status output indicates OK |
| 181 | `version` | `PASS` | 0.016 | informational command responded without failure pattern |
| 182 | `init` | `PASS` | 0.016 | informational command responded without failure pattern |
| 183 | `wake` | `PASS` | 0.031 | status output indicates OK |
| 184 | `rcount 0 0x0123` | `PASS` | 0.016 | matched configured expected-failure token: BUSY |
| 185 | `sleep` | `PASS` | 0.015 | status output indicates OK |
| 186 | `rcount 4 0x0123` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 187 | `settle 4 0x0011` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 188 | `clkdiv 4 2 3` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 189 | `offset 4 0x0010` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 190 | `idrive 4 5` | `PASS` | 0.032 | matched configured expected-failure token: INVALID_PARAM |
| 191 | `rcount 0 0x0004` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 192 | `clkdiv 0 0 1` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 193 | `clkdiv 0 1 0` | `PASS` | 0.031 | matched configured expected-failure token: INVALID_PARAM |
| 194 | `idrive 0 32` | `PASS` | 0.016 | matched configured expected-failure token: INVALID_PARAM |
| 195 | `errcfg 0x0002` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 196 | `highcurrent 1` | `PASS` | 0.032 | status output indicates OK |
| 197 | `autoscan 2` | `PASS` | 0.015 | matched configured expected-failure token: INVALID_PARAM |
| 198 | `highcurrent 0` | `PASS` | 0.016 | status output indicates OK |
| 199 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 200 | `sleep` | `PASS` | 0.015 | status output indicates OK |

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

### command 1: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 2: init
[36m[I][0m Initializing LDC1614...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device initialized in sleep mode. Use 'wake' to start conversions.
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [33m0[0m
  Total failures: [32m0[0m
  Success rate: [31m0.0%[0m
  Last OK: [33mnever[0m
  Last error: [32mnever[0m
> 
### command 3: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 4: rcount 0 0x0123
  Status: [31mBUSY[0m (code=8, detail=0)
  Message: [33mMust be in sleep mode to change config[0m
> 
### command 5: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 6: rcount 4 0x0123
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 7: settle 4 0x0011
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 8: clkdiv 4 2 3
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 9: offset 4 0x0010
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 10: idrive 4 5
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 11: rcount 0 0x0004
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mRCOUNT below minimum (0x0005)[0m
> 
### command 12: clkdiv 0 0 1
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FIN_DIVIDER (1-15)[0m
> 
### command 13: clkdiv 0 1 0
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FREF_DIVIDER (1-1023)[0m
> 
### command 14: idrive 0 32
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mIDRIVE exceeds maximum (31)[0m
> 
### command 15: errcfg 0x0002
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mERROR_CONFIG has reserved bits set[0m
> 
### command 16: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 17: autoscan 2
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mDisable HIGH_CURRENT_DRV before auto-scan[0m
> 
### command 18: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 19: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x020D
  Live CONFIG: 0x3401 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x04D6 SETTLE=0x000A CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=10
    Live RCOUNT: 0x04D6 (1238)
    Live SETTLECOUNT: 0x000A (10)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x5000 (IDRIVE=10, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 1 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 2 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 3 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  config_readback_failures=0
> 
### command 20: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 21: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 22: init
[36m[I][0m Initializing LDC1614...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device initialized in sleep mode. Use 'wake' to start conversions.
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [33m0[0m
  Total failures: [32m0[0m
  Success rate: [31m0.0%[0m
  Last OK: [33mnever[0m
  Last error: [32mnever[0m
> 
### command 23: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 24: rcount 0 0x0123
  Status: [31mBUSY[0m (code=8, detail=0)
  Message: [33mMust be in sleep mode to change config[0m
> 
### command 25: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 26: rcount 4 0x0123
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 27: settle 4 0x0011
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 28: clkdiv 4 2 3
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 29: offset 4 0x0010
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 30: idrive 4 5
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 31: rcount 0 0x0004
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mRCOUNT below minimum (0x0005)[0m
> 
### command 32: clkdiv 0 0 1
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FIN_DIVIDER (1-15)[0m
> 
### command 33: clkdiv 0 1 0
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FREF_DIVIDER (1-1023)[0m
> 
### command 34: idrive 0 32
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mIDRIVE exceeds maximum (31)[0m
> 
### command 35: errcfg 0x0002
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mERROR_CONFIG has reserved bits set[0m
> 
### command 36: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 37: autoscan 2
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mDisable HIGH_CURRENT_DRV before auto-scan[0m
> 
### command 38: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 39: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x020D
  Live CONFIG: 0x3401 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x04D6 SETTLE=0x000A CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=10
    Live RCOUNT: 0x04D6 (1238)
    Live SETTLECOUNT: 0x000A (10)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x5000 (IDRIVE=10, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 1 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 2 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 3 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  config_readback_failures=0
> 
### command 40: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 41: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 42: init
[36m[I][0m Initializing LDC1614...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device initialized in sleep mode. Use 'wake' to start conversions.
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [33m0[0m
  Total failures: [32m0[0m
  Success rate: [31m0.0%[0m
  Last OK: [33mnever[0m
  Last error: [32mnever[0m
> 
### command 43: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 44: rcount 0 0x0123
  Status: [31mBUSY[0m (code=8, detail=0)
  Message: [33mMust be in sleep mode to change config[0m
> 
### command 45: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 46: rcount 4 0x0123
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 47: settle 4 0x0011
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 48: clkdiv 4 2 3
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 49: offset 4 0x0010
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 50: idrive 4 5
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 51: rcount 0 0x0004
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mRCOUNT below minimum (0x0005)[0m
> 
### command 52: clkdiv 0 0 1
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FIN_DIVIDER (1-15)[0m
> 
### command 53: clkdiv 0 1 0
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FREF_DIVIDER (1-1023)[0m
> 
### command 54: idrive 0 32
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mIDRIVE exceeds maximum (31)[0m
> 
### command 55: errcfg 0x0002
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mERROR_CONFIG has reserved bits set[0m
> 
### command 56: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 57: autoscan 2
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mDisable HIGH_CURRENT_DRV before auto-scan[0m
> 
### command 58: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 59: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x020D
  Live CONFIG: 0x3401 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x04D6 SETTLE=0x000A CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=10
    Live RCOUNT: 0x04D6 (1238)
    Live SETTLECOUNT: 0x000A (10)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x5000 (IDRIVE=10, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 1 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 2 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 3 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  config_readback_failures=0
> 
### command 60: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 61: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 62: init
[36m[I][0m Initializing LDC1614...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device initialized in sleep mode. Use 'wake' to start conversions.
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [33m0[0m
  Total failures: [32m0[0m
  Success rate: [31m0.0%[0m
  Last OK: [33mnever[0m
  Last error: [32mnever[0m
> 
### command 63: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 64: rcount 0 0x0123
  Status: [31mBUSY[0m (code=8, detail=0)
  Message: [33mMust be in sleep mode to change config[0m
> 
### command 65: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 66: rcount 4 0x0123
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 67: settle 4 0x0011
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 68: clkdiv 4 2 3
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 69: offset 4 0x0010
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 70: idrive 4 5
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 71: rcount 0 0x0004
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mRCOUNT below minimum (0x0005)[0m
> 
### command 72: clkdiv 0 0 1
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FIN_DIVIDER (1-15)[0m
> 
### command 73: clkdiv 0 1 0
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FREF_DIVIDER (1-1023)[0m
> 
### command 74: idrive 0 32
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mIDRIVE exceeds maximum (31)[0m
> 
### command 75: errcfg 0x0002
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mERROR_CONFIG has reserved bits set[0m
> 
### command 76: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 77: autoscan 2
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mDisable HIGH_CURRENT_DRV before auto-scan[0m
> 
### command 78: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 79: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x020D
  Live CONFIG: 0x3401 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x04D6 SETTLE=0x000A CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=10
    Live RCOUNT: 0x04D6 (1238)
    Live SETTLECOUNT: 0x000A (10)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x5000 (IDRIVE=10, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 1 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 2 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 3 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  config_readback_failures=0
> 
### command 80: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 81: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 82: init
[36m[I][0m Initializing LDC1614...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device initialized in sleep mode. Use 'wake' to start conversions.
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [33m0[0m
  Total failures: [32m0[0m
  Success rate: [31m0.0%[0m
  Last OK: [33mnever[0m
  Last error: [32mnever[0m
> 
### command 83: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 84: rcount 0 0x0123
  Status: [31mBUSY[0m (code=8, detail=0)
  Message: [33mMust be in sleep mode to change config[0m
> 
### command 85: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 86: rcount 4 0x0123
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 87: settle 4 0x0011
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 88: clkdiv 4 2 3
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 89: offset 4 0x0010
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 90: idrive 4 5
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 91: rcount 0 0x0004
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mRCOUNT below minimum (0x0005)[0m
> 
### command 92: clkdiv 0 0 1
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FIN_DIVIDER (1-15)[0m
> 
### command 93: clkdiv 0 1 0
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FREF_DIVIDER (1-1023)[0m
> 
### command 94: idrive 0 32
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mIDRIVE exceeds maximum (31)[0m
> 
### command 95: errcfg 0x0002
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mERROR_CONFIG has reserved bits set[0m
> 
### command 96: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 97: autoscan 2
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mDisable HIGH_CURRENT_DRV before auto-scan[0m
> 
### command 98: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 99: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x020D
  Live CONFIG: 0x3401 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x04D6 SETTLE=0x000A CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=10
    Live RCOUNT: 0x04D6 (1238)
    Live SETTLECOUNT: 0x000A (10)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x5000 (IDRIVE=10, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 1 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 2 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 3 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  config_readback_failures=0
> 
### command 100: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 101: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 102: init
[36m[I][0m Initializing LDC1614...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device initialized in sleep mode. Use 'wake' to start conversions.
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [33m0[0m
  Total failures: [32m0[0m
  Success rate: [31m0.0%[0m
  Last OK: [33mnever[0m
  Last error: [32mnever[0m
> 
### command 103: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 104: rcount 0 0x0123
  Status: [31mBUSY[0m (code=8, detail=0)
  Message: [33mMust be in sleep mode to change config[0m
> 
### command 105: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 106: rcount 4 0x0123
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 107: settle 4 0x0011
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 108: clkdiv 4 2 3
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 109: offset 4 0x0010
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 110: idrive 4 5
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 111: rcount 0 0x0004
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mRCOUNT below minimum (0x0005)[0m
> 
### command 112: clkdiv 0 0 1
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FIN_DIVIDER (1-15)[0m
> 
### command 113: clkdiv 0 1 0
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FREF_DIVIDER (1-1023)[0m
> 
### command 114: idrive 0 32
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mIDRIVE exceeds maximum (31)[0m
> 
### command 115: errcfg 0x0002
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mERROR_CONFIG has reserved bits set[0m
> 
### command 116: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 117: autoscan 2
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mDisable HIGH_CURRENT_DRV before auto-scan[0m
> 
### command 118: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 119: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x020D
  Live CONFIG: 0x3401 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x04D6 SETTLE=0x000A CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=10
    Live RCOUNT: 0x04D6 (1238)
    Live SETTLECOUNT: 0x000A (10)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x5000 (IDRIVE=10, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 1 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 2 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 3 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  config_readback_failures=0
> 
### command 120: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 121: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 122: init
[36m[I][0m Initializing LDC1614...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device initialized in sleep mode. Use 'wake' to start conversions.
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [33m0[0m
  Total failures: [32m0[0m
  Success rate: [31m0.0%[0m
  Last OK: [33mnever[0m
  Last error: [32mnever[0m
> 
### command 123: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 124: rcount 0 0x0123
  Status: [31mBUSY[0m (code=8, detail=0)
  Message: [33mMust be in sleep mode to change config[0m
> 
### command 125: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 126: rcount 4 0x0123
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 127: settle 4 0x0011
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 128: clkdiv 4 2 3
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 129: offset 4 0x0010
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 130: idrive 4 5
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 131: rcount 0 0x0004
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mRCOUNT below minimum (0x0005)[0m
> 
### command 132: clkdiv 0 0 1
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FIN_DIVIDER (1-15)[0m
> 
### command 133: clkdiv 0 1 0
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FREF_DIVIDER (1-1023)[0m
> 
### command 134: idrive 0 32
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mIDRIVE exceeds maximum (31)[0m
> 
### command 135: errcfg 0x0002
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mERROR_CONFIG has reserved bits set[0m
> 
### command 136: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 137: autoscan 2
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mDisable HIGH_CURRENT_DRV before auto-scan[0m
> 
### command 138: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 139: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x020D
  Live CONFIG: 0x3401 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x04D6 SETTLE=0x000A CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=10
    Live RCOUNT: 0x04D6 (1238)
    Live SETTLECOUNT: 0x000A (10)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x5000 (IDRIVE=10, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 1 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 2 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 3 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  config_readback_failures=0
> 
### command 140: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 141: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 142: init
[36m[I][0m Initializing LDC1614...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device initialized in sleep mode. Use 'wake' to start conversions.
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [33m0[0m
  Total failures: [32m0[0m
  Success rate: [31m0.0%[0m
  Last OK: [33mnever[0m
  Last error: [32mnever[0m
> 
### command 143: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 144: rcount 0 0x0123
  Status: [31mBUSY[0m (code=8, detail=0)
  Message: [33mMust be in sleep mode to change config[0m
> 
### command 145: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 146: rcount 4 0x0123
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 147: settle 4 0x0011
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 148: clkdiv 4 2 3
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 149: offset 4 0x0010
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 150: idrive 4 5
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 151: rcount 0 0x0004
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mRCOUNT below minimum (0x0005)[0m
> 
### command 152: clkdiv 0 0 1
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FIN_DIVIDER (1-15)[0m
> 
### command 153: clkdiv 0 1 0
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FREF_DIVIDER (1-1023)[0m
> 
### command 154: idrive 0 32
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mIDRIVE exceeds maximum (31)[0m
> 
### command 155: errcfg 0x0002
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mERROR_CONFIG has reserved bits set[0m
> 
### command 156: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 157: autoscan 2
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mDisable HIGH_CURRENT_DRV before auto-scan[0m
> 
### command 158: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 159: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x020D
  Live CONFIG: 0x3401 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x04D6 SETTLE=0x000A CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=10
    Live RCOUNT: 0x04D6 (1238)
    Live SETTLECOUNT: 0x000A (10)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x5000 (IDRIVE=10, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 1 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 2 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 3 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  config_readback_failures=0
> 
### command 160: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 161: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 162: init
[36m[I][0m Initializing LDC1614...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device initialized in sleep mode. Use 'wake' to start conversions.
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [33m0[0m
  Total failures: [32m0[0m
  Success rate: [31m0.0%[0m
  Last OK: [33mnever[0m
  Last error: [32mnever[0m
> 
### command 163: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 164: rcount 0 0x0123
  Status: [31mBUSY[0m (code=8, detail=0)
  Message: [33mMust be in sleep mode to change config[0m
> 
### command 165: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 166: rcount 4 0x0123
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 167: settle 4 0x0011
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 168: clkdiv 4 2 3
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 169: offset 4 0x0010
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 170: idrive 4 5
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 171: rcount 0 0x0004
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mRCOUNT below minimum (0x0005)[0m
> 
### command 172: clkdiv 0 0 1
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FIN_DIVIDER (1-15)[0m
> 
### command 173: clkdiv 0 1 0
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FREF_DIVIDER (1-1023)[0m
> 
### command 174: idrive 0 32
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mIDRIVE exceeds maximum (31)[0m
> 
### command 175: errcfg 0x0002
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mERROR_CONFIG has reserved bits set[0m
> 
### command 176: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 177: autoscan 2
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mDisable HIGH_CURRENT_DRV before auto-scan[0m
> 
### command 178: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 179: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x020D
  Live CONFIG: 0x3401 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x04D6 SETTLE=0x000A CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=10
    Live RCOUNT: 0x04D6 (1238)
    Live SETTLECOUNT: 0x000A (10)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x5000 (IDRIVE=10, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 1 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 2 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 3 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  config_readback_failures=0
> 
### command 180: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 181: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 182: init
[36m[I][0m Initializing LDC1614...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device initialized in sleep mode. Use 'wake' to start conversions.
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [33m0[0m
  Total failures: [32m0[0m
  Success rate: [31m0.0%[0m
  Last OK: [33mnever[0m
  Last error: [32mnever[0m
> 
### command 183: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 184: rcount 0 0x0123
  Status: [31mBUSY[0m (code=8, detail=0)
  Message: [33mMust be in sleep mode to change config[0m
> 
### command 185: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 186: rcount 4 0x0123
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 187: settle 4 0x0011
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 188: clkdiv 4 2 3
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 189: offset 4 0x0010
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 190: idrive 4 5
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid channel[0m
> 
### command 191: rcount 0 0x0004
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mRCOUNT below minimum (0x0005)[0m
> 
### command 192: clkdiv 0 0 1
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FIN_DIVIDER (1-15)[0m
> 
### command 193: clkdiv 0 1 0
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mInvalid FREF_DIVIDER (1-1023)[0m
> 
### command 194: idrive 0 32
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mIDRIVE exceeds maximum (31)[0m
> 
### command 195: errcfg 0x0002
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mERROR_CONFIG has reserved bits set[0m
> 
### command 196: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 197: autoscan 2
  Status: [31mINVALID_PARAM[0m (code=5, detail=0)
  Message: [33mDisable HIGH_CURRENT_DRV before auto-scan[0m
> 
### command 198: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 199: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x020D
  Live CONFIG: 0x3401 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x04D6 SETTLE=0x000A CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=10
    Live RCOUNT: 0x04D6 (1238)
    Live SETTLECOUNT: 0x000A (10)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x5000 (IDRIVE=10, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 1 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 2 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  --- Channel 3 ---
    Cached: RCOUNT=0x0080 SETTLE=0x0000 CLOCK_DIV(FIN=1,FREF=1) OFFSET=0x0000 IDRIVE=0
    Live RCOUNT: 0x0080 (128)
    Live SETTLECOUNT: 0x0000 (0)
    Live CLOCK_DIV: 0x1001 (FIN=1, FREF=1)
    Live DRIVE_CURRENT: 0x0000 (IDRIVE=0, INIT_IDRIVE=0)
    Live OFFSET: 0x0000 (0)
  config_readback_failures=0
> 
### command 200: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
```
