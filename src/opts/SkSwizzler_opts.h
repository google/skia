/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSwizzler_opts_DEFINED
#define SkSwizzler_opts_DEFINED

#include "SkColorData.h"

#include <utility>

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
    #include <immintrin.h>
#elif defined(SK_ARM_HAS_NEON)
    #include <arm_neon.h>
#endif

namespace SK_OPTS_NS {

static void RGBA_to_rgbA_portable(uint32_t* dst, const uint32_t* src, int count) {
    for (int i = 0; i < count; i++) {
        uint8_t a = (src[i] >> 24) & 0xFF,
                b = (src[i] >> 16) & 0xFF,
                g = (src[i] >>  8) & 0xFF,
                r = (src[i] >>  0) & 0xFF;
        b = (b*a+127)/255;
        g = (g*a+127)/255;
        r = (r*a+127)/255;
        dst[i] = (uint32_t)a << 24
               | (uint32_t)b << 16
               | (uint32_t)g <<  8
               | (uint32_t)r <<  0;
    }
}

static void RGBA_to_bgrA_portable(uint32_t* dst, const uint32_t* src, int count) {
    for (int i = 0; i < count; i++) {
        uint8_t a = (src[i] >> 24) & 0xFF,
                b = (src[i] >> 16) & 0xFF,
                g = (src[i] >>  8) & 0xFF,
                r = (src[i] >>  0) & 0xFF;
        b = (b*a+127)/255;
        g = (g*a+127)/255;
        r = (r*a+127)/255;
        dst[i] = (uint32_t)a << 24
               | (uint32_t)r << 16
               | (uint32_t)g <<  8
               | (uint32_t)b <<  0;
    }
}

static void RGBA_to_BGRA_portable(uint32_t* dst, const uint32_t* src, int count) {
    for (int i = 0; i < count; i++) {
        uint8_t a = (src[i] >> 24) & 0xFF,
                b = (src[i] >> 16) & 0xFF,
                g = (src[i] >>  8) & 0xFF,
                r = (src[i] >>  0) & 0xFF;
        dst[i] = (uint32_t)a << 24
               | (uint32_t)r << 16
               | (uint32_t)g <<  8
               | (uint32_t)b <<  0;
    }
}

static void RGB_to_RGB1_portable(uint32_t dst[], const uint8_t* src, int count) {
    for (int i = 0; i < count; i++) {
        uint8_t r = src[0],
                g = src[1],
                b = src[2];
        src += 3;
        dst[i] = (uint32_t)0xFF << 24
               | (uint32_t)b    << 16
               | (uint32_t)g    <<  8
               | (uint32_t)r    <<  0;
    }
}

static void RGB_to_BGR1_portable(uint32_t dst[], const uint8_t* src, int count) {
    for (int i = 0; i < count; i++) {
        uint8_t r = src[0],
                g = src[1],
                b = src[2];
        src += 3;
        dst[i] = (uint32_t)0xFF << 24
               | (uint32_t)r    << 16
               | (uint32_t)g    <<  8
               | (uint32_t)b    <<  0;
    }
}

static void gray_to_RGB1_portable(uint32_t dst[], const uint8_t* src, int count) {
    for (int i = 0; i < count; i++) {
        dst[i] = (uint32_t)0xFF   << 24
               | (uint32_t)src[i] << 16
               | (uint32_t)src[i] <<  8
               | (uint32_t)src[i] <<  0;
    }
}

static void grayA_to_RGBA_portable(uint32_t dst[], const uint8_t* src, int count) {
    for (int i = 0; i < count; i++) {
        uint8_t g = src[0],
                a = src[1];
        src += 2;
        dst[i] = (uint32_t)a << 24
               | (uint32_t)g << 16
               | (uint32_t)g <<  8
               | (uint32_t)g <<  0;
    }
}

static void grayA_to_rgbA_portable(uint32_t dst[], const uint8_t* src, int count) {
    for (int i = 0; i < count; i++) {
        uint8_t g = src[0],
                a = src[1];
        src += 2;
        g = (g*a+127)/255;
        dst[i] = (uint32_t)a << 24
               | (uint32_t)g << 16
               | (uint32_t)g <<  8
               | (uint32_t)g <<  0;
    }
}

static void inverted_CMYK_to_RGB1_portable(uint32_t* dst, const uint32_t* src, int count) {
    for (int i = 0; i < count; i++) {
        uint8_t k = (src[i] >> 24) & 0xFF,
                y = (src[i] >> 16) & 0xFF,
                m = (src[i] >>  8) & 0xFF,
                c = (src[i] >>  0) & 0xFF;
        // See comments in SkSwizzler.cpp for details on the conversion formula.
        uint8_t b = (y*k+127)/255,
                g = (m*k+127)/255,
                r = (c*k+127)/255;
        dst[i] = (uint32_t)0xFF << 24
               | (uint32_t)   b << 16
               | (uint32_t)   g <<  8
               | (uint32_t)   r <<  0;
    }
}

static void inverted_CMYK_to_BGR1_portable(uint32_t* dst, const uint32_t* src, int count) {
    for (int i = 0; i < count; i++) {
        uint8_t k = (src[i] >> 24) & 0xFF,
                y = (src[i] >> 16) & 0xFF,
                m = (src[i] >>  8) & 0xFF,
                c = (src[i] >>  0) & 0xFF;
        uint8_t b = (y*k+127)/255,
                g = (m*k+127)/255,
                r = (c*k+127)/255;
        dst[i] = (uint32_t)0xFF << 24
               | (uint32_t)   r << 16
               | (uint32_t)   g <<  8
               | (uint32_t)   b <<  0;
    }
}

#if defined(SK_ARM_HAS_NEON)

// Rounded divide by 255, (x + 127) / 255
static uint8x8_t div255_round(uint16x8_t x) {
    // result = (x + 127) / 255
    // result = (x + 127) / 256 + error1
    //
    // error1 = (x + 127) / (255 * 256)
    // error1 = (x + 127) / (256 * 256) + error2
    //
    // error2 = (x + 127) / (255 * 256 * 256)
    //
    // The maximum value of error2 is too small to matter.  Thus:
    // result = (x + 127) / 256 + (x + 127) / (256 * 256)
    // result = ((x + 127) / 256 + x + 127) / 256
    // result = ((x + 127) >> 8 + x + 127) >> 8
    //
    // Use >>> to represent "rounded right shift" which, conveniently,
    // NEON supports in one instruction.
    // result = ((x >>> 8) + x) >>> 8
    //
    // Note that the second right shift is actually performed as an
    // "add, round, and narrow back to 8-bits" instruction.
    return vraddhn_u16(x, vrshrq_n_u16(x, 8));
}

// Scale a byte by another, (x * y + 127) / 255
static uint8x8_t scale(uint8x8_t x, uint8x8_t y) {
    return div255_round(vmull_u8(x, y));
}

template <bool kSwapRB>
static void premul_should_swapRB(uint32_t* dst, const uint32_t* src, int count) {
    while (count >= 8) {
        // Load 8 pixels.
        uint8x8x4_t rgba = vld4_u8((const uint8_t*) src);

        uint8x8_t a = rgba.val[3],
                  b = rgba.val[2],
                  g = rgba.val[1],
                  r = rgba.val[0];

        // Premultiply.
        b = scale(b, a);
        g = scale(g, a);
        r = scale(r, a);

        // Store 8 premultiplied pixels.
        if (kSwapRB) {
            rgba.val[2] = r;
            rgba.val[1] = g;
            rgba.val[0] = b;
        } else {
            rgba.val[2] = b;
            rgba.val[1] = g;
            rgba.val[0] = r;
        }
        vst4_u8((uint8_t*) dst, rgba);
        src += 8;
        dst += 8;
        count -= 8;
    }

    // Call portable code to finish up the tail of [0,8) pixels.
    auto proc = kSwapRB ? RGBA_to_bgrA_portable : RGBA_to_rgbA_portable;
    proc(dst, src, count);
}

/*not static*/ inline void RGBA_to_rgbA(uint32_t* dst, const uint32_t* src, int count) {
    premul_should_swapRB<false>(dst, src, count);
}

/*not static*/ inline void RGBA_to_bgrA(uint32_t* dst, const uint32_t* src, int count) {
    premul_should_swapRB<true>(dst, src, count);
}

/*not static*/ inline void RGBA_to_BGRA(uint32_t* dst, const uint32_t* src, int count) {
    using std::swap;
    while (count >= 16) {
        // Load 16 pixels.
        uint8x16x4_t rgba = vld4q_u8((const uint8_t*) src);

        // Swap r and b.
        swap(rgba.val[0], rgba.val[2]);

        // Store 16 pixels.
        vst4q_u8((uint8_t*) dst, rgba);
        src += 16;
        dst += 16;
        count -= 16;
    }

    if (count >= 8) {
        // Load 8 pixels.
        uint8x8x4_t rgba = vld4_u8((const uint8_t*) src);

        // Swap r and b.
        swap(rgba.val[0], rgba.val[2]);

        // Store 8 pixels.
        vst4_u8((uint8_t*) dst, rgba);
        src += 8;
        dst += 8;
        count -= 8;
    }

    RGBA_to_BGRA_portable(dst, src, count);
}

template <bool kSwapRB>
static void insert_alpha_should_swaprb(uint32_t dst[], const uint8_t* src, int count) {
    while (count >= 16) {
        // Load 16 pixels.
        uint8x16x3_t rgb = vld3q_u8(src);

        // Insert an opaque alpha channel and swap if needed.
        uint8x16x4_t rgba;
        if (kSwapRB) {
            rgba.val[0] = rgb.val[2];
            rgba.val[2] = rgb.val[0];
        } else {
            rgba.val[0] = rgb.val[0];
            rgba.val[2] = rgb.val[2];
        }
        rgba.val[1] = rgb.val[1];
        rgba.val[3] = vdupq_n_u8(0xFF);

        // Store 16 pixels.
        vst4q_u8((uint8_t*) dst, rgba);
        src += 16*3;
        dst += 16;
        count -= 16;
    }

    if (count >= 8) {
        // Load 8 pixels.
        uint8x8x3_t rgb = vld3_u8(src);

        // Insert an opaque alpha channel and swap if needed.
        uint8x8x4_t rgba;
        if (kSwapRB) {
            rgba.val[0] = rgb.val[2];
            rgba.val[2] = rgb.val[0];
        } else {
            rgba.val[0] = rgb.val[0];
            rgba.val[2] = rgb.val[2];
        }
        rgba.val[1] = rgb.val[1];
        rgba.val[3] = vdup_n_u8(0xFF);

        // Store 8 pixels.
        vst4_u8((uint8_t*) dst, rgba);
        src += 8*3;
        dst += 8;
        count -= 8;
    }

    // Call portable code to finish up the tail of [0,8) pixels.
    auto proc = kSwapRB ? RGB_to_BGR1_portable : RGB_to_RGB1_portable;
    proc(dst, src, count);
}

/*not static*/ inline void RGB_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
    insert_alpha_should_swaprb<false>(dst, src, count);
}

