# How to Design an Inductive Sensor with WEBENCH Coil Designer
**Source:** webench_coil_design.pdf | **Doc #:** SSZTCB1 | **Pages:** 6

## Key Takeaways
- WEBENCH Coil Designer generates custom PCB sensor coils with full layout export in ~5 minutes
- Supports circular, hexagonal, octagonal, and square coil shapes
- Enforces device-specific boundary conditions (frequency limits, drive constraints) and warns on violations
- A coil fill ratio (inner diameter / outer diameter) ≥ 0.3 is recommended — smaller ratios degrade Q without significant inductance gain
- Exports directly to Altium, Eagle, KiCad, PADS, and OrCAD PCB layout tools

## Summary
WEBENCH Coil Designer is TI's online tool for designing custom PCB sensor coils for LDC devices. It complements the WEBENCH Inductive Sensing Designer (which suggests coils from input parameters) by giving the designer direct control over physical coil properties including trace width, spacing, number of turns, layers, and outer diameter. The tool validates designs against the selected LDC device's operating limits and warns when sensor frequency or other parameters exceed boundaries.

The design process follows five steps: select device, choose coil shape, define geometry and PCB parameters, review output plots, and export the layout. The tool compares performance across coil shapes and generates output graphs for any selected parameter pair, enabling rapid design-space exploration.

## Technical Details

### 5-Step Design Process

**Step 1 — Select LDC Device**
Choose the target device (e.g., LDC1612). The tool applies device-specific constraints (max/min frequency, drive current limits).

**Step 2 — Select Coil Type**

| Shape | Characteristics |
|---|---|
| Circular | Highest Q-factor; preferred for most applications |
| Hexagonal | Intermediate Q; fits some geometries better |
| Octagonal | Intermediate Q; closer to circular with straight-line routing |
| Square | Lowest Q; useful when high inductance is needed in constrained area |

**Step 3 — Define Coil Geometry**
- **Trace width / trace spacing:** Narrower = more expensive PCB fabrication
- **Outer coil diameter:** Largest impact on maximum sensing range
- **Number of turns / layers:** More turns = higher inductance
- **Coil fill ratio:** inner diameter / outer diameter ≥ 0.3 recommended

> Innermost turns add little inductance relative to their AC resistance increase → diminished Q.

**Step 4 — Review Output Graphs**
- Sensor parameters plotted against input variables
- Cross-coil-type comparison available via drop-down menus
- Device boundary violations shown as warnings with recommended values

**Step 5 — Export Design**
- Supported formats: Altium Designer, Eagle, KiCad, PADS, OrCAD
- Exported file contains full PCB coil layout ready for integration

### Design Guidelines
- Coil fill ratio ≥ 0.3 for general use
- Metal Touch applications may use lower fill ratios
- Circular shape preferred unless mechanical constraints dictate otherwise
- Thicker copper (1 oz+) improves Q-factor
- Verify sensor frequency is within selected device's operating range

## Relevance to LDC1614 Implementation
When designing PCB coils for the LDC1614, select "LDC1612/4" in WEBENCH to apply the correct frequency and drive-current constraints. The exported layout can be placed directly into your PCB design. For multi-channel LDC1614 systems, design each channel's coil independently — different channels may use different coil sizes or shapes depending on their sensing requirements. After export, verify the coil parameters with the Racetrack Inductor Designer spreadsheet tool and confirm Rp, Q, and f_sensor are within the LDC1614's operating region for both no-target and closest-target conditions.
