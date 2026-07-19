# TunnelMonitor-node suitability audit

## LDC1614/LDC1612 inductance-to-digital converter library

Date: 2026-07-18

Audit result: **capable v2 foundation, focused refactor and product decision required before integration**

LDC1614 v2.0.0 already contains much of the chip work that should not be
copied into TunnelMonitor firmware. It has framework-neutral transport
callbacks, fixed memory, correct per-channel MSB-then-LSB data ordering,
granular transport errors, LDC1612/LDC1614 channel bounds, configuration-dirty
tracking, and poll-budgeted jobs for selected data reads and configuration
replay.

It is not suitable unchanged. Cold start still performs up to 26 I2C
transactions in one `begin()` call. Existing jobs cannot initialize an
uninitialized or returned device and cannot be cancelled. The staged DATA
read documentation incorrectly says DATAx reads do not affect STATUS or INTB.
The device does have those side effects. Multi-channel reads can leave partial
cache updates and cannot prove that all channels came from one scan cycle.
The default error policy disables the DRDY and error-reporting paths used by
several public helpers. Finally, the library's bus recovery, backoff, and
mandatory `OFFLINE` latch compete with `I2cTask`, which already owns those
policies.

There is also a separate product gate: TunnelMonitor does not currently define
an LDC1614 in its board, I2C, health, measurement, settings, or validation
contracts. The LDC's purpose, channel map, reference clock, coils, units,
calibration, cadence, deadline, and health role are undecided. The existing
RS485 VibWire frequency fields are not evidence that LDC1614 is their
replacement.

The recommended path is:

1. Freeze the intended TunnelMonitor hardware and measurement role.
2. Refactor the v2.0.0 library around one cooperative job engine.
3. Release and exact-pin a reviewed commit.
4. Integrate it through one small owner-private adapter inside `I2cTask`.

Do not add LDC1614 to TunnelMonitor's production dependencies before both the
product decision and library gates are complete.

## Audit basis

The audit used these revisions:

| Repository | Revision | Notes |
| --- | --- | --- |
| TunnelMonitor-node | `fff99fe17e60b9287ec4d8d3eca5b3230ae44223` | Branch `prompt-44b-sequence`; current architecture authority and direct I2C owner implementation |
| LDC1614 previous checkout | `9fb17f5493359f78436f1e9297e1faa786e42f05` | Branch `idf-port`; described as `v1.0.0-8-g9fb17f5-dirty`; materially older than the release candidate |
| LDC1614 current checkout and v2.0.0 | `26d1c1937f02449855060ea5d3a89085fc794b85` | Local `main`, `origin/main`, `origin/HEAD`, and lightweight tag `v2.0.0`; integration/refactor candidate audited here |

The previous `idf-port` checkout had pre-existing edits in changelog, IDF port,
example, manifest, and checker files. They are preserved in Git safety stash
`6fe47f39d3d30d2510ac60d09c47d7a333dc767d`, created as
`codex-safety-before-latest-main-20260718`. Only this audit report was restored
onto `main`. Findings about runtime behavior in this report apply to tagged
v2.0.0 unless the report says otherwise. TunnelMonitor should pin the full v2
commit SHA or a later reviewed commit, not only a movable branch name.

### Latest-branch revalidation

Revalidated on 2026-07-18 after `git fetch origin --prune --tags`. The remote
default branch is `origin/main`, and it is also the newest remote branch by
commit date. `origin/main` and tag `v2.0.0` resolve to
`26d1c1937f02449855060ea5d3a89085fc794b85`. The prior local `idf-port` branch
was zero commits ahead and 23 commits behind `origin/main`. Its seven modified
tracked files and the untracked audit were captured in the named safety stash.
The checkout was then switched and fast-forwarded to `main`; the audit alone
was restored. Local `main` is now exactly aligned with `origin/main`, with
ahead/behind `0/0`.

The report already evaluates the v2.0.0 `origin/main` implementation rather
than treating the older checkout as the integration candidate. The 23-commit
delta includes the v2 core, tests, native ESP-IDF example, CI guards, and HIL
evidence. Those changes are reflected in the strengths, hard findings,
validation evidence, and release gates below. Reinspection found no finding
that needs removal or severity change.

#### Final-main hard-finding verification

The hard findings were checked again against the source now directly checked
out at `main@26d1c193`:

| Finding | Final-main disposition |
| --- | --- |
| H-01 | Retained. TunnelMonitor still has no selected LDC1614 board or product measurement contract. |
| H-02 | Retained. `begin()` still performs the complete synchronous identity and configuration sequence; staged jobs cannot initialize an uninitialized instance. |
| H-03 | Retained. Active staged jobs still have no public cancellation operation. |
| H-04 | Retained. DATA register reads still have STATUS/UNREAD/INTB side effects that the public documentation does not model correctly. |
| H-05 | Retained. Default error configuration still disables readiness/error paths used by public helpers. |
| H-06 | Retained. Multi-channel completion still does not prove one coherent scan cycle or publish an atomic batch. |
| H-07 | Retained. Driver-owned OFFLINE, backoff, reset callback, and recovery admission still overlap `I2cTask` ownership. |
| H-08 | Retained. A structurally decoded sample can still be marked valid while sensor error conditions make it unusable for the product. |
| H-09 | Retained. High-level measurement reads are still available while hardware configuration is dirty or unverified. |
| H-10 | Retained. Frequency, reference-clock, offset, and timing calculations still lack a frozen TunnelMonitor product contract. |
| H-11 | Retained. Variant, channel, clock, coil, and electrical assumptions remain application decisions that the current API can make appear more certain than they are. |
| H-12 | Retained. The repository still documents both a blocking managed-synchronous model and poll-budgeted jobs without one clear production execution contract. |

