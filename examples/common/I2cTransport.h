/**
 * @file I2cTransport.h
 * @brief Wire-based I2C transport adapter for LDC1614 examples.
 *
 * This file provides Wire-compatible I2C callbacks that can be
 * used with the LDC1614 driver. The library does not depend on Wire
 * directly; this adapter bridges them.
 *
 * NOT part of the library API. Example-only.
 */

#pragma once

#include <Arduino.h>
#include <Wire.h>

#if defined(ARDUINO_ARCH_ESP32)
#include <esp_timer.h>
#endif

#include "LDC1614/Status.h"

namespace transport {

/**
 * @brief Wire-based I2C write implementation.
 *
 * Pass to Config::i2cWrite, and pass &Wire (or custom TwoWire*) to i2cUser.
 *
 * @param addr I2C 7-bit address
 * @param data Data buffer to send
 * @param len Number of bytes
 * @param timeoutMs Timeout requested by the driver (bus-manager owned in shared buses)
 * @param user Pointer to TwoWire instance
 * @return Status OK on success, I2C error on failure
 */
inline LDC1614::Status wireWrite(uint8_t addr, const uint8_t* data, size_t len,
                                 uint32_t timeoutMs, void* user) {
  TwoWire* wire = static_cast<TwoWire*>(user);
  if (wire == nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG, "Wire instance is null");
  }
  if (!data || len == 0) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM, "Invalid I2C write params");
  }

  // Check for oversized writes (ESP32 Wire buffer is 128 bytes)
  if (len > 128) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM, "Write exceeds I2C buffer",
                                  static_cast<int32_t>(len));
  }

#if defined(ARDUINO_ARCH_ESP32)
  wire->setTimeOut(timeoutMs);
#else
  (void)timeoutMs;
#endif

  wire->beginTransmission(addr);
  size_t written = wire->write(data, len);
  if (written != len) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_ERROR, "I2C write incomplete",
                                  static_cast<int32_t>(written));
  }

  uint8_t result = wire->endTransmission(true);  // Send STOP
  switch (result) {
    case 0:
      return LDC1614::Status::Ok();
    case 1:
      return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM, "I2C data too long", result);
    case 2:
      return LDC1614::Status::Error(LDC1614::Err::I2C_NACK_ADDR, "I2C address NACK", result);
    case 3:
      return LDC1614::Status::Error(LDC1614::Err::I2C_NACK_DATA, "I2C data NACK", result);
    case 4:
      return LDC1614::Status::Error(LDC1614::Err::I2C_BUS, "I2C bus error", result);
    case 5:
      return LDC1614::Status::Error(LDC1614::Err::I2C_TIMEOUT, "I2C timeout", result);
    default:
      return LDC1614::Status::Error(LDC1614::Err::I2C_ERROR, "I2C unknown error", result);
  }
}

/**
 * @brief Wire-based I2C write-read implementation.
 *
 * Pass to Config::i2cWriteRead, and pass &Wire (or custom TwoWire*) to i2cUser.
 *
 * @param addr I2C 7-bit address
 * @param tx TX buffer to send
 * @param txLen TX length
 * @param rx RX buffer for readback
 * @param rxLen RX length
 * @param timeoutMs Timeout requested by the driver (bus-manager owned in shared buses)
 * @param user Pointer to TwoWire instance
 * @return Status OK on success, I2C error on failure
 *
 * On ESP32 the repeated-start address phase and the read share this single
 * callback timeout; the second phase receives only the remaining time.
 */
inline LDC1614::Status wireWriteRead(uint8_t addr, const uint8_t* tx, size_t txLen,
                                     uint8_t* rx, size_t rxLen, uint32_t timeoutMs,
                                     void* user) {
  TwoWire* wire = static_cast<TwoWire*>(user);
  if (wire == nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG, "Wire instance is null");
  }
  if ((txLen > 0 && tx == nullptr) || (rxLen > 0 && rx == nullptr)) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM, "Invalid I2C read params");
  }
  if (txLen == 0 || rxLen == 0) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM, "I2C read length invalid");
  }
  if (txLen > 128 || rxLen > 128) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM, "I2C read exceeds buffer");
  }

