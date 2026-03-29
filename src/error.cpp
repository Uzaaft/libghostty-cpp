#include "libghostty_cpp/error.hpp"

namespace libghostty_cpp {

namespace {

const char *error_message(ErrorCode code) noexcept {
  switch (code) {
    case ErrorCode::OutOfMemory:
      return "libghostty: out of memory";
    case ErrorCode::InvalidValue:
      return "libghostty: invalid value";
    case ErrorCode::InvalidState:
      return "libghostty: invalid state";
    case ErrorCode::OutOfSpace:
      return "libghostty: out of space";
  }

  return "libghostty: unknown error";
}

} // namespace

Error::Error(ErrorCode code)
    : std::runtime_error(error_message(code)), code_(code) {}

ErrorCode Error::code() const noexcept {
  return code_;
}

} // namespace libghostty_cpp
