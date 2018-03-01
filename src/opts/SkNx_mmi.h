/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNx_mmi_DEFINED
#define SkNx_mmi_DEFINED

#include "loongson-mmintrin.h"

namespace {

template <>
class SkNx<4, int32_t> {
public:
    AI SkNx(const __m128i& vec) : fVec(vec) {}

    AI SkNx() {}
    AI SkNx(int32_t val) : fVec(_mm_set1_epi32(val)) {}
    AI static SkNx Load(const void* ptr) { return _mm_loadu_si128((const __m128i*)ptr); }
    AI SkNx(int32_t a, int32_t b, int32_t c, int32_t d) : fVec(_mm_setr_epi32(a,b,c,d)) {}

    AI void store(void* ptr) const { _mm_storeu_si128((__m128i*)ptr, fVec); }

    AI SkNx operator + (const SkNx& o) const { return _mm_add_epi32(fVec, o.fVec); }
    AI SkNx operator - (const SkNx& o) const { return _mm_sub_epi32(fVec, o.fVec); }
    AI SkNx operator * (const SkNx& o) const {
        __m128i mul20 = _mm_mul_epu32(fVec, o.fVec),
                mul31 = _mm_mul_epu32(_mm_srli_si128(fVec, 4), _mm_srli_si128(o.fVec, 4));
        return _mm_unpacklo_epi32(_mm_shuffle_epi32_0020(mul20),
                                  _mm_shuffle_epi32_0020(mul31));
    }

    AI SkNx operator & (const SkNx& o) const { return _mm_and_si128(fVec, o.fVec); }
    AI SkNx operator | (const SkNx& o) const { return _mm_or_si128(fVec, o.fVec);  }
    AI SkNx operator ^ (const SkNx& o) const { return _mm_xor_si128(fVec, o.fVec); }

    AI SkNx operator << (int bits) const { return _mm_slli_epi32(fVec, bits); }
    AI SkNx operator >> (int bits) const { return _mm_srai_epi32(fVec, bits); }

    AI SkNx operator == (const SkNx& o) const { return _mm_cmpeq_epi32 (fVec, o.fVec); }
    AI SkNx operator  < (const SkNx& o) const { return _mm_cmplt_epi32 (fVec, o.fVec); }
    AI SkNx operator  > (const SkNx& o) const { return _mm_cmpgt_epi32 (fVec, o.fVec); }

    AI int32_t operator[](int k) const {
        SkASSERT(0 <= k && k < 4);
        union { __m128i v; int32_t is[4]; } pun = {fVec};
        return pun.is[k&3];
    }

    AI SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return _mm_or_si128(_mm_and_si128   (fVec, t.fVec),
                            _mm_andnot_si128(fVec, e.fVec));
    }

    AI SkNx abs() const {
        SkNx mask = (*this) >> 31;
        return (mask ^ (*this)) - mask;
    }

    AI static SkNx Min(const SkNx& x, const SkNx& y) {
        return (x < y).thenElse(x, y);
    }

    AI static SkNx Max(const SkNx& x, const SkNx& y) {
        return (x > y).thenElse(x, y);
    }

    __m128i fVec;
};

template <>
class SkNx<4, uint32_t> {
public:
    AI SkNx(const __m128i& vec) : fVec(vec) {}

    AI SkNx() {}
    AI SkNx(uint32_t val) : fVec(_mm_set1_epi32(val)) {}
    AI static SkNx Load(const void* ptr) { return _mm_loadu_si128((const __m128i*)ptr); }
    AI SkNx(uint32_t a, uint32_t b, uint32_t c, uint32_t d) : fVec(_mm_setr_epi32(a,b,c,d)) {}

    AI void store(void* ptr) const { _mm_storeu_si128((__m128i*)ptr, fVec); }

