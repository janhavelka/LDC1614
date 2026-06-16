# LDC1614 Documentation Index

Use this index to avoid confusing historical planning artifacts with the current
merge/release review state.

## Current Authoritative Docs

- `../README.md` - public API, examples, contracts, and validation status
  summary.
- `../AGENTS.md` - repository engineering rules for agents and contributors.
- `LDC1614_INDUSTRY_HARDENING_FINAL_REPORT.md` - current software hardening
  status, Prompt 02 reconciliation, deferred evidence, and conservative
  merge/release recommendation.
- `LDC1614_MERGE_RELEASE_CLEANUP_REPORT.md` - final documentation cleanup, local
  command evidence, artifact cleanup, sync status, and remaining blockers for
  this merge/release review pass.
- `LDC1614_HARDENING_PROGRESS.md` - chronological hardening log. Use the final
  report and cleanup report for current status.
- `VALIDATION_STATUS.md` - maintained validation expectations and evidence
  boundaries.

## Hardware/HIL Docs

- `HARDWARE_INTEGRATION.md` - board/application integration checklist.
- `HIL_VALIDATION.md` - HIL procedure and evidence matrix.
- `hil/README.md` - location for real HIL output artifacts; currently no
  hardware logs are committed.

## ESP-IDF Docs

- `IDF_PORT.md` - maintained ESP-IDF component and diagnostic example status.
- `IDF_PORT_IMPLEMENTATION.md` - historical ESP-IDF hardening record.
- `../examples/esp_idf/basic/README.md` - native ESP-IDF diagnostic bring-up
  example notes.

## Application Notes / Datasheet Extractions

- `extracted-md/` - compact device facts extracted for implementation review.
- `pdf-extracted-md/` - markdown generated from source PDFs.
- `application_notes/` and `howto_guides/` - vendor reference material and
  derived notes. These are design aids, not validation evidence.
- `LDC1614_datasheet.pdf` - source datasheet reference.

## Historical Reports/Plans

- `LDC1614_INDUSTRY_READINESS_IMPLEMENTATION_PLAN.md` - baseline plan from the
  start of the hardening sequence.
- `../prompts/` - historical prompt set used to drive the hardening sequence.
  See `../prompts/README.md` before reusing any prompt text.
- `prompts/` - additional exploratory prompt artifacts retained for
  traceability.
