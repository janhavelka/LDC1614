/// @file Config.h
/// @brief Explicit transport and sensor-profile configuration.
#pragma once

#include <cstddef>
#include <cstdint>

#include "LDC1614/Status.h"

namespace LDC1614 {

using OperationId = uint64_t;

/// One bounded I2C write attempt. The callback must return within timeoutMs.
using I2cWriteFn = Status (*)(uint8_t address, const uint8_t* data, size_t length,
                              uint32_t timeoutMs, void* user);

/// One bounded combined I2C write/read attempt. The callback must return within timeoutMs.
using I2cWriteReadFn = Status (*)(uint8_t address,
                                  const uint8_t* txData, size_t txLength,
                                  uint8_t* rxData, size_t rxLength,
                                  uint32_t timeoutMs, void* user);

/// Optional application-owned, bus-silent INTB observation. The callback must
/// be non-blocking; no pin identity is retained by the driver.
using IntbAssertedFn = Status (*)(bool& asserted, void* user);

enum class I2cAddress : uint8_t {
  UNSPECIFIED = 0,
  ADDR_GND = 0x2A,
  ADDR_VDD = 0x2B,
};

enum class DeviceVariant : uint8_t {
  UNSPECIFIED = 0,
  LDC1612,
  LDC1614,
};

enum class Channel : uint8_t {
  CH0 = 0,
  CH1 = 1,
  CH2 = 2,
  CH3 = 3,
  NONE = 0xFF,
};

/// @brief Strong channel-mask type. Only bits 0..3 are valid.
struct ChannelMask {
  uint8_t bits = 0;

  constexpr bool empty() const { return bits == 0; }
  constexpr bool contains(Channel channel) const {
    const uint8_t index = static_cast<uint8_t>(channel);
    return index < 4U &&
           (bits & static_cast<uint8_t>(1U << index)) != 0U;
  }
};

constexpr ChannelMask channelBit(Channel channel) {
  const uint8_t index = static_cast<uint8_t>(channel);
  return index < 4U
             ? ChannelMask{static_cast<uint8_t>(1U << index)}
             : ChannelMask{};
}

constexpr ChannelMask operator|(ChannelMask left, ChannelMask right) {
  return ChannelMask{static_cast<uint8_t>(left.bits | right.bits)};
}

constexpr ChannelMask validChannelMask(DeviceVariant variant) {
  return variant == DeviceVariant::LDC1612
             ? ChannelMask{0x03}
             : (variant == DeviceVariant::LDC1614 ? ChannelMask{0x0F}
                                                  : ChannelMask{});
}

enum class RefClkSrc : uint8_t {
  UNSPECIFIED = 0,
  INTERNAL,
  EXTERNAL_CLOCK,
};

/// @brief Application-supplied reference-clock fact used for validation and calculations.
/// External-clock tolerance must remain inside the datasheet input range. The
/// internal-oscillator interval is conservatively clipped to its guaranteed
/// datasheet range.
struct ReferenceClock {
  RefClkSrc source = RefClkSrc::UNSPECIFIED;
  uint32_t frequencyHz = 0;
  uint32_t tolerancePpm = 0;
};

enum class SensorActivation : uint8_t {
  FULL_CURRENT = 0,
  LOW_POWER = 1,
};

enum class Deglitch : uint8_t {
  UNSPECIFIED = 0,
  BW_1MHZ = 1,
  BW_3MHZ = 4,
  BW_10MHZ = 5,
  BW_33MHZ = 7,
};

enum class OperatingMode : uint8_t {
  UNSPECIFIED = 0,
  SINGLE_CHANNEL,
  MULTI_CHANNEL_SEQUENTIAL,
};

enum class RRSequence : uint8_t {
  CH0_CH1 = 0,
  CH0_CH1_CH2 = 1,
  CH0_CH1_CH2_CH3 = 2,
  UNSPECIFIED = 0xFF,
};

/// @brief Typed ERROR_CONFIG policy. All fields are encoded by the driver.
struct ErrorReporting {
  bool dataUnderRange = false;
  bool dataOverRange = false;
  bool dataWatchdog = false;
  bool dataAmplitudeHigh = false;
  bool dataAmplitudeLow = false;
  bool statusUnderRange = false;
  bool statusOverRange = false;
  bool statusWatchdog = false;
  bool statusAmplitudeHigh = false;
  bool statusAmplitudeLow = false;
  bool statusZeroCount = false;
  bool dataReady = false;

  static constexpr ErrorReporting all() {
    return ErrorReporting{true, true, true, true, true, true,
                          true, true, true, true, true, true};
  }
};

/// @brief Typed five-bit IDRIVE code. 0xFF is the invalid/unspecified sentinel.
struct DriveCurrentCode {
  uint8_t value = 0xFF;

  constexpr DriveCurrentCode() = default;
  constexpr DriveCurrentCode(uint8_t code) : value(code) {}
  constexpr operator uint8_t() const { return value; }
};

/// @brief Explicit per-channel electrical and conversion profile.
struct ChannelConfig {
  uint16_t rcount = 0;              ///< 0 is unspecified; valid configured range starts at 0x0005.
  uint16_t settleCount = 0;
  uint8_t finDivider = 0;           ///< 0 is unspecified; valid range is 1..15.
  uint16_t frefDivider = 0;         ///< 0 is unspecified; valid range is 1..1023.
  uint16_t offset = 0;
  DriveCurrentCode driveCurrentCode{};  ///< Explicit code 0..31.
  uint32_t expectedSensorMinHz = 0;
  uint32_t expectedSensorMaxHz = 0;
};

/// @brief Complete desired device configuration and non-owning transport injection.
///
/// A default-constructed Config is deliberately invalid. The application must
/// explicitly select the part, address, reference clock, mode, and channel
/// mask; supply every per-channel register field for each physical channel of
/// the selected variant; and supply expected sensor-frequency bounds only for
/// channels selected in the channel mask before bind().
struct Config {
  I2cWriteFn i2cWrite = nullptr;
  I2cWriteReadFn i2cWriteRead = nullptr;
  void* i2cUser = nullptr;
  uint32_t i2cTimeoutMs = 0;
  I2cAddress i2cAddress = I2cAddress::UNSPECIFIED;

  IntbAssertedFn intbAsserted = nullptr;
  void* intbUser = nullptr;

  DeviceVariant variant = DeviceVariant::UNSPECIFIED;
  ChannelMask channels{};
  ChannelConfig channel[4]{};
  ReferenceClock referenceClock{};

  OperatingMode mode = OperatingMode::UNSPECIFIED;
  Channel activeChannel = Channel::NONE;
  RRSequence rrSequence = RRSequence::UNSPECIFIED;
  Deglitch deglitch = Deglitch::UNSPECIFIED;
  SensorActivation sensorActivation = SensorActivation::LOW_POWER;
  bool rpOverrideEnabled = true;
  bool autoAmplitudeCorrectionEnabled = false;
  bool highCurrentDriveEnabled = false;
  bool intbDisabled = true;
  ErrorReporting errorReporting{};
};

}  // namespace LDC1614
