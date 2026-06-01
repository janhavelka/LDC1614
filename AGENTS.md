# AGENTS.md - LDC1614 Production Embedded Guidelines

## Role and Target
You are a professional embedded software engineer building a production-grade LDC1614/LDC1612 multi-channel inductance-to-digital converter library.

- Target: ESP32-S2 / ESP32-S3, Arduino framework, PlatformIO, and native ESP-IDF component use.
- Goals: deterministic behavior, long-term stability, clean API contracts, portability, no surprises in the field.
- These rules are binding.

---

## Repository Model (Single Library)

```
include/LDC1614/         - Public API headers only (Doxygen)
  CommandTable.h         - Register addresses and bit masks
  Status.h
  Config.h
  LDC1614.h
  Version.h              - Auto-generated (do not edit)
src/                     - Implementation (.cpp)
examples/
  01_*/
  common/                - Example-only helpers (Log.h, BoardConfig.h, I2cTransport.h,
                           I2cScanner.h, CommandHandler.h)
platformio.ini
library.json
README.md
CHANGELOG.md
AGENTS.md
```

Rules:
- `examples/common/` is NOT part of the library. It simulates project glue and keeps examples self-contained.
- No board-specific pins/bus in library code; only in `Config`.
- Public headers only in `include/LDC1614/`.
- Examples demonstrate usage and may use `examples/common/BoardConfig.h`.
- Keep the layout boring and predictable.

Framework-boundary rules:
- Core/public headers and `src/` must remain framework-neutral. Do not include Arduino or ESP-IDF headers there unless the exception is documented in Doxygen and this file.
- Core/public headers and `src/` must not depend on Arduino, Wire, ESP-IDF, FreeRTOS, logging frameworks, global bus objects, framework delays, or heap-heavy framework types.
- Arduino examples may use Arduino APIs.
- ESP-IDF examples must be native IDF examples using `app_main`, `driver/i2c_master.h`, native GPIO/timer/task APIs, and fixed C buffers or `esp_console`/argtable.
- ESP-IDF examples must not include Arduino CLI sources or use `ArduinoCompat`, `IdfArduinoCompat`, `Arduino.h`, `Wire.h`, `String`, `Serial`, `TwoWire`, or equivalent Arduino facades.
- ESP-IDF examples must not depend on shared Arduino-style CLI code unless explicitly documented as a diagnostic exception and guarded by contract checks.
- Keep command parity through repo-local command contracts/checkers, not by compiling Arduino sketch sources into ESP-IDF examples.

---

## Core Engineering Rules (Mandatory)

- Deterministic: no unbounded loops/waits; all timeouts via deadlines, never `delay()` in library code.
- Non-blocking lifecycle: `Status begin(const Config&)`, `void tick(uint32_t nowMs)`, `void end()`.
- Any I/O that can exceed ~1-2 ms must be split into state machine steps driven by `tick()`.
- No heap allocation in steady state (no `String`, `std::vector`, `new` in normal ops).
- No logging in library code; examples may log.
- No macros for constants; use `static constexpr`. Macros only for conditional compile or logging helpers.
- Public APIs are not ISR-safe unless explicitly documented and proven.
- Driver instances are not internally thread-safe unless explicitly protected and documented.
- Conversion timing, settling timing, and sensor-frequency calculations must be documented and tested against configured counts and dividers.

---

## I2C Manager + Transport (Required)

- The library MUST NOT own I2C. It never touches `Wire` directly.
- `Config` MUST accept a transport adapter (function pointers or abstract interface).
- Transport errors MUST map to `Status` (no leaking `Wire`, `esp_err_t`, etc.).
- The library MUST NOT configure bus timeouts or pins.
- Bus ownership, locking, timeout policy, and recovery policy belong to the application or transport adapter.
- Transport callbacks are non-owning injections; the library must not retain ownership of framework bus objects.

---

## Status / Error Handling (Mandatory)

All fallible APIs return `Status`:

```cpp
struct Status {
  Err code;
  int32_t detail;
  const char* msg;  // static string only
};
```

- Silent failure is unacceptable.
- No exceptions.
- Public APIs that write device state must report failed register writes precisely.
- Multi-register configuration updates must avoid, report, or recover from partial hardware state.

---

## LDC1614 Driver Requirements

- I2C address configurable: 0x2A (ADDR->GND), 0x2B (ADDR->VDD).
- Check device presence in `begin()` by reading MANUFACTURER_ID and DEVICE_ID registers.
- Support 4 channels (LDC1614) or 2 channels (LDC1612) with configurable channel count.
- LDC1612 versus LDC1614 variant differences must be explicit. Channels 2 and 3 are LDC1614-only.
- Per-channel configurable: RCOUNT, SETTLECOUNT, CLOCK_DIVIDERS, OFFSET, DRIVE_CURRENT.
- Configurable operating modes:
  - **Single-channel continuous**: one channel converting continuously.
  - **Multi-channel sequential (auto-scan)**: round-robin across selected channels.
