# LDC1614 Compact Documentation Inventory

This directory contains curated engineering notes for the LDC1612/LDC1614
driver. These files are not raw PDF dumps. Source PDFs remain under
`docs/reference/` as the authoritative full documents.

## Source Set

| Source PDF | Pages | Role |
|---|---:|---|
| `docs/reference/LDC1614_datasheet.pdf` | 67 | Primary source for pinout, I2C protocol, register map, timing, conversion modes, status, reset, and electrical limits. |
| `docs/reference/application_notes/sensor_configuration_settings.pdf` | 8 | Supplemental source for drive-current selection and fixed-current recommendations. |
| `docs/reference/application_notes/sensor_status_monitoring.pdf` | 14 | Supplemental source for status, unread conversion, and INTB monitoring behavior. |
| `docs/reference/application_notes/configuring_rp.pdf` | 11 | Supplemental source for RP variation and drive-current implications. |
| `docs/reference/application_notes/measuring_rp.pdf` | 4 | Supplemental source for estimating sensor RP. |
| `docs/reference/application_notes/sensor_design.pdf` | 23 | Supplemental source for coil and target design constraints. |
| `docs/reference/application_notes/emi_considerations.pdf` | 6 | Supplemental source for EMI and deglitch/filter context. |
| Other PDFs under `docs/reference/application_notes/` and `docs/reference/howto_guides/` | varies | Background only when the note names LDC1612/LDC1614 or gives numeric LDC sensor constraints used in the notes. |

## Compact Notes

| File | Purpose |
|---|---|
| `01_chip_overview.md` | Device identity, measurement principle, channels, clocks, and implementation implications. |
| `02_pinout_and_signals.md` | Pins, package differences, I2C address selection, INTB, SD, CLKIN, and sensor inputs. |
| `03_electrical_and_timing.md` | Supply limits, current modes, I2C timing, reference clock limits, conversion and settle timing. |
| `04_protocol_commands_and_transactions.md` | I2C register access, 16-bit transactions, read coherency, addressing, and bus caveats. |
| `05_register_map.md` | Register addresses, reset values, key fields, data format, and driver-facing access rules. |
| `06_modes_interrupts_status_and_faults.md` | Sleep/shutdown, continuous and auto-scan modes, INTB, STATUS, ERROR_CONFIG, and data-overwrite behavior. |
| `07_initialization_reset_and_operational_notes.md` | Recommended bring-up sequence, reset, clock/divider choices, drive-current setup, and layout notes. |
| `08_variant_differences_and_open_questions.md` | LDC1612 vs LDC1614 differences, conflicts, gaps, and items not specified by the PDFs. |

## Source Priority

The datasheet is authoritative for chip behavior. Application notes are used only
for design guidance or additional explanation. If a supplemental document uses a
related part such as LDC0851, LDC1000, LDC1101, or LDC131x, that content is not
treated as an LDC1614 register or protocol source unless the note explicitly
states applicability to LDC1612/LDC1614.

## Cleanup Notes

The previous compact files contained raw page extracts, notices, OCR artifacts,
and unrelated supplemental-document text. Those dumps were removed from the
compact notes. The authoritative PDFs remain alongside concise summaries; raw
extraction artifacts are rebuildable and intentionally not retained.
