#include "Ldc1614Cli.h"

#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>

#ifndef LDC1614_CLI_LOG_LEVEL
#ifdef LOG_LEVEL
#define LDC1614_CLI_LOG_LEVEL LOG_LEVEL
#else
#define LDC1614_CLI_LOG_LEVEL 2
#endif
#endif

namespace ldc1614_cli {

namespace {

static constexpr const char* COLOR_RESET = "\033[0m";
static constexpr const char* COLOR_RED = "\033[31m";
static constexpr const char* COLOR_GREEN = "\033[32m";
static constexpr const char* COLOR_YELLOW = "\033[33m";
static constexpr const char* COLOR_BLUE = "\033[34m";
static constexpr const char* COLOR_CYAN = "\033[36m";
static constexpr const char* COLOR_GRAY = "\033[90m";
static constexpr size_t HELP_COMMAND_WIDTH = 32U;
static constexpr int STRESS_DEFAULT_COUNT = 10;
static constexpr int STRESS_MIX_DEFAULT_COUNT = 50;
static constexpr int DEMO_DEFAULT_COUNT = 5;
static constexpr int MAX_STRESS_COUNT = 100000;
static constexpr int MAX_DEMO_COUNT = 1000;
static constexpr uint32_t MAX_SAMPLE_RATE_COUNT = 5000;
static constexpr uint32_t DEFAULT_SAMPLE_RATE_TIMEOUT_MS = 200;
static constexpr uint32_t MAX_SAMPLE_RATE_TIMEOUT_MS = 1000;
static constexpr uint32_t MAX_STAGED_POLLS = 1000;
static constexpr uint8_t MAX_STAGED_INSTRUCTIONS = 32;

const char* resultColor(bool ok) {
  return ok ? COLOR_GREEN : COLOR_RED;
}

const char* stateBaseColor(bool online, uint8_t failures) {
  if (!online) {
    return COLOR_RED;
  }
  return failures > 0U ? COLOR_YELLOW : COLOR_GREEN;
}

const char* boolStr(bool value) {
  return value ? "yes" : "no";
}

const char* goodIfZeroColor(uint32_t value) {
  return value == 0U ? COLOR_GREEN : COLOR_RED;
}

const char* goodIfNonZeroColor(uint32_t value) {
  return value > 0U ? COLOR_GREEN : COLOR_YELLOW;
}

const char* onOffColor(bool enabled) {
  return enabled ? COLOR_GREEN : COLOR_RESET;
}

const char* yesNoColor(bool value) {
  return value ? COLOR_GREEN : COLOR_YELLOW;
}

const char* skipCountColor(uint32_t value) {
  return value > 0U ? COLOR_YELLOW : COLOR_RESET;
}

const char* successRateColor(float pct) {
  if (pct >= 99.9f) {
    return COLOR_GREEN;
  }
  if (pct >= 80.0f) {
    return COLOR_YELLOW;
  }
  return COLOR_RED;
}

const char* staleTimeColor(bool isErrorTimestamp) {
  return isErrorTimestamp ? COLOR_GREEN : COLOR_YELLOW;
}

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
    return COLOR_YELLOW;
  }
  return stateBaseColor(online, consecutiveFailures);
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

bool isSpace(char c) {
  return std::isspace(static_cast<unsigned char>(c)) != 0;
}

class Line {
public:
  Line() = default;
  explicit Line(const char* text) : _value(text == nullptr ? "" : text) {}
  explicit Line(std::string text) : _value(std::move(text)) {}

  const char* c_str() const { return _value.c_str(); }
  size_t length() const { return _value.length(); }
  bool empty() const { return _value.empty(); }

  void trim() {
    const auto first = std::find_if_not(_value.begin(), _value.end(), isSpace);
    if (first == _value.end()) {
      _value.clear();
      return;
    }
    const auto last = std::find_if_not(_value.rbegin(), _value.rend(), isSpace).base();
    _value.assign(first, last);
  }

