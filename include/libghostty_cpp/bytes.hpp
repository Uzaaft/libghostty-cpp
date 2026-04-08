#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace libghostty_cpp {

struct ByteView {
  const std::uint8_t* data = nullptr;
  std::size_t size = 0;

  constexpr ByteView() noexcept = default;

  constexpr ByteView(const std::uint8_t* bytes, std::size_t len) noexcept
      : data(bytes), size(len) {}

  ByteView(const void* bytes, std::size_t len) noexcept
      : data(static_cast<const std::uint8_t*>(bytes)), size(len) {}

  ByteView(std::string_view text) noexcept
      : data(reinterpret_cast<const std::uint8_t*>(text.data())), size(text.size()) {}

  [[nodiscard]] bool empty() const noexcept {
    return size == 0;
  }

  [[nodiscard]] const std::uint8_t* begin() const noexcept {
    return data;
  }

  [[nodiscard]] const std::uint8_t* end() const noexcept {
    return data == nullptr ? nullptr : data + size;
  }

  [[nodiscard]] std::string_view as_string_view() const noexcept {
    return data == nullptr
             ? std::string_view {}
             : std::string_view(reinterpret_cast<const char*>(data), size);
  }
};

} // namespace libghostty_cpp
