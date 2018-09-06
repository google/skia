/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMath_DEFINED
#define SkMath_DEFINED

#include "SkTypes.h"

#include <algorithm>

template <typename T> static constexpr T SkAlign2(T x) { return (x + 1) >> 1 << 1; }
template <typename T> static constexpr T SkAlign4(T x) { return (x + 3) >> 2 << 2; }
template <typename T> static constexpr T SkAlign8(T x) { return (x + 7) >> 3 << 3; }

template <typename T> static constexpr bool SkIsAlign2(T x) { return 0 == (x & 1); }
template <typename T> static constexpr bool SkIsAlign4(T x) { return 0 == (x & 3); }
template <typename T> static constexpr bool SkIsAlign8(T x) { return 0 == (x & 7); }

template <typename T> static constexpr T SkAlignPtr(T x) {
    return sizeof(void*) == 8 ? SkAlign8(x) : SkAlign4(x);
}
template <typename T> static constexpr bool SkIsAlignPtr(T x) {
    return sizeof(void*) == 8 ? SkIsAlign8(x) : SkIsAlign4(x);
}

static inline int32_t SkAbs32(int32_t value) {
    SkASSERT(value != SK_NaN32);  // The most negative int32_t can't be negated.
    if (value < 0) {
        value = -value;
    }
    return value;
}

static inline int32_t SkMax32(int32_t a, int32_t b) {
    if (a < b) {
        a = b;
    }
    return a;
}

static inline int32_t SkMin32(int32_t a, int32_t b) {
    if (a > b) {
        a = b;
    }
    return a;
}

static inline constexpr int32_t SkLeftShift(int32_t value, int32_t shift) {
    return (int32_t) ((uint32_t) value << shift);
}

static inline constexpr int64_t SkLeftShift(int64_t value, int32_t shift) {
    return (int64_t) ((uint64_t) value << shift);
}

template <typename T> static inline T SkTAbs(T value) {
    if (value < 0) {
        value = -value;
    }
    return value;
}

////////////////////////////////////////////////////////////////////////////////

template <typename T> constexpr const T& SkTMin(const T& a, const T& b) { return std::min(a, b); }

template <typename T> constexpr const T& SkTMax(const T& a, const T& b) { return std::max(a, b); }

/** @return value pinned (clamped) between min and max, inclusively.
*/
template <typename T> static constexpr const T& SkTClamp(const T& x, const T& lo, const T& hi) {
    return std::max(std::min(x, hi), lo);
}
template <typename T> static constexpr const T& SkTPin(const T& x, const T& lo, const T& hi) {
    return SkTClamp(x, lo, hi);
}

///////////////////////////////////////////////////////////////////////////////

// 64bit -> 32bit utilities

// Handy util that can be passed two ints, and will automatically promote to
// 64bits before the multiply, so the caller doesn't have to remember to cast
// e.g. (int64_t)a * b;
static inline int64_t sk_64_mul(int64_t a, int64_t b) {
    return a * b;
}

///////////////////////////////////////////////////////////////////////////////

/** Given an integer and a positive (max) integer, return the value
 *  pinned against 0 and max, inclusive.
 *  @param value    The value we want returned pinned between [0...max]
 *  @param max      The positive max value
 *  @return 0 if value < 0, max if value > max, else value
 */
static inline int SkClampMax(int value, int max) {
    // ensure that max is positive
    SkASSERT(max >= 0);
    if (value < 0) {
        value = 0;
    }
    if (value > max) {
        value = max;
    }
    return value;
}

/**
 *  Returns true if value is a power of 2. Does not explicitly check for
 *  value <= 0.
 */
template <typename T> constexpr inline bool SkIsPow2(T value) {
    return (value & (value - 1)) == 0;
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  Return a*b/((1 << shift) - 1), rounding any fractional bits.
 *  Only valid if a and b are unsigned and <= 32767 and shift is > 0 and <= 8
 */
static inline unsigned SkMul16ShiftRound(U16CPU a, U16CPU b, int shift) {
    SkASSERT(a <= 32767);
    SkASSERT(b <= 32767);
    SkASSERT(shift > 0 && shift <= 8);
    unsigned prod = a*b + (1 << (shift - 1));
    return (prod + (prod >> shift)) >> shift;
}

/**
 *  Return a*b/255, rounding any fractional bits.
 *  Only valid if a and b are unsigned and <= 32767.
 */
static inline U8CPU SkMulDiv255Round(U16CPU a, U16CPU b) {
    SkASSERT(a <= 32767);
    SkASSERT(b <= 32767);
    unsigned prod = a*b + 128;
    return (prod + (prod >> 8)) >> 8;
}

#endif
