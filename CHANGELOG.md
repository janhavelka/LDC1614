# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Public, bus-silent `validateConfig()` and canonical configuration-register
  expectation helpers for staged profile validation and exact masked readback.
- Comprehensive, table-driven Arduino and native ESP-IDF diagnostic CLIs with
  complete typed profile editing, register verification, INTB/SD visibility,
  pure calculation/decoder commands, and retained result/sample inspection.
- Fixed-memory cooperative scan, dump, verify, self-test, watch, stress,
  mixed-stress, sample-rate, and bounded soak sessions with cancellation and
  exact operation/session correlation.
- A host-only CLI contract manifest that enforces identical command, help,
  safety, evidence, and output schemas without compiling either framework's
  CLI implementation into the other.
- An explicit external-owner composition test covering immutable deadlines,
  one-transfer polling, cancellation/replacement, conservative transport-fault
  classification, invalidation, and full reinitialization.
- Bounded Arduino no-sensor HIL soak execution with explicit duration, complete
  cycle counts, per-command outcomes, latency/reset counters, exact final
  active-state checks, and optional raw transcript output.
- Sibling-library-style HIL configuration and invalid-input matrices covering
  safe cache-only setting boundaries, exact rejection output, recovery fences,
  and explicit `NOT_RUN` gates for sensor, INTB, SD, address/variant, drive, and
  active-cancellation fixtures.
- Native ESP-IDF diagnostic bus scanning with the same bounded NACK/timeout/
  bus-error contract as the Arduino bring-up CLI.
- Explicit application-owned `busrecover` diagnostics in both example CLIs;
  recovery invalidates applied state and requires complete initialization.

### Changed

- Replaced the compressed diagnostic help with sibling-library-compatible ANSI
  coloring, aligned 32-column command rows, safety sections, symbolic statuses,
  complete configuration/progress/result/fault output, and named sample quality.
- Configuration commands now edit a fixed staged profile with zero I2C; one
  explicit validated commit updates desired state and the cooperative `apply`
  job remains the only path that writes the complete profile.
- Arduino serial input and native ESP-IDF console input are bounded per owner
  pass, and asynchronous commands defer their prompt until a correlated
  terminal result or diagnostic-session summary is printed.
- HIL stress and sensor-gated sample-rate coverage now use stable CLI
  scheduled/result envelopes for both maintained firmware profiles.
- Replaced the completed TunnelMonitor suitability-audit ledger with a concise
  open-gates guide, and reduced validation status to repeatable release checks,
  retained evidence boundaries, and unresolved physical validation.
- Consolidated HIL artifact guidance into the maintained validation procedure;
  retained transcript-bearing JSON and raw serial captures while removing
  generated Markdown copies and completed report/audit narratives.
- Removed both rebuildable raw PDF extraction trees. All vendor PDFs, concise
  per-document summaries, and curated implementation notes remain.
- Removed unused library/example includes, an orphaned example LED constant,
  dead HIL and cross-project version-generator helpers, redundant CLI checks,
  stale PlatformIO flags, and the coverage environment that produced no report.
- Reduced CI to distinct checks; the HIL host suite already covers parser and
  no-port behavior, while the clean-consumer check already packs the library.
- Exported the maintained guides, contribution and Doxygen configuration, and
  primary datasheet so documentation links in packaged README files remain
  usable.
- The version generator now synchronizes and checks `idf_component.yml` and
  Doxygen project metadata from the authoritative `library.json` version.
- Pin maintained Arduino builds to pioarduino `55.03.311` (Arduino 3.3.11,
  ESP-IDF 5.5.5 libraries). PlatformIO is pinned to the required 6.1.19 host
  version. COM8 still reproduced the open ESP-IDF new-master post-NACK
  `ESP_ERR_INVALID_STATE` failure, so the pin is a build baseline rather than
  recovery qualification and every target must repeat combined write/read and
  post-NACK testing.
