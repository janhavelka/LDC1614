# Retained audit and port branch review

Review date: 2026-10-23. Reviewed current `main`:
`c0ab4c1712c4135b925354d70c4e6bed5cca62d8`.

## Decision

No important implementation exists only on a retained branch and needs to be
merged into this baseline. The unmerged exploration branch contains reports,
instructions, and prompts, not a driver fix. The old ESP-IDF port and all three
hardening branches are already ancestors of `main`. Their names alone do not
identify outstanding work.

The important remaining work is physical qualification of current firmware,
especially sensor-attached acquisition and the previously observed reset/read
failure. This is a validation gate with an unresolved physical cause, not proof
of a missing software patch. One low-priority metadata suggestion from the old
audit remains an explicit scope decision below.

## Snapshot and method

The lead fetched `origin`; local `main` and `origin/main` were identical and the
working tree was clean before editing. `main` was the newest remote branch tip
by commit date. No historical branch was checked out, merged, deleted, or
modified. The repository AGENTS and CI workflow were read first.

Counts below are `git rev-list --left-right --count main...<ref>`: commits
reachable only from current main / only from the retained ref. Full SHAs pin
this review independently of later branch movement.

| Remote ref after `origin/` | Tip SHA | Main-only / ref-only | Disposition |
| --- | --- | --- | --- |
| `audit/ldc1614-industry-readiness-exploration` | `66b5968d40704827b071e24fdd1a9d169a41c8fd` | 75 / 3 | Unmerged documentation; preserve historical artifact reference, then eligible for retirement. |
| `idf-port` | `9fb17f5493359f78436f1e9297e1faa786e42f05` | 80 / 0 | Already integrated; eligible for retirement. |
| `hardening/ldc1614-industry-readiness` | `aad9f226947d1b0e6c3efb382040b60018ec54ec` | 60 / 0 | Already integrated; eligible for retirement. |
| `hardening/tunnelmonitor-suitability-reaudit` | `1b38aea8725896dc0d2f40b5d8f10f8e6650b555` | 47 / 0 | Already integrated; eligible for retirement. |
| `hardening/com8-hil-transport` | `670907ad8641481c492daef559e47c47153ff30a` | 32 / 0 | Already integrated; eligible for retirement. |

The exploration merge base is
`09ca7d15d7d116668a158fec6e53050fd1ef4873`.
`git cherry main origin/audit/ldc1614-industry-readiness-exploration` reports
three `+` patches. Inspection of every patch confirms their scope:

| Unique commit | Contents | Current disposition |
| --- | --- | --- |
| `f43332661a28136adfe361bc19c34247b439d49a` | Initial exploration report and AGENTS wording. | Historical assessment; current contracts supersede its implementation assumptions. |
| `12fa1bfa12ff0599f755f153baacc94f8e726871` | Replaces generic prompts with a tailored five-stage hardening sequence. | Instructions, not implementation. Reintroducing `prompts/` also violates the current repository hygiene gate. |
| `66b5968d40704827b071e24fdd1a9d169a41c8fd` | Revises report and AGENTS wording. | Preserve as historical evidence; do not overwrite current AGENTS or current validation status. |

