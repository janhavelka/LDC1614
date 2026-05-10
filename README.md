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
- INTB pin and STATUS polling support for data-ready detection
- Comprehensive device status and per-channel error flag parsing
- Runtime setters for mode, autoscan sequence, deglitch, error reporting, INTB, clock source, drive policy, and per-channel timing
- Manual recovery ladder with backoff, optional bus reset, optional soft reset, and optional hard reset callbacks

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

  bool ready = false;
  auto readyStatus = device.readDataReady(ready);
  if (!readyStatus.ok()) {
    Serial.printf("DRDY check failed: %s\n", readyStatus.msg);
    return;
  }

  if (ready) {
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
| `isInitialized()` | Return `true` after successful `begin()` until `end()`. |
| `getConfig()` | Return the cached configuration snapshot currently held by the driver. |

### Data Readback

| Method | Description |
|--------|-------------|
| `readChannel(ch, data)` | Read conversion data for a single channel. |
| `readAllChannels(data, count)` | Read channels `0..count-1`; with `count=0`, reads the configured `channelCount`. |
| `readChannelBlocking(ch, data, timeoutMs)` | Wait for DRDY and then read one channel with a bounded timeout. |
| `readAllChannelsBlocking(data, timeoutMs, count)` | Wait for DRDY and then call `readAllChannels()` with a bounded timeout. |
| `readDataReady(ready)` | Check DRDY with explicit `Status` reporting. Uses INTB if configured and enabled; otherwise polls STATUS. |
| `dataReady()` | Convenience wrapper around `readDataReady()`. Returns `false` if the STATUS/INTB path fails. |

`readDeviceStatus()`, `readStatusRaw()`, and STATUS-based data-ready polling read the device STATUS register. Per the device behavior, that read can clear sticky status flags and de-assert INTB.

### Sample Cache

| Method | Description |
|--------|-------------|
| `getLastSample(ch, data)` | Return the last successfully read sample for a channel without I2C. |
| `sampleTimestampMs(ch)` | Timestamp of the last successful sample for a channel (`0` if none). |
| `sampleAgeMs(ch, nowMs)` | Age of the cached sample for a channel. |
| `isMeasuring()` | True when the device is awake and conversions are running. |

### Control

| Method | Description |
|--------|-------------|
| `sleep()` | Enter sleep mode (stop conversions, retain config). |
| `wake()` | Wake and start conversions. |
| `softReset()` | Reset device to defaults. Requires `begin()` to reinitialize. |
| `resetAndReapply()` | Reset the device and re-apply the cached configuration, returning to READY on success. |

### Runtime Configuration (requires sleep mode)

| Method | Description |
|--------|-------------|
| `setActiveChannel(ch)` | Set active channel for single-channel mode. |
| `setSingleChannelMode(ch)` | Disable autoscan and select the active single channel. |
| `setAutoScanMode(sequence)` | Enable autoscan using `CH0_CH1`, `CH0_CH1_CH2`, or `CH0_CH1_CH2_CH3`. |
| `setDeglitch(deglitch)` | Set the input deglitch filter bandwidth. |
| `setErrorConfig(mask)` / `getErrorConfig()` | Set/read cached `ERROR_CONFIG`. Reserved bits are rejected. |
| `setIntbDisabled(disabled)` | Enable or disable INTB output in `CONFIG.INTB_DIS`. |
| `setReferenceClockSource(source)` | Select internal oscillator or external CLKIN. |
| `setSensorActivation(activation)` | Select full-current or low-power sensor activation. |
| `setRpOverrideEnabled(enabled)` | Enable/disable fixed drive current override. |
| `setAutoAmplitudeCorrectionEnabled(enabled)` | Enable/disable automatic amplitude correction. |
| `setHighCurrentDriveEnabled(enabled)` | Enable high-current drive. Valid only for single-channel Ch0. |
| `setRcount(ch, rcount)` | Set reference count for channel. |
| `setSettleCount(ch, count)` | Set settling reference count. |
| `setClockDividers(ch, fin, fref)` | Set frequency dividers. |
| `setOffset(ch, offset)` | Set conversion offset. |
| `setDriveCurrent(ch, idrive)` | Set sensor drive current (0-31). |
| `readInitIdrive(ch, out)` | Read auto-calibrated INIT_IDRIVE value. |

Runtime setters require the device to be in sleep mode. Call `sleep()`, apply the changes, then call `wake()` when conversions should resume. Cached configuration is committed only after the corresponding register write succeeds.

### Diagnostics

| Method | Description |
|--------|-------------|
| `probe()` | Verify device identity (no health tracking). |
| `recover()` | Manual recovery ladder. Uses tracked identity reads, then optional bus reset, optional soft reset/reapply, and optional hard reset/reapply. |
| `readRegister16()` / `writeRegister16()` | Raw tracked register access for diagnostics and service operations. Valid addresses are `0x00`-`0x1C`, `0x1E`-`0x21`, `0x7E`, and `0x7F`. |
| `readDeviceStatus()` / `readStatusRaw()` | Parsed or raw STATUS register access. |
| `getSettings()` | Return a RAM-only snapshot of active settings and cached sample timestamps. |

`recover()` honors `Config::recoverBackoffMs` and validates both `MANUFACTURER_ID` and `DEVICE_ID` before reporting success. Transport failures update health counters; `probe()` is intentionally raw and does not affect health.

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

The bringup CLI includes raw `reg` / `wreg` commands for diagnostics. Invalid
register addresses are rejected before I2C, but valid diagnostic writes can still
desynchronize the cached configuration until a fresh `begin()` or `resetAndReapply()`.

The CLI also exposes runtime configuration commands for the driver features:
`single`, `autoscan`, `deglitch`, `errcfg`, `intb`, `refclk`, `activate`,
`rpoverride`, `autoamp`, `highcurrent`, `rcount`, `settle`, `clkdiv`,
`offset`, `idrive`, and `initidrive`.

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
| `CliShell.h` | Serial command-line shell with line editing |
| `CliStyle.h` | Shared CLI color and help formatting helpers |
| `CommandHandler.h` | Command parsing helpers (`readLine`, `match`, `parseInt`) |
| `HealthDiag.h` | Verbose driver-health diagnostics and snapshots |
| `HealthView.h` | Compact health/status formatting helpers |
| `TransportAdapter.h` | Function-pointer adapter for example transports |

## Behavioral Contracts

1. **Threading model**: Single-threaded. All calls from one task/loop.
2. **Timing model**: `tick()` is bounded (currently no-op). Blocking read waits use deadlines and a finite poll cap, so a stalled injected clock cannot spin forever.
3. **Resource ownership**: I2C bus and GPIO pins owned by the application. Provided via `Config`.
4. **Memory behavior**: All allocation in `begin()`. Zero heap allocation in steady state.
5. **Error handling**: All fallible APIs return `Status`. No silent failures. No exceptions.
6. **Health behavior**: `OFFLINE` is latched. Normal public I2C operations return `BUSY` with `Driver is offline; call recover()` without touching the bus until `recover()` succeeds.

## Configuration Constraints

| Setting | Constraint |
|---------|------------|
| `i2cWrite`, `i2cWriteRead` | Required. The library never touches `Wire` directly. |
| `i2cAddress` | `0x2A` or `0x2B`. |
| `channelCount` | `2` for LDC1612 or `4` for LDC1614. |
| Channel indexes | Must be less than `channelCount`. |
| `rrSequence` | LDC1612 accepts only `CH0_CH1`; LDC1614 accepts all defined sequences. |
| `deglitch` | Must be one of 1 MHz, 3.3 MHz, 10 MHz, or 33 MHz. |
| `errorConfig` | Only `cmd::MASK_ERRCFG_*` bits in `cmd::MASK_ERRCFG_ALLOWED` may be set. |
| INTB | If `intbPin >= 0`, `gpioRead` is required. |
| `highCurrentDrv` | Valid only in single-channel mode on Ch0. |
| Recovery | `recoverBackoffMs` gates repeated `recover()` attempts; bus/hard reset callbacks are optional. |

## Documentation

- `CHANGELOG.md` - Full release history
- `LDC1614_inductance_converter_implementation_manual.md` - Device documentation
- `docs/` - Datasheets and application notes

## License

MIT License. See `LICENSE`.
