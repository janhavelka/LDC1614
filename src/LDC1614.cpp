/// @file LDC1614.cpp
/// @brief Implementation of LDC1614/LDC1612 driver

#include "LDC1614/LDC1614.h"

#include <climits>
#include <cmath>

namespace LDC1614 {

namespace {

class ScopedOfflineI2cAllowance {
public:
  explicit ScopedOfflineI2cAllowance(bool& flag, bool allow) : _flag(flag), _old(flag) {
    _flag = allow;
  }

  ~ScopedOfflineI2cAllowance() {
    _flag = _old;
  }

  ScopedOfflineI2cAllowance(const ScopedOfflineI2cAllowance&) = delete;
  ScopedOfflineI2cAllowance& operator=(const ScopedOfflineI2cAllowance&) = delete;

private:
  bool& _flag;
  bool _old;
};

bool isValidChannel(uint8_t ch, uint8_t channelCount) {
  return ch < channelCount;
}

bool isValidRcount(uint16_t rcount) {
  return rcount >= cmd::RCOUNT_MIN;
}

bool isValidMultiChannelRcount(uint16_t rcount) {
  return rcount >= cmd::MULTI_RCOUNT_MIN;
}

bool isValidMultiChannelSettleCount(uint16_t settleCount) {
  return settleCount >= cmd::MULTI_SETTLE_MIN;
}

bool isValidFinDivider(uint8_t finDiv) {
  return finDiv >= cmd::FIN_DIVIDER_MIN && finDiv <= cmd::FIN_DIVIDER_MAX;
}

bool isValidFrefDivider(uint16_t frefDiv) {
  return frefDiv >= cmd::FREF_DIVIDER_MIN && frefDiv <= cmd::FREF_DIVIDER_MAX;
}

bool isValidIdrive(uint8_t idrive) {
  return idrive <= cmd::IDRIVE_MAX;
}

bool isValidDeglitch(Deglitch dg) {
  return dg == Deglitch::BW_1MHZ || dg == Deglitch::BW_3MHZ ||
         dg == Deglitch::BW_10MHZ || dg == Deglitch::BW_33MHZ;
}

bool isValidRRSequence(RRSequence seq) {
  return static_cast<uint8_t>(seq) <= static_cast<uint8_t>(RRSequence::CH0_CH1_CH2_CH3);
}

bool isRRSequenceAllowed(RRSequence seq, uint8_t channelCount) {
  if (!isValidRRSequence(seq)) {
    return false;
  }
  if (channelCount == 2) {
    return seq == RRSequence::CH0_CH1;
  }
  if (channelCount == 4) {
    return true;
  }
  return false;
}

uint8_t rrSequenceChannelCount(RRSequence seq) {
  switch (seq) {
    case RRSequence::CH0_CH1:
      return 2;
    case RRSequence::CH0_CH1_CH2:
      return 3;
    case RRSequence::CH0_CH1_CH2_CH3:
      return 4;
    default:
      return 0;
  }
}

bool isInAutoscanSequence(uint8_t ch, RRSequence seq) {
  return ch < rrSequenceChannelCount(seq);
}

Status validateMultiChannelTiming(const Config& config, RRSequence seq, Err errCode) {
  const uint8_t n = rrSequenceChannelCount(seq);
  if (n == 0 || n > config.channelCount) {
    return Status::Error(errCode, "Invalid RR sequence for channelCount");
  }
  for (uint8_t ch = 0; ch < n; ++ch) {
    if (!isValidMultiChannelRcount(config.channel[ch].rcount)) {
      return Status::Error(errCode, "RCOUNT below multi-channel minimum (0x0009)", ch);
    }
    if (!isValidMultiChannelSettleCount(config.channel[ch].settleCount)) {
      return Status::Error(errCode, "SETTLECOUNT below multi-channel minimum (0x0004)", ch);
    }
  }
  return Status::Ok();
}

bool isValidRefClkSrc(RefClkSrc src) {
  return static_cast<uint8_t>(src) <= static_cast<uint8_t>(RefClkSrc::EXT_CLK);
}

bool isValidSensorActivation(SensorActivation sa) {
  return static_cast<uint8_t>(sa) <= static_cast<uint8_t>(SensorActivation::LOW_POWER);
}

bool isValidErrorConfig(uint16_t errorConfig) {
  return (errorConfig & ~cmd::MASK_ERRCFG_ALLOWED) == 0;
}

bool isValidRegisterAddress(uint8_t reg) {
  return reg <= cmd::REG_RESET_DEV ||
         (reg >= cmd::REG_DRIVE_CURRENT0 && reg <= cmd::REG_DRIVE_CURRENT3) ||
         reg == cmd::REG_MANUFACTURER_ID ||
         reg == cmd::REG_DEVICE_ID;
}

static constexpr uint8_t DIRTY_PHASE_APPLY_CHANNEL = 0x01;
static constexpr uint8_t DIRTY_PHASE_APPLY_GLOBAL = 0x02;
static constexpr uint8_t DIRTY_PHASE_SETTER_SINGLE = 0x03;
static constexpr uint8_t DIRTY_PHASE_SETTER_MULTI_FIRST = 0x04;
static constexpr uint8_t DIRTY_PHASE_SETTER_MULTI_SECOND = 0x05;
static constexpr uint8_t DIRTY_PHASE_RAW_WRITE = 0x06;
static constexpr uint8_t DIRTY_PHASE_RESET = 0x07;
static constexpr uint8_t DIRTY_INDEX_GLOBAL = 0xFF;

int32_t packDirtyDetail(uint8_t phase, uint8_t reg, uint8_t index, int32_t originalDetail) {
  const uint32_t packed = (static_cast<uint32_t>(phase) << 24) |
                          (static_cast<uint32_t>(reg) << 16) |
                          (static_cast<uint32_t>(index) << 8) |
                          (static_cast<uint32_t>(originalDetail) & 0xFFU);
  return static_cast<int32_t>(packed);
}

Status dirtyStatusFrom(const Status& cause, uint8_t phase, uint8_t reg, uint8_t index) {
  if (cause.ok()) {
    return Status::Error(Err::CONFIG_DIRTY,
                         "Hardware config dirty after diagnostic write",
                         packDirtyDetail(phase, reg, index, cause.detail));
  }
  return Status{cause.code, packDirtyDetail(phase, reg, index, cause.detail), cause.msg};
}

} // namespace

// ============================================================================
// Lifecycle
// ============================================================================

Status LDC1614::begin(const Config& config) {
  const Config requestedConfig = config;

  _config = Config{};
  _initialized = false;
  _sleeping = true;
  _driverState = DriverState::UNINIT;
  _allowOfflineI2c = false;
  _clearHardwareConfigDirty();

  _lastOkMs = 0;
  _lastErrorMs = 0;
  _lastError = Status::Ok();
  _consecutiveFailures = 0;
  _totalFailures = 0;
  _totalSuccess = 0;
  _lastRecoverMs = 0;
  _lastRecoverValid = false;
  _chunkedJob = ChunkedJobKind::NONE;
  _chunkedConfigStep = 0;
  _chunkedReadMask = 0;
  _chunkedReadReadyMask = 0;
  _chunkedReadChannel = 0;
  _chunkedReadPhase = ChunkedReadPhase::MSB;
  _chunkedReadMsb = 0;
  _chunkedPollExecuting = false;

  for (uint8_t i = 0; i < cmd::MAX_CHANNELS; i++) {
    _lastChannelData[i] = ChannelData{};
    _hasSample[i] = false;
    _sampleTimestampMs[i] = 0;
  }

  if (requestedConfig.i2cWrite == nullptr || requestedConfig.i2cWriteRead == nullptr) {
    return Status::Error(Err::INVALID_CONFIG, "I2C callbacks required");
  }
  if (requestedConfig.i2cTimeoutMs == 0) {
    return Status::Error(Err::INVALID_CONFIG, "Timeout must be > 0");
  }
  if (requestedConfig.i2cAddress != 0x2A && requestedConfig.i2cAddress != 0x2B) {
    return Status::Error(Err::INVALID_CONFIG, "Invalid I2C address (must be 0x2A or 0x2B)");
  }
  if (requestedConfig.channelCount != 2 && requestedConfig.channelCount != 4) {
    return Status::Error(Err::INVALID_CONFIG, "channelCount must be 2 or 4");
  }
  if (requestedConfig.activeChan >= requestedConfig.channelCount) {
    return Status::Error(Err::INVALID_CONFIG, "activeChan exceeds channelCount");
  }
  if (!isValidDeglitch(requestedConfig.deglitch)) {
    return Status::Error(Err::INVALID_CONFIG, "Invalid deglitch value");
  }
  if (!isRRSequenceAllowed(requestedConfig.rrSequence, requestedConfig.channelCount)) {
    return Status::Error(Err::INVALID_CONFIG, "Invalid RR sequence for channelCount");
  }
  if (requestedConfig.autoScan) {
    Status multiTiming = validateMultiChannelTiming(requestedConfig,
                                                   requestedConfig.rrSequence,
                                                   Err::INVALID_CONFIG);
    if (!multiTiming.ok()) {
      return multiTiming;
    }
  }
  if (!isValidRefClkSrc(requestedConfig.refClkSrc)) {
    return Status::Error(Err::INVALID_CONFIG, "Invalid reference clock source");
  }
  if (!isValidSensorActivation(requestedConfig.sensorActivation)) {
    return Status::Error(Err::INVALID_CONFIG, "Invalid sensor activation mode");
  }
  if (requestedConfig.intbPin < -1) {
    return Status::Error(Err::INVALID_CONFIG, "Invalid INTB pin");
  }
  if (requestedConfig.intbPin >= 0 && requestedConfig.gpioRead == nullptr) {
    return Status::Error(Err::INVALID_CONFIG, "INTB gpioRead required");
  }
  if (requestedConfig.highCurrentDrv && requestedConfig.autoScan) {
    return Status::Error(Err::INVALID_CONFIG, "HIGH_CURRENT_DRV only in single-channel mode");
  }
  if (requestedConfig.highCurrentDrv && requestedConfig.activeChan != 0) {
    return Status::Error(Err::INVALID_CONFIG, "HIGH_CURRENT_DRV only on Ch0");
  }
  if (!isValidErrorConfig(requestedConfig.errorConfig)) {
    return Status::Error(Err::INVALID_CONFIG, "ERROR_CONFIG has reserved bits set");
  }

  // Validate per-channel config
  for (uint8_t ch = 0; ch < requestedConfig.channelCount; ch++) {
    const auto& cc = requestedConfig.channel[ch];
    if (!isValidRcount(cc.rcount)) {
      return Status::Error(Err::INVALID_CONFIG, "RCOUNT below minimum (0x0005)");
    }
    if (!isValidFinDivider(cc.finDivider)) {
      return Status::Error(Err::INVALID_CONFIG, "Invalid FIN_DIVIDER");
    }
    if (!isValidFrefDivider(cc.frefDivider)) {
      return Status::Error(Err::INVALID_CONFIG, "Invalid FREF_DIVIDER");
    }
    if (!isValidIdrive(cc.idrive)) {
      return Status::Error(Err::INVALID_CONFIG, "IDRIVE exceeds maximum (31)");
    }
  }

  _config = requestedConfig;
  if (_config.offlineThreshold == 0) {
    _config.offlineThreshold = 1;
  }

  // Verify device identity
  Status st = probe();
  if (!st.ok()) {
    return st;
  }

  // Apply full configuration
  st = _applyConfig();
  if (!st.ok()) {
    return st;
  }

  _initialized = true;
  _driverState = DriverState::READY;
  return Status::Ok();
}

void LDC1614::tick(uint32_t nowMs) {
  (void)nowMs;
  if (!_initialized) {
    return;
  }
  // LDC1614 has no state machine work needed in tick().
  // Data readback is synchronous via readChannel() / readAllChannels().
  // Application polls readDataReady() or uses INTB pin for notification.
}

void LDC1614::end() {
  // Best-effort: put device into sleep mode before shutting down
  if (_initialized && !_sleeping) {
    const uint16_t configReg = _buildConfigRegister(true);  // sleepMode=true
    const uint8_t buf[3] = {
        cmd::REG_CONFIG,
        static_cast<uint8_t>(configReg >> 8),
        static_cast<uint8_t>(configReg & 0xFF)
    };
    (void)_i2cWriteRaw(buf, sizeof(buf));  // ignore errors
  }
  _initialized = false;
  _sleeping = true;
  _driverState = DriverState::UNINIT;
  _chunkedJob = ChunkedJobKind::NONE;
  _chunkedConfigStep = 0;
  _chunkedReadMask = 0;
  _chunkedReadReadyMask = 0;
  _chunkedReadChannel = 0;
  _chunkedReadPhase = ChunkedReadPhase::MSB;
  _chunkedReadMsb = 0;
  _chunkedPollExecuting = false;
}

// ============================================================================
// Diagnostics
// ============================================================================

Status LDC1614::probe() {
  if (_initialized && _chunkedJob != ChunkedJobKind::NONE) {
    return Status::Error(Err::BUSY, "Poll-chunked job already active");
  }

  auto probeReadFailure = [](const Status& failure) -> Status {
    if (failure.code == Err::INVALID_CONFIG || failure.code == Err::INVALID_PARAM) {
      return failure;
    }
    if (failure.code == Err::I2C_NACK_ADDR) {
      return Status::Error(Err::DEVICE_NOT_FOUND, "LDC1614 not responding", failure.detail);
    }
    return failure;
  };

  uint16_t mfgId = 0;
  Status st = _readRegister16Raw(cmd::REG_MANUFACTURER_ID, mfgId);
  if (!st.ok()) {
    return probeReadFailure(st);
  }
  if (mfgId != cmd::MANUFACTURER_ID_VALUE) {
    return Status::Error(Err::DEVICE_NOT_FOUND, "Wrong MANUFACTURER_ID",
                         static_cast<int32_t>(mfgId));
  }

  uint16_t devId = 0;
  st = _readRegister16Raw(cmd::REG_DEVICE_ID, devId);
  if (!st.ok()) {
    return probeReadFailure(st);
  }
  if (devId != cmd::DEVICE_ID_VALUE) {
    return Status::Error(Err::DEVICE_NOT_FOUND, "Wrong DEVICE_ID",
                         static_cast<int32_t>(devId));
  }

  return Status::Ok();
}

Status LDC1614::recover() {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (_chunkedJob != ChunkedJobKind::NONE) {
    return Status::Error(Err::BUSY, "Poll-chunked job already active");
  }
  const bool startedOffline = _driverState == DriverState::OFFLINE;
  ScopedOfflineI2cAllowance allowOfflineI2c(_allowOfflineI2c, true);
  Status st = _performRecoveryLadder();
  if (startedOffline && !st.ok() && !st.inProgress()) {
    _reassertOfflineLatch();
  }
  return st;
}

// ============================================================================
// Data Readback
// ============================================================================

Status LDC1614::readChannel(uint8_t ch, ChannelData& out) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!isValidChannel(ch, _config.channelCount)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid channel");
  }
  Status allowed = _ensureNormalI2cAllowed();
  if (!allowed.ok()) {
    return allowed;
  }
  if (_sleeping) {
    return Status::Error(Err::BUSY, "Device is sleeping; wake first");
  }

  // Read MSB first (latches both MSB and LSB from shadow register)
  uint16_t msb = 0;
  Status st = readRegister16(cmd::regDataMsb(ch), msb);
  if (!st.ok()) {
    return st;
  }

  // Read LSB
  uint16_t lsb = 0;
  st = readRegister16(cmd::regDataLsb(ch), lsb);
  if (!st.ok()) {
    return st;
  }

  return _storeChannelData(ch, msb, lsb, out);
}