/*not static*/ inline void RGB_to_BGR1(uint32_t dst[], const uint8_t* src, int count) {
    insert_alpha_should_swaprb<true>(dst, src, count);
}

/*not static*/ inline void gray_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
    while (count >= 16) {
        // Load 16 pixels.
        uint8x16_t gray = vld1q_u8(src);

        // Set each of the color channels.
        uint8x16x4_t rgba;
        rgba.val[0] = gray;
        rgba.val[1] = gray;
        rgba.val[2] = gray;
        rgba.val[3] = vdupq_n_u8(0xFF);

        // Store 16 pixels.
        vst4q_u8((uint8_t*) dst, rgba);
        src += 16;
        dst += 16;
        count -= 16;
    }

    if (count >= 8) {
        // Load 8 pixels.
        uint8x8_t gray = vld1_u8(src);

        // Set each of the color channels.
        uint8x8x4_t rgba;
        rgba.val[0] = gray;
        rgba.val[1] = gray;
        rgba.val[2] = gray;
        rgba.val[3] = vdup_n_u8(0xFF);

        // Store 8 pixels.
        vst4_u8((uint8_t*) dst, rgba);
        src += 8;
        dst += 8;
        count -= 8;
    }

    gray_to_RGB1_portable(dst, src, count);
}

