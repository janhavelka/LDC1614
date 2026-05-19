/// @file main.cpp
/// @brief LDC1614 basic bringup example
/// @note This is an EXAMPLE, not part of the library

#include <Arduino.h>
#include <cstdlib>

#include "examples/common/BoardConfig.h"
#include "examples/common/BusDiag.h"
#include "examples/common/I2cScanner.h"
#include "examples/common/I2cTransport.h"
#include "examples/common/CliStyle.h"
#include "examples/common/Log.h"

#include "LDC1614/LDC1614.h"

// ============================================================================
// Globals
// ============================================================================

LDC1614::LDC1614 device;
bool verboseMode = false;

// ============================================================================
// Helper Functions
// ============================================================================

const char* errToStr(LDC1614::Err err) {
  using LDC1614::Err;
  switch (err) {
    case Err::OK:                   return "OK";
    case Err::NOT_INITIALIZED:      return "NOT_INITIALIZED";
    case Err::INVALID_CONFIG:       return "INVALID_CONFIG";
    case Err::I2C_ERROR:            return "I2C_ERROR";
    case Err::TIMEOUT:              return "TIMEOUT";
    case Err::INVALID_PARAM:        return "INVALID_PARAM";
    case Err::DEVICE_NOT_FOUND:     return "DEVICE_NOT_FOUND";
    case Err::CONVERSION_NOT_READY: return "CONVERSION_NOT_READY";
    case Err::BUSY:                 return "BUSY";
    case Err::IN_PROGRESS:          return "IN_PROGRESS";
    case Err::I2C_NACK_ADDR:        return "I2C_NACK_ADDR";
    case Err::I2C_NACK_DATA:        return "I2C_NACK_DATA";
    case Err::I2C_TIMEOUT:          return "I2C_TIMEOUT";
    case Err::I2C_BUS:              return "I2C_BUS";
    case Err::SENSOR_ERROR:         return "SENSOR_ERROR";
    default:                        return "UNKNOWN";
  }
}

const char* stateToStr(LDC1614::DriverState st) {
  using LDC1614::DriverState;
  switch (st) {
    case DriverState::UNINIT:   return "UNINIT";
    case DriverState::READY:    return "READY";
    case DriverState::DEGRADED: return "DEGRADED";
    case DriverState::OFFLINE:  return "OFFLINE";
    default:                    return "UNKNOWN";
  }
}

const char* stateColor(LDC1614::DriverState st, bool online, uint8_t consecutiveFailures) {
  if (st == LDC1614::DriverState::UNINIT) {
    return LOG_COLOR_YELLOW;
  }
  return LOG_COLOR_STATE(online, consecutiveFailures);
}

const char* goodIfZeroColor(uint32_t value) {
  return (value == 0U) ? LOG_COLOR_GREEN : LOG_COLOR_RED;
}

const char* goodIfNonZeroColor(uint32_t value) {
  return (value > 0U) ? LOG_COLOR_GREEN : LOG_COLOR_YELLOW;
}

const char* onOffColor(bool enabled) {
  return enabled ? LOG_COLOR_GREEN : LOG_COLOR_RESET;
}

const char* yesNoColor(bool value) {
  return value ? LOG_COLOR_GREEN : LOG_COLOR_YELLOW;
}

const char* skipCountColor(uint32_t value) {
  return (value > 0U) ? LOG_COLOR_YELLOW : LOG_COLOR_RESET;
}

const char* successRateColor(float pct) {
  if (pct >= 99.9f) return LOG_COLOR_GREEN;
  if (pct >= 80.0f) return LOG_COLOR_YELLOW;
  return LOG_COLOR_RED;
}

const char* staleTimeColor(bool isErrorTimestamp) {
  return isErrorTimestamp ? LOG_COLOR_GREEN : LOG_COLOR_YELLOW;
}

void printStatus(const LDC1614::Status& st) {
  Serial.printf("  Status: %s%s%s (code=%u, detail=%ld)\n",
                LOG_COLOR_RESULT(st.ok()),
                errToStr(st.code),
                LOG_COLOR_RESET,
                static_cast<unsigned>(st.code),
                static_cast<long>(st.detail));
  if (st.msg && st.msg[0]) {
    Serial.printf("  Message: %s%s%s\n", LOG_COLOR_YELLOW, st.msg, LOG_COLOR_RESET);
  }
}

void printDriverHealth() {
  const uint32_t now = millis();
  const uint32_t totalOk = device.totalSuccess();
  const uint32_t totalFail = device.totalFailures();
  const uint32_t total = totalOk + totalFail;
  const float successRate = (total > 0U)
                                ? (100.0f * static_cast<float>(totalOk) / static_cast<float>(total))
                                : 0.0f;
  const LDC1614::Status lastErr = device.lastError();
  const LDC1614::DriverState st = device.state();
  const bool online = device.isOnline();

  Serial.println("=== Driver Health ===");
  Serial.printf("  State: %s%s%s\n",
                stateColor(st, online, device.consecutiveFailures()),
                stateToStr(st),
                LOG_COLOR_RESET);
  Serial.printf("  Online: %s%s%s\n",
                online ? LOG_COLOR_GREEN : LOG_COLOR_RED,
                log_bool_str(online),
                LOG_COLOR_RESET);
  Serial.printf("  Sleeping: %s%s%s\n",
                device.isSleeping() ? LOG_COLOR_YELLOW : LOG_COLOR_GREEN,
                log_bool_str(device.isSleeping()),
                LOG_COLOR_RESET);
  Serial.printf("  Consecutive failures: %s%u%s\n",
                goodIfZeroColor(device.consecutiveFailures()),
                device.consecutiveFailures(),
                LOG_COLOR_RESET);
  Serial.printf("  Total success: %s%lu%s\n",
                goodIfNonZeroColor(totalOk),
                static_cast<unsigned long>(totalOk),
                LOG_COLOR_RESET);
  Serial.printf("  Total failures: %s%lu%s\n",
                goodIfZeroColor(totalFail),
                static_cast<unsigned long>(totalFail),
                LOG_COLOR_RESET);
  Serial.printf("  Success rate: %s%.1f%%%s\n",
                successRateColor(successRate),
                successRate,
                LOG_COLOR_RESET);

  const uint32_t lastOkMs = device.lastOkMs();
  if (lastOkMs > 0U) {
    Serial.printf("  Last OK: %s%lu ms ago (at %lu ms)%s\n",
                  LOG_COLOR_GREEN,
                  static_cast<unsigned long>(now - lastOkMs),
                  static_cast<unsigned long>(lastOkMs),
                  LOG_COLOR_RESET);
  } else {
    Serial.printf("  Last OK: %snever%s\n", staleTimeColor(false), LOG_COLOR_RESET);
  }

  const uint32_t lastErrorMs = device.lastErrorMs();
  if (lastErrorMs > 0U) {
    Serial.printf("  Last error: %s%lu ms ago (at %lu ms)%s\n",
                  LOG_COLOR_RED,
                  static_cast<unsigned long>(now - lastErrorMs),
                  static_cast<unsigned long>(lastErrorMs),
                  LOG_COLOR_RESET);
  } else {
    Serial.printf("  Last error: %snever%s\n", staleTimeColor(true), LOG_COLOR_RESET);
  }

  if (!lastErr.ok()) {
    Serial.printf("  Error code: %s%s%s\n",
                  LOG_COLOR_RED,
                  errToStr(lastErr.code),
                  LOG_COLOR_RESET);
    Serial.printf("  Error detail: %ld\n", static_cast<long>(lastErr.detail));
    if (lastErr.msg && lastErr.msg[0]) {
      Serial.printf("  Error msg: %s%s%s\n", LOG_COLOR_YELLOW, lastErr.msg, LOG_COLOR_RESET);
    }
  }
}

