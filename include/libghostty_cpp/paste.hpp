#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace libghostty_cpp::paste {

[[nodiscard]] bool is_safe(std::string_view data) noexcept;
[[nodiscard]] std::size_t required_size(std::string_view data, bool bracketed);
[[nodiscard]] std::size_t encode(
  std::string_view data,
  bool bracketed,
  std::uint8_t* output,
  std::size_t output_size
);
[[nodiscard]] std::vector<std::uint8_t> encode(std::string_view data, bool bracketed);
void encode_to(std::vector<std::uint8_t>& output, std::string_view data, bool bracketed);

} // namespace libghostty_cpp::paste
