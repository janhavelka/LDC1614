# Improving ENOB of a Multichannel LDC by 4 Bits Using Gain and Offset
**Source:** improving_enob.pdf | **Doc #:** SSZTCK1 | **Pages:** 5

## Key Takeaways
- LDC1312/LDC1314 have an internal 16-bit converter but only expose 12 bits by default — gain/offset registers can recover up to 4 additional bits
- Gain settings of 4×, 8×, 16× shift the data window by 2, 3, or 4 bits respectively with no impact on timing or power
- The offset register subtracts a fixed value from output to prevent full-scale clipping after gain is applied
- Maximum output range limits: ≤25% FS at 4×, ≤12.5% FS at 8×, ≤6.25% FS at 16× gain
- If 12-bit + 4-bit gain (effective 16-bit) is still insufficient, upgrade to LDC1612/LDC1614 (28-bit native resolution)

## Summary
This technical article describes a 3-step method to improve the effective number of bits (ENOB) on the LDC1312 and LDC1314 by up to 4 bits using the gain and offset registers. The LDC131x devices have a 16-bit internal ADC but only expose the 12 MSBs in the DATA_CHx registers by default. By applying gain (which shifts the output window to expose lower-significance bits) and subtracting an offset (to prevent clipping), the effective resolution can be improved from 12 bits to up to 16 bits without any timing or power penalty.

The method is demonstrated with a practical example: sensing a US quarter coin between 0.2 mm and infinite distance on an LDC1314 EVM. The raw delta is only 90 codes (2.2% of full scale). Applying 16× gain improves the code delta to 1,458 codes — a 4-bit improvement in ENOB (from 6.5 to 10.5 bits).

## Technical Details
### 3-Step ENOB Optimization Procedure
1. **Determine system boundaries:** Measure output codes at minimum and maximum target distances. Calculate the delta as a percentage of full scale (2^12 - 1 = 4,095)
2. **Apply gain:** Select the highest gain setting where the output range fits within the gain limit:
   - Gain = 1×: ≤ 100% of full scale
   - Gain = 4×: ≤ 25% of full scale
   - Gain = 8×: ≤ 12.5% of full scale
   - Gain = 16×: ≤ 6.25% of full scale
3. **Subtract offset:** If the amplified signal crosses the full-scale boundary (clips), subtract a fixed offset to center it within the valid range (0 to 4,095)

### Example Results
| Configuration | Min Distance Code | Max Distance Code | Delta (codes) | ENOB |
|---|---|---|---|---|
| Gain = 1×, Offset = 0 | 3,302 | 3,212 | 90 | 6.5 bits |
| Gain = 16×, Offset = 0 | clipped | 3,212 | — | clipped |
| Gain = 16×, Offset = -2000 | 2,670 | 1,212 | 1,458 | 10.5 bits |

### Register Configuration
- **Data registers:** DATA_CH0, DATA_CH1, DATA_CH2, DATA_CH3
- **Gain:** Configured via gain register (gain values: 1×, 4×, 8×, 16×)
- **Offset:** Configured via offset register (subtracted from output code)

## Relevance to LDC1614 Implementation
This technique applies to the LDC1312/LDC1314 (12-bit) devices only. The LDC1614 has native 28-bit resolution and does not need gain/offset tricks for most applications. However, understanding this method is valuable for: (1) mixed LDC131x/LDC161x system designs where cost optimization uses LDC1314 on less-critical channels, and (2) understanding the LDC architecture — the LDC1614's 28-bit resolution is its primary advantage over the LDC1314 for high-precision sensing.
