#include <unity.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "examples/common/Ldc1614Cli.h"
#include "test/support/FakeLdc1614Device.h"

namespace {

using ldc1614_test::FakeLdc1614Device;

struct CliFixture {
  FakeLdc1614Device fake{};
  uint64_t nowMs = 1000;
  uint16_t probeCalls = 0;
  uint16_t recoveryCalls = 0;
  uint16_t sdReadCalls = 0;
  uint16_t sdWriteCalls = 0;
  bool sdAsserted = false;
  bool useLdc1612 = false;
  char output[32768]{};
  size_t outputLength = 0;

  void clearOutput() {
    output[0] = '\0';
    outputLength = 0;
  }
};

void capturePrintf(void* user, const char* format, va_list args) {
  auto* fixture = static_cast<CliFixture*>(user);
  if (fixture == nullptr || fixture->outputLength >= sizeof(fixture->output) - 1U) {
    return;
  }
  const size_t remaining = sizeof(fixture->output) - fixture->outputLength;
  const int written = std::vsnprintf(fixture->output + fixture->outputLength,
                                     remaining, format, args);
  if (written <= 0) return;
  const size_t accepted = static_cast<size_t>(written) < remaining
                              ? static_cast<size_t>(written)
                              : remaining - 1U;
  fixture->outputLength += accepted;
}

uint64_t fixtureNow(void* user) {
  return static_cast<CliFixture*>(user)->nowMs;
}

LDC1614::Status fixtureIntb(bool& asserted, void* user) {
  auto* fixture = static_cast<CliFixture*>(user);
  asserted = fixture->fake.intbAsserted;
  return LDC1614::Status::Ok();
}

LDC1614::Config fixtureConfig(void* user) {
  auto* fixture = static_cast<CliFixture*>(user);
  LDC1614::Config config;
  config.i2cWrite = FakeLdc1614Device::write;
  config.i2cWriteRead = FakeLdc1614Device::writeRead;
  config.i2cUser = &fixture->fake;
  config.i2cTimeoutMs = 25;
  config.i2cAddress = LDC1614::I2cAddress::ADDR_GND;
  config.intbAsserted = fixtureIntb;
  config.intbUser = fixture;
  config.variant = fixture->useLdc1612 ? LDC1614::DeviceVariant::LDC1612
                                      : LDC1614::DeviceVariant::LDC1614;
  config.channels = LDC1614::channelBit(LDC1614::Channel::CH0);
  config.referenceClock =
      LDC1614::ReferenceClock{LDC1614::RefClkSrc::INTERNAL, 43000000U, 200000U};
  config.mode = LDC1614::OperatingMode::SINGLE_CHANNEL;
  config.activeChannel = LDC1614::Channel::CH0;
  config.rrSequence = LDC1614::RRSequence::UNSPECIFIED;
  config.deglitch = LDC1614::Deglitch::BW_10MHZ;
  config.sensorActivation = LDC1614::SensorActivation::FULL_CURRENT;
  config.rpOverrideEnabled = true;
  config.autoAmplitudeCorrectionEnabled = false;
  config.highCurrentDriveEnabled = false;
  config.intbDisabled = false;
  config.errorReporting = LDC1614::ErrorReporting::all();
  for (uint8_t channel = 0; channel < 4U; ++channel) {
    config.channel[channel].rcount = 0x04D6;
    config.channel[channel].settleCount = 0x000A;
    config.channel[channel].finDivider = 2;
    config.channel[channel].frefDivider = 2;
    config.channel[channel].offset = 0;
    config.channel[channel].driveCurrentCode = LDC1614::DriveCurrentCode(10);
  }
  config.channel[0].expectedSensorMinHz = 100000;
  config.channel[0].expectedSensorMaxHz = 5000000;
  return config;
}

ldc1614_cli::I2cProbeResult fixtureProbe(uint8_t address, uint32_t, void* user) {
  auto* fixture = static_cast<CliFixture*>(user);
  ++fixture->probeCalls;
  return address == fixture->fake.acceptedAddress
             ? ldc1614_cli::I2cProbeResult::ACK
             : ldc1614_cli::I2cProbeResult::NACK;
}

LDC1614::Status fixtureRecoverObserved(void* user) {
  ++static_cast<CliFixture*>(user)->recoveryCalls;
  return LDC1614::Status::Ok();
}

LDC1614::Status fixtureSdRead(bool& asserted, void* user) {
  auto* fixture = static_cast<CliFixture*>(user);
  ++fixture->sdReadCalls;
  asserted = fixture->sdAsserted;
  return LDC1614::Status::Ok();
}

LDC1614::Status fixtureSdWrite(bool asserted, void* user) {
  auto* fixture = static_cast<CliFixture*>(user);
  ++fixture->sdWriteCalls;
  fixture->sdAsserted = asserted;
  return LDC1614::Status::Ok();
}

ldc1614_cli::Cli::Platform fixturePlatform(CliFixture& fixture) {
  ldc1614_cli::Cli::Platform platform;
  platform.user = &fixture;
  platform.vprintf = capturePrintf;
  platform.makeConfig = fixtureConfig;
  platform.nowMs = fixtureNow;
  platform.i2cProbe = fixtureProbe;
  platform.i2cRecover = fixtureRecoverObserved;
  platform.sdRead = fixtureSdRead;
  platform.sdWrite = fixtureSdWrite;
  platform.scanTimeoutMs = 25;
  return platform;
}

bool contains(const CliFixture& fixture, const char* text) {
  return std::strstr(fixture.output, text) != nullptr;
}

ldc1614_cli::PromptAction serviceToIdle(ldc1614_cli::Cli& cli,
                                        CliFixture& fixture,
                                        uint16_t maximumPasses = 512,
                                        uint32_t stepMs = 1U) {
  ldc1614_cli::PromptAction finalAction = ldc1614_cli::PromptAction::NONE;
  for (uint16_t pass = 0; pass < maximumPasses && cli.asynchronousWorkActive(); ++pass) {
    const uint16_t before = fixture.fake.transferCalls;
    fixture.nowMs += stepMs;
    finalAction = cli.service();
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(1U,
                                    fixture.fake.transferCalls - before);
  }
  TEST_ASSERT_FALSE_MESSAGE(cli.asynchronousWorkActive(), fixture.output);
  return finalAction;
}

void initializeToSleeping(ldc1614_cli::Cli& cli, LDC1614::LDC1614& device,
                          CliFixture& fixture) {
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::NONE),
      static_cast<uint8_t>(cli.processCommand("init")));
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::PRINT),
      static_cast<uint8_t>(serviceToIdle(cli, fixture)));
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(LDC1614::AppliedConfigState::APPLIED_SLEEPING),
      static_cast<uint8_t>(device.appliedConfigState()));
}

