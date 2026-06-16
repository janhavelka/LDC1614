# How to Configure a Multichannel LDC System — Part 2: Timing
**Source:** configure_part2.pdf | **Doc #:** SSZTCQ7 | **Pages:** 4

## Key Takeaways
- LDC multichannel timing is fully deterministic — sample readiness can be calculated without polling DRDY
- Total dwell time per channel = sensor-activation time + conversion time + channel-switch delay
- Sensor-activation time depends on sensor Q-factor and oscillation frequency; configured per-channel via SETTLECOUNT_CHn
- Conversion time is set per-channel via RCOUNT_CHn (range: 80 to 1,048,560 FREF cycles)
- In multichannel mode, sensors are automatically powered off when not in use, requiring re-activation each cycle

## Summary
The multichannel LDC1312/LDC1314/LDC1612/LDC1614 devices have fully deterministic conversion timing. Each conversion cycle consists of three phases: sensor activation (settling), frequency measurement (conversion), and channel switching. The DRDY signal indicates new data availability, but because timing is deterministic, polling can be replaced by calculation.

In single-channel mode, sensor activation occurs only once when exiting sleep mode. In multichannel mode, each channel's sensor is powered down between uses, so the activation phase recurs every time the LDC switches to that channel. Conversion time controls the trade-off between sample rate and measurement precision — shorter conversions yield faster rates but noisier data.

## Technical Details

### Timing Components

#### 1. Wake-up Time
- **Shutdown → Sleep:** Device wake-up time (fixed, see datasheet)
- **Sleep → Active:** Wake-up from sleep time (fixed, see datasheet)

#### 2. Sensor-Activation Time (per channel)
| Register | Address | Description |
|---|---|---|
| SETTLECOUNT_CH0 | 0x10 | Settle time for channel 0 |
| SETTLECOUNT_CH1 | 0x11 | Settle time for channel 1 |
| SETTLECOUNT_CH2 | 0x12 | Settle time for channel 2 |
| SETTLECOUNT_CH3 | 0x13 | Settle time for channel 3 |

**Rules:**
- High Q-factor sensors need **longer** settle times
- High oscillation frequency sensors settle **faster**
- Too short → degraded measurement performance
- Too long → no quality impact, but reduces sample rate
- **Single-channel mode:** applies once on exiting sleep
- **Multichannel mode:** applies every channel switch (sensors auto-power-off when inactive)

#### 3. Conversion Time (per channel)
| Register | Address | Description |
|---|---|---|
| RCOUNT_CH0 | 0x08 | Reference count for channel 0 |
| RCOUNT_CH1 | 0x09 | Reference count for channel 1 |
| RCOUNT_CH2 | 0x0A | Reference count for channel 2 |
| RCOUNT_CH3 | 0x0B | Reference count for channel 3 |

**Conversion time range (at f_REF = 40 MHz):**
- Minimum: 80 FREF cycles → 2 µs
- Maximum: 1,048,560 FREF cycles → 26.2 ms

**Trade-off:** Faster conversion → higher sample rate, lower precision. Each channel can have independent RCOUNT settings.

#### 4. Channel-Switch Delay
- Applies in **multichannel mode only**
- Used to shut down the current sensor and switch to the next channel in the round-robin sequence

### Timing Formula

$$t_{dwell,channel} = t_{settle} + t_{conversion} + t_{switch}$$

$$t_{full\_cycle} = N_{channels} \times t_{dwell,channel}$$

### Worked Example
Configuration: CHn_RCOUNT = 0x08 (128 FREF cycles)

| Phase | Duration |
|---|---|
| Sensor-activation time | 1.8 ms |
| Conversion time | 3.2 ms |
| Channel-switch delay | 0.75 ms |
| **Per-channel dwell time** | **16.75 ms** |

| Mode | Full Cycle Time |
|---|---|
| Dual-channel (AUTOSCAN_EN=1, RR_SEQUENCE=00) | 33.5 ms |
| Quad-channel (AUTOSCAN_EN=1, RR_SEQUENCE=10) | 67.0 ms |

### Register Configuration Summary (Multichannel)

| Register | Address | Purpose |
|---|---|---|
| RCOUNT_CH0–CH3 | 0x08–0x0B | Conversion time per channel |
| SETTLECOUNT_CH0–CH3 | 0x10–0x13 | Sensor activation time per channel |
| MUX_CONFIG | 0x1B [15] | AUTOSCAN_EN: 1 = multichannel |
| MUX_CONFIG | 0x1B [14:13] | RR_SEQUENCE: channel count |

## Relevance to LDC1614 Implementation
When implementing an LDC1614 driver, the RCOUNT and SETTLECOUNT registers are the primary knobs for tuning the sample-rate vs. precision trade-off. For a 4-channel system, total cycle time is 4× the per-channel dwell time — plan data readout accordingly. If using interrupt-driven reads (DRDY pin), the deterministic timing allows you to calculate exact interrupt intervals. If polling, use the timing formula to set appropriate polling intervals and avoid reading stale data. Different channels can have different RCOUNT/SETTLECOUNT values to match heterogeneous sensor designs.