template <bool kPremul>
static void expand_grayA(uint32_t dst[], const uint8_t* src, int count) {
    while (count >= 16) {
        // Load 16 pixels.
        uint8x16x2_t ga = vld2q_u8(src);

        // Premultiply if requested.
        if (kPremul) {
            ga.val[0] = vcombine_u8(
                    scale(vget_low_u8(ga.val[0]),  vget_low_u8(ga.val[1])),
                    scale(vget_high_u8(ga.val[0]), vget_high_u8(ga.val[1])));
        }

        // Set each of the color channels.
        uint8x16x4_t rgba;
        rgba.val[0] = ga.val[0];
        rgba.val[1] = ga.val[0];
        rgba.val[2] = ga.val[0];
        rgba.val[3] = ga.val[1];

        // Store 16 pixels.
        vst4q_u8((uint8_t*) dst, rgba);
        src += 16*2;
        dst += 16;
        count -= 16;
    }

    if (count >= 8) {
        // Load 8 pixels.
        uint8x8x2_t ga = vld2_u8(src);

        // Premultiply if requested.
        if (kPremul) {
            ga.val[0] = scale(ga.val[0], ga.val[1]);
        }

        // Set each of the color channels.
        uint8x8x4_t rgba;
        rgba.val[0] = ga.val[0];
        rgba.val[1] = ga.val[0];
        rgba.val[2] = ga.val[0];
        rgba.val[3] = ga.val[1];

        // Store 8 pixels.
        vst4_u8((uint8_t*) dst, rgba);
        src += 8*2;
        dst += 8;
        count -= 8;
    }

    auto proc = kPremul ? grayA_to_rgbA_portable : grayA_to_RGBA_portable;
    proc(dst, src, count);
}

