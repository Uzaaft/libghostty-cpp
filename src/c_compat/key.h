#pragma once

#include "c_compat/vt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct libghostty_cpp_key_encoder libghostty_cpp_key_encoder;
typedef struct libghostty_cpp_key_event libghostty_cpp_key_event;

typedef enum libghostty_cpp_key_action {
  LIBGHOSTTY_CPP_KEY_ACTION_RELEASE = 0,
  LIBGHOSTTY_CPP_KEY_ACTION_PRESS = 1,
  LIBGHOSTTY_CPP_KEY_ACTION_REPEAT = 2,
} libghostty_cpp_key_action;

typedef enum libghostty_cpp_key_option_as_alt {
  LIBGHOSTTY_CPP_KEY_OPTION_AS_ALT_FALSE = 0,
  LIBGHOSTTY_CPP_KEY_OPTION_AS_ALT_TRUE = 1,
  LIBGHOSTTY_CPP_KEY_OPTION_AS_ALT_LEFT = 2,
  LIBGHOSTTY_CPP_KEY_OPTION_AS_ALT_RIGHT = 3,
} libghostty_cpp_key_option_as_alt;

typedef enum libghostty_cpp_key_encoder_option {
  LIBGHOSTTY_CPP_KEY_ENCODER_OPTION_CURSOR_KEY_APPLICATION = 0,
  LIBGHOSTTY_CPP_KEY_ENCODER_OPTION_KEYPAD_KEY_APPLICATION = 1,
  LIBGHOSTTY_CPP_KEY_ENCODER_OPTION_IGNORE_KEYPAD_WITH_NUMLOCK = 2,
  LIBGHOSTTY_CPP_KEY_ENCODER_OPTION_ALT_ESC_PREFIX = 3,
  LIBGHOSTTY_CPP_KEY_ENCODER_OPTION_MODIFY_OTHER_KEYS_STATE_2 = 4,
} libghostty_cpp_key_encoder_option;

typedef uint32_t libghostty_cpp_key_value;
typedef uint16_t libghostty_cpp_key_mods;
typedef uint8_t libghostty_cpp_kitty_key_flags;

libghostty_cpp_result libghostty_cpp_key_encoder_new(
  libghostty_cpp_key_encoder** encoder
);

void libghostty_cpp_key_encoder_free(libghostty_cpp_key_encoder* encoder);

libghostty_cpp_result libghostty_cpp_key_encoder_set_bool_option(
  libghostty_cpp_key_encoder* encoder,
  libghostty_cpp_key_encoder_option option,
  bool value
);

libghostty_cpp_result libghostty_cpp_key_encoder_set_kitty_flags(
  libghostty_cpp_key_encoder* encoder,
  libghostty_cpp_kitty_key_flags flags
);

libghostty_cpp_result libghostty_cpp_key_encoder_set_option_as_alt(
  libghostty_cpp_key_encoder* encoder,
  libghostty_cpp_key_option_as_alt option
);

libghostty_cpp_result libghostty_cpp_key_encoder_set_options_from_terminal(
  libghostty_cpp_key_encoder* encoder,
  const libghostty_cpp_terminal* terminal
);

libghostty_cpp_result libghostty_cpp_key_encoder_encode(
  libghostty_cpp_key_encoder* encoder,
  const libghostty_cpp_key_event* event,
  uint8_t* out_buf,
  size_t out_buf_size,
  size_t* out_len
);

libghostty_cpp_result libghostty_cpp_key_event_new(
  libghostty_cpp_key_event** event
);

void libghostty_cpp_key_event_free(libghostty_cpp_key_event* event);

libghostty_cpp_result libghostty_cpp_key_event_set_action(
  libghostty_cpp_key_event* event,
  libghostty_cpp_key_action action
);

libghostty_cpp_result libghostty_cpp_key_event_get_action(
  const libghostty_cpp_key_event* event,
  libghostty_cpp_key_action* out_action
);

libghostty_cpp_result libghostty_cpp_key_event_set_key(
  libghostty_cpp_key_event* event,
  libghostty_cpp_key_value key
);

libghostty_cpp_result libghostty_cpp_key_event_get_key(
  const libghostty_cpp_key_event* event,
  libghostty_cpp_key_value* out_key
);

libghostty_cpp_result libghostty_cpp_key_event_set_mods(
  libghostty_cpp_key_event* event,
  libghostty_cpp_key_mods mods
);

libghostty_cpp_result libghostty_cpp_key_event_get_mods(
  const libghostty_cpp_key_event* event,
  libghostty_cpp_key_mods* out_mods
);

libghostty_cpp_result libghostty_cpp_key_event_set_consumed_mods(
  libghostty_cpp_key_event* event,
  libghostty_cpp_key_mods mods
);

libghostty_cpp_result libghostty_cpp_key_event_get_consumed_mods(
  const libghostty_cpp_key_event* event,
  libghostty_cpp_key_mods* out_mods
);

libghostty_cpp_result libghostty_cpp_key_event_set_composing(
  libghostty_cpp_key_event* event,
  bool composing
);

libghostty_cpp_result libghostty_cpp_key_event_get_composing(
  const libghostty_cpp_key_event* event,
  bool* out_composing
);

libghostty_cpp_result libghostty_cpp_key_event_set_utf8(
  libghostty_cpp_key_event* event,
  const uint8_t* utf8,
  size_t len
);

libghostty_cpp_result libghostty_cpp_key_event_get_utf8(
  const libghostty_cpp_key_event* event,
  const uint8_t** out_utf8,
  size_t* out_len
);

libghostty_cpp_result libghostty_cpp_key_event_set_unshifted_codepoint(
  libghostty_cpp_key_event* event,
  uint32_t codepoint
);

libghostty_cpp_result libghostty_cpp_key_event_get_unshifted_codepoint(
  const libghostty_cpp_key_event* event,
  uint32_t* out_codepoint
);

#ifdef __cplusplus
}
#endif
