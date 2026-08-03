#pragma once

#include <cstdarg>
#include <cstdint>

#include "LDC1614/LDC1614.h"

namespace ldc1614_idf_cli {

enum class I2cProbeResult : uint8_t {
  ACK,
  NACK,
  TIMEOUT,
  ERROR,
};

/// Tells the platform loop whether one prompt is due after a command/service
/// pass. Asynchronous work owns the outstanding prompt until it terminates.
enum class PromptAction : uint8_t {
  NONE,
  PRINT,
};

enum class CommandId : uint8_t {
  HELP,
  VERSION,
  COLOR,
  VERBOSE,
  BIND,
  END,
  INIT,
  APPLY,
  RESET_REAPPLY,
  SLEEP,
  WAKE,
  CANCEL,
  JOB,
  RESULT,
  INVALIDATE,
  BUS_RECOVER,
  READ,
  LAST,
  WATCH,
  SAMPLE_RATE,
  READY,
  STATUS,
  STATUS_RAW,
  INTB,
  INIT_DRIVE,
  CFG,
  PROFILE,
  ADDR,
  VARIANT,
  MODE,
  REFCLK,
  DEGLITCH,
  ACTIVATION,
  TIMEOUT,
  RP,
  AUTOAMP,
  HIGH_CURRENT,
  INTB_CONFIG,
  ERRORS,
  ERROR,
  RCOUNT,
  SETTLE,
  FIN_DIV,
  FREF_DIV,
  OFFSET,
  DRIVE,
  SENSOR_BOUNDS,
  PROBE,
  SCAN,
  DUMP,
  VERIFY,
  REG,
  WREG,
  DECODE,
  FREQ,
  TIMING,
  DRIVE_UA,
  DRIVER,
  STATE,
  SELFTEST,
  STRESS,
  STRESS_MIX,
  SOAK,
  SD,
  UNKNOWN,
};

enum class SessionKind : uint8_t {
  NONE,
  PROBE,
  SCAN,
  DUMP_CONFIG,
  DUMP_ALL,
  VERIFY,
  WATCH,
  SAMPLE_RATE,
  STRESS,
  STRESS_MIX,
  SOAK,
  SELF_TEST,
};

enum class SessionPhase : uint8_t {
  NONE,
  WAIT_INTERVAL,
  SAMPLE_READY,
  WAIT_ACQUIRE,
  PROBE_MANUFACTURER,
  PROBE_DEVICE,
  SCAN_ADDRESS,
  DUMP_REGISTER,
  VERIFY_REGISTER,
  MIX_STATUS,
  MIX_READY,
  MIX_INTB,
  MIX_INIT_DRIVE,
  MIX_ACQUIRE,
  SELF_PURE,
  SELF_MANUFACTURER,
  SELF_DEVICE,
  SELF_STATUS,
  SELF_READY,
  SELF_INTB,
  SELF_INIT_DRIVE,
  SELF_VERIFY,
  SELF_ACQUIRE,
};

enum class OperationOwner : uint8_t {
  NONE,
  COMMAND,
  SESSION,
};

/// Native ESP-IDF diagnostic CLI. The surrounding application owns the
/// serial input buffer, bus, pins, scheduling, and recovery policy.
class Ldc1614IdfCli {
 public:
  using VPrintfFn = void (*)(void* user, const char* fmt, va_list args);
  using MakeConfigFn = LDC1614::Config (*)(void* user);
  using NowMsFn = uint64_t (*)(void* user);
  using I2cProbeFn = I2cProbeResult (*)(uint8_t address, uint32_t timeoutMs,
                                        void* user);
  using I2cRecoverFn = LDC1614::Status (*)(void* user);
  using SdReadFn = LDC1614::Status (*)(bool& asserted, void* user);
  using SdWriteFn = LDC1614::Status (*)(bool asserted, void* user);

  struct Platform {
    void* user = nullptr;
    VPrintfFn vprintf = nullptr;
    MakeConfigFn makeConfig = nullptr;
    NowMsFn nowMs = nullptr;
    I2cProbeFn i2cProbe = nullptr;
    I2cRecoverFn i2cRecover = nullptr;
    SdReadFn sdRead = nullptr;
    SdWriteFn sdWrite = nullptr;
    uint32_t scanTimeoutMs = 50;
  };

  Ldc1614IdfCli(LDC1614::LDC1614& device, Platform platform);
  Ldc1614IdfCli(const Ldc1614IdfCli&) = delete;
  Ldc1614IdfCli& operator=(const Ldc1614IdfCli&) = delete;
  Ldc1614IdfCli(Ldc1614IdfCli&&) = delete;
  Ldc1614IdfCli& operator=(Ldc1614IdfCli&&) = delete;

  LDC1614::Config makeDefaultConfig() const;
  PromptAction processCommand(const char* commandLine);
  PromptAction service();
  bool asynchronousWorkActive() const;
  void printPrompt() const;
  void printStatus(const LDC1614::Status& status) const;
  void printHelp() const;
  void logError(const char* fmt, ...) const;
  void logInfo(const char* fmt, ...) const;
  void printf(const char* fmt, ...) const;
  void println(const char* text = "") const;
  uint64_t nowMs() const;

 private:
  static constexpr uint8_t MAX_TOKENS = 8;
  static constexpr uint16_t MAX_LINE_LENGTH = 127;
  static constexpr uint32_t MAX_SESSION_COUNT = 100000;

  struct ParsedLine {
    char storage[MAX_LINE_LENGTH + 1U]{};
    const char* argv[MAX_TOKENS]{};
    uint8_t argc = 0;
  };

  struct PendingOperation {
    OperationOwner owner = OperationOwner::NONE;
    LDC1614::OperationId id = 0;
    LDC1614::JobKind kind = LDC1614::JobKind::NONE;
    CommandId command = CommandId::UNKNOWN;
    uint64_t startedMs = 0;
  };

