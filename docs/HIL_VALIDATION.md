# LDC1614 Hardware-in-the-Loop Validation

This document defines the HIL procedure and evidence expected before release or
field-readiness claims. Captured run artifacts live under `docs/reports/`.

Use:

```sh
python -m pip install --requirement requirements-dev.txt
python tools/ldc1614_hil_runner.py --profile arduino --port COM7 --baud 115200 --operator "<name>" --board "<exact board/fixture>" --expected-firmware-commit "<flashed Git SHA>" --json-out hil.json --markdown-out hil.md
```

For a board with the LDC1614 chip present but no LC sensor/coil attached, use
the no-sensor fixture matrix:

```sh
python tools/ldc1614_hil_runner.py --profile arduino --fixture no-sensor --port COM7 --baud 115200 --operator "<name>" --board "<exact board/fixture>" --expected-firmware-commit "<flashed Git SHA>" --json-out hil-no-sensor.json --markdown-out hil-no-sensor.md
```

This v3 mode exercises identity, selected safe register reads, cached profile
reporting, cooperative progress reporting, and sleep/wake. It intentionally
excludes acquisition, analog configuration mutation, reset/reapply, stress,
and sample-rate claims. Those require a sensor-aware fixture and a command
protocol that correlates the asynchronous terminal `OperationResult`, not just
the immediate command prompt.

To stress the same no-sensor matrix repeatedly in one captured run, add
`--repeat-command-set N`. The runner records both the base command count and
the expanded command count in the artifact.

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

## No Hardware Attached Audit

When no board with an LDC1614/LDC1612 is attached, do not run hardware commands
against an arbitrary serial port. Use the runner in no-port or dry-run mode to
produce software audit artifacts only:

```sh
python tools/ldc1614_hil_runner.py --profile arduino --dry-run --baud 115200 --operator "<name>" --board "no LDC1614/LDC1612 fixture attached" --note "no hardware audit only" --json-out docs/reports/hil-validation-COM8-YYYYMMDD.runner.json --markdown-out docs/reports/hil-validation-COM8-YYYYMMDD.runner.md --quiet
```

Dry-run artifacts list the planned bounded command sequence and are marked
`overall_status=NOT_RUN`, `hardware_attached=false`, and
`evidence_type=no_hardware_audit`. They are useful for review setup, parser
self-tests, and report traceability, but they are not HIL evidence and must not
be stored or described as pass logs.

## Firmware Profiles

| Profile | Intended firmware | Default safe commands |
| --- | --- | --- |
| `arduino` | `examples/01_basic_bringup_cli` | `help`, `version`, `scan`, `probe`, `drv`, `cfg`, `progress`, `status`, `sleep`, `wake`, `drdy`, `timing 0x01`, `selftest` |
| `arduino --fixture no-sensor` | `examples/01_basic_bringup_cli` with chip but no LC sensor | Identity, safe register reads, cached profile/progress, sleep/wake, timing; excludes conversion jobs |
| `idf` | `examples/esp_idf/basic` | `help`, `version`, `probe`, `drv`, `cfg`, `progress`, `status`, `sleep`, `wake`, `ready`, `timing 0x01`, `selftest` |

The runner is configurable. Use `--command` for board-specific commands and
`--skip-default-commands` when validating custom firmware.
Use `--expect-token`, `--failure-token`, and `--expected-failure-token` only for
documented fixture-specific cases. Expected-failure tokens are intended for
negative tests such as proving an invalid channel is rejected; default failure
classification remains strict.

Both CLIs expose cooperative jobs for interactive use, but the maintained HIL
runner's safe defaults intentionally use prompt-bounded commands only. A v3
sensor-acceptance fixture must capture the scheduled operation ID and matching
terminal result; an immediate `IN_PROGRESS` response is not a pass. The IDF
CLI does not expose scan, runtime address changes, stress, or sample-rate
commands.

## Safe Default Procedure

