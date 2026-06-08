# LDC1614 Driver Library

Framework-neutral LDC1614/LDC1612 multi-channel 28-bit inductance-to-digital
converter I2C driver core for ESP32-S2 / ESP32-S3 integration through Arduino
framework, PlatformIO, or native ESP-IDF component use. The architecture is
production-oriented, but deployment readiness depends on application hardware,
calibration, fault testing, and captured validation logs.

## Features

- Injected I2C transport (no Wire dependency in library code)
- Framework-neutral core (`include/` and `src/` do not include Arduino or ESP-IDF driver headers)
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

### ESP-IDF

The repository root is an ESP-IDF component. Add it through `EXTRA_COMPONENT_DIRS`
or component manager metadata, then provide `Config::i2cWrite`,
`Config::i2cWriteRead`, `Config::nowMs`, optional `Config::cooperativeYield`,
optional `Config::gpioRead`, and optional recovery callbacks from your
application-owned adapter. Production applications own bus lifecycle, locking,
timeouts, task scheduling, recovery/backoff policy, GPIO/INTB integration, and
hardware validation. A native ESP-IDF diagnostic bring-up CLI using the
`driver/i2c_master.h` API is in `examples/esp_idf/basic`; it is not a production
bus manager.

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
  cfg.nowMs = [](void*) { return millis(); };
  cfg.cooperativeYield = [](void*) { yield(); };
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

The numeric channel values above are example-sensor placeholders. Derive
RCOUNT, SETTLECOUNT, CLOCK_DIVIDERS, OFFSET, and IDRIVE from the sensor
frequency, Q/Rp, reference clock, target amplitude, and application timing
requirements. IDRIVE and coil behavior require board-level evidence; the
library does not calibrate distance, coating thickness, material identity, or
inductance for an application.

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
| `readAllChannels(data, count)` | Read channels `0..count-1`; with `count=0`, reads the configured `channelCount`. Returns latest register values, not guaranteed-fresh autoscan samples for every channel. |
| `readFreshChannels(data, count)` | Read STATUS once, then read only channels with `UNREADCONVx` set. Reports per-channel `fresh` / `valid` flags and cached stale samples where available. The overload with `DeviceStatus&` returns the STATUS snapshot that drove freshness. |
| `readChannelBlocking(ch, data, timeoutMs)` | Require `Config::nowMs`, wait for DRDY with a wall-clock timeout, then read one channel. |
| `readAllChannelsBlocking(data, timeoutMs, count)` | Require `Config::nowMs`, wait for one DRDY event, then call `readAllChannels()`. |
| `readDataReady(ready)` | Check DRDY with explicit `Status` reporting. Uses INTB if configured and enabled; otherwise polls STATUS. If STATUS contains DRDY plus sensor error flags, `ready` is true and the returned status is `SENSOR_ERROR`. |
| `dataReady()` | Convenience wrapper around `readDataReady()`. `false` can mean not ready, not initialized, OFFLINE/BUSY, or hidden transport/status/sensor failure. |

`readDeviceStatus()`, `readStatusRaw()`, and STATUS-based data-ready polling read the device STATUS register. Per the device behavior, that read can clear sticky status flags and de-assert INTB.

`readChannel()` reads `DATAx_MSB` before `DATAx_LSB` and masks the 28-bit
conversion value. The upper DATAx_MSB bits can report under-range, over-range,
watchdog, and amplitude error flags when the corresponding ERROR_CONFIG
`*_ERR2OUT` bits are enabled. The amplitude flag is the high/low amplitude
condition collapsed into `ChannelData::errAmplitude`; zero-count is reported via
STATUS/INTB, not DATAx_MSB.

