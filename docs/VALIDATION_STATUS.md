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
python tools/check_core_timing_guard.py
python tools/check_cli_contract.py
python tools/check_idf_example_contract.py
python tools/check_readiness_claims.py
python tools/test_ldc1614_hil_runner.py
python scripts/generate_version.py check
python tools/check_clean_consumer_compile.py
python -m platformio test -e native
python -m platformio run -e esp32s3dev
python -m platformio run -e esp32s2dev
```

The HIL runner host tests include parser self-test and no-port behavior. The
clean-consumer check creates and removes its own package archive in a temporary
directory.

CI additionally builds the native ESP-IDF diagnostic for ESP32-S2 and ESP32-S3
and generates Doxygen with warnings treated as errors. A successful build or
software test is not HIL evidence.

## Retained hardware evidence boundary

The [retained evidence index](https://github.com/janhavelka/LDC1614/blob/main/docs/reports/README.md) records five intentional
negative 2026-07-22 post-v3 runs and eight raw serial debug captures. The JSON
artifacts embed their complete transcripts, command results, and target
firmware identities. They show NACK-related combined-read failure on older
ESP-IDF stacks and justify upgrading beyond the original pioarduino `55.03.39`
selection. The maintained `55.03.311` baseline retains the upstream NACK-state
correction and adds ESP-IDF 5.5.5 I2C reset, allocation, and ISR fixes.

None of the retained records is positive release evidence. Earlier compact v2
artifacts contained no raw transcript and did not validate operation IDs,
deadlines, budgets, cancellation, applied state, or atomic acquisition, so they
remain available only through Git history. Follow the [HIL artifact rules](HIL_VALIDATION.md#evidence-rules)
and never replace target evidence with a handwritten pass summary.

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

Use [HIL validation](HIL_VALIDATION.md) to collect structured JSON and raw
transcript evidence. Without real hardware and matching
firmware-reported revision/status, a runner result is `NOT_RUN`, not a pass.
