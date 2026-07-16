#include <gtest/gtest.h>

#include <string>
#include <rsl/experimental/trie>


TEST(Trie, Matches) {
  auto trie = rsl::trie{
      {"apple", "banana", "cherry"}
  };

  ASSERT_TRUE(trie.matches("apple"));
  ASSERT_TRUE(trie.matches("banana"));
  ASSERT_TRUE(trie.matches("cherry"));

  ASSERT_FALSE(trie.matches("pear"));
  ASSERT_FALSE(trie.matches(""));
  ASSERT_FALSE(trie.matches("APPLE"));
}


TEST(Trie, Find) {
  auto trie = rsl::trie{
      {"apple", "banana", "cherry"}
  };

  ASSERT_EQ(trie.find("apple"), 0);
  ASSERT_EQ(trie.find("banana"), 1);
  ASSERT_EQ(trie.find("cherry"), 2);

  ASSERT_EQ(trie.find("pear"), -1);
  ASSERT_EQ(trie.find(""), -1);
  ASSERT_EQ(trie.find("APPLE"), -1);
}