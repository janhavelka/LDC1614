# LDC1614 Validation Status

This page summarizes maintained validation expectations. It is not a release
certificate and does not record hardware validation.

## Software Checks

Maintained CI/local checks cover:
- Core framework-boundary and timing guard.
- Arduino diagnostic CLI command contract.
- Native ESP-IDF example source contract.
- Generated version metadata check.
- Native PlatformIO unit tests, including fault-injection coverage.
- Arduino-framework PlatformIO builds for ESP32-S2 and ESP32-S3.
- PlatformIO package creation.

Pure ESP-IDF build status should be taken from `idf.py` output or CI logs for
`examples/esp_idf/basic`.

## Hardware Validation

Before release decisions for a target product, capture hardware logs for:
- Device identity and expected I2C address strap.
- LDC1612/LDC1614 channel availability.
- Configuration readback for timing, mode, drive-current, offset, and error
  registers.
- DATAx reads and DATAx/STATUS error handling.
- INTB behavior when wired.
- SD shutdown/wake behavior when wired.
- Address NACK, unplug/replug, timeout/fault, recovery, and bounded soak cases.
- Sensor clock plan, coil tuning, deglitch selection, and IDRIVE calibration.

Use `docs/HIL_VALIDATION.md` and `tools/ldc1614_hil_runner.py` to collect JSON
and Markdown evidence. A runner result without real hardware is `NOT_RUN`, not a
pass.
