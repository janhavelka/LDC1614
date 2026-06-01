# LDC1614 Industry-Readiness Exploration Report

Date: 2026-06-01
Branch: `audit/ldc1614-industry-readiness-exploration`
Mode: Exploration / audit-only, no production implementation

## Executive Summary

The repository has a solid framework-neutral core, transport-injected I2C, a useful managed synchronous health model, and a meaningful native fake-bus test suite. It is not industry-ready today. The main blockers are incomplete latency contracts, partial multi-register hardware-state handling, ESP-IDF example and CI gaps, overconfident production wording, and lack of real hardware/fault validation evidence.

This is not an architecture-blocked driver. It is ready for chunked implementation hardening, but production-readiness and industry-grade claims must wait until software gaps and hardware validation gaps are closed.

## Readiness Classification

Engineering-grade with major gaps.

The core design is stronger than a prototype: it is framework-neutral, status-returning, transport-injected, and has 70 native tests. It is still not a pre-production candidate because the ESP-IDF path is not validated by pure `idf.py`, the diagnostic IDF CLI still compiles heap-using shared `std::string` code, latency contracts are not tight enough, multi-register write failures can leave partial hardware state, and no real LDC1612/LDC1614 hardware validation logs were found.

## Scope Reviewed

- `AGENTS.md`
- `README.md`
- `CHANGELOG.md`
- `library.json`
- `idf_component.yml`
- `CMakeLists.txt`
- `platformio.ini`
- `.github/workflows/ci.yml`
- `include/LDC1614/CommandTable.h`
- `include/LDC1614/Config.h`
- `include/LDC1614/LDC1614.h`
- `include/LDC1614/Status.h`
- `include/LDC1614/Version.h`
- `src/LDC1614.cpp`
- `test/test_basic.cpp`
- `test/stubs/Arduino.h`
- `test/stubs/Wire.h`
- `tools/check_core_timing_guard.py`
- `tools/check_cli_contract.py`
- `tools/check_idf_example_contract.py`
- `scripts/generate_version.py`
- `examples/01_basic_bringup_cli/main.cpp`
- `examples/common/*`
- `examples/esp_idf/basic/*`
- `docs/IDF_PORT.md`
- `docs/IDF_PORT_IMPLEMENTATION.md`
- `docs/LDC1614_INDUSTRY_READINESS_IMPLEMENTATION_PLAN.md`
- `docs/extracted-md/*`
- `docs/pdf-extracted-md/LDC1614_datasheet.md`
- `docs/application_notes/*`
- `docs/howto_guides/*`
- `_txt/datasheet_LDC1614.txt`
- `prompts/*`

## Datasheet and Local Documentation Sources

- `docs/LDC1614_datasheet.pdf` - primary TI LDC1612/LDC1614 datasheet.
- `docs/pdf-extracted-md/LDC1614_datasheet.md` - extracted datasheet text. Used for pin descriptions, register map, timing, modes, INTB behavior, and application setup.
- `_txt/datasheet_LDC1614.txt` - raw extracted datasheet text. Used to confirm sensor range, supply range, address pin behavior, INTB, SD, and DATAx ordering.
- `docs/extracted-md/00_document_inventory.md` - local datasheet inventory and traceability.
- `docs/extracted-md/01_chip_overview.md` - 2-channel/4-channel, 28-bit, address, clock, and DATAx overview.
- `docs/extracted-md/02_pinout_and_signals.md` - SCL/SDA pull-ups, ADDR, INTB push-pull, SD, CLKIN.
- `docs/extracted-md/03_electrical_and_timing.md` - supply range, sensor frequency range, I2C timing, conversion and settle timing.
- `docs/extracted-md/04_protocol_commands_and_transactions.md` - 16-bit register protocol and DATAx MSB-before-LSB coherency.
- `docs/extracted-md/05_register_map.md` - local register map summary.
- `docs/extracted-md/06_modes_interrupts_status_and_faults.md` - sleep/shutdown, autoscan, STATUS, ERROR_CONFIG, INTB, stale-data behavior.
- `docs/extracted-md/07_initialization_reset_and_operational_notes.md` - initialization sequence, reset, clock, drive-current notes.
- `docs/extracted-md/08_variant_differences_and_open_questions.md` - LDC1612/LDC1614 variant and datasheet ambiguity notes.
- `docs/application_notes/sensor_status_monitoring.md` - STATUS, DATAx error bits, INTB behavior, sticky flags.
- `docs/application_notes/sensor_configuration_settings.md` - IDRIVE and oscilloscope/application calibration notes.
- `docs/application_notes/sensor_design.md` - coil design and 1 kHz to 10 MHz sensor frequency context.

Datasheet references used in findings include: pin descriptions around `docs/pdf-extracted-md/LDC1614_datasheet.md:212`, address behavior around `docs/pdf-extracted-md/LDC1614_datasheet.md:836`, DATAx/register map around `docs/pdf-extracted-md/LDC1614_datasheet.md:913`, STATUS/ERROR_CONFIG around `docs/pdf-extracted-md/LDC1614_datasheet.md:1458`, clock/deglitch around `docs/pdf-extracted-md/LDC1614_datasheet.md:1735` and `docs/pdf-extracted-md/LDC1614_datasheet.md:2600`, and INTB/status behavior around `docs/pdf-extracted-md/LDC1614_datasheet.md:2629`.

## Repository Map

- Core public API: `include/LDC1614/CommandTable.h`, `Config.h`, `LDC1614.h`, `Status.h`, generated `Version.h`.
- Core implementation: `src/LDC1614.cpp`.
- Arduino example: `examples/01_basic_bringup_cli/main.cpp`.
- Example helper layer: `examples/common/`, including Wire transport, CLI, scanner, health diagnostics, board config, and bus diagnostics.
- ESP-IDF example: `examples/esp_idf/basic/`, including native IDF `app_main`, `driver/i2c_master.h` transport, and CMake component files.
- Tests: `test/test_basic.cpp` with a register-aware fake bus and 70 Unity tests.
- Tooling: `tools/check_core_timing_guard.py`, `tools/check_cli_contract.py`, `tools/check_idf_example_contract.py`, `scripts/generate_version.py`.
- CI: `.github/workflows/ci.yml` runs PlatformIO Arduino builds, native tests, one core guard, one CLI guard, and package packing.
- Package metadata: `library.json`, `idf_component.yml`, root `CMakeLists.txt`, `platformio.ini`.
- Docs: README, IDF docs, extracted datasheet/application notes, industry-readiness implementation plan.
- HIL: no HIL runner, `hil_reports/`, or `docs/hil/` directory was found. HIL appears only in prompts/plans.

