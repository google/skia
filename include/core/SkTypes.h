/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypes_DEFINED
#define SkTypes_DEFINED

// IWYU pragma: begin_exports
#include "SkPreConfig.h"
#include "SkUserConfig.h"
#include "SkPostConfig.h"
#include <stddef.h>
#include <stdint.h>
// IWYU pragma: end_exports

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
    #define SkDEBUGF(...)               SkDebugf(__VA_ARGS__)
    #define SkAssertResult(cond)        SkASSERT(cond)
#else
    #define SkASSERT(cond)            static_cast<void>(0)
    #define SkASSERTF(cond, fmt, ...) static_cast<void>(0)
    #define SkDEBUGFAIL(message)
    #define SkDEBUGFAILF(fmt, ...)
    #define SkDEBUGCODE(...)
    #define SkDEBUGF(...)

    // unlike SkASSERT, this macro executes its condition in the non-debug build.
    // The if is present so that this can be used with functions marked SK_WARN_UNUSED_RESULT.
    #define SkAssertResult(cond)         if (cond) {} do {} while(false)
#endif

////////////////////////////////////////////////////////////////////////////////

/** Fast type for unsigned 8 bits. Use for parameter passing and local
    variables, not for storage
*/
typedef unsigned U8CPU;

/** Fast type for unsigned 16 bits. Use for parameter passing and local
    variables, not for storage
*/
typedef unsigned U16CPU;

/** @return false or true based on the condition
*/
template <typename T> static constexpr bool SkToBool(const T& x) { return 0 != x; }

static constexpr int16_t SK_MaxS16 = INT16_MAX;
static constexpr int16_t SK_MinS16 = -SK_MaxS16;

static constexpr int32_t SK_MaxS32 = INT32_MAX;
static constexpr int32_t SK_MinS32 = -SK_MaxS32;
static constexpr int32_t SK_NaN32  = INT32_MIN;

static constexpr int64_t SK_MaxS64 = INT64_MAX;
static constexpr int64_t SK_MinS64 = -SK_MaxS64;

////////////////////////////////////////////////////////////////////////////////

/** @return the number of entries in an array (not a pointer)
*/
template <typename T, size_t N> char (&SkArrayCountHelper(T (&array)[N]))[N];
#define SK_ARRAY_COUNT(array) (sizeof(SkArrayCountHelper(array)))

typedef uint32_t SkFourByteTag;
static inline constexpr SkFourByteTag SkSetFourByteTag(char a, char b, char c, char d) {
    return (((uint8_t)a << 24) | ((uint8_t)b << 16) | ((uint8_t)c << 8) | (uint8_t)d);
}

////////////////////////////////////////////////////////////////////////////////

/** 32 bit integer to hold a unicode value
*/
typedef int32_t SkUnichar;

/** 16 bit unsigned integer to hold a glyph index
*/
typedef uint16_t SkGlyphID;

/** 32 bit value to hold a millisecond duration
    Note that SK_MSecMax is about 25 days.
*/
typedef uint32_t SkMSec;

/** Maximum representable milliseconds; 24d 20h 31m 23.647s.
*/
static constexpr SkMSec SK_MSecMax = INT32_MAX;

/** The generation IDs in Skia reserve 0 has an invalid marker.
*/
static constexpr uint32_t SK_InvalidGenID = 0;

/** The unique IDs in Skia reserve 0 has an invalid marker.
*/
static constexpr uint32_t SK_InvalidUniqueID = 0;

////////////////////////////////////////////////////////////////////////////////

/** Indicates whether an allocation should count against a cache budget.
*/
enum class SkBudgeted : bool {
    kNo  = false,
    kYes = true
};

/** Indicates whether a backing store needs to be an exact match or can be
    larger than is strictly necessary
*/
enum class SkBackingFit {
    kApprox,
    kExact
};

#endif
