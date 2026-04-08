#include "libghostty_cpp/vt.hpp"

#include <cassert>
#include <variant>

int main() {
  using libghostty_cpp::Error;
  using libghostty_cpp::ErrorCode;
  using libghostty_cpp::GridCellWide;
  using libghostty_cpp::Point;
  using libghostty_cpp::PointCoordinate;
  using libghostty_cpp::ScrollViewport;
  using libghostty_cpp::Terminal;
  using libghostty_cpp::TerminalOptions;
  using libghostty_cpp::screen::CellContentTag;
  using libghostty_cpp::screen::CellWide;

  Terminal wrapped_terminal{TerminalOptions{4, 2, 64}};
  wrapped_terminal.vt_write("ABCDE");

  const libghostty_cpp::GridRef wrapped_head =
    wrapped_terminal.grid_ref(Point::viewport(PointCoordinate{0, 0}));
  assert(wrapped_head.row_is_wrapped());
  assert(wrapped_head.cell_has_text());
  assert(wrapped_head.cell_wide() == GridCellWide::Narrow);
  assert(wrapped_head.graphemes() == U"A");
  assert(wrapped_head.row().is_wrapped());
  assert(wrapped_head.cell().content_tag() == CellContentTag::Codepoint);
  assert(wrapped_head.cell().wide() == CellWide::Narrow);
  assert(wrapped_head.style().is_default());

  const libghostty_cpp::GridRef wrapped_tail =
    wrapped_terminal.grid_ref(Point::viewport(PointCoordinate{0, 1}));
  assert(!wrapped_tail.row_is_wrapped());
  assert(wrapped_tail.cell_has_text());
  assert(wrapped_tail.graphemes() == U"E");

  Terminal wide_terminal{TerminalOptions{4, 2, 64}};
  wide_terminal.vt_write(u8"中");

  const libghostty_cpp::GridRef wide_head =
    wide_terminal.grid_ref(Point::viewport(PointCoordinate{0, 0}));
  assert(wide_head.cell_has_text());
  assert(wide_head.cell_wide() == GridCellWide::Wide);
  assert(wide_head.graphemes() == U"中");

  const libghostty_cpp::GridRef wide_spacer =
    wide_terminal.grid_ref(Point::viewport(PointCoordinate{1, 0}));
  assert(!wide_spacer.cell_has_text());
  assert(wide_spacer.cell_wide() == GridCellWide::SpacerTail);
  assert(wide_spacer.graphemes().empty());
  assert(wide_spacer.cell().wide() == CellWide::SpacerTail);

  Terminal styled_terminal{TerminalOptions{4, 2, 64}};
  styled_terminal.vt_write("\x1B[31m!\x1B[0m");

  const libghostty_cpp::GridRef styled =
    styled_terminal.grid_ref(Point::viewport(PointCoordinate{0, 0}));
  assert(styled.cell().has_styling());
  assert(std::holds_alternative<libghostty_cpp::PaletteColor>(styled.style().fg_color));
  assert(std::get<libghostty_cpp::PaletteColor>(styled.style().fg_color).index == 1);

  Terminal scroll_terminal{TerminalOptions{4, 2, 64}};
  scroll_terminal.vt_write("1\r\n2\r\n3");

  const libghostty_cpp::GridRef viewport_bottom =
    scroll_terminal.grid_ref(Point::viewport(PointCoordinate{0, 0}));
  assert(viewport_bottom.graphemes() == U"2");

  scroll_terminal.scroll_viewport(ScrollViewport::top());

  const libghostty_cpp::GridRef viewport_top =
    scroll_terminal.grid_ref(Point::viewport(PointCoordinate{0, 0}));
  assert(viewport_top.graphemes() == U"1");

  const libghostty_cpp::GridRef screen_top =
    scroll_terminal.grid_ref(Point::screen(PointCoordinate{0, 0}));
  assert(screen_top.graphemes() == U"1");

  bool out_of_bounds_threw = false;
  try {
    static_cast<void>(scroll_terminal.grid_ref(Point::viewport(PointCoordinate{4, 0})));
  } catch (const Error& error) {
    out_of_bounds_threw = error.code() == ErrorCode::InvalidValue;
  }
  assert(out_of_bounds_threw);

  return 0;
}