## Scorecard

| Area | Rating | Notes |
| --- | --- | --- |
| Core framework neutrality | Strong | `include/` and `src/` are standard/local-header only; forbidden token scan found no Arduino/ESP-IDF/FreeRTOS/logging/framework timing usage. |
| I2C ownership/injection | Strong | `Config` injects callbacks and opaque context; core does not own Wire, IDF bus, pins, or bus reset policy. |
| Status/error model | Good | Fallible APIs mostly return `Status`; `dataReady()` is a convenience exception that hides errors as `false`. |
| Device ID / variant handling | Good | `begin()`/`probe()` read manufacturer/device IDs; high-level channels enforce 2 or 4 channels. |
| Register map correctness | Good | Constants match local register map; raw diagnostic helpers are not variant/access-type gated. |
| DATAx error-bit handling | Good | `readChannel()` reads MSB before LSB and masks DATAx MSB error bits. |
| Conversion timing model | Medium | Helpers exist and tests cover nominal cases, but latency contracts and scan-cycle use are incomplete. |
| Multi-channel sequencing | Medium | Autoscan modes exist; data collection does not use `UNREADCONVx` and can read stale channels after one DRDY. |
| Partial hardware state handling | Weak | Multi-register sequences can partially reach hardware without dirty/sync state or failed-register context. |
| Health/recovery behavior | Good | Managed states, tracked wrappers, manual `recover()`, backoff, and fake-bus tests exist. |
| Thread/ISR contract | Medium | README says single-threaded, but public Doxygen and type semantics do not strongly enforce or document it. |
| ESP-IDF component readiness | Medium | Root CMake exists; `idf_component.yml` lacks explicit targets and pure `idf.py` CI validation. |
| ESP-IDF example quality | Weak | Native IDF boundary exists, but example compiles shared `std::string` CLI, uses global state, blocking `getchar()`, no locking, and imperfect error mapping. |
| Arduino example quality | Medium | Useful bring-up CLI, but not clearly labeled as diagnostic only in docs and can run long blocking commands. |
| Native tests/fault injection | Medium | 70 tests are useful; gaps remain for nth-call failures, NACK/data NACK/bus errors, variant bounds, and begin identity mismatch. |
| Guard scripts | Medium | Core and CLI guards pass; core guard is narrow and IDF guard is not in CI. |
| CI/build matrix | Medium | PlatformIO S2/S3 and native tests run; no pure ESP-IDF build, no coverage threshold. |
| HIL readiness | Weak | No runner or evidence matrix exists yet. |
| Documentation honesty | Weak | README, `library.json`, and `idf_component.yml` use production-grade language without hardware evidence. |
| Hardware validation evidence | Weak | No real hardware logs found for identity, channel sequencing, INTB, SD, addresses, faults, or soak. |

## What Is Strong

- The core is framework-neutral. `src/LDC1614.cpp:4` and `include/LDC1614/LDC1614.h:5` include local and standard headers only.
- I2C is injected through non-owning callbacks in `include/LDC1614/Config.h:18` and `include/LDC1614/Config.h:30`; raw wrappers pass the injected timeout and user context through at `src/LDC1614.cpp:1119`.
- The core does not configure bus pins, Wire, IDF handles, GPIO modes, or framework timing. INTB is read only through an injected GPIO callback at `src/LDC1614.cpp:354`.
- Identity probing reads `MANUFACTURER_ID` and `DEVICE_ID` in `src/LDC1614.cpp:233`; expected values are defined in `include/LDC1614/CommandTable.h:54`.
- High-level channel count validates LDC1612 versus LDC1614 behavior in `src/LDC1614.cpp:60`, `src/LDC1614.cpp:130`, and `src/LDC1614.cpp:284`.
- DATA read coherency is implemented correctly in `src/LDC1614.cpp:290` and `src/LDC1614.cpp:297`, reading `DATAx_MSB` before `DATAx_LSB`.
- DATAx MSB error bits are parsed separately and masked out of the 28-bit raw value at `src/LDC1614.cpp:302`.
- The status model is explicit and static-string based in `include/LDC1614/Status.h:29`.
- Health state and manual recovery are documented in `include/LDC1614/LDC1614.h:85` and implemented through tracked wrappers around `src/LDC1614.cpp:1143`.
- Native tests exist with a fake register bus starting at `test/test_basic.cpp:23`, and `python -m platformio test -e native` passed 70/70 tests in this pass.

## High-Severity Findings

### H1. Production and industry-readiness claims are not supported by hardware validation

Severity: High

Evidence:
- `README.md:3` describes the library as production-grade.
- `library.json:4` uses "Production-grade" in package metadata.
- `idf_component.yml:2` uses "Production-grade" in component metadata.
- `docs/LDC1614_INDUSTRY_READINESS_IMPLEMENTATION_PLAN.md:101` records missing hardware validation evidence.
- No `tools/ldc1614_hil_runner.py`, `hil_reports/`, or `docs/hil/` path was found.
- Datasheet/application evidence shows hardware-specific behavior that must be measured: ADDR and SD not-floating requirements in `docs/extracted-md/02_pinout_and_signals.md:20`, INTB push-pull behavior in `docs/extracted-md/02_pinout_and_signals.md:21`, and status/INTB clearing behavior in `docs/extracted-md/06_modes_interrupts_status_and_faults.md:45`.

Impact:
- Users may treat a software-only, unvalidated driver as field-ready.
- Real devices may fail due to wrong address wiring, floating SD/ADDR, bad coil drive current, INTB assumptions, channel sequencing, or fault behavior that has not been validated.
- Package metadata can mislead downstream consumers and registries.