Status LDC1614::readAllChannels(ChannelData* out, uint8_t count) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (out == nullptr) {
    return Status::Error(Err::INVALID_PARAM, "Output buffer is null");
  }

  uint8_t n = (count > 0) ? count : _config.channelCount;
  if (n > _config.channelCount) {
    return Status::Error(Err::INVALID_PARAM, "Count exceeds channelCount");
  }

  for (uint8_t ch = 0; ch < n; ch++) {
    Status st = readChannel(ch, out[ch]);
    if (!st.ok()) {
      return st;
    }
  }

  return Status::Ok();
}

Status LDC1614::readFreshChannels(FreshChannelData* out, uint8_t count) {
  DeviceStatus status;
  return readFreshChannels(out, status, count);
}

Status LDC1614::readFreshChannels(FreshChannelData* out, DeviceStatus& statusOut,
                                  uint8_t count) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (out == nullptr) {
    return Status::Error(Err::INVALID_PARAM, "Output buffer is null");
  }

  const uint8_t n = (count > 0) ? count : _config.channelCount;
  if (n > _config.channelCount) {
    return Status::Error(Err::INVALID_PARAM, "Count exceeds channelCount");
  }
  Status allowed = _ensureNormalI2cAllowed();
  if (!allowed.ok()) {
    return allowed;
  }
  if (_sleeping) {
    return Status::Error(Err::BUSY, "Device is sleeping; wake first");
  }

  for (uint8_t ch = 0; ch < n; ++ch) {
    out[ch] = FreshChannelData{};
  }

  statusOut = DeviceStatus{};
  Status st = readDeviceStatus(statusOut);
  if (!st.ok()) {
    return st;
  }

  for (uint8_t ch = 0; ch < n; ++ch) {
    if (statusOut.unreadConv[ch]) {
      ChannelData data;
      st = readChannel(ch, data);
      if (!st.ok()) {
        return st;
      }
      out[ch].data = data;
      out[ch].valid = true;
      out[ch].fresh = true;
    } else if (hasSample(ch)) {
      out[ch].data = _lastChannelData[ch];
      out[ch].valid = true;
      out[ch].fresh = false;
    }
  }

  return Status::Ok();
}

bool LDC1614::dataReady() {
  bool ready = false;
  Status st = readDataReady(ready);
  return st.ok() && ready;
}

Status LDC1614::readDataReady(bool& ready) {
  ready = false;
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  Status allowed = _ensureNormalI2cAllowed();
  if (!allowed.ok()) {
    return allowed;
  }

  // Check INTB pin if configured (active low). The no-I2C high-level fast path
  // is valid only when DRDY is routed to INTB; otherwise poll STATUS for DRDY.
  if (_config.intbPin >= 0 && _config.gpioRead != nullptr && !_config.intbDisable) {
    const bool level = _config.gpioRead(_config.intbPin, _config.gpioUser);
    const bool drdyRoutedToIntb =
        (_config.errorConfig & cmd::MASK_ERRCFG_DRDY_2INT) != 0U;
    if (level && drdyRoutedToIntb) {
      return Status::Ok();
    }
    if (!level) {
      // INTB can be asserted by DRDY or by enabled error conditions. Read
      // STATUS to distinguish the source instead of treating every interrupt
      // as data.
      DeviceStatus status;
      Status st = readDeviceStatus(status);
      if (!st.ok()) {
        return st;
      }
      ready = status.dataReady;
      if (status.hasError()) {
        return Status::Error(Err::SENSOR_ERROR, "INTB asserted by sensor error",
                             static_cast<int32_t>(status.raw));
      }
      return Status::Ok();
    }
  }

  // Fall back to polling STATUS register DRDY bit
  DeviceStatus status;
  Status st = readDeviceStatus(status);
  if (!st.ok()) {
    return st;
  }
  ready = status.dataReady;
  if (status.hasError()) {
    return Status::Error(Err::SENSOR_ERROR, "STATUS reported sensor error",
                         static_cast<int32_t>(status.raw));
  }
  return Status::Ok();
}

// ============================================================================
// Status Register
// ============================================================================

