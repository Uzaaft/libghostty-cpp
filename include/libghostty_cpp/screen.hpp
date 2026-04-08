#pragma once

#include <cstdint>

#include "libghostty_cpp/error.hpp"
#include "libghostty_cpp/style.hpp"

namespace libghostty_cpp {
class GridRef;
class Terminal;
}

namespace libghostty_cpp::screen {

using StyleID = std::uint16_t;

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
  explicit Row(std::uint64_t raw) noexcept : raw_(raw) {}

  std::uint64_t raw_ = 0;

  friend class ::libghostty_cpp::GridRef;
  friend class ::libghostty_cpp::Terminal;
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
  [[nodiscard]] StyleID style_id() const;
  [[nodiscard]] bool has_hyperlink() const;
  [[nodiscard]] bool is_protected() const;
  [[nodiscard]] CellSemanticContent semantic_content() const;
  [[nodiscard]] std::uint8_t bg_color_palette() const;
  [[nodiscard]] RgbColor bg_color_rgb() const;

private:
  explicit Cell(std::uint64_t raw) noexcept : raw_(raw) {}

  std::uint64_t raw_ = 0;

  friend class ::libghostty_cpp::GridRef;
  friend class ::libghostty_cpp::Terminal;
};

} // namespace libghostty_cpp::screen
