# Retained HIL Evidence

This directory preserves transcript-bearing hardware evidence. It is an
evidence archive, not a release certificate; current acceptance gaps are listed
in [Validation status](../VALIDATION_STATUS.md).

## Structured runs

The five `hil-validation-COM8-20260722-*.runner.json` files are historical
post-v3 negative transport-regression runs. Each JSON file contains the complete
startup and command transcript plus machine-readable command results and target
firmware identity. Generated `.runner.md` copies are intentionally not retained.

Fixture: ESP32-S2 on COM8, LDC1614 at `0x2A`, a second responding device at
`0x3C`, and no LC sensor. Sensor-condition flags were permitted; transport,
identity, timeout, and nonzero-status failures were not.

| Firmware | Stack / recovery attempt | Retained observation |
|---|---|---|
| `a11fcb4` | pioarduino `54.03.20`, 400 kHz | Combined identity reads intermittently returned length mismatch, zero/`0xFFFF`, and `ESP_ERR_INVALID_STATE` after a scan found `0x2A` and `0x3C`. |
| `bf44cf1` | pioarduino `53.03.13`, NACK-heavy startup scan | 24 of 39 smoke commands failed after combined reads latched at zero length; the MCU and CLI remained responsive. |
| `ac710c1` | Wire teardown/rebegin after each scan | All 200 commands ran, but 16 of 25 following initialization reads failed despite reported recovery success. |
| `05a71d7` | Shared new-master delete/recreate | 20 of 25 following initialization reads failed with explicit `ESP_ERR_INVALID_STATE`; scans also produced false devices or timeouts. |
| `3036579` | ESP-IDF 5.3 public bus reset | 87 of 200 commands failed, including 19 of 25 initialization attempts; bus reset did not repair the stale state. |

