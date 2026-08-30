# LDC1612/LDC1614 driver audit &mdash; findings and proposals

One-time engineering artifact. Delete this file once the findings are dispositioned;
`tools/check_repository_hygiene.py` exists to keep exactly this kind of document from
accumulating in the tree.

Audited tree: commit `9a86034` (`v3.1.0`). Baseline at the time of audit: 48/48 native
tests, all six Python contract checkers, `generate_version.py check`, and Doxygen
(warnings-as-errors) all passing.

## Method

Twelve independent finder agents audited one dimension each (register encode/decode,
config validation, the job state machine, acquisition semantics, timing math, applied-state
lifecycle, protocol/hardware assumptions, API-vs-doc contract, examples, tests, dead code,
and UB/portability), grounded in a 364-fact extraction from the TI datasheet
(SNOSCY9A, 67 pages). Their 63 raw findings were merged into 34 candidates, and each
candidate was then attacked by three adversarial verifiers with distinct lenses
&mdash; **correctness** (read the path end to end, look for an existing guard),
**datasheet** (is the hardware claim actually right, and is this a documented deliberate
choice?), and **reachability** (construct a concrete firmware call sequence, or refute).
A finding is reported below only if at least two of three verifiers upheld it.

**23 findings upheld, 11 refuted.** Severity below is the verifiers' corrected severity,
not the finder's.

## What was already verified correct

These were checked against the datasheet and are **not** defects. They are recorded so the
same ground is not re-audited:

- The driver reproduces the datasheet's worked example (p.50&ndash;51) exactly:
  `MUX_CONFIG=0x820C`, `CONFIG=0x1601`, `DRIVE_CURRENT0=0x9000`, `CLOCK_DIVIDERS0=0x1002`.
- All 32 `nominalDriveCurrentMicroamps` values match Table 42 exactly.
- `calculateSensorFrequencyHz` matches datasheet equation (6) including the `OFFSET` term:
  `fSENSOR = FIN_DIV x fREF x (DATA/2^28 + OFFSET/2^16)`.
- The fREF limits (35/40/55 MHz), `RCOUNT>8`, `SETTLECOUNT>3` and `fIN < fREF/4` checks all
  match Table 43; `SETTLECOUNT` 0 and 1 both mapping to 32 cycles matches p.23.
- `tC = (RCOUNT x 16 + 4)/fREF` and channel-switch `692 ns + 5/fREF` match p.39.
- `readDataReady()` requiring `ErrorReporting::dataReady` is **correct** &mdash; the datasheet
  gates `STATUS.DRDY` on `ERROR_CONFIG.DRDY_2INT`, not just INTB.
