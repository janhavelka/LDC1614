#include "examples/common/Ldc1614Cli.h"

#include <cerrno>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "examples/common/CliStyle.h"

namespace ldc1614_cli {
namespace {

constexpr uint64_t JOB_DEADLINE_MS = 2000;
constexpr uint8_t FIRST_SCAN_ADDRESS = 0x08;
constexpr uint8_t LAST_SCAN_ADDRESS = 0x77;
constexpr uint8_t SCAN_PROBE_COUNT = 112;

enum class HelpSection : uint8_t {
  COMMON,
  LIFECYCLE,
  MEASUREMENTS,
  CONFIGURATION,
  REGISTERS_AND_HELPERS,
  DIAGNOSTICS,
};

enum class CommandSafety : uint8_t {
  SAFE,
  CONFIRM_MUTATION,
  CONFIRM_DESTRUCTIVE_READ,
  BUILD_PROFILE_ONLY,
};

enum class ExecutionKind : uint8_t {
  CACHE_ONLY,
  PURE,
  ONE_TRANSFER,
  CORE_JOB,
  CLI_JOB,
  OWNER_BUS,
  LIFECYCLE,
};

enum class FixtureRequirement : uint8_t {
  ANY,
  NO_SENSOR_OK,
  SENSOR_REQUIRED,
  INTB_WIRED,
  SD_WIRED,
  DRIVE_TUNING,
};

struct CommandSpec {
  CommandId id;
  const char* name;
  const char* aliases;
  const char* synopsis;
  const char* description;
  HelpSection section;
  CommandSafety safety;
  ExecutionKind execution;
  FixtureRequirement fixture;
  const char* evidence;
};

// One canonical row per command family. Help, dispatch, contract checking, and
// Arduino/IDF parity all key off this deliberately boring fixed table.
static constexpr CommandSpec COMMAND_SPECS[] = {
    {CommandId::HELP, "help", "?", "help / ?",
     "Show the complete command reference", HelpSection::COMMON, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "command_count"},
    {CommandId::VERSION, "version", "ver", "version / ver",
     "Show library and firmware identity", HelpSection::COMMON, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "version firmware_git firmware_status build_timestamp"},
    {CommandId::COLOR, "color", "", "color [on|off]",
     "Show or change ANSI colors", HelpSection::COMMON, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "enabled"},
    {CommandId::VERBOSE, "verbose", "", "verbose [0|1]",
     "Show or change per-step diagnostics", HelpSection::COMMON, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "enabled"},
    {CommandId::BIND, "bind", "", "bind",
     "Bind the platform default profile (zero I2C)", HelpSection::LIFECYCLE, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "command outcome code detail msg"},
    {CommandId::END, "end", "", "end",
     "Bus-silent cancel, discard results, and unbind", HelpSection::LIFECYCLE, CommandSafety::SAFE,
     ExecutionKind::LIFECYCLE, FixtureRequirement::ANY,
     "command outcome code"},
    {CommandId::INIT, "init", "", "init [deadline_ms]",
     "Verify identity and replay the full profile", HelpSection::LIFECYCLE, CommandSafety::SAFE,
     ExecutionKind::CORE_JOB, FixtureRequirement::ANY,
     "command session deadline_ms outcome code"},
    {CommandId::APPLY, "apply", "", "apply [deadline_ms]",
     "Replay the committed desired profile", HelpSection::LIFECYCLE, CommandSafety::SAFE,
     ExecutionKind::CORE_JOB, FixtureRequirement::ANY,
     "command session deadline_ms outcome code"},
    {CommandId::RESET_REAPPLY, "resetreapply", "", "resetreapply [ms] confirm",
     "Software-reset, verify identity, and replay", HelpSection::LIFECYCLE, CommandSafety::CONFIRM_MUTATION,
     ExecutionKind::CORE_JOB, FixtureRequirement::ANY,
     "command session deadline_ms outcome code"},
    {CommandId::SLEEP, "sleep", "", "sleep",
     "Enter sleep with one bounded write", HelpSection::LIFECYCLE, CommandSafety::SAFE,
     ExecutionKind::ONE_TRANSFER, FixtureRequirement::ANY,
     "command outcome code"},
    {CommandId::WAKE, "wake", "", "wake",
     "Leave sleep with one bounded write", HelpSection::LIFECYCLE, CommandSafety::SAFE,
     ExecutionKind::ONE_TRANSFER, FixtureRequirement::ANY,
     "command outcome code"},
    {CommandId::CANCEL, "cancel", "stop", "cancel / stop",
     "Bus-silently cancel active CLI work", HelpSection::LIFECYCLE, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "command outcome code session"},
    {CommandId::JOB, "job", "progress", "job / progress",
     "Show core job and CLI session progress", HelpSection::LIFECYCLE, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "active session kind phase transfers maximum requested completed deadline_ms"},
    {CommandId::RESULT, "result", "", "result",
     "Show the cached terminal operation", HelpSection::LIFECYCLE, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "command session kind outcome effects revision phase reg channel transfers maximum code detail msg"},
    {CommandId::INVALIDATE, "invalidate", "", "invalidate confirm",
     "Bus-silently mark applied hardware state unknown", HelpSection::LIFECYCLE, CommandSafety::CONFIRM_MUTATION,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "command outcome code"},
    {CommandId::BUS_RECOVER, "busrecover", "", "busrecover confirm",
     "Run explicit owner bus reset, then require init", HelpSection::LIFECYCLE, CommandSafety::CONFIRM_MUTATION,
     ExecutionKind::OWNER_BUS, FixtureRequirement::ANY,
     "command outcome code"},
    {CommandId::READ, "read", "acquire", "read / acquire [mask]",
     "Acquire one sequential readout batch", HelpSection::MEASUREMENTS, CommandSafety::SAFE,
     ExecutionKind::CORE_JOB, FixtureRequirement::NO_SENSOR_OK,
     "command session mask outcome code selected valid fresh error overrun revision completed_ms status_before status_after"},
    {CommandId::LAST, "last", "", "last [channel]",
     "Show the cached complete sample batch", HelpSection::MEASUREMENTS, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::SENSOR_REQUIRED,
     "channel raw quality msb lsb"},
    {CommandId::WATCH, "watch", "", "watch <mask> <count> [ms]",
     "Cooperatively print bounded periodic batches", HelpSection::MEASUREMENTS, CommandSafety::SAFE,
     ExecutionKind::CLI_JOB, FixtureRequirement::SENSOR_REQUIRED,
     "command session requested completed failed elapsed_ms outcome code"},
    {CommandId::SAMPLE_RATE, "samplerate", "", "samplerate <channel> <count>",
     "Measure cooperative acquisition throughput", HelpSection::MEASUREMENTS, CommandSafety::SAFE,
     ExecutionKind::CLI_JOB, FixtureRequirement::SENSOR_REQUIRED,
     "command session requested ok fail elapsed_ms hz outcome code"},
    {CommandId::READY, "ready", "drdy", "ready / drdy",
     "Read readiness and retain STATUS evidence", HelpSection::MEASUREMENTS, CommandSafety::SAFE,
     ExecutionKind::ONE_TRANSFER, FixtureRequirement::NO_SENSOR_OK,
     "ready code"},
    {CommandId::STATUS, "status", "", "status",
     "Read and decode destructive STATUS evidence", HelpSection::MEASUREMENTS, CommandSafety::SAFE,
     ExecutionKind::ONE_TRANSFER, FixtureRequirement::NO_SENSOR_OK,
     "observed raw drdy unread err_ch ur or wd ah al zc code"},
    {CommandId::STATUS_RAW, "status_raw", "", "status_raw",
     "Read destructive STATUS as a raw word", HelpSection::MEASUREMENTS, CommandSafety::SAFE,
     ExecutionKind::ONE_TRANSFER, FixtureRequirement::NO_SENSOR_OK,
     "raw code"},
    {CommandId::INTB, "intb", "", "intb",
     "Observe optional application-owned INTB", HelpSection::MEASUREMENTS, CommandSafety::SAFE,
     ExecutionKind::PURE, FixtureRequirement::INTB_WIRED,
     "asserted code"},
    {CommandId::INIT_DRIVE, "initdrive", "", "initdrive <channel>",
     "Read INIT_IDRIVE and nominal current", HelpSection::MEASUREMENTS, CommandSafety::SAFE,
     ExecutionKind::ONE_TRANSFER, FixtureRequirement::DRIVE_TUNING,
     "channel init_drive_code code"},
    {CommandId::CFG, "cfg", "settings", "cfg / settings",
     "Show desired and staged profiles", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "address variant variant_channels selected mode ref_source ref_hz tolerance_ppm deglitch activation timeout_ms rp_override auto_amplitude high_current intb_config error_reporting revision applied"},
    {CommandId::PROFILE, "profile", "", "profile show|reset|discard\nprofile validate\nprofile commit confirm",
     "Manage the bus-silent staged profile", HelpSection::CONFIGURATION, CommandSafety::CONFIRM_MUTATION,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "dirty valid field channel outcome code"},
    {CommandId::ADDR, "addr", "", "addr",
     "Show display-only binding address", HelpSection::CONFIGURATION, CommandSafety::BUILD_PROFILE_ONLY,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "address"},
    {CommandId::VARIANT, "variant", "", "variant",
     "Show display-only silicon variant", HelpSection::CONFIGURATION, CommandSafety::BUILD_PROFILE_ONLY,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "variant variant_channels"},
    {CommandId::MODE, "mode", "", "mode <single ch|seq count>",
     "Stage mode, channel mask, active channel, and RR atomically", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "mode channel count"},
    {CommandId::REFCLK, "refclk", "", "refclk <src> <hz> <ppm>",
     "Stage the explicit reference-clock fact", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "source hz ppm"},
    {CommandId::DEGLITCH, "deglitch", "", "deglitch <1|3|10|33>",
     "Stage input deglitch bandwidth in MHz", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "mhz"},
    {CommandId::ACTIVATION, "activation", "", "activation <full|low>",
     "Stage sensor activation current policy", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "mode"},
    {CommandId::TIMEOUT, "timeout", "", "timeout <ms>",
     "Stage per-callback timeout", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "timeout_ms"},
    {CommandId::RP, "rp", "", "rp <0|1>",
     "Stage RP override", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "enabled"},
    {CommandId::AUTOAMP, "autoamp", "", "autoamp <0|1>",
     "Stage automatic amplitude correction", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "enabled"},
    {CommandId::HIGH_CURRENT, "highcurrent", "", "highcurrent <0|1>",
     "Stage CH0 high-current drive", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "enabled"},
    {CommandId::INTB_CONFIG, "intbconfig", "", "intbconfig <0|1>",
     "Stage INTB output enable", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "enabled"},
    {CommandId::ERRORS, "errors", "", "errors [show|all|none]",
     "Show or stage all error routes", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "ur or wd ah al zc drdy"},
    {CommandId::ERROR, "error", "", "error <field> <0|1>",
     "Stage one named DATA/STATUS/DRDY route", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "field enabled"},
    {CommandId::RCOUNT, "rcount", "", "rcount <ch> <value>",
     "Stage channel RCOUNT", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "channel value"},
    {CommandId::SETTLE, "settle", "", "settle <ch> <value>",
     "Stage channel SETTLECOUNT", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "channel value"},
    {CommandId::FIN_DIV, "findiv", "", "findiv <ch> <value>",
     "Stage channel FIN divider", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "channel value"},
    {CommandId::FREF_DIV, "frefdiv", "", "frefdiv <ch> <value>",
     "Stage channel FREF divider", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "channel value"},
    {CommandId::OFFSET, "offset", "", "offset <ch> <value>",
     "Stage channel OFFSET", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "channel value"},
    {CommandId::DRIVE, "drive", "", "drive <ch> <code>",
     "Stage channel IDRIVE code", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::DRIVE_TUNING,
     "channel code"},
    {CommandId::SENSOR_BOUNDS, "sensorbounds", "", "sensorbounds <ch> <lo> <hi>",
     "Stage expected channel sensor-frequency bounds", HelpSection::CONFIGURATION, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::SENSOR_REQUIRED,
     "channel low_hz high_hz"},
    {CommandId::PROBE, "probe", "id", "probe / id",
     "Cooperatively read and verify both IDs", HelpSection::REGISTERS_AND_HELPERS, CommandSafety::SAFE,
     ExecutionKind::CLI_JOB, FixtureRequirement::NO_SENSOR_OK,
     "manufacturer_id device_id match code"},
    {CommandId::SCAN, "scan", "", "scan",
     "Cooperatively probe 0x08..0x77", HelpSection::REGISTERS_AND_HELPERS, CommandSafety::SAFE,
     ExecutionKind::OWNER_BUS, FixtureRequirement::ANY,
     "found probes code"},
    {CommandId::DUMP, "dump", "", "dump config\ndump all confirm",
     "Cooperatively dump mapped registers", HelpSection::REGISTERS_AND_HELPERS, CommandSafety::CONFIRM_DESTRUCTIVE_READ,
     ExecutionKind::CLI_JOB, FixtureRequirement::NO_SENSOR_OK,
     "scope register value count code"},
    {CommandId::VERIFY, "verify", "", "verify",
     "Compare every persistent register to desired", HelpSection::REGISTERS_AND_HELPERS, CommandSafety::SAFE,
     ExecutionKind::CLI_JOB, FixtureRequirement::NO_SENSOR_OK,
     "checked matched mismatched read_failures outcome code"},
    {CommandId::REG, "reg", "rreg", "reg / rreg <addr> [confirm]",
     "Read one mapped register; DATA/STATUS require confirm", HelpSection::REGISTERS_AND_HELPERS, CommandSafety::CONFIRM_DESTRUCTIVE_READ,
     ExecutionKind::ONE_TRANSFER, FixtureRequirement::NO_SENSOR_OK,
     "register value code"},
    {CommandId::WREG, "wreg", "", "wreg <addr> <value> confirm",
     "Write one mapped writable register and invalidate trust", HelpSection::REGISTERS_AND_HELPERS, CommandSafety::CONFIRM_MUTATION,
     ExecutionKind::ONE_TRANSFER, FixtureRequirement::NO_SENSOR_OK,
     "register value code"},
    {CommandId::DECODE, "decode", "", "decode status <raw16>\ndecode data <msb> <lsb>",
     "Run pure STATUS or DATA decoding", HelpSection::REGISTERS_AND_HELPERS, CommandSafety::SAFE,
     ExecutionKind::PURE, FixtureRequirement::ANY,
     "kind raw drdy unread err_ch ur or wd ah al zc count quality"},
    {CommandId::FREQ, "freq", "", "freq <channel> <raw28>",
     "Calculate sensor frequency", HelpSection::REGISTERS_AND_HELPERS, CommandSafety::SAFE,
     ExecutionKind::PURE, FixtureRequirement::ANY,
     "channel raw frequency_hz code"},
    {CommandId::TIMING, "timing", "", "timing [mask]",
     "Estimate conservative chip timing", HelpSection::REGISTERS_AND_HELPERS, CommandSafety::SAFE,
     ExecutionKind::PURE, FixtureRequirement::ANY,
     "mask wake_settle_us conversion_us sequential_frame_us acquisition_transfers code"},
    {CommandId::DRIVE_UA, "driveua", "", "driveua <code>",
     "Convert IDRIVE code to nominal uA", HelpSection::REGISTERS_AND_HELPERS, CommandSafety::SAFE,
     ExecutionKind::PURE, FixtureRequirement::ANY,
     "code microamps"},
    {CommandId::DRIVER, "drv", "health", "drv / health",
     "Show driver, transport, and fault provenance", HelpSection::DIAGNOSTICS, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "bound applied revision active result_available attempts success failures last_code"},
    {CommandId::STATE, "state", "", "state",
     "Show compact parseable driver state", HelpSection::DIAGNOSTICS, CommandSafety::SAFE,
     ExecutionKind::CACHE_ONLY, FixtureRequirement::ANY,
     "bound applied profile_dirty session_kind active pending_result"},
    {CommandId::SELFTEST, "selftest", "", "selftest",
     "Run full cooperative safe self-test", HelpSection::DIAGNOSTICS, CommandSafety::SAFE,
     ExecutionKind::CLI_JOB, FixtureRequirement::NO_SENSOR_OK,
     "command session identity config status helpers pass fail skip outcome code"},
    {CommandId::STRESS, "stress", "", "stress <count> [mask]",
     "Stress cooperative acquisitions", HelpSection::DIAGNOSTICS, CommandSafety::SAFE,
     ExecutionKind::CLI_JOB, FixtureRequirement::NO_SENSOR_OK,
     "command session requested ok fail elapsed_ms hz outcome code"},
    {CommandId::STRESS_MIX, "stress_mix", "", "stress_mix <n> [mask] confirm",
     "Stress mixed STATUS/INTB/IDRIVE/acquisition paths", HelpSection::DIAGNOSTICS, CommandSafety::CONFIRM_MUTATION,
     ExecutionKind::CLI_JOB, FixtureRequirement::NO_SENSOR_OK,
     "command session requested ok fail elapsed_ms outcome code"},
    {CommandId::SOAK, "soak", "", "soak <seconds> [mask]",
     "Run bounded periodic acquisition soak", HelpSection::DIAGNOSTICS, CommandSafety::SAFE,
     ExecutionKind::CLI_JOB, FixtureRequirement::NO_SENSOR_OK,
     "command session seconds cycles ok fail elapsed_ms outcome code"},
    {CommandId::SD, "sd", "", "sd status|assert|release confirm",
     "Use optional application-owned shutdown pin", HelpSection::DIAGNOSTICS, CommandSafety::CONFIRM_MUTATION,
     ExecutionKind::LIFECYCLE, FixtureRequirement::SD_WIRED,
     "state outcome code"},
};

const CommandSpec* commandSpec(CommandId id) {
  for (const CommandSpec& spec : COMMAND_SPECS) {
    if (spec.id == id) return &spec;
  }
  return nullptr;
}

const char* commandName(CommandId id) {
  const CommandSpec* spec = commandSpec(id);
  return spec != nullptr ? spec->name : "unknown";
}

static constexpr uint8_t CONFIG_REGISTERS[] = {
    LDC1614::cmd::REG_RCOUNT0, LDC1614::cmd::REG_RCOUNT1,
    LDC1614::cmd::REG_RCOUNT2, LDC1614::cmd::REG_RCOUNT3,
    LDC1614::cmd::REG_SETTLECOUNT0, LDC1614::cmd::REG_SETTLECOUNT1,
    LDC1614::cmd::REG_SETTLECOUNT2, LDC1614::cmd::REG_SETTLECOUNT3,
    LDC1614::cmd::REG_CLOCK_DIVIDERS0, LDC1614::cmd::REG_CLOCK_DIVIDERS1,
    LDC1614::cmd::REG_CLOCK_DIVIDERS2, LDC1614::cmd::REG_CLOCK_DIVIDERS3,
    LDC1614::cmd::REG_OFFSET0, LDC1614::cmd::REG_OFFSET1,
    LDC1614::cmd::REG_OFFSET2, LDC1614::cmd::REG_OFFSET3,
    LDC1614::cmd::REG_DRIVE_CURRENT0, LDC1614::cmd::REG_DRIVE_CURRENT1,
    LDC1614::cmd::REG_DRIVE_CURRENT2, LDC1614::cmd::REG_DRIVE_CURRENT3,
    LDC1614::cmd::REG_ERROR_CONFIG, LDC1614::cmd::REG_MUX_CONFIG,
    LDC1614::cmd::REG_CONFIG,
};

static constexpr uint8_t ALL_REGISTERS[] = {
    LDC1614::cmd::REG_DATA0_MSB, LDC1614::cmd::REG_DATA0_LSB,
    LDC1614::cmd::REG_DATA1_MSB, LDC1614::cmd::REG_DATA1_LSB,
    LDC1614::cmd::REG_DATA2_MSB, LDC1614::cmd::REG_DATA2_LSB,
    LDC1614::cmd::REG_DATA3_MSB, LDC1614::cmd::REG_DATA3_LSB,
    LDC1614::cmd::REG_RCOUNT0, LDC1614::cmd::REG_RCOUNT1,
    LDC1614::cmd::REG_RCOUNT2, LDC1614::cmd::REG_RCOUNT3,
    LDC1614::cmd::REG_OFFSET0, LDC1614::cmd::REG_OFFSET1,
    LDC1614::cmd::REG_OFFSET2, LDC1614::cmd::REG_OFFSET3,
    LDC1614::cmd::REG_SETTLECOUNT0, LDC1614::cmd::REG_SETTLECOUNT1,
    LDC1614::cmd::REG_SETTLECOUNT2, LDC1614::cmd::REG_SETTLECOUNT3,
    LDC1614::cmd::REG_CLOCK_DIVIDERS0, LDC1614::cmd::REG_CLOCK_DIVIDERS1,
    LDC1614::cmd::REG_CLOCK_DIVIDERS2, LDC1614::cmd::REG_CLOCK_DIVIDERS3,
    LDC1614::cmd::REG_STATUS, LDC1614::cmd::REG_ERROR_CONFIG,
    LDC1614::cmd::REG_CONFIG, LDC1614::cmd::REG_MUX_CONFIG,
    LDC1614::cmd::REG_RESET_DEV,
    LDC1614::cmd::REG_DRIVE_CURRENT0, LDC1614::cmd::REG_DRIVE_CURRENT1,
    LDC1614::cmd::REG_DRIVE_CURRENT2, LDC1614::cmd::REG_DRIVE_CURRENT3,
    LDC1614::cmd::REG_MANUFACTURER_ID, LDC1614::cmd::REG_DEVICE_ID,
};

const char* errName(LDC1614::Err error) {
  switch (error) {
    case LDC1614::Err::OK: return "OK";
    case LDC1614::Err::NOT_BOUND: return "NOT_BOUND";
    case LDC1614::Err::INVALID_CONFIG: return "INVALID_CONFIG";
    case LDC1614::Err::INVALID_PARAM: return "INVALID_PARAM";
    case LDC1614::Err::BUSY: return "BUSY";
    case LDC1614::Err::IN_PROGRESS: return "IN_PROGRESS";
    case LDC1614::Err::RESULT_NOT_READY: return "RESULT_NOT_READY";
    case LDC1614::Err::RESULT_QUEUE_FULL: return "RESULT_QUEUE_FULL";
    case LDC1614::Err::DUPLICATE_OPERATION_ID: return "DUPLICATE_OPERATION_ID";
    case LDC1614::Err::DEVICE_NOT_FOUND: return "DEVICE_NOT_FOUND";
    case LDC1614::Err::CONVERSION_NOT_READY: return "CONVERSION_NOT_READY";
    case LDC1614::Err::TIMEOUT: return "TIMEOUT";
    case LDC1614::Err::CANCELLED: return "CANCELLED";
    case LDC1614::Err::CONFIG_DIRTY: return "CONFIG_DIRTY";
    case LDC1614::Err::I2C_ERROR: return "I2C_ERROR";
    case LDC1614::Err::I2C_NACK_ADDR: return "I2C_NACK_ADDR";
    case LDC1614::Err::I2C_NACK_DATA: return "I2C_NACK_DATA";
    case LDC1614::Err::I2C_TIMEOUT: return "I2C_TIMEOUT";
    case LDC1614::Err::I2C_BUS: return "I2C_BUS";
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

const char* jobName(LDC1614::JobKind kind) {
  switch (kind) {
    case LDC1614::JobKind::NONE: return "NONE";
    case LDC1614::JobKind::INITIALIZE: return "INITIALIZE";
    case LDC1614::JobKind::APPLY_CONFIG: return "APPLY_CONFIG";
    case LDC1614::JobKind::RESET_AND_REAPPLY: return "RESET_AND_REAPPLY";
    case LDC1614::JobKind::ACQUIRE: return "ACQUIRE";
  }
  return "UNKNOWN";
}

const char* outcomeName(LDC1614::TerminalOutcome outcome) {
  switch (outcome) {
    case LDC1614::TerminalOutcome::NONE: return "NONE";
    case LDC1614::TerminalOutcome::SUCCESS: return "SUCCESS";
    case LDC1614::TerminalOutcome::FAILED: return "FAILED";
    case LDC1614::TerminalOutcome::CANCELLED: return "CANCELLED";
    case LDC1614::TerminalOutcome::TIMED_OUT: return "TIMED_OUT";
  }
  return "UNKNOWN";
}

const char* phaseName(LDC1614::JobPhase phase) {
  switch (phase) {
    case LDC1614::JobPhase::NONE: return "NONE";
    case LDC1614::JobPhase::VERIFY_MANUFACTURER: return "VERIFY_MANUFACTURER";
    case LDC1614::JobPhase::VERIFY_DEVICE: return "VERIFY_DEVICE";
    case LDC1614::JobPhase::FORCE_SLEEP: return "FORCE_SLEEP";
    case LDC1614::JobPhase::WRITE_RCOUNT: return "WRITE_RCOUNT";
    case LDC1614::JobPhase::WRITE_SETTLECOUNT: return "WRITE_SETTLECOUNT";
    case LDC1614::JobPhase::WRITE_CLOCK_DIVIDERS: return "WRITE_CLOCK_DIVIDERS";
    case LDC1614::JobPhase::WRITE_OFFSET: return "WRITE_OFFSET";
    case LDC1614::JobPhase::WRITE_DRIVE_CURRENT: return "WRITE_DRIVE_CURRENT";
    case LDC1614::JobPhase::WRITE_ERROR_CONFIG: return "WRITE_ERROR_CONFIG";
    case LDC1614::JobPhase::WRITE_MUX_CONFIG: return "WRITE_MUX_CONFIG";
    case LDC1614::JobPhase::WRITE_FINAL_CONFIG: return "WRITE_FINAL_CONFIG";
    case LDC1614::JobPhase::SOFTWARE_RESET: return "SOFTWARE_RESET";
    case LDC1614::JobPhase::READ_STATUS_BEFORE: return "READ_STATUS_BEFORE";
    case LDC1614::JobPhase::READ_DATA_MSB: return "READ_DATA_MSB";
    case LDC1614::JobPhase::READ_DATA_LSB: return "READ_DATA_LSB";
    case LDC1614::JobPhase::READ_STATUS_AFTER: return "READ_STATUS_AFTER";
  }
  return "UNKNOWN";
}

const char* variantName(LDC1614::DeviceVariant variant) {
  switch (variant) {
    case LDC1614::DeviceVariant::LDC1612: return "LDC1612";
    case LDC1614::DeviceVariant::LDC1614: return "LDC1614";
    case LDC1614::DeviceVariant::UNSPECIFIED: return "UNSPECIFIED";
  }
  return "UNKNOWN";
}

const char* modeName(LDC1614::OperatingMode mode) {
  switch (mode) {
    case LDC1614::OperatingMode::SINGLE_CHANNEL: return "SINGLE_CHANNEL";
    case LDC1614::OperatingMode::MULTI_CHANNEL_SEQUENTIAL: return "MULTI_CHANNEL_SEQUENTIAL";
    case LDC1614::OperatingMode::UNSPECIFIED: return "UNSPECIFIED";
  }
  return "UNKNOWN";
}

const char* sessionName(SessionKind kind) {
  switch (kind) {
    case SessionKind::NONE: return "NONE";
    case SessionKind::PROBE: return "PROBE";
    case SessionKind::SCAN: return "SCAN";
    case SessionKind::DUMP_CONFIG: return "DUMP_CONFIG";
    case SessionKind::DUMP_ALL: return "DUMP_ALL";
    case SessionKind::VERIFY: return "VERIFY";
    case SessionKind::WATCH: return "WATCH";
    case SessionKind::SAMPLE_RATE: return "SAMPLE_RATE";
    case SessionKind::STRESS: return "STRESS";
    case SessionKind::STRESS_MIX: return "STRESS_MIX";
    case SessionKind::SOAK: return "SOAK";
    case SessionKind::SELF_TEST: return "SELF_TEST";
  }
  return "UNKNOWN";
}

const char* sectionName(HelpSection section) {
  switch (section) {
    case HelpSection::COMMON: return "Common";
    case HelpSection::LIFECYCLE: return "Lifecycle and owner jobs";
    case HelpSection::MEASUREMENTS: return "Measurements";
    case HelpSection::CONFIGURATION: return "Staged configuration";
    case HelpSection::REGISTERS_AND_HELPERS: return "Registers and helpers";
    case HelpSection::DIAGNOSTICS: return "Driver diagnostics";
  }
  return "Commands";
}

bool parseUnsigned(const char* text, uint64_t maximum, uint64_t& value) {
  value = 0;
  if (text == nullptr || *text == '\0' || *text == '+' || *text == '-') return false;
  int base = 10;
  if (text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
    if (text[2] == '\0') return false;
    base = 16;
  }
  errno = 0;
  char* end = nullptr;
  const unsigned long long parsed = std::strtoull(text, &end, base);
  if (errno == ERANGE || end == text || *end != '\0' || parsed > maximum) return false;
  value = static_cast<uint64_t>(parsed);
  return true;
}

bool parseOnOff(const char* text, bool& value) {
  if (text != nullptr && std::strcmp(text, "on") == 0) {
    value = true;
    return true;
  }
  if (text != nullptr && std::strcmp(text, "off") == 0) {
    value = false;
    return true;
  }
  return false;
}

bool isConfirmed(const char* text) {
  return text != nullptr && std::strcmp(text, "confirm") == 0;
}

bool requireConfirmation(const char* text) { return isConfirmed(text); }

bool rejectExtraArguments(uint8_t actual, uint8_t expected) {
  return actual != expected;
}

bool aliasMatches(const char* aliases, const char* token) {
  if (aliases == nullptr || token == nullptr || *aliases == '\0') return false;
  const size_t tokenLength = std::strlen(token);
  const char* cursor = aliases;
  while (*cursor != '\0') {
    while (*cursor == ' ') ++cursor;
    const char* end = cursor;
    while (*end != '\0' && *end != ' ') ++end;
    if (static_cast<size_t>(end - cursor) == tokenLength &&
        std::strncmp(cursor, token, tokenLength) == 0) {
      return true;
    }
    cursor = end;
  }
  return false;
}

bool isDestructiveRegister(uint8_t reg) {
  return reg <= LDC1614::cmd::REG_DATA3_LSB || reg == LDC1614::cmd::REG_STATUS;
}

bool isChannelRegisterForUnavailableChannel(const LDC1614::Config& config,
                                             uint8_t reg) {
  if (config.variant != LDC1614::DeviceVariant::LDC1612) return false;
  return (reg >= LDC1614::cmd::REG_DATA2_MSB && reg <= LDC1614::cmd::REG_DATA3_LSB) ||
         (reg >= LDC1614::cmd::REG_RCOUNT2 && reg <= LDC1614::cmd::REG_RCOUNT3) ||
         (reg >= LDC1614::cmd::REG_OFFSET2 && reg <= LDC1614::cmd::REG_OFFSET3) ||
         (reg >= LDC1614::cmd::REG_SETTLECOUNT2 && reg <= LDC1614::cmd::REG_SETTLECOUNT3) ||
         (reg >= LDC1614::cmd::REG_CLOCK_DIVIDERS2 &&
          reg <= LDC1614::cmd::REG_CLOCK_DIVIDERS3) ||
         (reg >= LDC1614::cmd::REG_DRIVE_CURRENT2 &&
          reg <= LDC1614::cmd::REG_DRIVE_CURRENT3);
}

uint8_t physicalChannelCount(LDC1614::DeviceVariant variant) {
  return variant == LDC1614::DeviceVariant::LDC1612
             ? 2U
             : (variant == LDC1614::DeviceVariant::LDC1614 ? 4U : 0U);
}

bool deadlineReached(uint64_t now, uint64_t deadline) {
  return static_cast<int64_t>(now - deadline) >= 0;
}

const char* resetColor(bool enabled) {
  return cli_style::code(enabled, cli_style::Color::RESET);
}

}  // namespace

Cli::Cli(LDC1614::LDC1614& device, Platform platform)
    : _device(device), _platform(platform) {
#if LDC1614_CLI_ENABLE_COLOR && LDC1614_CLI_COLOR_DEFAULT
  _colorEnabled = true;
#else
  _colorEnabled = false;
#endif
}

LDC1614::Config Cli::makeDefaultConfig() const {
  return _platform.makeConfig != nullptr ? _platform.makeConfig(_platform.user)
                                         : LDC1614::Config{};
}

uint64_t Cli::nowMs() const {
  return _platform.nowMs != nullptr ? _platform.nowMs(_platform.user) : 0;
}

void Cli::vprintfToOutput(const char* fmt, va_list args) const {
  if (_platform.vprintf != nullptr) _platform.vprintf(_platform.user, fmt, args);
}

void Cli::printf(const char* fmt, ...) const {
  va_list args;
  va_start(args, fmt);
  vprintfToOutput(fmt, args);
  va_end(args);
}

void Cli::println(const char* text) const { printf("%s\n", text); }

void Cli::logError(const char* fmt, ...) const {
  printf("%s[E]%s ", cli_style::code(_colorEnabled, cli_style::Color::RED),
         resetColor(_colorEnabled));
  va_list args;
  va_start(args, fmt);
  vprintfToOutput(fmt, args);
  va_end(args);
  println();
}

void Cli::logInfo(const char* fmt, ...) const {
  printf("%s[I]%s ", cli_style::code(_colorEnabled, cli_style::Color::CYAN),
         resetColor(_colorEnabled));
  va_list args;
  va_start(args, fmt);
  vprintfToOutput(fmt, args);
  va_end(args);
  println();
}

void Cli::printPrompt() const { printf("> "); }

void Cli::printStatus(const LDC1614::Status& status) const {
  const cli_style::Color color = status.ok() ? cli_style::Color::GREEN
                                              : cli_style::Color::RED;
  printf("  Status: %s%s%s (code=%u, detail=%ld)\n",
         cli_style::code(_colorEnabled, color), errName(status.code),
         resetColor(_colorEnabled), static_cast<unsigned>(status.code),
         static_cast<long>(status.detail));
  printf("  Message: %s\n", status.msg != nullptr ? status.msg : "");
}

bool Cli::parseLine(const char* input, ParsedLine& parsed) const {
  parsed = ParsedLine{};
  if (input == nullptr) return false;
  size_t length = 0U;
  while (length <= MAX_LINE_LENGTH && input[length] != '\0') ++length;
  if (length > MAX_LINE_LENGTH) {
    logError("input exceeds %u bytes", static_cast<unsigned>(MAX_LINE_LENGTH));
    return false;
  }
  std::memcpy(parsed.storage, input, length + 1U);
  char* cursor = parsed.storage;
  while (*cursor != '\0') {
    while (*cursor == ' ' || *cursor == '\t') ++cursor;
    if (*cursor == '\0') break;
    if (parsed.argc >= MAX_TOKENS) {
      logError("too many arguments (maximum %u)", static_cast<unsigned>(MAX_TOKENS));
      parsed.argc = 0;
      return false;
    }
    if (static_cast<unsigned char>(*cursor) < 0x21U ||
        static_cast<unsigned char>(*cursor) > 0x7EU) {
      logError("input contains a non-printable character");
      parsed.argc = 0;
      return false;
    }
    parsed.argv[parsed.argc++] = cursor;
    while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t') {
      const unsigned char value = static_cast<unsigned char>(*cursor);
      if (value < 0x21U || value > 0x7EU) {
        logError("input contains a non-printable character");
        parsed.argc = 0;
        return false;
      }
      ++cursor;
    }
    if (*cursor != '\0') *cursor++ = '\0';
  }
  return parsed.argc != 0U;
}

CommandId Cli::resolveCommand(const char* token) const {
  for (const CommandSpec& spec : COMMAND_SPECS) {
    if (std::strcmp(spec.name, token) == 0 || aliasMatches(spec.aliases, token)) {
      return spec.id;
    }
  }
  return CommandId::UNKNOWN;
}

bool Cli::asynchronousWorkActive() const {
  return _pending.owner != OperationOwner::NONE ||
         _session.kind != SessionKind::NONE || _device.jobProgress().active;
}

bool Cli::commandAllowedWhileBusy(CommandId id) const {
  return id == CommandId::HELP || id == CommandId::VERSION ||
         id == CommandId::COLOR || id == CommandId::VERBOSE ||
         id == CommandId::JOB || id == CommandId::DRIVER ||
         id == CommandId::STATE || id == CommandId::RESULT ||
         id == CommandId::LAST || id == CommandId::CANCEL;
}

PromptAction Cli::promptActionForCurrentState() const {
  return asynchronousWorkActive() || _promptDeferred ? PromptAction::NONE
                                                      : PromptAction::PRINT;
}

PromptAction Cli::usage(const char* synopsis) const {
  logError("usage: %s", synopsis);
  return promptActionForCurrentState();
}

void Cli::printHelp() const {
  printf("%s=== LDC1614 CLI ===%s\n",
         cli_style::code(_colorEnabled, cli_style::Color::CYAN),
         resetColor(_colorEnabled));
  HelpSection current = HelpSection::COMMON;
  bool first = true;
  for (const CommandSpec& spec : COMMAND_SPECS) {
    if (first || spec.section != current) {
      current = spec.section;
      printf("\n%s[%s]%s\n",
             cli_style::code(_colorEnabled, cli_style::Color::GREEN),
             sectionName(current), resetColor(_colorEnabled));
      first = false;
    }
    const char* line = spec.synopsis;
    bool firstSynopsis = true;
    while (line != nullptr && *line != '\0') {
      const char* end = std::strchr(line, '\n');
      const int length = end != nullptr ? static_cast<int>(end - line)
                                        : static_cast<int>(std::strlen(line));
      printf("  %s%-32.*s%s - %s%s\n",
             cli_style::code(_colorEnabled, cli_style::Color::CYAN), length,
             line, resetColor(_colorEnabled),
             firstSynopsis ? spec.description : "continued",
             (firstSynopsis && spec.aliases[0] != '\0') ? " (alias available)" : "");
      firstSynopsis = false;
      line = end != nullptr ? end + 1 : nullptr;
    }
  }
  println("\nSettings edit a staged profile with zero I2C. Validate and commit explicitly.");
  println("DATA/STATUS raw reads are destructive; dangerous commands require 'confirm'.");
}

void Cli::ensureStagedProfile() {
  if (_stagedInitialized) return;
  _stagedConfig = _device.isBound() ? _device.config() : makeDefaultConfig();
  _stagedInitialized = true;
  _stagedDirty = false;
}

void Cli::discardStagedProfile() {
  _stagedConfig = _device.isBound() ? _device.config() : makeDefaultConfig();
  _stagedInitialized = true;
  _stagedDirty = false;
}

void Cli::resetStagedProfile() {
  const LDC1614::Config defaults = makeDefaultConfig();
  _stagedConfig = defaults;
  if (_device.isBound()) {
    const LDC1614::Config& desired = _device.config();
    // Binding identity is an application/platform fact, never a staged CLI
    // setting. Preserve it even when resetting profile values to defaults.
    _stagedConfig.i2cWrite = desired.i2cWrite;
    _stagedConfig.i2cWriteRead = desired.i2cWriteRead;
    _stagedConfig.i2cUser = desired.i2cUser;
    _stagedConfig.i2cAddress = desired.i2cAddress;
    _stagedConfig.variant = desired.variant;
    _stagedConfig.intbAsserted = desired.intbAsserted;
    _stagedConfig.intbUser = desired.intbUser;
  }
  _stagedInitialized = true;
  _stagedDirty = true;
}

void Cli::markStagedChanged(const char* field) {
  _stagedDirty = true;
  printf("CLI preview: field=%s dirty=1 valid=unknown channel=none outcome=STAGED "
         "code=0 i2c_attempts=0\n",
         field);
  printf("%s[W]%s staged values may be cross-field invalid; run 'profile validate'\n",
         cli_style::code(_colorEnabled, cli_style::Color::YELLOW),
         resetColor(_colorEnabled));
}

void Cli::printConfig(const LDC1614::Config& config, const char* label) const {
  printf("Configuration: %s revision=%lu applied=%s staged_dirty=%u\n", label,
         static_cast<unsigned long>(_device.configRevision()),
         appliedName(_device.appliedConfigState()), _stagedDirty ? 1U : 0U);
  printf("cfg label=%s address=0x%02X variant=%s variant_channels=%u selected=0x%02X "
         "mode=%s ref_source=%s ref_hz=%lu tolerance_ppm=%lu deglitch=%u "
         "activation=%s timeout_ms=%lu rp_override=%u auto_amplitude=%u "
         "high_current=%u intb_config=%u error_reporting=0x%04X revision=%lu "
         "applied=%s\n",
         label, static_cast<unsigned>(config.i2cAddress), variantName(config.variant),
         static_cast<unsigned>(physicalChannelCount(config.variant)),
         static_cast<unsigned>(config.channels.bits), modeName(config.mode),
         config.referenceClock.source == LDC1614::RefClkSrc::INTERNAL
             ? "internal"
             : (config.referenceClock.source == LDC1614::RefClkSrc::EXTERNAL_CLOCK
                    ? "external"
                    : "unspecified"),
         static_cast<unsigned long>(config.referenceClock.frequencyHz),
         static_cast<unsigned long>(config.referenceClock.tolerancePpm),
         static_cast<unsigned>(config.deglitch),
         config.sensorActivation == LDC1614::SensorActivation::FULL_CURRENT
             ? "full"
             : "low",
         static_cast<unsigned long>(config.i2cTimeoutMs),
         config.rpOverrideEnabled ? 1U : 0U,
         config.autoAmplitudeCorrectionEnabled ? 1U : 0U,
         config.highCurrentDriveEnabled ? 1U : 0U,
         config.intbDisabled ? 0U : 1U,
         LDC1614::LDC1614::encodeErrorReporting(config.errorReporting),
         static_cast<unsigned long>(_device.configRevision()),
         appliedName(_device.appliedConfigState()));
  printf("  binding address=0x%02X variant=%s physical_mask=0x%02X "
         "write_callback=%u read_callback=%u\n",
         static_cast<unsigned>(config.i2cAddress), variantName(config.variant),
         static_cast<unsigned>(LDC1614::validChannelMask(config.variant).bits),
         config.i2cWrite != nullptr ? 1U : 0U,
         config.i2cWriteRead != nullptr ? 1U : 0U);
  printf("  conversion selected_mask=0x%02X mode=%s active_channel=%u "
         "rr_sequence=%u\n",
         static_cast<unsigned>(config.channels.bits), modeName(config.mode),
         static_cast<unsigned>(config.activeChannel),
         static_cast<unsigned>(config.rrSequence));
  printf("  reference_clock source=%s frequency_hz=%lu tolerance_ppm=%lu\n",
         config.referenceClock.source == LDC1614::RefClkSrc::INTERNAL
             ? "INTERNAL"
             : (config.referenceClock.source == LDC1614::RefClkSrc::EXTERNAL_CLOCK
                    ? "EXTERNAL"
                    : "UNSPECIFIED"),
         static_cast<unsigned long>(config.referenceClock.frequencyHz),
         static_cast<unsigned long>(config.referenceClock.tolerancePpm));
  printf("  options timeout_ms=%lu deglitch=%u activation=%s rp_override=%u "
         "auto_amplitude=%u high_current=%u intb_enabled=%u intb_callback=%u\n",
         static_cast<unsigned long>(config.i2cTimeoutMs),
         static_cast<unsigned>(config.deglitch),
         config.sensorActivation == LDC1614::SensorActivation::FULL_CURRENT
             ? "FULL"
             : "LOW",
         config.rpOverrideEnabled ? 1U : 0U,
         config.autoAmplitudeCorrectionEnabled ? 1U : 0U,
         config.highCurrentDriveEnabled ? 1U : 0U,
         config.intbDisabled ? 0U : 1U,
         config.intbAsserted != nullptr ? 1U : 0U);
  const uint8_t channels = physicalChannelCount(config.variant);
  for (uint8_t index = 0; index < channels; ++index) {
    const LDC1614::ChannelConfig& channel = config.channel[index];
    uint16_t microamps = 0;
    const LDC1614::Status currentStatus =
        LDC1614::LDC1614::nominalDriveCurrentMicroamps(
            channel.driveCurrentCode, microamps);
    printf("  channel=%u selected=%u rcount=%u settle_count=%u fin_divider=%u "
           "fref_divider=%u offset=%u drive_code=%u drive_ua=%s%u "
           "sensor_min_hz=%lu sensor_max_hz=%lu\n",
           static_cast<unsigned>(index),
           config.channels.contains(static_cast<LDC1614::Channel>(index)) ? 1U : 0U,
           static_cast<unsigned>(channel.rcount),
           static_cast<unsigned>(channel.settleCount),
           static_cast<unsigned>(channel.finDivider),
           static_cast<unsigned>(channel.frefDivider),
           static_cast<unsigned>(channel.offset),
           static_cast<unsigned>(channel.driveCurrentCode.value),
           currentStatus.ok() ? "" : "invalid:", static_cast<unsigned>(microamps),
           static_cast<unsigned long>(channel.expectedSensorMinHz),
           static_cast<unsigned long>(channel.expectedSensorMaxHz));
  }
  const LDC1614::ErrorReporting& error = config.errorReporting;
  printf("  error_config=0x%04X data_under=%u data_over=%u data_watchdog=%u "
         "data_amplitude_high=%u data_amplitude_low=%u status_under=%u "
         "status_over=%u status_watchdog=%u status_amplitude_high=%u "
         "status_amplitude_low=%u status_zero_count=%u data_ready=%u\n",
         LDC1614::LDC1614::encodeErrorReporting(error),
         error.dataUnderRange ? 1U : 0U, error.dataOverRange ? 1U : 0U,
         error.dataWatchdog ? 1U : 0U, error.dataAmplitudeHigh ? 1U : 0U,
         error.dataAmplitudeLow ? 1U : 0U, error.statusUnderRange ? 1U : 0U,
         error.statusOverRange ? 1U : 0U, error.statusWatchdog ? 1U : 0U,
         error.statusAmplitudeHigh ? 1U : 0U,
         error.statusAmplitudeLow ? 1U : 0U,
         error.statusZeroCount ? 1U : 0U, error.dataReady ? 1U : 0U);
}

void Cli::printConfigFault(const LDC1614::ConfigFault& fault) const {
  printf("config_fault valid=%u job=%s phase=%s register=0x%02X channel=%u "
         "effects=0x%02X\n",
         fault.valid ? 1U : 0U, jobName(fault.job), phaseName(fault.phase),
         fault.registerAddress, static_cast<unsigned>(fault.channel),
         static_cast<unsigned>(fault.effects));
  if (fault.valid) printStatus(fault.cause);
}

void Cli::printDriver() const {
  const LDC1614::TransportStats transport = _device.transportStats();
  printState();
  printf("drv bound=%u applied=%s revision=%lu active=%u result_available=%u "
         "attempts=%lu success=%lu failures=%lu last_code=%u\n",
         _device.isBound() ? 1U : 0U, appliedName(_device.appliedConfigState()),
         static_cast<unsigned long>(_device.configRevision()),
         _device.jobProgress().active ? 1U : 0U,
         _device.resultAvailable() ? 1U : 0U,
         static_cast<unsigned long>(transport.totalAttempts),
         static_cast<unsigned long>(transport.totalSuccess),
         static_cast<unsigned long>(transport.totalFailures),
         static_cast<unsigned>(transport.lastStatus.code));
  printf("transport attempts=%lu success=%lu failures=%lu\n",
         static_cast<unsigned long>(transport.totalAttempts),
         static_cast<unsigned long>(transport.totalSuccess),
         static_cast<unsigned long>(transport.totalFailures));
  printStatus(transport.lastStatus);
  printConfigFault(_device.configFault());
}

void Cli::printState() const {
  const LDC1614::JobProgress progress = _device.jobProgress();
  printf("bound=%u applied=%s config_revision=%lu job_active=%u "
         "result_available=%u session=%s staged_dirty=%u\n",
         _device.isBound() ? 1U : 0U, appliedName(_device.appliedConfigState()),
         static_cast<unsigned long>(_device.configRevision()),
         progress.active ? 1U : 0U, _device.resultAvailable() ? 1U : 0U,
         sessionName(_session.kind), _stagedDirty ? 1U : 0U);
  printf("state bound=%u applied=%s profile_dirty=%u session_kind=%s active=%u "
         "pending_result=%u\n",
         _device.isBound() ? 1U : 0U, appliedName(_device.appliedConfigState()),
         _stagedDirty ? 1U : 0U, sessionName(_session.kind),
         progress.active ? 1U : 0U, _device.resultAvailable() ? 1U : 0U);
}

void Cli::printProgress() const {
  const LDC1614::JobProgress progress = _device.jobProgress();
  printf("job active=%u operation=%" PRIu64 " kind=%s phase=%s transfers=%u "
         "maximum=%u requested=0x%02X completed=0x%02X deadline_ms=%" PRIu64
         " effects=0x%02X revision=%lu\n",
         progress.active ? 1U : 0U, progress.operationId, jobName(progress.kind),
         phaseName(progress.phase), static_cast<unsigned>(progress.completedTransfers),
         static_cast<unsigned>(progress.maximumTransfers),
         static_cast<unsigned>(progress.requestedChannels.bits),
         static_cast<unsigned>(progress.completedChannels.bits), progress.deadlineMs,
         static_cast<unsigned>(progress.effects),
         static_cast<unsigned long>(progress.configRevision));
  printf("session active=%u id=%lu kind=%s phase=%u completed=%lu/%lu "
         "pass=%lu fail=%lu skip=%lu\n",
         _session.kind != SessionKind::NONE ? 1U : 0U,
         static_cast<unsigned long>(_session.id), sessionName(_session.kind),
         static_cast<unsigned>(_session.phase),
         static_cast<unsigned long>(_session.stats.completed),
         static_cast<unsigned long>(_session.stats.requested),
         static_cast<unsigned long>(_session.stats.passed),
         static_cast<unsigned long>(_session.stats.failed),
         static_cast<unsigned long>(_session.stats.skipped));
}

void Cli::printDeviceStatus(const LDC1614::DeviceStatus& status,
                            const char* label) const {
  printf("%s=0x%04X observed=%u raw=0x%04X drdy=%u unread=0x%02X err_ch=%u "
         "ur=%u or=%u wd=%u ah=%u al=%u zc=%u\n",
         label, status.raw, status.observed ? 1U : 0U, status.raw,
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

void Cli::printQuality(LDC1614::SampleQualityFlags quality) const {
  bool first = true;
  const struct {
    LDC1614::SampleQualityFlag flag;
    const char* name;
  } names[] = {
      {LDC1614::SampleQualityFlag::FRESH, "FRESH"},
      {LDC1614::SampleQualityFlag::STALE, "STALE"},
      {LDC1614::SampleQualityFlag::UNDER_RANGE, "UNDER_RANGE"},
      {LDC1614::SampleQualityFlag::OVER_RANGE, "OVER_RANGE"},
      {LDC1614::SampleQualityFlag::WATCHDOG, "WATCHDOG"},
      {LDC1614::SampleQualityFlag::AMPLITUDE_SUSPECT, "AMPLITUDE_SUSPECT"},
      {LDC1614::SampleQualityFlag::ZERO_COUNT, "ZERO_COUNT"},
      {LDC1614::SampleQualityFlag::DATA_LOST, "DATA_LOST"},
      {LDC1614::SampleQualityFlag::CONFIG_UNKNOWN, "CONFIG_UNKNOWN"},
  };
  for (const auto& item : names) {
    if (!LDC1614::hasSampleQuality(quality, item.flag)) continue;
    printf("%s%s", first ? "" : "|", item.name);
    first = false;
  }
  if (first) printf("NONE");
}

void Cli::printBatch(const LDC1614::SampleBatch& batch) const {
  printf("batch type=SEQUENTIAL_READOUT selected=0x%02X valid=0x%02X fresh=0x%02X "
         "error=0x%02X overrun=0x%02X revision=%lu completed_ms=%" PRIu64
         " simultaneous=0\n",
         static_cast<unsigned>(batch.selectedChannels.bits),
         static_cast<unsigned>(batch.validChannels.bits),
         static_cast<unsigned>(batch.freshChannels.bits),
         static_cast<unsigned>(batch.errorChannels.bits),
         static_cast<unsigned>(batch.overrunChannels.bits),
         static_cast<unsigned long>(batch.configRevision),
         batch.completedUptimeMs);
  printDeviceStatus(batch.statusBefore, "status_before");
  printDeviceStatus(batch.statusAfter, "status_after");
  for (uint8_t index = 0; index < 4U; ++index) {
    const uint8_t bit = static_cast<uint8_t>(1U << index);
    if ((batch.selectedChannels.bits & bit) == 0U) continue;
    const LDC1614::ChannelSample& sample = batch.channel[index];
    double frequency = 0.0;
    const LDC1614::Status frequencyStatus =
        _device.isBound()
            ? LDC1614::LDC1614::calculateSensorFrequencyHz(
                  _device.config(), static_cast<LDC1614::Channel>(index),
                  sample.rawCount28, frequency)
            : LDC1614::Status::Error(LDC1614::Err::NOT_BOUND, "Driver not bound");
    const bool withinBounds =
        frequencyStatus.ok() && _device.isBound() &&
        frequency >= _device.config().channel[index].expectedSensorMinHz &&
        frequency <= _device.config().channel[index].expectedSensorMaxHz;
    printf("sample channel=%u msb=0x%04X lsb=0x%04X raw=0x%07lX raw28=0x%07lX "
           "quality=0x%04X quality_names=",
           static_cast<unsigned>(index), sample.rawDataMsb, sample.rawDataLsb,
           static_cast<unsigned long>(sample.rawCount28),
           static_cast<unsigned long>(sample.rawCount28), sample.quality);
    printQuality(sample.quality);
    if (frequencyStatus.ok()) {
      printf(" frequency_hz=%.6f bounds=%s\n", frequency,
             withinBounds ? "PASS" : "OUT_OF_RANGE");
    } else {
      printf(" frequency_hz=unavailable bounds=UNKNOWN frequency_code=%u\n",
             static_cast<unsigned>(frequencyStatus.code));
    }
  }
}

void Cli::printResult(const LDC1614::OperationResult& result) const {
  printf("Operation result: operation=%" PRIu64 " kind=%s outcome=%s effects=0x%02X "
         "revision=%lu completed_ms=%" PRIu64 " phase=%s reg=0x%02X "
         "channel=%u transfers=%u maximum=%u code=%u detail=%ld msg=%s\n",
         result.operationId, jobName(result.kind), outcomeName(result.outcome),
         static_cast<unsigned>(result.effects),
         static_cast<unsigned long>(result.configRevision), result.completedUptimeMs,
         phaseName(result.finalProgress.phase), result.finalProgress.registerAddress,
         static_cast<unsigned>(result.finalProgress.channel),
         static_cast<unsigned>(result.finalProgress.completedTransfers),
         static_cast<unsigned>(result.finalProgress.maximumTransfers),
         static_cast<unsigned>(result.status.code),
         static_cast<long>(result.status.detail),
         result.status.msg != nullptr ? result.status.msg : "");
  printStatus(result.status);
  printConfigFault(result.configFault);
  if (result.hasSampleBatch) printBatch(result.sampleBatch);
}

PromptAction Cli::handleProfile(const ParsedLine& line) {
  if (line.argc != 2U && line.argc != 3U) {
    return usage("profile show|reset|discard\nprofile validate\nprofile commit confirm");
  }
  ensureStagedProfile();
  const char* action = line.argv[1];
  if (std::strcmp(action, "show") == 0 && line.argc == 2U) {
    printConfig(_stagedConfig, "staged");
    return promptActionForCurrentState();
  }
  if (std::strcmp(action, "reset") == 0 && line.argc == 2U) {
    resetStagedProfile();
    println("CLI preview: field=profile dirty=1 valid=unknown channel=none "
            "outcome=RESET code=0 i2c_attempts=0");
    return promptActionForCurrentState();
  }
  if (std::strcmp(action, "discard") == 0 && line.argc == 2U) {
    discardStagedProfile();
    println("CLI preview: field=profile dirty=0 valid=unknown channel=none "
            "outcome=DISCARDED code=0 i2c_attempts=0");
    return promptActionForCurrentState();
  }
  if (std::strcmp(action, "validate") == 0 && line.argc == 2U) {
    const LDC1614::Status status = LDC1614::LDC1614::validateConfig(_stagedConfig);
    printf("profile field=all channel=all dirty=%u valid=%u outcome=%s code=%u "
           "i2c_attempts=0\n",
           _stagedDirty ? 1U : 0U, status.ok() ? 1U : 0U,
           status.ok() ? "VALID" : "INVALID", static_cast<unsigned>(status.code));
    printStatus(status);
    return promptActionForCurrentState();
  }
  if (std::strcmp(action, "commit") == 0 && line.argc == 3U &&
      requireConfirmation(line.argv[2])) {
    if (!_device.isBound()) {
      printStatus(LDC1614::Status::Error(LDC1614::Err::NOT_BOUND,
                                         "Bind before profile commit"));
      return promptActionForCurrentState();
    }
    if (_device.jobProgress().active || _session.kind != SessionKind::NONE ||
        _pending.owner != OperationOwner::NONE) {
      printStatus(LDC1614::Status::Error(LDC1614::Err::BUSY,
                                         "CLI work active"));
      return promptActionForCurrentState();
    }
    if (_device.appliedConfigState() !=
        LDC1614::AppliedConfigState::APPLIED_SLEEPING) {
      printStatus(LDC1614::Status::Error(
          LDC1614::Err::BUSY,
          "Profile commit requires verified sleeping hardware; run sleep first"));
      return promptActionForCurrentState();
    }
    const bool wasDirty = _stagedDirty;
    LDC1614::Status status = LDC1614::LDC1614::validateConfig(_stagedConfig);
    if (status.ok() && wasDirty) {
      status = _device.updateDesiredConfig(_stagedConfig);
      if (status.ok()) _stagedDirty = false;
    }
    printf("profile_commit=%s config_revision=%lu applied=%s i2c_attempts=0\n",
           status.ok() ? (wasDirty ? "COMMITTED" : "UNCHANGED") : "REJECTED",
           static_cast<unsigned long>(_device.configRevision()),
           appliedName(_device.appliedConfigState()));
    printStatus(status);
    if (status.ok() && wasDirty) {
      println("Run 'apply' to replay and verify the committed profile.");
    }
    return promptActionForCurrentState();
  }
  return usage("profile show|reset|discard\nprofile validate\nprofile commit confirm");
}

PromptAction Cli::handleSetting(CommandId id, const ParsedLine& line) {
  ensureStagedProfile();
  const CommandSpec* settingSpec = commandSpec(id);
  const char* settingUsage = settingSpec != nullptr ? settingSpec->synopsis
                                                     : "setting";
  uint64_t channelValue = 0;
  uint64_t value = 0;
  bool enabled = false;

  if (id == CommandId::MODE) {
    if (line.argc != 3U) return usage(settingUsage);
    if (std::strcmp(line.argv[1], "single") == 0 &&
        parseUnsigned(line.argv[2], 3U, channelValue)) {
      if (_stagedConfig.variant == LDC1614::DeviceVariant::LDC1612 &&
          channelValue >= 2U) {
        logError("channel %lu is unavailable on LDC1612",
                 static_cast<unsigned long>(channelValue));
        return promptActionForCurrentState();
      }
      const auto channel = static_cast<LDC1614::Channel>(channelValue);
      _stagedConfig.mode = LDC1614::OperatingMode::SINGLE_CHANNEL;
      _stagedConfig.channels = LDC1614::channelBit(channel);
      _stagedConfig.activeChannel = channel;
      _stagedConfig.rrSequence = LDC1614::RRSequence::UNSPECIFIED;
      printf("mode=single channel=%lu count=1\n",
             static_cast<unsigned long>(channelValue));
      markStagedChanged("mode");
      return promptActionForCurrentState();
    }
    if (std::strcmp(line.argv[1], "seq") == 0 &&
        parseUnsigned(line.argv[2], 4U, value) && value >= 2U) {
      if (_stagedConfig.variant == LDC1614::DeviceVariant::LDC1612 && value != 2U) {
        logError("LDC1612 sequential mode supports exactly 2 channels");
        return promptActionForCurrentState();
      }
      LDC1614::ChannelMask channels{};
      LDC1614::RRSequence sequence = LDC1614::RRSequence::UNSPECIFIED;
      if (value == 2U) {
        channels = LDC1614::ChannelMask{0x03};
        sequence = LDC1614::RRSequence::CH0_CH1;
      } else if (value == 3U) {
        channels = LDC1614::ChannelMask{0x07};
        sequence = LDC1614::RRSequence::CH0_CH1_CH2;
      } else if (value == 4U) {
        channels = LDC1614::ChannelMask{0x0F};
        sequence = LDC1614::RRSequence::CH0_CH1_CH2_CH3;
      } else {
        return usage(settingUsage);
      }
      // Commit the validated tuple together; an invalid token never changes
      // staged state or its dirty flag.
      _stagedConfig.mode = LDC1614::OperatingMode::MULTI_CHANNEL_SEQUENTIAL;
      _stagedConfig.activeChannel = LDC1614::Channel::NONE;
      _stagedConfig.channels = channels;
      _stagedConfig.rrSequence = sequence;
      printf("mode=seq channel=none count=%lu\n",
             static_cast<unsigned long>(value));
      markStagedChanged("mode");
      return promptActionForCurrentState();
    }
    return usage(settingUsage);
  }

  if (id == CommandId::REFCLK) {
    uint64_t frequency = 0;
    uint64_t tolerance = 0;
    if (line.argc != 4U ||
        (std::strcmp(line.argv[1], "internal") != 0 &&
         std::strcmp(line.argv[1], "external") != 0) ||
        !parseUnsigned(line.argv[2], UINT32_MAX, frequency) || frequency == 0U ||
        !parseUnsigned(line.argv[3], 999999U, tolerance)) {
      return usage(settingUsage);
    }
    _stagedConfig.referenceClock.source =
        std::strcmp(line.argv[1], "internal") == 0
            ? LDC1614::RefClkSrc::INTERNAL
            : LDC1614::RefClkSrc::EXTERNAL_CLOCK;
    _stagedConfig.referenceClock.frequencyHz = static_cast<uint32_t>(frequency);
    _stagedConfig.referenceClock.tolerancePpm = static_cast<uint32_t>(tolerance);
    printf("source=%s hz=%lu ppm=%lu\n", line.argv[1],
           static_cast<unsigned long>(frequency),
           static_cast<unsigned long>(tolerance));
    markStagedChanged("refclk");
    return promptActionForCurrentState();
  }

  if (id == CommandId::DEGLITCH) {
    if (line.argc != 2U) return usage(settingUsage);
    if (std::strcmp(line.argv[1], "1") == 0) {
      _stagedConfig.deglitch = LDC1614::Deglitch::BW_1MHZ;
    } else if (std::strcmp(line.argv[1], "3") == 0) {
      _stagedConfig.deglitch = LDC1614::Deglitch::BW_3MHZ;
    } else if (std::strcmp(line.argv[1], "10") == 0) {
      _stagedConfig.deglitch = LDC1614::Deglitch::BW_10MHZ;
    } else if (std::strcmp(line.argv[1], "33") == 0) {
      _stagedConfig.deglitch = LDC1614::Deglitch::BW_33MHZ;
    } else {
      return usage(settingUsage);
    }
    printf("mhz=%s\n", line.argv[1]);
    markStagedChanged("deglitch");
    return promptActionForCurrentState();
  }

  if (id == CommandId::ACTIVATION) {
    if (line.argc != 2U ||
        (std::strcmp(line.argv[1], "full") != 0 &&
         std::strcmp(line.argv[1], "low") != 0)) {
      return usage(settingUsage);
    }
    _stagedConfig.sensorActivation =
        std::strcmp(line.argv[1], "full") == 0
            ? LDC1614::SensorActivation::FULL_CURRENT
            : LDC1614::SensorActivation::LOW_POWER;
    printf("mode=%s\n", line.argv[1]);
    markStagedChanged("activation");
    return promptActionForCurrentState();
  }

  if (id == CommandId::TIMEOUT) {
    if (line.argc != 2U || !parseUnsigned(line.argv[1], UINT32_MAX, value) ||
        value == 0U) {
      return usage(settingUsage);
    }
    _stagedConfig.i2cTimeoutMs = static_cast<uint32_t>(value);
    printf("timeout_ms=%lu\n", static_cast<unsigned long>(value));
    markStagedChanged("timeout");
    return promptActionForCurrentState();
  }

  if (id == CommandId::RP || id == CommandId::AUTOAMP ||
      id == CommandId::HIGH_CURRENT || id == CommandId::INTB_CONFIG) {
    if (line.argc != 2U ||
        (std::strcmp(line.argv[1], "0") != 0 &&
         std::strcmp(line.argv[1], "1") != 0)) {
      return usage(settingUsage);
    }
    enabled = std::strcmp(line.argv[1], "1") == 0;
    if (id == CommandId::RP) _stagedConfig.rpOverrideEnabled = enabled;
    if (id == CommandId::AUTOAMP) {
      _stagedConfig.autoAmplitudeCorrectionEnabled = enabled;
    }
    if (id == CommandId::HIGH_CURRENT) {
      _stagedConfig.highCurrentDriveEnabled = enabled;
    }
    if (id == CommandId::INTB_CONFIG) _stagedConfig.intbDisabled = !enabled;
    printf("enabled=%u\n", enabled ? 1U : 0U);
    markStagedChanged(line.argv[0]);
    return promptActionForCurrentState();
  }

  if (id == CommandId::ERRORS) {
    if (line.argc == 1U ||
        (line.argc == 2U && std::strcmp(line.argv[1], "show") == 0)) {
      const LDC1614::ErrorReporting& routes = _stagedConfig.errorReporting;
      printf("errors ur=%u or=%u wd=%u ah=%u al=%u zc=%u drdy=%u\n",
             routes.statusUnderRange ? 1U : 0U,
             routes.statusOverRange ? 1U : 0U,
             routes.statusWatchdog ? 1U : 0U,
             routes.statusAmplitudeHigh ? 1U : 0U,
             routes.statusAmplitudeLow ? 1U : 0U,
             routes.statusZeroCount ? 1U : 0U,
             routes.dataReady ? 1U : 0U);
      return promptActionForCurrentState();
    }
    if (line.argc != 2U ||
        (std::strcmp(line.argv[1], "all") != 0 &&
         std::strcmp(line.argv[1], "none") != 0)) {
      return usage(settingUsage);
    }
    _stagedConfig.errorReporting =
        std::strcmp(line.argv[1], "all") == 0
            ? LDC1614::ErrorReporting::all()
            : LDC1614::ErrorReporting{};
    const LDC1614::ErrorReporting& routes = _stagedConfig.errorReporting;
    printf("ur=%u or=%u wd=%u ah=%u al=%u zc=%u drdy=%u\n",
           routes.statusUnderRange ? 1U : 0U,
           routes.statusOverRange ? 1U : 0U,
           routes.statusWatchdog ? 1U : 0U,
           routes.statusAmplitudeHigh ? 1U : 0U,
           routes.statusAmplitudeLow ? 1U : 0U,
           routes.statusZeroCount ? 1U : 0U, routes.dataReady ? 1U : 0U);
    markStagedChanged("errors");
    return promptActionForCurrentState();
  }

  if (id == CommandId::ERROR) {
    if (line.argc != 3U ||
        (std::strcmp(line.argv[2], "0") != 0 &&
         std::strcmp(line.argv[2], "1") != 0)) {
      return usage(settingUsage);
    }
    enabled = std::strcmp(line.argv[2], "1") == 0;
    LDC1614::ErrorReporting& e = _stagedConfig.errorReporting;
    const char* route = line.argv[1];
    bool* target = nullptr;
    if (std::strcmp(route, "data-under") == 0) target = &e.dataUnderRange;
    else if (std::strcmp(route, "data-over") == 0) target = &e.dataOverRange;
    else if (std::strcmp(route, "data-watchdog") == 0) target = &e.dataWatchdog;
    else if (std::strcmp(route, "data-amplitude-high") == 0) target = &e.dataAmplitudeHigh;
    else if (std::strcmp(route, "data-amplitude-low") == 0) target = &e.dataAmplitudeLow;
    else if (std::strcmp(route, "status-under") == 0) target = &e.statusUnderRange;
    else if (std::strcmp(route, "status-over") == 0) target = &e.statusOverRange;
    else if (std::strcmp(route, "status-watchdog") == 0) target = &e.statusWatchdog;
    else if (std::strcmp(route, "status-amplitude-high") == 0) target = &e.statusAmplitudeHigh;
    else if (std::strcmp(route, "status-amplitude-low") == 0) target = &e.statusAmplitudeLow;
    else if (std::strcmp(route, "status-zero-count") == 0) target = &e.statusZeroCount;
    else if (std::strcmp(route, "data-ready") == 0) target = &e.dataReady;
    if (target == nullptr) {
      return usage(settingUsage);
    }
    *target = enabled;
    printf("field=%s enabled=%u\n", route, enabled ? 1U : 0U);
    markStagedChanged(route);
    return promptActionForCurrentState();
  }

  if (id == CommandId::RCOUNT || id == CommandId::SETTLE ||
      id == CommandId::FIN_DIV || id == CommandId::FREF_DIV ||
      id == CommandId::OFFSET || id == CommandId::DRIVE ||
      id == CommandId::SENSOR_BOUNDS) {
    const uint8_t requiredArguments = id == CommandId::SENSOR_BOUNDS ? 4U : 3U;
    if (line.argc != requiredArguments ||
        !parseUnsigned(line.argv[1], 3U, channelValue)) {
      return usage(settingUsage);
    }
    if (_stagedConfig.variant == LDC1614::DeviceVariant::LDC1612 &&
        channelValue >= 2U) {
      logError("channel %lu is unavailable on LDC1612",
               static_cast<unsigned long>(channelValue));
      return promptActionForCurrentState();
    }
    LDC1614::ChannelConfig& channel = _stagedConfig.channel[channelValue];
    if (id == CommandId::SENSOR_BOUNDS) {
      uint64_t minimum = 0;
      uint64_t maximum = 0;
      if (line.argc != 4U || !parseUnsigned(line.argv[2], UINT32_MAX, minimum) ||
          !parseUnsigned(line.argv[3], UINT32_MAX, maximum)) {
        return usage(settingUsage);
      }
      channel.expectedSensorMinHz = static_cast<uint32_t>(minimum);
      channel.expectedSensorMaxHz = static_cast<uint32_t>(maximum);
      printf("channel=%lu low_hz=%lu high_hz=%lu\n",
             static_cast<unsigned long>(channelValue),
             static_cast<unsigned long>(minimum),
             static_cast<unsigned long>(maximum));
      markStagedChanged("sensorbounds");
      return promptActionForCurrentState();
    }
    uint64_t maximum = UINT16_MAX;
    if (id == CommandId::FIN_DIV) maximum = 15U;
    if (id == CommandId::FREF_DIV) maximum = 1023U;
    if (id == CommandId::DRIVE) maximum = 31U;
    if (line.argc != 3U || !parseUnsigned(line.argv[2], maximum, value) ||
        ((id == CommandId::FIN_DIV || id == CommandId::FREF_DIV) && value == 0U) ||
        (id == CommandId::RCOUNT && value < LDC1614::cmd::RCOUNT_MIN)) {
      return usage(settingUsage);
    }
    if (id == CommandId::RCOUNT) channel.rcount = static_cast<uint16_t>(value);
    if (id == CommandId::SETTLE) channel.settleCount = static_cast<uint16_t>(value);
    if (id == CommandId::FIN_DIV) channel.finDivider = static_cast<uint8_t>(value);
    if (id == CommandId::FREF_DIV) channel.frefDivider = static_cast<uint16_t>(value);
    if (id == CommandId::OFFSET) channel.offset = static_cast<uint16_t>(value);
    if (id == CommandId::DRIVE) {
      channel.driveCurrentCode = LDC1614::DriveCurrentCode(static_cast<uint8_t>(value));
    }
    if (id == CommandId::DRIVE) {
      printf("channel=%lu code=%lu\n", static_cast<unsigned long>(channelValue),
             static_cast<unsigned long>(value));
    } else {
      printf("channel=%lu value=%lu\n", static_cast<unsigned long>(channelValue),
             static_cast<unsigned long>(value));
    }
    markStagedChanged(line.argv[0]);
    return promptActionForCurrentState();
  }

  return usage(settingUsage);
}

PromptAction Cli::handleDecode(const ParsedLine& line) {
  uint64_t first = 0;
  uint64_t second = 0;
  if (line.argc == 3U && std::strcmp(line.argv[1], "status") == 0 &&
      parseUnsigned(line.argv[2], UINT16_MAX, first)) {
    printf("decode kind=status ");
    printDeviceStatus(LDC1614::LDC1614::decodeDeviceStatus(
                          static_cast<uint16_t>(first)),
                      "decoded_status");
    return promptActionForCurrentState();
  }
  if (line.argc == 4U && std::strcmp(line.argv[1], "data") == 0 &&
      parseUnsigned(line.argv[2], UINT16_MAX, first) &&
      parseUnsigned(line.argv[3], UINT16_MAX, second)) {
    const LDC1614::ChannelSample sample =
        LDC1614::LDC1614::decodeChannelSample(static_cast<uint16_t>(first),
                                               static_cast<uint16_t>(second));
    printf("decode kind=data raw=0x%04lX msb=0x%04lX lsb=0x%04lX "
           "count=0x%07lX quality=0x%04X "
           "quality_names=",
           static_cast<unsigned long>(first), static_cast<unsigned long>(first),
           static_cast<unsigned long>(second),
           static_cast<unsigned long>(sample.rawCount28), sample.quality);
    printQuality(sample.quality);
    println();
    return promptActionForCurrentState();
  }
  return usage("decode status <raw16>\ndecode data <msb> <lsb>");
}

PromptAction Cli::handleSd(const ParsedLine& line) {
  if (line.argc == 2U && std::strcmp(line.argv[1], "status") == 0) {
    if (_platform.sdRead == nullptr) {
      printf("%s[SKIP]%s SD callback unavailable; build profile has no wired SD pin\n",
             cli_style::code(_colorEnabled, cli_style::Color::YELLOW),
             resetColor(_colorEnabled));
      println("sd state=unavailable outcome=SKIP code=0");
      return promptActionForCurrentState();
    }
    bool asserted = false;
    const LDC1614::Status status = _platform.sdRead(asserted, _platform.user);
    printf("sd state=%s outcome=%s code=%u\n",
           status.ok() ? (asserted ? "asserted" : "released") : "unknown",
           status.ok() ? "SUCCESS" : "FAILED", static_cast<unsigned>(status.code));
    printStatus(status);
    return promptActionForCurrentState();
  }
  if (line.argc == 3U &&
      (std::strcmp(line.argv[1], "assert") == 0 ||
       std::strcmp(line.argv[1], "release") == 0) &&
      requireConfirmation(line.argv[2])) {
    if (_platform.sdWrite == nullptr) {
      printf("%s[SKIP]%s SD callback unavailable; build profile has no wired SD pin\n",
             cli_style::code(_colorEnabled, cli_style::Color::YELLOW),
             resetColor(_colorEnabled));
      println("sd state=unavailable outcome=SKIP code=0");
      return promptActionForCurrentState();
    }
    const bool asserted = std::strcmp(line.argv[1], "assert") == 0;
    const LDC1614::Status status =
        _platform.sdWrite(asserted, _platform.user);
    if (status.ok() && _device.isBound()) {
      _device.invalidateAppliedState(LDC1614::Status::Error(
          LDC1614::Err::CONFIG_DIRTY,
          asserted ? "Owner asserted SD" : "Owner released SD"));
    }
    printf("sd state=%s outcome=%s code=%u\n",
           status.ok() ? (asserted ? "asserted" : "released") : "unknown",
           status.ok() ? "SUCCESS" : "FAILED", static_cast<unsigned>(status.code));
    printStatus(status);
    if (status.ok()) println("Run 'init' after SD is released before trusted use.");
    return promptActionForCurrentState();
  }
  return usage("sd status|assert|release confirm");
}

LDC1614::OperationId Cli::nextOperationId() {
  const LDC1614::OperationId id = _nextOperationId++;
  if (_nextOperationId == 0U) _nextOperationId = 1U;
  return id;
}

uint32_t Cli::nextSessionId() {
  const uint32_t id = _nextSessionId++;
  if (_nextSessionId == 0U) _nextSessionId = 1U;
  return id;
}

LDC1614::Status Cli::scheduleCommandJob(CommandId command,
                                         LDC1614::JobKind kind,
                                         LDC1614::ChannelMask channels,
                                         uint32_t deadlineMs) {
  if (asynchronousWorkActive()) {
    return LDC1614::Status::Error(LDC1614::Err::BUSY, "CLI work active");
  }
  if (_platform.nowMs == nullptr) {
    return LDC1614::Status::Error(
        LDC1614::Err::INVALID_CONFIG,
        "CLI monotonic clock callback is required for asynchronous work");
  }
  const LDC1614::OperationId operation = nextOperationId();
  const uint64_t started = nowMs();
  const uint64_t deadline = started + deadlineMs;
  LDC1614::Status status;
  if (kind == LDC1614::JobKind::INITIALIZE) {
    status = _device.startInitialize(operation, deadline);
  } else if (kind == LDC1614::JobKind::APPLY_CONFIG) {
    status = _device.startApplyConfig(operation, deadline);
  } else if (kind == LDC1614::JobKind::RESET_AND_REAPPLY) {
    status = _device.startResetAndReapply(operation, deadline);
  } else if (kind == LDC1614::JobKind::ACQUIRE) {
    status = _device.startAcquire(channels, operation, deadline);
  } else {
    return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM,
                                  "Unsupported CLI job kind");
  }
  if (status.inProgress()) {
    _pending.owner = OperationOwner::COMMAND;
    _pending.id = operation;
    _pending.kind = kind;
    _pending.command = command;
    _pending.startedMs = started;
    _promptDeferred = true;
    printf("CLI scheduled: command=%s session=0 operation=%" PRIu64
           " deadline_ms=%" PRIu64 " mask=0x%02X\n",
           commandName(command), operation, deadline,
           static_cast<unsigned>(channels.bits));
  }
  return status;
}

LDC1614::Status Cli::scheduleSessionAcquire() {
  if (_session.kind == SessionKind::NONE ||
      _pending.owner != OperationOwner::NONE || _device.jobProgress().active) {
    return LDC1614::Status::Error(LDC1614::Err::BUSY,
                                  "Cannot admit session acquisition");
  }
  const LDC1614::OperationId operation = nextOperationId();
  const uint64_t started = nowMs();
  const LDC1614::Status status = _device.startAcquire(
      _session.channels, operation, started + JOB_DEADLINE_MS);
  if (status.inProgress()) {
    _pending.owner = OperationOwner::SESSION;
    _pending.id = operation;
    _pending.kind = LDC1614::JobKind::ACQUIRE;
    _pending.command = _session.command;
    _pending.startedMs = started;
    _session.phase = SessionPhase::WAIT_ACQUIRE;
  }
  return status;
}

LDC1614::Status Cli::startSession(SessionKind kind, CommandId command,
                                   uint32_t count,
                                   LDC1614::ChannelMask channels,
                                   uint32_t periodMs, uint8_t channel) {
  if (asynchronousWorkActive()) {
    return LDC1614::Status::Error(LDC1614::Err::BUSY, "CLI work active");
  }
  if (_platform.nowMs == nullptr) {
    return LDC1614::Status::Error(
        LDC1614::Err::INVALID_CONFIG,
        "CLI monotonic clock callback is required for asynchronous work");
  }
  const bool needsBoundDevice =
      kind != SessionKind::SCAN && kind != SessionKind::SELF_TEST;
  if (needsBoundDevice && !_device.isBound()) {
    return LDC1614::Status::Error(LDC1614::Err::NOT_BOUND,
                                  "Driver not bound");
  }
  const bool sampling = kind == SessionKind::WATCH ||
                        kind == SessionKind::SAMPLE_RATE ||
                        kind == SessionKind::STRESS ||
                        kind == SessionKind::STRESS_MIX ||
                        kind == SessionKind::SOAK;
  if (sampling) {
    if (_device.appliedConfigState() !=
        LDC1614::AppliedConfigState::APPLIED_ACTIVE) {
      return LDC1614::Status::Error(LDC1614::Err::CONFIG_DIRTY,
                                    "Sampling requires APPLIED_ACTIVE");
    }
    if (channels.empty() ||
        (channels.bits & static_cast<uint8_t>(~_device.config().channels.bits)) !=
            0U) {
      return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM,
                                    "Session mask must be configured");
    }
  }
  _session = DiagnosticSession{};
  _session.kind = kind;
  _session.command = command;
  _session.id = nextSessionId();
  _session.channels = channels;
  _session.count = count;
  _session.periodMs = periodMs;
  _session.channel = channel;
  _session.scanAddress = FIRST_SCAN_ADDRESS;
  _session.stats.requested = count;
  _session.stats.startedMs = nowMs();
  switch (kind) {
    case SessionKind::PROBE: _session.phase = SessionPhase::PROBE_MANUFACTURER; break;
    case SessionKind::SCAN: _session.phase = SessionPhase::SCAN_ADDRESS; break;
    case SessionKind::DUMP_CONFIG:
    case SessionKind::DUMP_ALL: _session.phase = SessionPhase::DUMP_REGISTER; break;
    case SessionKind::VERIFY: _session.phase = SessionPhase::VERIFY_REGISTER; break;
    case SessionKind::WATCH:
    case SessionKind::SAMPLE_RATE:
    case SessionKind::STRESS:
    case SessionKind::SOAK:
      _session.phase = SessionPhase::WAIT_INTERVAL;
      _session.nextDueMs = _session.stats.startedMs;
      break;
    case SessionKind::STRESS_MIX: _session.phase = SessionPhase::MIX_STATUS; break;
    case SessionKind::SELF_TEST: _session.phase = SessionPhase::SELF_PURE; break;
    case SessionKind::NONE:
      return LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM,
                                    "Invalid diagnostic session");
  }
  _promptDeferred = true;
  printf("CLI scheduled: command=%s session=%lu\n", commandName(command),
         static_cast<unsigned long>(_session.id));
  return LDC1614::Status{LDC1614::Err::IN_PROGRESS, 0,
                         "CLI diagnostic session scheduled"};
}

void Cli::recordSessionFailure(const LDC1614::Status& status) {
  if (!_session.stats.hasFailure) {
    _session.stats.hasFailure = true;
    _session.stats.firstFailure = status;
  }
  _session.stats.lastFailure = status;
}

void Cli::recordLatency(uint64_t startedMs, uint64_t completedMs) {
  const uint64_t elapsed64 = completedMs - startedMs;
  const uint32_t elapsed =
      elapsed64 > UINT32_MAX ? UINT32_MAX : static_cast<uint32_t>(elapsed64);
  ++_session.stats.latencySamples;
  _session.stats.latencyTotalMs += elapsed;
  if (elapsed < _session.stats.minimumLatencyMs) {
    _session.stats.minimumLatencyMs = elapsed;
  }
  if (elapsed > _session.stats.maximumLatencyMs) {
    _session.stats.maximumLatencyMs = elapsed;
  }
}

void Cli::recordBatchStats(const LDC1614::SampleBatch& batch) {
  for (uint8_t index = 0; index < 4U; ++index) {
    const uint8_t bit = static_cast<uint8_t>(1U << index);
    if ((batch.selectedChannels.bits & bit) == 0U) continue;
    ChannelStats& stats = _session.stats.channel[index];
    ++stats.selected;
    if ((batch.validChannels.bits & bit) != 0U) ++stats.valid;
    if ((batch.freshChannels.bits & bit) != 0U) ++stats.fresh;
    if ((batch.errorChannels.bits & bit) != 0U) ++stats.errors;
    if ((batch.overrunChannels.bits & bit) != 0U) ++stats.overruns;
    if (!_device.isBound()) continue;
    double frequency = 0.0;
    const LDC1614::Status status = LDC1614::LDC1614::calculateSensorFrequencyHz(
        _device.config(), static_cast<LDC1614::Channel>(index),
        batch.channel[index].rawCount28, frequency);
    if (!status.ok()) continue;
    if (!stats.hasFrequency) {
      stats.minimumHz = frequency;
      stats.maximumHz = frequency;
      stats.hasFrequency = true;
    } else {
      if (frequency < stats.minimumHz) stats.minimumHz = frequency;
      if (frequency > stats.maximumHz) stats.maximumHz = frequency;
    }
    const LDC1614::ChannelConfig& config = _device.config().channel[index];
    if (frequency < config.expectedSensorMinHz ||
        frequency > config.expectedSensorMaxHz) {
      ++stats.boundsFailures;
    }
  }
}

void Cli::selfTestPass(const char* name) {
  ++_session.stats.passed;
  printf("%s[PASS]%s %s\n",
         cli_style::code(_colorEnabled, cli_style::Color::GREEN),
         resetColor(_colorEnabled), name);
}

void Cli::selfTestFail(const char* name, const LDC1614::Status& status) {
  ++_session.stats.failed;
  recordSessionFailure(status);
  printf("%s[FAIL]%s %s code=%u detail=%ld msg=%s\n",
         cli_style::code(_colorEnabled, cli_style::Color::RED),
         resetColor(_colorEnabled), name, static_cast<unsigned>(status.code),
         static_cast<long>(status.detail), status.msg != nullptr ? status.msg : "");
}

void Cli::selfTestSkip(const char* name, const char* reason) {
  ++_session.stats.skipped;
  printf("%s[SKIP]%s %s: %s\n",
         cli_style::code(_colorEnabled, cli_style::Color::YELLOW),
         resetColor(_colorEnabled), name, reason);
}

void Cli::printSessionSummary() const {
  const uint64_t elapsed = nowMs() - _session.stats.startedMs;
  const double hz = elapsed == 0U
                        ? 0.0
                        : static_cast<double>(_session.stats.completed) * 1000.0 /
                              static_cast<double>(elapsed);
  switch (_session.kind) {
    case SessionKind::PROBE:
      printf("manufacturer_id=0x%04X device_id=0x%04X match=%s code=%u\n",
             _session.manufacturerId, _session.deviceId,
             _session.stats.failed == 0U ? "YES" : "NO",
             _session.stats.failed == 0U
                 ? static_cast<unsigned>(LDC1614::Err::OK)
                 : static_cast<unsigned>(_session.stats.lastFailure.code));
      break;
    case SessionKind::SCAN:
      printf("scan complete found=%u probes=%u code=%u\n",
             static_cast<unsigned>(_session.scanFound), SCAN_PROBE_COUNT,
             _session.stats.failed == 0U
                 ? static_cast<unsigned>(LDC1614::Err::OK)
                 : static_cast<unsigned>(_session.stats.lastFailure.code));
      break;
    case SessionKind::DUMP_CONFIG:
    case SessionKind::DUMP_ALL:
      printf("dump complete scope=%s count=%lu failures=%lu\n",
             _session.kind == SessionKind::DUMP_CONFIG ? "config" : "all",
             static_cast<unsigned long>(_session.stats.completed),
             static_cast<unsigned long>(_session.stats.failed));
      break;
    case SessionKind::VERIFY:
      printf("verify complete checked=%lu matched=%lu mismatched=%lu "
             "read_failures=%lu\n",
             static_cast<unsigned long>(_session.stats.completed),
             static_cast<unsigned long>(_session.stats.passed),
             static_cast<unsigned long>(_session.stats.failed),
             static_cast<unsigned long>(_session.stats.skipped));
      break;
    case SessionKind::WATCH:
      printf("Watch results: requested=%lu completed=%lu failed=%lu elapsed_ms=%" PRIu64
             "\n",
             static_cast<unsigned long>(_session.stats.requested),
             static_cast<unsigned long>(_session.stats.completed),
             static_cast<unsigned long>(_session.stats.failed), elapsed);
      break;
    case SessionKind::SAMPLE_RATE:
      printf("SampleRate result: requested=%lu ok=%lu fail=%lu elapsed_ms=%" PRIu64
             " hz=%.6f ready_checks=%lu ready_status_raw=0x%04X\n",
             static_cast<unsigned long>(_session.stats.requested),
             static_cast<unsigned long>(_session.stats.passed),
             static_cast<unsigned long>(_session.stats.failed), elapsed, hz,
             static_cast<unsigned long>(_session.readyChecks),
             _session.hasReadyStatus ? _session.lastReadyStatus.raw : 0U);
      break;
    case SessionKind::STRESS:
      printf("Stress results: %lu ok, %lu failed\n",
             static_cast<unsigned long>(_session.stats.passed),
             static_cast<unsigned long>(_session.stats.failed));
      printf("Stress result: requested=%lu ok=%lu fail=%lu elapsed_ms=%" PRIu64
             " hz=%.6f\n",
             static_cast<unsigned long>(_session.stats.requested),
             static_cast<unsigned long>(_session.stats.passed),
             static_cast<unsigned long>(_session.stats.failed), elapsed, hz);
      break;
    case SessionKind::STRESS_MIX:
      printf("StressMix results: requested=%lu ok=%lu fail=%lu elapsed_ms=%" PRIu64
             "\n",
             static_cast<unsigned long>(_session.stats.requested),
             static_cast<unsigned long>(_session.stats.passed),
             static_cast<unsigned long>(_session.stats.failed), elapsed);
      break;
    case SessionKind::SOAK:
      printf("Soak results: seconds=%lu cycles=%lu ok=%lu fail=%lu elapsed_ms=%" PRIu64
             "\n",
             static_cast<unsigned long>(_session.count),
             static_cast<unsigned long>(_session.stats.completed),
             static_cast<unsigned long>(_session.stats.passed),
             static_cast<unsigned long>(_session.stats.failed), elapsed);
      break;
    case SessionKind::SELF_TEST:
      printf("selftest identity=completed config=completed status=completed "
             "helpers=completed\n");
      printf("Selftest result: pass=%lu fail=%lu skip=%lu\n",
             static_cast<unsigned long>(_session.stats.passed),
             static_cast<unsigned long>(_session.stats.failed),
             static_cast<unsigned long>(_session.stats.skipped));
      break;
    case SessionKind::NONE: break;
  }
  if (_session.kind == SessionKind::WATCH ||
      _session.kind == SessionKind::SAMPLE_RATE ||
      _session.kind == SessionKind::STRESS ||
      _session.kind == SessionKind::STRESS_MIX ||
      _session.kind == SessionKind::SOAK) {
    for (uint8_t index = 0; index < 4U; ++index) {
      const ChannelStats& channel = _session.stats.channel[index];
      if (channel.selected == 0U) continue;
      printf("session_channel=%u selected=%lu valid=%lu fresh=%lu error=%lu "
             "overrun=%lu bounds_fail=%lu",
             static_cast<unsigned>(index),
             static_cast<unsigned long>(channel.selected),
             static_cast<unsigned long>(channel.valid),
             static_cast<unsigned long>(channel.fresh),
             static_cast<unsigned long>(channel.errors),
             static_cast<unsigned long>(channel.overruns),
             static_cast<unsigned long>(channel.boundsFailures));
      if (channel.hasFrequency) {
        printf(" min_hz=%.6f max_hz=%.6f", channel.minimumHz, channel.maximumHz);
      }
      println();
    }
  }
  if (_session.stats.latencySamples != 0U) {
    const uint64_t mean =
        _session.stats.latencyTotalMs / _session.stats.latencySamples;
    printf("session_latency samples=%lu total_ms=%" PRIu64
           " min_ms=%lu max_ms=%lu mean_ms=%" PRIu64 "\n",
           static_cast<unsigned long>(_session.stats.latencySamples),
           _session.stats.latencyTotalMs,
           static_cast<unsigned long>(_session.stats.minimumLatencyMs),
           static_cast<unsigned long>(_session.stats.maximumLatencyMs), mean);
  }
}

void Cli::finishSession(const LDC1614::Status& status, const char* outcome) {
  const CommandId command = _session.command;
  const uint32_t sessionId = _session.id;
  printSessionSummary();
  if (_session.stats.hasFailure) {
    printf("first_failure ");
    printStatus(_session.stats.firstFailure);
    printf("last_failure ");
    printStatus(_session.stats.lastFailure);
  }
  printf("CLI result: command=%s session=%lu outcome=%s code=%u\n",
         commandName(command), static_cast<unsigned long>(sessionId), outcome,
         static_cast<unsigned>(status.code));
  _session = DiagnosticSession{};
}

void Cli::advanceSamplingSession(uint64_t now) {
  if (_session.stopRequested) {
    finishSession(LDC1614::Status::Error(LDC1614::Err::CANCELLED,
                                         "CLI session cancelled"),
                  "CANCELLED");
    return;
  }
  if (_session.kind == SessionKind::SOAK) {
    const uint64_t durationMs = static_cast<uint64_t>(_session.count) * 1000ULL;
    if (now - _session.stats.startedMs >= durationMs) {
      finishSession(_session.stats.failed == 0U
                        ? LDC1614::Status::Ok()
                        : _session.stats.lastFailure,
                    _session.stats.failed == 0U ? "SUCCESS" : "FAILED");
      return;
    }
  } else if (_session.stats.completed >= _session.stats.requested) {
    finishSession(_session.stats.failed == 0U
                      ? LDC1614::Status::Ok()
                      : _session.stats.lastFailure,
                  _session.stats.failed == 0U ? "SUCCESS" : "FAILED");
    return;
  }
  if (_session.phase == SessionPhase::WAIT_INTERVAL &&
      deadlineReached(now, _session.nextDueMs) &&
      _session.kind == SessionKind::SAMPLE_RATE) {
    _session.phase = SessionPhase::SAMPLE_READY;
    _session.readyDeadlineMs = now + JOB_DEADLINE_MS;
    _session.sampleReadyChecks = 0U;
  }
  if (_session.kind == SessionKind::SAMPLE_RATE &&
      _session.phase == SessionPhase::SAMPLE_READY) {
    if (deadlineReached(now, _session.readyDeadlineMs)) {
      const LDC1614::Status timeout = LDC1614::Status::Error(
          LDC1614::Err::TIMEOUT,
          "Sample-rate readiness deadline expired");
      ++_session.stats.completed;
      ++_session.stats.failed;
      recordSessionFailure(timeout);
      finishSession(timeout, "FAILED");
      return;
    }
    bool ready = false;
    LDC1614::DeviceStatus observed;
    const LDC1614::Status readyStatus = _device.readDataReady(ready, observed);
    ++_session.readyChecks;
    ++_session.sampleReadyChecks;
    if (readyStatus.ok()) {
      _session.lastReadyStatus = observed;
      _session.hasReadyStatus = true;
    }
    printf("samplerate_ready sample=%lu check=%lu ready=%u status_snapshot=%u "
           "status_raw=0x%04X code=%u deadline_ms=%" PRIu64 "\n",
           static_cast<unsigned long>(_session.index),
           static_cast<unsigned long>(_session.sampleReadyChecks), ready ? 1U : 0U,
           readyStatus.ok() ? 1U : 0U, observed.raw,
           static_cast<unsigned>(readyStatus.code), _session.readyDeadlineMs);
    if (!readyStatus.ok()) {
      ++_session.stats.completed;
      ++_session.stats.failed;
      recordSessionFailure(readyStatus);
      finishSession(readyStatus, "FAILED");
      return;
    }
    if (!ready) return;
    const LDC1614::Status acquireStatus = scheduleSessionAcquire();
    if (!acquireStatus.inProgress()) {
      ++_session.stats.completed;
      ++_session.stats.failed;
      recordSessionFailure(acquireStatus);
      finishSession(acquireStatus, "FAILED");
    }
    return;
  }
  if (_session.phase != SessionPhase::WAIT_INTERVAL ||
      !deadlineReached(now, _session.nextDueMs)) {
    return;
  }
  const LDC1614::Status status = scheduleSessionAcquire();
  if (!status.inProgress()) {
    ++_session.stats.completed;
    ++_session.stats.failed;
    recordSessionFailure(status);
    finishSession(status, "FAILED");
  }
}

void Cli::advanceMixedSession() {
  if (_session.stopRequested) {
    finishSession(LDC1614::Status::Error(LDC1614::Err::CANCELLED,
                                         "CLI session cancelled"),
                  "CANCELLED");
    return;
  }
  if (_session.stats.completed >= _session.stats.requested) {
    finishSession(_session.stats.failed == 0U
                      ? LDC1614::Status::Ok()
                      : _session.stats.lastFailure,
                  _session.stats.failed == 0U ? "SUCCESS" : "FAILED");
    return;
  }
  LDC1614::Status status = LDC1614::Status::Ok();
  if (_session.phase == SessionPhase::MIX_STATUS) {
    LDC1614::DeviceStatus deviceStatus;
    status = _device.readDeviceStatus(deviceStatus);
    if (_verbose && status.ok()) printDeviceStatus(deviceStatus, "mix_status");
    if (!status.ok()) {
      _session.iterationFailed = true;
      recordSessionFailure(status);
    }
    _session.phase = SessionPhase::MIX_READY;
    return;
  }
  if (_session.phase == SessionPhase::MIX_READY) {
    bool ready = false;
    LDC1614::DeviceStatus observed;
    status = _device.readDataReady(ready, observed);
    if (_verbose) printf("mix_ready=%u\n", ready ? 1U : 0U);
    if (!status.ok()) {
      _session.iterationFailed = true;
      recordSessionFailure(status);
    }
    _session.phase = SessionPhase::MIX_INTB;
    return;
  }
  if (_session.phase == SessionPhase::MIX_INTB) {
    bool asserted = false;
    status = _device.readIntb(asserted);
    if (status.is(LDC1614::Err::INVALID_CONFIG)) {
      ++_session.stats.skipped;
    } else if (!status.ok()) {
      _session.iterationFailed = true;
      recordSessionFailure(status);
    } else if (_verbose) {
      printf("mix_intb=%u\n", asserted ? 1U : 0U);
    }
    _session.phase = SessionPhase::MIX_INIT_DRIVE;
    return;
  }
  if (_session.phase == SessionPhase::MIX_INIT_DRIVE) {
    const uint8_t channelCount = physicalChannelCount(_device.config().variant);
    const uint8_t channel =
        channelCount == 0U ? 0U : static_cast<uint8_t>(_session.index % channelCount);
    uint8_t code = 0;
    status = _device.readInitDriveCurrent(static_cast<LDC1614::Channel>(channel), code);
    if (_verbose) {
      printf("mix_initdrive channel=%u code=%u\n", static_cast<unsigned>(channel),
             static_cast<unsigned>(code));
    }
    if (!status.ok()) {
      _session.iterationFailed = true;
      recordSessionFailure(status);
    }
    _session.phase = SessionPhase::MIX_ACQUIRE;
    return;
  }
  if (_session.phase == SessionPhase::MIX_ACQUIRE) {
    status = scheduleSessionAcquire();
    if (!status.inProgress()) {
      _session.iterationFailed = true;
      ++_session.stats.completed;
      ++_session.stats.failed;
      recordSessionFailure(status);
      _session.iterationFailed = false;
      ++_session.index;
      _session.phase = SessionPhase::MIX_STATUS;
    }
  }
}

void Cli::advanceProbeSession() {
  uint16_t value = 0;
  if (_session.phase == SessionPhase::PROBE_MANUFACTURER) {
    const LDC1614::Status status =
        _device.readRegister16(LDC1614::cmd::REG_MANUFACTURER_ID, value);
    if (!status.ok()) {
      recordSessionFailure(status);
      ++_session.stats.failed;
      finishSession(status, "FAILED");
      return;
    }
    _session.manufacturerId = value;
    _session.phase = SessionPhase::PROBE_DEVICE;
    return;
  }
  if (_session.phase == SessionPhase::PROBE_DEVICE) {
    LDC1614::Status status =
        _device.readRegister16(LDC1614::cmd::REG_DEVICE_ID, value);
    if (status.ok()) _session.deviceId = value;
    if (status.ok() &&
        (_session.manufacturerId != LDC1614::cmd::MANUFACTURER_ID_VALUE ||
         _session.deviceId != LDC1614::cmd::DEVICE_ID_VALUE)) {
      status = LDC1614::Status::Error(LDC1614::Err::DEVICE_NOT_FOUND,
                                      "Identity mismatch");
    }
    if (!status.ok()) {
      recordSessionFailure(status);
      ++_session.stats.failed;
    } else {
      ++_session.stats.passed;
    }
    _session.stats.completed = 2U;
    finishSession(status, status.ok() ? "SUCCESS" : "FAILED");
  }
}

void Cli::advanceScanSession() {
  if (_session.stopRequested) {
    finishSession(LDC1614::Status::Error(LDC1614::Err::CANCELLED,
                                         "CLI session cancelled"),
                  "CANCELLED");
    return;
  }
  if (_platform.i2cProbe == nullptr) {
    const LDC1614::Status status = LDC1614::Status::Error(
        LDC1614::Err::INVALID_CONFIG, "I2C probe callback unavailable");
    recordSessionFailure(status);
    finishSession(status, "FAILED");
    return;
  }
  const uint8_t address = _session.scanAddress;
  const I2cProbeResult result =
      _platform.i2cProbe(address, _platform.scanTimeoutMs, _platform.user);
  ++_session.stats.completed;
  if (result == I2cProbeResult::ACK) {
    ++_session.scanFound;
    printf("I2C device at 0x%02X\n", address);
  } else if (result == I2cProbeResult::TIMEOUT ||
             result == I2cProbeResult::ERROR) {
    const LDC1614::Status status = LDC1614::Status::Error(
        result == I2cProbeResult::TIMEOUT ? LDC1614::Err::I2C_TIMEOUT
                                          : LDC1614::Err::I2C_BUS,
        result == I2cProbeResult::TIMEOUT ? "I2C scan probe timed out"
                                          : "I2C scan probe failed",
        address);
    ++_session.stats.failed;
    recordSessionFailure(status);
    finishSession(status, "FAILED");
    return;
  }
  if (address == LAST_SCAN_ADDRESS) {
    finishSession(LDC1614::Status::Ok(), "SUCCESS");
  } else {
    ++_session.scanAddress;
  }
}

void Cli::advanceDumpSession() {
  const bool all = _session.kind == SessionKind::DUMP_ALL;
  const uint8_t* registers = all ? ALL_REGISTERS : CONFIG_REGISTERS;
  const size_t registerCount =
      all ? sizeof(ALL_REGISTERS) : sizeof(CONFIG_REGISTERS);
  while (_session.registerIndex < registerCount &&
         isChannelRegisterForUnavailableChannel(
             _device.config(), registers[_session.registerIndex])) {
    ++_session.registerIndex;
  }
  if (_session.registerIndex >= registerCount) {
    finishSession(_session.stats.failed == 0U
                      ? LDC1614::Status::Ok()
                      : _session.stats.lastFailure,
                  _session.stats.failed == 0U ? "SUCCESS" : "FAILED");
    return;
  }
  const uint8_t reg = registers[_session.registerIndex++];
  uint16_t value = 0;
  const LDC1614::Status status = _device.readRegister16(reg, value);
  ++_session.stats.completed;
  if (status.ok()) {
    ++_session.stats.passed;
    printf("dump scope=%s register=0x%02X value=0x%04X\n",
           all ? "all" : "config", reg, value);
  } else {
    ++_session.stats.failed;
    recordSessionFailure(status);
    printf("dump scope=%s register=0x%02X read_failed code=%u\n",
           all ? "all" : "config", reg,
           static_cast<unsigned>(status.code));
  }
}

void Cli::advanceVerifySession(bool selfTest) {
  while (_session.registerIndex < sizeof(CONFIG_REGISTERS)) {
    const uint8_t reg = CONFIG_REGISTERS[_session.registerIndex];
    uint16_t expected = 0;
    uint16_t mask = 0;
    const LDC1614::Status expectedStatus =
        LDC1614::LDC1614::expectedConfigurationRegister(
            _device.config(), reg, expected, mask);
    if (expectedStatus.is(LDC1614::Err::INVALID_PARAM)) {
      ++_session.registerIndex;
      continue;
    }
    if (!expectedStatus.ok()) {
      if (selfTest) {
        selfTestFail("configuration expectation", expectedStatus);
      } else {
        ++_session.stats.failed;
        recordSessionFailure(expectedStatus);
      }
      finishSession(expectedStatus, "FAILED");
      return;
    }
    uint16_t actual = 0;
    const LDC1614::Status readStatus = _device.readRegister16(reg, actual);
    ++_session.registerIndex;
    ++_session.stats.completed;
    if (!readStatus.ok()) {
      if (selfTest) {
        selfTestFail("configuration register read", readStatus);
      } else {
        ++_session.stats.skipped;
        ++_session.stats.failed;
        recordSessionFailure(readStatus);
      }
      printf("verify register=0x%02X read_failed code=%u\n", reg,
             static_cast<unsigned>(readStatus.code));
      return;
    }
    const bool match = (actual & mask) == (expected & mask);
    printf("verify register=0x%02X actual=0x%04X expected=0x%04X mask=0x%04X "
           "match=%s\n",
           reg, actual, expected, mask, match ? "YES" : "NO");
    if (selfTest) {
      if (match) {
        selfTestPass("configuration register");
      } else {
        selfTestFail("configuration register",
                     LDC1614::Status::Error(LDC1614::Err::CONFIG_DIRTY,
                                            "Configuration readback mismatch", reg));
      }
    } else if (match) {
      ++_session.stats.passed;
    } else {
      ++_session.stats.failed;
      recordSessionFailure(LDC1614::Status::Error(
          LDC1614::Err::CONFIG_DIRTY, "Configuration readback mismatch", reg));
    }
    return;
  }
  if (selfTest) {
    _session.phase = SessionPhase::SELF_ACQUIRE;
  } else {
    finishSession(_session.stats.failed == 0U
                      ? LDC1614::Status::Ok()
                      : _session.stats.lastFailure,
                  _session.stats.failed == 0U ? "SUCCESS" : "FAILED");
  }
}

void Cli::advanceSelfTestSession() {
  if (_session.stopRequested) {
    finishSession(LDC1614::Status::Error(LDC1614::Err::CANCELLED,
                                         "CLI session cancelled"),
                  "CANCELLED");
    return;
  }
  if (_session.phase == SessionPhase::SELF_PURE) {
    const LDC1614::Status validation =
        _device.isBound()
            ? LDC1614::LDC1614::validateConfig(_device.config())
            : LDC1614::Status::Error(LDC1614::Err::NOT_BOUND,
                                     "Driver not bound");
    if (validation.ok()) selfTestPass("desired configuration validation");
    else selfTestFail("desired configuration validation", validation);

    LDC1614::FrameTiming timing;
    const LDC1614::Status timingStatus =
        _device.isBound()
            ? LDC1614::LDC1614::estimateFrameTiming(
                  _device.config(), _device.config().channels, timing)
            : validation;
    if (timingStatus.ok()) selfTestPass("frame timing helper");
    else selfTestFail("frame timing helper", timingStatus);

    bool driveHelperOk = true;
    for (uint8_t code = 0; code <= 31U; ++code) {
      uint16_t microamps = 0;
      if (!LDC1614::LDC1614::nominalDriveCurrentMicroamps(
               LDC1614::DriveCurrentCode(code), microamps)
               .ok() ||
          microamps == 0U) {
        driveHelperOk = false;
        break;
      }
    }
    if (driveHelperOk) {
      selfTestPass("drive-current helper");
    } else {
      selfTestFail("drive-current helper",
                   LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM,
                                          "Drive-current helper failed"));
    }

    const LDC1614::DeviceStatus decodedStatus =
        LDC1614::LDC1614::decodeDeviceStatus(0U);
    const LDC1614::ChannelSample decodedSample =
        LDC1614::LDC1614::decodeChannelSample(0U, 0U);
    if (decodedStatus.observed && decodedStatus.raw == 0U &&
        decodedSample.rawCount28 == 0U) {
      selfTestPass("STATUS/DATA decode helpers");
    } else {
      selfTestFail("STATUS/DATA decode helpers",
                   LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM,
                                          "Decode helper invariant failed"));
    }

