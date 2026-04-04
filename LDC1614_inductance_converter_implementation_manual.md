# LDC1614 / LDC1612 — Comprehensive Implementation Extraction

## 1. Source Documents

| # | Document | TI Literature # | Pages | Role |
|---|----------|----------------|-------|------|
| 1 | LDC1612, LDC1614 Multi-Channel 28-Bit Inductance to Digital Converter Datasheet | SNOSCY9A (Dec 2014, Rev Mar 2018) | 67 | Primary datasheet |
| 2 | Configuring Inductive-to-Digital-Converters for Parallel Resistance (RP) Variation in L-C Tank Sensors | SNAA221B (Apr 2015, Rev Nov 2019) | 11 | RP configuration app note |
| 3 | Setting LDC1312/4, LDC1612/4, and LDC1101 Sensor Drive Configuration | SNOA950 (Apr 2016) | 8 | IDRIVE/sensor amplitude app note |
| 4 | LDC1312, LDC1314, LDC1612, LDC1614 Sensor Status Monitoring | SNOA959 (Oct 2016) | 14 | Status/error reporting app note |
| 5 | Inductive Sensing: Improve the ENOB of a Multichannel LDC by 4 Bits in 3 Simple Steps | SSZTCK1 (Jun 2015) | 5 | ENOB/gain/offset (LDC131x focus) |
| 6 | Measuring Rp of an L-C Sensor for Inductive Sensing | SNOA936 (Oct 2015) | 4 | RP measurement methods |
| 7 | Common Inductive and Capacitive Sensing Applications | SLYA048B (Mar 2020, Rev Jun 2021) | 19 | General LDC application overview |
| 8 | EMI Considerations for Inductive Sensing | SNOA962 (Feb 2017) | 6 | EMI design techniques |
| 9 | How to Configure a Multichannel LDC System – Part 1 | SSZTCR4 (May 2015) | 3 | Channel selection howto |
| 10 | How to Configure a Multichannel LDC System – Part 2 | SSZTCQ7 (May 2015) | 4 | Timing/conversion howto |
| 11 | Sensor Design for Inductive Sensing Applications Using LDC | SNOA930C (Mar 2015, Rev May 2021) | 23 | Coil/sensor design |
| 12 | LDC Target Design | SNOA957B (Sep 2016, Rev Jun 2021) | 11 | Target design |
| 13 | Inductive Sensing: How Far Can I Sense? | SSZTCL3 (Jun 2015) | 4 | Sensing distance |
| 14 | Inductive Sensing Touch-On-Metal Buttons Design Guide | SNOA951 (Jun 2016) | 21 | Touch-on-metal design |
| 15 | Inductive Sensing: Rethink the Button | SSZTBS6 (Dec 2015) | 3 | Touch button overview |
| 16 | LDC1612/LDC1614 Linear Position Sensing | SNOA931A (Apr 2015, Rev Nov 2019) | 15 | Linear position app note |
| 17 | Inductive Sensing: Target Size Matters | SSZTBX4 (Nov 2015) | 4 | Target size effects |
| 18 | LDC Reference Coils User's Guide | SNOU136 (May 2015) | 33 | Reference coil characterization |
| 19 | WEBENCH Coil Designer How-to | SSZTCB1 (Aug 2015) | 6 | Coil design tool |
| 20 | Stacked Coils for Switch Applications (WEBENCH) | SSZTAZ8 (Aug 2016) | 5 | Stacked coil design |
| 21 | How to Sense Spring Compression | SSZTCH7 (Jul 2015) | 4 | Spring sensor howto |
| 22 | Simulate Inductive Sensors Using FEMM | (undated) | 20 | FEM simulation guide |
| 23 | How to Use the LDC Calculations Tool | SSZTAN1 (Nov 2016) | 5 | Calculator spreadsheet |
| 24 | How You Can Use the LDC Racetrack Inductor Designer Tool | SSZTAK7 (Nov 2016) | 6 | Racetrack tool howto |

---

## 2. Device Identity and Variants

| Parameter | LDC1612 | LDC1614 |
|-----------|---------|---------|
| Channels | 2 (Ch 0, Ch 1) | 4 (Ch 0–3) |
| Resolution | 28-bit | 28-bit |
| Package | WSON-12 (DNT), 4 mm × 4 mm | WQFN-16 (RGH), 4 mm × 4 mm |
| Pin count | 12 + DAP | 16 + DAP |
| Sensor pins | IN0A/B, IN1A/B | IN0A/B, IN1A/B, IN2A/B, IN3A/B |
| Part marking | LDC1612DNT | LDC1614RGH |
| MANUFACTURER_ID | 0x5449 ("TI") | 0x5449 ("TI") |
| DEVICE_ID | 0x3055 | 0x3055 |

*(Datasheet SNOSCY9A, pp. 1, 4, 33)*

Pin-compatible family members (12-bit): LDC1312 (2-ch), LDC1314 (4-ch). Same register map with 12-bit data vs 28-bit data. *(Datasheet p. 1)*

The LDC1612 and LDC1614 share the same DEVICE_ID (0x3055). There is **no register-level method** to distinguish LDC1612 from LDC1614 — the firmware must know which variant is populated. Channels 2/3 registers exist in both variants but are only functional on the LDC1614. *(Datasheet pp. 15, 33)*

---

## 3. High-Level Functional Summary

The LDC1612/LDC1614 is a multi-channel inductance-to-digital converter (LDC) for inductive sensing. It measures the oscillation frequency of external LC resonator(s) and outputs a 28-bit digital value proportional to ƒ_SENSOR / ƒ_REF. *(Datasheet p. 10)*

Key capabilities:
- Sensor frequency range: 1 kHz to 10 MHz *(Datasheet p. 1, 6)*
- Sensor RP range: 1 kΩ to 100 kΩ (normal mode); down to 250 Ω (high-current drive, Ch0 only) *(Datasheet p. 6)*
- Configurable per-channel conversion time: ~1.2 µs to >26.2 ms *(Datasheet p. 11)*
- Single-channel continuous or multi-channel sequential (round-robin) operation *(Datasheet p. 10)*
- Internal 43 MHz reference oscillator or external clock up to 40 MHz *(Datasheet pp. 6, 11)*
- I2C interface up to 400 kHz *(Datasheet p. 7)*
- Interrupt output (INTB) for data-ready and error conditions *(Datasheet p. 4)*
- Sleep mode (35 µA typ), Shutdown mode (200 nA typ) *(Datasheet p. 6)*

---

## 4. Interface Summary

### I2C

| Parameter | Value | Source |
|-----------|-------|--------|
| Type | Standard I2C, 7-bit address | Datasheet p. 13 |
| Max speed | 400 kbit/s (Fast-mode) | Datasheet p. 7 |
| Min speed | 10 kHz | Datasheet p. 7 |
| Address (ADDR=L) | **0x2A** | Datasheet p. 4 |
| Address (ADDR=H) | **0x2B** | Datasheet p. 4 |
| Register width | 16 bits (all registers) | Datasheet p. 13 |
| Clock stretching | **Not used** | Datasheet p. 13 |
| Pull-ups | Required on SCL and SDA (open-drain) | Datasheet p. 4 |

**I2C Protocol:**
- **Write:** START → slave addr + W → ACK → register addr → ACK → data MSB → ACK → data LSB → ACK → STOP *(Datasheet p. 13, Fig. 11)*
- **Read:** START → slave addr + W → ACK → register addr → ACK → REPEATED START → slave addr + R → ACK → data MSB → ACK → data LSB → NACK → STOP *(Datasheet p. 13, Fig. 12)*

**I2C Timing (Fast-mode):** *(Datasheet p. 7)*

| Parameter | Symbol | Min | Max | Unit |
|-----------|--------|-----|-----|------|
| Clock frequency | ƒ_SCL | 10 | 400 | kHz |
| Clock low time | t_LOW | 1.3 | — | µs |
| Clock high time | t_HIGH | 0.6 | — | µs |
| Hold time START | t_HD;STA | 0.6 | — | µs |
| Setup time repeated START | t_SU;STA | 0.6 | — | µs |
| Data hold time | t_HD;DAT | 0 | — | µs |
| Data setup time | t_SU;DAT | 100 | — | ns |
| Setup time STOP | t_SU;STO | 0.6 | — | µs |
| Bus free time | t_BUF | 1.3 | — | µs |
| Data valid time | t_VD;DAT | — | 0.9 | µs |
| Spike suppression | t_SP | — | 50 | ns |
| Input high | V_IH | 0.7×VDD | — | V |
| Input low | V_IL | — | 0.3×VDD | V |
| Output low (3 mA sink) | V_OL | — | 0.4 | V |
| Hysteresis | HYS | 0.1×VDD | — | V |

### I2C Caveats

1. **No early termination:** STOP before ACK is not supported and may corrupt the current or next transaction. *(Datasheet p. 14)*
2. **SDA pulse sensitivity:** Any extraneous pulse on SDA during SCL low of the first bit of the address byte must be avoided. Pulse width must not exceed t_SP (50 ns). *(Datasheet p. 14, Fig. 13)*
3. **Multi-register read coherency:** Read DATA_MSB first, then DATA_LSB. Reading MSB latches both MSB and LSB from shadow register. Reading LSB alone returns stale data from the last MSB read. *(Datasheet p. 14)*

---

## 5. Electrical and Timing Constraints Relevant to Software

### Power Supply
| Parameter | Min | Typ | Max | Unit | Source |
|-----------|-----|-----|-----|------|--------|
| VDD | 2.7 | — | 3.6 | V | Datasheet p. 5 |
| IDD (active, 10 MHz ref) | — | 2.1 | — | mA | Datasheet p. 6 |
| IDD Sleep | — | 35 | 60 | µA | Datasheet p. 6 |
| IDD Shutdown (SD=VDD) | — | 0.2 | 1 | µA | Datasheet p. 6 |

*(Note: IDD does not include sensor current or I2C pull-up current)* *(Datasheet p. 6)*

### Timing
| Parameter | Value | Source |
|-----------|-------|--------|
| Wake-up time (SD pin low → I2C ready) | 2 ms (typ) | Datasheet p. 7 |
| Watchdog timeout / sensor recovery | 5.2 ms (typ) | Datasheet p. 7 |
| Sleep→Active first conversion start | 16384 / ƒ_INT after SLEEP_MODE_EN→0 | Datasheet p. 12 |
| Channel switch delay | 692 ns + 5/ƒ_REF | Datasheet p. 39 |

### Sensor Electrical Limits
| Parameter | Min | Max | Unit | Source |
|-----------|-----|-----|------|--------|
| Sensor frequency | 0.001 | 10 | MHz | Datasheet p. 6 |
| Sensor RP (normal) | 1 | 100 | kΩ | Datasheet p. 6 |
| Sensor RP (HIGH_CURRENT_DRV) | 0.25 | — | kΩ | Datasheet p. 6 |
| Max sensor current (normal) | — | 1.5 | mA | Datasheet p. 6 |
| Max sensor current (HIGH_CURRENT_DRV) | — | 6 | mA | Datasheet p. 6 |
| Max oscillation amplitude (peak) | — | 1.8 | V | Datasheet p. 6 |
| Sensor pin input capacitance | — | 4 | pF | Datasheet p. 6 |
| Max channel sample rate (single, 400kHz I2C) | — | 4.08 | kSPS | Datasheet p. 6 |

