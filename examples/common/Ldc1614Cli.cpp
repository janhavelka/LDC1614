#include "examples/common/Ldc1614Cli.h"

#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace ldc1614_cli {
namespace {

constexpr uint64_t JOB_DEADLINE_MS = 2000;

const char* outcomeName(LDC1614::TerminalOutcome outcome) {
  switch (outcome) {
    case LDC1614::TerminalOutcome::SUCCESS: return "SUCCESS";
    case LDC1614::TerminalOutcome::FAILED: return "FAILED";
    case LDC1614::TerminalOutcome::CANCELLED: return "CANCELLED";
    case LDC1614::TerminalOutcome::TIMED_OUT: return "TIMED_OUT";
    case LDC1614::TerminalOutcome::NONE: return "NONE";
  }
  return "UNKNOWN";
}

const char* appliedName(LDC1614::AppliedConfigState state) {
  switch (state) {
    case LDC1614::AppliedConfigState::UNKNOWN: return "UNKNOWN";
    case LDC1614::AppliedConfigState::APPLYING: return "APPLYING";
    case LDC1614::AppliedConfigState::APPLIED_SLEEPING: return "APPLIED_SLEEPING";
    case LDC1614::AppliedConfigState::APPLIED_ACTIVE: return "APPLIED_ACTIVE";
    case LDC1614::AppliedConfigState::DIRTY: return "DIRTY";
  }
  return "UNKNOWN";
}

bool parseUnsigned(const char* text, uint64_t maxValue, uint64_t& value) {
  if (text == nullptr || *text == '\0') {
    return false;
  }
  char* end = nullptr;
  const unsigned long parsed = std::strtoul(text, &end, 0);
  if (end == text || *end != '\0' || parsed > maxValue) {
    return false;
  }
  value = parsed;
  return true;
}

}  // namespace

Cli::Cli(LDC1614::LDC1614& device, Platform platform)
    : _device(device), _platform(platform) {}

LDC1614::Config Cli::makeDefaultConfig() const {
  return _platform.makeConfig != nullptr ? _platform.makeConfig(_platform.user)
                                         : LDC1614::Config{};
}

uint64_t Cli::nowMs() const {
  return _platform.nowMs != nullptr ? _platform.nowMs(_platform.user) : 0;
}

void Cli::vprintfToOutput(const char* fmt, va_list args) const {
  if (_platform.vprintf != nullptr) {
    _platform.vprintf(_platform.user, fmt, args);
  }
}

void Cli::printf(const char* fmt, ...) const {
  va_list args;
  va_start(args, fmt);
  vprintfToOutput(fmt, args);
  va_end(args);
}

void Cli::println(const char* text) const { printf("%s\n", text); }

void Cli::logError(const char* fmt, ...) const {
  printf("ERROR: ");
  va_list args;
  va_start(args, fmt);
  vprintfToOutput(fmt, args);
  va_end(args);
  println();
}

void Cli::logInfo(const char* fmt, ...) const {
  va_list args;
  va_start(args, fmt);
  vprintfToOutput(fmt, args);
  va_end(args);
  println();
}

void Cli::printPrompt() const { printf("> "); }

void Cli::printStatus(const LDC1614::Status& status) const {
  printf("status: code=%u detail=%ld msg=%s\n",
         static_cast<unsigned>(status.code), static_cast<long>(status.detail),
         status.msg != nullptr ? status.msg : "");
}

LDC1614::OperationId Cli::nextOperationId() {
  const LDC1614::OperationId result = _nextOperationId++;
  if (_nextOperationId == 0) {
    _nextOperationId = 1;
  }
  return result;
}

uint64_t Cli::deadlineFromNow() const { return nowMs() + JOB_DEADLINE_MS; }

void Cli::startInitialize(bool resetFirst) {
  const LDC1614::OperationId id = nextOperationId();
  const LDC1614::Status status =
      resetFirst ? _device.startResetAndReapply(id, deadlineFromNow())
                 : _device.startInitialize(id, deadlineFromNow());
  printf("scheduled operation=%" PRIu64 " reset=%u\n", id,
         resetFirst ? 1U : 0U);
  printStatus(status);
}

