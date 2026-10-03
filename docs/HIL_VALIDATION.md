# LDC1614 Hardware-in-the-Loop Validation

This document defines the HIL procedure and evidence expected before release or
field-readiness claims. Reviewed repository evidence is indexed under
[`docs/reports/`](https://github.com/janhavelka/LDC1614/blob/main/docs/reports/README.md).

The current library revision has **not been HIL tested** on a sensor-equipped
board. Historical no-sensor transcripts below cover only their named fixture
and firmware revision. They do not qualify another PCB, its LC sensors, or
later code changes. Host parser tests and dry runs exercise tooling only.

Use Python 3.11 with the `pyserial` version pinned in `requirements-dev.txt`.
On Windows, install that serial dependency into the runner's Python environment
without installing another PlatformIO Core:

```powershell
python -m pip install pyserial==3.5
```

The existing `scripts\pio.cmd` wrapper owns PlatformIO selection on Windows.
Linux CI installs the complete `requirements-dev.txt` tool set.

Use:

```sh
python tools/ldc1614_hil_runner.py --profile arduino --port "<port>" --baud 115200 --operator "<name>" --board "<exact board/fixture>" --expected-target esp32s2 --expected-firmware-commit "<flashed Git SHA>" --require-run --json-out hil.json --raw-transcript-out hil.serial.txt
```

For a board with the LDC1614 chip present but no LC sensor/coil attached, use
the no-sensor fixture matrix:

```sh
python tools/ldc1614_hil_runner.py --profile arduino --fixture no-sensor --port "<port>" --baud 115200 --operator "<name>" --board "<exact board/fixture>" --expected-target esp32s2 --expected-firmware-commit "<flashed Git SHA>" --require-run --json-out hil-no-sensor.json --raw-transcript-out hil-no-sensor.serial.txt
```

The no-sensor mode exercises target/device identity, owner bus, bus-frequency,
and transfer-statistics diagnostics, exhaustive desired-profile output, masked
configuration readback, STATUS and selected register reads, sleep/wake,
initialization, full apply, confirmed reset/reapply, one status-aware
acquisition, invalidation/re-initialization, confirmed diagnostic write
followed by full replay, confirmed controller-only bus recovery with the
required state check and re-initialization, end/bind/re-initialization, idle
cancellation, true self-test, and pure helpers. With `--include-stress`, it
also runs bounded protocol-only stress. Every asynchronous command
prints `CLI scheduled: command=<name> session=<id>` and exactly one correlated
terminal `CLI result`. A prompt or immediate in-progress status is never a
pass. With no LC sensor, acquisition and stress validate transport, protocol,
state, and error reporting only. They do not validate conversion accuracy,
channel physics, fresh sensor cadence, or drive suitability.

Here, `cancel` is the bus-silent idle-cancel command check. The base runner does
not claim active-job cancellation timing evidence; that remains an explicit
`NOT_RUN` gate requiring an interactive timing fixture. The confirmed raw
CONFIG write writes the known sleeping example-profile value and is immediately followed
by complete initialization/replay. It is not a general arbitrary-write test.

To stress the same no-sensor matrix repeatedly in one captured run, add
`--repeat-command-set N`. The runner records both the base command count and
the expanded command count in the artifact.

After any raw `ESP_ERR_INVALID_STATE` (`259`), first capture the exact failed
command, phase, register, and subsequent combined-read behavior. On the
Arduino profile's pinned ESP-IDF 5.5.5 this raw value includes ordinary NACK,
so it must not be labeled a stuck bus without independent evidence. Attempt the explicit controller-only owner
reconstruction, then require complete initialization/replay and repeated
combined reads without a power cycle. Do not line-clear solely because of
`259`, and do not use an address-only ACK as admission. If that gate fails,
physically remove and reapply power to the
ESP32 and LDC/shared bus before collecting another candidate run. A firmware
reboot is not a device power cycle. The first cold gate must read `version`,
`cfg`, `probe`, `init`, and `probe` without preceding discovery. Then run a
controlled absent-address combined-read NACK, explicit owner recovery, initialization,
and repeated valid combined reads. Run at least 100 correlated reset/reapply
cycles before the full matrix. Stop on the first failed gate; do not soak an
already-failed transport/device state. A logic-analyzer capture of SDA and SCL
at the first natural failure is required before assigning its physical cause
to the controller, signal integrity, or the LDC161x parser.

The maintained runner enforces that stop rule: after the first unexpected base
result it sends no later command and records the remaining matrix entries as
`NOT_RUN`. The diagnostic `busrecover` command reconstructs its sole owned
ESP-IDF bus/device lifecycle without pulsing the lines or probing an address,
invalidates applied state, and still requires `init`. A
production shared-bus manager must
coordinate and recreate all registered device handles and re-admit every
required peer; do not copy the single-device example as a general shared-bus
policy.

For a time-bounded no-sensor soak, request the duration explicitly. The soak
keeps the port open, executes only complete cycles, and ends every cycle awake:

```sh
python tools/ldc1614_hil_runner.py --profile arduino --fixture no-sensor --port "<port>" --operator "<name>" --board "<exact board/fixture>" --expected-target esp32s2 --expected-firmware-commit "<flashed Git SHA>" --include-long-soak --soak-duration-s 3600 --soak-cycle-delay-s 1 --json-out hil-soak.json --raw-transcript-out hil-soak.serial.txt
```

That complete default gate includes confirmed `resetreapply`. When investigating
the open reset-adjacent failure, use `--skip-default-commands`, supply an
explicit non-reset base sequence, and opt in with `--allow-reduced-soak-gate`;
label the artifact `custom_reduced`. An unconfirmed `resetreapply` may appear
only as an invalid-input test that proves zero-I2C rejection. Never describe a
reduced non-reset soak as RESET_DEV qualification. The retained one-hour
artifact `docs/reports/20260804/one-hour-nonreset-e4d0436.json` is exactly that
reduced variant: `--skip-default-commands --allow-reduced-soak-gate` with the
base sequence `version`, `cfg`, `discover`, `busrecover confirm`, `state`,
`init`, `wake`, `probe`, `drv`, `wake`, `drv`. No retained artifact records a
passing complete default gate; `comprehensive-no-sensor-5e3199e.json` is the
only default-scope run kept, and it stops at its confirmed `resetreapply`.

The runner first requires every base-matrix command and firmware identity check
to pass. It will not spend an hour soaking an ambiguous candidate. The fixed
soak cycle is `version`, `probe`, `status`, `sleep`, `wake`, `busrecover
confirm`, `state`, `init`, `wake`, `probe`, and `drv`. The intermediate
`state` must prove `applied=UNKNOWN`; every final `drv` must prove `bound=1`
and `applied=APPLIED_ACTIVE`. The summary records requested/actual duration,
complete cycles, command counts, failures, ambiguous responses, unexpected
startup banners, incomplete cycle, and worst command latency. Raw output is
journaled and flushed after startup and every command; a serial or close-time
exception after target payload produces an explicit failed artifact instead of
discarding the run.
A serial exception, unexpected reset, or command failure is not converted into a pass. The explicit no-sensor
classifier permits LDC under/over-range,
watchdog, amplitude, or zero-count flags only after the command's structured
response and successful transport status are present; it never permits an I2C,
identity, timeout, or nonzero-status failure.

If no serial port and real LDC1614/LDC1612 hardware are supplied, the runner
reports `NOT_RUN`. It must not be interpreted as a pass.

For automated acceptance, use `--require-run`, check the process exit code, and
require `overall_status=PASS` in the JSON. Without `--require-run`, a no-port
`NOT_RUN` exits successfully so tooling can inspect a planned run. `FAIL` and
`UNKNOWN` always exit nonzero. A passing automatic matrix covers only its
executed commands; listed manual or unpopulated-fixture checks remain untested.
Set `--expected-target`, `--address`, and `--channel-count` to the actual build
and hardware; defaults are `esp32s2`, `0x2A`, and four channels. Use
`--channel-count 2` for LDC1612. An opt-in coverage flag does not change wiring,
address, or silicon variant.

Supplying `--port` only proves that a serial port was requested. The runner
marks `hardware_attached=true` only when it captures real command/startup
payload from the target firmware. A port open with no firmware payload is
reported as `evidence_type=serial_not_run` unless a serial exception occurred;
exceptions produce `evidence_type=serial_failure` and fail the run.
Host exception text is not target payload. A serial disconnect after target
output or a Ctrl+C interruption retains the received bytes and fails the run.
An unexpected firmware
startup banner during any command also fails, even if a later response looks
successful.

For a real run, `--operator` and `--board` are mandatory evidence. The runner
requires the target `version` response to report a clean firmware Git revision,
compares it with `--expected-firmware-commit` (or the host HEAD when omitted),
and stores the host checkout identity separately. A host SHA is never treated
as proof of the flashed image. Firmware cleanliness includes tracked and
untracked source-tree changes; a failed Git-status query reports `unknown` and
cannot pass acceptance. Missing address, variant channel count, exact TI
identity, or target build identity makes the run fail. `--expected-target`
names the required firmware-reported build target; `--expected-idf-version`
names the exact ESP-IDF version and is required by the `idf` profile, while
the `arduino` profile is pinned to `5.5.5`. `UNKNOWN` is also a nonzero
verification exit, not a successful run.

## No-hardware runner checks

When no LDC1614/LDC1612 fixture is attached, do not run hardware commands
against an arbitrary serial port. Check the host tooling without creating
repository evidence:

```sh
python tools/ldc1614_hil_runner.py --parser-self-test
python tools/ldc1614_hil_runner.py --profile arduino --dry-run --quiet
```

If review needs generated no-hardware output, write it to a temporary directory.
Dry-run and no-port results are `NOT_RUN`; do not commit or describe them as HIL
evidence.

## Firmware Profiles

| Profile | Intended firmware | Default safe commands |
| --- | --- | --- |
| `arduino` | `examples/01_basic_bringup_cli` | Manifest-derived safe identity, profile, verify, status, timing, readiness, and self-test commands. Sensor acquisition/cadence commands require a sensor fixture. |
| `arduino --fixture no-sensor` | `examples/01_basic_bringup_cli` with chip but no LC sensor | Manifest-derived lifecycle, identity, full profile/readback, protocol acquisition, state/fault, raw-write/replay, decoder/timing, and self-test commands; stress is added by `--include-stress`. |
| `idf` | `examples/esp_idf/basic` | The same manifest-derived command and evidence contract, implemented by the native fixed-buffer CLI. |

The runner is configurable. Use `--command` for board-specific commands and
`--skip-default-commands` when validating custom firmware.
Every custom gate must include `version`, `cfg`, and a successful `probe`:
firmware/profile text and discovery at another address cannot prove the configured
chip is present. Contradictory target firmware revision, cleanliness, version,
or runtime records fail acceptance. Contradictory revision/status metadata is
reported as `inconsistent`, instead of silently selecting the first record.
Use `--expect-token`, `--failure-token`, and `--expected-failure-token` only for
documented fixture-specific cases. An expected-failure token can accept only a
structurally correlated failed asynchronous or immediate CLI result; it cannot
override timeout, missing-envelope, mismatched-session, or malformed normal
command output. Prefer `--expected-failure "COMMAND=TOKEN"` for one negative
operation inside a larger recovery sequence; its token cannot mask a later
recovery, replay, or identity failure. Built-in invalid-input coverage instead
requires each exact usage contract and proves that no job was admitted.

A soak with `--skip-default-commands` is rejected unless
`--allow-reduced-soak-gate` is also present. Such an artifact is labeled
`custom_reduced` and can support only the commands in its recorded base gate;
it never qualifies the omitted reset/reapply or complete default matrix.

Both CLIs expose the same cooperative core jobs and bounded diagnostic
sessions. The runner requires matching command/session scheduled and terminal
records. Runtime address and variant changes remain deliberately unavailable:
they are physical binding facts. Stress is diagnostic, not a production
scheduler. Sample-rate acceptance is enabled only for a sensor-equipped fixture
and counts fresh, valid, in-range samples with no error/overrun evidence.

The runner groups evidence into the following coverage categories, mapped to
the LDC1614 ownership contract:

| Category | Automatic LDC1614 evidence | Deliberate boundary |
| --- | --- | --- |
| Base | Ordered lifecycle, identity, profile/readback, destructive-status, helper, self-test, and final-state matrix | No sensor-physics claim |
| Configuration matrix | `--include-config-matrix`; staged legal values and numeric boundaries, then reset/validate/discard | No profile commit or live tuning |
| Applied channel modes | `--include-mode-matrix`; each physical single channel and every supported sequential length, apply/readback, acquisition, sleep/wake, then compiled-profile restoration | Requires suitable compiled settings for every channel; sensor mode requires populated, characterized coils |
| Invalid input | `--include-invalid-inputs`; exact usage rejection and no job admission | Does not substitute for core API invalid-parameter tests |
| Benchmark/stress | `--include-stress` for bounded protocol stress | Sample rate and physical quality require a sensor fixture |
| Cooperative job API | Scheduled/terminal command-session correlation, progress/result snapshots, and idle cancel | Active cancellation timing remains `NOT_RUN` without an interactive fixture |
| Destructive paths | Confirmed all-register dump and known CONFIG write followed immediately by full replay | Arbitrary raw writes are not automatic |
| Manual fixtures | Unrequested checks are listed as `NOT_RUN`; SD/INTB/drive opt-ins send their diagnostic commands | Physical behavior still needs independent observation; address/variant, unplug, stuck-bus, and active-cancel setup is external |

## Safe Default Procedure

1. Record operator, board, firmware, Git commit, serial port, baud, expected I2C
   address, channel count, and timestamp.
   Build from a clean commit with reviewed pins, clock, counts, error routing,
   drive settings, and populated sensors. Close other serial monitors. Check
   `--serial-dtr` and `--serial-rts` against the board's reset wiring; opening
   the port can reset some boards. Establish the intended cold-start condition
   and capture it rather than silently discarding repeated restarts.
2. Open the serial port and capture startup output.
3. Run the selected profile's safe commands.
4. Classify each command from the transcript. Ambiguous command output is
   `UNKNOWN`, not a pass. Maintained commands require command-specific output;
   arbitrary nonempty text and a bare `code=0` probe are failures. Device
   ID/probe/read failures are failures, not skips.
5. Write a JSON result with command classification evidence and retain a raw
   target transcript or logic-analyzer trace for the exact release fixture.
   When `--raw-transcript-out` is supplied, the JSON stores the raw filename,
   byte count, and SHA-256 digest instead of embedding duplicate transcript
   text. Repeated stress output may be condensed when metadata, command counts,
   per-base-command outcomes, firmware/device identity, and every non-pass
   detail remain available. `--markdown-out` is an optional review rendering;
   do not commit it beside the canonical JSON when it only duplicates the same
   transcript and results.

Preview the exact matrix with the same options plus `--dry-run` before opening
serial. `--command-timeout-s` is the host wait for a complete command response,
not an I2C timeout or core deadline. Set it to cover the requested count, CLI
session duration, and serial output. A too-short timeout is a failed run; do not
resume that artifact after changing it. After a failure, retain the JSON/raw
pair, correct the cause, restore the documented starting profile, and start a
new run. A failed mode matrix may leave the last applied profile in place.

## Optional Opt-in Procedure

Run these only when hardware and operator setup explicitly support them:

- Address `0x2B` requires rebuilding with an explicit `ADDR_VDD` profile; the
  runner records `--include-address-0x2b` as a skipped external setup item.
- `--include-stress` runs the bounded cooperative `stress`, confirmed
  `stress_mix`, protocol-complete `stress_id`, and short CLI `soak` sessions
  for either firmware profile; `--stress-count` sets the iteration count
  (default 10; the retained 187-command artifact used 100). A no-sensor run
  proves protocol/transport stability only.
- `--include-reset-stress` appends the confirmed bounded `stress_reset`
  session: repeated reset-and-reapply jobs, each writing `RESET_DEV` and
  replaying the complete profile. It is accepted on the no-sensor fixture but
  drives the software-reset path the Validation Matrix still records as
  negative, so run it only while that gate is being qualified; no retained
  artifact contains a completed `stress_reset` session.
- `--include-busfreq-stress` appends the confirmed bounded `stress_busfreq`
  session: repeated owner 100/400 kHz switch and restore, applied-state
  invalidation, and full re-initialization on the no-sensor fixture. It
  produced the 100/400 kHz re-admission result recorded in
  `docs/reports/20260804/nonreset-exhaustive-post-rail-v2-e4d0436.json`.
- `--include-config-matrix` exercises every safe cache-only setting family,
  legal enum values and per-physical-channel numeric boundaries. It never
  commits the staged profile; it finishes with profile reset, validation,
  discard, and a live-driver state snapshot.
- `--include-mode-matrix` applies each single channel and every sequential mode
  (0/1 for LDC1612; 0/1, 0/1/2, 0/1/2/3 for LDC1614), verifies register readback,
  acquires each selected channel, and checks sleep/wake. It starts each mode
  from `profile reset` and restores the **compiled profile** at the end; existing
  manual CLI edits are discarded. Every physical channel therefore needs safe
  clock/count/drive/sensor bounds in the build, even if the default profile
  selects only channel 0. With a sensor fixture, `--mode-sample-count` (default 3)
  requires that many fresh, valid, in-range conversions per channel per mode.
  Without sensors it checks complete protocol batches and preserves fault flags;
  it makes no measurement-quality claim. A failed step stops the matrix at that
  step, so the restoration is not claimed unless its final replay/readback passes.
- `--include-invalid-inputs` verifies bounded numeric, enum, argument-count,
  and confirmation rejection without admitting an asynchronous job or changing
  the staged/live profile.
- `--sample-rate-count` appends a counted freshness-gated acquisition session only for a
  sensor-equipped fixture. Every requested sample must be fresh, valid,
  in-range, non-error, and non-overrun; otherwise acceptance fails.
- SD shutdown/wake if SD is wired and controlled.
- INTB observation if INTB is wired to a host GPIO or analyzer.
- Unplug/replug or induced NACK.
- Stuck-bus fixture tests.
- A bounded automated soak on either Arduino or native ESP-IDF uses
  `--include-long-soak --soak-duration-s <seconds>`. The no-sensor cycle remains
  the protocol/recovery cycle above. A sensor cycle runs `version`, `probe`,
  `verify`, counted freshness-gated `samplerate`, and `drv`. `--soak-sample-count`
  (default 10) controls acquisitions per cycle; `--sample-rate-channel` selects
  the channel. This validates that channel in the configured mode, not every
  channel or the production scheduler's cadence. Use the applied mode matrix
  for all physical channels and the actual application fixture for scheduler,
  shared-bus, and production-cadence qualification. The first failed/ambiguous
  command or firmware restart stops the soak, records an incomplete cycle,
  and fails acceptance; no partial cycle counts as complete.
- Drive-current/coil tuning with oscilloscope or an application-specific
  amplitude procedure.

Example sensor-equipped native ESP-IDF run after configuring all populated
channels in the firmware build:

```sh
python tools/ldc1614_hil_runner.py --profile idf --port "<port>" --operator "<name>" --board "<board and coils>" --expected-target esp32s3 --expected-idf-version "<exact IDF version>" --expected-firmware-commit "<flashed clean Git SHA>" --include-mode-matrix --mode-sample-count 10 --include-reset-stress --stress-count 100 --include-long-soak --soak-duration-s 3600 --soak-sample-count 20 --sample-rate-channel 0 --json-out sensor-hil.json --raw-transcript-out sensor-hil.serial.txt
```

The existing runner was extended using the sibling ADS1115/TCA9548A bounded
soak and partial-cycle checks, SCD41 explicit lifecycle/requalification steps,
and INA228/OPT4001 command-specific evidence gates as reference. It remains one
runner backed by the shared Arduino/native-IDF CLI contract.

Each sensor `read`/`last` must contain exactly the selected channel rows, coherent
DATA MSB/LSB/raw counts, consistent quality/masks and destructive STATUS evidence,
and in-range sensor frequency. Sample-rate acceptance requires every numbered
per-sample quality and readiness row, not just a positive summary. The selected
channel's UNREAD bit comes from the acquisition's own STATUS-before snapshot;
an additional readiness STATUS read would destructively erase that evidence.
Global DRDY need not be asserted when a selected channel has fresh data partway
through a sequential scan. Each clean not-ready acquisition may be repeated
within the same immutable per-sample deadline. The host checks raw UNREAD,
readiness/quality consistency, check numbering, deadline stability, and the final
summary. Raw `read` does not wait for freshness, so a stale raw read fails sensor
acceptance; use `samplerate` to wait for a fresh selected-channel conversion.

Host capture is bounded to 1 MiB per response and 128 MiB per session. Exceeding
either limit fails the run and preserves an explicitly incomplete transcript;
it never silently truncates a passing report. Split long/high-volume campaigns
into separate retained runs before these limits are reached.

## Manual fixture procedures

The automatic runner cannot create the following physical conditions. Record
the fixture setup, stimulus, expected limit, raw observation, and outcome in a
separate test record. Link the command transcript and analyzer or oscilloscope
capture to that record. A CLI `code=0` proves only the reported operation;
it does not prove a pin voltage, waveform, current, or sensor response.

| Procedure | Setup and bounded action | Pass condition |
| --- | --- | --- |
| Sensor and channel mapping | Populate the selected channels with characterized coils. Run each legal mode with `--include-mode-matrix`, then move one target at a time over the documented range. Record raw counts, computed frequency, reference clock, and an independent frequency/amplitude measurement. | Each intended channel responds; all accepted samples are fresh, valid, within the declared bounds, and free of error/overrun evidence. Measured amplitude and accuracy meet the fixture's recorded limits. |
| Destructive reads and INTB | Enable the intended error/DRDY routes and observe active-low INTB independently. After a new conversion, compare one STATUS read with an immediate second read while no additional conversion can complete. Repeat with acquisition at the production cadence and with a deliberate bounded read delay. | The first snapshot retains the event and unread bits; the second reflects their consumption unless a new conversion occurred. Pin transitions and DATA/STATUS evidence agree. Sequential DRDY marks the last channel; it is not per-channel freshness. |
| SD and power loss | With controlled SD or device power, first retain a good batch. Assert shutdown, invalidate applied state in the owner, then release SD and wait at least 2 ms before full initialization. Read back the profile, wake, and collect fresh data. | No trusted acquisition is admitted while state is unknown. Identity/replay/readback and fresh acquisition succeed after recovery. Observe SD, rail voltage, and current independently. |
| MCU-only restart | Keep the LDC rail energized while restarting the MCU. Capture SDA, SCL, SD, ADDR, and the rail across restart, then attempt full identity and replay. Repeat the documented product startup sequence a fixed number of times. | Every admitted startup establishes identity and complete configuration before acquisition; no retained DATA is treated as fresh. Any failed combined read or unexplained reset fails this test. |
| Address and variant | Power down, set the address strap, and build for the actual LDC1612 or LDC1614. Run the matrix with matching `--address` and `--channel-count`; repeat for each supported fixture. | Exact identity and profile checks pass. LDC1612 permits channels 0/1 and rejects 2/3. Identity values alone must not be used to infer the variant. |
| Fault and shared-bus recovery | Use a controlled disconnect or fault fixture to inject one address NACK, transaction timeout, or held-low line. Record callback duration and status. Clear the physical fault, perform the owner's bounded recovery, invalidate, and initialize. Read another shared-bus device before, during, and after the test as its policy permits. | No callback exceeds its stated timeout, no hidden retries occur, partial/ambiguous effects remain visible, and recovery ends in verified admission or an explicit unavailable state. The other device meets its documented service limit. |
| Active cancellation and deadlines | Use a test owner that pauses between `poll(now, 1)` calls. Cancel after selected transfer boundaries, including after a write. Separately poll at the absolute deadline with zero and nonzero budgets. Drain the result, submit a new operation ID, and repeat with a pending cancelled result. | SDA/SCL show no callback after cancellation or deadline expiry. Exactly one matching terminal result is delivered, partial acquisition is absent, write effects are retained, and replacement work obeys the two-result capacity. |
| Error routes and drive limits | With a safe electrical fixture, exercise each enabled under/over-range, amplitude, zero-count, and continuous-mode watchdog condition. Record STATUS-before, DATA, STATUS-after, routing, and independent amplitude/frequency. Test normal and CH0 high-current drive separately if used. | The reported channel and quality agree with observable evidence. Disabled routes are recorded as unavailable evidence. No faulted sample is accepted as a valid measurement; current and amplitude stay within the fixture's limits. |

Record a fixed repeat count and timeout for each procedure before starting.
Stop on the first unexpected result and preserve the failing capture. Conditions
that the available fixture cannot safely create remain `NOT_RUN`; host tests
do not fill that physical-evidence gap. Use the application's actual owner and
timing for cancellation and fault tests; the stock unattended CLI matrix does
not exercise those timing boundaries.

## Validation Matrix

| Test | Safe default? | Requires hardware/operator? | Current evidence | Needed evidence |
| --- | --- | --- | --- | --- |
| Probe/device ID | Yes | LDC1612/LDC1614 board | Clean `e4d0436` passed exact identity in the 187-command matrix and every one of 2,926 soak cycles | Repeat on each sensor-equipped production fixture |
| Address `0x2A` | Yes | ADDR strapped low | Clean `e4d0436` exhaustive matrix and soak confirm the chip at `0x2A` | Repeat on each production board |
| Address `0x2B` | No | ADDR strapped high or selectable | Not run | Opt-in probe/read logs at `0x2B` |
| LDC1612 channel bounds | Yes if LDC1612 present | LDC1612 hardware | Native tests only | HIL showing channels 0/1 valid and 2/3 rejected |
| LDC1614 channel config 0..3 | Yes | LDC1614 hardware | Clean `e4d0436` passed complete replay/readback and every cache-only four-channel configuration boundary | Repeat with the production sensor profile |
| LDC1614 sensor reads 0..3 | Yes if channels populated | LDC1614 hardware/sensors | Not run, no sensor attached | Safe reads for channels 0..3 |
| Safe raw read per enabled channel | Yes | Sensors connected | Not run | Raw/read transcript with DATA error flags checked |
| Config readback | Yes | Hardware | Clean `e4d0436` exhaustive matrix passed complete replay, dump, and verification | Repeat cleanly with the production profile |
| Reset/reapply and owner recovery | Yes | Hardware | RESET_DEV remains negative: clean `5e3199e` wrote reset then failed the next ID read. No-reset owner recovery is positive: clean `e4d0436` passed 2,926 controller reconstruction/full-init cycles | Qualify RESET_DEV separately if exposed; repeat owner recovery with the product backend and shared peer |
| MCU-only reset with LDC energized | No | Hardware/analyzer | A clean `e4d0436` upload/reboot interval changed a previously readable target into the persistent failed-read state; a rail cycle restored it | Capture SDA/SCL/VDD/SD/ADDR across first recurrence and qualify defined-SD startup |
| Deadline/cancel/result identity | Yes | Hardware | Native tests only | Correlated operation IDs and bus-silent deadline/cancel trace |
| INTB behavior | No | INTB wired/observable | Not run | Active-low push-pull behavior logs or analyzer capture |
| SD shutdown/wake | No | SD wired/controlled | Not run | Shutdown/wake transcript and current/identity behavior |
| Induced address NACK | No | Operator/fault fixture | Clean `e4d0436` repeatedly tolerated protocol-complete `0x2B` NACK during discovery and re-admitted `0x2A`. A shared peer at `0x3C` appears only in superseded bundles retained in Git history through `d3b434a`, not in the current evidence set. | Repeat with controlled phase injection and prove the production shared peer remains usable |
| Unplug/replug | No | Operator/fault fixture | Not run | Failure, recovery, and post-recovery read logs |
| Stuck bus | No | Test fixture | Not run | Bounded timeout/recovery logs |
| Bounded soak | No | Stable fixture | Clean `e4d0436` completed 3,600.0 seconds: 2,926 no-reset reconstruction/re-admission cycles, 32,186 commands, zero failures/unknowns/resets, 32 ms worst latency | Repeat at production sensor cadence on the exact application-owned ESP32 backend; qualify RESET_DEV separately if exposed |
| Drive-current tuning | No | Sensor/oscilloscope/procedure | Not run | IDRIVE setting, amplitude evidence, application calibration notes |

## Evidence Rules

- Retain the JSON result and raw transcript with the release artifacts. Do not
  create pass artifacts by hand. Keep the transcript in the raw file only and
  bind it to compact JSON with its filename, byte count, and SHA-256 digest.
  Preserve counts, base-command outcomes, and every non-pass detail.
- At least one raw serial transcript or logic-analyzer trace must be retained
  for production acceptance of the exact board, sensor, wiring, configuration,
  and release revision. Positive committed no-sensor evidence exists for clean
  `e4d0436`, but no positive sensor-equipped artifact exists for the exact
  `v3.2.0` release commit.
- Standalone `*.log` files are ignored as temporary output. Use a nonignored
  extension such as `.serial.txt` for a reviewed repository capture; release-only
  captures may instead remain attached to the release.
- Dry-run, no-port, `NOT_RUN`, and empty-payload outputs are temporary review
  aids, not HIL evidence. Do not commit them unless a maintainer explicitly
  requires one for an active audit.
- Hardware logs must name the board, sensor/coil, address strap, channel count,
  firmware profile, firmware-reported Git commit/status, host checkout, and
  operator.
- Run `python tools/check_repository_hygiene.py` before committing evidence; it
  verifies that each structured artifact references one tracked raw transcript
  with matching size and SHA-256.
- Simulation, native tests, and PlatformIO/CI builds are useful software
  evidence, but they are not hardware validation.
