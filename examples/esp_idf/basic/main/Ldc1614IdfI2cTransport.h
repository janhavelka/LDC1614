#pragma once

#include <cstddef>
#include <cstdint>

#include "LDC1614/LDC1614.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"

struct Ldc1614IdfI2c {
  i2c_master_bus_handle_t bus = nullptr;
  i2c_master_dev_handle_t dev = nullptr;
  uint8_t address = 0x2A;
  gpio_num_t intb = GPIO_NUM_NC;
};

LDC1614::Status ldc1614IdfI2cWrite(uint8_t addr, const uint8_t* data, size_t len,
                                   uint32_t timeoutMs, void* user);
LDC1614::Status ldc1614IdfI2cWriteRead(uint8_t addr, const uint8_t* txData,
                                       size_t txLen, uint8_t* rxData,
                                       size_t rxLen, uint32_t timeoutMs,
                                       void* user);
LDC1614::Status ldc1614IdfIntbAsserted(bool& asserted, void* user);
uint64_t ldc1614IdfUptimeMs();
