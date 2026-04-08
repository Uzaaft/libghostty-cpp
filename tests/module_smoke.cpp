#include "libghostty_cpp/build_info.hpp"
#include "libghostty_cpp/fmt.hpp"
#include "libghostty_cpp/focus.hpp"
#include "libghostty_cpp/osc.hpp"
#include "libghostty_cpp/paste.hpp"
#include "libghostty_cpp/sgr.hpp"
#include "libghostty_cpp/terminal.hpp"

#include <cassert>
#include <string>
#include <variant>
#include <vector>

int main() {
  using libghostty_cpp::PaletteColor;
  using libghostty_cpp::Terminal;
  using libghostty_cpp::TerminalOptions;

  const std::vector<std::uint8_t> focus_gained =
    libghostty_cpp::focus::encode(libghostty_cpp::focus::Event::Gained);
  assert(focus_gained == std::vector<std::uint8_t>({0x1B, '[', 'I'}));

  assert(libghostty_cpp::paste::is_safe("hello"));
  assert(!libghostty_cpp::paste::is_safe("hello\nworld"));

  const std::vector<std::uint8_t> bracketed_paste =
    libghostty_cpp::paste::encode("hello\nworld", true);
  const std::string bracketed_text(bracketed_paste.begin(), bracketed_paste.end());
  assert(bracketed_text.rfind("\x1B[200~", 0) == 0);
  assert(bracketed_text.find("hello") != std::string::npos);
  assert(bracketed_text.size() >= 6);
  assert(bracketed_text.substr(bracketed_text.size() - 6) == "\x1B[201~");

  const std::vector<std::uint8_t> plain_paste =
    libghostty_cpp::paste::encode("hel\x1Blo\nworld", false);
  const std::string plain_text(plain_paste.begin(), plain_paste.end());
  assert(plain_text == "hel lo\rworld");

  assert(!libghostty_cpp::build_info::version_string().empty());

  auto png_decoder = libghostty_cpp::sys::install_png_decoder(
    [](libghostty_cpp::ByteView png_data)
      -> std::optional<libghostty_cpp::sys::DecodedImage> {
      assert(png_data.empty());
      return std::nullopt;
    }
  );
  assert(png_decoder.active());
  png_decoder.reset();
  assert(!png_decoder.active());

  libghostty_cpp::osc::Parser osc_parser;
  for (const char byte : std::string("2;Ghostling")) {
    osc_parser.next_byte(static_cast<std::uint8_t>(byte));
  }
  const libghostty_cpp::osc::Command osc_command = osc_parser.end(0x07);
  assert(osc_command.kind() == libghostty_cpp::osc::CommandKind::ChangeWindowTitle);
  assert(osc_command.title() == "Ghostling");

  libghostty_cpp::sgr::Parser sgr_parser;
  sgr_parser.set_params({1, 31});
  const auto bold = sgr_parser.next();
  assert(bold.has_value());
  assert(bold->tag == libghostty_cpp::sgr::AttributeTag::Bold);

  const auto fg = sgr_parser.next();
  assert(fg.has_value());
  assert(fg->tag == libghostty_cpp::sgr::AttributeTag::Fg8);
  assert(std::holds_alternative<PaletteColor>(fg->value));
  assert(std::get<PaletteColor>(fg->value).index == 1);
  assert(!sgr_parser.next().has_value());

  Terminal terminal(TerminalOptions{40, 5, 64});
  terminal.vt_write("Hello, formatter");

  libghostty_cpp::fmt::Formatter formatter(
    terminal,
    libghostty_cpp::fmt::FormatterOptions{libghostty_cpp::fmt::Format::Plain, false, false}
  );
  const std::string rendered = formatter.format_string();
  assert(rendered.find("Hello, formatter") != std::string::npos);

  return 0;
}
