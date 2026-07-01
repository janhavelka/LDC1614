# HIL Artifacts

Store hardware-in-the-loop run outputs here when an operator runs real hardware.
Do not create pass artifacts by hand.

Expected generated files:
- JSON result from `tools/ldc1614_hil_runner.py --json-out ...`
- Markdown summary from `tools/ldc1614_hil_runner.py --markdown-out ...`
- Optional logic-analyzer, oscilloscope, or bench notes referenced by the run

No hardware logs are currently committed.

No-hardware `NOT_RUN` / dry-run audit artifacts are review aids, not HIL logs.
Keep them in `docs/reports/` or an external review artifact directory unless a
maintainer explicitly wants to retain them here for traceability.
