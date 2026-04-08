#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "libghostty_cpp/error.hpp"
#include "libghostty_cpp/key.hpp"
#include "libghostty_cpp/screen.hpp"
#include "libghostty_cpp/style.hpp"

struct libghostty_cpp_terminal;

namespace libghostty_cpp {

namespace detail {
struct TerminalCallbacks;

constexpr std::uint16_t pack_mode(std::uint16_t value, bool is_ansi) noexcept {
  return static_cast<std::uint16_t>((value & 0x7FFFu) | (is_ansi ? 0x8000u : 0u));
}
}

namespace mouse {
class Encoder;
}

namespace fmt {
class Formatter;
}

namespace kitty_graphics {
class Graphics;
class Placement;
class PlacementIterator;
}

class RenderState;
class Terminal;

struct TerminalOptions {
  std::uint16_t cols;
  std::uint16_t rows;
  std::size_t max_scrollback = 0;
};

enum class ActiveScreen : std::uint8_t {
  Primary = 0,
  Alternate = 1,
};

struct Scrollbar {
  std::uint64_t total = 0;
  std::uint64_t offset = 0;
  std::uint64_t len = 0;
};

struct PointCoordinate {
  std::uint16_t x;
  std::uint32_t y;
};

class Point {
public:
  enum class Kind : std::uint8_t {
    Active = 0,
    Viewport = 1,
    Screen = 2,
    History = 3,
  };

  [[nodiscard]] static Point active(PointCoordinate coordinate) noexcept {
    return Point(Kind::Active, coordinate);
  }

  [[nodiscard]] static Point viewport(PointCoordinate coordinate) noexcept {
    return Point(Kind::Viewport, coordinate);
  }

  [[nodiscard]] static Point screen(PointCoordinate coordinate) noexcept {
    return Point(Kind::Screen, coordinate);
  }

  [[nodiscard]] static Point history(PointCoordinate coordinate) noexcept {
    return Point(Kind::History, coordinate);
  }

  [[nodiscard]] Kind kind() const noexcept {
    return kind_;
  }

  [[nodiscard]] PointCoordinate coordinate() const noexcept {
    return coordinate_;
  }

private:
  constexpr Point(Kind kind, PointCoordinate coordinate) noexcept
      : kind_(kind), coordinate_(coordinate) {}

  Kind kind_;
  PointCoordinate coordinate_;
};

class ScrollViewport {
public:
  enum class Kind : std::uint8_t {
    Top = 0,
    Bottom = 1,
    Delta = 2,
  };

  [[nodiscard]] static ScrollViewport top() noexcept {
    return ScrollViewport(Kind::Top, 0);
  }

  [[nodiscard]] static ScrollViewport bottom() noexcept {
    return ScrollViewport(Kind::Bottom, 0);
  }

  [[nodiscard]] static ScrollViewport delta(std::ptrdiff_t amount) noexcept {
    return ScrollViewport(Kind::Delta, amount);
  }

  [[nodiscard]] Kind kind() const noexcept {
    return kind_;
  }

  [[nodiscard]] std::optional<std::ptrdiff_t> delta() const noexcept {
    if (kind_ != Kind::Delta) {
      return std::nullopt;
    }

    return delta_;
  }

private:
  constexpr ScrollViewport(Kind kind, std::ptrdiff_t delta) noexcept
      : kind_(kind), delta_(delta) {}

  Kind kind_;
  std::ptrdiff_t delta_;
};

using GridCellWide = screen::CellWide;

class GridRef {
public:
  [[nodiscard]] bool row_is_wrapped() const noexcept {
    return row_is_wrapped_;
  }

  [[nodiscard]] bool cell_has_text() const noexcept {
    return cell_has_text_;
  }

  [[nodiscard]] GridCellWide cell_wide() const noexcept {
    return cell_wide_;
  }

  [[nodiscard]] const std::u32string &graphemes() const noexcept {
    return graphemes_;
  }

