# Modes, Interrupts, Status, and Faults

## Power and Conversion Modes

| Mode | Entry | Behavior |
|---|---|---|
| Shutdown | `SD` high | Lowest-power state; no conversions; I2C unavailable; registers return to defaults; active errors and INTB are cleared. Source: datasheet, p. 13. |
| Sleep | `CONFIG.SLEEP_MODE_EN = 1` | I2C remains available; configuration is retained; conversions stop; conversion results/errors are cleared and INTB deasserts. Source: datasheet, p. 12. |
| Normal conversion | `SD` low and `CONFIG.SLEEP_MODE_EN = 0` | Device converts according to `CONFIG.ACTIVE_CHAN` or `MUX_CONFIG` auto-scan settings. Source: datasheet, pp. 12, 28-30. |

After leaving sleep, sensor activation for the first conversion begins after
`16384 / fINT` according to the datasheet text. Source: datasheet, p. 12.

## Channel Sequencing

Single-channel continuous mode uses `MUX_CONFIG.AUTOSCAN_EN = 0` and selects the
channel with `CONFIG.ACTIVE_CHAN`. Source: datasheet, pp. 28-30.

Auto-scan mode uses `MUX_CONFIG.AUTOSCAN_EN = 1` and `RR_SEQUENCE`:

| `RR_SEQUENCE` | Channel sequence |
|---:|---|
| `b00` | Ch0, Ch1 |
| `b01` | Ch0, Ch1, Ch2 on LDC1614 |
| `b10` | Ch0, Ch1, Ch2, Ch3 on LDC1614 |
| `b11` | Ch0, Ch1 |

Source: datasheet, p. 29.

## STATUS Register

| Field | Purpose |
|---|---|
| `ERR_CHAN` | Channel associated with the reported error. |
| `ERR_UR` | Under-range error since last status read. |
| `ERR_OR` | Over-range error since last status read. |
| `ERR_WD` | Watchdog timeout error since last status read. |
| `ERR_AHE` / `ERR_ALE` | Amplitude high/low error since last status read. |
| `ERR_ZC` | Zero-count error since last status read. |
| `DRDY` | New conversion result ready. |
| `UNREADCONV0`-`UNREADCONV3` | Per-channel unread conversion flags; channels 2/3 are LDC1614 only. |

Source: datasheet, pp. 25-26.

Reading `STATUS` clears error status bits and `ERR_CHAN`, and deasserts `INTB`.
Reading `DATAx_MSB` also affects the relevant data-ready/unread-conversion
state. Source: datasheet, pp. 25-26, 47.

## ERROR_CONFIG and INTB

`ERROR_CONFIG` controls whether under-range, over-range, watchdog, amplitude
high/low, zero-count, and data-ready conditions update output registers,
`STATUS`, and/or `INTB`. Source: datasheet, pp. 27-28.

`CONFIG.INTB_DIS = 1` disables the interrupt function and holds `INTB` high.
Source: datasheet, p. 29.

In auto-scan mode, when the last channel in the selected group completes, the
device pulls `INTB` low if data-ready reporting to INTB is enabled. Source:
datasheet, pp. 47-48.

## Data Overwrite and Unread Flags

The device stores conversion results in an internal buffer, but the I2C-visible
`DATAx` field updates when `DATAx_MSB` is read. If reads are delayed, older
conversion results can be overwritten by newer conversions. Monitor
`UNREADCONVx` flags to detect whether the application is keeping up. Source:
datasheet, pp. 14, 47-48.

## Fault Interpretation

Under-range and over-range can also be inferred from raw data extremes:
`0x0000000` for under-range and `0x0FFFFFFF` for over-range. Source: datasheet,
p. 38.

Amplitude-related errors usually point to a settle-time or drive-current
problem. TI describes the intended sensor amplitude range and warns that
insufficient or excessive drive current can degrade repeatability or stop valid
conversion. Source: datasheet, pp. 42-44.
