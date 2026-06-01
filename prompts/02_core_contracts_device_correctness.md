# Prompt 02 — Core Contracts and LDC1612/LDC1614 Device Correctness

You are continuing the LDC1614 industry-readiness hardening sequence on the current hardening branch.

Complete only this prompt, commit and push/sync, then stop and report back.

## Goal

Harden the core driver contracts and device-specific correctness. Do not do broad architecture rewrites unless the code clearly violates the existing framework-neutral/injected-transport design.

## Subagents

Spawn at least:

- `core-contracts-agent`
- `datasheet-device-agent`
- `tests-fault-agent`
- `integration-review-agent`

Each must report file paths and concrete findings.

## Device-specific rules to verify and implement

Verify actual source first. Then implement only high-confidence changes.

### 1. LDC1612 vs LDC1614 variant handling

Audit all public channel APIs.

Requirements:

- Driver must know whether it is configured for LDC1612 or LDC1614.
- Channels 0 and 1 are valid on both variants.
- Channels 2 and 3 are valid only on LDC1614.
- Public APIs must reject invalid channel selections with a precise status, not silently access registers.
- Docs must state variant behavior.
- Native tests must cover invalid channel rejection for LDC1612 and valid channel 2/3 behavior for LDC1614.

### 2. Identity/probe correctness

Audit ID and probe behavior.

Requirements:

- Read and validate manufacturer/device ID where supported.
- Preserve precise I2C transport errors: address NACK, data NACK, timeout, bus error, generic I2C error.
- Do not collapse all probe failures into “device not found.”
- Add tests for probe success, wrong ID, NACK, timeout, and bus error.
- Document exact mapping.

### 3. DATAx coherency

Datasheet-critical rule: DATAx_MSB must be read before DATAx_LSB for coherent channel data.

Requirements:

- Ensure any `readChannel`, `readRaw`, `readFrequency`, or equivalent path reads MSB then LSB for the same channel.
- Do not allow a public API to read LSB-only or reverse order except low-level diagnostic register read clearly marked unsafe.
- For multi-channel reads, maintain per-channel MSB-before-LSB order.
- Add native tests that fail if the driver reads LSB before MSB.
- Document under API contract and Doxygen/README.

### 4. Data interpretation and out-of-range status

Audit conversion result handling.

Requirements:

- 28-bit DATAx assembly must be correct.
- Preserve or expose error bits/status bits from DATAx_MSB if present in the implementation.
- Treat 0x0000000 and 0x0FFFFFFF / 0xFFF'FFFF style documented under-range/over-range markers as status conditions, not just normal measurements.
- Add clear status values or diagnostic flags for under-range, over-range, amplitude error, watchdog/error bits if the device exposes them.
- Add tests for normal, under-range, over-range, and error-bit samples.

### 5. Timing model: RCOUNT, SETTLECOUNT, channel sequence

Audit any conversion-time or data-ready waiting logic.

Requirements:

- Conversion time calculation must match datasheet formula: `tC = (RCOUNTx * 16 + 4) / fREFx` or the repo’s equivalent with correct units.
- Sensor settle time must account for SETTLECOUNTx and fREF where implemented.
- Multi-channel autoscan timing must consider active channel count and configured sequence.
- If blocking read/wait APIs require a monotonic time hook, enforce and document it. Return `INVALID_CONFIG` before starting work when missing.
- Add README/Doxygen latency table for public APIs and conversion waits.
- Add tests for timing calculation and wraparound-safe deadline behavior.

### 6. Multi-register partial-state handling

Audit configuration operations that write multiple registers, such as per-channel setup, clock dividers, RCOUNT/SETTLECOUNT/OFFSET/DRIVE_CURRENT, CONFIG, MUX_CONFIG, ERROR_CONFIG.

Requirements:

- If a multi-register operation may partly reach hardware and then fail, expose a `hardwareConfigDirty` / `syncNeeded` state or avoid partial writes by design.
- Preserve original I2C error status.
- Provide a clear `recover()` / `syncConfig()` path that re-applies a known-good cache or re-reads hardware as documented.
- Add tests for failure at each write position of a multi-write operation.

### 7. Copy/move, thread/ISR, shutdown contracts

Requirements:

- Delete copy/move constructors and assignment operators if the driver stores mutable state or transport contexts.
- Document external serialization and non-ISR-safe public APIs.
- If a status-returning shutdown/sleep API does not exist, add one if clean. Keep any `end()`/`sleep()` best-effort API documented honestly.
- Avoid claiming hardware SD-pin control if the library only controls software sleep registers; distinguish SD pin hardware shutdown from register sleep/config.

## Tests

Add/update native fake transport tests. Minimum expected coverage:

- variant/channel validity,
- ID/probe mapping,
- DATA MSB-before-LSB ordering,
- 28-bit data assembly,
- under/over range markers,
- timing formula,
- missing timebase for blocking wait if applicable,
- partial multi-register write failures,
- dirty/sync-needed state,
- copy/move disabled.

## Docs

Update README and Doxygen comments for:

- variant support,
- addresses 0x2A/0x2B,
- DATA coherency rule,
- conversion/settling timing,
- latency/transaction table,
- partial state and recovery,
- thread/ISR safety,
- SD pin vs software sleep distinction,
- coil/sensor-frequency caveat: library config can help, but coil design/application calibration is outside generic driver validation.

## Verification

Run:

```bash
python tools/check_core_timing_guard.py || true
python scripts/generate_version.py check || true
python -m platformio test -e native
python -m platformio run -e esp32s3dev
python -m platformio run -e esp32s2dev
```

Run additional guards if present.

## Report update

Append to or create:

```text
docs/LDC1614_HARDENING_PROGRESS.md
```

Include:

- what changed,
- public API changes,
- tests added,
- commands run,
- remaining open items for prompts 03–05.

## Commit and sync

Before commit:

```bash
git diff --stat
git diff --check
git status --short
```

Commit message:

```text
feat: harden LDC1614 core contracts and device semantics
```

Push/sync, then stop.
