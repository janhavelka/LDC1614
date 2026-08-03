# Native ESP-IDF diagnostic CLI

This application is the independent native ESP-IDF counterpart of the Arduino
bring-up CLI. It uses `app_main`, `driver/i2c_master.h`, native GPIO/timer/task
APIs, a fixed input buffer, and a fixed-state command engine. It does not
include or compile Arduino or `examples/common/Ldc1614Cli.*`; repository
contract checks keep both implementations aligned.

The 64 command families cover the full public driver surface:

- identity, color, verbosity, and complete categorized help;
- bind/end, initialize/apply/reset jobs, sleep/wake, cancellation, result
  retention, applied-state invalidation, and explicit owner bus recovery;
- acquisition, cached batches, bounded watch/sample-rate sessions, readiness,
  decoded/raw STATUS, optional INTB, and INIT_IDRIVE diagnostics;
- a bus-silent staged profile for mode, clock, deglitch, activation, timeout,
  error routing, and every physical-channel register value and sensor bound;
- identity probe, bounded scan, register dumps, replay verification, guarded
  raw register access, STATUS/DATA decoding, frequency/timing helpers, and
  drive-current conversion;
- driver/state inspection, a multi-phase self-test, acquisition and mixed-I2C
  stress sessions, timed soak, and optional application-owned SD control.

Run `help` for the canonical grammar and aliases. Help is generated from a
fixed command table and matches the Arduino CLI's order, colors, grammar, and
descriptions. Repository contract checks compare its safety gates, fixture
requirements, execution classes, and evidence keys with the host manifest.
`color off` disables ANSI escapes without changing output fields.
Machine-parseable asynchronous output uses `CLI scheduled:`, `CLI preview:`,
and `CLI result:` envelopes.

## Ownership and nonblocking behavior

The example has one application owner task. One owner pass consumes a fixed
32-byte console budget, completes at most one command line, and invokes at most
one physical transport callback. A pass that handles a completed command does
not also service cooperative work. Core jobs advance through `poll(now, 1)`;
multi-step CLI sessions advance one direct transfer or one core poll per pass.
No CLI path retries a physical transfer, allocates heap state, waits for a
device operation, or hides an ambiguous hardware effect.

One `PromptAction` owns the terminal prompt. Immediate commands print one
prompt on return; asynchronous work defers it until the exact operation ID,
job kind, and command/session owner terminate. Unmatched terminal results are
retained as diagnostics and never complete unrelated work.

The application owns bus handles, serialization, pins, per-transfer timeout,
scheduling, recovery, and device-presence policy. The shared example transport
in `examples/esp32/I2cMasterTransport.*` owns native combined transfers, error
mapping, bounded probe operations, transactional open/rollback, and explicit
bus/device reconstruction. The diagnostic owns only one registered
device handle; a production shared-bus owner must coordinate every handle.
Transport counters are diagnostic facts, not applied-configuration authority.

## Profile and safety model

`bind` is zero-I2C. Initialization reads both identity registers and replays
the complete explicit profile. Profile setters modify only a staged fixed-size
`Config`; `profile validate` uses the same public validator as `bind`, and
`profile commit confirm` atomically updates desired state only after validation.
It never silently applies hardware changes: run `apply` explicitly. `addr` and
`variant` report build-time binding facts because changing either requires an
owner transport/device-handle rebuild and rebind.

Commands that can mutate hardware, perform broad/raw destructive reads, reset
the owner bus, or require special wiring use explicit `confirm` or fixture
gates. Explicit semantic STATUS/readiness/acquisition commands instead retain
their destructive-read evidence without a confirmation prompt. `scan` covers
usable addresses `0x08..0x77`, treats address NACK as a normal absence, and
stops on timeout/bus failure. After `busrecover confirm`, applied state is
invalid and a complete `init` is required. Reconstructing the diagnostic's bus
is containment, not a guarantee that the open ESP-IDF post-NACK invalid-state
failure was repaired.
Optional INTB and SD commands report `SKIP` when callbacks are not configured.
LDC1612 profiles reject channels 2 and 3 throughout settings, register helpers,
and sessions.

## Scope and evidence

This is diagnostic firmware, not a production bus manager. Internal pull-ups
and the example sensor profile are bring-up conveniences. No sensor, INTB, SD,
address-strap, fault-injection, drive-tuning, timing, or soak result is claimed
without a transcript from the exact fixture. Firmware prints its embedded Git
revision and dirty/clean status; HIL acceptance must match those target-reported
facts, not only the host checkout.

The generated `build/` directory is ignored. Generated `sdkconfig` and
`dependencies.lock` remain visible because they affect the flashed binary. For
release HIL, commit reviewed configuration and lock inputs before the final
clean rebuild so `firmware_status=clean` is meaningful.