In autoscan mode, `DRDY` means the selected conversion sequence reached its
documented data-ready condition; it is not a per-channel freshness bitmap.
`readAllChannels()` intentionally preserves backward-compatible latest-register
semantics. Production autoscan loops that must avoid stale channel data should
use `readDeviceStatus()` / `readFreshChannels()` and the `UNREADCONVx` bits,
then process only entries where `FreshChannelData::fresh` is true. Entries with
`valid=true` and `fresh=false` are cached samples from an earlier successful
read, not new conversions. Use the `readFreshChannels(..., DeviceStatus&, ...)`
overload when STATUS error flags must be captured before the STATUS read clears
sticky flags or de-asserts INTB.

Blocking helpers validate parameters and `Config::nowMs` before polling,
touching I2C, or calling `cooperativeYield`. Without a monotonic `nowMs`
callback, they return `INVALID_CONFIG`; nonblocking reads, status checks, and
cache access remain available. `timeoutMs` bounds the readiness wait. The final
DATAx readout still uses injected I2C transaction timeouts and bounded
application callback latency.

### Sample Cache

| Method | Description |
|--------|-------------|
| `getLastSample(ch, data)` | Return the last successfully read sample for a channel without I2C. |
| `sampleTimestampMs(ch)` | Timestamp of the last successful sample for a channel. Check `hasSample(ch)` first; timestamp `0` can be a valid sample time when no timebase is configured or the monotonic clock is at zero. |
| `sampleAgeMs(ch, nowMs)` | Age of the cached sample for a channel. |
| `isMeasuring()` | True when the device is awake and conversions are running. |

### Control

| Method | Description |
|--------|-------------|
| `sleep()` | Enter sleep mode (stop conversions, retain config). |
| `wake()` | Wake and start conversions. |
| `softReset()` | Reset device to defaults. Requires `begin()` to reinitialize. |
| `resetAndReapply()` | Reset the device and re-apply the cached configuration, returning to READY on success. |
| `syncConfig()` | Re-apply the cached configuration without a reset. Forces CONFIG sleep first, then applies channel/global registers, and leaves the device in sleep mode on success. |

### Runtime Configuration (requires sleep mode)

| Method | Description |
|--------|-------------|
| `setActiveChannel(ch)` | Set active channel for single-channel mode. |
| `setSingleChannelMode(ch)` | Disable autoscan and select the active single channel. |
| `setAutoScanMode(sequence)` | Enable autoscan using `CH0_CH1`, `CH0_CH1_CH2`, or `CH0_CH1_CH2_CH3`. Selected channels must meet the datasheet Table 43 multi-channel minima. |
| `setDeglitch(deglitch)` | Set the input deglitch filter bandwidth. |
| `setErrorConfig(mask)` / `getErrorConfig()` | Set/read cached `ERROR_CONFIG`. Reserved bits are rejected. |
| `setIntbDisabled(disabled)` | Enable or disable INTB output in `CONFIG.INTB_DIS`. |
| `setReferenceClockSource(source)` | Select internal oscillator or external CLKIN. |
| `setSensorActivation(activation)` | Select full-current or low-power sensor activation. |
| `setRpOverrideEnabled(enabled)` | Enable/disable fixed drive current override. |
| `setAutoAmplitudeCorrectionEnabled(enabled)` | Enable/disable automatic amplitude correction. |
| `setHighCurrentDriveEnabled(enabled)` | Enable high-current drive. Valid only for single-channel Ch0. |
| `setRcount(ch, rcount)` | Set reference count for channel. Autoscan selected channels require `>=0x0009`. |
| `setSettleCount(ch, count)` | Set settling reference count. Autoscan selected channels require `>=0x0004`. |
| `setClockDividers(ch, fin, fref)` | Set frequency dividers. Register-field ranges are enforced; physical clock-plan limits remain application-owned. |
| `setOffset(ch, offset)` | Set conversion offset. |
| `setDriveCurrent(ch, idrive)` | Set sensor drive current (0-31). |
| `readInitIdrive(ch, out)` | Read auto-calibrated INIT_IDRIVE value. |

