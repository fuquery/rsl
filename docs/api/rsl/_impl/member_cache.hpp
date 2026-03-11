#pragma once
#include <cstddef>
#include <utility>
#include <memory>
#include <meta>
#include <algorithm>

#include <rsl/macro.h>

namespace rsl::_impl {
template <std::meta::info... Members>
struct MemberAccessor {
  template <std::size_t Idx, typename S>
  $inline(always) constexpr static decltype(auto) get(S&& storage) noexcept;

  template <std::size_t Idx, typename S>
  $inline(always) constexpr static decltype(auto) get_addr(S&& storage) noexcept;

  constexpr static auto count                                 = sizeof...(Members);
  constexpr static std::array<std::meta::info, count> types   = {};
  constexpr static std::array<std::meta::info, count> members = {};

  static consteval std::size_t get_index_of(std::meta::info needle) {
    return -1UZ;
  }

  static consteval std::size_t get_index_of(std::string_view name) {
    return -1UZ;
  }

  static consteval bool has_member(std::string_view name) { return false; }
};

}  // namespace rsl::_impl
