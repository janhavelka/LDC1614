# LDC1614 Validation Status

This page records the maintained validation boundary. It is not a release
certificate.

## Current Local Evidence

The current working branch has software hardening evidence for:

- Native PlatformIO tests in `native` and `native_cov` environments, including
  per-transfer fault injection, every-phase cancellation/deadline silence,
  identity/FIFO/backpressure, behavioral DATA/STATUS/INTB effects, atomic
  acquisition, full physical-channel profile validation, quality/provenance,
  exact LDC1612/LDC1614 replay payloads, four-channel fault/cancel/deadline
  paths, lifecycle, and helper boundaries. On 2026-07-19 both environments
  passed 29/29 tests. `native_cov` is coverage-instrumented only: the repository does
  not generate a report or enforce a threshold, so it is not measured coverage.
- Arduino-framework PlatformIO builds on 2026-07-19 passed for ESP32-S3
  (23,104 bytes RAM; 369,442 bytes flash) and ESP32-S2 (37,536 bytes RAM;
  359,845 bytes flash), using pinned platform release 54.03.20.
- Core timing/framework-boundary guard.
- Arduino diagnostic CLI command-contract guard.
- Native ESP-IDF example source-contract guard.
- Readiness wording guard.
- HIL runner host parser/no-port/no-sensor artifact tests. The current 26-test
  host suite rejects missing command payloads, identity/config/build facts,
  dirty or mismatched flashed revisions, and ambiguous zero-exit results.
- Historical v2 ESP32-S2 no-sensor HIL exists for an LDC1614 at `0x2A`.
  It predates the v3 cooperative API and cannot be counted as v3 execution
  evidence. The fixture also had no LC sensor or physical DRDY/INTB wiring.
- Generated `Version.h` consistency check.
- Clean package consumer compile guard.
- PlatformIO package creation, with generated package archives removed after
  review.

Pure ESP-IDF build status is separate evidence. Claim it only from `idf.py`
output or CI logs for `examples/esp_idf/basic`. `idf.py` was unavailable in the
2026-07-19 local validation context, so no local pure-IDF build is claimed.

## Checks To Run Before Merge Review

```sh
python -B -m py_compile scripts/generate_version.py tools/ldc1614_hil_runner.py tools/test_ldc1614_hil_runner.py tools/check_readiness_claims.py tools/check_clean_consumer_compile.py
python tools/check_core_timing_guard.py
python tools/check_cli_contract.py
python tools/check_idf_example_contract.py
python tools/check_readiness_claims.py
python tools/ldc1614_hil_runner.py --parser-self-test
python tools/ldc1614_hil_runner.py --dry-run --quiet
python tools/test_ldc1614_hil_runner.py
python scripts/generate_version.py check
python tools/check_clean_consumer_compile.py
python -m platformio test -e native
python -m platformio test -e native_cov
python -m platformio run -e esp32s3dev
python -m platformio run -e esp32s2dev
python -m platformio pkg pack
```

Remove any generated `.tar.gz` package after `pkg pack` unless it is explicitly
being prepared as a release artifact outside the source tree.

## Hardware Validation

Committed compact COM8 HIL logs record historical v2 ESP32-S2 + LDC1614
chip-only validation with no LC sensor attached:

- `docs/reports/hil-validation-COM8-20260701.md`
- `docs/reports/hil-validation-COM8-20260701.no-sensor-stress.runner.json`
- `docs/reports/hil-validation-COM8-20260701.no-sensor-negative-stress.runner.json`

For the recorded v2 revision, these logs exercised identity, register access,
configuration write/readback, sleep/wake, reset/reapply, recovery, and bounded
precondition/error behavior. They do not validate v3 operation IDs, deadlines,
transfer budgets, cancellation, exact-once results, applied-state invalidation,
or atomic acquisition. The repeated serial transcript is not retained. The
compact evidence therefore does not satisfy the raw transcript/logic-trace
gate and does not validate live conversion behavior.

On 2026-06-30, COM8 was identified as an ESP32-S2 target. The Arduino-profile
`esp32s2dev` firmware uploaded successfully once, but the runner then captured
no target firmware payload and all bounded CLI commands timed out with empty
output. A later upload retry could not open COM8. The generated files under
`docs/reports/hil-validation-COM8-20260630.runner.*` are marked
`overall_status=NOT_RUN`, `hardware_attached=false`, and
`evidence_type=serial_not_run`. They are audit artifacts only and do not prove
LDC1614/LDC1612 behavior.

Before target deployment decisions or full sensing claims, capture hardware logs
for:

- Device identity and expected I2C address strap for each board variant.
- LDC1612/LDC1614 channel availability.
- Sensor-attached configuration readback for timing, mode, drive-current, offset,
  and error registers.
- DATAx read ordering and DATAx/STATUS side effects.
- v3 initialization/acquisition transfer budgets, operation ID correlation,
  bus-silent cancellation/deadline expiry, and exact-once result collection.
- INTB behavior when wired.
- SD shutdown/wake behavior when wired.
- Address NACK, unplug/replug, timeout/fault, recovery, and bounded soak cases.
- Sensor clock plan, coil tuning, deglitch selection, and IDRIVE calibration.

Use `HIL_VALIDATION.md` and `../tools/ldc1614_hil_runner.py` to collect JSON and
Markdown evidence. A runner result without real hardware is `NOT_RUN`, not a
pass.