  struct ChannelStats {
    uint32_t selected = 0;
    uint32_t valid = 0;
    uint32_t fresh = 0;
    uint32_t errors = 0;
    uint32_t overruns = 0;
    uint32_t boundsFailures = 0;
    double minimumHz = 0.0;
    double maximumHz = 0.0;
    bool hasFrequency = false;
  };

  struct SessionStats {
    uint32_t requested = 0;
    uint32_t completed = 0;
    uint32_t passed = 0;
    uint32_t failed = 0;
    uint32_t skipped = 0;
    uint64_t startedMs = 0;
    uint32_t latencySamples = 0;
    uint64_t latencyTotalMs = 0;
    uint32_t minimumLatencyMs = UINT32_MAX;
    uint32_t maximumLatencyMs = 0;
    bool hasFailure = false;
    LDC1614::Status firstFailure = LDC1614::Status::Ok();
    LDC1614::Status lastFailure = LDC1614::Status::Ok();
    ChannelStats channel[4]{};
  };

  struct DiagnosticSession {
    SessionKind kind = SessionKind::NONE;
    SessionPhase phase = SessionPhase::NONE;
    CommandId command = CommandId::UNKNOWN;
    uint32_t id = 0;
    LDC1614::ChannelMask channels{};
    uint32_t count = 0;
    uint32_t index = 0;
    uint32_t periodMs = 0;
    uint64_t nextDueMs = 0;
    uint64_t readyDeadlineMs = 0;
    uint32_t readyChecks = 0;
    uint32_t sampleReadyChecks = 0;
    uint8_t channel = 0;
    uint8_t physicalChannel = 0;
    uint8_t scanAddress = 0x08;
    uint8_t scanFound = 0;
    uint8_t registerIndex = 0;
    bool iterationFailed = false;
    bool stopRequested = false;
    uint16_t manufacturerId = 0;
    uint16_t deviceId = 0;
    LDC1614::DeviceStatus lastReadyStatus{};
    bool hasReadyStatus = false;
    SessionStats stats{};
  };

  void vprintfToOutput(const char* fmt, va_list args) const;
  bool parseLine(const char* input, ParsedLine& parsed) const;
  CommandId resolveCommand(const char* token) const;
  bool commandAllowedWhileBusy(CommandId id) const;
  PromptAction usage(const char* synopsis) const;
  PromptAction promptActionForCurrentState() const;

  PromptAction handleCommand(CommandId id, const ParsedLine& line);
  PromptAction handleProfile(const ParsedLine& line);
  PromptAction handleSetting(CommandId id, const ParsedLine& line);
  PromptAction handleDump(const ParsedLine& line);
  PromptAction handleDecode(const ParsedLine& line);
  PromptAction handleSd(const ParsedLine& line);

  LDC1614::Status scheduleCommandJob(CommandId command,
                                     LDC1614::JobKind kind,
                                     LDC1614::ChannelMask channels,
                                     uint32_t deadlineMs);
  LDC1614::Status scheduleSessionAcquire();
  void drainResults();
  void handleOperationResult(const LDC1614::OperationResult& result);
  void handleSessionOperationResult(const LDC1614::OperationResult& result);

  LDC1614::Status startSession(SessionKind kind, CommandId command,
                               uint32_t count = 0,
                               LDC1614::ChannelMask channels = {},
                               uint32_t periodMs = 0,
                               uint8_t channel = 0);
  void advanceSession(uint64_t nowMs);
  void advanceSamplingSession(uint64_t nowMs);
  void advanceMixedSession();
  void advanceProbeSession();
  void advanceScanSession();
  void advanceDumpSession();
  void advanceVerifySession(bool selfTest);
  void advanceSelfTestSession();
  void stopActiveWork();
  void finishSession(const LDC1614::Status& status, const char* outcome);
  void printSessionSummary() const;
  void recordSessionFailure(const LDC1614::Status& status);
  void recordBatchStats(const LDC1614::SampleBatch& batch);
  void recordLatency(uint64_t startedMs, uint64_t completedMs);
  void selfTestPass(const char* name);
  void selfTestFail(const char* name, const LDC1614::Status& status);
  void selfTestSkip(const char* name, const char* reason);

  void ensureStagedProfile();
  void resetStagedProfile();
  void discardStagedProfile();
  void markStagedChanged(const char* field);
  void printConfig(const LDC1614::Config& config, const char* label) const;
  void printDriver() const;
  void printState() const;
  void printProgress() const;
  void printResult(const LDC1614::OperationResult& result) const;
  void printBatch(const LDC1614::SampleBatch& batch) const;
  void printDeviceStatus(const LDC1614::DeviceStatus& status,
                         const char* label = "STATUS") const;
  void printConfigFault(const LDC1614::ConfigFault& fault) const;
  void printQuality(LDC1614::SampleQualityFlags quality) const;

  LDC1614::OperationId nextOperationId();
  uint32_t nextSessionId();

  LDC1614::LDC1614& _device;
  Platform _platform;
  LDC1614::OperationId _nextOperationId = 1;
  uint32_t _nextSessionId = 1;
  PendingOperation _pending{};
  DiagnosticSession _session{};
  LDC1614::OperationResult _lastResult{};
  LDC1614::SampleBatch _lastBatch{};
  bool _hasLastResult = false;
  bool _hasLastBatch = false;
  LDC1614::Config _stagedConfig{};
  bool _stagedInitialized = false;
  bool _stagedDirty = false;
  bool _colorEnabled = true;
  bool _verbose = false;
  bool _promptDeferred = false;
  uint64_t _lastNowMs = 0;
};

}  // namespace ldc1614_idf_cli