void initializeAndWake(ldc1614_cli::Cli& cli, LDC1614::LDC1614& device,
                       CliFixture& fixture) {
  initializeToSleeping(cli, device, fixture);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::PRINT),
      static_cast<uint8_t>(cli.processCommand("wake")));
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(LDC1614::AppliedConfigState::APPLIED_ACTIVE),
      static_cast<uint8_t>(device.appliedConfigState()));
}

void test_cli_fixed_memory_and_complete_colored_help() {
  TEST_ASSERT_LESS_OR_EQUAL_UINT32(2048U, sizeof(ldc1614_cli::Cli));
  CliFixture fixture;
  LDC1614::LDC1614 device;
  ldc1614_cli::Cli cli(device, fixturePlatform(fixture));
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::PRINT),
      static_cast<uint8_t>(cli.processCommand("help")));
  TEST_ASSERT_TRUE(contains(fixture, "\033[36m=== LDC1614 CLI ==="));
  TEST_ASSERT_TRUE(contains(fixture, "profile commit confirm"));
  TEST_ASSERT_TRUE(contains(fixture, "stress_mix <n> [mask] confirm"));
  TEST_ASSERT_TRUE(contains(fixture, "command_count=64"));

  fixture.clearOutput();
  cli.processCommand("version");
  cli.processCommand("verbose 1");
  TEST_ASSERT_TRUE(contains(fixture, "version="));
  TEST_ASSERT_TRUE(contains(fixture, "firmware_git="));
  TEST_ASSERT_TRUE(contains(fixture, "verbose enabled=1"));

  fixture.clearOutput();
  cli.processCommand("color off");
  cli.processCommand("help");
  TEST_ASSERT_NULL(std::strstr(fixture.output, "\033["));
}

void test_cli_strict_arguments_confirmation_and_zero_i2c_rejection() {
  CliFixture fixture;
  LDC1614::LDC1614 device;
  ldc1614_cli::Cli cli(device, fixturePlatform(fixture));
  TEST_ASSERT_TRUE(device.bind(fixtureConfig(&fixture)).ok());
  fixture.fake.clearIo();

  cli.processCommand("status extra");
  cli.processCommand("reg 0x18");
  cli.processCommand("wreg 0x08 0x1234");
  cli.processCommand("mode seq 9");
  TEST_ASSERT_EQUAL_UINT16(0U, fixture.fake.transferCalls);
  TEST_ASSERT_TRUE(contains(fixture, "usage: status"));
  TEST_ASSERT_TRUE(contains(fixture, "usage: reg / rreg <addr> [confirm]"));
  TEST_ASSERT_TRUE(contains(fixture, "usage: wreg <addr> <value> confirm"));
  TEST_ASSERT_FALSE(contains(fixture, "CLI preview: field=mode"));

  fixture.clearOutput();
  cli.processCommand("reg 0x7E confirm");
  TEST_ASSERT_EQUAL_UINT16(1U, fixture.fake.transferCalls);
  TEST_ASSERT_TRUE(contains(fixture, "register=0x7E value=0x5449 code=0"));
}

void test_cli_staging_is_bus_silent_transactional_and_commit_is_explicit() {
  CliFixture fixture;
  LDC1614::LDC1614 device;
  ldc1614_cli::Cli cli(device, fixturePlatform(fixture));
  TEST_ASSERT_TRUE(device.bind(fixtureConfig(&fixture)).ok());
  fixture.fake.clearIo();

  cli.processCommand("rcount 0 1300");
  cli.processCommand("refclk internal 43000000 200000");
  cli.processCommand("error data-ready 1");
  TEST_ASSERT_EQUAL_UINT16(0U, fixture.fake.transferCalls);
  TEST_ASSERT_TRUE(contains(fixture, "CLI preview: field=rcount"));

  fixture.clearOutput();
  cli.processCommand("profile commit confirm");
  TEST_ASSERT_TRUE(contains(fixture, "requires verified sleeping hardware"));
  TEST_ASSERT_EQUAL_UINT16(0U, fixture.fake.transferCalls);

  fixture.clearOutput();
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::NONE),
      static_cast<uint8_t>(cli.processCommand("init")));
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::PRINT),
      static_cast<uint8_t>(serviceToIdle(cli, fixture)));
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(LDC1614::AppliedConfigState::APPLIED_SLEEPING),
      static_cast<uint8_t>(device.appliedConfigState()));

  fixture.clearOutput();
  cli.processCommand("profile commit confirm");
  TEST_ASSERT_TRUE(contains(fixture, "profile_commit=COMMITTED"));
  TEST_ASSERT_TRUE(contains(fixture, "Run 'apply'"));
}

