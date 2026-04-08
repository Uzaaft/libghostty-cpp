#pragma once

#include <ghostty/vt.h>

#include "libghostty_cpp/error.hpp"

namespace libghostty_cpp::detail {

inline ErrorCode error_code_from_ghostty_result(GhosttyResult result) noexcept {
  switch (result) {
    case GHOSTTY_OUT_OF_MEMORY:
      return ErrorCode::OutOfMemory;
    case GHOSTTY_INVALID_VALUE:
      return ErrorCode::InvalidValue;
    case GHOSTTY_OUT_OF_SPACE:
      return ErrorCode::OutOfSpace;
    default:
      return ErrorCode::InvalidValue;
  }
}

inline void throw_if_ghostty_error(GhosttyResult result) {
  if (result == GHOSTTY_SUCCESS) {
    return;
  }

  throw Error(error_code_from_ghostty_result(result));
}

} // namespace libghostty_cpp::detail
