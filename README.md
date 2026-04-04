# LDC1614 Driver Library

Production-grade LDC1614/LDC1612 multi-channel 28-bit inductance-to-digital
converter I2C driver for ESP32-S2 / ESP32-S3 (Arduino framework, PlatformIO).

## Features

- Injected I2C transport (no Wire dependency in library code)
- Health monitoring with READY / DEGRADED / OFFLINE states
- 4-channel (LDC1614) and 2-channel (LDC1612) support
- Configurable per-channel RCOUNT, settle count, clock dividers, offset, drive current
- Single-channel and multi-channel auto-scan modes
- Internal (~43 MHz) or external reference clock
- Sensor frequency and conversion time calculation helpers
- INTB pin support for data-ready detection
- Comprehensive device status and per-channel error flag parsing

## Installation

### PlatformIO

Add to `platformio.ini`:

```ini
lib_deps =
  LDC1614
```

### Manual

Copy `include/LDC1614/` and `src/` into your project.

## Quick Start

```cpp
#include <Wire.h>
#include "LDC1614/LDC1614.h"

// Transport callbacks
LDC1614::Status i2cWrite(uint8_t addr, const uint8_t* data, size_t len,
                         uint32_t timeoutMs, void* user) {
  TwoWire* wire = static_cast<TwoWire*>(user);
  (void)timeoutMs;
  wire->beginTransmission(addr);
  wire->write(data, len);
  switch (wire->endTransmission(true)) {
    case 0: return LDC1614::Status::Ok();
    case 2: return LDC1614::Status::Error(LDC1614::Err::I2C_NACK_ADDR, "Address NACK");
    case 3: return LDC1614::Status::Error(LDC1614::Err::I2C_NACK_DATA, "Data NACK");
    case 5: return LDC1614::Status::Error(LDC1614::Err::I2C_TIMEOUT, "I2C timeout");
    case 4: return LDC1614::Status::Error(LDC1614::Err::I2C_BUS, "I2C bus error");
    default: return LDC1614::Status::Error(LDC1614::Err::I2C_ERROR, "Write failed");
  }
}

LDC1614::Status i2cWriteRead(uint8_t addr, const uint8_t* tx, size_t txLen,
                             uint8_t* rx, size_t rxLen,
                             uint32_t timeoutMs, void* user) {
  TwoWire* wire = static_cast<TwoWire*>(user);
  (void)timeoutMs;
  wire->beginTransmission(addr);
  wire->write(tx, txLen);
  uint8_t result = wire->endTransmission(false);
  if (result != 0) {
    return LDC1614::Status::Error(
      result == 2 ? LDC1614::Err::I2C_NACK_ADDR :
      result == 3 ? LDC1614::Err::I2C_NACK_DATA :
      result == 5 ? LDC1614::Err::I2C_TIMEOUT :
      result == 4 ? LDC1614::Err::I2C_BUS :
                    LDC1614::Err::I2C_ERROR,
      "Write phase failed");
  }
  if (wire->requestFrom(addr, rxLen) != rxLen) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_ERROR, "Read failed");
  }
  for (size_t i = 0; i < rxLen; ++i) {
    rx[i] = wire->read();
  }
  return LDC1614::Status::Ok();
}

LDC1614::LDC1614 device;

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9);

  LDC1614::Config cfg;
  cfg.i2cWrite = i2cWrite;
  cfg.i2cWriteRead = i2cWriteRead;
  cfg.i2cUser = &Wire;
  cfg.i2cAddress = 0x2A;
  cfg.channelCount = 4;

  // Channel 0 config
  cfg.channel[0].rcount = 0x04D6;
  cfg.channel[0].settleCount = 0x000A;
  cfg.channel[0].finDivider = 1;
  cfg.channel[0].frefDivider = 1;
  cfg.channel[0].idrive = 10;

  auto status = device.begin(cfg);
  if (!status.ok()) {
    Serial.printf("Init failed: %s\n", status.msg);
    return;
  }

  // Device is in sleep mode after begin(). Wake to start conversions.
  device.wake();
  Serial.println("LDC1614 initialized and running!");
}

void loop() {
  device.tick(millis());

  if (device.dataReady()) {
    LDC1614::ChannelData data;
    auto st = device.readChannel(0, data);
    if (st.ok()) {
      Serial.printf("Ch0: 0x%07lX\n", (unsigned long)data.rawData);
    }
  }
}
```

