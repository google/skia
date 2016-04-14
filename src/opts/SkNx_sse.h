/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNx_sse_DEFINED
#define SkNx_sse_DEFINED

// This file may assume <= SSE2, but must check SK_CPU_SSE_LEVEL for anything more recent.
// If you do, make sure this is in a static inline function... anywhere else risks violating ODR.

#define SKNX_IS_FAST

// SSE 4.1 has _mm_floor_ps to floor 4 floats.  We emulate it:
//   - roundtrip through integers via truncation
//   - subtract 1 if that's too big (possible for negative values).
// This restricts the domain of our inputs to a maximum somehwere around 2^31.  Seems plenty big.
static inline __m128 sse2_mm_floor_ps(__m128 v) {
    __m128 roundtrip = _mm_cvtepi32_ps(_mm_cvttps_epi32(v));
    __m128 too_big = _mm_cmpgt_ps(roundtrip, v);
    return _mm_sub_ps(roundtrip, _mm_and_ps(too_big, _mm_set1_ps(1.0f)));
}

template <>
class SkNx<2, float> {
public:
    SkNx(const __m128& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(float val) : fVec(_mm_set1_ps(val)) {}
    static SkNx Load(const void* ptr) {
        return _mm_castsi128_ps(_mm_loadl_epi64((const __m128i*)ptr));
    }
    SkNx(float a, float b) : fVec(_mm_setr_ps(a,b,0,0)) {}

    void store(void* ptr) const { _mm_storel_pi((__m64*)ptr, fVec); }

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

    SkNx   sqrt() const { return _mm_sqrt_ps (fVec);  }
    SkNx  rsqrt() const { return _mm_rsqrt_ps(fVec); }
    SkNx invert() const { return _mm_rcp_ps(fVec); }

    float operator[](int k) const {
        SkASSERT(0 <= k && k < 2);
        union { __m128 v; float fs[4]; } pun = {fVec};
        return pun.fs[k&1];
    }

    bool allTrue() const { return 0xff == (_mm_movemask_epi8(_mm_castps_si128(fVec)) & 0xff); }
    bool anyTrue() const { return 0x00 != (_mm_movemask_epi8(_mm_castps_si128(fVec)) & 0xff); }

    __m128 fVec;
};

template <>
class SkNx<4, float> {
public:
    SkNx(const __m128& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(float val)           : fVec( _mm_set1_ps(val) ) {}
    static SkNx Load(const void* ptr) { return _mm_loadu_ps((const float*)ptr); }

    SkNx(float a, float b, float c, float d) : fVec(_mm_setr_ps(a,b,c,d)) {}

    void store(void* ptr) const { _mm_storeu_ps((float*)ptr, fVec); }

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

    SkNx abs() const { return _mm_andnot_ps(_mm_set1_ps(-0.0f), fVec); }
    SkNx floor() const { return sse2_mm_floor_ps(fVec); }

    SkNx   sqrt() const { return _mm_sqrt_ps (fVec);  }
    SkNx  rsqrt() const { return _mm_rsqrt_ps(fVec); }
    SkNx invert() const { return _mm_rcp_ps(fVec); }

    float operator[](int k) const {
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
class SkNx<4, int> {
public:
    SkNx(const __m128i& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(int val) : fVec(_mm_set1_epi32(val)) {}
    static SkNx Load(const void* ptr) { return _mm_loadu_si128((const __m128i*)ptr); }
    SkNx(int a, int b, int c, int d) : fVec(_mm_setr_epi32(a,b,c,d)) {}

    void store(void* ptr) const { _mm_storeu_si128((__m128i*)ptr, fVec); }

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

    int operator[](int k) const {
        SkASSERT(0 <= k && k < 4);
        union { __m128i v; int is[4]; } pun = {fVec};
        return pun.is[k&3];
    }

    __m128i fVec;
};

template <>
class SkNx<4, uint16_t> {
public:
    SkNx(const __m128i& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(uint16_t val) : fVec(_mm_set1_epi16(val)) {}
    static SkNx Load(const void* ptr) { return _mm_loadl_epi64((const __m128i*)ptr); }
    SkNx(uint16_t a, uint16_t b, uint16_t c, uint16_t d) : fVec(_mm_setr_epi16(a,b,c,d,0,0,0,0)) {}

    void store(void* ptr) const { _mm_storel_epi64((__m128i*)ptr, fVec); }

    SkNx operator + (const SkNx& o) const { return _mm_add_epi16(fVec, o.fVec); }
    SkNx operator - (const SkNx& o) const { return _mm_sub_epi16(fVec, o.fVec); }
    SkNx operator * (const SkNx& o) const { return _mm_mullo_epi16(fVec, o.fVec); }

    SkNx operator << (int bits) const { return _mm_slli_epi16(fVec, bits); }
    SkNx operator >> (int bits) const { return _mm_srli_epi16(fVec, bits); }

    uint16_t operator[](int k) const {
        SkASSERT(0 <= k && k < 4);
        union { __m128i v; uint16_t us[8]; } pun = {fVec};
        return pun.us[k&3];
    }

    __m128i fVec;
};

template <>
class SkNx<8, uint16_t> {
public:
    SkNx(const __m128i& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(uint16_t val) : fVec(_mm_set1_epi16(val)) {}
    static SkNx Load(const void* ptr) { return _mm_loadu_si128((const __m128i*)ptr); }
    SkNx(uint16_t a, uint16_t b, uint16_t c, uint16_t d,
         uint16_t e, uint16_t f, uint16_t g, uint16_t h) : fVec(_mm_setr_epi16(a,b,c,d,e,f,g,h)) {}

    void store(void* ptr) const { _mm_storeu_si128((__m128i*)ptr, fVec); }

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

    uint16_t operator[](int k) const {
        SkASSERT(0 <= k && k < 8);
        union { __m128i v; uint16_t us[8]; } pun = {fVec};
        return pun.us[k&7];
    }

    __m128i fVec;
};

template <>
class SkNx<4, uint8_t> {
public:
    SkNx() {}
    SkNx(const __m128i& vec) : fVec(vec) {}
    SkNx(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
        : fVec(_mm_setr_epi8(a,b,c,d, 0,0,0,0, 0,0,0,0, 0,0,0,0)) {}


    static SkNx Load(const void* ptr) { return _mm_cvtsi32_si128(*(const int*)ptr); }
    void store(void* ptr) const { *(int*)ptr = _mm_cvtsi128_si32(fVec); }

    uint8_t operator[](int k) const {
        SkASSERT(0 <= k && k < 4);
        union { __m128i v; uint8_t us[16]; } pun = {fVec};
        return pun.us[k&3];
    }

    // TODO as needed

    __m128i fVec;
};

template <>
class SkNx<16, uint8_t> {
public:
    SkNx(const __m128i& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(uint8_t val) : fVec(_mm_set1_epi8(val)) {}
    static SkNx Load(const void* ptr) { return _mm_loadu_si128((const __m128i*)ptr); }
    SkNx(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
         uint8_t e, uint8_t f, uint8_t g, uint8_t h,
         uint8_t i, uint8_t j, uint8_t k, uint8_t l,
         uint8_t m, uint8_t n, uint8_t o, uint8_t p)
        : fVec(_mm_setr_epi8(a,b,c,d, e,f,g,h, i,j,k,l, m,n,o,p)) {}

    void store(void* ptr) const { _mm_storeu_si128((__m128i*)ptr, fVec); }

    SkNx saturatedAdd(const SkNx& o) const { return _mm_adds_epu8(fVec, o.fVec); }

    SkNx operator + (const SkNx& o) const { return _mm_add_epi8(fVec, o.fVec); }
    SkNx operator - (const SkNx& o) const { return _mm_sub_epi8(fVec, o.fVec); }

    static SkNx Min(const SkNx& a, const SkNx& b) { return _mm_min_epu8(a.fVec, b.fVec); }
    SkNx operator < (const SkNx& o) const {
        // There's no unsigned _mm_cmplt_epu8, so we flip the sign bits then use a signed compare.
        auto flip = _mm_set1_epi8(char(0x80));
        return _mm_cmplt_epi8(_mm_xor_si128(flip, fVec), _mm_xor_si128(flip, o.fVec));
    }

    uint8_t operator[](int k) const {
        SkASSERT(0 <= k && k < 16);
        union { __m128i v; uint8_t us[16]; } pun = {fVec};
        return pun.us[k&15];
    }

    SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return _mm_or_si128(_mm_and_si128   (fVec, t.fVec),
                            _mm_andnot_si128(fVec, e.fVec));
    }

    __m128i fVec;
};

template<> /*static*/ inline Sk4f SkNx_cast<float, int>(const Sk4i& src) {
    return _mm_cvtepi32_ps(src.fVec);
}

template <> /*static*/ inline Sk4i SkNx_cast<int, float>(const Sk4f& src) {
    return _mm_cvttps_epi32(src.fVec);
}

template<> /*static*/ inline Sk4h SkNx_cast<uint16_t, float>(const Sk4f& src) {
    auto _32 = _mm_cvttps_epi32(src.fVec);
    // Ideally we'd use _mm_packus_epi32 here.  But that's SSE4.1+.
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
    // With SSSE3, we can just shuffle the low 2 bytes from each lane right into place.
    const int _ = ~0;
    return _mm_shuffle_epi8(_32, _mm_setr_epi8(0,1, 4,5, 8,9, 12,13, _,_,_,_,_,_,_,_));
#else
    // With SSE2, we have to emulate _mm_packus_epi32 with _mm_packs_epi32:
    _32 = _mm_sub_epi32(_32, _mm_set1_epi32((int)0x00008000));
    return _mm_add_epi16(_mm_packs_epi32(_32, _32), _mm_set1_epi16((short)0x8000));
#endif
}

template<> /*static*/ inline Sk4b SkNx_cast<uint8_t, float>(const Sk4f& src) {
    auto _32 = _mm_cvttps_epi32(src.fVec);
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
    const int _ = ~0;
    return _mm_shuffle_epi8(_32, _mm_setr_epi8(0,4,8,12, _,_,_,_, _,_,_,_, _,_,_,_));
#else
    auto _16 = _mm_packus_epi16(_32, _32);
    return     _mm_packus_epi16(_16, _16);
#endif
}

template<> /*static*/ inline Sk4f SkNx_cast<float, uint8_t>(const Sk4b& src) {
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
    const int _ = ~0;
    auto _32 = _mm_shuffle_epi8(src.fVec, _mm_setr_epi8(0,_,_,_, 1,_,_,_, 2,_,_,_, 3,_,_,_));
#else
    auto _16 = _mm_unpacklo_epi8(src.fVec, _mm_setzero_si128()),
         _32 = _mm_unpacklo_epi16(_16,     _mm_setzero_si128());
#endif
    return _mm_cvtepi32_ps(_32);
}

template<> /*static*/ inline Sk4f SkNx_cast<float, uint16_t>(const Sk4h& src) {
    auto _32 = _mm_unpacklo_epi16(src.fVec, _mm_setzero_si128());
    return _mm_cvtepi32_ps(_32);
}

template<> /*static*/ inline Sk16b SkNx_cast<uint8_t, float>(const Sk16f& src) {
    Sk8f ab, cd;
    SkNx_split(src, &ab, &cd);

    Sk4f a,b,c,d;
    SkNx_split(ab, &a, &b);
    SkNx_split(cd, &c, &d);

    return _mm_packus_epi16(_mm_packus_epi16(_mm_cvttps_epi32(a.fVec),
                                             _mm_cvttps_epi32(b.fVec)),
                            _mm_packus_epi16(_mm_cvttps_epi32(c.fVec),
                                             _mm_cvttps_epi32(d.fVec)));
}

template<> /*static*/ inline Sk4h SkNx_cast<uint16_t, uint8_t>(const Sk4b& src) {
    return _mm_unpacklo_epi8(src.fVec, _mm_setzero_si128());
}

template<> /*static*/ inline Sk4b SkNx_cast<uint8_t, uint16_t>(const Sk4h& src) {
    return _mm_packus_epi16(src.fVec, src.fVec);
}

#endif//SkNx_sse_DEFINED
