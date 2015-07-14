/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNx_sse_DEFINED
#define SkNx_sse_DEFINED

// This file may assume <= SSE2, but must check SK_CPU_SSE_LEVEL for anything more recent.

namespace {  // See SkNx.h


template <>
class SkNf<2, float> {
public:
    SkNf(const __m128& vec) : fVec(vec) {}

    SkNf() {}
    explicit SkNf(float val) : fVec(_mm_set1_ps(val)) {}
    static SkNf Load(const float vals[2]) {
        return _mm_castsi128_ps(_mm_loadl_epi64((const __m128i*)vals));
    }
    SkNf(float a, float b) : fVec(_mm_setr_ps(a,b,0,0)) {}

    void store(float vals[2]) const { _mm_storel_pi((__m64*)vals, fVec); }

    SkNf operator + (const SkNf& o) const { return _mm_add_ps(fVec, o.fVec); }
    SkNf operator - (const SkNf& o) const { return _mm_sub_ps(fVec, o.fVec); }
    SkNf operator * (const SkNf& o) const { return _mm_mul_ps(fVec, o.fVec); }
    SkNf operator / (const SkNf& o) const { return _mm_div_ps(fVec, o.fVec); }

    SkNf operator == (const SkNf& o) const { return _mm_cmpeq_ps (fVec, o.fVec); }
    SkNf operator != (const SkNf& o) const { return _mm_cmpneq_ps(fVec, o.fVec); }
    SkNf operator  < (const SkNf& o) const { return _mm_cmplt_ps (fVec, o.fVec); }
    SkNf operator  > (const SkNf& o) const { return _mm_cmpgt_ps (fVec, o.fVec); }
    SkNf operator <= (const SkNf& o) const { return _mm_cmple_ps (fVec, o.fVec); }
    SkNf operator >= (const SkNf& o) const { return _mm_cmpge_ps (fVec, o.fVec); }

    static SkNf Min(const SkNf& l, const SkNf& r) { return _mm_min_ps(l.fVec, r.fVec); }
    static SkNf Max(const SkNf& l, const SkNf& r) { return _mm_max_ps(l.fVec, r.fVec); }

    SkNf  sqrt() const { return _mm_sqrt_ps (fVec);  }
    SkNf rsqrt0() const { return _mm_rsqrt_ps(fVec); }
    SkNf rsqrt1() const { return this->rsqrt0(); }
    SkNf rsqrt2() const { return this->rsqrt1(); }

    SkNf       invert() const { return SkNf(1) / *this; }
    SkNf approxInvert() const { return _mm_rcp_ps(fVec); }

    template <int k> float kth() const {
        SkASSERT(0 <= k && k < 2);
        union { __m128 v; float fs[4]; } pun = {fVec};
        return pun.fs[k&1];
    }

    bool allTrue() const { return 0xff == (_mm_movemask_epi8(_mm_castps_si128(fVec)) & 0xff); }
    bool anyTrue() const { return 0x00 != (_mm_movemask_epi8(_mm_castps_si128(fVec)) & 0xff); }

    __m128 fVec;
};

template <>
class SkNf<2, double> {
public:
    SkNf(const __m128d& vec) : fVec(vec) {}

    SkNf() {}
    explicit SkNf(double val)           : fVec( _mm_set1_pd(val) ) {}
    static SkNf Load(const double vals[2]) { return _mm_loadu_pd(vals); }
    SkNf(double a, double b) : fVec(_mm_setr_pd(a,b)) {}

    void store(double vals[2]) const { _mm_storeu_pd(vals, fVec); }

    SkNf operator + (const SkNf& o) const { return _mm_add_pd(fVec, o.fVec); }
    SkNf operator - (const SkNf& o) const { return _mm_sub_pd(fVec, o.fVec); }
    SkNf operator * (const SkNf& o) const { return _mm_mul_pd(fVec, o.fVec); }
    SkNf operator / (const SkNf& o) const { return _mm_div_pd(fVec, o.fVec); }

