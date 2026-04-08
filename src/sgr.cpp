#include "libghostty_cpp/sgr.hpp"

#include <ghostty/vt.h>

#include "ghostty_result.hpp"

#include <utility>

namespace libghostty_cpp::sgr {

namespace {

UnknownData translate_unknown(GhosttySgrUnknown unknown) {
  const auto* full_ptr = static_cast<const std::uint16_t*>(unknown.full_ptr);
  const auto* partial_ptr = static_cast<const std::uint16_t*>(unknown.partial_ptr);
  return UnknownData{
    std::vector<std::uint16_t>(full_ptr, full_ptr + unknown.full_len),
    std::vector<std::uint16_t>(partial_ptr, partial_ptr + unknown.partial_len),
  };
}

Attribute translate_attribute(GhosttySgrAttribute attribute) {
  switch (attribute.tag) {
    case GHOSTTY_SGR_ATTR_UNSET:
      return {AttributeTag::Unset, std::monostate{}};
    case GHOSTTY_SGR_ATTR_UNKNOWN:
      return {AttributeTag::Unknown, translate_unknown(attribute.value.unknown)};
    case GHOSTTY_SGR_ATTR_BOLD:
      return {AttributeTag::Bold, std::monostate{}};
    case GHOSTTY_SGR_ATTR_RESET_BOLD:
      return {AttributeTag::ResetBold, std::monostate{}};
    case GHOSTTY_SGR_ATTR_ITALIC:
      return {AttributeTag::Italic, std::monostate{}};
    case GHOSTTY_SGR_ATTR_RESET_ITALIC:
      return {AttributeTag::ResetItalic, std::monostate{}};
    case GHOSTTY_SGR_ATTR_FAINT:
      return {AttributeTag::Faint, std::monostate{}};
    case GHOSTTY_SGR_ATTR_UNDERLINE:
      return {
        AttributeTag::Underline,
        static_cast<Underline>(attribute.value.underline),
      };
    case GHOSTTY_SGR_ATTR_UNDERLINE_COLOR:
      return {
        AttributeTag::UnderlineColor,
        RgbColor{
          attribute.value.underline_color.r,
          attribute.value.underline_color.g,
          attribute.value.underline_color.b,
        },
      };
    case GHOSTTY_SGR_ATTR_UNDERLINE_COLOR_256:
      return {
        AttributeTag::UnderlineColor256,
        PaletteColor{attribute.value.underline_color_256},
      };
    case GHOSTTY_SGR_ATTR_RESET_UNDERLINE_COLOR:
      return {AttributeTag::ResetUnderlineColor, std::monostate{}};
    case GHOSTTY_SGR_ATTR_OVERLINE:
      return {AttributeTag::Overline, std::monostate{}};
    case GHOSTTY_SGR_ATTR_RESET_OVERLINE:
      return {AttributeTag::ResetOverline, std::monostate{}};
    case GHOSTTY_SGR_ATTR_BLINK:
      return {AttributeTag::Blink, std::monostate{}};
    case GHOSTTY_SGR_ATTR_RESET_BLINK:
      return {AttributeTag::ResetBlink, std::monostate{}};
    case GHOSTTY_SGR_ATTR_INVERSE:
      return {AttributeTag::Inverse, std::monostate{}};
    case GHOSTTY_SGR_ATTR_RESET_INVERSE:
      return {AttributeTag::ResetInverse, std::monostate{}};
    case GHOSTTY_SGR_ATTR_INVISIBLE:
      return {AttributeTag::Invisible, std::monostate{}};
    case GHOSTTY_SGR_ATTR_RESET_INVISIBLE:
      return {AttributeTag::ResetInvisible, std::monostate{}};
    case GHOSTTY_SGR_ATTR_STRIKETHROUGH:
      return {AttributeTag::Strikethrough, std::monostate{}};
    case GHOSTTY_SGR_ATTR_RESET_STRIKETHROUGH:
      return {AttributeTag::ResetStrikethrough, std::monostate{}};
    case GHOSTTY_SGR_ATTR_DIRECT_COLOR_FG:
      return {
        AttributeTag::DirectColorFg,
        RgbColor{
          attribute.value.direct_color_fg.r,
          attribute.value.direct_color_fg.g,
          attribute.value.direct_color_fg.b,
        },
      };
    case GHOSTTY_SGR_ATTR_DIRECT_COLOR_BG:
      return {
        AttributeTag::DirectColorBg,
        RgbColor{
          attribute.value.direct_color_bg.r,
          attribute.value.direct_color_bg.g,
          attribute.value.direct_color_bg.b,
        },
      };
    case GHOSTTY_SGR_ATTR_BG_8:
      return {AttributeTag::Bg8, PaletteColor{attribute.value.bg_8}};
    case GHOSTTY_SGR_ATTR_FG_8:
      return {AttributeTag::Fg8, PaletteColor{attribute.value.fg_8}};
    case GHOSTTY_SGR_ATTR_RESET_FG:
      return {AttributeTag::ResetFg, std::monostate{}};
    case GHOSTTY_SGR_ATTR_RESET_BG:
      return {AttributeTag::ResetBg, std::monostate{}};
    case GHOSTTY_SGR_ATTR_BRIGHT_BG_8:
      return {AttributeTag::BrightBg8, PaletteColor{attribute.value.bright_bg_8}};
    case GHOSTTY_SGR_ATTR_BRIGHT_FG_8:
      return {AttributeTag::BrightFg8, PaletteColor{attribute.value.bright_fg_8}};
    case GHOSTTY_SGR_ATTR_BG_256:
      return {AttributeTag::Bg256, PaletteColor{attribute.value.bg_256}};
    case GHOSTTY_SGR_ATTR_FG_256:
      return {AttributeTag::Fg256, PaletteColor{attribute.value.fg_256}};
  }

  throw Error(ErrorCode::InvalidValue);
}

} // namespace

Parser::Parser() {
  GhosttySgrParser handle = nullptr;
  detail::throw_if_ghostty_error(ghostty_sgr_new(nullptr, &handle));
  handle_ = handle;
}

Parser::~Parser() {
  release();
}

Parser::Parser(Parser&& other) noexcept
    : handle_(std::exchange(other.handle_, nullptr)) {}

Parser& Parser::operator=(Parser&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  release();
  handle_ = std::exchange(other.handle_, nullptr);
  return *this;
}

void Parser::set_params(
  const std::vector<std::uint16_t>& params,
  std::optional<std::string_view> separators
) {
  if (handle_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }

  if (separators.has_value() && separators->size() != params.size()) {
    throw Error(ErrorCode::InvalidValue);
  }

  const char* separator_ptr = separators.has_value() ? separators->data() : nullptr;
  detail::throw_if_ghostty_error(
    ghostty_sgr_set_params(
      static_cast<GhosttySgrParser>(handle_),
      params.data(),
      separator_ptr,
      params.size()
    )
  );
}

std::optional<Attribute> Parser::next() {
  if (handle_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }

  GhosttySgrAttribute attribute = {};
  if (!ghostty_sgr_next(static_cast<GhosttySgrParser>(handle_), &attribute)) {
    return std::nullopt;
  }

  return translate_attribute(attribute);
}

void Parser::reset() noexcept {
  if (handle_ != nullptr) {
    ghostty_sgr_reset(static_cast<GhosttySgrParser>(handle_));
  }
}

void Parser::release() noexcept {
  if (handle_ != nullptr) {
    ghostty_sgr_free(static_cast<GhosttySgrParser>(handle_));
    handle_ = nullptr;
  }
}

} // namespace libghostty_cpp::sgr