  void toLowerCase() {
    for (char& c : _value) {
      c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
  }

  bool startsWith(const char* prefix) const {
    if (prefix == nullptr) {
      return false;
    }
    const size_t len = std::strlen(prefix);
    return _value.size() >= len && _value.compare(0, len, prefix) == 0;
  }

  Line substring(size_t start) const {
    if (start >= _value.size()) {
      return Line("");
    }
    return Line(_value.substr(start));
  }

  Line substring(size_t start, size_t end) const {
    if (start >= _value.size() || end <= start) {
      return Line("");
    }
    return Line(_value.substr(start, end - start));
  }

  int indexOf(char needle) const {
    const size_t pos = _value.find(needle);
    if (pos == std::string::npos) {
      return -1;
    }
    return static_cast<int>(pos);
  }

  int toInt() const {
    return static_cast<int>(std::strtol(_value.c_str(), nullptr, 10));
  }

  float toFloat() const {
    return std::strtof(_value.c_str(), nullptr);
  }

  bool operator==(const char* rhs) const {
    return rhs != nullptr && _value == rhs;
  }

  bool operator!=(const char* rhs) const {
    return !(*this == rhs);
  }

private:
  std::string _value;
};

bool parseI32(const Line& token, int32_t& out) {
  char* end = nullptr;
  const long value = std::strtol(token.c_str(), &end, 0);
  if (end == token.c_str() || *end != '\0') {
    return false;
  }
  out = static_cast<int32_t>(value);
  return true;
}

bool parseU32(const Line& token, uint32_t& out) {
  char* end = nullptr;
  const unsigned long value = std::strtoul(token.c_str(), &end, 0);
  if (end == token.c_str() || *end != '\0') {
    return false;
  }
  out = static_cast<uint32_t>(value);
  return true;
}

bool parseU32Flexible(Line token, uint32_t& out) {
  token.trim();
  if (parseU32(token, out)) {
    return true;
  }

  const char* text = token.c_str();
  if (text[0] == '\0') {
    return false;
  }
  for (const char* p = text; *p != '\0'; ++p) {
    if (std::isxdigit(static_cast<unsigned char>(*p)) == 0) {
      return false;
    }
  }

  char* end = nullptr;
  const unsigned long value = std::strtoul(text, &end, 16);
  if (end == text || *end != '\0') {
    return false;
  }
  out = static_cast<uint32_t>(value);
  return true;
}

bool parseU32FlexibleList(const Line& args, uint32_t* out, size_t maxCount, size_t& count) {
  count = 0;
  if (out == nullptr || maxCount == 0U) {
    return false;
  }

  const char* p = args.c_str();
  while (*p != '\0') {
    while (std::isspace(static_cast<unsigned char>(*p)) != 0) {
      ++p;
    }
    if (*p == '\0') {
      break;
    }
    if (count >= maxCount) {
      return false;
    }

    const char* start = p;
    while (*p != '\0' && std::isspace(static_cast<unsigned char>(*p)) == 0) {
      ++p;
    }
    uint32_t value = 0;
    if (!parseU32Flexible(Line(std::string(start, static_cast<size_t>(p - start))), value)) {
      return false;
    }
    out[count++] = value;
  }

  return count > 0U;
}

bool parseBoolToken(Line token, bool& out) {
  token.trim();
  token.toLowerCase();
  if (token == "1" || token == "on" || token == "true" || token == "yes" ||
      token == "enable") {
    out = true;
    return true;
  }
  if (token == "0" || token == "off" || token == "false" || token == "no" ||
      token == "disable") {
    out = false;
    return true;
  }
  return false;
}

bool parseRRSequence(const Line& token, LDC1614::RRSequence& out) {
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

bool parseDeglitch(Line token, LDC1614::Deglitch& out) {
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

bool parseRefClk(Line token, LDC1614::RefClkSrc& out) {
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

bool parseActivation(Line token, LDC1614::SensorActivation& out) {
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

bool isLdcAddress(uint8_t addr) {
  return addr == 0x2AU || addr == 0x2BU;
}

}  // namespace

Cli::Cli(LDC1614::LDC1614& device, Platform platform)
    : _device(device), _platform(platform) {}

LDC1614::Config Cli::makeDefaultConfig() const {
  if (_platform.makeConfig == nullptr) {
    return LDC1614::Config{};
  }
  return _platform.makeConfig(_platform.user);
}

void Cli::vprintfToOutput(const char* fmt, va_list args) const {
  if (_platform.vprintf == nullptr || fmt == nullptr) {
    return;
  }
  _platform.vprintf(_platform.user, fmt, args);
}

void Cli::printf(const char* fmt, ...) const {
  va_list args;
  va_start(args, fmt);
  vprintfToOutput(fmt, args);
  va_end(args);
}

void Cli::println(const char* text) const {
  if (text == nullptr || text[0] == '\0') {
    printf("\n");
    return;
  }
  printf("%s\n", text);
}

void Cli::vlog(uint8_t minLevel, const char* color, const char* tag,
               const char* fmt, va_list args) const {
  if (LDC1614_CLI_LOG_LEVEL < minLevel) {
    return;
  }
  printf("%s[%s]%s ", color, tag, COLOR_RESET);
  vprintfToOutput(fmt, args);
  printf("\n");
}

void Cli::logError(const char* fmt, ...) const {
  va_list args;
  va_start(args, fmt);
  vlog(1, COLOR_RED, "E", fmt, args);
  va_end(args);
}

void Cli::logWarn(const char* fmt, ...) const {
  va_list args;
  va_start(args, fmt);
  vlog(2, COLOR_YELLOW, "W", fmt, args);
  va_end(args);
}

void Cli::logInfo(const char* fmt, ...) const {
  va_list args;
  va_start(args, fmt);
  vlog(2, COLOR_CYAN, "I", fmt, args);
  va_end(args);
}

void Cli::logVerbose(const char* fmt, ...) const {
  if (!_verboseMode) {
    return;
  }
  va_list args;
  va_start(args, fmt);
  printf("%s[V]%s ", COLOR_GRAY, COLOR_RESET);
  vprintfToOutput(fmt, args);
  printf("\n");
  va_end(args);
}

uint32_t Cli::nowMs() const {
  if (_platform.nowMs == nullptr) {
    return 0;
  }
  return _platform.nowMs(_platform.user);
}

void Cli::delayMs(uint32_t ms) const {
  if (_platform.delayMs != nullptr) {
    _platform.delayMs(ms, _platform.user);
  }
}

void Cli::yield() const {
  if (_platform.yield != nullptr) {
    _platform.yield(_platform.user);
  }
}

void Cli::printPrompt() const {
  printf("> ");
}

void Cli::printStatus(const LDC1614::Status& st) const {
  printf("  Status: %s%s%s (code=%u, detail=%ld)\n",
         resultColor(st.ok()),
         errToStr(st.code),
         COLOR_RESET,
         static_cast<unsigned>(st.code),
         static_cast<long>(st.detail));
  if (st.msg != nullptr && st.msg[0] != '\0') {
    printf("  Message: %s%s%s\n", COLOR_YELLOW, st.msg, COLOR_RESET);
  }
}

void Cli::printDriverHealth() const {
  const uint32_t now = nowMs();
  const uint32_t totalOk = _device.totalSuccess();
  const uint32_t totalFail = _device.totalFailures();
  const uint64_t total = static_cast<uint64_t>(totalOk) + totalFail;
  const float successRate = (total > 0U)
                                ? (100.0f * static_cast<float>(totalOk) /
                                   static_cast<float>(total))
                                : 0.0f;
  const LDC1614::Status lastErr = _device.lastError();
  const LDC1614::DriverState st = _device.state();
  const bool online = _device.isOnline();

  println("=== Driver Health ===");
  printf("  State: %s%s%s\n",
         stateColor(st, online, _device.consecutiveFailures()),
         stateToStr(st),
         COLOR_RESET);
  printf("  Online: %s%s%s\n",
         online ? COLOR_GREEN : COLOR_RED,
         boolStr(online),
         COLOR_RESET);
  printf("  Sleeping: %s%s%s\n",
         _device.isSleeping() ? COLOR_YELLOW : COLOR_GREEN,
         boolStr(_device.isSleeping()),
         COLOR_RESET);
  printf("  Hardware config dirty: %s%s%s\n",
         _device.hardwareConfigDirty() ? COLOR_RED : COLOR_GREEN,
         boolStr(_device.hardwareConfigDirty()),
         COLOR_RESET);
  if (_device.hardwareConfigDirty()) {
    printStatus(_device.hardwareConfigDirtyError());
  }
  printf("  Consecutive failures: %s%u%s\n",
         goodIfZeroColor(_device.consecutiveFailures()),
         _device.consecutiveFailures(),
         COLOR_RESET);
  printf("  Total success: %s%lu%s\n",
         goodIfNonZeroColor(totalOk),
         static_cast<unsigned long>(totalOk),
         COLOR_RESET);
  printf("  Total failures: %s%lu%s\n",
         goodIfZeroColor(totalFail),
         static_cast<unsigned long>(totalFail),
         COLOR_RESET);
  printf("  Success rate: %s%.1f%%%s\n",
         successRateColor(successRate),
         successRate,
         COLOR_RESET);

  const uint32_t lastOkMs = _device.lastOkMs();
  if (lastOkMs > 0U) {
    printf("  Last OK: %s%lu ms ago (at %lu ms)%s\n",
           COLOR_GREEN,
           static_cast<unsigned long>(now - lastOkMs),
           static_cast<unsigned long>(lastOkMs),
           COLOR_RESET);
  } else {
    printf("  Last OK: %snever%s\n", staleTimeColor(false), COLOR_RESET);
  }

  const uint32_t lastErrorMs = _device.lastErrorMs();
  if (lastErrorMs > 0U) {
    printf("  Last error: %s%lu ms ago (at %lu ms)%s\n",
           COLOR_RED,
           static_cast<unsigned long>(now - lastErrorMs),
           static_cast<unsigned long>(lastErrorMs),
           COLOR_RESET);
  } else {
    printf("  Last error: %snever%s\n", staleTimeColor(true), COLOR_RESET);
  }

  if (!lastErr.ok()) {
    printf("  Error code: %s%s%s\n", COLOR_RED, errToStr(lastErr.code), COLOR_RESET);
    printf("  Error detail: %ld\n", static_cast<long>(lastErr.detail));
    if (lastErr.msg != nullptr && lastErr.msg[0] != '\0') {
      printf("  Error msg: %s%s%s\n", COLOR_YELLOW, lastErr.msg, COLOR_RESET);
    }
  }
}

HealthSnapshot Cli::captureHealth() const {
  HealthSnapshot snap;
  snap.state = _device.state();
  snap.online = _device.isOnline();
  snap.consecutiveFailures = _device.consecutiveFailures();
  snap.totalFailures = _device.totalFailures();
  snap.totalSuccess = _device.totalSuccess();
  return snap;
}

void Cli::printHealthCompact() const {
  const HealthSnapshot snap = captureHealth();
  const uint32_t total = snap.totalSuccess + snap.totalFailures;
  const float pct = (total > 0U)
                        ? (100.0f * static_cast<float>(snap.totalSuccess) /
                           static_cast<float>(total))
                        : 0.0f;

  printf("Health: state=%s%s%s online=%s%s%s consec=%s%u%s ok=%s%lu%s "
         "fail=%s%lu%s rate=%s%.1f%%%s\n",
         stateColor(snap.state, snap.online, snap.consecutiveFailures),
         stateToStr(snap.state),
         COLOR_RESET,
         snap.online ? COLOR_GREEN : COLOR_RED,
         snap.online ? "true" : "false",
         COLOR_RESET,
         goodIfZeroColor(snap.consecutiveFailures),
         snap.consecutiveFailures,
         COLOR_RESET,
         goodIfNonZeroColor(snap.totalSuccess),
         static_cast<unsigned long>(snap.totalSuccess),
         COLOR_RESET,
         goodIfZeroColor(snap.totalFailures),
         static_cast<unsigned long>(snap.totalFailures),
         COLOR_RESET,
         successRateColor(pct),
         pct,
         COLOR_RESET);
}

void Cli::printHealthDiff(const HealthSnapshot& before, const HealthSnapshot& after) const {
  bool changed = false;

  if (before.state != after.state) {
    printf("  State: %s%s%s -> %s%s%s\n",
           stateColor(before.state, before.online, before.consecutiveFailures),
           stateToStr(before.state),
           COLOR_RESET,
           stateColor(after.state, after.online, after.consecutiveFailures),
           stateToStr(after.state),
           COLOR_RESET);
    changed = true;
  }
  if (before.online != after.online) {
    printf("  Online: %s%s%s -> %s%s%s\n",
           before.online ? COLOR_GREEN : COLOR_RED,
           before.online ? "true" : "false",
           COLOR_RESET,
           after.online ? COLOR_GREEN : COLOR_RED,
           after.online ? "true" : "false",
           COLOR_RESET);
    changed = true;
  }
  if (before.consecutiveFailures != after.consecutiveFailures) {
    printf("  ConsecFail: %s%u -> %u%s\n",
           goodIfZeroColor(after.consecutiveFailures),
           before.consecutiveFailures,
           after.consecutiveFailures,
           COLOR_RESET);
    changed = true;
  }
  if (before.totalSuccess != after.totalSuccess) {
    printf("  TotalOK: %lu -> %s%lu (+%lu)%s\n",
           static_cast<unsigned long>(before.totalSuccess),
           COLOR_GREEN,
           static_cast<unsigned long>(after.totalSuccess),
           static_cast<unsigned long>(after.totalSuccess - before.totalSuccess),
           COLOR_RESET);
    changed = true;
  }
  if (before.totalFailures != after.totalFailures) {
    printf("  TotalFail: %lu -> %s%lu (+%lu)%s\n",
           static_cast<unsigned long>(before.totalFailures),
           COLOR_RED,
           static_cast<unsigned long>(after.totalFailures),
           static_cast<unsigned long>(after.totalFailures - before.totalFailures),
           COLOR_RESET);
    changed = true;
  }

  if (!changed) {
    println("  (no health changes)");
  }
}

void Cli::printHelp() const {
  println();
  printf("%s=== LDC1614 CLI Help ===%s\n", COLOR_CYAN, COLOR_RESET);
  printf("\n%s[Common]%s\n", COLOR_GREEN, COLOR_RESET);
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "help / ?", COLOR_RESET, "Show this help");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "version / ver", COLOR_RESET, "Print firmware and library version info");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "scan", COLOR_RESET, "Scan I2C bus");

  printf("\n%s[Data]%s\n", COLOR_GREEN, COLOR_RESET);
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "read", COLOR_RESET, "Read configured channels");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "read <ch>", COLOR_RESET, "Read specific channel (0-3)");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "read <ch> [N]", COLOR_RESET, "Read channel N times");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "readfresh [count]", COLOR_RESET, "Read STATUS-driven fresh channel data");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "readstaged <mask> [polls] [instr]", COLOR_RESET, "Poll-budgeted DATAx read");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "samplerate <ch> <N> [timeoutMs]", COLOR_RESET, "DRDY-gated sample-rate smoke");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "readblocking", COLOR_RESET, "Blocking read configured channels (waits for DRDY)");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "readblocking <ch>", COLOR_RESET, "Blocking read specific channel");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "sample <ch>", COLOR_RESET, "Get last cached sample (no I2C)");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "sampleage <ch>", COLOR_RESET, "Show age of cached sample (ms)");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "drdy", COLOR_RESET, "Check data ready");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "status", COLOR_RESET, "Read and parse STATUS register");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "status_raw", COLOR_RESET, "Read raw STATUS register value");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "freq <ch> <fRef>", COLOR_RESET, "Read channel and calc sensor frequency");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "timing <ch> <fRef>", COLOR_RESET, "Calc conversion, settling, and sample time");

  printf("\n%s[Control]%s\n", COLOR_GREEN, COLOR_RESET);
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "init / begin", COLOR_RESET, "Initialize/reinitialize device");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "end", COLOR_RESET, "Shut down driver (returns to UNINIT)");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "sleep", COLOR_RESET, "Enter sleep mode (stop conversions)");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "wake", COLOR_RESET, "Wake and start conversions");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "reset", COLOR_RESET, "Software reset (returns to UNINIT)");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "resetreapply", COLOR_RESET, "Soft reset + re-apply config (stays READY)");

  printf("\n%s[Configuration]%s\n", COLOR_GREEN, COLOR_RESET);
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "cfg / config / settings", COLOR_RESET, "Print active configuration snapshot");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "snapshot", COLOR_RESET, "Print settings snapshot struct (no I2C)");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "channels", COLOR_RESET, "Show configured channel count");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "activech", COLOR_RESET, "Show current active channel");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "activech <ch>", COLOR_RESET, "Set active channel (single-ch mode)");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "single <ch>", COLOR_RESET, "Set single-channel mode and active channel");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "autoscan <2|3|4>", COLOR_RESET, "Set auto-scan sequence length");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "deglitch <1|3|10|33>", COLOR_RESET, "Set input deglitch bandwidth in MHz");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "errcfg [mask]", COLOR_RESET, "Show or set ERROR_CONFIG bit mask");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "intb [0|1]", COLOR_RESET, "Show or enable/disable INTB output");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "refclk <int|ext>", COLOR_RESET, "Set reference clock source");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "activate <full|low>", COLOR_RESET, "Set sensor activation current policy");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "rpoverride <0|1>", COLOR_RESET, "Enable/disable fixed RP override drive");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "autoamp <0|1>", COLOR_RESET, "Enable/disable auto amplitude correction");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "highcurrent <0|1>", COLOR_RESET, "Enable/disable high-current Ch0 drive");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "rcount <ch> <val>", COLOR_RESET, "Set RCOUNT for channel");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "settle <ch> <val>", COLOR_RESET, "Set SETTLECOUNT for channel");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "clkdiv <ch> <fin> <fref>", COLOR_RESET, "Set clock dividers");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "offset <ch> <val>", COLOR_RESET, "Set conversion offset");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "idrive <ch> <val>", COLOR_RESET, "Set drive current (0-31)");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "initidrive <ch>", COLOR_RESET, "Read auto-calibrated INIT_IDRIVE");

  printf("\n%s[Registers]%s\n", COLOR_GREEN, COLOR_RESET);
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "reg <addr>", COLOR_RESET, "Read register (hex address)");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "wreg <addr> <val>", COLOR_RESET,
         "Write register (diagnostic only; may desync cached config)");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "rawreg <reg> [addr]", COLOR_RESET, "Raw register read before begin");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "rawwreg <reg> <val> [addr]", COLOR_RESET, "Raw register write before begin");

  printf("\n%s[Diagnostics]%s\n", COLOR_GREEN, COLOR_RESET);
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "drv", COLOR_RESET, "Show driver state and health");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "state", COLOR_RESET, "Compact driver health summary");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "online", COLOR_RESET, "Check if device is online");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "id", COLOR_RESET, "Read MANUFACTURER_ID and DEVICE_ID");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "bus", COLOR_RESET, "I2C scan plus raw LDC identity checks");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "probeaddr <addr>", COLOR_RESET, "Raw LDC identity check at address");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "probe", COLOR_RESET, "Probe device (no health tracking)");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "recover", COLOR_RESET, "Manual recovery attempt");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "sync", COLOR_RESET, "Re-apply cached config and clear dirty state");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "verbose [0|1]", COLOR_RESET, "Enable/disable verbose output");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "stress [N]", COLOR_RESET, "Run N read cycles (default 10)");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "stress_mix [N]", COLOR_RESET, "Run N mixed-operation stress cycles");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "demo [N]", COLOR_RESET, "Run N sample demo workflow (default 5)");
  printf("  %s%-*s%s - %s\n", COLOR_CYAN, static_cast<int>(HELP_COMMAND_WIDTH),
         "selftest", COLOR_RESET, "Run safe command self-test report");
}

