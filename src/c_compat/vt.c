#include "c_compat/vt.h"

#include <ghostty/vt.h>

#include <stdlib.h>

struct libghostty_cpp_terminal {
  GhosttyTerminal inner;
};

static libghostty_cpp_result translate_result(GhosttyResult result) {
  switch (result) {
    case GHOSTTY_SUCCESS:
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_OUT_OF_MEMORY:
      return LIBGHOSTTY_CPP_RESULT_OUT_OF_MEMORY;
    case GHOSTTY_INVALID_VALUE:
      return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
    case GHOSTTY_OUT_OF_SPACE:
      return LIBGHOSTTY_CPP_RESULT_OUT_OF_SPACE;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

static libghostty_cpp_result get_u16(
  const libghostty_cpp_terminal* terminal,
  GhosttyTerminalData data,
  uint16_t* out_value
) {
  if (terminal == NULL || out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return translate_result(ghostty_terminal_get(terminal->inner, data, out_value));
}

libghostty_cpp_result libghostty_cpp_terminal_new(
  libghostty_cpp_terminal** terminal,
  libghostty_cpp_terminal_options options
) {
  if (terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *terminal = NULL;

  libghostty_cpp_terminal* wrapper = calloc(1, sizeof(*wrapper));
  if (wrapper == NULL) {
    return LIBGHOSTTY_CPP_RESULT_OUT_OF_MEMORY;
  }

  const GhosttyResult result = ghostty_terminal_new(
    NULL,
    &wrapper->inner,
    (GhosttyTerminalOptions) {
      .cols = options.cols,
      .rows = options.rows,
      .max_scrollback = options.max_scrollback,
    }
  );

  if (result != GHOSTTY_SUCCESS) {
    free(wrapper);
    return translate_result(result);
  }

  *terminal = wrapper;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

void libghostty_cpp_terminal_free(libghostty_cpp_terminal* terminal) {
  if (terminal == NULL) {
    return;
  }

  ghostty_terminal_free(terminal->inner);
  free(terminal);
}

void libghostty_cpp_terminal_vt_write(
  libghostty_cpp_terminal* terminal,
  const uint8_t* data,
  size_t len
) {
  if (terminal == NULL) {
    return;
  }

  if (data == NULL && len != 0) {
    return;
  }

  ghostty_terminal_vt_write(terminal->inner, data, len);
}

void libghostty_cpp_terminal_reset(libghostty_cpp_terminal* terminal) {
  if (terminal == NULL) {
    return;
  }

  ghostty_terminal_reset(terminal->inner);
}

libghostty_cpp_result libghostty_cpp_terminal_resize(
  libghostty_cpp_terminal* terminal,
  uint16_t cols,
  uint16_t rows,
  uint32_t cell_width_px,
  uint32_t cell_height_px
) {
  if (terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return translate_result(ghostty_terminal_resize(
    terminal->inner,
    cols,
    rows,
    cell_width_px,
    cell_height_px
  ));
}

libghostty_cpp_result libghostty_cpp_terminal_cols(
  const libghostty_cpp_terminal* terminal,
  uint16_t* out_cols
) {
  return get_u16(terminal, GHOSTTY_TERMINAL_DATA_COLS, out_cols);
}

libghostty_cpp_result libghostty_cpp_terminal_rows(
  const libghostty_cpp_terminal* terminal,
  uint16_t* out_rows
) {
  return get_u16(terminal, GHOSTTY_TERMINAL_DATA_ROWS, out_rows);
}

libghostty_cpp_result libghostty_cpp_terminal_cursor_x(
  const libghostty_cpp_terminal* terminal,
  uint16_t* out_cursor_x
) {
  return get_u16(terminal, GHOSTTY_TERMINAL_DATA_CURSOR_X, out_cursor_x);
}

libghostty_cpp_result libghostty_cpp_terminal_cursor_y(
  const libghostty_cpp_terminal* terminal,
  uint16_t* out_cursor_y
) {
  return get_u16(terminal, GHOSTTY_TERMINAL_DATA_CURSOR_Y, out_cursor_y);
}
