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

/// @brief Driver state for health monitoring.
enum class DriverState : uint8_t {
  UNINIT,    ///< begin() not called or end() called
  READY,     ///< Operational, consecutiveFailures == 0
  DEGRADED,  ///< 1 <= consecutiveFailures < offlineThreshold
  OFFLINE    ///< consecutiveFailures >= offlineThreshold
};

/// @brief Parsed device status flags from STATUS register.
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

/// @brief Per-channel data result from conversion readout.
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

/// @brief Per-channel result from readFreshChannels().
struct FreshChannelData {
  ChannelData data = {}; ///< Fresh data if fresh=true; cached data if valid=true and fresh=false
  bool valid = false;    ///< true when data contains either fresh or cached sample data
  bool fresh = false;    ///< true only when STATUS.UNREADCONVx was set and DATAx was read now
};

/// @brief Snapshot of driver configuration and operational state.
/// Returned by getSettings() for diagnostics without further I2C.
struct SettingsSnapshot {
  bool initialized = false;                    ///< True after begin() succeeds
  DriverState state = DriverState::UNINIT;      ///< Driver health state at snapshot time
  uint8_t i2cAddress = 0x2A;                   ///< Active 7-bit I2C address
  uint32_t i2cTimeoutMs = 0;                   ///< Active I2C timeout
  uint8_t offlineThreshold = 0;                ///< Failure threshold for OFFLINE
  bool hasNowMsHook = false;                   ///< True when Config::nowMs is set
  bool hasGpioReadHook = false;                ///< True when Config::gpioRead is set
  bool hasCooperativeYieldHook = false;        ///< True when Config::cooperativeYield is set
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
  bool hardwareConfigDirty = false;             ///< Cached config may not match hardware registers
  Status hardwareConfigDirtyError = Status::Ok(); ///< First status that made config dirty
  bool hasSample[4] = {};                       ///< Per-channel cached-sample flags
  uint32_t sampleTimestampMs[4] = {};   ///< Per-channel last sample timestamp; check hasSample first
  ChannelConfig channel[4] = {};        ///< Per-channel config at snapshot time
};

/// LDC1614/LDC1612 multi-channel inductance-to-digital converter driver.
///
/// Managed synchronous model with 4-state health tracking.
/// All public I2C operations are blocking. Transport is injected via Config.
/// The application controls retry strategy via recover().
///
/// Instances are not internally thread-safe. Public APIs are not ISR-safe.
/// Applications must serialize all calls that can touch shared driver state or
/// I2C. Transport callbacks must not recursively call into the same instance;
/// locking, task ownership, and bus arbitration belong in the injected
/// transport/application bus manager.
class LDC1614 {
public:
  LDC1614() = default;
  LDC1614(const LDC1614&) = delete;
  LDC1614& operator=(const LDC1614&) = delete;
  LDC1614(LDC1614&&) = delete;
  LDC1614& operator=(LDC1614&&) = delete;

  // === Lifecycle ===

  /// @brief Initialize the driver with the given configuration.
  /// Validates config, probes the device (MFG_ID + DEV_ID), and applies
  /// register settings. On success, driver enters READY state in sleep mode.
  /// If validation passes but probe/apply fails, the normalized requested
  /// configuration is retained for diagnostics while initialized remains false.
  /// Failed apply paths can also set hardwareConfigDirty().
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
  /// Safe to call before or after a successful begin(), but it requires
  /// configured transport callbacks. A fresh instance with no prior begin()
  /// attempt returns INVALID_CONFIG because no transport has been supplied.
  /// @return Status (OK if device responds with expected IDs)
  Status probe();

  /// @brief Attempt manual recovery through the configured recovery ladder.
  /// Uses tracked I2C — updates health counters on success/failure.
  /// On success, driver returns to READY state.
  /// Ladder: identity read, optional bus reset, optional soft reset/reapply,
  /// optional hard reset/reapply.
  /// @return Status
  Status recover();

  // === Driver State ===

  /// @brief Get current driver state.
  /// @return DriverState (UNINIT, READY, DEGRADED, or OFFLINE)
  DriverState state() const { return _driverState; }

  /// @brief Alias for state() used by shared diagnostics.
  DriverState driverState() const { return state(); }

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

