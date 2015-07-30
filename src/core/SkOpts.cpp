/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOnce.h"
#include "SkOpts.h"

#if defined(SK_CPU_X86)
    #if defined(SK_BUILD_FOR_WIN32)
        #include <intrin.h>
        static void cpuid(uint32_t abcd[4]) { __cpuid((int*)abcd, 1); }
    #else
        #include <cpuid.h>
        static void cpuid(uint32_t abcd[4]) { __get_cpuid(1, abcd+0, abcd+1, abcd+2, abcd+3); }
    #endif
#elif !defined(SK_ARM_HAS_NEON) && defined(SK_CPU_ARM32) && defined(SK_BUILD_FOR_ANDROID)
    #include <cpu-features.h>
#endif

static float rsqrt_portable(float x) {
    // Get initial estimate.
    int i = *SkTCast<int*>(&x);
    i = 0x5F1FFFF9 - (i>>1);
    float estimate = *SkTCast<float*>(&i);

    // One step of Newton's method to refine.
    const float estimate_sq = estimate*estimate;
    estimate *= 0.703952253f*(2.38924456f-x*estimate_sq);
    return estimate;
}

namespace SkOpts {
    // Define default function pointer values here...
    decltype(rsqrt) rsqrt = rsqrt_portable;

    // Each Init_foo() is defined in src/opts/SkOpts_foo.cpp.
    void Init_sse2();
    void Init_ssse3();
    void Init_sse41();
    void Init_neon();
    //TODO: _dsp2, _armv7, _armv8, _x86, _x86_64, _sse42, _avx, avx2, ... ?

    static void init() {
    #if defined(SK_CPU_X86)
        uint32_t abcd[] = {0,0,0,0};
        cpuid(abcd);
        if (abcd[3] & (1<<26)) { Init_sse2(); }
        if (abcd[2] & (1<< 9)) { Init_ssse3(); }
        if (abcd[2] & (1<<19)) { Init_sse41(); }
    #elif defined(SK_ARM_HAS_NEON)
        Init_neon();
    #elif defined(SK_CPU_ARM32) && defined(SK_BUILD_FOR_ANDROID)
        if (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) { Init_neon(); }
    #endif
    }

    SK_DECLARE_STATIC_ONCE(gInitOnce);
    void Init() { SkOnce(&gInitOnce, init); }

#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
    static struct AutoInit {
        AutoInit() { Init(); }
    } gAutoInit;
#endif
}
