/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPx_sse_DEFINED
#define SkPx_sse_DEFINED

// sse::SkPx's sweet spot is to work with 4 pixels at a time,
// stored interlaced, just as they sit in memory: rgba rgba rgba rgba.

// sse::SkPx's best way to work with alphas is similar,
// replicating the 4 alphas 4 times each across the pixel: aaaa aaaa aaaa aaaa.

// When working with fewer than 4 pixels, we load the pixels in the low lanes,
// usually filling the top lanes with zeros (but who cares, might be junk).

namespace sse {

struct SkPx {
    static const int N = 4;

    __m128i fVec;
    SkPx(__m128i vec) : fVec(vec) {}

    static SkPx Dup(uint32_t px) { return _mm_set1_epi32(px); }
    static SkPx Load(const uint32_t* px) { return _mm_loadu_si128((const __m128i*)px); }
    static SkPx Load(const uint32_t* px, int n) {
        SkASSERT(n > 0 && n < 4);
        switch (n) {
            case 1: return _mm_cvtsi32_si128(px[0]);
            case 2: return _mm_loadl_epi64((const __m128i*)px);
            case 3: return _mm_or_si128(_mm_loadl_epi64((const __m128i*)px),
                                        _mm_slli_si128(_mm_cvtsi32_si128(px[2]), 8));
        }
        return _mm_setzero_si128();  // Not actually reachable.
    }

    void store(uint32_t* px) const { _mm_storeu_si128((__m128i*)px, fVec); }
    void store(uint32_t* px, int n) const {
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
        static Alpha Load(const uint8_t* a) {
            __m128i as = _mm_cvtsi32_si128(*(const uint32_t*)a);    // ____ ____ ____ 3210
        #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
            return _mm_shuffle_epi8(as, _mm_set_epi8(3,3,3,3, 2,2,2,2, 1,1,1,1, 0,0,0,0));
        #else
            as = _mm_unpacklo_epi8 (as, as);                        // ____ ____ 3322 1100
            as = _mm_unpacklo_epi16(as, as);                        // 3333 2222 1111 0000
            return as;
        #endif
        }
        static Alpha Load(const uint8_t* a, int n) {
            SkASSERT(n > 0 && n < 4);
            uint8_t a4[] = { 0,0,0,0 };
            switch (n) {
                case 3: a4[2] = a[2];  // fall through
                case 2: a4[1] = a[1];  // fall through
                case 1: a4[0] = a[0];
            }
            return Load(a4);
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
        template <int bits> Wide shl() const {
            return Wide(_mm_slli_epi16(fLo, bits), _mm_slli_epi16(fHi, bits));
        }
        template <int bits> Wide shr() const {
            return Wide(_mm_srli_epi16(fLo, bits), _mm_srli_epi16(fHi, bits));
        }

        SkPx addNarrowHi(const SkPx& o) const {
            Wide sum = (*this + o.widenLo()).shr<8>();
            return _mm_packus_epi16(sum.fLo, sum.fHi);
        }
    };

    Alpha alpha() const {
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
        return _mm_shuffle_epi8(fVec, _mm_set_epi8(15,15,15,15, 11,11,11,11, 7,7,7,7, 3,3,3,3));
    #else
        // We exploit that A >= rgb for any premul pixel.
        __m128i as = fVec;                             // 3xxx 2xxx 1xxx 0xxx
        as = _mm_max_epu8(as, _mm_srli_epi32(as,  8)); // 33xx 22xx 11xx 00xx
        as = _mm_max_epu8(as, _mm_srli_epi32(as, 16)); // 3333 2222 1111 0000
        return as;
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

    SkPx    operator+(const SkPx& o) const { return _mm_add_epi8(fVec, o.fVec); }
    SkPx    operator-(const SkPx& o) const { return _mm_sub_epi8(fVec, o.fVec); }
    SkPx saturatedAdd(const SkPx& o) const { return _mm_adds_epi8(fVec, o.fVec); }

    Wide operator*(const Alpha& a) const {
        __m128i pLo = _mm_unpacklo_epi8(  fVec, _mm_setzero_si128()),
                aLo = _mm_unpacklo_epi8(a.fVec, _mm_setzero_si128()),
                pHi = _mm_unpackhi_epi8(  fVec, _mm_setzero_si128()),
                aHi = _mm_unpackhi_epi8(a.fVec, _mm_setzero_si128());
        return Wide(_mm_mullo_epi16(pLo, aLo), _mm_mullo_epi16(pHi, aHi));
    }
    SkPx approxMulDiv255(const Alpha& a) const {
        return (*this * a).addNarrowHi(*this);
    }

    SkPx addAlpha(const Alpha& a) const {
        return _mm_add_epi8(fVec, _mm_and_si128(a.fVec, _mm_set1_epi32(0xFF000000)));
    }
};

}  // namespace sse

typedef sse::SkPx SkPx;

#endif//SkPx_sse_DEFINED
