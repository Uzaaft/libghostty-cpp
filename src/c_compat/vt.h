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

typedef enum libghostty_cpp_terminal_screen {
  LIBGHOSTTY_CPP_TERMINAL_SCREEN_PRIMARY = 0,
  LIBGHOSTTY_CPP_TERMINAL_SCREEN_ALTERNATE = 1,
} libghostty_cpp_terminal_screen;

typedef struct libghostty_cpp_terminal_scrollbar {
  uint64_t total;
  uint64_t offset;
  uint64_t len;
} libghostty_cpp_terminal_scrollbar;

typedef enum libghostty_cpp_terminal_scroll_viewport_tag {
  LIBGHOSTTY_CPP_SCROLL_VIEWPORT_TOP = 0,
  LIBGHOSTTY_CPP_SCROLL_VIEWPORT_BOTTOM = 1,
  LIBGHOSTTY_CPP_SCROLL_VIEWPORT_DELTA = 2,
} libghostty_cpp_terminal_scroll_viewport_tag;

typedef union libghostty_cpp_terminal_scroll_viewport_value {
  ptrdiff_t delta;
  uint64_t padding[2];
} libghostty_cpp_terminal_scroll_viewport_value;

typedef struct libghostty_cpp_terminal_scroll_viewport {
  libghostty_cpp_terminal_scroll_viewport_tag tag;
  libghostty_cpp_terminal_scroll_viewport_value value;
} libghostty_cpp_terminal_scroll_viewport;

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

typedef void (*libghostty_cpp_terminal_bell_fn)(
  const libghostty_cpp_terminal* terminal,
  void* userdata
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

typedef void (*libghostty_cpp_terminal_title_changed_fn)(
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

libghostty_cpp_result libghostty_cpp_terminal_on_bell(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_bell_fn callback
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

libghostty_cpp_result libghostty_cpp_terminal_on_title_changed(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_title_changed_fn callback
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

libghostty_cpp_result libghostty_cpp_terminal_mode_get(
  const libghostty_cpp_terminal* terminal,
  uint16_t mode,
  bool* out_value
);

libghostty_cpp_result libghostty_cpp_terminal_mouse_tracking(
  const libghostty_cpp_terminal* terminal,
  bool* out_value
);

libghostty_cpp_result libghostty_cpp_terminal_get_active_screen(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_screen* out_screen
);

libghostty_cpp_result libghostty_cpp_terminal_get_scrollbar(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_scrollbar* out_scrollbar
);

libghostty_cpp_result libghostty_cpp_terminal_total_rows(
  const libghostty_cpp_terminal* terminal,
  size_t* out_total_rows
);

libghostty_cpp_result libghostty_cpp_terminal_scrollback_rows(
  const libghostty_cpp_terminal* terminal,
  size_t* out_scrollback_rows
);

void libghostty_cpp_terminal_set_scroll_viewport(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_scroll_viewport viewport
);

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

libghostty_cpp_result libghostty_cpp_terminal_title(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_string* out_title
);

libghostty_cpp_result libghostty_cpp_terminal_pwd(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_string* out_pwd
);

#ifdef __cplusplus
}
#endif
