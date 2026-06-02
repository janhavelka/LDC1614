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

## Prompt 04 — Fix M4 and strengthen all prior fixes: native tests, fake transport, CI, and guards

### Audit findings this prompt must address

- **M4**: Native tests are useful but do not cover enough negative and variant cases.
- Also strengthen H2/H3/H4 from prior prompts with deeper fault injection and guard coverage.

### Subagents

Spawn:
1. `fake-bus-agent` — call-count/register-specific failure injection and transaction logging.
2. `variant-agent` — LDC1612/LDC1614 channel and raw/safe register tests.
3. `error-mapping-agent` — NACK/data NACK/timeout/bus/generic errors.
4. `ci-guard-agent` — CI, guard scripts, coverage.
5. `integration-review-agent` — deterministic tests and no false assumptions.

### Tasks

#### 1. Extend fake bus/test harness

Add capabilities:
- fail next read/write;
- fail on Nth transaction;
- fail for specific register read;
- fail for specific register write;
- record partial write before failing when needed;
- return specific statuses:
  - `I2C_NACK_ADDR`;
  - `I2C_NACK_DATA`;
  - `I2C_TIMEOUT`;
  - `I2C_BUS`;
  - `I2C_ERROR`;
- transaction log with operation, register, value, and order.

Keep the test fake readable.

#### 2. Add missing tests from exploration report

##### Identity / probe / begin
- wrong manufacturer ID;
- wrong device ID;
- address NACK;
- timeout;
- bus error;
- generic I2C error;
- probe missing callbacks behavior.

##### Variant handling
- `channelCount = 2` allows channels 0/1;
- rejects channels 2/3 for all high-level channel APIs;
- `channelCount = 4` allows channels 0..3;
- invalid `activeChan`;
- invalid enum casts for modes/config if applicable.

##### Register/data
- MSB-before-LSB order assertion for `readChannel()`;
- DATAx MSB error-bit masking;
- DATAx error flags surfaced separately;
- raw diagnostic write dirty/safe policy;
- read-only write rejection if safe metadata exists.

##### Partial state
- `_applyConfig()` nth-write failure for each register group:
  - RCOUNT;
  - OFFSET;
  - SETTLECOUNT;
  - CLOCK_DIVIDERS;
  - DRIVE_CURRENT;
  - MUX_CONFIG;
  - CONFIG;
  - ERROR_CONFIG if in apply path.
- `setSingleChannelMode()` partial failure.
- `resetAndReapply()` partial failure.
- full successful recovery clears dirty;
- failed recovery does not.

##### Timing and readiness
- invalid blocking calls return before wait/I2C;
- missing `nowMs` policy;
- `readDataReady()` preserves errors;
- `dataReady()` documented convenience behavior;
- fresh/unread helper if added.

##### Recovery
- degraded/offline transitions after repeated failures;
- busReset/hardReset callback behavior;
- softReset state behavior.

#### 3. CI and guards

Ensure CI runs:
- core timing/framework guard;
- CLI contract guard;
- IDF example contract guard;
- native tests;
- Arduino S2/S3 builds;
- package pack;
- pure IDF builds if configured.

Strengthen guards:
- core ban list covers `std::string`, `new`, `malloc`, `std::vector`, framework logs/timing;
- IDF example guard covers actual compiled sources;
- docs/metadata guard can optionally flag unsupported "production-grade" wording until prompt 05 resolves it.

#### 4. Coverage

Try to add coverage reporting for native tests:
- PlatformIO native coverage environment, gcov/lcov, or a documented command.

If coverage is not practical, document why in progress report and final report.

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

Run any added coverage command. Run pure IDF builds if available.

### Progress report

Append:
```markdown
## Prompt 04 — M4 tests, fault injection, CI, and guards

### Findings addressed
### Test harness changes
### Tests added
### CI/guard/coverage changes
### Commands run
### Remaining untested paths
```

### Commit and sync

```bash
git status --short
git add test tools .github docs README.md platformio.ini examples CMakeLists.txt idf_component.yml
git commit -m "Expand LDC1614 fault-injection tests and CI guards"
git push
```