1. Record operator, board, firmware, Git commit, serial port, baud, expected I2C
   address, channel count, and timestamp.
2. Open the serial port and capture startup output.
3. Run the selected profile's safe commands.
4. Classify each command from the transcript. Ambiguous command output is
   `UNKNOWN`, not a pass. Maintained commands require command-specific output;
   arbitrary nonempty text and a bare `code=0` probe are failures. Device
   ID/probe/read failures are failures, not skips.
5. Write JSON and Markdown artifacts with command classification evidence.
   Repeated stress output may be condensed when metadata, command counts,
   per-base-command outcomes, firmware/device identity, and every non-pass
   detail remain available. Production acceptance must additionally retain at
   least one raw target transcript or logic-analyzer trace for the exact release
   fixture; a condensed summary is not a substitute for that raw artifact.

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
- Long soak.
- Drive-current/coil tuning with oscilloscope or an application-specific
  amplitude procedure.

## Validation Matrix

| Test | Safe default? | Requires hardware/operator? | Current evidence | Needed evidence |
| --- | --- | --- | --- | --- |
| Probe/device ID | Yes | LDC1612/LDC1614 board | Historical v2 COM8 chip-only evidence; no v3 run | Repeat at the exact v3 revision for each fixture |
| Address `0x2A` | Yes | ADDR strapped low | Historical v2 COM8 chip-only evidence; no v3 run | Probe/read logs for each production board |
| Address `0x2B` | No | ADDR strapped high or selectable | Not run | Opt-in probe/read logs at `0x2B` |
| LDC1612 channel bounds | Yes if LDC1612 present | LDC1612 hardware | Native tests only | HIL showing channels 0/1 valid and 2/3 rejected |
| LDC1614 channel config 0..3 | Yes | LDC1614 hardware | Historical v2 only | Repeat cooperative initialization/readback on target variant |
| LDC1614 sensor reads 0..3 | Yes if channels populated | LDC1614 hardware/sensors | Not run, no sensor attached | Safe reads for channels 0..3 |
| Safe raw read per enabled channel | Yes | Sensors connected | Not run | Raw/read transcript with DATA error flags checked |
| Config readback | Yes | Hardware | Historical v2 only | Repeat on target board variant |
| Reset/reapply and owner recovery | Yes | Hardware | Historical v2 does not validate v3 ownership contract | Correlated v3 job result plus owner recovery/replay trace |
| Deadline/cancel/result identity | Yes | Hardware | Native tests only | Correlated operation IDs and bus-silent deadline/cancel trace |
| INTB behavior | No | INTB wired/observable | Not run | Active-low push-pull behavior logs or analyzer capture |
| SD shutdown/wake | No | SD wired/controlled | Not run | Shutdown/wake transcript and current/identity behavior |
| Induced address NACK | No | Operator/fault fixture | Not run | Controlled NACK transcript with precise status |
| Unplug/replug | No | Operator/fault fixture | Not run | Failure, recovery, and post-recovery read logs |
| Stuck bus | No | Test fixture | Not run | Bounded timeout/recovery logs |
| Bounded soak | No | Stable fixture | Not run | Duration, command count, failure count, recovery count |
| Drive-current tuning | No | Sensor/oscilloscope/procedure | Not run | IDRIVE setting, amplitude evidence, application calibration notes |

## Evidence Rules

- JSON and Markdown outputs must be retained with the release artifacts. Compact
  summaries are acceptable for repeated stress runs when they preserve counts,
  command outcomes, and non-pass details.
- At least one raw serial transcript or logic-analyzer trace must be retained
  for production acceptance of the exact board, sensor, wiring, configuration,
  and release revision. No such raw artifact is currently committed.
- Hardware logs must name the board, sensor/coil, address strap, channel count,
  firmware profile, firmware-reported Git commit/status, host checkout, and
  operator.
- Simulation, native tests, and PlatformIO/CI builds are useful software
  evidence, but they are not hardware validation.
