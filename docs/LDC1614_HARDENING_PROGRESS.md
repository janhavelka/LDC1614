# LDC1614 Hardening Progress

## Prompt 01 - H2/M1/M3/M5 core contracts

### Findings addressed

- H2/H3 partial hardware state: configuration writes can leave hardware and cached configuration diverged.
- M1 raw diagnostics: raw register writes were not clearly documented as unsafe cache-desynchronizing diagnostics.
- M3 API contracts: driver copy/move, thread-safety, ISR-safety, and transport recursion contracts were too weak.
- M5 datasheet notes: deglitch 33 MHz ambiguity, IDRIVE table reference, reset/reapply timing assumptions, and `probe()` lifecycle caveat needed explicit documentation.

### Implemented changes

- Added `hardwareConfigDirty()` and `hardwareConfigDirtyError()` runtime diagnostics.
- Added dirty-state fields to `SettingsSnapshot`.
- Added `syncConfig()` to re-apply cached configuration without a reset.
- Dirty state is set for failed configuration writes that may have reached hardware, partial `_applyConfig()`, partial `setSingleChannelMode()`, partial `resetAndReapply()`, and public raw diagnostic writes.
- Dirty state is cleared only after full successful `begin()`, `syncConfig()`, `recover()` reapply, or `resetAndReapply()`.
- `softReset()` leaves dirty state intact until a later successful reinitialization or full sync path.
- `recover()` no longer treats identity-read success as configuration sync when dirty state is set; it reapplies cached configuration before returning success.
- Public raw register access remains available, but it is documented as diagnostic-only and not variant/access-type safe.
- Deleted driver copy/move construction and assignment.
- Added public Doxygen and README notes for thread/ISR safety, external serialization, and callback non-recursion.
- Documented the deglitch 33 MHz `b111` choice and the lack of local evidence for an arbitrary post-`RESET_DEV` delay.

### Public API changes

- Added `Err::CONFIG_DIRTY`.
- Added `LDC1614::hardwareConfigDirty()`.
- Added `LDC1614::hardwareConfigDirtyError()`.
- Added `LDC1614::syncConfig()`.
- Added `SettingsSnapshot::hardwareConfigDirty`.
- Added `SettingsSnapshot::hardwareConfigDirtyError`.
- Deleted copy/move operations for `LDC1614`.

### Tests added

- Copy/move deletion static assertions.
- Deglitch enum value regression test.
- Probe missing-callback behavior test.
- `_applyConfig()` partial failure dirty-state/detail test via `begin()`.
- Dirty clear/retain tests for `syncConfig()`.
- Dirty clear/retain tests for `recover()`.
- Raw diagnostic write dirty-policy tests.
- `setSingleChannelMode()` partial failure dirty-state/detail test.
- `resetAndReapply()` partial failure and success-clear tests.
- `softReset()` dirty-state retention until reinitialization.
- Existing single-register cache-failure test now also asserts dirty state.

### Commands run

- `python tools/check_core_timing_guard.py` -> `Core timing guard PASSED`.
- `python tools/check_cli_contract.py` -> `CLI contract PASSED`.
- `python tools/check_idf_example_contract.py` -> `IDF example contract PASSED`.
- `python scripts/generate_version.py check` -> `Up to date: C:\Users\Honza\Documents\Projects\LDC1614\include\LDC1614\Version.h`.
- `python -m platformio test -e native` -> passed; `84 test cases: 84 succeeded in 00:00:00.995`.
- `python -m platformio run -e esp32s3dev` -> passed; `esp32s3dev SUCCESS 00:00:05.195`.
- `python -m platformio run -e esp32s2dev` -> passed; `esp32s2dev SUCCESS 00:00:04.889`.
- `python -m platformio pkg pack` -> passed; wrote `LDC1614-1.0.0.tar.gz`, which was removed after packaging.

### Remaining related work

- Raw register access is still an expert diagnostic escape hatch, not a full metadata-enforced safe register API.
- H2 clock/divider validation and sleep-before-config sequencing are intentionally left for the next relevant hardening chunk.
- ESP-IDF pure-build validation and HIL/hardware validation remain unclaimed.

## Prompt 03 - H4/M6/L3 ESP-IDF hardening

### Findings addressed

