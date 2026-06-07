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

uint32_t arduinoNowMs(void*) {
  return millis();
}

void arduinoDelayMs(uint32_t ms, void*) {
  delay(ms);
}

void arduinoYield(void*) {
  yield();
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
  if (error == 5U) {
    return ldc1614_cli::I2cProbeResult::TIMEOUT;
  }
  return ldc1614_cli::I2cProbeResult::ERROR;
}

LDC1614::Config makeDefaultConfig(void*) {
  LDC1614::Config cfg;
  cfg.i2cWrite = transport::wireWrite;
  cfg.i2cWriteRead = transport::wireWriteRead;
  cfg.i2cUser = &Wire;
  cfg.nowMs = [](void*) { return millis(); };
  cfg.cooperativeYield = [](void*) { yield(); };
  cfg.i2cAddress = 0x2A;
  cfg.channelCount = 4;
  cfg.i2cTimeoutMs = board::I2C_TIMEOUT_MS;

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

  if (board::INTB_PIN >= 0) {
    cfg.intbPin = board::INTB_PIN;
    cfg.gpioRead = board::readIntbPin;
  }

  cfg.offlineThreshold = 5;
  return cfg;
}

ldc1614_cli::Cli ldcCli(
    device,
    ldc1614_cli::Cli::Platform{
        nullptr,
        arduinoVPrintf,
        makeDefaultConfig,
        arduinoNowMs,
        arduinoDelayMs,
        arduinoYield,
        arduinoI2cProbe,
        board::I2C_TIMEOUT_MS,
    });

void readCliInput() {
  static char inputBuffer[128];
  static size_t inputLen = 0;

  while (Serial.available() > 0) {
    const char c = static_cast<char>(Serial.read());
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
      inputBuffer[inputLen++] = c;
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

  ldcCli.processCommand("scan");

  const LDC1614::Status st = device.begin(ldcCli.makeDefaultConfig());
  if (!st.ok()) {
    ldcCli.logError("Failed to initialize device");
    ldcCli.printStatus(st);
    ldcCli.processCommand("id");
    ldcCli.processCommand("state");
    ldcCli.logInfo("Type 'begin' or 'init' to retry initialization");
  } else {
    ldcCli.logInfo("Device initialized successfully");
    ldcCli.printDriverHealth();
  }

  ldcCli.println("\nType 'help' for commands");
  ldcCli.printPrompt();
}

void loop() {
  device.tick(ldcCli.nowMs());
  readCliInput();
}
