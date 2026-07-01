#include "Ldc1614IdfCli.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "LDC1614/Version.h"

namespace {

constexpr size_t LINE_CAPACITY = 128;
constexpr int MAX_ARGS = 5;
constexpr float DEFAULT_REF_CLOCK_HZ = 43000000.0F;

bool equalsIgnoreCase(const char* lhs, const char* rhs) {
  if (lhs == nullptr || rhs == nullptr) {
    return false;
  }
  while (*lhs != '\0' && *rhs != '\0') {
    const unsigned char l = static_cast<unsigned char>(*lhs++);
    const unsigned char r = static_cast<unsigned char>(*rhs++);
    if (std::tolower(l) != std::tolower(r)) {
      return false;
    }
  }
  return *lhs == '\0' && *rhs == '\0';
}

int splitLine(char* line, char* argv[], int maxArgs) {
  int argc = 0;
  char* cursor = line;

  while (*cursor != '\0' && argc < maxArgs) {
    while (*cursor != '\0' &&
           std::isspace(static_cast<unsigned char>(*cursor)) != 0) {
      ++cursor;
    }
    if (*cursor == '\0') {
      break;
    }

    argv[argc++] = cursor;
    while (*cursor != '\0' &&
           std::isspace(static_cast<unsigned char>(*cursor)) == 0) {
      ++cursor;
    }
    if (*cursor != '\0') {
      *cursor++ = '\0';
    }
  }

  return argc;
}

bool parseUint8(const char* text, uint8_t maxValue, uint8_t& out) {
  if (text == nullptr || *text == '\0') {
    return false;
  }
  char* end = nullptr;
  const unsigned long value = std::strtoul(text, &end, 0);
  if (end == text || *end != '\0' || value > maxValue) {
    return false;
  }
  out = static_cast<uint8_t>(value);
  return true;
}

bool parseFloat(const char* text, float& out) {
  if (text == nullptr || *text == '\0') {
    return false;
  }
  char* end = nullptr;
  const float value = std::strtof(text, &end);
  if (end == text || *end != '\0' || value <= 0.0F) {
    return false;
  }
  out = value;
  return true;
}

const char* stateName(LDC1614::DriverState state) {
  switch (state) {
    case LDC1614::DriverState::UNINIT:
      return "UNINIT";
    case LDC1614::DriverState::READY:
      return "READY";
    case LDC1614::DriverState::DEGRADED:
      return "DEGRADED";
    case LDC1614::DriverState::OFFLINE:
      return "OFFLINE";
  }
  return "UNKNOWN";
}

void printStatusLine(const char* label, const LDC1614::Status& status) {
  std::printf("%s: code=%u detail=%ld msg=%s\n", label,
              static_cast<unsigned>(status.code), static_cast<long>(status.detail),
              status.msg != nullptr ? status.msg : "");
}

void printChannelData(uint8_t ch, const LDC1614::ChannelData& data) {
  std::printf("ch%u raw=0x%07lx errUR=%u errOR=%u errWD=%u errAmp=%u\n",
              static_cast<unsigned>(ch),
              static_cast<unsigned long>(data.rawData),
              data.errUnderRange ? 1U : 0U, data.errOverRange ? 1U : 0U,
              data.errWatchdog ? 1U : 0U, data.errAmplitude ? 1U : 0U);
}

}  // namespace

Ldc1614IdfCli::Ldc1614IdfCli(LDC1614::LDC1614& device,
                             const LDC1614::Config& defaultConfig)
    : _device(device), _defaultConfig(defaultConfig) {}

void Ldc1614IdfCli::printBanner() const {
  std::printf("\nLDC1614 ESP-IDF diagnostic bring-up CLI\n");
  std::printf("This example is not a production bus manager.\n");
  std::printf("Type 'help' for commands.\n");
}

void Ldc1614IdfCli::printPrompt() const {
  std::printf("ldc1614-idf> ");
  std::fflush(stdout);
}

void Ldc1614IdfCli::println(const char* text) const {
  std::printf("%s\n", text != nullptr ? text : "");
}

uint32_t Ldc1614IdfCli::nowMs() const {
  if (_defaultConfig.nowMs != nullptr) {
    return _defaultConfig.nowMs(_defaultConfig.timeUser);
  }
  return 0;
}

