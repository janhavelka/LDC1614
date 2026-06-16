# Prompt 01 — Baseline, Branch, AGENTS.md, and Audit Reconciliation

> Historical artifact: this was an execution prompt for a past hardening pass.
> It is preserved for traceability and is not current repository guidance.
> Current instructions live in `AGENTS.md`; current docs are indexed from
> `docs/README.md`.

You are working in the currently opened LDC1614/LDC1612 repository.

This is prompt 1 of a chunked industry-readiness hardening sequence. Complete only this prompt, commit and push/sync, then stop and report back. Later prompts will build on this work.

## Goal

Establish a clean production-readiness branch, update `AGENTS.md`, re-read the existing industry-readiness audit, verify it against the actual repository, and produce a corrected implementation plan before editing core behavior.

## Required starting checks

Run:

```bash
git status --short
git branch --show-current
git remote -v
git log --oneline -n 20
find . -maxdepth 3 -type f | sort | sed 's#^./##' | head -300
```

If the working tree is dirty, inspect the diff. Do not overwrite user work. If the dirty files are unrelated, stop and ask. If they are the existing audit/report files from the current task, continue carefully and document them.

## Branch policy

If not already on an appropriate hardening branch, create:

```bash
git checkout -b hardening/ldc1614-industry-readiness
```

If the branch already exists, continue on it only if it is clearly the intended branch. Do not delete existing branches.

## Spawn subagents

Use subagents and ask them for factual, code-grounded findings with file paths:

1. `core-contracts-agent`
   - framework neutrality, transport ownership, status model, copy/move, thread/ISR, allocation, blocking behavior.
2. `datasheet-device-agent`
   - LDC1612 vs LDC1614 register/device correctness, data coherency, channel sequencing, timing equations, INTB/SD/ADDR pins, status/error bits.
3. `idf-example-agent`
   - ESP-IDF example architecture, `std::string`/heap-heavy parsing, Arduino leakage, component metadata, target list, CMake boundaries.
4. `tests-ci-agent`
   - native tests, fake transport, guard scripts, CI, PlatformIO Arduino S2/S3, ESP-IDF build support.
5. `docs-report-agent`
   - README, docs, validation claims, hardware matrix, example labels.
6. `integration-review-agent`
   - final diff sanity and report accuracy before commit.

## Update AGENTS.md

Create or update `AGENTS.md` with LDC1614-specific rules:

- Core `include/` and `src/` must remain framework-neutral: no Arduino, Wire, ESP-IDF, FreeRTOS, logging framework, global bus objects, framework delays, or heap-heavy framework types.
- Core must use non-owning injected I2C transport. Bus ownership, locking, timeout policy, and recovery belong to the application or transport adapter.
- Fallible public APIs must return precise `Status`/error results; do not silently ignore failed register writes.
- Public APIs are not ISR-safe unless explicitly documented and proven. Instances are not internally thread-safe unless explicitly protected.
- LDC1612 vs LDC1614 variant differences must be explicit; channels 2/3 are LDC1614-only.
- DATAx coherency must respect datasheet ordering: read DATAx_MSB before DATAx_LSB.
- Multi-register configuration updates must avoid, report, or recover from partial hardware state.
- Conversion timing, settling timing, and sensor frequency calculations must be documented and tested.
- IDF examples must not depend on Arduino facades or shared Arduino-style CLI code unless explicitly documented as a deliberate diagnostic exception.
- Do not claim hardware validation unless real hardware logs were captured.

## Reconcile audit findings

Read the existing audit/report files under `docs/` and the current README/IDF docs. Verify the audit against actual code. Pay special attention to whether these reported issues are still true:

- IDF example uses shared/common CLI code or `std::string`.
- `idf_component.yml` lacks explicit ESP32 target metadata.
- local pure ESP-IDF build was not run.
- hardware validation evidence is missing or old.
- core architecture is already clean enough to avoid broad refactor.

Create or update:

```text
docs/LDC1614_INDUSTRY_READINESS_IMPLEMENTATION_PLAN.md
```

Required sections:

1. Baseline branch and commit.
2. Files inspected.
3. Existing audit findings confirmed.
4. Existing audit findings corrected or rejected.
5. New findings discovered.
6. Implementation chunk plan matching this prompt sequence.
7. Known hardware validation blockers.
8. Commands run and exact results.

## Verification for this prompt

Run available non-invasive checks:

```bash
python --version
python -m platformio --version
python -m platformio test -e native
python -m platformio run -e esp32s3dev
python -m platformio run -e esp32s2dev
```

Run any existing repo guard scripts, for example:

```bash
python tools/check_core_timing_guard.py
python tools/check_cli_contract.py
python tools/check_idf_example_contract.py
python scripts/generate_version.py check
```

If any command is missing, record it exactly.

## Commit and sync

Before commit:

```bash
git diff --stat
git diff --check
git status --short
```

Commit message:

```text
docs: baseline LDC1614 industry-readiness plan
```

Push/sync the branch. Then stop and report:

- branch name,
- commit hash,
- confirmed findings,
- commands run,
- failures or blockers,
- whether prompt 02 can proceed.
