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
  paths, lifecycle, and helper boundaries. On 2026-07-22 `native` passed 29/29
  after the post-v3 identity/state, sample-quality, offset-validation, and
  uint64 rollover corrections; `native_cov` also passed 29/29. `native_cov` is
  coverage-instrumented only: the repository does not generate a report or
  enforce a threshold, so it is not measured coverage.
- Arduino-framework ESP32-S2 and ESP32-S3 build evidence is refreshed after
  final HIL acceptance. The maintained platform is pioarduino `55.03.39`
  (Arduino 3.3.9 / ESP-IDF 5.5.4), selected because it contains Espressif's
  NACK-state correction; see the transport-regression report below.
- Core timing/framework-boundary guard.
- Arduino diagnostic CLI command-contract guard.
- Native ESP-IDF example source-contract guard.
- Readiness wording guard.
- HIL runner host parser/no-port/no-sensor artifact tests. The current 34-test
  host suite rejects missing command payloads, identity/config/build facts,
  dirty or mismatched flashed revisions, non-finite timing values, stale or
  mismatched async results, incomplete soak cycles, invalid final driver state,
  unexpected reset banners, and ambiguous zero-exit results.
- Historical v2 ESP32-S2 no-sensor HIL exists for an LDC1614 at `0x2A`.
  It predates the v3 cooperative API and cannot be counted as v3 execution
  evidence. The fixture also had no LC sensor or physical DRDY/INTB wiring.
- Generated `Version.h` consistency check.
- Clean package consumer compile guard.
- PlatformIO package creation, with generated package archives removed after
  review.

Post-v3 COM8 transport evidence is recorded in:

- `docs/reports/hil-validation-COM8-20260722-transport-regression.md`
- `docs/reports/hil-validation-COM8-20260722-v3-smoke.runner.json`
- `docs/reports/hil-validation-COM8-20260722-v3-smoke.runner.md`
- `docs/reports/hil-validation-COM8-20260722-bf44cf1-smoke.runner.json`
- `docs/reports/hil-validation-COM8-20260722-bf44cf1-smoke.runner.md`
- `docs/reports/hil-validation-COM8-20260722-ac710c1-recovery-stress.runner.json`
- `docs/reports/hil-validation-COM8-20260722-ac710c1-recovery-stress.runner.md`
- `docs/reports/hil-validation-COM8-20260722-05a71d7-recovery-stress.runner.json`
- `docs/reports/hil-validation-COM8-20260722-05a71d7-recovery-stress.runner.md`
- `docs/reports/hil-validation-COM8-20260722-3036579-recovery-stress.runner.json`
- `docs/reports/hil-validation-COM8-20260722-3036579-recovery-stress.runner.md`

The v3.0.0 and `bf44cf1` smokes are intentionally retained as negative `FAIL`
artifacts. The first records the pioarduino 54.03.20 transport regression; the
second records a pioarduino 53.03.13 controller state that still accepted
address/write traffic while combined reads returned zero length after
diagnostic NACK traffic. The `ac710c1` stress further records that Wire
teardown/rebegin reported success on every cycle but restored only 9/25
following initialization reads. The `05a71d7` shared-new-master run exposed
raw `ESP_ERR_INVALID_STATE` (`259`) and proved that deleting/recreating handles
alone did not reset the controller. The `3036579` run proved that the public
ESP-IDF 5.3 bus-reset API also could not repair the driver's stale NACK state.
None is positive release evidence. A clean ESP-IDF 5.5.4 candidate smoke and
one-hour soak are required before this section can record a positive COM8
result.

The annotated `v3.0.0` tag is not moved by this work. Because `[Unreleased]`
adds native-IDF scan parity, an automated soak mode, and wrap-safe deadline
behavior, the next release is a SemVer MINOR release, not a v3.0.0 rewrite or a
patch-only release.

Pure ESP-IDF build status is separate evidence. Claim it only from `idf.py`
output or CI logs for `examples/esp_idf/basic`. `idf.py` is unavailable in the
current local validation context, so no local pure-IDF build is claimed.

## Documentation Evidence

On 2026-07-22, Doxygen 1.13.2 generated the public headers and maintained guides
with undocumented-member, missing-parameter, and documentation-error warnings
enabled as errors. It completed without warnings. Generated HTML was inspected
for the consolidated v3.0.0 release identity and the compatibility-only `CONFIG_UNKNOWN`
wording, then removed because `docs/doxygen/` is ignored build output.
The maintained Markdown local-link check also passed. Both native environments
were repeated after the public-header documentation changes and passed 29/29;
the clean package consumer and v3.0.0 package build passed as well.

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