### Reference Clock
| Parameter | Min | Typ | Max | Unit | Source |
|-----------|-----|-----|-----|------|--------|
| External CLKIN frequency | 2 | — | 40 | MHz | Datasheet p. 6 |
| CLKIN duty cycle | 40% | — | 60% | — | Datasheet p. 6 |
| Internal oscillator frequency | 35 | 43.4 | 55 | MHz | Datasheet p. 7 |
| Internal oscillator temp coeff | — | -13 | — | ppm/°C | Datasheet p. 7 |

---

## 6. Power, Reset, Enable, and Startup Behavior

### Power-On Reset (POR)
On power-up, the device enters **Sleep Mode** with all registers at default values. *(Datasheet p. 12)*

### Software Reset
Write `0x8000` to RESET_DEV register (0x1C). All registers return to defaults, active conversions stop. The RESET_DEV bit always reads back 0. *(Datasheet p. 31)*

### Shutdown Mode (SD pin)
- Asserted by SD pin = HIGH. Lowest power state (200 nA typ). *(Datasheet pp. 4, 6, 12)*
- All registers reset to defaults. *(Datasheet p. 12)*
- I2C bus is **non-functional** during shutdown. *(Datasheet p. 12)*
- INTB is actively driven high (de-asserted). *(Datasheet p. 12)*
- ADDR pin may be changed during shutdown. *(Datasheet p. 12)*
- Exit shutdown by pulling SD low → device enters Sleep Mode after ~2 ms wake-up time. *(Datasheet p. 7)*

### Sleep Mode (Configuration Mode)
- Entered by CONFIG.SLEEP_MODE_EN = 1 (default after POR). *(Datasheet p. 12)*
- Device configuration retained, no conversions performed. *(Datasheet p. 12)*
- I2C interface is fully functional for reads/writes. *(Datasheet p. 12)*
- Entering sleep clears all conversion results, error conditions, and de-asserts INTB. *(Datasheet p. 12)*
- **Recommended:** Configure all registers while in Sleep Mode. *(Datasheet p. 12)*

### Startup Sequence
1. Power on or SD=LOW → device in Sleep Mode (default register values).
2. Write all configuration registers (RCOUNT, SETTLECOUNT, CLOCK_DIVIDERS, DRIVE_CURRENT, ERROR_CONFIG, MUX_CONFIG). *(Datasheet pp. 50–51)*
3. Write CONFIG register last (sets SLEEP_MODE_EN=0 to begin conversions). *(Datasheet p. 51)*
4. After SLEEP_MODE_EN→0, first sensor activation begins after 16384/ƒ_INT elapses (~377 µs at 43.4 MHz). *(Datasheet p. 12)*
5. **CONFIG must be written last** because device configuration changes are not permitted while in active mode. *(Datasheet p. 51)*

---

## 7. Pin Behavior Relevant to Firmware

### ADDR (pin 4)
- Input, must not float. *(Datasheet p. 4)*
- LOW → I2C address 0x2A; HIGH → I2C address 0x2B. *(Datasheet p. 4)*
- Thresholds: V_IL < 0.3×VDD, V_IH > 0.7×VDD. *(Datasheet p. 6)*
- May be changed while in Shutdown Mode. *(Datasheet p. 12)*

### SD (pin 6)
- Input, must not float. *(Datasheet p. 4)*
- LOW → normal operation; HIGH → Shutdown Mode. *(Datasheet p. 4)*
- Thresholds: V_IL < 0.3×VDD, V_IH > 0.7×VDD. *(Datasheet p. 6)*

### INTB (pin 5)
- Push-pull output, no pull-up required. *(Datasheet p. 4)*
- Active LOW when asserted (data ready or error). *(Datasheet p. 6)*
- V_OL ≤ 0.4 V (3 mA sink); V_OH ≥ 2.4 V. *(Datasheet p. 6)*
- Controlled by CONFIG.INTB_DIS: 0 = enabled, 1 = disabled (held high). *(Datasheet p. 30)*
- De-asserted by: reading STATUS register, entering Sleep Mode, POR, Shutdown, or S/W reset. *(Datasheet pp. 6, 47; SNOA959 p. 6)*

### CLKIN (pin 3)
- External reference clock input. *(Datasheet p. 4)*
- Tie to GND if using internal oscillator. *(Datasheet p. 4)*
- Thresholds: V_CLKIN_LO < 0.3×VDD, V_CLKIN_HI > 0.7×VDD. *(Datasheet p. 6)*

### INxA / INxB (sensor pins)
- Analog connections to external LC sensor. *(Datasheet p. 4)*
- Inactive channels tied to GND by ~10 Ω internally. *(Datasheet p. 43)*
- Unused INxA/INxB pins may be left no-connect. *(Datasheet p. 43)*

### DAP (Die Attach Pad)
- Internally connected to GND. Should be connected to GND for best performance but not used as primary ground. *(Datasheet p. 4)*

---

## 8. Register Map Overview

All registers are 16-bit wide. *(Datasheet p. 13)*

| Address | Name | Default | R/W | Description |
|---------|------|---------|-----|-------------|
| 0x00 | DATA0_MSB | 0x0000 | R | Ch 0 MSB conversion result + error flags |
| 0x01 | DATA0_LSB | 0x0000 | R | Ch 0 LSB conversion result |
| 0x02 | DATA1_MSB | 0x0000 | R | Ch 1 MSB conversion result + error flags |
| 0x03 | DATA1_LSB | 0x0000 | R | Ch 1 LSB conversion result |
| 0x04 | DATA2_MSB | 0x0000 | R | Ch 2 MSB result + error flags (LDC1614 only) |
| 0x05 | DATA2_LSB | 0x0000 | R | Ch 2 LSB result (LDC1614 only) |
| 0x06 | DATA3_MSB | 0x0000 | R | Ch 3 MSB result + error flags (LDC1614 only) |
| 0x07 | DATA3_LSB | 0x0000 | R | Ch 3 LSB result (LDC1614 only) |
| 0x08 | RCOUNT0 | 0x0080 | R/W | Ch 0 Reference Count |
| 0x09 | RCOUNT1 | 0x0080 | R/W | Ch 1 Reference Count |
| 0x0A | RCOUNT2 | 0x0080 | R/W | Ch 2 Reference Count (LDC1614 only) |
| 0x0B | RCOUNT3 | 0x0080 | R/W | Ch 3 Reference Count (LDC1614 only) |
| 0x0C | OFFSET0 | 0x0000 | R/W | Ch 0 Conversion Offset |
| 0x0D | OFFSET1 | 0x0000 | R/W | Ch 1 Conversion Offset |
| 0x0E | OFFSET2 | 0x0000 | R/W | Ch 2 Conversion Offset (LDC1614 only) |
| 0x0F | OFFSET3 | 0x0000 | R/W | Ch 3 Conversion Offset (LDC1614 only) |
| 0x10 | SETTLECOUNT0 | 0x0000 | R/W | Ch 0 Settling Reference Count |
| 0x11 | SETTLECOUNT1 | 0x0000 | R/W | Ch 1 Settling Reference Count |
| 0x12 | SETTLECOUNT2 | 0x0000 | R/W | Ch 2 Settling Reference Count (LDC1614 only) |
| 0x13 | SETTLECOUNT3 | 0x0000 | R/W | Ch 3 Settling Reference Count (LDC1614 only) |
| 0x14 | CLOCK_DIVIDERS0 | 0x0000 | R/W | Ch 0 Reference & Sensor Dividers |
| 0x15 | CLOCK_DIVIDERS1 | 0x0000 | R/W | Ch 1 Reference & Sensor Dividers |
| 0x16 | CLOCK_DIVIDERS2 | 0x0000 | R/W | Ch 2 Reference & Sensor Dividers (LDC1614 only) |
| 0x17 | CLOCK_DIVIDERS3 | 0x0000 | R/W | Ch 3 Reference & Sensor Dividers (LDC1614 only) |
| 0x18 | STATUS | 0x0000 | R | Device Status Report |
| 0x19 | ERROR_CONFIG | 0x0000 | R/W | Error Reporting Configuration |
| 0x1A | CONFIG | 0x2801 | R/W | Conversion Configuration |
| 0x1B | MUX_CONFIG | 0x020F | R/W | Channel Multiplexing Configuration |
| 0x1C | RESET_DEV | 0x0000 | R/W | Reset Device |
| 0x1D | — | — | — | *(Reserved / not documented)* |
| 0x1E | DRIVE_CURRENT0 | 0x0000 | R/W* | Ch 0 Sensor Current Drive |
| 0x1F | DRIVE_CURRENT1 | 0x0000 | R/W* | Ch 1 Sensor Current Drive |
| 0x20 | DRIVE_CURRENT2 | 0x0000 | R/W* | Ch 2 Sensor Current Drive (LDC1614 only) |
| 0x21 | DRIVE_CURRENT3 | 0x0000 | R/W* | Ch 3 Sensor Current Drive (LDC1614 only) |
| 0x7E | MANUFACTURER_ID | 0x5449 | R | Manufacturer ID |
| 0x7F | DEVICE_ID | 0x3055 | R | Device ID |

\* DRIVE_CURRENTx registers have both R/W (IDRIVE) and R-only (INIT_IDRIVE) fields.

*(Datasheet pp. 15–33)*

---

## 9. Detailed Register and Bitfield Breakdown

### 9.1 DATAx_MSB (0x00, 0x02, 0x04, 0x06)

| Bits | Field | Type | Reset | Description |
|------|-------|------|-------|-------------|
| 15 | ERR_URx | R | 0 | Under-range error flag. Cleared on read. |
| 14 | ERR_ORx | R | 0 | Over-range error flag. Cleared on read. |
| 13 | ERR_WDx | R | 0 | Watchdog timeout error flag. Cleared on read. |
| 12 | ERR_AEx | R | 0 | Amplitude error flag (OR of high+low if both enabled in ERROR_CONFIG). Cleared on read. |
| 11:0 | DATAx[27:16] | R | 0x000 | 12 MSBs of 28-bit conversion result. |

Error bits in DATAx_MSB are **not sticky** — cleared if next conversion on that channel has no error. Also cleared by reading the register. *(Datasheet p. 16; SNOA959 p. 4)*

If error reporting is not enabled in ERROR_CONFIG, the error flags in DATAx_MSB will not be set. *(SNOA959 p. 3)*

*(Datasheet pp. 16–19)*

### 9.2 DATAx_LSB (0x01, 0x03, 0x05, 0x07)

| Bits | Field | Type | Reset | Description |
|------|-------|------|-------|-------------|
| 15:0 | DATAx[15:0] | R | 0x0000 | 16 LSBs of 28-bit conversion result. |

**Must be read after DATAx_MSB** for data coherency. If read alone, returns stale data from last MSB read. *(Datasheet pp. 16–19)*

Full 28-bit result: `DATAx = DATAx_MSB[11:0] << 16 | DATAx_LSB[15:0]` *(Datasheet p. 38)*

- Value `0x0000000` = under-range condition *(Datasheet p. 38)*
- Value `0xFFFFFFF` = over-range condition *(Datasheet p. 38)*

### 9.3 RCOUNTx (0x08–0x0B)

| Bits | Field | Type | Reset | Description |
|------|-------|------|-------|-------------|
| 15:0 | RCOUNTx | R/W | 0x0080 | Reference count for conversion interval. |