- Deglitch `b111` for 33 MHz is the right resolution of the datasheet's internal conflict
  (the register-field text and the reset default `0x020F` outrank the application table's `b011`).
- Reserved-bit values written to `CONFIG`, `MUX_CONFIG`, `DRIVE_CURRENTx`, `CLOCK_DIVIDERSx`
  and `ERROR_CONFIG` are all correct, and read-only `INIT_IDRIVE` is correctly excluded from
  the readback comparison mask.

## Changes already applied

Applied in this pass, with the full check suite re-run green afterwards:

**Documentation cleanup** &mdash; 100 verified edits across 42 files. Each was proposed by a
scanner, independently re-derived by a verifier against the real tree, and applied only on a
unique verbatim text match. Highlights:

- `docs/RELEASING.md` was a transcript of the completed v3.1.0 release, hard-pinned to that
  version. Its own step-3 gate (*"the existing-tag query must print nothing"*) could never
  pass, because the annotated tag `v3.1.0` already exists and points at `HEAD`. Parameterised.
- `docs/VALIDATION_STATUS.md` still described 3.1.0 as an untagged candidate awaiting
  publication. Corrected to released-and-tagged.
- `docs/reports/README.md` and `docs/VALIDATION_STATUS.md` described the retained
  187/187 run as a pass of the maintained command matrix; the artifact itself records that
  the default matrix was skipped in favour of a hand-supplied list. Scope now stated.
- `examples/01_basic_bringup_cli/README.md` pointed readers at `makeBoardConfig()`, which
  does not exist anywhere in the repository; the real entry point is `makeDefaultConfig()`
  plus the `board::` constants.
- `tools/check_core_timing_guard.py` carried ~25 lines of unreachable code for a per-file
  allow-list contract that no longer exists.
- Fixture-specific and consumer-specific residue (a named COM port, a specific peer address)
  was generalised, keeping the library usable as both a standalone bring-up tool and a
  firmware component.

**Public API documentation corrections** (comment-only, no behaviour change):

- `Config.h` &mdash; `I2cWriteReadFn` now states the mandatory repeated START (datasheet 7.5.1,
  Figure 12). A STOP-separated write-then-read silently returns incoherent DATA, and nothing
  in the contract said so (F18).
- `Config.h` / `LDC1614.h` &mdash; `ErrorReporting::dataReady` and `DeviceStatus::dataReady` now
  state that `STATUS.DRDY` stays permanently clear unless the route is enabled (F19).
- `LDC1614.h` &mdash; `SampleQualityFlag::WATCHDOG` now documents that it is also raised from
  `STATUS.ERR_WD` via `ERR_CHAN` attribution, not only from `DATAx_MSB` (F20).
- `LDC1614.h` &mdash; `invalidateAppliedState()` now documents that it cancels the active job,
  consumes a result slot, and reserves that operation id until `takeResult()` drains it (F23).

Everything else below is left for your decision, because it changes behaviour, a public
contract, or a test expectation.

## Findings

Ordered by verified severity. `F04`, `F05`, `F09`, `F11`, `F15`, `F17`, `F21`, `F22`, `F24`,
`F29` and `F31` were refuted and are summarised at the end.

| ID | Severity | Status | Site | Upheld | What |
| --- | --- | --- | --- | --- | --- |
| **F01** | major | open | `src/LDC1614.cpp:1354` | 3/3 | Internal-oscillator worst case is taken from the caller's declared frequency/tolerance instead of... |
| **F02** | major | open | `src/LDC1614.cpp:963` | 3/3 | STATUS-after UNREADCONV is mislabelled DATA_LOST and invalidates coherent samples |
| **F03** | major | open | `src/LDC1614.cpp:1247` | 3/3 | writeRegister16 upgrades UNKNOWN to DIRTY, unlocking full config replay and acquisition on a devi... |
| **F06** | major | open | `src/LDC1614.cpp:1609` | 3/3 | estimateFrameTiming is not conservative for the internal oscillator: the clamp raises clockMin in... |
| **F16** | major | open | `examples/common/Ldc1614Cli.cpp:3083` | 3/3 | Session terminal results are relabelled FAILED, destroying CANCELLED/TIMED_OUT evidence |
| **F07** | minor | open | `src/LDC1614.cpp:803` | 3/3 | Failed destructive read records no hardware-effect evidence (no read counterpart to INDETERMINATE... |
| **F08** | minor | open | `src/LDC1614.cpp:746` | 3/3 | Successful configuration jobs zero _progress.effects, erasing all evidence that 14-27 register wr... |
| **F10** | minor | open | `src/LDC1614.cpp:938` | 2/3 | First acquisition after wake() reports a fabricated UNDER_RANGE error because sleep-cleared DATA ... |
| **F12** | minor | open | `src/LDC1614.cpp:312` | 2/3 | Every configuration replay writes CONFIG twice with a byte-identical value, wasting one I2C trans... |
| **F13** | minor | open | `src/LDC1614.cpp:646` | 2/3 | Write-failure effect/fault bookkeeping is copy-pasted across five sites, one of which re-implemen... |
| **F14** | minor | open | `src/LDC1614.cpp:673` | 2/3 | MANUFACTURER_ID and DEVICE_ID verification are two 31-line copy-pasted blocks |
| **F18** | minor | applied | `include/LDC1614/Config.h:19` | 3/3 | I2cWriteReadFn's contract never states the mandatory repeated START; a STOP-separated implementat... |
| **F19** | minor | applied | `include/LDC1614/Config.h:153` | 3/3 | ErrorReporting::dataReady is documented as INTB-only, but DRDY_2INT also gates STATUS.DRDY |
| **F20** | minor | applied | `include/LDC1614/LDC1614.h:127` | 3/3 | SampleQualityFlag::WATCHDOG is documented as DATA-only but is also raised from STATUS.ERR_WD |
| **F23** | minor | applied | `src/LDC1614.cpp:613` | 3/3 | invalidateAppliedState() silently cancels the active job and consumes a result-FIFO slot; its Dox... |
| **F25** | minor | open | `src/LDC1614.cpp:61` | 2/3 | firstChannel() is exactly nextChannel(mask, Channel::NONE) |
| **F26** | minor | open | `src/LDC1614.cpp:1145` | 3/3 | Sleep entry silently destroys unread conversions, latched errors, and INTB state with no effect f... |
| **F27** | minor | open | `include/LDC1614/LDC1614.h:161` | 2/3 | Sample quality silently degrades to raw endpoints when ERROR_CONFIG routing is off, and ERR_ZC ha... |
| **F28** | minor | applied | `tools/check_core_timing_guard.py:60` | 3/3 | check_core_timing_guard.py keeps ~25 lines of unreachable code for a per-file allow-list contract... |
| **F30** | minor | open | `examples/common/Ldc1614Cli.cpp:2821` | 3/3 | `verify` counts each register read failure as both a mismatch and a read failure |
| **F32** | minor | open | `examples/esp_idf/basic/main/CMakeLists.txt:42` | 3/3 | ESP-IDF example never supplies build metadata, so `version` reports an unusable build_timestamp |
| **F33** | minor | open | `test/test_basic.cpp:1781` | 2/3 | Test locks in silent loss of destructive-read provenance when a STATUS/DATAx_MSB read fails mid-a... |
| **F34** | minor | open | `test/support/FakeLdc1614Device.h:44` | 3/3 | LDC1612 register/channel gating is enforced only by the driver's own guards; the fake is variant-... |


---

### F01 &mdash; Internal-oscillator worst case is taken from the caller's declared frequency/tolerance instead of the datasheet 35-55 MHz device envelope, so validateConfig silently accepts configs that violate three Table 43 limits

**Severity:** major &nbsp;|&nbsp; **Site:** `src/LDC1614.cpp:1354` &nbsp;|&nbsp; **Independently reported by:** 3 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 3/3 upheld

**What is wrong**

For RefClkSrc::INTERNAL, clockMin/clockMax are derived from the application-declared `frequencyHz +/- tolerancePpm` and then only ever NARROWED to [35 MHz, 55 MHz]:

```cpp
  uint64_t clockMin = static_cast<uint64_t>(clock) *
      (PPM_SCALE - config.referenceClock.tolerancePpm) / PPM_SCALE;
  uint64_t clockMax = ceilDivide(
      static_cast<uint64_t>(clock) * (PPM_SCALE + config.referenceClock.tolerancePpm), PPM_SCALE);
  if (config.referenceClock.source == RefClkSrc::INTERNAL) {
    if (clockMin < INTERNAL_CLOCK_MIN_HZ) { clockMin = INTERNAL_CLOCK_MIN_HZ; }
    if (clockMax > INTERNAL_CLOCK_MAX_HZ) { clockMax = INTERNAL_CLOCK_MAX_HZ; }
  }
```

There is no corresponding WIDENING. The Electrical Characteristics table (p.7) specifies fINTCLK as MIN 35 / TYP 43.4 / MAX 55 MHz - a part-to-part device property that an application cannot narrow by declaring a tolerance. The normal declaration (`{INTERNAL, 43400000, 0}`) therefore leaves clockMin = clockMax = 43.4 MHz, and the clamp never fires.

Working out each downstream use of clockMin/clockMax shows the clamp direction is anti-conservative in every one of them:
- line 1443 `frefMin = clockMin / frefDivider` feeds the fIN < fREF/4 test at line 1464. A too-HIGH clockMin makes fREF/4 look larger, so the test is too weak.
- line 1444 `frefMax = ceilDivide(clockMax, frefDivider)` feeds the fREF <= 35/40/55 MHz test at line 1451 and the OFFSET test at line 1456. A too-LOW clockMax makes both too weak.

That the real floor is 35 MHz is confirmed by the datasheet's own Table 43 footnote "If fSENSOR >= 8.75 MHz, then FIN_DIVIDERx must be >= 2" - 8.75 MHz is exactly 35 MHz / 4, i.e. TI derives that rule from fREF being as low as 35 MHz.

Secondary consequence: a symmetric ppm interval cannot represent 35..55 MHz around 43.4 MHz at all (-19.4% / +26.7%), so the Config API cannot express the truth even if the integrator tries; and calculateSensorFrequencyHz (line 1578) then reports a sensor frequency built on the nominal with up to 27% error and no indication.

Also reported independently as: validateConfig narrows the internal-oscillator interval with the declared tolerance, so fREF-limit, fIN<fREF/4 and OFFSET checks accept configurations that violate Table 43 on real silicon; Internal-oscillator clock limits are narrowed by the caller's declared tolerance instead of the guaranteed 35-55 MHz device spread

**Evidence**

```
src/LDC1614.cpp:1354-1360 (the clamp), consumed at 1443-1444, 1451, 1456-1458, 1464. Verified by compiling src/LDC1614.cpp and calling LDC1614::validateConfig directly:

  A  internal 43,400,000 Hz, tol 0, SINGLE_CHANNEL CH0, FREF_DIVIDER=2, FIN_DIVIDER=2, sensor 8.9-9.0 MHz, deglitch 10 MHz -> ACCEPTED
  A' identical config declared as 35,000,000 Hz                              -> rejected "Worst-case sensor input must be below fREFx/4"

  B  internal 35,000,000 Hz, tol 0, SINGLE_CHANNEL, FREF_DIVIDER=1           -> ACCEPTED
  B' identical config declared as 55,000,000 Hz                              -> rejected "Channel reference clock tolerance exceeds limit"

  C  internal 35,000,000 Hz, tol 0, FREF_DIVIDER=2, FIN_DIVIDER=1, OFFSET=3744, expectedSensorMinHz=1,000,000 -> ACCEPTED
  C' identical config declared as 55,000,000 Hz                              -> rejected "OFFSET masks expected sensor minimum"

The existing test suite already sits on the edge of hole B: test/test_basic.cpp:489-491 asserts that internal 35 MHz with tolerancePpm = 1 and frefDivider = 1 is rejected; the same config with tolerancePpm = 0 is accepted.
```

**How it fails**

An integrator declares the internal oscillator honestly as `{RefClkSrc::INTERNAL, 43400000, 0}` (the datasheet TYP) and configures CH0 single-channel with FREF_DIVIDER0 = 2, FIN_DIVIDER0 = 2, expected sensor 8.9-9.0 MHz. bind() returns OK. On a part whose fINTCLK sits at the specified 35 MHz minimum, fREF0 = 17.5 MHz and fIN0 = 4.5 MHz, so fIN0 > fREF0/4 = 4.375 MHz - Table 43 is violated and the conversion result is meaningless, while the driver reports APPLIED_ACTIVE and hands the application a decoded 28-bit count with no error flag. The same declaration at 35 MHz nominal with FREF_DIVIDER = 1 lets a single-channel profile through whose fREF may actually be 55 MHz, 57% above the 35 MHz single-channel limit. In the OFFSET case the whole measurement range collapses: OFFSET = 3744 at the real 27.5 MHz fREF subtracts 1.57 MHz from a 1.0 MHz sensor, so DATAx reads 0x0000000 (permanent under-range) on every conversion. All three failures are part-dependent, so they pass bring-up on a typical unit and appear later in the field on units at the edge of the oscillator distribution.

**Proposed fix** (as corrected by adversarial review)

The finding's fix is directionally right and I verified it is behaviour-neutral on the existing suite, but make it a shared helper instead of an inline block, because estimateFrameTiming() has the identical bug at src/LDC1614.cpp:1605-1612 (it floors clockMin at 35 MHz only if the declared interval dips below, so {INTERNAL,43400000,0} yields frame timings up to 24% optimistic - the opposite of that function's documented 'conservative' contract).

Add one file-local helper next to ceilDivide:

  // fINTCLK is a device property (35 / 43.4 / 55 MHz, datasheet p.7). The
  // application cannot narrow it by declaring a tighter tolerance, so the
  // declared frequency/tolerance are never used as internal-clock limits.
  void referenceClockBoundsHz(const Config& config, uint64_t& minHz, uint64_t& maxHz) {
    if (config.referenceClock.source == RefClkSrc::INTERNAL) {
      minHz = INTERNAL_CLOCK_MIN_HZ;
      maxHz = INTERNAL_CLOCK_MAX_HZ;
      return;
    }
    const uint64_t clock = config.referenceClock.frequencyHz;
    minHz = clock * (PPM_SCALE - config.referenceClock.tolerancePpm) / PPM_SCALE;
    maxHz = ceilDivide(clock * (PPM_SCALE + config.referenceClock.tolerancePpm), PPM_SCALE);
  }

Then replace src/LDC1614.cpp:1347-1365 with a call plus the existing external-range rejection kept verbatim:

  uint64_t clockMin = 0; uint64_t clockMax = 0;
  referenceClockBoundsHz(config, clockMin, clockMax);
  if (config.referenceClock.source != RefClkSrc::INTERNAL &&
      (clockMin < EXTERNAL_CLOCK_MIN_HZ || clockMax > EXTERNAL_CLOCK_MAX_HZ)) {
    return Status::Error(Err::INVALID_CONFIG,
                         "Reference clock tolerance exceeds source range");
  }

and replace estimateFrameTiming's 1605-1612 with the same call, using minHz (delete its one-sided floor). Keep the declared-nominal 35..55 MHz check at 1333-1337 unchanged - it still guards calculateSensorFrequencyHz, which legitimately uses the nominal. Add one line to the ReferenceClock doc in Config.h: for RefClkSrc::INTERNAL, frequencyHz/tolerancePpm feed only the nominal frequency conversion and never limit checking.

Do NOT adopt the finding's extra 'Reference clock tolerance exceeds source range' restructuring as a new rule - that check already exists and must stay byte-identical or test_clock_offset_mode_and_config_encoding_boundaries (test_basic.cpp:2474-2478) changes message.

No test changes. I compiled both the original and the patched validateConfig and ran every config the suite exercises: valid single/multi 1612/1614, case17/18/22, toleranceLimit (35 MHz + 1 ppm + FREF_DIVIDER 1), worstCaseRatio, deglitchEquality, offset 1023/1024 external, ext 40 MHz/2 MHz + 1 ppm, wider tolerance 400000 ppm, and the datasheet worked example - accept/reject and message are identical in every case, because makeConfig's 200000 ppm already clamps to 35 MHz. Both timing assertions also hold: external.wakeAndSettleUs stays 471 (external path untouched; wake still uses the 35 MHz fINT floor) and conservative.sequentialFrameUs >= one.sequentialFrameUs becomes an equality, which still passes.

Regression test (test_basic.cpp, next to the toleranceLimit case at :488):
  Config internalEnvelope = valid;
  internalEnvelope.referenceClock = {RefClkSrc::INTERNAL, 43400000U, 0U};
  internalEnvelope.deglitch = Deglitch::BW_10MHZ;
  internalEnvelope.channel[0].finDivider = 2U;
  internalEnvelope.channel[0].expectedSensorMinHz = 8900000U;
  internalEnvelope.channel[0].expectedSensorMaxHz = 9000000U;
  assertCode(Err::INVALID_CONFIG,
             LDC1614::LDC1614::validateConfig(internalEnvelope));
No FakeLdc1614Device interaction is needed beyond supplying the callbacks, and validateConfig must still touch zero transfers (assert fake.transferCalls == 0 as the surrounding cases do).


---

### F02 &mdash; STATUS-after UNREADCONV is mislabelled DATA_LOST and invalidates coherent samples

**Severity:** major &nbsp;|&nbsp; **Site:** `src/LDC1614.cpp:963` &nbsp;|&nbsp; **Independently reported by:** 2 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 3/3 upheld

**What is wrong**

_buildAcquisition derives overrunChannels from the post-readout STATUS snapshot (lines 926-927: `_scratchStatusAfter.unreadChannels.bits & batch.selectedChannels.bits`), turns every such bit into SampleQualityFlag::DATA_LOST (lines 963-965), and then lists DATA_LOST in `invalidFlags` (line 974) so the channel is removed from `validChannels` (line 975) and added to `errorChannels` (line 982).

That inference is not sound. The datasheet (p.48) states "When the DATAx_MSB register is read, the DATAx_LSB register is updated with the corresponding LSB conversion data, and the UNREADCONVx flag is cleared", and a STATUS read does not clear UNREADCONVx (p.47 clearing list names only the error bits and ERR_CHAN). Every selected channel in a successful batch has had its DATAx_MSB read, so its UNREADCONVx was consumed. Therefore a UNREADCONVx bit still set in STATUS-after can only mean one thing: a NEW conversion for that channel completed after its MSB read. It cannot mean the sample the driver just published was lost or corrupted. Coherency is separately guaranteed by the buffered shadow (p.47: "the conversion results are stored in an internal buffer after every conversion, but the I2C DATAx field is only updated to reflect new data when the DATAx_MSB register is read"), so the published MSB/LSB pair is the newest complete conversion available at its read instant, regardless of what happens afterwards.

The driver therefore declares a perfectly good, coherent, newest-available sample invalid and flags the channel as being in error whenever the device converts faster than the batch takes to finish - a condition the datasheet explicitly calls normal (p.11: "it is possible to configure the conversion interval to be shorter than the time required to read back the DATAx registers"). Earlier-read channels in a multi-channel batch are hit far harder than the last-read channel, so the mask is also arbitrarily biased by read order.

The repository's own hardware log proves the incidence: docs/reports/20260804/nonreset-exhaustive-post-rail-v2-e4d0436.serial.txt line 1231 records `selected=100 valid=0 fresh=94 error=100 overrun=49` at 746 Hz for a single channel, and line 1238 records `overrun=51`. About half of all real acquisitions already carry the flag, and docs/HIL_VALIDATION.md:218 makes "non-overrun" a hard acceptance requirement for every sample, so that gate cannot pass on healthy hardware at ordinary conversion rates.

_Caveat on that log_: it is the **no-sensor** fixture (`bounds_fail=100`, every raw count 0, so `error=100` and `valid=0` are independently explained by the absent LC tank). What the run does establish is the **incidence** &mdash; roughly half of all batches already set the overrun flag at 746 Hz. The `valid=0` consequence of that flag only becomes the dominant effect once a real sensor is attached and the other invalidity reasons go away.

Also reported independently as: STATUS.UNREADCONVx observed after readout is decoded as data loss, invalidating coherent samples

**Evidence**

```
src/LDC1614.cpp:926-927
  batch.overrunChannels.bits = static_cast<uint8_t>(
      _scratchStatusAfter.unreadChannels.bits & batch.selectedChannels.bits);

src/LDC1614.cpp:963-965
    if ((batch.overrunChannels.bits & bit) != 0U) {
      sample.quality |= sampleQualityFlag(SampleQualityFlag::DATA_LOST);
    }

src/LDC1614.cpp:967-983
    const SampleQualityFlags invalidFlags =
        sampleQualityFlag(SampleQualityFlag::STALE) |
        ...
        sampleQualityFlag(SampleQualityFlag::DATA_LOST);
    if ((sample.quality & invalidFlags) == 0U) {
      batch.validChannels.bits |= bit;
    }
    ...
    if ((sample.quality & errorFlags) != 0U) {
      batch.errorChannels.bits |= bit;
    }

Hardware evidence, docs/reports/20260804/nonreset-exhaustive-post-rail-v2-e4d0436.serial.txt:1231
  session_channel=0 selected=100 valid=0 fresh=94 error=100 overrun=49 ...

Test that pins the wrong behaviour, test/test_basic.cpp:1900-1906
  fake.scheduleConversionAfter(2, 0, 0x07654321U);
  ...
  TEST_ASSERT_EQUAL_HEX32(0x01234567U, result.sampleBatch.channel[0].rawCount28);
  TEST_ASSERT_TRUE(hasSampleQuality(..., SampleQualityFlag::DATA_LOST));
  TEST_ASSERT_FALSE(result.sampleBatch.validChannels.contains(Channel::CH0));
(the asserted rawCount28 is exactly the coherent value the driver read, yet the channel is declared invalid)
```

**How it fails**

LDC1614 in single-channel continuous mode on CH0, external fREF = 40 MHz, RCOUNT0 = 0x0100 -> tC = (256*16+4)/40 MHz = 102 us. The owner polls acquisition on a 5 ms cadence with budget 1 per poll, so the four-transfer batch (STATUS, DATA0_MSB, DATA0_LSB, STATUS) spans well over 102 us. Sequence: STATUS-before shows UNREADCONV0=1 -> FRESH. DATA0_MSB read returns conversion N and clears UNREADCONV0; DATA0_LSB returns the matching shadow -> rawCount28 is conversion N, fully coherent. Before STATUS-after runs, conversion N+1 completes and sets UNREADCONV0 again. STATUS-after therefore reports UNREADCONV0=1. The driver publishes `overrunChannels = {CH0}`, marks the sample DATA_LOST, leaves `validChannels` EMPTY and sets `errorChannels = {CH0}`. The application, following README.md:230 ("All of those conditions exclude the affected channel from validChannels; errorChannels and per-channel quality retain the cause"), discards conversion N as unusable and logs a sensor error. Every acquisition behaves identically, so the driver never produces a usable sample and permanently reports the healthy sensor as faulty. The same run on real silicon at 746 Hz already shows this on 49-51 of every 100 batches.

**Proposed fix** (as corrected by adversarial review)

Minimal, and narrower than the finding's version - do not bundle the STALE question here, it is a separate claim and must be judged on its own evidence.

1. src/LDC1614.cpp:967-975: delete the line `sampleQualityFlag(SampleQualityFlag::DATA_LOST) |` from invalidFlags. That is the whole core fix - errorFlags at :978-981 is derived from invalidFlags, so removing it there also stops the channel entering errorChannels, with no second edit.
2. include/LDC1614/LDC1614.h:130: keep the enumerator name and value (v3.1.0 is released; renaming is a gratuitous API break) but correct the doc to what STATUS-after can actually prove: 'A newer conversion for this channel was already pending when the batch finished. The published sample is coherent and was the newest available at its DATAx_MSB read instant; this is a cadence/decimation signal, not a fault.' Mirror that at :164 for overrunChannels.
3. Fix the two owner gates that would otherwise keep rejecting healthy samples even after the mask is fixed - the finding's proposal misses these: examples/common/Ldc1614Cli.cpp:3217 and :3228, and examples/esp_idf/basic/main/Ldc1614IdfCli.cpp:3251 and :3262, must drop `overrun` from the rejection condition and its failure-reason bit. Keep the per-channel counting at Ldc1614Cli.cpp:2094 / Ldc1614IdfCli.cpp:2128 and the printed `overrun=` fields untouched - overrun stays an observable, it just stops being a verdict.
4. Docs: README.md:227-231 ('All of those conditions exclude the affected channel from validChannels') and docs/HIL_VALIDATION.md:218 (non-overrun as an acceptance requirement).

Exactly one existing test must change, and it is wrong today rather than merely inconvenient: test_acquire_detects_conversion_overrun_without_breaking_shadow_coherency (test_basic.cpp:1890-1907). It uses fake.scheduleConversionAfter(2, 0, 0x07654321U) so the new conversion lands after the MSB read, then asserts rawCount28 == 0x01234567 - i.e. it proves the driver captured the coherent pre-overrun pair - and then asserts TEST_ASSERT_FALSE(validChannels.contains(CH0)). The test simultaneously asserts the sample is correct and that the driver discards it; the second assertion is the defect, pinned. Rewrite the tail as:
  TEST_ASSERT_EQUAL_HEX32(0x01234567U, result.sampleBatch.channel[0].rawCount28);
  TEST_ASSERT_TRUE(result.sampleBatch.overrunChannels.contains(Channel::CH0));
  TEST_ASSERT_TRUE(hasSampleQuality(result.sampleBatch.channel[0].quality,
                                    SampleQualityFlag::DATA_LOST));
  TEST_ASSERT_TRUE(result.sampleBatch.validChannels.contains(Channel::CH0));
  TEST_ASSERT_FALSE(result.sampleBatch.errorChannels.contains(Channel::CH0));
and rename it to test_acquire_reports_overrun_without_invalidating_the_coherent_sample. Owners wanting the old policy compose `validChannels.bits & ~overrunChannels.bits`, which is the policy-outside-the-driver split AGENTS.md mandates.


---

### F03 &mdash; writeRegister16 upgrades UNKNOWN to DIRTY, unlocking full config replay and acquisition on a device whose identity was never verified

**Severity:** major &nbsp;|&nbsp; **Site:** `src/LDC1614.cpp:1247` &nbsp;|&nbsp; **Independently reported by:** 3 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 3/3 upheld

**What is wrong**

`AppliedConfigState` is used as the proxy for "identity has been established in this binding": `startApplyConfig()` (line 444) admits every state except `UNKNOWN`. But `writeRegister16()` assigns `DIRTY` unconditionally whenever a mutation is possible, without checking the current state. `DIRTY` is *more* trusted than `UNKNOWN` in this state machine, so a single diagnostic register write on a freshly bound driver silently promotes it out of the only state that forces `startInitialize()`. From `DIRTY` the owner can run `startApplyConfig()` (which performs no identity read at all), reach `APPLIED_SLEEPING`, `wake()` to `APPLIED_ACTIVE`, and then `startAcquire()` — with MANUFACTURER_ID/DEVICE_ID never having been read. This breaks AGENTS.md ("The initialization job reads MANUFACTURER_ID and DEVICE_ID before applying configuration"), the header contract for `startApplyConfig` ("Requires established identity/config state"), and docs/I2C_INTEGRATION.md ("Only the required complete initialization—both combined identity reads followed by full replay—re-establishes trusted LDC applied state"). Note that `updateDesiredConfig()` at line 428 already implements the correct rule (`if (_appliedState != AppliedConfigState::UNKNOWN) _appliedState = DIRTY;`), which shows the invariant was understood and simply not applied here.

Also reported independently as: startApplyConfig's documented "requires established identity" precondition is not enforced; DIRTY is reachable without ever reading MANUFACTURER_ID/DEVICE_ID; startResetAndReapply leaves DIRTY (not UNKNOWN) after a confirmed RESET_DEV write whose follow-up identity read failed, again permitting identity-free replay

**Evidence**

```
src/LDC1614.cpp:1245-1249
```cpp
  const bool mutationPossible = status.ok() || writeFailureMayHaveCommitted(status);
  if (mutationPossible) {
    _appliedState = reg == cmd::REG_RESET_DEV ? AppliedConfigState::UNKNOWN
                                              : AppliedConfigState::DIRTY;
  }
```
gate it defeats, src/LDC1614.cpp:444-446:
```cpp
  if (_bound && _appliedState == AppliedConfigState::UNKNOWN) {
    return Status::Error(Err::CONFIG_DIRTY,
                         "Identity/configuration not established");
```
Compiled repro (driver + test/support/FakeLdc1614Device.h), actual output:
```
A: bind ok=1 state=UNKNOWN
A: wreg ok=1 state=DIRTY
A: startApplyConfig code=5 inProgress=1
A: poll code=0 state=APPLIED_SLEEPING
A: identity registers ever read? <none in transferLog>
A: total transfers=15
A: wake code=0 state=APPLIED_ACTIVE
A: startAcquire inProgress=1
```
Reachable from the shipped CLI: `examples/common/Ldc1614Cli.cpp:3906` (`wreg` -> `writeRegister16`) followed by `examples/common/Ldc1614Cli.cpp:1887` (`apply` -> `startApplyConfig`).
```

**How it fails**

A board is strapped ADDR=VDD but the profile selects `I2cAddress::ADDR_GND` (0x2A), where an unrelated peripheral on the shared bus lives. The operator binds, pokes one register with `wreg 0x08 0x0123 confirm` to sanity-check the bus, then runs `apply`. Because `wreg` moved the driver UNKNOWN -> DIRTY, `startApplyConfig` is admitted and writes 14 LDC register values into the foreign device (which ACKs them), ends `APPLIED_SLEEPING`, `wake()` reports `APPLIED_ACTIVE`, and `startAcquire()` returns "verified active configuration" batches decoded from that device's registers. The identity gate that exists precisely to prevent this is never executed.

**Proposed fix** (as corrected by adversarial review)

Prefer the finding's SECOND option, not its first. The `_markDirty()` monotone helper alone does not close the startResetAndReapply path, because that transition is APPLYING -> DIRTY: `_markDirty()` would happily perform it. Stop overloading AppliedConfigState as the identity gate.

(a) In include/LDC1614/LDC1614.h add a private `bool _identityVerified = false;`. Set it true at exactly one place - src/LDC1614.cpp in the deviceStep block, immediately before `++_jobStep;` at ~:735, i.e. only after DEVICE_ID has matched (which implies MANUFACTURER_ID matched at ~:686). Clear it in bind() (:370, beside `_appliedState = UNKNOWN`), in end() (:386), and in invalidateAppliedState() (:513).

(b) Change the gate at :444 to:
  if (_bound && (!_identityVerified ||
                 _appliedState == AppliedConfigState::UNKNOWN)) {
    return Status::Error(Err::CONFIG_DIRTY,
                         "Identity/configuration not established");
  }
Keep the UNKNOWN half deliberately - dropping it would newly admit apply after a raw RESET_DEV write and after a re-INITIALIZE whose identity read failed on an already-verified binding, both of which today (correctly) demand a full initialize. This is a conjunction, so it can only tighten, never loosen.

(c) Optional one-liner, worth taking because it is free and makes the public appliedConfigState() stop over-reporting knowledge the driver does not have: at :1247 write
  _appliedState = (reg == cmd::REG_RESET_DEV ||
                   _appliedState == AppliedConfigState::UNKNOWN)
                      ? AppliedConfigState::UNKNOWN
                      : AppliedConfigState::DIRTY;
DIRTY means 'hardware may differ from desired'; on a never-initialized binding the honest answer is still UNKNOWN.

No existing test changes. I checked every startApplyConfig call site in test_basic.cpp (:840, :1148, :1246-1266, :1382, :1478, :1494, :1518, :1545, :2033, :2239, :2270) and test_cli.cpp (:708) - all are preceded by a successful initialize/initializeToSleeping, so _identityVerified is true in each. test_basic.cpp:2075 asserts DIRTY after writeRegister16, but that write follows initialize() + wake() + sleep(), so it transitions APPLIED_SLEEPING -> DIRTY and is unaffected by (c). No test asserts DIRTY after a diagnostic write on a never-initialized driver, and test_cli.cpp:661 asserts only the wreg output line and transferCalls == 5, neither of which moves.

Regression test (test_basic.cpp), covering both paths:
  // Raw write must not buy identity trust.
  FakeLdc1614Device fake; LDC1614::LDC1614 driver;
  Config config = makeConfig(fake);
  TEST_ASSERT_TRUE(driver.bind(config).ok());
  TEST_ASSERT_TRUE(driver.writeRegister16(cmd::REG_RCOUNT0, 0x0123).ok());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::UNKNOWN),
                          static_cast<uint8_t>(driver.appliedConfigState()));
  const uint16_t before = fake.transferCalls;
  assertCode(Err::CONFIG_DIRTY, driver.startApplyConfig(9401, DEADLINE_MS));
  TEST_ASSERT_EQUAL_UINT16(before, fake.transferCalls);
  // Reset write committed, identity read failed: still no replay trust.
  FakeLdc1614Device resetFake; LDC1614::LDC1614 resetDriver;
  Config resetConfig = makeConfig(resetFake);
  TEST_ASSERT_TRUE(resetDriver.bind(resetConfig).ok());
  resetFake.failOnTransfer(2U, FORCED_READ);
  TEST_ASSERT_TRUE(resetDriver.startResetAndReapply(9402, DEADLINE_MS).inProgress());
  assertCode(FORCED_READ.code, pollToTerminal(resetDriver));
  (void)takeResult(resetDriver);
  assertCode(Err::CONFIG_DIRTY, resetDriver.startApplyConfig(9403, DEADLINE_MS));


---

### F06 &mdash; estimateFrameTiming is not conservative for the internal oscillator: the clamp raises clockMin instead of pinning it to the 35 MHz device floor

**Severity:** major &nbsp;|&nbsp; **Site:** `src/LDC1614.cpp:1609` &nbsp;|&nbsp; **Independently reported by:** 2 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 3/3 upheld

**What is wrong**

For RefClkSrc::INTERNAL the worst-case (slowest) reference is derived from the application-declared nominal minus its declared tolerance, and is then clamped only UPWARD to 35 MHz:

  uint64_t clockMin = frequencyHz * (PPM_SCALE - tolerancePpm) / PPM_SCALE;
  if (source == RefClkSrc::INTERNAL && clockMin < INTERNAL_CLOCK_MIN_HZ) clockMin = INTERNAL_CLOCK_MIN_HZ;

