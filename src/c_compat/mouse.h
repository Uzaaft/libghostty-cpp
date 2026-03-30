#pragma once

#include "c_compat/key.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct libghostty_cpp_mouse_encoder libghostty_cpp_mouse_encoder;
typedef struct libghostty_cpp_mouse_event libghostty_cpp_mouse_event;

typedef enum libghostty_cpp_mouse_tracking_mode {
  LIBGHOSTTY_CPP_MOUSE_TRACKING_MODE_NONE = 0,
  LIBGHOSTTY_CPP_MOUSE_TRACKING_MODE_X10 = 1,
  LIBGHOSTTY_CPP_MOUSE_TRACKING_MODE_NORMAL = 2,
  LIBGHOSTTY_CPP_MOUSE_TRACKING_MODE_BUTTON = 3,
  LIBGHOSTTY_CPP_MOUSE_TRACKING_MODE_ANY = 4,
} libghostty_cpp_mouse_tracking_mode;

typedef enum libghostty_cpp_mouse_format {
  LIBGHOSTTY_CPP_MOUSE_FORMAT_X10 = 0,
  LIBGHOSTTY_CPP_MOUSE_FORMAT_UTF8 = 1,
  LIBGHOSTTY_CPP_MOUSE_FORMAT_SGR = 2,
  LIBGHOSTTY_CPP_MOUSE_FORMAT_URXVT = 3,
  LIBGHOSTTY_CPP_MOUSE_FORMAT_SGR_PIXELS = 4,
} libghostty_cpp_mouse_format;

typedef enum libghostty_cpp_mouse_action {
  LIBGHOSTTY_CPP_MOUSE_ACTION_PRESS = 0,
  LIBGHOSTTY_CPP_MOUSE_ACTION_RELEASE = 1,
  LIBGHOSTTY_CPP_MOUSE_ACTION_MOTION = 2,
} libghostty_cpp_mouse_action;

typedef enum libghostty_cpp_mouse_button {
  LIBGHOSTTY_CPP_MOUSE_BUTTON_UNKNOWN = 0,
  LIBGHOSTTY_CPP_MOUSE_BUTTON_LEFT = 1,
  LIBGHOSTTY_CPP_MOUSE_BUTTON_RIGHT = 2,
  LIBGHOSTTY_CPP_MOUSE_BUTTON_MIDDLE = 3,
  LIBGHOSTTY_CPP_MOUSE_BUTTON_FOUR = 4,
  LIBGHOSTTY_CPP_MOUSE_BUTTON_FIVE = 5,
  LIBGHOSTTY_CPP_MOUSE_BUTTON_SIX = 6,
  LIBGHOSTTY_CPP_MOUSE_BUTTON_SEVEN = 7,
  LIBGHOSTTY_CPP_MOUSE_BUTTON_EIGHT = 8,
  LIBGHOSTTY_CPP_MOUSE_BUTTON_NINE = 9,
  LIBGHOSTTY_CPP_MOUSE_BUTTON_TEN = 10,
  LIBGHOSTTY_CPP_MOUSE_BUTTON_ELEVEN = 11,
} libghostty_cpp_mouse_button;

typedef struct libghostty_cpp_mouse_position {
  float x;
  float y;
} libghostty_cpp_mouse_position;

typedef struct libghostty_cpp_mouse_encoder_size {
  uint32_t screen_width;
  uint32_t screen_height;
  uint32_t cell_width;
  uint32_t cell_height;
  uint32_t padding_top;
  uint32_t padding_bottom;
  uint32_t padding_right;
  uint32_t padding_left;
} libghostty_cpp_mouse_encoder_size;

libghostty_cpp_result libghostty_cpp_mouse_encoder_new(
  libghostty_cpp_mouse_encoder** encoder
);

void libghostty_cpp_mouse_encoder_free(libghostty_cpp_mouse_encoder* encoder);

libghostty_cpp_result libghostty_cpp_mouse_encoder_set_tracking_mode(
  libghostty_cpp_mouse_encoder* encoder,
  libghostty_cpp_mouse_tracking_mode value
);

libghostty_cpp_result libghostty_cpp_mouse_encoder_set_format(
  libghostty_cpp_mouse_encoder* encoder,
  libghostty_cpp_mouse_format value
);

libghostty_cpp_result libghostty_cpp_mouse_encoder_set_size(
  libghostty_cpp_mouse_encoder* encoder,
  libghostty_cpp_mouse_encoder_size value
);

libghostty_cpp_result libghostty_cpp_mouse_encoder_set_any_button_pressed(
  libghostty_cpp_mouse_encoder* encoder,
  bool value
);

libghostty_cpp_result libghostty_cpp_mouse_encoder_set_track_last_cell(
  libghostty_cpp_mouse_encoder* encoder,
  bool value
);

libghostty_cpp_result libghostty_cpp_mouse_encoder_set_options_from_terminal(
  libghostty_cpp_mouse_encoder* encoder,
  const libghostty_cpp_terminal* terminal
);

libghostty_cpp_result libghostty_cpp_mouse_encoder_reset(
  libghostty_cpp_mouse_encoder* encoder
);

libghostty_cpp_result libghostty_cpp_mouse_encoder_encode(
  libghostty_cpp_mouse_encoder* encoder,
  const libghostty_cpp_mouse_event* event,
  uint8_t* out_buf,
  size_t out_buf_size,
  size_t* out_len
);

libghostty_cpp_result libghostty_cpp_mouse_event_new(
  libghostty_cpp_mouse_event** event
);

void libghostty_cpp_mouse_event_free(libghostty_cpp_mouse_event* event);

libghostty_cpp_result libghostty_cpp_mouse_event_set_action(
  libghostty_cpp_mouse_event* event,
  libghostty_cpp_mouse_action action
);

libghostty_cpp_result libghostty_cpp_mouse_event_get_action(
  const libghostty_cpp_mouse_event* event,
  libghostty_cpp_mouse_action* out_action
);

libghostty_cpp_result libghostty_cpp_mouse_event_set_button(
  libghostty_cpp_mouse_event* event,
  libghostty_cpp_mouse_button button
);

libghostty_cpp_result libghostty_cpp_mouse_event_clear_button(
  libghostty_cpp_mouse_event* event
);

libghostty_cpp_result libghostty_cpp_mouse_event_get_button(
  const libghostty_cpp_mouse_event* event,
  bool* out_has_button,
  libghostty_cpp_mouse_button* out_button
);

libghostty_cpp_result libghostty_cpp_mouse_event_set_mods(
  libghostty_cpp_mouse_event* event,
  libghostty_cpp_key_mods mods
);

libghostty_cpp_result libghostty_cpp_mouse_event_get_mods(
  const libghostty_cpp_mouse_event* event,
  libghostty_cpp_key_mods* out_mods
);

libghostty_cpp_result libghostty_cpp_mouse_event_set_position(
  libghostty_cpp_mouse_event* event,
  libghostty_cpp_mouse_position position
);

libghostty_cpp_result libghostty_cpp_mouse_event_get_position(
  const libghostty_cpp_mouse_event* event,
  libghostty_cpp_mouse_position* out_position
);

#ifdef __cplusplus
}
#endif
