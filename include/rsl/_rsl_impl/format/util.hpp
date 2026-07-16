#pragma once
#include <cstddef>
#include <string_view>
#include <ranges>
#include <meta>


//  TODO extract?
namespace rsl::_format_impl {
consteval std::size_t get_member_index(std::meta::info type, std::string_view name) {
  std::vector<std::string_view> names = nonstatic_data_members_of(type, std::meta::access_context::current()) |
                                        std::views::transform(std::meta::identifier_of) |
                                        std::ranges::to<std::vector>();
  if (auto it = std::ranges::find(names, name); it != names.end()) {
    return std::distance(names.begin(), it);
  }
  return -1UZ;
}

template <typename T>
concept subscriptable = requires(T obj) { obj[0]; };

template <typename T>
using subscript_result = decltype(std::declval<T>()[0]);

consteval bool is_subscriptable(std::meta::info r) {
  return extract<bool>(substitute(^^subscriptable, {r}));
}

template <typename T>
concept tuple_like = requires(T obj) {
  typename std::tuple_element_t<0, std::remove_cvref_t<T>>;
  { get<0>(obj) } -> std::convertible_to<std::tuple_element_t<0, std::remove_cvref_t<T>>>;
};

consteval bool is_tuple_like(std::meta::info r) {
  return extract<bool>(substitute(^^tuple_like, {r}));
}

}  // namespace rsl::_format_impl