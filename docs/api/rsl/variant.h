#pragma once
#include <cstddef>
#include <concepts>
#include <type_traits>
#include <utility>
#include <memory>
#include <compare>
#include <ranges>
#include <algorithm>
#include <meta>

// #include <rsl/serialize.h>

// #include <rsl/_impl/traits.hpp>
#include <rsl/_impl/member_cache.hpp>
// #include <rsl/_impl/index_of.hpp>

#include <rsl/macro.h>

namespace rsl {
template <std::size_t, typename>
struct variant_alternative;

template <typename>
struct variant_size;

namespace _variant_impl {

inline constexpr std::size_t variant_npos = -1ULL;
[[noreturn]] void throw_bad_variant_access(bool);

class bad_variant_access : public std::exception {
  friend void throw_bad_variant_access(bool);
  char const* reason = "bad variant access";
  explicit bad_variant_access(const char* message) noexcept : reason(message) {}

public:
  bad_variant_access() noexcept = default;
  [[nodiscard]] const char* what() const noexcept override;
};

[[noreturn]] void throw_bad_variant_access(bool valueless);

template <typename Source, typename Dest>
concept allowed_conversion = requires(Source obj) { std::type_identity_t<Dest[]>{std::move(obj)}; };

template <std::size_t Idx, typename T>
struct Build_FUN {
  template <allowed_conversion<T> U>
  auto operator()(T, U&&) -> std::integral_constant<std::size_t, Idx>;
};

template <typename V, typename = std::make_index_sequence<V::size>>
struct Build_FUNs;

template <template <typename...> class V, typename... Ts, std::size_t... Idx>
struct Build_FUNs<V<Ts...>, std::index_sequence<Idx...>> : Build_FUN<Idx, Ts>... {
  using Build_FUN<Idx, Ts>::operator()...;
};

template <typename T, typename V>
inline constexpr auto selected_index = variant_npos;

template <typename T, typename V>
  requires std::invocable<Build_FUNs<V>, T, T>
inline constexpr auto selected_index<T, V> = std::invoke_result_t<Build_FUNs<V>, T, T>::value;

template <typename T>
concept is_in_place = false;

template <typename T>
concept has_get = requires(T obj) { obj.template get<0>(); };

template <typename Operator>
struct ComparisonVisitor {
  template <typename T1, typename T2>
  constexpr bool operator()(T1&& lhs, T2&& rhs);
};

struct placeholder{};
}  // namespace _variant_impl

namespace _visit_impl {
template <std::size_t... Dimensions>
struct Key {
  constexpr static std::size_t size      = sizeof...(Dimensions);
  constexpr static std::size_t max_index = (Dimensions * ... * 1);

  static consteval auto generate_offsets() {
    std::array<std::size_t, size> result = {};
    constexpr std::size_t dimensions[]   = {Dimensions...};
    result[0]                            = 1;
    for (std::size_t idx = 1; idx < size; ++idx) {
      result[idx] = dimensions[idx - 1] * result[idx - 1];
    }

    return result;
  }

  constexpr static auto offsets = generate_offsets();

  std::size_t index                             = static_cast<std::size_t>(-1);
  std::size_t subindices[sizeof...(Dimensions)] = {};

  constexpr explicit Key(std::size_t index_) noexcept : index(index_) {
    std::size_t key                    = index_;
    constexpr std::size_t dimensions[] = {Dimensions...};
    for (std::size_t idx = 0; idx < size; ++idx) {
      subindices[idx] = key % dimensions[idx];
      key /= dimensions[idx];
    }
  }

  constexpr explicit Key(std::convertible_to<std::size_t> auto... subindices_) noexcept
    requires(sizeof...(subindices_) > 1)
      : index(0)
      , subindices(subindices_...) {
    static_assert(sizeof...(subindices_) == size,
                  "Number of indices must match the number of dimensions.");
    for (std::size_t idx = 0; idx < size; ++idx) {
      index += subindices[idx] * offsets[idx];
    }
  }

  constexpr explicit operator std::size_t() const noexcept;
  constexpr std::size_t operator[](std::size_t position) const noexcept;
  friend constexpr auto operator<=>(Key const& self, std::size_t other);
  friend constexpr bool operator==(Key const& self, std::size_t other);
};

template <typename... Variants>  // possibly cv qualified
struct VisitImpl {
  using key_type = Key<variant_size<std::remove_cvref_t<Variants>>::value...>;
  static constexpr std::size_t max_index = key_type::max_index;

