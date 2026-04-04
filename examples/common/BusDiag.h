/**
 * @file BusDiag.h
 * @brief Bus diagnostic wrapper for examples.
 *
 * NOT part of the library. Example-only diagnostic tool.
 */

#pragma once

#include <Wire.h>

#include "I2cScanner.h"

namespace bus_diag {
inline void scan() {
  i2c_scanner::scan(Wire);
}
}  // namespace bus_diag
