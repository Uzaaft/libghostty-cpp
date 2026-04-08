#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct libghostty_cpp_rgb_color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} libghostty_cpp_rgb_color;

typedef enum libghostty_cpp_underline {
  LIBGHOSTTY_CPP_UNDERLINE_NONE = 0,
  LIBGHOSTTY_CPP_UNDERLINE_SINGLE = 1,
  LIBGHOSTTY_CPP_UNDERLINE_DOUBLE = 2,
  LIBGHOSTTY_CPP_UNDERLINE_CURLY = 3,
  LIBGHOSTTY_CPP_UNDERLINE_DOTTED = 4,
  LIBGHOSTTY_CPP_UNDERLINE_DASHED = 5,
} libghostty_cpp_underline;

typedef enum libghostty_cpp_style_color_tag {
  LIBGHOSTTY_CPP_STYLE_COLOR_NONE = 0,
  LIBGHOSTTY_CPP_STYLE_COLOR_PALETTE = 1,
  LIBGHOSTTY_CPP_STYLE_COLOR_RGB = 2,
} libghostty_cpp_style_color_tag;

typedef union libghostty_cpp_style_color_value {
  uint8_t palette;
  libghostty_cpp_rgb_color rgb;
  uint64_t _padding;
} libghostty_cpp_style_color_value;

typedef struct libghostty_cpp_style_color {
  libghostty_cpp_style_color_tag tag;
  libghostty_cpp_style_color_value value;
} libghostty_cpp_style_color;

typedef struct libghostty_cpp_style {
  libghostty_cpp_style_color fg_color;
  libghostty_cpp_style_color bg_color;
  libghostty_cpp_style_color underline_color;
  bool bold;
  bool italic;
  bool faint;
  bool blink;
  bool inverse;
  bool invisible;
  bool strikethrough;
  bool overline;
  libghostty_cpp_underline underline;
} libghostty_cpp_style;
