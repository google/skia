
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPreConfig_DEFINED
#define SkPreConfig_DEFINED

#ifdef WEBKIT_VERSION_MIN_REQUIRED
    #include "config.h"
#endif

// Allows embedders that want to disable macros that take arguments to just
// define that symbol to be one of these
//
#define SK_NOTHING_ARG1(arg1)
#define SK_NOTHING_ARG2(arg1, arg2)
#define SK_NOTHING_ARG3(arg1, arg2, arg3)

//////////////////////////////////////////////////////////////////////

#if !defined(SK_BUILD_FOR_ANDROID) && !defined(SK_BUILD_FOR_IOS) && !defined(SK_BUILD_FOR_PALM) && !defined(SK_BUILD_FOR_WINCE) && !defined(SK_BUILD_FOR_WIN32) && !defined(SK_BUILD_FOR_UNIX) && !defined(SK_BUILD_FOR_MAC) && !defined(SK_BUILD_FOR_SDL) && !defined(SK_BUILD_FOR_BREW) && !defined(SK_BUILD_FOR_NACL)

    #ifdef __APPLE__
        #include "TargetConditionals.h"
    #endif

    #if defined(PALMOS_SDK_VERSION)
        #define SK_BUILD_FOR_PALM
    #elif defined(UNDER_CE)
        #define SK_BUILD_FOR_WINCE
    #elif defined(WIN32)
        #define SK_BUILD_FOR_WIN32
    #elif defined(__SYMBIAN32__)
        #define SK_BUILD_FOR_WIN32
    #elif defined(ANDROID)
        #define SK_BUILD_FOR_ANDROID
    #elif defined(linux) || defined(__FreeBSD__) || defined(__OpenBSD__) || \
          defined(__sun) || defined(__NetBSD__) || defined(__DragonFly__) || \
          defined(__GLIBC__) || defined(__GNU__)
        #define SK_BUILD_FOR_UNIX
    #elif TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #define SK_BUILD_FOR_IOS
    #else
        #define SK_BUILD_FOR_MAC
    #endif

#endif

/* Even if the user only defined the framework variant we still need to build
 * the default (NDK-compliant) Android code. Therefore, when attempting to
 * include/exclude something from the framework variant check first that we are
 * building for Android then check the status of the framework define.
 */
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && !defined(SK_BUILD_FOR_ANDROID)
    #define SK_BUILD_FOR_ANDROID
#endif

//////////////////////////////////////////////////////////////////////

#if !defined(SK_DEBUG) && !defined(SK_RELEASE)
    #ifdef NDEBUG
        #define SK_RELEASE
    #else
        #define SK_DEBUG
    #endif
#endif

#ifdef SK_BUILD_FOR_WIN32
    #if !defined(SK_RESTRICT)
        #define SK_RESTRICT __restrict
    #endif
    #if !defined(SK_WARN_UNUSED_RESULT)
        #define SK_WARN_UNUSED_RESULT
    #endif
#endif

//////////////////////////////////////////////////////////////////////

#if !defined(SK_RESTRICT)
    #define SK_RESTRICT __restrict__
#endif

#if !defined(SK_WARN_UNUSED_RESULT)
    #define SK_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#endif

//////////////////////////////////////////////////////////////////////

#if !defined(SK_CPU_BENDIAN) && !defined(SK_CPU_LENDIAN)
    #if defined(__sparc) || defined(__sparc__) || \
      defined(_POWER) || defined(__powerpc__) || \
      defined(__ppc__) || defined(__hppa) || \
      defined(__PPC__) || defined(__PPC64__) || \
      defined(_MIPSEB) || defined(__ARMEB__) || \
      defined(__s390__) || \
      (defined(__sh__) && defined(__BIG_ENDIAN__)) || \
      (defined(__ia64) && defined(__BIG_ENDIAN__))
         #define SK_CPU_BENDIAN
    #else
        #define SK_CPU_LENDIAN
    #endif
#endif

//////////////////////////////////////////////////////////////////////

#if defined(__i386) || defined(_M_IX86) ||  defined(__x86_64__) || defined(_M_X64)
  #define SK_CPU_X86 1
#endif

/**
 *  SK_CPU_SSE_LEVEL
 *
 *  If defined, SK_CPU_SSE_LEVEL should be set to the highest supported level.
 *  On non-intel CPU this should be undefined.
 */

#define SK_CPU_SSE_LEVEL_SSE1     10
#define SK_CPU_SSE_LEVEL_SSE2     20
#define SK_CPU_SSE_LEVEL_SSE3     30
#define SK_CPU_SSE_LEVEL_SSSE3    31
#define SK_CPU_SSE_LEVEL_SSE41    41
#define SK_CPU_SSE_LEVEL_SSE42    42

