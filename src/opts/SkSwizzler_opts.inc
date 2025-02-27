/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkUtils.h"
#include "src/base/SkVx.h"
#include "src/core/SkColorData.h"
#include "src/core/SkSwizzlePriv.h"

#include <algorithm>
#include <cmath>
#include <utility>

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
    #include <immintrin.h>
#elif defined(SK_ARM_HAS_NEON)
    #include <arm_neon.h>
#elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    #include <lasxintrin.h>
#elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    #include <lsxintrin.h>
#endif

// This file is included in multiple translation units with different #defines set enabling
// different instruction use for different CPU architectures.
//
// A pair of files controls what #defines are defined: SkOpts_SetTarget.h set the flags, and
// SkOpts_RestoreTarget.h restores them. SkOpts_SetTarget is controlled by setting the
// SK_OPTS_TARGET define before included it.
//
// SkOpts_SetTarget also sets the #define SK_OPTS_NS to the unique namespace for this code.

#if defined(__clang__) || defined(__GNUC__)
#define SI __attribute__((always_inline)) static inline
#else
#define SI static inline
#endif

namespace SK_OPTS_NS {

#if defined(SK_USE_FAST_UNPREMUL_324099025)
constexpr bool kFastUnpremul = true;
#else
constexpr bool kFastUnpremul = false;
#endif

SI float reciprocal_alpha_times_255_portable(float a) {
    return a != 0 ? 255.0f / a : 0.0f;
}

SI float reciprocal_alpha_portable(float a) {
    return a != 0 ? 1.0f / a : 0.0f;
}

#if defined(SK_ARM_HAS_NEON)
// -- NEON -- Harden against timing attacks
// For neon, the portable versions create branchless code.
SI float reciprocal_alpha_times_255(float a) {
    return reciprocal_alpha_times_255_portable(a);
}

SI float reciprocal_alpha(float a) {
    return reciprocal_alpha_portable(a);
}
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1 && (defined(__clang__) || !defined(_MSC_VER))
// -- SSE -- Harden against timing attacks -- MSVC is not supported.
using F4 = __m128;

SK_NO_SANITIZE("float-divide-by-zero")
SI float reciprocal_alpha_times_255(float a) {
    SkASSERT(0 <= a && a <= 255);
    F4 vA{a, a, a, a};
    auto q = F4{255.0f} / vA;
    return _mm_and_ps(sk_bit_cast<__m128>(vA != F4{0.0f}), q)[0];
}

SK_NO_SANITIZE("float-divide-by-zero")
SI float reciprocal_alpha(float a) {
    SkASSERT(0 <= a && a <= 1);
    F4 vA{a, a, a, a};
    auto q = F4{1.0f} / vA;
    return _mm_and_ps(sk_bit_cast<__m128>(vA != F4{0.0f}), q)[0];
}
#else
// -- Portable -- *Not* hardened against timing attacks
SI float reciprocal_alpha_times_255(float a) {
    return reciprocal_alpha_times_255_portable(a);
}

SI float reciprocal_alpha(float a) {
    return reciprocal_alpha_portable(a);
}
#endif

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

// RP uses the following rounding routines in store_8888. There are three different
// styles of rounding:
//   1) +0.5 and floor - used by scalar and ARMv7
//   2) round to even for sure - ARMv8
//   3) round to even maybe - intel. The rounding on intel depends on MXCSR which
//                            defaults to round to even.
//
// Note: that vrndns_f32 is the single float version of vcvtnq_u32_f32.

SI uint32_t pixel_round_as_RP(float n) {
#if defined(SK_ARM_HAS_NEON) && defined(SK_CPU_ARM64)
    return vrndns_f32(n);
#elif defined(SK_ARM_HAS_NEON) && !defined(SK_CPU_ARM64)
    float32x4_t vN{n + 0.5f};
    return vcvtq_u32_f32(vN)[0];
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2 && (defined(__clang__) || !defined(_MSC_VER))
    return _mm_cvtps_epi32(__m128{n})[0];
#else
    return (uint32_t)(n + 0.5f);
#endif
}

// Doing the math for an original color b resulting in a premul color x,
//   x = ⌊(b * a + 127) / 255⌋,
//   x ≤ (b * a + 127) / 255 < x + 1,
//   255 * x ≤ b * a + 127 < 255 * (x + 1),
//   255 * x - 127 ≤ b * a < 255 * (x + 1) - 127,
//   255 * x - 127 ≤ b * a < 255 * x + 128,
//   (255 * x - 127) / a ≤ b < (255 * x + 128) / a.
// So, given a premul value x < a, the original color b can be in the above range.
// We can pick the middle of that range as
//   b = 255 * x / a
//   b = x * (255 / a)
SI uint32_t unpremul_quick(float reciprocalA, float c) {
    return (uint32_t)std::min(255.0f, (c * reciprocalA + 0.5f));
}

// Similar to unpremul but simulates Raster Pipeline by normalizing the pixel on the interval
// [0, 1] and uses round-to-even in most cases instead of round-up.
SI uint32_t unpremul_simulating_RP(float reciprocalA, float c) {
    const float normalizedC = c * (1.0f / 255.0f);
    const float answer = std::min(255.0f, normalizedC * reciprocalA * 255.0f);
    return pixel_round_as_RP(answer);
}

SI uint32_t rgbA_to_CCCA(float c00, float c08, float c16, float a) {
    if constexpr (kFastUnpremul) {
        const float reciprocalA = reciprocal_alpha_times_255(a);
        auto unpremul = [reciprocalA](float c) -> uint32_t {
            return unpremul_quick(reciprocalA, c);
        };
        return (uint32_t) a << 24
               | unpremul(c16) << 16
               | unpremul(c08) <<  8
               | unpremul(c00) <<  0;
    } else {
        const float normalizedA = a * (1.0f / 255.0f);
        const float reciprocalA = reciprocal_alpha(normalizedA);
        auto unpremul = [reciprocalA](float c) -> uint32_t {
            return unpremul_simulating_RP(reciprocalA, c);
        };
        return (uint32_t) a << 24
               | unpremul(c16) << 16
               | unpremul(c08) <<  8
               | unpremul(c00) <<  0;
    }
}

static void rgbA_to_RGBA_portable(uint32_t* dst, const uint32_t* src, int count) {
    for (int i = 0; i < count; i++) {
        const uint32_t p = src[i];

        const float a = (p >> 24) & 0xFF,
                    b = (p >> 16) & 0xFF,
                    g = (p >>  8) & 0xFF,
                    r = (p >>  0) & 0xFF;

        dst[i] = rgbA_to_CCCA(r, g, b, a);
    }
}

static void rgbA_to_BGRA_portable(uint32_t* dst, const uint32_t* src, int count) {
    for (int i = 0; i < count; i++) {
        const uint32_t p = src[i];

        const uint32_t a = (p >> 24) & 0xFF,
                       b = (p >> 16) & 0xFF,
                       g = (p >>  8) & 0xFF,
                       r = (p >>  0) & 0xFF;

        dst[i] = rgbA_to_CCCA(b, g, r, a);
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
// -- NEON -----------------------------------------------------------------------------------------
// Rounded divide by 255, (x + 127) / 255
SI uint8x8_t div255_round(uint16x8_t x) {
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
SI uint8x8_t scale(uint8x8_t x, uint8x8_t y) {
    return div255_round(vmull_u8(x, y));
}

static void premul_should_swapRB(bool kSwapRB, uint32_t* dst, const uint32_t* src, int count) {
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

void RGBA_to_rgbA(uint32_t* dst, const uint32_t* src, int count) {
    premul_should_swapRB(false, dst, src, count);
}

void RGBA_to_bgrA(uint32_t* dst, const uint32_t* src, int count) {
    premul_should_swapRB(true, dst, src, count);
}

void RGBA_to_BGRA(uint32_t* dst, const uint32_t* src, int count) {
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

static void expand_grayA(bool kPremul, uint32_t dst[], const uint8_t* src, int count) {
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

void grayA_to_RGBA(uint32_t dst[], const uint8_t* src, int count) {
    expand_grayA(false, dst, src, count);
}

void grayA_to_rgbA(uint32_t dst[], const uint8_t* src, int count) {
    expand_grayA(true, dst, src, count);
}

enum Format { kRGB1, kBGR1 };
static void inverted_cmyk_to(Format format, uint32_t* dst, const uint32_t* src, int count) {
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

void inverted_CMYK_to_RGB1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_cmyk_to(kRGB1, dst, src, count);
}

void inverted_CMYK_to_BGR1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_cmyk_to(kBGR1, dst, src, count);
}

template <bool swapRB>
static void common_rgbA_to_RGBA(uint32_t* dst, const uint32_t* src, int count) {

    // Only use the SIMD code if simulating RP, otherwise the quick code auto-vectorizes will
    // enough on ARM to not need a SIMD implementation.
    if constexpr (!kFastUnpremul) {
        while (count >= 8) {
            const uint8x8x4_t in = vld4_u8((const uint8_t*)src);

            auto round = [](float32x4_t v) -> uint32x4_t {
                #if defined(SK_CPU_ARM64)
                    return vcvtnq_u32_f32(v);
                #else
                    return vcvtq_u32_f32(v + 0.5f);
                #endif
            };

            static constexpr float kN = 1.0f / 255.0f;
            auto toNormalized = [](uint16x4_t v) -> float32x4_t {
                return vcvtq_f32_u32(vmovl_u16(v)) * kN;
            };

            auto unpremulHalf =
                    [toNormalized, round](float32x4_t invA, uint16x4_t v) -> uint16x4_t {
                const float32x4_t normalizedV = toNormalized(v);
                const float32x4_t divided = invA * normalizedV;
                const float32x4_t denormalized = divided * 255.0f;
                const uint32x4_t rounded = round(denormalized);
                return vqmovn_u32(rounded);
            };

            auto reciprocal = [](float32x4_t a) -> float32x4_t {
                uint32x4_t mask = sk_bit_cast<uint32x4_t>(a != float32x4_t{0, 0, 0, 0});
                auto recip = 1.0f / a;
                return sk_bit_cast<float32x4_t>(mask & sk_bit_cast<uint32x4_t>(recip));
            };

            const uint8x8_t a = in.val[3];
            const uint16x8_t intA = vmovl_u8(a);
            const float32x4_t invALow = reciprocal(toNormalized(vget_low_u16(intA)));
            const float32x4_t invAHigh = reciprocal(toNormalized(vget_high_u16(intA)));

            auto unpremul = [unpremulHalf, invALow, invAHigh](uint8x8_t v) -> uint8x8_t {
                const uint16x8_t to16 = vmovl_u8(v);

                const uint16x4_t low = unpremulHalf(invALow, vget_low_u16(to16));
                const uint16x4_t high = unpremulHalf(invAHigh, vget_high_u16(to16));

                const uint16x8_t combined = vcombine_u16(low, high);
                return vqmovn_u16(combined);
            };

            const uint8x8_t b = unpremul(in.val[2]);
            const uint8x8_t g = unpremul(in.val[1]);
            const uint8x8_t r = unpremul(in.val[0]);

            if constexpr (swapRB) {
                const uint8x8x4_t out{b, g, r, a};
                vst4_u8((uint8_t*)dst, out);
            } else {
                const uint8x8x4_t out{r, g, b, a};
                vst4_u8((uint8_t*)dst, out);
            }

            src += 8;
            dst += 8;
            count -= 8;
        }
    }

    // Handle the tail. Count will be < 8.
    if constexpr (swapRB) {
        rgbA_to_BGRA_portable(dst, src, count);
    } else {
        rgbA_to_RGBA_portable(dst, src, count);
    }
}

void rgbA_to_RGBA(uint32_t* dst, const uint32_t* src, int count) {
    common_rgbA_to_RGBA</*swapRB=*/false>(dst, src, count);
}

void rgbA_to_BGRA(uint32_t* dst, const uint32_t* src, int count) {
    common_rgbA_to_RGBA</*swapRB=*/true>(dst, src, count);
}

#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
// -- AVX2 -----------------------------------------------------------------------------------------

// Scale a byte by another.
// Inputs are stored in 16-bit lanes, but are not larger than 8-bits.
static __m256i scale(__m256i x, __m256i y) {
    const __m256i _128 = _mm256_set1_epi16(128);
    const __m256i _257 = _mm256_set1_epi16(257);

    // (x+127)/255 == ((x+128)*257)>>16 for 0 <= x <= 255*255.
    return _mm256_mulhi_epu16(_mm256_add_epi16(_mm256_mullo_epi16(x, y), _128), _257);
}

static void premul_should_swapRB(bool kSwapRB, uint32_t* dst, const uint32_t* src, int count) {

    auto premul8 = [=](__m256i* lo, __m256i* hi) {
        const __m256i zeros = _mm256_setzero_si256();
        __m256i planar;
        if (kSwapRB) {
            planar = _mm256_setr_epi8(2,6,10,14, 1,5,9,13, 0,4,8,12, 3,7,11,15,
                                      2,6,10,14, 1,5,9,13, 0,4,8,12, 3,7,11,15);
        } else {
            planar = _mm256_setr_epi8(0,4,8,12, 1,5,9,13, 2,6,10,14, 3,7,11,15,
                                      0,4,8,12, 1,5,9,13, 2,6,10,14, 3,7,11,15);
        }

        // Swizzle the pixels to 8-bit planar.
        *lo = _mm256_shuffle_epi8(*lo, planar);             // rrrrgggg bbbbaaaa rrrrgggg bbbbaaaa
        *hi = _mm256_shuffle_epi8(*hi, planar);             // RRRRGGGG BBBBAAAA RRRRGGGG BBBBAAAA
        __m256i rg = _mm256_unpacklo_epi32(*lo, *hi),       // rrrrRRRR ggggGGGG rrrrRRRR ggggGGGG
                ba = _mm256_unpackhi_epi32(*lo, *hi);       // bbbbBBBB aaaaAAAA bbbbBBBB aaaaAAAA

        // Unpack to 16-bit planar.
        __m256i r = _mm256_unpacklo_epi8(rg, zeros),        // r_r_r_r_ R_R_R_R_ r_r_r_r_ R_R_R_R_
                g = _mm256_unpackhi_epi8(rg, zeros),        // g_g_g_g_ G_G_G_G_ g_g_g_g_ G_G_G_G_
                b = _mm256_unpacklo_epi8(ba, zeros),        // b_b_b_b_ B_B_B_B_ b_b_b_b_ B_B_B_B_
                a = _mm256_unpackhi_epi8(ba, zeros);        // a_a_a_a_ A_A_A_A_ a_a_a_a_ A_A_A_A_

        // Premultiply!
        r = scale(r, a);
        g = scale(g, a);
        b = scale(b, a);

        // Repack into interlaced pixels.
        rg = _mm256_or_si256(r, _mm256_slli_epi16(g, 8));   // rgrgrgrg RGRGRGRG rgrgrgrg RGRGRGRG
        ba = _mm256_or_si256(b, _mm256_slli_epi16(a, 8));   // babababa BABABABA babababa BABABABA
        *lo = _mm256_unpacklo_epi16(rg, ba);                // rgbargba rgbargba rgbargba rgbargba
        *hi = _mm256_unpackhi_epi16(rg, ba);                // RGBARGBA RGBARGBA RGBARGBA RGBARGBA
    };

    while (count >= 16) {
        __m256i lo = _mm256_loadu_si256((const __m256i*) (src + 0)),
                hi = _mm256_loadu_si256((const __m256i*) (src + 8));

        premul8(&lo, &hi);

        _mm256_storeu_si256((__m256i*) (dst + 0), lo);
        _mm256_storeu_si256((__m256i*) (dst + 8), hi);

        src += 16;
        dst += 16;
        count -= 16;
    }

    if (count >= 8) {
        __m256i lo = _mm256_loadu_si256((const __m256i*) src),
                hi = _mm256_setzero_si256();

        premul8(&lo, &hi);

        _mm256_storeu_si256((__m256i*) dst, lo);

        src += 8;
        dst += 8;
        count -= 8;
    }

    // Call portable code to finish up the tail of [0,8) pixels.
    auto proc = kSwapRB ? RGBA_to_bgrA_portable : RGBA_to_rgbA_portable;
    proc(dst, src, count);
}

void RGBA_to_rgbA(uint32_t* dst, const uint32_t* src, int count) {
    premul_should_swapRB(false, dst, src, count);
}

void RGBA_to_bgrA(uint32_t* dst, const uint32_t* src, int count) {
    premul_should_swapRB(true, dst, src, count);
}

void RGBA_to_BGRA(uint32_t* dst, const uint32_t* src, int count) {
    const __m256i swapRB = _mm256_setr_epi8(2,1,0,3, 6,5,4,7, 10,9,8,11, 14,13,12,15,
                                            2,1,0,3, 6,5,4,7, 10,9,8,11, 14,13,12,15);

    while (count >= 8) {
        __m256i rgba = _mm256_loadu_si256((const __m256i*) src);
        __m256i bgra = _mm256_shuffle_epi8(rgba, swapRB);
        _mm256_storeu_si256((__m256i*) dst, bgra);

        src += 8;
        dst += 8;
        count -= 8;
    }

    RGBA_to_BGRA_portable(dst, src, count);
}

void grayA_to_RGBA(uint32_t dst[], const uint8_t* src, int count) {
    while (count >= 16) {
        __m256i ga = _mm256_loadu_si256((const __m256i*) src);

        __m256i gg = _mm256_or_si256(_mm256_and_si256(ga, _mm256_set1_epi16(0x00FF)),
                                     _mm256_slli_epi16(ga, 8));

        __m256i ggga_lo = _mm256_unpacklo_epi16(gg, ga);
        __m256i ggga_hi = _mm256_unpackhi_epi16(gg, ga);

        // Shuffle for pixel reorder
        // Note. 'p' stands for 'ggga'
        // Before shuffle:
        // ggga_lo = p0 p1 p2 p3 | p8  p9  p10 p11
        // ggga_hi = p4 p5 p6 p7 | p12 p13 p14 p15
        //
        // After shuffle:
        // ggga_lo_shuffle = p0 p1 p2  p3  | p4  p5  p6  p7
        // ggga_hi_shuffle = p8 p9 p10 p11 | p12 p13 p14 p15
        __m256i ggga_lo_shuffle = _mm256_permute2x128_si256(ggga_lo, ggga_hi, 0x20),
                ggga_hi_shuffle = _mm256_permute2x128_si256(ggga_lo, ggga_hi, 0x31);

        _mm256_storeu_si256((__m256i*) (dst +  0), ggga_lo_shuffle);
        _mm256_storeu_si256((__m256i*) (dst +  8), ggga_hi_shuffle);

        src += 16*2;
        dst += 16;
        count -= 16;
    }

    grayA_to_RGBA_portable(dst, src, count);
}

void grayA_to_rgbA(uint32_t dst[], const uint8_t* src, int count) {
    while (count >= 16) {
        __m256i grayA = _mm256_loadu_si256((const __m256i*) src);

        __m256i g0 = _mm256_and_si256(grayA, _mm256_set1_epi16(0x00FF));
        __m256i a0 = _mm256_srli_epi16(grayA, 8);

        // Premultiply
        g0 = scale(g0, a0);

        __m256i gg = _mm256_or_si256(g0, _mm256_slli_epi16(g0, 8));
        __m256i ga = _mm256_or_si256(g0, _mm256_slli_epi16(a0, 8));

        __m256i ggga_lo = _mm256_unpacklo_epi16(gg, ga);
        __m256i ggga_hi = _mm256_unpackhi_epi16(gg, ga);

        // Shuffle for pixel reorder, similar as grayA_to_RGBA
        __m256i ggga_lo_shuffle = _mm256_permute2x128_si256(ggga_lo, ggga_hi, 0x20),
                ggga_hi_shuffle = _mm256_permute2x128_si256(ggga_lo, ggga_hi, 0x31);

        _mm256_storeu_si256((__m256i*) (dst +  0), ggga_lo_shuffle);
        _mm256_storeu_si256((__m256i*) (dst +  8), ggga_hi_shuffle);

        src += 16*2;
        dst += 16;
        count -= 16;
    }

    grayA_to_rgbA_portable(dst, src, count);
}

enum Format { kRGB1, kBGR1 };
static void inverted_cmyk_to(Format format, uint32_t* dst, const uint32_t* src, int count) {
    auto convert8 = [=](__m256i* lo, __m256i* hi) {
        const __m256i zeros = _mm256_setzero_si256();
        __m256i planar;
        if (kBGR1 == format) {
            planar = _mm256_setr_epi8(2,6,10,14, 1,5,9,13, 0,4,8,12, 3,7,11,15,
                                      2,6,10,14, 1,5,9,13, 0,4,8,12, 3,7,11,15);
        } else {
            planar = _mm256_setr_epi8(0,4,8,12, 1,5,9,13, 2,6,10,14, 3,7,11,15,
                                      0,4,8,12, 1,5,9,13, 2,6,10,14, 3,7,11,15);
        }

        // Swizzle the pixels to 8-bit planar.
        *lo = _mm256_shuffle_epi8(*lo, planar);            // ccccmmmm yyyykkkk ccccmmmm yyyykkkk
        *hi = _mm256_shuffle_epi8(*hi, planar);            // CCCCMMMM YYYYKKKK CCCCMMMM YYYYKKKK
        __m256i cm = _mm256_unpacklo_epi32(*lo, *hi),      // ccccCCCC mmmmMMMM ccccCCCC mmmmMMMM
                yk = _mm256_unpackhi_epi32(*lo, *hi);      // yyyyYYYY kkkkKKKK yyyyYYYY kkkkKKKK

        // Unpack to 16-bit planar.
        __m256i c = _mm256_unpacklo_epi8(cm, zeros),       // c_c_c_c_ C_C_C_C_ c_c_c_c_ C_C_C_C_
                m = _mm256_unpackhi_epi8(cm, zeros),       // m_m_m_m_ M_M_M_M_ m_m_m_m_ M_M_M_M_
                y = _mm256_unpacklo_epi8(yk, zeros),       // y_y_y_y_ Y_Y_Y_Y_ y_y_y_y_ Y_Y_Y_Y_
                k = _mm256_unpackhi_epi8(yk, zeros);       // k_k_k_k_ K_K_K_K_ k_k_k_k_ K_K_K_K_

        // Scale to r, g, b.
        __m256i r = scale(c, k),
                g = scale(m, k),
                b = scale(y, k);

        // Repack into interlaced pixels:
        //     rg = rgrgrgrg RGRGRGRG rgrgrgrg RGRGRGRG
        //     ba = b1b1b1b1 B1B1B1B1 b1b1b1b1 B1B1B1B1
        __m256i rg = _mm256_or_si256(r, _mm256_slli_epi16(g, 8)),
                ba = _mm256_or_si256(b, _mm256_set1_epi16((uint16_t) 0xFF00));
        *lo = _mm256_unpacklo_epi16(rg, ba);               // rgb1rgb1 rgb1rgb1 rgb1rgb1 rgb1rgb1
        *hi = _mm256_unpackhi_epi16(rg, ba);               // RGB1RGB1 RGB1RGB1 RGB1RGB1 RGB1RGB1
    };

    while (count >= 16) {
        __m256i lo = _mm256_loadu_si256((const __m256i*) (src + 0)),
                hi = _mm256_loadu_si256((const __m256i*) (src + 8));

        convert8(&lo, &hi);

        _mm256_storeu_si256((__m256i*) (dst + 0), lo);
        _mm256_storeu_si256((__m256i*) (dst + 8), hi);

        src += 16;
        dst += 16;
        count -= 16;
    }

    if (count >= 8) {
        __m256i lo = _mm256_loadu_si256((const __m256i*) src),
                hi = _mm256_setzero_si256();

        convert8(&lo, &hi);

        _mm256_storeu_si256((__m256i*) dst, lo);

        src += 8;
        dst += 8;
        count -= 8;
    }

    auto proc = (kBGR1 == format) ? inverted_CMYK_to_BGR1_portable : inverted_CMYK_to_RGB1_portable;
    proc(dst, src, count);
}

void inverted_CMYK_to_RGB1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_cmyk_to(kRGB1, dst, src, count);
}

void inverted_CMYK_to_BGR1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_cmyk_to(kBGR1, dst, src, count);
}

void rgbA_to_RGBA(uint32_t* dst, const uint32_t* src, int count) {
    rgbA_to_RGBA_portable(dst, src, count);
}

void rgbA_to_BGRA(uint32_t* dst, const uint32_t* src, int count) {
    rgbA_to_BGRA_portable(dst, src, count);
}

#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
// -- SSSE3 ----------------------------------------------------------------------------------------

// Scale a byte by another.
// Inputs are stored in 16-bit lanes, but are not larger than 8-bits.
static __m128i scale(__m128i x, __m128i y) {
    const __m128i _128 = _mm_set1_epi16(128);
    const __m128i _257 = _mm_set1_epi16(257);

    // (x+127)/255 == ((x+128)*257)>>16 for 0 <= x <= 255*255.
    return _mm_mulhi_epu16(_mm_add_epi16(_mm_mullo_epi16(x, y), _128), _257);
}

static void premul_should_swapRB(bool kSwapRB, uint32_t* dst, const uint32_t* src, int count) {

    auto premul8 = [=](__m128i* lo, __m128i* hi) {
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

void RGBA_to_rgbA(uint32_t* dst, const uint32_t* src, int count) {
    premul_should_swapRB(false, dst, src, count);
}

void RGBA_to_bgrA(uint32_t* dst, const uint32_t* src, int count) {
    premul_should_swapRB(true, dst, src, count);
}

void RGBA_to_BGRA(uint32_t* dst, const uint32_t* src, int count) {
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

void grayA_to_RGBA(uint32_t dst[], const uint8_t* src, int count) {
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

void grayA_to_rgbA(uint32_t dst[], const uint8_t* src, int count) {
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
static void inverted_cmyk_to(Format format, uint32_t* dst, const uint32_t* src, int count) {
    auto convert8 = [=](__m128i* lo, __m128i* hi) {
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

void inverted_CMYK_to_RGB1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_cmyk_to(kRGB1, dst, src, count);
}

void inverted_CMYK_to_BGR1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_cmyk_to(kBGR1, dst, src, count);
}

void rgbA_to_RGBA(uint32_t* dst, const uint32_t* src, int count) {
    rgbA_to_RGBA_portable(dst, src, count);
}

void rgbA_to_BGRA(uint32_t* dst, const uint32_t* src, int count) {
    rgbA_to_BGRA_portable(dst, src, count);
}

#elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
// -- LASX ----------------------------------------------------------------------------------------

// Scale a byte by another.
// Inputs are stored in 16-bit lanes, but are not larger than 8-bits.
// (x+127)/255 == ((x+128)*257)>>16
SI __m256i scale(__m256i x, __m256i y) {
    const __m256i _128 = __lasx_xvreplgr2vr_h(128);
    const __m256i _257 = __lasx_xvreplgr2vr_h(257);

    // (x+127)/255 == ((x+128)*257)>>16
    return __lasx_xvmuh_hu(__lasx_xvadd_h(__lasx_xvmul_h(x, y), _128), _257);
}

static void premul_should_swapRB(bool kSwapRB, uint32_t* dst, const uint32_t* src, int count) {
    auto premul8 = [=](__m256i* lo, __m256i* hi) {
        const __m256i zeros = __lasx_xvldi(0);
        __m256i planar = __lasx_xvldi(0);
        if (kSwapRB) {
            planar = __lasx_xvinsgr2vr_d(planar, 0x0d0905010e0a0602 ,0);
            planar = __lasx_xvinsgr2vr_d(planar, 0x0f0b07030c080400 ,1);
            planar = __lasx_xvinsgr2vr_d(planar, 0x0d0905010e0a0602 ,2);
            planar = __lasx_xvinsgr2vr_d(planar, 0x0f0b07030c080400 ,3);
        } else {
            planar = __lasx_xvinsgr2vr_d(planar, 0x0d0905010c080400 ,0);
            planar = __lasx_xvinsgr2vr_d(planar, 0x0f0b07030e0a0602 ,1);
            planar = __lasx_xvinsgr2vr_d(planar, 0x0d0905010c080400 ,2);
            planar = __lasx_xvinsgr2vr_d(planar, 0x0f0b07030e0a0602 ,3);
        }

        // Swizzle the pixels to 8-bit planar.
        *lo = __lasx_xvshuf_b(zeros, *lo, planar);      // rrrrgggg bbbbaaaa rrrrgggg bbbbaaaa
        *hi = __lasx_xvshuf_b(zeros, *hi, planar);      // RRRRGGGG BBBBAAAA RRRRGGGG BBBBAAAA
        __m256i rg = __lasx_xvilvl_w(*hi, *lo),         // rrrrRRRR ggggGGGG rrrrRRRR ggggGGGG
                ba = __lasx_xvilvh_w(*hi, *lo);         // bbbbBBBB aaaaAAAA bbbbBBBB aaaaAAAA

        // Unpack to 16-bit planar.
        __m256i r = __lasx_xvilvl_b(zeros, rg),         // r_r_r_r_ R_R_R_R_ r_r_r_r_ R_R_R_R_
                g = __lasx_xvilvh_b(zeros, rg),         // g_g_g_g_ G_G_G_G_ g_g_g_g_ G_G_G_G_
                b = __lasx_xvilvl_b(zeros, ba),         // b_b_b_b_ B_B_B_B_ b_b_b_b_ B_B_B_B_
                a = __lasx_xvilvh_b(zeros, ba);         // a_a_a_a_ A_A_A_A_ a_a_a_a_ A_A_A_A_

        // Premultiply!
        r = scale(r, a);
        g = scale(g, a);
        b = scale(b, a);

        // Repack into interlaced pixels.
        rg = __lasx_xvor_v(r, __lasx_xvslli_h(g, 8));   // rgrgrgrg RGRGRGRG rgrgrgrg RGRGRGRG
        ba = __lasx_xvor_v(b, __lasx_xvslli_h(a, 8));   // babababa BABABABA babababa BABABABA
        *lo = __lasx_xvilvl_h(ba, rg);                  // rgbargba rgbargba rgbargba rgbargba
        *hi = __lasx_xvilvh_h(ba, rg);                  // RGBARGBA RGBARGBA RGBARGBA RGBARGBA
    };

    while (count >= 16) {
        __m256i lo = __lasx_xvld(src, 0),
                hi = __lasx_xvld(src, 32);

        premul8(&lo, &hi);

        __lasx_xvst(lo, dst, 0);
        __lasx_xvst(hi, dst, 32);

        src += 16;
        dst += 16;
        count -= 16;
    }

    if (count >= 8) {
        __m256i lo = __lasx_xvld(src, 0),
                hi = __lasx_xvldi(0);

        premul8(&lo, &hi);

        __lasx_xvst(lo, dst, 0);

        src += 8;
        dst += 8;
        count -= 8;
    }

    // Call portable code to finish up the tail of [0,4) pixels.
    auto proc = kSwapRB ? RGBA_to_bgrA_portable : RGBA_to_rgbA_portable;
    proc(dst, src, count);
}

/*not static*/ inline void RGBA_to_rgbA(uint32_t* dst, const uint32_t* src, int count) {
    premul_should_swapRB(false, dst, src, count);
}

/*not static*/ inline void RGBA_to_bgrA(uint32_t* dst, const uint32_t* src, int count) {
    premul_should_swapRB(true, dst, src, count);
}

/*not static*/ inline void RGBA_to_BGRA(uint32_t* dst, const uint32_t* src, int count) {
    while (count >= 8) {
        __m256i rgba = __lasx_xvld(src, 0);
        __m256i bgra = __lasx_xvshuf4i_b(rgba, 0xC6);
        __lasx_xvst(bgra, dst, 0);

        src += 8;
        dst += 8;
        count -= 8;
    }

    RGBA_to_BGRA_portable(dst, src, count);
}

/*not static*/ inline void grayA_to_RGBA(uint32_t dst[], const uint8_t* src, int count) {
    while (count >= 16) {
        __m256i ga = __lasx_xvld(src, 0);

        __m256i gg = __lasx_xvor_v(__lasx_xvand_v(ga, __lasx_xvreplgr2vr_h(0x00FF)),
                                   __lasx_xvslli_h(ga, 8));

        __m256i ggga_lo = __lasx_xvilvl_h(ga, gg);
        __m256i ggga_hi = __lasx_xvilvh_h(ga, gg);

        __lasx_xvst(__lasx_xvpermi_q(ggga_lo, ggga_hi, 0x02), dst, 0);
        __lasx_xvst(__lasx_xvpermi_q(ggga_lo, ggga_hi, 0x13), dst, 32);

        src += 16*2;
        dst += 16;
        count -= 16;
    }

    grayA_to_RGBA_portable(dst, src, count);
}

/*not static*/ inline void grayA_to_rgbA(uint32_t dst[], const uint8_t* src, int count) {
    while (count >= 16) {
        __m256i grayA = __lasx_xvld(src, 0);

        __m256i val = __lasx_xvreplgr2vr_h(0x00FF);

        __m256i g0 = __lasx_xvand_v(grayA, val);
        __m256i a0 = __lasx_xvsrli_h(grayA, 8);

        // Premultiply
        g0 = scale(g0, a0);

        __m256i gg = __lasx_xvor_v(g0, __lasx_xvslli_h(g0, 8));
        __m256i ga = __lasx_xvor_v(g0, __lasx_xvslli_h(a0, 8));

        __m256i ggga_lo = __lasx_xvilvl_h(ga, gg);
        __m256i ggga_hi = __lasx_xvilvh_h(ga, gg);

        val = __lasx_xvpermi_q(ggga_lo, ggga_hi, 0x02);
        __lasx_xvst(val, dst, 0);

        val = __lasx_xvpermi_q(ggga_lo, ggga_hi, 0x13);
        __lasx_xvst(val, dst, 32);

        src += 16*2;
        dst += 16;
        count -= 16;
    }

    grayA_to_rgbA_portable(dst, src, count);
}

enum Format { kRGB1, kBGR1 };
static void inverted_cmyk_to(Format format, uint32_t* dst, const uint32_t* src, int count) {
    auto convert8 = [=](__m256i *lo, __m256i* hi) {
        const __m256i zeros = __lasx_xvldi(0);
        __m256i planar = __lasx_xvldi(0);
        if (kBGR1 == format) {
            planar = __lasx_xvinsgr2vr_d(planar, 0x0d0905010e0a0602 ,0);
            planar = __lasx_xvinsgr2vr_d(planar, 0x0f0b07030c080400 ,1);
            planar = __lasx_xvinsgr2vr_d(planar, 0x0d0905010e0a0602 ,2);
            planar = __lasx_xvinsgr2vr_d(planar, 0x0f0b07030c080400 ,3);
        } else {
            planar = __lasx_xvinsgr2vr_d(planar, 0x0d0905010c080400 ,0);
            planar = __lasx_xvinsgr2vr_d(planar, 0x0f0b07030e0a0602 ,1);
            planar = __lasx_xvinsgr2vr_d(planar, 0x0d0905010c080400 ,2);
            planar = __lasx_xvinsgr2vr_d(planar, 0x0f0b07030e0a0602 ,3);
        }

        // Swizzle the pixels to 8-bit planar.
        *lo = __lasx_xvshuf_b(zeros, *lo, planar);   // ccccmmmm yyyykkkk ccccmmmm yyyykkkk
        *hi = __lasx_xvshuf_b(zeros, *hi, planar);   // CCCCMMMM YYYYKKKK CCCCMMMM YYYYKKKK
        __m256i cm = __lasx_xvilvl_w(*hi, *lo),      // ccccCCCC mmmmMMMM ccccCCCC mmmmMMMM
                yk = __lasx_xvilvh_w(*hi, *lo);      // yyyyYYYY kkkkKKKK yyyyYYYY kkkkKKKK

        // Unpack to 16-bit planar.
        __m256i c = __lasx_xvilvl_b(zeros, cm),      // c_c_c_c_ C_C_C_C_ c_c_c_c_ C_C_C_C_
                m = __lasx_xvilvh_b(zeros, cm),      // m_m_m_m_ M_M_M_M_ m_m_m_m_ M_M_M_M_
                y = __lasx_xvilvl_b(zeros, yk),      // y_y_y_y_ Y_Y_Y_Y_ y_y_y_y_ Y_Y_Y_Y_
                k = __lasx_xvilvh_b(zeros, yk);      // k_k_k_k_ K_K_K_K_ k_k_k_k_ K_K_K_K_

        // Scale to r, g, b.
        __m256i r = scale(c, k),
                g = scale(m, k),
                b = scale(y, k);

        // Repack into interlaced pixels:
        //     rg = rgrgrgrg RGRGRGRG rgrgrgrg RGRGRGRG
        //     ba = b1b1b1b1 B1B1B1B1 b1b1b1b1 B1B1B1B1
        __m256i rg = __lasx_xvor_v(r, __lasx_xvslli_h(g, 8)),
                ba = __lasx_xvor_v(b, __lasx_xvreplgr2vr_h(0xff00));
        *lo = __lasx_xvilvl_h(ba, rg);               // rgb1rgb1 rgb1rgb1 rgb1rgb1 rgb1rgb1
        *hi = __lasx_xvilvh_h(ba, rg);               // RGB1RGB1 RGB1RGB1 RGB1RGB1 RGB1RGB1
    };

    while (count >= 16) {
        __m256i lo = __lasx_xvld(src, 0),
                hi = __lasx_xvld(src, 32);

        convert8(&lo, &hi);

        __lasx_xvst(lo, dst, 0);
        __lasx_xvst(hi, dst, 32);

        src += 16;
        dst += 16;
        count -= 16;
    }

    while (count >= 8) {
        __m256i lo = __lasx_xvld(src, 0),
                hi = __lasx_xvldi(0);

        convert8(&lo, &hi);

        __lasx_xvst(lo, dst, 0);

        src += 8;
        dst += 8;
        count -= 8;
    }

    auto proc = (kBGR1 == format) ? inverted_CMYK_to_BGR1_portable : inverted_CMYK_to_RGB1_portable;
    proc(dst, src, count);
}

/*not static*/ inline void inverted_CMYK_to_RGB1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_cmyk_to(kRGB1, dst, src, count);
}

/*not static*/ inline void inverted_CMYK_to_BGR1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_cmyk_to(kBGR1, dst, src, count);
}

/*not static*/ inline void rgbA_to_RGBA(uint32_t* dst, const uint32_t* src, int count) {
    rgbA_to_RGBA_portable(dst, src, count);
}

/*not static*/ inline void rgbA_to_BGRA(uint32_t* dst, const uint32_t* src, int count) {
    rgbA_to_BGRA_portable(dst, src, count);
}

#elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
// -- LSX -----------------------------------------------------------------------------------------

// Scale a byte by another.
// Inputs are stored in 16-bit lanes, but are not larger than 8-bits.
SI __m128i scale(__m128i x, __m128i y) {
    const __m128i _128 = __lsx_vreplgr2vr_h(128);
    const __m128i _257 = __lsx_vreplgr2vr_h(257);

    // (x+127)/255 == ((x+128)*257)>>16
    return __lsx_vmuh_hu(__lsx_vadd_h(__lsx_vmul_h(x, y), _128), _257);
}

static void premul_should_swapRB(bool kSwapRB, uint32_t* dst, const uint32_t* src, int count) {

    auto premul8 = [=](__m128i *lo, __m128i *hi){
        const __m128i zeros = __lsx_vldi(0);
        __m128i planar = __lsx_vldi(0);
        if (kSwapRB) {
            planar = __lsx_vinsgr2vr_d(planar, 0x0d0905010e0a0602, 0);
            planar = __lsx_vinsgr2vr_d(planar, 0x0f0b07030c080400, 1);
        } else {
            planar = __lsx_vinsgr2vr_d(planar, 0x0d0905010c080400, 0);
            planar = __lsx_vinsgr2vr_d(planar, 0x0f0b07030e0a0602, 1);
        }

        // Swizzle the pixels to 8-bit planar.
        *lo = __lsx_vshuf_b(zeros, *lo, planar);             // rrrrgggg bbbbaaaa
        *hi = __lsx_vshuf_b(zeros, *hi, planar);             // RRRRGGGG BBBBAAAA
        __m128i rg = __lsx_vilvl_w(*hi, *lo),                // rrrrRRRR ggggGGGG
                ba = __lsx_vilvh_w(*hi, *lo);                // bbbbBBBB aaaaAAAA

        // Unpack to 16-bit planar.
        __m128i r = __lsx_vilvl_b(zeros, rg),                 // r_r_r_r_ R_R_R_R_
                g = __lsx_vilvh_b(zeros, rg),                 // g_g_g_g_ G_G_G_G_
                b = __lsx_vilvl_b(zeros, ba),                 // b_b_b_b_ B_B_B_B_
                a = __lsx_vilvh_b(zeros, ba);                 // a_a_a_a_ A_A_A_A_

        // Premultiply!
        r = scale(r, a);
        g = scale(g, a);
        b = scale(b, a);

        // Repack into interlaced pixels.
        rg = __lsx_vor_v(r, __lsx_vslli_h(g, 8));             // rgrgrgrg RGRGRGRG
        ba = __lsx_vor_v(b, __lsx_vslli_h(a, 8));             // babababa BABABABA
        *lo = __lsx_vilvl_h(ba, rg);                          // rgbargba rgbargba
        *hi = __lsx_vilvh_h(ba, rg);                          // RGBARGBA RGBARGBA
    };
    while (count >= 8) {
        __m128i lo = __lsx_vld(src ,0),
                hi = __lsx_vld(src ,16);

        premul8(&lo, &hi);

        __lsx_vst(lo, dst, 0);
        __lsx_vst(hi, dst, 16);

        src += 8;
        dst += 8;
        count -= 8;
    }

    if (count >= 4) {
        __m128i lo = __lsx_vld(src, 0),
                hi = __lsx_vldi(0);

        premul8(&lo, &hi);

        __lsx_vst(lo, dst, 0);

        src += 4;
        dst += 4;
        count -= 4;
    }

    // Call portable code to finish up the tail of [0,4) pixels.
    auto proc = kSwapRB ? RGBA_to_bgrA_portable : RGBA_to_rgbA_portable;
    proc(dst, src, count);
}

/*not static*/ inline void RGBA_to_rgbA(uint32_t* dst, const uint32_t* src, int count) {
    premul_should_swapRB(false, dst, src, count);
}

/*not static*/ inline void RGBA_to_bgrA(uint32_t* dst, const uint32_t* src, int count) {
    premul_should_swapRB(true, dst, src, count);
}

/*not static*/ inline void RGBA_to_BGRA(uint32_t* dst, const uint32_t* src, int count) {
    __m128i swapRB = __lsx_vldi(0);
    swapRB = __lsx_vinsgr2vr_d(swapRB, 0x0704050603000102, 0);
    swapRB = __lsx_vinsgr2vr_d(swapRB, 0x0f0c0d0e0b08090a, 1);

    while (count >= 4) {
        __m128i rgba = __lsx_vld(src, 0);
        __m128i bgra = __lsx_vshuf4i_b(rgba, 0xC6);
        __lsx_vst(bgra, dst, 0);

        src += 4;
        dst += 4;
        count -= 4;
    }

    RGBA_to_BGRA_portable(dst, src, count);
}

/*not static*/ inline void grayA_to_RGBA(uint32_t dst[], const uint8_t* src, int count) {
    while (count >= 8) {
        __m128i ga = __lsx_vld(src, 0);

        __m128i gg = __lsx_vor_v(__lsx_vand_v(ga, __lsx_vreplgr2vr_h(0x00FF)),
                                 __lsx_vslli_h(ga, 8));

        __m128i ggga_lo = __lsx_vilvl_h(ga, gg);
        __m128i ggga_hi = __lsx_vilvh_h(ga, gg);

        __lsx_vst(ggga_lo, dst, 0);
        __lsx_vst(ggga_hi, dst, 16);

        src += 8*2;
        dst += 8;
        count -= 8;
    }

    grayA_to_RGBA_portable(dst, src, count);
}

/*not static*/ inline void grayA_to_rgbA(uint32_t dst[], const uint8_t* src, int count) {
    while (count >= 8) {
        __m128i grayA = __lsx_vld(src, 0);

        __m128i g0 = __lsx_vand_v(grayA, __lsx_vreplgr2vr_h(0x00FF));
        __m128i a0 = __lsx_vsrli_h(grayA, 8);

        // Premultiply
        g0 = scale(g0, a0);

        __m128i gg = __lsx_vor_v(g0, __lsx_vslli_h(g0, 8));
        __m128i ga = __lsx_vor_v(g0, __lsx_vslli_h(a0, 8));

        __m128i ggga_lo = __lsx_vilvl_h(ga, gg);
        __m128i ggga_hi = __lsx_vilvh_h(ga, gg);

        __lsx_vst(ggga_lo, dst, 0);
        __lsx_vst(ggga_hi, dst, 16);

        src += 8*2;
        dst += 8;
        count -= 8;
    }

    grayA_to_rgbA_portable(dst, src, count);
}

enum Format { kRGB1, kBGR1 };
static void inverted_cmyk_to(Format format, uint32_t* dst, const uint32_t* src, int count) {
    auto convert8 = [=](__m128i *lo, __m128i* hi) {
        const __m128i zeros = __lsx_vldi(0);
        __m128i planar = __lsx_vldi(0);
        if (kBGR1 == format) {
            planar = __lsx_vinsgr2vr_d(planar, 0x0d0905010e0a0602, 0);
            planar = __lsx_vinsgr2vr_d(planar, 0x0f0b07030c080400, 1);
        } else {
            planar = __lsx_vinsgr2vr_d(planar, 0x0d0905010c080400, 0);
            planar = __lsx_vinsgr2vr_d(planar, 0x0f0b07030e0a0602, 1);
        }

        // Swizzle the pixels to 8-bit planar.
        *lo = __lsx_vshuf_b(zeros, *lo, planar);              // ccccmmmm yyyykkkk
        *hi = __lsx_vshuf_b(zeros, *hi, planar);              // CCCCMMMM YYYYKKKK
        __m128i cm = __lsx_vilvl_w(*hi, *lo),                 // ccccCCCC mmmmMMMM
                yk = __lsx_vilvh_w(*hi, *lo);                 // yyyyYYYY kkkkKKKK

        // Unpack to 16-bit planar.
        __m128i c = __lsx_vilvl_b(zeros, cm),                 // c_c_c_c_ C_C_C_C_
                m = __lsx_vilvh_b(zeros, cm),                 // m_m_m_m_ M_M_M_M_
                y = __lsx_vilvl_b(zeros, yk),                 // y_y_y_y_ Y_Y_Y_Y_
                k = __lsx_vilvh_b(zeros, yk);                 // k_k_k_k_ K_K_K_K_

        // Scale to r, g, b.
        __m128i r = scale(c, k),
                g = scale(m, k),
                b = scale(y, k);

        // Repack into interlaced pixels.
        // rgrgrgrg RGRGRGRG
        // b1b1b1b1 B1B1B1B1
        __m128i rg = __lsx_vor_v(r, __lsx_vslli_h(g, 8)),
                ba = __lsx_vor_v(b, __lsx_vreplgr2vr_h(0xff00));
        *lo = __lsx_vilvl_h(ba, rg);                          // rgbargba rgbargba
        *hi = __lsx_vilvl_h(ba, rg);                          // RGB1RGB1 RGB1RGB1
    };

    while (count >= 8) {
        __m128i lo = __lsx_vld(src, 0),
                hi = __lsx_vld(src, 16);

        convert8(&lo, &hi);

        __lsx_vst(lo, dst, 0);
        __lsx_vst(hi, dst, 16);

        src += 8;
        dst += 8;
        count -= 8;
    }

    if (count >= 4) {
        __m128i lo = __lsx_vld(src, 0),
                hi = __lsx_vldi(0);

        convert8(&lo, &hi);

        __lsx_vst(lo, dst, 0);

        src += 4;
        dst += 4;
        count -= 4;
    }

    auto proc = (kBGR1 == format) ? inverted_CMYK_to_BGR1_portable : inverted_CMYK_to_RGB1_portable;
    proc(dst, src, count);
}

/*not static*/ inline void inverted_CMYK_to_RGB1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_cmyk_to(kRGB1, dst, src, count);
}

/*not static*/ inline void inverted_CMYK_to_BGR1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_cmyk_to(kBGR1, dst, src, count);
}

/*not static*/ inline void rgbA_to_RGBA(uint32_t* dst, const uint32_t* src, int count) {
    rgbA_to_RGBA_portable(dst, src, count);
}

/*not static*/ inline void rgbA_to_BGRA(uint32_t* dst, const uint32_t* src, int count) {
    rgbA_to_BGRA_portable(dst, src, count);
}

#else
// -- No Opts --------------------------------------------------------------------------------------

void rgbA_to_RGBA(uint32_t* dst, const uint32_t* src, int count) {
    rgbA_to_RGBA_portable(dst, src, count);
}

void rgbA_to_BGRA(uint32_t* dst, const uint32_t* src, int count) {
    rgbA_to_BGRA_portable(dst, src, count);
}

void RGBA_to_rgbA(uint32_t* dst, const uint32_t* src, int count) {
    RGBA_to_rgbA_portable(dst, src, count);
}

void RGBA_to_bgrA(uint32_t* dst, const uint32_t* src, int count) {
    RGBA_to_bgrA_portable(dst, src, count);
}

void RGBA_to_BGRA(uint32_t* dst, const uint32_t* src, int count) {
    RGBA_to_BGRA_portable(dst, src, count);
}

void grayA_to_RGBA(uint32_t dst[], const uint8_t* src, int count) {
    grayA_to_RGBA_portable(dst, src, count);
}

void grayA_to_rgbA(uint32_t dst[], const uint8_t* src, int count) {
    grayA_to_rgbA_portable(dst, src, count);
}

void inverted_CMYK_to_RGB1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_CMYK_to_RGB1_portable(dst, src, count);
}

void inverted_CMYK_to_BGR1(uint32_t dst[], const uint32_t* src, int count) {
    inverted_CMYK_to_BGR1_portable(dst, src, count);
}
#endif

// Basically as above, but we found no benefit from AVX-512 for gray_to_RGB1.
static void gray_to_RGB1_portable(uint32_t dst[], const uint8_t* src, int count) {
    for (int i = 0; i < count; i++) {
        dst[i] = (uint32_t)0xFF   << 24
               | (uint32_t)src[i] << 16
               | (uint32_t)src[i] <<  8
               | (uint32_t)src[i] <<  0;
    }
}
#if defined(SK_ARM_HAS_NEON)
    void gray_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
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
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
    void gray_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
        const __m256i alphas = _mm256_set1_epi8((uint8_t) 0xFF);
        while (count >= 32) {
            __m256i grays = _mm256_loadu_si256((const __m256i*) src);

            __m256i gg_lo = _mm256_unpacklo_epi8(grays, grays);
            __m256i gg_hi = _mm256_unpackhi_epi8(grays, grays);
            __m256i ga_lo = _mm256_unpacklo_epi8(grays, alphas);
            __m256i ga_hi = _mm256_unpackhi_epi8(grays, alphas);

            __m256i ggga0 = _mm256_unpacklo_epi16(gg_lo, ga_lo);
            __m256i ggga1 = _mm256_unpackhi_epi16(gg_lo, ga_lo);
            __m256i ggga2 = _mm256_unpacklo_epi16(gg_hi, ga_hi);
            __m256i ggga3 = _mm256_unpackhi_epi16(gg_hi, ga_hi);

            // Shuffle for pixel reorder.
            // Note. 'p' stands for 'ggga'
            // Before shuffle:
            //     ggga0 = p0  p1  p2  p3  | p16 p17 p18 p19
            //     ggga1 = p4  p5  p6  p7  | p20 p21 p22 p23
            //     ggga2 = p8  p9  p10 p11 | p24 p25 p26 p27
            //     ggga3 = p12 p13 p14 p15 | p28 p29 p30 p31
            //
            // After shuffle:
            //     ggga0_shuffle = p0  p1  p2  p3  | p4  p5  p6  p7
            //     ggga1_shuffle = p8  p9  p10 p11 | p12 p13 p14 p15
            //     ggga2_shuffle = p16 p17 p18 p19 | p20 p21 p22 p23
            //     ggga3_shuffle = p24 p25 p26 p27 | p28 p29 p30 p31
            __m256i ggga0_shuffle = _mm256_permute2x128_si256(ggga0, ggga1, 0x20),
                    ggga1_shuffle = _mm256_permute2x128_si256(ggga2, ggga3, 0x20),
                    ggga2_shuffle = _mm256_permute2x128_si256(ggga0, ggga1, 0x31),
                    ggga3_shuffle = _mm256_permute2x128_si256(ggga2, ggga3, 0x31);

            _mm256_storeu_si256((__m256i*) (dst +  0), ggga0_shuffle);
            _mm256_storeu_si256((__m256i*) (dst +  8), ggga1_shuffle);
            _mm256_storeu_si256((__m256i*) (dst + 16), ggga2_shuffle);
            _mm256_storeu_si256((__m256i*) (dst + 24), ggga3_shuffle);

            src += 32;
            dst += 32;
            count -= 32;
        }
        gray_to_RGB1_portable(dst, src, count);
    }
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3  // TODO: just check >= SSE2?
    void gray_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
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
#elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    /*not static*/ inline void gray_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
        const __m256i alphas = __lasx_xvreplgr2vr_b(0xFF);
        while (count >= 32) {
            __m256i grays = __lasx_xvld(src, 0);

            __m256i gg_lo = __lasx_xvilvl_b(grays, grays);
            __m256i gg_hi = __lasx_xvilvh_b(grays, grays);
            __m256i ga_lo = __lasx_xvilvl_b(alphas, grays);
            __m256i ga_hi = __lasx_xvilvh_b(alphas, grays);

            __m256i ggga0 = __lasx_xvilvl_h(ga_lo, gg_lo);
            __m256i ggga1 = __lasx_xvilvh_h(ga_lo, gg_lo);
            __m256i ggga2 = __lasx_xvilvl_h(ga_hi, gg_hi);
            __m256i ggga3 = __lasx_xvilvh_h(ga_hi, gg_hi);

            __m256i ggga_0 = __lasx_xvpermi_q(ggga0, ggga1, 0x02);
            __m256i ggga_1 = __lasx_xvpermi_q(ggga2, ggga3, 0x02);
            __m256i ggga_2 = __lasx_xvpermi_q(ggga0, ggga1, 0x13);
            __m256i ggga_3 = __lasx_xvpermi_q(ggga2, ggga3, 0x13);

            __lasx_xvst(ggga_0, dst,  0);
            __lasx_xvst(ggga_1, dst, 32);
            __lasx_xvst(ggga_2, dst, 64);
            __lasx_xvst(ggga_3, dst, 96);

            src += 32;
            dst += 32;
            count -= 32;
        }
        gray_to_RGB1_portable(dst, src, count);
    }
#elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    /*not static*/ inline void gray_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
        const __m128i alphas = __lsx_vreplgr2vr_b(0xFF);
        while (count >= 16) {
            __m128i grays = __lsx_vld(src, 0);

            __m128i gg_lo = __lsx_vilvl_b(grays, grays);
            __m128i gg_hi = __lsx_vilvh_b(grays, grays);
            __m128i ga_lo = __lsx_vilvl_b(alphas, grays);
            __m128i ga_hi = __lsx_vilvh_b(alphas, grays);

            __m128i ggga0 = __lsx_vilvl_h(ga_lo, gg_lo);
            __m128i ggga1 = __lsx_vilvh_h(ga_lo, gg_lo);
            __m128i ggga2 = __lsx_vilvl_h(ga_hi, gg_hi);
            __m128i ggga3 = __lsx_vilvh_h(ga_hi, gg_hi);

            __lsx_vst(ggga0, dst,  0);
            __lsx_vst(ggga1, dst, 16);
            __lsx_vst(ggga2, dst, 32);
            __lsx_vst(ggga3, dst, 48);

            src += 16;
            dst += 16;
            count -= 16;
        }
        gray_to_RGB1_portable(dst, src, count);
    }
#else
    void gray_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
        gray_to_RGB1_portable(dst, src, count);
    }
