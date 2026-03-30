#pragma once

#include "c_compat/vt.h"

#include <ghostty/vt.h>

struct libghostty_cpp_terminal {
  GhosttyTerminal inner;
  void* callback_userdata;
  libghostty_cpp_terminal_pty_write_fn pty_write;
  libghostty_cpp_terminal_size_fn size;
  libghostty_cpp_terminal_device_attributes_fn device_attributes;
  libghostty_cpp_terminal_xtversion_fn xtversion;
  libghostty_cpp_terminal_color_scheme_fn color_scheme;
};

static inline libghostty_cpp_result libghostty_cpp_translate_result(
  GhosttyResult result
) {
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