    const uint16_t encoded = LDC1614::LDC1614::encodeErrorReporting(
        LDC1614::ErrorReporting::all());
    if (encoded != 0U) selfTestPass("error-routing encoder");
    else selfTestFail("error-routing encoder",
                      LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM,
                                             "Error encoder returned zero"));
    _session.phase = SessionPhase::SELF_MANUFACTURER;
    return;
  }

  if (_session.phase == SessionPhase::SELF_MANUFACTURER) {
    uint16_t value = 0;
    LDC1614::Status status =
        _device.readRegister16(LDC1614::cmd::REG_MANUFACTURER_ID, value);
    if (status.ok() && value != LDC1614::cmd::MANUFACTURER_ID_VALUE) {
      status = LDC1614::Status::Error(LDC1614::Err::DEVICE_NOT_FOUND,
                                      "Manufacturer ID mismatch", value);
    }
    _session.manufacturerId = value;
    if (status.ok()) selfTestPass("MANUFACTURER_ID");
    else selfTestFail("MANUFACTURER_ID", status);
    _session.phase = SessionPhase::SELF_DEVICE;
    return;
  }

  if (_session.phase == SessionPhase::SELF_DEVICE) {
    uint16_t value = 0;
    LDC1614::Status status =
        _device.readRegister16(LDC1614::cmd::REG_DEVICE_ID, value);
    if (status.ok() && value != LDC1614::cmd::DEVICE_ID_VALUE) {
      status = LDC1614::Status::Error(LDC1614::Err::DEVICE_NOT_FOUND,
                                      "Device ID mismatch", value);
    }
    _session.deviceId = value;
    if (status.ok()) selfTestPass("DEVICE_ID");
    else selfTestFail("DEVICE_ID", status);
    _session.phase = SessionPhase::SELF_STATUS;
    return;
  }

  if (_session.phase == SessionPhase::SELF_STATUS) {
    LDC1614::DeviceStatus deviceStatus;
    const LDC1614::Status status = _device.readDeviceStatus(deviceStatus);
    if (status.ok()) {
      selfTestPass("STATUS transport/decode");
      if (_verbose) printDeviceStatus(deviceStatus, "selftest_status");
    } else {
      selfTestFail("STATUS transport/decode", status);
    }
    _session.phase = SessionPhase::SELF_READY;
    return;
  }

  if (_session.phase == SessionPhase::SELF_READY) {
    bool ready = false;
    LDC1614::DeviceStatus observed;
    const LDC1614::Status status = _device.readDataReady(ready, observed);
    if (status.ok()) selfTestPass("readiness API");
    else selfTestFail("readiness API", status);
    _session.phase = SessionPhase::SELF_INTB;
    return;
  }

  if (_session.phase == SessionPhase::SELF_INTB) {
    bool asserted = false;
    const LDC1614::Status status = _device.readIntb(asserted);
    if (status.ok()) {
      selfTestPass("INTB observation");
    } else if (status.is(LDC1614::Err::INVALID_CONFIG)) {
      selfTestSkip("INTB observation", "callback/output not configured");
    } else {
      selfTestFail("INTB observation", status);
    }
    _session.physicalChannel = 0U;
    _session.phase = SessionPhase::SELF_INIT_DRIVE;
    return;
  }

  if (_session.phase == SessionPhase::SELF_INIT_DRIVE) {
    const uint8_t channelCount = physicalChannelCount(_device.config().variant);
    if (_session.physicalChannel >= channelCount) {
      _session.registerIndex = 0U;
      _session.phase = SessionPhase::SELF_VERIFY;
      return;
    }
    const uint8_t channel = _session.physicalChannel++;
    uint8_t code = 0;
    const LDC1614::Status status = _device.readInitDriveCurrent(
        static_cast<LDC1614::Channel>(channel), code);
    char name[40];
    std::snprintf(name, sizeof(name), "INIT_IDRIVE channel %u",
                  static_cast<unsigned>(channel));
    if (status.ok()) selfTestPass(name);
    else selfTestFail(name, status);
    return;
  }

  if (_session.phase == SessionPhase::SELF_VERIFY) {
    advanceVerifySession(true);
    return;
  }

  if (_session.phase == SessionPhase::SELF_ACQUIRE) {
    if (_device.appliedConfigState() !=
        LDC1614::AppliedConfigState::APPLIED_ACTIVE) {
      selfTestSkip("acquisition transport", "device is not APPLIED_ACTIVE");
      selfTestSkip("sensor quality", "no acquisition was attempted");
      finishSession(_session.stats.failed == 0U
                        ? LDC1614::Status::Ok()
                        : _session.stats.lastFailure,
                    _session.stats.failed == 0U ? "SUCCESS" : "FAILED");
      return;
    }
    _session.channels = _device.config().channels;
    const LDC1614::Status status = scheduleSessionAcquire();
    if (!status.inProgress()) {
      selfTestFail("acquisition transport", status);
      selfTestSkip("sensor quality", "acquisition start failed");
      finishSession(status, "FAILED");
    }
  }
}

