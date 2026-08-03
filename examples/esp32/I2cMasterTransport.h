/**
 * @file I2cMasterTransport.h
 * @brief Example-owned ESP-IDF new-master I2C transport for ESP32 targets.
 *
 * This module is shared by the Arduino and native ESP-IDF diagnostics. It is
 * application glue, not part of the LDC1614 library API. The application owns
 * bus creation, device handles, serialization, timeout policy, and recovery.
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "LDC1614/LDC1614.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"

namespace esp32_i2c {

struct BusConfig {
  i2c_port_num_t port = I2C_NUM_0;
  gpio_num_t sda = GPIO_NUM_NC;
  gpio_num_t scl = GPIO_NUM_NC;
  uint32_t frequencyHz = 400000;
  bool enableInternalPullups = true;
};

struct Context {
  i2c_master_bus_handle_t bus = nullptr;
  i2c_master_dev_handle_t device = nullptr;
  BusConfig busConfig{};
  uint8_t address = 0x2A;
  gpio_num_t intb = GPIO_NUM_NC;
  bool hasBusConfig = false;
};

enum class ProbeResult : uint8_t {
  ACK,
  NACK,
  TIMEOUT,
  ERROR,
};

LDC1614::Status open(Context& context, const BusConfig& config);
LDC1614::Status close(Context& context);
LDC1614::Status reopen(Context& context);
LDC1614::Status recover(Context& context, uint32_t timeoutMs);

LDC1614::Status write(uint8_t address, const uint8_t* data, size_t length,
                      uint32_t timeoutMs, void* user);
LDC1614::Status writeRead(uint8_t address, const uint8_t* txData,
                          size_t txLength, uint8_t* rxData, size_t rxLength,
                          uint32_t timeoutMs, void* user);
LDC1614::Status intbAsserted(bool& asserted, void* user);
ProbeResult probe(Context& context, uint8_t address, uint32_t timeoutMs);
uint64_t uptimeMs();

}  // namespace esp32_i2c
