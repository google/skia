/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPx_sse_DEFINED
#define SkPx_sse_DEFINED

// SkPx_sse's sweet spot is to work with 4 pixels at a time,
// stored interlaced, just as they sit in memory: rgba rgba rgba rgba.

// SkPx_sse's best way to work with alphas is similar,
// replicating the 4 alphas 4 times each across the pixel: aaaa aaaa aaaa aaaa.

// When working with fewer than 4 pixels, we load the pixels in the low lanes,
// usually filling the top lanes with zeros (but who cares, might be junk).

struct SkPx_sse {
    static const int N = 4;

    __m128i fVec;
    SkPx_sse(__m128i vec) : fVec(vec) {}

    static SkPx_sse Dup(uint32_t px) { return _mm_set1_epi32(px); }
    static SkPx_sse LoadN(const uint32_t* px) { return _mm_loadu_si128((const __m128i*)px); }
    static SkPx_sse Load(int n, const uint32_t* px) {
        SkASSERT(n > 0 && n < 4);
        switch (n) {
            case 1: return _mm_cvtsi32_si128(px[0]);
            case 2: return _mm_loadl_epi64((const __m128i*)px);
            case 3: return _mm_or_si128(_mm_loadl_epi64((const __m128i*)px),
                                        _mm_slli_si128(_mm_cvtsi32_si128(px[2]), 8));
        }
        return _mm_setzero_si128();  // Not actually reachable.
    }

    void storeN(uint32_t* px) const { _mm_storeu_si128((__m128i*)px, fVec); }
    void store(int n, uint32_t* px) const {
        SkASSERT(n > 0 && n < 4);
        __m128i v = fVec;
        if (n & 1) {
            *px++ = _mm_cvtsi128_si32(v);
            v = _mm_srli_si128(v, 4);
        }
        if (n & 2) {
            _mm_storel_epi64((__m128i*)px, v);
        }
    }

    struct Alpha {
        __m128i fVec;
        Alpha(__m128i vec) : fVec(vec) {}

        static Alpha Dup(uint8_t a) { return _mm_set1_epi8(a); }
        static Alpha LoadN(const uint8_t* a) {
            __m128i as = _mm_cvtsi32_si128(*(const uint32_t*)a);    // ____ ____ ____ 3210
        #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
            return _mm_shuffle_epi8(as, _mm_set_epi8(3,3,3,3, 2,2,2,2, 1,1,1,1, 0,0,0,0));
        #else
            as   = _mm_unpacklo_epi8 (as, _mm_setzero_si128());     // ____ ____ _3_2 _1_0
            as   = _mm_unpacklo_epi16(as, _mm_setzero_si128());     // ___3 ___2 ___1 ___0
            as   = _mm_or_si128(as, _mm_slli_si128(as, 1));         // __33 __22 __11 __00
            return _mm_or_si128(as, _mm_slli_si128(as, 2));         // 3333 2222 1111 0000
        #endif
        }
        static Alpha Load(int n, const uint8_t* a) {
            SkASSERT(n > 0 && n < 4);
            uint8_t a4[] = { 0,0,0,0 };
            switch (n) {
                case 3: a4[2] = a[2];  // fall through
                case 2: a4[1] = a[1];  // fall through
                case 1: a4[0] = a[0];
            }
            return LoadN(a4);
        }

        Alpha inv() const { return _mm_sub_epi8(_mm_set1_epi8(~0), fVec); }
    };

    struct Wide {
        __m128i fLo, fHi;
        Wide(__m128i lo, __m128i hi) : fLo(lo), fHi(hi) {}

        Wide operator+(const Wide& o) const {
            return Wide(_mm_add_epi16(fLo, o.fLo), _mm_add_epi16(fHi, o.fHi));
        }
        Wide operator-(const Wide& o) const {
            return Wide(_mm_sub_epi16(fLo, o.fLo), _mm_sub_epi16(fHi, o.fHi));
        }
        Wide operator<<(int bits) const {
            return Wide(_mm_slli_epi16(fLo, bits), _mm_slli_epi16(fHi, bits));
        }
        Wide operator>>(int bits) const {
            return Wide(_mm_srli_epi16(fLo, bits), _mm_srli_epi16(fHi, bits));
        }

        SkPx_sse addNarrowHi(const SkPx_sse& o) const {
            Wide sum = (*this + o.widenLo()) >> 8;
            return _mm_packus_epi16(sum.fLo, sum.fHi);
        }
    };

    Alpha alpha() const {
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
        return _mm_shuffle_epi8(fVec, _mm_set_epi8(15,15,15,15, 11,11,11,11, 7,7,7,7, 3,3,3,3));
    #else
        __m128i as = _mm_srli_epi32(fVec, 24);           // ___3 ___2 ___1 ___0
        as   = _mm_or_si128(as, _mm_slli_si128(as, 1));  // __33 __22 __11 __00
        return _mm_or_si128(as, _mm_slli_si128(as, 2));  // 3333 2222 1111 0000
    #endif
    }

    Wide widenLo() const {
        return Wide(_mm_unpacklo_epi8(fVec, _mm_setzero_si128()),
                    _mm_unpackhi_epi8(fVec, _mm_setzero_si128()));
    }
    Wide widenHi() const {
        return Wide(_mm_unpacklo_epi8(_mm_setzero_si128(), fVec),
                    _mm_unpackhi_epi8(_mm_setzero_si128(), fVec));
    }
    Wide widenLoHi() const {
        return Wide(_mm_unpacklo_epi8(fVec, fVec),
                    _mm_unpackhi_epi8(fVec, fVec));
    }

    SkPx_sse    operator+(const SkPx_sse& o) const { return _mm_add_epi8(fVec, o.fVec); }
    SkPx_sse    operator-(const SkPx_sse& o) const { return _mm_sub_epi8(fVec, o.fVec); }
    SkPx_sse saturatedAdd(const SkPx_sse& o) const { return _mm_adds_epi8(fVec, o.fVec); }

    Wide operator*(const Alpha& a) const {
        __m128i pLo = _mm_unpacklo_epi8(  fVec, _mm_setzero_si128()),
                aLo = _mm_unpacklo_epi8(a.fVec, _mm_setzero_si128()),
                pHi = _mm_unpackhi_epi8(  fVec, _mm_setzero_si128()),
                aHi = _mm_unpackhi_epi8(a.fVec, _mm_setzero_si128());
        return Wide(_mm_mullo_epi16(pLo, aLo), _mm_mullo_epi16(pHi, aHi));
    }
    SkPx_sse approxMulDiv255(const Alpha& a) const {
        return (*this * a).addNarrowHi(*this);
    }

    SkPx_sse addAlpha(const Alpha& a) const {
        return _mm_add_epi8(fVec, _mm_and_si128(a.fVec, _mm_set1_epi32(0xFF000000)));
    }
};

typedef SkPx_sse SkPx;

#endif//SkPx_sse_DEFINED
