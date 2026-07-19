# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

No changes yet.

## [3.0.0] - 2026-07-19

### Added

- One fixed-memory cooperative job engine for initialization, configuration
  apply, software reset/reapply, and status-aware channel acquisition.
- Caller-owned nonzero `OperationId`, absolute 64-bit deadline, per-poll
  transport budget, cache-only `JobProgress`, bus-silent `cancelJob()`, and a
  fixed two-entry exactly-once terminal `OperationResult` FIFO.
- Full terminal provenance: success/failure/cancel/timeout outcome, destructive
  read and partial/indeterminate write effects, desired configuration revision,
  and structured `ConfigFault` with the original status, phase, register, and
  channel.
- Fixed `SampleBatch` with pre/post DATA STATUS snapshots,
  selected/valid/fresh/error/overrun masks, per-channel raw register words,
  28-bit count, silicon quality flags, and owner completion timestamp.
- Explicit `DeviceVariant`, `I2cAddress`, `Channel`, `ChannelMask`,
  `ReferenceClock`, sensor-frequency bounds, and typed `ErrorReporting`.
- Bus-silent `invalidateAppliedState()` for owner-observed removal, brownout,
  chip reset, shutdown, or shared-bus recovery.
- Checked `double` frequency calculation, conservative fixed-unit frame timing,
  nominal drive-current lookup, pure STATUS/DATA decoders, and error-policy
  encoder.
- Behavioral fake-device tests for shadow latching, UNREAD consumption,
  STATUS/error clearing, INTB deassertion, conversion overrun, and ambiguous
  transport writes, plus exhaustive phase failure/cancellation tables.
- Pinned host tools in `requirements-dev.txt` and an exact maintained Espressif
  PlatformIO platform URL.

### Changed

- **Breaking:** `begin(config)` is replaced by zero-I2C `bind(config)` followed
  by `startInitialize(id, deadline)`, budgeted `poll()`, and exactly-once
  `takeResult()`.
- **Breaking:** `readChannel`, `readAllChannels`, `readFreshChannels`, and
  `startReadChannels` are replaced by one `startAcquire()` protocol that always
  preserves destructive STATUS evidence and commits a complete batch only.
- **Breaking:** `syncConfig()` becomes `startApplyConfig()`;
  `resetAndReapply()` becomes `startResetAndReapply()`.
- **Breaking:** `tick()`, blocking read helpers, driver-owned `recover()`, the
  OFFLINE/DEGRADED state machine, retry/backoff policy, and bus/hard-reset hooks
  are removed. Applications retain scheduling, health, retry, and recovery
  authority; `TransportStats` is non-authoritative diagnostics only.
- Default `Config` is intentionally invalid. Variant, address, reference clock,
  channel mask/profile, mode, deglitch, and readiness/error policy must be
  explicit before binding.
- Applied configuration is represented as unknown/applying/applied sleeping/
  applied active/dirty. Acquisition rejects untrusted or inactive hardware
  state, and matching identity after return still requires complete replay.
- Arduino and native ESP-IDF diagnostic examples now advance no more than one
  driver transport callback per service pass and consume terminal results by ID.
- HIL defaults were migrated away from removed v2 commands. Asynchronous v3
  jobs require correlated terminal-result evidence; immediate `IN_PROGRESS`
  output is not classified as completion.
- Documentation now states DATAx_MSB/STATUS/UNREAD/INTB destructive effects,
  sequential non-simultaneous channel timing, concurrency/ISR/latency limits,
  external-owner retry/deadline/recovery policy, and exact operation maxima.
- `native_cov` is described as coverage instrumentation only because no report
  or threshold is produced.

### Fixed

- Cold initialization, configuration replay, and reset/reapply can no longer
  monopolize an external bus owner in one library call.
- Deadline or application cancellation cannot leave an old job busy or permit
  a stale completion to be attributed to its replacement.
- Acquisition no longer discards the STATUS snapshot that can be destroyed by
  later DATA reads, nor publishes partial cache changes when a later phase
  fails or is cancelled.
- Raw endpoint saturation, watchdog, amplitude, zero-count, data-loss, and
  configuration-unknown conditions remain visible independently of transport
  success.
- A transport failure that may have committed a write remains explicitly
  indeterminate and is never converted to success by a blind retry.

### Validation limits

- Existing ESP32-S2/COM8 chip-only artifacts were produced against v2. They do
  not validate v3 IDs, deadlines, cancellation, result delivery, applied-state
  invalidation, or acquisition semantics.
- No committed raw v3 target transcript or logic trace validates sensor-attached
  conversion, INTB/SD, address variants, fault injection, exact board clock
  plans, coils, calibration, or soak behavior.

## [2.0.0] - 2026-07-01

### Added
- Dirty/sync-needed partial hardware-state contract with `Err::CONFIG_DIRTY`,
  `hardwareConfigDirty()`, `hardwareConfigDirtyError()`, `syncConfig()`, and
  dirty-state fields in `SettingsSnapshot`.
- Freshness APIs: `FreshChannelData` and `readFreshChannels()` for
  STATUS/`UNREADCONVx`-driven latest-versus-unread conversion handling.
- Poll-chunked I2C APIs for selected channel reads, cached config apply, and
  reset/reapply with one active job per driver instance.
- Clean package consumer compile guard that packs the library, extracts the
  archive, and compiles a consumer against the packaged public headers and
  source.
- HIL procedure and conservative runner: `docs/HIL_VALIDATION.md`,
  `docs/hil/README.md`, and `tools/ldc1614_hil_runner.py`. A no-hardware run is
  `NOT_RUN`, not a pass.
- Reproducible version metadata controls through `SOURCE_DATE_EPOCH` and
  `LDC1614_REPRODUCIBLE_BUILD=1`.
