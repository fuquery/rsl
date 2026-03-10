#pragma once
#include <algorithm>
#include <meta>
#include <ranges>

#include <rsl/macro.h>

namespace rsl::meta {

consteval bool is_specialization(std::meta::info type, std::meta::info templ);

using std::meta::is_function;
consteval bool is_member_function(std::meta::info r);
consteval bool is_static_member_function(std::meta::info r);
consteval bool is_nonstatic_member_function(std::meta::info r);

template <typename T>
consteval bool has_annotation(std::meta::info item);

consteval bool has_annotation(std::meta::info item, std::meta::info type);

template <typename T>
consteval bool has_annotation(std::meta::info item, T const& value);

consteval std::vector<std::meta::info> get_annotations(std::meta::info item, std::meta::info type);

consteval std::meta::info get_annotation(std::meta::info item, std::meta::info type);

consteval bool has_parent(std::meta::info R);

consteval std::meta::info get_member_by_name(std::meta::info r, std::string_view name);

consteval std::meta::info inject_aggregate(std::meta::reflection_range auto&& fields);
}  // namespace rsl::meta