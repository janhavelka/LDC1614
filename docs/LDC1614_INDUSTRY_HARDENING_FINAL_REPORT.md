# LDC1614 Industry Hardening Final Report

Date: 2026-06-08
Branch: `hardening/ldc1614-industry-readiness`
Mode: Sequential hardening prompts through Prompt 05, with Prompt 02 reconciled

Latest merge/release cleanup status is tracked in
`docs/LDC1614_MERGE_RELEASE_CLEANUP_REPORT.md`. This report remains the
authoritative hardening status summary; the cleanup report records final
documentation hygiene, local command evidence, and sync status for merge review.

## Starting Audit Summary

The exploration audit classified the repo as engineering-grade with major gaps:
the core architecture was already framework-neutral and transport-injected, but
production/readiness wording, partial hardware-state handling, ESP-IDF path
quality, test depth, and hardware validation evidence were not sufficient for a
field-readiness claim.

This branch is materially stronger after the hardening work, but it is not a
release or deployment certificate. Real LDC1612/LDC1614 hardware logs are still
absent from this branch.

## Finding Status

Note: the tailored prompt sequence used some finding IDs differently from the
original exploration report. This table follows the hardening prompt sequence
where practical and calls out deferred original-audit items explicitly.

| Finding | Status | Evidence / disposition |
| --- | --- | --- |
| H1 unsupported readiness claims | Partially fixed; validation deferred | `library.json`, README, IDF docs, and examples now avoid unsupported production/industry wording. HIL docs and runner exist. No hardware logs are present. |
| H2 partial hardware-state handling | Fixed | `hardwareConfigDirty()`, `hardwareConfigDirtyError()`, `syncConfig()`, recovery dirty handling, and partial-write tests were added in Prompt 01. |
| H3 timing/status/freshness contracts | Fixed for software contract; hardware timing validation deferred | Blocking helpers require `Config::nowMs`, validate before polling/I2C/yield, preserve readiness errors, and report STATUS sensor errors even when DRDY is set. `readFreshChannels()` provides STATUS/`UNREADCONVx`-driven freshness. Software-enforceable clock/config sequencing constraints are covered; physical clock-plan validation remains board/HIL work. |
| H4 ESP-IDF path | Fixed for source/build configuration; local IDF build deferred | IDF example now uses native fixed-buffer CLI, native I2C path, mutex, plain INTB input, guarded source list, and CI IDF build job. Local `idf.py` is unavailable. |
| M1 raw register diagnostics | Fixed by contract | Raw helpers are documented as diagnostic, not variant/access-type safe, and raw writes mark dirty. Native tests cover boundaries and policy. |
| M2 `dataReady()` convenience error collapse | Fixed by documentation/tests | README prefers `readDataReady()` for precise status. `dataReady()` remains documented convenience behavior returning false for not-ready or hidden transport/status/sensor errors. |
| M3 copy/move/thread/ISR contracts | Fixed | Copy/move deleted; Doxygen/README state no internal thread safety, no ISR safety, external serialization required. |
| M4 native tests/fault injection | Mostly fixed | Native fake bus now supports granular failure injection and transaction logs. Native test count is 126 after reconciliation. Coverage instrumentation exists, but no local percentage report because `gcovr` is unavailable. |
| M5 datasheet notes/nits | Fixed | Deglitch ambiguity, IDRIVE docs, reset timing assumptions, and `probe()` caveat are documented and tested where practical. |
| M6 INTB IDF handling | Fixed | IDF example configures INTB as plain input and documents push-pull active-low behavior. |
| M7 example labels/docs honesty | Fixed | Arduino and IDF examples are labeled diagnostic bring-up; README separates software architecture from hardware validation. |
| L1 reproducible version metadata | Fixed for generator path | `scripts/generate_version.py` supports `SOURCE_DATE_EPOCH` and `LDC1614_REPRODUCIBLE_BUILD=1`; generated fallback no longer uses compiler `__DATE__`/`__TIME__`. |
| L2 `probe()` docs | Fixed | Public docs state callbacks are required; default-constructed instances return `INVALID_CONFIG`. |
| L3 IDF target metadata | Fixed | `idf_component.yml` declares intended `esp32s2` and `esp32s3` targets. |

