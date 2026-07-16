#include <string>
#include <gtest/gtest.h>
#include <common/assertions.h>

#include <rsl/experimental/expect>

namespace {
TEST(Expect, Logic) {
  using namespace rsl::placeholders;
  ASSERT_TRUE(expect(_0 > 1)(10));
}

TEST(Expect, ArrayOfChecks) {
  using namespace rsl::placeholders;

  std::function<bool(int)> checks[] = {
    expect(_0 > 1),
    expect(20 < _0)
  };

  for (auto const& check : checks) {
    int x = 50;
    ASSERT_TRUE(check(x));
  }
}

struct TestStruct {
  int a;
  char b;
};

TEST(Expect, Typed) {
  using namespace rsl::typed_placeholders;
  auto x = TestStruct(42, 'd');
  ASSERT_TRUE(expect(_0<TestStruct>->a == 42)(x));
  ASSERT_TRUE(expect(_0<TestStruct>->b == 'd')(x));
}
}