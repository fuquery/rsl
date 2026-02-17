#include <rsl/tuple>
#include <common/assertions.h>
#include <common/types.h>
#include <gtest/gtest.h>
#include <utility>
int DefaultOnly::count = 0;
TEST(Tuple, constructor_lazy_sfinae) {
  {
    NonConstCopyable v(42);
    BlowsUpOnConstCopy<int> b;
    rsl::tuple<NonConstCopyable, BlowsUpOnConstCopy<int>> t(v, b);
    ASSERT_EQ(rsl::get<0>(t).value, 42);
  }
}

template <std::size_t... I>
constexpr bool CreateTuple(rsl::index_sequence<I...>) {
  using LargeTuple  = rsl::tuple<std::integral_constant<std::size_t, I>...>;
  using TargetTuple = rsl::tuple<decltype(I)...>;
  LargeTuple tuple(std::integral_constant<std::size_t, I>{}...);
  static_assert(get<0>(tuple).value == 0);
  static_assert(get<sizeof...(I) - 1>(tuple).value == sizeof...(I) - 1);

  TargetTuple t1 = tuple;  // converting copy constructor from &
  TargetTuple t2 =
      static_cast<LargeTuple const&>(tuple);  // converting copy constructor from const&
  TargetTuple t3 = std::move(tuple);          // converting rvalue constructor
  TargetTuple t4 = static_cast<LargeTuple const&&>(tuple);  // converting const rvalue constructor
  TargetTuple t5;                                           // default constructor
  (void)t1;
  (void)t2;
  (void)t3;
  (void)t4;
  (void)t5;

  t1 = tuple;                                   // converting assignment from &
  t1 = static_cast<LargeTuple const&>(tuple);   // converting assignment from const&
  t1 = std::move(tuple);                        // converting assignment from &&
  t1 = static_cast<LargeTuple const&&>(tuple);  // converting assignment from const&&
  std::swap(t1, t2);                            // swap

  return true;
}

// TEST(Tuple, constructor_recursion_depth) {
//   static_assert(CreateTuple(rsl::make_index_sequence<512>{}));
// }

template <class Elem>
void test_sfinae() {
  using Tup   = rsl::tuple<Elem>;
  using Alloc = std::allocator<int>;
  using Tag   = std::allocator_arg_t;
  // TODO : FIXME
  // special members
  {
    static_assert(std::is_default_constructible<Tup>::value, "");
    static_assert(std::is_move_constructible<Tup>::value, "");
    static_assert(!std::is_copy_constructible<Tup>::value, "");
    static_assert(!std::is_constructible<Tup, Tup&>::value, "");
  }
  // args constructors
  {
    static_assert(std::is_constructible<Tup, Elem&&>::value, "");
    static_assert(!std::is_constructible<Tup, Elem const&>::value, "");
    static_assert(!std::is_constructible<Tup, Elem&>::value, "");
  }
  // uses-allocator special member constructors
  {
    // static_assert(std::is_constructible<Tup, Tag, Alloc>::value, "");
    // static_assert(std::is_constructible<Tup, Tag, Alloc, Tup&&>::value, "");
    // static_assert(!std::is_constructible<Tup, Tag, Alloc, Tup const&>::value, "");
    // static_assert(!std::is_constructible<Tup, Tag, Alloc, Tup&>::value, "");
  }
  // uses-allocator args constructors
  {
    // static_assert(std::is_constructible<Tup, Tag, Alloc, Elem&&>::value, "");
    // static_assert(!std::is_constructible<Tup, Tag, Alloc, Elem const&>::value, "");
    // static_assert(!std::is_constructible<Tup, Tag, Alloc, Elem&>::value, "");
  }
}