void printHelp() {
  Serial.println();
  cli::printHelpHeader("LDC1614 CLI Help");
  cli::printHelpSection("Common");
  cli::printHelpItem("help / ?", "Show this help");
  cli::printHelpItem("version / ver", "Print firmware and library version info");
  cli::printHelpItem("scan", "Scan I2C bus");

  cli::printHelpSection("Data");
  cli::printHelpItem("read", "Read configured channels");
  cli::printHelpItem("read <ch>", "Read specific channel (0-3)");
  cli::printHelpItem("readblocking", "Blocking read configured channels (waits for DRDY)");
  cli::printHelpItem("readblocking <ch>", "Blocking read specific channel");
  cli::printHelpItem("sample <ch>", "Get last cached sample (no I2C)");
  cli::printHelpItem("sampleage <ch>", "Show age of cached sample (ms)");
  cli::printHelpItem("drdy", "Check data ready");
  cli::printHelpItem("status", "Read and parse STATUS register");
  cli::printHelpItem("status_raw", "Read raw STATUS register value");
  cli::printHelpItem("freq <ch> <fRef>", "Read channel and calc sensor frequency");
  cli::printHelpItem("timing <ch> <fRef>", "Calc conversion, settling, and sample time");

  cli::printHelpSection("Control");
  cli::printHelpItem("init / begin", "Initialize/reinitialize device");
  cli::printHelpItem("end", "Shut down driver (returns to UNINIT)");
  cli::printHelpItem("sleep", "Enter sleep mode (stop conversions)");
  cli::printHelpItem("wake", "Wake and start conversions");
  cli::printHelpItem("reset", "Software reset (returns to UNINIT)");
  cli::printHelpItem("resetreapply", "Soft reset + re-apply config (stays READY)");

  cli::printHelpSection("Configuration");
  cli::printHelpItem("cfg / settings", "Print active configuration snapshot");
  cli::printHelpItem("snapshot", "Print settings snapshot struct (no I2C)");
  cli::printHelpItem("channels", "Show configured channel count");
  cli::printHelpItem("activech", "Show current active channel");
  cli::printHelpItem("activech <ch>", "Set active channel (single-ch mode)");
  cli::printHelpItem("single <ch>", "Set single-channel mode and active channel");
  cli::printHelpItem("autoscan <2|3|4>", "Set auto-scan sequence length");
  cli::printHelpItem("deglitch <1|3|10|33>", "Set input deglitch bandwidth in MHz");
  cli::printHelpItem("errcfg [mask]", "Show or set ERROR_CONFIG bit mask");
  cli::printHelpItem("intb [0|1]", "Show or enable/disable INTB output");
  cli::printHelpItem("refclk <int|ext>", "Set reference clock source");
  cli::printHelpItem("activate <full|low>", "Set sensor activation current policy");
  cli::printHelpItem("rpoverride <0|1>", "Enable/disable fixed RP override drive");
  cli::printHelpItem("autoamp <0|1>", "Enable/disable auto amplitude correction");
  cli::printHelpItem("highcurrent <0|1>", "Enable/disable high-current Ch0 drive");
  cli::printHelpItem("rcount <ch> <val>", "Set RCOUNT for channel");
  cli::printHelpItem("settle <ch> <val>", "Set SETTLECOUNT for channel");
  cli::printHelpItem("clkdiv <ch> <fin> <fref>", "Set clock dividers");
  cli::printHelpItem("offset <ch> <val>", "Set conversion offset");
  cli::printHelpItem("idrive <ch> <val>", "Set drive current (0-31)");
  cli::printHelpItem("initidrive <ch>", "Read auto-calibrated INIT_IDRIVE");

  cli::printHelpSection("Registers");
  cli::printHelpItem("reg <addr>", "Read register (hex address)");
  cli::printHelpItem("wreg <addr> <val>", "Write register (diagnostic only; may desync cached config)");

  cli::printHelpSection("Diagnostics");
  cli::printHelpItem("drv", "Show driver state and health");
  cli::printHelpItem("online", "Check if device is online");
  cli::printHelpItem("id", "Read MANUFACTURER_ID and DEVICE_ID");
  cli::printHelpItem("probe", "Probe device (no health tracking)");
  cli::printHelpItem("recover", "Manual recovery attempt");
  cli::printHelpItem("verbose [0|1]", "Enable/disable verbose output");
  cli::printHelpItem("stress [N]", "Run N read cycles (default 10)");
  cli::printHelpItem("stress_mix [N]", "Run N mixed-operation stress cycles");
  cli::printHelpItem("selftest", "Run safe command self-test report");
}

void printVersionInfo() {
  Serial.println("=== Version Info ===");
  Serial.printf("  Example firmware build: %s %s\n", __DATE__, __TIME__);
  Serial.printf("  LDC1614 library version: %s\n", LDC1614::VERSION);
  Serial.printf("  LDC1614 version code: %d (major=%d minor=%d patch=%d)\n",
                LDC1614::VERSION_INT,
                LDC1614::VERSION_MAJOR,
                LDC1614::VERSION_MINOR,
                LDC1614::VERSION_PATCH);
}

void printDeviceStatus(const LDC1614::DeviceStatus& ds) {
  Serial.printf("  STATUS raw=0x%04X drdy=%s%s%s errCh=%u\n",
                ds.raw,
                yesNoColor(ds.dataReady),
                ds.dataReady ? "YES" : "no",
                LOG_COLOR_RESET,
                ds.errChan);
  if (ds.hasError()) {
    Serial.printf("  Errors: %sUR=%d OR=%d WD=%d AH=%d AL=%d ZC=%d%s\n",
                  LOG_COLOR_RED,
                  ds.errUnderRange, ds.errOverRange, ds.errWatchdog,
                  ds.errAmplitudeHigh, ds.errAmplitudeLow, ds.errZeroCount,
                  LOG_COLOR_RESET);
  }
  Serial.printf("  Unread: ch0=%d ch1=%d ch2=%d ch3=%d\n",
                ds.unreadConv[0], ds.unreadConv[1],
                ds.unreadConv[2], ds.unreadConv[3]);
}

bool parseI32(const String& token, int32_t& out) {
  char* end = nullptr;
  const long value = strtol(token.c_str(), &end, 0);
  if (end == token.c_str() || *end != '\0') {
    return false;
  }
  out = static_cast<int32_t>(value);
  return true;
}

bool parseU32(const String& token, uint32_t& out) {
  char* end = nullptr;
  const unsigned long value = strtoul(token.c_str(), &end, 0);
  if (end == token.c_str() || *end != '\0') {
    return false;
  }
  out = static_cast<uint32_t>(value);
  return true;
}

bool parseBoolToken(String token, bool& out) {
  token.trim();
  token.toLowerCase();
  if (token == "1" || token == "on" || token == "true" || token == "yes" || token == "enable") {
    out = true;
    return true;
  }
  if (token == "0" || token == "off" || token == "false" || token == "no" || token == "disable") {
    out = false;
    return true;
  }
  return false;
}

bool parseRRSequence(const String& token, LDC1614::RRSequence& out) {
  int32_t count = 0;
  if (!parseI32(token, count)) {
    return false;
  }
  if (count == 2) {
    out = LDC1614::RRSequence::CH0_CH1;
    return true;
  }
  if (count == 3) {
    out = LDC1614::RRSequence::CH0_CH1_CH2;
    return true;
  }
  if (count == 4) {
    out = LDC1614::RRSequence::CH0_CH1_CH2_CH3;
    return true;
  }
  return false;
}

bool parseDeglitch(String token, LDC1614::Deglitch& out) {
  token.trim();
  token.toLowerCase();
  if (token == "1" || token == "1mhz") {
    out = LDC1614::Deglitch::BW_1MHZ;
    return true;
  }
  if (token == "3" || token == "3mhz" || token == "3.3" || token == "3.3mhz") {
    out = LDC1614::Deglitch::BW_3MHZ;
    return true;
  }
  if (token == "10" || token == "10mhz") {
    out = LDC1614::Deglitch::BW_10MHZ;
    return true;
  }
  if (token == "33" || token == "33mhz") {
    out = LDC1614::Deglitch::BW_33MHZ;
    return true;
  }
  return false;
}