Status LDC1614::readDeviceStatus(DeviceStatus& out) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }

  uint16_t raw = 0;
  Status st = readRegister16(cmd::REG_STATUS, raw);
  if (!st.ok()) {
    return st;
  }

  out.raw = raw;
  out.errChan         = static_cast<uint8_t>((raw & cmd::MASK_STATUS_ERR_CHAN) >> cmd::BIT_STATUS_ERR_CHAN);
  out.errUnderRange   = (raw & cmd::MASK_STATUS_ERR_UR) != 0;
  out.errOverRange    = (raw & cmd::MASK_STATUS_ERR_OR) != 0;
  out.errWatchdog     = (raw & cmd::MASK_STATUS_ERR_WD) != 0;
  out.errAmplitudeHigh = (raw & cmd::MASK_STATUS_ERR_AHE) != 0;
  out.errAmplitudeLow = (raw & cmd::MASK_STATUS_ERR_ALE) != 0;
  out.errZeroCount    = (raw & cmd::MASK_STATUS_ERR_ZC) != 0;
  out.dataReady       = (raw & cmd::MASK_STATUS_DRDY) != 0;
  out.unreadConv[0]   = (raw & cmd::MASK_STATUS_UNREADCONV0) != 0;
  out.unreadConv[1]   = (raw & cmd::MASK_STATUS_UNREADCONV1) != 0;
  out.unreadConv[2]   = (raw & cmd::MASK_STATUS_UNREADCONV2) != 0;
  out.unreadConv[3]   = (raw & cmd::MASK_STATUS_UNREADCONV3) != 0;

  return Status::Ok();
}

Status LDC1614::readStatusRaw(uint16_t& out) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  return readRegister16(cmd::REG_STATUS, out);
}

// ============================================================================
// Control
// ============================================================================

Status LDC1614::sleep() {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (_sleeping) {
    return Status::Ok();
  }

  uint16_t configReg = _buildConfigRegister(true);
  Status st = _writeConfigRegister(cmd::REG_CONFIG, configReg,
                                   DIRTY_PHASE_SETTER_SINGLE, DIRTY_INDEX_GLOBAL);
  if (!st.ok()) {
    return st;
  }

  _sleeping = true;
  return Status::Ok();
}

Status LDC1614::wake() {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Ok();
  }

  uint16_t configReg = _buildConfigRegister(false);
  Status st = _writeConfigRegister(cmd::REG_CONFIG, configReg,
                                   DIRTY_PHASE_SETTER_SINGLE, DIRTY_INDEX_GLOBAL);
  if (!st.ok()) {
    return st;
  }

  _sleeping = false;
  return Status::Ok();
}

Status LDC1614::softReset() {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }

  ScopedOfflineI2cAllowance allowOfflineI2c(_allowOfflineI2c, true);
  Status st = _writeConfigRegister(cmd::REG_RESET_DEV, cmd::MASK_RESET_DEV,
                                   DIRTY_PHASE_RESET, DIRTY_INDEX_GLOBAL);
  if (!st.ok()) {
    return st;
  }

  _markHardwareConfigDirty(Status::Ok(), DIRTY_PHASE_RESET,
                           cmd::REG_RESET_DEV, DIRTY_INDEX_GLOBAL);
  _initialized = false;
  _sleeping = true;
  _driverState = DriverState::UNINIT;
  return Status::Ok();
}

Status LDC1614::resetAndReapply() {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }

  // Save config before reset (softReset clears _initialized)
  Config savedConfig = _config;

  ScopedOfflineI2cAllowance allowOfflineI2c(_allowOfflineI2c, true);
  Status st = [this, &savedConfig]() -> Status {
    Status inner = _writeRegister16Tracked(cmd::REG_RESET_DEV, cmd::MASK_RESET_DEV);
    if (!inner.ok()) {
      _markHardwareConfigDirty(inner, DIRTY_PHASE_RESET, cmd::REG_RESET_DEV,
                               DIRTY_INDEX_GLOBAL);
      return inner;
    }
    inner = _verifyIdentityTracked();
    if (!inner.ok()) {
      _markHardwareConfigDirty(inner, DIRTY_PHASE_RESET,
                               cmd::REG_RESET_DEV, DIRTY_INDEX_GLOBAL);
      _initialized = false;
      _driverState = DriverState::UNINIT;
      _sleeping = true;
      return inner;
    }

    // Restore state for re-initialization
    _config = savedConfig;
    _sleeping = true;

    // Re-apply full configuration
    inner = _applyConfig();
    if (!inner.ok()) {
      _initialized = false;
      _driverState = DriverState::UNINIT;
      return inner;
    }
    return Status::Ok();
  }();
  if (!st.ok()) {
    return st;
  }

  _initialized = true;
  _driverState = DriverState::READY;
  _consecutiveFailures = 0;
  _clearHardwareConfigDirty();
  return Status::Ok();
}

Status LDC1614::syncConfig() {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }

  ScopedOfflineI2cAllowance allowOfflineI2c(_allowOfflineI2c, true);
  Status st = _applyConfig();
  if (!st.ok()) {
    return st;
  }

  _initialized = true;
  _driverState = DriverState::READY;
  _consecutiveFailures = 0;
  return Status::Ok();
}

// ============================================================================
// Blocking Reads
// ============================================================================

Status LDC1614::readChannelBlocking(uint8_t ch, ChannelData& out, uint32_t timeoutMs) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!isValidChannel(ch, _config.channelCount)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid channel");
  }
  if (_config.nowMs == nullptr) {
    return Status::Error(Err::INVALID_CONFIG, "nowMs callback required for blocking reads");
  }
  if (_sleeping) {
    return Status::Error(Err::BUSY, "Device is sleeping; wake first");
  }

  const uint32_t startMs = _nowMs();
  const uint32_t maxPolls = (timeoutMs > (UINT32_MAX - 2U)) ? UINT32_MAX : (timeoutMs + 2U);
  uint32_t polls = 0;
  bool readySeen = false;
  while (polls < maxPolls) {
    bool ready = false;
    Status st = readDataReady(ready);
    if (!st.ok()) {
      return st;
    }
    if (ready) {
      readySeen = true;
      break;
    }
    if (static_cast<uint32_t>(_nowMs() - startMs) >= timeoutMs) {
      return Status::Error(Err::TIMEOUT, "Data ready timeout");
    }
    polls++;
    _cooperativeYield();
  }

  if (!readySeen) {
    return Status::Error(Err::TIMEOUT, "Data ready timeout");
  }

  return readChannel(ch, out);
}

Status LDC1614::readAllChannelsBlocking(ChannelData* out, uint32_t timeoutMs, uint8_t count) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (out == nullptr) {
    return Status::Error(Err::INVALID_PARAM, "Output buffer is null");
  }
  const uint8_t n = (count > 0) ? count : _config.channelCount;
  if (n > _config.channelCount) {
    return Status::Error(Err::INVALID_PARAM, "Count exceeds channelCount");
  }
  if (_config.nowMs == nullptr) {
    return Status::Error(Err::INVALID_CONFIG, "nowMs callback required for blocking reads");
  }
  if (_sleeping) {
    return Status::Error(Err::BUSY, "Device is sleeping; wake first");
  }

  const uint32_t startMs = _nowMs();
  const uint32_t maxPolls = (timeoutMs > (UINT32_MAX - 2U)) ? UINT32_MAX : (timeoutMs + 2U);
  uint32_t polls = 0;
  bool readySeen = false;
  while (polls < maxPolls) {
    bool ready = false;
    Status st = readDataReady(ready);
    if (!st.ok()) {
      return st;
    }
    if (ready) {
      readySeen = true;
      break;
    }
    if (static_cast<uint32_t>(_nowMs() - startMs) >= timeoutMs) {
      return Status::Error(Err::TIMEOUT, "Data ready timeout");
    }
    polls++;
    _cooperativeYield();
  }

  if (!readySeen) {
    return Status::Error(Err::TIMEOUT, "Data ready timeout");
  }

  return readAllChannels(out, n);
}

// ============================================================================
// Poll-chunked operations
// ============================================================================

Status LDC1614::startReadChannels(uint8_t mask) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (_chunkedJob != ChunkedJobKind::NONE) {
    return Status::Error(Err::BUSY, "Poll-chunked job already active");
  }
  const uint8_t validMask = static_cast<uint8_t>((1U << _config.channelCount) - 1U);
  if (mask == 0 || (mask & ~validMask) != 0) {
    return Status::Error(Err::INVALID_PARAM, "Invalid channel mask");
  }
  Status allowed = _ensureNormalI2cAllowed();
  if (!allowed.ok()) {
    return allowed;
  }
  if (_sleeping) {
    return Status::Error(Err::BUSY, "Device is sleeping; wake first");
  }

  _chunkedReadReadyMask = 0;
  _chunkedReadMask = mask;
  _chunkedReadChannel = _firstSelectedChannel(mask);
  _chunkedReadPhase = ChunkedReadPhase::MSB;
  _chunkedReadMsb = 0;
  return _startChunkedJob(ChunkedJobKind::READ_CHANNELS);
}

Status LDC1614::poll(uint32_t nowMs, uint8_t maxInstructions) {
  (void)nowMs;
  if (_chunkedJob == ChunkedJobKind::NONE) {
    return Status::Ok();
  }
  if (maxInstructions == 0) {
    return Status::Error(Err::IN_PROGRESS, "Poll-chunked job in progress");
  }

  const bool oldPollExecuting = _chunkedPollExecuting;
  _chunkedPollExecuting = true;
  uint8_t remaining = maxInstructions;
  Status st = Status::Ok();
  while (remaining > 0 && _chunkedJob != ChunkedJobKind::NONE) {
    switch (_chunkedJob) {
      case ChunkedJobKind::READ_CHANNELS:
        st = _pollReadChannels(remaining);
        break;
      case ChunkedJobKind::APPLY_CONFIG:
        st = _pollApplyConfig(remaining);
        break;
      case ChunkedJobKind::RESET_AND_REAPPLY:
        st = _pollResetAndReapply(remaining);
        break;
      case ChunkedJobKind::NONE:
        st = Status::Ok();
        break;
    }
    if (!st.ok() && !st.inProgress()) {
      _chunkedPollExecuting = oldPollExecuting;
      return st;
    }
    if (st.inProgress() && remaining == 0) {
      break;
    }
  }

  if (_chunkedJob == ChunkedJobKind::NONE) {
    _chunkedPollExecuting = oldPollExecuting;
    return Status::Ok();
  }
  _chunkedPollExecuting = oldPollExecuting;
  return Status::Error(Err::IN_PROGRESS, "Poll-chunked job in progress");
}

