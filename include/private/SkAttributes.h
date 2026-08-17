/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAttributes_DEFINED
#define SkAttributes_DEFINED

#include "include/private/SkFeatures.h" // IWYU pragma: keep
#include "include/private/SkLoadUserConfig.h" // IWYU pragma: keep

#if defined(__clang__) || defined(__GNUC__)
#  define SK_ATTRIBUTE(attr) __attribute__((attr))
#else
#  define SK_ATTRIBUTE(attr)
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
  #if defined(__has_attribute)
    #if __has_attribute(no_sanitize)
      // This should be for clang and versions of gcc >= 8.0
      #define SK_NO_SANITIZE(A) SK_ATTRIBUTE(no_sanitize(A))
    #else
      // For compilers that don't support sanitization, just do nothing.
      #define SK_NO_SANITIZE(A)
    #endif
  #else // no __has_attribute, e.g. MSVC
    #define SK_NO_SANITIZE(A)
  #endif
#endif

/**
 * Used to ignore CFI sanitizer warnings, supported only by Clang at the moment.
 */
#if defined(__clang__)
  #define SK_NO_SANITIZE_CFI SK_NO_SANITIZE("cfi")
#else
  #define SK_NO_SANITIZE_CFI
#endif

/**
 * Annotates a class' non-trivial special functions as trivial for the purposes of calls.
 * Allows a class with a non-trivial destructor to be __is_trivially_relocatable.
 * Use of this attribute on a public API breaks platform ABI.
 * Annotated classes may not hold pointers derived from `this`.
 * Annotated classes must implement move+delete as equivalent to memcpy+free.
 * Use may require more complete types, as callee destroys.
 *
 * https://clang.llvm.org/docs/AttributeReference.html#trivial-abi
 * https://libcxx.llvm.org/DesignDocs/UniquePtrTrivialAbi.html
 */
#if !defined(SK_TRIVIAL_ABI)
#  define SK_TRIVIAL_ABI
#endif

// Annotates a function indicating it can lead to out-of-bounds accesses (OOB)
// if given incorrect inputs.
//
// This commonly includes functions that take raw pointers, sizes, iterators,
// sentinels, etc., and cannot fully check their preconditions (e.g., that the
// provided pointer actually points to an allocation of at least the provided
// size). Useful to diagnose potential misuse via `-Wunsafe-buffer-usage`, as
// well as to mark functions in need of safer alternatives.
//
// All functions annotated with this macro should come with a `// PRECONDITIONS:`
// comment explaining what the caller must guarantee to ensure safe operation.
// Callers can then write `// SAFETY:` comments explaining why the specific
// preconditions have been met (e.g., verified by caller bounds, types, or
// nearby `SkASSERT`s).
//
// Ideally, unsafe functions should also be paired with a safer overload, e.g.,
// one that replaces pointer and size parameters with `SkSpan` (or `std::span`);
// otherwise, document safer replacement patterns callers can migrate to.
//
// Annotating a function `SK_UNSAFE_BUFFER_USAGE` means all call sites (that do
// not disable the warning) must wrap calls in `SK_UNSAFE_BUFFERS()`; see
// documentation there.
//
// See also:
//    https://chromium.googlesource.com/chromium/src/+/main/docs/unsafe_buffers.md
//    https://clang.llvm.org/docs/SafeBuffers.html
//    https://clang.llvm.org/docs/DiagnosticsReference.html#wunsafe-buffer-usage
//
// Usage:
//    Calls to this function must be wrapped in `SK_UNSAFE_BUFFERS()`.
//    SK_UNSAFE_BUFFER_USAGE void Func(T* input, T* end);
#if __has_cpp_attribute(clang::unsafe_buffer_usage)
#define SK_UNSAFE_BUFFER_USAGE [[clang::unsafe_buffer_usage]]
#else
#define SK_UNSAFE_BUFFER_USAGE
#endif

// Annotates code indicating that it should be permanently exempted from
// `-Wunsafe-buffer-usage`. For temporary cases such as migrating callers to
// safer patterns, use `SK_UNSAFE_TODO()` instead; see documentation there.
//
// All calls to functions annotated with `SK_UNSAFE_BUFFER_USAGE` must be
// marked with one of these two macros; they can also be used around pointer
// arithmetic, pointer subscripting, and the like.
//
// ** USE OF THIS MACRO SHOULD BE VERY RARE.** Using this macro indicates that
// the compiler cannot verify that the code avoids out-of-bounds (OOB) access,
// and manual review is required. Even with manual review, it's easy for
// assumptions to change and security bugs to creep in over time. Prefer safer
// patterns (such as `SkSpan` or `std::span`) instead.
//
// Usage should wrap the minimum necessary code, and *must* include a
// `// SAFETY: ...` comment that explains how the code guarantees safety or
// meets the requirements of called `SK_UNSAFE_BUFFER_USAGE` functions.
// Guarantees must be manually verifiable during code review using only local
// invariants. Valid invariants include:
// - Runtime conditions or `SkASSERT`s nearby
// - Invariants guaranteed by types in the surrounding code
// - Invariants guaranteed by function calls in the surrounding code
// - Caller requirements, if the containing function is itself annotated with
//   `SK_UNSAFE_BUFFER_USAGE`; this is less safe and should be a last resort
//
// See also:
//    https://chromium.googlesource.com/chromium/src/+/main/docs/unsafe_buffers.md
//    https://clang.llvm.org/docs/SafeBuffers.html
//    https://clang.llvm.org/docs/DiagnosticsReference.html#wunsafe-buffer-usage
//
// Usage:
//
//    // The following call will not trigger a compiler warning even if `Func()`
//    // is annotated `SK_UNSAFE_BUFFER_USAGE`.
//    return SK_UNSAFE_BUFFERS(Func(input, end));
//
// Test for `__clang__` directly, as there's no `__has_pragma` or similar (see
// https://github.com/llvm/llvm-project/issues/51887).
#if defined(__clang__) && defined(__has_warning)
#  if __has_warning("-Wunsafe-buffer-usage")
#    define SK_UNSAFE_BUFFERS(...) \
       _Pragma("clang unsafe_buffer_usage begin") \
       __VA_ARGS__ \
       _Pragma("clang unsafe_buffer_usage end")
#  endif
#endif

#ifndef SK_UNSAFE_BUFFERS
#  define SK_UNSAFE_BUFFERS(...) __VA_ARGS__
#endif

// Temporarily exempts code from `-Wunsafe-buffer-usage`.
// Functionally identical to `SK_UNSAFE_BUFFERS()`, but semantically flags the
// code for future migration to safer patterns.
//
// Usage: return SK_UNSAFE_TODO(Func(input, end));
#define SK_UNSAFE_TODO(...) SK_UNSAFE_BUFFERS(__VA_ARGS__)

#endif // SkAttributes_DEFINED