// Are we in GCC?
#ifndef SK_CPU_SSE_LEVEL
    // These checks must be done in descending order to ensure we set the highest
    // available SSE level.
    #if defined(__SSE4_2__)
        #define SK_CPU_SSE_LEVEL    SK_CPU_SSE_LEVEL_SSE42
    #elif defined(__SSE4_1__)
        #define SK_CPU_SSE_LEVEL    SK_CPU_SSE_LEVEL_SSE41
    #elif defined(__SSSE3__)
        #define SK_CPU_SSE_LEVEL    SK_CPU_SSE_LEVEL_SSSE3
    #elif defined(__SSE3__)
        #define SK_CPU_SSE_LEVEL    SK_CPU_SSE_LEVEL_SSE3
    #elif defined(__SSE2__)
        #define SK_CPU_SSE_LEVEL    SK_CPU_SSE_LEVEL_SSE2
    #endif
#endif

// Are we in VisualStudio?
#ifndef SK_CPU_SSE_LEVEL
    // These checks must be done in descending order to ensure we set the highest
    // available SSE level. 64-bit intel guarantees at least SSE2 support.
    #if defined(_M_X64) || defined(_M_AMD64)
        #define SK_CPU_SSE_LEVEL    SK_CPU_SSE_LEVEL_SSE2
    #elif defined (_M_IX86_FP)
        #if _M_IX86_FP >= 2
            #define SK_CPU_SSE_LEVEL    SK_CPU_SSE_LEVEL_SSE2
        #elif _M_IX86_FP == 1
            #define SK_CPU_SSE_LEVEL    SK_CPU_SSE_LEVEL_SSE1
        #endif
    #endif
#endif

//////////////////////////////////////////////////////////////////////
// ARM defines

#if defined(__arm__) && (!defined(__APPLE__) || !TARGET_IPHONE_SIMULATOR)
    #define SK_CPU_ARM32

    #if defined(__GNUC__)
        #if defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) \
                || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) \
                || defined(__ARM_ARCH_7EM__) || defined(_ARM_ARCH_7)
            #define SK_ARM_ARCH 7
        #elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) \
                || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) \
                || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6T2__) \
                || defined(__ARM_ARCH_6M__) || defined(_ARM_ARCH_6)
            #define SK_ARM_ARCH 6
        #elif defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5T__) \
                || defined(__ARM_ARCH_5E__) || defined(__ARM_ARCH_5TE__) \
                || defined(__ARM_ARCH_5TEJ__) || defined(_ARM_ARCH_5)
            #define SK_ARM_ARCH 5
        #elif defined(__ARM_ARCH_4__) || defined(__ARM_ARCH_4T__) || defined(_ARM_ARCH_4)
            #define SK_ARM_ARCH 4
        #else
            #define SK_ARM_ARCH 3
        #endif

        #if defined(__thumb2__) && (SK_ARM_ARCH >= 6) \
                || !defined(__thumb__) && ((SK_ARM_ARCH > 5) || defined(__ARM_ARCH_5E__) \
                || defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_5TEJ__))
            #define SK_ARM_HAS_EDSP
        #endif
    #endif
#endif

// Disable ARM64 optimizations for iOS due to complications regarding gyp and iOS.
#if defined(__aarch64__) && !defined(SK_BUILD_FOR_IOS)
    #define SK_CPU_ARM64
#endif

//////////////////////////////////////////////////////////////////////

#if !defined(SKIA_IMPLEMENTATION)
    #define SKIA_IMPLEMENTATION 0
#endif

#if defined(SKIA_DLL)
    #if defined(WIN32)
        #if SKIA_IMPLEMENTATION
            #define SK_API __declspec(dllexport)
        #else
            #define SK_API __declspec(dllimport)
        #endif
    #else
        #define SK_API __attribute__((visibility("default")))
    #endif
#else
    #define SK_API
#endif

//////////////////////////////////////////////////////////////////////

/**
 * Use SK_PURE_FUNC as an attribute to indicate that a function's
 * return value only depends on the value of its parameters. This
 * can help the compiler optimize out successive calls.
 *
 * Usage:
 *      void  function(int params)  SK_PURE_FUNC;
 */
#if defined(__GNUC__)
#  define  SK_PURE_FUNC  __attribute__((pure))
#else
#  define  SK_PURE_FUNC  /* nothing */
#endif

//////////////////////////////////////////////////////////////////////

/**
 * SK_HAS_ATTRIBUTE(<name>) should return true iff the compiler
 * supports __attribute__((<name>)). Mostly important because
 * Clang doesn't support all of GCC attributes.
 */
#if defined(__has_attribute)
#   define SK_HAS_ATTRIBUTE(x) __has_attribute(x)
#elif defined(__GNUC__)
#   define SK_HAS_ATTRIBUTE(x) 1
#else
#   define SK_HAS_ATTRIBUTE(x) 0
#endif

/**
 * SK_ATTRIBUTE_OPTIMIZE_O1 can be used as a function attribute
 * to specify individual optimization level of -O1, if the compiler
 * supports it.
 *
 * NOTE: Clang/ARM (r161757) does not support the 'optimize' attribute.
 */
#if SK_HAS_ATTRIBUTE(optimize)
#   define SK_ATTRIBUTE_OPTIMIZE_O1 __attribute__((optimize("O1")))
#else
#   define SK_ATTRIBUTE_OPTIMIZE_O1 /* nothing */
#endif

#endif
