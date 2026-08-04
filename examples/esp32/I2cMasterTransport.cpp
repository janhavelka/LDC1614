#include "I2cMasterTransport.h"

#include <limits>

#include "esp_err.h"
#include "esp_timer.h"

namespace esp32_i2c {
namespace {

constexpr uint32_t MIN_I2C_FREQUENCY_HZ = 10000U;
constexpr uint32_t MAX_I2C_FREQUENCY_HZ = 400000U;

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

LDC1614::Status mapTransactionErr(esp_err_t error, const char* context) {
  // ESP-IDF 5.5.x returns ESP_ERR_INVALID_STATE for every synchronous
  // transaction that does not finish in I2C_STATUS_DONE, including a normal
  // slave NACK. It does not expose enough phase information to distinguish an
  // address NACK, data NACK, timeout race, or controller condition. Preserve
  // the raw detail but do not mislabel this ambiguous device transaction as a
  // failed shared bus; that would invite destructive owner-wide recovery.
  if (error == ESP_ERR_INVALID_STATE) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_ERROR, context,
                                  static_cast<int32_t>(error));
  }
  return mapEspErr(error, context);
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

i2c_device_config_t makeDeviceConfig(uint8_t address, uint32_t frequencyHz) {
  i2c_device_config_t config{};
  config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  config.device_address = address;
  config.scl_speed_hz = frequencyHz;
  return config;
}

bool validFrequency(uint32_t frequencyHz) {
  return frequencyHz >= MIN_I2C_FREQUENCY_HZ &&
         frequencyHz <= MAX_I2C_FREQUENCY_HZ;
}

enum class TransferKind : uint8_t { WRITE, WRITE_READ, DISCOVERY };

void recordTransfer(Context& context, TransferKind kind,
                    const LDC1614::Status& status) {
  uint32_t* counter = kind == TransferKind::WRITE
                          ? &context.transferStats.writes
                          : (kind == TransferKind::WRITE_READ
                                 ? &context.transferStats.writeReads
                                 : &context.transferStats.discoveries);
  if (*counter < UINT32_MAX) ++(*counter);
  if (!status.ok() && context.transferStats.failures < UINT32_MAX) {
    ++context.transferStats.failures;
  }
  context.transferStats.lastStatus = status;
}

}  // namespace

LDC1614::Status open(Context& context, const BusConfig& config) {
  if (context.bus != nullptr || context.device != nullptr ||
      context.discoveryDevice != nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                                  "I2C context is already open");
  }
  if (config.sda == GPIO_NUM_NC || config.scl == GPIO_NUM_NC ||
      !validFrequency(config.frequencyHz) || context.address < 0x08U ||
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

  const i2c_device_config_t deviceConfig =
      makeDeviceConfig(context.address, config.frequencyHz);
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

LDC1614::Status close(Context& context) {
  if (context.discoveryDevice != nullptr) {
    LDC1614::Status status = mapEspErr(
        i2c_master_bus_rm_device(context.discoveryDevice),
        "I2C discovery device removal failed");
    if (!status.ok()) {
      return status;
    }
    context.discoveryDevice = nullptr;
    context.discoveryAddress = 0U;
  }
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
  if (context.bus != nullptr || context.device != nullptr ||
      context.discoveryDevice != nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                                  "I2C context is still open");
  }
  if (!context.hasBusConfig) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG,
                                  "I2C bus configuration unavailable");
  }
  const BusConfig config = context.busConfig;
  return open(context, config);
}

LDC1614::Status recover(Context& context) {
  if (!context.hasBusConfig) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG,
                                  "I2C bus configuration unavailable");
  }
  if ((context.device != nullptr || context.discoveryDevice != nullptr) &&
      context.bus == nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                                  "I2C context handles are inconsistent");
  }
  LDC1614::Status status = close(context);
  if (!status.ok()) {
    return status;
  }
  status = reopen(context);
  if (!status.ok()) {
    return status;
  }

  // Reconstruct only software/controller state here. ESP-IDF 5.5.x collapses
  // NACK and several controller outcomes into ESP_ERR_INVALID_STATE. Blindly
  // clocking the lines after that ambiguous result can corrupt an LDC161x
  // transaction, and an address-only probe does not qualify its required
  // combined register-read protocol. The owner must invalidate every affected
  // device and use full device-specific initialization/replay for admission.
  return LDC1614::Status::Ok();
}

uint32_t frequencyHz(const Context& context) {
  return context.hasBusConfig ? context.busConfig.frequencyHz : 0U;
}

