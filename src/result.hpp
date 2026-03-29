#pragma once

#include "c_compat/vt.h"
#include "libghostty_cpp/error.hpp"

#include <optional>

namespace libghostty_cpp {
namespace detail {

inline std::optional<ErrorCode> error_code_from_result(
  libghostty_cpp_result result
) noexcept {
  switch (result) {
    case LIBGHOSTTY_CPP_RESULT_SUCCESS:
      return std::nullopt;
    case LIBGHOSTTY_CPP_RESULT_OUT_OF_MEMORY:
      return ErrorCode::OutOfMemory;
    case LIBGHOSTTY_CPP_RESULT_INVALID_VALUE:
      return ErrorCode::InvalidValue;
    case LIBGHOSTTY_CPP_RESULT_OUT_OF_SPACE:
      return ErrorCode::OutOfSpace;
  }

  return ErrorCode::InvalidValue;
}

inline void throw_if_error(libghostty_cpp_result result) {
  if (const auto error_code = error_code_from_result(result)) {
    throw Error(*error_code);
  }
}

} // namespace detail
} // namespace libghostty_cpp
