# LDC1614 driver library

Framework-neutral, fixed-memory LDC1612/LDC1614 28-bit inductance-to-digital
converter driver for externally owned I2C buses. The v3 API is cooperative:
multi-register procedures execute only when the application calls `poll()` and
never exceed its transfer budget.

This tree is version **3.1.0**. Deployment still requires evidence for the exact
board, address strap, reference clock, LC sensors, channel mapping, INTB/SD
wiring, fault policy, calibration, cadence, and soak conditions. See the
maintained validation status and HIL guide before selecting a release.

## Core contract

- The application owns the I2C bus, pins, locking, scheduling, absolute
  operation deadline, per-transfer timeout cap, retries, device presence,
  health policy, backoff, and shared-bus recovery.
- `Config` injects non-owning write and combined write/read callbacks. The core
  includes no Arduino, ESP-IDF, FreeRTOS, logging, global bus, delay, allocation,
  or hidden retry path.
- `bind()` validates and retains an explicit desired profile with zero I2C.
  A default-constructed profile is invalid by design.
- One job may be active. Each `poll(nowMs, maxTransfers)` call invokes no more
  than the supplied number of transport callbacks; zero is bus-silent.
- Each start carries a caller-selected nonzero `OperationId` and absolute
  64-bit deadline. `nowMs` and deadlines share one owner-supplied timeline;
  natural `uint64_t` wrap is safe for deadline horizons shorter than 2^63 ms.
  Extend a wrapping 32-bit clock before use. A terminal
  `OperationResult` retains identity, outcome,
  status, side-effect flags, configuration revision, and fault provenance.
- The fixed two-entry result FIFO delivers each terminal result exactly once
  through `takeResult()`. The owner must drain results; starts fail explicitly
  when result capacity is reserved or full.
- `cancelJob()` is idempotent and bus-silent. It discards partial acquisition
  scratch, retains the previous complete batch, records possible write effects,
  and permits a replacement job while the cancelled result awaits collection.
- Transport counters are non-authoritative diagnostics. Failures never latch
  the library offline or suppress a later owner request.
- Instances are neither internally thread-safe nor ISR-safe. Serialize every
  call. Transport/INTB callbacks must not re-enter the same instance. An ISR
  may notify the owner, but must not call the driver.

## Installation

For PlatformIO:

```ini
lib_deps =
  LDC1614
```

The repository root is also an ESP-IDF component. Add it through
`EXTRA_COMPONENT_DIRS` or component-manager metadata. The native diagnostic
example is under `examples/esp_idf/basic`.

As an ESP-IDF component the library requires ESP-IDF 6.0 or newer;
`idf_component.yml` declares `idf: ">=6.0.0"`. The driver core compiles no
ESP-IDF API, so that floor records the maintained example and CI surface
rather than a source dependency. The manifest declares no target restriction
because the core is chip-independent; maintained CI builds the example for
ESP32-S2 and ESP32-S3 with the IDF version pinned in the workflow.

