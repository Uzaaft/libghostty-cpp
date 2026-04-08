#pragma once

#include <cstddef>
#include <cstdint>

namespace libghostty_cpp {

struct ByteView {
  const std::uint8_t* data = nullptr;
  std::size_t size = 0;

  [[nodiscard]] bool empty() const noexcept {
    return size == 0;
  }

  [[nodiscard]] const std::uint8_t* begin() const noexcept {
    return data;
  }

  [[nodiscard]] const std::uint8_t* end() const noexcept {
    return data == nullptr ? nullptr : data + size;
  }
};

} // namespace libghostty_cpp
