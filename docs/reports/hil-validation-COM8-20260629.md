# LDC1614 HIL Validation Audit - COM8 - 2026-06-29

Date/time: 2026-06-29 16:52:21 +02:00  
Timezone: Europe/Prague  
Repository: `c:\Users\Honza\Documents\Projects\LDC1614`  
Branch: `hardening/ldc1614-industry-readiness`  
Commit: `337186daa9c3f885e92e1052b042d8e2cd0a6f18`  
Dirty status at report time: local edits plus new `docs/reports/` artifacts  

## Executive Summary

No hardware-in-the-loop validation was run. No LDC1614/LDC1612 board or sensor
fixture was attached, so flashing, serial boot capture, device identity reads,
functional HIL, fault injection, sampling benchmarks, and the 8-hour soak are
all `NOT RUN`.

Pre-HIL software/tooling edits were completed and verified locally. The HIL
runner now supports explicit dry-run/parser self-test modes, elapsed per-command
reporting, configurable expected/failure tokens, expected-failure tokens for
negative tests, no-hardware evidence metadata, and a bounded Arduino sample-rate
command plan. Core driver audit fixes were implemented for INTB DRDY fallback,
sleeping DATA reads, soft-reset dirty state, recovery during active poll jobs,
and watchdog-marked DATA caching.

Runner artifacts:

- `docs/reports/hil-validation-COM8-20260629.runner.json`
- `docs/reports/hil-validation-COM8-20260629.runner.md`

Runner HIL command counts: `PASS=0`, `FAIL=0`, `UNKNOWN/REVIEW=0`, `NOT_RUN=15`.
Optional long soak: `NOT_RUN`.

## Setup

| Item | Value |
| --- | --- |
| OS | Microsoft Windows 11 Education 10.0.26200, 64-bit |
| Python | 3.12.10 |
| PlatformIO | Core 6.1.18 |
| CMake | 4.0.1 |
| ESP-IDF CLI | `idf.py` not found in PATH |
| Library | `LDC1614` version `1.0.0` |
| Target device | TI LDC1614/LDC1612 multi-channel 28-bit inductance-to-digital converter |
| Library type | Framework-neutral C++ driver core with injected I2C transport |
| Supported firmware targets | ESP32-S3 Arduino `esp32s3dev`; ESP32-S2 Arduino `esp32s2dev`; native ESP-IDF example source |
| Serial port requested | `COM8` |
| Baud | `115200` |
| Hardware fixture | Missing: no LDC1614/LDC1612 board attached |
| Electrical safety assumptions | No electrical tests performed; no unsafe stimulus applied |
| Detected device identity/address | `NOT RUN` |

## Exact Commands

Commands run:

```powershell
python -B -m py_compile scripts\generate_version.py tools\ldc1614_hil_runner.py tools\test_ldc1614_hil_runner.py tools\check_readiness_claims.py tools\check_clean_consumer_compile.py
python tools\ldc1614_hil_runner.py --parser-self-test
python tools\test_ldc1614_hil_runner.py
python -m platformio test -e native
python tools\check_core_timing_guard.py
python tools\check_cli_contract.py
python tools\check_idf_example_contract.py
python tools\check_readiness_claims.py
python scripts\generate_version.py check
python tools\check_clean_consumer_compile.py
python -m platformio test -e native_cov
python -m platformio run -e esp32s3dev
python -m platformio run -e esp32s2dev
python -m platformio pkg pack
git diff --check
```

Dry-run artifact command:

```powershell
python tools\ldc1614_hil_runner.py --profile arduino --port COM8 --baud 115200 --dry-run --operator Codex --board "No LDC1614/LDC1612 fixture attached" --note "No hardware attached; command plan only; all HIL steps NOT_RUN" --include-stress --stress-count 50 --sample-rate-count 50 --include-long-soak --json-out docs\reports\hil-validation-COM8-20260629.runner.json --markdown-out docs\reports\hil-validation-COM8-20260629.runner.md --quiet
```

Commands not run because the fixture is missing:

```powershell
pio run -e esp32s3dev -t upload --upload-port COM8
python tools\ldc1614_hil_runner.py --profile arduino --port COM8 --baud 115200 --require-run --verbose
```

## HIL Coverage Summary