Recommended remediation:
- Replace production-grade wording with "framework-neutral driver under hardening" or equivalent until evidence exists.
- Add a hardware validation matrix and a HIL runner or documented manual HIL procedure.
- Store real logs for LDC1612/LDC1614 identity reads, both address options when available, channel reads, INTB, SD, fault/unplug, recovery, and soak.
- Keep software readiness and hardware validation readiness in separate sections.

Suggested tests:
- HIL smoke: read manufacturer/device IDs at `0x2A`.
- HIL optional: read manufacturer/device IDs at `0x2B` if ADDR can be changed.
- HIL: read all enabled channels with DATAx error flags.
- HIL: assert/deassert INTB using DRDY and enabled error routing.
- HIL: SD shutdown/wake if SD is wired.
- HIL: operator-approved unplug/replug and NACK tests.
- HIL: bounded soak with health counters and sample timestamps.

### H2. Multi-register writes can leave partial hardware state without a dirty/sync contract

Severity: High

Evidence:
- `begin()` calls `_applyConfig()` and returns the first error at `src/LDC1614.cpp:192`.
- `_applyConfig()` performs per-channel writes and exits on first failure starting at `src/LDC1614.cpp:1396`.
- `setSingleChannelMode()` writes `MUX_CONFIG` and then `CONFIG`; if the second write fails, hardware may be partly changed while the cached config remains old at `src/LDC1614.cpp:721`.
- Runtime setters generally update a temporary cached state, write one register, then commit cache on success. That pattern is good for single-register setters but does not solve multi-register partial hardware state.

Impact:
- The device can be left in a different mode than the software cache reports.
- Recovery may reapply a cached config but the user cannot tell whether sync is required unless a later operation fails.
- Field diagnostics may misreport the operating mode after a write failure.

Recommended remediation:
- Add a public partial-state contract such as `hardwareConfigDirty`, `syncNeeded`, or a more precise failed-register status detail.
- For multi-register operations, either write in a sequence that is safe on partial failure, or expose a required `syncConfig()`/`resetAndReapply()` path.
- Include failed register address and operation phase in `Status::detail` where possible.
- Document that raw diagnostic writes can desynchronize cache and require re-sync.

Suggested tests:
- Fake transport that fails on the Nth write during `_applyConfig()`.
- Tests for `setSingleChannelMode()` where `MUX_CONFIG` succeeds and `CONFIG` fails.
- Tests for `resetAndReapply()` where reset succeeds and config reapply fails.
- Tests asserting dirty/sync state and recovery behavior after partial writes.

### H3. Blocking API latency contracts are incomplete

Severity: High

Evidence:
- All public I2C operations are documented as blocking in `include/LDC1614/LDC1614.h:88`.
- `_nowMs()` returns `0` when no `Config::nowMs` hook exists at `src/LDC1614.cpp:1494`.
- `readChannelBlocking()` uses deadline logic plus a finite poll cap at `src/LDC1614.cpp:524`; without an advancing clock, the wall-clock bound becomes poll count times transport timeout and yield latency.
- `readAllChannelsBlocking()` shares the same polling behavior at `src/LDC1614.cpp:563` and validates `count` only later through `readAllChannels()` at `src/LDC1614.cpp:327`.
- `recover()` can call injected `busReset` and `hardReset` callbacks whose latency is not bounded by the core at `src/LDC1614.cpp:1349` and `src/LDC1614.cpp:1379`.

Impact:
- A nominal 200 ms blocking read can be much longer if `nowMs` is absent and the transport timeout is large.
- Production loops can miss real-time deadlines when using blocking convenience APIs.
- Recovery latency may surprise applications if callback budgets are not documented.

Recommended remediation:
- Require `nowMs` for blocking helpers, or explicitly document the fallback as poll-count bounded rather than wall-clock bounded.
- Validate all input before any blocking wait.
- Add Doxygen latency tables with I2C transaction counts and callback latency dependencies.
- Add recommended production pattern: poll `readDataReady()` or INTB, then read one channel or only `UNREADCONVx` channels within an application budget.

Suggested tests:
- Native tests measuring poll count and I2C transaction count when `nowMs` is null.
- Native tests confirming invalid `count` returns before any I2C or wait.
- Tests for callback latency accounting in recovery documentation.

### H4. ESP-IDF path is native but not production-ready

Severity: High

Evidence:
- ESP-IDF example uses native headers and `app_main()` in `examples/esp_idf/basic/main/main.cpp:8` and `examples/esp_idf/basic/main/main.cpp:206`.
- ESP-IDF example CMake compiles shared CLI implementation at `examples/esp_idf/basic/main/CMakeLists.txt:5`.
- That shared CLI uses `<string>` and `std::string` at `examples/common/Ldc1614Cli.cpp:8`, `examples/common/Ldc1614Cli.cpp:157`, and `examples/common/Ldc1614Cli.cpp:230`.
- The IDF main loop calls `getchar()` inside an infinite loop at `examples/esp_idf/basic/main/main.cpp:169`.
- IDF transport maps `ESP_ERR_INVALID_RESPONSE` to generic `I2C_ERROR` and does not distinguish address/data NACKs at `examples/esp_idf/basic/main/Ldc1614IdfI2cTransport.cpp:17`.
- CI does not run `idf.py` builds or `tools/check_idf_example_contract.py`; PlatformIO and native CI are visible in `.github/workflows/ci.yml:41`, `.github/workflows/ci.yml:71`, and `.github/workflows/ci.yml:88`.

Impact:
- The "native fixed-buffer ESP-IDF" claim is weakened: input acquisition is fixed-buffer, but parsing uses heap-backed `std::string`.
- The example can stop ticking if console input blocks.
- Users may copy a diagnostic single-task bring-up example into a multi-task production app without locking, timeout, or error-mapping policy.
- Pure ESP-IDF breakage can reach users undetected.

Recommended remediation:
- Split command contracts from parser implementation so ESP-IDF uses fixed buffers, `esp_console`/argtable, or a C parser without `std::string`.
- Label the IDF example as diagnostic bring-up until bus ownership, locking, nonblocking input, and error mapping are production-grade.
- Add `sdkconfig.defaults` if needed and run `idf.py -C examples/esp_idf/basic set-target esp32s3 build` and `esp32s2 build` in CI.
- Wire `tools/check_idf_example_contract.py` into CI.
- Improve IDF error mapping and avoid `ESP_ERROR_CHECK` in example paths where a `Status` result is more instructive.

