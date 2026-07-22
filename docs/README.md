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
- [Validation status](VALIDATION_STATUS.md) - current software checks, local evidence boundaries,
  and hardware evidence still required.
- [TunnelMonitor suitability audit](TUNNELMONITOR_NODE_SUITABILITY_AUDIT.md) - external-owner suitability
  findings, dispositions, and the still-external TunnelMonitor product gate.
- [HIL artifact rules](https://github.com/janhavelka/LDC1614/blob/main/docs/hil/README.md) - location and rules for real HIL output artifacts.

The maintained examples have their own guides:

- [Arduino diagnostic CLI](https://github.com/janhavelka/LDC1614/blob/main/examples/01_basic_bringup_cli/README.md)
- [Native ESP-IDF diagnostic CLI](https://github.com/janhavelka/LDC1614/blob/main/examples/esp_idf/basic/README.md)

## Generated Doxygen Site

Run `doxygen Doxyfile` from the repository root. The generated site starts at
`docs/doxygen/html/index.html` and includes the public headers and the uniquely
named integration, hardware, IDF, HIL, validation, and suitability guides.
Repository/example `README.md` files remain linked from the main page rather
than duplicated as generated pages. Generation fails on undocumented public
API, missing parameter documentation, or documentation errors.

`docs/doxygen/` is ignored build output. Do not edit or commit it. Internal
engineering instructions, historical reports, and raw extracted vendor text
are deliberately excluded from the public site.

## Reference Material

- [Primary LDC1612/LDC1614 datasheet](reference/LDC1614_datasheet.pdf).
- `reference/extracted-md/` - compact curated device notes used while
  implementing and reviewing the driver.
- `reference/application_notes/` - vendor application-note PDFs and concise
  local summaries.
- `reference/howto_guides/` - vendor how-to PDFs and concise local summaries.
- `reference/pdf-extracted-md/` - raw generated markdown from source PDFs. This
  is trace material, not implementation guidance.

## Evidence Boundaries

`reports/` retains only hardware evidence cited by the maintained validation
status. One-time implementation instructions and no-hardware or empty-payload
run artifacts are not retained; Git history preserves them if an audit needs
them.
Raw PDF extraction under `reference/pdf-extracted-md/` is source traceability,
not implementation authority. Current behavior is defined by the public
headers, root README, maintained guides above, and changelog. Hardware readiness
still requires the evidence defined by the HIL procedure.
