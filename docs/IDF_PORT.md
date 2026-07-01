# LDC1614 ESP-IDF Port

The ESP-IDF example is a native IDF diagnostic bring-up application in
`examples/esp_idf/basic`. It is not a production bus manager. Production
applications must own I2C bus lifecycle, locking/serialization, timeouts, task
scheduling, recovery/backoff policy, GPIO/INTB integration, and hardware
validation.

The example does not compile Arduino example sources and does not provide
Arduino compatibility facades.

Native boundaries:
- Entry point: `app_main()`.
- I2C: `driver/i2c_master.h` with `i2c_new_master_bus()`,
  `i2c_master_bus_add_device()`, `i2c_master_transmit()`, and
  `i2c_master_transmit_receive()`.
- I2C access: example-owned mutex around native I2C operations. This serializes
  bus transactions only; applications with multiple tasks must still serialize
  public driver calls.
- CLI input: fixed C buffer with a timed `select()`/`read()` loop so the driver
  tick can run while waiting for console input.
- Timing/yield/reset: ESP-IDF timer, GPIO, and FreeRTOS APIs are injected
  through the example adapter.
- CLI: `examples/esp_idf/basic/main/Ldc1614IdfCli.*` uses fixed buffers and a
  limited diagnostic command set, including bounded `sleep` and `wake`
  commands. It does not compile the shared Arduino
  diagnostic CLI.
- Transport: `Ldc1614IdfI2cTransport.*` maps native I2C/GPIO/timing/reset APIs
  to the framework-neutral driver callbacks and uses an example-owned mutex for
  I2C operations.
- Forbidden in IDF examples: `Arduino.h`, `Wire.h`, `String`, `Serial`,
  `TwoWire`, `ArduinoCompat`, `IdfArduinoCompat`, and including
  `examples/01_basic_bringup_cli/main.cpp`.

The driver core remains framework-neutral. ESP-IDF APIs are confined to the
example adapter and must not appear under `include/` or `src/`. Hardware access
is injected through `Config::i2cWrite`, `Config::i2cWriteRead`, optional
GPIO/reset hooks, `Config::nowMs`, and `Config::cooperativeYield`.

Run the static contract checks after touching the IDF example:

```sh
python tools/check_idf_example_contract.py
python tools/check_core_timing_guard.py
```

The IDF checker parses the actual ESP-IDF `SRCS` list and scans compiled
sources plus local headers. Those checks validate source contracts only. Pure
ESP-IDF build success is claimed only when `idf.py` or CI build logs record it.
Hardware behavior, including INTB, SD, address variants, fault injection, sensor
behavior, and soak stability, requires captured logs from real
LDC1614/LDC1612 hardware.