- Values 0x0000–0x0004: **Reserved** — do not use.
- Values 0x0005–0xFFFF: `Conversion Time (tCx) = (RCOUNTx × 16) / ƒ_REFx`
- Higher RCOUNT → longer conversion → higher resolution.
- Maximum setting 0xFFFF required for full 28-bit resolution.
- Minimum usable: 0x0005 (80 ƒ_REF cycles → ~2 µs at 40 MHz).

*(Datasheet pp. 20–21, 39)*

### 9.4 OFFSETx (0x0C–0x0F)

| Bits | Field | Type | Reset | Description |
|------|-------|------|-------|-------------|
| 15:0 | OFFSETx | R/W | 0x0000 | Frequency offset subtracted from conversion. |

`ƒ_OFFSETx = (OFFSETx / 2^16) × ƒ_REFx`

The offset should be set to less than ƒ_SENSOR_MIN / ƒ_REFx to avoid masking changing LSBs. *(Datasheet pp. 21–22, 39)*

### 9.5 SETTLECOUNTx (0x10–0x13)

| Bits | Field | Type | Reset | Description |
|------|-------|------|-------|-------------|
| 15:0 | SETTLECOUNTx | R/W | 0x0000 | Settling time before conversion. |

- 0x0000: Settle Time = 32 / ƒ_REFx (minimum)
- 0x0001: Settle Time = 32 / ƒ_REFx (minimum)
- 0x0002–0xFFFF: `Settle Time (tSx) = (SETTLECOUNTx × 16) / ƒ_REFx`

**Minimum requirement:**
`SETTLECOUNTx ≥ Q_SENSOR × ƒ_REFx / (16 × ƒ_SENSORx)` — round up to next integer. *(Datasheet p. 40)*

If amplitude has not settled before conversion, an Amplitude error is generated. *(Datasheet pp. 22–24)*

### 9.6 CLOCK_DIVIDERSx (0x14–0x17)

| Bits | Field | Type | Reset | Description |
|------|-------|------|-------|-------------|
| 15:12 | FIN_DIVIDERx | R/W | 0x0 | Sensor input frequency divider. |
| 11:10 | RESERVED | R/W | 0b00 | Must be set to 0b00. |
| 9:0 | FREF_DIVIDERx | R/W | 0x000 | Reference clock divider. |

**FIN_DIVIDERx:**
- 0x0 (0b0000): **Reserved — do not use.**
- ≥0x1: `ƒ_INx = ƒ_SENSORx / FIN_DIVIDERx`
- Must be ≥2 if ƒ_SENSOR ≥ 8.75 MHz. *(Datasheet p. 24)*

**FREF_DIVIDERx:**
- 0x000: **Reserved — do not use.**
- ≥0x001: `ƒ_REFx = ƒ_CLK / FREF_DIVIDERx`

*(Datasheet pp. 24–26)*

### 9.7 STATUS (0x18) — Read-Only

| Bits | Field | Type | Reset | Description |
|------|-------|------|-------|-------------|
| 15:14 | ERR_CHAN | R | 0b00 | Channel that generated the flag/error. 00=Ch0, 01=Ch1, 10=Ch2, 11=Ch3. |
| 13 | ERR_UR | R | 0 | Under-range error. Sticky until STATUS read. |
| 12 | ERR_OR | R | 0 | Over-range error. Sticky until STATUS read. |
| 11 | ERR_WD | R | 0 | Watchdog timeout error. Sticky until STATUS read. |
| 10 | ERR_AHE | R | 0 | Amplitude High error (sensor amp > ~1.8 V). Sticky. |
| 9 | ERR_ALE | R | 0 | Amplitude Low error (sensor amp < ~1.2 V). Sticky. |
| 8 | ERR_ZC | R | 0 | Zero Count error. Sticky. |
| 7 | Reserved | R | 0 | Reads 0. |
| 6 | DRDY | R | 0 | Data Ready flag. In single-ch: per conversion. In multi-ch: after all selected channels complete. |
| 5:4 | Reserved | R | 0b00 | Reads 0. |
| 3 | UNREADCONV0 | R | 0 | Ch 0 unread conversion present. Cleared by reading DATA0_MSB. |
| 2 | UNREADCONV1 | R | 0 | Ch 1 unread conversion present. Cleared by reading DATA1_MSB. |
| 1 | UNREADCONV2 | R | 0 | Ch 2 unread conversion present (LDC1614). |
| 0 | UNREADCONV3 | R | 0 | Ch 3 unread conversion present (LDC1614). |

All bits except UNREADCONVx are **sticky** — must be cleared by reading STATUS. Reading STATUS also de-asserts INTB. *(Datasheet pp. 26–27; SNOA959 p. 5)*

UNREADCONVx flags are **not sticky** — cleared by reading corresponding DATAx_MSB register or STATUS register. *(SNOA959 p. 7)*

*(Datasheet pp. 26–27)*

### 9.8 ERROR_CONFIG (0x19)

| Bits | Field | Type | Reset | Description |
|------|-------|------|-------|-------------|
| 15 | UR_ERR2OUT | R/W | 0 | Report under-range errors in DATAx_MSB.ERR_URx. |
| 14 | OR_ERR2OUT | R/W | 0 | Report over-range errors in DATAx_MSB.ERR_ORx. |
| 13 | WD_ERR2OUT | R/W | 0 | Report watchdog errors in DATAx_MSB.ERR_WDx. |
| 12 | AH_ERR2OUT | R/W | 0 | Report amplitude high errors in DATAx_MSB.ERR_AEx. |
| 11 | AL_ERR2OUT | R/W | 0 | Report amplitude low errors in DATAx_MSB.ERR_AEx. |
| 10:8 | Reserved | R/W | 0b000 | Reserved. Set to 0b000. |
| 7 | UR_ERR2INT | R/W | 0 | Report under-range errors → INTB + STATUS.ERR_UR. |
| 6 | OR_ERR2INT | R/W | 0 | Report over-range errors → INTB + STATUS.ERR_OR. |
| 5 | WD_ERR2INT | R/W | 0 | Report watchdog errors → INTB + STATUS.ERR_WD. |
| 4 | AH_ERR2INT | R/W | 0 | Report amplitude high errors → INTB + STATUS.ERR_AHE. |
| 3 | AL_ERR2INT | R/W | 0 | Report amplitude low errors → INTB + STATUS.ERR_ALE. |
| 2 | ZC_ERR2INT | R/W | 0 | Report zero count errors → INTB + STATUS.ERR_ZC. |
| 1 | Reserved | R/W | 0 | Reserved. Set to 0. |
| 0 | DRDY_2INT | R/W | 0 | Report data ready → INTB + STATUS.DRDY. |

**Note:** If both AH_ERR2OUT=1 and AL_ERR2OUT=1, the ERR_AEx bit in DATAx_MSB reports the logical OR of both amplitude warnings. *(Datasheet pp. 28–29; SNOA959 p. 4)*

*(Datasheet pp. 28–29)*

### 9.9 CONFIG (0x1A)

Default: **0x2801** (SLEEP_MODE_EN=1, SENSOR_ACTIVATE_SEL=1, RESERVED[5:0]=0b000001)

| Bits | Field | Type | Reset | Description |
|------|-------|------|-------|-------------|
| 15:14 | ACTIVE_CHAN | R/W | 0b00 | Active channel for single-ch mode. 00=Ch0, 01=Ch1, 10=Ch2, 11=Ch3. Ignored if AUTOSCAN_EN=1. |
| 13 | SLEEP_MODE_EN | R/W | 1 | 0=Active (conversions), 1=Sleep Mode. |
| 12 | RP_OVERRIDE_EN | R/W | 0 | 0=Auto-calibration, 1=Use IDRIVEx for fixed drive current (**recommended for normal operation**). |
| 11 | SENSOR_ACTIVATE_SEL | R/W | 1 | 0=Full Current Activation (max drive during settle, faster), 1=Low Power Activation (uses IDRIVEx during settle). |
| 10 | AUTO_AMP_DIS | R/W | 0 | 0=Auto amplitude correction enabled, 1=Disabled (**recommended for precision applications**). |
| 9 | REF_CLK_SRC | R/W | 0 | 0=Internal oscillator, 1=External CLKIN. |
| 8 | RESERVED | R/W | 0 | Reserved. Set to 0. |
| 7 | INTB_DIS | R/W | 0 | 0=INTB enabled, 1=INTB disabled (held high). |
| 6 | HIGH_CURRENT_DRV | R/W | 0 | 0=Normal current (1.5 mA max), 1=High current for Ch0 only (>1.5 mA, up to ~6 mA). **Only in single-channel mode (AUTOSCAN_EN=0).** |
| 5:0 | RESERVED | R/W | 0b000001 | **Must be set to 0b000001.** |

*(Datasheet pp. 29–30)*

### 9.10 MUX_CONFIG (0x1B)

Default: **0x020F** (AUTOSCAN_EN=0, RR_SEQUENCE=0b00, RESERVED=0b0001000001, DEGLITCH=0b111)

| Bits | Field | Type | Reset | Description |
|------|-------|------|-------|-------------|
| 15 | AUTOSCAN_EN | R/W | 0 | 0=Single-channel continuous, 1=Multi-channel sequential (auto-scan). |
| 14:13 | RR_SEQUENCE | R/W | 0b00 | Round-robin sequence: 00=Ch0,Ch1; 01=Ch0,Ch1,Ch2 (LDC1614); 10=Ch0,Ch1,Ch2,Ch3 (LDC1614). **Note:** 11 maps to Ch0,Ch1 (same as 00). |
| 12:3 | RESERVED | R/W | 0b00_0100_0001 | **Must be set to 0b0001000001** (= 0x041 in bits 12:3). |
| 2:0 | DEGLITCH | R/W | 0b111 | Input deglitch filter bandwidth. |

**DEGLITCH values:**

| Value | Bandwidth |
|-------|-----------|
| 0b001 | 1.0 MHz |
| 0b100 | 3.3 MHz |
| 0b101 | 10 MHz |
| 0b111 | 33 MHz |

Select the lowest setting that exceeds the highest sensor frequency across all active channels. *(Datasheet pp. 30–31; configure_part1 p. 2)*

*(Datasheet pp. 30–31)*

### 9.11 RESET_DEV (0x1C)

| Bits | Field | Type | Reset | Description |
|------|-------|------|-------|-------------|
| 15 | RESET_DEV | R/W | 0 | Write 1 to reset device. Always reads 0. |
| 14:0 | RESERVED | R/W | 0x0000 | Must be 0. |

*(Datasheet p. 31)*

### 9.12 DRIVE_CURRENTx (0x1E–0x21)

| Bits | Field | Type | Reset | Description |
|------|-------|------|-------|-------------|
| 15:11 | IDRIVEx | R/W | 0b00000 | Sensor drive current (5-bit, 0–31). Used when RP_OVERRIDE_EN=1 and AUTO_AMP_DIS=1. |
| 10:6 | INIT_IDRIVEx | R | 0b00000 | Initial drive current measured during auto-calibration. Updated each amplitude correction if AUTO_AMP_DIS=0. **Set to 0b00000 when writing register.** |
| 5:0 | RESERVED | R/W | 0b000000 | Reserved. Set to 0b000000. |

**IDRIVE current table (32 levels):** *(Datasheet pp. 42–43; SNOA950 pp. 2–3)*