void Cli::printVersionInfo() const {
  println("=== Version Info ===");
  printf("  Example firmware build: not embedded; use HIL transcript timestamp\n");
  printf("  LDC1614 library version: %s\n", LDC1614::VERSION);
  printf("  LDC1614 full version: %s\n", LDC1614::VERSION_FULL);
  printf("  LDC1614 build timestamp: %s\n", LDC1614::BUILD_TIMESTAMP);
  printf("  LDC1614 git commit: %s\n", LDC1614::GIT_COMMIT);
  printf("  LDC1614 git status: %s\n", LDC1614::GIT_STATUS);
  printf("  LDC1614 version code: %d (major=%d minor=%d patch=%d)\n",
         LDC1614::VERSION_INT,
         LDC1614::VERSION_MAJOR,
         LDC1614::VERSION_MINOR,
         LDC1614::VERSION_PATCH);
}

void Cli::printDeviceStatus(const LDC1614::DeviceStatus& ds) const {
  printf("  STATUS raw=0x%04X drdy=%s%s%s errCh=%u\n",
         ds.raw,
         yesNoColor(ds.dataReady),
         ds.dataReady ? "YES" : "no",
         COLOR_RESET,
         ds.errChan);
  printf("  Errors: %sUR=%d OR=%d WD=%d AH=%d AL=%d ZC=%d%s\n",
         ds.hasError() ? COLOR_RED : COLOR_GREEN,
         ds.errUnderRange, ds.errOverRange, ds.errWatchdog,
         ds.errAmplitudeHigh, ds.errAmplitudeLow, ds.errZeroCount,
         COLOR_RESET);
  printf("  Unread: ch0=%d ch1=%d ch2=%d ch3=%d\n",
         ds.unreadConv[0], ds.unreadConv[1], ds.unreadConv[2], ds.unreadConv[3]);
}

void Cli::printChannelData(uint8_t ch, const LDC1614::ChannelData& data) const {
  printf("  Ch%u: raw=0x%07lX (%lu)", ch,
         static_cast<unsigned long>(data.rawData),
         static_cast<unsigned long>(data.rawData));
  if (data.hasError()) {
    printf(" %s[ERR: UR=%d OR=%d WD=%d AE=%d]%s",
           COLOR_RED,
           data.errUnderRange, data.errOverRange,
           data.errWatchdog, data.errAmplitude,
           COLOR_RESET);
  }
  println();
}

void Cli::scanI2c() {
  if (_platform.i2cProbe == nullptr) {
    logWarn("I2C scan is not configured.");
    return;
  }

  logInfo("Scanning I2C bus (timeout=%lums)...",
          static_cast<unsigned long>(_platform.scanTimeoutMs));
  logInfo("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");

  uint8_t count = 0;
  for (uint8_t row = 0; row < 8; row++) {
    printf("%02X: ", row * 16U);
    for (uint8_t col = 0; col < 16; col++) {
      const uint8_t addr = static_cast<uint8_t>(row * 16U + col);
      if (addr < 0x08U || addr > 0x77U) {
        printf("   ");
        continue;
      }

      if (isLdcAddress(addr)) {
        uint16_t manufacturer = 0;
        uint16_t deviceId = 0;
        LDC1614::Status failure = LDC1614::Status::Ok();
        if (readIdentityRaw(addr, manufacturer, deviceId, failure)) {
          const bool identityOk =
              manufacturer == LDC1614::cmd::MANUFACTURER_ID_VALUE &&
              deviceId == LDC1614::cmd::DEVICE_ID_VALUE;
          printf("%s%02X%s ", identityOk ? COLOR_GREEN : COLOR_YELLOW, addr, COLOR_RESET);
          count++;
        } else if (failure.code == LDC1614::Err::I2C_TIMEOUT ||
                   failure.code == LDC1614::Err::TIMEOUT) {
          printf("TO ");
        } else {
          printf("-- ");
        }
      } else {
        const I2cProbeResult result =
            _platform.i2cProbe(addr, _platform.scanTimeoutMs, _platform.user);
        if (result == I2cProbeResult::ACK) {
          printf("%02X ", addr);
          count++;
        } else if (result == I2cProbeResult::TIMEOUT) {
          printf("TO ");
        } else {
          printf("-- ");
        }
      }
      yield();
      delayMs(1);
    }
    println();
  }

  logInfo("Scan complete. Found %u device(s).", count);
  if (count > 0U) {
    logInfo("Common addresses: 0x2A/0x2B=LDC1614, 0x48-0x4B=ADS1115, "
            "0x51=RV3032, 0x76/0x77=BME280");
    logInfo("LDC addresses are verified with MANUFACTURER_ID/DEVICE_ID reads.");
  }
}

uint8_t Cli::diagnosticAddress() const {
  const LDC1614::Config cfg = makeDefaultConfig();
  if (isLdcAddress(cfg.i2cAddress)) {
    return cfg.i2cAddress;
  }

  const LDC1614::Config& active = _device.getConfig();
  if (isLdcAddress(active.i2cAddress)) {
    return active.i2cAddress;
  }

  return 0x2AU;
}

LDC1614::Status Cli::rawReadRegister16(uint8_t i2cAddress, uint8_t reg,
                                       uint16_t& value) const {
  const LDC1614::Config cfg = makeDefaultConfig();
  if (cfg.i2cWriteRead == nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG,
                                  "I2C read callback not configured");
  }
  if (i2cAddress < 0x08U || i2cAddress > 0x77U) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM,
                                  "I2C address out of 7-bit user range",
                                  i2cAddress);
  }

  const uint8_t tx = reg;
  uint8_t rx[2] = {};
  const uint32_t timeoutMs = cfg.i2cTimeoutMs > 0U ? cfg.i2cTimeoutMs : _platform.scanTimeoutMs;
  const LDC1614::Status st =
      cfg.i2cWriteRead(i2cAddress, &tx, 1U, rx, sizeof(rx), timeoutMs, cfg.i2cUser);
  if (!st.ok()) {
    return st;
  }

  value = static_cast<uint16_t>((static_cast<uint16_t>(rx[0]) << 8) | rx[1]);
  return LDC1614::Status::Ok();
}

LDC1614::Status Cli::rawWriteRegister16(uint8_t i2cAddress, uint8_t reg,
                                        uint16_t value) const {
  const LDC1614::Config cfg = makeDefaultConfig();
  if (cfg.i2cWrite == nullptr) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG,
                                  "I2C write callback not configured");
  }
  if (i2cAddress < 0x08U || i2cAddress > 0x77U) {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM,
                                  "I2C address out of 7-bit user range",
                                  i2cAddress);
  }

  const uint8_t tx[3] = {
      reg,
      static_cast<uint8_t>((value >> 8) & 0xFFU),
      static_cast<uint8_t>(value & 0xFFU),
  };
  const uint32_t timeoutMs = cfg.i2cTimeoutMs > 0U ? cfg.i2cTimeoutMs : _platform.scanTimeoutMs;
  return cfg.i2cWrite(i2cAddress, tx, sizeof(tx), timeoutMs, cfg.i2cUser);
}

bool Cli::readIdentityRaw(uint8_t address, uint16_t& manufacturer, uint16_t& deviceId,
                          LDC1614::Status& failure) const {
  failure = rawReadRegister16(address, LDC1614::cmd::REG_MANUFACTURER_ID, manufacturer);
  if (!failure.ok()) {
    return false;
  }

  failure = rawReadRegister16(address, LDC1614::cmd::REG_DEVICE_ID, deviceId);
  if (!failure.ok()) {
    return false;
  }

  failure = LDC1614::Status::Ok();
  return true;
}

void Cli::printRawIdentity(uint8_t address) const {
  println("=== Raw LDC Identity ===");
  printf("  Address: 0x%02X\n", address);

  uint16_t manufacturer = 0;
  LDC1614::Status st =
      rawReadRegister16(address, LDC1614::cmd::REG_MANUFACTURER_ID, manufacturer);
  printf("  Read MANUFACTURER_ID (0x%02X):\n", LDC1614::cmd::REG_MANUFACTURER_ID);
  printStatus(st);
  if (st.ok()) {
    const bool ok = manufacturer == LDC1614::cmd::MANUFACTURER_ID_VALUE;
    printf("  Value: 0x%04X expected=0x%04X match=%s%s%s\n",
           manufacturer,
           LDC1614::cmd::MANUFACTURER_ID_VALUE,
           yesNoColor(ok),
           ok ? "YES" : "NO",
           COLOR_RESET);
  }

  uint16_t deviceId = 0;
  st = rawReadRegister16(address, LDC1614::cmd::REG_DEVICE_ID, deviceId);
  printf("  Read DEVICE_ID (0x%02X):\n", LDC1614::cmd::REG_DEVICE_ID);
  printStatus(st);
  if (st.ok()) {
    const bool ok = deviceId == LDC1614::cmd::DEVICE_ID_VALUE;
    printf("  Value: 0x%04X expected=0x%04X match=%s%s%s\n",
           deviceId,
           LDC1614::cmd::DEVICE_ID_VALUE,
           yesNoColor(ok),
           ok ? "YES" : "NO",
           COLOR_RESET);
  }
}

void Cli::printBusDiagnostics() {
  const LDC1614::Config cfg = makeDefaultConfig();
  println("=== I2C Bus Diagnostics ===");
  printf("  Default LDC address: 0x%02X\n", cfg.i2cAddress);
  printf("  Timeout: %lu ms\n", static_cast<unsigned long>(cfg.i2cTimeoutMs));
  printf("  Callbacks: write=%s read=%s addr-probe=%s now=%s yield=%s gpio=%s\n",
         boolStr(cfg.i2cWrite != nullptr),
         boolStr(cfg.i2cWriteRead != nullptr),
         boolStr(_platform.i2cProbe != nullptr),
         boolStr(cfg.nowMs != nullptr),
         boolStr(cfg.cooperativeYield != nullptr),
         boolStr(cfg.gpioRead != nullptr));
  println("  LDC scan entries use repeated-start ID reads, not address-only probes.");
  scanI2c();
  printRawIdentity(0x2AU);
  printRawIdentity(0x2BU);
}

