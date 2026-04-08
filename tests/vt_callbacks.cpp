#include "libghostty_cpp/vt.hpp"

#include <cassert>
#include <stdexcept>
#include <optional>
#include <string>
#include <vector>

int main() {
  using libghostty_cpp::ColorScheme;
  using libghostty_cpp::ConformanceLevel;
  using libghostty_cpp::DeviceAttributeFeature;
  using libghostty_cpp::DeviceAttributes;
  using libghostty_cpp::DeviceType;
  using libghostty_cpp::SizeReportSize;
  using libghostty_cpp::Terminal;
  using libghostty_cpp::TerminalOptions;

  struct CallbackTracker {
    std::vector<std::string> writes;
    const Terminal* expected_terminal = nullptr;
    std::size_t pty_write_calls = 0;
    std::size_t bell_calls = 0;
    std::size_t enquiry_calls = 0;
    std::size_t size_calls = 0;
    std::size_t device_attributes_calls = 0;
    std::size_t xtversion_calls = 0;
    std::size_t title_changed_calls = 0;
    std::size_t color_scheme_calls = 0;
  } tracker;

  DeviceAttributes device_attributes;
  device_attributes.primary.conformance_level = ConformanceLevel::VT220;
  device_attributes.primary.features = {
    DeviceAttributeFeature::Columns132,
    DeviceAttributeFeature::SelectiveErase,
    DeviceAttributeFeature::AnsiColor,
  };
  device_attributes.secondary.device_type = DeviceType::VT220;
  device_attributes.secondary.firmware_version = 1;
  device_attributes.secondary.rom_cartridge = 0;
  device_attributes.tertiary.unit_id = 0xAABBCCDD;

  Terminal terminal(TerminalOptions{80, 24, 256});
  terminal
    .on_pty_write([&](const Terminal& callback_terminal, std::string_view data) {
      assert(&callback_terminal == tracker.expected_terminal);
      ++tracker.pty_write_calls;
      tracker.writes.emplace_back(data);
    })
    .on_bell([&](const Terminal& callback_terminal) {
      assert(&callback_terminal == tracker.expected_terminal);
      ++tracker.bell_calls;
    })
    .on_enquiry([&](const Terminal& callback_terminal)
                  -> std::optional<std::string> {
      assert(&callback_terminal == tracker.expected_terminal);
      ++tracker.enquiry_calls;
      return std::string("ghostling-enquiry");
    })
    .on_size([&](const Terminal& callback_terminal)
               -> std::optional<SizeReportSize> {
      assert(&callback_terminal == tracker.expected_terminal);
      assert(callback_terminal.cols() == 80);
      assert(callback_terminal.rows() == 24);
      ++tracker.size_calls;
      return SizeReportSize{24, 80, 9, 18};
    })
    .on_device_attributes([&](const Terminal& callback_terminal)
                            -> std::optional<DeviceAttributes> {
      assert(&callback_terminal == tracker.expected_terminal);
      ++tracker.device_attributes_calls;
      return device_attributes;
    })
    .on_xtversion([&](const Terminal& callback_terminal)
                     -> std::optional<std::string> {
      assert(&callback_terminal == tracker.expected_terminal);
      ++tracker.xtversion_calls;
      return std::string("libghostty-cpp test");
    })
    .on_title_changed([&](const Terminal& callback_terminal) {
      assert(&callback_terminal == tracker.expected_terminal);
      ++tracker.title_changed_calls;
    })
    .on_color_scheme([&](const Terminal& callback_terminal)
                        -> std::optional<ColorScheme> {
      assert(&callback_terminal == tracker.expected_terminal);
      ++tracker.color_scheme_calls;
      return ColorScheme::Dark;
    });

  Terminal moved_terminal(std::move(terminal));
  tracker.expected_terminal = &moved_terminal;
  assert(moved_terminal.title().empty());
  assert(moved_terminal.pwd().empty());

  moved_terminal.vt_write("\x07");
  assert(tracker.bell_calls == 1);

  moved_terminal.vt_write("\x05");
  assert(tracker.writes.back() == "ghostling-enquiry");

  moved_terminal.vt_write("\x1B[?7$p");
  assert(tracker.writes.back() == "\x1B[?7;1$y");

  moved_terminal.vt_write("\x1B[18t");
  assert(tracker.writes.back() == "\x1B[8;24;80t");

  moved_terminal.vt_write("\x1B[c");
  assert(tracker.writes.back() == "\x1B[?62;1;6;22c");

  moved_terminal.vt_write("\x1B[>c");
  assert(tracker.writes.back() == "\x1B[>1;1;0c");

  moved_terminal.vt_write("\x1B[=c");
  assert(tracker.writes.back() == "\x1BP!|AABBCCDD\x1B\\");

  moved_terminal.vt_write("\x1B[>q");
  assert(tracker.writes.back() == "\x1BP>|libghostty-cpp test\x1B\\");

  moved_terminal.vt_write("\x1B[?996n");
  assert(tracker.writes.back() == "\x1B[?997;1n");

  moved_terminal.vt_write("\x1B]2;ghostling-cpp test\x1B\\");
  assert(tracker.title_changed_calls == 1);
  assert(moved_terminal.title() == "ghostling-cpp test");

  assert(tracker.pty_write_calls == tracker.writes.size());
  assert(tracker.bell_calls == 1);
  assert(tracker.enquiry_calls == 1);
  assert(tracker.size_calls == 1);
  assert(tracker.device_attributes_calls == 3);
  assert(tracker.xtversion_calls == 1);
  assert(tracker.title_changed_calls == 1);
  assert(tracker.color_scheme_calls == 1);

  std::size_t size_throw_calls = 0;
  std::size_t color_scheme_after_throw_calls = 0;

  Terminal exception_terminal(TerminalOptions{80, 24, 0});
  exception_terminal
    .on_size([&](const Terminal&) -> std::optional<SizeReportSize> {
      ++size_throw_calls;
      throw std::runtime_error("size callback failed");
    })
    .on_color_scheme([&](const Terminal&) -> std::optional<ColorScheme> {
      ++color_scheme_after_throw_calls;
      return ColorScheme::Dark;
    });

  bool threw = false;
  try {
    exception_terminal.vt_write("\x1B[18t\x1B[?996n");
  } catch (const std::runtime_error& error) {
    threw = std::string_view(error.what()) == "size callback failed";
  }

  assert(threw);
  assert(size_throw_calls == 1);
  assert(color_scheme_after_throw_calls == 0);

  std::size_t bell_throw_calls = 0;
  std::size_t title_changed_after_throw_calls = 0;

  Terminal signal_exception_terminal(TerminalOptions{80, 24, 0});
  signal_exception_terminal
    .on_bell([&](const Terminal&) {
      ++bell_throw_calls;
      throw std::runtime_error("bell callback failed");
    })
    .on_title_changed([&](const Terminal&) {
      ++title_changed_after_throw_calls;
    });

  threw = false;
  try {
    signal_exception_terminal.vt_write("\x07\x1B]2;ignored\x1B\\");
  } catch (const std::runtime_error& error) {
    threw = std::string_view(error.what()) == "bell callback failed";
  }

  assert(threw);
  assert(bell_throw_calls == 1);
  assert(title_changed_after_throw_calls == 0);

  return 0;
}