bool LDC1614::readChannelsReady() const {
  return _chunkedJob == ChunkedJobKind::NONE &&
         _chunkedReadMask != 0 &&
         _chunkedReadReadyMask == _chunkedReadMask;
}

Status LDC1614::getChannelSample(uint8_t ch, ChannelData& out) const {
  if (!isValidChannel(ch, _config.channelCount)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid channel");
  }
  if (!readChannelsReady() || (_chunkedReadReadyMask & (1U << ch)) == 0) {
    return Status::Error(Err::CONVERSION_NOT_READY, "Chunked channel sample not ready");
  }
  out = _lastChannelData[ch];
  return Status::Ok();
}

Status LDC1614::startApplyConfig() {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (_chunkedJob != ChunkedJobKind::NONE) {
    return Status::Error(Err::BUSY, "Poll-chunked job already active");
  }
  Status allowed = _ensureNormalI2cAllowed();
  if (!allowed.ok()) {
    return allowed;
  }
  _chunkedConfigStep = 0;
  return _startChunkedJob(ChunkedJobKind::APPLY_CONFIG);
}

Status LDC1614::startResetAndReapply() {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (_chunkedJob != ChunkedJobKind::NONE) {
    return Status::Error(Err::BUSY, "Poll-chunked job already active");
  }
  Status allowed = _ensureNormalI2cAllowed();
  if (!allowed.ok()) {
    return allowed;
  }
  _chunkedConfigStep = 0;
  return _startChunkedJob(ChunkedJobKind::RESET_AND_REAPPLY);
}

// ============================================================================
// Sample Cache
// ============================================================================

Status LDC1614::getLastSample(uint8_t ch, ChannelData& out) const {
  if (!isValidChannel(ch, _config.channelCount)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid channel");
  }
  if (!_hasSample[ch]) {
    return Status::Error(Err::CONVERSION_NOT_READY, "No cached sample for channel");
  }
  out = _lastChannelData[ch];
  return Status::Ok();
}

bool LDC1614::hasSample(uint8_t ch) const {
  return isValidChannel(ch, _config.channelCount) && _hasSample[ch];
}

uint32_t LDC1614::sampleTimestampMs(uint8_t ch) const {
  if (!isValidChannel(ch, _config.channelCount)) {
    return 0;
  }
  return _sampleTimestampMs[ch];
}

uint32_t LDC1614::sampleAgeMs(uint8_t ch, uint32_t nowMs) const {
  if (!isValidChannel(ch, _config.channelCount) || !_hasSample[ch]) {
    return 0;
  }
  return nowMs - _sampleTimestampMs[ch];
}

// ============================================================================
// Settings Snapshot
// ============================================================================

Status LDC1614::getSettings(SettingsSnapshot& out) const {
  out.initialized = _initialized;
  out.state = _driverState;
  out.i2cAddress = _config.i2cAddress;
  out.i2cTimeoutMs = _config.i2cTimeoutMs;
  out.offlineThreshold = _config.offlineThreshold;
  out.hasNowMsHook = (_config.nowMs != nullptr);
  out.hasGpioReadHook = (_config.gpioRead != nullptr);
  out.hasCooperativeYieldHook = (_config.cooperativeYield != nullptr);
  out.sleeping = _sleeping;
  out.autoScan = _config.autoScan;
  out.activeChan = _config.activeChan;
  out.channelCount = _config.channelCount;
  out.rrSequence = _config.rrSequence;
  out.deglitch = _config.deglitch;
  out.refClkSrc = _config.refClkSrc;
  out.sensorActivation = _config.sensorActivation;
  out.rpOverrideEn = _config.rpOverrideEn;
  out.autoAmpDis = _config.autoAmpDis;
  out.highCurrentDrv = _config.highCurrentDrv;
  out.intbEnabled = (_config.intbPin >= 0 && !_config.intbDisable);
  out.hardwareConfigDirty = _hardwareConfigDirty;
  out.hardwareConfigDirtyError = _hardwareConfigDirtyError;
  for (uint8_t i = 0; i < cmd::MAX_CHANNELS; i++) {
    out.hasSample[i] = _hasSample[i];
    out.sampleTimestampMs[i] = _sampleTimestampMs[i];
    out.channel[i] = _config.channel[i];
  }
  return Status::Ok();
}

// ============================================================================
// Runtime Configuration
// ============================================================================

Status LDC1614::setActiveChannel(uint8_t ch) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Error(Err::BUSY, "Must be in sleep mode to change config");
  }
  if (!isValidChannel(ch, _config.channelCount)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid channel");
  }
  if (_config.highCurrentDrv && ch != 0) {
    return Status::Error(Err::INVALID_PARAM, "HIGH_CURRENT_DRV only on Ch0");
  }

  const uint8_t oldActive = _config.activeChan;
  _config.activeChan = ch;
  const uint16_t configReg = _buildConfigRegister(true);
  _config.activeChan = oldActive;

  Status st = _writeConfigRegister(cmd::REG_CONFIG, configReg,
                                   DIRTY_PHASE_SETTER_SINGLE, DIRTY_INDEX_GLOBAL);
  if (st.ok()) {
    _config.activeChan = ch;
  }
  return st;
}

Status LDC1614::setSingleChannelMode(uint8_t ch) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Error(Err::BUSY, "Must be in sleep mode to change config");
  }
  if (!isValidChannel(ch, _config.channelCount)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid channel");
  }
  if (_config.highCurrentDrv && ch != 0) {
    return Status::Error(Err::INVALID_PARAM, "HIGH_CURRENT_DRV only on Ch0");
  }

  const bool oldAutoScan = _config.autoScan;
  const uint8_t oldActive = _config.activeChan;
  _config.autoScan = false;
  _config.activeChan = ch;
  const uint16_t muxReg = _buildMuxConfigRegister();
  const uint16_t configReg = _buildConfigRegister(true);
  _config.autoScan = oldAutoScan;
  _config.activeChan = oldActive;

  Status st = _writeConfigRegister(cmd::REG_MUX_CONFIG, muxReg,
                                   DIRTY_PHASE_SETTER_MULTI_FIRST,
                                   DIRTY_INDEX_GLOBAL);
  if (!st.ok()) {
    return st;
  }
  st = _writeConfigRegister(cmd::REG_CONFIG, configReg,
                            DIRTY_PHASE_SETTER_MULTI_SECOND,
                            DIRTY_INDEX_GLOBAL);
  if (st.ok()) {
    _config.autoScan = false;
    _config.activeChan = ch;
  }
  return st;
}

Status LDC1614::setAutoScanMode(RRSequence sequence) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Error(Err::BUSY, "Must be in sleep mode to change config");
  }
  if (!isRRSequenceAllowed(sequence, _config.channelCount)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid RR sequence for channelCount");
  }
  if (_config.highCurrentDrv) {
    return Status::Error(Err::INVALID_PARAM, "Disable HIGH_CURRENT_DRV before auto-scan");
  }
  Status multiTiming = validateMultiChannelTiming(_config, sequence, Err::INVALID_PARAM);
  if (!multiTiming.ok()) {
    return multiTiming;
  }

  const bool oldAutoScan = _config.autoScan;
  const RRSequence oldSequence = _config.rrSequence;
  _config.autoScan = true;
  _config.rrSequence = sequence;
  const uint16_t muxReg = _buildMuxConfigRegister();
  _config.autoScan = oldAutoScan;
  _config.rrSequence = oldSequence;

  Status st = _writeConfigRegister(cmd::REG_MUX_CONFIG, muxReg,
                                   DIRTY_PHASE_SETTER_SINGLE, DIRTY_INDEX_GLOBAL);
  if (st.ok()) {
    _config.autoScan = true;
    _config.rrSequence = sequence;
  }
  return st;
}

Status LDC1614::setDeglitch(Deglitch deglitch) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Error(Err::BUSY, "Must be in sleep mode to change config");
  }
  if (!isValidDeglitch(deglitch)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid deglitch value");
  }

  const Deglitch oldDeglitch = _config.deglitch;
  _config.deglitch = deglitch;
  const uint16_t muxReg = _buildMuxConfigRegister();
  _config.deglitch = oldDeglitch;

  Status st = _writeConfigRegister(cmd::REG_MUX_CONFIG, muxReg,
                                   DIRTY_PHASE_SETTER_SINGLE, DIRTY_INDEX_GLOBAL);
  if (st.ok()) {
    _config.deglitch = deglitch;
  }
  return st;
}

Status LDC1614::setErrorConfig(uint16_t errorConfig) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Error(Err::BUSY, "Must be in sleep mode to change config");
  }
  if (!isValidErrorConfig(errorConfig)) {
    return Status::Error(Err::INVALID_PARAM, "ERROR_CONFIG has reserved bits set");
  }

  Status st = _writeConfigRegister(cmd::REG_ERROR_CONFIG, errorConfig,
                                   DIRTY_PHASE_SETTER_SINGLE, DIRTY_INDEX_GLOBAL);
  if (st.ok()) {
    _config.errorConfig = errorConfig;
  }
  return st;
}

Status LDC1614::setIntbDisabled(bool disabled) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Error(Err::BUSY, "Must be in sleep mode to change config");
  }

  const bool oldValue = _config.intbDisable;
  _config.intbDisable = disabled;
  const uint16_t configReg = _buildConfigRegister(true);
  _config.intbDisable = oldValue;

  Status st = _writeConfigRegister(cmd::REG_CONFIG, configReg,
                                   DIRTY_PHASE_SETTER_SINGLE, DIRTY_INDEX_GLOBAL);
  if (st.ok()) {
    _config.intbDisable = disabled;
  }
  return st;
}

