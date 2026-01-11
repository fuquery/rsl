#pragma once
#include <type_traits>

struct Explicit {
  int value;
  explicit Explicit(int x) : value(x) {}
};

struct Implicit {
  int value;
  Implicit(int x) : value(x) {}
};

struct ExplicitTwo {
  ExplicitTwo() {}
  ExplicitTwo(ExplicitTwo const&) {}
  ExplicitTwo(ExplicitTwo&&) {}

  template <class T, typename = typename std::enable_if<!std::is_same<T, ExplicitTwo>::value>::type>
  explicit ExplicitTwo(T) {}
};

struct MoveOnly {
  MoveOnly() : val(0) {}
  MoveOnly(int v) : val(v) {}
  MoveOnly(MoveOnly&& other) noexcept : val(other.val) { other.val = -1; }
  MoveOnly& operator=(MoveOnly&& other) noexcept {
    val       = other.val;
    other.val = -1;
    return *this;
  }
  MoveOnly(const MoveOnly&)            = delete;
  MoveOnly& operator=(const MoveOnly&) = delete;

  int val;
};

template <class T>
struct AssignableFrom {
  T v;

  constexpr AssignableFrom() = default;

  template <class U>
  constexpr AssignableFrom(U&& u)
    requires std::is_constructible_v<T, U&&>
      : v(std::forward<U>(u)) {}

  constexpr AssignableFrom& operator=(const T& t)
    requires std::is_copy_assignable_v<T>
  {
    v = t;
    return *this;
  }

  constexpr AssignableFrom& operator=(T&& t)
    requires std::is_move_assignable_v<T>
  {
    v = std::move(t);
    return *this;
  }

  constexpr const AssignableFrom& operator=(const T& t) const
    requires std::is_assignable_v<const T&, const T&>
  {
    v = t;
    return *this;
  }

  constexpr const AssignableFrom& operator=(T&& t) const
    requires std::is_assignable_v<const T&, T&&>
  {
    v = std::move(t);
    return *this;
  }
};

struct ConstCopyAssign {
  mutable int val;

  constexpr ConstCopyAssign() = default;
  constexpr ConstCopyAssign(int v) : val(v) {}

  constexpr const ConstCopyAssign& operator=(const ConstCopyAssign& other) const {
    val = other.val;
    return *this;
  }

  constexpr ConstCopyAssign& operator=(const ConstCopyAssign&)        = delete;
  constexpr ConstCopyAssign& operator=(ConstCopyAssign&&)             = delete;
  constexpr const ConstCopyAssign& operator=(ConstCopyAssign&&) const = delete;
};

struct ConstMoveAssign {
  mutable int val;

  constexpr ConstMoveAssign() = default;
  constexpr ConstMoveAssign(int v) : val(v) {}

  constexpr const ConstMoveAssign& operator=(ConstMoveAssign&& other) const {
    val = other.val;
    return *this;
  }

  constexpr ConstMoveAssign& operator=(const ConstMoveAssign&)             = delete;
  constexpr const ConstMoveAssign& operator=(const ConstMoveAssign&) const = delete;
  constexpr ConstMoveAssign& operator=(ConstMoveAssign&&)                  = delete;
};

struct TracedAssignment {
  int copyAssign              = 0;
  mutable int constCopyAssign = 0;
  int moveAssign              = 0;
  mutable int constMoveAssign = 0;

  constexpr TracedAssignment() = default;

  constexpr TracedAssignment& operator=(const TracedAssignment&) {
    copyAssign++;
    return *this;
  }
  constexpr const TracedAssignment& operator=(const TracedAssignment&) const {
    constCopyAssign++;
    return *this;
  }
  constexpr TracedAssignment& operator=(TracedAssignment&&) {
    moveAssign++;
    return *this;
  }
  constexpr const TracedAssignment& operator=(TracedAssignment&&) const {
    constMoveAssign++;
    return *this;
  }
};

// move_only type which triggers the empty base optimization
struct MoveOnlyEbo {
  MoveOnlyEbo()              = default;
  MoveOnlyEbo(MoveOnlyEbo&&) = default;
};

// a move_only type which does not trigger the empty base optimization
struct MoveOnlyLarge final {
  MoveOnlyLarge() : value(42) {}
  MoveOnlyLarge(MoveOnlyLarge&&) = default;
  int value;
};

struct TrackDtor {
  int* count_;
  constexpr explicit TrackDtor(int* count) : count_(count) {}
  constexpr TrackDtor(TrackDtor&& that) : count_(that.count_) { that.count_ = nullptr; }
  constexpr ~TrackDtor() {
    if (count_)
      ++*count_;
  }
};

