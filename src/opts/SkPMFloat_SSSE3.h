/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// For SkPMFloat(SkPMColor), we widen our 8 bit components (fix8) to 8-bit components in 32 bits
// (fix8_32), then convert those to floats.

// round() does the opposite, working from floats to 8-bit-in-32-bits, then back to packed 8 bit.

// roundClamp() is the same as _SSE2: floats to 8-in-32, to 8-in-16, to packed 8 bit, with
// _mm_packus_epi16() both clamping and narrowing.

inline SkPMFloat::SkPMFloat(SkPMColor c) {
    SkPMColorAssert(c);
    const int _ = 255;  // _ means to zero that byte.
    __m128i fix8    = _mm_set_epi32(0,0,0,c),
            fix8_32 = _mm_shuffle_epi8(fix8, _mm_set_epi8(_,_,_,3, _,_,_,2, _,_,_,1, _,_,_,0));
    fVec = _mm_cvtepi32_ps(fix8_32);
    SkASSERT(this->isValid());
}

inline SkPMColor SkPMFloat::trunc() const {
    const int _ = 255;  // _ means to zero that byte.
    __m128i fix8_32 = _mm_cvttps_epi32(fVec),
            fix8    = _mm_shuffle_epi8(fix8_32, _mm_set_epi8(_,_,_,_, _,_,_,_, _,_,_,_, 12,8,4,0));
    SkPMColor c = _mm_cvtsi128_si32(fix8);
    SkPMColorAssert(c);
    return c;
}

inline SkPMColor SkPMFloat::round() const {
    return SkPMFloat(Sk4f(0.5f) + *this).trunc();
}

inline SkPMColor SkPMFloat::roundClamp() const {
    // We don't use _mm_cvtps_epi32, because we want precise control over how 0.5 rounds (up).
    __m128i fix8_32 = _mm_cvttps_epi32(_mm_add_ps(_mm_set1_ps(0.5f), fVec)),
            fix8_16 = _mm_packus_epi16(fix8_32, fix8_32),
            fix8    = _mm_packus_epi16(fix8_16, fix8_16);
    SkPMColor c = _mm_cvtsi128_si32(fix8);
    SkPMColorAssert(c);
    return c;
}

inline void SkPMFloat::From4PMColors(const SkPMColor colors[4],
                                     SkPMFloat* a, SkPMFloat* b, SkPMFloat* c, SkPMFloat* d) {
    // Haven't beaten this yet.
    *a = FromPMColor(colors[0]);
    *b = FromPMColor(colors[1]);
    *c = FromPMColor(colors[2]);
    *d = FromPMColor(colors[3]);
}

inline void SkPMFloat::RoundTo4PMColors(
        const SkPMFloat& a, const SkPMFloat& b, const SkPMFloat&c, const SkPMFloat& d,
        SkPMColor colors[4]) {
    // Haven't beaten this yet.  Still faster than RoundClampTo4PMColors?
    colors[0] = a.round();
    colors[1] = b.round();
    colors[2] = c.round();
    colors[3] = d.round();
}

inline void SkPMFloat::RoundClampTo4PMColors(
        const SkPMFloat& a, const SkPMFloat& b, const SkPMFloat&c, const SkPMFloat& d,
        SkPMColor colors[4]) {
    // Same as _SSE2.h's.  We use 3 _mm_packus_epi16() where the naive loop uses 8.
    // We don't use _mm_cvtps_epi32, because we want precise control over how 0.5 rounds (up).
    __m128i c0 = _mm_cvttps_epi32(_mm_add_ps(_mm_set1_ps(0.5f), a.fVec)),
            c1 = _mm_cvttps_epi32(_mm_add_ps(_mm_set1_ps(0.5f), b.fVec)),
            c2 = _mm_cvttps_epi32(_mm_add_ps(_mm_set1_ps(0.5f), c.fVec)),
            c3 = _mm_cvttps_epi32(_mm_add_ps(_mm_set1_ps(0.5f), d.fVec));
    __m128i c3210 = _mm_packus_epi16(_mm_packus_epi16(c0, c1),
                                     _mm_packus_epi16(c2, c3));
    _mm_storeu_si128((__m128i*)colors, c3210);
    SkPMColorAssert(colors[0]);
    SkPMColorAssert(colors[1]);
    SkPMColorAssert(colors[2]);
    SkPMColorAssert(colors[3]);
}
