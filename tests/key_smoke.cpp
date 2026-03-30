#include "libghostty_cpp/key.hpp"
#include "libghostty_cpp/vt.hpp"

#include <array>
#include <cassert>
#include <optional>
#include <string_view>
#include <vector>

int main() {
  using libghostty_cpp::Error;
  using libghostty_cpp::ErrorCode;
  using libghostty_cpp::Terminal;
  using libghostty_cpp::TerminalOptions;
  using libghostty_cpp::key::Action;
  using libghostty_cpp::key::Encoder;
  using libghostty_cpp::key::Event;
  using libghostty_cpp::key::Key;
  using libghostty_cpp::key::Mods;

  Terminal terminal(TerminalOptions{80, 24, 0});
  Encoder encoder;
  Event event;

  event
    .set_action(Action::Press)
    .set_key(Key::A)
    .set_mods(Mods::Shift)
    .set_consumed_mods(Mods::Shift)
    .set_composing(false)
    .set_unshifted_codepoint(U'a')
    .set_utf8("A");

  assert(event.action() == Action::Press);
  assert(event.key() == Key::A);
  assert(event.mods() == Mods::Shift);
  assert(event.consumed_mods() == Mods::Shift);
  assert(!event.is_composing());
  assert(event.unshifted_codepoint() == U'a');
  assert(event.utf8().has_value());
  assert(*event.utf8() == "A");

  encoder.set_options_from_terminal(terminal);
  assert(encoder.required_size(event) == 1);

  std::array<std::uint8_t, 1> printable = {};
  assert(encoder.encode(event, printable.data(), printable.size()) == 1);
  assert(printable[0] == static_cast<std::uint8_t>('A'));

  std::vector<std::uint8_t> response = {0x1B};
  encoder.encode_to(response, event);
  assert(response.size() == 2);
  assert(response[0] == 0x1B);
  assert(response[1] == static_cast<std::uint8_t>('A'));

  Event arrow_up;
  arrow_up
    .set_action(Action::Press)
    .set_key(Key::ArrowUp)
    .set_mods(Mods::None)
    .set_consumed_mods(Mods::None)
    .set_composing(false)
    .set_unshifted_codepoint(U'\0')
    .clear_utf8();

  std::vector<std::uint8_t> normal_cursor;
  encoder.set_options_from_terminal(terminal).encode_to(normal_cursor, arrow_up);
  const std::vector<std::uint8_t> expected_normal = {
    0x1B,
    static_cast<std::uint8_t>('['),
    static_cast<std::uint8_t>('A'),
  };
  assert(normal_cursor == expected_normal);

  terminal.vt_write("\x1B[?1h");
  std::vector<std::uint8_t> application_cursor;
  encoder.set_options_from_terminal(terminal).encode_to(application_cursor, arrow_up);
  const std::vector<std::uint8_t> expected_application = {
    0x1B,
    static_cast<std::uint8_t>('O'),
    static_cast<std::uint8_t>('A'),
  };
  assert(application_cursor == expected_application);

  bool threw_out_of_space = false;
  std::array<std::uint8_t, 1> too_small = {};
  try {
    static_cast<void>(encoder.encode(arrow_up, too_small.data(), too_small.size()));
  } catch (const Error& error) {
    threw_out_of_space = error.code() == ErrorCode::OutOfSpace;
  }
  assert(threw_out_of_space);

  bool threw_invalid_codepoint = false;
  try {
    event.set_unshifted_codepoint(static_cast<char32_t>(0x110000u));
  } catch (const Error& error) {
    threw_invalid_codepoint = error.code() == ErrorCode::InvalidValue;
  }
  assert(threw_invalid_codepoint);

  return 0;
}