TEST(Tuple, constructor_move) {
  {
    using T = rsl::tuple<>;
    T t0;
    T t = std::move(t0);
    ((void)t);  // Prevent unused warning
  }
  {
    using T = rsl::tuple<MoveOnly>;
    T t0(MoveOnly(0));
    T t = std::move(t0);
    ASSERT_EQ(get<0>(t).val, 0);
  }
  {
    using T = rsl::tuple<MoveOnly, MoveOnly>;
    T t0(MoveOnly(0), MoveOnly(1));
    T t = std::move(t0);
    ASSERT_EQ(get<0>(t).val, 0);
    ASSERT_EQ(get<1>(t).val, 1);
  }
  {
    using T = rsl::tuple<MoveOnly, MoveOnly, MoveOnly>;
    T t0(MoveOnly(0), MoveOnly(1), MoveOnly(2));
    T t = std::move(t0);
    ASSERT_EQ(get<0>(t).val, 0);
    ASSERT_EQ(get<1>(t).val, 1);
    ASSERT_EQ(get<2>(t).val, 2);
  }
  {
    test_sfinae<MoveOnlyEbo>();
    test_sfinae<MoveOnlyLarge>();
  }
}

TEST(Tuple, constructor_trivially_constructible) {
  static_assert(std::is_trivially_constructible<rsl::tuple<>>::value, "");
}

TEST(Tuple, dtor_trivially_constructible) {
  static_assert(!std::is_trivially_destructible<TrackDtor>::value, "");

  static_assert(std::is_trivially_destructible<rsl::tuple<>>::value, "");
  static_assert(std::is_trivially_destructible<rsl::tuple<void*>>::value, "");
  static_assert(std::is_trivially_destructible<rsl::tuple<int, float>>::value, "");
  static_assert(!std::is_trivially_destructible<rsl::tuple<std::string>>::value, "");
  static_assert(!std::is_trivially_destructible<rsl::tuple<int, std::string>>::value, "");

  {
    int count = 0;
    {
      rsl::tuple<TrackDtor> tuple{TrackDtor(&count)};
      ASSERT_EQ(count, 0);
    }
    ASSERT_EQ(count, 1);
  }
}

TEST(Tuple, dtor_default) {
  {
    rsl::tuple<> t;
    (void)t;
  }
  {
    rsl::tuple<int> t;
    ASSERT_EQ(get<0>(t), 0);
  }
  {
    rsl::tuple<int, char*> t;
    ASSERT_EQ(get<0>(t), 0);
    ASSERT_EQ(get<1>(t), nullptr);
  }
  {
    rsl::tuple<int, char*, std::string> t;
    ASSERT_EQ(get<0>(t), 0);
    ASSERT_EQ(get<1>(t), nullptr);
    ASSERT_EQ(get<2>(t), "");
  }
  {
    rsl::tuple<int, char*, std::string, DefaultOnly> t;
    ASSERT_EQ(get<0>(t), 0);
    ASSERT_EQ(get<1>(t), nullptr);
    ASSERT_EQ(get<2>(t), "");
    ASSERT_EQ(get<3>(t), DefaultOnly());
  }
  {
    // See bug #21157.
    static_assert(!std::is_default_constructible<rsl::tuple<NoDefault>>(), "");
    static_assert(!std::is_default_constructible<rsl::tuple<DefaultOnly, NoDefault>>(), "");
    static_assert(!std::is_default_constructible<rsl::tuple<NoDefault, DefaultOnly, NoDefault>>(),
                  "");
  }
  {
    static_assert(noexcept(rsl::tuple<NoExceptDefault>()), "");
    static_assert(noexcept(rsl::tuple<NoExceptDefault, NoExceptDefault>()), "");

    static_assert(!noexcept(rsl::tuple<ThrowingDefault, NoExceptDefault>()), "");
    static_assert(!noexcept(rsl::tuple<NoExceptDefault, ThrowingDefault>()), "");
    static_assert(!noexcept(rsl::tuple<ThrowingDefault, ThrowingDefault>()), "");
  }
  {
    constexpr rsl::tuple<> t;
    (void)t;
  }
  {
    constexpr rsl::tuple<int> t{42};
    ASSERT_EQ(get<0>(t), 42);
  }
  {
    constexpr rsl::tuple<int> t{};
    ASSERT_EQ(get<0>(t), 0);
  }
  {
    constexpr rsl::tuple<int, char*> t{};
    ASSERT_EQ(get<0>(t), 0);
    ASSERT_EQ(get<1>(t), nullptr);
  }
  {
    // Not supported
    // constexpr rsl::tuple<int> t;
    // ASSERT_EQ(get<0>(t), 0);
  }
  {
    // Not supported
    // constexpr rsl::tuple<int, char*> t;
    // ASSERT_EQ(get<0>(t), 0);
    // ASSERT_EQ(get<1>(t), nullptr);
  }
  {
    // Check that the SFINAE on the default constructor is not evaluated when
    // it isn't needed. If the default constructor is evaluated then this test
    // should fail to compile.
    IllFormedDefault v(0);
    rsl::tuple<IllFormedDefault> t(v);
  }
  {
    struct Base {};
    struct Derived : Base {
    protected:
      Derived() = default;
    };
    static_assert(!std::is_default_constructible<rsl::tuple<Derived, int>>::value, "");
  }
}

