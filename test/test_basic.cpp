/// @file test_basic.cpp
/// @brief Native contract tests for LDC1614 lifecycle and health behavior.

#include <unity.h>

#include "Arduino.h"
#include "Wire.h"

#include <limits>
#include <type_traits>

SerialClass Serial;
TwoWire Wire;

#define private public
#include "LDC1614/LDC1614.h"
#undef private

using namespace LDC1614;

namespace {

// ============================================================================
// FakeBus — register-aware I2C mock
// ============================================================================

struct FakeBus {
  Status writeStatus = Status::Ok();
  Status readStatus = Status::Ok();
  uint32_t nowMs = 1234;
  uint32_t writeCalls = 0;
  uint32_t readCalls = 0;
  uint32_t yieldCalls = 0;
  uint16_t reg[128] = {};
  uint8_t lastWriteReg = 0;
  uint16_t lastWriteValue = 0;
  uint32_t failWriteCall = 0;
  uint8_t failWriteReg = 0xFF;
  Status failWriteStatus = Status::Error(Err::I2C_ERROR, "forced write failure", -42);
  struct WriteEvent {
    uint8_t reg = 0;
    uint16_t value = 0;
  };
  WriteEvent writeLog[96] = {};
  uint8_t writeLogCount = 0;
  bool gpioLevel = true;
  bool busResetCalled = false;

  FakeBus() {
    reg[cmd::REG_MANUFACTURER_ID] = cmd::MANUFACTURER_ID_VALUE;
    reg[cmd::REG_DEVICE_ID] = cmd::DEVICE_ID_VALUE;
  }
};

/// @brief Write callback that simply counts calls.
Status fakeWrite(uint8_t, const uint8_t* data, size_t len, uint32_t, void* user) {
  FakeBus* bus = static_cast<FakeBus*>(user);
  bus->writeCalls++;
  if (!bus->writeStatus.ok()) {
    return bus->writeStatus;
  }
  if (data == nullptr || len != 3) {
    return Status::Error(Err::INVALID_PARAM, "invalid fake write");
  }
  bus->lastWriteReg = data[0];
  bus->lastWriteValue = (static_cast<uint16_t>(data[1]) << 8) | data[2];
  if (bus->writeLogCount < 96) {
    bus->writeLog[bus->writeLogCount].reg = bus->lastWriteReg;
    bus->writeLog[bus->writeLogCount].value = bus->lastWriteValue;
    bus->writeLogCount++;
  }
  if ((bus->failWriteCall != 0 && bus->writeCalls == bus->failWriteCall) ||
      (bus->failWriteReg != 0xFF && bus->lastWriteReg == bus->failWriteReg)) {
    return bus->failWriteStatus;
  }
  if (data[0] < 128) {
    bus->reg[data[0]] = bus->lastWriteValue;
  }
  return Status::Ok();
}

/// @brief Read callback that returns MANUFACTURER_ID or DEVICE_ID when addressed.
Status fakeWriteRead(uint8_t, const uint8_t* txData, size_t txLen, uint8_t* rxData,
                     size_t rxLen, uint32_t, void* user) {
  FakeBus* bus = static_cast<FakeBus*>(user);
  bus->readCalls++;
  if (!bus->readStatus.ok()) {
    return bus->readStatus;
  }
  if (txData == nullptr || txLen == 0 || (rxLen > 0 && rxData == nullptr)) {
    return Status::Error(Err::INVALID_PARAM, "invalid fake I2C buffers");
  }

  uint8_t regAddr = txData[0];
  uint16_t val = 0x0000;
  if (regAddr < 128) {
    val = bus->reg[regAddr];
  }

  if (rxLen >= 1) {
    rxData[0] = static_cast<uint8_t>((val >> 8) & 0xFF);
  }
  if (rxLen >= 2) {
    rxData[1] = static_cast<uint8_t>(val & 0xFF);
  }
  for (size_t i = 2; i < rxLen; ++i) {
    rxData[i] = 0;
  }
  return Status::Ok();
}

uint32_t fakeNowMs(void* user) {
  return static_cast<FakeBus*>(user)->nowMs;
}

bool fakeGpioRead(int, void* user) {
  return static_cast<FakeBus*>(user)->gpioLevel;
}

void fakeYield(void* user) {
  static_cast<FakeBus*>(user)->yieldCalls++;
}

Config makeConfig(FakeBus& bus) {
  Config cfg;
  cfg.i2cWrite = fakeWrite;
  cfg.i2cWriteRead = fakeWriteRead;
  cfg.i2cUser = &bus;
  cfg.nowMs = fakeNowMs;
  cfg.cooperativeYield = fakeYield;
  cfg.timeUser = &bus;
  cfg.i2cAddress = 0x2A;
  cfg.i2cTimeoutMs = 10;
  cfg.channelCount = 4;
  cfg.offlineThreshold = 3;

  // Minimal valid per-channel config
  for (int i = 0; i < 4; i++) {
    cfg.channel[i].rcount = 0x0080;
    cfg.channel[i].settleCount = 0;
    cfg.channel[i].finDivider = 1;
    cfg.channel[i].frefDivider = 1;
    cfg.channel[i].offset = 0;
    cfg.channel[i].idrive = 0;
  }

  return cfg;
}

void resetIoCounters(FakeBus& bus) {
  bus.writeCalls = 0;
  bus.readCalls = 0;
  bus.yieldCalls = 0;
  bus.lastWriteReg = 0;
  bus.lastWriteValue = 0;
  bus.writeLogCount = 0;
  bus.failWriteCall = 0;
  bus.failWriteReg = 0xFF;
  bus.writeStatus = Status::Ok();
  bus.readStatus = Status::Ok();
}

void failNthWrite(FakeBus& bus, uint32_t call, Status st) {
  bus.failWriteCall = call;
  bus.failWriteReg = 0xFF;
  bus.failWriteStatus = st;
}

void failWriteToReg(FakeBus& bus, uint8_t reg, Status st) {
  bus.failWriteCall = 0;
  bus.failWriteReg = reg;
  bus.failWriteStatus = st;
}

uint8_t dirtyDetailPhase(const LDC1614::LDC1614& dev) {
  return static_cast<uint8_t>((static_cast<uint32_t>(dev.hardwareConfigDirtyError().detail) >> 24) & 0xFFU);
}

uint8_t dirtyDetailReg(const LDC1614::LDC1614& dev) {
  return static_cast<uint8_t>((static_cast<uint32_t>(dev.hardwareConfigDirtyError().detail) >> 16) & 0xFFU);
}

uint8_t dirtyDetailIndex(const LDC1614::LDC1614& dev) {
  return static_cast<uint8_t>((static_cast<uint32_t>(dev.hardwareConfigDirtyError().detail) >> 8) & 0xFFU);
}

} // namespace

void setUp() {}
void tearDown() {}

// ============================================================================
// Status Tests
// ============================================================================

void test_status_ok() {
  Status st = Status::Ok();
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::OK), static_cast<uint8_t>(st.code));
}

void test_status_error() {
  Status st = Status::Error(Err::I2C_ERROR, "Test error", 42);
  TEST_ASSERT_FALSE(st.ok());
  TEST_ASSERT_TRUE(st.is(Err::I2C_ERROR));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::I2C_ERROR), static_cast<uint8_t>(st.code));
  TEST_ASSERT_EQUAL_INT32(42, st.detail);
}

void test_status_in_progress() {
  Status st{Err::IN_PROGRESS, 0, "In progress"};
  TEST_ASSERT_FALSE(st.ok());
  TEST_ASSERT_TRUE(st.inProgress());
}

