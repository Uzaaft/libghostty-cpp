#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace libghostty_cpp {
class Terminal;
}

namespace libghostty_cpp::fmt {

enum class Format {
  Plain,
  Vt,
  Html,
};

struct FormatterOptions {
  Format format = Format::Plain;
  bool trim = false;
  bool unwrap = false;
};

class Formatter {
public:
  Formatter(const Terminal& terminal, FormatterOptions options);
  ~Formatter();

  Formatter(Formatter&& other) noexcept;
  Formatter& operator=(Formatter&& other) noexcept;

  Formatter(const Formatter&) = delete;
  Formatter& operator=(const Formatter&) = delete;

  [[nodiscard]] std::size_t format_len();
  [[nodiscard]] std::size_t format_buf(std::uint8_t* output, std::size_t output_size);
  [[nodiscard]] std::vector<std::uint8_t> format_bytes();
  [[nodiscard]] std::string format_string();

private:
  void release() noexcept;

  void* handle_ = nullptr;
};

} // namespace libghostty_cpp::fmt
