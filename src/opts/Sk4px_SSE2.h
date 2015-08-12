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
        __m128i as = _mm_srli_epi32(this->fVec, 24);   // ___3 ___2 ___1 ___0
        as = _mm_or_si128(as, _mm_slli_si128(as, 1));  // __33 __22 __11 __00
        as = _mm_or_si128(as, _mm_slli_si128(as, 2));  // 3333 2222 1111 0000
        return Sk16b(as);
    }

    inline Sk4px Sk4px::Load4Alphas(const SkAlpha a[4]) {
        __m128i as = _mm_cvtsi32_si128(*(const uint32_t*)a);  // ____ ____ ____ 3210
        as = _mm_unpacklo_epi8 (as, _mm_setzero_si128());     // ____ ____ _3_2 _1_0
        as = _mm_unpacklo_epi16(as, _mm_setzero_si128());     // ___3 ___2 ___1 ___0
        as = _mm_or_si128(as, _mm_slli_si128(as, 1));         // __33 __22 __11 __00
        as = _mm_or_si128(as, _mm_slli_si128(as, 2));         // 3333 2222 1111 0000
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

static inline __m128i widen_low_half_to_8888(__m128i v) {
    // RGB565 format:   |R....|G.....|B....|
    //           Bit:  16    11      5     0

    // First get each pixel into its own 32-bit lane.
    //      v == ____ ____  ____ ____  rgb3 rgb2  rgb1 rgb0
    // spread == 0000 rgb3  0000 rgb2  0000 rgb1  0000 rgb0
    auto spread = _mm_unpacklo_epi16(v, _mm_setzero_si128());

    // Get each color independently, still in 565 precison but down at bit 0.
    auto r5 = _mm_srli_epi32(spread, 11),
         g6 = _mm_and_si128(_mm_set1_epi32(63), _mm_srli_epi32(spread,  5)),
         b5 = _mm_and_si128(_mm_set1_epi32(31), spread);

    // Scale 565 precision up to 8-bit each, filling low 323 bits with high bits of each component.
    auto r8 = _mm_or_si128(_mm_slli_epi32(r5, 3), _mm_srli_epi32(r5, 2)),
         g8 = _mm_or_si128(_mm_slli_epi32(g6, 2), _mm_srli_epi32(g6, 4)),
         b8 = _mm_or_si128(_mm_slli_epi32(b5, 3), _mm_srli_epi32(b5, 2));

    // Now put all the 8-bit components into SkPMColor order.
    return _mm_or_si128(_mm_slli_epi32(r8, SK_R32_SHIFT),   // TODO: one of these shifts is zero...
           _mm_or_si128(_mm_slli_epi32(g8, SK_G32_SHIFT),
           _mm_or_si128(_mm_slli_epi32(b8, SK_B32_SHIFT),
                        _mm_set1_epi32(0xFF << SK_A32_SHIFT))));
}

static inline __m128i narrow_to_565(__m128i w) {
    // Extract out top RGB 565 bits of each pixel, with no rounding.
    auto r5 = _mm_and_si128(_mm_set1_epi32(31), _mm_srli_epi32(w, SK_R32_SHIFT + 3)),
         g6 = _mm_and_si128(_mm_set1_epi32(63), _mm_srli_epi32(w, SK_G32_SHIFT + 2)),
         b5 = _mm_and_si128(_mm_set1_epi32(31), _mm_srli_epi32(w, SK_B32_SHIFT + 3));

    // Now put the bits in place in the low 16-bits of each 32-bit lane.
    auto spread = _mm_or_si128(_mm_slli_epi32(r5, 11),
                  _mm_or_si128(_mm_slli_epi32(g6,  5),
                               b5));

    // We want to pack the bottom 16-bits of spread down into the low half of the register, v.
    // spread == 0000 rgb3  0000 rgb2  0000 rgb1  0000 rgb0
    //      v == ____ ____  ____ ____  rgb3 rgb2  rgb1 rgb0

    // Ideally now we'd use _mm_packus_epi32(spread, <anything>) to pack v.  But that's from SSE4.
    // With only SSE2, we need to use _mm_packs_epi32.  That does signed saturation, and
    // we need to preserve all 16 bits.  So we pretend our data is signed by sign-extending first.
    // TODO: is it faster to just _mm_shuffle_epi8 this when we have SSSE3?
    auto signExtended = _mm_srai_epi32(_mm_slli_epi32(spread, 16), 16);
    auto v = _mm_packs_epi32(signExtended, signExtended);
    return v;
}

inline Sk4px Sk4px::Load4(const SkPMColor16 src[4]) {
    return Sk16b(widen_low_half_to_8888(_mm_loadl_epi64((const __m128i*)src)));
}
inline Sk4px Sk4px::Load2(const SkPMColor16 src[2]) {
    auto src2 = ((uint32_t)src[0]      )
              | ((uint32_t)src[1] << 16);
    return Sk16b(widen_low_half_to_8888(_mm_cvtsi32_si128(src2)));
}
inline Sk4px Sk4px::Load1(const SkPMColor16 src[1]) {
    return Sk16b(widen_low_half_to_8888(_mm_insert_epi16(_mm_setzero_si128(), src[0], 0)));
}

inline void Sk4px::store4(SkPMColor16 dst[4]) const {
    _mm_storel_epi64((__m128i*)dst, narrow_to_565(this->fVec));
}
inline void Sk4px::store2(SkPMColor16 dst[2]) const {
    uint32_t dst2 = _mm_cvtsi128_si32(narrow_to_565(this->fVec));
    dst[0] = dst2;
    dst[1] = dst2 >> 16;
}
inline void Sk4px::store1(SkPMColor16 dst[1]) const {
    uint32_t dst2 = _mm_cvtsi128_si32(narrow_to_565(this->fVec));
    dst[0] = dst2;
}

}  // namespace
