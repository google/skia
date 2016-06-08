/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorXform_opts_DEFINED
#define SkColorXform_opts_DEFINED

#include "SkColorPriv.h"

namespace SK_OPTS_NS {

static uint8_t clamp_float_to_byte(float v) {
    if (v >= 254.5f) {
        return 255;
    } else if (v < 0.5f) {
        return 0;
    } else {
        return (uint8_t) (v + 0.5f);
    }
}

static void color_xform_2Dot2_RGBA_to_8888_portable(uint32_t* dst, const uint32_t* src, int len,
                                                    const float matrix[16]) {
    while (len-- > 0) {
        float srcFloats[3];
        srcFloats[0] = (float) ((*src >>  0) & 0xFF);
        srcFloats[1] = (float) ((*src >>  8) & 0xFF);
        srcFloats[2] = (float) ((*src >> 16) & 0xFF);

        // Convert to linear.
        // TODO (msarett):
        // We should use X^2.2 here instead of X^2.  What is the impact on correctness?
        // We should be able to get closer to 2.2 at a small performance cost.
        srcFloats[0] = srcFloats[0] * srcFloats[0];
        srcFloats[1] = srcFloats[1] * srcFloats[1];
        srcFloats[2] = srcFloats[2] * srcFloats[2];

        // Convert to dst gamut.
        float dstFloats[3];
        // TODO (msarett): matrix[12], matrix[13], and matrix[14] are almost always zero.
        // Should we have another optimized path that avoids the extra addition when they
        // are zero?
        dstFloats[0] = srcFloats[0] * matrix[0] + srcFloats[1] * matrix[4] +
                       srcFloats[2] * matrix[8] + matrix[12];
        dstFloats[1] = srcFloats[0] * matrix[1] + srcFloats[1] * matrix[5] +
                       srcFloats[2] * matrix[9] + matrix[13];
        dstFloats[2] = srcFloats[0] * matrix[2] + srcFloats[1] * matrix[6] +
                       srcFloats[2] * matrix[10] + matrix[14];

        // Convert to dst gamma.
        // TODO (msarett):
        // We should use X^(1/2.2) here instead of X^(1/2).  What is the impact on correctness?
        // We should be able to get closer to (1/2.2) at a small performance cost.
        dstFloats[0] = sqrtf(dstFloats[0]);
        dstFloats[1] = sqrtf(dstFloats[1]);
        dstFloats[2] = sqrtf(dstFloats[2]);

        *dst = SkPackARGB32NoCheck(((*src >> 24) & 0xFF),
                                   clamp_float_to_byte(dstFloats[0]),
                                   clamp_float_to_byte(dstFloats[1]),
                                   clamp_float_to_byte(dstFloats[2]));

        dst++;
        src++;
    }
}

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2

static void color_xform_2Dot2_RGBA_to_8888(uint32_t* dst, const uint32_t* src, int len,
                                           const float matrix[16]) {
    // Load transformation matrix.
    __m128 rXgXbX = _mm_loadu_ps(&matrix[0]);
    __m128 rYgYbY = _mm_loadu_ps(&matrix[4]);
    __m128 rZgZbZ = _mm_loadu_ps(&matrix[8]);
    __m128 rQgQbQ = _mm_loadu_ps(&matrix[12]);

    while (len >= 4) {
        // Load 4 pixels and convert them to floats.
        __m128i rgba = _mm_loadu_si128((const __m128i*) src);
        __m128i byteMask = _mm_set1_epi32(0xFF);
        __m128 reds   = _mm_cvtepi32_ps(_mm_and_si128(               rgba,      byteMask));
        __m128 greens = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(rgba,  8), byteMask));
        __m128 blues  = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(rgba, 16), byteMask));

        // Convert to linear.
        // FIXME (msarett):
        // Should we be more accurate?
        reds   = _mm_mul_ps(reds, reds);
        greens = _mm_mul_ps(greens, greens);
        blues  = _mm_mul_ps(blues, blues);

        // Apply the transformation matrix to dst gamut.
        // FIXME (msarett):
        // rQ, gQ, and bQ are almost always zero.  Can we save a couple instructions?

        // Splat rX, rY, rZ, and rQ each across a register.
        __m128 rX = _mm_shuffle_ps(rXgXbX, rXgXbX, 0x00);
        __m128 rY = _mm_shuffle_ps(rYgYbY, rYgYbY, 0x00);
        __m128 rZ = _mm_shuffle_ps(rZgZbZ, rZgZbZ, 0x00);
        __m128 rQ = _mm_shuffle_ps(rQgQbQ, rQgQbQ, 0x00);

        // dstReds = rX * reds + rY * greens + rZ * blues + rQ
        __m128 dstReds =                     _mm_mul_ps(reds,   rX);
               dstReds = _mm_add_ps(dstReds, _mm_mul_ps(greens, rY));
               dstReds = _mm_add_ps(dstReds, _mm_mul_ps(blues,  rZ));
               dstReds = _mm_add_ps(dstReds,                    rQ);

        // Splat gX, gY, gZ, and gQ each across a register.
        __m128 gX = _mm_shuffle_ps(rXgXbX, rXgXbX, 0x55);
        __m128 gY = _mm_shuffle_ps(rYgYbY, rYgYbY, 0x55);
        __m128 gZ = _mm_shuffle_ps(rZgZbZ, rZgZbZ, 0x55);
        __m128 gQ = _mm_shuffle_ps(rQgQbQ, rQgQbQ, 0x55);

        // dstGreens = gX * reds + gY * greens + gZ * blues + gQ
        __m128 dstGreens =                       _mm_mul_ps(reds,   gX);
               dstGreens = _mm_add_ps(dstGreens, _mm_mul_ps(greens, gY));
               dstGreens = _mm_add_ps(dstGreens, _mm_mul_ps(blues,  gZ));
               dstGreens = _mm_add_ps(dstGreens,                    gQ);

        // Splat bX, bY, bZ, and bQ each across a register.
        __m128 bX = _mm_shuffle_ps(rXgXbX, rXgXbX, 0xAA);
        __m128 bY = _mm_shuffle_ps(rYgYbY, rYgYbY, 0xAA);
        __m128 bZ = _mm_shuffle_ps(rZgZbZ, rZgZbZ, 0xAA);
        __m128 bQ = _mm_shuffle_ps(rQgQbQ, rQgQbQ, 0xAA);

        // dstBlues = bX * reds + bY * greens + bZ * blues + bQ
        __m128 dstBlues =                      _mm_mul_ps(reds,   bX);
               dstBlues = _mm_add_ps(dstBlues, _mm_mul_ps(greens, bY));
               dstBlues = _mm_add_ps(dstBlues, _mm_mul_ps(blues,  bZ));
               dstBlues = _mm_add_ps(dstBlues,                    bQ);

        // Convert to dst gamma.
        // Note that the reciprocal of the reciprocal sqrt, is just a fast sqrt.
        // FIXME (msarett):
        // Should we be more accurate?
        dstReds   = _mm_rcp_ps(_mm_rsqrt_ps(dstReds));
        dstGreens = _mm_rcp_ps(_mm_rsqrt_ps(dstGreens));
        dstBlues  = _mm_rcp_ps(_mm_rsqrt_ps(dstBlues));

        // Clamp floats to 0-255 range.
        dstReds   = _mm_max_ps(_mm_setzero_ps(), _mm_min_ps(dstReds,   _mm_set1_ps(255.0f)));
        dstGreens = _mm_max_ps(_mm_setzero_ps(), _mm_min_ps(dstGreens, _mm_set1_ps(255.0f)));
        dstBlues  = _mm_max_ps(_mm_setzero_ps(), _mm_min_ps(dstBlues,  _mm_set1_ps(255.0f)));

        // Convert to bytes and store to memory.
        rgba = _mm_and_si128(_mm_set1_epi32(0xFF000000), rgba);