Suggested tests:
- Static test that IDF example CMake does not compile `examples/common/Ldc1614Cli.cpp` if it remains heap-backed.
- Static test banning `std::string` from ESP-IDF example sources.
- Pure `idf.py` CI matrix for `esp32s3` and `esp32s2`.
- Native/unit test for ESP-IDF error mapping if transport is factored for testability.

## Medium-Severity Findings

### M1. Raw register diagnostics are not variant-aware or access-type-aware

Severity: Medium

Evidence:
- `isValidRegisterAddress()` accepts the LDC1614 map globally at `src/LDC1614.cpp:85`.
- Public `readRegister16()` and `writeRegister16()` only validate generic address range at `src/LDC1614.cpp:1172`.
- LDC1614-only channel 2/3 registers are marked in `include/LDC1614/CommandTable.h:18` and related entries.
- Datasheet/local docs identify channel 2/3 as LDC1614-only in `docs/extracted-md/05_register_map.md:9`.

Impact:
- In `channelCount = 2` mode, raw helpers can touch LDC1614-only registers.
- Diagnostic writes can target read-only identity registers or status/data registers.
- This may be acceptable for a low-level diagnostic escape hatch, but it must be explicitly documented and tested.

Recommended remediation:
- Add access metadata for registers, or document raw helpers as unsafe diagnostic escape hatches.
- Add optional variant-aware validation mode.
- Return precise `INVALID_PARAM` for read-only writes in safe mode.

Suggested tests:
- LDC1612 mode rejects or warns on raw access to channel 2/3 registers in safe mode.
- Writes to `MANUFACTURER_ID` and `DEVICE_ID` are rejected in safe mode.

### M2. `dataReady()` hides transport/status errors

Severity: Medium

Evidence:
- `dataReady()` is documented as a convenience wrapper at `include/LDC1614/LDC1614.h:184`.
- It calls `readDataReady()` and collapses failures to `false` at `src/LDC1614.cpp:342`.
- `readDataReady(bool&)` is the better Status-returning API at `include/LDC1614/LDC1614.h:191`.

Impact:
- Applications using `dataReady()` cannot distinguish "not ready" from I2C failure, offline state, or status-read side effects.
- This weakens the "silent failure is unacceptable" rule unless documented as a convenience-only API.

Recommended remediation:
- Keep `dataReady()` only if clearly documented as non-diagnostic convenience.
- Prefer examples and production docs using `readDataReady()`.
- Consider deprecating or renaming the bool helper.

Suggested tests:
- Existing failure-to-false test is useful; add documentation tests or examples favoring `readDataReady()`.

### M3. Copy/move, thread-safety, and ISR-safety contracts are too weak

Severity: Medium

Evidence:
- `LDC1614` owns mutable state and non-owning transport callbacks in `include/LDC1614/LDC1614.h:489`.
- The class does not visibly delete copy/move construction or assignment.
- README states single-threaded use at `README.md:289`, but the public Doxygen class contract around `include/LDC1614/LDC1614.h:85` does not strongly define thread or ISR safety.

Impact:
- Accidental copies can duplicate a driver's cache, health state, and hooks for the same physical device.
- ISR or multi-task use can race mutable fields and the injected transport.

Recommended remediation:
- Delete copy/move operations unless there is a strong reason to allow them.
- Document "not ISR-safe" and "not internally thread-safe" in public Doxygen.
- Document that locking belongs to the injected transport/application.

Suggested tests:
- Compile-time test or static assert that `LDC1614` is not copyable/movable if that policy is chosen.
- Doxygen/public docs update checked by review.

### M4. Native tests are useful but do not cover enough negative and variant cases

Severity: Medium

Evidence:
- `test/test_basic.cpp` contains 70 `RUN_TEST` entries.
- Fake bus starts at `test/test_basic.cpp:23`.
- Tests cover bad address, bad channel count, low RCOUNT, high IDRIVE, recovery, status parsing, timing helpers, and blocking timeout.
- Missing areas found by audit: invalid `activeChan`, invalid enum casts, bad INTB config, high-current Ch0 constraints at begin, bad `finDivider`, bad `frefDivider`, begin-time wrong IDs, NACK address/data errors, bus errors, nth-write failures, LDC1612 raw channel 2/3 paths, direct soft-reset behavior, hard-reset ladder, and coverage reporting.

Impact:
- Regressions in error mapping, variant enforcement, and partial-write behavior can pass CI.
- Test count is good, but industry readiness needs more precise fault injection and coverage thresholds.

Recommended remediation:
- Extend fake bus with call-count and register-specific failure injection.
- Add test tables for validation combinations.
- Add coverage tooling or at least line/branch coverage reports for native tests.

Suggested tests:
- Begin wrong manufacturer and wrong device ID.
- `readChannel(2)` and all setters in `channelCount = 2`.
- `I2C_NACK_ADDR`, `I2C_NACK_DATA`, `I2C_BUS`.
- Mid-`_applyConfig()` failures at each register class.
- MSB-before-LSB register access order assertion.

### M5. Datasheet ambiguity and documentation nits need explicit treatment

Severity: Medium

Evidence:
- Deglitch 33 MHz code: code uses `b111` at `include/LDC1614/CommandTable.h:216`; local extracted docs record datasheet conflict in `docs/extracted-md/08_variant_differences_and_open_questions.md:30`.
- `ChannelConfig::idrive` references "datasheet Table 11" at `include/LDC1614/Config.h:98`, but the datasheet register table places DRIVE_CURRENT later.
- `resetAndReapply()` writes reset and reapplies config immediately at `src/LDC1614.cpp:482`; no explicit post-reset delay is visible.

Impact:
- Future maintainers may "fix" the deglitch code to the conflicting table and break reset-default behavior.
- Users looking up the wrong drive-current table may configure IDRIVE incorrectly.
- Reset timing assumptions may be device- or transport-sensitive without evidence.

Recommended remediation:
- Add a code comment and docs note that 33 MHz follows MUX_CONFIG field text/default, and cite the datasheet conflict.
- Correct Doxygen table reference.
- Validate or document reset timing; add a delay hook only if datasheet/evidence requires it.

