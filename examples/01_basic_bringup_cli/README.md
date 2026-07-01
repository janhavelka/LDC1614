# LDC1614 Arduino Diagnostic Bring-up CLI

This example is diagnostic bring-up firmware for Arduino/PlatformIO targets. It
is not a production bus manager, production integration template, or hardware
validation claim.

The example owns Arduino `Wire`, serial input, board pins, diagnostic logging,
and cooperative timing hooks only as example glue. The framework-neutral driver
core still receives I2C, timing, GPIO, and recovery behavior through `Config`.

Production applications must own:
- I2C bus lifecycle, pull-ups, timeouts, and recovery policy.
- External serialization of all driver calls and transport access.
- INTB, SD, reset, and task scheduling policy.
- Sensor coil, IDRIVE, distance/material interpretation, and calibration.
- Captured hardware/fault/soak validation logs for the target board.

The shared Arduino CLI includes diagnostic raw register commands:
- `reg <addr>` reads a tracked register after `begin()`.
- `wreg <addr> <val>` writes a tracked register and can desynchronize cached
  configuration.
- `rawreg <reg> [addr]` and `rawwreg <reg> <val> [addr]` are pre-`begin()`
  service escape hatches.

After any diagnostic write, check `hardwareConfigDirty()` and run
`syncConfig()`, `recover()`, `resetAndReapply()`, or a fresh `begin()` before
trusting cached configuration-dependent behavior.
