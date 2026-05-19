# LDC1614 ESP-IDF v6.0.1 Port Audit

Last audited: 2026-05-19

This started as a readiness audit and now records the ESP-IDF implementation
target for branch `idf-port`. See `docs/IDF_PORT_IMPLEMENTATION.md` for the
implemented file-level summary and validation notes.

Official ESP-IDF references for the future port:
- I2C master driver: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/i2c.html
- ESP-IDF v6.0 peripheral migration guide: https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/migration-guides/release-6.x/6.0/peripherals.html

## Current Framework/Library State

- `library.json` version is `1.0.0`; the package declares `arduino` and
  `espidf` framework support on `espressif32`.
- `platformio.ini` builds the Arduino CLI example for ESP32-S3 and ESP32-S2 and
  includes a native Unity test environment.
- Public API is under `include/LDC1614/` and is already callback-based at the
  I2C boundary.
- `include/LDC1614/Config.h` exposes `I2cWriteFn`, `I2cWriteReadFn`,
  optional `GpioReadFn`, `NowMsFn`, `YieldFn`, `BusResetFn`, and `HardResetFn`.
- `include/LDC1614/LDC1614.h` exposes `Status begin(const Config&)`,
  `void tick(uint32_t nowMs)`, `void end()`, channel readout, sleep/wake/reset,
  INTB/data-ready helpers, recovery ladder, raw register access, sample cache,
  and four-state health tracking.
- `src/LDC1614.cpp` routes I2C through `_i2cWriteReadRaw`, `_i2cWriteRaw`,
  `_i2cWriteReadTracked`, and `_i2cWriteTracked`; health is updated from
  tracked wrappers.
- The library core no longer includes `<Arduino.h>` and no longer calls
  `millis()` or `yield()` from `_nowMs()` / `_cooperativeYield()`; applications
  should provide timing/yield hooks when needed.
- Arduino-only glue lives in `examples/common/I2cTransport.h`,
  `I2cScanner.h`, `BoardConfig.h`, and the Arduino platform wrapper.
- The user-visible CLI contract lives in framework-neutral
  `examples/common/Ldc1614Cli.cpp` and is used by both Arduino and ESP-IDF
  examples.

Readiness verdict: the driver core is framework-neutral and the Arduino/ESP-IDF
examples share the same CLI behavior. Final readiness still requires an
ESP-IDF 6.0.1 build and hardware validation.

## Portability Blockers

- ESP-IDF compilation has not been verified in this shell because `idf.py` was
  unavailable.
- Hardware validation remains outstanding.
- INTB support depends on `Config::gpioRead`; IDF examples must configure GPIO
  externally and pass a callback.
- Recovery support can call `Config::busReset` and `Config::hardReset`; IDF
  implementations must keep those bounded and must not silently reconfigure bus
  ownership inside the core driver.
- Arduino platform wrappers use `Serial`, `Wire`, `millis()`, `delay()`, and
  `yield()`; keep those calls outside `include/` and `src/`.
- IDF v6.0.1 warning profiles can expose implicit conversions in register field
  packing, channel indexing, and floating-point utility calculations.

## Implemented Files/APIs

- `src/LDC1614.cpp` has no Arduino or ESP-IDF framework includes and keeps
  `_i2cWriteReadRaw()`, `_i2cWriteRaw()`, `_i2cWriteReadTracked()`, and
  `_i2cWriteTracked()` as the only transport path.
- `include/LDC1614/Config.h` preserves callback-based I2C, GPIO, time/yield,
  bus reset, and hard reset hooks without including framework headers.
- Root `CMakeLists.txt` registers the library as an ESP-IDF component.
- `examples/esp_idf/basic/` owns bus/device setup, GPIO setup, console I/O, and
  native `driver/i2c_master.h` transport callbacks.
- `examples/common/Ldc1614Cli.cpp` owns the shared user-visible command
  behavior for Arduino and ESP-IDF examples.

## Proposed Architecture Preserving Arduino Compatibility