Primary chip behavior was checked against the bundled Texas Instruments
[LDC1612/LDC1614 datasheet](reference/LDC1614_datasheet.pdf), revision A, document
SNOSCY9A. PDF pages 14, 38 through 40, and 46 through 48 were rendered and
visually reviewed. They cover I2C transaction restrictions, 28-bit shadow
register ordering, frequency and conversion timing, multi-channel timing,
clock limits, STATUS side effects, and unread conversion loss. The bundled TI
sensor-status application note was also checked for DATAx/STATUS/INTB clear
behavior. Poppler was unavailable, so PyMuPDF was used. Temporary render files
were removed.

This audit changed no library or TunnelMonitor source, selected no production
dependency, and ran no new physical hardware test. The only intended source
tree change is this report.

## Decision summary

### Use after a focused refactor

These are the release gates for TunnelMonitor integration:

1. Add a zero-I2C bind/configure step and cooperative initialize/reinitialize
   jobs. Initial identity and configuration must obey the caller's transfer
   budget.
2. Add bus-silent cancellation and a cache-only job status/progress snapshot.
3. Replace separate freshness and staged-read paths with one cooperative
   acquisition job that returns STATUS evidence and fixed per-channel masks.
4. Correct DATAx/STATUS/INTB side-effect documentation and model those effects
   in the fake device tests.
5. Commit batch outputs and sample cache only when the complete requested job
   succeeds. Never expose a half batch as a new result.
6. Remove library-owned bus reset, recovery backoff, and mandatory offline
   admission control. Keep chip reset/configuration replay as explicit jobs.
7. Make readiness and error reporting an explicit typed configuration. Do not
   allow a blocking DRDY helper to wait on a DRDY source that is disabled.
8. Add explicit sample quality for under-range, over-range, watchdog,
   amplitude, stale, data-loss, and unknown-configuration cases.
9. Put the reference-clock facts in validated configuration and provide a
   precise, status-returning frequency helper plus conservative frame timing.
10. Exact-pin the reviewed commit and prove the private Tunnel adapter with
    native tests and sensor-attached ESP32-S3 hardware evidence.

### Do not solve this with adapter band-aids

Avoid these long-term workarounds:

- calling the current synchronous `begin()`, `readFreshChannels()`, or
  `recover()` from one owner poll;
- setting `offlineThreshold` to a large value to avoid the library latch;
- reconstructing the object or calling `end()` to cancel an expired job;
- reading STATUS in the adapter and DATA registers in the library as two
  competing protocol implementations;
- accepting partial per-channel cache changes after a failed batch;
- treating `FreshChannelData::valid` as proof that the count is usable;
- treating the existing VibWire Hz fields as LDC1614 fields;
- choosing IDRIVE, RCOUNT, SETTLECOUNT, deglitch, or clock values without the
  actual LC tank and target requirements;
- keeping direct and library LDC1614 protocols in parallel after qualification;
- adding another I2C task, a generic sensor framework, a registry, a plugin
  system, or dynamic measurement fields; or
- exposing LDC1614 library types through TunnelMonitor public contracts.

The missing chip behavior belongs in the reusable library. The firmware
adapter should contain transport mapping, owner scheduling, and application
policy only.

## TunnelMonitor requirements and current product gaps

The library must fit the existing owner model. The firmware should not weaken
that model to fit a driver.

| Requirement or fact | Current authority or evidence | Consequence for LDC1614 |
| --- | --- | --- |
| One I2C owner | `docs/guidelines/i2c_peripherals.md:28-35`; `include/TunnelMonitor/BoardPins.h` labels GPIO8/GPIO9 as owned by `I2cTask` | Only `I2cTask` may invoke LDC transport. The library owns no bus, task, queue, lock, retry, or bus recovery. |
| Cooperative normal work | `docs/guidelines/i2c_peripherals.md:100-133` | One normal owner poll advances at most one library transport callback. Multi-register work is an active job. |
| Bounded callbacks | `include/TunnelMonitor/i2c/I2cConfig.h:63-66` | Normal I2C callbacks use a 20 ms cap. The owner retains the whole-operation deadline. |
| Current I2C inventory | `src/i2c/I2cDiagnostics.cpp:45-54`; `include/TunnelMonitor/i2c/I2cConfig.h:81` | The five known devices are RTC, FRAM, ENV, INA228, and OLED. LDC1614 has no row, address, role, or operation. |
| Board source of truth | `include/TunnelMonitor/BoardPins.h:26-30,71-87` | There is no LDC address, ADDR strap, INTB, SD/shutdown, reference-clock, or channel wiring record. These must be decided first. |
| Public I2C contract | `include/TunnelMonitor/contracts/FieldBus.h:13-25` | `I2cOperation` has no LDC probe/configure/read operation. Add one only after product semantics are frozen. |
| Public device health | `include/TunnelMonitor/contracts/Health.h:76-97`; `include/TunnelMonitor/contracts/Capacities.h:81-90` | There is no LDC `DeviceId`, and the production device-health count already equals the fixed capacity of 16. Adding a row requires a bounded status/JSON review. |
| Current sample schema | `include/TunnelMonitor/contracts/Sample.h:19-90`; `docs/guidelines/measurement_data.md:136-160` | The schema is `tm.v1.vw8_shzk16_env_power`. It has no LDC count, frequency, inductance, displacement, or level fields. |
| Current frequency source | `include/TunnelMonitor/contracts/Rs485Devices.h:63-76`; `include/TunnelMonitor/contracts/Sample.h:42-58` | `vw_f_01..08` comes from the RS485 VibWire device and has a 300-6500 Hz acceptance range. Do not map LDC channels into it without an explicit replacement decision. |
| Fixed sample capacity | `include/TunnelMonitor/contracts/Capacities.h:97-100` | The current 37 fields leave 11 numeric slots. Four raw counts fit, but raw plus Hz plus derived values may not. Schema and replay/cloud compatibility must be designed deliberately. |
| Settings capacity | `docs/guidelines/settings.md:338-341` | Redacted status uses 49 of 51 rows. Do not add many per-channel settings piecemeal. Prefer one compile-time board profile first. |
| Dependency policy | `docs/guidelines/dependency_policy.md:10-12,71-90` | A production library must be exact-pinned, private behind the owner, natively tested, and HIL-proven. |
| Memory policy | TunnelMonitor `AGENTS.md`, Dependencies and Memory | Use fixed state only. Do not add dynamic containers or unbounded queues in steady firmware paths. |

