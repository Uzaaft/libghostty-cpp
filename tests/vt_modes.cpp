#include "libghostty_cpp/vt.hpp"

#include <cassert>
#include <cstdint>

int main() {
  using libghostty_cpp::ActiveScreen;
  using libghostty_cpp::Mode;
  using libghostty_cpp::ScrollViewport;
  using libghostty_cpp::Terminal;
  using libghostty_cpp::TerminalOptions;

  Terminal terminal{TerminalOptions{80, 24, 64}};

  assert(!terminal.is_mode_enabled(Mode::Insert));
  terminal.vt_write("\x1B[4h");
  assert(terminal.is_mode_enabled(Mode::Insert));
  terminal.vt_write("\x1B[4l");
  assert(!terminal.is_mode_enabled(Mode::Insert));

  assert(!terminal.is_mode_enabled(Mode::FocusEvent));
  terminal.vt_write("\x1B[?1004h");
  assert(terminal.is_mode_enabled(Mode::FocusEvent));
  terminal.vt_write("\x1B[?1004l");
  assert(!terminal.is_mode_enabled(Mode::FocusEvent));

  assert(!terminal.is_mode_enabled(Mode::BracketedPaste));
  terminal.vt_write("\x1B[?2004h");
  assert(terminal.is_mode_enabled(Mode::BracketedPaste));
  terminal.vt_write("\x1B[?2004l");
  assert(!terminal.is_mode_enabled(Mode::BracketedPaste));

  assert(!terminal.is_mouse_tracking());
  terminal.vt_write("\x1B[?1000h");
  assert(terminal.is_mouse_tracking());
  terminal.vt_write("\x1B[?1000l");
  assert(!terminal.is_mouse_tracking());

  Terminal viewport_terminal{TerminalOptions{8, 3, 64}};
  viewport_terminal.vt_write("1\r\n2\r\n3\r\n4\r\n5\r\n");

  const libghostty_cpp::Scrollbar bottom_scrollbar = viewport_terminal.scrollbar();
  assert(viewport_terminal.active_screen() == ActiveScreen::Primary);
  assert(viewport_terminal.scrollback_rows() > 0);
  assert(bottom_scrollbar.total == static_cast<std::uint64_t>(viewport_terminal.total_rows()));
  assert(
    bottom_scrollbar.total - bottom_scrollbar.len
    == static_cast<std::uint64_t>(viewport_terminal.scrollback_rows())
  );
  assert(bottom_scrollbar.offset + bottom_scrollbar.len == bottom_scrollbar.total);

  viewport_terminal.scroll_viewport(ScrollViewport::top());
  const libghostty_cpp::Scrollbar top_scrollbar = viewport_terminal.scrollbar();
  assert(top_scrollbar.offset == 0);
  assert(top_scrollbar.len == bottom_scrollbar.len);

  viewport_terminal.scroll_viewport(ScrollViewport::bottom());
  const libghostty_cpp::Scrollbar restored_scrollbar = viewport_terminal.scrollbar();
  assert(restored_scrollbar.offset + restored_scrollbar.len == restored_scrollbar.total);

  viewport_terminal.vt_write("\x1B[?1049h");
  assert(viewport_terminal.active_screen() == ActiveScreen::Alternate);
  viewport_terminal.vt_write("\x1B[?1049l");
  assert(viewport_terminal.active_screen() == ActiveScreen::Primary);

  return 0;
}