  /// @brief True when cached configuration may not match hardware registers.
  /// Set after raw diagnostic writes or failed configuration writes that may
  /// have reached the device. Cleared only after a full successful sync,
  /// recover/reapply, resetAndReapply(), or begin().
  bool hardwareConfigDirty() const { return _hardwareConfigDirty; }

  /// @brief First status that made hardwareConfigDirty() true.
  /// The stored detail packs diagnostic context as
  /// phase[31:24] | register[23:16] | index[15:8] | originalDetailLow8.
  Status hardwareConfigDirtyError() const { return _hardwareConfigDirtyError; }

  // === Data Readback ===

  /// @brief Read conversion data for a single channel.
  /// Reads DATAx_MSB first (latches both), then DATAx_LSB.
  /// @param ch Channel index (0-3)
  /// @param out Parsed channel data result
  /// @return Status
  Status readChannel(uint8_t ch, ChannelData& out);

  /// @brief Read conversion data for channels starting at channel 0.
  /// Returns latest DATAx register values. In autoscan mode, one DRDY does not
  /// prove every channel has a fresh unread conversion; use readFreshChannels()
  /// when per-channel freshness matters.
  /// @param out Array of ChannelData, must have at least count elements
  ///            (or channelCount elements when count is 0)
  /// @param count Number of channels to read (0 = use config channelCount)
  /// @return Status (first error encountered, or OK)
  Status readAllChannels(ChannelData* out, uint8_t count = 0);

  /// @brief Read STATUS.UNREADCONVx and then read only fresh channels.
  /// Reads STATUS once, then reads DATAx only for channels whose unread flag is
  /// set. Channels without unread data return the cached sample when available
  /// with valid=true/fresh=false; otherwise valid=false/fresh=false.
  /// STATUS reads can clear sticky error flags and de-assert INTB.
  /// @param out Array of FreshChannelData, must have at least count elements
  ///            (or channelCount elements when count is 0)
  /// @param count Number of channels to evaluate (0 = use config channelCount)
  /// @return Status (first STATUS/DATAx error encountered, or OK)
  Status readFreshChannels(FreshChannelData* out, uint8_t count = 0);

  /// @brief Read fresh channels and return the STATUS snapshot that drove freshness.
  /// Use this overload when STATUS error flags matter; reading STATUS can clear
  /// sticky flags and de-assert INTB, so the snapshot is the caller's evidence.
  /// @param out Array of FreshChannelData, must have at least count elements
  /// @param statusOut STATUS snapshot read before DATAx accesses
  /// @param count Number of channels to evaluate (0 = use config channelCount)
  /// @return Status (first STATUS/DATAx error encountered, or OK)
  Status readFreshChannels(FreshChannelData* out, DeviceStatus& statusOut,
                           uint8_t count = 0);

  /// @brief Check if data is ready (poll DRDY flag or INTB pin).
  /// Convenience wrapper around readDataReady(). Returns false if the driver is
  /// not initialized or if the underlying STATUS/INTB path fails. False can
  /// mean "not ready" or "hidden transport/status/sensor error"; use
  /// readDataReady() for production status handling.
  /// @return true if new conversion data is available
  bool dataReady();

  /// @brief Check if data is ready with explicit error reporting.
  /// Uses INTB pin if configured and enabled; otherwise reads STATUS.DRDY.
  /// If STATUS reports both DRDY and a sensor error, ready is set true and the
  /// return Status is SENSOR_ERROR with the raw STATUS value in detail. Reading
  /// STATUS can clear sticky flags and de-assert INTB.
  /// @param ready Output: true if new conversion data is available
  /// @return Status from the underlying GPIO/STATUS path
  Status readDataReady(bool& ready);

  /// @brief Read a channel with blocking wait for data ready.
  /// Polls readDataReady() with cooperative yield until timeout. Requires a
  /// monotonic Config::nowMs callback; returns INVALID_CONFIG if it is absent.
  /// The timeout bounds the readiness wait; the final DATAx readout still uses
  /// injected I2C transaction timeouts and callback latency.
  /// @param ch Channel index (0-3)
  /// @param out Parsed channel data result
  /// @param timeoutMs Maximum wait time in milliseconds (default 200)
  /// @return Status (TIMEOUT if data not ready within deadline)
  Status readChannelBlocking(uint8_t ch, ChannelData& out, uint32_t timeoutMs = 200);

