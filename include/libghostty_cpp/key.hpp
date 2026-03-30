#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "libghostty_cpp/error.hpp"

struct libghostty_cpp_key_encoder;
struct libghostty_cpp_key_event;

namespace libghostty_cpp {

class Terminal;

namespace key {

enum class Action : std::uint32_t {
  Release = 0,
  Press = 1,
  Repeat = 2,
};

enum class OptionAsAlt : std::uint32_t {
  False = 0,
  True = 1,
  Left = 2,
  Right = 3,
};

enum class Mods : std::uint16_t {
  None = 0,
  Shift = 1,
  Ctrl = 2,
  Alt = 4,
  Super = 8,
  CapsLock = 16,
  NumLock = 32,
  ShiftSide = 64,
  CtrlSide = 128,
  AltSide = 256,
  SuperSide = 512,
};

enum class KittyKeyFlags : std::uint8_t {
  Disabled = 0,
  Disambiguate = 1,
  ReportEvents = 2,
  ReportAlternates = 4,
  ReportAll = 8,
  ReportAssociated = 16,
  All = 31,
};

enum class Key : std::uint32_t {
  Unidentified = 0,
  Backquote = 1,
  Backslash = 2,
  BracketLeft = 3,
  BracketRight = 4,
  Comma = 5,
  Digit0 = 6,
  Digit1 = 7,
  Digit2 = 8,
  Digit3 = 9,
  Digit4 = 10,
  Digit5 = 11,
  Digit6 = 12,
  Digit7 = 13,
  Digit8 = 14,
  Digit9 = 15,
  Equal = 16,
  IntlBackslash = 17,
  IntlRo = 18,
  IntlYen = 19,
  A = 20,
  B = 21,
  C = 22,
  D = 23,
  E = 24,
  F = 25,
  G = 26,
  H = 27,
  I = 28,
  J = 29,
  K = 30,
  L = 31,
  M = 32,
  N = 33,
  O = 34,
  P = 35,
  Q = 36,
  R = 37,
  S = 38,
  T = 39,
  U = 40,
  V = 41,
  W = 42,
  X = 43,
  Y = 44,
  Z = 45,
  Minus = 46,
  Period = 47,
  Quote = 48,
  Semicolon = 49,
  Slash = 50,
  AltLeft = 51,
  AltRight = 52,
  Backspace = 53,
  CapsLock = 54,
  ContextMenu = 55,
  ControlLeft = 56,
  ControlRight = 57,
  Enter = 58,
  MetaLeft = 59,
  MetaRight = 60,
  ShiftLeft = 61,
  ShiftRight = 62,
  Space = 63,
  Tab = 64,
  Convert = 65,
  KanaMode = 66,
  NonConvert = 67,
  Delete = 68,
  End = 69,
  Help = 70,
  Home = 71,
  Insert = 72,
  PageDown = 73,
  PageUp = 74,
  ArrowDown = 75,
  ArrowLeft = 76,
  ArrowRight = 77,
  ArrowUp = 78,
  NumLock = 79,
  Numpad0 = 80,
  Numpad1 = 81,
  Numpad2 = 82,
  Numpad3 = 83,
  Numpad4 = 84,
  Numpad5 = 85,
  Numpad6 = 86,
  Numpad7 = 87,
  Numpad8 = 88,
  Numpad9 = 89,
  NumpadAdd = 90,
  NumpadBackspace = 91,
  NumpadClear = 92,
  NumpadClearEntry = 93,
  NumpadComma = 94,
  NumpadDecimal = 95,
  NumpadDivide = 96,
  NumpadEnter = 97,
  NumpadEqual = 98,
  NumpadMemoryAdd = 99,
  NumpadMemoryClear = 100,
  NumpadMemoryRecall = 101,
  NumpadMemoryStore = 102,
  NumpadMemorySubtract = 103,
  NumpadMultiply = 104,
  NumpadParenLeft = 105,
  NumpadParenRight = 106,
  NumpadSubtract = 107,
  NumpadSeparator = 108,
  NumpadUp = 109,
  NumpadDown = 110,
  NumpadRight = 111,
  NumpadLeft = 112,
  NumpadBegin = 113,
  NumpadHome = 114,
  NumpadEnd = 115,
  NumpadInsert = 116,
  NumpadDelete = 117,
  NumpadPageUp = 118,
  NumpadPageDown = 119,
  Escape = 120,
  F1 = 121,
  F2 = 122,
  F3 = 123,
  F4 = 124,
  F5 = 125,
  F6 = 126,
  F7 = 127,
  F8 = 128,
  F9 = 129,
  F10 = 130,
  F11 = 131,
  F12 = 132,
  F13 = 133,
  F14 = 134,
  F15 = 135,
  F16 = 136,
  F17 = 137,
  F18 = 138,
  F19 = 139,
  F20 = 140,
  F21 = 141,
  F22 = 142,
  F23 = 143,
  F24 = 144,
  F25 = 145,
  Fn = 146,
  FnLock = 147,
  PrintScreen = 148,
  ScrollLock = 149,
  Pause = 150,
  BrowserBack = 151,
  BrowserFavorites = 152,
  BrowserForward = 153,
  BrowserHome = 154,
  BrowserRefresh = 155,
  BrowserSearch = 156,
  BrowserStop = 157,
  Eject = 158,
  LaunchApp1 = 159,
  LaunchApp2 = 160,
  LaunchMail = 161,
  MediaPlayPause = 162,
  MediaSelect = 163,
  MediaStop = 164,
  MediaTrackNext = 165,
  MediaTrackPrevious = 166,
  Power = 167,
  Sleep = 168,
  AudioVolumeDown = 169,
  AudioVolumeMute = 170,
  AudioVolumeUp = 171,
  WakeUp = 172,
  Copy = 173,
  Cut = 174,
  Paste = 175,
};

constexpr Mods operator|(Mods lhs, Mods rhs) noexcept {
  return static_cast<Mods>(
    static_cast<std::uint16_t>(lhs) | static_cast<std::uint16_t>(rhs)
  );
}

constexpr Mods operator&(Mods lhs, Mods rhs) noexcept {
  return static_cast<Mods>(
    static_cast<std::uint16_t>(lhs) & static_cast<std::uint16_t>(rhs)
  );
}

constexpr Mods operator~(Mods value) noexcept {
  return static_cast<Mods>(~static_cast<std::uint16_t>(value));
}

inline Mods& operator|=(Mods& lhs, Mods rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

inline Mods& operator&=(Mods& lhs, Mods rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr KittyKeyFlags operator|(
  KittyKeyFlags lhs,
  KittyKeyFlags rhs
) noexcept {
  return static_cast<KittyKeyFlags>(
    static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs)
  );
}

constexpr KittyKeyFlags operator&(
  KittyKeyFlags lhs,
  KittyKeyFlags rhs
) noexcept {
  return static_cast<KittyKeyFlags>(
    static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs)
  );
}

constexpr KittyKeyFlags operator~(KittyKeyFlags value) noexcept {
  return static_cast<KittyKeyFlags>(~static_cast<std::uint8_t>(value));
}

inline KittyKeyFlags& operator|=(
  KittyKeyFlags& lhs,
  KittyKeyFlags rhs
) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

inline KittyKeyFlags& operator&=(
  KittyKeyFlags& lhs,
  KittyKeyFlags rhs
) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

class Event;

class Encoder {
public:
  Encoder();
  ~Encoder();