/*not static*/ inline void grayA_to_RGBA(uint32_t dst[], const uint8_t* src, int count) {
    expand_grayA<false>(dst, src, count);
}

/*not static*/ inline void grayA_to_rgbA(uint32_t dst[], const uint8_t* src, int count) {
    expand_grayA<true>(dst, src, count);
}

enum Format { kRGB1, kBGR1 };
template <Format format>
static void inverted_cmyk_to(uint32_t* dst, const uint32_t* src, int count) {
    while (count >= 8) {
        // Load 8 cmyk pixels.
        uint8x8x4_t pixels = vld4_u8((const uint8_t*) src);

        uint8x8_t k = pixels.val[3],
                  y = pixels.val[2],
                  m = pixels.val[1],
                  c = pixels.val[0];

        // Scale to r, g, b.
        uint8x8_t b = scale(y, k);
        uint8x8_t g = scale(m, k);
        uint8x8_t r = scale(c, k);

        // Store 8 rgba pixels.
        if (kBGR1 == format) {
            pixels.val[3] = vdup_n_u8(0xFF);
            pixels.val[2] = r;
            pixels.val[1] = g;
            pixels.val[0] = b;
        } else {
            pixels.val[3] = vdup_n_u8(0xFF);
            pixels.val[2] = b;
            pixels.val[1] = g;
            pixels.val[0] = r;
        }
        vst4_u8((uint8_t*) dst, pixels);
        src += 8;
        dst += 8;
        count -= 8;
    }

    auto proc = (kBGR1 == format) ? inverted_CMYK_to_BGR1_portable : inverted_CMYK_to_RGB1_portable;
    proc(dst, src, count);
}

