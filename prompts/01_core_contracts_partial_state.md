# LDC1614 Industry-Readiness Hardening — Tailored Sequential Prompt

You are working in the LDC1614 repository. This prompt is one step in a deliberately chunked sequence derived from `docs/LDC1614_INDUSTRY_READINESS_EXPLORATION_REPORT.md`.

You will receive the prompts one by one. Complete only the current prompt. Do not jump ahead into later chunks unless a small prerequisite is necessary for this prompt to compile and pass tests.

Global rules for every chunk:
- Start by running `git status --short` and `git branch --show-current`.
- Continue on or create `hardening/ldc1614-industry-readiness`.
- If there are unrelated user changes, stop and report them. Do not overwrite user work.
- Spawn subagents where available and have them report factual findings before code changes are finalized.
- Keep the core in `include/` and `src/` framework-neutral: no Arduino, Wire, ESP-IDF, FreeRTOS, logging framework, global bus, hidden heap allocation, or platform timing calls.
- Keep I2C ownership external/injected. The core must not own bus pins, Wire, IDF handles, GPIO setup, interrupts, or task scheduling.
- Public fallible APIs should return `Status` or an existing status-bearing result. Avoid silent failure paths.
- Preserve precise transport errors when possible.
- Do not claim hardware validation unless real hardware commands were run and logs were captured.
- Do not claim pure ESP-IDF validation unless `idf.py` or CI actually built the ESP-IDF example/component.
- Run all available checks listed in the prompt.
- Update `docs/LDC1614_HARDENING_PROGRESS.md`.
- Commit and push/sync after this prompt, then stop and report.

Baseline checks to run at the start:
```bash
git status --short
git branch --show-current
git checkout hardening/ldc1614-industry-readiness || git checkout -b hardening/ldc1614-industry-readiness
git status --short
```

## Prompt 01 — Fix H2/M1/M3/M5: core contracts, partial-state handling, raw diagnostics, and datasheet notes

### Audit findings this prompt must address

From the exploration report:
- **H2**: Multi-register writes can leave partial hardware state without dirty/sync contract.
- **M1**: Raw register diagnostics are not variant-aware or access-type-aware.
- **M3**: Copy/move, thread-safety, and ISR-safety contracts are too weak.
- **M5**: Datasheet ambiguity and documentation nits need explicit treatment.
- Related **L2**: `probe()` documentation can be misread before configuration exists.

### Subagents

Spawn these subagents:
1. `core-contracts-agent`
   - Inspect cached config, health state, multi-register write paths, and public API shape.
2. `datasheet-register-agent`
   - Verify LDC1612/LDC1614 register map, DATAx bits, CONFIG/MUX/RESET behavior, deglitch ambiguity, and access types.
3. `test-agent`
   - Add tests for every changed contract in this prompt.
4. `integration-review-agent`
   - Confirm no framework leakage, no broad refactor, and no false validation claims.

### Tasks

#### 1. Read the report and map exact code paths

Read at minimum:
```bash
sed -n '1,260p' docs/LDC1614_INDUSTRY_READINESS_EXPLORATION_REPORT.md
sed -n '260,620p' docs/LDC1614_INDUSTRY_READINESS_EXPLORATION_REPORT.md
```

Find all relevant methods:
```bash
rg -n "_applyConfig|setSingleChannelMode|resetAndReapply|recover|writeRegister16|readRegister16|probe|class LDC1614|ChannelConfig|Deglitch|idrive|DEVICE_ID|MANUFACTURER_ID" include src test README.md docs
```

#### 2. Implement a partial hardware-state contract

Add a deterministic dirty/sync-needed state for hardware configuration.

Required behavior:
- Add state such as `hardwareConfigDirty` / `syncNeeded`.
- Expose it publicly, for example:
  - `bool hardwareConfigDirty() const`;
  - `Status hardwareConfigDirtyError() const`;
  - and/or fields in an existing snapshot/health structure.