bool parseRefClk(String token, LDC1614::RefClkSrc& out) {
  token.trim();
  token.toLowerCase();
  if (token == "int" || token == "internal") {
    out = LDC1614::RefClkSrc::INTERNAL;
    return true;
  }
  if (token == "ext" || token == "external" || token == "clkin") {
    out = LDC1614::RefClkSrc::EXT_CLK;
    return true;
  }
  return false;
}

bool parseActivation(String token, LDC1614::SensorActivation& out) {
  token.trim();
  token.toLowerCase();
  if (token == "full" || token == "full_current") {
    out = LDC1614::SensorActivation::FULL_CURRENT;
    return true;
  }
  if (token == "low" || token == "low_power") {
    out = LDC1614::SensorActivation::LOW_POWER;
    return true;
  }
  return false;
}

const char* rrSequenceToStr(LDC1614::RRSequence sequence) {
  switch (sequence) {
    case LDC1614::RRSequence::CH0_CH1:         return "CH0_CH1";
    case LDC1614::RRSequence::CH0_CH1_CH2:     return "CH0_CH1_CH2";
    case LDC1614::RRSequence::CH0_CH1_CH2_CH3: return "CH0_CH1_CH2_CH3";
    default:                                   return "UNKNOWN";
  }
}

const char* deglitchToStr(LDC1614::Deglitch deglitch) {
  switch (deglitch) {
    case LDC1614::Deglitch::BW_1MHZ:  return "1 MHz";
    case LDC1614::Deglitch::BW_3MHZ:  return "3.3 MHz";
    case LDC1614::Deglitch::BW_10MHZ: return "10 MHz";
    case LDC1614::Deglitch::BW_33MHZ: return "33 MHz";
    default:                          return "UNKNOWN";
  }
}

const char* refClkToStr(LDC1614::RefClkSrc source) {
  return source == LDC1614::RefClkSrc::EXT_CLK ? "EXT_CLK" : "INTERNAL";
}

const char* activationToStr(LDC1614::SensorActivation activation) {
  return activation == LDC1614::SensorActivation::LOW_POWER ? "LOW_POWER" : "FULL_CURRENT";
}

LDC1614::Config makeDefaultConfig() {
  LDC1614::Config cfg;
  cfg.i2cWrite = transport::wireWrite;
  cfg.i2cWriteRead = transport::wireWriteRead;
  cfg.i2cUser = &Wire;
  cfg.nowMs = [](void*) { return millis(); };
  cfg.cooperativeYield = [](void*) { yield(); };
  cfg.i2cAddress = 0x2A;
  cfg.channelCount = 4;
  cfg.i2cTimeoutMs = board::I2C_TIMEOUT_MS;

  cfg.autoScan = false;
  cfg.activeChan = 0;
  cfg.deglitch = LDC1614::Deglitch::BW_10MHZ;
  cfg.refClkSrc = LDC1614::RefClkSrc::INTERNAL;
  cfg.rpOverrideEn = true;
  cfg.autoAmpDis = true;
  cfg.sensorActivation = LDC1614::SensorActivation::FULL_CURRENT;

  cfg.channel[0].rcount = 0x04D6;
  cfg.channel[0].settleCount = 0x000A;
  cfg.channel[0].finDivider = 1;
  cfg.channel[0].frefDivider = 1;
  cfg.channel[0].offset = 0x0000;
  cfg.channel[0].idrive = 10;

  cfg.errorConfig = LDC1614::cmd::MASK_ERRCFG_DRDY_2INT |
                    LDC1614::cmd::MASK_ERRCFG_UR_ERR2INT |
                    LDC1614::cmd::MASK_ERRCFG_OR_ERR2INT |
                    LDC1614::cmd::MASK_ERRCFG_WD_ERR2INT |
                    LDC1614::cmd::MASK_ERRCFG_AH_ERR2INT |
                    LDC1614::cmd::MASK_ERRCFG_AL_ERR2INT;

  if (board::INTB_PIN >= 0) {
    cfg.intbPin = board::INTB_PIN;
    cfg.gpioRead = board::readIntbPin;
  }

  cfg.offlineThreshold = 5;
  return cfg;
}

void runSelfTest() {
  struct TestStats {
    uint32_t pass = 0;
    uint32_t fail = 0;
    uint32_t skip = 0;
  } stats;

  enum class SelftestOutcome : uint8_t { PASS, FAIL, SKIP };
  auto report = [&](const char* name, SelftestOutcome outcome, const char* note) {
    const bool passed = (outcome == SelftestOutcome::PASS);
    const bool skipped = (outcome == SelftestOutcome::SKIP);
    const char* color = skipped ? LOG_COLOR_YELLOW : LOG_COLOR_RESULT(passed);
    const char* tag = skipped ? "SKIP" : (passed ? "PASS" : "FAIL");
    Serial.printf("  [%s%s%s] %s", color, tag, LOG_COLOR_RESET, name);
    if (note && note[0]) {
      Serial.printf(" - %s", note);
    }
    Serial.println();
    if (skipped) {
      stats.skip++;
    } else if (passed) {
      stats.pass++;
    } else {
      stats.fail++;
    }
  };
  auto reportCheck = [&](const char* name, bool passed, const char* note) {
    report(name, passed ? SelftestOutcome::PASS : SelftestOutcome::FAIL, note);
  };
  auto reportSkip = [&](const char* name, const char* note) {
    report(name, SelftestOutcome::SKIP, note);
  };

  Serial.println("=== LDC1614 selftest (safe commands) ===");

  const uint32_t succBefore = device.totalSuccess();
  const uint32_t failBefore = device.totalFailures();
  const uint8_t consBefore = device.consecutiveFailures();

  const LDC1614::Status pst = device.probe();
  if (pst.code == LDC1614::Err::NOT_INITIALIZED) {
    reportSkip("probe responds", "driver not initialized");
    reportSkip("remaining checks", "selftest aborted");
    Serial.printf("Selftest result: pass=%s%lu%s fail=%s%lu%s skip=%s%lu%s\n",
                  goodIfNonZeroColor(stats.pass), static_cast<unsigned long>(stats.pass), LOG_COLOR_RESET,
                  goodIfZeroColor(stats.fail), static_cast<unsigned long>(stats.fail), LOG_COLOR_RESET,
                  skipCountColor(stats.skip), static_cast<unsigned long>(stats.skip), LOG_COLOR_RESET);
    return;
  }
  const bool probeHealthUnchanged =
      device.totalSuccess() == succBefore &&
      device.totalFailures() == failBefore &&
      device.consecutiveFailures() == consBefore;
  reportCheck("probe responds", pst.ok(), pst.ok() ? "" : errToStr(pst.code));
  reportCheck("probe no-health-side-effects", probeHealthUnchanged, "");

  // Read STATUS register
  LDC1614::DeviceStatus ds;
  LDC1614::Status st = device.readDeviceStatus(ds);
  reportCheck("readDeviceStatus", st.ok(), st.ok() ? "" : errToStr(st.code));

  // Read a register
  uint16_t regVal = 0;
  st = device.readRegister16(LDC1614::cmd::REG_MANUFACTURER_ID, regVal);
  reportCheck("readRegister16(MFR_ID)", st.ok(), st.ok() ? "" : errToStr(st.code));
  if (st.ok()) {
    reportCheck("MFR_ID == 0x5449",
                regVal == LDC1614::cmd::MANUFACTURER_ID_VALUE, "");
  }

  st = device.readRegister16(LDC1614::cmd::REG_DEVICE_ID, regVal);
  reportCheck("readRegister16(DEV_ID)", st.ok(), st.ok() ? "" : errToStr(st.code));
  if (st.ok()) {
    reportCheck("DEV_ID == 0x3055",
                regVal == LDC1614::cmd::DEVICE_ID_VALUE, "");
  }

  // Sleep/wake cycle
  st = device.sleep();
  reportCheck("sleep", st.ok(), st.ok() ? "" : errToStr(st.code));
  reportCheck("isSleeping after sleep", device.isSleeping(), "");

  st = device.wake();
  reportCheck("wake", st.ok(), st.ok() ? "" : errToStr(st.code));
  reportCheck("not sleeping after wake", !device.isSleeping(), "");

  // Put back to sleep for safety
  st = device.sleep();
  reportCheck("sleep (restore)", st.ok(), st.ok() ? "" : errToStr(st.code));

  // Recovery
  st = device.recover();
  reportCheck("recover", st.ok(), st.ok() ? "" : errToStr(st.code));
  reportCheck("isOnline", device.isOnline(), "");

  Serial.printf("Selftest result: pass=%s%lu%s fail=%s%lu%s skip=%s%lu%s\n",
                goodIfNonZeroColor(stats.pass), static_cast<unsigned long>(stats.pass), LOG_COLOR_RESET,
                goodIfZeroColor(stats.fail), static_cast<unsigned long>(stats.fail), LOG_COLOR_RESET,
                skipCountColor(stats.skip), static_cast<unsigned long>(stats.skip), LOG_COLOR_RESET);
}

