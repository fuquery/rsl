#include <rsl/experimental/tuple>
#include <common/assertions.h>
#include <common/types.h>
#include <gtest/gtest.h>

namespace {

struct B {
  int id_;
  explicit B(int i = 0) : id_(i) {}
  B(const B&)            = default;
  B& operator=(const B&) = default;
  virtual ~B() {}
};

struct D : B {
  explicit D(int i) : B(i) {}
};

}  // namespace

TEST(Tuple, Assign_traits) {
  ASSERT_FALSE((std::is_assignable_v<const rsl::tuple<int>&, const rsl::tuple<int>&>));
  ASSERT_TRUE((std::is_assignable_v<const rsl::tuple<int&>&, const rsl::tuple<int&>&>));
  ASSERT_TRUE((std::is_assignable_v<const rsl::tuple<int&, int&>&, const rsl::tuple<int&, int&>&>));
  ASSERT_FALSE((std::is_assignable_v<const rsl::tuple<int&, int>&, const rsl::tuple<int&, int>&>));
}

TEST(Tuple, Assign) {
  {
    using T = rsl::tuple<>;
    T t0;
    T t;
    t = t0;
  }
  {
    using T = rsl::tuple<int&, int&&>;
    int x   = 42;
    int y   = 100;
    int x2  = -1;
    int y2  = 500;
    T t(x, std::move(y));
    T t2(x2, std::move(y2));
    t = t2;
    ASSERT_EQ(get<0>(t), x2);
    ASSERT_EQ(&get<0>(t), &x);
    ASSERT_EQ(get<1>(t), y2);
    ASSERT_EQ(&get<1>(t), &y);
  }
  {
    int i1    = 1;
    int i2    = 2;
    double d1 = 3.0;
    double d2 = 5.0;
    rsl::tuple<int&, double&> t1{i1, d1};
    const rsl::tuple<int&, double&> t2{i2, d2};
    t2 = std::move(t1);
    ASSERT_EQ(i1, 1);
    ASSERT_EQ(i2, 1);
    ASSERT_EQ(d1, 3.0);
    ASSERT_EQ(d2, 3.0);
    ASSERT_EQ(get<0>(t2), 1);
    ASSERT_EQ(get<1>(t2), 3.0);
  }
  {
    using T = rsl::tuple<MoveOnly>;
    T t0(MoveOnly(0));
    T t;
    t = std::move(t0);
    ASSERT_EQ(get<0>(t).val, 0);
  }
  {
    const rsl::tuple<ConstCopyAssign> t1{1};
    const rsl::tuple<ConstCopyAssign> t2{2};
    t2 = t1;
    ASSERT_EQ(get<0>(t2).val, 1);
  }
  {
    rsl::tuple<ConstMoveAssign> t1{1};
    const rsl::tuple<ConstMoveAssign> t2{2};
    t2 = std::move(t1);
    ASSERT_EQ(get<0>(t2).val, 1);
  }
  {
    using T = rsl::tuple<MoveOnly, MoveOnly>;
    T t0(MoveOnly(0), MoveOnly(1));
    T t;
    t = std::move(t0);
    ASSERT_EQ(get<0>(t).val, 0);
    ASSERT_EQ(get<1>(t).val, 1);
  }
  {
    using T = rsl::tuple<MoveOnly, MoveOnly, MoveOnly>;
    T t0(MoveOnly(0), MoveOnly(1), MoveOnly(2));
    T t;
    t = std::move(t0);
    ASSERT_EQ(get<0>(t).val, 0);
    ASSERT_EQ(get<1>(t).val, 1);
    ASSERT_EQ(get<2>(t).val, 2);
  }
  {
    rsl::tuple<TracedAssignment, const TracedAssignment> t1{};
    const rsl::tuple<TracedAssignment, const TracedAssignment> t2{};
    t2 = t1;
    ASSERT_EQ(get<0>(t2).constCopyAssign, 1);
    ASSERT_EQ(get<1>(t2).constCopyAssign, 1);
  }
  {
    rsl::tuple<TracedAssignment, const TracedAssignment> t1{};
    const rsl::tuple<TracedAssignment, const TracedAssignment> t2{};
    t2 = std::move(t1);
    ASSERT_EQ(get<0>(t2).constMoveAssign, 1);
    ASSERT_EQ(get<1>(t2).constCopyAssign, 1);
  }
}