### Product decisions that are still open

These are not library defects. They must be resolved before end-to-end
suitability can be declared:

- Is LDC1614 actually populated on hardware revision 2.0.0 or on a later board?
- Is the part LDC1614 or LDC1612? The shared device ID cannot distinguish them.
- Is ADDR strapped for `0x2A` or `0x2B`?
- Which channels are populated, and what physical quantity does each channel
  represent?
- Are output fields raw 28-bit counts, frequency, inductance, displacement,
  level, or a combination?
- Which units, rounding, invalid-value rules, and calibration version are used?
- What are the LC tank values, expected sensor-frequency range, coil Q,
  target material/range, and temperature compensation method?
- Is the reference clock internal or external? What is its actual frequency
  and tolerance?
- What sample cadence and owner operation deadline are required?
- Must every selected channel be fresh? Is a sequential mixed-age batch
  acceptable, or must data loss make the batch invalid?
- Is LDC health required, optional, or diagnostic-only?
- Are INTB and SD/shutdown physically wired? If yes, which owner-controlled
  GPIOs and polarities are used?
- Is configuration compile-time board policy or durable operator configuration?

A simple first direction is one fixed board profile, one explicit variant,
one fixed channel mask, raw 28-bit counts plus typed quality/status, and no
operator-tunable analog settings. Frequency or physical-unit fields can be
added after the clock and calibration contract is proven.

## Fit matrix

| Area | v2.0.0 state | TunnelMonitor decision |
| --- | --- | --- |
| Framework-neutral injected transport | Good | Keep |
| Fixed arrays and no core heap/log/delay | Good | Keep |
| Granular NACK/timeout/bus errors | Good | Keep and map in the adapter |
| Per-channel 28-bit MSB-then-LSB ordering | Good | Keep |
| LDC1612/LDC1614 channel bounds | Partial | Replace raw `channelCount` with explicit requested variant and masks |
| Autoscan minimum timing validation | Good | Keep and extend with board clock/profile validation |
| Configuration dirty tracking | Good base | Keep, return structured fault detail, and reject untrusted reads |
| Poll instruction budget | Good base | Keep as the one job engine |
| Cooperative cold initialization | Missing | Required |
| Cooperative fresh acquisition | Missing | Required |
| Public job cancellation | Missing | Required |
| Atomic multi-channel batch commit | Missing | Required |
| Correct STATUS/INTB side-effect contract | Incorrect | Fix before integration |
| Explicit sample quality and data-loss masks | Missing | Required |
| External health/recovery ownership | Conflicts | Refactor |
| Exact TunnelMonitor board evidence | Missing | Required after product decision |
| Sensor-attached conversion evidence | Missing | Required before field use |

## What already fits

These v2.0.0 properties should be retained:

- Core headers and source do not include Arduino, ESP-IDF, FreeRTOS, `Wire`,
  or logging APIs.
- Transport callbacks are non-owning, carry an explicit timeout, and preserve
  address NACK, data NACK, timeout, bus, and generic transport detail.
- The core uses fixed arrays and contains no `new`, heap container, unbounded
  queue, framework delay, or hidden logging path.
- Driver instances are noncopyable and nonmovable, which avoids duplicating
  non-owning callbacks and device state.
- `readChannel()` reads DATAx_MSB before DATAx_LSB and reconstructs the 28-bit
  count correctly. This matches the device shadow-register requirement.
- Configuration validation rejects invalid addresses, channel counts, enum
  values, reserved ERROR_CONFIG bits, divider bounds, LDC1612-only sequence
  violations, and the datasheet's selected autoscan minimum counts.
- `hardwareConfigDirty()` records partial configuration risk, and full replay
  writes a sleep-mode CONFIG image before channel/global configuration.
- `startReadChannels()`, `startApplyConfig()`, and
  `startResetAndReapply()` already use an explicit transfer-instruction budget.
- A selected channel is not made available through `getChannelSample()` until
  both its MSB and LSB reads succeed.
- Cached sample presence is separate from a timestamp of zero in v2.0.0.
- The public API documents external serialization and that it is neither
  thread-safe nor ISR-safe.
- Native tests and guard scripts cover a broad set of validation, transport,
  dirty-state, and poll-budget cases.

The existing v2 work is the right base. This audit does not recommend a new
generic driver framework or a register protocol rewrite in TunnelMonitor.

## Hard findings

### H-01: TunnelMonitor has no LDC1614 product contract

Priority: product integration blocker

A repository-wide search of current TunnelMonitor code and authoritative
guidelines finds no LDC1614, LDC1612, inductance, or inductive-sensor contract.
`BoardPins` defines no address or related wiring. The known I2C table and public
operations include only the current RTC, FRAM, ENV, power, and display devices.
The measurement schema contains RS485 VibWire frequency but no LDC data.

Without a physical role and result contract, it is impossible to judge whether
four channels, 28-bit raw counts, the achievable sample rate, internal-clock
accuracy, or multi-channel sequencing meet the product need.

