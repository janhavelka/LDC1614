#include "Ldc1614IdfCli.h"

#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "I2cMasterTransport.h"

namespace {

constexpr uint64_t JOB_DEADLINE_MS = 2000;

bool parseUnsigned(const char* text, uint64_t maximum, uint64_t& value) {
  if (text == nullptr || *text == '\0') {
    return false;
  }
  char* end = nullptr;
  const unsigned long parsed = std::strtoul(text, &end, 0);
  if (end == text || *end != '\0' || parsed > maximum) {
    return false;
  }
  value = parsed;
  return true;
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

}  // namespace

Ldc1614IdfCli::Ldc1614IdfCli(LDC1614::LDC1614& device,
                             const LDC1614::Config& defaultConfig,
                             esp32_i2c::Context& transport,
                             I2cRecoverFn i2cRecover,
                             void* recoverUser)
    : _device(device),
      _defaultConfig(defaultConfig),
      _transport(transport),
      _i2cRecover(i2cRecover),
      _recoverUser(recoverUser) {}

void Ldc1614IdfCli::printBanner() const {
  std::printf("LDC1614 v%s native ESP-IDF diagnostic CLI\n", LDC1614::VERSION);
  std::printf("One owner; cooperative jobs advance by one transfer per service pass.\n");
}

void Ldc1614IdfCli::printPrompt() const { std::printf("> "); }

void Ldc1614IdfCli::println(const char* text) const {
  std::printf("%s\n", text != nullptr ? text : "");
}

void Ldc1614IdfCli::printStatus(const LDC1614::Status& status) const {
  std::printf("status: code=%u detail=%ld msg=%s\n",
              static_cast<unsigned>(status.code),
              static_cast<long>(status.detail),
              status.msg != nullptr ? status.msg : "");
}

LDC1614::OperationId Ldc1614IdfCli::nextOperationId() {
  const LDC1614::OperationId result = _nextOperationId++;
  if (_nextOperationId == 0) {
    _nextOperationId = 1;
  }
  return result;
}

void Ldc1614IdfCli::printHelp() const {
  println("jobs: init, apply, resetreapply, acquire/read [mask], cancel, progress");
  println("controls: status, ready, sleep, wake, initdrive <channel>");
  println("owner bus: busrecover (explicit reinit; then run init)");
  println("diagnostics: version, scan, probe/id, drv, cfg, reg, wreg, selftest");
  println("pure helpers: timing [mask], freq <channel> <raw28>");
  println("lifecycle: bind, invalidate, end");
}

void Ldc1614IdfCli::scanI2c() const {
  if (_device.jobProgress().active) {
    printStatus(LDC1614::Status::Error(
        LDC1614::Err::BUSY, "scan unavailable while job active"));
    return;
  }
  uint8_t found = 0;
  uint8_t probes = 0;
  for (uint8_t address = 0x08U; address <= 0x77U; ++address) {
    ++probes;
    const esp32_i2c::ProbeResult result = esp32_i2c::probe(
        _transport, address, _defaultConfig.i2cTimeoutMs);
    if (result == esp32_i2c::ProbeResult::ACK) {
      ++found;
      std::printf("I2C device at 0x%02X\n", address);
    } else if (result == esp32_i2c::ProbeResult::TIMEOUT) {
      std::printf("scan failed address=0x%02X\n", address);
      printStatus(LDC1614::Status::Error(
          LDC1614::Err::I2C_TIMEOUT, "I2C scan probe timed out", address));
      return;
    } else if (result == esp32_i2c::ProbeResult::ERROR) {
      std::printf("scan failed address=0x%02X\n", address);
      printStatus(LDC1614::Status::Error(
          LDC1614::Err::I2C_BUS, "I2C scan probe failed", address));
      return;
    }
  }
  std::printf("scan complete found=%u probes=%u\n",
              static_cast<unsigned>(found), static_cast<unsigned>(probes));
  printStatus(LDC1614::Status::Ok());
}

void Ldc1614IdfCli::recoverI2c() {
  if (_device.jobProgress().active) {
    printStatus(LDC1614::Status::Error(
        LDC1614::Err::BUSY, "bus recovery unavailable while job active"));
    return;
  }
  if (_i2cRecover == nullptr) {
    printStatus(LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG,
                                       "bus recovery unavailable"));
    return;
  }
  const LDC1614::Status status = _i2cRecover(_recoverUser);
  _device.invalidateAppliedState(
      status.ok()
          ? LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                                   "Owner reinitialized I2C bus")
          : status);
  printStatus(status);
}