// ============================================================================
// Config Dump
// ============================================================================

void printConfig() {
  Serial.println("=== Active Configuration ===");
  const LDC1614::Config& cfg = device.getConfig();
  Serial.printf("  I2C address: 0x%02X\n", cfg.i2cAddress);
  Serial.printf("  Mode: %s  RR sequence: %s  Deglitch: %s\n",
                cfg.autoScan ? "auto-scan" : "single-channel",
                rrSequenceToStr(cfg.rrSequence),
                deglitchToStr(cfg.deglitch));
  Serial.printf("  RefClk: %s  Activation: %s\n",
                refClkToStr(cfg.refClkSrc),
                activationToStr(cfg.sensorActivation));
  Serial.printf("  RPoverride: %s  AutoAmp: %s  HighCurrentDrv: %s\n",
                log_bool_str(cfg.rpOverrideEn),
                log_bool_str(!cfg.autoAmpDis),
                log_bool_str(cfg.highCurrentDrv));
  Serial.printf("  INTB configured: %s  INTB output: %s\n",
                log_bool_str(cfg.intbPin >= 0),
                log_bool_str(cfg.intbPin >= 0 && !cfg.intbDisable));
  Serial.printf("  Cached ERROR_CONFIG: 0x%04X\n", cfg.errorConfig);

  // Read key registers to show current config
  uint16_t regVal = 0;
  auto st = device.readRegister16(LDC1614::cmd::REG_MUX_CONFIG, regVal);
  if (st.ok()) {
    Serial.printf("  MUX_CONFIG: 0x%04X\n", regVal);
  }
  st = device.readRegister16(LDC1614::cmd::REG_CONFIG, regVal);
  if (st.ok()) {
    Serial.printf("  CONFIG: 0x%04X (sleep=%d)\n", regVal, (regVal >> 13) & 1);
  }
  st = device.readRegister16(LDC1614::cmd::REG_ERROR_CONFIG, regVal);
  if (st.ok()) {
    Serial.printf("  ERROR_CONFIG: 0x%04X\n", regVal);
  }

  Serial.printf("  Channel count: %u\n", device.channelCount());
  Serial.printf("  Active channel: %u\n", device.getActiveChannel());
  Serial.printf("  Sleeping: %s%s%s\n",
                device.isSleeping() ? LOG_COLOR_YELLOW : LOG_COLOR_GREEN,
                log_bool_str(device.isSleeping()),
                LOG_COLOR_RESET);

  // Per-channel register dump
  for (uint8_t ch = 0; ch < device.channelCount() && ch < 4; ch++) {
    Serial.printf("  --- Channel %u ---\n", ch);
    st = device.readRegister16(LDC1614::cmd::regRcount(ch), regVal);
    if (st.ok()) Serial.printf("    RCOUNT: 0x%04X (%u)\n", regVal, regVal);
    st = device.readRegister16(LDC1614::cmd::regSettleCount(ch), regVal);
    if (st.ok()) Serial.printf("    SETTLECOUNT: 0x%04X (%u)\n", regVal, regVal);
    st = device.readRegister16(LDC1614::cmd::regClockDividers(ch), regVal);
    if (st.ok()) {
      const uint8_t finDiv = (regVal >> 12) & 0x0F;
      const uint16_t frefDiv = regVal & 0x03FF;
      Serial.printf("    CLOCK_DIV: 0x%04X (FIN=%u, FREF=%u)\n", regVal, finDiv, frefDiv);
    }
    st = device.readRegister16(LDC1614::cmd::regDriveCurrent(ch), regVal);
    if (st.ok()) {
      const uint8_t idrive = (regVal >> 11) & 0x1F;
      const uint8_t initIdrive = (regVal >> 6) & 0x1F;
      Serial.printf("    DRIVE_CURRENT: 0x%04X (IDRIVE=%u, INIT_IDRIVE=%u)\n",
                    regVal, idrive, initIdrive);
    }
    st = device.readRegister16(LDC1614::cmd::regOffset(ch), regVal);
    if (st.ok()) Serial.printf("    OFFSET: 0x%04X (%u)\n", regVal, regVal);
  }
}

void printIdentity() {
  uint16_t manufacturer = 0;
  uint16_t deviceId = 0;
  LDC1614::Status st = device.readRegister16(LDC1614::cmd::REG_MANUFACTURER_ID, manufacturer);
  if (!st.ok()) {
    printStatus(st);
    return;
  }
  st = device.readRegister16(LDC1614::cmd::REG_DEVICE_ID, deviceId);
  if (!st.ok()) {
    printStatus(st);
    return;
  }

  const bool manufacturerOk = manufacturer == LDC1614::cmd::MANUFACTURER_ID_VALUE;
  const bool deviceOk = deviceId == LDC1614::cmd::DEVICE_ID_VALUE;
  Serial.println("=== Device Identity ===");
  Serial.printf("  MANUFACTURER_ID: 0x%04X expected=0x%04X match=%s%s%s\n",
                manufacturer,
                LDC1614::cmd::MANUFACTURER_ID_VALUE,
                yesNoColor(manufacturerOk),
                manufacturerOk ? "YES" : "NO",
                LOG_COLOR_RESET);
  Serial.printf("  DEVICE_ID:       0x%04X expected=0x%04X match=%s%s%s\n",
                deviceId,
                LDC1614::cmd::DEVICE_ID_VALUE,
                yesNoColor(deviceOk),
                deviceOk ? "YES" : "NO",
                LOG_COLOR_RESET);
}

// ============================================================================
// Stress Mix
// ============================================================================