void test_type_traits_delete_copy_and_move() {
  static_assert(!std::is_copy_constructible<LDC1614::LDC1614>::value,
                "LDC1614 must not be copy constructible");
  static_assert(!std::is_copy_assignable<LDC1614::LDC1614>::value,
                "LDC1614 must not be copy assignable");
  static_assert(!std::is_move_constructible<LDC1614::LDC1614>::value,
                "LDC1614 must not be move constructible");
  static_assert(!std::is_move_assignable<LDC1614::LDC1614>::value,
                "LDC1614 must not be move assignable");
  TEST_ASSERT_TRUE(true);
}

// ============================================================================
// Config Defaults Tests
// ============================================================================

void test_config_defaults() {
  Config cfg;
  TEST_ASSERT_NULL(cfg.i2cWrite);
  TEST_ASSERT_NULL(cfg.i2cWriteRead);
  TEST_ASSERT_EQUAL_HEX8(0x2A, cfg.i2cAddress);
  TEST_ASSERT_EQUAL_UINT16(50, cfg.i2cTimeoutMs);
  TEST_ASSERT_EQUAL_UINT8(4, cfg.channelCount);
  TEST_ASSERT_FALSE(cfg.autoScan);
  TEST_ASSERT_EQUAL_UINT8(0, cfg.activeChan);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Deglitch::BW_33MHZ),
                          static_cast<uint8_t>(cfg.deglitch));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RefClkSrc::INTERNAL),
                          static_cast<uint8_t>(cfg.refClkSrc));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(SensorActivation::LOW_POWER),
                          static_cast<uint8_t>(cfg.sensorActivation));
  TEST_ASSERT_TRUE(cfg.rpOverrideEn);
  TEST_ASSERT_TRUE(cfg.autoAmpDis);
  TEST_ASSERT_FALSE(cfg.highCurrentDrv);
  TEST_ASSERT_EQUAL_INT(-1, cfg.intbPin);
  TEST_ASSERT_EQUAL_UINT8(5, cfg.offlineThreshold);
}

void test_channel_config_defaults() {
  ChannelConfig cc;
  TEST_ASSERT_EQUAL_UINT16(0x0080, cc.rcount);
  TEST_ASSERT_EQUAL_UINT16(0x0000, cc.settleCount);
  TEST_ASSERT_EQUAL_UINT8(1, cc.finDivider);
  TEST_ASSERT_EQUAL_UINT16(1, cc.frefDivider);
  TEST_ASSERT_EQUAL_UINT16(0x0000, cc.offset);
  TEST_ASSERT_EQUAL_UINT8(0, cc.idrive);
}

void test_deglitch_enum_matches_datasheet_mux_values() {
  TEST_ASSERT_EQUAL_UINT8(1u, static_cast<uint8_t>(Deglitch::BW_1MHZ));
  TEST_ASSERT_EQUAL_UINT8(4u, static_cast<uint8_t>(Deglitch::BW_3MHZ));
  TEST_ASSERT_EQUAL_UINT8(5u, static_cast<uint8_t>(Deglitch::BW_10MHZ));
  TEST_ASSERT_EQUAL_UINT8(7u, static_cast<uint8_t>(Deglitch::BW_33MHZ));
}

// ============================================================================
// begin() Tests
// ============================================================================

void test_begin_rejects_missing_callbacks() {
  LDC1614::LDC1614 dev;
  Config cfg;
  Status st = dev.begin(cfg);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_CONFIG),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::UNINIT),
                          static_cast<uint8_t>(dev.state()));
}

void test_begin_rejects_bad_address() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.i2cAddress = 0x50;  // Not 0x2A or 0x2B

  LDC1614::LDC1614 dev;
  Status st = dev.begin(cfg);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_CONFIG),
                          static_cast<uint8_t>(st.code));
}

void test_begin_rejects_bad_channel_count() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.channelCount = 3;

  LDC1614::LDC1614 dev;
  Status st = dev.begin(cfg);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_CONFIG),
                          static_cast<uint8_t>(st.code));
}

void test_begin_rejects_ldc1612_invalid_rr_sequence() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.channelCount = 2;
  cfg.autoScan = true;
  cfg.rrSequence = RRSequence::CH0_CH1_CH2;

  LDC1614::LDC1614 dev;
  Status st = dev.begin(cfg);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_CONFIG),
                          static_cast<uint8_t>(st.code));
}

void test_begin_rejects_reserved_error_config_bits() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.errorConfig = 0x0002;  // Reserved bit

  LDC1614::LDC1614 dev;
  Status st = dev.begin(cfg);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_CONFIG),
                          static_cast<uint8_t>(st.code));
}

void test_begin_rejects_rcount_below_minimum() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.channel[0].rcount = 0x0004;  // Below RCOUNT_MIN (0x0005)

  LDC1614::LDC1614 dev;
  Status st = dev.begin(cfg);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_CONFIG),
                          static_cast<uint8_t>(st.code));
}

void test_begin_rejects_idrive_above_maximum() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.channel[0].idrive = 32;  // Above IDRIVE_MAX (31)

  LDC1614::LDC1614 dev;
  Status st = dev.begin(cfg);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_CONFIG),
                          static_cast<uint8_t>(st.code));
}

void test_invalid_begin_resets_runtime_and_default_config() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  Config good = makeConfig(bus);
  good.i2cAddress = 0x2B;
  TEST_ASSERT_TRUE(dev.begin(good).ok());

  bus.readStatus = Status::Error(Err::TIMEOUT, "forced recover timeout", -9);
  (void)dev.recover();
  TEST_ASSERT_GREATER_THAN_UINT32(0u, dev.totalFailures());

  Config bad = makeConfig(bus);
  bad.i2cTimeoutMs = 0;
  Status st = dev.begin(bad);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_CONFIG),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_FALSE(dev.isInitialized());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::UNINIT),
                          static_cast<uint8_t>(dev.state()));
  TEST_ASSERT_NULL(dev.getConfig().i2cWrite);
  TEST_ASSERT_NULL(dev.getConfig().i2cWriteRead);
  TEST_ASSERT_EQUAL_HEX8(0x2A, dev.getConfig().i2cAddress);
  TEST_ASSERT_EQUAL_UINT8(5u, dev.getConfig().offlineThreshold);
  TEST_ASSERT_EQUAL_UINT16(0x0080u, dev.getConfig().channel[0].rcount);
  TEST_ASSERT_EQUAL_UINT32(0u, dev.totalSuccess());
  TEST_ASSERT_EQUAL_UINT32(0u, dev.totalFailures());
  TEST_ASSERT_EQUAL_UINT8(0u, dev.consecutiveFailures());
  TEST_ASSERT_EQUAL_UINT32(0u, dev.lastOkMs());
  TEST_ASSERT_EQUAL_UINT32(0u, dev.lastErrorMs());
}

void test_begin_normalizes_offline_threshold_on_stored_copy() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  Config cfg = makeConfig(bus);
  cfg.offlineThreshold = 0;

  Status st = dev.begin(cfg);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_EQUAL_UINT8(0u, cfg.offlineThreshold);
  TEST_ASSERT_EQUAL_UINT8(1u, dev.getConfig().offlineThreshold);
}

void test_begin_success_sets_ready() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  Status st = dev.begin(makeConfig(bus));
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::READY),
                          static_cast<uint8_t>(dev.state()));
  TEST_ASSERT_TRUE(dev.isOnline());
  TEST_ASSERT_TRUE(dev.isSleeping());   // Device stays in sleep after begin()
  TEST_ASSERT_EQUAL_UINT32(0u, dev.totalSuccess());
  TEST_ASSERT_EQUAL_UINT32(0u, dev.totalFailures());
  TEST_ASSERT_EQUAL_UINT8(0u, dev.consecutiveFailures());
  TEST_ASSERT_EQUAL_UINT32(0u, dev.lastOkMs());
}

