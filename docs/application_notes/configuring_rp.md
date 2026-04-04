# Configuring Inductive-to-Digital Converters for Parallel Resistance (Rp) Variation
**Source:** configuring_rp.pdf | **Doc #:** SNAA221B | **Pages:** 11

## Key Takeaways
- LDC1614 does not measure Rp but must be configured for proper drive current based on the sensor's Rp range (1 kΩ to 100 kΩ)
- Drive current is set via the DRIVE_CURRENT_CHx register (CHx_IDRIVE field, bits 15:11) with 32 discrete levels (0–31)
- The auto-calibration method reads CHx_INIT_DRIVE (bits 10:6) with target at maximum distance, then writes that value to CHx_IDRIVE for normal operation
- Maximum sensor oscillation amplitude is 1.8 V; exceeding this activates ESD clamping and corrupts output
- Rp = L / (Rs × C) — parallel resistance decreases as target approaches sensor

## Summary
This application note explains how to configure the parallel resistance (Rp) settings for TI's LDC family. While LDC10xx devices actively measure Rp (proximity data), the LDC1312/1314/1612/1614 devices only measure inductance (frequency). However, correct Rp-aware configuration is still critical for LDC161x devices because the drive current must be set to maintain sensor oscillation amplitude within a usable range as Rp varies with target movement.

Rp is the equivalent parallel resistance of the LC tank at resonance. It decreases as a conductive target approaches (eddy currents increase energy loss). For LDC161x devices, a constant current drive is programmed per channel. The current must be set so that oscillation amplitude stays below 1.8 V (ESD clamp threshold) across the full operating range while remaining high enough for adequate SNR.

## Technical Details
### Rp Fundamentals
- **Rp formula:** Rp = L / (Rs × C), where Rs is AC series resistance (frequency-dependent due to skin effect)
- **Sensor frequency:** f_sensor = 1 / (2π√(L×C))
- **Quality factor:** Q = (1/Rs) × √(L/C) = Rp × √(C/L)
- **Rp range (LDC131x/161x):** 1 kΩ to 100 kΩ supported
- **Minimum acceptable Rp (LDC10xx):** 798 Ω (below this, add series inductor)

### DRIVE_CURRENT_CHx Register — Current Drive Lookup Table
| Rp (kΩ) | Drive Value | Rp (kΩ) | Drive Value |
|----------|-------------|----------|-------------|
| 89.99 | 0 | 8.36 | 16 |
| 77.59 | 1 | 7.20 | 17 |
| 66.87 | 2 | 6.21 | 18 |
| 57.63 | 3 | 5.35 | 19 |
| 49.67 | 4 | 4.61 | 20 |
| 42.83 | 5 | 3.98 | 21 |
| 36.91 | 6 | 3.43 | 22 |
| 31.81 | 7 | 2.95 | 23 |
| 27.42 | 8 | 2.55 | 24 |
| 23.64 | 9 | 2.19 | 25 |
| 20.37 | 10 | 1.89 | 26 |
| 17.56 | 11 | 1.63 | 27 |
| 15.14 | 12 | 1.40 | 28 |
| 13.05 | 13 | 1.21 | 29 |
| 11.25 | 14 | 1.05 | 30 |
| 9.69 | 15 | 0.90 | 31 |

Each value targets ~1.65 V oscillation amplitude for the given Rp. If Rp falls between values, use the lower Rp row.

### Auto-Calibration Procedure (LDC1312/1314/1612/1614)
1. Place target at maximum operating distance from sensor
2. Enter SLEEP mode: set `CONFIG.SLEEP_MODE_EN` = 1
3. Program desired `SETTLECOUNT` and `RCOUNT` values
4. Enable auto-calibration: set `RP_OVERRIDE_EN` = 0
5. Exit SLEEP mode: set `CONFIG.SLEEP_MODE_EN` = 0
6. Allow at least one measurement with target stable at max range
7. Read `CHx_INIT_DRIVE` (bits 10:6) from `DRIVE_CURRENT_CHx` register (0x1E–0x21) — save this value
8. For normal operation, write saved value to `CHx_IDRIVE` (bits 15:11)
9. Set `RP_OVERRIDE_EN` = 1 for fixed current drive during normal operation

### Oscillation Amplitude Limits
- **Maximum:** 1.8 V (above this, ESD clamping activates → frequency shift → invalid output)
- **Minimum:** Application-dependent; below ~few hundred mV, SNR degrades significantly; at near-zero target range oscillations may stop entirely (output = 0)

## Relevance to LDC1614 Implementation
This is essential for LDC1614 driver initialization. The driver must:
1. Implement the auto-calibration sequence during startup (or use a known Rp to set CHx_IDRIVE from the lookup table)
2. Write the correct value to DRIVE_CURRENT_CHx registers (0x1E–0x21) for each active channel
3. Set RP_OVERRIDE_EN = 1 for stable operation
4. Ensure sensor oscillation amplitude stays below 1.8 V across the full target range to prevent ESD clamping artifacts