void runStressMix(int count) {
  struct OpStats {
    const char* name;
    uint32_t ok;
    uint32_t fail;
  };

  OpStats stats[] = {
      {"readChannel", 0, 0},
      {"readAllChans", 0, 0},
      {"readStatus", 0, 0},
      {"probe", 0, 0},
      {"sleep+wake", 0, 0},
      {"readRegister", 0, 0},
  };
  const int opCount = static_cast<int>(sizeof(stats) / sizeof(stats[0]));

  const uint32_t successBefore = device.totalSuccess();
  const uint32_t failBefore = device.totalFailures();
  const uint32_t startMs = millis();

  for (int i = 0; i < count; ++i) {
    LDC1614::Status st = LDC1614::Status::Ok();
    const int op = i % opCount;

    switch (op) {
      case 0: {
        LDC1614::ChannelData data;
        st = device.readChannel(0, data);
        break;
      }
      case 1: {
        LDC1614::ChannelData data[4];
        st = device.readAllChannels(data);
        break;
      }
      case 2: {
        LDC1614::DeviceStatus ds;
        st = device.readDeviceStatus(ds);
        break;
      }
      case 3: {
        st = device.probe();
        break;
      }
      case 4: {
        st = device.sleep();
        if (st.ok()) {
          st = device.wake();
        }
        break;
      }
      case 5: {
        uint16_t val = 0;
        st = device.readRegister16(LDC1614::cmd::REG_MANUFACTURER_ID, val);
        break;
      }
      default:
        break;
    }

    if (st.ok()) {
      stats[op].ok++;
    } else {
      stats[op].fail++;
      LOGV(verboseMode, "[%d] %s failed: %s", i, stats[op].name, errToStr(st.code));
    }

    if ((i + 1) % 50 == 0) {
      device.tick(millis());
    }
  }

  const uint32_t elapsed = millis() - startMs;
  uint32_t okTotal = 0;
  uint32_t failTotal = 0;
  for (int i = 0; i < opCount; ++i) {
    okTotal += stats[i].ok;
    failTotal += stats[i].fail;
  }

  Serial.println("=== stress_mix summary ===");
  const float successPct =
      (count > 0) ? (100.0f * static_cast<float>(okTotal) / static_cast<float>(count)) : 0.0f;
  Serial.printf("  Total: %sok=%lu%s %sfail=%lu%s (%s%.2f%%%s)\n",
                goodIfNonZeroColor(okTotal),
                static_cast<unsigned long>(okTotal),
                LOG_COLOR_RESET,
                goodIfZeroColor(failTotal),
                static_cast<unsigned long>(failTotal),
                LOG_COLOR_RESET,
                successRateColor(successPct),
                successPct,
                LOG_COLOR_RESET);
  Serial.printf("  Duration: %lu ms\n", static_cast<unsigned long>(elapsed));
  if (elapsed > 0) {
    Serial.printf("  Rate: %.2f ops/s\n", (1000.0f * static_cast<float>(count)) / elapsed);
  }
  for (int i = 0; i < opCount; ++i) {
    Serial.printf("  %-12s %sok=%lu%s %sfail=%lu%s\n",
                  stats[i].name,
                  goodIfNonZeroColor(stats[i].ok),
                  static_cast<unsigned long>(stats[i].ok),
                  LOG_COLOR_RESET,
                  goodIfZeroColor(stats[i].fail),
                  static_cast<unsigned long>(stats[i].fail),
                  LOG_COLOR_RESET);
  }
  const uint32_t successDelta = device.totalSuccess() - successBefore;
  const uint32_t failDelta = device.totalFailures() - failBefore;
  Serial.printf("  Health delta: %ssuccess +%lu%s, %sfailures +%lu%s\n",
                goodIfNonZeroColor(successDelta),
                static_cast<unsigned long>(successDelta),
                LOG_COLOR_RESET,
                goodIfZeroColor(failDelta),
                static_cast<unsigned long>(failDelta),
                LOG_COLOR_RESET);
}

// ============================================================================
// Command Processing
// ============================================================================

