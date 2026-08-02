# LDC1614 Reference Material

This directory holds source material used to implement and review the driver. It
is intentionally separated from the maintained integration docs in `docs/`.

- `LDC1614_datasheet.pdf` - primary device datasheet:
  - Vendor: Texas Instruments.
  - Title: *LDC1612, LDC1614 Multi-Channel 28-Bit Inductance to Digital
    Converter (LDC) for Inductive Sensing*.
  - Vendor document: `SNOSCY9A`, revision A, revised March 2018.
  - Vendor URL: <https://www.ti.com/lit/ds/symlink/ldc1614.pdf>.
  - Repository retrieval/addition date: 2026-04-04 (date of the first commit
    containing this PDF).
  - SHA-256:
    `b3bab7a84c9a8423448113f24de3b343c06ce7e7e4cbc6039b262b22130d8652`.
- `extracted-md/` - compact curated notes from the datasheet and selected
  application notes.
- `application_notes/` - vendor application-note PDFs and concise local
  summaries.
- `howto_guides/` - vendor how-to PDFs and concise local summaries.

The PDFs are the retained full-text vendor sources. Rebuildable raw text and
Markdown extraction dumps are intentionally not kept; use the concise sibling
summaries and curated notes for search and review, then verify exact claims in
the source PDF. [PDF_MANIFEST.md](PDF_MANIFEST.md) records verified page counts
and SHA-256 checksums for the complete retained set.

The `extracted-md/00_document_inventory.md` through
`extracted-md/08_variant_differences_and_open_questions.md` files are curated
chip notes, not disposable raw extracts. They preserve driver-relevant facts for
device identity, pinout, electrical limits, I2C transactions, register map,
status/interrupt behavior, reset/bring-up, timing, variant differences, and
known source ambiguities.

For normal integration and review, start with `../README.md`,
`../HARDWARE_INTEGRATION.md`, and `../VALIDATION_STATUS.md`.
