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

## Prompt 02 — Fix H3/M2 and freshness gaps: timing, blocking, data-ready, and UNREADCONV semantics

### Audit findings this prompt must address

- **H3**: Blocking API latency contracts are incomplete.
- **M2**: `dataReady()` hides transport/status errors.
- Device checklist partials:
  - conversion-ready / unread-conversion behavior is only partial;
  - conversion timing formula/approximation is only partial;
  - autoscan can read stale channels after one DRDY.

### Subagents

Spawn:
1. `timing-agent` — public blocking APIs, timeout/timebase, transaction counts.
2. `freshness-agent` — STATUS/UNREADCONVx/DATAx/autoscan semantics from datasheet.
3. `test-agent` — tests for validation-before-wait, timebase, and freshness.
4. `docs-agent` — README/Doxygen latency tables and production recipes.

### Tasks

#### 1. Audit current blocking and readiness paths

Run:
```bash
rg -n "readChannelBlocking|readAllChannelsBlocking|readDataReady|dataReady|UNREADCONV|readAllChannels|readDeviceStatus|timeoutMs|nowMs|cooperativeYield|calculate|conversion|RCOUNT|SETTLECOUNT|CLOCK_DIVIDERS" include src test README.md docs
```

#### 2. Validate before waiting or touching I2C

Fix any blocking helper that waits before validating:
- output pointer/reference;
- `count`;
- channel range;
- configured channel count;
- initialized/configured state;
- `nowMs` policy if required.

Specifically ensure `readAllChannelsBlocking()` validates `out`, `count`, and channel count before polling/waiting or touching I2C.

Tests:
- invalid call returns expected error before any fake-bus transaction;
- invalid call does not call yield/wait.

#### 3. Make blocking timebase contract deterministic

Choose the clean production contract:

Preferred:
- Blocking helpers that claim wall-clock timeout require a monotonic `Config::nowMs`.
- If `nowMs` is null, return `INVALID_CONFIG` before starting conversion/readiness polling.
- Keep nonblocking/latest reads available without `nowMs`.

Alternative only if compatibility requires:
- Keep finite poll-count fallback but explicitly document that it is not wall-clock timeout.
- Tests must prove the exact maximum poll/transaction count.

Update Doxygen and README to match actual behavior.

#### 4. Add API latency/transaction table

Add a table in README and public docs covering:
- `begin()`
- `probe()`
- `recover()`
- `readChannel()`
- `readAllChannels()`
- `readDataReady()`
- `dataReady()`
- `readDeviceStatus()`
- `sleep()`
- `wake()`
- `softReset()`
- `resetAndReapply()`
- `readChannelBlocking()`
- `readAllChannelsBlocking()`
- major setters
- raw register access

Use `R`, `W`, `N`, and `T` notation as in the audit report. Note callback latency for `busReset`, `hardReset`, and `cooperativeYield`.

#### 5. Make data-ready semantics honest

Fix M2:
- Keep `readDataReady(bool&)` as the production/status-returning API.
- Document `dataReady()` as convenience only, where `false` can mean not-ready or hidden error.
- Prefer `readDataReady()` in examples and production docs.
- Consider deprecating `dataReady()` if the repo supports deprecation.

Tests:
- `readDataReady()` preserves timeout/NACK/bus statuses.
- `dataReady()` collapses errors to false only if retained and documented.

#### 6. Fresh/latest/autoscan semantics

Address the audit's autoscan stale-channel risk.

Implement the smallest robust solution:

Option A, preferred if not too large:
- Add `readUnreadChannels()` / `readFreshChannels()` / equivalent.
- It reads STATUS, uses `UNREADCONVx`, and reads only channels with unread data.
- It returns per-channel freshness/validity and preserves DATAx error flags.
- Existing `readAllChannels()` remains latest-register semantics.

Option B:
- Keep API unchanged but document:
  - `readAllChannels()` returns latest register values, not guaranteed fresh samples for all channels after one DRDY.
  - Production autoscan should use STATUS/UNREADCONVx and/or INTB sequencing.
- Add a small example helper in docs.

Tests:
- STATUS with only channel 0 unread must not mark all channels fresh.
- New helper or documented sample logic handles per-channel freshness.
- Existing `readAllChannels()` behavior remains backward-compatible.

#### 7. Conversion timing model

Improve docs/tests for conversion timing:
- `RCOUNTx`
- reference clock / internal vs external
- `CLOCK_DIVIDERSx`
- `SETTLECOUNTx`
- channel count/autoscan
- unknown/ambiguous overhead.

Do not overclaim precise sample rate if only approximation exists.

### Checks

Run full checks:
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

Append:
```markdown
## Prompt 02 — H3/M2 timing and freshness

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
git add include src test docs README.md examples
git commit -m "Tighten LDC1614 timing and data-ready semantics"
git push
```