void processCommand(const String& cmdLine) {
  String cmd = cmdLine;
  cmd.trim();

  if (cmd.length() == 0) {
    return;
  }

  if (cmd == "help" || cmd == "?") {
    printHelp();
  } else if (cmd == "version" || cmd == "ver") {
    printVersionInfo();
  } else if (cmd == "scan") {
    bus_diag::scan();
  } else if (cmd == "probe") {
    LOGI("Probing device (no health tracking)...");
    auto st = device.probe();
    printStatus(st);
  } else if (cmd == "drv") {
    printDriverHealth();
  } else if (cmd == "recover") {
    LOGI("Attempting recovery...");
    auto st = device.recover();
    printStatus(st);
    printDriverHealth();
  } else if (cmd == "online") {
    const bool on = device.isOnline();
    Serial.printf("  Online: %s%s%s\n",
                  on ? LOG_COLOR_GREEN : LOG_COLOR_RED,
                  log_bool_str(on),
                  LOG_COLOR_RESET);
  } else if (cmd == "verbose") {
    LOGI("Verbose mode: %s%s%s", onOffColor(verboseMode), verboseMode ? "ON" : "OFF", LOG_COLOR_RESET);
  } else if (cmd.startsWith("verbose ")) {
    int val = cmd.substring(8).toInt();
    verboseMode = (val != 0);
    LOGI("Verbose mode: %s%s%s", onOffColor(verboseMode), verboseMode ? "ON" : "OFF", LOG_COLOR_RESET);
  } else if (cmd == "init" || cmd == "begin") {
    LOGI("Initializing LDC1614...");
    device.end();
    auto st = device.begin(makeDefaultConfig());
    printStatus(st);
    if (st.ok()) {
      LOGI("Device initialized in sleep mode. Use 'wake' to start conversions.");
      printDriverHealth();
    }
  } else if (cmd == "end") {
    LOGI("Shutting down driver...");
    device.end();
    LOGI("Driver state: UNINIT");
  } else if (cmd == "sleep") {
    auto st = device.sleep();
    printStatus(st);
  } else if (cmd == "wake") {
    auto st = device.wake();
    printStatus(st);
  } else if (cmd == "reset") {
    auto st = device.softReset();
    printStatus(st);
  } else if (cmd == "resetreapply") {
    LOGI("Soft reset + re-apply config...");
    auto st = device.resetAndReapply();
    printStatus(st);
    if (st.ok()) {
      LOGI("Device back in READY/sleep. Use 'wake' to start conversions.");
    }
  } else if (cmd == "status") {
    if (!device.isOnline()) {
      LOGW("Device not online.");
      return;
    }
    LDC1614::DeviceStatus ds;
    auto st = device.readDeviceStatus(ds);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    printDeviceStatus(ds);
  } else if (cmd == "status_raw") {
    if (!device.isOnline()) {
      LOGW("Device not online.");
      return;
    }
    uint16_t raw = 0;
    auto st = device.readStatusRaw(raw);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    Serial.printf("  STATUS raw = 0x%04X\n", raw);
  } else if (cmd.startsWith("freq ")) {
    // freq <ch> <fRef>
    String args = cmd.substring(5);
    args.trim();
    int split = args.indexOf(' ');
    if (split < 0) {
      LOGW("Usage: freq <ch> <fRef>");
      return;
    }
    int32_t ch = 0;
    if (!parseI32(args.substring(0, split), ch) || ch < 0 || ch > 3) {
      LOGW("Invalid channel (0-3)");
      return;
    }
    float fRef = args.substring(split + 1).toFloat();
    if (fRef <= 0.0f) {
      LOGW("Invalid fRef (must be > 0)");
      return;
    }
    if (!device.isOnline()) {
      LOGW("Device not online.");
      return;
    }
    LDC1614::ChannelData data;
    auto st = device.readChannel(static_cast<uint8_t>(ch), data);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    const float freq = device.calcSensorFrequency(static_cast<uint8_t>(ch), data.rawData, fRef);
    Serial.printf("  Ch%ld: raw=0x%07lX freq=%.2f Hz\n",
                  static_cast<long>(ch),
                  static_cast<unsigned long>(data.rawData),
                  freq);
  } else if (cmd.startsWith("timing ")) {
    // timing <ch> <fRef>
    String args = cmd.substring(7);
    args.trim();
    int split = args.indexOf(' ');
    if (split < 0) {
      LOGW("Usage: timing <ch> <fRef>");
      return;
    }
    int32_t ch = 0;
    if (!parseI32(args.substring(0, split), ch) || ch < 0 || ch > 3) {
      LOGW("Invalid channel (0-3)");
      return;
    }
    float fRef = args.substring(split + 1).toFloat();
    if (fRef <= 0.0f) {
      LOGW("Invalid fRef (must be > 0)");
      return;
    }
    const float convTimeUs = device.calcConversionTimeUs(static_cast<uint8_t>(ch), fRef);
    const float settleTimeUs = device.calcSettleTimeUs(static_cast<uint8_t>(ch), fRef);
    const float sampleTimeUs = device.calcSampleTimeUs(static_cast<uint8_t>(ch), fRef);
    Serial.printf("  Ch%ld: conversion time = %.2f us (%.3f ms)\n",
                  static_cast<long>(ch),
                  convTimeUs,
                  convTimeUs / 1000.0f);
    Serial.printf("  Ch%ld: settling time   = %.2f us (%.3f ms)\n",
                  static_cast<long>(ch),
                  settleTimeUs,
                  settleTimeUs / 1000.0f);
    Serial.printf("  Ch%ld: sample time     = %.2f us (%.3f ms)\n",
                  static_cast<long>(ch),
                  sampleTimeUs,
                  sampleTimeUs / 1000.0f);
  } else if (cmd == "channels") {
    Serial.printf("  Channel count: %u\n", device.channelCount());
  } else if (cmd == "drdy") {
    bool ready = false;
    auto st = device.readDataReady(ready);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    Serial.printf("  Data ready: %s%s%s\n",
                  yesNoColor(ready),
                  ready ? "YES" : "NO",
                  LOG_COLOR_RESET);
  } else if (cmd == "read") {
    if (!device.isOnline()) {
      LOGW("Device not online. Run 'init' first.");
      return;
    }
    LDC1614::ChannelData data[4];
    auto st = device.readAllChannels(data);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    for (uint8_t i = 0; i < device.channelCount(); i++) {
      Serial.printf("  Ch%u: raw=0x%07lX (%lu)", i,
                    static_cast<unsigned long>(data[i].rawData),
                    static_cast<unsigned long>(data[i].rawData));
      if (data[i].hasError()) {
        Serial.printf(" %s[ERR: UR=%d OR=%d WD=%d AE=%d]%s",
                      LOG_COLOR_RED,
                      data[i].errUnderRange, data[i].errOverRange,
                      data[i].errWatchdog, data[i].errAmplitude,
                      LOG_COLOR_RESET);
      }
      Serial.println();
    }
  } else if (cmd.startsWith("read ")) {
    if (!device.isOnline()) {
      LOGW("Device not online. Run 'init' first.");
      return;
    }
    int ch = cmd.substring(5).toInt();
    if (ch < 0 || ch > 3) {
      LOGW("Invalid channel (0-3)");
      return;
    }
    LDC1614::ChannelData data;
    auto st = device.readChannel(static_cast<uint8_t>(ch), data);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    Serial.printf("  Ch%d: raw=0x%07lX (%lu)", ch,
                  static_cast<unsigned long>(data.rawData),
                  static_cast<unsigned long>(data.rawData));
    if (data.hasError()) {
      Serial.printf(" %s[ERR: UR=%d OR=%d WD=%d AE=%d]%s",
                    LOG_COLOR_RED,
                    data.errUnderRange, data.errOverRange,
                    data.errWatchdog, data.errAmplitude,
                    LOG_COLOR_RESET);
    }
    Serial.println();
  } else if (cmd == "readblocking") {
    if (!device.isOnline()) {
      LOGW("Device not online. Run 'init' first.");
      return;
    }
    if (device.isSleeping()) {
      LOGW("Device is sleeping. Use 'wake' first.");
      return;
    }
    LDC1614::ChannelData data[4];
    auto st = device.readAllChannelsBlocking(data);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    for (uint8_t i = 0; i < device.channelCount(); i++) {
      Serial.printf("  Ch%u: raw=0x%07lX (%lu)", i,
                    static_cast<unsigned long>(data[i].rawData),
                    static_cast<unsigned long>(data[i].rawData));
      if (data[i].hasError()) {
        Serial.printf(" %s[ERR: UR=%d OR=%d WD=%d AE=%d]%s",
                      LOG_COLOR_RED,
                      data[i].errUnderRange, data[i].errOverRange,
                      data[i].errWatchdog, data[i].errAmplitude,
                      LOG_COLOR_RESET);
      }
      Serial.println();
    }
  } else if (cmd.startsWith("readblocking ")) {
    if (!device.isOnline()) {
      LOGW("Device not online. Run 'init' first.");
      return;
    }
    if (device.isSleeping()) {
      LOGW("Device is sleeping. Use 'wake' first.");
      return;
    }
    int ch = cmd.substring(13).toInt();
    if (ch < 0 || ch > 3) {
      LOGW("Invalid channel (0-3)");
      return;
    }
    LDC1614::ChannelData data;
    auto st = device.readChannelBlocking(static_cast<uint8_t>(ch), data);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    Serial.printf("  Ch%d: raw=0x%07lX (%lu)", ch,
                  static_cast<unsigned long>(data.rawData),
                  static_cast<unsigned long>(data.rawData));
    if (data.hasError()) {
      Serial.printf(" %s[ERR: UR=%d OR=%d WD=%d AE=%d]%s",
                    LOG_COLOR_RED,
                    data.errUnderRange, data.errOverRange,
                    data.errWatchdog, data.errAmplitude,
                    LOG_COLOR_RESET);
    }
    Serial.println();
  } else if (cmd.startsWith("sample ")) {
    int ch = cmd.substring(7).toInt();
    if (ch < 0 || ch > 3) {
      LOGW("Invalid channel (0-3)");
      return;
    }
    LDC1614::ChannelData data;
    auto st = device.getLastSample(static_cast<uint8_t>(ch), data);
    if (!st.ok()) {
      LOGW("No cached sample for Ch%d", ch);
      return;
    }
    const uint32_t ts = device.sampleTimestampMs(static_cast<uint8_t>(ch));
    const uint32_t age = device.sampleAgeMs(static_cast<uint8_t>(ch), millis());
    Serial.printf("  Ch%d: raw=0x%07lX (%lu) age=%lu ms (at %lu ms)\n",
                  ch,
                  static_cast<unsigned long>(data.rawData),
                  static_cast<unsigned long>(data.rawData),
                  static_cast<unsigned long>(age),
                  static_cast<unsigned long>(ts));
    if (data.hasError()) {
      Serial.printf("  %s[ERR: UR=%d OR=%d WD=%d AE=%d]%s\n",
                    LOG_COLOR_RED,
                    data.errUnderRange, data.errOverRange,
                    data.errWatchdog, data.errAmplitude,
                    LOG_COLOR_RESET);
    }
  } else if (cmd.startsWith("sampleage ")) {
    int ch = cmd.substring(10).toInt();
    if (ch < 0 || ch > 3) {
      LOGW("Invalid channel (0-3)");
      return;
    }
    const uint32_t ts = device.sampleTimestampMs(static_cast<uint8_t>(ch));
    if (ts == 0) {
      Serial.printf("  Ch%d: %snever read%s\n", ch, LOG_COLOR_YELLOW, LOG_COLOR_RESET);
    } else {
      const uint32_t age = device.sampleAgeMs(static_cast<uint8_t>(ch), millis());
      Serial.printf("  Ch%d: age=%lu ms (at %lu ms)\n", ch,
                    static_cast<unsigned long>(age),
                    static_cast<unsigned long>(ts));
    }
  } else if (cmd == "snapshot") {
    LDC1614::SettingsSnapshot snap;
    device.getSettings(snap);
    Serial.println("=== Settings Snapshot (no I2C) ===");
    Serial.printf("  State: %s%s%s\n",
                  stateColor(snap.state, snap.state == LDC1614::DriverState::READY ||
                             snap.state == LDC1614::DriverState::DEGRADED, 0),
                  stateToStr(snap.state),
                  LOG_COLOR_RESET);
    Serial.printf("  Sleeping: %s\n", log_bool_str(snap.sleeping));
    Serial.printf("  Measuring: %s\n", log_bool_str(device.isMeasuring()));
    Serial.printf("  Channels: %u  Active: %u  AutoScan: %s\n",
                  snap.channelCount, snap.activeChan,
                  log_bool_str(snap.autoScan));
    Serial.printf("  RR sequence: %s  Deglitch: %s\n",
                  rrSequenceToStr(snap.rrSequence),
                  deglitchToStr(snap.deglitch));
    Serial.printf("  RefClk: %s  Activation: %s  INTB: %s\n",
                  refClkToStr(snap.refClkSrc),
                  activationToStr(snap.sensorActivation),
                  log_bool_str(snap.intbEnabled));
    Serial.printf("  RPoverride: %s  AutoAmp: %s  HighCurrentDrv: %s\n",
                  log_bool_str(snap.rpOverrideEn),
                  log_bool_str(!snap.autoAmpDis),
                  log_bool_str(snap.highCurrentDrv));
    for (uint8_t i = 0; i < snap.channelCount && i < 4; i++) {
      const auto& cc = snap.channel[i];
      const uint32_t age = snap.sampleTimestampMs[i] > 0
                               ? (millis() - snap.sampleTimestampMs[i])
                               : 0;
      Serial.printf("  Ch%u: rcount=0x%04X settle=0x%04X fin=%u fref=%u "
                    "offset=0x%04X idrive=%u sample_age=%lu ms\n",
                    i, cc.rcount, cc.settleCount, cc.finDivider, cc.frefDivider,
                    cc.offset, cc.idrive,
                    static_cast<unsigned long>(age));
    }
  } else if (cmd.startsWith("rcount ")) {
    String args = cmd.substring(7);
    args.trim();
    int split = args.indexOf(' ');
    if (split < 0) {
      LOGW("Usage: rcount <ch> <val>");
      return;
    }
    int32_t ch = 0, val = 0;
    if (!parseI32(args.substring(0, split), ch) || !parseI32(args.substring(split + 1), val)) {
      LOGW("Usage: rcount <ch> <val>");
      return;
    }
    auto st = device.setRcount(static_cast<uint8_t>(ch), static_cast<uint16_t>(val));
    printStatus(st);
  } else if (cmd.startsWith("settle ")) {
    String args = cmd.substring(7);
    args.trim();
    int split = args.indexOf(' ');
    if (split < 0) {
      LOGW("Usage: settle <ch> <val>");
      return;
    }
    int32_t ch = 0, val = 0;
    if (!parseI32(args.substring(0, split), ch) || !parseI32(args.substring(split + 1), val)) {
      LOGW("Usage: settle <ch> <val>");
      return;
    }
    auto st = device.setSettleCount(static_cast<uint8_t>(ch), static_cast<uint16_t>(val));
    printStatus(st);
  } else if (cmd.startsWith("clkdiv ")) {
    String args = cmd.substring(7);
    args.trim();
    int sp1 = args.indexOf(' ');
    if (sp1 < 0) {
      LOGW("Usage: clkdiv <ch> <fin> <fref>");
      return;
    }
    String rest = args.substring(sp1 + 1);
    rest.trim();
    int sp2 = rest.indexOf(' ');
    if (sp2 < 0) {
      LOGW("Usage: clkdiv <ch> <fin> <fref>");
      return;
    }
    int32_t ch = 0, fin = 0, fref = 0;
    if (!parseI32(args.substring(0, sp1), ch) ||
        !parseI32(rest.substring(0, sp2), fin) ||
        !parseI32(rest.substring(sp2 + 1), fref)) {
      LOGW("Usage: clkdiv <ch> <fin> <fref>");
      return;
    }
    auto st = device.setClockDividers(static_cast<uint8_t>(ch),
                                       static_cast<uint8_t>(fin),
                                       static_cast<uint16_t>(fref));
    printStatus(st);
  } else if (cmd.startsWith("offset ")) {
    String args = cmd.substring(7);
    args.trim();
    int split = args.indexOf(' ');
    if (split < 0) {
      LOGW("Usage: offset <ch> <val>");
      return;
    }
    int32_t ch = 0, val = 0;
    if (!parseI32(args.substring(0, split), ch) || !parseI32(args.substring(split + 1), val)) {
      LOGW("Usage: offset <ch> <val>");
      return;
    }
    auto st = device.setOffset(static_cast<uint8_t>(ch), static_cast<uint16_t>(val));
    printStatus(st);
  } else if (cmd.startsWith("idrive ")) {
    String args = cmd.substring(7);
    args.trim();
    int split = args.indexOf(' ');
    if (split < 0) {
      LOGW("Usage: idrive <ch> <val>");
      return;
    }
    int32_t ch = 0, val = 0;
    if (!parseI32(args.substring(0, split), ch) || !parseI32(args.substring(split + 1), val)) {
      LOGW("Usage: idrive <ch> <val>");
      return;
    }
    auto st = device.setDriveCurrent(static_cast<uint8_t>(ch), static_cast<uint8_t>(val));
    printStatus(st);
  } else if (cmd.startsWith("initidrive ")) {
    int ch = cmd.substring(11).toInt();
    if (ch < 0 || ch > 3) {
      LOGW("Invalid channel (0-3)");
      return;
    }
    uint8_t initIdrive = 0;
    auto st = device.readInitIdrive(static_cast<uint8_t>(ch), initIdrive);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    Serial.printf("  Ch%d INIT_IDRIVE = %u\n", ch, initIdrive);
  } else if (cmd == "activech") {
    Serial.printf("  Active channel: %u\n", device.getActiveChannel());
  } else if (cmd.startsWith("activech ")) {
    int ch = cmd.substring(9).toInt();
    if (ch < 0 || ch > 3) {
      LOGW("Invalid channel (0-3)");
      return;
    }
    auto st = device.setActiveChannel(static_cast<uint8_t>(ch));
    printStatus(st);
  } else if (cmd.startsWith("single ")) {
    int ch = cmd.substring(7).toInt();
    if (ch < 0 || ch > 3) {
      LOGW("Invalid channel (0-3)");
      return;
    }
    auto st = device.setSingleChannelMode(static_cast<uint8_t>(ch));
    printStatus(st);
  } else if (cmd.startsWith("autoscan ")) {
    LDC1614::RRSequence sequence = LDC1614::RRSequence::CH0_CH1;
    if (!parseRRSequence(cmd.substring(9), sequence)) {
      LOGW("Usage: autoscan <2|3|4>");
      return;
    }
    auto st = device.setAutoScanMode(sequence);
    printStatus(st);
  } else if (cmd.startsWith("deglitch ")) {
    LDC1614::Deglitch deglitch = LDC1614::Deglitch::BW_33MHZ;
    if (!parseDeglitch(cmd.substring(9), deglitch)) {
      LOGW("Usage: deglitch <1|3|10|33>");
      return;
    }
    auto st = device.setDeglitch(deglitch);
    printStatus(st);
  } else if (cmd == "errcfg") {
    Serial.printf("  Cached ERROR_CONFIG: 0x%04X\n", device.getErrorConfig());
    if (device.isOnline()) {
      uint16_t value = 0;
      auto st = device.readRegister16(LDC1614::cmd::REG_ERROR_CONFIG, value);
      if (st.ok()) {
        Serial.printf("  Live ERROR_CONFIG:   0x%04X\n", value);
      } else {
        printStatus(st);
      }
    }
  } else if (cmd.startsWith("errcfg ")) {
    uint32_t value = 0;
    if (!parseU32(cmd.substring(7), value) || value > 0xFFFFU) {
      LOGW("Usage: errcfg <mask>");
      return;
    }
    auto st = device.setErrorConfig(static_cast<uint16_t>(value));
    printStatus(st);
  } else if (cmd == "intb") {
    const LDC1614::Config& cfg = device.getConfig();
    Serial.printf("  INTB pin: %d  output: %s%s%s\n",
                  cfg.intbPin,
                  cfg.intbPin >= 0 && !cfg.intbDisable ? LOG_COLOR_GREEN : LOG_COLOR_YELLOW,
                  log_bool_str(cfg.intbPin >= 0 && !cfg.intbDisable),
                  LOG_COLOR_RESET);
  } else if (cmd.startsWith("intb ")) {
    bool enabled = false;
    if (!parseBoolToken(cmd.substring(5), enabled)) {
      LOGW("Usage: intb <0|1>");
      return;
    }
    auto st = device.setIntbDisabled(!enabled);
    printStatus(st);
  } else if (cmd.startsWith("refclk ")) {
    LDC1614::RefClkSrc source = LDC1614::RefClkSrc::INTERNAL;
    if (!parseRefClk(cmd.substring(7), source)) {
      LOGW("Usage: refclk <int|ext>");
      return;
    }
    auto st = device.setReferenceClockSource(source);
    printStatus(st);
  } else if (cmd.startsWith("activate ")) {
    LDC1614::SensorActivation activation = LDC1614::SensorActivation::FULL_CURRENT;
    if (!parseActivation(cmd.substring(9), activation)) {
      LOGW("Usage: activate <full|low>");
      return;
    }
    auto st = device.setSensorActivation(activation);
    printStatus(st);
  } else if (cmd.startsWith("rpoverride ")) {
    bool enabled = false;
    if (!parseBoolToken(cmd.substring(11), enabled)) {
      LOGW("Usage: rpoverride <0|1>");
      return;
    }
    auto st = device.setRpOverrideEnabled(enabled);
    printStatus(st);
  } else if (cmd.startsWith("autoamp ")) {
    bool enabled = false;
    if (!parseBoolToken(cmd.substring(8), enabled)) {
      LOGW("Usage: autoamp <0|1>");
      return;
    }
    auto st = device.setAutoAmplitudeCorrectionEnabled(enabled);
    printStatus(st);
  } else if (cmd.startsWith("highcurrent ")) {
    bool enabled = false;
    if (!parseBoolToken(cmd.substring(12), enabled)) {
      LOGW("Usage: highcurrent <0|1>");
      return;
    }
    auto st = device.setHighCurrentDriveEnabled(enabled);
    printStatus(st);
  } else if (cmd.startsWith("wreg ")) {
    if (!device.isOnline()) {
      LOGW("Device not online.");
      return;
    }
    String args = cmd.substring(5);
    args.trim();
    int split = args.indexOf(' ');
    if (split < 0) {
      LOGW("Usage: wreg <addr> <val>");
      return;
    }
    uint32_t addr = 0, val = 0;
    if (!parseU32(args.substring(0, split), addr) ||
        !parseU32(args.substring(split + 1), val) ||
        addr > 0xFFu || val > 0xFFFFu) {
      LOGW("Usage: wreg <addr> <val>");
      return;
    }
    auto st = device.writeRegister16(static_cast<uint8_t>(addr), static_cast<uint16_t>(val));
    printStatus(st);
  } else if (cmd.startsWith("reg ")) {
    if (!device.isOnline()) {
      LOGW("Device not online.");
      return;
    }
    uint32_t addr = 0;
    if (!parseU32(cmd.substring(4), addr) || addr > 0xFFu) {
      LOGW("Usage: reg <addr>");
      return;
    }
    uint16_t val = 0;
    auto st = device.readRegister16(static_cast<uint8_t>(addr), val);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    Serial.printf("  Reg 0x%02lX = 0x%04X (%u)\n",
                  static_cast<unsigned long>(addr), val, val);
  } else if (cmd == "selftest") {
    runSelfTest();
  } else if (cmd == "cfg" || cmd == "settings") {
    if (!device.isOnline()) {
      LOGW("Device not online.");
      return;
    }
    printConfig();
  } else if (cmd == "id") {
    printIdentity();
  } else if (cmd == "stress_mix") {
    if (!device.isOnline()) {
      LOGW("Device not online. Run 'init' and 'wake' first.");
      return;
    }
    runStressMix(50);
  } else if (cmd.startsWith("stress_mix ")) {
    if (!device.isOnline()) {
      LOGW("Device not online. Run 'init' and 'wake' first.");
      return;
    }
    int count = cmd.substring(11).toInt();
    if (count <= 0 || count > 100000) {
      LOGW("Invalid count (1-100000)");
      return;
    }
    runStressMix(count);
  } else if (cmd.startsWith("stress")) {
    int count = 10;
    if (cmd.length() > 6) {
      count = cmd.substring(7).toInt();
    }
    if (count <= 0 || count > 100000) {
      LOGW("Invalid count (1-100000)");
      return;
    }
    if (!device.isOnline()) {
      LOGW("Device not online. Run 'init' and 'wake' first.");
      return;
    }
    int ok = 0;
    int fail = 0;
    bool hasFailure = false;
    LDC1614::Status firstFailure = LDC1614::Status::Ok();
    LDC1614::Status lastFailure = LDC1614::Status::Ok();
    for (int i = 0; i < count; ++i) {
      LDC1614::ChannelData data;
      auto st = device.readChannel(0, data);
      if (st.ok()) {
        ok++;
        LOGV(verboseMode, "  %d: raw=0x%07lX", i + 1,
             static_cast<unsigned long>(data.rawData));
      } else {
        fail++;
        if (!hasFailure) {
          firstFailure = st;
          hasFailure = true;
        }
        lastFailure = st;
        if (verboseMode) {
          printStatus(st);
        }
      }
    }
    const float pct = (count > 0) ? (100.0f * static_cast<float>(ok) / static_cast<float>(count)) : 0.0f;
    Serial.printf("  Stress results: %s%d ok%s, %s%d failed%s (%s%.2f%%%s)\n",
                  goodIfNonZeroColor(static_cast<uint32_t>(ok)),
                  ok,
                  LOG_COLOR_RESET,
                  goodIfZeroColor(static_cast<uint32_t>(fail)),
                  fail,
                  LOG_COLOR_RESET,
                  successRateColor(pct),
                  pct,
                  LOG_COLOR_RESET);
    if (hasFailure) {
      Serial.println("  Failure details:");
      Serial.println("  First failure:");
      printStatus(firstFailure);
      if (fail > 1) {
        Serial.println("  Last failure:");
        printStatus(lastFailure);
      }
    }
  } else {
    LOGW("Unknown command: %s", cmd.c_str());
  }
}