Required before library integration:

- write one hardware decision covering part variant, address, clock, channel
  wiring, LC tanks, INTB/SD wiring, and required versus optional role;
- define the selected channel mask, output units, calibration, cadence,
  deadlines, freshness, and stale/error policy;
- decide whether this is a new measurement source or a deliberate replacement
  for another source; and
- update capacities and durable schema versions only after those decisions.

Do not infer these values from the library defaults or from the current VibWire
schema.

### H-02: cold initialization is synchronous and exceeds the owner budget

Priority: library integration blocker

At v2.0.0, `begin()` validates configuration, performs two identity reads, and
applies the complete configuration synchronously
(`src/LDC1614.cpp:165-275,2195-2258`). The LDC1614 path executes 24
configuration writes, for 26 transport callbacks total. LDC1612 executes 16
callbacks total. `probe()` performs two reads in one call
(`src/LDC1614.cpp:312-348`). A synchronous LDC1614 reset/reapply can require 27
callbacks.

The staged APIs do not solve cold start. `startApplyConfig()` and
`startResetAndReapply()` require `_initialized == true`
(`src/LDC1614.cpp:917-944`). If the device is absent at boot, disappears, or a
reapply failure sets it uninitialized, the only complete entry path is another
synchronous `begin()`.

TunnelMonitor normally allows one backend call per owner poll. With a 20 ms
callback cap, a 26-callback call can hold the owner for far longer than one
normal work slice and delays unrelated devices on the shared bus.

Required refactor:

- add `bind(const DeviceConfig&)` that validates and stores callbacks and
  desired configuration with zero I2C;
- add `startInitialize()` using the existing poll instruction budget;
- include identity, forced sleep, all selected channel configuration, global
  configuration, and final applied-state commit in that job;
- use the same job for first boot, hotplug return, backend restart, and
  brownout recovery; and
- retain desired configuration after every failed phase.

Do not add a synchronous LDC exception to `I2cTask`.

### H-03: active jobs cannot be cancelled

Priority: library integration blocker

The library exposes start/poll methods but no public cancel operation and no
useful job progress snapshot (`include/LDC1614/LDC1614.h:293-338`). If the
owner's immutable operation deadline expires between MSB and LSB or partway
through configuration, the library remains busy. Later public I2C calls return
`BUSY` until the old job is resumed to completion or failure.

`end()` is not a safe cancellation substitute. When the device is believed to
be awake it performs a best-effort sleep write before clearing state
(`src/LDC1614.cpp:288-306`). A deadline cancellation must not issue new I2C.

Required refactor:

- add idempotent `cancelJob()` that performs no transport call;
- discard partial read scratch and leave the last complete batch unchanged;
- if a cancelled job may have written hardware, set applied state to dirty or
  unknown and retain the exact failed/cancelled phase;
- add cache-only `jobState()`/`jobProgress()`; and
- allow a new job immediately after cancellation.

### H-04: DATA reads have destructive status side effects that the API denies

Priority: correctness and diagnostics blocker

The v2.0.0 API documentation says `startReadChannels()` does not read STATUS,
so it does not clear STATUS sticky flags or de-assert INTB
(`include/LDC1614/LDC1614.h:293-301`). That statement is incomplete and can be
wrong in exactly the failure case operators need to diagnose.

The datasheet states that reading DATAx_MSB updates the channel's DATA shadow,
clears `UNREADCONVx`, and can release latched status. The bundled TI status
application note states more directly that reading DATAx clears its output
error bits and, when that channel caused STATUS/INTB, clears the corresponding
STATUS error. Therefore a DATA-only staged read can consume error and interrupt
evidence even though it never reads register `0x18`.

There is a second evidence-loss path. `readFreshChannels(out, count)` reads and
clears STATUS, but the overload discards the captured `DeviceStatus`
(`src/LDC1614.cpp:429-482`). The fake bus does not model shadow latching,
UNREAD clearing, STATUS error clearing, or INTB deassertion, so current native
tests do not detect either issue.

Required refactor:

- make the acquisition result always contain the pre-DATA STATUS snapshot;
- document all STATUS, DATAx, UNREAD, and INTB clear-on-read behavior on the
  relevant APIs;
- remove or deprecate overloads that discard destructive status evidence;
- model these side effects in the fake device; and
- test which evidence is retained when each channel is the error source.

### H-05: the default error policy disables the readiness and error paths

Priority: correctness blocker for default helpers

`Config::errorConfig` defaults to `0x0000`
(`include/LDC1614/Config.h:140-141`). Per the datasheet, that value disables:

- DRDY reporting to INTB and `STATUS.DRDY`;
- under-range, over-range, watchdog, and amplitude reporting in DATAx_MSB; and
- under-range, over-range, watchdog, amplitude, and zero-count reporting in
  STATUS/INTB.

`readDataReady()` polls `STATUS.DRDY` when it cannot use an asserted INTB.
Blocking read helpers rely on `readDataReady()`. With the default
`errorConfig`, those helpers can wait until timeout even while conversions are
running. The API accepts that configuration without warning.

Required refactor:

- replace the raw default mask with typed error/readiness policy;
- validate that any selected DRDY-based helper has DRDY reporting enabled;
- return a configuration error immediately when a disabled source is used;
- provide a named mask builder for DATA output errors, STATUS/INTB errors, and
  DRDY; and
- do not silently force INTB use when TunnelMonitor hardware has no INTB pin.

`UNREADCONVx`-driven acquisition remains a valid polling design, but it needs a
different explicit API from DRDY waiting.

### H-06: multi-channel completion is not an atomic or coherent batch

Priority: measurement integrity blocker

