#pragma once
#include <cstddef>
#include <utility>
#include <memory>
#include <meta>
#include <algorithm>

#include <rsl/macro>

namespace rsl::_impl {
template <std::meta::info... Members>
struct MemberAccessor {
  template <std::size_t Idx, typename S>
  $inline(always) constexpr static decltype(auto) get(S&& storage) noexcept {
    return std::forward_like<S>(storage.[:Members...[Idx]:]);
  }

  template <std::size_t Idx, typename S>
  $inline(always) constexpr static decltype(auto) get_addr(S&& storage) noexcept {
    return std::addressof(std::forward<S>(storage).[:Members...[Idx]:]);
  }

  constexpr static auto count                                 = sizeof...(Members);
  constexpr static std::array<std::meta::info, count> types   = {dealias(type_of(Members))...};
  constexpr static std::array<std::meta::info, count> members = {Members...};

  static consteval std::size_t get_index_of(std::meta::info needle) {
    constexpr std::size_t npos = ~0ZU;
    std::size_t selected       = npos;
    for (std::size_t idx = 0; idx < types.size(); ++idx) {
      if (types[idx] != needle) {
        continue;
      }

      if (selected != npos) {
        return npos;
      }

      selected = idx;
    }
    return selected;
  }

  static consteval std::size_t get_index_of(std::string_view name) {
    std::vector<std::string_view> names;
    names.reserve(count);
    for (auto member : std::vector<std::meta::info>{Members...}) {
      if (not has_identifier(member)) {
        continue;
      }
      names.push_back(identifier_of(member));
    }

    if (auto it = std::ranges::find(names, name); it != names.end()) {
      return std::distance(names.begin(), it);
    }
    return -1UZ;
  }

  static consteval bool has_member(std::string_view name) { return get_index_of(name) != -1UZ; }
};

template <std::meta::info... Members>
constexpr inline MemberAccessor<Members...> member_cache{};

consteval auto cache_members(auto&& member_range) {
  std::vector<std::meta::info> members;
  for (auto member : member_range) {
    members.push_back(std::meta::reflect_constant(member));
  }
  return substitute(^^member_cache, members);
}

consteval auto cache_members_t(auto&& member_range) {
  std::vector<std::meta::info> members;
  for (auto member : member_range) {
    members.push_back(std::meta::reflect_constant(member));
  }
  return substitute(^^MemberAccessor, members);
};
}  // namespace rsl::_impl
