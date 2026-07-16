#include <gtest/gtest.h>

#include <rsl/enum>
#include <rsl/serialize>

namespace {
enum E1 { A1 = 1 };
enum E2 : unsigned { A2 = 1 };
enum E3 : int { A3 = -1 };

enum class E4 { A4 = 1 };
enum class E5 : unsigned { A5 = 1 };
enum class E6 : int { A6 = -1 };
}  // namespace

TEST(ToString, Enum) {
  ASSERT_EQ(rsl::to_string(A1), "A1");
  ASSERT_EQ(rsl::to_string(A2), "A2");
  ASSERT_EQ(rsl::to_string(A3), "A3");
  ASSERT_EQ(rsl::to_string(E4::A4), "A4");
  ASSERT_EQ(rsl::to_string(E5::A5), "A5");
  ASSERT_EQ(rsl::to_string(E6::A6), "A6");

  ASSERT_EQ(rsl::to_string(E1(0)), "E1(0)");
  ASSERT_EQ(rsl::to_string(E2(0)), "E2(0)");
  ASSERT_EQ(rsl::to_string(E3(0)), "E3(0)");
  ASSERT_EQ(rsl::to_string(E4(0)), "E4(0)");
  ASSERT_EQ(rsl::to_string(E5(0)), "E5(0)");
  ASSERT_EQ(rsl::to_string(E6(0)), "E6(0)");
}

namespace {
enum[[= rsl::flag_enum]] F1 {
  A      = 1,
  ALSO_A = 1,
  B      = 2,
  C      = 4,
  D      = 8,
  AC     = 5,
  AD     = 9,
};

enum class[[= rsl::flag_enum]] F2 {
  A      = 1,
  ALSO_A = 1,
  B      = 2,
  C      = 4,
  D      = 8,
  AC     = 5,
  AD     = 9,
};
}  // namespace

TEST(ToString, FlagEnum) {
  ASSERT_EQ(rsl::to_string(F1(ALSO_A)), "A");
  ASSERT_EQ(rsl::to_string(F1(AC | B | D)), "AC|B|D");
  ASSERT_EQ(rsl::to_string(F1(AC | AD)), "AC|D");
  ASSERT_EQ(rsl::to_string(F1(AC | 16)), "AC|F1(16)");
  ASSERT_EQ(rsl::to_string(F1(16)), "F1(16)");

  ASSERT_EQ(rsl::to_string(F2(ALSO_A)), "A");
  ASSERT_EQ(rsl::to_string(F2(AC | B | D)), "AC|B|D");
  ASSERT_EQ(rsl::to_string(F2(AC | AD)), "AC|D");
  ASSERT_EQ(rsl::to_string(F2(AC | 16)), "AC|F2(16)");
  ASSERT_EQ(rsl::to_string(F2(16)), "F2(16)");
}