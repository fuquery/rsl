#pragma once
#include <algorithm>
#include <vector>
#include <meta>
#include <string_view>
#include <string>
#include <format>

#include <rsl/string_view>
#include <rsl/span>
#include <rsl/serialize>

#include <rsl/_impl/parser.hpp>

#include "accessor.hpp"
#include "style.hpp"
#include "util.hpp"

namespace rsl::_format_impl {

struct FormatResult {
  using join_t = std::string (*)(StyleMap const*);
  join_t join = nullptr;

  template <auto... Args>
  static std::string join_impl(std::string_view raw, StyleMap const* style_map) {
    static constexpr Style::Tag style_tags[] = {Args...};

    std::string result;
    result.reserve(raw.size() - sizeof...(Args));

    std::size_t index = 0;
    for (auto&& substr : std::views::split(raw, style_separator)) {
      result += std::string_view(substr);
      if (style_map != nullptr && sizeof...(Args) > index) {
        result += (*style_map)[style_tags[index++]];
      }
    }
    return result;
  }

  template <auto... Args>
  constexpr static auto join_impl_ptr = &join_impl<Args...>;

  FormatResult(std::string result, join_t joiner) : raw(std::move(result)), join(joiner) {}

  std::string raw;
  [[nodiscard]] std::string with_style(StyleMap const& style) const {
    return join(&style);
  }
  [[nodiscard]] std::string unstyled() const { return join(nullptr); }
  [[nodiscard]] constexpr explicit(false) operator std::string() const { return unstyled(); }
};



template <rsl::string_view fmt, typename Accessors, /*auto Joiner,*/ typename... Args>
FormatResult format_impl(Args&&... args) {
  auto str = [&]<typename... Xs>(AccessorList<Xs...>) {
    return std::format(fmt.data(), Xs::get(std::forward<Args...[Xs::index]>(args...[Xs::index]))...);
  }(Accessors{});
  return FormatResult(str, nullptr /*extract<FormatResult::join_t>(Joiner)*/);
}

template <rsl::string_view fmt, typename Accessors, std::meta::info Joiner, typename... Args>
constexpr auto format_impl_ptr = &format_impl<fmt, Accessors, /*Joiner,*/ Args...>;

struct FormatString {
  std::string string;
  std::vector<Style> style_tags;
  std::vector<std::meta::info> accessors;

  template <typename... Args>
  using format_type = FormatResult (*)(Args&&...);

  template <typename... Args>
  consteval format_type<Args...> get_format() {
    std::vector<std::meta::info> args = {
        std::meta::reflect_constant(rsl::string_view(std::define_static_string(string)))};
    args.push_back(substitute(^^AccessorList, accessors));

    std::vector<std::meta::info> join_args = {};
    for (auto const& style : style_tags) {
      join_args.emplace_back(style);
    }
    args.push_back(std::meta::reflect_constant(substitute(^^FormatResult::join_impl_ptr, join_args)));

    (args.push_back(^^Args), ...);
    return extract<format_type<Args...>>(substitute(^^format_impl_ptr, args));
  }
};

enum struct ReplacementType : std::uint8_t {
  invalid   = 0,
  remove    = 1,
  automatic = 2,
  indexed   = 3,
  field     = 4
};

struct Replacement final : _impl::Parser {
  ReplacementType kind = ReplacementType::invalid;
  std::string field;
  std::string specs;
  std::vector<std::string> style_tags;

  std::size_t index = -1ZU;

  using _impl::Parser::Parser;

  consteval void add_style(FormatString& out, bool enable) const {
    if (!style_tags.empty()) {
      out.string += style_separator;
      out.style_tags.emplace_back(style_tags, enable);
    }
  }

  consteval void render(FormatString& out) const {
    if (kind == ReplacementType::invalid) {
      // todo emit error?
      return;
    }

    add_style(out, true);
    if (kind == ReplacementType::remove) {
      return;
    }

    out.string += '{';
    out.string += to_string(index);
    if (!specs.empty()) {
      out.string += ":";
      out.string += specs;
    }
    out.string += '}';
    add_style(out, false);
  }