// ============================================================================
// Setup and Loop
// ============================================================================

void setup() {
  board::initSerial();
  delay(100);

  LOGI("=== LDC1614 Bringup Example ===");

  if (!board::initI2c()) {
    LOGE("Failed to initialize I2C");
    return;
  }
  LOGI("I2C initialized (SDA=%d, SCL=%d)", board::I2C_SDA, board::I2C_SCL);

  board::initIntbPin();

  bus_diag::scan();

  auto st = device.begin(makeDefaultConfig());
  if (!st.ok()) {
    LOGE("Failed to initialize device");
    printStatus(st);
    LOGI("Type 'begin' or 'init' to retry initialization");
  } else {
    LOGI("Device initialized successfully");
    printDriverHealth();
  }

  Serial.println("\nType 'help' for commands");
  cli::printPrompt();
}

void loop() {
  device.tick(millis());

  static String inputBuffer;
  static constexpr size_t kMaxInputLen = 128;
  while (Serial.available()) {
    char c = static_cast<char>(Serial.read());
    if (c == '\n' || c == '\r') {
      if (inputBuffer.length() > 0) {
        processCommand(inputBuffer);
        inputBuffer = "";
        cli::printPrompt();
      }
    } else if (inputBuffer.length() < kMaxInputLen) {
      inputBuffer += c;
    }
  }
}
