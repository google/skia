/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkNumbers_DEFINED
#define SkNumbers_DEFINED

#include "SkTypes.h"

#include <cstdint>
#include <utility>

#define SK_MaxS16   32767
#define SK_MinS16   -32767
#define SK_MaxU16   0xFFFF
#define SK_MinU16   0
#define SK_MaxS32   0x7FFFFFFF
#define SK_MinS32   -SK_MaxS32
#define SK_MaxU32   0xFFFFFFFF
#define SK_MinU32   0
#define SK_NaN32    ((int) (1U << 31))
#define SK_MaxSizeT SIZE_MAX
static constexpr int64_t SK_MaxS64 = 0x7FFFFFFFFFFFFFFF;
static constexpr int64_t SK_MinS64 = -SK_MaxS64;

inline int32_t SkAbs32(int32_t value) {
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

inline int32_t SkMax32(int32_t a, int32_t b) {
    if (a < b)
        a = b;
    return a;
}

inline int32_t SkMin32(int32_t a, int32_t b) {
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

inline int32_t SkFastMin32(int32_t value, int32_t max) {
    if (value > max) {
        value = max;
    }
    return value;
}

/** Returns value pinned between min and max, inclusively. */
template <typename T> static constexpr const T& SkTPin(const T& value, const T& min, const T& max) {
    return SkTMax(SkTMin(value, max), min);
}
#endif  // SkNumbers_DEFINED
