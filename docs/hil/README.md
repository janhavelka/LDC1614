# HIL Artifacts

This file defines artifact rules. Commit reviewed JSON/Markdown evidence under
`docs/reports/`; release-only captures may remain attached to the GitHub
release. Do not create pass artifacts by hand.

Expected generated files:

- JSON result from `tools/ldc1614_hil_runner.py --json-out ...`
- Markdown summary from `tools/ldc1614_hil_runner.py --markdown-out ...`
- Raw serial transcript from `tools/ldc1614_hil_runner.py` with
  `--raw-transcript-out ...`
- At least one raw serial transcript or logic-analyzer trace for production
  acceptance of the exact release fixture
- Optional oscilloscope or bench notes referenced by the run

The retained post-v3 negative Markdown/JSON artifacts embed their complete
serial transcripts. They are useful transport-regression evidence but do not
satisfy the positive exact-release acceptance gate. A standalone `*.log` is
ignored as temporary output; retain a reviewed raw capture as a release
artifact or use a non-ignored extension such as `.serial.txt` when repository
review specifically requires it.

No-hardware `NOT_RUN`, dry-run, and empty-payload outputs are review aids, not
HIL logs. Keep them as temporary CI or review artifacts rather than committing
them under `docs/reports/`, unless a maintainer explicitly requires a specific
artifact for an active audit.

The Arduino no-sensor runner also supports an explicit bounded soak with
`--include-long-soak --soak-duration-s <seconds>`. This is chip-only transport,
identity, STATUS, sleep/wake, and liveness evidence. Its pre-soak matrix also
checks matching operation IDs for initialize/apply/reset/acquire plus lifecycle
and replay paths. Without an LC sensor this is not conversion-accuracy or
production-cadence evidence.