void test_begin_with_2channels() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.channelCount = 2;

  LDC1614::LDC1614 dev;
  Status st = dev.begin(cfg);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_EQUAL_UINT32(0u, dev.totalSuccess());
}

void test_begin_applyConfig_partial_failure_sets_dirty_with_register_detail() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  const Status forced = Status::Error(Err::I2C_ERROR, "forced apply failure", -42);
  failWriteToReg(bus, cmd::regSettleCount(1), forced);

  LDC1614::LDC1614 dev;
  Status st = dev.begin(cfg);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::I2C_ERROR),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_FALSE(dev.isInitialized());
  TEST_ASSERT_TRUE(dev.hardwareConfigDirty());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::I2C_ERROR),
                          static_cast<uint8_t>(dev.hardwareConfigDirtyError().code));
  TEST_ASSERT_EQUAL_UINT8(0x01u, dirtyDetailPhase(dev));
  TEST_ASSERT_EQUAL_HEX8(cmd::regSettleCount(1), dirtyDetailReg(dev));
  TEST_ASSERT_EQUAL_UINT8(1u, dirtyDetailIndex(dev));
}

// ============================================================================
// probe() and recover() Tests
// ============================================================================

void test_probe_missing_callbacks_returns_invalid_config_without_health() {
  LDC1614::LDC1614 dev;
  Status st = dev.probe();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_CONFIG),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::UNINIT),
                          static_cast<uint8_t>(dev.state()));
  TEST_ASSERT_EQUAL_UINT32(0u, dev.totalSuccess());
  TEST_ASSERT_EQUAL_UINT32(0u, dev.totalFailures());
  TEST_ASSERT_FALSE(dev.hardwareConfigDirty());
}

void test_probe_failure_does_not_update_health() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  const uint32_t beforeSuccess = dev.totalSuccess();
  const uint32_t beforeFailures = dev.totalFailures();
  const uint8_t beforeConsecutive = dev.consecutiveFailures();
  const DriverState beforeState = dev.state();

  bus.readStatus = Status::Error(Err::TIMEOUT, "forced probe timeout", -7);
  Status st = dev.probe();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::DEVICE_NOT_FOUND),
                          static_cast<uint8_t>(st.code));
  // Health must NOT change (probe uses raw wrappers)
  TEST_ASSERT_EQUAL_UINT32(beforeSuccess, dev.totalSuccess());
  TEST_ASSERT_EQUAL_UINT32(beforeFailures, dev.totalFailures());
  TEST_ASSERT_EQUAL_UINT8(beforeConsecutive, dev.consecutiveFailures());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(beforeState),
                          static_cast<uint8_t>(dev.state()));
}

void test_recover_failure_updates_health() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  bus.readStatus = Status::Error(Err::TIMEOUT, "forced recover timeout", -9);
  Status st = dev.recover();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::TIMEOUT), static_cast<uint8_t>(st.code));
  TEST_ASSERT_EQUAL_UINT32(1u, dev.totalFailures());
  TEST_ASSERT_EQUAL_UINT8(1u, dev.consecutiveFailures());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::DEGRADED),
                          static_cast<uint8_t>(dev.state()));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::TIMEOUT),
                          static_cast<uint8_t>(dev.lastError().code));
  TEST_ASSERT_EQUAL_UINT32(bus.nowMs, dev.lastErrorMs());
}

void test_recover_success_returns_ready() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  // Force failure to reach DEGRADED
  bus.readStatus = Status::Error(Err::TIMEOUT, "forced timeout", -9);
  (void)dev.recover();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::DEGRADED),
                          static_cast<uint8_t>(dev.state()));

  // Recover successfully
  bus.nowMs = 4321;
  bus.readStatus = Status::Ok();
  Status st = dev.recover();
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::READY),
                          static_cast<uint8_t>(dev.state()));
  TEST_ASSERT_EQUAL_UINT8(0u, dev.consecutiveFailures());
  TEST_ASSERT_EQUAL_UINT32(2u, dev.totalSuccess());
  TEST_ASSERT_EQUAL_UINT32(1u, dev.totalFailures());
  TEST_ASSERT_EQUAL_UINT32(4321u, dev.lastOkMs());
}

void test_recover_rejects_wrong_device_id() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  bus.reg[cmd::REG_DEVICE_ID] = 0xFFFF;
  Status st = dev.recover();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::DEVICE_NOT_FOUND),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_EQUAL_UINT32(1u, dev.totalFailures());
  TEST_ASSERT_EQUAL_UINT8(1u, dev.consecutiveFailures());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::DEGRADED),
                          static_cast<uint8_t>(dev.state()));
}

void test_recover_reaches_offline_when_threshold_is_one() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  Config cfg = makeConfig(bus);
  cfg.offlineThreshold = 1;
  TEST_ASSERT_TRUE(dev.begin(cfg).ok());

  bus.readStatus = Status::Error(Err::TIMEOUT, "forced timeout", -11);
  Status st = dev.recover();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::TIMEOUT), static_cast<uint8_t>(st.code));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::OFFLINE),
                          static_cast<uint8_t>(dev.state()));
  TEST_ASSERT_FALSE(dev.isOnline());
}

void test_syncConfig_success_clears_dirty() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  TEST_ASSERT_TRUE(dev.writeRegister16(cmd::REG_CONFIG, cmd::CONFIG_DEFAULT).ok());
  TEST_ASSERT_TRUE(dev.hardwareConfigDirty());

  resetIoCounters(bus);
  Status st = dev.syncConfig();
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_FALSE(dev.hardwareConfigDirty());
  TEST_ASSERT_TRUE(dev.hardwareConfigDirtyError().ok());
  TEST_ASSERT_TRUE(dev.isSleeping());
}

void test_syncConfig_failure_keeps_dirty() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  TEST_ASSERT_TRUE(dev.writeRegister16(cmd::REG_CONFIG, cmd::CONFIG_DEFAULT).ok());
  TEST_ASSERT_TRUE(dev.hardwareConfigDirty());

  resetIoCounters(bus);
  failWriteToReg(bus, cmd::REG_MUX_CONFIG,
                 Status::Error(Err::I2C_ERROR, "forced sync failure", -12));
  Status st = dev.syncConfig();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::I2C_ERROR),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_TRUE(dev.hardwareConfigDirty());
}

void test_recover_success_clears_dirty_after_reapply() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  Config cfg = makeConfig(bus);
  cfg.recoverBackoffMs = 0;
  TEST_ASSERT_TRUE(dev.begin(cfg).ok());

  TEST_ASSERT_TRUE(dev.writeRegister16(cmd::REG_CONFIG, cmd::CONFIG_DEFAULT).ok());
  TEST_ASSERT_TRUE(dev.hardwareConfigDirty());

  resetIoCounters(bus);
  Status st = dev.recover();
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_FALSE(dev.hardwareConfigDirty());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::READY),
                          static_cast<uint8_t>(dev.state()));
}

void test_recover_reapply_failure_keeps_dirty() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  Config cfg = makeConfig(bus);
  cfg.recoverBackoffMs = 0;
  TEST_ASSERT_TRUE(dev.begin(cfg).ok());

  TEST_ASSERT_TRUE(dev.writeRegister16(cmd::REG_CONFIG, cmd::CONFIG_DEFAULT).ok());
  resetIoCounters(bus);
  failWriteToReg(bus, cmd::REG_MUX_CONFIG,
                 Status::Error(Err::I2C_ERROR, "forced recover sync failure", -13));

  Status st = dev.recover();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::I2C_ERROR),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_TRUE(dev.hardwareConfigDirty());
}

// ============================================================================
// Transport Validation Tests
// ============================================================================

