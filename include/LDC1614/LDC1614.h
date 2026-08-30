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

/// @brief Trust state of the desired configuration relative to hardware.
enum class AppliedConfigState : uint8_t {
  UNKNOWN = 0,      ///< Hardware configuration has not been established.
  APPLYING,         ///< A configuration-changing job is active.
  APPLIED_SLEEPING, ///< Complete desired profile is applied in sleep mode.
  APPLIED_ACTIVE,   ///< Complete desired profile is applied and converting.
  DIRTY,            ///< Hardware may be partial or differ from desired state.
};

/// @brief Cooperative multi-transfer operation class.
enum class JobKind : uint8_t {
  NONE = 0,          ///< No operation.
  INITIALIZE,        ///< Verify identity and replay the complete profile.
  APPLY_CONFIG,      ///< Replay the complete profile without identity reads.
  RESET_AND_REAPPLY, ///< Software reset, identity verification, and replay.
  ACQUIRE,           ///< Status-aware selected-channel acquisition.
};

/// @brief Exact protocol phase used for progress and fault provenance.
enum class JobPhase : uint8_t {
  NONE = 0,            ///< No active/recorded phase.
  VERIFY_MANUFACTURER, ///< Read and verify MANUFACTURER_ID.
  VERIFY_DEVICE,       ///< Read and verify DEVICE_ID.
  FORCE_SLEEP,         ///< Sleep before replay; clears DATA/error/INTB evidence.
  WRITE_RCOUNT,        ///< Write one channel RCOUNT register.
  WRITE_SETTLECOUNT,   ///< Write one channel SETTLECOUNT register.
  WRITE_CLOCK_DIVIDERS, ///< Write one channel CLOCK_DIVIDERS register.
  WRITE_OFFSET,        ///< Write one channel OFFSET register.
  WRITE_DRIVE_CURRENT, ///< Write one channel DRIVE_CURRENT register.
  WRITE_ERROR_CONFIG,  ///< Write ERROR_CONFIG.
  WRITE_MUX_CONFIG,    ///< Write MUX_CONFIG.
  WRITE_FINAL_CONFIG,  ///< Write final active/sleep CONFIG.
  SOFTWARE_RESET,      ///< Write RESET_DEV.
  READ_STATUS_BEFORE,  ///< Capture STATUS before destructive DATA reads.
  READ_DATA_MSB,       ///< Read DATAx_MSB and latch its LSB shadow.
  READ_DATA_LSB,       ///< Read the corresponding DATAx_LSB shadow.
  READ_STATUS_AFTER,   ///< Capture STATUS after DATA readout.
};

/// @brief Terminal classification independent of detailed Status.
enum class TerminalOutcome : uint8_t {
  NONE = 0,  ///< No terminal result.
  SUCCESS,   ///< Operation completed successfully.
  FAILED,    ///< Operation stopped on a reported failure.
  CANCELLED, ///< Owner cancelled the operation bus-silently.
  TIMED_OUT, ///< Absolute deadline expired before another callback began.
};

/// @brief Observable hardware-side effects accumulated by an operation.
enum class EffectFlag : uint8_t {
  NONE = 0,                       ///< No known device-side effect.
  READ_SIDE_EFFECTS = 1U << 0,   ///< Destructive read reached or may have reached hardware.
  PARTIAL_WRITE = 1U << 1,       ///< At least one register write was confirmed; partial on failure.
  INDETERMINATE_WRITE = 1U << 2, ///< Failed write may have reached hardware.
};

/// @brief Bitset of EffectFlag values.
using EffectFlags = uint8_t;

/// @param flag Effect to encode.
/// @return One-hot EffectFlags representation.
constexpr EffectFlags effectFlag(EffectFlag flag) {
  return static_cast<EffectFlags>(flag);
}

/// @param flags Effect bitset to inspect.
/// @param flag Effect to test.
/// @return True when `flag` is present.
constexpr bool hasEffect(EffectFlags flags, EffectFlag flag) {
  return (flags & effectFlag(flag)) != 0;
}

