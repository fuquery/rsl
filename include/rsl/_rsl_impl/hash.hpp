#pragma once
#include <cstdint>
#include <string_view>

namespace rsl::_impl {

class FNV1a {
  std::uint64_t state = 0xcbf29ce484222325ULL;

public:
  constexpr FNV1a& operator()(char const* data, std::size_t size) {
    for (std::size_t idx = 0; idx < size; ++idx) {
      state ^= static_cast<std::uint64_t>(data[idx]);
      state *= 0x100000001b3ULL;
    }
    return *this;
  }

  constexpr FNV1a& operator()(std::string_view str) { return (*this)(str.data(), str.size()); }

  [[nodiscard]] constexpr std::size_t finalize() const { return state; }
};

using hash_t = std::uint64_t;
constexpr hash_t fnv1a(char const* str, std::size_t size) {
  return FNV1a()(str, size).finalize();
}

constexpr hash_t fnv1a(std::string_view str) {
  return FNV1a()(str).finalize();
}

}  // namespace rsl::_impl