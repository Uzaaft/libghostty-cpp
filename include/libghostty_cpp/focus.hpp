#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace libghostty_cpp::focus {

enum class Event {
  Gained,
  Lost,
};

[[nodiscard]] std::size_t required_size(Event event);
[[nodiscard]] std::size_t encode(Event event, std::uint8_t* output, std::size_t output_size);
[[nodiscard]] std::vector<std::uint8_t> encode(Event event);
void encode_to(std::vector<std::uint8_t>& output, Event event);

} // namespace libghostty_cpp::focus
