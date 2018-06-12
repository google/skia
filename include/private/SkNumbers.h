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

#define SK_MaxS16   INT16_MAX
#define SK_MinS16   -SK_MaxS16

#define SK_MaxS32   INT32_MAX
#define SK_MinS32   -SK_MaxS32

#define SK_NaN32    INT32_MIN

static constexpr int64_t SK_MaxS64 = INT64_MAX;
static constexpr int64_t SK_MinS64 = -SK_MaxS64;

////////////////////////////////////////////////////////////////////////////////

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

template <typename T> static constexpr const T& SkTMin(const T& a, const T& b) {
    return (a < b) ? a : b;
}

template <typename T> static constexpr const T& SkTMax(const T& a, const T& b) {
    return (b < a) ? a : b;
}

static inline int32_t SkFastMin32(int32_t value, int32_t max) { return SkMin32(value, max); }


/** Returns value pinned between min and max, inclusively. */
template <typename T> static constexpr const T& SkTPin(const T& value, const T& min, const T& max) {
    return SkTMax(SkTMin(value, max), min);
}

#endif  // SkNumbers_DEFINED
