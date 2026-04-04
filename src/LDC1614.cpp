/// @file LDC1614.cpp
/// @brief Implementation of LDC1614/LDC1612 driver

#include "LDC1614/LDC1614.h"

#include <Arduino.h>
#include <climits>

namespace LDC1614 {

namespace {

bool isValidChannel(uint8_t ch, uint8_t channelCount) {
  return ch < channelCount;
}

bool isValidRcount(uint16_t rcount) {
  return rcount >= cmd::RCOUNT_MIN;
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

bool isValidRefClkSrc(RefClkSrc src) {
  return static_cast<uint8_t>(src) <= static_cast<uint8_t>(RefClkSrc::EXT_CLK);
}

bool isValidSensorActivation(SensorActivation sa) {
  return static_cast<uint8_t>(sa) <= static_cast<uint8_t>(SensorActivation::LOW_POWER);
}

} // namespace

// ============================================================================
// Lifecycle
// ============================================================================

Status LDC1614::begin(const Config& config) {
  _config = config;
  _initialized = false;
  _sleeping = true;
  _driverState = DriverState::UNINIT;

  _lastOkMs = 0;
  _lastErrorMs = 0;
  _lastError = Status::Ok();
  _consecutiveFailures = 0;
  _totalFailures = 0;
  _totalSuccess = 0;
  _lastRecoverMs = 0;
  _lastRecoverValid = false;

  for (uint8_t i = 0; i < cmd::MAX_CHANNELS; i++) {
    _lastChannelData[i] = ChannelData{};
    _sampleTimestampMs[i] = 0;
  }

  if (_config.i2cWrite == nullptr || _config.i2cWriteRead == nullptr) {
    return Status::Error(Err::INVALID_CONFIG, "I2C callbacks required");
  }
  if (_config.i2cTimeoutMs == 0) {
    return Status::Error(Err::INVALID_CONFIG, "Timeout must be > 0");
  }
  if (_config.i2cAddress != 0x2A && _config.i2cAddress != 0x2B) {
    return Status::Error(Err::INVALID_CONFIG, "Invalid I2C address (must be 0x2A or 0x2B)");
  }
  if (_config.channelCount != 2 && _config.channelCount != 4) {
    return Status::Error(Err::INVALID_CONFIG, "channelCount must be 2 or 4");
  }
  if (_config.activeChan >= _config.channelCount) {
    return Status::Error(Err::INVALID_CONFIG, "activeChan exceeds channelCount");
  }
  if (!isValidDeglitch(_config.deglitch)) {
    return Status::Error(Err::INVALID_CONFIG, "Invalid deglitch value");
  }
  if (!isValidRRSequence(_config.rrSequence)) {
    return Status::Error(Err::INVALID_CONFIG, "Invalid RR sequence");
  }
  if (!isValidRefClkSrc(_config.refClkSrc)) {
    return Status::Error(Err::INVALID_CONFIG, "Invalid reference clock source");
  }
  if (!isValidSensorActivation(_config.sensorActivation)) {
    return Status::Error(Err::INVALID_CONFIG, "Invalid sensor activation mode");
  }
  if (_config.intbPin < -1) {
    return Status::Error(Err::INVALID_CONFIG, "Invalid INTB pin");
  }
  if (_config.intbPin >= 0 && _config.gpioRead == nullptr) {
    return Status::Error(Err::INVALID_CONFIG, "INTB gpioRead required");
  }
  if (_config.highCurrentDrv && _config.autoScan) {
    return Status::Error(Err::INVALID_CONFIG, "HIGH_CURRENT_DRV only in single-channel mode");
  }
  if (_config.highCurrentDrv && _config.activeChan != 0) {
    return Status::Error(Err::INVALID_CONFIG, "HIGH_CURRENT_DRV only on Ch0");
  }

  // Validate per-channel config
  for (uint8_t ch = 0; ch < _config.channelCount; ch++) {
    const auto& cc = _config.channel[ch];
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
  // Application polls dataReady() or uses INTB pin for notification.
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
}

// ============================================================================
// Diagnostics
// ============================================================================

Status LDC1614::probe() {
  uint16_t mfgId = 0;
  Status st = _readRegister16Raw(cmd::REG_MANUFACTURER_ID, mfgId);
  if (!st.ok()) {
    if (st.code == Err::INVALID_CONFIG || st.code == Err::INVALID_PARAM) {
      return st;
    }
    return Status::Error(Err::DEVICE_NOT_FOUND, "LDC1614 not responding", st.detail);
  }
  if (mfgId != cmd::MANUFACTURER_ID_VALUE) {
    return Status::Error(Err::DEVICE_NOT_FOUND, "Wrong MANUFACTURER_ID",
                         static_cast<int32_t>(mfgId));
  }

  uint16_t devId = 0;
  st = _readRegister16Raw(cmd::REG_DEVICE_ID, devId);
  if (!st.ok()) {
    if (st.code == Err::INVALID_CONFIG || st.code == Err::INVALID_PARAM) {
      return st;
    }
    return Status::Error(Err::DEVICE_NOT_FOUND, "LDC1614 not responding", st.detail);
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
  return _performRecoveryLadder();
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

  // Parse error flags from MSB
  out.errUnderRange = (msb & cmd::MASK_DATA_ERR_UR) != 0;
  out.errOverRange  = (msb & cmd::MASK_DATA_ERR_OR) != 0;
  out.errWatchdog   = (msb & cmd::MASK_DATA_ERR_WD) != 0;
  out.errAmplitude  = (msb & cmd::MASK_DATA_ERR_AE) != 0;

  // Reconstruct 28-bit data: DATAx_MSB[11:0] << 16 | DATAx_LSB[15:0]
  out.rawData = (static_cast<uint32_t>(msb & cmd::MASK_DATA_MSB_DATA) << 16) |
                static_cast<uint32_t>(lsb);

  // Cache the result with timestamp
  _lastChannelData[ch] = out;
  _sampleTimestampMs[ch] = _nowMs();

  return Status::Ok();
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
    n = _config.channelCount;
  }

  for (uint8_t ch = 0; ch < n; ch++) {
    Status st = readChannel(ch, out[ch]);
    if (!st.ok()) {
      return st;
    }
  }

  return Status::Ok();
}

bool LDC1614::dataReady() {
  if (!_initialized) {
    return false;
  }

  // Check INTB pin if configured (active low)
  if (_config.intbPin >= 0 && _config.gpioRead != nullptr && !_config.intbDisable) {
    bool level = _config.gpioRead(_config.intbPin, _config.gpioUser);
    return !level;  // INTB is active low
  }

  // Fall back to polling STATUS register DRDY bit
  uint16_t status = 0;
  Status st = readRegister16(cmd::REG_STATUS, status);
  if (!st.ok()) {
    return false;
  }
  return (status & cmd::MASK_STATUS_DRDY) != 0;
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
  Status st = writeRegister16(cmd::REG_CONFIG, configReg);
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
  Status st = writeRegister16(cmd::REG_CONFIG, configReg);
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

  Status st = writeRegister16(cmd::REG_RESET_DEV, cmd::MASK_RESET_DEV);
  if (!st.ok()) {
    return st;
  }

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

  Status st = writeRegister16(cmd::REG_RESET_DEV, cmd::MASK_RESET_DEV);
  if (!st.ok()) {
    return st;
  }

  // Restore state for re-initialization
  _config = savedConfig;
  _sleeping = true;

  // Re-apply full configuration
  st = _applyConfig();
  if (!st.ok()) {
    _initialized = false;
    _driverState = DriverState::UNINIT;
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
  if (_sleeping) {
    return Status::Error(Err::BUSY, "Device is sleeping; wake first");
  }

  const uint32_t deadline = _nowMs() + timeoutMs;
  while (!dataReady()) {
    if (_nowMs() >= deadline) {
      return Status::Error(Err::TIMEOUT, "Data ready timeout");
    }
    _cooperativeYield();
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
  if (_sleeping) {
    return Status::Error(Err::BUSY, "Device is sleeping; wake first");
  }

  const uint32_t deadline = _nowMs() + timeoutMs;
  while (!dataReady()) {
    if (_nowMs() >= deadline) {
      return Status::Error(Err::TIMEOUT, "Data ready timeout");
    }
    _cooperativeYield();
  }

  return readAllChannels(out, count);
}

// ============================================================================
// Sample Cache
// ============================================================================

Status LDC1614::getLastSample(uint8_t ch, ChannelData& out) const {
  if (ch >= cmd::MAX_CHANNELS) {
    return Status::Error(Err::INVALID_PARAM, "Invalid channel");
  }
  if (_sampleTimestampMs[ch] == 0) {
    return Status::Error(Err::CONVERSION_NOT_READY, "No cached sample for channel");
  }
  out = _lastChannelData[ch];
  return Status::Ok();
}

uint32_t LDC1614::sampleTimestampMs(uint8_t ch) const {
  if (ch >= cmd::MAX_CHANNELS) {
    return 0;
  }
  return _sampleTimestampMs[ch];
}

uint32_t LDC1614::sampleAgeMs(uint8_t ch, uint32_t nowMs) const {
  if (ch >= cmd::MAX_CHANNELS || _sampleTimestampMs[ch] == 0) {
    return 0;
  }
  return nowMs - _sampleTimestampMs[ch];
}

// ============================================================================
// Settings Snapshot
// ============================================================================

void LDC1614::getSettings(SettingsSnapshot& out) const {
  out.state = _driverState;
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
  for (uint8_t i = 0; i < cmd::MAX_CHANNELS; i++) {
    out.sampleTimestampMs[i] = _sampleTimestampMs[i];
    out.channel[i] = _config.channel[i];
  }
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

  _config.activeChan = ch;
  return writeRegister16(cmd::REG_CONFIG, _buildConfigRegister(true));
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

  _config.channel[ch].rcount = rcount;
  return writeRegister16(cmd::regRcount(ch), rcount);
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

  _config.channel[ch].settleCount = count;
  return writeRegister16(cmd::regSettleCount(ch), count);
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

  _config.channel[ch].finDivider = finDiv;
  _config.channel[ch].frefDivider = frefDiv;

  uint16_t regVal = (static_cast<uint16_t>(finDiv) << cmd::BIT_FIN_DIVIDER) |
                    (frefDiv & cmd::MASK_FREF_DIVIDER);
  return writeRegister16(cmd::regClockDividers(ch), regVal);
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

  _config.channel[ch].offset = offset;
  return writeRegister16(cmd::regOffset(ch), offset);
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

  _config.channel[ch].idrive = idrive;
  // INIT_IDRIVE (bits 10:6) must be written as 0; reserved bits (5:0) must be 0
  uint16_t regVal = static_cast<uint16_t>(idrive) << cmd::BIT_IDRIVE;
  return writeRegister16(cmd::regDriveCurrent(ch), regVal);
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
  if (ch >= cmd::MAX_CHANNELS) {
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
  if (ch >= cmd::MAX_CHANNELS) {
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
  Status st = _i2cWriteReadRaw(txBuf, txLen, rxBuf, rxLen);
  if (st.code == Err::INVALID_CONFIG || st.code == Err::INVALID_PARAM) {
    return st;
  }
  return _updateHealth(st);
}

Status LDC1614::_i2cWriteTracked(const uint8_t* buf, size_t len) {
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
  uint8_t rx[2] = {0, 0};
  Status st = _i2cWriteReadTracked(&reg, 1, rx, sizeof(rx));
  if (!st.ok()) {
    return st;
  }
  value = (static_cast<uint16_t>(rx[0]) << 8) | rx[1];
  return Status::Ok();
}

Status LDC1614::writeRegister16(uint8_t reg, uint16_t value) {
  uint8_t tx[3] = {
    reg,
    static_cast<uint8_t>((value >> 8) & 0xFF),
    static_cast<uint8_t>(value & 0xFF)
  };
  return _i2cWriteTracked(tx, sizeof(tx));
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
  uint32_t nowMs = _nowMs();

  if (st.ok() || st.inProgress()) {
    _lastOkMs = nowMs;
    _consecutiveFailures = 0;
    if (_totalSuccess < UINT32_MAX) {
      _totalSuccess++;
    }

    if (_initialized) {
      _driverState = DriverState::READY;
    }
  } else {
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
  }

  return st;
}

// ============================================================================
// Internal
// ============================================================================

Status LDC1614::_performRecoveryLadder() {
  const uint32_t now = _nowMs();

  // Enforce recovery backoff
  if (_config.recoverBackoffMs > 0 && _lastRecoverValid &&
      (now - _lastRecoverMs) < _config.recoverBackoffMs) {
    return Status::Error(Err::BUSY, "Recovery backoff active");
  }
  _lastRecoverMs = now;
  _lastRecoverValid = true;

  // Step 1: Simple probe via tracked read (updates health on success)
  uint16_t mfgId = 0;
  Status last = readRegister16(cmd::REG_MANUFACTURER_ID, mfgId);
  if (last.ok()) {
    return Status::Ok();
  }

  // Step 2: Bus reset (SCL recovery) if callback provided
  if (_config.recoverUseBusReset && _config.busReset != nullptr) {
    Status st = _config.busReset(_config.i2cUser);
    if (st.ok()) {
      st = readRegister16(cmd::REG_MANUFACTURER_ID, mfgId);
      if (st.ok()) {
        return Status::Ok();
      }
      last = st;
    } else {
      last = st;
    }
  }

  // Step 3: Soft reset + re-apply config
  if (_config.recoverUseSoftReset) {
    // Attempt soft reset regardless of current sleep state
    Status st = writeRegister16(cmd::REG_RESET_DEV, cmd::MASK_RESET_DEV);
    if (st.ok()) {
      _sleeping = true;
      st = _applyConfig();
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
      st = _applyConfig();
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

Status LDC1614::_applyConfig() {
  // Write per-channel registers (in sleep mode after POR / probe)
  for (uint8_t ch = 0; ch < _config.channelCount; ch++) {
    const auto& cc = _config.channel[ch];

    // RCOUNT
    Status st = writeRegister16(cmd::regRcount(ch), cc.rcount);
    if (!st.ok()) return st;

    // SETTLECOUNT
    st = writeRegister16(cmd::regSettleCount(ch), cc.settleCount);
    if (!st.ok()) return st;

    // CLOCK_DIVIDERS: FIN_DIVIDER[15:12], reserved[11:10]=0, FREF_DIVIDER[9:0]
    uint16_t clkDiv = (static_cast<uint16_t>(cc.finDivider) << cmd::BIT_FIN_DIVIDER) |
                      (cc.frefDivider & cmd::MASK_FREF_DIVIDER);
    st = writeRegister16(cmd::regClockDividers(ch), clkDiv);
    if (!st.ok()) return st;

    // OFFSET
    st = writeRegister16(cmd::regOffset(ch), cc.offset);
    if (!st.ok()) return st;

    // DRIVE_CURRENT: IDRIVE[15:11], INIT_IDRIVE[10:6]=0, reserved[5:0]=0
    uint16_t drv = static_cast<uint16_t>(cc.idrive) << cmd::BIT_IDRIVE;
    st = writeRegister16(cmd::regDriveCurrent(ch), drv);
    if (!st.ok()) return st;
  }

  // ERROR_CONFIG
  Status st = writeRegister16(cmd::REG_ERROR_CONFIG, _config.errorConfig);
  if (!st.ok()) return st;

  // MUX_CONFIG
  st = writeRegister16(cmd::REG_MUX_CONFIG, _buildMuxConfigRegister());
  if (!st.ok()) return st;

  // CONFIG (must be written last — starts conversions if SLEEP_MODE_EN=0)
  // After _applyConfig(), device remains in sleep mode. Caller uses wake() to start.
  st = writeRegister16(cmd::REG_CONFIG, _buildConfigRegister(true));
  if (!st.ok()) return st;

  _sleeping = true;
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
  return millis();
}

void LDC1614::_cooperativeYield() const {
  if (_config.cooperativeYield != nullptr) {
    _config.cooperativeYield(_config.timeUser);
    return;
  }
  yield();
}

} // namespace LDC1614
