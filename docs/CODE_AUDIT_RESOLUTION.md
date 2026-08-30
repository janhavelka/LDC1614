# Code audit resolution report

## Scope and method

This report dispositions every F01-F34 candidate from the one-time audit of
commit `9a86034` and records the implementation completed on `main`. Before any
edit, the repository was fetched and confirmed clean, with local `main` exactly
at `origin/main` commit `022baa6` (the newest remote branch tip by commit time).

Each finding was retraced through the current public contract, implementation,
tests, examples, and maintained documentation. The work was split into three
independent review groups (core state/acquisition, core maintenance/timing, and
examples/tests), then reconciled against the complete driver state machine.
Hardware assertions were checked against the retained TI LDC1612/LDC1614
datasheet. A proposed remedy was used only when it remained the smallest proper
fix after that review.

## Finding dispositions

| ID | Disposition | Result |
| --- | --- | --- |
| F01 | Valid; fixed with F06 | Internal-oscillator validation now always uses the guaranteed 35-55 MHz device envelope for reference-clock and divider limits. The caller's nominal value remains the frequency-conversion input, not a way to narrow silicon tolerance. |
| F02 | Valid; proposal refined | STATUS-after `UNREADCONV` now records that a newer conversion is pending without invalidating the already coherent MSB/LSB pair. Overrun/data-loss evidence remains public. The CLI sample-rate and HIL non-overrun requirements were retained because they are valid owner cadence policies. |
| F03 | Valid; simpler invariant used | No extra identity boolean was added. `AppliedConfigState::UNKNOWN` now consistently means identity has not been established, while `DIRTY` means identity is known but the profile is untrusted. Raw writes, reset failures, identity failures, cancellation, deadlines, and apply admission preserve that fence. |
| F04 | Refuted; no change | A caller can create a reset/identity scheduling boundary with `poll(now, 1)` and defer the next poll. A core delay would duplicate owner scheduling policy. |
| F05 | Refuted; no change | Per-conversion faults are decoded from DATAx_MSB, while STATUS-after represents later/sticky device state. Folding it into the captured sample would misattribute faults. |
| F06 | Valid; fixed with F01 | Conservative timing uses 35 MHz as the internal-oscillator minimum instead of narrowing or raising the floor from caller facts. |
| F07 | Valid; fixed with F33 | Ambiguous failed STATUS and DATAx_MSB reads now report `READ_SIDE_EFFECTS`, because the destructive read may have reached the device. Confirmed address NACKs remain side-effect-free. No new redundant effect enum was added. |
| F08 | Valid; fixed | Successful apply, initialize, and reset/reapply results retain `PARTIAL_WRITE` evidence instead of clearing it at completion. |
| F09 | Refuted; no change | DRIVE_CURRENT remains meaningful through sensor activation even without RP override; the proposed validation rule contradicted supported device configurations. |
| F10 | Valid but overstated; fixed | Stale/sleep-cleared zero DATA is no longer inferred to be under-range unless silicon supplied an explicit range flag. Fresh zero remains an under-range endpoint. |
| F11 | Refuted; no change | SETTLECOUNT values 0 and 1 are defined device encodings for 32 reference cycles, not missing configuration. |
| F12 | Valid; fixed | Removed the byte-identical trailing sleeping CONFIG write. Maximum apply/initialize/reset-reapply callbacks are now 13/15/16 for LDC1612 and 23/25/26 for LDC1614. A subsequent `wake()` remains the single active CONFIG write. This is a patch-level correction, not a new feature. |
| F13 | Valid maintainability issue; narrowed fix | Reused `_recordConfigFault()` in the direct diagnostic-write path. A larger generic reset/replay helper was rejected after F03 made their `UNKNOWN` versus `DIRTY`/`APPLYING` transitions materially different. |
| F14 | Valid maintainability issue; fixed | Manufacturer and device identity verification now share a small table-driven step, retaining exact phase/register/status provenance. |
| F15 | Refuted; no change | Separate Arduino and native ESP-IDF CLI implementations are an explicit repository boundary. Command-surface parity is enforced by host contract checkers, with behavior maintained in mirrored implementations rather than cross-framework compilation. |
| F16 | Valid; fixed | CLI session completion no longer rewrites `CANCELLED` or `TIMED_OUT` as `FAILED`. Only a nominally successful acquisition with a missing batch is reclassified as failure. Cancellation is excluded from failure accounting. |
| F17 | Refuted; no change | Freshness is intentionally sampled by STATUS-before for the batch protocol. A conversion arriving during readout belongs to a later acquisition and is retained as overrun/pending evidence. |
| F18 | Valid contract gap; completed | The callback already documented repeated START after the original audit pass. Wording was tightened to require a combined pointer-write/read transaction and to state that STOP-separated emulation is unsupported and has no coherence guarantee. |
| F19 | Valid; already corrected | Confirmed the current docs correctly state that `DRDY_2INT` gates both STATUS.DRDY and INTB routing. |
| F20 | Valid; already corrected | Confirmed the current quality docs correctly state that WATCHDOG can be raised from DATAx_MSB or STATUS.ERR_WD. |
| F21 | Refuted; no change | `CONFIG_UNKNOWN` is an intentionally retained compatibility enumerator documented as never emitted; `AppliedConfigState` is the authoritative contract. |
| F22 | Refuted; no change | `CommandTable.h` is a public register-map header. Constants need not have an in-repository caller to serve downstream diagnostic users. |
| F23 | Valid contract gap; completed | Existing Doxygen already covered cancellation and result-slot effects. The recovery example now drains/correlates retained results and checks `startInitialize()` admission instead of silently ignoring backpressure. |
| F24 | Refuted; no change | Checked math helpers deliberately accept a complete validated-style `Config`; "pure" means bus-silent and side-effect-free, not transport-field-agnostic. |
| F25 | Valid maintainability issue; fixed | Removed the duplicate `firstChannel()` concept and centralized the repeated acquisition channel/phase transition in `advanceAcquireChannel()`. |
| F26 | Valid contract gap; fixed | Public and integration docs now state that sleep destroys DATA, unread/error, and INTB evidence. The behavioral fake clears the same conversion evidence, with a regression covering sleep/wake acquisition. |
| F27 | Valid observability limitation; documented | Valid configurations with disabled ERROR_CONFIG routes remain supported. Docs now explain that `all()` is the maximal available routing, STATUS attributes one channel, zero-count has no DATA route, and `validChannels` means fresh with no decoded range/silicon error rather than proof that silicon was fault-free. Overrun/data-loss is orthogonal and can coexist. |
| F28 | Valid; already corrected | Confirmed the obsolete timing-checker allow-list code had already been removed in the audit preparation commit. |
| F29 | Refuted; no change | The small `isConfirmed()`/`requireConfirmation()` separation is live, file-local policy code and introduces no behavioral or maintenance defect. |
| F30 | Valid; fixed | A verify read failure now increments `skipped`/read-failure accounting once, not both mismatch and failure totals. Session success requires both zero mismatches and zero skipped reads. Both CLIs are covered. |
| F31 | Refuted; no change | Owner transport counters intentionally count attempted physical transaction kinds; pre-transaction context/address rejection is outside that denominator. Changing it would violate the existing counter schema. |
| F32 | Valid; proposal improved | The native ESP-IDF CMake target now injects UTC build date and time. Both CLIs emit one whitespace-safe `YYYY-MM-DDTHH:MM:SS`-shaped token; the contract checker verifies all provenance definitions. |
| F33 | Valid duplicate of F07; fixed once | Code review covers possible destructive effects in failed STATUS-before, DATAx_MSB, and STATUS-after branches. Public regression pins the observable first-read ambiguity and confirmed address-NACK exception; later ambiguity shares the already-set aggregate effect bit. |
| F34 | Partly valid; direct contract test added | The production variant gates were correct; making the fake infer a variant would invent unsupported device behavior. Added direct bus-silent regressions for every channel 2/3 configuration-register class, a DATA boundary, and invalid/empty acquisition masks on an LDC1612 binding. |

