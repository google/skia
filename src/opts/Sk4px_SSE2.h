/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

namespace { // See Sk4px.h

inline Sk4px Sk4px::DupPMColor(SkPMColor px) { return Sk16b(_mm_set1_epi32(px)); }

inline Sk4px Sk4px::Load4(const SkPMColor px[4]) {
    return Sk16b(_mm_loadu_si128((const __m128i*)px));
}
inline Sk4px Sk4px::Load2(const SkPMColor px[2]) {
    return Sk16b(_mm_loadl_epi64((const __m128i*)px));
}
inline Sk4px Sk4px::Load1(const SkPMColor px[1]) { return Sk16b(_mm_cvtsi32_si128(*px)); }

inline void Sk4px::store4(SkPMColor px[4]) const { _mm_storeu_si128((__m128i*)px, this->fVec); }
inline void Sk4px::store2(SkPMColor px[2]) const { _mm_storel_epi64((__m128i*)px, this->fVec); }
inline void Sk4px::store1(SkPMColor px[1]) const { *px = _mm_cvtsi128_si32(this->fVec); }

inline Sk4px::Wide Sk4px::widenLo() const {
    return Sk16h(_mm_unpacklo_epi8(this->fVec, _mm_setzero_si128()),
                 _mm_unpackhi_epi8(this->fVec, _mm_setzero_si128()));
}

inline Sk4px::Wide Sk4px::widenHi() const {
    return Sk16h(_mm_unpacklo_epi8(_mm_setzero_si128(), this->fVec),
                 _mm_unpackhi_epi8(_mm_setzero_si128(), this->fVec));
}

inline Sk4px::Wide Sk4px::widenLoHi() const {
    return Sk16h(_mm_unpacklo_epi8(this->fVec, this->fVec),
                 _mm_unpackhi_epi8(this->fVec, this->fVec));
}

inline Sk4px::Wide Sk4px::mulWiden(const Sk16b& other) const {
    return this->widenLo() * Sk4px(other).widenLo();
}

inline Sk4px Sk4px::Wide::addNarrowHi(const Sk16h& other) const {
    Sk4px::Wide r = (*this + other) >> 8;
    return Sk4px(_mm_packus_epi16(r.fLo.fVec, r.fHi.fVec));
}

inline Sk4px Sk4px::Wide::div255() const {
    // (x + 127) / 255 == ((x+128) * 257)>>16,
    // and _mm_mulhi_epu16 makes the (_ * 257)>>16 part very convenient.
    const __m128i _128 = _mm_set1_epi16(128),
                  _257 = _mm_set1_epi16(257);
    return Sk4px(_mm_packus_epi16(_mm_mulhi_epu16(_mm_add_epi16(fLo.fVec, _128), _257),
                                  _mm_mulhi_epu16(_mm_add_epi16(fHi.fVec, _128), _257)));
}

// Load4Alphas and Load2Alphas use possibly-unaligned loads (SkAlpha[] -> uint16_t or uint32_t).
// These are safe on x86, often with no speed penalty.

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
    inline Sk4px Sk4px::alphas() const {
        static_assert(SK_A32_SHIFT == 24, "Intel's always little-endian.");
        __m128i splat = _mm_set_epi8(15,15,15,15, 11,11,11,11, 7,7,7,7, 3,3,3,3);
        return Sk16b(_mm_shuffle_epi8(this->fVec, splat));
    }

    inline Sk4px Sk4px::Load4Alphas(const SkAlpha a[4]) {
        uint32_t as = *(const uint32_t*)a;
        __m128i splat = _mm_set_epi8(3,3,3,3, 2,2,2,2, 1,1,1,1, 0,0,0,0);
        return Sk16b(_mm_shuffle_epi8(_mm_cvtsi32_si128(as), splat));
    }
#else
    inline Sk4px Sk4px::alphas() const {
        static_assert(SK_A32_SHIFT == 24, "Intel's always little-endian.");
        // We exploit that A >= rgb for any premul pixel.
        __m128i as = fVec;                             // 3xxx 2xxx 1xxx 0xxx
        as = _mm_max_epu8(as, _mm_srli_epi32(as,  8)); // 33xx 22xx 11xx 00xx
        as = _mm_max_epu8(as, _mm_srli_epi32(as, 16)); // 3333 2222 1111 0000
        return Sk16b(as);
    }

    inline Sk4px Sk4px::Load4Alphas(const SkAlpha a[4]) {
        __m128i as = _mm_cvtsi32_si128(*(const uint32_t*)a);  // ____ ____ ____ 3210
        as = _mm_unpacklo_epi8 (as, as);                      // ____ ____ 3322 1100
        as = _mm_unpacklo_epi16(as, as);                      // 3333 2222 1111 0000
        return Sk16b(as);
    }
#endif

inline Sk4px Sk4px::Load2Alphas(const SkAlpha a[2]) {
    uint32_t as = *(const uint16_t*)a;   // Aa -> Aa00
    return Load4Alphas((const SkAlpha*)&as);
}

inline Sk4px Sk4px::zeroColors() const {
    return Sk16b(_mm_and_si128(_mm_set1_epi32(0xFF << SK_A32_SHIFT), this->fVec));
}

inline Sk4px Sk4px::zeroAlphas() const {
    // andnot(a,b) == ~a & b
    return Sk16b(_mm_andnot_si128(_mm_set1_epi32(0xFF << SK_A32_SHIFT), this->fVec));
}

}  // namespace
