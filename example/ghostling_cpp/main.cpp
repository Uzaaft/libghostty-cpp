#include "libghostty_cpp/key.hpp"
#include "libghostty_cpp/mouse.hpp"
#include "libghostty_cpp/render.hpp"
#include "libghostty_cpp/vt.hpp"

#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QColor>
#include <QCoreApplication>
#include <QFocusEvent>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QString>
#include <QStringList>
#include <QSocketNotifier>
#include <QWheelEvent>
#include <QWidget>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#if defined(__linux__)
#include <pty.h>
#else
#include <util.h>
#endif

namespace {

constexpr double kFontPointSize = 11.0;
constexpr double kPadding = 6.0;
constexpr double kCellGap = 0.0;
constexpr double kRowGap = 2.0;
constexpr std::size_t kScrollbackLines = 1000;
constexpr std::size_t kEncodedInputReserve = 64;
constexpr double kWheelScrollScale = -2.5;
constexpr std::string_view kBracketedPasteStartSequence = "\x1B[200~";
constexpr std::string_view kBracketedPasteEndSequence = "\x1B[201~";
constexpr std::string_view kFocusGainedSequence = "\x1B[I";
constexpr std::string_view kFocusLostSequence = "\x1B[O";
constexpr int kInitialWindowWidth = 1024;
constexpr int kInitialWindowHeight = 720;
constexpr double kScrollbarWidth = 3.0;
constexpr double kScrollbarMinThumbHeight = 18.0;

struct GridMetrics {
  double cell_width = 1.0;
  double cell_height = 1.0;
  double baseline = 1.0;
  std::uint16_t cols = 1;
  std::uint16_t rows = 1;
  std::uint32_t cell_width_px = 1;
  std::uint32_t cell_height_px = 1;
  std::uint16_t window_width_px = 1;
  std::uint16_t window_height_px = 1;
};

struct Selection {
  libghostty_cpp::PointCoordinate anchor;
  libghostty_cpp::PointCoordinate focus;
};

struct SelectionBounds {
  libghostty_cpp::PointCoordinate start;
  libghostty_cpp::PointCoordinate end;
};

struct KeyMapping {
  libghostty_cpp::key::Key key;
  char32_t unshifted_codepoint;
};

[[nodiscard]] libghostty_cpp::key::Key offset_key(
  libghostty_cpp::key::Key base,
  int offset
) {
  return static_cast<libghostty_cpp::key::Key>(
    static_cast<std::uint32_t>(base) + static_cast<std::uint32_t>(offset)
  );
}

[[nodiscard]] libghostty_cpp::DeviceAttributes default_device_attributes() {
  libghostty_cpp::DeviceAttributes attributes;
  attributes.primary.conformance_level = libghostty_cpp::ConformanceLevel::VT220;
  attributes.primary.features = {
    libghostty_cpp::DeviceAttributeFeature::Columns132,
    libghostty_cpp::DeviceAttributeFeature::SelectiveErase,
    libghostty_cpp::DeviceAttributeFeature::AnsiColor,
  };
  attributes.secondary.device_type = libghostty_cpp::DeviceType::VT220;
  attributes.secondary.firmware_version = 1;
  attributes.secondary.rom_cartridge = 0;
  return attributes;
}

[[nodiscard]] std::uint16_t clamp_to_u16(long value) {
  if (value <= 1) {
    return 1;
  }

  if (value >= 0xFFFF) {
    return 0xFFFF;
  }

  return static_cast<std::uint16_t>(value);
}

[[nodiscard]] QColor to_qcolor(libghostty_cpp::RgbColor color) {
  return QColor(color.r, color.g, color.b);
}

[[nodiscard]] bool has_text_modifiers(Qt::KeyboardModifiers modifiers) {
  return modifiers.testFlag(Qt::ControlModifier)
         || modifiers.testFlag(Qt::AltModifier)
         || modifiers.testFlag(Qt::MetaModifier);
}

[[nodiscard]] bool is_printable_text(const QString& text) {
  if (text.isEmpty()) {
    return false;
  }

  for (const QChar ch : text) {
    if (ch.isNull() || ch.isLowSurrogate() || ch.isHighSurrogate()) {
      return false;
    }

    if (ch.isPrint()) {
      continue;
    }

    if (ch == QLatin1Char(' ')) {
      continue;
    }

    return false;
  }

  return true;
}

[[nodiscard]] std::optional<KeyMapping> map_qt_key(int key) {
  using libghostty_cpp::key::Key;

  if (key >= Qt::Key_A && key <= Qt::Key_Z) {
    const int offset = key - Qt::Key_A;
    return KeyMapping{offset_key(Key::A, offset), U'a' + static_cast<char32_t>(offset)};
  }

  if (key >= Qt::Key_0 && key <= Qt::Key_9) {
    const int offset = key - Qt::Key_0;
    return KeyMapping{
      offset_key(Key::Digit0, offset),
      U'0' + static_cast<char32_t>(offset),
    };
  }

  if (key >= Qt::Key_F1 && key <= Qt::Key_F12) {
    return KeyMapping{offset_key(Key::F1, key - Qt::Key_F1), U'\0'};
  }

  switch (key) {
    case Qt::Key_Space:
      return KeyMapping{Key::Space, U' '};
    case Qt::Key_Return:
      return KeyMapping{Key::Enter, U'\0'};
    case Qt::Key_Enter:
      return KeyMapping{Key::NumpadEnter, U'\0'};
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
      return KeyMapping{Key::Tab, U'\0'};
    case Qt::Key_Backspace:
      return KeyMapping{Key::Backspace, U'\0'};
    case Qt::Key_Delete:
      return KeyMapping{Key::Delete, U'\0'};
    case Qt::Key_Escape:
      return KeyMapping{Key::Escape, U'\0'};
    case Qt::Key_Up:
      return KeyMapping{Key::ArrowUp, U'\0'};
    case Qt::Key_Down:
      return KeyMapping{Key::ArrowDown, U'\0'};
    case Qt::Key_Left:
      return KeyMapping{Key::ArrowLeft, U'\0'};
    case Qt::Key_Right:
      return KeyMapping{Key::ArrowRight, U'\0'};
    case Qt::Key_Home:
      return KeyMapping{Key::Home, U'\0'};
    case Qt::Key_End:
      return KeyMapping{Key::End, U'\0'};
    case Qt::Key_PageUp:
      return KeyMapping{Key::PageUp, U'\0'};
    case Qt::Key_PageDown:
      return KeyMapping{Key::PageDown, U'\0'};
    case Qt::Key_Insert:
      return KeyMapping{Key::Insert, U'\0'};
    case Qt::Key_Minus:
      return KeyMapping{Key::Minus, U'-'};
    case Qt::Key_Equal:
      return KeyMapping{Key::Equal, U'='};
    case Qt::Key_BracketLeft:
      return KeyMapping{Key::BracketLeft, U'['};
    case Qt::Key_BracketRight:
      return KeyMapping{Key::BracketRight, U']'};
    case Qt::Key_Backslash:
      return KeyMapping{Key::Backslash, U'\\'};
    case Qt::Key_Semicolon:
      return KeyMapping{Key::Semicolon, U';'};
    case Qt::Key_Apostrophe:
      return KeyMapping{Key::Quote, U'\''};
    case Qt::Key_Comma:
      return KeyMapping{Key::Comma, U','};
    case Qt::Key_Period:
      return KeyMapping{Key::Period, U'.'};
    case Qt::Key_Slash:
      return KeyMapping{Key::Slash, U'/'};
    case Qt::Key_QuoteLeft:
      return KeyMapping{Key::Backquote, U'`'};
    default:
      return std::nullopt;
  }
}

[[nodiscard]] libghostty_cpp::key::Mods to_ghostty_mods(
  Qt::KeyboardModifiers modifiers
) {
  using libghostty_cpp::key::Mods;

  Mods result = Mods::None;
  if (modifiers.testFlag(Qt::ShiftModifier)) {
    result |= Mods::Shift;
  }
  if (modifiers.testFlag(Qt::ControlModifier)) {
    result |= Mods::Ctrl;
  }
  if (modifiers.testFlag(Qt::AltModifier)) {
    result |= Mods::Alt;
  }
  if (modifiers.testFlag(Qt::MetaModifier)) {
    result |= Mods::Super;
  }
  return result;
}

[[nodiscard]] QByteArray encoder_utf8_text(const QKeyEvent& event) {
  if (has_text_modifiers(event.modifiers())) {
    return {};
  }

  if (!is_printable_text(event.text())) {
    return {};
  }

  return event.text().toUtf8();
}

[[nodiscard]] Qt::KeyboardModifiers relevant_keyboard_modifiers(
  Qt::KeyboardModifiers modifiers
) {
  return modifiers
         & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier);
}

