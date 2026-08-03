/// @file main.cpp
/// @brief Native ESP-IDF LDC1614 diagnostic CLI application.

#include <cstdarg>
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

LDC1614::LDC1614 device;
esp32_i2c::Context i2cContext{};

void idfVPrintf(void*, const char* format, va_list args) {
  std::vprintf(format, args);
}

uint64_t idfNowMs(void*) { return esp32_i2c::uptimeMs(); }

LDC1614::Status statusFromEspErr(esp_err_t err, const char* context) {
  if (err == ESP_OK) return LDC1614::Status::Ok();
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

LDC1614::Status configureI2c(esp32_i2c::Context& context) {
  context.address = LDC1614_ADDRESS;
  context.intb = INTB_PIN;
  esp32_i2c::BusConfig config{};
  config.port = I2C_NUM_0;
  config.sda = I2C_SDA;
  config.scl = I2C_SCL;
  config.frequencyHz = I2C_FREQ_HZ;
  // Diagnostic convenience only; production hardware needs sized pull-ups.
  config.enableInternalPullups = true;
  return esp32_i2c::open(context, config);
}

LDC1614::Status configureGpio() {
  if (INTB_PIN == GPIO_NUM_NC) return LDC1614::Status::Ok();
  const int pin = static_cast<int>(INTB_PIN);
  if (pin < 0 || pin >= static_cast<int>(GPIO_NUM_MAX) || pin >= 64) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG,
                                  "INTB GPIO pin invalid", pin);
  }
  gpio_config_t config{};
  config.pin_bit_mask = 1ULL << static_cast<uint32_t>(pin);
  config.mode = GPIO_MODE_INPUT;
  // LDC1612/LDC1614 INTB is push-pull active-low/configurable.
  config.pull_up_en = GPIO_PULLUP_DISABLE;
  config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  config.intr_type = GPIO_INTR_DISABLE;
  return statusFromEspErr(gpio_config(&config), "INTB GPIO config failed");
}

ldc1614_idf_cli::I2cProbeResult idfI2cProbe(uint8_t address,
                                            uint32_t timeoutMs, void* user) {
  auto* context = static_cast<esp32_i2c::Context*>(user);
  if (context == nullptr) return ldc1614_idf_cli::I2cProbeResult::ERROR;
  switch (esp32_i2c::probe(*context, address, timeoutMs)) {
    case esp32_i2c::ProbeResult::ACK:
      return ldc1614_idf_cli::I2cProbeResult::ACK;
    case esp32_i2c::ProbeResult::NACK:
      return ldc1614_idf_cli::I2cProbeResult::NACK;
    case esp32_i2c::ProbeResult::TIMEOUT:
      return ldc1614_idf_cli::I2cProbeResult::TIMEOUT;
    case esp32_i2c::ProbeResult::ERROR:
      return ldc1614_idf_cli::I2cProbeResult::ERROR;
  }
  return ldc1614_idf_cli::I2cProbeResult::ERROR;
}

LDC1614::Status idfI2cRecover(void* user) {
  auto* context = static_cast<esp32_i2c::Context*>(user);
  if (context == nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG,
                                  "I2C recovery context missing");
  }
  return esp32_i2c::reset(*context);
}

LDC1614::Config makeDefaultConfig(void* user) {
  auto* context = static_cast<esp32_i2c::Context*>(user);
  LDC1614::Config config{};
  config.i2cWrite = esp32_i2c::write;
  config.i2cWriteRead = esp32_i2c::writeRead;
  config.i2cUser = context;
  config.i2cTimeoutMs = I2C_TIMEOUT_MS;
  config.i2cAddress = LDC1614_ADDRESS == 0x2B
                          ? LDC1614::I2cAddress::ADDR_VDD
                          : LDC1614::I2cAddress::ADDR_GND;
  config.variant = LDC1614::DeviceVariant::LDC1614;
  config.channels = LDC1614::channelBit(LDC1614::Channel::CH0);
  config.referenceClock =
      LDC1614::ReferenceClock{LDC1614::RefClkSrc::INTERNAL, 43000000U, 200000U};
  config.mode = LDC1614::OperatingMode::SINGLE_CHANNEL;
  config.activeChannel = LDC1614::Channel::CH0;
  config.deglitch = LDC1614::Deglitch::BW_10MHZ;
  config.sensorActivation = LDC1614::SensorActivation::FULL_CURRENT;
  config.rpOverrideEnabled = true;
  config.autoAmplitudeCorrectionEnabled = false;
  config.highCurrentDriveEnabled = false;
  config.intbDisabled = INTB_PIN == GPIO_NUM_NC;

  // Initialization replays every physical channel, not only selected CH0.
  for (uint8_t channel = 0; channel < 4; ++channel) {
    config.channel[channel].rcount = 0x04D6;
    config.channel[channel].settleCount = 0x000A;
    config.channel[channel].finDivider = 2;
    config.channel[channel].frefDivider = 2;
    config.channel[channel].offset = 0;
    config.channel[channel].driveCurrentCode = 10;
  }
  config.channel[0].expectedSensorMinHz = 100000;
  config.channel[0].expectedSensorMaxHz = 5000000;
  config.errorReporting = LDC1614::ErrorReporting::all();
  if (INTB_PIN != GPIO_NUM_NC) {
    config.intbAsserted = esp32_i2c::intbAsserted;
    config.intbUser = context;
  }
  return config;
}