  constexpr void do_parse() {
    if (!is_valid()) {
      kind = ReplacementType::automatic;
      return;
    }

    // color tags
    if (consume('[')) {
      auto start = cursor;
      while (is_valid() && current() != ']') {
        if (current() == ',') {
          style_tags.emplace_back(data.substr(start, cursor - start));
          start = cursor + 1;
        }
        ++cursor;
      }

      if (cursor != start) {
        style_tags.emplace_back(data.substr(start, cursor - start));
      }

      if (!consume(']')) {
        // std::println("missing closing ]");
        kind = ReplacementType::invalid;
        return;
      }
    }

    auto rest = data.substr(cursor);
    if (auto colon_pos = rest.find(':'); colon_pos != rest.npos) {
      field = rest.substr(0, colon_pos);
      specs = rest.substr(colon_pos + 1);
    } else {
      if (current() == ':') {
        specs = rest.substr(1);
      } else {
        field = rest;
      }
    }

    if (field.empty()) {
      kind = ReplacementType::automatic;
    } else if (field == "!") {
      kind = ReplacementType::remove;
      if (!specs.empty()) {
        kind = ReplacementType::invalid;
        // error
      }
    } else {
      kind = std::ranges::all_of(field, _impl::is_digit)  //
                 ? ReplacementType::indexed
                 : ReplacementType::field;
    }
  }

  consteval std::meta::info get_accessor(std::size_t index, std::meta::info type) {
    std::vector accessors{std::meta::reflect_constant(index)};
    for (auto subfield : std::views::split(field, '.')) {
      type = remove_cvref(type);
      std::meta::info accessor{};
      if (std::ranges::all_of(subfield, _impl::is_digit)) {
        index = _serialize_impl::stou(std::string_view{subfield});
        if (is_subscriptable(type)) {
          // member access by subscript operator
          type     = substitute(^^subscript_result, {type});
          accessor = substitute(^^SubscriptAccessor, {std::meta::reflect_constant(index)});
        } else if (is_tuple_like(type)) {
          // member access by get<Idx>(...)
          type     = substitute(^^std::tuple_element_t, {std::meta::reflect_constant(index), type});
          accessor = substitute(^^TupleAccessor, {std::meta::reflect_constant(index)});
        } else {
          // member access by index
          auto member = nonstatic_data_members_of(type, std::meta::access_context::current())[index];
          type        = type_of(member);
          accessor    = substitute(^^MemberAccessor, {reflect_constant(member)});
        }
      } else {
        // member access by name
        index       = get_member_index(type, std::string_view{subfield});
        auto member = nonstatic_data_members_of(type, std::meta::access_context::current())[index];
        type        = type_of(member);
        accessor    = substitute(^^MemberAccessor, {reflect_constant(member)});
      }
      accessors.push_back(accessor);
    }

    return substitute(^^Accessor, accessors);
  }
};

struct FormatParser : _impl::Parser {
  using Parser::Parser;

  FormatString result;

  consteval void push_text(int start, int end) { result.string += data.substr(start, end - start); }

  consteval static std::meta::info get_arg_accessor(std::size_t index) {
    return substitute(^^Accessor, {std::meta::reflect_constant(index)});
  }

  consteval void do_parse(std::vector<std::meta::info> const& arg_types) {
    auto direct = std::vector<int>(arg_types.size(), -1);
    int start   = cursor;

    bool has_positional = false;
    bool has_automatic  = false;

    while (is_valid()) {
      if (current() != '{') {
        ++cursor;
        continue;
      }

      // escaped curly braces
      if (peek() == '{') {
        // skip to first unbalanced } - this will match the outer {
        ++cursor;
        skip_to('}');
        ++cursor;
        continue;
      }
      push_text(start, cursor);
      ++cursor;

      start = cursor;
      skip_to('}');
      auto replacement = Replacement(data.substr(start, cursor - start));
      replacement.do_parse();
      std::size_t index = 0;

      // TODO move into Replacement?
      switch (replacement.kind) {
        case ReplacementType::invalid:
          // todo error
          break;
        case ReplacementType::remove: break;

        case ReplacementType::automatic: {
          has_automatic = true;
          result.accessors.push_back(get_arg_accessor(result.accessors.size()));
          replacement.index = result.accessors.size() - 1;
        } break;
        case ReplacementType::indexed: {
          has_positional = true;
          index          = _serialize_impl::stou(replacement.field);
          if (direct[index] == -1) {
            result.accessors.push_back(get_arg_accessor(index));
            direct[index] = int(result.accessors.size() - 1);
          }
          replacement.index = direct[index];
        } break;
        case ReplacementType::field: {
          has_positional = true;
          auto dot_pos   = replacement.field.find('.');
          if (dot_pos == replacement.field.npos || !_impl::is_digit(replacement.field[0])) {
            if (arg_types.size() != 1) {
              // todo error
              throw "implicit 0. not allowed with more than one arg";
            }
          } else {
            index             = _serialize_impl::stou(replacement.field.substr(0, dot_pos));
            replacement.field = replacement.field.substr(dot_pos + 1);
          }
          result.accessors.push_back(replacement.get_accessor(index, arg_types[index]));
          replacement.index = result.accessors.size() - 1;
        } break;
      }

      replacement.render(result);

      ++cursor;
      start = cursor;
    }

    if (start != cursor) {
      push_text(start, cursor);
    }
  }
};
}  // namespace rsl::_format_impl