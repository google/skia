/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCpu_DEFINED
#define SkCpu_DEFINED

#include "SkTypes.h"

struct SkCpu {
    enum {
        SSE1  = 1 << 0,
        SSE2  = 1 << 1,
        SSE3  = 1 << 2,
        SSSE3 = 1 << 3,
        SSE41 = 1 << 4,
        SSE42 = 1 << 5,
        AVX   = 1 << 6,
        F16C  = 1 << 7,
        FMA   = 1 << 8,
        AVX2  = 1 << 9,
    };
    enum {
        NEON     = 1 << 0,
        NEON_FMA = 1 << 1,
        VFP_FP16 = 1 << 2,
    };

    static void CacheRuntimeFeatures();
    static bool Supports(uint32_t);
private:
#if defined(_MSC_VER) || !defined(SkCpu_IMPL)
    static const uint32_t gCachedFeatures;
#else
    static       uint32_t gCachedFeatures;
#endif
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
    // FMA doesn't fit neatly into this total ordering.
    // It's available on Haswell+ just like AVX2, but it's technically a different bit.
    // TODO: circle back on this if we find ourselves limited by lack of compile-time FMA

#else
    #if defined(SK_ARM_HAS_NEON)
    features |= NEON;
    #endif

    #if defined(SK_CPU_ARM64)
    features |= NEON|NEON_FMA|VFP_FP16;
    #endif

#endif
    return (features & mask) == mask;
}

#endif//SkCpu_DEFINED
