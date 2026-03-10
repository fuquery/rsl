#pragma once

// compile environment checks

/**
 * @param compiler: one of UNKNOWN, CLANG, GCC, MSVC
 */
#define $compiler_is(compiler) (RSL_COMPILER == RSL_COMPILER_##compiler)

/**
 * @param stdlib: one of UNKNOWN, LIBCXX, GLIBCXX, MSVC
 */
#define $stdlib_is(stdlib) (RSL_STDLIB == RSL_STDLIB_##stdlib)

/**
 * @param os: one of UNKNOWN, WINDOWS, LINUX, MAC, FREEBSD, ANDROID, IOS
 */
#define $os_is(os) (RSL_OS == RSL_OS_##os)

// feature macro helper
#define $uses_opt(x) 

// attributes
/**
 * @param: one of always, never
 */
#define $inline(...) 

/** Enable this overload if expr evaluates to true. Clang only.
 * @param expr
 */
#define $enable_if(...) RSL_ENABLE_IF(__VA_ARGS__)

//! Workaround for expansion statements in GCC
#define $define_static_array(...)

/** Applies `macro` to every element and inserts delimiters produced by `delim` between elements
 * @param macro function-like macro applied to every element
 * @param delim function-like macro producing the delimiter
 * @param ... elements to loop over
*/
#define $for_each_delim(macro, delim, ...) RSL_IMPL_FOR_EACH_DELIM(macro, delim, __VA_ARGS__)

/** Applies `macro` to every element and inserts commas between elements
 * @param macro function-like macro applied to every element
 * @param ... elements to loop over
*/
#define $for_each_list(macro, ...) RSL_IMPL_FOR_EACH_DELIM(macro, RSL_IMPL_DELIM_COMMA, __VA_ARGS__)

/** Applies `macro` to every element
 * @param macro function-like macro applied to every element
 * @param ... elements to loop over
*/
#define $for_each(macro, ...) RSL_IMPL_FOR_EACH_DELIM(macro, RSL_IMPL_DELIM_NONE, __VA_ARGS__)

#define $diagnostics(op, ...) RSL_DIAG_##op (__VA_ARGS__)

/// Introduces an explicit template region
#define $template template for (auto _ : "")
