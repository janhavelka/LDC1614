# LDC1614 ESP-IDF v6.0.1 Port Audit

Last audited: 2026-05-17

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
  `I2cScanner.h`, `BoardConfig.h`, and the CLI example.

Readiness verdict: the driver core is framework-neutral and IDF component/example
scaffolding is present. Final readiness still requires an ESP-IDF 6.0.1 build
and hardware validation.

## Portability Blockers

- ESP-IDF compilation has not been verified in this shell because `idf.py` was
  unavailable.
- Hardware validation remains outstanding.
- INTB support depends on `Config::gpioRead`; IDF examples must configure GPIO
  externally and pass a callback.
- Recovery support can call `Config::busReset` and `Config::hardReset`; IDF
  implementations must keep those bounded and must not silently reconfigure bus
  ownership inside the core driver.
- Arduino examples use `Serial`, `String`, `Wire`, `millis()`, `delay()`, and
  `yield()`; keep them Arduino-only.
- IDF v6.0.1 warning profiles can expose implicit conversions in register field
  packing, channel indexing, and floating-point utility calculations.

## Exact Files/APIs To Change Later

- `src/LDC1614.cpp`
  - Remove the unconditional `#include <Arduino.h>`.
  - Keep `_i2cWriteReadRaw()`, `_i2cWriteRaw()`,
    `_i2cWriteReadTracked()`, and `_i2cWriteTracked()` as the only transport
    path.
  - Replace `_nowMs()` and `_cooperativeYield()` fallbacks with a portability
    boundary:
    - Arduino build: may call `millis()` and `yield()`.
    - ESP-IDF build: use `Config::nowMs` and `Config::cooperativeYield`, or
      guarded defaults using `esp_timer_get_time()` and `taskYIELD()`.
  - Do not add direct `i2c_master_*` calls to register helpers.
  - Keep `recover()` as a manual recovery ladder; do not add automatic retry
    loops.
- `include/LDC1614/Config.h`
  - Preserve existing callback signatures, including `BusResetFn` and
    `HardResetFn`.
  - Document that pure IDF users should set `nowMs`, `cooperativeYield`, and
    `gpioRead` when INTB is used.
  - Do not include IDF driver headers in the public core header.
- `include/LDC1614/LDC1614.h`
  - Preserve namespace, class name, enums, `Status`, sample cache APIs,
    recovery APIs, and health APIs.
- Add root `CMakeLists.txt`.
- Add IDF-only adapter/example files under a new path such as
  `examples/esp_idf/basic/`.
- Do not edit Arduino examples/common helpers except for separate Arduino
  regression fixes during the implementation PR.

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
  REQUIRES esp_timer freertos
)
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
```

If an IDF adapter is shipped inside the component, include its source and add
`PRIV_REQUIRES esp_driver_i2c esp_driver_gpio esp_timer freertos`. If the
adapter lives only in the example, put those requirements in the example
component instead.

## Example Plan

- Keep the existing Arduino CLI example as the Arduino reference.
- Add `examples/esp_idf/basic`:
  - create an I2C master bus with `i2c_new_master_bus()`;
  - add the LDC1614 device with `i2c_master_bus_add_device()` at `0x2A`;
  - fill `LDC1614::Config` with IDF callbacks, channel count, channel
    parameters, `nowMs`, `cooperativeYield`, and optional INTB/hard-reset hooks;
  - call `begin()`;
  - log MANUFACTURER_ID, DEVICE_ID, STATUS, one channel read, and health
    counters.
- Add a second IDF example only after basic success:
  - configure auto-scan across 2 or 4 channels;
  - use INTB data-ready via `gpioRead`;
  - demonstrate bounded `readAllChannelsBlocking()`.

## Test/Validation Plan

- Static checks:
  - `rg "<Arduino.h>|<Wire.h>|millis\\(|delay\\(|yield\\(" include src`
    should find no unguarded Arduino dependencies in the ESP-IDF build path.
  - `rg "driver/i2c.h|i2c_cmd_link|i2c_driver_install" .` should not find
    legacy I2C driver usage in IDF code.
- Arduino regression:
  - `pio test -e native`
  - `pio run -e esp32s3dev`
  - `pio run -e esp32s2dev`
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

## Ordered Implementation Checklist

1. Add the root `CMakeLists.txt` for the core component.
2. Remove or compile-guard the Arduino include and timing/yield fallbacks in
   `src/LDC1614.cpp`.
3. Build the core component under IDF with callback stubs.
4. Add the IDF I2C/GPIO adapter using `<driver/i2c_master.h>` and
   `driver/gpio.h`.
5. Add `examples/esp_idf/basic` and build for ESP32-S3.
6. Build the same example for ESP32-S2.
7. Run PlatformIO native and Arduino example builds as regression checks only.
8. Validate identity, single-channel reads, auto-scan, INTB, sleep/wake, and
   recovery on hardware.
9. Inject I2C failures and verify status/health/recovery behavior.
10. Add final `espidf` metadata/build matrix coverage and keep generated
    `Version.h` synchronized with `library.json`.
11. Add optional IDF component manifest only after both Arduino and IDF builds
    pass.
