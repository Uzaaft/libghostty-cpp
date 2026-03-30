#include "libghostty_cpp/key.hpp"

#include "c_compat/key.h"
#include "libghostty_cpp/vt.hpp"
#include "result.hpp"

#include <utility>

namespace libghostty_cpp {
namespace key {

namespace {

bool is_valid_unicode_scalar(char32_t codepoint) noexcept {
  return codepoint <= 0x10FFFFu
         && !(codepoint >= 0xD800u && codepoint <= 0xDFFFu);
}

Action get_action(const libghostty_cpp_key_event* handle) {
  libghostty_cpp_key_action action = LIBGHOSTTY_CPP_KEY_ACTION_PRESS;
  detail::throw_if_error(libghostty_cpp_key_event_get_action(handle, &action));
  return static_cast<Action>(action);
}

Key get_key(const libghostty_cpp_key_event* handle) {
  libghostty_cpp_key_value key = 0;
  detail::throw_if_error(libghostty_cpp_key_event_get_key(handle, &key));
  return static_cast<Key>(key);
}

Mods get_mods(
  const libghostty_cpp_key_event* handle,
  libghostty_cpp_result (*getter)(
    const libghostty_cpp_key_event*,
    libghostty_cpp_key_mods*
  )
) {
  libghostty_cpp_key_mods mods = 0;
  detail::throw_if_error(getter(handle, &mods));
  return static_cast<Mods>(mods);
}

Encoder& set_bool_option(
  libghostty_cpp_key_encoder* handle,
  libghostty_cpp_key_encoder_option option,
  bool value,
  Encoder& encoder
) {
  detail::throw_if_error(
    libghostty_cpp_key_encoder_set_bool_option(handle, option, value)
  );
  return encoder;
}

} // namespace

Encoder::Encoder() {
  libghostty_cpp_key_encoder* handle = nullptr;
  detail::throw_if_error(libghostty_cpp_key_encoder_new(&handle));
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

Encoder& Encoder::set_cursor_key_application(bool value) {
  ensure_handle();
  return set_bool_option(
    handle_,
    LIBGHOSTTY_CPP_KEY_ENCODER_OPTION_CURSOR_KEY_APPLICATION,
    value,
    *this
  );
}

Encoder& Encoder::set_keypad_key_application(bool value) {
  ensure_handle();
  return set_bool_option(
    handle_,
    LIBGHOSTTY_CPP_KEY_ENCODER_OPTION_KEYPAD_KEY_APPLICATION,
    value,
    *this
  );
}

Encoder& Encoder::set_ignore_keypad_with_numlock(bool value) {
  ensure_handle();
  return set_bool_option(
    handle_,
    LIBGHOSTTY_CPP_KEY_ENCODER_OPTION_IGNORE_KEYPAD_WITH_NUMLOCK,
    value,
    *this
  );
}

Encoder& Encoder::set_alt_esc_prefix(bool value) {
  ensure_handle();
  return set_bool_option(
    handle_,
    LIBGHOSTTY_CPP_KEY_ENCODER_OPTION_ALT_ESC_PREFIX,
    value,
    *this
  );
}

Encoder& Encoder::set_modify_other_keys_state_2(bool value) {
  ensure_handle();
  return set_bool_option(
    handle_,
    LIBGHOSTTY_CPP_KEY_ENCODER_OPTION_MODIFY_OTHER_KEYS_STATE_2,
    value,
    *this
  );
}

Encoder& Encoder::set_kitty_flags(KittyKeyFlags value) {
  ensure_handle();
  detail::throw_if_error(libghostty_cpp_key_encoder_set_kitty_flags(
    handle_,
    static_cast<libghostty_cpp_kitty_key_flags>(value)
  ));
  return *this;
}

Encoder& Encoder::set_macos_option_as_alt(OptionAsAlt value) {
  ensure_handle();
  detail::throw_if_error(libghostty_cpp_key_encoder_set_option_as_alt(
    handle_,
    static_cast<libghostty_cpp_key_option_as_alt>(value)
  ));
  return *this;
}

Encoder& Encoder::set_options_from_terminal(const Terminal& terminal) {
  ensure_handle();
  detail::throw_if_error(
    libghostty_cpp_key_encoder_set_options_from_terminal(handle_, terminal.handle_)
  );
  return *this;
}

std::size_t Encoder::required_size(const Event& event) const {
  ensure_handle();
  event.ensure_handle();

  std::size_t required = 0;
  const libghostty_cpp_result result = libghostty_cpp_key_encoder_encode(
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
    libghostty_cpp_key_encoder_encode(handle_, event.handle_, output, output_size, &written)
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
  libghostty_cpp_key_encoder_free(handle_);
  handle_ = nullptr;
}

Event::Event() {
  libghostty_cpp_key_event* handle = nullptr;
  detail::throw_if_error(libghostty_cpp_key_event_new(&handle));
  handle_ = handle;
}

Event::~Event() {
  release();
}

Event::Event(Event&& other) noexcept
    : handle_(std::exchange(other.handle_, nullptr)),
      utf8_(std::move(other.utf8_)) {
  if (handle_ != nullptr) {
    sync_utf8();
  }
}

Event& Event::operator=(Event&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  release();
  handle_ = std::exchange(other.handle_, nullptr);
  utf8_ = std::move(other.utf8_);
  if (handle_ != nullptr) {
    sync_utf8();
  }
  return *this;
}

Event& Event::set_action(Action action) {
  ensure_handle();
  detail::throw_if_error(
    libghostty_cpp_key_event_set_action(
      handle_,
      static_cast<libghostty_cpp_key_action>(action)
    )
  );
  return *this;
}

Event& Event::set_key(Key key) {
  ensure_handle();
  detail::throw_if_error(
    libghostty_cpp_key_event_set_key(
      handle_,
      static_cast<libghostty_cpp_key_value>(key)
    )
  );
  return *this;
}

Event& Event::set_mods(Mods mods) {
  ensure_handle();
  detail::throw_if_error(
    libghostty_cpp_key_event_set_mods(
      handle_,
      static_cast<libghostty_cpp_key_mods>(mods)
    )
  );
  return *this;
}

Event& Event::set_consumed_mods(Mods mods) {
  ensure_handle();
  detail::throw_if_error(
    libghostty_cpp_key_event_set_consumed_mods(
      handle_,
      static_cast<libghostty_cpp_key_mods>(mods)
    )
  );
  return *this;
}

Event& Event::set_composing(bool composing) {
  ensure_handle();
  detail::throw_if_error(libghostty_cpp_key_event_set_composing(handle_, composing));
  return *this;
}

Event& Event::set_utf8(std::string_view text) {
  ensure_handle();
  utf8_ = std::string(text);
  sync_utf8();
  return *this;
}

Event& Event::clear_utf8() {
  ensure_handle();
  utf8_.reset();
  sync_utf8();
  return *this;
}

Event& Event::set_unshifted_codepoint(char32_t codepoint) {
  ensure_handle();
  if (!is_valid_unicode_scalar(codepoint)) {
    throw Error(ErrorCode::InvalidValue);
  }

  const std::uint32_t raw_codepoint = static_cast<std::uint32_t>(codepoint);
  detail::throw_if_error(
    libghostty_cpp_key_event_set_unshifted_codepoint(handle_, raw_codepoint)
  );
  return *this;
}

Action Event::action() const {
  ensure_handle();
  return get_action(handle_);
}

Key Event::key() const {
  ensure_handle();
  return get_key(handle_);
}

Mods Event::mods() const {
  ensure_handle();
  return get_mods(handle_, libghostty_cpp_key_event_get_mods);
}

Mods Event::consumed_mods() const {
  ensure_handle();
  return get_mods(handle_, libghostty_cpp_key_event_get_consumed_mods);
}

bool Event::is_composing() const {
  ensure_handle();
  bool composing = false;
  detail::throw_if_error(libghostty_cpp_key_event_get_composing(handle_, &composing));
  return composing;
}

std::optional<std::string_view> Event::utf8() const {
  ensure_handle();
  if (!utf8_.has_value()) {
    return std::nullopt;
  }

  return std::string_view(*utf8_);
}

char32_t Event::unshifted_codepoint() const {
  ensure_handle();

  std::uint32_t codepoint = 0;
  detail::throw_if_error(
    libghostty_cpp_key_event_get_unshifted_codepoint(handle_, &codepoint)
  );
  return static_cast<char32_t>(codepoint);
}

void Event::ensure_handle() const {
  if (handle_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }
}

void Event::sync_utf8() {
  if (utf8_.has_value()) {
    detail::throw_if_error(libghostty_cpp_key_event_set_utf8(
      handle_,
      reinterpret_cast<const std::uint8_t*>(utf8_->data()),
      utf8_->size()
    ));
    return;
  }

  detail::throw_if_error(libghostty_cpp_key_event_set_utf8(handle_, nullptr, 0));
}

void Event::release() noexcept {
  libghostty_cpp_key_event_free(handle_);
  handle_ = nullptr;
  utf8_.reset();
}

} // namespace key
} // namespace libghostty_cpp