## API Reference

### Lifecycle

| Method | Description |
|--------|-------------|
| `begin(config)` | Initialize driver, verify device identity, apply config. Device remains in sleep mode. |
| `tick(nowMs)` | Cooperative update (currently a no-op; reserved for future use). |
| `end()` | Deinitialize driver, transition to UNINIT. |

### Data Readback

| Method | Description |
|--------|-------------|
| `readChannel(ch, data)` | Read conversion data for a single channel. |
| `readAllChannels(data)` | Read all active channels. |
| `dataReady()` | Check if new conversion data is available (via INTB pin or STATUS register). |

### Control

| Method | Description |
|--------|-------------|
| `sleep()` | Enter sleep mode (stop conversions, retain config). |
| `wake()` | Wake and start conversions. |
| `softReset()` | Reset device to defaults. Requires `begin()` to reinitialize. |

### Runtime Configuration (requires sleep mode)

| Method | Description |
|--------|-------------|
| `setActiveChannel(ch)` | Set active channel for single-channel mode. |
| `setRcount(ch, rcount)` | Set reference count for channel. |
| `setSettleCount(ch, count)` | Set settling reference count. |
| `setClockDividers(ch, fin, fref)` | Set frequency dividers. |
| `setOffset(ch, offset)` | Set conversion offset. |
| `setDriveCurrent(ch, idrive)` | Set sensor drive current (0-31). |
| `readInitIdrive(ch, out)` | Read auto-calibrated INIT_IDRIVE value. |

### Diagnostics

| Method | Description |
|--------|-------------|
| `probe()` | Verify device identity (no health tracking). |
| `recover()` | Attempt recovery by reading MANUFACTURER_ID (tracks health). |

### Health

| Method | Description |
|--------|-------------|
| `state()` | Current `DriverState` (UNINIT/READY/DEGRADED/OFFLINE). |
| `isOnline()` | True if READY or DEGRADED. |
| `consecutiveFailures()` | Count since last success. |
| `totalSuccess()` / `totalFailures()` | Lifetime counters. |
| `lastOkMs()` / `lastErrorMs()` | Timestamps of last events. |
| `lastError()` | Most recent error Status. |

## Examples

- `examples/01_basic_bringup_cli/` - Interactive CLI for LDC1614 features

### Example Helpers (`examples/common/`)

Not part of the library. These simulate project-level glue and keep examples self-contained:

| File | Purpose |
|------|---------|
| `BoardConfig.h` | Pin definitions and Wire init for supported boards |
| `BuildConfig.h` | Compile-time `LOG_LEVEL` configuration |
| `Log.h` | Serial logging macros (`LOGE`/`LOGW`/`LOGI`/`LOGD`/`LOGT`/`LOGV`) |
| `I2cTransport.h` | Wire-based I2C transport adapter |
| `I2cScanner.h` | I2C bus scanner with table output and bus recovery |
| `BusDiag.h` | Bus diagnostics wrapper |
| `CommandHandler.h` | Command parsing helpers (`readLine`, `match`, `parseInt`) |

## Behavioral Contracts

1. **Threading model**: Single-threaded. All calls from one task/loop.
2. **Timing model**: `tick()` is bounded (currently no-op). All I2C blocking.
3. **Resource ownership**: I2C bus and GPIO pins owned by the application. Provided via `Config`.
4. **Memory behavior**: All allocation in `begin()`. Zero heap allocation in steady state.
5. **Error handling**: All fallible APIs return `Status`. No silent failures. No exceptions.

## Documentation

- `CHANGELOG.md` - Full release history
- `LDC1614_inductance_converter_implementation_manual.md` - Device documentation
- `docs/` - Datasheets and application notes

## License

MIT License. See `LICENSE`.
