# LDC1614 Merge/Release Cleanup Report

Date: 2026-06-16
Branch: `hardening/ldc1614-industry-readiness`
Commit: pending at report creation; final cleanup commit hash is reported in
the final response and git history.

## Summary

Final documentation and repository hygiene cleanup for merge/release review.
This pass restores historical prompt/report evidence, adds current indexes, and
keeps the recommendation limited to software hardening/source release candidate
review with hardware validation pending.

## Docs cleaned

- Restored `docs/LDC1614_INDUSTRY_HARDENING_FINAL_REPORT.md`.
- Restored `docs/LDC1614_HARDENING_PROGRESS.md`.
- Restored `docs/LDC1614_INDUSTRY_READINESS_IMPLEMENTATION_PLAN.md` as a
  historical planning artifact.
- Restored `docs/IDF_PORT_IMPLEMENTATION.md` as a historical ESP-IDF hardening
  record.
- Added `docs/README.md` to identify authoritative readiness docs, hardware/HIL
  docs, ESP-IDF docs, application-note references, and historical artifacts.
- Added `docs/MERGE_RELEASE_CHECKLIST.md`.
- Updated README readiness text to separate software hardening from pending
  hardware/HIL and pure ESP-IDF evidence review.

## Prompts cleaned / archived

- Restored `prompts/` as historical evidence.
- Added `prompts/README.md`.
- Added `docs/prompts/README.md` for additional exploratory prompt artifacts.
- Added historical headers to restored prompt files so stale assumptions are not
  confused with current instructions.

## Metadata and release wording

- `library.json` remains at `1.0.0` with conservative framework-neutral wording.
- `idf_component.yml` remains at `1.0.0` with conservative component wording and
  explicit `esp32s2`/`esp32s3` targets.
- No version bump was made because this pass is docs/release-review cleanup only;
  publishing the current `Unreleased` changes should use an intentional later
  version decision.
- `Version.h` remains generated from `library.json`; it is now allowed to be
  tracked so source/package consumers get the required public header in clean
  checkouts.

## Changelog/release notes updates

- `CHANGELOG.md` now calls out dirty/sync-needed partial hardware-state
  contracts, timing/freshness APIs, ESP-IDF fixed-buffer CLI hardening, expanded
  tests/fault injection, HIL runner/procedure, reproducible version metadata,
  breaking/API changes, and remaining validation limits.

## Checks run

| Command | Result |
| --- | --- |
| `git status --short` | Clean at prompt start |
| `git branch --show-current` | `hardening/ldc1614-industry-readiness` |
| `git log --oneline --decorate -20` | Reviewed; prompt started at `f985aeb` |
| `python -m py_compile scripts/generate_version.py tools/ldc1614_hil_runner.py tools/check_readiness_claims.py` | Passed |
| `python tools/check_core_timing_guard.py` | `Core timing/framework guard PASSED` |
| `python tools/check_cli_contract.py` | `CLI contract PASSED` |
| `python tools/check_idf_example_contract.py` | `IDF example contract PASSED` |
| `python tools/check_readiness_claims.py` | `Readiness claims guard PASSED` |
| `python scripts/generate_version.py check` | `Up to date: ... include\\LDC1614\\Version.h` |
| `python -m platformio test -e native` | Passed; `126 test cases: 126 succeeded in 00:00:01.798` |
| `python -m platformio test -e native_cov` | Passed; `126 test cases: 126 succeeded in 00:00:01.297` |
| `python -m platformio run -e esp32s3dev` | Passed; `esp32s3dev SUCCESS 00:00:05.118` |
| `python -m platformio run -e esp32s2dev` | Passed; `esp32s2dev SUCCESS 00:00:04.647` |
| `python -m platformio pkg pack` | Passed; wrote `LDC1614-1.0.0.tar.gz` |
| Package artifact cleanup | Removed `LDC1614-1.0.0.tar.gz`; no package archive remains in the worktree |
| `python -m gcovr --version` | Failed locally: `No module named gcovr` |
| `idf.py --version` | Failed locally: `idf.py` is not recognized as a cmdlet/function/script/program |
| `idf.py -C examples/esp_idf/basic set-target esp32s3 build` | Not run locally because `idf.py` is unavailable |
| `idf.py -C examples/esp_idf/basic set-target esp32s2 build` | Not run locally because `idf.py` is unavailable |
| `git diff --check` | Passed; PowerShell/Git reported LF-to-CRLF normalization warnings only |

## CI items to verify externally

- GitHub Actions status for the pushed branch.
- Pure ESP-IDF `esp32s2` and `esp32s3` build matrix logs.
- Any package/consumer checks that require a clean external clone.

## Hardware/HIL status

No real LDC1612/LDC1614 hardware or serial port was selected for this cleanup
pass. The HIL runner exists, but no hardware validation logs were captured.

## Remaining blockers

- Real hardware/HIL logs.
- Pure ESP-IDF CI/local evidence review.
- Board-specific clock-plan validation.
- STATUS/DATAx/INTB/SD side-effect evidence.
- Fault injection and soak evidence on target hardware.

## Merge recommendation

Ready to merge as software hardening branch.

## Release recommendation

Ready for release only after listed evidence.

## Files changed

- `AGENTS.md`
- `.gitignore`
- `README.md`
- `CHANGELOG.md`
- `docs/README.md`
- `docs/MERGE_RELEASE_CHECKLIST.md`
- `docs/LDC1614_MERGE_RELEASE_CLEANUP_REPORT.md`
- `docs/LDC1614_INDUSTRY_HARDENING_FINAL_REPORT.md`
- `docs/LDC1614_HARDENING_PROGRESS.md`
- `docs/LDC1614_INDUSTRY_READINESS_IMPLEMENTATION_PLAN.md`
- `docs/IDF_PORT_IMPLEMENTATION.md`
- `docs/prompts/`
- `prompts/`
- `include/LDC1614/Version.h`