TEST(Tuple, ConvertingAssign) {
  {
    using T0 = rsl::tuple<long>;
    using T1 = rsl::tuple<long long>;
    T0 t0(2);
    T1 t1;
    t1 = t0;
    ASSERT_EQ(get<0>(t1), 2);
  }

  {
    using T0 = rsl::tuple<long, char>;
    using T1 = rsl::tuple<long long, int>;
    T0 t0(2, 'a');
    T1 t1;
    t1 = t0;
    ASSERT_EQ(get<0>(t1), 2);
    ASSERT_EQ(get<1>(t1), 'a');
  }
  {
    using T0 = rsl::tuple<long, char, D>;
    using T1 = rsl::tuple<long long, int, B>;
    T0 t0(2, 'a', D(3));
    T1 t1;
    t1 = t0;
    ASSERT_EQ(get<0>(t1), 2);
    ASSERT_EQ(get<1>(t1), 'a');
    ASSERT_EQ(get<2>(t1).id_, 3);
  }
  {
    D d(3);
    D d2(2);
    using T0 = rsl::tuple<long, char, D&>;
    using T1 = rsl::tuple<long long, int, B&>;
    T0 t0(2, 'a', d2);
    T1 t1(1, 'b', d);
    t1 = t0;
    ASSERT_EQ(get<0>(t1), 2);
    ASSERT_EQ(get<1>(t1), 'a');
    ASSERT_EQ(get<2>(t1).id_, 2);
  }
  {
    // Test that tuple evaluates correctly applies an lvalue reference
    // before evaluating is_assignable (i.e. 'is_assignable<int&, int&>')
    // instead of evaluating 'is_assignable<int&&, int&>' which is false.
    int x = 42;
    int y = 43;
    rsl::tuple<int&&> t(std::move(x));
    rsl::tuple<int&> t2(y);
    t = t2;
    ASSERT_EQ(get<0>(t), 43);
    ASSERT_EQ(&get<0>(t), &x);
  }
  {
    using T0 = rsl::tuple<long>;
    using T1 = rsl::tuple<long long>;
    T0 t0(2);
    T1 t1;
    t1 = std::move(t0);
    ASSERT_EQ(get<0>(t1), 2);
  }
  {
    using T0 = rsl::tuple<long, char>;
    using T1 = rsl::tuple<long long, int>;
    T0 t0(2, 'a');
    T1 t1;
    t1 = std::move(t0);
    ASSERT_EQ(get<0>(t1), 2);
    ASSERT_EQ(get<1>(t1), 'a');
  }

  {
    int i1  = 1;
    int i2  = 2;
    long j1 = 3;
    long j2 = 4;
    const rsl::tuple<int&, int&> t1{i1, i2};
    const rsl::tuple<long&, long&> t2{j1, j2};
    t2 = t1;
    ASSERT_EQ(get<0>(t2), 1);
    ASSERT_EQ(get<1>(t2), 2);
  }

  {
    const rsl::tuple<ConstCopyAssign> t1{1};
    const rsl::tuple<AssignableFrom<ConstCopyAssign>> t2{2};
    t2 = t1;
    ASSERT_EQ(get<0>(t2).v.val, 1);
  }

  {
    rsl::tuple<TracedAssignment> t1{};
    const rsl::tuple<AssignableFrom<TracedAssignment>> t2{};
    t2 = t1;
    ASSERT_EQ(get<0>(t2).v.constCopyAssign, 1);
  }

  {
    int i1  = 1;
    int i2  = 2;
    long j1 = 3;
    long j2 = 4;
    rsl::tuple<int&, int&> t1{i1, i2};
    const rsl::tuple<long&, long&> t2{j1, j2};
    t2 = std::move(t1);
    ASSERT_EQ(get<0>(t2), 1);
    ASSERT_EQ(get<1>(t2), 2);
  }

  {
    rsl::tuple<ConstMoveAssign> t1{1};
    const rsl::tuple<AssignableFrom<ConstMoveAssign>> t2{2};
    t2 = std::move(t1);
    ASSERT_EQ(get<0>(t2).v.val, 1);
  }

  {
    rsl::tuple<TracedAssignment> t1{};
    const rsl::tuple<AssignableFrom<TracedAssignment>> t2{};
    t2 = std::move(t1);
    ASSERT_EQ(get<0>(t2).v.constMoveAssign, 1);
  }
}
