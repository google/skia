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

namespace {  // See SkNx.h

template <>
class SkNf<8> {
public:
    SkNf(const __m256& vec) : fVec(vec) {}

    SkNf() {}
    SkNf(float val) : fVec(_mm256_set1_ps(val)) {}
    static SkNf Load(const float vals[8]) { return _mm256_loadu_ps(vals); }

    static SkNf FromBytes(const uint8_t bytes[8]) {
        __m128i fix8  = _mm_loadl_epi64((const __m128i*)bytes),
                fix16 = _mm_unpacklo_epi8 (fix8 , _mm_setzero_si128()),
                 lo32 = _mm_unpacklo_epi16(fix16, _mm_setzero_si128()),
                 hi32 = _mm_unpackhi_epi16(fix16, _mm_setzero_si128());
        __m256i fix32 = _mm256_insertf128_si256(_mm256_castsi128_si256(lo32), hi32, 1);
        return _mm256_cvtepi32_ps(fix32);
    }

    SkNf(float a, float b, float c, float d,
         float e, float f, float g, float h) : fVec(_mm256_setr_ps(a,b,c,d,e,f,g,h)) {}

    void store(float vals[8]) const { _mm256_storeu_ps(vals, fVec); }
    void toBytes(uint8_t bytes[8]) const {
        __m256i fix32 = _mm256_cvttps_epi32(fVec);
        __m128i  lo32 = _mm256_extractf128_si256(fix32, 0),
                 hi32 = _mm256_extractf128_si256(fix32, 1),
                fix16 = _mm_packus_epi32(lo32, hi32),
                fix8  = _mm_packus_epi16(fix16, fix16);
        _mm_storel_epi64((__m128i*)bytes, fix8);
    }

    SkNf operator + (const SkNf& o) const { return _mm256_add_ps(fVec, o.fVec); }
    SkNf operator - (const SkNf& o) const { return _mm256_sub_ps(fVec, o.fVec); }
    SkNf operator * (const SkNf& o) const { return _mm256_mul_ps(fVec, o.fVec); }
    SkNf operator / (const SkNf& o) const { return _mm256_div_ps(fVec, o.fVec); }

    SkNf operator == (const SkNf& o) const { return _mm256_cmp_ps(fVec, o.fVec, _CMP_EQ_OQ); }
    SkNf operator != (const SkNf& o) const { return _mm256_cmp_ps(fVec, o.fVec, _CMP_NEQ_OQ); }
    SkNf operator  < (const SkNf& o) const { return _mm256_cmp_ps(fVec, o.fVec, _CMP_LT_OQ); }
    SkNf operator  > (const SkNf& o) const { return _mm256_cmp_ps(fVec, o.fVec, _CMP_GT_OQ); }
    SkNf operator <= (const SkNf& o) const { return _mm256_cmp_ps(fVec, o.fVec, _CMP_LE_OQ); }
    SkNf operator >= (const SkNf& o) const { return _mm256_cmp_ps(fVec, o.fVec, _CMP_GE_OQ); }

    static SkNf Min(const SkNf& l, const SkNf& r) { return _mm256_min_ps(l.fVec, r.fVec); }
    static SkNf Max(const SkNf& l, const SkNf& r) { return _mm256_max_ps(l.fVec, r.fVec); }

    SkNf  sqrt() const { return _mm256_sqrt_ps (fVec);  }
    SkNf rsqrt0() const { return _mm256_rsqrt_ps(fVec); }
    SkNf rsqrt1() const { return this->rsqrt0(); }
    SkNf rsqrt2() const { return this->rsqrt1(); }

    SkNf       invert() const { return SkNf(1) / *this; }
    SkNf approxInvert() const { return _mm256_rcp_ps(fVec); }

    template <int k> float kth() const {
        SkASSERT(0 <= k && k < 8);
        union { __m256 v; float fs[8]; } pun = {fVec};
        return pun.fs[k&7];
    }

    bool allTrue() const { return 0xff == _mm256_movemask_ps(fVec); }
    bool anyTrue() const { return 0x00 != _mm256_movemask_ps(fVec); }

    SkNf thenElse(const SkNf& t, const SkNf& e) const {
        return _mm256_blendv_ps(e.fVec, t.fVec, fVec);
    }

    __m256 fVec;
};

}  // namespace

#endif//SkNx_avx_DEFINED
