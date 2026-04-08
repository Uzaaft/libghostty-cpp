#include "libghostty_cpp/render.hpp"

#include "c_compat/render.h"
#include "libghostty_cpp/vt.hpp"
#include "result.hpp"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace libghostty_cpp {

namespace {

RgbColor translate_color(libghostty_cpp_rgb_color color) noexcept {
  return RgbColor{color.r, color.g, color.b};
}

StyleColor translate_style_color(libghostty_cpp_style_color color) {
  switch (color.tag) {
    case LIBGHOSTTY_CPP_STYLE_COLOR_NONE:
      return std::monostate{};
    case LIBGHOSTTY_CPP_STYLE_COLOR_PALETTE:
      return PaletteColor{color.value.palette};
    case LIBGHOSTTY_CPP_STYLE_COLOR_RGB:
      return translate_color(color.value.rgb);
  }

  throw Error(ErrorCode::InvalidValue);
}

Underline translate_underline(libghostty_cpp_underline underline) {
  switch (underline) {
    case LIBGHOSTTY_CPP_UNDERLINE_NONE:
      return Underline::None;
    case LIBGHOSTTY_CPP_UNDERLINE_SINGLE:
      return Underline::Single;
    case LIBGHOSTTY_CPP_UNDERLINE_DOUBLE:
      return Underline::Double;
    case LIBGHOSTTY_CPP_UNDERLINE_CURLY:
      return Underline::Curly;
    case LIBGHOSTTY_CPP_UNDERLINE_DOTTED:
      return Underline::Dotted;
    case LIBGHOSTTY_CPP_UNDERLINE_DASHED:
      return Underline::Dashed;
  }

  throw Error(ErrorCode::InvalidValue);
}

Style translate_style(libghostty_cpp_style style) {
  Style translated{};
  translated.fg_color = translate_style_color(style.fg_color);
  translated.bg_color = translate_style_color(style.bg_color);
  translated.underline_color = translate_style_color(style.underline_color);
  translated.bold = style.bold;
  translated.italic = style.italic;
  translated.faint = style.faint;
  translated.blink = style.blink;
  translated.inverse = style.inverse;
  translated.invisible = style.invisible;
  translated.strikethrough = style.strikethrough;
  translated.overline = style.overline;
  translated.underline = translate_underline(style.underline);
  return translated;
}

RowSemanticPrompt translate_row_semantic_prompt(
  libghostty_cpp_row_semantic_prompt semantic_prompt
) {
  switch (semantic_prompt) {
    case LIBGHOSTTY_CPP_ROW_SEMANTIC_PROMPT_NONE:
      return RowSemanticPrompt::None;
    case LIBGHOSTTY_CPP_ROW_SEMANTIC_PROMPT_PROMPT:
      return RowSemanticPrompt::Prompt;
    case LIBGHOSTTY_CPP_ROW_SEMANTIC_PROMPT_CONTINUATION:
      return RowSemanticPrompt::Continuation;
  }

  throw Error(ErrorCode::InvalidValue);
}

CellContentTag translate_cell_content_tag(
  libghostty_cpp_cell_content_tag content_tag
) {
  switch (content_tag) {
    case LIBGHOSTTY_CPP_CELL_CONTENT_TAG_CODEPOINT:
      return CellContentTag::Codepoint;
    case LIBGHOSTTY_CPP_CELL_CONTENT_TAG_CODEPOINT_GRAPHEME:
      return CellContentTag::CodepointGrapheme;
    case LIBGHOSTTY_CPP_CELL_CONTENT_TAG_BG_COLOR_PALETTE:
      return CellContentTag::BgColorPalette;
    case LIBGHOSTTY_CPP_CELL_CONTENT_TAG_BG_COLOR_RGB:
      return CellContentTag::BgColorRgb;
  }

  throw Error(ErrorCode::InvalidValue);
}

CellWide translate_cell_wide(libghostty_cpp_cell_wide wide) {
  switch (wide) {
    case LIBGHOSTTY_CPP_CELL_WIDE_NARROW:
      return CellWide::Narrow;
    case LIBGHOSTTY_CPP_CELL_WIDE_WIDE:
      return CellWide::Wide;
    case LIBGHOSTTY_CPP_CELL_WIDE_SPACER_TAIL:
      return CellWide::SpacerTail;
    case LIBGHOSTTY_CPP_CELL_WIDE_SPACER_HEAD:
      return CellWide::SpacerHead;
  }

  throw Error(ErrorCode::InvalidValue);
}

CellSemanticContent translate_cell_semantic_content(
  libghostty_cpp_cell_semantic_content semantic_content
) {
  switch (semantic_content) {
    case LIBGHOSTTY_CPP_CELL_SEMANTIC_CONTENT_OUTPUT:
      return CellSemanticContent::Output;
    case LIBGHOSTTY_CPP_CELL_SEMANTIC_CONTENT_INPUT:
      return CellSemanticContent::Input;
    case LIBGHOSTTY_CPP_CELL_SEMANTIC_CONTENT_PROMPT:
      return CellSemanticContent::Prompt;
  }

  throw Error(ErrorCode::InvalidValue);
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

bool get_row_bool(
  std::uint64_t row,
  libghostty_cpp_result (*getter)(libghostty_cpp_row, bool *)
) {
  bool value = false;
  detail::throw_if_error(getter(row, &value));
  return value;
}

std::uint32_t get_cell_u32(
  std::uint64_t cell,
  libghostty_cpp_result (*getter)(libghostty_cpp_cell, std::uint32_t *)
) {
  std::uint32_t value = 0;
  detail::throw_if_error(getter(cell, &value));
  return value;
}

bool get_cell_bool(
  std::uint64_t cell,
  libghostty_cpp_result (*getter)(libghostty_cpp_cell, bool *)
) {
  bool value = false;
  detail::throw_if_error(getter(cell, &value));
  return value;
}

std::uint16_t get_cell_u16(
  std::uint64_t cell,
  libghostty_cpp_result (*getter)(libghostty_cpp_cell, std::uint16_t *)
) {
  std::uint16_t value = 0;
  detail::throw_if_error(getter(cell, &value));
  return value;
}

} // namespace

bool Style::is_default() const noexcept {
  return std::holds_alternative<std::monostate>(fg_color)
         && std::holds_alternative<std::monostate>(bg_color)
         && std::holds_alternative<std::monostate>(underline_color)
         && !bold
         && !italic
         && !faint
         && !blink
         && !inverse
         && !invisible
         && !strikethrough
         && !overline
         && underline == Underline::None;
}

Row::Row(
  std::uint64_t raw,
  const RenderState *render_state,
  std::size_t generation
) noexcept
    : raw_(raw), render_state_(render_state), generation_(generation) {}

void Row::ensure_current() const {
  if (render_state_ == nullptr || render_state_->generation_ != generation_) {
    throw Error(ErrorCode::InvalidState);
  }
}

bool Row::is_wrapped() const {
  ensure_current();
  return get_row_bool(raw_, libghostty_cpp_row_is_wrapped);
}

bool Row::is_wrap_continuation() const {
  ensure_current();
  return get_row_bool(raw_, libghostty_cpp_row_is_wrap_continuation);
}

bool Row::has_grapheme_cluster() const {
  ensure_current();
  return get_row_bool(raw_, libghostty_cpp_row_has_grapheme_cluster);
}

bool Row::is_styled() const {
  ensure_current();
  return get_row_bool(raw_, libghostty_cpp_row_is_styled);
}

bool Row::has_hyperlink() const {
  ensure_current();
  return get_row_bool(raw_, libghostty_cpp_row_has_hyperlink);
}

RowSemanticPrompt Row::semantic_prompt() const {
  ensure_current();
  libghostty_cpp_row_semantic_prompt semantic_prompt =
    LIBGHOSTTY_CPP_ROW_SEMANTIC_PROMPT_NONE;
  detail::throw_if_error(libghostty_cpp_row_get_semantic_prompt(raw_, &semantic_prompt));
  return translate_row_semantic_prompt(semantic_prompt);
}

bool Row::has_kitty_virtual_placeholder() const {
  ensure_current();
  return get_row_bool(raw_, libghostty_cpp_row_has_kitty_virtual_placeholder);
}

bool Row::is_dirty() const {
  ensure_current();
  return get_row_bool(raw_, libghostty_cpp_row_is_dirty);
}

Cell::Cell(
  std::uint64_t raw,
  const RenderState *render_state,
  std::size_t generation
) noexcept
    : raw_(raw), render_state_(render_state), generation_(generation) {}

void Cell::ensure_current() const {
  if (render_state_ == nullptr || render_state_->generation_ != generation_) {
    throw Error(ErrorCode::InvalidState);
  }
}

std::uint32_t Cell::codepoint() const {
  ensure_current();
  return get_cell_u32(raw_, libghostty_cpp_cell_codepoint);
}

CellContentTag Cell::content_tag() const {
  ensure_current();
  libghostty_cpp_cell_content_tag content_tag = LIBGHOSTTY_CPP_CELL_CONTENT_TAG_CODEPOINT;
  detail::throw_if_error(libghostty_cpp_cell_get_content_tag(raw_, &content_tag));
  return translate_cell_content_tag(content_tag);
}

CellWide Cell::wide() const {
  ensure_current();
  libghostty_cpp_cell_wide wide = LIBGHOSTTY_CPP_CELL_WIDE_NARROW;
  detail::throw_if_error(libghostty_cpp_cell_get_wide(raw_, &wide));
  return translate_cell_wide(wide);
}

bool Cell::has_text() const {
  ensure_current();
  return get_cell_bool(raw_, libghostty_cpp_cell_has_text);
}

bool Cell::has_styling() const {
  ensure_current();
  return get_cell_bool(raw_, libghostty_cpp_cell_has_styling);
}

bool Cell::has_hyperlink() const {
  ensure_current();
  return get_cell_bool(raw_, libghostty_cpp_cell_has_hyperlink);
}

bool Cell::is_protected() const {
  ensure_current();
  return get_cell_bool(raw_, libghostty_cpp_cell_is_protected);
}

CellSemanticContent Cell::semantic_content() const {
  ensure_current();
  libghostty_cpp_cell_semantic_content semantic_content =
    LIBGHOSTTY_CPP_CELL_SEMANTIC_CONTENT_OUTPUT;
  detail::throw_if_error(
    libghostty_cpp_cell_get_semantic_content(raw_, &semantic_content)
  );
  return translate_cell_semantic_content(semantic_content);
}

std::uint16_t Cell::style_id() const {
  ensure_current();
  return get_cell_u16(raw_, libghostty_cpp_cell_style_id);
}

std::uint8_t Cell::bg_color_palette() const {
  ensure_current();
  std::uint8_t palette = 0;
  detail::throw_if_error(libghostty_cpp_cell_bg_color_palette(raw_, &palette));
  return palette;
}

RgbColor Cell::bg_color_rgb() const {
  ensure_current();
  libghostty_cpp_rgb_color color = {};
  detail::throw_if_error(libghostty_cpp_cell_bg_color_rgb(raw_, &color));
  return translate_color(color);
}

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

RowIterator::RowIterator() {
  libghostty_cpp_render_state_row_iterator *handle = nullptr;
  detail::throw_if_error(libghostty_cpp_render_state_row_iterator_new(&handle));
  handle_ = handle;
}

RowIterator::~RowIterator() {
  release();
}

RowIterator::RowIterator(RowIterator &&other) noexcept
    : handle_(std::exchange(other.handle_, nullptr)),
      render_state_(std::exchange(other.render_state_, nullptr)),
      generation_(std::exchange(other.generation_, 0)),
      row_revision_(std::exchange(other.row_revision_, 0)),
      positioned_(std::exchange(other.positioned_, false)) {}

RowIterator &RowIterator::operator=(RowIterator &&other) noexcept {
  if (this == &other) {
    return *this;
  }

  release();
  handle_ = std::exchange(other.handle_, nullptr);
  render_state_ = std::exchange(other.render_state_, nullptr);
  generation_ = std::exchange(other.generation_, 0);
  row_revision_ = std::exchange(other.row_revision_, 0);
  positioned_ = std::exchange(other.positioned_, false);
  return *this;
}

void RowIterator::bind(RenderState &render_state) {
  detail::throw_if_error(
    libghostty_cpp_render_state_row_iterator_update(render_state.handle_, handle_)
  );
  render_state_ = &render_state;
  generation_ = render_state.generation_;
  row_revision_ = 0;
  positioned_ = false;
}

bool RowIterator::next() {
  ensure_bound();

  positioned_ = libghostty_cpp_render_state_row_iterator_next(handle_);
  if (positioned_) {
    row_revision_ += 1;
  }

  return positioned_;
}

bool RowIterator::dirty() const {
  ensure_positioned();

  bool dirty = false;
  detail::throw_if_error(libghostty_cpp_render_state_row_iterator_dirty(handle_, &dirty));
  return dirty;
}

void RowIterator::set_dirty(bool dirty) {
  ensure_positioned();

  detail::throw_if_error(libghostty_cpp_render_state_row_iterator_set_dirty(handle_, dirty));
}

Row RowIterator::row() const {
  ensure_positioned();

  libghostty_cpp_row row = 0;
  detail::throw_if_error(libghostty_cpp_render_state_row_iterator_row(handle_, &row));
  return Row(row, render_state_, generation_);
}

void RowIterator::ensure_bound() const {
  if (render_state_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }

  if (render_state_->generation_ != generation_) {
    throw Error(ErrorCode::InvalidState);
  }
}

void RowIterator::ensure_positioned() const {
  ensure_bound();

  if (!positioned_) {
    throw Error(ErrorCode::InvalidState);
  }
}

void RowIterator::release() noexcept {
  libghostty_cpp_render_state_row_iterator_free(handle_);
  handle_ = nullptr;
  render_state_ = nullptr;
  generation_ = 0;
  row_revision_ = 0;
  positioned_ = false;
}

CellIterator::CellIterator() {
  libghostty_cpp_render_state_row_cells *handle = nullptr;
  detail::throw_if_error(libghostty_cpp_render_state_row_cells_new(&handle));
  handle_ = handle;
}

CellIterator::~CellIterator() {
  release();
}

CellIterator::CellIterator(CellIterator &&other) noexcept
    : handle_(std::exchange(other.handle_, nullptr)),
      row_iterator_(std::exchange(other.row_iterator_, nullptr)),
      generation_(std::exchange(other.generation_, 0)),
      row_revision_(std::exchange(other.row_revision_, 0)),
      positioned_(std::exchange(other.positioned_, false)) {}

CellIterator &CellIterator::operator=(CellIterator &&other) noexcept {
  if (this == &other) {
    return *this;
  }

  release();
  handle_ = std::exchange(other.handle_, nullptr);
  row_iterator_ = std::exchange(other.row_iterator_, nullptr);
  generation_ = std::exchange(other.generation_, 0);
  row_revision_ = std::exchange(other.row_revision_, 0);
  positioned_ = std::exchange(other.positioned_, false);
  return *this;
}

void CellIterator::bind(RowIterator &row_iterator) {
  row_iterator.ensure_positioned();

  detail::throw_if_error(
    libghostty_cpp_render_state_row_cells_update(row_iterator.handle_, handle_)
  );
  row_iterator_ = &row_iterator;
  generation_ = row_iterator.generation_;
  row_revision_ = row_iterator.row_revision_;
  positioned_ = false;
}

bool CellIterator::next() {
  ensure_bound();

  positioned_ = libghostty_cpp_render_state_row_cells_next(handle_);
  return positioned_;
}

void CellIterator::select(std::uint16_t x) {
  ensure_bound();

  detail::throw_if_error(libghostty_cpp_render_state_row_cells_select(handle_, x));
  positioned_ = true;
}

Cell CellIterator::cell() const {
  ensure_positioned();

  libghostty_cpp_cell cell = 0;
  detail::throw_if_error(libghostty_cpp_render_state_row_cells_cell(handle_, &cell));
  return Cell(cell, row_iterator_->render_state_, generation_);
}

Style CellIterator::style() const {
  ensure_positioned();

  libghostty_cpp_style style = {};
  detail::throw_if_error(libghostty_cpp_render_state_row_cells_style(handle_, &style));
  return translate_style(style);
}

std::optional<RgbColor> CellIterator::resolved_fg_color() const {
  ensure_positioned();

  bool has_value = false;
  libghostty_cpp_rgb_color color = {};
  detail::throw_if_error(
    libghostty_cpp_render_state_row_cells_fg_color(handle_, &has_value, &color)
  );

  if (!has_value) {
    return std::nullopt;
  }

  return translate_color(color);
}

std::optional<RgbColor> CellIterator::resolved_bg_color() const {
  ensure_positioned();

  bool has_value = false;
  libghostty_cpp_rgb_color color = {};
  detail::throw_if_error(
    libghostty_cpp_render_state_row_cells_bg_color(handle_, &has_value, &color)
  );

  if (!has_value) {
    return std::nullopt;
  }

  return translate_color(color);
}

std::size_t CellIterator::graphemes_len() const {
  ensure_positioned();

  std::size_t len = 0;
  detail::throw_if_error(libghostty_cpp_render_state_row_cells_graphemes_len(handle_, &len));
  return len;
}

std::u32string CellIterator::graphemes() const {
  ensure_positioned();

  const std::size_t len = graphemes_len();
  if (len == 0) {
    return {};
  }

  std::vector<std::uint32_t> codepoints(len, 0);
  detail::throw_if_error(
    libghostty_cpp_render_state_row_cells_graphemes(handle_, codepoints.data())
  );

  std::u32string graphemes;
  graphemes.reserve(len);
  for (const std::uint32_t codepoint : codepoints) {
    graphemes.push_back(static_cast<char32_t>(codepoint));
  }

  return graphemes;
}

void CellIterator::ensure_bound() const {
  if (row_iterator_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }

  row_iterator_->ensure_positioned();

  if (row_iterator_->generation_ != generation_
      || row_iterator_->row_revision_ != row_revision_) {
    throw Error(ErrorCode::InvalidState);
  }
}

void CellIterator::ensure_positioned() const {
  ensure_bound();

  if (!positioned_) {
    throw Error(ErrorCode::InvalidState);
  }
}

void CellIterator::release() noexcept {
  libghostty_cpp_render_state_row_cells_free(handle_);
  handle_ = nullptr;
  row_iterator_ = nullptr;
  generation_ = 0;
  row_revision_ = 0;
  positioned_ = false;
}

} // namespace libghostty_cpp
