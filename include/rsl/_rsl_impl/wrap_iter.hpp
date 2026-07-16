#pragma once
#include <iterator>

namespace rsl::_span_impl {
struct span_common;
}

namespace rsl::_impl {
template <typename It>
class WrappedIterator {
public:
  using iterator_type     = It;
  using value_type        = typename std::iterator_traits<iterator_type>::value_type;
  using difference_type   = typename std::iterator_traits<iterator_type>::difference_type;
  using pointer           = typename std::iterator_traits<iterator_type>::pointer;
  using reference         = typename std::iterator_traits<iterator_type>::reference;
  using iterator_category = typename std::iterator_traits<iterator_type>::iterator_category;
  using iterator_concept  = std::contiguous_iterator_tag;

private:
  iterator_type _impl_iter;

public:
  constexpr WrappedIterator() noexcept : _impl_iter() {}
  template <typename OtherIt>
    requires(
        std::is_convertible_v<const OtherIt&, It> &&
        (std::same_as<reference, typename std::iterator_traits<OtherIt>::reference> ||
         std::same_as<
             reference,
             std::remove_reference_t<typename std::iterator_traits<OtherIt>::reference> const&>))
  constexpr explicit(false) WrappedIterator(const WrappedIterator<OtherIt>& other) noexcept
      : _impl_iter(other._impl_iter) {}

  constexpr reference operator*() const noexcept { return *_impl_iter; }
  constexpr pointer operator->() const noexcept { return std::to_address(_impl_iter); }

  constexpr WrappedIterator& operator++() noexcept {
    ++_impl_iter;
    return *this;
  }

  constexpr WrappedIterator operator++(int) noexcept {
    WrappedIterator tmp(*this);
    ++(*this);
    return tmp;
  }

  constexpr WrappedIterator& operator--() noexcept {
    --_impl_iter;
    return *this;
  }

  constexpr WrappedIterator operator--(int) noexcept {
    WrappedIterator tmp(*this);
    --(*this);
    return tmp;
  }

  constexpr WrappedIterator operator+(difference_type n) const noexcept {
    WrappedIterator tmp(*this);
    tmp += n;
    return tmp;
  }

  constexpr WrappedIterator& operator+=(difference_type n) noexcept {
    _impl_iter += n;
    return *this;
  }

  constexpr WrappedIterator operator-(difference_type n) const noexcept { return *this + (-n); }

  constexpr WrappedIterator& operator-=(difference_type n) noexcept {
    *this += -n;
    return *this;
  }
  constexpr reference operator[](difference_type n) const noexcept { return _impl_iter[n]; }

  constexpr iterator_type base() const noexcept { return _impl_iter; }

private:
  constexpr explicit WrappedIterator(iterator_type it) noexcept : _impl_iter(it) {}

  template <typename T>
  friend class WrappedIterator;

  template <typename CharT, typename Traits>
  friend class basic_string_view;

  template <typename T, std::size_t>
  friend class span;
  friend class _span_impl::span_common;
};

template <typename It>
constexpr bool operator==(const WrappedIterator<It>& rhs,
                                  const WrappedIterator<It>& lhs) noexcept {
  return rhs.base() == lhs.base();
}

template <typename It1, typename It2>
constexpr bool operator==(const WrappedIterator<It1>& rhs,
                                  const WrappedIterator<It2>& lhs) noexcept {
  return rhs.base() == lhs.base();
}

template <typename It>
constexpr bool operator<(const WrappedIterator<It>& rhs,
                         const WrappedIterator<It>& lhs) noexcept {
  return rhs.base() < lhs.base();
}

template <typename It1, typename It2>
constexpr bool operator<(const WrappedIterator<It1>& rhs,
                         const WrappedIterator<It2>& lhs) noexcept {
  return rhs.base() < lhs.base();
}

template <typename It1, typename It2>
constexpr std::strong_ordering operator<=>(const WrappedIterator<It1>& rhs,
                                           const WrappedIterator<It2>& lhs) noexcept {
  if constexpr (std::three_way_comparable_with<It1, It2, std::strong_ordering>) {
    return rhs.base() <=> lhs.base();
  } else {
    if (rhs.base() < lhs.base()){
      return std::strong_ordering::less;}

    if (rhs.base() == lhs.base()){
      return std::strong_ordering::equal;}

    return std::strong_ordering::greater;
  }
}

template <typename It1, typename It2>
constexpr auto operator-(const WrappedIterator<It1>& rhs,
                         const WrappedIterator<It2>& lhs) noexcept
    -> decltype(rhs.base() - lhs.base()) {
  return rhs.base() - lhs.base();
}

template <typename It>
constexpr WrappedIterator<It> operator+(typename WrappedIterator<It>::difference_type n,
                                            WrappedIterator<It> it) noexcept {
  it += n;
  return it;
}
}  // namespace rsl::_impl

template <typename It>
struct std::pointer_traits<rsl::_impl::WrappedIterator<It> > {
  using pointer         = rsl::_impl::WrappedIterator<It>;
  using element_type    = typename std::pointer_traits<It>::element_type;
  using difference_type = typename std::pointer_traits<It>::difference_type;

  constexpr static element_type* to_address(pointer it) noexcept {
    return std::to_address(it.base());
  }
};