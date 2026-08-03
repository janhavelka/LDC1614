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
    return LDC1614::Status::Error(LDC1614::Err::I2C_TIMEOUT, context,
                                  static_cast<int32_t>(error));
  }
  if (error == ESP_ERR_INVALID_RESPONSE) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_ERROR, context,
                                  static_cast<int32_t>(error));
  }
  if (error == ESP_ERR_INVALID_ARG) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM, context,
                                  static_cast<int32_t>(error));
  }
  if (error == ESP_ERR_INVALID_STATE) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS, context,
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

LDC1614::Status openWithPolicy(Context& context, const BusConfig& config,
                               bool resetBeforeDevice) {
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
  context.busConfig = config;
  context.hasBusConfig = true;

  i2c_master_bus_config_t nativeBusConfig{};
  nativeBusConfig.clk_source = I2C_CLK_SRC_DEFAULT;
  nativeBusConfig.i2c_port = config.port;
  nativeBusConfig.sda_io_num = config.sda;
  nativeBusConfig.scl_io_num = config.scl;
  nativeBusConfig.glitch_ignore_cnt = 7;
  nativeBusConfig.flags.enable_internal_pullup = config.enableInternalPullups;
  i2c_master_bus_handle_t bus = nullptr;
  LDC1614::Status status = mapEspErr(
      i2c_new_master_bus(&nativeBusConfig, &bus), "I2C bus init failed");
  if (!status.ok()) {
    return status;
  }

  if (resetBeforeDevice) {
    status = mapEspErr(i2c_master_bus_reset(bus),
                       "I2C recovery bus reset failed");
    if (!status.ok()) {
      const esp_err_t rollback = i2c_del_master_bus(bus);
      if (rollback != ESP_OK) {
        context.bus = bus;
        return mapEspErr(rollback, "I2C bus rollback failed");
      }
      return status;
    }
  }

  i2c_device_config_t deviceConfig{};
  deviceConfig.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  deviceConfig.device_address = context.address;
  deviceConfig.scl_speed_hz = config.frequencyHz;
  i2c_master_dev_handle_t device = nullptr;
  status = mapEspErr(
      i2c_master_bus_add_device(bus, &deviceConfig, &device),
      "I2C device add failed");
  if (!status.ok()) {
    const esp_err_t rollback = i2c_del_master_bus(bus);
    if (rollback != ESP_OK) {
      context.bus = bus;
      return mapEspErr(rollback, "I2C bus rollback failed");
    }
    return status;
  }

  context.bus = bus;
  context.device = device;
  return LDC1614::Status::Ok();
}

}  // namespace

LDC1614::Status open(Context& context, const BusConfig& config) {
  return openWithPolicy(context, config, false);
}

LDC1614::Status close(Context& context) {
  if (context.device != nullptr) {
    LDC1614::Status status = mapEspErr(
        i2c_master_bus_rm_device(context.device),
        "I2C device removal failed");
    if (!status.ok()) {
      return status;
    }
    context.device = nullptr;
  }
  if (context.bus != nullptr) {
    LDC1614::Status status = mapEspErr(
        i2c_del_master_bus(context.bus), "I2C bus deletion failed");
    if (!status.ok()) {
      return status;
    }
    context.bus = nullptr;
  }
  return LDC1614::Status::Ok();
}

LDC1614::Status reopen(Context& context) {
  if (context.bus != nullptr || context.device != nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                                  "I2C context is still open");
  }
  if (!context.hasBusConfig) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG,
                                  "I2C bus configuration unavailable");
  }
  const BusConfig config = context.busConfig;
  return openWithPolicy(context, config, false);
}

LDC1614::Status recover(Context& context, uint32_t timeoutMs) {
  if (!context.hasBusConfig) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG,
                                  "I2C bus configuration unavailable");
  }
  if (timeoutMs == 0U) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM,
                                  "I2C recovery timeout must be nonzero");
  }
  if (context.device != nullptr && context.bus == nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                                  "I2C context handles are inconsistent");
  }
  LDC1614::Status status = close(context);
  if (!status.ok()) {
    return status;
  }
  status = openWithPolicy(context, context.busConfig, true);
  if (!status.ok()) {
    return status;
  }

  // The pinned ESP-IDF 5.5.5 backend uses one generic INVALID_STATE result for
  // several failed synchronous-transaction outcomes. Its bounded probe path
  // both proves that the rebuilt target is responding and normalizes the
  // new-master terminal status before the caller starts combined reads.
  const esp_err_t probeError = i2c_master_probe(
      context.bus, context.address, clampTimeoutMs(timeoutMs));
  if (probeError == ESP_OK) {
    return LDC1614::Status::Ok();
  }
  if (probeError == ESP_ERR_NOT_FOUND) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_NACK_ADDR,
                                  "I2C recovery target did not ACK",
                                  static_cast<int32_t>(probeError));
  }
  return mapEspErr(probeError, "I2C recovery target probe failed");
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
