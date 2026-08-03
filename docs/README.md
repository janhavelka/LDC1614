# LDC1614 Docs

This directory contains maintained integration and validation guides plus
cited hardware evidence and vendor references. Start with the repository
[README](../README.md) for the public API contract.

## Maintained Docs

- [Hardware integration](HARDWARE_INTEGRATION.md) - board, sensor, timing, INTB, SD, and I2C
  integration checklist.
- [I2C owner integration](I2C_INTEGRATION.md) - injected transport, owner deadlines/budgets,
  cancellation, result identity, applied-state, and recovery contracts.
- [Native ESP-IDF integration](IDF_PORT.md) - component and diagnostic example boundaries.
- [HIL validation](HIL_VALIDATION.md) - hardware-in-the-loop procedure and evidence matrix.
- [Validation status](VALIDATION_STATUS.md) - repeatable release checks,
  retained evidence boundaries, and hardware evidence still required.
- Repository-only [TunnelMonitor integration gates](https://github.com/janhavelka/LDC1614/blob/main/docs/TUNNELMONITOR_INTEGRATION_GATES.md) -
  unresolved product, release, owner-module, and target-HIL requirements; this
  consumer-specific ledger is excluded from the generic library package.

The maintained examples have their own guides:

- [Arduino diagnostic CLI](../examples/01_basic_bringup_cli/README.md) -
  complete colored command surface and cooperative diagnostic sessions.
- [Native ESP-IDF diagnostic CLI](../examples/esp_idf/basic/README.md) -
  independently implemented, host-contract-checked CLI parity.

## Generated Doxygen Site

Run `doxygen Doxyfile` from the repository root. The generated site starts at
`docs/doxygen/html/index.html` and includes the public headers and the reusable
integration, hardware, IDF, HIL, and validation guides.
Repository/example `README.md` files remain linked from the main page rather
than duplicated as generated pages. Generation fails on undocumented public
API, missing parameter documentation, or documentation errors.

`docs/doxygen/` is ignored build output. Do not edit or commit it.
Product-specific gates, internal engineering instructions, historical evidence,
and vendor reference material are deliberately excluded from the public site.

## Reference Material

- [Primary LDC1612/LDC1614 datasheet](reference/LDC1614_datasheet.pdf).
- [Vendor PDF manifest](https://github.com/janhavelka/LDC1614/blob/main/docs/reference/PDF_MANIFEST.md) - page counts and verified
  SHA-256 checksums for every retained PDF.
- Repository-only `reference/extracted-md/` - compact curated device notes used while
  implementing and reviewing the driver.
- Repository-only `reference/application_notes/` - vendor application-note PDFs and concise
  local summaries.
- Repository-only `reference/howto_guides/` - vendor how-to PDFs and concise local summaries.

## Evidence Boundaries

The repository-only [`reports/`](https://github.com/janhavelka/LDC1614/blob/main/docs/reports/README.md) archive retains structured HIL evidence and raw serial transcripts, not
generated Markdown copies or finished audit narratives. One-time implementation
instructions and no-hardware or empty-payload artifacts are not retained; Git
history preserves them if an audit needs them. Current behavior is defined by
the public headers, root README, maintained guides above, and changelog.
Hardware readiness still requires the evidence defined by the HIL procedure.