    AI SkNx operator + (const SkNx& o) const { return _mm_add_epi32(fVec, o.fVec); }
    AI SkNx operator - (const SkNx& o) const { return _mm_sub_epi32(fVec, o.fVec); }
    AI SkNx operator * (const SkNx& o) const {
        __m128i mul20 = _mm_mul_epu32(fVec, o.fVec),
                mul31 = _mm_mul_epu32(_mm_srli_si128(fVec, 4), _mm_srli_si128(o.fVec, 4));
        return _mm_unpacklo_epi32(_mm_shuffle_epi32_0020(mul20),
                                  _mm_shuffle_epi32_0020(mul31));
    }

    AI SkNx operator & (const SkNx& o) const { return _mm_and_si128(fVec, o.fVec); }
    AI SkNx operator | (const SkNx& o) const { return _mm_or_si128(fVec, o.fVec);  }
    AI SkNx operator ^ (const SkNx& o) const { return _mm_xor_si128(fVec, o.fVec); }

    AI SkNx operator << (int bits) const { return _mm_slli_epi32(fVec, bits); }
    AI SkNx operator >> (int bits) const { return _mm_srli_epi32(fVec, bits); }

    AI SkNx operator == (const SkNx& o) const { return _mm_cmpeq_epi32 (fVec, o.fVec); }
    // operator < and > take a little extra fiddling to make work for unsigned ints.

    AI uint32_t operator[](int k) const {
        SkASSERT(0 <= k && k < 4);
        union { __m128i v; uint32_t us[4]; } pun = {fVec};
        return pun.us[k&3];
    }

    AI SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return _mm_or_si128(_mm_and_si128   (fVec, t.fVec),
                            _mm_andnot_si128(fVec, e.fVec));
    }

    AI SkNx mulHi(SkNx m) const {
        SkNx v20{_mm_mul_epu32(m.fVec, fVec)};
        SkNx v31{_mm_mul_epu32(_mm_srli_si128(m.fVec, 4), _mm_srli_si128(fVec, 4))};

        return SkNx{v20[1], v31[1], v20[3], v31[3]};
    }

    __m128i fVec;
};

template <>
class SkNx<4, uint16_t> {
public:
    AI SkNx(const __m128i& vec) : fVec(vec) {}

    AI SkNx() {}
    AI SkNx(uint16_t val) : fVec(_mm_set1_epi16(val)) {}
    AI SkNx(uint16_t a, uint16_t b, uint16_t c, uint16_t d)
        : fVec(_mm_setr_epi16(a,b,c,d,0,0,0,0)) {}

    AI static SkNx Load(const void* ptr) { return _mm_loadl_epi64((const __m128i*)ptr); }
    AI void store(void* ptr) const { _mm_storel_epi64((__m128i*)ptr, fVec); }

    AI static void Load4(const void* ptr, SkNx* r, SkNx* g, SkNx* b, SkNx* a) {
        __m128i lo = _mm_loadu_si128(((__m128i*)ptr) + 0),
                hi = _mm_loadu_si128(((__m128i*)ptr) + 1);
        __m128i even = _mm_unpacklo_epi16(lo, hi),   // r0 r2 g0 g2 b0 b2 a0 a2
                 odd = _mm_unpackhi_epi16(lo, hi);   // r1 r3 ...
        __m128i rg = _mm_unpacklo_epi16(even, odd),  // r0 r1 r2 r3 g0 g1 g2 g3
                ba = _mm_unpackhi_epi16(even, odd);  // b0 b1 ...   a0 a1 ...
        *r = rg;
        *g = _mm_srli_si128(rg, 8);
        *b = ba;
        *a = _mm_srli_si128(ba, 8);
    }
    AI static void Load3(const void* ptr, SkNx* r, SkNx* g, SkNx* b) {
        // The idea here is to get 4 vectors that are R G B _ _ _ _ _.
        // The second load is at a funny location to make sure we don't read past
        // the bounds of memory.  This is fine, we just need to shift it a little bit.
        const uint8_t* ptr8 = (const uint8_t*) ptr;
        __m128i rgb0 = _mm_loadu_si128((const __m128i*) (ptr8 + 0));
        __m128i rgb1 = _mm_srli_si128(rgb0, 3*2);
        __m128i rgb2 = _mm_srli_si128(_mm_loadu_si128((const __m128i*) (ptr8 + 4*2)), 2*2);
        __m128i rgb3 = _mm_srli_si128(rgb2, 3*2);

        __m128i rrggbb01 = _mm_unpacklo_epi16(rgb0, rgb1);
        __m128i rrggbb23 = _mm_unpacklo_epi16(rgb2, rgb3);
        *r = _mm_unpacklo_epi32(rrggbb01, rrggbb23);
        *g = _mm_srli_si128(r->fVec, 4*2);
        *b = _mm_unpackhi_epi32(rrggbb01, rrggbb23);
    }
    AI static void Store4(void* dst, const SkNx& r, const SkNx& g, const SkNx& b, const SkNx& a) {
        __m128i rg = _mm_unpacklo_epi16(r.fVec, g.fVec);
        __m128i ba = _mm_unpacklo_epi16(b.fVec, a.fVec);
        __m128i lo = _mm_unpacklo_epi32(rg, ba);
        __m128i hi = _mm_unpackhi_epi32(rg, ba);
        _mm_storeu_si128(((__m128i*) dst) + 0, lo);
        _mm_storeu_si128(((__m128i*) dst) + 1, hi);
    }

