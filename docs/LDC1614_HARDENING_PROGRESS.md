# LDC1614 Hardening Progress

## Prompt 01 - H2/M1/M3/M5 core contracts

### Findings addressed

- H2/H3 partial hardware state: configuration writes can leave hardware and cached configuration diverged.
- M1 raw diagnostics: raw register writes were not clearly documented as unsafe cache-desynchronizing diagnostics.
- M3 API contracts: driver copy/move, thread-safety, ISR-safety, and transport recursion contracts were too weak.
- M5 datasheet notes: deglitch 33 MHz ambiguity, IDRIVE table reference, reset/reapply timing assumptions, and `probe()` lifecycle caveat needed explicit documentation.

### Implemented changes

- Added `hardwareConfigDirty()` and `hardwareConfigDirtyError()` runtime diagnostics.
- Added dirty-state fields to `SettingsSnapshot`.
- Added `syncConfig()` to re-apply cached configuration without a reset.
- Dirty state is set for failed configuration writes that may have reached hardware, partial `_applyConfig()`, partial `setSingleChannelMode()`, partial `resetAndReapply()`, and public raw diagnostic writes.
- Dirty state is cleared only after full successful `begin()`, `syncConfig()`, `recover()` reapply, or `resetAndReapply()`.
- `softReset()` leaves dirty state intact until a later successful reinitialization or full sync path.
- `recover()` no longer treats identity-read success as configuration sync when dirty state is set; it reapplies cached configuration before returning success.
- Public raw register access remains available, but it is documented as diagnostic-only and not variant/access-type safe.
- Deleted driver copy/move construction and assignment.
- Added public Doxygen and README notes for thread/ISR safety, external serialization, and callback non-recursion.
- Documented the deglitch 33 MHz `b111` choice and the lack of local evidence for an arbitrary post-`RESET_DEV` delay.

### Public API changes

- Added `Err::CONFIG_DIRTY`.
- Added `LDC1614::hardwareConfigDirty()`.
- Added `LDC1614::hardwareConfigDirtyError()`.
- Added `LDC1614::syncConfig()`.
- Added `SettingsSnapshot::hardwareConfigDirty`.
- Added `SettingsSnapshot::hardwareConfigDirtyError`.
- Deleted copy/move operations for `LDC1614`.

### Tests added

- Copy/move deletion static assertions.
- Deglitch enum value regression test.
- Probe missing-callback behavior test.
- `_applyConfig()` partial failure dirty-state/detail test via `begin()`.
- Dirty clear/retain tests for `syncConfig()`.
- Dirty clear/retain tests for `recover()`.
- Raw diagnostic write dirty-policy tests.
- `setSingleChannelMode()` partial failure dirty-state/detail test.
- `resetAndReapply()` partial failure and success-clear tests.
- `softReset()` dirty-state retention until reinitialization.
- Existing single-register cache-failure test now also asserts dirty state.

### Commands run

- `python tools/check_core_timing_guard.py` -> `Core timing guard PASSED`.
- `python tools/check_cli_contract.py` -> `CLI contract PASSED`.
- `python tools/check_idf_example_contract.py` -> `IDF example contract PASSED`.
- `python scripts/generate_version.py check` -> `Up to date: C:\Users\Honza\Documents\Projects\LDC1614\include\LDC1614\Version.h`.
- `python -m platformio test -e native` -> passed; `84 test cases: 84 succeeded in 00:00:00.995`.
- `python -m platformio run -e esp32s3dev` -> passed; `esp32s3dev SUCCESS 00:00:05.195`.
- `python -m platformio run -e esp32s2dev` -> passed; `esp32s2dev SUCCESS 00:00:04.889`.
- `python -m platformio pkg pack` -> passed; wrote `LDC1614-1.0.0.tar.gz`, which was removed after packaging.

### Remaining related work

- Raw register access is still an expert diagnostic escape hatch, not a full metadata-enforced safe register API.
- H2 clock/divider validation and sleep-before-config sequencing are intentionally left for the next relevant hardening chunk.
- ESP-IDF pure-build validation and HIL/hardware validation remain unclaimed.