void test_raw_transport_rejects_invalid_buffers() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  uint8_t byte = 0;
  uint8_t rx = 0;

  Status st = dev._i2cWriteRaw(nullptr, 1);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_PARAM),
                          static_cast<uint8_t>(st.code));

  st = dev._i2cWriteRaw(&byte, 0);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_PARAM),
                          static_cast<uint8_t>(st.code));

  st = dev._i2cWriteReadRaw(nullptr, 1, &rx, 1);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_PARAM),
                          static_cast<uint8_t>(st.code));

  st = dev._i2cWriteReadRaw(&byte, 0, &rx, 1);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_PARAM),
                          static_cast<uint8_t>(st.code));

  st = dev._i2cWriteReadRaw(&byte, 1, nullptr, 1);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_PARAM),
                          static_cast<uint8_t>(st.code));

  st = dev._i2cWriteReadRaw(&byte, 1, &rx, 0);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_PARAM),
                          static_cast<uint8_t>(st.code));
}

void test_invalid_register_address_does_not_touch_bus() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  const uint32_t readsBefore = bus.readCalls;
  const uint32_t writesBefore = bus.writeCalls;
  uint16_t value = 0;
  Status st = dev.readRegister16(0x1D, value);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_PARAM),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_EQUAL_UINT32(readsBefore, bus.readCalls);

  st = dev.writeRegister16(0x1D, 0x1234);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_PARAM),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_EQUAL_UINT32(writesBefore, bus.writeCalls);
}

void test_raw_diagnostic_write_marks_dirty_on_success() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());
  TEST_ASSERT_FALSE(dev.hardwareConfigDirty());

  Status st = dev.writeRegister16(cmd::REG_CONFIG, cmd::CONFIG_DEFAULT);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_TRUE(dev.hardwareConfigDirty());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::CONFIG_DIRTY),
                          static_cast<uint8_t>(dev.hardwareConfigDirtyError().code));
  TEST_ASSERT_EQUAL_UINT8(0x06u, dirtyDetailPhase(dev));
  TEST_ASSERT_EQUAL_HEX8(cmd::REG_CONFIG, dirtyDetailReg(dev));
}

void test_raw_diagnostic_write_failure_does_not_clear_existing_dirty() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());
  TEST_ASSERT_TRUE(dev.writeRegister16(cmd::REG_CONFIG, cmd::CONFIG_DEFAULT).ok());
  TEST_ASSERT_TRUE(dev.hardwareConfigDirty());
  const int32_t firstDetail = dev.hardwareConfigDirtyError().detail;

  bus.writeStatus = Status::Error(Err::I2C_ERROR, "forced raw write failure", -31);
  Status st = dev.writeRegister16(cmd::REG_MUX_CONFIG, cmd::MUX_CONFIG_DEFAULT);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::I2C_ERROR),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_TRUE(dev.hardwareConfigDirty());
  TEST_ASSERT_EQUAL_INT32(firstDetail, dev.hardwareConfigDirtyError().detail);
}

// ============================================================================
// Precondition Tests
// ============================================================================

void test_readChannel_not_initialized() {
  LDC1614::LDC1614 dev;
  ChannelData data;
  Status st = dev.readChannel(0, data);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::NOT_INITIALIZED),
                          static_cast<uint8_t>(st.code));
}

void test_sleep_not_initialized() {
  LDC1614::LDC1614 dev;
  Status st = dev.sleep();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::NOT_INITIALIZED),
                          static_cast<uint8_t>(st.code));
}

void test_recover_not_initialized() {
  LDC1614::LDC1614 dev;
  Status st = dev.recover();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::NOT_INITIALIZED),
                          static_cast<uint8_t>(st.code));
}

void test_register_access_not_initialized_does_not_touch_bus() {
  FakeBus bus;
  LDC1614::LDC1614 dev;

  uint16_t value = 0;
  Status st = dev.readRegister16(cmd::REG_STATUS, value);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::NOT_INITIALIZED),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_EQUAL_UINT32(0u, bus.readCalls);

  st = dev.writeRegister16(cmd::REG_CONFIG, cmd::CONFIG_DEFAULT);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::NOT_INITIALIZED),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_EQUAL_UINT32(0u, bus.writeCalls);
}

// ============================================================================
// end() Tests
// ============================================================================

void test_end_resets_to_uninit() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::READY),
                          static_cast<uint8_t>(dev.state()));

  dev.end();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::UNINIT),
                          static_cast<uint8_t>(dev.state()));
  TEST_ASSERT_FALSE(dev.isOnline());
}

// ============================================================================
// Millis Fallback Tests
// ============================================================================

void test_begin_without_now_ms_uses_millis_fallback() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.nowMs = nullptr;
  cfg.timeUser = nullptr;

  LDC1614::LDC1614 dev;
  Status st = dev.begin(cfg);
  TEST_ASSERT_TRUE(st.ok());
  // Missing nowMs hook returns 0; lastOkMs should stay 0.
  TEST_ASSERT_EQUAL_UINT32(0u, dev.lastOkMs());
}

// ============================================================================
// Offline Threshold Tests
// ============================================================================

void test_multiple_failures_reach_offline() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.offlineThreshold = 3;

  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(cfg).ok());

  bus.readStatus = Status::Error(Err::TIMEOUT, "forced timeout", -9);

  // First failure -> DEGRADED
  bus.nowMs = 1000;
  (void)dev.recover();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::DEGRADED),
                          static_cast<uint8_t>(dev.state()));
  TEST_ASSERT_EQUAL_UINT8(1u, dev.consecutiveFailures());
  TEST_ASSERT_TRUE(dev.isOnline());  // DEGRADED is still online

  // Second failure -> still DEGRADED
  bus.nowMs = 2000;
  (void)dev.recover();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::DEGRADED),
                          static_cast<uint8_t>(dev.state()));
  TEST_ASSERT_EQUAL_UINT8(2u, dev.consecutiveFailures());
  TEST_ASSERT_TRUE(dev.isOnline());

  // Third failure -> OFFLINE
  bus.nowMs = 3000;
  (void)dev.recover();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::OFFLINE),
                          static_cast<uint8_t>(dev.state()));
  TEST_ASSERT_EQUAL_UINT8(3u, dev.consecutiveFailures());
  TEST_ASSERT_FALSE(dev.isOnline());

  // Recover -> back to READY
  bus.readStatus = Status::Ok();
  bus.nowMs = 4000;
  Status st = dev.recover();
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::READY),
                          static_cast<uint8_t>(dev.state()));
  TEST_ASSERT_EQUAL_UINT8(0u, dev.consecutiveFailures());
  TEST_ASSERT_TRUE(dev.isOnline());
}

void test_offline_read_channel_returns_busy_without_i2c() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.offlineThreshold = 1;

  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(cfg).ok());

  bus.readStatus = Status::Error(Err::TIMEOUT, "forced timeout", -9);
  (void)dev.recover();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::OFFLINE),
                          static_cast<uint8_t>(dev.state()));

  const uint32_t readsBefore = bus.readCalls;
  const uint32_t writesBefore = bus.writeCalls;
  ChannelData data;
  Status st = dev.readChannel(0, data);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::BUSY),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_EQUAL_STRING("Driver is offline; call recover()", st.msg);
  TEST_ASSERT_EQUAL_UINT32(readsBefore, bus.readCalls);
  TEST_ASSERT_EQUAL_UINT32(writesBefore, bus.writeCalls);
}