void test_cli_async_prompt_correlation_and_one_transfer_service_budget() {
  CliFixture fixture;
  LDC1614::LDC1614 device;
  ldc1614_cli::Cli cli(device, fixturePlatform(fixture));
  TEST_ASSERT_TRUE(device.bind(fixtureConfig(&fixture)).ok());

  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::NONE),
      static_cast<uint8_t>(cli.processCommand("init 5000")));
  TEST_ASSERT_TRUE(contains(fixture, "CLI scheduled: command=init session=0"));
  const ldc1614_cli::PromptAction terminal = serviceToIdle(cli, fixture);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(ldc1614_cli::PromptAction::PRINT),
                          static_cast<uint8_t>(terminal));
  TEST_ASSERT_TRUE(contains(fixture, "CLI result: command=init session=0 outcome=SUCCESS code=0"));
  TEST_ASSERT_TRUE(contains(fixture, "maximum="));
}

void test_cli_cooperative_probe_scan_watch_and_machine_evidence() {
  CliFixture fixture;
  LDC1614::LDC1614 device;
  ldc1614_cli::Cli cli(device, fixturePlatform(fixture));
  TEST_ASSERT_TRUE(device.bind(fixtureConfig(&fixture)).ok());

  cli.processCommand("probe");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "manufacturer_id=0x5449"));
  TEST_ASSERT_TRUE(contains(fixture, "device_id=0x3055"));

  fixture.clearOutput();
  fixture.probeCalls = 0;
  cli.processCommand("scan");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_EQUAL_UINT16(112U, fixture.probeCalls);
  TEST_ASSERT_TRUE(contains(fixture, "scan complete found=1 probes=112 code=0"));

  fixture.clearOutput();
  cli.processCommand("init");
  serviceToIdle(cli, fixture);
  cli.processCommand("wake");
  fixture.fake.injectConversion(0, 0x01234567U);
  cli.processCommand("read 0x01");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "CLI result: command=read"));
  TEST_ASSERT_TRUE(contains(fixture, "revision="));
  cli.processCommand("last 0");
  TEST_ASSERT_TRUE(contains(fixture, "sample channel=0"));
  fixture.fake.injectConversion(0, 0x01234567U);
  cli.processCommand("watch 0x01 2 0");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "batch type=SEQUENTIAL_READOUT"));
  TEST_ASSERT_TRUE(contains(fixture, "status_before="));
  TEST_ASSERT_TRUE(contains(fixture, "status_after="));
  TEST_ASSERT_TRUE(contains(fixture, "Watch results: requested=2 completed=2"));
}

void test_cli_all_canonical_commands_reject_surplus_arguments_without_i2c() {
  static const char* const commands[] = {
      "help extra",
      "version extra",
      "color on extra",
      "verbose 1 extra",
      "bind extra",
      "end extra",
      "init 100 200",
      "apply 100 200",
      "resetreapply 100 confirm extra",
      "sleep extra",
      "wake extra",
      "cancel extra",
      "job extra",
      "result extra",
      "invalidate confirm extra",
      "busrecover confirm extra",
      "read 1 extra",
      "last 0 extra",
      "watch 1 1 0 extra",
      "samplerate 0 1 extra",
      "ready extra",
      "status extra",
      "status_raw extra",
      "intb extra",
      "initdrive 0 extra",
      "cfg extra",
      "profile show extra",
      "addr extra",
      "variant extra",
      "mode single 0 extra",
      "refclk internal 43000000 200000 extra",
      "deglitch 10 extra",
      "activation full extra",
      "timeout 25 extra",
      "rp 1 extra",
      "autoamp 1 extra",
      "highcurrent 1 extra",
      "intbconfig 1 extra",
      "errors show extra",
      "error data-ready 1 extra",
      "rcount 0 1300 extra",
      "settle 0 10 extra",
      "findiv 0 2 extra",
      "frefdiv 0 2 extra",
      "offset 0 0 extra",
      "drive 0 10 extra",
      "sensorbounds 0 100000 5000000 extra",
      "probe extra",
      "scan extra",
      "dump config extra",
      "verify extra",
      "reg 0x7E confirm extra",
      "wreg 0x08 1 confirm extra",
      "decode status 0 extra",
      "freq 0 1 extra",
      "timing 1 extra",
      "driveua 1 extra",
      "drv extra",
      "state extra",
      "selftest extra",
      "stress 1 1 extra",
      "stress_mix 1 1 confirm extra",
      "soak 1 1 extra",
      "sd status extra",
  };
  static_assert(sizeof(commands) / sizeof(commands[0]) == 64U,
                "Canonical surplus-argument matrix must cover all commands");

  CliFixture fixture;
  LDC1614::LDC1614 device;
  ldc1614_cli::Cli cli(device, fixturePlatform(fixture));
  TEST_ASSERT_TRUE(device.bind(fixtureConfig(&fixture)).ok());
  fixture.fake.clearIo();
  for (const char* command : commands) {
    fixture.clearOutput();
    const uint16_t before = fixture.fake.transferCalls;
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(ldc1614_cli::PromptAction::PRINT),
        static_cast<uint8_t>(cli.processCommand(command)));
    TEST_ASSERT_EQUAL_UINT16(before, fixture.fake.transferCalls);
    TEST_ASSERT_TRUE(contains(fixture, "usage:"));
    TEST_ASSERT_FALSE(contains(fixture, "unknown command"));
    TEST_ASSERT_FALSE(cli.asynchronousWorkActive());
  }
}

