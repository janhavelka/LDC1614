# TunnelMonitor-node suitability audit

Date: 2026-07-19

Result: **the reusable library findings are addressed by the v3 cooperative
contract; TunnelMonitor integration remains blocked by missing product and
physical-hardware decisions.**

No TunnelMonitor-node source was changed. Its current authoritative contracts
contain no LDC1612/LDC1614 hardware, operation, health, measurement, settings,
or calibration decision. Adding an adapter or public fields now would invent
product intent.

## Audit basis

| Repository | Exact revision inspected | Working-tree basis |
| --- | --- | --- |
| LDC1614 baseline | `a09f492659d4eb8299dc9fa41524bf6014b62dd6` on `hardening/tunnelmonitor-suitability-reaudit` | Clean baseline before the v3 implementation. |
| LDC1614 v3.0.0 release | `d9983e08b4fa0cf717cf3c7686adab418508c174` | Remote branch and annotated `v3.0.0` tag target; tag object `0fed6ff0e1b5fc5c305f9a1135560229549f2438`. Published tags are immutable. |
| LDC1614 v3.0.1 reviewed implementation | `d06e39bd3067b2ab37f1c4395c48864f336d4647` | Clean corrective implementation after independent core, test, documentation, and final-integration review. Exact native, target-build, Doxygen, package, and guard results are recorded in `VALIDATION_STATUS.md`. |
| TunnelMonitor-node baseline | `0897f12c1a1369367747d1063936906005391580` on `develop` | Clean when the contract inspection and native/target validation began; left unchanged by this work. |
| TunnelMonitor-node final checkout | `b708f511964db6c51e949e99c67820476f00f9c7` on `docs/mb85rc-suitability-contract-facts` | Clean. Its tree `1e7ae125548addb34dea22a6c58a65d623f642e3` is identical to the inspected baseline tree, so the source/contracts validated here did not change when the checkout branch moved externally. |

The previous audit evaluated LDC1614 v2 and an older TunnelMonitor revision.
Old line numbers and proposed type spellings were not treated as authority. The
current public headers, implementation, fault tests, examples, maintained docs,
CI, every applicable `AGENTS.md`, and current TunnelMonitor ownership contracts
were re-read. The corrective re-audit added literal replay transcripts and
exact stage/fault tables rather than relying only on transfer counts or
non-sentinel provenance.

The v3.0.0 implementation review ended at
**546c2b59eadd0dd660db6e1674414e7effb938ac**; its annotated tag points to the
subsequent audit-metadata-only release commit above. Corrective re-audit changes
after that immutable tag use the v3.0.1 reviewed implementation recorded above,
never a moved tag. The annotated `v3.0.1` tag targets the subsequent
documentation-only release commit containing these final results. Its exact
remote tag object and peeled target are verified after publication and reported
in the release handoff because a commit cannot contain its own object ID.

Primary chip behavior was checked against the bundled TI LDC1612/LDC1614
datasheet, SNOSCY9A Revision A (March 2018), whose official URL, repository
retrieval date, and SHA-256 are recorded in `reference/README.md`.

## Current TunnelMonitor contract

TunnelMonitor's `I2cTask` is the only shared-bus owner. It owns request
admission, locking/serialization, absolute deadlines, callback timeout,
retries, device presence, health, backoff, bus reset/SCL recovery, and result
publication/lifetime. It has deadline expiry and result take/reclaim, but no
current public I2C withdrawal/cancellation API. Normal work advances one
backend/library callback per owner poll. Foreground identity is the tuple
`(requestId, submissionToken, device, operation)`; result reservations are
fixed and reclaimed exactly once.

The inspected authorities were:

- `docs/guidelines/i2c_peripherals.md` and
  `docs/guidelines/dependency_policy.md`;
- `include/TunnelMonitor/BoardPins.h`;
- `include/TunnelMonitor/contracts/FieldBus.h`, `Health.h`, `Capacities.h`, and
  `Sample.h`;
- `include/TunnelMonitor/i2c/I2cConfig.h` and
  `src/i2c/I2cDiagnostics.cpp`; and
- measurement/settings guidelines and current dependency metadata.

Current concrete facts:

- board profile `tunnelmonitor_s3_hw200` is ESP32-S3-N16R8 hardware `2.0.0`,
  with I2C on GPIO8/GPIO9;
