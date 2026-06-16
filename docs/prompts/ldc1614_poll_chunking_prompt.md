# Prompt: LDC1614 Poll-Chunked I2C Execution

> Historical artifact: this was an exploratory prompt, not current repository
> guidance. Current instructions live in `../../AGENTS.md`; current docs are
> indexed from `../README.md`.

Target repo: https://github.com/janhavelka/LDC1614

## Goal

Add a straightforward poll-chunked model for LDC1612/LDC1614 configuration and
channel reads. The default must advance only one I2C instruction per poll, while
setup/diagnostics can request a larger fixed budget.

## Companion prompt boundary

The companion audit prompt owns package metadata, begin/probe error taxonomy,
offline/status decisions, dirty/config-state policy, and raw output
requirements. This prompt owns only execution sequencing, instruction
accounting, delay gates, and budget tests.

## Required subagents

Before editing, spawn subagents:

- one explorer for LDC1614 register sequencing and channel data reads,
- one explorer for transaction-budget tests and current sequencing gaps,
- worker subagents only with disjoint files and no-revert instructions.

## Common naming and semantics

Use:

- `startReadChannels(mask)`, `poll(nowMs, maxInstructions)`,
  `readChannelsReady()`, `getChannelSample(ch)`, `startApplyConfig()`,
  `startResetAndReapply()`.
- One register `writeRead` or register write is one instruction.
- Delay/ready checks that do not touch I2C consume zero instructions.
- Keep one active job per driver instance.

## LDC1614 chip sequencing

Atomic one-instruction operations:

- Read one 16-bit register.
- Write one 16-bit register.
- Treat `DATAx_MSB` plus `DATAx_LSB` as one logical channel-read unit. Implement
  it with the safest chip-supported transfer shape, and do not expose a half
  channel sample as valid.
- `STATUS` may be clear-on-read; model that side effect explicitly.

Chunked jobs:

- Init/config:
  read device ID/status, write reference count, settle count, clock divider,
  drive current, mux/config registers one instruction at a time.
- Channel read:
  read status/data-ready if requested,
  read each selected channel as a coherent MSB/LSB unit,
  assemble counts in CPU-only code.
- Recovery:
  reset/reprobe/reapply config with the same chunked config job.

Multiple instructions per poll:

- Config apply can use a larger budget during lifecycle/setup.
- Channel reads can use larger budgets only when the owner accepts the added
  latency; default background reads stay one instruction per poll.
- Stop on first I2C failure and propagate the companion-audit dirty/config
  policy.
- Keep DRDY/STATUS polling one instruction per poll. Fixed config writes while
  asleep may use a larger budget.

## Delay gates

- Gate sleep-to-active and channel-settle/conversion intervals.
- Respect shutdown wake/I2C availability, channel switch delay, DRDY/INTB, and
  watchdog recovery timing.

## Tests

- Budget tests for config apply and selected-channel reads.
- Job failure tests proving the chunked state machine stops and reports status.