The poll-chunked channel job correctly protects each individual 28-bit channel
from half-read publication. It does not protect the batch:

- each completed channel is committed to the general last-sample cache before
  later channels finish (`src/LDC1614.cpp:1747-1789,1954-1976`);
- if a later channel fails, the ready mask is cleared but earlier cache entries
  remain newer;
- synchronous `readAllChannels()` can leave earlier caller entries and cache
  entries updated when a later read fails; and
- neither staged path reads `UNREADCONVx` or reports selected, fresh, valid,
  error, and overrun masks.

There is also a silicon limit. In autoscan, channels convert sequentially. A
new conversion can overwrite an unread old conversion while a slow multi-poll
read is in progress. Correct MSB-then-LSB order gives coherent bits for one
channel. It does not make four channels simultaneous or guarantee they all
belong to the same scan cycle.

Required refactor:

- read into fixed private scratch for the complete selected mask;
- publish one `SampleBatch` only after every requested phase terminates;
- include `selectedMask`, `validMask`, `freshMask`, `errorMask`, and
  `overrunMask`;
- preserve the pre-read STATUS snapshot and optionally use a post-read status
  check to detect conversions that arrived during readout;
- leave the previous complete batch untouched on transport failure; and
- document sequential timing and mixed-generation limits. Do not name the
  result an atomic or simultaneous frame.

### H-07: health and recovery ownership conflicts with `I2cTask`

Priority: architecture blocker

The library claims application-owned recovery in its integration notes, but
`Config` exposes bus-reset and hard-reset callbacks plus recovery policy,
backoff, and an offline threshold (`include/LDC1614/Config.h:103-156`).
`recover()` can reset the shared bus, perform identity reads, reset the chip,
and replay many registers synchronously (`src/LDC1614.cpp:350-365,2067-2165`).
Normal calls are suppressed when the internal driver reaches `OFFLINE`.

This duplicates TunnelMonitor policy. `I2cTask` already owns queue admission,
deadlines, device presence, retries, shared-bus reset, SCL recovery, backoff,
health, and hotplug behavior.

The internal failure streak is also counted per transfer, not per logical
operation (`src/LDC1614.cpp:1642-1675`). A successful DATAx_MSB read resets the
streak before a repeated DATAx_LSB failure. With a threshold above one, that
logical channel read can fail repeatedly without ever reaching the intended
offline threshold.

There is a brownout risk. Recovery first checks identity and replays
configuration only if `hardwareConfigDirty()` is already true. A reset chip can
respond with the same identity while all registers are back at defaults and
the cached dirty flag is still false. Recovery can then return READY without
restoring the desired configuration.

Required refactor:

- remove bus-reset and hard-reset callbacks from the chip core;
- remove library retry/backoff and mandatory offline admission control;
- keep optional non-authoritative counters only if they do not suppress owner
  requests;
- expose chip-only reset/reinitialize/replay jobs;
- add `invalidateAppliedState()` for owner-observed removal, bus recovery,
  brownout, or device power loss; and
- require full replay or verified configuration fingerprint before samples are
  trusted after return.

### H-08: `valid` does not mean the conversion is usable

Priority: data-quality blocker

`_storeChannelData()` treats only the DATA watchdog flag as an invalid sample.
Under-range, over-range, and amplitude flags are stored but the call returns OK
and the sample is cached as present (`src/LDC1614.cpp:1954-1976`).
`FreshChannelData::valid` means only that data or cache exists.

The datasheet defines raw `0x0000000` as under-range and `0x0FFFFFFF` as
over-range. Those sentinel values remain visible even when ERROR_CONFIG
disables DATA error bits. The current driver does not infer their invalid
quality. A caller that checks only `Status::ok()` and `valid` can publish a
saturated count as a normal measurement.

Required refactor:

- separate transport success from measurement quality;
- preserve the raw 28-bit count for diagnosis;
- mark endpoint under-range/over-range independently of the reporting mask;
- classify watchdog as invalid and amplitude errors as a distinct suspect or
  policy-visible condition;
- include STATUS zero-count and error-channel evidence; and
- let TunnelMonitor decide whether a suspect channel invalidates one field or
  the complete selected batch.

### H-09: high-level reads remain allowed when configuration is untrusted

Priority: correctness blocker

v2.0.0 has useful dirty-state tracking, and `isMeasuring()` returns false when
configuration is dirty. High-level read and staged-read preconditions do not
reject dirty state. They can therefore return a count interpreted with cached
dividers, offset, drive, and mode even when the library explicitly says the
hardware may differ.

Dirty diagnostic detail is packed into one integer as phase, register, index,
and only the low eight bits of the original transport detail. This loses
backend evidence that TunnelMonitor may need.

Required refactor:

- reject normal acquisition while applied configuration is `Unknown` or
  `Dirty`, or mark every result quality as configuration-unknown;
- use a typed `AppliedConfigState`;
- return a structured `ConfigFault` containing the full original `Status`,
  phase, register, and channel; and
- clear dirty state only after a complete successful replay or explicit
  readback verification.

### H-10: frequency, clock, and timing helpers are not yet a platform contract

Priority: required if TunnelMonitor publishes frequency; otherwise P1

`calcSensorFrequency()` accepts a per-call `float fRef` and returns `float`.
Invalid input returns `0.0`, which is not distinguishable from a numeric result
without separate validation (`src/LDC1614.cpp:1461-1471`). A binary32 float has
24 bits of integer precision, so it cannot preserve every step of a 28-bit raw
code. The per-call clock can also disagree with `Config::refClkSrc` and the
configured channel dividers.

The timing helpers correctly document that they omit host readout and
multi-channel overhead. They do not provide the complete conservative budget
that an owner needs: initial wake activation, every selected channel's settle
and conversion time, channel-switch delays, STATUS/DATA readout, and clock
tolerance.

