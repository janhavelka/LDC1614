#include "I2cMasterTransport.h"

#include <limits>

#include "esp_err.h"
#include "esp_timer.h"

namespace esp32_i2c {
namespace {

int clampTimeoutMs(uint32_t timeoutMs) {
  const uint32_t maximum =
      static_cast<uint32_t>(std::numeric_limits<int>::max());
  return static_cast<int>(timeoutMs > maximum ? maximum : timeoutMs);
}

LDC1614::Status mapEspErr(esp_err_t error, const char* context) {
  if (error == ESP_OK) {
    return LDC1614::Status::Ok();
  }
  if (error == ESP_ERR_TIMEOUT) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_TIMEOUT, "I2C timeout",
                                  static_cast<int32_t>(error));
  }
  if (error == ESP_ERR_INVALID_RESPONSE) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_ERROR,
                                  "I2C invalid response or NACK",
                                  static_cast<int32_t>(error));
  }
  if (error == ESP_ERR_INVALID_ARG) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM, context,
                                  static_cast<int32_t>(error));
  }
  if (error == ESP_ERR_INVALID_STATE) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS, "I2C invalid state",
                                  static_cast<int32_t>(error));
  }
  if (error == ESP_ERR_NO_MEM || error == ESP_ERR_NOT_FOUND) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS, context,
                                  static_cast<int32_t>(error));
  }
  return LDC1614::Status::Error(LDC1614::Err::I2C_ERROR, context,
                                static_cast<int32_t>(error));
}

LDC1614::Status validateContext(uint8_t address, void* user,
                                Context*& context) {
  context = static_cast<Context*>(user);
  if (context == nullptr || context->device == nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                                  "I2C device is not configured");
  }
  if (address != context->address) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM,
                                  "Unexpected I2C address",
                                  static_cast<int32_t>(address));
  }
  return LDC1614::Status::Ok();
}

}  // namespace

LDC1614::Status open(Context& context, const BusConfig& config) {
  if (context.bus != nullptr || context.device != nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                                  "I2C context is already open");
  }
  if (config.sda == GPIO_NUM_NC || config.scl == GPIO_NUM_NC ||
      config.frequencyHz == 0U || context.address < 0x08U ||
      context.address > 0x77U) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG,
                                  "I2C bus configuration is invalid");
  }

  i2c_master_bus_config_t busConfig{};
  busConfig.clk_source = I2C_CLK_SRC_DEFAULT;
  busConfig.i2c_port = config.port;
  busConfig.sda_io_num = config.sda;
  busConfig.scl_io_num = config.scl;
  busConfig.glitch_ignore_cnt = 7;
  busConfig.flags.enable_internal_pullup = config.enableInternalPullups;
  LDC1614::Status status = mapEspErr(
      i2c_new_master_bus(&busConfig, &context.bus), "I2C bus init failed");
  if (!status.ok()) {
    context.bus = nullptr;
    return status;
  }

  i2c_device_config_t deviceConfig{};
  deviceConfig.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  deviceConfig.device_address = context.address;
  deviceConfig.scl_speed_hz = config.frequencyHz;
  status = mapEspErr(
      i2c_master_bus_add_device(context.bus, &deviceConfig, &context.device),
      "I2C device add failed");
  if (!status.ok()) {
    context.device = nullptr;
  }
  return status;
}

LDC1614::Status reset(Context& context) {
  if (context.bus == nullptr || context.device == nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                                  "I2C context is not open");
  }
  return mapEspErr(i2c_master_bus_reset(context.bus),
                   "I2C bus reset failed");
}

LDC1614::Status write(uint8_t address, const uint8_t* data, size_t length,
                      uint32_t timeoutMs, void* user) {
  Context* context = nullptr;
  LDC1614::Status status = validateContext(address, user, context);
  if (!status.ok()) {
    return status;
  }
  if (data == nullptr || length == 0U) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM,
                                  "Invalid I2C write buffer");
  }

  return mapEspErr(i2c_master_transmit(context->device, data, length,
                                       clampTimeoutMs(timeoutMs)),
                   "I2C write failed");
}

LDC1614::Status writeRead(uint8_t address, const uint8_t* txData,
                          size_t txLength, uint8_t* rxData, size_t rxLength,
                          uint32_t timeoutMs, void* user) {
  Context* context = nullptr;
  LDC1614::Status status = validateContext(address, user, context);
  if (!status.ok()) {
    return status;
  }
  if (txData == nullptr || txLength == 0U || rxData == nullptr ||
      rxLength == 0U) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM,
                                  "Invalid I2C write-read buffer");
  }

  return mapEspErr(i2c_master_transmit_receive(
                       context->device, txData, txLength, rxData, rxLength,
                       clampTimeoutMs(timeoutMs)),
                   "I2C write-read failed");
}

LDC1614::Status intbAsserted(bool& asserted, void* user) {
  auto* context = static_cast<Context*>(user);
  if (context == nullptr || context->intb == GPIO_NUM_NC) {
    asserted = false;
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG,
                                  "INTB pin not configured");
  }
  asserted = gpio_get_level(context->intb) == 0;
  return LDC1614::Status::Ok();
}

ProbeResult probe(Context& context, uint8_t address, uint32_t timeoutMs) {
  if (context.bus == nullptr || address < 0x08U || address > 0x77U) {
    return ProbeResult::ERROR;
  }
  const esp_err_t error =
      i2c_master_probe(context.bus, address, clampTimeoutMs(timeoutMs));
  if (error == ESP_OK) {
    return ProbeResult::ACK;
  }
  if (error == ESP_ERR_NOT_FOUND) {
    return ProbeResult::NACK;
  }
  if (error == ESP_ERR_TIMEOUT) {
    return ProbeResult::TIMEOUT;
  }
  return ProbeResult::ERROR;
}

uint64_t uptimeMs() {
  return static_cast<uint64_t>(esp_timer_get_time() / 1000);
}

}  // namespace esp32_i2c
