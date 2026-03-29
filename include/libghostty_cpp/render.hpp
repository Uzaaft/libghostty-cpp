#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

#include "libghostty_cpp/error.hpp"

struct libghostty_cpp_render_state;

namespace libghostty_cpp {

class Terminal;

enum class Dirty {
  Clean,
  Partial,
  Full,
};

enum class CursorVisualStyle {
  Bar,
  Block,
  Underline,
  BlockHollow,
};

struct RgbColor {
  std::uint8_t r;
  std::uint8_t g;
  std::uint8_t b;
};

struct CursorViewport {
  std::uint16_t x;
  std::uint16_t y;
  bool at_wide_tail;
};

struct RenderColors {
  RgbColor background;
  RgbColor foreground;
  std::optional<RgbColor> cursor;
  std::array<RgbColor, 256> palette{};
};

class RenderState {
public:
  RenderState();
  ~RenderState();

  RenderState(RenderState &&other) noexcept;
  RenderState &operator=(RenderState &&other) noexcept;

  RenderState(const RenderState &) = delete;
  RenderState &operator=(const RenderState &) = delete;

  void update(Terminal &terminal);
  void set_dirty(Dirty dirty);

  [[nodiscard]] Dirty dirty() const;
  [[nodiscard]] std::uint16_t cols() const;
  [[nodiscard]] std::uint16_t rows() const;
  [[nodiscard]] RenderColors colors() const;
  [[nodiscard]] bool cursor_visible() const;
  [[nodiscard]] bool cursor_blinking() const;
  [[nodiscard]] bool cursor_password_input() const;
  [[nodiscard]] CursorVisualStyle cursor_visual_style() const;
  [[nodiscard]] std::optional<CursorViewport> cursor_viewport() const;

private:
  void release() noexcept;

  libghostty_cpp_render_state *handle_ = nullptr;
  std::size_t generation_ = 0;
};

} // namespace libghostty_cpp
