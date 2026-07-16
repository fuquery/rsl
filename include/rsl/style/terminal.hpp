#pragma once
#include <rsl/experimental/format>
#include <unordered_map>

namespace rsl::style {
struct Terminal : rsl::style_map {
  static inline std::unordered_map<std::string_view, std::string_view> map = {
      {  "reset",  "\x1b[0m"},
      {  "black", "\x1b[30m"},
      {    "red", "\x1b[31m"},
      {  "green", "\x1b[32m"},
      { "yellow", "\x1b[33m"},
      {   "blue", "\x1b[34m"},
      {"magenta", "\x1b[35m"},
      {   "cyan", "\x1b[36m"},
      {  "white", "\x1b[37m"},
      {"default", "\x1b[3m8"}
  };

  std::string_view operator[](rsl::_format_impl::Style::Tag style) const override {
    if (not style.enable) {
      return "\x1b[0m";
    }

    for (auto tag : style.tags) {
      if (auto it = map.find(tag); it != map.end()) {
        return it->second;
      }
    }
    return "";
  }
};
}  // namespace rsl::style