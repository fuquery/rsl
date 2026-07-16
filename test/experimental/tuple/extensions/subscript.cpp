#include <rsl/macro>
#if $compiler_is(CLANG)

#include <gtest/gtest.h>
#include <common/assertions.h>

#include <rsl/experimental/tuple>

TEST(Tuple, ExtSubscriptCategory) {
    auto tuple = rsl::tuple<int>{42};
    auto const ctuple = rsl::tuple<int>{42};

    ASSERT_SAME(decltype(tuple[0]), int&);
    ASSERT_SAME(decltype(ctuple[0]), int const&);

    ASSERT_SAME(decltype(std::move(tuple)[0]), int&&);
    ASSERT_SAME(decltype(std::move(ctuple)[0]), int const&&);
}

#endif