/*not static*/ inline void inverted_CMYK_to_RGB1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_cmyk_to<kRGB1>(dst, src, count);
}

/*not static*/ inline void inverted_CMYK_to_BGR1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_cmyk_to<kBGR1>(dst, src, count);
}

#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3

// Scale a byte by another.
// Inputs are stored in 16-bit lanes, but are not larger than 8-bits.
static __m128i scale(__m128i x, __m128i y) {
    const __m128i _128 = _mm_set1_epi16(128);
    const __m128i _257 = _mm_set1_epi16(257);

    // (x+127)/255 == ((x+128)*257)>>16 for 0 <= x <= 255*255.
    return _mm_mulhi_epu16(_mm_add_epi16(_mm_mullo_epi16(x, y), _128), _257);
}

template <bool kSwapRB>
static void premul_should_swapRB(uint32_t* dst, const uint32_t* src, int count) {

    auto premul8 = [](__m128i* lo, __m128i* hi) {
        const __m128i zeros = _mm_setzero_si128();
        __m128i planar;
        if (kSwapRB) {
            planar = _mm_setr_epi8(2,6,10,14, 1,5,9,13, 0,4,8,12, 3,7,11,15);
        } else {
            planar = _mm_setr_epi8(0,4,8,12, 1,5,9,13, 2,6,10,14, 3,7,11,15);
        }

        // Swizzle the pixels to 8-bit planar.
        *lo = _mm_shuffle_epi8(*lo, planar);                      // rrrrgggg bbbbaaaa
        *hi = _mm_shuffle_epi8(*hi, planar);                      // RRRRGGGG BBBBAAAA
        __m128i rg = _mm_unpacklo_epi32(*lo, *hi),                // rrrrRRRR ggggGGGG
                ba = _mm_unpackhi_epi32(*lo, *hi);                // bbbbBBBB aaaaAAAA

        // Unpack to 16-bit planar.
        __m128i r = _mm_unpacklo_epi8(rg, zeros),                 // r_r_r_r_ R_R_R_R_
                g = _mm_unpackhi_epi8(rg, zeros),                 // g_g_g_g_ G_G_G_G_
                b = _mm_unpacklo_epi8(ba, zeros),                 // b_b_b_b_ B_B_B_B_
                a = _mm_unpackhi_epi8(ba, zeros);                 // a_a_a_a_ A_A_A_A_

        // Premultiply!
        r = scale(r, a);
        g = scale(g, a);
        b = scale(b, a);

        // Repack into interlaced pixels.
        rg = _mm_or_si128(r, _mm_slli_epi16(g, 8));               // rgrgrgrg RGRGRGRG
        ba = _mm_or_si128(b, _mm_slli_epi16(a, 8));               // babababa BABABABA
        *lo = _mm_unpacklo_epi16(rg, ba);                         // rgbargba rgbargba
        *hi = _mm_unpackhi_epi16(rg, ba);                         // RGBARGBA RGBARGBA
    };

    while (count >= 8) {
        __m128i lo = _mm_loadu_si128((const __m128i*) (src + 0)),
                hi = _mm_loadu_si128((const __m128i*) (src + 4));

        premul8(&lo, &hi);

        _mm_storeu_si128((__m128i*) (dst + 0), lo);
        _mm_storeu_si128((__m128i*) (dst + 4), hi);

        src += 8;
        dst += 8;
        count -= 8;
    }

    if (count >= 4) {
        __m128i lo = _mm_loadu_si128((const __m128i*) src),
                hi = _mm_setzero_si128();

        premul8(&lo, &hi);

        _mm_storeu_si128((__m128i*) dst, lo);

        src += 4;
        dst += 4;
        count -= 4;
    }

    // Call portable code to finish up the tail of [0,4) pixels.
    auto proc = kSwapRB ? RGBA_to_bgrA_portable : RGBA_to_rgbA_portable;
    proc(dst, src, count);
}

