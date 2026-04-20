#include <print>
#include "interface.hpp"

auto main(int argc, char *argv[]) -> int {
  std::println("{}", foo());
  return 0;
}
