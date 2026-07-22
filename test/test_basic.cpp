/// @file test_basic.cpp
/// @brief Native production-contract and fault-injection tests for the v3 driver.

#include <unity.h>

#include <cstdint>
#include <limits>
#include <type_traits>

#include "LDC1614/LDC1614.h"
#include "support/FakeLdc1614Device.h"

using namespace LDC1614;
using ldc1614_test::FakeLdc1614Device;

namespace {

constexpr uint64_t NOW_MS = 100;
constexpr uint64_t DEADLINE_MS = 1000;
constexpr Status FORCED_READ =
    Status::Error(Err::I2C_TIMEOUT, "forced full-detail timeout", -1234567);
constexpr Status FORCED_WRITE =
    Status::Error(Err::I2C_BUS, "forced full-detail write fault", -7654321);

Status failingIntb(bool& asserted, void*) {
  asserted = false;
  return Status::Error(Err::I2C_ERROR, "forced INTB failure", -9001);
}

void assertCode(Err expected, const Status& actual) {
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected),
                          static_cast<uint8_t>(actual.code));
}

void assertOutcome(TerminalOutcome expected, const OperationResult& actual) {
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected),
                          static_cast<uint8_t>(actual.outcome));
}

void assertKind(JobKind expected, const OperationResult& actual) {
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected),
                          static_cast<uint8_t>(actual.kind));
}

Config makeConfig(FakeLdc1614Device& fake,
                  DeviceVariant variant = DeviceVariant::LDC1612,
                  bool multiChannel = false) {
  Config config;
  config.i2cWrite = FakeLdc1614Device::write;
  config.i2cWriteRead = FakeLdc1614Device::writeRead;
  config.i2cUser = &fake;
  config.i2cTimeoutMs = 5;
  config.i2cAddress = I2cAddress::ADDR_GND;
  config.variant = variant;
  config.referenceClock = {RefClkSrc::INTERNAL, 43000000U, 200000U};
  config.deglitch = Deglitch::BW_1MHZ;
  config.errorReporting = ErrorReporting::all();
  config.intbDisabled = true;

  const uint8_t channelCount = variant == DeviceVariant::LDC1612 ? 2U : 4U;
  for (uint8_t ch = 0; ch < channelCount; ++ch) {
    ChannelConfig& channel = config.channel[ch];
    channel.rcount = 0x0080;
    channel.settleCount = multiChannel ? 0x0004 : 0x0000;
    channel.finDivider = 1;
    // Internal 43 MHz must be divided to <=35 MHz at the channel reference.
    channel.frefDivider = 2;
    channel.offset = 0;
    channel.driveCurrentCode = 16;
    channel.expectedSensorMinHz = 100000;
    channel.expectedSensorMaxHz = 500000;
  }

  if (!multiChannel) {
    config.channels = channelBit(Channel::CH0);
    config.mode = OperatingMode::SINGLE_CHANNEL;
    config.activeChannel = Channel::CH0;
    config.rrSequence = RRSequence::UNSPECIFIED;
  } else if (variant == DeviceVariant::LDC1612) {
    config.channels = ChannelMask{0x03};
    config.mode = OperatingMode::MULTI_CHANNEL_SEQUENTIAL;
    config.activeChannel = Channel::NONE;
    config.rrSequence = RRSequence::CH0_CH1;
  } else {
    config.channels = ChannelMask{0x0F};
    config.mode = OperatingMode::MULTI_CHANNEL_SEQUENTIAL;
    config.activeChannel = Channel::NONE;
    config.rrSequence = RRSequence::CH0_CH1_CH2_CH3;
  }
  return config;
}

uint8_t initTransfers(DeviceVariant variant) {
  return variant == DeviceVariant::LDC1612 ? 16U : 26U;
}

uint8_t applyTransfers(DeviceVariant variant) {
  return variant == DeviceVariant::LDC1612 ? 14U : 24U;
}

uint8_t resetTransfers(DeviceVariant variant) {
  return variant == DeviceVariant::LDC1612 ? 17U : 27U;
}

struct ExpectedStage {
  JobPhase phase;
  uint8_t reg;
  Channel channel;
};

ExpectedStage configStage(DeviceVariant variant, uint8_t transfer) {
  const uint8_t channelCount =
      variant == DeviceVariant::LDC1612 ? 2U : 4U;
  if (transfer == 1U) {
    return {JobPhase::FORCE_SLEEP, cmd::REG_CONFIG, Channel::NONE};
  }
  const uint8_t fieldTransfer = static_cast<uint8_t>(transfer - 2U);
  if (fieldTransfer < static_cast<uint8_t>(channelCount * 5U)) {
    const uint8_t channel = static_cast<uint8_t>(fieldTransfer / 5U);
    const uint8_t field = static_cast<uint8_t>(fieldTransfer % 5U);
    const JobPhase phases[] = {
        JobPhase::WRITE_RCOUNT, JobPhase::WRITE_SETTLECOUNT,
        JobPhase::WRITE_CLOCK_DIVIDERS, JobPhase::WRITE_OFFSET,
        JobPhase::WRITE_DRIVE_CURRENT};
    const uint8_t registers[] = {
        cmd::regRcount(channel), cmd::regSettleCount(channel),
        cmd::regClockDividers(channel), cmd::regOffset(channel),
        cmd::regDriveCurrent(channel)};
    return {phases[field], registers[field], static_cast<Channel>(channel)};
  }
  const uint8_t global =
      static_cast<uint8_t>(fieldTransfer - channelCount * 5U);
  if (global == 0U) {
    return {JobPhase::WRITE_ERROR_CONFIG, cmd::REG_ERROR_CONFIG, Channel::NONE};
  }
  if (global == 1U) {
    return {JobPhase::WRITE_MUX_CONFIG, cmd::REG_MUX_CONFIG, Channel::NONE};
  }
  return {JobPhase::WRITE_FINAL_CONFIG, cmd::REG_CONFIG, Channel::NONE};
}

ExpectedStage initializeStage(DeviceVariant variant, uint8_t transfer) {
  if (transfer == 1U) {
    return {JobPhase::VERIFY_MANUFACTURER, cmd::REG_MANUFACTURER_ID,
            Channel::NONE};
  }
  if (transfer == 2U) {
    return {JobPhase::VERIFY_DEVICE, cmd::REG_DEVICE_ID, Channel::NONE};
  }
  return configStage(variant, static_cast<uint8_t>(transfer - 2U));
}

ExpectedStage resetStage(DeviceVariant variant, uint8_t transfer) {
  if (transfer == 1U) {
    return {JobPhase::SOFTWARE_RESET, cmd::REG_RESET_DEV, Channel::NONE};
  }
  if (transfer == 2U) {
    return {JobPhase::VERIFY_MANUFACTURER, cmd::REG_MANUFACTURER_ID,
            Channel::NONE};
  }
  if (transfer == 3U) {
    return {JobPhase::VERIFY_DEVICE, cmd::REG_DEVICE_ID, Channel::NONE};
  }
  return configStage(variant, static_cast<uint8_t>(transfer - 3U));
}

ExpectedStage acquireStage(ChannelMask channels, uint8_t transfer) {
  if (transfer == 1U) {
    return {JobPhase::READ_STATUS_BEFORE, cmd::REG_STATUS, Channel::NONE};
  }
  uint8_t remaining = static_cast<uint8_t>(transfer - 2U);
  for (uint8_t channel = 0; channel < 4U; ++channel) {
    if ((channels.bits & static_cast<uint8_t>(1U << channel)) == 0U) {
      continue;
    }
    if (remaining == 0U) {
      return {JobPhase::READ_DATA_MSB, cmd::regDataMsb(channel),
              static_cast<Channel>(channel)};
    }
    if (remaining == 1U) {
      return {JobPhase::READ_DATA_LSB, cmd::regDataLsb(channel),
              static_cast<Channel>(channel)};
    }
    remaining = static_cast<uint8_t>(remaining - 2U);
  }
  return {JobPhase::READ_STATUS_AFTER, cmd::REG_STATUS, Channel::NONE};
}

void assertStage(const ExpectedStage& expected, const JobProgress& actual) {
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected.phase),
                          static_cast<uint8_t>(actual.phase));
  TEST_ASSERT_EQUAL_HEX8(expected.reg, actual.registerAddress);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected.channel),
                          static_cast<uint8_t>(actual.channel));
}

void assertFaultStage(const ExpectedStage& expected,
                      const ConfigFault& actual) {
  TEST_ASSERT_TRUE(actual.valid);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected.phase),
                          static_cast<uint8_t>(actual.phase));
  TEST_ASSERT_EQUAL_HEX8(expected.reg, actual.registerAddress);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(expected.channel),
                          static_cast<uint8_t>(actual.channel));
}

Status pollToTerminal(LDC1614::LDC1614& driver, uint64_t nowMs = NOW_MS,
                      uint8_t budget = 255) {
  Status status = Status::Error(Err::IN_PROGRESS, "not polled");
  for (uint8_t call = 0; call < 64U; ++call) {
    status = driver.poll(nowMs, budget);
    if (!driver.jobProgress().active) {
      return status;
    }
  }
  TEST_FAIL_MESSAGE("bounded test poll did not terminate");
  return status;
}

OperationResult takeResult(LDC1614::LDC1614& driver) {
  OperationResult result;
  TEST_ASSERT_TRUE(driver.resultAvailable());
  TEST_ASSERT_TRUE(driver.takeResult(result).ok());
  return result;
}

void initialize(LDC1614::LDC1614& driver, FakeLdc1614Device& fake,
                const Config& config, OperationId operationId = 1) {
  TEST_ASSERT_TRUE(driver.bind(config).ok());
  TEST_ASSERT_TRUE(driver.startInitialize(operationId, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(pollToTerminal(driver).ok());
  OperationResult result = takeResult(driver);
  assertOutcome(TerminalOutcome::SUCCESS, result);
  assertKind(JobKind::INITIALIZE, result);
  TEST_ASSERT_EQUAL_UINT64(operationId, result.operationId);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::APPLIED_SLEEPING),
                          static_cast<uint8_t>(driver.appliedConfigState()));
  (void)fake;
}

void initializeAndWake(LDC1614::LDC1614& driver, FakeLdc1614Device& fake,
                       const Config& config, OperationId operationId = 1) {
  initialize(driver, fake, config, operationId);
  TEST_ASSERT_TRUE(driver.wake().ok());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::APPLIED_ACTIVE),
                          static_cast<uint8_t>(driver.appliedConfigState()));
}

uint16_t directRead(FakeLdc1614Device& fake, uint8_t reg) {
  uint8_t rx[2] = {};
  TEST_ASSERT_TRUE(FakeLdc1614Device::writeRead(
      fake.acceptedAddress, &reg, 1, rx, sizeof(rx), 5, &fake).ok());
  return static_cast<uint16_t>((static_cast<uint16_t>(rx[0]) << 8U) | rx[1]);
}

void test_status_and_public_type_contracts() {
  static_assert(sizeof(OperationId) == sizeof(uint64_t), "operation IDs are fixed width");
  static_assert(LDC1614::LDC1614::RESULT_CAPACITY == 2, "terminal capacity is contractual");
  static_assert(!std::is_copy_constructible<LDC1614::LDC1614>::value,
                "driver must not copy non-owning transport state");
  static_assert(!std::is_copy_assignable<LDC1614::LDC1614>::value,
                "driver must not copy non-owning transport state");
  static_assert(!std::is_move_constructible<LDC1614::LDC1614>::value,
                "driver address and state must remain stable");
  static_assert(!std::is_move_assignable<LDC1614::LDC1614>::value,
                "driver address and state must remain stable");
  static_assert(std::is_trivially_copyable<Config>::value, "Config stays value-like");
  static_assert(std::is_trivially_copyable<JobProgress>::value,
                "progress stays fixed-memory");
  static_assert(std::is_trivially_copyable<OperationResult>::value,
                "results stay fixed-memory");
  static_assert(std::is_standard_layout<SampleBatch>::value,
                "batch stays a portable aggregate");
  static_assert(sizeof(OperationResult) <= 384U, "terminal result size is bounded");
  static_assert(sizeof(LDC1614::LDC1614) <= 1024U, "driver instance size is bounded");

  TEST_ASSERT_TRUE(Status::Ok().ok());
  TEST_ASSERT_TRUE(Status::Error(Err::IN_PROGRESS, "working").inProgress());
  TEST_ASSERT_EQUAL_HEX8(0x03, validChannelMask(DeviceVariant::LDC1612).bits);
  TEST_ASSERT_EQUAL_HEX8(0x0F, validChannelMask(DeviceVariant::LDC1614).bits);
  TEST_ASSERT_TRUE((channelBit(Channel::CH0) | channelBit(Channel::CH3)).contains(
      Channel::CH3));
  TEST_ASSERT_FALSE(ChannelMask{0x0F}.contains(static_cast<Channel>(32)));
  TEST_ASSERT_TRUE(channelBit(static_cast<Channel>(32)).empty());
}

void test_behavioral_fake_models_shadow_status_unread_and_intb() {
  FakeLdc1614Device fake;
  fake.reg[cmd::REG_ERROR_CONFIG] = ErrorReporting::all().dataReady
                                             ? cmd::MASK_ERRCFG_DRDY_2INT |
                                                   cmd::MASK_ERRCFG_OR_ERR2INT
                                             : 0U;
  fake.injectConversion(0, 0x01234567U, cmd::MASK_DATA_ERR_OR,
                        cmd::MASK_STATUS_ERR_OR);
  TEST_ASSERT_TRUE(fake.intbAsserted);
  TEST_ASSERT_TRUE((fake.statusValue() & cmd::MASK_STATUS_UNREADCONV0) != 0U);

  const uint16_t status = directRead(fake, cmd::REG_STATUS);
  TEST_ASSERT_TRUE((status & cmd::MASK_STATUS_ERR_OR) != 0U);
  TEST_ASSERT_FALSE(fake.intbAsserted);
  TEST_ASSERT_EQUAL_HEX16(0, fake.stickyStatusErrors);
  TEST_ASSERT_TRUE((fake.statusValue() & cmd::MASK_STATUS_UNREADCONV0) != 0U);

  const uint16_t msb = directRead(fake, cmd::REG_DATA0_MSB);
  fake.injectConversion(0, 0x07654321U);
  const uint16_t lsb = directRead(fake, cmd::REG_DATA0_LSB);
  TEST_ASSERT_EQUAL_HEX16(cmd::MASK_DATA_ERR_OR | 0x0123U, msb);
  TEST_ASSERT_EQUAL_HEX16(0x4567U, lsb);
  TEST_ASSERT_TRUE((fake.statusValue() & cmd::MASK_STATUS_UNREADCONV0) != 0U);

  (void)directRead(fake, cmd::REG_DATA0_MSB);
  TEST_ASSERT_FALSE((fake.statusValue() & cmd::MASK_STATUS_UNREADCONV0) != 0U);
}

