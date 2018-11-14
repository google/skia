/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcState_opts_SSSE3.h"
#include <tmmintrin.h>

// This same basic packing scheme is used throughout the file.
static void decode_packed_coordinates_and_weight(uint32_t packed, int* v0, int* v1, int* w) {
    // The top 14 bits are the integer coordinate x0 or y0.
    *v0 = packed >> 18;

    // The bottom 14 bits are the integer coordinate x1 or y1.
    *v1 = packed & 0x3fff;

    // The middle 4 bits are the interpolating factor between the two, i.e. the weight for v1.
    *w = (packed >> 14) & 0xf;
}

// As above, 4x.
static void decode_packed_coordinates_and_weight(__m128i packed, int v0[4], int v1[4], __m128i* w) {
    _mm_storeu_si128((__m128i*)v0, _mm_srli_epi32(packed, 18));
    _mm_storeu_si128((__m128i*)v1, _mm_and_si128 (packed, _mm_set1_epi32(0x3fff)));
    *w = _mm_and_si128(_mm_srli_epi32(packed, 14), _mm_set1_epi32(0xf));
}


// This is the crux of the whole file, interpolating in X for up to two output pixels (A and B).
static inline __m128i interpolate_in_x(uint32_t A0, uint32_t A1,
                                       uint32_t B0, uint32_t B1,
                                       const __m128i& interlaced_x_weights) {
    // _mm_maddubs_epi16() is a little idiosyncratic, but very helpful as the core of a lerp.
    //
    // It takes two arguments interlaced byte-wise:
    //    - first  arg: [ x,y, ... 7 more pairs of 8-bit values ...]
    //    - second arg: [ z,w, ... 7 more pairs of 8-bit values ...]
    // and returns 8 16-bit values: [ x*z + y*w, ... 7 more 16-bit values ... ].
    //
    // That's why we go to all this trouble to make interlaced_x_weights,
    // and here we're interlacing A0 with A1, B0 with B1 to match.

    __m128i interlaced_A = _mm_unpacklo_epi8(_mm_cvtsi32_si128(A0), _mm_cvtsi32_si128(A1)),
            interlaced_B = _mm_unpacklo_epi8(_mm_cvtsi32_si128(B0), _mm_cvtsi32_si128(B1));

    return _mm_maddubs_epi16(_mm_unpacklo_epi64(interlaced_A, interlaced_B),
                             interlaced_x_weights);
}

// Interpolate {A0..A3} --> output pixel A, and {B0..B3} --> output pixel B.
// Returns two pixels, with each channel in a 16-bit lane of the __m128i.
static inline __m128i interpolate_in_x_and_y(uint32_t A0, uint32_t A1,
                                             uint32_t A2, uint32_t A3,
                                             uint32_t B0, uint32_t B1,
                                             uint32_t B2, uint32_t B3,
                                             const __m128i& interlaced_x_weights,
                                             int wy) {
    // The stored Y weight wy is for y1, and y0 gets a weight 16-wy.
    const __m128i wy1 = _mm_set1_epi16(wy),
                  wy0 = _mm_sub_epi16(_mm_set1_epi16(16), wy1);

    // First interpolate in X,
    // leaving the values in 16-bit lanes scaled up by those [0,16] interlaced_x_weights.
    __m128i row0 = interpolate_in_x(A0,A1, B0,B1, interlaced_x_weights),
            row1 = interpolate_in_x(A2,A3, B2,B3, interlaced_x_weights);

    // Interpolate in Y across the two rows,
    // then scale everything down by the maximum total weight 16x16 = 256.
    return _mm_srli_epi16(_mm_add_epi16(_mm_mullo_epi16(row0, wy0),
                                        _mm_mullo_epi16(row1, wy1)), 8);
}