Suggested tests:
- Static test or unit assertion for deglitch enum values.
- Documentation review for Doxygen references.
- HIL reset/reapply timing test.

### M6. INTB handling in the IDF example conflicts with datasheet wording

Severity: Medium

Evidence:
- Datasheet/local docs say INTB is push-pull and does not require pull-up in `docs/extracted-md/02_pinout_and_signals.md:21`.
- Arduino board config uses plain input for INTB at `examples/common/BoardConfig.h:56`.
- ESP-IDF example enables a pull-up for INTB at `examples/esp_idf/basic/main/main.cpp:129`.

Impact:
- Internal pull-up is not usually destructive, but it documents the wrong electrical assumption and can mask hardware wiring mistakes.
- Users may treat INTB as open-drain.

Recommended remediation:
- Configure IDF INTB as plain input by default.
- Document INTB as push-pull active-low and no external/internal pull-up required.
- Add a comment if board-level reasons require a pull-up.

Suggested tests:
- HIL observe INTB high/low behavior on DRDY and status events.
- Static example contract checking INTB pull-up policy if desired.

### M7. Example labels and production guidance are too informal

Severity: Medium

Evidence:
- Arduino example file says bringup CLI at `examples/01_basic_bringup_cli/main.cpp:2`.
- README lists the Arduino interactive CLI at `README.md:244` but does not strongly label it as diagnostic-only.
- IDF banner says "Bringup Example" in `examples/esp_idf/basic/main/main.cpp:210`.
- `README.md:289` states single-threaded use but does not give an ESP-IDF production integration template with locking.

Impact:
- Users may copy diagnostic CLI loops into production code.
- Long CLI commands can block health/tick observation and bus access budgeting.

Recommended remediation:
- Label every example as diagnostic bring-up, safe smoke test, or production integration template.
- Add a compact production integration section for application-owned I2C locking, timeouts, INTB task model, and recovery policy.
- Keep raw `reg/wreg` clearly documented as diagnostics that can desynchronize cache.

Suggested tests:
- Documentation checklist that each example has a label and intended use.
- Static check for raw diagnostics warnings in README/docs.

## Low-Severity Findings

### L1. Generated version metadata is not reproducible

Severity: Low

Evidence:
- Generated `include/LDC1614/Version.h` uses `__DATE__` and `__TIME__`.
- `.gitignore:41` ignores `include/LDC1614/Version.h`, but generated files still affect local builds.

Impact:
- Builds can vary by timestamp.
- This is a packaging/reproducibility issue, not a driver behavior issue.

Recommended remediation:
- Allow reproducible build mode or generate stable metadata from `library.json`.

Suggested tests:
- Version generation check already exists; add reproducibility mode if needed.

### L2. `probe()` documentation can be misread before configuration exists

Severity: Low

Evidence:
- `probe()` is documented as safe before or after `begin()` at `include/LDC1614/LDC1614.h:118`.
- A fresh instance has no stored transport callbacks, so raw reads return `INVALID_CONFIG` at `src/LDC1614.cpp:233`.

Impact:
- Users may expect `probe()` to work on a fresh default object without a configured transport.

Recommended remediation:
- Clarify that `probe()` uses the currently stored config and requires callbacks to have been provided by a prior `begin()` attempt or future config path.

Suggested tests:
- Existing behavior is likely acceptable; add a documentation-only check.

### L3. `idf_component.yml` lacks explicit target metadata

Severity: Low

Evidence:
- `idf_component.yml:1` contains version/license/url and dependency constraints.
- No `targets:` key was found.

Impact:
- Registry/component consumers do not see explicit ESP32-S2/ESP32-S3 support metadata.

Recommended remediation:
- Add supported targets after pure IDF builds are validated.

Suggested tests:
- CI check for component metadata.

## Device-Specific Correctness Checklist

| Item | Status | Evidence / notes |
| --- | --- | --- |
| LDC1612 2-channel support | PASS | `channelCount` restricted to 2 or 4 at `src/LDC1614.cpp:130`; LDC1612 RR sequence restricted at `src/LDC1614.cpp:60`. |
| LDC1614 4-channel support | PASS | Default `channelCount` is 4 in `include/LDC1614/Config.h:102`; channel 0..3 register helpers exist. |
| Address 0x2A/0x2B only | PASS | Config default/validation in `include/LDC1614/Config.h:116` and `src/LDC1614.cpp:127`; datasheet source `docs/extracted-md/02_pinout_and_signals.md:20`. |
| ADDR pin not-floating warning | PARTIAL | Datasheet docs include warning; README does not make it prominent for hardware integration. |
| SD pin behavior and not-floating warning | PARTIAL | Datasheet docs include warning in `docs/extracted-md/02_pinout_and_signals.md:22`; examples only optionally handle shutdown/hard reset. |
| CLKIN internal-clock tie-to-GND documentation | PARTIAL | Local docs state it in `docs/extracted-md/02_pinout_and_signals.md:19`; README should surface it in hardware notes. |
| DEVICE_ID / MANUFACTURER_ID checks | PASS | `probe()` reads both at `src/LDC1614.cpp:233`; IDs defined in `include/LDC1614/CommandTable.h:54`. |
| DATAx_MSB/DATAx_LSB extraction | PASS | `readChannel()` reads MSB then LSB and assembles 28-bit data at `src/LDC1614.cpp:288`. |
| DATAx error-bit masking and reporting | PASS | Error bits parsed and masked at `src/LDC1614.cpp:302`; channel 0 test exists at `test/test_basic.cpp:742`. |
| RCOUNTx configuration | PASS | Config validation and writes in `src/LDC1614.cpp:165`, `src/LDC1614.cpp:936`, and `_applyConfig()`. |
| SETTLECOUNTx configuration | PASS | Setters and `_applyConfig()` write values; timing helper handles 0/1 minimum. |
| CLOCK_DIVIDERSx configuration | PASS | `finDivider`/`frefDivider` validation and packing in `src/LDC1614.cpp:975`. |
| DRIVE_CURRENTx configuration | PASS | `idrive` validation and packing in `src/LDC1614.cpp:1020`; Doxygen table reference needs correction. |
| OFFSETx behavior | PASS | Offset setter and frequency calculation support exist in `src/LDC1614.cpp:1002` and timing/frequency helpers. |
| MUX_CONFIG / channel sequencing | PASS | Single/autoscan setters exist at `src/LDC1614.cpp:698` and `src/LDC1614.cpp:733`; partial-write risk remains. |
| ERROR_CONFIG / INTB behavior | PARTIAL | Register masks and parsing exist; real INTB behavior not hardware-validated. |
| RESET_DEV / soft reset behavior | PARTIAL | `softReset()` and `resetAndReapply()` exist; direct reset timing/hardware evidence missing. |
| Conversion-ready / unread-conversion behavior | PARTIAL | STATUS parses `UNREADCONVx`; read helpers do not use it to avoid stale autoscan channels. |
| Internal vs external reference clock | PARTIAL | CONFIG bit support exists; hardware clock source validation and README warnings incomplete. |
| Conversion timing formula or approximation | PARTIAL | Helpers exist; datasheet has formula ambiguity and scan-cycle/channel-switch overhead is not fully contracted. |
| Coil-design / application-calibration disclaimers | PARTIAL | Application notes exist; README and metadata still overclaim generic production readiness. |