#if defined(ARDUINO_ARCH_ESP32)
  const uint64_t startedAtUs = static_cast<uint64_t>(esp_timer_get_time());
  wire->setTimeOut(timeoutMs);
#else
  (void)timeoutMs;
#endif

  wire->beginTransmission(addr);
  size_t written = wire->write(tx, txLen);
  if (written != txLen) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_ERROR, "I2C write incomplete",
                                  static_cast<int32_t>(written));
  }

  uint8_t result = wire->endTransmission(false);  // Repeated start
  switch (result) {
    case 0:
      break;
    case 1:
      return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM, "I2C data too long", result);
    case 2:
      return LDC1614::Status::Error(LDC1614::Err::I2C_NACK_ADDR, "I2C address NACK", result);
    case 3:
      return LDC1614::Status::Error(LDC1614::Err::I2C_NACK_DATA, "I2C data NACK", result);
    case 4:
      return LDC1614::Status::Error(LDC1614::Err::I2C_BUS, "I2C bus error", result);
    case 5:
      return LDC1614::Status::Error(LDC1614::Err::I2C_TIMEOUT, "I2C timeout", result);
    default:
      return LDC1614::Status::Error(LDC1614::Err::I2C_ERROR, "I2C write failed", result);
  }

#if defined(ARDUINO_ARCH_ESP32)
  // endTransmission(false) and requestFrom() are one injected callback. Give
  // the read only the callback budget that the address phase did not consume.
  const uint64_t budgetUs = static_cast<uint64_t>(timeoutMs) * 1000ULL;
  const uint64_t elapsedUs =
      static_cast<uint64_t>(esp_timer_get_time()) - startedAtUs;
  const uint32_t remainingMs = elapsedUs < budgetUs
                                   ? static_cast<uint32_t>((budgetUs - elapsedUs) / 1000ULL)
                                   : 0U;
  if (remainingMs == 0U) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_TIMEOUT,
                                  "I2C write-read deadline exhausted");
  }
  wire->setTimeOut(remainingMs);
#endif

  size_t read = wire->requestFrom(addr, static_cast<uint8_t>(rxLen));
  if (read != rxLen) {
    return LDC1614::Status::Error(LDC1614::Err::I2C_ERROR, "I2C read length mismatch",
                                  static_cast<int32_t>(read));
  }

  for (size_t i = 0; i < rxLen; ++i) {
    if (wire->available()) {
      rx[i] = static_cast<uint8_t>(wire->read());
    } else {
      return LDC1614::Status::Error(LDC1614::Err::I2C_ERROR, "I2C data not available");
    }
  }

  return LDC1614::Status::Ok();
}

/**
 * @brief Initialize Wire with default pins and frequency.
 *
 * @param sda SDA pin number
 * @param scl SCL pin number
 * @param freq I2C clock frequency in Hz (default 400kHz)
 * @param timeoutMs I2C timeout in milliseconds (default 50ms)
 * @return true on success
 */
inline bool initWire(int sda, int scl, uint32_t freq = 400000, uint16_t timeoutMs = 50) {
#if defined(ARDUINO_ARCH_ESP32)
  // Toggle SCL to release any stuck slave
  pinMode(scl, OUTPUT);
  pinMode(sda, INPUT_PULLUP);
  for (int i = 0; i < 9; i++) {
    digitalWrite(scl, LOW);
    delayMicroseconds(5);
    digitalWrite(scl, HIGH);
    delayMicroseconds(5);
  }
  // Generate STOP condition
  pinMode(sda, OUTPUT);
  digitalWrite(sda, LOW);
  delayMicroseconds(5);
  digitalWrite(scl, HIGH);
  delayMicroseconds(5);
  digitalWrite(sda, HIGH);
  delayMicroseconds(5);
#endif

#if defined(ARDUINO_ARCH_ESP32)
  if (!Wire.begin(sda, scl, freq)) {
    return false;
  }
  Wire.setTimeOut(timeoutMs);
#else
  Wire.begin(sda, scl);
  Wire.setClock(freq);
  (void)timeoutMs;
#endif
  return true;
}

}  // namespace transport
