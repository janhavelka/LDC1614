/// @file LDC1614.h
/// @brief Main driver class for LDC1614/LDC1612
#pragma once

#include <cstddef>
#include <cstdint>

#include "LDC1614/CommandTable.h"
#include "LDC1614/Config.h"
#include "LDC1614/Status.h"
#include "LDC1614/Version.h"

namespace LDC1614 {

/// Driver state for health monitoring
enum class DriverState : uint8_t {
  UNINIT,    ///< begin() not called or end() called
  READY,     ///< Operational, consecutiveFailures == 0
  DEGRADED,  ///< 1 <= consecutiveFailures < offlineThreshold
  OFFLINE    ///< consecutiveFailures >= offlineThreshold
};

/// Parsed device status flags from STATUS register
struct DeviceStatus {
  uint8_t errChan = 0;      ///< Channel that generated the error (0-3)
  bool errUnderRange = false;    ///< ERR_UR: conversion result below valid range
  bool errOverRange = false;     ///< ERR_OR: conversion result above valid range
  bool errWatchdog = false;      ///< ERR_WD: sensor timeout/watchdog event
  bool errAmplitudeHigh = false; ///< ERR_AH: sensor amplitude too high
  bool errAmplitudeLow = false;  ///< ERR_AL: sensor amplitude too low
  bool errZeroCount = false;     ///< ERR_ZC: zero-count condition detected
  bool dataReady = false;        ///< DRDY: at least one unread conversion is available
  bool unreadConv[4] = {};   ///< Per-channel unread conversion flags
  uint16_t raw = 0;          ///< Raw STATUS register value

  /// @return true if any error flag is set
  bool hasError() const {
    return errUnderRange || errOverRange || errWatchdog ||
           errAmplitudeHigh || errAmplitudeLow || errZeroCount;
  }
};

/// Per-channel data result from conversion readout
struct ChannelData {
  uint32_t rawData = 0;     ///< 28-bit conversion result
  bool errUnderRange = false; ///< Channel result below valid operating range
  bool errOverRange = false;  ///< Channel result above valid operating range
  bool errWatchdog = false;   ///< Channel watchdog/timeout flag
  bool errAmplitude = false;  ///< Channel amplitude error (high or low)

  /// @return true if any per-channel error flag is set
  bool hasError() const {
    return errUnderRange || errOverRange || errWatchdog || errAmplitude;
  }
};

/// Snapshot of driver configuration and operational state.
/// Returned by getSettings() for diagnostics without further I2C.
struct SettingsSnapshot {
  DriverState state = DriverState::UNINIT;      ///< Driver health state at snapshot time
  bool sleeping = true;                         ///< True if the driver believes conversions are stopped
  bool autoScan = false;                        ///< Multi-channel autoscan enabled in MUX_CONFIG
  uint8_t activeChan = 0;                       ///< Active channel when single-channel mode is selected
  uint8_t channelCount = 4;                     ///< Configured logical channel count (2 or 4)
  RRSequence rrSequence = RRSequence::CH0_CH1;  ///< Conversion sequence programmed in MUX_CONFIG
  Deglitch deglitch = Deglitch::BW_33MHZ;       ///< Input deglitch filter bandwidth
  RefClkSrc refClkSrc = RefClkSrc::INTERNAL;    ///< Reference clock source selection
  SensorActivation sensorActivation = SensorActivation::LOW_POWER; ///< Sensor current drive policy
  bool rpOverrideEn = true;                     ///< RP_OVERRIDE_EN bit state
  bool autoAmpDis = true;                       ///< AUTO_AMP_DIS bit state
  bool highCurrentDrv = false;                  ///< High-current drive mode enabled
  bool intbEnabled = false;                     ///< INTB pin enabled for DRDY/error signaling
  uint32_t sampleTimestampMs[4] = {};   ///< Per-channel last sample timestamp (0 = never)
  ChannelConfig channel[4] = {};        ///< Per-channel config at snapshot time
};

/// LDC1614/LDC1612 multi-channel inductance-to-digital converter driver.
///
/// Managed synchronous model with 4-state health tracking.
/// All public I2C operations are blocking. Transport is injected via Config.
/// The application controls retry strategy via recover().
class LDC1614 {
public:
  // === Lifecycle ===