    AI SkNx operator + (const SkNx& o) const { return _mm_add_epi16(fVec, o.fVec); }
    AI SkNx operator - (const SkNx& o) const { return _mm_sub_epi16(fVec, o.fVec); }
    AI SkNx operator * (const SkNx& o) const { return _mm_mullo_epi16(fVec, o.fVec); }
    AI SkNx operator & (const SkNx& o) const { return _mm_and_si128(fVec, o.fVec); }
    AI SkNx operator | (const SkNx& o) const { return _mm_or_si128(fVec, o.fVec); }

    AI SkNx operator << (int bits) const { return _mm_slli_epi16(fVec, bits); }
    AI SkNx operator >> (int bits) const { return _mm_srli_epi16(fVec, bits); }

    AI uint16_t operator[](int k) const {
        SkASSERT(0 <= k && k < 4);
        union { __m128i v; uint16_t us[8]; } pun = {fVec};
        return pun.us[k&3];
    }

    __m128i fVec;
};

template <>
class SkNx<8, uint16_t> {
public:
    AI SkNx(const __m128i& vec) : fVec(vec) {}

    AI SkNx() {}
    AI SkNx(uint16_t val) : fVec(_mm_set1_epi16(val)) {}
    AI SkNx(uint16_t a, uint16_t b, uint16_t c, uint16_t d,
            uint16_t e, uint16_t f, uint16_t g, uint16_t h)
        : fVec(_mm_setr_epi16(a,b,c,d,e,f,g,h)) {}

    AI static SkNx Load(const void* ptr) { return _mm_loadu_si128((const __m128i*)ptr); }
    AI void store(void* ptr) const { _mm_storeu_si128((__m128i*)ptr, fVec); }

