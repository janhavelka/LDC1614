# How to Configure a Multichannel LDC System — Part 1: Channel Selection
**Source:** configure_part1.pdf | **Doc #:** SSZTCR4 | **Pages:** 3

## Key Takeaways
- The LDC1614 supports two operating modes: single-channel (continuous) and multichannel (sequential round-robin)
- HIGH_CURRENT_DRV is only available in single-channel mode on channel 0
- DEGLITCH setting is shared across all channels — must be set above the highest-frequency channel
- Multichannel designs reduce system cost, improve matching, and lower power consumption
- One channel can serve as a reference coil for temperature compensation or threshold calibration

## Summary
The LDC1312/LDC1314/LDC1612/LDC1614 multichannel LDCs can operate in single-channel or multichannel mode. In single-channel mode, the device continuously converts on one selected channel. In multichannel mode, the device sequences through selected channels in round-robin fashion. Multichannel mode enables differential designs where matched channels cancel common-mode errors, or where one channel provides a reference measurement.

The internal deglitch filter reduces sensitivity to sensor noise. Since it is shared across all channels, the bandwidth must be set high enough to accommodate the highest-frequency sensor in the system, even if other channels use lower frequencies.

## Technical Details

### Single-Channel (Continuous) Mode
| Step | Register | Bits | Setting | Description |
|---|---|---|---|---|
| 1 | MUX_CONFIG (0x1B) | [15] AUTOSCAN_EN | 0 | Disable autoscan → single-channel mode |
| 2 | CONFIG (0x1A) | [15:14] ACTIVE_CHAN | 00–11 | Select active channel (00=CH0, 01=CH1, 10=CH2, 11=CH3) |

- When AUTOSCAN_EN = 0, RR_SEQUENCE (0x1B [14:13]) has no effect
- HIGH_CURRENT_DRV (0x1A [6]) is **only available** in single-channel mode for channel 0

### Multichannel (Sequential) Mode
| Step | Register | Bits | Setting | Description |
|---|---|---|---|---|
| 1 | MUX_CONFIG (0x1B) | [15] AUTOSCAN_EN | 1 | Enable autoscan → multichannel mode |
| 2 | MUX_CONFIG (0x1B) | [14:13] RR_SEQUENCE | 00/01/10 | Select channel sequence |

- When AUTOSCAN_EN = 1, ACTIVE_CHAN (0x1A [15:14]) has no effect

#### RR_SEQUENCE Channel Selection (LDC1314/LDC1614)
| RR_SEQUENCE | Channels Enabled |
|---|---|
| 00 | CH0, CH1 (2-channel devices: LDC1312/LDC1612) |
| 01 | CH0, CH1, CH2 (3 channels) |
| 10 | CH0, CH1, CH2, CH3 (all 4 channels) |

### Deglitch Filter
| Register | Bits | Description |
|---|---|---|
| MUX_CONFIG (0x1B) | [2:0] DEGLITCH | Internal noise-rejection filter bandwidth |

**Rule:** Set DEGLITCH bandwidth to the lowest value that is still **above** the highest sensor oscillation frequency across all enabled channels.

### Multichannel Benefits
- Single IC for multiple sensors → lower cost, simpler routing
- Well-matched channel parasitics → high-precision differential designs
- Reference coil channel (fixed target or no target) → temperature compensation, threshold setting, distance normalization
- Reduced system overhead → lower power consumption

## Relevance to LDC1614 Implementation
For the LDC1614, all four channels are available in multichannel mode with RR_SEQUENCE = 10. Register configuration must be performed while the device is in sleep mode (SLEEP_MODE_EN = 1). If using a single channel for rapid continuous conversion, note that HIGH_CURRENT_DRV on channel 0 may improve sensor Rp for low-Q sensors. The DEGLITCH setting is a common source of configuration errors — always verify it against the actual sensor frequencies before enabling conversion.