Status LDC1614::setReferenceClockSource(RefClkSrc source) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Error(Err::BUSY, "Must be in sleep mode to change config");
  }
  if (!isValidRefClkSrc(source)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid reference clock source");
  }

  const RefClkSrc oldSource = _config.refClkSrc;
  _config.refClkSrc = source;
  const uint16_t configReg = _buildConfigRegister(true);
  _config.refClkSrc = oldSource;

  Status st = _writeConfigRegister(cmd::REG_CONFIG, configReg,
                                   DIRTY_PHASE_SETTER_SINGLE, DIRTY_INDEX_GLOBAL);
  if (st.ok()) {
    _config.refClkSrc = source;
  }
  return st;
}

Status LDC1614::setSensorActivation(SensorActivation activation) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Error(Err::BUSY, "Must be in sleep mode to change config");
  }
  if (!isValidSensorActivation(activation)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid sensor activation mode");
  }

  const SensorActivation oldActivation = _config.sensorActivation;
  _config.sensorActivation = activation;
  const uint16_t configReg = _buildConfigRegister(true);
  _config.sensorActivation = oldActivation;

  Status st = _writeConfigRegister(cmd::REG_CONFIG, configReg,
                                   DIRTY_PHASE_SETTER_SINGLE, DIRTY_INDEX_GLOBAL);
  if (st.ok()) {
    _config.sensorActivation = activation;
  }
  return st;
}

Status LDC1614::setRpOverrideEnabled(bool enabled) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Error(Err::BUSY, "Must be in sleep mode to change config");
  }

  const bool oldValue = _config.rpOverrideEn;
  _config.rpOverrideEn = enabled;
  const uint16_t configReg = _buildConfigRegister(true);
  _config.rpOverrideEn = oldValue;

  Status st = _writeConfigRegister(cmd::REG_CONFIG, configReg,
                                   DIRTY_PHASE_SETTER_SINGLE, DIRTY_INDEX_GLOBAL);
  if (st.ok()) {
    _config.rpOverrideEn = enabled;
  }
  return st;
}

Status LDC1614::setAutoAmplitudeCorrectionEnabled(bool enabled) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Error(Err::BUSY, "Must be in sleep mode to change config");
  }

  const bool oldValue = _config.autoAmpDis;
  _config.autoAmpDis = !enabled;
  const uint16_t configReg = _buildConfigRegister(true);
  _config.autoAmpDis = oldValue;

  Status st = _writeConfigRegister(cmd::REG_CONFIG, configReg,
                                   DIRTY_PHASE_SETTER_SINGLE, DIRTY_INDEX_GLOBAL);
  if (st.ok()) {
    _config.autoAmpDis = !enabled;
  }
  return st;
}

Status LDC1614::setHighCurrentDriveEnabled(bool enabled) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Error(Err::BUSY, "Must be in sleep mode to change config");
  }
  if (enabled && _config.autoScan) {
    return Status::Error(Err::INVALID_PARAM, "HIGH_CURRENT_DRV only in single-channel mode");
  }
  if (enabled && _config.activeChan != 0) {
    return Status::Error(Err::INVALID_PARAM, "HIGH_CURRENT_DRV only on Ch0");
  }

  const bool oldValue = _config.highCurrentDrv;
  _config.highCurrentDrv = enabled;
  const uint16_t configReg = _buildConfigRegister(true);
  _config.highCurrentDrv = oldValue;

  Status st = _writeConfigRegister(cmd::REG_CONFIG, configReg,
                                   DIRTY_PHASE_SETTER_SINGLE, DIRTY_INDEX_GLOBAL);
  if (st.ok()) {
    _config.highCurrentDrv = enabled;
  }
  return st;
}

Status LDC1614::setRcount(uint8_t ch, uint16_t rcount) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Error(Err::BUSY, "Must be in sleep mode to change config");
  }
  if (!isValidChannel(ch, _config.channelCount)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid channel");
  }
  if (!isValidRcount(rcount)) {
    return Status::Error(Err::INVALID_PARAM, "RCOUNT below minimum (0x0005)");
  }
  if (_config.autoScan && isInAutoscanSequence(ch, _config.rrSequence) &&
      !isValidMultiChannelRcount(rcount)) {
    return Status::Error(Err::INVALID_PARAM, "RCOUNT below multi-channel minimum (0x0009)");
  }

  Status st = _writeConfigRegister(cmd::regRcount(ch), rcount,
                                   DIRTY_PHASE_SETTER_SINGLE, ch);
  if (st.ok()) {
    _config.channel[ch].rcount = rcount;
  }
  return st;
}

Status LDC1614::setSettleCount(uint8_t ch, uint16_t count) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Error(Err::BUSY, "Must be in sleep mode to change config");
  }
  if (!isValidChannel(ch, _config.channelCount)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid channel");
  }
  if (_config.autoScan && isInAutoscanSequence(ch, _config.rrSequence) &&
      !isValidMultiChannelSettleCount(count)) {
    return Status::Error(Err::INVALID_PARAM, "SETTLECOUNT below multi-channel minimum (0x0004)");
  }

  Status st = _writeConfigRegister(cmd::regSettleCount(ch), count,
                                   DIRTY_PHASE_SETTER_SINGLE, ch);
  if (st.ok()) {
    _config.channel[ch].settleCount = count;
  }
  return st;
}

Status LDC1614::setClockDividers(uint8_t ch, uint8_t finDiv, uint16_t frefDiv) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Error(Err::BUSY, "Must be in sleep mode to change config");
  }
  if (!isValidChannel(ch, _config.channelCount)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid channel");
  }
  if (!isValidFinDivider(finDiv)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid FIN_DIVIDER (1-15)");
  }
  if (!isValidFrefDivider(frefDiv)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid FREF_DIVIDER (1-1023)");
  }

  uint16_t regVal = (static_cast<uint16_t>(finDiv) << cmd::BIT_FIN_DIVIDER) |
                    (frefDiv & cmd::MASK_FREF_DIVIDER);
  Status st = _writeConfigRegister(cmd::regClockDividers(ch), regVal,
                                   DIRTY_PHASE_SETTER_SINGLE, ch);
  if (st.ok()) {
    _config.channel[ch].finDivider = finDiv;
    _config.channel[ch].frefDivider = frefDiv;
  }
  return st;
}

Status LDC1614::setOffset(uint8_t ch, uint16_t offset) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Error(Err::BUSY, "Must be in sleep mode to change config");
  }
  if (!isValidChannel(ch, _config.channelCount)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid channel");
  }

  Status st = _writeConfigRegister(cmd::regOffset(ch), offset,
                                   DIRTY_PHASE_SETTER_SINGLE, ch);
  if (st.ok()) {
    _config.channel[ch].offset = offset;
  }
  return st;
}

Status LDC1614::setDriveCurrent(uint8_t ch, uint8_t idrive) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!_sleeping) {
    return Status::Error(Err::BUSY, "Must be in sleep mode to change config");
  }
  if (!isValidChannel(ch, _config.channelCount)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid channel");
  }
  if (!isValidIdrive(idrive)) {
    return Status::Error(Err::INVALID_PARAM, "IDRIVE exceeds maximum (31)");
  }

  // INIT_IDRIVE (bits 10:6) must be written as 0; reserved bits (5:0) must be 0
  uint16_t regVal = static_cast<uint16_t>(idrive) << cmd::BIT_IDRIVE;
  Status st = _writeConfigRegister(cmd::regDriveCurrent(ch), regVal,
                                   DIRTY_PHASE_SETTER_SINGLE, ch);
  if (st.ok()) {
    _config.channel[ch].idrive = idrive;
  }
  return st;
}

Status LDC1614::readInitIdrive(uint8_t ch, uint8_t& out) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!isValidChannel(ch, _config.channelCount)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid channel");
  }

  uint16_t regVal = 0;
  Status st = readRegister16(cmd::regDriveCurrent(ch), regVal);
  if (!st.ok()) {
    return st;
  }

  out = static_cast<uint8_t>((regVal & cmd::MASK_INIT_IDRIVE) >> cmd::BIT_INIT_IDRIVE);
  return Status::Ok();
}

// ============================================================================
// Utility
// ============================================================================

float LDC1614::calcSensorFrequency(uint8_t ch, uint32_t rawData, float fRef) const {
  if (!isValidChannel(ch, _config.channelCount) || !std::isfinite(fRef) || fRef <= 0.0f) {
    return 0.0f;
  }

  const auto& cc = _config.channel[ch];
  float fRefCh = fRef / static_cast<float>(cc.frefDivider);
  float fOffset = (static_cast<float>(cc.offset) / 65536.0f) * fRefCh;
  float fSensor = (static_cast<float>(rawData) / 268435456.0f) * fRefCh * static_cast<float>(cc.finDivider)
                  + fOffset * static_cast<float>(cc.finDivider);
  return fSensor;
}

float LDC1614::calcConversionTimeUs(uint8_t ch, float fRef) const {
  if (!isValidChannel(ch, _config.channelCount) || !std::isfinite(fRef) || fRef <= 0.0f) {
    return 0.0f;
  }

  const auto& cc = _config.channel[ch];
  float fRefCh = fRef / static_cast<float>(cc.frefDivider);
  if (fRefCh <= 0.0f) {
    return 0.0f;
  }

  // tCx = (RCOUNTx * 16 + 4) / fREFx
  float tConv = (static_cast<float>(cc.rcount) * 16.0f + 4.0f) / fRefCh;
  return tConv * 1e6f;  // Convert seconds to microseconds
}

float LDC1614::calcSettleTimeUs(uint8_t ch, float fRef) const {
  if (!isValidChannel(ch, _config.channelCount) || !std::isfinite(fRef) || fRef <= 0.0f) {
    return 0.0f;
  }

  const auto& cc = _config.channel[ch];
  const float fRefCh = fRef / static_cast<float>(cc.frefDivider);
  if (fRefCh <= 0.0f) {
    return 0.0f;
  }

  const float settleCycles = (cc.settleCount <= 1U)
                                 ? 32.0f
                                 : (static_cast<float>(cc.settleCount) * 16.0f);
  return (settleCycles / fRefCh) * 1e6f;
}

