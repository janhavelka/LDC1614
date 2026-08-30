# Setting LDC1312/4, LDC1612/4, and LDC1101 Sensor Drive Configuration
**Source:** sensor_configuration_settings.pdf | **Doc #:** SNOA950 | **Pages:** 8

> **Naming note.** Register and field names below are quoted as SNOA950 writes them, which predates datasheet revision A. See the [pre-Revision-A name map](../extracted-md/05_register_map.md#pre-revision-a-name-map) for the current identifiers; addresses and bit positions are unchanged.

## Key Takeaways
- The IDRIVE register controls sensor oscillation amplitude and must be tuned per-sensor based on the parallel resistance RP.
- Target oscillation amplitude: **1.2 Vp ≤ VOSC ≤ 1.8 Vp** — set IDRIVE to the highest value that keeps VOSC below 1.8 Vp.
- For normal operation, always use manual drive: set **RP_OVERRIDE_EN=1** and **AUTO_AMP_DIS=1** to disable automatic amplitude control.
- Channel 0 has a HIGH_CURRENT_DRV mode for very low RP sensors, but requires AUTOSCAN_EN=0. SNOA950 states 3 mA (2x the 1.5 mA normal maximum); the LDC1612/LDC1614 datasheet specifies IHDSENSORMAX = 6 mA with a typical drive current above 3.5 mA and RP_HD_MIN = 250 Ω (pp. 6, 45). Use the datasheet values.
- When using identical sensors across channels, use the same IDRIVE for all — take the lowest acceptable setting.

## Summary
The LDC1612/LDC1614 feature adjustable sensor current-drive (IDRIVE) to set the sensor oscillation amplitude. Different sensors have different RP values (parallel resistance at resonant frequency), and thus require different drive currents to achieve the correct amplitude. The IDRIVE register field in DRIVE_CURRENT_CHx controls this on a per-channel basis, with 32 settings from 16 µA (IDRIVE=0) to 1.563 mA (IDRIVE=31).

The correct amplitude is critical for measurement accuracy. If VOSC exceeds 1.8 Vp, accuracy degrades over temperature. If VOSC drops below 1.2 Vp, SNR suffers. Below ~0.5 Vp, oscillation may collapse entirely. Since RP decreases as a target approaches (lowering amplitude), IDRIVE must be set with the target at its **maximum operating distance** to avoid exceeding 1.8 Vp.

Automatic amplitude control should only be used for prototyping — never in production. It can introduce offset steps during measurements. For normal operation, set RP_OVERRIDE_EN=1 and AUTO_AMP_DIS=1 to force the LDC to use the fixed IDRIVE setting.

## Technical Details

### IDRIVE to RP Mapping (DRIVE_CURRENT_CHx Register, bits [15:11])

| IDRIVE Value | Bits [15:11] | Nominal Current (µA) | Max Sensor RP (kΩ) |
|---|---|---|---|
| 0 | b00000 | 16 | 90.0 |
| 1 | b00001 | 18 | 77.6 |
| 2 | b00010 | 20 | 66.9 |
| 3 | b00011 | 23 | 57.6 |
| 4 | b00100 | 28 | 49.7 |
| 5 | b00101 | 32 | 42.8 |
| 6 | b00110 | 40 | 36.9 |
| 7 | b00111 | 46 | 31.8 |
| 8 | b01000 | 52 | 27.4 |
| 9 | b01001 | 59 | 23.6 |
| 10 | b01010 | 72 | 20.4 |
| 11 | b01011 | 82 | 17.6 |
| 12 | b01100 | 95 | 15.1 |
| 13 | b01101 | 110 | 13.0 |
| 14 | b01110 | 127 | 11.2 |
| 15 | b01111 | 146 | 9.69 |
| 16 | b10000 | 169 | 8.36 |
| 17 | b10001 | 195 | 7.20 |
| 18 | b10010 | 212 | 6.21 |
| 19 | b10011 | 244 | 5.35 |
| 20 | b10100 | 297 | 4.61 |
| 21 | b10101 | 342 | 3.98 |
| 22 | b10110 | 424 | 3.43 |
| 23 | b10111 | 489 | 2.95 |
| 24 | b11000 | 551 | 2.55 |
| 25 | b11001 | 635 | 2.20 |
| 26 | b11010 | 763 | 1.89 |
| 27 | b11011 | 880 | 1.63 |
| 28 | b11100 | 1017 | 1.40 |
| 29 | b11101 | 1173 | 1.21 |
| 30 | b11110 | 1355 | 1.05 |
| 31 | b11111 | 1563 | 0.90 |

### Key Formulas
- **RP from RS:** `RP = L / (RS × C)` where L = inductance, C = sensor capacitor, RS = series resistance at resonant frequency
- **Oscillation amplitude:** `VOSC = 4 × RP × IDRIVE` (peak voltage)
- **Sensor frequency:** `fSENSOR = 1 / (2π × √(L × C))`

### Register Configuration for Normal Operation
| Setting | Value | Purpose |
|---|---|---|
| RP_OVERRIDE_EN | 1 | Disable automatic RP measurement during conversion |
| AUTO_AMP_DIS | 1 | Disable automatic amplitude correction |
| DRIVE_CURRENT_CHx[15:11] | Per sensor | Set IDRIVE to highest value where VOSC ≤ 1.8 Vp |
| HIGH_CURRENT_DRV | 0 or 1 | CH0 only; requires AUTOSCAN_EN=0. SNOA950 says 3 mA; the datasheet says IHDSENSORMAX = 6 mA, typical above 3.5 mA, sensor RP down to 250 Ω (pp. 6, 45) |

### IDRIVE Calibration Procedure (Oscilloscope Method)
1. Move target to maximum operating distance from sensor
2. Set IDRIVE = 31 (maximum) as starting point
3. Measure oscillation amplitude at INAx pin with oscilloscope
4. Reduce IDRIVE until VOSC < 1.8 Vp
5. Verify at minimum target distance that VOSC > ~0.5 Vp (oscillation stable)

### Multi-Channel Considerations
- Each channel has independent IDRIVE setting
- If sensors are identical, use the **same** IDRIVE for all channels
- If calibration yields different optimal values (e.g., 14, 13, 13, 14), use the **lowest** (13) for all channels

### Amplitude Warning Flags
- **ERR_AHE** (bit 10 in STATUS): VOSC > 1.8 Vp — reduce IDRIVE
- **ERR_ALE** (bit 9 in STATUS): VOSC < 1.2 Vp — increase IDRIVE or check sensor

## Relevance to LDC1614 Implementation
Directly impacts driver initialization. The DRIVE_CURRENT_CHx registers (addresses vary per channel) must be programmed with the correct IDRIVE value during setup. The driver can set RP_OVERRIDE_EN=1 and AUTO_AMP_DIS=1 in the CONFIG register for stable fixed-drive operation when that is appropriate for the application. The amplitude warning flags (ERR_AHE, ERR_ALE) should be monitored during development and optionally in deployment-specific fault monitoring. For multi-channel LDC1614 systems, ensure consistent IDRIVE across channels when using identical sensors.
