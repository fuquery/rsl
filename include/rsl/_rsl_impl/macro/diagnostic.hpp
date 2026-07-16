#pragma once

#include "compiler.hpp"

#define RSL_STRINGIZE_IMPL(x) #x
#define RSL_STRINGIZE(x)      RSL_STRINGIZE_IMPL(x)
#if RSL_COMPILER == RSL_COMPILER_MSVC
#  define RSL_PRAGMA(x) __pragma(x)
#else
#  define RSL_PRAGMA(x) _Pragma(RSL_STRINGIZE(x))
#endif


#if RSL_COMPILER == RSL_COMPILER_MSVC
#  define RSL_DIAG_PUSH        RSL_PRAGMA(warning(push))
#  define RSL_DIAG_POP         RSL_PRAGMA(warning(pop))
#  define RSL_DIAG_DISABLE(id) RSL_PRAGMA(warning(disable : RSL_DIAGS_##id))
#  define RSL_DIAG_ERROR(id)   RSL_PRAGMA(warning(error : RSL_DIAGS_##id))
#elif RSL_COMPILER == RSL_COMPILER_GCC || RSL_COMPILER == RSL_COMPILER_CLANG
#  define RSL_DIAG_PUSH       RSL_PRAGMA(RSL_COMPILER_ID diagnostic push)
#  define RSL_DIAG_POP        RSL_PRAGMA(RSL_COMPILER_ID diagnostic pop)
#  define RSL_DIAG_DISABLE(w) RSL_PRAGMA(RSL_COMPILER_ID diagnostic ignored RSL_DIAGS_##w)
#  define RSL_DIAG_ERROR(w)   RSL_PRAGMA(RSL_COMPILER_ID diagnostic error RSL_DIAGS_##w)
#else
#  warning "Unsupported compiler"
#endif

#define RSL_IMPL_CONSUME(...)
#define RSL_DIAG_push    RSL_DIAG_PUSH RSL_IMPL_CONSUME
#define RSL_DIAG_pop     RSL_DIAG_POP RSL_IMPL_CONSUME
#define RSL_DIAG_disable RSL_DIAG_DISABLE
#define RSL_DIAG_error   RSL_DIAG_ERROR

// Diagnostic map
#if RSL_COMPILER == RSL_COMPILER_MSVC
#  define RSL_DIAGS_literal_suffix 4455
#  define RSL_DIAGS_narrowing      4838
#elif RSL_COMPILER == RSL_COMPILER_GCC
#  define RSL_DIAGS_literal_suffix "-Wliteral-suffix"
#  define RSL_DIAGS_narrowing      "-Wnarrowing"
#elif RSL_COMPILER == RSL_COMPILER_CLANG
#  define RSL_DIAGS_literal_suffix "-Wuser-defined-literals"
#  define RSL_DIAGS_narrowing      "-Wc++11-narrowing"
#else
#  warning "Unsupported compiler"
#endif