Host validation tools are pinned in `requirements-dev.txt`; the maintained
Arduino build uses pioarduino `55.03.311` (Arduino 3.3.11 with ESP-IDF 5.5.5),
pinned in `platformio.ini`. In this ESP-IDF generation, a synchronous
transaction that does not reach the driver's internal `DONE` state is returned
as `ESP_ERR_INVALID_STATE` (`259`), including an ordinary slave NACK. The
example transport therefore preserves `259` as raw detail but maps it to a
generic transaction failure, never by itself to a failed shared bus. ESP-IDF 6
renames the NACK result to `ESP_ERR_INVALID_RESPONSE`; see the
[official migration note](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/migration-guides/release-6.x/6.0/peripherals.html#i2c-master-driver-updates).

Both maintained ESP32 diagnostics share one example-owned ESP-IDF new-master
transport rather than a parallel Wire backend. Its explicit `busrecover`
boundary is example/build-tool policy, not a dependency of the
framework-neutral core; see [I2C owner integration](docs/I2C_INTEGRATION.md)
for the controller-reconstruction contract and why an address ACK is not
device admission.

## Explicit profile

The following values are illustrative only. Derive the clock, frequency
bounds, timing counts, dividers, deglitch bandwidth, and drive current from the
actual target.

```cpp
LDC1614::Config makeProfile(void* busContext) {
  LDC1614::Config cfg{};
  cfg.i2cWrite = appI2cWrite;
  cfg.i2cWriteRead = appI2cWriteRead;
  cfg.i2cUser = busContext;
  cfg.i2cTimeoutMs = 20;
  cfg.i2cAddress = LDC1614::I2cAddress::ADDR_GND;

  cfg.variant = LDC1614::DeviceVariant::LDC1614;
  cfg.channels = LDC1614::channelBit(LDC1614::Channel::CH0);
  cfg.referenceClock = {
      LDC1614::RefClkSrc::INTERNAL, 43000000U, 200000U};
  cfg.mode = LDC1614::OperatingMode::SINGLE_CHANNEL;
  cfg.activeChannel = LDC1614::Channel::CH0;
  cfg.deglitch = LDC1614::Deglitch::BW_10MHZ;
  cfg.sensorActivation = LDC1614::SensorActivation::FULL_CURRENT;
  cfg.rpOverrideEnabled = true;
  cfg.autoAmplitudeCorrectionEnabled = false;
  cfg.intbDisabled = true;
  cfg.errorReporting = LDC1614::ErrorReporting::all();

  // Initialization writes every physical variant channel to a known value.
  for (uint8_t channel = 0; channel < 4; ++channel) {
    auto& profile = cfg.channel[channel];
    profile.rcount = 0x04D6;
    profile.settleCount = 0x000A;
    profile.finDivider = 2;
    profile.frefDivider = 2;
    profile.offset = 0;
    profile.driveCurrentCode = 10;
  }
  cfg.channel[0].expectedSensorMinHz = 100000;
  cfg.channel[0].expectedSensorMaxHz = 5000000;
  return cfg;
}
```

`DeviceVariant` is a hardware fact: LDC1612 and LDC1614 expose the same checked
identity values, so software cannot discover the variant. `I2cAddress`, the
selected `ChannelMask` and clock frequency/tolerance are also explicit.
Initialization writes a known register profile for every physical channel of
the selected variant, so those register values must all be supplied; expected
sensor-frequency bounds are required for channels selected for conversion.
Validation uses both reference-clock extrema for fREF limits and the
fIN < fREF/4 rule, and requires deglitch bandwidth to be strictly above the
maximum expected sensor frequency. External-clock extrema come from the
declared nominal frequency and tolerance. Internal-clock limit and timing
checks always use the guaranteed 35 MHz to 55 MHz oscillator range; its nominal
value is used only for sensor-frequency conversion. OFFSET must remain below
the selected channel's worst-case minimum-sensor-frequency ratio so it cannot
mask changing result bits.

## Owner loop

```cpp
LDC1614::LDC1614 device;

// Startup in the application's I2C-owner context.
LDC1614::Status st = device.bind(makeProfile(&bus));    // zero I2C
if (st.ok()) {
  st = device.startInitialize(1001, nowMs + 2000);      // schedules only
}

// On each owner-task service pass:
if (device.jobProgress().active) {
  st = device.poll(nowMs, 1);                           // at most one transfer
}

LDC1614::OperationResult result;
while (device.resultAvailable()) {
  if (!device.takeResult(result).ok()) {
    break;
  }
  publishToMatchingRequest(result.operationId, result); // exactly once
}
```

The illustrative result-drain loop is bounded by `RESULT_CAPACITY == 2`.
Production code should use its existing fixed request identity and result
reservation rather than inventing a second queue around the driver.

At or after the absolute deadline, `poll()` completes the job as timed out
without issuing another transfer. Before work starts, the poll divides the
remaining deadline budget across the callbacks it may invoke; every callback
is also capped by `Config::i2cTimeoutMs`, so their worst-case timeout sum cannot
exceed the remaining time observed at that poll boundary. Time does not
advance inside the library and there are no sleeps or yields.

## Operation classes and bounds

One instruction is one physical transport callback.

| Class | API | Maximum callbacks | Scheduling contract |
| --- | --- | ---: | --- |
| Steady state | `readDeviceStatus`, `readDataReady`, `sleep`, `wake`, `readInitDriveCurrent`, raw register read/write | 0 or 1 per call | Bounded by the injected callback timeout; no retry. INTB observation may be bus-silent. |
| Acquisition | `startAcquire` | `2 + 2N`, up to 10 for four channels | STATUS before, MSB/LSB per selected channel, STATUS after; caller budgets each poll. |
| Configuration apply | `startApplyConfig` | 13 for LDC1612; 23 for LDC1614 | Multi-poll write procedure; failure/cancel may leave partial or indeterminate hardware state. |
| Initialization | `startInitialize` | 15 for LDC1612; 25 for LDC1614 | Two identity reads plus complete configuration replay. |
| Chip reset/reapply | `startResetAndReapply` | 16 for LDC1612; 26 for LDC1614 | One software-reset write plus the complete initialization procedure. |

TI specifies the software-reset command but no software-reset recovery
interval. The core therefore does not invent a delay or retry. An owner that
needs a scheduling boundary may service reset/reapply with transfer budget one;
a confirmed address NACK on the reset write retains the prior applied state
because the device did not accept the transaction. Once reset reached or may
have reached the device, a later failure terminates with the exact transport
status and unknown applied-state evidence.

The LDC1612/LDC1614 has no library-managed NVM programming or calibration
storage procedure. Commissioning/calibration remains application work. Raw
diagnostic writes are single-transfer advanced operations, not a maintenance
framework and never receive blind retries.

## Acquisition integrity

`startAcquire(mask, id, deadline)` is the production multi-channel read path.
It uses fixed private scratch and publishes a new `SampleBatch` only after the
complete requested protocol terminates successfully. A failed or cancelled job
does not expose a half batch or update the last complete publication.

Every result includes:

- terminal phase, register, channel, completed/maximum transfer counts, and
  requested/completed channel masks in `finalProgress`;
- immutable operation identity, kind, outcome, full status/effect provenance,
  configuration revision, and the owner timestamp supplied at the terminal
  `poll()` boundary (zero for bus-silent `cancelJob()`);

Successful acquisition results additionally include:

- selected, valid, fresh, error, and overrun channel masks;
- the pre-DATA and post-DATA STATUS snapshots;
- raw 28-bit count, raw register words, and silicon-level quality flags per
  selected channel;
- owner-supplied terminal poll-boundary time and the applied configuration
  revision. Timestamp after `poll()` returns if wall-clock completion time is
  required by the application.

The device has destructive read behavior:

- reading `DATAx_MSB` latches that channel's LSB shadow, consumes
  `UNREADCONVx`, and may clear the channel's latched STATUS/error/INTB evidence;
- reading STATUS captures then clears sticky status and can deassert INTB; and
- a new conversion can overwrite unread data while a chunked read is active.

For that reason acquisition always reads STATUS before DATA, preserves both
STATUS snapshots, and reports detected overrun/data-loss evidence. DATAx is
read MSB before LSB, so an overrun observed afterward means a newer conversion
is pending; it does not invalidate the coherent pair already captured. An
application may still reject overruns when its cadence policy requires every
conversion. Fresh under-range zero and over-range `0x0FFFFFFF` are classified
even when corresponding device reporting bits are disabled; stale zero is not
misreported as a range fault. When routed and observed, watchdog, amplitude,
zero-count, stale, and data-loss conditions remain visible separately from
transport success. `validChannels` means fresh with no decoded range/silicon
error, not proof that every silicon fault route was enabled; overrun/data-loss
is orthogonal and can coexist. Disabled `ERROR_CONFIG` routes can hide evidence,
and zero-count can latch after STATUS-before then be cleared by the matching
DATAx_MSB read. `errorChannels` and per-channel quality retain every cause the
driver can observe. Configuration
trust is reported by `AppliedConfigState`; acquisition is rejected before I2C
unless that state is `APPLIED_ACTIVE`.

LDC multi-channel conversion is sequential. Atomic batch publication means the
software result is committed together; it does not mean channels were sampled
simultaneously or prove a single-instant physical frame.

## Applied configuration and recovery

`AppliedConfigState` distinguishes unknown, applying, applied-sleeping,
applied-active, and dirty state. Normal acquisition is rejected unless the
desired configuration is trusted as applied and active.

After application-observed removal, reset, brownout, SD shutdown, device power
loss, or shared-bus recovery, call:

```cpp
device.invalidateAppliedState(reason);                 // zero I2C
while (device.resultAvailable()) {
  LDC1614::OperationResult abandoned;
  if (!device.takeResult(abandoned).ok()) {
    break;
  }
  publishToMatchingRequest(abandoned.operationId, abandoned);
}
LDC1614::Status st =
    device.startInitialize(newId, absoluteDeadlineMs);  // full identity + replay
if (!st.ok()) {
  reportAdmissionFailure(st);
}
```

The application decides if and when to retry. The driver does not reset the
bus, toggle application GPIOs, apply backoff, or declare a device offline.
Matching identity after return is not treated as proof that configuration
survived; initialization replays the complete desired profile.

Entering sleep is destructive to conversion evidence: DATA values, unread and
error status, and INTB assertion are no longer available. Wake and acquire only
after the application has accepted that boundary.

Do not infer a stuck shared bus from ESP-IDF 5.5.x raw detail 259 or retry only
the failed register. If controller reconstruction and idle-high lines do not
restore complete identity reads, leave the LDC unavailable. A product with
owner-controlled SD or isolated switched power may perform one bounded
device-local reset, wait at least 2 ms after SD is released, and then run full
initialization. Without that hardware, a true LDC rail cycle is the safe
recovery boundary.

Also qualify an MCU-only reset while the LDC rail remains powered. On COM8, an
LDC that was readable before an ESP32-S2 upload/reboot refused the first
combined read afterward and recovered only after a true rail cycle. The exact
electrical edge was not captured, so this is a proven transition interval, not
proof of a GPIO or silicon cause. A defined SD shutdown default through MCU
boot, followed by the data-sheet wake interval and one full initialization, is
the preferred device-local design when the hardware can provide it.

A configuration write that could have reached hardware records full
`ConfigFault` provenance: original `Status`, job and phase, register, channel,
and confirmed-partial or indeterminate effect flags. `updateDesiredConfig()` is
bus-silent and increments the desired revision; a later apply/reinitialize is
required. Advanced raw writes invalidate the high-level applied-state contract.

## Pure helpers and diagnostics

- `validateConfig()` applies the complete bind/update validation contract with
  zero I2C and without retaining the candidate.
- `expectedConfigurationRegister()` returns the exact replay value plus a
  stable readback mask for each persistent register in the selected variant.
  The mask includes documented mandatory R/W constants and excludes only
  read-only INIT_IDRIVE plus the runtime sleep bit. Readback comparison is
  diagnostic and does not change applied trust state.
- `calculateSensorFrequencyHz()` returns `Status` plus `double`, using the
  explicit reference clock, channel dividers, offset, and raw count.
- `estimateFrameTiming()` returns conservative device timing and acquisition
  transfer count. Multi-channel device time includes the complete configured
  auto-scan sequence even when the requested readout mask is a subset;
  acquisition transfer count follows the requested mask. Application queueing,
  lock wait, I2C callback duration, and processing time remain outside this
  chip estimate.
- `encodeErrorReporting()`, `nominalDriveCurrentMicroamps()`,
  `decodeDeviceStatus()`, and `decodeChannelSample()` are bus-silent pure
  helpers.
- `readRegister16()` and `writeRegister16()` are advanced diagnostic access.
  Prefer typed jobs/controls for normal use and reconcile any raw mutation by
  invalidation plus replay.
- `readIntb()` reports the application-observed INTB level through the optional
  bus-silent `Config::intbAsserted` callback. It performs no I2C and returns
  `INVALID_CONFIG` when no INTB observer is enabled.
- `TransportStats` records attempts, successes, failures, and the last status
  for diagnostics only; it does not own health or admission policy.

## Diagnostic CLI

The maintained Arduino and native ESP-IDF examples expose the same fixed-memory
diagnostic surface through independent framework-local implementations. A
host-only manifest checks their command tables, aligned 32-column ANSI help,
safety confirmations, stable evidence records, and key output fields for exact
parity.

Commands cover lifecycle/jobs, complete desired configuration, acquisition and
cached batches, STATUS/INTB/SD visibility, every persistent configuration
register, owner bus/speed and transport-counter diagnostics, pure
timing/frequency/current/decoder helpers, and bounded protocol-qualified
discovery, verify, self-test, watch, acquisition/mixed/identity/reset/bus-speed
stress, sample-rate, and soak sessions.
`cfg` prints every global field, every physical-channel register value and
sensor bound, error routing, desired/applied revision, INTB availability, and
configuration-fault provenance.

Configuration editing is deliberately staged. Field commands copy and modify a
fixed candidate with zero I2C; `profile validate` checks the whole candidate and
`profile commit confirm` updates desired state only. The application must then
run the cooperative `apply` job. Address and variant remain physical binding
facts and require `end()` plus a rebuilt/rebound transport profile.

Discovery, multi-register diagnostics, self-test, and stress functions are
finite CLI-owned state machines. Each owner service pass performs at most one
`poll(now, 1)` callback or one direct diagnostic callback. Raw writes,
destructive all-register dumps, reset/reapply, recovery, invalidation,
mixed-stress, and example-owned SD transitions require explicit confirmation.
These tools aid bring-up; protocol stress on a no-sensor board is not evidence
of sensor accuracy, production cadence, coil suitability, or physical INTB/SD
behavior.

## Migration from v2

v3 is a deliberate breaking release.

| v2 | v3 |
| --- | --- |
| `begin(config)` | `bind(config)` then `startInitialize(id, deadline)` / `poll()` / `takeResult()` |
| `tick()` plus separate staged APIs | one `poll(nowMs, maxTransfers)` job engine; no `tick()` |
| `readChannel`, `readAllChannels`, `readFreshChannels`, `startReadChannels` | `startAcquire(mask, id, deadline)` with one status-aware `SampleBatch` result |
| blocking channel reads and cached-sample age/accessors | owner-driven `startAcquire` / `poll` / `takeResult`; retain or timestamp the returned batch in application storage |
| `dataReady()` | `readDataReady(bool&, DeviceStatus&)`, which preserves the destructive STATUS snapshot |
| `readStatusRaw()` | diagnostic `readRegister16(REG_STATUS, value)`; prefer `readDeviceStatus()` when decoded fields are needed |
| `probe()` | diagnostic reads of `REG_MANUFACTURER_ID` and `REG_DEVICE_ID`; production admission uses `startInitialize()` |
| `syncConfig()` | `startApplyConfig(id, deadline)` |
| `resetAndReapply()` | `startResetAndReapply(id, deadline)` |
| `softReset()` | `startResetAndReapply()`; a raw RESET_DEV write is diagnostic and leaves applied state unknown |
| runtime `set*()` configuration calls | sleep, `updateDesiredConfig()`, then `startApplyConfig()`; transport/address/variant changes require `end()` and `bind()` |
| `readInitIdrive()` | `readInitDriveCurrent()` |
| `getSettings()` / `settings()` / `getConfig()` | `config()`, `appliedConfigState()`, `configRevision()`, and `jobProgress()` |
| `calcSensorFrequency()` | checked `calculateSensorFrequencyHz()` |
| separate settle/sample/conversion-time helpers | conservative `estimateFrameTiming()` and its fixed-unit fields |
| library `recover()`, OFFLINE latch, backoff, bus/hard-reset hooks | application recovery plus `invalidateAppliedState()` and explicit initialization/replay |
| raw `channelCount`, address, error mask, and optional clock facts | explicit typed variant, address, channel mask, clock, profile, and `ErrorReporting` |
| per-transfer health admission | non-authoritative `TransportStats`; owner retains health/admission authority |

## Examples and validation

- [Arduino diagnostic CLI](https://github.com/janhavelka/LDC1614/blob/main/examples/01_basic_bringup_cli/README.md): cooperative
  bring-up firmware with colored comprehensive help and a one-transfer owner
  service budget.
- [Native ESP-IDF diagnostic CLI](https://github.com/janhavelka/LDC1614/blob/main/examples/esp_idf/basic/README.md): fixed-buffer
  parity-checked example using the native I2C master driver.
- [I2C owner integration](docs/I2C_INTEGRATION.md): ownership, deadlines,
  results, side effects, and recovery.
- [Hardware integration](docs/HARDWARE_INTEGRATION.md): board, sensor, timing,
  and physical-evidence checklist.
- [HIL validation](docs/HIL_VALIDATION.md): target procedure and evidence rules.
- [Validation status](docs/VALIDATION_STATUS.md): repeatable release checks,
  retained evidence boundaries, and remaining physical gates.
- [Release procedure](https://github.com/janhavelka/LDC1614/blob/main/docs/RELEASING.md): exact maintainer
  validation, commit, annotated-tag, and GitHub publication sequence.
- [Documentation index](https://github.com/janhavelka/LDC1614/blob/main/docs/README.md): maintained guides, references, and
  archive boundaries.

Clean firmware `e4d0436` passed the retained 187-command no-reset matrix and a
one-hour ESP32-S2/LDC1614 no-sensor soak (2,926 cycles and 32,186 commands with
zero failures, unknowns, or resets). This is transport/lifecycle evidence, not
sensor-equipped or exact-release target acceptance; `RESET_DEV` also remains
unqualified. See [Validation status](docs/VALIDATION_STATUS.md) for the precise
boundary.

Generate the public API and maintained-guide site locally with:

```sh
doxygen Doxyfile
```

Open `docs/doxygen/html/index.html` after generation. The output directory is
ignored; edit the source Markdown or public headers, never generated HTML.

Do not infer target hardware suitability from a successful native test or
firmware build. See the validation documents before making a deployment claim.
