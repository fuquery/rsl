#pragma once
#include <meta>
#include <rsl/meta.h>

namespace rsl {

namespace meta {
template <std::meta::info T, std::meta::info Template>
concept specializes = is_specialization(T, Template);

template <std::meta::info R>
concept function = is_function(R);

template <std::meta::info R>
concept member_function = is_member_function(R);

template <std::meta::info R>
concept static_member_function = is_static_member_function(R);

template <std::meta::info R>
concept nonstatic_member_function = is_nonstatic_member_function(R);

template <std::meta::info R, std::meta::info T>
concept annotated_with = has_annotation(R, T);

template <std::meta::info T>
concept complete_type = is_complete_type(T);

template <std::meta::info T>
concept incomplete_type = !complete_type<T>;
}  // namespace meta

template <typename T, std::meta::info Template>
concept specializes = meta::specializes<$lift(T), Template>;

template <typename T>
concept complete_type = meta::complete_type<$lift(T)>;

template <typename T>
concept incomplete_type = !complete_type<T>;
}  // namespace rsl