#include <gtest/gtest.h>
#include <rsl/variant>
#include <rsl/_rsl_impl/traits.hpp>

#include <common/assertions.h>


TEST(Hash, Monostate) {
  auto const hasher = std::hash<rsl::monostate>{};

  auto obj1 = rsl::monostate{};
  auto const obj2 = rsl::monostate{};

  ASSERT_EQ(hasher(obj1), hasher(obj1));
  ASSERT_EQ(hasher(obj2), hasher(obj2));
  ASSERT_EQ(hasher(obj1), hasher(obj2));
  ASSERT_EQ(hasher(obj2), hasher(obj1));
  ASSERT_SAME(decltype(hasher(obj1)), std::size_t);
  ASSERT_NOEXCEPT(hasher(obj1));
  ASSERT_TRUE(rsl::_impl::is_hashable<rsl::monostate>);
}