/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSwizzler_opts_DEFINED
#define SkSwizzler_opts_DEFINED

#include "SkColorPriv.h"

namespace SK_OPTS_NS {

static void RGBA_to_rgbA_portable(uint32_t* dst, const void* vsrc, int count) {
    auto src = (const uint32_t*)vsrc;
    for (int i = 0; i < count; i++) {
        uint8_t a = src[i] >> 24,
                b = src[i] >> 16,
                g = src[i] >>  8,
                r = src[i] >>  0;
        b = (b*a+127)/255;
        g = (g*a+127)/255;
        r = (r*a+127)/255;
        dst[i] = (uint32_t)a << 24
               | (uint32_t)b << 16
               | (uint32_t)g <<  8
               | (uint32_t)r <<  0;
    }
}

static void RGBA_to_bgrA_portable(uint32_t* dst, const void* vsrc, int count) {
    auto src = (const uint32_t*)vsrc;
    for (int i = 0; i < count; i++) {
        uint8_t a = src[i] >> 24,
                b = src[i] >> 16,
                g = src[i] >>  8,
                r = src[i] >>  0;
        b = (b*a+127)/255;
        g = (g*a+127)/255;
        r = (r*a+127)/255;
        dst[i] = (uint32_t)a << 24
               | (uint32_t)r << 16
               | (uint32_t)g <<  8
               | (uint32_t)b <<  0;
    }
}

static void RGBA_to_BGRA_portable(uint32_t* dst, const void* vsrc, int count) {
    auto src = (const uint32_t*)vsrc;
    for (int i = 0; i < count; i++) {
        uint8_t a = src[i] >> 24,
                b = src[i] >> 16,
                g = src[i] >>  8,
                r = src[i] >>  0;
        dst[i] = (uint32_t)a << 24
               | (uint32_t)r << 16
               | (uint32_t)g <<  8
               | (uint32_t)b <<  0;
    }
}

static void RGB_to_RGB1_portable(uint32_t dst[], const void* vsrc, int count) {
    const uint8_t* src = (const uint8_t*)vsrc;
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

static void RGB_to_BGR1_portable(uint32_t dst[], const void* vsrc, int count) {
    const uint8_t* src = (const uint8_t*)vsrc;
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
static void premul_should_swapRB(uint32_t* dst, const void* vsrc, int count) {
    auto src = (const uint32_t*)vsrc;
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

static void RGBA_to_rgbA(uint32_t* dst, const void* src, int count) {
    premul_should_swapRB<false>(dst, src, count);
}

static void RGBA_to_bgrA(uint32_t* dst, const void* src, int count) {
    premul_should_swapRB<true>(dst, src, count);
}

static void RGBA_to_BGRA(uint32_t* dst, const void* vsrc, int count) {
    auto src = (const uint32_t*)vsrc;
    while (count >= 16) {
        // Load 16 pixels.
        uint8x16x4_t rgba = vld4q_u8((const uint8_t*) src);

        // Swap r and b.
        SkTSwap(rgba.val[0], rgba.val[2]);

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
        SkTSwap(rgba.val[0], rgba.val[2]);

        // Store 8 pixels.
        vst4_u8((uint8_t*) dst, rgba);
        src += 8;
        dst += 8;
        count -= 8;
    }

    RGBA_to_BGRA_portable(dst, src, count);
}

template <bool kSwapRB>
static void insert_alpha_should_swaprb(uint32_t dst[], const void* vsrc, int count) {
    const uint8_t* src = (const uint8_t*) vsrc;
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

static void RGB_to_RGB1(uint32_t dst[], const void* src, int count) {
    insert_alpha_should_swaprb<false>(dst, src, count);
}

static void RGB_to_BGR1(uint32_t dst[], const void* src, int count) {
    insert_alpha_should_swaprb<true>(dst, src, count);
}

#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3

template <bool kSwapRB>
static void premul_should_swapRB(uint32_t* dst, const void* vsrc, int count) {
    auto src = (const uint32_t*)vsrc;

    auto premul8 = [](__m128i* lo, __m128i* hi) {
        const __m128i zeros = _mm_setzero_si128();
        const __m128i _128 = _mm_set1_epi16(128);
        const __m128i _257 = _mm_set1_epi16(257);
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

        // Premultiply!  (x+127)/255 == ((x+128)*257)>>16 for 0 <= x <= 255*255.
        r = _mm_mulhi_epu16(_mm_add_epi16(_mm_mullo_epi16(r, a), _128), _257);
        g = _mm_mulhi_epu16(_mm_add_epi16(_mm_mullo_epi16(g, a), _128), _257);
        b = _mm_mulhi_epu16(_mm_add_epi16(_mm_mullo_epi16(b, a), _128), _257);

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

static void RGBA_to_rgbA(uint32_t* dst, const void* src, int count) {
    premul_should_swapRB<false>(dst, src, count);
}

static void RGBA_to_bgrA(uint32_t* dst, const void* src, int count) {
    premul_should_swapRB<true>(dst, src, count);
}

static void RGBA_to_BGRA(uint32_t* dst, const void* vsrc, int count) {
    auto src = (const uint32_t*)vsrc;
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
static void insert_alpha_should_swaprb(uint32_t dst[], const void* vsrc, int count) {
    const uint8_t* src = (const uint8_t*) vsrc;

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

static void RGB_to_RGB1(uint32_t dst[], const void* src, int count) {
    insert_alpha_should_swaprb<false>(dst, src, count);
}

static void RGB_to_BGR1(uint32_t dst[], const void* src, int count) {
    insert_alpha_should_swaprb<true>(dst, src, count);
}

#else

static void RGBA_to_rgbA(uint32_t* dst, const void* src, int count) {
    RGBA_to_rgbA_portable(dst, src, count);
}

static void RGBA_to_bgrA(uint32_t* dst, const void* src, int count) {
    RGBA_to_bgrA_portable(dst, src, count);
}

static void RGBA_to_BGRA(uint32_t* dst, const void* src, int count) {
    RGBA_to_BGRA_portable(dst, src, count);
}

static void RGB_to_RGB1(uint32_t dst[], const void* src, int count) {
    RGB_to_RGB1_portable(dst, src, count);
}

static void RGB_to_BGR1(uint32_t dst[], const void* src, int count) {
    RGB_to_BGR1_portable(dst, src, count);
}

#endif

}

#endif // SkSwizzler_opts_DEFINED
