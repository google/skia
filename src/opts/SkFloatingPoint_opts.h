/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFloatingPoint_opts_DEFINED
#define SkFloatingPoint_opts_DEFINED

#include "SkFloatingPoint.h"

namespace SK_OPTS_NS {

#if defined(SK_ARM_HAS_NEON)
    static float rsqrt(float x) {
        return sk_float_rsqrt(x);  // This sk_float_rsqrt copy will take the NEON compile-time path.
    }
#else
    static float rsqrt(float x) {
        // Get initial estimate.
        int i = *SkTCast<int*>(&x);
        i = 0x5F1FFFF9 - (i>>1);
        float estimate = *SkTCast<float*>(&i);

        // One step of Newton's method to refine.
        const float estimate_sq = estimate*estimate;
        estimate *= 0.703952253f*(2.38924456f-x*estimate_sq);
        return estimate;
    }
#endif

}  // namespace SK_OPTS_NS

#endif//SkFloatingPoint_opts_DEFINED
