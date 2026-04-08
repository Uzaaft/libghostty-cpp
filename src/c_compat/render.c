#include "c_compat/render.h"

#include "c_compat/internal.h"

#include <stddef.h>
#include <stdlib.h>

struct libghostty_cpp_render_state {
  GhosttyRenderState inner;
};

struct libghostty_cpp_render_state_row_iterator {
  GhosttyRenderStateRowIterator inner;
};

struct libghostty_cpp_render_state_row_cells {
  GhosttyRenderStateRowCells inner;
};

static libghostty_cpp_rgb_color translate_color(GhosttyColorRgb color) {
  return (libghostty_cpp_rgb_color) {
    .r = color.r,
    .g = color.g,
    .b = color.b,
  };
}

static libghostty_cpp_result translate_underline(
  GhosttySgrUnderline underline,
  libghostty_cpp_underline *out_underline
) {
  if (out_underline == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  switch (underline) {
    case GHOSTTY_SGR_UNDERLINE_NONE:
      *out_underline = LIBGHOSTTY_CPP_UNDERLINE_NONE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_SGR_UNDERLINE_SINGLE:
      *out_underline = LIBGHOSTTY_CPP_UNDERLINE_SINGLE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_SGR_UNDERLINE_DOUBLE:
      *out_underline = LIBGHOSTTY_CPP_UNDERLINE_DOUBLE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_SGR_UNDERLINE_CURLY:
      *out_underline = LIBGHOSTTY_CPP_UNDERLINE_CURLY;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_SGR_UNDERLINE_DOTTED:
      *out_underline = LIBGHOSTTY_CPP_UNDERLINE_DOTTED;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_SGR_UNDERLINE_DASHED:
      *out_underline = LIBGHOSTTY_CPP_UNDERLINE_DASHED;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

static libghostty_cpp_result translate_style_color(
  GhosttyStyleColor color,
  libghostty_cpp_style_color *out_color
) {
  if (out_color == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  switch (color.tag) {
    case GHOSTTY_STYLE_COLOR_NONE:
      out_color->tag = LIBGHOSTTY_CPP_STYLE_COLOR_NONE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_STYLE_COLOR_PALETTE:
      out_color->tag = LIBGHOSTTY_CPP_STYLE_COLOR_PALETTE;
      out_color->value.palette = color.value.palette;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_STYLE_COLOR_RGB:
      out_color->tag = LIBGHOSTTY_CPP_STYLE_COLOR_RGB;
      out_color->value.rgb = translate_color(color.value.rgb);
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

static libghostty_cpp_result translate_style(
  GhosttyStyle style,
  libghostty_cpp_style *out_style
) {
  if (out_style == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  const libghostty_cpp_result fg_result =
    translate_style_color(style.fg_color, &out_style->fg_color);
  if (fg_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return fg_result;
  }

  const libghostty_cpp_result bg_result =
    translate_style_color(style.bg_color, &out_style->bg_color);
  if (bg_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return bg_result;
  }

  const libghostty_cpp_result underline_color_result =
    translate_style_color(style.underline_color, &out_style->underline_color);
  if (underline_color_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return underline_color_result;
  }

  out_style->bold = style.bold;
  out_style->italic = style.italic;
  out_style->faint = style.faint;
  out_style->blink = style.blink;
  out_style->inverse = style.inverse;
  out_style->invisible = style.invisible;
  out_style->strikethrough = style.strikethrough;
  out_style->overline = style.overline;

  return translate_underline((GhosttySgrUnderline) style.underline, &out_style->underline);
}

static libghostty_cpp_result translate_row_semantic_prompt(
  GhosttyRowSemanticPrompt semantic_prompt,
  libghostty_cpp_row_semantic_prompt *out_semantic_prompt
) {
  if (out_semantic_prompt == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  switch (semantic_prompt) {
    case GHOSTTY_ROW_SEMANTIC_NONE:
      *out_semantic_prompt = LIBGHOSTTY_CPP_ROW_SEMANTIC_PROMPT_NONE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_ROW_SEMANTIC_PROMPT:
      *out_semantic_prompt = LIBGHOSTTY_CPP_ROW_SEMANTIC_PROMPT_PROMPT;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_ROW_SEMANTIC_PROMPT_CONTINUATION:
      *out_semantic_prompt = LIBGHOSTTY_CPP_ROW_SEMANTIC_PROMPT_CONTINUATION;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

static libghostty_cpp_result translate_cell_content_tag(
  GhosttyCellContentTag content_tag,
  libghostty_cpp_cell_content_tag *out_content_tag
) {
  if (out_content_tag == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  switch (content_tag) {
    case GHOSTTY_CELL_CONTENT_CODEPOINT:
      *out_content_tag = LIBGHOSTTY_CPP_CELL_CONTENT_TAG_CODEPOINT;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_CELL_CONTENT_CODEPOINT_GRAPHEME:
      *out_content_tag = LIBGHOSTTY_CPP_CELL_CONTENT_TAG_CODEPOINT_GRAPHEME;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_CELL_CONTENT_BG_COLOR_PALETTE:
      *out_content_tag = LIBGHOSTTY_CPP_CELL_CONTENT_TAG_BG_COLOR_PALETTE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_CELL_CONTENT_BG_COLOR_RGB:
      *out_content_tag = LIBGHOSTTY_CPP_CELL_CONTENT_TAG_BG_COLOR_RGB;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

static libghostty_cpp_result translate_cell_wide(
  GhosttyCellWide wide,
  libghostty_cpp_cell_wide *out_wide
) {
  if (out_wide == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  switch (wide) {
    case GHOSTTY_CELL_WIDE_NARROW:
      *out_wide = LIBGHOSTTY_CPP_CELL_WIDE_NARROW;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_CELL_WIDE_WIDE:
      *out_wide = LIBGHOSTTY_CPP_CELL_WIDE_WIDE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_CELL_WIDE_SPACER_TAIL:
      *out_wide = LIBGHOSTTY_CPP_CELL_WIDE_SPACER_TAIL;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_CELL_WIDE_SPACER_HEAD:
      *out_wide = LIBGHOSTTY_CPP_CELL_WIDE_SPACER_HEAD;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

static libghostty_cpp_result translate_cell_semantic_content(
  GhosttyCellSemanticContent semantic_content,
  libghostty_cpp_cell_semantic_content *out_semantic_content
) {
  if (out_semantic_content == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  switch (semantic_content) {
    case GHOSTTY_CELL_SEMANTIC_OUTPUT:
      *out_semantic_content = LIBGHOSTTY_CPP_CELL_SEMANTIC_CONTENT_OUTPUT;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_CELL_SEMANTIC_INPUT:
      *out_semantic_content = LIBGHOSTTY_CPP_CELL_SEMANTIC_CONTENT_INPUT;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_CELL_SEMANTIC_PROMPT:
      *out_semantic_content = LIBGHOSTTY_CPP_CELL_SEMANTIC_CONTENT_PROMPT;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

static libghostty_cpp_result translate_dirty(
  GhosttyRenderStateDirty dirty,
  libghostty_cpp_render_dirty *out_dirty
) {
  if (out_dirty == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  switch (dirty) {
    case GHOSTTY_RENDER_STATE_DIRTY_FALSE:
      *out_dirty = LIBGHOSTTY_CPP_RENDER_DIRTY_CLEAN;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_RENDER_STATE_DIRTY_PARTIAL:
      *out_dirty = LIBGHOSTTY_CPP_RENDER_DIRTY_PARTIAL;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_RENDER_STATE_DIRTY_FULL:
      *out_dirty = LIBGHOSTTY_CPP_RENDER_DIRTY_FULL;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

static libghostty_cpp_result translate_cursor_visual_style(
  GhosttyRenderStateCursorVisualStyle style,
  libghostty_cpp_cursor_visual_style *out_style
) {
  if (out_style == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  switch (style) {
    case GHOSTTY_RENDER_STATE_CURSOR_VISUAL_STYLE_BAR:
      *out_style = LIBGHOSTTY_CPP_CURSOR_VISUAL_STYLE_BAR;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_RENDER_STATE_CURSOR_VISUAL_STYLE_BLOCK:
      *out_style = LIBGHOSTTY_CPP_CURSOR_VISUAL_STYLE_BLOCK;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_RENDER_STATE_CURSOR_VISUAL_STYLE_UNDERLINE:
      *out_style = LIBGHOSTTY_CPP_CURSOR_VISUAL_STYLE_UNDERLINE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case GHOSTTY_RENDER_STATE_CURSOR_VISUAL_STYLE_BLOCK_HOLLOW:
      *out_style = LIBGHOSTTY_CPP_CURSOR_VISUAL_STYLE_BLOCK_HOLLOW;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

static libghostty_cpp_result translate_dirty_from_cpp(
  libghostty_cpp_render_dirty dirty,
  GhosttyRenderStateDirty *out_dirty
) {
  if (out_dirty == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  switch (dirty) {
    case LIBGHOSTTY_CPP_RENDER_DIRTY_CLEAN:
      *out_dirty = GHOSTTY_RENDER_STATE_DIRTY_FALSE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_RENDER_DIRTY_PARTIAL:
      *out_dirty = GHOSTTY_RENDER_STATE_DIRTY_PARTIAL;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_RENDER_DIRTY_FULL:
      *out_dirty = GHOSTTY_RENDER_STATE_DIRTY_FULL;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

static libghostty_cpp_result render_state_get_bool(
  libghostty_cpp_render_state *render_state,
  GhosttyRenderStateData data,
  bool *out_value
) {
  if (render_state == NULL || out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_render_state_get(render_state->inner, data, out_value)
  );
}

static libghostty_cpp_result render_state_get_u16(
  libghostty_cpp_render_state *render_state,
  GhosttyRenderStateData data,
  uint16_t *out_value
) {
  if (render_state == NULL || out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_render_state_get(render_state->inner, data, out_value)
  );
}

static libghostty_cpp_result row_get_bool(
  libghostty_cpp_row row,
  GhosttyRowData data,
  bool *out_value
) {
  if (out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_row_get((GhosttyRow) row, data, out_value)
  );
}

static libghostty_cpp_result cell_get_bool(
  libghostty_cpp_cell cell,
  GhosttyCellData data,
  bool *out_value
) {
  if (out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_cell_get((GhosttyCell) cell, data, out_value)
  );
}

static libghostty_cpp_result cell_get_u32(
  libghostty_cpp_cell cell,
  GhosttyCellData data,
  uint32_t *out_value
) {
  if (out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_cell_get((GhosttyCell) cell, data, out_value)
  );
}

static libghostty_cpp_result cell_get_u16(
  libghostty_cpp_cell cell,
  GhosttyCellData data,
  uint16_t *out_value
) {
  if (out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_cell_get((GhosttyCell) cell, data, out_value)
  );
}

libghostty_cpp_result libghostty_cpp_render_state_new(
  libghostty_cpp_render_state **render_state
) {
  if (render_state == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *render_state = NULL;

  libghostty_cpp_render_state *wrapper = calloc(1, sizeof(*wrapper));
  if (wrapper == NULL) {
    return LIBGHOSTTY_CPP_RESULT_OUT_OF_MEMORY;
  }

  const GhosttyResult result = ghostty_render_state_new(NULL, &wrapper->inner);
  if (result != GHOSTTY_SUCCESS) {
    free(wrapper);
    return libghostty_cpp_translate_result(result);
  }

  *render_state = wrapper;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

void libghostty_cpp_render_state_free(libghostty_cpp_render_state *render_state) {
  if (render_state == NULL) {
    return;
  }

  ghostty_render_state_free(render_state->inner);
  free(render_state);
}

libghostty_cpp_result libghostty_cpp_render_state_update(
  libghostty_cpp_render_state *render_state,
  libghostty_cpp_terminal *terminal
) {
  if (render_state == NULL || terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_render_state_update(render_state->inner, terminal->inner)
  );
}

libghostty_cpp_result libghostty_cpp_render_state_set_dirty(
  libghostty_cpp_render_state *render_state,
  libghostty_cpp_render_dirty dirty
) {
  if (render_state == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyRenderStateDirty raw_dirty = GHOSTTY_RENDER_STATE_DIRTY_FALSE;
  const libghostty_cpp_result convert_result =
    translate_dirty_from_cpp(dirty, &raw_dirty);
  if (convert_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return convert_result;
  }

  return libghostty_cpp_translate_result(ghostty_render_state_set(
    render_state->inner,
    GHOSTTY_RENDER_STATE_OPTION_DIRTY,
    &raw_dirty
  ));
}

libghostty_cpp_result libghostty_cpp_render_state_dirty(
  libghostty_cpp_render_state *render_state,
  libghostty_cpp_render_dirty *out_dirty
) {
  if (render_state == NULL || out_dirty == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyRenderStateDirty raw_dirty = GHOSTTY_RENDER_STATE_DIRTY_FALSE;
  const GhosttyResult result = ghostty_render_state_get(
    render_state->inner,
    GHOSTTY_RENDER_STATE_DATA_DIRTY,
    &raw_dirty
  );
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  return translate_dirty(raw_dirty, out_dirty);
}

libghostty_cpp_result libghostty_cpp_render_state_cols(
  libghostty_cpp_render_state *render_state,
  uint16_t *out_cols
) {
  return render_state_get_u16(render_state, GHOSTTY_RENDER_STATE_DATA_COLS, out_cols);
}

libghostty_cpp_result libghostty_cpp_render_state_rows(
  libghostty_cpp_render_state *render_state,
  uint16_t *out_rows
) {
  return render_state_get_u16(render_state, GHOSTTY_RENDER_STATE_DATA_ROWS, out_rows);
}

libghostty_cpp_result libghostty_cpp_render_state_colors(
  libghostty_cpp_render_state *render_state,
  libghostty_cpp_render_colors *out_colors
) {
  if (render_state == NULL || out_colors == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyRenderStateColors raw_colors = GHOSTTY_INIT_SIZED(GhosttyRenderStateColors);
  const GhosttyResult result = ghostty_render_state_colors_get(
    render_state->inner,
    &raw_colors
  );
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  out_colors->background = translate_color(raw_colors.background);
  out_colors->foreground = translate_color(raw_colors.foreground);
  out_colors->cursor = translate_color(raw_colors.cursor);
  out_colors->cursor_has_value = raw_colors.cursor_has_value;

  for (size_t i = 0; i < 256; ++i) {
    out_colors->palette[i] = translate_color(raw_colors.palette[i]);
  }

  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_render_state_cursor_visible(
  libghostty_cpp_render_state *render_state,
  bool *out_visible
) {
  return render_state_get_bool(
    render_state,
    GHOSTTY_RENDER_STATE_DATA_CURSOR_VISIBLE,
    out_visible
  );
}

libghostty_cpp_result libghostty_cpp_render_state_cursor_blinking(
  libghostty_cpp_render_state *render_state,
  bool *out_blinking
) {
  return render_state_get_bool(
    render_state,
    GHOSTTY_RENDER_STATE_DATA_CURSOR_BLINKING,
    out_blinking
  );
}

libghostty_cpp_result libghostty_cpp_render_state_cursor_password_input(
  libghostty_cpp_render_state *render_state,
  bool *out_password_input
) {
  return render_state_get_bool(
    render_state,
    GHOSTTY_RENDER_STATE_DATA_CURSOR_PASSWORD_INPUT,
    out_password_input
  );
}

libghostty_cpp_result libghostty_cpp_render_state_cursor_visual_style(
  libghostty_cpp_render_state *render_state,
  libghostty_cpp_cursor_visual_style *out_style
) {
  if (render_state == NULL || out_style == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyRenderStateCursorVisualStyle raw_style =
    GHOSTTY_RENDER_STATE_CURSOR_VISUAL_STYLE_BLOCK;
  const GhosttyResult result = ghostty_render_state_get(
    render_state->inner,
    GHOSTTY_RENDER_STATE_DATA_CURSOR_VISUAL_STYLE,
    &raw_style
  );
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  return translate_cursor_visual_style(raw_style, out_style);
}

libghostty_cpp_result libghostty_cpp_render_state_cursor_viewport(
  libghostty_cpp_render_state *render_state,
  bool *out_has_value,
  libghostty_cpp_cursor_viewport *out_viewport
) {
  if (render_state == NULL || out_has_value == NULL || out_viewport == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  const libghostty_cpp_result has_value_result = render_state_get_bool(
    render_state,
    GHOSTTY_RENDER_STATE_DATA_CURSOR_VIEWPORT_HAS_VALUE,
    out_has_value
  );
  if (has_value_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return has_value_result;
  }

  if (!*out_has_value) {
    return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  const libghostty_cpp_result x_result = render_state_get_u16(
    render_state,
    GHOSTTY_RENDER_STATE_DATA_CURSOR_VIEWPORT_X,
    &out_viewport->x
  );
  if (x_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return x_result;
  }

  const libghostty_cpp_result y_result = render_state_get_u16(
    render_state,
    GHOSTTY_RENDER_STATE_DATA_CURSOR_VIEWPORT_Y,
    &out_viewport->y
  );
  if (y_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return y_result;
  }

  return render_state_get_bool(
    render_state,
    GHOSTTY_RENDER_STATE_DATA_CURSOR_VIEWPORT_WIDE_TAIL,
    &out_viewport->at_wide_tail
  );
}

libghostty_cpp_result libghostty_cpp_render_state_row_iterator_new(
  libghostty_cpp_render_state_row_iterator **row_iterator
) {
  if (row_iterator == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *row_iterator = NULL;

  libghostty_cpp_render_state_row_iterator *wrapper = calloc(1, sizeof(*wrapper));
  if (wrapper == NULL) {
    return LIBGHOSTTY_CPP_RESULT_OUT_OF_MEMORY;
  }

  const GhosttyResult result = ghostty_render_state_row_iterator_new(NULL, &wrapper->inner);
  if (result != GHOSTTY_SUCCESS) {
    free(wrapper);
    return libghostty_cpp_translate_result(result);
  }

  *row_iterator = wrapper;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

void libghostty_cpp_render_state_row_iterator_free(
  libghostty_cpp_render_state_row_iterator *row_iterator
) {
  if (row_iterator == NULL) {
    return;
  }

  ghostty_render_state_row_iterator_free(row_iterator->inner);
  free(row_iterator);
}

libghostty_cpp_result libghostty_cpp_render_state_row_iterator_update(
  libghostty_cpp_render_state *render_state,
  libghostty_cpp_render_state_row_iterator *row_iterator
) {
  if (render_state == NULL || row_iterator == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(ghostty_render_state_get(
    render_state->inner,
    GHOSTTY_RENDER_STATE_DATA_ROW_ITERATOR,
    &row_iterator->inner
  ));
}

bool libghostty_cpp_render_state_row_iterator_next(
  libghostty_cpp_render_state_row_iterator *row_iterator
) {
  if (row_iterator == NULL) {
    return false;
  }

  return ghostty_render_state_row_iterator_next(row_iterator->inner);
}

libghostty_cpp_result libghostty_cpp_render_state_row_iterator_dirty(
  libghostty_cpp_render_state_row_iterator *row_iterator,
  bool *out_dirty
) {
  if (row_iterator == NULL || out_dirty == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(ghostty_render_state_row_get(
    row_iterator->inner,
    GHOSTTY_RENDER_STATE_ROW_DATA_DIRTY,
    out_dirty
  ));
}

libghostty_cpp_result libghostty_cpp_render_state_row_iterator_set_dirty(
  libghostty_cpp_render_state_row_iterator *row_iterator,
  bool dirty
) {
  if (row_iterator == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(ghostty_render_state_row_set(
    row_iterator->inner,
    GHOSTTY_RENDER_STATE_ROW_OPTION_DIRTY,
    &dirty
  ));
}

libghostty_cpp_result libghostty_cpp_render_state_row_iterator_row(
  libghostty_cpp_render_state_row_iterator *row_iterator,
  libghostty_cpp_row *out_row
) {
  if (row_iterator == NULL || out_row == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyRow row = 0;
  const GhosttyResult result = ghostty_render_state_row_get(
    row_iterator->inner,
    GHOSTTY_RENDER_STATE_ROW_DATA_RAW,
    &row
  );
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  *out_row = row;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_render_state_row_cells_new(
  libghostty_cpp_render_state_row_cells **row_cells
) {
  if (row_cells == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *row_cells = NULL;

  libghostty_cpp_render_state_row_cells *wrapper = calloc(1, sizeof(*wrapper));
  if (wrapper == NULL) {
    return LIBGHOSTTY_CPP_RESULT_OUT_OF_MEMORY;
  }

  const GhosttyResult result = ghostty_render_state_row_cells_new(NULL, &wrapper->inner);
  if (result != GHOSTTY_SUCCESS) {
    free(wrapper);
    return libghostty_cpp_translate_result(result);
  }

  *row_cells = wrapper;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

void libghostty_cpp_render_state_row_cells_free(
  libghostty_cpp_render_state_row_cells *row_cells
) {
  if (row_cells == NULL) {
    return;
  }

  ghostty_render_state_row_cells_free(row_cells->inner);
  free(row_cells);
}

libghostty_cpp_result libghostty_cpp_render_state_row_cells_update(
  libghostty_cpp_render_state_row_iterator *row_iterator,
  libghostty_cpp_render_state_row_cells *row_cells
) {
  if (row_iterator == NULL || row_cells == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(ghostty_render_state_row_get(
    row_iterator->inner,
    GHOSTTY_RENDER_STATE_ROW_DATA_CELLS,
    &row_cells->inner
  ));
}

bool libghostty_cpp_render_state_row_cells_next(
  libghostty_cpp_render_state_row_cells *row_cells
) {
  if (row_cells == NULL) {
    return false;
  }

  return ghostty_render_state_row_cells_next(row_cells->inner);
}

libghostty_cpp_result libghostty_cpp_render_state_row_cells_select(
  libghostty_cpp_render_state_row_cells *row_cells,
  uint16_t x
) {
  if (row_cells == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_render_state_row_cells_select(row_cells->inner, x)
  );
}

libghostty_cpp_result libghostty_cpp_render_state_row_cells_cell(
  libghostty_cpp_render_state_row_cells *row_cells,
  libghostty_cpp_cell *out_cell
) {
  if (row_cells == NULL || out_cell == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyCell cell = 0;
  const GhosttyResult result = ghostty_render_state_row_cells_get(
    row_cells->inner,
    GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_RAW,
    &cell
  );
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  *out_cell = cell;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_render_state_row_cells_style(
  libghostty_cpp_render_state_row_cells *row_cells,
  libghostty_cpp_style *out_style
) {
  if (row_cells == NULL || out_style == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyStyle style = GHOSTTY_INIT_SIZED(GhosttyStyle);
  const GhosttyResult result = ghostty_render_state_row_cells_get(
    row_cells->inner,
    GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_STYLE,
    &style
  );
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  return translate_style(style, out_style);
}

libghostty_cpp_result libghostty_cpp_render_state_row_cells_fg_color(
  libghostty_cpp_render_state_row_cells *row_cells,
  bool *out_has_value,
  libghostty_cpp_rgb_color *out_color
) {
  if (row_cells == NULL || out_has_value == NULL || out_color == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyColorRgb color = {0};
  const GhosttyResult result = ghostty_render_state_row_cells_get(
    row_cells->inner,
    GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_FG_COLOR,
    &color
  );
  if (result == GHOSTTY_INVALID_VALUE) {
    *out_has_value = false;
    return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  *out_has_value = true;
  *out_color = translate_color(color);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_render_state_row_cells_bg_color(
  libghostty_cpp_render_state_row_cells *row_cells,
  bool *out_has_value,
  libghostty_cpp_rgb_color *out_color
) {
  if (row_cells == NULL || out_has_value == NULL || out_color == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyColorRgb color = {0};
  const GhosttyResult result = ghostty_render_state_row_cells_get(
    row_cells->inner,
    GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_BG_COLOR,
    &color
  );
  if (result == GHOSTTY_INVALID_VALUE) {
    *out_has_value = false;
    return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  *out_has_value = true;
  *out_color = translate_color(color);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_render_state_row_cells_graphemes_len(
  libghostty_cpp_render_state_row_cells *row_cells,
  size_t *out_len
) {
  if (row_cells == NULL || out_len == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  uint32_t len = 0;
  const GhosttyResult result = ghostty_render_state_row_cells_get(
    row_cells->inner,
    GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_GRAPHEMES_LEN,
    &len
  );
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  *out_len = len;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_render_state_row_cells_graphemes(
  libghostty_cpp_render_state_row_cells *row_cells,
  uint32_t *out_codepoints
) {
  if (row_cells == NULL || out_codepoints == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(ghostty_render_state_row_cells_get(
    row_cells->inner,
    GHOSTTY_RENDER_STATE_ROW_CELLS_DATA_GRAPHEMES_BUF,
    out_codepoints
  ));
}

libghostty_cpp_result libghostty_cpp_row_is_wrapped(
  libghostty_cpp_row row,
  bool *out_is_wrapped
) {
  return row_get_bool(row, GHOSTTY_ROW_DATA_WRAP, out_is_wrapped);
}

libghostty_cpp_result libghostty_cpp_row_is_wrap_continuation(
  libghostty_cpp_row row,
  bool *out_is_wrap_continuation
) {
  return row_get_bool(
    row,
    GHOSTTY_ROW_DATA_WRAP_CONTINUATION,
    out_is_wrap_continuation
  );
}

libghostty_cpp_result libghostty_cpp_row_has_grapheme_cluster(
  libghostty_cpp_row row,
  bool *out_has_grapheme_cluster
) {
  return row_get_bool(row, GHOSTTY_ROW_DATA_GRAPHEME, out_has_grapheme_cluster);
}

libghostty_cpp_result libghostty_cpp_row_is_styled(
  libghostty_cpp_row row,
  bool *out_is_styled
) {
  return row_get_bool(row, GHOSTTY_ROW_DATA_STYLED, out_is_styled);
}

libghostty_cpp_result libghostty_cpp_row_has_hyperlink(
  libghostty_cpp_row row,
  bool *out_has_hyperlink
) {
  return row_get_bool(row, GHOSTTY_ROW_DATA_HYPERLINK, out_has_hyperlink);
}

libghostty_cpp_result libghostty_cpp_row_get_semantic_prompt(
  libghostty_cpp_row row,
  libghostty_cpp_row_semantic_prompt *out_semantic_prompt
) {
  if (out_semantic_prompt == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyRowSemanticPrompt semantic_prompt = GHOSTTY_ROW_SEMANTIC_NONE;
  const GhosttyResult result = ghostty_row_get(
    (GhosttyRow) row,
    GHOSTTY_ROW_DATA_SEMANTIC_PROMPT,
    &semantic_prompt
  );
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  return translate_row_semantic_prompt(semantic_prompt, out_semantic_prompt);
}

libghostty_cpp_result libghostty_cpp_row_has_kitty_virtual_placeholder(
  libghostty_cpp_row row,
  bool *out_has_kitty_virtual_placeholder
) {
  return row_get_bool(
    row,
    GHOSTTY_ROW_DATA_KITTY_VIRTUAL_PLACEHOLDER,
    out_has_kitty_virtual_placeholder
  );
}

libghostty_cpp_result libghostty_cpp_row_is_dirty(
  libghostty_cpp_row row,
  bool *out_is_dirty
) {
  return row_get_bool(row, GHOSTTY_ROW_DATA_DIRTY, out_is_dirty);
}

libghostty_cpp_result libghostty_cpp_cell_codepoint(
  libghostty_cpp_cell cell,
  uint32_t *out_codepoint
) {
  return cell_get_u32(cell, GHOSTTY_CELL_DATA_CODEPOINT, out_codepoint);
}

libghostty_cpp_result libghostty_cpp_cell_get_content_tag(
  libghostty_cpp_cell cell,
  libghostty_cpp_cell_content_tag *out_content_tag
) {
  if (out_content_tag == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyCellContentTag content_tag = GHOSTTY_CELL_CONTENT_CODEPOINT;
  const GhosttyResult result = ghostty_cell_get(
    (GhosttyCell) cell,
    GHOSTTY_CELL_DATA_CONTENT_TAG,
    &content_tag
  );
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  return translate_cell_content_tag(content_tag, out_content_tag);
}

libghostty_cpp_result libghostty_cpp_cell_get_wide(
  libghostty_cpp_cell cell,
  libghostty_cpp_cell_wide *out_wide
) {
  if (out_wide == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyCellWide wide = GHOSTTY_CELL_WIDE_NARROW;
  const GhosttyResult result = ghostty_cell_get(
    (GhosttyCell) cell,
    GHOSTTY_CELL_DATA_WIDE,
    &wide
  );
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  return translate_cell_wide(wide, out_wide);
}

libghostty_cpp_result libghostty_cpp_cell_has_text(
  libghostty_cpp_cell cell,
  bool *out_has_text
) {
  return cell_get_bool(cell, GHOSTTY_CELL_DATA_HAS_TEXT, out_has_text);
}

libghostty_cpp_result libghostty_cpp_cell_has_styling(
  libghostty_cpp_cell cell,
  bool *out_has_styling
) {
  return cell_get_bool(cell, GHOSTTY_CELL_DATA_HAS_STYLING, out_has_styling);
}

libghostty_cpp_result libghostty_cpp_cell_style_id(
  libghostty_cpp_cell cell,
  uint16_t *out_style_id
) {
  return cell_get_u16(cell, GHOSTTY_CELL_DATA_STYLE_ID, out_style_id);
}

libghostty_cpp_result libghostty_cpp_cell_has_hyperlink(
  libghostty_cpp_cell cell,
  bool *out_has_hyperlink
) {
  return cell_get_bool(cell, GHOSTTY_CELL_DATA_HAS_HYPERLINK, out_has_hyperlink);
}

libghostty_cpp_result libghostty_cpp_cell_is_protected(
  libghostty_cpp_cell cell,
  bool *out_is_protected
) {
  return cell_get_bool(cell, GHOSTTY_CELL_DATA_PROTECTED, out_is_protected);
}

libghostty_cpp_result libghostty_cpp_cell_get_semantic_content(
  libghostty_cpp_cell cell,
  libghostty_cpp_cell_semantic_content *out_semantic_content
) {
  if (out_semantic_content == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyCellSemanticContent semantic_content = GHOSTTY_CELL_SEMANTIC_OUTPUT;
  const GhosttyResult result = ghostty_cell_get(
    (GhosttyCell) cell,
    GHOSTTY_CELL_DATA_SEMANTIC_CONTENT,
    &semantic_content
  );
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  return translate_cell_semantic_content(semantic_content, out_semantic_content);
}

libghostty_cpp_result libghostty_cpp_cell_bg_color_palette(
  libghostty_cpp_cell cell,
  uint8_t *out_palette
) {
  if (out_palette == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_cell_get((GhosttyCell) cell, GHOSTTY_CELL_DATA_COLOR_PALETTE, out_palette)
  );
}

libghostty_cpp_result libghostty_cpp_cell_bg_color_rgb(
  libghostty_cpp_cell cell,
  libghostty_cpp_rgb_color *out_color
) {
  if (out_color == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyColorRgb color = {0};
  const GhosttyResult result = ghostty_cell_get(
    (GhosttyCell) cell,
    GHOSTTY_CELL_DATA_COLOR_RGB,
    &color
  );
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  *out_color = translate_color(color);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}