  /// @brief Read channels starting at channel 0 with blocking wait.
  /// Waits for one DRDY event, then calls readAllChannels(); the returned
  /// channel values are latest-register semantics, not guaranteed fresh for all
  /// autoscan channels. Requires a monotonic Config::nowMs callback.
  /// The timeout bounds the readiness wait; the final DATAx readout still uses
  /// injected I2C transaction timeouts and callback latency.
  /// @param out Array of ChannelData, at least count elements
  ///            (or channelCount elements when count is 0)
  /// @param timeoutMs Maximum wait time for data ready (default 200)
  /// @param count Number of channels (0 = use config channelCount)
  /// @return Status
  Status readAllChannelsBlocking(ChannelData* out, uint32_t timeoutMs = 200, uint8_t count = 0);

  // === Poll-chunked operations ===

  /// @brief Start a poll-driven selected-channel read job.
  ///
  /// The job reads only DATAx registers for channels selected in mask bits 0..3.
  /// It does not read STATUS, so it does not clear STATUS sticky flags or
  /// de-assert INTB as a hidden side effect. Each DATAx_MSB or DATAx_LSB
  /// register read consumes one poll instruction, but a channel sample is not
  /// exposed through getChannelSample() until both registers have been read.
  /// Only one poll-chunked job may be active per driver instance.
  /// @param mask Channel bit mask; bit 0 selects channel 0.
  /// @return IN_PROGRESS when scheduled, or an error precondition status
  Status startReadChannels(uint8_t mask);

  /// @brief Advance the active poll-chunked job.
  ///
  /// A register write or write-read consumes one instruction. CPU-only state
  /// transitions and delay/ready checks consume zero instructions. Passing
  /// maxInstructions=0 never touches I2C and returns IN_PROGRESS while a job is
  /// active.
  /// @param nowMs Current monotonic time in milliseconds for delay gates
  /// @param maxInstructions Maximum I2C instructions to execute this call
  /// @return OK when no job remains, IN_PROGRESS while work remains, or first failure
  Status poll(uint32_t nowMs, uint8_t maxInstructions = 1);

  /// @brief True after the most recent startReadChannels() job completed.
  bool readChannelsReady() const;

  /// @brief Get a sample produced by the most recent completed channel-read job.
  /// @param ch Channel index
  /// @param out Output sample
  /// @return OK if the requested channel was selected and read by the last job
  Status getChannelSample(uint8_t ch, ChannelData& out) const;

  /// @brief Start a poll-driven cached configuration apply job.
  ///
  /// This mirrors syncConfig() but writes one register per poll instruction.
  /// The device is forced asleep before channel/global config writes and remains
  /// asleep on success.
  /// @return IN_PROGRESS when scheduled, or an error precondition status
  Status startApplyConfig();

  /// @brief Start a poll-driven RESET_DEV plus cached configuration reapply job.
  /// @return IN_PROGRESS when scheduled, or an error precondition status
  Status startResetAndReapply();

  // === Sample Cache ===

  /// @brief Get the last successfully read data for a channel (cached, no I2C).
  /// @param ch Channel index (0-3)
  /// @param out Last cached channel data
  /// @return Status (CONVERSION_NOT_READY if never read)
  Status getLastSample(uint8_t ch, ChannelData& out) const;

  /// @brief True after a sample has been cached for a channel.
  /// @param ch Channel index (0-3)
  bool hasSample(uint8_t ch) const;

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
  /// The datasheet evidence in this repository does not require an arbitrary
  /// post-RESET_DEV delay before I2C reconfiguration. If board hardware needs
  /// reset timing, keep it in the injected hardReset callback or a bounded
  /// application-owned recovery policy.
  /// @return Status (OK if reset and reconfiguration succeed)
  Status resetAndReapply();