float LDC1614::calcSampleTimeUs(uint8_t ch, float fRef) const {
  return calcConversionTimeUs(ch, fRef) + calcSettleTimeUs(ch, fRef);
}

// ============================================================================
// Transport Wrappers
// ============================================================================

Status LDC1614::_i2cWriteReadRaw(const uint8_t* txBuf, size_t txLen,
                                 uint8_t* rxBuf, size_t rxLen) {
  if (_config.i2cWriteRead == nullptr) {
    return Status::Error(Err::INVALID_CONFIG, "I2C read callback missing");
  }
  if (txBuf == nullptr || txLen == 0 || rxBuf == nullptr || rxLen == 0) {
    return Status::Error(Err::INVALID_PARAM, "Invalid I2C read parameters");
  }
  return _config.i2cWriteRead(_config.i2cAddress, txBuf, txLen,
                              rxBuf, rxLen, _config.i2cTimeoutMs,
                              _config.i2cUser);
}

Status LDC1614::_i2cWriteRaw(const uint8_t* buf, size_t len) {
  if (_config.i2cWrite == nullptr) {
    return Status::Error(Err::INVALID_CONFIG, "I2C write callback missing");
  }
  if (buf == nullptr || len == 0) {
    return Status::Error(Err::INVALID_PARAM, "Invalid I2C write parameters");
  }
  return _config.i2cWrite(_config.i2cAddress, buf, len,
                          _config.i2cTimeoutMs, _config.i2cUser);
}

Status LDC1614::_i2cWriteReadTracked(const uint8_t* txBuf, size_t txLen,
                                     uint8_t* rxBuf, size_t rxLen) {
  Status allowed = _ensureNormalI2cAllowed();
  if (!allowed.ok()) {
    return allowed;
  }
  Status st = _i2cWriteReadRaw(txBuf, txLen, rxBuf, rxLen);
  if (st.code == Err::INVALID_CONFIG || st.code == Err::INVALID_PARAM) {
    return st;
  }
  return _updateHealth(st);
}

Status LDC1614::_i2cWriteTracked(const uint8_t* buf, size_t len) {
  Status allowed = _ensureNormalI2cAllowed();
  if (!allowed.ok()) {
    return allowed;
  }
  Status st = _i2cWriteRaw(buf, len);
  if (st.code == Err::INVALID_CONFIG || st.code == Err::INVALID_PARAM) {
    return st;
  }
  return _updateHealth(st);
}

// ============================================================================
// Register Access
// ============================================================================

Status LDC1614::readRegister16(uint8_t reg, uint16_t& value) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!isValidRegisterAddress(reg)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid register address");
  }
  return _readRegister16Tracked(reg, value);
}

Status LDC1614::writeRegister16(uint8_t reg, uint16_t value) {
  if (!_initialized) {
    return Status::Error(Err::NOT_INITIALIZED, "Driver not initialized");
  }
  if (!isValidRegisterAddress(reg)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid register address");
  }
  Status st = _writeRegister16Tracked(reg, value);
  if (st.ok()) {
    _markHardwareConfigDirty(Status::Ok(), DIRTY_PHASE_RAW_WRITE, reg,
                             DIRTY_INDEX_GLOBAL);
  } else if (st.code != Err::BUSY && st.code != Err::INVALID_CONFIG &&
             st.code != Err::INVALID_PARAM) {
    _markHardwareConfigDirty(st, DIRTY_PHASE_RAW_WRITE, reg,
                             DIRTY_INDEX_GLOBAL);
  }
  return st;
}

Status LDC1614::_readRegister16Tracked(uint8_t reg, uint16_t& value) {
  uint8_t rx[2] = {0, 0};
  Status st = _i2cWriteReadTracked(&reg, 1, rx, sizeof(rx));
  if (!st.ok()) {
    return st;
  }
  value = (static_cast<uint16_t>(rx[0]) << 8) | rx[1];
  return Status::Ok();
}

Status LDC1614::_writeRegister16Tracked(uint8_t reg, uint16_t value) {
  uint8_t tx[3] = {
    reg,
    static_cast<uint8_t>((value >> 8) & 0xFF),
    static_cast<uint8_t>(value & 0xFF)
  };
  return _i2cWriteTracked(tx, sizeof(tx));
}

Status LDC1614::_writeConfigRegister(uint8_t reg, uint16_t value,
                                     uint8_t phase, uint8_t index) {
  Status st = _writeRegister16Tracked(reg, value);
  if (!st.ok() && st.code != Err::BUSY && st.code != Err::INVALID_CONFIG &&
      st.code != Err::INVALID_PARAM) {
    _markHardwareConfigDirty(st, phase, reg, index);
  }
  return st;
}

Status LDC1614::_readRegister16Raw(uint8_t reg, uint16_t& value) {
  uint8_t rx[2] = {0, 0};
  Status st = _i2cWriteReadRaw(&reg, 1, rx, sizeof(rx));
  if (!st.ok()) {
    return st;
  }
  value = (static_cast<uint16_t>(rx[0]) << 8) | rx[1];
  return Status::Ok();
}

// ============================================================================
// Health Tracking
// ============================================================================

Status LDC1614::_updateHealth(const Status& st) {
  if (!_initialized || st.inProgress()) {
    return st;
  }

  uint32_t nowMs = _nowMs();

  if (st.ok()) {
    _lastOkMs = nowMs;
    _consecutiveFailures = 0;
    if (_totalSuccess < UINT32_MAX) {
      _totalSuccess++;
    }

    _driverState = DriverState::READY;
  } else {
    _lastErrorMs = nowMs;
    _lastError = st;

    if (_consecutiveFailures < UINT8_MAX) {
      _consecutiveFailures++;
    }
    if (_totalFailures < UINT32_MAX) {
      _totalFailures++;
    }

    if (_consecutiveFailures >= _config.offlineThreshold) {
      _driverState = DriverState::OFFLINE;
    } else {
      _driverState = DriverState::DEGRADED;
    }
  }

  return st;
}

Status LDC1614::_recordFailure(const Status& st) {
  if (st.ok() || st.inProgress()) {
    return st;
  }

  uint32_t nowMs = _nowMs();
  _lastErrorMs = nowMs;
  _lastError = st;

  if (_consecutiveFailures < UINT8_MAX) {
    _consecutiveFailures++;
  }
  if (_totalFailures < UINT32_MAX) {
    _totalFailures++;
  }

  if (_initialized) {
    if (_consecutiveFailures >= _config.offlineThreshold) {
      _driverState = DriverState::OFFLINE;
    } else {
      _driverState = DriverState::DEGRADED;
    }
  }

  return st;
}

Status LDC1614::_verifyIdentityTracked() {
  uint16_t mfgId = 0;
  Status st = readRegister16(cmd::REG_MANUFACTURER_ID, mfgId);
  if (!st.ok()) {
    return st;
  }
  if (mfgId != cmd::MANUFACTURER_ID_VALUE) {
    return _recordFailure(Status::Error(Err::DEVICE_NOT_FOUND, "Wrong MANUFACTURER_ID",
                                        static_cast<int32_t>(mfgId)));
  }

  uint16_t devId = 0;
  st = readRegister16(cmd::REG_DEVICE_ID, devId);
  if (!st.ok()) {
    return st;
  }
  if (devId != cmd::DEVICE_ID_VALUE) {
    return _recordFailure(Status::Error(Err::DEVICE_NOT_FOUND, "Wrong DEVICE_ID",
                                        static_cast<int32_t>(devId)));
  }

  return Status::Ok();
}

void LDC1614::_reassertOfflineLatch() {
  _driverState = DriverState::OFFLINE;
  const uint8_t threshold = _config.offlineThreshold == 0 ? 1 : _config.offlineThreshold;
  if (_consecutiveFailures < threshold) {
    _consecutiveFailures = threshold;
  }
}

Status LDC1614::_ensureNormalI2cAllowed() const {
  if (_initialized && _driverState == DriverState::OFFLINE && !_allowOfflineI2c) {
    return Status::Error(Err::BUSY, "Driver is offline; call recover()");
  }
  if (_initialized && _chunkedJob != ChunkedJobKind::NONE && !_chunkedPollExecuting) {
    return Status::Error(Err::BUSY, "Poll-chunked job already active");
  }
  return Status::Ok();
}

Status LDC1614::_pollReadChannels(uint8_t& remainingInstructions) {
  while (remainingInstructions > 0) {
    if (_chunkedReadChannel >= _config.channelCount) {
      return _finishChunkedJob(Status::Ok());
    }

    if (_chunkedReadPhase == ChunkedReadPhase::MSB) {
      uint16_t msb = 0;
      Status st = readRegister16(cmd::regDataMsb(_chunkedReadChannel), msb);
      remainingInstructions--;
      if (!st.ok()) {
        return _finishChunkedJob(st);
      }
      _chunkedReadMsb = msb;
      _chunkedReadPhase = ChunkedReadPhase::LSB;
      if (remainingInstructions == 0) {
        return Status::Error(Err::IN_PROGRESS, "Poll-chunked job in progress");
      }
    } else {
      uint16_t lsb = 0;
      Status st = readRegister16(cmd::regDataLsb(_chunkedReadChannel), lsb);
      remainingInstructions--;
      if (!st.ok()) {
        return _finishChunkedJob(st);
      }

      ChannelData data;
      st = _storeChannelData(_chunkedReadChannel, _chunkedReadMsb, lsb, data);
      if (!st.ok()) {
        return _finishChunkedJob(st);
      }
      _chunkedReadReadyMask |= static_cast<uint8_t>(1U << _chunkedReadChannel);

      const uint8_t next = _nextSelectedChannel(_chunkedReadChannel, _chunkedReadMask);
      if (next >= _config.channelCount) {
        return _finishChunkedJob(Status::Ok());
      }
      _chunkedReadChannel = next;
      _chunkedReadPhase = ChunkedReadPhase::MSB;
      _chunkedReadMsb = 0;
    }
  }
  return Status::Error(Err::IN_PROGRESS, "Poll-chunked job in progress");
}