- The Arduino and native ESP-IDF diagnostics now share one example-owned
  ESP-IDF new-master transport with exact backend error detail and bounded
  explicit recovery. Recovery removes the owned device, deletes/recreates the
  owned bus, recreates the device handle, runs the driver's bounded bus
  reset/line-clear, and requires one bounded target ACK probe before reporting
  success; initial
  open now rolls back the bus if device registration fails. The redundant Wire
  adapter and unused host framework stubs were removed. This contains recovery
  policy in the diagnostic owner and is not a claim that the upstream
  post-NACK state bug is fixed.

### Fixed

- Arduino and native ESP-IDF sample-rate sessions now apply the same bounded
  readiness gate and reject stale, invalid, faulted, overrun, or out-of-range
  samples instead of counting transport success as measurement success.
- Diagnostic sessions retain the latest correlated core result and print full
  effect, phase, register, channel, status, and configuration-fault provenance
  on failed or cancelled acquisitions.
- CLI input length checks are bounded before copying, asynchronous work rejects
  a missing monotonic-clock callback instead of freezing at time zero, and
  cancelling a synchronous session no longer emits a second deferred prompt.
- Removed the experimental fixed post-reset delay after COM8 evidence showed it
  did not correct the ESP-IDF transport failure. Reset/reapply remains fully
  cooperative and reports the first reset-adjacent transport error unchanged.
- ESP32 example recovery no longer reports success merely because handle
  reconstruction succeeded. Its bounded bus reset/line-clear and final
  target-address probe expose each failed phase and normalize the new-master
  terminal state on the pinned ESP-IDF 5.5.5 baseline before the required
  complete initialization/replay.
- `selftest` is now a real bounded diagnostic rather than an alias for two
  identity reads; it reports explicit pass, failure, and fixture-dependent skip
  counts.
- Acquisition output now includes both STATUS snapshots, raw register halves,
  calculated frequency/range evidence, configuration revision, and symbolic
  quality; driver output includes full cached configuration-fault provenance.
- Long HIL runs journal raw serial evidence incrementally and convert serial
  command or serial-context close exceptions into explicit failed artifacts
  instead of losing the complete run before output creation.
- Expected HIL failures can be scoped to one normalized command so a known
  negative reset result cannot mask recovery, replay, or identity failures.
- The HIL runner now refuses to enter a timed soak unless every base command,
  firmware/fixture expectation, and reset-banner check passes; complete-cycle
  accounting no longer treats a partially executed cycle as complete.
- Raw writes, destructive all-register dumps, silicon reset/reapply, owner bus
  recovery, invalidation, mixed stress, and example-owned shutdown transitions
  require explicit confirmation and preserve dirty/unknown state evidence.
- The no-sensor HIL classifier now accepts observed silicon sensor-condition
  flags only when the command still supplies its required structured evidence;
  transport, identity, timeout, and nonzero-status failures remain failures.
- Failed identity reads during re-initialization now invalidate previously
  trusted applied configuration and retain exact `ConfigFault` provenance.
- Acquisition reports destructive-read effects only after a STATUS or DATA-MSB
  transaction succeeds; an address NACK before any read no longer claims a
  hardware side effect.
- Amplitude-high/low fault samples remain explicit but are no longer included
  in `validChannels`.
- OFFSET validation now includes `FIN_DIVIDER`, matching the public frequency
  calculation and preventing an accepted offset from masking the configured
  minimum sensor frequency.
- Diagnostic bus scans now refuse to interleave with an active cooperative
  driver job, skip reserved I2C address groups, and no longer run implicitly at
  Arduino example startup. The example HIL sequence explicitly reinitializes
  the application-owned bus and replays configuration after scan NACK traffic.
- ESP32-S2 internal-USB uploads now use automatic 1200-baud bootloader entry,
  wait for port re-enumeration, avoid a second pre-flash reset, and hard-reset
  back into the application instead of requiring operator reset cycles or
  remaining in the flasher stub.
- Maintained ESP32 PlatformIO uploads suppress esptool's Unicode progress bar,
  avoiding Windows console-encoding failures that can interrupt a flash and
  leave the native USB loader endpoint occupied.
- Absolute 64-bit job deadlines now use half-range wrap-safe comparison, so a
  valid deadline immediately after `uint64_t` rollover is not timed out early.