This comparison selected an upgrade containing Espressif's earlier NACK
reporting fix (`459b75f`) instead of another core-library workaround. It did
not close the intermittent combined-read failure. Later exact-source review
showed that this ESP-IDF generation reports every synchronous non-`DONE`
transaction, including an ordinary NACK, as raw
`ESP_ERR_INVALID_STATE` (`259`). The retained failures are compatible with
[Espressif issue #14030](https://github.com/espressif/esp-idf/issues/14030),
but without the original SDA/SCL trace they do not prove the same root cause.

Compact 2026-07-01 v2 artifacts had no raw transcript and did not exercise the
v3 ownership contract, so they are available only through Git history.

## Raw serial transcripts

`transcripts/20260722/` contains eight raw serial captures recovered from the
old PlatformIO work directory before build-output cleanup:

- `hil-identity-characterization.serial.txt`
- `hil-inplace-identity-25x.serial.txt`
- `hil-post-failure-scan.serial.txt`
- `hil-transport-debug-100k.serial.txt`
- `hil-transport-debug-400k.serial.txt`
- `hil-transport-debug-400k-no-scan.serial.txt`
- `hil-transport-debug-400k-pio53.serial.txt`
- `hil-transport-debug-400k-pio53-cold.serial.txt`

These captures are historical debug evidence, including failures and dirty
firmware runs. They are not positive exact-release acceptance evidence.

## 2026-08-03 pioarduino 55.03.311 run

`20260803/` retains the exact clean-firmware diagnostics and the failed
one-hour invocation for ESP32-S2 COM8, LDC1614 at `0x2A`, a second device at
`0x3C`, and no LC sensor:

- `preflight.*`: 5/5 commands passed on clean `1263ab1`.
- `full-matrix.*`: 41/41 commands passed.
- `full-matrix-25x.*`: 1,025/1,025 command results passed.
- `hil-one-hour.*`: completed the requested 3,600-second duration, but base command 26
  (`resetreapply`) failed at the first identity read with backend detail 259
  (`ESP_ERR_INVALID_STATE`). The old runner incorrectly continued into 3,204
  meaningless soak cycles; its 9,674 subsequent failures are retained as
  negative evidence, not counted as an accepted soak.
- `postfix-matrix.*`: clean `fd5ccb3` firmware with an experimental 2 ms
  post-reset guard reproduced the same failure, disproving that delay as a
  corrective fix.
- `cold-gate.*`: after a physical power cycle, clean `2358c30` firmware passed
  the ordered no-scan `version`, `cfg`, `probe`, `init`, `probe` gate (5/5).
- `cold-reset-100x.*`: all 300 issued hardware commands passed, but the host
  artifact failed two fixture metadata checks because `cfg` was accidentally
  omitted. It is retained as procedural negative evidence, not promoted.
- `cold-reset-100x-pass.*`: the corrected clean-firmware run passed 400/400
  issued commands: 100 each of `version`, `cfg`, `resetreapply`, and `probe`.
- `nack-recovery-gate.*`: the controlled scan/recovery sequence passed 8/8;
  scan found only `0x2A` and the shared `0x3C` device, initialization replayed
  26/26 transfers, and the final driver state was clean and active.
- `post-nack-probe-100x.*`: 100 clean repetitions of `version`, `cfg`, and
  combined identity `probe` passed (300/300) after that NACK-heavy scan.
- `comprehensive-no-sensor.*`: 163/169 commands were classified PASS. Six
  false failures exposed host-runner defects in ANSI parsing, activation order,
  RCOUNT assumptions, and no-sensor quality-counter handling. The raw target
  output showed 1,000/1,000 stress and 1,000/1,000 mixed-stress operations with
  no transport failure; the artifact remains FAIL and is not acceptance.
- `comprehensive-no-sensor-reset-failure.*`: after correcting those parser and
  sequence defects, the clean target passed scan, explicit owner recovery,
  initialization, and apply, then failed `resetreapply` at its first identity
  read with backend detail 259. The old runner continued and recorded 28
  failures; the runner now fails fast and records unsent commands as
  `NOT_RUN`.

The runner now gates soak entry on a complete passing base matrix, preserves
partial serial evidence on exceptions, stops after the first unexpected base
failure, and counts only complete cycles. The reset failure left subsequent
combined reads unavailable, so the intended one-hour acceptance soak did not
start; raw detail 259 alone cannot identify whether the initiating NACK was
caused by the controller waveform, the LDC parser, or the electrical fixture.
Temporary dirty-firmware investigations were excluded from this 2026-08-03
candidate bundle. The separately indexed 2026-08-04 discriminator transcripts
are retained as diagnostic evidence and are not used to support acceptance.

### Earlier clean candidate and recovery experiments

An earlier clean diagnostic candidate was `c3e2ed8`:

- `final-c3e2ed8-default-matrix.*`: 49/49 commands passed, including the
  default reset/reapply path.
- `final-c3e2ed8-extended-matrix.*`: 171/171 commands passed, including all
  cache-only configuration boundaries, invalid-input rejection, and
  1,000/1,000 hardware stress operations.
- `final-c3e2ed8-one-hour-default-gate-failure.*`: the later complete soak gate
  reproduced detail 259 at base command 12 (`resetreapply`), so the soak did
  not start. The retained pre-fix JSON mislabeled that gate reason as a reboot
  banner because normal `help` output contained `=== LDC1614 CLI ===`; the
  corrected classifier reports the actual command failure first.
- `runner-81f0faa-c3e2ed8-corrected-gate-failure.*`: the clean post-fix runner,
  still exercising clean `c3e2ed8` firmware without an intervening power
  cycle, correctly reported `base command init was FAIL`. The preceding
  `busrecover` had returned success and target ACK, but the first initialization
  identity read again returned detail 259. The requested one-second
  confirmation soak did not start.
- `final-c3e2ed8-one-hour-reduced.*`: an explicitly labeled `custom_reduced`
  gate passed scan, owner recovery, full replay, wake, identity, and final
  active-state checks, then completed exactly 3,600 seconds: 3,197 complete
  cycles, 19,182 commands, zero soak-command failures, zero unknowns, zero
  resets, and 32 ms worst latency. Because each cycle reconstructed the
  controller and reinitialized the LDC, it is bounded no-reset owner-recovery
  and re-admission stress, not RESET_DEV or production-cadence qualification.
- `recovery-efce1be-live-pass.*`: one exact clean failed-state recovery/replay
  passed without a power cycle.
- `recovery-d5ce61b-repeat-failure.*`: the next repeated recovery stress failed
  its first initialization identity read after bus reset and target ACK.
- `recovery-24a198b-order-experiment-failure.*`: moving device registration
  after bus reset still failed at cycle 7; source review showed the reordering
  was hardware-state equivalent, so commit `24a198b` was reverted by `51c9a68`.

Together these artifacts demonstrate stable non-reset traffic while preserving
the unresolved reset-adjacent combined-read/recovery failure as a hard
acceptance failure. They do not establish a definitive physical culprit.

Positive release acceptance still requires a proven explicit recovery/replay
after the observed failure and a production sensor-cadence soak whose complete
gate includes that recovery behavior.
Sensor conversion, INTB, SD, `0x2B`, LDC1612, coil behavior, and ESP32-S3
product-owner behavior remain outside this archive.

## 2026-08-04 failure discriminator

`20260804/` retains the corrected clean diagnostic failure and the temporary
pin-level discriminator runs used to localize it:

- `comprehensive-no-sensor-4efeb5e.*`: clean `4efeb5e` firmware failed its
  first automatic MANUFACTURER_ID combined read with generic transaction code
  14 and raw ESP-IDF detail 259. No discovery, address-only probe, line clear,
  software reset, or owner recovery preceded that first transfer.
- `controller-recovery-4efeb5e.*`: controller/device-handle close and reopen
  succeeded, but the following complete initialization again failed on its
  first MANUFACTURER_ID read. Controller-resource reconstruction therefore did
  not recover the live condition.
- `transient-direct-identity-discriminator.*` and
  `transient-direct-identity-5x.*`: after releasing the ESP-IDF controller, a
  temporary roughly 100 kHz open-drain implementation repeatedly observed
  idle-high lines, ACK for `0x2A` write, ACK for pointer `0x7E`, then NACK for
  `0x2A` read after the repeated START. These dirty-build artifacts localize
  the persistent failed state but have no clean firmware identity and are not
  release acceptance evidence. The repetitions are also not independent
  recovery trials because TI warns that terminating an unexpected NACK path
  can affect a following transaction.
- `cold-start-direct-discriminator.*`: after a confirmed ten-second removal of
  power from both the ESP32-S2 and LDC rail, corrected instrumentation
  preloaded both GPIO output latches high, refused a non-idle bus, and retained
  its first pre-controller result. That first read and a later post-controller
  read both received every ACK and returned MANUFACTURER_ID `0x5449`. The
  command passed; the overall artifact intentionally remains FAIL because the
  one-off dirty discriminator did not report clean release metadata.
- `frequency-discriminator-4efeb5e.*` and
  `usb-control-line-check-4efeb5e.*` are incomplete serial-control experiments.
  They establish no I2C conclusion and are retained only so failed work is not
  silently discarded.
- `comprehensive-no-sensor-5e3199e.*`: exact clean `5e3199e` firmware passed
  startup, discovery, 26-transfer initialization, 24-transfer apply, and wake.
  Its successful `RESET_DEV` write was immediately followed by a failed
  MANUFACTURER_ID combined read at transfer 2 of 27 (`code=14`, raw detail
  `259`, `PARTIAL_WRITE`, applied state `DIRTY`). Fail-fast left 176 entries
  `NOT_RUN`. This repeats the reset-adjacent boundary but does not prove what
  originally drives the target into that state.
- `nonreset-exhaustive-5e3199e.*`: after another rail cycle, exact clean target
  output passed through initialization, apply, wake, probe, and the structured
  MANUFACTURER_ID `reg 0x7E` read. The older host matcher then falsely rejected
  that successful register output because metadata separated the address and
  value; 170 entries remained `NOT_RUN`. This is parser-negative evidence, not
  a completed hardware matrix.

Together these runs prove that the observed live failure was not a missing
coil, held bus, absent `0x2A` target, or stale ESP-IDF handle: the target could
receive its write address and pointer, but refused the transition to the read
address, and controller recreation did not change that state. A true target
rail cycle cleared it. The originating electrical edge remains unobserved, so
the evidence does not distinguish a prior unsupported/early-terminated
transaction from a marginal VDD/SD/ADDR transition or signal-integrity fault.
Assigning that initiating cause requires an SDA/SCL plus rail/control-pin
capture at the first recurrence.