    AI static void Load4(const void* ptr, SkNx* r, SkNx* g, SkNx* b, SkNx* a) {
        __m128i _01 = _mm_loadu_si128(((__m128i*)ptr) + 0),
                _23 = _mm_loadu_si128(((__m128i*)ptr) + 1),
                _45 = _mm_loadu_si128(((__m128i*)ptr) + 2),
                _67 = _mm_loadu_si128(((__m128i*)ptr) + 3);

        __m128i _02 = _mm_unpacklo_epi16(_01, _23),  // r0 r2 g0 g2 b0 b2 a0 a2
                _13 = _mm_unpackhi_epi16(_01, _23),  // r1 r3 g1 g3 b1 b3 a1 a3
                _46 = _mm_unpacklo_epi16(_45, _67),
                _57 = _mm_unpackhi_epi16(_45, _67);

        __m128i rg0123 = _mm_unpacklo_epi16(_02, _13),  // r0 r1 r2 r3 g0 g1 g2 g3
                ba0123 = _mm_unpackhi_epi16(_02, _13),  // b0 b1 b2 b3 a0 a1 a2 a3
                rg4567 = _mm_unpacklo_epi16(_46, _57),
                ba4567 = _mm_unpackhi_epi16(_46, _57);

        *r = _mm_unpacklo_epi64(rg0123, rg4567);
        *g = _mm_unpackhi_epi64(rg0123, rg4567);
        *b = _mm_unpacklo_epi64(ba0123, ba4567);
        *a = _mm_unpackhi_epi64(ba0123, ba4567);
    }
    AI static void Load3(const void* ptr, SkNx* r, SkNx* g, SkNx* b) {
        const uint8_t* ptr8 = (const uint8_t*) ptr;
        __m128i rgb0 = _mm_loadu_si128((const __m128i*) (ptr8 +  0*2));
        __m128i rgb1 = _mm_srli_si128(rgb0, 3*2);
        __m128i rgb2 = _mm_loadu_si128((const __m128i*) (ptr8 +  6*2));
        __m128i rgb3 = _mm_srli_si128(rgb2, 3*2);
        __m128i rgb4 = _mm_loadu_si128((const __m128i*) (ptr8 + 12*2));
        __m128i rgb5 = _mm_srli_si128(rgb4, 3*2);
        __m128i rgb6 = _mm_srli_si128(_mm_loadu_si128((const __m128i*) (ptr8 + 16*2)), 2*2);
        __m128i rgb7 = _mm_srli_si128(rgb6, 3*2);

        __m128i rgb01 = _mm_unpacklo_epi16(rgb0, rgb1);
        __m128i rgb23 = _mm_unpacklo_epi16(rgb2, rgb3);
        __m128i rgb45 = _mm_unpacklo_epi16(rgb4, rgb5);
        __m128i rgb67 = _mm_unpacklo_epi16(rgb6, rgb7);

        __m128i rg03 = _mm_unpacklo_epi32(rgb01, rgb23);
        __m128i bx03 = _mm_unpackhi_epi32(rgb01, rgb23);
        __m128i rg47 = _mm_unpacklo_epi32(rgb45, rgb67);
        __m128i bx47 = _mm_unpackhi_epi32(rgb45, rgb67);

        *r = _mm_unpacklo_epi64(rg03, rg47);
        *g = _mm_unpackhi_epi64(rg03, rg47);
        *b = _mm_unpacklo_epi64(bx03, bx47);
    }
    AI static void Store4(void* ptr, const SkNx& r, const SkNx& g, const SkNx& b, const SkNx& a) {
        __m128i rg0123 = _mm_unpacklo_epi16(r.fVec, g.fVec),  // r0 g0 r1 g1 r2 g2 r3 g3
                rg4567 = _mm_unpackhi_epi16(r.fVec, g.fVec),  // r4 g4 r5 g5 r6 g6 r7 g7
                ba0123 = _mm_unpacklo_epi16(b.fVec, a.fVec),
                ba4567 = _mm_unpackhi_epi16(b.fVec, a.fVec);

        _mm_storeu_si128((__m128i*)ptr + 0, _mm_unpacklo_epi32(rg0123, ba0123));
        _mm_storeu_si128((__m128i*)ptr + 1, _mm_unpackhi_epi32(rg0123, ba0123));
        _mm_storeu_si128((__m128i*)ptr + 2, _mm_unpacklo_epi32(rg4567, ba4567));
        _mm_storeu_si128((__m128i*)ptr + 3, _mm_unpackhi_epi32(rg4567, ba4567));
    }

    AI SkNx operator + (const SkNx& o) const { return _mm_add_epi16(fVec, o.fVec); }
    AI SkNx operator - (const SkNx& o) const { return _mm_sub_epi16(fVec, o.fVec); }
    AI SkNx operator * (const SkNx& o) const { return _mm_mullo_epi16(fVec, o.fVec); }
    AI SkNx operator & (const SkNx& o) const { return _mm_and_si128(fVec, o.fVec); }
    AI SkNx operator | (const SkNx& o) const { return _mm_or_si128(fVec, o.fVec); }

