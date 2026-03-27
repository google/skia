/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCpu_DEFINED
#define SkCpu_DEFINED

#include "include/core/SkTypes.h"

#include <cstdint>

struct SkCpu {
    enum {
        SSE1       = 1 << 0,
        SSE2       = 1 << 1,
        SSE3       = 1 << 2,
        SSSE3      = 1 << 3,
        SSE41      = 1 << 4,
        SSE42      = 1 << 5,
        AVX        = 1 << 6,
        F16C       = 1 << 7,
        FMA        = 1 << 8,
        AVX2       = 1 << 9,
        BMI1       = 1 << 10,
        BMI2       = 1 << 11,
        // Handy alias for all the cool Haswell+ instructions.
        HSW = AVX2 | BMI1 | BMI2 | F16C | FMA,

        AVX512F    = 1 << 12,
        AVX512DQ   = 1 << 13,
        AVX512IFMA = 1 << 14,
        AVX512PF   = 1 << 15,
        AVX512ER   = 1 << 16,
        AVX512CD   = 1 << 17,
        AVX512BW   = 1 << 18,
        AVX512VL   = 1 << 19,

        // Handy alias for all the cool Skylake Xeon+ instructions.
        SKX = AVX512F  | AVX512DQ | AVX512CD | AVX512BW | AVX512VL,

        ERMS       = 1 << 20,
    };

    enum {
        LOONGARCH_SX = 1 << 0,
        LOONGARCH_ASX = 1 << 1,
    };

    static void CacheRuntimeFeatures();
    static bool Supports(uint32_t);
private:
    static uint32_t gCachedFeatures;
};

inline bool SkCpu::Supports(uint32_t mask) {
    uint32_t features = gCachedFeatures;

    // If we mask in compile-time known lower limits, the compiler can
    // often compile away this entire function.
#if SK_CPU_X86
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
    features |= SSE1;
    #endif
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    features |= SSE2;
    #endif
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE3
    features |= SSE3;
    #endif
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
    features |= SSSE3;
    #endif
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
    features |= SSE41;
    #endif
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE42
    features |= SSE42;
    #endif
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX
    features |= AVX;
    #endif
    // F16C goes here if we add SK_CPU_SSE_LEVEL_F16C
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
    features |= AVX2;
    #endif
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SKX
    features |= (AVX512F | AVX512DQ | AVX512CD | AVX512BW | AVX512VL);
    #endif
    // FMA doesn't fit neatly into this total ordering.
    // It's available on Haswell+ just like AVX2, but it's technically a different bit.
    // TODO: circle back on this if we find ourselves limited by lack of compile-time FMA

    #if defined(SK_CPU_LIMIT_AVX)
    features &= (SSE1 | SSE2 | SSE3 | SSSE3 | SSE41 | SSE42 | AVX);
    #elif defined(SK_CPU_LIMIT_SSE41)
    features &= (SSE1 | SSE2 | SSE3 | SSSE3 | SSE41);
    #elif defined(SK_CPU_LIMIT_SSE2)
    features &= (SSE1 | SSE2);
    #endif

#elif SK_CPU_LOONGARCH
    #if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    features |= LOONGARCH_SX;
    #endif
    #if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    features |= LOONGARCH_ASX;
    #endif

#endif
    return (features & mask) == mask;
}

#endif//SkCpu_DEFINED
