#include <cstdarg>
#include <cstdint>
#include <cstdio>

#include "LDC1614/LDC1614.h"
#include "Ldc1614IdfI2cTransport.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "examples/common/Ldc1614Cli.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace {

constexpr gpio_num_t I2C_SDA = GPIO_NUM_8;
constexpr gpio_num_t I2C_SCL = GPIO_NUM_9;
constexpr gpio_num_t INTB_PIN = GPIO_NUM_NC;
constexpr gpio_num_t SHDN_PIN = GPIO_NUM_NC;
constexpr uint32_t I2C_FREQ_HZ = 400000;
constexpr uint32_t I2C_TIMEOUT_MS = 50;
constexpr uint8_t LDC1614_ADDRESS = 0x2A;

struct AppContext {
  Ldc1614IdfI2c transport{};
};

LDC1614::LDC1614 device;
AppContext app{};

void idfVPrintf(void*, const char* fmt, va_list args) {
  (void)vprintf(fmt, args);
}

uint32_t idfNowMs(void*) {
  return ldc1614IdfNowMs(nullptr);
}

void idfDelayMs(uint32_t ms, void*) {
  vTaskDelay(pdMS_TO_TICKS(ms));
}

void idfYield(void*) {
  ldc1614IdfYield(nullptr);
}

ldc1614_cli::I2cProbeResult idfI2cProbe(uint8_t address, uint32_t timeoutMs, void* user) {
  auto* ctx = static_cast<AppContext*>(user);
  if (ctx == nullptr || ctx->transport.bus == nullptr) {
    return ldc1614_cli::I2cProbeResult::ERROR;
  }
  const esp_err_t err = i2c_master_probe(ctx->transport.bus, address,
                                         static_cast<int>(timeoutMs));
  if (err == ESP_OK) {
    return ldc1614_cli::I2cProbeResult::ACK;
  }
  if (err == ESP_ERR_TIMEOUT) {
    return ldc1614_cli::I2cProbeResult::TIMEOUT;
  }
  return ldc1614_cli::I2cProbeResult::ERROR;
}

LDC1614::Config makeDefaultConfig(void* user) {
  auto* ctx = static_cast<AppContext*>(user);
  LDC1614::Config cfg{};
  cfg.i2cWrite = ldc1614IdfI2cWrite;
  cfg.i2cWriteRead = ldc1614IdfI2cWriteRead;
  cfg.i2cUser = &ctx->transport;
  cfg.busReset = ldc1614IdfBusReset;
  if (ctx->transport.shdn != GPIO_NUM_NC) {
    cfg.hardReset = ldc1614IdfHardReset;
  }
  cfg.nowMs = ldc1614IdfNowMs;
  cfg.cooperativeYield = ldc1614IdfYield;
  cfg.i2cAddress = LDC1614_ADDRESS;
  cfg.i2cTimeoutMs = I2C_TIMEOUT_MS;
  cfg.channelCount = 4;

  cfg.autoScan = false;
  cfg.activeChan = 0;
  cfg.deglitch = LDC1614::Deglitch::BW_10MHZ;
  cfg.refClkSrc = LDC1614::RefClkSrc::INTERNAL;
  cfg.rpOverrideEn = true;
  cfg.autoAmpDis = true;
  cfg.sensorActivation = LDC1614::SensorActivation::FULL_CURRENT;

  cfg.channel[0].rcount = 0x04D6;
  cfg.channel[0].settleCount = 0x000A;
  cfg.channel[0].finDivider = 1;
  cfg.channel[0].frefDivider = 1;
  cfg.channel[0].offset = 0x0000;
  cfg.channel[0].idrive = 10;

  cfg.errorConfig = LDC1614::cmd::MASK_ERRCFG_DRDY_2INT |
                    LDC1614::cmd::MASK_ERRCFG_UR_ERR2INT |
                    LDC1614::cmd::MASK_ERRCFG_OR_ERR2INT |
                    LDC1614::cmd::MASK_ERRCFG_WD_ERR2INT |
                    LDC1614::cmd::MASK_ERRCFG_AH_ERR2INT |
                    LDC1614::cmd::MASK_ERRCFG_AL_ERR2INT;

  if (ctx->transport.intb != GPIO_NUM_NC) {
    cfg.intbPin = static_cast<int>(ctx->transport.intb);
    cfg.gpioRead = ldc1614IdfGpioRead;
    cfg.gpioUser = &ctx->transport;
  }

  cfg.offlineThreshold = 5;
  return cfg;
}

