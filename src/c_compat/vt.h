#pragma once

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

typedef struct libghostty_cpp_terminal_options {
  uint16_t cols;
  uint16_t rows;
  size_t max_scrollback;
} libghostty_cpp_terminal_options;

libghostty_cpp_result libghostty_cpp_terminal_new(
  libghostty_cpp_terminal** terminal,
  libghostty_cpp_terminal_options options
);

void libghostty_cpp_terminal_free(libghostty_cpp_terminal* terminal);

void libghostty_cpp_terminal_vt_write(
  libghostty_cpp_terminal* terminal,
  const uint8_t* data,
  size_t len
);

void libghostty_cpp_terminal_reset(libghostty_cpp_terminal* terminal);

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