## Changes by Prompt

Prompt 01:
- Added dirty/sync-needed public contract and recovery behavior.
- Deleted copy/move for `LDC1614`.
- Documented thread/ISR/callback recursion contracts.
- Documented raw diagnostic write cache desynchronization.

Prompt 02:
- Added `FreshChannelData` and `readFreshChannels()` overloads for
  STATUS/`UNREADCONVx`-driven autoscan freshness.
- Required `Config::nowMs` for blocking read wall-clock timeouts.
- Validated blocking helper arguments before polling/I2C/yield.
- Preserved granular readiness and INTB STATUS errors.
- Replaced timestamp-as-cache-sentinel behavior with explicit sample-valid
  state.
- Enforced autoscan Table 43 minima that are visible from configuration:
  selected channels require `RCOUNTx >= 0x0009` and `SETTLECOUNTx >= 0x0004`.
- Full configuration apply now forces CONFIG sleep before channel/global writes
  and leaves the device asleep.
- Added README/Doxygen latency, freshness, and conversion-timing contracts.

Prompt 03:
- Replaced the ESP-IDF shared CLI path with native fixed-buffer CLI sources.
- Added native IDF transport locking, bounded console polling, precise-enough
  status mapping, and plain INTB input setup.
- Added IDF source guards and CI IDF build configuration.

Prompt 04:
- Expanded fake transport and negative/variant/fault tests.
- Strengthened core/IDF guards and added a coverage-instrumented native env.
- Added generated-version check to CI.

Prompt 05:
- Removed unsupported readiness wording from maintained metadata/docs.
- Added `docs/HARDWARE_INTEGRATION.md`.
- Added `docs/HIL_VALIDATION.md` and `docs/hil/README.md`.
- Added `tools/ldc1614_hil_runner.py`.
- Added `tools/check_readiness_claims.py` and CI wiring.
- Added reproducible build metadata controls in `scripts/generate_version.py`.
- Added this final report.

## Public API Changes

Public API changes came from earlier hardening prompts, not Prompt 05:
- `Err::CONFIG_DIRTY`
- `hardwareConfigDirty()`
- `hardwareConfigDirtyError()`
- `syncConfig()`
- `SettingsSnapshot::hardwareConfigDirty`
- `SettingsSnapshot::hardwareConfigDirtyError`
- Deleted copy/move construction and assignment for `LDC1614`
- `FreshChannelData`
- `readFreshChannels(FreshChannelData* out, uint8_t count = 0)`
- `readFreshChannels(FreshChannelData* out, DeviceStatus& statusOut, uint8_t count = 0)`
- `sampleTimestampMs(ch)` can now return `0` for a valid cached sample; callers
  should use `hasSample(ch)` to test validity.

Prompt 05 added no core public API.

## Tests and Guards Added

- Native tests now cover partial config failures, raw diagnostic dirty policy,
  LDC1612/LDC1614 variant bounds, DATAx ordering, granular transport errors,
  Wire adapter mapping, readiness/freshness semantics, blocking precondition
  ordering, Table 43 autoscan minima, clock-divider field encoding, sleep-first
  full configuration apply, explicit zero-timestamp cache validity, and recovery
  callback edge cases.
- `tools/check_core_timing_guard.py` now bans broader framework/timing/allocation
  leakage from `include/` and `src/`.
- `tools/check_idf_example_contract.py` now parses actual IDF CMake sources and
  bans Arduino/shared-CLI leakage from the native IDF path.
- `tools/check_readiness_claims.py` guards maintained release surfaces against
  unsupported wording.
- `native_cov` exists for coverage-instrumented native test runs.

## Prompt 02 Reconciliation / Timing and Freshness Closure

### Evidence reviewed

- Git history shows Prompt 02 was implemented on this branch in commit
  `68b380e` (`Tighten LDC1614 timing and data-ready semantics`), so the work
  was partial/stale in the reports rather than absent from code.
