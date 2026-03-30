#include "libghostty_cpp/mouse.hpp"

#include "c_compat/mouse.h"
#include "libghostty_cpp/vt.hpp"
#include "result.hpp"

#include <utility>

namespace libghostty_cpp {
namespace mouse {

namespace {

Action get_action(const libghostty_cpp_mouse_event* handle) {
  libghostty_cpp_mouse_action action = LIBGHOSTTY_CPP_MOUSE_ACTION_PRESS;
  detail::throw_if_error(libghostty_cpp_mouse_event_get_action(handle, &action));
  return static_cast<Action>(action);
}

Position get_position(const libghostty_cpp_mouse_event* handle) {
  libghostty_cpp_mouse_position position = {};
  detail::throw_if_error(libghostty_cpp_mouse_event_get_position(handle, &position));
  return Position {position.x, position.y};
}

} // namespace

Encoder::Encoder() {
  libghostty_cpp_mouse_encoder* handle = nullptr;
  detail::throw_if_error(libghostty_cpp_mouse_encoder_new(&handle));
  handle_ = handle;
}

Encoder::~Encoder() {
  release();
}

Encoder::Encoder(Encoder&& other) noexcept
    : handle_(std::exchange(other.handle_, nullptr)) {}

Encoder& Encoder::operator=(Encoder&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  release();
  handle_ = std::exchange(other.handle_, nullptr);
  return *this;
}

Encoder& Encoder::set_tracking_mode(TrackingMode value) {
  ensure_handle();
  detail::throw_if_error(libghostty_cpp_mouse_encoder_set_tracking_mode(
    handle_,
    static_cast<libghostty_cpp_mouse_tracking_mode>(value)
  ));
  return *this;
}

Encoder& Encoder::set_format(Format value) {
  ensure_handle();
  detail::throw_if_error(libghostty_cpp_mouse_encoder_set_format(
    handle_,
    static_cast<libghostty_cpp_mouse_format>(value)
  ));
  return *this;
}

Encoder& Encoder::set_size(const EncoderSize& value) {
  ensure_handle();
  detail::throw_if_error(libghostty_cpp_mouse_encoder_set_size(
    handle_,
    libghostty_cpp_mouse_encoder_size {
      value.screen_width,
      value.screen_height,
      value.cell_width,
      value.cell_height,
      value.padding_top,
      value.padding_bottom,
      value.padding_right,
      value.padding_left,
    }
  ));
  return *this;
}

Encoder& Encoder::set_any_button_pressed(bool value) {
  ensure_handle();
  detail::throw_if_error(
    libghostty_cpp_mouse_encoder_set_any_button_pressed(handle_, value)
  );
  return *this;
}

Encoder& Encoder::set_track_last_cell(bool value) {
  ensure_handle();
  detail::throw_if_error(
    libghostty_cpp_mouse_encoder_set_track_last_cell(handle_, value)
  );
  return *this;
}

Encoder& Encoder::set_options_from_terminal(const Terminal& terminal) {
  ensure_handle();
  detail::throw_if_error(
    libghostty_cpp_mouse_encoder_set_options_from_terminal(handle_, terminal.handle_)
  );
  return *this;
}

void Encoder::reset() {
  ensure_handle();
  detail::throw_if_error(libghostty_cpp_mouse_encoder_reset(handle_));
}

std::size_t Encoder::required_size(const Event& event) const {
  ensure_handle();
  event.ensure_handle();

  std::size_t required = 0;
  const libghostty_cpp_result result = libghostty_cpp_mouse_encoder_encode(
    handle_,
    event.handle_,
    nullptr,
    0,
    &required
  );

  if (result == LIBGHOSTTY_CPP_RESULT_SUCCESS
      || result == LIBGHOSTTY_CPP_RESULT_OUT_OF_SPACE) {
    return required;
  }

  detail::throw_if_error(result);
  return 0;
}

std::size_t Encoder::encode(
  const Event& event,
  std::uint8_t* output,
  std::size_t output_size
) const {
  ensure_handle();
  event.ensure_handle();

  std::size_t written = 0;
  detail::throw_if_error(
    libghostty_cpp_mouse_encoder_encode(handle_, event.handle_, output, output_size, &written)
  );
  return written;
}

void Encoder::encode_to(
  std::vector<std::uint8_t>& output,
  const Event& event
) const {
  const std::size_t required = required_size(event);
  if (required == 0) {
    return;
  }

  const std::size_t original_size = output.size();
  output.resize(original_size + required);

  try {
    const std::size_t written = encode(
      event,
      output.data() + original_size,
      required
    );
    output.resize(original_size + written);
  } catch (...) {
    output.resize(original_size);
    throw;
  }
}

void Encoder::ensure_handle() const {
  if (handle_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }
}

void Encoder::release() noexcept {
  libghostty_cpp_mouse_encoder_free(handle_);
  handle_ = nullptr;
}

Event::Event() {
  libghostty_cpp_mouse_event* handle = nullptr;
  detail::throw_if_error(libghostty_cpp_mouse_event_new(&handle));
  handle_ = handle;
}

Event::~Event() {
  release();
}

Event::Event(Event&& other) noexcept
    : handle_(std::exchange(other.handle_, nullptr)) {}

Event& Event::operator=(Event&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  release();
  handle_ = std::exchange(other.handle_, nullptr);
  return *this;
}

Event& Event::set_action(Action action) {
  ensure_handle();
  detail::throw_if_error(libghostty_cpp_mouse_event_set_action(
    handle_,
    static_cast<libghostty_cpp_mouse_action>(action)
  ));
  return *this;
}

Event& Event::set_button(std::optional<Button> button) {
  ensure_handle();

  if (!button.has_value()) {
    detail::throw_if_error(libghostty_cpp_mouse_event_clear_button(handle_));
    return *this;
  }

  detail::throw_if_error(libghostty_cpp_mouse_event_set_button(
    handle_,
    static_cast<libghostty_cpp_mouse_button>(*button)
  ));
  return *this;
}

Event& Event::set_mods(key::Mods mods) {
  ensure_handle();
  detail::throw_if_error(libghostty_cpp_mouse_event_set_mods(
    handle_,
    static_cast<libghostty_cpp_key_mods>(mods)
  ));
  return *this;
}

Event& Event::set_position(Position position) {
  ensure_handle();
  detail::throw_if_error(libghostty_cpp_mouse_event_set_position(
    handle_,
    libghostty_cpp_mouse_position {position.x, position.y}
  ));
  return *this;
}

Action Event::action() const {
  ensure_handle();
  return get_action(handle_);
}

std::optional<Button> Event::button() const {
  ensure_handle();

  bool has_button = false;
  libghostty_cpp_mouse_button button = LIBGHOSTTY_CPP_MOUSE_BUTTON_UNKNOWN;
  detail::throw_if_error(
    libghostty_cpp_mouse_event_get_button(handle_, &has_button, &button)
  );
  if (!has_button) {
    return std::nullopt;
  }

  return static_cast<Button>(button);
}

key::Mods Event::mods() const {
  ensure_handle();

  libghostty_cpp_key_mods mods = 0;
  detail::throw_if_error(libghostty_cpp_mouse_event_get_mods(handle_, &mods));
  return static_cast<key::Mods>(mods);
}

Position Event::position() const {
  ensure_handle();
  return get_position(handle_);
}

void Event::ensure_handle() const {
  if (handle_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }
}

void Event::release() noexcept {
  libghostty_cpp_mouse_event_free(handle_);
  handle_ = nullptr;
}

} // namespace mouse
} // namespace libghostty_cpp