void Ldc1614IdfCli::printDriver() const {
  const LDC1614::TransportStats stats = _device.transportStats();
  const LDC1614::JobProgress progress = _device.jobProgress();
  std::printf("bound=%u applied=%s revision=%lu active=%u result=%u\n",
              _device.isBound() ? 1U : 0U,
              appliedName(_device.appliedConfigState()),
              static_cast<unsigned long>(_device.configRevision()),
              progress.active ? 1U : 0U,
              _device.resultAvailable() ? 1U : 0U);
  std::printf("transport attempts=%lu success=%lu failures=%lu\n",
              static_cast<unsigned long>(stats.totalAttempts),
              static_cast<unsigned long>(stats.totalSuccess),
              static_cast<unsigned long>(stats.totalFailures));
  printStatus(stats.lastStatus);
}

void Ldc1614IdfCli::printConfig() const {
  if (!_device.isBound()) {
    println("not bound");
    return;
  }
  const LDC1614::Config& config = _device.config();
  const unsigned variantChannels =
      config.variant == LDC1614::DeviceVariant::LDC1612 ? 2U : 4U;
  std::printf("address=0x%02X variant=%u variantChannels=%u selected=0x%02X mode=%u refHz=%lu "
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

void Ldc1614IdfCli::printProgress() const {
  const LDC1614::JobProgress progress = _device.jobProgress();
  std::printf("active=%u operation=%" PRIu64 " kind=%u phase=%u transfers=%u/%u "
              "requested=0x%02X completed=0x%02X deadline=%" PRIu64 "\n",
              progress.active ? 1U : 0U, progress.operationId,
              static_cast<unsigned>(progress.kind),
              static_cast<unsigned>(progress.phase),
              static_cast<unsigned>(progress.completedTransfers),
              static_cast<unsigned>(progress.maximumTransfers),
              static_cast<unsigned>(progress.requestedChannels.bits),
              static_cast<unsigned>(progress.completedChannels.bits),
              progress.deadlineMs);
}

void Ldc1614IdfCli::printDeviceStatus(
    const LDC1614::DeviceStatus& status) const {
  std::printf("STATUS observed=%u raw=0x%04X drdy=%u unread=0x%02X errCh=%u "
              "UR=%u OR=%u WD=%u AH=%u AL=%u ZC=%u\n",
              status.observed ? 1U : 0U, status.raw,
              status.dataReady ? 1U : 0U,
              static_cast<unsigned>(status.unreadChannels.bits),
              static_cast<unsigned>(status.errorChannel),
              status.errorUnderRange ? 1U : 0U,
              status.errorOverRange ? 1U : 0U,
              status.errorWatchdog ? 1U : 0U,
              status.errorAmplitudeHigh ? 1U : 0U,
              status.errorAmplitudeLow ? 1U : 0U,
              status.errorZeroCount ? 1U : 0U);
}

void Ldc1614IdfCli::printResult(
    const LDC1614::OperationResult& result) const {
  std::printf("result operation=%" PRIu64 " kind=%u outcome=%s effects=0x%02X "
              "revision=%lu phase=%u reg=0x%02X channel=%u transfers=%u/%u\n",
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
    std::printf("configFault phase=%u reg=0x%02X channel=%u effects=0x%02X\n",
                static_cast<unsigned>(result.configFault.phase),
                result.configFault.registerAddress,
                static_cast<unsigned>(result.configFault.channel),
                static_cast<unsigned>(result.configFault.effects));
    printStatus(result.configFault.cause);
  }
  if (!result.hasSampleBatch) {
    return;
  }
  const LDC1614::SampleBatch& batch = result.sampleBatch;
  std::printf("batch selected=0x%02X valid=0x%02X fresh=0x%02X error=0x%02X "
              "overrun=0x%02X completedMs=%" PRIu64 "\n",
              static_cast<unsigned>(batch.selectedChannels.bits),
              static_cast<unsigned>(batch.validChannels.bits),
              static_cast<unsigned>(batch.freshChannels.bits),
              static_cast<unsigned>(batch.errorChannels.bits),
              static_cast<unsigned>(batch.overrunChannels.bits),
              batch.completedUptimeMs);
  printDeviceStatus(batch.statusBefore);
  for (uint8_t channel = 0; channel < 4; ++channel) {
    if ((batch.selectedChannels.bits & static_cast<uint8_t>(1U << channel)) == 0U) {
      continue;
    }
    const LDC1614::ChannelSample& sample = batch.channel[channel];
    std::printf("ch%u raw=0x%07lX quality=0x%04X\n",
                static_cast<unsigned>(channel),
                static_cast<unsigned long>(sample.rawCount28), sample.quality);
  }
}

void Ldc1614IdfCli::startInitialize(bool resetFirst, uint64_t nowMs) {
  const LDC1614::OperationId operationId = nextOperationId();
  const LDC1614::Status status =
      resetFirst
          ? _device.startResetAndReapply(operationId, nowMs + JOB_DEADLINE_MS)
          : _device.startInitialize(operationId, nowMs + JOB_DEADLINE_MS);
  std::printf("scheduled operation=%" PRIu64 " reset=%u\n", operationId,
              resetFirst ? 1U : 0U);
  printStatus(status);
}

void Ldc1614IdfCli::startAcquire(LDC1614::ChannelMask channels,
                                 uint64_t nowMs) {
  const LDC1614::OperationId operationId = nextOperationId();
  const LDC1614::Status status =
      _device.startAcquire(channels, operationId, nowMs + JOB_DEADLINE_MS);
  std::printf("scheduled acquire operation=%" PRIu64 " mask=0x%02X\n",
              operationId, static_cast<unsigned>(channels.bits));
  printStatus(status);
}

void Ldc1614IdfCli::service(uint64_t nowMs) {
  _lastNowMs = nowMs;
  if (_device.jobProgress().active) {
    const LDC1614::Status status = _device.poll(nowMs, 1);
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

void Ldc1614IdfCli::processLine(const char* line) {
  if (line == nullptr) {
    return;
  }
  char buffer[128];
  std::strncpy(buffer, line, sizeof(buffer) - 1U);
  buffer[sizeof(buffer) - 1U] = '\0';
  char* command = std::strtok(buffer, " \t");
  if (command == nullptr) {
    return;
  }

  if (std::strcmp(command, "help") == 0) {
    printHelp();
  } else if (std::strcmp(command, "version") == 0) {
    std::printf(
        "version: %s firmware_git=%s firmware_status=%s build_timestamp=%s\n",
        LDC1614::VERSION, LDC1614::GIT_COMMIT, LDC1614::GIT_STATUS,
        LDC1614::BUILD_TIMESTAMP);
  } else if (std::strcmp(command, "scan") == 0) {
    scanI2c();
  } else if (std::strcmp(command, "busrecover") == 0) {
    recoverI2c();
  } else if (std::strcmp(command, "bind") == 0) {
    printStatus(_device.bind(_defaultConfig));
  } else if (std::strcmp(command, "init") == 0 ||
             std::strcmp(command, "begin") == 0) {
    startInitialize(false, _lastNowMs);
  } else if (std::strcmp(command, "apply") == 0) {
    const LDC1614::OperationId operationId = nextOperationId();
    const LDC1614::Status status = _device.startApplyConfig(
        operationId, _lastNowMs + JOB_DEADLINE_MS);
    std::printf("scheduled operation=%" PRIu64 " kind=apply\n", operationId);
    printStatus(status);
  } else if (std::strcmp(command, "resetreapply") == 0) {
    startInitialize(true, _lastNowMs);
  } else if (std::strcmp(command, "cancel") == 0) {
    printStatus(_device.cancelJob());
  } else if (std::strcmp(command, "progress") == 0) {
    printProgress();
  } else if (std::strcmp(command, "drv") == 0 ||
             std::strcmp(command, "state") == 0) {
    printDriver();
  } else if (std::strcmp(command, "cfg") == 0 ||
             std::strcmp(command, "settings") == 0) {
    printConfig();
  } else if (std::strcmp(command, "status") == 0) {
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
    std::printf("ready=%u\n", ready ? 1U : 0U);
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
    startAcquire(LDC1614::ChannelMask{static_cast<uint8_t>(mask)}, _lastNowMs);
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
    std::printf("MANUFACTURER_ID=0x%04X DEVICE_ID=0x%04X\n", manufacturer,
                deviceId);
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
    std::printf("reg 0x%02lX = 0x%04X\n", static_cast<unsigned long>(reg), value);
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
  } else if (std::strcmp(command, "initdrive") == 0) {
    uint64_t channel = 0;
    if (!parseUnsigned(std::strtok(nullptr, " \t"), 3, channel)) {
      println("usage: initdrive <channel>");
      return;
    }
    uint8_t code = 0;
    const LDC1614::Status status = _device.readInitDriveCurrent(
        static_cast<LDC1614::Channel>(channel), code);
    std::printf("channel=%lu initDriveCode=%u\n",
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
    std::printf("wakeSettleUs=%" PRIu64 " conversionUs=%" PRIu64
                " sequentialFrameUs=%" PRIu64 " acquisitionTransfers=%u\n",
                timing.wakeAndSettleUs, timing.conversionUs,
                timing.sequentialFrameUs,
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
    std::printf("frequencyHz=%.6f\n", frequency);
    printStatus(status);
  } else if (std::strcmp(command, "invalidate") == 0) {
    _device.invalidateAppliedState(
        LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                               "Owner invalidated applied state"));
    printStatus(LDC1614::Status::Ok());
  } else if (std::strcmp(command, "end") == 0) {
    _device.end();
    printStatus(LDC1614::Status::Ok());
  } else {
    println("unknown command; type help");
  }
}
