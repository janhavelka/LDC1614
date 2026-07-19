/// @file LDC1614.h
/// @brief Cooperative, externally scheduled LDC1612/LDC1614 driver.
#pragma once

#include <cstddef>
#include <cstdint>

#include "LDC1614/CommandTable.h"
#include "LDC1614/Config.h"
#include "LDC1614/Status.h"
#include "LDC1614/Version.h"

namespace LDC1614 {

enum class AppliedConfigState : uint8_t {
  UNKNOWN = 0,
  APPLYING,
  APPLIED_SLEEPING,
  APPLIED_ACTIVE,
  DIRTY,
};

enum class JobKind : uint8_t {
  NONE = 0,
  INITIALIZE,
  APPLY_CONFIG,
  RESET_AND_REAPPLY,
  ACQUIRE,
};

enum class JobPhase : uint8_t {
  NONE = 0,
  VERIFY_MANUFACTURER,
  VERIFY_DEVICE,
  FORCE_SLEEP,
  WRITE_RCOUNT,
  WRITE_SETTLECOUNT,
  WRITE_CLOCK_DIVIDERS,
  WRITE_OFFSET,
  WRITE_DRIVE_CURRENT,
  WRITE_ERROR_CONFIG,
  WRITE_MUX_CONFIG,
  WRITE_FINAL_CONFIG,
  SOFTWARE_RESET,
  READ_STATUS_BEFORE,
  READ_DATA_MSB,
  READ_DATA_LSB,
  READ_STATUS_AFTER,
};

enum class TerminalOutcome : uint8_t {
  NONE = 0,
  SUCCESS,
  FAILED,
  CANCELLED,
  TIMED_OUT,
};

enum class EffectFlag : uint8_t {
  NONE = 0,
  READ_SIDE_EFFECTS = 1U << 0,
  PARTIAL_WRITE = 1U << 1,
  INDETERMINATE_WRITE = 1U << 2,
};

using EffectFlags = uint8_t;

constexpr EffectFlags effectFlag(EffectFlag flag) {
  return static_cast<EffectFlags>(flag);
}

constexpr bool hasEffect(EffectFlags flags, EffectFlag flag) {
  return (flags & effectFlag(flag)) != 0;
}

/// @brief Full provenance when trusted configuration/identity becomes partial,
/// uncertain, or disproven.
struct ConfigFault {
  bool valid = false;
  Status cause = Status::Ok();
  JobKind job = JobKind::NONE;
  JobPhase phase = JobPhase::NONE;
  uint8_t registerAddress = 0xFF;
  Channel channel = Channel::NONE;
  EffectFlags effects = 0;
};

/// @brief Parsed STATUS register snapshot retained before destructive DATA reads.
struct DeviceStatus {
  bool observed = false;  ///< True only when a STATUS register snapshot was read.
  Channel errorChannel = Channel::NONE;
  bool errorUnderRange = false;
  bool errorOverRange = false;
  bool errorWatchdog = false;
  bool errorAmplitudeHigh = false;
  bool errorAmplitudeLow = false;
  bool errorZeroCount = false;
  bool dataReady = false;
  ChannelMask unreadChannels{};
  uint16_t raw = 0;

