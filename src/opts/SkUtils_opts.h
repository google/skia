/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUtils_opts_DEFINED
#define SkUtils_opts_DEFINED

namespace SK_OPTS_NS {

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2

static void memset16(uint16_t* dst, uint16_t val, int n) {
    auto dst8 = (__m128i*)dst;
    auto val8 = _mm_set1_epi16(val);
    for ( ; n >= 8; n -= 8) {
        _mm_storeu_si128(dst8++, val8);
    }
    dst = (uint16_t*)dst8;
    if (n & 4) {
        _mm_storel_epi64((__m128i*)dst, val8);
        dst += 4;
    }
    if (n & 2) {
        *(uint32_t*)dst = _mm_cvtsi128_si32(val8);
        dst += 2;
    }
    if (n & 1) {
        *dst = val;
    }
}

static void memset32(uint32_t* dst, uint32_t val, int n) {
    auto dst4 = (__m128i*)dst;
    auto val4 = _mm_set1_epi32(val);
    for ( ; n >= 4; n -= 4) {
        _mm_storeu_si128(dst4++, val4);
    }
    dst = (uint32_t*)dst4;
    if (n & 2) {
        _mm_storel_epi64((__m128i*)dst, val4);
        dst += 2;
    }
    if (n & 1) {
        *dst = val;
    }
}

#elif defined(SK_ARM_HAS_NEON)

static void memset16(uint16_t* dst, uint16_t value, int n) {
    uint16x8_t   v8  = vdupq_n_u16(value);
    uint16x8x4_t v32 = {{ v8, v8, v8, v8 }};

    while (n >= 32) {
        vst4q_u16(dst, v32);  // This swizzles, but we don't care: all lanes are the same, value.
        dst += 32;
        n   -= 32;
    }
    switch (n / 8) {
        case 3: vst1q_u16(dst, v8); dst += 8;
        case 2: vst1q_u16(dst, v8); dst += 8;
        case 1: vst1q_u16(dst, v8); dst += 8;
    }
    if (n & 4) {
        vst1_u16(dst, vget_low_u16(v8));
        dst += 4;
    }
    switch (n & 3) {
        case 3: *dst++ = value;
        case 2: *dst++ = value;
        case 1: *dst   = value;
    }
}

static void memset32(uint32_t* dst, uint32_t value, int n) {
    uint32x4_t   v4  = vdupq_n_u32(value);
    uint32x4x4_t v16 = {{ v4, v4, v4, v4 }};

    while (n >= 16) {
        vst4q_u32(dst, v16);  // This swizzles, but we don't care: all lanes are the same, value.
        dst += 16;
        n   -= 16;
    }
    switch (n / 4) {
        case 3: vst1q_u32(dst, v4); dst += 4;
        case 2: vst1q_u32(dst, v4); dst += 4;
        case 1: vst1q_u32(dst, v4); dst += 4;
    }
    if (n & 2) {
        vst1_u32(dst, vget_low_u32(v4));
        dst += 2;
    }
    if (n & 1) {
        *dst = value;
    }
}

#else // Neither NEON nor SSE2.

static void memset16(uint16_t* dst, uint16_t val, int n) { while (n --> 0) { *dst++ = val; } }
static void memset32(uint32_t* dst, uint32_t val, int n) { while (n --> 0) { *dst++ = val; } }

#endif

}  // namespace SK_OPTS_NS

#endif//SkUtils_opts_DEFINED
