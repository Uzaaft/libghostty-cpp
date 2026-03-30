#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "libghostty_cpp/error.hpp"
#include "libghostty_cpp/key.hpp"

struct libghostty_cpp_mouse_encoder;
struct libghostty_cpp_mouse_event;

namespace libghostty_cpp {

class Terminal;

namespace mouse {

enum class TrackingMode : std::uint32_t {
  None = 0,
  X10 = 1,
  Normal = 2,
  Button = 3,
  Any = 4,
};

enum class Format : std::uint32_t {
  X10 = 0,
  Utf8 = 1,
  Sgr = 2,
  Urxvt = 3,
  SgrPixels = 4,
};

enum class Action : std::uint32_t {
  Press = 0,
  Release = 1,
  Motion = 2,
};

enum class Button : std::uint32_t {
  Unknown = 0,
  Left = 1,
  Right = 2,
  Middle = 3,
  Four = 4,
  Five = 5,
  Six = 6,
  Seven = 7,
  Eight = 8,
  Nine = 9,
  Ten = 10,
  Eleven = 11,
};

struct Position {
  float x = 0.0F;
  float y = 0.0F;
};

struct EncoderSize {
  std::uint32_t screen_width = 0;
  std::uint32_t screen_height = 0;
  std::uint32_t cell_width = 0;
  std::uint32_t cell_height = 0;
  std::uint32_t padding_top = 0;
  std::uint32_t padding_bottom = 0;
  std::uint32_t padding_right = 0;
  std::uint32_t padding_left = 0;
};

class Event;

class Encoder {
public:
  Encoder();
  ~Encoder();

  Encoder(Encoder&& other) noexcept;
  Encoder& operator=(Encoder&& other) noexcept;

  Encoder(const Encoder&) = delete;
  Encoder& operator=(const Encoder&) = delete;

  Encoder& set_tracking_mode(TrackingMode value);
  Encoder& set_format(Format value);
  Encoder& set_size(const EncoderSize& value);
  Encoder& set_any_button_pressed(bool value);
  Encoder& set_track_last_cell(bool value);
  Encoder& set_options_from_terminal(const Terminal& terminal);
  void reset();

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

  libghostty_cpp_mouse_encoder* handle_ = nullptr;
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
  Event& set_button(std::optional<Button> button);
  Event& set_mods(key::Mods mods);
  Event& set_position(Position position);

  [[nodiscard]] Action action() const;
  [[nodiscard]] std::optional<Button> button() const;
  [[nodiscard]] key::Mods mods() const;
  [[nodiscard]] Position position() const;

private:
  friend class Encoder;

  void ensure_handle() const;
  void release() noexcept;

  libghostty_cpp_mouse_event* handle_ = nullptr;
};

} // namespace mouse
} // namespace libghostty_cpp