  [[nodiscard]] std::size_t graphemes_len() const noexcept {
    return graphemes_.size();
  }

  [[nodiscard]] screen::Row row() const noexcept {
    return row_;
  }

  [[nodiscard]] screen::Cell cell() const noexcept {
    return cell_;
  }

  [[nodiscard]] const Style &style() const noexcept {
    return style_;
  }

  [[nodiscard]] const std::string &hyperlink_uri() const noexcept {
    return hyperlink_uri_;
  }

private:
  GridRef(
    screen::Row row,
    screen::Cell cell,
    Style style,
    bool row_is_wrapped,
    bool cell_has_text,
    GridCellWide cell_wide,
    std::u32string graphemes,
    std::string hyperlink_uri
  )
      : row_(row),
        cell_(cell),
        style_(std::move(style)),
        row_is_wrapped_(row_is_wrapped),
        cell_has_text_(cell_has_text),
        cell_wide_(cell_wide),
        graphemes_(std::move(graphemes)),
        hyperlink_uri_(std::move(hyperlink_uri)) {}

  screen::Row row_;
  screen::Cell cell_;
  Style style_;
  bool row_is_wrapped_ = false;
  bool cell_has_text_ = false;
  GridCellWide cell_wide_ = GridCellWide::Narrow;
  std::u32string graphemes_;
  std::string hyperlink_uri_;