  bool hasError() const {
    return errorUnderRange || errorOverRange || errorWatchdog ||
           errorAmplitudeHigh || errorAmplitudeLow || errorZeroCount;
  }
};

enum class SampleQualityFlag : uint16_t {
  NONE = 0,
  FRESH = 1U << 0,
  STALE = 1U << 1,
  UNDER_RANGE = 1U << 2,
  OVER_RANGE = 1U << 3,
  WATCHDOG = 1U << 4,
  AMPLITUDE_SUSPECT = 1U << 5,
  ZERO_COUNT = 1U << 6,
  DATA_LOST = 1U << 7,
  CONFIG_UNKNOWN = 1U << 8,
};

using SampleQualityFlags = uint16_t;

constexpr SampleQualityFlags sampleQualityFlag(SampleQualityFlag flag) {
  return static_cast<SampleQualityFlags>(flag);
}

constexpr bool hasSampleQuality(SampleQualityFlags flags, SampleQualityFlag flag) {
  return (flags & sampleQualityFlag(flag)) != 0;
}

struct ChannelSample {
  uint32_t rawCount28 = 0;
  uint16_t rawDataMsb = 0;
  uint16_t rawDataLsb = 0;
  SampleQualityFlags quality = 0;
};

/// @brief One sequential readout batch; channels are not simultaneous samples.
struct SampleBatch {
  ChannelMask selectedChannels{};
  ChannelMask validChannels{};     ///< Silicon-usable samples only.
  ChannelMask freshChannels{};
  ChannelMask errorChannels{};
  ChannelMask overrunChannels{};   ///< New unread data observed after readout.
  DeviceStatus statusBefore{};
  DeviceStatus statusAfter{};
  ChannelSample channel[4]{};
  uint64_t completedUptimeMs = 0;
  uint32_t configRevision = 0;
};

struct JobProgress {
  bool active = false;
  OperationId operationId = 0;
  JobKind kind = JobKind::NONE;
  JobPhase phase = JobPhase::NONE;
  uint8_t registerAddress = 0xFF;  ///< Current/next protocol register.
  Channel channel = Channel::NONE;
  uint64_t deadlineMs = 0;
  uint8_t completedTransfers = 0;
  uint8_t maximumTransfers = 0;
  ChannelMask requestedChannels{};
  ChannelMask completedChannels{};
  EffectFlags effects = 0;
  uint32_t configRevision = 0;
};

/// @brief Exactly-once terminal record returned by takeResult().
struct OperationResult {
  OperationId operationId = 0;
  JobKind kind = JobKind::NONE;
  TerminalOutcome outcome = TerminalOutcome::NONE;
  Status status = Status::Ok();
  EffectFlags effects = 0;
  ConfigFault configFault{};
  uint32_t configRevision = 0;
  uint64_t completedUptimeMs = 0;  ///< Owner-supplied poll time; zero for bus-silent cancel.
  JobProgress finalProgress{};     ///< Terminal phase/count/channel provenance; active is false.
  bool hasSampleBatch = false;
  SampleBatch sampleBatch{};
};

/// @brief Non-authoritative transport diagnostics. These never suppress owner requests.
struct TransportStats {
  uint32_t totalAttempts = 0;
  uint32_t totalSuccess = 0;
  uint32_t totalFailures = 0;
  Status lastStatus = Status::Ok();
};

/// @brief Conservative chip-time estimate; application-owned bus time is separate.
struct FrameTiming {
  uint64_t wakeAndSettleUs = 0;
  uint64_t conversionUs = 0;
  uint64_t sequentialFrameUs = 0;
  uint8_t acquisitionTransfers = 0;  ///< STATUS-before + DATA pairs + STATUS-after.
};

/// @brief Framework-neutral cooperative LDC1612/LDC1614 driver.
///
/// bind(), cancellation, progress/result access, invalidation, and end() are
/// bus-silent. Multi-transfer chip procedures advance only through poll(). Each
/// transport callback consumes one unit of the caller's transfer budget. There
/// are no hidden retries, waits, yields, bus recovery, logging, or allocation.
/// Instances are neither thread-safe nor ISR-safe; the application must
/// serialize access and retain ownership of bus scheduling and recovery.
class LDC1614 {
 public:
  static constexpr uint8_t RESULT_CAPACITY = 2;

  LDC1614() = default;
  LDC1614(const LDC1614&) = delete;
  LDC1614& operator=(const LDC1614&) = delete;
  LDC1614(LDC1614&&) = delete;
  LDC1614& operator=(LDC1614&&) = delete;

  /// Validate and retain the explicit desired profile. Performs zero I2C.
  /// Returns BUSY if already bound; call end() before rebinding.
  Status bind(const Config& config);

  /// Bus-silent cancellation/unbind. Pending terminal results are discarded.
  void end();

  bool isBound() const { return _bound; }
  const Config& config() const { return _config; }
  AppliedConfigState appliedConfigState() const { return _appliedState; }
  uint32_t configRevision() const { return _configRevision; }

  /// Replace the desired profile with zero I2C; a later apply is required.
  /// Requires a bound, idle driver that is not APPLIED_ACTIVE. Transport
  /// callbacks/context, address, and variant are binding identity and require
  /// end() followed by bind() when changed.
  Status updateDesiredConfig(const Config& config);

  /// Verify both identity registers, then replay the complete profile into
  /// sleeping hardware. Maximum: 16 LDC1612 or 26 LDC1614 transfers.
  Status startInitialize(OperationId operationId, uint64_t deadlineMs);

  /// Replay the complete profile without identity reads. Requires established
  /// identity/config state. Maximum: 14 LDC1612 or 24 LDC1614 transfers.
  Status startApplyConfig(OperationId operationId, uint64_t deadlineMs);

  /// Issue software reset, verify identity, and replay the complete profile.
  /// Maximum: 17 LDC1612 or 27 LDC1614 transfers; no write is retried.
  Status startResetAndReapply(OperationId operationId, uint64_t deadlineMs);

  /// Acquire one atomic software batch for a nonempty subset of configured
  /// channels. Requires APPLIED_ACTIVE. Maximum: 2 + 2N transfers.
  Status startAcquire(ChannelMask channels, OperationId operationId,
                      uint64_t deadlineMs);

  /// Advance at most maxTransfers physical callbacks. nowMs is one owner clock
  /// snapshot; time does not advance internally. maxTransfers==0 is bus-silent
  /// but still applies deadline state. Callback timeouts share the remaining
  /// deadline budget so their worst-case sum cannot exceed the remaining time
  /// observed at this poll boundary.
  Status poll(uint64_t nowMs, uint8_t maxTransfers = 1);

