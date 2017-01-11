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

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include <immintrin.h>
#elif defined(SK_ARM_HAS_NEON)
    #include <arm_neon.h>
#endif

namespace SK_OPTS_NS {

static inline void srcover_srgb_srgb_1(uint32_t* dst, uint32_t src) {
    if (src >= 0xFF000000) {
        *dst = src;
        return;
    } else if (src <= 0x00FFFFFF) {
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

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2

    static inline __m128i load(const uint32_t* p) {
        return _mm_loadu_si128(reinterpret_cast<const __m128i*>(p));
    }

    static inline void store(uint32_t* p, __m128i v) {
        _mm_storeu_si128(reinterpret_cast<__m128i*>(p), v);
    }

    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41

        static void srcover_srgb_srgb(
            uint32_t* dst, const uint32_t* const srcStart, int ndst, const int nsrc) {
            const __m128i alphaMask = _mm_set1_epi32(0xFF000000);
            while (ndst > 0) {
                int count = SkTMin(ndst, nsrc);
                ndst -= count;
                const uint32_t* src = srcStart;
                const uint32_t* end = dst + (count & ~3);
                ptrdiff_t delta = src - dst;

                while (dst < end) {
                    __m128i pixels = load(src);
                    if (_mm_testc_si128(pixels, alphaMask)) {
                         uint32_t* start = dst;
                        do {
                            store(dst, pixels);
                            dst += 4;
                        } while (dst < end
                                 && _mm_testc_si128(pixels = load(dst + delta), alphaMask));
                        src += dst - start;
                    } else if (_mm_testz_si128(pixels, alphaMask)) {
                        do {
                            dst += 4;
                            src += 4;
                        } while (dst < end
                                 && _mm_testz_si128(pixels = load(src), alphaMask));
                    } else {
                        uint32_t* start = dst;
                        do {
                            srcover_srgb_srgb_4(dst, dst + delta);
                            dst += 4;
                        } while (dst < end
                                 && _mm_testnzc_si128(pixels = load(dst + delta), alphaMask));
                        src += dst - start;
                    }
                }

                count = count & 3;
                while (count-- > 0) {
                    srcover_srgb_srgb_1(dst++, *src++);
                }
            }
        }
    #else
    // SSE2 versions

        // Note: In the next three comparisons a group of 4 pixels is converted to a group of
        // "signed" pixels because the sse2 does not have an unsigned comparison.
        // Make it so that we can use the signed comparison operators by biasing
        // 0x00xxxxxx to 0x80xxxxxxx which is the smallest values and biasing 0xffxxxxxx to
        // 0x7fxxxxxx which is the largest set of values.
        static inline bool check_opaque_alphas(__m128i pixels) {
            __m128i signedPixels = _mm_xor_si128(pixels, _mm_set1_epi32(0x80000000));
            int mask =
                _mm_movemask_epi8(
                    _mm_cmplt_epi32(signedPixels, _mm_set1_epi32(0x7F000000)));
            return mask == 0;
        }

        static inline bool check_transparent_alphas(__m128i pixels) {
            __m128i signedPixels = _mm_xor_si128(pixels, _mm_set1_epi32(0x80000000));
            int mask =
                _mm_movemask_epi8(
                    _mm_cmpgt_epi32(signedPixels, _mm_set1_epi32(0x80FFFFFF)));
            return mask == 0;
        }

        static inline bool check_partial_alphas(__m128i pixels) {
            __m128i signedPixels = _mm_xor_si128(pixels, _mm_set1_epi32(0x80000000));
            __m128i opaque       = _mm_cmplt_epi32(signedPixels, _mm_set1_epi32(0x7F000000));
            __m128i transparent  = _mm_cmpgt_epi32(signedPixels, _mm_set1_epi32(0x80FFFFFF));
            int mask             = _mm_movemask_epi8(_mm_xor_si128(opaque, transparent));
            return mask == 0;
        }

        static void srcover_srgb_srgb(
            uint32_t* dst, const uint32_t* const srcStart, int ndst, const int nsrc) {
            while (ndst > 0) {
                int count = SkTMin(ndst, nsrc);
                ndst -= count;
                const uint32_t* src = srcStart;
                const uint32_t* end = dst + (count & ~3);
                const ptrdiff_t delta = src - dst;

                __m128i pixels = load(src);
                do {
                    if (check_opaque_alphas(pixels)) {
                        uint32_t* start = dst;
                        do {
                            store(dst, pixels);
                            dst += 4;
                        } while (dst < end && check_opaque_alphas((pixels = load(dst + delta))));
                        src += dst - start;
                    } else if (check_transparent_alphas(pixels)) {
                        const uint32_t* start = dst;
                        do {
                            dst += 4;
                        } while (dst < end && check_transparent_alphas(pixels = load(dst + delta)));
                        src += dst - start;
                    } else {
                        const uint32_t* start = dst;
                        do {
                            srcover_srgb_srgb_4(dst, dst + delta);
                            dst += 4;
                        } while (dst < end && check_partial_alphas(pixels = load(dst + delta)));
                        src += dst - start;
                    }
                } while (dst < end);

                count = count & 3;
                while (count-- > 0) {
                    srcover_srgb_srgb_1(dst++, *src++);
                }
            }
        }
    #endif
#elif defined(SK_ARM_HAS_NEON)
    static inline uint32x4_t load(const uint32_t* p) {
        return vld1q_u32(p);
    }

    static inline void store(uint32_t* p, uint32x4_t v) {
        vst1q_u32(p, v);
    }

    static inline bool check_opaque_alphas(uint32x4_t pixels) {
        uint64_t mask =
            vget_lane_u64(
                vreinterpret_u64_u16(
                    vmovn_u32(
                        vcltq_u32(pixels, vdupq_n_u32(0xFF000000)))),
                0);
        return mask == 0;
    }

    static inline bool check_transparent_alphas(uint32x4_t pixels) {
        uint64_t mask =
            vget_lane_u64(
                vreinterpret_u64_u16(
                    vmovn_u32(
                        vcgtq_u32(pixels, vdupq_n_u32(0x00FFFFFF)))),
                0);
        return mask == 0;
    }

    static inline bool check_partial_alphas(uint32x4_t pixels) {
        uint32x4_t opaque      = vcltq_u32(pixels, vdupq_n_u32(0xFF000000));
        uint32x4_t transparent = vcgtq_u32(pixels, vdupq_n_u32(0x00FFFFFF));
        uint64_t mask =
            vget_lane_u64(
                vreinterpret_u64_u16(
                    vmovn_u32(
                        veorq_u32(opaque, transparent))),
                0);
        return mask == 0;
    }

    static void srcover_srgb_srgb(
        uint32_t* dst, const uint32_t* const srcStart, int ndst, const int nsrc) {
        while (ndst > 0) {
            int count = SkTMin(ndst, nsrc);
            ndst -= count;
            const uint32_t* src = srcStart;
            const uint32_t* end = dst + (count & ~3);
            const ptrdiff_t delta = src - dst;

            uint32x4_t pixels = load(src);
            do {
                if (check_opaque_alphas(pixels)) {
                    uint32_t* start = dst;
                    do {
                        store(dst, pixels);
                        dst += 4;
                    } while (dst < end && check_opaque_alphas((pixels = load(dst + delta))));
                    src += dst - start;
                } else if (check_transparent_alphas(pixels)) {
                    const uint32_t* start = dst;
                    do {
                        dst += 4;
                    } while (dst < end && check_transparent_alphas(pixels = load(dst + delta)));
                    src += dst - start;
                } else {
                    const uint32_t* start = dst;
                    do {
                        srcover_srgb_srgb_4(dst, dst + delta);
                        dst += 4;
                    } while (dst < end && check_partial_alphas(pixels = load(dst + delta)));
                    src += dst - start;
                }
            } while (dst < end);

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
