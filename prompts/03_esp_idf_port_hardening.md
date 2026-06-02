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

## Prompt 03 — Fix H4/M6/L3: ESP-IDF path hardening

### Audit findings this prompt must address

- **H4**: ESP-IDF path is native but not production-ready.
- **M6**: INTB handling in the IDF example conflicts with datasheet wording.
- **L3**: `idf_component.yml` lacks explicit target metadata.
- Related docs: examples must be honest about diagnostic vs production use.

### Subagents

Spawn:
1. `idf-cli-agent` — IDF example input loop and command parser.
2. `idf-transport-agent` — IDF I2C error mapping, timeout, locking.
3. `ci-agent` — IDF guard/CI and component metadata.
4. `docs-agent` — IDF docs and example labels.
5. `integration-review-agent` — no Arduino leakage, no `std::string` in IDF path.

### Tasks

#### 1. Inspect actual IDF compile path

Run:
```bash
rg -n "std::string|<string>|String|Arduino|Wire|Serial|driver/i2c_master|i2c_master|gpio_|vTaskDelay|esp_|ESP_LOG|ESP_ERROR_CHECK|getchar|while|for \\(;;\\)" examples/esp_idf examples/common CMakeLists.txt idf_component.yml .github/workflows tools
cat examples/esp_idf/basic/main/CMakeLists.txt
```

#### 2. Remove heap-backed shared CLI from IDF example

The audit found the IDF example compiles shared CLI using `std::string`. Fix it.

Preferred:
- Create an IDF-specific fixed-buffer parser with limited safe commands:
  - `help`
  - `version`
  - `probe`
  - `status`/`drv`
  - `cfg`
  - `read`
  - `readall`
  - `ready`
  - `recover`
  - `timing`
  - `selftest` only if already safe and simple.
- Keep Arduino/common CLI unchanged if needed.
- Ensure IDF CMake no longer compiles heap-backed `examples/common/Ldc1614Cli.cpp` if it uses `std::string`.

Alternative:
- Refactor shared CLI to fixed-buffer/no-heap for both Arduino and IDF, only if smaller and safe.

Update `tools/check_idf_example_contract.py`:
- inspect actual IDF CMake source list if practical;
- fail if compiled IDF example sources contain:
  - `std::string`
  - `<string>`
  - Arduino `String`
  - `Arduino.h`
  - `Wire.h`
  - `Serial`.

#### 3. Bounded/nonblocking CLI loop

Replace or constrain blocking `getchar()` so periodic health/tick/progress can still run.

Acceptable:
- UART read with timeout;
- nonblocking stdin/VFS;
- command task plus driver task;
- loop that regularly calls a tick/progress function.

Document any remaining diagnostic limitation honestly.

#### 4. Add IDF I2C locking and better error mapping

In IDF transport/example:
- create/extend bus context with mutex/semaphore if multi-task access is possible;
- lock around I2C transactions;
- handle lock failures;
- map IDF errors as precisely as possible:
  - timeout => `I2C_TIMEOUT`;
  - invalid arg/state => invalid config/param or bus as appropriate;
  - NACK if distinguishable;
  - generic => `I2C_ERROR`;
- document any IDF API limitation where address/data NACK cannot be distinguished.

Avoid `ESP_ERROR_CHECK` in operational paths that are meant to demonstrate robust status handling.

#### 5. Fix INTB configuration

The audit says local docs/datasheet describe INTB as push-pull and no pull-up required.

- Configure IDF INTB GPIO as plain input by default.
- Add comment: INTB is push-pull active-low/configurable; do not assume open-drain.
- If a specific board pull-up is needed, document it as board-specific, not device-required.

#### 6. Component metadata and CI

- Add `targets:` to `idf_component.yml` only if build evidence or intended support is clear. Use `esp32s2` and `esp32s3` if CI/builds are configured for them.
- Wire `tools/check_idf_example_contract.py` into CI.
- Add pure ESP-IDF CI builds for `esp32s2` and `esp32s3` if repository CI style supports it.
- Add `sdkconfig.defaults` if needed.

Do not claim local pure IDF success unless `idf.py` was actually run.

#### 7. Docs

Update README/IDF docs:
- IDF example is diagnostic bring-up unless truly production-grade.
- Production apps own bus lifecycle, locking, timeouts, task scheduling, and recovery policy.
- Core remains IDF-free.
- Mention that `std::string`/heap-heavy parser was removed from IDF path if done.

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
idf.py --version
idf.py -C examples/esp_idf/basic set-target esp32s3 build
idf.py -C examples/esp_idf/basic set-target esp32s2 build
```

If `idf.py` is unavailable, report exact failure and ensure CI configuration is present.

### Progress report

Append:
```markdown
## Prompt 03 — H4/M6/L3 ESP-IDF hardening

### Findings addressed
### Implemented changes
### IDF example/component changes
### Tests/guards/CI added
### Commands run
### Remaining related work
```

### Commit and sync

```bash
git status --short
git add examples/esp_idf examples/common tools .github CMakeLists.txt idf_component.yml docs README.md
git commit -m "Harden LDC1614 ESP-IDF example and component path"
git push
```
