#include "libghostty_cpp/sys.hpp"

#include <ghostty/vt.h>

#include "ghostty_result.hpp"
#include "libghostty_cpp/error.hpp"

#include <algorithm>
#include <cstring>
#include <limits>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

namespace libghostty_cpp::sys {

namespace {

bool dispatch_decode_png(
  void* userdata,
  const GhosttyAllocator* allocator,
  const std::uint8_t* data,
  std::size_t data_len,
  GhosttySysImage* out
);

struct DecoderRegistration {
  std::uint64_t token;
  DecodePngCallback callback;
};

std::mutex g_png_decoder_mutex;
std::vector<DecoderRegistration> g_png_decoders;
std::uint64_t g_png_decoder_next_token = 1;

[[nodiscard]] bool is_valid_decoded_image(const DecodedImage& image) {
  if (image.width == 0 || image.height == 0) {
    return false;
  }

  const std::size_t pixel_count = static_cast<std::size_t>(image.width)
                                  * static_cast<std::size_t>(image.height);
  if (pixel_count > (std::numeric_limits<std::size_t>::max() / 4U)) {
    return false;
  }

  return image.pixels_rgba.size() == (pixel_count * 4U);
}

[[nodiscard]] GhosttySysDecodePngFn active_decoder_fn() {
  return g_png_decoders.empty() ? nullptr : &dispatch_decode_png;
}

bool dispatch_decode_png(
  void* /*userdata*/,
  const GhosttyAllocator* allocator,
  const std::uint8_t* data,
  std::size_t data_len,
  GhosttySysImage* out
) {
  if (out == nullptr) {
    return false;
  }

  DecodePngCallback decoder;
  {
    const std::lock_guard<std::mutex> lock(g_png_decoder_mutex);
    if (g_png_decoders.empty()) {
      return false;
    }

    decoder = g_png_decoders.back().callback;
  }

  std::optional<DecodedImage> decoded;
  try {
    decoded = decoder(ByteView{data, data_len});
  } catch (...) {
    return false;
  }

  if (!decoded.has_value() || !is_valid_decoded_image(*decoded)) {
    return false;
  }

  std::uint8_t* buf = ghostty_alloc(allocator, decoded->pixels_rgba.size());
  if (buf == nullptr) {
    return false;
  }

  std::memcpy(buf, decoded->pixels_rgba.data(), decoded->pixels_rgba.size());
  out->width = decoded->width;
  out->height = decoded->height;
  out->data = buf;
  out->data_len = decoded->pixels_rgba.size();
  return true;
}

} // namespace

PngDecoderRegistration::~PngDecoderRegistration() noexcept {
  reset();
}

PngDecoderRegistration::PngDecoderRegistration(PngDecoderRegistration&& other) noexcept
    : token_(std::exchange(other.token_, 0)) {}

PngDecoderRegistration& PngDecoderRegistration::operator=(
  PngDecoderRegistration&& other
) noexcept {
  if (this == &other) {
    return *this;
  }

  reset();
  token_ = std::exchange(other.token_, 0);
  return *this;
}

bool PngDecoderRegistration::active() const noexcept {
  return token_ != 0;
}

void PngDecoderRegistration::reset() noexcept {
  if (token_ == 0) {
    return;
  }

  const std::lock_guard<std::mutex> lock(g_png_decoder_mutex);
  const auto it = std::find_if(
    g_png_decoders.begin(),
    g_png_decoders.end(),
    [token = token_](const DecoderRegistration& registration) {
      return registration.token == token;
    }
  );
  if (it != g_png_decoders.end()) {
    g_png_decoders.erase(it);
    const GhosttySysDecodePngFn fn = active_decoder_fn();
    (void)ghostty_sys_set(
      GHOSTTY_SYS_OPT_DECODE_PNG,
      reinterpret_cast<const void*>(fn)
    );
  }

  token_ = 0;
}

PngDecoderRegistration install_png_decoder(DecodePngCallback callback) {
  if (!callback) {
    throw Error(ErrorCode::InvalidValue);
  }

  const std::lock_guard<std::mutex> lock(g_png_decoder_mutex);
  const std::uint64_t token = g_png_decoder_next_token++;
  g_png_decoders.push_back(DecoderRegistration{token, std::move(callback)});

  const GhosttySysDecodePngFn fn = active_decoder_fn();
  const GhosttyResult result = ghostty_sys_set(
    GHOSTTY_SYS_OPT_DECODE_PNG,
    reinterpret_cast<const void*>(fn)
  );
  if (result != GHOSTTY_SUCCESS) {
    g_png_decoders.pop_back();
    detail::throw_if_ghostty_error(result);
  }

  return PngDecoderRegistration(token);
}

} // namespace libghostty_cpp::sys