The staged `poll(nowMs, ...)` currently ignores `nowMs`. Sample timestamps use
a separate optional 32-bit callback. With no callback, a new sample is stamped
zero and `sampleAgeMs(now)` can describe time since boot rather than sample
age. TunnelMonitor already owns a wrap-safe 64-bit monotonic clock.

Required refactor:

- include `ReferenceClock { source, frequencyHz, tolerancePpm }` in validated
  desired configuration;
- return `Status` plus `double` or a checked fixed-unit integer such as mHz;
- keep raw count as the primary library result;
- add a fixed-unit conservative selected-frame timing result; and
- accept the owner timestamp at final batch commit, preferably 64-bit, instead
  of maintaining a second ambiguous clock.

Do not add displacement, level, or target calibration to the chip driver.

### H-11: variant and electrical configuration are too easy to imply

Priority: platform API refactor

The library uses a raw `uint8_t channelCount` to represent LDC1612 versus
LDC1614. Both parts report manufacturer ID `0x5449` and device ID `0x3055`, so
software cannot discover the variant from the current identity registers. The
variant must be an explicit hardware fact.

The default `ChannelConfig` is syntactically valid, but it is not a universal
sensor profile. Safe RCOUNT, SETTLECOUNT, FIN/FREF dividers, deglitch, IDRIVE,
offset, and reference source depend on the LC tank, target, clock, required
resolution, and sample rate. `begin()` leaves the chip asleep, which is a good
safety property, but a later `wake()` can run an electrically poor default
profile.

Required refactor:

- add explicit `DeviceVariant { Ldc1612, Ldc1614 }`;
- derive the valid channel mask and sequence rules from that variant;
- use typed `Channel` and `ChannelMask` helpers;
- require an explicit application profile before wake;
- validate fREF/fIN/deglitch/RCOUNT/SETTLE relationships using the known clock
  and expected sensor-frequency range; and
- make drive-current codes typed and provide a datasheet lookup helper.

Do not claim automatic part-variant detection.

### H-12: library guidance contains two incompatible execution models

Priority: maintenance blocker before platformization

The v2 repository guidance requires non-blocking lifecycle and says I/O that
can exceed roughly 1-2 ms must be split into state-machine steps. The same file
later declares a managed synchronous model where all public I2C operations are
blocking. The public header also calls `tick()` a no-op while poll-chunked jobs
use a separate `poll()` API.

This conflict allowed a half-cooperative API: staged reads and replay exist,
but initial probe/configure, fresh acquisition, many setters, and recovery
remain synchronous.

Required refactor:

- choose one production model: zero-I2C bind plus explicit cooperative jobs;
- implement synchronous diagnostic convenience only as a clearly marked
  wrapper outside the production core, if it is still useful;
- make `poll()` the single active-job advancement function;
- remove or redefine the no-op `tick()`; and
- update README, integration notes, AGENTS guidance, and examples together.

## Recommended API and types

Names below are illustrative. Keep the API small and chip-specific.

### Core types

```cpp
enum class DeviceVariant : uint8_t {
  Ldc1612,
  Ldc1614,
};

enum class Channel : uint8_t {
  Ch0,
  Ch1,
  Ch2,
  Ch3,
};

using ChannelMask = uint8_t;

enum class AppliedConfigState : uint8_t {
  Unknown,
  Applying,
  AppliedSleeping,
  AppliedActive,
  Dirty,
};

enum class SampleQuality : uint8_t {
  Good,
  Stale,
  UnderRange,
  OverRange,
  Watchdog,
  AmplitudeSuspect,
  DataLost,
  ConfigUnknown,
};
```

Useful supporting types:

- `I2cAddress` for `0x2A` and `0x2B`;
- `ReferenceClock` with source, integer Hz, and tolerance ppm;
- `DriveCurrentCode` plus a pure microamp lookup;
- `ErrorReporting` typed bit flags and safe mask builders;
- `ChannelConfig` with explicit expected sensor-frequency bounds;
- `DeviceConfig` with variant, selected mask, channel array, and clock;
- `ConfigFault` with full original status, phase, register, and channel;
- `DeviceStatusSnapshot` with raw STATUS and parsed flags; and
- `JobProgress` with kind, phase, requested mask, and completed mask.

### Acquisition result

```cpp
struct ChannelSample {
  uint32_t rawCount28;
  SampleQuality quality;
  uint16_t rawDataMsb;
  bool fresh;
};

struct SampleBatch {
  ChannelMask selectedMask;
  ChannelMask validMask;
  ChannelMask freshMask;
  ChannelMask errorMask;
  ChannelMask overrunMask;
  DeviceStatusSnapshot statusBeforeRead;
  ChannelSample channel[4];
  uint64_t completedUptimeMs;
};
```

The batch must be fixed-size. `validMask` must mean usable according to the
library's silicon-level rules, not merely present in cache. Application policy
can be stricter.

### Job API

```cpp
Status bind(const DeviceConfig& config);        // zero I2C
Status startInitialize();
Status startAcquire(ChannelMask mask);
Status startApplyConfig();
Status startResetAndReapply();
Status poll(uint64_t nowMs, uint8_t maxInstructions = 1);
Status cancelJob();                             // zero I2C
JobProgress jobProgress() const;                // zero I2C
Status takeSampleBatch(SampleBatch& out);       // zero I2C
void invalidateAppliedState();                  // zero I2C
```

The reusable job engine should enforce:

- one active job;
- zero I2C when `maxInstructions == 0`;
- at most the supplied transfer count otherwise;
- immediate stop on first transport failure;
- no hidden retry, wait, yield, sleep, bus reset, or logging;
- full scratch reset on cancel;
- atomic result commit; and
- applied-state dirty marking after a possible partial write.