TEST(Tuple, constructor_copy) {
  {
    using T = rsl::tuple<>;
    T t0;
    T t = t0;
    ((void)t);  // Prevent unused warning
  }
  {
    using T = rsl::tuple<int>;
    T t0(2);
    T t = t0;
    ASSERT_EQ(get<0>(t), 2);
  }
  {
    using T = rsl::tuple<int, char>;
    T t0(2, 'a');
    T t = t0;
    ASSERT_EQ(get<0>(t), 2);
    ASSERT_EQ(get<1>(t), 'a');
  }
  {
    using T = rsl::tuple<int, char, std::string>;
    const T t0(2, 'a', "some text");
    T t = t0;
    ASSERT_EQ(get<0>(t), 2);
    ASSERT_EQ(get<1>(t), 'a');
    ASSERT_EQ(get<2>(t), "some text");
  }
  {
    using T = rsl::tuple<int>;
    constexpr T t0(2);
    constexpr T t = t0;
    static_assert(get<0>(t) == 2, "");
  }
  {
    using T = rsl::tuple<Empty>;
    constexpr T t0;
    constexpr T t     = t0;
    constexpr Empty e = get<0>(t);
    ((void)e);  // Prevent unused warning
  }
}

TEST(Tuple, constructor_convert_non_const_copy) {
  // test implicit conversions.
  // sizeof...(Types) == 1
  {
    rsl::tuple<MutableCopy> t1{1};
    rsl::tuple<ConvertibleFrom<MutableCopy>> t2 = t1;
    ASSERT_EQ(get<0>(t2).v.val, 1);
  }

  // test implicit conversions.
  // sizeof...(Types) > 1
  {
    rsl::tuple<float, int> t1{1, 2};
    rsl::tuple<double, int> t2{t1};
    rsl::tuple<double, int> t3 = t1;
    ASSERT_EQ(get<1>(t2), 2);
  }
  {
    rsl::tuple<MutableCopy, int> t1{1, 2};
    rsl::tuple<ConvertibleFrom<MutableCopy>, int> t2{t1};
    rsl::tuple<ConvertibleFrom<MutableCopy>, int> t3 = t1;
    ASSERT_EQ(get<0>(t2).v.val, 1);
    ASSERT_EQ(get<1>(t2), 2);
  }

  // test explicit conversions.
  // sizeof...(Types) == 1
  {
    rsl::tuple<MutableCopy> t1{1};
    rsl::tuple<ExplicitConstructibleFrom<MutableCopy>> t2{t1};
    ASSERT_EQ(get<0>(t2).v.val, 1);
  }

  // test explicit conversions.
  // sizeof...(Types) > 1
  {
    rsl::tuple<MutableCopy, int> t1{1, 2};
    rsl::tuple<ExplicitConstructibleFrom<MutableCopy>, int> t2{t1};
    ASSERT_EQ(get<0>(t2).v.val, 1);
    ASSERT_EQ(get<1>(t2), 2);
  }

  // test constraints

  // sizeof...(Types) != sizeof...(UTypes)
  static_assert(!std::is_constructible_v<rsl::tuple<int, int>, rsl::tuple<int>&>);
  static_assert(!std::is_constructible_v<rsl::tuple<int>, rsl::tuple<int, int>&>);
  static_assert(!std::is_constructible_v<rsl::tuple<int, int, int>, rsl::tuple<int, int>&>);

  // !(is_constructible_v<Types, decltype(get<I>(FWD(u)))> && ...)
  static_assert(
      !std::is_constructible_v<rsl::tuple<int, NoConstructorFromInt>, rsl::tuple<int, int>&>);

  // sizeof...(Types) == 1 && other branch of "||" satisfied
  {
    rsl::tuple<TracedCopyMove> t1{};
    rsl::tuple<ConvertibleFrom<TracedCopyMove>> t2{t1};
    ASSERT_TRUE(non_const_copy_constructor_called(get<0>(t2).v));
  }

  // sizeof...(Types) == 1 && is_same_v<T, U>
  {
    rsl::tuple<TracedCopyMove> t1{};
    rsl::tuple<TracedCopyMove> t2{t1};
    ASSERT_FALSE(non_const_copy_constructor_called(get<0>(t2)));
  }

  // sizeof...(Types) != 1
  {
    rsl::tuple<TracedCopyMove, TracedCopyMove> t1{};
    rsl::tuple<TracedCopyMove, TracedCopyMove> t2{t1};
    ASSERT_TRUE(non_const_copy_constructor_called(get<0>(t2)));
  }
}