[[nodiscard]] bool is_shift_insert_paste(const QKeyEvent& event) {
  return event.key() == Qt::Key_Insert
         && relevant_keyboard_modifiers(event.modifiers()) == Qt::ShiftModifier;
}

[[nodiscard]] bool is_paste_shortcut(const QKeyEvent& event) {
  return event.matches(QKeySequence::Paste) || is_shift_insert_paste(event);
}

[[nodiscard]] bool is_copy_shortcut(const QKeyEvent& event) {
  return event.key() == Qt::Key_C
         && relevant_keyboard_modifiers(event.modifiers())
              == (Qt::ShiftModifier | Qt::ControlModifier);
}

[[nodiscard]] bool point_precedes_or_equals(
  libghostty_cpp::PointCoordinate lhs,
  libghostty_cpp::PointCoordinate rhs
) noexcept {
  return lhs.y < rhs.y || (lhs.y == rhs.y && lhs.x <= rhs.x);
}

[[nodiscard]] std::optional<libghostty_cpp::mouse::Button> map_qt_mouse_button(
  Qt::MouseButton button
) {
  using libghostty_cpp::mouse::Button;

  switch (button) {
    case Qt::LeftButton:
      return Button::Left;
    case Qt::RightButton:
      return Button::Right;
    case Qt::MiddleButton:
      return Button::Middle;
    default:
      return std::nullopt;
  }
}

[[nodiscard]] std::optional<libghostty_cpp::mouse::Button> first_pressed_mouse_button(
  Qt::MouseButtons buttons
) {
  constexpr std::array<Qt::MouseButton, 3> kSupportedButtons = {
    Qt::LeftButton,
    Qt::RightButton,
    Qt::MiddleButton,
  };

  for (const Qt::MouseButton button : kSupportedButtons) {
    if (buttons.testFlag(button)) {
      return map_qt_mouse_button(button);
    }
  }

  return std::nullopt;
}

[[nodiscard]] double wheel_steps(const QWheelEvent& event) {
  const QPoint pixel_delta = event.pixelDelta();
  if (!pixel_delta.isNull()) {
    return static_cast<double>(pixel_delta.y()) / 40.0;
  }

  const QPoint angle_delta = event.angleDelta();
  if (!angle_delta.isNull()) {
    return static_cast<double>(angle_delta.y()) / 120.0;
  }

  return 0.0;
}

[[nodiscard]] QString terminal_font_path() {
  return QCoreApplication::applicationDirPath() + "/JetBrainsMono-Medium.ttf";
}

