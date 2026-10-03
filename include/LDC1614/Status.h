/// @file Status.h
/// @brief Framework-neutral status contract for LDC1612/LDC1614 operations.
#pragma once

#include <cstdint>

namespace LDC1614 {

/// @brief Error codes returned by fallible public operations.
enum class Err : uint8_t {
  OK = 0,                ///< The requested action completed successfully.
  NOT_BOUND,             ///< bind() must succeed before this action.
  INVALID_CONFIG,        ///< Profile facts or a configuration requirement are invalid.
  INVALID_PARAM,         ///< An argument is outside the API's allowed values.
  BUSY,                  ///< Active work or the current state prevents this action.
  IN_PROGRESS,           ///< Work was admitted and needs more poll() calls.
  RESULT_NOT_READY,      ///< No terminal result is available to take.
  RESULT_QUEUE_FULL,     ///< No terminal-result slot can be reserved for new work.
  DUPLICATE_OPERATION_ID, ///< The ID belongs to active work or a queued result.
  DEVICE_NOT_FOUND,       ///< A completed identity read returned an unexpected value.
  /// Reserved for v3.0 numeric/source compatibility; current readiness APIs
  /// return a successful false snapshot or a precise precondition/I2C status.
  CONVERSION_NOT_READY,
  TIMEOUT,       ///< The owner operation deadline expired before another transfer.
  CANCELLED,     ///< The owner cancelled an admitted job.
  CONFIG_DIRTY,  ///< Required identity or applied-configuration trust is missing.
  I2C_ERROR,     ///< A transport attempt failed without a more precise classification.
  I2C_NACK_ADDR,  ///< Backend-confirmed address NACK; no device data was accepted.
  I2C_NACK_DATA,  ///< Backend-confirmed data NACK; a partial write remains possible.
  I2C_TIMEOUT,   ///< One transport attempt timed out; not an operation-deadline verdict.
  I2C_BUS,       ///< Backend-reported bus failure; recovery remains application policy.
};

/// @brief Status returned by all fallible operations.
struct Status {
  Err code = Err::OK;       ///< Machine-readable status code.
  int32_t detail = 0;       ///< Full transport or implementation detail.
  const char* msg = "";     ///< Static-lifetime diagnostic string.

  /// @brief Construct an OK status with empty diagnostic text.
  constexpr Status() = default;

  /// @brief Construct a status from its complete public representation.
  /// @param codeIn Machine-readable code.
  /// @param detailIn Backend or implementation-specific detail.
  /// @param msgIn Static-lifetime diagnostic string.
  constexpr Status(Err codeIn, int32_t detailIn, const char* msgIn)
      : code(codeIn), detail(detailIn), msg(msgIn) {}

  /// @return True only when `code == Err::OK`.
  constexpr bool ok() const { return code == Err::OK; }

  /// @param err Code to compare.
  /// @return True when this status contains `err`.
  constexpr bool is(Err err) const { return code == err; }

  /// @return True only when the operation remains in progress.
  constexpr bool inProgress() const { return code == Err::IN_PROGRESS; }

  /// @return Canonical successful status.
  static constexpr Status Ok() { return Status{Err::OK, 0, "OK"}; }

  /// @brief Build a non-success status without dynamic allocation.
  /// @param err Machine-readable error code.
  /// @param message Static-lifetime diagnostic string.
  /// @param detailCode Optional backend or implementation detail.
  /// @return Complete error status.
  static constexpr Status Error(Err err, const char* message,
                                int32_t detailCode = 0) {
    return Status{err, detailCode, message};
  }
};

}  // namespace LDC1614