- H4 ESP-IDF path: native example still compiled the shared heap-capable CLI and had weak build/guard coverage.
- M6 INTB handling: native IDF example configured INTB with a pull-up despite the documented push-pull device behavior.
- L3 component metadata: `idf_component.yml` did not state intended ESP-IDF targets.
- Related documentation: IDF example scope needed explicit diagnostic bring-up labeling and production ownership caveats.

### Implemented changes

- Removed `examples/common/Ldc1614Cli.cpp` from the ESP-IDF example compile path.
- Added `examples/esp_idf/basic/main/Ldc1614IdfCli.*`, a fixed-buffer native IDF diagnostic CLI with the limited safe command set: `help`, `version`, `probe`, `status`, `drv`, `cfg`, `read`, `readall`, `ready`, `recover`, `timing`, and `selftest`.
- Replaced blocking `getchar()` with a timed `select()`/`read()` console loop that regularly calls `device.tick()`.
- Converted IDF GPIO/I2C setup from `ESP_ERROR_CHECK` abort paths to `Status` reporting.
- Added an example-owned FreeRTOS mutex around native IDF I2C transport operations and reset paths.
- Kept IDF I2C error mapping bounded and conservative: timeout maps to `I2C_TIMEOUT`; invalid response/NACK stays phase-unspecified with raw `esp_err_t` in `detail`.
- Configured INTB as a plain input with pull-up disabled and documented that LDC1612/LDC1614 INTB is push-pull active-low/configurable.
- Added CMake-time `Version.h` generation for pure ESP-IDF builds from clean checkouts.

### IDF example/component changes

- `examples/esp_idf/basic/main/CMakeLists.txt` now compiles only native IDF example sources.
- `examples/esp_idf/basic/README.md`, `docs/IDF_PORT.md`, and `docs/IDF_PORT_IMPLEMENTATION.md` now label the IDF app as diagnostic bring-up and separate static checks, pure IDF builds, and hardware/HIL validation.
- `idf_component.yml` now lists intended targets `esp32s2` and `esp32s3` and avoids the previous production-grade wording.

### Tests/guards/CI added

- `tools/check_idf_example_contract.py` now parses the actual IDF `SRCS` list and scans compiled sources plus local headers for Arduino/`std::string` leakage, `getchar()`, and `ESP_ERROR_CHECK`.
- `tools/check_cli_contract.py` now preserves Arduino shared-CLI checks while enforcing a native fixed-buffer IDF CLI contract.
- GitHub Actions now runs the IDF source-contract guard and includes a pure ESP-IDF build matrix for `esp32s2` and `esp32s3`.

### Commands run

- `git status --short` -> clean at prompt start.
- `git branch --show-current` -> `hardening/ldc1614-industry-readiness`.
- `git checkout hardening/ldc1614-industry-readiness` -> already on branch.
- `python tools/check_core_timing_guard.py` -> `Core timing guard PASSED`.
- `python tools/check_cli_contract.py` -> `CLI contract PASSED`.
- `python tools/check_idf_example_contract.py` -> `IDF example contract PASSED`.
- `python scripts/generate_version.py check` -> `Up to date: C:\Users\Honza\Documents\Projects\LDC1614\include\LDC1614\Version.h`.
- `python -m platformio test -e native` -> passed; `84 test cases: 84 succeeded in 00:00:01.054`.
- `python -m platformio run -e esp32s3dev` -> passed; `esp32s3dev SUCCESS 00:00:07.827`.
- `python -m platformio run -e esp32s2dev` -> passed; `esp32s2dev SUCCESS 00:00:07.557`.
- `python -m platformio pkg pack` -> passed; wrote `LDC1614-1.0.0.tar.gz`, which was removed after packaging.
- `idf.py --version` -> failed locally: `idf.py` is not recognized as a cmdlet, function, script file, or operable program.
- `idf.py -C examples/esp_idf/basic set-target esp32s3 build` -> not run locally because `idf.py` is unavailable.
- `idf.py -C examples/esp_idf/basic set-target esp32s2 build` -> not run locally because `idf.py` is unavailable.

### Remaining related work