The [historical report at its exact tip](https://github.com/janhavelka/LDC1614/blob/66b5968d40704827b071e24fdd1a9d169a41c8fd/docs/LDC1614_INDUSTRY_READINESS_EXPLORATION_REPORT.md)
is the source of the IDs below. `git cherry` only detects patch equivalence;
the following conclusions also inspect the current public API, state machine,
test cases, examples, and maintained guides. Source line numbers refer to the
reviewed main SHA, not the later documentation commit.

## Historical findings reconciled with current implementation

| Old ID / concern | Current disposition and evidence |
| --- | --- |
| H1: unsupported readiness wording and no HIL | Wording/tooling resolved; physical qualification still open. `tools/check_readiness_claims.py`, `docs/VALIDATION_STATUS.md:21`, and `docs/reports/README.md` distinguish retained no-sensor evidence from current-firmware acceptance. See R1/R2 below. |
| H2: clocks, sleep-before-configure, frequency domain | Implemented. `src/LDC1614.cpp:349` uses the guaranteed internal-clock envelope; `:1302` validates the full profile and per-mode divider limits; `:640` replays through FORCE_SLEEP; `:1557` uses explicit configured source/dividers. Tests at `test/test_basic.cpp:520`, `:687`, `:1002`, `:2907`, and `:2998` cover invalid clock/profile and exact boundary behavior. |
| H3: partial multi-register effects | Implemented by the newer operation model. `src/LDC1614.cpp:640` records exact failed phase/register/channel, preserves original Status, and sets UNKNOWN/DIRTY on ambiguous effects; raw writes at `:1224` invalidate authority. Every initialization/apply/reset failure position is covered at `test/test_basic.cpp:1122`, `:1711`, and `:1829`. Successful replay is not claimed to be independent register readback. |
| H4: blocking/recovery latency | Superseded by owner-driven jobs. `poll` at `src/LDC1614.cpp:547` takes a caller time/budget; cancellation at `:597` is bus-silent. There is no old core backoff/bus-reset/yield recovery ladder to repair. Deadline and per-phase cancellation regressions are at `test/test_basic.cpp:1231`, `:1261`, and `:1345`. |
| H5: native IDF clean build, heap CLI, ownership | Implemented using independent native sources, fixed buffers, component version checks, a clean consumer checker, and real S2/S3 IDF build jobs. Inspect `CMakeLists.txt`, `examples/esp_idf/basic/main/CMakeLists.txt`, `Ldc1614IdfCli.h`, `tools/check_idf_example_contract.py`, and `.github/workflows/ci.yml`. Example ownership is documented in `docs/IDF_PORT.md` and `docs/I2C_INTEGRATION.md`. Physical IDF qualification remains R1. |
| M1: variant/access-aware diagnostics | Implemented address/access fences at `src/LDC1614.cpp:1205` and `:1224`, with LDC1612 rejection tests at `test/test_basic.cpp:2354`. Raw configuration writes remain explicitly destructive diagnostics; they do not claim safe validated production values and invalidate applied state. |
| M2: bool readiness hides errors | Superseded: status-returning `readDataReady` at `src/LDC1614.cpp:1092` returns observed status; the old error-hiding bool convenience API is absent. |
| M3: copying/thread/ISR contract | Copy/move deleted at `include/LDC1614/LDC1614.h:247`; external serialization and callback non-reentrancy are current public/owner contracts. Compile/type assertions are in `test/test_basic.cpp:326`. |
| M4: negative/variant/failure coverage | Material gaps addressed: both variants and identities, every initialization transfer, all replay jobs, acquisition order/failure, ambiguous effects and address-NACK distinctions are exercised in `test/test_basic.cpp:1002`, `:1122`, `:1940`, `:1999`, `:2354`, and `:2579`. A numerical code-coverage threshold is not implemented; that optional tooling suggestion is not an unmerged driver fix. Tests were inspected, not re-executed in this documentation review. |
| M5: deglitch ambiguity, IDRIVE reference, reset timing | Deglitch uses encoding 7 in `include/LDC1614/CommandTable.h:220`; the conflicting vendor table is retained in `docs/reference/extracted-md/08_variant_differences_and_open_questions.md:30`. Typed current configuration replaces the obsolete table reference. Reset timing remains a physical gate R2; the owner can separate reset and identity callbacks with `poll(now, 1)`. No unsupported mandatory delay is inferred. |
| M6: IDF INTB pull-up | Fixed: both internal pulls are disabled in `examples/esp_idf/basic/main/main.cpp:88`; physical INTB validation is still R1. |
| M7: diagnostic/production-owner boundary | Covered by `docs/I2C_INTEGRATION.md`, `docs/IDF_PORT.md`, and both example READMEs. Application locking, deadlines, bus recovery, and retries are outside the core. |
| M8: error routing hides channel faults | Diagnostic defaults use `ErrorReporting::all()` in Arduino `main.cpp:150` and native IDF `main.cpp:189`; `buildErrorConfigRegister` and boundary tests distinguish routes. `docs/CODE_AUDIT_RESOLUTION.md` F27 explains residual silicon observability limits. |
| L1: unstable/missing generated version metadata | Implemented: tracked `Version.h` has stable unknown defaults, `scripts/generate_version.py:157` handles reproducible build timestamps, and the root component checks generated metadata. CI checks clean package consumption and version drift. |
| L2: probe before binding ambiguity | Superseded by explicit zero-I/O `bind` and identity-verifying `startInitialize`; typed diagnostics return NOT_BOUND before binding (`src/LDC1614.cpp:370`, `:445`, `:1205`). |
| L3: component targets metadata | Still absent from `idf_component.yml`. CI explicitly builds S2/S3, while the neutral component is not restricted to those targets. This is R3, a low-priority policy decision, not missing portability code. |

## Work that remains useful

### R1 - Required before deployment claims: qualify current firmware

Owner: board/application integration. Follow [HIL validation](HIL_VALIDATION.md)
and [validation status](VALIDATION_STATUS.md). Existing clean `e4d0436` logs
exercise an ESP32-S2/LDC1614 fixture without sensors and predate behavior changes;
they cannot validate this baseline. Native tests cannot supply the missing
sensor, wiring, timing, or analog evidence.

Acceptance requires firmware-reported exact revision and clean status, fixture
and clock/profile identity, real coils on all selected channels, frequency and
freshness evidence, STATUS/DATA/UNREAD/INTB side effects, safe induced faults,
owner cancellation/recovery, production-cadence soak, and variant/address/backend
coverage actually claimed by the product. Retain raw serial plus the runner's
hash-bound JSON; record unavailable cases explicitly. Include native IDF 6.x
when that is the deployed backend. This work is not recoverable by merging the
old exploration report.

### R2 - Required when reset is exposed: resolve the persistent read failure

`docs/VALIDATION_STATUS.md` records a successful RESET_DEV write followed by a
failed manufacturer-ID read on `5e3199e`, and an MCU-only restart read failure
on `e4d0436` relieved by a rail cycle. Raw detail 259 is ambiguous on that
Arduino/IDF generation; it does not prove a stuck shared bus or a core bug.

Acceptance: reproduce with exact current firmware and reviewed sensor/SD/rail
wiring, capture reset/clock/rail and combined I2C transactions, distinguish
device state from controller state, and prove bounded return to identity plus
configuration replay/readback. If a software defect is demonstrated, add a
targeted native regression and fix current main, then repeat physical tests.
Until then, keep reset qualification open and follow the documented recovery
boundary; do not transplant the old synchronous recovery ladder.

### R3 - Optional metadata decision: supported target declaration

Owner: package maintainer. Decide whether `targets` should limit component
registry admission to S2/S3 or whether the framework-neutral component should
stay target-independent. Either record the latter explicitly in the IDF guide,
or add the intended targets and keep component/package checks and real target
builds aligned. A missing targets list alone is not a build failure. This change
is outside this documentation-only review.

## Reproduction and retain/retire workflow

Run the comparison again after any branch movement, before deciding retirement:

```powershell
git status --short --branch
git fetch origin
git rev-parse HEAD origin/main
git for-each-ref --format='%(refname:short) %(objectname) %(committerdate:iso-strict)' refs/remotes/origin
$auditRef = 'origin/audit/ldc1614-industry-readiness-exploration'
git merge-base main $auditRef
git rev-list --left-right --count "main...$auditRef"
git log --format='%H %s' "main..$auditRef"
git cherry main $auditRef
git diff --stat "main...$auditRef"
git show "${auditRef}:docs/LDC1614_INDUSTRY_READINESS_EXPLORATION_REPORT.md"
git merge-base --is-ancestor origin/idf-port main
```

1. Keep current implementation and this disposition on the existing main branch.
2. Track R1/R2 with the product fixture owner; do not reopen superseded code
   findings merely because their original commits are unmerged.
3. Retire already-integrated refs when no active consumer/PR depends on them.
   Before retiring the unique audit ref, preserve its exact tip under a durable
   archival ref or a verified Git bundle. A written SHA alone does not
   keep an otherwise unreachable object alive. Retirement is a separate action;
   this review deleted no refs.
4. For a demonstrated behavior fix, add a focused regression, follow existing
   CI, and verify the exact final commit. Do not replay old prompts or merge the
   report branch wholesale. Commit only owned changes and push without force;
   confirm upstream synchronization, as required by AGENTS.

## Checks and CI boundary

For this documentation-only change, these current repository checks passed:
`check_core_timing_guard.py`, `check_cli_contract.py`,
`check_idf_example_contract.py`, `check_readiness_claims.py`,
`check_repository_hygiene.py` (all under `tools/`),
`python scripts/generate_version.py check`, and `git diff --check`.
The repository hygiene check includes local Markdown links. This review made
no claim to execute native C++ tests, target builds, Doxygen, or physical HIL.

For implementation changes use the exact commands in
[validation status](VALIDATION_STATUS.md) and `.github/workflows/ci.yml`:
native PlatformIO tests, Arduino S2/S3 builds, core/CLI/IDF guards, HIL parser
tests, clean consumer compilation, generated metadata, and strict Doxygen.
CI additionally builds native S2/S3 with ESP-IDF 6.0.2. Windows PlatformIO runs
must use `scripts/pio.cmd` and the existing installation. No manual CI dispatch
is required for this documentation chunk; a normal main push may trigger CI.
No older green run is evidence for a new implementation commit.