- Maintained docs index and validation status pages that separate software
  hardening evidence from hardware validation limitations.
- ESP-IDF component metadata, root `CMakeLists.txt`, and an interactive
  `examples/esp_idf/basic` CLI application using the ESP-IDF new I2C master
  driver, `esp_timer`, FreeRTOS yield hook, optional INTB GPIO hook, and
  optional bus/hard reset callbacks.
- Arduino diagnostic CLI coverage through `examples/common/Ldc1614Cli.cpp` and
  a separate ESP-IDF fixed-buffer native CLI with a contract-checked safe command
  subset.
- `tools/check_idf_example_contract.py` to guard ESP-IDF CLI parity, native I2C
  driver usage, and absence of Arduino compatibility facades.
- IDF port notes documenting the framework-neutral core boundary and validation
  status.
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
- `LDC1614` copy and move construction/assignment are deleted to avoid implicit
  duplication of non-owning transport callbacks and cached device state.
- `sampleTimestampMs()` may return `0` for a valid cached sample; use
  `hasSample()` or `getLastSample()` validity status to test whether a sample is
  present.
- Full configuration apply and runtime configuration writes now document and
  report dirty hardware/cache divergence when a multi-register sequence can
  partially reach the device.
- ESP-IDF diagnostic CLI path now uses native fixed-buffer sources and is guarded
  against Arduino facade/shared Arduino CLI leakage.
- Active poll-chunked jobs block other public I2C APIs with `BUSY` until
  `poll()` completes or fails the job.
- `probe()` now preserves non-address transport failures instead of collapsing
  all I2C failures into `DEVICE_NOT_FOUND`.
- Recovery backoff is enforced only when `Config::nowMs` is configured, avoiding
  permanent `BUSY` recovery behavior in no-timebase integrations.
- Package export contents are explicit in `library.json`, and ESP-IDF CMake now
  checks generated `Version.h` consistency instead of mutating source trees.
- Readiness wording was tightened across maintained docs and metadata. Hardware
  validation remains pending for target boards and sensors.
- Removed Arduino `millis()` and `yield()` fallbacks from the driver core.
  Applications should provide `Config::nowMs` and `Config::cooperativeYield`
  when blocking helpers need wall-clock time or cooperative scheduling.
- Declared `espidf` framework support in PlatformIO metadata while keeping the
  Arduino example functionality equivalent through example-local hooks.
- Doxyfile project metadata now matches `library.json`.
- Explicit recovery/reset bypass internals now use the shared `ScopedOfflineI2cAllowance` / `_reassertOfflineLatch()` procedure so failed recovery attempts that begin from `OFFLINE` keep the latch asserted.
- Doxyfile inputs now focus generated API docs on public headers and top-level
  project docs, avoiding extracted application-note math warnings.
- Reference documentation now uses human-readable vendor PDF names and keeps
  compact notes, application notes, how-to guides, and raw PDF markdown under
  `docs/reference/`.
- Blocking read helpers now propagate `readDataReady()` failures instead of converting I2C errors into timeouts.
- Blocking read helpers now have a finite poll cap even if the injected clock callback stops advancing.
- Runtime setters now commit cached configuration only after successful register writes.
- `begin()` now resets cached runtime/health before validation, normalizes `offlineThreshold = 0` to one, and startup I2C no longer inflates runtime success counters.
- `recover()` now validates both `MANUFACTURER_ID` and `DEVICE_ID` and documents the recovery ladder.
- Public raw register helpers now reject calls before `begin()` and reject addresses outside the LDC1614 register map before touching I2C.
- README now documents runtime setters, configuration constraints, CLI coverage, and STATUS/INTB data-ready behavior.
- Health behavior is now standardized on latched `OFFLINE`: normal public I2C operations return `BUSY` with `Driver is offline; call recover()` and do not touch I2C until `recover()` succeeds.
- Arduino and ESP-IDF examples maintain overlapping diagnostic behavior through
  repo-local command/source contract checks while each framework owns its own
  bus, timing, CLI input, and pin setup.
- Core timing guard now enforces zero Arduino timing calls/includes in
  `include/` and `src/`.

### Fixed
- Prompt 02 timing/freshness reconciliation is complete in software: blocking
  helpers validate clock callbacks before polling, preserve data-ready/status
  failures, and expose fresh unread conversions explicitly.
- Expanded native tests and fake-bus fault injection cover lifecycle, health,
  register, validation, recovery, timing, and poll-budget behavior.
- INTB data-ready checks now read STATUS when the pin is asserted so sensor errors are not misreported as data-ready events.
- Channel cache and calculation helpers now reject channels outside the configured LDC1612/LDC1614 channel count.
- Recovery identity mismatches now update health counters/state instead of returning a semantic failure with a healthy driver state.

### Validation limits
- Local pure ESP-IDF builds require `idf.py` or CI logs and must not be inferred
  from PlatformIO Arduino builds.
- ESP32-S2 no-sensor HIL logs under `docs/reports/` validate LDC1614 identity,
  I2C register access, configuration readback, reset/reapply, recovery, and
  bounded precondition/error paths with the chip present at `0x2A`.
- No committed sensor-attached HIL logs validate live DATAx/STATUS conversion
  side effects, INTB/SD behavior, address strap variants, fault injection, board
  clock plans, sensor limits, or soak behavior.

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

[Unreleased]: https://github.com/janhavelka/LDC1614/compare/v3.0.0...HEAD
[3.0.0]: https://github.com/janhavelka/LDC1614/compare/v2.0.0...v3.0.0
[2.0.0]: https://github.com/janhavelka/LDC1614/compare/v1.0.0...v2.0.0
[1.0.0]: https://github.com/janhavelka/LDC1614/releases/tag/v1.0.0
