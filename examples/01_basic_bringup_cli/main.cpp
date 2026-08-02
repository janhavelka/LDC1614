/// @file main.cpp
/// @brief LDC1614 Arduino diagnostic bring-up CLI example.
/// @note This is diagnostic firmware, not a production bus manager or hardware
/// validation claim. It is not part of the library core.

#include <Arduino.h>

#include <cstdarg>
#include <cstdio>

#include "examples/common/BoardConfig.h"
#include "examples/common/Ldc1614Cli.h"
#include "examples/esp32/I2cMasterTransport.h"

namespace {

LDC1614::LDC1614 device;
esp32_i2c::Context i2cContext{};

void arduinoVPrintf(void*, const char* fmt, va_list args) {
  char buffer[512];
  const int written = vsnprintf(buffer, sizeof(buffer), fmt, args);
  if (written <= 0) {
    return;
  }
  buffer[sizeof(buffer) - 1U] = '\0';
  Serial.print(buffer);
}

uint64_t arduinoNowMs(void*) {
  return esp32_i2c::uptimeMs();
}

LDC1614::Status configureI2c(esp32_i2c::Context& context) {
  context.address = board::LDC_I2C_ADDRESS;
  context.intb = board::INTB_PIN < 0
                     ? GPIO_NUM_NC
                     : static_cast<gpio_num_t>(board::INTB_PIN);
  esp32_i2c::BusConfig config{};
  config.port = I2C_NUM_0;
  config.sda = static_cast<gpio_num_t>(board::I2C_SDA);
  config.scl = static_cast<gpio_num_t>(board::I2C_SCL);
  config.frequencyHz = board::I2C_FREQ_HZ;
  // Diagnostic convenience only; production hardware needs sized pull-ups.
  config.enableInternalPullups = true;
  return esp32_i2c::open(context, config);
}

ldc1614_cli::I2cProbeResult arduinoI2cProbe(uint8_t address,
                                            uint32_t timeoutMs, void* user) {
  auto* context = static_cast<esp32_i2c::Context*>(user);
  if (context == nullptr) {
    return ldc1614_cli::I2cProbeResult::ERROR;
  }
  switch (esp32_i2c::probe(*context, address, timeoutMs)) {
    case esp32_i2c::ProbeResult::ACK:
      return ldc1614_cli::I2cProbeResult::ACK;
    case esp32_i2c::ProbeResult::NACK:
      return ldc1614_cli::I2cProbeResult::NACK;
    case esp32_i2c::ProbeResult::TIMEOUT:
      return ldc1614_cli::I2cProbeResult::TIMEOUT;
    case esp32_i2c::ProbeResult::ERROR:
      return ldc1614_cli::I2cProbeResult::ERROR;
  }
  return ldc1614_cli::I2cProbeResult::ERROR;
}

LDC1614::Status arduinoI2cRecover(void* user) {
  auto* context = static_cast<esp32_i2c::Context*>(user);
  if (context == nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG,
                                  "I2C recovery context missing");
  }
  return esp32_i2c::reset(*context);
}

LDC1614::Config makeDefaultConfig(void* user) {
  auto* context = static_cast<esp32_i2c::Context*>(user);
  LDC1614::Config cfg;
  cfg.i2cWrite = esp32_i2c::write;
  cfg.i2cWriteRead = esp32_i2c::writeRead;
  cfg.i2cUser = context;
  cfg.i2cTimeoutMs = board::I2C_TIMEOUT_MS;
  cfg.i2cAddress = board::LDC_I2C_ADDRESS == 0x2B
                       ? LDC1614::I2cAddress::ADDR_VDD
                       : LDC1614::I2cAddress::ADDR_GND;
  cfg.variant = board::LDC_CHANNEL_COUNT == 2
                    ? LDC1614::DeviceVariant::LDC1612
                    : LDC1614::DeviceVariant::LDC1614;
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
  cfg.intbDisabled = board::INTB_PIN < 0;

  // Initialization writes every physical channel register to a known value,
  // even though this diagnostic profile converts only CH0.
  for (uint8_t channel = 0; channel < board::LDC_CHANNEL_COUNT; ++channel) {
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

  if (board::INTB_PIN >= 0) {
    cfg.intbAsserted = esp32_i2c::intbAsserted;
    cfg.intbUser = context;
  }
  return cfg;
}

ldc1614_cli::Cli::Platform makeCliPlatform() {
  ldc1614_cli::Cli::Platform platform;
  platform.user = &i2cContext;
  platform.vprintf = arduinoVPrintf;
  platform.makeConfig = makeDefaultConfig;
  platform.nowMs = arduinoNowMs;
  platform.i2cProbe = arduinoI2cProbe;
  platform.i2cRecover = arduinoI2cRecover;
  platform.scanTimeoutMs = board::I2C_TIMEOUT_MS;
  return platform;
}

ldc1614_cli::Cli ldcCli(device, makeCliPlatform());

void readCliInput() {
  static char inputBuffer[128];
  static size_t inputLen = 0;
  static bool overflow = false;

  while (Serial.available() > 0) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\b' || c == 0x7F) {
      if (!overflow && inputLen > 0U) {
        inputLen--;
      }
      continue;
    }
    if (c == '\n' || c == '\r') {
      if (overflow) {
        inputLen = 0;
        overflow = false;
        ldcCli.println("input too long");
        ldcCli.printPrompt();
      } else if (inputLen > 0U) {
        inputBuffer[inputLen] = '\0';
        ldcCli.processCommand(inputBuffer);
        inputLen = 0;
        ldcCli.printPrompt();
      }
      continue;
    }
    if (overflow) {
      continue;
    }
    if (inputLen < sizeof(inputBuffer) - 1U) {
      inputBuffer[inputLen++] = c;
    } else {
      inputLen = 0;
      overflow = true;
    }
  }
}

}  // namespace

void setup() {
  board::initSerial();
  delay(100);

  ldcCli.logInfo("=== LDC1614 Arduino Diagnostic Bring-up Example ===");
  ldcCli.logInfo("Diagnostic firmware only; production apps own bus policy and validation.");

  const LDC1614::Status i2cStatus = configureI2c(i2cContext);
  if (!i2cStatus.ok()) {
    ldcCli.logError("Failed to initialize I2C");
    ldcCli.printStatus(i2cStatus);
    return;
  }
  ldcCli.logInfo("I2C initialized (SDA=%d, SCL=%d)", board::I2C_SDA, board::I2C_SCL);

  board::initIntbPin();

  const LDC1614::Status st = device.bind(ldcCli.makeDefaultConfig());
  if (!st.ok()) {
    ldcCli.logError("Failed to bind explicit device profile");
    ldcCli.printStatus(st);
  } else {
    ldcCli.logInfo("Profile bound with zero I2C; scheduling cooperative initialization");
    ldcCli.processCommand("init");
  }

  ldcCli.println("\nType 'help' for commands");
  ldcCli.printPrompt();
}

void loop() {
  // This diagnostic application is the single owner. Each pass advances at
  // most one transport callback and prints terminal results exactly once.
  ldcCli.service();
  readCliInput();
}
