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
#endif

namespace SK_OPTS_NS {

// An implementation of SrcOver from bytes to bytes in linear space that takes advantage of the
// observation that the 255's cancel.
//    invA = 1 - (As / 255);
//
//    R = 255 * sqrt((Rs/255)^2 + (Rd/255)^2 * invA)
// => R = 255 * sqrt((Rs^2 + Rd^2 * invA)/255^2)
// => R = sqrt(Rs^2 + Rd^2 * invA)
static inline void blend_srgb_srgb_1(uint32_t* dst, const uint32_t pixel) {
    Sk4f s = srgb_to_linear(to_4f(pixel));
    Sk4f d = srgb_to_linear(to_4f(*dst));
    Sk4f invAlpha = 1.0f - Sk4f{s[SkPM4f::A]} * (1.0f / 255.0f);
    Sk4f r = linear_to_srgb(s + d * invAlpha) + 0.5f;
    *dst = to_4b(r);
}

static inline void srcover_srgb_srgb_1(uint32_t* dst, const uint32_t pixel) {
    if ((~pixel & 0xFF000000) == 0) {
        *dst = pixel;
    } else if ((pixel & 0xFF000000) != 0) {
        blend_srgb_srgb_1(dst, pixel);
    }
}

static inline void srcover_srgb_srgb_2(uint32_t* dst, const uint32_t* src) {
    srcover_srgb_srgb_1(dst++, *src++);
    srcover_srgb_srgb_1(dst, *src);
}

static inline void srcover_srgb_srgb_4(uint32_t* dst, const uint32_t* src) {
    srcover_srgb_srgb_1(dst++, *src++);
    srcover_srgb_srgb_1(dst++, *src++);
    srcover_srgb_srgb_1(dst++, *src++);
    srcover_srgb_srgb_1(dst, *src);
}

void best_non_simd_srcover_srgb_srgb(
    uint32_t* dst, const uint32_t* const src, int ndst, const int nsrc) {
    uint64_t* ddst = reinterpret_cast<uint64_t*>(dst);

    while (ndst >0) {
        int count = SkTMin(ndst, nsrc);
        ndst -= count;
        const uint64_t* dsrc = reinterpret_cast<const uint64_t*>(src);
        const uint64_t* end = dsrc + (count >> 1);
        do {
            if ((~*dsrc & 0xFF000000FF000000) == 0) {
                do {
                    *ddst++ = *dsrc++;
                } while (dsrc < end && (~*dsrc & 0xFF000000FF000000) == 0);
            } else if ((*dsrc & 0xFF000000FF000000) == 0) {
                do {
                    dsrc++;
                    ddst++;
                } while (dsrc < end && (*dsrc & 0xFF000000FF000000) == 0);
            } else {
                srcover_srgb_srgb_2(reinterpret_cast<uint32_t*>(ddst++),
                                    reinterpret_cast<const uint32_t*>(dsrc++));
            }
        } while (dsrc < end);

        if ((count & 1) != 0) {
            srcover_srgb_srgb_1(reinterpret_cast<uint32_t*>(ddst),
                                *reinterpret_cast<const uint32_t*>(dsrc));
        }
    }
}

void brute_force_srcover_srgb_srgb(
    uint32_t* dst, const uint32_t* const src, int ndst, const int nsrc) {
    while (ndst > 0) {
        int n = SkTMin(ndst, nsrc);

        for (int i = 0; i < n; i++) {
            blend_srgb_srgb_1(dst++, src[i]);
        }
        ndst -= n;
    }
}

void trivial_srcover_srgb_srgb(
    uint32_t* dst, const uint32_t* const src, int ndst, const int nsrc) {
    while (ndst > 0) {
        int n = SkTMin(ndst, nsrc);

        for (int i = 0; i < n; i++) {
            srcover_srgb_srgb_1(dst++, src[i]);
        }
        ndst -= n;
    }
}

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2

    static inline __m128i load(const uint32_t* p) {
        return _mm_loadu_si128(reinterpret_cast<const __m128i*>(p));
    }

    static inline void store(uint32_t* p, __m128i v) {
        _mm_storeu_si128(reinterpret_cast<__m128i*>(p), v);
    }

    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41

        void srcover_srgb_srgb(
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

        void srcover_srgb_srgb(
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
#else

    void srcover_srgb_srgb(
        uint32_t* dst, const uint32_t* const src, int ndst, const int nsrc) {
        trivial_srcover_srgb_srgb(dst, src, ndst, nsrc);
    }

#endif

}  // namespace SK_OPTS_NS

#endif//SkBlend_opts_DEFINED