## Implementation summary

- Corrected internal-clock bounds, timing estimates, acquisition validity, and
  destructive-read effect provenance in the framework-neutral core.
- Strengthened the identity/applied-state lifecycle without adding state,
  allocation, retry, delay, or a second execution model.
- Simplified replay and state-machine duplication while preserving exact
  register/channel/phase fault provenance.
- Corrected CLI terminal outcomes, verify accounting, and build provenance in
  both maintained implementations.
- Expanded the behavioral fake and native regressions for timing boundaries,
  successful write effects, reset/identity fences, sleep evidence, overrun
  semantics, ambiguous reads, and LDC1612 gating.
- Updated public and maintained integration documentation, callback counts,
  recovery handling, and the Unreleased changelog.

No retained HIL artifact was modified and no new hardware-validation claim is
made. The changes alter software contracts and simulation only; sensor, INTB,
SD, address-strap, and soak acceptance still require current target evidence.

## Fresh independent re-audit

On 2026-08-30 the original audit was read again from Git history and the
completed change was reviewed independently against the actual code and diff.
Three parallel passes covered F01-F34 requirement disposition, production-core
state/timing/acquisition behavior, and examples/tests/documentation/scope. The
following residual gaps were confirmed and corrected:

- Updated the contributor contract's replay maxima to 13/23 apply, 15/25
  initialize, and 16/26 reset/reapply callbacks.
