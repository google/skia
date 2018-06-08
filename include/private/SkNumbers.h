/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkNumbers_DEFINED
#define SkNumbers_DEFINED

#include <cstddef>
#include <cstdint>

#define SK_MaxS16   0x7FFF
#define SK_MinS16   -SK_MaxS16
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

static inline int32_t SkLeftShift(int32_t value, int32_t shift) {
    return (int32_t) ((uint32_t) value << shift);
}

static inline int64_t SkLeftShift(int64_t value, int32_t shift) {
    return (int64_t) ((uint64_t) value << shift);
}

/** Returns the number of entries in an array (not a pointer) */
template <typename T, size_t N> char (&SkArrayCountHelper(T (&array)[N]))[N];
#define SK_ARRAY_COUNT(array) (sizeof(SkArrayCountHelper(array)))

template <int N, typename T>
inline constexpr T SkAlignNBits(T x) { return (x + ((1 << N) - 1)) >> N << N; }

template <int N, typename T>
inline constexpr bool SkIsAlignNBits(T x) { return 0 == (x & ((1 << N) - 1)); }

template <typename T> inline constexpr T    SkAlign2  (T x) { return SkAlignNBits  <1>(x); }

template <typename T> inline constexpr bool SkIsAlign2(T x) { return SkIsAlignNBits<1>(x); }

template <typename T> inline constexpr T    SkAlign4  (T x) { return SkAlignNBits  <2>(x); }

template <typename T> inline constexpr bool SkIsAlign4(T x) { return SkIsAlignNBits<2>(x); }

template <typename T> inline constexpr T    SkAlign8  (T x) { return SkAlignNBits  <3>(x); }

template <typename T> inline constexpr bool SkIsAlign8(T x) { return SkIsAlignNBits<3>(x); }

template <typename T> inline constexpr T    SkAlignPtr  (T x) {
    return sizeof(void*) == 8 ?   SkAlign8(x) :   SkAlign4(x);
}
template <typename T> inline constexpr bool SkIsAlignPtr(T x) {
    return sizeof(void*) == 8 ? SkIsAlign8(x) : SkIsAlign4(x);
}

/** Faster than SkToBool for integral conditions. Returns 0 or 1 */
static inline constexpr int Sk32ToBool(uint32_t n) {
    return (n | (0-n)) >> 31;
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

/** Use to combine multiple bits in a bitmask in a type safe way.  */
template <typename T>
T SkTBitOr(T a, T b) {
    return (T)(a | b);
}

/** Use to cast a pointer to a different type, and maintaining strict-aliasing */
template <typename Dst> Dst SkTCast(const void* ptr) {
    union {
        const void* src;
        Dst dst;
    } data;
    data.src = ptr;
    return data.dst;
}

#endif  // SkNumbers_DEFINED