/*not static*/ inline void RGBA_to_rgbA(uint32_t* dst, const uint32_t* src, int count) {
    premul_should_swapRB<false>(dst, src, count);
}

/*not static*/ inline void RGBA_to_bgrA(uint32_t* dst, const uint32_t* src, int count) {
    premul_should_swapRB<true>(dst, src, count);
}

/*not static*/ inline void RGBA_to_BGRA(uint32_t* dst, const uint32_t* src, int count) {
    const __m128i swapRB = _mm_setr_epi8(2,1,0,3, 6,5,4,7, 10,9,8,11, 14,13,12,15);

    while (count >= 4) {
        __m128i rgba = _mm_loadu_si128((const __m128i*) src);
        __m128i bgra = _mm_shuffle_epi8(rgba, swapRB);
        _mm_storeu_si128((__m128i*) dst, bgra);

        src += 4;
        dst += 4;
        count -= 4;
    }

    RGBA_to_BGRA_portable(dst, src, count);
}

template <bool kSwapRB>
static void insert_alpha_should_swaprb(uint32_t dst[], const uint8_t* src, int count) {
    const __m128i alphaMask = _mm_set1_epi32(0xFF000000);
    __m128i expand;
    const uint8_t X = 0xFF; // Used a placeholder.  The value of X is irrelevant.
    if (kSwapRB) {
        expand = _mm_setr_epi8(2,1,0,X, 5,4,3,X, 8,7,6,X, 11,10,9,X);
    } else {
        expand = _mm_setr_epi8(0,1,2,X, 3,4,5,X, 6,7,8,X, 9,10,11,X);
    }

    while (count >= 6) {
        // Load a vector.  While this actually contains 5 pixels plus an
        // extra component, we will discard all but the first four pixels on
        // this iteration.
        __m128i rgb = _mm_loadu_si128((const __m128i*) src);

        // Expand the first four pixels to RGBX and then mask to RGB(FF).
        __m128i rgba = _mm_or_si128(_mm_shuffle_epi8(rgb, expand), alphaMask);

        // Store 4 pixels.
        _mm_storeu_si128((__m128i*) dst, rgba);

        src += 4*3;
        dst += 4;
        count -= 4;
    }

    // Call portable code to finish up the tail of [0,4) pixels.
    auto proc = kSwapRB ? RGB_to_BGR1_portable : RGB_to_RGB1_portable;
    proc(dst, src, count);
}

/*not static*/ inline void RGB_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
    insert_alpha_should_swaprb<false>(dst, src, count);
}

/*not static*/ inline void RGB_to_BGR1(uint32_t dst[], const uint8_t* src, int count) {
    insert_alpha_should_swaprb<true>(dst, src, count);
}

