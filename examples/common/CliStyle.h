#pragma once

#ifndef LDC1614_CLI_ENABLE_COLOR
#define LDC1614_CLI_ENABLE_COLOR 1
#endif

#ifndef LDC1614_CLI_COLOR_DEFAULT
#define LDC1614_CLI_COLOR_DEFAULT 1
#endif

namespace ldc1614_cli {
namespace cli_style {

enum class Color {
  RESET,
  RED,
  GREEN,
  YELLOW,
  CYAN,
};

inline const char* code(bool enabled, Color color) {
#if LDC1614_CLI_ENABLE_COLOR
  if (!enabled) return "";
  switch (color) {
    case Color::RESET: return "\033[0m";
    case Color::RED: return "\033[31m";
    case Color::GREEN: return "\033[32m";
    case Color::YELLOW: return "\033[33m";
    case Color::CYAN: return "\033[36m";
  }
#else
  (void)enabled;
  (void)color;
#endif
  return "";
}

}  // namespace cli_style
}  // namespace ldc1614_cli
