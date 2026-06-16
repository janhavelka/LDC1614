# LDC1614 Industry-Readiness Implementation Plan

> Historical note: this document was created before the final hardening and
> Prompt 02 reconciliation passes. The current authoritative readiness status is
> in `docs/LDC1614_INDUSTRY_HARDENING_FINAL_REPORT.md`, with final cleanup
> evidence in `docs/LDC1614_MERGE_RELEASE_CLEANUP_REPORT.md`.

This document is the baseline reconciliation for the LDC1614/LDC1612
industry-readiness hardening sequence. It records what was inspected in Prompt
01, what was confirmed against the current repository, and how later prompts
should proceed without broad, unnecessary rewrites.

## 1. Baseline Branch and Commit

- Hardening branch: `hardening/ldc1614-industry-readiness`
- Branch created from: `main`
- Starting commit: `f93afaf feat: enhance CLI with health diagnostics and I2C probing commands`
- Working tree at prompt start: clean
- Remote: `origin https://github.com/janhavelka/LDC1614.git`

The literal Unix file-listing command from the prompt could not run in this
Windows PowerShell shell because `bash` is not installed. A PowerShell
equivalent was run and listed the first 300 files within depth 3.

## 2. Files Inspected

Primary files and directories inspected:

- `AGENTS.md`
- `README.md`
- `CHANGELOG.md`
- `library.json`
- `idf_component.yml`
- `CMakeLists.txt`
- `.github/workflows/ci.yml`
- `platformio.ini`
- `include/LDC1614/CommandTable.h`
- `include/LDC1614/Config.h`
- `include/LDC1614/LDC1614.h`
- `include/LDC1614/Status.h`
- `src/LDC1614.cpp`
- `test/test_basic.cpp`
- `test/stubs/Arduino.h`
- `test/stubs/Wire.h`
- `tools/check_core_timing_guard.py`
- `tools/check_cli_contract.py`
- `tools/check_idf_example_contract.py`
- `scripts/generate_version.py`
- `examples/01_basic_bringup_cli/main.cpp`
- `examples/common/Ldc1614Cli.h`
- `examples/common/Ldc1614Cli.cpp`
- `examples/common/I2cTransport.h`
- `examples/common/I2cScanner.h`
- `examples/esp_idf/basic/main/main.cpp`
- `examples/esp_idf/basic/main/CMakeLists.txt`
- `examples/esp_idf/basic/main/Ldc1614IdfI2cTransport.h`
- `examples/esp_idf/basic/main/Ldc1614IdfI2cTransport.cpp`
- `docs/IDF_PORT.md`
- `docs/IDF_PORT_IMPLEMENTATION.md`
- `docs/extracted-md/*.md`
- `docs/pdf-extracted-md/LDC1614_datasheet.md`
- `prompts/00_overview_and_sequence.md`
- `prompts/01_baseline_agents_branch_report.md`
- `prompts/02_core_contracts_device_correctness.md`
- `prompts/03_espidf_example_component_guards.md`
- `prompts/04_tests_docs_ci_package.md`
- `prompts/05_hil_runner_final_report.md`

No prior `docs/*INDUSTRY*` implementation plan or final hardware validation
report was found before this file was created.

## 3. Existing Audit Findings Confirmed

- The core driver is already framework-neutral. Searches found no Arduino,
  Wire, ESP-IDF, FreeRTOS, framework timing calls, `std::string`,
  `std::vector`, heap allocation APIs, or exceptions in `include/` or `src/`.
  The core includes are standard/library headers only in `src/LDC1614.cpp`,
  `include/LDC1614/Config.h`, and `include/LDC1614/LDC1614.h`.
- Transport ownership is injected and non-owning. `Config` stores transport
  function pointers and user contexts, and raw I2C wrappers call those callbacks
  rather than owning a bus.
- The `Status` model is present and broadly consistent. Fallible public driver
  APIs return `Status`, with static-message `Status` values in
  `include/LDC1614/Status.h`.
- LDC1612 versus LDC1614 channel count is mostly explicit. `begin()` accepts
  only `channelCount` 2 or 4, rejects invalid active channels, and rejects
  LDC1612-incompatible round-robin sequences.