void test_failed_recover_from_offline_keeps_latch_after_intermediate_success() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.offlineThreshold = 3;

  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(cfg).ok());

  bus.readStatus = Status::Error(Err::TIMEOUT, "forced timeout", -21);
  for (uint8_t i = 0; i < cfg.offlineThreshold; ++i) {
    bus.nowMs = 1000u * static_cast<uint32_t>(i + 1u);
    TEST_ASSERT_FALSE(dev.recover().ok());
  }
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::OFFLINE),
                          static_cast<uint8_t>(dev.state()));

  bus.readStatus = Status::Ok();
  bus.reg[cmd::REG_DEVICE_ID] = 0x1234;
  bus.nowMs = 5000;
  const Status st = dev.recover();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::DEVICE_NOT_FOUND),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::OFFLINE),
                          static_cast<uint8_t>(dev.state()));
  TEST_ASSERT_TRUE(dev.consecutiveFailures() >= cfg.offlineThreshold);
}

// ============================================================================
// Config Recovery Defaults Tests
// ============================================================================

void test_config_recovery_defaults() {
  Config cfg;
  TEST_ASSERT_NULL(cfg.busReset);
  TEST_ASSERT_NULL(cfg.hardReset);
  TEST_ASSERT_EQUAL_UINT32(100u, cfg.recoverBackoffMs);
  TEST_ASSERT_TRUE(cfg.recoverUseBusReset);
  TEST_ASSERT_FALSE(cfg.recoverUseSoftReset);
  TEST_ASSERT_TRUE(cfg.recoverUseHardReset);
}

// ============================================================================
// Sample Cache Tests
// ============================================================================

void test_getLastSample_before_any_read() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  ChannelData data;
  Status st = dev.getLastSample(0, data);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::CONVERSION_NOT_READY),
                          static_cast<uint8_t>(st.code));
}

void test_sampleTimestampMs_before_read() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  TEST_ASSERT_EQUAL_UINT32(0u, dev.sampleTimestampMs(0));
  TEST_ASSERT_EQUAL_UINT32(0u, dev.sampleAgeMs(0, 5000));
}

void test_readChannel_caches_data_and_timestamp() {
  FakeBus bus;
  bus.nowMs = 5000;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  // Wake for reading
  Status st = dev.wake();
  TEST_ASSERT_TRUE(st.ok());

  // Read channel 0
  ChannelData data;
  st = dev.readChannel(0, data);
  TEST_ASSERT_TRUE(st.ok());

  // Verify cache
  TEST_ASSERT_EQUAL_UINT32(5000u, dev.sampleTimestampMs(0));

  ChannelData cached;
  st = dev.getLastSample(0, cached);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_EQUAL_UINT32(data.rawData, cached.rawData);

  // Verify age
  bus.nowMs = 5500;
  TEST_ASSERT_EQUAL_UINT32(500u, dev.sampleAgeMs(0, 5500));
}

void test_readChannel_reconstructs_28bit_data_and_error_flags() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  bus.reg[cmd::REG_DATA0_MSB] = cmd::MASK_DATA_ERR_UR | cmd::MASK_DATA_ERR_AE | 0x0ABC;
  bus.reg[cmd::REG_DATA0_LSB] = 0xDEF0;

  ChannelData data;
  Status st = dev.readChannel(0, data);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_EQUAL_HEX32(0x0ABCDEF0u, data.rawData);
  TEST_ASSERT_TRUE(data.errUnderRange);
  TEST_ASSERT_FALSE(data.errOverRange);
  TEST_ASSERT_FALSE(data.errWatchdog);
  TEST_ASSERT_TRUE(data.errAmplitude);
}

void test_readAllChannels_rejects_count_above_channel_count() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  ChannelData data[4];
  Status st = dev.readAllChannels(data, 5);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_PARAM),
                          static_cast<uint8_t>(st.code));
}

void test_readDeviceStatus_parses_flags() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  bus.reg[cmd::REG_STATUS] =
      (2u << cmd::BIT_STATUS_ERR_CHAN) |
      cmd::MASK_STATUS_ERR_UR |
      cmd::MASK_STATUS_ERR_ALE |
      cmd::MASK_STATUS_DRDY |
      cmd::MASK_STATUS_UNREADCONV2;

  DeviceStatus status;
  Status st = dev.readDeviceStatus(status);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_EQUAL_UINT8(2u, status.errChan);
  TEST_ASSERT_TRUE(status.errUnderRange);
  TEST_ASSERT_FALSE(status.errOverRange);
  TEST_ASSERT_FALSE(status.errWatchdog);
  TEST_ASSERT_FALSE(status.errAmplitudeHigh);
  TEST_ASSERT_TRUE(status.errAmplitudeLow);
  TEST_ASSERT_FALSE(status.errZeroCount);
  TEST_ASSERT_TRUE(status.dataReady);
  TEST_ASSERT_FALSE(status.unreadConv[0]);
  TEST_ASSERT_TRUE(status.unreadConv[2]);
  TEST_ASSERT_TRUE(status.hasError());
}

void test_getLastSample_invalid_channel() {
  LDC1614::LDC1614 dev;
  ChannelData data;
  Status st = dev.getLastSample(5, data);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_PARAM),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_FALSE(dev.hasSample(5));
}

// ============================================================================
// Settings Snapshot Tests
// ============================================================================

void test_getSettings_captures_state() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  Config cfg = makeConfig(bus);
  cfg.autoScan = true;
  cfg.activeChan = 2;
  cfg.channelCount = 4;
  cfg.refClkSrc = RefClkSrc::EXT_CLK;
  cfg.channel[0].rcount = 0x1000;

  TEST_ASSERT_TRUE(dev.begin(cfg).ok());

  SettingsSnapshot snap;
  TEST_ASSERT_TRUE(dev.getSettings(snap).ok());

  TEST_ASSERT_TRUE(snap.initialized);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::READY),
                          static_cast<uint8_t>(snap.state));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::READY),
                          static_cast<uint8_t>(dev.driverState()));
  TEST_ASSERT_EQUAL_HEX8(0x2A, snap.i2cAddress);
  TEST_ASSERT_EQUAL_UINT32(10u, snap.i2cTimeoutMs);
  TEST_ASSERT_EQUAL_UINT8(3u, snap.offlineThreshold);
  TEST_ASSERT_TRUE(snap.hasNowMsHook);
  TEST_ASSERT_FALSE(snap.hasSample[0]);
  TEST_ASSERT_TRUE(snap.sleeping);
  TEST_ASSERT_TRUE(snap.autoScan);
  TEST_ASSERT_FALSE(snap.hardwareConfigDirty);
  TEST_ASSERT_TRUE(snap.hardwareConfigDirtyError.ok());
  TEST_ASSERT_EQUAL_UINT8(2u, snap.activeChan);
  TEST_ASSERT_EQUAL_UINT8(4u, snap.channelCount);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RefClkSrc::EXT_CLK),
                          static_cast<uint8_t>(snap.refClkSrc));
  TEST_ASSERT_EQUAL_UINT16(0x1000, snap.channel[0].rcount);

  SettingsSnapshot byValue = dev.settings();
  TEST_ASSERT_EQUAL_UINT8(4u, byValue.channelCount);
}

// ============================================================================
// Runtime Setter Tests
// ============================================================================

void test_setRcount_does_not_commit_cache_on_write_failure() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  const uint16_t before = dev.getConfig().channel[0].rcount;
  bus.writeStatus = Status::Error(Err::I2C_ERROR, "forced write failure");
  Status st = dev.setRcount(0, 0x1234);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::I2C_ERROR),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_EQUAL_UINT16(before, dev.getConfig().channel[0].rcount);
  TEST_ASSERT_TRUE(dev.hardwareConfigDirty());
  TEST_ASSERT_EQUAL_HEX8(cmd::regRcount(0), dirtyDetailReg(dev));
}

void test_setErrorConfig_rejects_reserved_bits() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  Status st = dev.setErrorConfig(0x0002);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_PARAM),
                          static_cast<uint8_t>(st.code));
}

