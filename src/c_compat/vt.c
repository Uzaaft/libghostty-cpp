#include "c_compat/vt.h"

#include "c_compat/internal.h"
#include "c_compat/render.h"

#include <stdlib.h>

static libghostty_cpp_terminal* terminal_from_userdata(
  GhosttyTerminal terminal,
  void* userdata
) {
  libghostty_cpp_terminal* wrapper = userdata;
  if (wrapper == NULL || wrapper->inner != terminal) {
    return NULL;
  }

  return wrapper;
}

static libghostty_cpp_result set_terminal_option(
  libghostty_cpp_terminal* terminal,
  GhosttyTerminalOption option,
  const void* value
) {
  if (terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_terminal_set(terminal->inner, option, value)
  );
}

typedef union libghostty_cpp_terminal_option_value {
  const void* value;
  GhosttyTerminalWritePtyFn pty_write;
  GhosttyTerminalBellFn bell;
  GhosttyTerminalEnquiryFn enquiry;
  GhosttyTerminalSizeFn size;
  GhosttyTerminalDeviceAttributesFn device_attributes;
  GhosttyTerminalXtversionFn xtversion;
  GhosttyTerminalTitleChangedFn title_changed;
  GhosttyTerminalColorSchemeFn color_scheme;
} libghostty_cpp_terminal_option_value;

_Static_assert(
  sizeof(const void*) == sizeof(GhosttyTerminalWritePtyFn),
  "Ghostty callback pointers must fit in a const void *"
);
_Static_assert(
  sizeof(const void*) == sizeof(GhosttyTerminalBellFn),
  "Ghostty callback pointers must fit in a const void *"
);
_Static_assert(
  sizeof(const void*) == sizeof(GhosttyTerminalEnquiryFn),
  "Ghostty callback pointers must fit in a const void *"
);
_Static_assert(
  sizeof(const void*) == sizeof(GhosttyTerminalSizeFn),
  "Ghostty callback pointers must fit in a const void *"
);
_Static_assert(
  sizeof(const void*) == sizeof(GhosttyTerminalTitleChangedFn),
  "Ghostty callback pointers must fit in a const void *"
);
_Static_assert(
  sizeof(const void*) == sizeof(GhosttyTerminalDeviceAttributesFn),
  "Ghostty callback pointers must fit in a const void *"
);
_Static_assert(
  sizeof(const void*) == sizeof(GhosttyTerminalXtversionFn),
  "Ghostty callback pointers must fit in a const void *"
);
_Static_assert(
  sizeof(const void*) == sizeof(GhosttyTerminalColorSchemeFn),
  "Ghostty callback pointers must fit in a const void *"
);

// Ghostty stores callback hooks as `const void *`, so keep the ABI-specific
// conversion in one checked place instead of spreading raw casts around.
static const void* option_value_from_pty_write(
  GhosttyTerminalWritePtyFn callback
) {
  libghostty_cpp_terminal_option_value option_value = {.pty_write = callback};
  return option_value.value;
}

static const void* option_value_from_bell(
  GhosttyTerminalBellFn callback
) {
  libghostty_cpp_terminal_option_value option_value = {.bell = callback};
  return option_value.value;
}

static const void* option_value_from_enquiry(
  GhosttyTerminalEnquiryFn callback
) {
  libghostty_cpp_terminal_option_value option_value = {.enquiry = callback};
  return option_value.value;
}

static const void* option_value_from_size(
  GhosttyTerminalSizeFn callback
) {
  libghostty_cpp_terminal_option_value option_value = {.size = callback};
  return option_value.value;
}

static const void* option_value_from_device_attributes(
  GhosttyTerminalDeviceAttributesFn callback
) {
  libghostty_cpp_terminal_option_value option_value = {
    .device_attributes = callback,
  };
  return option_value.value;
}

static const void* option_value_from_xtversion(
  GhosttyTerminalXtversionFn callback
) {
  libghostty_cpp_terminal_option_value option_value = {.xtversion = callback};
  return option_value.value;
}

static const void* option_value_from_title_changed(
  GhosttyTerminalTitleChangedFn callback
) {
  libghostty_cpp_terminal_option_value option_value = {.title_changed = callback};
  return option_value.value;
}

static const void* option_value_from_color_scheme(
  GhosttyTerminalColorSchemeFn callback
) {
  libghostty_cpp_terminal_option_value option_value = {
    .color_scheme = callback,
  };
  return option_value.value;
}

static GhosttyString to_ghostty_string(libghostty_cpp_string value) {
  if (value.data == NULL && value.len != 0) {
    return (GhosttyString) {.ptr = NULL, .len = 0};
  }

  return (GhosttyString) {
    .ptr = value.data,
    .len = value.len,
  };
}

