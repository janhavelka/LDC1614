/// @file main.cpp
/// @brief LDC1614 Arduino diagnostic bring-up CLI example.
/// @note This is diagnostic firmware, not a production bus manager or hardware
/// validation claim. It is not part of the library core.

#include <Arduino.h>
#include <Wire.h>

#include <cstdarg>
#include <cstdio>

#include "examples/common/BoardConfig.h"
#include "examples/common/Ldc1614Cli.h"

namespace {

LDC1614::LDC1614 device;

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
  // Extend the framework's wrapping 32-bit counter for operation deadlines.
  static uint32_t previous = 0;
  static uint64_t high = 0;
  const uint32_t current = static_cast<uint32_t>(millis());
  if (current < previous) {
    high += (UINT64_C(1) << 32U);
  }
  previous = current;
  return high | current;
}

ldc1614_cli::I2cProbeResult arduinoI2cProbe(uint8_t address, uint32_t timeoutMs, void*) {
#if defined(ARDUINO_ARCH_ESP32)
  Wire.setTimeOut(timeoutMs);
#else
  (void)timeoutMs;
#endif
  Wire.beginTransmission(address);
  const uint8_t error = Wire.endTransmission(true);
  if (error == 0U) {
    return ldc1614_cli::I2cProbeResult::ACK;
  }
  if (error == 2U || error == 3U) {
    return ldc1614_cli::I2cProbeResult::NACK;
  }
  if (error == 5U) {
    return ldc1614_cli::I2cProbeResult::TIMEOUT;
  }
  return ldc1614_cli::I2cProbeResult::ERROR;
}

LDC1614::Status arduinoI2cRecover(void*) {
  if (!Wire.end()) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                                  "I2C bus shutdown failed");
  }
  if (!board::initI2c()) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                                  "I2C bus reinitialization failed");
  }
  return LDC1614::Status::Ok();
}

LDC1614::Config makeDefaultConfig(void*) {
  LDC1614::Config cfg;
  cfg.i2cWrite = transport::wireWrite;
  cfg.i2cWriteRead = transport::wireWriteRead;
  cfg.i2cUser = &Wire;
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
    cfg.intbAsserted = board::readIntbAsserted;
  }
  return cfg;
}

ldc1614_cli::Cli::Platform makeCliPlatform() {
  ldc1614_cli::Cli::Platform platform;
  platform.user = nullptr;
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

  if (!board::initI2c()) {
    ldcCli.logError("Failed to initialize I2C");
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
