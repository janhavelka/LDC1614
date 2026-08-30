/// @file Config.h
/// @brief Explicit transport and sensor-profile configuration.
#pragma once

#include <cstddef>
#include <cstdint>

#include "LDC1614/Status.h"

namespace LDC1614 {

/// @brief Caller-owned identifier used to correlate jobs and terminal results.
using OperationId = uint64_t;

/// One bounded I2C write attempt. The callback must return within timeoutMs.
using I2cWriteFn = Status (*)(uint8_t address, const uint8_t* data, size_t length,
                              uint32_t timeoutMs, void* user);

/// One bounded combined-format I2C write/read attempt: the register-pointer
/// write and the two-byte read are joined by a repeated START with no STOP
/// between them (datasheet 7.5.1, Figure 12). A write terminated by STOP
/// followed by a separate read does not return coherent DATA. The callback
/// must return within timeoutMs.
using I2cWriteReadFn = Status (*)(uint8_t address,
                                  const uint8_t* txData, size_t txLength,
                                  uint8_t* rxData, size_t rxLength,
                                  uint32_t timeoutMs, void* user);

/// Optional application-owned, bus-silent INTB observation. The callback must
/// be non-blocking; no pin identity is retained by the driver.
using IntbAssertedFn = Status (*)(bool& asserted, void* user);

/// @brief Supported 7-bit LDC1612/LDC1614 I2C addresses.
enum class I2cAddress : uint8_t {
  UNSPECIFIED = 0,  ///< Invalid sentinel used by a default profile.
  ADDR_GND = 0x2A, ///< ADDR pin tied to ground.
  ADDR_VDD = 0x2B, ///< ADDR pin tied to VDD.
};

/// @brief Application-supplied silicon variant; identity registers are shared.
enum class DeviceVariant : uint8_t {
  UNSPECIFIED = 0, ///< Invalid sentinel used by a default profile.
  LDC1612,        ///< Two-channel device; channels 0 and 1 exist.
  LDC1614,        ///< Four-channel device; channels 0 through 3 exist.
};

/// @brief Physical conversion channel.
enum class Channel : uint8_t {
  CH0 = 0,    ///< Channel 0.
  CH1 = 1,    ///< Channel 1.
  CH2 = 2,    ///< Channel 2; LDC1614 only.
  CH3 = 3,    ///< Channel 3; LDC1614 only.
  NONE = 0xFF, ///< Invalid/no-channel sentinel.
};

/// @brief Strong channel-mask type. Only bits 0..3 are valid.
struct ChannelMask {
  uint8_t bits = 0; ///< One bit per physical channel; bits 4..7 must be zero.

  /// @return True when no channel bit is set.
  constexpr bool empty() const { return bits == 0; }

