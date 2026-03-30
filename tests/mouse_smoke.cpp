#include "libghostty_cpp/mouse.hpp"
#include "libghostty_cpp/vt.hpp"

#include <array>
#include <cassert>
#include <string_view>
#include <vector>

int main() {
  using libghostty_cpp::Error;
  using libghostty_cpp::ErrorCode;
  using libghostty_cpp::Terminal;
  using libghostty_cpp::TerminalOptions;
  using libghostty_cpp::key::Mods;
  using libghostty_cpp::mouse::Action;
  using libghostty_cpp::mouse::Button;
  using libghostty_cpp::mouse::Encoder;
  using libghostty_cpp::mouse::EncoderSize;
  using libghostty_cpp::mouse::Event;
  using libghostty_cpp::mouse::Format;
  using libghostty_cpp::mouse::Position;
  using libghostty_cpp::mouse::TrackingMode;

  Terminal terminal(TerminalOptions{80, 24, 0});
  Encoder encoder;
  Event press_event;

  press_event
    .set_action(Action::Press)
    .set_button(Button::Left)
    .set_mods(Mods::Shift)
    .set_position(Position{5.0F, 5.0F});

  assert(press_event.action() == Action::Press);
  assert(press_event.button().has_value());
  assert(*press_event.button() == Button::Left);
  assert(press_event.mods() == Mods::Shift);
  assert(press_event.position().x == 5.0F);
  assert(press_event.position().y == 5.0F);

  encoder
    .set_tracking_mode(TrackingMode::Normal)
    .set_format(Format::Sgr)
    .set_size(EncoderSize{100, 100, 10, 10, 0, 0, 0, 0})
    .set_any_button_pressed(false)
    .set_track_last_cell(true);

  assert(encoder.required_size(press_event) == 9);

  std::array<std::uint8_t, 9> fixed = {};
  const std::size_t written = encoder.encode(press_event, fixed.data(), fixed.size());
  assert(written == 9);
  assert(std::string_view(reinterpret_cast<const char*>(fixed.data()), written) == "\x1B[<4;1;1M");

  std::vector<std::uint8_t> appended = {0x1B};
  encoder.encode_to(appended, press_event);
  assert(appended.size() == 10);
  assert(appended[0] == 0x1B);
  assert(std::string_view(
           reinterpret_cast<const char*>(appended.data() + 1),
           appended.size() - 1
         ) == "\x1B[<4;1;1M");

  terminal.vt_write("\x1B[?1000h\x1B[?1006h");
  std::vector<std::uint8_t> terminal_configured;
  encoder.set_options_from_terminal(terminal).encode_to(terminal_configured, press_event);
  assert(std::string_view(
           reinterpret_cast<const char*>(terminal_configured.data()),
           terminal_configured.size()
         ) == "\x1B[<4;1;1M");

  Event motion_event;
  motion_event
    .set_action(Action::Motion)
    .set_button(std::nullopt)
    .set_mods(Mods::None)
    .set_position(Position{15.0F, 15.0F});

  encoder
    .set_tracking_mode(TrackingMode::Any)
    .set_format(Format::Sgr)
    .set_size(EncoderSize{100, 100, 10, 10, 0, 0, 0, 0})
    .set_any_button_pressed(false)
    .set_track_last_cell(true)
    .reset();

  std::vector<std::uint8_t> first_motion;
  encoder.encode_to(first_motion, motion_event);
  assert(!first_motion.empty());

  std::vector<std::uint8_t> second_motion;
  encoder.encode_to(second_motion, motion_event);
  assert(second_motion.empty());

  encoder.reset();
  std::vector<std::uint8_t> reset_motion;
  encoder.encode_to(reset_motion, motion_event);
  assert(reset_motion == first_motion);

  bool threw_out_of_space = false;
  std::array<std::uint8_t, 2> too_small = {};
  try {
    static_cast<void>(encoder.encode(press_event, too_small.data(), too_small.size()));
  } catch (const Error& error) {
    threw_out_of_space = error.code() == ErrorCode::OutOfSpace;
  }
  assert(threw_out_of_space);

  return 0;
}
