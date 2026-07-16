#pragma once
#include <meta>
#include <vector>
#include <string_view>

#include <rsl/span>
#include <rsl/string_view>

namespace rsl::_format_impl {
inline constexpr char style_separator = '\037';

struct Style {
  std::vector<std::string> tags;
  bool enable = true;

  struct Tag {
    rsl::span<rsl::string_view const> tags;
    bool enable;
  };

  consteval explicit operator std::meta::info() const {
    auto args = std::vector<rsl::string_view>();
    for (auto const& tag : tags) {
      args.emplace_back(std::define_static_string(tag));
    }
    return std::meta::reflect_constant(Tag{std::define_static_array(args), enable});
  }
};

struct StyleMap {
  virtual ~StyleMap() = default;
  virtual std::string_view operator[](Style::Tag) const { return ""; }
};

}  // namespace rsl::_format_impl