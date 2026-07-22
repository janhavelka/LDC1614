# Arduino diagnostic bring-up CLI

This PlatformIO/Arduino example demonstrates the v3 cooperative owner contract.
It is diagnostic firmware, not a production bus manager or hardware-validation
claim.

The application owns `Wire`, pins, serialization, the 64-bit monotonic time
extension, absolute job deadlines, transfer budget, and recovery policy. It
first calls bus-silent `bind()`, schedules `startInitialize()`, then advances
the active job by at most one I2C callback per `loop()` pass. Terminal results
are consumed through `takeResult()` exactly once.

The ESP32 Wire adapter treats its repeated-start address and read phases as one
callback and gives the read only the timeout remaining after the address phase.
The repository pins pioarduino `53.03.13`; do not upgrade the example toolchain
without repeating combined-read and post-NACK recovery testing on real LDC
hardware.

The maintained ESP32-S2 PlatformIO profile uses its internal USB CDC upload
path: automatic bootloader entry, port re-enumeration, and return to the
application. Routine firmware updates must not require operator reset cycles.

Useful commands:

- `init`, `apply`, `resetreapply`, `acquire [mask]`, `cancel`, `progress`;
- `status`, `ready`, `sleep`, `wake`, `initdrive <channel>`;
- `busrecover` for one explicit owner-controlled `Wire.end()`/reinitialize,
  followed by a complete `init` replay;
- `drv`, `cfg`, `probe`, `reg`, `wreg`, `timing`, and `freq`; and
- `invalidate` after owner-observed power loss, reset, removal, or bus recovery.

`probe` is an explicitly diagnostic two-register identity read. Raw register
writes can make applied configuration unknown or dirty; replay configuration
before acquiring trusted data. An acquisition result includes the destructive
pre-DATA STATUS snapshot, quality/freshness/error/overrun masks, and sequential
channel samples. A batch is not a simultaneous measurement.

`scan` is a bounded external diagnostic loop over the 112 usable addresses
`0x08..0x77`; reserved I2C address groups are deliberately excluded. Address
NACK means no device and is ignored; timeout or bus failure stops the scan and
prints a non-OK status. It is not a production shared-bus scan policy or one
core-driver job. Startup does not scan automatically. Run `busrecover` and
`init` after scan diagnostics before resuming device operations; this makes
post-NACK controller recovery explicit and avoids an MCU reboot.

The example profile values, GPIO8/GPIO9 pins, 43 MHz internal-clock estimate,
sensor-frequency bounds, and drive-current code are placeholders. Replace and
validate them against the exact board, LC tank, target, and clock plan.
