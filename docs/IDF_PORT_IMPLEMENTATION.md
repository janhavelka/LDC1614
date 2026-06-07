# LDC1614 ESP-IDF Port Implementation

Implementation status:
- `examples/esp_idf/basic/main/Ldc1614IdfCli.*` contains a native ESP-IDF
  diagnostic CLI with fixed buffers and no `std::string` parser path.
- `examples/esp_idf/basic/main/main.cpp` owns `app_main()`, timed console
  input polling, GPIO setup, and native I2C bus creation.
- `Ldc1614IdfI2cTransport.*` maps ESP-IDF I2C/GPIO/timing/reset APIs to the
  framework-neutral driver callbacks and locks native I2C operations with an
  example-owned mutex.
- The ESP-IDF CMake target does not compile Arduino sources, the shared Arduino
  bring-up CLI, or compatibility facades.

The command/source contract is enforced by `tools/check_idf_example_contract.py`.
The checker parses the actual ESP-IDF `SRCS` list, scans compiled sources and
local headers for Arduino/`std::string` leakage, and checks the limited native
IDF diagnostic command set. This is a source-structure check, not runtime
validation.

Validation status:
- Static contract checks can confirm framework-boundary and source-list rules.
- PlatformIO ESP32-S2/S3 builds are Arduino-framework builds, not pure ESP-IDF
  builds.
- Pure ESP-IDF build success is claimed only when `idf.py` or CI logs record the
  build result for `examples/esp_idf/basic`.
- No LDC1614/LDC1612 hardware behavior, INTB behavior, address-pin variant,
  SD shutdown/wake, fault-injection, sensor behavior, or soak validation is
  claimed without captured hardware logs.

When adding future IDF commands, keep handlers short and single-action so the
CLI task returns to the periodic tick loop promptly. If a command intentionally
differs from the Arduino diagnostic CLI, document why parity is not applicable.
