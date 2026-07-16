#include <string>
#include <gtest/gtest.h>
#include <common/assertions.h>

#include <string_view>
#include <vector>
#include <unordered_map>

#include <rsl/repr>

namespace {
namespace TestNS {
struct HasADL {};
consteval auto preferred_name(std::type_identity<HasADL>) {
  return "foo";
}

struct HasFriend {
  friend consteval auto preferred_name(std::type_identity<HasFriend>) { return "foo"; }
};

struct[[= rsl::preferred_name("foo")]] HasAnnotation {};

struct HasSpecialization {};
struct HasMemberConstant {
  static constexpr auto preferred_name = "foo";
};
struct HasMemberFunction {
  static consteval auto preferred_name() { return "foo"; }
};

template <typename T, int Arg>
struct Dummy {};

template <typename T, int Arg, template <typename, int> class S>
struct HasTArgs {};
}  // namespace TestNS

}  // namespace

template <>
struct rsl::preferred_name<TestNS::HasSpecialization> {
  constexpr static auto value = "foo";
};

using namespace std::string_literals;

TEST(CanonicalName, Basic) {
  ASSERT_EQ(rsl::unqualified_name<int>, "int"s);
  ASSERT_EQ(rsl::unqualified_name<int const>, "int const"s);
  ASSERT_EQ(rsl::unqualified_name<int const&>, "int const&"s);
  ASSERT_EQ(rsl::unqualified_name<int&>, "int&"s);
  ASSERT_EQ(rsl::unqualified_name<int&&>, "int&&"s);
  ASSERT_EQ(rsl::unqualified_name<int*>, "int*"s);
  ASSERT_EQ(rsl::unqualified_name<int const*>, "int const*"s);
  ASSERT_EQ(rsl::unqualified_name<int const* const>, "int const* const"s);
  ASSERT_EQ(rsl::unqualified_name<int* const>, "int* const"s);

  ASSERT_EQ(rsl::unqualified_name_of(^^int), "int"s);
  ASSERT_EQ(rsl::unqualified_name_of(^^int const* const), "int const* const"s);

  ASSERT_EQ((rsl::unqualified_name<TestNS::HasTArgs<int, 3, TestNS::Dummy>>),
            "HasTArgs<int, 3, Dummy>"s);
}

TEST(CanonicalName, StdAliases) {
  ASSERT_EQ(rsl::unqualified_name<std::string>, "string"s);
  ASSERT_EQ(rsl::unqualified_name<std::string_view>, "string_view"s);
}

TEST(CanonicalName, TemplateDefaults) {
  ASSERT_EQ(rsl::unqualified_name<std::vector<std::string>>, "vector<string>"s);
  ASSERT_EQ(
      (rsl::unqualified_name<std::unordered_map<std::string_view, std::vector<std::string>>>),
      "unordered_map<string_view, vector<string>>"s);

  ASSERT_EQ(rsl::unqualified_name<std::vector<std::string> const>, "vector<string> const"s);
  ASSERT_EQ(rsl::unqualified_name<std::vector<std::string> const&>, "vector<string> const&"s);
  ASSERT_EQ(rsl::unqualified_name<std::vector<std::string> const&&>, "vector<string> const&&"s);
  ASSERT_EQ(rsl::unqualified_name<std::vector<std::string>&>, "vector<string>&"s);
  ASSERT_EQ(rsl::unqualified_name<std::vector<std::string>&&>, "vector<string>&&"s);
  ASSERT_EQ(rsl::unqualified_name<std::vector<std::string>*>, "vector<string>*"s);
}

TEST(CanonicalName, Annotation) {
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasAnnotation>, "foo"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasAnnotation const>, "foo const"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasAnnotation const&>, "foo const&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasAnnotation const&&>, "foo const&&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasAnnotation&>, "foo&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasAnnotation&&>, "foo&&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasAnnotation*>, "foo*"s);
}

TEST(CanonicalName, MemberConstant) {
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasMemberConstant>, "foo"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasMemberConstant const>, "foo const"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasMemberConstant const&>, "foo const&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasMemberConstant const&&>, "foo const&&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasMemberConstant&>, "foo&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasMemberConstant&&>, "foo&&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasMemberConstant*>, "foo*"s);
}

TEST(CanonicalName, MemberFunction) {
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasMemberFunction>, "foo"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasMemberFunction const>, "foo const"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasMemberFunction const&>, "foo const&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasMemberFunction const&&>, "foo const&&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasMemberFunction&>, "foo&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasMemberFunction&&>, "foo&&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasMemberFunction*>, "foo*"s);
}