| Area | Result | Observed Result | Notes |
| --- | --- | --- | --- |
| Serial boot and prompt | `NOT RUN` | No serial hardware session | Missing LDC1614/LDC1612 fixture |
| Version/build info over serial | `NOT RUN` | Dry-run command only | `version` planned |
| Bus scan/address discovery | `NOT RUN` | Dry-run command only | `scan` planned |
| Probe/device identity | `NOT RUN` | Dry-run command only | `probe` and `id` planned |
| Driver health/config/status | `NOT RUN` | Dry-run command only | `drv`, `cfg`, `status`, `drdy` planned |
| Data reads/conversions | `NOT RUN` | Dry-run command only | `read` planned |
| Recovery | `NOT RUN` | Dry-run command only | `recover` planned |
| Timing helper | `NOT RUN` | Dry-run command only | `timing 0 43000000` planned |
| Self-test | `NOT RUN` | Dry-run command only | `selftest` planned |
| Stress/sampling | `NOT RUN` | Dry-run command only | `stress 50`, `read 0 50` planned |
| Fault injection | `NOT RUN` | No fixture | Unsafe/unsupported without hardware |
| 8-hour soak | `NOT RUN` | No fixture | `--include-long-soak` recorded as skipped optional |

## Dry-Run Command Table

| Test ID | Feature Area | Command | Expected Result | Observed Result | Elapsed | Result | Notes |
| --- | --- | --- | --- | --- | ---: | --- | --- |
| DRY-001 | CLI | `help` | Command responds | Not sent | 0.000 s | `NOT_RUN` | Dry-run only |
| DRY-002 | Version | `version` | Version output | Not sent | 0.000 s | `NOT_RUN` | Dry-run only |
| DRY-003 | Bus discovery | `scan` | I2C scan output | Not sent | 0.000 s | `NOT_RUN` | Dry-run only |
| DRY-004 | Identity | `probe` | OK identity probe | Not sent | 0.000 s | `NOT_RUN` | Dry-run only |
| DRY-005 | Identity | `id` | Manufacturer/device IDs | Not sent | 0.000 s | `NOT_RUN` | Dry-run only |
| DRY-006 | Health | `drv` | Driver health snapshot | Not sent | 0.000 s | `NOT_RUN` | Dry-run only |
| DRY-007 | Config | `cfg` | Config snapshot | Not sent | 0.000 s | `NOT_RUN` | Dry-run only |
| DRY-008 | Status | `status` | STATUS decode | Not sent | 0.000 s | `NOT_RUN` | Dry-run only |
| DRY-009 | Data-ready | `drdy` | Ready state | Not sent | 0.000 s | `NOT_RUN` | Dry-run only |
| DRY-010 | Data read | `read` | Channel data | Not sent | 0.000 s | `NOT_RUN` | Dry-run only |
| DRY-011 | Recovery | `recover` | Bounded recovery | Not sent | 0.000 s | `NOT_RUN` | Dry-run only |
| DRY-012 | Timing | `timing 0 43000000` | Timing estimate | Not sent | 0.000 s | `NOT_RUN` | Dry-run only |
| DRY-013 | Self-test | `selftest` | Self-test summary | Not sent | 0.000 s | `NOT_RUN` | Dry-run only |
| DRY-014 | Stress | `stress 50` | 50 bounded operations | Not sent | 0.000 s | `NOT_RUN` | Dry-run only |
| DRY-015 | Sample-rate smoke | `read 0 50` | 50 bounded reads | Not sent | 0.000 s | `NOT_RUN` | Dry-run only |

Transcript: none captured. The runner artifact contains an empty transcript and
`not_run_reason="dry-run requested; no serial commands were sent"`.

## Timing And Soak

No sampling frequency, latency, DRDY cadence, recovery timing, reset timing, or
8-hour soak measurements were captured on hardware. The only timing evidence in
this pass is software test/build duration and dry-run elapsed values of `0.000 s`.

8-hour soak summary:

| Field | Value |
| --- | --- |
| Result | `NOT RUN` |
| Duration | `0 s` |
| Start/end | Not applicable |
| Command mix | Planned only; not sent |
| Sample counts | `0` |
| Error counts | `0 observed`, no hardware evidence |
| Reset/recovery counts | `0 observed`, no hardware evidence |
| Worst latency | Not measured |
| Health-state changes | Not measured |

## Audit Findings And Fixes