Runtime setters require the device to be in sleep mode. Call `sleep()`, apply the changes, then call `wake()` when conversions should resume. Cached configuration is committed only after the corresponding register write succeeds. Full apply paths (`begin()`, `syncConfig()`, recovery reapply, and `resetAndReapply()`) write a sleep-mode CONFIG image before channel/global registers so reconfiguration is performed while asleep.

### Diagnostics

| Method | Description |
|--------|-------------|
| `probe()` | Verify device identity (no health tracking). |
| `recover()` | Manual recovery ladder. Uses tracked identity reads, then optional bus reset, optional soft reset/reapply, and optional hard reset/reapply. |
| `readRegister16()` / `writeRegister16()` | Diagnostic-only tracked register access. Valid addresses are `0x00`-`0x1C`, `0x1E`-`0x21`, `0x7E`, and `0x7F`. Raw access is not variant/access-type safe. |
| `readDeviceStatus()` / `readStatusRaw()` | Parsed or raw STATUS register access. |
| `getSettings(snap)` | Populate an expanded RAM-only snapshot of active settings, hook presence, cached samples, timestamps, and health. |
| `settings()` | Return the same snapshot by value for compact diagnostics. |
| `driverState()` | Cross-library alias for the current `DriverState`. |
| `hasSample(ch)` | Check whether a configured channel has a cached sample. |
| `calcSettleTimeUs(ch, fRef)` / `calcSampleTimeUs(ch, fRef)` | Calculate configured settling and conversion-plus-settling timing for service diagnostics. |
| `calcSensorFrequency(ch, rawData, fRef)` / `calcConversionTimeUs(ch, fRef)` | Calculate sensor frequency and conversion timing from raw data, reference clock, dividers, and offset. Frequency is not calibrated inductance or distance. |

`probe()` is intentionally raw and does not affect health. It requires configured
transport callbacks; a fresh default instance returns `INVALID_CONFIG` because no
transport has been supplied.

`recover()` honors `Config::recoverBackoffMs` and validates both `MANUFACTURER_ID`
and `DEVICE_ID` before reporting success. Transport failures update health
counters. If `hardwareConfigDirty()` is true, recovery also re-applies the cached
configuration before returning success.

### Health

| Method | Description |
|--------|-------------|
| `state()` | Current `DriverState` (UNINIT/READY/DEGRADED/OFFLINE). |
| `isOnline()` | True if READY or DEGRADED. |
| `consecutiveFailures()` | Count since last success. |
| `totalSuccess()` / `totalFailures()` | Lifetime counters. |
| `lastOkMs()` / `lastErrorMs()` | Timestamps of last events. |
| `lastError()` | Most recent error Status. |
| `hardwareConfigDirty()` | True when cached configuration may not match hardware. |
| `hardwareConfigDirtyError()` | First status/detail that made the dirty state true. |

## Examples

- `examples/01_basic_bringup_cli/` - Arduino diagnostic bring-up CLI for
  exercising LDC1614 features. This is diagnostic firmware, not a production
  bus manager or field-readiness certificate.
- `examples/esp_idf/basic/` - Native ESP-IDF diagnostic bring-up CLI with
  `driver/i2c_master.h` transport glue. This example is diagnostic bring-up
  code; production applications own lifecycle, locking, recovery, and hardware
  validation.

The Arduino bring-up example uses the shared example-only command implementation
in `examples/common/Ldc1614Cli.cpp`. The ESP-IDF example uses its own
fixed-buffer native parser so the IDF compile path does not include
`std::string`, Arduino `String`, `Arduino.h`, `Wire.h`, `Serial`, or Arduino
facades. These CLIs are diagnostic examples, not production bus-management
templates.

The CLI includes raw `reg` / `wreg` commands for diagnostics. Invalid register
addresses are rejected before I2C, but valid diagnostic writes mark
`hardwareConfigDirty()` because they can desynchronize the cached configuration.
Use `syncConfig()`, `recover()`, `resetAndReapply()`, or a fresh `begin()` before
trusting cached configuration-dependent behavior again.

