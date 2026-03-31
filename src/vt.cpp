#include "libghostty_cpp/vt.hpp"

#include "c_compat/vt.h"
#include "result.hpp"

#include <exception>
#include <string>
#include <utility>

namespace libghostty_cpp {

namespace detail {

struct TerminalCallbacks {
  Terminal* owner = nullptr;
  PtyWriteCallback pty_write;
  BellCallback bell;
  SizeCallback size;
  DeviceAttributesCallback device_attributes;
  XtversionCallback xtversion;
  TitleChangedCallback title_changed;
  ColorSchemeCallback color_scheme;
  std::string xtversion_storage;
  std::exception_ptr pending_exception;

  [[nodiscard]] bool has_owner(
    const libghostty_cpp_terminal* handle
  ) const noexcept {
    return owner != nullptr && owner->handle_ == handle;
  }

  [[nodiscard]] const Terminal& terminal(
    const libghostty_cpp_terminal* handle
  ) const {
    if (!has_owner(handle)) {
      throw Error(ErrorCode::InvalidState);
    }

    return *owner;
  }

  void capture_current_exception() noexcept {
    if (pending_exception == nullptr) {
      pending_exception = std::current_exception();
    }
  }

  void clear_pending_exception() noexcept {
    pending_exception = nullptr;
  }

