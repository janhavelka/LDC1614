# Inductive Sensing: Rethink the Button
**Source:** rethink_button_design.pdf | **Doc #:** SSZTBS6 | **Pages:** 3

## Key Takeaways
- Touch-on-Metal (ToM) buttons using LDC technology eliminate moving parts and are immune to moisture, contaminants, and glove interference — problems that plague both mechanical and capacitive buttons.
- The LDC1612/LDC1614 can detect sub-micrometer metal deflections caused by light finger presses on a flat metal surface, requiring no visible deformation.
- A 20 mm diameter button on 0.25 mm thick aluminum at 0.55 mm nominal distance produces ~5 µm deflection from a typical button press — easily detected at 155 SPS with 13 bits of resolution.
- A configurable threshold in firmware determines press sensitivity; haptic feedback can acknowledge the button event.

## Summary
Inductance-to-digital conversion with Touch-on-Metal technology enables buttons made from a single piece of metal with no moving parts. The PCB coil sensor is mounted inside the enclosure, protected from the environment. When a user presses on the metal surface, microscopic deflection changes the inductance of the sensor coil, which the LDC converts to a digital code.

The article demonstrates a specific example: a 20 mm circular button on 0.25 mm thick aluminum sheet, with the sensor coil at 0.55 mm nominal distance. A typical finger press produces approximately 5 µm of deflection. The LDC1612, configured for 155 SPS and 13-bit effective resolution, reliably detects this deflection. A microcontroller compares the LDC output against a pre-set threshold to determine button press events and can trigger haptic feedback.

## Technical Details
- **Button diameter:** 20 mm
- **Metal:** 0.25 mm thick aluminum
- **Nominal sensor-to-metal distance:** 0.55 mm
- **Typical deflection from press:** ~5 µm
- **LDC configuration:** 155 SPS, 13 bits effective resolution
- **Detection method:** Threshold comparison on raw LDC output code; threshold adjustable for weak/strong press sensitivity
- **Feedback:** Haptic feedback triggered by MCU upon threshold crossing

## Relevance to LDC1614 Implementation
Demonstrates the core Touch-on-Metal use case for the LDC1612/LDC1614 family. The same principles apply to multi-button designs using the quad-channel LDC1614. Key driver requirements: configurable sample rate and resolution tradeoff, threshold-based detection in firmware, and I2C data streaming for real-time button event processing.