void test_bind_is_zero_i2c_and_validates_complete_explicit_profile() {
  FakeLdc1614Device fake;
  LDC1614::LDC1614 driver;
  Config valid = makeConfig(fake);
  TEST_ASSERT_TRUE(driver.bind(valid).ok());
  TEST_ASSERT_EQUAL_UINT16(0, fake.transferCalls);
  TEST_ASSERT_TRUE(driver.isBound());
  TEST_ASSERT_EQUAL_UINT32(1, driver.configRevision());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::UNKNOWN),
                          static_cast<uint8_t>(driver.appliedConfigState()));

  assertCode(Err::BUSY, driver.bind(valid));
  TEST_ASSERT_EQUAL_UINT16(0, fake.transferCalls);
  driver.end();
  TEST_ASSERT_FALSE(driver.isBound());
  TEST_ASSERT_EQUAL_UINT16(0, fake.transferCalls);

  Config cases[24] = {};
  for (Config& config : cases) {
    config = valid;
  }
  cases[0].i2cWrite = nullptr;
  cases[1].i2cWriteRead = nullptr;
  cases[2].i2cTimeoutMs = 0;
  cases[3].i2cAddress = I2cAddress::UNSPECIFIED;
  cases[4].variant = DeviceVariant::UNSPECIFIED;
  cases[5].channels = ChannelMask{};
  cases[6].channels = ChannelMask{0x80};
  cases[7].referenceClock.source = RefClkSrc::UNSPECIFIED;
  cases[8].referenceClock.frequencyHz = 0;
  cases[9].mode = OperatingMode::UNSPECIFIED;
  cases[10].deglitch = Deglitch::UNSPECIFIED;
  cases[11].channel[0].driveCurrentCode = 0xFF;
  cases[12].channel[0].rcount = 0x0004;
  cases[13].channel[0].finDivider = 0;
  cases[14].channel[0].frefDivider = 0;
  cases[15].channel[0].expectedSensorMinHz = 0;
  cases[16].channel[0].expectedSensorMaxHz =
      cases[16].channel[0].expectedSensorMinHz - 1U;
  cases[17].channel[0].expectedSensorMaxHz = 2000000U;
  cases[18].channel[0].expectedSensorMaxHz = 9000000U;
  cases[19].channels = channelBit(Channel::CH2);
  cases[20].mode = OperatingMode::MULTI_CHANNEL_SEQUENTIAL;
  cases[20].activeChannel = Channel::NONE;
  cases[20].rrSequence = RRSequence::UNSPECIFIED;
  cases[21].deglitch = static_cast<Deglitch>(2);
  cases[22].channel[0].frefDivider = 1;  // 43 MHz / 1 exceeds 35 MHz.
  // The complete physical register profile is required even when a physical
  // channel is not selected for conversion.
  cases[23].channel[1].finDivider = 0;

  for (uint8_t index = 0; index < 24U; ++index) {
    LDC1614::LDC1614 invalidDriver;
    const Status invalidStatus = invalidDriver.bind(cases[index]);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        static_cast<uint8_t>(Err::INVALID_CONFIG),
        static_cast<uint8_t>(invalidStatus.code), "invalid config case index");
    TEST_ASSERT_FALSE(invalidDriver.isBound());
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(0, fake.transferCalls,
                                     "bind validation touched I2C");
  }

  Config toleranceLimit = valid;
  toleranceLimit.referenceClock.frequencyHz = 35000000U;
  toleranceLimit.referenceClock.tolerancePpm = 1U;
  toleranceLimit.channel[0].frefDivider = 1;
  LDC1614::LDC1614 toleranceLimitDriver;
  assertCode(Err::INVALID_CONFIG, toleranceLimitDriver.bind(toleranceLimit));

  Config worstCaseRatio = valid;
  worstCaseRatio.deglitch = Deglitch::BW_10MHZ;
  worstCaseRatio.channel[0].expectedSensorMaxHz = 4375000U;
  LDC1614::LDC1614 worstCaseRatioDriver;
  assertCode(Err::INVALID_CONFIG, worstCaseRatioDriver.bind(worstCaseRatio));

  Config deglitchEquality = valid;
  deglitchEquality.channel[0].expectedSensorMaxHz = 1000000U;
  LDC1614::LDC1614 deglitchEqualityDriver;
  assertCode(Err::INVALID_CONFIG,
             deglitchEqualityDriver.bind(deglitchEquality));
  TEST_ASSERT_EQUAL_UINT16(0, fake.transferCalls);
}

void test_initialize_exact_transfer_counts_and_zero_one_large_budgets() {
  const DeviceVariant variants[] = {DeviceVariant::LDC1612, DeviceVariant::LDC1614};
  for (DeviceVariant variant : variants) {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    Config config = makeConfig(fake, variant);
    TEST_ASSERT_TRUE(driver.bind(config).ok());
    TEST_ASSERT_TRUE(driver.startInitialize(10, DEADLINE_MS).inProgress());
    TEST_ASSERT_EQUAL_UINT8(initTransfers(variant), driver.jobProgress().maximumTransfers);

    TEST_ASSERT_TRUE(driver.poll(NOW_MS, 0).inProgress());
    TEST_ASSERT_EQUAL_UINT16(0, fake.transferCalls);
    while (driver.jobProgress().active) {
      const uint16_t before = fake.transferCalls;
      const Status status = driver.poll(NOW_MS, 1);
      TEST_ASSERT_LESS_OR_EQUAL_UINT16(1, fake.transferCalls - before);
      TEST_ASSERT_TRUE(status.ok() || status.inProgress());
    }
    TEST_ASSERT_EQUAL_UINT16(initTransfers(variant), fake.transferCalls);
    OperationResult result = takeResult(driver);
    assertOutcome(TerminalOutcome::SUCCESS, result);

    driver.end();
    fake.clearIo();
    TEST_ASSERT_TRUE(driver.bind(config).ok());
    TEST_ASSERT_TRUE(driver.startInitialize(11, DEADLINE_MS).inProgress());
    TEST_ASSERT_TRUE(driver.poll(NOW_MS, 255).ok());
    TEST_ASSERT_EQUAL_UINT16(initTransfers(variant), fake.transferCalls);
    TEST_ASSERT_FALSE(driver.jobProgress().active);
    (void)takeResult(driver);
  }
}

struct ExpectedWrite {
  uint8_t reg;
  uint16_t value;
};

void assertConfigWrites(const FakeLdc1614Device& fake, uint8_t offset,
                        const ExpectedWrite* expected, uint8_t count) {
  for (uint8_t index = 0; index < count; ++index) {
    const FakeLdc1614Device::Transfer& actual =
        fake.transferLog[static_cast<uint8_t>(offset + index)];
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(FakeLdc1614Device::TransferKind::WRITE),
        static_cast<uint8_t>(actual.kind));
    TEST_ASSERT_EQUAL_HEX8(expected[index].reg, actual.reg);
    TEST_ASSERT_EQUAL_HEX16(expected[index].value, actual.value);
  }
}

void test_exact_register_replay_for_both_variants_and_all_replay_jobs() {
  static constexpr ExpectedWrite LDC1612_WRITES[] = {
      {cmd::REG_CONFIG, 0x3C81},
      {cmd::REG_RCOUNT0, 0x0080}, {cmd::REG_SETTLECOUNT0, 0x0000},
      {cmd::REG_CLOCK_DIVIDERS0, 0x1002}, {cmd::REG_OFFSET0, 0x0000},
      {cmd::REG_DRIVE_CURRENT0, 0x8000},
      {cmd::REG_RCOUNT1, 0x0080}, {cmd::REG_SETTLECOUNT1, 0x0000},
      {cmd::REG_CLOCK_DIVIDERS1, 0x1002}, {cmd::REG_OFFSET1, 0x0000},
      {cmd::REG_DRIVE_CURRENT1, 0x8000},
      {cmd::REG_ERROR_CONFIG, 0xF8FD}, {cmd::REG_MUX_CONFIG, 0x0209},
      {cmd::REG_CONFIG, 0x3C81},
  };
  {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    Config config = makeConfig(fake, DeviceVariant::LDC1612);
    TEST_ASSERT_TRUE(driver.bind(config).ok());
    TEST_ASSERT_TRUE(driver.startInitialize(12001, DEADLINE_MS).inProgress());
    TEST_ASSERT_TRUE(pollToTerminal(driver).ok());
    (void)takeResult(driver);
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(FakeLdc1614Device::TransferKind::READ),
        static_cast<uint8_t>(fake.transferLog[0].kind));
    TEST_ASSERT_EQUAL_HEX8(cmd::REG_MANUFACTURER_ID,
                           fake.transferLog[0].reg);
    TEST_ASSERT_EQUAL_HEX16(cmd::MANUFACTURER_ID_VALUE,
                            fake.transferLog[0].value);
    TEST_ASSERT_EQUAL_HEX8(cmd::REG_DEVICE_ID, fake.transferLog[1].reg);
    TEST_ASSERT_EQUAL_HEX16(cmd::DEVICE_ID_VALUE, fake.transferLog[1].value);
    assertConfigWrites(fake, 2U, LDC1612_WRITES, 14U);
  }

  static constexpr ExpectedWrite LDC1614_WRITES[] = {
      {cmd::REG_CONFIG, 0x2281},
      {cmd::REG_RCOUNT0, 0x0100}, {cmd::REG_SETTLECOUNT0, 0x0020},
      {cmd::REG_CLOCK_DIVIDERS0, 0x1008}, {cmd::REG_OFFSET0, 0x0001},
      {cmd::REG_DRIVE_CURRENT0, 0x5000},
      {cmd::REG_RCOUNT1, 0x0101}, {cmd::REG_SETTLECOUNT1, 0x0021},
      {cmd::REG_CLOCK_DIVIDERS1, 0x2009}, {cmd::REG_OFFSET1, 0x0002},
      {cmd::REG_DRIVE_CURRENT1, 0x5800},
      {cmd::REG_RCOUNT2, 0x0102}, {cmd::REG_SETTLECOUNT2, 0x0022},
      {cmd::REG_CLOCK_DIVIDERS2, 0x300A}, {cmd::REG_OFFSET2, 0x0003},
      {cmd::REG_DRIVE_CURRENT2, 0x6000},
      {cmd::REG_RCOUNT3, 0x0103}, {cmd::REG_SETTLECOUNT3, 0x0023},
      {cmd::REG_CLOCK_DIVIDERS3, 0x400B}, {cmd::REG_OFFSET3, 0x0004},
      {cmd::REG_DRIVE_CURRENT3, 0x6800},
      {cmd::REG_ERROR_CONFIG, 0xF8FD}, {cmd::REG_MUX_CONFIG, 0xC20F},
      {cmd::REG_CONFIG, 0x2281},
  };
  FakeLdc1614Device fake;
  LDC1614::LDC1614 driver;
  Config config = makeConfig(fake, DeviceVariant::LDC1614, true);
  config.referenceClock = {RefClkSrc::EXTERNAL_CLOCK, 20000000U, 0U};
  config.deglitch = Deglitch::BW_33MHZ;
  config.sensorActivation = SensorActivation::FULL_CURRENT;
  config.rpOverrideEnabled = false;
  config.autoAmplitudeCorrectionEnabled = true;
  for (uint8_t channel = 0; channel < 4U; ++channel) {
    config.channel[channel].rcount = static_cast<uint16_t>(0x0100U + channel);
    config.channel[channel].settleCount = static_cast<uint16_t>(0x0020U + channel);
    config.channel[channel].finDivider = static_cast<uint8_t>(channel + 1U);
    config.channel[channel].frefDivider = static_cast<uint16_t>(8U + channel);
    config.channel[channel].offset = static_cast<uint16_t>(channel + 1U);
    config.channel[channel].driveCurrentCode =
        DriveCurrentCode{static_cast<uint8_t>(10U + channel)};
    config.channel[channel].expectedSensorMinHz = 100000U;
    config.channel[channel].expectedSensorMaxHz = 300000U;
  }
  TEST_ASSERT_TRUE(driver.bind(config).ok());
  TEST_ASSERT_TRUE(driver.startInitialize(12002, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(pollToTerminal(driver).ok());
  (void)takeResult(driver);
  assertConfigWrites(fake, 2U, LDC1614_WRITES, 24U);

  fake.clearIo();
  TEST_ASSERT_TRUE(driver.startApplyConfig(12003, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(pollToTerminal(driver).ok());
  (void)takeResult(driver);
  assertConfigWrites(fake, 0U, LDC1614_WRITES, 24U);

  fake.clearIo();
  TEST_ASSERT_TRUE(driver.startResetAndReapply(12004, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(pollToTerminal(driver).ok());
  (void)takeResult(driver);
  const ExpectedWrite reset = {cmd::REG_RESET_DEV, cmd::MASK_RESET_DEV};
  assertConfigWrites(fake, 0U, &reset, 1U);
  TEST_ASSERT_EQUAL_HEX8(cmd::REG_MANUFACTURER_ID, fake.transferLog[1].reg);
  TEST_ASSERT_EQUAL_HEX8(cmd::REG_DEVICE_ID, fake.transferLog[2].reg);
  assertConfigWrites(fake, 3U, LDC1614_WRITES, 24U);
}

void test_large_poll_budget_uses_actual_remaining_transfer_count() {
  const DeviceVariant variants[] = {DeviceVariant::LDC1612,
                                    DeviceVariant::LDC1614};
  for (DeviceVariant variant : variants) {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    Config config = makeConfig(fake, variant);
    config.i2cTimeoutMs = 20U;
    TEST_ASSERT_TRUE(driver.bind(config).ok());
    TEST_ASSERT_TRUE(driver.startInitialize(12100U + initTransfers(variant),
                                            2000U).inProgress());
    TEST_ASSERT_TRUE(driver.poll(100U, 255U).ok());
    TEST_ASSERT_EQUAL_UINT16(initTransfers(variant), fake.transferCalls);
    for (uint8_t index = 0; index < initTransfers(variant); ++index) {
      TEST_ASSERT_EQUAL_UINT32(20U, fake.transferLog[index].timeoutMs);
    }
    (void)takeResult(driver);
  }

  FakeLdc1614Device fake;
  LDC1614::LDC1614 driver;
  Config config = makeConfig(fake);
  config.i2cTimeoutMs = 20U;
  TEST_ASSERT_TRUE(driver.bind(config).ok());
  TEST_ASSERT_TRUE(driver.startInitialize(12199, 2000U).inProgress());
  TEST_ASSERT_TRUE(driver.poll(100U, 15U).inProgress());
  TEST_ASSERT_TRUE(driver.poll(100U, 255U).ok());
  TEST_ASSERT_EQUAL_UINT32(20U, fake.transferLog[15].timeoutMs);
  (void)takeResult(driver);
}

void test_initialize_fails_once_at_every_transfer_with_full_provenance() {
  const DeviceVariant variants[] = {DeviceVariant::LDC1612, DeviceVariant::LDC1614};
  for (DeviceVariant variant : variants) {
    const uint8_t maximum = initTransfers(variant);
    for (uint8_t failed = 1; failed <= maximum; ++failed) {
      FakeLdc1614Device fake;
      LDC1614::LDC1614 driver;
      Config config = makeConfig(fake, variant);
      TEST_ASSERT_TRUE(driver.bind(config).ok());
      fake.failOnTransfer(failed, failed <= 2U ? FORCED_READ : FORCED_WRITE);
      TEST_ASSERT_TRUE(driver.startInitialize(1000U + failed, DEADLINE_MS).inProgress());
      const Status status = pollToTerminal(driver);
      assertCode(failed <= 2U ? FORCED_READ.code : FORCED_WRITE.code, status);
      TEST_ASSERT_EQUAL_UINT16_MESSAGE(failed, fake.transferCalls,
                                       "initialize retried or ran past failure");

      OperationResult result = takeResult(driver);
      assertOutcome(TerminalOutcome::FAILED, result);
      assertKind(JobKind::INITIALIZE, result);
      TEST_ASSERT_FALSE(result.finalProgress.active);
      TEST_ASSERT_EQUAL_UINT64(result.operationId,
                               result.finalProgress.operationId);
      TEST_ASSERT_EQUAL_UINT8(failed,
                              result.finalProgress.completedTransfers);
      const ExpectedStage failedStage = initializeStage(variant, failed);
      assertStage(failedStage, result.finalProgress);
      TEST_ASSERT_EQUAL_INT32(failed <= 2U ? FORCED_READ.detail : FORCED_WRITE.detail,
                              result.status.detail);
      if (failed <= 2U) {
        TEST_ASSERT_FALSE(result.configFault.valid);
      } else {
        assertFaultStage(failedStage, result.configFault);
        TEST_ASSERT_TRUE(hasEffect(result.effects, EffectFlag::INDETERMINATE_WRITE));
        TEST_ASSERT_EQUAL_INT32(FORCED_WRITE.detail, result.configFault.cause.detail);
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::DIRTY),
                                static_cast<uint8_t>(driver.appliedConfigState()));
      }
    }
  }

  const uint8_t identityRegs[] = {cmd::REG_MANUFACTURER_ID, cmd::REG_DEVICE_ID};
  const uint16_t expectedCalls[] = {1, 2};
  for (uint8_t index = 0; index < 2U; ++index) {
    FakeLdc1614Device fake;
    fake.reg[identityRegs[index]] ^= 0x0001U;
    LDC1614::LDC1614 driver;
    TEST_ASSERT_TRUE(driver.bind(makeConfig(fake)).ok());
    TEST_ASSERT_TRUE(driver.startInitialize(1900U + index, DEADLINE_MS).inProgress());
    assertCode(Err::DEVICE_NOT_FOUND, pollToTerminal(driver));
    TEST_ASSERT_EQUAL_UINT16(expectedCalls[index], fake.transferCalls);
    OperationResult result = takeResult(driver);
    assertOutcome(TerminalOutcome::FAILED, result);
    TEST_ASSERT_TRUE(result.configFault.valid);
    TEST_ASSERT_EQUAL_HEX8(identityRegs[index],
                           result.configFault.registerAddress);
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(index == 0U ? JobPhase::VERIFY_MANUFACTURER
                                         : JobPhase::VERIFY_DEVICE),
        static_cast<uint8_t>(result.configFault.phase));
    TEST_ASSERT_EQUAL_UINT32(driver.configRevision(), result.configRevision);
  }

  FakeLdc1614Device replaced;
  LDC1614::LDC1614 repeated;
  Config repeatedConfig = makeConfig(replaced);
  initializeAndWake(repeated, replaced, repeatedConfig);
  replaced.clearIo();
  replaced.reg[cmd::REG_DEVICE_ID] ^= 1U;
  TEST_ASSERT_TRUE(repeated.startInitialize(1950, DEADLINE_MS).inProgress());
  assertCode(Err::DEVICE_NOT_FOUND, pollToTerminal(repeated));
  OperationResult mismatch = takeResult(repeated);
  TEST_ASSERT_TRUE(mismatch.configFault.valid);
  TEST_ASSERT_EQUAL_HEX8(cmd::REG_DEVICE_ID,
                         mismatch.configFault.registerAddress);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::UNKNOWN),
                          static_cast<uint8_t>(repeated.appliedConfigState()));
  assertCode(Err::CONFIG_DIRTY,
             repeated.startAcquire(channelBit(Channel::CH0), 1951,
                                   DEADLINE_MS));
}

