
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

#if !defined(GR_ANDROID_BUILD)
    #define GR_ANDROID_BUILD    0
#endif
#if !defined(GR_IOS_BUILD)
    #define GR_IOS_BUILD        0
#endif
#if !defined(GR_LINUX_BUILD)
    #define GR_LINUX_BUILD      0
#endif
#if !defined(GR_MAC_BUILD)
    #define GR_MAC_BUILD        0
#endif
#if !defined(GR_WIN32_BUILD)
    #define GR_WIN32_BUILD      0
#endif
#if !defined(GR_QNX_BUILD)
    #define GR_QNX_BUILD        0
#endif
#if !defined(GR_CACHE_STATS)
    #define GR_CACHE_STATS      0
#endif

/**
 *  If no build target has been defined, attempt to infer.
 */
#if !GR_ANDROID_BUILD && !GR_IOS_BUILD && !GR_LINUX_BUILD && !GR_MAC_BUILD && !GR_WIN32_BUILD && !GR_QNX_BUILD
    #if defined(_WIN32)
        #undef GR_WIN32_BUILD
        #define GR_WIN32_BUILD      1
//      #error "WIN"
    #elif TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #undef GR_IOS_BUILD
        #define GR_IOS_BUILD        1
//      #error "IOS"
    #elif defined(SK_BUILD_FOR_ANDROID)
        #undef GR_ANDROID_BUILD
        #define GR_ANDROID_BUILD    1
//      #error "ANDROID"
    #elif TARGET_OS_MAC
        #undef GR_MAC_BUILD
        #define GR_MAC_BUILD        1
//      #error "MAC"
    #elif TARGET_OS_QNX || defined(__QNXNTO__)
        #undef GR_QNX_BUILD
        #define GR_QNX_BUILD        1
//      #error "QNX"
    #else
        #undef GR_LINUX_BUILD
        #define GR_LINUX_BUILD      1
//      #error "LINUX"
    #endif
#endif

// we need both GR_DEBUG and GR_RELEASE to be defined as 0 or 1
//
#ifndef GR_DEBUG
    #ifdef GR_RELEASE
        #define GR_DEBUG !GR_RELEASE
    #else
        #ifdef NDEBUG
            #define GR_DEBUG    0
        #else
            #define GR_DEBUG    1
        #endif
    #endif
#endif

#ifndef GR_RELEASE
    #define GR_RELEASE  !GR_DEBUG
#endif

#if GR_DEBUG == GR_RELEASE
    #error "GR_DEBUG and GR_RELEASE must not be the same"
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if GR_WIN32_BUILD
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

/*
 *  The "user config" file can be empty, and everything should work. It is
 *  meant to store a given platform/client's overrides of our guess-work.
 *
 *  A alternate user config file can be specified by defining
 *  GR_USER_CONFIG_FILE. It should be defined relative to GrConfig.h
 *
 *  e.g. it can specify GR_DEBUG/GR_RELEASE as it please, change the BUILD
 *  target, or supply its own defines for anything else (e.g. GR_DEFAULT_TEXTURE_CACHE_MB_LIMIT)
 */
#if !defined(GR_USER_CONFIG_FILE)
    #include "GrUserConfig.h"
#else
    #include GR_USER_CONFIG_FILE
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// postconfig section:
//

// GR_IMPLEMENTATION should be define to 1 when building Gr and 0 when including
// it in another dependent build. The Gr makefile/ide-project should define this
// to 1.
#if !defined(GR_IMPLEMENTATION)
    #define GR_IMPLEMENTATION 0
#endif

// If Gr is built as a shared library then GR_DLL should be defined to 1 (both
// when building Gr and when including its headers in dependent builds). Only
// currently supported minimally for Chrome's Win32 Multi-DLL build (TODO:
// correctly exort all of the public API correctly and support shared lib on
// other platforms).
#if !defined(GR_DLL)
    #define GR_DLL 0
#endif

#if GR_DLL
    #if GR_WIN32_BUILD
        #if GR_IMPLEMENTATION
            #define GR_API __declspec(dllexport)
        #else
            #define GR_API __declspec(dllimport)
        #endif
    #else
        #define GR_API __attribute__((visibility("default")))
    #endif
#else
    #define GR_API
#endif

// By now we must have a GR_..._BUILD symbol set to 1, and a decision about
// debug -vs- release
//

#define GrPrintf SkDebugf

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
#if defined(_MSC_VER) && _MSC_VER
    #define GR_WARN(MSG) (GR_FILE_AND_LINE_STR "WARNING: " MSG)
#else//__GNUC__ - may need other defines for different compilers
    #define GR_WARN(MSG) ("WARNING: " MSG)
#endif

/**
 *  GR_ALWAYSBREAK is an unconditional break in all builds.
 */
#if !defined(GR_ALWAYSBREAK)
    #if     GR_WIN32_BUILD
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
    #if GR_DEBUG
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
                GrPrintf("%s %s failed\n", GR_FILE_AND_LINE_STR, #COND); \
                GR_ALWAYSBREAK;                                          \
            }                                                            \
        } while (false)
