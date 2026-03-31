#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum libghostty_cpp_result {
  LIBGHOSTTY_CPP_RESULT_SUCCESS = 0,
  LIBGHOSTTY_CPP_RESULT_OUT_OF_MEMORY = 1,
  LIBGHOSTTY_CPP_RESULT_INVALID_VALUE = 2,
  LIBGHOSTTY_CPP_RESULT_OUT_OF_SPACE = 3,
} libghostty_cpp_result;

typedef struct libghostty_cpp_terminal libghostty_cpp_terminal;

typedef struct libghostty_cpp_string {
  const uint8_t* data;
  size_t len;
} libghostty_cpp_string;

typedef enum libghostty_cpp_color_scheme {
  LIBGHOSTTY_CPP_COLOR_SCHEME_LIGHT = 0,
  LIBGHOSTTY_CPP_COLOR_SCHEME_DARK = 1,
} libghostty_cpp_color_scheme;

typedef struct libghostty_cpp_terminal_options {
  uint16_t cols;
  uint16_t rows;
  size_t max_scrollback;
} libghostty_cpp_terminal_options;

typedef struct libghostty_cpp_size_report_size {
  uint16_t rows;
  uint16_t columns;
  uint32_t cell_width;
  uint32_t cell_height;
} libghostty_cpp_size_report_size;

typedef struct libghostty_cpp_device_attributes_primary {
  uint16_t conformance_level;
  uint16_t features[64];
  size_t num_features;
} libghostty_cpp_device_attributes_primary;

typedef struct libghostty_cpp_device_attributes_secondary {
  uint16_t device_type;
  uint16_t firmware_version;
  uint16_t rom_cartridge;
} libghostty_cpp_device_attributes_secondary;

typedef struct libghostty_cpp_device_attributes_tertiary {
  uint32_t unit_id;
} libghostty_cpp_device_attributes_tertiary;

typedef struct libghostty_cpp_device_attributes {
  libghostty_cpp_device_attributes_primary primary;
  libghostty_cpp_device_attributes_secondary secondary;
  libghostty_cpp_device_attributes_tertiary tertiary;
} libghostty_cpp_device_attributes;

typedef void (*libghostty_cpp_terminal_pty_write_fn)(
  const libghostty_cpp_terminal* terminal,
  void* userdata,
  const uint8_t* data,
  size_t len
);

typedef bool (*libghostty_cpp_terminal_size_fn)(
  const libghostty_cpp_terminal* terminal,
  void* userdata,
  libghostty_cpp_size_report_size* out_size
);

typedef bool (*libghostty_cpp_terminal_device_attributes_fn)(
  const libghostty_cpp_terminal* terminal,
  void* userdata,
  libghostty_cpp_device_attributes* out_attributes
);

typedef libghostty_cpp_string (*libghostty_cpp_terminal_xtversion_fn)(
  const libghostty_cpp_terminal* terminal,
  void* userdata
);

typedef bool (*libghostty_cpp_terminal_color_scheme_fn)(
  const libghostty_cpp_terminal* terminal,
  void* userdata,
  libghostty_cpp_color_scheme* out_scheme
);

libghostty_cpp_result libghostty_cpp_terminal_new(
  libghostty_cpp_terminal** terminal,
  libghostty_cpp_terminal_options options
);

libghostty_cpp_result libghostty_cpp_terminal_set_callback_userdata(
  libghostty_cpp_terminal* terminal,
  void* userdata
);

void libghostty_cpp_terminal_free(libghostty_cpp_terminal* terminal);

libghostty_cpp_result libghostty_cpp_terminal_on_pty_write(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_pty_write_fn callback
);

libghostty_cpp_result libghostty_cpp_terminal_on_size(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_size_fn callback
);

libghostty_cpp_result libghostty_cpp_terminal_on_device_attributes(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_device_attributes_fn callback
);

libghostty_cpp_result libghostty_cpp_terminal_on_xtversion(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_xtversion_fn callback
);

libghostty_cpp_result libghostty_cpp_terminal_on_color_scheme(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_color_scheme_fn callback
);

void libghostty_cpp_terminal_vt_write(
  libghostty_cpp_terminal* terminal,
  const uint8_t* data,
  size_t len
);

void libghostty_cpp_terminal_reset(libghostty_cpp_terminal* terminal);

void libghostty_cpp_terminal_scroll_viewport_delta(
  libghostty_cpp_terminal* terminal,
  ptrdiff_t delta
);

libghostty_cpp_result libghostty_cpp_terminal_resize(
  libghostty_cpp_terminal* terminal,
  uint16_t cols,
  uint16_t rows,
  uint32_t cell_width_px,
  uint32_t cell_height_px
);

libghostty_cpp_result libghostty_cpp_terminal_cols(
  const libghostty_cpp_terminal* terminal,
  uint16_t* out_cols
);

libghostty_cpp_result libghostty_cpp_terminal_rows(
  const libghostty_cpp_terminal* terminal,
  uint16_t* out_rows
);

libghostty_cpp_result libghostty_cpp_terminal_cursor_x(
  const libghostty_cpp_terminal* terminal,
  uint16_t* out_cursor_x
);

libghostty_cpp_result libghostty_cpp_terminal_cursor_y(
  const libghostty_cpp_terminal* terminal,
  uint16_t* out_cursor_y
);

#ifdef __cplusplus
}
#endif