static libghostty_cpp_rgb_color translate_color(GhosttyColorRgb color) {
  return (libghostty_cpp_rgb_color) {
    .r = color.r,
    .g = color.g,
    .b = color.b,
  };
}

static GhosttyColorRgb to_ghostty_color_rgb(libghostty_cpp_rgb_color color) {
  return (GhosttyColorRgb) {
    .r = color.r,
    .g = color.g,
    .b = color.b,
  };
}

static libghostty_cpp_result translate_underline(
  GhosttySgrUnderline underline,
  libghostty_cpp_underline* out_underline
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
  libghostty_cpp_style_color* out_color
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
  libghostty_cpp_style* out_style
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

static bool to_ghostty_color_scheme(
  libghostty_cpp_color_scheme scheme,
  GhosttyColorScheme* out_scheme
) {
  if (out_scheme == NULL) {
    return false;
  }

  switch (scheme) {
    case LIBGHOSTTY_CPP_COLOR_SCHEME_LIGHT:
      *out_scheme = GHOSTTY_COLOR_SCHEME_LIGHT;
      return true;
    case LIBGHOSTTY_CPP_COLOR_SCHEME_DARK:
      *out_scheme = GHOSTTY_COLOR_SCHEME_DARK;
      return true;
  }

  return false;
}

static bool to_ghostty_device_attributes(
  libghostty_cpp_device_attributes value,
  GhosttyDeviceAttributes* out_attributes
) {
  if (out_attributes == NULL || value.primary.num_features > 64) {
    return false;
  }

  *out_attributes = (GhosttyDeviceAttributes) {
    .primary = {
      .conformance_level = value.primary.conformance_level,
      .num_features = value.primary.num_features,
    },
    .secondary = {
      .device_type = value.secondary.device_type,
      .firmware_version = value.secondary.firmware_version,
      .rom_cartridge = value.secondary.rom_cartridge,
    },
    .tertiary = {
      .unit_id = value.tertiary.unit_id,
    },
  };

  for (size_t i = 0; i < value.primary.num_features; ++i) {
    out_attributes->primary.features[i] = value.primary.features[i];
  }

  return true;
}

static bool from_ghostty_terminal_screen(
  GhosttyTerminalScreen screen,
  libghostty_cpp_terminal_screen* out_screen
) {
  if (out_screen == NULL) {
    return false;
  }

  switch (screen) {
    case GHOSTTY_TERMINAL_SCREEN_PRIMARY:
      *out_screen = LIBGHOSTTY_CPP_TERMINAL_SCREEN_PRIMARY;
      return true;
    case GHOSTTY_TERMINAL_SCREEN_ALTERNATE:
      *out_screen = LIBGHOSTTY_CPP_TERMINAL_SCREEN_ALTERNATE;
      return true;
  }

  return false;
}

static bool to_ghostty_point(
  libghostty_cpp_point point,
  GhosttyPoint* out_point
) {
  if (out_point == NULL) {
    return false;
  }

  *out_point = (GhosttyPoint) {
    .tag = GHOSTTY_POINT_TAG_ACTIVE,
    .value = {
      .coordinate = {
        .x = point.value.coordinate.x,
        .y = point.value.coordinate.y,
      },
    },
  };

  switch (point.tag) {
    case LIBGHOSTTY_CPP_POINT_ACTIVE:
      out_point->tag = GHOSTTY_POINT_TAG_ACTIVE;
      return true;
    case LIBGHOSTTY_CPP_POINT_VIEWPORT:
      out_point->tag = GHOSTTY_POINT_TAG_VIEWPORT;
      return true;
    case LIBGHOSTTY_CPP_POINT_SCREEN:
      out_point->tag = GHOSTTY_POINT_TAG_SCREEN;
      return true;
    case LIBGHOSTTY_CPP_POINT_HISTORY:
      out_point->tag = GHOSTTY_POINT_TAG_HISTORY;
      return true;
  }

  return false;
}

static libghostty_cpp_result resolve_grid_ref(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_point point,
  GhosttyGridRef* out_ref
) {
  if (terminal == NULL || out_ref == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyPoint raw_point = {
    .tag = GHOSTTY_POINT_TAG_ACTIVE,
    .value = {
      .coordinate = {0, 0},
    },
  };
  if (!to_ghostty_point(point, &raw_point)) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *out_ref = GHOSTTY_INIT_SIZED(GhosttyGridRef);
  return libghostty_cpp_translate_result(
    ghostty_terminal_grid_ref(terminal->inner, raw_point, out_ref)
  );
}

static void on_pty_write(
  GhosttyTerminal terminal,
  void* userdata,
  const uint8_t* data,
  size_t len
) {
  libghostty_cpp_terminal* wrapper = terminal_from_userdata(terminal, userdata);
  if (wrapper == NULL || wrapper->pty_write == NULL) {
    return;
  }

  wrapper->pty_write(wrapper, wrapper->callback_userdata, data, len);
}

static void on_bell(
  GhosttyTerminal terminal,
  void* userdata
) {
  libghostty_cpp_terminal* wrapper = terminal_from_userdata(terminal, userdata);
  if (wrapper == NULL || wrapper->bell == NULL) {
    return;
  }

  wrapper->bell(wrapper, wrapper->callback_userdata);
}

static GhosttyString on_enquiry(
  GhosttyTerminal terminal,
  void* userdata
) {
  libghostty_cpp_terminal* wrapper = terminal_from_userdata(terminal, userdata);
  if (wrapper == NULL || wrapper->enquiry == NULL) {
    return (GhosttyString) {.ptr = NULL, .len = 0};
  }

  return to_ghostty_string(wrapper->enquiry(wrapper, wrapper->callback_userdata));
}

static bool on_size(
  GhosttyTerminal terminal,
  void* userdata,
  GhosttySizeReportSize* out_size
) {
  libghostty_cpp_terminal* wrapper = terminal_from_userdata(terminal, userdata);
  if (wrapper == NULL || wrapper->size == NULL || out_size == NULL) {
    return false;
  }

  libghostty_cpp_size_report_size size = {0};
  if (!wrapper->size(wrapper, wrapper->callback_userdata, &size)) {
    return false;
  }

  *out_size = (GhosttySizeReportSize) {
    .rows = size.rows,
    .columns = size.columns,
    .cell_width = size.cell_width,
    .cell_height = size.cell_height,
  };
  return true;
}

static bool on_device_attributes(
  GhosttyTerminal terminal,
  void* userdata,
  GhosttyDeviceAttributes* out_attributes
) {
  libghostty_cpp_terminal* wrapper = terminal_from_userdata(terminal, userdata);
  if (wrapper == NULL
      || wrapper->device_attributes == NULL
      || out_attributes == NULL) {
    return false;
  }

  libghostty_cpp_device_attributes attributes = {0};
  if (!wrapper->device_attributes(
        wrapper,
        wrapper->callback_userdata,
        &attributes
      )) {
    return false;
  }

  return to_ghostty_device_attributes(attributes, out_attributes);
}

static GhosttyString on_xtversion(
  GhosttyTerminal terminal,
  void* userdata
) {
  libghostty_cpp_terminal* wrapper = terminal_from_userdata(terminal, userdata);
  if (wrapper == NULL || wrapper->xtversion == NULL) {
    return (GhosttyString) {.ptr = NULL, .len = 0};
  }

  return to_ghostty_string(
    wrapper->xtversion(wrapper, wrapper->callback_userdata)
  );
}

static void on_title_changed(
  GhosttyTerminal terminal,
  void* userdata
) {
  libghostty_cpp_terminal* wrapper = terminal_from_userdata(terminal, userdata);
  if (wrapper == NULL || wrapper->title_changed == NULL) {
    return;
  }

  wrapper->title_changed(wrapper, wrapper->callback_userdata);
}

static bool on_color_scheme(
  GhosttyTerminal terminal,
  void* userdata,
  GhosttyColorScheme* out_scheme
) {
  libghostty_cpp_terminal* wrapper = terminal_from_userdata(terminal, userdata);
  if (wrapper == NULL || wrapper->color_scheme == NULL || out_scheme == NULL) {
    return false;
  }

  libghostty_cpp_color_scheme scheme = LIBGHOSTTY_CPP_COLOR_SCHEME_LIGHT;
  if (!wrapper->color_scheme(wrapper, wrapper->callback_userdata, &scheme)) {
    return false;
  }

  return to_ghostty_color_scheme(scheme, out_scheme);
}

static libghostty_cpp_result get_u16(
  const libghostty_cpp_terminal* terminal,
  GhosttyTerminalData data,
  uint16_t* out_value
) {
  if (terminal == NULL || out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_terminal_get(terminal->inner, data, out_value)
  );
}

static libghostty_cpp_result get_bool(
  const libghostty_cpp_terminal* terminal,
  GhosttyTerminalData data,
  bool* out_value
) {
  if (terminal == NULL || out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_terminal_get(terminal->inner, data, out_value)
  );
}

static libghostty_cpp_result get_u32(
  const libghostty_cpp_terminal* terminal,
  GhosttyTerminalData data,
  uint32_t* out_value
) {
  if (terminal == NULL || out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_terminal_get(terminal->inner, data, out_value)
  );
}

static libghostty_cpp_result get_size(
  const libghostty_cpp_terminal* terminal,
  GhosttyTerminalData data,
  size_t* out_value
) {
  if (terminal == NULL || out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_terminal_get(terminal->inner, data, out_value)
  );
}

static libghostty_cpp_result get_string(
  const libghostty_cpp_terminal* terminal,
  GhosttyTerminalData data,
  libghostty_cpp_string* out_value
) {
  if (terminal == NULL || out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_terminal_get(terminal->inner, data, out_value)
  );
}

static libghostty_cpp_result get_optional_color(
  const libghostty_cpp_terminal* terminal,
  GhosttyTerminalData data,
  bool* out_has_value,
  libghostty_cpp_rgb_color* out_color
) {
  if (terminal == NULL || out_has_value == NULL || out_color == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyColorRgb color = {0};
  const GhosttyResult result = ghostty_terminal_get(terminal->inner, data, &color);
  if (result == GHOSTTY_NO_VALUE) {
    *out_has_value = false;
    *out_color = (libghostty_cpp_rgb_color) {0, 0, 0};
    return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  *out_has_value = true;
  *out_color = translate_color(color);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

static libghostty_cpp_result get_palette(
  const libghostty_cpp_terminal* terminal,
  GhosttyTerminalData data,
  libghostty_cpp_rgb_color* out_palette
) {
  if (terminal == NULL || out_palette == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyColorRgb colors[256] = {0};
  const GhosttyResult result = ghostty_terminal_get(terminal->inner, data, colors);
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  for (size_t i = 0; i < 256; ++i) {
    out_palette[i] = translate_color(colors[i]);
  }

  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

static libghostty_cpp_result set_string_option(
  libghostty_cpp_terminal* terminal,
  GhosttyTerminalOption option,
  const libghostty_cpp_string* value
) {
  if (terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyString raw = {0};
  const GhosttyString* raw_ptr = NULL;
  if (value != NULL) {
    raw = to_ghostty_string(*value);
    raw_ptr = &raw;
  }

  return set_terminal_option(terminal, option, raw_ptr);
}

static libghostty_cpp_result set_optional_color_option(
  libghostty_cpp_terminal* terminal,
  GhosttyTerminalOption option,
  const libghostty_cpp_rgb_color* color
) {
  if (terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyColorRgb raw = {0};
  const GhosttyColorRgb* raw_ptr = NULL;
  if (color != NULL) {
    raw = to_ghostty_color_rgb(*color);
    raw_ptr = &raw;
  }

  return set_terminal_option(terminal, option, raw_ptr);
}

static libghostty_cpp_result set_palette_option(
  libghostty_cpp_terminal* terminal,
  GhosttyTerminalOption option,
  const libghostty_cpp_rgb_color* colors
) {
  if (terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyColorRgb raw_colors[256] = {0};
  const GhosttyColorRgb* raw_ptr = NULL;
  if (colors != NULL) {
    for (size_t i = 0; i < 256; ++i) {
      raw_colors[i] = to_ghostty_color_rgb(colors[i]);
    }
    raw_ptr = raw_colors;
  }

  return set_terminal_option(terminal, option, raw_ptr);
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
    return libghostty_cpp_translate_result(result);
  }

  const GhosttyResult userdata_result = ghostty_terminal_set(
    wrapper->inner,
    GHOSTTY_TERMINAL_OPT_USERDATA,
    wrapper
  );
  if (userdata_result != GHOSTTY_SUCCESS) {
    ghostty_terminal_free(wrapper->inner);
    free(wrapper);
    return libghostty_cpp_translate_result(userdata_result);
  }

  *terminal = wrapper;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_terminal_set_callback_userdata(
  libghostty_cpp_terminal* terminal,
  void* userdata
) {
  if (terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  terminal->callback_userdata = userdata;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

void libghostty_cpp_terminal_free(libghostty_cpp_terminal* terminal) {
  if (terminal == NULL) {
    return;
  }

  ghostty_terminal_free(terminal->inner);
  free(terminal);
}

libghostty_cpp_result libghostty_cpp_terminal_on_pty_write(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_pty_write_fn callback
) {
  if (terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  terminal->pty_write = callback;
  return set_terminal_option(
    terminal,
    GHOSTTY_TERMINAL_OPT_WRITE_PTY,
    callback != NULL ? option_value_from_pty_write(on_pty_write) : NULL
  );
}

libghostty_cpp_result libghostty_cpp_terminal_on_bell(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_bell_fn callback
) {
  if (terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  terminal->bell = callback;
  return set_terminal_option(
    terminal,
    GHOSTTY_TERMINAL_OPT_BELL,
    callback != NULL ? option_value_from_bell(on_bell) : NULL
  );
}

libghostty_cpp_result libghostty_cpp_terminal_on_enquiry(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_enquiry_fn callback
) {
  if (terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  terminal->enquiry = callback;
  return set_terminal_option(
    terminal,
    GHOSTTY_TERMINAL_OPT_ENQUIRY,
    callback != NULL ? option_value_from_enquiry(on_enquiry) : NULL
  );
}

libghostty_cpp_result libghostty_cpp_terminal_on_size(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_size_fn callback
) {
  if (terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  terminal->size = callback;
  return set_terminal_option(
    terminal,
    GHOSTTY_TERMINAL_OPT_SIZE,
    callback != NULL ? option_value_from_size(on_size) : NULL
  );
}

libghostty_cpp_result libghostty_cpp_terminal_on_device_attributes(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_device_attributes_fn callback
) {
  if (terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  terminal->device_attributes = callback;
  return set_terminal_option(
    terminal,
    GHOSTTY_TERMINAL_OPT_DEVICE_ATTRIBUTES,
    callback != NULL ? option_value_from_device_attributes(on_device_attributes)
                     : NULL
  );
}

libghostty_cpp_result libghostty_cpp_terminal_on_xtversion(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_xtversion_fn callback
) {
  if (terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  terminal->xtversion = callback;
  return set_terminal_option(
    terminal,
    GHOSTTY_TERMINAL_OPT_XTVERSION,
    callback != NULL ? option_value_from_xtversion(on_xtversion) : NULL
  );
}

libghostty_cpp_result libghostty_cpp_terminal_on_title_changed(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_title_changed_fn callback
) {
  if (terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  terminal->title_changed = callback;
  return set_terminal_option(
    terminal,
    GHOSTTY_TERMINAL_OPT_TITLE_CHANGED,
    callback != NULL ? option_value_from_title_changed(on_title_changed) : NULL
  );
}

libghostty_cpp_result libghostty_cpp_terminal_on_color_scheme(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_color_scheme_fn callback
) {
  if (terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  terminal->color_scheme = callback;
  return set_terminal_option(
    terminal,
    GHOSTTY_TERMINAL_OPT_COLOR_SCHEME,
    callback != NULL ? option_value_from_color_scheme(on_color_scheme) : NULL
  );
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

libghostty_cpp_result libghostty_cpp_terminal_mode_get(
  const libghostty_cpp_terminal* terminal,
  uint16_t mode,
  bool* out_value
) {
  if (terminal == NULL || out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_terminal_mode_get(terminal->inner, mode, out_value)
  );
}

libghostty_cpp_result libghostty_cpp_terminal_mode_set(
  libghostty_cpp_terminal* terminal,
  uint16_t mode,
  bool value
) {
  if (terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_terminal_mode_set(terminal->inner, mode, value)
  );
}

libghostty_cpp_result libghostty_cpp_terminal_cursor_pending_wrap(
  const libghostty_cpp_terminal* terminal,
  bool* out_value
) {
  return get_bool(terminal, GHOSTTY_TERMINAL_DATA_CURSOR_PENDING_WRAP, out_value);
}

libghostty_cpp_result libghostty_cpp_terminal_cursor_visible(
  const libghostty_cpp_terminal* terminal,
  bool* out_value
) {
  return get_bool(terminal, GHOSTTY_TERMINAL_DATA_CURSOR_VISIBLE, out_value);
}

libghostty_cpp_result libghostty_cpp_terminal_kitty_keyboard_flags(
  const libghostty_cpp_terminal* terminal,
  uint8_t* out_value
) {
  if (terminal == NULL || out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(
    ghostty_terminal_get(terminal->inner, GHOSTTY_TERMINAL_DATA_KITTY_KEYBOARD_FLAGS, out_value)
  );
}

libghostty_cpp_result libghostty_cpp_terminal_cursor_style(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_style* out_style
) {
  if (terminal == NULL || out_style == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyStyle style = GHOSTTY_INIT_SIZED(GhosttyStyle);
  const GhosttyResult result = ghostty_terminal_get(
    terminal->inner,
    GHOSTTY_TERMINAL_DATA_CURSOR_STYLE,
    &style
  );
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  return translate_style(style, out_style);
}

libghostty_cpp_result libghostty_cpp_terminal_mouse_tracking(
  const libghostty_cpp_terminal* terminal,
  bool* out_value
) {
  return get_bool(terminal, GHOSTTY_TERMINAL_DATA_MOUSE_TRACKING, out_value);
}

libghostty_cpp_result libghostty_cpp_terminal_get_active_screen(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_screen* out_screen
) {
  if (terminal == NULL || out_screen == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyTerminalScreen screen = GHOSTTY_TERMINAL_SCREEN_PRIMARY;
  const libghostty_cpp_result result = libghostty_cpp_translate_result(
    ghostty_terminal_get(terminal->inner, GHOSTTY_TERMINAL_DATA_ACTIVE_SCREEN, &screen)
  );
  if (result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return result;
  }

  if (!from_ghostty_terminal_screen(screen, out_screen)) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_terminal_get_scrollbar(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_scrollbar* out_scrollbar
) {
  if (terminal == NULL || out_scrollbar == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyTerminalScrollbar scrollbar = {0};
  const libghostty_cpp_result result = libghostty_cpp_translate_result(
    ghostty_terminal_get(terminal->inner, GHOSTTY_TERMINAL_DATA_SCROLLBAR, &scrollbar)
  );
  if (result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return result;
  }

  *out_scrollbar = (libghostty_cpp_terminal_scrollbar) {
    .total = scrollbar.total,
    .offset = scrollbar.offset,
    .len = scrollbar.len,
  };
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_terminal_total_rows(
  const libghostty_cpp_terminal* terminal,
  size_t* out_total_rows
) {
  return get_size(terminal, GHOSTTY_TERMINAL_DATA_TOTAL_ROWS, out_total_rows);
}

libghostty_cpp_result libghostty_cpp_terminal_scrollback_rows(
  const libghostty_cpp_terminal* terminal,
  size_t* out_scrollback_rows
) {
  return get_size(
    terminal,
    GHOSTTY_TERMINAL_DATA_SCROLLBACK_ROWS,
    out_scrollback_rows
  );
}

libghostty_cpp_result libghostty_cpp_terminal_width_px(
  const libghostty_cpp_terminal* terminal,
  uint32_t* out_width_px
) {
  return get_u32(terminal, GHOSTTY_TERMINAL_DATA_WIDTH_PX, out_width_px);
}

libghostty_cpp_result libghostty_cpp_terminal_height_px(
  const libghostty_cpp_terminal* terminal,
  uint32_t* out_height_px
) {
  return get_u32(terminal, GHOSTTY_TERMINAL_DATA_HEIGHT_PX, out_height_px);
}

libghostty_cpp_result libghostty_cpp_terminal_color_foreground(
  const libghostty_cpp_terminal* terminal,
  bool* out_has_value,
  libghostty_cpp_rgb_color* out_color
) {
  return get_optional_color(
    terminal,
    GHOSTTY_TERMINAL_DATA_COLOR_FOREGROUND,
    out_has_value,
    out_color
  );
}

libghostty_cpp_result libghostty_cpp_terminal_color_foreground_default(
  const libghostty_cpp_terminal* terminal,
  bool* out_has_value,
  libghostty_cpp_rgb_color* out_color
) {
  return get_optional_color(
    terminal,
    GHOSTTY_TERMINAL_DATA_COLOR_FOREGROUND_DEFAULT,
    out_has_value,
    out_color
  );
}

libghostty_cpp_result libghostty_cpp_terminal_color_background(
  const libghostty_cpp_terminal* terminal,
  bool* out_has_value,
  libghostty_cpp_rgb_color* out_color
) {
  return get_optional_color(
    terminal,
    GHOSTTY_TERMINAL_DATA_COLOR_BACKGROUND,
    out_has_value,
    out_color
  );
}

libghostty_cpp_result libghostty_cpp_terminal_color_background_default(
  const libghostty_cpp_terminal* terminal,
  bool* out_has_value,
  libghostty_cpp_rgb_color* out_color
) {
  return get_optional_color(
    terminal,
    GHOSTTY_TERMINAL_DATA_COLOR_BACKGROUND_DEFAULT,
    out_has_value,
    out_color
  );
}

libghostty_cpp_result libghostty_cpp_terminal_color_cursor(
  const libghostty_cpp_terminal* terminal,
  bool* out_has_value,
  libghostty_cpp_rgb_color* out_color
) {
  return get_optional_color(
    terminal,
    GHOSTTY_TERMINAL_DATA_COLOR_CURSOR,
    out_has_value,
    out_color
  );
}

libghostty_cpp_result libghostty_cpp_terminal_color_cursor_default(
  const libghostty_cpp_terminal* terminal,
  bool* out_has_value,
  libghostty_cpp_rgb_color* out_color
) {
  return get_optional_color(
    terminal,
    GHOSTTY_TERMINAL_DATA_COLOR_CURSOR_DEFAULT,
    out_has_value,
    out_color
  );
}

libghostty_cpp_result libghostty_cpp_terminal_color_palette(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_rgb_color* out_palette
) {
  return get_palette(terminal, GHOSTTY_TERMINAL_DATA_COLOR_PALETTE, out_palette);
}

libghostty_cpp_result libghostty_cpp_terminal_color_palette_default(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_rgb_color* out_palette
) {
  return get_palette(
    terminal,
    GHOSTTY_TERMINAL_DATA_COLOR_PALETTE_DEFAULT,
    out_palette
  );
}

libghostty_cpp_result libghostty_cpp_terminal_set_title(
  libghostty_cpp_terminal* terminal,
  const libghostty_cpp_string* title
) {
  return set_string_option(terminal, GHOSTTY_TERMINAL_OPT_TITLE, title);
}

libghostty_cpp_result libghostty_cpp_terminal_set_pwd(
  libghostty_cpp_terminal* terminal,
  const libghostty_cpp_string* pwd
) {
  return set_string_option(terminal, GHOSTTY_TERMINAL_OPT_PWD, pwd);
}

libghostty_cpp_result libghostty_cpp_terminal_set_color_foreground_default(
  libghostty_cpp_terminal* terminal,
  const libghostty_cpp_rgb_color* color
) {
  return set_optional_color_option(terminal, GHOSTTY_TERMINAL_OPT_COLOR_FOREGROUND, color);
}

libghostty_cpp_result libghostty_cpp_terminal_set_color_background_default(
  libghostty_cpp_terminal* terminal,
  const libghostty_cpp_rgb_color* color
) {
  return set_optional_color_option(terminal, GHOSTTY_TERMINAL_OPT_COLOR_BACKGROUND, color);
}

libghostty_cpp_result libghostty_cpp_terminal_set_color_cursor_default(
  libghostty_cpp_terminal* terminal,
  const libghostty_cpp_rgb_color* color
) {
  return set_optional_color_option(terminal, GHOSTTY_TERMINAL_OPT_COLOR_CURSOR, color);
}

libghostty_cpp_result libghostty_cpp_terminal_set_color_palette_default(
  libghostty_cpp_terminal* terminal,
  const libghostty_cpp_rgb_color* colors
) {
  return set_palette_option(terminal, GHOSTTY_TERMINAL_OPT_COLOR_PALETTE, colors);
}

libghostty_cpp_result libghostty_cpp_terminal_grid_ref_snapshot(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_point point,
  libghostty_cpp_grid_ref_snapshot* out_snapshot
) {
  if (out_snapshot == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyGridRef ref = GHOSTTY_INIT_SIZED(GhosttyGridRef);
  const libghostty_cpp_result ref_result = resolve_grid_ref(terminal, point, &ref);
  if (ref_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return ref_result;
  }

  GhosttyRow row = 0;
  const GhosttyResult row_result = ghostty_grid_ref_row(&ref, &row);
  if (row_result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(row_result);
  }

  const libghostty_cpp_result wrap_result =
    libghostty_cpp_row_is_wrapped((libghostty_cpp_row) row, &out_snapshot->row_is_wrapped);
  if (wrap_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return wrap_result;
  }

  GhosttyCell cell = 0;
  const GhosttyResult cell_result = ghostty_grid_ref_cell(&ref, &cell);
  if (cell_result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(cell_result);
  }

  const libghostty_cpp_result has_text_result = libghostty_cpp_cell_has_text(
    (libghostty_cpp_cell) cell,
    &out_snapshot->cell_has_text
  );
  if (has_text_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return has_text_result;
  }

  libghostty_cpp_cell_wide wide = LIBGHOSTTY_CPP_CELL_WIDE_NARROW;
  const libghostty_cpp_result wide_result = libghostty_cpp_cell_get_wide(
    (libghostty_cpp_cell) cell,
    &wide
  );
  if (wide_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return wide_result;
  }

  switch (wide) {
    case LIBGHOSTTY_CPP_CELL_WIDE_NARROW:
      out_snapshot->cell_wide = LIBGHOSTTY_CPP_GRID_CELL_WIDE_NARROW;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_CELL_WIDE_WIDE:
      out_snapshot->cell_wide = LIBGHOSTTY_CPP_GRID_CELL_WIDE_WIDE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_CELL_WIDE_SPACER_TAIL:
      out_snapshot->cell_wide = LIBGHOSTTY_CPP_GRID_CELL_WIDE_SPACER_TAIL;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_CELL_WIDE_SPACER_HEAD:
      out_snapshot->cell_wide = LIBGHOSTTY_CPP_GRID_CELL_WIDE_SPACER_HEAD;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

libghostty_cpp_result libghostty_cpp_terminal_grid_ref_row(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_point point,
  uint64_t* out_row
) {
  if (out_row == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyGridRef ref = GHOSTTY_INIT_SIZED(GhosttyGridRef);
  const libghostty_cpp_result ref_result = resolve_grid_ref(terminal, point, &ref);
  if (ref_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return ref_result;
  }

  GhosttyRow row = 0;
  const GhosttyResult result = ghostty_grid_ref_row(&ref, &row);
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  *out_row = (uint64_t) row;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_terminal_grid_ref_cell(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_point point,
  uint64_t* out_cell
) {
  if (out_cell == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyGridRef ref = GHOSTTY_INIT_SIZED(GhosttyGridRef);
  const libghostty_cpp_result ref_result = resolve_grid_ref(terminal, point, &ref);
  if (ref_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return ref_result;
  }

  GhosttyCell cell = 0;
  const GhosttyResult result = ghostty_grid_ref_cell(&ref, &cell);
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  *out_cell = (uint64_t) cell;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_terminal_grid_ref_style(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_point point,
  libghostty_cpp_style* out_style
) {
  if (out_style == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyGridRef ref = GHOSTTY_INIT_SIZED(GhosttyGridRef);
  const libghostty_cpp_result ref_result = resolve_grid_ref(terminal, point, &ref);
  if (ref_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return ref_result;
  }

  GhosttyStyle style = GHOSTTY_INIT_SIZED(GhosttyStyle);
  const GhosttyResult result = ghostty_grid_ref_style(&ref, &style);
  if (result != GHOSTTY_SUCCESS) {
    return libghostty_cpp_translate_result(result);
  }

  return translate_style(style, out_style);
}

libghostty_cpp_result libghostty_cpp_terminal_grid_ref_graphemes_len(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_point point,
  size_t* out_len
) {
  if (out_len == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyGridRef ref = GHOSTTY_INIT_SIZED(GhosttyGridRef);
  const libghostty_cpp_result ref_result = resolve_grid_ref(terminal, point, &ref);
  if (ref_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return ref_result;
  }

  const GhosttyResult result = ghostty_grid_ref_graphemes(&ref, NULL, 0, out_len);
  if (result == GHOSTTY_SUCCESS || result == GHOSTTY_OUT_OF_SPACE) {
    return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return libghostty_cpp_translate_result(result);
}

libghostty_cpp_result libghostty_cpp_terminal_grid_ref_graphemes(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_point point,
  uint32_t* out_codepoints,
  size_t out_codepoints_len
) {
  if (out_codepoints == NULL && out_codepoints_len != 0) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyGridRef ref = GHOSTTY_INIT_SIZED(GhosttyGridRef);
  const libghostty_cpp_result ref_result = resolve_grid_ref(terminal, point, &ref);
  if (ref_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return ref_result;
  }

  size_t len = 0;
  return libghostty_cpp_translate_result(
    ghostty_grid_ref_graphemes(&ref, out_codepoints, out_codepoints_len, &len)
  );
}

void libghostty_cpp_terminal_set_scroll_viewport(
  libghostty_cpp_terminal* terminal,
  libghostty_cpp_terminal_scroll_viewport viewport
) {
  if (terminal == NULL) {
    return;
  }

  GhosttyTerminalScrollViewport raw_viewport = {0};
  switch (viewport.tag) {
    case LIBGHOSTTY_CPP_SCROLL_VIEWPORT_TOP:
      raw_viewport.tag = GHOSTTY_SCROLL_VIEWPORT_TOP;
      break;
    case LIBGHOSTTY_CPP_SCROLL_VIEWPORT_BOTTOM:
      raw_viewport.tag = GHOSTTY_SCROLL_VIEWPORT_BOTTOM;
      break;
    case LIBGHOSTTY_CPP_SCROLL_VIEWPORT_DELTA:
      raw_viewport.tag = GHOSTTY_SCROLL_VIEWPORT_DELTA;
      raw_viewport.value.delta = viewport.value.delta;
      break;
    default:
      return;
  }

  ghostty_terminal_scroll_viewport(terminal->inner, raw_viewport);
}

void libghostty_cpp_terminal_scroll_viewport_delta(
  libghostty_cpp_terminal* terminal,
  ptrdiff_t delta
) {
  libghostty_cpp_terminal_set_scroll_viewport(
    terminal,
    (libghostty_cpp_terminal_scroll_viewport) {
      .tag = LIBGHOSTTY_CPP_SCROLL_VIEWPORT_DELTA,
      .value = {.delta = delta},
    }
  );
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

  return libghostty_cpp_translate_result(ghostty_terminal_resize(
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

libghostty_cpp_result libghostty_cpp_terminal_title(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_string* out_title
) {
  return get_string(terminal, GHOSTTY_TERMINAL_DATA_TITLE, out_title);
}

libghostty_cpp_result libghostty_cpp_terminal_pwd(
  const libghostty_cpp_terminal* terminal,
  libghostty_cpp_string* out_pwd
) {
  return get_string(terminal, GHOSTTY_TERMINAL_DATA_PWD, out_pwd);
}
