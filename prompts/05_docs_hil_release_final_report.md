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

## Prompt 05 — Fix H1/M7/L1/L2 docs and HIL gaps: documentation honesty, HIL workflow, and final report

### Audit findings this prompt must address

- **H1**: Production and industry-readiness claims are not supported by hardware validation.
- **M7**: Example labels and production guidance are too informal.
- **L1**: Generated version metadata is not reproducible.
- **L2**: `probe()` docs can be misread before configuration exists.
- Remaining hardware validation evidence gaps from the HIL matrix.

### Subagents

Spawn:
1. `docs-honesty-agent` — remove unsupported claims and label examples.
2. `hardware-integration-agent` — create LDC-specific hardware checklist.
3. `hil-runner-agent` — inspect actual CLI commands and create safe HIL runner/procedure.
4. `release-report-agent` — final report and release wording.
5. `integration-review-agent` — verify no false claims and all findings addressed.

### Tasks

#### 1. Remove or qualify production-grade claims

Audit:
```bash
rg -n "production-grade|industry-grade|field-ready|release-ready|validated|certified|HIL|hardware validated|production ready" README.md library.json idf_component.yml docs examples .github || true
```

Replace unsupported wording with honest wording:
- "framework-neutral";
- "production-oriented architecture";
- "under industry-readiness hardening";
- "hardware validation pending";
- "not a field-readiness certificate".

Keep verified strengths, but separate:
- software architecture readiness;
- build/test readiness;
- hardware validation readiness.

#### 2. Hardware integration checklist

Create or update:

```text
docs/HARDWARE_INTEGRATION.md
```

Include:
- LDC1612 has 2 channels; LDC1614 has 4.
- ADDR:
  - low => `0x2A`;
  - high => `0x2B`;
  - must not float.
- SD:
  - low normal;
  - high inactive;
  - must not float.
- CLKIN:
  - tie to GND for internal oscillator.
- INTB:
  - push-pull configurable interrupt;
  - not open-drain by default;
  - no pull-up required by IC;
  - board-specific conditioning may still exist.
- SCL/SDA require pull-ups.
- VDD 2.7 V to 3.6 V.
- Sensor frequency 1 kHz to 10 MHz.
- DATAx MSB contains error bits and data bits; do not treat raw 32-bit read as pure conversion data.
- RCOUNT/SETTLECOUNT/CLOCK_DIVIDERS determine timing.
- Coil design, target distance, metal/coating interpretation, and IDRIVE tuning are application-specific and require calibration/evidence.
- IDRIVE tuning may require oscilloscope/application procedure.

#### 3. Label examples

For every example:
- add clear label:
  - diagnostic bring-up;
  - safe smoke;
  - production integration template;
  - HIL-only;
  - fault-injection opt-in.
- Raw `reg/wreg` or equivalent commands must be labeled diagnostic and cache-desynchronizing unless safe metadata fully prevents that.
- Production docs should prefer:
  - app-owned bus;
  - injected transport;
  - external serialization;
  - `readDataReady()` and fresh/unread semantics;
  - explicit recovery after dirty state.

#### 4. Reproducible version metadata

Fix L1 if reasonable:
- Provide a reproducible mode for `scripts/generate_version.py`, or document why `__DATE__`/`__TIME__` is intentionally used.
- Do not break existing version generation.
- Add/update tests/checks if script behavior changes.

#### 5. Probe docs

Ensure L2 is corrected:
- `probe()` requires configured transport callbacks.
- It may be safe before successful `begin()` only if callbacks/config were supplied.
- It is not magic on a default constructed object.

#### 6. HIL runner/procedure

Inspect actual CLI support first:
```bash
rg -n "help|selftest|probe|read|readall|status|cfg|config|recover|sleep|wake|addr|scan|stress|HIL|Serial|command" examples tools docs src include
```

Create:

```text
tools/ldc1614_hil_runner.py
docs/HIL_VALIDATION.md
docs/hil/README.md
```

Only create commands the firmware actually supports, or make runner configurable.

Safe default group:
- connect/open serial;
- `help`;
- `version` if available;
- `scan` if available;
- `probe`;
- identity/config/status;
- safe read channel(s);
- safe read all;
- safe readiness/status;
- recover;
- short bounded stress/soak if supported.

Optional opt-in groups:
- address 0x2B if hardware selectable;
- SD shutdown/wake if SD wired;
- INTB observation;
- unplug/replug;
- induced NACK;
- stuck bus;
- long soak;
- drive-current/coil tuning evidence.

Runner requirements:
- no destructive tests by default;
- flags for opt-in tests;
- JSON and Markdown output;
- record timestamp, git commit, library version, firmware version, port, baud, address, channel count, command transcript, pass/fail/skip;
- never hide failures;
- if no hardware/serial is available, report "not run", not pass.

#### 7. Final comprehensive report

Create:

```text
docs/LDC1614_INDUSTRY_HARDENING_FINAL_REPORT.md
```

It must include:
- starting audit summary;
- each finding H1/H2/H3/H4/M1/M2/M3/M4/M5/M6/M7/L1/L2/L3 and how it was fixed or why deferred;
- changes by prompt;
- public API changes;
- tests added;
- commands run with exact results;
- CI/IDF status;
- HIL status:
  - runner/procedure created;
  - real hardware logs captured or not;
  - pending hardware matrix;
- remaining P0/P1/P2 work;
- merge recommendation;
- release recommendation;
- safe release wording and forbidden wording.

#### 8. Checks

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

Run HIL only if hardware is present and operator confirms.

#### 9. Progress report

Append:
```markdown
## Prompt 05 — H1/M7 docs, HIL, and final report

### Findings addressed
### Documentation changes
### HIL runner/procedure changes
### Hardware actually run
### Commands run
### Remaining work
```

### Commit and sync

```bash
git status --short
git add README.md library.json idf_component.yml docs tools examples scripts .github platformio.ini CMakeLists.txt
git commit -m "Document LDC1614 readiness and add HIL workflow"
git push
```

### Final response

Report:
- branch;
- commit hash;
- push/sync status;
- final report path;
- checks run;
- HIL status;
- whether ready to merge;
- whether ready to release;
- remaining blockers.