The CLI also exposes runtime configuration commands for the driver features:
`single`, `autoscan`, `deglitch`, `errcfg`, `intb`, `refclk`, `activate`,
`rpoverride`, `autoamp`, `highcurrent`, `rcount`, `settle`, `clkdiv`,
`offset`, `idrive`, and `initidrive`. Additional service commands include
`begin` / `init` for reinitialization, `id` for manufacturer/device identity,
`demo [N]`, `selftest`, `stress [N]`, `stress_mix [N]`, and
`timing <ch> <fRef>` for conversion, settling, and total sample-time
calculations.

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
| `Ldc1614Cli.h/.cpp` | Shared example-only diagnostic command implementation used by the Arduino bring-up example |
| `CommandHandler.h` | Command parsing helpers (`readLine`, `match`, `parseInt`) |
| `HealthDiag.h` | Verbose driver-health diagnostics and snapshots |
| `HealthView.h` | Compact health/status formatting helpers |
| `TransportAdapter.h` | Function-pointer adapter for example transports |

## Behavioral Contracts

1. **Threading model**: Instances are not internally thread-safe, and public APIs are not ISR-safe. Serialize all driver calls and I2C access in the application or injected transport. Transport callbacks must not recursively call into the same driver instance.
2. **Timing model**: `tick()` is bounded (currently no-op). Blocking read waits require a monotonic `Config::nowMs`, use explicit deadlines, and keep a saturated finite poll cap so a stalled injected clock cannot spin forever.
3. **Resource ownership**: I2C bus, locking/serialization, GPIO pins, timeouts, task ownership, INTB integration, and recovery/backoff policy are owned by the application or injected transport. Provided via `Config`.
4. **Framework boundary**: Core code does not call `Wire`, `Serial`, `delay()`, `yield()`, `millis()`, ESP-IDF, FreeRTOS, or logging APIs directly. Arduino and ESP-IDF examples provide those hooks externally.
5. **Memory behavior**: All allocation in `begin()`. Zero heap allocation in steady state.
6. **Error handling**: All fallible APIs return `Status`. No silent failures. No exceptions.
7. **Health behavior**: `OFFLINE` is latched. Normal public I2C operations return `BUSY` with `Driver is offline; call recover()` without touching the bus until `recover()` succeeds.
8. **Dirty hardware config behavior**: failed configuration writes and diagnostic raw writes can leave hardware and cache diverged. Check `hardwareConfigDirty()` / `hardwareConfigDirtyError()`, stop trusting cached configuration, then call `syncConfig()`, `recover()`, `resetAndReapply()`, or `begin()`. Trust the cache again only after dirty state is clear.

## Configuration Constraints

| Setting | Constraint |
|---------|------------|
| `i2cWrite`, `i2cWriteRead` | Required. The library never touches `Wire` directly. |
| `i2cAddress` | `0x2A` or `0x2B`. |
| `i2cTimeoutMs` | Passed through to the injected transport; the core does not configure bus hardware timeouts. |
| `nowMs` | Required for wall-clock blocking reads and recovery backoff timing. Nonblocking reads and status APIs can run without it, but timestamps are then `0`. |
| `cooperativeYield` | Optional application callback between blocking-read polls. It must be bounded and must not recursively call into the same driver instance. |
| `channelCount` | `2` for LDC1612 or `4` for LDC1614. |
| Channel indexes | Must be less than `channelCount`. |
| `rrSequence` | LDC1612 accepts only `CH0_CH1`; LDC1614 accepts all defined sequences. |
| `RCOUNTx` | Register value must be `0x0005..0xFFFF`; channels selected by autoscan require `>=0x0009` per datasheet Table 43. |
| `SETTLECOUNTx` | Single-channel mode accepts the register-defined values; channels selected by autoscan require `>=0x0004` per datasheet Table 43. |
| `FIN_DIVIDERx` | Register field must be `1..15`. The application clock plan must choose `>=2` when actual sensor frequency is `>=8.75 MHz`. |
| `FREF_DIVIDERx` | Register field must be `1..1023`; reserved bits in CLOCK_DIVIDERSx are kept clear. |
| Physical clock plan | The driver does not know actual `fCLK`, `fREFx`, sensor frequency, coil tolerance, or deglitch margin. Validate `fINx < fREFx/4`, external/internal clock accuracy, 1 kHz to 10 MHz sensor range, and deglitch selection on the target board. |
| `deglitch` | Must be one of 1 MHz, 3.3 MHz, 10 MHz, or 33 MHz. |
| `errorConfig` | Only `cmd::MASK_ERRCFG_*` bits in `cmd::MASK_ERRCFG_ALLOWED` may be set. |
| INTB | If `intbPin >= 0`, `gpioRead` is required. |
| `highCurrentDrv` | Valid only in single-channel mode on Ch0. |
| Full configuration apply | `begin()`, `syncConfig()`, `recover()` reapply, and `resetAndReapply()` force CONFIG sleep before writing channel/global configuration and leave the device asleep. |
| Recovery | `recoverBackoffMs` gates repeated `recover()` attempts; bus/hard reset callbacks are optional. |