| ID | Severity | Reference | Finding | Fix Implemented | Native Test |
| --- | --- | --- | --- | --- | --- |
| AUD-001 | Medium | `src/LDC1614.cpp:501` | `readDataReady()` could treat INTB high as not-ready even when DRDY was not routed to INTB. | INTB high fast path is now used only when `ERROR_CONFIG.DRDY_2INT` is enabled; otherwise STATUS is polled. | `test_readDataReady_intb_high_polls_status_when_drdy_not_routed` |
| AUD-002 | Medium | `src/LDC1614.cpp:384`, `src/LDC1614.cpp:816` | High-level DATA reads could run while asleep and cache cleared/stale DATA registers. | `readChannel()`, `readFreshChannels()`, and `startReadChannels()` now return `BUSY` while sleeping before DATAx I2C. Raw register diagnostics remain available. | `test_high_level_reads_reject_sleeping_without_i2c` |
| AUD-003 | Medium | `src/LDC1614.cpp:633` | A successful clean `softReset()` left `hardwareConfigDirty()==false` while hardware registers were reset to defaults. | Successful `softReset()` marks hardware config dirty until `begin()` reinitializes. | `test_softReset_success_marks_clean_cache_dirty_until_reinit` |
| AUD-004 | Medium | `src/LDC1614.cpp:364` | `recover()` could continue the recovery ladder while a poll-chunked job was active. | `recover()` now returns `BUSY` immediately when a chunked job is active. | `test_recover_returns_busy_without_reset_during_active_poll_job` |
| AUD-005 | Medium | `src/LDC1614.cpp:1918` | Watchdog-marked DATA was parsed and cached like a normal sample. | `_storeChannelData()` now returns `SENSOR_ERROR` for `ERR_WD` and does not update the cache. | `test_readChannel_watchdog_sample_is_sensor_error_and_not_cached`, `test_poll_readChannels_watchdog_sample_is_sensor_error_and_not_cached` |
| AUD-006 | Low | `tools/ldc1614_hil_runner.py:148` | Runner classifier could miss semantic CLI failure tokens when output also contained `code=0`. | Added semantic failure patterns and configurable expected/failure/expected-failure tokens. | `tools/test_ldc1614_hil_runner.py` |
| AUD-007 | Low | `tools/ldc1614_hil_runner.py:280` | No explicit dry-run/parser-self-test mode or no-hardware evidence metadata. | Added dry-run rows, parser self-test, elapsed command timing, `hardware_attached`, and `evidence_type`. | `tools/test_ldc1614_hil_runner.py` |

Required future HIL regressions: run the same scenarios on real hardware where
safe: INTB high with `DRDY_2INT=0`, DATA read while sleeping, soft reset then
identity/config reinit, recover during active poll job, watchdog/error DATA path
if a safe fixture can induce it, and dry-run/non-dry-run runner parsing.

## Verification Results

| Command | Result |
| --- | --- |
| `python -B -m py_compile ...` | PASS |
| `python tools\ldc1614_hil_runner.py --parser-self-test` | PASS |
| `python tools\test_ldc1614_hil_runner.py` | PASS, 15 tests |
| `python tools\check_core_timing_guard.py` | PASS |
| `python tools\check_cli_contract.py` | PASS |
| `python tools\check_idf_example_contract.py` | PASS |
| `python tools\check_readiness_claims.py` | PASS |
| `python scripts\generate_version.py check` | PASS |
| `python tools\check_clean_consumer_compile.py` | PASS |
| `python -m platformio test -e native` | PASS, 151 tests |
| `python -m platformio test -e native_cov` | PASS, 151 tests |
| `python -m platformio run -e esp32s3dev` | PASS |
| `python -m platformio run -e esp32s2dev` | PASS |
| `python -m platformio pkg pack` | PASS; generated `LDC1614-1.0.0.tar.gz` removed |
| `git diff --check` | PASS; Git emitted line-ending normalization warnings only |

ESP-IDF source contract passed, but a native `idf.py build` was not run because
`idf.py` is not available in PATH on this machine.

## Limitations

- No real hardware evidence was captured.
- No firmware was flashed to `COM8`.
- No serial boot transcript or prompt responsiveness was captured.
- No LDC1612/LDC1614 identity, address strap, channel, INTB, SD, fault, coil,
  deglitch, drive-current, recovery, or soak behavior was measured.
- No CI status was checked.

This report is software audit evidence only. It is not a hardware validation
pass and must not be used as a production readiness claim.
