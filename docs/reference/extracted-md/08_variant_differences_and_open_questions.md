# Variant Differences and Open Questions

## LDC1612 vs LDC1614

| Topic | LDC1612 | LDC1614 | Source |
|---|---|---|---|
| Channels | 2 channels | 4 channels | datasheet, pp. 1, 4, 15 |
| Package | WSON-12 | WQFN-16 | datasheet, pp. 1, 4 |
| Channel registers | Ch0/Ch1 registers valid | Ch0/Ch1/Ch2/Ch3 registers valid | datasheet, pp. 15-31 |
| Auto-scan sequences | Ch0/Ch1 only | Ch0/Ch1, Ch0/Ch1/Ch2, or Ch0/Ch1/Ch2/Ch3 | datasheet, p. 29 |

The compact notes focus on the four-channel LDC1614 driver but call out
LDC1612-only limits where they affect register validation.

## Documented IDs

| Register | Value | Source |
|---|---:|---|
| `MANUFACTURER_ID` | `0x5449` | datasheet, p. 33 |
| `DEVICE_ID` | `0x3055` | datasheet, p. 33 |

The datasheet does not list a separate device ID for LDC1612 versus LDC1614 in
the extracted register table. Treat channel-count selection as a driver
configuration decision unless another verified source is added.

## Source Conflicts and Ambiguities

| Topic | Notes |
|---|---|
| Deglitch 33 MHz code | `MUX_CONFIG` field text lists 33 MHz as `b111`, while the application deglitch table lists 33 MHz as `b011`. Source: datasheet, pp. 29-30, 46. |
| Conversion-time formula | Register tables describe conversion time as `RCOUNTx * 16 / fREFx`; the application text includes an added `+4` reference-clock term. Source: datasheet, pp. 20, 39. |
| Channel table typos | Datasheet Table 37 (p. 38) mislabels the channel 2 and channel 3 rows as `DATA1_MSB`/`DATA1_LSB` and describes the channel 3 LSB as Channel 0. The register list (p. 15) is authoritative: `DATA2_*` at `0x04`/`0x05` and `DATA3_*` at `0x06`/`0x07`. Source: datasheet, pp. 15, 38. |
| Sleep bit polarity in the auto-calibration sequence | Section 8.1.5.2 (p. 44) steps 2 and 5 say to enter SLEEP with `CONFIG.SLEEP_MODE_EN = b0` and to exit with `b1`, inverting the CONFIG field definition on p. 29, where `b1` is Sleep Mode and `b0` is active. Follow the register definition. The design example on pp. 49-51 is correct: step 8b sets `SLEEP_MODE_EN = b0` to enable conversion and `CONFIG = 0x1601` has bit 13 clear. Source: datasheet, pp. 12, 29, 44, 49-51. |
| Section 8.2.4 example self-conflict | Step 1b (p. 50) says the reference divider "can be set to 1" via `FREF_DIVIDER0 = 0x01`, but step 1c gives `CLOCK_DIVIDERS0 = 0x1002`, steps 2c and 3 compute with fREF = 20 MHz, and Table 47 (p. 51) states `FREF_DIVIDER0 = 2`. Section 8.2.3 (p. 49) states 500 SPS (TSAMPLE = 2.00 ms) while step 4 budgets 1000 - 8 - 1 = 991 us and Tables 47/48 label the result 1 kSPS. `07_initialization_reset_and_operational_notes.md` follows the register values (`FREF_DIVIDER0 = 2`, 1 kSPS). Source: datasheet, pp. 49-51. |

## Facts Not Documented in the PDFs

- The provided PDFs do not define nonvolatile configuration memory.
- No FIFO is documented; only per-channel conversion result registers and
  unread-conversion flags are documented.
- No CRC, checksum, or packet-level bus integrity feature is documented for the
  I2C register interface.
- The datasheet does not provide a universal reserved-bit policy beyond the
  individual register-field notes.
- Application notes include many related LDC/FDC devices; those notes should not
  be used as LDC1614 register evidence unless they explicitly reference
  LDC1612/LDC1614.

## Extraction Caveats

PDF text extraction can introduce symbol, figure, and table errors. These
compact notes normalize units to ASCII-friendly text and avoid copying long
passages. Verify exact wording, values, and diagrams in the retained source
PDFs rather than treating generated text as implementation-ready documentation.