#endif

/**
 *  GR_DEBUGASSERT is an assertion in debug builds only.
 */
#if !defined(GR_DEBUGASSERT)
    #if GR_DEBUG
        #define GR_DEBUGASSERT(COND) GR_ALWAYSASSERT(COND)
    #else
        #define GR_DEBUGASSERT(COND)
    #endif
#endif

/**
 *  Prettier forms of the above macros.
 */
#define GrAssert(COND) GR_DEBUGASSERT(COND)
#define GrAlwaysAssert(COND) GR_ALWAYSASSERT(COND)

/**
 * Crash from unrecoverable condition, optionally with a message.
 */
inline void GrCrash() { GrAlwaysAssert(false); }
inline void GrCrash(const char* msg) { GrPrintf(msg); GrAlwaysAssert(false); }

/**
 *  GR_DEBUGCODE compiles the code X in debug builds only
 */
#if !defined(GR_DEBUGCODE)
    #if GR_DEBUG
        #define GR_DEBUGCODE(X) X
    #else
        #define GR_DEBUGCODE(X)
    #endif
#endif

/**
 *  GR_STATIC_ASSERT is a compile time assertion. Depending on the platform
 *  it may print the message in the compiler log. Obviously, the condition must
 *  be evaluatable at compile time.
 */
// VS 2010 and GCC compiled with c++0x or gnu++0x support the new
// static_assert.
#if !defined(GR_STATIC_ASSERT)
    #if (defined(_MSC_VER) && _MSC_VER >= 1600) || (defined(__GXX_EXPERIMENTAL_CXX0X__) && __GXX_EXPERIMENTAL_CXX0X__)
        #define GR_STATIC_ASSERT(CONDITION) static_assert(CONDITION, "bug")
    #else
        template <bool> class GR_STATIC_ASSERT_FAILURE;
        template <> class GR_STATIC_ASSERT_FAILURE<true> {};
        #define GR_STATIC_ASSERT(CONDITION) \
            enum {GR_CONCAT(X,__LINE__) = \
            sizeof(GR_STATIC_ASSERT_FAILURE<CONDITION>)}
    #endif
#endif

/**
 *  GR_STATIC_RECT_VB controls whether rects are drawn by issuing a vertex
 *  for each corner or using a static vb that is positioned by modifying the
 *  view / texture matrix.
 */
#if !defined(GR_STATIC_RECT_VB)
    #define GR_STATIC_RECT_VB 0
#endif

/**
 * GR_GEOM_BUFFER_LOCK_THRESHOLD gives a threshold (in bytes) for when Gr should
 * lock a GrGeometryBuffer to update its contents. It will use lock() if the
 * size of the updated region is greater than the threshold. Otherwise it will
 * use updateData().
 */
#if !defined(GR_GEOM_BUFFER_LOCK_THRESHOLD)
    #define GR_GEOM_BUFFER_LOCK_THRESHOLD (1 << 15)
#endif

/**
 * GR_DEFAULT_TEXTURE_CACHE_MB_LIMIT gives a threshold (in megabytes) for the
 * maximum size of the texture cache in vram. The value is only a default and
 * can be overridden at runtime.
 */
#if !defined(GR_DEFAULT_TEXTURE_CACHE_MB_LIMIT)
    #define GR_DEFAULT_TEXTURE_CACHE_MB_LIMIT 96
#endif

/**
 * GR_STROKE_PATH_RENDERING controls whether or not the GrStrokePathRenderer can be selected
 * as a path renderer. GrStrokePathRenderer is currently an experimental path renderer.
 */
#if !defined(GR_STROKE_PATH_RENDERING)
    #define GR_STROKE_PATH_RENDERING                 0
#endif

///////////////////////////////////////////////////////////////////////////////
// tail section:
//
// Now we just assert if we are missing some required define, or if we detect
// and inconsistent combination of defines
//


/**
 *  Only one build target macro should be 1 and the rest should be 0.
 */
#define GR_BUILD_SUM    (GR_WIN32_BUILD + GR_MAC_BUILD + GR_IOS_BUILD + GR_ANDROID_BUILD + GR_LINUX_BUILD + GR_QNX_BUILD)
#if 0 == GR_BUILD_SUM
    #error "Missing a GR_BUILD define"
#elif 1 != GR_BUILD_SUM
    #error "More than one GR_BUILD defined"
#endif

#if 0
#if GR_WIN32_BUILD
//    #pragma message GR_WARN("GR_WIN32_BUILD")
#endif
#if GR_MAC_BUILD
//    #pragma message GR_WARN("GR_MAC_BUILD")
#endif
#if GR_IOS_BUILD
//    #pragma message GR_WARN("GR_IOS_BUILD")
#endif
#if GR_ANDROID_BUILD
//    #pragma message GR_WARN("GR_ANDROID_BUILD")
#endif
#if GR_LINUX_BUILD
//    #pragma message GR_WARN("GR_LINUX_BUILD")
#endif
#if GR_QNX_BUILD
//    #pragma message GR_WARN("GR_QNX_BUILD")
#endif
#endif

#endif