## API Latency and Transaction Model

Notation: `R` = 16-bit register read transaction, `W` = 16-bit register write
transaction, `N` = configured channel count/effective `count`, `F` = channels
with `UNREADCONVx` set, `P` = readiness poll count until ready/timeout, and
`T` = injected per-transaction timeout (`Config::i2cTimeoutMs`). Callback
latency for `gpioRead`, `busReset`, `hardReset`, and `cooperativeYield` is
application-owned and must be bounded by the injected implementation.

| API | I2C transactions | Other waits | Bound / notes |
| --- | ---: | --- | --- |
| `begin()` | `2R + N*5W + 4W` | none in core | Probe identity, force CONFIG sleep, apply channel registers plus ERROR_CONFIG/MUX_CONFIG/final CONFIG. Leaves device asleep. |
| `probe()` | `2R` | none | Raw identity reads, no health tracking. Requires configured callbacks. |
| `recover()` | `1R..2R` before optional recovery steps | optional bus/hard reset callbacks | Identity failure can return after MANUFACTURER_ID. May add bus reset, RESET_DEV write, hard reset, and full config reapply. Backoff depends on `nowMs` when configured. |
| `readChannel(ch)` | `2R` | none | DATAx_MSB then DATAx_LSB. |
| `readAllChannels(out, N)` | `2N R` | none | Latest-register semantics; not per-channel freshness proof. |
| `readFreshChannels(out, N)` | `1R + 2F R` | none | `F` is channels with `UNREADCONVx` set. Non-fresh channels return cached data if available. |
| `readDataReady(ready)` | `0R` or `1R` | optional `gpioRead` | INTB high path uses no I2C; polling or asserted INTB reads STATUS. STATUS sensor errors return `SENSOR_ERROR`, with `ready` still reflecting DRDY. |
| `dataReady()` | `0R` or `1R` | optional `gpioRead` | Convenience only; false can mean not ready or hidden transport/status/sensor error. |
| `readDeviceStatus()` / `readStatusRaw()` | `1R` | none | STATUS read can clear sticky status and de-assert INTB. |
| `sleep()` / `wake()` | `0W` or `1W` | none | No write if already in requested state. |
| `softReset()` | `1W` | none in core | Writes RESET_DEV and transitions UNINIT on success. |
| `syncConfig()` | `N*5W + 4W` | none in core | Force CONFIG sleep, apply cached config, final sleeping CONFIG. |
| `resetAndReapply()` | `1W + N*5W + 4W` | none in core | RESET_DEV plus full config reapply. |
| `readChannelBlocking()` | `P*ready + 2R` | `cooperativeYield` between polls | Requires `nowMs`; `ready` is `0R` or `1R` per poll. Timeout bounds readiness wait only. |
| `readAllChannelsBlocking(N)` | `P*ready + 2N R` | `cooperativeYield` between polls | Requires `nowMs`; waits for one DRDY, then latest-register readout. Timeout bounds readiness wait only. |
| Major setters | usually `1W`, `setSingleChannelMode()` `2W` | none | Setters require sleep mode and commit cache after successful writes. |
| Raw register access | `1R` or `1W` | none | Diagnostic only; raw writes mark hardware config dirty. |