  /// @brief Initialize the driver with the given configuration.
  /// Validates config, probes the device (MFG_ID + DEV_ID), and applies
  /// register settings. On success, driver enters READY state in sleep mode.
  /// @param config Driver configuration (transport, channel settings, etc.)
  /// @return Status (OK on success, INVALID_CONFIG/DEVICE_NOT_FOUND/I2C_ERROR on failure)
  Status begin(const Config& config);

  /// @brief Cooperative tick — call periodically from loop().
  /// Currently a no-op for LDC1614 (all reads are synchronous).
  /// @param nowMs Current monotonic time in milliseconds
  void tick(uint32_t nowMs);

  /// @brief Shut down the driver. Best-effort sleep before UNINIT.
  /// After end(), call begin() to reinitialize.
  void end();

  /// @brief Check if begin() completed successfully and end() has not been called.
  bool isInitialized() const { return _initialized; }

  /// @brief Get the active configuration snapshot.
  const Config& getConfig() const { return _config; }

  // === Diagnostics (no health tracking) ===

  /// @brief Probe device presence by reading MANUFACTURER_ID and DEVICE_ID.
  /// Uses raw I2C — does NOT update health counters.
  /// Safe to call before or after begin().
  /// @return Status (OK if device responds with expected IDs)
  Status probe();

  /// @brief Attempt manual recovery by re-reading MANUFACTURER_ID.
  /// Uses tracked I2C — updates health counters on success/failure.
  /// On success, driver returns to READY state.
  /// @return Status
  Status recover();

  // === Driver State ===

  /// @brief Get current driver state.
  /// @return DriverState (UNINIT, READY, DEGRADED, or OFFLINE)
  DriverState state() const { return _driverState; }

  /// @brief Check if driver is operational (READY or DEGRADED).
  /// @return true if the driver can perform I2C operations
  bool isOnline() const {
    return _driverState == DriverState::READY ||
           _driverState == DriverState::DEGRADED;
  }

  // === Health Tracking ===

  /// @brief Timestamp of last successful I2C operation (ms).
  uint32_t lastOkMs() const { return _lastOkMs; }

  /// @brief Timestamp of last failed I2C operation (ms).
  uint32_t lastErrorMs() const { return _lastErrorMs; }

  /// @brief Most recent error status from tracked I2C operations.
  Status lastError() const { return _lastError; }

  /// @brief Number of consecutive I2C failures since last success.
  uint8_t consecutiveFailures() const { return _consecutiveFailures; }

  /// @brief Lifetime count of failed I2C operations (wraps at UINT32_MAX).
  uint32_t totalFailures() const { return _totalFailures; }

  /// @brief Lifetime count of successful I2C operations (wraps at UINT32_MAX).
  uint32_t totalSuccess() const { return _totalSuccess; }

  // === Data Readback ===

  /// @brief Read conversion data for a single channel.
  /// Reads DATAx_MSB first (latches both), then DATAx_LSB.
  /// @param ch Channel index (0-3)
  /// @param out Parsed channel data result
  /// @return Status
  Status readChannel(uint8_t ch, ChannelData& out);

  /// @brief Read conversion data for all active channels.
  /// @param out Array of ChannelData, must have at least channelCount elements
  /// @param count Number of channels to read (0 = use config channelCount)
  /// @return Status (first error encountered, or OK)
  Status readAllChannels(ChannelData* out, uint8_t count = 0);

  /// @brief Check if data is ready (poll DRDY flag or INTB pin).
  /// @return true if new conversion data is available
  bool dataReady();

  /// @brief Read a channel with blocking wait for data ready.
  /// Polls dataReady() with cooperative yield until timeout.
  /// @param ch Channel index (0-3)
  /// @param out Parsed channel data result
  /// @param timeoutMs Maximum wait time in milliseconds (default 200)
  /// @return Status (TIMEOUT if data not ready within deadline)
  Status readChannelBlocking(uint8_t ch, ChannelData& out, uint32_t timeoutMs = 200);

