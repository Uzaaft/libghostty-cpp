#pragma once

#include "c_compat/vt.h"

#include <ghostty/vt.h>

struct libghostty_cpp_terminal {
  GhosttyTerminal inner;
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