void test_cli_all_aliases_dispatch_to_their_canonical_families() {
  struct AliasCase {
    const char* command;
    const char* evidence;
    bool asynchronous;
  };
  static const AliasCase aliases[] = {
      {"?", "command_count=64", false},
      {"ver", "version=", false},
      {"stop", "command=cancel", false},
      {"progress", "job active=", false},
      {"acquire 0x01", "command=read", false},
      {"drdy", "ready=", false},
      {"settings", "cfg label=", false},
      {"id", "manufacturer_id=", true},
      {"rreg 0x7E", "register=0x7E", false},
      {"health", "drv bound=", false},
  };
  CliFixture fixture;
  LDC1614::LDC1614 device;
  ldc1614_cli::Cli cli(device, fixturePlatform(fixture));
  TEST_ASSERT_TRUE(device.bind(fixtureConfig(&fixture)).ok());
  for (const AliasCase& alias : aliases) {
    fixture.clearOutput();
    const ldc1614_cli::PromptAction action = cli.processCommand(alias.command);
    if (alias.asynchronous) {
      TEST_ASSERT_EQUAL_UINT8(
          static_cast<uint8_t>(ldc1614_cli::PromptAction::NONE),
          static_cast<uint8_t>(action));
      serviceToIdle(cli, fixture);
    } else {
      TEST_ASSERT_EQUAL_UINT8(
          static_cast<uint8_t>(ldc1614_cli::PromptAction::PRINT),
          static_cast<uint8_t>(action));
    }
    TEST_ASSERT_TRUE_MESSAGE(contains(fixture, alias.evidence), alias.command);
    TEST_ASSERT_FALSE(contains(fixture, "unknown command"));
  }
}

void test_cli_complete_staged_profile_surface_is_silent_and_transactional() {
  struct SettingCase {
    const char* command;
    const char* evidence;
  };
  static const SettingCase settings[] = {
      {"mode single 0", "mode=single channel=0 count=1"},
      {"refclk external 40000000 100", "source=external hz=40000000 ppm=100"},
      {"deglitch 33", "mhz=33"},
      {"activation low", "mode=low"},
      {"timeout 50", "timeout_ms=50"},
      {"rp 0", "enabled=0"},
      {"autoamp 1", "enabled=1"},
      {"highcurrent 1", "enabled=1"},
      {"intbconfig 0", "enabled=0"},
      {"errors none", "ur=0 or=0 wd=0 ah=0 al=0 zc=0 drdy=0"},
      {"rcount 0 1300", "channel=0 value=1300"},
      {"settle 0 20", "channel=0 value=20"},
      {"findiv 0 3", "channel=0 value=3"},
      {"frefdiv 0 4", "channel=0 value=4"},
      {"offset 0 5", "channel=0 value=5"},
      {"drive 0 6", "channel=0 code=6"},
      {"sensorbounds 0 200000 4000000", "channel=0 low_hz=200000 high_hz=4000000"},
  };
  static const char* const errorRoutes[] = {
      "data-under",          "data-over",          "data-watchdog",
      "data-amplitude-high", "data-amplitude-low", "status-under",
      "status-over",         "status-watchdog",    "status-amplitude-high",
      "status-amplitude-low", "status-zero-count",  "data-ready",
  };

  CliFixture fixture;
  LDC1614::LDC1614 device;
  ldc1614_cli::Cli cli(device, fixturePlatform(fixture));
  TEST_ASSERT_TRUE(device.bind(fixtureConfig(&fixture)).ok());
  initializeToSleeping(cli, device, fixture);
  fixture.fake.clearIo();

  fixture.clearOutput();
  cli.processCommand("profile validate");
  TEST_ASSERT_TRUE(contains(fixture, "dirty=0 valid=1 outcome=VALID"));
  cli.processCommand("profile commit confirm");
  TEST_ASSERT_TRUE(contains(fixture, "profile_commit=UNCHANGED"));
  TEST_ASSERT_EQUAL_UINT16(0U, fixture.fake.transferCalls);

  fixture.clearOutput();
  cli.processCommand("profile reset");
  cli.processCommand("profile show");
  TEST_ASSERT_TRUE(contains(fixture, "outcome=RESET"));
  TEST_ASSERT_TRUE(contains(fixture, "cfg label=staged"));
  cli.processCommand("profile discard");
  TEST_ASSERT_TRUE(contains(fixture, "outcome=DISCARDED"));

  for (const SettingCase& setting : settings) {
    fixture.clearOutput();
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(ldc1614_cli::PromptAction::PRINT),
        static_cast<uint8_t>(cli.processCommand(setting.command)));
    TEST_ASSERT_TRUE_MESSAGE(contains(fixture, setting.evidence), setting.command);
    TEST_ASSERT_TRUE(contains(fixture, "outcome=STAGED"));
    TEST_ASSERT_EQUAL_UINT16(0U, fixture.fake.transferCalls);
  }
  for (const char* route : errorRoutes) {
    char command[64];
    std::snprintf(command, sizeof(command), "error %s 1", route);
    fixture.clearOutput();
    cli.processCommand(command);
    TEST_ASSERT_TRUE(contains(fixture, "field="));
    TEST_ASSERT_TRUE(contains(fixture, "enabled=1"));
    TEST_ASSERT_EQUAL_UINT16(0U, fixture.fake.transferCalls);
  }

  fixture.clearOutput();
  cli.processCommand("mode seq 3");
  TEST_ASSERT_TRUE(contains(fixture, "mode=seq channel=none count=3"));
  fixture.clearOutput();
  cli.processCommand("mode sequential 4");
  TEST_ASSERT_TRUE(contains(fixture, "usage: mode <single ch|seq count>"));
  cli.processCommand("profile show");
  TEST_ASSERT_TRUE_MESSAGE(
      contains(fixture, "selected=0x07 mode=MULTI_CHANNEL_SEQUENTIAL"),
      fixture.output);

  fixture.clearOutput();
  cli.processCommand("cfg");
  cli.processCommand("addr");
  cli.processCommand("variant");
  cli.processCommand("errors show");
  cli.processCommand("errors all");
  TEST_ASSERT_TRUE(contains(fixture, "cfg label=desired"));
  TEST_ASSERT_TRUE(contains(fixture, "address=0x2A build_profile_only=1"));
  TEST_ASSERT_TRUE(contains(fixture, "variant=LDC1614 variant_channels=4"));
  TEST_ASSERT_TRUE(contains(fixture, "ur=1 or=1 wd=1 ah=1 al=1 zc=1 drdy=1"));

  fixture.clearOutput();
  cli.processCommand("sensorbounds 0 5000000 100000");
  cli.processCommand("profile validate");
  TEST_ASSERT_TRUE(contains(fixture, "outcome=INVALID"));
  cli.processCommand("profile discard");
  cli.processCommand("profile validate");
  TEST_ASSERT_TRUE(contains(fixture, "dirty=0 valid=1 outcome=VALID"));
  TEST_ASSERT_EQUAL_UINT16(0U, fixture.fake.transferCalls);
}

