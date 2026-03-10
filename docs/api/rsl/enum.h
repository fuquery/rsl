#pragma once
#include <meta>
#include <ranges>
#include <type_traits>
#include <utility>
#include <rsl/meta.h>
#include <rsl/_impl/traits.hpp>

namespace rsl {
inline namespace annotations {
struct FlagEnumTag {};
constexpr inline FlagEnumTag flag_enum;

}  // namespace annotations

template <typename T>
concept is_flag_enum = std::is_enum_v<T> && meta::has_annotation($lift(T), $lift(annotations::FlagEnumTag));

template <typename T>
concept is_fixed_enum = std::is_enum_v<T> and requires { T{0}; };

consteval bool has_fixed_underlying_type(std::meta::info r);

template <is_flag_enum E>
constexpr bool has_flag(E flags, E needle);

template <is_flag_enum E>
constexpr bool has_flag(E flags, std::underlying_type_t<E> needle);

namespace _impl {
template <typename E>
struct enum_range {
  using underlying = std::underlying_type_t<E>;
  struct Range {
    underlying min;
    underlying max;
  };

private:
  static consteval Range get_enumerator_range();

  static constexpr int get_bit_width();

public:
  static constexpr auto range     = get_enumerator_range();
  static constexpr auto bit_width = get_bit_width();
  static constexpr auto min_value = E(range.min < 0 ? -(1ULL << (bit_width - 1U)) : 0);
  static constexpr auto max_value = E(range.min < 0 ? -(static_cast<underlying>(min_value) + 1)
                                                    : std::numeric_limits<uint64_t>::max() >>
                                                          (bit_width >= 64 ? 0U : 64 - bit_width));
};
}  // namespace _impl

template <typename T>
struct numeric_limits;

template <typename T>
  requires std::numeric_limits<T>::is_specialized
struct numeric_limits<T> : std::numeric_limits<T> {};

template <typename E>
  requires std::is_enum_v<E> && (not is_fixed_enum<E> or is_flag_enum<E>)
struct numeric_limits<E> {
  using type                                 = E;
  static constexpr const bool is_specialized = false;
  [[nodiscard]] static constexpr type min() noexcept;
  [[nodiscard]] static constexpr type max() noexcept;
  [[nodiscard]] static constexpr type lowest() noexcept;

  static constexpr const int digits       = _impl::enum_range<E>::bit_width;
  static constexpr const int digits10     = digits * 3 / 10;
  static constexpr const int max_digits10 = 0;
  static constexpr const bool is_signed   = std::is_signed_v<std::underlying_type_t<E>>;
  static constexpr const bool is_integer  = true;
  static constexpr const bool is_exact    = true;
  static constexpr const int radix        = 2;
  [[nodiscard]] static constexpr type epsilon() noexcept;
  [[nodiscard]] static constexpr type round_error() noexcept;

  static constexpr const int min_exponent   = 0;
  static constexpr const int min_exponent10 = 0;
  static constexpr const int max_exponent   = 0;
  static constexpr const int max_exponent10 = 0;

  static constexpr const bool has_infinity                                 = false;
  static constexpr const bool has_quiet_NaN                                = false;
  static constexpr const bool has_signaling_NaN                            = false;
  [[deprecated]] static constexpr const std::float_denorm_style has_denorm = std::denorm_absent;
  [[deprecated]] static constexpr const bool has_denorm_loss               = false;
  [[nodiscard]] static constexpr type infinity() noexcept;
  [[nodiscard]] static constexpr type quiet_NaN() noexcept;
  [[nodiscard]] static constexpr type signaling_NaN() noexcept;
  [[nodiscard]] static constexpr type denorm_min() noexcept;

  static constexpr const bool is_iec559  = false;
  static constexpr const bool is_bounded = true;
  static constexpr const bool is_modulo  = false;

  static constexpr const bool traps = std::numeric_limits<std::underlying_type_t<E>>::traps;
  static constexpr const bool tinyness_before               = false;
  static constexpr const std::float_round_style round_style = std::round_toward_zero;
};

template <is_fixed_enum E>
  requires is_fixed_enum<E> and (not is_flag_enum<E>)
struct numeric_limits<E> : std::numeric_limits<std::underlying_type_t<E>> {};

template <typename T, typename V>
  requires std::is_enum_v<T> and
           (std::same_as<V, T> or std::convertible_to<V, std::underlying_type_t<T>>)
constexpr bool in_enum(V value);

template <_impl::integer_type T, _impl::integer_type U>
constexpr bool in_range(U value) noexcept;

// essentially std::in_range but with support for enums
template <typename T, typename U>
  requires std::is_enum_v<T> and
           (std::same_as<U, T> or std::convertible_to<U, std::underlying_type_t<T>>)
constexpr bool in_range(U value) noexcept;
}  // namespace rsl

