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

template <typename T>
static constexpr T SkAlign2(T x) { return (x + 1) >> 1 << 1; }

template <typename T>
static constexpr T SkAlign4(T x) { return (x + 3) >> 2 << 2; }

template <typename T>
static constexpr T SkAlign8(T x) { return (x + 7) >> 3 << 3; }

template <typename T>
static constexpr bool SkIsAlign2(T x) { return 0 == (x & 1); }

template <typename T>
static constexpr bool SkIsAlign4(T x) { return 0 == (x & 3); }

template <typename T>
static constexpr bool SkIsAlign8(T x) { return 0 == (x & 7); }

template <typename T>
static constexpr T SkAlignPtr(T x) { return sizeof(void*) == 8 ? SkAlign8(x) : SkAlign4(x); }

template <typename T>
static constexpr bool SkIsAlignPtr(T x) {
    return sizeof(void*) == 8 ? SkIsAlign8(x) : SkIsAlign4(x);
}

static inline constexpr int32_t SkLeftShift(int32_t value, int32_t shift) {
    return (int32_t)((uint32_t)value << shift);
}

static inline constexpr int64_t SkLeftShift(int64_t value, int32_t shift) {
    return (int64_t)((uint64_t)value << shift);
}

static inline int32_t SkAbs32(int32_t value) {
    SkASSERT(value != SK_NaN32);  // The most negative int32_t can't be negated.
    if (value < 0) {
        value = -value;
    }
    return value;
}

template <typename T>
static inline T SkTAbs(T value) {
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

template <typename T>
constexpr const T& SkTMin(const T& a, const T& b) { return (a < b) ? a : b; }

template <typename T>
constexpr const T& SkTMax(const T& a, const T& b) { return (b < a) ? a : b; }

template <typename T>
constexpr const T& SkTClamp(const T& x, const T& lo, const T& hi) {
    return (x < lo) ? lo : SkTMin(x, hi);
}

static inline int32_t SkFastMin32(int32_t value, int32_t max) {
    if (value > max) {
        value = max;
    }
    return value;
}

/** @return value pinned (clamped) between min and max, inclusively.
*/
template <typename T>
static constexpr const T& SkTPin(const T& value, const T& min, const T& max) {
    return SkTMax(SkTMin(value, max), min);
}

#endif  // SkNumbers_DEFINED