void Cli::advanceSession(uint64_t now) {
  switch (_session.kind) {
    case SessionKind::NONE: return;
    case SessionKind::PROBE: advanceProbeSession(); return;
    case SessionKind::SCAN: advanceScanSession(); return;
    case SessionKind::DUMP_CONFIG:
    case SessionKind::DUMP_ALL: advanceDumpSession(); return;
    case SessionKind::VERIFY: advanceVerifySession(false); return;
    case SessionKind::WATCH:
    case SessionKind::SAMPLE_RATE:
    case SessionKind::STRESS:
    case SessionKind::SOAK: advanceSamplingSession(now); return;
    case SessionKind::STRESS_MIX: advanceMixedSession(); return;
    case SessionKind::SELF_TEST: advanceSelfTestSession(); return;
  }
}

void Cli::handleSessionOperationResult(
    const LDC1614::OperationResult& result) {
  _lastResult = result;
  _hasLastResult = true;
  const bool success = result.outcome == LDC1614::TerminalOutcome::SUCCESS &&
                       result.status.ok() && result.hasSampleBatch;
  if (result.outcome != LDC1614::TerminalOutcome::CANCELLED) {
    recordLatency(_pending.startedMs, result.completedUptimeMs);
  }
  if (!success) printResult(result);
  if (success) {
    _lastBatch = result.sampleBatch;
    _hasLastBatch = true;
    recordBatchStats(result.sampleBatch);
  } else {
    recordSessionFailure(result.status);
  }

  if (_session.stopRequested ||
      result.outcome == LDC1614::TerminalOutcome::CANCELLED) {
    _pending = PendingOperation{};
    finishSession(LDC1614::Status::Error(LDC1614::Err::CANCELLED,
                                         "CLI session cancelled"),
                  "CANCELLED");
    return;
  }

  if (_session.kind == SessionKind::SELF_TEST) {
    if (success) {
      selfTestPass("acquisition transport");
      const LDC1614::SampleBatch& batch = result.sampleBatch;
      if (batch.validChannels.bits == batch.selectedChannels.bits &&
          batch.errorChannels.empty()) {
        selfTestPass("sensor quality");
      } else {
        selfTestSkip("sensor quality",
                     "transport passed but fixture/sample flags are not acceptance evidence");
      }
    } else {
      selfTestFail("acquisition transport", result.status);
      selfTestSkip("sensor quality", "acquisition transport failed");
    }
    _pending = PendingOperation{};
    finishSession(_session.stats.failed == 0U
                      ? LDC1614::Status::Ok()
                      : _session.stats.lastFailure,
                  _session.stats.failed == 0U ? "SUCCESS" : "FAILED");
    return;
  }

  if (_session.kind == SessionKind::STRESS_MIX) {
    if (!success) _session.iterationFailed = true;
    ++_session.stats.completed;
    if (_session.iterationFailed) ++_session.stats.failed;
    else ++_session.stats.passed;
    ++_session.index;
    _session.iterationFailed = false;
    _session.phase = SessionPhase::MIX_STATUS;
    _pending = PendingOperation{};
    return;
  }

  if (_session.kind == SessionKind::SAMPLE_RATE) {
    LDC1614::Status acceptance = result.status;
    bool accepted = success;
    bool selected = false;
    bool valid = false;
    bool fresh = false;
    bool error = false;
    bool overrun = false;
    bool withinBounds = false;
    double frequency = 0.0;
    if (success) {
      const uint8_t bit = static_cast<uint8_t>(1U << _session.channel);
      const LDC1614::SampleBatch& batch = result.sampleBatch;
      selected = (batch.selectedChannels.bits & bit) != 0U;
      valid = (batch.validChannels.bits & bit) != 0U;
      fresh = (batch.freshChannels.bits & bit) != 0U;
      error = (batch.errorChannels.bits & bit) != 0U;
      overrun = (batch.overrunChannels.bits & bit) != 0U;
      const LDC1614::Status frequencyStatus =
          LDC1614::LDC1614::calculateSensorFrequencyHz(
              _device.config(), static_cast<LDC1614::Channel>(_session.channel),
              batch.channel[_session.channel].rawCount28, frequency);
      if (!frequencyStatus.ok()) {
        accepted = false;
        acceptance = frequencyStatus;
      } else {
        const LDC1614::ChannelConfig& channelConfig =
            _device.config().channel[_session.channel];
        withinBounds = frequency >= channelConfig.expectedSensorMinHz &&
                       frequency <= channelConfig.expectedSensorMaxHz;
      }
      if (accepted &&
          (!selected || !valid || !fresh || error || overrun || !withinBounds)) {
        accepted = false;
        acceptance = LDC1614::Status::Error(
            LDC1614::Err::CONVERSION_NOT_READY,
            withinBounds
                ? "Sample-rate batch was stale, invalid, or faulted"
                : "Sample-rate frequency outside configured bounds",
            static_cast<int32_t>((selected ? 0x01U : 0U) |
                                 (valid ? 0x02U : 0U) |
                                 (fresh ? 0x04U : 0U) |
                                 (error ? 0x08U : 0U) |
                                 (overrun ? 0x10U : 0U) |
                                 (withinBounds ? 0x20U : 0U)));
      }
    } else if (acceptance.ok()) {
      acceptance = LDC1614::Status::Error(
          LDC1614::Err::I2C_ERROR,
          "Sample-rate acquisition ended without a complete batch");
    }
    printf("samplerate_sample=%lu selected=%u valid=%u fresh=%u error=%u "
           "overrun=%u within_bounds=%u frequency_hz=%.6f code=%u\n",
           static_cast<unsigned long>(_session.index), selected ? 1U : 0U,
           valid ? 1U : 0U, fresh ? 1U : 0U, error ? 1U : 0U,
           overrun ? 1U : 0U, withinBounds ? 1U : 0U, frequency,
           static_cast<unsigned>(acceptance.code));
    ++_session.stats.completed;
    if (accepted) {
      ++_session.stats.passed;
    } else {
      ++_session.stats.failed;
      recordSessionFailure(acceptance);
    }
    ++_session.index;
    _pending = PendingOperation{};
    if (!accepted) {
      finishSession(acceptance, "FAILED");
      return;
    }
    _session.nextDueMs = nowMs();
    _session.phase = SessionPhase::WAIT_INTERVAL;
    return;
  }

  ++_session.stats.completed;
  if (success) ++_session.stats.passed;
  else ++_session.stats.failed;
  if (success && (_session.kind == SessionKind::WATCH || _verbose)) {
    printBatch(result.sampleBatch);
  }
  ++_session.index;
  _session.nextDueMs = nowMs() + _session.periodMs;
  _session.phase = SessionPhase::WAIT_INTERVAL;
  _pending = PendingOperation{};
}