    AI SkNx operator << (int bits) const { return _mm_slli_epi16(fVec, bits); }
    AI SkNx operator >> (int bits) const { return _mm_srli_epi16(fVec, bits); }

    AI static SkNx Min(const SkNx& a, const SkNx& b) {
        // No unsigned _mm_min_epu16, so we'll shift into a space where we can use the
        // signed version, _mm_min_epi16, then shift back.
        const uint16_t top = 0x8000; // Keep this separate from _mm_set1_epi16 or MSVC will whine.
        const __m128i top_8x = _mm_set1_epi16(top);
        return _mm_add_epi8(top_8x, _mm_min_epi16(_mm_sub_epi8(a.fVec, top_8x),
                                                  _mm_sub_epi8(b.fVec, top_8x)));
    }

    AI SkNx mulHi(const SkNx& m) const {
        return _mm_mulhi_epu16(fVec, m.fVec);
    }

    AI SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return _mm_or_si128(_mm_and_si128   (fVec, t.fVec),
                            _mm_andnot_si128(fVec, e.fVec));
    }

    AI uint16_t operator[](int k) const {
        SkASSERT(0 <= k && k < 8);
        union { __m128i v; uint16_t us[8]; } pun = {fVec};
        return pun.us[k&7];
    }

    __m128i fVec;
};

template <>
class SkNx<4, uint8_t> {
public:
    AI SkNx() {}
    AI SkNx(const __m128i& vec) : fVec(vec) {}
    AI SkNx(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
        : fVec(_mm_setr_epi8(a,b,c,d, 0,0,0,0, 0,0,0,0, 0,0,0,0)) {}

    AI static SkNx Load(const void* ptr) { return _mm_cvtsi32_si128(*(const int*)ptr); }
    AI void store(void* ptr) const { *(int*)ptr = _mm_cvtsi128_si32(fVec); }

    AI uint8_t operator[](int k) const {
        SkASSERT(0 <= k && k < 4);
        union { __m128i v; uint8_t us[16]; } pun = {fVec};
        return pun.us[k&3];
    }

    // TODO as needed

    __m128i fVec;
};

template <>
class SkNx<8, uint8_t> {
public:
    AI SkNx(const __m128i& vec) : fVec(vec) {}

    AI SkNx() {}
    AI SkNx(uint8_t val) : fVec(_mm_set1_epi8(val)) {}
    AI static SkNx Load(const void* ptr) { return _mm_loadl_epi64((const __m128i*)ptr); }
    AI SkNx(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
            uint8_t e, uint8_t f, uint8_t g, uint8_t h)
            : fVec(_mm_setr_epi8(a,b,c,d, e,f,g,h, 0,0,0,0, 0,0,0,0)) {}

    AI void store(void* ptr) const {_mm_storel_epi64((__m128i*)ptr, fVec);}

    AI SkNx saturatedAdd(const SkNx& o) const { return _mm_adds_epu8(fVec, o.fVec); }

    AI SkNx operator + (const SkNx& o) const { return _mm_add_epi8(fVec, o.fVec); }
    AI SkNx operator - (const SkNx& o) const { return _mm_sub_epi8(fVec, o.fVec); }

    AI static SkNx Min(const SkNx& a, const SkNx& b) { return _mm_min_epu8(a.fVec, b.fVec); }
    AI SkNx operator < (const SkNx& o) const {
        // There's no unsigned _mm_cmplt_epu8, so we flip the sign bits then use a signed compare.
        auto flip = _mm_set1_epi8(char(0x80));
        return _mm_cmplt_epi8(_mm_xor_si128(flip, fVec), _mm_xor_si128(flip, o.fVec));
    }

    AI uint8_t operator[](int k) const {
        SkASSERT(0 <= k && k < 16);
        union { __m128i v; uint8_t us[16]; } pun = {fVec};
        return pun.us[k&15];
    }