void Cli::startAcquire(LDC1614::ChannelMask channels) {
  const LDC1614::OperationId id = nextOperationId();
  const LDC1614::Status status =
      _device.startAcquire(channels, id, deadlineFromNow());
  printf("scheduled acquire operation=%" PRIu64 " mask=0x%02X\n", id,
         static_cast<unsigned>(channels.bits));
  printStatus(status);
}

void Cli::printDeviceStatus(const LDC1614::DeviceStatus& status) const {
  printf("STATUS observed=%u raw=0x%04X drdy=%u unread=0x%02X errCh=%u "
         "UR=%u OR=%u WD=%u AH=%u AL=%u ZC=%u\n",
         status.observed ? 1U : 0U, status.raw, status.dataReady ? 1U : 0U,
         static_cast<unsigned>(status.unreadChannels.bits),
         static_cast<unsigned>(status.errorChannel),
         status.errorUnderRange ? 1U : 0U,
         status.errorOverRange ? 1U : 0U,
         status.errorWatchdog ? 1U : 0U,
         status.errorAmplitudeHigh ? 1U : 0U,
         status.errorAmplitudeLow ? 1U : 0U,
         status.errorZeroCount ? 1U : 0U);
}

void Cli::printBatch(const LDC1614::SampleBatch& batch) const {
  printf("batch selected=0x%02X valid=0x%02X fresh=0x%02X error=0x%02X "
         "overrun=0x%02X configRevision=%lu completedMs=%" PRIu64 "\n",
         static_cast<unsigned>(batch.selectedChannels.bits),
         static_cast<unsigned>(batch.validChannels.bits),
         static_cast<unsigned>(batch.freshChannels.bits),
         static_cast<unsigned>(batch.errorChannels.bits),
         static_cast<unsigned>(batch.overrunChannels.bits),
         static_cast<unsigned long>(batch.configRevision),
         batch.completedUptimeMs);
  printDeviceStatus(batch.statusBefore);
  for (uint8_t channel = 0; channel < 4; ++channel) {
    if ((batch.selectedChannels.bits & static_cast<uint8_t>(1U << channel)) == 0U) {
      continue;
    }
    const LDC1614::ChannelSample& sample = batch.channel[channel];
    printf("ch%u raw=0x%07lX quality=0x%04X msb=0x%04X lsb=0x%04X\n",
           static_cast<unsigned>(channel),
           static_cast<unsigned long>(sample.rawCount28), sample.quality,
           sample.rawDataMsb, sample.rawDataLsb);
  }
}

void Cli::printResult(const LDC1614::OperationResult& result) const {
  printf("result operation=%" PRIu64 " kind=%u outcome=%s effects=0x%02X "
         "configRevision=%lu phase=%u reg=0x%02X channel=%u transfers=%u/%u\n",
         result.operationId, static_cast<unsigned>(result.kind),
         outcomeName(result.outcome), static_cast<unsigned>(result.effects),
         static_cast<unsigned long>(result.configRevision),
         static_cast<unsigned>(result.finalProgress.phase),
         result.finalProgress.registerAddress,
         static_cast<unsigned>(result.finalProgress.channel),
         static_cast<unsigned>(result.finalProgress.completedTransfers),
         static_cast<unsigned>(result.finalProgress.maximumTransfers));
  printStatus(result.status);
  if (result.configFault.valid) {
    printf("configFault phase=%u reg=0x%02X channel=%u effects=0x%02X\n",
           static_cast<unsigned>(result.configFault.phase),
           result.configFault.registerAddress,
           static_cast<unsigned>(result.configFault.channel),
           static_cast<unsigned>(result.configFault.effects));
    printStatus(result.configFault.cause);
  }
  if (result.hasSampleBatch) {
    printBatch(result.sampleBatch);
  }
}