## API Latency / Blocking Model

Assume `R` is one 16-bit register read via injected `i2cWriteRead`, `W` is one 16-bit register write via injected `i2cWrite`, `N` is configured channel count, and `T` is `Config::i2cTimeoutMs`. Bounds depend on the transport honoring `T`.

| API | I2C transactions | Other waits | Worst-case bound | Notes |
| --- | ---: | --- | --- | --- |
| `begin()` | `2R + (5N + 3)W` | none in core | Transport-bound | N=2 => 15 transactions; N=4 => 25 transactions. |
| `probe()` | `2R` | none | Transport-bound | Raw diagnostic, no health tracking. |
| `recover()` | min `2R`, default can add `2R + busReset + hardReset + (5N + 3)W` | callback-dependent | Unknown if callbacks are unbounded | Manual recovery ladder. |
| `end()` | `0` or `1W` | none | Transport-bound if initialized and awake | Best-effort sleep; errors ignored. |
| `readChannel(ch)` | `2R` | none | Transport-bound | MSB then LSB; no DRDY wait. |
| `readAllChannels(out,count)` | `2 * count R` | none | Transport-bound | Default reads all configured channels. |
| `readDataReady(ready)` | 0 or `1R` | optional GPIO read | GPIO/transport-bound | INTB high path can avoid I2C; INTB low/status path reads STATUS. |
| `dataReady()` | 0 or `1R` | optional GPIO read | GPIO/transport-bound | Hides errors as `false`. |
| `readDeviceStatus()` | `1R` | none | Transport-bound | STATUS read can clear sticky flags/deassert INTB. |
| `readStatusRaw()` | `1R` | none | Transport-bound | Same STATUS side effects. |
| `sleep()` | `0` or `1W` | none | Transport-bound | No write if already sleeping. |
| `wake()` | `0` or `1W` | none | Transport-bound | No write if already awake. |
| `softReset()` | `1W` | no explicit reset wait | Transport-bound plus device reset timing unknown | Transitions driver to UNINIT. |
| `resetAndReapply()` | `1W + (5N + 3)W` | no explicit reset wait | Transport-bound plus reset timing unknown | Reapplies cached config. |
| `readChannelBlocking()` | up to `(timeoutMs + 2)` ready polls plus `2R` | `cooperativeYield` callback | Not a strict wall-clock bound unless `nowMs` advances | With null `nowMs`, finite poll cap dominates. |
| `readAllChannelsBlocking()` | same ready polling plus `2 * count R` | `cooperativeYield` callback | Not a strict wall-clock bound unless `nowMs` advances | Count validation occurs after wait. |
| Runtime single-register setters | usually `1W` | none | Transport-bound | Cache commits on write success. |
| `setSingleChannelMode()` | `2W` | none | Transport-bound, partial-state risk | Writes MUX then CONFIG. |
| `readRegister16()` | `1R` | none | Transport-bound | Diagnostic raw register access. |
| `writeRegister16()` | `1W` | none | Transport-bound | Diagnostic raw register access. |
| Cache/settings/timing helpers | 0 | none | CPU-only | `tick()` is currently no-op. |

## Partial-State / Cache Consistency Assessment

Single-register setters usually avoid committing the software cache until the register write succeeds. That is a good pattern.

The gap is multi-register state. `_applyConfig()` writes many registers and stops at the first failure. `setSingleChannelMode()` writes `MUX_CONFIG` before `CONFIG`. `resetAndReapply()` writes reset and then reapplies the cached config. In all of these cases, hardware can be partially updated while the cache remains old or while the user lacks a precise sync-needed signal.

The driver should expose a deterministic partial-state contract. Acceptable approaches include a `hardwareConfigDirty` bit, a `syncNeeded()` query, failed-register details in `Status::detail`, a `syncConfig()` method, or a policy that forces reset/reapply after any multi-register failure.

## ESP-IDF Port Assessment

- Is there a pure ESP-IDF component? Partially. Root `CMakeLists.txt` registers the core, and `idf_component.yml` exists, but pure `idf.py` build was not available locally and is not in CI.
- Is there a pure ESP-IDF example? Partially. It uses native IDF APIs and `app_main()`, but compiles shared CLI code using `std::string`.
- Does it use native IDF APIs, not Arduino? Yes at the IDF boundary. Searches found no `Arduino.h`, `Wire.h`, `Serial`, `String`, `TwoWire`, or legacy `driver/i2c.h` under `examples/esp_idf`.
- Does the core stay IDF-free? Yes. `include/` and `src/` searches found no ESP-IDF/FreeRTOS tokens.
- Does the example own the bus only as diagnostic code? The example owns the bus, but docs should more clearly label it as diagnostic bring-up rather than production shared-bus guidance.
- Does it map IDF errors precisely? Partially. Timeout maps clearly, but NACK/address/data distinctions are not preserved.
- Does it lock I2C access if multitask? No. The example is single-task and has no mutex/semaphore.
- Does CI build it? No pure `idf.py` build was found in CI. PlatformIO Arduino builds pass; this is not equivalent.