- DATA coherency is implemented in core. `readChannel()` reads `DATAx_MSB`
  before `DATAx_LSB` before assembling the 28-bit result.
- Timing helper formulas are implemented and tested for the repository's chosen
  formulas: conversion time uses `(RCOUNT * 16 + 4) / fREFx`, and settling time
  uses the 32-cycle minimum for `SETTLECOUNT` 0 or 1.
- The IDF example still compiles the shared CLI implementation:
  `examples/esp_idf/basic/main/CMakeLists.txt` includes
  `../../../common/Ldc1614Cli.cpp`.
- The shared CLI uses heap-capable `std::string` parsing in
  `examples/common/Ldc1614Cli.cpp`. Therefore the IDF example's input buffer is
  fixed C storage, but command parsing is not a fully fixed-buffer native IDF
  CLI. This is a temporary Prompt 03 gap, not an approved long-term diagnostic
  exception.
- `idf_component.yml` lacks explicit ESP32 target metadata. It currently
  declares version, description, license, URL, and `idf >=6.0.0` only.
- CI does not run a pure `idf.py` build. Current CI runs PlatformIO Arduino
  S2/S3 builds, native tests, the core timing guard, the CLI contract guard, and
  `pio pkg pack`.
- Hardware validation evidence is missing. No hardware validation matrix, HIL
  log, soak-test log, INTB validation log, ADDR 0x2A/0x2B validation evidence,
  real channel-sequencing evidence, fault-injection evidence, or LDC1612
  validation log was found.
- Documentation and metadata currently overstate readiness. `README.md` and
  `library.json` use "Production-grade" language without accompanying hardware
  validation evidence.

## 4. Existing Audit Findings Corrected or Rejected

- A broad core rewrite is not justified. The core architecture is already
  callback-transport based, framework-neutral, fixed-storage, Status-driven, and
  guarded by tests.
- The IDF example does not compile Arduino facades. The IDF main uses
  `app_main`, `driver/i2c_master.h`, GPIO, ESP timer, and FreeRTOS directly,
  and the IDF CMake target does not compile Arduino sketch sources.
- The shared CLI issue is not an Arduino-facade issue. The current confirmed
  issue is shared `std::string` command parsing and shared CLI architecture in
  the IDF diagnostic example.
- A separate LDC1612 versus LDC1614 device ID should not be assumed. The
  extracted notes state the available ID value is shared, and the command table
  uses shared `DEVICE_ID_VALUE = 0x3055`.
- The raw PDF extraction has channel 2/3 table artifacts, but the curated
  extracted notes and command table correct the LDC1614 data register addresses
  to `DATA2` at `0x04/0x05` and `DATA3` at `0x06/0x07`.
- "No meaningful native tests" is rejected. `test/test_basic.cpp` currently
  runs 70 Unity tests using a deterministic fake bus.

## 5. New Findings Discovered

- Public copy/move behavior is not explicitly controlled. `LDC1614` has mutable
  state and non-owning transport pointers but does not delete copy/move
  operations.
- Public Doxygen does not explicitly state that driver instances are not
  internally thread-safe and public APIs are not ISR-safe.
- Partial hardware state is possible for multi-register updates. For example,
  `setSingleChannelMode()` writes `MUX_CONFIG` then `CONFIG`; if the second
  write fails, hardware may be partially changed while cached config remains
  old. `_applyConfig()` can similarly leave partially programmed hardware on an
  early failure during `begin()` or recovery.
- `probe()` collapses transport failures into `DEVICE_NOT_FOUND`, preserving
  detail but losing original error category such as timeout versus bus error.
- `readAllChannelsBlocking()` validates some input only after waiting for data
  readiness through `readAllChannels()`, so invalid `count` can spend time in a
  polling loop before failing.
- Public raw register access is not variant-aware. `readRegister16()` and
  `writeRegister16()` can access LDC1614-only channel 2/3 registers even when
  the driver is configured for `channelCount = 2`.
- Tests verify 28-bit assembly but do not record I2C register read order, so
  DATAx MSB-before-LSB coherency is not directly regression-protected.
