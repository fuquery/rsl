#pragma once
#include <cstdio>
#include <rsl/format.h>

namespace rsl {
namespace _impl_platform { struct isatty{}; }

using rsl::_impl_platform::isatty;

template <typename... Args>
void println(rsl::format_string<Args...> fmt, Args&&... args);
}  // namespace rsl