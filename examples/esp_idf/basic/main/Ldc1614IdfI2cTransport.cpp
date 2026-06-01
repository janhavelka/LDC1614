#include "Ldc1614IdfI2cTransport.h"

#include <limits>

#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
    return LDC1614::Status::Error(LDC1614::Err::I2C_ERROR, "I2C invalid response/NACK",
                                  static_cast<int32_t>(err));
  }
  return LDC1614::Status::Error(LDC1614::Err::I2C_BUS, context, static_cast<int32_t>(err));
}

LDC1614::Status validateContext(uint8_t addr, const void* user, const Ldc1614IdfI2c*& ctx) {
  ctx = static_cast<const Ldc1614IdfI2c*>(user);
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
  const Ldc1614IdfI2c* ctx = nullptr;
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
  const Ldc1614IdfI2c* ctx = nullptr;
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

bool ldc1614IdfGpioRead(int pin, void*) {
  return gpio_get_level(static_cast<gpio_num_t>(pin)) != 0;
}

uint32_t ldc1614IdfNowMs(void*) {
  return static_cast<uint32_t>(esp_timer_get_time() / 1000);
}

void ldc1614IdfYield(void*) {
  taskYIELD();
}

LDC1614::Status ldc1614IdfBusReset(void* user) {
  const auto* ctx = static_cast<const Ldc1614IdfI2c*>(user);
  if (ctx == nullptr || ctx->bus == nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS, "IDF I2C bus not configured");
  }
  return mapEspErr(i2c_master_bus_reset(ctx->bus), "I2C bus reset failed");
}

LDC1614::Status ldc1614IdfHardReset(void* user) {
  const auto* ctx = static_cast<const Ldc1614IdfI2c*>(user);
  if (ctx == nullptr || ctx->shdn == GPIO_NUM_NC) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG, "SHDN pin not configured");
  }

  gpio_set_level(ctx->shdn, 1);
  vTaskDelay(pdMS_TO_TICKS(2));
  gpio_set_level(ctx->shdn, 0);
  vTaskDelay(pdMS_TO_TICKS(5));
  return LDC1614::Status::Ok();
}
