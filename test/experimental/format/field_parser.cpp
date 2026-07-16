#include <string_view>
#include <vector>
#include <string>
#include <rsl/_impl/format/fmt_parser.hpp>

#include <gtest/gtest.h>

using namespace rsl::_format_impl;

namespace {
Replacement parse(std::string_view str) {
  auto parser = Replacement(str);
  parser.do_parse();
  return parser;
}
}  // namespace


TEST(FormatField, Simple) {
  auto empty = parse("");
  ASSERT_EQ(empty.kind, ReplacementType::automatic);
  ASSERT_TRUE(empty.field.empty());
  ASSERT_TRUE(empty.specs.empty());
  ASSERT_TRUE(empty.style_tags.empty());

  auto indexed = parse("1");
  ASSERT_EQ(indexed.kind, ReplacementType::indexed);
  ASSERT_EQ(indexed.field, "1");
  ASSERT_TRUE(indexed.specs.empty());
  ASSERT_TRUE(indexed.style_tags.empty());

  auto field = parse("foo");
  ASSERT_EQ(field.kind, ReplacementType::field);
  ASSERT_EQ(field.field, "foo");
  ASSERT_TRUE(field.specs.empty());
  ASSERT_TRUE(field.style_tags.empty());

  auto bang = parse("!");
  ASSERT_EQ(bang.kind, ReplacementType::remove);
  ASSERT_EQ(bang.field, "!");
  ASSERT_TRUE(bang.specs.empty());
  ASSERT_TRUE(bang.style_tags.empty());
}

TEST(FormatField, WithSpecs) {
  auto empty = parse(":[<3");
  ASSERT_EQ(empty.kind, ReplacementType::automatic);
  ASSERT_TRUE(empty.field.empty());
  ASSERT_EQ(empty.specs, "[<3");
  ASSERT_TRUE(empty.style_tags.empty());

  auto indexed = parse("1:[<3");
  ASSERT_EQ(indexed.kind, ReplacementType::indexed);
  ASSERT_EQ(indexed.field, "1");
  ASSERT_EQ(indexed.specs, "[<3");
  ASSERT_TRUE(indexed.style_tags.empty());

  auto field = parse("foo:[<3");
  ASSERT_EQ(field.kind, ReplacementType::field);
  ASSERT_EQ(field.field, "foo");
  ASSERT_EQ(field.specs, "[<3");
  ASSERT_TRUE(field.style_tags.empty());

  auto bang = parse("!:[<3");
  ASSERT_EQ(bang.kind, ReplacementType::invalid);
}

TEST(FormatField, WithColor) {
  auto expected = std::vector<std::string>{"red"};
  auto empty = parse("[red]");
  ASSERT_EQ(empty.kind, ReplacementType::automatic);
  ASSERT_TRUE(empty.field.empty());
  ASSERT_EQ(empty.style_tags, expected);

  auto indexed = parse("[red]1");
  ASSERT_EQ(indexed.kind, ReplacementType::indexed);
  ASSERT_EQ(indexed.field, "1");
  ASSERT_EQ(indexed.style_tags, expected);

  auto field = parse("[red]foo");
  ASSERT_EQ(field.kind, ReplacementType::field);
  ASSERT_EQ(field.field, "foo");
  ASSERT_EQ(field.style_tags, expected);

  auto bang = parse("[red]!");
  ASSERT_EQ(bang.kind, ReplacementType::remove);
  ASSERT_EQ(bang.field, "!");
  ASSERT_EQ(bang.style_tags, expected);
}

TEST(FormatField, WithColorAndSpec) {
  auto expected = std::vector<std::string>{"red"};
  auto empty = parse("[red]:[<3");
  ASSERT_EQ(empty.kind, ReplacementType::automatic);
  ASSERT_TRUE(empty.field.empty());
  ASSERT_EQ(empty.specs, "[<3");
  ASSERT_EQ(empty.style_tags, expected);

  auto indexed = parse("[red]1:[<3");
  ASSERT_EQ(indexed.kind, ReplacementType::indexed);
  ASSERT_EQ(indexed.field, "1");
  ASSERT_EQ(indexed.specs, "[<3");
  ASSERT_EQ(indexed.style_tags, expected);

  auto field = parse("[red]foo:[<3");
  ASSERT_EQ(field.kind, ReplacementType::field);
  ASSERT_EQ(field.field, "foo");
  ASSERT_EQ(field.specs, "[<3");
  ASSERT_EQ(field.style_tags, expected);

  auto bang = parse("[red]!:[<3");
  ASSERT_EQ(bang.kind, ReplacementType::invalid);
}

TEST(FormatField, MultiColor) {
  auto expected = std::vector<std::string>{"fg=red", "bg=blue"};
  
  auto no_space = parse("[fg=red,bg=blue]");
  ASSERT_EQ(no_space.style_tags, expected);
}