void test_initialize_cancel_is_bus_silent_at_every_phase() {
  const uint8_t maximum = initTransfers(DeviceVariant::LDC1614);
  for (uint8_t completed = 0; completed < maximum; ++completed) {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    Config config = makeConfig(fake, DeviceVariant::LDC1614);
    TEST_ASSERT_TRUE(driver.bind(config).ok());
    TEST_ASSERT_TRUE(driver.startInitialize(2000U + completed, DEADLINE_MS).inProgress());
    for (uint8_t step = 0; step < completed; ++step) {
      TEST_ASSERT_TRUE(driver.poll(NOW_MS, 1).inProgress());
    }
    const uint16_t before = fake.transferCalls;
    TEST_ASSERT_TRUE(driver.cancelJob().ok());
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(before, fake.transferCalls, "cancel touched I2C");
    TEST_ASSERT_FALSE(driver.jobProgress().active);
    OperationResult result = takeResult(driver);
    assertOutcome(TerminalOutcome::CANCELLED, result);
    assertCode(Err::CANCELLED, result.status);
    TEST_ASSERT_FALSE(result.hasSampleBatch);
    if (completed > 2U) {
      TEST_ASSERT_TRUE(hasEffect(result.effects, EffectFlag::PARTIAL_WRITE));
      assertFaultStage(configStage(DeviceVariant::LDC1614,
                                   static_cast<uint8_t>(completed - 2U)),
                       result.configFault);
      TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::DIRTY),
                              static_cast<uint8_t>(driver.appliedConfigState()));
    }
  }
}

void test_deadline_timeout_is_bus_silent_wrap_safe_and_caps_callback_timeout() {
  {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    TEST_ASSERT_TRUE(driver.bind(makeConfig(fake)).ok());
    TEST_ASSERT_TRUE(driver.startInitialize(3001, 500).inProgress());
    assertCode(Err::TIMEOUT, driver.poll(500, 0));
    TEST_ASSERT_EQUAL_UINT16(0, fake.transferCalls);
    OperationResult result = takeResult(driver);
    assertOutcome(TerminalOutcome::TIMED_OUT, result);
  }
  {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    Config config = makeConfig(fake);
    config.i2cTimeoutMs = 50;
    TEST_ASSERT_TRUE(driver.bind(config).ok());
    const uint64_t wrapBoundary = static_cast<uint64_t>(UINT32_MAX) + 2ULL;
    TEST_ASSERT_TRUE(driver.startInitialize(3002, wrapBoundary).inProgress());
    TEST_ASSERT_TRUE(driver.poll(wrapBoundary - 3ULL, 1).inProgress());
    TEST_ASSERT_EQUAL_UINT32(3, fake.transferLog[0].timeoutMs);
    const uint16_t before = fake.transferCalls;
    assertCode(Err::TIMEOUT, driver.poll(wrapBoundary, 255));
    TEST_ASSERT_EQUAL_UINT16(before, fake.transferCalls);
    assertOutcome(TerminalOutcome::TIMED_OUT, takeResult(driver));
  }
  {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    TEST_ASSERT_TRUE(driver.bind(makeConfig(fake)).ok());
    const uint64_t deadline = std::numeric_limits<uint64_t>::max();
    TEST_ASSERT_TRUE(driver.startInitialize(3003, deadline).inProgress());
    TEST_ASSERT_TRUE(driver.poll(deadline - 1ULL, 1).inProgress());
    TEST_ASSERT_EQUAL_UINT32(1, fake.transferLog[0].timeoutMs);
  }
  {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    Config config = makeConfig(fake);
    config.i2cTimeoutMs = 50;
    TEST_ASSERT_TRUE(driver.bind(config).ok());
    TEST_ASSERT_TRUE(driver.startInitialize(3004, 105).inProgress());
    TEST_ASSERT_TRUE(driver.poll(100, 5).inProgress());
    TEST_ASSERT_EQUAL_UINT16(5, fake.transferCalls);
    for (uint8_t index = 0; index < 5U; ++index) {
      TEST_ASSERT_EQUAL_UINT32(1, fake.transferLog[index].timeoutMs);
    }
  }
}

void test_deadline_expiry_is_silent_at_every_multistep_operation_phase() {
  for (uint8_t completed = 0; completed < initTransfers(DeviceVariant::LDC1614);
       ++completed) {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    TEST_ASSERT_TRUE(driver.bind(makeConfig(fake, DeviceVariant::LDC1614)).ok());
    TEST_ASSERT_TRUE(driver.startInitialize(3100U + completed, 200).inProgress());
    for (uint8_t step = 0; step < completed; ++step) {
      TEST_ASSERT_TRUE(driver.poll(100, 1).inProgress());
    }
    const uint16_t before = fake.transferCalls;
    assertCode(Err::TIMEOUT, driver.poll(200, 0));
    TEST_ASSERT_EQUAL_UINT16(before, fake.transferCalls);
    OperationResult result = takeResult(driver);
    assertOutcome(TerminalOutcome::TIMED_OUT, result);
    TEST_ASSERT_FALSE(result.hasSampleBatch);
    assertStage(initializeStage(DeviceVariant::LDC1614,
                                static_cast<uint8_t>(completed + 1U)),
                result.finalProgress);
    if (completed > 2U) {
      assertFaultStage(configStage(DeviceVariant::LDC1614,
                                   static_cast<uint8_t>(completed - 2U)),
                       result.configFault);
      TEST_ASSERT_TRUE(hasEffect(result.effects, EffectFlag::PARTIAL_WRITE));
      TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::DIRTY),
                              static_cast<uint8_t>(driver.appliedConfigState()));
    } else {
      TEST_ASSERT_FALSE(result.configFault.valid);
    }
  }

  for (uint8_t completed = 0; completed < applyTransfers(DeviceVariant::LDC1614);
       ++completed) {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    Config config = makeConfig(fake, DeviceVariant::LDC1614);
    initialize(driver, fake, config);
    fake.clearIo();
    TEST_ASSERT_TRUE(driver.startApplyConfig(3200U + completed, 200).inProgress());
    for (uint8_t step = 0; step < completed; ++step) {
      TEST_ASSERT_TRUE(driver.poll(100, 1).inProgress());
    }
    const uint16_t before = fake.transferCalls;
    assertCode(Err::TIMEOUT, driver.poll(200, 0));
    TEST_ASSERT_EQUAL_UINT16(before, fake.transferCalls);
    OperationResult result = takeResult(driver);
    assertOutcome(TerminalOutcome::TIMED_OUT, result);
    assertStage(configStage(DeviceVariant::LDC1614,
                            static_cast<uint8_t>(completed + 1U)),
                result.finalProgress);
    if (completed != 0U) {
      assertFaultStage(configStage(DeviceVariant::LDC1614, completed),
                       result.configFault);
      TEST_ASSERT_TRUE(hasEffect(result.effects, EffectFlag::PARTIAL_WRITE));
    } else {
      TEST_ASSERT_FALSE(result.configFault.valid);
    }
  }

  for (uint8_t completed = 0; completed < resetTransfers(DeviceVariant::LDC1614);
       ++completed) {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    Config config = makeConfig(fake, DeviceVariant::LDC1614);
    initialize(driver, fake, config);
    fake.clearIo();
    TEST_ASSERT_TRUE(
        driver.startResetAndReapply(3300U + completed, 200).inProgress());
    for (uint8_t step = 0; step < completed; ++step) {
      TEST_ASSERT_TRUE(driver.poll(100, 1).inProgress());
    }
    const uint16_t before = fake.transferCalls;
    assertCode(Err::TIMEOUT, driver.poll(200, 0));
    TEST_ASSERT_EQUAL_UINT16(before, fake.transferCalls);
    OperationResult result = takeResult(driver);
    assertOutcome(TerminalOutcome::TIMED_OUT, result);
    assertStage(resetStage(DeviceVariant::LDC1614,
                           static_cast<uint8_t>(completed + 1U)),
                result.finalProgress);
    if (completed != 0U) {
      const ExpectedStage lastWrite = completed <= 3U
                                          ? resetStage(DeviceVariant::LDC1614, 1U)
                                          : configStage(
                                                DeviceVariant::LDC1614,
                                                static_cast<uint8_t>(completed - 3U));
      assertFaultStage(lastWrite, result.configFault);
      TEST_ASSERT_TRUE(hasEffect(result.effects, EffectFlag::PARTIAL_WRITE));
    } else {
      TEST_ASSERT_FALSE(result.configFault.valid);
    }
  }

  for (uint8_t completed = 0; completed < 6U; ++completed) {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    Config config = makeConfig(fake, DeviceVariant::LDC1612, true);
    initializeAndWake(driver, fake, config);
    fake.clearIo();
    fake.injectConversion(0, 0x00111111U);
    fake.injectConversion(1, 0x00222222U);
    TEST_ASSERT_TRUE(driver.startAcquire(ChannelMask{0x03}, 3400U + completed,
                                         200).inProgress());
    for (uint8_t step = 0; step < completed; ++step) {
      TEST_ASSERT_TRUE(driver.poll(100, 1).inProgress());
    }
    const uint16_t before = fake.transferCalls;
    assertCode(Err::TIMEOUT, driver.poll(200, 0));
    TEST_ASSERT_EQUAL_UINT16(before, fake.transferCalls);
    OperationResult result = takeResult(driver);
    assertOutcome(TerminalOutcome::TIMED_OUT, result);
    TEST_ASSERT_FALSE(result.hasSampleBatch);
    assertStage(acquireStage(ChannelMask{0x03},
                             static_cast<uint8_t>(completed + 1U)),
                result.finalProgress);
    TEST_ASSERT_EQUAL_UINT8(completed,
                            result.finalProgress.completedTransfers);
    TEST_ASSERT_EQUAL_UINT8(
        completed == 0U ? 0U : effectFlag(EffectFlag::READ_SIDE_EFFECTS),
        result.effects);
  }
}

