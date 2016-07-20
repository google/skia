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

extern const float sk_linear_from_srgb[256];

static inline Sk4i sk_linear_to_srgb(const Sk4f& x) {
    // Approximation of the sRGB gamma curve (within 1 when scaled to 8-bit pixels).
    //
    // Tuned by brute force to minimize the number of bytes that fail to round trip,
    // here 0 (of 256), and then to minimize the number of points halfway between bytes
    // (in linear space) that fail to hit the right byte, here 131 (of 255), and to
    // minimize the number of monotonicity regressions over the range [0,1], here 0.

    auto rsqrt = x.rsqrt(),
         sqrt  = rsqrt.invert(),
         ftrt  = rsqrt.rsqrt();

    auto lo = (13.0471f * 255.0f) * x;

    auto hi = (-0.0974983f * 255.0f)
            + (+0.687999f  * 255.0f) * sqrt
            + (+0.412999f  * 255.0f) * ftrt;

    return SkNx_cast<int>( (x < 0.0048f).thenElse(lo, hi) );
}

#endif//SkSRGB_DEFINED