void test_cli_ldc1612_rejects_every_channel_2_3_staged_path_atomically() {
  static const char* const invalidCommands[] = {
      "mode single 2",       "mode single 3",       "mode seq 3",
      "mode seq 4",          "rcount 2 1300",       "settle 2 10",
      "findiv 2 2",          "frefdiv 2 2",         "offset 2 0",
      "drive 2 10",          "sensorbounds 2 1 2",  "rcount 3 1300",
      "settle 3 10",         "findiv 3 2",          "frefdiv 3 2",
      "offset 3 0",          "drive 3 10",          "sensorbounds 3 1 2",
  };
  CliFixture fixture;
  fixture.useLdc1612 = true;
  LDC1614::LDC1614 device;
  ldc1614_cli::Cli cli(device, fixturePlatform(fixture));
  TEST_ASSERT_TRUE(device.bind(fixtureConfig(&fixture)).ok());
  fixture.fake.clearIo();
  for (const char* command : invalidCommands) {
    fixture.clearOutput();
    cli.processCommand(command);
    TEST_ASSERT_FALSE(contains(fixture, "outcome=STAGED"));
    TEST_ASSERT_TRUE(contains(fixture, "LDC1612") ||
                     contains(fixture, "unavailable"));
  }
  fixture.clearOutput();
  cli.processCommand("profile show");
  TEST_ASSERT_TRUE(contains(fixture, "variant=LDC1612 variant_channels=2 selected=0x01"));
  TEST_ASSERT_EQUAL_UINT16(0U, fixture.fake.transferCalls);
}

void test_cli_reading_register_and_pure_helpers_emit_complete_evidence() {
  CliFixture fixture;
  LDC1614::LDC1614 device;
  ldc1614_cli::Cli cli(device, fixturePlatform(fixture));
  TEST_ASSERT_TRUE(device.bind(fixtureConfig(&fixture)).ok());
  fixture.fake.clearIo();
  fixture.fake.injectConversion(
      0U, 0x01234567U, LDC1614::cmd::MASK_DATA_ERR_AE,
      LDC1614::cmd::MASK_STATUS_ERR_AHE);

  cli.processCommand("status");
  cli.processCommand("status_raw");
  fixture.fake.injectConversion(0U, 0x00123456U);
  cli.processCommand("ready");
  cli.processCommand("intb");
  cli.processCommand("initdrive 0");
  cli.processCommand("reg 0x7E");
  cli.processCommand("wreg 0x08 0x04D6 confirm");
  cli.processCommand("decode status 0x0049");
  cli.processCommand("decode data 0x0001 0x0002");
  cli.processCommand("freq 0 0x01234567");
  cli.processCommand("timing 0x01");
  cli.processCommand("driveua 10");
  cli.processCommand("sd status");
  cli.processCommand("sd assert confirm");
  cli.processCommand("sd release confirm");

  TEST_ASSERT_TRUE(contains(fixture, "STATUS=0x"));
  TEST_ASSERT_TRUE(contains(fixture, "observed=1 raw=0x"));
  TEST_ASSERT_TRUE(contains(fixture, "status_raw=0x"));
  TEST_ASSERT_TRUE(contains(fixture, "ready="));
  TEST_ASSERT_TRUE(contains(fixture, "intb asserted="));
  TEST_ASSERT_TRUE(contains(fixture, "channel=0 init_drive_code="));
  TEST_ASSERT_TRUE(contains(fixture, "register=0x7E value=0x5449 code=0"));
  TEST_ASSERT_TRUE(contains(fixture, "register=0x08 value=0x04D6 code=0"));
  TEST_ASSERT_TRUE(contains(fixture, "decode kind=status"));
  TEST_ASSERT_TRUE(contains(fixture, "decode kind=data"));
  TEST_ASSERT_TRUE(contains(fixture, "frequency_hz="));
  TEST_ASSERT_TRUE(contains(fixture, "wake_settle_us="));
  TEST_ASSERT_TRUE(contains(fixture, "microamps="));
  TEST_ASSERT_TRUE(contains(fixture, "sd state=released outcome=SUCCESS code=0"));
  TEST_ASSERT_EQUAL_UINT16(2U, fixture.sdWriteCalls);
  TEST_ASSERT_EQUAL_UINT16(1U, fixture.sdReadCalls);
  // STATUS, status_raw, INIT_IDRIVE, reg, and wreg each transfer once. READY
  // reports CONFIG_DIRTY without touching the bus before configuration applies.
  TEST_ASSERT_EQUAL_UINT16(5U, fixture.fake.transferCalls);
}

