# LDC1614 Validation Status

This page records the maintained validation boundary. It is not a release
certificate.

## Current Local Evidence

The current working branch has software hardening evidence for:

- Native PlatformIO tests in `native` and `native_cov` environments, including
  fault-injection and poll-budget coverage. The latest local run in this working
  branch passed 162 test cases in each environment.
- Arduino-framework PlatformIO builds for ESP32-S2 and ESP32-S3.
- Core timing/framework-boundary guard.
- Arduino diagnostic CLI command-contract guard.
- Native ESP-IDF example source-contract guard.
- Readiness wording guard.
- HIL runner host parser/no-port/no-sensor artifact tests. The current host
  unit suite contains 24 runner tests.
- ESP32-S2 no-sensor HIL on COM8 with an LDC1614 at `0x2A`: 1010/1010
  chip-only command-set stress commands passed and 200/200 negative/precondition
  stress commands passed. The fixture had no LC sensor and no physical DRDY/INTB
  wiring.
- Generated `Version.h` consistency check.
- Clean package consumer compile guard.
- PlatformIO package creation, with generated package archives removed after
  review.

Pure ESP-IDF build status is separate evidence. Claim it only from `idf.py`
output or CI logs for `examples/esp_idf/basic`.

## Checks To Run Before Merge Review

```sh
python -B -m py_compile scripts/generate_version.py tools/ldc1614_hil_runner.py tools/test_ldc1614_hil_runner.py tools/check_readiness_claims.py tools/check_clean_consumer_compile.py
python tools/check_core_timing_guard.py
python tools/check_cli_contract.py
python tools/check_idf_example_contract.py
python tools/check_readiness_claims.py
python tools/ldc1614_hil_runner.py --parser-self-test
python tools/ldc1614_hil_runner.py --dry-run --quiet
python tools/test_ldc1614_hil_runner.py
python scripts/generate_version.py check
python tools/check_clean_consumer_compile.py
python -m platformio test -e native
python -m platformio test -e native_cov
python -m platformio run -e esp32s3dev
python -m platformio run -e esp32s2dev
python -m platformio pkg pack
```

Remove any generated `.tar.gz` package after `pkg pack` unless it is explicitly
being prepared as a release artifact outside the source tree.

## Hardware Validation

Committed COM8 HIL logs record real ESP32-S2 + LDC1614 chip-only validation
with no LC sensor attached:

- `docs/reports/hil-validation-COM8-20260701.md`
- `docs/reports/hil-validation-COM8-20260701.no-sensor-stress.runner.json`
- `docs/reports/hil-validation-COM8-20260701.no-sensor-negative-stress.runner.json`

These logs validate identity, I2C register access, configuration write/readback,
sleep/wake, reset/reapply, reset/re-init, recovery, timing calculations, and
bounded `BUSY` / `INVALID_PARAM` behavior. They do not validate live conversion
behavior.

On 2026-06-30, COM8 was identified as an ESP32-S2 target. The Arduino-profile
`esp32s2dev` firmware uploaded successfully once, but the runner then captured
no target firmware payload and all bounded CLI commands timed out with empty
output. A later upload retry could not open COM8. The generated files under
`docs/reports/hil-validation-COM8-20260630.runner.*` are marked
`overall_status=NOT_RUN`, `hardware_attached=false`, and
`evidence_type=serial_not_run`. They are audit artifacts only and do not prove
LDC1614/LDC1612 behavior.

Before target deployment decisions or full sensing claims, capture hardware logs
for:

- Device identity and expected I2C address strap for each board variant.
- LDC1612/LDC1614 channel availability.
- Sensor-attached configuration readback for timing, mode, drive-current, offset,
  and error registers.
- DATAx read ordering and DATAx/STATUS side effects.
- INTB behavior when wired.
- SD shutdown/wake behavior when wired.
- Address NACK, unplug/replug, timeout/fault, recovery, and bounded soak cases.
- Sensor clock plan, coil tuning, deglitch selection, and IDRIVE calibration.

Use `HIL_VALIDATION.md` and `../tools/ldc1614_hil_runner.py` to collect JSON and
Markdown evidence. A runner result without real hardware is `NOT_RUN`, not a
pass.