void test_setErrorConfig_writes_and_commits_cache() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  const uint16_t errorConfig =
      cmd::MASK_ERRCFG_DRDY_2INT | cmd::MASK_ERRCFG_OR_ERR2OUT;
  Status st = dev.setErrorConfig(errorConfig);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_EQUAL_HEX8(cmd::REG_ERROR_CONFIG, bus.lastWriteReg);
  TEST_ASSERT_EQUAL_HEX16(errorConfig, bus.lastWriteValue);
  TEST_ASSERT_EQUAL_HEX16(errorConfig, dev.getErrorConfig());
}

void test_setAutoScanMode_rejects_ldc1612_invalid_sequence() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.channelCount = 2;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(cfg).ok());

  Status st = dev.setAutoScanMode(RRSequence::CH0_CH1_CH2);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_PARAM),
                          static_cast<uint8_t>(st.code));
}

void test_setDeglitch_writes_mux_config() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  Status st = dev.setDeglitch(Deglitch::BW_3MHZ);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_EQUAL_HEX8(cmd::REG_MUX_CONFIG, bus.lastWriteReg);
  TEST_ASSERT_EQUAL_HEX16(cmd::MUX_CONFIG_RESERVED_VALUE |
                          static_cast<uint16_t>(Deglitch::BW_3MHZ),
                          bus.lastWriteValue);
}

void test_setSingleChannelMode_writes_mux_and_config() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.autoScan = true;
  cfg.rrSequence = RRSequence::CH0_CH1_CH2;

  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(cfg).ok());

  Status st = dev.setSingleChannelMode(2);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_FALSE(dev.getConfig().autoScan);
  TEST_ASSERT_EQUAL_UINT8(2u, dev.getConfig().activeChan);
  TEST_ASSERT_EQUAL_UINT16(0u, bus.reg[cmd::REG_MUX_CONFIG] & cmd::MASK_MUX_AUTOSCAN_EN);
  TEST_ASSERT_EQUAL_UINT16(2u,
                           (bus.reg[cmd::REG_CONFIG] & cmd::MASK_CFG_ACTIVE_CHAN) >>
                               cmd::BIT_CFG_ACTIVE_CHAN);
}

void test_setSingleChannelMode_config_write_failure_marks_dirty() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.autoScan = true;
  cfg.rrSequence = RRSequence::CH0_CH1_CH2;

  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(cfg).ok());
  resetIoCounters(bus);
  failNthWrite(bus, 2, Status::Error(Err::I2C_ERROR, "forced config failure", -14));

  Status st = dev.setSingleChannelMode(2);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::I2C_ERROR),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_TRUE(dev.getConfig().autoScan);
  TEST_ASSERT_EQUAL_UINT8(0u, dev.getConfig().activeChan);
  TEST_ASSERT_TRUE(dev.hardwareConfigDirty());
  TEST_ASSERT_EQUAL_UINT8(0x05u, dirtyDetailPhase(dev));
  TEST_ASSERT_EQUAL_HEX8(cmd::REG_CONFIG, dirtyDetailReg(dev));
  TEST_ASSERT_TRUE((bus.reg[cmd::REG_MUX_CONFIG] & cmd::MASK_MUX_AUTOSCAN_EN) == 0u);
}

void test_setAutoScanMode_writes_mux_and_commits() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  Status st = dev.setAutoScanMode(RRSequence::CH0_CH1_CH2_CH3);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_TRUE(dev.getConfig().autoScan);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RRSequence::CH0_CH1_CH2_CH3),
                          static_cast<uint8_t>(dev.getConfig().rrSequence));
  TEST_ASSERT_TRUE((bus.reg[cmd::REG_MUX_CONFIG] & cmd::MASK_MUX_AUTOSCAN_EN) != 0u);
  TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(RRSequence::CH0_CH1_CH2_CH3),
                           (bus.reg[cmd::REG_MUX_CONFIG] & cmd::MASK_MUX_RR_SEQUENCE) >>
                               cmd::BIT_MUX_RR_SEQUENCE);
}

void test_setHighCurrentDrive_rejects_autoscan() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.autoScan = true;
  cfg.rrSequence = RRSequence::CH0_CH1;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(cfg).ok());

  Status st = dev.setHighCurrentDriveEnabled(true);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::INVALID_PARAM),
                          static_cast<uint8_t>(st.code));
}

void test_config_bit_setters_write_config_and_commit() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  Status st = dev.setIntbDisabled(true);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_TRUE(dev.getConfig().intbDisable);
  TEST_ASSERT_TRUE((bus.reg[cmd::REG_CONFIG] & cmd::MASK_CFG_INTB_DIS) != 0u);

  st = dev.setReferenceClockSource(RefClkSrc::EXT_CLK);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RefClkSrc::EXT_CLK),
                          static_cast<uint8_t>(dev.getConfig().refClkSrc));
  TEST_ASSERT_TRUE((bus.reg[cmd::REG_CONFIG] & cmd::MASK_CFG_REF_CLK_SRC) != 0u);

  st = dev.setSensorActivation(SensorActivation::FULL_CURRENT);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(SensorActivation::FULL_CURRENT),
                          static_cast<uint8_t>(dev.getConfig().sensorActivation));
  TEST_ASSERT_EQUAL_UINT16(0u, bus.reg[cmd::REG_CONFIG] & cmd::MASK_CFG_SENSOR_ACTIVATE_SEL);

  st = dev.setRpOverrideEnabled(false);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_FALSE(dev.getConfig().rpOverrideEn);
  TEST_ASSERT_EQUAL_UINT16(0u, bus.reg[cmd::REG_CONFIG] & cmd::MASK_CFG_RP_OVERRIDE_EN);

  st = dev.setAutoAmplitudeCorrectionEnabled(true);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_FALSE(dev.getConfig().autoAmpDis);
  TEST_ASSERT_EQUAL_UINT16(0u, bus.reg[cmd::REG_CONFIG] & cmd::MASK_CFG_AUTO_AMP_DIS);

  st = dev.setHighCurrentDriveEnabled(true);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_TRUE(dev.getConfig().highCurrentDrv);
  TEST_ASSERT_TRUE((bus.reg[cmd::REG_CONFIG] & cmd::MASK_CFG_HIGH_CURRENT_DRV) != 0u);
}

void test_calc_helpers_use_channel_config() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.channel[0].rcount = 10;
  cfg.channel[0].settleCount = 5;
  cfg.channel[0].finDivider = 2;
  cfg.channel[0].frefDivider = 4;
  cfg.channel[0].offset = 0;

  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(cfg).ok());

  const float freq = dev.calcSensorFrequency(0, 0x08000000u, 40000000.0f);
  TEST_ASSERT_FLOAT_WITHIN(1.0f, 10000000.0f, freq);

  const float convUs = dev.calcConversionTimeUs(0, 40000000.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 16.4f, convUs);

  const float settleUs = dev.calcSettleTimeUs(0, 40000000.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 8.0f, settleUs);

  const float sampleUs = dev.calcSampleTimeUs(0, 40000000.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 24.4f, sampleUs);
}

void test_calc_settle_uses_datasheet_minimum_for_zero_and_one() {
  FakeBus bus;
  Config cfg = makeConfig(bus);
  cfg.channel[0].settleCount = 0;
  cfg.channel[0].frefDivider = 4;

  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(cfg).ok());

  TEST_ASSERT_FLOAT_WITHIN(0.01f, 3.2f, dev.calcSettleTimeUs(0, 40000000.0f));
  TEST_ASSERT_TRUE(dev.setSettleCount(0, 1).ok());
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 3.2f, dev.calcSettleTimeUs(0, 40000000.0f));
}