| IDRIVEx | Nominal Current (µA) | Min Sensor RP (kΩ) | Max Sensor RP (kΩ) |
|---------|---------------------|--------------------|--------------------|
| 0 | 16 | 60.0 | 90.0 |
| 1 | 18 | 51.8 | 77.6 |
| 2 | 20 | 44.6 | 66.9 |
| 3 | 23 | 38.4 | 57.6 |
| 4 | 28 | 33.7 | 49.7 |
| 5 | 32 | 29.5 | 42.8 |
| 6 | 40 | 23.6 | 36.9 |
| 7 | 46 | 20.5 | 31.8 |
| 8 | 52 | 18.1 | 27.4 |
| 9 | 59 | 16.1 | 23.6 |
| 10 | 72 | 13.1 | 20.4 |
| 11 | 82 | 11.5 | 17.6 |
| 12 | 95 | 9.92 | 15.1 |
| 13 | 110 | 8.57 | 13.0 |
| 14 | 127 | 7.42 | 11.2 |
| 15 | 146 | 6.46 | 9.69 |
| 16 | 169 | 5.58 | 8.35 |
| 17 | 195 | 4.83 | 7.20 |
| 18 | 212 | 4.45 | 6.21 |
| 19 | 244 | 3.86 | 5.35 |
| 20 | 297 | 3.17 | 4.61 |
| 21 | 342 | 2.76 | 3.97 |
| 22 | 424 | 2.22 | 3.42 |
| 23 | 489 | 1.93 | 2.95 |
| 24 | 551 | 1.71 | 2.54 |
| 25 | 635 | 1.48 | 2.19 |
| 26 | 763 | 1.24 | 1.89 |
| 27 | 880 | 1.07 | 1.63 |
| 28 | 1017 | 0.93 | 1.40 |
| 29 | 1173 | 0.80 | 1.21 |
| 30 | 1355 | 0.70 | 1.05 |
| 31 | 1563 | 0.60 | 0.90 |

**IDRIVE calculation:** `IDRIVE = π × V_P / (4 × R_P)` where V_P = target peak sensor voltage (recommend 1.2–1.8 V). *(Datasheet p. 42)*

**Preferred IDRIVE:** Highest value that keeps V_OSC ≤ 1.8 V_P with target at maximum operating distance. *(SNOA950 p. 3)*

*(Datasheet pp. 31–33)*

### 9.13 MANUFACTURER_ID (0x7E) — Read-Only

| Bits | Field | Type | Reset | Description |
|------|-------|------|-------|-------------|
| 15:0 | MANUFACTURER_ID | R | 0x5449 | "TI" in ASCII. |

### 9.14 DEVICE_ID (0x7F) — Read-Only

| Bits | Field | Type | Reset | Description |
|------|-------|------|-------|-------------|
| 15:0 | DEVICE_ID | R | 0x3055 | Shared between LDC1612 and LDC1614. |

*(Datasheet p. 33)*

---

## 10. Commands and Transaction-Level Behaviors

The LDC1612/1614 has no "command" registers per se — all operations are performed through register writes and reads.

### Write Transaction
Standard I2C 16-bit register write as described in Section 4. *(Datasheet p. 13, Fig. 11)*

### Read Transaction
Standard I2C 16-bit register read with repeated START. *(Datasheet p. 13, Fig. 12)*

### Multi-Register Data Read Coherency
When DATAx_MSB is read, the device latches both MSB and LSB from an internal shadow register into the I2C-accessible DATAx_MSB and DATAx_LSB registers. Therefore:
1. Always read DATAx_MSB first.
2. Then read DATAx_LSB.
3. If only DATAx_LSB is read, it returns the value from the last DATAx_MSB read. *(Datasheet p. 14)*

### Register Write Restrictions
- For registers with mixed R and R/W fields, write the reset value to R-only fields. *(Datasheet p. 15)*
- Configuration registers should only be written while in Sleep Mode. *(Datasheet p. 12)*
- CONFIG register must be written last during initialization (it takes the device out of sleep). *(Datasheet p. 51)*

---

## 11. Initialization and Configuration Sequences

### 11.1 Single-Channel Continuous Mode — Example Init Sequence

*(From Datasheet pp. 50–51, Table 47)*

```
1. Write RCOUNT0 (0x08) = desired value (e.g. 0x04D6)
2. Write SETTLECOUNT0 (0x10) = desired value (e.g. 0x000A)
3. Write CLOCK_DIVIDERS0 (0x14) = FIN_DIV | FREF_DIV (e.g. 0x1002)
4. Write ERROR_CONFIG (0x19) = 0x0000 (or enable desired error reporting)
5. Write MUX_CONFIG (0x1B) = 0x020C (single-ch, DEGLITCH=3.3MHz)
6. Write DRIVE_CURRENT0 (0x1E) = IDRIVE<<11 (e.g. 0x9000 for IDRIVE=18)
7. Write CONFIG (0x1A) = 0x1601 (ACTIVE_CHAN=00, SLEEP_MODE_EN=0,
      RP_OVERRIDE_EN=1, SENSOR_ACTIVATE_SEL=0, AUTO_AMP_DIS=1,
      REF_CLK_SRC=1, INTB_DIS=0, HIGH_CURRENT_DRV=0, RESERVED=0x01)
   ** Must be last write — starts conversions **
```

### 11.2 Multi-Channel Sequential Mode — Example Init Sequence

*(From Datasheet pp. 50–51, Table 48)*

```
1. Write RCOUNT0 (0x08) and RCOUNT1 (0x09) — both channels
2. Write SETTLECOUNT0 (0x10) and SETTLECOUNT1 (0x11) — both channels
3. Write CLOCK_DIVIDERS0 (0x14) and CLOCK_DIVIDERS1 (0x15) — both channels
4. Write ERROR_CONFIG (0x19) = 0x0000
5. Write MUX_CONFIG (0x1B) = 0x820C (AUTOSCAN_EN=1, RR_SEQUENCE=00,
      DEGLITCH=3.3MHz)
6. Write DRIVE_CURRENT0 (0x1E) and DRIVE_CURRENT1 (0x1F)
7. Write CONFIG (0x1A) = 0x1601 (RP_OVERRIDE_EN=1, AUTO_AMP_DIS=1,
      SENSOR_ACTIVATE_SEL=0, REF_CLK_SRC=1, SLEEP_MODE_EN=0)
   ** Must be last write **
```

### 11.3 IDRIVE Auto-Calibration Sequence

*(From Datasheet p. 44; SNAA221B pp. 7–8)*

```
1. Place target at maximum operating distance from sensor.
2. Set CONFIG.SLEEP_MODE_EN = 1 (Sleep Mode).
3. Program SETTLECOUNTx and RCOUNTx.
4. Set CONFIG.RP_OVERRIDE_EN = 0 (auto-calibration enabled).
5. Set CONFIG.SLEEP_MODE_EN = 0 (start conversion).
6. Wait for at least one conversion to complete (target stable at max distance).
7. Read DRIVE_CURRENTx register → save INIT_IDRIVEx field (bits 10:6).
8. For normal operation: write saved value into IDRIVEx (bits 15:11).
9. Set CONFIG.RP_OVERRIDE_EN = 1 (fixed drive).
```

### 11.4 Changing Configuration at Runtime

*(Datasheet p. 12)*

```
1. Set CONFIG.SLEEP_MODE_EN = 1 (return to Sleep Mode).
2. Change desired registers.
3. Set CONFIG.SLEEP_MODE_EN = 0 (resume conversions).
```

---

## 12. Operating Modes and State Machine Behavior

### Mode Transitions

```
Power-On → Sleep Mode (default)
         ↕ SLEEP_MODE_EN bit
       Active (Conversion) Mode
         
SD pin HIGH → Shutdown Mode → SD pin LOW → Sleep Mode (after 2ms)

Software Reset (RESET_DEV) → Sleep Mode (default config)
```

### Sleep Mode
- CONFIG.SLEEP_MODE_EN = 1. *(Datasheet p. 12)*
- Configuration retained, no conversions.
- I2C functional.
- All DATA registers cleared, errors cleared, INTB de-asserted.
- Current: 35 µA typ. *(Datasheet p. 6)*

### Active (Conversion) Mode
- CONFIG.SLEEP_MODE_EN = 0. *(Datasheet p. 12)*
- Conversion starts after 16384/ƒ_INT delay.
- Two sub-modes:
  - **Single-channel continuous:** AUTOSCAN_EN=0, selected by ACTIVE_CHAN.
  - **Multi-channel sequential:** AUTOSCAN_EN=1, sequence per RR_SEQUENCE.

### Shutdown Mode
- SD pin = HIGH. *(Datasheet p. 12)*
- All registers reset. I2C non-functional.
- Current: 200 nA typ. *(Datasheet p. 6)*

### Single-Channel Mode Timing
```
[Sensor Activation] → [Conversion] → [Amplitude Correction*] → [Conversion] → ...
* Only if AUTO_AMP_DIS=0
```
Sensor activation happens **once** when entering Active Mode. After that, conversions repeat continuously. *(Datasheet p. 38, Fig. 54; configure_part2 p. 2)*

### Multi-Channel Mode Timing
```
[Ch0 Settle] → [Ch0 Convert] → [Switch Delay] →
[Ch1 Settle] → [Ch1 Convert] → [Switch Delay] →
[Ch0 Settle] → [Ch0 Convert] → ... (round-robin)
```
Per-channel dwell time = Settle Time + Conversion Time + Switch Delay. *(Datasheet p. 40; configure_part2 p. 3)*

Channel switch delay = 692 ns + 5/ƒ_REF. *(Datasheet p. 39)*

Inactive channel sensor pins (INxA/INxB) are tied to ground (~10 Ω). *(Datasheet p. 43)*

---

## 13. Measurement / Data Path Behavior

### Frequency Measurement
The LDC measures the ratio of sensor frequency to reference frequency:

`DATAx = (ƒ_SENSORx / ƒ_REFx) × 2^28` *(Datasheet p. 38)*

More precisely, with dividers and offset:

`ƒ_SENSORx = (DATAx / 2^28) × ƒ_REFx × FIN_DIVIDERx + ƒ_OFFSETx × FIN_DIVIDERx`

where `ƒ_OFFSETx = (OFFSETx / 2^16) × ƒ_REFx` *(Datasheet p. 39)*

With FIN_DIVIDERx=1, FREF_DIVIDERx=1, OFFSETx=0:
`ƒ_SENSORx = (DATAx / 2^28) × ƒ_CLK` *(Datasheet p. 38)*

### Conversion Time Formula
`tCx = (RCOUNTx × 16 + 4) / ƒ_REFx` *(Datasheet p. 39)*

### Settling Time Formula
`tSx = (SETTLECOUNTx × 16) / ƒ_REFx` *(Datasheet p. 40)*

Minimum requirement: `SETTLECOUNTx ≥ Q_SENSOR × ƒ_REFx / (16 × ƒ_SENSORx)` *(Datasheet p. 40)*

### Overall Dwell Time Per Channel (Multi-Channel)
`t_dwell = tSx + tCx + t_switch`
where `t_switch = 692 ns + 5/ƒ_REF` *(Datasheet pp. 39–40)*

### Clock Requirements

| Mode | Reference Source | Max ƒ_REFx | ƒ_INx requirement | Min SETTLECOUNTx | Min RCOUNTx |
|------|-----------------|------------|-------------------|-----------------|-------------|
| Multi-channel | Internal | ≤55 MHz | < ƒ_REFx / 4 | > 3 | > 8 |
| Multi-channel | External | ≤40 MHz | < ƒ_REFx / 4 | > 3 | > 8 |
| Single-channel | Either | ≤35 MHz | — | — | — |

