/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypes_DEFINED
#define SkTypes_DEFINED

// IWYU pragma: begin_exports

// In at least two known scenarios when using GCC with libc++:
//  * GCC 4.8 targeting ARMv7 with NEON
//  * GCC 4.9 targeting ARMv8 64 bit
// we need to typedef float float32_t (or include <arm_neon.h> which does that)
// before #including <memory>. This makes no sense.  I'm not very interested in
// understanding why... these are old, bizarre platform configuration that we
// should just let die.
// See https://llvm.org/bugs/show_bug.cgi?id=25608 .
#include <ciso646>  // Include something innocuous to define _LIBCPP_VERISON if it's libc++.
#if defined(__GNUC__) && __GNUC__ == 4 \
 && ((defined(__arm__) && (defined(__ARM_NEON__) || defined(__ARM_NEON))) || defined(__aarch64__)) \
 && defined(_LIBCPP_VERSION)
    typedef float float32_t;
    #include <memory>
#endif

#include "SkPreConfig.h"
#include "SkUserConfig.h"
#include "SkPostConfig.h"

#include <cstddef>
#include <cstdint>
// IWYU pragma: end_exports

#include <cstring>

#include "../private/SkNumbers.h"
#include "../private/SkTFitsIn.h"

/** \file SkTypes.h
*/

/** See SkGraphics::GetVersion() to retrieve these at runtime
 */
#define SKIA_VERSION_MAJOR  1
#define SKIA_VERSION_MINOR  0
#define SKIA_VERSION_PATCH  0


/** Called internally if we hit an unrecoverable error.
    The platform implementation must not return, but should either throw
    an exception or otherwise exit.
*/
SK_API extern void sk_abort_no_print(void);

#define SK_INIT_TO_AVOID_WARNING    = 0

#ifndef SkDebugf
    SK_API void SkDebugf(const char format[], ...);
#endif

