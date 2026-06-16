# How to Use the LDC Racetrack Inductor Designer Tool
**Source:** using_racetrack_design_tool.pdf | **Doc #:** SSZTAK7 | **Pages:** 6

## Key Takeaways
- The Racetrack Inductor Designer in the LDC calculator spreadsheet provides quick sensor parameter estimation (typically within 10% of physical sensor)
- Accuracy degrades when the racetrack long-to-short side ratio exceeds 4:1
- A coil fill ratio of ~0.3 (inner diameter / outer diameter) is optimal for most applications; Metal Touch applications may use lower values
- The tool accounts for target interaction effects on L, f_RES, Rp, and Q — all must remain within LDC operating region
- Racetrack shapes yield higher inductance but lower Q than circular sensors

## Summary
The Racetrack Inductor Designer is a tab within TI's LDC Calculator spreadsheet that designs circular or racetrack-shaped PCB sensor coils. Unlike WEBENCH Coil Designer (which generates PCB layouts), this tool only calculates sensor parameters but is faster for iterative design. The tool accepts PCB fabrication constraints, mechanical envelope limits, and target distance, then outputs sensor inductance, resonant frequency, Rp, and Q-factor.

The designer warns when parameters fall outside the selected LDC device's valid operating region with red warning labels. The user must verify that sensor parameters remain valid both with and without target interaction, since a nearby conductive target shifts inductance, frequency, and Rp.

## Technical Details

### Step-by-Step Design Procedure
1. **Select LDC device** — e.g., LDC1612/4
2. **Set trace spacing and width** — per PCB manufacturer minimums (e.g., 0.125 mm / 5 mil)
3. **Set inter-layer spacing** — PCB stack-up thickness (e.g., 32 mil)
4. **Set copper thickness** — thicker is better for Q (e.g., 1.0 oz-Cu)
5. **Set number of layers** — typically 2 or 4
6. **Enter outer diameter** — per mechanical constraints
7. **Set ratio** — 1.0 = circular; >1.0 = racetrack (higher L, lower Q)
8. **Set number of turns** — target coil fill ratio ~0.3; check inner diameter ≥ minimum
9. **Enter target distance** — closest approach distance
10. **Adjust sensor capacitor** — ensure f_RES', Rp', Q' stay within LDC design space

### PCB Fabrication Constraints (Example)

| Parameter | Value |
|---|---|
| Min trace width/space | 0.125 mm (5 mil) |
| Min via pad size | 0.6 mm (24 mil) |
| Min via hole size | 0.25 mm (10 mil) |
| Min inner diameter | 0.825 mm (via pad + 2× trace space) |

### Example Design Result (9 mm circular sensor)

| Parameter | Value |
|---|---|
| Sensor capacitance | 130 pF (10% tolerance) |
| Layers | 2 |
| Turns | 14 |
| Outer diameter | 9.0 mm |
| Ratio | 1.0 (circular) |
| Spacing / Width | 5 mil / 5 mil |
| PCB thickness | 32 mil |
| Copper | 1.0 oz-Cu |

### With Target at 1.8 mm

| Parameter | Value |
|---|---|
| L' (inductance w/ target) | 2.380 µH |
| f_RES' (frequency w/ target) | 9.049 MHz |
| Rp' (parallel resistance w/ target) | 4.90 kΩ |
| Q' (quality factor w/ target) | 36.3 |

## Relevance to LDC1614 Implementation
This tool is essential for designing PCB sensor coils for the LDC1614. The LDC1612/4 option in the device selector applies the correct drive-current limits and frequency boundaries. When designing multi-channel systems, each channel's sensor must be verified independently — especially with different target distances per channel. Always check that the sensor parameters with target interaction (L', f_RES', Rp', Q') remain within the LDC1614's valid operating region to avoid amplitude regulation errors or conversion failures.
