# LDC1614 Validation Status

This page tracks repeatable release checks and evidence that is still missing.
It is not a journal of completed implementation work and is not a release
certificate.

## Release state

`library.json` and the latest annotated tag identify `v3.0.0`. The current
branch contains unreleased features and corrections, including the wrap-safe
deadline behavior required by external owners. The next release must be a
reviewed annotated SemVer minor release; do not move or reinterpret `v3.0.0`.

No retained run is positive acceptance evidence for the current code on an
exact sensor-equipped target.

## Required software checks

Run these checks on the final review revision:

```sh
python -B -m py_compile scripts/generate_version.py scripts/configure_esptool_upload.py tools/ldc1614_hil_runner.py tools/test_ldc1614_hil_runner.py tools/check_readiness_claims.py tools/check_clean_consumer_compile.py
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

`native_cov` is coverage-instrumented only. The repository does not generate a
coverage report or enforce a threshold. Remove generated package archives
after review unless they are intentionally retained outside the source tree as
release artifacts.

CI additionally builds the native ESP-IDF diagnostic for ESP32-S2 and ESP32-S3
and generates Doxygen with warnings treated as errors. A successful build or
software test is not HIL evidence.

## Retained hardware evidence boundary

The [committed 2026-07-01 ESP32-S2 evidence](https://github.com/janhavelka/LDC1614/blob/main/docs/reports/hil-validation-COM8-20260701.md)
is a historical v2 chip-only run with an LDC1614 at `0x2A` and no LC sensor. It
does not validate the v3 operation-ID, deadline, budget, cancellation,
applied-state, or atomic acquisition contract.

The retained 2026-07-22 post-v3 artifacts are intentional negative transport
records. They show NACK-related combined-read failure on older ESP-IDF stacks
and justify the original pioarduino `55.03.39` selection. The maintained
`55.03.311` baseline retains that NACK-state correction and adds ESP-IDF 5.5.5
I2C reset, allocation, and ISR fixes. The
[transport-regression report](https://github.com/janhavelka/LDC1614/blob/main/docs/reports/hil-validation-COM8-20260722-transport-regression.md)
indexes those artifacts and their embedded transcripts. None is positive
release evidence.

Keep these reports under `docs/reports/` while they remain cited here. Follow
the [artifact retention rules](https://github.com/janhavelka/LDC1614/blob/main/docs/hil/README.md);
do not replace evidence with a handwritten pass summary.

## Missing hardware evidence

Before release or target-deployment claims, capture evidence for:

- device identity and expected address strap for every board variant;
- LDC1612/LDC1614 channel availability and populated-channel mapping;
- sensor-attached configuration readback for timing, mode, drive current,
  offset, deglitch, clock, and error policy;
- live DATA MSB/LSB ordering plus DATA/STATUS/UNREAD/INTB side effects;
- correlated v3 initialization, apply, reset, acquisition, cancellation,
  deadline, invalidation, and exactly-once result behavior;
- address NACK, timeout, unplug/replug, controlled power loss, owner bus
  recovery, and complete replay while another shared-bus device remains usable;
- INTB and SD behavior when those pins are wired;
- sensor clock plan, coil tuning, target response, deglitch choice, and IDRIVE
  calibration; and
- a bounded soak at the production channel mask, cadence, and clock profile.

A clean pioarduino `55.03.311` / ESP-IDF 5.5.5 candidate no-sensor smoke and
one-hour soak are also required to close the COM8 transport regression.
Product-specific consumers may impose additional gates; TunnelMonitor's
remaining requirements are listed
in [TunnelMonitor integration gates](https://github.com/janhavelka/LDC1614/blob/main/docs/TUNNELMONITOR_INTEGRATION_GATES.md).

Use [HIL validation](HIL_VALIDATION.md) to collect structured JSON, Markdown,
and raw transcript evidence. Without real hardware and matching
firmware-reported revision/status, a runner result is `NOT_RUN`, not a pass.