// SkASSERT, SkASSERTF and SkASSERT_RELEASE can be used as stand alone assertion expressions, e.g.
//    uint32_t foo(int x) {
//        SkASSERT(x > 4);
//        return x - 4;
//    }
// and are also written to be compatible with constexpr functions:
//    constexpr uint32_t foo(int x) {
//        return SkASSERT(x > 4),
//               x - 4;
//    }
#define SkASSERT_RELEASE(cond) \
        static_cast<void>( (cond) ? (void)0 : []{ SK_ABORT("assert(" #cond ")"); }() )

#ifdef SK_DEBUG
    #define SkASSERT(cond) SkASSERT_RELEASE(cond)
    #define SkASSERTF(cond, fmt, ...) static_cast<void>( (cond) ? (void)0 : [&]{ \
                                          SkDebugf(fmt"\n", __VA_ARGS__);        \
                                          SK_ABORT("assert(" #cond ")");         \
                                      }() )
    #define SkDEBUGFAIL(message)        SK_ABORT(message)
    #define SkDEBUGFAILF(fmt, ...)      SkASSERTF(false, fmt, ##__VA_ARGS__)
    #define SkDEBUGCODE(...)            __VA_ARGS__
    #define SkDEBUGF(args       )       SkDebugf args
    #define SkAssertResult(cond)        SkASSERT(cond)
#else
    #define SkASSERT(cond)            static_cast<void>(0)
    #define SkASSERTF(cond, fmt, ...) static_cast<void>(0)
    #define SkDEBUGFAIL(message)
    #define SkDEBUGFAILF(fmt, ...)
    #define SkDEBUGCODE(...)
    #define SkDEBUGF(args)

    // unlike SkASSERT, this guy executes its condition in the non-debug build.
    // The if is present so that this can be used with functions marked SK_WARN_UNUSED_RESULT.
    #define SkAssertResult(cond)         if (cond) {} do {} while(false)
#endif

// some clients (e.g. third_party/WebKit/Source/platform/fonts/FontCustomPlatformData.h)
// depend on SkString forward declaration below. Remove this once dependencies are fixed.
class SkString;

/*
 *  Usage:  SK_MACRO_CONCAT(a, b)   to construct the symbol ab
 *
 *  SK_MACRO_CONCAT_IMPL_PRIV just exists to make this work. Do not use directly
 *
 */
#define SK_MACRO_CONCAT(X, Y)           SK_MACRO_CONCAT_IMPL_PRIV(X, Y)
#define SK_MACRO_CONCAT_IMPL_PRIV(X, Y)  X ## Y

/*
 *  Usage: SK_MACRO_APPEND_LINE(foo)    to make foo123, where 123 is the current
 *                                      line number. Easy way to construct
 *                                      unique names for local functions or
 *                                      variables.
 */
#define SK_MACRO_APPEND_LINE(name)  SK_MACRO_CONCAT(name, __LINE__)

/**
 * For some classes, it's almost always an error to instantiate one without a name, e.g.
 *   {
 *       SkAutoMutexAcquire(&mutex);
 *       <some code>
 *   }
 * In this case, the writer meant to hold mutex while the rest of the code in the block runs,
 * but instead the mutex is acquired and then immediately released.  The correct usage is
 *   {
 *       SkAutoMutexAcquire lock(&mutex);
 *       <some code>
 *   }
 *
 * To prevent callers from instantiating your class without a name, use SK_REQUIRE_LOCAL_VAR
 * like this:
 *   class classname {
 *       <your class>
 *   };
 *   #define classname(...) SK_REQUIRE_LOCAL_VAR(classname)
 *
 * This won't work with templates, and you must inline the class' constructors and destructors.
 * Take a look at SkAutoFree and SkAutoMalloc in this file for examples.
 */
#define SK_REQUIRE_LOCAL_VAR(classname) \
    static_assert(false, "missing name for " #classname)

///////////////////////////////////////////////////////////////////////

/**
 *  Fast type for signed 8 bits. Use for parameter passing and local variables,
 *  not for storage.
 */
typedef int S8CPU;

/**
 *  Fast type for unsigned 8 bits. Use for parameter passing and local
 *  variables, not for storage
 */
typedef unsigned U8CPU;

/**
 *  Fast type for signed 16 bits. Use for parameter passing and local variables,
 *  not for storage
 */
typedef int S16CPU;

/**
 *  Fast type for unsigned 16 bits. Use for parameter passing and local
 *  variables, not for storage
 */
typedef unsigned U16CPU;

/**
 *  Meant to be a small version of bool, for storage purposes. Will be 0 or 1
 */
typedef uint8_t SkBool8;

template <typename D, typename S> constexpr D SkTo(S s) {
    return SkASSERT(SkTFitsIn<D>(s)),
           static_cast<D>(s);
}
#define SkToS8(x)    SkTo<int8_t>(x)
#define SkToU8(x)    SkTo<uint8_t>(x)
#define SkToS16(x)   SkTo<int16_t>(x)
#define SkToU16(x)   SkTo<uint16_t>(x)
#define SkToS32(x)   SkTo<int32_t>(x)
#define SkToU32(x)   SkTo<uint32_t>(x)
#define SkToInt(x)   SkTo<int>(x)
#define SkToUInt(x)  SkTo<unsigned>(x)
#define SkToSizeT(x) SkTo<size_t>(x)

/** Returns 0 or 1 based on the condition
*/
#define SkToBool(cond)  ((cond) != 0)

//////////////////////////////////////////////////////////////////////////////

// Can be used to bracket data types that must be dense, e.g. hash keys.
#if defined(__clang__)  // This should work on GCC too, but GCC diagnostic pop didn't seem to work!
    #define SK_BEGIN_REQUIRE_DENSE _Pragma("GCC diagnostic push") \
                                   _Pragma("GCC diagnostic error \"-Wpadded\"")
    #define SK_END_REQUIRE_DENSE   _Pragma("GCC diagnostic pop")
#else
    #define SK_BEGIN_REQUIRE_DENSE
    #define SK_END_REQUIRE_DENSE
#endif


typedef uint32_t SkFourByteTag;
constexpr inline SkFourByteTag SkSetFourByteTag(char a, char b, char c, char d) {
    return ((uint8_t)a << 24) | ((uint8_t)b << 16) | ((uint8_t)c << 8) | ((uint8_t)d << 0);
}

/** 32 bit integer to hold a unicode value
*/
typedef int32_t SkUnichar;

/** 16 bit unsigned integer to hold a glyph index
*/
typedef uint16_t SkGlyphID;

/** 32 bit value to hold a millisecond duration
 *  Note that SK_MSecMax is about 25 days.
 */
typedef uint32_t SkMSec;
/** 1 second measured in milliseconds
*/
#define SK_MSec1 1000
/** maximum representable milliseconds; 24d 20h 31m 23.647s.
*/
#define SK_MSecMax 0x7FFFFFFF
/** Returns a < b for milliseconds, correctly handling wrap-around from 0xFFFFFFFF to 0
*/
#define SkMSec_LT(a, b)     ((int32_t)(a) - (int32_t)(b) < 0)
/** Returns a <= b for milliseconds, correctly handling wrap-around from 0xFFFFFFFF to 0
*/
#define SkMSec_LE(a, b)     ((int32_t)(a) - (int32_t)(b) <= 0)

/** The generation IDs in Skia reserve 0 has an invalid marker.
 */
#define SK_InvalidGenID     0
/** The unique IDs in Skia reserve 0 has an invalid marker.
 */
#define SK_InvalidUniqueID  0


/** Generic swap function. Classes with efficient swaps should specialize this function to take
    their fast path. This function is used by SkTSort. */
template <typename T> static inline void SkTSwap(T& a, T& b) {
    T c(std::move(a));
    a = std::move(b);
    b = std::move(c);
}

static inline int32_t SkAbs32(int32_t value) {
    SkASSERT(value != SK_NaN32);  // The most negative int32_t can't be negated.
    if (value < 0) {
        value = -value;
    }
    return value;
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  Indicates whether an allocation should count against a cache budget.
 */
enum class SkBudgeted : bool {
    kNo  = false,
    kYes = true
};

/**
 * Indicates whether a backing store needs to be an exact match or can be larger
 * than is strictly necessary
 */
enum class SkBackingFit {
    kApprox,
    kExact
};

///////////////////////////////////////////////////////////////////////////////

/** \class SkNoncopyable

SkNoncopyable is the base class for objects that do not want to
be copied. It hides its copy-constructor and its assignment-operator.
*/
class SK_API SkNoncopyable {
public:
    SkNoncopyable() = default;

    SkNoncopyable(SkNoncopyable&&) = default;
    SkNoncopyable& operator =(SkNoncopyable&&) = default;

    SkNoncopyable(const SkNoncopyable&) = delete;
    SkNoncopyable& operator=(const SkNoncopyable&) = delete;
};

#endif
