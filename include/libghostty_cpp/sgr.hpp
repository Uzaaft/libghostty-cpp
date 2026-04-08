#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

#include "libghostty_cpp/style.hpp"

namespace libghostty_cpp::sgr {

enum class AttributeTag {
  Unset,
  Unknown,
  Bold,
  ResetBold,
  Italic,
  ResetItalic,
  Faint,
  Underline,
  UnderlineColor,
  UnderlineColor256,
  ResetUnderlineColor,
  Overline,
  ResetOverline,
  Blink,
  ResetBlink,
  Inverse,
  ResetInverse,
  Invisible,
  ResetInvisible,
  Strikethrough,
  ResetStrikethrough,
  DirectColorFg,
  DirectColorBg,
  Bg8,
  Fg8,
  ResetFg,
  ResetBg,
  BrightBg8,
  BrightFg8,
  Bg256,
  Fg256,
};

struct UnknownData {
  std::vector<std::uint16_t> full;
  std::vector<std::uint16_t> partial;
};

using AttributeValue = std::variant<std::monostate, UnknownData, Underline, RgbColor, PaletteColor>;

struct Attribute {
  AttributeTag tag = AttributeTag::Unset;
  AttributeValue value{};
};

class Parser {
public:
  Parser();
  ~Parser();

  Parser(Parser&& other) noexcept;
  Parser& operator=(Parser&& other) noexcept;

  Parser(const Parser&) = delete;
  Parser& operator=(const Parser&) = delete;

  void set_params(
    const std::vector<std::uint16_t>& params,
    std::optional<std::string_view> separators = std::nullopt
  );
  [[nodiscard]] std::optional<Attribute> next();
  void reset() noexcept;

private:
  void release() noexcept;

  void* handle_ = nullptr;
};

} // namespace libghostty_cpp::sgr