The datasheet (p.7, digest 'Clocks') gives fINTCLK as MIN 35 / TYP 43.4 / MAX 55 MHz, and the digest states explicitly that this spread is a DEVICE property that the application cannot narrow by declaring a tighter tolerance. There is no ppm spec, no trim, and no way for firmware to know where in 35..55 MHz its part sits. So the only correct worst-case-slow internal reference is 35 MHz, unconditionally; max(declared_min, 35 MHz) is an upper bound on the slowest clock, which is exactly the wrong direction for an estimate documented as conservative (LDC1614.h:212 'Conservative chip-time estimate', LDC1614.h:417 'Pure conservative first-post-wake device-time', README.md:292). ReferenceClock::tolerancePpm defaults to 0 (Config.h:107), so the natural declaration {RefClkSrc::INTERNAL, 43400000U} yields clockMin = 43.4 MHz and every term (wake, settle, conversion, switch) is short by the full 43.4/35 = 1.24 factor. The same defect propagates into the wake term at LDC1614.cpp:1620-1623, where wakeClockMin correctly uses 35 MHz for an external reference (fINT clocks the 16384-cycle post-sleep delay per p.12) but uses the inflated clockMin for the internal case, so the one branch the comment claims to handle conservatively is the one that is wrong. Note the unit test at test_basic.cpp:2463-2470 cannot catch this: its base config declares 200000 ppm, which already clamps to 35 MHz, so the 'conservative' assertion compares 35 MHz against 35 MHz.

Also reported independently as: estimateFrameTiming is documented as conservative but under-estimates internal-clock frame time by up to 24% because it uses the declared nominal as the fREF lower bound

**Evidence**

```
src/LDC1614.cpp:1606-1625
  uint64_t clockMin =
      static_cast<uint64_t>(config.referenceClock.frequencyHz) *
      (PPM_SCALE - config.referenceClock.tolerancePpm) / PPM_SCALE;
  if (config.referenceClock.source == RefClkSrc::INTERNAL &&
      clockMin < INTERNAL_CLOCK_MIN_HZ) {
    clockMin = INTERNAL_CLOCK_MIN_HZ;
  }
  ...
  const uint64_t wakeClockMin =
      config.referenceClock.source == RefClkSrc::INTERNAL
          ? clockMin
          : static_cast<uint64_t>(INTERNAL_CLOCK_MIN_HZ);
  timing.wakeAndSettleUs =
      ceilDivide(WAKE_DELAY_CYCLES * MICROS_PER_SECOND, wakeClockMin);
```

**How it fails**

Profile shaped like the shipped HIL profile (docs/reports/.../one-hour-nonreset: single channel, rcount=1238, settleCount=10, frefDivider=2) but with the internal clock declared at its datasheet typical with the struct default tolerance: referenceClock = {RefClkSrc::INTERNAL, 43400000U, 0U}. estimateFrameTiming returns wakeAndSettleUs=386, conversionUs=913, sequentialFrameUs=1300. A part whose fINTCLK sits at the datasheet minimum 35 MHz actually needs wakeAndSettleUs=479, conversionUs=1133, sequentialFrameUs=1613 us - the estimate is 19.4% short (1613/1300 = 1.24x). docs/I2C_INTEGRATION.md:165-167 tells the owner to 'Use the timing estimate ... and application cadence' to schedule reads, so an owner that clears SLEEP and issues startAcquire at now + sequentialFrameUs (or sizes the immutable operation deadline from it) reads 313 us too early on an in-spec device: STATUS shows no DRDY/UNREADCONV, the SampleBatch comes back with the channel absent from freshChannels (or the job expires and completes TerminalOutcome::TIMED_OUT), and the application sees systematic stale/duplicate frames and spurious timeouts on hardware that is fully within datasheet spec. The under-estimate is invisible on the developer's own board if its oscillator happens to run near 43 MHz and only appears on parts at the cold/slow corner.

**Proposed fix** (as corrected by adversarial review)

Minimal, estimate-only change in src/LDC1614.cpp::estimateFrameTiming. Replace lines 1606-1612 with a source-selected lower bound rather than a one-sided clamp:

  // The internal oscillator's 35..55 MHz spread is a device property; an
  // application cannot narrow it by declaring a tighter tolerance.
  const uint64_t clockMin =
      config.referenceClock.source == RefClkSrc::INTERNAL
          ? static_cast<uint64_t>(INTERNAL_CLOCK_MIN_HZ)
          : static_cast<uint64_t>(config.referenceClock.frequencyHz) *
                (PPM_SCALE - config.referenceClock.tolerancePpm) / PPM_SCALE;

Keep the `clockMin == 0` guard (still reachable for EXTERNAL_CLOCK). Then collapse the ternary at 1620-1623 to the constant, keeping the existing comment, since 16384/fINT is clocked from the internal oscillator for BOTH sources (digest p.12):

  timing.wakeAndSettleUs = ceilDivide(WAKE_DELAY_CYCLES * MICROS_PER_SECOND,
                                      static_cast<uint64_t>(INTERNAL_CLOCK_MIN_HZ));

That is a net deletion of the special case - a patch here beats a refactor. Leave validateConfig alone in this change and open the internal-interval question (validateConfig's clockMin/clockMax clamp at 1354-1360 is anti-conservative for the fIN<fREF/4 and fREF-limit checks in exactly the same way) as a separate, semver-scoped item.

Docs: Config.h:101-103 currently says the internal interval is "conservatively clipped to its guaranteed datasheet range" - change to state that for RefClkSrc::INTERNAL the guaranteed 35..55 MHz interval IS the interval used by the timing estimate, and that `frequencyHz` serves only as the nominal for calculateSensorFrequencyHz.

Test: replace the vacuous test_basic.cpp:2463-2470 block with a declared-typical case and exact expectations (base makeConfig profile: rcount=0x0080, settleCount=0, frefDivider=2, single channel):

  Config internalTyp = makeConfig(fake);
  internalTyp.referenceClock = {RefClkSrc::INTERNAL, 43400000U, 0U};
  FrameTiming typ;
  TEST_ASSERT_TRUE(LDC1614::LDC1614::estimateFrameTiming(
      internalTyp, channelBit(Channel::CH0), typ).ok());
  TEST_ASSERT_EQUAL_UINT64(471, typ.wakeAndSettleUs);   // 469 wake + 2 settle at 35 MHz
  TEST_ASSERT_EQUAL_UINT64(118, typ.conversionUs);      // (128*16+4)/17.5 MHz
  TEST_ASSERT_EQUAL_UINT64(590, typ.sequentialFrameUs);

Before the fix this returns 380/95/476; after it, the values above. No other test changes.


---

### F16 &mdash; Session terminal results are relabelled FAILED, destroying CANCELLED/TIMED_OUT evidence

**Severity:** major &nbsp;|&nbsp; **Site:** `examples/common/Ldc1614Cli.cpp:3083` &nbsp;|&nbsp; **Independently reported by:** 1 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 3/3 upheld

**What is wrong**

`handleSessionOperationResult()` caches the true terminal record in `_lastResult` (line 3064) and then prints a *copy* whose `outcome` is unconditionally forced to `TerminalOutcome::FAILED` whenever `success` is false. `success` is false for every non-`SUCCESS` outcome, not just for the one case the override was written for (a `SUCCESS` acquisition that arrived without its `SampleBatch`, expressed by `terminalSuccess && expectsSample && !result.hasSampleBatch`). As a result a genuinely `CANCELLED` or `TIMED_OUT` core operation owned by a CLI session is printed as `outcome=FAILED`, while the `result` command printed one line later reports the real outcome from the untouched `_lastResult`. The same code exists verbatim in the IDF twin at examples/esp_idf/basic/main/Ldc1614IdfCli.cpp:3117. `test_cli.cpp` only covers cancelling a COMMAND-owned `init` (which goes through `handleOperationResult()` and prints the raw result correctly) and cancelling `scan` (which owns no core job), so this path is untested. AGENTS.md requires terminal results to retain their identity and `TerminalOutcome` to be a terminal classification independent of the detailed `Status`; this collapses three distinct classifications into one.

**Evidence**

```
examples/common/Ldc1614Cli.cpp:3064-3090
  _lastResult = result;            // keeps the true CANCELLED/TIMED_OUT outcome
  ...
  const bool success = terminalSuccess &&
                       (!expectsSample || result.hasSampleBatch);
  ...
  if (!success) {
    LDC1614::OperationResult effectiveResult = result;
    effectiveResult.outcome = LDC1614::TerminalOutcome::FAILED;   // <-- unconditional
    effectiveResult.status = effectiveStatus;
    printResult(effectiveResult);
    recordSessionFailure(effectiveStatus);
  }
```

**How it fails**

Operator runs `watch 1 100`, then types `cancel`. `stopActiveWork()` calls `_device.cancelJob()`, the driver queues an `OperationResult` with `outcome = CANCELLED`, `status.code = CANCELLED`. `drainResults()` -> `handleSessionOperationResult()` prints `Operation result: operation=7 kind=ACQUIRE outcome=FAILED ... code=13`, then `finishSession()` prints `CLI result: command=watch session=3 outcome=CANCELLED code=13`. A subsequent `result` command prints `outcome=CANCELLED` for the *same* operation. The transcript therefore contains two contradictory outcomes for one operation ID. The identical loss happens for a deadline expiry during `soak`/`stress`: a real `TIMED_OUT` acquisition is reported as `FAILED`, hiding the fact that the 2000 ms job deadline (not the device or the bus) ended the operation. Additionally `recordSessionFailure()` is invoked for an operator-initiated cancel, so the session summary prints `first_failure`/`last_failure` blocks while `fail=0`.

**Proposed fix** (as corrected by adversarial review)

The finder's shape is right; simplify both halves. Inside `if (!success)` the predicate `terminalSuccess` is already exactly 'SUCCESS acquisition without its batch' (success==false && terminalSuccess implies expectsSample && !hasSampleBatch), so no new predicate is needed. Replace examples/common/Ldc1614Cli.cpp:3081-3086 with:

  if (!success) {
    LDC1614::OperationResult effectiveResult = result;
    if (terminalSuccess) {
      // Only a SUCCESS acquisition missing its batch is reclassified.
      effectiveResult.outcome = LDC1614::TerminalOutcome::FAILED;
    }
    effectiveResult.status = effectiveStatus;
    printResult(effectiveResult);
    if (result.outcome != LDC1614::TerminalOutcome::CANCELLED) {
      recordSessionFailure(effectiveStatus);
    }
  }

For the failure-accounting half, guard on `result.outcome != CANCELLED` alone rather than the finder's `CANCELLED && _session.stopRequested`: it reuses the identical predicate already at line 3078 for recordLatency, and a driver-issued CANCELLED can only originate from cancelJob(), which this CLI calls only from stopActiveWork(). Adding the stopRequested conjunct would create a second, subtly different cancel predicate in the same function.

Apply the byte-identical edit to examples/esp_idf/basic/main/Ldc1614IdfCli.cpp:3115-3120 - tools/check_idf_example_contract.py requires the two CLIs stay in parity (and see F15: nothing today would catch it if you fixed only one).

Regression test in test/test_cli.cpp against FakeLdc1614Device (no new fixture support needed):

  void test_cli_session_cancel_and_deadline_keep_terminal_outcome() {
    CliFixture fixture; LDC1614::LDC1614 device;
    ldc1614_cli::Cli cli(device, fixturePlatform(fixture));
    TEST_ASSERT_TRUE(device.bind(fixtureConfig(&fixture)).ok());
    initializeAndWake(cli, device, fixture);
    fixture.clearOutput(); fixture.fake.clearIo();
    fixture.fake.injectConversion(0U, 0x01234567U);
    cli.processCommand("watch 1 100");
    ++fixture.nowMs; cli.service();            // schedules the session ACQUIRE
    ++fixture.nowMs; cli.service();            // consumes one transfer
    TEST_ASSERT_TRUE(device.jobProgress().active);
    cli.processCommand("cancel");
    serviceToIdle(cli, fixture);
    TEST_ASSERT_TRUE(contains(fixture, "kind=ACQUIRE outcome=CANCELLED"));
    TEST_ASSERT_FALSE(contains(fixture, "kind=ACQUIRE outcome=FAILED"));
    TEST_ASSERT_FALSE(contains(fixture, "first_failure"));
    // deadline arm: re-arm the session, then jump past JOB_DEADLINE_MS (2000)
    fixture.clearOutput(); fixture.fake.clearIo();
    cli.processCommand("watch 1 100");
    ++fixture.nowMs; cli.service();            // schedules the ACQUIRE
    fixture.nowMs += 2500; cli.service();      // poll() -> TIMED_OUT, no I2C
    TEST_ASSERT_TRUE(contains(fixture, "kind=ACQUIRE outcome=TIMED_OUT"));
  }

No allocation, no logging, no blocking; the driver core is untouched.


---

### F07 &mdash; Failed destructive read records no hardware-effect evidence (no read counterpart to INDETERMINATE_WRITE)

**Severity:** minor &nbsp;|&nbsp; **Site:** `src/LDC1614.cpp:803` &nbsp;|&nbsp; **Independently reported by:** 4 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 3/3 upheld

**What is wrong**

Every read failure path in the acquisition state machine returns straight to _finishJob without touching _progress.effects, so EffectFlag::READ_SIDE_EFFECTS is set only after a read that SUCCEEDED (src/LDC1614.cpp:805, :831, :850, :876). There is no INDETERMINATE_READ counterpart to EffectFlag::INDETERMINATE_WRITE, and no read-side analogue of writeFailureMayHaveCommitted() (src/LDC1614.cpp:140). But an I2C read is exactly as ambiguous as a write: a combined write-read that fails with I2C_TIMEOUT / I2C_BUS / I2C_ERROR may have already clocked the data phase, in which case the LDC has irreversibly performed its documented destructive effects - a STATUS read clears the sticky ERR_* bits and ERR_CHAN and de-asserts INTB; a DATAx_MSB read latches the LSB shadow and clears UNREADCONVx. This matters most for the very first transfer of a batch (READ_STATUS_BEFORE), because after that READ_SIDE_EFFECTS is already set by the STATUS-before read and the owner is at least warned. It is not a theoretical mapping either: the maintained ESP-IDF transport never produces Err::I2C_NACK_ADDR at all (examples/esp32/I2cMasterTransport.cpp:48-61 deliberately maps ESP_ERR_INVALID_STATE, IDF's return for a plain NACK, to Err::I2C_ERROR), so on the reference target no read failure can ever be proven harmless. AGENTS.md requires that 'Partial and ambiguous hardware effects remain visible through structured fault/effect provenance', and docs/I2C_INTEGRATION.md only narrows OperationResult::effects to 'whether at least one destructive read succeeded' - it never engages with the failed-but-possibly-completed read, even though the same document reasons carefully about exactly that case for writes.

Also reported independently as: A failed destructive read records no device-side effect, so a consumed STATUS latch is reported as EffectFlag::NONE; A failed destructive read records no effect evidence, so a consumed conversion is invisible in EffectFlags; A failed or timed-out acquisition discards the destructive STATUS snapshots it already consumed

**Evidence**

```
Status status = _readRegister(cmd::REG_STATUS, raw, transferTimeoutMs);
      --remainingTransfers;
      ++_progress.completedTransfers;
      if (!status.ok()) {
        return _finishJob(outcomeForFailure(status), status, nowMs);
      }
      _progress.effects |= effectFlag(EffectFlag::READ_SIDE_EFFECTS);
```

**How it fails**

Device is APPLIED_ACTIVE on an ESP32-S3 with an INTB-driven owner and ERROR_CONFIG routing watchdog/amplitude errors to STATUS+INTB. A watchdog error latches, INTB asserts, the owner calls startAcquire(). The first transfer, READ_STATUS_BEFORE, is issued; the LDC shifts out the STATUS bytes (clearing ERR_WD, ERR_CHAN and de-asserting INTB) and the transaction then fails at the controller with ESP_ERR_TIMEOUT, mapped to Err::I2C_TIMEOUT. _pollAcquire returns at line 803 with _progress.effects still 0. takeResult() hands the owner outcome=FAILED, effects=0, hasSampleBatch=false. Per the documented meaning of effects, the owner concludes no destructive read occurred, so it does not re-read STATUS and does not raise a sensor-fault event; INTB is now low-asserted-cleared so its interrupt-driven scheduler never fires again for that error. The latched watchdog fault is gone from the silicon and was never reported to the application. The identical failure on a write is reported correctly as INDETERMINATE_WRITE + DIRTY + ConfigFault.

**Proposed fix** (as corrected by adversarial review)

1) include/LDC1614/LDC1614.h:63-69 - append one enumerator (backward-compatible, MINOR bump per AGENTS.md):
   INDETERMINATE_READ = 1U << 3, ///< Failed destructive read may have completed on the device.

2) src/LDC1614.cpp:140 - rename the existing predicate instead of adding a twin, since the rule is identical for both directions:
   bool transferFailureMayHaveReachedDevice(const Status& status) { return status.code != Err::I2C_NACK_ADDR; }
   Update its three existing call sites (config write path ~757-760, writeRegister16 ~1244).

3) src/LDC1614.cpp - add a two-line private helper and use it at the THREE destructive read sites only (READ_STATUS_BEFORE ~803, READ_DATA_MSB ~829, READ_STATUS_AFTER ~874), leaving the READ_DATA_LSB site (~848) as a bare return because an LSB read has no documented device-side effect:
   Status LDC1614::_finishFailedRead(const Status& s, uint64_t nowMs) {
     if (transferFailureMayHaveReachedDevice(s)) {
       _progress.effects |= effectFlag(EffectFlag::INDETERMINATE_READ);
     }
     return _finishJob(outcomeForFailure(s), s, nowMs);
   }

4) examples/common/Ldc1614Cli.cpp:465-478 - effectNames() masks `effects & 0x07U`, so an INDETERMINATE_READ-only value would print "NONE" and the CLI would state the opposite of the truth. Replace the switch with a static const char* table indexed by `effects & 0x0FU` (16 fixed entries, no allocation, no new formatting) or extend the switch to 0x0F. Non-optional: without it the new flag is invisible on the HIL path.

5) docs/I2C_INTEGRATION.md - one paragraph next to the existing INDETERMINATE_WRITE text (lines 115-122): a failed STATUS or DATAx_MSB read may still have clocked its data phase, consuming the sticky STATUS/ERR_CHAN latch, de-asserting INTB, or clearing UNREADCONVx; owners must treat INDETERMINATE_READ as lost error/unread evidence and re-read STATUS on retry. Mirror the I2C_NACK_ADDR exception.