- Reviewed `include/LDC1614/LDC1614.h`, `include/LDC1614/Config.h`,
  `include/LDC1614/CommandTable.h`, `src/LDC1614.cpp`, `test/test_basic.cpp`,
  README, `docs/HARDWARE_INTEGRATION.md`, this final report, and local
  datasheet extracts.
- Subagent review found no broad framework leakage or false hardware validation
  claim. It did identify two concrete closure items: DRDY plus STATUS sensor
  error priority, and sleep-before-full-configuration sequencing.

### What was already implemented

- Blocking read helpers validate arguments and `Config::nowMs` before polling,
  I2C, or yielding.
- Blocking read helpers require injected monotonic time for wall-clock
  readiness waits.
- `readFreshChannels()` uses STATUS/`UNREADCONVx` for autoscan freshness and
  keeps `readAllChannels()` as latest-register semantics.
- README/Doxygen already described conversion timing as an estimate, not a
  hardware-proven sample-rate guarantee.

### What was missing and fixed now

- `readDataReady()` now reports `SENSOR_ERROR` when STATUS has sensor error bits
  even if DRDY is also set; `ready` still reflects DRDY.
- `dataReady()` now collapses that sensor-error readiness result to `false`,
  matching its convenience-only contract.
- Autoscan Table 43 minima are enforced where the driver has enough information:
  selected channels require `RCOUNTx >= 0x0009` and
  `SETTLECOUNTx >= 0x0004`.
- `begin()`, `syncConfig()`, recovery reapply, and `resetAndReapply()` now force
  CONFIG sleep before channel/global configuration writes and leave the device
  asleep after full apply.
- README, Doxygen, and hardware integration docs now document these contracts.

### What was documented instead of changed

- The driver still cannot validate physical clock-plan facts it does not know:
  actual `fCLK`, actual `fREFx`, `fINx < fREFx/4`, sensor frequency across
  tolerance/temperature/target variation, deglitch margin, or the 8.75 MHz
  `FIN_DIVIDERx >= 2` rule.
- Those constraints are documented as application design and HIL/bench
  validation duties.

### Datasheet references

- `docs/pdf-extracted-md/LDC1614_datasheet.md`: RCOUNTx range,
  SETTLECOUNTx behavior, CLOCK_DIVIDERSx fields/reserved bits,
  CONFIG.SLEEP_MODE_EN, MUX_CONFIG deglitch field text.
- `_txt/datasheet_LDC1614.txt`: Table 43 multi-channel requirements including
  `SETTLECOUNTx > 3`, `RCOUNTx > 8`, valid `fREFx`, and `fINx < fREFx/4`.

### Tests added

- Autoscan Table 43 begin/runtime rejection tests before I2C.
- Clock-divider field bounds and reserved-bit-clear encoding tests.
- Runtime `setClockDividers()` sleep precondition test.
- `readDataReady()` and `dataReady()` tests for DRDY plus STATUS sensor errors.
- `begin()` and `syncConfig()` write-order tests proving CONFIG sleep precedes
  channel register writes.

### Commands run

- Reconciliation started clean on branch `hardening/ldc1614-industry-readiness`.
- Native tests initially failed once during local editing due to a redeclared
  `Status st`, then passed at `126/126` after the fix.
- Full guard/build/package results for this reconciliation commit are listed in
  the command table below.

### Remaining blockers

- No real LDC1612/LDC1614 hardware/HIL logs are present.
- Local pure ESP-IDF build evidence remains unavailable because `idf.py` is not
  installed in this environment.
- Board-specific clock-plan validation and real STATUS/DATAx side-effect
  evidence remain release blockers.

## Commands Run in Reconciliation Pass

