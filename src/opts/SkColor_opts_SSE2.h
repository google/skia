/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColor_opts_SSE2_DEFINED
#define SkColor_opts_SSE2_DEFINED

#include <emmintrin.h>

// Because no _mm_mul_epi32() in SSE2, we emulate it here.
// Multiplies 4 32-bit integers from a by 4 32-bit intergers from b.
// The 4 multiplication results should be represented within 32-bit
// integers, otherwise they would be overflow.
static inline  __m128i Multiply32_SSE2(const __m128i& a, const __m128i& b) {
    // Calculate results of a0 * b0 and a2 * b2.
    __m128i r1 = _mm_mul_epu32(a, b);
    // Calculate results of a1 * b1 and a3 * b3.
    __m128i r2 = _mm_mul_epu32(_mm_srli_si128(a, 4), _mm_srli_si128(b, 4));
    // Shuffle results to [63..0] and interleave the results.
    __m128i r = _mm_unpacklo_epi32(_mm_shuffle_epi32(r1, _MM_SHUFFLE(0,0,2,0)),
                                   _mm_shuffle_epi32(r2, _MM_SHUFFLE(0,0,2,0)));
    return r;
}

static inline __m128i SkAlpha255To256_SSE2(const __m128i& alpha) {
    return _mm_add_epi32(alpha, _mm_set1_epi32(1));
}

// See #define SkAlphaMulAlpha(a, b)  SkMulDiv255Round(a, b) in SkXfermode.cpp.
static inline __m128i SkAlphaMulAlpha_SSE2(const __m128i& a,
                                           const __m128i& b) {
    __m128i prod = _mm_mullo_epi16(a, b);
    prod = _mm_add_epi32(prod, _mm_set1_epi32(128));
    prod = _mm_add_epi32(prod, _mm_srli_epi32(prod, 8));
    prod = _mm_srli_epi32(prod, 8);

    return prod;
}

// Portable version SkAlphaMulQ is in SkColorPriv.h.
static inline __m128i SkAlphaMulQ_SSE2(const __m128i& c, const __m128i& scale) {
    __m128i mask = _mm_set1_epi32(0xFF00FF);
    __m128i s = _mm_or_si128(_mm_slli_epi32(scale, 16), scale);

    // uint32_t rb = ((c & mask) * scale) >> 8
    __m128i rb = _mm_and_si128(mask, c);
    rb = _mm_mullo_epi16(rb, s);
    rb = _mm_srli_epi16(rb, 8);

    // uint32_t ag = ((c >> 8) & mask) * scale
    __m128i ag = _mm_srli_epi16(c, 8);
    ag = _mm_and_si128(ag, mask);
    ag = _mm_mullo_epi16(ag, s);

    // (rb & mask) | (ag & ~mask)
    rb = _mm_and_si128(mask, rb);
    ag = _mm_andnot_si128(mask, ag);
    return _mm_or_si128(rb, ag);
}

static inline __m128i SkGetPackedA32_SSE2(const __m128i& src) {
    __m128i a = _mm_slli_epi32(src, (24 - SK_A32_SHIFT));
    return _mm_srli_epi32(a, 24);
}

static inline __m128i SkGetPackedR32_SSE2(const __m128i& src) {
    __m128i r = _mm_slli_epi32(src, (24 - SK_R32_SHIFT));
    return _mm_srli_epi32(r, 24);
}

static inline __m128i SkGetPackedG32_SSE2(const __m128i& src) {
    __m128i g = _mm_slli_epi32(src, (24 - SK_G32_SHIFT));
    return _mm_srli_epi32(g, 24);
}

static inline __m128i SkGetPackedB32_SSE2(const __m128i& src) {
    __m128i b = _mm_slli_epi32(src, (24 - SK_B32_SHIFT));
    return _mm_srli_epi32(b, 24);
}

static inline __m128i SkMul16ShiftRound_SSE2(const __m128i& a,
                                             const __m128i& b, int shift) {
    __m128i prod = _mm_mullo_epi16(a, b);
    prod = _mm_add_epi16(prod, _mm_set1_epi16(1 << (shift - 1)));
    prod = _mm_add_epi16(prod, _mm_srli_epi16(prod, shift));
    prod = _mm_srli_epi16(prod, shift);

    return prod;
}

static inline __m128i SkPackRGB16_SSE2(const __m128i& r,
                                       const __m128i& g, const __m128i& b) {
    __m128i dr = _mm_slli_epi16(r, SK_R16_SHIFT);
    __m128i dg = _mm_slli_epi16(g, SK_G16_SHIFT);
    __m128i db = _mm_slli_epi16(b, SK_B16_SHIFT);

    __m128i c = _mm_or_si128(dr, dg);
    return _mm_or_si128(c, db);
}

