/// @file Config.h
/// @brief Configuration structure for LDC1614 driver
#pragma once

#include <cstddef>
#include <cstdint>
#include "LDC1614/Status.h"

namespace LDC1614 {

/// @brief I2C write callback signature.
/// @param addr     I2C device address (7-bit)
/// @param data     Pointer to data to write
/// @param len      Number of bytes to write
/// @param timeoutMs Maximum time to wait for completion
/// @param user     User context pointer passed through from Config
/// @return Status indicating success or failure
using I2cWriteFn = Status (*)(uint8_t addr, const uint8_t* data, size_t len,
                              uint32_t timeoutMs, void* user);

/// @brief I2C write-then-read callback signature.
/// @param addr     I2C device address (7-bit)
/// @param txData   Pointer to data to write
/// @param txLen    Number of bytes to write
/// @param rxData   Pointer to buffer for read data
/// @param rxLen    Number of bytes to read
/// @param timeoutMs Maximum time to wait for completion
/// @param user     User context pointer passed through from Config
/// @return Status indicating success or failure
using I2cWriteReadFn = Status (*)(uint8_t addr, const uint8_t* txData, size_t txLen,
                                  uint8_t* rxData, size_t rxLen, uint32_t timeoutMs,
                                  void* user);

/// @brief GPIO read callback signature for the INTB pin.
/// @param pin      GPIO pin number
/// @param user     User context pointer passed through from Config
/// @return true if pin level is HIGH, false if LOW
using GpioReadFn = bool (*)(int pin, void* user);

/// @brief Optional monotonic millisecond timestamp callback.
/// @param user User context pointer passed through from Config
/// @return Current monotonic milliseconds
/// @note Framework-neutral builds do not call platform time APIs; if unset,
/// health timestamps use 0 and blocking helpers cannot advance from wall time.
using NowMsFn = uint32_t (*)(void* user);

/// @brief Optional cooperative yield callback.
/// @param user User context pointer passed through from Config
/// @note If unset, no scheduler/yield API is called by the driver core.
using YieldFn = void (*)(void* user);

/// @brief Optional bus reset callback (for example, SCL pulse recovery).
/// Called by recover() if provided and recoverUseBusReset is true.
/// @param user User context pointer (Config::i2cUser)
/// @return Status indicating success or failure
using BusResetFn = Status (*)(void* user);

/// @brief Optional hard reset callback (for example, SHDN pin toggle).
/// Called by recover() if provided and recoverUseHardReset is true.
/// @param user User context pointer (Config::i2cUser)
/// @return Status indicating success or failure
using HardResetFn = Status (*)(void* user);

/// @brief Reference clock source.
enum class RefClkSrc : uint8_t {
  INTERNAL = 0,   ///< Internal oscillator (~43 MHz)
  EXT_CLK = 1     ///< External CLKIN (2-40 MHz)
};

/// @brief Sensor activation mode.
enum class SensorActivation : uint8_t {
  FULL_CURRENT = 0,   ///< Full current activation (max drive during settle, faster)
  LOW_POWER    = 1    ///< Low power activation (uses IDRIVEx during settle)
};

/// @brief Deglitch filter bandwidth.
enum class Deglitch : uint8_t {
  BW_1MHZ  = 1,   ///< 1.0 MHz
  BW_3MHZ  = 4,   ///< 3.3 MHz
  BW_10MHZ = 5,   ///< 10 MHz
  BW_33MHZ = 7    ///< 33 MHz
};

/// @brief Round-robin channel sequence for multi-channel auto-scan mode.
enum class RRSequence : uint8_t {
  CH0_CH1          = 0,   ///< Ch0, Ch1
  CH0_CH1_CH2      = 1,   ///< Ch0, Ch1, Ch2 (LDC1614 only)
  CH0_CH1_CH2_CH3  = 2    ///< Ch0, Ch1, Ch2, Ch3 (LDC1614 only)
};

/// @brief Per-channel configuration.
struct ChannelConfig {
  uint16_t rcount = 0x0080;          ///< Reference count (0x0005-0xFFFF). Higher = longer conversion, more resolution.
  uint16_t settleCount = 0x0000;     ///< Settling reference count. 0 = minimum (32/fREF).
  uint8_t  finDivider = 1;           ///< Sensor frequency divider (1-15). Must be >=2 if fSensor >= 8.75 MHz.
  uint16_t frefDivider = 1;          ///< Reference clock divider (1-1023).
  uint16_t offset = 0x0000;          ///< Conversion offset (subtracted from measurement).
  uint8_t  idrive = 0;              ///< Sensor drive current index (0-31). See datasheet Table 11.
};

/// @brief Configuration for LDC1614 driver.
struct Config {
  // === I2C Transport (required) ===
  I2cWriteFn i2cWrite = nullptr;
  I2cWriteReadFn i2cWriteRead = nullptr;
  void* i2cUser = nullptr;
  BusResetFn busReset = nullptr;           ///< Optional bus reset callback (SCL recovery)
  HardResetFn hardReset = nullptr;         ///< Optional hard reset callback (SHDN pin toggle)

