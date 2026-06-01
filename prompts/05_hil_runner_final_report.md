# Prompt 05 — HIL Diagnostic Runner, Hardware Validation Matrix, and Final Report

You are continuing the LDC1614 industry-readiness hardening sequence on the current hardening branch.

Complete only this prompt, commit and push/sync, then stop and report back.

## Goal

Add or improve the automatic hardware diagnostic surface so the repository can collect evidence later, then produce a comprehensive final hardening report. Do not claim hardware validation unless real commands are run on real hardware in this session.

## Subagents

Spawn:

- `hil-diagnostics-agent`
- `device-datasheet-agent`
- `docs-report-agent`
- `integration-review-agent`

## Main tasks

### 1. Hardware diagnostic CLI / runner audit

Inspect existing Arduino and ESP-IDF diagnostic CLIs and any Python HIL scripts.

Look for support for:

- version and build metadata;
- I2C scan;
- active address 0x2A/0x2B selection;
- manufacturer/device ID read;
- driver health/state;
- per-channel raw DATA read;
- frequency calculation output;
- status register decode;
- error register/config decode;
- RCOUNT/SETTLECOUNT/CLOCK_DIVIDERS/OFFSET/DRIVE_CURRENT dump;
- single-channel mode and autoscan mode;
- channel sequence selection;
- INTB/data-ready observation if available;
- reset/recover;
- sleep/wake or shutdown distinction;
- stress read loop;
- mixed operation stress;
- safe selftest that restores baseline config.

### 2. Add/extend Python HIL runner if suitable

If the repo already has a Python serial HIL runner, extend it. If not, add one only if consistent with the repository style.

Suggested file:

```text
tools/ldc1614_hil_runner.py
```

Runner requirements:

- accepts serial port, baud, address, channel mask, loop counts;
- supports dry-run mode;
- records exact command transcript;
- has safe default smoke profile;
- has opt-in longer stress profile;
- does not perform destructive sensor drive-current experiments by default;
- emits markdown report file under `hil_reports/` or `docs/hil/`;
- clearly marks `OPERATOR_REVIEW_REQUIRED` if any manual/physical observation is needed.

Safe default sequence:

```text
version
scan
addr
id
status
cfg
channels
read 0
read 1
read 2
read 3
selftest
stress 100
stress_mix 100
status
```

For LDC1612, channel 2/3 should be skipped or expected invalid. For LDC1614, they should be included if configured.

Optional profiles:

- `--include-autoscan`
- `--include-intb`
- `--include-sleep-wake`
- `--include-reset`
- `--include-long-stress`
- `--include-drive-current-readback`
- `--include-fault-unplug` with operator prompts

Do not include unsafe automatic drive-current sweeps unless the docs and hardware procedure make it clearly opt-in.

### 3. Hardware validation matrix

Create/update:

```text
docs/LDC1614_HARDWARE_VALIDATION_MATRIX.md
```

Include:

- address 0x2A and 0x2B validation;
- LDC1612 vs LDC1614 variant validation;
- all active channels;
- single-channel and autoscan modes;
- RCOUNT/SETTLECOUNT timing sanity;
- status/error flags;
- under/over range condition if safely inducible;
- INTB behavior;
- SD pin behavior if wired;
- CLKIN internal/external policy;
- reset/recover;
- I2C NACK/unplug/replug;
- stuck bus if test fixture supports it;
- brownout/power cycle;
- short stress, long soak;
- coil application validation explicitly separate from driver validation.

### 4. Final hardening report

Create:

```text
docs/LDC1614_HARDENING_FINAL_REPORT.md
```

Required sections:

1. Date, branch, commit.
2. Executive summary.
3. Original audit findings addressed.
4. Audit findings corrected/rejected.
5. Public API changes and migration notes.
6. Core changes.
7. Device-specific correctness changes.
8. ESP-IDF changes.
9. Arduino example changes.
10. Tests added.
11. Guard scripts added/updated.
12. Documentation changes.
13. CI/build/package coverage.
14. Exact local commands run and results.
15. Exact commands not run and why.
16. Hardware validation performed in this session, if any.
17. Hardware validation still pending.
18. Known risks.
19. Industry-readiness verdict.
20. What remains before merge/release.

The final verdict must be honest. Suggested wording unless real HIL is completed:

```text
Software hardening is materially improved and ready for code review/merge after checks pass. Full industry-grade/field-ready status remains blocked on physical LDC1614/LDC1612 hardware validation, INTB behavior, fault injection, and soak testing.
```

### 5. Final integration review

Before final commit, run:

```bash
git diff --stat
git diff --check
git status --short
```

Inspect for:

- build artifacts,
- generated package tarballs,
- accidental logs with local paths/secrets,
- overclaiming docs,
- source changes outside scope,
- untested hardware claims.

## Verification

Run all available:

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

If hardware is attached and safe, run the new/updated HIL runner. If not, do not pretend it ran.

## Commit and sync

Commit message:

```text
docs: add LDC1614 HIL matrix and final readiness report
```

Push/sync, then return concise final status with:

- branch,
- final commit,
- tests run,
- HIL status,
- remaining blockers,
- merge recommendation.