[[nodiscard]] QFont load_terminal_font() {
  const QString font_path = terminal_font_path();
  const int font_id = QFontDatabase::addApplicationFont(font_path);
  if (font_id < 0) {
    throw std::runtime_error(
      std::string("failed to load bundled font: ") + font_path.toStdString()
    );
  }

  const QStringList families = QFontDatabase::applicationFontFamilies(font_id);
  if (families.isEmpty()) {
    throw std::runtime_error("bundled font did not register any font families");
  }

  QFont font(families.front());
  font.setStyleHint(QFont::Monospace);
  font.setFixedPitch(true);
  font.setPointSizeF(kFontPointSize);
  return font;
}

class TerminalWidget final : public QWidget {
public:
  TerminalWidget()
      : terminal_font_(load_terminal_font()),
        terminal_(libghostty_cpp::TerminalOptions{1, 1, kScrollbackLines}) {
    setWindowTitle(QStringLiteral("ghostling-cpp"));
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(true);
    resize(kInitialWindowWidth, kInitialWindowHeight);
    setFont(terminal_font_);
    encoded_input_.reserve(kEncodedInputReserve);

    configure_terminal();
    apply_grid(measure_grid());
    spawn_shell();
  }

  ~TerminalWidget() override {
    close_pty();
    reap_child(true);
  }

protected:
  void paintEvent(QPaintEvent*) override {
    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    try {
      render_terminal(painter);
    } catch (const std::exception& error) {
      paint_error(painter, error.what());
    }
  }

  void resizeEvent(QResizeEvent* event) override {
    QWidget::resizeEvent(event);
    apply_grid(measure_grid());
    resize_pty();
    update();
  }

  void keyPressEvent(QKeyEvent* event) override {
    try {
      if (handle_key_press(*event)) {
        event->accept();
        return;
      }
    } catch (const std::exception& error) {
      handle_input_error(error);
      event->accept();
      return;
    }

    QWidget::keyPressEvent(event);
  }

  void keyReleaseEvent(QKeyEvent* event) override {
    try {
      if (handle_key_release(*event)) {
        event->accept();
        return;
      }
    } catch (const std::exception& error) {
      handle_input_error(error);
      event->accept();
      return;
    }

    QWidget::keyReleaseEvent(event);
  }

  void focusInEvent(QFocusEvent* event) override {
    try {
      report_focus(true);
    } catch (const std::exception& error) {
      handle_input_error(error);
    }

    QWidget::focusInEvent(event);
  }

  void focusOutEvent(QFocusEvent* event) override {
    try {
      report_focus(false);
    } catch (const std::exception& error) {
      handle_input_error(error);
    }

    QWidget::focusOutEvent(event);
  }

  void mousePressEvent(QMouseEvent* event) override {
    setFocus(Qt::MouseFocusReason);

    try {
      if (handle_mouse_button(*event, libghostty_cpp::mouse::Action::Press)) {
        event->accept();
        return;
      }
    } catch (const std::exception& error) {
      handle_input_error(error);
      event->accept();
      return;
    }

    QWidget::mousePressEvent(event);
  }

  void mouseReleaseEvent(QMouseEvent* event) override {
    try {
      if (handle_mouse_button(*event, libghostty_cpp::mouse::Action::Release)) {
        event->accept();
        return;
      }
    } catch (const std::exception& error) {
      handle_input_error(error);
      event->accept();
      return;
    }

    QWidget::mouseReleaseEvent(event);
  }

  void mouseMoveEvent(QMouseEvent* event) override {
    try {
      if (handle_mouse_move(*event)) {
        event->accept();
        return;
      }
    } catch (const std::exception& error) {
      handle_input_error(error);
      event->accept();
      return;
    }

    QWidget::mouseMoveEvent(event);
  }

  void wheelEvent(QWheelEvent* event) override {
    try {
      if (handle_wheel(*event)) {
        event->accept();
        return;
      }
    } catch (const std::exception& error) {
      handle_input_error(error);
      event->accept();
      return;
    }

    QWidget::wheelEvent(event);
  }

private:
  [[nodiscard]] bool has_active_pty() const noexcept {
    return pty_fd_ >= 0;
  }

  [[nodiscard]] bool is_mode_enabled(libghostty_cpp::Mode mode) const {
    return terminal_.is_mode_enabled(mode);
  }

  void clear_selection() noexcept {
    selection_.reset();
    selection_in_progress_ = false;
  }

  void clear_primary_selection() const {
    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard == nullptr || !clipboard->supportsSelection()) {
      return;
    }

