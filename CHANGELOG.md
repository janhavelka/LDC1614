# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- ESP-IDF component metadata, root `CMakeLists.txt`, and an interactive
  `examples/esp_idf/basic` CLI application using the ESP-IDF new I2C master
  driver, `esp_timer`, FreeRTOS yield hook, optional INTB GPIO hook, and
  optional bus/hard reset callbacks.
- Shared framework-neutral example CLI implementation used by both Arduino and
  ESP-IDF examples, including identical help text, command aliases, colors,
  prompts, health diagnostics, raw register access, probe/recover/reset,
  `selftest`, `stress`, `stress_mix`, and `demo` workflows.
- `tools/check_idf_example_contract.py` to guard ESP-IDF CLI parity, native I2C
  driver usage, and stale wording.
- IDF port implementation notes documenting the framework-neutral core boundary
  and validation status.
- Explicit `readDataReady(bool&)` API for DRDY checks with `Status` error reporting.
- Expanded settings snapshot API: `getSettings(SettingsSnapshot&)`, by-value `settings()`, `driverState()`, and per-channel `hasSample()`.
- Public timing helpers `calcSettleTimeUs()` and `calcSampleTimeUs()`.
- Runtime setters for single-channel mode, auto-scan sequence, deglitch bandwidth, `ERROR_CONFIG`, INTB output, reference clock source, sensor activation policy, RP override, auto amplitude correction, and high-current drive.
- `Status::is(Err)` for type-safe error checks.
- Validation for `ERROR_CONFIG` reserved bits and LDC1612 round-robin sequence limits.
- Native tests for data-ready error propagation, STATUS parsing, channel data parsing, recovery identity validation, runtime setter cache commits, and configuration constraints.
- Native tests for public raw-register preconditions, invalid register rejection, explicit channel-count rejection, non-finite reference clocks, and stalled-clock blocking wait timeouts.
- Native coverage proving latched `OFFLINE` blocks normal I2C operations without touching the bus while `recover()` remains the explicit recovery path.
- Bringup CLI commands covering the runtime configuration surface, identity readback, reinitialization aliases, and timing calculations.

### Changed
- Removed Arduino `millis()` and `yield()` fallbacks from the driver core.
  Applications should provide `Config::nowMs` and `Config::cooperativeYield`
  when blocking helpers need wall-clock time or cooperative scheduling.
- Declared `espidf` framework support in PlatformIO metadata while keeping the
  Arduino example functionality equivalent through example-local hooks.
- Doxyfile project metadata now matches `library.json`.
- Explicit recovery/reset bypass internals now use the shared `ScopedOfflineI2cAllowance` / `_reassertOfflineLatch()` procedure so failed recovery attempts that begin from `OFFLINE` keep the latch asserted.
- Doxyfile inputs now focus generated API docs on public headers and top-level
  project docs, avoiding extracted application-note math warnings.
- Reference documentation now uses human-readable vendor PDF names and separates compact inductance-converter notes from full PDF/application-note extractions under `docs/extracted-md/` and `docs/pdf-extracted-md/`.
- Blocking read helpers now propagate `readDataReady()` failures instead of converting I2C errors into timeouts.
- Blocking read helpers now have a finite poll cap even if the injected clock callback stops advancing.
- Runtime setters now commit cached configuration only after successful register writes.
- `begin()` now resets cached runtime/health before validation, normalizes `offlineThreshold = 0` to one, and startup I2C no longer inflates runtime success counters.
- `recover()` now validates both `MANUFACTURER_ID` and `DEVICE_ID` and documents the recovery ladder.
- Public raw register helpers now reject calls before `begin()` and reject addresses outside the LDC1614 register map before touching I2C.
- README now documents runtime setters, configuration constraints, CLI coverage, and STATUS/INTB data-ready behavior.
- Health behavior is now standardized on latched `OFFLINE`: normal public I2C operations return `BUSY` with `Driver is offline; call recover()` and do not touch I2C until `recover()` succeeds.
- Arduino bringup example now delegates command behavior to the shared CLI
  source, while retaining Arduino-owned Wire, Serial, timing, and pin setup.
- Core timing guard now enforces zero Arduino timing calls/includes in
  `include/` and `src/`.

### Fixed
- INTB data-ready checks now read STATUS when the pin is asserted so sensor errors are not misreported as data-ready events.
- Channel cache and calculation helpers now reject channels outside the configured LDC1612/LDC1614 channel count.
- Recovery identity mismatches now update health counters/state instead of returning a semantic failure with a healthy driver state.

## [1.0.0] - 2026-04-05

### Added
- Initial release of the LDC1614/LDC1612 driver library.
- Complete register map in `CommandTable.h` with all addresses, bit masks, field values, and per-channel address helpers.
- `Status` struct with error codes including `SENSOR_ERROR` for device-reported faults.
- `Config` struct with transport callbacks, per-channel configuration, global conversion settings, and health tracking.
- `LDC1614` driver class with managed synchronous architecture and 4-state health tracking (UNINIT/READY/DEGRADED/OFFLINE).
- Lifecycle: `begin()` with comprehensive config validation, `tick()`, `end()`.
- Diagnostics: `probe()` (raw, no health tracking) and `recover()` (tracked).
- Data readback: `readChannel()`, `readAllChannels()`, `dataReady()` (INTB pin or STATUS polling).
- Device status: `readDeviceStatus()` with full error flag parsing, `readStatusRaw()`.
- Control: `sleep()`, `wake()`, `softReset()`.
- Runtime configuration (sleep mode only): `setActiveChannel()`, `setRcount()`, `setSettleCount()`, `setClockDividers()`, `setOffset()`, `setDriveCurrent()`, `readInitIdrive()`.
- Raw register access: `readRegister16()`, `writeRegister16()`.
- Utility: `calcSensorFrequency()`, `calcConversionTimeUs()`.
- Transport wrapper architecture: Raw and Tracked wrappers with health tracking in tracked layer only.
- Example helpers in `examples/common/` (BoardConfig, Log, I2cTransport, I2cScanner, CommandHandler, BusDiag, BuildConfig).
- Interactive CLI example in `examples/01_basic_bringup_cli/`.
- Native contract tests with FakeBus mock covering Status, Config, lifecycle, health transitions, and transport validation.
- PlatformIO build configuration for ESP32-S2, ESP32-S3, and native test environments.
- Auto-generated `Version.h` via `scripts/generate_version.py`.
- Lifecycle accessors `isInitialized()` and `getConfig()` for cached state inspection without extra I2C.
- Blocking read helpers `readChannelBlocking()` / `readAllChannelsBlocking()` plus sample-cache helpers (`getLastSample()`, `sampleTimestampMs()`, `sampleAgeMs()`).
- `getSettings()` snapshot coverage in the public API and README.

### Changed
- README and bringup example documentation now describe `resetAndReapply()`, raw `readRegister16()` / `writeRegister16()` access, and the full `examples/common/` helper set.
- CLI help now marks raw register writes as diagnostic/service operations that can desynchronize cached config until reinitialization.

[Unreleased]: https://github.com/janhavelka/LDC1614/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/janhavelka/LDC1614/releases/tag/v1.0.0