*(Datasheet p. 46)*

FIN_DIVIDERx must be ≥2 if ƒ_SENSOR ≥ 8.75 MHz. *(Datasheet p. 24)*

### Data Output Format
28-bit unsigned result split across two 16-bit registers:
- DATAx_MSB[11:0] = bits 27:16
- DATAx_LSB[15:0] = bits 15:0
- DATAx_MSB[15:12] = error flags (not part of data)

Full reconstruction: `DATAx = (DATAx_MSB & 0x0FFF) << 16 | DATAx_LSB`

### Frequency Offset
`ƒ_OFFSETx = (OFFSETx / 65536) × ƒ_REFx` *(Datasheet p. 39)*

Offset is subtracted from the measured frequency digitally. Useful for maximizing dynamic range when sensor frequency varies over a narrow range. *(Datasheet p. 39)*

### Inductor Self-Resonant Frequency
Sensor frequency should be kept below 80% of the inductor's self-resonant frequency (SRF): `ƒ_SENSOR < 0.8 × SRF`. *(Datasheet p. 53)*

### Sensor Oscillation Amplitude
- Optimal range: 1.2 V_P to 1.8 V_P. *(SNOA950 p. 3)*
- Above 1.8 V_P: internal ESD clamp activates, causes frequency shift and invalid data. *(Datasheet p. 41; SNOA950 p. 3)*
- Below ~0.5 V_P: oscillation may be unstable or stop entirely. *(SNOA950 p. 3)*
- Below 1.2 V_P: degraded SNR. *(SNOA950 p. 3)*

### Sensor RP and Drive Current Relationship
`V_OSC = IDRIVE × π × R_P / 4` *(SNOA950 p. 2)*

`R_P = L / (R_S × C)` *(SNOA930C p. 3)*

---

## 14. Interrupts, Alerts, Status, and Faults

### INTB Pin Behavior
- Active low, push-pull output. *(Datasheet p. 4)*
- Asserted when enabled conditions occur (error or data ready). *(Datasheet p. 47)*
- Must be enabled: CONFIG.INTB_DIS = 0 AND appropriate bits in ERROR_CONFIG set. *(SNOA959 p. 6)*

### Conditions Reportable via INTB

| Condition | ERROR_CONFIG bit to enable | STATUS bit set | DATAx_MSB bit |
|-----------|--------------------------|----------------|---------------|
| Data ready | DRDY_2INT (bit 0) | DRDY (bit 6) | N/A |
| Under-range | UR_ERR2INT (bit 7) | ERR_UR (bit 13) | ERR_URx (bit 15) if UR_ERR2OUT=1 |
| Over-range | OR_ERR2INT (bit 6) | ERR_OR (bit 12) | ERR_ORx (bit 14) if OR_ERR2OUT=1 |
| Watchdog timeout | WD_ERR2INT (bit 5) | ERR_WD (bit 11) | ERR_WDx (bit 13) if WD_ERR2OUT=1 |
| Amplitude high | AH_ERR2INT (bit 4) | ERR_AHE (bit 10) | ERR_AEx (bit 12) if AH_ERR2OUT=1 |
| Amplitude low | AL_ERR2INT (bit 3) | ERR_ALE (bit 9) | ERR_AEx (bit 12) if AL_ERR2OUT=1 |
| Zero count | ZC_ERR2INT (bit 2) | ERR_ZC (bit 8) | N/A |

*(Datasheet pp. 28–29; SNOA959 pp. 3–9)*

### INTB Clear Conditions
1. Reading the STATUS register (clears errors and de-asserts INTB). *(Datasheet p. 47; SNOA959 p. 5)*
2. Entering Sleep Mode. *(SNOA959 p. 6)*
3. Power-on reset (POR). *(SNOA959 p. 6)*
4. Entering Shutdown Mode (SD asserted). *(SNOA959 p. 6)*
5. Software reset. *(SNOA959 p. 6)*

### Data Ready (DRDY)
- **Single-channel mode:** DRDY asserts after each conversion completes. *(SNOA959 p. 7)*
- **Multi-channel mode:** DRDY asserts after the **last** channel in the sequence completes. *(SNOA959 p. 7)*
- Also reportable via INTB if DRDY_2INT=1. *(Datasheet p. 29)*

### Error Details

**Under-range error (ERR_UR):** Output code would be negative after subtracting offset. LDC1612/1614 reports DATA[27:0] = 0x0000000. Fix: reduce OFFSETx or increase RCOUNTx. *(SNOA959 p. 8)*

**Over-range error (ERR_OR):** Sensor frequency exceeds reference frequency. DATA[27:0] = 0xFFFFFFF. Fix: increase ƒ_REF, decrease ƒ_SENSOR, increase FIN_DIVIDERx, or decrease FREF_DIVIDERx. *(SNOA959 p. 8)*

**Watchdog timeout (ERR_WD):** Sensor not oscillating or frequency < 250 Hz. LDC aborts conversion, attempts sensor restart. Recovery time ~5.2 ms. Occurs only in **continuous** (single-channel) mode. Data from a watchdog event is **invalid**. *(SNOA959 p. 8; Datasheet p. 7)*

**Amplitude high (ERR_AHE):** Sensor amplitude > ~1.8 V_P. Reduce IDRIVEx. *(Datasheet p. 27)*

**Amplitude low (ERR_ALE):** Sensor amplitude < ~1.2 V_P. May indicate low RP, sensor damage, or too-low IDRIVEx. *(Datasheet p. 27)*

**Zero count (ERR_ZC):** No sensor oscillations counted during conversion. May precede watchdog timeout. DATA reads 0x0000 or 0x8000. *(SNOA959 p. 9)*

### STATUS Register Multi-Error Behavior
ERR_CHAN records the **first** channel that flagged an error. Subsequent errors from other channels are **not** reported in ERR_CHAN until STATUS is read (which clears it). Use INTB-based reporting + STATUS reads to catch errors from multiple channels. *(SNOA959 pp. 4–5)*

### DATAx Error Flag Behavior
Error bits in DATAx_MSB (bits 15:12):
- Set only if corresponding ERR2OUT bit is enabled in ERROR_CONFIG.
- **Not sticky:** cleared automatically if next conversion on that channel succeeds.
- Also cleared by reading DATAx_MSB.
- Reading DATAx_MSB also clears the corresponding error in STATUS if that channel caused it.

*(SNOA959 p. 4)*

---

## 15. Nonvolatile Memory / OTP / EEPROM Behavior

The LDC1612/LDC1614 has **no nonvolatile memory, OTP, or EEPROM**. All registers are volatile and reset to defaults on:
- Power-on reset
- Software reset (RESET_DEV)
- Entering Shutdown Mode (SD=HIGH)

*(Datasheet pp. 12, 31)*

All configuration must be written by the host MCU after every power-up or reset.

---

## 16. Special Behaviors, Caveats, and Footnotes

1. **CONFIG.RESERVED[5:0] must be 0b000001.** Any other value results in improper operation. *(Datasheet p. 30)*

2. **MUX_CONFIG.RESERVED[12:3] must be 0b0001000001** (decimal 65 in that field). *(Datasheet p. 31)*

3. **CLOCK_DIVIDERSx.RESERVED[11:10] must be 0b00.** *(Datasheet p. 24)*

4. **FIN_DIVIDERx = 0 is reserved** — do not use. Minimum is 1. *(Datasheet p. 24)*

5. **FREF_DIVIDERx = 0 is reserved** — do not use. Minimum is 1. *(Datasheet p. 24)*

6. **RCOUNTx values 0x0000–0x0004 are reserved.** *(Datasheet p. 20)*

7. **HIGH_CURRENT_DRV only works on Channel 0 in single-channel mode** (AUTOSCAN_EN=0). *(Datasheet p. 30)*

8. **RR_SEQUENCE = 0b11 maps to Ch0, Ch1** — same as 0b00. Not documented as enabling all 4 channels. *(Datasheet p. 30)*

9. **Auto-amplitude correction (AUTO_AMP_DIS=0) can introduce output code offsets** — not recommended for precision applications. *(Datasheet p. 44; SNOA950 p. 4)*

10. **Entering Sleep Mode clears all conversion data.** Re-entering Active Mode requires full reconversion cycle. *(Datasheet p. 12)*

11. **INIT_IDRIVEx field should be written as 0b00000** when writing DRIVE_CURRENTx. It is a read-only field used by auto-calibration. *(Datasheet p. 31)*

12. **Internal ESD clamp activates above 1.8 V sensor amplitude** — causes sensor frequency shift and invalid output. *(Datasheet p. 41)*

13. **RP > 90 kΩ:** Place a 100 kΩ resistor in parallel with the inductor to reduce effective RP into drive range. *(Datasheet p. 43)*

14. **Device does not distinguish LDC1612 from LDC1614** via register. DEVICE_ID is the same (0x3055). *(Datasheet p. 33)*

15. **Unused sensor channels:** INxA/INxB pins may be left no-connect. Inactive channels are grounded internally by ~10 Ω. *(Datasheet p. 43)*

16. **I2C SDA pulse avoidance:** Between START and first address bit, any SDA transition longer than t_SP (50 ns) can corrupt communication. *(Datasheet p. 14)*

17. **Watchdog timeout errors occur only in continuous (single-channel) mode,** not in sequential mode. Use zero count and amplitude warnings for stopped-oscillation detection in sequential mode. *(SNOA959 p. 8)*

18. **First STATUS read after entering Active Mode** should be performed only after INTB assertion (if enabled). *(Datasheet p. 47)*

19. **Sensor frequency must stay below 0.8× inductor SRF** to avoid capacitive behavior. *(Datasheet p. 53)*

20. **Bypass capacitor:** 1 µF MLCC (X7R) between VDD and GND, placed close to device. Add 10 µF if supply is far. *(Datasheet p. 53)*

---

## 17. Recommended Polling and Control Strategy Hints from the Docs

### Interrupt-Driven Data Readback (Recommended)
1. Configure ERROR_CONFIG.DRDY_2INT = 1 and CONFIG.INTB_DIS = 0.
2. On INTB assertion (falling edge), read STATUS register.
3. Check DRDY bit; if set, read DATAx_MSB then DATAx_LSB for each active channel.
4. Reading STATUS de-asserts INTB.

*(Datasheet p. 47; SNOA959 pp. 6–7)*

### Polling-Based Data Readback
1. If MCU and LDC share the same clock, calculate exact conversion time and poll at fixed intervals.
2. If not sharing clocks, ensure conversion time > I2C read time to avoid duplicate reads.
3. Use UNREADCONVx flags in STATUS to detect new data.
4. Read at interval matching expected conversion period.

*(SNOA959 p. 7)*

### Multi-Channel Data Readback Strategy
1. Wait for INTB (DRDY for the full sequence).
2. Read all channels: for each channel, read DATAx_MSB then DATAx_LSB.
3. Read promptly after INTB to avoid data being overwritten by next conversion cycle.
4. Monitor UNREADCONVx to detect data loss from delayed reads.

*(Datasheet pp. 47–48, Fig. 58)*

### Error Monitoring Strategy
1. Enable desired error bits in ERROR_CONFIG (both ERR2INT and ERR2OUT).
2. On INTB, read STATUS to get error type and ERR_CHAN.
3. Read the corresponding DATAx_MSB to clear per-channel error flags.
4. This two-level approach catches multi-channel errors. *(SNOA959 pp. 4–6)*