void Cli::handleOperationResult(const LDC1614::OperationResult& result) {
  if (_pending.owner == OperationOwner::SESSION) {
    handleSessionOperationResult(result);
    return;
  }
  const CommandId command = _pending.command;
  _lastResult = result;
  _hasLastResult = true;
  if (result.hasSampleBatch) {
    _lastBatch = result.sampleBatch;
    _hasLastBatch = true;
  }
  printResult(result);
  printf("CLI result: command=%s session=0 outcome=%s code=%u\n",
         commandName(command), outcomeName(result.outcome),
         static_cast<unsigned>(result.status.code));
  _pending = PendingOperation{};
}

void Cli::drainResults() {
  uint8_t drained = 0;
  while (_device.resultAvailable() && drained < LDC1614::LDC1614::RESULT_CAPACITY) {
    LDC1614::OperationResult result;
    const LDC1614::Status takeStatus = _device.takeResult(result);
    if (!takeStatus.ok()) {
      logError("takeResult failed");
      printStatus(takeStatus);
      return;
    }
    ++drained;
    if (_pending.owner == OperationOwner::NONE || result.operationId != _pending.id ||
        result.kind != _pending.kind) {
      logError("unmatched terminal result operation=%" PRIu64
               " kind=%s expected_operation=%" PRIu64 " expected_kind=%s",
               result.operationId, jobName(result.kind), _pending.id,
               jobName(_pending.kind));
      printResult(result);
      continue;
    }
    handleOperationResult(result);
  }
  if (_pending.owner != OperationOwner::NONE &&
      !_device.jobProgress().active && !_device.resultAvailable()) {
    const LDC1614::Status lost = LDC1614::Status::Error(
        LDC1614::Err::RESULT_NOT_READY,
        "Correlated terminal result missing");
    const OperationOwner owner = _pending.owner;
    const CommandId command = _pending.command;
    _pending = PendingOperation{};
    if (owner == OperationOwner::SESSION && _session.kind != SessionKind::NONE) {
      ++_session.stats.failed;
      recordSessionFailure(lost);
      finishSession(lost, "FAILED");
    } else {
      logError("correlated terminal result missing");
      printf("CLI result: command=%s session=0 outcome=FAILED code=%u\n",
             commandName(command), static_cast<unsigned>(lost.code));
      printStatus(lost);
    }
  }
}