  /// @brief Re-apply the cached configuration to hardware.
  ///
  /// Recovery recipe for dirty hardware state:
  /// 1. Check hardwareConfigDirty().
  /// 2. Stop trusting cached configuration-dependent behavior.
  /// 3. Call syncConfig(), recover(), resetAndReapply(), or begin().
  /// 4. Trust the cache again only after hardwareConfigDirty() is false.
  ///
  /// Full reapply first writes CONFIG with SLEEP_MODE_EN=1, then writes
  /// channel/global configuration, then writes the final sleeping CONFIG image.
  /// On success the device remains in sleep mode. On failure the original
  /// transport/status error is returned and the dirty state remains set.
  /// @return Status
  Status syncConfig();

  // === Runtime Configuration ===

  /// @brief Set active channel for single-channel mode.
  /// Device must be in sleep mode to change configuration.
  /// @param ch Channel index (0-3)
  /// @return Status
  Status setActiveChannel(uint8_t ch);

  /// @brief Get current active channel index.
  uint8_t getActiveChannel() const { return _config.activeChan; }

  /// @brief Set single-channel continuous mode and select active channel.
  /// Device must be in sleep mode. Writes MUX_CONFIG and CONFIG.
  /// @param ch Channel index (0-3)
  /// @return Status
  Status setSingleChannelMode(uint8_t ch);

  /// @brief Set multi-channel auto-scan mode.
  /// Device must be in sleep mode. Sequence must fit the configured device
  /// channel count (for example, LDC1612 only supports CH0_CH1).
  /// @param sequence Round-robin conversion sequence
  /// @return Status
  Status setAutoScanMode(RRSequence sequence);

  /// @brief Set input deglitch filter bandwidth.
  /// Device must be in sleep mode. Writes MUX_CONFIG.
  /// @param deglitch Deglitch filter bandwidth
  /// @return Status
  Status setDeglitch(Deglitch deglitch);

  /// @brief Set ERROR_CONFIG register and cached error-reporting configuration.
  /// Device must be in sleep mode.
  /// @param errorConfig Bit mask using cmd::MASK_ERRCFG_* constants
  /// @return Status
  Status setErrorConfig(uint16_t errorConfig);

  /// @brief Get cached ERROR_CONFIG value.
  uint16_t getErrorConfig() const { return _config.errorConfig; }

  /// @brief Enable or disable INTB output in CONFIG.INTB_DIS.
  /// Device must be in sleep mode.
  /// @param disabled true disables INTB and holds pin high
  /// @return Status
  Status setIntbDisabled(bool disabled);

  /// @brief Set reference clock source.
  /// Device must be in sleep mode. Writes CONFIG.
  /// @param source Internal oscillator or external CLKIN
  /// @return Status
  Status setReferenceClockSource(RefClkSrc source);

  /// @brief Set sensor activation current policy.
  /// Device must be in sleep mode. Writes CONFIG.
  /// @param activation Full-current or low-power activation policy
  /// @return Status
  Status setSensorActivation(SensorActivation activation);

  /// @brief Set RP_OVERRIDE_EN.
  /// Device must be in sleep mode. Writes CONFIG.
  /// @param enabled true uses fixed IDRIVEx drive current
  /// @return Status
  Status setRpOverrideEnabled(bool enabled);

  /// @brief Enable or disable automatic amplitude correction.
  /// Device must be in sleep mode. Writes CONFIG.AUTO_AMP_DIS.
  /// @param enabled true enables automatic amplitude correction
  /// @return Status
  Status setAutoAmplitudeCorrectionEnabled(bool enabled);

  /// @brief Set high-current drive mode.
  /// Device must be in sleep mode. Valid only in single-channel Ch0 mode.
  /// @param enabled true enables high current drive
  /// @return Status
  Status setHighCurrentDriveEnabled(bool enabled);

  /// @brief Set RCOUNT for a channel.
  /// Device must be in sleep mode.
  /// @param ch Channel index (0-3)
  /// @param rcount Reference count value (0x0005-0xFFFF; autoscan selected
  /// channels require >=0x0009)
  /// @return Status
  Status setRcount(uint8_t ch, uint16_t rcount);

  /// @brief Set SETTLECOUNT for a channel.
  /// Device must be in sleep mode.
  /// @param ch Channel index (0-3)
  /// @param count Settle count value (autoscan selected channels require >=0x0004)
  /// @return Status
  Status setSettleCount(uint8_t ch, uint16_t count);

