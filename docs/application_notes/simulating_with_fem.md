# Simulate Inductive Sensors Using FEMM (Finite Element Method Magnetics)
**Source:** simulating_with_fem.pdf | **Doc #:** SNOAA04A | **Pages:** 20

## Key Takeaways
- FEMM is free, open-source software for simulating electromagnetic problems — suitable for predicting sensor inductance and eddy current behavior before building prototypes.
- Use **axisymmetric problems** for circular coils (most accurate) and **planar problems** for racetrack coils.
- FEMM calculates inductance directly but does not model the capacitor — an iterative calculate/simulate loop is needed to converge on resonant frequency.
- System resolution can be estimated by combining FEMM-simulated frequency shifts with measured noise floor from an LDC EVM.
- The Excel_FEMM tool (part of LDC Tools) can automate simulation for smooth targets ≥ sensor coil size.

## Summary
FEMM enables pre-prototype simulation of inductive sensing systems by solving 2D electromagnetic problems. For circular PCB coils, the axisymmetric problem type rotates the 2D cross-section around a symmetry axis to create an accurate 3D approximation. The simulation requires defining the coil geometry (from the LDC Tools Spiral Inductor Designer), materials (copper coils, air, target material), boundary conditions, and circuit properties including operating current and number of turns.

The simulation produces magnetic field contour plots, flux density maps, and eddy current density visualizations on the target. The inductance is read directly from the Circuit Properties output (Flux/Current). Since FEMM does not model the resonant capacitor, an iterative approach is needed: simulate at an initial frequency, compute the resonant frequency from the calculated inductance and known capacitor value using `f = 1/(2π√(LC))`, then re-simulate at the new frequency until convergence.

Resolution is estimated by simulating at two target positions (separated by the desired minimum detectable distance), computing the frequency shift Δf, and comparing to the measured noise floor σN from an LDC EVM. The signal-to-noise ratio must be ≥ 10: `SNR = Δf / (3σN)`.

## Technical Details

### FEMM Setup Procedure
1. **Create problem:** File → New → Magnetics Problem
2. **Problem type:** Axisymmetric (circular coils) or Planar (racetrack coils)
3. **Define problem:** Set frequency to sensor resonant frequency (from LDC Tools), units in mm, AC Solver = Succ. Approx.
4. **Place geometry:** Create cross-section of coil traces and target using nodes + segments; trace dimensions from Spiral Inductor Designer
5. **Define boundaries:** Use Boundary Builder for gradient-to-infinity model
6. **Define materials:** Air, Copper (for target), Copper Coil (custom: conductivity 58.5 MS/m, magnet wire type)
7. **Copper Coil strand diameter:** Calculate equivalent round wire from `T × W = πr²` where T = trace thickness, W = trace width
8. **Add circuits:** Define current (1 mA to 1 A acceptable), series type
9. **Assign materials and circuits to blocks:** Set turns per boundary = 1 (most realistic)
10. **Set mesh:** ~0.01 for fine mesh around coils and target
11. **Solve:** Click Solve button → generates .ans file

### Key Formulas
- **Resonant frequency:** `f = 1 / (2π√(LC))`
- **Capacitance from inductance:** `C = 1 / (2πf²L)`
- **Resolution SNR:** `SNR = Δf / (3σN)` where Δf = frequency shift from target movement, σN = noise standard deviation
- **Strand diameter:** `d = 2 × √(T × W / π)` where T = trace thickness (mm), W = trace width (mm)

### Iterative Frequency Convergence (Known Coil + Capacitor)
1. Pick initial frequency, simulate in FEMM at max target distance → get inductance L
2. Calculate `f = 1/(2π√(LC))` from known C and simulated L
3. Re-simulate at new f → get updated L
4. Repeat until L converges
5. Move target by minimum resolution distance, repeat → get Δf

### Resolution Estimation
1. Simulate at two target positions to get Δf
2. Measure noise floor σN using LDC EVM + Sensing Solutions GUI (Data Streaming tab → Show Statistics)
3. Compute `SNR = Δf / (3σN)`; require SNR ≥ 10
4. If insufficient, modify sensor or target design

## Relevance to LDC1614 Implementation
Useful for validating sensor coil designs before hardware prototyping. Simulation results inform the expected inductance range and frequency shift, which determine the appropriate RCOUNT, CLOCK_DIVIDERS, and OFFSET register settings in the LDC1614 driver. The resolution estimation procedure helps verify that the chosen sensor/target combination will meet system requirements within the LDC1614's 28-bit measurement capabilities.
