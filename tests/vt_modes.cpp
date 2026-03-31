#include "libghostty_cpp/vt.hpp"

#include <cassert>

int main() {
  using libghostty_cpp::Mode;
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

  return 0;
}
