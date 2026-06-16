# LDC1614 I2C Integration Notes

This driver does not own I2C. Applications inject transport callbacks through
`Config::i2cWrite` and `Config::i2cWriteRead`, then own bus lifecycle,
serialization, timeout policy, recovery policy, GPIO wiring, and scheduling.

## Transport Boundary

- Core code under `include/` and `src/` is framework-neutral.
- Transport callbacks are non-owning; the driver does not retain ownership of
  Arduino `Wire`, ESP-IDF bus handles, locks, tasks, or pins.
- Serialize public driver calls externally. Instances are not internally
  thread-safe and public APIs are not ISR-safe.
- Every fallible public API returns `Status`; transport-specific errors must be
  mapped by the injected adapter.

## Error Taxonomy

- `probe()` uses raw I2C and does not update health counters.
- Address NACK during probe maps to `DEVICE_NOT_FOUND`.
- Data NACK, timeout, bus, and generic I2C failures preserve their original
  `Err` code, detail, and static message where the transport can distinguish
  them.
- Normal public I2C operations on a latched `OFFLINE` driver return `BUSY`
  with `Driver is offline; call recover()` and do not touch the bus.
- `recover()` is the explicit path allowed to use I2C while offline.

## Dirty Hardware Configuration

Multi-register configuration writes can partially reach the device. Full apply
paths (`begin()`, `syncConfig()`, recovery reapply, and `resetAndReapply()`)
force a sleep-mode CONFIG write before channel/global registers and clear dirty
state only after the full apply succeeds.

Check `hardwareConfigDirty()` and `hardwareConfigDirtyError()` after failed
configuration writes or diagnostic raw writes. Do not trust
configuration-dependent cached behavior again until `syncConfig()`, `recover()`,
`resetAndReapply()`, or a fresh `begin()` clears the dirty state.

## Bounded Poll Integration

For application-owned I2C managers that advance work in bounded polls, the
driver exposes one-active-job APIs:

- `startReadChannels(mask)` schedules selected DATAx reads without reading
  STATUS.
- `poll(nowMs, maxInstructions)` advances the active job by at most
  `maxInstructions` register transfers.
- `readChannelsReady()` reports successful completion of the most recent
  channel-read job.
- `getChannelSample(ch, out)` returns samples from the completed read job.
- `startApplyConfig()` schedules cached configuration apply.
- `startResetAndReapply()` schedules `RESET_DEV` followed by cached config
  apply.

One 16-bit register read or write consumes one instruction. DATAx_MSB and
DATAx_LSB are two reads, but no channel sample is exposed until both complete
in datasheet order. While a poll job is active, other public I2C APIs return
`BUSY`; only `poll()` advances that job.

`startReadChannels()` intentionally does not read STATUS, because STATUS reads
can clear sticky flags and de-assert INTB. Use `readFreshChannels()` or
`readDeviceStatus()` when STATUS/UNREADCONVx evidence is required.

## Package Boundary

`include/LDC1614/Version.h` is generated from `library.json` and tracked because
it is a public header included by `LDC1614/LDC1614.h`. Before packaging or
tagging, run:

```sh
python scripts/generate_version.py check
python tools/check_clean_consumer_compile.py
python -m platformio pkg pack
```

Package archives are local artifacts and should be removed from the source tree
after review.
