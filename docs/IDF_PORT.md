# LDC1614 ESP-IDF Port

The ESP-IDF example is a native IDF application in `examples/esp_idf/basic`.
It does not compile Arduino example sources and does not provide Arduino
compatibility facades.

Native boundaries:
- Entry point: `app_main()`.
- I2C: `driver/i2c_master.h` with `i2c_new_master_bus()`,
  `i2c_master_bus_add_device()`, `i2c_master_probe()`,
  `i2c_master_transmit()`, and `i2c_master_transmit_receive()`.
- CLI input: fixed C buffer using `getchar()`.
- Timing/yield/reset: ESP-IDF timer, GPIO, and FreeRTOS APIs are injected
  through the example adapter.
- Forbidden in IDF examples: `Arduino.h`, `Wire.h`, `String`, `Serial`,
  `TwoWire`, `ArduinoCompat`, `IdfArduinoCompat`, and including
  `examples/01_basic_bringup_cli/main.cpp`.

The driver core remains framework-neutral. Hardware access is injected through
`Config::i2cWrite`, `Config::i2cWriteRead`, optional GPIO/reset hooks,
`Config::nowMs`, and `Config::cooperativeYield`.

Run the static contract check after touching the IDF example:

```sh
python tools/check_idf_example_contract.py
python tools/check_core_timing_guard.py
```
