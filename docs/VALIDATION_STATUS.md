# LDC1614 Validation Status

This page tracks repeatable release checks and evidence that is still missing.
It is not a journal of completed implementation work and is not a release
certificate.

## Release state

The latest annotated tag is `v3.0.0`. `library.json` identifies the current
unreleased `3.1.0` candidate, including the public validation/readback helpers,
comprehensive diagnostic CLIs, and wrap-safe deadline behavior required by
external owners. The next release must be a reviewed annotated SemVer minor
release; do not move or reinterpret `v3.0.0`.

No retained run is positive acceptance evidence for the current code on an
exact sensor-equipped target.

## Required software checks

Run these checks on the final review revision:

```powershell
python tools/check_core_timing_guard.py
python tools/check_cli_contract.py
python tools/check_idf_example_contract.py
python tools/check_readiness_claims.py
python tools/test_ldc1614_hil_runner.py
python scripts/generate_version.py check
python tools/check_clean_consumer_compile.py
.\scripts\pio.cmd test -e native
.\scripts\pio.cmd run -e esp32s3dev
.\scripts\pio.cmd run -e esp32s2dev
```

The HIL runner host tests include parser self-test and no-port behavior. The
clean-consumer check creates and removes its own package archive in a temporary
directory.

CI additionally builds the native ESP-IDF diagnostic for ESP32-S2 and ESP32-S3
and generates Doxygen with warnings treated as errors. A successful build or
software test is not HIL evidence.

## Retained hardware evidence boundary

The [retained evidence index](https://github.com/janhavelka/LDC1614/blob/main/docs/reports/README.md)
records the intentional 2026-07-22 negative investigations plus the 2026-08-03
COM8 matrix and one-hour regression evidence. The JSON artifacts embed their
complete command results and target firmware identities; raw serial captures
are retained beside them. Both the older stacks and the maintained
pioarduino `55.03.311` / ESP-IDF 5.5.5 baseline exhibited NACK-related combined
read failures. Exact source review established that ESP-IDF 5.5.5 collapses
every synchronous non-`DONE` transaction result—including an ordinary
NACK—to `ESP_ERR_INVALID_STATE` (`259`). Historical detail 259 therefore does
not identify the failed ACK phase or prove a poisoned controller. The platform
update must not be presented as a recovery fix.

After a physical power cycle, clean `2358c30` firmware passed a no-scan cold
gate, 100 reset/reapply plus identity cycles (400/400 commands), one controlled
NACK/scan followed by explicit recovery/replay (8/8), and 100 post-NACK identity
cycles (300/300). A later comprehensive sequence nevertheless reproduced
detail 259 on the first identity read after a successful software-reset write.
These positive subsets narrow the trigger but do not close the intermittent
backend regression or qualify recovery.

A 2026-08-04 clean `4efeb5e` run failed its first automatic combined identity
read before any discovery, address-only probe, line clear, reset, or recovery.
Controller/device-handle reconstruction succeeded but the next complete
initialization failed in the same phase. A temporary direct open-drain reader
then observed idle-high lines, ACK for the `0x2A` write address, ACK for pointer
`0x7E`, and NACK for the repeated-start `0x2A` read address. After a confirmed
ten-second removal of both MCU and LDC power, corrected latch-preloaded
instrumentation received every ACK and returned MANUFACTURER_ID `0x5449` both
before and after normal ESP-IDF controller use. These dirty discriminator runs
are negative/diagnostic evidence, not release acceptance.

This proves a persistent LDC/physical-domain read-transition state that a
software controller rebuild does not clear and a true rail cycle does clear.
It rules out missing coils, a statically absent or shutdown target, held lines,
wrong static address, and stale ESP-IDF handles as the live failure. It does
not prove the initiating edge. TI explicitly warns that early-terminated I2C
traffic or an extraneous SDA pulse can corrupt the current or following
transaction; marginal VDD, SD, ADDR, pull-ups, or repeated-start integrity
remain physical alternatives until SDA/SCL and the control rails are captured
at the first recurrence. The retained TI PDF is byte-identical to the current
vendor download (Rev. A, March 2018; SHA-256
`B3BAB7A84C9A8423448113F24DE3B343C06CE7E7E4CBC6039B262B22130D8652`).

Exact clean `5e3199e` firmware repeated the reset-adjacent boundary: discovery,
26-transfer initialization, 24-transfer apply, and wake succeeded; the
`RESET_DEV` write succeeded; the immediately following MANUFACTURER_ID combined
read failed as transfer 2 of 27 with generic code 14/raw detail 259 and retained
partial-write/dirty provenance. After another rail cycle, a non-reset matrix
passed the same lifecycle and its MANUFACTURER_ID register read on target, but
the host runner falsely rejected the current structured output and stopped.
That parser-negative run is not a matrix pass; its unsent entries remain
`NOT_RUN`.

The current candidate has positive no-sensor diagnostic subsets, but none is a
release certificate or sensor-equipped product evidence.
Earlier compact v2
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

Historical clean `c3e2ed8` passed the 49-command default matrix, a 171-command extended
matrix with all configuration/invalid-input coverage, and 1,000/1,000 hardware
stress operations. Its full one-hour gate later reproduced detail 259 at
`resetreapply`, so that acceptance soak correctly did not start. The clean
post-fix runner then classified a following `init` failure correctly after
`busrecover` had returned success and target ACK. A separately labeled
`custom_reduced` gate passed scan, owner recovery, full replay,
wake, identity, and active-state checks before completing exactly 3,600 seconds:
3,197 cycles, 19,182 commands, zero soak-command failures/unknowns/resets, and
32 ms worst latency. Because every cycle reconstructs and re-admits the device,
this is positive no-reset owner-recovery stress, not a production-cadence soak.
Historical
recovery stress still showed bus-reset plus target ACK followed by an
initialization failure. The corrected diagnostic no longer treats that
sequence as recovery: it performs controller-only reconstruction, maps raw 259
as a generic transaction failure, and requires combined identity plus full
replay. A clean final one-hour gate for that corrected firmware remains
required.
Product-specific consumers may impose additional gates; TunnelMonitor's
remaining requirements are listed
in [TunnelMonitor integration gates](https://github.com/janhavelka/LDC1614/blob/main/docs/TUNNELMONITOR_INTEGRATION_GATES.md).

Use [HIL validation](HIL_VALIDATION.md) to collect structured JSON and raw
transcript evidence. Without real hardware and matching
firmware-reported revision/status, a runner result is `NOT_RUN`, not a pass.