void test_result_ids_fifo_backpressure_exactly_once_and_stale_clear() {
  FakeLdc1614Device fake;
  LDC1614::LDC1614 driver;
  Config config = makeConfig(fake);
  TEST_ASSERT_TRUE(driver.bind(config).ok());
  assertCode(Err::INVALID_PARAM, driver.startInitialize(0, DEADLINE_MS));

  TEST_ASSERT_TRUE(driver.startInitialize(41, DEADLINE_MS).inProgress());
  assertCode(Err::DUPLICATE_OPERATION_ID,
             driver.startInitialize(41, DEADLINE_MS));
  TEST_ASSERT_TRUE(pollToTerminal(driver).ok());
  assertCode(Err::DUPLICATE_OPERATION_ID,
             driver.startApplyConfig(41, DEADLINE_MS));

  TEST_ASSERT_TRUE(driver.startApplyConfig(42, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(pollToTerminal(driver).ok());
  assertCode(Err::RESULT_QUEUE_FULL,
             driver.startApplyConfig(43, DEADLINE_MS));

  OperationResult first = takeResult(driver);
  OperationResult second = takeResult(driver);
  TEST_ASSERT_EQUAL_UINT64(41, first.operationId);
  TEST_ASSERT_EQUAL_UINT64(42, second.operationId);
  TEST_ASSERT_FALSE(driver.resultAvailable());

  OperationResult stale;
  stale.operationId = 0xDEADBEEFU;
  stale.hasSampleBatch = true;
  assertCode(Err::RESULT_NOT_READY, driver.takeResult(stale));
  TEST_ASSERT_EQUAL_UINT64(0, stale.operationId);
  TEST_ASSERT_FALSE(stale.hasSampleBatch);

  TEST_ASSERT_TRUE(driver.startApplyConfig(41, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(pollToTerminal(driver).ok());
  TEST_ASSERT_EQUAL_UINT64(41, takeResult(driver).operationId);
}

void test_cancelled_result_survives_immediate_replacement_and_cancel_is_idempotent() {
  FakeLdc1614Device fake;
  LDC1614::LDC1614 driver;
  TEST_ASSERT_TRUE(driver.bind(makeConfig(fake)).ok());
  TEST_ASSERT_TRUE(driver.startInitialize(51, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(driver.poll(NOW_MS, 1).inProgress());
  const uint16_t beforeCancel = fake.transferCalls;
  TEST_ASSERT_TRUE(driver.cancelJob().ok());
  TEST_ASSERT_EQUAL_UINT16(beforeCancel, fake.transferCalls);
  TEST_ASSERT_TRUE(driver.cancelJob().ok());
  TEST_ASSERT_EQUAL_UINT16(beforeCancel, fake.transferCalls);

  TEST_ASSERT_TRUE(driver.startInitialize(52, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(pollToTerminal(driver).ok());
  OperationResult cancelled = takeResult(driver);
  OperationResult replacement = takeResult(driver);
  TEST_ASSERT_EQUAL_UINT64(51, cancelled.operationId);
  assertOutcome(TerminalOutcome::CANCELLED, cancelled);
  TEST_ASSERT_EQUAL_UINT64(52, replacement.operationId);
  assertOutcome(TerminalOutcome::SUCCESS, replacement);
}

void test_progress_is_cache_only_and_reports_exact_phase_and_budget() {
  FakeLdc1614Device fake;
  LDC1614::LDC1614 driver;
  TEST_ASSERT_TRUE(driver.bind(makeConfig(fake, DeviceVariant::LDC1614)).ok());
  TEST_ASSERT_TRUE(driver.startInitialize(61, DEADLINE_MS).inProgress());
  const JobProgress initial = driver.jobProgress();
  TEST_ASSERT_TRUE(initial.active);
  TEST_ASSERT_EQUAL_UINT64(61, initial.operationId);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(JobPhase::VERIFY_MANUFACTURER),
                          static_cast<uint8_t>(initial.phase));
  TEST_ASSERT_EQUAL_UINT8(26, initial.maximumTransfers);
  TEST_ASSERT_EQUAL_UINT16(0, fake.transferCalls);

  TEST_ASSERT_TRUE(driver.poll(NOW_MS, 1).inProgress());
  const uint16_t afterPoll = fake.transferCalls;
  const JobProgress progress = driver.jobProgress();
  TEST_ASSERT_EQUAL_UINT8(1, progress.completedTransfers);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(JobPhase::VERIFY_DEVICE),
                          static_cast<uint8_t>(progress.phase));
  (void)driver.jobProgress();
  (void)driver.resultAvailable();
  TEST_ASSERT_EQUAL_UINT16(afterPoll, fake.transferCalls);
}

void test_apply_exact_counts_failure_cancel_and_ambiguous_write_provenance() {
  const DeviceVariant variants[] = {DeviceVariant::LDC1612, DeviceVariant::LDC1614};
  for (DeviceVariant variant : variants) {
    {
      FakeLdc1614Device fake;
      LDC1614::LDC1614 driver;
      Config config = makeConfig(fake, variant);
      initialize(driver, fake, config);
      fake.clearIo();
      TEST_ASSERT_TRUE(driver.startApplyConfig(70, DEADLINE_MS).inProgress());
      TEST_ASSERT_TRUE(driver.poll(NOW_MS, 0).inProgress());
      TEST_ASSERT_EQUAL_UINT16(0, fake.transferCalls);
      TEST_ASSERT_TRUE(driver.poll(NOW_MS, 255).ok());
      TEST_ASSERT_EQUAL_UINT16(applyTransfers(variant), fake.transferCalls);
      assertOutcome(TerminalOutcome::SUCCESS, takeResult(driver));
    }

    const uint8_t maximum = applyTransfers(variant);
    for (uint8_t failed = 1; failed <= maximum; ++failed) {
      FakeLdc1614Device fake;
      LDC1614::LDC1614 driver;
      Config config = makeConfig(fake, variant);
      initialize(driver, fake, config);
      fake.clearIo();
      fake.failOnTransfer(failed, FORCED_WRITE, (failed & 1U) != 0U);
      TEST_ASSERT_TRUE(driver.startApplyConfig(7100U + failed, DEADLINE_MS).inProgress());
      assertCode(FORCED_WRITE.code, pollToTerminal(driver));
      TEST_ASSERT_EQUAL_UINT16_MESSAGE(failed, fake.transferCalls,
                                       "apply retried or ran past failure");
      OperationResult result = takeResult(driver);
      assertOutcome(TerminalOutcome::FAILED, result);
      const ExpectedStage failedStage = configStage(variant, failed);
      assertStage(failedStage, result.finalProgress);
      assertFaultStage(failedStage, result.configFault);
      TEST_ASSERT_EQUAL_INT32(FORCED_WRITE.detail, result.configFault.cause.detail);
      TEST_ASSERT_TRUE(hasEffect(result.effects, EffectFlag::INDETERMINATE_WRITE));
      if (failed > 1U) {
        TEST_ASSERT_TRUE(hasEffect(result.effects, EffectFlag::PARTIAL_WRITE));
      }
    }
  }

  for (uint8_t completed = 0; completed < applyTransfers(DeviceVariant::LDC1614);
       ++completed) {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    Config config = makeConfig(fake, DeviceVariant::LDC1614);
    initialize(driver, fake, config);
    fake.clearIo();
    TEST_ASSERT_TRUE(driver.startApplyConfig(7200U + completed, DEADLINE_MS).inProgress());
    for (uint8_t step = 0; step < completed; ++step) {
      TEST_ASSERT_TRUE(driver.poll(NOW_MS, 1).inProgress());
    }
    const uint16_t before = fake.transferCalls;
    TEST_ASSERT_TRUE(driver.cancelJob().ok());
    TEST_ASSERT_EQUAL_UINT16(before, fake.transferCalls);
    OperationResult result = takeResult(driver);
    assertOutcome(TerminalOutcome::CANCELLED, result);
    if (completed != 0U) {
      TEST_ASSERT_TRUE(hasEffect(result.effects, EffectFlag::PARTIAL_WRITE));
      assertFaultStage(configStage(DeviceVariant::LDC1614, completed),
                       result.configFault);
      TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::DIRTY),
                              static_cast<uint8_t>(driver.appliedConfigState()));
    }
  }
}

void test_config_fault_retains_exact_register_channel_phase_status_and_effect() {
  FakeLdc1614Device fake;
  LDC1614::LDC1614 driver;
  Config config = makeConfig(fake);
  initialize(driver, fake, config);
  fake.clearIo();
  // FORCE_SLEEP, RCOUNT0, SETTLECOUNT0, then CLOCK_DIVIDERS0.
  fake.failOnTransfer(4, FORCED_WRITE, true);
  TEST_ASSERT_TRUE(driver.startApplyConfig(80, DEADLINE_MS).inProgress());
  assertCode(Err::I2C_BUS, pollToTerminal(driver));
  OperationResult result = takeResult(driver);
  TEST_ASSERT_TRUE(result.configFault.valid);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(JobKind::APPLY_CONFIG),
                          static_cast<uint8_t>(result.configFault.job));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(JobPhase::WRITE_CLOCK_DIVIDERS),
                          static_cast<uint8_t>(result.configFault.phase));
  TEST_ASSERT_EQUAL_HEX8(cmd::REG_CLOCK_DIVIDERS0,
                         result.configFault.registerAddress);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Channel::CH0),
                          static_cast<uint8_t>(result.configFault.channel));
  TEST_ASSERT_EQUAL_INT32(FORCED_WRITE.detail, result.configFault.cause.detail);
  TEST_ASSERT_EQUAL_STRING(FORCED_WRITE.msg, result.configFault.cause.msg);
  TEST_ASSERT_TRUE(hasEffect(result.configFault.effects,
                             EffectFlag::INDETERMINATE_WRITE));
  TEST_ASSERT_EQUAL_HEX16(config.channel[0].finDivider << cmd::BIT_FIN_DIVIDER |
                              config.channel[0].frefDivider,
                          fake.reg[cmd::REG_CLOCK_DIVIDERS0]);

  fake.clearIo();
  TEST_ASSERT_TRUE(
      driver.writeRegister16(cmd::REG_RCOUNT1, config.channel[1].rcount).ok());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(JobKind::NONE),
                          static_cast<uint8_t>(driver.configFault().job));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(JobPhase::WRITE_RCOUNT),
                          static_cast<uint8_t>(driver.configFault().phase));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Channel::CH1),
                          static_cast<uint8_t>(driver.configFault().channel));
  fake.clearIo();
  TEST_ASSERT_TRUE(driver.startInitialize(81, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(pollToTerminal(driver).ok());
  TEST_ASSERT_EQUAL_UINT16(16, fake.transferCalls);
  assertOutcome(TerminalOutcome::SUCCESS, takeResult(driver));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::APPLIED_SLEEPING),
                          static_cast<uint8_t>(driver.appliedConfigState()));
}