/*not static*/ inline void gray_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
    const __m128i alphas = _mm_set1_epi8((uint8_t) 0xFF);
    while (count >= 16) {
        __m128i grays = _mm_loadu_si128((const __m128i*) src);

        __m128i gg_lo = _mm_unpacklo_epi8(grays, grays);
        __m128i gg_hi = _mm_unpackhi_epi8(grays, grays);
        __m128i ga_lo = _mm_unpacklo_epi8(grays, alphas);
        __m128i ga_hi = _mm_unpackhi_epi8(grays, alphas);

        __m128i ggga0 = _mm_unpacklo_epi16(gg_lo, ga_lo);
        __m128i ggga1 = _mm_unpackhi_epi16(gg_lo, ga_lo);
        __m128i ggga2 = _mm_unpacklo_epi16(gg_hi, ga_hi);
        __m128i ggga3 = _mm_unpackhi_epi16(gg_hi, ga_hi);

        _mm_storeu_si128((__m128i*) (dst +  0), ggga0);
        _mm_storeu_si128((__m128i*) (dst +  4), ggga1);
        _mm_storeu_si128((__m128i*) (dst +  8), ggga2);
        _mm_storeu_si128((__m128i*) (dst + 12), ggga3);

        src += 16;
        dst += 16;
        count -= 16;
    }

    gray_to_RGB1_portable(dst, src, count);
}

/*not static*/ inline void grayA_to_RGBA(uint32_t dst[], const uint8_t* src, int count) {
    while (count >= 8) {
        __m128i ga = _mm_loadu_si128((const __m128i*) src);

        __m128i gg = _mm_or_si128(_mm_and_si128(ga, _mm_set1_epi16(0x00FF)),
                                  _mm_slli_epi16(ga, 8));

        __m128i ggga_lo = _mm_unpacklo_epi16(gg, ga);
        __m128i ggga_hi = _mm_unpackhi_epi16(gg, ga);

        _mm_storeu_si128((__m128i*) (dst +  0), ggga_lo);
        _mm_storeu_si128((__m128i*) (dst +  4), ggga_hi);

        src += 8*2;
        dst += 8;
        count -= 8;
    }

    grayA_to_RGBA_portable(dst, src, count);
}

/*not static*/ inline void grayA_to_rgbA(uint32_t dst[], const uint8_t* src, int count) {
    while (count >= 8) {
        __m128i grayA = _mm_loadu_si128((const __m128i*) src);

        __m128i g0 = _mm_and_si128(grayA, _mm_set1_epi16(0x00FF));
        __m128i a0 = _mm_srli_epi16(grayA, 8);

        // Premultiply
        g0 = scale(g0, a0);

        __m128i gg = _mm_or_si128(g0, _mm_slli_epi16(g0, 8));
        __m128i ga = _mm_or_si128(g0, _mm_slli_epi16(a0, 8));


        __m128i ggga_lo = _mm_unpacklo_epi16(gg, ga);
        __m128i ggga_hi = _mm_unpackhi_epi16(gg, ga);

        _mm_storeu_si128((__m128i*) (dst +  0), ggga_lo);
        _mm_storeu_si128((__m128i*) (dst +  4), ggga_hi);

        src += 8*2;
        dst += 8;
        count -= 8;
    }

    grayA_to_rgbA_portable(dst, src, count);
}

