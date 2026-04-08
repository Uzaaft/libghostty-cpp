#include "libghostty_cpp/sys.hpp"

#include <ghostty/vt.h>

#include "ghostty_result.hpp"

#include <cstring>
#include <utility>

namespace libghostty_cpp::sys {

namespace {

DecodePngCallback g_png_decoder;

bool dispatch_decode_png(
  void* /*userdata*/,
  const GhosttyAllocator* allocator,
  const std::uint8_t* data,
  std::size_t data_len,
  GhosttySysImage* out
) {
  if (!g_png_decoder || out == nullptr) {
    return false;
  }

  DecodedImage decoded{};
  if (!g_png_decoder(data, data_len, decoded)) {
    return false;
  }

  std::uint8_t* buf = ghostty_alloc(allocator, decoded.data_len);
  if (buf == nullptr) {
    return false;
  }

  std::memcpy(buf, decoded.data, decoded.data_len);
  out->width = decoded.width;
  out->height = decoded.height;
  out->data = buf;
  out->data_len = decoded.data_len;
  return true;
}

} // namespace

void set_png_decoder(DecodePngCallback callback) {
  g_png_decoder = std::move(callback);
  GhosttySysDecodePngFn fn = g_png_decoder ? &dispatch_decode_png : nullptr;
  detail::throw_if_ghostty_error(
    ghostty_sys_set(GHOSTTY_SYS_OPT_DECODE_PNG, reinterpret_cast<const void*>(fn))
  );
}

void clear_png_decoder() {
  g_png_decoder = nullptr;
  detail::throw_if_ghostty_error(
    ghostty_sys_set(GHOSTTY_SYS_OPT_DECODE_PNG, nullptr)
  );
}

} // namespace libghostty_cpp::sys
