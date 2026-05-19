# LDC1614 ESP-IDF Port Implementation

Implementation status:
- `examples/common/Ldc1614Cli.*` contains a framework-neutral command
  implementation used by the native IDF example.
- `examples/esp_idf/basic/main/main.cpp` owns `app_main()`, fixed-buffer CLI
  input, GPIO setup, and native I2C bus creation.
- `Ldc1614IdfI2cTransport.*` maps ESP-IDF I2C/GPIO/timing/reset APIs to the
  framework-neutral driver callbacks.
- The ESP-IDF CMake target does not compile Arduino sources or compatibility
  facades.

The command contract is enforced by `tools/check_idf_example_contract.py`.
When adding a CLI command to the Arduino bring-up example, add the matching
framework-neutral/native IDF command or explicitly document why parity is not
applicable.
