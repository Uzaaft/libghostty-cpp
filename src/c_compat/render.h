#pragma once

#include "c_compat/style.h"
#include "c_compat/vt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct libghostty_cpp_render_state libghostty_cpp_render_state;
typedef struct libghostty_cpp_render_state_row_iterator libghostty_cpp_render_state_row_iterator;
typedef struct libghostty_cpp_render_state_row_cells libghostty_cpp_render_state_row_cells;

typedef uint64_t libghostty_cpp_row;
typedef uint64_t libghostty_cpp_cell;

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

typedef enum libghostty_cpp_row_semantic_prompt {
  LIBGHOSTTY_CPP_ROW_SEMANTIC_PROMPT_NONE = 0,
  LIBGHOSTTY_CPP_ROW_SEMANTIC_PROMPT_PROMPT = 1,
  LIBGHOSTTY_CPP_ROW_SEMANTIC_PROMPT_CONTINUATION = 2,
} libghostty_cpp_row_semantic_prompt;

typedef enum libghostty_cpp_cell_content_tag {
  LIBGHOSTTY_CPP_CELL_CONTENT_TAG_CODEPOINT = 0,
  LIBGHOSTTY_CPP_CELL_CONTENT_TAG_CODEPOINT_GRAPHEME = 1,
  LIBGHOSTTY_CPP_CELL_CONTENT_TAG_BG_COLOR_PALETTE = 2,
  LIBGHOSTTY_CPP_CELL_CONTENT_TAG_BG_COLOR_RGB = 3,
} libghostty_cpp_cell_content_tag;

typedef enum libghostty_cpp_cell_wide {
  LIBGHOSTTY_CPP_CELL_WIDE_NARROW = 0,
  LIBGHOSTTY_CPP_CELL_WIDE_WIDE = 1,
  LIBGHOSTTY_CPP_CELL_WIDE_SPACER_TAIL = 2,
  LIBGHOSTTY_CPP_CELL_WIDE_SPACER_HEAD = 3,
} libghostty_cpp_cell_wide;

typedef enum libghostty_cpp_cell_semantic_content {
  LIBGHOSTTY_CPP_CELL_SEMANTIC_CONTENT_OUTPUT = 0,
  LIBGHOSTTY_CPP_CELL_SEMANTIC_CONTENT_INPUT = 1,
  LIBGHOSTTY_CPP_CELL_SEMANTIC_CONTENT_PROMPT = 2,
} libghostty_cpp_cell_semantic_content;

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

libghostty_cpp_result libghostty_cpp_render_state_row_iterator_new(
  libghostty_cpp_render_state_row_iterator **row_iterator
);

void libghostty_cpp_render_state_row_iterator_free(
  libghostty_cpp_render_state_row_iterator *row_iterator
);

libghostty_cpp_result libghostty_cpp_render_state_row_iterator_update(
  libghostty_cpp_render_state *render_state,
  libghostty_cpp_render_state_row_iterator *row_iterator
);

bool libghostty_cpp_render_state_row_iterator_next(
  libghostty_cpp_render_state_row_iterator *row_iterator
);

libghostty_cpp_result libghostty_cpp_render_state_row_iterator_dirty(
  libghostty_cpp_render_state_row_iterator *row_iterator,
  bool *out_dirty
);

libghostty_cpp_result libghostty_cpp_render_state_row_iterator_set_dirty(
  libghostty_cpp_render_state_row_iterator *row_iterator,
  bool dirty
);

libghostty_cpp_result libghostty_cpp_render_state_row_iterator_row(
  libghostty_cpp_render_state_row_iterator *row_iterator,
  libghostty_cpp_row *out_row
);

libghostty_cpp_result libghostty_cpp_render_state_row_cells_new(
  libghostty_cpp_render_state_row_cells **row_cells
);

void libghostty_cpp_render_state_row_cells_free(
  libghostty_cpp_render_state_row_cells *row_cells
);

libghostty_cpp_result libghostty_cpp_render_state_row_cells_update(
  libghostty_cpp_render_state_row_iterator *row_iterator,
  libghostty_cpp_render_state_row_cells *row_cells
);

bool libghostty_cpp_render_state_row_cells_next(
  libghostty_cpp_render_state_row_cells *row_cells
);

libghostty_cpp_result libghostty_cpp_render_state_row_cells_select(
  libghostty_cpp_render_state_row_cells *row_cells,
  uint16_t x
);