void test_cli_lifecycle_recovery_cancellation_failure_and_cached_result_paths() {
  CliFixture fixture;
  LDC1614::LDC1614 device;
  ldc1614_cli::Cli cli(device, fixturePlatform(fixture));

  cli.processCommand("bind");
  TEST_ASSERT_TRUE(device.isBound());
  initializeToSleeping(cli, device, fixture);
  fixture.clearOutput();
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::NONE),
      static_cast<uint8_t>(cli.processCommand("apply")));
  cli.processCommand("job");
  TEST_ASSERT_TRUE_MESSAGE(contains(fixture, "job active=1"), fixture.output);
  serviceToIdle(cli, fixture);
  cli.processCommand("result");
  TEST_ASSERT_TRUE(contains(fixture, "kind=APPLY_CONFIG outcome=SUCCESS"));

  fixture.clearOutput();
  cli.processCommand("resetreapply 5000 confirm");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "command=resetreapply"));
  cli.processCommand("wake");
  cli.processCommand("sleep");
  cli.processCommand("invalidate confirm");
  TEST_ASSERT_TRUE(contains(fixture, "command=invalidate outcome=SUCCESS"));
  cli.processCommand("busrecover confirm");
  TEST_ASSERT_EQUAL_UINT16(1U, fixture.recoveryCalls);
  TEST_ASSERT_TRUE(contains(fixture, "command=busrecover outcome=SUCCESS"));
  cli.processCommand("drv");
  cli.processCommand("state");
  TEST_ASSERT_TRUE(contains(fixture, "drv bound=1"));
  TEST_ASSERT_TRUE(contains(fixture, "state bound=1"));

  cli.processCommand("end");
  TEST_ASSERT_FALSE(device.isBound());
  cli.processCommand("bind");
  fixture.clearOutput();
  cli.processCommand("init");
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::NONE),
      static_cast<uint8_t>(cli.processCommand("cancel")));
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::PRINT),
      static_cast<uint8_t>(serviceToIdle(cli, fixture)));
  TEST_ASSERT_TRUE(contains(fixture, "command=init session=0 outcome=CANCELLED"));

  fixture.clearOutput();
  cli.processCommand("scan");
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::PRINT),
      static_cast<uint8_t>(cli.processCommand("cancel")));
  TEST_ASSERT_TRUE(contains(fixture, "command=scan"));
  TEST_ASSERT_TRUE(contains(fixture, "outcome=CANCELLED"));

  fixture.clearOutput();
  fixture.fake.clearIo();
  fixture.fake.failOnTransfer(
      1U, LDC1614::Status::Error(LDC1614::Err::I2C_TIMEOUT,
                                 "CLI forced init failure", -9100));
  cli.processCommand("init");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "outcome=FAILED"));
  TEST_ASSERT_TRUE(contains(fixture, "detail=-9100"));
}