  friend class Terminal;
};

struct SizeReportSize {
  std::uint16_t rows;
  std::uint16_t columns;
  std::uint32_t cell_width;
  std::uint32_t cell_height;
};

enum class ColorScheme : std::uint8_t {
  Light = 0,
  Dark = 1,
};

enum class Mode : std::uint16_t {
  Kam = detail::pack_mode(2, true),
  Insert = detail::pack_mode(4, true),
  SendReceive = detail::pack_mode(12, true),
  Linefeed = detail::pack_mode(20, true),
  CursorKeys = detail::pack_mode(1, false),
  Columns132 = detail::pack_mode(3, false),
  SlowScroll = detail::pack_mode(4, false),
  ReverseColors = detail::pack_mode(5, false),
  Origin = detail::pack_mode(6, false),
  Wraparound = detail::pack_mode(7, false),
  Autorepeat = detail::pack_mode(8, false),
  X10Mouse = detail::pack_mode(9, false),
  CursorBlinking = detail::pack_mode(12, false),
  CursorVisible = detail::pack_mode(25, false),
  Allow132Column = detail::pack_mode(40, false),
  ReverseWrap = detail::pack_mode(45, false),
  AltScreenLegacy = detail::pack_mode(47, false),
  KeypadKeys = detail::pack_mode(66, false),
  LeftRightMargin = detail::pack_mode(69, false),
  NormalMouse = detail::pack_mode(1000, false),
  ButtonMouse = detail::pack_mode(1002, false),
  AnyMouse = detail::pack_mode(1003, false),
  FocusEvent = detail::pack_mode(1004, false),
  Utf8Mouse = detail::pack_mode(1005, false),
  SgrMouse = detail::pack_mode(1006, false),
  AltScroll = detail::pack_mode(1007, false),
  UrxvtMouse = detail::pack_mode(1015, false),
  SgrPixelsMouse = detail::pack_mode(1016, false),
  NumlockKeypad = detail::pack_mode(1035, false),
  AltEscPrefix = detail::pack_mode(1036, false),
  AltSendsEsc = detail::pack_mode(1039, false),
  ReverseWrapExtended = detail::pack_mode(1045, false),
  AltScreen = detail::pack_mode(1047, false),
  SaveCursor = detail::pack_mode(1048, false),
  AltScreenSave = detail::pack_mode(1049, false),
  BracketedPaste = detail::pack_mode(2004, false),
  SynchronizedOutput = detail::pack_mode(2026, false),
  GraphemeCluster = detail::pack_mode(2027, false),
  ColorSchemeReport = detail::pack_mode(2031, false),
  InBandResize = detail::pack_mode(2048, false),
};

enum class ConformanceLevel : std::uint16_t {
  VT100 = 1,
  VT101 = 1,
  VT102 = 6,
  VT125 = 12,
  VT131 = 7,
  VT132 = 4,
  VT220 = 62,
  VT240 = 62,
  VT320 = 63,
  VT340 = 63,
  VT420 = 64,
  VT510 = 65,
  VT520 = 65,
  VT525 = 65,
  Level2 = 62,
  Level3 = 63,
  Level4 = 64,
  Level5 = 65,
};

enum class DeviceAttributeFeature : std::uint16_t {
  Columns132 = 1,
  Printer = 2,
  Regis = 3,
  Sixel = 4,
  SelectiveErase = 6,
  UserDefinedKeys = 8,
  NationalReplacement = 9,
  TechnicalCharacters = 15,
  Locator = 16,
  TerminalState = 17,
  Windowing = 18,
  HorizontalScrolling = 21,
  AnsiColor = 22,
  RectangularEditing = 28,
  AnsiTextLocator = 29,
  Clipboard = 52,
};

enum class DeviceType : std::uint16_t {
  VT100 = 0,
  VT220 = 1,
  VT240 = 2,
  VT330 = 18,
  VT340 = 19,
  VT320 = 24,
  VT382 = 32,
  VT420 = 41,
  VT510 = 61,
  VT520 = 64,
  VT525 = 65,
};

struct PrimaryDeviceAttributes {
  ConformanceLevel conformance_level = ConformanceLevel::VT220;
  std::vector<DeviceAttributeFeature> features;
};

struct SecondaryDeviceAttributes {
  DeviceType device_type = DeviceType::VT220;
  std::uint16_t firmware_version = 0;
  std::uint16_t rom_cartridge = 0;
};

struct TertiaryDeviceAttributes {
  std::uint32_t unit_id = 0;
};

struct DeviceAttributes {
  PrimaryDeviceAttributes primary;
  SecondaryDeviceAttributes secondary;
  TertiaryDeviceAttributes tertiary;
};

using PtyWriteCallback =
  std::function<void(const Terminal &, std::string_view data)>;
using BellCallback =
  std::function<void(const Terminal &)>;
using EnquiryCallback =
  std::function<std::optional<std::string>(const Terminal &)>;
using SizeCallback =
  std::function<std::optional<SizeReportSize>(const Terminal &)>;
using DeviceAttributesCallback =
  std::function<std::optional<DeviceAttributes>(const Terminal &)>;
using XtversionCallback =
  std::function<std::optional<std::string>(const Terminal &)>;
using TitleChangedCallback =
  std::function<void(const Terminal &)>;
using ColorSchemeCallback =
  std::function<std::optional<ColorScheme>(const Terminal &)>;

class Terminal {
public:
  explicit Terminal(TerminalOptions options);
  ~Terminal();

  Terminal(Terminal &&other) noexcept;
  Terminal &operator=(Terminal &&other) noexcept;

  Terminal(const Terminal &) = delete;
  Terminal &operator=(const Terminal &) = delete;

  void vt_write(std::string_view data);
  void reset() noexcept;
  [[nodiscard]] bool mode(Mode mode) const;
  [[nodiscard]] bool is_mode_enabled(Mode mode) const;
  Terminal &set_mode(Mode mode, bool value);
  [[nodiscard]] GridRef grid_ref(Point point) const;
  void resize(std::uint16_t cols, std::uint16_t rows,
              std::uint32_t cell_width_px = 0,
              std::uint32_t cell_height_px = 0);
  void scroll_viewport(ScrollViewport scroll) noexcept;
  void scroll_viewport_delta(std::ptrdiff_t delta) noexcept;