- the five known I2C devices are OLED `0x3C`, RTC `0x51`, FRAM `0x50`, ENV
  `0x76`, and INA `0x41`; no LDC row or operation exists;
- device health uses all 16 fixed entries; service health uses 15 of 16;
- the `tm.v1.vw8_shzk16_env_power` sample schema has 37 numeric fields in a
  capacity of 48; its vibration-wire frequency fields are not an LDC contract;
- redacted settings status uses 49 of 51 fixed rows; and
- production dependencies must be private behind their owner and pinned to an
  exact immutable commit.

The v3 library fits that ownership shape: its operation ID can be derived from
or stored with the owner reservation, `poll(..., 1)` respects the normal
callback budget, `cancelJob()` is bus-silent, terminal results are fixed and
exactly once, and library transport statistics never take over health policy.
An eventual adapter must translate owner deadline expiry or a future withdrawal
path into `cancelJob()` and consume the correlated terminal result.

## Hard-finding disposition

| Finding | Current status | Resolution and evidence | Direct native coverage |
| --- | --- | --- | --- |
| H-01: no TunnelMonitor product contract | **Open external blocker** | Current TunnelMonitor still has no LDC board population, physical role, output schema, calibration, cadence, or health decision. No firmware change was made. | Repository/contract inspection; requires product and hardware evidence, not a library unit test. |
| H-02: synchronous cold initialization | **Resolved in v3** | `bind()` validates with zero I2C. `startInitialize()` performs identity and complete replay only through budgeted `poll()`: exactly 16 transfers for LDC1612 or 26 for LDC1614. | `bind_is_zero_i2c_and_validates_complete_explicit_profile`; `initialize_exact_transfer_counts_and_zero_one_large_budgets`; exhaustive initialize failure test. |
| H-03: no cancellation or progress | **Resolved in v3** | `cancelJob()` is idempotent and bus-silent, publishes a correlated `CANCELLED` terminal result, preserves possible write effects, and permits replacement. `jobProgress()` is cache-only. | `initialize_cancel_is_bus_silent_at_every_phase`; `cancelled_result_survives_immediate_replacement_and_cancel_is_idempotent`; `progress_is_cache_only_and_reports_exact_phase_and_budget`; apply/reset/acquire cancellation tables. |
| H-04: destructive DATA/STATUS/INTB effects denied | **Resolved in v3** | Acquisition always preserves STATUS before DATA, then reads MSB before LSB, then STATUS after. Public docs describe shadow latch, UNREAD/error/INTB clearing, and evidence loss. The behavioral fake models those effects. | `behavioral_fake_models_shadow_status_unread_and_intb`; `acquire_exact_budget_order_status_evidence_and_each_error_channel`. |
| H-05: default error policy disables readiness paths | **Resolved in v3** | Default `Config` is invalid; readiness/error policy is explicit typed `ErrorReporting`. The checked helper rejects an unusable readiness configuration instead of waiting. | `bind_is_zero_i2c_and_validates_complete_explicit_profile`; `pure_error_status_frequency_and_timing_helpers_cover_boundaries`; owner-safe operation tests. |
| H-06: partial/non-coherent multi-channel publication | **Resolved at the chip-library boundary** | Fixed scratch is committed only as one complete `SampleBatch`; prior publication survives failure/cancel. Selected/valid/fresh/error/overrun masks and both STATUS snapshots are retained. Sequential channels remain non-simultaneous and TunnelMonitor must decide if mixed-age results are acceptable. | `acquire_failure_at_every_phase_preserves_prior_complete_publication`; `acquire_cancel_every_phase_is_silent_atomic_and_restartable`; overrun/shadow test. |
| H-07: driver health/recovery conflicts with owner | **Resolved in v3** | OFFLINE admission, internal backoff, retries, bus reset, and hard-reset callbacks were removed. `TransportStats` is diagnostic only. Owner recovery calls bus-silent `invalidateAppliedState()` then explicitly schedules full replay. | `transport_stats_are_diagnostic_only_and_failures_never_suppress_requests`; `dirty_unknown_invalidation_rejects_acquire_and_matching_return_replays_all`; lifecycle test. |
| H-08: decoded `valid` can hide unusable conversion | **Resolved in v3** | Transport outcome, trusted `AppliedConfigState`, and `SampleQualityFlags` are separate. Acquisition is rejected before I2C when configuration is unknown/dirty. For admitted acquisition, raw endpoints, watchdog, amplitude, STATUS zero-count, stale, and data-loss evidence remain explicit; the application may apply stricter field policy. | `sample_quality_endpoints_watchdog_amplitude_and_status_zero_count`; invalidation/replay and acquisition status/error-channel tests. |
| H-09: reads allowed with untrusted configuration | **Resolved in v3** | `AppliedConfigState` is explicit. Acquisition rejects unknown/dirty/not-active configuration. `ConfigFault` retains full cause/phase/register/channel/effects and clears only after complete replay. | `dirty_unknown_invalidation_rejects_acquire_and_matching_return_replays_all`; config-fault and ambiguous-write tests; diagnostic-dirty test. |
| H-10: clock/frequency/timing facts ambiguous | **Resolved for reusable chip calculations; Tunnel product choice open** | `ReferenceClock` is validated configuration. Frequency returns `Status` plus `double`; frame timing uses conservative integer units and explicit transfer count. Host scheduling and physical calibration remain outside the helper. | `pure_error_status_frequency_and_timing_helpers_cover_boundaries`. |
| H-11: variant/electrical assumptions appear implicit | **Resolved in v3; physical values still external** | Explicit `DeviceVariant`, `I2cAddress`, `Channel`, `ChannelMask`, reference clock, known register values for every physical variant channel, and expected sensor ranges for selected channels replace inferred/default facts. Default config cannot bind. The common device ID is not presented as variant detection. | explicit-profile bind/validation test; pure helper boundaries; lifecycle/rebind test. |
| H-12: conflicting synchronous/cooperative models | **Resolved in v3** | `AGENTS.md`, headers, README, integration docs, Arduino example, and native IDF example now specify one zero-I2C start / budgeted poll / exactly-once result model. `tick()`, blocking lifecycle, synchronous recovery, and split staged-read API were removed. | compile/source guards plus all job budget/lifecycle tests. |

