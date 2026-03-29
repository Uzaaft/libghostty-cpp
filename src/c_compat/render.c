#include "c_compat/render.h"

#include "c_compat/internal.h"

#include <stddef.h>
#include <stdlib.h>

struct libghostty_cpp_render_state {
  GhosttyRenderState inner;
};

static libghostty_cpp_rgb_color translate_color(GhosttyColorRgb color) {
  return (libghostty_cpp_rgb_color) {
    .r = color.r,
    .g = color.g,
    .b = color.b,
  };
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