    SkNf operator == (const SkNf& o) const { return _mm_cmpeq_pd (fVec, o.fVec); }
    SkNf operator != (const SkNf& o) const { return _mm_cmpneq_pd(fVec, o.fVec); }
    SkNf operator  < (const SkNf& o) const { return _mm_cmplt_pd (fVec, o.fVec); }
    SkNf operator  > (const SkNf& o) const { return _mm_cmpgt_pd (fVec, o.fVec); }
    SkNf operator <= (const SkNf& o) const { return _mm_cmple_pd (fVec, o.fVec); }
    SkNf operator >= (const SkNf& o) const { return _mm_cmpge_pd (fVec, o.fVec); }

    static SkNf Min(const SkNf& l, const SkNf& r) { return _mm_min_pd(l.fVec, r.fVec); }
    static SkNf Max(const SkNf& l, const SkNf& r) { return _mm_max_pd(l.fVec, r.fVec); }

    SkNf  sqrt() const { return _mm_sqrt_pd(fVec);  }
    SkNf rsqrt0() const { return _mm_cvtps_pd(_mm_rsqrt_ps(_mm_cvtpd_ps(fVec))); }
    SkNf rsqrt1() const { return this->rsqrt0(); }
    SkNf rsqrt2() const { return this->rsqrt1(); }

    SkNf       invert() const { return SkNf(1) / *this; }
    SkNf approxInvert() const { return _mm_cvtps_pd(_mm_rcp_ps(_mm_cvtpd_ps(fVec))); }

    template <int k> double kth() const {
        SkASSERT(0 <= k && k < 2);
        union { __m128d v; double ds[2]; } pun = {fVec};
        return pun.ds[k&1];
    }

    bool allTrue() const { return 0xffff == _mm_movemask_epi8(_mm_castpd_si128(fVec)); }
    bool anyTrue() const { return 0x0000 != _mm_movemask_epi8(_mm_castpd_si128(fVec)); }

    __m128d fVec;
};

template <>
class SkNi<4, int> {
public:
    SkNi(const __m128i& vec) : fVec(vec) {}

    SkNi() {}
    explicit SkNi(int val) : fVec(_mm_set1_epi32(val)) {}
    static SkNi Load(const int vals[4]) { return _mm_loadu_si128((const __m128i*)vals); }
    SkNi(int a, int b, int c, int d) : fVec(_mm_setr_epi32(a,b,c,d)) {}

    void store(int vals[4]) const { _mm_storeu_si128((__m128i*)vals, fVec); }

    SkNi operator + (const SkNi& o) const { return _mm_add_epi32(fVec, o.fVec); }
    SkNi operator - (const SkNi& o) const { return _mm_sub_epi32(fVec, o.fVec); }
    SkNi operator * (const SkNi& o) const {
        __m128i mul20 = _mm_mul_epu32(fVec, o.fVec),
                mul31 = _mm_mul_epu32(_mm_srli_si128(fVec, 4), _mm_srli_si128(o.fVec, 4));
        return _mm_unpacklo_epi32(_mm_shuffle_epi32(mul20, _MM_SHUFFLE(0,0,2,0)),
                                  _mm_shuffle_epi32(mul31, _MM_SHUFFLE(0,0,2,0)));
    }

    SkNi operator << (int bits) const { return _mm_slli_epi32(fVec, bits); }
    SkNi operator >> (int bits) const { return _mm_srai_epi32(fVec, bits); }

    template <int k> int kth() const {
        SkASSERT(0 <= k && k < 4);
        switch (k) {
            case 0: return _mm_cvtsi128_si32(fVec);
            case 1: return _mm_cvtsi128_si32(_mm_srli_si128(fVec,  4));
            case 2: return _mm_cvtsi128_si32(_mm_srli_si128(fVec,  8));
            case 3: return _mm_cvtsi128_si32(_mm_srli_si128(fVec, 12));
            default: SkASSERT(false); return 0;
        }
    }

    __m128i fVec;
};

template <>
class SkNf<4, float> {
public:
    SkNf(const __m128& vec) : fVec(vec) {}

    SkNf() {}
    explicit SkNf(float val)           : fVec( _mm_set1_ps(val) ) {}
    static SkNf Load(const float vals[4]) { return _mm_loadu_ps(vals); }
    SkNf(float a, float b, float c, float d) : fVec(_mm_setr_ps(a,b,c,d)) {}

    void store(float vals[4]) const { _mm_storeu_ps(vals, fVec); }

