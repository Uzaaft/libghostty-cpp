#include "libghostty_cpp/focus.hpp"

#include <ghostty/vt.h>

#include "ghostty_result.hpp"

namespace libghostty_cpp::focus {

namespace {

GhosttyFocusEvent translate_event(Event event) {
  switch (event) {
    case Event::Gained:
      return GHOSTTY_FOCUS_GAINED;
    case Event::Lost:
      return GHOSTTY_FOCUS_LOST;
  }

  throw Error(ErrorCode::InvalidValue);
}

} // namespace

std::size_t required_size(Event event) {
  std::size_t written = 0;
  const GhosttyResult result = ghostty_focus_encode(
    translate_event(event),
    nullptr,
    0,
    &written
  );

  if (result == GHOSTTY_SUCCESS || result == GHOSTTY_OUT_OF_SPACE) {
    return written;
  }

  detail::throw_if_ghostty_error(result);
  return 0;
}

std::size_t encode(Event event, std::uint8_t* output, std::size_t output_size) {
  std::size_t written = 0;
  detail::throw_if_ghostty_error(
    ghostty_focus_encode(
      translate_event(event),
      reinterpret_cast<char*>(output),
      output_size,
      &written
    )
  );
  return written;
}

std::vector<std::uint8_t> encode(Event event) {
  std::vector<std::uint8_t> output;
  encode_to(output, event);
  return output;
}

void encode_to(std::vector<std::uint8_t>& output, Event event) {
  const std::size_t required = required_size(event);
  if (required == 0) {
    return;
  }

  const std::size_t original_size = output.size();
  output.resize(original_size + required);

  try {
    const std::size_t written = encode(event, output.data() + original_size, required);
    output.resize(original_size + written);
  } catch (...) {
    output.resize(original_size);
    throw;
  }
}

} // namespace libghostty_cpp::focus