static inline __m128i SkPackARGB32_SSE2(const __m128i& a, const __m128i& r,
                                        const __m128i& g, const __m128i& b) {
    __m128i da = _mm_slli_epi32(a, SK_A32_SHIFT);
    __m128i dr = _mm_slli_epi32(r, SK_R32_SHIFT);
    __m128i dg = _mm_slli_epi32(g, SK_G32_SHIFT);
    __m128i db = _mm_slli_epi32(b, SK_B32_SHIFT);

    __m128i c = _mm_or_si128(da, dr);
    c = _mm_or_si128(c, dg);
    return _mm_or_si128(c, db);
}

static inline __m128i SkPacked16ToR32_SSE2(const __m128i& src) {
    __m128i r = _mm_srli_epi32(src, SK_R16_SHIFT);
    r = _mm_and_si128(r, _mm_set1_epi32(SK_R16_MASK));
    r = _mm_or_si128(_mm_slli_epi32(r, (8 - SK_R16_BITS)),
                     _mm_srli_epi32(r, (2 * SK_R16_BITS - 8)));

    return r;
}

static inline __m128i SkPacked16ToG32_SSE2(const __m128i& src) {
    __m128i g = _mm_srli_epi32(src, SK_G16_SHIFT);
    g = _mm_and_si128(g, _mm_set1_epi32(SK_G16_MASK));
    g = _mm_or_si128(_mm_slli_epi32(g, (8 - SK_G16_BITS)),
                     _mm_srli_epi32(g, (2 * SK_G16_BITS - 8)));

    return g;
}

static inline __m128i SkPacked16ToB32_SSE2(const __m128i& src) {
    __m128i b = _mm_srli_epi32(src, SK_B16_SHIFT);
    b = _mm_and_si128(b, _mm_set1_epi32(SK_B16_MASK));
    b = _mm_or_si128(_mm_slli_epi32(b, (8 - SK_B16_BITS)),
                     _mm_srli_epi32(b, (2 * SK_B16_BITS - 8)));

    return b;
}

static inline __m128i SkPixel16ToPixel32_SSE2(const __m128i& src) {
    __m128i r = SkPacked16ToR32_SSE2(src);
    __m128i g = SkPacked16ToG32_SSE2(src);
    __m128i b = SkPacked16ToB32_SSE2(src);

    return SkPackARGB32_SSE2(_mm_set1_epi32(0xFF), r, g, b);
}

static inline __m128i SkPixel32ToPixel16_ToU16_SSE2(const __m128i& src_pixel1,
                                                    const __m128i& src_pixel2) {
    // Calculate result r.
    __m128i r1 = _mm_srli_epi32(src_pixel1,
                                SK_R32_SHIFT + (8 - SK_R16_BITS));
    r1 = _mm_and_si128(r1, _mm_set1_epi32(SK_R16_MASK));
    __m128i r2 = _mm_srli_epi32(src_pixel2,
                                SK_R32_SHIFT + (8 - SK_R16_BITS));
    r2 = _mm_and_si128(r2, _mm_set1_epi32(SK_R16_MASK));
    __m128i r = _mm_packs_epi32(r1, r2);

    // Calculate result g.
    __m128i g1 = _mm_srli_epi32(src_pixel1,
                                SK_G32_SHIFT + (8 - SK_G16_BITS));
    g1 = _mm_and_si128(g1, _mm_set1_epi32(SK_G16_MASK));
    __m128i g2 = _mm_srli_epi32(src_pixel2,
                                SK_G32_SHIFT + (8 - SK_G16_BITS));
    g2 = _mm_and_si128(g2, _mm_set1_epi32(SK_G16_MASK));
    __m128i g = _mm_packs_epi32(g1, g2);

    // Calculate result b.
    __m128i b1 = _mm_srli_epi32(src_pixel1,
                                SK_B32_SHIFT + (8 - SK_B16_BITS));
    b1 = _mm_and_si128(b1, _mm_set1_epi32(SK_B16_MASK));
    __m128i b2 = _mm_srli_epi32(src_pixel2,
                                SK_B32_SHIFT + (8 - SK_B16_BITS));
    b2 = _mm_and_si128(b2, _mm_set1_epi32(SK_B16_MASK));
    __m128i b = _mm_packs_epi32(b1, b2);

    // Store 8 16-bit colors in dst.
    __m128i d_pixel = SkPackRGB16_SSE2(r, g, b);

    return d_pixel;
}

#endif // SkColor_opts_SSE2_DEFINED
