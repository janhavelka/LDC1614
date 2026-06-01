#pragma once

#include <cstdarg>
#include <cstdint>

#include "LDC1614/LDC1614.h"

namespace ldc1614_cli {

enum class I2cProbeResult : uint8_t {
  ACK,
  TIMEOUT,
  ERROR,
};

struct HealthSnapshot {
  LDC1614::DriverState state = LDC1614::DriverState::UNINIT;
  bool online = false;
  uint8_t consecutiveFailures = 0;
  uint32_t totalFailures = 0;
  uint32_t totalSuccess = 0;
};

class Cli {
public:
  using VPrintfFn = void (*)(void* user, const char* fmt, va_list args);
  using MakeConfigFn = LDC1614::Config (*)(void* user);
  using NowMsFn = uint32_t (*)(void* user);
  using DelayMsFn = void (*)(uint32_t ms, void* user);
  using YieldFn = void (*)(void* user);
  using I2cProbeFn = I2cProbeResult (*)(uint8_t address, uint32_t timeoutMs, void* user);

  struct Platform {
    void* user = nullptr;
    VPrintfFn vprintf = nullptr;
    MakeConfigFn makeConfig = nullptr;
    NowMsFn nowMs = nullptr;
    DelayMsFn delayMs = nullptr;
    YieldFn yield = nullptr;
    I2cProbeFn i2cProbe = nullptr;
    uint32_t scanTimeoutMs = 50;
  };

  Cli(LDC1614::LDC1614& device, Platform platform);

  LDC1614::Config makeDefaultConfig() const;
  void processCommand(const char* cmdLine);
  void printPrompt() const;
  void printStatus(const LDC1614::Status& st) const;
  void printDriverHealth() const;
  void printHelp() const;
  void logError(const char* fmt, ...) const;
  void logWarn(const char* fmt, ...) const;
  void logInfo(const char* fmt, ...) const;
  void logVerbose(const char* fmt, ...) const;
  void printf(const char* fmt, ...) const;
  void println(const char* text = "") const;
  uint32_t nowMs() const;
  void delayMs(uint32_t ms) const;
  void yield() const;

private:
  void vprintfToOutput(const char* fmt, va_list args) const;
  void vlog(uint8_t minLevel, const char* color, const char* tag,
            const char* fmt, va_list args) const;
  HealthSnapshot captureHealth() const;
  void printHealthCompact() const;
  void printHealthDiff(const HealthSnapshot& before, const HealthSnapshot& after) const;
  void scanI2c();
  void printBusDiagnostics();
  void printVersionInfo() const;
  void printDeviceStatus(const LDC1614::DeviceStatus& ds) const;
  void printConfig();
  void printIdentity();
  void printRawIdentity(uint8_t address) const;
  void printChannelData(uint8_t ch, const LDC1614::ChannelData& data) const;
  LDC1614::Status rawReadRegister16(uint8_t i2cAddress, uint8_t reg, uint16_t& value) const;
  LDC1614::Status rawWriteRegister16(uint8_t i2cAddress, uint8_t reg, uint16_t value) const;
  bool readIdentityRaw(uint8_t address, uint16_t& manufacturer, uint16_t& deviceId,
                       LDC1614::Status& failure) const;
  uint8_t diagnosticAddress() const;
  void runSelfTest();
  void runStress(int count);
  void runStressMix(int count);
  void runDemo(int count);

  LDC1614::LDC1614& _device;
  Platform _platform;
  bool _verboseMode = false;
};

}  // namespace ldc1614_cli