void test_reset_reapply_exact_counts_all_failures_cancellation_and_no_retry() {
  const DeviceVariant variants[] = {DeviceVariant::LDC1612, DeviceVariant::LDC1614};
  for (DeviceVariant variant : variants) {
    {
      FakeLdc1614Device fake;
      LDC1614::LDC1614 driver;
      Config config = makeConfig(fake, variant);
      initialize(driver, fake, config);
      fake.clearIo();
      TEST_ASSERT_TRUE(driver.startResetAndReapply(90, DEADLINE_MS).inProgress());
      TEST_ASSERT_TRUE(driver.poll(NOW_MS, 255).ok());
      TEST_ASSERT_EQUAL_UINT16(resetTransfers(variant), fake.transferCalls);
      assertOutcome(TerminalOutcome::SUCCESS, takeResult(driver));
    }

    for (uint8_t failed = 1; failed <= resetTransfers(variant); ++failed) {
      FakeLdc1614Device fake;
      LDC1614::LDC1614 driver;
      Config config = makeConfig(fake, variant);
      initialize(driver, fake, config);
      fake.clearIo();
      const Status forced = failed == 1U || failed > 3U ? FORCED_WRITE : FORCED_READ;
      fake.failOnTransfer(failed, forced, failed == 1U);
      TEST_ASSERT_TRUE(
          driver.startResetAndReapply(9000U + failed, DEADLINE_MS).inProgress());
      assertCode(forced.code, pollToTerminal(driver));
      TEST_ASSERT_EQUAL_UINT16_MESSAGE(failed, fake.transferCalls,
                                       "reset/reapply retried or ran past failure");
      OperationResult result = takeResult(driver);
      assertOutcome(TerminalOutcome::FAILED, result);
      const ExpectedStage failedStage = resetStage(variant, failed);
      assertStage(failedStage, result.finalProgress);
      assertFaultStage(failedStage, result.configFault);
      TEST_ASSERT_EQUAL_INT32(forced.detail, result.configFault.cause.detail);
      TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::DIRTY),
                              static_cast<uint8_t>(driver.appliedConfigState()));
    }
  }

  for (uint8_t completed = 0; completed < resetTransfers(DeviceVariant::LDC1614);
       ++completed) {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    Config config = makeConfig(fake, DeviceVariant::LDC1614);
    initialize(driver, fake, config);
    fake.clearIo();
    TEST_ASSERT_TRUE(
        driver.startResetAndReapply(9200U + completed, DEADLINE_MS).inProgress());
    for (uint8_t step = 0; step < completed; ++step) {
      TEST_ASSERT_TRUE(driver.poll(NOW_MS, 1).inProgress());
    }
    const uint16_t before = fake.transferCalls;
    TEST_ASSERT_TRUE(driver.cancelJob().ok());
    TEST_ASSERT_EQUAL_UINT16(before, fake.transferCalls);
    OperationResult result = takeResult(driver);
    assertOutcome(TerminalOutcome::CANCELLED, result);
    if (completed != 0U) {
      TEST_ASSERT_TRUE(hasEffect(result.effects, EffectFlag::PARTIAL_WRITE));
      const ExpectedStage lastWrite = completed <= 3U
                                          ? resetStage(DeviceVariant::LDC1614, 1U)
                                          : configStage(
                                                DeviceVariant::LDC1614,
                                                static_cast<uint8_t>(completed - 3U));
      assertFaultStage(lastWrite, result.configFault);
      TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::DIRTY),
                              static_cast<uint8_t>(driver.appliedConfigState()));
    }
  }

  FakeLdc1614Device mismatched;
  LDC1614::LDC1614 mismatchDriver;
  Config mismatchConfig = makeConfig(mismatched);
  initialize(mismatchDriver, mismatched, mismatchConfig);
  mismatched.clearIo();
  TEST_ASSERT_TRUE(
      mismatchDriver.startResetAndReapply(9300, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(mismatchDriver.poll(NOW_MS, 1).inProgress());
  mismatched.reg[cmd::REG_DEVICE_ID] ^= 1U;
  assertCode(Err::DEVICE_NOT_FOUND,
             pollToTerminal(mismatchDriver, NOW_MS, 1));
  OperationResult mismatch = takeResult(mismatchDriver);
  TEST_ASSERT_TRUE(mismatch.configFault.valid);
  TEST_ASSERT_TRUE(hasEffect(mismatch.effects, EffectFlag::PARTIAL_WRITE));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::UNKNOWN),
                          static_cast<uint8_t>(mismatchDriver.appliedConfigState()));
  const uint16_t beforeApply = mismatched.transferCalls;
  assertCode(Err::CONFIG_DIRTY,
             mismatchDriver.startApplyConfig(9301, DEADLINE_MS));
  TEST_ASSERT_EQUAL_UINT16(beforeApply, mismatched.transferCalls);
}

OperationResult acquire(LDC1614::LDC1614& driver, ChannelMask mask,
                        OperationId operationId, uint64_t nowMs = NOW_MS,
                        uint8_t budget = 255) {
  TEST_ASSERT_TRUE(driver.startAcquire(mask, operationId, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(pollToTerminal(driver, nowMs, budget).ok());
  return takeResult(driver);
}

void test_acquire_exact_budget_order_status_evidence_and_each_error_channel() {
  FakeLdc1614Device fake;
  LDC1614::LDC1614 driver;
  Config config = makeConfig(fake, DeviceVariant::LDC1614, true);
  config.intbDisabled = false;
  config.intbAsserted = FakeLdc1614Device::readIntb;
  config.intbUser = &fake;
  initializeAndWake(driver, fake, config);

  for (uint8_t source = 0; source < 4U; ++source) {
    fake.clearIo();
    const uint32_t raw = 0x00100000U + source;
    fake.injectConversion(source, raw, cmd::MASK_DATA_ERR_AE,
                          cmd::MASK_STATUS_ERR_ALE);
    bool intb = false;
    const uint16_t beforeIntb = fake.transferCalls;
    TEST_ASSERT_TRUE(driver.readIntb(intb).ok());
    TEST_ASSERT_TRUE(intb);
    TEST_ASSERT_EQUAL_UINT16(beforeIntb, fake.transferCalls);
    const ChannelMask mask{static_cast<uint8_t>(1U << source)};
    OperationResult result = acquire(driver, mask, 100U + source);
    assertOutcome(TerminalOutcome::SUCCESS, result);
    TEST_ASSERT_TRUE(result.hasSampleBatch);
    TEST_ASSERT_EQUAL_UINT64(NOW_MS, result.sampleBatch.completedUptimeMs);
    TEST_ASSERT_EQUAL_UINT32(driver.configRevision(),
                             result.sampleBatch.configRevision);
    TEST_ASSERT_EQUAL_UINT16(4, fake.transferCalls);
    TEST_ASSERT_EQUAL_UINT8(4, result.finalProgress.completedTransfers);
    TEST_ASSERT_EQUAL_HEX8(cmd::REG_STATUS,
                           result.finalProgress.registerAddress);
    TEST_ASSERT_EQUAL_HEX8(cmd::REG_STATUS, fake.transferLog[0].reg);
    TEST_ASSERT_EQUAL_HEX8(cmd::regDataMsb(source), fake.transferLog[1].reg);
    TEST_ASSERT_EQUAL_HEX8(cmd::regDataLsb(source), fake.transferLog[2].reg);
    TEST_ASSERT_EQUAL_HEX8(cmd::REG_STATUS, fake.transferLog[3].reg);
    TEST_ASSERT_EQUAL_UINT8(source,
                            static_cast<uint8_t>(result.sampleBatch.statusBefore.errorChannel));
    TEST_ASSERT_TRUE(result.sampleBatch.statusBefore.errorAmplitudeLow);
    TEST_ASSERT_TRUE(result.sampleBatch.statusBefore.unreadChannels.contains(
        static_cast<Channel>(source)));
    TEST_ASSERT_EQUAL_HEX32(raw, result.sampleBatch.channel[source].rawCount28);
    TEST_ASSERT_TRUE(hasEffect(result.effects, EffectFlag::READ_SIDE_EFFECTS));
    TEST_ASSERT_FALSE(fake.intbAsserted);
    TEST_ASSERT_TRUE(driver.readIntb(intb).ok());
    TEST_ASSERT_FALSE(intb);
  }

  fake.clearIo();
  for (uint8_t ch = 0; ch < 4U; ++ch) {
    fake.injectConversion(ch, 0x00200000U + ch);
  }
  TEST_ASSERT_TRUE(driver.startAcquire(ChannelMask{0x0F}, 105, DEADLINE_MS).inProgress());
  TEST_ASSERT_EQUAL_UINT8(10, driver.jobProgress().maximumTransfers);
  TEST_ASSERT_TRUE(driver.poll(NOW_MS, 0).inProgress());
  TEST_ASSERT_EQUAL_UINT16(0, fake.transferCalls);
  TEST_ASSERT_TRUE(driver.poll(NOW_MS, 255).ok());
  TEST_ASSERT_EQUAL_UINT16(10, fake.transferCalls);
  TEST_ASSERT_EQUAL_HEX8(0x0F, takeResult(driver).sampleBatch.validChannels.bits);
}

void test_acquire_failure_at_every_phase_preserves_prior_complete_publication() {
  for (uint8_t failed = 1; failed <= 6U; ++failed) {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    Config config = makeConfig(fake, DeviceVariant::LDC1612, true);
    initializeAndWake(driver, fake, config);

    fake.injectConversion(0, 0x00111111U);
    fake.injectConversion(1, 0x00222222U);
    TEST_ASSERT_TRUE(
        driver.startAcquire(ChannelMask{0x03}, 201, DEADLINE_MS).inProgress());
    TEST_ASSERT_TRUE(pollToTerminal(driver).ok());
    // Keep the complete batch queued while the next operation fails.

    fake.clearIo();
    fake.injectConversion(0, 0x00333333U);
    fake.injectConversion(1, 0x00444444U);
    fake.failOnTransfer(failed, FORCED_READ);
    TEST_ASSERT_TRUE(
        driver.startAcquire(ChannelMask{0x03}, 202, DEADLINE_MS).inProgress());
    assertCode(FORCED_READ.code, pollToTerminal(driver));
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(failed, fake.transferCalls,
                                     "acquisition retried or ran past failure");

    OperationResult prior = takeResult(driver);
    OperationResult failure = takeResult(driver);
    TEST_ASSERT_EQUAL_UINT64(201, prior.operationId);
    TEST_ASSERT_TRUE(prior.hasSampleBatch);
    TEST_ASSERT_EQUAL_HEX32(0x00111111U, prior.sampleBatch.channel[0].rawCount28);
    TEST_ASSERT_EQUAL_HEX32(0x00222222U, prior.sampleBatch.channel[1].rawCount28);
    TEST_ASSERT_EQUAL_UINT64(202, failure.operationId);
    assertOutcome(TerminalOutcome::FAILED, failure);
    TEST_ASSERT_FALSE(failure.hasSampleBatch);
    TEST_ASSERT_EQUAL_UINT8(failed,
                            failure.finalProgress.completedTransfers);
    assertStage(acquireStage(ChannelMask{0x03}, failed),
                failure.finalProgress);
    TEST_ASSERT_EQUAL_INT32(FORCED_READ.detail, failure.status.detail);
    TEST_ASSERT_TRUE(hasEffect(failure.effects, EffectFlag::READ_SIDE_EFFECTS));
  }
}

void test_acquire_cancel_every_phase_is_silent_atomic_and_restartable() {
  for (uint8_t completed = 0; completed < 6U; ++completed) {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    Config config = makeConfig(fake, DeviceVariant::LDC1612, true);
    initializeAndWake(driver, fake, config);
    fake.clearIo();
    fake.injectConversion(0, 0x00111111U);
    fake.injectConversion(1, 0x00222222U);
    TEST_ASSERT_TRUE(driver.startAcquire(ChannelMask{0x03}, 300U + completed,
                                         DEADLINE_MS).inProgress());
    for (uint8_t step = 0; step < completed; ++step) {
      TEST_ASSERT_TRUE(driver.poll(NOW_MS, 1).inProgress());
    }
    const uint16_t before = fake.transferCalls;
    TEST_ASSERT_TRUE(driver.cancelJob().ok());
    TEST_ASSERT_EQUAL_UINT16(before, fake.transferCalls);
    OperationResult cancelled = takeResult(driver);
    assertOutcome(TerminalOutcome::CANCELLED, cancelled);
    TEST_ASSERT_FALSE(cancelled.hasSampleBatch);

    fake.injectConversion(0, 0x00555555U);
    OperationResult replacement = acquire(driver, channelBit(Channel::CH0),
                                          400U + completed);
    assertOutcome(TerminalOutcome::SUCCESS, replacement);
    TEST_ASSERT_EQUAL_HEX32(0x00555555U,
                            replacement.sampleBatch.channel[0].rawCount28);
  }
}

void test_ldc1614_acquire_failure_cancel_deadline_and_fresh_stale_paths() {
  for (uint8_t failed = 1U; failed <= 10U; ++failed) {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    Config config = makeConfig(fake, DeviceVariant::LDC1614, true);
    initializeAndWake(driver, fake, config);
    fake.clearIo();
    for (uint8_t channel = 0; channel < 4U; ++channel) {
      fake.injectConversion(channel, 0x00100000U + channel);
    }
    fake.failOnTransfer(failed, FORCED_READ);
    TEST_ASSERT_TRUE(
        driver.startAcquire(ChannelMask{0x0F}, 13000U + failed,
                            DEADLINE_MS).inProgress());
    assertCode(FORCED_READ.code, pollToTerminal(driver));
    OperationResult result = takeResult(driver);
    assertOutcome(TerminalOutcome::FAILED, result);
    assertStage(acquireStage(ChannelMask{0x0F}, failed), result.finalProgress);
    TEST_ASSERT_FALSE(result.hasSampleBatch);
  }

  for (uint8_t completed = 0U; completed < 10U; ++completed) {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    Config config = makeConfig(fake, DeviceVariant::LDC1614, true);
    initializeAndWake(driver, fake, config);
    fake.clearIo();
    for (uint8_t channel = 0; channel < 4U; ++channel) {
      fake.injectConversion(channel, 0x00200000U + channel);
    }
    TEST_ASSERT_TRUE(
        driver.startAcquire(ChannelMask{0x0F}, 13100U + completed,
                            200U).inProgress());
    for (uint8_t step = 0U; step < completed; ++step) {
      TEST_ASSERT_TRUE(driver.poll(100U, 1U).inProgress());
    }
    const uint16_t before = fake.transferCalls;
    if ((completed & 1U) == 0U) {
      TEST_ASSERT_TRUE(driver.cancelJob().ok());
      TEST_ASSERT_EQUAL_UINT16(before, fake.transferCalls);
      OperationResult result = takeResult(driver);
      assertOutcome(TerminalOutcome::CANCELLED, result);
      assertStage(acquireStage(ChannelMask{0x0F},
                               static_cast<uint8_t>(completed + 1U)),
                  result.finalProgress);
    } else {
      assertCode(Err::TIMEOUT, driver.poll(200U, 0U));
      TEST_ASSERT_EQUAL_UINT16(before, fake.transferCalls);
      OperationResult result = takeResult(driver);
      assertOutcome(TerminalOutcome::TIMED_OUT, result);
      assertStage(acquireStage(ChannelMask{0x0F},
                               static_cast<uint8_t>(completed + 1U)),
                  result.finalProgress);
    }
  }

  FakeLdc1614Device fake;
  LDC1614::LDC1614 driver;
  Config config = makeConfig(fake);
  initializeAndWake(driver, fake, config);
  fake.injectConversion(0, 0x00555555U);
  OperationResult fresh = acquire(driver, channelBit(Channel::CH0), 13200U);
  TEST_ASSERT_TRUE(fresh.sampleBatch.freshChannels.contains(Channel::CH0));
  TEST_ASSERT_TRUE(hasSampleQuality(fresh.sampleBatch.channel[0].quality,
                                    SampleQualityFlag::FRESH));
  TEST_ASSERT_FALSE(hasSampleQuality(fresh.sampleBatch.channel[0].quality,
                                     SampleQualityFlag::STALE));
  OperationResult stale = acquire(driver, channelBit(Channel::CH0), 13201U);
  TEST_ASSERT_FALSE(stale.sampleBatch.freshChannels.contains(Channel::CH0));
  TEST_ASSERT_FALSE(stale.sampleBatch.validChannels.contains(Channel::CH0));
  TEST_ASSERT_TRUE(hasSampleQuality(stale.sampleBatch.channel[0].quality,
                                    SampleQualityFlag::STALE));
}

void test_acquire_detects_conversion_overrun_without_breaking_shadow_coherency() {
  FakeLdc1614Device fake;
  LDC1614::LDC1614 driver;
  Config config = makeConfig(fake);
  initializeAndWake(driver, fake, config);
  fake.clearIo();
  fake.injectConversion(0, 0x01234567U);
  // STATUS-before is transfer 1, DATA0_MSB is transfer 2. The new conversion
  // arrives after the MSB has latched the old matching LSB.
  fake.scheduleConversionAfter(2, 0, 0x07654321U);
  OperationResult result = acquire(driver, channelBit(Channel::CH0), 501);
  TEST_ASSERT_EQUAL_HEX32(0x01234567U, result.sampleBatch.channel[0].rawCount28);
  TEST_ASSERT_TRUE(result.sampleBatch.overrunChannels.contains(Channel::CH0));
  TEST_ASSERT_TRUE(hasSampleQuality(result.sampleBatch.channel[0].quality,
                                    SampleQualityFlag::DATA_LOST));
  TEST_ASSERT_FALSE(result.sampleBatch.validChannels.contains(Channel::CH0));
}

void test_sample_quality_endpoints_watchdog_amplitude_and_status_zero_count() {
  ChannelSample under = LDC1614::LDC1614::decodeChannelSample(0x0000, 0x0000);
  TEST_ASSERT_TRUE(hasSampleQuality(under.quality, SampleQualityFlag::UNDER_RANGE));
  ChannelSample over = LDC1614::LDC1614::decodeChannelSample(0x0FFF, 0xFFFF);
  TEST_ASSERT_TRUE(hasSampleQuality(over.quality, SampleQualityFlag::OVER_RANGE));
  ChannelSample watchdog = LDC1614::LDC1614::decodeChannelSample(
      static_cast<uint16_t>(cmd::MASK_DATA_ERR_WD | 0x0001U), 0x0002);
  TEST_ASSERT_TRUE(hasSampleQuality(watchdog.quality, SampleQualityFlag::WATCHDOG));
  ChannelSample amplitude = LDC1614::LDC1614::decodeChannelSample(
      static_cast<uint16_t>(cmd::MASK_DATA_ERR_AE | 0x0001U), 0x0002);
  TEST_ASSERT_TRUE(hasSampleQuality(amplitude.quality,
                                    SampleQualityFlag::AMPLITUDE_SUSPECT));

  struct Case {
    uint32_t raw;
    uint16_t dataErrors;
    uint16_t statusErrors;
    SampleQualityFlag expected;
    bool expectedValid;
  };
  const Case cases[] = {
      {0x00000000U, 0, 0, SampleQualityFlag::UNDER_RANGE, false},
      {0x0FFFFFFFU, 0, 0, SampleQualityFlag::OVER_RANGE, false},
      {0x00010002U, cmd::MASK_DATA_ERR_WD, cmd::MASK_STATUS_ERR_WD,
       SampleQualityFlag::WATCHDOG, false},
      {0x00010002U, cmd::MASK_DATA_ERR_AE, cmd::MASK_STATUS_ERR_ALE,
       SampleQualityFlag::AMPLITUDE_SUSPECT, true},
      {0x00010002U, 0, cmd::MASK_STATUS_ERR_ZC,
       SampleQualityFlag::ZERO_COUNT, false},
  };
  for (uint8_t index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
    FakeLdc1614Device fake;
    LDC1614::LDC1614 driver;
    Config config = makeConfig(fake);
    initializeAndWake(driver, fake, config);
    fake.injectConversion(0, cases[index].raw, cases[index].dataErrors,
                          cases[index].statusErrors);
    OperationResult result = acquire(driver, channelBit(Channel::CH0),
                                     600U + index);
    const ChannelSample& sample = result.sampleBatch.channel[0];
    TEST_ASSERT_TRUE(hasSampleQuality(sample.quality, cases[index].expected));
    TEST_ASSERT_EQUAL(cases[index].expectedValid,
                      result.sampleBatch.validChannels.contains(Channel::CH0));
    TEST_ASSERT_TRUE(result.sampleBatch.errorChannels.contains(Channel::CH0));
  }
}

void test_dirty_unknown_invalidation_rejects_acquire_and_matching_return_replays_all() {
  FakeLdc1614Device fake;
  LDC1614::LDC1614 driver;
  Config config = makeConfig(fake, DeviceVariant::LDC1614);
  initializeAndWake(driver, fake, config);
  fake.clearIo();

  const Status brownout = Status::Error(Err::I2C_BUS, "owner observed brownout", -99123);
  driver.invalidateAppliedState(brownout);
  TEST_ASSERT_EQUAL_UINT16(0, fake.transferCalls);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::UNKNOWN),
                          static_cast<uint8_t>(driver.appliedConfigState()));
  TEST_ASSERT_TRUE(driver.configFault().valid);
  TEST_ASSERT_EQUAL_INT32(brownout.detail, driver.configFault().cause.detail);
  assertCode(Err::CONFIG_DIRTY,
             driver.startAcquire(channelBit(Channel::CH0), 701, DEADLINE_MS));
  TEST_ASSERT_EQUAL_UINT16(0, fake.transferCalls);

  TEST_ASSERT_TRUE(driver.startInitialize(702, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(pollToTerminal(driver).ok());
  TEST_ASSERT_EQUAL_UINT16(26, fake.transferCalls);
  assertOutcome(TerminalOutcome::SUCCESS, takeResult(driver));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::APPLIED_SLEEPING),
                          static_cast<uint8_t>(driver.appliedConfigState()));
}

