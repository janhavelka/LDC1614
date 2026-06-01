/**
 * @file I2cScanner.h
 * @brief Simple I2C bus scanner utility for examples.
 *
 * NOT part of the library API. This is a diagnostic tool for examples.
 */

#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "LDC1614/CommandTable.h"
#include "examples/common/Log.h"

namespace i2c_scanner {

inline bool isLdcAddress(uint8_t addr) {
  return addr == 0x2A || addr == 0x2B;
}

inline bool readRegister16(TwoWire& wire, uint8_t addr, uint8_t reg, uint16_t& value) {
  wire.beginTransmission(addr);
  if (wire.write(&reg, 1) != 1U) {
    return false;
  }

  if (wire.endTransmission(false) != 0U) {
    return false;
  }

  const size_t read = wire.requestFrom(addr, static_cast<uint8_t>(2));
  if (read != 2U) {
    while (wire.available() > 0) {
      (void)wire.read();
    }
    return false;
  }

  if (wire.available() < 2) {
    return false;
  }

  const int msb = wire.read();
  const int lsb = wire.read();
  if (msb < 0 || lsb < 0) {
    return false;
  }

  value = (static_cast<uint16_t>(static_cast<uint8_t>(msb)) << 8) |
          static_cast<uint8_t>(lsb);
  return true;
}

inline bool probeLdcAddress(TwoWire& wire, uint8_t addr) {
  uint16_t manufacturer = 0;
  if (!readRegister16(wire, addr, LDC1614::cmd::REG_MANUFACTURER_ID, manufacturer)) {
    return false;
  }
  if (manufacturer != LDC1614::cmd::MANUFACTURER_ID_VALUE) {
    return false;
  }

  uint16_t device = 0;
  if (!readRegister16(wire, addr, LDC1614::cmd::REG_DEVICE_ID, device)) {
    return false;
  }
  return device == LDC1614::cmd::DEVICE_ID_VALUE;
}

/**
 * @brief Attempt to recover a stuck I2C bus by toggling SCL.
 * @param sda SDA pin number
 * @param scl SCL pin number
 */
inline void recoverBus(int sda, int scl) {
  Wire.end();

  pinMode(scl, OUTPUT);
  pinMode(sda, INPUT_PULLUP);

  for (int i = 0; i < 9; i++) {
    digitalWrite(scl, LOW);
    delayMicroseconds(5);
    digitalWrite(scl, HIGH);
    delayMicroseconds(5);
    if (digitalRead(sda)) {
      break;
    }
  }

  pinMode(sda, OUTPUT);
  digitalWrite(sda, LOW);
  delayMicroseconds(5);
  digitalWrite(scl, HIGH);
  delayMicroseconds(5);
  digitalWrite(sda, HIGH);
  delayMicroseconds(5);

  Wire.begin(sda, scl);
}

/**
 * @brief Scan I2C bus and print found devices.
 * @param wire Reference to Wire object (must be initialized).
 * @param timeoutMs Timeout per address probe in milliseconds (default 50ms).
 */
inline void scan(TwoWire& wire, uint16_t timeoutMs = 50) {
  LOGI("Scanning I2C bus (timeout=%dms)...", timeoutMs);
  LOG_SERIAL.flush();

#if defined(ARDUINO_ARCH_ESP32)
  wire.setTimeOut(timeoutMs);
#endif

  LOGI("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
  LOG_SERIAL.flush();

  uint8_t count = 0;
  for (uint8_t row = 0; row < 8; row++) {
    LOG_SERIAL.printf("%02X: ", row * 16);
    LOG_SERIAL.flush();

    for (uint8_t col = 0; col < 16; col++) {
      uint8_t addr = row * 16 + col;
      if (addr < 0x08 || addr > 0x77) {
        LOG_SERIAL.print("   ");
        continue;
      }

      bool ack = false;
      bool timeout = false;

      if (isLdcAddress(addr)) {
        // LDC1612/LDC1614 does not tolerate early-terminated I2C
        // transactions. Use a complete register read instead of an
        // address-only scan probe for the possible LDC addresses.
        ack = probeLdcAddress(wire, addr);
      } else {
        wire.beginTransmission(addr);
        const uint8_t error = wire.endTransmission(true);
        ack = error == 0U;
        timeout = error == 5U;
      }

      if (ack) {
        LOG_SERIAL.printf("%02X ", addr);
        count++;
      } else if (timeout) {
        LOG_SERIAL.print("TO ");
      } else {
        LOG_SERIAL.print("-- ");
      }

      yield();
      delay(1);
    }
    LOG_SERIAL.println();
    LOG_SERIAL.flush();
  }

  LOGI("Scan complete. Found %d device(s).", count);
  LOG_SERIAL.flush();

  if (count > 0) {
    LOGI("Common addresses: 0x2A/0x2B=LDC1614, 0x48-0x4B=ADS1115, 0x51=RV3032, 0x76/0x77=BME280");
  }
}

}  // namespace i2c_scanner
