# LDC1614 COM8 Audit And HIL Validation Report

Date: 2026-06-30
Local time: 2026-06-30T16:31:26+02:00
Operator: Codex
Repository: `c:\Users\Honza\Documents\Projects\LDC1614`
Branch: `hardening/ldc1614-industry-readiness`
Commit: `e96f117baa7ebaa3108d09c930b6c0cce04ab0e2`
Working tree: dirty, with implementation, test, runner, and documentation edits

## Host And Tooling

| Item | Value |
| --- | --- |
| OS | Microsoft Windows 11 Education 10.0.26200 |
| Python | 3.12.10 |
| PlatformIO | Core 6.1.18 |
| ESP-IDF CLI | `idf.py` not available on PATH |
| Target PlatformIO envs | `esp32s3dev`, `esp32s2dev`, `native`, `native_cov` |
| Corrected attached MCU board | ESP32-S2, detected as ESP32-S2FH4 |
| Requested HIL port | COM8, 115200 baud |
| Runner profile | `arduino` |

Detected serial ports:

| Port | Device |
| --- | --- |
| COM5 | USB Serial Device, VID:PID 303A:1001 |
| COM8 | USB Serial Device, VID:PID 303A:0002 |
| COM19 | USB Serial Device, VID:PID 303A:1001 |
| COM20 | USB Serial Device, VID:PID 303A:1001 |
| COM21 | USB Serial Device, VID:PID 303A:1001 |

## HIL Result

No LDC1614/LDC1612 hardware validation was completed in this run.

After board correction, the Arduino `esp32s2dev` firmware uploaded successfully
to COM8 once. Esptool identified the chip as `ESP32-S2FH4` revision v1.0 with
embedded 4 MB flash and MAC `48:f6:ee:71:86:66`.

The runner was then executed against COM8, but it captured no target firmware
payload. Every bounded CLI command timed out with empty output, so no
LDC1614/LDC1612 command evidence was collected. The S2 PlatformIO environment
had been using `board_upload.after_reset = no_reset_stub`, which left the board
in the flasher stub after the successful upload; this was changed to
`hard_reset` for future upload/run cycles.

A subsequent upload attempt with the corrected reset setting could not open
COM8:

```text
Cannot configure port, something went wrong. Original message:
PermissionError(13, 'A device attached to the system is not functioning.', None, 31)
```

No LDC1614/LDC1612 device identity, I2C address, channel count, sensor data,
INTB behavior, SD behavior, fault recovery, or soak behavior was validated.

Runner artifacts:

| Artifact | Status |
| --- | --- |
| `docs/reports/hil-validation-COM8-20260630.runner.json` | `overall_status=NOT_RUN`, `hardware_attached=false`, `evidence_type=serial_not_run` |
| `docs/reports/hil-validation-COM8-20260630.runner.md` | Bounded command transcript with empty command outputs/timeouts, no firmware payload |

HIL counts:

| Area | PASS | FAIL | UNKNOWN | NOT_RUN |
| --- | ---: | ---: | ---: | ---: |
| Firmware payload capture | 0 | 0 | 0 | 1 |
| Bounded CLI command attempts | 0 | 17 | 0 | 0 |
| Functional HIL checks | 0 | 0 | 0 | 1 |
| Sample-rate HIL check | 0 | 0 | 0 | 1 |
| Stress HIL check | 0 | 0 | 0 | 1 |
| 8-hour soak | 0 | 0 | 0 | 1 |

The 8-hour soak elapsed time was 0 seconds.

## Software Edits Completed Before HIL

| Area | Change |
| --- | --- |
| Driver sample cache | Cleared cached samples after successful config setters, config apply, reset/reapply, soft reset, `begin()`, `end()`, and diagnostic raw writes that can invalidate old conversion data. |
| Driver output safety | Cleared caller output structures before failed reads so stale caller data is not mistaken for fresh device data. |
| Dirty/offline behavior | Made idempotent `sleep()` / `wake()` report dirty configuration or offline state instead of returning silent success. |
| Measuring state | Tightened `isMeasuring()` to require initialized, online, awake, and clean hardware configuration state. |
| Arduino CLI | Added `readfresh`, `readstaged`, `samplerate`, and `sync`; expanded STATUS, INTB, dirty-state, and config-readback diagnostics; tightened numeric parsing. |
| Board example config | Added build-flag overrides for diagnostic example I2C address and channel count. |
| ESP-IDF CLI | Added native fixed-buffer `sleep` and `wake` commands without Arduino compatibility code. |
| PlatformIO ESP32-S2 upload | Changed `esp32s2dev` upload reset behavior from `no_reset_stub` to `hard_reset` so upload sessions should reset into firmware instead of staying in the flasher stub. |
| HIL runner | Separated requested serial port from hardware evidence, classified ambiguous command output as `UNKNOWN`, added bounded argument checks, added DRDY-gated sample-rate parsing, and expanded metadata for stress/soak/reporting. |
| Tests | Added native regression coverage for stale output/cache behavior, dirty sleep/wake behavior, reset/config apply cache clearing, and measuring-state edge cases. |

## Final Validation Run Locally

The following checks passed after implementation, documentation, and report
updates:

| Check | Result |
| --- | --- |
| `python -B -m py_compile scripts\generate_version.py tools\ldc1614_hil_runner.py tools\test_ldc1614_hil_runner.py tools\check_readiness_claims.py tools\check_clean_consumer_compile.py tools\check_cli_contract.py tools\check_idf_example_contract.py` | PASS |
| `python tools\check_core_timing_guard.py` | PASS |
| `python tools\check_cli_contract.py` | PASS |
| `python tools\check_idf_example_contract.py` | PASS |
| `python tools\check_readiness_claims.py` | PASS |
| `python tools\ldc1614_hil_runner.py --parser-self-test --quiet` | PASS |
| `python tools\test_ldc1614_hil_runner.py` | PASS, 19 tests |
| `python tools\ldc1614_hil_runner.py --dry-run --quiet` | PASS, NOT_RUN dry-run artifact only |
| `python scripts\generate_version.py check` | PASS |
| `python tools\check_clean_consumer_compile.py` | PASS |
| `python -m platformio test -e native` | PASS, 162 tests |
| `python -m platformio test -e native_cov` | PASS, 162 tests |
| `python -m platformio run -e esp32s3dev` | PASS |
| `python -m platformio run -e esp32s2dev` | PASS |
| `python -m platformio run -e esp32s2dev -t upload --upload-port COM8` | PASS once after board correction; later retry failed to open COM8 |
| `python -m platformio pkg pack` | PASS, generated archive removed after check |
| `git diff --check` | PASS; only Git LF-to-CRLF working-copy warnings were printed |

`idf.py --version` failed because `idf.py` is not available on PATH, so pure
ESP-IDF build evidence was not captured in this run.

## Hardware Evidence Still Required

Before any deployment decision for a target board, capture real hardware logs
for at least:

- Device identity and expected address strap.
- LDC1612/LDC1614 channel availability and rejected invalid channels.
- Config readback for timing, mode, drive current, offset, and error registers.
- DATAx ordering, STATUS side effects, fresh-sample behavior, and sample rate.
- INTB and SD behavior when those pins are wired.
- Address NACK, unplug/replug, timeout/fault, recovery, and bounded soak cases.
- Sensor clock plan, coil behavior, deglitch margin, and IDRIVE calibration.
