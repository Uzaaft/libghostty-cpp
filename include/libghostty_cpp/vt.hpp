#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "libghostty_cpp/error.hpp"

struct libghostty_cpp_terminal;

namespace libghostty_cpp {

class RenderState;

struct TerminalOptions {
  std::uint16_t cols;
  std::uint16_t rows;
  std::size_t max_scrollback = 0;
};

class Terminal {
public:
  explicit Terminal(TerminalOptions options);
  ~Terminal();

  Terminal(Terminal &&other) noexcept;
  Terminal &operator=(Terminal &&other) noexcept;

  Terminal(const Terminal &) = delete;
  Terminal &operator=(const Terminal &) = delete;

  void vt_write(std::string_view data) noexcept;
  void reset() noexcept;
  void resize(std::uint16_t cols, std::uint16_t rows,
              std::uint32_t cell_width_px = 0,
              std::uint32_t cell_height_px = 0);

  [[nodiscard]] std::uint16_t cols() const;
  [[nodiscard]] std::uint16_t rows() const;
  [[nodiscard]] std::uint16_t cursor_x() const;
  [[nodiscard]] std::uint16_t cursor_y() const;

private:
  friend class RenderState;

  void release() noexcept;

  libghostty_cpp_terminal *handle_ = nullptr;
};

} // namespace libghostty_cpp