| Command | Result |
| --- | --- |
| `git status --short` | Clean at reconciliation start |
| `git branch --show-current` | `hardening/ldc1614-industry-readiness` |
| `git log --oneline --decorate --all --grep="timing\\|fresh\\|data-ready\\|Prompt 02\\|clock\\|divider\\|UNREAD\\|blocking" -50` | Found Prompt 02 commits including `68b380e` |
| `rg -n "readChannelBlocking\|readAllChannelsBlocking\|readDataReady\|dataReady\|UNREAD\|UNREADCONV\|nowMs\|timeoutMs\|conversion\|RCOUNT\|SETTLECOUNT\|CLOCK_DIVIDERS\|Table 43\|sleep\|wake\|CONFIG\|MUX_CONFIG" include src test README.md docs` | Reviewed timing/freshness/config paths |
| `python -m platformio test -e native` | First run during edit failed on a local `Status st` redeclaration; final run passed 126/126 in 00:00:01.151 |
| `python tools/check_core_timing_guard.py` | `Core timing/framework guard PASSED` |
| `python tools/check_cli_contract.py` | `CLI contract PASSED` |
| `python tools/check_idf_example_contract.py` | `IDF example contract PASSED` |
| `python tools/check_readiness_claims.py` | `Readiness claims guard PASSED` |
| `python scripts/generate_version.py check` | `Up to date: ... include\\LDC1614\\Version.h` |
| `python -m platformio run -e esp32s3dev` | Passed; `esp32s3dev SUCCESS 00:00:05.389` |
| `python -m platformio run -e esp32s2dev` | Passed; `esp32s2dev SUCCESS 00:00:04.977` |
| `python -m platformio pkg pack` | Passed; wrote `LDC1614-1.0.0.tar.gz`, then tarball removed |
| `python -m platformio test -e native_cov` | Passed; 126/126 in 00:00:01.484 |
| `python -m gcovr --version` | Failed locally: `No module named gcovr` |
| `idf.py --version` | Failed locally: `idf.py` is not recognized as a cmdlet/function/script/program |
| `idf.py -C examples/esp_idf/basic set-target esp32s3 build` | Not run locally because `idf.py` is unavailable |
| `idf.py -C examples/esp_idf/basic set-target esp32s2 build` | Not run locally because `idf.py` is unavailable |
| `git diff --check` | Passed; only LF-to-CRLF normalization warnings |

## Commands Run in Prompt 05

| Command | Result |
| --- | --- |
| `git status --short` | Clean at prompt start |
| `git branch --show-current` | `hardening/ldc1614-industry-readiness` |
| `git checkout hardening/ldc1614-industry-readiness` | Already on branch; up to date |
| `python -m py_compile scripts/generate_version.py tools/ldc1614_hil_runner.py tools/check_readiness_claims.py` | Passed |
| `python scripts/generate_version.py sync` | Updated ignored generated `include/LDC1614/Version.h` |
| `python tools/check_core_timing_guard.py` | `Core timing/framework guard PASSED` |
| `python tools/check_cli_contract.py` | `CLI contract PASSED` |
| `python tools/check_idf_example_contract.py` | `IDF example contract PASSED` |
| `python tools/check_readiness_claims.py` | `Readiness claims guard PASSED` |
| `python scripts/generate_version.py check` | `Up to date: ... include\\LDC1614\\Version.h` |
| `python -m platformio test -e native` | Passed; 105/105 tests in 00:00:01.333 |
| `python -m platformio run -e esp32s3dev` | Passed; SUCCESS in 00:00:05.889 |
| `python -m platformio run -e esp32s2dev` | Passed; SUCCESS in 00:00:05.214 |
| `python -m platformio pkg pack` | Passed; wrote `LDC1614-1.0.0.tar.gz`, then tarball removed |
| `idf.py --version` | Failed locally: `idf.py` is not recognized as a cmdlet/function/script/program |
| `idf.py -C examples/esp_idf/basic set-target esp32s3 build` | Not run locally because `idf.py` is unavailable |
| `idf.py -C examples/esp_idf/basic set-target esp32s2 build` | Not run locally because `idf.py` is unavailable |
| `python tools/ldc1614_hil_runner.py --json-out .pio\\hil_not_run.json --markdown-out .pio\\hil_not_run.md --quiet` | Produced ignored dry-run artifact with `overall_status: NOT_RUN`, reason `serial port was not supplied` |

## Commands Run in Prompt 02 Backfill