void test_update_rebind_end_and_repeated_lifecycle_are_zero_i2c_and_revisioned() {
  FakeLdc1614Device first;
  FakeLdc1614Device second;
  LDC1614::LDC1614 driver;
  Config firstConfig = makeConfig(first);
  initialize(driver, first, firstConfig);
  TEST_ASSERT_TRUE(driver.wake().ok());
  first.clearIo();

  Config forbiddenWhileActive = firstConfig;
  forbiddenWhileActive.channel[0].rcount++;
  const uint32_t activeRevision = driver.configRevision();
  assertCode(Err::BUSY, driver.updateDesiredConfig(forbiddenWhileActive));
  TEST_ASSERT_EQUAL_UINT16(0, first.transferCalls);
  TEST_ASSERT_EQUAL_UINT32(activeRevision, driver.configRevision());
  TEST_ASSERT_TRUE(driver.sleep().ok());
  first.clearIo();

  Config invalid = firstConfig;
  invalid.channel[0].driveCurrentCode = 0xFF;
  const uint32_t originalRevision = driver.configRevision();
  assertCode(Err::INVALID_CONFIG, driver.updateDesiredConfig(invalid));
  TEST_ASSERT_EQUAL_UINT16(0, first.transferCalls);
  TEST_ASSERT_EQUAL_UINT32(originalRevision, driver.configRevision());

  Config changedBinding = firstConfig;
  changedBinding.i2cUser = &second;
  assertCode(Err::INVALID_CONFIG, driver.updateDesiredConfig(changedBinding));
  changedBinding = firstConfig;
  changedBinding.i2cAddress = I2cAddress::ADDR_VDD;
  assertCode(Err::INVALID_CONFIG, driver.updateDesiredConfig(changedBinding));
  changedBinding = firstConfig;
  changedBinding.variant = DeviceVariant::LDC1614;
  changedBinding.channel[2] = changedBinding.channel[0];
  changedBinding.channel[3] = changedBinding.channel[0];
  assertCode(Err::INVALID_CONFIG, driver.updateDesiredConfig(changedBinding));
  TEST_ASSERT_EQUAL_UINT16(0, first.transferCalls);
  TEST_ASSERT_EQUAL_UINT16(0, second.transferCalls);
  TEST_ASSERT_EQUAL_UINT32(originalRevision, driver.configRevision());

  Config changed = firstConfig;
  changed.channel[0].rcount++;
  const uint32_t beforeRevision = driver.configRevision();
  TEST_ASSERT_TRUE(driver.updateDesiredConfig(changed).ok());
  TEST_ASSERT_EQUAL_UINT16(0, first.transferCalls);
  TEST_ASSERT_EQUAL_UINT32(beforeRevision + 1U, driver.configRevision());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::DIRTY),
                          static_cast<uint8_t>(driver.appliedConfigState()));

  TEST_ASSERT_TRUE(driver.startApplyConfig(801, DEADLINE_MS).inProgress());
  TEST_ASSERT_EQUAL_UINT32(driver.configRevision(),
                           driver.jobProgress().configRevision);
  assertCode(Err::BUSY, driver.updateDesiredConfig(firstConfig));
  TEST_ASSERT_TRUE(driver.cancelJob().ok());
  OperationResult revisioned = takeResult(driver);
  TEST_ASSERT_EQUAL_UINT32(driver.configRevision(), revisioned.configRevision);

  driver.end();
  TEST_ASSERT_EQUAL_UINT16(0, first.transferCalls);
  TEST_ASSERT_FALSE(driver.isBound());
  OperationResult discarded;
  assertCode(Err::RESULT_NOT_READY, driver.takeResult(discarded));

  Config secondConfig = makeConfig(second);
  secondConfig.i2cAddress = I2cAddress::ADDR_VDD;
  second.acceptedAddress = 0x2B;
  TEST_ASSERT_TRUE(driver.bind(secondConfig).ok());
  TEST_ASSERT_EQUAL_UINT16(0, second.transferCalls);
  TEST_ASSERT_TRUE(driver.startInitialize(802, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(pollToTerminal(driver).ok());
  TEST_ASSERT_EQUAL_UINT16(0, first.transferCalls);
  TEST_ASSERT_EQUAL_UINT16(16, second.transferCalls);
  TEST_ASSERT_EQUAL_HEX8(0x2B, second.transferLog[0].address);
}

void test_owner_safe_single_transfer_and_diagnostic_dirty_contracts() {
  FakeLdc1614Device fake;
  LDC1614::LDC1614 driver;
  Config config = makeConfig(fake);
  initialize(driver, fake, config);
  fake.clearIo();

  TEST_ASSERT_TRUE(driver.wake().ok());
  TEST_ASSERT_EQUAL_UINT16(1, fake.transferCalls);
  fake.injectConversion(0, 0x00123456U);
  DeviceStatus status;
  TEST_ASSERT_TRUE(driver.readDeviceStatus(status).ok());
  TEST_ASSERT_EQUAL_UINT16(2, fake.transferCalls);
  TEST_ASSERT_TRUE(driver.sleep().ok());
  TEST_ASSERT_EQUAL_UINT16(3, fake.transferCalls);

  TEST_ASSERT_TRUE(driver.writeRegister16(cmd::REG_CONFIG, cmd::CONFIG_DEFAULT).ok());
  TEST_ASSERT_EQUAL_UINT16(4, fake.transferCalls);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::DIRTY),
                          static_cast<uint8_t>(driver.appliedConfigState()));
  const ConfigFault rawFault = driver.configFault();
  TEST_ASSERT_TRUE(rawFault.valid);
  Config revised = driver.config();
  revised.channel[0].rcount++;
  TEST_ASSERT_TRUE(driver.updateDesiredConfig(revised).ok());
  TEST_ASSERT_TRUE(driver.configFault().valid);
  TEST_ASSERT_EQUAL_HEX8(rawFault.registerAddress,
                         driver.configFault().registerAddress);
  TEST_ASSERT_EQUAL_INT32(rawFault.cause.detail,
                          driver.configFault().cause.detail);

  fake.clearIo();
  fake.failOnTransfer(1, FORCED_WRITE, true);
  assertCode(FORCED_WRITE.code,
             driver.writeRegister16(cmd::REG_RCOUNT1, 0x1234));
  const ConfigFault diagnosticFailure = driver.configFault();
  TEST_ASSERT_TRUE(diagnosticFailure.valid);
  TEST_ASSERT_EQUAL_HEX8(cmd::REG_RCOUNT1,
                         diagnosticFailure.registerAddress);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Channel::CH1),
                          static_cast<uint8_t>(diagnosticFailure.channel));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(JobPhase::WRITE_RCOUNT),
                          static_cast<uint8_t>(diagnosticFailure.phase));
  TEST_ASSERT_EQUAL_INT32(FORCED_WRITE.detail,
                          diagnosticFailure.cause.detail);

  FakeLdc1614Device noDrdyFake;
  LDC1614::LDC1614 noDrdy;
  Config noDrdyConfig = makeConfig(noDrdyFake);
  noDrdyConfig.errorReporting.dataReady = false;
  initializeAndWake(noDrdy, noDrdyFake, noDrdyConfig);
  noDrdyFake.clearIo();
  bool ready = true;
  DeviceStatus observed;
  assertCode(Err::INVALID_CONFIG, noDrdy.readDataReady(ready, observed));
  TEST_ASSERT_FALSE(ready);
  TEST_ASSERT_EQUAL_UINT16(0, noDrdyFake.transferCalls);

  FakeLdc1614Device intbFake;
  LDC1614::LDC1614 intbDriver;
  Config intbConfig = makeConfig(intbFake);
  intbConfig.intbDisabled = false;
  intbConfig.intbAsserted = FakeLdc1614Device::readIntb;
  intbConfig.intbUser = &intbFake;
  initializeAndWake(intbDriver, intbFake, intbConfig);
  intbFake.clearIo();
  ready = true;
  observed = DeviceStatus{};
  TEST_ASSERT_TRUE(intbDriver.readDataReady(ready, observed).ok());
  TEST_ASSERT_FALSE(ready);
  TEST_ASSERT_FALSE(observed.observed);
  TEST_ASSERT_EQUAL_UINT16(0, intbFake.transferCalls);
  intbFake.stickyStatusErrors = cmd::MASK_STATUS_ERR_ZC;
  intbFake.errorChannel = 0;
  intbFake.unreadMask = 0;
  intbFake.intbAsserted = true;
  ready = true;
  observed = DeviceStatus{};
  TEST_ASSERT_TRUE(intbDriver.readDataReady(ready, observed).ok());
  TEST_ASSERT_FALSE(ready);
  TEST_ASSERT_TRUE(observed.errorZeroCount);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Channel::CH0),
                          static_cast<uint8_t>(observed.errorChannel));
  TEST_ASSERT_EQUAL_UINT16(1, intbFake.transferCalls);
  TEST_ASSERT_EQUAL_HEX16(0, intbFake.stickyStatusErrors);
}