- Pure ESP-IDF build success remains unclaimed locally until `idf.py` or CI logs are available.
- The example mutex serializes native I2C transactions only; production applications with multiple tasks must still serialize public driver API calls.
- No hardware/HIL validation is claimed for INTB, SD, address variants, fault injection, sensor behavior, or soak stability.

## Prompt 04 - M4 tests, fault injection, CI, and guards

### Findings addressed

- M4 native coverage gaps: identity failures, granular transport errors, LDC1612/LDC1614 channel bounds, DATAx ordering, raw diagnostic boundaries, partial configuration failures, and callback recovery paths needed deeper native tests.
- Prior H2/H3/H4 hardening needed stronger regression coverage around partial hardware state, tracked error preservation, and IDF source-contract enforcement.

### Test harness changes

- Extended `FakeBus` with fail-next read/write, Nth transaction failure, register-specific read/write failure, optional partial-write side effect recording, granular `Status` injection, and a bounded ordered transaction log.
- Kept existing write-log helpers for older assertions while adding read/write operation logging for order and call-count tests.
- Extended the native `Wire` stub so Arduino example transport mapping can be tested without real Arduino hardware.

### Tests added

- Identity/begin tests for wrong manufacturer ID, wrong device ID, address/data NACK, timeout, bus error, generic I2C error, and invalid enum casts.
- LDC1612/LDC1614 variant tests for 2-channel apply writes, high-level channel bounds, default two-channel read-all behavior, and CH2/CH3 positive coverage on 4-channel mode.
- Register/data tests for DATAx MSB-before-LSB ordering, DATAx error-bit masking, raw diagnostic boundary access, raw variant-unsafe policy, raw write dirty behavior, and partial-write side effects.
- Partial-state tests for `_applyConfig()` failures across RCOUNT, SETTLECOUNT, CLOCK_DIVIDERS, OFFSET, DRIVE_CURRENT, ERROR_CONFIG, MUX_CONFIG, and CONFIG.
- Error-preservation tests for tracked read, tracked write, and recover paths across `I2C_NACK_ADDR`, `I2C_NACK_DATA`, `I2C_TIMEOUT`, `I2C_BUS`, and `I2C_ERROR`.
- Timing/readiness/recovery edge tests for invalid blocking calls returning before wait/I2C, data-ready error preservation, hard-reset callback failure propagation, and existing dirty-state recovery paths.
- Arduino example transport tests for Wire `endTransmission()` and short-read mapping.

### CI/guard/coverage changes

- Strengthened `tools/check_core_timing_guard.py` into a broader timing/framework guard covering core Arduino/Wire/Serial/String leakage, STL string/vector, allocation, exceptions, printf/logging, ESP-IDF, and FreeRTOS tokens.
- Strengthened `tools/check_idf_example_contract.py` to reject compiled IDF sources and include dirs outside the native IDF example path or under Arduino/common example paths; it also scans `.hpp` headers and Arduino-like timing calls.
- Added `native_cov` PlatformIO environment with coverage instrumentation.
- CI now runs `pio test -e native_cov` and checks generated `Version.h` after sync before packaging.

### Commands run

- `git status --short` -> clean at prompt start.
- `git branch --show-current` -> `hardening/ldc1614-industry-readiness`.
- `git checkout hardening/ldc1614-industry-readiness` -> already on branch and up to date.
- `python tools/check_core_timing_guard.py` -> `Core timing/framework guard PASSED`.
- `python tools/check_cli_contract.py` -> `CLI contract PASSED`.
- `python tools/check_idf_example_contract.py` -> `IDF example contract PASSED`.
- `python scripts/generate_version.py check` -> `Up to date: C:\Users\Honza\Documents\Projects\LDC1614\include\LDC1614\Version.h`.
- `python -m platformio test -e native` -> passed; `105 test cases: 105 succeeded in 00:00:01.452`.
- `python -m platformio test -e native_cov` -> initially failed because PlatformIO ignored `link_flags` and libgcov was not linked; fixed by moving `-lgcov` into `build_flags`.
- `python -m platformio test -e native_cov` -> passed after fix; `105 test cases: 105 succeeded in 00:00:01.528`.
- `python -m gcovr --version` -> failed locally: `No module named gcovr`.
- `python -m platformio run -e esp32s3dev` -> passed; `esp32s3dev SUCCESS 00:00:09.415`.
- `python -m platformio run -e esp32s2dev` -> passed; `esp32s2dev SUCCESS 00:00:08.935`.
- `python -m platformio pkg pack` -> passed; wrote `LDC1614-1.0.0.tar.gz`, which was removed after packaging.
- `idf.py --version` -> failed locally: `idf.py` is not recognized as a cmdlet, function, script file, or operable program.