### IDRIVE Configuration Strategy
1. During development: use oscilloscope on INxA to measure amplitude.
2. Set IDRIVE to highest value that keeps V_OSC ≤ 1.8 V_P with target at max distance.
3. For multi-sensor systems with identical sensors: use the **lowest** common IDRIVE across all channels.
4. For production: use auto-calibration sequence once, save INIT_IDRIVEx value, then use fixed IDRIVEx. *(SNOA950 pp. 5–7; SNAA221B pp. 7–8)*

### Power Optimization
- Return to Sleep Mode between measurement bursts for low-duty-cycle applications.
- Sleep Mode current: 35 µA vs Active: ~2.1 mA + sensor current.
- Shutdown Mode for extended idle periods (but must reprogram all registers on wake).

*(Datasheet p. 12)*

---

## 18. Ambiguities, Conflicts, and Missing Information

1. **DEVICE_ID ambiguity:** Both LDC1612 and LDC1614 report DEVICE_ID = 0x3055. No documented register-level distinction between variants. Firmware must be hardcoded for the correct variant.

2. **Auto-calibration sleep/wake bit ordering:** The datasheet (p. 44) lists step 2 as "set SLEEP_MODE_EN to b0" but context says "put into SLEEP mode". The app note SNAA221B (p. 8) correctly says "set SLEEP_MODE_EN to b1" for Sleep and "b0" for Active. **The datasheet step 2/5 descriptions appear swapped** — follow SNAA221B or infer from context (Sleep=1, Active=0).

3. **MUX_CONFIG reserved bits value:** Default 0x020F implies reserved[12:3] = 0b0000000100 and DEGLITCH = 0b111. But the datasheet says reserved must be 0b0001000001. The combined default 0x020F = 0b0000_0010_0000_1111, which has reserved[12:3] = 0b0001000001 and DEGLITCH=0b111. These are consistent when decoded byte-by-byte.

4. **LDC1312/1314 gain/offset feature:** The ENOB improvement via gain and offset (SSZTCK1) applies to LDC1312/1314 only, not LDC1612/1614 which already have 28-bit output. The LDC1612/1614 use OFFSET registers for frequency offset only.

5. **ƒ_INx constraint in single-channel mode:** Table 43 shows ƒ_INx < ƒ_REFx/4 for multi-channel but no explicit constraint for single-channel. The constraint may still apply but is not documented for single-channel.

6. **Channel switch delay formula:** The datasheet gives `692 ns + 5/ƒ_REF` but the configure_part2 article says `~0.75 ms` for their specific configuration. No explicit formula units clarification (the 5/ƒ_REF term with ƒ_REF in Hz gives a time in seconds).

7. **Internal oscillator frequency range:** Specified as 35–55 MHz (min/max over process/temp) with typical 43.4 MHz. The typical value used with watchdog/startup timing functions is not precisely stable across all conditions. *(Datasheet p. 7)*

8. **Conversion time formula discrepancy:** The register description says `tCx = (RCOUNTx × 16) / ƒ_REFx` (without +4), but the application section says `tCx = (RCOUNTx × 16 + 4) / ƒ_REFx`. The +4 term is negligible for large RCOUNT but matters for minimum values.

---

## 19. Raw Implementation Checklist

### Power-On / Initialization
- [ ] Ensure SD pin is LOW for normal operation (not floating).
- [ ] Ensure ADDR pin is tied to a valid level (not floating).
- [ ] Wait for device to be in Sleep Mode after power-up (~2 ms if coming from shutdown).
- [ ] Verify device identity: read MANUFACTURER_ID (0x7E) → expect 0x5449.
- [ ] Verify device identity: read DEVICE_ID (0x7F) → expect 0x3055.

### Configuration (while in Sleep Mode)
- [ ] Write RCOUNTx for each active channel (min 0x0005, max 0xFFFF).
- [ ] Write SETTLECOUNTx for each active channel (calculate from Q, ƒ_SENSOR, ƒ_REF).
- [ ] Write CLOCK_DIVIDERSx: set FIN_DIVIDERx ≥ 1 (≥ 2 if ƒ_SENSOR ≥ 8.75 MHz), set FREF_DIVIDERx ≥ 1, bits[11:10] = 0b00.
- [ ] Write OFFSETx if needed for dynamic range optimization.
- [ ] Write ERROR_CONFIG: enable desired error/DRDY reporting.
- [ ] Write MUX_CONFIG: set AUTOSCAN_EN, RR_SEQUENCE, DEGLITCH. Set reserved[12:3] = 0b0001000001.
- [ ] Write DRIVE_CURRENTx: set IDRIVEx (bits 15:11), INIT_IDRIVEx = 0 (bits 10:6), reserved = 0 (bits 5:0).
- [ ] Write CONFIG **last**: set SLEEP_MODE_EN=0 to start conversion. Set RP_OVERRIDE_EN=1, AUTO_AMP_DIS=1 for normal operation. Set reserved[5:0] = 0b000001.

### Data Readback
- [ ] Wait for INTB assertion (if DRDY_2INT enabled) or poll STATUS.DRDY.
- [ ] For each channel: read DATAx_MSB first, then DATAx_LSB.
- [ ] Extract 28-bit data: `(DATAx_MSB & 0x0FFF) << 16 | DATAx_LSB`.
- [ ] Check error flags in DATAx_MSB[15:12] if error reporting is enabled.
- [ ] Read STATUS register to clear INTB and sticky error bits.

### Error Handling
- [ ] Check STATUS for ERR_UR, ERR_OR, ERR_WD, ERR_AHE, ERR_ALE, ERR_ZC.
- [ ] Use ERR_CHAN to identify which channel caused the error.
- [ ] If watchdog timeout: data is invalid — discard and wait for next conversion.
- [ ] If zero count error: check sensor connections.
- [ ] If amplitude high: reduce IDRIVEx for that channel.
- [ ] If amplitude low: increase IDRIVEx or check sensor RP.
- [ ] If under-range: reduce OFFSETx.
- [ ] If over-range: adjust clock dividers or sensor frequency.

### Reconfiguration
- [ ] Set CONFIG.SLEEP_MODE_EN = 1 before changing registers.
- [ ] Modify registers as needed.
- [ ] Set CONFIG.SLEEP_MODE_EN = 0 to resume (write CONFIG last).

### Shutdown / Power Management
- [ ] For deep sleep: set SD pin HIGH (all registers lost; 200 nA).
- [ ] For light sleep: set CONFIG.SLEEP_MODE_EN = 1 (config retained; 35 µA).
- [ ] After SD low→high→low transition: wait 2 ms, then re-initialize all registers.

---

## 20. Source Citation Appendix

| Fact | Source | Page |
|------|--------|------|
| I2C address 0x2A/0x2B | Datasheet SNOSCY9A | p. 4 |
| I2C max speed 400 kHz | Datasheet SNOSCY9A | p. 7 |
| No clock stretching | Datasheet SNOSCY9A | p. 13 |
| 16-bit register width | Datasheet SNOSCY9A | p. 13 |
| Data coherency (read MSB first) | Datasheet SNOSCY9A | p. 14 |
| SDA pulse avoidance | Datasheet SNOSCY9A | p. 14 |
| No early termination support | Datasheet SNOSCY9A | p. 14 |
| VDD 2.7–3.6 V | Datasheet SNOSCY9A | p. 5 |
| IDD active 2.1 mA | Datasheet SNOSCY9A | p. 6 |
| Sleep current 35 µA | Datasheet SNOSCY9A | p. 6 |
| Shutdown current 200 nA | Datasheet SNOSCY9A | p. 6 |
| Wake-up time 2 ms | Datasheet SNOSCY9A | p. 7 |
| Watchdog recovery 5.2 ms | Datasheet SNOSCY9A | p. 7 |
| Internal osc 43.4 MHz typ | Datasheet SNOSCY9A | p. 7 |
| Internal osc TC -13 ppm/°C | Datasheet SNOSCY9A | p. 7 |
| External CLKIN 2–40 MHz | Datasheet SNOSCY9A | p. 6 |
| Sensor frequency 1 kHz–10 MHz | Datasheet SNOSCY9A | p. 6 |
| Sensor RP 1–100 kΩ | Datasheet SNOSCY9A | p. 6 |
| High-current RP min 250 Ω | Datasheet SNOSCY9A | p. 6 |
| Max sensor current 1.5 mA | Datasheet SNOSCY9A | p. 6 |
| High-current drive max 6 mA | Datasheet SNOSCY9A | p. 6 |
| Max oscillation 1.8 VP | Datasheet SNOSCY9A | p. 6 |
| DEVICE_ID 0x3055 | Datasheet SNOSCY9A | p. 33 |
| MANUFACTURER_ID 0x5449 | Datasheet SNOSCY9A | p. 33 |
| Register map full listing | Datasheet SNOSCY9A | pp. 15–33 |
| CONFIG default 0x2801 | Datasheet SNOSCY9A | p. 15 |
| MUX_CONFIG default 0x020F | Datasheet SNOSCY9A | p. 15 |
| RCOUNT default 0x0080 | Datasheet SNOSCY9A | p. 15 |
| RCOUNT reserved 0x0000–0x0004 | Datasheet SNOSCY9A | p. 20 |
| Conversion time formula | Datasheet SNOSCY9A | p. 39 |
| Settle time formula | Datasheet SNOSCY9A | p. 40 |
| Settle count minimum formula | Datasheet SNOSCY9A | p. 40 |
| Channel switch delay formula | Datasheet SNOSCY9A | p. 39 |
| DATAx = ƒ_SENSOR/ƒ_REF × 2^28 | Datasheet SNOSCY9A | p. 38 |
| Offset formula | Datasheet SNOSCY9A | p. 39 |
| IDRIVE calculation formula | Datasheet SNOSCY9A | p. 42 |
| IDRIVE current table (32 values) | Datasheet SNOSCY9A | pp. 42–43 |
| Clock requirements table | Datasheet SNOSCY9A | p. 46 |
| Deglitch filter values | Datasheet SNOSCY9A | p. 31 |
| Sleep→Active delay 16384/ƒ_INT | Datasheet SNOSCY9A | p. 12 |
| Software reset via RESET_DEV | Datasheet SNOSCY9A | p. 31 |
| Shutdown: all registers reset | Datasheet SNOSCY9A | p. 12 |
| Sleep: config retained, data cleared | Datasheet SNOSCY9A | p. 12 |
| CONFIG register must be written last | Datasheet SNOSCY9A | p. 51 |
| Example init single-channel | Datasheet SNOSCY9A | p. 51, Table 47 |
| Example init multi-channel | Datasheet SNOSCY9A | p. 51, Table 48 |
| INTB clear conditions | Datasheet SNOSCY9A | p. 47 |
| Status register sticky behavior | SNOA959 | p. 5 |
| UNREADCONV not sticky | SNOA959 | p. 7 |
| Error reporting options table | SNOA959 | p. 3, Table 1 |
| Watchdog only in continuous mode | SNOA959 | p. 8 |
| Amplitude warning interpretation | SNOA959 | pp. 9 |
| Zero count error details | SNOA959 | p. 9 |
| ERR_CHAN reports first error only | SNOA959 | pp. 4–5 |
| IDRIVE auto-calibration sequence | Datasheet SNOSCY9A | p. 44 |
| IDRIVE auto-calibration (SNAA221B) | SNAA221B | pp. 7–8 |
| Preferred IDRIVE setting | SNOA950 | p. 3 |
| IDRIVE current values & RP table | SNOA950 | pp. 2–3 |
| Optimal amplitude range 1.2–1.8 VP | SNOA950 | p. 3 |
| ESD clamp above 1.8 V | Datasheet SNOSCY9A | p. 41 |
| Bypass cap 1 µF MLCC | Datasheet SNOSCY9A | p. 53 |
| Layout: no planes under sensor | Datasheet SNOSCY9A | p. 54 |
| INxA/INxB grounded ~10 Ω inactive | Datasheet SNOSCY9A | p. 43 |
| Unused INxA/INxB no-connect OK | Datasheet SNOSCY9A | p. 43 |
| LDC0851/LDC211x/LDC3114 are different families | SLYA048B | pp. 9–10 |
| Auto-amp correction not for precision | SNOA950 | p. 4 |
| Multi-sensor same IDRIVE recommendation | SNOA950 | pp. 6–7 |
| Single-ch mode timing | SSZTCR4 | p. 2 |
| Multi-ch mode timing | SSZTCQ7 | pp. 2–3 |
| Sensing range ~2× coil diameter (LDC161x) | SSZTCL3 | p. 3 |
| Target > coil diameter for best response | SNOA957B | pp. 2–3 |
| EMI: shielding, filters, supply routing | SNOA962 | pp. 2–5 |
| SRF limit 0.8× | Datasheet SNOSCY9A | p. 53 |
| RP = L/(RS×C) | SNOA930C | p. 3 |
| Spring as sensor possible | SSZTCH7 | pp. 1–3 |

