# Native ESP-IDF integration

`examples/esp_idf/basic` is a native ESP-IDF diagnostic application, not a
production bus manager. It uses `app_main`, `driver/i2c_master.h`, one
application owner task, fixed C input/output buffers, `esp_timer`, FreeRTOS task
APIs, and native GPIO configuration. It contains no Arduino compatibility
facade, `String`, `Serial`, `Wire`, or `TwoWire`.

The example demonstrates the v3 boundary:

- `Config` retains only non-owning I2C callbacks and optional bus-silent INTB
  observation;
- `bind()` validates the explicit address, variant, clock, channel, electrical,
  error, and timing profile with zero I2C;
- initialization and acquisition are scheduled with an operation ID and
  absolute 64-bit deadline;
- the fixed-memory local CLI exposes the same table-driven command, help,
  safety, result, and diagnostic-session contract as the Arduino example;
- the console owner drains only a fixed byte budget and calls `poll(now, 1)`,
  so input cannot starve jobs and one pass performs at most one physical driver
  callback; and
- terminal results are removed from the fixed FIFO exactly once.

The application owns bus creation/destruction, lock policy, callback timeout,
task scheduling, deadline, retry/backoff, device presence, health, SD/reset
GPIOs, shared-bus recovery, and physical validation. If it resets or recovers
the device or bus, it must call `invalidateAppliedState(reason)` with the
owner-observed cause and schedule complete initialization/replay before
acquisition.

Because the diagnostic has one driver-owning task, its transport does not add
a second mutex wait ahead of the native I2C timeout. Multi-task applications
must serialize before invoking the driver and include lock wait in their own
whole-request deadline.

The diagnostic `probe` session performs two identity reads and is not the
normal owner initialization path. Discovery, dump, verify, self-test, watch,
stress, sample-rate, and soak are bounded example-owned sessions rather than
production scheduling policy. The example's internal I2C pull-up setting,
GPIO8/GPIO9 pins, `0x2A` address, internal-clock estimate, and single-channel
sensor profile are placeholders. Production hardware needs sized external
pull-ups and a reviewed board profile.

Run source-boundary guards after changing the example:

```sh
python tools/check_idf_example_contract.py
python tools/check_core_timing_guard.py
```

The checker parses the ESP-IDF `SRCS` list, scans compiled sources and local
headers, and compares the local command table against the host-only CLI
manifest and Arduino implementation. The IDF build never compiles the Arduino
or common CLI source. The checker does not replace an `idf.py` build. Maintained CI builds ESP32-S2
and ESP32-S3 with the pinned IDF version recorded in the workflow, and
`idf_component.yml` declares the matching `idf: ">=6.0.0"` component floor.
No hardware behavior is claimed without target logs for the exact variant,
address, clock,
sensor, INTB/SD wiring, deadline/fault procedure, and soak configuration.
