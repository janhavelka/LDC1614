# Electrical and Timing Notes

## Operating Limits

| Parameter | Value | Source |
|---|---:|---|
| Supply voltage, `VDD` | 2.7 V to 3.6 V | datasheet, p. 5 |
| Sensor oscillation frequency | 1 kHz to 10 MHz | datasheet, pp. 1, 6 |
| External `CLKIN` frequency | 2 MHz to 40 MHz | datasheet, p. 6 |
| I2C clock frequency | 10 kHz to 400 kHz | datasheet, p. 7 |
| Sensor `RP` operating range | 1 kohm to 100 kohm | datasheet, p. 6 |
| Operating junction temperature in abs-max table | -55 degC to +150 degC | datasheet, p. 5 |

The datasheet feature summary lists 2.7 V to 3.6 V operation, 35 uA low-power
sleep mode, and 200 nA shutdown mode. Detailed current values vary by mode,
clock, sensor current, and whether I2C pullup current is included. Source:
datasheet, pp. 1, 6.

## I2C Timing

The device uses standard I2C transactions up to 400 kbit/s and does not use I2C
clock stretching. Registers are 16 bits wide and the documented read sequence
uses a repeated start between pointer write and data read. Source: datasheet,
p. 13.

The I2C timing table gives the bus timing requirements and is the source to use
for setup/hold timing in a low-level transport test. Source: datasheet, p. 7.

## Conversion Timing

`RCOUNTx` controls conversion interval. Valid programmed values are
`0x0005` through `0xFFFF`; larger conversion times improve resolution, and
`0xFFFF` is required for full resolution. Source: datasheet, pp. 20, 39.

The application section gives conversion time as a function of `RCOUNTx` and
the channel reference frequency. The register-field tables express the same
relationship as a multiple of 16 reference-clock periods, while the application
text includes a small additional term. Keep the driver configurable and do not
hide this distinction in a hard-coded delay. Source: datasheet, pp. 20, 39.

`SETTLECOUNTx` controls sensor activation/settling before conversion. It must be
long enough for sensor amplitude to settle; otherwise amplitude errors can be
generated if those errors are enabled. Source: datasheet, pp. 22-23, 40.

The datasheet gives a minimum settle-count relationship based on sensor Q,
sensor frequency, and reference frequency. Source: datasheet, p. 40.

## Clock and Divider Constraints

Each channel has `FIN_DIVIDERx` and `FREF_DIVIDERx` fields in
`CLOCK_DIVIDERSx`. The dividers derive the measured input frequency and channel
reference frequency. Source: datasheet, pp. 23-24, 45-46.

For external clock operation, the datasheet application table requires the
reference source and divided frequencies to stay within documented ranges; if
the sensor frequency is at least 8.75 MHz, `FIN_DIVIDERx` must be at least 2.
Source: datasheet, p. 46.

## Deglitch Filter

`MUX_CONFIG.DEGLITCH` selects the input deglitch bandwidth. Choose the lowest
bandwidth above the highest sensor oscillation frequency across enabled
channels. Documented choices include 1.0 MHz, 3.3 MHz, 10 MHz, and 33 MHz.
Source: datasheet, pp. 29-30, 46.

The register field text and application table disagree on the code for the
33 MHz setting (`b111` vs `b011`). This conflict is recorded in
`08_variant_differences_and_open_questions.md`.
