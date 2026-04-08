#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include "libghostty_cpp/error.hpp"
#include "libghostty_cpp/style.hpp"

struct libghostty_cpp_render_state;
struct libghostty_cpp_render_state_row_iterator;
struct libghostty_cpp_render_state_row_cells;

namespace libghostty_cpp {

class Terminal;
class RenderState;
class RowIterator;
class CellIterator;

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

enum class RowSemanticPrompt {
  None,
  Prompt,
  Continuation,
};

class Row {
public:
  [[nodiscard]] bool is_wrapped() const;
  [[nodiscard]] bool is_wrap_continuation() const;
  [[nodiscard]] bool has_grapheme_cluster() const;
  [[nodiscard]] bool is_styled() const;
  [[nodiscard]] bool has_hyperlink() const;
  [[nodiscard]] RowSemanticPrompt semantic_prompt() const;
  [[nodiscard]] bool has_kitty_virtual_placeholder() const;
  [[nodiscard]] bool is_dirty() const;

private:
  explicit Row(
    std::uint64_t raw,
    const RenderState *render_state,
    std::size_t generation
  ) noexcept;

  void ensure_current() const;

  std::uint64_t raw_ = 0;
  const RenderState *render_state_ = nullptr;
  std::size_t generation_ = 0;

  friend class RowIterator;
};

enum class CellContentTag {
  Codepoint,
  CodepointGrapheme,
  BgColorPalette,
  BgColorRgb,
};

enum class CellWide {
  Narrow,
  Wide,
  SpacerTail,
  SpacerHead,
};

enum class CellSemanticContent {
  Output,
  Input,
  Prompt,
};

class Cell {
public:
  [[nodiscard]] std::uint32_t codepoint() const;
  [[nodiscard]] CellContentTag content_tag() const;
  [[nodiscard]] CellWide wide() const;
  [[nodiscard]] bool has_text() const;
  [[nodiscard]] bool has_styling() const;
  [[nodiscard]] bool has_hyperlink() const;
  [[nodiscard]] bool is_protected() const;
  [[nodiscard]] CellSemanticContent semantic_content() const;

private:
  explicit Cell(
    std::uint64_t raw,
    const RenderState *render_state,
    std::size_t generation
  ) noexcept;

  void ensure_current() const;

  std::uint64_t raw_ = 0;
  const RenderState *render_state_ = nullptr;
  std::size_t generation_ = 0;

  friend class CellIterator;
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
  friend class Row;
  friend class Cell;
  friend class RowIterator;

  void release() noexcept;

  libghostty_cpp_render_state *handle_ = nullptr;
  std::size_t generation_ = 0;
};

class RowIterator {
public:
  RowIterator();
  ~RowIterator();

  RowIterator(RowIterator &&other) noexcept;
  RowIterator &operator=(RowIterator &&other) noexcept;

  RowIterator(const RowIterator &) = delete;
  RowIterator &operator=(const RowIterator &) = delete;

  // Rebind after each RenderState::update. Row data from an older frame is invalid.
  void bind(RenderState &render_state);
  [[nodiscard]] bool next();
  [[nodiscard]] bool dirty() const;
  void set_dirty(bool dirty);
  [[nodiscard]] Row row() const;

private:
  friend class CellIterator;

  void ensure_bound() const;
  void ensure_positioned() const;
  void release() noexcept;

  libghostty_cpp_render_state_row_iterator *handle_ = nullptr;
  const RenderState *render_state_ = nullptr;
  std::size_t generation_ = 0;
  std::size_t row_revision_ = 0;
  bool positioned_ = false;
};

class CellIterator {
public:
  CellIterator();
  ~CellIterator();

  CellIterator(CellIterator &&other) noexcept;
  CellIterator &operator=(CellIterator &&other) noexcept;

  CellIterator(const CellIterator &) = delete;
  CellIterator &operator=(const CellIterator &) = delete;

  // Rebind after moving the row iterator or after advancing to a new frame.
  void bind(RowIterator &row_iterator);
  [[nodiscard]] bool next();
  void select(std::uint16_t x);
  [[nodiscard]] Cell cell() const;
  [[nodiscard]] Style style() const;
  [[nodiscard]] std::optional<RgbColor> resolved_fg_color() const;
  [[nodiscard]] std::optional<RgbColor> resolved_bg_color() const;
  [[nodiscard]] std::size_t graphemes_len() const;
  [[nodiscard]] std::u32string graphemes() const;

private:
  void ensure_bound() const;
  void ensure_positioned() const;
  void release() noexcept;

  libghostty_cpp_render_state_row_cells *handle_ = nullptr;
  const RowIterator *row_iterator_ = nullptr;
  std::size_t generation_ = 0;
  std::size_t row_revision_ = 0;
  bool positioned_ = false;
};

} // namespace libghostty_cpp
