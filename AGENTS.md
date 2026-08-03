# AGENTS.md - LDC1614 Production Embedded Guidelines

## PlatformIO

Before editing, fetch remotes and fast-forward the newest intended working
branch to its upstream. Stop and report dirty, divergent, or conflicted state;
never overwrite work to force a sync.

On Windows, use `.\scripts\pio.cmd <arguments>`; it selects the current user's
VS Code-managed installation. Never install another PlatformIO Core; if the
wrapper cannot find it, stop and report the missing installation.

## Role and Target

You are a professional embedded software engineer building a production-oriented LDC1614/LDC1612 multi-channel inductance-to-digital converter library.

- Target: ESP32-S2 / ESP32-S3, Arduino framework, PlatformIO, and native ESP-IDF component use.
- Goals: deterministic behavior, long-term stability, clean API contracts, portability, no surprises in the field.
- These rules are binding.

## Repository Model (Single Library)

```
include/LDC1614/         - Public API headers only (Doxygen)
  CommandTable.h         - Register addresses and bit masks
  Status.h
  Config.h
  LDC1614.h
  Version.h              - Auto-generated (do not edit)
src/                     - Implementation (.cpp)
examples/
  01_*/
  common/                - Example-only helpers
  esp32/                 - Native ESP32 transport shared by target examples
platformio.ini
library.json
README.md
CHANGELOG.md
AGENTS.md
```

- `examples/common/` is not part of the library. It simulates project glue and keeps examples self-contained.
- `examples/esp32/` is also example-only application glue. It owns native
  ESP-IDF bus/device handles for the maintained ESP32 diagnostics.
- No board-specific pins or bus objects in library code; inject non-owning callbacks through `Config`.
- Public headers live only in `include/LDC1614/`.
- Keep the layout boring and predictable.

Framework boundaries:

- Core/public headers and `src/` remain framework-neutral.
- Core/public headers and `src/` must not depend on Arduino, Wire, ESP-IDF, FreeRTOS, logging frameworks, global bus objects, framework delays, or heap-heavy framework types.
- Arduino examples may use Arduino APIs.
- Native ESP-IDF examples use `app_main`, `driver/i2c_master.h`, and native GPIO/timer/task APIs. They must not use Arduino compatibility facades.
- Arduino and native ESP-IDF diagnostics may share the ESP-IDF new-master
  transport in `examples/esp32/`. It remains example-only and contains no
  Arduino API dependency.
- Keep Arduino/IDF example command parity through repo-local contracts/checkers, not by compiling one framework's sources into the other.

## Core Engineering Rules (Mandatory)

- Prefer simplicity, clarity, correctness, robustness, safety, and readability over clever abstractions or speculative flexibility.
- Inspect whether existing code can be simplified, reused, or deleted before adding code.
- Extend existing owners and contracts instead of creating parallel abstractions.
- Do not add placeholder managers, future stubs, generic frameworks, registries, or plugin systems without a current caller and test.
- Keep changes tightly scoped and preserve unrelated dirty changes.
- No unbounded loops, waits, retries, allocations, queues, or buffers.
- Every transport callback is timeout-bounded and has an observable failure path.
- Recovery and reconciliation are bounded, deterministic, and testable.
- Prefer explicit state and ownership. Never hide hardware failure behind retry or fake success.
- Cooperative lifecycle: zero-I2C `bind(const Config&)`, explicit `start*()` jobs, budgeted `poll(uint64_t nowMs, uint8_t maxTransfers)`, bus-silent `cancelJob()`/`invalidateAppliedState()`, and bus-silent `end()`.
- Every multi-transfer operation is a state machine advanced only by `poll()`. Each callback consumes one poll budget unit; `maxTransfers == 0` performs no I2C.
- No heap allocation in steady state; no `String`, `std::vector`, or `new` in normal paths.
- No logging or delays in library code.
- Use `static constexpr` for constants. Macros are only for conditional compilation or example logging.
- Public APIs are not ISR-safe. Driver instances are not internally thread-safe. Document application serialization.
- Injected transport and INTB callbacks must not re-enter the same driver
  instance. Owner time supplied to jobs/polling is one modulo-`uint64_t`
  monotonic timeline; deadline horizons are shorter than 2^63 ms. Wrapping
  hardware clocks narrower than 64 bits are extended before entering the core.
- Conversion timing, settling timing, and sensor-frequency calculations are checked against explicit clock/count/divider facts.

## I2C Manager and Transport (Required)

- The application has one clear I2C owner. The library never owns, configures, resets, or recovers the bus.
- The application owns locking, pins, scheduling, per-transfer timeout policy, retries, device admission, health, and recovery.
- `Config` injects non-owning function pointers and contexts. Transport errors map to `Status`; framework error types do not leak.
- Every callback invocation is one physical attempt. The core never retries transport.
- Keep chip protocol in this driver and application policy outside it.
- Do not add fake devices or simulated buses to production paths.