---

## 21. Addendum — Missing Information from Audit

The following sections fill gaps identified by systematic comparison of this extraction against the datasheet SNOSCY9A and key application notes.

---

### 21.1 Addendum: Absolute Maximum Ratings

*(Datasheet SNOSCY9A, p. 5)*

Stresses beyond these values may cause permanent damage. These are stress ratings only and do not imply functional operation.

| Parameter | Symbol | Min | Max | Unit |
|-----------|--------|-----|-----|------|
| Supply Voltage | VDD | — | 5 | V |
| Voltage on any pin | Vi | −0.3 | VDD + 0.3 | V |
| Input current on any INx pin | IA | −8 | 8 | mA |
| Input current on any Digital pin | ID | −5 | 5 | mA |
| Junction Temperature | Tj | −55 | 150 | °C |
| Storage Temperature | Tstg | −65 | 150 | °C |

---

### 21.2 Addendum: ESD Ratings

*(Datasheet SNOSCY9A, p. 5)*

| Package | Test | Standard | Value | Unit |
|---------|------|----------|-------|------|
| LDC1612 WSON-12 | HBM | ANSI/ESDA/JEDEC JS-001 | ±2000 | V |
| LDC1612 WSON-12 | CDM | JEDEC JESD22-C101 | ±750 | V |
| LDC1614 WQFN-16 | HBM | ANSI/ESDA/JEDEC JS-001 | ±2000 | V |
| LDC1614 WQFN-16 | CDM | JEDEC JESD22-C101 | ±750 | V |

Note: JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process. *(Datasheet p. 5)*

---

### 21.3 Addendum: Recommended Operating Conditions

*(Datasheet SNOSCY9A, p. 5)*

| Parameter | Symbol | Min | Nom | Max | Unit |
|-----------|--------|-----|-----|-----|------|
| Supply Voltage | VDD | 2.7 | — | 3.6 | V |
| Operating Temperature | TA | −40 | — | 125 | °C |

All limits specified at TA = 25°C, VDD = 3.3 V unless otherwise noted. *(Datasheet p. 5)*

---

### 21.4 Addendum: Thermal Information

*(Datasheet SNOSCY9A, p. 5)*

| Thermal Metric | LDC1612 WSON-12 (DNT) | LDC1614 WQFN-16 (RGH) | Unit |
|----------------|------------------------|------------------------|------|
| RθJA (Junction-to-ambient) | 50 | 38 | °C/W |

---

### 21.5 Addendum: I2C Timing — Missing Parameter

*(Datasheet SNOSCY9A, p. 7)*

The I2C timing table in Section 4 was missing one parameter:

| Parameter | Symbol | Min | Max | Unit |
|-----------|--------|-----|-----|------|
| Data valid acknowledge time | tVD;ACK | — | 0.9 | µs |

---

### 21.6 Addendum: Active Mode IDD with Sensor Current (Typical Characteristics)

*(Datasheet SNOSCY9A, pp. 8–9)*

The IDD spec of 2.1 mA in Section 5 is measured at ƒ_CLKIN = 10 MHz and does **not** include sensor current or I2C pull-up current. The typical characteristics graphs (Figures 2–3, pp. 8) show the active-mode IDD **including** 1.57 mA sensor coil current:

| Condition | Typical IDD (25°C, 3.3V) | Notes |
|-----------|--------------------------|-------|
| Active (ƒ_CLKIN = 10 MHz, excl. sensor) | 2.1 mA | Electrical characteristics table |
| Active (ƒ_CLKIN = 40 MHz, incl. 1.57 mA sensor) | ~3.1 mA | Typical characteristics, Fig. 2 |

Test conditions for typical characteristics: Sensor inductor 2-layer 32 turns/layer 14 mm PCB, L=19.4 µH, RP=5.7 kΩ at 2 MHz; C=330 pF COG/NP0; ƒ_CLKIN=40 MHz; FIN_DIVIDER0=0x1; FREF_DIVIDER0=0x001; RCOUNT0=0xFFFF; SETTLECOUNT0=0x0100; RP_OVERRIDE=1; AUTO_AMP_DIS=1; DRIVE_CURRENT0=0x9800. *(Datasheet p. 6 footnote 6, p. 8)*

---

### 21.7 Addendum: CLOCK_DIVIDERSx Default of 0x0000 Is Non-Functional

*(Datasheet SNOSCY9A, pp. 24–26)*

The reset/default value for CLOCK_DIVIDERSx registers is **0x0000**, which sets both FIN_DIVIDERx = 0x0 and FREF_DIVIDERx = 0x000. Both of these are **reserved values** that must not be used:

- FIN_DIVIDERx = 0b0000: Reserved — do not use. Minimum is 0b0001 (1).
- FREF_DIVIDERx = 0x000: Reserved — do not use. Minimum is 0x001 (1).

**Implication:** CLOCK_DIVIDERSx registers **must always be configured** before starting conversions. The device will not function correctly with the default register values.

---

### 21.8 Addendum: Deglitch Filter Value Discrepancy in Datasheet

*(Datasheet SNOSCY9A, pp. 31 vs. 47)*

The datasheet contains an internal discrepancy for the 33 MHz deglitch setting:

| Location | Value for 33 MHz |
|----------|-----------------|
| MUX_CONFIG register description (p. 31, Table 28) | **b111** |
| Input Deglitch Filter table (p. 47, Table 45) | **b011** |

The register description on p. 31 is the authoritative source. The MUX_CONFIG default value 0x020F has DEGLITCH = 0b111 = 33 MHz, which is consistent with the p. 31 definition. **Use b111 for 33 MHz.**

---

### 21.9 Addendum: Sensor Resonant Frequency and Q Formulas

*(Datasheet SNOSCY9A, pp. 35–36)*

These fundamental sensor equations are referenced by settle time and drive current calculations:

**Resonant frequency:**
`ƒ_SENSOR = 1 / (2π × √(L × C))`

where L = sensor inductance, C = sensor capacitance (C_SENSOR + C_PARASITIC). *(Datasheet p. 35, Eq. 1)*

**Sensor quality factor:**
`Q = R_P × √(C / L)`

*(Datasheet p. 35, Eq. 2)*

**Measured inductance with target:**
`L(d) = 1 / ((2π × ƒ_SENSOR(d))² × C)`

where d = distance between sensor and target. *(Datasheet p. 36, Eq. 3)*

---

### 21.10 Addendum: Single-Channel Clock Requirements Specifics

*(Datasheet SNOSCY9A, p. 46, Table 43)*

Table 43 specifies clock requirements differently for single-channel vs. multi-channel mode:

| Mode | Ref Source | Max ƒ_REFx | ƒ_INx Constraint | Min SETTLECOUNTx | Min RCOUNTx |
|------|-----------|------------|------------------|-----------------|-------------|
| Multi-channel | Internal | ≤ 55 MHz | < ƒ_REFx / 4 | > 3 | > 8 |
| Multi-channel | External | ≤ 40 MHz | < ƒ_REFx / 4 | > 3 | > 8 |
| Single-channel | Either | ≤ 35 MHz | *(not specified)* | *(not specified)* | *(not specified)* |

**Key observation:** For single-channel mode, the datasheet does **not** specify an ƒ_INx constraint or minimum SETTLECOUNT/RCOUNT values. In multi-channel mode, ƒ_INx must be < ƒ_REFx / 4 and minimum RCOUNT is > 8. The general minimum RCOUNT of 0x0005 (from register description, p. 20) still applies in single-channel mode. *(Datasheet p. 46)*

Note: The FIN_DIVIDERx ≥ 2 requirement if ƒ_SENSOR ≥ 8.75 MHz applies **regardless** of mode. *(Datasheet p. 24, footnote 2 in Table 43)*

---

### 21.11 Addendum: Multi-Channel Data Readback Timing — Data Loss Scenarios

*(Datasheet SNOSCY9A, pp. 47–48, Figure 58)*

When DRDY_2INT=1 in multi-channel mode, INTB asserts after the last channel in the sequence completes. The datasheet Figure 58 illustrates three important scenarios:

**Case 1 — No Data Loss:** I2C readback of all DATAx registers completes before the next conversion cycle overwrites any channel's data. This requires reading promptly after INTB assertion.

**Case 2 — Partial Data Loss:** If readback is delayed such that a new Channel 0 conversion completes before Channel 0 data is read, Channel 0 conversion N is overwritten by conversion N+1. The read returns Channel 0 conversion N+1 mixed with Channel 1 conversion N.

**Case 3 — Full Data Loss:** If readback is delayed even further, both Channel 0 and Channel 1 conversion N results are overwritten by conversion N+1 results. All channels return conversion N+1 data.

**Practical guidance:**
- Monitor UNREADCONVx flags in STATUS to detect data loss (overwriting). *(Datasheet p. 48)*
- A delayed read may show UNREADCONV0=0 (data already overwritten) while other channels still have UNREADCONV=1. *(Datasheet p. 48)*
- To avoid data loss, total I2C readback time for all active channels must be less than the conversion time of the first channel in the sequence. *(Datasheet pp. 47–48)*

---

### 21.12 Addendum: Sensor Activation Pattern Detail

*(Datasheet SNOSCY9A, pp. 40–41, Figure 55)*

Two sensor activation modes are configured via CONFIG.SENSOR_ACTIVATE_SEL (bit 11):

**Full Current Activation (SENSOR_ACTIVATE_SEL = 0):**
- During the sensor settle time, the LDC drives the **maximum** sensor current (nominally 1.56 mA, IDRIVE=31) regardless of the IDRIVEx setting.
- Results in faster sensor amplitude stabilization for high-Q sensors.
- Sensors already configured to use maximum drive (IDRIVEx = b11111) see no difference.
- **Recommended for most applications.** *(Datasheet p. 40)*