  /// @brief Read all active channels with blocking wait.
  /// @param out Array of ChannelData, at least channelCount elements
  /// @param timeoutMs Maximum wait time for data ready (default 200)
  /// @param count Number of channels (0 = use config channelCount)
  /// @return Status
  Status readAllChannelsBlocking(ChannelData* out, uint32_t timeoutMs = 200, uint8_t count = 0);

  // === Sample Cache ===

  /// @brief Get the last successfully read data for a channel (cached, no I2C).
  /// @param ch Channel index (0-3)
  /// @param out Last cached channel data
  /// @return Status (CONVERSION_NOT_READY if never read)
  Status getLastSample(uint8_t ch, ChannelData& out) const;

  /// @brief Timestamp (ms) of the last successful read for a channel.
  /// @param ch Channel index (0-3)
  /// @return Timestamp in milliseconds (0 if never read)
  uint32_t sampleTimestampMs(uint8_t ch) const;

  /// @brief Age (ms) of the cached sample for a channel.
  /// @param ch Channel index (0-3)
  /// @param nowMs Current monotonic time in milliseconds
  /// @return Age in milliseconds (0 if never read)
  uint32_t sampleAgeMs(uint8_t ch, uint32_t nowMs) const;

  /// @brief Check if the device is currently converting (not sleeping).
  /// @return true if device is initialized and not in sleep mode
  bool isMeasuring() const { return _initialized && !_sleeping; }

  // === Status Register ===

  /// @brief Read and parse the STATUS register.
  /// Also clears sticky error flags and de-asserts INTB.
  /// @param out Parsed device status
  /// @return Status
  Status readDeviceStatus(DeviceStatus& out);

  /// @brief Read raw STATUS register value.
  /// @param out Raw 16-bit register value
  /// @return Status
  Status readStatusRaw(uint16_t& out);

  // === Control ===

  /// @brief Enter sleep mode (stop conversions, retain configuration).
  /// @return Status
  Status sleep();

  /// @brief Wake from sleep mode and start conversions.
  /// CONFIG register is written with SLEEP_MODE_EN=0.
  /// @return Status
  Status wake();

  /// @brief Check if device is in sleep mode.
  /// @return true if in sleep mode
  bool isSleeping() const { return _sleeping; }

  /// @brief Software reset. All registers return to defaults.
  /// After reset, the driver transitions to UNINIT state.
  /// Call begin() again to reinitialize.
  /// @return Status
  Status softReset();

  /// @brief Software reset followed by re-applying the stored configuration.
  /// Unlike softReset(), the driver re-enters READY state on success.
  /// @return Status (OK if reset and reconfiguration succeed)
  Status resetAndReapply();

  // === Runtime Configuration ===

  /// @brief Set active channel for single-channel mode.
  /// Device must be in sleep mode to change configuration.
  /// @param ch Channel index (0-3)
  /// @return Status
  Status setActiveChannel(uint8_t ch);

  /// @brief Get current active channel index.
  uint8_t getActiveChannel() const { return _config.activeChan; }

  /// @brief Set RCOUNT for a channel.
  /// Device must be in sleep mode.
  /// @param ch Channel index (0-3)
  /// @param rcount Reference count value (0x0005-0xFFFF)
  /// @return Status
  Status setRcount(uint8_t ch, uint16_t rcount);

  /// @brief Set SETTLECOUNT for a channel.
  /// Device must be in sleep mode.
  /// @param ch Channel index (0-3)
  /// @param count Settle count value
  /// @return Status
  Status setSettleCount(uint8_t ch, uint16_t count);

  /// @brief Set CLOCK_DIVIDERS for a channel.
  /// Device must be in sleep mode.
  /// @param ch Channel index (0-3)
  /// @param finDiv Sensor frequency divider (1-15)
  /// @param frefDiv Reference clock divider (1-1023)
  /// @return Status
  Status setClockDividers(uint8_t ch, uint8_t finDiv, uint16_t frefDiv);

