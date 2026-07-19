# HIL Artifacts

Store hardware-in-the-loop run outputs here when an operator runs real hardware.
Do not create pass artifacts by hand.

Expected generated files:
- JSON result from `tools/ldc1614_hil_runner.py --json-out ...`
- Markdown summary from `tools/ldc1614_hil_runner.py --markdown-out ...`
- At least one raw serial transcript or logic-analyzer trace for production
  acceptance of the exact release fixture
- Optional oscilloscope or bench notes referenced by the run

No raw serial transcript or logic-analyzer trace is currently committed. The
compact chip-only summaries under `docs/reports/` remain valid limited evidence,
but do not satisfy the raw-artifact production acceptance gate.

No-hardware `NOT_RUN` / dry-run audit artifacts are review aids, not HIL logs.
Keep them in `docs/reports/` or an external review artifact directory unless a
maintainer explicitly wants to retain them here for traceability.
