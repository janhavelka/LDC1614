# LDC1614 Hardware-in-the-Loop Validation

No hardware validation is recorded by this document. It defines the procedure
and evidence expected before release or field-readiness claims.

Use:

```sh
python tools/ldc1614_hil_runner.py --profile arduino --port COM7 --baud 115200 --json-out hil.json --markdown-out hil.md
```

If no serial port and real LDC1614/LDC1612 hardware are supplied, the runner
reports `NOT_RUN`. It must not be interpreted as a pass.

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
| `arduino` | `examples/01_basic_bringup_cli` | `help`, `version`, `scan`, `probe`, `id`, `drv`, `cfg`, `status`, `drdy`, `read`, `recover`, `timing 0 43000000`, `selftest` |
| `idf` | `examples/esp_idf/basic` | `help`, `version`, `probe`, `drv`, `cfg`, `status`, `ready`, `read`, `readall`, `recover`, `timing 0 43000000`, `selftest` |

The runner is configurable. Use `--command` for board-specific commands and
`--skip-default-commands` when validating custom firmware.
Use `--expect-token`, `--failure-token`, and `--expected-failure-token` only for
documented fixture-specific cases. Expected-failure tokens are intended for
negative tests such as proving an invalid channel is rejected; default failure
classification remains strict.

The IDF diagnostic CLI does not currently expose `wake`, `sleep`, `scan`,
`probeaddr`, or stress commands. Its `read` / `readall` commands are useful as
bounded I2C/data-register smoke checks, but live-conversion evidence requires
firmware that starts conversions for the tested configuration.

## Safe Default Procedure

1. Record operator, board, firmware, Git commit, serial port, baud, expected I2C
   address, channel count, and timestamp.
2. Open the serial port and capture startup output.
3. Run the selected profile's safe commands.
4. Classify each command from the transcript. Device ID/probe/read failures are
   failures, not skips.
5. Write JSON and Markdown artifacts with the full transcript.

## Optional Opt-in Procedure

Run these only when hardware and operator setup explicitly support them:
- Address `0x2B` strap/probe with `--include-address-0x2b`.
- Short diagnostic stress/soak with `--include-stress --stress-count N` on the
  Arduino profile.
- Bounded sample-rate smoke command with `--sample-rate-count N` on the Arduino
  profile; the runner appends `read <channel> <N>` and records elapsed time.
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
| Probe/device ID | Yes | LDC1612/LDC1614 board | Not yet run | Runner JSON/Markdown transcript showing expected IDs |
| Address `0x2A` | Yes | ADDR strapped low | Not run | Probe/read logs at `0x2A` |
| Address `0x2B` | No | ADDR strapped high or selectable | Not run | Opt-in probe/read logs at `0x2B` |
| LDC1612 channel bounds | Yes if LDC1612 present | LDC1612 hardware | Native tests only | HIL showing channels 0/1 valid and 2/3 rejected |
| LDC1614 channels 0..3 | Yes if channels populated | LDC1614 hardware/sensors | Native tests only | Safe reads for channels 0..3 |
| Safe raw read per enabled channel | Yes | Sensors connected | Not run | Raw/read transcript with DATA error flags checked |
| Config readback | Yes | Hardware | Not run | RCOUNT/SETTLECOUNT/CLOCK_DIVIDERS/DRIVE_CURRENT/OFFSET/CONFIG readback |
| INTB behavior | No | INTB wired/observable | Not run | Active-low push-pull behavior logs or analyzer capture |
| SD shutdown/wake | No | SD wired/controlled | Not run | Shutdown/wake transcript and current/identity behavior |
| Induced address NACK | No | Operator/fault fixture | Not run | Controlled NACK transcript with precise status |
| Unplug/replug | No | Operator/fault fixture | Not run | Failure, recovery, and post-recovery read logs |
| Stuck bus | No | Test fixture | Not run | Bounded timeout/recovery logs |
| Bounded soak | No | Stable fixture | Not run | Duration, command count, failure count, recovery count |
| Drive-current tuning | No | Sensor/oscilloscope/procedure | Not run | IDRIVE setting, amplitude evidence, application calibration notes |

## Evidence Rules

- JSON and Markdown outputs must be retained with the release artifacts.
- Hardware logs must name the board, sensor/coil, address strap, channel count,
  firmware profile, Git commit, and operator.
- Simulation, native tests, and PlatformIO/CI builds are useful software
  evidence, but they are not hardware validation.
