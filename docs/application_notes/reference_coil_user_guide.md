# LDC Reference Coils User's Guide
**Source:** reference_coil_user_guide.pdf | **Doc #:** SNOU136 | **Pages:** 33

## Key Takeaways
- 19 reference coils (A–S) covering three geometries: circular, rectangular, and stretched — all on a single snap-apart PCB
- Circular coils range from ⌀3 mm (3 turns) to ⌀46 mm (50 turns); stretched coils up to 100×15 mm
- Sensor capacitors are not populated — must be soldered to match desired oscillation frequency: f₀ = 1/(2π√(LC))
- Coils N, P, Q, R include 0805 footprint for a series inductor to meet Rp/fSENSOR boundary conditions
- Self-resonant frequencies range from 0.8 MHz (Coil I, ⌀46 mm 4-layer) to >15 MHz (small coils D, E, F) — sensor frequency must stay well below SRF

## Summary
The LDC Reference Coils board is a collection of 19 pre-designed PCB sensor coils that can be used with any LDC evaluation module, including the LDC1614 EVM. The board provides three coil geometries: circular coils for proximity/distance sensing and triangular-target position sensing, rectangular coils for compact sensing, and stretched coils for lateral position sensing with rectangular targets. Each coil is separated by perforations to avoid inter-coil interference during use.

The user's guide provides detailed characterization data for each coil: inductance (L) vs. frequency, AC series resistance (Rs) vs. frequency, quality factor (Q) vs. frequency, and self-resonant frequency (SRF). PCB layout files for all four layers (top, mid-1, mid-2, bottom) are included. All measurements were taken without a sensor capacitor using an HP 4194A Impedance/Gain-Phase analyzer.

## Technical Details
### Coil Characteristics Summary
| Coil | Type | Dimensions | Turns/Layer | Trace Width | Trace Spacing | Layers | SRF |
|------|------|-----------|-------------|-------------|---------------|--------|-----|
| A | Stretched | 100×15 mm | 23 | 6 mil | 6 mil | 4 | 1.9 MHz |
| B, C | Stretched | 100×8 mm | 17 | 4 mil | 4 mil | 2 | 10.0 MHz |
| D | Stretched | 40×5 mm | 11 | 4 mil | 4 mil | 2 | >15 MHz |
| E | Rectangular | 11×6 mm | 12 | 4 mil | 4 mil | 4 | >15 MHz |
| F | Stretched | 40×5 mm | 11 | 4 mil | 4 mil | 4 | >15 MHz |
| G | Stretched | 100×10 mm | 23 | 4 mil | 4 mil | 2 | 6.5 MHz |
| H | Circular | ⌀46 mm | 50 | 6 mil | 6 mil | 2 | 2.0 MHz |
| I | Circular | ⌀46 mm | 40 | 8 mil | 6 mil | 4 | 0.8 MHz |
| J | Circular | ⌀29 mm | 35 | 6 mil | 6 mil | 2 | 5.7 MHz |
| K | Circular | ⌀29 mm | 30 | 8 mil | 6 mil | 4 | 2.3 MHz |
| L | Circular | ⌀13 mm | 24 | 4 mil | 4 mil | 4 | — |
| M | Circular | ⌀13 mm | 25 | 4 mil | 4 mil | 2 | — |
| N | Circular | ⌀3 mm | 3 | 4 mil | 4 mil | 4 | — |
| O | Circular | ⌀8 mm | 11 | 4 mil | 4 mil | 4 | — |
| P | Circular | ⌀5 mm | 9 | 4 mil | 4 mil | 4 | — |
| Q | Circular | ⌀6 mm | 9 | 4 mil | 4 mil | 4 | — |
| R | Circular | ⌀4 mm | 6 | 4 mil | 4 mil | 4 | — |
| S | Circular | ⌀10 mm | 16 | 4 mil | 4 mil | 2 | — |

### Key Design Rules
- **Capacitor selection:** f₀ = 1/(2π√(LC)) — choose C to set desired operating frequency
- **Capacitor footprint:** 0603 on all coils
- **Series inductor footprint:** 0805 on coils N, P, Q, R (for Rp/frequency boundary compliance)
- **Sensor frequency must be below SRF** — operating near SRF causes non-ideal behavior (L and Q degrade)
- **Coil geometry selection:**
  - Circular: proximity sensing, distance detection, triangular-target position sensing
  - Rectangular: compact sensing in tight spaces
  - Stretched: lateral position sensing with rectangular targets (asymmetric field distribution)

### Trace Design Parameters
- Trace widths: 4 mil (0.10 mm), 6 mil (0.15 mm), or 8 mil (0.20 mm)
- Trace spacing: 4 mil (0.10 mm) or 6 mil (0.15 mm)
- PCB layers: 2-layer or 4-layer designs
- Loop stepping (stretched coils): 3.45 mm to 5.69 mm

### Characterization Equipment
- HP 4194A Impedance/Gain-Phase Analyzer
- All measurements without sensor capacitor

## Relevance to LDC1614 Implementation
The reference coils provide ready-made sensor options for LDC1614 prototyping and EVM testing. For custom designs, the coil parameters (turns, trace width, spacing, layer count, dimensions) serve as validated starting points. Key considerations for LDC1614: (1) select a coil whose SRF is well above the target sensor frequency; (2) LDC1614 supports sensor frequencies from 1 kHz to 10 MHz — the coil + capacitor must produce f₀ within this range; (3) coils N, P, Q, R with series inductor footprints are useful when the bare coil Rp is too low for LDC1614 limits; (4) 4-layer coils generally have higher inductance and lower SRF than 2-layer equivalents of the same diameter.
