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
using SizeCallback =
  std::function<std::optional<SizeReportSize>(const Terminal &)>;
using DeviceAttributesCallback =
  std::function<std::optional<DeviceAttributes>(const Terminal &)>;
using XtversionCallback =
  std::function<std::optional<std::string>(const Terminal &)>;
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
  void resize(std::uint16_t cols, std::uint16_t rows,
              std::uint32_t cell_width_px = 0,
              std::uint32_t cell_height_px = 0);
  void scroll_viewport_delta(std::ptrdiff_t delta) noexcept;

  // Callback references are only valid for the duration of the callback.
  Terminal &on_pty_write(PtyWriteCallback callback);
  Terminal &on_size(SizeCallback callback);
  Terminal &on_device_attributes(DeviceAttributesCallback callback);
  Terminal &on_xtversion(XtversionCallback callback);
  Terminal &on_color_scheme(ColorSchemeCallback callback);

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