### Pure helpers

Reasonable helpers for a platform library:

- `channelBit(Channel)` and `validMask(DeviceVariant)`;
- selected mask/channel count from `RRSequence`;
- pure DATAx and STATUS decoding;
- ERROR_CONFIG builders with reserved-bit validation;
- checked frequency conversion to mHz or `double`;
- conversion, settling, wake, channel-switch, selected-frame, and host-read
  timing estimates in integer microseconds;
- RCOUNT/SETTLECOUNT selection helpers from explicit timing requirements;
- deglitch selection from maximum expected sensor frequency;
- drive-current code to nominal microamps; and
- profile validation returning the first concrete violated constraint.

Nice-to-have helpers must remain pure. Do not add a calibration manager,
dynamic channel registry, embedded scheduler, or generic sensor base class.

## Refactor scope by priority

### P0 - required before any TunnelMonitor dependency

1. Zero-I2C bind and cooperative initialize/reinitialize.
2. Bus-silent cancel and job progress.
3. Correct STATUS/DATA/INTB side-effect contract and fake-device behavior.
4. Cooperative acquisition with mandatory status snapshot and fixed masks.
5. Atomic complete-batch and cache commit.
6. Explicit readiness/error reporting policy.
7. External bus recovery and health ownership.
8. Full replay after device return/reset and dirty-state acquisition guard.

### P1 - required before measurement use

1. Typed variant, channel, mask, clock, error policy, and sample quality.
2. Under/overrange sentinel validation and zero-count/status mapping.
3. Precise frequency helper and conservative selected-frame timing.
4. Full original error detail in structured configuration faults.
5. Owner-provided completion timestamp.
6. Sensor-attached tests of shadow, unread, error, and data-loss behavior.

### P2 - release and maintenance improvements

1. Exact-pin PlatformIO and the Espressif platform used by maintained CI.
2. Add pinned host-tool requirements for the serial HIL runner.
3. Rename or document `native_cov` as coverage instrumentation until a report
   and threshold are actually generated.
4. Record vendor URL, document ID/revision, retrieval date, and SHA-256 for the
   primary datasheet in the reference index.
5. Prefer an annotated release tag, while still pinning TunnelMonitor to the
   exact commit SHA.
6. Retain at least one raw target transcript or logic trace for production
   acceptance, not only condensed command counts.

## Validation evidence reviewed

The following checks were first run from an isolated archive of v2.0.0.
Temporary files and the generated package were removed. After the local
checkout was safely switched to the same v2.0.0 commit, the core guards, clean
consumer check, HIL runner host tests, native suite, and both Arduino builds
were rerun directly from `main`.

| Check | Result |
| --- | --- |
| Python syntax and repository static guards | Pass |
| Core framework/timing guard | Pass |
| Arduino CLI contract guard | Pass |
| Native ESP-IDF source contract guard | Pass |
| Readiness wording guard | Pass |
| Version header consistency | Pass |
| Clean packed-consumer compile | Pass |
| HIL parser self-test and dry run | Pass |
| HIL runner host tests | 24/24 pass |
| PlatformIO native tests | 162/162 pass |
| PlatformIO `native_cov` tests | 162/162 pass |
| Arduino ESP32-S3 build | Pass; RAM 22,568 bytes, flash 409,742 bytes |
| Arduino ESP32-S2 build | Pass; RAM 37,000 bytes, flash 401,261 bytes |
| PlatformIO package creation | Pass |

The latest-checkout rerun passed 162/162 native tests and 24/24 HIL runner host
tests. ESP32-S3 used 22,568 bytes RAM and 409,742 bytes flash; ESP32-S2 used
37,000 bytes RAM and 401,261 bytes flash. The timing/framework, CLI, native
ESP-IDF source, readiness wording, generated version, and clean consumer guards
also passed. `native_cov`, package creation, and pure local ESP-IDF compilation
were not repeated during the checkout switch; the exact-commit evidence in the
table and CI paragraph remains the applicable evidence for those checks.

`native_cov` is coverage-instrumented, but the repository generates no
coverage report and enforces no threshold. It must not be described as measured
coverage.

GitHub Actions run `28533687464` at the exact v2.0.0 commit completed
successfully for native tests, library validation, Arduino ESP32-S2/S3, and
ESP-IDF v6.0.2 ESP32-S2/S3. Local pure ESP-IDF build was not rerun in this
audit.

The retained physical evidence is real but limited. The committed COM8 run
used ESP32-S2 and LDC1614 at `0x2A` with no LC sensor and no physical INTB/DRDY
or SD wiring. It reports 1010/1010 chip-only stress commands and 200/200
negative/precondition commands passing. The HIL library core at commit
`42feb3b` has no relevant core/test/build differences from v2.0.0.

That evidence proves chip presence and broad register/configuration command
liveness. It does not prove live sensing. The runner also treats several
informational commands as pass when they return non-empty text without a known
failure token. Repeated `cfg` passes are not field-by-field expected register
comparisons.

## Required new tests

### Library native tests

- `bind()` validates all fields and performs zero callbacks.
- Initialize LDC1612 in exactly 16 instructions and LDC1614 in exactly 26,
  with budgets of zero, one, and larger bounded values.
- Cancel at every read, identity, reset, and configuration phase.
- Cancellation never touches I2C and never publishes partial scratch.
- Configuration cancellation after possible mutation marks state dirty.
- Fake DATAx_MSB reads latch LSB, clear UNREAD, and clear the documented
  channel error/INTB evidence.
- Acquisition returns the pre-read status snapshot even after later DATA
  side effects.
- DRDY helper rejects a configuration where DRDY reporting is disabled.
- Failure injection at every batch phase leaves the prior complete batch and
  public cache unchanged.
