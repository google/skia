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

template <typename V>
static inline V sk_clamp_0_255(const V& x) {
    // The order of the arguments is important here.  We want to make sure that NaN
    // clamps to zero.  Note that max(NaN, 0) = 0, while max(0, NaN) = NaN.
    return V::Min(V::Max(x, 0.0f), 255.0f);
}

// [0.0f, 1.0f] -> [0.0f, 255.xf], for small x.  Correct after truncation.
template <typename V>
static inline V sk_linear_to_srgb_needs_trunc(const V& x) {
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

    auto hi = SkNx_fma(V{+0.412999f  * 255.0f}, ftrt,
              SkNx_fma(V{+0.687999f  * 255.0f}, sqrt,
                       V{-0.0974983f * 255.0f}));
    return (x < 0.0048f).thenElse(lo, hi);
}

// [0.0f, 1.0f] -> [0.0f, 1.0f].  Correct after rounding.
template <typename V>
static inline V sk_linear_to_srgb_needs_round(const V& x) {
    // Tuned to round trip each sRGB byte after rounding.
    auto rsqrt = x.rsqrt(),
         sqrt  = rsqrt.invert(),
         ftrt  = rsqrt.rsqrt();

    auto lo = 12.46f * x;

    auto hi = V::Min(1.0f, SkNx_fma(V{+0.411192f}, ftrt,
                           SkNx_fma(V{+0.689206f}, sqrt,
                                    V{-0.0988f})));
    return (x < 0.0043f).thenElse(lo, hi);
}

template <int N>
static inline SkNx<N,int> sk_linear_to_srgb(const SkNx<N,float>& x) {
    auto f = sk_linear_to_srgb_needs_trunc(x);
    return SkNx_cast<int>(sk_clamp_0_255(f));
}


// sRGB -> linear, using math instead of table lookups.
template <typename V>
static inline V sk_linear_from_srgb_math(const V& x) {
    // Non-linear segment of sRGB curve approximated by
    // l = 0.0025 + 0.6975x^2 + 0.3x^3
    const V k0 = 0.0025f,
            k2 = 0.6975f,
            k3 = 0.3000f;
    auto hi = SkNx_fma(x*x, SkNx_fma(x, k3, k2), k0);

    // Linear segment of sRGB curve: the normal slope, extended a little further than normal.
    auto lo = x * (1/12.92f);

    return (x < 0.055f).thenElse(lo, hi);
}

// Same as above, starting from ints.
template <int N>
static inline SkNx<N,float> sk_linear_from_srgb_math(const SkNx<N,int>& s) {
    auto x = SkNx_cast<float>(s);

    // Same math as above, but working with x in [0,255], so x^n needs scaling by u^n.
    const float u = 1/255.0f;

    const SkNx<N,float> k0 = 0.0025f,
                        k2 = 0.6975f * u*u,
                        k3 = 0.3000f * u*u*u;
    auto hi = SkNx_fma(x*x, SkNx_fma(x, k3, k2), k0);
    auto lo = x * (u/12.92f);
    return (x < (0.055f/u)).thenElse(lo, hi);
}

#endif//SkSRGB_DEFINED
