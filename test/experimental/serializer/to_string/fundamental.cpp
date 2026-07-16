#include <climits>
#include <gtest/gtest.h>

#include <rsl/serialize>

TEST(ToString, Bool) {
  ASSERT_EQ(rsl::to_string(true), "true");
  ASSERT_EQ(rsl::to_string(false), "false");
}

TEST(ToString, Integral) {
  ASSERT_EQ(rsl::to_string(static_cast<signed char>(0)), "0");
  ASSERT_EQ(rsl::to_string(static_cast<short>(0)), "0");
  ASSERT_EQ(rsl::to_string(static_cast<int>(0)), "0");
  ASSERT_EQ(rsl::to_string(static_cast<long>(0)), "0");
  ASSERT_EQ(rsl::to_string(static_cast<long long>(0)), "0");

  ASSERT_EQ(rsl::to_string(static_cast<unsigned char>(0)), "0");
  ASSERT_EQ(rsl::to_string(static_cast<unsigned short>(0)), "0");
  ASSERT_EQ(rsl::to_string(static_cast<unsigned int>(0)), "0");
  ASSERT_EQ(rsl::to_string(static_cast<unsigned long>(0)), "0");
  ASSERT_EQ(rsl::to_string(static_cast<unsigned long long>(0)), "0");

  if (SCHAR_MIN == -128 && SCHAR_MAX == 127) {
    ASSERT_EQ(rsl::to_string(static_cast<long long>(-128)), "-128");
    ASSERT_EQ(rsl::to_string(static_cast<signed char>(127)), "127");
  }
  if (SHRT_MIN == -32768 && SHRT_MAX == 32767) {
    ASSERT_EQ(rsl::to_string(static_cast<long long>(-32768)), "-32768");
    ASSERT_EQ(rsl::to_string(static_cast<short>(32767)), "32767");
  }
  if (INT_MIN == -2147483648 && INT_MAX == 2147483647) {
    ASSERT_EQ(rsl::to_string(static_cast<long long>(-2147483648)), "-2147483648");
    ASSERT_EQ(rsl::to_string(static_cast<int>(2147483647)), "2147483647");
  }
  if (LONG_MIN == -9223372036854775807L - 1 && LONG_MAX == 9223372036854775807LL) {
    ASSERT_EQ(rsl::to_string(static_cast<long long>(-9223372036854775807LL - 1)),
              "-9223372036854775808");
    ASSERT_EQ(rsl::to_string(static_cast<long>(9223372036854775807LL)), "9223372036854775807");
  }
  if (LLONG_MIN == -9223372036854775807LL - 1 && LLONG_MAX == 9223372036854775807LL) {
    ASSERT_EQ(rsl::to_string(static_cast<long long>(-9223372036854775807LL - 1)),
              "-9223372036854775808");
    ASSERT_EQ(rsl::to_string(static_cast<long long>(9223372036854775807LL)), "9223372036854775807");
  }

  if (UCHAR_MAX == 255) {
    ASSERT_EQ(rsl::to_string(static_cast<unsigned char>(255)), "255");
  }
  if (USHRT_MAX == 65535) {
    ASSERT_EQ(rsl::to_string(static_cast<unsigned short>(65535)), "65535");
  }
  if (UINT_MAX == 4294967295) {
    ASSERT_EQ(rsl::to_string(static_cast<unsigned int>(4294967295)), "4294967295");
  }
  if (ULONG_MAX == 18446744073709551615ULL) {
    ASSERT_EQ(rsl::to_string(static_cast<unsigned long>(18446744073709551615ULL)),
              "18446744073709551615");
  }
  if (ULLONG_MAX == 18446744073709551615ULL) {
    ASSERT_EQ(rsl::to_string(static_cast<unsigned long long>(18446744073709551615ULL)),
              "18446744073709551615");
  }

  static_assert(rsl::to_string(1234) == "1234");
  static_assert(rsl::to_string(-1234) == "-1234");
}

TEST(ToString, floating) {
  ASSERT_TRUE(rsl::to_string(1.23F).starts_with("1.23"));
  ASSERT_TRUE(rsl::to_string(1.23).starts_with("1.23"));
  ASSERT_TRUE(rsl::to_string(1.23L).starts_with("1.23"));
}