- The HIL runner rejects non-finite timing arguments and applies one absolute
  command timeout across both scheduled and terminal async responses.
- The HIL runner strips ANSI only for parsing while retaining raw evidence,
  distinguishes expected no-sensor quality counters from transport failures,
  activates both fixture profiles after initialization/reset replay, validates
  the true RCOUNT boundaries, requires a clean final staged profile, and stops
  at the first unexpected base-command failure. Every unsent command is
  retained as explicit `NOT_RUN` instead of exercising a poisoned backend.

## [3.0.0] - 2026-07-22

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
  28-bit count, silicon quality flags, and owner poll-boundary timestamp.
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

- Curated Doxygen now publishes the public headers plus maintained integration,
  validation, and HIL guides while excluding internal instructions and raw
  vendor/reference trace material.
- Documentation generation now fails on undocumented public API, missing
  parameter documentation, and Doxygen documentation errors, and is exercised
  in CI.
- Reconciled README navigation, contribution checks, validation evidence, and
  the consolidated v3.0.0 release identity.
- Expanded native tests with exact LDC1612/LDC1614 replay transcripts, precise
  phase/register/channel provenance, four-channel acquisition faults,
  cancellation/deadline paths, freshness, lifecycle, one-transfer boundaries,
  clock/OFFSET/frequency endpoints, and individual register-field encodings.
- Removed unused v2-era example helper headers and their stale guard
  requirements; current examples retain only their actual owners.
- Clarified monotonic 64-bit owner time, non-reentrant callbacks, poll-boundary
  timestamps, TunnelMonitor retry boundaries, release-tag verification, and HIL
  evidence limits.
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

### Removed

- Removed one-time implementation instructions and superseded COM8 dry-run and
  empty-payload audit artifacts. Current guides, cited chip-only hardware
  evidence, and vendor-source traceability remain.

### Fixed

- Divide each poll's remaining deadline across the callbacks actually left in
  the job, so a large caller budget cannot spuriously shorten callback timeouts
  near completion.
- Validate external reference-clock tolerance and the datasheet OFFSET versus
  minimum-sensor-frequency constraint with overflow-safe worst-case arithmetic.
- Treat a confirmed address NACK as a known no-device-mutation result while
  retaining conservative ambiguity for timeout, bus, and data-NACK failures.
- Make advanced raw access variant-aware and reject writes to read-only
  DATA/STATUS/identity registers.
- Require trusted active configuration before destructive readiness polling.
- Correct the behavioral fake so wrong-address transactions cannot mutate
  device or destructive-read state.
- Require command-specific HIL evidence, exact device identity, configured
  address/channel facts, clean firmware-reported Git identity, and nonzero exit
  for ambiguous verification runs. Host and flashed firmware revisions are now
  recorded separately.
- Distinguish normal address NACK from Arduino scan transport failures and stop
  the bounded diagnostic scan on timeout/bus error.
- Share one timeout budget across the Arduino combined write/read phases instead
  of allowing each phase to consume the full callback timeout.
- Include untracked source files in flashed-firmware cleanliness metadata and
  report Git status failures as unknown rather than clean.
- Install Graphviz with Doxygen in CI so enabled include and collaboration
  diagrams do not fail documentation generation when `dot` is unavailable.
- Cold initialization, configuration replay, and reset/reapply can no longer
  monopolize an external bus owner in one library call.
- Deadline or application cancellation cannot leave an old job busy or permit
  a stale completion to be attributed to its replacement.
- Acquisition no longer discards the STATUS snapshot that can be destroyed by
  later DATA reads, nor publishes partial cache changes when a later phase
  fails or is cancelled.
- Raw endpoint saturation, watchdog, amplitude, zero-count, and data-loss
  conditions remain visible independently of transport success. Configuration
  trust remains explicit through applied state and acquisition admission.
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
- HIL procedure and conservative runner: `docs/HIL_VALIDATION.md` and
  `tools/ldc1614_hil_runner.py`. A no-hardware run is
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
- Blocking timing/freshness helpers validate clock callbacks before polling,
  preserve data-ready/status failures, and expose fresh unread conversions
  explicitly.
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
