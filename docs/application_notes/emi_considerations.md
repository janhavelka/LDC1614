# EMI Considerations for Inductive Sensing
**Source:** emi_considerations.pdf | **Doc #:** SNOA962 | **Pages:** 6

## Key Takeaways
- LDC1614 has inherent narrowband EMI immunity but requires proper shielding, filtering, and routing for EMC compliance
- Three primary mitigation techniques for LDC1614: coil/trace shielding, passive filters (common-mode chokes + shunt caps), and supply/return routing
- A solid ground plane beneath components can improve emissions by up to 40 dB
- Coil shields must use cut-outs (orthogonal lines to sensor traces) — never use a solid ground plane over the sensor
- EMI failures below 300 MHz typically originate from external cabling; above 300 MHz from PCB routing/shielding issues

## Summary
This application note details EMI reduction techniques for TI's LDC devices, including the LDC1614. The LDC architecture provides inherent narrowband noise immunity and includes internal EMI/deglitch filters. However, the sensor coil intentionally radiates a magnetic field, and PCB traces, wires, and the coil itself can act as antennas at high frequencies. Long supply wires are typical radiation sources at low frequencies.

Three categories of mitigation are covered: shielding (trace shielding on inner PCB layers, coil shielding with cut-out ground planes, solid ground plane beneath components), passive filtering (common-mode chokes placed close to the device, shunt capacitors much smaller than sensor capacitor), and supply/return routing (clean power supply, proper bypass capacitor selection, minimizing trace antenna effects).

## Technical Details
### Shielding
- **Trace shielding:** Route signal traces on middle PCB layers with ground shield above and below
- **Remote sensors:** Use short, shielded, twisted-pair cables
- **Coil shielding:** Use ground plane with cut-outs (orthogonal lines to sensor traces) — prevents eddy currents while blocking E-field
- **Never** use solid ground plane directly over sensor (blocks B-field, prevents sensing)
- **Component ground plane:** Solid un-cut ground plane beneath all components — up to 40 dB emission improvement

### Passive Filters
- **Common-mode chokes:** Place as close to LDC device as possible; produces high-impedance node to block common-mode signals
- **Example choke:** SRF3216-222Y (used on LDC EVMs)
- **Shunt capacitors:** Place near device in conjunction with chokes; footprints pre-implemented on LDC131x/161x EVMs
- **Capacitor sizing rule:** Shunt caps must be much smaller than sensor capacitor to avoid sensitivity loss
- **EVM filter network:** 18 pF shunt capacitors on sensor pins, 330 pF capacitors on I2C lines (per LDC1612 EVM schematic)

### Supply/Return Routing
- **Rise/fall times:** Fast edges create EMI even at frequencies outside normal operating range
- **Bypass capacitors:** Improper values can create impedance resonances making supply lines more susceptible to EMI
- **Trace routing:** Keep traces short; long traces act as antennas. Refer to LDC Calculator Tool for maximum recommended trace length
- **Return path:** Solid return path directly underneath signal trace; minimize cuts/splits in ground plane

### Troubleshooting EMC Failures
| Failure Frequency | Likely Source | Mitigation |
|---|---|---|
| < 300 MHz | External cabling (λ ≥ 1 m) | Add ferrite beads to cables |
| > 300 MHz | PCB routing/shielding/bypassing | Board re-spin with improved layout |

## Relevance to LDC1614 Implementation
For LDC1614 PCB design, the key rules are: (1) route sensor traces on inner layers with ground shields, (2) add common-mode choke footprints on sensor lines close to the IC, (3) include 18 pF shunt capacitor footprints on sensor pins (do not populate unless needed), (4) maintain solid ground plane under the LDC1614, (5) keep sensor-to-IC trace length within LDC Calculator Tool limits. The EVM schematic filter network (18 pF shunts + 330 pF on I2C) serves as a reference design for EMI filtering.