LDC1614::Status setFrequency(Context& context, uint32_t requestedHz) {
  if (context.bus == nullptr || context.device == nullptr ||
      !context.hasBusConfig) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                                  "I2C context is not open");
  }
  if (!validFrequency(requestedHz)) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM,
                                  "I2C frequency must be 10 kHz to 400 kHz",
                                  static_cast<int32_t>(requestedHz));
  }
  const uint32_t previousHz = context.busConfig.frequencyHz;
  if (requestedHz == previousHz) return LDC1614::Status::Ok();

  if (context.discoveryDevice != nullptr) {
    LDC1614::Status status = mapEspErr(
        i2c_master_bus_rm_device(context.discoveryDevice),
        "I2C discovery device removal for frequency change failed");
    if (!status.ok()) return status;
    context.discoveryDevice = nullptr;
    context.discoveryAddress = 0U;
  }

  LDC1614::Status status = mapEspErr(
      i2c_master_bus_rm_device(context.device),
      "I2C device removal for frequency change failed");
  if (!status.ok()) return status;
  context.device = nullptr;

  const i2c_device_config_t requestedConfig =
      makeDeviceConfig(context.address, requestedHz);
  i2c_master_dev_handle_t replacement = nullptr;
  status = mapEspErr(
      i2c_master_bus_add_device(context.bus, &requestedConfig, &replacement),
      "I2C device add for frequency change failed");
  if (status.ok()) {
    context.device = replacement;
    context.busConfig.frequencyHz = requestedHz;
    return status;
  }

  // Resource-only rollback: no wire transaction and no hidden device retry.
  const i2c_device_config_t rollbackConfig =
      makeDeviceConfig(context.address, previousHz);
  i2c_master_dev_handle_t rollbackDevice = nullptr;
  const LDC1614::Status rollback = mapEspErr(
      i2c_master_bus_add_device(context.bus, &rollbackConfig, &rollbackDevice),
      "I2C frequency rollback failed");
  if (!rollback.ok()) return rollback;
  context.device = rollbackDevice;
  return status;
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

  status = mapTransactionErr(i2c_master_transmit(context->device, data, length,
                                                 clampTimeoutMs(timeoutMs)),
                             "I2C write failed");
  recordTransfer(*context, TransferKind::WRITE, status);
  return status;
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

  status = mapTransactionErr(
      i2c_master_transmit_receive(context->device, txData, txLength, rxData,
                                  rxLength, clampTimeoutMs(timeoutMs)),
      "I2C write-read failed");
  recordTransfer(*context, TransferKind::WRITE_READ, status);
  return status;
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

LDC1614::Status readRegisterAt(Context& context, uint8_t address,
                               uint8_t registerAddress, uint16_t& value,
                               uint32_t timeoutMs) {
  value = 0U;
  if (context.bus == nullptr || context.device == nullptr ||
      !context.hasBusConfig || address < 0x08U || address > 0x77U) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                                  "I2C discovery context is invalid");
  }

  i2c_master_dev_handle_t handle = context.device;
  if (address != context.address) {
    if (context.discoveryDevice != nullptr &&
        context.discoveryAddress != address) {
      const LDC1614::Status removed = mapEspErr(
          i2c_master_bus_rm_device(context.discoveryDevice),
          "I2C stale discovery device removal failed");
      if (!removed.ok()) return removed;
      context.discoveryDevice = nullptr;
      context.discoveryAddress = 0U;
    }
    if (context.discoveryDevice == nullptr) {
      const i2c_device_config_t config =
          makeDeviceConfig(address, context.busConfig.frequencyHz);
      i2c_master_dev_handle_t discoveryDevice = nullptr;
      const LDC1614::Status added = mapEspErr(
          i2c_master_bus_add_device(context.bus, &config, &discoveryDevice),
          "I2C discovery device add failed");
      if (!added.ok()) return added;
      context.discoveryDevice = discoveryDevice;
      context.discoveryAddress = address;
    }
    handle = context.discoveryDevice;
  }

  uint8_t bytes[2]{};
  const esp_err_t transaction = i2c_master_transmit_receive(
      handle, &registerAddress, 1U, bytes, sizeof(bytes),
      clampTimeoutMs(timeoutMs));
  const LDC1614::Status status =
      mapTransactionErr(transaction, "I2C discovery register read failed");
  recordTransfer(context, TransferKind::DISCOVERY, status);

  if (!status.ok()) return status;
  value = static_cast<uint16_t>((static_cast<uint16_t>(bytes[0]) << 8U) |
                                static_cast<uint16_t>(bytes[1]));
  return LDC1614::Status::Ok();
}

uint64_t uptimeMs() {
  return static_cast<uint64_t>(esp_timer_get_time() / 1000);
}

}  // namespace esp32_i2c