## Conversion Timing Model

The helper methods use the local datasheet approximation:

- Channel reference clock: `fREFx = fRef / FREF_DIVIDERx`.
- Conversion time: approximately `(RCOUNTx * 16 + 4) / fREFx`.
- Settling time: `32 / fREFx` for SETTLECOUNT 0 or 1, otherwise
  `(SETTLECOUNTx * 16) / fREFx`.
- `calcSampleTimeUs()` returns conversion plus settling time for one channel.
- In autoscan, estimate the nominal frame time by summing enabled channel sample
  times, then validate the observed cadence on hardware.
- The driver enforces register-field ranges and the datasheet Table 43
  multi-channel minima it can check from configuration: selected autoscan
  channels require `RCOUNTx >= 0x0009` and `SETTLECOUNTx >= 0x0004`.

The `fRef` argument is the pre-divider reference clock supplied to the LDC
channel, not already-divided `fREFx`. Internal versus external clock accuracy,
multi-channel sequencing overhead/switching behavior, I2C readout time,
interrupt latency, and sensor restart/error behavior are not included in the
helper result. Treat the result as an estimate for scheduling and validation,
not a hardware-proven sample-rate guarantee.

The application clock plan must still validate facts the core cannot infer:
actual `fCLK`/`fREFx`, `fINx < fREFx/4`, the 1 kHz to 10 MHz sensor operating
range, `FIN_DIVIDERx >= 2` when actual sensor frequency is at least 8.75 MHz,
and deglitch bandwidth above the maximum sensor frequency. Capture board logs
or bench measurements before making timing/readiness claims.

## Documentation

- `CHANGELOG.md` - Full release history
- `docs/HARDWARE_INTEGRATION.md` - LDC1612/LDC1614 hardware integration checklist
- `docs/HIL_VALIDATION.md` - Hardware-in-the-loop validation procedure and matrix
- `docs/IDF_PORT.md` - ESP-IDF portability guidance
- `docs/IDF_PORT_IMPLEMENTATION.md` - ESP-IDF implementation notes and validation status
- `docs/LDC1614_INDUSTRY_HARDENING_FINAL_REPORT.md` - Hardening summary and remaining release blockers
- `LDC1614_inductance_converter_implementation_manual.md` - Device documentation
- `docs/` - Datasheets and application notes

## Readiness and Validation Status

The core has a framework-neutral, injected-transport architecture and native
software tests/guards. That is not the same as hardware validation. No
deployment-readiness, certification, or hardware-proven operation claim is made
until real LDC1614/LDC1612 logs cover the board, address strap, INTB/SD wiring,
sensor configuration, fault cases, and soak profile for the target application.

Use `tools/ldc1614_hil_runner.py` and `docs/HIL_VALIDATION.md` to collect
hardware evidence. If the runner is not connected to real firmware and hardware,
its result is `NOT_RUN`, not pass.

## Reproducible Version Metadata

`scripts/generate_version.py` keeps `Version.h` synchronized with
`library.json`. PlatformIO builds can inject build timestamp and Git metadata.
For deterministic build metadata, set `SOURCE_DATE_EPOCH=<unix-seconds>`. If no
source epoch is available, set `LDC1614_REPRODUCIBLE_BUILD=1` to use
`1970-01-01 00:00:00` as the injected timestamp. Without injected metadata, the
generated header falls back to `unknown-date unknown-time` rather than compiler
`__DATE__` / `__TIME__`.

## License

MIT License. See `LICENSE`.
