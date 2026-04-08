#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

#include "libghostty_cpp/bytes.hpp"

namespace libghostty_cpp::sys {

struct DecodedImage {
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::vector<std::uint8_t> pixels_rgba;
};

using DecodePngCallback = std::function<std::optional<DecodedImage>(ByteView png_data)>;

class PngDecoderRegistration {
public:
  PngDecoderRegistration() = default;
  ~PngDecoderRegistration() noexcept;

  PngDecoderRegistration(PngDecoderRegistration&& other) noexcept;
  PngDecoderRegistration& operator=(PngDecoderRegistration&& other) noexcept;

  PngDecoderRegistration(const PngDecoderRegistration&) = delete;
  PngDecoderRegistration& operator=(const PngDecoderRegistration&) = delete;

  [[nodiscard]] bool active() const noexcept;
  void reset() noexcept;

private:
  explicit PngDecoderRegistration(std::uint64_t token) noexcept : token_(token) {}

  std::uint64_t token_ = 0;

  friend PngDecoderRegistration install_png_decoder(DecodePngCallback callback);
};

[[nodiscard]] PngDecoderRegistration install_png_decoder(DecodePngCallback callback);

} // namespace libghostty_cpp::sys