void test_single_transfer_boundaries_confirmed_nack_and_lifecycle_cleanup() {
  FakeLdc1614Device fake;
  LDC1614::LDC1614 driver;
  Config config = makeConfig(fake);
  TEST_ASSERT_TRUE(driver.bind(config).ok());

  bool ready = true;
  DeviceStatus observed;
  assertCode(Err::CONFIG_DIRTY, driver.readDataReady(ready, observed));
  TEST_ASSERT_FALSE(ready);
  TEST_ASSERT_EQUAL_UINT16(0U, fake.transferCalls);

  TEST_ASSERT_TRUE(driver.startInitialize(14000U, DEADLINE_MS).inProgress());
  assertCode(Err::BUSY, driver.readDataReady(ready, observed));
  TEST_ASSERT_TRUE(driver.poll(NOW_MS, 3U).inProgress());
  const uint16_t beforeEnd = fake.transferCalls;
  driver.end();
  TEST_ASSERT_EQUAL_UINT16(beforeEnd, fake.transferCalls);
  TEST_ASSERT_FALSE(driver.isBound());
  TEST_ASSERT_FALSE(driver.resultAvailable());

  fake.clearIo();
  TEST_ASSERT_TRUE(driver.bind(config).ok());
  TEST_ASSERT_TRUE(driver.startInitialize(14001U, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(driver.poll(NOW_MS, 3U).inProgress());
  const uint16_t beforeInvalidate = fake.transferCalls;
  driver.invalidateAppliedState(
      Status::Error(Err::I2C_BUS, "owner recovery", -14001));
  TEST_ASSERT_EQUAL_UINT16(beforeInvalidate, fake.transferCalls);
  OperationResult cancelled = takeResult(driver);
  assertOutcome(TerminalOutcome::CANCELLED, cancelled);
  assertCode(Err::CONFIG_DIRTY,
             driver.startAcquire(channelBit(Channel::CH0), 14002U,
                                 DEADLINE_MS));

  driver.end();
  fake.clearIo();
  TEST_ASSERT_TRUE(driver.bind(config).ok());
  TEST_ASSERT_TRUE(driver.startInitialize(14003U, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(pollToTerminal(driver).ok());
  (void)takeResult(driver);
  TEST_ASSERT_TRUE(driver.sleep().ok());
  TEST_ASSERT_EQUAL_UINT16(initTransfers(DeviceVariant::LDC1612),
                           fake.transferCalls);
  TEST_ASSERT_TRUE(driver.wake().ok());
  TEST_ASSERT_EQUAL_UINT16(initTransfers(DeviceVariant::LDC1612) + 1U,
                           fake.transferCalls);
  fake.injectConversion(0, 0x00123456U);
  ready = false;
  observed = DeviceStatus{};
  TEST_ASSERT_TRUE(driver.readDataReady(ready, observed).ok());
  TEST_ASSERT_TRUE(ready);
  TEST_ASSERT_TRUE(observed.observed);
  TEST_ASSERT_EQUAL_UINT16(initTransfers(DeviceVariant::LDC1612) + 2U,
                           fake.transferCalls);
  TEST_ASSERT_TRUE(driver.wake().ok());
  TEST_ASSERT_TRUE(driver.sleep().ok());
  TEST_ASSERT_EQUAL_UINT16(initTransfers(DeviceVariant::LDC1612) + 3U,
                           fake.transferCalls);
  TEST_ASSERT_TRUE(driver.sleep().ok());

  fake.reg[cmd::REG_DRIVE_CURRENT1] =
      static_cast<uint16_t>(0x1BU << cmd::BIT_INIT_IDRIVE);
  uint8_t initDrive = 0xFFU;
  TEST_ASSERT_TRUE(driver.readInitDriveCurrent(Channel::CH1, initDrive).ok());
  TEST_ASSERT_EQUAL_UINT8(0x1BU, initDrive);
  const uint16_t beforeInvalidChannel = fake.transferCalls;
  assertCode(Err::INVALID_PARAM,
             driver.readInitDriveCurrent(Channel::CH2, initDrive));
  TEST_ASSERT_EQUAL_UINT8(0U, initDrive);
  TEST_ASSERT_EQUAL_UINT16(beforeInvalidChannel, fake.transferCalls);

  uint16_t raw = 0xFFFFU;
  assertCode(Err::INVALID_PARAM,
             driver.readRegister16(cmd::REG_DATA2_MSB, raw));
  TEST_ASSERT_EQUAL_UINT16(0U, raw);
  assertCode(Err::INVALID_PARAM,
             driver.writeRegister16(cmd::REG_STATUS, 0U));
  assertCode(Err::INVALID_PARAM,
             driver.writeRegister16(cmd::REG_MANUFACTURER_ID, 0U));
  TEST_ASSERT_EQUAL_UINT16(beforeInvalidChannel, fake.transferCalls);

  fake.clearIo();
  fake.failOnTransfer(1U, FORCED_READ);
  raw = 0xFFFFU;
  const Status rawReadFailure = driver.readRegister16(cmd::REG_RCOUNT0, raw);
  assertCode(FORCED_READ.code, rawReadFailure);
  TEST_ASSERT_EQUAL_INT32(FORCED_READ.detail, rawReadFailure.detail);
  TEST_ASSERT_EQUAL_UINT16(0U, raw);
  TEST_ASSERT_EQUAL_UINT16(1U, fake.transferCalls);

  fake.acceptedAddress = 0x2BU;
  const uint16_t priorConfig = fake.reg[cmd::REG_CONFIG];
  TEST_ASSERT_TRUE(driver.startApplyConfig(14004U, DEADLINE_MS).inProgress());
  assertCode(Err::I2C_NACK_ADDR, pollToTerminal(driver));
  OperationResult applyNack = takeResult(driver);
  assertOutcome(TerminalOutcome::FAILED, applyNack);
  TEST_ASSERT_EQUAL_UINT8(0U, applyNack.effects);
  TEST_ASSERT_FALSE(applyNack.configFault.valid);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::APPLIED_SLEEPING),
                          static_cast<uint8_t>(driver.appliedConfigState()));
  assertCode(Err::I2C_NACK_ADDR, driver.wake());
  TEST_ASSERT_EQUAL_HEX16(priorConfig, fake.reg[cmd::REG_CONFIG]);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::APPLIED_SLEEPING),
                          static_cast<uint8_t>(driver.appliedConfigState()));
  TEST_ASSERT_FALSE(driver.configFault().valid);

  fake.acceptedAddress = 0x2AU;
  fake.clearIo();
  fake.failOnTransfer(1U, FORCED_WRITE, true);
  assertCode(FORCED_WRITE.code, driver.wake());
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::DIRTY),
                          static_cast<uint8_t>(driver.appliedConfigState()));
  TEST_ASSERT_TRUE(hasEffect(driver.configFault().effects,
                             EffectFlag::INDETERMINATE_WRITE));

  FakeLdc1614Device partialNackFake;
  LDC1614::LDC1614 partialNackDriver;
  Config partialNackConfig = makeConfig(partialNackFake);
  initialize(partialNackDriver, partialNackFake, partialNackConfig);
  partialNackFake.clearIo();
  partialNackFake.failOnTransfer(
      2U, Status::Error(Err::I2C_NACK_ADDR, "forced address NACK", 0x2A));
  TEST_ASSERT_TRUE(
      partialNackDriver.startApplyConfig(14005U, DEADLINE_MS).inProgress());
  assertCode(Err::I2C_NACK_ADDR, pollToTerminal(partialNackDriver));
  OperationResult partialNack = takeResult(partialNackDriver);
  TEST_ASSERT_TRUE(hasEffect(partialNack.effects, EffectFlag::PARTIAL_WRITE));
  TEST_ASSERT_FALSE(
      hasEffect(partialNack.effects, EffectFlag::INDETERMINATE_WRITE));
  assertFaultStage(configStage(DeviceVariant::LDC1612, 2U),
                   partialNack.configFault);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AppliedConfigState::DIRTY),
                          static_cast<uint8_t>(
                              partialNackDriver.appliedConfigState()));

  FakeLdc1614Device gpioFake;
  LDC1614::LDC1614 gpioDriver;
  Config gpioConfig = makeConfig(gpioFake);
  gpioConfig.intbDisabled = false;
  gpioConfig.intbAsserted = failingIntb;
  initializeAndWake(gpioDriver, gpioFake, gpioConfig);
  gpioFake.clearIo();
  ready = true;
  observed = DeviceStatus{};
  const Status gpioFailure = gpioDriver.readDataReady(ready, observed);
  assertCode(Err::I2C_ERROR, gpioFailure);
  TEST_ASSERT_EQUAL_INT32(-9001, gpioFailure.detail);
  TEST_ASSERT_FALSE(ready);
  TEST_ASSERT_EQUAL_UINT16(0U, gpioFake.transferCalls);
}

void test_transport_stats_are_diagnostic_only_and_failures_never_suppress_requests() {
  FakeLdc1614Device fake;
  LDC1614::LDC1614 driver;
  Config config = makeConfig(fake);
  initializeAndWake(driver, fake, config);
  fake.clearIo();
  for (uint8_t attempt = 1; attempt <= 6U; ++attempt) {
    fake.failOnTransfer(attempt, FORCED_READ);
    DeviceStatus status;
    assertCode(FORCED_READ.code, driver.readDeviceStatus(status));
  }
  const TransportStats stats = driver.transportStats();
  TEST_ASSERT_GREATER_OR_EQUAL_UINT32(6, stats.totalFailures);
  TEST_ASSERT_EQUAL_INT32(FORCED_READ.detail, stats.lastStatus.detail);

  fake.failTransfer = 0;
  DeviceStatus status;
  TEST_ASSERT_TRUE(driver.readDeviceStatus(status).ok());
  TEST_ASSERT_EQUAL_UINT16(7, fake.transferCalls);
}