TEST(CanonicalName, ADL) {
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasADL>, "foo"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasADL const>, "foo const"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasADL const&>, "foo const&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasADL const&&>, "foo const&&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasADL&>, "foo&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasADL&&>, "foo&&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasADL*>, "foo*"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasFriend>, "foo"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasFriend const>, "foo const"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasFriend const&>, "foo const&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasFriend const&&>, "foo const&&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasFriend&>, "foo&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasFriend&&>, "foo&&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasFriend*>, "foo*"s);
}

TEST(CanonicalName, Specialization) {
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasSpecialization>, "foo"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasSpecialization const>, "foo const"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasSpecialization const&>, "foo const&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasSpecialization const&&>, "foo const&&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasSpecialization&>, "foo&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasSpecialization&&>, "foo&&"s);
  ASSERT_EQ(rsl::unqualified_name<TestNS::HasSpecialization*>, "foo*"s);
}

// TODO test precedence

TEST(QualifiedName, Basic) {
  ASSERT_EQ(rsl::qualified_name<int>, "int"s);
  ASSERT_EQ(rsl::fully_qualified_name<int>, "int"s);
  ASSERT_EQ(rsl::qualified_name_of(^^int), "int"s);
  ASSERT_EQ(rsl::fully_qualified_name_of(^^int), "int"s);
}

TEST(QualifiedName, StdAliases) {
  ASSERT_EQ(rsl::qualified_name<std::string>, "std::string"s);
  ASSERT_EQ(rsl::fully_qualified_name<std::string>, "::std::string"s);
  ASSERT_EQ(rsl::qualified_name_of(^^std::string), "std::string"s);
  ASSERT_EQ(rsl::fully_qualified_name_of(^^std::string), "::std::string"s);
}

TEST(QualifiedName, TemplateDefaults) {
  ASSERT_EQ(rsl::qualified_name<std::vector<std::string>>, "std::vector<std::string>"s);
  ASSERT_EQ(rsl::fully_qualified_name<std::vector<std::string>>,
            "::std::vector<::std::string>"s);
  ASSERT_EQ(
      (rsl::qualified_name<std::unordered_map<std::string_view, std::vector<std::string>>>),
      "std::unordered_map<std::string_view, std::vector<std::string>>"s);
  ASSERT_EQ((rsl::fully_qualified_name<
                std::unordered_map<std::string_view, std::vector<std::string>>>),
            "::std::unordered_map<::std::string_view, ::std::vector<::std::string>>"s);
}

TEST(QualifiedName, Annotation) {
  ASSERT_EQ(rsl::qualified_name<TestNS::HasAnnotation>, "TestNS::foo"s);
  ASSERT_EQ(rsl::fully_qualified_name<TestNS::HasAnnotation>, "::TestNS::foo"s);
}

TEST(QualifiedName, MemberConstant) {
  ASSERT_EQ(rsl::qualified_name<TestNS::HasMemberConstant>, "TestNS::foo"s);
  ASSERT_EQ(rsl::fully_qualified_name<TestNS::HasMemberConstant>, "::TestNS::foo"s);
}

TEST(QualifiedName, MemberFunction) {
  ASSERT_EQ(rsl::qualified_name<TestNS::HasMemberFunction>, "TestNS::foo"s);
  ASSERT_EQ(rsl::fully_qualified_name<TestNS::HasMemberFunction>, "::TestNS::foo"s);
}

TEST(QualifiedName, ADL) {
  ASSERT_EQ(rsl::qualified_name<TestNS::HasADL>, "TestNS::foo"s);
  ASSERT_EQ(rsl::qualified_name<TestNS::HasFriend>, "TestNS::foo"s);
  ASSERT_EQ(rsl::fully_qualified_name<TestNS::HasADL>, "::TestNS::foo"s);
  ASSERT_EQ(rsl::fully_qualified_name<TestNS::HasFriend>, "::TestNS::foo"s);
}

TEST(QualifiedName, Specialization) {
  ASSERT_EQ(rsl::qualified_name<TestNS::HasSpecialization>, "TestNS::foo"s);
  ASSERT_EQ(rsl::fully_qualified_name<TestNS::HasSpecialization>, "::TestNS::foo"s);
  ASSERT_EQ((rsl::type_name<TestNS::HasSpecialization, rsl::NameMode::qualified>), "TestNS::foo"s);
  ASSERT_EQ((rsl::type_name<TestNS::HasSpecialization, rsl::NameMode::fully_qualified>),
            "::TestNS::foo"s);
}