  /// @brief Set OFFSET for a channel.
  /// Device must be in sleep mode.
  /// @param ch Channel index (0-3)
  /// @param offset Conversion offset value
  /// @return Status
  Status setOffset(uint8_t ch, uint16_t offset);

  /// @brief Set drive current for a channel.
  /// Device must be in sleep mode.
  /// @param ch Channel index (0-3)
  /// @param idrive Drive current index (0-31)
  /// @return Status
  Status setDriveCurrent(uint8_t ch, uint8_t idrive);

  /// @brief Read INIT_IDRIVE from DRIVE_CURRENTx register (auto-calibration result).
  /// @param ch Channel index (0-3)
  /// @param out The 5-bit INIT_IDRIVE value
  /// @return Status
  Status readInitIdrive(uint8_t ch, uint8_t& out);

  // === Raw Register Access ===

  /// @brief Read a 16-bit register.
  /// Uses tracked I2C — updates health counters.
  /// @param reg Register address
  /// @param value Output: register value
  /// @return Status
  Status readRegister16(uint8_t reg, uint16_t& value);

  /// @brief Write a 16-bit register.
  /// Uses tracked I2C — updates health counters.
  /// @warning Bypasses driver-level validation. Use with care.
  /// @param reg Register address
  /// @param value Value to write
  /// @return Status
  Status writeRegister16(uint8_t reg, uint16_t value);

  // === Settings Snapshot ===

  /// @brief Get a snapshot of current driver configuration and state (no I2C).
  /// Captures driver state, mode, channel config, and sample timestamps.
  /// @param out Settings snapshot
  void getSettings(SettingsSnapshot& out) const;

  // === Utility ===

  /// @brief Calculate sensor frequency from raw 28-bit data value.
  /// @param ch Channel index (used for divider/offset config)
  /// @param rawData 28-bit conversion result
  /// @param fRef Reference clock frequency in Hz
  /// @return Sensor frequency in Hz
  float calcSensorFrequency(uint8_t ch, uint32_t rawData, float fRef) const;

  /// @brief Calculate conversion time for a channel in microseconds.
  /// @param ch Channel index
  /// @param fRef Reference clock frequency in Hz
  /// @return Conversion time in microseconds
  float calcConversionTimeUs(uint8_t ch, float fRef) const;

  /// @brief Get the configured channel count.
  uint8_t channelCount() const { return _config.channelCount; }

private:
  // === Transport Wrappers ===
  Status _i2cWriteReadRaw(const uint8_t* txBuf, size_t txLen,
                          uint8_t* rxBuf, size_t rxLen);
  Status _i2cWriteRaw(const uint8_t* buf, size_t len);
  Status _i2cWriteReadTracked(const uint8_t* txBuf, size_t txLen,
                              uint8_t* rxBuf, size_t rxLen);
  Status _i2cWriteTracked(const uint8_t* buf, size_t len);

  // === Register Access (raw, no health tracking) ===
  Status _readRegister16Raw(uint8_t reg, uint16_t& value);

  // === Health Tracking ===
  Status _updateHealth(const Status& st);

  // === Internal ===
  Status _applyConfig();
  Status _performRecoveryLadder();
  uint16_t _buildConfigRegister(bool sleepMode) const;
  uint16_t _buildMuxConfigRegister() const;
  uint32_t _nowMs() const;
  void _cooperativeYield() const;

  // === State ===
  Config _config;
  bool _initialized = false;
  bool _sleeping = true;
  DriverState _driverState = DriverState::UNINIT;

  // === Health Counters ===
  uint32_t _lastOkMs = 0;
  uint32_t _lastErrorMs = 0;
  Status _lastError = Status::Ok();
  uint8_t _consecutiveFailures = 0;
  uint32_t _totalFailures = 0;
  uint32_t _totalSuccess = 0;

  // === Sample Cache (per channel) ===
  ChannelData _lastChannelData[4] = {};
  uint32_t _sampleTimestampMs[4] = {};

  // === Recovery Backoff ===
  uint32_t _lastRecoverMs = 0;
  bool _lastRecoverValid = false;
};

} // namespace LDC1614
