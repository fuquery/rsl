#include <vector>
#include <map>
#include <string>
#include <string_view>

#include <gtest/gtest.h>
#include <common/assertions.h>

#include <rsl/repr>

TEST(Repr, Fundamental) {
  ASSERT_EQ(rsl::repr(true), "true");
  ASSERT_EQ(rsl::repr(false), "false");
  ASSERT_EQ(rsl::repr(0), "0");
  ASSERT_EQ(rsl::repr(0ZU), "0UL"); // cannot distinguish from aliased type
  ASSERT_EQ(rsl::repr(0L), "0L");
  ASSERT_EQ(rsl::repr(0LL), "0LL");
  ASSERT_EQ(rsl::repr(0U), "0U");
  ASSERT_EQ(rsl::repr(0UL), "0UL");
  ASSERT_EQ(rsl::repr(0ULL), "0ULL");

  ASSERT_EQ(rsl::repr(1.2), "1.2");
  ASSERT_EQ(rsl::repr(1.2F), "1.2F");
  ASSERT_EQ(rsl::repr(1.2L), "1.2L");

  ASSERT_EQ(rsl::repr("0"), "\"0\"");
  ASSERT_EQ(rsl::repr('0'), "'0'");
}

namespace {
namespace TestNS {
struct TestAgg {
  int a;
  char b;
};

struct Unsupported{
  explicit Unsupported(int){}
};
}  // namespace TestNS
}  // namespace

TEST(Repr, StringLikes) {
  char const* var = "foo";
  ASSERT_EQ(rsl::repr("foo"), "\"foo\"");
  ASSERT_EQ(rsl::repr(var), "\"foo\"");
  ASSERT_EQ(rsl::repr(std::string("foo")), "\"foo\"");
  ASSERT_EQ(rsl::repr(std::string_view("foo")), "\"foo\"");

  static_assert(rsl::repr(std::string_view("foo")) == "\"foo\"");
}

TEST(Repr, Aggregates) {
  auto obj = TestNS::TestAgg{42, 'c'};
  ASSERT_EQ(rsl::repr(obj), "TestAgg{42, 'c'}");
  ASSERT_EQ(rsl::repr(obj, {.names=rsl::NameMode::qualified}), "TestNS::TestAgg{42, 'c'}");
}

TEST(Repr, Iterables) {
  ASSERT_EQ(rsl::repr(std::vector<int>({1, 2, 3, 4})), "vector<int>{1, 2, 3, 4}");
  ASSERT_EQ(rsl::repr(std::vector<int>({1, 2, 3, 4}), {.names=rsl::NameMode::qualified}), "std::vector<int>{1, 2, 3, 4}");

  ASSERT_EQ(rsl::repr(std::map<int, int>({{1, 2}, {3, 4}})), "map<int, int>{{1, 2}, {3, 4}}");
}

TEST(Repr, Unsupported) {
  ASSERT_EQ(rsl::repr(TestNS::Unsupported(3), {.names=rsl::NameMode::unqualified}), "Unsupported{/*...*/}");
}