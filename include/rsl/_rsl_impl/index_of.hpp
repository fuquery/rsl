#pragma once
#include <cstddef>
#include <utility>

// TODO we should do this reflectively instead. The used trick is neat, but not necessary anymore

namespace rsl::_impl {
namespace _typelist_impl {
template <std::size_t Idx, typename T>
struct Tagged {
  using type                         = T;
  constexpr static std::size_t value = Idx;
};

template <std::size_t Idx, typename T>
constexpr T get_type(Tagged<Idx, T>) {
  static_assert(false, "get_type not allowed in an evaluated context");
}

template <typename T, std::size_t Idx>
constexpr Tagged<Idx, T> get_index(Tagged<Idx, T>) {
  static_assert(false, "get_index not allowed in an evaluated context");
}

}  // namespace _typelist_impl

template <typename... Ts>
struct TypeList {
  static constexpr std::size_t size = sizeof...(Ts);

  template <typename = std::index_sequence_for<Ts...>>
  struct GetHelper;

  template <std::size_t... Idx>
  struct GetHelper<std::index_sequence<Idx...>> : _typelist_impl::Tagged<Idx, Ts>... {};

  /**
   * @brief Aliases the type at the specified index
   *
   * @tparam Idx
   */
  template <std::size_t Idx>
    requires(Idx < sizeof...(Ts))
  using type_at =
#if __has_builtin(__type_pack_element)
      __type_pack_element<Idx, Ts...>;
#else
      decltype(_typelist_impl::get_type<Idx>(GetHelper{}));
#endif

  /**
   * @brief Reverse mapping type -> index. Returns sizeof...(Ts) if the type could not be found
   *        or it appeared more than once in the list.
   *
   * @tparam T
   */
  template <typename T>
  constexpr static std::size_t index_of = sizeof...(Ts);

  template <typename T>
  // constraint is not fulfilled if T is contained more than once (=> get_index would be ambiguous)
    requires requires(GetHelper<> obj) { _typelist_impl::get_index<T>(obj); }
  constexpr static std::size_t index_of<T> =
      decltype(_typelist_impl::get_index<T>(GetHelper<>{}))::value;

  using index_type = std::conditional_t<(sizeof...(Ts) >= 255), unsigned short, unsigned char>;

  template <template <typename> class Trait>
  constexpr static bool all = (Trait<Ts>::value && ...);

  template <template <typename> class Trait>
  constexpr static bool any = (Trait<Ts>::value || ...);
};

template <std::size_t Idx, typename List>
using type_at = typename List::template type_at<Idx>;

template <typename T, typename List>
constexpr inline std::size_t index_of = List::template index_of<T>;

}  // namespace rsl::_impl