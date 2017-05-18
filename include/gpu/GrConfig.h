
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrConfig_DEFINED
#define GrConfig_DEFINED

#include "SkTypes.h"

///////////////////////////////////////////////////////////////////////////////
// preconfig section:
//
// All the work before including GrUserConfig.h should center around guessing
// what platform we're on, and defining low-level symbols based on that.
//
// A build environment may have already defined symbols, so we first check
// for that
//

// hack to ensure we know what sort of Apple platform we're on
#if defined(__APPLE_CPP__) || defined(__APPLE_CC__)
    #include <TargetConditionals.h>
#endif

/**
 *  Gr defines are set to 0 or 1, rather than being undefined or defined
 */

#if !defined(GR_CACHE_STATS)
  #if defined(SK_DEBUG) || defined(SK_DUMP_STATS)
      #define GR_CACHE_STATS  1
  #else
      #define GR_CACHE_STATS  0
  #endif
#endif

#if !defined(GR_GPU_STATS)
  #if defined(SK_DEBUG) || defined(SK_DUMP_STATS)
      #define GR_GPU_STATS    1
  #else
      #define GR_GPU_STATS    0
  #endif
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(SK_BUILD_FOR_WIN32)
// VC8 doesn't support stdint.h, so we define those types here.
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
/*
 *  Include stdint.h with defines that trigger declaration of C99 limit/const
 *  macros here before anyone else has a chance to include stdint.h without
 *  these.
 */
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#include <stdint.h>
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// postconfig section:
//

/**
 *  GR_STRING makes a string of X where X is expanded before conversion to a string
 *  if X itself contains macros.
 */
#define GR_STRING(X) GR_STRING_IMPL(X)
#define GR_STRING_IMPL(X) #X

/**
 *  GR_CONCAT concatenates X and Y  where each is expanded before
 *  contanenation if either contains macros.
 */
#define GR_CONCAT(X,Y) GR_CONCAT_IMPL(X,Y)
#define GR_CONCAT_IMPL(X,Y) X##Y

/**
 *  Creates a string of the form "<filename>(<linenumber>) : "
 */
#define GR_FILE_AND_LINE_STR __FILE__ "(" GR_STRING(__LINE__) ") : "

/**
 *  Compilers have different ways of issuing warnings. This macro
 *  attempts to abstract them, but may need to be specialized for your
 *  particular compiler.
 *  To insert compiler warnings use "#pragma message GR_WARN(<string>)"
 */
#if defined(_MSC_VER)
    #define GR_WARN(MSG) (GR_FILE_AND_LINE_STR "WARNING: " MSG)
#else//__GNUC__ - may need other defines for different compilers
    #define GR_WARN(MSG) ("WARNING: " MSG)
#endif

/**
 *  GR_ALWAYSBREAK is an unconditional break in all builds.
 */
#if !defined(GR_ALWAYSBREAK)
    #if     defined(SK_BUILD_FOR_WIN32)
        #define GR_ALWAYSBREAK SkNO_RETURN_HINT(); __debugbreak()
    #else
        // TODO: do other platforms really not have continuable breakpoints?
        // sign extend for 64bit architectures to be sure this is
        // in the high address range
        #define GR_ALWAYSBREAK SkNO_RETURN_HINT(); *((int*)(int64_t)(int32_t)0xbeefcafe) = 0;
    #endif
#endif

/**
 *  GR_DEBUGBREAK is an unconditional break in debug builds.
 */
#if !defined(GR_DEBUGBREAK)
    #ifdef SK_DEBUG
        #define GR_DEBUGBREAK GR_ALWAYSBREAK
    #else
        #define GR_DEBUGBREAK
    #endif
#endif

/**
 *  GR_ALWAYSASSERT is an assertion in all builds.
 */
#if !defined(GR_ALWAYSASSERT)
    #define GR_ALWAYSASSERT(COND)                                        \
        do {                                                             \
            if (!(COND)) {                                               \
                SkDebugf("%s %s failed\n", GR_FILE_AND_LINE_STR, #COND); \
                GR_ALWAYSBREAK;                                          \
            }                                                            \
        } while (false)
#endif

/**
 *  GR_DEBUGASSERT is an assertion in debug builds only.
 */
#if !defined(GR_DEBUGASSERT)
    #ifdef SK_DEBUG
        #define GR_DEBUGASSERT(COND) GR_ALWAYSASSERT(COND)
    #else
        #define GR_DEBUGASSERT(COND)
    #endif
#endif

/**
 *  Prettier forms of the above macros.
 */
#define GrAlwaysAssert(COND) GR_ALWAYSASSERT(COND)

/**
 *  GR_STATIC_ASSERT is a compile time assertion. Depending on the platform
 *  it may print the message in the compiler log. Obviously, the condition must
 *  be evaluatable at compile time.
 */
#define GR_STATIC_ASSERT(CONDITION) static_assert(CONDITION, "bug")

#endif
