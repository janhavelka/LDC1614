# Chip Overview

## Device Identity

The LDC1612 and LDC1614 are multi-channel 28-bit inductance-to-digital
converters for inductive sensing. LDC1612 provides two channels; LDC1614
provides four channels. Both devices use an I2C control/data interface and are
intended for LC sensors whose oscillation frequency is in the 1 kHz to 10 MHz
range. Source: datasheet, p. 1.

The devices are designed for remote inductive sensors, environmental and aging
compensation across channels, and sensing ranges beyond two coil diameters.
Source: datasheet, p. 1.

## Driver-Relevant Capabilities

| Area | Fact |
|---|---|
| Resolution | Conversion result is 28 bits, split across `DATAx_MSB` and `DATAx_LSB`. Source: datasheet, pp. 16, 38. |
| Channels | Channel 0 and 1 exist on LDC1612/LDC1614; channel 2 and 3 are LDC1614 only. Source: datasheet, pp. 4, 15. |
| Interface | I2C register interface, maximum 400 kbit/s, 7-bit address selected by `ADDR`. Source: datasheet, p. 13. |
| Clocking | Uses internal oscillator by default or external reference clock on `CLKIN`; feature summary lists 40 MHz external clock support. Source: datasheet, pp. 1, 45-46. |
| Power modes | Sleep mode keeps I2C active; shutdown through `SD` disables I2C. Source: datasheet, pp. 12-13. |
| Modes | Supports single-channel continuous conversion and multi-channel auto-scan. Source: datasheet, pp. 12, 29. |
| Status | Can report conversion data ready, unread conversions, under-range, over-range, watchdog, amplitude, and zero-count conditions. Source: datasheet, pp. 25-28, 47. |

## Measurement Model

Each channel measures a sensor oscillation frequency against a reference
frequency. The raw `DATAx` code represents the ratio of sensor frequency to
reference frequency after the configured input/reference dividers. Source:
datasheet, pp. 11, 38, 45-46.

For coherent reads, read `DATAx_MSB` first and then `DATAx_LSB` for the same
channel. The MSB register carries error flags in bits 15:12 and the upper 12
bits of the 28-bit result in bits 11:0. Source: datasheet, pp. 16, 38.

`OFFSETx` can compensate a frequency offset and is part of the frequency
calculation. Keep any driver helper explicit about whether it returns raw
28-bit data, offset-adjusted frequency, or user-level inductance; the datasheet
defines raw/frequency relationships, not a complete application calibration
model. Source: datasheet, p. 39.

## Sensor and System Context

Inductive sensing detects target-induced changes in the LC tank. Conductive
targets create eddy currents that change the apparent inductance and parallel
resistance of the sensor. Source: datasheet, pp. 34-35; sensor design app note,
pp. 2-5.

The sensor drive current should be chosen so the sensor amplitude remains in
the intended operating range. The datasheet warns that too little current
increases noise or can stop oscillation, while too much current can activate
internal clamps and shift the measured frequency. Source: datasheet, pp. 42-44.

For repeatable high-resolution measurements, TI recommends fixed drive current
after characterization for most applications; automatic amplitude correction can
change drive current between conversions and introduce output offsets. Source:
datasheet, pp. 43-44; sensor configuration settings app note, pp. 3-5.