namespace constructor_convert_move {

struct B {
  int id_;

  explicit B(int i) : id_(i) {}
  B(const B&)            = default;
  B& operator=(const B&) = default;
  virtual ~B() {}
};

struct D : B {
  explicit D(int i) : B(i) {}
};

}  // namespace constructor_convert_move

TEST(Tuple, constructor_convert_move) {
  {
    using T0 = rsl::tuple<long>;
    using T1 = rsl::tuple<long long>;
    T0 t0(2);
    T1 t1 = std::move(t0);
    ASSERT_EQ(get<0>(t1), 2);
  }
  {
    using T0 = rsl::tuple<long, char>;
    using T1 = rsl::tuple<long long, int>;
    T0 t0(2, 'a');
    T1 t1 = std::move(t0);
    ASSERT_EQ(get<0>(t1), 2);
    ASSERT_EQ(get<1>(t1), int('a'));
  }
  {
    using T0 = rsl::tuple<long, char, constructor_convert_move::D>;
    using T1 = rsl::tuple<long long, int, constructor_convert_move::B>;
    T0 t0(2, 'a', constructor_convert_move::D(3));
    T1 t1 = std::move(t0);
    ASSERT_EQ(get<0>(t1), 2);
    ASSERT_EQ(get<1>(t1), int('a'));
    ASSERT_EQ(get<2>(t1).id_, 3);
  }
  {
    constructor_convert_move::D d(3);
    using T0 = rsl::tuple<long, char, constructor_convert_move::D&>;
    using T1 = rsl::tuple<long long, int, constructor_convert_move::B&>;
    T0 t0(2, 'a', d);
    T1 t1 = std::move(t0);
    d.id_ = 2;
    ASSERT_EQ(get<0>(t1), 2);
    ASSERT_EQ(get<1>(t1), int('a'));
    ASSERT_EQ(get<2>(t1).id_, 2);
  }
  {
    using T0 = rsl::tuple<long, char, std::unique_ptr<constructor_convert_move::D>>;
    using T1 = rsl::tuple<long long, int, std::unique_ptr<constructor_convert_move::B>>;
    T0 t0(2, 'a', std::unique_ptr<constructor_convert_move::D>(new constructor_convert_move::D(3)));
    T1 t1 = std::move(t0);
    ASSERT_EQ(get<0>(t1), 2);
    ASSERT_EQ(get<1>(t1), int('a'));
    ASSERT_EQ(get<2>(t1)->id_, 3);
  }
  {
    rsl::tuple<int> t1(42);
    rsl::tuple<Explicit> t2(std::move(t1));
    ASSERT_EQ(get<0>(t2).value, 42);
  }
  {
    rsl::tuple<int> t1(42);
    rsl::tuple<Implicit> t2 = std::move(t1);
    ASSERT_EQ(get<0>(t2).value, 42);
  }
}

