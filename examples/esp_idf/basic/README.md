# LDC1614 Native ESP-IDF Basic Example

This is a native ESP-IDF diagnostic bring-up example. It uses
`driver/i2c_master.h`, an example-owned I2C mutex, a fixed-buffer CLI parser,
and ESP-IDF GPIO/timer/task hooks injected into the framework-neutral driver.
The CLI includes bounded `sleep` and `wake` controls for conversion-state
smoke checks.

This example is not a production bus manager. Production applications must own
bus lifecycle, driver-call serialization, lock timeout policy, task scheduling,
recovery/backoff policy, GPIO/INTB integration, and hardware validation.

No hardware behavior is claimed unless real LDC1614/LDC1612 logs are captured
for the specific board, address strap, INTB/SD wiring, sensor configuration, and
fault tests.
