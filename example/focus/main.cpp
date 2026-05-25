#include "libghostty_cpp/focus.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

int main() {
  const std::vector<std::uint8_t> encoded =
    libghostty_cpp::focus::encode(libghostty_cpp::focus::Event::Gained);
  const std::string text(encoded.begin(), encoded.end());

  std::cout << "Encoded " << encoded.size() << " bytes: " << text << '\n';
  return 0;
}