Status LDC1614::_pollApplyConfig(uint8_t& remainingInstructions) {
  while (remainingInstructions > 0) {
    uint8_t reg = 0;
    uint16_t value = 0;
    uint8_t phase = DIRTY_PHASE_APPLY_GLOBAL;
    uint8_t index = DIRTY_INDEX_GLOBAL;
    if (!_configStep(_chunkedConfigStep, reg, value, phase, index)) {
      _clearHardwareConfigDirty();
      _sleeping = true;
      _consecutiveFailures = 0;
      _driverState = DriverState::READY;
      return _finishChunkedJob(Status::Ok());
    }

    Status st = _writeConfigRegister(reg, value, phase, index);
    remainingInstructions--;
    if (!st.ok()) {
      return _finishChunkedJob(st);
    }
    const bool finalStep = _chunkedConfigStep + 1U >= _configStepCount();
    if (_chunkedConfigStep == 0 || finalStep) {
      _sleeping = true;
    }
    _chunkedConfigStep++;
    if (finalStep) {
      _clearHardwareConfigDirty();
      _consecutiveFailures = 0;
      _driverState = DriverState::READY;
      return _finishChunkedJob(Status::Ok());
    }
  }
  return Status::Error(Err::IN_PROGRESS, "Poll-chunked job in progress");
}

Status LDC1614::_pollResetAndReapply(uint8_t& remainingInstructions) {
  while (remainingInstructions > 0) {
    if (_chunkedConfigStep == 0) {
      Status st = _writeConfigRegister(cmd::REG_RESET_DEV, cmd::MASK_RESET_DEV,
                                       DIRTY_PHASE_RESET, DIRTY_INDEX_GLOBAL);
      remainingInstructions--;
      if (!st.ok()) {
        return _finishChunkedJob(st);
      }
      _sleeping = true;
      _chunkedConfigStep = 1;
      if (remainingInstructions == 0) {
        return Status::Error(Err::IN_PROGRESS, "Poll-chunked job in progress");
      }
    } else if (_chunkedConfigStep == 1) {
      uint16_t mfgId = 0;
      Status st = readRegister16(cmd::REG_MANUFACTURER_ID, mfgId);
      remainingInstructions--;
      if (!st.ok()) {
        _markHardwareConfigDirty(st, DIRTY_PHASE_RESET,
                                 cmd::REG_RESET_DEV, DIRTY_INDEX_GLOBAL);
        _initialized = false;
        _driverState = DriverState::UNINIT;
        _sleeping = true;
        return _finishChunkedJob(st);
      }
      if (mfgId != cmd::MANUFACTURER_ID_VALUE) {
        _initialized = false;
        _driverState = DriverState::UNINIT;
        _sleeping = true;
        st = _recordFailure(Status::Error(Err::DEVICE_NOT_FOUND, "Wrong MANUFACTURER_ID",
                                          static_cast<int32_t>(mfgId)));
        _markHardwareConfigDirty(st, DIRTY_PHASE_RESET,
                                 cmd::REG_RESET_DEV, DIRTY_INDEX_GLOBAL);
        return _finishChunkedJob(st);
      }
      _chunkedConfigStep = 2;
      if (remainingInstructions == 0) {
        return Status::Error(Err::IN_PROGRESS, "Poll-chunked job in progress");
      }
    } else if (_chunkedConfigStep == 2) {
      uint16_t devId = 0;
      Status st = readRegister16(cmd::REG_DEVICE_ID, devId);
      remainingInstructions--;
      if (!st.ok()) {
        _markHardwareConfigDirty(st, DIRTY_PHASE_RESET,
                                 cmd::REG_RESET_DEV, DIRTY_INDEX_GLOBAL);
        _initialized = false;
        _driverState = DriverState::UNINIT;
        _sleeping = true;
        return _finishChunkedJob(st);
      }
      if (devId != cmd::DEVICE_ID_VALUE) {
        _initialized = false;
        _driverState = DriverState::UNINIT;
        _sleeping = true;
        st = _recordFailure(Status::Error(Err::DEVICE_NOT_FOUND, "Wrong DEVICE_ID",
                                          static_cast<int32_t>(devId)));
        _markHardwareConfigDirty(st, DIRTY_PHASE_RESET,
                                 cmd::REG_RESET_DEV, DIRTY_INDEX_GLOBAL);
        return _finishChunkedJob(st);
      }
      _chunkedConfigStep = 3;
      if (remainingInstructions == 0) {
        return Status::Error(Err::IN_PROGRESS, "Poll-chunked job in progress");
      }
    } else {
      const uint8_t applyStep = static_cast<uint8_t>(_chunkedConfigStep - 3U);
      uint8_t reg = 0;
      uint16_t value = 0;
      uint8_t phase = DIRTY_PHASE_APPLY_GLOBAL;
      uint8_t index = DIRTY_INDEX_GLOBAL;
      if (!_configStep(applyStep, reg, value, phase, index)) {
        _clearHardwareConfigDirty();
        _sleeping = true;
        _consecutiveFailures = 0;
        _driverState = DriverState::READY;
        return _finishChunkedJob(Status::Ok());
      }

      Status st = _writeConfigRegister(reg, value, phase, index);
      remainingInstructions--;
      if (!st.ok()) {
        _initialized = false;
        _driverState = DriverState::UNINIT;
        _sleeping = true;
        return _finishChunkedJob(st);
      }
      const bool finalStep = applyStep + 1U >= _configStepCount();
      if (applyStep == 0 || finalStep) {
        _sleeping = true;
      }
      _chunkedConfigStep++;
      if (finalStep) {
        _clearHardwareConfigDirty();
        _consecutiveFailures = 0;
        _driverState = DriverState::READY;
        return _finishChunkedJob(Status::Ok());
      }
    }
  }
  return Status::Error(Err::IN_PROGRESS, "Poll-chunked job in progress");
}

Status LDC1614::_startChunkedJob(ChunkedJobKind kind) {
  if (kind != ChunkedJobKind::READ_CHANNELS) {
    _chunkedReadMask = 0;
  }
  _chunkedReadReadyMask = 0;
  _chunkedJob = kind;
  return Status::Error(Err::IN_PROGRESS, "Poll-chunked job scheduled");
}

Status LDC1614::_finishChunkedJob(const Status& st) {
  if (!st.ok() && !st.inProgress()) {
    _chunkedReadReadyMask = 0;
  }
  _chunkedJob = ChunkedJobKind::NONE;
  _chunkedConfigStep = 0;
  _chunkedReadPhase = ChunkedReadPhase::MSB;
  _chunkedReadMsb = 0;
  return st;
}

Status LDC1614::_storeChannelData(uint8_t ch, uint16_t msb, uint16_t lsb,
                                  ChannelData& out) {
  if (!_chunkedPollExecuting && _chunkedReadReadyMask != 0U) {
    _chunkedReadMask = 0;
    _chunkedReadReadyMask = 0;
  }

  out.errUnderRange = (msb & cmd::MASK_DATA_ERR_UR) != 0;
  out.errOverRange  = (msb & cmd::MASK_DATA_ERR_OR) != 0;
  out.errWatchdog   = (msb & cmd::MASK_DATA_ERR_WD) != 0;
  out.errAmplitude  = (msb & cmd::MASK_DATA_ERR_AE) != 0;
  out.rawData = (static_cast<uint32_t>(msb & cmd::MASK_DATA_MSB_DATA) << 16) |
                static_cast<uint32_t>(lsb);

  if (out.errWatchdog) {
    return Status::Error(Err::SENSOR_ERROR, "DATAx watchdog error; sample invalid",
                         static_cast<int32_t>(msb));
  }

  _lastChannelData[ch] = out;
  _hasSample[ch] = true;
  _sampleTimestampMs[ch] = _nowMs();
  return Status::Ok();
}

bool LDC1614::_configStep(uint8_t step, uint8_t& reg, uint16_t& value,
                          uint8_t& phase, uint8_t& index) const {
  if (step >= _configStepCount()) {
    return false;
  }

  if (step == 0) {
    reg = cmd::REG_CONFIG;
    value = _buildConfigRegister(true);
    phase = DIRTY_PHASE_APPLY_GLOBAL;
    index = DIRTY_INDEX_GLOBAL;
    return true;
  }

  const uint8_t channelStepCount = static_cast<uint8_t>(_config.channelCount * 5U);
  const uint8_t channelStep = static_cast<uint8_t>(step - 1U);
  if (channelStep < channelStepCount) {
    const uint8_t ch = static_cast<uint8_t>(channelStep / 5U);
    const uint8_t field = static_cast<uint8_t>(channelStep % 5U);
    const auto& cc = _config.channel[ch];
    phase = DIRTY_PHASE_APPLY_CHANNEL;
    index = ch;
    switch (field) {
      case 0:
        reg = cmd::regRcount(ch);
        value = cc.rcount;
        return true;
      case 1:
        reg = cmd::regSettleCount(ch);
        value = cc.settleCount;
        return true;
      case 2:
        reg = cmd::regClockDividers(ch);
        value = (static_cast<uint16_t>(cc.finDivider) << cmd::BIT_FIN_DIVIDER) |
                (cc.frefDivider & cmd::MASK_FREF_DIVIDER);
        return true;
      case 3:
        reg = cmd::regOffset(ch);
        value = cc.offset;
        return true;
      default:
        reg = cmd::regDriveCurrent(ch);
        value = static_cast<uint16_t>(cc.idrive) << cmd::BIT_IDRIVE;
        return true;
    }
  }

  const uint8_t globalStep = static_cast<uint8_t>(channelStep - channelStepCount);
  phase = DIRTY_PHASE_APPLY_GLOBAL;
  index = DIRTY_INDEX_GLOBAL;
  switch (globalStep) {
    case 0:
      reg = cmd::REG_ERROR_CONFIG;
      value = _config.errorConfig;
      return true;
    case 1:
      reg = cmd::REG_MUX_CONFIG;
      value = _buildMuxConfigRegister();
      return true;
    default:
      reg = cmd::REG_CONFIG;
      value = _buildConfigRegister(true);
      return true;
  }
}

uint8_t LDC1614::_configStepCount() const {
  return static_cast<uint8_t>(1U + (_config.channelCount * 5U) + 3U);
}

