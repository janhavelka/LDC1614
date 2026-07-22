#include "Ldc1614IdfI2cTransport.h"

#include <limits>

#include "esp_err.h"
#include "esp_timer.h"

namespace {

int clampTimeoutMs(uint32_t timeoutMs) {
  const uint32_t maxTimeout = static_cast<uint32_t>(std::numeric_limits<int>::max());
  return static_cast<int>(timeoutMs > maxTimeout ? maxTimeout : timeoutMs);
}

LDC1614::Status mapEspErr(esp_err_t err, const char* context) {
  if (err == ESP_OK) {
    return LDC1614::Status::Ok();
  }
  if (err == ESP_ERR_TIMEOUT) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_TIMEOUT, "I2C timeout",
                                  static_cast<int32_t>(err));
  }
  if (err == ESP_ERR_INVALID_RESPONSE) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_ERROR,
                                  "I2C invalid response or NACK",
                                  static_cast<int32_t>(err));
  }
  if (err == ESP_ERR_INVALID_ARG) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM, context,
                                  static_cast<int32_t>(err));
  }
  if (err == ESP_ERR_INVALID_STATE) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS, "I2C invalid state",
                                  static_cast<int32_t>(err));
  }
  if (err == ESP_ERR_NO_MEM || err == ESP_ERR_NOT_FOUND) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS, context,
                                  static_cast<int32_t>(err));
  }
  return LDC1614::Status::Error(LDC1614::Err::I2C_ERROR, context,
                                static_cast<int32_t>(err));
}

LDC1614::Status validateContext(uint8_t addr, void* user, Ldc1614IdfI2c*& ctx) {
  ctx = static_cast<Ldc1614IdfI2c*>(user);
  if (ctx == nullptr || ctx->dev == nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS, "IDF I2C device not configured");
  }
  if (addr != ctx->address) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM, "Unexpected I2C address",
                                  static_cast<int32_t>(addr));
  }
  return LDC1614::Status::Ok();
}

}  // namespace

LDC1614::Status ldc1614IdfI2cWrite(uint8_t addr, const uint8_t* data, size_t len,
                                   uint32_t timeoutMs, void* user) {
  Ldc1614IdfI2c* ctx = nullptr;
  LDC1614::Status st = validateContext(addr, user, ctx);
  if (!st.ok()) {
    return st;
  }
  if (data == nullptr || len == 0) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM, "Invalid I2C write buffer");
  }

  const esp_err_t err =
      i2c_master_transmit(ctx->dev, data, len, clampTimeoutMs(timeoutMs));
  return mapEspErr(err, "I2C write failed");
}

LDC1614::Status ldc1614IdfI2cWriteRead(uint8_t addr, const uint8_t* txData,
                                       size_t txLen, uint8_t* rxData,
                                       size_t rxLen, uint32_t timeoutMs,
                                       void* user) {
  Ldc1614IdfI2c* ctx = nullptr;
  LDC1614::Status st = validateContext(addr, user, ctx);
  if (!st.ok()) {
    return st;
  }
  if (txData == nullptr || txLen == 0 || (rxLen > 0 && rxData == nullptr)) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM, "Invalid I2C read buffer");
  }

  const esp_err_t err = i2c_master_transmit_receive(
      ctx->dev, txData, txLen, rxData, rxLen, clampTimeoutMs(timeoutMs));
  return mapEspErr(err, "I2C write-read failed");
}

LDC1614::Status ldc1614IdfIntbAsserted(bool& asserted, void* user) {
  auto* ctx = static_cast<Ldc1614IdfI2c*>(user);
  if (ctx == nullptr || ctx->intb == GPIO_NUM_NC) {
    asserted = false;
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG,
                                  "INTB pin not configured");
  }
  asserted = gpio_get_level(ctx->intb) == 0;
  return LDC1614::Status::Ok();
}

Ldc1614IdfProbeResult ldc1614IdfI2cProbeAddress(
    Ldc1614IdfI2c& context, uint8_t address, uint32_t timeoutMs) {
  if (context.bus == nullptr || address == 0U || address >= 0x7FU) {
    return Ldc1614IdfProbeResult::ERROR;
  }
  const esp_err_t err = i2c_master_probe(
      context.bus, address, clampTimeoutMs(timeoutMs));
  if (err == ESP_OK) {
    return Ldc1614IdfProbeResult::ACK;
  }
  if (err == ESP_ERR_NOT_FOUND) {
    return Ldc1614IdfProbeResult::NACK;
  }
  if (err == ESP_ERR_TIMEOUT) {
    return Ldc1614IdfProbeResult::TIMEOUT;
  }
  return Ldc1614IdfProbeResult::ERROR;
}

uint64_t ldc1614IdfUptimeMs() {
  return static_cast<uint64_t>(esp_timer_get_time() / 1000);
}
