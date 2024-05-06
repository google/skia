/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFloatingPoint_DEFINED
#define SkFloatingPoint_DEFINED

#include "include/private/base/SkAttributes.h"
#include "include/private/base/SkMath.h"

#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

inline constexpr float SK_FloatSqrt2 = 1.41421356f;
inline constexpr float SK_FloatPI    = 3.14159265f;
inline constexpr double SK_DoublePI  = 3.14159265358979323846264338327950288;

static constexpr int sk_float_sgn(float x) {
    return (0.0f < x) - (x < 0.0f);
}

static constexpr float sk_float_degrees_to_radians(float degrees) {
    return degrees * (SK_FloatPI / 180);
}

static constexpr float sk_float_radians_to_degrees(float radians) {
    return radians * (180 / SK_FloatPI);
}

// floor(double+0.5) vs. floorf(float+0.5f) give comparable performance, but upcasting to double
// means tricky values like 0.49999997 and 2^24 get rounded correctly. If these were rounded
// as floatf(x + .5f), they would be 1 higher than expected.
#define sk_float_round(x) (float)sk_double_round((double)(x))

template <typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
static inline constexpr bool SkIsNaN(T x) {
    return x != x;
}

// Subtracting a value from itself will result in zero, except for NAN or ±Inf, which make NAN.
// Multiplying a group of values against zero will result in zero for each product, except for
// NAN or ±Inf, which will result in NAN and continue resulting in NAN for the rest of the elements.
// This generates better code than `std::isfinite` when building with clang-cl (April 2024).
template <typename T, typename... Pack, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
static inline bool SkIsFinite(T x, Pack... values) {
    T prod = x - x;
    prod = (prod * ... * values);
    // At this point, `prod` will either be NaN or 0.
    return prod == prod;
}

template <typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
static inline bool SkIsFinite(const T array[], int count) {
    T x = array[0];
    T prod = x - x;
    for (int i = 1; i < count; ++i) {
        prod *= array[i];
    }
    // At this point, `prod` will either be NaN or 0.
    return prod == prod;
}

inline constexpr int SK_MaxS32FitsInFloat = 2147483520;
inline constexpr int SK_MinS32FitsInFloat = -SK_MaxS32FitsInFloat;

// 0x7fffff8000000000
inline constexpr int64_t SK_MaxS64FitsInFloat = SK_MaxS64 >> (63-24) << (63-24);
inline constexpr int64_t SK_MinS64FitsInFloat = -SK_MaxS64FitsInFloat;

// sk_[float|double]_saturate2int are written to return their maximum values when passed NaN.
// MSVC 19.38+ has a bug with this implementation, leading to incorrect results:
// https://developercommunity.visualstudio.com/t/Optimizer-incorrectly-handles-NaN-floati/10654403
//
// We inject an explicit NaN test on MSVC to work around the problem.
#if defined(_MSC_VER) && !defined(__clang__)
    #define SK_CHECK_NAN(resultVal) if (SkIsNaN(x)) { return resultVal; }
#else
    #define SK_CHECK_NAN(resultVal)
#endif

/**
 *  Return the closest int for the given float. Returns SK_MaxS32FitsInFloat for NaN.
 */
static constexpr int sk_float_saturate2int(float x) {
    SK_CHECK_NAN(SK_MaxS32FitsInFloat)
    x = x < SK_MaxS32FitsInFloat ? x : SK_MaxS32FitsInFloat;
    x = x > SK_MinS32FitsInFloat ? x : SK_MinS32FitsInFloat;
    return (int)x;
}

/**
 *  Return the closest int for the given double. Returns SK_MaxS32 for NaN.
 */
static constexpr int sk_double_saturate2int(double x) {
    SK_CHECK_NAN(SK_MaxS32)
    x = x < SK_MaxS32 ? x : SK_MaxS32;
    x = x > SK_MinS32 ? x : SK_MinS32;
    return (int)x;
}

/**
 *  Return the closest int64_t for the given float. Returns SK_MaxS64FitsInFloat for NaN.
 */
static constexpr int64_t sk_float_saturate2int64(float x) {
    SK_CHECK_NAN(SK_MaxS64FitsInFloat)
    x = x < SK_MaxS64FitsInFloat ? x : SK_MaxS64FitsInFloat;
    x = x > SK_MinS64FitsInFloat ? x : SK_MinS64FitsInFloat;
    return (int64_t)x;
}

#undef SK_CHECK_NAN

#define sk_float_floor2int(x)   sk_float_saturate2int(std::floor(x))
#define sk_float_round2int(x)   sk_float_saturate2int(sk_float_round(x))
#define sk_float_ceil2int(x)    sk_float_saturate2int(std::ceil(x))

#define sk_float_floor2int_no_saturate(x)   ((int)std::floor(x))
#define sk_float_round2int_no_saturate(x)   ((int)sk_float_round(x))
#define sk_float_ceil2int_no_saturate(x)    ((int)std::ceil(x))

#define sk_double_round(x)          (std::floor((x) + 0.5))
#define sk_double_floor2int(x)      ((int)std::floor(x))
#define sk_double_round2int(x)      ((int)std::round(x))
#define sk_double_ceil2int(x)       ((int)std::ceil(x))

// Cast double to float, ignoring any warning about too-large finite values being cast to float.
// Clang thinks this is undefined, but it's actually implementation defined to return either
// the largest float or infinity (one of the two bracketing representable floats).  Good enough!
SK_NO_SANITIZE("float-cast-overflow")
static constexpr float sk_double_to_float(double x) {
    return static_cast<float>(x);
}

inline constexpr float SK_FloatNaN = std::numeric_limits<float>::quiet_NaN();
inline constexpr float SK_FloatInfinity = std::numeric_limits<float>::infinity();
inline constexpr float SK_FloatNegativeInfinity = -SK_FloatInfinity;

inline constexpr double SK_DoubleNaN = std::numeric_limits<double>::quiet_NaN();

// Calculate the midpoint between a and b. Similar to std::midpoint in c++20.
static constexpr float sk_float_midpoint(float a, float b) {
    // Use double math to avoid underflow and overflow.
    return static_cast<float>(0.5 * (static_cast<double>(a) + b));
}

static inline float sk_float_rsqrt_portable(float x) { return 1.0f / std::sqrt(x); }
static inline float sk_float_rsqrt         (float x) { return 1.0f / std::sqrt(x); }

// IEEE defines how float divide behaves for non-finite values and zero-denoms, but C does not,
// so we have a helper that suppresses the possible undefined-behavior warnings.
#ifdef SK_BUILD_FOR_WIN
#pragma warning(push)
#pragma warning(disable : 4723)
#endif
SK_NO_SANITIZE("float-divide-by-zero")
static constexpr float sk_ieee_float_divide(float numer, float denom) {
    return numer / denom;
}

SK_NO_SANITIZE("float-divide-by-zero")
static constexpr double sk_ieee_double_divide(double numer, double denom) {
    return numer / denom;
}
#ifdef SK_BUILD_FOR_WIN
#pragma warning( pop )
#endif

// Returns true iff the provided number is within a small epsilon of 0.
bool sk_double_nearly_zero(double a);

// Compare two doubles and return true if they are within maxUlpsDiff of each other.
// * nan as a or b - returns false.
// * infinity, infinity or -infinity, -infinity - returns true.
// * infinity and any other number - returns false.
//
// ulp is an initialism for Units in the Last Place.
bool sk_doubles_nearly_equal_ulps(double a, double b, uint8_t maxUlpsDiff = 16);

#endif