### Remaining untested paths

- Pure ESP-IDF build success remains dependent on CI or a local ESP-IDF install; no local `idf.py` build was run.
- No hardware/HIL validation is claimed.
- Native coverage now has an instrumented environment, but no local percentage report was produced because `gcovr` is not installed.
- A claims/metadata wording guard and remaining `library.json` production-grade wording are deferred to the docs/release-readiness prompt.

## Prompt 05 - H1/M7 docs, HIL, and final report

### Findings addressed

- H1 unsupported readiness wording: maintained metadata/docs now avoid unsupported readiness claims and separate software architecture from hardware validation.
- M7 example labeling: Arduino and ESP-IDF examples are explicitly diagnostic bring-up, not production bus managers.
- L1 version metadata reproducibility: `scripts/generate_version.py` supports deterministic metadata via `SOURCE_DATE_EPOCH` or `LDC1614_REPRODUCIBLE_BUILD=1`.
- L2 `probe()` documentation remained correct and is reinforced in README/final docs.
- HIL gaps: hardware checklist, HIL procedure, artifact directory README, and a conservative serial runner now exist.

### Documentation changes

- Added `docs/HARDWARE_INTEGRATION.md`.
- Added `docs/HIL_VALIDATION.md`.
- Added `docs/hil/README.md`.
- Added `docs/LDC1614_INDUSTRY_HARDENING_FINAL_REPORT.md`.
- Added `examples/01_basic_bringup_cli/README.md`.
- Updated README readiness, example labels, DATAx error-bit notes, raw/frequency caveats, HIL links, and reproducible version metadata notes.
- Reworded `library.json` and maintained application-note summaries to avoid unsupported readiness claims.

### HIL runner/procedure changes

- Added `tools/ldc1614_hil_runner.py`.
- Runner profiles distinguish Arduino shared CLI commands from native ESP-IDF CLI commands.
- Default no-port behavior records `overall_status: NOT_RUN`, not pass.
- Optional address, stress, SD, INTB, unplug, stuck-bus, long-soak, and drive-tuning items are gated behind explicit flags or marked skipped/manual.
- Added JSON and Markdown output support.

### Hardware actually run

- No real LDC1614/LDC1612 hardware was run in this prompt.
- `python tools/ldc1614_hil_runner.py --json-out .pio\hil_not_run.json --markdown-out .pio\hil_not_run.md --quiet` was run only as a no-hardware dry run and produced `overall_status: NOT_RUN`, reason `serial port was not supplied`.

### Commands run

- `git status --short` -> clean at prompt start.
- `git branch --show-current` -> `hardening/ldc1614-industry-readiness`.
- `git checkout hardening/ldc1614-industry-readiness` -> already on branch and up to date.
- `python -m py_compile scripts/generate_version.py tools/ldc1614_hil_runner.py tools/check_readiness_claims.py` -> passed.
- `python scripts/generate_version.py sync` -> updated ignored generated `include/LDC1614/Version.h`.
- `python tools/check_core_timing_guard.py` -> `Core timing/framework guard PASSED`.
- `python tools/check_cli_contract.py` -> `CLI contract PASSED`.
- `python tools/check_idf_example_contract.py` -> `IDF example contract PASSED`.
- `python tools/check_readiness_claims.py` -> `Readiness claims guard PASSED`.
- `python scripts/generate_version.py check` -> up to date.
- `python -m platformio test -e native` -> passed; `105 test cases: 105 succeeded in 00:00:01.333`.
- `python -m platformio run -e esp32s3dev` -> passed; `esp32s3dev SUCCESS 00:00:05.889`.
- `python -m platformio run -e esp32s2dev` -> passed; `esp32s2dev SUCCESS 00:00:05.214`.
- `python -m platformio pkg pack` -> passed; wrote `LDC1614-1.0.0.tar.gz`, which was removed after packaging.
- `idf.py --version` -> failed locally: `idf.py` is not recognized as a cmdlet, function, script file, or operable program.
- `idf.py -C examples/esp_idf/basic set-target esp32s3 build` -> not run locally because `idf.py` is unavailable.
- `idf.py -C examples/esp_idf/basic set-target esp32s2 build` -> not run locally because `idf.py` is unavailable.

