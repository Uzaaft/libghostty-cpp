#include "libghostty_cpp/render.hpp"

#include "c_compat/render.h"
#include "libghostty_cpp/vt.hpp"
#include "result.hpp"

#include <cstddef>
#include <utility>

namespace libghostty_cpp {

namespace {

RgbColor translate_color(libghostty_cpp_rgb_color color) noexcept {
  return RgbColor{color.r, color.g, color.b};
}

Dirty translate_dirty(libghostty_cpp_render_dirty dirty) {
  switch (dirty) {
    case LIBGHOSTTY_CPP_RENDER_DIRTY_CLEAN:
      return Dirty::Clean;
    case LIBGHOSTTY_CPP_RENDER_DIRTY_PARTIAL:
      return Dirty::Partial;
    case LIBGHOSTTY_CPP_RENDER_DIRTY_FULL:
      return Dirty::Full;
  }

  throw Error(ErrorCode::InvalidValue);
}

libghostty_cpp_render_dirty translate_dirty(Dirty dirty) {
  switch (dirty) {
    case Dirty::Clean:
      return LIBGHOSTTY_CPP_RENDER_DIRTY_CLEAN;
    case Dirty::Partial:
      return LIBGHOSTTY_CPP_RENDER_DIRTY_PARTIAL;
    case Dirty::Full:
      return LIBGHOSTTY_CPP_RENDER_DIRTY_FULL;
  }

  throw Error(ErrorCode::InvalidValue);
}

CursorVisualStyle translate_cursor_visual_style(
  libghostty_cpp_cursor_visual_style style
) {
  switch (style) {
    case LIBGHOSTTY_CPP_CURSOR_VISUAL_STYLE_BAR:
      return CursorVisualStyle::Bar;
    case LIBGHOSTTY_CPP_CURSOR_VISUAL_STYLE_BLOCK:
      return CursorVisualStyle::Block;
    case LIBGHOSTTY_CPP_CURSOR_VISUAL_STYLE_UNDERLINE:
      return CursorVisualStyle::Underline;
    case LIBGHOSTTY_CPP_CURSOR_VISUAL_STYLE_BLOCK_HOLLOW:
      return CursorVisualStyle::BlockHollow;
  }

  throw Error(ErrorCode::InvalidValue);
}

std::uint16_t get_u16(
  libghostty_cpp_render_state *handle,
  libghostty_cpp_result (*getter)(libghostty_cpp_render_state *, std::uint16_t *)
) {
  std::uint16_t value = 0;
  detail::throw_if_error(getter(handle, &value));
  return value;
}

bool get_bool(
  libghostty_cpp_render_state *handle,
  libghostty_cpp_result (*getter)(libghostty_cpp_render_state *, bool *)
) {
  bool value = false;
  detail::throw_if_error(getter(handle, &value));
  return value;
}

} // namespace

RenderState::RenderState() {
  libghostty_cpp_render_state *handle = nullptr;
  detail::throw_if_error(libghostty_cpp_render_state_new(&handle));
  handle_ = handle;
}

RenderState::~RenderState() {
  release();
}

RenderState::RenderState(RenderState &&other) noexcept
    : handle_(std::exchange(other.handle_, nullptr)),
      generation_(std::exchange(other.generation_, 0)) {}

RenderState &RenderState::operator=(RenderState &&other) noexcept {
  if (this == &other) {
    return *this;
  }

  release();
  handle_ = std::exchange(other.handle_, nullptr);
  generation_ = std::exchange(other.generation_, 0);
  return *this;
}

void RenderState::update(Terminal &terminal) {
  detail::throw_if_error(libghostty_cpp_render_state_update(handle_, terminal.handle_));
  generation_ += 1;
}

void RenderState::set_dirty(Dirty dirty) {
  detail::throw_if_error(
    libghostty_cpp_render_state_set_dirty(handle_, translate_dirty(dirty))
  );
}

Dirty RenderState::dirty() const {
  libghostty_cpp_render_dirty dirty = LIBGHOSTTY_CPP_RENDER_DIRTY_CLEAN;
  detail::throw_if_error(libghostty_cpp_render_state_dirty(handle_, &dirty));
  return translate_dirty(dirty);
}

std::uint16_t RenderState::cols() const {
  return get_u16(handle_, libghostty_cpp_render_state_cols);
}

std::uint16_t RenderState::rows() const {
  return get_u16(handle_, libghostty_cpp_render_state_rows);
}

RenderColors RenderState::colors() const {
  libghostty_cpp_render_colors raw_colors = {};
  detail::throw_if_error(libghostty_cpp_render_state_colors(handle_, &raw_colors));

  RenderColors colors{};
  colors.background = translate_color(raw_colors.background);
  colors.foreground = translate_color(raw_colors.foreground);

  if (raw_colors.cursor_has_value) {
    colors.cursor = translate_color(raw_colors.cursor);
  }

  for (std::size_t i = 0; i < colors.palette.size(); ++i) {
    colors.palette[i] = translate_color(raw_colors.palette[i]);
  }

  return colors;
}

bool RenderState::cursor_visible() const {
  return get_bool(handle_, libghostty_cpp_render_state_cursor_visible);
}

bool RenderState::cursor_blinking() const {
  return get_bool(handle_, libghostty_cpp_render_state_cursor_blinking);
}

bool RenderState::cursor_password_input() const {
  return get_bool(handle_, libghostty_cpp_render_state_cursor_password_input);
}

CursorVisualStyle RenderState::cursor_visual_style() const {
  libghostty_cpp_cursor_visual_style style = LIBGHOSTTY_CPP_CURSOR_VISUAL_STYLE_BLOCK;
  detail::throw_if_error(libghostty_cpp_render_state_cursor_visual_style(handle_, &style));
  return translate_cursor_visual_style(style);
}

std::optional<CursorViewport> RenderState::cursor_viewport() const {
  bool has_value = false;
  libghostty_cpp_cursor_viewport raw_viewport = {};
  detail::throw_if_error(
    libghostty_cpp_render_state_cursor_viewport(handle_, &has_value, &raw_viewport)
  );

  if (!has_value) {
    return std::nullopt;
  }

  return CursorViewport{raw_viewport.x, raw_viewport.y, raw_viewport.at_wide_tail};
}

void RenderState::release() noexcept {
  libghostty_cpp_render_state_free(handle_);
  handle_ = nullptr;
  generation_ = 0;
}

} // namespace libghostty_cpp
