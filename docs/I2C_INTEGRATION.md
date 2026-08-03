# I2C owner integration contract

The driver owns the LDC1612/LDC1614 register protocol, fixed job scratch, and
result provenance. The application owns the shared bus and every scheduling or
recovery decision.

## Transport boundary

- Inject non-owning `Config::i2cWrite` and `Config::i2cWriteRead` callbacks.
  Each invocation is one complete physical transaction and must return within
  the supplied timeout.
- Map framework errors to `Status`; retain backend detail in `Status::detail`.
  Do not leak `Wire` or `esp_err_t` types into the library contract.
- The application configures and owns pins, bus lifecycle, pull-ups, locks,
  queue admission, transaction timeout cap, retries, presence, health,
  backoff, GPIO reset/SD, and bus/SCL recovery.
- Serialize all calls to an instance. The driver is not thread-safe and no API
  is ISR-safe. INTB may wake/notify the owner; driver calls stay in owner/task
  context. Injected transport and INTB callbacks must not re-enter the same
  instance.

The driver never calls a transport callback from `bind()`, `updateDesiredConfig()`,
`cancelJob()`, `jobProgress()`, `takeResult()`, `invalidateAppliedState()`, or
`end()`. `validateConfig()` and `expectedConfigurationRegister()` are also
bus-silent pure helpers. A successful masked register comparison is diagnostic
evidence only; it does not authorize the helper or CLI to mark hardware
configuration trusted. Only a complete successful driver replay establishes an
applied state.

TI specifies the software-reset command but no software-reset recovery
interval. The core therefore neither guesses one nor retries a reset-adjacent
failure. Use `poll(nowMs, 1)` when the owner needs a scheduling boundary between
the reset write and the next identity read. A NACK or backend failure terminates
the job, preserves reset/write provenance, and leaves applied state dirty; the
application then owns any bus/device recovery and complete reinitialization.

The maintained ESP32 diagnostic demonstrates one explicit recovery policy for
its sole owned device handle: remove the device, delete and recreate the bus,
recreate the handle, then require one bounded target-address ACK probe. The
probe is owner recovery evidence, not a register retry; on the pinned ESP-IDF
5.5.5 baseline it also normalizes the new-master terminal state after the
reproduced NACK failure and before the required initialization replay. This is
not library behavior and is not a general shared-bus recipe. A production owner
must serialize the operation,
rebuild any owner-retained device handles, require bounded device-specific
admission before each affected module resumes, invalidate each affected
driver's applied state, and surface any failed recovery phase without a hidden
transaction retry. Not every I2C device supports address-only probing, so peer
admission is a product/device contract rather than a universal probe sweep.

## Request lifecycle

1. Build and `bind()` one explicit desired profile. Binding performs zero I2C.
2. Reserve the application's request/result identity and choose a nonzero
   `OperationId` plus immutable absolute 64-bit deadline. Deadlines and
   `poll(nowMs, ...)` use one owner timeline. Natural `uint64_t` wrap is safe
   for deadline horizons shorter than 2^63 ms; extend a wrapping 32-bit
   millisecond clock to 64 bits before passing it to the driver.
3. Start one job. Starting validates and reserves state but performs zero I2C.
4. On each owner service pass, call `poll(nowMs, budget)`. A normal shared-bus
   policy can use budget one. A larger explicit budget is allowed for startup
   or maintenance scheduling.
5. Observe cache-only `JobProgress` or cancel bus-silently when the owner
   request expires or is withdrawn.
6. Consume each terminal `OperationResult` exactly once with `takeResult()` and
   correlate it to the application request before publication.

`OperationId == 0` is invalid. IDs in the active job or fixed terminal FIFO
cannot be reused. The FIFO has two entries so a cancelled terminal record can
remain available while one replacement job runs. Starts report
`RESULT_QUEUE_FULL` or `DUPLICATE_OPERATION_ID` instead of overwriting evidence.