void Cli::stopActiveWork() {
  if (_session.kind != SessionKind::NONE) {
    _session.stopRequested = true;
    if (_pending.owner == OperationOwner::SESSION && _device.jobProgress().active) {
      const LDC1614::Status status = _device.cancelJob();
      if (!status.ok()) {
        recordSessionFailure(status);
        finishSession(status, "FAILED");
      }
    } else if (_pending.owner == OperationOwner::NONE) {
      finishSession(LDC1614::Status::Error(LDC1614::Err::CANCELLED,
                                           "CLI session cancelled"),
                    "CANCELLED");
    }
    return;
  }
  if (_pending.owner == OperationOwner::COMMAND) {
    printStatus(_device.cancelJob());
    return;
  }
  printStatus(LDC1614::Status::Ok());
}

PromptAction Cli::service() {
  _lastNowMs = nowMs();
  if (_device.jobProgress().active) {
    // A service pass that polls never also invokes a direct diagnostic
    // callback. Terminal results are cache-only to drain after the poll.
    (void)_device.poll(_lastNowMs, 1U);
    drainResults();
    if (_promptDeferred && !asynchronousWorkActive()) {
      _promptDeferred = false;
      return PromptAction::PRINT;
    }
    return PromptAction::NONE;
  }

  drainResults();
  if (_pending.owner == OperationOwner::NONE &&
      _session.kind != SessionKind::NONE) {
    advanceSession(_lastNowMs);
  }
  if (_promptDeferred && !asynchronousWorkActive()) {
    _promptDeferred = false;
    return PromptAction::PRINT;
  }
  return PromptAction::NONE;
}