uint8_t LDC1614::_firstSelectedChannel(uint8_t mask) const {
  return _nextSelectedChannel(0xFF, mask);
}

uint8_t LDC1614::_nextSelectedChannel(uint8_t after, uint8_t mask) const {
  const uint8_t start = (after == 0xFF) ? 0 : static_cast<uint8_t>(after + 1U);
  for (uint8_t ch = start; ch < _config.channelCount; ++ch) {
    if ((mask & (1U << ch)) != 0) {
      return ch;
    }
  }
  return _config.channelCount;
}

// ============================================================================
// Internal
// ============================================================================

Status LDC1614::_performRecoveryLadder() {
  const uint32_t now = _nowMs();

  auto syncIfDirty = [this]() -> Status {
    if (!_hardwareConfigDirty) {
      return Status::Ok();
    }
    return _applyConfig();
  };

  // Enforce recovery backoff only when the application supplied a timebase.
  const bool enforceBackoff = _config.recoverBackoffMs > 0 && _config.nowMs != nullptr;
  if (enforceBackoff && _lastRecoverValid &&
      (now - _lastRecoverMs) < _config.recoverBackoffMs) {
    return Status::Error(Err::BUSY, "Recovery backoff active");
  }
  _lastRecoverMs = now;
  _lastRecoverValid = enforceBackoff;

  // Step 1: Identity probe via tracked reads (updates health on I2C access)
  Status last = _verifyIdentityTracked();
  if (last.ok()) {
    Status st = syncIfDirty();
    if (st.ok()) {
      _consecutiveFailures = 0;
      _driverState = DriverState::READY;
      return Status::Ok();
    }
    last = st;
  }

  // Step 2: Bus reset (SCL recovery) if callback provided
  if (_config.recoverUseBusReset && _config.busReset != nullptr) {
    Status st = _config.busReset(_config.i2cUser);
    if (st.ok()) {
      st = _verifyIdentityTracked();
      if (st.ok()) {
        st = syncIfDirty();
        if (st.ok()) {
          _consecutiveFailures = 0;
          _driverState = DriverState::READY;
          return Status::Ok();
        }
        last = st;
      } else {
        last = st;
      }
    } else {
      last = st;
    }
  }

  // Step 3: Soft reset + re-apply config
  if (_config.recoverUseSoftReset) {
    // Attempt soft reset regardless of current sleep state
    Status st = _writeConfigRegister(cmd::REG_RESET_DEV, cmd::MASK_RESET_DEV,
                                     DIRTY_PHASE_RESET, DIRTY_INDEX_GLOBAL);
    if (st.ok()) {
      _sleeping = true;
      st = _verifyIdentityTracked();
      if (!st.ok()) {
        _markHardwareConfigDirty(st, DIRTY_PHASE_RESET,
                                 cmd::REG_RESET_DEV, DIRTY_INDEX_GLOBAL);
      }
      if (st.ok()) {
        st = _applyConfig();
      }
      if (st.ok()) {
        _consecutiveFailures = 0;
        _driverState = DriverState::READY;
        return Status::Ok();
      }
    }
    last = st;
  }

  // Step 4: Hard reset (e.g., SHDN pin toggle) if callback provided
  if (_config.recoverUseHardReset && _config.hardReset != nullptr) {
    Status st = _config.hardReset(_config.i2cUser);
    if (st.ok()) {
      _sleeping = true;
      st = _verifyIdentityTracked();
      if (!st.ok()) {
        _markHardwareConfigDirty(st, DIRTY_PHASE_RESET,
                                 cmd::REG_RESET_DEV, DIRTY_INDEX_GLOBAL);
      }
      if (st.ok()) {
        st = _applyConfig();
      }
      if (st.ok()) {
        _consecutiveFailures = 0;
        _driverState = DriverState::READY;
        return Status::Ok();
      }
    }
    last = st;
  }

  return last;
}

void LDC1614::_markHardwareConfigDirty(const Status& cause, uint8_t phase,
                                       uint8_t reg, uint8_t index) {
  if (_hardwareConfigDirty) {
    return;
  }
  _hardwareConfigDirty = true;
  _hardwareConfigDirtyError = dirtyStatusFrom(cause, phase, reg, index);
}

void LDC1614::_clearHardwareConfigDirty() {
  _hardwareConfigDirty = false;
  _hardwareConfigDirtyError = Status::Ok();
}

Status LDC1614::_applyConfig() {
  // Datasheet configuration changes are made while the device is in sleep mode.
  // Probe does not prove the previous application left the device asleep, so
  // full apply/reapply paths first force a cached sleep-mode CONFIG image.
  Status st = _writeConfigRegister(cmd::REG_CONFIG, _buildConfigRegister(true),
                                   DIRTY_PHASE_APPLY_GLOBAL,
                                   DIRTY_INDEX_GLOBAL);
  if (!st.ok()) return st;
  _sleeping = true;

  // Write per-channel registers while asleep.
  for (uint8_t ch = 0; ch < _config.channelCount; ch++) {
    const auto& cc = _config.channel[ch];

    // RCOUNT
    st = _writeConfigRegister(cmd::regRcount(ch), cc.rcount,
                              DIRTY_PHASE_APPLY_CHANNEL, ch);
    if (!st.ok()) return st;

    // SETTLECOUNT
    st = _writeConfigRegister(cmd::regSettleCount(ch), cc.settleCount,
                              DIRTY_PHASE_APPLY_CHANNEL, ch);
    if (!st.ok()) return st;

    // CLOCK_DIVIDERS: FIN_DIVIDER[15:12], reserved[11:10]=0, FREF_DIVIDER[9:0]
    uint16_t clkDiv = (static_cast<uint16_t>(cc.finDivider) << cmd::BIT_FIN_DIVIDER) |
                      (cc.frefDivider & cmd::MASK_FREF_DIVIDER);
    st = _writeConfigRegister(cmd::regClockDividers(ch), clkDiv,
                              DIRTY_PHASE_APPLY_CHANNEL, ch);
    if (!st.ok()) return st;

    // OFFSET
    st = _writeConfigRegister(cmd::regOffset(ch), cc.offset,
                              DIRTY_PHASE_APPLY_CHANNEL, ch);
    if (!st.ok()) return st;

    // DRIVE_CURRENT: IDRIVE[15:11], INIT_IDRIVE[10:6]=0, reserved[5:0]=0
    uint16_t drv = static_cast<uint16_t>(cc.idrive) << cmd::BIT_IDRIVE;
    st = _writeConfigRegister(cmd::regDriveCurrent(ch), drv,
                              DIRTY_PHASE_APPLY_CHANNEL, ch);
    if (!st.ok()) return st;
  }

  // ERROR_CONFIG
  st = _writeConfigRegister(cmd::REG_ERROR_CONFIG, _config.errorConfig,
                            DIRTY_PHASE_APPLY_GLOBAL,
                            DIRTY_INDEX_GLOBAL);
  if (!st.ok()) return st;

  // MUX_CONFIG
  st = _writeConfigRegister(cmd::REG_MUX_CONFIG, _buildMuxConfigRegister(),
                            DIRTY_PHASE_APPLY_GLOBAL, DIRTY_INDEX_GLOBAL);
  if (!st.ok()) return st;

  // CONFIG (must be written last — starts conversions if SLEEP_MODE_EN=0)
  // After _applyConfig(), device remains in sleep mode. Caller uses wake() to start.
  st = _writeConfigRegister(cmd::REG_CONFIG, _buildConfigRegister(true),
                            DIRTY_PHASE_APPLY_GLOBAL, DIRTY_INDEX_GLOBAL);
  if (!st.ok()) return st;

  _sleeping = true;
  _clearHardwareConfigDirty();
  return Status::Ok();
}

uint16_t LDC1614::_buildConfigRegister(bool sleepMode) const {
  uint16_t config = cmd::CONFIG_RESERVED_VALUE;  // Reserved [5:0] = 0x01

  config |= (static_cast<uint16_t>(_config.activeChan) << cmd::BIT_CFG_ACTIVE_CHAN) &
            cmd::MASK_CFG_ACTIVE_CHAN;

  if (sleepMode) {
    config |= cmd::MASK_CFG_SLEEP_MODE_EN;
  }

  if (_config.rpOverrideEn) {
    config |= cmd::MASK_CFG_RP_OVERRIDE_EN;
  }

  if (_config.sensorActivation == SensorActivation::LOW_POWER) {
    config |= cmd::MASK_CFG_SENSOR_ACTIVATE_SEL;
  }

  if (_config.autoAmpDis) {
    config |= cmd::MASK_CFG_AUTO_AMP_DIS;
  }

  if (_config.refClkSrc == RefClkSrc::EXT_CLK) {
    config |= cmd::MASK_CFG_REF_CLK_SRC;
  }

  if (_config.intbDisable) {
    config |= cmd::MASK_CFG_INTB_DIS;
  }

  if (_config.highCurrentDrv) {
    config |= cmd::MASK_CFG_HIGH_CURRENT_DRV;
  }

  return config;
}

uint16_t LDC1614::_buildMuxConfigRegister() const {
  uint16_t mux = cmd::MUX_CONFIG_RESERVED_VALUE;  // Reserved [12:3] must be 0x0208

  if (_config.autoScan) {
    mux |= cmd::MASK_MUX_AUTOSCAN_EN;
  }

  mux |= (static_cast<uint16_t>(_config.rrSequence) << cmd::BIT_MUX_RR_SEQUENCE) &
         cmd::MASK_MUX_RR_SEQUENCE;

  mux |= static_cast<uint16_t>(_config.deglitch) & cmd::MASK_MUX_DEGLITCH;

  return mux;
}

uint32_t LDC1614::_nowMs() const {
  if (_config.nowMs != nullptr) {
    return _config.nowMs(_config.timeUser);
  }
  return 0;
}

void LDC1614::_cooperativeYield() const {
  if (_config.cooperativeYield != nullptr) {
    _config.cooperativeYield(_config.timeUser);
    return;
  }
}

} // namespace LDC1614
