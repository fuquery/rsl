#include <gtest/gtest.h>

#include <rsl/annotations>

struct HasMemberWithRename {
  [[= rsl::rename("foo")]] int member;
  [[= rsl::preferred_name("foo")]] int preferred;
  int other;
};

TEST(Rename, Fundamental) {
  ASSERT_TRUE([] consteval {
    return extract<rsl::rename>(annotations_of(^^HasMemberWithRename::member)[0]) ==
           rsl::rename("foo");
  }());
  ASSERT_TRUE(rsl::is_renamed(^^HasMemberWithRename::member));
  ASSERT_TRUE(rsl::is_renamed(^^HasMemberWithRename::preferred));
  ASSERT_FALSE(rsl::is_renamed(^^HasMemberWithRename::other));
}

TEST(Rename, TemplateArgument) {
  auto foo = []<rsl::rename R>() {
    ASSERT_EQ(R, rsl::rename("bar"));
    ASSERT_NE(R, rsl::rename("baz"));
    ASSERT_STREQ(R.value.data(), std::define_static_string("bar"));
  };

  foo.template operator()<rsl::rename("bar")>();
}
