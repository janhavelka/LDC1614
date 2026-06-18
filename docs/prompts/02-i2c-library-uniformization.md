# LDC1614 I2C Uniformization Prompt

Repository: `LDC1614`

Absolute path: `C:\Users\Honza\Documents\Projects\LDC1614`

## Execution Rules

You are working inside this single repository. Implement this prompt directly;
do not repeat the cross-repository audit.

You may spawn subagents for read-only inspection of APIs, tests, I2C
transactions, docs, and diagnostics. Keep final judgment, edits, and
verification in the main agent.

Prefer simple, robust, readable code. Before adding code, inspect whether
existing code can be simplified, reused, tightened, or deleted.

Preserve dirty user changes. Do not commit unless explicitly asked.

## Common Uniformization Target

Apply this shared I2C library contract: injected non-owning transport, `Status` returns, cache-only `getSettings(SettingsSnapshot&) const`, active `probe()`/diagnostics named explicitly, `DriverState` with `state()` and `driverState()`, `isOnline()`, `lastOkMs()`, `lastErrorMs()`, `lastError()`, `consecutiveFailures()`, `totalFailures()`, and `totalSuccess()`.

Keep the common `Err` vocabulary append-only where missing: `OK`, `NOT_INITIALIZED`, `INVALID_CONFIG`, `INVALID_PARAM`, `I2C_ERROR`, `I2C_NACK_ADDR`, `I2C_NACK_DATA`, `I2C_TIMEOUT`, `I2C_BUS`, `DEVICE_NOT_FOUND`, `TIMEOUT`, `BUSY`, and `IN_PROGRESS`. Preserve LDC1614-specific sensor, conversion, and config-dirty codes.

Uniformization is not a new base class or framework. Make only local, source-compatible additions and tests.

## Current State

- Public lifecycle and health are in `include\LDC1614\LDC1614.h`: `DriverState` at line 16, `SettingsSnapshot` at line 66, `probe()` at line 148, `recover()` at line 156, `driverState()` at line 165, `lastOkMs()` through `totalSuccess()` at lines 177-192.
- `SettingsSnapshot` includes `hardwareConfigDirty` and `hardwareConfigDirtyError` at `include\LDC1614\LDC1614.h:87-88`.
- Diagnostic writes are documented as dirty-state producers around `include\LDC1614\LDC1614.h:552`; implementation marks dirty in `src\LDC1614.cpp:153` and updates health in `src\LDC1614.cpp:1572`.
- Recovery has bounded backoff/reset behavior covered by native tests: `test_recover_backoff_prevents_rapid_retry`, `test_recover_uses_bus_reset_callback`, and `test_recover_hard_reset_callback_failure_is_returned`.
- HIL runner exists as `tools\ldc1614_hil_runner.py`.

## Best Sources To Adapt

- Keep LDC1614 as one of the source patterns for bounded reset-aware recovery.
- For HIL parser/contract testing, adapt the BME280 and SSD1315 host-test approach: `BME280\tools\test_run_i2c_hil_parser.py` and `SSD1315\tools\test_hil_runner_parser.py`.
- For dirty config naming, stay aligned with BME280/ADS1115 rather than PCA9555's GPIO-specific `hardwareStateDirty`.

## Implementation Tasks

1. Preserve the existing state, health, dirty-state, and reset-aware recovery contracts. Do not rename public dirty-state APIs.
   Preserve existing compatibility aliases; do not remove or rename public APIs to achieve uniform naming.
2. Add a host-side parser/classifier test for `tools\ldc1614_hil_runner.py` if none exists. Use stdlib `unittest` unless the repo already has a declared Python test dependency. Cover the common minimum `version`, `scan`, `probe`, `settings`, `health`, failure-token classification, and dry-run/parser test contract where the CLI supports it.
3. Ensure HIL runner output can never report PASS without a serial transcript. `tools\ldc1614_hil_runner.py:4-5` already states this intent; add a regression test around the no-port `NOT_RUN` artifact.
4. Review README and Doxygen so `getSettings()` is described as cache-only and `probe()`/diagnostic reads are clearly I2C-active.
5. Keep bus reset and hard reset as injected application callbacks only. Do not add direct pin or bus control.

## API Changes Required

- None expected.

## Simplifications Before Adding Code

- If HIL runner command lists are duplicated in docs, prefer a compact README section plus `python tools\ldc1614_hil_runner.py --help`.

## Tests To Add Or Update

- Host parser/contract test for HIL classification and no-port `NOT_RUN`.
- Native test: `getSettings(SettingsSnapshot&) const` is bus-silent.
- Native tests only if recovery or dirty-state behavior changes.

## Commands To Run

- `pio test -e native`
- `pio run -e esp32s3dev`
- `python tools\ldc1614_hil_runner.py` without `--port` should produce a NOT_RUN artifact and must not claim PASS.
- Live HIL only with hardware and explicit serial port.

## Constraints And Non-Goals

- Do not copy SHT3x command/CRC abstractions into LDC1614 unless the LDC command model needs them.
- Do not introduce unbounded polling for conversion readiness.
- Injected transport only: no global `Wire`, new bus manager, pin ownership, or shared bus reset from the device driver.
- Preserve distinct timeout, address NACK/device-not-found, data NACK, bus, sensor, conversion, and config-dirty statuses. Do not collapse them into generic `I2C_ERROR` or use `DEVICE_NOT_FOUND` for timeout/data/bus failures.

## Risks And Open Questions

- Open: whether LDC1614 HIL should standardize its command name to `tools\run_i2c_hil.py` or keep the device-specific runner name.