- `readDataReady()` can return OK with `ready = true` when STATUS includes both
  DRDY and error bits. That may hide status-routed sensor errors during
  data-ready events.
- No helper currently uses `STATUS.UNREADCONVx` to drain only fresh channels or
  detect overwrite pressure in multi-channel mode.
- Sensor-frequency and timing validation cannot enforce all physical
  constraints because `Config` does not know actual sensor frequency, Q, coil
  parameters, or application calibration limits.
- INTB is push-pull per extracted notes. The Arduino example uses plain input,
  but the current IDF example enables an internal GPIO pull-up for INTB.
- SD pin handling is application/example-level only. Core has an optional hard
  reset callback and examples can toggle a shutdown pin, but the core cannot
  validate a floating or asserted SD pin except through failed I2C identity
  reads.
- The IDF transport rejects CLI raw identity checks for any address other than
  the configured device handle address. This limits `probeaddr 0x2B` and the
  `bus` command under IDF until the transport can temporarily address both
  possible LDC addresses.
- `Version.h` is generated and ignored. PlatformIO generates it, but a plain
  clean CMake/ESP-IDF component build path does not visibly generate it.

## 6. Implementation Chunk Plan Matching This Prompt Sequence

Prompt 01 - Baseline and audit reconciliation:

- Create the hardening branch.
- Update `AGENTS.md` with explicit framework-boundary, transport ownership,
  thread/ISR, partial-write, variant, coherency, IDF-example, and validation
  claim rules.
- Produce this implementation plan.

Prompt 02 - Core contracts and device correctness:

- Add explicit non-copyable/non-movable driver semantics if compatible with the
  public API contract.
- Document thread/ISR safety in public Doxygen.
- Tighten input validation before blocking waits where practical.
- Add variant-aware raw register validation or document the current diagnostic
  behavior explicitly.
- Add regression tests for DATAx MSB-before-LSB read ordering.
- Add tests and/or status for partial multi-register configuration writes.
- Review `readDataReady()` behavior when DRDY and sensor error flags coexist.
- Avoid broad refactors unless a targeted contract fix requires one.

Prompt 03 - Native ESP-IDF example and component metadata:

- Replace or isolate the IDF CLI so the native IDF example no longer depends on
  shared `std::string` parsing.
- Update `tools/check_idf_example_contract.py` to enforce the new IDF contract.
- Add explicit component-manager target metadata if supported and honest.
- Narrow IDF example include paths if practical.
- Fix IDF raw address diagnostics or clearly limit them.
- Attempt pure ESP-IDF build if `idf.py` is available; otherwise record the
  missing tool exactly.

Prompt 04 - Tests, documentation, CI, and package readiness:

- Expand deterministic tests for device-correctness and failure modes.
- Add CI guard coverage for the IDF contract and any new metadata/docs checks.
- Reword README and metadata so claims separate software design readiness from
  hardware validation.
- Add or draft a software readiness checklist and hardware validation checklist.
- Confirm package generation and version generation behavior.

Prompt 05 - HIL diagnostic runner and final report:

- Add or improve a hardware diagnostic runner if consistent with repository
  style.
- Add a hardware validation matrix covering LDC1612/LDC1614, ADDR 0x2A/0x2B,
  INTB, SD, channel sequencing, fault/unplug cases, coil limits, and soak/HIL
  stress.
- Run HIL only if safe real hardware is available. Do not invent hardware
  results.
- Produce a final report separating software readiness from hardware validation
  gaps.

## 7. Known Hardware Validation Blockers

- No captured real LDC1614 or LDC1612 identity logs in the repository.
- No proof that both ADDR pin states, 0x2A and 0x2B, were tested.
- No real INTB assertion/deassertion evidence for data-ready or error events.
- No SD pin shutdown/inactive-mode validation evidence.
- No real multi-channel auto-scan sequencing evidence.
- No real sensor/coils validation against frequency, Rp, Q, drive-current, and
  deglitch constraints.
- No unplug/fault-injection logs for under-range, over-range, watchdog,
  amplitude, or zero-count behavior.
- No long-duration HIL/soak testing logs.
- No hardware evidence for LDC1612 two-channel variant behavior.

## 8. Commands Run and Exact Results