/// @brief Full provenance when trusted configuration/identity becomes partial,
/// uncertain, or disproven.
struct ConfigFault {
  bool valid = false;                    ///< True when provenance is populated.
  Status cause = Status::Ok();           ///< Original status that broke trust.
  JobKind job = JobKind::NONE;           ///< Operation in which the fault occurred.
  JobPhase phase = JobPhase::NONE;       ///< Exact protocol phase.
  uint8_t registerAddress = 0xFF;        ///< Register involved, or 0xFF when none.
  Channel channel = Channel::NONE;       ///< Channel involved, or NONE.
  EffectFlags effects = 0;               ///< Partial/indeterminate effect evidence.
};

/// @brief Parsed STATUS register snapshot retained before destructive DATA reads.
struct DeviceStatus {
  bool observed = false;  ///< True only when a STATUS register snapshot was read.
  Channel errorChannel = Channel::NONE; ///< Channel encoded by STATUS.ERR_CHAN.
  bool errorUnderRange = false;         ///< Sticky under-range indication.
  bool errorOverRange = false;          ///< Sticky over-range indication.
  bool errorWatchdog = false;           ///< Sticky watchdog indication.
  bool errorAmplitudeHigh = false;      ///< Sticky high-amplitude indication.
  bool errorAmplitudeLow = false;       ///< Sticky low-amplitude indication.
  bool errorZeroCount = false;          ///< Sticky zero-count indication.
  bool dataReady = false;               ///< STATUS.DRDY snapshot; always false
                                        ///< unless ErrorReporting::dataReady is set.
  ChannelMask unreadChannels{};         ///< STATUS.UNREADCONVx snapshot.
  uint16_t raw = 0;                     ///< Unmodified STATUS register value.

  /// @return True when any decoded silicon error is asserted.
  bool hasError() const {
    return errorUnderRange || errorOverRange || errorWatchdog ||
           errorAmplitudeHigh || errorAmplitudeLow || errorZeroCount;
  }
};

/// @brief Per-channel silicon/evidence quality bits.
enum class SampleQualityFlag : uint16_t {
  NONE = 0,                      ///< No quality evidence.
  FRESH = 1U << 0,              ///< Pre-read STATUS reported unread conversion.
  STALE = 1U << 1,              ///< Pre-read STATUS did not report unread data.
  UNDER_RANGE = 1U << 2,        ///< Raw endpoint (retained for fresh batches) or silicon flag.
  OVER_RANGE = 1U << 3,         ///< Raw endpoint (retained for fresh batches) or silicon flag.
  WATCHDOG = 1U << 4,           ///< DATA or STATUS watchdog fault attributed to this channel.
  AMPLITUDE_SUSPECT = 1U << 5,  ///< DATA/STATUS amplitude fault; sample is invalid.
  ZERO_COUNT = 1U << 6,         ///< STATUS zero-count was attributed to the channel.
  DATA_LOST = 1U << 7,          ///< New conversion pending after this coherent sample was read.
  CONFIG_UNKNOWN = 1U << 8,     ///< Compatibility-only; never emitted. Use AppliedConfigState.
};

/// @brief Bitset of SampleQualityFlag values.
using SampleQualityFlags = uint16_t;

/// @param flag Quality flag to encode.
/// @return One-hot SampleQualityFlags representation.
constexpr SampleQualityFlags sampleQualityFlag(SampleQualityFlag flag) {
  return static_cast<SampleQualityFlags>(flag);
}

/// @param flags Quality bitset to inspect.
/// @param flag Quality flag to test.
/// @return True when `flag` is present.
constexpr bool hasSampleQuality(SampleQualityFlags flags, SampleQualityFlag flag) {
  return (flags & sampleQualityFlag(flag)) != 0;
}

/// @brief Raw coherent channel result plus silicon/evidence quality.
struct ChannelSample {
  uint32_t rawCount28 = 0;          ///< Combined 28-bit conversion code.
  uint16_t rawDataMsb = 0;         ///< Unmodified DATAx_MSB register.
  uint16_t rawDataLsb = 0;         ///< Unmodified latched DATAx_LSB register.
  SampleQualityFlags quality = 0;   ///< Decoded quality evidence.
};

