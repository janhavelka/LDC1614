# LDC1614 Hardware-in-the-Loop Validation

This document defines the HIL procedure and evidence expected before release or
field-readiness claims. Reviewed repository evidence is indexed under
[`docs/reports/`](https://github.com/janhavelka/LDC1614/blob/main/docs/reports/README.md).

Use:

```sh
python -m pip install --requirement requirements-dev.txt
python tools/ldc1614_hil_runner.py --profile arduino --port "<port>" --baud 115200 --operator "<name>" --board "<exact board/fixture>" --expected-firmware-commit "<flashed Git SHA>" --json-out hil.json --raw-transcript-out hil.serial.txt
```

For a board with the LDC1614 chip present but no LC sensor/coil attached, use
the no-sensor fixture matrix:

```sh
python tools/ldc1614_hil_runner.py --profile arduino --fixture no-sensor --port "<port>" --baud 115200 --operator "<name>" --board "<exact board/fixture>" --expected-firmware-commit "<flashed Git SHA>" --json-out hil-no-sensor.json --raw-transcript-out hil-no-sensor.serial.txt
```

The no-sensor mode exercises target/device identity, exhaustive desired-profile
output, masked configuration readback, STATUS and selected register reads,
sleep/wake, initialization, full apply, confirmed reset/reapply, one
status-aware acquisition, invalidation/re-initialization, confirmed diagnostic
write followed by full replay, end/bind/re-initialization, idle cancellation, true
self-test, and pure helpers. With `--include-stress`, it also runs bounded
protocol-only stress. Every asynchronous command
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
command, phase, register, and subsequent combined-read behavior. On ESP-IDF
5.5.5 this raw value includes ordinary NACK, so it must not be labeled a stuck
bus without independent evidence. Attempt the explicit controller-only owner
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

Supplying `--port` only proves that a serial port was requested. The runner
marks `hardware_attached=true` only when it captures real command/startup
payload from the target firmware. A port open with no firmware payload is
reported as `evidence_type=serial_not_run`.

For a real run, `--operator` and `--board` are mandatory evidence. The runner
requires the target `version` response to report a clean firmware Git revision,
compares it with `--expected-firmware-commit` (or the host HEAD when omitted),
and stores the host checkout identity separately. A host SHA is never treated
as proof of the flashed image. Firmware cleanliness includes tracked and
untracked source-tree changes; a failed Git-status query reports `unknown` and
cannot pass acceptance. Missing address, variant channel count, exact TI
identity, or target build identity makes the run fail. `UNKNOWN` is also a
nonzero verification exit, not a successful run.

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

The runner follows the same coverage categories used by the maintained BME280
and INA228 I2C libraries, adapted to the LDC1614 ownership contract:

| Category | Automatic LDC1614 evidence | Deliberate boundary |
| --- | --- | --- |
| Base | Ordered lifecycle, identity, profile/readback, destructive-status, helper, self-test, and final-state matrix | No sensor-physics claim |
| Configuration matrix | `--include-config-matrix`; staged legal values and numeric boundaries, then reset/validate/discard | No profile commit or live tuning |
| Invalid input | `--include-invalid-inputs`; exact usage rejection and no job admission | Does not substitute for core API invalid-parameter tests |
| Benchmark/stress | `--include-stress` for bounded protocol stress | Sample rate and physical quality require a sensor fixture |
| Cooperative job API | Scheduled/terminal command-session correlation, progress/result snapshots, and idle cancel | Active cancellation timing remains `NOT_RUN` without an interactive fixture |
| Destructive paths | Confirmed all-register dump and known CONFIG write followed immediately by full replay | Arbitrary raw writes are not automatic |
| Manual fixtures | Explicit `NOT_RUN` rows in every artifact | INTB, SD, 0x2B/LDC1612, coil/drive, unplug, and stuck-bus evidence must be collected on the named fixture |

## Safe Default Procedure

1. Record operator, board, firmware, Git commit, serial port, baud, expected I2C
   address, channel count, and timestamp.
2. Open the serial port and capture startup output.
3. Run the selected profile's safe commands.
4. Classify each command from the transcript. Ambiguous command output is
   `UNKNOWN`, not a pass. Maintained commands require command-specific output;
   arbitrary nonempty text and a bare `code=0` probe are failures. Device
   ID/probe/read failures are failures, not skips.
5. Write a JSON result with command classification evidence and retain a raw
   target transcript or logic-analyzer trace for the exact release fixture.
   Repeated stress output may be condensed when metadata, command counts,
   per-base-command outcomes, firmware/device identity, and every non-pass
   detail remain available. `--markdown-out` is an optional review rendering;
   do not commit it beside the canonical JSON when it only duplicates the same
   transcript and results.

## Optional Opt-in Procedure

Run these only when hardware and operator setup explicitly support them:

- Address `0x2B` requires rebuilding with an explicit `ADDR_VDD` profile; the
  runner records `--include-address-0x2b` as a skipped external setup item.
- `--include-stress` runs the bounded cooperative `stress`, confirmed
  `stress_mix`, and short CLI `soak` sessions for either firmware profile. A
  no-sensor run proves protocol/transport stability only.
- `--include-config-matrix` exercises every safe cache-only setting family,
  legal enum values and per-physical-channel numeric boundaries. It never
  commits the staged profile; it finishes with profile reset, validation,
  discard, and a live-driver state snapshot.
- `--include-invalid-inputs` verifies bounded numeric, enum, argument-count,
  and confirmation rejection without admitting an asynchronous job or changing
  the staged/live profile.
- `--sample-rate-count` appends a counted DRDY/acquisition session only for a
  sensor-equipped fixture. Every requested sample must be fresh, valid,
  in-range, non-error, and non-overrun; otherwise acceptance fails.
- SD shutdown/wake if SD is wired and controlled.
- INTB observation if INTB is wired to a host GPIO or analyzer.
- Unplug/replug or induced NACK.
- Stuck-bus fixture tests.
- A bounded automated no-sensor soak may use `--include-long-soak` with
  `--soak-duration-s <seconds>`. Sensor-equipped production-cadence soak still
  requires an application fixture that correlates operation results.
- Drive-current/coil tuning with oscilloscope or an application-specific
  amplitude procedure.

## Validation Matrix

| Test | Safe default? | Requires hardware/operator? | Current evidence | Needed evidence |
| --- | --- | --- | --- | --- |
| Probe/device ID | Yes | LDC1612/LDC1614 board | Clean `c3e2ed8` default/extended matrices and 3,197-cycle reduced soak passed exact identity checks | Repeat on each sensor-equipped production fixture |
| Address `0x2A` | Yes | ADDR strapped low | Clean `c3e2ed8` matrices and reduced soak confirm the chip at `0x2A` | Repeat on each production board |
| Address `0x2B` | No | ADDR strapped high or selectable | Not run | Opt-in probe/read logs at `0x2B` |
| LDC1612 channel bounds | Yes if LDC1612 present | LDC1612 hardware | Native tests only | HIL showing channels 0/1 valid and 2/3 rejected |
| LDC1614 channel config 0..3 | Yes | LDC1614 hardware | Clean `c3e2ed8` extended matrix passed complete profile/readback and configuration boundaries | Repeat with the production sensor profile |
| LDC1614 sensor reads 0..3 | Yes if channels populated | LDC1614 hardware/sensors | Not run, no sensor attached | Safe reads for channels 0..3 |
| Safe raw read per enabled channel | Yes | Sensors connected | Not run | Raw/read transcript with DATA error flags checked |
| Config readback | Yes | Hardware | Clean `c3e2ed8` default and extended matrices passed | Repeat cleanly with the production profile |
| Reset/reapply and owner recovery | Yes | Hardware | Clean `c3e2ed8` default and extended matrices each passed one reset/reapply, then the full soak gate reproduced ambiguous detail 259; a clean post-fix runner correctly classified the following initialization failure after historical bus-reset/ACK recovery | Qualify controller-only reconstruction plus complete identity/replay and repeated valid combined reads without a power cycle |
| Deadline/cancel/result identity | Yes | Hardware | Native tests only | Correlated operation IDs and bus-silent deadline/cancel trace |
| INTB behavior | No | INTB wired/observable | Not run | Active-low push-pull behavior logs or analyzer capture |
| SD shutdown/wake | No | SD wired/controlled | Not run | Shutdown/wake transcript and current/identity behavior |
| Induced address NACK | No | Operator/fault fixture | Clean scans observed `0x2A` and peer `0x3C`; individual recovery/replay passed, but repeated reset/recovery still failed on pioarduino `55.03.311` | Controlled NACK, deterministic recovery/replay, valid combined reads, and shared-device proof |
| Unplug/replug | No | Operator/fault fixture | Not run | Failure, recovery, and post-recovery read logs |
| Stuck bus | No | Test fixture | Not run | Bounded timeout/recovery logs |
| Bounded soak | No | Stable fixture | Clean `c3e2ed8` completed a labeled reduced 3,600-second soak: 3,197 cycles/19,182 commands, zero soak-command failures/unknowns/resets, 32 ms worst latency; the complete default gate separately failed reset/reapply | Repeat at production sensor cadence after the full reset/recovery gate passes |
| Drive-current tuning | No | Sensor/oscilloscope/procedure | Not run | IDRIVE setting, amplitude evidence, application calibration notes |

## Evidence Rules

- Retain the JSON result and raw transcript with the release artifacts. Do not
  create pass artifacts by hand. Compact JSON is acceptable for repeated stress
  only when it preserves counts, command outcomes, and every non-pass detail.
- At least one raw serial transcript or logic-analyzer trace must be retained
  for production acceptance of the exact board, sensor, wiring, configuration,
  and release revision. The committed post-v3 transcripts are negative
  transport-regression evidence; no positive exact-release raw artifact exists.
- Standalone `*.log` files are ignored as temporary output. Use a nonignored
  extension such as `.serial.txt` for a reviewed repository capture; release-only
  captures may instead remain attached to the release.
- Dry-run, no-port, `NOT_RUN`, and empty-payload outputs are temporary review
  aids, not HIL evidence. Do not commit them unless a maintainer explicitly
  requires one for an active audit.
- Hardware logs must name the board, sensor/coil, address strap, channel count,
  firmware profile, firmware-reported Git commit/status, host checkout, and
  operator.
- Simulation, native tests, and PlatformIO/CI builds are useful software
  evidence, but they are not hardware validation.
