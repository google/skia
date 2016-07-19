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
 *      - for linear -> sRGB, call sk_linear_to_srgb() for R,G,B, and round;
 *      - the alpha channel is linear in both formats, needing at most *(1/255.0f) or *255.0f.
 *
 *  sk_linear_to_srgb()'s output requires rounding; it does not round for you.
 *
 *  Given inputs in [0,1], sk_linear_to_srgb() will not underflow 0 but may overflow 255.
 *  The overflow is small enough to be handled by rounding.
 *  (But if you don't trust the inputs are in [0,1], you'd better clamp both sides immediately.)
 *
 *  sk_linear_to_srgb() will run a little faster than usual when compiled with SSE4.1+.
 */

extern const float sk_linear_from_srgb[256];

static inline Sk4f sk_linear_to_srgb(const Sk4f& x) {
    // Approximation of the sRGB gamma curve (within 1 when scaled to 8-bit pixels).
    // For 0.00000f <= x <  0.00349f,    12.92 * x
    // For 0.00349f <= x <= 1.00000f,    0.679*(x.^0.5) + 0.423*x.^(0.25) - 0.101
    // Note that 0.00349 was selected because it is a point where both functions produce the
    // same pixel value when rounded.
    auto rsqrt = x.rsqrt(),
         sqrt  = rsqrt.invert(),
         ftrt  = rsqrt.rsqrt();

    auto lo = (12.92f * 255.0f) * x;

    auto hi = (-0.101115084998961f * 255.0f) +
              (+0.678513029959381f * 255.0f) * sqrt +
              (+0.422602055039580f * 255.0f) * ftrt;

    return (x < 0.00349f).thenElse(lo, hi);
}

#endif//SkSRGB_DEFINED
