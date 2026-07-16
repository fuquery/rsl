#pragma once
#include <meta>
#include <string>
#include <rsl/_rsl_impl/hash.hpp>

namespace rsl::_serialize_impl {
constexpr std::string op_to_string(std::meta::operators op) {
  switch (op) {
    using enum std::meta::operators;
    case op_new: return "new";
    case op_delete: return "delete";
    case op_array_new: return "new[]";
    case op_array_delete: return "delete[]";
    case op_co_await: return "co_await";
    case op_parentheses: return "()";
    case op_square_brackets: return "[]";
    case op_arrow: return "->";
    case op_arrow_star: return "->*";
    case op_tilde: return "~";
    case op_exclamation: return "!";
    case op_plus: return "+";
    case op_minus: return "-";
    case op_star: return "*";
    case op_slash: return "/";
    case op_percent: return "%";
    case op_caret: return "^";
    case op_ampersand: return "&";
    case op_pipe: return "|";
    case op_equals: return "=";
    case op_plus_equals: return "+=";
    case op_minus_equals: return "-=";
    case op_star_equals: return "*=";
    case op_slash_equals: return "/=";
    case op_percent_equals: return "%=";
    case op_caret_equals: return "^=";
    case op_ampersand_equals: return "&=";
    case op_pipe_equals: return "|=";
    case op_equals_equals: return "==";
    case op_exclamation_equals: return "!=";
    case op_less: return "<";
    case op_greater: return ">";
    case op_less_equals: return "<=";
    case op_greater_equals: return ">=";
    case op_spaceship: return "<=>";
    case op_ampersand_ampersand: return "&&";
    case op_pipe_pipe: return "||";
    case op_less_less: return "<<";
    case op_greater_greater: return ">>";
    case op_less_less_equals: return "<<=";
    case op_greater_greater_equals: return ">>=";
    case op_plus_plus: return "++";
    case op_minus_minus: return "--";
    case op_comma: return ",";
  }
  return "<invalid operator>";
}

constexpr std::meta::operators to_operator(std::string_view text) {
  constexpr static auto hash = [](std::string_view str) constexpr {
    auto hasher = _impl::FNV1a{};
    hasher(str);
    return hasher.finalize();
  };

  switch (hash(text)) {
    using enum std::meta::operators;
    case hash("new"): return op_new;
    case hash("delete"): return op_delete;
    case hash("new[]"): return op_array_new;
    case hash("delete[]"): return op_array_delete;
    case hash("co_await"): return op_co_await;
    case hash("()"): return op_parentheses;
    case hash("[]"): return op_square_brackets;
    case hash("->"): return op_arrow;
    case hash("->*"): return op_arrow_star;
    case hash("~"): return op_tilde;
    case hash("!"): return op_exclamation;
    case hash("+"): return op_plus;
    case hash("-"): return op_minus;
    case hash("*"): return op_star;
    case hash("/"): return op_slash;
    case hash("%"): return op_percent;
    case hash("^"): return op_caret;
    case hash("&"): return op_ampersand;
    case hash("|"): return op_pipe;
    case hash("="): return op_equals;
    case hash("+="): return op_plus_equals;
    case hash("-="): return op_minus_equals;
    case hash("*="): return op_star_equals;
    case hash("/="): return op_slash_equals;
    case hash("%="): return op_percent_equals;
    case hash("^="): return op_caret_equals;
    case hash("&="): return op_ampersand_equals;
    case hash("|="): return op_pipe_equals;
    case hash("=="): return op_equals_equals;
    case hash("!="): return op_exclamation_equals;
    case hash("<"): return op_less;
    case hash(">"): return op_greater;
    case hash("<="): return op_less_equals;
    case hash(">="): return op_greater_equals;
    case hash("<=>"): return op_spaceship;
    case hash("&&"): return op_ampersand_ampersand;
    case hash("||"): return op_pipe_pipe;
    case hash("<<"): return op_less_less;
    case hash(">>"): return op_greater_greater;
    case hash("<<="): return op_less_less_equals;
    case hash(">>="): return op_greater_greater_equals;
    case hash("++"): return op_plus_plus;
    case hash("--"): return op_minus_minus;
    case hash(","): return op_comma;
    default: return {};
  }
  return {};
}
}  // namespace rsl::_impl