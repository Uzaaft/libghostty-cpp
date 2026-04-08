#include "libghostty_cpp/build_info.hpp"

#include <ghostty/vt.h>

#include "ghostty_result.hpp"

namespace libghostty_cpp::build_info {

namespace {

template <typename T>
T get(GhosttyBuildInfo tag) {
  T value{};
  detail::throw_if_ghostty_error(ghostty_build_info(tag, &value));
  return value;
}

std::string_view translate_string(GhosttyString value) {
  if (value.ptr == nullptr) {
    return {};
  }

  return std::string_view(reinterpret_cast<const char*>(value.ptr), value.len);
}

} // namespace

bool supports_simd() {
  return get<bool>(GHOSTTY_BUILD_INFO_SIMD);
}

bool supports_kitty_graphics() {
  return get<bool>(GHOSTTY_BUILD_INFO_KITTY_GRAPHICS);
}

bool supports_tmux_control_mode() {
  return get<bool>(GHOSTTY_BUILD_INFO_TMUX_CONTROL_MODE);
}

OptimizeMode optimize_mode() {
  const GhosttyOptimizeMode mode = get<GhosttyOptimizeMode>(GHOSTTY_BUILD_INFO_OPTIMIZE);
  switch (mode) {
    case GHOSTTY_OPTIMIZE_DEBUG:
      return OptimizeMode::Debug;
    case GHOSTTY_OPTIMIZE_RELEASE_SAFE:
      return OptimizeMode::ReleaseSafe;
    case GHOSTTY_OPTIMIZE_RELEASE_SMALL:
      return OptimizeMode::ReleaseSmall;
    case GHOSTTY_OPTIMIZE_RELEASE_FAST:
      return OptimizeMode::ReleaseFast;
  }

  throw Error(ErrorCode::InvalidValue);
}

std::string_view version_string() {
  return translate_string(get<GhosttyString>(GHOSTTY_BUILD_INFO_VERSION_STRING));
}

std::size_t major_version() {
  return get<std::size_t>(GHOSTTY_BUILD_INFO_VERSION_MAJOR);
}

std::size_t minor_version() {
  return get<std::size_t>(GHOSTTY_BUILD_INFO_VERSION_MINOR);
}

std::size_t patch_version() {
  return get<std::size_t>(GHOSTTY_BUILD_INFO_VERSION_PATCH);
}

std::string_view build_version() {
  return translate_string(get<GhosttyString>(GHOSTTY_BUILD_INFO_VERSION_BUILD));
}

std::string_view version_pre() {
  return translate_string(get<GhosttyString>(GHOSTTY_BUILD_INFO_VERSION_PRE));
}

} // namespace libghostty_cpp::build_info