void Cli::service() {
  const LDC1614::JobProgress progress = _device.jobProgress();
  if (progress.active) {
    const LDC1614::Status status = _device.poll(nowMs(), 1);
    if (!status.ok() && !status.inProgress()) {
      printStatus(status);
    }
  }

  LDC1614::OperationResult result;
  while (_device.resultAvailable()) {
    const LDC1614::Status status = _device.takeResult(result);
    if (!status.ok()) {
      printStatus(status);
      break;
    }
    printResult(result);
  }
}

void Cli::printDriver() const {
  const LDC1614::JobProgress progress = _device.jobProgress();
  const LDC1614::TransportStats stats = _device.transportStats();
  printf("bound=%u applied=%s configRevision=%lu active=%u resultAvailable=%u\n",
         _device.isBound() ? 1U : 0U,
         appliedName(_device.appliedConfigState()),
         static_cast<unsigned long>(_device.configRevision()),
         progress.active ? 1U : 0U, _device.resultAvailable() ? 1U : 0U);
  printf("transport attempts=%lu success=%lu failures=%lu\n",
         static_cast<unsigned long>(stats.totalAttempts),
         static_cast<unsigned long>(stats.totalSuccess),
         static_cast<unsigned long>(stats.totalFailures));
  printStatus(stats.lastStatus);
}

void Cli::printConfig() const {
  if (!_device.isBound()) {
    println("not bound");
    return;
  }
  const LDC1614::Config& config = _device.config();
  const unsigned variantChannels =
      config.variant == LDC1614::DeviceVariant::LDC1612 ? 2U : 4U;
  printf("address=0x%02X variant=%u variantChannels=%u selected=0x%02X mode=%u refHz=%lu "
         "tolerancePpm=%lu timeoutMs=%lu\n",
         static_cast<unsigned>(config.i2cAddress),
         static_cast<unsigned>(config.variant),
         variantChannels,
         static_cast<unsigned>(config.channels.bits),
         static_cast<unsigned>(config.mode),
         static_cast<unsigned long>(config.referenceClock.frequencyHz),
         static_cast<unsigned long>(config.referenceClock.tolerancePpm),
         static_cast<unsigned long>(config.i2cTimeoutMs));
}

void Cli::printProgress() const {
  const LDC1614::JobProgress progress = _device.jobProgress();
  printf("active=%u operation=%" PRIu64 " kind=%u phase=%u transfers=%u/%u "
         "requested=0x%02X completed=0x%02X deadlineMs=%" PRIu64 "\n",
         progress.active ? 1U : 0U, progress.operationId,
         static_cast<unsigned>(progress.kind),
         static_cast<unsigned>(progress.phase),
         static_cast<unsigned>(progress.completedTransfers),
         static_cast<unsigned>(progress.maximumTransfers),
         static_cast<unsigned>(progress.requestedChannels.bits),
         static_cast<unsigned>(progress.completedChannels.bits),
         progress.deadlineMs);
}

void Cli::scanI2c() const {
  if (_platform.i2cProbe == nullptr) {
    println("scan unavailable");
    return;
  }
  for (uint8_t address = 1; address < 0x7F; ++address) {
    const I2cProbeResult result =
        _platform.i2cProbe(address, _platform.scanTimeoutMs, _platform.user);
    if (result == I2cProbeResult::ACK) {
      printf("I2C device at 0x%02X\n", address);
    }
  }
  printStatus(LDC1614::Status::Ok());
}

void Cli::printHelp() const {
  println("Owner-driven jobs: init, apply, resetreapply, acquire [mask], cancel, progress");
  println("One-transfer controls: status, ready/drdy, sleep, wake, initdrive <ch>");
  println("Diagnostics: version, scan, probe/id, drv/state, cfg/settings, reg, wreg");
  println("Pure helpers: timing [mask], freq <ch> <raw28>");
  println("Lifecycle: bind, invalidate, end; results print automatically from service()");
}

