#pragma once
#include <type_traits>

namespace rsl::_impl {
template <typename T>
concept is_hashable = 
    std::is_default_constructible_v<std::hash<T>>
    && std::is_copy_constructible_v<std::hash<T>>
    && std::is_move_constructible_v<std::hash<T>>
    && std::is_invocable_r_v<std::size_t, std::hash<T>, T const&>;

template <typename T>
concept is_void = std::is_void_v<T>;

template <typename T>
concept is_non_void = !is_void<T>;

template <class T>
inline const bool is_signed_integer_v = false;
template <>
inline const bool is_signed_integer_v<signed char> = true;
template <>
inline const bool is_signed_integer_v<signed short> = true;
template <>
inline const bool is_signed_integer_v<signed int> = true;
template <>
inline const bool is_signed_integer_v<signed long> = true;
template <>
inline const bool is_signed_integer_v<signed long long> = true;

template <class T>
inline const bool is_unsigned_integer_v = false;
template <>
inline const bool is_unsigned_integer_v<unsigned char> = true;
template <>
inline const bool is_unsigned_integer_v<unsigned short> = true;
template <>
inline const bool is_unsigned_integer_v<unsigned int> = true;
template <>
inline const bool is_unsigned_integer_v<unsigned long> = true;
template <>
inline const bool is_unsigned_integer_v<unsigned long long> = true;

template <class T>
concept signed_integer = is_signed_integer_v<T>;

template <class T>
concept unsigned_integer = is_unsigned_integer_v<T>;

template <class T>
concept integer_type = signed_integer<T> or unsigned_integer<T>;
}  // namespace rsl::_impl