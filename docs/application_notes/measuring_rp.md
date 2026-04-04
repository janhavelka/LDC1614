# Measuring Rp of an L-C Sensor for Inductive Sensing
**Source:** measuring_rp.pdf | **Doc #:** SNOA936 | **Pages:** 4

## Key Takeaways
- Three practical methods exist to measure Rp: network analyzer, impedance analyzer, or signal generator + oscilloscope
- Rp must be measured at both minimum and maximum target distances to determine the full operating range
- The Rp formula is: Rp = (XL² + Rs²) / Rs, where XL = 2πfL (reactance) and Rs is AC series resistance
- The signal generator method measures Rp directly at resonance by adjusting R until VPP2 = 2×VPP1
- Measured Rp values feed directly into the DRIVE_CURRENT_CHx register configuration for LDC161x devices

## Summary
This application note describes three methods for measuring the equivalent parallel resistance (Rp) of an LC tank circuit, which is necessary for properly configuring LDC devices. Rp varies with target distance: minimum Rp occurs at closest target distance, maximum Rp at farthest. Both boundary values must be measured to ensure the Rp falls within the device's supported range.

Method 1 uses a vector network analyzer to measure XL (reactance) and Rs at the operating frequency, then calculates Rp. Method 2 uses an impedance analyzer, which may have a built-in Rp measurement function, or can derive it from Ls and Rs measurements. Method 3 is a low-cost approach using a signal generator and oscilloscope: drive the LC tank through a resistor R, tune to resonance (maximize VPP1), adjust R until VPP2 = 2×VPP1, then measure R with an ohmmeter — this R equals Rp.

## Technical Details
### Method 1: Network Analyzer
- Measure XL (reactance) and Rs (series loss resistance) at the desired frequency
- **Rp = (XL² + Rs²) / Rs**
- XL = 2πfL

### Method 2: Impedance Analyzer
- Measure Ls (inductance) and Rs (series resistance) at operating frequency
- Use built-in Rp function if available, or calculate: **Rp = (2πfLs)² / Rs + Rs ≈ (2πfLs)² / Rs** (for high-Q sensors where XL >> Rs)

### Method 3: Signal Generator + Oscilloscope (Low-Cost)
- Connect signal generator → resistor R → LC sensor (with capacitor populated)
- Monitor VPP1 across LC tank and VPP2 across generator output
- Tune frequency until VPP1 is maximized (resonance)
- Adjust R until VPP2 = 2 × VPP1
- Iterate for accuracy
- Measure R with ohmmeter: **R = Rp** of the LC sensor at resonance

### Measurement Requirements
- Measure Rp at **minimum target distance** (closest) → gives minimum Rp
- Measure Rp at **maximum target distance** (farthest) → gives maximum Rp
- Both values must fall within LDC device operating range (1 kΩ – 100 kΩ for LDC161x)

## Relevance to LDC1614 Implementation
Rp measurement is a prerequisite for LDC1614 configuration. The measured Rp_max (no target / max distance) determines the initial DRIVE_CURRENT_CHx setting — either via the lookup table in SNAA221B or via the auto-calibration procedure. During prototyping, Method 3 (signal generator + oscilloscope) provides a practical benchtop approach. If the measured Rp is outside the 1 kΩ – 100 kΩ range, the sensor must be redesigned (lower L or increase C to reduce Rp; add series inductor to increase Rp below minimum).
