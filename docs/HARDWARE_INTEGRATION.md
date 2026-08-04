# LDC1614/LDC1612 hardware integration checklist

This is a board/application integration aid, not hardware-validation evidence.
Use the exact target schematic, the TI LDC1612/LDC1614 datasheet, and captured
target logs for a release decision.

## Freeze the hardware profile

`Config` is deliberately invalid by default. Before binding the driver, the
application must explicitly supply:

- `DeviceVariant::LDC1612` or `DeviceVariant::LDC1614`; the common identity
  registers cannot distinguish the variants;
- `I2cAddress::ADDR_GND` (`0x2A`) or `I2cAddress::ADDR_VDD` (`0x2B`);
- the populated `ChannelMask`, operating mode, active channel or round-robin
  sequence, known register values for every physical variant channel, and
  sensor-frequency bounds for every selected channel;
- the internal or external reference-clock frequency and tolerance; and
- the required readiness and error-reporting policy.

The LDC1612 supports channels 0 and 1. Channels 2 and 3 are LDC1614-only. Do
not infer populated channels from the part name or from a successful identity
read.

## Electrical checklist

| Signal | Integration rule |
| --- | --- |
| VDD | Supply 2.7 V to 3.6 V and verify tolerance, startup, ripple, and brownout behavior on the target. |
| SCL/SDA | Size pull-ups for bus capacitance, rise time, speed, and voltage domain. The application owns pins, pull-ups, bus setup, locking, transfer timeout, and recovery. |
| ADDR | Low selects `0x2A`; high selects `0x2B`. Do not let it float. |
| SD | Low is normal operation and high is shutdown. The application owns this pin and must invalidate the driver's applied-state belief after device power loss or reset. |
| CLKIN | Tie CLKIN to ground for the internal oscillator. Measure and configure the actual external clock when external CLKIN is used. |
| INTB | This is a configurable push-pull output. The application owns its GPIO setup and may inject a bus-silent asserted-state callback. Do not assume open-drain behavior or an internal driver pull-up. |

## Sensor and timing checklist

- Keep each LC sensor within the datasheet's 1 kHz to 10 MHz supported range
  across tolerance, temperature, target position, and material variation.
- Validate `RCOUNT`, `SETTLECOUNT`, FIN/FREF dividers, offset, deglitch, and
  drive-current code from the actual LC tank, Q/Rp, target amplitude, clock,
  resolution, and cadence requirements. Library validation catches known
  register and clock-plan contradictions; it cannot qualify a physical coil.
- In sequential mode, selected channels convert at different times. A returned
  batch is transactionally committed as one result, but it is not a
  simultaneous sensor frame. Budget channel settling/conversion, switch time,
  STATUS/DATA readout, clock tolerance, and application scheduling latency.
- Treat raw zero and `0x0FFFFFFF` as under-range and over-range endpoints even
  if device error reporting is disabled. Use the per-channel quality and batch
  masks rather than treating a decoded 28-bit value as automatically usable.
- Frequency conversion is a chip-level calculation, not calibration to
  inductance, displacement, level, coating thickness, or material identity.

## Firmware ownership and side effects

- The driver is framework-neutral and retains non-owning transport callbacks.
  It owns no bus, task, queue, lock, retry, backoff, GPIO, or recovery policy.
- Serialize all calls to one instance. Public APIs are not internally
  thread-safe and are not ISR-safe. An interrupt may notify the owner, but the
  owner task must call the driver.
- Start operations with a caller-owned ID and absolute deadline. Advance them
  with a caller-selected transport budget. A budget of zero performs no I2C.
  Cancellation and invalidation are bus-silent.
- After owner-observed removal, reset, brownout, shutdown, or shared-bus
  recovery, call `invalidateAppliedState()` and complete initialization or
  configuration replay before acquiring another trusted sample.
- Reading `DATAx_MSB` latches its corresponding LSB, consumes
  `UNREADCONVx`, and can clear the channel's latched error/INTB evidence.
  Reading STATUS captures then clears sticky status/INTB evidence. Production
  acquisition therefore preserves a STATUS snapshot before any DATA read and
  reports a later STATUS comparison for data-loss detection.
- A failed or cancelled write sequence can leave hardware partially changed or
  indeterminate. Inspect the terminal operation result and configuration fault;
  do not publish cached configuration as verified hardware state.
- Raw register access is advanced diagnostics. It must either be reconciled by
  an explicit readback procedure or followed by applied-state invalidation and
  full replay.

## Diagnostic CLI safety boundary

- Syntactic acceptance or pure validation of `refclk`, `sensorbounds`,
  `deglitch`, `drive`, `highcurrent`, and channel timing values does not qualify
  an LC tank, reference clock, target, or thermal/current limit.
- `profile commit confirm` updates desired cache only. A subsequent cooperative
  replay and readback are still not a substitute for electrical measurements.
- STATUS, DATA, destructive raw-register reads, and `dump all confirm` consume
  or alter silicon evidence. Use the retained pre/post STATUS and effect fields.
- A no-sensor self-test, protocol-qualified address discovery, configuration
  verify, or protocol stress run can
  validate bounded transport behavior and register protocol only. It cannot
  validate conversion accuracy, fresh sensor cadence, drive tuning, INTB/SD
  electrical behavior, or production application policy.

## Required physical evidence before release

Capture raw logs or a logic trace for the exact board, variant, address strap,
clock, channel mask, LC tanks, and firmware cadence:

- identity and full configuration readback;
- DATAx_MSB-before-DATAx_LSB ordering under active conversions;
- fresh values and expected physical sensor response on every selected channel;
- STATUS, UNREAD, error-channel, INTB, and delayed-read data-loss behavior;
- under-range, over-range, watchdog, zero-count, and amplitude conditions where
  electrically safe;
- deadline cancellation followed by a clean new operation;
- unplug/replug or controlled power loss followed by complete replay;
- owner-controlled shared-bus recovery while another device remains usable;
- SD and INTB behavior if those pins are populated; and
- a bounded soak at production settings and cadence.

The retained ESP32-S2 chip-only/no-sensor evidence does not satisfy these
sensor-attached target gates.