void S32_alpha_D32_filter_DX_SSSE3(const SkBitmapProcState& s,
                                   const uint32_t* xy,
                                   int count, uint32_t* colors) {
    SkASSERT(count > 0 && colors != nullptr);
    SkASSERT(s.fFilterQuality != kNone_SkFilterQuality);
    SkASSERT(kN32_SkColorType == s.fPixmap.colorType());

    int alpha = s.fAlphaScale;

    // Return (px * s.fAlphaScale) / 256.   (s.fAlphaScale is in [0,256].)
    auto scale_by_alpha = [alpha](const __m128i& px) {
        return alpha == 256 ? px
                            : _mm_srli_epi16(_mm_mullo_epi16(px, _mm_set1_epi16(alpha)), 8);
    };

    // We're in _DX_ mode here, so we're only varying in X.
    // That means the first entry of xy is our constant pair of Y coordinates and weight in Y.
    // All the other entries in xy will be pairs of X coordinates and the X weight.
    int y0, y1, wy;
    decode_packed_coordinates_and_weight(*xy++, &y0, &y1, &wy);

    auto row0 = (const uint32_t*)( (const uint8_t*)s.fPixmap.addr() + y0 * s.fPixmap.rowBytes() ),
         row1 = (const uint32_t*)( (const uint8_t*)s.fPixmap.addr() + y1 * s.fPixmap.rowBytes() );

    while (count >= 4) {
        // We can really get going, loading 4 X pairs at a time to produce 4 output pixels.
        const __m128i xx = _mm_loadu_si128((const __m128i*)xy);

        int x0[4],
            x1[4];
        __m128i wx;
        decode_packed_coordinates_and_weight(xx, x0, x1, &wx);

        // Splat out each x weight wx four times (one for each pixel channel) as wx1,
        // and sixteen minus that as the weight for x0, wx0.
        __m128i wx1 = _mm_shuffle_epi8(wx, _mm_setr_epi8(0,0,0,0, 4,4,4,4, 8,8,8,8, 12,12,12,12)),
                wx0 = _mm_sub_epi8(_mm_set1_epi8(16), wx1);

        // We need to interlace wx0 and wx1 for _mm_maddubs_epi16().
        __m128i interlaced_x_weights_AB = _mm_unpacklo_epi8(wx0,wx1),
                interlaced_x_weights_CD = _mm_unpackhi_epi8(wx0,wx1);

        // interpolate_in_x_and_y() can produce two output pixels (A and B) at a time
        // from eight input pixels {A0..A3} and {B0..B3}, arranged in a 2x2 grid for each.
        __m128i AB = interpolate_in_x_and_y(row0[x0[0]], row0[x1[0]],
                                            row1[x0[0]], row1[x1[0]],
                                            row0[x0[1]], row0[x1[1]],
                                            row1[x0[1]], row1[x1[1]],
                                            interlaced_x_weights_AB, wy);

        // Once more with the other half of the x-weights for two more pixels C,D.
        __m128i CD = interpolate_in_x_and_y(row0[x0[2]], row0[x1[2]],
                                            row1[x0[2]], row1[x1[2]],
                                            row0[x0[3]], row0[x1[3]],
                                            row1[x0[3]], row1[x1[3]],
                                            interlaced_x_weights_CD, wy);

        // Scale them all by alpha, pack back together to 8-bit lanes, and write out four pixels!
        _mm_storeu_si128((__m128i*)colors, _mm_packus_epi16(scale_by_alpha(AB),
                                                            scale_by_alpha(CD)));
        xy     += 4;
        colors += 4;
        count  -= 4;
    }

    while (count --> 0) {
        // This is exactly the same flow as the count >= 4 loop above, but writing one pixel.
        int x0, x1, wx;
        decode_packed_coordinates_and_weight(*xy++, &x0, &x1, &wx);

        // As above, splat out wx four times as wx1, and sixteen minus that as wx0.
        __m128i wx1 = _mm_set1_epi8(wx),     // This splats it out 16 times, but that's fine.
                wx0 = _mm_sub_epi8(_mm_set1_epi8(16), wx1);

        __m128i interlaced_x_weights_A = _mm_unpacklo_epi8(wx0, wx1);

        __m128i A = interpolate_in_x_and_y(row0[x0], row0[x1],
                                           row1[x0], row1[x1],
                                                  0,        0,
                                                  0,        0,
                                           interlaced_x_weights_A, wy);

        *colors++ = _mm_cvtsi128_si32(_mm_packus_epi16(scale_by_alpha(A), _mm_setzero_si128()));
    }
}
