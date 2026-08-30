# Register Map

## Address Summary

| Address | Register | Reset | Notes |
|---:|---|---:|---|
| `0x00`/`0x01` | `DATA0_MSB` / `DATA0_LSB` | `0x0000` | Channel 0 result and MSB error flags. |
| `0x02`/`0x03` | `DATA1_MSB` / `DATA1_LSB` | `0x0000` | Channel 1 result and MSB error flags. |
| `0x04`/`0x05` | `DATA2_MSB` / `DATA2_LSB` | `0x0000` | LDC1614 channel 2 only. |
| `0x06`/`0x07` | `DATA3_MSB` / `DATA3_LSB` | `0x0000` | LDC1614 channel 3 only. |
| `0x08`-`0x0B` | `RCOUNT0`-`RCOUNT3` | `0x0080` | Conversion interval; channels 2/3 are LDC1614 only. |
| `0x0C`-`0x0F` | `OFFSET0`-`OFFSET3` | `0x0000` | Frequency offset; channels 2/3 are LDC1614 only. |
| `0x10`-`0x13` | `SETTLECOUNT0`-`SETTLECOUNT3` | `0x0000` | Sensor settling count; channels 2/3 are LDC1614 only. |
| `0x14`-`0x17` | `CLOCK_DIVIDERS0`-`CLOCK_DIVIDERS3` | `0x0000` | Input/reference dividers; channels 2/3 are LDC1614 only. |
| `0x18` | `STATUS` | `0x0000` | Error, data-ready, and unread-conversion flags. |
| `0x19` | `ERROR_CONFIG` | `0x0000` | Routes errors to output registers, `STATUS`, and `INTB`. |
| `0x1A` | `CONFIG` | `0x2801` | Active channel, sleep, clock, INTB, current behavior. |
| `0x1B` | `MUX_CONFIG` | `0x020F` | Auto-scan sequence and deglitch bandwidth. |
| `0x1C` | `RESET_DEV` | `0x0000` | Software reset trigger. |
| `0x1E`-`0x21` | `DRIVE_CURRENT0`-`DRIVE_CURRENT3` | `0x0000` | Sensor drive current; channels 2/3 are LDC1614 only. |
| `0x7E` | `MANUFACTURER_ID` | `0x5449` | Read-only manufacturer ID. |
| `0x7F` | `DEVICE_ID` | `0x3055` | Read-only device ID. |

Source: datasheet, pp. 15-31.

## Result Registers

`DATAx_MSB` bits 15:12 report per-channel under-range, over-range, watchdog,
and amplitude error flags when enabled through `ERROR_CONFIG`. Bits 11:0 are
the top 12 bits of the 28-bit conversion result. `DATAx_LSB` holds the lower
16 bits. Source: datasheet, pp. 16-19.

Always read `DATAx_MSB` first. Source: datasheet, pp. 16-19, 38.

## Timing Registers

`RCOUNTx` values below `0x0005` are not valid for normal conversion timing.
`SETTLECOUNTx` values `0x0000` and `0x0001` have special short-settle meanings;
`0x0002` through `0xFFFF` scale with the channel reference frequency. Source:
datasheet, pp. 20, 22-23. Multi-channel operation applies tighter Table 43
minimums; see
[Electrical and timing notes](03_electrical_and_timing.md#conversion-timing).

`CLOCK_DIVIDERSx` fields:

| Field | Bits | Purpose |
|---|---:|---|
| `FIN_DIVIDERx` | 15:12 | Divides sensor input frequency before measurement. |
| Reserved | 11:10 | Preserve documented reserved behavior. |
| `FREF_DIVIDERx` | 9:0 | Divides the selected reference clock for channel timing. |

Source: datasheet, pp. 23-24.

## Control Registers

`CONFIG` includes `ACTIVE_CHAN`, `SLEEP_MODE_EN`, `RP_OVERRIDE_EN`,
`SENSOR_ACTIVATE_SEL`, `AUTO_AMP_DIS`, `REF_CLK_SRC`, `INTB_DIS`, and
`HIGH_CURRENT_DRV`. `RP_OVERRIDE_EN = 1` makes the conversion use the
programmed `IDRIVEx` value instead of the automatically determined drive
current. Source: datasheet, pp. 28-29.

`MUX_CONFIG` includes `AUTOSCAN_EN`, `RR_SEQUENCE`, reserved bits that must be
set per the datasheet, and `DEGLITCH`. Source: datasheet, pp. 29-30.

`RESET_DEV.RESET_DEV` triggers a device reset when written. Source: datasheet,
p. 31.

`DRIVE_CURRENTx` contains `IDRIVEx` and `INIT_IDRIVEx`; when writing
`DRIVE_CURRENTx`, the lower reserved field is documented as zero. Source:
datasheet, pp. 31-33.

## Pre-Revision-A Name Map

The retained vendor application notes and how-to guides predate datasheet
revision A and use the older `CHx_NAME` / `NAME_CHx` identifiers. Revision A
renamed them; addresses and bit positions are unchanged. The revision-A names
are the ones used by the datasheet register tables and by
`include/LDC1614/CommandTable.h`.

| Pre-revision-A name | Revision-A name |
|---|---|
| `DATA_MSB_CHx` / `DATA_LSB_CHx` / `DATA_CHx` | `DATAx_MSB` / `DATAx_LSB` |
| `CHx_ERR_UR` / `CHx_ERR_OR` / `CHx_ERR_WD` / `CHx_ERR_AE` | `ERR_URx` / `ERR_ORx` / `ERR_WDx` / `ERR_AEx` |
| `RCOUNT_CHx`, `CHx_RCOUNT` | `RCOUNTx` |
| `OFFSET_CHx`, `CHx_OFFSET` | `OFFSETx` |
| `SETTLECOUNT_CHx`, `CHx_SETTLECOUNT` | `SETTLECOUNTx` |
| `CLOCK_DIVIDERS_CHx`, `CHx_FIN_DIVIDER`, `CHx_FREF_DIVIDER` | `CLOCK_DIVIDERSx`, `FIN_DIVIDERx`, `FREF_DIVIDERx` |
| `CHx_UNREADCONV` | `UNREADCONVx` |
| `DRIVE_CURRENT_CHx`, `CHx_IDRIVE`, `CHx_INIT_IDRIVE` (`CHx_INIT_DRIVE` in SNAA221B) | `DRIVE_CURRENTx`, `IDRIVEx`, `INIT_IDRIVEx` |

Source: datasheet, p. 2 (Revision History).
