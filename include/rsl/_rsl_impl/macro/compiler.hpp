#pragma once

#define RSL_COMPILER_UNKNOWN 0
#define RSL_COMPILER_CLANG   1
#define RSL_COMPILER_GCC     2
#define RSL_COMPILER_MSVC    3

#define RSL_VERSION_ENCODE(major, minor, patch) ((major) * 1000000 + (minor) * 1000 + (patch))

#if defined(__clang__)
#  define RSL_COMPILER    RSL_COMPILER_CLANG
#  define RSL_COMPILER_ID clang
#  define RSL_COMPILER_VERSION \
    RSL_VERSION_ENCODE(__clang_major__, __clang_minor__, __clang_patchlevel__)
#elif defined(__GNUC__)
#  define RSL_COMPILER         RSL_COMPILER_GCC
#  define RSL_COMPILER_ID      GCC
#  define RSL_COMPILER_VERSION RSL_VERSION_ENCODE(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
#elif defined(_MSC_VER)
#  define RSL_COMPILER    RSL_COMPILER_MSVC
#  define RSL_COMPILER_ID MSVC
#  if defined(_MSC_FULL_VER)
#    define RSL_COMPILER_VERSION _MSC_FULL_VER
#  else
#    define RSL_COMPILER_VERSION _MSC_VER
#  endif

#else
#  define RSL_COMPILER         RSL_COMPILER_UNKNOWN
#  define RSL_COMPILER_ID      UNKNOWN
#  define RSL_COMPILER_VERSION 0
#endif

#define RSL_STDLIB_UNKNOWN 0
#define RSL_STDLIB_LIBCXX  1
#define RSL_STDLIB_GLIBCXX 2
#define RSL_STDLIB_MSVC    3

#if defined(_MSC_VER) && defined(_MSVC_STL_VERSION)
#  define RSL_STDLIB         RSL_STDLIB_MSVC
#  define RSL_STDLIB_VERSION _MSVC_STL_VERSION
#elif defined(__GLIBCXX__)
#  define RSL_STDLIB         RSL_STDLIB_GLIBCXX
#  define RSL_STDLIB_VERSION __GLIBCXX__
#elif defined(_LIBCPP_VERSION) || defined(__llvm_libc__)
#  define RSL_STDLIB         RSL_STDLIB_LIBCXX
#  define RSL_STDLIB_VERSION _LIBCPP_VERSION
#else
#  define RSL_STDLIB         RSL_STDLIB_UNKNOWN
#  define RSL_STDLIB_VERSION 0
#endif