    AI SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return _mm_or_si128(_mm_and_si128   (fVec, t.fVec),
                            _mm_andnot_si128(fVec, e.fVec));
    }

    __m128i fVec;
};

template <>
class SkNx<16, uint8_t> {
public:
    AI SkNx(const __m128i& vec) : fVec(vec) {}

    AI SkNx() {}
    AI SkNx(uint8_t val) : fVec(_mm_set1_epi8(val)) {}
    AI static SkNx Load(const void* ptr) { return _mm_loadu_si128((const __m128i*)ptr); }
    AI SkNx(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
            uint8_t e, uint8_t f, uint8_t g, uint8_t h,
            uint8_t i, uint8_t j, uint8_t k, uint8_t l,
            uint8_t m, uint8_t n, uint8_t o, uint8_t p)
        : fVec(_mm_setr_epi8(a,b,c,d, e,f,g,h, i,j,k,l, m,n,o,p)) {}

    AI void store(void* ptr) const { _mm_storeu_si128((__m128i*)ptr, fVec); }

    AI SkNx saturatedAdd(const SkNx& o) const { return _mm_adds_epu8(fVec, o.fVec); }

    AI SkNx operator + (const SkNx& o) const { return _mm_add_epi8(fVec, o.fVec); }
    AI SkNx operator - (const SkNx& o) const { return _mm_sub_epi8(fVec, o.fVec); }

    AI static SkNx Min(const SkNx& a, const SkNx& b) { return _mm_min_epu8(a.fVec, b.fVec); }
    AI SkNx operator < (const SkNx& o) const {
        // There's no unsigned _mm_cmplt_epu8, so we flip the sign bits then use a signed compare.
        auto flip = _mm_set1_epi8(char(0x80));
        return _mm_cmplt_epi8(_mm_xor_si128(flip, fVec), _mm_xor_si128(flip, o.fVec));
    }

    AI uint8_t operator[](int k) const {
        SkASSERT(0 <= k && k < 16);
        union { __m128i v; uint8_t us[16]; } pun = {fVec};
        return pun.us[k&15];
    }

    AI SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return _mm_or_si128(_mm_and_si128   (fVec, t.fVec),
                            _mm_andnot_si128(fVec, e.fVec));
    }

    __m128i fVec;
};

template<> AI /*static*/ Sk4f SkNx_cast<float, int32_t>(const Sk4i& src) {
    float a, b, c, d;
    a = (float)src[0];
    b = (float)src[1];
    c = (float)src[2];
    d = (float)src[3];
    return Sk4f(a, b, c, d);
}

template<> AI /*static*/ Sk4f SkNx_cast<float, uint32_t>(const Sk4u& src) {
    return SkNx_cast<float>(Sk4i::Load(&src));
}

template <> AI /*static*/ Sk4i SkNx_cast<int32_t, float>(const Sk4f& src) {
    int32_t a, b, c, d;
    a = (int32_t)src[0];
    b = (int32_t)src[1];
    c = (int32_t)src[2];
    d = (int32_t)src[3];
    return Sk4i(a, b, c, d);
}

template<> AI /*static*/ Sk4h SkNx_cast<uint16_t, int32_t>(const Sk4i& src) {
    // With SSE2, we have to sign extend our input, making _mm_packs_epi32 do the pack we want.
    __m128i x = _mm_srai_epi32(_mm_slli_epi32(src.fVec, 16), 16);
    return _mm_packs_epi32(x,x);
}

template<> AI /*static*/ Sk4h SkNx_cast<uint16_t, float>(const Sk4f& src) {
    return SkNx_cast<uint16_t>(SkNx_cast<int32_t>(src));
}

template<> AI /*static*/ Sk4b SkNx_cast<uint8_t, float>(const Sk4f& src) {
    int32_t a, b, c, d;
    a = (int32_t)src[0];
    b = (int32_t)src[1];
    c = (int32_t)src[2];
    d = (int32_t)src[3];
    auto _32 = _mm_setr_epi32(a, b, c, d);
    auto _16 = _mm_packus_epi16(_32, _32);
    return     _mm_packus_epi16(_16, _16);
}

