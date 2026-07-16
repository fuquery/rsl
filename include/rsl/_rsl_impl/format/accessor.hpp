#pragma once
#include <utility>
#include <meta>

namespace rsl::_format_impl {
  template <std::meta::info Member>
struct MemberAccessor {
  template <typename T>
  friend constexpr decltype(auto) operator>>(T&& obj, MemberAccessor) {
    return std::forward_like<T>(std::forward<T>(obj).[:Member:]);
  }
};

template <std::size_t Idx>
struct SubscriptAccessor {
  template <typename T>
  friend constexpr decltype(auto) operator>>(T&& obj, SubscriptAccessor) {
    return std::forward_like<T>(std::forward<T>(obj)[Idx]);
  }
};

template <std::size_t Idx>
struct TupleAccessor {
  template <typename T>
  friend constexpr decltype(auto) operator>>(T&& obj, TupleAccessor) {
    using std::get;
    return get<Idx>(std::forward<T>(obj));
  }
};

template <std::size_t Idx, typename... Fields>
struct Accessor {
  static constexpr std::size_t index = Idx;

  template <typename T>
  static constexpr decltype(auto) get(T&& obj) {
    return (std::forward<T>(obj) >> ... >> Fields{});
  }
};

template <typename...>
struct AccessorList {};
}