  template <key_type Tag, typename = std::make_index_sequence<sizeof...(Variants)>>
  struct Dispatch;

  template <key_type Tag, std::size_t... Idx>
  struct Dispatch<Tag, std::index_sequence<Idx...>> {
    template <typename F, typename... U>
    $inline(always) constexpr static decltype(auto) visit(F&& visitor, U&&... variants);
  };

  template <std::size_t Idx, typename F, typename... U>
  $inline(always) constexpr static decltype(auto) visit(F&& visitor, U&&... variants);
};

template <typename Variant>
struct VisitImpl<Variant> {
  using key_type                         = std::size_t;
  static constexpr std::size_t max_index = variant_size<std::remove_cvref_t<Variant>>::value;

  template <std::size_t Idx, typename F, typename U>
  $inline(always) constexpr static decltype(auto) visit(F&& visitor, U&& variant);
};

template <std::size_t Idx,
          typename V,
          typename Alt = typename rsl::variant_alternative<0, std::remove_reference_t<V>>::type>
// TODO: Vs... might be derived from variant
using get_t = std::conditional_t<std::is_lvalue_reference_v<V>, Alt&, Alt&&>;

// non-exhaustive result type check
template <typename F, typename... Vs>
using visit_result_t = std::invoke_result_t<F, get_t<0, Vs>...>;

template <typename R, typename F, typename V>
constexpr R visit_at_enumerated(std::size_t idx, F&& visitor, V&& variant);

template <typename R, typename F, typename... Vs>
constexpr R visit_at(std::size_t idx, F&& visitor, Vs&&... variants);
}  // namespace _visit_impl

template <typename R, typename F, typename... Vs>
constexpr R visit(F&& visitor, Vs&&... variants);

template <typename F, typename... Vs>
constexpr decltype(auto) visit(F&& visitor, Vs&&... variants);

namespace _variant_impl {
template <typename Storage>
class variant_base {
protected:
  friend struct variant_alternative;
  friend struct variant_size<variant_base>;

  constexpr void reset();

  [[nodiscard]] constexpr bool can_nothrow_move() const;

  template <typename V>
  void do_construct(variant_base& lhs, V&& rhs);

public:
  constexpr static auto alternatives = _impl::MemberAccessor<{},{}>();
  using index_type = std::conditional_t<(alternatives.count >= 255), unsigned short, unsigned char>;
  static constexpr auto npos = index_type(-1ULL);

  union {
    Storage _impl_storage;
  };
  index_type _impl_discriminator = npos;

  // default constructor, only if alternative #0 is default constructible
  constexpr variant_base()                                                    //
      noexcept(false)  // [variant.ctor]/5
    requires(is_default_constructible_type(std::meta::info{}));           // [variant.ctor]/2
                                                                              // [variant.ctor]/3

  constexpr variant_base(variant_base const& other) = default;
  constexpr variant_base(variant_base const& other)                              //
      noexcept(false)  //
    requires(!std::is_trivially_destructible_v<Storage> &&                       // [variant.ctor]/9
             std::ranges::all_of(alternatives.types,                             //
                                 std::meta::is_copy_constructible_type));
  
  constexpr variant_base(variant_base&& other) = default;
  constexpr variant_base(variant_base&& other)  //
      noexcept(false) 
    requires(!std::is_trivially_destructible_v<Storage> &&                     // [variant.ctor]/13
             std::ranges::all_of(alternatives.types, std::meta::is_move_constructible_type));

  // TODO rewrite selected_index
  template <typename T>
  constexpr static auto selected_index = 0;

  // converting constructor
  template <typename T>
    requires(alternatives.count != 0                                 // [variant.ctor]/15.1
             && !std::same_as<std::remove_cvref_t<T>, variant_base>  // [variant.ctor]/15.2
             && !_variant_impl::is_in_place<std::remove_cvref_t<T>>  // [variant.ctor]/15.3
             && selected_index<T> != variant_npos)  // [variant.ctor]/15.4, [variant.ctor]/15.5
  constexpr explicit variant_base(T&& obj)          //
      noexcept(false)  // [variant.ctor]/18
      ;

  // in place constructors
  template <typename T, typename... Args>
  constexpr explicit variant_base(std::in_place_type_t<T>, Args&&... args)  //
      noexcept(std::is_nothrow_constructible_v<T, Args...>)                 // [variant.ctor]/23
      ;

