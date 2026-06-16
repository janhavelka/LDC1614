# Inductive Sensing: Target Size Matters
**Source:** target_size.pdf | **Doc #:** SSZTBX4 | **Pages:** 4

## Key Takeaways
- Target size relative to coil size directly impacts sensing range and sensitivity
- Targets smaller than the coil diameter produce significantly reduced frequency shifts
- Targets larger than the coil diameter yield diminishing returns in sensitivity beyond ~100% coil area coverage
- A 10×10 mm target on a 29 mm coil achieves only 1.1% of maximum frequency shift at 5 mm distance
- Findings apply to all LDC devices: LDC1312, LDC1314, LDC1612, LDC1614, LDC0851, LDC1101, LDC2114, LDC2112, LDC3114-Q1

## Summary
This article investigates the relationship between conductive target size and inductive sensing range using the LDC1612 with a 29 mm diameter coil ("coil J" from the reference coil board EVM, sensing area 660.5 mm²). Copper tape targets ranging from 3×3 mm to 50×50 mm were measured across distances from 0.1 mm to 14.5 mm (half the coil diameter).

The results demonstrate that larger targets produce significantly higher sensitivity, manifesting as either increased sensing range or improved system accuracy. However, increasing target size beyond the coil diameter provides diminishing improvements. The data shows that a 30×30 mm target (136% of coil area) achieves nearly the same frequency shift at 12.5 mm distance as a 10×10 mm target (15% of coil area) achieves at only 5 mm.

## Technical Details

### Test Configuration
- **Coil:** "Coil J" from reference coil board EVM, 29 mm diameter (660.5 mm² area)
- **Device:** LDC1612
- **Travel range:** 0.1 mm to 14.5 mm (half coil diameter)
- **Target material:** Copper tape

### Target Size vs. Coil Area Coverage

| Target (mm) | Area (mm²) | % of Coil Area |
|---|---|---|
| 3×3 | 9 | 1.4% |
| 5×5 | 25 | 3.8% |
| 8×8 | 64 | 9.7% |
| 10×10 | 100 | 15.1% |
| 15×15 | 225 | 34.1% |
| 20×20 | 400 | 60.6% |
| 25×25 | 625 | 94.6% |
| 30×30 | 900 | 136.3% |
| 35×35 | 1,225 | 185.5% |
| 40×40 | 1,600 | 242.2% |
| 50×50 | 2,500 | 378.5% |

### Key Observations
- At 5 mm distance: 10×10 mm target → 1.1% of max frequency shift
- At 12.5 mm distance: 30×30 mm target → same 1.1% frequency shift
- Sensitivity gain saturates once target area exceeds ~100% of coil area

## Relevance to LDC1614 Implementation
When designing an LDC1614 sensing system, target geometry must be matched to coil size. For optimal sensitivity, the conductive target should be at least as large as the coil's outer diameter. Using undersize targets severely reduces sensing range and resolution. If mechanical constraints limit target size, compensate by using a smaller coil — though this also reduces absolute sensing range. This trade-off is critical during the mechanical design phase of any LDC1614-based proximity, position, or button-sensing application.