| Command | Result |
| --- | --- |
| `git status --short` | Clean at prompt start |
| `git branch --show-current` | `hardening/ldc1614-industry-readiness` |
| `git checkout hardening/ldc1614-industry-readiness` | Already on branch; up to date |
| `python tools/check_core_timing_guard.py` | `Core timing/framework guard PASSED` |
| `python tools/check_cli_contract.py` | `CLI contract PASSED` |
| `python tools/check_idf_example_contract.py` | `IDF example contract PASSED` |
| `python tools/check_readiness_claims.py` | `Readiness claims guard PASSED` |
| `python scripts/generate_version.py check` | `Up to date: ... include\\LDC1614\\Version.h` |
| `python -m platformio test -e native` | Passed; 114/114 tests in 00:00:02.124 |
| `python -m platformio run -e esp32s3dev` | Passed; SUCCESS in 00:00:06.950 |
| `python -m platformio run -e esp32s2dev` | Passed; SUCCESS in 00:00:05.348 |
| `python -m platformio pkg pack` | Passed; wrote `LDC1614-1.0.0.tar.gz`, then tarball removed |
| `git diff --check` | Passed; only LF-to-CRLF normalization warnings |

## CI / IDF Status

- GitHub Actions has PlatformIO ESP32-S2/S3 Arduino builds.
- GitHub Actions has native tests and coverage-instrumented native tests.
- GitHub Actions has guard checks for core framework/timing, Arduino CLI, IDF
  source path, generated version sync/check, package pack, and readiness wording.
- GitHub Actions has an ESP-IDF build matrix for `esp32s2` and `esp32s3`.
- The pure ESP-IDF matrix is workflow-configured; treat it as evidence only
  after reviewing actual GitHub Actions logs for the target commit.
- Local pure ESP-IDF builds were not run because `idf.py` is not installed in
  this environment.

## HIL Status

- HIL runner/procedure created:
  - `tools/ldc1614_hil_runner.py`
  - `docs/HIL_VALIDATION.md`
  - `docs/hil/README.md`
- Real hardware logs captured in this prompt: none.
- Dry-run behavior checked: no serial port reports `NOT_RUN`, not pass.
- Pending matrix: identity, both address straps where available, configured
  channels, DATAx error flags, config readback, INTB, SD, unplug/replug,
  induced NACK, stuck-bus fixture, soak, and drive-current tuning evidence.

## Remaining Work

P0 before any release/readiness claim:
- Capture real hardware/HIL logs for the target board and sensor.
- Run or obtain CI logs for pure ESP-IDF `esp32s2` and `esp32s3` builds.
- Validate the board clock plan and sensor configuration on hardware, including
  actual `fCLK`/`fREFx`, `fINx < fREFx/4`, sensor frequency range, deglitch
  margin, INTB/SD wiring, and STATUS/DATAx side effects.

P1 before merge/release:
- Add coverage percentage reporting once `gcovr` or an equivalent tool is
  available.
- Add hardware/HIL coverage for STATUS/DATAx side effects such as `UNREADCONVx`
  clearing behavior after reads.
- Review package consumer behavior around ignored generated `Version.h` in clean
  component/library usage.

P2 later hardening:
- Add safe register metadata for variant/access-type-aware diagnostics.
- Add optional HIL parsing for board-specific SD/INTB/fault fixtures.
- Add release artifact templates for HIL JSON/Markdown and bench notes.

## Merge Recommendation

Ready to merge only as a software hardening branch if reviewers accept the
explicit deferred items. Do not merge as a release branch until the P0 items are
handled or intentionally scoped out.

## Release Recommendation

Not ready for release as a deployment-ready driver. A source/pre-release build
can be described as:
- "Framework-neutral LDC1614/LDC1612 driver under industry-readiness hardening."
- "Production-oriented architecture with injected transport and native tests."
- "Hardware validation pending for the target board and sensor."

Forbidden release wording until evidence exists:
- "production-grade"
- "industry-grade"
- "field-ready"
- "certified"
- "hardware validated"
- "release-ready"
- "production ready"

## Final Verdict

Ready for review as a software hardening branch. Blocked from release by
missing hardware/fault validation, missing local pure ESP-IDF build evidence,
and missing board-level clock/sensor validation evidence. The original-audit
clock/divider/config sequencing concerns that are enforceable in software are
now implemented and tested; remaining clock-plan work requires target hardware
evidence. Not blocked by core architecture.