- A conversion arriving during readout sets `overrunMask` or an equivalent
  explicit data-loss result.
- Under-range zero and over-range `0x0FFFFFFF` are invalid even with DATA error
  reporting disabled.
- Dirty/unknown applied state rejects or explicitly qualifies acquisition.
- Device return with matching identity still performs complete configuration
  replay.
- Frequency vectors cover raw endpoints, offsets, dividers, invalid clock,
  and precision that exposes binary32 loss.
- Timing tests cover selected frame, clock tolerance, and unsigned wrap if any
  32-bit timing API remains.
- Core guards continue proving no framework calls, logging, heap, or dynamic
  containers.

### TunnelMonitor native tests

- Private adapter maps every library transport and sensor status to the
  existing project error/result types.
- One owner poll invokes at most one normal library transport callback.
- Owner deadline expiry calls bus-silent cancellation and permits a new job.
- Queue wait does not reset the immutable operation deadline.
- Hotplug disappearance invalidates applied state; return triggers identity
  and full replay before acquisition.
- Owner bus recovery remains usable by RTC, FRAM, ENV, INA228, and OLED after
  an LDC failure.
- Required/optional health role matches the product decision.
- A failed current-cycle LDC result is not copied as a new sample.
- Public contracts contain no LDC library type and remain fixed-capacity.
- Sample/schema/capacity changes remain backward-safe and explicit.

### Hardware-in-the-loop tests

Run only the variants and address straps selected for the product. Do not add
irrelevant HIL just to fill a matrix.

For the selected TunnelMonitor board, prove:

- exact ESP32-S3-N16R8 hardware revision and 400 kHz shared bus;
- expected LDC address and both identity registers;
- actual reference clock frequency/tolerance;
- populated channel map and LC tank values;
- configuration readback for RCOUNT, SETTLECOUNT, dividers, offset, deglitch,
  mode, and IDRIVE;
- fresh sensor-attached DATA values and expected physical response;
- DATAx_MSB then DATAx_LSB ordering under active conversions;
- STATUS/UNREAD/INTB side effects and delayed-read data loss;
- under-range, over-range, watchdog, zero-count, and amplitude conditions where
  electrically safe;
- operation deadline cancellation and later clean restart;
- unplug/replug or controlled sensor power loss followed by full replay;
- owner-controlled bus recovery while another shared-bus device remains
  responsive;
- INTB and SD behavior only if those pins are actually wired; and
- a bounded soak at the production channel mask and cadence.

No new HIL was run for this audit. Do not claim these cases passed until the
raw or condensed evidence exists.

## Recommended TunnelMonitor integration shape

After product and library gates are complete:

1. Add one owner-private LDC adapter inside `I2cTask`.
2. Keep generic command admission, priority, absolute deadline, callback
   timeout, cancellation, recovery, presence, health, and result delivery in
   `I2cTask`.
3. Let the library own register addresses, bit encoding, identity checks,
   configuration phases, DATA/STATUS side effects, and fixed job scratch.
4. Configure once after proven identity and after every invalidation/return.
   Routine samples must not replay all registers.
5. Add append-only project `DeviceId`, `I2cOperation`, command/result, status,
   event, and sample fields only after their role is frozen.
6. Map raw count and silicon quality into driver-free project DTOs. Keep
   physical calibration and field acceptance in firmware policy.
7. Exact-pin the reviewed immutable commit in production metadata.
8. Remove any duplicated direct LDC protocol when the adapter is qualified.

The adapter should be small. If it contains register constants, DATA bit
assembly, ERROR_CONFIG masks, scan sequencing, or status-clear workarounds,
the reusable library refactor is incomplete.

## Release acceptance checklist

### Product gate

- [ ] LDC hardware role and board revision are approved.
- [ ] Variant, address, clock, channels, coils, and target are documented.
- [ ] Units, calibration, cadence, deadline, freshness, and health role are
      approved.
- [ ] Sample, status, settings, replay, cloud, and capacity impacts are
      reviewed.

### Library gate

- [ ] Zero-I2C bind exists.
- [ ] Initialize, acquire, apply, and reset/reapply are cooperative jobs.
- [ ] Every job is bus-silently cancellable.
- [ ] STATUS/DATA/INTB side effects are correct and tested.
- [ ] Complete batch commit is atomic.
- [ ] Readiness/error policy and sample quality are explicit.
- [ ] Applied state survives hotplug and brownout policy correctly.
- [ ] Bus recovery, backoff, and authoritative health remain outside the
      library.
- [ ] Clock/frequency/timing helpers use checked, precise units.
- [ ] Clean consumer and exact-version metadata pass.

### TunnelMonitor gate

- [ ] Dependency is pinned to a full reviewed commit SHA.
- [ ] Only `I2cTask` invokes transport.
- [ ] One normal callback per poll is proven.
- [ ] Owner deadline and cancellation are proven.
- [ ] No third-party type leaks into contracts.
- [ ] Fixed-capacity native tests pass.
- [ ] Exact-board sensor-attached HIL and bounded soak pass.

## Final recommendation

Use LDC1614 v2.0.0 as the refactor base, not as an unchanged production
dependency. Its protocol definitions, framework boundary, fixed memory,
per-channel read ordering, dirty-state work, and initial poll engine are worth
keeping.

Do not platformize the current split synchronous/cooperative API. Complete the
job model around cold initialization, cancellation, status-aware atomic
acquisition, external recovery ownership, and explicit sample quality. In
parallel, decide what LDC1614 means on TunnelMonitor hardware. Once both are
done, an exact-pinned private adapter can reduce firmware chip protocol and
field risk without weakening the existing I2C architecture.
