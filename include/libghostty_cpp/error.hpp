#pragma once

#include <stdexcept>

namespace libghostty_cpp {

enum class ErrorCode {
  OutOfMemory,
  InvalidValue,
  InvalidState,
  OutOfSpace,
};

class Error : public std::runtime_error {
public:
  explicit Error(ErrorCode code);

  [[nodiscard]] ErrorCode code() const noexcept;

private:
  ErrorCode code_;
};

} // namespace libghostty_cpp
