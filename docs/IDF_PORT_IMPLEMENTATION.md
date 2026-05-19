# LDC1614 ESP-IDF Port Implementation

Implemented on branch `idf-port`.

## Core Boundary

- `include/` and `src/` are framework-neutral and do not include Arduino,
  `Wire`, `Serial`, ESP-IDF I2C, FreeRTOS, or GPIO headers.
- The driver receives all I2C access through `Config::i2cWrite` and
  `Config::i2cWriteRead`; optional INTB reads use `Config::gpioRead`.
- Optional recovery stays application-owned through `Config::busReset` and
  `Config::hardReset`.
- `Config::nowMs` and `Config::cooperativeYield` remain optional. If `nowMs` is
  not supplied, health timestamps use `0`; if `cooperativeYield` is not supplied,
  the driver performs no scheduler call.

## ESP-IDF Additions

- Root `CMakeLists.txt` registers the library as an ESP-IDF component.
- `idf_component.yml` declares component-manager metadata for ESP-IDF 6.x.
- `examples/esp_idf/basic` demonstrates application-owned bus/device setup with
  the new `driver/i2c_master.h` API, `esp_timer_get_time()` timing, a FreeRTOS
  yield hook, optional INTB input handling, `i2c_master_bus_reset()`, and an
  optional SHDN hard-reset hook.
- Arduino and ESP-IDF examples both use `examples/common/Ldc1614Cli.cpp` for the
  user-visible CLI contract. Platform-specific files now provide only output,
  time/delay/yield, default `Config`, and I2C address-probe callbacks.
- The ESP-IDF example exposes the same command set as the Arduino bringup CLI,
  including aliases, help sections, raw `reg` / `wreg`, health diagnostics,
  `probe`, `recover`, `reset`, `resetreapply`, `selftest`, `stress`,
  `stress_mix`, and `demo`.

## Validation

- Static check target: `python tools/check_core_timing_guard.py`
  enforces no Arduino timing calls/includes in `include/` and `src/`.
- CLI parity checks: `python tools/check_cli_contract.py` and
  `python tools/check_idf_example_contract.py`.
- Arduino examples remain under `examples/01_basic_bringup_cli` and continue to
  provide `Wire`, `millis()`, and `yield()` through example-local callbacks.
- PlatformIO Arduino builds pass for ESP32-S3 and ESP32-S2 through
  `python -m platformio run -e esp32s3dev` and
  `python -m platformio run -e esp32s2dev`.
- Native tests pass through `python -m platformio test -e native`.
- IDF builds were not run in this environment because `idf.py` was not on PATH.

## Remaining Hardware Work

- Build `examples/esp_idf/basic` for ESP32-S3 and ESP32-S2 with ESP-IDF 6.0.1.
- Validate manufacturer ID, device ID, LDC1612/LDC1614 channel-count configs,
  single-channel reads, autoscan, INTB, sleep/wake, bus reset, hard reset, and
  health/recovery behavior on hardware.
