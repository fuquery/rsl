#include <gtest/gtest.h>
#include <common/assertions.h>

#include <rsl/tuple>

TEST(Tuple, GetIdxCategory) {
  auto tuple        = rsl::tuple<int>{42};
  auto const ctuple = rsl::tuple<int>{42};

  ASSERT_SAME(decltype(get<0>(tuple)), int&);
  ASSERT_SAME(decltype(get<0>(ctuple)), int const&);

  ASSERT_SAME(decltype(get<0>(std::move(tuple))), int&&);
  ASSERT_SAME(decltype(get<0>(std::move(ctuple))), int const&&);

  ASSERT_SAME(decltype(tuple.template get<0>()), int&);
  ASSERT_SAME(decltype(ctuple.template get<0>()), int const&);

  ASSERT_SAME(decltype(std::move(tuple).template get<0>()), int&&);
  ASSERT_SAME(decltype(std::move(ctuple).template get<0>()), int const&&);
}