## Tests and Build Coverage

Run locally in this pass:

- `git status --short` -> initially clean before branch creation; later `M AGENTS.md`; after report creation expected `M AGENTS.md` and report file.
- `git branch --show-current` -> `audit/ldc1614-industry-readiness-exploration`.
- `python --version` -> `Python 3.12.10`.
- `python -m platformio --version` -> `PlatformIO Core, version 6.1.18`.
- `python tools/check_core_timing_guard.py` -> `Core timing guard PASSED`.
- `python tools/check_cli_contract.py` -> `CLI contract PASSED`.
- `python tools/check_idf_example_contract.py` -> `IDF example contract PASSED`.
- `python scripts/generate_version.py check` -> `Up to date: C:\Users\Honza\Documents\Projects\LDC1614\include\LDC1614\Version.h`.
- `python -m platformio test -e native` -> passed; `70 test cases: 70 succeeded`.
- `python -m platformio run -e esp32s3dev` -> passed; `esp32s3dev SUCCESS`.
- `python -m platformio run -e esp32s2dev` -> passed; `esp32s2dev SUCCESS`.
- `python -m platformio pkg pack` -> passed; produced `LDC1614-1.0.0.tar.gz`, then the generated tarball was removed to keep the audit diff scoped.
- `idf.py --version` -> failed; `idf.py` is not recognized as a cmdlet/function/script/program in this shell.

Present but not run:

- `idf.py -C examples/esp_idf/basic set-target esp32s3 build` - not run because `idf.py` is unavailable.
- `idf.py -C examples/esp_idf/basic set-target esp32s2 build` - not run because `idf.py` is unavailable.

Missing:

- Pure ESP-IDF CI build.
- CI invocation of `tools/check_idf_example_contract.py`.
- Coverage tooling and thresholds.
- HIL runner and hardware evidence storage.
- Nth-call fault injection in fake bus.

Should be added:

- Native tests for partial writes, NACK variants, begin identity mismatch, variant raw register access, and DATAx MSB-before-LSB transaction order.
- Pure IDF build job for S2/S3.
- HIL safe smoke runner and optional fault profiles.
- Documentation checks for readiness claims and example labels.

## HIL / Hardware Validation Matrix

| Test | Safe default? | Requires hardware/operator? | Current evidence | Needed evidence |
| --- | --- | --- | --- | --- |
| Probe/device ID at configured address | Yes | Hardware required | None found | Log of `MANUFACTURER_ID=0x5449`, `DEVICE_ID=0x3055`. |
| Address 0x2A | Yes | Hardware wired ADDR low | None found | Log proving response at 0x2A and no unintended alternate address. |
| Address 0x2B if available | Yes | Hardware wired or switchable ADDR high | None found | Log proving response at 0x2B. |
| LDC1612 channel bounds if applicable | Yes | LDC1612 hardware | None found | Channels 0/1 usable, channels 2/3 rejected/no access. |
| LDC1614 channels 0..3 | Yes | LDC1614 hardware with sensors | None found | Raw reads and status for all four channels. |
| Safe raw read per enabled channel | Yes | Hardware required | None found | DATAx MSB/LSB reads with error flags recorded. |
| Config readback | Yes | Hardware required | None found | Readback for CONFIG, MUX_CONFIG, ERROR_CONFIG. |
| RCOUNT/SETTLECOUNT/CLOCK_DIVIDERS readback | Yes | Hardware required | None found | Per-channel register readback after begin. |
| INTB behavior if wired | Yes if passive observation | INTB wired and operator confirms | None found | DRDY/error INTB assert/deassert logs; STATUS/DATA read clearing behavior. |
| SD shutdown/wake if wired | Yes with safe timing | SD wired and operator confirms | None found | I2C unavailable in shutdown; wake returns to expected state after reinit. |
| Induced address NACK | Yes if no destructive action | Operator may change address/target | None found | NACK status and recovery log. |
| Unplug/replug | No default | Operator required | None found | Offline/degraded transitions and recovery log. |
| Stuck bus if fixture supports it | No default | Fixture/operator required | None found | Bus-reset behavior and bounded failure log. |
| Bounded soak | Yes if hardware stable | Hardware required | None found | Duration, sample count, error counters, min/max latency. |
| Drive-current tuning note | Documentation only by default | Oscilloscope/calibration for real tuning | Application notes only | Procedure and evidence that IDRIVE is safe for the actual coil/sensor. |

## Recommended Implementation Plan

### P0 - Must fix before production-readiness claim

- Replace unsupported production-grade claims in README, `library.json`, and `idf_component.yml` with honest readiness wording.
- Add partial multi-register write contract: dirty/sync state, failed-register detail, and recovery/sync path.
- Tighten blocking API latency documentation and require/document `nowMs` for wall-clock blocking helpers.
- Add fake-bus nth-call/register-specific failure injection and tests for partial config writes.
- Remove `std::string` dependency from the ESP-IDF example path or clearly isolate it as temporary diagnostic code.
- Add pure ESP-IDF build validation for `esp32s3` and `esp32s2`.
- Add HIL validation matrix and safe smoke runner/procedure before any field-ready claim.

### P1 - Should fix before release/merge

- Delete or document `LDC1614` copy/move semantics.
- Add Doxygen thread-safety and ISR-safety contracts.
- Make raw register helpers variant-aware or explicitly unsafe diagnostic-only.
- Prefer `readDataReady()` over `dataReady()` in examples.
- Fix IDF INTB pull-up configuration or document why a board requires it.
- Improve IDF error mapping and avoid abort-style setup errors where Status-based examples are better.
- Wire `tools/check_idf_example_contract.py` into CI.
- Add tests for LDC1612 channel 2/3 rejection across reads, setters, timing helpers, and raw safe mode if implemented.

### P2 - Useful hardening / later

- Add coverage reporting for native tests.
- Add a reproducible version-generation mode that avoids timestamp variance.
- Add documentation checks for example labels and raw diagnostic warnings.
- Add a production ESP-IDF integration template with explicit bus locking and task model.
- Add reset timing validation and a post-reset delay hook only if evidence requires it.
- Expand application notes into a concise hardware integration checklist: ADDR, SD, CLKIN, INTB, SCL/SDA pull-ups, coil frequency, IDRIVE, and calibration.

