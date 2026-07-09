#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <rsl/variant>
#include <utility>

#include "common/assertions.h"

enum class ScopedEnum {
  INT[[=^^int]],
  CHAR[[=^^char]],
  BOOL[[=^^bool]]
};

namespace unscoped {
enum UnscopedEnum {  //
  INT[[=^^int]],
  CHAR[[=^^char]],
  BOOL[[=^^bool]]
};
}

TEST(TaggedVariant, Switch) {
  using variant_type = rsl::tagged_variant<ScopedEnum>;
  variant_type variant{42};
  switch (variant) {
    using enum variant_type::tags;
    case INT:  ASSERT_EQ(get<INT>(variant), 42); break;
    case CHAR: FAIL();
    case BOOL: FAIL();
  }
  
  variant = variant_type{std::in_place_index<2>, false};
  switch (variant) {
    using enum variant_type::tags;
    case INT:  FAIL();
    case CHAR: FAIL();
    case BOOL: ASSERT_EQ(variant->BOOL, false); break;
  }
}