### Remaining work

- No hardware/HIL validation logs exist yet.
- Pure ESP-IDF build success remains unclaimed locally until `idf.py` or CI logs are available.
- Original audit clock/divider and sleep-before-configuration concerns were
  reopened in the reconciliation pass; software-enforceable items are now
  covered by code/tests, while physical clock-plan limits remain board/HIL
  validation work.
- Native coverage is instrumented but still lacks a local percentage report because `gcovr` is unavailable.

## Prompt 02 - H3/M2 timing and freshness

### Findings addressed

- H3 blocking latency contracts: blocking read helpers now require an injected
  monotonic `Config::nowMs` for wall-clock timeouts, validate parameters before
  polling/I2C/yield, and return `INVALID_CONFIG` when the timebase is absent.
- M2 `dataReady()` convenience behavior: documentation now states that `false`
  can mean not ready or hidden error; production docs prefer `readDataReady()`.
- Freshness/autoscan gap: added STATUS/`UNREADCONVx`-driven fresh-channel
  readout so autoscan code can avoid treating latest-register values as fresh.
- Cache timestamp gap: cached sample validity no longer depends on timestamp
  being nonzero, so nonblocking reads remain cacheable without `nowMs`.

### Implemented changes

- Added `FreshChannelData` and `readFreshChannels()` overloads.
- `readFreshChannels()` reads STATUS once, preserves the caller-visible
  `DeviceStatus` snapshot when requested, reads only channels with
  `UNREADCONVx`, and returns cached stale samples with `valid=true/fresh=false`.
- `readDataReady()` now checks OFFLINE/BUSY state before the INTB no-I2C fast
  path so blocking waits cannot spin on a latched offline driver.
- `readChannelBlocking()` and `readAllChannelsBlocking()` reject missing
  `nowMs` before polling and use a saturated finite poll cap.
- `readAllChannelsBlocking()` validates `out`, effective `count`, and channel
  count before readiness polling or yield.
- Replaced timestamp-as-cache-sentinel behavior with explicit per-channel
  sample-valid flags.
- Updated README, public Doxygen, and hardware integration docs with latency,
  freshness, data-ready, and conversion-timing contracts.

### Public API changes

- Added `FreshChannelData`.
- Added `Status readFreshChannels(FreshChannelData* out, uint8_t count = 0)`.
- Added `Status readFreshChannels(FreshChannelData* out, DeviceStatus& statusOut, uint8_t count = 0)`.
- `sampleTimestampMs(ch)` can now return `0` for a valid cached sample; callers
  should use `hasSample(ch)` to test validity.

### Tests added

- Blocking helpers reject missing `nowMs` before I2C/yield.
- Invalid `readAllChannelsBlocking()` calls return before wait/I2C/yield.
- Blocking helpers return `BUSY` while OFFLINE before GPIO/INTB polling.
- Sleeping blocking preconditions do not touch I2C or yield.
- `readDataReady()` preserves granular I2C error codes/details.
- `dataReady()` collapses failures to `false` while preserving health detail.
- INTB asserted by sensor error returns `SENSOR_ERROR` with STATUS raw detail.
- INTB asserted plus STATUS read failure preserves granular transport detail.
- All-channel blocking propagates readiness failure before DATAx reads.
- `readFreshChannels()` reads only `UNREADCONVx` channels, returns stale cached
  data as non-fresh, preserves the STATUS snapshot, validates before STATUS
  read, and ignores LDC1614-only unread bits when `channelCount=2`.
- Cached samples remain valid when timestamp is zero or no `nowMs` hook exists.

### Commands run

