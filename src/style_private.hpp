#pragma once

#include "c_compat/style.h"
#include "libghostty_cpp/error.hpp"
#include "libghostty_cpp/style.hpp"

namespace libghostty_cpp::detail {

inline RgbColor translate_color(libghostty_cpp_rgb_color color) noexcept {
  return RgbColor{color.r, color.g, color.b};
}

inline StyleColor translate_style_color(libghostty_cpp_style_color color) {
  switch (color.tag) {
    case LIBGHOSTTY_CPP_STYLE_COLOR_NONE:
      return std::monostate{};
    case LIBGHOSTTY_CPP_STYLE_COLOR_PALETTE:
      return PaletteColor{color.value.palette};
    case LIBGHOSTTY_CPP_STYLE_COLOR_RGB:
      return translate_color(color.value.rgb);
  }

  throw Error(ErrorCode::InvalidValue);
}

inline Underline translate_underline(libghostty_cpp_underline underline) {
  switch (underline) {
    case LIBGHOSTTY_CPP_UNDERLINE_NONE:
      return Underline::None;
    case LIBGHOSTTY_CPP_UNDERLINE_SINGLE:
      return Underline::Single;
    case LIBGHOSTTY_CPP_UNDERLINE_DOUBLE:
      return Underline::Double;
    case LIBGHOSTTY_CPP_UNDERLINE_CURLY:
      return Underline::Curly;
    case LIBGHOSTTY_CPP_UNDERLINE_DOTTED:
      return Underline::Dotted;
    case LIBGHOSTTY_CPP_UNDERLINE_DASHED:
      return Underline::Dashed;
  }

  throw Error(ErrorCode::InvalidValue);
}

inline Style translate_style(libghostty_cpp_style style) {
  Style translated{};
  translated.fg_color = translate_style_color(style.fg_color);
  translated.bg_color = translate_style_color(style.bg_color);
  translated.underline_color = translate_style_color(style.underline_color);
  translated.bold = style.bold;
  translated.italic = style.italic;
  translated.faint = style.faint;
  translated.blink = style.blink;
  translated.inverse = style.inverse;
  translated.invisible = style.invisible;
  translated.strikethrough = style.strikethrough;
  translated.overline = style.overline;
  translated.underline = translate_underline(style.underline);
  return translated;
}

} // namespace libghostty_cpp::detail
