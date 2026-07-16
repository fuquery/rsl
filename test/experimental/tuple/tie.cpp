#include <rsl/experimental/tuple>
#include <common/assertions.h>
#include <gtest/gtest.h>

#include <concepts>

TEST(Tuple, Tie) {
  int i         = 42;
  double f      = 1.1;
  using ExpectT = rsl::tuple<int&, decltype(rsl::ignore)&, double&>;
  auto res      = rsl::tie(i, rsl::ignore, f);
  ASSERT_TRUE((std::same_as<ExpectT, decltype(res)>));
  ASSERT_EQ(&get<0>(res), &i);
  ASSERT_EQ(&get<1>(res), &rsl::ignore);
  ASSERT_EQ(&get<2>(res), &f);

}
