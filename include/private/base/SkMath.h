/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMath_DEFINED
#define SkMath_DEFINED

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkCPUTypes.h"

#include <bit>
#include <cstdint>
#include <climits>

// Max Signed 16 bit value
static constexpr int16_t SK_MaxS16 = INT16_MAX;
static constexpr int16_t SK_MinS16 = -SK_MaxS16;

static constexpr int32_t SK_MaxS32 = INT32_MAX;
static constexpr int32_t SK_MinS32 = -SK_MaxS32;
static constexpr int32_t SK_NaN32  = INT32_MIN;

static constexpr int64_t SK_MaxS64 = INT64_MAX;
static constexpr int64_t SK_MinS64 = -SK_MaxS64;

// 64bit -> 32bit utilities

// Handy util that can be passed two ints, and will automatically promote to
// 64bits before the multiply, so the caller doesn't have to remember to cast
// e.g. (int64_t)a * b;
static inline int64_t sk_64_mul(int64_t a, int64_t b) {
    return a * b;
}

static inline constexpr int32_t SkLeftShift(int32_t value, int32_t shift) {
    return (int32_t) ((uint32_t) value << shift);
}

static inline constexpr int64_t SkLeftShift(int64_t value, int32_t shift) {
    return (int64_t) ((uint64_t) value << shift);
}

///////////////////////////////////////////////////////////////////////////////

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
    return SkMul16ShiftRound(a, b, 8);
}

/**
 *  Returns the log2 of the specified value, were that value to be rounded up
 *  to the next power of 2. It is undefined to pass 0. Examples:
 *  SkNextLog2(1) -> 0
 *  SkNextLog2(2) -> 1
 *  SkNextLog2(3) -> 2
 *  SkNextLog2(4) -> 2
 *  SkNextLog2(5) -> 3
 */
static inline int SkNextLog2(uint32_t value) {
    SkASSERT(value != 0);
    return 32 - std::countl_zero(value - 1);
}

/**
 *  Returns the smallest power-of-2 that is >= the specified value. If value
 *  is already a power of 2, then it is returned unchanged. It is undefined
 *  if value is <= 0.
 */
static inline int SkNextPow2(int value) {
    SkASSERT(value > 0);
    return 1 << SkNextLog2(static_cast<uint32_t>(value));
}

#endif
