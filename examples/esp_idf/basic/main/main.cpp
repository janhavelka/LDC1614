#include <cstdint>

#include "LDC1614/LDC1614.h"
#include "Ldc1614IdfI2cTransport.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace {

constexpr char TAG[] = "ldc1614_basic";
constexpr gpio_num_t I2C_SDA = GPIO_NUM_8;
constexpr gpio_num_t I2C_SCL = GPIO_NUM_9;
constexpr gpio_num_t INTB_PIN = GPIO_NUM_NC;
constexpr gpio_num_t SHDN_PIN = GPIO_NUM_NC;
constexpr uint32_t I2C_FREQ_HZ = 400000;
constexpr uint8_t LDC1614_ADDRESS = 0x2A;

}  // namespace

extern "C" void app_main(void) {
  Ldc1614IdfI2c transport{};
  transport.address = LDC1614_ADDRESS;
  transport.intb = INTB_PIN;
  transport.shdn = SHDN_PIN;

  i2c_master_bus_config_t busConfig{};
  busConfig.clk_source = I2C_CLK_SRC_DEFAULT;
  busConfig.i2c_port = I2C_NUM_0;
  busConfig.sda_io_num = I2C_SDA;
  busConfig.scl_io_num = I2C_SCL;
  busConfig.glitch_ignore_cnt = 7;
  busConfig.flags.enable_internal_pullup = true;
  ESP_ERROR_CHECK(i2c_new_master_bus(&busConfig, &transport.bus));

  i2c_device_config_t devConfig{};
  devConfig.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  devConfig.device_address = LDC1614_ADDRESS;
  devConfig.scl_speed_hz = I2C_FREQ_HZ;
  ESP_ERROR_CHECK(i2c_master_bus_add_device(transport.bus, &devConfig, &transport.dev));

  if constexpr (INTB_PIN != GPIO_NUM_NC) {
    gpio_config_t gpioConfig{};
    gpioConfig.pin_bit_mask = 1ULL << static_cast<uint32_t>(INTB_PIN);
    gpioConfig.mode = GPIO_MODE_INPUT;
    gpioConfig.pull_up_en = GPIO_PULLUP_ENABLE;
    gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpioConfig.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&gpioConfig));
  }
  if constexpr (SHDN_PIN != GPIO_NUM_NC) {
    gpio_config_t gpioConfig{};
    gpioConfig.pin_bit_mask = 1ULL << static_cast<uint32_t>(SHDN_PIN);
    gpioConfig.mode = GPIO_MODE_OUTPUT;
    gpioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpioConfig.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&gpioConfig));
    gpio_set_level(SHDN_PIN, 0);
  }

  LDC1614::LDC1614 device;
  LDC1614::Config cfg{};
  cfg.i2cWrite = ldc1614IdfI2cWrite;
  cfg.i2cWriteRead = ldc1614IdfI2cWriteRead;
  cfg.i2cUser = &transport;
  cfg.busReset = ldc1614IdfBusReset;
  if constexpr (SHDN_PIN != GPIO_NUM_NC) {
    cfg.hardReset = ldc1614IdfHardReset;
  }
  cfg.nowMs = ldc1614IdfNowMs;
  cfg.cooperativeYield = ldc1614IdfYield;
  cfg.i2cAddress = LDC1614_ADDRESS;
  cfg.i2cTimeoutMs = 50;
  cfg.channelCount = 4;
  cfg.activeChan = 0;
  cfg.deglitch = LDC1614::Deglitch::BW_10MHZ;
  cfg.sensorActivation = LDC1614::SensorActivation::FULL_CURRENT;
  cfg.channel[0].rcount = 0x04D6;
  cfg.channel[0].settleCount = 0x000A;
  cfg.channel[0].finDivider = 1;
  cfg.channel[0].frefDivider = 1;
  cfg.channel[0].idrive = 10;
  cfg.errorConfig = LDC1614::cmd::MASK_ERRCFG_DRDY_2INT;
  if constexpr (INTB_PIN != GPIO_NUM_NC) {
    cfg.intbPin = static_cast<int>(INTB_PIN);
    cfg.gpioRead = ldc1614IdfGpioRead;
    cfg.gpioUser = &transport;
  }

  LDC1614::Status st = device.begin(cfg);
  if (!st.ok()) {
    ESP_LOGE(TAG, "begin failed: %s (%d detail=%ld)", st.msg, static_cast<int>(st.code),
             static_cast<long>(st.detail));
    return;
  }

  uint16_t manufacturer = 0;
  uint16_t deviceId = 0;
  (void)device.readRegister16(LDC1614::cmd::REG_MANUFACTURER_ID, manufacturer);
  (void)device.readRegister16(LDC1614::cmd::REG_DEVICE_ID, deviceId);
  ESP_LOGI(TAG, "manufacturer=0x%04X device=0x%04X", manufacturer, deviceId);

  st = device.wake();
  if (!st.ok()) {
    ESP_LOGE(TAG, "wake failed: %s (%d detail=%ld)", st.msg, static_cast<int>(st.code),
             static_cast<long>(st.detail));
    return;
  }

  vTaskDelay(pdMS_TO_TICKS(20));

  bool ready = false;
  st = device.readDataReady(ready);
  if (st.ok() && ready) {
    LDC1614::ChannelData data{};
    st = device.readChannel(0, data);
    if (st.ok()) {
      ESP_LOGI(TAG, "ch0 raw=0x%07lX err=%u", static_cast<unsigned long>(data.rawData),
               static_cast<unsigned>(data.hasError()));
    }
  } else if (!st.ok()) {
    ESP_LOGW(TAG, "data-ready check failed: %s (%d detail=%ld)", st.msg,
             static_cast<int>(st.code), static_cast<long>(st.detail));
  } else {
    ESP_LOGI(TAG, "data not ready yet");
  }

  ESP_LOGI(TAG, "state=%u successes=%lu failures=%lu", static_cast<unsigned>(device.state()),
           static_cast<unsigned long>(device.totalSuccess()),
           static_cast<unsigned long>(device.totalFailures()));
}