void Ldc1614IdfCli::processLine(const char* line) {
  char buffer[LINE_CAPACITY] = {};
  if (line != nullptr) {
    size_t i = 0;
    while (line[i] != '\0' && i < (sizeof(buffer) - 1U)) {
      buffer[i] = line[i];
      ++i;
    }
    buffer[i] = '\0';
  }

  char* argv[MAX_ARGS] = {};
  const int argc = splitLine(buffer, argv, MAX_ARGS);
  if (argc == 0) {
    return;
  }

  const char* cmd = argv[0];
  if (equalsIgnoreCase(cmd, "help") || equalsIgnoreCase(cmd, "?")) {
    printHelp();
  } else if (equalsIgnoreCase(cmd, "version")) {
    std::printf("version: %s\n", LDC1614_VERSION_STRING);
  } else if (equalsIgnoreCase(cmd, "begin") || equalsIgnoreCase(cmd, "init")) {
    handleBegin();
  } else if (equalsIgnoreCase(cmd, "probe")) {
    handleProbe();
  } else if (equalsIgnoreCase(cmd, "status")) {
    handleDeviceStatus();
  } else if (equalsIgnoreCase(cmd, "drv")) {
    printDriver();
  } else if (equalsIgnoreCase(cmd, "cfg")) {
    printConfig();
  } else if (equalsIgnoreCase(cmd, "read")) {
    handleRead(argc, argv);
  } else if (equalsIgnoreCase(cmd, "readall")) {
    handleReadAll();
  } else if (equalsIgnoreCase(cmd, "ready")) {
    handleReady();
  } else if (equalsIgnoreCase(cmd, "sleep")) {
    handleSleep();
  } else if (equalsIgnoreCase(cmd, "wake")) {
    handleWake();
  } else if (equalsIgnoreCase(cmd, "recover")) {
    handleRecover();
  } else if (equalsIgnoreCase(cmd, "timing")) {
    handleTiming(argc, argv);
  } else if (equalsIgnoreCase(cmd, "selftest")) {
    handleSelfTest();
  } else {
    std::printf("unknown command: %s\n", cmd);
  }
}

void Ldc1614IdfCli::printHelp() const {
  std::printf("commands:\n");
  std::printf("  help | ?              Show commands\n");
  std::printf("  version               Print library version\n");
  std::printf("  begin | init           Reinitialize with default example config\n");
  std::printf("  probe                 Read MANUFACTURER_ID and DEVICE_ID\n");
  std::printf("  status                Read device STATUS register\n");
  std::printf("  drv                   Print driver health snapshot\n");
  std::printf("  cfg                   Print cached configuration snapshot\n");
  std::printf("  read [ch]             Read one channel once\n");
  std::printf("  readall               Read configured channels once\n");
  std::printf("  ready                 Check data-ready status\n");
  std::printf("  sleep                 Enter device sleep mode\n");
  std::printf("  wake                  Wake and start conversions\n");
  std::printf("  recover               Run manual recovery ladder\n");
  std::printf("  timing [ch] [frefHz]  Print timing estimates\n");
  std::printf("  selftest              Probe and read safe diagnostic status\n");
}

void Ldc1614IdfCli::printStatus(const LDC1614::Status& status) const {
  printStatusLine("status", status);
}

void Ldc1614IdfCli::printDriver() const {
  const LDC1614::SettingsSnapshot snapshot = _device.settings();
  const LDC1614::Status dirty = _device.hardwareConfigDirtyError();
  std::printf("state=%s initialized=%u online=%u dirty=%u sleep=%u\n",
              stateName(_device.driverState()), _device.isInitialized() ? 1U : 0U,
              _device.isOnline() ? 1U : 0U,
              _device.hardwareConfigDirty() ? 1U : 0U,
              _device.isSleeping() ? 1U : 0U);
  std::printf("success=%lu failures=%lu consecutive=%u lastOkMs=%lu lastErrMs=%lu\n",
              static_cast<unsigned long>(_device.totalSuccess()),
              static_cast<unsigned long>(_device.totalFailures()),
              static_cast<unsigned>(_device.consecutiveFailures()),
              static_cast<unsigned long>(_device.lastOkMs()),
              static_cast<unsigned long>(_device.lastErrorMs()));
  printStatusLine("lastError", _device.lastError());
  if (snapshot.hardwareConfigDirty) {
    printStatusLine("dirtyCause", dirty);
  }
}

void Ldc1614IdfCli::printConfig() const {
  const LDC1614::SettingsSnapshot snapshot = _device.settings();
  std::printf("addr=0x%02x channels=%u timeoutMs=%lu autoscan=%u active=%u intb=%u\n",
              static_cast<unsigned>(snapshot.i2cAddress),
              static_cast<unsigned>(snapshot.channelCount),
              static_cast<unsigned long>(snapshot.i2cTimeoutMs),
              snapshot.autoScan ? 1U : 0U,
              static_cast<unsigned>(snapshot.activeChan),
              snapshot.intbEnabled ? 1U : 0U);
  for (uint8_t ch = 0; ch < snapshot.channelCount && ch < 4U; ++ch) {
    const LDC1614::ChannelConfig& cfg = snapshot.channel[ch];
    std::printf("ch%u rcount=0x%04x settle=0x%04x finDiv=%u frefDiv=%u offset=0x%04x idrive=%u\n",
                static_cast<unsigned>(ch), static_cast<unsigned>(cfg.rcount),
                static_cast<unsigned>(cfg.settleCount),
                static_cast<unsigned>(cfg.finDivider),
                static_cast<unsigned>(cfg.frefDivider),
                static_cast<unsigned>(cfg.offset), static_cast<unsigned>(cfg.idrive));
  }
}

