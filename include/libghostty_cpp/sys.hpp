#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace libghostty_cpp::sys {

struct DecodedImage {
  std::uint32_t width;
  std::uint32_t height;
  const std::uint8_t* data;
  std::size_t data_len;
};

using DecodePngCallback =
  std::function<bool(const std::uint8_t* data, std::size_t data_len, DecodedImage& out)>;

void set_png_decoder(DecodePngCallback callback);
void clear_png_decoder();

} // namespace libghostty_cpp::sys
