# Inductive Sensing: WEBENCH Coil Designer for Stacked Coils
**Source:** coil_design.pdf | **Doc #:** SSZTAZ8 | **Pages:** 5

## Key Takeaways
- WEBENCH Coil Designer can produce stacked and side-by-side coil designs with CAD export in under 5 minutes
- Side-by-side coils offer greatest sensitivity on 2-layer PCBs; stacked coils save space on 4-layer PCBs
- For sensing small targets (e.g., screw heads), increase turns so the coil fill ratio (dIN/dOUT) is < 0.3
- Maximum switching distance scales with coil diameter (e.g., ~6.8 mm for a 20 mm stacked coil)
- Stacked coils require threshold adjust mode on LDC0851; ADJ pin settings 1–15 scale switching distance from ~6.8 mm down to ~1.2 mm

## Summary
This technical article describes how to use TI's WEBENCH Coil Designer to create stacked coil layouts for inductive switch applications, primarily targeting the LDC0851 but applicable to sensor coil concepts used with the LDC1614. Stacked coils place sense and reference coils on top of each other on a 4-layer PCB (sense coil on top two layers, reference coil on bottom two), which is ideal for space-constrained proximity-sensing applications like door open/close detection.

The side-by-side arrangement provides greater sensitivity and is simpler to implement on a 2-layer PCB. The stacked arrangement minimizes PCB footprint. WEBENCH exports coil designs to Altium Designer, Cadence Allegro, CadSoft EAGLE, DesignSpark PCB, and Mentor Graphics PADS.

## Technical Details
- **Coil arrangements:** Side-by-side (2-layer PCB, highest sensitivity) vs. stacked (4-layer PCB, smallest footprint)
- **Fill ratio rule:** For small targets, dIN/dOUT < 0.3 (add more turns)
- **Switching distance (20 mm stacked coil example):**
  - ADJ = 1 → ~6.8 mm max switching distance
  - ADJ = 15 → ~1.2 mm switching distance
- **CAD export formats:** Altium Designer, Cadence Allegro 16.0–16.6, CadSoft EAGLE v6.4+, DesignSpark PCB, Mentor Graphics PADS PCB
- **Layer assignment (stacked):** Top Layer + MidLayer1 = sense coil; MidLayer2 + Bottom Layer = reference coil

## Relevance to LDC1614 Implementation
While this article focuses on the LDC0851 inductive switch, the coil design principles directly apply to LDC1614 sensor design. The WEBENCH Coil Designer tool can generate single-coil PCB sensors for LDC1614 applications. The general stacked/side-by-side trade-offs inform PCB coil layout decisions when designing LDC1614-based sensing systems. The dIN/dOUT < 0.3 figure above is SSZTAZ8's small-target exception for an LDC0851 EVM coil; for general LDC1614 sensor design follow SNOA930C, which recommends dIN/dOUT > 0.3 for optimal Q (see [sensor_design.md](sensor_design.md)), with lower ratios only for touch-on-metal and small-target cases. Multi-layer coil designs increase inductance density, which is relevant for LDC1614 sensor frequency planning.
