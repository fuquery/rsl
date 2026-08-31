#include <rsl/inject>


namespace foo_impl
{
  static constexpr auto target_name = "injected_implementation";
  [[=Inject("demo/file.cpp")]]
  std::string generateFunction()
  {
    return " int foo() { return 42;}";
  }
} // namespace inject


RSLINJECT_ENABLE_NS(foo_impl)
