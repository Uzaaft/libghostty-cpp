#include "c_compat/key.h"

#include "c_compat/internal.h"

#include <stdlib.h>

struct libghostty_cpp_key_encoder {
  GhosttyKeyEncoder inner;
};

struct libghostty_cpp_key_event {
  GhosttyKeyEvent inner;
};

static libghostty_cpp_result translate_option(
  libghostty_cpp_key_encoder_option option,
  GhosttyKeyEncoderOption* out_option
) {
  if (out_option == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  switch (option) {
    case LIBGHOSTTY_CPP_KEY_ENCODER_OPTION_CURSOR_KEY_APPLICATION:
      *out_option = GHOSTTY_KEY_ENCODER_OPT_CURSOR_KEY_APPLICATION;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_KEY_ENCODER_OPTION_KEYPAD_KEY_APPLICATION:
      *out_option = GHOSTTY_KEY_ENCODER_OPT_KEYPAD_KEY_APPLICATION;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_KEY_ENCODER_OPTION_IGNORE_KEYPAD_WITH_NUMLOCK:
      *out_option = GHOSTTY_KEY_ENCODER_OPT_IGNORE_KEYPAD_WITH_NUMLOCK;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_KEY_ENCODER_OPTION_ALT_ESC_PREFIX:
      *out_option = GHOSTTY_KEY_ENCODER_OPT_ALT_ESC_PREFIX;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_KEY_ENCODER_OPTION_MODIFY_OTHER_KEYS_STATE_2:
      *out_option = GHOSTTY_KEY_ENCODER_OPT_MODIFY_OTHER_KEYS_STATE_2;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

static libghostty_cpp_result translate_option_as_alt(
  libghostty_cpp_key_option_as_alt option,
  GhosttyOptionAsAlt* out_option
) {
  if (out_option == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  switch (option) {
    case LIBGHOSTTY_CPP_KEY_OPTION_AS_ALT_FALSE:
      *out_option = GHOSTTY_OPTION_AS_ALT_FALSE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_KEY_OPTION_AS_ALT_TRUE:
      *out_option = GHOSTTY_OPTION_AS_ALT_TRUE;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_KEY_OPTION_AS_ALT_LEFT:
      *out_option = GHOSTTY_OPTION_AS_ALT_LEFT;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
    case LIBGHOSTTY_CPP_KEY_OPTION_AS_ALT_RIGHT:
      *out_option = GHOSTTY_OPTION_AS_ALT_RIGHT;
      return LIBGHOSTTY_CPP_RESULT_SUCCESS;
  }

  return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
}

static libghostty_cpp_result get_key_action(
  const libghostty_cpp_key_event* event,
  libghostty_cpp_key_action* out_action
) {
  if (event == NULL || out_action == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *out_action = (libghostty_cpp_key_action) ghostty_key_event_get_action(event->inner);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

static libghostty_cpp_result get_key_u32(
  const libghostty_cpp_key_event* event,
  uint32_t (*getter)(GhosttyKeyEvent),
  uint32_t* out_value
) {
  if (event == NULL || out_value == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *out_value = getter(event->inner);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

static libghostty_cpp_result get_key_mods(
  const libghostty_cpp_key_event* event,
  GhosttyMods (*getter)(GhosttyKeyEvent),
  libghostty_cpp_key_mods* out_mods
) {
  if (event == NULL || out_mods == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *out_mods = getter(event->inner);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_key_encoder_new(
  libghostty_cpp_key_encoder** encoder
) {
  if (encoder == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *encoder = NULL;

  libghostty_cpp_key_encoder* wrapper = calloc(1, sizeof(*wrapper));
  if (wrapper == NULL) {
    return LIBGHOSTTY_CPP_RESULT_OUT_OF_MEMORY;
  }

  const GhosttyResult result = ghostty_key_encoder_new(NULL, &wrapper->inner);
  if (result != GHOSTTY_SUCCESS) {
    free(wrapper);
    return libghostty_cpp_translate_result(result);
  }

  *encoder = wrapper;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

void libghostty_cpp_key_encoder_free(libghostty_cpp_key_encoder* encoder) {
  if (encoder == NULL) {
    return;
  }

  ghostty_key_encoder_free(encoder->inner);
  free(encoder);
}

libghostty_cpp_result libghostty_cpp_key_encoder_set_bool_option(
  libghostty_cpp_key_encoder* encoder,
  libghostty_cpp_key_encoder_option option,
  bool value
) {
  if (encoder == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyKeyEncoderOption raw_option = GHOSTTY_KEY_ENCODER_OPT_CURSOR_KEY_APPLICATION;
  const libghostty_cpp_result option_result = translate_option(option, &raw_option);
  if (option_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return option_result;
  }

  ghostty_key_encoder_setopt(encoder->inner, raw_option, &value);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_key_encoder_set_kitty_flags(
  libghostty_cpp_key_encoder* encoder,
  libghostty_cpp_kitty_key_flags flags
) {
  if (encoder == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  ghostty_key_encoder_setopt(
    encoder->inner,
    GHOSTTY_KEY_ENCODER_OPT_KITTY_FLAGS,
    &flags
  );
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_key_encoder_set_option_as_alt(
  libghostty_cpp_key_encoder* encoder,
  libghostty_cpp_key_option_as_alt option
) {
  if (encoder == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  GhosttyOptionAsAlt raw_option = GHOSTTY_OPTION_AS_ALT_FALSE;
  const libghostty_cpp_result option_result =
    translate_option_as_alt(option, &raw_option);
  if (option_result != LIBGHOSTTY_CPP_RESULT_SUCCESS) {
    return option_result;
  }

  ghostty_key_encoder_setopt(
    encoder->inner,
    GHOSTTY_KEY_ENCODER_OPT_MACOS_OPTION_AS_ALT,
    &raw_option
  );
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_key_encoder_set_options_from_terminal(
  libghostty_cpp_key_encoder* encoder,
  const libghostty_cpp_terminal* terminal
) {
  if (encoder == NULL || terminal == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  ghostty_key_encoder_setopt_from_terminal(encoder->inner, terminal->inner);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_key_encoder_encode(
  libghostty_cpp_key_encoder* encoder,
  const libghostty_cpp_key_event* event,
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

  return libghostty_cpp_translate_result(ghostty_key_encoder_encode(
    encoder->inner,
    event->inner,
    (char*) out_buf,
    out_buf_size,
    out_len
  ));
}

libghostty_cpp_result libghostty_cpp_key_event_new(
  libghostty_cpp_key_event** event
) {
  if (event == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *event = NULL;

  libghostty_cpp_key_event* wrapper = calloc(1, sizeof(*wrapper));
  if (wrapper == NULL) {
    return LIBGHOSTTY_CPP_RESULT_OUT_OF_MEMORY;
  }

  const GhosttyResult result = ghostty_key_event_new(NULL, &wrapper->inner);
  if (result != GHOSTTY_SUCCESS) {
    free(wrapper);
    return libghostty_cpp_translate_result(result);
  }

  *event = wrapper;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

void libghostty_cpp_key_event_free(libghostty_cpp_key_event* event) {
  if (event == NULL) {
    return;
  }

  ghostty_key_event_free(event->inner);
  free(event);
}

libghostty_cpp_result libghostty_cpp_key_event_set_action(
  libghostty_cpp_key_event* event,
  libghostty_cpp_key_action action
) {
  if (event == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  ghostty_key_event_set_action(event->inner, (GhosttyKeyAction) action);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_key_event_get_action(
  const libghostty_cpp_key_event* event,
  libghostty_cpp_key_action* out_action
) {
  return get_key_action(event, out_action);
}

libghostty_cpp_result libghostty_cpp_key_event_set_key(
  libghostty_cpp_key_event* event,
  libghostty_cpp_key_value key
) {
  if (event == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  ghostty_key_event_set_key(event->inner, key);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_key_event_get_key(
  const libghostty_cpp_key_event* event,
  libghostty_cpp_key_value* out_key
) {
  return get_key_u32(event, ghostty_key_event_get_key, out_key);
}

libghostty_cpp_result libghostty_cpp_key_event_set_mods(
  libghostty_cpp_key_event* event,
  libghostty_cpp_key_mods mods
) {
  if (event == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  ghostty_key_event_set_mods(event->inner, mods);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_key_event_get_mods(
  const libghostty_cpp_key_event* event,
  libghostty_cpp_key_mods* out_mods
) {
  return get_key_mods(event, ghostty_key_event_get_mods, out_mods);
}

libghostty_cpp_result libghostty_cpp_key_event_set_consumed_mods(
  libghostty_cpp_key_event* event,
  libghostty_cpp_key_mods mods
) {
  if (event == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  ghostty_key_event_set_consumed_mods(event->inner, mods);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_key_event_get_consumed_mods(
  const libghostty_cpp_key_event* event,
  libghostty_cpp_key_mods* out_mods
) {
  return get_key_mods(event, ghostty_key_event_get_consumed_mods, out_mods);
}

libghostty_cpp_result libghostty_cpp_key_event_set_composing(
  libghostty_cpp_key_event* event,
  bool composing
) {
  if (event == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  ghostty_key_event_set_composing(event->inner, composing);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_key_event_get_composing(
  const libghostty_cpp_key_event* event,
  bool* out_composing
) {
  if (event == NULL || out_composing == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  *out_composing = ghostty_key_event_get_composing(event->inner);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_key_event_set_utf8(
  libghostty_cpp_key_event* event,
  const uint8_t* utf8,
  size_t len
) {
  if (event == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  if (utf8 == NULL && len != 0) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  ghostty_key_event_set_utf8(event->inner, (const char*) utf8, len);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_key_event_get_utf8(
  const libghostty_cpp_key_event* event,
  const uint8_t** out_utf8,
  size_t* out_len
) {
  if (event == NULL || out_utf8 == NULL || out_len == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  const char* utf8 = ghostty_key_event_get_utf8(event->inner, out_len);
  *out_utf8 = (const uint8_t*) utf8;
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_key_event_set_unshifted_codepoint(
  libghostty_cpp_key_event* event,
  uint32_t codepoint
) {
  if (event == NULL) {
    return LIBGHOSTTY_CPP_RESULT_INVALID_VALUE;
  }

  ghostty_key_event_set_unshifted_codepoint(event->inner, codepoint);
  return LIBGHOSTTY_CPP_RESULT_SUCCESS;
}

libghostty_cpp_result libghostty_cpp_key_event_get_unshifted_codepoint(
  const libghostty_cpp_key_event* event,
  uint32_t* out_codepoint
) {
  return get_key_u32(event, ghostty_key_event_get_unshifted_codepoint, out_codepoint);
}
