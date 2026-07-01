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
| Channel table typos | The application sample-data table text has channel 2/3 row labeling errors in the raw extraction. The register map names `DATA2_*` at `0x04`/`0x05` and `DATA3_*` at `0x06`/`0x07`. Source: datasheet, pp. 15, 38. |
| Sleep bit wording in example text | The configuration example text in the raw extraction appears to invert sleep-mode wording in two steps; the register definition is clear that `SLEEP_MODE_EN = 1` means sleep and `0` means conversion. Source: datasheet, pp. 12, 28, 49-51. |

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

The raw PDF text contains symbol encoding artifacts and figure/table extraction
errors. These compact notes normalize units to ASCII-friendly text and avoid
copying long passages. Use `docs/reference/pdf-extracted-md/` only as a traceable raw
source, not as implementation-ready documentation.