/// @brief One sequential readout batch; channels are not simultaneous samples.
/// @note Quality decoding is bounded by ERROR_CONFIG routing. STATUS reports
/// one error channel; additional concurrent channel faults require their DATA
/// routes. Zero-count has no DATA route and can latch after STATUS-before then
/// be cleared by DATAx_MSB before it is decoded. With routes disabled,
/// validChannels means fresh with no decoded range/silicon error, not
/// fault-free silicon. Overrun/DATA_LOST is orthogonal and can coexist.
struct SampleBatch {
  ChannelMask selectedChannels{};  ///< Channels requested by the owner.
  ChannelMask validChannels{};     ///< Fresh samples with no decoded range/silicon error.
  ChannelMask freshChannels{};     ///< Channels with pre-read unread evidence.
  ChannelMask errorChannels{};     ///< Channels with decoded error quality.
  ChannelMask overrunChannels{};   ///< Newer conversion pending after channel read.
  DeviceStatus statusBefore{};     ///< STATUS captured before DATA reads.
  DeviceStatus statusAfter{};      ///< STATUS captured after DATA reads.
  ChannelSample channel[4]{};      ///< Fixed storage indexed by physical channel.
  uint64_t completedUptimeMs = 0;  ///< Owner poll-boundary timestamp.
  uint32_t configRevision = 0;     ///< Desired/applied revision used for acquisition.
};

/// @brief Cache-only snapshot of active or terminal job progress.
struct JobProgress {
  bool active = false;                    ///< True while a job is admitted.
  OperationId operationId = 0;            ///< Caller correlation identity.
  JobKind kind = JobKind::NONE;           ///< Operation class.
  JobPhase phase = JobPhase::NONE;        ///< Current/terminal protocol phase.
  uint8_t registerAddress = 0xFF;  ///< Current/next protocol register.
  Channel channel = Channel::NONE;        ///< Current/next physical channel.
  uint64_t deadlineMs = 0;                ///< Immutable absolute owner deadline.
  uint8_t completedTransfers = 0;         ///< Physical callbacks already attempted.
  uint8_t maximumTransfers = 0;           ///< Fixed worst-case callbacks for the job.
  ChannelMask requestedChannels{};        ///< Requested acquisition channels.
  ChannelMask completedChannels{};        ///< Channels fully read into private scratch.
  EffectFlags effects = 0;                ///< Effects observed so far.
  uint32_t configRevision = 0;            ///< Desired revision captured at admission.
};

/// @brief Exactly-once terminal record returned by takeResult().
struct OperationResult {
  OperationId operationId = 0;            ///< Immutable caller correlation identity.
  JobKind kind = JobKind::NONE;           ///< Completed operation class.
  TerminalOutcome outcome = TerminalOutcome::NONE; ///< Terminal classification.
  Status status = Status::Ok();           ///< Complete terminal status.
  EffectFlags effects = 0;                ///< Read/write side-effect evidence.
  ConfigFault configFault{};              ///< Configuration-trust fault provenance.
  uint32_t configRevision = 0;            ///< Desired revision captured at admission.
  uint64_t completedUptimeMs = 0;  ///< Owner-supplied poll time; zero for bus-silent cancel.
  JobProgress finalProgress{};     ///< Terminal phase/count/channel provenance; active is false.
  bool hasSampleBatch = false;             ///< True only for successful acquisition.
  SampleBatch sampleBatch{};               ///< Atomic acquisition payload when present.
};

/// @brief Non-authoritative transport diagnostics. These never suppress owner requests.
struct TransportStats {
  uint32_t totalAttempts = 0;        ///< Saturating lifetime callback attempts.
  uint32_t totalSuccess = 0;         ///< Saturating lifetime successful callbacks.
  uint32_t totalFailures = 0;        ///< Saturating lifetime failed callbacks.
  Status lastStatus = Status::Ok();  ///< Most recent callback status.
};

/// @brief Conservative chip-time estimate; application-owned bus time is separate.
struct FrameTiming {
  uint64_t wakeAndSettleUs = 0;   ///< Conservative wake plus selected-channel settling time.
  uint64_t conversionUs = 0;      ///< Conservative selected-channel conversion time.
  uint64_t sequentialFrameUs = 0; ///< Conservative full configured auto-scan frame time.
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
/// Injected transport and INTB callbacks must not re-enter the same instance.
class LDC1614 {
 public:
  /// @brief Number of fixed terminal-result slots.
  static constexpr uint8_t RESULT_CAPACITY = 2;