#ifdef SK_PMCOLOR_IS_RGBA
        rgba = _mm_or_si128(rgba,                _mm_cvtps_epi32(dstReds)       );
        rgba = _mm_or_si128(rgba, _mm_slli_epi32(_mm_cvtps_epi32(dstGreens),  8));
        rgba = _mm_or_si128(rgba, _mm_slli_epi32(_mm_cvtps_epi32(dstBlues),  16));
#else
        rgba = _mm_or_si128(rgba,                _mm_cvtps_epi32(dstBlues)      );
        rgba = _mm_or_si128(rgba, _mm_slli_epi32(_mm_cvtps_epi32(dstGreens),  8));
        rgba = _mm_or_si128(rgba, _mm_slli_epi32(_mm_cvtps_epi32(dstReds),   16));
#endif
        _mm_storeu_si128((__m128i*) dst, rgba);

        dst += 4;
        src += 4;
        len -= 4;
    }

    color_xform_2Dot2_RGBA_to_8888_portable(dst, src, len, matrix);
}

#else

static void color_xform_2Dot2_RGBA_to_8888(uint32_t* dst, const uint32_t* src, int len,
                                           const float matrix[16]) {
    color_xform_2Dot2_RGBA_to_8888_portable(dst, src, len, matrix);
}

#endif

}

#endif // SkColorXform_opts_DEFINED