  void rethrow_pending_exception() {
    if (pending_exception == nullptr) {
      return;
    }

    const std::exception_ptr exception = pending_exception;
    pending_exception = nullptr;
    std::rethrow_exception(exception);
  }
};

} // namespace detail

namespace {

using TerminalU16Getter =
  libghostty_cpp_result (*)(const libghostty_cpp_terminal*, uint16_t*);
using TerminalStringGetter =
  libghostty_cpp_result (*)(const libghostty_cpp_terminal*, libghostty_cpp_string*);

std::uint16_t get_u16(const libghostty_cpp_terminal* handle, TerminalU16Getter getter) {
  std::uint16_t value = 0;
  detail::throw_if_error(getter(handle, &value));
  return value;
}

std::string_view get_string(
  const libghostty_cpp_terminal* handle,
  TerminalStringGetter getter
) {
  libghostty_cpp_string value = {nullptr, 0};
  detail::throw_if_error(getter(handle, &value));
  if (value.data == nullptr) {
    if (value.len != 0) {
      throw Error(ErrorCode::InvalidValue);
    }

    return {};
  }

  return std::string_view(reinterpret_cast<const char*>(value.data), value.len);
}

bool query_mode_enabled(const libghostty_cpp_terminal* handle, Mode mode) {
  bool value = false;
  detail::throw_if_error(
    libghostty_cpp_terminal_mode_get(handle, static_cast<std::uint16_t>(mode), &value)
  );
  return value;
}

libghostty_cpp_size_report_size translate_size_report_size(
  const SizeReportSize& size
) noexcept {
  return libghostty_cpp_size_report_size {
    size.rows,
    size.columns,
    size.cell_width,
    size.cell_height,
  };
}

libghostty_cpp_color_scheme translate_color_scheme(ColorScheme scheme) {
  switch (scheme) {
    case ColorScheme::Light:
      return LIBGHOSTTY_CPP_COLOR_SCHEME_LIGHT;
    case ColorScheme::Dark:
      return LIBGHOSTTY_CPP_COLOR_SCHEME_DARK;
  }

  throw Error(ErrorCode::InvalidValue);
}

libghostty_cpp_device_attributes translate_device_attributes(
  const DeviceAttributes& attributes
) {
  if (attributes.primary.features.size() > 64) {
    throw Error(ErrorCode::InvalidValue);
  }

  libghostty_cpp_device_attributes translated = {};
  translated.primary.conformance_level = static_cast<std::uint16_t>(
    attributes.primary.conformance_level
  );
  translated.primary.num_features = attributes.primary.features.size();
  for (std::size_t i = 0; i < attributes.primary.features.size(); ++i) {
    translated.primary.features[i] = static_cast<std::uint16_t>(
      attributes.primary.features[i]
    );
  }

  translated.secondary.device_type = static_cast<std::uint16_t>(
    attributes.secondary.device_type
  );
  translated.secondary.firmware_version = attributes.secondary.firmware_version;
  translated.secondary.rom_cartridge = attributes.secondary.rom_cartridge;
  translated.tertiary.unit_id = attributes.tertiary.unit_id;
  return translated;
}

detail::TerminalCallbacks* get_callback_state(void* userdata) noexcept {
  return static_cast<detail::TerminalCallbacks*>(userdata);
}

void dispatch_pty_write(
  const libghostty_cpp_terminal* handle,
  void* userdata,
  const std::uint8_t* data,
  std::size_t len
) {
  detail::TerminalCallbacks* callbacks = get_callback_state(userdata);
  if (callbacks == nullptr
      || callbacks->pending_exception != nullptr
      || !callbacks->pty_write
      || !callbacks->has_owner(handle)) {
    return;
  }

  const std::string_view view = data == nullptr
                                 ? std::string_view {}
                                 : std::string_view(
                                     reinterpret_cast<const char*>(data),
                                     len
                                   );

  try {
    callbacks->pty_write(callbacks->terminal(handle), view);
  } catch (...) {
    callbacks->capture_current_exception();
  }
}

void dispatch_bell(
  const libghostty_cpp_terminal* handle,
  void* userdata
) {
  detail::TerminalCallbacks* callbacks = get_callback_state(userdata);
  if (callbacks == nullptr
      || callbacks->pending_exception != nullptr
      || !callbacks->bell
      || !callbacks->has_owner(handle)) {
    return;
  }

  try {
    callbacks->bell(callbacks->terminal(handle));
  } catch (...) {
    callbacks->capture_current_exception();
  }
}

bool dispatch_size(
  const libghostty_cpp_terminal* handle,
  void* userdata,
  libghostty_cpp_size_report_size* out_size
) {
  detail::TerminalCallbacks* callbacks = get_callback_state(userdata);
  if (callbacks == nullptr
      || callbacks->pending_exception != nullptr
      || !callbacks->size
      || out_size == nullptr
      || !callbacks->has_owner(handle)) {
    return false;
  }

  try {
    const std::optional<SizeReportSize> size = callbacks->size(
      callbacks->terminal(handle)
    );
    if (!size.has_value()) {
      return false;
    }

    *out_size = translate_size_report_size(*size);
    return true;
  } catch (...) {
    callbacks->capture_current_exception();
    return false;
  }
}

bool dispatch_device_attributes(
  const libghostty_cpp_terminal* handle,
  void* userdata,
  libghostty_cpp_device_attributes* out_attributes
) {
  detail::TerminalCallbacks* callbacks = get_callback_state(userdata);
  if (callbacks == nullptr
      || callbacks->pending_exception != nullptr
      || !callbacks->device_attributes
      || out_attributes == nullptr
      || !callbacks->has_owner(handle)) {
    return false;
  }

  try {
    const std::optional<DeviceAttributes> attributes =
      callbacks->device_attributes(callbacks->terminal(handle));
    if (!attributes.has_value()) {
      return false;
    }

    *out_attributes = translate_device_attributes(*attributes);
    return true;
  } catch (...) {
    callbacks->capture_current_exception();
    return false;
  }
}

libghostty_cpp_string dispatch_xtversion(
  const libghostty_cpp_terminal* handle,
  void* userdata
) {
  detail::TerminalCallbacks* callbacks = get_callback_state(userdata);
  if (callbacks == nullptr
      || callbacks->pending_exception != nullptr
      || !callbacks->xtversion
      || !callbacks->has_owner(handle)) {
    return libghostty_cpp_string {nullptr, 0};
  }

  try {
    const std::optional<std::string> xtversion = callbacks->xtversion(
      callbacks->terminal(handle)
    );
    if (!xtversion.has_value()) {
      callbacks->xtversion_storage.clear();
      return libghostty_cpp_string {nullptr, 0};
    }

    callbacks->xtversion_storage = *xtversion;
    return libghostty_cpp_string {
      reinterpret_cast<const std::uint8_t*>(callbacks->xtversion_storage.data()),
      callbacks->xtversion_storage.size(),
    };
  } catch (...) {
    callbacks->capture_current_exception();
    callbacks->xtversion_storage.clear();
    return libghostty_cpp_string {nullptr, 0};
  }
}

void dispatch_title_changed(
  const libghostty_cpp_terminal* handle,
  void* userdata
) {
  detail::TerminalCallbacks* callbacks = get_callback_state(userdata);
  if (callbacks == nullptr
      || callbacks->pending_exception != nullptr
      || !callbacks->title_changed
      || !callbacks->has_owner(handle)) {
    return;
  }

  try {
    callbacks->title_changed(callbacks->terminal(handle));
  } catch (...) {
    callbacks->capture_current_exception();
  }
}

bool dispatch_color_scheme(
  const libghostty_cpp_terminal* handle,
  void* userdata,
  libghostty_cpp_color_scheme* out_scheme
) {
  detail::TerminalCallbacks* callbacks = get_callback_state(userdata);
  if (callbacks == nullptr
      || callbacks->pending_exception != nullptr
      || !callbacks->color_scheme
      || out_scheme == nullptr
      || !callbacks->has_owner(handle)) {
    return false;
  }

  try {
    const std::optional<ColorScheme> scheme = callbacks->color_scheme(
      callbacks->terminal(handle)
    );
    if (!scheme.has_value()) {
      return false;
    }

    *out_scheme = translate_color_scheme(*scheme);
    return true;
  } catch (...) {
    callbacks->capture_current_exception();
    return false;
  }
}

}  // namespace

Terminal::Terminal(TerminalOptions options) {
  auto callbacks = std::make_unique<detail::TerminalCallbacks>();
  callbacks->owner = this;

  libghostty_cpp_terminal_options raw_options = {};
  raw_options.cols = options.cols;
  raw_options.rows = options.rows;
  raw_options.max_scrollback = options.max_scrollback;

  libghostty_cpp_terminal* handle = nullptr;
  detail::throw_if_error(libghostty_cpp_terminal_new(&handle, raw_options));
  try {
    detail::throw_if_error(
      libghostty_cpp_terminal_set_callback_userdata(handle, callbacks.get())
    );
  } catch (...) {
    libghostty_cpp_terminal_free(handle);
    throw;
  }

  handle_ = handle;
  callbacks_ = std::move(callbacks);
}

Terminal::~Terminal() {
  release();
}

Terminal::Terminal(Terminal&& other) noexcept
    : handle_(std::exchange(other.handle_, nullptr)),
      callbacks_(std::move(other.callbacks_)) {
  if (callbacks_ != nullptr) {
    callbacks_->owner = this;
  }
}

Terminal& Terminal::operator=(Terminal&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  release();
  handle_ = std::exchange(other.handle_, nullptr);
  callbacks_ = std::move(other.callbacks_);
  if (callbacks_ != nullptr) {
    callbacks_->owner = this;
  }
  return *this;
}

void Terminal::vt_write(std::string_view data) {
  if (callbacks_ != nullptr) {
    callbacks_->clear_pending_exception();
  }

  libghostty_cpp_terminal_vt_write(
    handle_,
    reinterpret_cast<const std::uint8_t*>(data.data()),
    data.size()
  );

  if (callbacks_ != nullptr) {
    callbacks_->rethrow_pending_exception();
  }
}

void Terminal::reset() noexcept {
  libghostty_cpp_terminal_reset(handle_);
}

bool Terminal::is_mode_enabled(Mode mode) const {
  return query_mode_enabled(handle_, mode);
}

void Terminal::scroll_viewport_delta(std::ptrdiff_t delta) noexcept {
  libghostty_cpp_terminal_scroll_viewport_delta(handle_, delta);
}

void Terminal::resize(
  std::uint16_t cols,
  std::uint16_t rows,
  std::uint32_t cell_width_px,
  std::uint32_t cell_height_px
) {
  detail::throw_if_error(libghostty_cpp_terminal_resize(
    handle_,
    cols,
    rows,
    cell_width_px,
    cell_height_px
  ));
}

Terminal& Terminal::on_pty_write(PtyWriteCallback callback) {
  if (callbacks_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }

  callbacks_->pty_write = std::move(callback);
  detail::throw_if_error(libghostty_cpp_terminal_on_pty_write(
    handle_,
    callbacks_->pty_write ? &dispatch_pty_write : nullptr
  ));
  return *this;
}

Terminal& Terminal::on_bell(BellCallback callback) {
  if (callbacks_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }

  callbacks_->bell = std::move(callback);
  detail::throw_if_error(libghostty_cpp_terminal_on_bell(
    handle_,
    callbacks_->bell ? &dispatch_bell : nullptr
  ));
  return *this;
}

Terminal& Terminal::on_size(SizeCallback callback) {
  if (callbacks_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }

  callbacks_->size = std::move(callback);
  detail::throw_if_error(libghostty_cpp_terminal_on_size(
    handle_,
    callbacks_->size ? &dispatch_size : nullptr
  ));
  return *this;
}

Terminal& Terminal::on_device_attributes(DeviceAttributesCallback callback) {
  if (callbacks_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }

  callbacks_->device_attributes = std::move(callback);
  detail::throw_if_error(libghostty_cpp_terminal_on_device_attributes(
    handle_,
    callbacks_->device_attributes ? &dispatch_device_attributes : nullptr
  ));
  return *this;
}

Terminal& Terminal::on_xtversion(XtversionCallback callback) {
  if (callbacks_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }

  callbacks_->xtversion = std::move(callback);
  detail::throw_if_error(libghostty_cpp_terminal_on_xtversion(
    handle_,
    callbacks_->xtversion ? &dispatch_xtversion : nullptr
  ));
  return *this;
}

Terminal& Terminal::on_title_changed(TitleChangedCallback callback) {
  if (callbacks_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }

  callbacks_->title_changed = std::move(callback);
  detail::throw_if_error(libghostty_cpp_terminal_on_title_changed(
    handle_,
    callbacks_->title_changed ? &dispatch_title_changed : nullptr
  ));
  return *this;
}

Terminal& Terminal::on_color_scheme(ColorSchemeCallback callback) {
  if (callbacks_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }

  callbacks_->color_scheme = std::move(callback);
  detail::throw_if_error(libghostty_cpp_terminal_on_color_scheme(
    handle_,
    callbacks_->color_scheme ? &dispatch_color_scheme : nullptr
  ));
  return *this;
}

std::string_view Terminal::title() const {
  return get_string(handle_, libghostty_cpp_terminal_title);
}

std::uint16_t Terminal::cols() const {
  return get_u16(handle_, libghostty_cpp_terminal_cols);
}

std::uint16_t Terminal::rows() const {
  return get_u16(handle_, libghostty_cpp_terminal_rows);
}

std::uint16_t Terminal::cursor_x() const {
  return get_u16(handle_, libghostty_cpp_terminal_cursor_x);
}

std::uint16_t Terminal::cursor_y() const {
  return get_u16(handle_, libghostty_cpp_terminal_cursor_y);
}

void Terminal::release() noexcept {
  if (callbacks_ != nullptr) {
    callbacks_->owner = nullptr;
    callbacks_->clear_pending_exception();
  }

  libghostty_cpp_terminal_free(handle_);
  handle_ = nullptr;
  callbacks_.reset();
}

}  // namespace libghostty_cpp
