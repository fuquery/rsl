#pragma once
#include <meta>

#include "compiler.hpp"

#if RSL_COMPILER == RSL_COMPILER_GCC
#  define RSL_DEFINE_STATIC_ARRAY(...) [:::std::meta::reflect_constant_array(__VA_ARGS__):]
#else
#  define RSL_DEFINE_STATIC_ARRAY(...) (::std::define_static_array(__VA_ARGS__))
#endif
