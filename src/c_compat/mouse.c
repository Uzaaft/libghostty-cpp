#include "c_compat/mouse.h"

#include "c_compat/internal.h"

#include <stdlib.h>

struct libghostty_cpp_mouse_encoder {
  GhosttyMouseEncoder inner;
};

struct libghostty_cpp_mouse_event {
  GhosttyMouseEvent inner;
};

static libghostty_cpp_result translate_tracking_mode(
  libghostty_cpp_mouse_tracking_mode value,
  GhosttyMouseTrackingMode* out_value
) {
  if (out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  switch (value) {
    case LIBGHOSTTY_CPP_MOUSE_TRACKING_MODE_NONE:
      *out_value = GHOSTTY_MOUSE_TRACKING_NONE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_TRACKING_MODE_X10:
      *out_value = GHOSTTY_MOUSE_TRACKING_X10;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_TRACKING_MODE_NORMAL:
      *out_value = GHOSTTY_MOUSE_TRACKING_NORMAL;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_TRACKING_MODE_BUTTON:
      *out_value = GHOSTTY_MOUSE_TRACKING_BUTTON;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_TRACKING_MODE_ANY:
      *out_value = GHOSTTY_MOUSE_TRACKING_ANY;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

static libghostty_cpp_result translate_format(
  libghostty_cpp_mouse_format value,
  GhosttyMouseFormat* out_value
) {
  if (out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  switch (value) {
    case LIBGHOSTTY_CPP_MOUSE_FORMAT_X10:
      *out_value = GHOSTTY_MOUSE_FORMAT_X10;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_FORMAT_UTF8:
      *out_value = GHOSTTY_MOUSE_FORMAT_UTF8;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_FORMAT_SGR:
      *out_value = GHOSTTY_MOUSE_FORMAT_SGR;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_FORMAT_URXVT:
      *out_value = GHOSTTY_MOUSE_FORMAT_URXVT;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_FORMAT_SGR_PIXELS:
      *out_value = GHOSTTY_MOUSE_FORMAT_SGR_PIXELS;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

static libghostty_cpp_result translate_action(
  libghostty_cpp_mouse_action value,
  GhosttyMouseAction* out_value
) {
  if (out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  switch (value) {
    case LIBGHOSTTY_CPP_MOUSE_ACTION_PRESS:
      *out_value = GHOSTTY_MOUSE_ACTION_PRESS;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_ACTION_RELEASE:
      *out_value = GHOSTTY_MOUSE_ACTION_RELEASE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_ACTION_MOTION:
      *out_value = GHOSTTY_MOUSE_ACTION_MOTION;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

static libghostty_cpp_result translate_button(
  libghostty_cpp_mouse_button value,
  GhosttyMouseButton* out_value
) {
  if (out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  switch (value) {
    case LIBGHOSTTY_CPP_MOUSE_BUTTON_UNKNOWN:
      *out_value = GHOSTTY_MOUSE_BUTTON_UNKNOWN;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_BUTTON_LEFT:
      *out_value = GHOSTTY_MOUSE_BUTTON_LEFT;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_BUTTON_RIGHT:
      *out_value = GHOSTTY_MOUSE_BUTTON_RIGHT;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_BUTTON_MIDDLE:
      *out_value = GHOSTTY_MOUSE_BUTTON_MIDDLE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_BUTTON_FOUR:
      *out_value = GHOSTTY_MOUSE_BUTTON_FOUR;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_BUTTON_FIVE:
      *out_value = GHOSTTY_MOUSE_BUTTON_FIVE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_BUTTON_SIX:
      *out_value = GHOSTTY_MOUSE_BUTTON_SIX;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_BUTTON_SEVEN:
      *out_value = GHOSTTY_MOUSE_BUTTON_SEVEN;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_BUTTON_EIGHT:
      *out_value = GHOSTTY_MOUSE_BUTTON_EIGHT;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_BUTTON_NINE:
      *out_value = GHOSTTY_MOUSE_BUTTON_NINE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_BUTTON_TEN:
      *out_value = GHOSTTY_MOUSE_BUTTON_TEN;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_MOUSE_BUTTON_ELEVEN:
      *out_value = GHOSTTY_MOUSE_BUTTON_ELEVEN;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

static libghostty_cpp_mouse_position translate_position(GhosttyMousePosition value) {
  return (libghostty_cpp_mouse_position) {
    .x = value.x,
    .y = value.y,
  };
}

static libghostty_cpp_result get_mouse_action(
  const libghostty_cpp_mouse_event* event,
  libghostty_cpp_mouse_action* out_action
) {
  if (event == NULL || out_action == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *out_action = (libghostty_cpp_mouse_action) ghostty_mouse_event_get_action(event->inner);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_mouse_encoder_new(
  libghostty_cpp_mouse_encoder** encoder
) {
  if (encoder == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *encoder = NULL;

  libghostty_cpp_mouse_encoder* wrapper = calloc(1, sizeof(*wrapper));
  if (wrapper == NULL) {
    return LIBGHOSTTY_CPP_RESULT_OUT_OF_MEMORY;
  }

  const GhosttyResult result = ghostty_mouse_encoder_new(NULL, &wrapper->inner);
  if (result != GHOSTTY_SUCCESS) {
    free(wrapper);
    return libghostty_cpp_translate_result(result);
  }

  *encoder = wrapper;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

void libghostty_cpp_mouse_encoder_free(libghostty_cpp_mouse_encoder* encoder) {
  if (encoder == NULL) {
    return;
  }

  ghostty_mouse_encoder_free(encoder->inner);
  free(encoder);
}

libghostty_cpp_result libghostty_cpp_mouse_encoder_set_tracking_mode(
  libghostty_cpp_mouse_encoder* encoder,
  libghostty_cpp_mouse_tracking_mode value
) {
  if (encoder == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyMouseTrackingMode raw_value = GHOSTTY_MOUSE_TRACKING_NONE;
  const libghostty_cpp_result translate_result =
    translate_tracking_mode(value, &raw_value);
  if (translate_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return translate_result;
  }

  ghostty_mouse_encoder_setopt(encoder->inner, GHOSTTY_MOUSE_ENCODER_OPT_EVENT, &raw_value);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_mouse_encoder_set_format(
  libghostty_cpp_mouse_encoder* encoder,
  libghostty_cpp_mouse_format value
) {
  if (encoder == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyMouseFormat raw_value = GHOSTTY_MOUSE_FORMAT_X10;
  const libghostty_cpp_result translate_result = translate_format(value, &raw_value);
  if (translate_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return translate_result;
  }

  ghostty_mouse_encoder_setopt(encoder->inner, GHOSTTY_MOUSE_ENCODER_OPT_FORMAT, &raw_value);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_mouse_encoder_set_size(
  libghostty_cpp_mouse_encoder* encoder,
  libghostty_cpp_mouse_encoder_size value
) {
  if (encoder == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyMouseEncoderSize raw_value = {
    .size = sizeof(GhosttyMouseEncoderSize),
    .screen_width = value.screen_width,
    .screen_height = value.screen_height,
    .cell_width = value.cell_width,
    .cell_height = value.cell_height,
    .padding_top = value.padding_top,
    .padding_bottom = value.padding_bottom,
    .padding_right = value.padding_right,
    .padding_left = value.padding_left,
  };

  ghostty_mouse_encoder_setopt(encoder->inner, GHOSTTY_MOUSE_ENCODER_OPT_SIZE, &raw_value);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_mouse_encoder_set_any_button_pressed(
  libghostty_cpp_mouse_encoder* encoder,
  bool value
) {
  if (encoder == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  ghostty_mouse_encoder_setopt(
    encoder->inner,
    GHOSTTY_MOUSE_ENCODER_OPT_ANY_BUTTON_PRESSED,
    &value
  );
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_mouse_encoder_set_track_last_cell(
  libghostty_cpp_mouse_encoder* encoder,
  bool value
) {
  if (encoder == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  ghostty_mouse_encoder_setopt(
    encoder->inner,
    GHOSTTY_MOUSE_ENCODER_OPT_TRACK_LAST_CELL,
    &value
  );
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_mouse_encoder_set_options_from_terminal(
  libghostty_cpp_mouse_encoder* encoder,
  const libghostty_cpp_terminal* terminal
) {
  if (encoder == NULL || terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  ghostty_mouse_encoder_setopt_from_terminal(encoder->inner, terminal->inner);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_mouse_encoder_reset(
  libghostty_cpp_mouse_encoder* encoder
) {
  if (encoder == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  ghostty_mouse_encoder_reset(encoder->inner);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_mouse_encoder_encode(
  libghostty_cpp_mouse_encoder* encoder,
  const libghostty_cpp_mouse_event* event,
  uint8_t* out_buf,
  size_t out_buf_size,
  size_t* out_len
) {
  if (encoder == NULL || event == NULL || out_len == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  if (out_buf == NULL && out_buf_size != 0) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  return libghostty_cpp_translate_result(ghostty_mouse_encoder_encode(
    encoder->inner,
    event->inner,
    (char*) out_buf,
    out_buf_size,
    out_len
  ));
}

libghostty_cpp_result libghostty_cpp_mouse_event_new(
  libghostty_cpp_mouse_event** event
) {
  if (event == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *event = NULL;

  libghostty_cpp_mouse_event* wrapper = calloc(1, sizeof(*wrapper));
  if (wrapper == NULL) {
    return LIBGHOSTTY_CPP_RESULT_OUT_OF_MEMORY;
  }

  const GhosttyResult result = ghostty_mouse_event_new(NULL, &wrapper->inner);
  if (result != GHOSTTY_SUCCESS) {
    free(wrapper);
    return libghostty_cpp_translate_result(result);
  }

  *event = wrapper;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

void libghostty_cpp_mouse_event_free(libghostty_cpp_mouse_event* event) {
  if (event == NULL) {
    return;
  }

  ghostty_mouse_event_free(event->inner);
  free(event);
}

libghostty_cpp_result libghostty_cpp_mouse_event_set_action(
  libghostty_cpp_mouse_event* event,
  libghostty_cpp_mouse_action action
) {
  if (event == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyMouseAction raw_action = GHOSTTY_MOUSE_ACTION_PRESS;
  const libghostty_cpp_result translate_result = translate_action(action, &raw_action);
  if (translate_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return translate_result;
  }

  ghostty_mouse_event_set_action(event->inner, raw_action);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_mouse_event_get_action(
  const libghostty_cpp_mouse_event* event,
  libghostty_cpp_mouse_action* out_action
) {
  return get_mouse_action(event, out_action);
}

libghostty_cpp_result libghostty_cpp_mouse_event_set_button(
  libghostty_cpp_mouse_event* event,
  libghostty_cpp_mouse_button button
) {
  if (event == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyMouseButton raw_button = GHOSTTY_MOUSE_BUTTON_UNKNOWN;
  const libghostty_cpp_result translate_result = translate_button(button, &raw_button);
  if (translate_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return translate_result;
  }

  ghostty_mouse_event_set_button(event->inner, raw_button);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_mouse_event_clear_button(
  libghostty_cpp_mouse_event* event
) {
  if (event == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  ghostty_mouse_event_clear_button(event->inner);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_mouse_event_get_button(
  const libghostty_cpp_mouse_event* event,
  bool* out_has_button,
  libghostty_cpp_mouse_button* out_button
) {
  if (event == NULL || out_has_button == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyMouseButton raw_button = GHOSTTY_MOUSE_BUTTON_UNKNOWN;
  *out_has_button = ghostty_mouse_event_get_button(event->inner, &raw_button);
  if (!*out_has_button) {
    return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  if (out_button == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *out_button = (libghostty_cpp_mouse_button) raw_button;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_mouse_event_set_mods(
  libghostty_cpp_mouse_event* event,
  libghostty_cpp_key_mods mods
) {
  if (event == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  ghostty_mouse_event_set_mods(event->inner, mods);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_mouse_event_get_mods(
  const libghostty_cpp_mouse_event* event,
  libghostty_cpp_key_mods* out_mods
) {
  if (event == NULL || out_mods == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *out_mods = ghostty_mouse_event_get_mods(event->inner);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_mouse_event_set_position(
  libghostty_cpp_mouse_event* event,
  libghostty_cpp_mouse_position position
) {
  if (event == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  ghostty_mouse_event_set_position(event->inner, (GhosttyMousePosition) {
    .x = position.x,
    .y = position.y,
  });
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_mouse_event_get_position(
  const libghostty_cpp_mouse_event* event,
  libghostty_cpp_mouse_position* out_position
) {
  if (event == NULL || out_position == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *out_position = translate_position(ghostty_mouse_event_get_position(event->inner));
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}