- Keep the LDC1614 core callback-based and framework-neutral.
- Keep the Arduino `Wire` adapter in `examples/common/I2cTransport.h`.
- Add an IDF I2C adapter outside the driver core. It owns IDF bus/device
  handles and supplies callbacks to `LDC1614::Config`.
- Implement optional IDF GPIO callbacks for INTB and hard-reset/SHDN handling
  outside the core driver.
- Keep bus setup, pins, pull-ups, clock speed, and bus lifetime in the
  application/example.
- Preserve existing health and recovery semantics:
  - `probe()` uses raw I2C and does not update health;
  - public register helpers use tracked wrappers;
  - validation errors are not transport failures;
  - `recover()` may call identity reads, bus reset, soft reset/reapply, and
    hard reset/reapply according to `Config`.
- Keep Arduino and IDF examples separate.

## IDF Transport Adapter Contract

The adapter should use the ESP-IDF v6.0.1 new I2C master driver only:

```cpp
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

struct Ldc1614IdfI2c {
  i2c_master_bus_handle_t bus = nullptr;
  i2c_master_dev_handle_t dev = nullptr;
  uint8_t address = 0x2A;
  gpio_num_t intb = GPIO_NUM_NC;
  gpio_num_t shdn = GPIO_NUM_NC;
};
```

Callback behavior:
- `i2cWrite(addr, data, len, timeoutMs, user)` calls
  `i2c_master_transmit(dev, data, len, timeoutMs)`.
- `i2cWriteRead(addr, txData, txLen, rxData, rxLen, timeoutMs, user)` calls
  `i2c_master_transmit_receive(dev, txData, txLen, rxData, rxLen, timeoutMs)`.
- The callbacks must be synchronous from the driver point of view. Do not
  register `i2c_master_register_event_callbacks()` on this handle unless the
  adapter waits for completion before returning; tracked wrappers update health
  immediately after the callback returns.
- Reject addresses other than the configured device address. LDC1614 valid
  addresses are `0x2A` and `0x2B`.
- Map `ESP_OK` to `LDC1614::Status::Ok()`.
- Map `ESP_ERR_TIMEOUT` to `LDC1614::Err::I2C_TIMEOUT`.
- Map `ESP_ERR_INVALID_RESPONSE` to an I2C NACK-related status. The simple
  ESP-IDF master APIs do not distinguish address and data phase, so prefer
  `LDC1614::Err::I2C_ERROR` with `Status.detail = ESP_ERR_INVALID_RESPONSE`
  unless a custom adapter can prove the phase.
- Map other adapter or bus failures to `LDC1614::Err::I2C_BUS` or
  `LDC1614::Err::I2C_ERROR`; preserve raw `esp_err_t` in `Status::detail`.
- Clamp or reject `timeoutMs` before passing it to ESP-IDF's signed
  `xfer_timeout_ms`; never allow overflow to become `-1` because `-1` waits
  forever.
- `gpioRead(pin, user)` should call `gpio_get_level()`.
- `busReset(user)` should be a bounded bus-recovery routine owned by the
  application. Prefer `i2c_master_bus_reset()` when the bus handle remains
  valid. If the adapter removes/recreates the bus or device, it must update
  every cached handle in `Ldc1614IdfI2c` and leave `dev` valid before returning
  `OK`.
- `hardReset(user)` should toggle SHDN or the board-specific reset circuit with
  bounded delays.
- `nowMs(user)` should return `esp_timer_get_time() / 1000`.
- `cooperativeYield(user)` should call `taskYIELD()` or `vTaskDelay(1)`.

## Component/CMake Layout

Recommended component layout:

```text
LDC1614/
  CMakeLists.txt
  include/LDC1614/*.h
  src/LDC1614.cpp
  examples/esp_idf/basic/
    CMakeLists.txt
    main/CMakeLists.txt
    main/main.cpp
    main/Ldc1614IdfI2cTransport.cpp
```

Core-only component:

```cmake
idf_component_register(
  SRCS "src/LDC1614.cpp"
  INCLUDE_DIRS "include"
)
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
```