## Architecture selected

The library exposes one coherent fixed-memory engine:

```text
bind/update desired (zero I2C)
        |
start job(operation ID, absolute deadline) (zero I2C)
        |
owner poll(now, transfer budget)
        |
one terminal record in fixed FIFO
        |
takeResult() exactly once (zero I2C)
```

Jobs are initialization, configuration apply, software reset/reapply, and
status-aware acquisition. One-transfer controls and advanced register access
remain available for callers whose owner has admitted one bounded transaction.
There are no synchronous multi-transfer convenience paths in the production
core.

The two-entry terminal FIFO is intentional, not a general queue: it retains a
cancelled result while one replacement operation runs. Starts fail rather than
overwrite a reserved terminal record. IDs are rejected while active or queued,
preventing stale completion from being published for a later request.

### Bounded operation classes

| Class | Procedures and limits |
| --- | --- |
| Steady state | STATUS/readiness, sleep/wake, init-drive and raw diagnostics use no more than one callback. |
| Multi-step runtime | Acquisition uses `2 + 2N` callbacks (maximum 10); apply uses 14/24; initialize 16/26; reset/reapply 17/27 for LDC1612/LDC1614. Every job has caller deadline, transfer budget, progress, cancellation, terminal outcome, and effect provenance. |
| Rare/one-time | The chip has no library-managed NVM/calibration-storage procedure. Commissioning is application policy. Diagnostic writes are single-transfer, never blindly retried, and make unverified hardware state visible. |

On first failure a job stops. A write failure may be indeterminate because the
transport cannot prove whether silicon accepted it; the result preserves that
fact and the driver never performs a blind retry. Deadline evaluation with
budget zero is bus-silent. Callback timeout is capped by remaining deadline.

## Remaining TunnelMonitor product decisions

All of these are external to a general-purpose chip library:

- whether an LDC is populated on hardware 2.0.0 or a later revision;
- LDC1612 versus LDC1614, ADDR strap `0x2A`/`0x2B`, selected channel map, and
  each channel's physical meaning;
- raw count, frequency, inductance, displacement, level, or other output;
- units, rounding, invalid-value policy, calibration version, and temperature
  compensation;
