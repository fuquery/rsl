#include <rsl/experimental/tuple>
#include <common/assertions.h>
#include <gtest/gtest.h>

TEST(Tuple, make_tuple) {
  int i                           = 0;
  float j                         = 0;
  rsl::tuple<int, int&, float&> t = rsl::make_tuple(1, std::ref(i), std::ref(j));
  ASSERT_EQ(get<0>(t), 1);
  ASSERT_EQ(get<1>(t), 0);
  ASSERT_EQ(get<2>(t), 0);
  i = 2;
  j = 3.5;
  ASSERT_EQ(get<0>(t), 1);
  ASSERT_EQ(get<1>(t), 2);
  ASSERT_EQ(get<2>(t), 3.5);
  get<1>(t) = 0;
  get<2>(t) = 0;
  ASSERT_EQ(i, 0);
  ASSERT_EQ(j, 0);

  {
    constexpr auto t1   = rsl::make_tuple(0, 1, 3.14);
    constexpr int i1    = get<1>(t1);
    constexpr double d1 = get<2>(t1);
    ASSERT_EQ(i1, 1);
    ASSERT_EQ(d1, 3.14);
  }
}
