# Inductive Sensing: How Far Can I Sense?
**Source:** sensing_distance.pdf | **Doc #:** SSZTCL3 | **Pages:** 4

## Key Takeaways
- Maximum sensing distance scales with sensor coil diameter — this is the primary design lever for range.
- LDC1312/LDC1314 (12-bit): effective sensing up to **0.5× coil diameter**.
- LDC1612/LDC1614 (28-bit): effective sensing up to **2× coil diameter**, thanks to higher resolution and better SNR.
- Higher resolution allows both more accurate position determination and detection of targets at greater distances.
- Testing used 0.1 mm incremental steps of an aluminum target on a 14 mm diameter sensor coil, evaluating code change per step and standard deviation.

## Summary
The sensing distance of an LDC system is fundamentally limited by the magnetic field extent of the sensor coil, which is proportional to the coil's outer diameter. However, the LDC's resolution and signal-to-noise ratio determine how far away a target can be while still producing a measurable response above the noise floor.

Testing was performed by stepping an aluminum target in 0.1 mm increments axially away from a 14 mm diameter sensor coil on an LDC1612 EVM. Both the code change between steps and the standard deviation of 100 samples per step were recorded. The maximum distance was defined as the point where either the code change was insufficient to resolve 0.1 mm steps, or the noise floor exceeded the 6σ probability threshold (99.99966%).

## Technical Details
- **Test sensor:** 14 mm diameter coil on LDC1612 EVM
- **Target:** Aluminum, stepped in 0.1 mm increments axially
- **Resolution criterion:** Code step sufficient to resolve 0.1 mm movement
- **Noise criterion:** 6σ probability (99.99966%) that position can be determined
- **Rules of thumb:**
  - LDC1312/LDC1314: max target distance ≤ 0.5 × coil diameter
  - LDC1612/LDC1614: max target distance ≤ 2 × coil diameter
- **Design implication:** For a 4 mm sensing range, minimum coil diameter is 8 mm (for LDC131x) or 2 mm (for LDC161x, with reduced resolution)

## Relevance to LDC1614 Implementation
Critical for determining sensor coil size requirements. The LDC1614's 28-bit resolution provides up to 2× coil diameter sensing range. When designing a multi-channel system, coil diameters must be sized to the required sensing distances. The driver's RCOUNT and settle count settings affect effective resolution and thus achievable sensing distance.
