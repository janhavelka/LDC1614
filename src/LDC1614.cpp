/// @file LDC1614.cpp
/// @brief Cooperative LDC1612/LDC1614 driver implementation.

#include "LDC1614/LDC1614.h"

#include <climits>
#include <cmath>
#include <cstdint>

namespace LDC1614 {

namespace {

constexpr uint32_t RAW_COUNT_MAX = 0x0FFFFFFFU;
constexpr uint32_t SENSOR_FREQUENCY_MIN_HZ = 1000U;
constexpr uint32_t SENSOR_FREQUENCY_MAX_HZ = 10000000U;
constexpr uint32_t INTERNAL_CLOCK_MIN_HZ = 35000000U;
constexpr uint32_t INTERNAL_CLOCK_MAX_HZ = 55000000U;
constexpr uint32_t EXTERNAL_CLOCK_MIN_HZ = 2000000U;
constexpr uint32_t EXTERNAL_CLOCK_MAX_HZ = 40000000U;
constexpr uint32_t MAX_TOLERANCE_PPM = 999999U;
constexpr uint64_t PPM_SCALE = 1000000ULL;
constexpr uint64_t MICROS_PER_SECOND = 1000000ULL;
constexpr uint64_t NANOS_PER_SECOND = 1000000000ULL;
constexpr uint64_t WAKE_DELAY_CYCLES = 16384ULL;
constexpr uint64_t CHANNEL_SWITCH_BASE_NS = 692ULL;
constexpr uint64_t CHANNEL_SWITCH_CYCLES = 5ULL;

uint8_t channelIndex(Channel channel) {
  return static_cast<uint8_t>(channel);
}

bool isConcreteChannel(Channel channel) {
  return channelIndex(channel) < cmd::MAX_CHANNELS;
}

uint8_t channelCountFor(DeviceVariant variant) {
  if (variant == DeviceVariant::LDC1612) {
    return 2;
  }
  if (variant == DeviceVariant::LDC1614) {
    return 4;
  }
  return 0;
}

uint8_t popcount(ChannelMask mask) {
  uint8_t count = 0;
  uint8_t bits = mask.bits;
  while (bits != 0U) {
    count = static_cast<uint8_t>(count + (bits & 1U));
    bits = static_cast<uint8_t>(bits >> 1U);
  }
  return count;
}

Channel firstChannel(ChannelMask mask) {
  for (uint8_t index = 0; index < cmd::MAX_CHANNELS; ++index) {
    if ((mask.bits & static_cast<uint8_t>(1U << index)) != 0U) {
      return static_cast<Channel>(index);
    }
  }
  return Channel::NONE;
}

Channel nextChannel(ChannelMask mask, Channel after) {
  const uint8_t start =
      after == Channel::NONE ? 0U : static_cast<uint8_t>(channelIndex(after) + 1U);
  for (uint8_t index = start; index < cmd::MAX_CHANNELS; ++index) {
    if ((mask.bits & static_cast<uint8_t>(1U << index)) != 0U) {
      return static_cast<Channel>(index);
    }
  }
  return Channel::NONE;
}

ChannelMask sequenceMask(RRSequence sequence) {
  switch (sequence) {
    case RRSequence::CH0_CH1:
      return ChannelMask{0x03};
    case RRSequence::CH0_CH1_CH2:
      return ChannelMask{0x07};
    case RRSequence::CH0_CH1_CH2_CH3:
      return ChannelMask{0x0F};
    default:
      return ChannelMask{};
  }
}

bool isValidDeglitch(Deglitch deglitch) {
  return deglitch == Deglitch::BW_1MHZ ||
         deglitch == Deglitch::BW_3MHZ ||
         deglitch == Deglitch::BW_10MHZ ||
         deglitch == Deglitch::BW_33MHZ;
}

uint32_t deglitchBandwidthHz(Deglitch deglitch) {
  switch (deglitch) {
    case Deglitch::BW_1MHZ:
      return 1000000U;
    case Deglitch::BW_3MHZ:
      return 3300000U;
    case Deglitch::BW_10MHZ:
      return 10000000U;
    case Deglitch::BW_33MHZ:
      return 33000000U;
    default:
      return 0U;
  }
}

bool isValidRegisterAddress(uint8_t reg) {
  return reg <= cmd::REG_RESET_DEV ||
         (reg >= cmd::REG_DRIVE_CURRENT0 && reg <= cmd::REG_DRIVE_CURRENT3) ||
         reg == cmd::REG_MANUFACTURER_ID ||
         reg == cmd::REG_DEVICE_ID;
}

bool isConfigurationRegister(uint8_t reg) {
  return (reg >= cmd::REG_RCOUNT0 && reg <= cmd::REG_CLOCK_DIVIDERS3) ||
         reg == cmd::REG_ERROR_CONFIG || reg == cmd::REG_CONFIG ||
         reg == cmd::REG_MUX_CONFIG || reg == cmd::REG_RESET_DEV ||
         (reg >= cmd::REG_DRIVE_CURRENT0 && reg <= cmd::REG_DRIVE_CURRENT3);
}

JobPhase configurationPhaseForRegister(uint8_t reg) {
  if (reg >= cmd::REG_RCOUNT0 && reg <= cmd::REG_RCOUNT3) {
    return JobPhase::WRITE_RCOUNT;
  }
  if (reg >= cmd::REG_SETTLECOUNT0 && reg <= cmd::REG_SETTLECOUNT3) {
    return JobPhase::WRITE_SETTLECOUNT;
  }
  if (reg >= cmd::REG_CLOCK_DIVIDERS0 &&
      reg <= cmd::REG_CLOCK_DIVIDERS3) {
    return JobPhase::WRITE_CLOCK_DIVIDERS;
  }
  if (reg >= cmd::REG_OFFSET0 && reg <= cmd::REG_OFFSET3) {
    return JobPhase::WRITE_OFFSET;
  }
  if (reg >= cmd::REG_DRIVE_CURRENT0 && reg <= cmd::REG_DRIVE_CURRENT3) {
    return JobPhase::WRITE_DRIVE_CURRENT;
  }
  if (reg == cmd::REG_ERROR_CONFIG) return JobPhase::WRITE_ERROR_CONFIG;
  if (reg == cmd::REG_MUX_CONFIG) return JobPhase::WRITE_MUX_CONFIG;
  if (reg == cmd::REG_CONFIG) return JobPhase::WRITE_FINAL_CONFIG;
  if (reg == cmd::REG_RESET_DEV) return JobPhase::SOFTWARE_RESET;
  return JobPhase::NONE;
}

Channel configurationChannelForRegister(uint8_t reg) {
  if (reg >= cmd::REG_RCOUNT0 && reg <= cmd::REG_RCOUNT3) {
    return static_cast<Channel>(reg - cmd::REG_RCOUNT0);
  }
  if (reg >= cmd::REG_SETTLECOUNT0 && reg <= cmd::REG_SETTLECOUNT3) {
    return static_cast<Channel>(reg - cmd::REG_SETTLECOUNT0);
  }
  if (reg >= cmd::REG_CLOCK_DIVIDERS0 &&
      reg <= cmd::REG_CLOCK_DIVIDERS3) {
    return static_cast<Channel>(reg - cmd::REG_CLOCK_DIVIDERS0);
  }
  if (reg >= cmd::REG_OFFSET0 && reg <= cmd::REG_OFFSET3) {
    return static_cast<Channel>(reg - cmd::REG_OFFSET0);
  }
  if (reg >= cmd::REG_DRIVE_CURRENT0 && reg <= cmd::REG_DRIVE_CURRENT3) {
    return static_cast<Channel>(reg - cmd::REG_DRIVE_CURRENT0);
  }
  return Channel::NONE;
}

bool readHasDocumentedSideEffects(uint8_t reg) {
  return reg <= cmd::REG_DATA3_LSB || reg == cmd::REG_STATUS;
}

uint16_t buildConfigRegister(const Config& config, bool sleeping) {
  uint16_t value = cmd::CONFIG_RESERVED_VALUE;
  if (isConcreteChannel(config.activeChannel)) {
    value |= static_cast<uint16_t>(
        (static_cast<uint16_t>(channelIndex(config.activeChannel))
         << cmd::BIT_CFG_ACTIVE_CHAN) &
        cmd::MASK_CFG_ACTIVE_CHAN);
  }
  if (sleeping) {
    value |= cmd::MASK_CFG_SLEEP_MODE_EN;
  }
  if (config.rpOverrideEnabled) {
    value |= cmd::MASK_CFG_RP_OVERRIDE_EN;
  }
  if (config.sensorActivation == SensorActivation::LOW_POWER) {
    value |= cmd::MASK_CFG_SENSOR_ACTIVATE_SEL;
  }
  if (!config.autoAmplitudeCorrectionEnabled) {
    value |= cmd::MASK_CFG_AUTO_AMP_DIS;
  }
  if (config.referenceClock.source == RefClkSrc::EXTERNAL_CLOCK) {
    value |= cmd::MASK_CFG_REF_CLK_SRC;
  }
  if (config.intbDisabled) {
    value |= cmd::MASK_CFG_INTB_DIS;
  }
  if (config.highCurrentDriveEnabled) {
    value |= cmd::MASK_CFG_HIGH_CURRENT_DRV;
  }
  return value;
}

uint16_t buildMuxConfigRegister(const Config& config) {
  uint16_t value = cmd::MUX_CONFIG_RESERVED_VALUE;
  if (config.mode == OperatingMode::MULTI_CHANNEL_SEQUENTIAL) {
    value |= cmd::MASK_MUX_AUTOSCAN_EN;
    value |= static_cast<uint16_t>(
        (static_cast<uint16_t>(config.rrSequence) << cmd::BIT_MUX_RR_SEQUENCE) &
        cmd::MASK_MUX_RR_SEQUENCE);
  }
  value |= static_cast<uint16_t>(config.deglitch) & cmd::MASK_MUX_DEGLITCH;
  return value;
}

uint8_t configurationTransferCount(uint8_t channelCount) {
  return static_cast<uint8_t>(1U + channelCount * 5U + 3U);
}

struct ConfigWrite {
  bool valid = false;
  uint8_t reg = 0;
  uint16_t value = 0;
  JobPhase phase = JobPhase::NONE;
  Channel channel = Channel::NONE;
};

ConfigWrite configWriteAt(const Config& config, uint8_t channelCount,
                          uint8_t step) {
  ConfigWrite write{};
  const uint8_t total = configurationTransferCount(channelCount);
  if (step >= total) {
    return write;
  }
  write.valid = true;
  if (step == 0U) {
    write.reg = cmd::REG_CONFIG;
    write.value = buildConfigRegister(config, true);
    write.phase = JobPhase::FORCE_SLEEP;
    return write;
  }

  const uint8_t perChannelStep = static_cast<uint8_t>(step - 1U);
  const uint8_t perChannelCount = static_cast<uint8_t>(channelCount * 5U);
  if (perChannelStep < perChannelCount) {
    const uint8_t index = static_cast<uint8_t>(perChannelStep / 5U);
    const uint8_t field = static_cast<uint8_t>(perChannelStep % 5U);
    const ChannelConfig& channel = config.channel[index];
    write.channel = static_cast<Channel>(index);
    switch (field) {
      case 0:
        write.reg = cmd::regRcount(index);
        write.value = channel.rcount;
        write.phase = JobPhase::WRITE_RCOUNT;
        break;
      case 1:
        write.reg = cmd::regSettleCount(index);
        write.value = channel.settleCount;
        write.phase = JobPhase::WRITE_SETTLECOUNT;
        break;
      case 2:
        write.reg = cmd::regClockDividers(index);
        write.value = static_cast<uint16_t>(
            (static_cast<uint16_t>(channel.finDivider)
             << cmd::BIT_FIN_DIVIDER) |
            (channel.frefDivider & cmd::MASK_FREF_DIVIDER));
        write.phase = JobPhase::WRITE_CLOCK_DIVIDERS;
        break;
      case 3:
        write.reg = cmd::regOffset(index);
        write.value = channel.offset;
        write.phase = JobPhase::WRITE_OFFSET;
        break;
      default:
        write.reg = cmd::regDriveCurrent(index);
        write.value =
            static_cast<uint16_t>(channel.driveCurrentCode) << cmd::BIT_IDRIVE;
        write.phase = JobPhase::WRITE_DRIVE_CURRENT;
        break;
    }
    return write;
  }

  const uint8_t globalStep =
      static_cast<uint8_t>(perChannelStep - perChannelCount);
  if (globalStep == 0U) {
    write.reg = cmd::REG_ERROR_CONFIG;
    write.value = LDC1614::encodeErrorReporting(config.errorReporting);
    write.phase = JobPhase::WRITE_ERROR_CONFIG;
  } else if (globalStep == 1U) {
    write.reg = cmd::REG_MUX_CONFIG;
    write.value = buildMuxConfigRegister(config);
    write.phase = JobPhase::WRITE_MUX_CONFIG;
  } else {
    write.reg = cmd::REG_CONFIG;
    write.value = buildConfigRegister(config, true);
    write.phase = JobPhase::WRITE_FINAL_CONFIG;
  }
  return write;
}

uint64_t ceilDivide(uint64_t numerator, uint64_t denominator) {
  return denominator == 0U ? 0U : (numerator + denominator - 1U) / denominator;
}

TerminalOutcome outcomeForFailure(const Status& status) {
  (void)status;
  // Only expiry of the immutable whole-operation deadline is classified as
  // TIMED_OUT. A callback-level timeout is a terminal transport failure and,
  // for writes, may also carry INDETERMINATE_WRITE provenance.
  return TerminalOutcome::FAILED;
}

}  // namespace

Status LDC1614::bind(const Config& config) {
  if (_bound) {
    return Status::Error(Err::BUSY, "Driver already bound; call end first");
  }
  Status validation = _validateConfig(config);
  if (!validation.ok()) {
    return validation;
  }

  _config = config;
  _bound = true;
  _appliedState = AppliedConfigState::UNKNOWN;
  _configRevision = 1;
  _configFault = ConfigFault{};
  _transportStats = TransportStats{};
  _clearActiveJob();
  _clearSampleCache();
  _resultHead = 0;
  _resultCount = 0;
  for (uint8_t index = 0; index < RESULT_CAPACITY; ++index) {
    _results[index] = OperationResult{};
  }
  return Status::Ok();
}

void LDC1614::end() {
  _bound = false;
  _config = Config{};
  _appliedState = AppliedConfigState::UNKNOWN;
  _configRevision = 0;
  _configFault = ConfigFault{};
  _transportStats = TransportStats{};
  _clearActiveJob();
  _clearSampleCache();
  _resultHead = 0;
  _resultCount = 0;
  for (uint8_t index = 0; index < RESULT_CAPACITY; ++index) {
    _results[index] = OperationResult{};
  }
}

Status LDC1614::updateDesiredConfig(const Config& config) {
  if (!_bound) {
    return Status::Error(Err::NOT_BOUND, "Driver not bound");
  }
  if (_progress.active) {
    return Status::Error(Err::BUSY, "Operation active");
  }
  if (_appliedState == AppliedConfigState::APPLIED_ACTIVE) {
    return Status::Error(Err::BUSY,
                         "Sleep device before replacing desired config");
  }
  Status validation = _validateConfig(config);
  if (!validation.ok()) {
    return validation;
  }
  if (config.i2cWrite != _config.i2cWrite ||
      config.i2cWriteRead != _config.i2cWriteRead ||
      config.i2cUser != _config.i2cUser ||
      config.i2cAddress != _config.i2cAddress ||
      config.variant != _config.variant) {
    return Status::Error(
        Err::INVALID_CONFIG,
        "Transport, address, and variant changes require end/rebind");
  }
  if (_configRevision == UINT32_MAX) {
    return Status::Error(Err::INVALID_CONFIG, "Config revision exhausted");
  }

  _config = config;
  ++_configRevision;
  if (_appliedState != AppliedConfigState::UNKNOWN) {
    _appliedState = AppliedConfigState::DIRTY;
  }
  _clearSampleCache();
  return Status::Ok();
}

Status LDC1614::startInitialize(OperationId operationId,
                                uint64_t deadlineMs) {
  return _startJob(JobKind::INITIALIZE, ChannelMask{}, operationId, deadlineMs,
                   static_cast<uint8_t>(2U +
                                        configurationTransferCount(
                                            _configuredChannelCount())));
}

Status LDC1614::startApplyConfig(OperationId operationId,
                                 uint64_t deadlineMs) {
  if (_bound && _appliedState == AppliedConfigState::UNKNOWN) {
    return Status::Error(Err::CONFIG_DIRTY,
                         "Identity/configuration not established");
  }
  return _startJob(JobKind::APPLY_CONFIG, ChannelMask{}, operationId,
                   deadlineMs,
                   configurationTransferCount(_configuredChannelCount()));
}

Status LDC1614::startResetAndReapply(OperationId operationId,
                                     uint64_t deadlineMs) {
  return _startJob(JobKind::RESET_AND_REAPPLY, ChannelMask{}, operationId,
                   deadlineMs,
                   static_cast<uint8_t>(3U +
                                        configurationTransferCount(
                                            _configuredChannelCount())));
}

Status LDC1614::startAcquire(ChannelMask channels, OperationId operationId,
                             uint64_t deadlineMs) {
  if (!_bound) {
    return Status::Error(Err::NOT_BOUND, "Driver not bound");
  }
  if (_bound && _appliedState != AppliedConfigState::APPLIED_ACTIVE) {
    return Status::Error(Err::CONFIG_DIRTY,
                         "Acquisition requires verified active configuration");
  }
  if (channels.empty() ||
      (channels.bits & static_cast<uint8_t>(~_config.channels.bits)) != 0U) {
    return Status::Error(Err::INVALID_PARAM,
                         "Acquisition mask must be configured and nonzero");
  }
  const uint8_t maximum =
      static_cast<uint8_t>(2U + 2U * popcount(channels));
  return _startJob(JobKind::ACQUIRE, channels, operationId, deadlineMs,
                   maximum);
}

Status LDC1614::_startJob(JobKind kind, ChannelMask channels,
                          OperationId operationId, uint64_t deadlineMs,
                          uint8_t maximumTransfers) {
  if (!_bound) {
    return Status::Error(Err::NOT_BOUND, "Driver not bound");
  }
  if (operationId == 0U) {
    return Status::Error(Err::INVALID_PARAM, "Operation id must be nonzero");
  }
  if (deadlineMs == 0U) {
    return Status::Error(Err::INVALID_PARAM, "Deadline must be nonzero");
  }
  if (_operationIdInUse(operationId)) {
    return Status::Error(Err::DUPLICATE_OPERATION_ID,
                         "Operation id already active or pending");
  }
  if (_progress.active) {
    return Status::Error(Err::BUSY, "Operation active");
  }
  if (!_reserveResultSlot()) {
    return Status::Error(Err::RESULT_QUEUE_FULL,
                         "Terminal result capacity exhausted");
  }

  _clearActiveJob();
  _progress.active = true;
  _progress.operationId = operationId;
  _progress.kind = kind;
  _progress.deadlineMs = deadlineMs;
  _progress.maximumTransfers = maximumTransfers;
  _progress.requestedChannels = channels;
  _progress.configRevision = _configRevision;
  _jobChannel = Channel::NONE;
  switch (kind) {
    case JobKind::INITIALIZE:
      _progress.phase = JobPhase::VERIFY_MANUFACTURER;
      _progress.registerAddress = cmd::REG_MANUFACTURER_ID;
      break;
    case JobKind::APPLY_CONFIG:
      _progress.phase = JobPhase::FORCE_SLEEP;
      _progress.registerAddress = cmd::REG_CONFIG;
      break;
    case JobKind::RESET_AND_REAPPLY:
      _progress.phase = JobPhase::SOFTWARE_RESET;
      _progress.registerAddress = cmd::REG_RESET_DEV;
      break;
    case JobKind::ACQUIRE:
      _progress.phase = JobPhase::READ_STATUS_BEFORE;
      _progress.registerAddress = cmd::REG_STATUS;
      break;
    default:
      _clearActiveJob();
      return Status::Error(Err::INVALID_PARAM, "Invalid operation kind");
  }
  return Status::Error(Err::IN_PROGRESS, "Operation scheduled");
}

Status LDC1614::poll(uint64_t nowMs, uint8_t maxTransfers) {
  if (!_progress.active) {
    return Status::Ok();
  }
  if (nowMs >= _progress.deadlineMs) {
    const Status timeout =
        Status::Error(Err::TIMEOUT, "Operation deadline expired");
    if (_progress.kind != JobKind::ACQUIRE &&
        hasEffect(_progress.effects, EffectFlag::PARTIAL_WRITE)) {
      _appliedState = AppliedConfigState::DIRTY;
      _recordConfigFault(timeout, _lastWritePhase, _lastWriteRegister,
                         _lastWriteChannel, _progress.effects);
    }
    return _finishJob(TerminalOutcome::TIMED_OUT, timeout, nowMs);
  }
  if (maxTransfers == 0U) {
    return Status::Error(Err::IN_PROGRESS, "Operation in progress");
  }

  const uint64_t deadlineBudgetMs = _progress.deadlineMs - nowMs;
  uint8_t remaining = maxTransfers;
  if (deadlineBudgetMs < remaining) {
    remaining = static_cast<uint8_t>(deadlineBudgetMs);
  }
  const uint64_t fairShareMs = deadlineBudgetMs / remaining;
  const uint32_t transferTimeoutMs = static_cast<uint32_t>(
      fairShareMs < _config.i2cTimeoutMs ? fairShareMs
                                        : _config.i2cTimeoutMs);
  while (_progress.active && remaining > 0U) {
    Status status =
        _progress.kind == JobKind::ACQUIRE
            ? _pollAcquire(nowMs, remaining, transferTimeoutMs)
            : _pollInitializeOrApply(
                  nowMs, remaining, transferTimeoutMs,
                  _progress.kind == JobKind::RESET_AND_REAPPLY);
    if (!status.ok() && !status.inProgress()) {
      return status;
    }
  }
  return _progress.active
             ? Status::Error(Err::IN_PROGRESS, "Operation in progress")
             : Status::Ok();
}

Status LDC1614::cancelJob() {
  if (!_progress.active) {
    return Status::Ok();
  }
  const Status cancelled =
      Status::Error(Err::CANCELLED, "Operation cancelled");
  if (_progress.kind != JobKind::ACQUIRE &&
      hasEffect(_progress.effects, EffectFlag::PARTIAL_WRITE)) {
    _appliedState = AppliedConfigState::DIRTY;
    _recordConfigFault(cancelled, _lastWritePhase, _lastWriteRegister,
                       _lastWriteChannel, _progress.effects);
  }
  (void)_finishJob(TerminalOutcome::CANCELLED, cancelled, 0);
  return Status::Ok();
}

Status LDC1614::takeResult(OperationResult& out) {
  out = OperationResult{};
  if (_resultCount == 0U) {
    return Status::Error(Err::RESULT_NOT_READY, "No terminal result available");
  }
  out = _results[_resultHead];
  _results[_resultHead] = OperationResult{};
  _resultHead = static_cast<uint8_t>((_resultHead + 1U) % RESULT_CAPACITY);
  --_resultCount;
  return Status::Ok();
}

void LDC1614::invalidateAppliedState(const Status& reason) {
  if (_progress.active) {
    (void)cancelJob();
  }
  _appliedState = AppliedConfigState::UNKNOWN;
  _clearSampleCache();
  _configFault = ConfigFault{};
  _configFault.valid = true;
  _configFault.cause =
      reason.ok() ? Status::Error(Err::CONFIG_DIRTY,
                                 "Applied configuration invalidated")
                  : reason;
}

Status LDC1614::_pollInitializeOrApply(uint64_t nowMs,
                                       uint8_t& remainingTransfers,
                                       uint32_t transferTimeoutMs,
                                       bool resetFirst) {
  const JobKind kind = _progress.kind;
  uint8_t applyOffset = 0;
  if (kind == JobKind::INITIALIZE) {
    applyOffset = 2;
  } else if (resetFirst) {
    applyOffset = 3;
  }

  while (_progress.active && remainingTransfers > 0U) {
    if (resetFirst && _jobStep == 0U) {
      _progress.phase = JobPhase::SOFTWARE_RESET;
      _progress.registerAddress = cmd::REG_RESET_DEV;
      _progress.channel = Channel::NONE;
      Status status = _writeRegister(cmd::REG_RESET_DEV, cmd::MASK_RESET_DEV,
                                     transferTimeoutMs);
      --remainingTransfers;
      ++_progress.completedTransfers;
      if (!status.ok()) {
        _progress.effects |= effectFlag(EffectFlag::INDETERMINATE_WRITE);
        _appliedState = AppliedConfigState::DIRTY;
        _recordConfigFault(status, JobPhase::SOFTWARE_RESET,
                           cmd::REG_RESET_DEV, Channel::NONE,
                           _progress.effects);
        return _finishJob(outcomeForFailure(status), status, nowMs);
      }
      _progress.effects |= effectFlag(EffectFlag::PARTIAL_WRITE);
      _lastWritePhase = JobPhase::SOFTWARE_RESET;
      _lastWriteRegister = cmd::REG_RESET_DEV;
      _lastWriteChannel = Channel::NONE;
      _appliedState = AppliedConfigState::APPLYING;
      ++_jobStep;
      _progress.phase = JobPhase::VERIFY_MANUFACTURER;
      _progress.registerAddress = cmd::REG_MANUFACTURER_ID;
      continue;
    }

    const bool manufacturerStep =
        (kind == JobKind::INITIALIZE && _jobStep == 0U) ||
        (resetFirst && _jobStep == 1U);
    if (manufacturerStep) {
      _progress.phase = JobPhase::VERIFY_MANUFACTURER;
      _progress.registerAddress = cmd::REG_MANUFACTURER_ID;
      _progress.channel = Channel::NONE;
      uint16_t value = 0;
      Status status = _readRegister(cmd::REG_MANUFACTURER_ID, value,
                                    transferTimeoutMs);
      --remainingTransfers;
      ++_progress.completedTransfers;
      if (!status.ok()) {
        if (resetFirst) {
          _appliedState = AppliedConfigState::DIRTY;
          _recordConfigFault(status, JobPhase::VERIFY_MANUFACTURER,
                             cmd::REG_MANUFACTURER_ID, Channel::NONE,
                             _progress.effects);
        }
        return _finishJob(outcomeForFailure(status), status, nowMs);
      }
      if (value != cmd::MANUFACTURER_ID_VALUE) {
        status = Status::Error(Err::DEVICE_NOT_FOUND,
                               "Wrong MANUFACTURER_ID", value);
        _appliedState = AppliedConfigState::UNKNOWN;
        _clearSampleCache();
        _recordConfigFault(status, JobPhase::VERIFY_MANUFACTURER,
                           cmd::REG_MANUFACTURER_ID, Channel::NONE,
                           _progress.effects);
        return _finishJob(TerminalOutcome::FAILED, status, nowMs);
      }
      ++_jobStep;
      _progress.phase = JobPhase::VERIFY_DEVICE;
      _progress.registerAddress = cmd::REG_DEVICE_ID;
      continue;
    }

    const bool deviceStep =
        (kind == JobKind::INITIALIZE && _jobStep == 1U) ||
        (resetFirst && _jobStep == 2U);
    if (deviceStep) {
      _progress.phase = JobPhase::VERIFY_DEVICE;
      _progress.registerAddress = cmd::REG_DEVICE_ID;
      _progress.channel = Channel::NONE;
      uint16_t value = 0;
      Status status = _readRegister(cmd::REG_DEVICE_ID, value,
                                    transferTimeoutMs);
      --remainingTransfers;
      ++_progress.completedTransfers;
      if (!status.ok()) {
        if (resetFirst) {
          _appliedState = AppliedConfigState::DIRTY;
          _recordConfigFault(status, JobPhase::VERIFY_DEVICE,
                             cmd::REG_DEVICE_ID, Channel::NONE,
                             _progress.effects);
        }
        return _finishJob(outcomeForFailure(status), status, nowMs);
      }
      if (value != cmd::DEVICE_ID_VALUE) {
        status =
            Status::Error(Err::DEVICE_NOT_FOUND, "Wrong DEVICE_ID", value);
        _appliedState = AppliedConfigState::UNKNOWN;
        _clearSampleCache();
        _recordConfigFault(status, JobPhase::VERIFY_DEVICE,
                           cmd::REG_DEVICE_ID, Channel::NONE,
                           _progress.effects);
        return _finishJob(TerminalOutcome::FAILED, status, nowMs);
      }
      ++_jobStep;
      _progress.phase = JobPhase::FORCE_SLEEP;
      _progress.registerAddress = cmd::REG_CONFIG;
      continue;
    }

    const uint8_t applyStep = static_cast<uint8_t>(_jobStep - applyOffset);
    const ConfigWrite write =
        configWriteAt(_config, _configuredChannelCount(), applyStep);
    if (!write.valid) {
      _appliedState = AppliedConfigState::APPLIED_SLEEPING;
      _configFault = ConfigFault{};
      _clearSampleCache();
      _progress.effects = 0;
      return _finishJob(TerminalOutcome::SUCCESS, Status::Ok(), nowMs);
    }

    _progress.phase = write.phase;
    _progress.registerAddress = write.reg;
    _progress.channel = write.channel;
    _jobChannel = write.channel;
    Status status = _writeRegister(write.reg, write.value, transferTimeoutMs);
    --remainingTransfers;
    ++_progress.completedTransfers;
    if (!status.ok()) {
      _progress.effects |= effectFlag(EffectFlag::INDETERMINATE_WRITE);
      _appliedState = AppliedConfigState::DIRTY;
      _recordConfigFault(status, write.phase, write.reg, write.channel,
                         _progress.effects);
      return _finishJob(outcomeForFailure(status), status, nowMs);
    }
    _progress.effects |= effectFlag(EffectFlag::PARTIAL_WRITE);
    _lastWritePhase = write.phase;
    _lastWriteRegister = write.reg;
    _lastWriteChannel = write.channel;
    _appliedState = AppliedConfigState::APPLYING;
    ++_jobStep;
    if (static_cast<uint8_t>(applyStep + 1U) >=
        configurationTransferCount(_configuredChannelCount())) {
      _appliedState = AppliedConfigState::APPLIED_SLEEPING;
      _configFault = ConfigFault{};
      _clearSampleCache();
      _progress.effects = 0;
      return _finishJob(TerminalOutcome::SUCCESS, Status::Ok(), nowMs);
    }
    const ConfigWrite nextWrite = configWriteAt(
        _config, _configuredChannelCount(),
        static_cast<uint8_t>(applyStep + 1U));
    _progress.phase = nextWrite.phase;
    _progress.registerAddress = nextWrite.reg;
    _progress.channel = nextWrite.channel;
  }
  return Status::Error(Err::IN_PROGRESS, "Operation in progress");
}

Status LDC1614::_pollAcquire(uint64_t nowMs,
                             uint8_t& remainingTransfers,
                             uint32_t transferTimeoutMs) {
  while (_progress.active && remainingTransfers > 0U) {
    if (_progress.phase == JobPhase::READ_STATUS_BEFORE) {
      _progress.registerAddress = cmd::REG_STATUS;
      _progress.channel = Channel::NONE;
      _progress.effects |= effectFlag(EffectFlag::READ_SIDE_EFFECTS);
      uint16_t raw = 0;
      Status status = _readRegister(cmd::REG_STATUS, raw, transferTimeoutMs);
      --remainingTransfers;
      ++_progress.completedTransfers;
      if (!status.ok()) {
        return _finishJob(outcomeForFailure(status), status, nowMs);
      }
      _scratchStatusBefore = decodeDeviceStatus(raw);
      _jobChannel = firstChannel(_progress.requestedChannels);
      _progress.phase = _jobChannel == Channel::NONE
                            ? JobPhase::READ_STATUS_AFTER
                            : JobPhase::READ_DATA_MSB;
      _progress.registerAddress =
          _jobChannel == Channel::NONE
              ? cmd::REG_STATUS
              : cmd::regDataMsb(channelIndex(_jobChannel));
      _progress.channel = _jobChannel;
      continue;
    }

    if (_progress.phase == JobPhase::READ_DATA_MSB) {
      _progress.registerAddress = cmd::regDataMsb(channelIndex(_jobChannel));
      _progress.channel = _jobChannel;
      _progress.effects |= effectFlag(EffectFlag::READ_SIDE_EFFECTS);
      uint16_t value = 0;
      Status status =
          _readRegister(cmd::regDataMsb(channelIndex(_jobChannel)), value,
                        transferTimeoutMs);
      --remainingTransfers;
      ++_progress.completedTransfers;
      if (!status.ok()) {
        return _finishJob(outcomeForFailure(status), status, nowMs);
      }
      _scratchMsb = value;
      _progress.phase = JobPhase::READ_DATA_LSB;
      _progress.registerAddress = cmd::regDataLsb(channelIndex(_jobChannel));
      continue;
    }

    if (_progress.phase == JobPhase::READ_DATA_LSB) {
      _progress.registerAddress = cmd::regDataLsb(channelIndex(_jobChannel));
      _progress.channel = _jobChannel;
      uint16_t value = 0;
      Status status =
          _readRegister(cmd::regDataLsb(channelIndex(_jobChannel)), value,
                        transferTimeoutMs);
      --remainingTransfers;
      ++_progress.completedTransfers;
      if (!status.ok()) {
        return _finishJob(outcomeForFailure(status), status, nowMs);
      }
      const uint8_t index = channelIndex(_jobChannel);
      _scratchSamples[index] = decodeChannelSample(_scratchMsb, value);
      _scratchCompleted.bits |= static_cast<uint8_t>(1U << index);
      _progress.completedChannels = _scratchCompleted;
      _jobChannel = nextChannel(_progress.requestedChannels, _jobChannel);
      _progress.phase = _jobChannel == Channel::NONE
                            ? JobPhase::READ_STATUS_AFTER
                            : JobPhase::READ_DATA_MSB;
      _progress.registerAddress =
          _jobChannel == Channel::NONE
              ? cmd::REG_STATUS
              : cmd::regDataMsb(channelIndex(_jobChannel));
      _progress.channel = _jobChannel;
      continue;
    }

    if (_progress.phase == JobPhase::READ_STATUS_AFTER) {
      _progress.registerAddress = cmd::REG_STATUS;
      _progress.channel = Channel::NONE;
      _progress.effects |= effectFlag(EffectFlag::READ_SIDE_EFFECTS);
      uint16_t raw = 0;
      Status status = _readRegister(cmd::REG_STATUS, raw, transferTimeoutMs);
      --remainingTransfers;
      ++_progress.completedTransfers;
      if (!status.ok()) {
        return _finishJob(outcomeForFailure(status), status, nowMs);
      }
      _scratchStatusAfter = decodeDeviceStatus(raw);
      _commitAcquisition(nowMs);
      return _finishJob(TerminalOutcome::SUCCESS, Status::Ok(), nowMs);
    }

    const Status invalid =
        Status::Error(Err::INVALID_PARAM, "Invalid acquisition phase");
    return _finishJob(TerminalOutcome::FAILED, invalid, nowMs);
  }
  return Status::Error(Err::IN_PROGRESS, "Operation in progress");
}

Status LDC1614::_finishJob(TerminalOutcome outcome, const Status& status,
                           uint64_t completedUptimeMs) {
  const JobKind kind = _progress.kind;
  OperationResult result{};
  result.operationId = _progress.operationId;
  result.kind = kind;
  result.outcome = outcome;
  result.status = status;
  result.effects = _progress.effects;
  result.configFault = _configFault;
  result.configRevision = _progress.configRevision;
  result.completedUptimeMs = completedUptimeMs;
  result.finalProgress = _progress;
  result.finalProgress.active = false;
  if (kind == JobKind::ACQUIRE && outcome == TerminalOutcome::SUCCESS &&
      _hasLastBatch) {
    result.hasSampleBatch = true;
    result.sampleBatch = _lastBatch;
    result.sampleBatch.completedUptimeMs = completedUptimeMs;
  }

  const uint8_t tail =
      static_cast<uint8_t>((_resultHead + _resultCount) % RESULT_CAPACITY);
  _results[tail] = result;
  if (_resultCount < RESULT_CAPACITY) {
    ++_resultCount;
  }
  _clearActiveJob();
  return outcome == TerminalOutcome::SUCCESS ? Status::Ok() : status;
}

void LDC1614::_commitAcquisition(uint64_t completedUptimeMs) {
  SampleBatch batch{};
  batch.selectedChannels = _progress.requestedChannels;
  batch.statusBefore = _scratchStatusBefore;
  batch.statusAfter = _scratchStatusAfter;
  batch.completedUptimeMs = completedUptimeMs;
  batch.configRevision = _configRevision;
  batch.freshChannels.bits = static_cast<uint8_t>(
      _scratchStatusBefore.unreadChannels.bits & batch.selectedChannels.bits);
  batch.overrunChannels.bits = static_cast<uint8_t>(
      _scratchStatusAfter.unreadChannels.bits & batch.selectedChannels.bits);

  for (uint8_t index = 0; index < _configuredChannelCount(); ++index) {
    const uint8_t bit = static_cast<uint8_t>(1U << index);
    if ((batch.selectedChannels.bits & bit) == 0U) {
      continue;
    }
    ChannelSample sample = _scratchSamples[index];
    if ((batch.freshChannels.bits & bit) != 0U) {
      sample.quality |= sampleQualityFlag(SampleQualityFlag::FRESH);
    } else {
      sample.quality |= sampleQualityFlag(SampleQualityFlag::STALE);
    }

    const bool statusApplies =
        _scratchStatusBefore.hasError() &&
        _scratchStatusBefore.errorChannel == static_cast<Channel>(index);
    if (statusApplies) {
      if (_scratchStatusBefore.errorUnderRange) {
        sample.quality |= sampleQualityFlag(SampleQualityFlag::UNDER_RANGE);
      }
      if (_scratchStatusBefore.errorOverRange) {
        sample.quality |= sampleQualityFlag(SampleQualityFlag::OVER_RANGE);
      }
      if (_scratchStatusBefore.errorWatchdog) {
        sample.quality |= sampleQualityFlag(SampleQualityFlag::WATCHDOG);
      }
      if (_scratchStatusBefore.errorAmplitudeHigh ||
          _scratchStatusBefore.errorAmplitudeLow) {
        sample.quality |=
            sampleQualityFlag(SampleQualityFlag::AMPLITUDE_SUSPECT);
      }
      if (_scratchStatusBefore.errorZeroCount) {
        sample.quality |= sampleQualityFlag(SampleQualityFlag::ZERO_COUNT);
      }
    }
    if ((batch.overrunChannels.bits & bit) != 0U) {
      sample.quality |= sampleQualityFlag(SampleQualityFlag::DATA_LOST);
    }

    const SampleQualityFlags invalidFlags =
        sampleQualityFlag(SampleQualityFlag::STALE) |
        sampleQualityFlag(SampleQualityFlag::UNDER_RANGE) |
        sampleQualityFlag(SampleQualityFlag::OVER_RANGE) |
        sampleQualityFlag(SampleQualityFlag::WATCHDOG) |
        sampleQualityFlag(SampleQualityFlag::ZERO_COUNT) |
        sampleQualityFlag(SampleQualityFlag::DATA_LOST) |
        sampleQualityFlag(SampleQualityFlag::CONFIG_UNKNOWN);
    if ((sample.quality & invalidFlags) == 0U) {
      batch.validChannels.bits |= bit;
    }
    const SampleQualityFlags errorFlags =
        static_cast<SampleQualityFlags>(
            (invalidFlags &
             static_cast<SampleQualityFlags>(
                 ~sampleQualityFlag(SampleQualityFlag::STALE))) |
            sampleQualityFlag(SampleQualityFlag::AMPLITUDE_SUSPECT));
    if ((sample.quality & errorFlags) != 0U) {
      batch.errorChannels.bits |= bit;
    }
    batch.channel[index] = sample;
  }
  _lastBatch = batch;
  _hasLastBatch = true;
}

void LDC1614::_clearActiveJob() {
  _progress = JobProgress{};
  _jobStep = 0;
  _jobChannel = Channel::NONE;
  _lastWritePhase = JobPhase::NONE;
  _lastWriteRegister = INVALID_REGISTER;
  _lastWriteChannel = Channel::NONE;
  _scratchMsb = 0;
  _scratchStatusBefore = DeviceStatus{};
  _scratchStatusAfter = DeviceStatus{};
  _scratchCompleted = ChannelMask{};
  for (uint8_t index = 0; index < cmd::MAX_CHANNELS; ++index) {
    _scratchSamples[index] = ChannelSample{};
  }
}

void LDC1614::_clearSampleCache() {
  _lastBatch = SampleBatch{};
  _hasLastBatch = false;
}

bool LDC1614::_operationIdInUse(OperationId operationId) const {
  if (_progress.active && _progress.operationId == operationId) {
    return true;
  }
  for (uint8_t offset = 0; offset < _resultCount; ++offset) {
    const uint8_t index =
        static_cast<uint8_t>((_resultHead + offset) % RESULT_CAPACITY);
    if (_results[index].operationId == operationId) {
      return true;
    }
  }
  return false;
}

bool LDC1614::_reserveResultSlot() const {
  return _resultCount < RESULT_CAPACITY;
}

uint8_t LDC1614::_configuredChannelCount() const {
  return channelCountFor(_config.variant);
}

Status LDC1614::_readRegister(uint8_t reg, uint16_t& value,
                              uint32_t timeoutMs) {
  value = 0;
  uint8_t rx[2] = {0, 0};
  Status status = _config.i2cWriteRead(
      static_cast<uint8_t>(_config.i2cAddress), &reg, 1, rx, sizeof(rx),
      timeoutMs, _config.i2cUser);
  status = _tracked(status);
  if (status.ok()) {
    value =
        static_cast<uint16_t>((static_cast<uint16_t>(rx[0]) << 8U) | rx[1]);
  }
  return status;
}

Status LDC1614::_writeRegister(uint8_t reg, uint16_t value,
                               uint32_t timeoutMs) {
  const uint8_t tx[3] = {
      reg, static_cast<uint8_t>((value >> 8U) & 0xFFU),
      static_cast<uint8_t>(value & 0xFFU)};
  return _tracked(_config.i2cWrite(static_cast<uint8_t>(_config.i2cAddress),
                                   tx, sizeof(tx), timeoutMs,
                                   _config.i2cUser));
}

Status LDC1614::_tracked(const Status& status) {
  if (_transportStats.totalAttempts < UINT32_MAX) {
    ++_transportStats.totalAttempts;
  }
  if (status.ok()) {
    if (_transportStats.totalSuccess < UINT32_MAX) {
      ++_transportStats.totalSuccess;
    }
  } else if (_transportStats.totalFailures < UINT32_MAX) {
    ++_transportStats.totalFailures;
  }
  _transportStats.lastStatus = status;
  return status;
}

void LDC1614::_recordConfigFault(const Status& cause, JobPhase phase,
                                 uint8_t reg, Channel channel,
                                 EffectFlags effects) {
  _configFault.valid = true;
  _configFault.cause = cause;
  _configFault.job = _progress.kind;
  _configFault.phase = phase;
  _configFault.registerAddress = reg;
  _configFault.channel = channel;
  _configFault.effects = effects;
}

Status LDC1614::readDeviceStatus(DeviceStatus& out) {
  out = DeviceStatus{};
  if (!_bound) {
    return Status::Error(Err::NOT_BOUND, "Driver not bound");
  }
  if (_progress.active) {
    return Status::Error(Err::BUSY, "Operation active");
  }
  uint16_t raw = 0;
  Status status = _readRegister(cmd::REG_STATUS, raw, _config.i2cTimeoutMs);
  if (status.ok()) {
    out = decodeDeviceStatus(raw);
  }
  return status;
}

Status LDC1614::readDataReady(bool& ready, DeviceStatus& observedStatus) {
  ready = false;
  observedStatus = DeviceStatus{};
  if (!_bound) {
    return Status::Error(Err::NOT_BOUND, "Driver not bound");
  }
  if (!_config.errorReporting.dataReady) {
    return Status::Error(Err::INVALID_CONFIG,
                         "Data-ready reporting is disabled");
  }
  if (_progress.active) {
    return Status::Error(Err::BUSY, "Operation active");
  }
  if (_config.intbAsserted != nullptr && !_config.intbDisabled) {
    bool asserted = false;
    Status gpio = _config.intbAsserted(asserted, _config.intbUser);
    if (!gpio.ok()) {
      return gpio;
    }
    if (!asserted) {
      return Status::Ok();
    }
  }
  Status result = readDeviceStatus(observedStatus);
  if (result.ok()) {
    ready = observedStatus.dataReady;
  }
  return result;
}

Status LDC1614::sleep() {
  if (!_bound) {
    return Status::Error(Err::NOT_BOUND, "Driver not bound");
  }
  if (_progress.active) {
    return Status::Error(Err::BUSY, "Operation active");
  }
  if (_appliedState == AppliedConfigState::APPLIED_SLEEPING) {
    return Status::Ok();
  }
  if (_appliedState != AppliedConfigState::APPLIED_ACTIVE) {
    return Status::Error(Err::CONFIG_DIRTY,
                         "Applied configuration is not trusted");
  }
  Status status =
      _writeRegister(cmd::REG_CONFIG, buildConfigRegister(_config, true),
                     _config.i2cTimeoutMs);
  if (!status.ok()) {
    _appliedState = AppliedConfigState::DIRTY;
    _recordConfigFault(
        status, JobPhase::WRITE_FINAL_CONFIG, cmd::REG_CONFIG, Channel::NONE,
        effectFlag(EffectFlag::INDETERMINATE_WRITE));
    return status;
  }
  _appliedState = AppliedConfigState::APPLIED_SLEEPING;
  return Status::Ok();
}

Status LDC1614::wake() {
  if (!_bound) {
    return Status::Error(Err::NOT_BOUND, "Driver not bound");
  }
  if (_progress.active) {
    return Status::Error(Err::BUSY, "Operation active");
  }
  if (_appliedState == AppliedConfigState::APPLIED_ACTIVE) {
    return Status::Ok();
  }
  if (_appliedState != AppliedConfigState::APPLIED_SLEEPING) {
    return Status::Error(Err::CONFIG_DIRTY,
                         "Applied configuration is not trusted");
  }
  Status status =
      _writeRegister(cmd::REG_CONFIG, buildConfigRegister(_config, false),
                     _config.i2cTimeoutMs);
  if (!status.ok()) {
    _appliedState = AppliedConfigState::DIRTY;
    _recordConfigFault(
        status, JobPhase::WRITE_FINAL_CONFIG, cmd::REG_CONFIG, Channel::NONE,
        effectFlag(EffectFlag::INDETERMINATE_WRITE));
    return status;
  }
  _appliedState = AppliedConfigState::APPLIED_ACTIVE;
  return Status::Ok();
}

Status LDC1614::readInitDriveCurrent(Channel channel, uint8_t& code) {
  code = 0;
  if (!_bound) {
    return Status::Error(Err::NOT_BOUND, "Driver not bound");
  }
  if (!isConcreteChannel(channel) ||
      !validChannelMask(_config.variant).contains(channel)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid channel");
  }
  uint16_t value = 0;
  Status status =
      readRegister16(cmd::regDriveCurrent(channelIndex(channel)), value);
  if (status.ok()) {
    code = static_cast<uint8_t>(
        (value & cmd::MASK_INIT_IDRIVE) >> cmd::BIT_INIT_IDRIVE);
  }
  return status;
}

Status LDC1614::readRegister16(uint8_t reg, uint16_t& value) {
  value = 0;
  if (!_bound) {
    return Status::Error(Err::NOT_BOUND, "Driver not bound");
  }
  if (_progress.active) {
    return Status::Error(Err::BUSY, "Operation active");
  }
  if (!isValidRegisterAddress(reg)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid register address");
  }
  Status status = _readRegister(reg, value, _config.i2cTimeoutMs);
  if (status.ok() && readHasDocumentedSideEffects(reg)) {
    _clearSampleCache();
  }
  return status;
}

Status LDC1614::writeRegister16(uint8_t reg, uint16_t value) {
  if (!_bound) {
    return Status::Error(Err::NOT_BOUND, "Driver not bound");
  }
  if (_progress.active) {
    return Status::Error(Err::BUSY, "Operation active");
  }
  if (!isValidRegisterAddress(reg)) {
    return Status::Error(Err::INVALID_PARAM, "Invalid register address");
  }
  Status status = _writeRegister(reg, value, _config.i2cTimeoutMs);
  if (isConfigurationRegister(reg)) {
    _clearSampleCache();
    _appliedState =
        reg == cmd::REG_RESET_DEV ? AppliedConfigState::UNKNOWN
                                  : AppliedConfigState::DIRTY;
    const EffectFlags effects =
        status.ok() ? effectFlag(EffectFlag::PARTIAL_WRITE)
                    : effectFlag(EffectFlag::INDETERMINATE_WRITE);
    _configFault.valid = true;
    _configFault.job = JobKind::NONE;
    _configFault.cause =
        status.ok()
            ? Status::Error(Err::CONFIG_DIRTY,
                            "Diagnostic write invalidated applied state")
            : status;
    _configFault.phase = configurationPhaseForRegister(reg);
    _configFault.registerAddress = reg;
    _configFault.channel = configurationChannelForRegister(reg);
    _configFault.effects = effects;
  }
  return status;
}

Status LDC1614::readIntb(bool& asserted) const {
  asserted = false;
  if (!_bound) {
    return Status::Error(Err::NOT_BOUND, "Driver not bound");
  }
  if (_config.intbAsserted == nullptr || _config.intbDisabled) {
    return Status::Error(Err::INVALID_CONFIG, "INTB callback not enabled");
  }
  return _config.intbAsserted(asserted, _config.intbUser);
}

Status LDC1614::_validateConfig(const Config& config) const {
  if (config.i2cWrite == nullptr || config.i2cWriteRead == nullptr) {
    return Status::Error(Err::INVALID_CONFIG, "I2C callbacks required");
  }
  if (config.i2cTimeoutMs == 0U) {
    return Status::Error(Err::INVALID_CONFIG, "I2C timeout must be nonzero");
  }
  if (config.i2cAddress != I2cAddress::ADDR_GND &&
      config.i2cAddress != I2cAddress::ADDR_VDD) {
    return Status::Error(Err::INVALID_CONFIG, "Invalid I2C address");
  }
  const uint8_t channelCount = channelCountFor(config.variant);
  if (channelCount == 0U) {
    return Status::Error(Err::INVALID_CONFIG, "Device variant required");
  }
  const ChannelMask valid = validChannelMask(config.variant);
  if (config.channels.empty() ||
      (config.channels.bits & static_cast<uint8_t>(~valid.bits)) != 0U) {
    return Status::Error(Err::INVALID_CONFIG,
                         "Invalid configured channel mask");
  }
  if (config.referenceClock.source != RefClkSrc::INTERNAL &&
      config.referenceClock.source != RefClkSrc::EXTERNAL_CLOCK) {
    return Status::Error(Err::INVALID_CONFIG, "Reference clock required");
  }
  const uint32_t clock = config.referenceClock.frequencyHz;
  if (config.referenceClock.source == RefClkSrc::INTERNAL) {
    if (clock < INTERNAL_CLOCK_MIN_HZ || clock > INTERNAL_CLOCK_MAX_HZ) {
      return Status::Error(Err::INVALID_CONFIG,
                           "Internal reference clock outside 35-55 MHz");
    }
  } else if (clock < EXTERNAL_CLOCK_MIN_HZ ||
             clock > EXTERNAL_CLOCK_MAX_HZ) {
    return Status::Error(Err::INVALID_CONFIG,
                         "External reference clock outside 2-40 MHz");
  }
  if (config.referenceClock.tolerancePpm > MAX_TOLERANCE_PPM) {
    return Status::Error(Err::INVALID_CONFIG,
                         "Reference clock tolerance must be <1000000 ppm");
  }
  const uint64_t clockMin =
      static_cast<uint64_t>(clock) *
      (PPM_SCALE - config.referenceClock.tolerancePpm) / PPM_SCALE;
  const uint64_t clockMax = ceilDivide(
      static_cast<uint64_t>(clock) *
          (PPM_SCALE + config.referenceClock.tolerancePpm),
      PPM_SCALE);
  if (!isValidDeglitch(config.deglitch)) {
    return Status::Error(Err::INVALID_CONFIG, "Deglitch bandwidth required");
  }
  if (config.sensorActivation != SensorActivation::FULL_CURRENT &&
      config.sensorActivation != SensorActivation::LOW_POWER) {
    return Status::Error(Err::INVALID_CONFIG,
                         "Invalid sensor activation mode");
  }
  if (config.mode == OperatingMode::SINGLE_CHANNEL) {
    if (!isConcreteChannel(config.activeChannel) ||
        !config.channels.contains(config.activeChannel) ||
        config.channels.bits != channelBit(config.activeChannel).bits ||
        config.rrSequence != RRSequence::UNSPECIFIED) {
      return Status::Error(Err::INVALID_CONFIG,
                           "Single-channel mode/profile mismatch");
    }
  } else if (config.mode == OperatingMode::MULTI_CHANNEL_SEQUENTIAL) {
    if (config.activeChannel != Channel::NONE ||
        sequenceMask(config.rrSequence).bits != config.channels.bits ||
        (sequenceMask(config.rrSequence).bits &
         static_cast<uint8_t>(~valid.bits)) != 0U) {
      return Status::Error(Err::INVALID_CONFIG,
                           "Sequential mode/profile mismatch");
    }
  } else {
    return Status::Error(Err::INVALID_CONFIG, "Operating mode required");
  }
  if (config.highCurrentDriveEnabled &&
      (config.mode != OperatingMode::SINGLE_CHANNEL ||
       config.activeChannel != Channel::CH0)) {
    return Status::Error(Err::INVALID_CONFIG,
                         "High-current drive is single-channel CH0 only");
  }
  if (!config.intbDisabled && config.intbAsserted == nullptr) {
    return Status::Error(Err::INVALID_CONFIG,
                         "Enabled INTB requires observation callback");
  }

  const uint32_t bandwidth = deglitchBandwidthHz(config.deglitch);
  for (uint8_t index = 0; index < channelCount; ++index) {
    const uint8_t bit = static_cast<uint8_t>(1U << index);
    const ChannelConfig& channel = config.channel[index];
    if (channel.rcount < cmd::RCOUNT_MIN ||
        channel.finDivider < cmd::FIN_DIVIDER_MIN ||
        channel.finDivider > cmd::FIN_DIVIDER_MAX ||
        channel.frefDivider < cmd::FREF_DIVIDER_MIN ||
        channel.frefDivider > cmd::FREF_DIVIDER_MAX ||
        channel.driveCurrentCode > cmd::IDRIVE_MAX) {
      return Status::Error(Err::INVALID_CONFIG,
                           "Incomplete channel register profile", index);
    }
    if ((config.channels.bits & bit) == 0U) {
      continue;
    }
    if (config.mode == OperatingMode::MULTI_CHANNEL_SEQUENTIAL &&
        (channel.rcount < cmd::MULTI_RCOUNT_MIN ||
         channel.settleCount < cmd::MULTI_SETTLE_MIN)) {
      return Status::Error(Err::INVALID_CONFIG,
                           "Sequential timing below datasheet minimum", index);
    }
    if (channel.expectedSensorMinHz < SENSOR_FREQUENCY_MIN_HZ ||
        channel.expectedSensorMaxHz > SENSOR_FREQUENCY_MAX_HZ ||
        channel.expectedSensorMinHz > channel.expectedSensorMaxHz) {
      return Status::Error(Err::INVALID_CONFIG,
                           "Expected sensor range must be 1 kHz-10 MHz", index);
    }
    if (channel.expectedSensorMaxHz >= bandwidth) {
      return Status::Error(Err::INVALID_CONFIG,
                           "Deglitch bandwidth must exceed sensor maximum",
                           index);
    }
    if (channel.expectedSensorMaxHz >= 8750000U &&
        channel.finDivider < 2U) {
      return Status::Error(Err::INVALID_CONFIG,
                           "FIN divider must be >=2 at 8.75 MHz or above",
                           index);
    }
    const uint64_t frefMin = clockMin / channel.frefDivider;
    const uint64_t frefMax = ceilDivide(clockMax, channel.frefDivider);
    const uint64_t frefLimit =
        config.mode == OperatingMode::SINGLE_CHANNEL
            ? 35000000ULL
            : (config.referenceClock.source == RefClkSrc::EXTERNAL_CLOCK
                   ? 40000000ULL
                   : 55000000ULL);
    if (frefMin == 0U || frefMax > frefLimit) {
      return Status::Error(Err::INVALID_CONFIG,
                           "Channel reference clock tolerance exceeds limit",
                           index);
    }
    const uint64_t fin =
        ceilDivide(channel.expectedSensorMaxHz, channel.finDivider);
    if (fin * 4ULL >= frefMin) {
      return Status::Error(Err::INVALID_CONFIG,
                           "Worst-case sensor input must be below fREFx/4",
                           index);
    }
  }
  return Status::Ok();
}

uint16_t LDC1614::encodeErrorReporting(
    const ErrorReporting& reporting) {
  uint16_t value = 0;
  if (reporting.dataUnderRange) value |= cmd::MASK_ERRCFG_UR_ERR2OUT;
  if (reporting.dataOverRange) value |= cmd::MASK_ERRCFG_OR_ERR2OUT;
  if (reporting.dataWatchdog) value |= cmd::MASK_ERRCFG_WD_ERR2OUT;
  if (reporting.dataAmplitudeHigh) value |= cmd::MASK_ERRCFG_AH_ERR2OUT;
  if (reporting.dataAmplitudeLow) value |= cmd::MASK_ERRCFG_AL_ERR2OUT;
  if (reporting.statusUnderRange) value |= cmd::MASK_ERRCFG_UR_ERR2INT;
  if (reporting.statusOverRange) value |= cmd::MASK_ERRCFG_OR_ERR2INT;
  if (reporting.statusWatchdog) value |= cmd::MASK_ERRCFG_WD_ERR2INT;
  if (reporting.statusAmplitudeHigh) value |= cmd::MASK_ERRCFG_AH_ERR2INT;
  if (reporting.statusAmplitudeLow) value |= cmd::MASK_ERRCFG_AL_ERR2INT;
  if (reporting.statusZeroCount) value |= cmd::MASK_ERRCFG_ZC_ERR2INT;
  if (reporting.dataReady) value |= cmd::MASK_ERRCFG_DRDY_2INT;
  return value;
}

Status LDC1614::nominalDriveCurrentMicroamps(DriveCurrentCode code,
                                             uint16_t& microamps) {
  static constexpr uint16_t NOMINAL_MICROAMPS[32] = {
      16, 18, 20, 23, 28, 32, 40, 46, 52, 59, 72, 82, 95, 110, 127, 146,
      169, 195, 212, 244, 297, 342, 424, 489, 551, 635, 763, 880,
      1017, 1173, 1355, 1563};
  microamps = 0;
  if (code.value > cmd::IDRIVE_MAX) {
    return Status::Error(Err::INVALID_PARAM, "Invalid drive-current code");
  }
  microamps = NOMINAL_MICROAMPS[code.value];
  return Status::Ok();
}

DeviceStatus LDC1614::decodeDeviceStatus(uint16_t raw) {
  DeviceStatus status{};
  status.observed = true;
  status.raw = raw;
  status.errorUnderRange = (raw & cmd::MASK_STATUS_ERR_UR) != 0U;
  status.errorOverRange = (raw & cmd::MASK_STATUS_ERR_OR) != 0U;
  status.errorWatchdog = (raw & cmd::MASK_STATUS_ERR_WD) != 0U;
  status.errorAmplitudeHigh = (raw & cmd::MASK_STATUS_ERR_AHE) != 0U;
  status.errorAmplitudeLow = (raw & cmd::MASK_STATUS_ERR_ALE) != 0U;
  status.errorZeroCount = (raw & cmd::MASK_STATUS_ERR_ZC) != 0U;
  status.dataReady = (raw & cmd::MASK_STATUS_DRDY) != 0U;
  if ((raw & cmd::MASK_STATUS_UNREADCONV0) != 0U) {
    status.unreadChannels.bits |= channelBit(Channel::CH0).bits;
  }
  if ((raw & cmd::MASK_STATUS_UNREADCONV1) != 0U) {
    status.unreadChannels.bits |= channelBit(Channel::CH1).bits;
  }
  if ((raw & cmd::MASK_STATUS_UNREADCONV2) != 0U) {
    status.unreadChannels.bits |= channelBit(Channel::CH2).bits;
  }
  if ((raw & cmd::MASK_STATUS_UNREADCONV3) != 0U) {
    status.unreadChannels.bits |= channelBit(Channel::CH3).bits;
  }
  if (status.hasError()) {
    status.errorChannel = static_cast<Channel>(
        (raw & cmd::MASK_STATUS_ERR_CHAN) >> cmd::BIT_STATUS_ERR_CHAN);
  }
  return status;
}

ChannelSample LDC1614::decodeChannelSample(uint16_t msb,
                                           uint16_t lsb) {
  ChannelSample sample{};
  sample.rawDataMsb = msb;
  sample.rawDataLsb = lsb;
  sample.rawCount28 =
      (static_cast<uint32_t>(msb & cmd::MASK_DATA_MSB_DATA) << 16U) |
      static_cast<uint32_t>(lsb);
  if ((msb & cmd::MASK_DATA_ERR_UR) != 0U ||
      sample.rawCount28 == 0U) {
    sample.quality |= sampleQualityFlag(SampleQualityFlag::UNDER_RANGE);
  }
  if ((msb & cmd::MASK_DATA_ERR_OR) != 0U ||
      sample.rawCount28 == RAW_COUNT_MAX) {
    sample.quality |= sampleQualityFlag(SampleQualityFlag::OVER_RANGE);
  }
  if ((msb & cmd::MASK_DATA_ERR_WD) != 0U) {
    sample.quality |= sampleQualityFlag(SampleQualityFlag::WATCHDOG);
  }
  if ((msb & cmd::MASK_DATA_ERR_AE) != 0U) {
    sample.quality |=
        sampleQualityFlag(SampleQualityFlag::AMPLITUDE_SUSPECT);
  }
  return sample;
}

Status LDC1614::calculateSensorFrequencyHz(const Config& config,
                                           Channel channel,
                                           uint32_t rawCount28,
                                           double& frequencyHz) {
  frequencyHz = 0.0;
  LDC1614 validator;
  Status validation = validator._validateConfig(config);
  if (!validation.ok()) {
    return validation;
  }
  if (!isConcreteChannel(channel) ||
      !config.channels.contains(channel) || rawCount28 > RAW_COUNT_MAX) {
    return Status::Error(Err::INVALID_PARAM,
                         "Invalid channel or 28-bit count");
  }
  const ChannelConfig& channelConfig =
      config.channel[channelIndex(channel)];
  const double fref =
      static_cast<double>(config.referenceClock.frequencyHz) /
      static_cast<double>(channelConfig.frefDivider);
  const double ratio =
      static_cast<double>(rawCount28) / 268435456.0 +
      static_cast<double>(channelConfig.offset) / 65536.0;
  frequencyHz =
      ratio * fref * static_cast<double>(channelConfig.finDivider);
  if (!std::isfinite(frequencyHz)) {
    frequencyHz = 0.0;
    return Status::Error(Err::INVALID_PARAM,
                         "Frequency calculation overflow");
  }
  return Status::Ok();
}

Status LDC1614::estimateFrameTiming(const Config& config,
                                    ChannelMask channels,
                                    FrameTiming& timing) {
  timing = FrameTiming{};
  LDC1614 validator;
  Status validation = validator._validateConfig(config);
  if (!validation.ok()) {
    return validation;
  }
  if (channels.empty() ||
      (channels.bits & static_cast<uint8_t>(~config.channels.bits)) != 0U) {
    return Status::Error(Err::INVALID_PARAM,
                         "Timing mask must be configured and nonzero");
  }
  const uint64_t clockMin =
      static_cast<uint64_t>(config.referenceClock.frequencyHz) *
      (PPM_SCALE - config.referenceClock.tolerancePpm) / PPM_SCALE;
  if (clockMin == 0U) {
    return Status::Error(Err::INVALID_CONFIG,
                         "Reference clock minimum is zero");
  }
  // The post-sleep startup delay is clocked from fINT even when an external
  // reference is selected. Use the configured lower bound for internal-clock
  // operation and the datasheet internal-oscillator minimum otherwise.
  const uint64_t wakeClockMin =
      config.referenceClock.source == RefClkSrc::INTERNAL
          ? clockMin
          : static_cast<uint64_t>(INTERNAL_CLOCK_MIN_HZ);
  timing.wakeAndSettleUs =
      ceilDivide(WAKE_DELAY_CYCLES * MICROS_PER_SECOND, wakeClockMin);
  uint64_t switchTotalUs = 0;
  const ChannelMask timingChannels =
      config.mode == OperatingMode::MULTI_CHANNEL_SEQUENTIAL
          ? config.channels
          : channels;

  for (uint8_t index = 0; index < channelCountFor(config.variant); ++index) {
    if ((timingChannels.bits & static_cast<uint8_t>(1U << index)) == 0U) {
      continue;
    }
    const ChannelConfig& channel = config.channel[index];
    const uint64_t fref = clockMin / channel.frefDivider;
    if (fref == 0U) {
      return Status::Error(Err::INVALID_CONFIG,
                           "Channel minimum reference clock is zero", index);
    }
    const uint64_t settleCycles =
        channel.settleCount <= 1U
            ? 32ULL
            : static_cast<uint64_t>(channel.settleCount) * 16ULL;
    timing.wakeAndSettleUs +=
        ceilDivide(settleCycles * MICROS_PER_SECOND, fref);
    const uint64_t conversionCycles =
        static_cast<uint64_t>(channel.rcount) * 16ULL + 4ULL;
    timing.conversionUs +=
        ceilDivide(conversionCycles * MICROS_PER_SECOND, fref);
    const uint64_t switchNs =
        CHANNEL_SWITCH_BASE_NS +
        ceilDivide(CHANNEL_SWITCH_CYCLES * NANOS_PER_SECOND, fref);
    switchTotalUs += ceilDivide(switchNs, 1000ULL);
  }
  timing.sequentialFrameUs =
      timing.wakeAndSettleUs + timing.conversionUs + switchTotalUs;
  timing.acquisitionTransfers =
      static_cast<uint8_t>(2U + 2U * popcount(channels));
  return Status::Ok();
}

}  // namespace LDC1614
