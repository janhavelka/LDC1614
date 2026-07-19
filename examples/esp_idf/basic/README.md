# Native ESP-IDF diagnostic example

This native ESP-IDF application uses `app_main`, `driver/i2c_master.h`, a
fixed-buffer CLI, one application owner task, ESP timer timekeeping, and native GPIO
and task APIs. It contains no Arduino facade.

The example binds an explicit profile without I2C, schedules cooperative
initialization, and advances at most one driver transport callback per console
service pass. This diagnostic has one task and therefore no redundant mutex;
an application with multiple callers must own serialization. The application
owns the bus, locking, absolute deadline,
transaction budget, task scheduling, pins, retry/backoff, recovery, and device
presence policy. The library's transport statistics are diagnostic only.

Commands cover initialization/apply/reset jobs, acquisition, bus-silent
cancellation and invalidation, STATUS/readiness, sleep/wake, raw diagnostics,
and pure timing/frequency helpers. `probe` deliberately performs two diagnostic
identity reads; it is not the production initialization path.

This is not a production bus manager. The internal pull-up setting and example
sensor profile are bring-up conveniences. No sensor, INTB, SD, address-strap,
fault, timing, or soak behavior is claimed without captured evidence from the
exact target hardware.

The generated `build/` directory is ignored. Generated `sdkconfig` and
`dependencies.lock` remain visible to Git because they affect the flashed
binary. For release-acceptance HIL, commit the reviewed configuration and lock
inputs before the final clean rebuild; leaving either untracked intentionally
reports `firmware_status=dirty`.