  /// @brief Construct an unbound driver with no owned resources.
  LDC1614() = default;
  LDC1614(const LDC1614&) = delete;
  LDC1614& operator=(const LDC1614&) = delete;
  LDC1614(LDC1614&&) = delete;
  LDC1614& operator=(LDC1614&&) = delete;

  /// Validate and retain the explicit desired profile. Performs zero I2C.
  /// Returns BUSY if already bound; call end() before rebinding.
  /// @param config Complete explicit profile and non-owning callbacks.
  /// @return OK on bind, or a precise validation/precondition status.
  Status bind(const Config& config);

  /// Bus-silent cancellation/unbind. Pending terminal results are discarded.
  void end();

  /// @return True after successful bind and before end.
  bool isBound() const { return _bound; }
  /// @return Bound desired profile; meaningful only while bound.
  const Config& config() const { return _config; }
  /// @return Current trust state of desired configuration versus hardware.
  AppliedConfigState appliedConfigState() const { return _appliedState; }
  /// @return Monotonic desired-configuration revision for this binding.
  uint32_t configRevision() const { return _configRevision; }

  /// Replace the desired profile with zero I2C; a later apply is required.
  /// Requires a bound, idle driver that is not APPLIED_ACTIVE. Transport
  /// callbacks/context, address, and variant are binding identity and require
  /// end() followed by bind() when changed.
  /// @param config Replacement desired profile with unchanged binding identity.
  /// @return OK when retained, or a precise validation/precondition status.
  Status updateDesiredConfig(const Config& config);

  /// Verify both identity registers, then replay the complete profile into
  /// sleeping hardware. An identity-read failure makes applied configuration
  /// unknown and records ConfigFault provenance. Maximum: 15 LDC1612 or 25
  /// LDC1614 transfers.
  /// @param operationId Nonzero caller correlation identity.
  /// @param deadlineMs Immutable absolute deadline on the owner timeline. The
  /// deadline horizon must be less than 2^63 ms; natural uint64_t wrap is safe.
  /// @return IN_PROGRESS when admitted, otherwise a precise start failure.
  Status startInitialize(OperationId operationId, uint64_t deadlineMs);

  /// Replay the complete profile without identity reads. Requires established
  /// identity/config state. Maximum: 13 LDC1612 or 23 LDC1614 transfers.
  /// @param operationId Nonzero caller correlation identity.
  /// @param deadlineMs Immutable absolute deadline on the owner timeline. The
  /// deadline horizon must be less than 2^63 ms; natural uint64_t wrap is safe.
  /// @return IN_PROGRESS when admitted, otherwise a precise start failure.
  Status startApplyConfig(OperationId operationId, uint64_t deadlineMs);

  /// Issue software reset, verify identity, and replay the complete profile.
  /// Maximum: 16 LDC1612 or 26 LDC1614 transfers; no write is retried.
  /// @param operationId Nonzero caller correlation identity.
  /// @param deadlineMs Immutable absolute deadline on the owner timeline. The
  /// deadline horizon must be less than 2^63 ms; natural uint64_t wrap is safe.
  /// @return IN_PROGRESS when admitted, otherwise a precise start failure.
  Status startResetAndReapply(OperationId operationId, uint64_t deadlineMs);

  /// Acquire one atomic software batch for a nonempty subset of configured
  /// channels. Requires APPLIED_ACTIVE. Maximum: 2 + 2N transfers.
  /// @param channels Nonempty subset of configured channels to read.
  /// @param operationId Nonzero caller correlation identity.
  /// @param deadlineMs Immutable absolute deadline on the owner timeline. The
  /// deadline horizon must be less than 2^63 ms; natural uint64_t wrap is safe.
  /// @return IN_PROGRESS when admitted, otherwise a precise start failure.
  Status startAcquire(ChannelMask channels, OperationId operationId,
                      uint64_t deadlineMs);

