#include "libghostty_cpp/vt.hpp"

#include "c_compat/vt.h"
#include "result.hpp"
#include "style_private.hpp"

#include <array>
#include <exception>
#include <string>
#include <utility>

namespace libghostty_cpp {

namespace detail {

struct TerminalCallbacks {
  Terminal* owner = nullptr;
  PtyWriteCallback pty_write;
  BellCallback bell;
  EnquiryCallback enquiry;
  SizeCallback size;
  DeviceAttributesCallback device_attributes;
  XtversionCallback xtversion;
  TitleChangedCallback title_changed;
  ColorSchemeCallback color_scheme;
  std::string enquiry_storage;
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
using TerminalU32Getter =
  libghostty_cpp_result (*)(const libghostty_cpp_terminal*, uint32_t*);
using TerminalU64Getter =
  libghostty_cpp_result (*)(const libghostty_cpp_terminal*, uint64_t*);
using TerminalBoolGetter =
  libghostty_cpp_result (*)(const libghostty_cpp_terminal*, bool*);
using TerminalSizeGetter =
  libghostty_cpp_result (*)(const libghostty_cpp_terminal*, size_t*);
using TerminalStringGetter =
  libghostty_cpp_result (*)(const libghostty_cpp_terminal*, libghostty_cpp_string*);
using TerminalOptionalColorGetter = libghostty_cpp_result (*) (
  const libghostty_cpp_terminal*,
  bool*,
  libghostty_cpp_rgb_color*
);
using TerminalPaletteGetter =
  libghostty_cpp_result (*)(const libghostty_cpp_terminal*, libghostty_cpp_rgb_color*);

GridCellWide translate_grid_cell_wide(libghostty_cpp_grid_cell_wide wide) {
  switch (wide) {
    case LIBGHOSTTY_CPP_GRID_CELL_WIDE_NARROW:
      return GridCellWide::Narrow;
    case LIBGHOSTTY_CPP_GRID_CELL_WIDE_WIDE:
      return GridCellWide::Wide;
    case LIBGHOSTTY_CPP_GRID_CELL_WIDE_SPACER_TAIL:
      return GridCellWide::SpacerTail;
    case LIBGHOSTTY_CPP_GRID_CELL_WIDE_SPACER_HEAD:
      return GridCellWide::SpacerHead;
  }

  throw Error(ErrorCode::InvalidValue);
}

libghostty_cpp_point translate_point(Point point) noexcept {
  const PointCoordinate coordinate = point.coordinate();
  libghostty_cpp_point raw_point = {};
  raw_point.value.coordinate.x = coordinate.x;
  raw_point.value.coordinate.y = coordinate.y;
  switch (point.kind()) {
    case Point::Kind::Active:
      raw_point.tag = LIBGHOSTTY_CPP_POINT_ACTIVE;
      break;
    case Point::Kind::Viewport:
      raw_point.tag = LIBGHOSTTY_CPP_POINT_VIEWPORT;
      break;
    case Point::Kind::Screen:
      raw_point.tag = LIBGHOSTTY_CPP_POINT_SCREEN;
      break;
    case Point::Kind::History:
      raw_point.tag = LIBGHOSTTY_CPP_POINT_HISTORY;
      break;
  }

  return raw_point;
}

std::uint16_t get_u16(const libghostty_cpp_terminal* handle, TerminalU16Getter getter) {
  std::uint16_t value = 0;
  detail::throw_if_error(getter(handle, &value));
  return value;
}

std::uint32_t get_u32(const libghostty_cpp_terminal* handle, TerminalU32Getter getter) {
  std::uint32_t value = 0;
  detail::throw_if_error(getter(handle, &value));
  return value;
}

std::uint64_t get_u64(const libghostty_cpp_terminal* handle, TerminalU64Getter getter) {
  std::uint64_t value = 0;
  detail::throw_if_error(getter(handle, &value));
  return value;
}

bool get_bool(const libghostty_cpp_terminal* handle, TerminalBoolGetter getter) {
  bool value = false;
  detail::throw_if_error(getter(handle, &value));
  return value;
}

std::size_t get_size(const libghostty_cpp_terminal* handle, TerminalSizeGetter getter) {
  std::size_t value = 0;
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

std::optional<RgbColor> get_optional_color(
  const libghostty_cpp_terminal* handle,
  TerminalOptionalColorGetter getter
) {
  bool has_value = false;
  libghostty_cpp_rgb_color color = {};
  detail::throw_if_error(getter(handle, &has_value, &color));
  if (!has_value) {
    return std::nullopt;
  }

  return detail::translate_color(color);
}

std::array<RgbColor, 256> get_palette(
  const libghostty_cpp_terminal* handle,
  TerminalPaletteGetter getter
) {
  std::array<libghostty_cpp_rgb_color, 256> raw_palette{};
  detail::throw_if_error(getter(handle, raw_palette.data()));

  std::array<RgbColor, 256> palette{};
  for (std::size_t i = 0; i < palette.size(); ++i) {
    palette[i] = detail::translate_color(raw_palette[i]);
  }

  return palette;
}

void set_string_option(
  libghostty_cpp_terminal* handle,
  libghostty_cpp_result (*setter)(libghostty_cpp_terminal*, const libghostty_cpp_string*),
  std::optional<std::string_view> value
) {
  if (!value.has_value()) {
    detail::throw_if_error(setter(handle, nullptr));
    return;
  }

  const libghostty_cpp_string raw_value = {
    reinterpret_cast<const std::uint8_t*>(value->data()),
    value->size(),
  };
  detail::throw_if_error(setter(handle, &raw_value));
}

void set_optional_color_option(
  libghostty_cpp_terminal* handle,
  libghostty_cpp_result (*setter)(libghostty_cpp_terminal*, const libghostty_cpp_rgb_color*),
  std::optional<RgbColor> value
) {
  if (!value.has_value()) {
    detail::throw_if_error(setter(handle, nullptr));
    return;
  }

  const libghostty_cpp_rgb_color raw_color = {value->r, value->g, value->b};
  detail::throw_if_error(setter(handle, &raw_color));
}

void set_palette_option(
  libghostty_cpp_terminal* handle,
  libghostty_cpp_result (*setter)(libghostty_cpp_terminal*, const libghostty_cpp_rgb_color*),
  const std::array<RgbColor, 256>* value
) {
  if (value == nullptr) {
    detail::throw_if_error(setter(handle, nullptr));
    return;
  }

  std::array<libghostty_cpp_rgb_color, 256> raw_palette{};
  for (std::size_t i = 0; i < raw_palette.size(); ++i) {
    raw_palette[i] = {(*value)[i].r, (*value)[i].g, (*value)[i].b};
  }

  detail::throw_if_error(setter(handle, raw_palette.data()));
}

bool query_mode_enabled(const libghostty_cpp_terminal* handle, Mode mode) {
  bool value = false;
  detail::throw_if_error(
    libghostty_cpp_terminal_mode_get(handle, static_cast<std::uint16_t>(mode), &value)
  );
  return value;
}

ActiveScreen translate_active_screen(libghostty_cpp_terminal_screen screen) {
  switch (screen) {
    case LIBGHOSTTY_CPP_TERMINAL_SCREEN_PRIMARY:
      return ActiveScreen::Primary;
    case LIBGHOSTTY_CPP_TERMINAL_SCREEN_ALTERNATE:
      return ActiveScreen::Alternate;
  }

  throw Error(ErrorCode::InvalidValue);
}

ActiveScreen get_active_screen(const libghostty_cpp_terminal* handle) {
  libghostty_cpp_terminal_screen screen = LIBGHOSTTY_CPP_TERMINAL_SCREEN_PRIMARY;
  detail::throw_if_error(libghostty_cpp_terminal_get_active_screen(handle, &screen));
  return translate_active_screen(screen);
}

Scrollbar get_scrollbar(const libghostty_cpp_terminal* handle) {
  libghostty_cpp_terminal_scrollbar scrollbar = {0, 0, 0};
  detail::throw_if_error(libghostty_cpp_terminal_get_scrollbar(handle, &scrollbar));
  return Scrollbar{scrollbar.total, scrollbar.offset, scrollbar.len};
}

libghostty_cpp_terminal_scroll_viewport translate_scroll_viewport(ScrollViewport scroll) {
  libghostty_cpp_terminal_scroll_viewport raw_viewport = {};
  switch (scroll.kind()) {
    case ScrollViewport::Kind::Top:
      raw_viewport.tag = LIBGHOSTTY_CPP_SCROLL_VIEWPORT_TOP;
      break;
    case ScrollViewport::Kind::Bottom:
      raw_viewport.tag = LIBGHOSTTY_CPP_SCROLL_VIEWPORT_BOTTOM;
      break;
    case ScrollViewport::Kind::Delta:
      raw_viewport.tag = LIBGHOSTTY_CPP_SCROLL_VIEWPORT_DELTA;
      raw_viewport.value.delta = scroll.delta().value_or(0);
      break;
  }

  return raw_viewport;
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

libghostty_cpp_string dispatch_enquiry(
  const libghostty_cpp_terminal* handle,
  void* userdata
) {
  detail::TerminalCallbacks* callbacks = get_callback_state(userdata);
  if (callbacks == nullptr
      || callbacks->pending_exception != nullptr
      || !callbacks->enquiry
      || !callbacks->has_owner(handle)) {
    return libghostty_cpp_string {nullptr, 0};
  }

  try {
    const std::optional<std::string> enquiry = callbacks->enquiry(
      callbacks->terminal(handle)
    );
    if (!enquiry.has_value()) {
      callbacks->enquiry_storage.clear();
      return libghostty_cpp_string {nullptr, 0};
    }

    callbacks->enquiry_storage = *enquiry;
    return libghostty_cpp_string {
      reinterpret_cast<const std::uint8_t*>(callbacks->enquiry_storage.data()),
      callbacks->enquiry_storage.size(),
    };
  } catch (...) {
    callbacks->capture_current_exception();
    callbacks->enquiry_storage.clear();
    return libghostty_cpp_string {nullptr, 0};
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

bool Terminal::mode(Mode mode) const {
  return query_mode_enabled(handle_, mode);
}

bool Terminal::is_mode_enabled(Mode mode) const {
  return this->mode(mode);
}

Terminal& Terminal::set_mode(Mode mode, bool value) {
  detail::throw_if_error(
    libghostty_cpp_terminal_mode_set(handle_, static_cast<std::uint16_t>(mode), value)
  );
  return *this;
}

GridRef Terminal::grid_ref(Point point) const {
  const libghostty_cpp_point raw_point = translate_point(point);

  libghostty_cpp_grid_ref_snapshot snapshot = {};
  detail::throw_if_error(
    libghostty_cpp_terminal_grid_ref_snapshot(handle_, raw_point, &snapshot)
  );

  std::uint64_t row = 0;
  detail::throw_if_error(libghostty_cpp_terminal_grid_ref_row(handle_, raw_point, &row));

  std::uint64_t cell = 0;
  detail::throw_if_error(libghostty_cpp_terminal_grid_ref_cell(handle_, raw_point, &cell));

  libghostty_cpp_style style = {};
  detail::throw_if_error(libghostty_cpp_terminal_grid_ref_style(handle_, raw_point, &style));

  std::size_t len = 0;
  detail::throw_if_error(
    libghostty_cpp_terminal_grid_ref_graphemes_len(handle_, raw_point, &len)
  );

  std::u32string graphemes;
  if (len > 0) {
    std::vector<std::uint32_t> codepoints(len, 0);
    detail::throw_if_error(
      libghostty_cpp_terminal_grid_ref_graphemes(handle_, raw_point, codepoints.data(), len)
    );

    graphemes.reserve(len);
    for (const std::uint32_t codepoint : codepoints) {
      graphemes.push_back(static_cast<char32_t>(codepoint));
    }
  }

  std::size_t uri_len = 0;
  detail::throw_if_error(
    libghostty_cpp_terminal_grid_ref_hyperlink_uri_len(handle_, raw_point, &uri_len)
  );

  std::string hyperlink_uri;
  if (uri_len > 0) {
    hyperlink_uri.resize(uri_len);
    detail::throw_if_error(
      libghostty_cpp_terminal_grid_ref_hyperlink_uri(
        handle_, raw_point,
        reinterpret_cast<std::uint8_t*>(hyperlink_uri.data()), uri_len
      )
    );
  }

  return GridRef(
    screen::Row(row),
    screen::Cell(cell),
    detail::translate_style(style),
    snapshot.row_is_wrapped,
    snapshot.cell_has_text,
    translate_grid_cell_wide(snapshot.cell_wide),
    std::move(graphemes),
    std::move(hyperlink_uri)
  );
}

void Terminal::scroll_viewport(ScrollViewport scroll) noexcept {
  libghostty_cpp_terminal_set_scroll_viewport(handle_, translate_scroll_viewport(scroll));
}

void Terminal::scroll_viewport_delta(std::ptrdiff_t delta) noexcept {
  scroll_viewport(ScrollViewport::delta(delta));
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

Terminal& Terminal::on_enquiry(EnquiryCallback callback) {
  if (callbacks_ == nullptr) {
    throw Error(ErrorCode::InvalidState);
  }

  callbacks_->enquiry = std::move(callback);
  detail::throw_if_error(libghostty_cpp_terminal_on_enquiry(
    handle_,
    callbacks_->enquiry ? &dispatch_enquiry : nullptr
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

Terminal& Terminal::set_title(std::string_view value) {
  set_string_option(handle_, libghostty_cpp_terminal_set_title, value);
  return *this;
}

Terminal& Terminal::clear_title() {
  set_string_option(handle_, libghostty_cpp_terminal_set_title, std::nullopt);
  return *this;
}

Terminal& Terminal::set_pwd(std::string_view value) {
  set_string_option(handle_, libghostty_cpp_terminal_set_pwd, value);
  return *this;
}

Terminal& Terminal::clear_pwd() {
  set_string_option(handle_, libghostty_cpp_terminal_set_pwd, std::nullopt);
  return *this;
}

std::string_view Terminal::title() const {
  return get_string(handle_, libghostty_cpp_terminal_title);
}

std::string_view Terminal::pwd() const {
  return get_string(handle_, libghostty_cpp_terminal_pwd);
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

bool Terminal::is_cursor_pending_wrap() const {
  return get_bool(handle_, libghostty_cpp_terminal_cursor_pending_wrap);
}

bool Terminal::is_cursor_visible() const {
  return get_bool(handle_, libghostty_cpp_terminal_cursor_visible);
}

key::KittyKeyFlags Terminal::kitty_keyboard_flags() const {
  std::uint8_t value = 0;
  detail::throw_if_error(libghostty_cpp_terminal_kitty_keyboard_flags(handle_, &value));
  return static_cast<key::KittyKeyFlags>(value);
}

Style Terminal::cursor_style() const {
  libghostty_cpp_style style = {};
  detail::throw_if_error(libghostty_cpp_terminal_cursor_style(handle_, &style));
  return detail::translate_style(style);
}

ActiveScreen Terminal::active_screen() const {
  return get_active_screen(handle_);
}

Scrollbar Terminal::scrollbar() const {
  return get_scrollbar(handle_);
}

bool Terminal::is_mouse_tracking() const {
  return get_bool(handle_, libghostty_cpp_terminal_mouse_tracking);
}

std::size_t Terminal::total_rows() const {
  return get_size(handle_, libghostty_cpp_terminal_total_rows);
}

std::size_t Terminal::scrollback_rows() const {
  return get_size(handle_, libghostty_cpp_terminal_scrollback_rows);
}

std::uint32_t Terminal::width_px() const {
  return get_u32(handle_, libghostty_cpp_terminal_width_px);
}

std::uint32_t Terminal::height_px() const {
  return get_u32(handle_, libghostty_cpp_terminal_height_px);
}

std::optional<RgbColor> Terminal::fg_color() const {
  return get_optional_color(handle_, libghostty_cpp_terminal_color_foreground);
}

std::optional<RgbColor> Terminal::default_fg_color() const {
  return get_optional_color(handle_, libghostty_cpp_terminal_color_foreground_default);
}

Terminal& Terminal::set_default_fg_color(std::optional<RgbColor> value) {
  set_optional_color_option(handle_, libghostty_cpp_terminal_set_color_foreground_default, value);
  return *this;
}

std::optional<RgbColor> Terminal::bg_color() const {
  return get_optional_color(handle_, libghostty_cpp_terminal_color_background);
}

std::optional<RgbColor> Terminal::default_bg_color() const {
  return get_optional_color(handle_, libghostty_cpp_terminal_color_background_default);
}

Terminal& Terminal::set_default_bg_color(std::optional<RgbColor> value) {
  set_optional_color_option(handle_, libghostty_cpp_terminal_set_color_background_default, value);
  return *this;
}

std::optional<RgbColor> Terminal::cursor_color() const {
  return get_optional_color(handle_, libghostty_cpp_terminal_color_cursor);
}

std::optional<RgbColor> Terminal::default_cursor_color() const {
  return get_optional_color(handle_, libghostty_cpp_terminal_color_cursor_default);
}

Terminal& Terminal::set_default_cursor_color(std::optional<RgbColor> value) {
  set_optional_color_option(handle_, libghostty_cpp_terminal_set_color_cursor_default, value);
  return *this;
}

std::array<RgbColor, 256> Terminal::color_palette() const {
  return get_palette(handle_, libghostty_cpp_terminal_color_palette);
}

std::array<RgbColor, 256> Terminal::default_color_palette() const {
  return get_palette(handle_, libghostty_cpp_terminal_color_palette_default);
}

Terminal& Terminal::set_default_color_palette(const std::array<RgbColor, 256>& value) {
  set_palette_option(handle_, libghostty_cpp_terminal_set_color_palette_default, &value);
  return *this;
}

Terminal& Terminal::clear_default_color_palette() {
  set_palette_option(handle_, libghostty_cpp_terminal_set_color_palette_default, nullptr);
  return *this;
}

std::uint64_t Terminal::kitty_image_storage_limit() const {
  return get_u64(handle_, libghostty_cpp_terminal_kitty_image_storage_limit);
}

Terminal& Terminal::set_kitty_image_storage_limit(std::uint64_t limit) {
  detail::throw_if_error(libghostty_cpp_terminal_set_kitty_image_storage_limit(handle_, limit));
  return *this;
}

bool Terminal::is_kitty_image_from_file_allowed() const {
  return get_bool(handle_, libghostty_cpp_terminal_kitty_image_from_file_allowed);
}

Terminal& Terminal::set_kitty_image_from_file_allowed(bool allowed) {
  detail::throw_if_error(libghostty_cpp_terminal_set_kitty_image_from_file_allowed(handle_, allowed));
  return *this;
}

bool Terminal::is_kitty_image_from_temp_file_allowed() const {
  return get_bool(handle_, libghostty_cpp_terminal_kitty_image_from_temp_file_allowed);
}

Terminal& Terminal::set_kitty_image_from_temp_file_allowed(bool allowed) {
  detail::throw_if_error(libghostty_cpp_terminal_set_kitty_image_from_temp_file_allowed(handle_, allowed));
  return *this;
}

bool Terminal::is_kitty_image_from_shared_mem_allowed() const {
  return get_bool(handle_, libghostty_cpp_terminal_kitty_image_from_shared_mem_allowed);
}

Terminal& Terminal::set_kitty_image_from_shared_mem_allowed(bool allowed) {
  detail::throw_if_error(libghostty_cpp_terminal_set_kitty_image_from_shared_mem_allowed(handle_, allowed));
  return *this;
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
