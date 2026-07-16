#pragma once
#include <string_view>
#include <algorithm>
#include <concepts>

namespace rsl::_impl {

constexpr bool is_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

constexpr bool is_digit(char c) {
  return c >= '0' && c <= '9';
}

constexpr bool is_integer(std::string_view str) {
  if (str.empty()) {
    return false;
  }

  if (str[0] == '-') {
    str.remove_prefix(1);
  }
  return std::ranges::all_of(str, is_digit);
}

struct Parser {
  std::string_view data;
  int cursor = 0;

  explicit constexpr Parser(std::string_view data) : data(data) {}

  [[nodiscard]] constexpr bool is_valid() const { return cursor >= 0 && cursor < data.size(); }
  [[nodiscard]] constexpr char current() const { return is_valid() ? data[cursor] : '\0'; }
  [[nodiscard]] constexpr char peek() const {
    return (is_valid() && cursor + 1 < data.size()) ? data[cursor + 1] : '\0';
  }

  constexpr bool consume(char expected) {
    if (current() == expected) {
      ++cursor;
      return true;
    }
    return false;
  }

  constexpr void skip_whitespace() {
    while (is_valid()) {
      if (char c = current(); is_whitespace(c) || c == '\\') {
        ++cursor;
      } else {
        break;
      }
    }
  }

  constexpr void skip_to(std::same_as<char> auto... needles) {
    int brace_count = 0;
    while (is_valid()) {
      if (char c = current(); brace_count == 0 && ((c == needles) || ...)) {
        break;
      } else if (c == '[' || c == '{' || c == '(') {
        ++brace_count;
      } else if (c == ']' || c == '}' || c == ')') {
        --brace_count;
      }
      ++cursor;
    }
  }
};
}  // namespace rsl::_impl