void Cli::runSelfTest() {
  struct TestStats {
    uint32_t pass = 0;
    uint32_t fail = 0;
    uint32_t skip = 0;
  } stats;

  enum class SelftestOutcome : uint8_t { PASS, FAIL, SKIP };
  auto report = [&](const char* name, SelftestOutcome outcome, const char* note) {
    const bool passed = outcome == SelftestOutcome::PASS;
    const bool skipped = outcome == SelftestOutcome::SKIP;
    const char* color = skipped ? COLOR_YELLOW : resultColor(passed);
    const char* tag = skipped ? "SKIP" : (passed ? "PASS" : "FAIL");
    printf("  [%s%s%s] %s", color, tag, COLOR_RESET, name);
    if (note != nullptr && note[0] != '\0') {
      printf(" - %s", note);
    }
    println();
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

  println("=== LDC1614 selftest (safe commands) ===");

  const uint32_t succBefore = _device.totalSuccess();
  const uint32_t failBefore = _device.totalFailures();
  const uint8_t consBefore = _device.consecutiveFailures();

  const LDC1614::Status pst = _device.probe();
  if (pst.code == LDC1614::Err::NOT_INITIALIZED) {
    reportSkip("probe responds", "driver not initialized");
    reportSkip("remaining checks", "selftest aborted");
    printf("Selftest result: pass=%s%lu%s fail=%s%lu%s skip=%s%lu%s\n",
           goodIfNonZeroColor(stats.pass), static_cast<unsigned long>(stats.pass), COLOR_RESET,
           goodIfZeroColor(stats.fail), static_cast<unsigned long>(stats.fail), COLOR_RESET,
           skipCountColor(stats.skip), static_cast<unsigned long>(stats.skip), COLOR_RESET);
    return;
  }
  const bool probeHealthUnchanged =
      _device.totalSuccess() == succBefore &&
      _device.totalFailures() == failBefore &&
      _device.consecutiveFailures() == consBefore;
  reportCheck("probe responds", pst.ok(), pst.ok() ? "" : errToStr(pst.code));
  reportCheck("probe no-health-side-effects", probeHealthUnchanged, "");

  LDC1614::DeviceStatus ds;
  LDC1614::Status st = _device.readDeviceStatus(ds);
  reportCheck("readDeviceStatus", st.ok(), st.ok() ? "" : errToStr(st.code));

  uint16_t regVal = 0;
  st = _device.readRegister16(LDC1614::cmd::REG_MANUFACTURER_ID, regVal);
  reportCheck("readRegister16(MFR_ID)", st.ok(), st.ok() ? "" : errToStr(st.code));
  if (st.ok()) {
    reportCheck("MFR_ID == 0x5449", regVal == LDC1614::cmd::MANUFACTURER_ID_VALUE, "");
  }

  st = _device.readRegister16(LDC1614::cmd::REG_DEVICE_ID, regVal);
  reportCheck("readRegister16(DEV_ID)", st.ok(), st.ok() ? "" : errToStr(st.code));
  if (st.ok()) {
    reportCheck("DEV_ID == 0x3055", regVal == LDC1614::cmd::DEVICE_ID_VALUE, "");
  }

  st = _device.sleep();
  reportCheck("sleep", st.ok(), st.ok() ? "" : errToStr(st.code));
  reportCheck("isSleeping after sleep", _device.isSleeping(), "");

  st = _device.wake();
  reportCheck("wake", st.ok(), st.ok() ? "" : errToStr(st.code));
  reportCheck("not sleeping after wake", !_device.isSleeping(), "");

  st = _device.recover();
  reportCheck("recover", st.ok(), st.ok() ? "" : errToStr(st.code));
  reportCheck("isOnline", _device.isOnline(), "");

  st = _device.sleep();
  reportCheck("sleep (restore)", st.ok(), st.ok() ? "" : errToStr(st.code));

  printf("Selftest result: pass=%s%lu%s fail=%s%lu%s skip=%s%lu%s\n",
         goodIfNonZeroColor(stats.pass), static_cast<unsigned long>(stats.pass), COLOR_RESET,
         goodIfZeroColor(stats.fail), static_cast<unsigned long>(stats.fail), COLOR_RESET,
         skipCountColor(stats.skip), static_cast<unsigned long>(stats.skip), COLOR_RESET);
}

void Cli::printConfig() {
  println("=== Active Configuration ===");
  LDC1614::SettingsSnapshot snap;
  (void)_device.getSettings(snap);

  const LDC1614::Config defaultConfig = makeDefaultConfig();
  const LDC1614::Config& activeConfig = _device.getConfig();
  const LDC1614::Config& cfg =
      (activeConfig.i2cWrite != nullptr && activeConfig.i2cWriteRead != nullptr)
          ? activeConfig
          : defaultConfig;
  const bool liveReads = _device.isOnline();
  const uint8_t channelCount = cfg.channelCount <= 4U ? cfg.channelCount : 4U;

  printf("  Initialized: %s%s%s  State: %s%s%s  Online: %s%s%s\n",
         snap.initialized ? COLOR_GREEN : COLOR_YELLOW,
         boolStr(snap.initialized),
         COLOR_RESET,
         stateColor(snap.state, _device.isOnline(), _device.consecutiveFailures()),
         stateToStr(snap.state),
         COLOR_RESET,
         liveReads ? COLOR_GREEN : COLOR_RED,
         boolStr(liveReads),
         COLOR_RESET);
  printf("  I2C address: 0x%02X  timeout=%lu ms  offlineThreshold=%u\n",
         cfg.i2cAddress,
         static_cast<unsigned long>(cfg.i2cTimeoutMs),
         cfg.offlineThreshold);
  printf("  Callbacks: write=%s read=%s now=%s yield=%s gpio=%s busReset=%s hardReset=%s\n",
         boolStr(cfg.i2cWrite != nullptr),
         boolStr(cfg.i2cWriteRead != nullptr),
         boolStr(cfg.nowMs != nullptr),
         boolStr(cfg.cooperativeYield != nullptr),
         boolStr(cfg.gpioRead != nullptr),
         boolStr(cfg.busReset != nullptr),
         boolStr(cfg.hardReset != nullptr));
  printf("  Mode: %s  RR sequence: %s  Deglitch: %s\n",
         cfg.autoScan ? "auto-scan" : "single-channel",
         rrSequenceToStr(cfg.rrSequence),
         deglitchToStr(cfg.deglitch));
  printf("  RefClk: %s  Activation: %s\n",
         refClkToStr(cfg.refClkSrc),
         activationToStr(cfg.sensorActivation));
  printf("  RPoverride: %s  AutoAmp: %s  HighCurrentDrv: %s\n",
         boolStr(cfg.rpOverrideEn),
         boolStr(!cfg.autoAmpDis),
         boolStr(cfg.highCurrentDrv));
  printf("  INTB configured: %s  INTB output: %s\n",
         boolStr(cfg.intbPin >= 0),
         boolStr(cfg.intbPin >= 0 && !cfg.intbDisable));
  printf("  Cached ERROR_CONFIG: 0x%04X\n", cfg.errorConfig);
  printf("  Hardware config dirty: %s%s%s\n",
         _device.hardwareConfigDirty() ? COLOR_RED : COLOR_GREEN,
         boolStr(_device.hardwareConfigDirty()),
         COLOR_RESET);
  if (_device.hardwareConfigDirty()) {
    printStatus(_device.hardwareConfigDirtyError());
  }

  uint16_t regVal = 0;
  uint32_t configReadbackFailures = 0;
  auto readLiveRegister = [&](uint8_t reg, uint16_t& value) -> LDC1614::Status {
    const LDC1614::Status st = _device.readRegister16(reg, value);
    if (!st.ok()) {
      configReadbackFailures++;
      printStatus(st);
    }
    return st;
  };
  if (liveReads) {
    LDC1614::Status st = readLiveRegister(LDC1614::cmd::REG_MUX_CONFIG, regVal);
    if (st.ok()) {
      printf("  Live MUX_CONFIG: 0x%04X\n", regVal);
    }
    st = readLiveRegister(LDC1614::cmd::REG_CONFIG, regVal);
    if (st.ok()) {
      printf("  Live CONFIG: 0x%04X (sleep=%d)\n", regVal, (regVal >> 13) & 1);
    }
    st = readLiveRegister(LDC1614::cmd::REG_ERROR_CONFIG, regVal);
    if (st.ok()) {
      printf("  Live ERROR_CONFIG: 0x%04X\n", regVal);
    }
  } else {
    println("  Live registers: unavailable (driver is not online)");
  }

  printf("  Channel count: %u\n", channelCount);
  printf("  Active channel: %u\n", cfg.activeChan);
  printf("  Sleeping: %s%s%s\n",
         _device.isSleeping() ? COLOR_YELLOW : COLOR_GREEN,
         boolStr(_device.isSleeping()),
         COLOR_RESET);

  for (uint8_t ch = 0; ch < channelCount; ch++) {
    const auto& cc = cfg.channel[ch];
    printf("  --- Channel %u ---\n", ch);
    printf("    Cached: RCOUNT=0x%04X SETTLE=0x%04X CLOCK_DIV(FIN=%u,FREF=%u) "
           "OFFSET=0x%04X IDRIVE=%u\n",
           cc.rcount,
           cc.settleCount,
           cc.finDivider,
           cc.frefDivider,
           cc.offset,
           cc.idrive);
    if (liveReads) {
      LDC1614::Status st = readLiveRegister(LDC1614::cmd::regRcount(ch), regVal);
      if (st.ok()) {
        printf("    Live RCOUNT: 0x%04X (%u)\n", regVal, regVal);
      }
      st = readLiveRegister(LDC1614::cmd::regSettleCount(ch), regVal);
      if (st.ok()) {
        printf("    Live SETTLECOUNT: 0x%04X (%u)\n", regVal, regVal);
      }
      st = readLiveRegister(LDC1614::cmd::regClockDividers(ch), regVal);
      if (st.ok()) {
        const uint8_t finDiv = static_cast<uint8_t>((regVal >> 12) & 0x0F);
        const uint16_t frefDiv = regVal & 0x03FF;
        printf("    Live CLOCK_DIV: 0x%04X (FIN=%u, FREF=%u)\n",
               regVal, finDiv, frefDiv);
      }
      st = readLiveRegister(LDC1614::cmd::regDriveCurrent(ch), regVal);
      if (st.ok()) {
        const uint8_t idrive = static_cast<uint8_t>((regVal >> 11) & 0x1F);
        const uint8_t initIdrive = static_cast<uint8_t>((regVal >> 6) & 0x1F);
        printf("    Live DRIVE_CURRENT: 0x%04X (IDRIVE=%u, INIT_IDRIVE=%u)\n",
               regVal, idrive, initIdrive);
      }
      st = readLiveRegister(LDC1614::cmd::regOffset(ch), regVal);
      if (st.ok()) {
        printf("    Live OFFSET: 0x%04X (%u)\n", regVal, regVal);
      }
    }
  }
  printf("  config_readback_failures=%lu\n",
         static_cast<unsigned long>(configReadbackFailures));
}

void Cli::printIdentity() {
  if (!_device.isOnline()) {
    logWarn("Device not online; using raw identity reads.");
    printRawIdentity(diagnosticAddress());
    return;
  }

  uint16_t manufacturer = 0;
  uint16_t deviceId = 0;
  LDC1614::Status st = _device.readRegister16(LDC1614::cmd::REG_MANUFACTURER_ID,
                                              manufacturer);
  if (!st.ok()) {
    printStatus(st);
    printRawIdentity(diagnosticAddress());
    return;
  }
  st = _device.readRegister16(LDC1614::cmd::REG_DEVICE_ID, deviceId);
  if (!st.ok()) {
    printStatus(st);
    printRawIdentity(diagnosticAddress());
    return;
  }

  const bool manufacturerOk = manufacturer == LDC1614::cmd::MANUFACTURER_ID_VALUE;
  const bool deviceOk = deviceId == LDC1614::cmd::DEVICE_ID_VALUE;
  println("=== Device Identity ===");
  printf("  MANUFACTURER_ID: 0x%04X expected=0x%04X match=%s%s%s\n",
         manufacturer,
         LDC1614::cmd::MANUFACTURER_ID_VALUE,
         yesNoColor(manufacturerOk),
         manufacturerOk ? "YES" : "NO",
         COLOR_RESET);
  printf("  DEVICE_ID:       0x%04X expected=0x%04X match=%s%s%s\n",
         deviceId,
         LDC1614::cmd::DEVICE_ID_VALUE,
         yesNoColor(deviceOk),
         deviceOk ? "YES" : "NO",
         COLOR_RESET);
}

void Cli::runStressMix(int count) {
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

  const uint32_t successBefore = _device.totalSuccess();
  const uint32_t failBefore = _device.totalFailures();
  const uint32_t startMs = nowMs();

  for (int i = 0; i < count; ++i) {
    LDC1614::Status st = LDC1614::Status::Ok();
    const int op = i % opCount;

    switch (op) {
      case 0: {
        LDC1614::ChannelData data;
        st = _device.readChannel(0, data);
        break;
      }
      case 1: {
        LDC1614::ChannelData data[4];
        st = _device.readAllChannels(data);
        break;
      }
      case 2: {
        LDC1614::DeviceStatus ds;
        st = _device.readDeviceStatus(ds);
        break;
      }
      case 3:
        st = _device.probe();
        break;
      case 4:
        st = _device.sleep();
        if (st.ok()) {
          st = _device.wake();
        }
        break;
      case 5: {
        uint16_t val = 0;
        st = _device.readRegister16(LDC1614::cmd::REG_MANUFACTURER_ID, val);
        break;
      }
      default:
        break;
    }

    if (st.ok()) {
      stats[op].ok++;
    } else {
      stats[op].fail++;
      if (_verboseMode) {
        logVerbose("[%d] %s failed: %s", i, stats[op].name, errToStr(st.code));
      }
    }

    if ((i + 1) % 50 == 0) {
      _device.tick(nowMs());
    }
  }

  const uint32_t elapsed = nowMs() - startMs;
  uint32_t okTotal = 0;
  uint32_t failTotal = 0;
  for (int i = 0; i < opCount; ++i) {
    okTotal += stats[i].ok;
    failTotal += stats[i].fail;
  }

  println("=== stress_mix summary ===");
  const float successPct =
      count > 0 ? (100.0f * static_cast<float>(okTotal) / static_cast<float>(count)) : 0.0f;
  printf("  Total: %sok=%lu%s %sfail=%lu%s (%s%.2f%%%s)\n",
         goodIfNonZeroColor(okTotal),
         static_cast<unsigned long>(okTotal),
         COLOR_RESET,
         goodIfZeroColor(failTotal),
         static_cast<unsigned long>(failTotal),
         COLOR_RESET,
         successRateColor(successPct),
         successPct,
         COLOR_RESET);
  printf("  Duration: %lu ms\n", static_cast<unsigned long>(elapsed));
  if (elapsed > 0U) {
    printf("  Rate: %.2f ops/s\n", (1000.0f * static_cast<float>(count)) / elapsed);
  }
  for (int i = 0; i < opCount; ++i) {
    printf("  %-12s %sok=%lu%s %sfail=%lu%s\n",
           stats[i].name,
           goodIfNonZeroColor(stats[i].ok),
           static_cast<unsigned long>(stats[i].ok),
           COLOR_RESET,
           goodIfZeroColor(stats[i].fail),
           static_cast<unsigned long>(stats[i].fail),
           COLOR_RESET);
  }
  const uint32_t successDelta = _device.totalSuccess() - successBefore;
  const uint32_t failDelta = _device.totalFailures() - failBefore;
  printf("  Health delta: %ssuccess +%lu%s, %sfailures +%lu%s\n",
         goodIfNonZeroColor(successDelta),
         static_cast<unsigned long>(successDelta),
         COLOR_RESET,
         goodIfZeroColor(failDelta),
         static_cast<unsigned long>(failDelta),
         COLOR_RESET);
}

void Cli::runStress(int count) {
  int ok = 0;
  int fail = 0;
  bool hasFailure = false;
  LDC1614::Status firstFailure = LDC1614::Status::Ok();
  LDC1614::Status lastFailure = LDC1614::Status::Ok();
  for (int i = 0; i < count; ++i) {
    LDC1614::ChannelData data;
    LDC1614::Status st = _device.readChannel(0, data);
    if (st.ok()) {
      ok++;
      if (_verboseMode) {
        logVerbose("  %d: raw=0x%07lX", i + 1,
                   static_cast<unsigned long>(data.rawData));
      }
    } else {
      fail++;
      if (!hasFailure) {
        firstFailure = st;
        hasFailure = true;
      }
      lastFailure = st;
      if (_verboseMode) {
        printStatus(st);
      }
    }
  }
  const float pct = count > 0 ? (100.0f * static_cast<float>(ok) /
                                 static_cast<float>(count)) : 0.0f;
  printf("  Stress results: %s%d ok%s, %s%d failed%s (%s%.2f%%%s)\n",
         goodIfNonZeroColor(static_cast<uint32_t>(ok)),
         ok,
         COLOR_RESET,
         goodIfZeroColor(static_cast<uint32_t>(fail)),
         fail,
         COLOR_RESET,
         successRateColor(pct),
         pct,
         COLOR_RESET);
  if (hasFailure) {
    println("  Failure details:");
    println("  First failure:");
    printStatus(firstFailure);
    if (fail > 1) {
      println("  Last failure:");
      printStatus(lastFailure);
    }
  }
}

void Cli::runDemo(int count) {
  println("=== demo workflow ===");
  if (!_device.isOnline()) {
    logWarn("Device not online. Run 'init' first.");
    return;
  }

  const bool wasSleeping = _device.isSleeping();

  println("  Step: probe");
  LDC1614::Status st = _device.probe();
  printStatus(st);
  if (!st.ok()) {
    return;
  }

  println("  Step: wake");
  st = _device.wake();
  printStatus(st);
  if (!st.ok()) {
    return;
  }

  uint32_t readyCount = 0;
  uint32_t readOk = 0;
  uint32_t readFail = 0;
  for (int i = 0; i < count; ++i) {
    delayMs(20);
    _device.tick(nowMs());

    bool ready = false;
    st = _device.readDataReady(ready);
    if (!st.ok()) {
      readFail++;
      printf("  Sample %d: drdy=%sERROR%s\n", i + 1, COLOR_RED, COLOR_RESET);
      printStatus(st);
      continue;
    }

    if (ready) {
      readyCount++;
      LDC1614::ChannelData data;
      st = _device.readChannel(0, data);
      if (st.ok()) {
        readOk++;
        printf("  Sample %d: drdy=%sYES%s", i + 1, COLOR_GREEN, COLOR_RESET);
        printf(" raw=0x%07lX (%lu)",
               static_cast<unsigned long>(data.rawData),
               static_cast<unsigned long>(data.rawData));
        if (data.hasError()) {
          printf(" %s[ERR: UR=%d OR=%d WD=%d AE=%d]%s",
                 COLOR_RED,
                 data.errUnderRange, data.errOverRange,
                 data.errWatchdog, data.errAmplitude,
                 COLOR_RESET);
        }
        println();
      } else {
        readFail++;
        printf("  Sample %d: read failed\n", i + 1);
        printStatus(st);
      }
    } else {
      printf("  Sample %d: drdy=%sNO%s\n", i + 1, COLOR_YELLOW, COLOR_RESET);
    }
  }

  LDC1614::DeviceStatus ds;
  st = _device.readDeviceStatus(ds);
  if (st.ok()) {
    printDeviceStatus(ds);
  } else {
    printStatus(st);
  }

  if (wasSleeping) {
    println("  Step: restore sleep");
    st = _device.sleep();
    printStatus(st);
  }

  const float pct = count > 0 ? (100.0f * static_cast<float>(readOk) /
                                 static_cast<float>(count)) : 0.0f;
  printf("  Demo result: ready=%lu/%d read_ok=%s%lu%s read_fail=%s%lu%s (%s%.2f%%%s)\n",
         static_cast<unsigned long>(readyCount),
         count,
         goodIfNonZeroColor(readOk),
         static_cast<unsigned long>(readOk),
         COLOR_RESET,
         goodIfZeroColor(readFail),
         static_cast<unsigned long>(readFail),
         COLOR_RESET,
         successRateColor(pct),
         pct,
         COLOR_RESET);
  printDriverHealth();
}

void Cli::processCommand(const char* cmdLine) {
  Line cmd(cmdLine);
  cmd.trim();

  if (cmd.empty()) {
    return;
  }

  if (cmd == "help" || cmd == "?") {
    printHelp();
  } else if (cmd == "version" || cmd == "ver") {
    printVersionInfo();
  } else if (cmd == "scan") {
    scanI2c();
  } else if (cmd == "bus" || cmd == "i2cdiag") {
    printBusDiagnostics();
  } else if (cmd.startsWith("probeaddr ")) {
    uint32_t addr = 0;
    if (!parseU32Flexible(cmd.substring(10), addr) || addr > 0x7FU) {
      logWarn("Usage: probeaddr <addr>");
      return;
    }
    if (!isLdcAddress(static_cast<uint8_t>(addr))) {
      logWarn("0x%02lX is not an LDC1614 address (expected 0x2A or 0x2B)",
              static_cast<unsigned long>(addr));
    }
    printRawIdentity(static_cast<uint8_t>(addr));
  } else if (cmd == "probe") {
    logInfo("Probing device (no health tracking)...");
    printRawIdentity(diagnosticAddress());
    const HealthSnapshot before = captureHealth();
    const LDC1614::Status st = _device.probe();
    printStatus(st);
    const HealthSnapshot after = captureHealth();
    println("  Health changes:");
    printHealthDiff(before, after);
  } else if (cmd == "drv") {
    printDriverHealth();
  } else if (cmd == "state") {
    printHealthCompact();
  } else if (cmd == "recover") {
    logInfo("Attempting recovery...");
    const HealthSnapshot before = captureHealth();
    const LDC1614::Status st = _device.recover();
    printStatus(st);
    const HealthSnapshot after = captureHealth();
    println("  Health changes:");
    printHealthDiff(before, after);
    printHealthCompact();
  } else if (cmd == "sync") {
    logInfo("Re-applying cached configuration...");
    const HealthSnapshot before = captureHealth();
    const LDC1614::Status st = _device.syncConfig();
    printStatus(st);
    const HealthSnapshot after = captureHealth();
    println("  Health changes:");
    printHealthDiff(before, after);
    printHealthCompact();
  } else if (cmd == "online") {
    const bool on = _device.isOnline();
    printf("  Online: %s%s%s\n", on ? COLOR_GREEN : COLOR_RED, boolStr(on), COLOR_RESET);
  } else if (cmd == "verbose") {
    logInfo("Verbose mode: %s%s%s", onOffColor(_verboseMode),
            _verboseMode ? "ON" : "OFF", COLOR_RESET);
  } else if (cmd.startsWith("verbose ")) {
    bool val = false;
    if (!parseBoolToken(cmd.substring(8), val)) {
      logWarn("Usage: verbose <0|1>");
      return;
    }
    _verboseMode = val;
    logInfo("Verbose mode: %s%s%s", onOffColor(_verboseMode),
            _verboseMode ? "ON" : "OFF", COLOR_RESET);
  } else if (cmd == "init" || cmd == "begin") {
    logInfo("Initializing LDC1614...");
    _device.end();
    const LDC1614::Status st = _device.begin(makeDefaultConfig());
    printStatus(st);
    if (st.ok()) {
      logInfo("Device initialized in sleep mode. Use 'wake' to start conversions.");
      printDriverHealth();
    } else {
      printRawIdentity(diagnosticAddress());
      printHealthCompact();
    }
  } else if (cmd == "end") {
    logInfo("Shutting down driver...");
    _device.end();
    logInfo("Driver state: UNINIT");
  } else if (cmd == "sleep") {
    printStatus(_device.sleep());
  } else if (cmd == "wake") {
    printStatus(_device.wake());
  } else if (cmd == "reset") {
    printStatus(_device.softReset());
  } else if (cmd == "resetreapply") {
    logInfo("Soft reset + re-apply config...");
    const LDC1614::Status st = _device.resetAndReapply();
    printStatus(st);
    if (st.ok()) {
      logInfo("Device back in READY/sleep. Use 'wake' to start conversions.");
    }
  } else if (cmd == "status") {
    if (!_device.isOnline()) {
      logWarn("Device not online.");
      return;
    }
    LDC1614::DeviceStatus ds;
    const LDC1614::Status st = _device.readDeviceStatus(ds);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    printDeviceStatus(ds);
  } else if (cmd == "status_raw") {
    if (!_device.isOnline()) {
      logWarn("Device not online.");
      return;
    }
    uint16_t raw = 0;
    const LDC1614::Status st = _device.readStatusRaw(raw);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    printf("  STATUS raw = 0x%04X\n", raw);
  } else if (cmd.startsWith("freq ")) {
    Line args = cmd.substring(5);
    args.trim();
    const int split = args.indexOf(' ');
    if (split < 0) {
      logWarn("Usage: freq <ch> <fRef>");
      return;
    }
    int32_t ch = 0;
    if (!parseI32(args.substring(0, static_cast<size_t>(split)), ch) || ch < 0 || ch > 3) {
      logWarn("Invalid channel (0-3)");
      return;
    }
    const float fRef = args.substring(static_cast<size_t>(split) + 1U).toFloat();
    if (fRef <= 0.0f) {
      logWarn("Invalid fRef (must be > 0)");
      return;
    }
    if (!_device.isOnline()) {
      logWarn("Device not online.");
      return;
    }
    LDC1614::ChannelData data;
    const LDC1614::Status st = _device.readChannel(static_cast<uint8_t>(ch), data);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    const float freq = _device.calcSensorFrequency(static_cast<uint8_t>(ch), data.rawData, fRef);
    printf("  Ch%ld: raw=0x%07lX freq=%.2f Hz\n",
           static_cast<long>(ch), static_cast<unsigned long>(data.rawData), freq);
  } else if (cmd.startsWith("timing ")) {
    Line args = cmd.substring(7);
    args.trim();
    const int split = args.indexOf(' ');
    if (split < 0) {
      logWarn("Usage: timing <ch> <fRef>");
      return;
    }
    int32_t ch = 0;
    if (!parseI32(args.substring(0, static_cast<size_t>(split)), ch) || ch < 0 || ch > 3) {
      logWarn("Invalid channel (0-3)");
      return;
    }
    const float fRef = args.substring(static_cast<size_t>(split) + 1U).toFloat();
    if (fRef <= 0.0f) {
      logWarn("Invalid fRef (must be > 0)");
      return;
    }
    const float convTimeUs = _device.calcConversionTimeUs(static_cast<uint8_t>(ch), fRef);
    const float settleTimeUs = _device.calcSettleTimeUs(static_cast<uint8_t>(ch), fRef);
    const float sampleTimeUs = _device.calcSampleTimeUs(static_cast<uint8_t>(ch), fRef);
    printf("  Ch%ld: conversion time = %.2f us (%.3f ms)\n",
           static_cast<long>(ch), convTimeUs, convTimeUs / 1000.0f);
    printf("  Ch%ld: settling time   = %.2f us (%.3f ms)\n",
           static_cast<long>(ch), settleTimeUs, settleTimeUs / 1000.0f);
    printf("  Ch%ld: sample time     = %.2f us (%.3f ms)\n",
           static_cast<long>(ch), sampleTimeUs, sampleTimeUs / 1000.0f);
  } else if (cmd == "channels") {
    printf("  Channel count: %u\n", _device.channelCount());
  } else if (cmd == "drdy") {
    bool ready = false;
    const LDC1614::Status st = _device.readDataReady(ready);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    printf("  Data ready: %s%s%s\n", yesNoColor(ready), ready ? "YES" : "NO", COLOR_RESET);
  } else if (cmd == "read") {
    if (!_device.isOnline()) {
      logWarn("Device not online. Run 'init' first.");
      return;
    }
    LDC1614::ChannelData data[4];
    const LDC1614::Status st = _device.readAllChannels(data);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    for (uint8_t i = 0; i < _device.channelCount(); i++) {
      printChannelData(i, data[i]);
    }
  } else if (cmd.startsWith("read ")) {
    if (!_device.isOnline()) {
      logWarn("Device not online. Run 'init' first.");
      return;
    }
    uint32_t values[2] = {};
    size_t count = 0;
    if (!parseU32FlexibleList(cmd.substring(5), values, 2U, count) ||
        count < 1U || values[0] > 3U) {
      logWarn("Usage: read <ch> [count]");
      return;
    }
    const uint32_t sampleCount = count == 2U ? values[1] : 1U;
    if (sampleCount == 0U || sampleCount > static_cast<uint32_t>(MAX_DEMO_COUNT)) {
      logWarn("Invalid count (1-%d)", MAX_DEMO_COUNT);
      return;
    }
    for (uint32_t i = 0; i < sampleCount; ++i) {
      LDC1614::ChannelData data;
      const LDC1614::Status st = _device.readChannel(static_cast<uint8_t>(values[0]), data);
      if (!st.ok()) {
        printf("  Sample %lu/%lu failed\n",
               static_cast<unsigned long>(i + 1U),
               static_cast<unsigned long>(sampleCount));
        printStatus(st);
        return;
      }
      if (sampleCount > 1U) {
        printf("  Sample %lu/%lu:\n",
               static_cast<unsigned long>(i + 1U),
               static_cast<unsigned long>(sampleCount));
      }
      printChannelData(static_cast<uint8_t>(values[0]), data);
    }
  } else if (cmd == "readfresh" || cmd.startsWith("readfresh ")) {
    if (!_device.isOnline()) {
      logWarn("Device not online. Run 'init' first.");
      return;
    }
    uint32_t requested = 0;
    if (cmd.length() > 9U &&
        (!parseU32Flexible(cmd.substring(10), requested) || requested > _device.channelCount())) {
      logWarn("Usage: readfresh [count]");
      return;
    }
    LDC1614::FreshChannelData fresh[4] = {};
    LDC1614::DeviceStatus status;
    const uint8_t count = requested == 0U ? _device.channelCount() : static_cast<uint8_t>(requested);
    const LDC1614::Status st = _device.readFreshChannels(fresh, status, count);
    printStatus(st);
    if (!st.ok()) {
      return;
    }
    printDeviceStatus(status);
    for (uint8_t ch = 0; ch < count && ch < 4U; ++ch) {
      const LDC1614::ChannelData& data = fresh[ch].data;
      printf("  Fresh ch%u fresh=%u valid=%u raw=0x%07lX errUR=%u errOR=%u errWD=%u errAmp=%u\n",
             static_cast<unsigned>(ch),
             fresh[ch].fresh ? 1U : 0U,
             fresh[ch].valid ? 1U : 0U,
             static_cast<unsigned long>(data.rawData),
             data.errUnderRange ? 1U : 0U,
             data.errOverRange ? 1U : 0U,
             data.errWatchdog ? 1U : 0U,
             data.errAmplitude ? 1U : 0U);
    }
    printf("  ReadFresh result: channels=%u\n", count);
  } else if (cmd.startsWith("readstaged ")) {
    if (!_device.isOnline()) {
      logWarn("Device not online. Run 'init' first.");
      return;
    }
    uint32_t values[3] = {};
    size_t count = 0;
    if (!parseU32FlexibleList(cmd.substring(11), values, 3U, count) ||
        count < 1U || values[0] == 0U || values[0] > 0x0FU) {
      logWarn("Usage: readstaged <mask> [maxPolls] [instrPerPoll]");
      return;
    }
    const uint8_t mask = static_cast<uint8_t>(values[0]);
    const uint32_t maxPolls = count >= 2U ? values[1] : 16U;
    const uint8_t instrPerPoll = count >= 3U ? static_cast<uint8_t>(values[2]) : 1U;
    if (maxPolls == 0U || maxPolls > MAX_STAGED_POLLS ||
        instrPerPoll == 0U || instrPerPoll > MAX_STAGED_INSTRUCTIONS) {
      logWarn("Invalid staged bounds (polls=1-%lu, instr=1-%u)",
              static_cast<unsigned long>(MAX_STAGED_POLLS),
              static_cast<unsigned>(MAX_STAGED_INSTRUCTIONS));
      return;
    }
    LDC1614::Status st = _device.startReadChannels(mask);
    printf("  readstaged start mask=0x%02X\n", mask);
    printStatus(st);
    if (!st.inProgress()) {
      return;
    }

    uint32_t polls = 0;
    for (; polls < maxPolls; ++polls) {
      st = _device.poll(nowMs(), instrPerPoll);
      if (st.inProgress()) {
        printf("  readstaged poll=%lu state=IN_PROGRESS\n",
               static_cast<unsigned long>(polls + 1U));
        continue;
      }
      printf("  readstaged poll=%lu\n", static_cast<unsigned long>(polls + 1U));
      printStatus(st);
      break;
    }
    if (st.inProgress()) {
      logWarn("readstaged timeout after %lu polls", static_cast<unsigned long>(maxPolls));
      return;
    }
    if (!st.ok()) {
      return;
    }
    for (uint8_t ch = 0; ch < _device.channelCount() && ch < 4U; ++ch) {
      if ((mask & (1U << ch)) == 0U) {
        continue;
      }
      LDC1614::ChannelData data;
      const LDC1614::Status sampleStatus = _device.getChannelSample(ch, data);
      printStatus(sampleStatus);
      if (sampleStatus.ok()) {
        printChannelData(ch, data);
      }
    }
    printf("  ReadStaged result: mask=0x%02X polls=%lu instr=%u\n",
           mask,
           static_cast<unsigned long>(polls + 1U),
           static_cast<unsigned>(instrPerPoll));
  } else if (cmd.startsWith("samplerate ")) {
    if (!_device.isOnline()) {
      logWarn("Device not online. Run 'init' and 'wake' first.");
      return;
    }
    uint32_t values[3] = {};
    size_t count = 0;
    if (!parseU32FlexibleList(cmd.substring(11), values, 3U, count) ||
        count < 2U || values[0] > 3U ||
        values[1] == 0U || values[1] > MAX_SAMPLE_RATE_COUNT) {
      logWarn("Usage: samplerate <ch> <count> [timeoutMs]");
      return;
    }
    const uint8_t ch = static_cast<uint8_t>(values[0]);
    if (ch >= _device.channelCount()) {
      logWarn("Invalid channel for configured variant");
      return;
    }
    const uint32_t requested = values[1];
    const uint32_t timeoutMs =
        count >= 3U ? values[2] : DEFAULT_SAMPLE_RATE_TIMEOUT_MS;
    if (timeoutMs == 0U || timeoutMs > MAX_SAMPLE_RATE_TIMEOUT_MS) {
      logWarn("Invalid timeout (1-%lu ms)",
              static_cast<unsigned long>(MAX_SAMPLE_RATE_TIMEOUT_MS));
      return;
    }

    uint32_t ok = 0;
    uint32_t fail = 0;
    uint32_t worstMs = 0;
    uint32_t firstRaw = 0;
    uint32_t lastRaw = 0;
    bool haveRaw = false;
    LDC1614::Status firstFailure = LDC1614::Status::Ok();
    LDC1614::Status lastFailure = LDC1614::Status::Ok();
    const uint32_t startMs = nowMs();
    for (uint32_t i = 0; i < requested; ++i) {
      LDC1614::ChannelData data;
      const uint32_t sampleStartMs = nowMs();
      const LDC1614::Status st = _device.readChannelBlocking(ch, data, timeoutMs);
      const uint32_t sampleMs = nowMs() - sampleStartMs;
      if (sampleMs > worstMs) {
        worstMs = sampleMs;
      }
      if (st.ok()) {
        ok++;
        if (!haveRaw) {
          firstRaw = data.rawData;
          haveRaw = true;
        }
        lastRaw = data.rawData;
      } else {
        fail++;
        if (fail == 1U) {
          firstFailure = st;
        }
        lastFailure = st;
        if (st.code != LDC1614::Err::TIMEOUT) {
          break;
        }
      }
      yield();
    }
    const uint32_t elapsedMs = nowMs() - startMs;
    const float hz = elapsedMs > 0U
                         ? (1000.0f * static_cast<float>(ok) /
                            static_cast<float>(elapsedMs))
                         : 0.0f;
    printf("  SampleRate result: requested=%lu ok=%lu fail=%lu elapsed_ms=%lu hz=%.3f worst_ms=%lu first_raw=0x%07lX last_raw=0x%07lX\n",
           static_cast<unsigned long>(requested),
           static_cast<unsigned long>(ok),
           static_cast<unsigned long>(fail),
           static_cast<unsigned long>(elapsedMs),
           hz,
           static_cast<unsigned long>(worstMs),
           static_cast<unsigned long>(firstRaw),
           static_cast<unsigned long>(lastRaw));
    if (fail > 0U) {
      println("  First failure:");
      printStatus(firstFailure);
      if (fail > 1U) {
        println("  Last failure:");
        printStatus(lastFailure);
      }
    }
  } else if (cmd == "readblocking") {
    if (!_device.isOnline()) {
      logWarn("Device not online. Run 'init' first.");
      return;
    }
    if (_device.isSleeping()) {
      logWarn("Device is sleeping. Use 'wake' first.");
      return;
    }
    LDC1614::ChannelData data[4];
    const LDC1614::Status st = _device.readAllChannelsBlocking(data);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    for (uint8_t i = 0; i < _device.channelCount(); i++) {
      printChannelData(i, data[i]);
    }
  } else if (cmd.startsWith("readblocking ")) {
    if (!_device.isOnline()) {
      logWarn("Device not online. Run 'init' first.");
      return;
    }
    if (_device.isSleeping()) {
      logWarn("Device is sleeping. Use 'wake' first.");
      return;
    }
    int32_t ch = 0;
    if (!parseI32(cmd.substring(13), ch)) {
      logWarn("Invalid channel (0-3)");
      return;
    }
    if (ch < 0 || ch > 3) {
      logWarn("Invalid channel (0-3)");
      return;
    }
    LDC1614::ChannelData data;
    const LDC1614::Status st = _device.readChannelBlocking(static_cast<uint8_t>(ch), data);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    printChannelData(static_cast<uint8_t>(ch), data);
  } else if (cmd.startsWith("sample ")) {
    int32_t ch = 0;
    if (!parseI32(cmd.substring(7), ch)) {
      logWarn("Invalid channel (0-3)");
      return;
    }
    if (ch < 0 || ch > 3) {
      logWarn("Invalid channel (0-3)");
      return;
    }
    LDC1614::ChannelData data;
    const LDC1614::Status st = _device.getLastSample(static_cast<uint8_t>(ch), data);
    if (!st.ok()) {
      logWarn("No cached sample for Ch%d", ch);
      return;
    }
    const uint32_t ts = _device.sampleTimestampMs(static_cast<uint8_t>(ch));
    const uint32_t age = _device.sampleAgeMs(static_cast<uint8_t>(ch), nowMs());
    printf("  Ch%d: raw=0x%07lX (%lu) age=%lu ms (at %lu ms)\n",
           ch,
           static_cast<unsigned long>(data.rawData),
           static_cast<unsigned long>(data.rawData),
           static_cast<unsigned long>(age),
           static_cast<unsigned long>(ts));
    if (data.hasError()) {
      printf("  %s[ERR: UR=%d OR=%d WD=%d AE=%d]%s\n",
             COLOR_RED,
             data.errUnderRange, data.errOverRange,
             data.errWatchdog, data.errAmplitude,
             COLOR_RESET);
    }
  } else if (cmd.startsWith("sampleage ")) {
    int32_t ch = 0;
    if (!parseI32(cmd.substring(10), ch)) {
      logWarn("Invalid channel (0-3)");
      return;
    }
    if (ch < 0 || ch > 3) {
      logWarn("Invalid channel (0-3)");
      return;
    }
    const uint32_t ts = _device.sampleTimestampMs(static_cast<uint8_t>(ch));
    if (ts == 0U) {
      printf("  Ch%d: %snever read%s\n", ch, COLOR_YELLOW, COLOR_RESET);
    } else {
      const uint32_t age = _device.sampleAgeMs(static_cast<uint8_t>(ch), nowMs());
      printf("  Ch%d: age=%lu ms (at %lu ms)\n", ch,
             static_cast<unsigned long>(age),
             static_cast<unsigned long>(ts));
    }
  } else if (cmd == "snapshot") {
    LDC1614::SettingsSnapshot snap;
    _device.getSettings(snap);
    println("=== Settings Snapshot (no I2C) ===");
    printf("  State: %s%s%s\n",
           stateColor(snap.state, snap.state == LDC1614::DriverState::READY ||
                                  snap.state == LDC1614::DriverState::DEGRADED, 0),
           stateToStr(snap.state),
           COLOR_RESET);
    printf("  Sleeping: %s\n", boolStr(snap.sleeping));
    printf("  Measuring: %s\n", boolStr(_device.isMeasuring()));
    printf("  hardwareConfigDirty=%d\n", snap.hardwareConfigDirty ? 1 : 0);
    if (snap.hardwareConfigDirty) {
      printStatus(snap.hardwareConfigDirtyError);
    }
    printf("  Channels: %u  Active: %u  AutoScan: %s\n",
           snap.channelCount, snap.activeChan, boolStr(snap.autoScan));
    printf("  RR sequence: %s  Deglitch: %s\n",
           rrSequenceToStr(snap.rrSequence),
           deglitchToStr(snap.deglitch));
    printf("  RefClk: %s  Activation: %s  INTB: %s\n",
           refClkToStr(snap.refClkSrc),
           activationToStr(snap.sensorActivation),
           boolStr(snap.intbEnabled));
    printf("  RPoverride: %s  AutoAmp: %s  HighCurrentDrv: %s\n",
           boolStr(snap.rpOverrideEn),
           boolStr(!snap.autoAmpDis),
           boolStr(snap.highCurrentDrv));
    for (uint8_t i = 0; i < snap.channelCount && i < 4; i++) {
      const auto& cc = snap.channel[i];
      const uint32_t age = snap.sampleTimestampMs[i] > 0U
                               ? (nowMs() - snap.sampleTimestampMs[i])
                               : 0U;
      printf("  Ch%u: rcount=0x%04X settle=0x%04X fin=%u fref=%u "
             "offset=0x%04X idrive=%u sample_age=%lu ms\n",
             i, cc.rcount, cc.settleCount, cc.finDivider, cc.frefDivider,
             cc.offset, cc.idrive, static_cast<unsigned long>(age));
    }
  } else if (cmd.startsWith("rcount ")) {
    Line args = cmd.substring(7);
    args.trim();
    const int split = args.indexOf(' ');
    if (split < 0) {
      logWarn("Usage: rcount <ch> <val>");
      return;
    }
    int32_t ch = 0;
    int32_t val = 0;
    if (!parseI32(args.substring(0, static_cast<size_t>(split)), ch) ||
        !parseI32(args.substring(static_cast<size_t>(split) + 1U), val)) {
      logWarn("Usage: rcount <ch> <val>");
      return;
    }
    printStatus(_device.setRcount(static_cast<uint8_t>(ch), static_cast<uint16_t>(val)));
  } else if (cmd.startsWith("settle ")) {
    Line args = cmd.substring(7);
    args.trim();
    const int split = args.indexOf(' ');
    if (split < 0) {
      logWarn("Usage: settle <ch> <val>");
      return;
    }
    int32_t ch = 0;
    int32_t val = 0;
    if (!parseI32(args.substring(0, static_cast<size_t>(split)), ch) ||
        !parseI32(args.substring(static_cast<size_t>(split) + 1U), val)) {
      logWarn("Usage: settle <ch> <val>");
      return;
    }
    printStatus(_device.setSettleCount(static_cast<uint8_t>(ch), static_cast<uint16_t>(val)));
  } else if (cmd.startsWith("clkdiv ")) {
    Line args = cmd.substring(7);
    args.trim();
    const int sp1 = args.indexOf(' ');
    if (sp1 < 0) {
      logWarn("Usage: clkdiv <ch> <fin> <fref>");
      return;
    }
    Line rest = args.substring(static_cast<size_t>(sp1) + 1U);
    rest.trim();
    const int sp2 = rest.indexOf(' ');
    if (sp2 < 0) {
      logWarn("Usage: clkdiv <ch> <fin> <fref>");
      return;
    }
    int32_t ch = 0;
    int32_t fin = 0;
    int32_t fref = 0;
    if (!parseI32(args.substring(0, static_cast<size_t>(sp1)), ch) ||
        !parseI32(rest.substring(0, static_cast<size_t>(sp2)), fin) ||
        !parseI32(rest.substring(static_cast<size_t>(sp2) + 1U), fref)) {
      logWarn("Usage: clkdiv <ch> <fin> <fref>");
      return;
    }
    printStatus(_device.setClockDividers(static_cast<uint8_t>(ch),
                                         static_cast<uint8_t>(fin),
                                         static_cast<uint16_t>(fref)));
  } else if (cmd.startsWith("offset ")) {
    Line args = cmd.substring(7);
    args.trim();
    const int split = args.indexOf(' ');
    if (split < 0) {
      logWarn("Usage: offset <ch> <val>");
      return;
    }
    int32_t ch = 0;
    int32_t val = 0;
    if (!parseI32(args.substring(0, static_cast<size_t>(split)), ch) ||
        !parseI32(args.substring(static_cast<size_t>(split) + 1U), val)) {
      logWarn("Usage: offset <ch> <val>");
      return;
    }
    printStatus(_device.setOffset(static_cast<uint8_t>(ch), static_cast<uint16_t>(val)));
  } else if (cmd.startsWith("idrive ")) {
    Line args = cmd.substring(7);
    args.trim();
    const int split = args.indexOf(' ');
    if (split < 0) {
      logWarn("Usage: idrive <ch> <val>");
      return;
    }
    int32_t ch = 0;
    int32_t val = 0;
    if (!parseI32(args.substring(0, static_cast<size_t>(split)), ch) ||
        !parseI32(args.substring(static_cast<size_t>(split) + 1U), val)) {
      logWarn("Usage: idrive <ch> <val>");
      return;
    }
    printStatus(_device.setDriveCurrent(static_cast<uint8_t>(ch), static_cast<uint8_t>(val)));
  } else if (cmd.startsWith("initidrive ")) {
    int32_t ch = 0;
    if (!parseI32(cmd.substring(11), ch)) {
      logWarn("Invalid channel (0-3)");
      return;
    }
    if (ch < 0 || ch > 3) {
      logWarn("Invalid channel (0-3)");
      return;
    }
    uint8_t initIdrive = 0;
    const LDC1614::Status st = _device.readInitIdrive(static_cast<uint8_t>(ch), initIdrive);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    printf("  Ch%d INIT_IDRIVE = %u\n", ch, initIdrive);
  } else if (cmd == "activech") {
    printf("  Active channel: %u\n", _device.getActiveChannel());
  } else if (cmd.startsWith("activech ")) {
    int32_t ch = 0;
    if (!parseI32(cmd.substring(9), ch)) {
      logWarn("Invalid channel (0-3)");
      return;
    }
    if (ch < 0 || ch > 3) {
      logWarn("Invalid channel (0-3)");
      return;
    }
    printStatus(_device.setActiveChannel(static_cast<uint8_t>(ch)));
  } else if (cmd.startsWith("single ")) {
    int32_t ch = 0;
    if (!parseI32(cmd.substring(7), ch)) {
      logWarn("Invalid channel (0-3)");
      return;
    }
    if (ch < 0 || ch > 3) {
      logWarn("Invalid channel (0-3)");
      return;
    }
    printStatus(_device.setSingleChannelMode(static_cast<uint8_t>(ch)));
  } else if (cmd.startsWith("autoscan ")) {
    LDC1614::RRSequence sequence = LDC1614::RRSequence::CH0_CH1;
    if (!parseRRSequence(cmd.substring(9), sequence)) {
      logWarn("Usage: autoscan <2|3|4>");
      return;
    }
    printStatus(_device.setAutoScanMode(sequence));
  } else if (cmd.startsWith("deglitch ")) {
    LDC1614::Deglitch deglitch = LDC1614::Deglitch::BW_33MHZ;
    if (!parseDeglitch(cmd.substring(9), deglitch)) {
      logWarn("Usage: deglitch <1|3|10|33>");
      return;
    }
    printStatus(_device.setDeglitch(deglitch));
  } else if (cmd == "errcfg") {
    printf("  Cached ERROR_CONFIG: 0x%04X\n", _device.getErrorConfig());
    if (_device.isOnline()) {
      uint16_t value = 0;
      const LDC1614::Status st =
          _device.readRegister16(LDC1614::cmd::REG_ERROR_CONFIG, value);
      if (st.ok()) {
        printf("  Live ERROR_CONFIG:   0x%04X\n", value);
      } else {
        printStatus(st);
      }
    }
  } else if (cmd.startsWith("errcfg ")) {
    uint32_t value = 0;
    if (!parseU32(cmd.substring(7), value) || value > 0xFFFFU) {
      logWarn("Usage: errcfg <mask>");
      return;
    }
    printStatus(_device.setErrorConfig(static_cast<uint16_t>(value)));
  } else if (cmd == "intb") {
    const LDC1614::Config& cfg = _device.getConfig();
    const bool outputEnabled = cfg.intbPin >= 0 && !cfg.intbDisable;
    const bool drdyRouted = (cfg.errorConfig & LDC1614::cmd::MASK_ERRCFG_DRDY_2INT) != 0U;
    printf("  INTB pin: %d  output: %s%s%s  DRDY_2INT=%d\n",
           cfg.intbPin,
           outputEnabled ? COLOR_GREEN : COLOR_YELLOW,
           boolStr(outputEnabled),
           COLOR_RESET,
           drdyRouted ? 1 : 0);
    if (cfg.intbPin >= 0 && cfg.gpioRead != nullptr) {
      const bool level = cfg.gpioRead(cfg.intbPin, cfg.gpioUser);
      printf("  INTB gpio level: %s%s%s (%s)\n",
             level ? COLOR_GREEN : COLOR_YELLOW,
             level ? "HIGH" : "LOW",
             COLOR_RESET,
             level ? "not asserted" : "asserted");
    } else {
      println("  INTB gpio level: unavailable");
    }
  } else if (cmd.startsWith("intb ")) {
    bool enabled = false;
    if (!parseBoolToken(cmd.substring(5), enabled)) {
      logWarn("Usage: intb <0|1>");
      return;
    }
    printStatus(_device.setIntbDisabled(!enabled));
  } else if (cmd.startsWith("refclk ")) {
    LDC1614::RefClkSrc source = LDC1614::RefClkSrc::INTERNAL;
    if (!parseRefClk(cmd.substring(7), source)) {
      logWarn("Usage: refclk <int|ext>");
      return;
    }
    printStatus(_device.setReferenceClockSource(source));
  } else if (cmd.startsWith("activate ")) {
    LDC1614::SensorActivation activation = LDC1614::SensorActivation::FULL_CURRENT;
    if (!parseActivation(cmd.substring(9), activation)) {
      logWarn("Usage: activate <full|low>");
      return;
    }
    printStatus(_device.setSensorActivation(activation));
  } else if (cmd.startsWith("rpoverride ")) {
    bool enabled = false;
    if (!parseBoolToken(cmd.substring(11), enabled)) {
      logWarn("Usage: rpoverride <0|1>");
      return;
    }
    printStatus(_device.setRpOverrideEnabled(enabled));
  } else if (cmd.startsWith("autoamp ")) {
    bool enabled = false;
    if (!parseBoolToken(cmd.substring(8), enabled)) {
      logWarn("Usage: autoamp <0|1>");
      return;
    }
    printStatus(_device.setAutoAmplitudeCorrectionEnabled(enabled));
  } else if (cmd.startsWith("highcurrent ")) {
    bool enabled = false;
    if (!parseBoolToken(cmd.substring(12), enabled)) {
      logWarn("Usage: highcurrent <0|1>");
      return;
    }
    printStatus(_device.setHighCurrentDriveEnabled(enabled));
  } else if (cmd.startsWith("rawreg ")) {
    uint32_t values[2] = {};
    size_t count = 0;
    if (!parseU32FlexibleList(cmd.substring(7), values, 2U, count) ||
        count < 1U || values[0] > 0xFFU ||
        (count == 2U && (values[1] < 0x08U || values[1] > 0x77U))) {
      logWarn("Usage: rawreg <reg> [addr]");
      return;
    }
    const uint8_t reg = static_cast<uint8_t>(values[0]);
    const uint8_t addr =
        count == 2U ? static_cast<uint8_t>(values[1]) : diagnosticAddress();
    uint16_t val = 0;
    const LDC1614::Status st = rawReadRegister16(addr, reg, val);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    printf("  Raw 0x%02X[0x%02X] = 0x%04X (%u)\n", addr, reg, val, val);
  } else if (cmd.startsWith("rawwreg ")) {
    uint32_t values[3] = {};
    size_t count = 0;
    if (!parseU32FlexibleList(cmd.substring(8), values, 3U, count) ||
        count < 2U || values[0] > 0xFFU || values[1] > 0xFFFFU ||
        (count == 3U && (values[2] < 0x08U || values[2] > 0x77U))) {
      logWarn("Usage: rawwreg <reg> <val> [addr]");
      return;
    }
    const uint8_t reg = static_cast<uint8_t>(values[0]);
    const uint16_t val = static_cast<uint16_t>(values[1]);
    const uint8_t addr =
        count == 3U ? static_cast<uint8_t>(values[2]) : diagnosticAddress();
    logWarn("Raw register write may desync cached driver config.");
    printStatus(rawWriteRegister16(addr, reg, val));
  } else if (cmd.startsWith("wreg ")) {
    if (!_device.isOnline()) {
      logWarn("Device not online.");
      return;
    }
    Line args = cmd.substring(5);
    args.trim();
    const int split = args.indexOf(' ');
    if (split < 0) {
      logWarn("Usage: wreg <addr> <val>");
      return;
    }
    uint32_t addr = 0;
    uint32_t val = 0;
    if (!parseU32(args.substring(0, static_cast<size_t>(split)), addr) ||
        !parseU32(args.substring(static_cast<size_t>(split) + 1U), val) ||
        addr > 0xFFU || val > 0xFFFFU) {
      logWarn("Usage: wreg <addr> <val>");
      return;
    }
    printStatus(_device.writeRegister16(static_cast<uint8_t>(addr),
                                        static_cast<uint16_t>(val)));
  } else if (cmd.startsWith("reg ")) {
    if (!_device.isOnline()) {
      logWarn("Device not online. Use 'rawreg <reg> [addr]' for pre-init diagnostics.");
      return;
    }
    uint32_t addr = 0;
    if (!parseU32(cmd.substring(4), addr) || addr > 0xFFU) {
      logWarn("Usage: reg <addr>");
      return;
    }
    uint16_t val = 0;
    const LDC1614::Status st = _device.readRegister16(static_cast<uint8_t>(addr), val);
    if (!st.ok()) {
      printStatus(st);
      return;
    }
    printf("  Reg 0x%02lX = 0x%04X (%u)\n", static_cast<unsigned long>(addr), val, val);
  } else if (cmd == "selftest") {
    runSelfTest();
  } else if (cmd == "cfg" || cmd == "settings" || cmd == "config") {
    printConfig();
  } else if (cmd == "id") {
    printIdentity();
  } else if (cmd == "demo") {
    runDemo(DEMO_DEFAULT_COUNT);
  } else if (cmd.startsWith("demo ")) {
    int32_t count = 0;
    if (!parseI32(cmd.substring(5), count)) {
      logWarn("Invalid count (1-%d)", MAX_DEMO_COUNT);
      return;
    }
    if (count <= 0 || count > MAX_DEMO_COUNT) {
      logWarn("Invalid count (1-%d)", MAX_DEMO_COUNT);
      return;
    }
    runDemo(count);
  } else if (cmd == "stress_mix") {
    if (!_device.isOnline()) {
      logWarn("Device not online. Run 'init' and 'wake' first.");
      return;
    }
    runStressMix(STRESS_MIX_DEFAULT_COUNT);
  } else if (cmd.startsWith("stress_mix ")) {
    if (!_device.isOnline()) {
      logWarn("Device not online. Run 'init' and 'wake' first.");
      return;
    }
    int32_t count = 0;
    if (!parseI32(cmd.substring(11), count)) {
      logWarn("Invalid count (1-%d)", MAX_STRESS_COUNT);
      return;
    }
    if (count <= 0 || count > MAX_STRESS_COUNT) {
      logWarn("Invalid count (1-%d)", MAX_STRESS_COUNT);
      return;
    }
    runStressMix(count);
  } else if (cmd.startsWith("stress")) {
    int count = STRESS_DEFAULT_COUNT;
    if (cmd.length() > 6U) {
      int32_t parsedCount = 0;
      if (!parseI32(cmd.substring(7), parsedCount)) {
        logWarn("Invalid count (1-%d)", MAX_STRESS_COUNT);
        return;
      }
      count = static_cast<int>(parsedCount);
    }
    if (count <= 0 || count > MAX_STRESS_COUNT) {
      logWarn("Invalid count (1-%d)", MAX_STRESS_COUNT);
      return;
    }
    if (!_device.isOnline()) {
      logWarn("Device not online. Run 'init' and 'wake' first.");
      return;
    }
    runStress(count);
  } else {
    logWarn("Unknown command: %s", cmd.c_str());
  }
}

}  // namespace ldc1614_cli
