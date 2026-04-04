# How to Sense Spring Compression with Inductive Sensing
**Source:** spring_compression.pdf | **Doc #:** SSZTCH7 | **Pages:** 4

## Key Takeaways
- A physical spring can serve as an inductive sensor for LDC devices — its inductance varies with compression/extension
- Springs typically have too little inductance for direct LDC use; a series wire-wound inductor can boost total impedance
- Over a 50 mm to 100 mm extension range, 1 µm of stretch produces ~44 LDC1612 output codes on average
- The inductance-vs-length relationship is monotonic, enabling precise length determination
- This technique extends to any mechanical system where a spring deflects proportionally to an applied force

## Summary
LDC devices can use a spring as an inductive sensor because the spring's inductance changes with physical deformation (compression or extension). A steel spring (0.7 mm wire, 46 turns, 7.3 mm diameter) was connected to an LDC1612 EVM. Because the spring's native inductance was too low for the LDC1612, a 2.2 µH fixed wire-wound SMD inductor was added in series, giving a total sensor oscillation frequency of 2.5 MHz with a 1 nF capacitor.

The spring was extended from 50 mm to 100 mm in 5 mm increments. The LDC1612 output data was monotonic across the entire range, with inductance decreasing from 1.92 µH to 1.01 µH (output codes from 16,644,000 to 18,840,000). This yields an average resolution of ~44 codes per micrometer of extension.

## Technical Details

### Sensor Setup
- **Spring:** 0.7 mm steel wire, 46 turns, 7.3 mm diameter
- **Series inductor:** 2.2 µH wire-wound SMD (to increase total L above LDC minimum)
- **Sensor capacitor:** 1 nF
- **Oscillation frequency:** ~2.5 MHz
- **Device:** LDC1612 EVM (f_REF = 40 MHz)

### Inductance from Output Code

$$L_{sensor} = \frac{1}{C_{sensor} \cdot (2\pi \cdot f_{sensor})^2}$$

where:

$$f_{sensor} = \frac{f_{REF} \cdot \text{DATA}}{2^{28}}$$

and $f_{REF}$ = 40 MHz (LDC1612 EVM reference clock).

Spring inductance = $L_{sensor} - L_{series}$ (subtract the 2.2 µH fixed inductor).

### Measurement Results

| Spring Length | Spring Inductance | LDC Output Code |
|---|---|---|
| 50 mm (compressed) | 1.92 µH | 16,644,000 |
| 100 mm (extended) | 1.01 µH | 18,840,000 |

- Total code range: 2,196,000 codes over 50 mm
- Average resolution: **~44 codes/µm**

## Relevance to LDC1614 Implementation
This technique directly applies to the LDC1614. When using a spring sensor, note: (1) the series inductor's value must be stable over temperature and current — use a wire-wound type, not multilayer ceramic; (2) verify that the combined sensor (spring + series inductor + capacitor) keeps f_sensor and Rp within the LDC1614's valid operating range across the full compression range; (3) in a multi-channel LDC1614 system, one channel could measure the spring while another provides a temperature-reference measurement. Configure SETTLECOUNT appropriately, as the spring's Q-factor may differ significantly from PCB coil sensors.