- `git status --short` -> clean at prompt start.
- `git branch --show-current` -> `hardening/ldc1614-industry-readiness`.
- `git checkout hardening/ldc1614-industry-readiness` -> already on branch and up to date.
- `python tools/check_core_timing_guard.py` -> `Core timing/framework guard PASSED`.
- `python tools/check_cli_contract.py` -> `CLI contract PASSED`.
- `python tools/check_idf_example_contract.py` -> `IDF example contract PASSED`.
- `python tools/check_readiness_claims.py` -> `Readiness claims guard PASSED`.
- `python scripts/generate_version.py check` -> `Up to date: C:\Users\Honza\Documents\Projects\LDC1614\include\LDC1614\Version.h`.
- `python -m platformio test -e native` -> passed; `114 test cases: 114 succeeded in 00:00:02.124`.
- `python -m platformio run -e esp32s3dev` -> passed; `esp32s3dev SUCCESS 00:00:06.950`.
- `python -m platformio run -e esp32s2dev` -> passed; `esp32s2dev SUCCESS 00:00:05.348`.
- `python -m platformio pkg pack` -> passed; wrote `LDC1614-1.0.0.tar.gz`, which was removed after packaging.
- `git diff --check` -> passed; Git reported only local LF-to-CRLF normalization warnings.

### Remaining related work

- Pure ESP-IDF build success remains unclaimed locally until `idf.py` or CI logs
  are available; Prompt 02 did not run pure IDF builds.
- No hardware/HIL validation is claimed.
- Conversion timing helpers remain estimates and still need board-level timing
  evidence for release claims.
- Native tests do not model hardware side effects where reading DATAx or STATUS
  clears `UNREADCONVx`; hardware/HIL should cover that behavior.

## Prompt 02 Reconciliation / Timing and Freshness Closure

### Evidence reviewed

- Local git history shows Prompt 02 was already implemented on this branch in
  commit `68b380e` (`Tighten LDC1614 timing and data-ready semantics`), after
  Prompt 05 had initially been committed.
- Reviewed `include/LDC1614/LDC1614.h`, `include/LDC1614/Config.h`,
  `include/LDC1614/CommandTable.h`, `src/LDC1614.cpp`, `test/test_basic.cpp`,
  README timing/freshness docs, `docs/HARDWARE_INTEGRATION.md`, and the final
  hardening report.
- Subagent read-only findings confirmed Prompt 02 was not missing, but found
  two closure items: STATUS sensor errors could be hidden when DRDY was also
  set, and full configuration apply should force device sleep before writing
  channel/global registers.
- Datasheet extracts reviewed locally:
  `docs/pdf-extracted-md/LDC1614_datasheet.md` and `_txt/datasheet_LDC1614.txt`
  for RCOUNTx, SETTLECOUNTx, CLOCK_DIVIDERSx, CONFIG.SLEEP_MODE_EN,
  MUX_CONFIG/deglitch, and Table 43 multi-channel requirements.

### What was already implemented

- Blocking read helpers validate parameters and `Config::nowMs` before waiting,
  touching I2C, or calling `cooperativeYield`.
- Blocking read helpers require injected monotonic time for wall-clock
  readiness waits and keep a finite saturated poll cap.
- `readFreshChannels()` and `FreshChannelData` provide STATUS/`UNREADCONVx`
  driven autoscan freshness while preserving latest-register read semantics for
  `readAllChannels()`.
- `dataReady()` remains a documented convenience wrapper, and production docs
  prefer `readDataReady()`.
- Conversion timing helpers and README docs describe the datasheet timing
  approximation and avoid hardware-proven sample-rate claims.

### What was missing and fixed now

- `readDataReady()` now returns `SENSOR_ERROR` when STATUS contains sensor error
  flags, even if DRDY is also set. The `ready` output still reflects DRDY so a
  caller can distinguish "ready with sensor error" from transport failure.
- `dataReady()` now returns `false` for the DRDY-plus-sensor-error case because
  it intentionally collapses all non-OK readiness results.
- Autoscan Table 43 minima are now explicit driver constraints:
  selected autoscan channels require `RCOUNTx >= 0x0009` and
  `SETTLECOUNTx >= 0x0004`.
- `begin()`, `syncConfig()`, recovery reapply, and `resetAndReapply()` now share
  `_applyConfig()` behavior that writes a sleeping CONFIG image before channel
  and global configuration writes, then writes the final sleeping CONFIG image.
