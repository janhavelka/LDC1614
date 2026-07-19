/// @file Status.h
/// @brief Framework-neutral status contract for LDC1612/LDC1614 operations.
#pragma once

#include <cstdint>

namespace LDC1614 {

/// @brief Error codes returned by fallible public operations.
enum class Err : uint8_t {
  OK = 0,
  NOT_BOUND,
  INVALID_CONFIG,
  INVALID_PARAM,
  BUSY,
  IN_PROGRESS,
  RESULT_NOT_READY,
  RESULT_QUEUE_FULL,
  DUPLICATE_OPERATION_ID,
  DEVICE_NOT_FOUND,
  CONVERSION_NOT_READY,
  TIMEOUT,
  CANCELLED,
  CONFIG_DIRTY,
  I2C_ERROR,
  I2C_NACK_ADDR,
  I2C_NACK_DATA,
  I2C_TIMEOUT,
  I2C_BUS,
};

/// @brief Status returned by all fallible operations.
struct Status {
  Err code = Err::OK;
  int32_t detail = 0;       ///< Full transport or implementation detail.
  const char* msg = "";     ///< Static-lifetime diagnostic string.

  constexpr Status() = default;
  constexpr Status(Err codeIn, int32_t detailIn, const char* msgIn)
      : code(codeIn), detail(detailIn), msg(msgIn) {}

  constexpr bool ok() const { return code == Err::OK; }
  constexpr bool is(Err err) const { return code == err; }
  constexpr bool inProgress() const { return code == Err::IN_PROGRESS; }

  static constexpr Status Ok() { return Status{Err::OK, 0, "OK"}; }
  static constexpr Status Error(Err err, const char* message,
                                int32_t detailCode = 0) {
    return Status{err, detailCode, message};
  }
};

}  // namespace LDC1614