ldc1614_cli::Cli ldcCli(
    device,
    ldc1614_cli::Cli::Platform{
        &app,
        idfVPrintf,
        makeDefaultConfig,
        idfNowMs,
        idfDelayMs,
        idfYield,
        idfI2cProbe,
        I2C_TIMEOUT_MS,
    });

void configureGpio() {
  if (INTB_PIN != GPIO_NUM_NC) {
    gpio_config_t gpioConfig{};
    gpioConfig.pin_bit_mask = 1ULL << static_cast<uint32_t>(INTB_PIN);
    gpioConfig.mode = GPIO_MODE_INPUT;
    gpioConfig.pull_up_en = GPIO_PULLUP_ENABLE;
    gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpioConfig.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&gpioConfig));
  }

  if (SHDN_PIN != GPIO_NUM_NC) {
    gpio_config_t gpioConfig{};
    gpioConfig.pin_bit_mask = 1ULL << static_cast<uint32_t>(SHDN_PIN);
    gpioConfig.mode = GPIO_MODE_OUTPUT;
    gpioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpioConfig.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&gpioConfig));
    gpio_set_level(SHDN_PIN, 0);
  }
}

void configureI2c() {
  app.transport.address = LDC1614_ADDRESS;
  app.transport.intb = INTB_PIN;
  app.transport.shdn = SHDN_PIN;

  i2c_master_bus_config_t busConfig{};
  busConfig.clk_source = I2C_CLK_SRC_DEFAULT;
  busConfig.i2c_port = I2C_NUM_0;
  busConfig.sda_io_num = I2C_SDA;
  busConfig.scl_io_num = I2C_SCL;
  busConfig.glitch_ignore_cnt = 7;
  busConfig.flags.enable_internal_pullup = true;
  ESP_ERROR_CHECK(i2c_new_master_bus(&busConfig, &app.transport.bus));

  i2c_device_config_t devConfig{};
  devConfig.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  devConfig.device_address = LDC1614_ADDRESS;
  devConfig.scl_speed_hz = I2C_FREQ_HZ;
  ESP_ERROR_CHECK(i2c_master_bus_add_device(app.transport.bus, &devConfig,
                                            &app.transport.dev));
}

void readCliInputForever() {
  static char inputBuffer[128];
  static size_t inputLen = 0;

  while (true) {
    device.tick(ldcCli.nowMs());
    const int c = getchar();
    if (c == EOF) {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    if (c == '\b' || c == 0x7F) {
      if (inputLen > 0U) {
        inputLen--;
      }
      continue;
    }

    if (c == '\n' || c == '\r') {
      if (inputLen > 0U) {
        inputBuffer[inputLen] = '\0';
        ldcCli.processCommand(inputBuffer);
        inputLen = 0;
        ldcCli.printPrompt();
      }
      continue;
    }

    if (inputLen < sizeof(inputBuffer) - 1U) {
      inputBuffer[inputLen++] = static_cast<char>(c);
    }
  }
}

}  // namespace

extern "C" void app_main(void) {
  setvbuf(stdin, nullptr, _IONBF, 0);
  setvbuf(stdout, nullptr, _IONBF, 0);

  ldcCli.logInfo("=== LDC1614 Bringup Example ===");
  configureI2c();
  ldcCli.logInfo("I2C initialized (SDA=%d, SCL=%d)", static_cast<int>(I2C_SDA),
                 static_cast<int>(I2C_SCL));
  configureGpio();

  ldcCli.processCommand("scan");

  const LDC1614::Status st = device.begin(ldcCli.makeDefaultConfig());
  if (!st.ok()) {
    ldcCli.logError("Failed to initialize device");
    ldcCli.printStatus(st);
    ldcCli.logInfo("Type 'begin' or 'init' to retry initialization");
  } else {
    ldcCli.logInfo("Device initialized successfully");
    ldcCli.printDriverHealth();
  }

  ldcCli.println("\nType 'help' for commands");
  ldcCli.printPrompt();
  readCliInputForever();
}
