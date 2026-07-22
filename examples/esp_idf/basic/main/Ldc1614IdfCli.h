#pragma once

#include <cstdint>

#include "LDC1614/LDC1614.h"

struct Ldc1614IdfI2c;

/// Native fixed-buffer ESP-IDF diagnostic CLI.
class Ldc1614IdfCli {
 public:
  Ldc1614IdfCli(LDC1614::LDC1614& device,
                const LDC1614::Config& defaultConfig,
                Ldc1614IdfI2c& transport);

  void printBanner() const;
  void printPrompt() const;
  void println(const char* text) const;
  void processLine(const char* line);
  void service(uint64_t nowMs);

 private:
  void printHelp() const;
  void printStatus(const LDC1614::Status& status) const;
  void printDriver() const;
  void printConfig() const;
  void printProgress() const;
  void printDeviceStatus(const LDC1614::DeviceStatus& status) const;
  void printResult(const LDC1614::OperationResult& result) const;
  void scanI2c() const;
  void startInitialize(bool resetFirst, uint64_t nowMs);
  void startAcquire(LDC1614::ChannelMask channels, uint64_t nowMs);
  LDC1614::OperationId nextOperationId();

  LDC1614::LDC1614& _device;
  LDC1614::Config _defaultConfig;
  Ldc1614IdfI2c& _transport;
  LDC1614::OperationId _nextOperationId = 1;
  uint64_t _lastNowMs = 0;
};