struct NoDefault {
  NoDefault() = delete;
  explicit NoDefault(int) {}
};

struct NoExceptDefault {
  NoExceptDefault() noexcept = default;
};

struct ThrowingDefault {
  ThrowingDefault() {}
};

struct IllFormedDefault {
  IllFormedDefault(int x) : value(x) {}
  template <bool Pred = false>
  constexpr IllFormedDefault() {
    static_assert(Pred, "The default constructor should not be instantiated");
  }
  int value;
};

class DefaultOnly {
  int data_;

  DefaultOnly(const DefaultOnly&);
  DefaultOnly& operator=(const DefaultOnly&);

public:
  static int count;

  DefaultOnly() : data_(-1) { ++count; }
  ~DefaultOnly() {
    data_ = 0;
    --count;
  }

  friend bool operator==(const DefaultOnly& x, const DefaultOnly& y) { return x.data_ == y.data_; }
  friend bool operator<(const DefaultOnly& x, const DefaultOnly& y) { return x.data_ < y.data_; }
};

struct NonConstCopyable {
  NonConstCopyable() = default;
  explicit NonConstCopyable(int v) : value(v) {}
  NonConstCopyable(NonConstCopyable&)       = default;
  NonConstCopyable(NonConstCopyable const&) = delete;
  int value;
};

template <typename T>
struct BlowsUpOnConstCopy {
  BlowsUpOnConstCopy() = default;
  constexpr BlowsUpOnConstCopy(BlowsUpOnConstCopy const&) {
    static_assert(!std::is_same<T, T>::value, "");
  }
  BlowsUpOnConstCopy(BlowsUpOnConstCopy&) = default;
};

struct Empty {};

struct MutableCopy {
  int val;
  bool alloc_constructed{false};

  constexpr MutableCopy() = default;
  constexpr MutableCopy(int _val) : val(_val) {}
  constexpr MutableCopy(MutableCopy&)       = default;
  constexpr MutableCopy(const MutableCopy&) = delete;
};

struct ConstCopy {
  int val;
  bool alloc_constructed{false};

  constexpr ConstCopy() = default;
  constexpr ConstCopy(int _val) : val(_val) {}
  constexpr ConstCopy(const ConstCopy&) = default;
  constexpr ConstCopy(ConstCopy&)       = delete;
};

struct MutableMove {
  int val;
  bool alloc_constructed{false};

  constexpr MutableMove() = default;
  constexpr MutableMove(int _val) : val(_val) {}
  constexpr MutableMove(MutableMove&&)       = default;
  constexpr MutableMove(const MutableMove&&) = delete;
};

struct ConstMove {
  int val;
  bool alloc_constructed{false};

  constexpr ConstMove() = default;
  constexpr ConstMove(int _val) : val(_val) {}
  constexpr ConstMove(const ConstMove&& o) : val(o.val) {}
  constexpr ConstMove(ConstMove&&) = delete;
};

template <typename T>
struct ConvertibleFrom {
  T v;
  bool alloc_constructed{false};

  constexpr ConvertibleFrom() = default;
  constexpr ConvertibleFrom(T& _v)
    requires(std::is_constructible_v<T, T&>)
      : v(_v) {}
  constexpr ConvertibleFrom(const T& _v)
    requires(std::is_constructible_v<T, const T&> && !std::is_const_v<T>)
      : v(_v) {}
  constexpr ConvertibleFrom(T&& _v)
    requires(std::is_constructible_v<T, T &&>)
      : v(std::move(_v)) {}
  constexpr ConvertibleFrom(const T&& _v)
    requires(std::is_constructible_v<T, const T &&> && !std::is_const_v<T>)
      : v(std::move(_v)) {}
};

template <typename T>
struct ExplicitConstructibleFrom {
  T v;
  bool alloc_constructed{false};

  constexpr explicit ExplicitConstructibleFrom() = default;
  constexpr explicit ExplicitConstructibleFrom(T& _v)
    requires(std::is_constructible_v<T, T&>)
      : v(_v) {}
  constexpr explicit ExplicitConstructibleFrom(const T& _v)
    requires(std::is_constructible_v<T, const T&> && !std::is_const_v<T>)
      : v(_v) {}
  constexpr explicit ExplicitConstructibleFrom(T&& _v)
    requires(std::is_constructible_v<T, T &&>)
      : v(std::move(_v)) {}
  constexpr explicit ExplicitConstructibleFrom(const T&& _v)
    requires(std::is_constructible_v<T, const T &&> && !std::is_const_v<T>)
      : v(std::move(_v)) {}
};

