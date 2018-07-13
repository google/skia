/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSRGB_DEFINED
#define SkSRGB_DEFINED

#include "SkNx.h"

/** Components for building our canonical sRGB -> linear and linear -> sRGB transformations.
 *
 *  Current best practices:
 *      - for sRGB -> linear, lookup R,G,B in sk_linear_from_srgb;
 *      - for linear -> sRGB, call sk_linear_to_srgb() for R,G,B;
 *      - the alpha channel is linear in both formats, needing at most *(1/255.0f) or *255.0f.
 *
 *  sk_linear_to_srgb() will run a little faster than usual when compiled with SSE4.1+.
 */

extern const float    sk_linear_from_srgb[256];
extern const uint16_t sk_linear12_from_srgb[256];
extern const uint8_t  sk_linear12_to_srgb[4096];

// [0.0f, 1.0f] -> [0, 255].
static inline Sk4i sk_linear_to_srgb(const Sk4f& x) {
    // Approximation of the sRGB gamma curve (within 1 when scaled to 8-bit pixels).
    //
    // Constants tuned by brute force to minimize (in order of importance) after truncation:
    //    1) the number of bytes that fail to round trip (0 of 256);
    //    2) the number of points in [FLT_MIN, 1.0f] that are non-monotonic (0 of ~1 billion);
    //    3) the number of points halfway between bytes that hit the wrong byte (131 of 255).
    auto rsqrt = x.rsqrt(),
         sqrt  = rsqrt.invert(),
         ftrt  = rsqrt.rsqrt();

    auto lo = (13.0471f * 255.0f) * x;

    auto hi = SkNx_fma(Sk4f{+0.412999f  * 255.0f}, ftrt,
              SkNx_fma(Sk4f{+0.687999f  * 255.0f}, sqrt,
                       Sk4f{-0.0974983f * 255.0f}));
    auto s = (x < 0.0048f).thenElse(lo, hi);

    // Now clamp and truncate.
    // The order of the arguments is important here.  We want to make sure that NaN
    // clamps to zero.  Note that max(NaN, 0) = 0, while max(0, NaN) = NaN.
    return SkNx_cast<int>(Sk4f::Min(Sk4f::Max(s, 0.0f), 255.0f));
}

#endif//SkSRGB_DEFINED
