#include "libghostty_cpp/mouse.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

int main() {
  using libghostty_cpp::mouse::Action;
  using libghostty_cpp::mouse::Button;
  using libghostty_cpp::mouse::Encoder;
  using libghostty_cpp::mouse::EncoderSize;
  using libghostty_cpp::mouse::Event;
  using libghostty_cpp::mouse::Format;
  using libghostty_cpp::mouse::Position;
  using libghostty_cpp::mouse::TrackingMode;

  Encoder encoder;
  encoder
    .set_tracking_mode(TrackingMode::Normal)
    .set_format(Format::Sgr)
    .set_size(EncoderSize{
      800,
      600,
      10,
      20,
      0,
      0,
      0,
      0,
    });

  Event event;
  event
    .set_action(Action::Press)
    .set_button(Button::Left)
    .set_position(Position{50.0F, 40.0F});

  const std::vector<std::uint8_t> encoded = encoder.encode(event);
  const std::string text(encoded.begin(), encoded.end());

  std::cout << text;
  return 0;
}
