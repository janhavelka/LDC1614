# How to Use the LDC Calculations Tool
**Source:** using_calculation_tools.pdf | **Doc #:** SSZTAN1 | **Pages:** 5

## Key Takeaways
- TI provides a single consolidated Excel spreadsheet with multiple LDC calculation tools (no macros required)
- Yellow fields are user inputs; orange fields are calculated results — do not modify orange cells
- The Quick Sensor L/C/f Calculator on the Contents tab computes inductance, capacitance, or oscillation frequency from any two known values
- The Rp/Rs/Q Calculator converts between series resistance (Rs), parallel resistance (Rp), and Q-factor
- Individual calculator tabs are self-contained and can be copied to other spreadsheets

## Summary
The LDC Calculations Tool is a consolidated Excel spreadsheet providing multiple design calculators for TI's inductance-to-digital converter family. Each calculator resides on a separate tab, accessible from the Contents tab. The tool requires no macros and works directly in Microsoft Excel.

Users enter parameters in yellow-highlighted fields and read results from orange-highlighted fields. The Contents tab includes two inline calculators: the Quick Sensor L/C/f Calculator (for computing any one of sensor frequency, inductance, or capacitance given the other two) and the Rp/Rs/Q Calculator (for converting between parallel resistance, series resistance, and Q-factor). Both can be copied as standalone 3×3 cell blocks into other spreadsheets.

## Technical Details

### Available Calculator Tabs

| Tab | Purpose |
|---|---|
| **Racetrack Inductor Designer** | Design circular or racetrack-shaped PCB sensor coils |
| **Sample Rate Calculator** | Calculate sample rates for various LDC devices |
| **Inductance and Frequency from Output Code** | Compute sensor inductance, resonant frequency, and Rp from device output code |
| **Skin Depth Calculation** | Determine skin depth for target thickness/material analysis |
| **LDC161x/LDC131x Current Consumption Estimator** | Estimate current draw with sleep/shutdown modes |
| **LDC131x/LDC161x Sensor Configuration** | Calculate IDRIVE settings for sensor drive |
| **Remote Coil Maximum Distance Calculator** | Determine max cable length between LDC and remote sensor |
| **LDC0851 Calculator Tool** | Sample rate, current consumption, switching distance for LDC0851 |
| **LDC1101 Rp Configuration** | Calculate RP_MIN, RP_MAX, T1, C1, T2, C2 for LDC1101 |
| **Spring Sensor Calculator Tool** | Estimate performance when using a spring as the LDC sensor |

### Quick Sensor L/C/f Calculator
Computes one of three parameters given the other two:
- **f_sensor** — sensor oscillation frequency
- **L** — sensor inductance
- **C** — sensor capacitance

Relationship: $f_{sensor} = \frac{1}{2\pi\sqrt{LC}}$

### Rp/Rs/Q Calculator
Converts between:
- **Rp** — parallel resistance
- **Rs** — series resistance  
- **Q** — quality factor

## Relevance to LDC1614 Implementation
The LDC161x/LDC131x-specific tabs (Sensor Configuration, Current Consumption Estimator, Sample Rate Calculator) are directly applicable to LDC1614 designs. The Sensor Configuration tab calculates IDRIVE register values. The Sample Rate Calculator determines achievable conversion rates given RCOUNT and SETTLECOUNT settings. Use the Quick L/C/f Calculator to verify sensor frequency is within the LDC1614's valid range (1 kHz – 10 MHz for full accuracy mode) when selecting sensor capacitor values.
