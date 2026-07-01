# Sensor Design for Inductive Sensing Applications Using LDC
**Source:** sensor_design.pdf | **Doc #:** SNOA930C | **Pages:** 23

## Key Takeaways
- Use the **largest circular coil** that physically fits — coil diameter is the primary determinant of sensing range.
- LDC161x effective sensing range: up to **2× coil diameter**; LDC131x: up to **0.5× coil diameter**.
- For most applications, maintain **dIN/dOUT > 0.3** for optimal Q. Exception: Touch-on-Metal allows dIN/dOUT as low as 0.05.
- Use **NP0/C0G capacitors** (300 pF to 2 nF typical) placed as close as possible to the inductor.
- Keep sensor frequency below **75% of the self-resonance frequency (SRF)** of the inductor.
- Keep all conductors (ground planes, traces) at least **30% of coil diameter** away from the sensor.

## Summary
This is the definitive sensor design reference for all LDC devices. The sensor is an LC tank formed by a PCB spiral inductor and a parallel capacitor. The sensor frequency is `f = 1 / (2π√(LC))`, and the LDC1612/1614 family operates from 1 kHz to 10 MHz.

The inductor's series resistance RS and its parallel equivalent RP are critical parameters. RP = L/(RS×C) = (2πf×L)²/RS. Lower RS (higher RP) means less loss and better measurement accuracy. The LDC must be able to drive the sensor at its lowest RP operating point. Circular coils provide the best Q for a given area. Multi-layer inductors connected in series dramatically increase total inductance through mutual coupling — a 4-layer design can yield ~2.8× the inductance of simple summation (39 µH vs 14 µH for four 3.5 µH coils). Multi-layer parallel configurations can reduce RS for optimized RP measurements.

Capacitor selection is critical: NP0/C0G grade ceramics are mandatory for their stability, low ESR, absence of piezoelectric effects, and minimal dC/dV. Parasitic capacitance from traces and ground planes introduces measurement error, which is minimized by using larger sensor capacitor values (300 pF–2 nF) and keeping conductors away from the sensor.

## Technical Details

### Inductance Calculation (Mohan's Equation for Single-Layer Spiral)
`L = (K1 × µ0 × n² × davg) / (1 + K2 × ρ)` where:
- K1, K2 = geometry-dependent constants (circle: c1=1.0, c2=2.46, c3=0, c4=0.20)
- µ0 = 4π×10⁻⁷ H/m
- n = number of turns
- davg = (dOUT + dIN) / 2
- ρ = (dOUT − dIN) / (dOUT + dIN) (fill ratio)

### Key Formulas
- **Sensor frequency:** `f = 1 / (2π√(LC))`
- **RP from RS:** `RP = L / (RS × C) = (2πf_SENSOR × L)² / RS`
- **Multi-layer series inductance (2 coils):** `L_TOTAL = L1 + L2 + 2M`, where `M = k × √(L1 × L2)`

### Inductor Design Rules
| Rule | Value |
|---|---|
| Recommended shape | Circular (best Q for given area) |
| dIN/dOUT ratio (general) | > 0.3 |
| dIN/dOUT ratio (Touch-on-Metal) | as low as 0.05 |
| Minimum PCB trace width | 0.1 mm (4 mil) typical |
| Multi-layer rotation | Alternate CW/CCW per layer |
| Sensing range (LDC161x) | Up to 2 × dOUT |
| Sensing range (LDC131x) | Up to 0.5 × dOUT |
| Sensor frequency | < 75% of inductor SRF |
| Conductor clearance | ≥ 30% of dOUT from sensor |

### Multi-Layer Inductance Example
- Single layer: 18 mm dOUT, 0.15 mm trace, 12 turns → 3.5 µH
- 4-layer series (1.0 mm PCB): 39 µH total (14 µH from coils + 25 µH from mutual inductance)
- Mutual inductance does NOT increase RS — it's "free" inductance

### Capacitor Requirements
- **Type:** NP0/C0G ceramic only
- **Typical range:** 300 pF to 2 nF (47 pF for close-proximity ToM)
- **Properties:** No polarization, minimal aging, ±30 ppm/°C, very low ESR, no piezoelectric effects, no dC/dV
- **Placement:** As close to inductor as possible (exception: LDC0851/LDC211x/LDC3114 — place near device pin)

### PCB Layout
- Remove ground plane within 30% of coil diameter
- Inner vias close to traces, not at center
- Thicker traces (≥10 mil / 0.25 mm) between inductor and capacitor
- No ferrite beads as sensor inductors
- No ground pour or thieving around sensor

### Design Process (WEBENCH)
1. Set dOUT to maximum physical size
2. Set trace width/spacing to PCB fab minimum (typically 4–6 mil)
3. Set copper thickness (typically 1 oz / 35 µm)
4. Set number of layers to match PCB
5. Set capacitance (300 pF–2 nF default range)
6. Adjust turns for dIN/dOUT > 0.3
7. Export to PCB CAD tool

## Relevance to LDC1614 Implementation
Foundational for any LDC1614-based system. The sensor design determines the operating frequency range, RP bounds, and achievable resolution — all of which constrain the driver's register configuration (RCOUNT, SETTLECOUNT, CLOCK_DIVIDERS, DRIVE_CURRENT). The RP formula is needed to calculate the correct IDRIVE setting. Multi-layer coil designs are especially relevant for increasing inductance without degrading RS, enabling longer sensing ranges with the LDC1614's 28-bit resolution.
