#include "SkColorPriv.h"
#include <tmmintrin.h>

// For set(), we widen our 8 bit components (fix8) to 8-bit components in 32 bits (fix8_32),
// then convert those to floats.

// get() does the opposite, working from floats to 8-bit-in-32-bits, then back to packed 8 bit.

// clamped() is the same as _SSE2: floats to 8-in-32, to 8-in-16, to packed 8 bit, with
// _mm_packus_epi16() both clamping and narrowing.

inline void SkPMFloat::set(SkPMColor c) {
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
