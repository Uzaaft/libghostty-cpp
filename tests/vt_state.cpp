#include "libghostty_cpp/terminal.hpp"

#include <array>
#include <cassert>

int main() {
  using libghostty_cpp::Mode;
  using libghostty_cpp::RgbColor;
  using libghostty_cpp::Terminal;
  using libghostty_cpp::TerminalOptions;

  Terminal terminal(TerminalOptions{80, 24, 128});
  terminal.resize(80, 24, 9, 18);

  assert(terminal.width_px() == 720);
  assert(terminal.height_px() == 432);

  terminal.set_title("manual title").set_pwd("/tmp/libghostty-cpp");
  assert(terminal.title() == "manual title");
  assert(terminal.pwd() == "/tmp/libghostty-cpp");

  terminal.clear_title().clear_pwd();
  assert(terminal.title().empty());
  assert(terminal.pwd().empty());

  assert(!terminal.is_cursor_pending_wrap());
  assert(terminal.is_cursor_visible());
  assert(terminal.cursor_style().is_default());
  assert(terminal.kitty_keyboard_flags() == libghostty_cpp::key::KittyKeyFlags::Disabled);

  terminal.set_mode(Mode::CursorVisible, false);
  assert(!terminal.mode(Mode::CursorVisible));
  assert(!terminal.is_mode_enabled(Mode::CursorVisible));
  assert(!terminal.is_cursor_visible());

  terminal.set_mode(Mode::CursorVisible, true);
  assert(terminal.mode(Mode::CursorVisible));
  assert(terminal.is_cursor_visible());

  const RgbColor foreground{0x11, 0x22, 0x33};
  const RgbColor background{0x44, 0x55, 0x66};
  const RgbColor cursor{0x77, 0x88, 0x99};

  terminal
    .set_default_fg_color(foreground)
    .set_default_bg_color(background)
    .set_default_cursor_color(cursor);

  assert(terminal.default_fg_color().has_value());
  assert(terminal.default_bg_color().has_value());
  assert(terminal.default_cursor_color().has_value());
  assert(terminal.fg_color()->r == foreground.r);
  assert(terminal.bg_color()->g == background.g);
  assert(terminal.cursor_color()->b == cursor.b);

  terminal
    .set_default_fg_color(std::nullopt)
    .set_default_bg_color(std::nullopt)
    .set_default_cursor_color(std::nullopt);

  assert(!terminal.default_fg_color().has_value());
  assert(!terminal.default_bg_color().has_value());
  assert(!terminal.default_cursor_color().has_value());

  std::array<RgbColor, 256> palette{};
  palette[1] = RgbColor{0xAA, 0xBB, 0xCC};
  palette[255] = RgbColor{0x12, 0x34, 0x56};
  terminal.set_default_color_palette(palette);

  const auto default_palette = terminal.default_color_palette();
  const auto effective_palette = terminal.color_palette();
  assert(default_palette[1].r == 0xAA);
  assert(default_palette[1].g == 0xBB);
  assert(default_palette[255].b == 0x56);
  assert(effective_palette[1].r == default_palette[1].r);
  assert(effective_palette[255].b == default_palette[255].b);

  terminal.clear_default_color_palette();
  const auto reset_palette = terminal.default_color_palette();
  assert(reset_palette[1].r != 0xAA || reset_palette[1].g != 0xBB || reset_palette[1].b != 0xCC);

  return 0;
}