ldc1614_idf_cli::Ldc1614IdfCli::Platform makeCliPlatform() {
  ldc1614_idf_cli::Ldc1614IdfCli::Platform platform{};
  platform.user = &i2cContext;
  platform.vprintf = idfVPrintf;
  platform.makeConfig = makeDefaultConfig;
  platform.nowMs = idfNowMs;
  platform.i2cProbe = idfI2cProbe;
  platform.i2cRecover = idfI2cRecover;
  // No SD callbacks: the fixed profile has no application-owned SD wiring.
  platform.scanTimeoutMs = I2C_TIMEOUT_MS;
  return platform;
}

ldc1614_idf_cli::Ldc1614IdfCli cli(device, makeCliPlatform());

void emitPrompt(ldc1614_idf_cli::PromptAction action) {
  if (action == ldc1614_idf_cli::PromptAction::PRINT) cli.printPrompt();
}

int readConsoleChar(uint32_t timeoutMs) {
  fd_set readSet;
  FD_ZERO(&readSet);
  FD_SET(STDIN_FILENO, &readSet);
  timeval timeout{};
  timeout.tv_sec = static_cast<time_t>(timeoutMs / 1000U);
  timeout.tv_usec = static_cast<suseconds_t>((timeoutMs % 1000U) * 1000U);
  const int ready = select(STDIN_FILENO + 1, &readSet, nullptr, nullptr, &timeout);
  if (ready <= 0 || !FD_ISSET(STDIN_FILENO, &readSet)) return -1;
  uint8_t character = 0;
  return read(STDIN_FILENO, &character, 1) == 1
             ? static_cast<int>(character)
             : -1;
}

// Consumes a fixed byte budget and completes at most one line per owner pass.
// A completed command skips service() in that pass, preserving the one-
// transfer maximum even for immediate diagnostic commands.
bool readCliInputPass() {
  static constexpr uint8_t INPUT_CHARACTERS_PER_PASS = 32;
  static char inputBuffer[128];
  static size_t inputLength = 0;
  static bool overflow = false;
  uint8_t consumed = 0;
  while (consumed < INPUT_CHARACTERS_PER_PASS) {
    const int value = readConsoleChar(0);
    if (value < 0) return false;
    ++consumed;
    const char character = static_cast<char>(value);
    if (character == '\b' || character == 0x7F) {
      if (!overflow && inputLength > 0U) --inputLength;
      continue;
    }
    if (character == '\n' || character == '\r') {
      if (overflow) {
        inputLength = 0;
        overflow = false;
        cli.println("input too long");
        if (!cli.asynchronousWorkActive()) cli.printPrompt();
        return true;
      }
      if (inputLength == 0U) continue;
      inputBuffer[inputLength] = '\0';
      inputLength = 0;
      emitPrompt(cli.processCommand(inputBuffer));
      return true;
    }
    if (overflow) continue;
    if (inputLength < sizeof(inputBuffer) - 1U) {
      inputBuffer[inputLength++] = character;
    } else {
      inputLength = 0;
      overflow = true;
    }
  }
  return false;
}

void runCliForever() {
  while (true) {
    if (!readCliInputPass()) emitPrompt(cli.service());
    vTaskDelay(1);
  }
}

}  // namespace

extern "C" void app_main(void) {
  setvbuf(stdin, nullptr, _IONBF, 0);
  setvbuf(stdout, nullptr, _IONBF, 0);

  cli.logInfo("=== LDC1614 Native ESP-IDF Diagnostic Bring-up Example ===");
  cli.logInfo("Diagnostic firmware only; production apps own bus policy and validation.");
  const LDC1614::Status i2cStatus = configureI2c(i2cContext);
  const LDC1614::Status gpioStatus = configureGpio();
  if (!i2cStatus.ok() || !gpioStatus.ok()) {
    cli.logError("Platform setup failed");
    cli.printStatus(!i2cStatus.ok() ? i2cStatus : gpioStatus);
    cli.println("\nType 'help' for commands");
    cli.printPrompt();
    runCliForever();
    return;
  }

  const LDC1614::Status bindStatus = device.bind(cli.makeDefaultConfig());
  if (!bindStatus.ok()) {
    cli.logError("Failed to bind explicit device profile");
    cli.printStatus(bindStatus);
    cli.println("\nType 'help' for commands");
    cli.printPrompt();
  } else {
    cli.logInfo("Profile bound with zero I2C; scheduling cooperative initialization");
    cli.println("\nType 'help' for commands");
    emitPrompt(cli.processCommand("init"));
  }
  runCliForever();
}
