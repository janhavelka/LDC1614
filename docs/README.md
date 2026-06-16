# LDC1614 Docs

This directory is the maintained documentation surface for the library. It is
kept intentionally small: usage and API contracts live in the repository
`README.md`; these files cover integration, validation, and supporting source
material.

## Maintained Docs

- `HARDWARE_INTEGRATION.md` - board, sensor, timing, INTB, SD, and I2C
  integration checklist.
- `I2C_INTEGRATION.md` - injected-transport, error-taxonomy, dirty-config, and
  bounded poll-integration notes for application-owned I2C managers.
- `IDF_PORT.md` - native ESP-IDF component and diagnostic example boundaries.
- `HIL_VALIDATION.md` - hardware-in-the-loop procedure and evidence matrix.
- `VALIDATION_STATUS.md` - current software checks, local evidence boundaries,
  and hardware evidence still required.
- `hil/README.md` - location and rules for real HIL output artifacts.

## Reference Material

- `reference/LDC1614_datasheet.pdf` - primary device datasheet.
- `reference/extracted-md/` - compact curated device notes used while
  implementing and reviewing the driver.
- `reference/application_notes/` - vendor application-note PDFs and concise
  local summaries.
- `reference/howto_guides/` - vendor how-to PDFs and concise local summaries.
- `reference/pdf-extracted-md/` - raw generated markdown from source PDFs. This
  is trace material, not implementation guidance.

## What Was Removed From Active Docs

Prompt-era reports, hardening progress logs, implementation plans, cleanup
reports, and prompt files are not part of the maintained docs surface. Current
status is represented by `VALIDATION_STATUS.md`, the changelog, and the root
`README.md`; hardware evidence still has to be captured with the HIL procedure
before target deployment decisions.