6) Regression test in test_basic.cpp, next to test_acquire_failure_at_every_phase_...:
   fake.clearIo(); fake.injectConversion(0, 0x00123456U);
   fake.failOnTransfer(1U, FORCED_READ);              // I2C_TIMEOUT on STATUS-before
   driver.startAcquire(channelBit(Channel::CH0), 601, DEADLINE_MS); pollToTerminal(driver);
   OperationResult r = takeResult(driver);
   TEST_ASSERT_FALSE(hasEffect(r.effects, EffectFlag::READ_SIDE_EFFECTS));
   TEST_ASSERT_TRUE(hasEffect(r.effects, EffectFlag::INDETERMINATE_READ));
   then repeat with Status::Error(Err::I2C_NACK_ADDR, "forced NACK", 0x2A) and assert effects == 0.


---

### F08 &mdash; Successful configuration jobs zero _progress.effects, erasing all evidence that 14-27 register writes (including RESET_DEV) reached the device

**Severity:** minor &nbsp;|&nbsp; **Site:** `src/LDC1614.cpp:746` &nbsp;|&nbsp; **Independently reported by:** 2 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 3/3 upheld

**What is wrong**

Both success exits of `_pollInitializeOrApply` clear `_progress.effects` before calling `_finishJob()`. `_finishJob()` copies that field into both `result.effects` and `result.finalProgress.effects`, so a successful INITIALIZE, APPLY_CONFIG, or RESET_AND_REAPPLY terminal record reports `effects == 0`, whose documented meaning in LDC1614.h:65 is `NONE = 0  ///< No known device-side effect.` That is false: every one of those jobs committed the complete register profile, and RESET_AND_REAPPLY additionally issued RESET_DEV. `EffectFlag::PARTIAL_WRITE` is documented at LDC1614.h:67 as "At least one prior write was confirmed" and is set on every intermediate successful write (lines 659, 768) — then thrown away at the finish line. The behaviour is also inconsistent with the acquisition path, which deliberately retains `READ_SIDE_EFFECTS` on success (line 876, pinned by test/test_basic.cpp:1724). Clearing `_configFault` on the same lines is correct and required by AGENTS.md; clearing `effects` is not required by anything and contradicts "Partial and ambiguous hardware effects remain visible through structured fault/effect provenance". No test pins `effects == 0` on success, so nothing depends on it.

Also reported independently as: A fully successful apply/initialize zeroes EffectFlag::PARTIAL_WRITE, contradicting the flag's documented meaning

**Evidence**

```
src/LDC1614.cpp:743-748 and the identical block at 774-780:
```cpp
    if (!write.valid) {
      _appliedState = AppliedConfigState::APPLIED_SLEEPING;
      _configFault = ConfigFault{};
      _progress.effects = 0;
      return _finishJob(TerminalOutcome::SUCCESS, Status::Ok(), nowMs);
    }
```
Compiled repro, actual output:
```
C: INITIALIZE outcome=1 effects=0x00 finalProgress.effects=0x00 transfers=16
C: RESET_AND_REAPPLY outcome=1 effects=0x00 (reset write invisible)
```
Contrast src/LDC1614.cpp:876-878 (acquire success keeps its effect flag) and test/test_basic.cpp:1724.
```

**How it fails**

A firmware maintenance scheduler runs `startResetAndReapply()` during a service window and journals `OperationResult::effects` to decide whether the chip's register state changed and therefore whether a post-maintenance readback verification pass (via `expectedConfigurationRegister()`) is needed. The result comes back SUCCESS with `effects == 0` = "No known device-side effect", so the scheduler skips verification and logs that nothing was written to the device — even though RESET_DEV plus 24 configuration registers were committed. The same erasure means a crash/log analysis after the window cannot tell a successful full replay apart from a job that performed no writes at all.

**Proposed fix** (as corrected by adversarial review)

Delete the two `_progress.effects = 0;` statements at src/LDC1614.cpp:746 and 778, keeping `_configFault = ConfigFault{};` on both lines (that is the field that actually carries trust state, and clearing it is required by AGENTS.md). A successful INITIALIZE/APPLY_CONFIG/RESET_AND_REAPPLY then publishes effects==PARTIAL_WRITE (0x02), matching the acquisition path's retention of READ_SIDE_EFFECTS on success and making result.finalProgress.effects honest too.

Do not add a WRITES_COMMITTED enumerator: it buys no decision that `outcome`+`appliedConfigState()` do not already answer, and it costs an API bump plus another effectNames() case. Instead reword the enumerator doc at include/LDC1614/LDC1614.h:67 to remove the ambiguity the name creates:
  PARTIAL_WRITE = 1U << 1, ///< At least one register write was confirmed to reach the device; combined with a non-SUCCESS outcome it means the profile is only partly committed.

Add one assertion to the existing initialize test in test_basic.cpp (near line 1016, on the success record rather than the failure records) - `TEST_ASSERT_TRUE(hasEffect(result.effects, EffectFlag::PARTIAL_WRITE));` - so the behaviour is pinned in the direction opposite to today's silent zero. No existing test needs changing.

Separately: re-record the HIL evidence under docs/reports/ (the stored serial captures show `effects=0x00 effects_names=NONE` for successful init/apply) and note the changed field value in the release notes, since it is an observable output change even though no API changes.


---

### F10 &mdash; First acquisition after wake() reports a fabricated UNDER_RANGE error because sleep-cleared DATA registers are decoded as a conversion result

**Severity:** minor &nbsp;|&nbsp; **Site:** `src/LDC1614.cpp:938` &nbsp;|&nbsp; **Independently reported by:** 2 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 2/3 upheld

**What is wrong**

`wake()` sets `APPLIED_ACTIVE` the instant the CONFIG write is ACKed, and `startAcquire()` is admitted immediately. But the datasheet (digest p.12) requires 16384/fINT plus settling plus one conversion before the first valid sample, and (p.47) "If the device is put into Sleep mode or Shutdown mode, all DATAx_MSB and DATAx_LSB registers are cleared of conversion data." So in the post-wake window `DATAx_MSB`/`DATAx_LSB` read back 0x0000/0x0000. `decodeChannelSample()` applies the datasheet endpoint rule `rawCount28 == 0 => UNDER_RANGE` (line 1543-1546) to that never-populated register, and `_buildAcquisition()` then puts the channel into `batch.errorChannels` because `UNDER_RANGE` is part of `errorFlags` (lines 978-984). The driver already holds the evidence that distinguishes the two cases — `STATUS.UNREADCONVx` is clear, so it has just classified the channel `STALE` at line 938 — but does not use it to suppress the endpoint heuristic. The result is that a channel which has simply not converted yet is reported as a silicon under-range fault, i.e. `errorChannels` carries a cause that does not exist. README.md:232 promises the opposite: "`errorChannels` and per-channel quality retain the cause." This is not an edge case: every application hits it on its first acquisition after its first `wake()`, and a duty-cycled application hits it on every single wake.

Also reported independently as: rawCount28 == 0 is reported as UNDER_RANGE on channels that never produced a conversion, including after every sleep/wake

**Evidence**

```
src/LDC1614.cpp:934-939 (the STALE evidence the decoder ignores):
```cpp
    ChannelSample sample = _scratchSamples[index];
    if ((batch.freshChannels.bits & bit) != 0U) {
      sample.quality |= sampleQualityFlag(SampleQualityFlag::FRESH);
    } else {
      sample.quality |= sampleQualityFlag(SampleQualityFlag::STALE);
    }
```
src/LDC1614.cpp:1543-1546 (endpoint rule applied unconditionally):
```cpp
  if ((msb & cmd::MASK_DATA_ERR_UR) != 0U ||
      sample.rawCount28 == 0U) {
    sample.quality |= sampleQualityFlag(SampleQualityFlag::UNDER_RANGE);
  }
```
src/LDC1614.cpp:978-984 (UNDER_RANGE promotes the channel into errorChannels).
Compiled repro: bind, startInitialize, wake, startAcquire immediately:
```
D: acquire outcome=1 selected=0x01 valid=0x00 fresh=0x00 error=0x01 quality0=0x0006 raw=0
```
(0x0006 = STALE | UNDER_RANGE; the channel is in errorChannels with no device error bit set anywhere.)
Consumer impact in-tree: examples/common/Ldc1614Cli.cpp:2093 accumulates `stats.errors` from `batch.errorChannels`, and :3155 gates on `batch.errorChannels.empty()`.
```

**How it fails**

A battery-powered product duty-cycles the sensor: `sleep()` between measurements, `wake()`, wait, `startAcquire()`. On the first acquisition of each cycle the DATA registers are still the zeros that sleep left behind, so the batch comes back with `validChannels == 0` (correct) *and* `errorChannels == selectedChannels` carrying UNDER_RANGE (fabricated). The product's health monitor, which escalates on repeated under-range as "sensor coil open or detuned", raises a permanent sensor fault after N wake cycles even though the coil is fine and every steady-state sample is good. A field engineer inspecting `SampleBatch` sees a real datasheet-flavoured error code with no way to tell it apart from a genuine under-range.

**Proposed fix** (as corrected by adversarial review)

The proposed fix is the right shape but should be folded into the existing else-branch rather than added as a second `if` after it. In _buildAcquisition (src/LDC1614.cpp:935-939) replace the STALE arm with:

```cpp
    } else {
      sample.quality |= sampleQualityFlag(SampleQualityFlag::STALE);
      // The 0x0000000/0x0FFFFFFF endpoint rule describes a conversion result.
      // A channel with no unread conversion may be holding registers that
      // sleep cleared (datasheet p.47) or data already consumed, so keep only
      // range flags the silicon actually asserted.
      if ((sample.rawDataMsb &
           (cmd::MASK_DATA_ERR_UR | cmd::MASK_DATA_ERR_OR)) == 0U) {
        sample.quality &= static_cast<SampleQualityFlags>(
            ~(sampleQualityFlag(SampleQualityFlag::UNDER_RANGE) |
              sampleQualityFlag(SampleQualityFlag::OVER_RANGE)));
      }
    }
```

Placement matters and is correct here: it runs before the `statusApplies` block, so a STATUS-reported ERR_UR/ERR_OR on the STATUS error channel still tags the sample. Leave decodeChannelSample() untouched - it is a public pure decoder whose endpoint rule is right for a real conversion pair and is directly unit-tested (test_basic.cpp:1909-1913). No allocation, no logging, no extra transfer: contract-clean.

Docs: amend include/LDC1614/LDC1614.h:125-126 to say the endpoint classification applies to fresh samples and silicon flags always apply, and the README sentence at ~line 229 ('Under-range zero and over-range 0x0FFFFFFF are classified even when corresponding device reporting bits are disabled') to add 'for channels that report an unread conversion'.

No existing test changes behaviour. test_basic.cpp:1930-1936 drives every endpoint case through fake.injectConversion(), which sets unreadMask, so those samples are FRESH and keep their flags and their errorChannels assertion at line 1954. The stale test at test_basic.cpp:1884-1888 uses raw 0x00555555, which never hit the endpoint rule.

Regression test to add to test/test_basic.cpp (fixture supports it as-is):
```cpp
void test_first_acquire_after_wake_is_stale_without_fabricated_under_range() {
  FakeLdc1614Device fake; LDC1614::LDC1614 driver;
  Config config = makeConfig(fake);
  initializeAndWake(driver, fake, config);      // no injectConversion
  OperationResult r = acquire(driver, channelBit(Channel::CH0), 14100U);
  const ChannelSample& s = r.sampleBatch.channel[0];
  TEST_ASSERT_EQUAL_HEX32(0U, s.rawCount28);
  TEST_ASSERT_TRUE(hasSampleQuality(s.quality, SampleQualityFlag::STALE));
  TEST_ASSERT_FALSE(hasSampleQuality(s.quality, SampleQualityFlag::UNDER_RANGE));
  TEST_ASSERT_FALSE(r.sampleBatch.validChannels.contains(Channel::CH0));
  TEST_ASSERT_FALSE(r.sampleBatch.errorChannels.contains(Channel::CH0));
}
```
For a true sleep/wake variant note that FakeLdc1614Device::applyWrite (test/support/FakeLdc1614Device.h:311-318) clears unreadMask and sticky status on the sleep CONFIG write but does NOT clear liveMsb/liveLsb/shadow*, so it under-models datasheet p.47; adding those four clears to that branch is a worthwhile one-line fixture fidelity fix and lets the regression test exercise the duty-cycle path directly.


---

### F12 &mdash; Every configuration replay writes CONFIG twice with a byte-identical value, wasting one I2C transaction per initialize/apply/reset

**Severity:** minor &nbsp;|&nbsp; **Site:** `src/LDC1614.cpp:312` &nbsp;|&nbsp; **Independently reported by:** 2 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 2/3 upheld

**What is wrong**

configWriteAt() emits REG_CONFIG at step 0 (`write.value = buildConfigRegister(config, true); write.phase = JobPhase::FORCE_SLEEP;`, lines 253-257) and again as the final step (`write.value = buildConfigRegister(config, true); write.phase = JobPhase::WRITE_FINAL_CONFIG;`, lines 311-315). Both call buildConfigRegister with `sleeping = true` and identical config, so the two writes are byte-identical by construction, and no intervening step writes CONFIG (steps in between cover RCOUNT/SETTLECOUNT/CLOCK_DIVIDERS/OFFSET/DRIVE_CURRENT/ERROR_CONFIG/MUX_CONFIG only). The device is asleep for the whole sequence, so the sleep-mode side effect (clearing DATA and error state) has nothing left to clear the second time. The datasheet rule that CONFIG must be written last (p.51 Tables 47-48: "This register write must occur last because device configuration is not permitted while the LDC is in active mode") refers to the CONFIG write that clears SLEEP_MODE_EN; in this driver that write is issued by wake(), not by the replay, so the trailing sleep-CONFIG write serves no documented purpose.

Also reported independently as: Final CONFIG write of every replay is a byte-identical no-op that can still fail the whole job

**Evidence**

```
The tests state the redundancy explicitly. test/test_basic.cpp:766-775 lists LDC1612_WRITES beginning `{cmd::REG_CONFIG, 0x3C81}` and ending `{cmd::REG_CONFIG, 0x3C81}`; test/test_basic.cpp:797-813 lists LDC1614_WRITES beginning and ending with `{cmd::REG_CONFIG, 0x2281}`. configurationTransferCount() at src/LDC1614.cpp:233 returns `1U + channelCount * 5U + 3U`, where the trailing 3 is ERROR_CONFIG, MUX_CONFIG, and this duplicate CONFIG.
```

**How it fails**