- Public Doxygen, README, and hardware integration docs now document the
  sleep-first full-apply sequence and the board-owned clock-plan limits.

### What was documented instead of changed

- The core still does not accept actual reference-clock frequency, sensor
  frequency range, coil tolerance, or deglitch margin as configuration inputs.
  Therefore it cannot enforce `fINx < fREFx/4`, the actual 1 kHz to 10 MHz
  sensor operating range, internal/external clock accuracy, or
  `FIN_DIVIDERx >= 2` when the actual sensor frequency is at least 8.75 MHz.
- Those constraints are documented as application clock-plan and HIL/bench
  validation duties rather than silently guessed by the driver.
- Conversion helper outputs remain estimates for scheduling and diagnostics, not
  hardware validation evidence.

### Datasheet references

- `docs/pdf-extracted-md/LDC1614_datasheet.md`: RCOUNTx valid range
  `0x0005..0xFFFF`, SETTLECOUNTx `0/1` minimum behavior, CLOCK_DIVIDERSx field
  ranges/reserved bits, CONFIG.SLEEP_MODE_EN polarity, and MUX_CONFIG deglitch
  field text.
- `_txt/datasheet_LDC1614.txt`: Table 43 multi-channel requirements including
  `SETTLECOUNTx > 3`, `RCOUNTx > 8`, valid `fREFx`, and `fINx < fREFx/4`.

### Tests added

- Begin rejects autoscan selected-channel `RCOUNTx < 0x0009` or
  `SETTLECOUNTx < 0x0004` before any I2C.
- Runtime autoscan setters reject selected-channel Table 43 minima before I2C.
- Begin and runtime clock-divider tests cover FIN/FREF reserved values and
  reserved-bit-clear encoding.
- Runtime `setClockDividers()` requires sleep before I2C.
- `readDataReady()` reports `SENSOR_ERROR` for STATUS DRDY plus error flags on
  both STATUS polling and INTB asserted paths.
- `dataReady()` collapses DRDY plus sensor error to `false`.
- `begin()` and `syncConfig()` write CONFIG sleep before channel registers.

### Commands run

- `git status --short` -> clean at reconciliation start.
- `git branch --show-current` -> `hardening/ldc1614-industry-readiness`.
- `python -m platformio test -e native` -> initially failed once due to a local
  redeclared `Status st` while editing `_applyConfig()`, then passed with
  `126 test cases: 126 succeeded in 00:00:01.210`.
- `python tools/check_core_timing_guard.py` -> `Core timing/framework guard PASSED`.
- `python tools/check_cli_contract.py` -> `CLI contract PASSED`.
- `python tools/check_idf_example_contract.py` -> `IDF example contract PASSED`.
- `python tools/check_readiness_claims.py` -> `Readiness claims guard PASSED`.
- `python scripts/generate_version.py check` -> `Up to date: C:\Users\Honza\Documents\Projects\LDC1614\include\LDC1614\Version.h`.
- `python -m platformio test -e native` -> passed; `126 test cases: 126 succeeded in 00:00:01.151`.
- `python -m platformio run -e esp32s3dev` -> passed; `esp32s3dev SUCCESS 00:00:05.389`.
- `python -m platformio run -e esp32s2dev` -> passed; `esp32s2dev SUCCESS 00:00:04.977`.
- `python -m platformio pkg pack` -> passed; wrote `LDC1614-1.0.0.tar.gz`, which was removed after packaging.
- `python -m platformio test -e native_cov` -> passed; `126 test cases: 126 succeeded in 00:00:01.484`.
- `python -m gcovr --version` -> failed; `No module named gcovr`.
- `idf.py --version` -> failed; `idf.py` is not recognized as a cmdlet/function/script/program.
- Pure ESP-IDF `esp32s3`/`esp32s2` builds were not run locally because `idf.py`
  is unavailable.
- `git diff --check` -> passed; Git reported only local LF-to-CRLF normalization warnings.

### Remaining blockers

- No real LDC1612/LDC1614 hardware/HIL logs are present on this branch.
- Pure ESP-IDF local build evidence remains unavailable until `idf.py` is
  installed or CI logs are reviewed.
- Board-specific clock-plan validation, STATUS/DATAx side effects, INTB/SD
  wiring, fault injection, and soak evidence remain release blockers.