## Suggested Chunked Prompt Sequence

1. Core contracts and datasheet correctness.
   - Delete/document copy/move.
   - Add thread/ISR Doxygen.
   - Add partial-state contract.
   - Clarify raw register safe/diagnostic behavior.
   - Fix Doxygen table references and datasheet ambiguity notes.

2. Timing/conversion model and status/error precision.
   - Tighten blocking helper contracts.
   - Validate before waits.
   - Add transaction-count docs.
   - Add `UNREADCONVx`-aware read guidance or helper.
   - Clarify `dataReady()` behavior.

3. ESP-IDF component/example/guard hardening.
   - Remove `std::string` from IDF example path.
   - Add native IDF CLI/parser or `esp_console`/argtable.
   - Add `sdkconfig.defaults` if needed.
   - Add CI `idf.py` builds.
   - Improve IDF error mapping and example labeling.

4. Native tests/fake transport/fault injection.
   - Add nth-call and per-register fake failures.
   - Add partial-write tests.
   - Add NACK/data NACK/bus tests.
   - Add begin identity mismatch tests.
   - Add LDC1612 variant-bound tests.
   - Add coverage reporting.

5. Docs/HIL/final release readiness.
   - Replace unsupported production claims.
   - Add hardware validation matrix and HIL runner/procedure.
   - Add example labels and production integration checklist.
   - Run HIL only if real hardware is available.
   - Produce final readiness report separating software readiness from hardware validation.

Do not implement these in this audit pass.

## Commands Run

- `git status --short` -> clean before branch creation.
- `git branch --show-current` -> started from `hardening/ldc1614-industry-readiness`.
- `git checkout -b audit/ldc1614-industry-readiness-exploration` -> `Switched to a new branch 'audit/ldc1614-industry-readiness-exploration'`.
- `git status --short` -> `M AGENTS.md` after the required guardrail update.
- `git branch --show-current` -> `audit/ldc1614-industry-readiness-exploration`.
- `rg --files` -> succeeded and listed repository files.
- PowerShell datasheet search using `Get-ChildItem -Path .,.. -Recurse -Depth 5 ...` -> succeeded but included nearby projects; relevant local LDC1614 files were found under `docs/`, `_txt/`, and `docs/extracted-md/`.
- `rg -n "Arduino\.h|Wire\.h|TwoWire|String|Serial|delay\(|millis\(|micros\(|vTaskDelay|FreeRTOS|driver/i2c|esp_|ESP_LOG|printf|std::string|new |malloc|std::vector" include src` -> only Doxygen text around "new conversion data" matched; no forbidden core dependency usage.
- `rg -n "DATA|RCOUNT|SETTLE|CLOCK|DRIVE|OFFSET|ERROR_CONFIG|MUX_CONFIG|CONFIG|RESET|DEVICE_ID|MANUFACTURER|STATUS|ADDR|INTB|SD|CLKIN|push-pull|open-drain|28-bit|1 kHz|10 MHz|2.7|3.6" include src README.md docs AGENTS.md` -> succeeded.
- `rg -n "std::string|String|Serial|Arduino|Wire|driver/i2c_master|i2c_master|gpio_|vTaskDelay|esp_|ESP_LOG|console|argtable|while|for \(;;\)|readline" examples\esp_idf examples\common CMakeLists.txt idf_component.yml .github\workflows` -> succeeded.
- `python --version` -> `Python 3.12.10`.
- `python -m platformio --version` -> `PlatformIO Core, version 6.1.18`.
- `python tools/check_core_timing_guard.py` -> `Core timing guard PASSED`.
- `python tools/check_cli_contract.py` -> `CLI contract PASSED`.
- `python tools/check_idf_example_contract.py` -> `IDF example contract PASSED`.
- `python scripts/generate_version.py check` -> `Up to date: C:\Users\Honza\Documents\Projects\LDC1614\include\LDC1614\Version.h`.
- `python -m platformio test -e native` -> passed; `70 test cases: 70 succeeded in 00:00:01.379`.
- `python -m platformio run -e esp32s3dev` -> passed; `esp32s3dev SUCCESS 00:00:06.804`.
- `python -m platformio run -e esp32s2dev` -> passed; `esp32s2dev SUCCESS 00:00:06.192`.
- `python -m platformio pkg pack` -> passed; wrote `LDC1614-1.0.0.tar.gz`; generated tarball was removed.
- `idf.py --version` -> failed; `idf.py` is not recognized as the name of a cmdlet, function, script file, or operable program.
- `idf.py -C examples/esp_idf/basic set-target esp32s3 build` -> not run because `idf.py` is unavailable.
- `idf.py -C examples/esp_idf/basic set-target esp32s2 build` -> not run because `idf.py` is unavailable.
- `git diff --check` -> passed; Git emitted only an `AGENTS.md` CRLF conversion warning.
- `git status --short` -> `M AGENTS.md` and `?? docs/LDC1614_INDUSTRY_READINESS_EXPLORATION_REPORT.md`.

Subagents:

- Repository discovery subagent completed read-only.
- Core architecture subagent completed read-only.
- Datasheet correctness subagent completed read-only.
- Timing and determinism subagent completed read-only.
- ESP-IDF port subagent completed read-only.
- Tests, CI, and tooling subagent completed read-only.
- Docs/examples/HIL subagent attempt failed due session usage limit; docs/HIL audit was completed locally using repository evidence.
- Final reviewer subagent was not spawned because the session hit the usage limit; final review was completed locally.

## Files Changed

- `AGENTS.md`
- `docs/LDC1614_INDUSTRY_READINESS_EXPLORATION_REPORT.md`

No production source, example, test, CI, metadata, or public API files were changed.

## Final Verdict

The repository is ready for implementation hardening. It is not blocked by architecture. It is blocked from production/industry-grade claims by specific software gaps and, more importantly, by missing hardware and fault-validation evidence.

The next work should proceed with chunked implementation prompts. The first implementation chunk should focus on core contracts, partial-state handling, timing contracts, and datasheet correctness before changing example or documentation claims.
