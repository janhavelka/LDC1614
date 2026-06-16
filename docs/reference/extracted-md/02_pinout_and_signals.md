# Pinout and Signals

## Packages

LDC1612 uses a WSON-12 package. LDC1614 uses a WQFN-16 package. Both package
drawings are top-view in the datasheet. Source: datasheet, p. 4.

The exposed die attach pad has an internal electrical connection to GND. TI
allows it to float, but recommends connecting it to the same potential as GND
for best performance. It must not be used as the device's primary ground.
Source: datasheet, p. 4.

## Common Control and Supply Pins

| Pin | Function | Driver note |
|---|---|---|
| `SCL` | I2C clock, open-drain, requires pullup. Source: datasheet, p. 4. | Transport code supplies clocking; library code should not own the bus. |
| `SDA` | I2C data, open-drain, requires pullup. Source: datasheet, p. 4. | 16-bit register reads use a repeated start. |
| `CLKIN` | External reference clock input. Tie to GND when the internal oscillator is used. Source: datasheet, p. 4. | `CONFIG.REF_CLK_SRC` selects internal vs external reference source. |
| `ADDR` | I2C address select: low = `0x2A`, high = `0x2B`; do not float. Source: datasheet, pp. 4, 13. | Address may be changed while the device is in shutdown. |
| `INTB` | Push-pull configurable interrupt output; no pullup required. Source: datasheet, p. 4. | Asserted by enabled status events unless `CONFIG.INTB_DIS` disables it. |
| `SD` | Shutdown input: low = normal/sleep register access, high = shutdown; do not float. Source: datasheet, pp. 4, 13. | I2C is unavailable in shutdown. |
| `VDD` | 2.7 V to 3.6 V supply range. Source: datasheet, pp. 5, 10. | Place bypassing close to VDD/GND. |
| `GND` | Device ground. Source: datasheet, p. 4. | Must always be connected; DAP is not a substitute. |

## Sensor Inputs

| Channel | Pins | Availability |
|---:|---|---|
| 0 | `IN0A`, `IN0B` | LDC1612 and LDC1614 |
| 1 | `IN1A`, `IN1B` | LDC1612 and LDC1614 |
| 2 | `IN2A`, `IN2B` | LDC1614 only |
| 3 | `IN3A`, `IN3B` | LDC1614 only |

Source: datasheet, p. 4.

Sensor inputs connect to external LC tanks. The datasheet absolute maximum table
limits current into any `INx` pin to +/-8 mA; normal operation depends on the
configured drive-current fields and the external sensor. Source: datasheet,
pp. 5, 42-44.

## Signal Behavior Notes

- `INTB` can indicate data-ready and selected error events. Reading `STATUS` or
  the relevant `DATAx_MSB` clears status/INTB behavior as documented for the
  selected event. Source: datasheet, pp. 25-28, 47.
- `SD` high enters the lowest-power shutdown state, clears active conversion and
  error state, returns registers to default values, and prevents I2C access.
  Source: datasheet, p. 13.
- Sleep mode is not the same as shutdown: sleep keeps I2C active, retains
  configuration, and stops conversions. Source: datasheet, p. 12.
