#include "libghostty_cpp/render.hpp"
#include "libghostty_cpp/vt.hpp"

#include <cassert>
#include <variant>

int main() {
  using libghostty_cpp::CellContentTag;
  using libghostty_cpp::CellIterator;
  using libghostty_cpp::CellWide;
  using libghostty_cpp::Dirty;
  using libghostty_cpp::Error;
  using libghostty_cpp::ErrorCode;
  using libghostty_cpp::PaletteColor;
  using libghostty_cpp::RenderState;
  using libghostty_cpp::RowIterator;
  using libghostty_cpp::Terminal;
  using libghostty_cpp::TerminalOptions;

  Terminal terminal(TerminalOptions{80, 24, 128});
  assert(terminal.cols() == 80);
  assert(terminal.rows() == 24);

  terminal.vt_write("hi\r\n\x1b[31m!\x1b[0m");
  assert(terminal.cursor_x() == 1);
  assert(terminal.cursor_y() == 1);

  RenderState render_state;
  render_state.update(terminal);

  assert(render_state.cols() == 80);
  assert(render_state.rows() == 24);

  const auto dirty = render_state.dirty();
  assert(dirty == Dirty::Partial || dirty == Dirty::Full);

  const auto colors = render_state.colors();
  static_cast<void>(colors.background);
  static_cast<void>(colors.foreground);

  const auto cursor_viewport = render_state.cursor_viewport();
  assert(cursor_viewport.has_value());
  assert(cursor_viewport->x == 1);
  assert(cursor_viewport->y == 1);

  RowIterator row_iterator;
  row_iterator.bind(render_state);
  assert(row_iterator.next());

  const auto first_row = row_iterator.row();
  assert(!first_row.is_wrapped());
  assert(!first_row.is_wrap_continuation());

  CellIterator cell_iterator;
  cell_iterator.bind(row_iterator);
  assert(cell_iterator.next());

  const auto first_cell = cell_iterator.cell();
  assert(first_cell.codepoint() == static_cast<std::uint32_t>('h'));
  assert(first_cell.content_tag() == CellContentTag::Codepoint);
  assert(first_cell.wide() == CellWide::Narrow);
  assert(first_cell.has_text());
  assert(!first_cell.has_hyperlink());

  const auto first_graphemes = cell_iterator.graphemes();
  assert(first_graphemes == U"h");

  const auto first_style = cell_iterator.style();
  assert(first_style.is_default());
  assert(!cell_iterator.resolved_fg_color().has_value());
  assert(!cell_iterator.resolved_bg_color().has_value());

  cell_iterator.select(1);
  const auto second_cell = cell_iterator.cell();
  assert(second_cell.codepoint() == static_cast<std::uint32_t>('i'));

  assert(row_iterator.next());
  const auto second_row = row_iterator.row();
  assert(!second_row.is_wrap_continuation());

  bool stale_cell_iterator_threw = false;
  try {
    static_cast<void>(cell_iterator.cell());
  } catch (const Error &error) {
    stale_cell_iterator_threw = error.code() == ErrorCode::InvalidState;
  }
  assert(stale_cell_iterator_threw);

  cell_iterator.bind(row_iterator);
  cell_iterator.select(0);
  const auto styled_cell = cell_iterator.cell();
  assert(styled_cell.codepoint() == static_cast<std::uint32_t>('!'));

  const auto styled_style = cell_iterator.style();
  assert(std::holds_alternative<PaletteColor>(styled_style.fg_color));
  assert(std::get<PaletteColor>(styled_style.fg_color).index == 1);

  const auto styled_fg = cell_iterator.resolved_fg_color();
  assert(styled_fg.has_value());
  assert(styled_fg->r != 0 || styled_fg->g != 0 || styled_fg->b != 0);

  row_iterator.set_dirty(false);
  assert(!row_iterator.dirty());

  render_state.set_dirty(Dirty::Clean);
  assert(render_state.dirty() == Dirty::Clean);

  return 0;
}