namespace constructor_convert_copy {

struct B {
  int id_;

  explicit B(int i) : id_(i) {}
};

struct D : B {
  explicit D(int i) : B(i) {}
};

struct A {
  int id_;

  constexpr A(int i) : id_(i) {}
  friend constexpr bool operator==(const A& x, const A& y) { return x.id_ == y.id_; }
};

struct C {
  int id_;

  constexpr explicit C(int i) : id_(i) {}
  friend constexpr bool operator==(const C& x, const C& y) { return x.id_ == y.id_; }
};

}  // namespace constructor_convert_copy

TEST(Tuple, constructor_convert_copy) {
  {
    using T0 = rsl::tuple<long>;
    using T1 = rsl::tuple<long long>;
    T0 t0(2);
    T1 t1 = t0;
    ASSERT_EQ(get<0>(t1), 2);
  }
  {
    using T0 = rsl::tuple<int>;
    using T1 = rsl::tuple<constructor_convert_copy::A>;
    constexpr T0 t0(2);
    constexpr T1 t1 = t0;
    static_assert(get<0>(t1) == 2, "");
  }
  {
    using T0 = rsl::tuple<int>;
    using T1 = rsl::tuple<constructor_convert_copy::C>;
    constexpr T0 t0(2);
    constexpr T1 t1{t0};
    static_assert(get<0>(t1) == constructor_convert_copy::C(2), "");
  }
  {
    using T0 = rsl::tuple<long, char>;
    using T1 = rsl::tuple<long long, int>;
    T0 t0(2, 'a');
    T1 t1 = t0;
    ASSERT_EQ(get<0>(t1), 2);
    ASSERT_EQ(get<1>(t1), int('a'));
  }
  {
    using T0 = rsl::tuple<long, char, constructor_convert_copy::D>;
    using T1 = rsl::tuple<long long, int, constructor_convert_copy::B>;
    T0 t0(2, 'a', constructor_convert_copy::D(3));
    T1 t1 = t0;
    ASSERT_EQ(get<0>(t1), 2);
    ASSERT_EQ(get<1>(t1), int('a'));
    ASSERT_EQ(get<2>(t1).id_, 3);
  }
  {
    constructor_convert_copy::D d(3);
    using T0 = rsl::tuple<long, char, constructor_convert_copy::D&>;
    using T1 = rsl::tuple<long long, int, constructor_convert_copy::B&>;
    T0 t0(2, 'a', d);
    T1 t1 = t0;
    d.id_ = 2;
    ASSERT_EQ(get<0>(t1), 2);
    ASSERT_EQ(get<1>(t1), int('a'));
    ASSERT_EQ(get<2>(t1).id_, 2);
  }
  {
    using T0 = rsl::tuple<long, char, int>;
    using T1 = rsl::tuple<long long, int, constructor_convert_copy::B>;
    T0 t0(2, 'a', 3);
    T1 t1(t0);
    ASSERT_EQ(get<0>(t1), 2);
    ASSERT_EQ(get<1>(t1), int('a'));
    ASSERT_EQ(get<2>(t1).id_, 3);
  }
  {
    const std::tuple<int> t1(42);
    std::tuple<Explicit> t2(t1);
    ASSERT_EQ(get<0>(t2).value, 42);
  }
  {
    const std::tuple<int> t1(42);
    std::tuple<Implicit> t2 = t1;
    ASSERT_EQ(get<0>(t2).value, 42);
  }
  {
    static_assert(std::is_convertible<ExplicitTwo&&, ExplicitTwo>::value, "");
    static_assert(
        std::is_convertible<std::tuple<ExplicitTwo&&>&&, const std::tuple<ExplicitTwo>&>::value,
        "");
    ExplicitTwo e;
    std::tuple<ExplicitTwo> t = std::tuple<ExplicitTwo&&>(std::move(e));
    ((void)t);
  }
}

