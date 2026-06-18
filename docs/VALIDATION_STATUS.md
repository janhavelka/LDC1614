# LDC1614 Validation Status

This page records the maintained validation boundary. It is not a release
certificate and it does not record hardware validation.

## Current Local Evidence

The current working branch has software hardening evidence for:

- Native PlatformIO tests in `native` and `native_cov` environments, including
  fault-injection and poll-budget coverage. The latest local run in this working
  branch passed 145 test cases in each environment.
- Arduino-framework PlatformIO builds for ESP32-S2 and ESP32-S3.
- Core timing/framework-boundary guard.
- Arduino diagnostic CLI command-contract guard.
- Native ESP-IDF example source-contract guard.
- Readiness wording guard.
- HIL runner host parser/no-port artifact tests.
- Generated `Version.h` consistency check.
- Clean package consumer compile guard.
- PlatformIO package creation, with generated package archives removed after
  review.

Pure ESP-IDF build status is separate evidence. Claim it only from `idf.py`
output or CI logs for `examples/esp_idf/basic`.

## Checks To Run Before Merge Review

```sh
python -m py_compile scripts/generate_version.py tools/ldc1614_hil_runner.py tools/test_ldc1614_hil_runner.py tools/check_readiness_claims.py tools/check_clean_consumer_compile.py
python tools/check_core_timing_guard.py
python tools/check_cli_contract.py
python tools/check_idf_example_contract.py
python tools/check_readiness_claims.py
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

No committed real LDC1612/LDC1614 HIL logs are recorded here.

Before target deployment decisions, capture hardware logs for:

- Device identity and expected I2C address strap.
- LDC1612/LDC1614 channel availability.
- Configuration readback for timing, mode, drive-current, offset, and error
  registers.
- DATAx read ordering and DATAx/STATUS side effects.
- INTB behavior when wired.
- SD shutdown/wake behavior when wired.
- Address NACK, unplug/replug, timeout/fault, recovery, and bounded soak cases.
- Sensor clock plan, coil tuning, deglitch selection, and IDRIVE calibration.

Use `HIL_VALIDATION.md` and `../tools/ldc1614_hil_runner.py` to collect JSON and
Markdown evidence. A runner result without real hardware is `NOT_RUN`, not a
pass.
