/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS sse2
#include "SkBlurImageFilter_opts.h"
#include "SkXfermode_opts.h"

namespace sse2 {  // This helps identify methods from this file when debugging / profiling.

static void memset16(uint16_t* dst, uint16_t val, int n) {
    auto dst8 = (__m128i*)dst;
    auto val8 = _mm_set1_epi16(val);
    for ( ; n >= 8; n -= 8) {
        _mm_storeu_si128(dst8++, val8);
    }
    dst = (uint16_t*)dst8;
    if (n & 4) {
        _mm_storel_epi64((__m128i*)dst, val8);
        dst += 4;
    }
    if (n & 2) {
        *(uint32_t*)dst = _mm_cvtsi128_si32(val8);
        dst += 2;
    }
    if (n & 1) {
        *dst = val;
    }
}

static void memset32(uint32_t* dst, uint32_t val, int n) {
    auto dst4 = (__m128i*)dst;
    auto val4 = _mm_set1_epi32(val);
    for ( ; n >= 4; n -= 4) {
        _mm_storeu_si128(dst4++, val4);
    }
    dst = (uint32_t*)dst4;
    if (n & 2) {
        _mm_storel_epi64((__m128i*)dst, val4);
        dst += 2;
    }
    if (n & 1) {
        *dst = val;
    }
}

}  // namespace sse2

namespace SkOpts {
    void Init_sse2() {
        memset16        = sse2::memset16;
        memset32        = sse2::memset32;
        create_xfermode = SkCreate4pxXfermode;

        static const auto x = sse2::kX, y = sse2::kY;
        box_blur_xx = sse2::box_blur<x,x>;
        box_blur_xy = sse2::box_blur<x,y>;
        box_blur_yx = sse2::box_blur<y,x>;
    }
}