  /// Advance at most maxTransfers physical callbacks. nowMs is one snapshot of
  /// the owner's 64-bit millisecond timeline. Deadline comparisons tolerate
  /// natural uint64_t wrap for horizons shorter than 2^63 ms; owners using a
  /// wrapping 32-bit clock must still extend it before calling the driver. Time
  /// does not advance internally. maxTransfers==0 is bus-silent but still
  /// applies deadline state. Callback timeouts share the remaining deadline
  /// budget so their worst-case sum cannot exceed the remaining time observed
  /// at this poll boundary.
  /// @param nowMs Current owner monotonic time in milliseconds.
  /// @param maxTransfers Maximum physical callbacks permitted by this call.
  /// @return IN_PROGRESS while active, terminal status on completion, or a
  /// precise precondition/error status.
  Status poll(uint64_t nowMs, uint8_t maxTransfers = 1);

  /// Idempotent and bus-silent. An active job produces one CANCELLED result;
  /// its completion timestamp is zero because cancelJob() accepts no clock.
  /// @return OK after cancellation or when no job is active.
  Status cancelJob();

  /// @return Cache-only progress snapshot; performs zero I2C.
  JobProgress jobProgress() const { return _progress; }
  /// @return True when at least one terminal result is queued.
  bool resultAvailable() const { return _resultCount != 0; }
  /// Remove the oldest terminal result exactly once. The output is cleared
  /// when no result is ready.
  /// @param out Destination for the oldest result.
  /// @return OK when a result was removed, or RESULT_NOT_READY.
  Status takeResult(OperationResult& out);

  /// Mark hardware configuration unknown after removal, reset, brownout, or
  /// owner recovery. Performs zero I2C and preserves the supplied full status
  /// as provenance. An active job is cancelled and produces one CANCELLED
  /// terminal result that occupies a result slot and reserves its operation id
  /// until takeResult() drains it, so drain results before starting recovery
  /// work or the next start reports RESULT_QUEUE_FULL.
  /// @param reason Owner-observed cause retained as configuration-fault evidence.
  void invalidateAppliedState(const Status& reason);

  /// @return Cache-only configuration fault provenance.
  ConfigFault configFault() const { return _configFault; }
  /// @return Non-authoritative lifetime transport diagnostics.
  TransportStats transportStats() const { return _transportStats; }

  /// @brief Read and decode STATUS in one bounded callback.
  /// @param out Destination snapshot, cleared before the attempt.
  /// @return Precise precondition or transport status.
  Status readDeviceStatus(DeviceStatus& out);
  /// Observe DRDY without discarding the STATUS snapshot that may contain
  /// destructive error evidence. An inactive enabled INTB can return
  /// bus-silently with a zero snapshot.
  /// @param ready Receives the decoded data-ready state.
  /// @param observedStatus Receives the STATUS snapshot when one is read.
  /// @return Precise precondition, INTB-observation, or transport status.
  Status readDataReady(bool& ready, DeviceStatus& observedStatus);
  /// @brief Enter sleep mode with one bounded CONFIG write.
  /// @details Sleep entry clears DATA registers, unread and latched error
  /// evidence, and de-asserts INTB. Drain required evidence before sleeping;
  /// after wake, wait for a fresh conversion before treating DATA as current.
  /// @return Precise precondition or transport status.
  Status sleep();
  /// @brief Leave sleep mode with one bounded CONFIG write.
  /// @return Precise precondition or transport status.
  Status wake();
  /// @brief Read the silicon-selected initial IDRIVE code.
  /// @param channel Physical channel to read.
  /// @param code Receives the five-bit INIT_IDRIVE code.
  /// @return Precise parameter, precondition, or transport status.
  Status readInitDriveCurrent(Channel channel, uint8_t& code);

  /// @brief Read one mapped register for advanced diagnostics.
  /// @details Access is variant-aware. DATA/STATUS reads retain their
  /// documented destructive effects.
  /// @param reg Register address.
  /// @param value Receives the 16-bit register value.
  /// @return Precise parameter, precondition, or transport status.
  Status readRegister16(uint8_t reg, uint16_t& value);
  /// @brief Write one mapped writable register for advanced diagnostics.
  /// @details Read-only DATA/STATUS/identity registers are rejected. Possible
  /// configuration mutations invalidate trusted applied state.
  /// @param reg Register address.
  /// @param value 16-bit value to write.
  /// @return Precise parameter, precondition, or transport status.
  Status writeRegister16(uint8_t reg, uint16_t value);
  /// @brief Observe INTB through the optional bus-silent callback.
  /// @param asserted Receives the application-observed asserted state.
  /// @return Precise configuration or callback status.
  Status readIntb(bool& asserted) const;