  /// Idempotent and bus-silent. An active job produces one CANCELLED result;
  /// its completion timestamp is zero because cancelJob() accepts no clock.
  Status cancelJob();

  JobProgress jobProgress() const { return _progress; }
  bool resultAvailable() const { return _resultCount != 0; }
  /// Remove the oldest terminal result exactly once. The output is cleared
  /// when no result is ready.
  Status takeResult(OperationResult& out);

  /// Mark hardware configuration unknown after removal, reset, brownout, or owner recovery.
  /// Performs zero I2C and preserves the supplied full status as provenance.
  void invalidateAppliedState(const Status& reason);

  ConfigFault configFault() const { return _configFault; }
  TransportStats transportStats() const { return _transportStats; }

  // Owner-safe one-transfer operations. Each uses Config::i2cTimeoutMs and
  // performs no internal retry.
  Status readDeviceStatus(DeviceStatus& out);
  /// Observe DRDY without discarding the STATUS snapshot that may contain
  /// destructive error evidence. An inactive enabled INTB can return
  /// bus-silently with a zero snapshot.
  Status readDataReady(bool& ready, DeviceStatus& observedStatus);
  Status sleep();
  Status wake();
  Status readInitDriveCurrent(Channel channel, uint8_t& code);

  // Advanced diagnostics. DATA/STATUS reads retain their documented
  // destructive effects; configuration writes invalidate trusted applied state.
  Status readRegister16(uint8_t reg, uint16_t& value);
  Status writeRegister16(uint8_t reg, uint16_t value);
  Status readIntb(bool& asserted) const;

  /// Pure, checked inverse DATA conversion using the configured nominal
  /// reference clock, FIN/FREF dividers, and OFFSET.
  static Status calculateSensorFrequencyHz(const Config& config, Channel channel,
                                           uint32_t rawCount28, double& frequencyHz);

  /// Pure conservative first-post-wake device-time and read-transfer estimate.
  /// Multi-channel timing includes the complete configured auto-scan sequence
  /// even when channels is only a readout subset. Application queue, lock,
  /// callback, and processing time are excluded.
  static Status estimateFrameTiming(const Config& config, ChannelMask channels,
                                    FrameTiming& timing);

  static uint16_t encodeErrorReporting(const ErrorReporting& reporting);
  /// Convert a five-bit IDRIVE code to its datasheet nominal current in uA.
  static Status nominalDriveCurrentMicroamps(DriveCurrentCode code,
                                              uint16_t& microamps);
  static DeviceStatus decodeDeviceStatus(uint16_t raw);
  static ChannelSample decodeChannelSample(uint16_t msb, uint16_t lsb);

 private:
  static constexpr uint8_t INVALID_REGISTER = 0xFF;

  Status _validateConfig(const Config& config) const;
  Status _startJob(JobKind kind, ChannelMask channels, OperationId operationId,
                   uint64_t deadlineMs, uint8_t maximumTransfers);
  Status _pollInitializeOrApply(uint64_t nowMs, uint8_t& remainingTransfers,
                                uint32_t transferTimeoutMs, bool resetFirst);
  Status _pollAcquire(uint64_t nowMs, uint8_t& remainingTransfers,
                      uint32_t transferTimeoutMs);
  Status _readRegister(uint8_t reg, uint16_t& value, uint32_t timeoutMs);
  Status _writeRegister(uint8_t reg, uint16_t value, uint32_t timeoutMs);
  Status _tracked(const Status& status);
  void _recordConfigFault(const Status& cause, JobPhase phase, uint8_t reg,
                          Channel channel, EffectFlags effects);
  Status _finishJob(TerminalOutcome outcome, const Status& status,
                    uint64_t completedUptimeMs);
  void _commitAcquisition(uint64_t completedUptimeMs);
  void _clearActiveJob();
  void _clearSampleCache();
  bool _operationIdInUse(OperationId operationId) const;
  bool _reserveResultSlot() const;
  uint8_t _configuredChannelCount() const;

  Config _config{};
  bool _bound = false;
  AppliedConfigState _appliedState = AppliedConfigState::UNKNOWN;
  uint32_t _configRevision = 0;
  ConfigFault _configFault{};
  TransportStats _transportStats{};

  JobProgress _progress{};
  uint8_t _jobStep = 0;
  Channel _jobChannel = Channel::NONE;
  JobPhase _lastWritePhase = JobPhase::NONE;
  uint8_t _lastWriteRegister = INVALID_REGISTER;
  Channel _lastWriteChannel = Channel::NONE;
  uint16_t _scratchMsb = 0;
  DeviceStatus _scratchStatusBefore{};
  DeviceStatus _scratchStatusAfter{};
  ChannelSample _scratchSamples[4]{};
  ChannelMask _scratchCompleted{};

  SampleBatch _lastBatch{};
  bool _hasLastBatch = false;

  OperationResult _results[RESULT_CAPACITY]{};
  uint8_t _resultHead = 0;
  uint8_t _resultCount = 0;
};

}  // namespace LDC1614
