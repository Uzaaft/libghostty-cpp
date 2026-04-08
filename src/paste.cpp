#include "libghostty_cpp/paste.hpp"

#include <ghostty/vt.h>

#include "ghostty_result.hpp"

namespace libghostty_cpp::paste {

namespace {

std::vector<std::uint8_t> mutable_copy(std::string_view data) {
  return std::vector<std::uint8_t>(data.begin(), data.end());
}

} // namespace

bool is_safe(std::string_view data) noexcept {
  return ghostty_paste_is_safe(data.data(), data.size());
}

std::size_t required_size(std::string_view data, bool bracketed) {
  std::vector<std::uint8_t> scratch = mutable_copy(data);
  std::size_t written = 0;
  const GhosttyResult result = ghostty_paste_encode(
    reinterpret_cast<char*>(scratch.data()),
    scratch.size(),
    bracketed,
    nullptr,
    0,
    &written
  );

  if (result == GHOSTTY_SUCCESS || result == GHOSTTY_OUT_OF_SPACE) {
    return written;
  }

  detail::throw_if_ghostty_error(result);
  return 0;
}

std::size_t encode(
  std::string_view data,
  bool bracketed,
  std::uint8_t* output,
  std::size_t output_size
) {
  std::vector<std::uint8_t> scratch = mutable_copy(data);
  std::size_t written = 0;
  detail::throw_if_ghostty_error(
    ghostty_paste_encode(
      reinterpret_cast<char*>(scratch.data()),
      scratch.size(),
      bracketed,
      reinterpret_cast<char*>(output),
      output_size,
      &written
    )
  );
  return written;
}

std::vector<std::uint8_t> encode(std::string_view data, bool bracketed) {
  std::vector<std::uint8_t> output;
  encode_to(output, data, bracketed);
  return output;
}

void encode_to(std::vector<std::uint8_t>& output, std::string_view data, bool bracketed) {
  const std::size_t required = required_size(data, bracketed);
  if (required == 0) {
    return;
  }

  const std::size_t original_size = output.size();
  output.resize(original_size + required);

  try {
    const std::size_t written = encode(data, bracketed, output.data() + original_size, required);
    output.resize(original_size + written);
  } catch (...) {
    output.resize(original_size);
    throw;
  }
}

} // namespace libghostty_cpp::paste
