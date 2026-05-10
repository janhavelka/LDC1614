# Initialization, Reset, and Operational Notes

## Minimal Driver Bring-Up

1. Hold or ensure `SD` low so the device is not in shutdown.
2. Use the configured I2C address selected by `ADDR` (`0x2A` or `0x2B`).
3. Read `MANUFACTURER_ID` (`0x5449`) and `DEVICE_ID` (`0x3055`) to verify bus
   presence and expected silicon.
4. Keep the device in sleep while programming channel timing, dividers,
   `ERROR_CONFIG`, `MUX_CONFIG`, and `DRIVE_CURRENTx`.
5. Clear sleep by writing `CONFIG.SLEEP_MODE_EN = 0` after configuration.
6. Read `STATUS` and `DATAx_MSB`/`DATAx_LSB` according to the selected mode and
   interrupt policy.

Sources: datasheet, pp. 12-15, 25-31, 38, 49-51.

## Reset Behavior

Writing `RESET_DEV.RESET_DEV` resets the device. Any active conversion stops
and registers return to their reset values. Source: datasheet, pp. 13, 31.

Entering shutdown with `SD` high also stops conversions, clears error state,
deasserts `INTB`, returns registers to defaults, and disables I2C access until
`SD` returns low. Source: datasheet, p. 13.

## Clock Configuration

Use the internal oscillator unless an external reference is required for better
stability or system synchronization. If using external clocking, drive `CLKIN`
and set `CONFIG.REF_CLK_SRC = 1`. Source: datasheet, pp. 4, 28, 45-46.

Choose `FREF_DIVIDERx` and `FIN_DIVIDERx` so the reference and divided sensor
frequencies stay within the datasheet ranges. The datasheet example for a
2.4 MHz sensor and 40 MHz external clock uses `FIN_DIVIDER0 = 1` and
`FREF_DIVIDER0 = 2`. Source: datasheet, pp. 45-51.

## Timing Configuration

Set `SETTLECOUNTx` from sensor Q, sensor frequency, and reference frequency.
Then choose `RCOUNTx` from the desired sample period after accounting for
settling time and channel-switch delay. Source: datasheet, pp. 39-41, 49-51.

The datasheet example for 1 kSPS operation with a 2.4 MHz sensor uses
`SETTLECOUNT0 = 0x000A`, `RCOUNT0 = 0x04D6`, and
`CLOCK_DIVIDERS0 = 0x1002`. Source: datasheet, pp. 49-51.

## Drive Current

For production measurements, prefer a fixed characterized `IDRIVEx` value when
repeatability matters. TI notes that automatic amplitude correction can choose
drive-current settings with poorer repeatability and can introduce output code
offsets when current changes. Source: datasheet, pp. 43-44; sensor configuration
settings app note, pp. 3-5.

The datasheet example sets `DRIVE_CURRENT0 = 0x9000` for its example sensor.
Do not copy this value blindly; it depends on sensor RP and desired amplitude.
Source: datasheet, pp. 49-51.

## Layout and Board Notes

Use local supply bypassing close to `VDD` and `GND`; the datasheet recommends a
1 uF X7R multilayer ceramic capacitor and additional capacitance if the supply
is physically distant. Source: datasheet, p. 53.

Route sensor traces as a closely coupled pair and avoid unnecessary loop area.
Sensor design and EMI application notes provide board-level guidance, but they
do not change the register behavior documented in the datasheet. Source:
datasheet, p. 54; sensor design app note, pp. 10-17; EMI app note, pp. 2-5.