- Corrected the reset failure prose: a confirmed address NACK on RESET_DEV
  retains prior applied state, while a reset that reached or may have reached
  the device followed by failure leaves identity/configuration unknown.
- Corrected validation status to state that the behavior-changing core work is
  not covered by the retained `e4d0436` no-sensor HIL evidence.
- Added a parity guard for the two audited CLI behavior owners so native
  ESP-IDF cannot silently diverge on terminal outcomes or verify accounting.
- Tightened the native CMake provenance guard to inspect the actual private
  compile-definition block rather than accepting macro names elsewhere.
- Replaced the HIL runner's impossible cached-result fixture with the reachable
  reset/reapply result from the maintained sequence. The parser now accepts the
  reachable success envelopes for every job/variant and rejects contradictory
  identity, revision, outcome, status, effect, phase, and transfer evidence.
- Added negative HIL-parser regressions for malformed build timestamps and a
  behavioral regression for the retained sample-rate non-overrun cadence gate.
- Removed redundant later-read effect mutations. STATUS-before has already set
  the aggregate destructive-read flag before DATAx_MSB or STATUS-after can run;
  ambiguous failure on the first STATUS read remains explicitly handled.

No production-core correctness defect remained after these corrections. The
fresh review reconfirmed the original refutations, the deliberate F02 owner
cadence policy, and the narrowed F13 simplification. The requested resolution
report remains in the tree as this review record; the original one-time audit
input remains available only through Git history.

## Validation

- `scripts/pio.cmd test -e native`: 52/52 test cases passed on the fresh pass.
- `scripts/pio.cmd run -e esp32s2dev` and `-e esp32s3dev`: both Arduino target
  builds passed using the maintained pinned platform.
- Core timing, CLI command-surface, native ESP-IDF example, readiness-claim,
  and repository-hygiene checkers passed.
- HIL runner host suite: 65/65 tests passed. This is parser/tooling validation,
  not a physical HIL run.
- Generated-version drift and clean-package consumer compile checks passed.
- Doxygen and `git diff --check` completed without errors.
- A local native ESP-IDF firmware build was not run because `idf.py` was not
  installed in this shell; the repository's native ESP-IDF contract checker
  passed, and the maintained CI ESP32-S2/S3 IDF build matrix remains the native
  firmware-build authority.
