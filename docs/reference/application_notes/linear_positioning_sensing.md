# LDC1612/LDC1614 Linear Position Sensing
**Source:** linear_positioning_sensing.pdf | **Doc #:** SNOA931A | **Pages:** 15

> **Naming note.** Register and field names below are quoted as SNOA931A writes them, which predates datasheet revision A. See the [pre-Revision-A name map](../extracted-md/05_register_map.md#pre-revision-a-name-map) for the current identifiers; addresses and bit positions are unchanged.

## Key Takeaways
- Two approaches: circular coil + triangular target (higher resolution, larger target) vs. stretched coil + rectangular target (smaller target, simpler shape)
- At RCOUNT = 0xFFFF, measurement noise as low as 0.36 µm (circular) and 0.85 µm (stretched) standard deviation
- Target distance dz should be kept below 1 coil diameter; lower dz gives higher resolution but increases z-axis tolerance sensitivity
- Increasing RCOUNT from 0x00FF to 0xFFFF improves SNR by 33.8–43.8 dB at the cost of conversion time (0.1 ms → 26.2 ms at 40 MHz fREF)
- Dual-coil z-axis compensation eliminates target height tolerance errors for high-precision systems

## Summary
This application note presents two methods for linear position sensing using the LDC1612/LDC1614. Both approaches use PCB coils as sensors and measure inductance changes caused by eddy currents in a conductive target moving laterally across the sensor.

**Approach 1 (Circular coil + triangular target):** A 29 mm circular PCB coil (70 turns/layer, 2 layers) senses a copper isosceles triangle (25 mm wide, 70 mm long). Moving the target from 0–100 mm at dz = 2 mm produces a code range of 3,998,031 to 5,316,099. Effective resolution varies: 25.3 codes/µm at center (dx = 50 mm) but only 0.1 codes/µm at extremes. The usable precision range is approximately 60% of travel.

**Approach 2 (Stretched coil + rectangular target):** A 100×15 mm stretched coil (28 turns/layer, 2 layers, 3.3 mm stepping) senses a 14×25 mm aluminum target. Code range is 5,900,000 to 6,250,000 over 100 mm travel. Center resolution is 3.1 codes/µm. Usable range is ~77% of coil length (monotonic region).

## Technical Details
### Approach 1: Circular Coil + Triangular Target
- **Coil:** 29 mm diameter, 70 turns/layer, 2-layer PCB
- **Target:** Copper isosceles triangle, 25 mm wide × 70 mm long; extend beyond hypotenuse by coil diameter for monotonicity
- **Inductance range:** 216.3 µH (no target) → 122.2 µH (full coverage) at dz = 2 mm
- **Resolution at center (dx = 50 mm):**
  - dz = 1 mm: 43.1 codes/µm
  - dz = 2 mm: 25.3 codes/µm
  - dz = 3 mm: 16.2 codes/µm
- **Usable precision range:** 13 mm ≤ dx ≤ 95 mm @ ≥1 code/µm; 23.5 mm ≤ dx ≤ 82.5 mm @ ≥10 codes/µm

### Approach 2: Stretched Coil + Rectangular Target
- **Coil:** 100 × 15 mm, 28 turns/layer, 2-layer PCB, 3.3 mm loop stepping
- **Target:** 14 × 25 mm aluminum rectangle; YTARGET must extend past coil width
- **Inductance range:** 95.4 µH → 89.1 µH over center region (15–92 mm)
- **Resolution at center (dx = 50 mm):**
  - dz = 1 mm: 4.7 codes/µm
  - dz = 2 mm: 3.1 codes/µm
  - dz = 3 mm: 2.1 codes/µm
- **Three regions:** 0–15 mm (entry, low resolution), 15–92 mm (monotonic, usable), 92–100 mm (reversal, unusable)

### RCOUNT vs. Noise Performance (Circular Coil, dx = 50 mm, dz = 2 mm)
| RCOUNT | Conversion Time (fREF = 40 MHz) | Std Dev (codes) | Std Dev (µm) |
|--------|----------------------------------|-----------------|--------------|
| 0xFFFF | 26.2 ms | 5.7 | 0.36 |
| 0x0FFF | 1.6 ms | 20.6 | 1.29 |
| 0x00FF | 0.1 ms | 277.6 | 17.35 |

### RCOUNT vs. Noise Performance (Stretched Coil, dx = 50 mm, dz = 2 mm)
| RCOUNT | Conversion Time (fREF = 40 MHz) | Std Dev (codes) | Std Dev (µm) |
|--------|----------------------------------|-----------------|--------------|
| 0xFFFF | 26.2 ms | 2.38 | 0.85 |
| 0x0FFF | 1.6 ms | 15.85 | 5.66 |
| 0x00FF | 0.1 ms | 370.40 | 132.28 |

### Z-axis Tolerance (Circular Coil, dx = 50 mm)
| dz | Output Code | Error (codes) | Equivalent dx Error |
|----|-------------|---------------|---------------------|
| 1.8 mm | 4,613,716 | +49,152 | +1,940 µm |
| 2.0 mm | 4,564,564 | 0 | 0 |
| 2.2 mm | 4,519,173 | -45,391 | -1,792 µm |

### Quality Factor
- Q = (1/Rs) × √(L/C) — maximize Q for noise immunity, temperature stability, and measurement accuracy

### Output Linearization Methods
1. Best-fit polynomial curve (requires MCU processing power)
2. Look-up table (requires memory, minimal CPU)
3. Custom target shape (requires FEM modeling or iterative experiments)

## Relevance to LDC1614 Implementation
This is directly applicable to LDC1614 linear slider designs. Key driver/system parameters: (1) CHx_RCOUNT register controls resolution/noise trade-off — 0xFFFF gives best resolution at 26.2 ms/sample; (2) DRIVE_CURRENT_CHx must accommodate Rp variation over the full target travel range; (3) for precision systems, allocate a second LDC1614 channel for z-axis compensation using a dual-coil approach; (4) the stretched coil approach is preferred when target size is constrained. WEBENCH and TI coil scripts assist with sensor coil PCB layout generation.
