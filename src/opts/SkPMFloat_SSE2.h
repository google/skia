/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// For SkPMFloat(SkPMColor), we widen our 8 bit components (fix8) to 8-bit components in 16 bits
// (fix8_16), then widen those to 8-bit-in-32-bits (fix8_32), and finally convert those to floats.

// round() and roundClamp() do the opposite, working from floats to 8-bit-in-32-bit,
// to 8-bit-in-16-bit, back down to 8-bit components.
// _mm_packus_epi16() gives us clamping for free while narrowing.

inline SkPMFloat::SkPMFloat(SkPMColor c) {
    SkPMColorAssert(c);
    __m128i fix8    = _mm_set_epi32(0,0,0,c),
            fix8_16 = _mm_unpacklo_epi8 (fix8,    _mm_setzero_si128()),
            fix8_32 = _mm_unpacklo_epi16(fix8_16, _mm_setzero_si128());
    fVec = _mm_cvtepi32_ps(fix8_32);
    SkASSERT(this->isValid());
}

inline SkPMColor SkPMFloat::round() const {
    return this->roundClamp();  // Haven't beaten this yet.
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

inline SkPMColor SkPMFloat::trunc() const {
    // Basically, same as roundClamp(), but no rounding.
    __m128i fix8_32 = _mm_cvttps_epi32(fVec),
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
    // Haven't beaten this yet.
    RoundClampTo4PMColors(a,b,c,d, colors);
}

inline void SkPMFloat::RoundClampTo4PMColors(
        const SkPMFloat& a, const SkPMFloat& b, const SkPMFloat&c, const SkPMFloat& d,
        SkPMColor colors[4]) {
    // Same as _SSSE3.h's.  We use 3 _mm_packus_epi16() where the naive loop uses 8.
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