struct TracedCopyMove {
  int nonConstCopy       = 0;
  int constCopy          = 0;
  int nonConstMove       = 0;
  int constMove          = 0;
  bool alloc_constructed = false;

  constexpr TracedCopyMove() = default;
  constexpr TracedCopyMove(const TracedCopyMove& other)
      : nonConstCopy(other.nonConstCopy)
      , constCopy(other.constCopy + 1)
      , nonConstMove(other.nonConstMove)
      , constMove(other.constMove) {}
  constexpr TracedCopyMove(TracedCopyMove& other)
      : nonConstCopy(other.nonConstCopy + 1)
      , constCopy(other.constCopy)
      , nonConstMove(other.nonConstMove)
      , constMove(other.constMove) {}

  constexpr TracedCopyMove(TracedCopyMove&& other)
      : nonConstCopy(other.nonConstCopy)
      , constCopy(other.constCopy)
      , nonConstMove(other.nonConstMove + 1)
      , constMove(other.constMove) {}

  constexpr TracedCopyMove(const TracedCopyMove&& other)
      : nonConstCopy(other.nonConstCopy)
      , constCopy(other.constCopy)
      , nonConstMove(other.nonConstMove)
      , constMove(other.constMove + 1) {}
};

// If the constructor tuple(tuple<UTypes...>&) is not available,
// the fallback call to `tuple(const tuple&) = default;` or any other
// constructor that takes const ref would increment the constCopy.
inline constexpr bool non_const_copy_constructor_called(const TracedCopyMove& obj) {
  return obj.nonConstCopy == 1 && obj.constCopy == 0 && obj.constMove == 0 && obj.nonConstMove == 0;
}

// If the constructor tuple(const tuple<UTypes...>&&) is not available,
// the fallback call to `tuple(const tuple&) = default;` or any other
// constructor that takes const ref would increment the constCopy.
inline constexpr bool const_move_constructor_called(const TracedCopyMove& obj) {
  return obj.nonConstMove == 0 && obj.constMove == 1 && obj.constCopy == 0 && obj.nonConstCopy == 0;
}

struct NoConstructorFromInt {};

struct CvtFromTupleRef : TracedCopyMove {
  constexpr CvtFromTupleRef() = default;
  constexpr CvtFromTupleRef(std::tuple<CvtFromTupleRef>& other)
      : TracedCopyMove(static_cast<TracedCopyMove&>(std::get<0>(other))) {}
};

struct ExplicitCtrFromTupleRef : TracedCopyMove {
  constexpr explicit ExplicitCtrFromTupleRef() = default;
  constexpr explicit ExplicitCtrFromTupleRef(std::tuple<ExplicitCtrFromTupleRef>& other)
      : TracedCopyMove(static_cast<TracedCopyMove&>(std::get<0>(other))) {}
};

struct CvtFromConstTupleRefRef : TracedCopyMove {
  constexpr CvtFromConstTupleRefRef() = default;
  constexpr CvtFromConstTupleRefRef(const std::tuple<CvtFromConstTupleRefRef>&& other)
      : TracedCopyMove(static_cast<const TracedCopyMove&&>(std::get<0>(other))) {}
};

struct ExplicitCtrFromConstTupleRefRef : TracedCopyMove {
  constexpr explicit ExplicitCtrFromConstTupleRefRef() = default;
  constexpr explicit ExplicitCtrFromConstTupleRefRef(
      std::tuple<const ExplicitCtrFromConstTupleRefRef>&& other)
      : TracedCopyMove(static_cast<const TracedCopyMove&&>(std::get<0>(other))) {}
};

template <typename T>
void conversion_test(T);

template <typename T, typename... Args>
concept ImplicitlyConstructible =
    requires(Args&&... args) { conversion_test<T>({std::forward<Args>(args)...}); };

struct CopyAssign {
  int val;

  constexpr CopyAssign() = default;
  constexpr CopyAssign(int v) : val(v) {}

  constexpr CopyAssign& operator=(const CopyAssign&) = default;

  constexpr const CopyAssign& operator=(const CopyAssign&) const = delete;
  constexpr CopyAssign& operator=(CopyAssign&&)                  = delete;
  constexpr const CopyAssign& operator=(CopyAssign&&) const      = delete;
};

struct MoveAssign {
  int val;

  constexpr MoveAssign() = default;
  constexpr MoveAssign(int v) : val(v) {}

  constexpr MoveAssign& operator=(MoveAssign&&) = default;

  constexpr MoveAssign& operator=(const MoveAssign&)             = delete;
  constexpr const MoveAssign& operator=(const MoveAssign&) const = delete;
  constexpr const MoveAssign& operator=(MoveAssign&&) const      = delete;
};