void test_pure_error_status_frequency_and_timing_helpers_cover_boundaries() {
  const uint16_t allMask = LDC1614::LDC1614::encodeErrorReporting(ErrorReporting::all());
  TEST_ASSERT_EQUAL_HEX16(cmd::MASK_ERRCFG_ALLOWED, allMask);
  const uint16_t individualMasks[] = {
      cmd::MASK_ERRCFG_UR_ERR2OUT, cmd::MASK_ERRCFG_OR_ERR2OUT,
      cmd::MASK_ERRCFG_WD_ERR2OUT, cmd::MASK_ERRCFG_AH_ERR2OUT,
      cmd::MASK_ERRCFG_AL_ERR2OUT, cmd::MASK_ERRCFG_UR_ERR2INT,
      cmd::MASK_ERRCFG_OR_ERR2INT, cmd::MASK_ERRCFG_WD_ERR2INT,
      cmd::MASK_ERRCFG_AH_ERR2INT, cmd::MASK_ERRCFG_AL_ERR2INT,
      cmd::MASK_ERRCFG_ZC_ERR2INT, cmd::MASK_ERRCFG_DRDY_2INT};
  for (uint8_t index = 0; index < 12U; ++index) {
    ErrorReporting reporting{};
    switch (index) {
      case 0: reporting.dataUnderRange = true; break;
      case 1: reporting.dataOverRange = true; break;
      case 2: reporting.dataWatchdog = true; break;
      case 3: reporting.dataAmplitudeHigh = true; break;
      case 4: reporting.dataAmplitudeLow = true; break;
      case 5: reporting.statusUnderRange = true; break;
      case 6: reporting.statusOverRange = true; break;
      case 7: reporting.statusWatchdog = true; break;
      case 8: reporting.statusAmplitudeHigh = true; break;
      case 9: reporting.statusAmplitudeLow = true; break;
      case 10: reporting.statusZeroCount = true; break;
      default: reporting.dataReady = true; break;
    }
    TEST_ASSERT_EQUAL_HEX16(
        individualMasks[index],
        LDC1614::LDC1614::encodeErrorReporting(reporting));
  }

  uint16_t nominalMicroamps = 0;
  TEST_ASSERT_TRUE(LDC1614::LDC1614::nominalDriveCurrentMicroamps(
                       DriveCurrentCode{0}, nominalMicroamps)
                       .ok());
  TEST_ASSERT_EQUAL_UINT16(16, nominalMicroamps);
  TEST_ASSERT_TRUE(LDC1614::LDC1614::nominalDriveCurrentMicroamps(
                       DriveCurrentCode{cmd::IDRIVE_MAX}, nominalMicroamps)
                       .ok());
  TEST_ASSERT_EQUAL_UINT16(1563, nominalMicroamps);
  nominalMicroamps = 0xFFFFU;
  assertCode(Err::INVALID_PARAM,
             LDC1614::LDC1614::nominalDriveCurrentMicroamps(
                 DriveCurrentCode{static_cast<uint8_t>(cmd::IDRIVE_MAX + 1U)},
                 nominalMicroamps));
  TEST_ASSERT_EQUAL_UINT16(0, nominalMicroamps);

  const uint16_t rawStatus = static_cast<uint16_t>(
      (2U << cmd::BIT_STATUS_ERR_CHAN) | cmd::MASK_STATUS_ERR_UR |
      cmd::MASK_STATUS_ERR_ZC | cmd::MASK_STATUS_DRDY |
      cmd::MASK_STATUS_UNREADCONV2);
  const DeviceStatus decoded = LDC1614::LDC1614::decodeDeviceStatus(rawStatus);
  TEST_ASSERT_TRUE(decoded.observed);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Channel::CH2),
                          static_cast<uint8_t>(decoded.errorChannel));
  TEST_ASSERT_TRUE(decoded.errorUnderRange);
  TEST_ASSERT_TRUE(decoded.errorZeroCount);
  TEST_ASSERT_TRUE(decoded.dataReady);
  TEST_ASSERT_TRUE(decoded.unreadChannels.contains(Channel::CH2));

  FakeLdc1614Device fake;
  Config config = makeConfig(fake, DeviceVariant::LDC1612, true);
  double frequency = -1.0;
  TEST_ASSERT_TRUE(LDC1614::LDC1614::calculateSensorFrequencyHz(
      config, Channel::CH0, 0x08000000U, frequency).ok());
  TEST_ASSERT_TRUE(frequency >= 10749999.999 && frequency <= 10750000.001);
  double nextFrequency = 0.0;
  TEST_ASSERT_TRUE(LDC1614::LDC1614::calculateSensorFrequencyHz(
      config, Channel::CH0, 0x08000001U, nextFrequency).ok());
  TEST_ASSERT_TRUE(nextFrequency > frequency);

  double endpointFrequency = -1.0;
  TEST_ASSERT_TRUE(LDC1614::LDC1614::calculateSensorFrequencyHz(
      config, Channel::CH0, 0U, endpointFrequency).ok());
  TEST_ASSERT_TRUE(endpointFrequency == 0.0);
  TEST_ASSERT_TRUE(LDC1614::LDC1614::calculateSensorFrequencyHz(
      config, Channel::CH0, 0x0FFFFFFFU, endpointFrequency).ok());
  TEST_ASSERT_TRUE(endpointFrequency > 21499999.0 &&
                   endpointFrequency < 21500001.0);
  endpointFrequency = 123.0;
  assertCode(Err::INVALID_PARAM,
             LDC1614::LDC1614::calculateSensorFrequencyHz(
                 config, Channel::CH0, 0x10000000U, endpointFrequency));
  TEST_ASSERT_TRUE(endpointFrequency == 0.0);

  Config finDividerConfig = config;
  finDividerConfig.channel[0].finDivider = 2U;
  double dividedInputFrequency = 0.0;
  TEST_ASSERT_TRUE(LDC1614::LDC1614::calculateSensorFrequencyHz(
      finDividerConfig, Channel::CH0, 0x04000000U,
      dividedInputFrequency).ok());
  TEST_ASSERT_TRUE(dividedInputFrequency >= 10749999.999 &&
                   dividedInputFrequency <= 10750000.001);

  Config offsetConfig = config;
  offsetConfig.channel[0].offset = 1;
  double offsetFrequency = 0.0;
  TEST_ASSERT_TRUE(LDC1614::LDC1614::calculateSensorFrequencyHz(
      offsetConfig, Channel::CH0, 0x08000000U, offsetFrequency).ok());
  TEST_ASSERT_TRUE(offsetFrequency > frequency);

  Config invalidClock = config;
  invalidClock.referenceClock.frequencyHz = 0;
  frequency = 123.0;
  assertCode(Err::INVALID_CONFIG,
             LDC1614::LDC1614::calculateSensorFrequencyHz(
                 invalidClock, Channel::CH0, 1, frequency));
  TEST_ASSERT_TRUE(frequency == 0.0);
  assertCode(Err::INVALID_PARAM,
             LDC1614::LDC1614::calculateSensorFrequencyHz(
                 config, Channel::CH3, 1, frequency));

  FrameTiming one;
  TEST_ASSERT_TRUE(LDC1614::LDC1614::estimateFrameTiming(
      config, channelBit(Channel::CH0), one).ok());
  TEST_ASSERT_EQUAL_UINT8(4, one.acquisitionTransfers);
  TEST_ASSERT_TRUE(one.wakeAndSettleUs > 0);
  TEST_ASSERT_TRUE(one.conversionUs > 0);
  TEST_ASSERT_TRUE(one.sequentialFrameUs >= one.conversionUs);

  FrameTiming two;
  TEST_ASSERT_TRUE(LDC1614::LDC1614::estimateFrameTiming(
      config, ChannelMask{0x03}, two).ok());
  TEST_ASSERT_EQUAL_UINT8(6, two.acquisitionTransfers);
  TEST_ASSERT_EQUAL_UINT64(two.sequentialFrameUs, one.sequentialFrameUs);

  Config externalTiming = makeConfig(fake);
  externalTiming.referenceClock =
      {RefClkSrc::EXTERNAL_CLOCK, 40000000U, 0U};
  FrameTiming external;
  TEST_ASSERT_TRUE(LDC1614::LDC1614::estimateFrameTiming(
      externalTiming, channelBit(Channel::CH0), external).ok());
  // Wake uses the 35 MHz fINT lower bound, while settle/conversion/switch use
  // the configured 20 MHz channel reference after FREF_DIVIDER=2.
  TEST_ASSERT_EQUAL_UINT64(471, external.wakeAndSettleUs);
  TEST_ASSERT_EQUAL_UINT64(103, external.conversionUs);
  TEST_ASSERT_EQUAL_UINT64(575, external.sequentialFrameUs);

  FrameTiming invalidTiming;
  assertCode(Err::INVALID_PARAM, LDC1614::LDC1614::estimateFrameTiming(
                                     config, ChannelMask{}, invalidTiming));
  assertCode(Err::INVALID_PARAM, LDC1614::LDC1614::estimateFrameTiming(
                                     config, ChannelMask{0x80}, invalidTiming));

  Config widerTolerance = config;
  widerTolerance.referenceClock.tolerancePpm = 400000;
  FrameTiming conservative;
  TEST_ASSERT_TRUE(LDC1614::LDC1614::estimateFrameTiming(
      widerTolerance, channelBit(Channel::CH0), conservative).ok());
  TEST_ASSERT_TRUE(conservative.sequentialFrameUs >= one.sequentialFrameUs);
}

void test_clock_offset_mode_and_config_encoding_boundaries() {
  FakeLdc1614Device fake;
  Config external = makeConfig(fake);
  external.referenceClock = {RefClkSrc::EXTERNAL_CLOCK, 40000000U, 1U};
  LDC1614::LDC1614 externalHigh;
  assertCode(Err::INVALID_CONFIG, externalHigh.bind(external));
  external.referenceClock = {RefClkSrc::EXTERNAL_CLOCK, 2000000U, 1U};
  LDC1614::LDC1614 externalLow;
  assertCode(Err::INVALID_CONFIG, externalLow.bind(external));

  Config offset = makeConfig(fake);
  offset.referenceClock = {RefClkSrc::EXTERNAL_CLOCK, 20000000U, 0U};
  offset.channel[0].expectedSensorMinHz = 156250U;
  offset.channel[0].offset = 1023U;
  LDC1614::LDC1614 below;
  TEST_ASSERT_TRUE(below.bind(offset).ok());
  offset.channel[0].offset = 1024U;
  LDC1614::LDC1614 equal;
  assertCode(Err::INVALID_CONFIG, equal.bind(offset));
  offset.channel[0].offset = 1025U;
  LDC1614::LDC1614 above;
  assertCode(Err::INVALID_CONFIG, above.bind(offset));

  for (uint8_t active = 0U; active < 4U; ++active) {
    FakeLdc1614Device modeFake;
    LDC1614::LDC1614 modeDriver;
    Config mode = makeConfig(modeFake, DeviceVariant::LDC1614);
    mode.channels = channelBit(static_cast<Channel>(active));
    mode.activeChannel = static_cast<Channel>(active);
    TEST_ASSERT_TRUE(modeDriver.bind(mode).ok());
    TEST_ASSERT_TRUE(
        modeDriver.startInitialize(15000U + active, DEADLINE_MS).inProgress());
    TEST_ASSERT_TRUE(pollToTerminal(modeDriver).ok());
    (void)takeResult(modeDriver);
    const uint16_t expected = static_cast<uint16_t>(
        0x3C81U | (static_cast<uint16_t>(active) <<
                   cmd::BIT_CFG_ACTIVE_CHAN));
    TEST_ASSERT_EQUAL_HEX16(expected, modeFake.reg[cmd::REG_CONFIG]);
  }

  FakeLdc1614Device highCurrentFake;
  LDC1614::LDC1614 highCurrentDriver;
  Config highCurrent = makeConfig(highCurrentFake, DeviceVariant::LDC1614);
  highCurrent.highCurrentDriveEnabled = true;
  TEST_ASSERT_TRUE(highCurrentDriver.bind(highCurrent).ok());
  TEST_ASSERT_TRUE(
      highCurrentDriver.startInitialize(15010U, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(pollToTerminal(highCurrentDriver).ok());
  (void)takeResult(highCurrentDriver);
  TEST_ASSERT_TRUE((highCurrentFake.reg[cmd::REG_CONFIG] &
                    cmd::MASK_CFG_HIGH_CURRENT_DRV) != 0U);

  FakeLdc1614Device threeFake;
  LDC1614::LDC1614 threeDriver;
  Config three = makeConfig(threeFake, DeviceVariant::LDC1614, true);
  three.channels = ChannelMask{0x07};
  three.rrSequence = RRSequence::CH0_CH1_CH2;
  TEST_ASSERT_TRUE(threeDriver.bind(three).ok());
  TEST_ASSERT_TRUE(threeDriver.startInitialize(15011U, DEADLINE_MS).inProgress());
  TEST_ASSERT_TRUE(pollToTerminal(threeDriver).ok());
  (void)takeResult(threeDriver);
  TEST_ASSERT_EQUAL_HEX16(0xA209U, threeFake.reg[cmd::REG_MUX_CONFIG]);

  const Deglitch deglitches[] = {Deglitch::BW_1MHZ, Deglitch::BW_3MHZ,
                                 Deglitch::BW_10MHZ, Deglitch::BW_33MHZ};
  const uint16_t encodings[] = {0x0209U, 0x020CU, 0x020DU, 0x020FU};
  for (uint8_t index = 0; index < 4U; ++index) {
    FakeLdc1614Device deglitchFake;
    LDC1614::LDC1614 deglitchDriver;
    Config deglitch = makeConfig(deglitchFake);
    deglitch.deglitch = deglitches[index];
    TEST_ASSERT_TRUE(deglitchDriver.bind(deglitch).ok());
    TEST_ASSERT_TRUE(deglitchDriver.startInitialize(15020U + index,
                                                    DEADLINE_MS).inProgress());
    TEST_ASSERT_TRUE(pollToTerminal(deglitchDriver).ok());
    (void)takeResult(deglitchDriver);
    TEST_ASSERT_EQUAL_HEX16(encodings[index],
                            deglitchFake.reg[cmd::REG_MUX_CONFIG]);
  }
}

}  // namespace

void setUp() {}
void tearDown() {}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_status_and_public_type_contracts);
  RUN_TEST(test_behavioral_fake_models_shadow_status_unread_and_intb);
  RUN_TEST(test_bind_is_zero_i2c_and_validates_complete_explicit_profile);
  RUN_TEST(test_initialize_exact_transfer_counts_and_zero_one_large_budgets);
  RUN_TEST(test_exact_register_replay_for_both_variants_and_all_replay_jobs);
  RUN_TEST(test_large_poll_budget_uses_actual_remaining_transfer_count);
  RUN_TEST(test_initialize_fails_once_at_every_transfer_with_full_provenance);
  RUN_TEST(test_initialize_cancel_is_bus_silent_at_every_phase);
  RUN_TEST(test_deadline_timeout_is_bus_silent_wrap_safe_and_caps_callback_timeout);
  RUN_TEST(test_deadline_expiry_is_silent_at_every_multistep_operation_phase);
  RUN_TEST(test_result_ids_fifo_backpressure_exactly_once_and_stale_clear);
  RUN_TEST(test_cancelled_result_survives_immediate_replacement_and_cancel_is_idempotent);
  RUN_TEST(test_progress_is_cache_only_and_reports_exact_phase_and_budget);
  RUN_TEST(test_apply_exact_counts_failure_cancel_and_ambiguous_write_provenance);
  RUN_TEST(test_config_fault_retains_exact_register_channel_phase_status_and_effect);
  RUN_TEST(test_reset_reapply_exact_counts_all_failures_cancellation_and_no_retry);
  RUN_TEST(test_acquire_exact_budget_order_status_evidence_and_each_error_channel);
  RUN_TEST(test_acquire_failure_at_every_phase_preserves_prior_complete_publication);
  RUN_TEST(test_acquire_cancel_every_phase_is_silent_atomic_and_restartable);
  RUN_TEST(test_ldc1614_acquire_failure_cancel_deadline_and_fresh_stale_paths);
  RUN_TEST(test_acquire_detects_conversion_overrun_without_breaking_shadow_coherency);
  RUN_TEST(test_sample_quality_endpoints_watchdog_amplitude_and_status_zero_count);
  RUN_TEST(test_dirty_unknown_invalidation_rejects_acquire_and_matching_return_replays_all);
  RUN_TEST(test_update_rebind_end_and_repeated_lifecycle_are_zero_i2c_and_revisioned);
  RUN_TEST(test_owner_safe_single_transfer_and_diagnostic_dirty_contracts);
  RUN_TEST(test_single_transfer_boundaries_confirmed_nack_and_lifecycle_cleanup);
  RUN_TEST(test_transport_stats_are_diagnostic_only_and_failures_never_suppress_requests);
  RUN_TEST(test_pure_error_status_frequency_and_timing_helpers_cover_boundaries);
  RUN_TEST(test_clock_offset_mode_and_config_encoding_boundaries);
  return UNITY_END();
}
