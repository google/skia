/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

inline Sk4px::Sk4px(SkPMColor px) : INHERITED(_mm_set1_epi32(px)) {}

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

inline Sk4px::Wide Sk4px::mulWiden(const Sk16b& other) const {
    return this->widenLo() * Sk4px(other).widenLo();
}

inline Sk4px Sk4px::Wide::addNarrowHi(const Sk16h& other) const {
    Sk4px::Wide r = (*this + other) >> 8;
    return Sk4px(_mm_packus_epi16(r.fLo.fVec, r.fHi.fVec));
}
