# LDC1614 HIL Run

Overall status: `PASS`
Timestamp UTC: `2026-07-01T14:29:03+00:00`
Git commit: `42feb3b`
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
Base command count: `101`
Notes: Release v2.0.0 no-sensor command-set HIL stress: 101 chip-only commands repeated 10 times; conversion checks excluded

## Commands

| # | Command | Status | Elapsed s | Reason |
| ---: | --- | --- | ---: | --- |
| 1 | `help` | `PASS` | 0.047 | informational command responded without failure pattern |
| 2 | `version` | `PASS` | 0.016 | informational command responded without failure pattern |
| 3 | `init` | `PASS` | 0.031 | informational command responded without failure pattern |
| 4 | `scan` | `PASS` | 0.140 | informational command responded without failure pattern |
| 5 | `probeaddr 0x2A` | `PASS` | 0.016 | status output indicates OK |
| 6 | `probe` | `PASS` | 0.031 | status output indicates OK |
| 7 | `id` | `PASS` | 0.016 | informational command responded without failure pattern |
| 8 | `drv` | `PASS` | 0.016 | informational command responded without failure pattern |
| 9 | `state` | `PASS` | 0.031 | informational command responded without failure pattern |
| 10 | `online` | `PASS` | 0.015 | informational command responded without failure pattern |
| 11 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 12 | `snapshot` | `PASS` | 0.016 | informational command responded without failure pattern |
| 13 | `channels` | `PASS` | 0.016 | informational command responded without failure pattern |
| 14 | `activech` | `PASS` | 0.031 | informational command responded without failure pattern |
| 15 | `status` | `PASS` | 0.015 | informational command responded without failure pattern |
| 16 | `status_raw` | `PASS` | 0.016 | informational command responded without failure pattern |
| 17 | `rawreg 0x7E` | `PASS` | 0.031 | informational command responded without failure pattern |
| 18 | `rawreg 0x7F` | `PASS` | 0.016 | informational command responded without failure pattern |
| 19 | `reg 0x7E` | `PASS` | 0.016 | informational command responded without failure pattern |
| 20 | `reg 0x7F` | `PASS` | 0.031 | informational command responded without failure pattern |
| 21 | `reg 0x19` | `PASS` | 0.015 | informational command responded without failure pattern |
| 22 | `reg 0x1A` | `PASS` | 0.016 | informational command responded without failure pattern |
| 23 | `reg 0x1B` | `PASS` | 0.016 | informational command responded without failure pattern |
| 24 | `sleep` | `PASS` | 0.031 | status output indicates OK |
| 25 | `wake` | `PASS` | 0.016 | status output indicates OK |
| 26 | `sleep` | `PASS` | 0.015 | status output indicates OK |
| 27 | `single 0` | `PASS` | 0.031 | status output indicates OK |
| 28 | `rcount 0 0x0123` | `PASS` | 0.016 | status output indicates OK |
| 29 | `settle 0 0x0011` | `PASS` | 0.016 | status output indicates OK |
| 30 | `clkdiv 0 2 3` | `PASS` | 0.031 | status output indicates OK |
| 31 | `offset 0 0x0010` | `PASS` | 0.016 | status output indicates OK |
| 32 | `idrive 0 5` | `PASS` | 0.015 | status output indicates OK |
| 33 | `rcount 1 0x0124` | `PASS` | 0.031 | status output indicates OK |
| 34 | `settle 1 0x0012` | `PASS` | 0.016 | status output indicates OK |
| 35 | `clkdiv 1 2 4` | `PASS` | 0.016 | status output indicates OK |
| 36 | `offset 1 0x0011` | `PASS` | 0.031 | status output indicates OK |
| 37 | `idrive 1 6` | `PASS` | 0.016 | status output indicates OK |
| 38 | `rcount 2 0x0125` | `PASS` | 0.015 | status output indicates OK |
| 39 | `settle 2 0x0013` | `PASS` | 0.031 | status output indicates OK |
| 40 | `clkdiv 2 2 5` | `PASS` | 0.016 | status output indicates OK |
| 41 | `offset 2 0x0012` | `PASS` | 0.016 | status output indicates OK |
| 42 | `idrive 2 7` | `PASS` | 0.031 | status output indicates OK |
| 43 | `rcount 3 0x0126` | `PASS` | 0.016 | status output indicates OK |
| 44 | `settle 3 0x0014` | `PASS` | 0.015 | status output indicates OK |
| 45 | `clkdiv 3 2 6` | `PASS` | 0.031 | status output indicates OK |
| 46 | `offset 3 0x0013` | `PASS` | 0.016 | status output indicates OK |
| 47 | `idrive 3 8` | `PASS` | 0.016 | status output indicates OK |
| 48 | `initidrive 0` | `PASS` | 0.031 | informational command responded without failure pattern |
| 49 | `initidrive 1` | `PASS` | 0.016 | informational command responded without failure pattern |
| 50 | `initidrive 2` | `PASS` | 0.015 | informational command responded without failure pattern |
| 51 | `initidrive 3` | `PASS` | 0.016 | informational command responded without failure pattern |
| 52 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 53 | `activech 1` | `PASS` | 0.015 | informational command responded without failure pattern |
| 54 | `activech 2` | `PASS` | 0.032 | informational command responded without failure pattern |
| 55 | `activech 3` | `PASS` | 0.015 | informational command responded without failure pattern |
| 56 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 57 | `single 1` | `PASS` | 0.031 | status output indicates OK |
| 58 | `single 2` | `PASS` | 0.016 | status output indicates OK |
| 59 | `single 3` | `PASS` | 0.015 | status output indicates OK |
| 60 | `single 0` | `PASS` | 0.032 | status output indicates OK |
| 61 | `autoscan 2` | `PASS` | 0.015 | status output indicates OK |
| 62 | `autoscan 3` | `PASS` | 0.016 | status output indicates OK |
| 63 | `autoscan 4` | `PASS` | 0.015 | status output indicates OK |
| 64 | `single 0` | `PASS` | 0.032 | status output indicates OK |
| 65 | `deglitch 1` | `PASS` | 0.015 | status output indicates OK |
| 66 | `deglitch 3` | `PASS` | 0.016 | status output indicates OK |
| 67 | `deglitch 10` | `PASS` | 0.031 | status output indicates OK |
| 68 | `deglitch 33` | `PASS` | 0.016 | status output indicates OK |
| 69 | `errcfg 0x0000` | `PASS` | 0.015 | informational command responded without failure pattern |
| 70 | `errcfg 0x00F9` | `PASS` | 0.032 | informational command responded without failure pattern |
| 71 | `errcfg` | `PASS` | 0.015 | informational command responded without failure pattern |
| 72 | `intb 0` | `PASS` | 0.016 | informational command responded without failure pattern |
| 73 | `intb 1` | `PASS` | 0.031 | informational command responded without failure pattern |
| 74 | `intb 0` | `PASS` | 0.016 | informational command responded without failure pattern |
| 75 | `refclk ext` | `PASS` | 0.015 | status output indicates OK |
| 76 | `refclk int` | `PASS` | 0.032 | status output indicates OK |
| 77 | `activate low` | `PASS` | 0.015 | status output indicates OK |
| 78 | `activate full` | `PASS` | 0.016 | status output indicates OK |
| 79 | `rpoverride 0` | `PASS` | 0.031 | status output indicates OK |
| 80 | `rpoverride 1` | `PASS` | 0.016 | status output indicates OK |
| 81 | `autoamp 1` | `PASS` | 0.015 | status output indicates OK |
| 82 | `autoamp 0` | `PASS` | 0.016 | status output indicates OK |
| 83 | `highcurrent 1` | `PASS` | 0.031 | status output indicates OK |
| 84 | `highcurrent 0` | `PASS` | 0.016 | status output indicates OK |
| 85 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 86 | `wreg 0x19 0x00F9` | `PASS` | 0.015 | status output indicates OK |
| 87 | `sync` | `PASS` | 0.016 | informational command responded without failure pattern |
| 88 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 89 | `rawwreg 0x19 0x00F9` | `PASS` | 0.016 | status output indicates OK |
| 90 | `cfg` | `PASS` | 0.046 | informational command responded without failure pattern |
| 91 | `resetreapply` | `PASS` | 0.016 | status output indicates OK |
| 92 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 93 | `recover` | `PASS` | 0.016 | status output indicates OK |
| 94 | `timing 0 43000000` | `PASS` | 0.031 | informational command responded without failure pattern |
| 95 | `timing 1 43000000` | `PASS` | 0.015 | informational command responded without failure pattern |
| 96 | `timing 2 43000000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 97 | `timing 3 43000000` | `PASS` | 0.031 | informational command responded without failure pattern |
| 98 | `reset` | `PASS` | 0.016 | status output indicates OK |
| 99 | `init` | `PASS` | 0.016 | informational command responded without failure pattern |
| 100 | `cfg` | `PASS` | 0.046 | informational command responded without failure pattern |
| 101 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 102 | `help` | `PASS` | 0.047 | informational command responded without failure pattern |
| 103 | `version` | `PASS` | 0.016 | informational command responded without failure pattern |
| 104 | `init` | `PASS` | 0.015 | informational command responded without failure pattern |
| 105 | `scan` | `PASS` | 0.156 | informational command responded without failure pattern |
| 106 | `probeaddr 0x2A` | `PASS` | 0.016 | status output indicates OK |
| 107 | `probe` | `PASS` | 0.016 | status output indicates OK |
| 108 | `id` | `PASS` | 0.031 | informational command responded without failure pattern |
| 109 | `drv` | `PASS` | 0.016 | informational command responded without failure pattern |
| 110 | `state` | `PASS` | 0.015 | informational command responded without failure pattern |
| 111 | `online` | `PASS` | 0.031 | informational command responded without failure pattern |
| 112 | `cfg` | `PASS` | 0.032 | informational command responded without failure pattern |
| 113 | `snapshot` | `PASS` | 0.031 | informational command responded without failure pattern |
| 114 | `channels` | `PASS` | 0.016 | informational command responded without failure pattern |
| 115 | `activech` | `PASS` | 0.015 | informational command responded without failure pattern |
| 116 | `status` | `PASS` | 0.016 | informational command responded without failure pattern |
| 117 | `status_raw` | `PASS` | 0.031 | informational command responded without failure pattern |
| 118 | `rawreg 0x7E` | `PASS` | 0.016 | informational command responded without failure pattern |
| 119 | `rawreg 0x7F` | `PASS` | 0.015 | informational command responded without failure pattern |
| 120 | `reg 0x7E` | `PASS` | 0.032 | informational command responded without failure pattern |
| 121 | `reg 0x7F` | `PASS` | 0.015 | informational command responded without failure pattern |
| 122 | `reg 0x19` | `PASS` | 0.016 | informational command responded without failure pattern |
| 123 | `reg 0x1A` | `PASS` | 0.031 | informational command responded without failure pattern |
| 124 | `reg 0x1B` | `PASS` | 0.016 | informational command responded without failure pattern |
| 125 | `sleep` | `PASS` | 0.015 | status output indicates OK |
| 126 | `wake` | `PASS` | 0.032 | status output indicates OK |
| 127 | `sleep` | `PASS` | 0.015 | status output indicates OK |
| 128 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 129 | `rcount 0 0x0123` | `PASS` | 0.015 | status output indicates OK |
| 130 | `settle 0 0x0011` | `PASS` | 0.032 | status output indicates OK |
| 131 | `clkdiv 0 2 3` | `PASS` | 0.015 | status output indicates OK |
| 132 | `offset 0 0x0010` | `PASS` | 0.016 | status output indicates OK |
| 133 | `idrive 0 5` | `PASS` | 0.031 | status output indicates OK |
| 134 | `rcount 1 0x0124` | `PASS` | 0.016 | status output indicates OK |
| 135 | `settle 1 0x0012` | `PASS` | 0.015 | status output indicates OK |
| 136 | `clkdiv 1 2 4` | `PASS` | 0.032 | status output indicates OK |
| 137 | `offset 1 0x0011` | `PASS` | 0.015 | status output indicates OK |
| 138 | `idrive 1 6` | `PASS` | 0.016 | status output indicates OK |
| 139 | `rcount 2 0x0125` | `PASS` | 0.031 | status output indicates OK |
| 140 | `settle 2 0x0013` | `PASS` | 0.016 | status output indicates OK |
| 141 | `clkdiv 2 2 5` | `PASS` | 0.015 | status output indicates OK |
| 142 | `offset 2 0x0012` | `PASS` | 0.032 | status output indicates OK |
| 143 | `idrive 2 7` | `PASS` | 0.015 | status output indicates OK |
| 144 | `rcount 3 0x0126` | `PASS` | 0.016 | status output indicates OK |
| 145 | `settle 3 0x0014` | `PASS` | 0.031 | status output indicates OK |
| 146 | `clkdiv 3 2 6` | `PASS` | 0.016 | status output indicates OK |
| 147 | `offset 3 0x0013` | `PASS` | 0.015 | status output indicates OK |
| 148 | `idrive 3 8` | `PASS` | 0.016 | status output indicates OK |
| 149 | `initidrive 0` | `PASS` | 0.031 | informational command responded without failure pattern |
| 150 | `initidrive 1` | `PASS` | 0.016 | informational command responded without failure pattern |
| 151 | `initidrive 2` | `PASS` | 0.016 | informational command responded without failure pattern |
| 152 | `initidrive 3` | `PASS` | 0.031 | informational command responded without failure pattern |
| 153 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 154 | `activech 1` | `PASS` | 0.031 | informational command responded without failure pattern |
| 155 | `activech 2` | `PASS` | 0.016 | informational command responded without failure pattern |
| 156 | `activech 3` | `PASS` | 0.016 | informational command responded without failure pattern |
| 157 | `single 0` | `PASS` | 0.031 | status output indicates OK |
| 158 | `single 1` | `PASS` | 0.015 | status output indicates OK |
| 159 | `single 2` | `PASS` | 0.016 | status output indicates OK |
| 160 | `single 3` | `PASS` | 0.031 | status output indicates OK |
| 161 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 162 | `autoscan 2` | `PASS` | 0.016 | status output indicates OK |
| 163 | `autoscan 3` | `PASS` | 0.015 | status output indicates OK |
| 164 | `autoscan 4` | `PASS` | 0.031 | status output indicates OK |
| 165 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 166 | `deglitch 1` | `PASS` | 0.016 | status output indicates OK |
| 167 | `deglitch 3` | `PASS` | 0.031 | status output indicates OK |
| 168 | `deglitch 10` | `PASS` | 0.016 | status output indicates OK |
| 169 | `deglitch 33` | `PASS` | 0.015 | status output indicates OK |
| 170 | `errcfg 0x0000` | `PASS` | 0.031 | informational command responded without failure pattern |
| 171 | `errcfg 0x00F9` | `PASS` | 0.016 | informational command responded without failure pattern |
| 172 | `errcfg` | `PASS` | 0.016 | informational command responded without failure pattern |
| 173 | `intb 0` | `PASS` | 0.031 | informational command responded without failure pattern |
| 174 | `intb 1` | `PASS` | 0.016 | informational command responded without failure pattern |
| 175 | `intb 0` | `PASS` | 0.015 | informational command responded without failure pattern |
| 176 | `refclk ext` | `PASS` | 0.031 | status output indicates OK |
| 177 | `refclk int` | `PASS` | 0.016 | status output indicates OK |
| 178 | `activate low` | `PASS` | 0.016 | status output indicates OK |
| 179 | `activate full` | `PASS` | 0.015 | status output indicates OK |
| 180 | `rpoverride 0` | `PASS` | 0.032 | status output indicates OK |
| 181 | `rpoverride 1` | `PASS` | 0.015 | status output indicates OK |
| 182 | `autoamp 1` | `PASS` | 0.016 | status output indicates OK |
| 183 | `autoamp 0` | `PASS` | 0.031 | status output indicates OK |
| 184 | `highcurrent 1` | `PASS` | 0.016 | status output indicates OK |
| 185 | `highcurrent 0` | `PASS` | 0.015 | status output indicates OK |
| 186 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 187 | `wreg 0x19 0x00F9` | `PASS` | 0.016 | status output indicates OK |
| 188 | `sync` | `PASS` | 0.031 | informational command responded without failure pattern |
| 189 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 190 | `rawwreg 0x19 0x00F9` | `PASS` | 0.032 | status output indicates OK |
| 191 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 192 | `resetreapply` | `PASS` | 0.031 | status output indicates OK |
| 193 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 194 | `recover` | `PASS` | 0.032 | status output indicates OK |
| 195 | `timing 0 43000000` | `PASS` | 0.015 | informational command responded without failure pattern |
| 196 | `timing 1 43000000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 197 | `timing 2 43000000` | `PASS` | 0.031 | informational command responded without failure pattern |
| 198 | `timing 3 43000000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 199 | `reset` | `PASS` | 0.015 | status output indicates OK |
| 200 | `init` | `PASS` | 0.016 | informational command responded without failure pattern |
| 201 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 202 | `sleep` | `PASS` | 0.015 | status output indicates OK |
| 203 | `help` | `PASS` | 0.047 | informational command responded without failure pattern |
| 204 | `version` | `PASS` | 0.016 | informational command responded without failure pattern |
| 205 | `init` | `PASS` | 0.031 | informational command responded without failure pattern |
| 206 | `scan` | `PASS` | 0.141 | informational command responded without failure pattern |
| 207 | `probeaddr 0x2A` | `PASS` | 0.015 | status output indicates OK |
| 208 | `probe` | `PASS` | 0.032 | status output indicates OK |
| 209 | `id` | `PASS` | 0.015 | informational command responded without failure pattern |
| 210 | `drv` | `PASS` | 0.016 | informational command responded without failure pattern |
| 211 | `state` | `PASS` | 0.031 | informational command responded without failure pattern |
| 212 | `online` | `PASS` | 0.016 | informational command responded without failure pattern |
| 213 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 214 | `snapshot` | `PASS` | 0.015 | informational command responded without failure pattern |
| 215 | `channels` | `PASS` | 0.016 | informational command responded without failure pattern |
| 216 | `activech` | `PASS` | 0.016 | informational command responded without failure pattern |
| 217 | `status` | `PASS` | 0.031 | informational command responded without failure pattern |
| 218 | `status_raw` | `PASS` | 0.015 | informational command responded without failure pattern |
| 219 | `rawreg 0x7E` | `PASS` | 0.016 | informational command responded without failure pattern |
| 220 | `rawreg 0x7F` | `PASS` | 0.031 | informational command responded without failure pattern |
| 221 | `reg 0x7E` | `PASS` | 0.016 | informational command responded without failure pattern |
| 222 | `reg 0x7F` | `PASS` | 0.016 | informational command responded without failure pattern |
| 223 | `reg 0x19` | `PASS` | 0.031 | informational command responded without failure pattern |
| 224 | `reg 0x1A` | `PASS` | 0.015 | informational command responded without failure pattern |
| 225 | `reg 0x1B` | `PASS` | 0.016 | informational command responded without failure pattern |
| 226 | `sleep` | `PASS` | 0.031 | status output indicates OK |
| 227 | `wake` | `PASS` | 0.016 | status output indicates OK |
| 228 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 229 | `single 0` | `PASS` | 0.031 | status output indicates OK |
| 230 | `rcount 0 0x0123` | `PASS` | 0.015 | status output indicates OK |
| 231 | `settle 0 0x0011` | `PASS` | 0.016 | status output indicates OK |
| 232 | `clkdiv 0 2 3` | `PASS` | 0.031 | status output indicates OK |
| 233 | `offset 0 0x0010` | `PASS` | 0.016 | status output indicates OK |
| 234 | `idrive 0 5` | `PASS` | 0.016 | status output indicates OK |
| 235 | `rcount 1 0x0124` | `PASS` | 0.015 | status output indicates OK |
| 236 | `settle 1 0x0012` | `PASS` | 0.031 | status output indicates OK |
| 237 | `clkdiv 1 2 4` | `PASS` | 0.016 | status output indicates OK |
| 238 | `offset 1 0x0011` | `PASS` | 0.016 | status output indicates OK |
| 239 | `idrive 1 6` | `PASS` | 0.031 | status output indicates OK |
| 240 | `rcount 2 0x0125` | `PASS` | 0.016 | status output indicates OK |
| 241 | `settle 2 0x0013` | `PASS` | 0.015 | status output indicates OK |
| 242 | `clkdiv 2 2 5` | `PASS` | 0.031 | status output indicates OK |
| 243 | `offset 2 0x0012` | `PASS` | 0.016 | status output indicates OK |
| 244 | `idrive 2 7` | `PASS` | 0.016 | status output indicates OK |
| 245 | `rcount 3 0x0126` | `PASS` | 0.031 | status output indicates OK |
| 246 | `settle 3 0x0014` | `PASS` | 0.016 | status output indicates OK |
| 247 | `clkdiv 3 2 6` | `PASS` | 0.015 | status output indicates OK |
| 248 | `offset 3 0x0013` | `PASS` | 0.031 | status output indicates OK |
| 249 | `idrive 3 8` | `PASS` | 0.016 | status output indicates OK |
| 250 | `initidrive 0` | `PASS` | 0.016 | informational command responded without failure pattern |
| 251 | `initidrive 1` | `PASS` | 0.015 | informational command responded without failure pattern |
| 252 | `initidrive 2` | `PASS` | 0.032 | informational command responded without failure pattern |
| 253 | `initidrive 3` | `PASS` | 0.015 | informational command responded without failure pattern |
| 254 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 255 | `activech 1` | `PASS` | 0.016 | informational command responded without failure pattern |
| 256 | `activech 2` | `PASS` | 0.015 | informational command responded without failure pattern |
| 257 | `activech 3` | `PASS` | 0.032 | informational command responded without failure pattern |
| 258 | `single 0` | `PASS` | 0.015 | status output indicates OK |
| 259 | `single 1` | `PASS` | 0.016 | status output indicates OK |
| 260 | `single 2` | `PASS` | 0.015 | status output indicates OK |
| 261 | `single 3` | `PASS` | 0.032 | status output indicates OK |
| 262 | `single 0` | `PASS` | 0.015 | status output indicates OK |
| 263 | `autoscan 2` | `PASS` | 0.016 | status output indicates OK |
| 264 | `autoscan 3` | `PASS` | 0.031 | status output indicates OK |
| 265 | `autoscan 4` | `PASS` | 0.016 | status output indicates OK |
| 266 | `single 0` | `PASS` | 0.015 | status output indicates OK |
| 267 | `deglitch 1` | `PASS` | 0.032 | status output indicates OK |
| 268 | `deglitch 3` | `PASS` | 0.015 | status output indicates OK |
| 269 | `deglitch 10` | `PASS` | 0.016 | status output indicates OK |
| 270 | `deglitch 33` | `PASS` | 0.031 | status output indicates OK |
| 271 | `errcfg 0x0000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 272 | `errcfg 0x00F9` | `PASS` | 0.015 | informational command responded without failure pattern |
| 273 | `errcfg` | `PASS` | 0.032 | informational command responded without failure pattern |
| 274 | `intb 0` | `PASS` | 0.015 | informational command responded without failure pattern |
| 275 | `intb 1` | `PASS` | 0.016 | informational command responded without failure pattern |
| 276 | `intb 0` | `PASS` | 0.031 | informational command responded without failure pattern |
| 277 | `refclk ext` | `PASS` | 0.016 | status output indicates OK |
| 278 | `refclk int` | `PASS` | 0.015 | status output indicates OK |
| 279 | `activate low` | `PASS` | 0.032 | status output indicates OK |
| 280 | `activate full` | `PASS` | 0.015 | status output indicates OK |
| 281 | `rpoverride 0` | `PASS` | 0.016 | status output indicates OK |
| 282 | `rpoverride 1` | `PASS` | 0.031 | status output indicates OK |
| 283 | `autoamp 1` | `PASS` | 0.016 | status output indicates OK |
| 284 | `autoamp 0` | `PASS` | 0.015 | status output indicates OK |
| 285 | `highcurrent 1` | `PASS` | 0.016 | status output indicates OK |
| 286 | `highcurrent 0` | `PASS` | 0.031 | status output indicates OK |
| 287 | `cfg` | `PASS` | 0.032 | informational command responded without failure pattern |
| 288 | `wreg 0x19 0x00F9` | `PASS` | 0.031 | status output indicates OK |
| 289 | `sync` | `PASS` | 0.015 | informational command responded without failure pattern |
| 290 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 291 | `rawwreg 0x19 0x00F9` | `PASS` | 0.016 | status output indicates OK |
| 292 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 293 | `resetreapply` | `PASS` | 0.015 | status output indicates OK |
| 294 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 295 | `recover` | `PASS` | 0.016 | status output indicates OK |
| 296 | `timing 0 43000000` | `PASS` | 0.031 | informational command responded without failure pattern |
| 297 | `timing 1 43000000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 298 | `timing 2 43000000` | `PASS` | 0.015 | informational command responded without failure pattern |
| 299 | `timing 3 43000000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 300 | `reset` | `PASS` | 0.031 | status output indicates OK |
| 301 | `init` | `PASS` | 0.016 | informational command responded without failure pattern |
| 302 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 303 | `sleep` | `PASS` | 0.015 | status output indicates OK |
| 304 | `help` | `PASS` | 0.047 | informational command responded without failure pattern |
| 305 | `version` | `PASS` | 0.016 | informational command responded without failure pattern |
| 306 | `init` | `PASS` | 0.016 | informational command responded without failure pattern |
| 307 | `scan` | `PASS` | 0.156 | informational command responded without failure pattern |
| 308 | `probeaddr 0x2A` | `PASS` | 0.015 | status output indicates OK |
| 309 | `probe` | `PASS` | 0.016 | status output indicates OK |
| 310 | `id` | `PASS` | 0.031 | informational command responded without failure pattern |
| 311 | `drv` | `PASS` | 0.016 | informational command responded without failure pattern |
| 312 | `state` | `PASS` | 0.016 | informational command responded without failure pattern |
| 313 | `online` | `PASS` | 0.015 | informational command responded without failure pattern |
| 314 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 315 | `snapshot` | `PASS` | 0.016 | informational command responded without failure pattern |
| 316 | `channels` | `PASS` | 0.016 | informational command responded without failure pattern |
| 317 | `activech` | `PASS` | 0.016 | informational command responded without failure pattern |
| 318 | `status` | `PASS` | 0.015 | informational command responded without failure pattern |
| 319 | `status_raw` | `PASS` | 0.031 | informational command responded without failure pattern |
| 320 | `rawreg 0x7E` | `PASS` | 0.016 | informational command responded without failure pattern |
| 321 | `rawreg 0x7F` | `PASS` | 0.016 | informational command responded without failure pattern |
| 322 | `reg 0x7E` | `PASS` | 0.031 | informational command responded without failure pattern |
| 323 | `reg 0x7F` | `PASS` | 0.016 | informational command responded without failure pattern |
| 324 | `reg 0x19` | `PASS` | 0.015 | informational command responded without failure pattern |
| 325 | `reg 0x1A` | `PASS` | 0.031 | informational command responded without failure pattern |
| 326 | `reg 0x1B` | `PASS` | 0.016 | informational command responded without failure pattern |
| 327 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 328 | `wake` | `PASS` | 0.031 | status output indicates OK |
| 329 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 330 | `single 0` | `PASS` | 0.015 | status output indicates OK |
| 331 | `rcount 0 0x0123` | `PASS` | 0.031 | status output indicates OK |
| 332 | `settle 0 0x0011` | `PASS` | 0.016 | status output indicates OK |
| 333 | `clkdiv 0 2 3` | `PASS` | 0.016 | status output indicates OK |
| 334 | `offset 0 0x0010` | `PASS` | 0.015 | status output indicates OK |
| 335 | `idrive 0 5` | `PASS` | 0.032 | status output indicates OK |
| 336 | `rcount 1 0x0124` | `PASS` | 0.015 | status output indicates OK |
| 337 | `settle 1 0x0012` | `PASS` | 0.016 | status output indicates OK |
| 338 | `clkdiv 1 2 4` | `PASS` | 0.031 | status output indicates OK |
| 339 | `offset 1 0x0011` | `PASS` | 0.016 | status output indicates OK |
| 340 | `idrive 1 6` | `PASS` | 0.015 | status output indicates OK |
| 341 | `rcount 2 0x0125` | `PASS` | 0.032 | status output indicates OK |
| 342 | `settle 2 0x0013` | `PASS` | 0.015 | status output indicates OK |
| 343 | `clkdiv 2 2 5` | `PASS` | 0.016 | status output indicates OK |
| 344 | `offset 2 0x0012` | `PASS` | 0.031 | status output indicates OK |
| 345 | `idrive 2 7` | `PASS` | 0.016 | status output indicates OK |
| 346 | `rcount 3 0x0126` | `PASS` | 0.015 | status output indicates OK |
| 347 | `settle 3 0x0014` | `PASS` | 0.032 | status output indicates OK |
| 348 | `clkdiv 3 2 6` | `PASS` | 0.015 | status output indicates OK |
| 349 | `offset 3 0x0013` | `PASS` | 0.016 | status output indicates OK |
| 350 | `idrive 3 8` | `PASS` | 0.031 | status output indicates OK |
| 351 | `initidrive 0` | `PASS` | 0.016 | informational command responded without failure pattern |
| 352 | `initidrive 1` | `PASS` | 0.015 | informational command responded without failure pattern |
| 353 | `initidrive 2` | `PASS` | 0.016 | informational command responded without failure pattern |
| 354 | `initidrive 3` | `PASS` | 0.031 | informational command responded without failure pattern |
| 355 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 356 | `activech 1` | `PASS` | 0.032 | informational command responded without failure pattern |
| 357 | `activech 2` | `PASS` | 0.015 | informational command responded without failure pattern |
| 358 | `activech 3` | `PASS` | 0.016 | informational command responded without failure pattern |
| 359 | `single 0` | `PASS` | 0.031 | status output indicates OK |
| 360 | `single 1` | `PASS` | 0.016 | status output indicates OK |
| 361 | `single 2` | `PASS` | 0.015 | status output indicates OK |
| 362 | `single 3` | `PASS` | 0.032 | status output indicates OK |
| 363 | `single 0` | `PASS` | 0.015 | status output indicates OK |
| 364 | `autoscan 2` | `PASS` | 0.016 | status output indicates OK |
| 365 | `autoscan 3` | `PASS` | 0.016 | status output indicates OK |
| 366 | `autoscan 4` | `PASS` | 0.031 | status output indicates OK |
| 367 | `single 0` | `PASS` | 0.015 | status output indicates OK |
| 368 | `deglitch 1` | `PASS` | 0.016 | status output indicates OK |
| 369 | `deglitch 3` | `PASS` | 0.031 | status output indicates OK |
| 370 | `deglitch 10` | `PASS` | 0.016 | status output indicates OK |
| 371 | `deglitch 33` | `PASS` | 0.016 | status output indicates OK |
| 372 | `errcfg 0x0000` | `PASS` | 0.031 | informational command responded without failure pattern |
| 373 | `errcfg 0x00F9` | `PASS` | 0.015 | informational command responded without failure pattern |
| 374 | `errcfg` | `PASS` | 0.016 | informational command responded without failure pattern |
| 375 | `intb 0` | `PASS` | 0.031 | informational command responded without failure pattern |
| 376 | `intb 1` | `PASS` | 0.016 | informational command responded without failure pattern |
| 377 | `intb 0` | `PASS` | 0.016 | informational command responded without failure pattern |
| 378 | `refclk ext` | `PASS` | 0.031 | status output indicates OK |
| 379 | `refclk int` | `PASS` | 0.015 | status output indicates OK |
| 380 | `activate low` | `PASS` | 0.016 | status output indicates OK |
| 381 | `activate full` | `PASS` | 0.031 | status output indicates OK |
| 382 | `rpoverride 0` | `PASS` | 0.016 | status output indicates OK |
| 383 | `rpoverride 1` | `PASS` | 0.016 | status output indicates OK |
| 384 | `autoamp 1` | `PASS` | 0.015 | status output indicates OK |
| 385 | `autoamp 0` | `PASS` | 0.031 | status output indicates OK |
| 386 | `highcurrent 1` | `PASS` | 0.016 | status output indicates OK |
| 387 | `highcurrent 0` | `PASS` | 0.016 | status output indicates OK |
| 388 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 389 | `wreg 0x19 0x00F9` | `PASS` | 0.015 | status output indicates OK |
| 390 | `sync` | `PASS` | 0.031 | informational command responded without failure pattern |
| 391 | `cfg` | `PASS` | 0.032 | informational command responded without failure pattern |
| 392 | `rawwreg 0x19 0x00F9` | `PASS` | 0.031 | status output indicates OK |
| 393 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 394 | `resetreapply` | `PASS` | 0.031 | status output indicates OK |
| 395 | `cfg` | `PASS` | 0.032 | informational command responded without failure pattern |
| 396 | `recover` | `PASS` | 0.015 | status output indicates OK |
| 397 | `timing 0 43000000` | `PASS` | 0.032 | informational command responded without failure pattern |
| 398 | `timing 1 43000000` | `PASS` | 0.015 | informational command responded without failure pattern |
| 399 | `timing 2 43000000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 400 | `timing 3 43000000` | `PASS` | 0.031 | informational command responded without failure pattern |
| 401 | `reset` | `PASS` | 0.016 | status output indicates OK |
| 402 | `init` | `PASS` | 0.015 | informational command responded without failure pattern |
| 403 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 404 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 405 | `help` | `PASS` | 0.047 | informational command responded without failure pattern |
| 406 | `version` | `PASS` | 0.015 | informational command responded without failure pattern |
| 407 | `init` | `PASS` | 0.032 | informational command responded without failure pattern |
| 408 | `scan` | `PASS` | 0.140 | informational command responded without failure pattern |
| 409 | `probeaddr 0x2A` | `PASS` | 0.016 | status output indicates OK |
| 410 | `probe` | `PASS` | 0.031 | status output indicates OK |
| 411 | `id` | `PASS` | 0.016 | informational command responded without failure pattern |
| 412 | `drv` | `PASS` | 0.015 | informational command responded without failure pattern |
| 413 | `state` | `PASS` | 0.032 | informational command responded without failure pattern |
| 414 | `online` | `PASS` | 0.015 | informational command responded without failure pattern |
| 415 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 416 | `snapshot` | `PASS` | 0.032 | informational command responded without failure pattern |
| 417 | `channels` | `PASS` | 0.015 | informational command responded without failure pattern |
| 418 | `activech` | `PASS` | 0.016 | informational command responded without failure pattern |
| 419 | `status` | `PASS` | 0.031 | informational command responded without failure pattern |
| 420 | `status_raw` | `PASS` | 0.016 | informational command responded without failure pattern |
| 421 | `rawreg 0x7E` | `PASS` | 0.015 | informational command responded without failure pattern |
| 422 | `rawreg 0x7F` | `PASS` | 0.032 | informational command responded without failure pattern |
| 423 | `reg 0x7E` | `PASS` | 0.015 | informational command responded without failure pattern |
| 424 | `reg 0x7F` | `PASS` | 0.016 | informational command responded without failure pattern |
| 425 | `reg 0x19` | `PASS` | 0.031 | informational command responded without failure pattern |
| 426 | `reg 0x1A` | `PASS` | 0.016 | informational command responded without failure pattern |
| 427 | `reg 0x1B` | `PASS` | 0.015 | informational command responded without failure pattern |
| 428 | `sleep` | `PASS` | 0.032 | status output indicates OK |
| 429 | `wake` | `PASS` | 0.015 | status output indicates OK |
| 430 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 431 | `single 0` | `PASS` | 0.031 | status output indicates OK |
| 432 | `rcount 0 0x0123` | `PASS` | 0.016 | status output indicates OK |
| 433 | `settle 0 0x0011` | `PASS` | 0.015 | status output indicates OK |
| 434 | `clkdiv 0 2 3` | `PASS` | 0.032 | status output indicates OK |
| 435 | `offset 0 0x0010` | `PASS` | 0.015 | status output indicates OK |
| 436 | `idrive 0 5` | `PASS` | 0.016 | status output indicates OK |
| 437 | `rcount 1 0x0124` | `PASS` | 0.031 | status output indicates OK |
| 438 | `settle 1 0x0012` | `PASS` | 0.016 | status output indicates OK |
| 439 | `clkdiv 1 2 4` | `PASS` | 0.015 | status output indicates OK |
| 440 | `offset 1 0x0011` | `PASS` | 0.016 | status output indicates OK |
| 441 | `idrive 1 6` | `PASS` | 0.031 | status output indicates OK |
| 442 | `rcount 2 0x0125` | `PASS` | 0.016 | status output indicates OK |
| 443 | `settle 2 0x0013` | `PASS` | 0.016 | status output indicates OK |
| 444 | `clkdiv 2 2 5` | `PASS` | 0.031 | status output indicates OK |
| 445 | `offset 2 0x0012` | `PASS` | 0.015 | status output indicates OK |
| 446 | `idrive 2 7` | `PASS` | 0.016 | status output indicates OK |
| 447 | `rcount 3 0x0126` | `PASS` | 0.031 | status output indicates OK |
| 448 | `settle 3 0x0014` | `PASS` | 0.016 | status output indicates OK |
| 449 | `clkdiv 3 2 6` | `PASS` | 0.016 | status output indicates OK |
| 450 | `offset 3 0x0013` | `PASS` | 0.031 | status output indicates OK |
| 451 | `idrive 3 8` | `PASS` | 0.015 | status output indicates OK |
| 452 | `initidrive 0` | `PASS` | 0.016 | informational command responded without failure pattern |
| 453 | `initidrive 1` | `PASS` | 0.031 | informational command responded without failure pattern |
| 454 | `initidrive 2` | `PASS` | 0.016 | informational command responded without failure pattern |
| 455 | `initidrive 3` | `PASS` | 0.016 | informational command responded without failure pattern |
| 456 | `cfg` | `PASS` | 0.046 | informational command responded without failure pattern |
| 457 | `activech 1` | `PASS` | 0.016 | informational command responded without failure pattern |
| 458 | `activech 2` | `PASS` | 0.016 | informational command responded without failure pattern |
| 459 | `activech 3` | `PASS` | 0.031 | informational command responded without failure pattern |
| 460 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 461 | `single 1` | `PASS` | 0.015 | status output indicates OK |
| 462 | `single 2` | `PASS` | 0.031 | status output indicates OK |
| 463 | `single 3` | `PASS` | 0.016 | status output indicates OK |
| 464 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 465 | `autoscan 2` | `PASS` | 0.031 | status output indicates OK |
| 466 | `autoscan 3` | `PASS` | 0.016 | status output indicates OK |
| 467 | `autoscan 4` | `PASS` | 0.015 | status output indicates OK |
| 468 | `single 0` | `PASS` | 0.031 | status output indicates OK |
| 469 | `deglitch 1` | `PASS` | 0.016 | status output indicates OK |
| 470 | `deglitch 3` | `PASS` | 0.016 | status output indicates OK |
| 471 | `deglitch 10` | `PASS` | 0.031 | status output indicates OK |
| 472 | `deglitch 33` | `PASS` | 0.016 | status output indicates OK |
| 473 | `errcfg 0x0000` | `PASS` | 0.015 | informational command responded without failure pattern |
| 474 | `errcfg 0x00F9` | `PASS` | 0.031 | informational command responded without failure pattern |
| 475 | `errcfg` | `PASS` | 0.016 | informational command responded without failure pattern |
| 476 | `intb 0` | `PASS` | 0.016 | informational command responded without failure pattern |
| 477 | `intb 1` | `PASS` | 0.031 | informational command responded without failure pattern |
| 478 | `intb 0` | `PASS` | 0.016 | informational command responded without failure pattern |
| 479 | `refclk ext` | `PASS` | 0.015 | status output indicates OK |
| 480 | `refclk int` | `PASS` | 0.031 | status output indicates OK |
| 481 | `activate low` | `PASS` | 0.016 | status output indicates OK |
| 482 | `activate full` | `PASS` | 0.016 | status output indicates OK |
| 483 | `rpoverride 0` | `PASS` | 0.031 | status output indicates OK |
| 484 | `rpoverride 1` | `PASS` | 0.016 | status output indicates OK |
| 485 | `autoamp 1` | `PASS` | 0.015 | status output indicates OK |
| 486 | `autoamp 0` | `PASS` | 0.031 | status output indicates OK |
| 487 | `highcurrent 1` | `PASS` | 0.016 | status output indicates OK |
| 488 | `highcurrent 0` | `PASS` | 0.016 | status output indicates OK |
| 489 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 490 | `wreg 0x19 0x00F9` | `PASS` | 0.015 | status output indicates OK |
| 491 | `sync` | `PASS` | 0.016 | informational command responded without failure pattern |
| 492 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 493 | `rawwreg 0x19 0x00F9` | `PASS` | 0.015 | status output indicates OK |
| 494 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 495 | `resetreapply` | `PASS` | 0.016 | status output indicates OK |
| 496 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 497 | `recover` | `PASS` | 0.015 | status output indicates OK |
| 498 | `timing 0 43000000` | `PASS` | 0.032 | informational command responded without failure pattern |
| 499 | `timing 1 43000000` | `PASS` | 0.015 | informational command responded without failure pattern |
| 500 | `timing 2 43000000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 501 | `timing 3 43000000` | `PASS` | 0.031 | informational command responded without failure pattern |
| 502 | `reset` | `PASS` | 0.016 | status output indicates OK |
| 503 | `init` | `PASS` | 0.015 | informational command responded without failure pattern |
| 504 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 505 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 506 | `help` | `PASS` | 0.047 | informational command responded without failure pattern |
| 507 | `version` | `PASS` | 0.015 | informational command responded without failure pattern |
| 508 | `init` | `PASS` | 0.016 | informational command responded without failure pattern |
| 509 | `scan` | `PASS` | 0.156 | informational command responded without failure pattern |
| 510 | `probeaddr 0x2A` | `PASS` | 0.016 | status output indicates OK |
| 511 | `probe` | `PASS` | 0.015 | status output indicates OK |
| 512 | `id` | `PASS` | 0.032 | informational command responded without failure pattern |
| 513 | `drv` | `PASS` | 0.015 | informational command responded without failure pattern |
| 514 | `state` | `PASS` | 0.016 | informational command responded without failure pattern |
| 515 | `online` | `PASS` | 0.031 | informational command responded without failure pattern |
| 516 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 517 | `snapshot` | `PASS` | 0.032 | informational command responded without failure pattern |
| 518 | `channels` | `PASS` | 0.015 | informational command responded without failure pattern |
| 519 | `activech` | `PASS` | 0.016 | informational command responded without failure pattern |
| 520 | `status` | `PASS` | 0.016 | informational command responded without failure pattern |
| 521 | `status_raw` | `PASS` | 0.031 | informational command responded without failure pattern |
| 522 | `rawreg 0x7E` | `PASS` | 0.015 | informational command responded without failure pattern |
| 523 | `rawreg 0x7F` | `PASS` | 0.016 | informational command responded without failure pattern |
| 524 | `reg 0x7E` | `PASS` | 0.031 | informational command responded without failure pattern |
| 525 | `reg 0x7F` | `PASS` | 0.016 | informational command responded without failure pattern |
| 526 | `reg 0x19` | `PASS` | 0.016 | informational command responded without failure pattern |
| 527 | `reg 0x1A` | `PASS` | 0.031 | informational command responded without failure pattern |
| 528 | `reg 0x1B` | `PASS` | 0.015 | informational command responded without failure pattern |
| 529 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 530 | `wake` | `PASS` | 0.031 | status output indicates OK |
| 531 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 532 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 533 | `rcount 0 0x0123` | `PASS` | 0.015 | status output indicates OK |
| 534 | `settle 0 0x0011` | `PASS` | 0.031 | status output indicates OK |
| 535 | `clkdiv 0 2 3` | `PASS` | 0.016 | status output indicates OK |
| 536 | `offset 0 0x0010` | `PASS` | 0.016 | status output indicates OK |
| 537 | `idrive 0 5` | `PASS` | 0.031 | status output indicates OK |
| 538 | `rcount 1 0x0124` | `PASS` | 0.016 | status output indicates OK |
| 539 | `settle 1 0x0012` | `PASS` | 0.015 | status output indicates OK |
| 540 | `clkdiv 1 2 4` | `PASS` | 0.031 | status output indicates OK |
| 541 | `offset 1 0x0011` | `PASS` | 0.016 | status output indicates OK |
| 542 | `idrive 1 6` | `PASS` | 0.016 | status output indicates OK |
| 543 | `rcount 2 0x0125` | `PASS` | 0.031 | status output indicates OK |
| 544 | `settle 2 0x0013` | `PASS` | 0.016 | status output indicates OK |
| 545 | `clkdiv 2 2 5` | `PASS` | 0.015 | status output indicates OK |
| 546 | `offset 2 0x0012` | `PASS` | 0.031 | status output indicates OK |
| 547 | `idrive 2 7` | `PASS` | 0.016 | status output indicates OK |
| 548 | `rcount 3 0x0126` | `PASS` | 0.016 | status output indicates OK |
| 549 | `settle 3 0x0014` | `PASS` | 0.031 | status output indicates OK |
| 550 | `clkdiv 3 2 6` | `PASS` | 0.016 | status output indicates OK |
| 551 | `offset 3 0x0013` | `PASS` | 0.015 | status output indicates OK |
| 552 | `idrive 3 8` | `PASS` | 0.031 | status output indicates OK |
| 553 | `initidrive 0` | `PASS` | 0.016 | informational command responded without failure pattern |
| 554 | `initidrive 1` | `PASS` | 0.016 | informational command responded without failure pattern |
| 555 | `initidrive 2` | `PASS` | 0.031 | informational command responded without failure pattern |
| 556 | `initidrive 3` | `PASS` | 0.016 | informational command responded without failure pattern |
| 557 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 558 | `activech 1` | `PASS` | 0.031 | informational command responded without failure pattern |
| 559 | `activech 2` | `PASS` | 0.016 | informational command responded without failure pattern |
| 560 | `activech 3` | `PASS` | 0.015 | informational command responded without failure pattern |
| 561 | `single 0` | `PASS` | 0.032 | status output indicates OK |
| 562 | `single 1` | `PASS` | 0.015 | status output indicates OK |
| 563 | `single 2` | `PASS` | 0.016 | status output indicates OK |
| 564 | `single 3` | `PASS` | 0.031 | status output indicates OK |
| 565 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 566 | `autoscan 2` | `PASS` | 0.015 | status output indicates OK |
| 567 | `autoscan 3` | `PASS` | 0.032 | status output indicates OK |
| 568 | `autoscan 4` | `PASS` | 0.015 | status output indicates OK |
| 569 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 570 | `deglitch 1` | `PASS` | 0.031 | status output indicates OK |
| 571 | `deglitch 3` | `PASS` | 0.016 | status output indicates OK |
| 572 | `deglitch 10` | `PASS` | 0.015 | status output indicates OK |
| 573 | `deglitch 33` | `PASS` | 0.016 | status output indicates OK |
| 574 | `errcfg 0x0000` | `PASS` | 0.031 | informational command responded without failure pattern |
| 575 | `errcfg 0x00F9` | `PASS` | 0.016 | informational command responded without failure pattern |
| 576 | `errcfg` | `PASS` | 0.015 | informational command responded without failure pattern |
| 577 | `intb 0` | `PASS` | 0.032 | informational command responded without failure pattern |
| 578 | `intb 1` | `PASS` | 0.015 | informational command responded without failure pattern |
| 579 | `intb 0` | `PASS` | 0.016 | informational command responded without failure pattern |
| 580 | `refclk ext` | `PASS` | 0.031 | status output indicates OK |
| 581 | `refclk int` | `PASS` | 0.016 | status output indicates OK |
| 582 | `activate low` | `PASS` | 0.015 | status output indicates OK |
| 583 | `activate full` | `PASS` | 0.032 | status output indicates OK |
| 584 | `rpoverride 0` | `PASS` | 0.015 | status output indicates OK |
| 585 | `rpoverride 1` | `PASS` | 0.016 | status output indicates OK |
| 586 | `autoamp 1` | `PASS` | 0.031 | status output indicates OK |
| 587 | `autoamp 0` | `PASS` | 0.016 | status output indicates OK |
| 588 | `highcurrent 1` | `PASS` | 0.015 | status output indicates OK |
| 589 | `highcurrent 0` | `PASS` | 0.032 | status output indicates OK |
| 590 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 591 | `wreg 0x19 0x00F9` | `PASS` | 0.031 | status output indicates OK |
| 592 | `sync` | `PASS` | 0.016 | informational command responded without failure pattern |
| 593 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 594 | `rawwreg 0x19 0x00F9` | `PASS` | 0.031 | status output indicates OK |
| 595 | `cfg` | `PASS` | 0.032 | informational command responded without failure pattern |
| 596 | `resetreapply` | `PASS` | 0.031 | status output indicates OK |
| 597 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 598 | `recover` | `PASS` | 0.031 | status output indicates OK |
| 599 | `timing 0 43000000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 600 | `timing 1 43000000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 601 | `timing 2 43000000` | `PASS` | 0.031 | informational command responded without failure pattern |
| 602 | `timing 3 43000000` | `PASS` | 0.015 | informational command responded without failure pattern |
| 603 | `reset` | `PASS` | 0.016 | status output indicates OK |
| 604 | `init` | `PASS` | 0.031 | informational command responded without failure pattern |
| 605 | `cfg` | `PASS` | 0.032 | informational command responded without failure pattern |
| 606 | `sleep` | `PASS` | 0.031 | status output indicates OK |
| 607 | `help` | `PASS` | 0.031 | informational command responded without failure pattern |
| 608 | `version` | `PASS` | 0.031 | informational command responded without failure pattern |
| 609 | `init` | `PASS` | 0.016 | informational command responded without failure pattern |
| 610 | `scan` | `PASS` | 0.141 | informational command responded without failure pattern |
| 611 | `probeaddr 0x2A` | `PASS` | 0.031 | status output indicates OK |
| 612 | `probe` | `PASS` | 0.015 | status output indicates OK |
| 613 | `id` | `PASS` | 0.016 | informational command responded without failure pattern |
| 614 | `drv` | `PASS` | 0.016 | informational command responded without failure pattern |
| 615 | `state` | `PASS` | 0.031 | informational command responded without failure pattern |
| 616 | `online` | `PASS` | 0.016 | informational command responded without failure pattern |
| 617 | `cfg` | `PASS` | 0.046 | informational command responded without failure pattern |
| 618 | `snapshot` | `PASS` | 0.016 | informational command responded without failure pattern |
| 619 | `channels` | `PASS` | 0.016 | informational command responded without failure pattern |
| 620 | `activech` | `PASS` | 0.031 | informational command responded without failure pattern |
| 621 | `status` | `PASS` | 0.016 | informational command responded without failure pattern |
| 622 | `status_raw` | `PASS` | 0.015 | informational command responded without failure pattern |
| 623 | `rawreg 0x7E` | `PASS` | 0.031 | informational command responded without failure pattern |
| 624 | `rawreg 0x7F` | `PASS` | 0.016 | informational command responded without failure pattern |
| 625 | `reg 0x7E` | `PASS` | 0.016 | informational command responded without failure pattern |
| 626 | `reg 0x7F` | `PASS` | 0.031 | informational command responded without failure pattern |
| 627 | `reg 0x19` | `PASS` | 0.016 | informational command responded without failure pattern |
| 628 | `reg 0x1A` | `PASS` | 0.015 | informational command responded without failure pattern |
| 629 | `reg 0x1B` | `PASS` | 0.031 | informational command responded without failure pattern |
| 630 | `sleep` | `PASS` | 0.016 | status output indicates OK |
| 631 | `wake` | `PASS` | 0.016 | status output indicates OK |
| 632 | `sleep` | `PASS` | 0.015 | status output indicates OK |
| 633 | `single 0` | `PASS` | 0.032 | status output indicates OK |
| 634 | `rcount 0 0x0123` | `PASS` | 0.015 | status output indicates OK |
| 635 | `settle 0 0x0011` | `PASS` | 0.016 | status output indicates OK |
| 636 | `clkdiv 0 2 3` | `PASS` | 0.031 | status output indicates OK |
| 637 | `offset 0 0x0010` | `PASS` | 0.016 | status output indicates OK |
| 638 | `idrive 0 5` | `PASS` | 0.015 | status output indicates OK |
| 639 | `rcount 1 0x0124` | `PASS` | 0.032 | status output indicates OK |
| 640 | `settle 1 0x0012` | `PASS` | 0.015 | status output indicates OK |
| 641 | `clkdiv 1 2 4` | `PASS` | 0.016 | status output indicates OK |
| 642 | `offset 1 0x0011` | `PASS` | 0.031 | status output indicates OK |
| 643 | `idrive 1 6` | `PASS` | 0.016 | status output indicates OK |
| 644 | `rcount 2 0x0125` | `PASS` | 0.015 | status output indicates OK |
| 645 | `settle 2 0x0013` | `PASS` | 0.032 | status output indicates OK |
| 646 | `clkdiv 2 2 5` | `PASS` | 0.015 | status output indicates OK |
| 647 | `offset 2 0x0012` | `PASS` | 0.016 | status output indicates OK |
| 648 | `idrive 2 7` | `PASS` | 0.031 | status output indicates OK |
| 649 | `rcount 3 0x0126` | `PASS` | 0.016 | status output indicates OK |
| 650 | `settle 3 0x0014` | `PASS` | 0.015 | status output indicates OK |
| 651 | `clkdiv 3 2 6` | `PASS` | 0.016 | status output indicates OK |
| 652 | `offset 3 0x0013` | `PASS` | 0.031 | status output indicates OK |
| 653 | `idrive 3 8` | `PASS` | 0.016 | status output indicates OK |
| 654 | `initidrive 0` | `PASS` | 0.015 | informational command responded without failure pattern |
| 655 | `initidrive 1` | `PASS` | 0.032 | informational command responded without failure pattern |
| 656 | `initidrive 2` | `PASS` | 0.015 | informational command responded without failure pattern |
| 657 | `initidrive 3` | `PASS` | 0.016 | informational command responded without failure pattern |
| 658 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 659 | `activech 1` | `PASS` | 0.015 | informational command responded without failure pattern |
| 660 | `activech 2` | `PASS` | 0.032 | informational command responded without failure pattern |
| 661 | `activech 3` | `PASS` | 0.015 | informational command responded without failure pattern |
| 662 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 663 | `single 1` | `PASS` | 0.031 | status output indicates OK |
| 664 | `single 2` | `PASS` | 0.016 | status output indicates OK |
| 665 | `single 3` | `PASS` | 0.015 | status output indicates OK |
| 666 | `single 0` | `PASS` | 0.032 | status output indicates OK |
| 667 | `autoscan 2` | `PASS` | 0.015 | status output indicates OK |
| 668 | `autoscan 3` | `PASS` | 0.016 | status output indicates OK |
| 669 | `autoscan 4` | `PASS` | 0.031 | status output indicates OK |
| 670 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 671 | `deglitch 1` | `PASS` | 0.015 | status output indicates OK |
| 672 | `deglitch 3` | `PASS` | 0.032 | status output indicates OK |
| 673 | `deglitch 10` | `PASS` | 0.015 | status output indicates OK |
| 674 | `deglitch 33` | `PASS` | 0.016 | status output indicates OK |
| 675 | `errcfg 0x0000` | `PASS` | 0.031 | informational command responded without failure pattern |
| 676 | `errcfg 0x00F9` | `PASS` | 0.016 | informational command responded without failure pattern |
| 677 | `errcfg` | `PASS` | 0.015 | informational command responded without failure pattern |
| 678 | `intb 0` | `PASS` | 0.032 | informational command responded without failure pattern |
| 679 | `intb 1` | `PASS` | 0.015 | informational command responded without failure pattern |
| 680 | `intb 0` | `PASS` | 0.016 | informational command responded without failure pattern |
| 681 | `refclk ext` | `PASS` | 0.016 | status output indicates OK |
| 682 | `refclk int` | `PASS` | 0.031 | status output indicates OK |
| 683 | `activate low` | `PASS` | 0.015 | status output indicates OK |
| 684 | `activate full` | `PASS` | 0.016 | status output indicates OK |
| 685 | `rpoverride 0` | `PASS` | 0.031 | status output indicates OK |
| 686 | `rpoverride 1` | `PASS` | 0.016 | status output indicates OK |
| 687 | `autoamp 1` | `PASS` | 0.016 | status output indicates OK |
| 688 | `autoamp 0` | `PASS` | 0.031 | status output indicates OK |
| 689 | `highcurrent 1` | `PASS` | 0.015 | status output indicates OK |
| 690 | `highcurrent 0` | `PASS` | 0.016 | status output indicates OK |
| 691 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 692 | `wreg 0x19 0x00F9` | `PASS` | 0.016 | status output indicates OK |
| 693 | `sync` | `PASS` | 0.031 | informational command responded without failure pattern |
| 694 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 695 | `rawwreg 0x19 0x00F9` | `PASS` | 0.031 | status output indicates OK |
| 696 | `cfg` | `PASS` | 0.032 | informational command responded without failure pattern |
| 697 | `resetreapply` | `PASS` | 0.015 | status output indicates OK |
| 698 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 699 | `recover` | `PASS` | 0.016 | status output indicates OK |
| 700 | `timing 0 43000000` | `PASS` | 0.031 | informational command responded without failure pattern |
| 701 | `timing 1 43000000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 702 | `timing 2 43000000` | `PASS` | 0.015 | informational command responded without failure pattern |
| 703 | `timing 3 43000000` | `PASS` | 0.031 | informational command responded without failure pattern |
| 704 | `reset` | `PASS` | 0.016 | status output indicates OK |
| 705 | `init` | `PASS` | 0.016 | informational command responded without failure pattern |
| 706 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 707 | `sleep` | `PASS` | 0.015 | status output indicates OK |
| 708 | `help` | `PASS` | 0.047 | informational command responded without failure pattern |
| 709 | `version` | `PASS` | 0.016 | informational command responded without failure pattern |
| 710 | `init` | `PASS` | 0.031 | informational command responded without failure pattern |
| 711 | `scan` | `PASS` | 0.141 | informational command responded without failure pattern |
| 712 | `probeaddr 0x2A` | `PASS` | 0.015 | status output indicates OK |
| 713 | `probe` | `PASS` | 0.016 | status output indicates OK |
| 714 | `id` | `PASS` | 0.031 | informational command responded without failure pattern |
| 715 | `drv` | `PASS` | 0.016 | informational command responded without failure pattern |
| 716 | `state` | `PASS` | 0.015 | informational command responded without failure pattern |
| 717 | `online` | `PASS` | 0.032 | informational command responded without failure pattern |
| 718 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 719 | `snapshot` | `PASS` | 0.031 | informational command responded without failure pattern |
| 720 | `channels` | `PASS` | 0.016 | informational command responded without failure pattern |
| 721 | `activech` | `PASS` | 0.015 | informational command responded without failure pattern |
| 722 | `status` | `PASS` | 0.032 | informational command responded without failure pattern |
| 723 | `status_raw` | `PASS` | 0.015 | informational command responded without failure pattern |
| 724 | `rawreg 0x7E` | `PASS` | 0.016 | informational command responded without failure pattern |
| 725 | `rawreg 0x7F` | `PASS` | 0.031 | informational command responded without failure pattern |
| 726 | `reg 0x7E` | `PASS` | 0.016 | informational command responded without failure pattern |
| 727 | `reg 0x7F` | `PASS` | 0.015 | informational command responded without failure pattern |
| 728 | `reg 0x19` | `PASS` | 0.032 | informational command responded without failure pattern |
| 729 | `reg 0x1A` | `PASS` | 0.015 | informational command responded without failure pattern |
| 730 | `reg 0x1B` | `PASS` | 0.016 | informational command responded without failure pattern |
| 731 | `sleep` | `PASS` | 0.015 | status output indicates OK |
| 732 | `wake` | `PASS` | 0.032 | status output indicates OK |
| 733 | `sleep` | `PASS` | 0.015 | status output indicates OK |
| 734 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 735 | `rcount 0 0x0123` | `PASS` | 0.031 | status output indicates OK |
| 736 | `settle 0 0x0011` | `PASS` | 0.016 | status output indicates OK |
| 737 | `clkdiv 0 2 3` | `PASS` | 0.015 | status output indicates OK |
| 738 | `offset 0 0x0010` | `PASS` | 0.032 | status output indicates OK |
| 739 | `idrive 0 5` | `PASS` | 0.015 | status output indicates OK |
| 740 | `rcount 1 0x0124` | `PASS` | 0.016 | status output indicates OK |
| 741 | `settle 1 0x0012` | `PASS` | 0.031 | status output indicates OK |
| 742 | `clkdiv 1 2 4` | `PASS` | 0.016 | status output indicates OK |
| 743 | `offset 1 0x0011` | `PASS` | 0.015 | status output indicates OK |
| 744 | `idrive 1 6` | `PASS` | 0.032 | status output indicates OK |
| 745 | `rcount 2 0x0125` | `PASS` | 0.015 | status output indicates OK |
| 746 | `settle 2 0x0013` | `PASS` | 0.016 | status output indicates OK |
| 747 | `clkdiv 2 2 5` | `PASS` | 0.016 | status output indicates OK |
| 748 | `offset 2 0x0012` | `PASS` | 0.031 | status output indicates OK |
| 749 | `idrive 2 7` | `PASS` | 0.015 | status output indicates OK |
| 750 | `rcount 3 0x0126` | `PASS` | 0.016 | status output indicates OK |
| 751 | `settle 3 0x0014` | `PASS` | 0.031 | status output indicates OK |
| 752 | `clkdiv 3 2 6` | `PASS` | 0.016 | status output indicates OK |
| 753 | `offset 3 0x0013` | `PASS` | 0.016 | status output indicates OK |
| 754 | `idrive 3 8` | `PASS` | 0.031 | status output indicates OK |
| 755 | `initidrive 0` | `PASS` | 0.015 | informational command responded without failure pattern |
| 756 | `initidrive 1` | `PASS` | 0.016 | informational command responded without failure pattern |
| 757 | `initidrive 2` | `PASS` | 0.031 | informational command responded without failure pattern |
| 758 | `initidrive 3` | `PASS` | 0.016 | informational command responded without failure pattern |
| 759 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 760 | `activech 1` | `PASS` | 0.015 | informational command responded without failure pattern |
| 761 | `activech 2` | `PASS` | 0.016 | informational command responded without failure pattern |
| 762 | `activech 3` | `PASS` | 0.031 | informational command responded without failure pattern |
| 763 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 764 | `single 1` | `PASS` | 0.016 | status output indicates OK |
| 765 | `single 2` | `PASS` | 0.015 | status output indicates OK |
| 766 | `single 3` | `PASS` | 0.015 | status output indicates OK |
| 767 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 768 | `autoscan 2` | `PASS` | 0.031 | status output indicates OK |
| 769 | `autoscan 3` | `PASS` | 0.016 | status output indicates OK |
| 770 | `autoscan 4` | `PASS` | 0.016 | status output indicates OK |
| 771 | `single 0` | `PASS` | 0.031 | status output indicates OK |
| 772 | `deglitch 1` | `PASS` | 0.015 | status output indicates OK |
| 773 | `deglitch 3` | `PASS` | 0.016 | status output indicates OK |
| 774 | `deglitch 10` | `PASS` | 0.031 | status output indicates OK |
| 775 | `deglitch 33` | `PASS` | 0.016 | status output indicates OK |
| 776 | `errcfg 0x0000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 777 | `errcfg 0x00F9` | `PASS` | 0.015 | informational command responded without failure pattern |
| 778 | `errcfg` | `PASS` | 0.015 | informational command responded without failure pattern |
| 779 | `intb 0` | `PASS` | 0.016 | informational command responded without failure pattern |
| 780 | `intb 1` | `PASS` | 0.031 | informational command responded without failure pattern |
| 781 | `intb 0` | `PASS` | 0.016 | informational command responded without failure pattern |
| 782 | `refclk ext` | `PASS` | 0.016 | status output indicates OK |
| 783 | `refclk int` | `PASS` | 0.031 | status output indicates OK |
| 784 | `activate low` | `PASS` | 0.015 | status output indicates OK |
| 785 | `activate full` | `PASS` | 0.016 | status output indicates OK |
| 786 | `rpoverride 0` | `PASS` | 0.016 | status output indicates OK |
| 787 | `rpoverride 1` | `PASS` | 0.031 | status output indicates OK |
| 788 | `autoamp 1` | `PASS` | 0.016 | status output indicates OK |
| 789 | `autoamp 0` | `PASS` | 0.015 | status output indicates OK |
| 790 | `highcurrent 1` | `PASS` | 0.031 | status output indicates OK |
| 791 | `highcurrent 0` | `PASS` | 0.016 | status output indicates OK |
| 792 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 793 | `wreg 0x19 0x00F9` | `PASS` | 0.016 | status output indicates OK |
| 794 | `sync` | `PASS` | 0.015 | informational command responded without failure pattern |
| 795 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 796 | `rawwreg 0x19 0x00F9` | `PASS` | 0.016 | status output indicates OK |
| 797 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 798 | `resetreapply` | `PASS` | 0.015 | status output indicates OK |
| 799 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 800 | `recover` | `PASS` | 0.016 | status output indicates OK |
| 801 | `timing 0 43000000` | `PASS` | 0.031 | informational command responded without failure pattern |
| 802 | `timing 1 43000000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 803 | `timing 2 43000000` | `PASS` | 0.015 | informational command responded without failure pattern |
| 804 | `timing 3 43000000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 805 | `reset` | `PASS` | 0.031 | status output indicates OK |
| 806 | `init` | `PASS` | 0.016 | informational command responded without failure pattern |
| 807 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 808 | `sleep` | `PASS` | 0.015 | status output indicates OK |
| 809 | `help` | `PASS` | 0.047 | informational command responded without failure pattern |
| 810 | `version` | `PASS` | 0.016 | informational command responded without failure pattern |
| 811 | `init` | `PASS` | 0.015 | informational command responded without failure pattern |
| 812 | `scan` | `PASS` | 0.157 | informational command responded without failure pattern |
| 813 | `probeaddr 0x2A` | `PASS` | 0.015 | status output indicates OK |
| 814 | `probe` | `PASS` | 0.016 | status output indicates OK |
| 815 | `id` | `PASS` | 0.031 | informational command responded without failure pattern |
| 816 | `drv` | `PASS` | 0.016 | informational command responded without failure pattern |
| 817 | `state` | `PASS` | 0.015 | informational command responded without failure pattern |
| 818 | `online` | `PASS` | 0.016 | informational command responded without failure pattern |
| 819 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 820 | `snapshot` | `PASS` | 0.015 | informational command responded without failure pattern |
| 821 | `channels` | `PASS` | 0.032 | informational command responded without failure pattern |
| 822 | `activech` | `PASS` | 0.015 | informational command responded without failure pattern |
| 823 | `status` | `PASS` | 0.016 | informational command responded without failure pattern |
| 824 | `status_raw` | `PASS` | 0.031 | informational command responded without failure pattern |
| 825 | `rawreg 0x7E` | `PASS` | 0.016 | informational command responded without failure pattern |
| 826 | `rawreg 0x7F` | `PASS` | 0.015 | informational command responded without failure pattern |
| 827 | `reg 0x7E` | `PASS` | 0.032 | informational command responded without failure pattern |
| 828 | `reg 0x7F` | `PASS` | 0.015 | informational command responded without failure pattern |
| 829 | `reg 0x19` | `PASS` | 0.016 | informational command responded without failure pattern |
| 830 | `reg 0x1A` | `PASS` | 0.031 | informational command responded without failure pattern |
| 831 | `reg 0x1B` | `PASS` | 0.016 | informational command responded without failure pattern |
| 832 | `sleep` | `PASS` | 0.015 | status output indicates OK |
| 833 | `wake` | `PASS` | 0.032 | status output indicates OK |
| 834 | `sleep` | `PASS` | 0.015 | status output indicates OK |
| 835 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 836 | `rcount 0 0x0123` | `PASS` | 0.016 | status output indicates OK |
| 837 | `settle 0 0x0011` | `PASS` | 0.031 | status output indicates OK |
| 838 | `clkdiv 0 2 3` | `PASS` | 0.015 | status output indicates OK |
| 839 | `offset 0 0x0010` | `PASS` | 0.016 | status output indicates OK |
| 840 | `idrive 0 5` | `PASS` | 0.031 | status output indicates OK |
| 841 | `rcount 1 0x0124` | `PASS` | 0.016 | status output indicates OK |
| 842 | `settle 1 0x0012` | `PASS` | 0.016 | status output indicates OK |
| 843 | `clkdiv 1 2 4` | `PASS` | 0.031 | status output indicates OK |
| 844 | `offset 1 0x0011` | `PASS` | 0.015 | status output indicates OK |
| 845 | `idrive 1 6` | `PASS` | 0.016 | status output indicates OK |
| 846 | `rcount 2 0x0125` | `PASS` | 0.031 | status output indicates OK |
| 847 | `settle 2 0x0013` | `PASS` | 0.016 | status output indicates OK |
| 848 | `clkdiv 2 2 5` | `PASS` | 0.016 | status output indicates OK |
| 849 | `offset 2 0x0012` | `PASS` | 0.031 | status output indicates OK |
| 850 | `idrive 2 7` | `PASS` | 0.015 | status output indicates OK |
| 851 | `rcount 3 0x0126` | `PASS` | 0.016 | status output indicates OK |
| 852 | `settle 3 0x0014` | `PASS` | 0.031 | status output indicates OK |
| 853 | `clkdiv 3 2 6` | `PASS` | 0.016 | status output indicates OK |
| 854 | `offset 3 0x0013` | `PASS` | 0.016 | status output indicates OK |
| 855 | `idrive 3 8` | `PASS` | 0.015 | status output indicates OK |
| 856 | `initidrive 0` | `PASS` | 0.031 | informational command responded without failure pattern |
| 857 | `initidrive 1` | `PASS` | 0.016 | informational command responded without failure pattern |
| 858 | `initidrive 2` | `PASS` | 0.016 | informational command responded without failure pattern |
| 859 | `initidrive 3` | `PASS` | 0.031 | informational command responded without failure pattern |
| 860 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 861 | `activech 1` | `PASS` | 0.031 | informational command responded without failure pattern |
| 862 | `activech 2` | `PASS` | 0.016 | informational command responded without failure pattern |
| 863 | `activech 3` | `PASS` | 0.016 | informational command responded without failure pattern |
| 864 | `single 0` | `PASS` | 0.031 | status output indicates OK |
| 865 | `single 1` | `PASS` | 0.016 | status output indicates OK |
| 866 | `single 2` | `PASS` | 0.015 | status output indicates OK |
| 867 | `single 3` | `PASS` | 0.031 | status output indicates OK |
| 868 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 869 | `autoscan 2` | `PASS` | 0.016 | status output indicates OK |
| 870 | `autoscan 3` | `PASS` | 0.015 | status output indicates OK |
| 871 | `autoscan 4` | `PASS` | 0.032 | status output indicates OK |
| 872 | `single 0` | `PASS` | 0.015 | status output indicates OK |
| 873 | `deglitch 1` | `PASS` | 0.016 | status output indicates OK |
| 874 | `deglitch 3` | `PASS` | 0.031 | status output indicates OK |
| 875 | `deglitch 10` | `PASS` | 0.016 | status output indicates OK |
| 876 | `deglitch 33` | `PASS` | 0.015 | status output indicates OK |
| 877 | `errcfg 0x0000` | `PASS` | 0.032 | informational command responded without failure pattern |
| 878 | `errcfg 0x00F9` | `PASS` | 0.015 | informational command responded without failure pattern |
| 879 | `errcfg` | `PASS` | 0.016 | informational command responded without failure pattern |
| 880 | `intb 0` | `PASS` | 0.031 | informational command responded without failure pattern |
| 881 | `intb 1` | `PASS` | 0.016 | informational command responded without failure pattern |
| 882 | `intb 0` | `PASS` | 0.015 | informational command responded without failure pattern |
| 883 | `refclk ext` | `PASS` | 0.032 | status output indicates OK |
| 884 | `refclk int` | `PASS` | 0.015 | status output indicates OK |
| 885 | `activate low` | `PASS` | 0.016 | status output indicates OK |
| 886 | `activate full` | `PASS` | 0.031 | status output indicates OK |
| 887 | `rpoverride 0` | `PASS` | 0.016 | status output indicates OK |
| 888 | `rpoverride 1` | `PASS` | 0.015 | status output indicates OK |
| 889 | `autoamp 1` | `PASS` | 0.032 | status output indicates OK |
| 890 | `autoamp 0` | `PASS` | 0.015 | status output indicates OK |
| 891 | `highcurrent 1` | `PASS` | 0.016 | status output indicates OK |
| 892 | `highcurrent 0` | `PASS` | 0.031 | status output indicates OK |
| 893 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 894 | `wreg 0x19 0x00F9` | `PASS` | 0.032 | status output indicates OK |
| 895 | `sync` | `PASS` | 0.015 | informational command responded without failure pattern |
| 896 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 897 | `rawwreg 0x19 0x00F9` | `PASS` | 0.016 | status output indicates OK |
| 898 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 899 | `resetreapply` | `PASS` | 0.015 | status output indicates OK |
| 900 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 901 | `recover` | `PASS` | 0.016 | status output indicates OK |
| 902 | `timing 0 43000000` | `PASS` | 0.015 | informational command responded without failure pattern |
| 903 | `timing 1 43000000` | `PASS` | 0.032 | informational command responded without failure pattern |
| 904 | `timing 2 43000000` | `PASS` | 0.015 | informational command responded without failure pattern |
| 905 | `timing 3 43000000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 906 | `reset` | `PASS` | 0.031 | status output indicates OK |
| 907 | `init` | `PASS` | 0.016 | informational command responded without failure pattern |
| 908 | `cfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 909 | `sleep` | `PASS` | 0.031 | status output indicates OK |
| 910 | `help` | `PASS` | 0.031 | informational command responded without failure pattern |
| 911 | `version` | `PASS` | 0.016 | informational command responded without failure pattern |
| 912 | `init` | `PASS` | 0.015 | informational command responded without failure pattern |
| 913 | `scan` | `PASS` | 0.141 | informational command responded without failure pattern |
| 914 | `probeaddr 0x2A` | `PASS` | 0.031 | status output indicates OK |
| 915 | `probe` | `PASS` | 0.016 | status output indicates OK |
| 916 | `id` | `PASS` | 0.015 | informational command responded without failure pattern |
| 917 | `drv` | `PASS` | 0.032 | informational command responded without failure pattern |
| 918 | `state` | `PASS` | 0.015 | informational command responded without failure pattern |
| 919 | `online` | `PASS` | 0.016 | informational command responded without failure pattern |
| 920 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 921 | `snapshot` | `PASS` | 0.015 | informational command responded without failure pattern |
| 922 | `channels` | `PASS` | 0.032 | informational command responded without failure pattern |
| 923 | `activech` | `PASS` | 0.015 | informational command responded without failure pattern |
| 924 | `status` | `PASS` | 0.016 | informational command responded without failure pattern |
| 925 | `status_raw` | `PASS` | 0.031 | informational command responded without failure pattern |
| 926 | `rawreg 0x7E` | `PASS` | 0.016 | informational command responded without failure pattern |
| 927 | `rawreg 0x7F` | `PASS` | 0.015 | informational command responded without failure pattern |
| 928 | `reg 0x7E` | `PASS` | 0.032 | informational command responded without failure pattern |
| 929 | `reg 0x7F` | `PASS` | 0.015 | informational command responded without failure pattern |
| 930 | `reg 0x19` | `PASS` | 0.016 | informational command responded without failure pattern |
| 931 | `reg 0x1A` | `PASS` | 0.031 | informational command responded without failure pattern |
| 932 | `reg 0x1B` | `PASS` | 0.016 | informational command responded without failure pattern |
| 933 | `sleep` | `PASS` | 0.015 | status output indicates OK |
| 934 | `wake` | `PASS` | 0.016 | status output indicates OK |
| 935 | `sleep` | `PASS` | 0.031 | status output indicates OK |
| 936 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 937 | `rcount 0 0x0123` | `PASS` | 0.016 | status output indicates OK |
| 938 | `settle 0 0x0011` | `PASS` | 0.031 | status output indicates OK |
| 939 | `clkdiv 0 2 3` | `PASS` | 0.015 | status output indicates OK |
| 940 | `offset 0 0x0010` | `PASS` | 0.016 | status output indicates OK |
| 941 | `idrive 0 5` | `PASS` | 0.031 | status output indicates OK |
| 942 | `rcount 1 0x0124` | `PASS` | 0.016 | status output indicates OK |
| 943 | `settle 1 0x0012` | `PASS` | 0.016 | status output indicates OK |
| 944 | `clkdiv 1 2 4` | `PASS` | 0.031 | status output indicates OK |
| 945 | `offset 1 0x0011` | `PASS` | 0.015 | status output indicates OK |
| 946 | `idrive 1 6` | `PASS` | 0.016 | status output indicates OK |
| 947 | `rcount 2 0x0125` | `PASS` | 0.031 | status output indicates OK |
| 948 | `settle 2 0x0013` | `PASS` | 0.016 | status output indicates OK |
| 949 | `clkdiv 2 2 5` | `PASS` | 0.016 | status output indicates OK |
| 950 | `offset 2 0x0012` | `PASS` | 0.031 | status output indicates OK |
| 951 | `idrive 2 7` | `PASS` | 0.015 | status output indicates OK |
| 952 | `rcount 3 0x0126` | `PASS` | 0.016 | status output indicates OK |
| 953 | `settle 3 0x0014` | `PASS` | 0.031 | status output indicates OK |
| 954 | `clkdiv 3 2 6` | `PASS` | 0.016 | status output indicates OK |
| 955 | `offset 3 0x0013` | `PASS` | 0.016 | status output indicates OK |
| 956 | `idrive 3 8` | `PASS` | 0.031 | status output indicates OK |
| 957 | `initidrive 0` | `PASS` | 0.015 | informational command responded without failure pattern |
| 958 | `initidrive 1` | `PASS` | 0.016 | informational command responded without failure pattern |
| 959 | `initidrive 2` | `PASS` | 0.031 | informational command responded without failure pattern |
| 960 | `initidrive 3` | `PASS` | 0.016 | informational command responded without failure pattern |
| 961 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 962 | `activech 1` | `PASS` | 0.015 | informational command responded without failure pattern |
| 963 | `activech 2` | `PASS` | 0.016 | informational command responded without failure pattern |
| 964 | `activech 3` | `PASS` | 0.031 | informational command responded without failure pattern |
| 965 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 966 | `single 1` | `PASS` | 0.016 | status output indicates OK |
| 967 | `single 2` | `PASS` | 0.031 | status output indicates OK |
| 968 | `single 3` | `PASS` | 0.015 | status output indicates OK |
| 969 | `single 0` | `PASS` | 0.016 | status output indicates OK |
| 970 | `autoscan 2` | `PASS` | 0.031 | status output indicates OK |
| 971 | `autoscan 3` | `PASS` | 0.016 | status output indicates OK |
| 972 | `autoscan 4` | `PASS` | 0.016 | status output indicates OK |
| 973 | `single 0` | `PASS` | 0.015 | status output indicates OK |
| 974 | `deglitch 1` | `PASS` | 0.031 | status output indicates OK |
| 975 | `deglitch 3` | `PASS` | 0.016 | status output indicates OK |
| 976 | `deglitch 10` | `PASS` | 0.016 | status output indicates OK |
| 977 | `deglitch 33` | `PASS` | 0.031 | status output indicates OK |
| 978 | `errcfg 0x0000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 979 | `errcfg 0x00F9` | `PASS` | 0.015 | informational command responded without failure pattern |
| 980 | `errcfg` | `PASS` | 0.031 | informational command responded without failure pattern |
| 981 | `intb 0` | `PASS` | 0.016 | informational command responded without failure pattern |
| 982 | `intb 1` | `PASS` | 0.016 | informational command responded without failure pattern |
| 983 | `intb 0` | `PASS` | 0.031 | informational command responded without failure pattern |
| 984 | `refclk ext` | `PASS` | 0.016 | status output indicates OK |
| 985 | `refclk int` | `PASS` | 0.015 | status output indicates OK |
| 986 | `activate low` | `PASS` | 0.031 | status output indicates OK |
| 987 | `activate full` | `PASS` | 0.016 | status output indicates OK |
| 988 | `rpoverride 0` | `PASS` | 0.016 | status output indicates OK |
| 989 | `rpoverride 1` | `PASS` | 0.031 | status output indicates OK |
| 990 | `autoamp 1` | `PASS` | 0.016 | status output indicates OK |
| 991 | `autoamp 0` | `PASS` | 0.015 | status output indicates OK |
| 992 | `highcurrent 1` | `PASS` | 0.031 | status output indicates OK |
| 993 | `highcurrent 0` | `PASS` | 0.016 | status output indicates OK |
| 994 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 995 | `wreg 0x19 0x00F9` | `PASS` | 0.016 | status output indicates OK |
| 996 | `sync` | `PASS` | 0.015 | informational command responded without failure pattern |
| 997 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 998 | `rawwreg 0x19 0x00F9` | `PASS` | 0.016 | status output indicates OK |
| 999 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 1000 | `resetreapply` | `PASS` | 0.015 | status output indicates OK |
| 1001 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 1002 | `recover` | `PASS` | 0.016 | status output indicates OK |
| 1003 | `timing 0 43000000` | `PASS` | 0.015 | informational command responded without failure pattern |
| 1004 | `timing 1 43000000` | `PASS` | 0.032 | informational command responded without failure pattern |
| 1005 | `timing 2 43000000` | `PASS` | 0.015 | informational command responded without failure pattern |
| 1006 | `timing 3 43000000` | `PASS` | 0.016 | informational command responded without failure pattern |
| 1007 | `reset` | `PASS` | 0.031 | status output indicates OK |
| 1008 | `init` | `PASS` | 0.016 | informational command responded without failure pattern |
| 1009 | `cfg` | `PASS` | 0.047 | informational command responded without failure pattern |
| 1010 | `sleep` | `PASS` | 0.015 | status output indicates OK |

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

[36m=== LDC1614 CLI Help ===[0m

[32m[Common][0m
  [36mhelp / ?                        [0m - Show this help
  [36mversion / ver                   [0m - Print firmware and library version info
  [36mscan                            [0m - Scan I2C bus

[32m[Data][0m
  [36mread                            [0m - Read configured channels
  [36mread <ch>                       [0m - Read specific channel (0-3)
  [36mread <ch> [N]                   [0m - Read channel N times
  [36mreadfresh [count]               [0m - Read STATUS-driven fresh channel data
  [36mreadstaged <mask> [polls] [instr][0m - Poll-budgeted DATAx read
  [36msamplerate <ch> <N> [timeoutMs] [0m - DRDY-gated sample-rate smoke
  [36mreadblocking                    [0m - Blocking read configured channels (waits for DRDY)
  [36mreadblocking <ch>               [0m - Blocking read specific channel
  [36msample <ch>                     [0m - Get last cached sample (no I2C)
  [36msampleage <ch>                  [0m - Show age of cached sample (ms)
  [36mdrdy                            [0m - Check data ready
  [36mstatus                          [0m - Read and parse STATUS register
  [36mstatus_raw                      [0m - Read raw STATUS register value
  [36mfreq <ch> <fRef>                [0m - Read channel and calc sensor frequency
  [36mtiming <ch> <fRef>              [0m - Calc conversion, settling, and sample time

[32m[Control][0m
  [36minit / begin                    [0m - Initialize/reinitialize device
  [36mend                             [0m - Shut down driver (returns to UNINIT)
  [36msleep                           [0m - Enter sleep mode (stop conversions)
  [36mwake                            [0m - Wake and start conversions
  [36mreset                           [0m - Software reset (returns to UNINIT)
  [36mresetreapply                    [0m - Soft reset + re-apply config (stays READY)

[32m[Configuration][0m
  [36mcfg / config / settings         [0m - Print active configuration snapshot
  [36msnapshot                        [0m - Print settings snapshot struct (no I2C)
  [36mchannels                        [0m - Show configured channel count
  [36mactivech                        [0m - Show current active channel
  [36mactivech <ch>                   [0m - Set active channel (single-ch mode)
  [36msingle <ch>                     [0m - Set single-channel mode and active channel
  [36mautoscan <2|3|4>                [0m - Set auto-scan sequence length
  [36mdeglitch <1|3|10|33>            [0m - Set input deglitch bandwidth in MHz
  [36merrcfg [mask]                   [0m - Show or set ERROR_CONFIG bit mask
  [36mintb [0|1]                      [0m - Show or enable/disable INTB output
  [36mrefclk <int|ext>                [0m - Set reference clock source
  [36mactivate <full|low>             [0m - Set sensor activation current policy
  [36mrpoverride <0|1>                [0m - Enable/disable fixed RP override drive
  [36mautoamp <0|1>                   [0m - Enable/disable auto amplitude correction
  [36mhighcurrent <0|1>               [0m - Enable/disable high-current Ch0 drive
  [36mrcount <ch> <val>               [0m - Set RCOUNT for channel
  [36msettle <ch> <val>               [0m - Set SETTLECOUNT for channel
  [36mclkdiv <ch> <fin> <fref>        [0m - Set clock dividers
  [36moffset <ch> <val>               [0m - Set conversion offset
  [36midrive <ch> <val>               [0m - Set drive current (0-31)
  [36minitidrive <ch>                 [0m - Read auto-calibrated INIT_IDRIVE

[32m[Registers][0m
  [36mreg <addr>                      [0m - Read register (hex address)
  [36mwreg <addr> <val>               [0m - Write register (diagnostic only; may desync cached config)
  [36mrawreg <reg> [addr]             [0m - Raw register read before begin
  [36mrawwreg <reg> <val> [addr]      [0m - Raw register write before begin

[32m[Diagnostics][0m
  [36mdrv                             [0m - Show driver state and health
  [36mstate                           [0m - Compact driver health summary
  [36monline                          [0m - Check if device is online
  [36mid                              [0m - Read MANUFACTURER_ID and DEVICE_ID
  [36mbus                             [0m - I2C scan plus raw LDC identity checks
  [36mprobeaddr <addr>                [0m - Raw LDC identity check at address
  [36mprobe                           [0m - Probe device (no health tracking)
  [36mrecover                         [0m - Manual recovery attempt
  [36msync                            [0m - Re-apply cached config and clear dirty state
  [36mverbose [0|1]                   [0m - Enable/disable verbose output
  [36mstress [N]                      [0m - Run N read cycles (default 10)
  [36mstress_mix [N]                  [0m - Run N mixed-operation stress cycles
  [36mdemo [N]                        [0m - Run N sample demo workflow (default 5)
  [36mselftest                        [0m - Run safe command self-test report
> 
### command 2: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 3: init
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
### command 4: scan
[36m[I][0m Scanning I2C bus (timeout=50ms)...
[36m[I][0m      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
00:                         -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- [32m2A[0m -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- 3C -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- --                         
[36m[I][0m Scan complete. Found 2 device(s).
[36m[I][0m Common addresses: 0x2A/0x2B=LDC1614, 0x48-0x4B=ADS1115, 0x51=RV3032, 0x76/0x77=BME280
[36m[I][0m LDC addresses are verified with MANUFACTURER_ID/DEVICE_ID reads.
> 
### command 5: probeaddr 0x2A
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 6: probe
[36m[I][0m Probing device (no health tracking)...
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  (no health changes)
> 
### command 7: id
=== Device Identity ===
  MANUFACTURER_ID: 0x5449 expected=0x5449 match=[32mYES[0m
  DEVICE_ID:       0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 8: drv
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [32m2[0m
  Total failures: [32m0[0m
  Success rate: [32m100.0%[0m
  Last OK: [32m19 ms ago (at 47057 ms)[0m
  Last error: [32mnever[0m
> 
### command 9: state
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m2[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 10: online
  Online: [32myes[0m
> 
### command 11: cfg
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
### command 12: snapshot
=== Settings Snapshot (no I2C) ===
  State: [32mREADY[0m
  Sleeping: yes
  Measuring: no
  hardwareConfigDirty=0
  Channels: 4  Active: 0  AutoScan: no
  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT  INTB: no
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  Ch0: rcount=0x04D6 settle=0x000A fin=1 fref=1 offset=0x0000 idrive=10 sample_age=0 ms
  Ch1: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch2: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch3: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
> 
### command 13: channels
  Channel count: 4
> 
### command 14: activech
  Active channel: 0
> 
### command 15: status
  STATUS raw=0x0008 drdy=[33mno[0m errCh=0
  Errors: [32mUR=0 OR=0 WD=0 AH=0 AL=0 ZC=0[0m
  Unread: ch0=1 ch1=0 ch2=0 ch3=0
> 
### command 16: status_raw
  STATUS raw = 0x0008
> 
### command 17: rawreg 0x7E
  Raw 0x2A[0x7E] = 0x5449 (21577)
> 
### command 18: rawreg 0x7F
  Raw 0x2A[0x7F] = 0x3055 (12373)
> 
### command 19: reg 0x7E
  Reg 0x7E = 0x5449 (21577)
> 
### command 20: reg 0x7F
  Reg 0x7F = 0x3055 (12373)
> 
### command 21: reg 0x19
  Reg 0x19 = 0x00F9 (249)
> 
### command 22: reg 0x1A
  Reg 0x1A = 0x3401 (13313)
> 
### command 23: reg 0x1B
  Reg 0x1B = 0x020D (525)
> 
### command 24: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 25: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 26: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 27: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 28: rcount 0 0x0123
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 29: settle 0 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 30: clkdiv 0 2 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 31: offset 0 0x0010
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 32: idrive 0 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 33: rcount 1 0x0124
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 34: settle 1 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 35: clkdiv 1 2 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 36: offset 1 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 37: idrive 1 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 38: rcount 2 0x0125
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 39: settle 2 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 40: clkdiv 2 2 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 41: offset 2 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 42: idrive 2 7
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 43: rcount 3 0x0126
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 44: settle 3 0x0014
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 45: clkdiv 3 2 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 46: offset 3 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 47: idrive 3 8
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 48: initidrive 0
  Ch0 INIT_IDRIVE = 0
> 
### command 49: initidrive 1
  Ch1 INIT_IDRIVE = 0
> 
### command 50: initidrive 2
  Ch2 INIT_IDRIVE = 0
> 
### command 51: initidrive 3
  Ch3 INIT_IDRIVE = 0
> 
### command 52: cfg
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
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 53: activech 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 54: activech 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 55: activech 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 56: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 57: single 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 58: single 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 59: single 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 60: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 61: autoscan 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 62: autoscan 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 63: autoscan 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 64: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 65: deglitch 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 66: deglitch 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 67: deglitch 10
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 68: deglitch 33
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 69: errcfg 0x0000
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 70: errcfg 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 71: errcfg
  Cached ERROR_CONFIG: 0x00F9
  Live ERROR_CONFIG:   0x00F9
> 
### command 72: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 73: intb 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 74: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 75: refclk ext
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 76: refclk int
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 77: activate low
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 78: activate full
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 79: rpoverride 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 80: rpoverride 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 81: autoamp 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 82: autoamp 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 83: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 84: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 85: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 86: wreg 0x19 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 87: sync
[36m[I][0m Re-applying cached configuration...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 145 -> [32m169 (+24)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m169[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 88: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 89: rawwreg 0x19 0x00F9
[33m[W][0m Raw register write may desync cached driver config.
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 90: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 91: resetreapply
[36m[I][0m Soft reset + re-apply config...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device back in READY/sleep. Use 'wake' to start conversions.
> 
### command 92: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 93: recover
[36m[I][0m Attempting recovery...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 265 -> [32m267 (+2)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m267[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 94: timing 0 43000000
  Ch0: conversion time = 325.12 us (0.325 ms)
  Ch0: settling time   = 18.98 us (0.019 ms)
  Ch0: sample time     = 344.09 us (0.344 ms)
> 
### command 95: timing 1 43000000
  Ch1: conversion time = 434.98 us (0.435 ms)
  Ch1: settling time   = 26.79 us (0.027 ms)
  Ch1: sample time     = 461.77 us (0.462 ms)
> 
### command 96: timing 2 43000000
  Ch2: conversion time = 545.58 us (0.546 ms)
  Ch2: settling time   = 35.35 us (0.035 ms)
  Ch2: sample time     = 580.93 us (0.581 ms)
> 
### command 97: timing 3 43000000
  Ch3: conversion time = 656.93 us (0.657 ms)
  Ch3: settling time   = 44.65 us (0.045 ms)
  Ch3: sample time     = 701.58 us (0.702 ms)
> 
### command 98: reset
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 99: init
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
### command 100: cfg
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
### command 101: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 102: help

[36m=== LDC1614 CLI Help ===[0m

[32m[Common][0m
  [36mhelp / ?                        [0m - Show this help
  [36mversion / ver                   [0m - Print firmware and library version info
  [36mscan                            [0m - Scan I2C bus

[32m[Data][0m
  [36mread                            [0m - Read configured channels
  [36mread <ch>                       [0m - Read specific channel (0-3)
  [36mread <ch> [N]                   [0m - Read channel N times
  [36mreadfresh [count]               [0m - Read STATUS-driven fresh channel data
  [36mreadstaged <mask> [polls] [instr][0m - Poll-budgeted DATAx read
  [36msamplerate <ch> <N> [timeoutMs] [0m - DRDY-gated sample-rate smoke
  [36mreadblocking                    [0m - Blocking read configured channels (waits for DRDY)
  [36mreadblocking <ch>               [0m - Blocking read specific channel
  [36msample <ch>                     [0m - Get last cached sample (no I2C)
  [36msampleage <ch>                  [0m - Show age of cached sample (ms)
  [36mdrdy                            [0m - Check data ready
  [36mstatus                          [0m - Read and parse STATUS register
  [36mstatus_raw                      [0m - Read raw STATUS register value
  [36mfreq <ch> <fRef>                [0m - Read channel and calc sensor frequency
  [36mtiming <ch> <fRef>              [0m - Calc conversion, settling, and sample time

[32m[Control][0m
  [36minit / begin                    [0m - Initialize/reinitialize device
  [36mend                             [0m - Shut down driver (returns to UNINIT)
  [36msleep                           [0m - Enter sleep mode (stop conversions)
  [36mwake                            [0m - Wake and start conversions
  [36mreset                           [0m - Software reset (returns to UNINIT)
  [36mresetreapply                    [0m - Soft reset + re-apply config (stays READY)

[32m[Configuration][0m
  [36mcfg / config / settings         [0m - Print active configuration snapshot
  [36msnapshot                        [0m - Print settings snapshot struct (no I2C)
  [36mchannels                        [0m - Show configured channel count
  [36mactivech                        [0m - Show current active channel
  [36mactivech <ch>                   [0m - Set active channel (single-ch mode)
  [36msingle <ch>                     [0m - Set single-channel mode and active channel
  [36mautoscan <2|3|4>                [0m - Set auto-scan sequence length
  [36mdeglitch <1|3|10|33>            [0m - Set input deglitch bandwidth in MHz
  [36merrcfg [mask]                   [0m - Show or set ERROR_CONFIG bit mask
  [36mintb [0|1]                      [0m - Show or enable/disable INTB output
  [36mrefclk <int|ext>                [0m - Set reference clock source
  [36mactivate <full|low>             [0m - Set sensor activation current policy
  [36mrpoverride <0|1>                [0m - Enable/disable fixed RP override drive
  [36mautoamp <0|1>                   [0m - Enable/disable auto amplitude correction
  [36mhighcurrent <0|1>               [0m - Enable/disable high-current Ch0 drive
  [36mrcount <ch> <val>               [0m - Set RCOUNT for channel
  [36msettle <ch> <val>               [0m - Set SETTLECOUNT for channel
  [36mclkdiv <ch> <fin> <fref>        [0m - Set clock dividers
  [36moffset <ch> <val>               [0m - Set conversion offset
  [36midrive <ch> <val>               [0m - Set drive current (0-31)
  [36minitidrive <ch>                 [0m - Read auto-calibrated INIT_IDRIVE

[32m[Registers][0m
  [36mreg <addr>                      [0m - Read register (hex address)
  [36mwreg <addr> <val>               [0m - Write register (diagnostic only; may desync cached config)
  [36mrawreg <reg> [addr]             [0m - Raw register read before begin
  [36mrawwreg <reg> <val> [addr]      [0m - Raw register write before begin

[32m[Diagnostics][0m
  [36mdrv                             [0m - Show driver state and health
  [36mstate                           [0m - Compact driver health summary
  [36monline                          [0m - Check if device is online
  [36mid                              [0m - Read MANUFACTURER_ID and DEVICE_ID
  [36mbus                             [0m - I2C scan plus raw LDC identity checks
  [36mprobeaddr <addr>                [0m - Raw LDC identity check at address
  [36mprobe                           [0m - Probe device (no health tracking)
  [36mrecover                         [0m - Manual recovery attempt
  [36msync                            [0m - Re-apply cached config and clear dirty state
  [36mverbose [0|1]                   [0m - Enable/disable verbose output
  [36mstress [N]                      [0m - Run N read cycles (default 10)
  [36mstress_mix [N]                  [0m - Run N mixed-operation stress cycles
  [36mdemo [N]                        [0m - Run N sample demo workflow (default 5)
  [36mselftest                        [0m - Run safe command self-test report
> 
### command 103: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 104: init
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
### command 105: scan
[36m[I][0m Scanning I2C bus (timeout=50ms)...
[36m[I][0m      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
00:                         -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- [32m2A[0m -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- 3C -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- --                         
[36m[I][0m Scan complete. Found 2 device(s).
[36m[I][0m Common addresses: 0x2A/0x2B=LDC1614, 0x48-0x4B=ADS1115, 0x51=RV3032, 0x76/0x77=BME280
[36m[I][0m LDC addresses are verified with MANUFACTURER_ID/DEVICE_ID reads.
> 
### command 106: probeaddr 0x2A
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 107: probe
[36m[I][0m Probing device (no health tracking)...
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  (no health changes)
> 
### command 108: id
=== Device Identity ===
  MANUFACTURER_ID: 0x5449 expected=0x5449 match=[32mYES[0m
  DEVICE_ID:       0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 109: drv
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [32m2[0m
  Total failures: [32m0[0m
  Success rate: [32m100.0%[0m
  Last OK: [32m19 ms ago (at 49427 ms)[0m
  Last error: [32mnever[0m
> 
### command 110: state
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m2[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 111: online
  Online: [32myes[0m
> 
### command 112: cfg
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
### command 113: snapshot
=== Settings Snapshot (no I2C) ===
  State: [32mREADY[0m
  Sleeping: yes
  Measuring: no
  hardwareConfigDirty=0
  Channels: 4  Active: 0  AutoScan: no
  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT  INTB: no
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  Ch0: rcount=0x04D6 settle=0x000A fin=1 fref=1 offset=0x0000 idrive=10 sample_age=0 ms
  Ch1: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch2: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch3: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
> 
### command 114: channels
  Channel count: 4
> 
### command 115: activech
  Active channel: 0
> 
### command 116: status
  STATUS raw=0x0000 drdy=[33mno[0m errCh=0
  Errors: [32mUR=0 OR=0 WD=0 AH=0 AL=0 ZC=0[0m
  Unread: ch0=0 ch1=0 ch2=0 ch3=0
> 
### command 117: status_raw
  STATUS raw = 0x0000
> 
### command 118: rawreg 0x7E
  Raw 0x2A[0x7E] = 0x5449 (21577)
> 
### command 119: rawreg 0x7F
  Raw 0x2A[0x7F] = 0x3055 (12373)
> 
### command 120: reg 0x7E
  Reg 0x7E = 0x5449 (21577)
> 
### command 121: reg 0x7F
  Reg 0x7F = 0x3055 (12373)
> 
### command 122: reg 0x19
  Reg 0x19 = 0x00F9 (249)
> 
### command 123: reg 0x1A
  Reg 0x1A = 0x3401 (13313)
> 
### command 124: reg 0x1B
  Reg 0x1B = 0x020D (525)
> 
### command 125: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 126: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 127: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 128: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 129: rcount 0 0x0123
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 130: settle 0 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 131: clkdiv 0 2 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 132: offset 0 0x0010
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 133: idrive 0 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 134: rcount 1 0x0124
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 135: settle 1 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 136: clkdiv 1 2 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 137: offset 1 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 138: idrive 1 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 139: rcount 2 0x0125
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 140: settle 2 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 141: clkdiv 2 2 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 142: offset 2 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 143: idrive 2 7
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 144: rcount 3 0x0126
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 145: settle 3 0x0014
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 146: clkdiv 3 2 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 147: offset 3 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 148: idrive 3 8
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 149: initidrive 0
  Ch0 INIT_IDRIVE = 0
> 
### command 150: initidrive 1
  Ch1 INIT_IDRIVE = 0
> 
### command 151: initidrive 2
  Ch2 INIT_IDRIVE = 0
> 
### command 152: initidrive 3
  Ch3 INIT_IDRIVE = 0
> 
### command 153: cfg
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
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 154: activech 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 155: activech 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 156: activech 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 157: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 158: single 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 159: single 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 160: single 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 161: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 162: autoscan 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 163: autoscan 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 164: autoscan 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 165: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 166: deglitch 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 167: deglitch 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 168: deglitch 10
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 169: deglitch 33
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 170: errcfg 0x0000
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 171: errcfg 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 172: errcfg
  Cached ERROR_CONFIG: 0x00F9
  Live ERROR_CONFIG:   0x00F9
> 
### command 173: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 174: intb 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 175: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 176: refclk ext
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 177: refclk int
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 178: activate low
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 179: activate full
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 180: rpoverride 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 181: rpoverride 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 182: autoamp 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 183: autoamp 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 184: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 185: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 186: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 187: wreg 0x19 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 188: sync
[36m[I][0m Re-applying cached configuration...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 145 -> [32m169 (+24)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m169[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 189: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 190: rawwreg 0x19 0x00F9
[33m[W][0m Raw register write may desync cached driver config.
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 191: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 192: resetreapply
[36m[I][0m Soft reset + re-apply config...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device back in READY/sleep. Use 'wake' to start conversions.
> 
### command 193: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 194: recover
[36m[I][0m Attempting recovery...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 265 -> [32m267 (+2)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m267[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 195: timing 0 43000000
  Ch0: conversion time = 325.12 us (0.325 ms)
  Ch0: settling time   = 18.98 us (0.019 ms)
  Ch0: sample time     = 344.09 us (0.344 ms)
> 
### command 196: timing 1 43000000
  Ch1: conversion time = 434.98 us (0.435 ms)
  Ch1: settling time   = 26.79 us (0.027 ms)
  Ch1: sample time     = 461.77 us (0.462 ms)
> 
### command 197: timing 2 43000000
  Ch2: conversion time = 545.58 us (0.546 ms)
  Ch2: settling time   = 35.35 us (0.035 ms)
  Ch2: sample time     = 580.93 us (0.581 ms)
> 
### command 198: timing 3 43000000
  Ch3: conversion time = 656.93 us (0.657 ms)
  Ch3: settling time   = 44.65 us (0.045 ms)
  Ch3: sample time     = 701.58 us (0.702 ms)
> 
### command 199: reset
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 200: init
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
### command 201: cfg
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
### command 202: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 203: help

[36m=== LDC1614 CLI Help ===[0m

[32m[Common][0m
  [36mhelp / ?                        [0m - Show this help
  [36mversion / ver                   [0m - Print firmware and library version info
  [36mscan                            [0m - Scan I2C bus

[32m[Data][0m
  [36mread                            [0m - Read configured channels
  [36mread <ch>                       [0m - Read specific channel (0-3)
  [36mread <ch> [N]                   [0m - Read channel N times
  [36mreadfresh [count]               [0m - Read STATUS-driven fresh channel data
  [36mreadstaged <mask> [polls] [instr][0m - Poll-budgeted DATAx read
  [36msamplerate <ch> <N> [timeoutMs] [0m - DRDY-gated sample-rate smoke
  [36mreadblocking                    [0m - Blocking read configured channels (waits for DRDY)
  [36mreadblocking <ch>               [0m - Blocking read specific channel
  [36msample <ch>                     [0m - Get last cached sample (no I2C)
  [36msampleage <ch>                  [0m - Show age of cached sample (ms)
  [36mdrdy                            [0m - Check data ready
  [36mstatus                          [0m - Read and parse STATUS register
  [36mstatus_raw                      [0m - Read raw STATUS register value
  [36mfreq <ch> <fRef>                [0m - Read channel and calc sensor frequency
  [36mtiming <ch> <fRef>              [0m - Calc conversion, settling, and sample time

[32m[Control][0m
  [36minit / begin                    [0m - Initialize/reinitialize device
  [36mend                             [0m - Shut down driver (returns to UNINIT)
  [36msleep                           [0m - Enter sleep mode (stop conversions)
  [36mwake                            [0m - Wake and start conversions
  [36mreset                           [0m - Software reset (returns to UNINIT)
  [36mresetreapply                    [0m - Soft reset + re-apply config (stays READY)

[32m[Configuration][0m
  [36mcfg / config / settings         [0m - Print active configuration snapshot
  [36msnapshot                        [0m - Print settings snapshot struct (no I2C)
  [36mchannels                        [0m - Show configured channel count
  [36mactivech                        [0m - Show current active channel
  [36mactivech <ch>                   [0m - Set active channel (single-ch mode)
  [36msingle <ch>                     [0m - Set single-channel mode and active channel
  [36mautoscan <2|3|4>                [0m - Set auto-scan sequence length
  [36mdeglitch <1|3|10|33>            [0m - Set input deglitch bandwidth in MHz
  [36merrcfg [mask]                   [0m - Show or set ERROR_CONFIG bit mask
  [36mintb [0|1]                      [0m - Show or enable/disable INTB output
  [36mrefclk <int|ext>                [0m - Set reference clock source
  [36mactivate <full|low>             [0m - Set sensor activation current policy
  [36mrpoverride <0|1>                [0m - Enable/disable fixed RP override drive
  [36mautoamp <0|1>                   [0m - Enable/disable auto amplitude correction
  [36mhighcurrent <0|1>               [0m - Enable/disable high-current Ch0 drive
  [36mrcount <ch> <val>               [0m - Set RCOUNT for channel
  [36msettle <ch> <val>               [0m - Set SETTLECOUNT for channel
  [36mclkdiv <ch> <fin> <fref>        [0m - Set clock dividers
  [36moffset <ch> <val>               [0m - Set conversion offset
  [36midrive <ch> <val>               [0m - Set drive current (0-31)
  [36minitidrive <ch>                 [0m - Read auto-calibrated INIT_IDRIVE

[32m[Registers][0m
  [36mreg <addr>                      [0m - Read register (hex address)
  [36mwreg <addr> <val>               [0m - Write register (diagnostic only; may desync cached config)
  [36mrawreg <reg> [addr]             [0m - Raw register read before begin
  [36mrawwreg <reg> <val> [addr]      [0m - Raw register write before begin

[32m[Diagnostics][0m
  [36mdrv                             [0m - Show driver state and health
  [36mstate                           [0m - Compact driver health summary
  [36monline                          [0m - Check if device is online
  [36mid                              [0m - Read MANUFACTURER_ID and DEVICE_ID
  [36mbus                             [0m - I2C scan plus raw LDC identity checks
  [36mprobeaddr <addr>                [0m - Raw LDC identity check at address
  [36mprobe                           [0m - Probe device (no health tracking)
  [36mrecover                         [0m - Manual recovery attempt
  [36msync                            [0m - Re-apply cached config and clear dirty state
  [36mverbose [0|1]                   [0m - Enable/disable verbose output
  [36mstress [N]                      [0m - Run N read cycles (default 10)
  [36mstress_mix [N]                  [0m - Run N mixed-operation stress cycles
  [36mdemo [N]                        [0m - Run N sample demo workflow (default 5)
  [36mselftest                        [0m - Run safe command self-test report
> 
### command 204: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 205: init
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
### command 206: scan
[36m[I][0m Scanning I2C bus (timeout=50ms)...
[36m[I][0m      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
00:                         -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- [32m2A[0m -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- 3C -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- --                         
[36m[I][0m Scan complete. Found 2 device(s).
[36m[I][0m Common addresses: 0x2A/0x2B=LDC1614, 0x48-0x4B=ADS1115, 0x51=RV3032, 0x76/0x77=BME280
[36m[I][0m LDC addresses are verified with MANUFACTURER_ID/DEVICE_ID reads.
> 
### command 207: probeaddr 0x2A
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 208: probe
[36m[I][0m Probing device (no health tracking)...
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  (no health changes)
> 
### command 209: id
=== Device Identity ===
  MANUFACTURER_ID: 0x5449 expected=0x5449 match=[32mYES[0m
  DEVICE_ID:       0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 210: drv
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [32m2[0m
  Total failures: [32m0[0m
  Success rate: [32m100.0%[0m
  Last OK: [32m20 ms ago (at 51790 ms)[0m
  Last error: [32mnever[0m
> 
### command 211: state
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m2[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 212: online
  Online: [32myes[0m
> 
### command 213: cfg
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
### command 214: snapshot
=== Settings Snapshot (no I2C) ===
  State: [32mREADY[0m
  Sleeping: yes
  Measuring: no
  hardwareConfigDirty=0
  Channels: 4  Active: 0  AutoScan: no
  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT  INTB: no
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  Ch0: rcount=0x04D6 settle=0x000A fin=1 fref=1 offset=0x0000 idrive=10 sample_age=0 ms
  Ch1: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch2: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch3: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
> 
### command 215: channels
  Channel count: 4
> 
### command 216: activech
  Active channel: 0
> 
### command 217: status
  STATUS raw=0x0000 drdy=[33mno[0m errCh=0
  Errors: [32mUR=0 OR=0 WD=0 AH=0 AL=0 ZC=0[0m
  Unread: ch0=0 ch1=0 ch2=0 ch3=0
> 
### command 218: status_raw
  STATUS raw = 0x0000
> 
### command 219: rawreg 0x7E
  Raw 0x2A[0x7E] = 0x5449 (21577)
> 
### command 220: rawreg 0x7F
  Raw 0x2A[0x7F] = 0x3055 (12373)
> 
### command 221: reg 0x7E
  Reg 0x7E = 0x5449 (21577)
> 
### command 222: reg 0x7F
  Reg 0x7F = 0x3055 (12373)
> 
### command 223: reg 0x19
  Reg 0x19 = 0x00F9 (249)
> 
### command 224: reg 0x1A
  Reg 0x1A = 0x3401 (13313)
> 
### command 225: reg 0x1B
  Reg 0x1B = 0x020D (525)
> 
### command 226: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 227: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 228: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 229: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 230: rcount 0 0x0123
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 231: settle 0 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 232: clkdiv 0 2 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 233: offset 0 0x0010
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 234: idrive 0 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 235: rcount 1 0x0124
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 236: settle 1 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 237: clkdiv 1 2 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 238: offset 1 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 239: idrive 1 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 240: rcount 2 0x0125
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 241: settle 2 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 242: clkdiv 2 2 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 243: offset 2 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 244: idrive 2 7
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 245: rcount 3 0x0126
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 246: settle 3 0x0014
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 247: clkdiv 3 2 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 248: offset 3 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 249: idrive 3 8
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 250: initidrive 0
  Ch0 INIT_IDRIVE = 0
> 
### command 251: initidrive 1
  Ch1 INIT_IDRIVE = 0
> 
### command 252: initidrive 2
  Ch2 INIT_IDRIVE = 0
> 
### command 253: initidrive 3
  Ch3 INIT_IDRIVE = 0
> 
### command 254: cfg
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
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 255: activech 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 256: activech 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 257: activech 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 258: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 259: single 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 260: single 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 261: single 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 262: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 263: autoscan 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 264: autoscan 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 265: autoscan 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 266: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 267: deglitch 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 268: deglitch 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 269: deglitch 10
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 270: deglitch 33
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 271: errcfg 0x0000
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 272: errcfg 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 273: errcfg
  Cached ERROR_CONFIG: 0x00F9
  Live ERROR_CONFIG:   0x00F9
> 
### command 274: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 275: intb 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 276: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 277: refclk ext
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 278: refclk int
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 279: activate low
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 280: activate full
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 281: rpoverride 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 282: rpoverride 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 283: autoamp 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 284: autoamp 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 285: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 286: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 287: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 288: wreg 0x19 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 289: sync
[36m[I][0m Re-applying cached configuration...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 145 -> [32m169 (+24)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m169[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 290: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 291: rawwreg 0x19 0x00F9
[33m[W][0m Raw register write may desync cached driver config.
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 292: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 293: resetreapply
[36m[I][0m Soft reset + re-apply config...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device back in READY/sleep. Use 'wake' to start conversions.
> 
### command 294: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 295: recover
[36m[I][0m Attempting recovery...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 265 -> [32m267 (+2)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m267[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 296: timing 0 43000000
  Ch0: conversion time = 325.12 us (0.325 ms)
  Ch0: settling time   = 18.98 us (0.019 ms)
  Ch0: sample time     = 344.09 us (0.344 ms)
> 
### command 297: timing 1 43000000
  Ch1: conversion time = 434.98 us (0.435 ms)
  Ch1: settling time   = 26.79 us (0.027 ms)
  Ch1: sample time     = 461.77 us (0.462 ms)
> 
### command 298: timing 2 43000000
  Ch2: conversion time = 545.58 us (0.546 ms)
  Ch2: settling time   = 35.35 us (0.035 ms)
  Ch2: sample time     = 580.93 us (0.581 ms)
> 
### command 299: timing 3 43000000
  Ch3: conversion time = 656.93 us (0.657 ms)
  Ch3: settling time   = 44.65 us (0.045 ms)
  Ch3: sample time     = 701.58 us (0.702 ms)
> 
### command 300: reset
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 301: init
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
### command 302: cfg
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
### command 303: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 304: help

[36m=== LDC1614 CLI Help ===[0m

[32m[Common][0m
  [36mhelp / ?                        [0m - Show this help
  [36mversion / ver                   [0m - Print firmware and library version info
  [36mscan                            [0m - Scan I2C bus

[32m[Data][0m
  [36mread                            [0m - Read configured channels
  [36mread <ch>                       [0m - Read specific channel (0-3)
  [36mread <ch> [N]                   [0m - Read channel N times
  [36mreadfresh [count]               [0m - Read STATUS-driven fresh channel data
  [36mreadstaged <mask> [polls] [instr][0m - Poll-budgeted DATAx read
  [36msamplerate <ch> <N> [timeoutMs] [0m - DRDY-gated sample-rate smoke
  [36mreadblocking                    [0m - Blocking read configured channels (waits for DRDY)
  [36mreadblocking <ch>               [0m - Blocking read specific channel
  [36msample <ch>                     [0m - Get last cached sample (no I2C)
  [36msampleage <ch>                  [0m - Show age of cached sample (ms)
  [36mdrdy                            [0m - Check data ready
  [36mstatus                          [0m - Read and parse STATUS register
  [36mstatus_raw                      [0m - Read raw STATUS register value
  [36mfreq <ch> <fRef>                [0m - Read channel and calc sensor frequency
  [36mtiming <ch> <fRef>              [0m - Calc conversion, settling, and sample time

[32m[Control][0m
  [36minit / begin                    [0m - Initialize/reinitialize device
  [36mend                             [0m - Shut down driver (returns to UNINIT)
  [36msleep                           [0m - Enter sleep mode (stop conversions)
  [36mwake                            [0m - Wake and start conversions
  [36mreset                           [0m - Software reset (returns to UNINIT)
  [36mresetreapply                    [0m - Soft reset + re-apply config (stays READY)

[32m[Configuration][0m
  [36mcfg / config / settings         [0m - Print active configuration snapshot
  [36msnapshot                        [0m - Print settings snapshot struct (no I2C)
  [36mchannels                        [0m - Show configured channel count
  [36mactivech                        [0m - Show current active channel
  [36mactivech <ch>                   [0m - Set active channel (single-ch mode)
  [36msingle <ch>                     [0m - Set single-channel mode and active channel
  [36mautoscan <2|3|4>                [0m - Set auto-scan sequence length
  [36mdeglitch <1|3|10|33>            [0m - Set input deglitch bandwidth in MHz
  [36merrcfg [mask]                   [0m - Show or set ERROR_CONFIG bit mask
  [36mintb [0|1]                      [0m - Show or enable/disable INTB output
  [36mrefclk <int|ext>                [0m - Set reference clock source
  [36mactivate <full|low>             [0m - Set sensor activation current policy
  [36mrpoverride <0|1>                [0m - Enable/disable fixed RP override drive
  [36mautoamp <0|1>                   [0m - Enable/disable auto amplitude correction
  [36mhighcurrent <0|1>               [0m - Enable/disable high-current Ch0 drive
  [36mrcount <ch> <val>               [0m - Set RCOUNT for channel
  [36msettle <ch> <val>               [0m - Set SETTLECOUNT for channel
  [36mclkdiv <ch> <fin> <fref>        [0m - Set clock dividers
  [36moffset <ch> <val>               [0m - Set conversion offset
  [36midrive <ch> <val>               [0m - Set drive current (0-31)
  [36minitidrive <ch>                 [0m - Read auto-calibrated INIT_IDRIVE

[32m[Registers][0m
  [36mreg <addr>                      [0m - Read register (hex address)
  [36mwreg <addr> <val>               [0m - Write register (diagnostic only; may desync cached config)
  [36mrawreg <reg> [addr]             [0m - Raw register read before begin
  [36mrawwreg <reg> <val> [addr]      [0m - Raw register write before begin

[32m[Diagnostics][0m
  [36mdrv                             [0m - Show driver state and health
  [36mstate                           [0m - Compact driver health summary
  [36monline                          [0m - Check if device is online
  [36mid                              [0m - Read MANUFACTURER_ID and DEVICE_ID
  [36mbus                             [0m - I2C scan plus raw LDC identity checks
  [36mprobeaddr <addr>                [0m - Raw LDC identity check at address
  [36mprobe                           [0m - Probe device (no health tracking)
  [36mrecover                         [0m - Manual recovery attempt
  [36msync                            [0m - Re-apply cached config and clear dirty state
  [36mverbose [0|1]                   [0m - Enable/disable verbose output
  [36mstress [N]                      [0m - Run N read cycles (default 10)
  [36mstress_mix [N]                  [0m - Run N mixed-operation stress cycles
  [36mdemo [N]                        [0m - Run N sample demo workflow (default 5)
  [36mselftest                        [0m - Run safe command self-test report
> 
### command 305: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 306: init
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
### command 307: scan
[36m[I][0m Scanning I2C bus (timeout=50ms)...
[36m[I][0m      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
00:                         -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- [32m2A[0m -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- 3C -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- --                         
[36m[I][0m Scan complete. Found 2 device(s).
[36m[I][0m Common addresses: 0x2A/0x2B=LDC1614, 0x48-0x4B=ADS1115, 0x51=RV3032, 0x76/0x77=BME280
[36m[I][0m LDC addresses are verified with MANUFACTURER_ID/DEVICE_ID reads.
> 
### command 308: probeaddr 0x2A
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 309: probe
[36m[I][0m Probing device (no health tracking)...
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  (no health changes)
> 
### command 310: id
=== Device Identity ===
  MANUFACTURER_ID: 0x5449 expected=0x5449 match=[32mYES[0m
  DEVICE_ID:       0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 311: drv
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [32m2[0m
  Total failures: [32m0[0m
  Success rate: [32m100.0%[0m
  Last OK: [32m20 ms ago (at 54159 ms)[0m
  Last error: [32mnever[0m
> 
### command 312: state
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m2[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 313: online
  Online: [32myes[0m
> 
### command 314: cfg
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
### command 315: snapshot
=== Settings Snapshot (no I2C) ===
  State: [32mREADY[0m
  Sleeping: yes
  Measuring: no
  hardwareConfigDirty=0
  Channels: 4  Active: 0  AutoScan: no
  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT  INTB: no
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  Ch0: rcount=0x04D6 settle=0x000A fin=1 fref=1 offset=0x0000 idrive=10 sample_age=0 ms
  Ch1: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch2: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch3: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
> 
### command 316: channels
  Channel count: 4
> 
### command 317: activech
  Active channel: 0
> 
### command 318: status
  STATUS raw=0x0000 drdy=[33mno[0m errCh=0
  Errors: [32mUR=0 OR=0 WD=0 AH=0 AL=0 ZC=0[0m
  Unread: ch0=0 ch1=0 ch2=0 ch3=0
> 
### command 319: status_raw
  STATUS raw = 0x0000
> 
### command 320: rawreg 0x7E
  Raw 0x2A[0x7E] = 0x5449 (21577)
> 
### command 321: rawreg 0x7F
  Raw 0x2A[0x7F] = 0x3055 (12373)
> 
### command 322: reg 0x7E
  Reg 0x7E = 0x5449 (21577)
> 
### command 323: reg 0x7F
  Reg 0x7F = 0x3055 (12373)
> 
### command 324: reg 0x19
  Reg 0x19 = 0x00F9 (249)
> 
### command 325: reg 0x1A
  Reg 0x1A = 0x3401 (13313)
> 
### command 326: reg 0x1B
  Reg 0x1B = 0x020D (525)
> 
### command 327: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 328: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 329: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 330: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 331: rcount 0 0x0123
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 332: settle 0 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 333: clkdiv 0 2 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 334: offset 0 0x0010
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 335: idrive 0 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 336: rcount 1 0x0124
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 337: settle 1 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 338: clkdiv 1 2 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 339: offset 1 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 340: idrive 1 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 341: rcount 2 0x0125
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 342: settle 2 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 343: clkdiv 2 2 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 344: offset 2 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 345: idrive 2 7
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 346: rcount 3 0x0126
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 347: settle 3 0x0014
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 348: clkdiv 3 2 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 349: offset 3 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 350: idrive 3 8
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 351: initidrive 0
  Ch0 INIT_IDRIVE = 0
> 
### command 352: initidrive 1
  Ch1 INIT_IDRIVE = 0
> 
### command 353: initidrive 2
  Ch2 INIT_IDRIVE = 0
> 
### command 354: initidrive 3
  Ch3 INIT_IDRIVE = 0
> 
### command 355: cfg
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
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 356: activech 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 357: activech 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 358: activech 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 359: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 360: single 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 361: single 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 362: single 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 363: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 364: autoscan 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 365: autoscan 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 366: autoscan 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 367: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 368: deglitch 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 369: deglitch 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 370: deglitch 10
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 371: deglitch 33
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 372: errcfg 0x0000
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 373: errcfg 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 374: errcfg
  Cached ERROR_CONFIG: 0x00F9
  Live ERROR_CONFIG:   0x00F9
> 
### command 375: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 376: intb 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 377: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 378: refclk ext
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 379: refclk int
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 380: activate low
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 381: activate full
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 382: rpoverride 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 383: rpoverride 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 384: autoamp 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 385: autoamp 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 386: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 387: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 388: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 389: wreg 0x19 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 390: sync
[36m[I][0m Re-applying cached configuration...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 145 -> [32m169 (+24)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m169[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 391: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 392: rawwreg 0x19 0x00F9
[33m[W][0m Raw register write may desync cached driver config.
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 393: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 394: resetreapply
[36m[I][0m Soft reset + re-apply config...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device back in READY/sleep. Use 'wake' to start conversions.
> 
### command 395: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 396: recover
[36m[I][0m Attempting recovery...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 265 -> [32m267 (+2)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m267[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 397: timing 0 43000000
  Ch0: conversion time = 325.12 us (0.325 ms)
  Ch0: settling time   = 18.98 us (0.019 ms)
  Ch0: sample time     = 344.09 us (0.344 ms)
> 
### command 398: timing 1 43000000
  Ch1: conversion time = 434.98 us (0.435 ms)
  Ch1: settling time   = 26.79 us (0.027 ms)
  Ch1: sample time     = 461.77 us (0.462 ms)
> 
### command 399: timing 2 43000000
  Ch2: conversion time = 545.58 us (0.546 ms)
  Ch2: settling time   = 35.35 us (0.035 ms)
  Ch2: sample time     = 580.93 us (0.581 ms)
> 
### command 400: timing 3 43000000
  Ch3: conversion time = 656.93 us (0.657 ms)
  Ch3: settling time   = 44.65 us (0.045 ms)
  Ch3: sample time     = 701.58 us (0.702 ms)
> 
### command 401: reset
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 402: init
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
### command 403: cfg
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
### command 404: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 405: help

[36m=== LDC1614 CLI Help ===[0m

[32m[Common][0m
  [36mhelp / ?                        [0m - Show this help
  [36mversion / ver                   [0m - Print firmware and library version info
  [36mscan                            [0m - Scan I2C bus

[32m[Data][0m
  [36mread                            [0m - Read configured channels
  [36mread <ch>                       [0m - Read specific channel (0-3)
  [36mread <ch> [N]                   [0m - Read channel N times
  [36mreadfresh [count]               [0m - Read STATUS-driven fresh channel data
  [36mreadstaged <mask> [polls] [instr][0m - Poll-budgeted DATAx read
  [36msamplerate <ch> <N> [timeoutMs] [0m - DRDY-gated sample-rate smoke
  [36mreadblocking                    [0m - Blocking read configured channels (waits for DRDY)
  [36mreadblocking <ch>               [0m - Blocking read specific channel
  [36msample <ch>                     [0m - Get last cached sample (no I2C)
  [36msampleage <ch>                  [0m - Show age of cached sample (ms)
  [36mdrdy                            [0m - Check data ready
  [36mstatus                          [0m - Read and parse STATUS register
  [36mstatus_raw                      [0m - Read raw STATUS register value
  [36mfreq <ch> <fRef>                [0m - Read channel and calc sensor frequency
  [36mtiming <ch> <fRef>              [0m - Calc conversion, settling, and sample time

[32m[Control][0m
  [36minit / begin                    [0m - Initialize/reinitialize device
  [36mend                             [0m - Shut down driver (returns to UNINIT)
  [36msleep                           [0m - Enter sleep mode (stop conversions)
  [36mwake                            [0m - Wake and start conversions
  [36mreset                           [0m - Software reset (returns to UNINIT)
  [36mresetreapply                    [0m - Soft reset + re-apply config (stays READY)

[32m[Configuration][0m
  [36mcfg / config / settings         [0m - Print active configuration snapshot
  [36msnapshot                        [0m - Print settings snapshot struct (no I2C)
  [36mchannels                        [0m - Show configured channel count
  [36mactivech                        [0m - Show current active channel
  [36mactivech <ch>                   [0m - Set active channel (single-ch mode)
  [36msingle <ch>                     [0m - Set single-channel mode and active channel
  [36mautoscan <2|3|4>                [0m - Set auto-scan sequence length
  [36mdeglitch <1|3|10|33>            [0m - Set input deglitch bandwidth in MHz
  [36merrcfg [mask]                   [0m - Show or set ERROR_CONFIG bit mask
  [36mintb [0|1]                      [0m - Show or enable/disable INTB output
  [36mrefclk <int|ext>                [0m - Set reference clock source
  [36mactivate <full|low>             [0m - Set sensor activation current policy
  [36mrpoverride <0|1>                [0m - Enable/disable fixed RP override drive
  [36mautoamp <0|1>                   [0m - Enable/disable auto amplitude correction
  [36mhighcurrent <0|1>               [0m - Enable/disable high-current Ch0 drive
  [36mrcount <ch> <val>               [0m - Set RCOUNT for channel
  [36msettle <ch> <val>               [0m - Set SETTLECOUNT for channel
  [36mclkdiv <ch> <fin> <fref>        [0m - Set clock dividers
  [36moffset <ch> <val>               [0m - Set conversion offset
  [36midrive <ch> <val>               [0m - Set drive current (0-31)
  [36minitidrive <ch>                 [0m - Read auto-calibrated INIT_IDRIVE

[32m[Registers][0m
  [36mreg <addr>                      [0m - Read register (hex address)
  [36mwreg <addr> <val>               [0m - Write register (diagnostic only; may desync cached config)
  [36mrawreg <reg> [addr]             [0m - Raw register read before begin
  [36mrawwreg <reg> <val> [addr]      [0m - Raw register write before begin

[32m[Diagnostics][0m
  [36mdrv                             [0m - Show driver state and health
  [36mstate                           [0m - Compact driver health summary
  [36monline                          [0m - Check if device is online
  [36mid                              [0m - Read MANUFACTURER_ID and DEVICE_ID
  [36mbus                             [0m - I2C scan plus raw LDC identity checks
  [36mprobeaddr <addr>                [0m - Raw LDC identity check at address
  [36mprobe                           [0m - Probe device (no health tracking)
  [36mrecover                         [0m - Manual recovery attempt
  [36msync                            [0m - Re-apply cached config and clear dirty state
  [36mverbose [0|1]                   [0m - Enable/disable verbose output
  [36mstress [N]                      [0m - Run N read cycles (default 10)
  [36mstress_mix [N]                  [0m - Run N mixed-operation stress cycles
  [36mdemo [N]                        [0m - Run N sample demo workflow (default 5)
  [36mselftest                        [0m - Run safe command self-test report
> 
### command 406: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 407: init
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
### command 408: scan
[36m[I][0m Scanning I2C bus (timeout=50ms)...
[36m[I][0m      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
00:                         -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- [32m2A[0m -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- 3C -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- --                         
[36m[I][0m Scan complete. Found 2 device(s).
[36m[I][0m Common addresses: 0x2A/0x2B=LDC1614, 0x48-0x4B=ADS1115, 0x51=RV3032, 0x76/0x77=BME280
[36m[I][0m LDC addresses are verified with MANUFACTURER_ID/DEVICE_ID reads.
> 
### command 409: probeaddr 0x2A
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 410: probe
[36m[I][0m Probing device (no health tracking)...
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  (no health changes)
> 
### command 411: id
=== Device Identity ===
  MANUFACTURER_ID: 0x5449 expected=0x5449 match=[32mYES[0m
  DEVICE_ID:       0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 412: drv
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [32m2[0m
  Total failures: [32m0[0m
  Success rate: [32m100.0%[0m
  Last OK: [32m20 ms ago (at 56525 ms)[0m
  Last error: [32mnever[0m
> 
### command 413: state
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m2[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 414: online
  Online: [32myes[0m
> 
### command 415: cfg
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
### command 416: snapshot
=== Settings Snapshot (no I2C) ===
  State: [32mREADY[0m
  Sleeping: yes
  Measuring: no
  hardwareConfigDirty=0
  Channels: 4  Active: 0  AutoScan: no
  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT  INTB: no
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  Ch0: rcount=0x04D6 settle=0x000A fin=1 fref=1 offset=0x0000 idrive=10 sample_age=0 ms
  Ch1: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch2: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch3: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
> 
### command 417: channels
  Channel count: 4
> 
### command 418: activech
  Active channel: 0
> 
### command 419: status
  STATUS raw=0x0000 drdy=[33mno[0m errCh=0
  Errors: [32mUR=0 OR=0 WD=0 AH=0 AL=0 ZC=0[0m
  Unread: ch0=0 ch1=0 ch2=0 ch3=0
> 
### command 420: status_raw
  STATUS raw = 0x0000
> 
### command 421: rawreg 0x7E
  Raw 0x2A[0x7E] = 0x5449 (21577)
> 
### command 422: rawreg 0x7F
  Raw 0x2A[0x7F] = 0x3055 (12373)
> 
### command 423: reg 0x7E
  Reg 0x7E = 0x5449 (21577)
> 
### command 424: reg 0x7F
  Reg 0x7F = 0x3055 (12373)
> 
### command 425: reg 0x19
  Reg 0x19 = 0x00F9 (249)
> 
### command 426: reg 0x1A
  Reg 0x1A = 0x3401 (13313)
> 
### command 427: reg 0x1B
  Reg 0x1B = 0x020D (525)
> 
### command 428: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 429: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 430: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 431: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 432: rcount 0 0x0123
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 433: settle 0 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 434: clkdiv 0 2 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 435: offset 0 0x0010
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 436: idrive 0 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 437: rcount 1 0x0124
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 438: settle 1 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 439: clkdiv 1 2 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 440: offset 1 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 441: idrive 1 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 442: rcount 2 0x0125
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 443: settle 2 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 444: clkdiv 2 2 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 445: offset 2 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 446: idrive 2 7
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 447: rcount 3 0x0126
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 448: settle 3 0x0014
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 449: clkdiv 3 2 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 450: offset 3 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 451: idrive 3 8
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 452: initidrive 0
  Ch0 INIT_IDRIVE = 0
> 
### command 453: initidrive 1
  Ch1 INIT_IDRIVE = 0
> 
### command 454: initidrive 2
  Ch2 INIT_IDRIVE = 0
> 
### command 455: initidrive 3
  Ch3 INIT_IDRIVE = 0
> 
### command 456: cfg
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
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 457: activech 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 458: activech 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 459: activech 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 460: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 461: single 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 462: single 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 463: single 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 464: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 465: autoscan 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 466: autoscan 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 467: autoscan 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 468: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 469: deglitch 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 470: deglitch 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 471: deglitch 10
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 472: deglitch 33
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 473: errcfg 0x0000
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 474: errcfg 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 475: errcfg
  Cached ERROR_CONFIG: 0x00F9
  Live ERROR_CONFIG:   0x00F9
> 
### command 476: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 477: intb 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 478: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 479: refclk ext
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 480: refclk int
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 481: activate low
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 482: activate full
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 483: rpoverride 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 484: rpoverride 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 485: autoamp 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 486: autoamp 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 487: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 488: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 489: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 490: wreg 0x19 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 491: sync
[36m[I][0m Re-applying cached configuration...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 145 -> [32m169 (+24)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m169[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 492: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 493: rawwreg 0x19 0x00F9
[33m[W][0m Raw register write may desync cached driver config.
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 494: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 495: resetreapply
[36m[I][0m Soft reset + re-apply config...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device back in READY/sleep. Use 'wake' to start conversions.
> 
### command 496: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 497: recover
[36m[I][0m Attempting recovery...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 265 -> [32m267 (+2)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m267[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 498: timing 0 43000000
  Ch0: conversion time = 325.12 us (0.325 ms)
  Ch0: settling time   = 18.98 us (0.019 ms)
  Ch0: sample time     = 344.09 us (0.344 ms)
> 
### command 499: timing 1 43000000
  Ch1: conversion time = 434.98 us (0.435 ms)
  Ch1: settling time   = 26.79 us (0.027 ms)
  Ch1: sample time     = 461.77 us (0.462 ms)
> 
### command 500: timing 2 43000000
  Ch2: conversion time = 545.58 us (0.546 ms)
  Ch2: settling time   = 35.35 us (0.035 ms)
  Ch2: sample time     = 580.93 us (0.581 ms)
> 
### command 501: timing 3 43000000
  Ch3: conversion time = 656.93 us (0.657 ms)
  Ch3: settling time   = 44.65 us (0.045 ms)
  Ch3: sample time     = 701.58 us (0.702 ms)
> 
### command 502: reset
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 503: init
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
### command 504: cfg
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
### command 505: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 506: help

[36m=== LDC1614 CLI Help ===[0m

[32m[Common][0m
  [36mhelp / ?                        [0m - Show this help
  [36mversion / ver                   [0m - Print firmware and library version info
  [36mscan                            [0m - Scan I2C bus

[32m[Data][0m
  [36mread                            [0m - Read configured channels
  [36mread <ch>                       [0m - Read specific channel (0-3)
  [36mread <ch> [N]                   [0m - Read channel N times
  [36mreadfresh [count]               [0m - Read STATUS-driven fresh channel data
  [36mreadstaged <mask> [polls] [instr][0m - Poll-budgeted DATAx read
  [36msamplerate <ch> <N> [timeoutMs] [0m - DRDY-gated sample-rate smoke
  [36mreadblocking                    [0m - Blocking read configured channels (waits for DRDY)
  [36mreadblocking <ch>               [0m - Blocking read specific channel
  [36msample <ch>                     [0m - Get last cached sample (no I2C)
  [36msampleage <ch>                  [0m - Show age of cached sample (ms)
  [36mdrdy                            [0m - Check data ready
  [36mstatus                          [0m - Read and parse STATUS register
  [36mstatus_raw                      [0m - Read raw STATUS register value
  [36mfreq <ch> <fRef>                [0m - Read channel and calc sensor frequency
  [36mtiming <ch> <fRef>              [0m - Calc conversion, settling, and sample time

[32m[Control][0m
  [36minit / begin                    [0m - Initialize/reinitialize device
  [36mend                             [0m - Shut down driver (returns to UNINIT)
  [36msleep                           [0m - Enter sleep mode (stop conversions)
  [36mwake                            [0m - Wake and start conversions
  [36mreset                           [0m - Software reset (returns to UNINIT)
  [36mresetreapply                    [0m - Soft reset + re-apply config (stays READY)

[32m[Configuration][0m
  [36mcfg / config / settings         [0m - Print active configuration snapshot
  [36msnapshot                        [0m - Print settings snapshot struct (no I2C)
  [36mchannels                        [0m - Show configured channel count
  [36mactivech                        [0m - Show current active channel
  [36mactivech <ch>                   [0m - Set active channel (single-ch mode)
  [36msingle <ch>                     [0m - Set single-channel mode and active channel
  [36mautoscan <2|3|4>                [0m - Set auto-scan sequence length
  [36mdeglitch <1|3|10|33>            [0m - Set input deglitch bandwidth in MHz
  [36merrcfg [mask]                   [0m - Show or set ERROR_CONFIG bit mask
  [36mintb [0|1]                      [0m - Show or enable/disable INTB output
  [36mrefclk <int|ext>                [0m - Set reference clock source
  [36mactivate <full|low>             [0m - Set sensor activation current policy
  [36mrpoverride <0|1>                [0m - Enable/disable fixed RP override drive
  [36mautoamp <0|1>                   [0m - Enable/disable auto amplitude correction
  [36mhighcurrent <0|1>               [0m - Enable/disable high-current Ch0 drive
  [36mrcount <ch> <val>               [0m - Set RCOUNT for channel
  [36msettle <ch> <val>               [0m - Set SETTLECOUNT for channel
  [36mclkdiv <ch> <fin> <fref>        [0m - Set clock dividers
  [36moffset <ch> <val>               [0m - Set conversion offset
  [36midrive <ch> <val>               [0m - Set drive current (0-31)
  [36minitidrive <ch>                 [0m - Read auto-calibrated INIT_IDRIVE

[32m[Registers][0m
  [36mreg <addr>                      [0m - Read register (hex address)
  [36mwreg <addr> <val>               [0m - Write register (diagnostic only; may desync cached config)
  [36mrawreg <reg> [addr]             [0m - Raw register read before begin
  [36mrawwreg <reg> <val> [addr]      [0m - Raw register write before begin

[32m[Diagnostics][0m
  [36mdrv                             [0m - Show driver state and health
  [36mstate                           [0m - Compact driver health summary
  [36monline                          [0m - Check if device is online
  [36mid                              [0m - Read MANUFACTURER_ID and DEVICE_ID
  [36mbus                             [0m - I2C scan plus raw LDC identity checks
  [36mprobeaddr <addr>                [0m - Raw LDC identity check at address
  [36mprobe                           [0m - Probe device (no health tracking)
  [36mrecover                         [0m - Manual recovery attempt
  [36msync                            [0m - Re-apply cached config and clear dirty state
  [36mverbose [0|1]                   [0m - Enable/disable verbose output
  [36mstress [N]                      [0m - Run N read cycles (default 10)
  [36mstress_mix [N]                  [0m - Run N mixed-operation stress cycles
  [36mdemo [N]                        [0m - Run N sample demo workflow (default 5)
  [36mselftest                        [0m - Run safe command self-test report
> 
### command 507: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 508: init
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
### command 509: scan
[36m[I][0m Scanning I2C bus (timeout=50ms)...
[36m[I][0m      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
00:                         -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- [32m2A[0m -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- 3C -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- --                         
[36m[I][0m Scan complete. Found 2 device(s).
[36m[I][0m Common addresses: 0x2A/0x2B=LDC1614, 0x48-0x4B=ADS1115, 0x51=RV3032, 0x76/0x77=BME280
[36m[I][0m LDC addresses are verified with MANUFACTURER_ID/DEVICE_ID reads.
> 
### command 510: probeaddr 0x2A
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 511: probe
[36m[I][0m Probing device (no health tracking)...
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  (no health changes)
> 
### command 512: id
=== Device Identity ===
  MANUFACTURER_ID: 0x5449 expected=0x5449 match=[32mYES[0m
  DEVICE_ID:       0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 513: drv
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [32m2[0m
  Total failures: [32m0[0m
  Success rate: [32m100.0%[0m
  Last OK: [32m19 ms ago (at 58895 ms)[0m
  Last error: [32mnever[0m
> 
### command 514: state
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m2[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 515: online
  Online: [32myes[0m
> 
### command 516: cfg
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
### command 517: snapshot
=== Settings Snapshot (no I2C) ===
  State: [32mREADY[0m
  Sleeping: yes
  Measuring: no
  hardwareConfigDirty=0
  Channels: 4  Active: 0  AutoScan: no
  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT  INTB: no
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  Ch0: rcount=0x04D6 settle=0x000A fin=1 fref=1 offset=0x0000 idrive=10 sample_age=0 ms
  Ch1: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch2: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch3: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
> 
### command 518: channels
  Channel count: 4
> 
### command 519: activech
  Active channel: 0
> 
### command 520: status
  STATUS raw=0x0000 drdy=[33mno[0m errCh=0
  Errors: [32mUR=0 OR=0 WD=0 AH=0 AL=0 ZC=0[0m
  Unread: ch0=0 ch1=0 ch2=0 ch3=0
> 
### command 521: status_raw
  STATUS raw = 0x0000
> 
### command 522: rawreg 0x7E
  Raw 0x2A[0x7E] = 0x5449 (21577)
> 
### command 523: rawreg 0x7F
  Raw 0x2A[0x7F] = 0x3055 (12373)
> 
### command 524: reg 0x7E
  Reg 0x7E = 0x5449 (21577)
> 
### command 525: reg 0x7F
  Reg 0x7F = 0x3055 (12373)
> 
### command 526: reg 0x19
  Reg 0x19 = 0x00F9 (249)
> 
### command 527: reg 0x1A
  Reg 0x1A = 0x3401 (13313)
> 
### command 528: reg 0x1B
  Reg 0x1B = 0x020D (525)
> 
### command 529: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 530: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 531: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 532: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 533: rcount 0 0x0123
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 534: settle 0 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 535: clkdiv 0 2 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 536: offset 0 0x0010
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 537: idrive 0 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 538: rcount 1 0x0124
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 539: settle 1 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 540: clkdiv 1 2 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 541: offset 1 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 542: idrive 1 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 543: rcount 2 0x0125
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 544: settle 2 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 545: clkdiv 2 2 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 546: offset 2 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 547: idrive 2 7
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 548: rcount 3 0x0126
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 549: settle 3 0x0014
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 550: clkdiv 3 2 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 551: offset 3 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 552: idrive 3 8
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 553: initidrive 0
  Ch0 INIT_IDRIVE = 0
> 
### command 554: initidrive 1
  Ch1 INIT_IDRIVE = 0
> 
### command 555: initidrive 2
  Ch2 INIT_IDRIVE = 0
> 
### command 556: initidrive 3
  Ch3 INIT_IDRIVE = 0
> 
### command 557: cfg
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
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 558: activech 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 559: activech 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 560: activech 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 561: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 562: single 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 563: single 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 564: single 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 565: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 566: autoscan 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 567: autoscan 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 568: autoscan 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 569: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 570: deglitch 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 571: deglitch 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 572: deglitch 10
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 573: deglitch 33
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 574: errcfg 0x0000
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 575: errcfg 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 576: errcfg
  Cached ERROR_CONFIG: 0x00F9
  Live ERROR_CONFIG:   0x00F9
> 
### command 577: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 578: intb 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 579: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 580: refclk ext
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 581: refclk int
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 582: activate low
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 583: activate full
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 584: rpoverride 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 585: rpoverride 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 586: autoamp 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 587: autoamp 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 588: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 589: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 590: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 591: wreg 0x19 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 592: sync
[36m[I][0m Re-applying cached configuration...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 145 -> [32m169 (+24)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m169[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 593: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 594: rawwreg 0x19 0x00F9
[33m[W][0m Raw register write may desync cached driver config.
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 595: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 596: resetreapply
[36m[I][0m Soft reset + re-apply config...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device back in READY/sleep. Use 'wake' to start conversions.
> 
### command 597: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 598: recover
[36m[I][0m Attempting recovery...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 265 -> [32m267 (+2)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m267[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 599: timing 0 43000000
  Ch0: conversion time = 325.12 us (0.325 ms)
  Ch0: settling time   = 18.98 us (0.019 ms)
  Ch0: sample time     = 344.09 us (0.344 ms)
> 
### command 600: timing 1 43000000
  Ch1: conversion time = 434.98 us (0.435 ms)
  Ch1: settling time   = 26.79 us (0.027 ms)
  Ch1: sample time     = 461.77 us (0.462 ms)
> 
### command 601: timing 2 43000000
  Ch2: conversion time = 545.58 us (0.546 ms)
  Ch2: settling time   = 35.35 us (0.035 ms)
  Ch2: sample time     = 580.93 us (0.581 ms)
> 
### command 602: timing 3 43000000
  Ch3: conversion time = 656.93 us (0.657 ms)
  Ch3: settling time   = 44.65 us (0.045 ms)
  Ch3: sample time     = 701.58 us (0.702 ms)
> 
### command 603: reset
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 604: init
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
### command 605: cfg
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
### command 606: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 607: help

[36m=== LDC1614 CLI Help ===[0m

[32m[Common][0m
  [36mhelp / ?                        [0m - Show this help
  [36mversion / ver                   [0m - Print firmware and library version info
  [36mscan                            [0m - Scan I2C bus

[32m[Data][0m
  [36mread                            [0m - Read configured channels
  [36mread <ch>                       [0m - Read specific channel (0-3)
  [36mread <ch> [N]                   [0m - Read channel N times
  [36mreadfresh [count]               [0m - Read STATUS-driven fresh channel data
  [36mreadstaged <mask> [polls] [instr][0m - Poll-budgeted DATAx read
  [36msamplerate <ch> <N> [timeoutMs] [0m - DRDY-gated sample-rate smoke
  [36mreadblocking                    [0m - Blocking read configured channels (waits for DRDY)
  [36mreadblocking <ch>               [0m - Blocking read specific channel
  [36msample <ch>                     [0m - Get last cached sample (no I2C)
  [36msampleage <ch>                  [0m - Show age of cached sample (ms)
  [36mdrdy                            [0m - Check data ready
  [36mstatus                          [0m - Read and parse STATUS register
  [36mstatus_raw                      [0m - Read raw STATUS register value
  [36mfreq <ch> <fRef>                [0m - Read channel and calc sensor frequency
  [36mtiming <ch> <fRef>              [0m - Calc conversion, settling, and sample time

[32m[Control][0m
  [36minit / begin                    [0m - Initialize/reinitialize device
  [36mend                             [0m - Shut down driver (returns to UNINIT)
  [36msleep                           [0m - Enter sleep mode (stop conversions)
  [36mwake                            [0m - Wake and start conversions
  [36mreset                           [0m - Software reset (returns to UNINIT)
  [36mresetreapply                    [0m - Soft reset + re-apply config (stays READY)

[32m[Configuration][0m
  [36mcfg / config / settings         [0m - Print active configuration snapshot
  [36msnapshot                        [0m - Print settings snapshot struct (no I2C)
  [36mchannels                        [0m - Show configured channel count
  [36mactivech                        [0m - Show current active channel
  [36mactivech <ch>                   [0m - Set active channel (single-ch mode)
  [36msingle <ch>                     [0m - Set single-channel mode and active channel
  [36mautoscan <2|3|4>                [0m - Set auto-scan sequence length
  [36mdeglitch <1|3|10|33>            [0m - Set input deglitch bandwidth in MHz
  [36merrcfg [mask]                   [0m - Show or set ERROR_CONFIG bit mask
  [36mintb [0|1]                      [0m - Show or enable/disable INTB output
  [36mrefclk <int|ext>                [0m - Set reference clock source
  [36mactivate <full|low>             [0m - Set sensor activation current policy
  [36mrpoverride <0|1>                [0m - Enable/disable fixed RP override drive
  [36mautoamp <0|1>                   [0m - Enable/disable auto amplitude correction
  [36mhighcurrent <0|1>               [0m - Enable/disable high-current Ch0 drive
  [36mrcount <ch> <val>               [0m - Set RCOUNT for channel
  [36msettle <ch> <val>               [0m - Set SETTLECOUNT for channel
  [36mclkdiv <ch> <fin> <fref>        [0m - Set clock dividers
  [36moffset <ch> <val>               [0m - Set conversion offset
  [36midrive <ch> <val>               [0m - Set drive current (0-31)
  [36minitidrive <ch>                 [0m - Read auto-calibrated INIT_IDRIVE

[32m[Registers][0m
  [36mreg <addr>                      [0m - Read register (hex address)
  [36mwreg <addr> <val>               [0m - Write register (diagnostic only; may desync cached config)
  [36mrawreg <reg> [addr]             [0m - Raw register read before begin
  [36mrawwreg <reg> <val> [addr]      [0m - Raw register write before begin

[32m[Diagnostics][0m
  [36mdrv                             [0m - Show driver state and health
  [36mstate                           [0m - Compact driver health summary
  [36monline                          [0m - Check if device is online
  [36mid                              [0m - Read MANUFACTURER_ID and DEVICE_ID
  [36mbus                             [0m - I2C scan plus raw LDC identity checks
  [36mprobeaddr <addr>                [0m - Raw LDC identity check at address
  [36mprobe                           [0m - Probe device (no health tracking)
  [36mrecover                         [0m - Manual recovery attempt
  [36msync                            [0m - Re-apply cached config and clear dirty state
  [36mverbose [0|1]                   [0m - Enable/disable verbose output
  [36mstress [N]                      [0m - Run N read cycles (default 10)
  [36mstress_mix [N]                  [0m - Run N mixed-operation stress cycles
  [36mdemo [N]                        [0m - Run N sample demo workflow (default 5)
  [36mselftest                        [0m - Run safe command self-test report
> 
### command 608: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 609: init
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
### command 610: scan
[36m[I][0m Scanning I2C bus (timeout=50ms)...
[36m[I][0m      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
00:                         -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- [32m2A[0m -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- 3C -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- --                         
[36m[I][0m Scan complete. Found 2 device(s).
[36m[I][0m Common addresses: 0x2A/0x2B=LDC1614, 0x48-0x4B=ADS1115, 0x51=RV3032, 0x76/0x77=BME280
[36m[I][0m LDC addresses are verified with MANUFACTURER_ID/DEVICE_ID reads.
> 
### command 611: probeaddr 0x2A
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 612: probe
[36m[I][0m Probing device (no health tracking)...
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  (no health changes)
> 
### command 613: id
=== Device Identity ===
  MANUFACTURER_ID: 0x5449 expected=0x5449 match=[32mYES[0m
  DEVICE_ID:       0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 614: drv
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [32m2[0m
  Total failures: [32m0[0m
  Success rate: [32m100.0%[0m
  Last OK: [32m21 ms ago (at 61262 ms)[0m
  Last error: [32mnever[0m
> 
### command 615: state
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m2[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 616: online
  Online: [32myes[0m
> 
### command 617: cfg
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
### command 618: snapshot
=== Settings Snapshot (no I2C) ===
  State: [32mREADY[0m
  Sleeping: yes
  Measuring: no
  hardwareConfigDirty=0
  Channels: 4  Active: 0  AutoScan: no
  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT  INTB: no
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  Ch0: rcount=0x04D6 settle=0x000A fin=1 fref=1 offset=0x0000 idrive=10 sample_age=0 ms
  Ch1: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch2: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch3: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
> 
### command 619: channels
  Channel count: 4
> 
### command 620: activech
  Active channel: 0
> 
### command 621: status
  STATUS raw=0x0000 drdy=[33mno[0m errCh=0
  Errors: [32mUR=0 OR=0 WD=0 AH=0 AL=0 ZC=0[0m
  Unread: ch0=0 ch1=0 ch2=0 ch3=0
> 
### command 622: status_raw
  STATUS raw = 0x0000
> 
### command 623: rawreg 0x7E
  Raw 0x2A[0x7E] = 0x5449 (21577)
> 
### command 624: rawreg 0x7F
  Raw 0x2A[0x7F] = 0x3055 (12373)
> 
### command 625: reg 0x7E
  Reg 0x7E = 0x5449 (21577)
> 
### command 626: reg 0x7F
  Reg 0x7F = 0x3055 (12373)
> 
### command 627: reg 0x19
  Reg 0x19 = 0x00F9 (249)
> 
### command 628: reg 0x1A
  Reg 0x1A = 0x3401 (13313)
> 
### command 629: reg 0x1B
  Reg 0x1B = 0x020D (525)
> 
### command 630: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 631: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 632: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 633: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 634: rcount 0 0x0123
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 635: settle 0 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 636: clkdiv 0 2 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 637: offset 0 0x0010
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 638: idrive 0 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 639: rcount 1 0x0124
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 640: settle 1 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 641: clkdiv 1 2 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 642: offset 1 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 643: idrive 1 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 644: rcount 2 0x0125
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 645: settle 2 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 646: clkdiv 2 2 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 647: offset 2 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 648: idrive 2 7
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 649: rcount 3 0x0126
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 650: settle 3 0x0014
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 651: clkdiv 3 2 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 652: offset 3 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 653: idrive 3 8
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 654: initidrive 0
  Ch0 INIT_IDRIVE = 0
> 
### command 655: initidrive 1
  Ch1 INIT_IDRIVE = 0
> 
### command 656: initidrive 2
  Ch2 INIT_IDRIVE = 0
> 
### command 657: initidrive 3
  Ch3 INIT_IDRIVE = 0
> 
### command 658: cfg
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
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 659: activech 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 660: activech 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 661: activech 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 662: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 663: single 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 664: single 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 665: single 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 666: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 667: autoscan 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 668: autoscan 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 669: autoscan 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 670: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 671: deglitch 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 672: deglitch 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 673: deglitch 10
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 674: deglitch 33
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 675: errcfg 0x0000
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 676: errcfg 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 677: errcfg
  Cached ERROR_CONFIG: 0x00F9
  Live ERROR_CONFIG:   0x00F9
> 
### command 678: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 679: intb 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 680: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 681: refclk ext
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 682: refclk int
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 683: activate low
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 684: activate full
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 685: rpoverride 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 686: rpoverride 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 687: autoamp 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 688: autoamp 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 689: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 690: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 691: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 692: wreg 0x19 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 693: sync
[36m[I][0m Re-applying cached configuration...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 145 -> [32m169 (+24)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m169[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 694: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 695: rawwreg 0x19 0x00F9
[33m[W][0m Raw register write may desync cached driver config.
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 696: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 697: resetreapply
[36m[I][0m Soft reset + re-apply config...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device back in READY/sleep. Use 'wake' to start conversions.
> 
### command 698: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 699: recover
[36m[I][0m Attempting recovery...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 265 -> [32m267 (+2)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m267[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 700: timing 0 43000000
  Ch0: conversion time = 325.12 us (0.325 ms)
  Ch0: settling time   = 18.98 us (0.019 ms)
  Ch0: sample time     = 344.09 us (0.344 ms)
> 
### command 701: timing 1 43000000
  Ch1: conversion time = 434.98 us (0.435 ms)
  Ch1: settling time   = 26.79 us (0.027 ms)
  Ch1: sample time     = 461.77 us (0.462 ms)
> 
### command 702: timing 2 43000000
  Ch2: conversion time = 545.58 us (0.546 ms)
  Ch2: settling time   = 35.35 us (0.035 ms)
  Ch2: sample time     = 580.93 us (0.581 ms)
> 
### command 703: timing 3 43000000
  Ch3: conversion time = 656.93 us (0.657 ms)
  Ch3: settling time   = 44.65 us (0.045 ms)
  Ch3: sample time     = 701.58 us (0.702 ms)
> 
### command 704: reset
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 705: init
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
### command 706: cfg
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
### command 707: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 708: help

[36m=== LDC1614 CLI Help ===[0m

[32m[Common][0m
  [36mhelp / ?                        [0m - Show this help
  [36mversion / ver                   [0m - Print firmware and library version info
  [36mscan                            [0m - Scan I2C bus

[32m[Data][0m
  [36mread                            [0m - Read configured channels
  [36mread <ch>                       [0m - Read specific channel (0-3)
  [36mread <ch> [N]                   [0m - Read channel N times
  [36mreadfresh [count]               [0m - Read STATUS-driven fresh channel data
  [36mreadstaged <mask> [polls] [instr][0m - Poll-budgeted DATAx read
  [36msamplerate <ch> <N> [timeoutMs] [0m - DRDY-gated sample-rate smoke
  [36mreadblocking                    [0m - Blocking read configured channels (waits for DRDY)
  [36mreadblocking <ch>               [0m - Blocking read specific channel
  [36msample <ch>                     [0m - Get last cached sample (no I2C)
  [36msampleage <ch>                  [0m - Show age of cached sample (ms)
  [36mdrdy                            [0m - Check data ready
  [36mstatus                          [0m - Read and parse STATUS register
  [36mstatus_raw                      [0m - Read raw STATUS register value
  [36mfreq <ch> <fRef>                [0m - Read channel and calc sensor frequency
  [36mtiming <ch> <fRef>              [0m - Calc conversion, settling, and sample time

[32m[Control][0m
  [36minit / begin                    [0m - Initialize/reinitialize device
  [36mend                             [0m - Shut down driver (returns to UNINIT)
  [36msleep                           [0m - Enter sleep mode (stop conversions)
  [36mwake                            [0m - Wake and start conversions
  [36mreset                           [0m - Software reset (returns to UNINIT)
  [36mresetreapply                    [0m - Soft reset + re-apply config (stays READY)

[32m[Configuration][0m
  [36mcfg / config / settings         [0m - Print active configuration snapshot
  [36msnapshot                        [0m - Print settings snapshot struct (no I2C)
  [36mchannels                        [0m - Show configured channel count
  [36mactivech                        [0m - Show current active channel
  [36mactivech <ch>                   [0m - Set active channel (single-ch mode)
  [36msingle <ch>                     [0m - Set single-channel mode and active channel
  [36mautoscan <2|3|4>                [0m - Set auto-scan sequence length
  [36mdeglitch <1|3|10|33>            [0m - Set input deglitch bandwidth in MHz
  [36merrcfg [mask]                   [0m - Show or set ERROR_CONFIG bit mask
  [36mintb [0|1]                      [0m - Show or enable/disable INTB output
  [36mrefclk <int|ext>                [0m - Set reference clock source
  [36mactivate <full|low>             [0m - Set sensor activation current policy
  [36mrpoverride <0|1>                [0m - Enable/disable fixed RP override drive
  [36mautoamp <0|1>                   [0m - Enable/disable auto amplitude correction
  [36mhighcurrent <0|1>               [0m - Enable/disable high-current Ch0 drive
  [36mrcount <ch> <val>               [0m - Set RCOUNT for channel
  [36msettle <ch> <val>               [0m - Set SETTLECOUNT for channel
  [36mclkdiv <ch> <fin> <fref>        [0m - Set clock dividers
  [36moffset <ch> <val>               [0m - Set conversion offset
  [36midrive <ch> <val>               [0m - Set drive current (0-31)
  [36minitidrive <ch>                 [0m - Read auto-calibrated INIT_IDRIVE

[32m[Registers][0m
  [36mreg <addr>                      [0m - Read register (hex address)
  [36mwreg <addr> <val>               [0m - Write register (diagnostic only; may desync cached config)
  [36mrawreg <reg> [addr]             [0m - Raw register read before begin
  [36mrawwreg <reg> <val> [addr]      [0m - Raw register write before begin

[32m[Diagnostics][0m
  [36mdrv                             [0m - Show driver state and health
  [36mstate                           [0m - Compact driver health summary
  [36monline                          [0m - Check if device is online
  [36mid                              [0m - Read MANUFACTURER_ID and DEVICE_ID
  [36mbus                             [0m - I2C scan plus raw LDC identity checks
  [36mprobeaddr <addr>                [0m - Raw LDC identity check at address
  [36mprobe                           [0m - Probe device (no health tracking)
  [36mrecover                         [0m - Manual recovery attempt
  [36msync                            [0m - Re-apply cached config and clear dirty state
  [36mverbose [0|1]                   [0m - Enable/disable verbose output
  [36mstress [N]                      [0m - Run N read cycles (default 10)
  [36mstress_mix [N]                  [0m - Run N mixed-operation stress cycles
  [36mdemo [N]                        [0m - Run N sample demo workflow (default 5)
  [36mselftest                        [0m - Run safe command self-test report
> 
### command 709: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 710: init
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
### command 711: scan
[36m[I][0m Scanning I2C bus (timeout=50ms)...
[36m[I][0m      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
00:                         -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- [32m2A[0m -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- 3C -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- --                         
[36m[I][0m Scan complete. Found 2 device(s).
[36m[I][0m Common addresses: 0x2A/0x2B=LDC1614, 0x48-0x4B=ADS1115, 0x51=RV3032, 0x76/0x77=BME280
[36m[I][0m LDC addresses are verified with MANUFACTURER_ID/DEVICE_ID reads.
> 
### command 712: probeaddr 0x2A
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 713: probe
[36m[I][0m Probing device (no health tracking)...
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  (no health changes)
> 
### command 714: id
=== Device Identity ===
  MANUFACTURER_ID: 0x5449 expected=0x5449 match=[32mYES[0m
  DEVICE_ID:       0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 715: drv
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [32m2[0m
  Total failures: [32m0[0m
  Success rate: [32m100.0%[0m
  Last OK: [32m20 ms ago (at 63632 ms)[0m
  Last error: [32mnever[0m
> 
### command 716: state
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m2[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 717: online
  Online: [32myes[0m
> 
### command 718: cfg
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
### command 719: snapshot
=== Settings Snapshot (no I2C) ===
  State: [32mREADY[0m
  Sleeping: yes
  Measuring: no
  hardwareConfigDirty=0
  Channels: 4  Active: 0  AutoScan: no
  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT  INTB: no
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  Ch0: rcount=0x04D6 settle=0x000A fin=1 fref=1 offset=0x0000 idrive=10 sample_age=0 ms
  Ch1: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch2: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch3: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
> 
### command 720: channels
  Channel count: 4
> 
### command 721: activech
  Active channel: 0
> 
### command 722: status
  STATUS raw=0x0000 drdy=[33mno[0m errCh=0
  Errors: [32mUR=0 OR=0 WD=0 AH=0 AL=0 ZC=0[0m
  Unread: ch0=0 ch1=0 ch2=0 ch3=0
> 
### command 723: status_raw
  STATUS raw = 0x0000
> 
### command 724: rawreg 0x7E
  Raw 0x2A[0x7E] = 0x5449 (21577)
> 
### command 725: rawreg 0x7F
  Raw 0x2A[0x7F] = 0x3055 (12373)
> 
### command 726: reg 0x7E
  Reg 0x7E = 0x5449 (21577)
> 
### command 727: reg 0x7F
  Reg 0x7F = 0x3055 (12373)
> 
### command 728: reg 0x19
  Reg 0x19 = 0x00F9 (249)
> 
### command 729: reg 0x1A
  Reg 0x1A = 0x3401 (13313)
> 
### command 730: reg 0x1B
  Reg 0x1B = 0x020D (525)
> 
### command 731: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 732: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 733: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 734: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 735: rcount 0 0x0123
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 736: settle 0 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 737: clkdiv 0 2 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 738: offset 0 0x0010
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 739: idrive 0 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 740: rcount 1 0x0124
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 741: settle 1 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 742: clkdiv 1 2 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 743: offset 1 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 744: idrive 1 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 745: rcount 2 0x0125
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 746: settle 2 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 747: clkdiv 2 2 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 748: offset 2 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 749: idrive 2 7
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 750: rcount 3 0x0126
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 751: settle 3 0x0014
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 752: clkdiv 3 2 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 753: offset 3 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 754: idrive 3 8
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 755: initidrive 0
  Ch0 INIT_IDRIVE = 0
> 
### command 756: initidrive 1
  Ch1 INIT_IDRIVE = 0
> 
### command 757: initidrive 2
  Ch2 INIT_IDRIVE = 0
> 
### command 758: initidrive 3
  Ch3 INIT_IDRIVE = 0
> 
### command 759: cfg
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
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 760: activech 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 761: activech 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 762: activech 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 763: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 764: single 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 765: single 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 766: single 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 767: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 768: autoscan 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 769: autoscan 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 770: autoscan 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 771: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 772: deglitch 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 773: deglitch 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 774: deglitch 10
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 775: deglitch 33
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 776: errcfg 0x0000
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 777: errcfg 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 778: errcfg
  Cached ERROR_CONFIG: 0x00F9
  Live ERROR_CONFIG:   0x00F9
> 
### command 779: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 780: intb 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 781: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 782: refclk ext
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 783: refclk int
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 784: activate low
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 785: activate full
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 786: rpoverride 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 787: rpoverride 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 788: autoamp 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 789: autoamp 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 790: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 791: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 792: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 793: wreg 0x19 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 794: sync
[36m[I][0m Re-applying cached configuration...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 145 -> [32m169 (+24)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m169[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 795: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 796: rawwreg 0x19 0x00F9
[33m[W][0m Raw register write may desync cached driver config.
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 797: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 798: resetreapply
[36m[I][0m Soft reset + re-apply config...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device back in READY/sleep. Use 'wake' to start conversions.
> 
### command 799: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 800: recover
[36m[I][0m Attempting recovery...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 265 -> [32m267 (+2)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m267[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 801: timing 0 43000000
  Ch0: conversion time = 325.12 us (0.325 ms)
  Ch0: settling time   = 18.98 us (0.019 ms)
  Ch0: sample time     = 344.09 us (0.344 ms)
> 
### command 802: timing 1 43000000
  Ch1: conversion time = 434.98 us (0.435 ms)
  Ch1: settling time   = 26.79 us (0.027 ms)
  Ch1: sample time     = 461.77 us (0.462 ms)
> 
### command 803: timing 2 43000000
  Ch2: conversion time = 545.58 us (0.546 ms)
  Ch2: settling time   = 35.35 us (0.035 ms)
  Ch2: sample time     = 580.93 us (0.581 ms)
> 
### command 804: timing 3 43000000
  Ch3: conversion time = 656.93 us (0.657 ms)
  Ch3: settling time   = 44.65 us (0.045 ms)
  Ch3: sample time     = 701.58 us (0.702 ms)
> 
### command 805: reset
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 806: init
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
### command 807: cfg
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
### command 808: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 809: help

[36m=== LDC1614 CLI Help ===[0m

[32m[Common][0m
  [36mhelp / ?                        [0m - Show this help
  [36mversion / ver                   [0m - Print firmware and library version info
  [36mscan                            [0m - Scan I2C bus

[32m[Data][0m
  [36mread                            [0m - Read configured channels
  [36mread <ch>                       [0m - Read specific channel (0-3)
  [36mread <ch> [N]                   [0m - Read channel N times
  [36mreadfresh [count]               [0m - Read STATUS-driven fresh channel data
  [36mreadstaged <mask> [polls] [instr][0m - Poll-budgeted DATAx read
  [36msamplerate <ch> <N> [timeoutMs] [0m - DRDY-gated sample-rate smoke
  [36mreadblocking                    [0m - Blocking read configured channels (waits for DRDY)
  [36mreadblocking <ch>               [0m - Blocking read specific channel
  [36msample <ch>                     [0m - Get last cached sample (no I2C)
  [36msampleage <ch>                  [0m - Show age of cached sample (ms)
  [36mdrdy                            [0m - Check data ready
  [36mstatus                          [0m - Read and parse STATUS register
  [36mstatus_raw                      [0m - Read raw STATUS register value
  [36mfreq <ch> <fRef>                [0m - Read channel and calc sensor frequency
  [36mtiming <ch> <fRef>              [0m - Calc conversion, settling, and sample time

[32m[Control][0m
  [36minit / begin                    [0m - Initialize/reinitialize device
  [36mend                             [0m - Shut down driver (returns to UNINIT)
  [36msleep                           [0m - Enter sleep mode (stop conversions)
  [36mwake                            [0m - Wake and start conversions
  [36mreset                           [0m - Software reset (returns to UNINIT)
  [36mresetreapply                    [0m - Soft reset + re-apply config (stays READY)

[32m[Configuration][0m
  [36mcfg / config / settings         [0m - Print active configuration snapshot
  [36msnapshot                        [0m - Print settings snapshot struct (no I2C)
  [36mchannels                        [0m - Show configured channel count
  [36mactivech                        [0m - Show current active channel
  [36mactivech <ch>                   [0m - Set active channel (single-ch mode)
  [36msingle <ch>                     [0m - Set single-channel mode and active channel
  [36mautoscan <2|3|4>                [0m - Set auto-scan sequence length
  [36mdeglitch <1|3|10|33>            [0m - Set input deglitch bandwidth in MHz
  [36merrcfg [mask]                   [0m - Show or set ERROR_CONFIG bit mask
  [36mintb [0|1]                      [0m - Show or enable/disable INTB output
  [36mrefclk <int|ext>                [0m - Set reference clock source
  [36mactivate <full|low>             [0m - Set sensor activation current policy
  [36mrpoverride <0|1>                [0m - Enable/disable fixed RP override drive
  [36mautoamp <0|1>                   [0m - Enable/disable auto amplitude correction
  [36mhighcurrent <0|1>               [0m - Enable/disable high-current Ch0 drive
  [36mrcount <ch> <val>               [0m - Set RCOUNT for channel
  [36msettle <ch> <val>               [0m - Set SETTLECOUNT for channel
  [36mclkdiv <ch> <fin> <fref>        [0m - Set clock dividers
  [36moffset <ch> <val>               [0m - Set conversion offset
  [36midrive <ch> <val>               [0m - Set drive current (0-31)
  [36minitidrive <ch>                 [0m - Read auto-calibrated INIT_IDRIVE

[32m[Registers][0m
  [36mreg <addr>                      [0m - Read register (hex address)
  [36mwreg <addr> <val>               [0m - Write register (diagnostic only; may desync cached config)
  [36mrawreg <reg> [addr]             [0m - Raw register read before begin
  [36mrawwreg <reg> <val> [addr]      [0m - Raw register write before begin

[32m[Diagnostics][0m
  [36mdrv                             [0m - Show driver state and health
  [36mstate                           [0m - Compact driver health summary
  [36monline                          [0m - Check if device is online
  [36mid                              [0m - Read MANUFACTURER_ID and DEVICE_ID
  [36mbus                             [0m - I2C scan plus raw LDC identity checks
  [36mprobeaddr <addr>                [0m - Raw LDC identity check at address
  [36mprobe                           [0m - Probe device (no health tracking)
  [36mrecover                         [0m - Manual recovery attempt
  [36msync                            [0m - Re-apply cached config and clear dirty state
  [36mverbose [0|1]                   [0m - Enable/disable verbose output
  [36mstress [N]                      [0m - Run N read cycles (default 10)
  [36mstress_mix [N]                  [0m - Run N mixed-operation stress cycles
  [36mdemo [N]                        [0m - Run N sample demo workflow (default 5)
  [36mselftest                        [0m - Run safe command self-test report
> 
### command 810: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 811: init
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
### command 812: scan
[36m[I][0m Scanning I2C bus (timeout=50ms)...
[36m[I][0m      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
00:                         -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- [32m2A[0m -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- 3C -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- --                         
[36m[I][0m Scan complete. Found 2 device(s).
[36m[I][0m Common addresses: 0x2A/0x2B=LDC1614, 0x48-0x4B=ADS1115, 0x51=RV3032, 0x76/0x77=BME280
[36m[I][0m LDC addresses are verified with MANUFACTURER_ID/DEVICE_ID reads.
> 
### command 813: probeaddr 0x2A
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 814: probe
[36m[I][0m Probing device (no health tracking)...
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  (no health changes)
> 
### command 815: id
=== Device Identity ===
  MANUFACTURER_ID: 0x5449 expected=0x5449 match=[32mYES[0m
  DEVICE_ID:       0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 816: drv
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [32m2[0m
  Total failures: [32m0[0m
  Success rate: [32m100.0%[0m
  Last OK: [32m20 ms ago (at 66003 ms)[0m
  Last error: [32mnever[0m
> 
### command 817: state
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m2[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 818: online
  Online: [32myes[0m
> 
### command 819: cfg
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
### command 820: snapshot
=== Settings Snapshot (no I2C) ===
  State: [32mREADY[0m
  Sleeping: yes
  Measuring: no
  hardwareConfigDirty=0
  Channels: 4  Active: 0  AutoScan: no
  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT  INTB: no
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  Ch0: rcount=0x04D6 settle=0x000A fin=1 fref=1 offset=0x0000 idrive=10 sample_age=0 ms
  Ch1: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch2: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch3: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
> 
### command 821: channels
  Channel count: 4
> 
### command 822: activech
  Active channel: 0
> 
### command 823: status
  STATUS raw=0x0000 drdy=[33mno[0m errCh=0
  Errors: [32mUR=0 OR=0 WD=0 AH=0 AL=0 ZC=0[0m
  Unread: ch0=0 ch1=0 ch2=0 ch3=0
> 
### command 824: status_raw
  STATUS raw = 0x0000
> 
### command 825: rawreg 0x7E
  Raw 0x2A[0x7E] = 0x5449 (21577)
> 
### command 826: rawreg 0x7F
  Raw 0x2A[0x7F] = 0x3055 (12373)
> 
### command 827: reg 0x7E
  Reg 0x7E = 0x5449 (21577)
> 
### command 828: reg 0x7F
  Reg 0x7F = 0x3055 (12373)
> 
### command 829: reg 0x19
  Reg 0x19 = 0x00F9 (249)
> 
### command 830: reg 0x1A
  Reg 0x1A = 0x3401 (13313)
> 
### command 831: reg 0x1B
  Reg 0x1B = 0x020D (525)
> 
### command 832: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 833: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 834: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 835: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 836: rcount 0 0x0123
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 837: settle 0 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 838: clkdiv 0 2 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 839: offset 0 0x0010
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 840: idrive 0 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 841: rcount 1 0x0124
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 842: settle 1 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 843: clkdiv 1 2 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 844: offset 1 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 845: idrive 1 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 846: rcount 2 0x0125
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 847: settle 2 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 848: clkdiv 2 2 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 849: offset 2 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 850: idrive 2 7
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 851: rcount 3 0x0126
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 852: settle 3 0x0014
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 853: clkdiv 3 2 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 854: offset 3 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 855: idrive 3 8
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 856: initidrive 0
  Ch0 INIT_IDRIVE = 0
> 
### command 857: initidrive 1
  Ch1 INIT_IDRIVE = 0
> 
### command 858: initidrive 2
  Ch2 INIT_IDRIVE = 0
> 
### command 859: initidrive 3
  Ch3 INIT_IDRIVE = 0
> 
### command 860: cfg
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
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 861: activech 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 862: activech 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 863: activech 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 864: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 865: single 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 866: single 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 867: single 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 868: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 869: autoscan 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 870: autoscan 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 871: autoscan 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 872: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 873: deglitch 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 874: deglitch 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 875: deglitch 10
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 876: deglitch 33
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 877: errcfg 0x0000
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 878: errcfg 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 879: errcfg
  Cached ERROR_CONFIG: 0x00F9
  Live ERROR_CONFIG:   0x00F9
> 
### command 880: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 881: intb 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 882: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 883: refclk ext
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 884: refclk int
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 885: activate low
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 886: activate full
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 887: rpoverride 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 888: rpoverride 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 889: autoamp 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 890: autoamp 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 891: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 892: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 893: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 894: wreg 0x19 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 895: sync
[36m[I][0m Re-applying cached configuration...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 145 -> [32m169 (+24)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m169[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 896: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 897: rawwreg 0x19 0x00F9
[33m[W][0m Raw register write may desync cached driver config.
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 898: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 899: resetreapply
[36m[I][0m Soft reset + re-apply config...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device back in READY/sleep. Use 'wake' to start conversions.
> 
### command 900: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 901: recover
[36m[I][0m Attempting recovery...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 265 -> [32m267 (+2)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m267[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 902: timing 0 43000000
  Ch0: conversion time = 325.12 us (0.325 ms)
  Ch0: settling time   = 18.98 us (0.019 ms)
  Ch0: sample time     = 344.09 us (0.344 ms)
> 
### command 903: timing 1 43000000
  Ch1: conversion time = 434.98 us (0.435 ms)
  Ch1: settling time   = 26.79 us (0.027 ms)
  Ch1: sample time     = 461.77 us (0.462 ms)
> 
### command 904: timing 2 43000000
  Ch2: conversion time = 545.58 us (0.546 ms)
  Ch2: settling time   = 35.35 us (0.035 ms)
  Ch2: sample time     = 580.93 us (0.581 ms)
> 
### command 905: timing 3 43000000
  Ch3: conversion time = 656.93 us (0.657 ms)
  Ch3: settling time   = 44.65 us (0.045 ms)
  Ch3: sample time     = 701.58 us (0.702 ms)
> 
### command 906: reset
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 907: init
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
### command 908: cfg
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
### command 909: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 910: help

[36m=== LDC1614 CLI Help ===[0m

[32m[Common][0m
  [36mhelp / ?                        [0m - Show this help
  [36mversion / ver                   [0m - Print firmware and library version info
  [36mscan                            [0m - Scan I2C bus

[32m[Data][0m
  [36mread                            [0m - Read configured channels
  [36mread <ch>                       [0m - Read specific channel (0-3)
  [36mread <ch> [N]                   [0m - Read channel N times
  [36mreadfresh [count]               [0m - Read STATUS-driven fresh channel data
  [36mreadstaged <mask> [polls] [instr][0m - Poll-budgeted DATAx read
  [36msamplerate <ch> <N> [timeoutMs] [0m - DRDY-gated sample-rate smoke
  [36mreadblocking                    [0m - Blocking read configured channels (waits for DRDY)
  [36mreadblocking <ch>               [0m - Blocking read specific channel
  [36msample <ch>                     [0m - Get last cached sample (no I2C)
  [36msampleage <ch>                  [0m - Show age of cached sample (ms)
  [36mdrdy                            [0m - Check data ready
  [36mstatus                          [0m - Read and parse STATUS register
  [36mstatus_raw                      [0m - Read raw STATUS register value
  [36mfreq <ch> <fRef>                [0m - Read channel and calc sensor frequency
  [36mtiming <ch> <fRef>              [0m - Calc conversion, settling, and sample time

[32m[Control][0m
  [36minit / begin                    [0m - Initialize/reinitialize device
  [36mend                             [0m - Shut down driver (returns to UNINIT)
  [36msleep                           [0m - Enter sleep mode (stop conversions)
  [36mwake                            [0m - Wake and start conversions
  [36mreset                           [0m - Software reset (returns to UNINIT)
  [36mresetreapply                    [0m - Soft reset + re-apply config (stays READY)

[32m[Configuration][0m
  [36mcfg / config / settings         [0m - Print active configuration snapshot
  [36msnapshot                        [0m - Print settings snapshot struct (no I2C)
  [36mchannels                        [0m - Show configured channel count
  [36mactivech                        [0m - Show current active channel
  [36mactivech <ch>                   [0m - Set active channel (single-ch mode)
  [36msingle <ch>                     [0m - Set single-channel mode and active channel
  [36mautoscan <2|3|4>                [0m - Set auto-scan sequence length
  [36mdeglitch <1|3|10|33>            [0m - Set input deglitch bandwidth in MHz
  [36merrcfg [mask]                   [0m - Show or set ERROR_CONFIG bit mask
  [36mintb [0|1]                      [0m - Show or enable/disable INTB output
  [36mrefclk <int|ext>                [0m - Set reference clock source
  [36mactivate <full|low>             [0m - Set sensor activation current policy
  [36mrpoverride <0|1>                [0m - Enable/disable fixed RP override drive
  [36mautoamp <0|1>                   [0m - Enable/disable auto amplitude correction
  [36mhighcurrent <0|1>               [0m - Enable/disable high-current Ch0 drive
  [36mrcount <ch> <val>               [0m - Set RCOUNT for channel
  [36msettle <ch> <val>               [0m - Set SETTLECOUNT for channel
  [36mclkdiv <ch> <fin> <fref>        [0m - Set clock dividers
  [36moffset <ch> <val>               [0m - Set conversion offset
  [36midrive <ch> <val>               [0m - Set drive current (0-31)
  [36minitidrive <ch>                 [0m - Read auto-calibrated INIT_IDRIVE

[32m[Registers][0m
  [36mreg <addr>                      [0m - Read register (hex address)
  [36mwreg <addr> <val>               [0m - Write register (diagnostic only; may desync cached config)
  [36mrawreg <reg> [addr]             [0m - Raw register read before begin
  [36mrawwreg <reg> <val> [addr]      [0m - Raw register write before begin

[32m[Diagnostics][0m
  [36mdrv                             [0m - Show driver state and health
  [36mstate                           [0m - Compact driver health summary
  [36monline                          [0m - Check if device is online
  [36mid                              [0m - Read MANUFACTURER_ID and DEVICE_ID
  [36mbus                             [0m - I2C scan plus raw LDC identity checks
  [36mprobeaddr <addr>                [0m - Raw LDC identity check at address
  [36mprobe                           [0m - Probe device (no health tracking)
  [36mrecover                         [0m - Manual recovery attempt
  [36msync                            [0m - Re-apply cached config and clear dirty state
  [36mverbose [0|1]                   [0m - Enable/disable verbose output
  [36mstress [N]                      [0m - Run N read cycles (default 10)
  [36mstress_mix [N]                  [0m - Run N mixed-operation stress cycles
  [36mdemo [N]                        [0m - Run N sample demo workflow (default 5)
  [36mselftest                        [0m - Run safe command self-test report
> 
### command 911: version
=== Version Info ===
  Example firmware build: not embedded; use HIL transcript timestamp
  LDC1614 library version: 2.0.0
  LDC1614 full version: 2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)
  LDC1614 build timestamp: 2026-07-01 16:27:00
  LDC1614 git commit: 42feb3b
  LDC1614 git status: clean
  LDC1614 version code: 20000 (major=2 minor=0 patch=0)
> 
### command 912: init
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
### command 913: scan
[36m[I][0m Scanning I2C bus (timeout=50ms)...
[36m[I][0m      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
00:                         -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- [32m2A[0m -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- 3C -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- --                         
[36m[I][0m Scan complete. Found 2 device(s).
[36m[I][0m Common addresses: 0x2A/0x2B=LDC1614, 0x48-0x4B=ADS1115, 0x51=RV3032, 0x76/0x77=BME280
[36m[I][0m LDC addresses are verified with MANUFACTURER_ID/DEVICE_ID reads.
> 
### command 914: probeaddr 0x2A
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 915: probe
[36m[I][0m Probing device (no health tracking)...
=== Raw LDC Identity ===
  Address: 0x2A
  Read MANUFACTURER_ID (0x7E):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x5449 expected=0x5449 match=[32mYES[0m
  Read DEVICE_ID (0x7F):
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Value: 0x3055 expected=0x3055 match=[32mYES[0m
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  (no health changes)
> 
### command 916: id
=== Device Identity ===
  MANUFACTURER_ID: 0x5449 expected=0x5449 match=[32mYES[0m
  DEVICE_ID:       0x3055 expected=0x3055 match=[32mYES[0m
> 
### command 917: drv
=== Driver Health ===
  State: [32mREADY[0m
  Online: [32myes[0m
  Sleeping: [33myes[0m
  Hardware config dirty: [32mno[0m
  Consecutive failures: [32m0[0m
  Total success: [32m2[0m
  Total failures: [32m0[0m
  Success rate: [32m100.0%[0m
  Last OK: [32m20 ms ago (at 68374 ms)[0m
  Last error: [32mnever[0m
> 
### command 918: state
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m2[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 919: online
  Online: [32myes[0m
> 
### command 920: cfg
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
### command 921: snapshot
=== Settings Snapshot (no I2C) ===
  State: [32mREADY[0m
  Sleeping: yes
  Measuring: no
  hardwareConfigDirty=0
  Channels: 4  Active: 0  AutoScan: no
  RR sequence: CH0_CH1  Deglitch: 10 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT  INTB: no
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  Ch0: rcount=0x04D6 settle=0x000A fin=1 fref=1 offset=0x0000 idrive=10 sample_age=0 ms
  Ch1: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch2: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
  Ch3: rcount=0x0080 settle=0x0000 fin=1 fref=1 offset=0x0000 idrive=0 sample_age=0 ms
> 
### command 922: channels
  Channel count: 4
> 
### command 923: activech
  Active channel: 0
> 
### command 924: status
  STATUS raw=0x0000 drdy=[33mno[0m errCh=0
  Errors: [32mUR=0 OR=0 WD=0 AH=0 AL=0 ZC=0[0m
  Unread: ch0=0 ch1=0 ch2=0 ch3=0
> 
### command 925: status_raw
  STATUS raw = 0x0000
> 
### command 926: rawreg 0x7E
  Raw 0x2A[0x7E] = 0x5449 (21577)
> 
### command 927: rawreg 0x7F
  Raw 0x2A[0x7F] = 0x3055 (12373)
> 
### command 928: reg 0x7E
  Reg 0x7E = 0x5449 (21577)
> 
### command 929: reg 0x7F
  Reg 0x7F = 0x3055 (12373)
> 
### command 930: reg 0x19
  Reg 0x19 = 0x00F9 (249)
> 
### command 931: reg 0x1A
  Reg 0x1A = 0x3401 (13313)
> 
### command 932: reg 0x1B
  Reg 0x1B = 0x020D (525)
> 
### command 933: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 934: wake
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 935: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 936: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 937: rcount 0 0x0123
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 938: settle 0 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 939: clkdiv 0 2 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 940: offset 0 0x0010
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 941: idrive 0 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 942: rcount 1 0x0124
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 943: settle 1 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 944: clkdiv 1 2 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 945: offset 1 0x0011
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 946: idrive 1 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 947: rcount 2 0x0125
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 948: settle 2 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 949: clkdiv 2 2 5
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 950: offset 2 0x0012
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 951: idrive 2 7
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 952: rcount 3 0x0126
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 953: settle 3 0x0014
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 954: clkdiv 3 2 6
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 955: offset 3 0x0013
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 956: idrive 3 8
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 957: initidrive 0
  Ch0 INIT_IDRIVE = 0
> 
### command 958: initidrive 1
  Ch1 INIT_IDRIVE = 0
> 
### command 959: initidrive 2
  Ch2 INIT_IDRIVE = 0
> 
### command 960: initidrive 3
  Ch3 INIT_IDRIVE = 0
> 
### command 961: cfg
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
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 962: activech 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 963: activech 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 964: activech 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 965: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 966: single 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 967: single 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 968: single 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 969: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 970: autoscan 2
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 971: autoscan 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 972: autoscan 4
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 973: single 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 974: deglitch 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 975: deglitch 3
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 976: deglitch 10
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 977: deglitch 33
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 978: errcfg 0x0000
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 979: errcfg 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 980: errcfg
  Cached ERROR_CONFIG: 0x00F9
  Live ERROR_CONFIG:   0x00F9
> 
### command 981: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 982: intb 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 983: intb 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 984: refclk ext
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 985: refclk int
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 986: activate low
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 987: activate full
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 988: rpoverride 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 989: rpoverride 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 990: autoamp 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 991: autoamp 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 992: highcurrent 1
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 993: highcurrent 0
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 994: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 995: wreg 0x19 0x00F9
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 996: sync
[36m[I][0m Re-applying cached configuration...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 145 -> [32m169 (+24)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m169[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 997: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 998: rawwreg 0x19 0x00F9
[33m[W][0m Raw register write may desync cached driver config.
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 999: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 1000: resetreapply
[36m[I][0m Soft reset + re-apply config...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
[36m[I][0m Device back in READY/sleep. Use 'wake' to start conversions.
> 
### command 1001: cfg
=== Active Configuration ===
  Initialized: [32myes[0m  State: [32mREADY[0m  Online: [32myes[0m
  I2C address: 0x2A  timeout=50 ms  offlineThreshold=5
  Callbacks: write=yes read=yes now=yes yield=yes gpio=no busReset=no hardReset=no
  Mode: single-channel  RR sequence: CH0_CH1_CH2_CH3  Deglitch: 33 MHz
  RefClk: INTERNAL  Activation: FULL_CURRENT
  RPoverride: yes  AutoAmp: no  HighCurrentDrv: no
  INTB configured: no  INTB output: no
  Cached ERROR_CONFIG: 0x00F9
  Hardware config dirty: [32mno[0m
  Live MUX_CONFIG: 0x420F
  Live CONFIG: 0x3481 (sleep=1)
  Live ERROR_CONFIG: 0x00F9
  Channel count: 4
  Active channel: 0
  Sleeping: [33myes[0m
  --- Channel 0 ---
    Cached: RCOUNT=0x0123 SETTLE=0x0011 CLOCK_DIV(FIN=2,FREF=3) OFFSET=0x0010 IDRIVE=5
    Live RCOUNT: 0x0123 (291)
    Live SETTLECOUNT: 0x0011 (17)
    Live CLOCK_DIV: 0x2003 (FIN=2, FREF=3)
    Live DRIVE_CURRENT: 0x2800 (IDRIVE=5, INIT_IDRIVE=0)
    Live OFFSET: 0x0010 (16)
  --- Channel 1 ---
    Cached: RCOUNT=0x0124 SETTLE=0x0012 CLOCK_DIV(FIN=2,FREF=4) OFFSET=0x0011 IDRIVE=6
    Live RCOUNT: 0x0124 (292)
    Live SETTLECOUNT: 0x0012 (18)
    Live CLOCK_DIV: 0x2004 (FIN=2, FREF=4)
    Live DRIVE_CURRENT: 0x3000 (IDRIVE=6, INIT_IDRIVE=0)
    Live OFFSET: 0x0011 (17)
  --- Channel 2 ---
    Cached: RCOUNT=0x0125 SETTLE=0x0013 CLOCK_DIV(FIN=2,FREF=5) OFFSET=0x0012 IDRIVE=7
    Live RCOUNT: 0x0125 (293)
    Live SETTLECOUNT: 0x0013 (19)
    Live CLOCK_DIV: 0x2005 (FIN=2, FREF=5)
    Live DRIVE_CURRENT: 0x3800 (IDRIVE=7, INIT_IDRIVE=0)
    Live OFFSET: 0x0012 (18)
  --- Channel 3 ---
    Cached: RCOUNT=0x0126 SETTLE=0x0014 CLOCK_DIV(FIN=2,FREF=6) OFFSET=0x0013 IDRIVE=8
    Live RCOUNT: 0x0126 (294)
    Live SETTLECOUNT: 0x0014 (20)
    Live CLOCK_DIV: 0x2006 (FIN=2, FREF=6)
    Live DRIVE_CURRENT: 0x4000 (IDRIVE=8, INIT_IDRIVE=0)
    Live OFFSET: 0x0013 (19)
  config_readback_failures=0
> 
### command 1002: recover
[36m[I][0m Attempting recovery...
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
  Health changes:
  TotalOK: 265 -> [32m267 (+2)[0m
Health: state=[32mREADY[0m online=[32mtrue[0m consec=[32m0[0m ok=[32m267[0m fail=[32m0[0m rate=[32m100.0%[0m
> 
### command 1003: timing 0 43000000
  Ch0: conversion time = 325.12 us (0.325 ms)
  Ch0: settling time   = 18.98 us (0.019 ms)
  Ch0: sample time     = 344.09 us (0.344 ms)
> 
### command 1004: timing 1 43000000
  Ch1: conversion time = 434.98 us (0.435 ms)
  Ch1: settling time   = 26.79 us (0.027 ms)
  Ch1: sample time     = 461.77 us (0.462 ms)
> 
### command 1005: timing 2 43000000
  Ch2: conversion time = 545.58 us (0.546 ms)
  Ch2: settling time   = 35.35 us (0.035 ms)
  Ch2: sample time     = 580.93 us (0.581 ms)
> 
### command 1006: timing 3 43000000
  Ch3: conversion time = 656.93 us (0.657 ms)
  Ch3: settling time   = 44.65 us (0.045 ms)
  Ch3: sample time     = 701.58 us (0.702 ms)
> 
### command 1007: reset
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
### command 1008: init
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
### command 1009: cfg
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
### command 1010: sleep
  Status: [32mOK[0m (code=0, detail=0)
  Message: [33mOK[0m
> 
```
