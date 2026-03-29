#pragma once

#include "c_compat/vt.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct libghostty_cpp_render_state libghostty_cpp_render_state;

typedef enum libghostty_cpp_render_dirty {
  LIBGHOSTTY_CPP_RENDER_DIRTY_CLEAN = 0,
  LIBGHOSTTY_CPP_RENDER_DIRTY_PARTIAL = 1,
  LIBGHOSTTY_CPP_RENDER_DIRTY_FULL = 2,
} libghostty_cpp_render_dirty;

typedef enum libghostty_cpp_cursor_visual_style {
  LIBGHOSTTY_CPP_CURSOR_VISUAL_STYLE_BAR = 0,
  LIBGHOSTTY_CPP_CURSOR_VISUAL_STYLE_BLOCK = 1,
  LIBGHOSTTY_CPP_CURSOR_VISUAL_STYLE_UNDERLINE = 2,
  LIBGHOSTTY_CPP_CURSOR_VISUAL_STYLE_BLOCK_HOLLOW = 3,
} libghostty_cpp_cursor_visual_style;

typedef struct libghostty_cpp_rgb_color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} libghostty_cpp_rgb_color;

typedef struct libghostty_cpp_cursor_viewport {
  uint16_t x;
  uint16_t y;
  bool at_wide_tail;
} libghostty_cpp_cursor_viewport;

typedef struct libghostty_cpp_render_colors {
  libghostty_cpp_rgb_color background;
  libghostty_cpp_rgb_color foreground;
  libghostty_cpp_rgb_color cursor;
  bool cursor_has_value;
  libghostty_cpp_rgb_color palette[256];
} libghostty_cpp_render_colors;

libghostty_cpp_result libghostty_cpp_render_state_new(
  libghostty_cpp_render_state **render_state
);

void libghostty_cpp_render_state_free(libghostty_cpp_render_state *render_state);

libghostty_cpp_result libghostty_cpp_render_state_update(
  libghostty_cpp_render_state *render_state,
  libghostty_cpp_terminal *terminal
);

libghostty_cpp_result libghostty_cpp_render_state_set_dirty(
  libghostty_cpp_render_state *render_state,
  libghostty_cpp_render_dirty dirty
);

libghostty_cpp_result libghostty_cpp_render_state_dirty(
  libghostty_cpp_render_state *render_state,
  libghostty_cpp_render_dirty *out_dirty
);

libghostty_cpp_result libghostty_cpp_render_state_cols(
  libghostty_cpp_render_state *render_state,
  uint16_t *out_cols
);

libghostty_cpp_result libghostty_cpp_render_state_rows(
  libghostty_cpp_render_state *render_state,
  uint16_t *out_rows
);

libghostty_cpp_result libghostty_cpp_render_state_colors(
  libghostty_cpp_render_state *render_state,
  libghostty_cpp_render_colors *out_colors
);

libghostty_cpp_result libghostty_cpp_render_state_cursor_visible(
  libghostty_cpp_render_state *render_state,
  bool *out_visible
);

libghostty_cpp_result libghostty_cpp_render_state_cursor_blinking(
  libghostty_cpp_render_state *render_state,
  bool *out_blinking
);

libghostty_cpp_result libghostty_cpp_render_state_cursor_password_input(
  libghostty_cpp_render_state *render_state,
  bool *out_password_input
);

libghostty_cpp_result libghostty_cpp_render_state_cursor_visual_style(
  libghostty_cpp_render_state *render_state,
  libghostty_cpp_cursor_visual_style *out_style
);

libghostty_cpp_result libghostty_cpp_render_state_cursor_viewport(
  libghostty_cpp_render_state *render_state,
  bool *out_has_value,
  libghostty_cpp_cursor_viewport *out_viewport
);

#ifdef __cplusplus
}
#endif
