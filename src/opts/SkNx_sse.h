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
class SkNx<2, float> {
public:
    SkNx(const __m128& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(float val) : fVec(_mm_set1_ps(val)) {}
    static SkNx Load(const float vals[2]) {
        return _mm_castsi128_ps(_mm_loadl_epi64((const __m128i*)vals));
    }
    SkNx(float a, float b) : fVec(_mm_setr_ps(a,b,0,0)) {}

    void store(float vals[2]) const { _mm_storel_pi((__m64*)vals, fVec); }

    SkNx operator + (const SkNx& o) const { return _mm_add_ps(fVec, o.fVec); }
    SkNx operator - (const SkNx& o) const { return _mm_sub_ps(fVec, o.fVec); }
    SkNx operator * (const SkNx& o) const { return _mm_mul_ps(fVec, o.fVec); }
    SkNx operator / (const SkNx& o) const { return _mm_div_ps(fVec, o.fVec); }

    SkNx operator == (const SkNx& o) const { return _mm_cmpeq_ps (fVec, o.fVec); }
    SkNx operator != (const SkNx& o) const { return _mm_cmpneq_ps(fVec, o.fVec); }
    SkNx operator  < (const SkNx& o) const { return _mm_cmplt_ps (fVec, o.fVec); }
    SkNx operator  > (const SkNx& o) const { return _mm_cmpgt_ps (fVec, o.fVec); }
    SkNx operator <= (const SkNx& o) const { return _mm_cmple_ps (fVec, o.fVec); }
    SkNx operator >= (const SkNx& o) const { return _mm_cmpge_ps (fVec, o.fVec); }

    static SkNx Min(const SkNx& l, const SkNx& r) { return _mm_min_ps(l.fVec, r.fVec); }
    static SkNx Max(const SkNx& l, const SkNx& r) { return _mm_max_ps(l.fVec, r.fVec); }

    SkNx  sqrt() const { return _mm_sqrt_ps (fVec);  }
    SkNx rsqrt0() const { return _mm_rsqrt_ps(fVec); }
    SkNx rsqrt1() const { return this->rsqrt0(); }
    SkNx rsqrt2() const { return this->rsqrt1(); }

    SkNx       invert() const { return SkNx(1) / *this; }
    SkNx approxInvert() const { return _mm_rcp_ps(fVec); }

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
class SkNx<4, int> {
public:
    SkNx(const __m128i& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(int val) : fVec(_mm_set1_epi32(val)) {}
    static SkNx Load(const int vals[4]) { return _mm_loadu_si128((const __m128i*)vals); }
    SkNx(int a, int b, int c, int d) : fVec(_mm_setr_epi32(a,b,c,d)) {}

    void store(int vals[4]) const { _mm_storeu_si128((__m128i*)vals, fVec); }

    SkNx operator + (const SkNx& o) const { return _mm_add_epi32(fVec, o.fVec); }
    SkNx operator - (const SkNx& o) const { return _mm_sub_epi32(fVec, o.fVec); }
    SkNx operator * (const SkNx& o) const {
        __m128i mul20 = _mm_mul_epu32(fVec, o.fVec),
                mul31 = _mm_mul_epu32(_mm_srli_si128(fVec, 4), _mm_srli_si128(o.fVec, 4));
        return _mm_unpacklo_epi32(_mm_shuffle_epi32(mul20, _MM_SHUFFLE(0,0,2,0)),
                                  _mm_shuffle_epi32(mul31, _MM_SHUFFLE(0,0,2,0)));
    }

    SkNx operator << (int bits) const { return _mm_slli_epi32(fVec, bits); }
    SkNx operator >> (int bits) const { return _mm_srai_epi32(fVec, bits); }

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
class SkNx<4, float> {
public:
    SkNx(const __m128& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(float val)           : fVec( _mm_set1_ps(val) ) {}
    static SkNx Load(const float vals[4]) { return _mm_loadu_ps(vals); }

    static SkNx FromBytes(const uint8_t bytes[4]) {
        __m128i fix8 = _mm_cvtsi32_si128(*(const int*)bytes);
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
        const char _ = ~0;  // Zero these bytes.
        __m128i fix8_32 = _mm_shuffle_epi8(fix8, _mm_setr_epi8(0,_,_,_, 1,_,_,_, 2,_,_,_, 3,_,_,_));
    #else
        __m128i fix8_16 = _mm_unpacklo_epi8 (fix8,    _mm_setzero_si128()),
                fix8_32 = _mm_unpacklo_epi16(fix8_16, _mm_setzero_si128());
    #endif
        return SkNx(_mm_cvtepi32_ps(fix8_32));
        // TODO: use _mm_cvtepu8_epi32 w/SSE4.1?
    }

    SkNx(float a, float b, float c, float d) : fVec(_mm_setr_ps(a,b,c,d)) {}

    void store(float vals[4]) const { _mm_storeu_ps(vals, fVec); }
    void toBytes(uint8_t bytes[4]) const {
        __m128i fix8_32 = _mm_cvttps_epi32(fVec),
                fix8_16 = _mm_packus_epi16(fix8_32, fix8_32),
                fix8    = _mm_packus_epi16(fix8_16, fix8_16);
        *(int*)bytes = _mm_cvtsi128_si32(fix8);
    }

    static void ToBytes(uint8_t bytes[16],
                        const SkNx& a, const SkNx& b, const SkNx& c, const SkNx& d) {
        _mm_storeu_si128((__m128i*)bytes,
                         _mm_packus_epi16(_mm_packus_epi16(_mm_cvttps_epi32(a.fVec),
                                                           _mm_cvttps_epi32(b.fVec)),
                                          _mm_packus_epi16(_mm_cvttps_epi32(c.fVec),
                                                           _mm_cvttps_epi32(d.fVec))));
    }

    SkNx operator + (const SkNx& o) const { return _mm_add_ps(fVec, o.fVec); }
    SkNx operator - (const SkNx& o) const { return _mm_sub_ps(fVec, o.fVec); }
    SkNx operator * (const SkNx& o) const { return _mm_mul_ps(fVec, o.fVec); }
    SkNx operator / (const SkNx& o) const { return _mm_div_ps(fVec, o.fVec); }

    SkNx operator == (const SkNx& o) const { return _mm_cmpeq_ps (fVec, o.fVec); }
    SkNx operator != (const SkNx& o) const { return _mm_cmpneq_ps(fVec, o.fVec); }
    SkNx operator  < (const SkNx& o) const { return _mm_cmplt_ps (fVec, o.fVec); }
    SkNx operator  > (const SkNx& o) const { return _mm_cmpgt_ps (fVec, o.fVec); }
    SkNx operator <= (const SkNx& o) const { return _mm_cmple_ps (fVec, o.fVec); }
    SkNx operator >= (const SkNx& o) const { return _mm_cmpge_ps (fVec, o.fVec); }

    static SkNx Min(const SkNx& l, const SkNx& r) { return _mm_min_ps(l.fVec, r.fVec); }
    static SkNx Max(const SkNx& l, const SkNx& r) { return _mm_max_ps(l.fVec, r.fVec); }

    SkNx  sqrt() const { return _mm_sqrt_ps (fVec);  }
    SkNx rsqrt0() const { return _mm_rsqrt_ps(fVec); }
    SkNx rsqrt1() const { return this->rsqrt0(); }
    SkNx rsqrt2() const { return this->rsqrt1(); }

    SkNx       invert() const { return SkNx(1) / *this; }
    SkNx approxInvert() const { return _mm_rcp_ps(fVec); }

    template <int k> float kth() const {
        SkASSERT(0 <= k && k < 4);
        union { __m128 v; float fs[4]; } pun = {fVec};
        return pun.fs[k&3];
    }

    bool allTrue() const { return 0xffff == _mm_movemask_epi8(_mm_castps_si128(fVec)); }
    bool anyTrue() const { return 0x0000 != _mm_movemask_epi8(_mm_castps_si128(fVec)); }

    SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return _mm_or_ps(_mm_and_ps   (fVec, t.fVec),
                         _mm_andnot_ps(fVec, e.fVec));
    }

    __m128 fVec;
};

template <>
class SkNx<4, uint16_t> {
public:
    SkNx(const __m128i& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(uint16_t val) : fVec(_mm_set1_epi16(val)) {}
    static SkNx Load(const uint16_t vals[4]) { return _mm_loadl_epi64((const __m128i*)vals); }
    SkNx(uint16_t a, uint16_t b, uint16_t c, uint16_t d) : fVec(_mm_setr_epi16(a,b,c,d,0,0,0,0)) {}

    void store(uint16_t vals[4]) const { _mm_storel_epi64((__m128i*)vals, fVec); }

    SkNx operator + (const SkNx& o) const { return _mm_add_epi16(fVec, o.fVec); }
    SkNx operator - (const SkNx& o) const { return _mm_sub_epi16(fVec, o.fVec); }
    SkNx operator * (const SkNx& o) const { return _mm_mullo_epi16(fVec, o.fVec); }

    SkNx operator << (int bits) const { return _mm_slli_epi16(fVec, bits); }
    SkNx operator >> (int bits) const { return _mm_srli_epi16(fVec, bits); }

    template <int k> uint16_t kth() const {
        SkASSERT(0 <= k && k < 4);
        return _mm_extract_epi16(fVec, k);
    }

    __m128i fVec;
};

template <>
class SkNx<8, uint16_t> {
public:
    SkNx(const __m128i& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(uint16_t val) : fVec(_mm_set1_epi16(val)) {}
    static SkNx Load(const uint16_t vals[8]) { return _mm_loadu_si128((const __m128i*)vals); }
    SkNx(uint16_t a, uint16_t b, uint16_t c, uint16_t d,
         uint16_t e, uint16_t f, uint16_t g, uint16_t h) : fVec(_mm_setr_epi16(a,b,c,d,e,f,g,h)) {}

    void store(uint16_t vals[8]) const { _mm_storeu_si128((__m128i*)vals, fVec); }

    SkNx operator + (const SkNx& o) const { return _mm_add_epi16(fVec, o.fVec); }
    SkNx operator - (const SkNx& o) const { return _mm_sub_epi16(fVec, o.fVec); }
    SkNx operator * (const SkNx& o) const { return _mm_mullo_epi16(fVec, o.fVec); }

    SkNx operator << (int bits) const { return _mm_slli_epi16(fVec, bits); }
    SkNx operator >> (int bits) const { return _mm_srli_epi16(fVec, bits); }

    static SkNx Min(const SkNx& a, const SkNx& b) {
        // No unsigned _mm_min_epu16, so we'll shift into a space where we can use the
        // signed version, _mm_min_epi16, then shift back.
        const uint16_t top = 0x8000; // Keep this separate from _mm_set1_epi16 or MSVC will whine.
        const __m128i top_8x = _mm_set1_epi16(top);
        return _mm_add_epi8(top_8x, _mm_min_epi16(_mm_sub_epi8(a.fVec, top_8x),
                                                  _mm_sub_epi8(b.fVec, top_8x)));
    }

    SkNx thenElse(const SkNx& t, const SkNx& e) const {
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
class SkNx<16, uint8_t> {
public:
    SkNx(const __m128i& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(uint8_t val) : fVec(_mm_set1_epi8(val)) {}
    static SkNx Load(const uint8_t vals[16]) { return _mm_loadu_si128((const __m128i*)vals); }
    SkNx(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
         uint8_t e, uint8_t f, uint8_t g, uint8_t h,
         uint8_t i, uint8_t j, uint8_t k, uint8_t l,
         uint8_t m, uint8_t n, uint8_t o, uint8_t p)
        : fVec(_mm_setr_epi8(a,b,c,d, e,f,g,h, i,j,k,l, m,n,o,p)) {}

    void store(uint8_t vals[16]) const { _mm_storeu_si128((__m128i*)vals, fVec); }

    SkNx saturatedAdd(const SkNx& o) const { return _mm_adds_epu8(fVec, o.fVec); }

    SkNx operator + (const SkNx& o) const { return _mm_add_epi8(fVec, o.fVec); }
    SkNx operator - (const SkNx& o) const { return _mm_sub_epi8(fVec, o.fVec); }

    static SkNx Min(const SkNx& a, const SkNx& b) { return _mm_min_epu8(a.fVec, b.fVec); }
    SkNx operator < (const SkNx& o) const {
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

    SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return _mm_or_si128(_mm_and_si128   (fVec, t.fVec),
                            _mm_andnot_si128(fVec, e.fVec));
    }

    __m128i fVec;
};


template<>
inline SkNx<4, int> SkNx_cast<int, float, 4>(const SkNx<4, float>& src) {
    return _mm_cvttps_epi32(src.fVec);
}

}  // namespace

#endif//SkNx_sse_DEFINED
