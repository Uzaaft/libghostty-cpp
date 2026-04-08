#pragma once

#include <cstdint>
#include <variant>

namespace libghostty_cpp {

struct RgbColor {
  std::uint8_t r;
  std::uint8_t g;
  std::uint8_t b;
};

struct PaletteColor {
  std::uint8_t index;
};

using StyleColor = std::variant<std::monostate, PaletteColor, RgbColor>;

enum class Underline {
  None,
  Single,
  Double,
  Curly,
  Dotted,
  Dashed,
};

struct Style {
  StyleColor fg_color;
  StyleColor bg_color;
  StyleColor underline_color;
  bool bold = false;
  bool italic = false;
  bool faint = false;
  bool blink = false;
  bool inverse = false;
  bool invisible = false;
  bool strikethrough = false;
  bool overline = false;
  Underline underline = Underline::None;

  [[nodiscard]] bool is_default() const noexcept;
};

} // namespace libghostty_cpp
