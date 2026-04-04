# Common Inductive and Capacitive Sensing Applications
**Source:** common_sensing.pdf | **Doc #:** SLYA048B | **Pages:** 19

## Key Takeaways
- LDC1614 offers 28-bit resolution and can sense targets up to 200% of coil diameter (highest range in LDC family)
- Sensing distance sensitivity is highest within 20% of total sensing distance due to logarithmic inductance-vs-distance curve
- For high-precision applications, use a reference coil on a spare LDC1614 channel for temperature/environmental compensation
- Circular coils provide highest Q factor for a given size; rectangular coils trade Q for form factor flexibility
- Resolution is inversely proportional to sample rate — configure RCOUNT accordingly

## Summary
This application report surveys four common inductive and capacitive sensing applications: capacitive level sensing (FDC1004), inductive touch buttons, incremental encoders/event counting, and metal proximity sensing. For each application, it covers the theory of operation, device selection, design challenges, and relevant collateral.

The inductive sensing principle centers on an LC tank circuit whose resonant frequency shifts when a conductive target approaches the sensor coil. Eddy currents in the target oppose the coil's magnetic field, reducing observed inductance and increasing resonant frequency. The LDC devices convert this frequency shift to a digital output code. The document provides device selection flow charts and tables comparing LDC0851, LDC131x, LDC161x, LDC211x, and LDC3114 across different application types.

Key design considerations include coil geometry (circular for best Q), target size (at least coil size for best response), sensing distance limits, and environmental compensation strategies.

## Technical Details
- **Resonant frequency:** f₀ = 1 / (2π√(LC))
- **LDC1614 sensing range:** Up to 200% of coil diameter (highest in LDC family)
- **LDC131x sensing range:** Up to 50% of coil diameter for high precision, 100% for presence detection
- **LDC0851 sensing range:** 40% of coil diameter (side-by-side), 30% (stacked)
- **Sensitivity zone:** Highest within 20% of total sensing range (logarithmic inductance vs. distance)
- **Target sizing rule:** Metal target ≥ coil diameter for best axial sensing response; flat, uniform surface recommended
- **Environmental compensation:** Use spare channel as reference sensor with duplicate coil for differential approach
- **Device selection for touch buttons:**
  - LDC161x: Highest resolution (28-bit), sensing beyond 2 coil diameters
  - LDC211x: Lowest power (6 µA at 0.625 SPS), built-in baseline tracking
  - LDC3114-Q1: Automotive-qualified, configurable baseline tracking, raw data mode
  - LDC0851: Simplest (push/pull output), lowest cost
- **Encoder capability:** > 300 events/second detection with LDC0851

## Relevance to LDC1614 Implementation
The LDC1614 is the recommended device for applications requiring high resolution (28-bit) and long sensing range (up to 2× coil diameter). This document confirms the LDC1614 is suitable for touch buttons, metal proximity sensing, and by extension linear/rotational position sensing. For driver implementation, the key takeaway is that environmental compensation via a reference channel should be considered for precision applications. The logarithmic sensitivity curve means that system-level linearization will be needed for absolute distance measurements.
