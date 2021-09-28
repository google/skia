/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef QMath_DEFINED
#define QMath_DEFINED

template <int N, typename T> using V = T __attribute__((ext_vector_type(N)));

#if !defined(__clang__)
static_assert(false, "This only works on clang.");
#endif

#if defined(__SSSE3__)
#include <immintrin.h>
#endif

#if defined(__ARM_NEON)
// From section 5.5.5 of the ARM C Language Extensions (ACLE)
    #include <arm_neon.h>
#endif

#include <cassert>
#include <cstdint>

using Q15 = V<8, uint16_t>;
using I16 = V<8, int16_t>;
using U16 = V<8, uint16_t>;


static inline U16 constrained_add(I16 a, U16 b) {
for (size_t i = 0; i < 8; i++) {
    // Ensure that a + b is on the interval [0, UINT16_MAX]
    assert(-b[i] <= a[i] && a[i] <= UINT16_MAX - b[i]);
}
    U16 answer = b + a;
    return answer;
}

// A pure C version of the ssse3 intrinsic mm_mulhrs_epi16;
static inline I16 simulate_ssse3_mm_mulhrs_epi16(I16 a, I16 b) {
    I16 result;
    auto m = [](int16_t r, int16_t s) {
        const int32_t rounding = 1 << 14;
        int32_t temp = (int32_t)r * (int32_t)s + rounding;
        return (int16_t)(temp >> 15);
    };
    for (int i = 0; i < 8; i++) {
        result[i] = m(a[i], b[i]);
    }
    return result;
}

// A pure C version of the neon intrinsic vqrdmulhq_s16;
static inline Q15 simulate_neon_vqrdmulhq_s16(Q15 a, Q15 b) {
    Q15 result;
    const int esize = 16;
    auto m = [](int16_t r, int16_t s) {
        const int64_t rounding = 1 << (esize - 1);
        int64_t product = 2LL * (int64_t)r * (int64_t)s + rounding;
        int64_t result = product >> esize;

        // Saturate the result
        if (int64_t limit =  (1LL << (esize - 1)) - 1; result > limit) { result = limit; }
        if (int64_t limit = -(1LL << (esize - 1))    ; result < limit) { result = limit; }
        return result;
    };
    for (int i = 0; i < 8; i++) {
        result[i] = m(a[i], b[i]);
    }
    return result;
}

#endif  // QMath_DEFINED
