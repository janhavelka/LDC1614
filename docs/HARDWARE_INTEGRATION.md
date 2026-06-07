# LDC1614/LDC1612 Hardware Integration Checklist

This checklist is a board/application integration aid. It is not hardware
validation evidence. Use the datasheet and captured board logs for release
decisions.

Local sources used:
- `docs/LDC1614_datasheet.pdf`
- `docs/extracted-md/01_chip_overview.md`
- `docs/extracted-md/02_pinout_and_signals.md`
- `docs/extracted-md/03_electrical_and_timing.md`
- `docs/extracted-md/04_protocol_commands_and_transactions.md`
- `docs/extracted-md/05_register_map.md`
- `docs/extracted-md/06_modes_interrupts_status_and_faults.md`
- `docs/extracted-md/07_initialization_reset_and_operational_notes.md`
- `docs/application_notes/sensor_configuration_settings.md`
- `docs/application_notes/sensor_status_monitoring.md`

## Device Selection

- LDC1612 is the 2-channel device. Configure `Config::channelCount = 2`; high
  level APIs reject channels 2 and 3.
- LDC1614 is the 4-channel device. Configure `Config::channelCount = 4` when
  all channels are populated and exercised on the target hardware.
- Both devices provide 28-bit conversion data. `DATAx_MSB` also contains
  error/status bits; do not treat a raw 32-bit `DATAx_MSB`/`DATAx_LSB` read as
  pure conversion data.

## Electrical Pins

| Pin / signal | Integration rule |
| --- | --- |
| VDD | 2.7 V to 3.6 V supply. Verify board supply tolerance, startup, ripple, and brownout behavior. |
| SCL/SDA | I2C lines require pull-ups sized for bus capacitance, rise time, selected bus speed, and voltage/level-shifting domain. Internal MCU pull-ups are diagnostic convenience only, not a production design assumption. The core driver never configures pull-ups. |
| ADDR | Low selects `0x2A`; high selects `0x2B`; do not let ADDR float. |
| SD | Low is normal operation; high is inactive/shutdown mode; do not let SD float. |
| CLKIN | Tie CLKIN to GND when using the internal oscillator. Provide and validate the external reference clock when `RefClkSrc::EXT_CLK` is used. |
| INTB | Configurable push-pull interrupt output. It is not open-drain by default and the IC does not require a pull-up for INTB. Board-specific conditioning may still exist. |

## Sensor and Timing Configuration

- Supported sensor frequency range is 1 kHz to 10 MHz. Coil + capacitor design
  must keep the operating range inside this limit across tolerance, temperature,
  target position, and material variation.
- RCOUNTx, SETTLECOUNTx, CLOCK_DIVIDERSx, and reference clock/dividers determine
  conversion and settling timing. Validate the sample interval for every enabled
  channel and auto-scan sequence.
- DRIVE_CURRENTx / IDRIVE tuning is application-specific. It may require an
  oscilloscope or a board-specific amplitude procedure, plus monitoring of
  amplitude warning/error flags.
- OFFSETx shifts conversion output; treat offset selection as part of the
  application calibration plan.
- Coil geometry, target distance, coating thickness, material detection, and
  inductance interpretation are not guaranteed by this library. They require
  application calibration and evidence.

## Firmware Integration Rules

- The application owns the I2C bus, pins, pull-ups, locking, timeout policy,
  reset policy, task scheduling, INTB wiring, and SD control.
- Inject transport callbacks through `Config::i2cWrite` and
  `Config::i2cWriteRead`; the core never owns Arduino `Wire`, ESP-IDF handles,
  bus pins, or global bus objects.
- Serialize all public driver calls externally. Driver instances are not
  internally thread-safe and public APIs are not ISR-safe.
- Prefer typed APIs for normal operation. Raw register writes are diagnostic and
  can desynchronize cached configuration; check `hardwareConfigDirty()` and run
  `syncConfig()`, `recover()`, `resetAndReapply()`, or `begin()` before trusting
  cached configuration again.
- Read conversion data in datasheet order: `DATAx_MSB` before `DATAx_LSB`.
- Use `readDataReady()` when the caller needs precise failure status.
  `dataReady()` is a convenience wrapper that returns `false` on transport or
  status-read failure.

## Hardware Evidence Required Before Release Claims

Capture logs for the exact board and sensor configuration:
- Device identity at `0x2A` and/or `0x2B`, depending on ADDR strap.
- Config readback for RCOUNT, SETTLECOUNT, CLOCK_DIVIDERS, DRIVE_CURRENT,
  OFFSET, MUX_CONFIG, ERROR_CONFIG, and CONFIG.
- Safe raw reads on every enabled channel.
- INTB behavior when wired.
- SD shutdown/wake behavior when wired.
- Unplug/replug, address NACK, timeout/fault paths, and recovery behavior.
- Bounded soak with the configured channel sequence and sensor/target setup.
