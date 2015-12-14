/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNx_avx_DEFINED
#define SkNx_avx_DEFINED

// This file may assume <= AVX, but must check SK_CPU_SSE_LEVEL for anything more recent.

// All the SSE specializations are still good ideas.  We'll just add Sk8f.
#include "SkNx_sse.h"

// SkNx_sse.h defines SKNX_IS_FAST.

namespace {  // See SkNx.h

template <>
class SkNx<8, float> {
public:
    SkNx(const __m256& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(float val) : fVec(_mm256_set1_ps(val)) {}
    static SkNx Load(const float vals[8]) { return _mm256_loadu_ps(vals); }

    SkNx(float a, float b, float c, float d,
         float e, float f, float g, float h) : fVec(_mm256_setr_ps(a,b,c,d,e,f,g,h)) {}

    void store(float vals[8]) const { _mm256_storeu_ps(vals, fVec); }

    SkNx operator + (const SkNx& o) const { return _mm256_add_ps(fVec, o.fVec); }
    SkNx operator - (const SkNx& o) const { return _mm256_sub_ps(fVec, o.fVec); }
    SkNx operator * (const SkNx& o) const { return _mm256_mul_ps(fVec, o.fVec); }
    SkNx operator / (const SkNx& o) const { return _mm256_div_ps(fVec, o.fVec); }

    SkNx operator == (const SkNx& o) const { return _mm256_cmp_ps(fVec, o.fVec, _CMP_EQ_OQ); }
    SkNx operator != (const SkNx& o) const { return _mm256_cmp_ps(fVec, o.fVec, _CMP_NEQ_OQ); }
    SkNx operator  < (const SkNx& o) const { return _mm256_cmp_ps(fVec, o.fVec, _CMP_LT_OQ); }
    SkNx operator  > (const SkNx& o) const { return _mm256_cmp_ps(fVec, o.fVec, _CMP_GT_OQ); }
    SkNx operator <= (const SkNx& o) const { return _mm256_cmp_ps(fVec, o.fVec, _CMP_LE_OQ); }
    SkNx operator >= (const SkNx& o) const { return _mm256_cmp_ps(fVec, o.fVec, _CMP_GE_OQ); }

    static SkNx Min(const SkNx& l, const SkNx& r) { return _mm256_min_ps(l.fVec, r.fVec); }
    static SkNx Max(const SkNx& l, const SkNx& r) { return _mm256_max_ps(l.fVec, r.fVec); }

    SkNx  sqrt() const { return _mm256_sqrt_ps (fVec);  }
    SkNx rsqrt0() const { return _mm256_rsqrt_ps(fVec); }
    SkNx rsqrt1() const { return this->rsqrt0(); }
    SkNx rsqrt2() const { return this->rsqrt1(); }

    SkNx       invert() const { return SkNx(1) / *this; }
    SkNx approxInvert() const { return _mm256_rcp_ps(fVec); }

    template <int k> float kth() const {
        SkASSERT(0 <= k && k < 8);
        union { __m256 v; float fs[8]; } pun = {fVec};
        return pun.fs[k&7];
    }

    bool allTrue() const { return 0xff == _mm256_movemask_ps(fVec); }
    bool anyTrue() const { return 0x00 != _mm256_movemask_ps(fVec); }

    SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return _mm256_blendv_ps(e.fVec, t.fVec, fVec);
    }

    __m256 fVec;
};

template<> inline Sk8b SkNx_cast<uint8_t, float, 8>(const Sk8f& src) {
    __m256i _32 = _mm256_cvttps_epi32(src.fVec);
    __m128i  lo = _mm256_extractf128_si256(_32, 0),
             hi = _mm256_extractf128_si256(_32, 1),
            _16 = _mm_packus_epi32(lo, hi);
    return _mm_packus_epi16(_16, _16);
}

template<> inline Sk8f SkNx_cast<float, uint8_t, 8>(const Sk8b& src) {
    /* TODO lo = _mm_cvtepu8_epi32(src.fVec),
     *      hi = _mm_cvtepu8_epi32(_mm_srli_si128(src.fVec, 4))
     */
    __m128i _16 = _mm_unpacklo_epi8(src.fVec, _mm_setzero_si128()),
             lo = _mm_unpacklo_epi16(_16, _mm_setzero_si128()),
             hi = _mm_unpackhi_epi16(_16, _mm_setzero_si128());
    __m256i _32 = _mm256_insertf128_si256(_mm256_castsi128_si256(lo), hi, 1);
    return _mm256_cvtepi32_ps(_32);
}

}  // namespace

#endif//SkNx_avx_DEFINED