- Store the first error that made the hardware dirty.
- Include failed register or phase in `Status::detail` when practical.
- Set dirty state when:
  - `_applyConfig()` fails after at least one register write may have reached hardware;
  - `setSingleChannelMode()` writes `MUX_CONFIG` successfully but `CONFIG` fails;
  - `resetAndReapply()` soft-reset succeeds but reapply fails;
  - raw diagnostic `writeRegister16()` can desynchronize cache, unless you create an explicitly named unsafe method and document it strongly.
- Clear dirty state only after a full successful sync/recover/reapply.
- Preserve the original transport error as the returned status.

Add a recovery recipe in Doxygen and docs:
1. Detect dirty state.
2. Stop trusting cached configuration.
3. Call `syncConfig()` / `recover()` / `resetAndReapply()`.
4. Verify dirty cleared only after full success.

If there is no existing `syncConfig()`, add the smallest clean one, or make `recover()`/`resetAndReapply()` the documented sync path.

#### 3. Strengthen raw register diagnostics

Fix M1 with the least risky design.

Minimum acceptable:
- Mark `readRegister16()` / `writeRegister16()` as diagnostic-only in Doxygen and README.
- State that raw writes can desynchronize cached config and therefore mark dirty or require sync/recover.
- Ensure LDC1612 mode cannot accidentally use normal high-level APIs to touch LDC1614-only channels.

Better if feasible:
- Add register metadata for:
  - variant validity: LDC1612/LDC1614/both;
  - access type: RO/WO/RW/diagnostic-unsafe;
  - reserved/unsafe.
- Add safe diagnostic validation mode without removing expert escape hatches.

Do not break useful low-level diagnostics without a migration note.

#### 4. Delete or explicitly define copy/move

Given the driver holds mutable cache, health state, callbacks, and user context, delete:
- copy constructor;
- copy assignment;
- move constructor;
- move assignment.

Only keep move/copy if a subagent proves it is intentional and safe; otherwise delete.

Add tests/static assertions for copy/move policy.

#### 5. Thread/ISR/public contract

Add Doxygen in public header and README note:
- instances are not internally thread-safe;
- public APIs are not ISR-safe;
- all I2C access must be externally serialized;
- transport callbacks must not recursively call into the same instance;
- locking belongs in the injected transport/application bus manager.

#### 6. Datasheet notes/nits

Fix M5:
- Add comment/docs for the deglitch 33 MHz datasheet ambiguity and chosen enum/value.
- Correct the `ChannelConfig::idrive` Doxygen table reference.
- Clarify `probe()` docs: it requires configured transport callbacks; it is safe before successful `begin()` only after callbacks have been supplied by a prior configuration/begin path.
- Document reset/reapply timing assumptions. Do not add arbitrary delay unless local datasheet evidence supports it; if a delay hook is needed, make it injected/bounded and tested.

### Tests required

Add native tests for:
- Dirty set when `_applyConfig()` fails after earlier write success.
- Dirty includes useful failed-register/phase detail.
- Dirty clears only after successful complete sync/recover/reapply.
- Dirty does not clear after failed sync/recover.
- `setSingleChannelMode()` partial failure marks dirty.
- `resetAndReapply()` partial failure marks dirty.
- Raw diagnostic write follows chosen dirty/sync policy.
- Copy/move deleted or explicitly policy-tested.
- `probe()` missing callbacks returns expected status and docs match behavior.
- Deglitch enum/value and IDRIVE docs do not regress if testable.

### Checks

Run:
```bash
python tools/check_core_timing_guard.py
python tools/check_cli_contract.py
python tools/check_idf_example_contract.py
python scripts/generate_version.py check
python -m platformio test -e native
python -m platformio run -e esp32s3dev
python -m platformio run -e esp32s2dev
python -m platformio pkg pack
```

### Progress report

Create/update:
```text
docs/LDC1614_HARDENING_PROGRESS.md
```

Append:
```markdown
## Prompt 01 — H2/M1/M3/M5 core contracts

### Findings addressed
### Implemented changes
### Public API changes
### Tests added
### Commands run
### Remaining related work
```

### Commit and sync

```bash
git status --short
git add AGENTS.md include src test docs README.md
git commit -m "Harden LDC1614 core partial-state and API contracts"
git push
```
