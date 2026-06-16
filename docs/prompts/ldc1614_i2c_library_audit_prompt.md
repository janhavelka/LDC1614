# Prompt: LDC1614 I2C Library Hardening And TunnelMonitor Fit

> Historical artifact: this was an exploratory prompt, not current repository
> guidance. Current instructions live in `../../AGENTS.md`; current docs are
> indexed from `../README.md`.

Target repo: https://github.com/janhavelka/LDC1614

## Context

TunnelMonitor requires I2C device libraries to stay behind the `I2cTask` owner.
The owner advances active jobs through bounded polls and normally performs one
backend transfer per poll. LDC1614 adoption needs packaging cleanup,
transport-error fidelity, and a step-safe acquisition/config path.

This prompt is for the LDC1614 library repo only.

## Companion prompt boundary

This audit prompt owns correctness, stability, packaging, status/error
taxonomy, partial-config risk, dirty/readback policy, framework neutrality, and
raw output availability.

The companion `ldc1614_poll_chunking_prompt.md` owns atomic register/channel
read units, read-channel/config job sequencing, `maxInstructions`, delay gates,
and transaction-budget tests. Do not duplicate those details here.

## Subagents required

Before coding, spawn subagents:

- API/timing explorer: inspect `begin()`, `probe()`, conversion polling,
  recovery, `_applyConfig()`, and hidden blocking or partial-write risks. Leave
  exact poll sequencing to the companion prompt.
- Test/package explorer: verify include/package metadata, native tests, and
  clean-consumer compile behavior.
- Use worker subagents only with disjoint files and no-revert instructions.

## Hard constraints

- Core must be framework-neutral and callback-driven.
- No 64-bit timer work is needed. Use `uint32_t` wrap-safe local timing.
- Raw register/count outputs are acceptable and preferred for integration.
- Keep health/status local; TunnelMonitor owns aggregate optional/required
  health projection.

## What is already good

- Preserve simple callbacks, bounded caches, LDC1612/LDC1614 channel
  validation, recovery hooks, and useful native coverage.

## Required audit and fixes

1. Fix packaging: `include/LDC1614/LDC1614.h` includes
   `LDC1614/Version.h`, but that header is missing from the package. Check in
   generated metadata or make clean builds generate it reliably. Add a
   clean-consumer compile test.
2. Review `begin()` failure semantics. It stores `_config` before `probe()` and
   `_applyConfig()` can fail. Either intentionally retain failed-begin
   diagnostics or clean state like the stronger sibling drivers. Add tests.
3. Preserve detailed `probe()` transport failures. Only definite address
   absence should become `DEVICE_NOT_FOUND`.
4. Align offline taxonomy. Current offline normal I2C returns `BUSY` because
   there is no `Err::OFFLINE`; either add `OFFLINE` consistently or document a
   family-wide `BUSY` choice.
5. Harden multi-register config paths against partial hardware writes. Add
   dirty/readback or forced-reapply diagnostics.
6. Audit conversion-wait loops for hidden blocking and document their public API
   classification. The companion prompt owns owner-step sequencing.

## Required tests

- Clean package import/compile test.
- Failed probe and partial apply state tests.
- Error mapping matrix.
- Existing native tests must pass.

## Deliverables

- Code/tests plus a compatibility report describing package fixes, status
  taxonomy, and steady-path API candidates.
