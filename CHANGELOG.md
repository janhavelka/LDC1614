# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Explicit `readDataReady(bool&)` API for DRDY checks with `Status` error reporting.
- Runtime setters for single-channel mode, auto-scan sequence, deglitch bandwidth, `ERROR_CONFIG`, INTB output, reference clock source, sensor activation policy, RP override, auto amplitude correction, and high-current drive.
- Validation for `ERROR_CONFIG` reserved bits and LDC1612 round-robin sequence limits.
- Native tests for data-ready error propagation, STATUS parsing, channel data parsing, recovery identity validation, runtime setter cache commits, and configuration constraints.
- Bringup CLI commands covering the runtime configuration surface.

### Changed
- Blocking read helpers now propagate `readDataReady()` failures instead of converting I2C errors into timeouts.
- Runtime setters now commit cached configuration only after successful register writes.
- `recover()` now validates both `MANUFACTURER_ID` and `DEVICE_ID` and documents the recovery ladder.
- README now documents runtime setters, configuration constraints, CLI coverage, and STATUS/INTB data-ready behavior.

### Fixed
- INTB data-ready checks now read STATUS when the pin is asserted so sensor errors are not misreported as data-ready events.
- Channel cache and calculation helpers now reject channels outside the configured LDC1612/LDC1614 channel count.

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
