# LDC1614 HIL Validation - COM8 - 2026-07-01

## Result

PASS for no-sensor chip-only HIL on an ESP32-S2 board with an LDC1614 present at
I2C address `0x2A`.

This run did not include an LC sensor/tank and did not include physical
DRDY/INTB wiring. It validates chip identity, I2C register access, bounded CLI
command handling, configuration write/readback paths, reset/recovery commands,
and expected precondition/parameter failures that do not require a sensor.

## Fixture

- Operator: Codex
- Host port: `COM8`
- Target: ESP32-S2
- Device: LDC1614 at `0x2A`
- Fixture: no LC sensor attached
- DRDY/INTB: not wired
- Profile: Arduino `esp32s2dev`

## Firmware Under Test

The firmware flashed for the no-sensor stress run reported:

- Library version: `2.0.0`
- Full version: `2.0.0 (42feb3b, 2026-07-01 16:27:00, clean)`
- Git commit embedded in firmware: `42feb3b`
- Git status embedded in firmware: `clean`

The negative/precondition runner was launched after committing the first stress
artifact. Its runner metadata therefore records host commit `6c88592`, while
the target firmware still reports the same clean `42feb3b` build.

## Artifacts

- `hil-validation-COM8-20260701.no-sensor-stress.runner.json`
- `hil-validation-COM8-20260701.no-sensor-stress.runner.md`
- `hil-validation-COM8-20260701.no-sensor-negative-stress.runner.json`
- `hil-validation-COM8-20260701.no-sensor-negative-stress.runner.md`

## Executed Coverage

- 1010/1010 no-sensor chip-only commands passed.
- 101 base commands were repeated 10 times.
- 200/200 negative/precondition commands passed.
- 20 base negative/precondition commands were repeated 10 times.
- Expected failures included `BUSY` and `INVALID_PARAM` cases.
- Conversion checks, DRDY-gated sample-rate checks, and sensor-frequency checks
  were intentionally excluded because no sensor/tank was attached.

## Out Of Scope

The following still require a sensor-equipped fixture or additional wiring and
are not proven by this report:

- Sensor-attached live conversion quality.
- DATAx and STATUS behavior with a real LC tank.
- Physical DRDY/INTB behavior.
- SD-pin behavior.
- Address-pin variant at `0x2B`.
- Fault injection under real sensor fault conditions.
- Long soak or environmental stress.
- Sensor tuning, frequency, amplitude, and IDRIVE calibration limits.