PromptAction Cli::handleDump(const ParsedLine& line) {
  if (line.argc == 2U && std::strcmp(line.argv[1], "config") == 0) {
    const LDC1614::Status status =
        startSession(SessionKind::DUMP_CONFIG, CommandId::DUMP);
    if (!status.inProgress()) printStatus(status);
    return status.inProgress() ? PromptAction::NONE : promptActionForCurrentState();
  }
  if (line.argc == 3U && std::strcmp(line.argv[1], "all") == 0 &&
      requireConfirmation(line.argv[2])) {
    printf("%s[W]%s DATA and STATUS reads consume destructive evidence\n",
           cli_style::code(_colorEnabled, cli_style::Color::YELLOW),
           resetColor(_colorEnabled));
    const LDC1614::Status status =
        startSession(SessionKind::DUMP_ALL, CommandId::DUMP);
    if (!status.inProgress()) printStatus(status);
    return status.inProgress() ? PromptAction::NONE : promptActionForCurrentState();
  }
  return usage("dump config\ndump all confirm");
}

PromptAction Cli::handleCommand(CommandId id, const ParsedLine& line) {
  uint64_t value = 0;
  uint64_t second = 0;
  LDC1614::Status status = LDC1614::Status::Ok();

  if (id == CommandId::HELP) {
    if (rejectExtraArguments(line.argc, 1U)) return usage("help / ?");
    printHelp();
    printf("command_count=%u\n",
           static_cast<unsigned>(sizeof(COMMAND_SPECS) / sizeof(COMMAND_SPECS[0])));
    return promptActionForCurrentState();
  }
  if (id == CommandId::VERSION) {
    if (rejectExtraArguments(line.argc, 1U)) return usage("version / ver");
    printf("version=%s firmware_git=%s firmware_status=%s build_timestamp=%s\n",
           LDC1614::VERSION, LDC1614::GIT_COMMIT, LDC1614::GIT_STATUS,
           LDC1614::BUILD_TIMESTAMP);
    return promptActionForCurrentState();
  }
  if (id == CommandId::COLOR) {
    if (line.argc == 1U) {
      printf("color enabled=%u\n", _colorEnabled ? 1U : 0U);
      return promptActionForCurrentState();
    }
    bool enabled = false;
    if (line.argc != 2U || !parseOnOff(line.argv[1], enabled)) {
      return usage("color [on|off]");
    }
    _colorEnabled = enabled;
    printf("color enabled=%u\n", _colorEnabled ? 1U : 0U);
    return promptActionForCurrentState();
  }
  if (id == CommandId::VERBOSE) {
    if (line.argc == 1U) {
      printf("verbose enabled=%u\n", _verbose ? 1U : 0U);
      return promptActionForCurrentState();
    }
    if (line.argc != 2U ||
        (std::strcmp(line.argv[1], "0") != 0 &&
         std::strcmp(line.argv[1], "1") != 0)) {
      return usage("verbose [0|1]");
    }
    _verbose = std::strcmp(line.argv[1], "1") == 0;
    printf("verbose enabled=%u\n", _verbose ? 1U : 0U);
    return promptActionForCurrentState();
  }

  if (id == CommandId::BIND) {
    if (line.argc != 1U) return usage("bind");
    status = _device.bind(makeDefaultConfig());
    if (status.ok()) discardStagedProfile();
    printStatus(status);
    printf("CLI result: command=bind outcome=%s code=%u detail=%ld msg=%s\n",
           status.ok() ? "SUCCESS" : "FAILED",
           static_cast<unsigned>(status.code), static_cast<long>(status.detail),
           status.msg != nullptr ? status.msg : "");
    return promptActionForCurrentState();
  }
  if (id == CommandId::END) {
    if (line.argc != 1U) return usage("end");
    _device.end();
    _pending = PendingOperation{};
    _lastResult = LDC1614::OperationResult{};
    _lastBatch = LDC1614::SampleBatch{};
    _hasLastResult = false;
    _hasLastBatch = false;
    _stagedInitialized = false;
    _stagedDirty = false;
    println("CLI result: command=end outcome=SUCCESS code=0");
    return promptActionForCurrentState();
  }

  if (id == CommandId::INIT || id == CommandId::APPLY) {
    uint32_t deadlineMs = static_cast<uint32_t>(JOB_DEADLINE_MS);
    if (line.argc == 2U) {
      if (!parseUnsigned(line.argv[1], UINT32_MAX, value) || value == 0U) {
        return usage(id == CommandId::INIT ? "init [deadline_ms]"
                                           : "apply [deadline_ms]");
      }
      deadlineMs = static_cast<uint32_t>(value);
    } else if (line.argc != 1U) {
      return usage(id == CommandId::INIT ? "init [deadline_ms]"
                                         : "apply [deadline_ms]");
    }
    status = scheduleCommandJob(
        id, id == CommandId::INIT ? LDC1614::JobKind::INITIALIZE
                                  : LDC1614::JobKind::APPLY_CONFIG,
        LDC1614::ChannelMask{}, deadlineMs);
    if (!status.inProgress()) {
      printStatus(status);
      printf("CLI result: command=%s session=0 outcome=FAILED code=%u\n",
             commandName(id), static_cast<unsigned>(status.code));
    }
    return status.inProgress() ? PromptAction::NONE : promptActionForCurrentState();
  }

  if (id == CommandId::RESET_REAPPLY) {
    uint32_t deadlineMs = static_cast<uint32_t>(JOB_DEADLINE_MS);
    if (line.argc == 2U && requireConfirmation(line.argv[1])) {
      // Default deadline.
    } else if (line.argc == 3U &&
               parseUnsigned(line.argv[1], UINT32_MAX, value) && value != 0U &&
               requireConfirmation(line.argv[2])) {
      deadlineMs = static_cast<uint32_t>(value);
    } else {
      return usage("resetreapply [ms] confirm");
    }
    status = scheduleCommandJob(CommandId::RESET_REAPPLY,
                                LDC1614::JobKind::RESET_AND_REAPPLY,
                                LDC1614::ChannelMask{}, deadlineMs);
    if (!status.inProgress()) {
      printStatus(status);
      printf("CLI result: command=resetreapply session=0 outcome=FAILED code=%u\n",
             static_cast<unsigned>(status.code));
    }
    return status.inProgress() ? PromptAction::NONE : promptActionForCurrentState();
  }

  if (id == CommandId::SLEEP || id == CommandId::WAKE) {
    if (line.argc != 1U) return usage(id == CommandId::SLEEP ? "sleep" : "wake");
    status = id == CommandId::SLEEP ? _device.sleep() : _device.wake();
    printStatus(status);
    printf("CLI result: command=%s outcome=%s code=%u\n", commandName(id),
           status.ok() ? "SUCCESS" : "FAILED",
           static_cast<unsigned>(status.code));
    return promptActionForCurrentState();
  }

  if (id == CommandId::CANCEL) {
    if (line.argc != 1U) return usage("cancel / stop");
    stopActiveWork();
    if (asynchronousWorkActive()) {
      _promptDeferred = true;
      return PromptAction::NONE;
    }
    // A synchronous diagnostic session (for example, scan) can finish inside
    // stopActiveWork(). Its original deferred prompt is superseded by the
    // prompt returned for this cancel command.
    _promptDeferred = false;
    println("CLI result: command=cancel outcome=SUCCESS code=0 session=0");
    return PromptAction::PRINT;
  }
  if (id == CommandId::JOB) {
    if (line.argc != 1U) return usage("job / progress");
    printProgress();
    return promptActionForCurrentState();
  }
  if (id == CommandId::RESULT) {
    if (line.argc != 1U) return usage("result");
    if (!_hasLastResult) {
      printStatus(LDC1614::Status::Error(LDC1614::Err::RESULT_NOT_READY,
                                         "No cached terminal result"));
    } else {
      printResult(_lastResult);
    }
    return promptActionForCurrentState();
  }
  if (id == CommandId::INVALIDATE) {
    if (line.argc != 2U || !requireConfirmation(line.argv[1])) {
      return usage("invalidate confirm");
    }
    _device.invalidateAppliedState(LDC1614::Status::Error(
        LDC1614::Err::CONFIG_DIRTY, "Owner invalidated applied state"));
    println("CLI result: command=invalidate outcome=SUCCESS code=0");
    printState();
    return promptActionForCurrentState();
  }
  if (id == CommandId::BUS_RECOVER) {
    if (line.argc != 2U || !requireConfirmation(line.argv[1])) {
      return usage("busrecover confirm");
    }
    status = _platform.i2cRecover != nullptr
                 ? _platform.i2cRecover(_platform.user)
                 : LDC1614::Status::Error(LDC1614::Err::INVALID_CONFIG,
                                          "Bus recovery callback unavailable");
    _device.invalidateAppliedState(
        status.ok() ? LDC1614::Status::Error(LDC1614::Err::I2C_BUS,
                                             "Owner reinitialized I2C bus")
                    : status);
    printStatus(status);
    printf("CLI result: command=busrecover outcome=%s code=%u\n",
           status.ok() ? "SUCCESS" : "FAILED",
           static_cast<unsigned>(status.code));
    println("Run 'init' before trusted device use.");
    return promptActionForCurrentState();
  }

  if (id == CommandId::READ) {
    uint64_t mask = _device.isBound() ? _device.config().channels.bits : 0U;
    if (line.argc == 2U) {
      if (!parseUnsigned(line.argv[1], 0x0FU, mask) || mask == 0U) {
        return usage("read / acquire [mask]");
      }
    } else if (line.argc != 1U) {
      return usage("read / acquire [mask]");
    }
    status = scheduleCommandJob(CommandId::READ, LDC1614::JobKind::ACQUIRE,
                                LDC1614::ChannelMask{static_cast<uint8_t>(mask)},
                                static_cast<uint32_t>(JOB_DEADLINE_MS));
    if (!status.inProgress()) {
      printStatus(status);
      printf("CLI result: command=read session=0 mask=0x%02lX outcome=FAILED code=%u\n",
             static_cast<unsigned long>(mask), static_cast<unsigned>(status.code));
    }
    return status.inProgress() ? PromptAction::NONE : promptActionForCurrentState();
  }
  if (id == CommandId::LAST) {
    if (line.argc != 1U && line.argc != 2U) return usage("last [channel]");
    if (!_hasLastBatch) {
      printStatus(LDC1614::Status::Error(LDC1614::Err::RESULT_NOT_READY,
                                         "No cached sample batch"));
      return promptActionForCurrentState();
    }
    if (line.argc == 1U) {
      printBatch(_lastBatch);
      return promptActionForCurrentState();
    }
    if (!parseUnsigned(line.argv[1], 3U, value) ||
        (_device.isBound() && _device.config().variant ==
                                  LDC1614::DeviceVariant::LDC1612 &&
         value >= 2U)) {
      return usage("last [channel]");
    }
    const uint8_t bit = static_cast<uint8_t>(1U << value);
    if ((_lastBatch.selectedChannels.bits & bit) == 0U) {
      printStatus(LDC1614::Status::Error(LDC1614::Err::INVALID_PARAM,
                                         "Channel absent from cached batch"));
    } else {
      LDC1614::SampleBatch one = _lastBatch;
      one.selectedChannels.bits &= bit;
      one.validChannels.bits &= bit;
      one.freshChannels.bits &= bit;
      one.errorChannels.bits &= bit;
      one.overrunChannels.bits &= bit;
      printBatch(one);
    }
    return promptActionForCurrentState();
  }

  if (id == CommandId::WATCH) {
    uint64_t mask = 0;
    uint64_t count = 0;
    uint64_t period = 0;
    if ((line.argc != 3U && line.argc != 4U) ||
        !parseUnsigned(line.argv[1], 0x0FU, mask) || mask == 0U ||
        !parseUnsigned(line.argv[2], MAX_SESSION_COUNT, count) || count == 0U ||
        (line.argc == 4U && !parseUnsigned(line.argv[3], UINT32_MAX, period))) {
      return usage("watch <mask> <count> [ms]");
    }
    status = startSession(SessionKind::WATCH, CommandId::WATCH,
                          static_cast<uint32_t>(count),
                          LDC1614::ChannelMask{static_cast<uint8_t>(mask)},
                          static_cast<uint32_t>(period));
    if (!status.inProgress()) printStatus(status);
    return status.inProgress() ? PromptAction::NONE : promptActionForCurrentState();
  }
  if (id == CommandId::SAMPLE_RATE) {
    uint64_t count = 0;
    if (line.argc != 3U || !parseUnsigned(line.argv[1], 3U, value) ||
        !_device.isBound() ||
        (_device.config().variant == LDC1614::DeviceVariant::LDC1612 && value >= 2U) ||
        !parseUnsigned(line.argv[2], MAX_SESSION_COUNT, count) || count == 0U) {
      return usage("samplerate <channel> <count>");
    }
    status = startSession(SessionKind::SAMPLE_RATE, CommandId::SAMPLE_RATE,
                          static_cast<uint32_t>(count),
                          LDC1614::channelBit(static_cast<LDC1614::Channel>(value)),
                          0U, static_cast<uint8_t>(value));
    if (!status.inProgress()) printStatus(status);
    return status.inProgress() ? PromptAction::NONE : promptActionForCurrentState();
  }

  if (id == CommandId::READY) {
    if (line.argc != 1U) return usage("ready / drdy");
    bool ready = false;
    LDC1614::DeviceStatus observed;
    status = _device.readDataReady(ready, observed);
    printf("ready=%u code=%u\n", ready ? 1U : 0U,
           static_cast<unsigned>(status.code));
    if (status.ok()) printDeviceStatus(observed, "ready_status");
    printStatus(status);
    return promptActionForCurrentState();
  }
  if (id == CommandId::STATUS || id == CommandId::STATUS_RAW) {
    if (line.argc != 1U) return usage(id == CommandId::STATUS ? "status" : "status_raw");
    LDC1614::DeviceStatus deviceStatus;
    status = _device.readDeviceStatus(deviceStatus);
    if (id == CommandId::STATUS_RAW) {
      printf("status_raw=0x%04X code=%u\n", deviceStatus.raw,
             static_cast<unsigned>(status.code));
    } else if (status.ok()) {
      printDeviceStatus(deviceStatus, "STATUS");
    }
    printStatus(status);
    return promptActionForCurrentState();
  }
  if (id == CommandId::INTB) {
    if (line.argc != 1U) return usage("intb");
    bool asserted = false;
    status = _device.readIntb(asserted);
    printf("intb asserted=%u code=%u\n", asserted ? 1U : 0U,
           static_cast<unsigned>(status.code));
    printStatus(status);
    return promptActionForCurrentState();
  }
  if (id == CommandId::INIT_DRIVE) {
    if (line.argc != 2U || !parseUnsigned(line.argv[1], 3U, value) ||
        (_device.isBound() && _device.config().variant ==
                                  LDC1614::DeviceVariant::LDC1612 &&
         value >= 2U)) {
      return usage("initdrive <channel>");
    }
    uint8_t code = 0;
    status = _device.readInitDriveCurrent(static_cast<LDC1614::Channel>(value), code);
    uint16_t microamps = 0;
    const LDC1614::Status currentStatus =
        LDC1614::LDC1614::nominalDriveCurrentMicroamps(
            LDC1614::DriveCurrentCode(code), microamps);
    printf("channel=%lu init_drive_code=%u microamps=%s%u code=%u\n",
           static_cast<unsigned long>(value), static_cast<unsigned>(code),
           currentStatus.ok() ? "" : "unavailable:", static_cast<unsigned>(microamps),
           static_cast<unsigned>(status.code));
    printStatus(status);
    return promptActionForCurrentState();
  }

  if (id == CommandId::CFG) {
    if (line.argc != 1U) return usage("cfg / settings");
    if (_device.isBound()) printConfig(_device.config(), "desired");
    else println("Configuration: desired unavailable (driver not bound)");
    ensureStagedProfile();
    printConfig(_stagedConfig, "staged");
    return promptActionForCurrentState();
  }
  if (id == CommandId::PROFILE) return handleProfile(line);
  if (id == CommandId::ADDR) {
    if (line.argc != 1U) return usage("addr");
    ensureStagedProfile();
    printf("address=0x%02X build_profile_only=1\n",
           static_cast<unsigned>(_stagedConfig.i2cAddress));
    println("Change BoardConfig/transport identity, then end, reopen the device, and bind.");
    return promptActionForCurrentState();
  }
  if (id == CommandId::VARIANT) {
    if (line.argc != 1U) return usage("variant");
    ensureStagedProfile();
    printf("variant=%s variant_channels=%u build_profile_only=1\n",
           variantName(_stagedConfig.variant),
           static_cast<unsigned>(physicalChannelCount(_stagedConfig.variant)));
    println("Change BoardConfig and the complete channel profile, then end/rebuild/rebind.");
    return promptActionForCurrentState();
  }
  if (id == CommandId::MODE || id == CommandId::REFCLK ||
      id == CommandId::DEGLITCH || id == CommandId::ACTIVATION ||
      id == CommandId::TIMEOUT || id == CommandId::RP ||
      id == CommandId::AUTOAMP || id == CommandId::HIGH_CURRENT ||
      id == CommandId::INTB_CONFIG || id == CommandId::ERRORS ||
      id == CommandId::ERROR || id == CommandId::RCOUNT ||
      id == CommandId::SETTLE || id == CommandId::FIN_DIV ||
      id == CommandId::FREF_DIV || id == CommandId::OFFSET ||
      id == CommandId::DRIVE || id == CommandId::SENSOR_BOUNDS) {
    return handleSetting(id, line);
  }

  if (id == CommandId::PROBE) {
    if (line.argc != 1U) return usage("probe / id");
    status = startSession(SessionKind::PROBE, CommandId::PROBE);
    if (!status.inProgress()) printStatus(status);
    return status.inProgress() ? PromptAction::NONE : promptActionForCurrentState();
  }
  if (id == CommandId::SCAN) {
    if (line.argc != 1U) return usage("scan");
    status = startSession(SessionKind::SCAN, CommandId::SCAN);
    if (!status.inProgress()) printStatus(status);
    return status.inProgress() ? PromptAction::NONE : promptActionForCurrentState();
  }
  if (id == CommandId::DUMP) return handleDump(line);
  if (id == CommandId::VERIFY) {
    if (line.argc != 1U) return usage("verify");
    status = startSession(SessionKind::VERIFY, CommandId::VERIFY);
    if (!status.inProgress()) printStatus(status);
    return status.inProgress() ? PromptAction::NONE : promptActionForCurrentState();
  }
  if (id == CommandId::REG) {
    if ((line.argc != 2U && line.argc != 3U) ||
        !parseUnsigned(line.argv[1], 0x7FU, value)) {
      return usage("reg / rreg <addr> [confirm]");
    }
    const bool destructive = isDestructiveRegister(static_cast<uint8_t>(value));
    if ((destructive && (line.argc != 3U || !requireConfirmation(line.argv[2]))) ||
        (!destructive && line.argc == 3U &&
         !requireConfirmation(line.argv[2]))) {
      return usage("reg / rreg <addr> [confirm]");
    }
    uint16_t registerValue = 0;
    status = _device.readRegister16(static_cast<uint8_t>(value), registerValue);
    printf("register=0x%02lX value=0x%04X code=%u\n",
           static_cast<unsigned long>(value), registerValue,
           static_cast<unsigned>(status.code));
    printStatus(status);
    return promptActionForCurrentState();
  }
  if (id == CommandId::WREG) {
    if (line.argc != 4U || !parseUnsigned(line.argv[1], 0x7FU, value) ||
        !parseUnsigned(line.argv[2], UINT16_MAX, second) ||
        !requireConfirmation(line.argv[3])) {
      return usage("wreg <addr> <value> confirm");
    }
    status = _device.writeRegister16(static_cast<uint8_t>(value),
                                     static_cast<uint16_t>(second));
    printf("register=0x%02lX value=0x%04lX code=%u\n",
           static_cast<unsigned long>(value), static_cast<unsigned long>(second),
           static_cast<unsigned>(status.code));
    printStatus(status);
    return promptActionForCurrentState();
  }
  if (id == CommandId::DECODE) return handleDecode(line);
  if (id == CommandId::FREQ) {
    uint64_t raw = 0;
    if (line.argc != 3U || !parseUnsigned(line.argv[1], 3U, value) ||
        (_device.isBound() && _device.config().variant ==
                                  LDC1614::DeviceVariant::LDC1612 && value >= 2U) ||
        !parseUnsigned(line.argv[2], 0x0FFFFFFFU, raw)) {
      return usage("freq <channel> <raw28>");
    }
    ensureStagedProfile();
    double frequency = 0.0;
    status = LDC1614::LDC1614::calculateSensorFrequencyHz(
        _device.isBound() ? _device.config() : _stagedConfig,
        static_cast<LDC1614::Channel>(value), static_cast<uint32_t>(raw),
        frequency);
    printf("channel=%lu raw=0x%07lX frequency_hz=%.6f code=%u\n",
           static_cast<unsigned long>(value), static_cast<unsigned long>(raw),
           frequency, static_cast<unsigned>(status.code));
    printStatus(status);
    return promptActionForCurrentState();
  }
  if (id == CommandId::TIMING) {
    uint64_t mask = _device.isBound() ? _device.config().channels.bits : 0U;
    if ((line.argc != 1U && line.argc != 2U) ||
        (line.argc == 2U &&
         (!parseUnsigned(line.argv[1], 0x0FU, mask) || mask == 0U))) {
      return usage("timing [mask]");
    }
    ensureStagedProfile();
    LDC1614::FrameTiming timing;
    status = LDC1614::LDC1614::estimateFrameTiming(
        _device.isBound() ? _device.config() : _stagedConfig,
        LDC1614::ChannelMask{static_cast<uint8_t>(mask)}, timing);
    printf("mask=0x%02lX wake_settle_us=%" PRIu64 " conversion_us=%" PRIu64
           " sequential_frame_us=%" PRIu64 " acquisition_transfers=%u code=%u\n",
           static_cast<unsigned long>(mask), timing.wakeAndSettleUs,
           timing.conversionUs, timing.sequentialFrameUs,
           static_cast<unsigned>(timing.acquisitionTransfers),
           static_cast<unsigned>(status.code));
    printStatus(status);
    return promptActionForCurrentState();
  }
  if (id == CommandId::DRIVE_UA) {
    if (line.argc != 2U || !parseUnsigned(line.argv[1], 31U, value)) {
      return usage("driveua <code>");
    }
    uint16_t microamps = 0;
    status = LDC1614::LDC1614::nominalDriveCurrentMicroamps(
        LDC1614::DriveCurrentCode(static_cast<uint8_t>(value)), microamps);
    printf("code=%lu microamps=%u\n", static_cast<unsigned long>(value),
           static_cast<unsigned>(microamps));
    printStatus(status);
    return promptActionForCurrentState();
  }

  if (id == CommandId::DRIVER) {
    if (line.argc != 1U) return usage("drv / health");
    printDriver();
    return promptActionForCurrentState();
  }
  if (id == CommandId::STATE) {
    if (line.argc != 1U) return usage("state");
    printState();
    return promptActionForCurrentState();
  }
  if (id == CommandId::SELFTEST) {
    if (line.argc != 1U) return usage("selftest");
    status = startSession(SessionKind::SELF_TEST, CommandId::SELFTEST);
    if (!status.inProgress()) printStatus(status);
    return status.inProgress() ? PromptAction::NONE : promptActionForCurrentState();
  }
  if (id == CommandId::STRESS) {
    uint64_t count = 0;
    uint64_t mask = _device.isBound() ? _device.config().channels.bits : 0U;
    if ((line.argc != 2U && line.argc != 3U) ||
        !parseUnsigned(line.argv[1], MAX_SESSION_COUNT, count) || count == 0U ||
        (line.argc == 3U &&
         (!parseUnsigned(line.argv[2], 0x0FU, mask) || mask == 0U))) {
      return usage("stress <count> [mask]");
    }
    status = startSession(SessionKind::STRESS, CommandId::STRESS,
                          static_cast<uint32_t>(count),
                          LDC1614::ChannelMask{static_cast<uint8_t>(mask)});
    if (!status.inProgress()) printStatus(status);
    return status.inProgress() ? PromptAction::NONE : promptActionForCurrentState();
  }
  if (id == CommandId::STRESS_MIX) {
    uint64_t count = 0;
    uint64_t mask = _device.isBound() ? _device.config().channels.bits : 0U;
    bool valid = false;
    if (line.argc == 3U && requireConfirmation(line.argv[2])) {
      valid = parseUnsigned(line.argv[1], MAX_SESSION_COUNT, count) && count != 0U;
    } else if (line.argc == 4U && requireConfirmation(line.argv[3])) {
      valid = parseUnsigned(line.argv[1], MAX_SESSION_COUNT, count) && count != 0U &&
              parseUnsigned(line.argv[2], 0x0FU, mask) && mask != 0U;
    }
    if (!valid) return usage("stress_mix <n> [mask] confirm");
    status = startSession(SessionKind::STRESS_MIX, CommandId::STRESS_MIX,
                          static_cast<uint32_t>(count),
                          LDC1614::ChannelMask{static_cast<uint8_t>(mask)});
    if (!status.inProgress()) printStatus(status);
    return status.inProgress() ? PromptAction::NONE : promptActionForCurrentState();
  }
  if (id == CommandId::SOAK) {
    uint64_t seconds = 0;
    uint64_t mask = _device.isBound() ? _device.config().channels.bits : 0U;
    if ((line.argc != 2U && line.argc != 3U) ||
        !parseUnsigned(line.argv[1], 86400U, seconds) || seconds == 0U ||
        (line.argc == 3U &&
         (!parseUnsigned(line.argv[2], 0x0FU, mask) || mask == 0U))) {
      return usage("soak <seconds> [mask]");
    }
    status = startSession(SessionKind::SOAK, CommandId::SOAK,
                          static_cast<uint32_t>(seconds),
                          LDC1614::ChannelMask{static_cast<uint8_t>(mask)}, 1000U);
    if (!status.inProgress()) printStatus(status);
    return status.inProgress() ? PromptAction::NONE : promptActionForCurrentState();
  }
  if (id == CommandId::SD) return handleSd(line);

  logError("internal command dispatch error");
  return promptActionForCurrentState();
}

PromptAction Cli::processCommand(const char* commandLine) {
  ParsedLine line;
  if (!parseLine(commandLine, line)) return promptActionForCurrentState();
  const CommandId id = resolveCommand(line.argv[0]);
  if (id == CommandId::UNKNOWN) {
    logError("unknown command '%s'; type help", line.argv[0]);
    return promptActionForCurrentState();
  }
  if (asynchronousWorkActive() && !commandAllowedWhileBusy(id)) {
    printStatus(LDC1614::Status::Error(LDC1614::Err::BUSY,
                                       "CLI work active; use job or cancel"));
    return PromptAction::NONE;
  }
  return handleCommand(id, line);
}

static_assert(sizeof(Cli) <= 2048U,
              "Diagnostic CLI fixed state must remain at most 2 KiB");

}  // namespace ldc1614_cli
