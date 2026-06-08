# LDC1614 Industry Hardening Final Report

Date: 2026-06-08
Branch: `hardening/ldc1614-industry-readiness`
Mode: Sequential hardening prompts through Prompt 05, with Prompt 02 backfilled

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
| H3 timing/status/freshness contracts | Fixed for software contract; hardware timing validation deferred | Blocking helpers require `Config::nowMs`, validate before polling/I2C/yield, and preserve readiness errors. `readFreshChannels()` provides STATUS/`UNREADCONVx`-driven freshness. Conversion timing remains an estimate until hardware evidence exists. |
| H4 ESP-IDF path | Fixed for source/build configuration; local IDF build deferred | IDF example now uses native fixed-buffer CLI, native I2C path, mutex, plain INTB input, guarded source list, and CI IDF build job. Local `idf.py` is unavailable. |
| M1 raw register diagnostics | Fixed by contract | Raw helpers are documented as diagnostic, not variant/access-type safe, and raw writes mark dirty. Native tests cover boundaries and policy. |
| M2 `dataReady()` convenience error collapse | Fixed by documentation/tests | README prefers `readDataReady()` for precise status. `dataReady()` remains documented convenience behavior returning false for not-ready or hidden errors while health retains tracked failure detail. |
| M3 copy/move/thread/ISR contracts | Fixed | Copy/move deleted; Doxygen/README state no internal thread safety, no ISR safety, external serialization required. |
| M4 native tests/fault injection | Mostly fixed | Native fake bus now supports granular failure injection and transaction logs. Native test count is 114 after Prompt 02. Coverage instrumentation exists, but no local percentage report because `gcovr` is unavailable. |
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
  ordering, explicit zero-timestamp cache validity, and recovery callback edge
  cases.
- `tools/check_core_timing_guard.py` now bans broader framework/timing/allocation
  leakage from `include/` and `src/`.
- `tools/check_idf_example_contract.py` now parses actual IDF CMake sources and
  bans Arduino/shared-CLI leakage from the native IDF path.
- `tools/check_readiness_claims.py` guards maintained release surfaces against
  unsupported wording.
- `native_cov` exists for coverage-instrumented native test runs.

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
- Resolve original audit clock/divider configuration constraints and any
  remaining sleep-before-configuration sequencing questions outside Prompt 02's
  timing/freshness scope.
- Run or obtain CI logs for pure ESP-IDF `esp32s2` and `esp32s3` builds.

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
and unresolved original-audit clock/configuration constraints outside Prompt
02's timing/freshness scope. Not blocked by core architecture.
