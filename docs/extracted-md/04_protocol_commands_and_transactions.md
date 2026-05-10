# Protocol, Commands, and Transactions

## Bus Model

The LDC1612/LDC1614 exposes 16-bit registers over I2C. The bus uses a 7-bit
slave address selected by `ADDR`: `0x2A` when `ADDR` is low and `0x2B` when
`ADDR` is high. Source: datasheet, p. 13.

The device supports I2C up to 400 kbit/s and does not use clock stretching.
Source: datasheet, p. 13.

There are no separate command opcodes in the datasheet. Public driver methods
should be expressed as typed register operations and higher-level configuration
helpers, not as an invented command layer.

## Register Writes

A register write sends the I2C address with write bit, an 8-bit register pointer,
then the 16-bit register value. The datasheet diagrams show MSB first for the
register value. Source: datasheet, p. 13.

Where a register contains both read-only and read/write fields, preserve or
write only documented writable fields as appropriate. The datasheet revision
history specifically notes added instructions for registers with both `R` and
`R/W` fields. Source: datasheet, p. 2.

## Register Reads

A register read writes the 8-bit register pointer, issues a repeated start, then
reads the 16-bit register value. Source: datasheet, p. 13.

For conversion results, always read `DATAx_MSB` before `DATAx_LSB` for the same
channel. The device uses an internal shadow register so that this sequence gives
coherent data. Reading only `DATAx_LSB` does not update the I2C-facing `DATAx`
field. Source: datasheet, pp. 14, 38, 47.

## Data Result Assembly

`DATAx_MSB` bits 15:12 are channel error flags. `DATAx_MSB` bits 11:0 contain
result bits 27:16. `DATAx_LSB` contains result bits 15:0. Source: datasheet,
pp. 16-19, 38.

Assemble raw data as:

```text
raw28 = ((DATAx_MSB & 0x0FFF) << 16) | DATAx_LSB
```

A raw value of `0x0000000` indicates under-range; `0x0FFFFFFF` indicates
over-range. Source: datasheet, p. 38.

## I2C Waveform Caveat

The datasheet says the interface is not suitable for an I2C system that supports
early transaction termination. It also warns against an extraneous SDA pulse
during the SCL-low period of the first bit of the I2C address byte. Source:
datasheet, p. 14.

## Shutdown and Address Changes

I2C is unavailable while `SD` is high. The `ADDR` pin may be changed while the
device is in shutdown to select the alternate address. Source: datasheet,
pp. 13-14.