void test_cli_dump_verify_selftest_sampling_stress_soak_and_failures() {
  CliFixture fixture;
  LDC1614::LDC1614 device;
  ldc1614_cli::Cli cli(device, fixturePlatform(fixture));
  TEST_ASSERT_TRUE(device.bind(fixtureConfig(&fixture)).ok());
  initializeAndWake(cli, device, fixture);

  fixture.clearOutput();
  fixture.fake.clearIo();
  cli.processCommand("dump config");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "dump scope=config register="));
  TEST_ASSERT_TRUE(contains(fixture, "dump complete scope=config count="));

  fixture.clearOutput();
  fixture.fake.clearIo();
  cli.processCommand("dump all confirm");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "DATA and STATUS reads consume"));
  TEST_ASSERT_TRUE(contains(fixture, "dump complete scope=all count="));

  fixture.clearOutput();
  fixture.fake.clearIo();
  cli.processCommand("verify");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "checked="));
  TEST_ASSERT_TRUE(contains(fixture, "matched="));
  TEST_ASSERT_TRUE(contains(fixture, "read_failures="));

  fixture.clearOutput();
  fixture.fake.clearIo();
  fixture.fake.injectConversion(0U, 0x01234567U);
  cli.processCommand("selftest");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "selftest identity=completed config=completed status=completed helpers=completed"));
  TEST_ASSERT_TRUE(contains(fixture, "Selftest result: pass="));

  fixture.clearOutput();
  fixture.fake.clearIo();
  fixture.fake.injectConversion(0U, 0x01234567U);
  cli.processCommand("samplerate 0 1");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "SampleRate result: requested=1 ok=1 fail=0"));

  fixture.clearOutput();
  fixture.fake.clearIo();
  fixture.fake.injectConversion(0U, 0x01234567U);
  cli.processCommand("stress 2 0x01");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "Stress results: 2 ok, 0 failed"));

  fixture.clearOutput();
  fixture.fake.clearIo();
  fixture.fake.injectConversion(0U, 0x01234567U);
  cli.processCommand("stress_mix 1 0x01 confirm");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "StressMix results: requested=1 ok=1 fail=0"));

  fixture.clearOutput();
  fixture.fake.clearIo();
  fixture.fake.injectConversion(0U, 0x01234567U);
  cli.processCommand("soak 1 0x01");
  serviceToIdle(cli, fixture, 128U, 100U);
  TEST_ASSERT_TRUE(contains(fixture, "Soak results: seconds=1 cycles="));

  fixture.clearOutput();
  fixture.fake.clearIo();
  fixture.fake.failOnTransfer(
      1U, LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                                 "CLI forced probe failure", -9200));
  cli.processCommand("probe");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "command=probe"));
  TEST_ASSERT_TRUE(contains(fixture, "outcome=FAILED"));

  fixture.clearOutput();
  fixture.fake.clearIo();
  fixture.fake.reg[LDC1614::cmd::REG_RCOUNT0] ^= 1U;
  cli.processCommand("verify");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "mismatched=1"));
  TEST_ASSERT_TRUE(contains(fixture, "outcome=FAILED"));
}

void test_cli_samplerate_requires_ready_fresh_valid_fault_free_in_bounds_samples() {
  CliFixture fixture;
  LDC1614::LDC1614 device;
  ldc1614_cli::Cli cli(device, fixturePlatform(fixture));
  TEST_ASSERT_TRUE(device.bind(fixtureConfig(&fixture)).ok());
  initializeAndWake(cli, device, fixture);

  fixture.clearOutput();
  fixture.fake.clearIo();
  fixture.fake.injectConversion(0U, 0x01234567U);
  cli.processCommand("samplerate 0 1");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "samplerate_ready sample=0 check=1 ready=1"));
  TEST_ASSERT_TRUE(contains(fixture, "selected=1 valid=1 fresh=1 error=0 overrun=0 within_bounds=1"));
  TEST_ASSERT_TRUE(contains(fixture, "requested=1 ok=1 fail=0"));
  TEST_ASSERT_TRUE(contains(fixture, "outcome=SUCCESS code=0"));

  fixture.clearOutput();
  fixture.fake.clearIo();
  fixture.fake.injectConversion(0U, 0x01234567U);
  cli.processCommand("samplerate 0 1");
  ++fixture.nowMs;
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::NONE),
      static_cast<uint8_t>(cli.service()));
  fixture.fake.unreadMask = 0U;
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "fresh=0"));
  TEST_ASSERT_TRUE(contains(fixture, "requested=1 ok=0 fail=1"));
  TEST_ASSERT_TRUE(contains(fixture, "outcome=FAILED"));

  fixture.clearOutput();
  fixture.fake.clearIo();
  fixture.fake.injectConversion(0U, 0x01234567U,
                                LDC1614::cmd::MASK_DATA_ERR_AE);
  cli.processCommand("samplerate 0 1");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "error=1"));
  TEST_ASSERT_TRUE(contains(fixture, "outcome=FAILED"));

  fixture.clearOutput();
  fixture.fake.clearIo();
  fixture.fake.injectConversion(0U, 1U);
  cli.processCommand("samplerate 0 1");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(fixture, "within_bounds=0"));
  TEST_ASSERT_TRUE(contains(fixture, "frequency outside configured bounds"));
  TEST_ASSERT_TRUE(contains(fixture, "outcome=FAILED"));

  fixture.clearOutput();
  fixture.fake.clearIo();
  cli.processCommand("samplerate 0 1");
  serviceToIdle(cli, fixture, 64U, 100U);
  TEST_ASSERT_TRUE(contains(fixture, "ready=0"));
  TEST_ASSERT_TRUE(contains(fixture, "readiness deadline expired"));
  TEST_ASSERT_TRUE(contains(fixture, "requested=1 ok=0 fail=1"));
  TEST_ASSERT_TRUE(contains(fixture, "outcome=FAILED"));

  fixture.clearOutput();
  fixture.fake.clearIo();
  fixture.fake.injectConversion(0U, 0x01234567U);
  fixture.fake.failOnTransfer(
      2U, LDC1614::Status::Error(LDC1614::Err::I2C_TIMEOUT,
                                 "CLI forced sample failure", -9300));
  cli.processCommand("samplerate 0 1");
  serviceToIdle(cli, fixture);
  TEST_ASSERT_TRUE(contains(
      fixture, "Operation result: operation="));
  TEST_ASSERT_TRUE(contains(fixture, "kind=ACQUIRE outcome=FAILED effects="));
  TEST_ASSERT_TRUE(contains(fixture, "detail=-9300"));
  fixture.clearOutput();
  cli.processCommand("result");
  TEST_ASSERT_TRUE(contains(fixture, "kind=ACQUIRE outcome=FAILED effects="));
  TEST_ASSERT_TRUE(contains(fixture, "detail=-9300"));
}