enum Format { kRGB1, kBGR1 };
template <Format format>
static void inverted_cmyk_to(uint32_t* dst, const uint32_t* src, int count) {
    auto convert8 = [](__m128i* lo, __m128i* hi) {
        const __m128i zeros = _mm_setzero_si128();
        __m128i planar;
        if (kBGR1 == format) {
            planar = _mm_setr_epi8(2,6,10,14, 1,5,9,13, 0,4,8,12, 3,7,11,15);
        } else {
            planar = _mm_setr_epi8(0,4,8,12, 1,5,9,13, 2,6,10,14, 3,7,11,15);
        }

        // Swizzle the pixels to 8-bit planar.
        *lo = _mm_shuffle_epi8(*lo, planar);                                 // ccccmmmm yyyykkkk
        *hi = _mm_shuffle_epi8(*hi, planar);                                 // CCCCMMMM YYYYKKKK
        __m128i cm = _mm_unpacklo_epi32(*lo, *hi),                           // ccccCCCC mmmmMMMM
                yk = _mm_unpackhi_epi32(*lo, *hi);                           // yyyyYYYY kkkkKKKK

        // Unpack to 16-bit planar.
        __m128i c = _mm_unpacklo_epi8(cm, zeros),                            // c_c_c_c_ C_C_C_C_
                m = _mm_unpackhi_epi8(cm, zeros),                            // m_m_m_m_ M_M_M_M_
                y = _mm_unpacklo_epi8(yk, zeros),                            // y_y_y_y_ Y_Y_Y_Y_
                k = _mm_unpackhi_epi8(yk, zeros);                            // k_k_k_k_ K_K_K_K_

        // Scale to r, g, b.
        __m128i r = scale(c, k),
                g = scale(m, k),
                b = scale(y, k);

        // Repack into interlaced pixels.
        __m128i rg = _mm_or_si128(r, _mm_slli_epi16(g, 8)),                  // rgrgrgrg RGRGRGRG
                ba = _mm_or_si128(b, _mm_set1_epi16((uint16_t) 0xFF00));     // b1b1b1b1 B1B1B1B1
        *lo = _mm_unpacklo_epi16(rg, ba);                                    // rgbargba rgbargba
        *hi = _mm_unpackhi_epi16(rg, ba);                                    // RGB1RGB1 RGB1RGB1
    };

    while (count >= 8) {
        __m128i lo = _mm_loadu_si128((const __m128i*) (src + 0)),
                hi = _mm_loadu_si128((const __m128i*) (src + 4));

        convert8(&lo, &hi);

        _mm_storeu_si128((__m128i*) (dst + 0), lo);
        _mm_storeu_si128((__m128i*) (dst + 4), hi);

        src += 8;
        dst += 8;
        count -= 8;
    }

    if (count >= 4) {
        __m128i lo = _mm_loadu_si128((const __m128i*) src),
                hi = _mm_setzero_si128();

        convert8(&lo, &hi);

        _mm_storeu_si128((__m128i*) dst, lo);

        src += 4;
        dst += 4;
        count -= 4;
    }

    auto proc = (kBGR1 == format) ? inverted_CMYK_to_BGR1_portable : inverted_CMYK_to_RGB1_portable;
    proc(dst, src, count);
}

/*not static*/ inline void inverted_CMYK_to_RGB1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_cmyk_to<kRGB1>(dst, src, count);
}

/*not static*/ inline void inverted_CMYK_to_BGR1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_cmyk_to<kBGR1>(dst, src, count);
}

#else

/*not static*/ inline void RGBA_to_rgbA(uint32_t* dst, const uint32_t* src, int count) {
    RGBA_to_rgbA_portable(dst, src, count);
}

/*not static*/ inline void RGBA_to_bgrA(uint32_t* dst, const uint32_t* src, int count) {
    RGBA_to_bgrA_portable(dst, src, count);
}

/*not static*/ inline void RGBA_to_BGRA(uint32_t* dst, const uint32_t* src, int count) {
    RGBA_to_BGRA_portable(dst, src, count);
}

/*not static*/ inline void RGB_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
    RGB_to_RGB1_portable(dst, src, count);
}

/*not static*/ inline void RGB_to_BGR1(uint32_t dst[], const uint8_t* src, int count) {
    RGB_to_BGR1_portable(dst, src, count);
}

/*not static*/ inline void gray_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
    gray_to_RGB1_portable(dst, src, count);
}

/*not static*/ inline void grayA_to_RGBA(uint32_t dst[], const uint8_t* src, int count) {
    grayA_to_RGBA_portable(dst, src, count);
}

/*not static*/ inline void grayA_to_rgbA(uint32_t dst[], const uint8_t* src, int count) {
    grayA_to_rgbA_portable(dst, src, count);
}

/*not static*/ inline void inverted_CMYK_to_RGB1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_CMYK_to_RGB1_portable(dst, src, count);
}

/*not static*/ inline void inverted_CMYK_to_BGR1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_CMYK_to_BGR1_portable(dst, src, count);
}

#endif

}

#endif // SkSwizzler_opts_DEFINED