libghostty_cpp_result libghostty_cpp_render_state_row_cells_cell(
  libghostty_cpp_render_state_row_cells *row_cells,
  libghostty_cpp_cell *out_cell
);

libghostty_cpp_result libghostty_cpp_render_state_row_cells_style(
  libghostty_cpp_render_state_row_cells *row_cells,
  libghostty_cpp_style *out_style
);

libghostty_cpp_result libghostty_cpp_render_state_row_cells_fg_color(
  libghostty_cpp_render_state_row_cells *row_cells,
  bool *out_has_value,
  libghostty_cpp_rgb_color *out_color
);

libghostty_cpp_result libghostty_cpp_render_state_row_cells_bg_color(
  libghostty_cpp_render_state_row_cells *row_cells,
  bool *out_has_value,
  libghostty_cpp_rgb_color *out_color
);

libghostty_cpp_result libghostty_cpp_render_state_row_cells_graphemes_len(
  libghostty_cpp_render_state_row_cells *row_cells,
  size_t *out_len
);

libghostty_cpp_result libghostty_cpp_render_state_row_cells_graphemes(
  libghostty_cpp_render_state_row_cells *row_cells,
  uint32_t *out_codepoints
);

libghostty_cpp_result libghostty_cpp_row_is_wrapped(
  libghostty_cpp_row row,
  bool *out_is_wrapped
);

libghostty_cpp_result libghostty_cpp_row_is_wrap_continuation(
  libghostty_cpp_row row,
  bool *out_is_wrap_continuation
);

libghostty_cpp_result libghostty_cpp_row_has_grapheme_cluster(
  libghostty_cpp_row row,
  bool *out_has_grapheme_cluster
);

libghostty_cpp_result libghostty_cpp_row_is_styled(
  libghostty_cpp_row row,
  bool *out_is_styled
);

libghostty_cpp_result libghostty_cpp_row_has_hyperlink(
  libghostty_cpp_row row,
  bool *out_has_hyperlink
);

libghostty_cpp_result libghostty_cpp_row_get_semantic_prompt(
  libghostty_cpp_row row,
  libghostty_cpp_row_semantic_prompt *out_semantic_prompt
);

libghostty_cpp_result libghostty_cpp_row_has_kitty_virtual_placeholder(
  libghostty_cpp_row row,
  bool *out_has_kitty_virtual_placeholder
);

libghostty_cpp_result libghostty_cpp_row_is_dirty(
  libghostty_cpp_row row,
  bool *out_is_dirty
);

libghostty_cpp_result libghostty_cpp_cell_codepoint(
  libghostty_cpp_cell cell,
  uint32_t *out_codepoint
);

libghostty_cpp_result libghostty_cpp_cell_get_content_tag(
  libghostty_cpp_cell cell,
  libghostty_cpp_cell_content_tag *out_content_tag
);

libghostty_cpp_result libghostty_cpp_cell_get_wide(
  libghostty_cpp_cell cell,
  libghostty_cpp_cell_wide *out_wide
);

libghostty_cpp_result libghostty_cpp_cell_has_text(
  libghostty_cpp_cell cell,
  bool *out_has_text
);

libghostty_cpp_result libghostty_cpp_cell_has_styling(
  libghostty_cpp_cell cell,
  bool *out_has_styling
);

libghostty_cpp_result libghostty_cpp_cell_style_id(
  libghostty_cpp_cell cell,
  uint16_t *out_style_id
);

libghostty_cpp_result libghostty_cpp_cell_has_hyperlink(
  libghostty_cpp_cell cell,
  bool *out_has_hyperlink
);

libghostty_cpp_result libghostty_cpp_cell_is_protected(
  libghostty_cpp_cell cell,
  bool *out_is_protected
);

libghostty_cpp_result libghostty_cpp_cell_get_semantic_content(
  libghostty_cpp_cell cell,
  libghostty_cpp_cell_semantic_content *out_semantic_content
);

libghostty_cpp_result libghostty_cpp_cell_bg_color_palette(
  libghostty_cpp_cell cell,
  uint8_t *out_palette
);

libghostty_cpp_result libghostty_cpp_cell_bg_color_rgb(
  libghostty_cpp_cell cell,
  libghostty_cpp_rgb_color *out_color
);

#ifdef __cplusplus
}
#endif
