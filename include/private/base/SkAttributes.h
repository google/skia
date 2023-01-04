/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFeatures.h" // IWYU pragma: keep

#if !defined(SK_ATTRIBUTE)
#  if defined(__clang__) || defined(__GNUC__)
#    define SK_ATTRIBUTE(attr) __attribute__((attr))
#  else
#    define SK_ATTRIBUTE(attr)
#  endif
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
