#pragma once
#include <meta>
#include <ranges>
#include <algorithm>
#include <bit>

namespace rsl::_serialize_impl {
template <typename T>
struct Enumerator {
  T value;
  char const* name;
};

template <typename T>
consteval auto sorted_enum_pairs() {
  auto enumerators =
      std::vector(std::from_range, enumerators_of(^^T) | std::views::transform([](auto e) {
                                     return Enumerator(extract<T>(constant_of(e)),
                                                       define_static_string(identifier_of(e)));
                                   }));

  using sort_t = std::make_unsigned_t<std::underlying_type_t<T>>;
  std::ranges::stable_sort(enumerators, std::ranges::greater{}, [](auto const& e) {
    return std::popcount(static_cast<sort_t>(e.value));
  });

  return enumerators;
}

}  // namespace rsl::_serialize_impl