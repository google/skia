/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

namespace {  // See SkPMFloat.h

inline SkPMFloat::SkPMFloat(SkPMColor c) {
    SkPMColorAssert(c);
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
    const int _ = 255;  // Zero these bytes.
    __m128i fix8    = _mm_cvtsi32_si128((int)c),
            fix8_32 = _mm_shuffle_epi8(fix8, _mm_setr_epi8(0,_,_,_, 1,_,_,_, 2,_,_,_, 3,_,_,_));
#else
    __m128i fix8    = _mm_cvtsi32_si128((int)c),
            fix8_16 = _mm_unpacklo_epi8 (fix8,    _mm_setzero_si128()),
            fix8_32 = _mm_unpacklo_epi16(fix8_16, _mm_setzero_si128());
#endif
    fVec = _mm_mul_ps(_mm_cvtepi32_ps(fix8_32), _mm_set1_ps(1.0f / 255));
    SkASSERT(this->isValid());
}

inline SkPMColor SkPMFloat::round() const {
    // We don't use _mm_cvtps_epi32, because we want precise control over how 0.5 rounds (up).
    __m128 scaled = _mm_mul_ps(_mm_set1_ps(255), fVec);
    __m128i fix8_32 = _mm_cvttps_epi32(_mm_add_ps(_mm_set1_ps(0.5f), scaled)),
            fix8_16 = _mm_packus_epi16(fix8_32, fix8_32),
            fix8    = _mm_packus_epi16(fix8_16, fix8_16);
    SkPMColor c = _mm_cvtsi128_si32(fix8);
    SkPMColorAssert(c);
    return c;
}

inline Sk4f SkPMFloat::alphas() const {
    static_assert(SK_A32_SHIFT == 24, "");
    return _mm_shuffle_ps(fVec, fVec, 0xff);  // Read as 11 11 11 11, copying lane 3 to all lanes.
}

}  // namespace
