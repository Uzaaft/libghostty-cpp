#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "libghostty_cpp/error.hpp"

struct libghostty_cpp_terminal;

namespace libghostty_cpp {

namespace detail {
struct TerminalCallbacks;

constexpr std::uint16_t pack_mode(std::uint16_t value, bool is_ansi) noexcept {
  return static_cast<std::uint16_t>((value & 0x7FFFu) | (is_ansi ? 0x8000u : 0u));
}
}

namespace key {
class Encoder;
}

namespace mouse {
class Encoder;
}

class RenderState;
class Terminal;

struct TerminalOptions {
  std::uint16_t cols;
  std::uint16_t rows;
  std::size_t max_scrollback = 0;
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
  [[nodiscard]] bool is_mode_enabled(Mode mode) const;
  void resize(std::uint16_t cols, std::uint16_t rows,
              std::uint32_t cell_width_px = 0,
              std::uint32_t cell_height_px = 0);
  void scroll_viewport_delta(std::ptrdiff_t delta) noexcept;

  // Callback references are only valid for the duration of the callback.
  Terminal &on_pty_write(PtyWriteCallback callback);
  Terminal &on_bell(BellCallback callback);
  Terminal &on_size(SizeCallback callback);
  Terminal &on_device_attributes(DeviceAttributesCallback callback);
  Terminal &on_xtversion(XtversionCallback callback);
  Terminal &on_title_changed(TitleChangedCallback callback);
  Terminal &on_color_scheme(ColorSchemeCallback callback);

  // Returned view is invalidated by the next vt_write() or reset().
  [[nodiscard]] std::string_view title() const;
  [[nodiscard]] std::uint16_t cols() const;
  [[nodiscard]] std::uint16_t rows() const;
  [[nodiscard]] std::uint16_t cursor_x() const;
  [[nodiscard]] std::uint16_t cursor_y() const;

private:
  friend class RenderState;
  friend class key::Encoder;
  friend class mouse::Encoder;
  friend struct detail::TerminalCallbacks;

  void release() noexcept;

  libghostty_cpp_terminal *handle_ = nullptr;
  std::unique_ptr<detail::TerminalCallbacks> callbacks_;
};

} // namespace libghostty_cpp
