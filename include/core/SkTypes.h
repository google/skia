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
#include <stddef.h>
#include <stdint.h>
// IWYU pragma: end_exports

#include <string.h>
// TODO(herb): remove after chromuim skia/ext/SkMemory_new_handler.cpp
// has been updated to point to private/SkMalloc.h
#include "../private/SkMalloc.h"

// enable to test new device-base clipping
//#define SK_USE_DEVICE_CLIPPING

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

///////////////////////////////////////////////////////////////////////////////

#ifdef override_GLOBAL_NEW
#include <new>

inline void* operator new(size_t size) {
    return sk_malloc_throw(size);
}

inline void operator delete(void* p) {
    sk_free(p);
}
#endif

///////////////////////////////////////////////////////////////////////////////

#define SK_INIT_TO_AVOID_WARNING    = 0

#ifndef SkDebugf
    SK_API void SkDebugf(const char format[], ...);
#endif

#define SkREQUIRE_SEMICOLON_AFTER(code) do { code } while (false)

#define SkASSERT_RELEASE(cond) \
    SkREQUIRE_SEMICOLON_AFTER(if (!(cond)) { SK_ABORT(#cond); } )

#ifdef SK_DEBUG
    #define SkASSERT(cond) \
        SkREQUIRE_SEMICOLON_AFTER(if (!(cond)) { SK_ABORT("assert(" #cond ")"); })
    #define SkASSERTF(cond, fmt, ...) \
        SkREQUIRE_SEMICOLON_AFTER(if (!(cond)) { \
                                      SkDebugf(fmt"\n", __VA_ARGS__); \
                                      SK_ABORT("assert(" #cond ")"); \
                                  })
    #define SkDEBUGFAIL(message)        SK_ABORT(message)
    #define SkDEBUGFAILF(fmt, ...)      SkASSERTF(false, fmt, ##__VA_ARGS__)
    #define SkDEBUGCODE(...)            __VA_ARGS__
    #define SkDECLAREPARAM(type, var)   , type var
    #define SkPARAM(var)                , var
    #define SkDEBUGF(args       )       SkDebugf args
    #define SkAssertResult(cond)        SkASSERT(cond)
#else
    #define SkASSERT(cond)
    #define SkASSERTF(cond, fmt, ...)
    #define SkDEBUGFAIL(message)
    #define SkDEBUGFAILF(fmt, ...)
    #define SkDEBUGCODE(...)
    #define SkDEBUGF(args)
    #define SkDECLAREPARAM(type, var)
    #define SkPARAM(var)

    // unlike SkASSERT, this guy executes its condition in the non-debug build.
    // The if is present so that this can be used with functions marked SK_WARN_UNUSED_RESULT.
    #define SkAssertResult(cond)         if (cond) {} do {} while(false)
#endif

// Legacy macro names for SK_ABORT
#define SkFAIL(message)                 SK_ABORT(message)
#define sk_throw()                      SK_ABORT("sk_throw")

#ifdef SK_IGNORE_TO_STRING
    #define SK_TO_STRING_NONVIRT()
    #define SK_TO_STRING_VIRT()
    #define SK_TO_STRING_PUREVIRT()
    #define SK_TO_STRING_OVERRIDE()
#else
    class SkString;
    // the 'toString' helper functions convert Sk* objects to human-readable
    // form in developer mode
    #define SK_TO_STRING_NONVIRT() void toString(SkString* str) const;
    #define SK_TO_STRING_VIRT() virtual void toString(SkString* str) const;
    #define SK_TO_STRING_PUREVIRT() virtual void toString(SkString* str) const = 0;
    #define SK_TO_STRING_OVERRIDE() void toString(SkString* str) const override;
#endif

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

#include "../private/SkTFitsIn.h"
template <typename D, typename S> D SkTo(S s) {
    SkASSERT(SkTFitsIn<D>(s));
    return static_cast<D>(s);
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

#define SK_MaxS16   32767
#define SK_MinS16   -32767
#define SK_MaxU16   0xFFFF
#define SK_MinU16   0
#define SK_MaxS32   0x7FFFFFFF
#define SK_MinS32   -SK_MaxS32
#define SK_MaxU32   0xFFFFFFFF
#define SK_MinU32   0
#define SK_NaN32    ((int) (1U << 31))

/** Returns true if the value can be represented with signed 16bits
 */
static inline bool SkIsS16(long x) {
    return (int16_t)x == x;
}

/** Returns true if the value can be represented with unsigned 16bits
 */
static inline bool SkIsU16(long x) {
    return (uint16_t)x == x;
}

static inline int32_t SkLeftShift(int32_t value, int32_t shift) {
    return (int32_t) ((uint32_t) value << shift);
}

static inline int64_t SkLeftShift(int64_t value, int32_t shift) {
    return (int64_t) ((uint64_t) value << shift);
}

//////////////////////////////////////////////////////////////////////////////

/** Returns the number of entries in an array (not a pointer) */
template <typename T, size_t N> char (&SkArrayCountHelper(T (&array)[N]))[N];
#define SK_ARRAY_COUNT(array) (sizeof(SkArrayCountHelper(array)))

// Can be used to bracket data types that must be dense, e.g. hash keys.
#if defined(__clang__)  // This should work on GCC too, but GCC diagnostic pop didn't seem to work!
    #define SK_BEGIN_REQUIRE_DENSE _Pragma("GCC diagnostic push") \
                                   _Pragma("GCC diagnostic error \"-Wpadded\"")
    #define SK_END_REQUIRE_DENSE   _Pragma("GCC diagnostic pop")
#else
    #define SK_BEGIN_REQUIRE_DENSE
    #define SK_END_REQUIRE_DENSE
#endif

#define SkAlign2(x)     (((x) + 1) >> 1 << 1)
#define SkIsAlign2(x)   (0 == ((x) & 1))

#define SkAlign4(x)     (((x) + 3) >> 2 << 2)
#define SkIsAlign4(x)   (0 == ((x) & 3))

#define SkAlign8(x)     (((x) + 7) >> 3 << 3)
#define SkIsAlign8(x)   (0 == ((x) & 7))

#define SkAlign16(x)     (((x) + 15) >> 4 << 4)
#define SkIsAlign16(x)   (0 == ((x) & 15))

#define SkAlignPtr(x)   (sizeof(void*) == 8 ?   SkAlign8(x) :   SkAlign4(x))
#define SkIsAlignPtr(x) (sizeof(void*) == 8 ? SkIsAlign8(x) : SkIsAlign4(x))

typedef uint32_t SkFourByteTag;
#define SkSetFourByteTag(a, b, c, d)    (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

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

/****************************************************************************
    The rest of these only build with C++
*/
#ifdef __cplusplus

/** Faster than SkToBool for integral conditions. Returns 0 or 1
*/
static inline constexpr int Sk32ToBool(uint32_t n) {
    return (n | (0-n)) >> 31;
}

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

template <typename T> static inline T SkTAbs(T value) {
    if (value < 0) {
        value = -value;
    }
    return value;
}

static inline int32_t SkMax32(int32_t a, int32_t b) {
    if (a < b)
        a = b;
    return a;
}

static inline int32_t SkMin32(int32_t a, int32_t b) {
    if (a > b)
        a = b;
    return a;
}

template <typename T> constexpr const T& SkTMin(const T& a, const T& b) {
    return (a < b) ? a : b;
}

template <typename T> constexpr const T& SkTMax(const T& a, const T& b) {
    return (b < a) ? a : b;
}

static inline int32_t SkSign32(int32_t a) {
    return (a >> 31) | ((unsigned) -a >> 31);
}

static inline int32_t SkFastMin32(int32_t value, int32_t max) {
    if (value > max) {
        value = max;
    }
    return value;
}

/** Returns value pinned between min and max, inclusively. */
template <typename T> static constexpr const T& SkTPin(const T& value, const T& min, const T& max) {
    return SkTMax(SkTMin(value, max), min);
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

/** Use to combine multiple bits in a bitmask in a type safe way.
 */
template <typename T>
T SkTBitOr(T a, T b) {
    return (T)(a | b);
}

/**
 *  Use to cast a pointer to a different type, and maintaining strict-aliasing
 */
template <typename Dst> Dst SkTCast(const void* ptr) {
    union {
        const void* src;
        Dst dst;
    } data;
    data.src = ptr;
    return data.dst;
}

//////////////////////////////////////////////////////////////////////////////

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

#endif /* C++ */

#endif