    SkNi<4, int> castTrunc() const { return _mm_cvttps_epi32(fVec); }

    SkNf operator + (const SkNf& o) const { return _mm_add_ps(fVec, o.fVec); }
    SkNf operator - (const SkNf& o) const { return _mm_sub_ps(fVec, o.fVec); }
    SkNf operator * (const SkNf& o) const { return _mm_mul_ps(fVec, o.fVec); }
    SkNf operator / (const SkNf& o) const { return _mm_div_ps(fVec, o.fVec); }

    SkNf operator == (const SkNf& o) const { return _mm_cmpeq_ps (fVec, o.fVec); }
    SkNf operator != (const SkNf& o) const { return _mm_cmpneq_ps(fVec, o.fVec); }
    SkNf operator  < (const SkNf& o) const { return _mm_cmplt_ps (fVec, o.fVec); }
    SkNf operator  > (const SkNf& o) const { return _mm_cmpgt_ps (fVec, o.fVec); }
    SkNf operator <= (const SkNf& o) const { return _mm_cmple_ps (fVec, o.fVec); }
    SkNf operator >= (const SkNf& o) const { return _mm_cmpge_ps (fVec, o.fVec); }

    static SkNf Min(const SkNf& l, const SkNf& r) { return _mm_min_ps(l.fVec, r.fVec); }
    static SkNf Max(const SkNf& l, const SkNf& r) { return _mm_max_ps(l.fVec, r.fVec); }

    SkNf  sqrt() const { return _mm_sqrt_ps (fVec);  }
    SkNf rsqrt0() const { return _mm_rsqrt_ps(fVec); }
    SkNf rsqrt1() const { return this->rsqrt0(); }
    SkNf rsqrt2() const { return this->rsqrt1(); }

    SkNf       invert() const { return SkNf(1) / *this; }
    SkNf approxInvert() const { return _mm_rcp_ps(fVec); }

    template <int k> float kth() const {
        SkASSERT(0 <= k && k < 4);
        union { __m128 v; float fs[4]; } pun = {fVec};
        return pun.fs[k&3];
    }

    bool allTrue() const { return 0xffff == _mm_movemask_epi8(_mm_castps_si128(fVec)); }
    bool anyTrue() const { return 0x0000 != _mm_movemask_epi8(_mm_castps_si128(fVec)); }

    SkNf thenElse(const SkNf& t, const SkNf& e) const {
        return _mm_or_ps(_mm_and_ps   (fVec, t.fVec),
                         _mm_andnot_ps(fVec, e.fVec));
    }

    __m128 fVec;
};

template <>
class SkNi<4, uint16_t> {
public:
    SkNi(const __m128i& vec) : fVec(vec) {}

    SkNi() {}
    explicit SkNi(uint16_t val) : fVec(_mm_set1_epi16(val)) {}
    static SkNi Load(const uint16_t vals[4]) { return _mm_loadl_epi64((const __m128i*)vals); }
    SkNi(uint16_t a, uint16_t b, uint16_t c, uint16_t d) : fVec(_mm_setr_epi16(a,b,c,d,0,0,0,0)) {}

    void store(uint16_t vals[4]) const { _mm_storel_epi64((__m128i*)vals, fVec); }

    SkNi operator + (const SkNi& o) const { return _mm_add_epi16(fVec, o.fVec); }
    SkNi operator - (const SkNi& o) const { return _mm_sub_epi16(fVec, o.fVec); }
    SkNi operator * (const SkNi& o) const { return _mm_mullo_epi16(fVec, o.fVec); }

    SkNi operator << (int bits) const { return _mm_slli_epi16(fVec, bits); }
    SkNi operator >> (int bits) const { return _mm_srli_epi16(fVec, bits); }

    template <int k> uint16_t kth() const {
        SkASSERT(0 <= k && k < 4);
        return _mm_extract_epi16(fVec, k);
    }

    __m128i fVec;
};

template <>
class SkNi<8, uint16_t> {
public:
    SkNi(const __m128i& vec) : fVec(vec) {}