template <rsl::is_flag_enum E>
  requires std::is_scoped_enum_v<E>
constexpr E operator~(E v) noexcept;

template <rsl::is_flag_enum E, std::convertible_to<std::underlying_type_t<E>> T>
  requires std::is_scoped_enum_v<E>
constexpr bool operator==(E lhs, T rhs) noexcept;

template <rsl::is_flag_enum E, std::convertible_to<std::underlying_type_t<E>> T>
  requires std::is_scoped_enum_v<E>
constexpr bool operator==(T lhs, E rhs) noexcept;

template <rsl::is_flag_enum E, std::convertible_to<std::underlying_type_t<E>> T>
  requires std::is_scoped_enum_v<E>
constexpr bool operator!=(E lhs, T rhs) noexcept;

template <rsl::is_flag_enum E, std::convertible_to<std::underlying_type_t<E>> T>
  requires std::is_scoped_enum_v<E>
constexpr bool operator!=(T lhs, E rhs) noexcept;

template <rsl::is_flag_enum E>
  requires std::is_scoped_enum_v<E>
constexpr E operator|(E lhs, E rhs) noexcept;

template <rsl::is_flag_enum E>
  requires std::is_scoped_enum_v<E>
constexpr E operator&(E lhs, E rhs) noexcept;

template <rsl::is_flag_enum E>
  requires std::is_scoped_enum_v<E>
constexpr E operator^(E lhs, E rhs) noexcept;

template <rsl::is_flag_enum E, std::convertible_to<std::underlying_type_t<E>> T>
  requires std::is_scoped_enum_v<E>
constexpr E operator|(E lhs, T rhs) noexcept;

template <rsl::is_flag_enum E, std::convertible_to<std::underlying_type_t<E>> T>
  requires std::is_scoped_enum_v<E>
constexpr E operator&(E lhs, T rhs) noexcept;

template <rsl::is_flag_enum E, std::convertible_to<std::underlying_type_t<E>> T>
  requires std::is_scoped_enum_v<E>
constexpr E operator^(E lhs, T rhs) noexcept;

template <rsl::is_flag_enum E, std::convertible_to<std::underlying_type_t<E>> T>
  requires std::is_scoped_enum_v<E>
constexpr E operator|(T lhs, E rhs) noexcept;

template <rsl::is_flag_enum E, std::convertible_to<std::underlying_type_t<E>> T>
  requires std::is_scoped_enum_v<E>
constexpr E operator&(T lhs, E rhs) noexcept;

template <rsl::is_flag_enum E, std::convertible_to<std::underlying_type_t<E>> T>
  requires std::is_scoped_enum_v<E>
constexpr E operator^(T lhs, E rhs) noexcept;

template <rsl::is_flag_enum E>
  requires std::is_scoped_enum_v<E>
constexpr E& operator|=(E& lhs, E rhs) noexcept;

template <rsl::is_flag_enum E>
  requires std::is_scoped_enum_v<E>
constexpr E& operator&=(E& lhs, E rhs) noexcept;

template <rsl::is_flag_enum E>
  requires std::is_scoped_enum_v<E>
constexpr E& operator^=(E& lhs, E rhs) noexcept;

template <rsl::is_flag_enum E, std::convertible_to<std::underlying_type_t<E>> T>
  requires std::is_scoped_enum_v<E>
constexpr E& operator|=(E& lhs, T rhs) noexcept;

template <rsl::is_flag_enum E, std::convertible_to<std::underlying_type_t<E>> T>
  requires std::is_scoped_enum_v<E>
constexpr E& operator&=(E& lhs, T rhs) noexcept;

template <rsl::is_flag_enum E, std::convertible_to<std::underlying_type_t<E>> T>
  requires std::is_scoped_enum_v<E>
constexpr E& operator^=(E& lhs, T rhs) noexcept;

template <rsl::is_flag_enum E, std::convertible_to<std::underlying_type_t<E>> T>
  requires std::is_scoped_enum_v<E>
constexpr T& operator|=(T& lhs, E rhs) noexcept;
template <rsl::is_flag_enum E, std::convertible_to<std::underlying_type_t<E>> T>
  requires std::is_scoped_enum_v<E>
constexpr T& operator&=(T& lhs, E rhs) noexcept;

template <rsl::is_flag_enum E, std::convertible_to<std::underlying_type_t<E>> T>
  requires std::is_scoped_enum_v<E>
constexpr T& operator^=(T& lhs, E rhs) noexcept;