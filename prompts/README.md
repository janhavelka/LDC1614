# LDC1614 Prompt Archive

This directory preserves the prompts used during the LDC1614 industry-readiness
hardening sequence. The files are historical evidence and should not be treated
as current instructions.

Current authoritative status is in:

- `docs/LDC1614_INDUSTRY_HARDENING_FINAL_REPORT.md`
- `docs/LDC1614_MERGE_RELEASE_CLEANUP_REPORT.md`
- `docs/LDC1614_HARDENING_PROGRESS.md`
- `docs/README.md`

Prompt files:

- `00_overview_and_sequence.md` - original chunked hardening sequence overview.
- `01_baseline_agents_branch_report.md` - baseline audit and branch report.
- `02_core_contracts_device_correctness.md` - core contract and device
  correctness prompt from the sequence.
- `03_espidf_example_component_guards.md` - ESP-IDF hardening prompt.
- `04_tests_docs_ci_package.md` - tests, docs, CI, and packaging prompt.
- `05_hil_runner_final_report.md` - HIL procedure/final-report prompt.

Important status correction: older prompt assumptions about missing Prompt 02
work are stale. Timing/freshness reconciliation is implemented in the current
branch, subject to the hardware/HIL evidence limits documented in the final
report.

Future prompts should start from the current docs listed above, not from these
historical prompt assumptions.