- Configurable deglitch filter bandwidth (1.0, 3.3, 10, 33 MHz).
- Sleep mode / active mode transitions via CONFIG register.
- Software reset via RESET_DEV register.
- 28-bit conversion result readout with proper MSB-first coherency.
- DATAx coherency must follow datasheet ordering: read DATAx_MSB before DATAx_LSB.
- Sensor frequency calculation from raw data, reference clock, dividers, and offset.
- Status register readout with error flag parsing.
- Error reporting configuration (under-range, over-range, watchdog, amplitude, zero count).
- INTB pin support for data-ready and error notification.

---

## Driver Architecture: Managed Synchronous Driver

The driver follows a **managed synchronous** model with health tracking:

- All public I2C operations are **blocking** (no complex async - LDC1614 has no EEPROM/NVM writes).
- `tick()` may be used for polling data ready status.
- Health is tracked via **tracked transport wrappers** -- public API never calls `_updateHealth()` directly.
- Recovery is **manual** via `recover()` - the application controls retry strategy.

### DriverState (4 states only)

```cpp
enum class DriverState : uint8_t {
  UNINIT,    // begin() not called or end() called
  READY,     // Operational, consecutiveFailures == 0
  DEGRADED,  // 1 <= consecutiveFailures < offlineThreshold
  OFFLINE    // consecutiveFailures >= offlineThreshold
};
```

State transitions:
- `begin()` success -> READY
- Any I2C failure in READY -> DEGRADED
- Success in DEGRADED/OFFLINE -> READY
- Failures reach `offlineThreshold` -> OFFLINE
- `end()` -> UNINIT

### Transport Wrapper Architecture

All I2C goes through layered wrappers:

```
Public API (readChannel, readStatus, etc.)
    ↓
Register helpers (readRegister16, writeRegister16)
    ↓
TRACKED wrappers (_i2cWriteReadTracked, _i2cWriteTracked)
    ↓  <- _updateHealth() called here ONLY
RAW wrappers (_i2cWriteReadRaw, _i2cWriteRaw)
    ↓
Transport callbacks (Config::i2cWrite, i2cWriteRead)
```

**Rules:**
- Public API methods NEVER call `_updateHealth()` directly
- `readRegister16()`/`writeRegister16()` use TRACKED wrappers -> health updated automatically
- `probe()` uses RAW wrappers -> no health tracking (diagnostic only)
- `recover()` tracks probe failures (driver is initialized, so failures count)

### Health Tracking Rules

- `_updateHealth()` called ONLY inside tracked transport wrappers.
- State transitions guarded by `_initialized` (no DEGRADED/OFFLINE before `begin()` succeeds).
- NOT called for config/param validation errors (INVALID_CONFIG, INVALID_PARAM).
- NOT called for precondition errors (NOT_INITIALIZED).
- `probe()` uses raw I2C and does NOT update health (diagnostic only).

### Health Tracking Fields

- `_lastOkMs` - timestamp of last successful I2C operation
- `_lastErrorMs` - timestamp of last failed I2C operation
- `_lastError` - most recent error Status
- `_consecutiveFailures` - failures since last success (resets on success)
- `_totalFailures` / `_totalSuccess` - lifetime counters (wrap at max)

---

## Versioning and Releases

Single source of truth: `library.json`. `Version.h` is auto-generated and must never be edited.

SemVer:
- MAJOR: breaking API/Config/enum changes.
- MINOR: new backward-compatible features or error codes (append only).
- PATCH: bug fixes, refactors, docs.

Release steps:
1. Update `library.json`.
2. Update `CHANGELOG.md` (Added/Changed/Fixed/Removed).
3. Update `README.md` if API or examples changed.
4. Commit and tag: `Release vX.Y.Z`.

---

## Validation Claims

- Do not claim hardware validation unless real LDC1614/LDC1612 hardware logs were captured in the current validation context.
- Separate software readiness from hardware validation. INTB behavior, SD-pin behavior, fault injection, channel sequencing on real sensors, coil/sensor operating limits, address-pin variants, and long soak/HIL stress require physical evidence.
- Documentation must label examples honestly: diagnostic bring-up examples are not production bus managers unless they demonstrate application-owned locking, timeout policy, and recovery policy.

---

## Naming Conventions

- Member variables: `_camelCase`
- Methods/Functions: `camelCase`
- Constants: `CAPS_CASE`
- Enum values: `CAPS_CASE`
- Locals/params: `camelCase`
- Config fields: `camelCase`