  template <typename T, typename U, typename... Args>
  constexpr explicit variant_base(std::in_place_type_t<T>,
                                  std::initializer_list<U> init_list,
                                  Args&&... args)  //
      noexcept(std::is_nothrow_constructible_v<T,
                                               std::initializer_list<U>&,
                                               Args...>)  // [variant.ctor]/28
      ;

  template <std::size_t Idx, typename... Args>
  constexpr explicit variant_base(std::in_place_index_t<Idx>, Args&&... args)  //
      noexcept(false)
      ;

  template <std::size_t Idx, typename U, typename... Args>
  constexpr explicit variant_base(std::in_place_index_t<Idx>,
                                  std::initializer_list<U> init_list,
                                  Args&&... args)  //
      noexcept(false)
      ;

  constexpr variant_base& operator=(variant_base const& other) = default;
  constexpr variant_base& operator=(variant_base&& other)      = default;

  constexpr variant_base& operator=(variant_base const& other)
    requires(!std::is_trivially_destructible_v<Storage> &&
             std::ranges::all_of(alternatives.types, std::meta::is_copy_assignable_type))
  ;

  constexpr variant_base& operator=(variant_base&& other) noexcept
    requires(!std::is_trivially_destructible_v<Storage> &&
             std::ranges::all_of(alternatives.types, std::meta::is_move_assignable_type))
  ;

  // converting assignment
  template <typename T>
    requires(!std::same_as<std::remove_cvref_t<T>, variant_base> &&
             !_variant_impl::is_in_place<std::remove_cvref_t<T>> &&
             selected_index<T> != variant_npos &&
             std::assignable_from<_variant_impl::placeholder&, T>)
  constexpr variant_base& operator=(T&& obj) noexcept(false);

  constexpr ~variant_base()
    requires std::is_trivially_destructible_v<Storage>
  = default;
  constexpr ~variant_base();
  [[nodiscard]] constexpr bool valueless_by_exception() const noexcept;
  [[nodiscard]] constexpr std::size_t index() const noexcept;

  template <std::size_t Idx, typename... Args>
  constexpr void emplace(Args&&... args);

  template <typename T, typename... Args>
  constexpr void emplace(Args&&... args);

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get_alt(this Self&& self);

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self);

  template <typename T, typename Self>
  constexpr decltype(auto) get(this Self&& self);

  void swap(variant_base& other);
};

template <typename Storage>
concept is_threeway_comparable = false;

consteval std::meta::info common_comparison_category(auto&& R);

template <typename Storage>
using common_comparison_category_t = $splice(common_comparison_category(
                                           variant_base<Storage>::alternatives.types));
}  // namespace _variant_impl

// needed for visitation in base
template <typename Storage>
struct variant_size<_variant_impl::variant_base<Storage>>
    : std::integral_constant<std::size_t,
                             _variant_impl::variant_base<Storage>::alternatives.count> {};

template <typename Storage>
struct variant_size<_variant_impl::variant_base<Storage> const>
    : variant_size<_variant_impl::variant_base<Storage>> {};

template <std::size_t Idx, typename Storage>
struct variant_alternative<Idx, _variant_impl::variant_base<Storage>> {
  static_assert(Idx < _variant_impl::variant_base<Storage>::alternatives.count,
                "variant_alternative index out of range");
  using type = $splice(_variant_impl::variant_base<Storage>::alternatives.types[Idx]);
};

template <std::size_t Idx, typename Storage>
struct variant_alternative<Idx, _variant_impl::variant_base<Storage> const>
    : variant_alternative<Idx, _variant_impl::variant_base<Storage>> {};

template <std::size_t Idx, typename V>
using variant_alternative_t = typename variant_alternative<Idx, std::remove_reference_t<V>>::type;

template <typename T>
inline constexpr std::size_t variant_size_v = rsl::variant_size<T>::value;

struct monostate {};

template <typename Storage>
constexpr auto swap(_variant_impl::variant_base<Storage>& lhs,
                    _variant_impl::variant_base<Storage>& rhs) noexcept(noexcept(lhs.swap(rhs)))
    -> decltype(lhs.swap(rhs));

template <typename Storage>
  requires(_variant_impl::is_threeway_comparable<Storage>)
