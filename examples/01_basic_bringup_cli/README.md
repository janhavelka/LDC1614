# Arduino diagnostic bring-up CLI

This PlatformIO/Arduino example demonstrates the v3 cooperative owner contract.
It is diagnostic firmware, not a production bus manager or hardware-validation
claim.

The application owns `Wire`, pins, serialization, the 64-bit monotonic time
extension, absolute job deadlines, transfer budget, and recovery policy. It
first calls bus-silent `bind()`, schedules `startInitialize()`, then advances
the active job by at most one I2C callback per `loop()` pass. Terminal results
are consumed through `takeResult()` exactly once.

Useful commands:

- `init`, `apply`, `resetreapply`, `acquire [mask]`, `cancel`, `progress`;
- `status`, `ready`, `sleep`, `wake`, `initdrive <channel>`;
- `drv`, `cfg`, `probe`, `reg`, `wreg`, `timing`, and `freq`; and
- `invalidate` after owner-observed power loss, reset, removal, or bus recovery.

`probe` is an explicitly diagnostic two-register identity read. Raw register
writes can make applied configuration unknown or dirty; replay configuration
before acquiring trusted data. An acquisition result includes the destructive
pre-DATA STATUS snapshot, quality/freshness/error/overrun masks, and sequential
channel samples. A batch is not a simultaneous measurement.

The example profile values, GPIO8/GPIO9 pins, 43 MHz internal-clock estimate,
sensor-frequency bounds, and drive-current code are placeholders. Replace and
validate them against the exact board, LC tank, target, and clock plan.
