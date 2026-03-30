#include "libghostty_cpp/key.hpp"
#include "libghostty_cpp/render.hpp"
#include "libghostty_cpp/vt.hpp"

#include <QApplication>
#include <QByteArray>
#include <QColor>
#include <QCoreApplication>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QKeyEvent>
#include <QPainter>
#include <QPalette>
#include <QString>
#include <QStringList>
#include <QSocketNotifier>
#include <QWidget>

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
constexpr int kInitialWindowWidth = 1024;
constexpr int kInitialWindowHeight = 720;

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
    resize(kInitialWindowWidth, kInitialWindowHeight);
    setFont(terminal_font_);

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
      paint_error_frame_ = QString::fromUtf8(error.what());
      update();
      event->accept();
      return;
    }

    QWidget::keyPressEvent(event);
  }

private:
  void configure_terminal() {
    terminal_
      .on_pty_write([this](const libghostty_cpp::Terminal&, std::string_view data) {
        write_pty(data);
      })
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

    render_state_.set_dirty(libghostty_cpp::Dirty::Clean);
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

  void drain_pty() {
    if (pty_fd_ < 0) {
      return;
    }

    bool changed = false;
    std::array<char, 4096> buffer = {};
    for (;;) {
      const ssize_t read_len = ::read(pty_fd_, buffer.data(), buffer.size());
      if (read_len > 0) {
        terminal_.vt_write(std::string_view(buffer.data(), static_cast<std::size_t>(read_len)));
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
      update();
    }
  }

  [[nodiscard]] bool handle_key_press(const QKeyEvent& event) {
    if (pty_fd_ < 0) {
      return false;
    }

    const std::optional<KeyMapping> mapping = map_qt_key(event.key());
    const QByteArray utf8 = encoder_utf8_text(event);

    if (mapping.has_value()) {
      using libghostty_cpp::key::Action;
      using libghostty_cpp::key::Mods;

      Mods consumed_mods = Mods::None;
      if (!utf8.isEmpty() && event.modifiers().testFlag(Qt::ShiftModifier)
          && mapping->unshifted_codepoint != U'\0') {
        consumed_mods |= Mods::Shift;
      }

      key_event_
        .set_action(event.isAutoRepeat() ? Action::Repeat : Action::Press)
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

      std::vector<std::uint8_t> encoded;
      encoded.reserve(16);
      key_encoder_.set_options_from_terminal(terminal_).encode_to(encoded, key_event_);
      if (!encoded.empty()) {
        write_pty(std::string_view(
          reinterpret_cast<const char*>(encoded.data()),
          encoded.size()
        ));
      }
      return true;
    }

    if (!has_text_modifiers(event.modifiers()) && is_printable_text(event.text())) {
      const QByteArray text = event.text().toUtf8();
      write_pty(std::string_view(text.constData(), text.size()));
      return true;
    }

    return false;
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
