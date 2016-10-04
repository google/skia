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

static inline Sk4f sk_clamp_0_255(const Sk4f& x) {
    // The order of the arguments is important here.  We want to make sure that NaN
    // clamps to zero.  Note that max(NaN, 0) = 0, while max(0, NaN) = NaN.
    return Sk4f::Min(Sk4f::Max(x, 0.0f), 255.0f);
}

// This should probably only be called from sk_linear_to_srgb() or sk_linear_to_srgb_noclamp().
// It generally doesn't make sense to work with sRGB floats.
static inline Sk4f sk_linear_to_srgb_needs_trunc(const Sk4f& x) {
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

    auto hi = (-0.0974983f * 255.0f)
            + (+0.687999f  * 255.0f) * sqrt
            + (+0.412999f  * 255.0f) * ftrt;
    return (x < 0.0048f).thenElse(lo, hi);
}

static inline Sk4i sk_linear_to_srgb(const Sk4f& x) {
    Sk4f f = sk_linear_to_srgb_needs_trunc(x);
    return SkNx_cast<int>(sk_clamp_0_255(f));
}

static inline Sk4i sk_linear_to_srgb_noclamp(const Sk4f& x) {
    Sk4f f = sk_linear_to_srgb_needs_trunc(x);
    for (int i = 0; i < 4; i++) {
        SkASSERTF(0.0f <= f[i] && f[i] < 256.0f, "f[%d] was %g, outside [0,256)\n", i, f[i]);
    }
    return SkNx_cast<int>(f);
}

// sRGB -> linear, using math instead of table lookups, scaling better to larger SIMD vectors.
static inline Sk4f sk_linear_from_srgb_math(const Sk4i& s) {
    auto x = SkNx_cast<float>(s);

    const float u = 1/255.0f;  // x is [0,255], so x^n needs scaling by u^n.

    // Non-linear segment of sRGB curve approximated by
    // l = 0.0025 + 0.6975x^2 + 0.3x^3
    const float k0 = 0.0025f,
                k2 = 0.6975f * u*u,
                k3 = 0.3000f * u*u*u;
    auto hi = k0 + (k2 + k3*x) * (x*x);

    // Linear segment of sRGB curve: the normal slope, extended a little further than normal.
    auto lo = x * (u/12.92f);

    return (x < 14.025f).thenElse(lo, hi);
}

#endif//SkSRGB_DEFINED
