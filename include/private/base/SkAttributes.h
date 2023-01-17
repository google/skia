/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAttributes_DEFINED
#define SkAttributes_DEFINED

#include "include/private/base/SkFeatures.h" // IWYU pragma: keep
#include "include/private/base/SkLoadUserConfig.h" // IWYU pragma: keep

#if defined(__clang__) || defined(__GNUC__)
#  define SK_ATTRIBUTE(attr) __attribute__((attr))
#else
#  define SK_ATTRIBUTE(attr)
#endif

#if !defined(SK_UNUSED)
#  if !defined(__clang__) && defined(_MSC_VER)
#    define SK_UNUSED __pragma(warning(suppress:4189))
#  else
#    define SK_UNUSED SK_ATTRIBUTE(unused)
#  endif
#endif

#if !defined(SK_WARN_UNUSED_RESULT)
    #define SK_WARN_UNUSED_RESULT SK_ATTRIBUTE(warn_unused_result)
#endif

/**
 * If your judgment is better than the compiler's (i.e. you've profiled it),
 * you can use SK_ALWAYS_INLINE to force inlining. E.g.
 *     inline void someMethod() { ... }             // may not be inlined
 *     SK_ALWAYS_INLINE void someMethod() { ... }   // should always be inlined
 */
#if !defined(SK_ALWAYS_INLINE)
#  if defined(SK_BUILD_FOR_WIN)
#    define SK_ALWAYS_INLINE __forceinline
#  else
#    define SK_ALWAYS_INLINE SK_ATTRIBUTE(always_inline) inline
#  endif
#endif

/**
 * If your judgment is better than the compiler's (i.e. you've profiled it),
 * you can use SK_NEVER_INLINE to prevent inlining.
 */
#if !defined(SK_NEVER_INLINE)
#  if defined(SK_BUILD_FOR_WIN)
#    define SK_NEVER_INLINE __declspec(noinline)
#  else
#    define SK_NEVER_INLINE SK_ATTRIBUTE(noinline)
#  endif
#endif

/**
 * Used to annotate a function as taking printf style arguments.
 * `A` is the (1 based) index of the format string argument.
 * `B` is the (1 based) index of the first argument used by the format string.
 */
#if !defined(SK_PRINTF_LIKE)
#  define SK_PRINTF_LIKE(A, B) SK_ATTRIBUTE(format(printf, (A), (B)))
#endif

/**
 * Used to ignore sanitizer warnings.
 */
#if !defined(SK_NO_SANITIZE)
#  define SK_NO_SANITIZE(A) SK_ATTRIBUTE(no_sanitize(A))
#endif

#endif
