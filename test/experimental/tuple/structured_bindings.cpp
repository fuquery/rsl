#include <gtest/gtest.h>
#include <common/assertions.h>

#define RSL_STD_COMPAT 1
#include <rsl/experimental/tuple>

TEST(Tuple, StructuredBindingSimple) {
    auto tuple = rsl::tuple<int, char>{42, 'c'};
    auto [a, b] = tuple;
    ASSERT_SAME(decltype(a), int);
    ASSERT_SAME(decltype(b), char);
    ASSERT_EQ(a, 42);
    ASSERT_EQ(b, 'c');
}

TEST(Tuple, StructuredBindingVariadic) {
    auto tuple = rsl::tuple<int, char>{42, 'c'};
    [&tuple]<int=3>(){
        auto [...members] = tuple;
    
        ASSERT_EQ(sizeof...(members), 2);
        ASSERT_SAME(decltype(members...[0]), int);
        ASSERT_SAME(decltype(members...[1]), char);
        ASSERT_EQ(members...[0], 42);
        ASSERT_EQ(members...[1], 'c');
    }();
}

using in_template = rsl::tuple<int, double, char>;

TEST(Tuple, AsTemplateParameter) {
    auto const check = []<in_template T>() {
        auto [a, b, c] = T;
        ASSERT_SAME(decltype(a), int);
        ASSERT_SAME(decltype(b), double);
        ASSERT_SAME(decltype(c), char);
        ASSERT_EQ(a, 42);
        ASSERT_EQ(b, 3.14);
        ASSERT_EQ(c, 'c');
    };

    check.template operator()<rsl::tuple<int, double, char>{42, 3.14, 'c'}>();
}

