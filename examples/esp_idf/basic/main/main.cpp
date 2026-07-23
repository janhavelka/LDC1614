#include <cstdint>
#include <cstdio>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#include "LDC1614/LDC1614.h"
#include "I2cMasterTransport.h"
#include "Ldc1614IdfCli.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace {

constexpr gpio_num_t I2C_SDA = GPIO_NUM_8;
constexpr gpio_num_t I2C_SCL = GPIO_NUM_9;
constexpr gpio_num_t INTB_PIN = GPIO_NUM_NC;
constexpr uint32_t I2C_FREQ_HZ = 400000;
constexpr uint32_t I2C_TIMEOUT_MS = 50;
constexpr uint8_t LDC1614_ADDRESS = 0x2A;

struct AppContext {
  esp32_i2c::Context transport{};
};

LDC1614::LDC1614 device;
AppContext app{};

LDC1614::Config makeDefaultConfig(void* user) {
  auto* ctx = static_cast<AppContext*>(user);
  LDC1614::Config cfg{};
  cfg.i2cWrite = esp32_i2c::write;
  cfg.i2cWriteRead = esp32_i2c::writeRead;
  cfg.i2cUser = &ctx->transport;
  cfg.i2cTimeoutMs = I2C_TIMEOUT_MS;
  cfg.i2cAddress = LDC1614::I2cAddress::ADDR_GND;
  cfg.variant = LDC1614::DeviceVariant::LDC1614;
  cfg.channels = LDC1614::channelBit(LDC1614::Channel::CH0);
  cfg.referenceClock =
      LDC1614::ReferenceClock{LDC1614::RefClkSrc::INTERNAL, 43000000U, 200000U};
  cfg.mode = LDC1614::OperatingMode::SINGLE_CHANNEL;
  cfg.activeChannel = LDC1614::Channel::CH0;
  cfg.deglitch = LDC1614::Deglitch::BW_10MHZ;
  cfg.sensorActivation = LDC1614::SensorActivation::FULL_CURRENT;
  cfg.rpOverrideEnabled = true;
  cfg.autoAmplitudeCorrectionEnabled = false;
  cfg.highCurrentDriveEnabled = false;
  cfg.intbDisabled = ctx->transport.intb == GPIO_NUM_NC;

  // Initialization writes every physical channel register to a known value,
  // even though this diagnostic profile converts only CH0.
  for (uint8_t channel = 0; channel < 4; ++channel) {
    cfg.channel[channel].rcount = 0x04D6;
    cfg.channel[channel].settleCount = 0x000A;
    cfg.channel[channel].finDivider = 2;
    cfg.channel[channel].frefDivider = 2;
    cfg.channel[channel].offset = 0x0000;
    cfg.channel[channel].driveCurrentCode = 10;
  }
  cfg.channel[0].expectedSensorMinHz = 100000;
  cfg.channel[0].expectedSensorMaxHz = 5000000;
  cfg.errorReporting = LDC1614::ErrorReporting::all();

  if (ctx->transport.intb != GPIO_NUM_NC) {
    cfg.intbAsserted = esp32_i2c::intbAsserted;
    cfg.intbUser = &ctx->transport;
  }
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

LDC1614::Status makeGpioMask(gpio_num_t pin, uint64_t& mask, const char* context) {
  mask = 0;
  if (pin == GPIO_NUM_NC) {
    return LDC1614::Status::Ok();
  }

  const int pinNumber = static_cast<int>(pin);
  if (pinNumber < 0 || pinNumber >= static_cast<int>(GPIO_NUM_MAX) ||
      pinNumber >= 64) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG, context,
                                  static_cast<int32_t>(pinNumber));
  }

  mask = 1ULL << static_cast<uint32_t>(pinNumber);
  return LDC1614::Status::Ok();
}

LDC1614::Status configureGpio() {
  uint64_t intbMask = 0;
  LDC1614::Status status =
      makeGpioMask(INTB_PIN, intbMask, "INTB GPIO pin invalid");
  if (!status.ok()) {
    return status;
  }
  if (intbMask != 0) {
    gpio_config_t gpioConfig{};
    gpioConfig.pin_bit_mask = intbMask;
    gpioConfig.mode = GPIO_MODE_INPUT;
    // LDC1612/LDC1614 INTB is push-pull active-low/configurable; do not assume open-drain.
    gpioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpioConfig.intr_type = GPIO_INTR_DISABLE;
    status = statusFromEspErr(gpio_config(&gpioConfig), "INTB GPIO config failed");
    if (!status.ok()) {
      return status;
    }
  }

  return LDC1614::Status::Ok();
}

LDC1614::Status configureI2c(esp32_i2c::Context& transport) {
  transport.address = LDC1614_ADDRESS;
  transport.intb = INTB_PIN;

  esp32_i2c::BusConfig busConfig{};
  busConfig.port = I2C_NUM_0;
  busConfig.sda = I2C_SDA;
  busConfig.scl = I2C_SCL;
  busConfig.frequencyHz = I2C_FREQ_HZ;
  // Diagnostic convenience only; production hardware needs sized external pull-ups.
  busConfig.enableInternalPullups = true;
  return esp32_i2c::open(transport, busConfig);
}

LDC1614::Status recoverI2c(void* user) {
  auto* context = static_cast<AppContext*>(user);
  if (context == nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG,
                                  "I2C recovery context missing");
  }
  return esp32_i2c::reset(context->transport);
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
    // This diagnostic task is the sole driver owner. One pass consumes at most
    // one driver transport callback.
    cli.service(esp32_i2c::uptimeMs());
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
  const LDC1614::Status i2cStatus = configureI2c(app.transport);
  printSetupStatus("i2c", i2cStatus);
  const LDC1614::Status gpioStatus = configureGpio();
  printSetupStatus("gpio", gpioStatus);

  Ldc1614IdfCli cli(device, makeDefaultConfig(&app), app.transport,
                    recoverI2c, &app);
  cli.printBanner();

  if (i2cStatus.ok() && gpioStatus.ok()) {
    const LDC1614::Status st = device.bind(makeDefaultConfig(&app));
    if (!st.ok()) {
      cli.println("device profile bind failed");
      printSetupStatus("bind", st);
    } else {
      cli.println("profile bound with zero I2C; scheduling initialization");
      cli.service(esp32_i2c::uptimeMs());
      cli.processLine("init");
    }
  } else {
    cli.println("setup failed; fix I2C/GPIO setup before device commands can succeed");
  }

  cli.printPrompt();
  readCliInputForever(cli);
}