constexpr auto operator<=>(_variant_impl::variant_base<Storage> const& lhs,
                           _variant_impl::variant_base<Storage> const& rhs)
    -> _variant_impl::common_comparison_category_t<Storage>;

template <typename Storage>
constexpr bool operator==(_variant_impl::variant_base<Storage> const& lhs,
                          _variant_impl::variant_base<Storage> const& rhs);

template <typename Storage>
constexpr bool operator!=(_variant_impl::variant_base<Storage> const& lhs,
                          _variant_impl::variant_base<Storage> const& rhs);

template <typename Storage>
constexpr bool operator<(_variant_impl::variant_base<Storage> const& lhs,
                         _variant_impl::variant_base<Storage> const& rhs);

template <typename Storage>
constexpr bool operator>(_variant_impl::variant_base<Storage> const& lhs,
                         _variant_impl::variant_base<Storage> const& rhs);

template <typename Storage>
constexpr bool operator<=(_variant_impl::variant_base<Storage> const& lhs,
                          _variant_impl::variant_base<Storage> const& rhs);

template <typename Storage>
constexpr bool operator>=(_variant_impl::variant_base<Storage> const& lhs,
                          _variant_impl::variant_base<Storage> const& rhs);

constexpr bool operator==(monostate, monostate) noexcept;
constexpr std::strong_ordering operator<=>(monostate, monostate) noexcept;

//? [variant.get], value access

/**
 * @brief Check if the desired alternative is currently held
 * @warning non-standard extension
 * @tparam Idx index of the desired alternative
 * @tparam Ts
 * @param obj
 * @return true
 * @return false
 */
template <std::size_t Idx, typename Storage>
constexpr bool holds_alternative(_variant_impl::variant_base<Storage> const& obj) noexcept;

/**
 * @brief Check if the desired alternative is currently held
 * @warning non-standard extension
 * @tparam T type of the desired alternative
 * @tparam Ts
 * @param obj
 * @return true
 * @return false
 */
template <class T, typename Storage>
constexpr bool holds_alternative(_variant_impl::variant_base<Storage> const& obj) noexcept;

template <std::size_t Idx, _variant_impl::has_get V>
constexpr decltype(auto) get(V&& variant_);

template <typename T, _variant_impl::has_get V>
constexpr decltype(auto) get(V&& variant_);

template <std::size_t Idx, typename Storage>
constexpr auto get_if(_variant_impl::variant_base<Storage>* variant_) noexcept
    -> variant_alternative_t<Idx, _variant_impl::variant_base<Storage>>*;

template <typename T, typename Storage>
constexpr auto* get_if(_variant_impl::variant_base<Storage>* variant_) noexcept;

template <std::size_t, typename>
struct variant_alternative;

template <typename>
struct variant_size;

inline constexpr std::size_t variant_npos = _variant_impl::variant_npos;

using bad_variant_access = _variant_impl::bad_variant_access;

namespace _impl {
template <typename... Ts>
struct Storage {
  union type;
};
}  // namespace _impl

template <typename... Ts>
class variant : public _variant_impl::variant_base<typename _impl::Storage<Ts...>::type> {
  static_assert(sizeof...(Ts) > 0, "variant must contain at least one alternative");
  static_assert((!std::is_reference_v<Ts> && ...), "variant must not have reference alternatives");
  static_assert((!std::is_void_v<Ts> && ...), "variant must not have void alternatives");
  using storage_type = _impl::Storage<Ts...>::type;
  using base         = _variant_impl::variant_base<storage_type>;

public:
  using _variant_impl::variant_base<storage_type>::variant_base;
  constexpr variant(variant const&) = default;
  constexpr variant(variant&&)      = default;
  using _variant_impl::variant_base<storage_type>::variant_base::operator=;
  constexpr variant& operator=(variant const&) = default;
  constexpr variant& operator=(variant&&)      = default;
  constexpr ~variant()                         = default;

  using base::emplace;
  using base::get;
  using base::get_alt;  // TODO hide
  using base::index;
  using base::swap;
  using base::valueless_by_exception;

  template <typename Self, typename V>
  constexpr decltype(auto) visit(this Self&& self, V&& visitor);
};

//? [variant.helper], variant helper classes
template <typename... Ts>
struct variant_size<variant<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <typename... Ts>
struct variant_size<variant<Ts...> const> : std::integral_constant<std::size_t, sizeof...(Ts)> {};
template <std::size_t Idx, typename... Ts>
struct variant_alternative<Idx, variant<Ts...>> {
  static_assert(Idx < sizeof...(Ts), "variant_alternative index out of range");
  using type = Ts...[Idx];
};

