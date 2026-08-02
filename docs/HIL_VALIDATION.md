# LDC1614 Hardware-in-the-Loop Validation

This document defines the HIL procedure and evidence expected before release or
field-readiness claims. Reviewed repository evidence is indexed under
[`docs/reports/`](https://github.com/janhavelka/LDC1614/blob/main/docs/reports/README.md).

Use:

```sh
python -m pip install --requirement requirements-dev.txt
python tools/ldc1614_hil_runner.py --profile arduino --port COM7 --baud 115200 --operator "<name>" --board "<exact board/fixture>" --expected-firmware-commit "<flashed Git SHA>" --json-out hil.json --raw-transcript-out hil.serial.txt
```

For a board with the LDC1614 chip present but no LC sensor/coil attached, use
the no-sensor fixture matrix:

```sh
python tools/ldc1614_hil_runner.py --profile arduino --fixture no-sensor --port COM7 --baud 115200 --operator "<name>" --board "<exact board/fixture>" --expected-firmware-commit "<flashed Git SHA>" --json-out hil-no-sensor.json --raw-transcript-out hil-no-sensor.serial.txt
```

This v3 mode exercises identity, selected register reads, cached profile and
progress, sleep/wake, initialization, full apply, reset/reapply, one
status-aware channel acquisition, invalidation/re-initialization, a diagnostic
write followed by full replay, end/bind/re-initialization, cancellation while
idle, and pure timing/frequency helpers. For every asynchronous job the runner
requires the scheduled operation ID to match a successful terminal
`OperationResult`; the immediate `IN_PROGRESS` status is never a pass. With no
LC sensor, acquisition validates transport/protocol/error reporting only. It
does not validate conversion accuracy, channel physics, sample rate, or drive
current.

To stress the same no-sensor matrix repeatedly in one captured run, add
`--repeat-command-set N`. The runner records both the base command count and
the expanded command count in the artifact.

For a time-bounded no-sensor soak, request the duration explicitly. The soak
keeps the port open, executes only complete cycles, and ends every cycle awake:

```sh
python tools/ldc1614_hil_runner.py --profile arduino --fixture no-sensor --port COM7 --operator "<name>" --board "<exact board/fixture>" --expected-firmware-commit "<flashed Git SHA>" --include-long-soak --soak-duration-s 3600 --soak-cycle-delay-s 1 --json-out hil-soak.json --raw-transcript-out hil-soak.serial.txt
```

The fixed soak cycle is `version`, `probe`, `status`, `sleep`, `wake`, and
`drv`. Every `drv` response must prove `bound=1` and
`applied=APPLIED_ACTIVE`. The summary records requested/actual duration,
complete cycles, command counts, failures, ambiguous responses, unexpected
startup banners, and worst command latency. A serial exception, unexpected
reset, or command failure is not converted into a pass. The explicit no-sensor
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
| `arduino` | `examples/01_basic_bringup_cli` | `help`, `version`, `scan`, `busrecover`, `init`, `probe`, `drv`, `cfg`, `progress`, `status`, `sleep`, `wake`, `drdy`, `timing 0x01`, `selftest` |
| `arduino --fixture no-sensor` | `examples/01_basic_bringup_cli` with chip but no LC sensor | Identity/registers, state/progress, init/apply/reset/reapply/acquire correlation, invalidation/replay, diagnostic write/replay, end/bind, sleep/wake, timing/frequency |
| `idf` | `examples/esp_idf/basic` | `help`, `version`, `scan`, `busrecover`, `init`, `probe`, `drv`, `cfg`, `progress`, `status`, `sleep`, `wake`, `ready`, `timing 0x01`, `selftest` |

The runner is configurable. Use `--command` for board-specific commands and
`--skip-default-commands` when validating custom firmware.
Use `--expect-token`, `--failure-token`, and `--expected-failure-token` only for
documented fixture-specific cases. Expected-failure tokens are intended for
negative tests such as proving an invalid channel is rejected; default failure
classification remains strict.

Both CLIs expose cooperative jobs for interactive use. The runner handles those
commands as two-phase responses and requires matching scheduled/terminal IDs.
A sensor-acceptance fixture must additionally validate physical results and
cadence. Neither CLI exposes runtime address changes, production stress, or a
sample-rate benchmark.

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
- `--include-stress` and `--sample-rate-count` are recorded as skipped because
  the v3 diagnostic CLIs intentionally provide no production-cadence loop.
  Use a bounded fixture application that owns scheduling and correlates every
  operation ID/result instead of treating a diagnostic CLI loop as production
  evidence.
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
| Probe/device ID | Yes | LDC1612/LDC1614 board | Post-v3 COM8 runs reached valid identity reads, but every retained candidate run failed overall transport acceptance | Clean exact-revision positive probe/read artifact for each fixture |
| Address `0x2A` | Yes | ADDR strapped low | Post-v3 COM8 negative artifacts confirm the chip at `0x2A`; no positive candidate run | Clean probe/read logs for each production board |
| Address `0x2B` | No | ADDR strapped high or selectable | Not run | Opt-in probe/read logs at `0x2B` |
| LDC1612 channel bounds | Yes if LDC1612 present | LDC1612 hardware | Native tests only | HIL showing channels 0/1 valid and 2/3 rejected |
| LDC1614 channel config 0..3 | Yes | LDC1614 hardware | Some post-v3 initialization jobs completed, but retained runs failed overall transport acceptance | Clean cooperative initialization/readback on target variant |
| LDC1614 sensor reads 0..3 | Yes if channels populated | LDC1614 hardware/sensors | Not run, no sensor attached | Safe reads for channels 0..3 |
| Safe raw read per enabled channel | Yes | Sensors connected | Not run | Raw/read transcript with DATA error flags checked |
| Config readback | Yes | Hardware | Post-v3 negative runs include successful individual reads but no accepted complete candidate run | Repeat cleanly on target board variant |
| Reset/reapply and owner recovery | Yes | Hardware | Historical v2 does not validate v3 ownership contract | Correlated v3 job result plus owner recovery/replay trace |
| Deadline/cancel/result identity | Yes | Hardware | Native tests only | Correlated operation IDs and bus-silent deadline/cancel trace |
| INTB behavior | No | INTB wired/observable | Not run | Active-low push-pull behavior logs or analyzer capture |
| SD shutdown/wake | No | SD wired/controlled | Not run | Shutdown/wake transcript and current/identity behavior |
| Induced address NACK | No | Operator/fault fixture | Not run | Controlled NACK transcript with precise status |
| Unplug/replug | No | Operator/fault fixture | Not run | Failure, recovery, and post-recovery read logs |
| Stuck bus | No | Test fixture | Not run | Bounded timeout/recovery logs |
| Bounded soak | No | Stable fixture | No accepted positive exact-revision run | Duration, complete cycles, command/unknown/reset counts, worst latency |
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