template<> AI /*static*/ Sk4u SkNx_cast<uint32_t, uint8_t>(const Sk4b& src) {
    auto _16 = _mm_unpacklo_epi8(src.fVec, _mm_setzero_si128());
    return _mm_unpacklo_epi16(_16, _mm_setzero_si128());
}

template<> AI /*static*/ Sk4i SkNx_cast<int32_t, uint8_t>(const Sk4b& src) {
    return SkNx_cast<uint32_t>(src).fVec;
}

template<> AI /*static*/ Sk4f SkNx_cast<float, uint8_t>(const Sk4b& src) {
    auto _16 = _mm_unpacklo_epi8(src.fVec, _mm_setzero_si128()),
         _32 = _mm_unpacklo_epi16(_16,     _mm_setzero_si128());
    Sk4u _u = Sk4u(_32);
    float a, b, c, d;
    a = (float)_u[0];
    b = (float)_u[1];
    c = (float)_u[2];
    d = (float)_u[3];
    return Sk4f(a, b, c, d);
}

template<> AI /*static*/ Sk4f SkNx_cast<float, uint16_t>(const Sk4h& src) {
    auto _32 = _mm_unpacklo_epi16(src.fVec, _mm_setzero_si128());
    Sk4u _u = Sk4u(_32);
    float a, b, c, d;
    a = (float)_u[0];
    b = (float)_u[1];
    c = (float)_u[2];
    d = (float)_u[3];
    return Sk4f(a, b, c, d);
}

template<> AI /*static*/ Sk8b SkNx_cast<uint8_t, int32_t>(const Sk8i& src) {
    Sk4i lo, hi;
    SkNx_split(src, &lo, &hi);

    auto t = _mm_packs_epi32(lo.fVec, hi.fVec);
    return _mm_packus_epi16(t, t);
}

template<> AI /*static*/ Sk16b SkNx_cast<uint8_t, float>(const Sk16f& src) {
    Sk8f ab, cd;
    SkNx_split(src, &ab, &cd);

    Sk4f a,b,c,d;
    SkNx_split(ab, &a, &b);
    SkNx_split(cd, &c, &d);

    int32_t A, B, C, D;
    A = (int32_t)a[0];
    B = (int32_t)a[1];
    C = (int32_t)a[2];
    D = (int32_t)a[3];

    int32_t E, F, G, H;
    E = (int32_t)b[0];
    F = (int32_t)b[1];
    G = (int32_t)b[2];
    H = (int32_t)b[3];

    int32_t I, J, K, L;
    I = (int32_t)c[0];
    J = (int32_t)c[1];
    K = (int32_t)c[2];
    L = (int32_t)c[3];

    int32_t M, N, O, P;
    M = (int32_t)d[0];
    N = (int32_t)d[1];
    O = (int32_t)d[2];
    P = (int32_t)d[3];

    return _mm_packus_epi16(_mm_packus_epi16(_mm_setr_epi32(A, B, C, D),
                                             _mm_setr_epi32(E, F, G, H)),
                            _mm_packus_epi16(_mm_setr_epi32(I, J, K, L),
                                             _mm_setr_epi32(M, N, O, P)));
}

template<> AI /*static*/ Sk4h SkNx_cast<uint16_t, uint8_t>(const Sk4b& src) {
    return _mm_unpacklo_epi8(src.fVec, _mm_setzero_si128());
}

template<> AI /*static*/ Sk8h SkNx_cast<uint16_t, uint8_t>(const Sk8b& src) {
    return _mm_unpacklo_epi8(src.fVec, _mm_setzero_si128());
}

template<> AI /*static*/ Sk4b SkNx_cast<uint8_t, uint16_t>(const Sk4h& src) {
    return _mm_packus_epi16(src.fVec, src.fVec);
}

template<> AI /*static*/ Sk8b SkNx_cast<uint8_t, uint16_t>(const Sk8h& src) {
    return _mm_packus_epi16(src.fVec, src.fVec);
}

