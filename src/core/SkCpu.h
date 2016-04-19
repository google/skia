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

    static bool Supports(uint32_t);

private:
    // Consider a loop like this that expands 16-bit floats out to 32-bit, does math, and repacks:
    //    for (int i = 0; i < N; i++) {
    //        if (SkCpu::Supports(SkCpu::F16C)) {
    //            f32s = SkCpu::F16C_cvtph_ps(f16s);
    //        } else {
    //            f32s = some_slower_f16_to_f32_routine(f16s);
    //        }
    //
    //        ... do some math with f32s ...
    //
    //        if (SkCpu::Supports(SkCpu::F16C)) {
    //            f16s = SkCpu::F16C_cvtps_ph(f32s);
    //        } else {
    //            f16s = some_slower_f32_to_f16_routine(f32s);
    //        }
    //    }
    //
    // We would like SkCpu::Supports() to participate in common sub-expression elimination,
    // so that it's called exactly 1 time, rather than N or 2N times.  This is especially
    // important when the if-else blocks you see above are really inline functions.
    //
    // The key to this is to make sure to implement RuntimeCpuFeatures() with the same
    // capacity for common sub-expression elimination.
    //
    // __attribute__((const)) works perfectly when available.
    //
    // When it's not (MSVC), we fall back to a static initializer.
    // (Static intializers would work fine everywhere, but Chrome really dislikes them.)

#if defined(__GNUC__) || defined(__clang__)  // i.e. GCC, Clang, or clang-cl
    __attribute__((const))
    static uint32_t RuntimeCpuFeatures();
#else
    static const uint32_t gCachedCpuFeatures;
    static uint32_t RuntimeCpuFeatures() {
        return gCachedCpuFeatures;
    }
#endif
};

inline bool SkCpu::Supports(uint32_t mask) {
    uint32_t features = RuntimeCpuFeatures();

    // If we mask in compile-time known lower limits, the compiler can completely
    // drop many calls to RuntimeCpuFeatures().
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
