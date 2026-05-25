#include "libghostty_cpp/build_info.hpp"

#include <iostream>
#include <string_view>

namespace {

const char* enabled(bool value) noexcept {
  return value ? "enabled" : "disabled";
}

std::string_view optional_text(std::string_view value) noexcept {
  if (value.empty()) {
    return "(none)";
  }

  return value;
}

} // namespace

int main() {
  namespace build_info = libghostty_cpp::build_info;

  std::cout << "SIMD: " << enabled(build_info::supports_simd()) << '\n';
  std::cout << "Kitty graphics: " << enabled(build_info::supports_kitty_graphics()) << '\n';
  std::cout << "Tmux control mode: " << enabled(build_info::supports_tmux_control_mode()) << '\n';

  std::cout << "Version: " << build_info::version_string() << '\n';
  std::cout << "Version major: " << build_info::major_version() << '\n';
  std::cout << "Version minor: " << build_info::minor_version() << '\n';
  std::cout << "Version patch: " << build_info::patch_version() << '\n';
  std::cout << "Version pre  : " << optional_text(build_info::version_pre()) << '\n';
  std::cout << "Version build: " << optional_text(build_info::build_version()) << '\n';

  return 0;
}
