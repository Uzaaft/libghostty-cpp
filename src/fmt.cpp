#include "libghostty_cpp/fmt.hpp"

#include "c_compat/internal.h"

#include <ghostty/vt.h>

#include "ghostty_result.hpp"
#include "libghostty_cpp/vt.hpp"

#include <utility>

namespace libghostty_cpp::fmt {

namespace {

GhosttyFormatterFormat translate_format(Format format) {
  switch (format) {
    case Format::Plain:
      return GHOSTTY_FORMATTER_FORMAT_PLAIN;
    case Format::Vt:
      return GHOSTTY_FORMATTER_FORMAT_VT;
    case Format::Html:
      return GHOSTTY_FORMATTER_FORMAT_HTML;
  }

  throw Error(ErrorCode::InvalidValue);
}

GhosttyFormatterTerminalOptions translate_options(FormatterOptions options) {
  GhosttyFormatterTerminalOptions translated = GHOSTTY_INIT_SIZED(GhosttyFormatterTerminalOptions);
  translated.emit = translate_format(options.format);
  translated.trim = options.trim;
  translated.unwrap = options.unwrap;
  return translated;
}

} // namespace

Formatter::Formatter(const Terminal& terminal, FormatterOptions options) {
  GhosttyFormatter handle = nullptr;
  detail::throw_if_ghostty_error(
    ghostty_formatter_terminal_new(
      nullptr,
      &handle,
      terminal.handle_->inner,
      translate_options(options)
    )
  );
  handle_ = handle;
}

Formatter::~Formatter() {
  release();
}

Formatter::Formatter(Formatter&& other) noexcept
    : handle_(std::exchange(other.handle_, nullptr)) {}

Formatter& Formatter::operator=(Formatter&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  release();
  handle_ = std::exchange(other.handle_, nullptr);
  return *this;
}

std::size_t Formatter::format_len() {
  if (handle_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }

  std::size_t written = 0;
  const GhosttyResult result = ghostty_formatter_format_buf(
    static_cast<GhosttyFormatter>(handle_),
    nullptr,
    0,
    &written
  );
  if (result == GHOSTTY_OUT_OF_SPACE) {
    return written;
  }

  detail::throw_if_ghostty_error(result);
  return written;
}

std::size_t Formatter::format_buf(std::uint8_t* output, std::size_t output_size) {
  if (handle_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }

  std::size_t written = 0;
  detail::throw_if_ghostty_error(
    ghostty_formatter_format_buf(
      static_cast<GhosttyFormatter>(handle_),
      output,
      output_size,
      &written
    )
  );
  return written;
}

std::vector<std::uint8_t> Formatter::format_bytes() {
  const std::size_t required = format_len();
  std::vector<std::uint8_t> output(required);
  output.resize(format_buf(output.data(), output.size()));
  return output;
}

std::string Formatter::format_string() {
  const std::vector<std::uint8_t> bytes = format_bytes();
  return std::string(bytes.begin(), bytes.end());
}

void Formatter::release() noexcept {
  if (handle_ != nullptr) {
    ghostty_formatter_free(static_cast<GhosttyFormatter>(handle_));
    handle_ = nullptr;
  }
}

} // namespace libghostty_cpp::fmt
