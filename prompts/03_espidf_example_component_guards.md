# Prompt 03 — Native ESP-IDF Example, Component Metadata, and Guards

You are continuing the LDC1614 industry-readiness hardening sequence on the current hardening branch.

Complete only this prompt, commit and push/sync, then stop and report back.

## Goal

Make the ESP-IDF port/example honest, native, and production-credible enough for diagnostics. Fix the known audit issue where the IDF example reportedly pulls shared/common CLI code using `std::string`, and fix component metadata ambiguity.

## Subagents

Spawn:

- `idf-example-agent`
- `cli-contract-agent`
- `tests-ci-agent`
- `docs-report-agent`
- `integration-review-agent`

## Main tasks

### 1. Inspect current IDF example boundaries

Inspect:

```text
examples/esp_idf/basic/
examples/esp_idf/basic/main/CMakeLists.txt
examples/esp_idf/basic/main/*
examples/common/*
idf_component.yml
CMakeLists.txt
platformio.ini
.github/workflows/*
tools/*idf* tools/*cli* tools/*guard*
```

Determine whether the IDF example currently compiles or includes:

- `examples/common/Ldc1614Cli.cpp`,
- shared Arduino-oriented CLI code,
- `std::string`,
- Arduino `String`,
- `Arduino.h`, `Wire.h`, `Serial`, `TwoWire`,
- Arduino-like `millis()` / `delay()` / `yield()` shims,
- global bus mutation hidden in CLI helpers.

### 2. Implement native fixed-buffer ESP-IDF CLI

If the IDF example currently uses shared/common CLI code or heap-heavy parsing, replace it with a native ESP-IDF CLI implementation scoped to the IDF example.

Requirements:

- Use fixed-size C buffers for command input/parsing.
- Use ESP-IDF timing primitives clearly named, e.g. `nowMs()` based on `esp_timer_get_time()`.
- Use FreeRTOS only in the example/adapter, never in core.
- Keep the driver tick/progress path independent from blocking console input where practical. Prefer a periodic task for driver `tick()`/housekeeping if the driver requires it.
- Preserve functional parity with the Arduino diagnostic CLI at workflow level, not necessarily identical code.
- Keep commands safe for hardware: probe, id, status, read channel(s), autoscan config, RCOUNT/SETTLECOUNT inspect, deglitch, drive current inspect/set with warnings, INTB/status, reset/recover, stress, selftest.
- Do not claim hardware validation; just provide diagnostic surface.

If a full CLI rewrite is too large, implement the minimum safe native CLI and explicitly document remaining parity gaps. Do not leave advertised commands as placeholders.

### 3. IDF transport and bus ownership

Ensure the IDF example demonstrates good boundaries:

- transport owns or receives an IDF I2C bus handle explicitly;
- timeout is propagated to `i2c_master_transmit` / `i2c_master_transmit_receive` or equivalent;
- IDF errors are mapped to driver status detail as precisely as practical;
- shared-bus locking is shown with a mutex or documented clearly as application responsibility;
- no core dependency on IDF headers.

If the example creates its own bus, label it as diagnostic bring-up. Add a separate doc section showing how production code should inject a transport backed by an application-owned bus manager.

### 4. Component metadata

Update `idf_component.yml` if appropriate:

- add explicit target list for ESP32-S2 and ESP32-S3 if this repo’s component-manager format supports it;
- verify version/name/dependencies are accurate;
- do not add false targets if untested or unsupported.

Ensure root `CMakeLists.txt` and example `CMakeLists.txt` include only necessary paths. Avoid exposing the whole repository root to the IDF example if narrower includes work.

### 5. Guards

Add/update guard scripts so regressions are caught.

Minimum guards:

- core framework guard: no Arduino/IDF/FreeRTOS tokens in `include/` and `src/`;
- IDF example guard: no Arduino tokens, no `std::string` if fixed-buffer CLI is now the contract, no shared Arduino/common CLI sources in the IDF example build;
- CLI contract guard: help text and implemented handlers match; no advertised placeholder commands.

Suggested command names:

```bash
python tools/check_core_timing_guard.py
python tools/check_idf_example_contract.py
python tools/check_cli_contract.py
```

### 6. Documentation

Update:

- `README.md`,
- `docs/IDF_PORT.md`,
- `docs/IDF_PORT_IMPLEMENTATION.md`,
- progress/final report draft.

Clarify:

- IDF example is diagnostic unless it demonstrates production shared-bus management;
- exact commands supported by IDF CLI;
- what still requires hardware;
- what was changed from shared CLI to native IDF CLI;
- how to run IDF builds locally and in CI.

## Verification

Run:

```bash
python tools/check_core_timing_guard.py || true
python tools/check_idf_example_contract.py || true
python tools/check_cli_contract.py || true
python scripts/generate_version.py check || true
python -m platformio test -e native
python -m platformio run -e esp32s3dev
python -m platformio run -e esp32s2dev
```

Attempt pure IDF if available:

```bash
idf.py --version
idf.py -C examples/esp_idf/basic set-target esp32s3 build
idf.py -C examples/esp_idf/basic set-target esp32s2 build
```

If `idf.py` is unavailable, record exact output.

## Report update

Update `docs/LDC1614_HARDENING_PROGRESS.md` with:

- whether shared CLI/std::string was present,
- exact fix or documented exception,
- component metadata changes,
- guard scripts added/updated,
- checks run and results,
- remaining parity or IDF build gaps.

## Commit and sync

Before commit:

```bash
git diff --stat
git diff --check
git status --short
```

Commit message:

```text
feat: harden LDC1614 ESP-IDF example and component guards
```

Push/sync, then stop.
