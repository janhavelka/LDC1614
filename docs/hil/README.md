# HIL Artifacts

Store hardware-in-the-loop run outputs here when an operator runs real hardware.
Do not create pass artifacts by hand.

Expected generated files:
- JSON result from `tools/ldc1614_hil_runner.py --json-out ...`
- Markdown summary from `tools/ldc1614_hil_runner.py --markdown-out ...`
- Optional logic-analyzer, oscilloscope, or bench notes referenced by the run

No hardware logs are committed in this prompt.