  /// @brief Set CLOCK_DIVIDERS for a channel.
  /// Device must be in sleep mode.
  /// @param ch Channel index (0-3)
  /// @param finDiv Sensor frequency divider (1-15; application must use >=2
  /// when the actual sensor frequency is >=8.75 MHz)
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

  /// @brief Diagnostic-only read of a 16-bit register.
  /// Uses tracked I2C — updates health counters.
  /// Rejects access before begin() and register addresses outside the LDC1614 map.
  /// This escape hatch is intentionally not variant/access-type safe: typed
  /// APIs remain the safe path for normal operation. STATUS and DATAx reads can
  /// have datasheet-defined side effects.
  /// @param reg Register address
  /// @param value Output: register value
  /// @return Status
  Status readRegister16(uint8_t reg, uint16_t& value);

  /// @brief Diagnostic-only write of a 16-bit register.
  /// Uses tracked I2C — updates health counters.
  /// Rejects access before begin() and register addresses outside the LDC1614 map.
  /// @warning Any successful diagnostic write marks hardwareConfigDirty()
  /// because it can desynchronize cached configuration. Call syncConfig(),
  /// recover(), resetAndReapply(), or begin() before trusting the cache.
  /// @param reg Register address
  /// @param value Value to write
  /// @return Status
  Status writeRegister16(uint8_t reg, uint16_t value);

  // === Settings Snapshot ===

  /// @brief Get a snapshot of current driver configuration and state (no I2C).
  /// Captures driver state, mode, channel config, and sample timestamps.
  /// @param out Settings snapshot
  /// @return Status::Ok() always
  Status getSettings(SettingsSnapshot& out) const;

  /// @brief Return a by-value settings snapshot.
  SettingsSnapshot settings() const {
    SettingsSnapshot out;
    (void)getSettings(out);
    return out;
  }

  // === Utility ===

  /// @brief Estimate sensor frequency from raw 28-bit data value.
  /// Uses fSENSOR = rawData * fREFx / 2^28, where
  /// fREFx = fRef / CLOCK_DIVIDERSx.FREF_DIVIDER. The fRef argument is the
  /// pre-divider reference clock at the device, not already divided fREFx.
  /// @param ch Channel index (used for divider/offset config)
  /// @param rawData 28-bit conversion result
  /// @param fRef Reference clock frequency in Hz before FREF_DIVIDER
  /// @return Estimated sensor frequency in Hz, or 0 on invalid input
  float calcSensorFrequency(uint8_t ch, uint32_t rawData, float fRef) const;

  /// @brief Calculate conversion time for a channel in microseconds.
  /// Uses the datasheet approximation (RCOUNTx * 16 + 4) / fREFx. This does
  /// not include settling, host readout, interrupt latency, or sequencing
  /// overhead.
  /// @param ch Channel index
  /// @param fRef Reference clock frequency in Hz before FREF_DIVIDER
  /// @return Estimated conversion time in microseconds, or 0 on invalid input
  float calcConversionTimeUs(uint8_t ch, float fRef) const;

  /// @brief Calculate sensor settling time for a channel in microseconds.
  /// SETTLECOUNT 0 and 1 use the datasheet minimum of 32 fREFx cycles;
  /// otherwise this uses SETTLECOUNTx * 16 cycles.
  /// @param ch Channel index
  /// @param fRef Reference clock frequency in Hz before FREF_DIVIDER
  /// @return Estimated settling time in microseconds, or 0 on invalid input
  float calcSettleTimeUs(uint8_t ch, float fRef) const;

  /// @brief Calculate conversion plus settling time for a channel in microseconds.
  /// Does not include host readout time, INTB/STATUS polling latency, or
  /// multi-channel switching/sequencing overhead. For autoscan, estimate the
  /// frame time by summing enabled channels and then validate on hardware.
  /// @param ch Channel index
  /// @param fRef Reference clock frequency in Hz before FREF_DIVIDER
  /// @return Nominal per-channel sample time in microseconds, or 0 on invalid input
  float calcSampleTimeUs(uint8_t ch, float fRef) const;

