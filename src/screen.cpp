#include "libghostty_cpp/screen.hpp"

#include "c_compat/render.h"
#include "libghostty_cpp/style.hpp"
#include "result.hpp"

namespace libghostty_cpp::screen {

namespace {

bool get_row_bool(
  std::uint64_t row,
  libghostty_cpp_result (*getter)(libghostty_cpp_row, bool*)
) {
  bool value = false;
  detail::throw_if_error(getter(row, &value));
  return value;
}

std::uint32_t get_cell_u32(
  std::uint64_t cell,
  libghostty_cpp_result (*getter)(libghostty_cpp_cell, std::uint32_t*)
) {
  std::uint32_t value = 0;
  detail::throw_if_error(getter(cell, &value));
  return value;
}

std::uint16_t get_cell_u16(
  std::uint64_t cell,
  libghostty_cpp_result (*getter)(libghostty_cpp_cell, std::uint16_t*)
) {
  std::uint16_t value = 0;
  detail::throw_if_error(getter(cell, &value));
  return value;
}

bool get_cell_bool(
  std::uint64_t cell,
  libghostty_cpp_result (*getter)(libghostty_cpp_cell, bool*)
) {
  bool value = false;
  detail::throw_if_error(getter(cell, &value));
  return value;
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

} // namespace

bool Row::is_wrapped() const {
  return get_row_bool(raw_, libghostty_cpp_row_is_wrapped);
}

bool Row::is_wrap_continuation() const {
  return get_row_bool(raw_, libghostty_cpp_row_is_wrap_continuation);
}

bool Row::has_grapheme_cluster() const {
  return get_row_bool(raw_, libghostty_cpp_row_has_grapheme_cluster);
}

bool Row::is_styled() const {
  return get_row_bool(raw_, libghostty_cpp_row_is_styled);
}

bool Row::has_hyperlink() const {
  return get_row_bool(raw_, libghostty_cpp_row_has_hyperlink);
}

RowSemanticPrompt Row::semantic_prompt() const {
  libghostty_cpp_row_semantic_prompt semantic_prompt =
    LIBGHOSTTY_CPP_ROW_SEMANTIC_PROMPT_NONE;
  detail::throw_if_error(libghostty_cpp_row_get_semantic_prompt(raw_, &semantic_prompt));
  return translate_row_semantic_prompt(semantic_prompt);
}

bool Row::has_kitty_virtual_placeholder() const {
  return get_row_bool(raw_, libghostty_cpp_row_has_kitty_virtual_placeholder);
}

bool Row::is_dirty() const {
  return get_row_bool(raw_, libghostty_cpp_row_is_dirty);
}

std::uint32_t Cell::codepoint() const {
  return get_cell_u32(raw_, libghostty_cpp_cell_codepoint);
}

CellContentTag Cell::content_tag() const {
  libghostty_cpp_cell_content_tag content_tag = LIBGHOSTTY_CPP_CELL_CONTENT_TAG_CODEPOINT;
  detail::throw_if_error(libghostty_cpp_cell_get_content_tag(raw_, &content_tag));
  return translate_cell_content_tag(content_tag);
}

CellWide Cell::wide() const {
  libghostty_cpp_cell_wide wide = LIBGHOSTTY_CPP_CELL_WIDE_NARROW;
  detail::throw_if_error(libghostty_cpp_cell_get_wide(raw_, &wide));
  return translate_cell_wide(wide);
}

bool Cell::has_text() const {
  return get_cell_bool(raw_, libghostty_cpp_cell_has_text);
}

bool Cell::has_styling() const {
  return get_cell_bool(raw_, libghostty_cpp_cell_has_styling);
}

StyleID Cell::style_id() const {
  return get_cell_u16(raw_, libghostty_cpp_cell_style_id);
}

bool Cell::has_hyperlink() const {
  return get_cell_bool(raw_, libghostty_cpp_cell_has_hyperlink);
}

bool Cell::is_protected() const {
  return get_cell_bool(raw_, libghostty_cpp_cell_is_protected);
}

CellSemanticContent Cell::semantic_content() const {
  libghostty_cpp_cell_semantic_content semantic_content =
    LIBGHOSTTY_CPP_CELL_SEMANTIC_CONTENT_OUTPUT;
  detail::throw_if_error(
    libghostty_cpp_cell_get_semantic_content(raw_, &semantic_content)
  );
  return translate_cell_semantic_content(semantic_content);
}

std::uint8_t Cell::bg_color_palette() const {
  std::uint8_t palette = 0;
  detail::throw_if_error(libghostty_cpp_cell_bg_color_palette(raw_, &palette));
  return palette;
}

RgbColor Cell::bg_color_rgb() const {
  libghostty_cpp_rgb_color color = {};
  detail::throw_if_error(libghostty_cpp_cell_bg_color_rgb(raw_, &color));
  return RgbColor{color.r, color.g, color.b};
}

} // namespace libghostty_cpp::screen
