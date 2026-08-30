# LDC1312, LDC1314, LDC1612, LDC1614 Sensor Status Monitoring
**Source:** sensor_status_monitoring.pdf | **Doc #:** SNOA959 | **Pages:** 14

> **Naming note.** Register and field names below are quoted as SNOA959 writes them, which predates datasheet revision A. See the [pre-Revision-A name map](../extracted-md/05_register_map.md#pre-revision-a-name-map) for the current identifiers; addresses and bit positions are unchanged.

## Key Takeaways
- Three reporting mechanisms: DATA_CHx register error bits, STATUS register, and INTB pin assertion.
- Six error/warning conditions: Under-range, Over-range, Watchdog Timeout, Amplitude High, Amplitude Low, and Zero Count.
- STATUS register bits are **sticky** — cleared by reading STATUS, or by reading the DATAx_MSB register of the error channel; either read also de-asserts INTB (datasheet pp. 26, 47; SNOA959 Table 4, p. 10). SNOA959 p. 5 mentions only the STATUS read. Exception: CHx_UNREADCONV is not sticky.
- Use INTB reporting **in addition to** STATUS polling to avoid missing errors from multiple channels.
- DATA_CHx error bits are **not sticky** — cleared by next successful conversion or by reading the register.

## Summary
The LDC1612/LDC1614 provide comprehensive error and status reporting through three mechanisms. The DATA_CHx output registers embed error flags in the four MSBs alongside conversion data. The STATUS register (0x18) provides a global view of all error conditions with channel identification. The INTB pin provides hardware interrupt notification when errors or data-ready conditions occur.

Error reporting is highly configurable through the ERROR_CONFIG register (0x19). Each error type can be independently routed to the DATA output registers (via `*_ERR2OUT` bits) and/or to the STATUS register and INTB pin (via `*_ERR2INT` bits). The STATUS register bits are sticky — they latch on first occurrence and are cleared by reading STATUS or by reading the DATAx_MSB register of the channel named in ERR_CHAN (datasheet pp. 26, 47; SNOA959 Table 4, p. 10). If an error occurs on CH0 and is not cleared before CH1 errors, the CH1 error will be missed unless INTB-based reporting is also used (reading STATUS clears the error and re-arms INTB for the next event).

Data readiness can be detected via the DRDY bit in STATUS, the CHx_UNREADCONV bits for per-channel notification in multi-channel mode, or by polling at a calculated fixed interval when the LDC and MCU share a clock source.

## Technical Details

### Reporting Mechanism Summary

| Condition | DATA_CHx Reporting | STATUS Register | INTB Pin |
|---|---|---|---|
| Data Ready (DRDY) | N/A | Reported | Set DRDY_2INT=1 |
| Unread Conversion | N/A | Reported | N/A |
| Under-range Error | Set UR_ERR2OUT=1 | Reported | Set UR_ERR2INT=1 |
| Over-range Error | Set OR_ERR2OUT=1 | Reported | Set OR_ERR2INT=1 |
| Watchdog Timeout | Set WD_ERR2OUT=1 | Reported | Set WD_ERR2INT=1 |
| Amplitude High | Set AH_ERR2OUT=1 | Reported | Set AH_ERR2INT=1 |
| Amplitude Low | Set AL_ERR2OUT=1 | Reported | Set AL_ERR2INT=1 |
| Zero Count | N/A | Reported | Set ZC_ERR2INT=1 |

### DATA_CHx Error Output Values (when error reported via ERR2OUT)

| Condition | DATA_CHx Output (LDC131x) | DATA_CHx Output (LDC161x) |
|---|---|---|
| Under-range | 0x8000 (DATA=0x000) | DATA[27:0]=0x0000000 |
| Over-range | 0x4FFF (DATA=0xFFF) | DATA[27:0]=0xFFFFFFF |
| Watchdog Timeout | 0x2000 | Invalid — discard |
| Amplitude Warning | 0x1XXX (valid data present) | 0x1XXX (valid data present) |
| Zero Count | 0x0000 or 0x8000 | — |

### STATUS Register (Address 0x18) — Complete Bit Map

| Bit | Field | Type | Reset | Description |
|---|---|---|---|---|
| 15:14 | ERR_CHAN | R | 00 | Error source channel: b00=CH0, b01=CH1, b10=CH2, b11=CH3 |
| 13 | ERR_UR | R | 0 | Under-range error flag (sticky; cleared by reading STATUS or the error channel's DATAx_MSB) |
| 12 | ERR_OR | R | 0 | Over-range error flag (sticky) |
| 11 | ERR_WD | R | 0 | Watchdog timeout error flag (sticky) |
| 10 | ERR_AHE | R | 0 | Amplitude high error flag (sticky) |
| 9 | ERR_ALE | R | 0 | Amplitude low warning flag (sticky) |
| 8 | ERR_ZC | R | 0 | Zero count error flag (sticky) |
| 6 | DRDY | R | 0 | Data ready flag |
| 3 | CH0_UNREADCONV | R | 0 | CH0 unread conversion (not sticky) |
| 2 | CH1_UNREADCONV | R | 0 | CH1 unread conversion (not sticky) |
| 1 | CH2_UNREADCONV | R | 0 | CH2 unread conversion (LDC1614/LDC1314 only) |
| 0 | CH3_UNREADCONV | R | 0 | CH3 unread conversion (LDC1614/LDC1314 only) |

### ERROR_CONFIG Register (Address 0x19) — Complete Bit Map

| Bit | Field | Type | Reset | Description |
|---|---|---|---|---|
| 15 | UR_ERR2OUT | R/W | 0 | Route under-range errors to DATA_CHx.CHx_ERR_UR |
| 14 | OR_ERR2OUT | R/W | 0 | Route over-range errors to DATA_CHx.CHx_ERR_OR |
| 13 | WD_ERR2OUT | R/W | 0 | Route watchdog timeout errors to DATA_CHx.CHx_ERR_WD |
| 12 | AH_ERR2OUT | R/W | 0 | Route amplitude high errors to DATA_CHx.CHx_ERR_AE |
| 11 | AL_ERR2OUT | R/W | 0 | Route amplitude low warnings to DATA_CHx.CHx_ERR_AE |
| 7 | UR_ERR2INT | R/W | 0 | Route under-range errors to INTB + STATUS.ERR_UR |
| 6 | OR_ERR2INT | R/W | 0 | Route over-range errors to INTB + STATUS.ERR_OR |
| 5 | WD_ERR2INT | R/W | 0 | Route watchdog timeout to INTB + STATUS.ERR_WD |
| 4 | AH_ERR2INT | R/W | 0 | Route amplitude high to INTB + STATUS.ERR_AHE |
| 3 | AL_ERR2INT | R/W | 0 | Route amplitude low to INTB + STATUS.ERR_ALE |
| 2 | ZC_ERR2INT | R/W | 0 | Route zero count to INTB + STATUS.ERR_ZC |
| 1 | Reserved | R/W | 0 | Set to 0 |
| 0 | DRDY_2INT | R/W | 0 | Route data ready to INTB + STATUS.DRDY |

**Note:** When both AH_ERR2OUT=1 and AL_ERR2OUT=1, the CHx_ERR_AE bit in DATA_CHx reports a logic OR of both amplitude warnings.

### DATA_CHx Register Error Bits (Address 0x00 for CH0, etc.)

| Bit | Field | Description |
|---|---|---|
| 15 | CHx_ERR_UR | Under-range error flag (cleared by reading DATA_CHx) |
| 14 | CHx_ERR_OR | Over-range error flag |
| 13 | CHx_ERR_WD | Watchdog timeout error flag |
| 12 | CHx_ERR_AE | Amplitude warning (OR of high/low if both enabled) |
| 11:0 (LDC131x) / 27:0 (LDC161x) | DATAx | Conversion result |

### Error Conditions — Detailed Descriptions

**Frequency Under-Range (ERR_UR):**
- Occurs when output code would be negative after subtracting CHx_OFFSET
- Fix: Reduce OFFSET_CHx value or increase CHx_RCOUNT
- Output: DATA = 0x000 (LDC131x) or 0x0000000 (LDC161x)

**Frequency Over-Range (ERR_OR):**
- Occurs when sensor frequency exceeds reference frequency
- Fix: Increase reference frequency, decrease sensor frequency, increase CHx_FIN_DIVIDER, or decrease CHx_FREF_DIVIDER
- Output: DATA = 0xFFF (LDC131x) or 0xFFFFFFF (LDC161x)

**Watchdog Timeout (ERR_WD):**
- Occurs when sensor stops oscillating or oscillates below 250 Hz in continuous mode
- Recovery time: ~5.2 ms (must generate at least 1 oscillation)
- LDC aborts conversion and attempts sensor restart; repeated errors if sensor doesn't recover
- **Data from DATA registers is invalid during watchdog errors — must be discarded**
- Only occurs in continuous mode; use zero count + amplitude warnings for sequential mode

**Amplitude High (ERR_AHE):**
- VOSC > 1.8 Vp at conversion start
- Fix: Reduce IDRIVE setting

**Amplitude Low (ERR_ALE):**
- VOSC < 1.2 Vp at conversion start
- Fix: Increase IDRIVE or check sensor hardware (e.g., disconnected capacitor)

**Zero Count (ERR_ZC):**
- No oscillations recorded for sensor or reference input
- Causes: conversion time < 1 oscillation period, FIN_DIVIDER too large, sensor frequency too low, or sensor/clock stopped
- Fix: Increase CHx_RCOUNT, increase FREF_DIVIDER, reduce FIN_DIVIDER, or increase sensor frequency

### INTB Pin Behavior
- Asserted when an enabled error condition occurs (CONFIG.INTB_DIS must be 0)
- **Cleared by:** reading STATUS register, entering Sleep Mode, POR, Shutdown Mode, or software reset
- Set CONFIG.INTB_DIS=1 to disable INTB and hold pin high

### Recommended Error Handling Strategy
1. Enable all errors via ERR2INT bits for INTB notification
2. On INTB assertion, read STATUS register (clears error and de-asserts INTB)
3. Check ERR_CHAN to identify error source channel
4. Check individual error bits (ERR_UR, ERR_OR, ERR_WD, ERR_AHE, ERR_ALE, ERR_ZC) to identify error type
5. Optionally enable ERR2OUT bits to embed error flags in DATA registers for per-read validation

### Data Ready Detection Methods
1. **INTB-driven:** Set DRDY_2INT=1; INTB asserts on conversion complete
2. **Polling STATUS.DRDY:** Check bit 6 periodically
3. **Polling CHx_UNREADCONV:** Check bits 3:0 for per-channel completion in multi-channel mode
4. **Fixed-interval polling:** Calculate conversion time from RCOUNT and clock settings if clocks are shared

**Multi-channel DRDY behavior:**
- AUTOSCAN_EN=0 (single channel): DRDY on every conversion
- AUTOSCAN_EN=1 (sequential): DRDY on completion of last channel in sequence (e.g., RR_SEQUENCE=0 → DRDY after both CH0 and CH1 complete)

## Relevance to LDC1614 Implementation
This is the most critical application note for driver error handling. The driver must:
1. Configure ERROR_CONFIG (0x19) during initialization to enable desired error reporting
2. Implement an INTB interrupt handler that reads STATUS (0x18) and dispatches based on error type
3. Validate DATA_CHx reads by checking the upper 4 error bits before using conversion data
4. Handle watchdog errors by discarding data and potentially resetting the sensor
5. Use CHx_UNREADCONV bits (STATUS bits 3:0) for efficient multi-channel data collection on the LDC1614
6. Implement DRDY-based or interrupt-based readout rather than blind polling for reliable data acquisition