  Encoder(Encoder&& other) noexcept;
  Encoder& operator=(Encoder&& other) noexcept;

  Encoder(const Encoder&) = delete;
  Encoder& operator=(const Encoder&) = delete;

  Encoder& set_cursor_key_application(bool value);
  Encoder& set_keypad_key_application(bool value);
  Encoder& set_ignore_keypad_with_numlock(bool value);
  Encoder& set_alt_esc_prefix(bool value);
  Encoder& set_modify_other_keys_state_2(bool value);
  Encoder& set_kitty_flags(KittyKeyFlags value);
  Encoder& set_macos_option_as_alt(OptionAsAlt value);
  Encoder& set_options_from_terminal(const Terminal& terminal);

  [[nodiscard]] std::size_t required_size(const Event& event) const;
  [[nodiscard]] std::size_t encode(
    const Event& event,
    std::uint8_t* output,
    std::size_t output_size
  ) const;
  void encode_to(std::vector<std::uint8_t>& output, const Event& event) const;

private:
  void ensure_handle() const;
  void release() noexcept;

  libghostty_cpp_key_encoder* handle_ = nullptr;
};

class Event {
public:
  Event();
  ~Event();

  Event(Event&& other) noexcept;
  Event& operator=(Event&& other) noexcept;

  Event(const Event&) = delete;
  Event& operator=(const Event&) = delete;

  Event& set_action(Action action);
  Event& set_key(Key key);
  Event& set_mods(Mods mods);
  Event& set_consumed_mods(Mods mods);
  Event& set_composing(bool composing);
  Event& set_utf8(std::string_view text);
  Event& clear_utf8();
  Event& set_unshifted_codepoint(char32_t codepoint);

  [[nodiscard]] Action action() const;
  [[nodiscard]] Key key() const;
  [[nodiscard]] Mods mods() const;
  [[nodiscard]] Mods consumed_mods() const;
  [[nodiscard]] bool is_composing() const;
  [[nodiscard]] std::optional<std::string_view> utf8() const;
  [[nodiscard]] char32_t unshifted_codepoint() const;

private:
  friend class Encoder;

  void ensure_handle() const;
  void sync_utf8();
  void release() noexcept;

  libghostty_cpp_key_event* handle_ = nullptr;
  std::optional<std::string> utf8_;
};

} // namespace key
} // namespace libghostty_cpp
