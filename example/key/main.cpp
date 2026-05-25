#include "libghostty_cpp/key.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

int main() {
  using libghostty_cpp::key::Action;
  using libghostty_cpp::key::Encoder;
  using libghostty_cpp::key::Event;
  using libghostty_cpp::key::Key;
  using libghostty_cpp::key::KittyKeyFlags;
  using libghostty_cpp::key::Mods;

  Encoder encoder;
  encoder.set_kitty_flags(KittyKeyFlags::All);

  Event event;
  event
    .set_action(Action::Release)
    .set_key(Key::ControlLeft)
    .set_mods(Mods::Ctrl);

  const std::vector<std::uint8_t> encoded = encoder.encode(event);
  const std::string text(encoded.begin(), encoded.end());

  std::cout << text;
  return 0;
}
