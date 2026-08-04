/**
 * @file BoardConfig.h
 * @brief Example board configuration for ESP32-S2 / ESP32-S3 reference hardware.
 *
 * These are convenience defaults for reference designs only.
 * NOT part of the library API. Override for your hardware.
 *
 * @warning The library itself is board-agnostic and owns no pins. The example
 *          transport/GPIO callbacks close over these application-owned pins.
 */

#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace board {

// ====================================================================
// EXAMPLE DEFAULTS - ESP32-S2 / ESP32-S3 REFERENCE HARDWARE
// ====================================================================
// These values are NOT library defaults. They are example-only values.
// Override them for your board by creating your own BoardConfig.h or
// passing explicit values to Config structs in your application.
// ====================================================================

/// @brief I2C SDA pin (data line). Example default for ESP32-S2/S3.
static constexpr int I2C_SDA = 8;

/// @brief I2C SCL pin (clock line). Example default for ESP32-S2/S3.
static constexpr int I2C_SCL = 9;

#ifndef LDC1614_EXAMPLE_I2C_FREQUENCY_HZ
#define LDC1614_EXAMPLE_I2C_FREQUENCY_HZ 100000
#endif

/// @brief Example I2C clock frequency in Hz. Override for HIL A/B tests.
static constexpr uint32_t I2C_FREQ_HZ = LDC1614_EXAMPLE_I2C_FREQUENCY_HZ;

/// @brief I2C timeout in milliseconds for example transactions.
static constexpr uint16_t I2C_TIMEOUT_MS = 50;

#ifndef LDC1614_EXAMPLE_I2C_ADDRESS
#define LDC1614_EXAMPLE_I2C_ADDRESS 0x2A
#endif

#ifndef LDC1614_EXAMPLE_CHANNEL_COUNT
#define LDC1614_EXAMPLE_CHANNEL_COUNT 4
#endif

/// @brief Example default LDC I2C address. Override with build flags for HIL.
static constexpr uint8_t LDC_I2C_ADDRESS = LDC1614_EXAMPLE_I2C_ADDRESS;

/// @brief Example default LDC channel count. Override with build flags for LDC1612.
static constexpr uint8_t LDC_CHANNEL_COUNT = LDC1614_EXAMPLE_CHANNEL_COUNT;

static_assert(LDC_I2C_ADDRESS == 0x2A || LDC_I2C_ADDRESS == 0x2B,
              "LDC example address must be 0x2A or 0x2B");
static_assert(LDC_CHANNEL_COUNT == 2 || LDC_CHANNEL_COUNT == 4,
              "LDC example variant must have 2 or 4 channels");
static_assert(I2C_FREQ_HZ > 0U && I2C_FREQ_HZ <= 400000U,
              "LDC example I2C frequency must be 1..400000 Hz");

/// @brief INTB pin from LDC1614 (push-pull, active low).
/// Set to -1 to disable.
static constexpr int INTB_PIN = -1;

/// @brief Initialize INTB pin for examples.
inline void initIntbPin() {
  if (INTB_PIN >= 0) {
    pinMode(INTB_PIN, INPUT);
  }
}

/// @brief Initialize Serial for examples.
inline void initSerial(uint32_t baud = 115200) {
  Serial.begin(baud);
  // Allow native USB CDC targets (ESP32-S2/S3) to enumerate before first log.
  const uint32_t startMs = millis();
  while (!Serial && (millis() - startMs) < 3000U) {
    delay(10);
  }
}

}  // namespace board