  /// @brief Get the configured channel count.
  uint8_t channelCount() const { return _config.channelCount; }

private:
  enum class ChunkedJobKind : uint8_t {
    NONE,
    READ_CHANNELS,
    APPLY_CONFIG,
    RESET_AND_REAPPLY
  };

  enum class ChunkedReadPhase : uint8_t {
    MSB,
    LSB
  };

  // === Transport Wrappers ===
  Status _i2cWriteReadRaw(const uint8_t* txBuf, size_t txLen,
                          uint8_t* rxBuf, size_t rxLen);
  Status _i2cWriteRaw(const uint8_t* buf, size_t len);
  Status _i2cWriteReadTracked(const uint8_t* txBuf, size_t txLen,
                              uint8_t* rxBuf, size_t rxLen);
  Status _i2cWriteTracked(const uint8_t* buf, size_t len);

  // === Register Access (raw, no health tracking) ===
  Status _readRegister16Raw(uint8_t reg, uint16_t& value);
  Status _readRegister16Tracked(uint8_t reg, uint16_t& value);
  Status _writeRegister16Tracked(uint8_t reg, uint16_t value);
  Status _writeConfigRegister(uint8_t reg, uint16_t value,
                              uint8_t phase, uint8_t index);

  // === Health Tracking ===
  Status _updateHealth(const Status& st);
  Status _recordFailure(const Status& st);
  void _reassertOfflineLatch();
  Status _ensureNormalI2cAllowed() const;

  // === Internal ===
  Status _applyConfig();
  Status _performRecoveryLadder();
  Status _pollReadChannels(uint8_t& remainingInstructions);
  Status _pollApplyConfig(uint8_t& remainingInstructions);
  Status _pollResetAndReapply(uint8_t& remainingInstructions);
  Status _startChunkedJob(ChunkedJobKind kind);
  Status _finishChunkedJob(const Status& st);
  void _storeChannelData(uint8_t ch, uint16_t msb, uint16_t lsb,
                         ChannelData& out);
  bool _configStep(uint8_t step, uint8_t& reg, uint16_t& value,
                   uint8_t& phase, uint8_t& index) const;
  uint8_t _configStepCount() const;
  uint8_t _firstSelectedChannel(uint8_t mask) const;
  uint8_t _nextSelectedChannel(uint8_t after, uint8_t mask) const;
  void _markHardwareConfigDirty(const Status& cause, uint8_t phase,
                                uint8_t reg, uint8_t index);
  void _clearHardwareConfigDirty();
  uint16_t _buildConfigRegister(bool sleepMode) const;
  uint16_t _buildMuxConfigRegister() const;
  uint32_t _nowMs() const;
  void _cooperativeYield() const;

  // === State ===
  Config _config;
  bool _initialized = false;
  bool _sleeping = true;
  DriverState _driverState = DriverState::UNINIT;
  bool _hardwareConfigDirty = false;
  Status _hardwareConfigDirtyError = Status::Ok();

  // === Health Counters ===
  uint32_t _lastOkMs = 0;
  uint32_t _lastErrorMs = 0;
  Status _lastError = Status::Ok();
  uint8_t _consecutiveFailures = 0;
  uint32_t _totalFailures = 0;
  uint32_t _totalSuccess = 0;
  bool _allowOfflineI2c = false;

  // === Sample Cache (per channel) ===
  ChannelData _lastChannelData[4] = {};
  bool _hasSample[4] = {};
  uint32_t _sampleTimestampMs[4] = {};

  // === Recovery Backoff ===
  uint32_t _lastRecoverMs = 0;
  bool _lastRecoverValid = false;

  // === Poll-chunked job state ===
  ChunkedJobKind _chunkedJob = ChunkedJobKind::NONE;
  uint8_t _chunkedConfigStep = 0;
  uint8_t _chunkedReadMask = 0;
  uint8_t _chunkedReadReadyMask = 0;
  uint8_t _chunkedReadChannel = 0;
  ChunkedReadPhase _chunkedReadPhase = ChunkedReadPhase::MSB;
  uint16_t _chunkedReadMsb = 0;
  bool _chunkedPollExecuting = false;
};

} // namespace LDC1614
