#pragma once

#include <cstdarg>
#include <cstdint>

#include "LDC1614/LDC1614.h"

namespace ldc1614_cli {

enum class I2cProbeResult : uint8_t {
  ACK,
  NACK,
  TIMEOUT,
  ERROR,
};

/// Diagnostic Arduino CLI. It demonstrates owner-driven jobs; it is not a
/// production bus manager and deliberately contains no retry/recovery policy.
class Cli {
 public:
  using VPrintfFn = void (*)(void* user, const char* fmt, va_list args);
  using MakeConfigFn = LDC1614::Config (*)(void* user);
  using NowMsFn = uint64_t (*)(void* user);
  using I2cProbeFn = I2cProbeResult (*)(uint8_t address, uint32_t timeoutMs,
                                        void* user);

  struct Platform {
    void* user = nullptr;
    VPrintfFn vprintf = nullptr;
    MakeConfigFn makeConfig = nullptr;
    NowMsFn nowMs = nullptr;
    I2cProbeFn i2cProbe = nullptr;
    uint32_t scanTimeoutMs = 50;
  };

  Cli(LDC1614::LDC1614& device, Platform platform);

  LDC1614::Config makeDefaultConfig() const;
  void processCommand(const char* commandLine);
  void service();
  void printPrompt() const;
  void printStatus(const LDC1614::Status& status) const;
  void printHelp() const;
  void logError(const char* fmt, ...) const;
  void logInfo(const char* fmt, ...) const;
  void printf(const char* fmt, ...) const;
  void println(const char* text = "") const;
  uint64_t nowMs() const;

 private:
  void vprintfToOutput(const char* fmt, va_list args) const;
  void startInitialize(bool resetFirst);
  void startAcquire(LDC1614::ChannelMask channels);
  void printDriver() const;
  void printConfig() const;
  void printProgress() const;
  void printResult(const LDC1614::OperationResult& result) const;
  void printBatch(const LDC1614::SampleBatch& batch) const;
  void printDeviceStatus(const LDC1614::DeviceStatus& status) const;
  /// External diagnostic loop capped at 126 one-attempt probes. It is not a
  /// core driver job or a production shared-bus scan policy.
  void scanI2c() const;
  LDC1614::OperationId nextOperationId();
  uint64_t deadlineFromNow() const;

  LDC1614::LDC1614& _device;
  Platform _platform;
  LDC1614::OperationId _nextOperationId = 1;
};

}  // namespace ldc1614_cli
