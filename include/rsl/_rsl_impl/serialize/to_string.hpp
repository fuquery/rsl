#pragma once
#include <string>
#include <cstdint>

namespace rsl::_serialize_impl {
constexpr std::string utos(std::uint64_t value) {
  std::string out{};
  do {
    out += static_cast<char>('0' + (value % 10U));
    value /= 10;
  } while (value > 0);
  return {out.rbegin(), out.rend()};
}

constexpr std::uint64_t stou(std::string_view str) {
  unsigned result = 0;
  for (char const c : str) {
    (result *= 10) += c - '0';
  }
  return result;
}
}  // namespace rsl::_serialize_impl
