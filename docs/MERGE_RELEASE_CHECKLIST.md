# LDC1614 Merge/Release Review Checklist

Use this checklist with `docs/LDC1614_MERGE_RELEASE_CLEANUP_REPORT.md` during
merge and source release candidate review.

## Local Checks

- `python -m py_compile scripts/generate_version.py tools/ldc1614_hil_runner.py tools/check_readiness_claims.py`
- `python tools/check_core_timing_guard.py`
- `python tools/check_cli_contract.py`
- `python tools/check_idf_example_contract.py`
- `python tools/check_readiness_claims.py`
- `python scripts/generate_version.py check`
- `python -m platformio test -e native`
- `python -m platformio test -e native_cov`
- `python -m platformio run -e esp32s3dev`
- `python -m platformio run -e esp32s2dev`
- `python -m platformio pkg pack`
- Remove any generated package archive from the worktree after pack.

## External CI Evidence To Review

- Native tests and `native_cov`.
- Arduino-framework ESP32-S2 and ESP32-S3 PlatformIO builds.
- Core timing guard.
- CLI contract guard.
- ESP-IDF example source guard.
- Readiness wording guard.
- Generated version sync/check.
- Package pack.
- Pure ESP-IDF `esp32s2` and `esp32s3` build matrix logs.

## Hardware/HIL Evidence Still Required For Deployment Claims

- Real LDC1612/LDC1614 identity and address-strap logs.
- Board-specific clock plan and sensor frequency validation.
- DATAx/STATUS side-effect evidence, including unread conversion behavior.
- INTB behavior when wired.
- SD shutdown/wake behavior when wired.
- Fault-injection and recovery evidence.
- Soak/stability evidence on target hardware.

## Release Notes

- Confirm `CHANGELOG.md` includes hardening API changes and validation limits.
- Confirm package metadata wording remains conservative.
- Confirm no generated package archive remains in the worktree.
- Confirm no unsupported deployment-readiness claims are introduced.
