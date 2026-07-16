#pragma once
#include "compiler.hpp"

#if RSL_COMPILER == RSL_COMPILER_CLANG
#  define RSL_ALWAYS_INLINE  [[clang::always_inline]] inline
#  define RSL_NO_INLINE      [[clang::noinline]]

#elif RSL_COMPILER == RSL_COMPILER_GCC
#  define RSL_ALWAYS_INLINE __attribute__((always_inline)) inline
#  define RSL_NO_INLINE     __attribute__((noinline))

#elif RSL_COMPILER == RSL_COMPILER_MSVC
#  define RSL_ALWAYS_INLINE __forceinline
#  define RSL_NO_INLINE     __declspec(noinline)

#endif

#define RSL_INLINE_opt_never  RSL_NO_INLINE
#define RSL_INLINE_opt_always RSL_ALWAYS_INLINE
#define RSL_INLINE(_opt)      RSL_INLINE_opt_##_opt

// Clang specific extensions
#if RSL_COMPILER == RSL_COMPILER_CLANG
#  define RSL_ENABLE_IF(...) __attribute__((enable_if(__VA_ARGS__)))
#else
#  define RSL_ENABLE_IF(...)
#endif