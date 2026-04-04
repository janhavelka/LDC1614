# Inductive Sensing Touch-on-Metal Buttons Design Guide
**Source:** sensing_touch_on_metal.pdf | **Doc #:** SNOA951 | **Pages:** 21

## Key Takeaways
- Complete design guide for Touch-on-Metal (ToM) buttons covering mechanical design, sensor coil design, power management, EMI compliance, and detection algorithms.
- Recommended metal-to-sensor spacing: >0.2 mm but within 20% of coil diameter for peak sensitivity.
- Coil outer diameter should be 50–60% of the metal button diameter; inner fill ratio dIN/dOUT < 0.4 for ToM.
- LDC1614 supports 4 buttons natively; >4 buttons require external multiplexing.
- System passes CISPR-22 EMI emissions testing at 9 MHz sensor frequency with 1.3 Vp oscillation amplitude.

## Summary
This comprehensive application note covers all aspects of designing a Touch-on-Metal button system using the LDC1612/LDC1614. When force is applied to a flat metal surface, microscopic deflection (typically 20–50 µm for a proper design) shifts the inductance of a PCB coil sensor underneath. The LDC measures this shift and a microcontroller interprets it as a button press.

The mechanical design section covers material selection (aluminum AL6061-T6 preferred for flexibility and high inductance shift), metal thickness tradeoffs, button structure with spacers or milled recesses, and mechanical isolation between adjacent buttons. The sensor design section provides PCB coil guidelines optimized for detecting concave metal deflection, with emphasis on maximizing inner turns for close-proximity sensitivity. A complete example uses a 0.8 mm thick aluminum panel with 12 mm coils, 100 pF capacitors (9.1 MHz sensor frequency), and achieves >47 dB SNR at 0.5 N force.

Power consumption can be minimized through duty cycling — a typical 2-button system at 10 SPS draws only ~225 µA average. The software algorithm uses a simple moving average with a dynamically adjusted threshold, supporting haptic feedback through drivers like the DRV2605.

## Technical Details

### Mechanical Design
- **Materials:** Aluminum AL6061-T6 (Young's modulus 68.9 GPa) preferred; Stainless Steel SS304 (203 GPa) also viable but ~3× stiffer
- **Metal thickness:** Thinner = more deflection. 0.25 mm Al at 1 N force → 27 µm peak deflection; 1 mm Al at 1 N → only 0.42 µm
- **Natural button force:** 0.5–2 N for non-moving buttons (vs. 2–5 N for mechanical)
- **Desired deflection:** ≥10× system noise (e.g., if noise = ±0.5 µm, need ≥5 µm deflection)
- **Target distance:** Sensor within 20% of coil diameter from metal, but >0.2 mm for manufacturing tolerance
- **Mechanical isolation:** Button-to-button separation > 1 coil diameter; use standoffs/grooves between buttons

### Sensor Coil Design (Example)
| Parameter | Value |
|---|---|
| dOUT | 12 mm |
| dIN | 3.3 mm |
| Turns/layer | 14 |
| Layers | 2 |
| Trace width | 6 mil (0.15 mm) |
| Trace spacing | 6 mil (0.15 mm) |
| Nominal inductance (at 0.55 mm) | 3 µH |
| Sensor capacitor | 100 pF (NP0/C0G) |
| Sensor frequency | 9.1 MHz |
| Oscillation amplitude | 1.3 Vp |

### Button Quantity vs Device
| Buttons | Device |
|---|---|
| 1–2 | LDC1612 |
| 3–4 | LDC1614 |
| 5+ | LDC1614 + external multiplexing |

### Power Consumption (Example: 2 buttons, 10 SPS)
- fREF = 20 MHz, RCOUNT = 0x1000, Sensor Q = 20
- Settle count = 64 (register 0x10–0x13, minimum setting 0x4)
- Single conversion time = 3.28 ms
- **Shutdown mode:** 225 µA average
- **Sleep mode:** 256 µA average

### EMI Performance
- Passes CISPR-22 emissions standard at 9 MHz sensor frequency, 1.3 Vp amplitude
- Common-mode choke (e.g., SRF3216-222Y) + 33 pF caps needed only for long wire runs

### Detection Results
- 1 N force → 15 µm deflection → -1.07% inductance change
- Mechanical crosstalk: adjacent button shifts only 0.03% in opposite direction
- SNR at 0.5 N: >47 dB (minimum recommended: 20 dB)
- SNR formula: SNR = 20 × log₁₀(Code change from baseline / (6 × σ_noise))

## Relevance to LDC1614 Implementation
The most comprehensive reference for ToM button design with the LDC1614. Directly informs driver requirements including: IDRIVE configuration for 1.3 Vp amplitude, RCOUNT settings for desired sample rate, settle count programming (registers 0x10–0x13), duty-cycling via sleep/shutdown modes, and INTB interrupt-driven data readout. The multi-button algorithm pattern (moving average + dynamic threshold) is the recommended firmware approach.