void Ldc1614IdfCli::handleBegin() {
  _device.end();
  const LDC1614::Status status = _device.begin(_defaultConfig);
  printStatus(status);
}

void Ldc1614IdfCli::handleProbe() {
  const LDC1614::Status status = _device.probe();
  printStatus(status);
}

void Ldc1614IdfCli::handleDeviceStatus() {
  LDC1614::DeviceStatus deviceStatus;
  const LDC1614::Status status = _device.readDeviceStatus(deviceStatus);
  printStatus(status);
  if (status.ok()) {
    std::printf("raw=0x%04x drdy=%u err=%u errChan=%u ur=%u or=%u wd=%u ah=%u al=%u zc=%u\n",
                static_cast<unsigned>(deviceStatus.raw),
                deviceStatus.dataReady ? 1U : 0U,
                deviceStatus.hasError() ? 1U : 0U,
                static_cast<unsigned>(deviceStatus.errChan),
                deviceStatus.errUnderRange ? 1U : 0U,
                deviceStatus.errOverRange ? 1U : 0U,
                deviceStatus.errWatchdog ? 1U : 0U,
                deviceStatus.errAmplitudeHigh ? 1U : 0U,
                deviceStatus.errAmplitudeLow ? 1U : 0U,
                deviceStatus.errZeroCount ? 1U : 0U);
  }
}

void Ldc1614IdfCli::handleReady() {
  bool ready = false;
  const LDC1614::Status status = _device.readDataReady(ready);
  printStatus(status);
  if (status.ok()) {
    std::printf("ready=%u\n", ready ? 1U : 0U);
  }
}

void Ldc1614IdfCli::handleSleep() {
  printStatus(_device.sleep());
}

void Ldc1614IdfCli::handleWake() {
  printStatus(_device.wake());
}

void Ldc1614IdfCli::handleRead(int argc, char* argv[]) {
  uint8_t ch = 0;
  if (argc >= 2 && !parseUint8(argv[1], 3, ch)) {
    std::printf("invalid channel\n");
    return;
  }

  LDC1614::ChannelData data;
  const LDC1614::Status status = _device.readChannel(ch, data);
  printStatus(status);
  if (status.ok()) {
    printChannelData(ch, data);
  }
}

void Ldc1614IdfCli::handleReadAll() {
  LDC1614::ChannelData data[4] = {};
  const uint8_t count = _device.channelCount();
  const LDC1614::Status status = _device.readAllChannels(data, count);
  printStatus(status);
  if (status.ok()) {
    for (uint8_t ch = 0; ch < count && ch < 4U; ++ch) {
      printChannelData(ch, data[ch]);
    }
  }
}

void Ldc1614IdfCli::handleRecover() {
  const LDC1614::Status status = _device.recover();
  printStatus(status);
}

void Ldc1614IdfCli::handleTiming(int argc, char* argv[]) {
  uint8_t ch = 0;
  float fRef = DEFAULT_REF_CLOCK_HZ;
  if (argc >= 2 && !parseUint8(argv[1], 3, ch)) {
    std::printf("invalid channel\n");
    return;
  }
  if (argc >= 3 && !parseFloat(argv[2], fRef)) {
    std::printf("invalid reference clock\n");
    return;
  }

  const float convUs = _device.calcConversionTimeUs(ch, fRef);
  const float settleUs = _device.calcSettleTimeUs(ch, fRef);
  const float sampleUs = _device.calcSampleTimeUs(ch, fRef);
  std::printf("ch%u fRef=%.1fHz conversion=%.2fus settle=%.2fus sample=%.2fus\n",
              static_cast<unsigned>(ch), static_cast<double>(fRef), static_cast<double>(convUs),
              static_cast<double>(settleUs), static_cast<double>(sampleUs));
}

void Ldc1614IdfCli::handleSelfTest() {
  std::printf("selftest: probe\n");
  printStatus(_device.probe());
  if (!_device.isInitialized()) {
    std::printf("selftest: driver not initialized, skipping STATUS/ready reads\n");
    return;
  }
  std::printf("selftest: ready\n");
  handleReady();
  std::printf("selftest: status\n");
  handleDeviceStatus();
}
