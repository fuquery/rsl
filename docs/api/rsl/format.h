#pragma once
#include <concepts>
#include <string_view>
#include <utility>

namespace rsl {
namespace _format_impl {

struct FormatResult;

struct FormatString { 
  template <typename... Args>
  using format_type = FormatResult(*)(Args...);
};


template <typename... Args>
struct Fmt {
  FormatString::format_type<Args...> do_format;

  template <typename T>
    requires std::convertible_to<T const&, std::string_view>
  consteval explicit(false) Fmt(T const& fmt);
};
}  // namespace _format_impl

// using style_map = _format_impl::StyleMap;

using format_result = _format_impl::FormatResult;

template <typename... Args>
using format_string = _format_impl::Fmt<std::type_identity_t<Args>...>;

template <typename... Args>
format_result format(format_string<Args...> fmt, Args&&... args);
}  // namespace rsl