    SkNi() {}
    explicit SkNi(uint16_t val) : fVec(_mm_set1_epi16(val)) {}
    static SkNi Load(const uint16_t vals[8]) { return _mm_loadu_si128((const __m128i*)vals); }
    SkNi(uint16_t a, uint16_t b, uint16_t c, uint16_t d,
         uint16_t e, uint16_t f, uint16_t g, uint16_t h) : fVec(_mm_setr_epi16(a,b,c,d,e,f,g,h)) {}

    void store(uint16_t vals[8]) const { _mm_storeu_si128((__m128i*)vals, fVec); }

    SkNi operator + (const SkNi& o) const { return _mm_add_epi16(fVec, o.fVec); }
    SkNi operator - (const SkNi& o) const { return _mm_sub_epi16(fVec, o.fVec); }
    SkNi operator * (const SkNi& o) const { return _mm_mullo_epi16(fVec, o.fVec); }

    SkNi operator << (int bits) const { return _mm_slli_epi16(fVec, bits); }
    SkNi operator >> (int bits) const { return _mm_srli_epi16(fVec, bits); }

    static SkNi Min(const SkNi& a, const SkNi& b) {
        // No unsigned _mm_min_epu16, so we'll shift into a space where we can use the
        // signed version, _mm_min_epi16, then shift back.
        const uint16_t top = 0x8000; // Keep this separate from _mm_set1_epi16 or MSVC will whine.
        const __m128i top_8x = _mm_set1_epi16(top);
        return _mm_add_epi8(top_8x, _mm_min_epi16(_mm_sub_epi8(a.fVec, top_8x),
                                                  _mm_sub_epi8(b.fVec, top_8x)));
    }

    SkNi thenElse(const SkNi& t, const SkNi& e) const {
        return _mm_or_si128(_mm_and_si128   (fVec, t.fVec),
                            _mm_andnot_si128(fVec, e.fVec));
    }

    template <int k> uint16_t kth() const {
        SkASSERT(0 <= k && k < 8);
        return _mm_extract_epi16(fVec, k);
    }

    __m128i fVec;
};

template <>
class SkNi<16, uint8_t> {
public:
    SkNi(const __m128i& vec) : fVec(vec) {}

    SkNi() {}
    explicit SkNi(uint8_t val) : fVec(_mm_set1_epi8(val)) {}
    static SkNi Load(const uint8_t vals[16]) { return _mm_loadu_si128((const __m128i*)vals); }
    SkNi(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
         uint8_t e, uint8_t f, uint8_t g, uint8_t h,
         uint8_t i, uint8_t j, uint8_t k, uint8_t l,
         uint8_t m, uint8_t n, uint8_t o, uint8_t p)
        : fVec(_mm_setr_epi8(a,b,c,d, e,f,g,h, i,j,k,l, m,n,o,p)) {}

    void store(uint8_t vals[16]) const { _mm_storeu_si128((__m128i*)vals, fVec); }

    SkNi saturatedAdd(const SkNi& o) const { return _mm_adds_epu8(fVec, o.fVec); }

    SkNi operator + (const SkNi& o) const { return _mm_add_epi8(fVec, o.fVec); }
    SkNi operator - (const SkNi& o) const { return _mm_sub_epi8(fVec, o.fVec); }

    static SkNi Min(const SkNi& a, const SkNi& b) { return _mm_min_epu8(a.fVec, b.fVec); }
    SkNi operator < (const SkNi& o) const {
        // There's no unsigned _mm_cmplt_epu8, so we flip the sign bits then use a signed compare.
        auto flip = _mm_set1_epi8(char(0x80));
        return _mm_cmplt_epi8(_mm_xor_si128(flip, fVec), _mm_xor_si128(flip, o.fVec));
    }

    template <int k> uint8_t kth() const {
        SkASSERT(0 <= k && k < 16);
        // SSE4.1 would just `return _mm_extract_epi8(fVec, k)`.  We have to read 16-bits instead.
        int pair = _mm_extract_epi16(fVec, k/2);
        return k % 2 == 0 ? pair : (pair >> 8);
    }

    SkNi thenElse(const SkNi& t, const SkNi& e) const {
        return _mm_or_si128(_mm_and_si128   (fVec, t.fVec),
                            _mm_andnot_si128(fVec, e.fVec));
    }

    __m128i fVec;
};

}  // namespace

#endif//SkNx_sse_DEFINED