  /// @param channel Channel to test.
  /// @return True when `channel` is valid and selected.
  constexpr bool contains(Channel channel) const {
    const uint8_t index = static_cast<uint8_t>(channel);
    return index < 4U &&
           (bits & static_cast<uint8_t>(1U << index)) != 0U;
  }
};

/// @param channel Channel to encode.
/// @return One-hot channel mask, or an empty mask for `Channel::NONE`.
constexpr ChannelMask channelBit(Channel channel) {
  const uint8_t index = static_cast<uint8_t>(channel);
  return index < 4U
             ? ChannelMask{static_cast<uint8_t>(1U << index)}
             : ChannelMask{};
}

/// @param left First channel mask.
/// @param right Second channel mask.
/// @return Union of the two masks.
constexpr ChannelMask operator|(ChannelMask left, ChannelMask right) {
  return ChannelMask{static_cast<uint8_t>(left.bits | right.bits)};
}

/// @param variant Explicit device variant.
/// @return Mask of physical channels implemented by `variant`.
constexpr ChannelMask validChannelMask(DeviceVariant variant) {
  return variant == DeviceVariant::LDC1612
             ? ChannelMask{0x03}
             : (variant == DeviceVariant::LDC1614 ? ChannelMask{0x0F}
                                                  : ChannelMask{});
}

/// @brief Reference-clock source used by CONFIG.REF_CLK_SRC.
enum class RefClkSrc : uint8_t {
  UNSPECIFIED = 0, ///< Invalid sentinel used by a default profile.
  INTERNAL,        ///< Device internal oscillator.
  EXTERNAL_CLOCK,  ///< Application-supplied CLKIN source.
};

/// @brief Application-supplied reference-clock fact used for validation and calculations.
/// External-clock tolerance must remain inside the datasheet input range. The
/// internal-oscillator interval is conservatively clipped to its guaranteed
/// datasheet range.
struct ReferenceClock {
  RefClkSrc source = RefClkSrc::UNSPECIFIED; ///< Selected clock source.
  uint32_t frequencyHz = 0;                  ///< Nominal clock frequency in hertz.
  uint32_t tolerancePpm = 0;                 ///< Symmetric tolerance in parts per million.
};

/// @brief Sensor-activation current policy.
enum class SensorActivation : uint8_t {
  FULL_CURRENT = 0, ///< Use full-current sensor activation.
  LOW_POWER = 1,   ///< Use low-power sensor activation.
};

/// @brief Input deglitch-filter bandwidth encoding.
enum class Deglitch : uint8_t {
  UNSPECIFIED = 0, ///< Invalid sentinel used by a default profile.
  BW_1MHZ = 1,    ///< 1.0 MHz bandwidth.
  BW_3MHZ = 4,    ///< 3.3 MHz bandwidth.
  BW_10MHZ = 5,   ///< 10 MHz bandwidth.
  BW_33MHZ = 7,   ///< 33 MHz bandwidth.
};

/// @brief Device conversion mode.
enum class OperatingMode : uint8_t {
  UNSPECIFIED = 0,        ///< Invalid sentinel used by a default profile.
  SINGLE_CHANNEL,         ///< Convert one active channel continuously.
  MULTI_CHANNEL_SEQUENTIAL, ///< Auto-scan a round-robin channel sequence.
};

/// @brief Hardware round-robin sequence for sequential conversion.
enum class RRSequence : uint8_t {
  CH0_CH1 = 0,          ///< Scan channels 0 and 1.
  CH0_CH1_CH2 = 1,      ///< Scan channels 0..2; LDC1614 only.
  CH0_CH1_CH2_CH3 = 2,  ///< Scan channels 0..3; LDC1614 only.
  UNSPECIFIED = 0xFF,   ///< Invalid sentinel used by a default profile.
};

/// @brief Typed ERROR_CONFIG policy. All fields are encoded by the driver.
struct ErrorReporting {
  bool dataUnderRange = false;     ///< Route under-range to DATAx_MSB.
  bool dataOverRange = false;      ///< Route over-range to DATAx_MSB.
  bool dataWatchdog = false;       ///< Route watchdog timeout to DATAx_MSB.
  bool dataAmplitudeHigh = false;  ///< Route high-amplitude error to DATAx_MSB.
  bool dataAmplitudeLow = false;   ///< Route low-amplitude error to DATAx_MSB.
  bool statusUnderRange = false;   ///< Route under-range to STATUS/INTB.
  bool statusOverRange = false;    ///< Route over-range to STATUS/INTB.
  bool statusWatchdog = false;     ///< Route watchdog timeout to STATUS/INTB.
  bool statusAmplitudeHigh = false; ///< Route high-amplitude error to STATUS/INTB.
  bool statusAmplitudeLow = false; ///< Route low-amplitude error to STATUS/INTB.
  bool statusZeroCount = false;    ///< Route zero-count error to STATUS/INTB.
  bool dataReady = false;          ///< Route data-ready to STATUS/INTB;
                                   ///< false leaves STATUS.DRDY permanently clear.