template<> AI /*static*/ Sk4i SkNx_cast<int32_t, uint16_t>(const Sk4h& src) {
    return _mm_unpacklo_epi16(src.fVec, _mm_setzero_si128());
}

template<> AI /*static*/ Sk4b SkNx_cast<uint8_t, int32_t>(const Sk4i& src) {
    return _mm_packus_epi16(_mm_packus_epi16(src.fVec, src.fVec), src.fVec);
}

template<> AI /*static*/ Sk4b SkNx_cast<uint8_t, uint32_t>(const Sk4u& src) {
    return _mm_packus_epi16(_mm_packus_epi16(src.fVec, src.fVec), src.fVec);
}

template<> AI /*static*/ Sk4i SkNx_cast<int32_t, uint32_t>(const Sk4u& src) {
    return src.fVec;
}

AI static Sk4i Sk4f_round(const Sk4f& x) {
    return { (int) lrintf (x[0]),
             (int) lrintf (x[1]),
             (int) lrintf (x[2]),
             (int) lrintf (x[3]), };
}

AI static  void Sk4h_load4(const void* ptr, Sk4h* r, Sk4h* g, Sk4h* b, Sk4h* a) {
    __m128i lo = _mm_loadu_si128(((__m128i*)ptr) + 0),
            hi = _mm_loadu_si128(((__m128i*)ptr) + 1);
    __m128i even = _mm_unpacklo_epi16(lo, hi),   // r0 r2 g0 g2 b0 b2 a0 a2
             odd = _mm_unpackhi_epi16(lo, hi);   // r1 r3 ...
    __m128i rg = _mm_unpacklo_epi16(even, odd),  // r0 r1 r2 r3 g0 g1 g2 g3
            ba = _mm_unpackhi_epi16(even, odd);  // b0 b1 ...   a0 a1 ...
    *r = rg;
    *g = _mm_srli_si128(rg, 8);
    *b = ba;
    *a = _mm_srli_si128(ba, 8);
}

AI static void Sk4h_store4(void* dst, const Sk4h& r, const Sk4h& g, const Sk4h& b,
                               const Sk4h& a) {
    __m128i rg = _mm_unpacklo_epi16(r.fVec, g.fVec);
    __m128i ba = _mm_unpacklo_epi16(b.fVec, a.fVec);
    __m128i lo = _mm_unpacklo_epi32(rg, ba);
    __m128i hi = _mm_unpackhi_epi32(rg, ba);
    _mm_storeu_si128(((__m128i*) dst) + 0, lo);
    _mm_storeu_si128(((__m128i*) dst) + 1, hi);
}

// Load 4 Sk4f and transpose them (512 bits total).
AI static void Sk4f_load4(const void* vptr, Sk4f* r, Sk4f* g, Sk4f* b, Sk4f* a) {
    const float* ptr = (const float*) vptr;
    auto p0 = Sk4f::Load(ptr +  0),
         p1 = Sk4f::Load(ptr +  4),
         p2 = Sk4f::Load(ptr +  8),
         p3 = Sk4f::Load(ptr + 12);
    *r = { p0[0], p1[0], p2[0], p3[0] };
    *g = { p0[1], p1[1], p2[1], p3[1] };
    *b = { p0[2], p1[2], p2[2], p3[2] };
    *a = { p0[3], p1[3], p2[3], p3[3] };
}

// Transpose 4 Sk4f and store (512 bits total).
AI static void Sk4f_store4(void* vdst, const Sk4f& r, const Sk4f& g, const Sk4f& b, const Sk4f& a) {
    float* dst = (float*) vdst;
    Sk4f(r[0], g[0], b[0], a[0]).store(dst +  0);
    Sk4f(r[1], g[1], b[1], a[1]).store(dst +  4);
    Sk4f(r[2], g[2], b[2], a[2]).store(dst +  8);
    Sk4f(r[3], g[3], b[3], a[3]).store(dst + 12);
}

}  // namespace
#endif//SkNx_mmi_DEFINED
