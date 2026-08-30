# LDC Target Design
**Source:** target_design.pdf | **Doc #:** SNOA957B | **Pages:** 11

> **Naming note.** Register and field names below are quoted as SNOA957B writes them, which predates datasheet revision A. See the [pre-Revision-A name map](../extracted-md/05_register_map.md#pre-revision-a-name-map) for the current identifiers; addresses and bit positions are unchanged.

## Key Takeaways
- Target must be at least as large as the sensor coil for maximum inductance shift; undersized targets drastically reduce response.
- Target thickness should be ≥ 2–3 skin depths for optimal eddy current generation (95% current at 3δ).
- Aluminum and copper are the best practical target materials; silver is theoretically optimal but costly.
- Magnetic metals (steel) produce non-monotonic inductance response at mid-frequencies — use fSENSOR < 20 kHz or > 1 MHz.
- Discontinuities (gaps, voids) in the target increase noise by disrupting eddy current paths.

## Summary
The target is the conductive object whose movement the LDC senses. When the target enters the sensor's AC magnetic field, eddy currents are induced on its surface, opposing the field and reducing the sensor inductance. The LDC measures this inductance shift. Larger, more conductive, and smoother targets produce stronger eddy currents and therefore larger, more resolvable inductance shifts.

Target size critically affects performance — when the target matches or exceeds the sensor coil size, image currents on the target can fully mirror the sensor current pattern, producing maximum inductance shift (>21% in the test case). Targets smaller than the sensor produce progressively weaker responses. Skin depth (δ = √(ρ/(πfµ))) determines how much of the target thickness carries current; at least 2–3 skin depths are recommended. The sensor's magnetic field is symmetrical above and below the PCB, so conductive objects on the opposite side from the target can interfere — mitigation includes shielding with ferrite or a ≥3 skin depth metal sheet.

## Technical Details

### Skin Depth Formula
δ = √(ρ / (π × f × µ))
- µ = µ₀ × µr (magnetic permeability)
- ρ = resistivity of conductor
- f = sensor AC frequency

### Skin Depth at Common Frequencies

| Material | 0.1 MHz | 1 MHz | 10 MHz |
|---|---|---|---|
| Aluminum | ~0.26 mm | ~0.08 mm | ~0.03 mm |
| Copper | ~0.21 mm | ~0.07 mm | ~0.02 mm |
| Steel 1006 | ~0.05 mm | ~0.02 mm | ~0.006 mm |
| Carbon | ~5 mm | ~1.6 mm | ~0.5 mm |

### Current vs Conductor Thickness

| Thickness (skin depths) | Current carried |
|---|---|
| 1δ | 63.2% |
| 2δ | ~86% |
| 3δ | ~95% |
| 5δ | ~99% |

### Target Size Rules
- Target should be ≥ sensor coil diameter for maximum response
- Undersized targets (< sensor radius) produce < 1% inductance shift
- Matching size (= sensor diameter): > 21% shift in test example (14 mm sensor, SS304, 1 mm distance)
- Image currents flow as concentric loops matching sensor shape

### Target Material Comparison (14 mm sensor, 3.11 MHz free-space)
Best response (largest frequency shift): Aluminum AL1100, Copper CDA110
Moderate: Bronze, Steel C10101
Weakest: SS304, SS430L, Nickel N200

### Material Guidelines
| Material | Notes |
|---|---|
| **Aluminum** | Excellent: high conductivity, light, machinable, corrosion-resistant. Skin depth ~25% larger than copper. Temp coeff: 4200 ppm/°C. Anodized finish OK. |
| **Copper** | Excellent conductivity (95% of Ag), but heavier. PCB copper targets effective: 1-oz Cu = 37 µm = 1δ at 3.1 MHz. Use fSENSOR > 6 MHz for full response with 1-oz Cu. |
| **Steel/magnetic** | Non-monotonic L response at mid-frequencies due to competing permeability and eddy current effects. Use fSENSOR < 20 kHz or > 1 MHz. RP measurements unaffected. |
| **Conductive ink** | ~30% conductivity of pure Ag, typically <15 µm thick. Response typically <10% of solid conductor. Acceptable for low-precision applications. |
| **Water/body** | Ineffective. Water conductivity = 0.000001% of copper. Body response is capacitive, not inductive. |

### Interference Mitigation (Opposite Side of Sensor)
1. Move interferer far from sensor
2. Bring target closer to sensor
3. Reduce sensor size (if interferer is farther than target)
4. Fix interferer position (static loading OK if within drive capability)
5. Change interferer to non-conductive material
6. Use ferrite shielding (can even extend range slightly)
7. Use metal shield ≥ 3 skin depths thick

### Target Shape
- Smooth, continuous surfaces are optimal
- Gaps or voids force eddy currents to detour → weaker coupling, more noise
- PCB copper targets on FR4: excellent substrate (stable, low loss tangent, matched thermal expansion)

## Relevance to LDC1614 Implementation
Target design constraints determine the expected inductance shift range the driver must handle. Small or thin targets at extended distances produce very small shifts, requiring maximum RCOUNT and resolution settings. PCB copper targets at >3.1 MHz need the FIN_DIVIDER configured appropriately if sensor frequency exceeds 10 MHz. For systems using magnetic steel targets, the driver should be aware that inductance may increase (not decrease) approaching the target at certain frequencies, which affects threshold/comparison logic. The input dividers (CHx_FIN_DIVIDER) can compensate if the optimal frequency range is outside the LDC's internal resolution sweet spot.
