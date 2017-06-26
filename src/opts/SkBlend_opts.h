/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
ninja -C out/Release dm nanobench ; and ./out/Release/dm --match Blend_opts ; and ./out/Release/nanobench  --samples 300 --nompd --match LinearSrcOver -q
 */

#ifndef SkBlend_opts_DEFINED
#define SkBlend_opts_DEFINED

#include "SkNx.h"
#include "SkPM4fPriv.h"

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
    #include <immintrin.h>
#endif

namespace SK_OPTS_NS {

static inline void srcover_srgb_srgb_1(uint32_t* dst, uint32_t src) {
    if (src >= 0xFF000000) {
        *dst = src;
        return;
    }
    auto d = Sk4f_fromS32(*dst),
         s = Sk4f_fromS32( src);
    *dst = Sk4f_toS32(s + d * (1.0f - s[3]));
}

static inline void srcover_srgb_srgb_4(uint32_t* dst, const uint32_t* src) {
    srcover_srgb_srgb_1(dst++, *src++);
    srcover_srgb_srgb_1(dst++, *src++);
    srcover_srgb_srgb_1(dst++, *src++);
    srcover_srgb_srgb_1(dst  , *src  );
}

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41

    static inline __m128i load(const uint32_t* p) {
        return _mm_loadu_si128(reinterpret_cast<const __m128i*>(p));
    }

    static inline void store(uint32_t* p, __m128i v) {
        _mm_storeu_si128(reinterpret_cast<__m128i*>(p), v);
    }

    static void srcover_srgb_srgb(
            uint32_t* dst, const uint32_t* const srcStart, int ndst, const int nsrc) {
        const __m128i alphaMask = _mm_set1_epi32(0xFF000000);
        while (ndst > 0) {
            int count = SkTMin(ndst, nsrc);
            ndst -= count;
            const uint32_t* src = srcStart;
            const uint32_t* end = dst + (count & ~3);

            while (dst < end) {
                __m128i pixels = load(src);

                if (_mm_testc_si128(pixels, alphaMask)) {
                    store(dst, pixels);
                } else if (!_mm_testz_si128(pixels, alphaMask)) {
                    srcover_srgb_srgb_4(dst, src);
                }

                dst += 4;
                src += 4;
            }

            count = count & 3;
            while (count-- > 0) {
                srcover_srgb_srgb_1(dst++, *src++);
            }
        }
    }

#else

    static void srcover_srgb_srgb(
        uint32_t* dst, const uint32_t* const src, int ndst, const int nsrc) {
        while (ndst > 0) {
            int n = SkTMin(ndst, nsrc);

            for (int i = 0; i < n; i++) {
                srcover_srgb_srgb_1(dst++, src[i]);
            }
            ndst -= n;
        }
    }

#endif

}  // namespace SK_OPTS_NS

#endif//SkBlend_opts_DEFINED
