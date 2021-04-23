/*
 * Copyright 2021 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFloatRsqrt_DEFINED
#define SkFloatRsqrt_DEFINED

#include "include/private/SkFloatingPoint.h"


#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
    #include <xmmintrin.h>
#elif defined(SK_ARM_HAS_NEON)
    #include <arm_neon.h>
#endif

#if defined(SK_LEGACY_FLOAT_RSQRT)
static inline float sk_float_rsqrt_portable(float x) {
    // Get initial estimate.
    int i;
    memcpy(&i, &x, 4);
    i = 0x5F1FFFF9 - (i>>1);
    float estimate;
    memcpy(&estimate, &i, 4);

    // One step of Newton's method to refine.
    const float estimate_sq = estimate*estimate;
    estimate *= 0.703952253f*(2.38924456f-x*estimate_sq);
    return estimate;
}

// Fast, approximate inverse square root.
// Compare to name-brand "1.0f / sk_float_sqrt(x)".  Should be around 10x faster on SSE, 2x on NEON.
static inline float sk_float_rsqrt(float x) {
// We want all this inlined, so we'll inline SIMD and just take the hit when we don't know we've got
// it at compile time.  This is going to be too fast to productively hide behind a function pointer.
//
// We do one step of Newton's method to refine the estimates in the NEON and portable paths.  No
// refinement is faster, but very innacurate.  Two steps is more accurate, but slower than 1/sqrt.
//
// Optimized constants in the portable path courtesy of http://rrrola.wz.cz/inv_sqrt.html
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
#elif defined(SK_ARM_HAS_NEON)
    // Get initial estimate.
    const float32x2_t xx = vdup_n_f32(x);  // Clever readers will note we're doing everything 2x.
    float32x2_t estimate = vrsqrte_f32(xx);

    // One step of Newton's method to refine.
    const float32x2_t estimate_sq = vmul_f32(estimate, estimate);
    estimate = vmul_f32(estimate, vrsqrts_f32(xx, estimate_sq));
    return vget_lane_f32(estimate, 0);  // 1 will work fine too; the answer's in both places.
#else
    return sk_float_rsqrt_portable(x);
#endif
}
#else

static inline float sk_float_rsqrt_portable(float x) { return 1.0f / sk_float_sqrt(x); }
static inline float sk_float_rsqrt         (float x) { return 1.0f / sk_float_sqrt(x); }

#endif

#endif
