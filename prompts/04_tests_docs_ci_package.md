# Prompt 04 — Tests, Documentation, CI, Packaging, and Release Contracts

You are continuing the LDC1614 industry-readiness hardening sequence on the current hardening branch.

Complete only this prompt, commit and push/sync, then stop and report back.

## Goal

Make the repository’s test/build/documentation surface strong enough that software readiness is reproducible and release claims are honest. This prompt should not depend on physical hardware.

## Subagents

Spawn:

- `tests-fault-agent`
- `docs-report-agent`
- `ci-packaging-agent`
- `integration-review-agent`

## Main tasks

### 1. Expand native/fake-transport tests

Audit current test suite and add missing high-value tests.

Minimum coverage expected after this prompt:

- config validation and invalid addresses 0x2A/0x2B only if applicable;
- LDC1612 vs LDC1614 channel count;
- register read/write endianness;
- DATAx MSB-before-LSB ordering;
- DATAx 28-bit assembly and status/error bits;
- under-range and over-range markers;
- manufacturer/device ID success and mismatch;
- I2C NACK/timeout/bus error mapping;
- partial multi-register write failures;
- dirty/sync-needed state and recovery clear behavior if implemented;
- conversion-time and settle-time formula;
- autoscan channel sequence config bits;
- deglitch and drive-current register encoding bounds;
- copy/move deletion;
- public APIs reject use before begin/init where appropriate.

Use a deterministic fake transport. Avoid hardware assumptions.

### 2. Documentation contracts

Update README/Doxygen/docs to include:

- explicit supported variants: LDC1612 and/or LDC1614;
- I2C addresses 0x2A/0x2B and ADDR pin behavior;
- INTB is push-pull and does not require pull-up, unlike many open-drain interrupt pins;
- SD pin hardware shutdown/inactive behavior and must-not-float warning;
- CLKIN tie-to-ground rule when internal oscillator is used;
- VDD 2.7–3.6 V and I2C 400 kbit/s target limit;
- channel availability and LDC1614-only channels 2/3;
- DATAx coherency rule;
- conversion/settling timing formulas;
- sensor-frequency range and coil design caveat;
- drive-current tuning warning: requires real sensor/oscilloscope/application validation;
- status/error model and recovery recipe;
- thread/ISR safety;
- public API transaction/latency table;
- diagnostic vs production example labels;
- hardware validation matrix.

### 3. CI and packaging

Audit and update if safe:

- `.github/workflows/ci.yml`,
- `platformio.ini`,
- `library.json`,
- `CMakeLists.txt`,
- `idf_component.yml`,
- version-generation scripts.

Expected CI coverage where practical:

- native tests;
- Arduino ESP32-S2 build;
- Arduino ESP32-S3 build;
- guard scripts;
- package validation;
- pure ESP-IDF build job if CI container supports ESP-IDF.

Do not claim CI executes a job unless the workflow actually has it.

### 4. Static checks and search hygiene

Run searches and fix/document issues:

```bash
grep -RIn "Arduino.h\|Wire.h\|String\|Serial\|TwoWire\|delay(\|millis(\|yield(" include src || true
grep -RIn "TODO\|FIXME\|placeholder\|not implemented" include src examples docs README.md || true
grep -RIn "industry-grade\|production-ready\|validated\|verified" README.md docs examples || true
```

Docs may contain these words, but they must not overclaim.

### 5. Release-readiness report draft

Update/create:

```text
docs/LDC1614_RELEASE_READINESS_CHECKLIST.md
```

Include:

- software readiness checklist,
- build/test checklist,
- ESP-IDF status,
- Arduino status,
- hardware validation checklist,
- known non-blockers,
- release blockers.

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
python -m platformio pkg pack
```

Attempt pure IDF if available:

```bash
idf.py --version
idf.py -C examples/esp_idf/basic set-target esp32s3 build
idf.py -C examples/esp_idf/basic set-target esp32s2 build
```

Remove generated package artifacts after validation unless the repo normally tracks them.

## Report update

Update `docs/LDC1614_HARDENING_PROGRESS.md` with:

- tests added,
- docs updated,
- CI/package changes,
- exact command results,
- what remains for hardware/HIL and final report.

## Commit and sync

Before commit:

```bash
git diff --stat
git diff --check
git status --short
```

Commit message:

```text
test: expand LDC1614 readiness coverage and docs
```

Push/sync, then stop.