    clipboard->setText(QString{}, QClipboard::Selection);
  }

  void clear_selection_if_any() {
    if (!selection_.has_value()) {
      return;
    }

    clear_selection();
    clear_primary_selection();
    update();
  }

  [[nodiscard]] std::optional<SelectionBounds> selection_bounds() const noexcept {
    if (!selection_.has_value()) {
      return std::nullopt;
    }

    if (point_precedes_or_equals(selection_->anchor, selection_->focus)) {
      return SelectionBounds{selection_->anchor, selection_->focus};
    }

    return SelectionBounds{selection_->focus, selection_->anchor};
  }

  [[nodiscard]] bool is_selected_cell(
    std::uint16_t row_index,
    std::uint16_t column_index
  ) const noexcept {
    const std::optional<SelectionBounds> bounds = selection_bounds();
    if (!bounds.has_value()) {
      return false;
    }

    const libghostty_cpp::PointCoordinate point{column_index, row_index};
    return point_precedes_or_equals(bounds->start, point)
           && point_precedes_or_equals(point, bounds->end);
  }

  [[nodiscard]] std::optional<libghostty_cpp::PointCoordinate> viewport_point_from_position(
    const QPointF& position,
    bool clamp_to_grid
  ) const {
    const double width = static_cast<double>(grid_.cols) * grid_.cell_width;
    const double height = static_cast<double>(grid_.rows) * grid_.cell_height;
    if (width <= 0.0 || height <= 0.0) {
      return std::nullopt;
    }

    double x = position.x() - kPadding;
    double y = position.y() - kPadding;
    if (!clamp_to_grid) {
      if (x < 0.0 || y < 0.0 || x >= width || y >= height) {
        return std::nullopt;
      }
    } else {
      x = std::clamp(x, 0.0, std::max(0.0, width - 1e-6));
      y = std::clamp(y, 0.0, std::max(0.0, height - 1e-6));
    }

    const long column = static_cast<long>(std::floor(x / grid_.cell_width));
    const long row = static_cast<long>(std::floor(y / grid_.cell_height));
    return libghostty_cpp::PointCoordinate{
      static_cast<std::uint16_t>(std::clamp<long>(column, 0, grid_.cols - 1)),
      static_cast<std::uint32_t>(std::clamp<long>(row, 0, grid_.rows - 1)),
    };
  }

  [[nodiscard]] QString selection_text() const {
    const std::optional<SelectionBounds> bounds = selection_bounds();
    if (!bounds.has_value()) {
      return {};
    }

    std::u32string text;
    for (std::uint32_t row = bounds->start.y;; ++row) {
      const std::uint16_t first_column = row == bounds->start.y ? bounds->start.x : 0;
      const std::uint16_t last_column = row == bounds->end.y ? bounds->end.x : grid_.cols - 1;

      bool row_is_wrapped = false;
      std::u32string row_text;
      for (std::uint16_t column = first_column; column <= last_column; ++column) {
        const libghostty_cpp::GridRef ref = terminal_.grid_ref(
          libghostty_cpp::Point::viewport(libghostty_cpp::PointCoordinate{column, row})
        );
        row_is_wrapped = ref.row_is_wrapped();

        if (ref.cell_wide() == libghostty_cpp::GridCellWide::SpacerTail
            || ref.cell_wide() == libghostty_cpp::GridCellWide::SpacerHead) {
          continue;
        }

        if (ref.cell_has_text()) {
          row_text += ref.graphemes();
          continue;
        }

        row_text.push_back(U' ');
      }

      if (!row_is_wrapped) {
        while (!row_text.empty() && row_text.back() == U' ') {
          row_text.pop_back();
        }
      }

      text += row_text;
      if (row == bounds->end.y) {
        break;
      }

      if (!row_is_wrapped) {
        text.push_back(U'\n');
      }
    }

    if (text.empty()) {
      return {};
    }

    return QString::fromUcs4(text.data(), static_cast<qsizetype>(text.size()));
  }

  void sync_primary_selection() const {
    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard == nullptr || !clipboard->supportsSelection()) {
      return;
    }

    clipboard->setText(selection_text(), QClipboard::Selection);
  }

  [[nodiscard]] bool copy_selection_to_clipboard() const {
    const QString text = selection_text();
    if (text.isEmpty()) {
      return false;
    }

    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard == nullptr) {
      return false;
    }

    clipboard->setText(text, QClipboard::Clipboard);
    return true;
  }

  [[nodiscard]] bool begin_selection(const QPointF& position) {
    const bool had_selection = selection_.has_value();
    clear_selection();

    const std::optional<libghostty_cpp::PointCoordinate> point =
      viewport_point_from_position(position, false);
    if (!point.has_value()) {
      if (had_selection) {
        clear_primary_selection();
        update();
      }

      return had_selection;
    }

    selection_ = Selection{*point, *point};
    selection_in_progress_ = true;
    sync_primary_selection();
    update();
    return true;
  }

  [[nodiscard]] bool update_selection(const QPointF& position, bool clamp_to_grid) {
    if (!selection_.has_value() || !selection_in_progress_) {
      return false;
    }

    const std::optional<libghostty_cpp::PointCoordinate> point =
      viewport_point_from_position(position, clamp_to_grid);
    if (!point.has_value()) {
      return false;
    }

    selection_->focus = *point;
    sync_primary_selection();
    update();
    return true;
  }

  [[nodiscard]] bool finish_selection(const QPointF& position) {
    if (!selection_in_progress_) {
      return false;
    }

    const bool handled = update_selection(position, true);
    selection_in_progress_ = false;
    return handled;
  }

  void handle_input_error(const std::exception& error) {
    paint_error_frame_ = QString::fromUtf8(error.what());
    update();
  }

  void sync_window_title() {
    const std::string_view title = terminal_.title();
    if (title.empty()) {
      setWindowTitle(QStringLiteral("ghostling-cpp"));
      return;
    }

    setWindowTitle(QString::fromUtf8(title.data(), static_cast<qsizetype>(title.size())));
  }

  void configure_terminal() {
    terminal_
      .on_pty_write([this](const libghostty_cpp::Terminal&, std::string_view data) {
        write_pty(data);
      })
      .on_bell([](const libghostty_cpp::Terminal&) { QApplication::beep(); })
      .on_size([this](const libghostty_cpp::Terminal&)
                 -> std::optional<libghostty_cpp::SizeReportSize> {
        return libghostty_cpp::SizeReportSize{
          grid_.rows,
          grid_.cols,
          grid_.cell_width_px,
          grid_.cell_height_px,
        };
      })
      .on_device_attributes(
        [](const libghostty_cpp::Terminal&)
          -> std::optional<libghostty_cpp::DeviceAttributes> {
          return default_device_attributes();
        }
      )
      .on_xtversion([](const libghostty_cpp::Terminal&)
                      -> std::optional<std::string> {
        return std::string("ghostling-cpp");
      })
      .on_title_changed([this](const libghostty_cpp::Terminal&) { title_dirty_ = true; })
      .on_color_scheme([this](const libghostty_cpp::Terminal&)
                         -> std::optional<libghostty_cpp::ColorScheme> {
        const QColor window = palette().color(QPalette::Window);
        if (window.lightness() < 128) {
          return libghostty_cpp::ColorScheme::Dark;
        }

        return libghostty_cpp::ColorScheme::Light;
      });
  }

  void apply_grid(GridMetrics metrics) {
    clear_selection();
    clear_primary_selection();
    grid_ = metrics;
    terminal_.resize(
      grid_.cols,
      grid_.rows,
      grid_.cell_width_px,
      grid_.cell_height_px
    );
  }

  void paint_error(QPainter& painter, std::string_view message) {
    paint_error_frame_ = QString::fromUtf8(message.data(), static_cast<qsizetype>(message.size()));
    painter.fillRect(rect(), QColor(32, 0, 0));
    painter.setPen(QColor(255, 220, 220));
    painter.drawText(
      rect().adjusted(12, 12, -12, -12),
      Qt::TextWordWrap,
      paint_error_frame_
    );
  }

  void render_terminal(QPainter& painter) {
    render_state_.update(terminal_);
    const libghostty_cpp::RenderColors colors = render_state_.colors();
    const QColor background = to_qcolor(colors.background);
    const QColor foreground = to_qcolor(colors.foreground);
    const std::optional<libghostty_cpp::CursorViewport> cursor =
      render_state_.cursor_visible() ? render_state_.cursor_viewport() : std::nullopt;
    const QColor cursor_color = colors.cursor.has_value()
                                 ? to_qcolor(*colors.cursor)
                                 : foreground;

    paint_error_frame_.clear();
    painter.fillRect(rect(), background);
    row_iterator_.bind(render_state_);

    std::uint16_t row_index = 0;
    while (row_iterator_.next()) {
      double x = kPadding;
      const double y = kPadding + static_cast<double>(row_index) * grid_.cell_height;
      cell_iterator_.bind(row_iterator_);

      std::uint16_t column_index = 0;
      while (cell_iterator_.next()) {
        render_cell(
          painter,
          x,
          y,
          row_index,
          column_index,
          background,
          foreground,
          cursor,
          cursor_color
        );
        x += grid_.cell_width;
        ++column_index;
      }

      ++row_index;
    }

    render_scrollbar(painter, foreground);

    render_state_.set_dirty(libghostty_cpp::Dirty::Clean);
  }

  void render_scrollbar(QPainter& painter, const QColor& foreground) const {
    if (terminal_.active_screen() == libghostty_cpp::ActiveScreen::Alternate) {
      return;
    }

    const libghostty_cpp::Scrollbar scrollbar = terminal_.scrollbar();
    if (scrollbar.total <= scrollbar.len || scrollbar.len == 0) {
      return;
    }

    const double track_height = static_cast<double>(grid_.rows) * grid_.cell_height;
    if (track_height <= 0.0) {
      return;
    }

    const double track_x = static_cast<double>(width()) - kPadding
                           + ((kPadding - kScrollbarWidth) * 0.5);
    const double track_y = kPadding;

    QColor track_color = foreground;
    track_color.setAlpha(48);
    QColor thumb_color = foreground;
    thumb_color.setAlpha(144);

    painter.fillRect(QRectF(track_x, track_y, kScrollbarWidth, track_height), track_color);

    double thumb_height = std::max(
      kScrollbarMinThumbHeight,
      track_height * static_cast<double>(scrollbar.len) / static_cast<double>(scrollbar.total)
    );
    thumb_height = std::min(thumb_height, track_height);

    const std::uint64_t offset_range_rows = scrollbar.total - scrollbar.len;
    const double thumb_travel = std::max(0.0, track_height - thumb_height);
    double thumb_y = track_y;
    if (offset_range_rows > 0 && thumb_travel > 0.0) {
      const double offset_fraction = static_cast<double>(scrollbar.offset)
                                     / static_cast<double>(offset_range_rows);
      thumb_y += std::clamp(offset_fraction, 0.0, 1.0) * thumb_travel;
    }

    painter.fillRect(QRectF(track_x, thumb_y, kScrollbarWidth, thumb_height), thumb_color);
  }

  void render_cell(
    QPainter& painter,
    double x,
    double y,
    std::uint16_t row_index,
    std::uint16_t column_index,
    const QColor& background,
    const QColor& foreground,
    const std::optional<libghostty_cpp::CursorViewport>& cursor,
    const QColor& cursor_color
  ) {
    const std::u32string graphemes = cell_iterator_.graphemes();
    const libghostty_cpp::Style style = cell_iterator_.style();
    const std::optional<libghostty_cpp::RgbColor> fg_rgb =
      cell_iterator_.resolved_fg_color();
    const std::optional<libghostty_cpp::RgbColor> bg_rgb =
      cell_iterator_.resolved_bg_color();

    QColor cell_fg = fg_rgb.has_value() ? to_qcolor(*fg_rgb) : foreground;
    QColor cell_bg = bg_rgb.has_value() ? to_qcolor(*bg_rgb) : background;
    bool has_background = bg_rgb.has_value();

    if (style.inverse) {
      std::swap(cell_fg, cell_bg);
      has_background = true;
    }

    if (is_selected_cell(row_index, column_index)) {
      QColor selection_color = foreground;
      selection_color.setAlpha(96);
      cell_bg = selection_color;
      has_background = true;
    }

    const bool is_cursor_cell = cursor.has_value()
                                && cursor->x == column_index
                                && cursor->y == row_index;
    if (is_cursor_cell) {
      cell_bg = cursor_color;
      cell_fg = background;
      has_background = true;
    }

    if (has_background) {
      painter.fillRect(QRectF(x, y, grid_.cell_width, grid_.cell_height), cell_bg);
    }

    if (style.invisible || graphemes.empty()) {
      return;
    }

    QFont cell_font = terminal_font_;
    cell_font.setBold(style.bold);
    cell_font.setItalic(style.italic);
    painter.setFont(cell_font);
    painter.setPen(cell_fg);

    const QString text = QString::fromUcs4(
      graphemes.data(),
      static_cast<qsizetype>(graphemes.size())
    );
    painter.drawText(QPointF(x, y + grid_.baseline), text);
  }

  [[nodiscard]] GridMetrics measure_grid() const {
    const QFontMetricsF font_metrics(terminal_font_);
    const double device_pixel_ratio = devicePixelRatioF();
    const double cell_width = std::max(
      1.0,
      font_metrics.horizontalAdvance(QStringLiteral("M")) + kCellGap
    );
    const double cell_height = std::max(1.0, font_metrics.lineSpacing() + kRowGap);
    const double width_available = std::max(1.0, width() - 2.0 * kPadding);
    const double height_available = std::max(1.0, height() - 2.0 * kPadding);
    const long cols = static_cast<long>(std::floor(width_available / cell_width));
    const long rows = static_cast<long>(std::floor(height_available / cell_height));
    const double baseline = ((cell_height - font_metrics.height()) * 0.5)
                            + font_metrics.ascent();

    GridMetrics metrics;
    metrics.cell_width = cell_width;
    metrics.cell_height = cell_height;
    metrics.baseline = baseline;
    metrics.cols = clamp_to_u16(cols);
    metrics.rows = clamp_to_u16(rows);
    metrics.cell_width_px = static_cast<std::uint32_t>(std::max(
      1.0,
      std::round(cell_width * device_pixel_ratio)
    ));
    metrics.cell_height_px = static_cast<std::uint32_t>(std::max(
      1.0,
      std::round(cell_height * device_pixel_ratio)
    ));
    metrics.window_width_px = clamp_to_u16(static_cast<long>(std::lround(
      static_cast<double>(width()) * device_pixel_ratio
    )));
    metrics.window_height_px = clamp_to_u16(static_cast<long>(std::lround(
      static_cast<double>(height()) * device_pixel_ratio
    )));
    return metrics;
  }

  [[nodiscard]] winsize make_winsize() const {
    winsize size = {};
    size.ws_col = grid_.cols;
    size.ws_row = grid_.rows;
    size.ws_xpixel = grid_.window_width_px;
    size.ws_ypixel = grid_.window_height_px;
    return size;
  }

  [[nodiscard]] const char* resolve_shell() const {
    const char* shell = std::getenv("SHELL");
    if (shell != nullptr && shell[0] != '\0') {
      return shell;
    }

    const passwd* user = ::getpwuid(::getuid());
    if (user != nullptr && user->pw_shell != nullptr && user->pw_shell[0] != '\0') {
      return user->pw_shell;
    }

    return "/bin/sh";
  }

  void spawn_shell() {
    const winsize size = make_winsize();
    int master_fd = -1;
    const pid_t pid = ::forkpty(&master_fd, nullptr, nullptr, &size);
    if (pid < 0) {
      throw std::system_error(errno, std::generic_category(), "forkpty failed");
    }

    if (pid == 0) {
      const char* shell = resolve_shell();
      const char* arg0 = std::strrchr(shell, '/');
      arg0 = arg0 == nullptr ? shell : arg0 + 1;
      ::setenv("TERM", "xterm-256color", 1);
      ::execl(shell, arg0, static_cast<char*>(nullptr));
      ::execl("/bin/sh", "sh", static_cast<char*>(nullptr));
      _exit(127);
    }

    const int flags = ::fcntl(master_fd, F_GETFL, 0);
    if (flags < 0 || ::fcntl(master_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
      const int error = errno;
      ::close(master_fd);
      ::kill(pid, SIGHUP);
      (void)::waitpid(pid, nullptr, 0);
      throw std::system_error(error, std::generic_category(), "failed to set O_NONBLOCK");
    }

    child_pid_ = pid;
    pty_fd_ = master_fd;

    socket_notifier_ = std::make_unique<QSocketNotifier>(pty_fd_, QSocketNotifier::Read, this);
    connect(
      socket_notifier_.get(),
      &QSocketNotifier::activated,
      this,
      [this](QSocketDescriptor, QSocketNotifier::Type) { drain_pty(); }
    );
  }

  void close_pty() {
    if (socket_notifier_ != nullptr) {
      socket_notifier_->setEnabled(false);
      socket_notifier_.reset();
    }

    if (pty_fd_ >= 0) {
      ::close(pty_fd_);
      pty_fd_ = -1;
    }
  }

  void reap_child(bool blocking) {
    if (child_pid_ <= 0) {
      return;
    }

    if (blocking) {
      ::kill(child_pid_, SIGHUP);

      int status = 0;
      while (::waitpid(child_pid_, &status, 0) < 0) {
        if (errno == EINTR) {
          continue;
        }

        break;
      }

      child_pid_ = -1;
      return;
    }

    int status = 0;
    const pid_t waited = ::waitpid(child_pid_, &status, WNOHANG);
    if (waited == child_pid_) {
      child_pid_ = -1;
    }
  }

  void resize_pty() const {
    if (pty_fd_ < 0) {
      return;
    }

    const winsize size = make_winsize();
    if (::ioctl(pty_fd_, TIOCSWINSZ, &size) < 0) {
      std::fprintf(stderr, "ghostling-cpp: failed to resize pty: %s\n", std::strerror(errno));
    }
  }

  void write_pty(std::string_view data) const {
    if (pty_fd_ < 0 || data.empty()) {
      return;
    }

    const char* cursor = data.data();
    std::size_t remaining = data.size();
    while (remaining > 0) {
      const ssize_t written = ::write(pty_fd_, cursor, remaining);
      if (written > 0) {
        cursor += written;
        remaining -= static_cast<std::size_t>(written);
        continue;
      }

      if (written < 0 && errno == EINTR) {
        continue;
      }

      break;
    }
  }

  void write_encoded_bytes(const std::vector<std::uint8_t>& encoded) const {
    if (encoded.empty()) {
      return;
    }

    write_pty(std::string_view(
      reinterpret_cast<const char*>(encoded.data()),
      encoded.size()
    ));
  }

  void clear_encoded_input() {
    encoded_input_.clear();
  }

  [[nodiscard]] bool flush_encoded_input() const {
    if (encoded_input_.empty()) {
      return false;
    }

    write_encoded_bytes(encoded_input_);
    return true;
  }

  void drain_pty() {
    if (pty_fd_ < 0) {
      return;
    }

    bool changed = false;
    std::array<char, 4096> buffer = {};
    for (;;) {
      const ssize_t read_len = ::read(pty_fd_, buffer.data(), buffer.size());
      if (read_len > 0) {
        title_dirty_ = false;
        terminal_.vt_write(std::string_view(buffer.data(), static_cast<std::size_t>(read_len)));
        if (title_dirty_) {
          sync_window_title();
        }
        changed = true;
        continue;
      }

      if (read_len == 0 || (read_len < 0 && errno == EIO)) {
        close_pty();
        reap_child(false);
        QCoreApplication::quit();
        return;
      }

      if (read_len < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        break;
      }

      if (read_len < 0 && errno == EINTR) {
        continue;
      }

      std::fprintf(stderr, "ghostling-cpp: failed to read pty: %s\n", std::strerror(errno));
      break;
    }

    if (changed) {
      clear_selection();
      clear_primary_selection();
      update();
    }
  }

  [[nodiscard]] libghostty_cpp::mouse::EncoderSize make_mouse_encoder_size() const {
    const std::uint32_t padding_px = static_cast<std::uint32_t>(std::lround(
      kPadding * devicePixelRatioF()
    ));
    return libghostty_cpp::mouse::EncoderSize{
      grid_.window_width_px,
      grid_.window_height_px,
      grid_.cell_width_px,
      grid_.cell_height_px,
      padding_px,
      padding_px,
      padding_px,
      padding_px,
    };
  }

  [[nodiscard]] bool is_bracketed_paste_enabled() const {
    return is_mode_enabled(libghostty_cpp::Mode::BracketedPaste);
  }

  void report_focus(bool focused) const {
    if (!has_active_pty() || !is_mode_enabled(libghostty_cpp::Mode::FocusEvent)) {
      return;
    }

    write_pty(focused ? kFocusGainedSequence : kFocusLostSequence);
  }

  void append_encoded_input(std::string_view data) {
    encoded_input_.insert(encoded_input_.end(), data.begin(), data.end());
  }

  [[nodiscard]] bool handle_paste() {
    if (!has_active_pty()) {
      return false;
    }

    clear_selection_if_any();

    const QClipboard* clipboard = QApplication::clipboard();
    if (clipboard == nullptr) {
      return false;
    }

    const QString text = clipboard->text(QClipboard::Clipboard);
    if (text.isEmpty()) {
      return false;
    }

    const QByteArray utf8 = text.toUtf8();
    if (utf8.isEmpty()) {
      return false;
    }

    const bool bracketed_paste = is_bracketed_paste_enabled();

    clear_encoded_input();
    if (bracketed_paste) {
      append_encoded_input(kBracketedPasteStartSequence);
    }

    encoded_input_.insert(encoded_input_.end(), utf8.begin(), utf8.end());

    if (bracketed_paste) {
      append_encoded_input(kBracketedPasteEndSequence);
    }

    return flush_encoded_input();
  }

  void configure_mouse_input(
    const QPointF& position,
    Qt::KeyboardModifiers modifiers,
    Qt::MouseButtons buttons
  ) {
    mouse_event_
      .set_mods(to_ghostty_mods(modifiers))
      .set_position(libghostty_cpp::mouse::Position{
        static_cast<float>(position.x()),
        static_cast<float>(position.y()),
      });

    mouse_encoder_
      .set_options_from_terminal(terminal_)
      .set_size(make_mouse_encoder_size())
      .set_any_button_pressed(buttons != Qt::NoButton)
      .set_track_last_cell(true);
  }

  [[nodiscard]] bool handle_mouse_button(
    const QMouseEvent& event,
    libghostty_cpp::mouse::Action action
  ) {
    if (!has_active_pty()) {
      return false;
    }

    const std::optional<libghostty_cpp::mouse::Button> button =
      map_qt_mouse_button(event.button());
    if (!button.has_value()) {
      return false;
    }

    if (!terminal_.is_mouse_tracking() && *button == libghostty_cpp::mouse::Button::Left) {
      if (action == libghostty_cpp::mouse::Action::Press) {
        return begin_selection(event.position());
      }

      if (action == libghostty_cpp::mouse::Action::Release) {
        return finish_selection(event.position());
      }
    }

    clear_selection_if_any();

    configure_mouse_input(event.position(), event.modifiers(), event.buttons());
    mouse_event_.set_action(action).set_button(button);

    clear_encoded_input();
    mouse_encoder_.encode_to(encoded_input_, mouse_event_);
    return flush_encoded_input();
  }

  [[nodiscard]] bool handle_mouse_move(const QMouseEvent& event) {
    if (!has_active_pty()) {
      return false;
    }

    if (selection_in_progress_) {
      return update_selection(event.position(), true);
    }

    if (!terminal_.is_mouse_tracking()) {
      return false;
    }

    configure_mouse_input(event.position(), event.modifiers(), event.buttons());
    mouse_event_
      .set_action(libghostty_cpp::mouse::Action::Motion)
      .set_button(first_pressed_mouse_button(event.buttons()));

    clear_encoded_input();
    mouse_encoder_.encode_to(encoded_input_, mouse_event_);
    return flush_encoded_input();
  }

  [[nodiscard]] bool handle_wheel(const QWheelEvent& event) {
    if (!has_active_pty()) {
      return false;
    }

    const double delta_steps = wheel_steps(event);
    if (std::abs(delta_steps) < 1e-6) {
      return false;
    }

    const libghostty_cpp::mouse::Button scroll_button = delta_steps > 0.0
                                                         ? libghostty_cpp::mouse::Button::Four
                                                         : libghostty_cpp::mouse::Button::Five;

    if (terminal_.is_mouse_tracking()) {
      configure_mouse_input(event.position(), event.modifiers(), event.buttons());

      clear_encoded_input();
      mouse_event_.set_button(scroll_button).set_action(libghostty_cpp::mouse::Action::Press);
      mouse_encoder_.encode_to(encoded_input_, mouse_event_);

      mouse_event_.set_action(libghostty_cpp::mouse::Action::Release);
      mouse_encoder_.encode_to(encoded_input_, mouse_event_);

      return flush_encoded_input();
    }

    clear_selection_if_any();

    std::ptrdiff_t scroll_delta =
      static_cast<std::ptrdiff_t>(std::lround(delta_steps * kWheelScrollScale));
    if (scroll_delta == 0) {
      scroll_delta = delta_steps > 0.0 ? -1 : 1;
    }

    terminal_.scroll_viewport(libghostty_cpp::ScrollViewport::delta(scroll_delta));
    update();
    return true;
  }

  [[nodiscard]] bool handle_key_event(
    const QKeyEvent& event,
    libghostty_cpp::key::Action action,
    bool allow_text_fallback
  ) {
    using libghostty_cpp::key::Mods;

    if (!has_active_pty()) {
      return false;
    }

    const std::optional<KeyMapping> mapping = map_qt_key(event.key());
    const QByteArray utf8 = action == libghostty_cpp::key::Action::Release
                              ? QByteArray{}
                              : encoder_utf8_text(event);

    if (mapping.has_value()) {
      if (action != libghostty_cpp::key::Action::Release) {
        clear_selection_if_any();
      }

      Mods consumed_mods = Mods::None;
      if (!utf8.isEmpty() && event.modifiers().testFlag(Qt::ShiftModifier)
          && mapping->unshifted_codepoint != U'\0') {
        consumed_mods |= Mods::Shift;
      }

      key_event_
        .set_action(action)
        .set_key(mapping->key)
        .set_mods(to_ghostty_mods(event.modifiers()))
        .set_consumed_mods(consumed_mods)
        .set_composing(false)
        .set_unshifted_codepoint(mapping->unshifted_codepoint);

      if (utf8.isEmpty()) {
        key_event_.clear_utf8();
      } else {
        key_event_.set_utf8(std::string_view(utf8.constData(), utf8.size()));
      }

      clear_encoded_input();
      key_encoder_.set_options_from_terminal(terminal_).encode_to(encoded_input_, key_event_);
      static_cast<void>(flush_encoded_input());
      return true;
    }

    if (allow_text_fallback && !has_text_modifiers(event.modifiers())
        && is_printable_text(event.text())) {
      clear_selection_if_any();
      const QByteArray text = event.text().toUtf8();
      write_pty(std::string_view(text.constData(), text.size()));
      return true;
    }

    return false;
  }

  [[nodiscard]] bool handle_key_press(const QKeyEvent& event) {
    using libghostty_cpp::key::Action;

    if (is_copy_shortcut(event)) {
      if (!event.isAutoRepeat() && copy_selection_to_clipboard()) {
        suppressed_key_release_ = event.key();
        return true;
      }

      return false;
    }

    if (is_paste_shortcut(event)) {
      if (!event.isAutoRepeat()) {
        suppressed_key_release_ = event.key();
        static_cast<void>(handle_paste());
      }

      return true;
    }

    return handle_key_event(
      event,
      event.isAutoRepeat() ? Action::Repeat : Action::Press,
      true
    );
  }

  [[nodiscard]] bool handle_key_release(const QKeyEvent& event) {
    if (suppressed_key_release_.has_value() && *suppressed_key_release_ == event.key()) {
      if (!event.isAutoRepeat()) {
        suppressed_key_release_.reset();
      }

      return true;
    }

    if (event.isAutoRepeat()) {
      return true;
    }

    return handle_key_event(event, libghostty_cpp::key::Action::Release, false);
  }

  QFont terminal_font_;
  GridMetrics grid_;
  QString paint_error_frame_;
  libghostty_cpp::Terminal terminal_;
  libghostty_cpp::RenderState render_state_;
  libghostty_cpp::RowIterator row_iterator_;
  libghostty_cpp::CellIterator cell_iterator_;
  libghostty_cpp::key::Encoder key_encoder_;
  libghostty_cpp::key::Event key_event_;
  libghostty_cpp::mouse::Encoder mouse_encoder_;
  libghostty_cpp::mouse::Event mouse_event_;
  std::vector<std::uint8_t> encoded_input_;
  bool title_dirty_ = false;
  std::optional<Selection> selection_;
  bool selection_in_progress_ = false;
  std::optional<int> suppressed_key_release_;
  int pty_fd_ = -1;
  pid_t child_pid_ = -1;
  std::unique_ptr<QSocketNotifier> socket_notifier_;
};

} // namespace

int main(int argc, char** argv) {
  try {
    QApplication app(argc, argv);
    TerminalWidget terminal_widget;
    terminal_widget.show();
    return app.exec();
  } catch (const std::exception& error) {
    std::fprintf(stderr, "ghostling-cpp: %s\n", error.what());
    return 1;
  }
}