Starting checks:

- `git status --short` -> clean
- `git branch --show-current` -> `main`
- `git remote -v` ->
  `origin https://github.com/janhavelka/LDC1614.git (fetch)` and
  `origin https://github.com/janhavelka/LDC1614.git (push)`
- `git log --oneline -n 20` -> latest commit was
  `f93afaf feat: enhance CLI with health diagnostics and I2C probing commands`
- `bash -lc "find . -maxdepth 3 -type f | sort | sed 's#^./##' | head -300"` ->
  failed: `bash : The term 'bash' is not recognized as the name of a cmdlet,
  function, script file, or operable program.`
- PowerShell equivalent file listing -> succeeded and listed the first 300
  depth-limited files.
- `git branch --list hardening/ldc1614-industry-readiness` -> no existing
  branch listed
- `git checkout -b hardening/ldc1614-industry-readiness` -> `Switched to a new
  branch 'hardening/ldc1614-industry-readiness'`

Read-only audit commands:

- `rg --files docs prompts | Sort-Object` -> succeeded
- `rg -n "audit|industry|readiness|validation|hardware|IDF|idf_component|std::string|Arduino|ESP-IDF|report" ...` -> succeeded
- `rg -n "Arduino\.h|Wire\.h|String|Serial|TwoWire|delay\(|millis\(|yield\(|esp_|freertos|driver/|std::string|std::vector|new |malloc|String" ...` -> succeeded
- `rg -n "DATA|readChannel|readAllChannels|regData|REG_DATA|MSB|LSB|calc|Settle|RCOUNT|CONFIG|writeRegister|set[A-Z]|hardwareConfigDirty|sync" src include test` -> succeeded
- `rg -n "validated|verified|production|industry|hardware|real hardware|tested|INTB|HIL|logs|claim" ...` -> succeeded
- Multiple `Get-Content` inspections of files listed in Section 2 -> succeeded

Subagent read-only audit results:

- `core-contracts-agent` -> completed; no files edited
- `datasheet-device-agent` -> completed; no files edited
- `idf-example-agent` -> completed; no files edited
- `tests-ci-agent` -> completed; no files edited
- `docs-report-agent` -> completed; no files edited
- `integration-review-agent` -> completed; no files edited; requested this
  document record the integration review, final pre-commit checks, and the
  temporary nature of the IDF shared-CLI gap before commit

Verification commands:

- `python --version` -> `Python 3.12.10`
- `python -m platformio --version` -> `PlatformIO Core, version 6.1.18`
- `python tools/check_core_timing_guard.py` -> `Core timing guard PASSED`
- `python tools/check_cli_contract.py` -> `CLI contract PASSED`
- `python tools/check_idf_example_contract.py` -> `IDF example contract PASSED`
- `python scripts/generate_version.py check` ->
  `Up to date: C:\Users\Honza\Documents\Projects\LDC1614\include\LDC1614\Version.h`
- `python -m platformio test -e native` -> passed. Summary:
  `70 test cases: 70 succeeded in 00:00:00.878`
- `python -m platformio run -e esp32s3dev` -> passed. Summary:
  `esp32s3dev SUCCESS 00:00:04.859`
- `python -m platformio run -e esp32s2dev` -> passed. Summary:
  `esp32s2dev SUCCESS 00:00:04.726`
- PlatformIO emitted this warning on PlatformIO commands:
  `Obsolete PIO Core v6.1.18 is used (previous was 6.1.19)`
- `idf.py --version` -> failed because the command is unavailable:
  `idf.py : The term 'idf.py' is not recognized as the name of a cmdlet,
  function, script file, or operable program.`

Final pre-commit checks after integration review:

- `git diff --stat` -> reported `AGENTS.md | 23 +++++++++++++++++++++--`
  because the new implementation-plan file was still untracked and therefore
  not included in unstaged diff output
- `git diff --check` -> passed; PowerShell/Git reported only the line-ending
  warning `LF will be replaced by CRLF the next time Git touches it`
- `git status --short` -> current intended status is `M AGENTS.md` and
  `?? docs/LDC1614_INDUSTRY_READINESS_IMPLEMENTATION_PLAN.md`