void Cli::processCommand(const char* commandLine) {
  if (commandLine == nullptr) {
    return;
  }
  char buffer[128];
  std::strncpy(buffer, commandLine, sizeof(buffer) - 1U);
  buffer[sizeof(buffer) - 1U] = '\0';
  char* command = std::strtok(buffer, " \t");
  if (command == nullptr) {
    return;
  }

  if (std::strcmp(command, "help") == 0) {
    printHelp();
  } else if (std::strcmp(command, "version") == 0) {
    printf("version: %s\n", LDC1614::VERSION);
  } else if (std::strcmp(command, "scan") == 0) {
    scanI2c();
  } else if (std::strcmp(command, "bind") == 0) {
    printStatus(_device.bind(makeDefaultConfig()));
  } else if (std::strcmp(command, "init") == 0 ||
             std::strcmp(command, "begin") == 0) {
    startInitialize(false);
  } else if (std::strcmp(command, "apply") == 0 ||
             std::strcmp(command, "sync") == 0) {
    const LDC1614::OperationId id = nextOperationId();
    printStatus(_device.startApplyConfig(id, deadlineFromNow()));
  } else if (std::strcmp(command, "resetreapply") == 0) {
    startInitialize(true);
  } else if (std::strcmp(command, "cancel") == 0) {
    printStatus(_device.cancelJob());
  } else if (std::strcmp(command, "progress") == 0) {
    printProgress();
  } else if (std::strcmp(command, "drv") == 0 ||
             std::strcmp(command, "state") == 0 ||
             std::strcmp(command, "health") == 0) {
    printDriver();
  } else if (std::strcmp(command, "cfg") == 0 ||
             std::strcmp(command, "settings") == 0) {
    printConfig();
  } else if (std::strcmp(command, "invalidate") == 0) {
    _device.invalidateAppliedState(
        LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                               "Owner invalidated applied state"));
    printStatus(LDC1614::Status::Ok());
  } else if (std::strcmp(command, "end") == 0) {
    _device.end();
    printStatus(LDC1614::Status::Ok());
  } else if (std::strcmp(command, "status") == 0 ||
             std::strcmp(command, "status_raw") == 0) {
    LDC1614::DeviceStatus status;
    const LDC1614::Status result = _device.readDeviceStatus(status);
    if (result.ok()) {
      printDeviceStatus(status);
    }
    printStatus(result);
  } else if (std::strcmp(command, "ready") == 0 ||
             std::strcmp(command, "drdy") == 0) {
    bool ready = false;
    LDC1614::DeviceStatus observed;
    const LDC1614::Status status = _device.readDataReady(ready, observed);
    printf("ready=%u\n", ready ? 1U : 0U);
    if (status.ok()) {
      printDeviceStatus(observed);
    }
    printStatus(status);
  } else if (std::strcmp(command, "sleep") == 0) {
    printStatus(_device.sleep());
  } else if (std::strcmp(command, "wake") == 0) {
    printStatus(_device.wake());
  } else if (std::strcmp(command, "acquire") == 0 ||
             std::strcmp(command, "read") == 0 ||
             std::strcmp(command, "readall") == 0) {
    uint64_t mask = _device.isBound() ? _device.config().channels.bits : 0;
    const char* argument = std::strtok(nullptr, " \t");
    if (argument != nullptr && !parseUnsigned(argument, 0x0F, mask)) {
      println("invalid channel mask");
      return;
    }
    startAcquire(LDC1614::ChannelMask{static_cast<uint8_t>(mask)});
  } else if (std::strcmp(command, "probe") == 0 ||
             std::strcmp(command, "id") == 0 ||
             std::strcmp(command, "selftest") == 0) {
    uint16_t manufacturer = 0;
    uint16_t deviceId = 0;
    LDC1614::Status status =
        _device.readRegister16(LDC1614::cmd::REG_MANUFACTURER_ID, manufacturer);
    if (status.ok()) {
      status = _device.readRegister16(LDC1614::cmd::REG_DEVICE_ID, deviceId);
    }
    printf("MANUFACTURER_ID=0x%04X DEVICE_ID=0x%04X\n", manufacturer, deviceId);
    if (status.ok() && (manufacturer != LDC1614::cmd::MANUFACTURER_ID_VALUE ||
                        deviceId != LDC1614::cmd::DEVICE_ID_VALUE)) {
      status = LDC1614::Status::Error(LDC1614::Err::DEVICE_NOT_FOUND,
                                      "Identity mismatch");
    }
    printStatus(status);
  } else if (std::strcmp(command, "reg") == 0) {
    uint64_t reg = 0;
    if (!parseUnsigned(std::strtok(nullptr, " \t"), 0x7F, reg)) {
      println("usage: reg <address>");
      return;
    }
    uint16_t value = 0;
    const LDC1614::Status status =
        _device.readRegister16(static_cast<uint8_t>(reg), value);
    printf("reg 0x%02lX = 0x%04X\n", static_cast<unsigned long>(reg), value);
    printStatus(status);
  } else if (std::strcmp(command, "wreg") == 0) {
    uint64_t reg = 0;
    uint64_t value = 0;
    if (!parseUnsigned(std::strtok(nullptr, " \t"), 0x7F, reg) ||
        !parseUnsigned(std::strtok(nullptr, " \t"), 0xFFFF, value)) {
      println("usage: wreg <address> <value>");
      return;
    }
    printStatus(_device.writeRegister16(static_cast<uint8_t>(reg),
                                        static_cast<uint16_t>(value)));
  } else if (std::strcmp(command, "initdrive") == 0 ||
             std::strcmp(command, "initidrive") == 0) {
    uint64_t channel = 0;
    if (!parseUnsigned(std::strtok(nullptr, " \t"), 3, channel)) {
      println("usage: initdrive <channel>");
      return;
    }
    uint8_t code = 0;
    const LDC1614::Status status = _device.readInitDriveCurrent(
        static_cast<LDC1614::Channel>(channel), code);
    printf("channel=%lu initDriveCode=%u\n",
           static_cast<unsigned long>(channel), static_cast<unsigned>(code));
    printStatus(status);
  } else if (std::strcmp(command, "timing") == 0) {
    uint64_t mask = _device.isBound() ? _device.config().channels.bits : 0;
    const char* argument = std::strtok(nullptr, " \t");
    if (argument != nullptr && !parseUnsigned(argument, 0x0F, mask)) {
      println("usage: timing [channel-mask]");
      return;
    }
    LDC1614::FrameTiming timing;
    const LDC1614::Status status = LDC1614::LDC1614::estimateFrameTiming(
        _device.config(), LDC1614::ChannelMask{static_cast<uint8_t>(mask)}, timing);
    printf("wakeSettleUs=%" PRIu64 " conversionUs=%" PRIu64
           " sequentialFrameUs=%" PRIu64 " acquisitionTransfers=%u\n",
           timing.wakeAndSettleUs, timing.conversionUs, timing.sequentialFrameUs,
           static_cast<unsigned>(timing.acquisitionTransfers));
    printStatus(status);
  } else if (std::strcmp(command, "freq") == 0) {
    uint64_t channel = 0;
    uint64_t raw = 0;
    if (!parseUnsigned(std::strtok(nullptr, " \t"), 3, channel) ||
        !parseUnsigned(std::strtok(nullptr, " \t"), 0x0FFFFFFF, raw)) {
      println("usage: freq <channel> <raw28>");
      return;
    }
    double frequency = 0.0;
    const LDC1614::Status status =
        LDC1614::LDC1614::calculateSensorFrequencyHz(
            _device.config(), static_cast<LDC1614::Channel>(channel),
            static_cast<uint32_t>(raw), frequency);
    printf("frequencyHz=%.6f\n", frequency);
    printStatus(status);
  } else {
    println("unknown command; type help");
  }
}

}  // namespace ldc1614_cli
