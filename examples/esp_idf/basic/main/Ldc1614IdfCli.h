#pragma once

#include <cstdint>

#include "LDC1614/LDC1614.h"

class Ldc1614IdfCli {
 public:
  Ldc1614IdfCli(LDC1614::LDC1614& device, const LDC1614::Config& defaultConfig);

  void printBanner() const;
  void printPrompt() const;
  void println(const char* text) const;
  void processLine(const char* line);
  uint32_t nowMs() const;

 private:
  void printHelp() const;
  void printStatus(const LDC1614::Status& status) const;
  void printDriver() const;
  void printConfig() const;
  void handleBegin();
  void handleProbe();
  void handleDeviceStatus();
  void handleReady();
  void handleSleep();
  void handleWake();
  void handleRead(int argc, char* argv[]);
  void handleReadAll();
  void handleRecover();
  void handleTiming(int argc, char* argv[]);
  void handleSelfTest();

  LDC1614::LDC1614& _device;
  LDC1614::Config _defaultConfig;
};