void test_cli_prompt_actions_and_parser_boundaries_are_exact_and_bus_silent() {
  CliFixture fixture;
  LDC1614::LDC1614 device;
  ldc1614_cli::Cli cli(device, fixturePlatform(fixture));
  TEST_ASSERT_TRUE(device.bind(fixtureConfig(&fixture)).ok());

  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::PRINT),
      static_cast<uint8_t>(cli.processCommand("help")));
  fixture.clearOutput();
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::NONE),
      static_cast<uint8_t>(cli.processCommand("init")));
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::NONE),
      static_cast<uint8_t>(cli.processCommand("job")));
  uint16_t promptCount = 0U;
  for (uint16_t pass = 0U; pass < 128U && cli.asynchronousWorkActive(); ++pass) {
    ++fixture.nowMs;
    if (cli.service() == ldc1614_cli::PromptAction::PRINT) ++promptCount;
  }
  TEST_ASSERT_FALSE(cli.asynchronousWorkActive());
  TEST_ASSERT_EQUAL_UINT16(1U, promptCount);
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::NONE),
      static_cast<uint8_t>(cli.service()));

  fixture.clearOutput();
  fixture.fake.clearIo();
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::NONE),
      static_cast<uint8_t>(cli.processCommand("scan")));
  TEST_ASSERT_TRUE(cli.asynchronousWorkActive());
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::PRINT),
      static_cast<uint8_t>(cli.processCommand("cancel")));
  TEST_ASSERT_FALSE(cli.asynchronousWorkActive());
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::NONE),
      static_cast<uint8_t>(cli.service()));
  TEST_ASSERT_TRUE(contains(fixture, "command=cancel outcome=SUCCESS"));

  fixture.fake.clearIo();
  char longLine[130];
  std::memset(longLine, 'x', sizeof(longLine) - 1U);
  longLine[sizeof(longLine) - 1U] = '\0';
  cli.processCommand(longLine);
  char unterminatedLine[128];
  std::memset(unterminatedLine, 'x', sizeof(unterminatedLine));
  cli.processCommand(unterminatedLine);
  cli.processCommand("help a b c d e f g h");
  cli.processCommand("timeout -1");
  cli.processCommand("timeout +1");
  cli.processCommand("timeout 0x100000000");
  char nonPrintable[] = {'h', 'e', 'l', 'p', '\x01', '\0'};
  cli.processCommand(nonPrintable);
  TEST_ASSERT_EQUAL_UINT16(0U, fixture.fake.transferCalls);
  TEST_ASSERT_TRUE(contains(fixture, "input exceeds"));
  TEST_ASSERT_TRUE(contains(fixture, "too many arguments"));
  TEST_ASSERT_TRUE(contains(fixture, "non-printable"));

  CliFixture missingClockFixture;
  LDC1614::LDC1614 missingClockDevice;
  ldc1614_cli::Cli::Platform missingClockPlatform =
      fixturePlatform(missingClockFixture);
  missingClockPlatform.nowMs = nullptr;
  ldc1614_cli::Cli missingClockCli(missingClockDevice, missingClockPlatform);
  TEST_ASSERT_TRUE(
      missingClockDevice.bind(fixtureConfig(&missingClockFixture)).ok());
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::PRINT),
      static_cast<uint8_t>(missingClockCli.processCommand("init")));
  TEST_ASSERT_EQUAL_UINT8(
      static_cast<uint8_t>(ldc1614_cli::PromptAction::PRINT),
      static_cast<uint8_t>(missingClockCli.processCommand("scan")));
  TEST_ASSERT_FALSE(missingClockCli.asynchronousWorkActive());
  TEST_ASSERT_EQUAL_UINT16(0U, missingClockFixture.fake.transferCalls);
  TEST_ASSERT_TRUE(contains(missingClockFixture, "monotonic clock callback"));
}

}  // namespace

/// Registration hook intentionally kept separate from test_basic.cpp so the
/// root test owner can place it in the single Unity main without a second main.
void registerLdc1614CliTests() {
  RUN_TEST(test_cli_fixed_memory_and_complete_colored_help);
  RUN_TEST(test_cli_strict_arguments_confirmation_and_zero_i2c_rejection);
  RUN_TEST(test_cli_staging_is_bus_silent_transactional_and_commit_is_explicit);
  RUN_TEST(test_cli_async_prompt_correlation_and_one_transfer_service_budget);
  RUN_TEST(test_cli_cooperative_probe_scan_watch_and_machine_evidence);
  RUN_TEST(test_cli_all_canonical_commands_reject_surplus_arguments_without_i2c);
  RUN_TEST(test_cli_all_aliases_dispatch_to_their_canonical_families);
  RUN_TEST(test_cli_complete_staged_profile_surface_is_silent_and_transactional);
  RUN_TEST(test_cli_ldc1612_rejects_every_channel_2_3_staged_path_atomically);
  RUN_TEST(test_cli_reading_register_and_pure_helpers_emit_complete_evidence);
  RUN_TEST(test_cli_lifecycle_recovery_cancellation_failure_and_cached_result_paths);
  RUN_TEST(test_cli_dump_verify_selftest_sampling_stress_soak_and_failures);
  RUN_TEST(test_cli_samplerate_requires_ready_fresh_valid_fault_free_in_bounds_samples);
  RUN_TEST(test_cli_prompt_actions_and_parser_boundaries_are_exact_and_bus_silent);
}
