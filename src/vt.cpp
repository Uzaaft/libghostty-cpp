#include "libghostty_cpp/vt.hpp"

#include "c_compat/vt.h"

#include <optional>
#include <utility>

namespace libghostty_cpp {

namespace {

const char* error_message(ErrorCode code) noexcept {
  switch (code) {
    case ErrorCode::OutOfMemory:
      return "libghostty: out of memory";
    case ErrorCode::InvalidValue:
      return "libghostty: invalid value";
    case ErrorCode::OutOfSpace:
      return "libghostty: out of space";
  }

  return "libghostty: unknown error";
}

std::optional<ErrorCode> error_code_from_result(libghostty_cpp_result result) noexcept {
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

void throw_if_error(libghostty_cpp_result result) {
  if (const auto error_code = error_code_from_result(result)) {
    throw Error(*error_code);
  }
}

using TerminalU16Getter = libghostty_cpp_result (*)(const libghostty_cpp_terminal*, uint16_t*);

std::uint16_t get_u16(const libghostty_cpp_terminal* handle, TerminalU16Getter getter) {
  std::uint16_t value = 0;
  throw_if_error(getter(handle, &value));
  return value;
}

}  // namespace

Error::Error(ErrorCode code)
    : std::runtime_error(error_message(code)), code_(code) {}

ErrorCode Error::code() const noexcept {
  return code_;
}

Terminal::Terminal(TerminalOptions options) {
  libghostty_cpp_terminal_options raw_options = {};
  raw_options.cols = options.cols;
  raw_options.rows = options.rows;
  raw_options.max_scrollback = options.max_scrollback;

  libghostty_cpp_terminal* handle = nullptr;
  throw_if_error(libghostty_cpp_terminal_new(&handle, raw_options));
  handle_ = handle;
}

Terminal::~Terminal() {
  release();
}

Terminal::Terminal(Terminal&& other) noexcept : handle_(std::exchange(other.handle_, nullptr)) {}

Terminal& Terminal::operator=(Terminal&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  release();
  handle_ = std::exchange(other.handle_, nullptr);
  return *this;
}

void Terminal::vt_write(std::string_view data) noexcept {
  libghostty_cpp_terminal_vt_write(
    handle_,
    reinterpret_cast<const std::uint8_t*>(data.data()),
    data.size()
  );
}

void Terminal::reset() noexcept {
  libghostty_cpp_terminal_reset(handle_);
}

void Terminal::resize(
  std::uint16_t cols,
  std::uint16_t rows,
  std::uint32_t cell_width_px,
  std::uint32_t cell_height_px
) {
  throw_if_error(libghostty_cpp_terminal_resize(
    handle_,
    cols,
    rows,
    cell_width_px,
    cell_height_px
  ));
}

std::uint16_t Terminal::cols() const {
  return get_u16(handle_, libghostty_cpp_terminal_cols);
}

std::uint16_t Terminal::rows() const {
  return get_u16(handle_, libghostty_cpp_terminal_rows);
}

std::uint16_t Terminal::cursor_x() const {
  return get_u16(handle_, libghostty_cpp_terminal_cursor_x);
}

std::uint16_t Terminal::cursor_y() const {
  return get_u16(handle_, libghostty_cpp_terminal_cursor_y);
}

void Terminal::release() noexcept {
  libghostty_cpp_terminal_free(handle_);
  handle_ = nullptr;
}

}  // namespace libghostty_cpp