void test_calc_helpers_reject_nonfinite_reference_clock() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  const float nanRef = std::numeric_limits<float>::quiet_NaN();
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, dev.calcSensorFrequency(0, 0x08000000u, nanRef));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, dev.calcConversionTimeUs(0, nanRef));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, dev.calcSettleTimeUs(0, nanRef));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, dev.calcSampleTimeUs(0, nanRef));
}

// ============================================================================
// isMeasuring Tests
// ============================================================================

void test_isMeasuring_false_when_sleeping() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());
  TEST_ASSERT_FALSE(dev.isMeasuring());  // After begin, device is sleeping
}

void test_isMeasuring_true_after_wake() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());
  TEST_ASSERT_TRUE(dev.wake().ok());
  TEST_ASSERT_TRUE(dev.isMeasuring());
}

void test_isMeasuring_false_when_uninit() {
  LDC1614::LDC1614 dev;
  TEST_ASSERT_FALSE(dev.isMeasuring());
}

// ============================================================================
// Blocking Read Tests
// ============================================================================

void test_readChannelBlocking_not_initialized() {
  LDC1614::LDC1614 dev;
  ChannelData data;
  Status st = dev.readChannelBlocking(0, data);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::NOT_INITIALIZED),
                          static_cast<uint8_t>(st.code));
}

void test_readChannelBlocking_rejects_sleeping() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  ChannelData data;
  Status st = dev.readChannelBlocking(0, data);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::BUSY),
                          static_cast<uint8_t>(st.code));
}

void test_readAllChannelsBlocking_not_initialized() {
  LDC1614::LDC1614 dev;
  ChannelData data[4];
  Status st = dev.readAllChannelsBlocking(data);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::NOT_INITIALIZED),
                          static_cast<uint8_t>(st.code));
}

void test_readDataReady_status_polling_success() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  bus.reg[cmd::REG_STATUS] = cmd::MASK_STATUS_DRDY;
  bool ready = false;
  Status st = dev.readDataReady(ready);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_TRUE(ready);
}

void test_readDataReady_propagates_i2c_failure() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  bus.readStatus = Status::Error(Err::I2C_TIMEOUT, "forced data-ready timeout");
  bool ready = true;
  Status st = dev.readDataReady(ready);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::I2C_TIMEOUT),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_FALSE(ready);
  TEST_ASSERT_EQUAL_UINT8(1u, dev.consecutiveFailures());
}

void test_dataReady_convenience_returns_false_on_failure() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  bus.readStatus = Status::Error(Err::I2C_TIMEOUT, "forced data-ready timeout");
  TEST_ASSERT_FALSE(dev.dataReady());
}

void test_readDataReady_intb_reads_status_to_distinguish_error() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  Config cfg = makeConfig(bus);
  cfg.intbPin = 4;
  cfg.gpioRead = fakeGpioRead;
  cfg.gpioUser = &bus;
  TEST_ASSERT_TRUE(dev.begin(cfg).ok());

  bus.gpioLevel = false;  // INTB asserted, active low
  bus.reg[cmd::REG_STATUS] = cmd::MASK_STATUS_ERR_OR;
  bool ready = true;
  Status st = dev.readDataReady(ready);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::SENSOR_ERROR),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_FALSE(ready);
}

void test_readChannelBlocking_propagates_dataReady_failure() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());
  TEST_ASSERT_TRUE(dev.wake().ok());

  bus.readStatus = Status::Error(Err::I2C_TIMEOUT, "forced data-ready timeout");
  ChannelData data;
  Status st = dev.readChannelBlocking(0, data, 10);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::I2C_TIMEOUT),
                          static_cast<uint8_t>(st.code));
}

void test_readChannelBlocking_times_out_with_stalled_clock() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());
  TEST_ASSERT_TRUE(dev.wake().ok());

  bus.reg[cmd::REG_STATUS] = 0x0000;
  ChannelData data;
  Status st = dev.readChannelBlocking(0, data, 5);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::TIMEOUT),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_TRUE(bus.yieldCalls > 0);
}

void test_readAllChannelsBlocking_times_out_with_stalled_clock() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());
  TEST_ASSERT_TRUE(dev.wake().ok());

  bus.reg[cmd::REG_STATUS] = 0x0000;
  ChannelData data[4];
  Status st = dev.readAllChannelsBlocking(data, 5);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::TIMEOUT),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_TRUE(bus.yieldCalls > 0);
}

// ============================================================================
// resetAndReapply Tests
// ============================================================================

void test_resetAndReapply_not_initialized() {
  LDC1614::LDC1614 dev;
  Status st = dev.resetAndReapply();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::NOT_INITIALIZED),
                          static_cast<uint8_t>(st.code));
}

void test_softReset_success_keeps_dirty_until_reinit() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  Config cfg = makeConfig(bus);
  TEST_ASSERT_TRUE(dev.begin(cfg).ok());
  TEST_ASSERT_TRUE(dev.writeRegister16(cmd::REG_CONFIG, cmd::CONFIG_DEFAULT).ok());
  TEST_ASSERT_TRUE(dev.hardwareConfigDirty());

  Status st = dev.softReset();
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_FALSE(dev.isInitialized());
  TEST_ASSERT_TRUE(dev.hardwareConfigDirty());

  st = dev.begin(cfg);
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_FALSE(dev.hardwareConfigDirty());
}

void test_resetAndReapply_success_keeps_ready() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());

  Status st = dev.resetAndReapply();
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::READY),
                          static_cast<uint8_t>(dev.state()));
  TEST_ASSERT_TRUE(dev.isSleeping());  // Re-applies config in sleep mode
}

void test_resetAndReapply_partial_failure_sets_dirty_with_register_detail() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());
  resetIoCounters(bus);
  failWriteToReg(bus, cmd::REG_MUX_CONFIG,
                 Status::Error(Err::I2C_ERROR, "forced reapply failure", -15));

  Status st = dev.resetAndReapply();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::I2C_ERROR),
                          static_cast<uint8_t>(st.code));
  TEST_ASSERT_FALSE(dev.isInitialized());
  TEST_ASSERT_TRUE(dev.hardwareConfigDirty());
  TEST_ASSERT_EQUAL_UINT8(0x02u, dirtyDetailPhase(dev));
  TEST_ASSERT_EQUAL_HEX8(cmd::REG_MUX_CONFIG, dirtyDetailReg(dev));
  TEST_ASSERT_EQUAL_UINT8(0xFFu, dirtyDetailIndex(dev));
}

void test_resetAndReapply_success_clears_dirty() {
  FakeBus bus;
  LDC1614::LDC1614 dev;
  TEST_ASSERT_TRUE(dev.begin(makeConfig(bus)).ok());
  TEST_ASSERT_TRUE(dev.writeRegister16(cmd::REG_CONFIG, cmd::CONFIG_DEFAULT).ok());
  TEST_ASSERT_TRUE(dev.hardwareConfigDirty());

  Status st = dev.resetAndReapply();
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_FALSE(dev.hardwareConfigDirty());
  TEST_ASSERT_TRUE(dev.hardwareConfigDirtyError().ok());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriverState::READY),
                          static_cast<uint8_t>(dev.state()));
}

// ============================================================================
// Recovery Ladder Tests
// ============================================================================

void test_recover_backoff_prevents_rapid_retry() {
  FakeBus bus;
  bus.nowMs = 1000;
  LDC1614::LDC1614 dev;
  Config cfg = makeConfig(bus);
  cfg.recoverBackoffMs = 200;
  TEST_ASSERT_TRUE(dev.begin(cfg).ok());

  // Force failure for first recover
  bus.readStatus = Status::Error(Err::TIMEOUT, "forced timeout");
  Status st = dev.recover();
  TEST_ASSERT_FALSE(st.ok());

  // Second recover too fast -> BUSY (backoff active)
  bus.nowMs = 1100;  // Only 100ms later, backoff is 200ms
  bus.readStatus = Status::Ok();
  st = dev.recover();
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Err::BUSY),
                          static_cast<uint8_t>(st.code));

  // After backoff window -> should succeed
  bus.nowMs = 1300;  // 300ms later, backoff was 200ms from first attempt at 1000
  st = dev.recover();
  TEST_ASSERT_TRUE(st.ok());
}