  /// @return Policy with every supported error/data-ready route enabled.
  static constexpr ErrorReporting all() {
    return ErrorReporting{true, true, true, true, true, true,
                          true, true, true, true, true, true};
  }
};

/// @brief Typed five-bit IDRIVE code. 0xFF is the invalid/unspecified sentinel.
struct DriveCurrentCode {
  uint8_t value = 0xFF; ///< Five-bit IDRIVE code or 0xFF sentinel.

  /// @brief Construct the invalid/unspecified sentinel.
  constexpr DriveCurrentCode() = default;
  /// @param code Raw five-bit IDRIVE code; validation rejects values above 31.
  constexpr DriveCurrentCode(uint8_t code) : value(code) {}
  /// @return Raw stored IDRIVE code.
  constexpr operator uint8_t() const { return value; }
};

/// @brief Explicit per-channel electrical and conversion profile.
struct ChannelConfig {
  uint16_t rcount = 0;              ///< 0 is unspecified; valid configured range starts at 0x0005.
  uint16_t settleCount = 0;         ///< Reference-clock settling count.
  uint8_t finDivider = 0;           ///< 0 is unspecified; valid range is 1..15.
  uint16_t frefDivider = 0;         ///< 0 is unspecified; valid range is 1..1023.
  uint16_t offset = 0;              ///< Offset including FIN divider must remain below sensor minimum.
  DriveCurrentCode driveCurrentCode{};  ///< Explicit code 0..31.
  uint32_t expectedSensorMinHz = 0; ///< Expected minimum sensor frequency in hertz.
  uint32_t expectedSensorMaxHz = 0; ///< Expected maximum sensor frequency in hertz.
};

/// @brief Complete desired device configuration and non-owning transport injection.
///
/// A default-constructed Config is deliberately invalid. The application must
/// explicitly select the part, address, reference clock, mode, and channel
/// mask; supply every per-channel register field for each physical channel of
/// the selected variant; and supply expected sensor-frequency bounds only for
/// channels selected in the channel mask before bind().
struct Config {
  I2cWriteFn i2cWrite = nullptr;         ///< Non-owning single-write callback.
  I2cWriteReadFn i2cWriteRead = nullptr; ///< Non-owning combined write/read callback.
  void* i2cUser = nullptr;               ///< Opaque transport context passed unchanged.
  uint32_t i2cTimeoutMs = 0;             ///< Per-callback timeout cap in milliseconds.
  I2cAddress i2cAddress = I2cAddress::UNSPECIFIED; ///< Explicit 7-bit address.

  IntbAssertedFn intbAsserted = nullptr; ///< Optional bus-silent INTB observer.
  void* intbUser = nullptr;              ///< Opaque INTB callback context.

  DeviceVariant variant = DeviceVariant::UNSPECIFIED; ///< Explicit silicon variant.
  ChannelMask channels{};                            ///< Channels selected for conversion.
  ChannelConfig channel[4]{};                        ///< Register profile for every physical channel.
  ReferenceClock referenceClock{};                   ///< Clock fact used by validation/calculation.

  OperatingMode mode = OperatingMode::UNSPECIFIED; ///< Single or sequential mode.
  Channel activeChannel = Channel::NONE;           ///< Active channel in single-channel mode.
  RRSequence rrSequence = RRSequence::UNSPECIFIED; ///< Sequence in multi-channel mode.
  Deglitch deglitch = Deglitch::UNSPECIFIED;       ///< Input deglitch bandwidth.
  SensorActivation sensorActivation = SensorActivation::LOW_POWER; ///< Activation-current mode.
  bool rpOverrideEnabled = true;                    ///< CONFIG.RP_OVERRIDE_EN value.
  bool autoAmplitudeCorrectionEnabled = false;     ///< Enable automatic amplitude correction.
  bool highCurrentDriveEnabled = false;             ///< Enable channel-0 high-current drive.
  bool intbDisabled = true;                         ///< Disable the INTB output when true.
  ErrorReporting errorReporting{};                  ///< Typed ERROR_CONFIG policy.
};

}  // namespace LDC1614
