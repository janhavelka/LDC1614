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

No-hardware `NOT_RUN`, dry-run, and empty-payload outputs are review aids, not
HIL logs. Keep them as temporary CI or review artifacts rather than committing
them under `docs/reports/`, unless a maintainer explicitly requires a specific
artifact for an active audit.