template <std::size_t Idx, typename... Ts>
struct variant_alternative<Idx, variant<Ts...> const> {
  static_assert(Idx < sizeof...(Ts), "variant_alternative index out of range");
  using type = std::add_const_t<Ts...[Idx]>;
};

namespace _impl {
template <typename T>
concept is_enum = std::is_scoped_enum_v<T> || std::is_enum_v<T>;

template <typename T>
struct TypeTag {
  using type = T;
};

template <typename T>
using unwrap_type_tag = typename T::type;
}  // namespace _impl

template <typename T>
constexpr inline _impl::TypeTag<T> type{};

namespace _tagged_variant_impl {
template <_impl::is_enum T>
consteval std::span<T const> make_transitions() {
  return {};
}

template <_impl::is_enum T>
consteval std::size_t find_inverse_transition(T value) {
  return -1ULL;
}

template <auto V>
constexpr inline auto inverse_transition = find_inverse_transition(V);

template <_impl::is_enum E>
constexpr static auto transitions = _tagged_variant_impl::make_transitions<E>();

template <_impl::is_enum E>
struct Storage {
  union type;
};

}  // namespace _tagged_variant_impl

template <typename E>
struct tagged_variant : public _variant_impl::variant_base<typename _tagged_variant_impl::Storage<E>::type> {
  using storage_type = _tagged_variant_impl::Storage<E>::type;
  using base         = _variant_impl::variant_base<storage_type>;

public:
  using _variant_impl::variant_base<storage_type>::variant_base;
  constexpr tagged_variant(tagged_variant const&)            = default;
  constexpr tagged_variant(tagged_variant&&)                 = default;
  constexpr tagged_variant& operator=(tagged_variant const&) = default;
  constexpr tagged_variant& operator=(tagged_variant&&)      = default;
  constexpr ~tagged_variant()                                = default;

  using tags = E;

  storage_type* operator->();
  storage_type const* operator->() const;
  storage_type* operator*();
  storage_type const* operator*() const;

  explicit(false) operator E() const;

  using base::emplace;
  using base::get;
  using base::get_alt;  // TODO hide
  using base::index;
  using base::swap;
  using base::valueless_by_exception;

  template <typename Self, typename V>
  constexpr decltype(auto) visit(this Self&& self, V&& visitor);
};

template <_impl::is_enum E>
constexpr E get_tag(tagged_variant<E> const& variant_);

template <auto E>
  requires(_impl::is_enum<decltype(E)>)
constexpr bool holds_alternative(tagged_variant<decltype(E)> const& variant_) noexcept;

template <auto E, _variant_impl::has_get V>
  requires(_impl::is_enum<decltype(E)> &&
           std::same_as<rsl::tagged_variant<decltype(E)>, std::remove_cvref_t<V>>)
constexpr decltype(auto) get(V&& variant_);

template <auto E>
  requires(_impl::is_enum<decltype(E)>)
constexpr auto* get_if(tagged_variant<decltype(E)>* variant_) noexcept;

namespace _impl {

template <typename E>
concept all_alts_hashable = false;

template <typename E>
concept all_alts_noexcept_hashable = false;
}  // namespace _impl
}  // namespace rsl

template <typename E>
  requires(rsl::_impl::all_alts_hashable<E>)
struct std::hash<rsl::tagged_variant<E>> {
  using result_type = std::size_t;

  std::size_t operator()(rsl::tagged_variant<E> const& obj) const
      noexcept(rsl::_impl::all_alts_noexcept_hashable<E>);
};

template <>
struct std::hash<rsl::monostate> {
  using result_type   = std::size_t;
  using argument_type = rsl::monostate;

  std::size_t operator()(const rsl::monostate&) const noexcept;
};

namespace rsl::_impl { template<typename...> constexpr auto is_hashable = false; }

template <typename... Ts>
  requires(rsl::_impl::is_hashable<Ts> && ...)
struct std::hash<rsl::variant<Ts...>> {
  using result_type = std::size_t;

  std::size_t operator()(rsl::variant<Ts...> const& obj) const
      noexcept((std::is_nothrow_invocable_v<std::hash<std::remove_const_t<Ts>>, Ts const&> &&
                ...));
};