`poll(now, 0)` performs no I2C but still applies deadline state. At or after the
deadline, it emits `TIMED_OUT` without beginning another transfer. For a
transfer that may start, the poll shares remaining whole-operation time across
its admitted callbacks and also applies the configured per-callback cap. Their
worst-case timeout sum cannot exceed the remaining time observed at the poll
boundary. The driver never sleeps, yields,
retries, or advances its own clock.

## Operation classes

| Class | Bound |
| --- | --- |
| One-transfer steady state | STATUS/readiness, sleep/wake, init-drive read, raw read/write: no more than one callback. |
| Acquisition | STATUS-before + MSB/LSB per selected channel + STATUS-after: `2 + 2N`, maximum 10. |
| Apply | 14 callbacks for LDC1612 or 24 for LDC1614. |
| Initialize | Two identity reads plus apply: 16 or 26 callbacks. |
| Reset/reapply | Software-reset write plus initialize: 17 or 27 callbacks. |

There is no device NVM procedure in this library. Calibration/commissioning
storage and endurance policy are therefore outside its operation set. Raw
diagnostic writes are bounded single transactions and never retried blindly.

## Failure, cancellation, and hardware effects

Jobs stop on the first transport failure. `OperationResult::effects` records
whether at least one destructive read succeeded and whether configuration writes are known
partial or may have taken effect despite an error. `ConfigFault` retains the
full original status, job, exact phase, register, channel, and effect flags.

A failed callback cannot always prove whether a write reached the device. The
driver reports `INDETERMINATE_WRITE`, marks applied state dirty, and leaves the
retry/reconciliation decision to the owner. It does not repeat the write. A
confirmed `I2C_NACK_ADDR` is different: the addressed device did not accept the
transaction, so that attempt alone is not reported as an indeterminate device
mutation. Earlier successful writes in the same job remain partial effects.

Cancellation is idempotent and issues no callback. Cancelling acquisition
discards private scratch and leaves the previous complete batch untouched.
Cancelling after a possible configuration mutation records dirty/unknown
applied state. A new operation can start immediately when result capacity is
available; the cancelled operation's ID cannot be mistaken for the replacement.

## Applied state and recovery

Desired configuration is not asserted as hardware truth. The driver exposes
`UNKNOWN`, `APPLYING`, `APPLIED_SLEEPING`, `APPLIED_ACTIVE`, and `DIRTY`.
Acquisition requires a trusted active configuration.

When the application observes removal, power loss, SD assertion, brownout,
external chip reset, or shared-bus recovery, it calls
`invalidateAppliedState(reason)` with the original evidence. This is bus-silent.
After the device returns, run `startInitialize()` to check both identity
registers and replay every configured register. A matching identity alone is
not proof that configuration survived. If either identity transaction fails,
the driver makes applied configuration unknown and records the exact fault;
normal acquisition remains blocked until a complete initialization succeeds.

The library has no OFFLINE admission latch, retry count, recovery backoff,
bus-reset callback, or hard-reset callback. `TransportStats` is diagnostic
only and never suppresses an owner request.

## DATA, STATUS, and INTB effects

LDC reads are not observationally neutral:

- `DATAx_MSB` latches its LSB shadow, clears `UNREADCONVx`, and can clear the
  channel error that asserted STATUS/INTB;
- STATUS returns a snapshot, clears sticky status evidence, and can deassert
  INTB; and
- another conversion can overwrite an unread result during a chunked batch.

`startAcquire()` therefore owns the complete protocol: STATUS before DATA,
MSB then LSB for each channel, and STATUS after DATA. Its fixed `SampleBatch`
preserves both snapshots and selected/valid/fresh/error/overrun masks. Do not
split STATUS reads into an application adapter and DATA reads into the library.

Sequential autoscan channels are measured at different times. Complete-batch
commit prevents partial publication but does not create simultaneous sampling
or guarantee that a slow host read saw one same-instant sensor frame. Use the
timing estimate, masks, clock tolerance, and application cadence to decide
whether mixed-age sequential results meet the product contract.

## Package checks

Before publishing:

```sh
python scripts/generate_version.py check
python tools/check_clean_consumer_compile.py
```

The clean-consumer check packs the library in a temporary directory and compiles
against the extracted package. `Version.h` is generated from `library.json`;
never edit it directly.