If an IDF adapter is shipped inside the component, include its source and add
`PRIV_REQUIRES esp_driver_i2c esp_driver_gpio esp_timer freertos`. If the
adapter lives only in the example, put those requirements in the example
component instead.

## Example Status

- `examples/01_basic_bringup_cli` and `examples/esp_idf/basic` use the same
  shared CLI implementation for the command contract.
- The ESP-IDF example creates an I2C master bus with `i2c_new_master_bus()`,
  adds the LDC1614 device with `i2c_master_bus_add_device()` at `0x2A`, and uses
  `i2c_master_probe()` for the shared `scan` command.
- The ESP-IDF example fills `LDC1614::Config` with IDF callbacks, channel count,
  channel parameters, `nowMs`, `cooperativeYield`, and optional INTB/hard-reset
  hooks.
- Auto-scan, INTB data-ready, bounded blocking reads, diagnostics, raw register
  access, self-test, stress, and demo workflows are available through CLI
  commands rather than separate one-off sample code.

## Test/Validation Plan

- Static checks:
  - `python tools/check_core_timing_guard.py`
  - `python tools/check_cli_contract.py`
  - `python tools/check_idf_example_contract.py`
  - `rg "driver/i2c.h|i2c_cmd_link|i2c_driver_install" .` should not find
    legacy I2C driver usage in IDF code.
- Arduino regression:
  - `python -m platformio test -e native`
  - `python -m platformio run -e esp32s3dev`
  - `python -m platformio run -e esp32s2dev`
- IDF build:
  - `idf.py set-target esp32s3 build` from `examples/esp_idf/basic`
  - `idf.py set-target esp32s2 build` from `examples/esp_idf/basic`
- Hardware validation:
  - `begin()` verifies manufacturer and device IDs.
  - Validate `channelCount` for LDC1612-style 2-channel and LDC1614 4-channel
    configurations.
  - Read DATAx_MSB then DATAx_LSB coherently and validate 28-bit parsing.
  - Validate sleep/wake, single-channel mode, auto-scan mode, deglitch settings,
    and per-channel RCOUNT/SETTLE/CLOCK/OFFSET/DRIVE settings.
  - Validate INTB and STATUS data-ready paths.
  - Validate recovery backoff, optional bus reset, optional hard reset, and
    OFFLINE transition behavior.

## ESP-IDF v6.0.1 Migration Hazards

- Do not use legacy `<driver/i2c.h>` or command-link APIs. New code must use
  `<driver/i2c_master.h>` and declare `esp_driver_i2c`.
- `ESP_ERR_INVALID_RESPONSE` is the new-driver NACK indication; map it
  consistently and keep the numeric detail.
- ESP-IDF components must declare split driver dependencies explicitly.
- The I2C callback timeout is milliseconds for the new master API. Clamp it to
  a finite signed millisecond value and do not pass FreeRTOS ticks by mistake.
- IDF v6 warning-as-error profiles can fail on register packing conversions and
  channel index comparisons. Fix warnings before CI gating.
- Keep bus ownership outside the LDC1614 driver. Recovery callbacks may repair
  the bus, but the driver must not create or destroy the bus internally.
- Keep `recover()` manual and bounded. Do not add background retry loops.

## Implementation Checklist Status

1. Done: root `CMakeLists.txt` registers the core component.
2. Done: `include/` and `src/` have no Arduino include or timing/yield calls.
3. Done: IDF I2C/GPIO adapter uses `<driver/i2c_master.h>` and `driver/gpio.h`.
4. Done: `examples/esp_idf/basic` uses the same shared CLI command contract as
   the Arduino example.
5. Done: PlatformIO native tests and Arduino ESP32-S3/ESP32-S2 builds pass in
   this environment.
6. Pending: ESP-IDF `idf.py set-target esp32s3 build` and `esp32s2 build`
   because `idf.py` was not on PATH in this shell.
7. Pending: hardware validation for identity, single-channel reads, auto-scan,
   INTB, sleep/wake, recovery, and injected I2C failures.