- LC values, sensor-frequency range, Q/Rp, target material/range, deglitch,
  counts/dividers, and drive-current evidence;
- internal/external reference clock, measured frequency, and tolerance;
- acquisition cadence, immutable owner deadline, and allowed transfer budget;
- required freshness, acceptable sequential mixed age, and data-loss policy;
- required/optional/diagnostic health role and the capacity/schema effects;
- INTB and SD wiring, polarity, and owner-controlled GPIOs; and
- compile-time board profile versus durable operator configuration.

Until those are approved, do not add a TunnelMonitor dependency, device row,
operation, health entry, settings, or sample fields. Do not reuse the existing
RS485 VibWire frequency fields as implicit LDC outputs.

## P2 release/maintenance disposition

| Item | Status |
| --- | --- |
| Exact-pin PlatformIO and maintained Espressif platform | **Closed:** `requirements-dev.txt` pins PlatformIO 6.1.18; `platformio.ini` pins pioarduino platform-espressif32 release 54.03.20. CI installs the requirements file. |
| Pin serial-HIL host dependency | **Closed:** `pyserial==3.5` is recorded with the host tools. |
| Honest `native_cov` wording | **Closed:** it is documented as instrumentation only; no report or threshold is claimed. |
| Primary datasheet provenance | **Closed:** vendor, title, SNOSCY9A Revision A, official URL, repository date, and SHA-256 are indexed. |
| Annotated release tag | **Closed:** v3.0.0 tag object `0fed6ff0e1b5fc5c305f9a1135560229549f2438` resolves to `d9983e08b4fa0cf717cf3c7686adab418508c174`; corrective v3.0.1 uses its own annotated tag on the final documentation-only descendant of reviewed implementation `d06e39b`. Published tags are immutable and downstream still pins a full commit SHA. |
| Raw physical transcript/logic trace | **Open physical gate:** no raw v3 target artifact is committed. Historical compact v2 chip-only summaries are insufficient. |

## Validation and remaining physical gates

The v3 native suite contains direct normal, boundary, and exhaustive
stage-failure/cancellation coverage for binding, initialization, apply,
reset/reapply, acquisition, deadline and callback caps, result identity/FIFO,
behavioral DATA/STATUS/INTB side effects, atomic publication, data loss and
quality, applied-state invalidation/replay, transport statistics, and pure
frequency/timing helpers. On 2026-07-19 both `native` and the
coverage-instrumented `native_cov` environments passed 29/29 tests. Exact build
results are maintained in `VALIDATION_STATUS.md`; no CI run or hardware result
is inferred here.

Before a TunnelMonitor integration or field decision, capture on the exact
ESP32-S3 board and selected sensors:

- variant, address, both identities, clock frequency/tolerance, populated
  channels, LC values, and complete configuration readback;
- expected physical response with DATA MSB/LSB order under live conversion;
- STATUS/UNREAD/error-channel/INTB side effects and delayed-read data loss;
- under/over-range, watchdog, amplitude, and zero-count conditions where safe;
- owner deadline and bus-silent cancellation followed by a clean new ID/result;
- removal or controlled power loss, owner bus recovery, invalidation, and full
  replay while another shared-bus device remains responsive;
- INTB/SD behavior only if wired; and
- bounded soak at production channel mask, cadence, and clock profile.

At least one raw serial transcript or logic-analyzer trace must accompany the
exact release fixture. Software tests, builds, dry runs, and compact historical
reports do not satisfy that gate.

## Final recommendation

Use the exact reviewed patch revision, not a moving branch. The reusable driver no
longer requires an application-side state machine to compensate for blocking
initialization, missing cancellation, destructive evidence loss, partial batch
publication, or driver-owned recovery policy.

Do not integrate it into TunnelMonitor until H-01's product decisions and the
selected-board HIL gates are complete. When they are, add one small private
adapter inside `I2cTask`; keep queueing, deadline, retry, health, recovery, and
public DTO policy in TunnelMonitor, and keep all LDC register protocol inside
this library. The LDC transport callback must be exactly one physical attempt:
it must bypass TunnelMonitor's generic hidden per-transfer retry/recovery path.
After an indeterminate LDC write, terminate the job, invalidate applied state
after owner recovery, and schedule a new complete initialize/replay job; never
resend only the failed register write.
