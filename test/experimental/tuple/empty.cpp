#include <rsl/experimental/tuple>
#include <common/assertions.h>

TEST(Tuple, Empty) {
    auto tuple = rsl::tuple<>{};
    ASSERT_EQ(rsl::tuple_size_v<rsl::tuple<>>, 0);
}