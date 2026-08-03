# TunnelMonitor Integration Gates

This page contains only the work that remains before TunnelMonitor may consume
this library. Reusable driver behavior is documented by the public API,
[I2C owner integration](I2C_INTEGRATION.md), and the changelog rather than
repeated here.

Last cross-repository review: 2026-08-03.

- LDC1614 checkout: unreleased `3.1.0` candidate. Clean COM8 firmware
  `c3e2ed876dd884ceefc46ac6e579effa83e45325` passed the current default and
  extended matrices plus a labeled one-hour steady-state soak. Reset/recovery
  remained intermittent, so replace this line with a reviewed release commit
  only after the platform recovery gate is closed.
- TunnelMonitor-node checkout:
  `f05cd296d59c62ad4dfe293ca63f9b3798053ce8` on
  `prompt-45-platformization`, clean and synchronized with its upstream.

At that review, TunnelMonitor had no LDC device kind, health identifier, I2C
operation, build-profile row, dependency, reading schema, settings,
calibration, or concrete module. Do not add those pieces until the product
contract below is approved. Its existing native suite passed 1,200/1,200 tests
and its ESP32-S3 production environment built at the reviewed checkout; those
software checks prove only that the proposed library work did not disturb the
current consumer, not that an absent LDC integration works.

## Product contract gate

TunnelMonitor must decide and document:

- whether an LDC is populated on hardware 2.0.0 or a later board revision;
- LDC1612 versus LDC1614, ADDR strap `0x2A`/`0x2B`, selected channels, and each
  channel's physical meaning;
- raw count, frequency, inductance, displacement, level, or another output;
- units, rounding, invalid-value policy, calibration version, and temperature
  compensation;
- LC values, sensor-frequency bounds, Q/Rp, target material/range, deglitch,
  counts/dividers, offset, and drive-current evidence;
- internal or external reference clock, measured frequency, and tolerance;
- acquisition cadence, immutable owner deadline, and transfer budget;
- required freshness, acceptable sequential mixed age, and data-loss policy;
- required, optional, or diagnostic health role and all capacity/schema
  effects;
- INTB and SD wiring, polarity, ownership, and behavior; and
- compile-time board profile versus durable operator configuration.

Existing RS485 VibWire frequency fields are not an LDC contract and must not be
reused as implicit LDC outputs.

## Release gate

TunnelMonitor's sibling-library policy requires an exact annotated
`vMAJOR.MINOR.PATCH` tag. The required deadline, identity/state, quality,
offset-validation, and transport corrections are newer than `v3.0.0`.

Create and review a new annotated minor release before adding the dependency.
Do not consume this branch, a moving commit, or a reinterpreted `v3.0.0` tag.

## Consumer platform gate

TunnelMonitor currently pins pioarduino `54.03.20`; this library's maintained
Arduino builds pin `55.03.311`. COM8 testing reproduced persistent post-NACK
`ESP_ERR_INVALID_STATE` on the latter as well, so neither pin is qualified as a
TunnelMonitor recovery fix. Treat `55.03.311` as a separate platform-migration
candidate and qualify it on the exact ESP32-S3 target across I2C, OTA, USB,
RMT, Wi-Fi, SD, and PSRAM before changing TunnelMonitor's active pin. Preserve
TunnelMonitor's existing failed-read bus-recreation fence until target fault
injection proves a simpler policy safe. A successful build alone does not
close this gate. The COM8 diagnostic's bus reset and target ACK also failed to
guarantee the following combined read, reinforcing that TunnelMonitor must
require module-specific replay/admission after its owner recovery.

## Owner-module gate

After the product contract is approved, add one owner-private concrete LDC
module called only by TunnelMonitor's `I2cTask`. The module must:

- retain the complete TunnelMonitor request identity and assign the LDC job a
  private nonzero operation ID;
- pass the original owner deadline unchanged to the LDC start operation;
- call `poll(nowMs, 1)` no more than once per module poll;
- immediately consume exactly one terminal result and verify its operation ID,
  kind, outcome, and deadline context before publication;
- route each LDC transport callback through
  `I2cOwnerTransport::transferOnce()` exactly once, with no library-side retry;
- use `cancelJob()` only for explicit withdrawal, shutdown, or replacement;
  deadline expiry must be observed by `poll()`;
- call `invalidateAppliedState()` on owner bus invalidation and require a
  complete initialize/replay before acquisition resumes; and
- after a serialized shared-bus reconstruction, recreate any owner-retained
  device handles and require each affected module's bounded device-specific
  admission before that module resumes combined reads or configuration replay;
  and
- keep queueing, retry, health, recovery, settings, calibration, and public DTO
  policy in TunnelMonitor.

If the owner cannot distinguish address-NACK from data-NACK, map a failed write
conservatively to data-NACK or generic I2C failure. After an indeterminate
write, terminate the job, perform owner recovery, invalidate applied state, and
schedule complete initialize/replay. Never retry only the failed register.

## Target HIL gate

TunnelMonitor currently uses an ESP32-S3 native ESP-IDF backend whose
per-transfer device-handle lifecycle differs from this repository's diagnostic
transport. Before integration, exercise that exact backend on the selected
board:

1. induce a controlled absent-address NACK;
2. perform repeated valid combined reads while retaining raw backend codes and
   proving that expected scan/probe NACKs did not poison the next transaction;
3. execute explicit owner controller recovery without rebooting the MCU;
4. invalidate LDC applied state and complete full initialization/replay; and
5. prove another shared-bus device remains responsive throughout recovery.

The library diagnostic can reconstruct its sole owned bus/device handle after
explicit `busrecover`. TunnelMonitor must not copy that teardown literally:
its shared owner must serialize recovery and recreate every handle registered
on that bus before admitting any device operation.

For the selected LDC board and sensors, also retain:

- variant, address, identity, reference-clock frequency/tolerance, populated
  channels, LC values, and complete configuration readback;
- live DATA MSB-before-LSB behavior and expected physical response;
- STATUS, UNREAD, error-channel, INTB, and delayed-read data-loss behavior;
- safe under/over-range, watchdog, amplitude, and zero-count cases;
- correlated deadline/cancellation followed by a clean replacement operation;
- removal or controlled power loss followed by owner recovery and full replay;
- INTB and SD evidence when those pins are wired; and
- a one-hour bounded soak at the production channel mask, cadence, and clock
  profile.

At least one raw serial transcript or logic-analyzer trace must accompany the
exact release fixture. Software tests, builds, dry runs, historical v2 results,
and retained negative transport reports do not satisfy this gate. Follow
[HIL validation](HIL_VALIDATION.md) and
[validation status](VALIDATION_STATUS.md).

## Completion

Remove this gate page after all product decisions are represented in
TunnelMonitor contracts, an exact dependency tag is selected, the private
module and owner tests pass, and the target HIL evidence is accepted.
