#pragma once

#include <cstddef>
#include <string_view>

namespace libghostty_cpp::build_info {

enum class OptimizeMode {
  Debug,
  ReleaseSafe,
  ReleaseSmall,
  ReleaseFast,
};

[[nodiscard]] bool supports_simd();
[[nodiscard]] bool supports_kitty_graphics();
[[nodiscard]] bool supports_tmux_control_mode();
[[nodiscard]] OptimizeMode optimize_mode();
[[nodiscard]] std::string_view version_string();
[[nodiscard]] std::size_t major_version();
[[nodiscard]] std::size_t minor_version();
[[nodiscard]] std::size_t patch_version();
[[nodiscard]] std::string_view build_version();
[[nodiscard]] std::string_view version_pre();

} // namespace libghostty_cpp::build_info