void test_recover_uses_bus_reset_callback() {
  FakeBus bus;
  auto busResetFn = [](void* user) -> Status {
    FakeBus* bus = static_cast<FakeBus*>(user);
    bus->busResetCalled = true;
    bus->readStatus = Status::Ok();
    return Status::Ok();
  };

  LDC1614::LDC1614 dev;
  Config cfg = makeConfig(bus);
  cfg.busReset = busResetFn;
  cfg.i2cUser = &bus;
  cfg.recoverBackoffMs = 0;  // No backoff for test
  TEST_ASSERT_TRUE(dev.begin(cfg).ok());

  bus.readStatus = Status::Error(Err::TIMEOUT, "forced timeout");
  Status st = dev.recover();
  TEST_ASSERT_TRUE(st.ok());
  TEST_ASSERT_TRUE(bus.busResetCalled);
}

// ============================================================================
// Main
// ============================================================================

int main() {
  UNITY_BEGIN();

  // Status
  RUN_TEST(test_status_ok);
  RUN_TEST(test_status_error);
  RUN_TEST(test_status_in_progress);
  RUN_TEST(test_type_traits_delete_copy_and_move);

  // Config
  RUN_TEST(test_config_defaults);
  RUN_TEST(test_channel_config_defaults);
  RUN_TEST(test_deglitch_enum_matches_datasheet_mux_values);

  // begin()
  RUN_TEST(test_begin_rejects_missing_callbacks);
  RUN_TEST(test_begin_rejects_bad_address);
  RUN_TEST(test_begin_rejects_bad_channel_count);
  RUN_TEST(test_begin_rejects_rcount_below_minimum);
  RUN_TEST(test_begin_rejects_idrive_above_maximum);
  RUN_TEST(test_begin_rejects_ldc1612_invalid_rr_sequence);
  RUN_TEST(test_begin_rejects_reserved_error_config_bits);
  RUN_TEST(test_invalid_begin_resets_runtime_and_default_config);
  RUN_TEST(test_begin_normalizes_offline_threshold_on_stored_copy);
  RUN_TEST(test_begin_success_sets_ready);
  RUN_TEST(test_begin_with_2channels);
  RUN_TEST(test_begin_applyConfig_partial_failure_sets_dirty_with_register_detail);

  // probe/recover
  RUN_TEST(test_probe_missing_callbacks_returns_invalid_config_without_health);
  RUN_TEST(test_probe_failure_does_not_update_health);
  RUN_TEST(test_recover_failure_updates_health);
  RUN_TEST(test_recover_success_returns_ready);
  RUN_TEST(test_recover_rejects_wrong_device_id);
  RUN_TEST(test_recover_reaches_offline_when_threshold_is_one);
  RUN_TEST(test_syncConfig_success_clears_dirty);
  RUN_TEST(test_syncConfig_failure_keeps_dirty);
  RUN_TEST(test_recover_success_clears_dirty_after_reapply);
  RUN_TEST(test_recover_reapply_failure_keeps_dirty);

  // Transport
  RUN_TEST(test_raw_transport_rejects_invalid_buffers);
  RUN_TEST(test_invalid_register_address_does_not_touch_bus);
  RUN_TEST(test_raw_diagnostic_write_marks_dirty_on_success);
  RUN_TEST(test_raw_diagnostic_write_failure_does_not_clear_existing_dirty);

  // Preconditions
  RUN_TEST(test_readChannel_not_initialized);
  RUN_TEST(test_sleep_not_initialized);
  RUN_TEST(test_recover_not_initialized);
  RUN_TEST(test_register_access_not_initialized_does_not_touch_bus);

  // end()
  RUN_TEST(test_end_resets_to_uninit);

  // Millis fallback
  RUN_TEST(test_begin_without_now_ms_uses_millis_fallback);

  // Offline threshold
  RUN_TEST(test_multiple_failures_reach_offline);
  RUN_TEST(test_offline_read_channel_returns_busy_without_i2c);
  RUN_TEST(test_failed_recover_from_offline_keeps_latch_after_intermediate_success);

  // Config recovery defaults
  RUN_TEST(test_config_recovery_defaults);

  // Sample cache
  RUN_TEST(test_getLastSample_before_any_read);
  RUN_TEST(test_sampleTimestampMs_before_read);
  RUN_TEST(test_readChannel_caches_data_and_timestamp);
  RUN_TEST(test_readChannel_reconstructs_28bit_data_and_error_flags);
  RUN_TEST(test_readAllChannels_rejects_count_above_channel_count);
  RUN_TEST(test_readDeviceStatus_parses_flags);
  RUN_TEST(test_getLastSample_invalid_channel);

  // Settings snapshot
  RUN_TEST(test_getSettings_captures_state);

  // Runtime setters
  RUN_TEST(test_setRcount_does_not_commit_cache_on_write_failure);
  RUN_TEST(test_setErrorConfig_rejects_reserved_bits);
  RUN_TEST(test_setErrorConfig_writes_and_commits_cache);
  RUN_TEST(test_setAutoScanMode_rejects_ldc1612_invalid_sequence);
  RUN_TEST(test_setDeglitch_writes_mux_config);
  RUN_TEST(test_setSingleChannelMode_writes_mux_and_config);
  RUN_TEST(test_setSingleChannelMode_config_write_failure_marks_dirty);
  RUN_TEST(test_setAutoScanMode_writes_mux_and_commits);
  RUN_TEST(test_setHighCurrentDrive_rejects_autoscan);
  RUN_TEST(test_config_bit_setters_write_config_and_commit);
  RUN_TEST(test_calc_helpers_use_channel_config);
  RUN_TEST(test_calc_settle_uses_datasheet_minimum_for_zero_and_one);
  RUN_TEST(test_calc_helpers_reject_nonfinite_reference_clock);

  // isMeasuring
  RUN_TEST(test_isMeasuring_false_when_sleeping);
  RUN_TEST(test_isMeasuring_true_after_wake);
  RUN_TEST(test_isMeasuring_false_when_uninit);

  // Blocking reads
  RUN_TEST(test_readChannelBlocking_not_initialized);
  RUN_TEST(test_readChannelBlocking_rejects_sleeping);
  RUN_TEST(test_readAllChannelsBlocking_not_initialized);
  RUN_TEST(test_readDataReady_status_polling_success);
  RUN_TEST(test_readDataReady_propagates_i2c_failure);
  RUN_TEST(test_dataReady_convenience_returns_false_on_failure);
  RUN_TEST(test_readDataReady_intb_reads_status_to_distinguish_error);
  RUN_TEST(test_readChannelBlocking_propagates_dataReady_failure);
  RUN_TEST(test_readChannelBlocking_times_out_with_stalled_clock);
  RUN_TEST(test_readAllChannelsBlocking_times_out_with_stalled_clock);

  // resetAndReapply
  RUN_TEST(test_resetAndReapply_not_initialized);
  RUN_TEST(test_softReset_success_keeps_dirty_until_reinit);
  RUN_TEST(test_resetAndReapply_success_keeps_ready);
  RUN_TEST(test_resetAndReapply_partial_failure_sets_dirty_with_register_detail);
  RUN_TEST(test_resetAndReapply_success_clears_dirty);

  // Recovery ladder
  RUN_TEST(test_recover_backoff_prevents_rapid_retry);
  RUN_TEST(test_recover_uses_bus_reset_callback);

  return UNITY_END();
}
