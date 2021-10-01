/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cassert>
#include <cstdio>
#include <cstdint>
#include "experimental/lowp-basic/QMath.h"

// Compile for x86_64 + ssse3 with:
//     c++ -O3 --std=c++17 -mssse3 experimental/lowp-basic/lowp_experiments.cpp -o lowp
//
// Compile for aarch64 with (Mac os):
//    c++ -O3 --std=c++17 -arch arm64 experimental/lowp-basic/lowp_experiments.cpp  -o lowp
//
// View assembly:
//    dumpobj -d lowp | less

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

using Q15 = V<8, uint16_t>;

#if defined(__SSSE3__)
static void test_mm_mulhrs_epi16_simulation() {
    for (int i = -32768; i < 32768; i++) {
        for (int j = -32768; j < 32768; j++) {
            Q15 a(i);
            Q15 b(j);
            Q15 simResult = simulate_ssse3_mm_mulhrs_epi16(a, b);
            Q15 intrinsicResult = _mm_mulhrs_epi16(a, b);
            for (int i = 0; i < 8; i++) {
                if (simResult[i] != intrinsicResult[i]) {
                    printf("simulate_ssse3_mm_mulhrs_epi16 broken\n");
                    printf("i: %d, a: %hx b: %hx, intrinsic: %hx, sim: %hx\n",
                           i, a[i], b[i], intrinsicResult[i], simResult[i]);
                    exit(1);
                }
            }
        }
    }
}

// Use ssse3 to simulate saturating multiply on arm.
static Q15 ssse3_vqrdmulhq_s16(Q15 a, Q15 b) {
    constexpr Q15 limit(0x8000);
    const Q15 product = _mm_mulhrs_epi16(a, b);
    const Q15 eq = _mm_cmpeq_epi16(product, limit);
    return _mm_xor_si128(eq, product);
}

static void test_ssse3_vqrdmulhq_s16() {
    for (int i = -32768; i < 32768; i++) {
        for (int j = -32768; j < 32768; j++) {
            Q15 a(i);
            Q15 b(j);
            Q15 simResult = ssse3_vqrdmulhq_s16(a, b);
            Q15 realVqrdmulhqS16 = simulate_neon_vqrdmulhq_s16(a, b);
            for (int i = 0; i < 8; i++) {
                if (simResult[i] != realVqrdmulhqS16[i]) {
                    printf("simulating vqrdmulhq_s16 with ssse3 broken\n");
                    printf("i: %d, a: %hx b: %hx, intrinsic: %hx, sim: %hx\n",
                           i, a[i], b[i], realVqrdmulhqS16[i], simResult[i]);
                    exit(1);
                }
            }
        }
    }
}

#endif

#if defined(__ARM_NEON)
static void test_neon_vqrdmulhq_s16_simulation() {
    for (int i = -32768; i < 32768; i++) {
        for (int j = -32768; j < 32768; j++) {
            Q15 a(i);
            Q15 b(j);
            Q15 simResult = simulate_neon_vqrdmulhq_s16(a, b);
            Q15 intrinsicResult = vqrdmulhq_s16(a, b);
            for (int i = 0; i < 8; i++) {
                if (simResult[i] != intrinsicResult[i]) {
                    printf("simulate_neon_vqrdmulhq_s16 broken\n");
                    printf("i: %d, a: %hx b: %hx, intrinsic: %hx, sim: %hx\n",
                           i, a[i], b[i], intrinsicResult[i], simResult[i]);
                    exit(1);
                }
            }
        }
    }
}
#endif

int main() {
    #if defined(__SSSE3__)
        //test_mm_mulhrs_epi16_simulation();
        test_ssse3_vqrdmulhq_s16();
    #endif
    #if defined(__ARM_NEON)
        test_neon_vqrdmulhq_s16_simulation();
    #endif
    printf("Done.\n");
    return 0;
}