## Status and Effect Integrity (Mandatory)

All fallible APIs return:

```cpp
struct Status {
  Err code;
  int32_t detail;
  const char* msg;  // static string only
};
```

- No exceptions or silent failures.
- Public device writes report the exact failed phase/register/channel.
- Partial and ambiguous hardware effects remain visible through structured fault/effect provenance with the full original `Status`.
- Cached desired state is never presented as verified hardware state.
- Only a complete successful replay or verification clears dirty/unknown applied state.

## LDC1612/LDC1614 Requirements

- Typed addresses support `0x2A` (ADDR to GND) and `0x2B` (ADDR to VDD).
- `DeviceVariant` is an explicit application fact. Identity registers do not imply automatic LDC1612/LDC1614 detection.
- The initialization job reads MANUFACTURER_ID and DEVICE_ID before applying configuration.
- LDC1612 exposes channels 0-1; LDC1614 exposes channels 0-3.
- Every physical channel of the selected variant has explicit RCOUNT,
  SETTLECOUNT, CLOCK_DIVIDERS, OFFSET, and DRIVE_CURRENT values because replay
  writes them all. Selected channels additionally have expected
  sensor-frequency bounds.
- Support single-channel continuous and multi-channel sequential modes, typed deglitch bandwidth, sleep/active transitions, and software reset.
- DATA reads follow MSB-before-LSB coherency and preserve STATUS/DATA/UNREAD/INTB destructive-read evidence.
- Acquisition publishes fixed-size selected/valid/fresh/error/overrun masks,
  explicit quality, pre/post STATUS, and the owner-supplied terminal poll
  boundary timestamp. Applications timestamp again after `poll()` if they need
  wall-clock completion time.
- Multi-channel results are sequential readout batches, never claimed simultaneous frames.
- Error reporting and reference-clock frequency/tolerance are explicit validated configuration.
- Sensor-frequency and conservative frame-timing helpers are checked and status-returning.

## Cooperative External-Owner Architecture

- `bind()` validates and retains transport/profile with zero I2C. A default `Config` is deliberately invalid.
- Initialize, full apply, reset/reapply, and selected-channel acquisition use one active-job engine.
- Each job has a nonzero caller operation ID, immutable absolute deadline, cache-only progress, and fixed maximum transfer count.
- Deadline expiry and cancellation issue no new transport callback.
- Terminal results retain operation identity and are consumed exactly once from a fixed-capacity store. A pending result is never overwritten.
- Cancellation can be followed immediately by replacement work while its result remains takeable; finite result capacity provides explicit backpressure.
- Initialization always performs identity plus full replay. It is the path for first boot, hotplug return, backend restart, and brownout recovery.
- Acquisition reads STATUS-before, DATAx MSB/LSB into private scratch, then
  STATUS-after. It publishes a batch only in the terminal result after the
  complete job succeeds.
- A failed or cancelled acquisition publishes no partial batch. Previously
  queued terminal results remain untouched. Destructive read effects remain
  reported.
- `AppliedConfigState` is separate from non-authoritative transport counters. Normal acquisition requires verified active configuration.
- Owner-observed removal, reset, brownout, or bus recovery calls `invalidateAppliedState()` before reuse.
- The core exposes chip reset/replay jobs but no bus-reset callbacks, retry policy, backoff, OFFLINE latch, scheduler wait, yield callback, or second `tick()` execution model.
- Single-transfer typed/diagnostic operations may invoke one bounded callback directly. They expose failure and any cache/config side effect.
- Synchronous multi-transfer convenience loops belong only in clearly labelled external diagnostic helpers, not the production core.

## Versioning and Releases

`library.json` is the version source of truth. `Version.h` is generated and must not be edited.

- MAJOR: breaking API/Config/enum changes.
- MINOR: backward-compatible features or appended error codes.
- PATCH: fixes, refactors, and documentation.

Release steps:

1. Update `library.json`.
2. Update `CHANGELOG.md`.
3. Update README/API/examples together.
4. Commit, then create an annotated tag:
   `git tag -a vX.Y.Z -m "Release vX.Y.Z"`.

## Validation Claims

- Never claim hardware validation without current real LDC1612/LDC1614 logs.
- HIL acceptance must parse identity and the firmware-reported target Git
  revision/status. The host checkout SHA is metadata, not proof of what was
  flashed. Ambiguous or incomplete command output never passes.
- Separate software readiness from physical evidence for INTB/SD behavior, fault injection, sequencing, coil limits, address variants, and soak/HIL stress.
- Bring-up examples are not production bus managers unless they demonstrate application-owned locking, timeout, and recovery policy.

## Naming Conventions

- Members: `_camelCase`
- Methods/functions/locals/parameters/config fields: `camelCase`
- Constants and enum values: `CAPS_CASE`
