#include <cstdint>
#include <cstdio>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#include "LDC1614/LDC1614.h"
#include "Ldc1614IdfCli.h"
#include "Ldc1614IdfI2cTransport.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
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

LDC1614::Status statusFromEspErr(esp_err_t err, const char* context) {
  if (err == ESP_OK) {
    return LDC1614::Status::Ok();
  }
  if (err == ESP_ERR_TIMEOUT) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_TIMEOUT, context,
                                  static_cast<int32_t>(err));
  }
  if (err == ESP_ERR_INVALID_ARG) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG, context,
                                  static_cast<int32_t>(err));
  }
  if (err == ESP_ERR_NO_MEM || err == ESP_ERR_NOT_FOUND ||
      err == ESP_ERR_INVALID_STATE) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS, context,
                                  static_cast<int32_t>(err));
  }
  return LDC1614::Status::Error(LDC1614::Err::I2C_ERROR, context,
                                static_cast<int32_t>(err));
}

void printSetupStatus(const char* label, const LDC1614::Status& status) {
  std::printf("%s: code=%u detail=%ld msg=%s\n", label,
              static_cast<unsigned>(status.code), static_cast<long>(status.detail),
              status.msg != nullptr ? status.msg : "");
}

LDC1614::Status configureGpio() {
  if (INTB_PIN != GPIO_NUM_NC) {
    gpio_config_t gpioConfig{};
    gpioConfig.pin_bit_mask = 1ULL << static_cast<uint32_t>(INTB_PIN);
    gpioConfig.mode = GPIO_MODE_INPUT;
    // LDC1612/LDC1614 INTB is push-pull active-low/configurable; do not assume open-drain.
    gpioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpioConfig.intr_type = GPIO_INTR_DISABLE;
    LDC1614::Status status =
        statusFromEspErr(gpio_config(&gpioConfig), "INTB GPIO config failed");
    if (!status.ok()) {
      return status;
    }
  }

  if (SHDN_PIN != GPIO_NUM_NC) {
    gpio_config_t gpioConfig{};
    gpioConfig.pin_bit_mask = 1ULL << static_cast<uint32_t>(SHDN_PIN);
    gpioConfig.mode = GPIO_MODE_OUTPUT;
    gpioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpioConfig.intr_type = GPIO_INTR_DISABLE;
    LDC1614::Status status =
        statusFromEspErr(gpio_config(&gpioConfig), "SHDN GPIO config failed");
    if (!status.ok()) {
      return status;
    }
    status = statusFromEspErr(gpio_set_level(SHDN_PIN, 0), "SHDN release failed");
    if (!status.ok()) {
      return status;
    }
  }

  return LDC1614::Status::Ok();
}

LDC1614::Status configureI2c() {
  app.transport.address = LDC1614_ADDRESS;
  app.transport.lockTimeoutMs = I2C_TIMEOUT_MS;
  app.transport.intb = INTB_PIN;
  app.transport.shdn = SHDN_PIN;
  if (app.transport.mutex == nullptr) {
    app.transport.mutex = xSemaphoreCreateMutex();
    if (app.transport.mutex == nullptr) {
      return LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                                    "IDF I2C mutex allocation failed");
    }
  }

  i2c_master_bus_config_t busConfig{};
  busConfig.clk_source = I2C_CLK_SRC_DEFAULT;
  busConfig.i2c_port = I2C_NUM_0;
  busConfig.sda_io_num = I2C_SDA;
  busConfig.scl_io_num = I2C_SCL;
  busConfig.glitch_ignore_cnt = 7;
  busConfig.flags.enable_internal_pullup = true;
  LDC1614::Status status = statusFromEspErr(
      i2c_new_master_bus(&busConfig, &app.transport.bus), "I2C bus init failed");
  if (!status.ok()) {
    return status;
  }

  i2c_device_config_t devConfig{};
  devConfig.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  devConfig.device_address = LDC1614_ADDRESS;
  devConfig.scl_speed_hz = I2C_FREQ_HZ;
  status = statusFromEspErr(
      i2c_master_bus_add_device(app.transport.bus, &devConfig, &app.transport.dev),
      "I2C device add failed");
  if (!status.ok()) {
    return status;
  }

  return LDC1614::Status::Ok();
}

int readConsoleChar(uint32_t timeoutMs) {
  fd_set readSet;
  FD_ZERO(&readSet);
  FD_SET(STDIN_FILENO, &readSet);

  timeval timeout{};
  timeout.tv_sec = static_cast<time_t>(timeoutMs / 1000U);
  timeout.tv_usec = static_cast<suseconds_t>((timeoutMs % 1000U) * 1000U);

  const int ready =
      select(STDIN_FILENO + 1, &readSet, nullptr, nullptr, &timeout);
  if (ready <= 0 || !FD_ISSET(STDIN_FILENO, &readSet)) {
    return -1;
  }

  uint8_t ch = 0;
  const ssize_t n = read(STDIN_FILENO, &ch, 1);
  if (n == 1) {
    return static_cast<int>(ch);
  }
  return -1;
}

void readCliInputForever(Ldc1614IdfCli& cli) {
  static char inputBuffer[128];
  static size_t inputLen = 0;
  static bool overflow = false;

  while (true) {
    device.tick(cli.nowMs());
    const int c = readConsoleChar(20);
    if (c < 0) {
      vTaskDelay(pdMS_TO_TICKS(1));
      continue;
    }

    if (c == '\b' || c == 0x7F) {
      if (!overflow && inputLen > 0U) {
        inputLen--;
      }
      continue;
    }

    if (c == '\n' || c == '\r') {
      if (overflow) {
        std::printf("input too long\n");
      } else if (inputLen > 0U) {
        inputBuffer[inputLen] = '\0';
        cli.processLine(inputBuffer);
      }
      inputLen = 0;
      overflow = false;
      cli.printPrompt();
      continue;
    }

    if (!overflow && inputLen < sizeof(inputBuffer) - 1U) {
      inputBuffer[inputLen++] = static_cast<char>(c);
    } else {
      overflow = true;
    }
  }
}

}  // namespace

extern "C" void app_main(void) {
  setvbuf(stdin, nullptr, _IONBF, 0);
  setvbuf(stdout, nullptr, _IONBF, 0);

  std::printf("=== LDC1614 ESP-IDF diagnostic bring-up example ===\n");
  const LDC1614::Status i2cStatus = configureI2c();
  printSetupStatus("i2c", i2cStatus);
  const LDC1614::Status gpioStatus = configureGpio();
  printSetupStatus("gpio", gpioStatus);

  Ldc1614IdfCli cli(device, makeDefaultConfig(&app));
  cli.printBanner();

  if (i2cStatus.ok() && gpioStatus.ok()) {
    const LDC1614::Status st = device.begin(makeDefaultConfig(&app));
    if (!st.ok()) {
      cli.println("device initialization failed");
      printSetupStatus("begin", st);
      cli.processLine("probe");
      cli.println("Type 'begin' or 'init' to retry initialization");
    } else {
      cli.println("device initialized successfully");
      cli.processLine("drv");
    }
  } else {
    cli.println("setup failed; fix I2C/GPIO setup before device commands can succeed");
  }

  cli.printPrompt();
  readCliInputForever(cli);
}