TEST(Tuple, constructor_convert_const_move) {
  // test implicit conversions.
  // sizeof...(Types) == 1
  {
    const rsl::tuple<ConstMove> t1{1};
    rsl::tuple<ConvertibleFrom<ConstMove>> t2 = std::move(t1);
    ASSERT_EQ(get<0>(t2).v.val, 1);
  }

  // test implicit conversions.
  // sizeof...(Types) > 1
  {
    const rsl::tuple<ConstMove, int> t1{1, 2};
    rsl::tuple<ConvertibleFrom<ConstMove>, int> t2 = std::move(t1);
    ASSERT_EQ(get<0>(t2).v.val, 1);
    ASSERT_EQ(get<1>(t2), 2);
  }

  // test explicit conversions.
  // sizeof...(Types) == 1
  {
    const rsl::tuple<ConstMove> t1{1};
    rsl::tuple<ExplicitConstructibleFrom<ConstMove>> t2{std::move(t1)};
    ASSERT_EQ(get<0>(t2).v.val, 1);
  }

  // test explicit conversions.
  // sizeof...(Types) > 1
  {
    const rsl::tuple<ConstMove, int> t1{1, 2};
    rsl::tuple<ExplicitConstructibleFrom<ConstMove>, int> t2{std::move(t1)};
    ASSERT_EQ(get<0>(t2).v.val, 1);
    ASSERT_EQ(get<1>(t2), 2);
  }

  // test constraints

  // sizeof...(Types) != sizeof...(UTypes)
  static_assert(!std::is_constructible_v<rsl::tuple<int, int>, const rsl::tuple<int>&&>);
  static_assert(!std::is_constructible_v<rsl::tuple<int, int, int>, const rsl::tuple<int, int>&&>);

  // !(is_constructible_v<Types, decltype(get<I>(FWD(u)))> && ...)
  static_assert(!std::is_constructible_v<rsl::tuple<int, NoConstructorFromInt>,
                                         const rsl::tuple<int, int>&&>);

  // sizeof...(Types) == 1 && other branch of "||" satisfied
  {
    const rsl::tuple<TracedCopyMove> t1{};
    rsl::tuple<ConvertibleFrom<TracedCopyMove>> t2{std::move(t1)};
    ASSERT_TRUE(const_move_constructor_called(get<0>(t2).v));
  }

  // sizeof...(Types) == 1 && is_same_v<T, U>
  {
    const rsl::tuple<TracedCopyMove> t1{};
    rsl::tuple<TracedCopyMove> t2{t1};
    ASSERT_TRUE(!const_move_constructor_called(get<0>(t2)));
  }

  // sizeof...(Types) != 1
  {
    const rsl::tuple<TracedCopyMove, TracedCopyMove> t1{};
    rsl::tuple<TracedCopyMove, TracedCopyMove> t2{std::move(t1)};
    ASSERT_TRUE(const_move_constructor_called(get<0>(t2)));
  }

  // sizeof...(Types) == 1 && is_convertible_v<decltype(u), T>
  {
    const rsl::tuple<CvtFromConstTupleRefRef> t1{};
    rsl::tuple<ConvertibleFrom<CvtFromConstTupleRefRef>> t2{std::move(t1)};
    ASSERT_TRUE(!const_move_constructor_called(get<0>(t2).v));
  }

  // sizeof...(Types) == 1 && is_constructible_v<decltype(u), T>
  {
    const rsl::tuple<ExplicitCtrFromConstTupleRefRef> t1{};
    rsl::tuple<ConvertibleFrom<ExplicitCtrFromConstTupleRefRef>> t2{std::move(t1)};
    ASSERT_TRUE(!const_move_constructor_called(get<0>(t2).v));
  }
}