  // Callback references are only valid for the duration of the callback.
  Terminal &on_pty_write(PtyWriteCallback callback);
  Terminal &on_bell(BellCallback callback);
  Terminal &on_enquiry(EnquiryCallback callback);
  Terminal &on_size(SizeCallback callback);
  Terminal &on_device_attributes(DeviceAttributesCallback callback);
  Terminal &on_xtversion(XtversionCallback callback);
  Terminal &on_title_changed(TitleChangedCallback callback);
  Terminal &on_color_scheme(ColorSchemeCallback callback);

  // Returned views are invalidated by the next vt_write() or reset().
  Terminal &set_title(std::string_view value);
  Terminal &clear_title();
  Terminal &set_pwd(std::string_view value);
  Terminal &clear_pwd();
  [[nodiscard]] std::string_view title() const;
  [[nodiscard]] std::string_view pwd() const;
  [[nodiscard]] std::uint16_t cols() const;
  [[nodiscard]] std::uint16_t rows() const;
  [[nodiscard]] std::uint16_t cursor_x() const;
  [[nodiscard]] std::uint16_t cursor_y() const;
  [[nodiscard]] bool is_cursor_pending_wrap() const;
  [[nodiscard]] bool is_cursor_visible() const;
  [[nodiscard]] key::KittyKeyFlags kitty_keyboard_flags() const;
  [[nodiscard]] Style cursor_style() const;
  [[nodiscard]] ActiveScreen active_screen() const;
  [[nodiscard]] Scrollbar scrollbar() const;
  [[nodiscard]] bool is_mouse_tracking() const;
  [[nodiscard]] std::size_t total_rows() const;
  [[nodiscard]] std::size_t scrollback_rows() const;
  [[nodiscard]] std::uint32_t width_px() const;
  [[nodiscard]] std::uint32_t height_px() const;
  [[nodiscard]] std::optional<RgbColor> fg_color() const;
  [[nodiscard]] std::optional<RgbColor> default_fg_color() const;
  Terminal &set_default_fg_color(std::optional<RgbColor> value);
  [[nodiscard]] std::optional<RgbColor> bg_color() const;
  [[nodiscard]] std::optional<RgbColor> default_bg_color() const;
  Terminal &set_default_bg_color(std::optional<RgbColor> value);
  [[nodiscard]] std::optional<RgbColor> cursor_color() const;
  [[nodiscard]] std::optional<RgbColor> default_cursor_color() const;
  Terminal &set_default_cursor_color(std::optional<RgbColor> value);
  [[nodiscard]] std::array<RgbColor, 256> color_palette() const;
  [[nodiscard]] std::array<RgbColor, 256> default_color_palette() const;
  Terminal &set_default_color_palette(const std::array<RgbColor, 256> &value);
  Terminal &clear_default_color_palette();

  [[nodiscard]] std::uint64_t kitty_image_storage_limit() const;
  Terminal &set_kitty_image_storage_limit(std::uint64_t limit);
  [[nodiscard]] bool is_kitty_image_from_file_allowed() const;
  Terminal &set_kitty_image_from_file_allowed(bool allowed);
  [[nodiscard]] bool is_kitty_image_from_temp_file_allowed() const;
  Terminal &set_kitty_image_from_temp_file_allowed(bool allowed);
  [[nodiscard]] bool is_kitty_image_from_shared_mem_allowed() const;
  Terminal &set_kitty_image_from_shared_mem_allowed(bool allowed);

  [[nodiscard]] kitty_graphics::Graphics kitty_graphics() const;

private:
  friend class RenderState;
  friend class key::Encoder;
  friend class mouse::Encoder;
  friend class fmt::Formatter;
  friend class kitty_graphics::Graphics;
  friend class kitty_graphics::Placement;
  friend class kitty_graphics::PlacementIterator;
  friend struct detail::TerminalCallbacks;

  void release() noexcept;

  libghostty_cpp_terminal *handle_ = nullptr;
  std::unique_ptr<detail::TerminalCallbacks> callbacks_;
};

} // namespace libghostty_cpp