**Low Power Activation (SENSOR_ACTIVATE_SEL = 1, default):**
- During the sensor settle time, the LDC uses the programmed IDRIVEx value.
- Reduces power consumption during settling.
- May require longer settle times for high-Q sensors. *(Datasheet p. 41)*

**Single-channel vs. multi-channel activation behavior:**
- **Single-channel:** Sensor activation occurs **once** when entering Active Mode. After that, the sensor remains active and only conversions repeat (with optional amplitude correction between conversions if AUTO_AMP_DIS=0). *(Datasheet p. 38, Fig. 54)*
- **Multi-channel:** Sensor activation occurs **every time** the LDC switches channels — the previously active sensor is shut off (~10 Ω to GND), and the next sensor must be re-activated. *(Datasheet p. 43; SSZTCQ7 p. 2)*

---

### 21.13 Addendum: Auto-Calibration Sequence Errata (Datasheet Step Numbering)

*(Datasheet SNOSCY9A, p. 44)*

The auto-calibration sequence on datasheet p. 44 contains **swapped descriptions** for steps 2 and 5:

| Step | Datasheet text (p. 44) | Correct interpretation |
|------|----------------------|----------------------|
| 2 | "set CONFIG.SLEEP_MODE_EN to b0" | Should say **b1** (enter Sleep Mode) |
| 5 | "set CONFIG.SLEEP_MODE_EN to b1" | Should say **b0** (exit Sleep Mode, start conversion) |

The app note SNAA221B (pp. 7–8) provides the correct sequence. The extraction Section 11.3 already has the correct sequence. This addendum confirms the datasheet text has the bit values inverted in steps 2 and 5.

---

### 21.14 Addendum: Zero Count Error — LDC1612/1614 Specific Behavior

*(SNOA959 pp. 9, 3–4)*

The Zero Count error has unique reporting characteristics compared to other errors:

- **Not reportable via DATAx_MSB error flags.** Table 1 of SNOA959 shows "N/A" for DATA_CHx reporting of zero count errors. It can only be detected via STATUS.ERR_ZC (bit 8) and INTB (if ZC_ERR2INT=1 in ERROR_CONFIG). *(SNOA959 p. 3, Table 1)*
- **Data output during zero count:** For the LDC1312/1314 (12-bit), output is 0x0000 or 0x8000. For the LDC1612/1614 (28-bit), the DATA registers will contain all zeros or have the ERR_UR flag set (since zero sensor counts maps to an under-range condition). *(SNOA959 pp. 9)*
- **Zero count may precede watchdog timeout:** If the sensor fails to oscillate, a zero count error may occur first, followed by a watchdog timeout. *(SNOA959 p. 9)*

---

### 21.15 Addendum: CONFIG.ACTIVE_CHAN Interaction with AUTOSCAN_EN

*(Datasheet SNOSCY9A, pp. 29–30; SSZTCR4 p. 2)*

Explicitly documented interaction rules:

- When **AUTOSCAN_EN = 0** (single-channel mode): CONFIG.ACTIVE_CHAN selects which channel is continuously converted. RR_SEQUENCE has no effect. *(Datasheet p. 30; SSZTCR4 p. 2)*
- When **AUTOSCAN_EN = 1** (multi-channel mode): CONFIG.ACTIVE_CHAN is **ignored**. Channel sequencing is controlled by MUX_CONFIG.RR_SEQUENCE. *(Datasheet p. 30; SSZTCR4 p. 2)*
- **HIGH_CURRENT_DRV (bit 6):** Only functional when AUTOSCAN_EN = 0 (single-channel mode) **and** ACTIVE_CHAN = 00 (Channel 0). Not supported in multi-channel mode. *(Datasheet p. 30)*

---

### 21.16 Addendum: ERROR_CONFIG Reserved Bits Detail

*(Datasheet SNOSCY9A, pp. 28–29)*

The ERROR_CONFIG register (0x19) has reserved bits that must be written correctly:

- **Bits [10:8]:** Reserved, must be set to **0b000**. *(Datasheet p. 28)*
- **Bit [1]:** Reserved, must be set to **0b0**. *(Datasheet p. 29)*

Default value 0x0000 satisfies these constraints. When enabling error reporting, ensure reserved bits remain 0.

---

### 21.17 Addendum: RESET_DEV Register Full Format

*(Datasheet SNOSCY9A, p. 31)*

| Bits | Field | Type | Reset | Description |
|------|-------|------|-------|-------------|
| 15 | RESET_DEV | R/W | 0 | Write b1 to reset device. Always reads back 0. |
| 14:0 | RESERVED | R/W | 0x0000 | Must be set to b000_0000_0000_0000. |

Full reset write value: **0x8000**. *(Datasheet p. 31)*

---

### 21.18 Addendum: Sensor Oscillation Amplitude Operating Regions

*(Datasheet SNOSCY9A, pp. 41, 44; SNOA950 pp. 2–4)*

| Amplitude Range (V_peak) | Behavior | Action |
|--------------------------|----------|--------|
| > 1.8 VP | Internal ESD clamp activates. Frequency shift, invalid data. | Reduce IDRIVEx. *(Datasheet p. 41)* |
| 1.2 – 1.8 VP | **Optimum operating range.** Best SNR. | Target this range. *(SNOA950 p. 3)* |
| 0.6 – 1.2 VP | Sensor still converts but with degraded SNR (noisier). | Increase IDRIVEx if possible. *(SNOA950 p. 3)* |
| < 0.6 VP | Oscillation may be unstable or stop completely. LDC stops converting. | Increase IDRIVEx or check sensor RP. *(Datasheet p. 41)* |
| 0 VP | Sensor not oscillating. Watchdog timeout (single-ch) or zero count. | Check connections, RP, IDRIVEx. *(SNOA959 pp. 8–9)* |

**ERR_AHE threshold:** Nominal 1.8 VP. *(Datasheet p. 27)*
**ERR_ALE threshold:** Nominal 1.2 VP. *(Datasheet p. 27)*

---

### 21.19 Addendum: Power Supply Layout Recommendations

*(Datasheet SNOSCY9A, pp. 53–54)*

- **Bypass capacitor:** 1 µF MLCC, X7R dielectric, between VDD and GND pins. Place as close as possible to the device. Minimize the loop area formed by the bypass capacitor, VDD pin, and GND pin. *(Datasheet p. 53)*
- **Additional bulk capacitor:** If supply is located more than a few inches from the LDC, add 10 µF ceramic capacitor. *(Datasheet p. 53)*
- **Sensor trace routing:** Route INxA/INxB as differential pairs (parallel, close together). Lower trace impedance (even < 100 Ω) is acceptable to reduce parasitic inductance. *(Datasheet p. 54)*
- **Ground/power planes:** Do not place filled planes underneath or between sensor layers. Maintain a gap of ≥ 20% of sensor diameter between any plane and the outermost sensor coil. No continuous conductor rings should encircle the sensor. *(Datasheet p. 54)*
- **Sensor capacitor placement:** Place close to the inductor to minimize sensor RP. *(Datasheet p. 54)*

---

### 21.20 Addendum: Watchdog and Internal Oscillator Timing Dependencies

*(Datasheet SNOSCY9A, pp. 7, 12, 45)*

Several timing functions use the internal oscillator (ƒ_INT) **regardless** of whether an external clock is selected:

| Function | Timing | Clock Source |
|----------|--------|-------------|
| Sleep → Active delay (first conversion start) | 16384 / ƒ_INT (~377 µs at 43.4 MHz) | Internal oscillator (always) |
| Watchdog timeout / sensor recovery | ~5.2 ms | Internal oscillator (always) |

This means watchdog timeout and sleep-to-active timing vary with internal oscillator frequency (35–55 MHz over process/temperature). *(Datasheet pp. 7, 12, 45)*

---

### 21.21 Addendum: Complete Conversion and Frequency Formulas Summary

*(Datasheet SNOSCY9A, pp. 38–40)*

For reference, the complete set of computation formulas:

**Output code (basic, FIN_DIVIDERx=1, FREF_DIVIDERx=1, OFFSETx=0):**
`DATAx = (ƒ_SENSORx / ƒ_CLK) × 2^28` *(Eq. 4, p. 38)*

**Output code (general):**
`DATAx = ((ƒ_SENSORx / FIN_DIVIDERx) / ƒ_REFx − OFFSETx / 2^16) × 2^28` *(implied from Eq. 6)*

**Sensor frequency from output code (general):**
`ƒ_SENSORx = (DATAx / 2^28 + OFFSETx / 2^16) × ƒ_REFx × FIN_DIVIDERx` *(Eq. 6, p. 39)*

**Reconstruction of DATAx from registers:**
`DATAx = DATAx_MSB[11:0] × 65536 + DATAx_LSB[15:0]` *(Eq. 5, p. 38)*

**Conversion time:**
`tCx = (RCOUNTx × 16 + 4) / ƒ_REFx` *(Eq. 7, p. 39)*

**Settle time:**
`tSx = (SETTLECOUNTx × 16) / ƒ_REFx` *(Eq. 9, p. 40)*

**Minimum settle count:**
`SETTLECOUNTx ≥ Q_SENSORx × ƒ_REFx / (16 × ƒ_SENSORx)` — round up *(Eq. 10, p. 40)*

**Channel switch delay:**
`t_switch = 692 ns + 5 / ƒ_REF` *(Eq. 8, p. 39)*

**Per-channel dwell time (multi-channel):**
`t_dwell = tSx + tCx + t_switch` *(Datasheet p. 40)*

**Frequency offset:**
`ƒ_OFFSETx = (OFFSETx / 2^16) × ƒ_REFx` *(Register description, p. 21)*

**Reference frequency:**
`ƒ_REFx = ƒ_CLK / FREF_DIVIDERx` *(p. 24)*

**Input frequency:**
`ƒ_INx = ƒ_SENSORx / FIN_DIVIDERx` *(p. 24)*

**Sensor drive current:**
`IDRIVE = π × V_P / (4 × R_P)` *(Eq. 12, p. 42)*

**Sensor oscillation amplitude:**
`V_OSC ≈ IDRIVE × π × R_P / 4` *(SNOA950 p. 2, Eq. 2)*

**Sensor resonant frequency:**
`ƒ_SENSOR = 1 / (2π × √(L × C))` *(Eq. 1, p. 35)*

**Sensor quality factor:**
`Q = R_P × √(C / L)` *(Eq. 2, p. 35)*

**Parallel resistance from series model:**
`R_P = L / (R_S × C)` *(SNAA221B p. 3, Eq. 1)*

---

### 21.22 Addendum: Conversion Time Formula Discrepancy Clarification

*(Datasheet SNOSCY9A, pp. 20 vs. 39)*

The register description tables (pp. 20–21) give conversion time as:
`tCx = (RCOUNTx × 16) / ƒ_REFx`

The application section (p. 39, Eq. 7) gives:
`tCx = (RCOUNTx × 16 + 4) / ƒ_REFx`

The **+4** term represents 4 additional reference clock cycles of overhead. For firmware timing calculations:
- At RCOUNT = 0xFFFF: difference is 4/1048580 = 0.0004% — negligible.
- At RCOUNT = 0x0005: difference is 4/84 = 4.8% — significant for minimum conversion times.
- **Use the Eq. 7 form (with +4)** for accurate timing budgets. *(Datasheet p. 39)*