  // === Timing Hooks (optional) ===
  NowMsFn nowMs = nullptr;                 ///< Optional monotonic millisecond source
  YieldFn cooperativeYield = nullptr;      ///< Optional cooperative scheduler hint
  void* timeUser = nullptr;                ///< User context for timing hooks

  // === Device Settings ===
  uint8_t i2cAddress = 0x2A;       ///< 0x2A (ADDR=GND) or 0x2B (ADDR=VDD)
  uint32_t i2cTimeoutMs = 50;      ///< I2C transaction timeout in ms

  // === Number of channels ===
  uint8_t channelCount = 4;        ///< 2 for LDC1612, 4 for LDC1614

  // === Per-Channel Configuration ===
  ChannelConfig channel[4] = {};    ///< Channel 0-3 configuration

  // === Global Conversion Settings ===
  bool autoScan = false;                        ///< true = multi-channel sequential, false = single-channel
  uint8_t activeChan = 0;                       ///< Active channel for single-channel mode (0-3)
  RRSequence rrSequence = RRSequence::CH0_CH1;  ///< Round-robin sequence for auto-scan mode (LDC1612: CH0_CH1 only)
  Deglitch deglitch = Deglitch::BW_33MHZ;       ///< Input deglitch filter bandwidth

  // === CONFIG Register Settings ===
  RefClkSrc refClkSrc = RefClkSrc::INTERNAL;              ///< Reference clock source
  SensorActivation sensorActivation = SensorActivation::LOW_POWER; ///< Sensor activation mode
  bool rpOverrideEn = true;                                ///< true = use IDRIVEx for fixed drive (recommended)
  bool autoAmpDis = true;                                  ///< true = disable auto amplitude correction (recommended for precision)
  bool highCurrentDrv = false;                             ///< true = high current drive for Ch0 only (single-channel only)

  // === Error Reporting ===
  uint16_t errorConfig = 0x0000;    ///< ERROR_CONFIG register value (reserved bits must remain 0; see CommandTable.h masks)

  // === INTB Pin (optional) ===
  int intbPin = -1;                 ///< GPIO pin for INTB; -1 means not used
  GpioReadFn gpioRead = nullptr;
  void* gpioUser = nullptr;
  bool intbDisable = false;         ///< true = disable INTB output

  // === Health Tracking ===
  uint8_t offlineThreshold = 5;    ///< Consecutive failures before OFFLINE

  // === Recovery Strategy ===
  uint32_t recoverBackoffMs = 100;       ///< Minimum time between recover() attempts (0 = no limit)
  bool recoverUseBusReset = true;        ///< Use bus reset in recover() if callback provided
  bool recoverUseSoftReset = false;      ///< Use soft reset in recover() (requires re-begin)
  bool recoverUseHardReset = true;       ///< Use hard reset in recover() if callback provided
};

} // namespace LDC1614