Every startInitialize/startApplyConfig/startResetAndReapply spends one extra bounded transport callback (a 3-byte write plus address phase, roughly 90 us of bus occupancy at 400 kHz plus the owner's per-transfer lock/queue overhead) writing a value the device already holds. On a shared bus with poll(now, 1) that is one extra whole service pass per replay, and the owner's deadline and transfer budget must be sized for the inflated documented maxima (docs/I2C_INTEGRATION.md: 14/24 apply, 16/26 initialize, 17/27 reset) even though 13/23, 15/25 and 16/26 would suffice.

**Proposed fix** (as corrected by adversarial review)

The proposed fix is the right one and I would make it as stated, with two additions the proposal misses.

Code: delete the trailing `else` arm of configWriteAt (src/LDC1614.cpp:310-315) so globalStep 1 (MUX_CONFIG) is the last step, and change configurationTransferCount (src/LDC1614.cpp:232-234) to `1U + channelCount * 5U + 2U`. Keep JobPhase::WRITE_FINAL_CONFIG - sleep() (src/LDC1614.cpp:1151), wake() (:1181) and configurationPhaseForRegister (:164) still use it for fault provenance on raw CONFIG writes. No other src change is needed; the replay loop already terminates on `!write.valid`.

Docs/tests, all mechanical: the three maxima in include/LDC1614/LDC1614.h (startInitialize 16/26 -> 15/25 in the comment above line 275, startApplyConfig 14/24 -> 13/23 at :278, startResetAndReapply 17/27 -> 16/26 at :286); the same numbers in docs/I2C_INTEGRATION.md:101-102; test/test_basic.cpp initTransfers/applyTransfers/resetTransfers, configStage() (line 185 must now return {JobPhase::WRITE_MUX_CONFIG, cmd::REG_MUX_CONFIG, Channel::NONE} for the final step), the two ExpectedWrite arrays (drop the trailing {cmd::REG_CONFIG, 0x3C81} at :775 and {cmd::REG_CONFIG, 0x2281} at :812) and the 14U/24U counts passed to assertConfigWrites.

Two things the proposal omits and that decide whether the change is worth scheduling now: (a) it shrinks a documented public budget, so it is a MINOR release under the AGENTS.md versioning rules, not a silent patch - owners who sized deadlines/transfer budgets from the header get a smaller number, which is safe, but the CHANGELOG must say so; (b) the archived HIL evidence in docs/reports/20260804/*.json and *.serial.txt records `phase=WRITE_FINAL_CONFIG ... transfers=26 maximum=26` and `transfers=24 maximum=24` for init/apply, so the change invalidates that evidence and should be batched with a HIL re-run rather than landed on its own. If the maintainers do not want to re-run HIL for a 90 us saving, the acceptable alternative is to leave the code alone and add one comment at src/LDC1614.cpp:310 recording that the write is knowingly redundant and kept only to mirror the datasheet's write ordering - but do not leave it undocumented as it is today.


---

### F13 &mdash; Write-failure effect/fault bookkeeping is copy-pasted across five sites, one of which re-implements _recordConfigFault() inline

**Severity:** minor &nbsp;|&nbsp; **Site:** `src/LDC1614.cpp:646` &nbsp;|&nbsp; **Independently reported by:** 1 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 2/3 upheld

**What is wrong**

The same 10-line failure-bookkeeping shape (set INDETERMINATE_WRITE when writeFailureMayHaveCommitted, mark DIRTY, record the fault, finish the job) and the same 5-line success shape (set PARTIAL_WRITE, latch _lastWritePhase/_lastWriteRegister/_lastWriteChannel, set APPLYING) appear twice in _pollInitializeOrApply (RESET_DEV write at 642-667, config write at 753-772), and near-copies appear in sleep() (1146-1156) and wake() (1176-1186). Worse, writeRegister16() at 1255-1267 assigns all seven _configFault members by hand instead of calling the private helper _recordConfigFault() that exists for exactly this purpose at 1068-1078. The inline version is behaviorally identical: it sets `_configFault.job = JobKind::NONE`, and _recordConfigFault sets `_configFault.job = _progress.kind`, which is guaranteed to be JobKind::NONE here because writeRegister16 returns BUSY when _progress.active and _clearActiveJob() resets _progress to a default JobProgress{}.

**Evidence**

```
src/LDC1614.cpp:646-658 and 756-767 are the same statements with (JobPhase::SOFTWARE_RESET, REG_RESET_DEV, Channel::NONE) substituted for (write.phase, write.reg, write.channel). src/LDC1614.cpp:1256-1266 duplicates the body of `void LDC1614::_recordConfigFault(...)` at src/LDC1614.cpp:1071-1078.
```

**How it fails**

A correction to the effect model -- for example deciding that Err::I2C_NACK_DATA must also be treated as a confirmed non-mutation, or that a failed write must clear _lastWriteRegister -- is applied to the config-write path at 756 but not to the RESET_DEV path at 646. startResetAndReapply() then reports different effects/ConfigFault provenance than startApplyConfig() for the identical transport failure, and the owner's recovery decision (which is driven entirely by EffectFlags and ConfigFault) diverges by job kind. The inline _configFault block at 1255 has the same exposure: adding a field to ConfigFault updates _recordConfigFault and every job path, but silently leaves that field stale after every diagnostic writeRegister16().

**Proposed fix** (as corrected by adversarial review)

Two independent, strictly behaviour-preserving edits; do not touch the ConfigWrite table.

1. src/LDC1614.cpp:1255-1267 -- reuse the existing helper:
```cpp
  if (mutationPossible) {
    _recordConfigFault(
        status.ok() ? Status::Error(Err::CONFIG_DIRTY,
                                    "Diagnostic write invalidated applied state")
                    : status,
        configurationPhaseForRegister(reg), reg,
        configurationChannelForRegister(reg), effects);
  }
```
Identical result because _progress.kind is provably JobKind::NONE behind the BUSY guard (see reasoning). Net -6 lines, one fewer place to update when ConfigFault gains a field.

2. Add one small private method for the two in-job write sites only (declare next to _recordConfigFault in include/LDC1614/LDC1614.h):
```cpp
bool LDC1614::_noteJobWrite(const Status& status, JobPhase phase,
                            uint8_t reg, Channel channel) {
  if (!status.ok()) {
    if (writeFailureMayHaveCommitted(status)) {
      _progress.effects |= effectFlag(EffectFlag::INDETERMINATE_WRITE);
    }
    if (writeFailureMayHaveCommitted(status) ||
        hasEffect(_progress.effects, EffectFlag::PARTIAL_WRITE)) {
      _appliedState = AppliedConfigState::DIRTY;
      _recordConfigFault(status, phase, reg, channel, _progress.effects);
    }
    return false;
  }
  _progress.effects |= effectFlag(EffectFlag::PARTIAL_WRITE);
  _lastWritePhase = phase;
  _lastWriteRegister = reg;
  _lastWriteChannel = channel;
  _appliedState = AppliedConfigState::APPLYING;
  return true;
}
```
(keep the original `writeFailureMayHaveCommitted(status) || hasEffect(PARTIAL_WRITE)` condition rather than testing INDETERMINATE_WRITE, so the rewrite cannot depend on whether that flag could ever be pre-set). Each call site collapses to:
```cpp
      if (!_noteJobWrite(status, JobPhase::SOFTWARE_RESET,
                         cmd::REG_RESET_DEV, Channel::NONE)) {
        return _finishJob(outcomeForFailure(status), status, nowMs);
      }
```
and the same with (write.phase, write.reg, write.channel). Net ~-20 lines, the effect model lives in one place, the transport call/budget/termination stay visible at the call site. Leave sleep()/wake() alone -- their rule is deliberately different, and merging them would be the kind of speculative abstraction AGENTS.md:62 forbids. No behaviour change, so all 48 tests keep passing.


---

### F14 &mdash; MANUFACTURER_ID and DEVICE_ID verification are two 31-line copy-pasted blocks

**Severity:** minor &nbsp;|&nbsp; **Site:** `src/LDC1614.cpp:673` &nbsp;|&nbsp; **Independently reported by:** 1 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 2/3 upheld

**What is wrong**

_pollInitializeOrApply contains the manufacturer block (lines 673-703) and the device block (lines 708-738), which are identical statement-for-statement except for four substitutions: REG_MANUFACTURER_ID/REG_DEVICE_ID, MANUFACTURER_ID_VALUE/DEVICE_ID_VALUE, JobPhase::VERIFY_MANUFACTURER/VERIFY_DEVICE, the diagnostic string, and the next phase set on success. Both also repeat the identical `_appliedState = resetFirst ? DIRTY : UNKNOWN;` transport-failure rule and the identical `_appliedState = UNKNOWN;` mismatch rule.

**Evidence**

```
src/LDC1614.cpp:682-698 versus 717-733 -- the same five statements in the same order, including the subtle asymmetry that a transport failure yields DIRTY under RESET_AND_REAPPLY but UNKNOWN otherwise, while an identity mismatch always yields UNKNOWN.
```

**How it fails**

A change to identity handling -- for example recording the observed value in ConfigFault::cause detail on a transport failure, or tightening the applied-state rule so a post-reset identity read failure also clears provenance -- is applied to the manufacturer block and missed in the device block. The driver then reports different applied state and different fault provenance depending on which of the two identity registers failed, and test/test_basic.cpp's per-transfer failure sweep would have to be read line by line to notice, because both paths currently pass the same assertions.

**Proposed fix** (as corrected by adversarial review)

Merge the two blocks in src/LDC1614.cpp:670-738 into one table-driven step, no header change. Note that identity steps exist exactly when applyOffset != 0 (applyOffset is 2 for INITIALIZE and 3 for resetFirst, per 629-635), and that they occupy _jobStep in [applyOffset-2, applyOffset):
```cpp
    struct IdentityStep {
      uint8_t reg;
      uint16_t expected;
      JobPhase phase;
      const char* mismatch;
      JobPhase nextPhase;
      uint8_t nextRegister;
    };
    static constexpr IdentityStep kIdentitySteps[2] = {
        {cmd::REG_MANUFACTURER_ID, cmd::MANUFACTURER_ID_VALUE,
         JobPhase::VERIFY_MANUFACTURER, "Wrong MANUFACTURER_ID",
         JobPhase::VERIFY_DEVICE, cmd::REG_DEVICE_ID},
        {cmd::REG_DEVICE_ID, cmd::DEVICE_ID_VALUE, JobPhase::VERIFY_DEVICE,
         "Wrong DEVICE_ID", JobPhase::FORCE_SLEEP, cmd::REG_CONFIG},
    };
    if (applyOffset != 0U && _jobStep < applyOffset) {
      const IdentityStep& step =
          kIdentitySteps[_jobStep - static_cast<uint8_t>(applyOffset - 2U)];
      _progress.phase = step.phase;
      _progress.registerAddress = step.reg;
      _progress.channel = Channel::NONE;
      uint16_t value = 0;
      Status status = _readRegister(step.reg, value, transferTimeoutMs);
      --remainingTransfers;
      ++_progress.completedTransfers;
      if (!status.ok()) {
        _appliedState = resetFirst ? AppliedConfigState::DIRTY
                                   : AppliedConfigState::UNKNOWN;
        _recordConfigFault(status, step.phase, step.reg, Channel::NONE,
                           _progress.effects);
        return _finishJob(outcomeForFailure(status), status, nowMs);
      }
      if (value != step.expected) {
        status = Status::Error(Err::DEVICE_NOT_FOUND, step.mismatch, value);
        _appliedState = AppliedConfigState::UNKNOWN;
        _recordConfigFault(status, step.phase, step.reg, Channel::NONE,
                           _progress.effects);
        return _finishJob(TerminalOutcome::FAILED, status, nowMs);
      }
      ++_jobStep;
      _progress.phase = step.nextPhase;
      _progress.registerAddress = step.nextRegister;
      continue;
    }
```
This deletes the `manufacturerStep`/`deviceStep` predicates and both 31-line blocks (~-38 lines, +~14), puts the DIRTY-vs-UNKNOWN rule and the mismatch rule in exactly one place, adds no allocation and no header surface. When resetFirst is set the earlier RESET_DEV block already consumed _jobStep==0 and `continue`d, so _jobStep >= applyOffset-2 always holds here. Behaviour-preserving: all 48 tests, including test_basic.cpp's per-transfer failure sweep, keep passing unchanged.


---

### F18 &mdash; I2cWriteReadFn's contract never states the mandatory repeated START; a STOP-separated implementation silently corrupts transactions

**Severity:** minor &nbsp;|&nbsp; **Site:** `include/LDC1614/Config.h:19` &nbsp;|&nbsp; **Independently reported by:** 2 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 3/3 upheld

**What is wrong**

The register-read callback the application must supply is documented only as "One bounded combined I2C write/read attempt. The callback must return within timeoutMs." (repeated at Config.h:195 as "Non-owning combined write/read callback"). Neither the header, README.md, nor docs/I2C_INTEGRATION.md ever states that the pointer write and the data read must be joined by a repeated START with no intervening STOP - docs/I2C_INTEGRATION.md only says "Each invocation is one complete physical transaction and must return within the supplied timeout", which a write-STOP-read implementation also satisfies on its face.

The datasheet is explicit and unusually strict about this. src/LDC1614.cpp:1023-1028 issues every register read through this callback (`&reg, 1` out, 2 bytes in), so a STOP-separated implementation affects identity verification, STATUS, and every DATA read, and the driver cannot detect it.

Also reported independently as: The binding transport contract never mandates the repeated-START combined read the datasheet requires

**Evidence**

```
Header (include/LDC1614/Config.h:19-23):
  /// One bounded combined I2C write/read attempt. The callback must return within timeoutMs.
  using I2cWriteReadFn = Status (*)(uint8_t address,
                                    const uint8_t* txData, size_t txLength,
                                    uint8_t* rxData, size_t rxLength,
                                    uint32_t timeoutMs, void* user);

Datasheet (.audit/LDC1614_datasheet.txt, section 7.5.1 and 7.5.2):
  "The device registers are 16 bits wide, and so a repeated start is used to access the 2nd byte of data."
  "it is not suitable for use in an I2C system which supports early termination of transactions. A STOP
   condition or other early termination occurring before the normal end of a transaction (ACK) is not
   supported and may corrupt that transaction and/or the following transaction."

Sole use site (src/LDC1614.cpp:1023-1028):
    Status status = _config.i2cWriteRead(
        static_cast<uint8_t>(_config.i2cAddress), &reg, 1, rx, sizeof(rx),
        timeoutMs, _config.i2cUser);
```

**How it fails**

An Arduino integrator implements the callback with the default Wire idiom - `Wire.beginTransmission(addr); Wire.write(txData, txLength); Wire.endTransmission(); Wire.requestFrom(addr, rxLength);` - because `endTransmission()` with no argument emits a STOP and nothing in the injected-callback contract says that is forbidden. Every read is then a STOP-separated pair. Per the datasheet this may corrupt that transaction and/or the following one, so the failure is intermittent: identity reads mostly succeed, then startInitialize fails at VERIFY_DEVICE with a garbage value and DEVICE_NOT_FOUND, or an acquisition returns a DATAx_LSB word belonging to the previous transaction, producing a coherent-looking but wrong rawCount28 that passes every quality check in decodeChannelSample. The driver reports OK and publishes the bad sample.

**Proposed fix** (as corrected by adversarial review)

Two comment/doc edits, no code change, no test impact.

1. include/LDC1614/Config.h:19, keeping the core header framework-neutral per AGENTS.md (no Wire/esp_err_t names in the library contract):
   /// One bounded combined-format I2C write/read attempt: the register-pointer
   /// write and the 2-byte read are joined by a repeated START with no STOP
   /// between them (datasheet 7.5.1, Figure 12). The callback must return
   /// within timeoutMs.

2. docs/I2C_INTEGRATION.md, extend the existing "Transport boundary" bullet (line 9-11) - that document already names Wire and esp_err_t, so framework primitives belong here, not in the header: "...Each invocation is one complete physical transaction and must return within the supplied timeout. `i2cWriteRead` must issue a combined-format transfer - pointer write, repeated START, 2-byte read - not a write terminated by STOP followed by a separate read (`i2c_master_transmit_receive()`; on Arduino `Wire.endTransmission(false)` before `requestFrom`). TI documents only the repeated-START read sequence for this part."

Do not repeat the 7.5.2 early-termination sentence as the justification; it is about a STOP before a transaction's ACK and does not cleanly cover the write-STOP-read idiom. Cite 7.5.1/Figure 12 instead. Skipped from the finder's proposal: no change is needed to the i2cWrite comment (a register write is a single uninterrupted transaction already).


---

### F19 &mdash; ErrorReporting::dataReady is documented as INTB-only, but DRDY_2INT also gates STATUS.DRDY

**Severity:** minor &nbsp;|&nbsp; **Site:** `include/LDC1614/Config.h:153` &nbsp;|&nbsp; **Independently reported by:** 1 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 3/3 upheld

**What is wrong**

Config.h documents the field as "Route data-ready to INTB." The datasheet (register 0x19, DRDY_2INT) defines b0 as "Do not report Data Ready Flag by asserting INTB pin *and STATUS register*", i.e. clearing the bit also permanently clears STATUS.DRDY - which is precisely why the repo's own CommandTable.h:154 annotates the mask as "Data ready -> INTB + STATUS" and why every neighbouring ErrorReporting field is worded "Route ... to STATUS/INTB". `dataReady` is the one field whose Doxygen omits the STATUS half.

The driver relies on the correct semantics internally (readDataReady() refuses with INVALID_CONFIG when the field is false, src/LDC1614.cpp:1099-1102, and the baseline audit confirms that guard is right), but nothing guards the other STATUS.DRDY consumers: DeviceStatus::dataReady from readDeviceStatus() (src/LDC1614.cpp:1075) and decodeDeviceStatus() (src/LDC1614.cpp:1470), and SampleBatch::statusBefore/statusAfter.dataReady published by every acquisition.

**Evidence**

```
Config.h (include/LDC1614/Config.h:148-153):
    bool statusAmplitudeLow = false;   ///< Route low-amplitude error to STATUS/INTB.
    bool statusZeroCount = false;      ///< Route zero-count error to STATUS/INTB.
    bool dataReady = false;            ///< Route data-ready to INTB.

Same bit, correctly annotated in include/LDC1614/CommandTable.h:154:
  static constexpr uint16_t MASK_ERRCFG_DRDY_2INT   = 0x0001;  ///< Data ready -> INTB + STATUS

Datasheet (.audit/digest.md, ERROR_CONFIG section):
  DRDY_2INT b0 = "Do not report Data Ready Flag by asserting INTB pin **and STATUS register**".
  So STATUS.DRDY IS gated by ERROR_CONFIG.DRDY_2INT.
```

**How it fails**

An integrator wires no INTB line, sets `cfg.intbDisabled = true`, and reads Config.h:153 - "Route data-ready to INTB" - concluding the bit is irrelevant to a polled design, so leaves `errorReporting.dataReady = false` (the default) rather than using ErrorReporting::all(). encodeErrorReporting() therefore writes DRDY_2INT = 0. Their poll loop calls readDeviceStatus() and waits on `snapshot.dataReady`, which the silicon now never sets; the loop never fires and the product reports "no conversions" on perfectly healthy hardware. The same silently-dead field appears in every SampleBatch::statusBefore.dataReady. readDataReady() would have caught the misconfiguration with INVALID_CONFIG, but the doc gives the integrator no reason to prefer it over readDeviceStatus().

**Proposed fix** (as corrected by adversarial review)

Comment-only, two lines, both stating the consequence rather than the register name (the register name is already in CommandTable.h):

1. include/LDC1614/Config.h:153 ->
   bool dataReady = false;          ///< Route data-ready to STATUS/INTB; false leaves STATUS.DRDY permanently clear.

2. include/LDC1614/LDC1614.h:108 (DeviceStatus::dataReady, currently "STATUS.DRDY snapshot.") -> "STATUS.DRDY snapshot; always false unless ErrorReporting::dataReady is enabled." This is the half that actually closes the finder's failure scenario, since the misled integrator consumes DeviceStatus/SampleBatch, not the Config field.

Prefer this to the finder's longer single-line text: it matches the sibling wording exactly ("Route ... to STATUS/INTB") and puts the actionable warning on the field the victim reads. No code, no test, no checker change - tools/ scripts do not inspect header comments.


---

### F20 &mdash; SampleQualityFlag::WATCHDOG is documented as DATA-only but is also raised from STATUS.ERR_WD

**Severity:** minor &nbsp;|&nbsp; **Site:** `include/LDC1614/LDC1614.h:127` &nbsp;|&nbsp; **Independently reported by:** 1 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 3/3 upheld

**What is wrong**

The flag is documented as "DATA watchdog flag was set." _buildAcquisition raises it from two independent sources: decodeChannelSample() from DATAx_MSB bit 13 (src/LDC1614.cpp:1499-1501), and the STATUS-before snapshot's ERR_WD bit attributed via ERR_CHAN (src/LDC1614.cpp:951-953). The two neighbouring flags that share this dual sourcing document it correctly - UNDER_RANGE/OVER_RANGE say "Raw endpoint or silicon flag" and AMPLITUDE_SUSPECT says "DATA/STATUS amplitude fault" - so WATCHDOG is the only inconsistent entry. The two sources are genuinely independent because ERROR_CONFIG routes them with separate bits (WD_ERR2OUT -> DATAx_MSB, WD_ERR2INT -> STATUS).

**Evidence**

```
Header (include/LDC1614/LDC1614.h:124-129):
    UNDER_RANGE = 1U << 2,        ///< Raw endpoint or silicon flag indicates under-range.
    OVER_RANGE = 1U << 3,         ///< Raw endpoint or silicon flag indicates over-range.
    WATCHDOG = 1U << 4,           ///< DATA watchdog flag was set.
    AMPLITUDE_SUSPECT = 1U << 5,  ///< DATA/STATUS amplitude fault; sample is invalid.

STATUS-sourced assignment (src/LDC1614.cpp:941-953):
    const bool statusApplies =
        _scratchStatusBefore.hasError() &&
        _scratchStatusBefore.errorChannel == static_cast<Channel>(index);
    if (statusApplies) {
      ...
      if (_scratchStatusBefore.errorWatchdog) {
        sample.quality |= sampleQualityFlag(SampleQualityFlag::WATCHDOG);
      }
```

**How it fails**

An integrator uses ErrorReporting with statusWatchdog = true but dataWatchdog = false (a common choice: keep the 12 data bits clean, route faults to STATUS/INTB only). ERROR_CONFIG then has WD_ERR2INT=1, WD_ERR2OUT=0. A watchdog timeout fires on CH1: DATA1_MSB bit 13 stays clear, STATUS reports ERR_WD with ERR_CHAN=1. The batch's quality for CH1 carries WATCHDOG and CH1 is excluded from validChannels. The application's fault classifier, written from the header comment, does `if (hasSampleQuality(q, SampleQualityFlag::WATCHDOG)) { assert(sample.rawDataMsb & cmd::MASK_DATA_ERR_WD); reportRawWatchdogWord(sample.rawDataMsb); }` - the assert trips in a debug build, and in release the diagnostic reports a raw word with no watchdog bit, so the operator sees a channel dropped from validChannels with no visible cause.

**Proposed fix** (as corrected by adversarial review)

Accept the proposed fix; it is already the simplest correct one. include/LDC1614/LDC1614.h:127 ->
   WATCHDOG = 1U << 4,           ///< DATA or STATUS watchdog fault attributed to this channel.

("attributed to this channel" is the load-bearing phrase: the STATUS path reaches the sample only via ERR_CHAN attribution at src/LDC1614.cpp:941-943.) No wider refactor is warranted - the code is correct and the two sources are deliberate; only the comment is behind it. Fold this into the same commit as F19, since both are one-line Doxygen corrections in the same public-API surface and both are PATCH-level per AGENTS.md versioning.


---

### F23 &mdash; invalidateAppliedState() silently cancels the active job and consumes a result-FIFO slot; its Doxygen documents neither

**Severity:** minor &nbsp;|&nbsp; **Site:** `src/LDC1614.cpp:613` &nbsp;|&nbsp; **Independently reported by:** 2 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 3/3 upheld

**What is wrong**

include/LDC1614/LDC1614.h:332-335 documents invalidateAppliedState() only as "Mark hardware configuration unknown after removal, reset, brownout, or owner recovery. Performs zero I2C and preserves the supplied full status as provenance." It returns void.

The implementation additionally calls cancelJob() when a job is active, which routes through _finishJob(TerminalOutcome::CANCELLED, ...) at src/LDC1614.cpp:588 and enqueues a terminal OperationResult. That has two owner-visible consequences the header never states: it occupies one of the two RESULT_CAPACITY slots, and it makes the cancelled job's OperationId "in use" (src/LDC1614.cpp:1005 _operationIdInUse scans pending results) until the owner drains it.

By contrast cancelJob()'s own Doxygen (LDC1614.h:317-318) explicitly says "An active job produces one CANCELLED result". The recovery gesture the README documents (README.md, "Applied configuration and recovery": `device.invalidateAppliedState(reason); device.startInitialize(newId, absoluteDeadlineMs);`) shows no drain and no status check on the start.

Also reported independently as: invalidateAppliedState overwrites the partial-write provenance that its own cancelJob() just recorded

**Evidence**

```
Header (include/LDC1614/LDC1614.h:332-336):
  /// Mark hardware configuration unknown after removal, reset, brownout, or owner recovery.
  /// Performs zero I2C and preserves the supplied full status as provenance.
  /// @param reason Owner-observed cause retained as configuration-fault evidence.
  void invalidateAppliedState(const Status& reason);

Implementation (src/LDC1614.cpp:612-616):
  void LDC1614::invalidateAppliedState(const Status& reason) {
    if (_progress.active) {
      (void)cancelJob();
    }
    _appliedState = AppliedConfigState::UNKNOWN;

vs. the documented sibling (include/LDC1614/LDC1614.h:317-318):
  /// Idempotent and bus-silent. An active job produces one CANCELLED result;
  /// its completion timestamp is zero because cancelJob() accepts no clock.
```

**How it fails**

Owner task holds one undrained OperationResult (e.g. it publishes results on the next scheduler pass) and has an ACQUIRE in flight. A brownout/INTB observation triggers the README recovery gesture: invalidateAppliedState(reason) cancels the acquire and pushes a second result, so _resultCount == RESULT_CAPACITY == 2. The immediately following startInitialize(newId, deadline) hits `!_reserveResultSlot()` at src/LDC1614.cpp:495 and returns RESULT_QUEUE_FULL. invalidateAppliedState returned void, so nothing signalled the state change, and an owner that mirrors the README snippet (no status check on the start) has silently stopped recovering the device: appliedConfigState() is UNKNOWN forever, every subsequent startAcquire returns CONFIG_DIRTY, and no job is ever admitted again until something else happens to drain the FIFO.

**Proposed fix** (as corrected by adversarial review)

Documentation only; no behaviour change and no signature change. (1) In include/LDC1614/LDC1614.h replace the invalidateAppliedState comment block with: '/// Mark hardware configuration unknown after removal, reset, brownout, or owner\n/// recovery. Bus-silent. An active job is cancelled and produces one CANCELLED\n/// terminal result that occupies a result slot and reserves its operation id\n/// until takeResult() drains it; drain results before starting recovery work.\n/// The cancelled result carries any partial-write provenance; this call replaces\n/// the live configuration fault with the supplied full status.' (2) In README.md 'Applied configuration and recovery', extend the snippet to the honest gesture: 'device.invalidateAppliedState(reason);  // zero I2C; cancels any active job' / 'while (device.resultAvailable()) { device.takeResult(result); }' / 'const Status started = device.startInitialize(newId, absoluteDeadlineMs);' with one line noting that an undrained result plus a cancelled job exhausts RESULT_CAPACITY and yields RESULT_QUEUE_FULL. (3) Optionally pin the contract with a regression test in test/test_basic.cpp using the existing helpers: initializeAndWake(driver, fake, config); startAcquire(CH0,900)+pollToTerminal without takeResult; startAcquire(CH0,901) then poll(NOW_MS,1U) (still inProgress); driver.invalidateAppliedState(Status::Error(Err::I2C_BUS,"brownout",-1)); assertCode(Err::RESULT_QUEUE_FULL, driver.startInitialize(902, DEADLINE_MS)); then drain both results and assert the second is CANCELLED with operationId 901. No existing test changes behaviour — test_basic.cpp:2170-2180 already depends on the cancel and stays green.


---

### F25 &mdash; firstChannel() is exactly nextChannel(mask, Channel::NONE)

**Severity:** minor &nbsp;|&nbsp; **Site:** `src/LDC1614.cpp:61` &nbsp;|&nbsp; **Independently reported by:** 6 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 2/3 upheld

**What is wrong**

firstChannel(ChannelMask) scans indices 0..3 for the lowest set bit. nextChannel(ChannelMask, Channel after) does the same scan starting at `after == Channel::NONE ? 0U : channelIndex(after) + 1U`, so nextChannel(mask, Channel::NONE) starts at index 0 and returns the identical result for every input, including the empty mask (both return Channel::NONE). firstChannel has exactly one call site.

Also reported independently as: configurationPhaseForRegister/configurationChannelForRegister re-derive the register-to-phase/channel mapping that configWriteAt already owns; The acquire channel-advance block is duplicated verbatim in READ_STATUS_BEFORE and READ_DATA_LSB; outcomeForFailure() ignores its parameter and always returns TerminalOutcome::FAILED; _scratchCompleted duplicates _progress.completedChannels and is never read; _configuredChannelCount() returns the variant's physical channel count, not the count of configured channels

**Evidence**

```
src/LDC1614.cpp:61-68 (firstChannel) and 70-79 (nextChannel) contain the same loop body `if ((mask.bits & static_cast<uint8_t>(1U << index)) != 0U) return static_cast<Channel>(index);`. The single call is src/LDC1614.cpp:807 `_jobChannel = firstChannel(_progress.requestedChannels);`.
```

**How it fails**

A change to channel ordering or to the mask validity rule -- for example honoring only channels present on the selected variant -- is made in nextChannel, which is called on every subsequent channel step, and missed in firstChannel, which is called once. The acquisition then starts on a channel the new rule excludes but skips it on every later iteration, producing a batch whose first entry violates the rule the other entries follow.

**Proposed fix** (as corrected by adversarial review)

Do the whole duplication in one edit rather than only the firstChannel half, because after the proposed one-liner the two advance blocks become byte-identical apart from the `after` argument.

1) Delete firstChannel entirely (src/LDC1614.cpp:61-68).
2) Add one file-local helper next to nextChannel (still inside the anonymous namespace, which is inside namespace LDC1614, so JobProgress/JobPhase/cmd are all in scope - no header change, no new member, no ABI impact):

  Channel advanceAcquireChannel(JobProgress& progress, Channel after) {
    const Channel next = nextChannel(progress.requestedChannels, after);
    progress.phase = next == Channel::NONE ? JobPhase::READ_STATUS_AFTER
                                           : JobPhase::READ_DATA_MSB;
    progress.registerAddress = next == Channel::NONE
                                   ? cmd::REG_STATUS
                                   : cmd::regDataMsb(channelIndex(next));
    progress.channel = next;
    return next;
  }

3) Replace src/LDC1614.cpp:807-813 with `_jobChannel = advanceAcquireChannel(_progress, Channel::NONE);` and 854-862 with `_jobChannel = advanceAcquireChannel(_progress, _jobChannel);`.

Net effect: about 22 lines removed, one channel-advance behavior, zero behavioral change. No test changes - all 48 tests exercise these paths only through poll() and observe identical register order and identical JobProgress fields. Leave outcomeForFailure, _scratchCompleted and _configuredChannelCount alone in this change: the first is deliberate and commented, the second is a two-line redundancy whose removal (`_progress.completedChannels.bits |= bit;` plus dropping the member and its reset at line 1000) is a separate trivial cleanup, and renaming _configuredChannelCount to _variantChannelCount is a separate rename that should not be mixed into a dedup commit.


---

### F26 &mdash; Sleep entry silently destroys unread conversions, latched errors, and INTB state with no effect flag or documented warning

**Severity:** minor &nbsp;|&nbsp; **Site:** `src/LDC1614.cpp:1145` &nbsp;|&nbsp; **Independently reported by:** 1 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 3/3 upheld

**What is wrong**

The datasheet is unambiguous: "Entering Sleep Mode will clear all conversion results, any error conditions, and de-assert the INTB pin" (p.12) and "If the device is put into Sleep mode or Shutdown mode, all DATAx_MSB and DATAx_LSB registers are cleared of conversion data" (p.47). The driver enters sleep from two places -- the public `sleep()` (src/LDC1614.cpp:1145) and the `FORCE_SLEEP` first step of every INITIALIZE / APPLY_CONFIG / RESET_AND_REAPPLY replay (src/LDC1614.cpp:255-256) -- and neither records anything beyond `EffectFlag::PARTIAL_WRITE`, which means "a configuration write was confirmed", not "device-side read evidence was destroyed". `EffectFlag` has no value expressing the latter. The Doxygen on `sleep()` (include/LDC1614/LDC1614.h:353) says only "Enter sleep mode with one bounded CONFIG write", and neither docs/I2C_INTEGRATION.md's "DATA, STATUS, and INTB effects" section nor docs/HARDWARE_INTEGRATION.md mentions it. Every destructive *read* in this library is enumerated in detail across three documents; this equally destructive *write* is documented nowhere, which is exactly the kind of asymmetry that misleads an integrator who has read the contract carefully.

**Evidence**

```
Status status =
      _writeRegister(cmd::REG_CONFIG, buildConfigRegister(_config, true),
                     _config.i2cTimeoutMs);
  if (!status.ok()) {
    ...
  }
  _appliedState = AppliedConfigState::APPLIED_SLEEPING;
  return Status::Ok();
```

**How it fails**

A duty-cycled owner wakes the part, waits for INTB, and -- before draining the batch -- takes a power-saving decision and calls `sleep()`, intending to read the completed conversion afterwards, which is superficially safe because the datasheet states the I2C interface remains functional in sleep. The unread conversion and every latched ERR_* / ERR_CHAN are gone. After the next `wake()` and `startAcquire()`, the DATA registers read back as zero until the first post-wake conversion completes, and `decodeChannelSample()` flags `rawCount28 == 0` as `SampleQualityFlag::UNDER_RANGE` -- reporting an under-range fault the sensor never produced. The `sleep()` call returned `Status::Ok()` with no effect evidence, so nothing in the terminal record or the API documentation told the owner this would happen.

**Proposed fix** (as corrected by adversarial review)

Documentation only. No code change, no enum change, no test change; nothing in the 48 tests or the 6 checkers is affected (tools/check_readiness_claims.py only bans readiness wording -- plain descriptive text passes).

1. include/LDC1614/LDC1614.h:353-355, extend the sleep() Doxygen:
   /// @brief Enter sleep mode with one bounded CONFIG write.
   /// @details Sleep entry is destructive on the device side: every DATAx_MSB
   /// and DATAx_LSB register is cleared, all latched STATUS error conditions
   /// and UNREADCONVx are cleared, and INTB is de-asserted. Drain any pending
   /// conversion with startAcquire() before calling. The first acquisition
   /// after wake() reads zeros until a new conversion completes; that batch
   /// decodes as STALE plus UNDER_RANGE and is excluded from validChannels.

2. include/LDC1614/LDC1614.h:38, extend the FORCE_SLEEP enumerator comment:
   FORCE_SLEEP, ///< Sleep before replay; discards unread DATA and latched errors.

3. docs/I2C_INTEGRATION.md, immediately after the read-effects list at lines 152-156, add a short write-effects paragraph:
   'Two writes are destructive as well: entering sleep -- through sleep() or
   through the FORCE_SLEEP first step of initialization, apply, and
   reset/reapply -- clears all DATAx registers, clears all latched error
   conditions and UNREADCONVx, and de-asserts INTB. Drain a pending batch
   before sleeping or before starting any replay job.'

Optional, only if machine-checkable evidence is wanted later: give FakeLdc1614Device an `enterSleep()` that zeroes liveMsb/liveLsb/shadowMsb/shadowLsb/shadowValid/unreadMask/stickyStatusErrors and clears intbAsserted, invoked from its CONFIG write handler when MASK_CFG_SLEEP_MODE_EN is set; then a test `sleepClearsDeviceEvidence` asserting that after sleep/wake/startAcquire({CH0}) the batch has freshChannels.bits == 0, validChannels.bits == 0, and CH0 quality == STALE|UNDER_RANGE. That is a fixture-fidelity improvement, not part of the fix.


---

### F27 &mdash; Sample quality silently degrades to raw endpoints when ERROR_CONFIG routing is off, and ERR_ZC has no DATAx_MSB route at all

**Severity:** minor &nbsp;|&nbsp; **Site:** `include/LDC1614/LDC1614.h:161` &nbsp;|&nbsp; **Independently reported by:** 1 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 2/3 upheld

**What is wrong**

Every non-endpoint quality bit the driver decodes depends on ERROR_CONFIG routing that `validateConfig` never checks. Per Table 26 (p.28-29), each ERRx_2INT bit gates whether the error appears in STATUS at all ("b0: Do not report ... by asserting INTB pin and STATUS register"), and each ERRx_2OUT bit gates whether it appears in DATAx_MSB. `ErrorReporting` default-constructs with all twelve fields false, and `validateConfig` (src/LDC1614.cpp, the whole function) validates clocks, dividers, counts, modes, deglitch and INTB but never inspects `config.errorReporting`. A profile with all routing disabled therefore binds and applies cleanly, after which `_buildAcquisition` can only ever see raw 0x0000000 / 0x0FFFFFFF endpoints, yet still publishes `validChannels` documented as "Silicon-usable samples only" with no indication that error decoding is inert.

Two structural limits compound this and answer the ERR_CHAN attribution question directly. First, STATUS carries exactly one ERR_CHAN, so when two channels error in the same interval only one is attributable; the other is recoverable only from its DATAx_MSB flags, i.e. only if the matching ERRx_2OUT route is enabled. Second, ERR_ZC is the one error with NO ERR2OUT bit in the register map - zero-count exists only as STATUS.ERR_ZC. Combined with the datasheet's latch rule (Table 25, p.26: the error "is latched and maintained until either the STATUS register or the DATAx_MSB register corresponding to the Error Channel is read"), a zero-count error latched after the batch's STATUS-before read is destroyed by the driver's own DATAx_MSB read for that channel and can never reach `errorChannels` under any configuration. In practice a zero-count conversion also reads DATA = 0 and gets flagged UNDER_RANGE, so the verdict survives but the reported cause is wrong.

**Evidence**

```
include/LDC1614/LDC1614.h:161
  ChannelMask validChannels{};     ///< Silicon-usable samples only.

src/LDC1614.cpp:941-962  (all decoded status errors come from STATUS, which the *_2INT bits gate)
    const bool statusApplies =
        _scratchStatusBefore.hasError() && ...
      if (_scratchStatusBefore.errorZeroCount) {
        sample.quality |= sampleQualityFlag(SampleQualityFlag::ZERO_COUNT);
      }

include/LDC1614/CommandTable.h:129-140  (no ZC_ERR2OUT constant exists because the bit does not exist)
  static constexpr uint16_t MASK_ERRCFG_AL_ERR2OUT  = 0x0800;
  static constexpr uint16_t MASK_ERRCFG_UR_ERR2INT  = 0x0080;
  ...
  static constexpr uint16_t MASK_ERRCFG_ZC_ERR2INT  = 0x0004;

Datasheet Table 26 (p.28), bit 3 AL_ERR2INT: "b0: Do not report Amplitude Low errors by asserting INTB pin and STATUS register."
Datasheet Table 25 (p.26), ERR_CHAN: "Once flagged, any reported error is latched and maintained until either the STATUS register or the DATAx_MSB register corresponding to the Error Channel is read."
```

**How it fails**

An integrator building a low-power product wants only data-ready on INTB and no error interrupts, so it sets `cfg.errorReporting = ErrorReporting{}; cfg.errorReporting.dataReady = true;`. validateConfig accepts it, initialization and apply succeed, and AppliedConfigState reaches APPLIED_ACTIVE. ERROR_CONFIG is written as 0x0001, so no ERRx_2INT and no ERRx_2OUT route is enabled. CH0's sensor then enters a watchdog timeout: the silicon detects it, but ERR_WD reaches neither STATUS nor DATA0_MSB. The conversion result is mid-scale garbage, not 0x0000000 or 0x0FFFFFFF, so neither endpoint heuristic fires. Every batch reports `validChannels = {CH0}`, `errorChannels = {}` and quality FRESH only. The application publishes garbage inductance readings as verified-good measurements for as long as the fault persists, with nothing in the SampleBatch hinting that the driver's error decoding was disabled by configuration. The same integrator using the CLI's per-route toggles (examples/common/Ldc1614Cli.cpp:1693) can reach the identical state at runtime.

**Proposed fix** (as corrected by adversarial review)

Documentation only -- fix the contract so it stops claiming more than the decoder can deliver. No API change, no validateConfig change, no behavior change, so all 48 tests and all 6 checkers keep passing.

1. include/LDC1614/LDC1614.h:161, replace the misleading one-liner:
   ChannelMask validChannels{};  ///< No decoded quality objection; see routing note.

2. include/LDC1614/LDC1614.h, immediately above `struct SampleBatch` (~line 158), add a @note:
   /// @note Quality decoding is bounded by ERROR_CONFIG routing. WATCHDOG,
   /// AMPLITUDE_SUSPECT and ZERO_COUNT are decodable only when the matching
   /// ErrorReporting.status* route is set; DATA-borne WATCHDOG and
   /// AMPLITUDE_SUSPECT only when the matching ErrorReporting.data* route is
   /// set. Raw-endpoint UNDER_RANGE/OVER_RANGE are the only route-independent
   /// classifications. STATUS reports one error channel, so a second channel
   /// erroring in the same interval is visible only through its DATAx_MSB
   /// flags. Zero-count has no DATAx_MSB route, so a zero-count latched after
   /// the STATUS-before read is cleared by this job's own DATAx_MSB read of
   /// the error channel and is not decoded. With every route disabled,
   /// validChannels reflects endpoint and freshness evidence only.

3. include/LDC1614/Config.h, above `struct ErrorReporting` (line 141): one sentence stating that a route left false makes the corresponding SampleQualityFlag undecodable, and pointing at ErrorReporting::all() as the fully-observable policy.

4. README.md after line 231, one sentence carrying the same three points, next to the existing 'even when corresponding device reporting bits are disabled' sentence it completes.

Regression sketch (pins the boundary rather than changing it), test/test_basic.cpp against FakeLdc1614Device: build cfg with `cfg.errorReporting = ErrorReporting{}; cfg.errorReporting.dataReady = true;`, initializeAndWake(driver, fake, cfg), then `fake.injectConversion(0, 0x0800000, 0, 0)` -- note FakeLdc1614Device does NOT model ERROR_CONFIG gating (injectConversion sets DATA/STATUS error bits regardless of ERROR_CONFIG, FakeLdc1614Device.h:65-70), so gated silicon is expressed by injecting no error bits -- startAcquire({CH0}), poll to SUCCESS, then assert batch.validChannels.bits == 0x01, batch.errorChannels.bits == 0x00 and CH0 quality == FRESH. Name it `routingOffLeavesQualityUndecodable` so the documented limit has a test behind it.


---

### F28 &mdash; check_core_timing_guard.py keeps ~25 lines of unreachable code for a per-file allow-list contract that no longer exists

**Severity:** minor &nbsp;|&nbsp; **Site:** `tools/check_core_timing_guard.py:60` &nbsp;|&nbsp; **Independently reported by:** 1 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 3/3 upheld

**What is wrong**

ALLOWED_CALL_COUNTS and ALLOWED_INCLUDE_COUNTS are declared as empty dicts at lines 60-61 and never populated anywhere. Three code paths are therefore dead: (a) at line 119 `if rel not in ALLOWED_CALL_COUNTS` is always true, so the `continue` always fires and lines 122-128 (`expected = ALLOWED_CALL_COUNTS[rel]` and its per-call comparison) can never execute; (b) the loop at line 130 `for rel, expected in ALLOWED_CALL_COUNTS.items():` never iterates, making its 12-line body dead; (c) at line 143 `exp = ALLOWED_INCLUDE_COUNTS.get(rel, 0)` is always 0, so the `if exp != 0:` branch that appends "unsupported include allow-list entry" is dead. The guard's real behavior is simply "any forbidden call, include, or token in src/ or include/ is an error" -- which is what the surviving three lines do.

**Evidence**

```
tools/check_core_timing_guard.py:60 `ALLOWED_CALL_COUNTS: Dict[str, Dict[str, int]] = {}` and :61 `ALLOWED_INCLUDE_COUNTS: Dict[str, int] = {}`, with no assignment to either name anywhere in the repository (grep for ALLOWED_ in tools/ returns only lines 60, 61, 119, 122, 130, 143).
```

**How it fails**

A maintainer adding a legitimately allowed occurrence reads lines 119-141, believes there is a working allow-list mechanism, adds an entry to ALLOWED_CALL_COUNTS, and gets a confusing double report: the entry is checked twice (once by the dead branch that now wakes up at 122 and once by the reverse loop at 130), and the reverse loop reports a mismatch for any file that has zero observed calls. The maintenance cost is paid today for a contract that was removed.

**Proposed fix** (as corrected by adversarial review)

Two hunks in tools/check_core_timing_guard.py, no behavior change to what the guard enforces.

Hunk 1 - delete lines 60-61 (the two empty allow-list declarations) and one of the surrounding blank lines. Keep `from typing import Dict` on line 7: it is still used by the observed_calls/observed_includes/observed_tokens annotations at 83-85, 92, 100 and 108.

Hunk 2 - replace lines 118-146 with:

    for rel, counts in observed_calls.items():
        errors.append(f"forbidden timing calls in core: {rel} -> {counts}")

    for rel, counts in observed_includes.items():
        errors.append(f"forbidden framework/STL includes in core: {rel} -> {counts}")

That leaves the three observed_* reports in the same shape as the existing token report at 148-149. Verify with `python tools/check_core_timing_guard.py` (must still print 'Core timing/framework guard PASSED', exit 0) and, since the clean tree cannot exercise the failure path, once against a scratch fixture (a temporary src file containing `#include <Arduino.h>` and a `delay(` call) to confirm exit 1 with the two expected rows; delete the fixture afterwards. Nothing else in the repo changes - no C++ test, no other checker, and no doc references the removed identifiers or messages. If per-file allowances are ever genuinely needed, reintroduce them with a populated dict and a test rather than restoring speculative machinery.


---

### F30 &mdash; `verify` counts each register read failure as both a mismatch and a read failure

**Severity:** minor &nbsp;|&nbsp; **Site:** `examples/common/Ldc1614Cli.cpp:2821` &nbsp;|&nbsp; **Independently reported by:** 1 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 3/3 upheld

**What is wrong**

In `advanceVerifySession()` a failed `readRegister16()` increments both `_session.stats.skipped` and `_session.stats.failed`, while a genuine readback mismatch increments only `_session.stats.failed`. `printSessionSummary()` then prints `mismatched` from `failed` and `read_failures` from `skipped`, so every read failure is reported twice — once as a mismatch and once as a read failure. This breaks the arithmetic identity `matched + mismatched + read_failures == checked` that the repository's own HIL acceptance runner asserts (tools/ldc1614_hil_runner.py:1661), and it tells the operator that a register both mismatched and could not be read. Identical code and summary exist in the IDF twin at examples/esp_idf/basic/main/Ldc1614IdfCli.cpp:2855 and :2207.

**Evidence**

```
examples/common/Ldc1614Cli.cpp:2817-2823
    if (!readStatus.ok()) {
      if (selfTest) {
        selfTestFail("configuration register read", readStatus);
      } else {
        ++_session.stats.skipped;
        ++_session.stats.failed;      // <-- also counted as a mismatch

examples/common/Ldc1614Cli.cpp:2173-2179
    case SessionKind::VERIFY:
      printf("verify complete checked=%lu matched=%lu mismatched=%lu "
             "read_failures=%lu\n",
             ... _session.stats.completed,   // checked
             ... _session.stats.passed,      // matched
             ... _session.stats.failed,      // mismatched  (includes read failures)
             ... _session.stats.skipped);    // read_failures

tools/ldc1614_hil_runner.py:1661
        if matched + mismatched + read_failures != checked:
```

**How it fails**

On an LDC1614 fixture the device NACKs one CONFIG_REGISTERS read (bus glitch, or the device dropped off after a brownout) out of 23. The firmware prints `verify complete checked=23 matched=22 mismatched=1 read_failures=1`. 22 + 1 + 1 = 24 != 23, so the HIL runner's invariant check fires with a misleading "inconsistent verify accounting" diagnosis instead of the true single read failure, and a human reading the transcript concludes two registers were bad when only one read failed and none actually mismatched.

**Proposed fix** (as corrected by adversarial review)

Simpler than either option offered. `skipped` is already the read-failure counter and SessionStats is reset per session, so do not add a field and do not print a subtraction (a subtraction leaves the in-flight `session pass/fail/skip` line still violating the runner invariant at ldc1614_hil_runner.py:1418). Two edits per file: (1) delete the `++_session.stats.failed;` line in the read-failure branch (Ldc1614Cli.cpp:2822, Ldc1614IdfCli.cpp:2856), keeping `++_session.stats.skipped;` and `recordSessionFailure(readStatus);`; (2) make the verify terminal outcome account for read failures, replacing the final finishSession call (Ldc1614Cli.cpp:2851-2855) with `const bool clean = _session.stats.failed == 0U && _session.stats.skipped == 0U; finishSession(clean ? LDC1614::Status::Ok() : _session.stats.lastFailure, clean ? "SUCCESS" : "FAILED");` and the same in the IDF twin. That makes matched+mismatched+read_failures == checked true by construction, keeps pass+fail+skip == completed, and preserves FAILED on a read failure. No allocation, no logging, no blocking. None of the 48 tests change: test/test_cli.cpp:843 asserts `mismatched=1` after `fixture.fake.reg[REG_RCOUNT0] ^= 1U`, a genuine mismatch where skipped stays 0, and test/test_cli.cpp:790 only asserts the `read_failures=` substring. Add the regression sketched above to test_cli.cpp asserting `matched=22`/`mismatched=0`/`read_failures=1`/`outcome=FAILED`. Out of scope but worth a note: the earlier `expectedStatus` abort branch (2803-2810) increments `failed` without incrementing `completed`, which would break the same identity - it is unreachable while bind()/apply() validate the config, and the runner's `checked not in (13,23)` check fires there first, so leave it alone.


---

### F32 &mdash; ESP-IDF example never supplies build metadata, so `version` reports an unusable build_timestamp

**Severity:** minor &nbsp;|&nbsp; **Site:** `examples/esp_idf/basic/main/CMakeLists.txt:42` &nbsp;|&nbsp; **Independently reported by:** 1 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 3/3 upheld

**What is wrong**

The native example's CMake injects `LDC1614_GIT_COMMIT` and `LDC1614_GIT_STATUS` but nothing for `LDC1614_BUILD_DATE` / `LDC1614_BUILD_TIME` / `LDC1614_BUILD_TIMESTAMP`. The root component CMakeLists.txt only runs `generate_version.py check`; the build-metadata defines are appended exclusively by the PlatformIO pre-script path (`_append_build_metadata_defines`, scripts/generate_version.py:170), which the IDF build never reaches. `include/LDC1614/Version.h` therefore falls back to its `#ifndef` defaults and `LDC1614::BUILD_TIMESTAMP` expands to the two-word literal `"unknown-date unknown-time"`, which the CLI's `version` command prints verbatim into a whitespace-delimited key=value evidence line. Neither tools/check_cli_contract.py nor tools/check_idf_example_contract.py catches this: both only assert that the literal token `build_timestamp=` appears somewhere in the sources (check_idf_example_contract.py:269).

**Evidence**

```
examples/esp_idf/basic/main/CMakeLists.txt:42-45
target_compile_definitions(${COMPONENT_LIB} PRIVATE
  "LDC1614_GIT_COMMIT=\"${LDC1614_EXAMPLE_GIT_COMMIT}\""
  "LDC1614_GIT_STATUS=\"${LDC1614_EXAMPLE_GIT_STATUS}\""
)

include/LDC1614/Version.h:20-31
#ifndef LDC1614_BUILD_DATE
#define LDC1614_BUILD_DATE "unknown-date"
#endif
...
#define LDC1614_BUILD_TIMESTAMP LDC1614_BUILD_DATE " " LDC1614_BUILD_TIME

examples/esp_idf/basic/main/Ldc1614IdfCli.cpp:3470-3441 (version command)
           LDC1614::VERSION, LDC1614::GIT_COMMIT, LDC1614::GIT_STATUS,
           LDC1614::BUILD_TIMESTAMP, _platform.platformName, ...
```

**How it fails**

An operator flashes the native ESP-IDF diagnostic and runs `version` for a release HIL transcript. The line reads `version=3.1.0 firmware_git=9a86034 firmware_status=clean build_timestamp=unknown-date unknown-time platform=esp-idf-native framework=esp-idf ...`. A parser splitting the line on whitespace into key=value pairs sees `build_timestamp=unknown-date` followed by a bare `unknown-time` token before `platform=`, and there is no way to tell which firmware image produced the transcript when two builds share a Git SHA (for example a `firmware_status=dirty` rebuild during bring-up). The Arduino example, built through PlatformIO, prints a real date and time for the same field, so the two maintained diagnostics disagree on the provenance evidence they are contracted to emit.

**Proposed fix** (as corrected by adversarial review)

Simpler than proposed: define only the two leaf macros and let Version.h compose the timestamp (its `#ifndef LDC1614_BUILD_TIMESTAMP` default is already `LDC1614_BUILD_DATE " " LDC1614_BUILD_TIME`). In examples/esp_idf/basic/main/CMakeLists.txt, next to the existing git block, add `string(TIMESTAMP LDC1614_EXAMPLE_BUILD_DATE "%Y-%m-%d" UTC)` and `string(TIMESTAMP LDC1614_EXAMPLE_BUILD_TIME "%H:%M:%S" UTC)`, then extend the existing target_compile_definitions call at :42-45 with `"LDC1614_BUILD_DATE=\"${LDC1614_EXAMPLE_BUILD_DATE}\""` and `"LDC1614_BUILD_TIME=\"${LDC1614_EXAMPLE_BUILD_TIME}\""`. No SOURCE_DATE_EPOCH / LDC1614_REPRODUCIBLE_BUILD plumbing is needed: CMake's string(TIMESTAMP) already honours SOURCE_DATE_EPOCH, and the formats match generate_version.py's %Y-%m-%d / %H:%M:%S so both diagnostics emit the identical shape. Note in the surrounding comment that this is a configure-time value, i.e. exactly the same staleness class as the git commit/status already captured there -- no new inaccuracy is introduced, and the two provenance fields stay mutually consistent. For the regression guard, add to tools/check_idf_example_contract.py right after the existing provenance loop (:269-271) a check against `cmake_text` specifically, not `all_native` (which already contains cmake_text and so cannot distinguish): `for macro in ("LDC1614_GIT_COMMIT", "LDC1614_GIT_STATUS", "LDC1614_BUILD_DATE", "LDC1614_BUILD_TIME"): if macro not in cmake_text: fail(...)`.


---

### F33 &mdash; Test locks in silent loss of destructive-read provenance when a STATUS/DATAx_MSB read fails mid-acquisition

**Severity:** minor &nbsp;|&nbsp; **Site:** `test/test_basic.cpp:1781` &nbsp;|&nbsp; **Independently reported by:** 6 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 2/3 upheld

**What is wrong**

The driver reports EffectFlag::READ_SIDE_EFFECTS only after a read callback *succeeds* (src/LDC1614.cpp:802-805, 828-831, 873-876). A failing read returns straight to _finishJob with no effect evidence at all. For writes the driver deliberately handles exactly this ambiguity via writeFailureMayHaveCommitted() (src/LDC1614.cpp:139-141) -> INDETERMINATE_WRITE, but the read path has no counterpart. A STATUS or DATAx_MSB read is destructive on the wire: once the device ACKs and clocks out the two data bytes, STATUS sticky errors + ERR_CHAN are cleared and INTB de-asserts, and DATAx_MSB consumes UNREADCONVx and latches the LSB shadow. A transport that reports I2C_TIMEOUT / I2C_BUS after the device already served the bytes therefore leaves the driver claiming effects == 0 while the evidence is gone. test_acquire_failure_at_every_phase_preserves_prior_complete_publication asserts this wrong behaviour as the contract, with FORCED_READ deliberately being an I2C_TIMEOUT (test_basic.cpp:20-21) - the exact class of failure that is ambiguous. The fixture cannot even express the ambiguous case: it has commitWriteBeforeFailure for writes (FakeLdc1614Device.h:64, 187) but writeRead() skips applyReadSideEffects() unconditionally when the transfer is marked failed (FakeLdc1614Device.h:217-221).

Also reported independently as: Datasheet FIN_DIVIDER>=2 rule above 8.75 MHz has zero coverage; the case that appears to cover it is shadowed; Per-channel STATUS error attribution via ERR_CHAN is never exercised by any acquisition test; Five validateConfig guards have no negative test and survive being disabled; Sensor-frequency OFFSET term is only checked for monotonicity, and the datasheet worked example has no regression test; SampleBatch.statusAfter payload is never asserted by any test

**Evidence**

```
test/test_basic.cpp:1781-1783:
    TEST_ASSERT_EQUAL(failed > 1U,
                      hasEffect(failure.effects,
                                EffectFlag::READ_SIDE_EFFECTS));
src/LDC1614.cpp:799-805:
      Status status = _readRegister(cmd::REG_STATUS, raw, transferTimeoutMs);
      --remainingTransfers;
      ++_progress.completedTransfers;
      if (!status.ok()) {
        return _finishJob(outcomeForFailure(status), status, nowMs);
      }
      _progress.effects |= effectFlag(EffectFlag::READ_SIDE_EFFECTS);
VERIFIED: patching the two acquisition read-failure branches to `if (writeFailureMayHaveCommitted(status)) _progress.effects |= effectFlag(EffectFlag::READ_SIDE_EFFECTS);` makes exactly one assertion fail - test_basic.cpp:1781 'Expected 0 Was 1'. Nothing else in the 48-test suite notices.
```

**How it fails**

Owner runs startAcquire on CH0..CH3 while STATUS holds a latched ERR_WD for CH2. poll() issues the STATUS-before read; the LDC1614 ACKs and returns 0x2800|..., clearing ERR_WD, ERR_CHAN and de-asserting INTB. The owner's I2C driver hits its own per-transfer deadline during the STOP phase and returns Err::I2C_TIMEOUT. The driver publishes OperationResult{outcome=FAILED, effects=0}. The owner, following the documented contract 'Destructive read effects remain reported', concludes nothing was consumed and simply re-issues the acquisition. The watchdog error for CH2 is gone forever and the next batch reports CH2 as valid.

**Proposed fix** (as corrected by adversarial review)

Widen the existing flag instead of adding public API surface.

1. src/LDC1614.cpp:139 - rename the anonymous-namespace helper writeFailureMayHaveCommitted -> transferFailureMayHaveReachedDevice (file-local, lines 11-357 are an unnamed namespace, so zero API impact; 6 call sites).
2. In _pollAcquire, in the READ_STATUS_BEFORE (~803), READ_DATA_MSB (~829) and READ_STATUS_AFTER (~874) failure branches only, replace the bare return with:
     if (transferFailureMayHaveReachedDevice(status)) {
       _progress.effects |= effectFlag(EffectFlag::READ_SIDE_EFFECTS);
     }
     return _finishJob(outcomeForFailure(status), status, nowMs);
   Leave READ_DATA_LSB (838-862) exactly as it is: DATAx_LSB has no documented side effect.
3. Reword include/LDC1614/LDC1614.h:66 to 'At least one destructive read reached, or may have reached, the device.' and docs/I2C_INTEGRATION.md:112 to match ('...whether at least one destructive read reached or may have reached the device...'). No contract checker greps that sentence (verified across tools/*.py).

Do NOT add EffectFlag::INDETERMINATE_READ: EffectFlags is public API, both CLI renderers mask 0x07 and would each need a 16-case switch (examples/common/Ldc1614Cli.cpp:465 and examples/esp_idf/basic/main/Ldc1614IdfCli.cpp:499), and the single-transfer read APIs could never deliver the 4th bit anyway. Over-reporting is the safe direction - it can only make an owner distrust evidence it already should distrust - and for reads there is no proven-vs-possible action split (unlike PARTIAL_WRITE vs INDETERMINATE_WRITE, which drive AppliedConfigState). This keeps it a PATCH/MINOR doc-level change, not an enum append.

Tests: test_acquire_failure_at_every_phase_preserves_prior_complete_publication (test_basic.cpp:1743) line 1781 becomes an unconditional TEST_ASSERT_TRUE(hasEffect(failure.effects, EffectFlag::READ_SIDE_EFFECTS)) for failed 1..6 - state in the commit that the old assertion was correct for the old wording, not buggy. Then add one short case in the same test that sets fake.failStatus to an Err::I2C_NACK_ADDR status at transfer 1 and asserts failure.effects == 0, pinning the carve-out. No fixture change is required: NACK_ADDR already skips applyReadSideEffects, which is exactly right (an unacknowledged address means no register access occurred). FakeLdc1614Device::applyReadSideEffectsBeforeFailure is unnecessary - the driver cannot observe it, so it would assert nothing.


---

### F34 &mdash; LDC1612 register/channel gating is enforced only by the driver's own guards; the fake is variant-blind

**Severity:** minor &nbsp;|&nbsp; **Site:** `test/support/FakeLdc1614Device.h:44` &nbsp;|&nbsp; **Independently reported by:** 5 finder(s) &nbsp;|&nbsp; **Adversarial vote:** 3/3 upheld

**What is wrong**

FakeLdc1614Device holds a flat 128-entry register file and answers reads/writes for channel-2 and channel-3 registers regardless of the DeviceVariant the driver was bound with. The three guards that keep an LDC1612 binding off nonexistent registers - isLdc1614OnlyRegister() (src/LDC1614.cpp:130-138) used by readRegister16 (src/LDC1614.cpp:1221-1224) and writeRegister16 (src/LDC1614.cpp:1240-1243), and the acquisition-mask guard in startAcquire (src/LDC1614.cpp:471-475) - are therefore checked only by the driver itself. The suite exercises exactly one of them: readRegister16(cmd::REG_DATA2_MSB) at test_basic.cpp:2220. There is no test at all for writeRegister16 on an LDC1612-only register, none for the RCOUNT2/OFFSET2/SETTLECOUNT2/CLOCK_DIVIDERS2/DRIVE_CURRENT2 ranges, and none for an out-of-profile acquisition mask. (test_cli_ldc1612_rejects_every_channel_2_3_staged_path_atomically only covers the CLI's own staging layer, not the driver.)

Also reported independently as: Test fixture reset does not restore RCOUNTx to the datasheet default 0x0080; Test fixture reports STATUS error bits that ERROR_CONFIG masking suppresses on real silicon; Fixture omits the datasheet's sleep-clears-DATA rule and never exercises its own MSB-clears-error-latch branch; Fixture gates STATUS.DRDY on ERROR_CONFIG but leaves the sticky error bits ungated

**Evidence**

```
test/support/FakeLdc1614Device.h:44:
  uint16_t reg[128] = {};
VERIFIED by mutation, each surviving all 48 tests:
  (a) deleting `(reg >= cmd::REG_OFFSET2 && reg <= cmd::REG_OFFSET3) ||` from isLdc1614OnlyRegister;
  (b) deleting `(reg >= cmd::REG_RCOUNT2 && reg <= cmd::REG_RCOUNT3) ||`;
  (c) deleting the whole `if (channels.empty() || (channels.bits & ~_config.channels.bits) != 0U)` guard from startAcquire.
```

**How it fails**

With guard (c) gone, an application on a real LDC1612 calls startAcquire(ChannelMask{0x0F}, ...). _pollAcquire walks requestedChannels 0..3 and puts reads of registers 0x04/0x05/0x06/0x07 on the bus - addresses the LDC1612 does not implement. The device returns undefined data or NACKs mid-frame, and _buildAcquisition (which loops only over _configuredChannelCount()) publishes a SUCCESS batch whose selectedChannels claims four channels while channel[2]/channel[3] are zero-initialised, i.e. reported as UNDER_RANGE hardware faults on a part that has no such channels. Likewise with (a)/(b), a CLI 'reg 0x0E' or 'wreg 0x0A' against an LDC1612 is accepted and silently reads/writes a nonexistent register.

**Proposed fix** (as corrected by adversarial review)

Reject the proposed fix's shape - do NOT make FakeLdc1614Device variant-aware. The datasheet does not specify how an LDC1612 answers a channel-2/3 address, so making the fixture NACK (or return an 0xFFFF sentinel) would encode invented device behaviour, and a `variantViolations` tripwire is fixture machinery to catch what a direct API assertion catches outright. The gap is in the driver's public contract, so assert it at the public API.

Minimal fix: one new test in test/test_basic.cpp (no fixture change, no header change, no behaviour change to any of the 48 existing tests - this only adds assertions), registered in main():

void test_ldc1612_binding_rejects_ldc1614_only_registers_and_masks() {
  FakeLdc1614Device fake;
  LDC1614::LDC1614 driver;
  Config config = makeConfig(fake, DeviceVariant::LDC1612, true);  // channels = 0x03
  initializeAndWake(driver, fake, config);
  fake.clearIo();
  static constexpr uint8_t LDC1614_ONLY[] = {
      cmd::REG_RCOUNT2, cmd::REG_RCOUNT3,
      cmd::REG_OFFSET2, cmd::REG_OFFSET3,
      cmd::REG_SETTLECOUNT2, cmd::REG_SETTLECOUNT3,
      cmd::REG_CLOCK_DIVIDERS2, cmd::REG_CLOCK_DIVIDERS3,
      cmd::REG_DRIVE_CURRENT2, cmd::REG_DRIVE_CURRENT3};
  for (uint8_t reg : LDC1614_ONLY) {          // config registers: only the
    uint16_t raw = 0xFFFFU;                    // variant gate can reject them
    assertCode(Err::INVALID_PARAM, driver.readRegister16(reg, raw));
    TEST_ASSERT_EQUAL_HEX16(0U, raw);
    assertCode(Err::INVALID_PARAM, driver.writeRegister16(reg, 0x1234U));
  }
  uint16_t raw = 0xFFFFU;
  assertCode(Err::INVALID_PARAM, driver.readRegister16(cmd::REG_DATA3_LSB, raw));
  assertCode(Err::INVALID_PARAM, driver.startAcquire(ChannelMask{0x0F}, 900U, DEADLINE_MS));
  assertCode(Err::INVALID_PARAM, driver.startAcquire(ChannelMask{0x04}, 901U, DEADLINE_MS));
  assertCode(Err::INVALID_PARAM, driver.startAcquire(ChannelMask{}, 902U, DEADLINE_MS));
  TEST_ASSERT_EQUAL_UINT16(0U, fake.transferCalls);   // nothing reached the bus
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::APPLIED_ACTIVE),
                          static_cast<uint8_t>(driver.appliedConfigState()));
}

Why this is sufficient and minimal: the RCOUNT2..DRIVE_CURRENT3 registers pass isValidRegisterAddress() and isConfigurationRegister(), so the ONLY thing that can reject them is the variant gate - each deleted clause of isLdc1614OnlyRegister turns one loop iteration from INVALID_PARAM into a real bus transfer, killing mutations (a) and (b) and every sibling clause. The three startAcquire cases kill mutation (c) (out-of-profile 4-channel mask, out-of-profile single bit, empty mask), and transferCalls==0 pins the bus-silence half of the contract. Roughly 25 lines, no new API, no allocation, no logging, no blocking. If fixture-side defence-in-depth is still wanted later, the cheapest honest form is a test-only tripwire - `uint16_t ldc1614OnlyAccesses` incremented in readValue()/applyWrite() for 0x04-0x07/0x0A-0x0B/0x0E-0x0F/0x12-0x13/0x16-0x17/0x20-0x21, asserted zero at the end of LDC1612 tests, with no change to what the fixture returns - but that is optional and must not gate the fix above.


---

## Claims that were checked and refuted

These were reported by a finder but did not survive independent verification. They are listed so the same ground is not re-covered.

- **F04 &mdash; RESET_AND_REAPPLY cannot enforce a post-reset settle interval before the identity read** (`src/LDC1614.cpp:665`) &mdash; refuted 3/3. REFUTED. The load-bearing claim - 'no driver state can hold the identity read until a caller-chosen interval has elapsed' and 'the owner has no knob at all' - is false, and it is false because of the cooperative contract this driver is built on. poll() (src/LDC1614.cpp:567-579) executes at most maxTransfers transfers and returns IN_PROGRESS; with budget 1 the RESET_DEV write at 640-668 consumes the budget, the loop exits, and the job sits with _progress.phase == JobPhase::VERIFY_MANUFACTURER, registerAddress == 0x7E, completedTransfers == 1, all of it readable through the public bus-silent jobProgress() (include/LDC1614/LDC1614.h:323). Nothing happens on the bus until the owner chooses to ca

- **F05 &mdash; STATUS-after error bits are never decoded, so errorChannels can report a clean batch while the device latched an error** (`src/LDC1614.cpp:941`) &mdash; refuted 3/3. The code description is accurate (only _scratchStatusBefore feeds per-channel quality) but both stated consequences fail against the datasheet and the published API.

Consequence (1) is wrong in both directions. Per-sample error cause does NOT come from STATUS at all: DATAx_MSB carries ERR_URx[15]/ERR_ORx[14]/ERR_WDx[13]/ERR_AEx[12] for the very conversion being published (facts.md:260, digest 'DATAx_MSB' line), and src/LDC1614.cpp:1535-1559 decodeChannelSample already converts all four into UNDER_RANGE/OVER_RANGE/WATCHDOG/AMPLITUDE_SUSPECT. So a published sample that was itself faulty is flagged with its real cause independently of either STATUS snapshot. Conversely, an error that latches A

- **F09 &mdash; validateConfig accepts CONFIG bit combinations in which the mandatory per-channel IDRIVE code has no hardware effect, then reports the ineffective value as verified applied state** (`src/LDC1614.cpp:1413`) &mdash; refuted 3/3. Misreads the datasheet in three ways.

(1) IDRIVEx is not inert when RP_OVERRIDE_EN=0. CONFIG.SENSOR_ACTIVATE_SEL b1 (Low Power Activation) is documented as "the LDC uses the value programmed in DRIVE_CURRENTx during sensor activation" (datasheet p.29 field table, line 3378 of the extracted text), and Config.h:212 defaults sensorActivation to LOW_POWER. So in the exact CLI scenario the finding describes (`rp 0` from defaults), the programmed IDRIVE still governs the activation drive. The claim that the profile is left "inert" and the sensor runs at "the frozen INIT_IDRIVE (0x00 = 16 uA)" is fabricated - nothing in the datasheet says INIT_IDRIVE is frozen at reset when RP_OVERRIDE_EN=0; §8.1.

- **F11 &mdash; SETTLECOUNT has no lower bound in single-channel mode, so the reset/never-assigned value 0 validates and produces a chronic amplitude error** (`src/LDC1614.cpp:1420`) &mdash; refuted 3/3. SETTLECOUNTx = 0x0000 is a defined, working register encoding, not an unassigned sentinel, and the driver models it deliberately.

(a) Datasheet Table 17 (p.22) enumerates the encoding: "0x0000: Settle Time = 32/fREF0; 0x0001: Settle Time = 32/fREF0; 0x0002-0xFFFF: (SETTLECOUNT0*16)/fREF0". That is why Config.h documents rcount/finDivider/frefDivider as "0 is unspecified" but not settleCount: for those three, 0 is a datasheet-reserved encoding (FIN_DIVIDER b0000 and FREF_DIVIDER 0x000 are "do not use"), while for SETTLECOUNT 0 is a legal 32-cycle setting. src/LDC1614.cpp:1642-1645 implements exactly that rule (`settleCount <= 1U ? 32ULL : count*16`) - the baseline lists this as verified-corr

- **F15 &mdash; The 4,239-line ESP-IDF CLI is a verbatim copy of the Arduino CLI; only the class name, namespace, and an inlined CliStyle.h differ** (`examples/esp_idf/basic/main/Ldc1614IdfCli.cpp:1`) &mdash; refuted 2/3. The raw facts are right but the defect framing is not, and the evidence materially misstates the existing enforcement.

What I confirmed: the normalized diff really is ~40 changed lines confined to the first 45 lines (include line, inlined CliStyle.h body, namespace/class rename); neither .cpp contains an Arduino, ESP-IDF or FreeRTOS symbol; platformio.ini [env:native] compiles only examples/common/Ldc1614Cli.cpp, so the IDF copy has no unit-test coverage.

Why it is refuted:
1. It contradicts an explicit, binding AGENTS.md rule: 'Keep Arduino/IDF example command parity through repo-local contracts/checkers, not by compiling one framework's sources into the other.' The transport carve-out th

- **F17 &mdash; freshChannels is sampled once at the start of the batch, so a channel that converts mid-batch is published as STALE and discarded** (`src/LDC1614.cpp:924`) &mdash; refuted 3/3. The mechanism is described accurately (STATUS-before at src/LDC1614.cpp:806, per-channel MSB/LSB at 819-863, STATUS-after at 866-878, freshness derived from the pre-read snapshot at 924-925 and folded into invalidFlags at 968), but it is not a defect.

1. The behaviour is the documented contract, not an oversight. README.md:230-232 states verbatim: "Watchdog, amplitude, zero-count, stale, and data-loss conditions remain visible separately from transport success. All of those conditions exclude the affected channel from `validChannels`; `errorChannels` and per-channel quality retain the cause." `errorChannels` exists precisely because STALE is deliberately in `validChannels` but deliberately 

- **F21 &mdash; SampleQualityFlag::CONFIG_UNKNOWN can never be set, and both CLIs carry an unreachable decode row for it** (`include/LDC1614/LDC1614.h:131`) &mdash; refuted 2/3. The factual core is true but it is not a defect. include/LDC1614/LDC1614.h:131 declares `CONFIG_UNKNOWN = 1U << 8, ///< Compatibility-only; never emitted. Use AppliedConfigState.` — the enumerator documents, on its own line, both that it is never set and the exact replacement the failure scenario says the integrator missed. I confirmed by grep that the only other references in the tree are the two CLI decode rows (examples/common/Ldc1614Cli.cpp:1244, examples/esp_idf/basic/main/Ldc1614IdfCli.cpp:1278). REACHABILITY: there is no call sequence that misbehaves. I traced examples/common/Ldc1614Cli.cpp:1230-1252 `Cli::printQuality`: the table is a pure decode loop (`if (!hasSampleQuality(quality,

- **F22 &mdash; 35 constants in the public CommandTable.h have no reader anywhere in the repository** (`include/LDC1614/CommandTable.h:61`) &mdash; refuted 3/3. The raw grep result is correct - I re-ran it per symbol across src/ include/ examples/ test/ tools/ scripts/ docs/ with CommandTable.h excluded, and all 35 named symbols return 0 external references while their mask twins return 3-4 - but 'no reader inside the repository' is the expected state for this header, not evidence of a defect. CommandTable.h is a PUBLIC header (AGENTS.md:25 lists it under 'Public API headers only') whose stated job is to publish the chip register map; the driver additionally exposes readRegister16()/writeRegister16() 'for advanced diagnostics' (include/LDC1614/LDC1614.h:365-378), so the intended readers of the shifts, reset defaults and field encodings are downstrea

- **F24 &mdash; Documented pure math helpers reject valid profiles on transport grounds and re-run full validateConfig on every per-sample call** (`src/LDC1614.cpp:1566`) &mdash; refuted 3/3. Both mechanical facts check out - src/LDC1614.cpp:1565-1569 and 1596-1600 open with an unconditional validateConfig(), which enforces transport preconditions at 1308-1315 and 1400-1403 that the arithmetic never uses - but the 'documented pure helper' premise that makes this a defect is wrong. The helpers' own Doxygen says 'Pure, CHECKED inverse DATA conversion' and '@param config Explicit validated-style profile' (LDC1614.h:409-413), and README.md:280 heads a section listing decodeDeviceStatus()/encodeErrorReporting() alongside them, i.e. 'pure' there means bus-silent and side-effect-free, not 'accepts a partially populated Config'. Config is documented as one indivisible object - 'Complete 

- **F29 &mdash; isConfirmed() exists only to be wrapped by the one-line requireConfirmation(), in both CLI copies** (`examples/common/Ldc1614Cli.cpp:656`) &mdash; refuted 2/3. The code fact is accurate: examples/common/Ldc1614Cli.cpp:656-660 defines `isConfirmed` and the next line defines `requireConfirmation` as a pure pass-through, with the same pair at Ldc1614IdfCli.cpp:690/694, and all 15 call sites in each file go through `requireConfirmation`. But nothing here is a defect. (1) The category is wrong: `isConfirmed` is not dead code, it has a caller, and both functions live in the file-local anonymous namespace (Ldc1614Cli.cpp:12-13 `namespace ldc1614_cli { namespace {` ... 771 `}  // namespace`), so if a future edit did orphan it the compiler's -Wunused-function fires immediately. (2) The failure scenario does not hold. Because `requireConfirmation` is the sol

- **F31 &mdash; Owner transport counters ignore every not-configured/address-mismatch failure** (`examples/esp32/I2cMasterTransport.cpp:291`) &mdash; refuted 2/3. The mechanical observation is true (write()/writeRead() at I2cMasterTransport.cpp:287-303 and :305-324 return the validateContext() status before recordTransfer(), so a refused call updates no counter), but it is not a defect: it is the invariant the whole xfer feature is built on, and the proposed fix would break the repository's own contract. (1) recordTransfer() (:91-103) increments a per-kind counter writes/writeReads/discoveries and treats `failures` as a subset of those; the CLI derives total = writes+writeReads+discoveries (Ldc1614Cli.cpp:4046-4048) and tools/ldc1614_hil_runner.py:1583-1586 enforces `failures <= total` AND `total == 0 => failures == 0 and last_code == 0`. So the contr