#endif

// Again as above, this time not even finding benefit from AVX2 for RGB_to_{RGB,BGR}1.
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
#if defined(SK_ARM_HAS_NEON)
    static void insert_alpha_should_swaprb(bool kSwapRB,
                                           uint32_t dst[], const uint8_t* src, int count) {
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

    void RGB_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
        insert_alpha_should_swaprb(false, dst, src, count);
    }
    void RGB_to_BGR1(uint32_t dst[], const uint8_t* src, int count) {
        insert_alpha_should_swaprb(true, dst, src, count);
    }
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
    static void insert_alpha_should_swaprb(bool kSwapRB,
                                           uint32_t dst[], const uint8_t* src, int count) {
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

    void RGB_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
        insert_alpha_should_swaprb(false, dst, src, count);
    }
    void RGB_to_BGR1(uint32_t dst[], const uint8_t* src, int count) {
        insert_alpha_should_swaprb(true, dst, src, count);
    }
#elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    static void insert_alpha_should_swaprb(bool kSwapRB,
                                           uint32_t dst[], const uint8_t* src, int count) {
        const __m256i alphaMask = __lasx_xvreplgr2vr_w(0xFF000000);

        __m256i expand = __lasx_xvldi(0);
        if (kSwapRB) {
            expand = __lasx_xvinsgr2vr_d(expand, 0x0503040502000102, 0);
            expand = __lasx_xvinsgr2vr_d(expand, 0x0b090a0b08060708, 1);
            expand = __lasx_xvinsgr2vr_d(expand, 0x110f10110e0c0d0e, 2);
            expand = __lasx_xvinsgr2vr_d(expand, 0x1715161714121314, 3);
        } else {
            expand = __lasx_xvinsgr2vr_d(expand, 0x0505040302020100, 0);
            expand = __lasx_xvinsgr2vr_d(expand, 0x0b0b0a0908080706, 1);
            expand = __lasx_xvinsgr2vr_d(expand, 0x1111100f0e0e0d0c, 2);
            expand = __lasx_xvinsgr2vr_d(expand, 0x1717161514141312, 3);
        }

        while (count >= 8) {
            // Load a vector.  While this actually contains 5 pixels plus an
            // extra component, we will discard all but the first four pixels on
            // this iteration.
            __m256i rgb = __lasx_xvld(src, 0);
            __m256i rgb_l = __lasx_xvpermi_d(rgb, 0x44);
            __m256i rgb_h = __lasx_xvpermi_d(rgb, 0xEE);

            // Expand the first four pixels to RGBX and then mask to RGB(FF).
            __m256i rgba = __lasx_xvor_v(__lasx_xvshuf_b(rgb_h, rgb_l, expand), alphaMask);

            // Store 8 pixels.
            __lasx_xvst(rgba, dst, 0);

            src += 4*6;
            dst += 8;
            count -= 8;
        }

        // Call portable code to finish up the tail of [0,4) pixels.
        auto proc = kSwapRB ? RGB_to_BGR1_portable : RGB_to_RGB1_portable;
        proc(dst, src, count);
    }
    /*not static*/ inline void RGB_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
        insert_alpha_should_swaprb(false, dst, src, count);
    }
    /*not static*/ inline void RGB_to_BGR1(uint32_t dst[], const uint8_t* src, int count) {
        insert_alpha_should_swaprb(true, dst, src, count);
    }
#elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    static void insert_alpha_should_swaprb(bool kSwapRB,
                                           uint32_t dst[], const uint8_t* src, int count) {
        const __m128i alphaMask = __lsx_vreplgr2vr_w(0xFF000000);

        __m128i expand = __lsx_vldi(0);
        if (kSwapRB) {
            expand = __lsx_vinsgr2vr_d(expand, 0x0503040502000102, 0);
            expand = __lsx_vinsgr2vr_d(expand, 0x0b090a0b08060708, 1);
        } else {
            expand = __lsx_vinsgr2vr_d(expand, 0x0505040302020100, 0);
            expand = __lsx_vinsgr2vr_d(expand, 0x0b0b0a0908080706, 1);
        }

        while (count >= 6) {
            // Load a vector.  While this actually contains 5 pixels plus an
            // extra component, we will discard all but the first four pixels on
            // this iteration.
            __m128i rgb = __lsx_vld(src, 0);

            // Expand the first four pixels to RGBX and then mask to RGB(FF).
            __m128i rgba = __lsx_vor_v(__lsx_vshuf_b(rgb, rgb, expand), alphaMask);

            // Store 4 pixels.
            __lsx_vst(rgba, dst, 0);

            src += 4*3;
            dst += 4;
            count -= 4;
        }

        // Call portable code to finish up the tail of [0,4) pixels.
        auto proc = kSwapRB ? RGB_to_BGR1_portable : RGB_to_RGB1_portable;
        proc(dst, src, count);
    }
    /*not static*/ inline void RGB_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
        insert_alpha_should_swaprb(false, dst, src, count);
    }
    /*not static*/ inline void RGB_to_BGR1(uint32_t dst[], const uint8_t* src, int count) {
        insert_alpha_should_swaprb(true, dst, src, count);
    }
#else
    void RGB_to_RGB1(uint32_t dst[], const uint8_t* src, int count) {
        RGB_to_RGB1_portable(dst, src, count);
    }
    void RGB_to_BGR1(uint32_t dst[], const uint8_t* src, int count) {
        RGB_to_BGR1_portable(dst, src, count);
    }
#endif

}  // namespace SK_OPTS_NS

#undef SI