  /// Validate one complete explicit desired profile without retaining it or
  /// invoking any callback.
  /// @param config Complete desired profile and non-owning callbacks.
  /// @return OK or the same precise validation status used by bind().
  static Status validateConfig(const Config& config);

  /// Return the canonical replay value and stable readback-comparison mask for
  /// one persistent configuration register. The comparison mask excludes
  /// read-only DRIVE_CURRENT.INIT_IDRIVE and CONFIG.SLEEP_MODE_EN because
  /// sleep/active state is controlled at runtime. Documented mandatory R/W
  /// constants are included. Compare `(actual & comparisonMask)` with
  /// `(expectedValue & comparisonMask)`.
  /// @param config Complete desired profile.
  /// @param reg Configuration-register address for the selected variant.
  /// @param expectedValue Receives the exact canonical replay value.
  /// @param comparisonMask Receives bits with stable Config-owned readback.
  /// @return OK, a precise configuration status, or INVALID_PARAM when reg is
  /// not part of the selected variant's persistent configuration profile.
  static Status expectedConfigurationRegister(const Config& config,
                                               uint8_t reg,
                                               uint16_t& expectedValue,
                                               uint16_t& comparisonMask);

  /// Pure, checked inverse DATA conversion using the configured nominal
  /// reference clock, FIN/FREF dividers, and OFFSET.
  /// @param config Explicit validated-style profile.
  /// @param channel Channel whose divider/offset fields apply.
  /// @param rawCount28 Raw 28-bit conversion code.
  /// @param frequencyHz Receives calculated sensor frequency in hertz.
  /// @return OK or a precise invalid-input/configuration status.
  static Status calculateSensorFrequencyHz(const Config& config, Channel channel,
                                           uint32_t rawCount28, double& frequencyHz);

  /// Pure conservative first-post-wake device-time and read-transfer estimate.
  /// Multi-channel timing includes the complete configured auto-scan sequence
  /// even when channels is only a readout subset. Application queue, lock,
  /// callback, and processing time are excluded.
  /// @param config Explicit desired profile.
  /// @param channels Nonempty configured readout subset.
  /// @param timing Receives conservative fixed-unit estimates.
  /// @return OK or a precise invalid-input/configuration status.
  static Status estimateFrameTiming(const Config& config, ChannelMask channels,
                                    FrameTiming& timing);

  /// @param reporting Typed error-routing policy.
  /// @return Encoded ERROR_CONFIG register value with reserved bits clear.
  static uint16_t encodeErrorReporting(const ErrorReporting& reporting);
  /// Convert a five-bit IDRIVE code to its datasheet nominal current in uA.
  /// @param code Five-bit IDRIVE code.
  /// @param microamps Receives nominal current in microamps.
  /// @return OK or INVALID_PARAM for an out-of-range code.
  static Status nominalDriveCurrentMicroamps(DriveCurrentCode code,
                                              uint16_t& microamps);
  /// @param raw Raw STATUS register value.
  /// @return Decoded, observed STATUS snapshot.
  static DeviceStatus decodeDeviceStatus(uint16_t raw);
  /// @param msb Raw DATAx_MSB register value.
  /// @param lsb Raw latched DATAx_LSB register value.
  /// @return Coherent raw count and decoded DATA quality flags.
  static ChannelSample decodeChannelSample(uint16_t msb, uint16_t lsb);

 private:
  static constexpr uint8_t INVALID_REGISTER = 0xFF;

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
  SampleBatch _buildAcquisition(uint64_t completedUptimeMs) const;
  void _clearActiveJob();
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

  OperationResult _results[RESULT_CAPACITY]{};
  uint8_t _resultHead = 0;
  uint8_t _resultCount = 0;
};

}  // namespace LDC1614
