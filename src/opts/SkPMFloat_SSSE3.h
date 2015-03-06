#include "SkColorPriv.h"
#include <tmmintrin.h>

// For SkPMFloat(SkPMColor), we widen our 8 bit components (fix8) to 8-bit components in 32 bits
// (fix8_32), then convert those to floats.

// get() does the opposite, working from floats to 8-bit-in-32-bits, then back to packed 8 bit.

// clamped() is the same as _SSE2: floats to 8-in-32, to 8-in-16, to packed 8 bit, with
// _mm_packus_epi16() both clamping and narrowing.

inline SkPMFloat::SkPMFloat(SkPMColor c) {
    SkPMColorAssert(c);
    const int _ = 255;  // _ means to zero that byte.
    __m128i fix8    = _mm_set_epi32(0,0,0,c),
            fix8_32 = _mm_shuffle_epi8(fix8, _mm_set_epi8(_,_,_,3, _,_,_,2, _,_,_,1, _,_,_,0));
    _mm_store_ps(fColor, _mm_cvtepi32_ps(fix8_32));
    SkASSERT(this->isValid());
}

inline SkPMColor SkPMFloat::get() const {
    SkASSERT(this->isValid());
    const int _ = 255;  // _ means to zero that byte.
    __m128i fix8_32 = _mm_cvtps_epi32(_mm_load_ps(fColor)),  // _mm_cvtps_epi32 rounds for us!
            fix8    = _mm_shuffle_epi8(fix8_32, _mm_set_epi8(_,_,_,_, _,_,_,_, _,_,_,_, 12,8,4,0));
    SkPMColor c = _mm_cvtsi128_si32(fix8);
    SkPMColorAssert(c);
    return c;
}

inline SkPMColor SkPMFloat::clamped() const {
    __m128i fix8_32 = _mm_cvtps_epi32(_mm_load_ps(fColor)),  // _mm_cvtps_epi32 rounds for us!
            fix8_16 = _mm_packus_epi16(fix8_32, fix8_32),
            fix8    = _mm_packus_epi16(fix8_16, fix8_16);
    SkPMColor c = _mm_cvtsi128_si32(fix8);
    SkPMColorAssert(c);
    return c;
}

inline void SkPMFloat::From4PMColors(SkPMFloat floats[4], const SkPMColor colors[4]) {
    // Haven't beaten this yet.
    for (int i = 0; i < 4; i++) { floats[i] = FromPMColor(colors[i]); }
}

inline void SkPMFloat::To4PMColors(SkPMColor colors[4], const SkPMFloat floats[4]) {
    // Haven't beaten this yet.  Still faster than ClampTo4PMColors too.
    for (int i = 0; i < 4; i++) { colors[i] = floats[i].get(); }
}

inline void SkPMFloat::ClampTo4PMColors(SkPMColor colors[4], const SkPMFloat floats[4]) {
    // Same as _SSE2.h's.  We use 3 _mm_packus_epi16() where the naive loop uses 8.
    __m128i c0 = _mm_cvtps_epi32(_mm_load_ps(floats[0].fColor)),  // _mm_cvtps_epi32 rounds for us!
            c1 = _mm_cvtps_epi32(_mm_load_ps(floats[1].fColor)),
            c2 = _mm_cvtps_epi32(_mm_load_ps(floats[2].fColor)),
            c3 = _mm_cvtps_epi32(_mm_load_ps(floats[3].fColor));
    __m128i c3210 = _mm_packus_epi16(_mm_packus_epi16(c0, c1),
                                     _mm_packus_epi16(c2, c3));
    _mm_storeu_si128((__m128i*)colors, c3210);
    SkPMColorAssert(colors[0]);
    SkPMColorAssert(colors[1]);
    SkPMColorAssert(colors[2]);
    SkPMColorAssert(colors[3]);
}
