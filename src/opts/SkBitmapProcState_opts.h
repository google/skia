/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapProcState_opts_DEFINED
#define SkBitmapProcState_opts_DEFINED

#include "src/core/SkBitmapProcState.h"

// SkBitmapProcState optimized Shader, Sample, or Matrix procs.
//
// Only S32_alpha_D32_filter_DX exploits instructions beyond
// our common baseline SSE2/NEON instruction sets, so that's
// all that lives here.
//
// The rest are scattershot at the moment but I want to get them
// all migrated to be normal code inside SkBitmapProcState.cpp.

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include <immintrin.h>
#elif defined(SK_ARM_HAS_NEON)
    #include <arm_neon.h>
#endif

namespace SK_OPTS_NS {

// This same basic packing scheme is used throughout the file.
static void decode_packed_coordinates_and_weight(uint32_t packed, int* v0, int* v1, int* w) {
    // The top 14 bits are the integer coordinate x0 or y0.
    *v0 = packed >> 18;

    // The bottom 14 bits are the integer coordinate x1 or y1.
    *v1 = packed & 0x3fff;

    // The middle 4 bits are the interpolating factor between the two, i.e. the weight for v1.
    *w = (packed >> 14) & 0xf;
}

#if 1 && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3

    // As above, 4x.
    static void decode_packed_coordinates_and_weight(__m128i packed,
                                                     int v0[4], int v1[4], __m128i* w) {
        _mm_storeu_si128((__m128i*)v0, _mm_srli_epi32(packed, 18));
        _mm_storeu_si128((__m128i*)v1, _mm_and_si128 (packed, _mm_set1_epi32(0x3fff)));
        *w = _mm_and_si128(_mm_srli_epi32(packed, 14), _mm_set1_epi32(0xf));
    }

    // This is the crux of the SSSE3 implementation,
    // interpolating in X for up to two output pixels (A and B) using _mm_maddubs_epi16().
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

    /*not static*/ inline
    void S32_alpha_D32_filter_DX(const SkBitmapProcState& s,
                                 const uint32_t* xy, int count, uint32_t* colors) {
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

        auto row0 = (const uint32_t*)((const uint8_t*)s.fPixmap.addr() + y0 * s.fPixmap.rowBytes()),
             row1 = (const uint32_t*)((const uint8_t*)s.fPixmap.addr() + y1 * s.fPixmap.rowBytes());

        while (count >= 4) {
            // We can really get going, loading 4 X pairs at a time to produce 4 output pixels.
            const __m128i xx = _mm_loadu_si128((const __m128i*)xy);

            int x0[4],
                x1[4];
            __m128i wx;
            decode_packed_coordinates_and_weight(xx, x0, x1, &wx);

            // Splat out each x weight wx four times (one for each pixel channel) as wx1,
            // and sixteen minus that as the weight for x0, wx0.
            __m128i wx1 = _mm_shuffle_epi8(wx, _mm_setr_epi8(0,0,0,0,4,4,4,4,8,8,8,8,12,12,12,12)),
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

            // Scale by alpha, pack back together to 8-bit lanes, and write out four pixels!
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


#elif 1 && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2

    /*not static*/ inline
    void S32_alpha_D32_filter_DX(const SkBitmapProcState& s,
                                 const uint32_t* xy, int count, uint32_t* colors) {
        SkASSERT(count > 0 && colors != nullptr);
        SkASSERT(s.fFilterQuality != kNone_SkFilterQuality);
        SkASSERT(kN32_SkColorType == s.fPixmap.colorType());
        SkASSERT(s.fAlphaScale <= 256);

        int y0, y1, wy;
        decode_packed_coordinates_and_weight(*xy++, &y0, &y1, &wy);

        auto row0 = (const uint32_t*)( (const char*)s.fPixmap.addr() + y0 * s.fPixmap.rowBytes() ),
             row1 = (const uint32_t*)( (const char*)s.fPixmap.addr() + y1 * s.fPixmap.rowBytes() );

        // We'll put one pixel in the low 4 16-bit lanes to line up with wy,
        // and another in the upper 4 16-bit lanes to line up with 16 - wy.
        const __m128i allY = _mm_unpacklo_epi64(_mm_set1_epi16(   wy),   // Bottom pixel goes here.
                                                _mm_set1_epi16(16-wy));  // Top pixel goes here.

        while (count --> 0) {
            int x0, x1, wx;
            decode_packed_coordinates_and_weight(*xy++, &x0, &x1, &wx);

            // Load the 4 pixels we're interpolating, in this grid:
            //    | tl  tr |
            //    | bl  br |
            const __m128i tl = _mm_cvtsi32_si128(row0[x0]), tr = _mm_cvtsi32_si128(row0[x1]),
                          bl = _mm_cvtsi32_si128(row1[x0]), br = _mm_cvtsi32_si128(row1[x1]);

            // Unpack the left pixels tl and bl to line up with 16-bit allY weights,
            // scaling them with 16-wx since they're left pixels.
            __m128i L = _mm_mullo_epi16(_mm_unpacklo_epi8(_mm_unpacklo_epi32(bl, tl),
                                                          _mm_setzero_si128()),
                                        _mm_set1_epi16(16-wx));

            // Same for the right pixels tr and br but with X weight wx.
            __m128i R = _mm_mullo_epi16(_mm_unpacklo_epi8(_mm_unpacklo_epi32(br, tr),
                                                          _mm_setzero_si128()),
                                        _mm_set1_epi16(   wx));

            // Add the two intermediates, summing across in the X direction,
            // and finally apply those Y weights that we lined them up for.
            __m128i sum_in_x = _mm_mullo_epi16(_mm_add_epi16(L, R),
                                               allY);

            // Add the lower and upper register halves to sum in the Y direction.
            __m128i sum = _mm_add_epi16(sum_in_x, _mm_srli_si128(sum_in_x, 8));

            // Get back to [0,255] by dividing by maximum weight 16x16 = 256.
            sum = _mm_srli_epi16(sum, 8);

            if (s.fAlphaScale < 256) {
                // Scale by alpha, which is in [0,256].
                sum = _mm_mullo_epi16(sum, _mm_set1_epi16(s.fAlphaScale));
                sum = _mm_srli_epi16(sum, 8);
            }

            // Pack back into 8-bit values and store.
            *colors++ = _mm_cvtsi128_si32(_mm_packus_epi16(sum, _mm_setzero_si128()));
        }
    }

#else

    // The NEON code only actually differs from the portable code in the
    // filtering step after we've loaded all four pixels we want to bilerp.

    #if defined(SK_ARM_HAS_NEON)
        static void filter_and_scale_by_alpha(unsigned x, unsigned y,
                                              SkPMColor a00, SkPMColor a01,
                                              SkPMColor a10, SkPMColor a11,
                                              SkPMColor *dst,
                                              uint16_t scale) {
            uint8x8_t vy, vconst16_8, v16_y, vres;
            uint16x4_t vx, vconst16_16, v16_x, tmp, vscale;
            uint32x2_t va0, va1;
            uint16x8_t tmp1, tmp2;

            vy = vdup_n_u8(y);                // duplicate y into vy
            vconst16_8 = vmov_n_u8(16);       // set up constant in vconst16_8
            v16_y = vsub_u8(vconst16_8, vy);  // v16_y = 16-y

            va0 = vdup_n_u32(a00);            // duplicate a00
            va1 = vdup_n_u32(a10);            // duplicate a10
            va0 = vset_lane_u32(a01, va0, 1); // set top to a01
            va1 = vset_lane_u32(a11, va1, 1); // set top to a11

            tmp1 = vmull_u8(vreinterpret_u8_u32(va0), v16_y); // tmp1 = [a01|a00] * (16-y)
            tmp2 = vmull_u8(vreinterpret_u8_u32(va1), vy);    // tmp2 = [a11|a10] * y

            vx = vdup_n_u16(x);                // duplicate x into vx
            vconst16_16 = vmov_n_u16(16);      // set up constant in vconst16_16
            v16_x = vsub_u16(vconst16_16, vx); // v16_x = 16-x

            tmp = vmul_u16(vget_high_u16(tmp1), vx);        // tmp  = a01 * x
            tmp = vmla_u16(tmp, vget_high_u16(tmp2), vx);   // tmp += a11 * x
            tmp = vmla_u16(tmp, vget_low_u16(tmp1), v16_x); // tmp += a00 * (16-x)
            tmp = vmla_u16(tmp, vget_low_u16(tmp2), v16_x); // tmp += a10 * (16-x)

            if (scale < 256) {
                vscale = vdup_n_u16(scale);        // duplicate scale
                tmp = vshr_n_u16(tmp, 8);          // shift down result by 8
                tmp = vmul_u16(tmp, vscale);       // multiply result by scale
            }

            vres = vshrn_n_u16(vcombine_u16(tmp, vcreate_u16(0)), 8); // shift down result by 8
            vst1_lane_u32(dst, vreinterpret_u32_u8(vres), 0);         // store result
        }
    #else
        static void filter_and_scale_by_alpha(unsigned x, unsigned y,
                                              SkPMColor a00, SkPMColor a01,
                                              SkPMColor a10, SkPMColor a11,
                                              SkPMColor* dstColor,
                                              unsigned alphaScale) {
            SkASSERT((unsigned)x <= 0xF);
            SkASSERT((unsigned)y <= 0xF);
            SkASSERT(alphaScale <= 256);

            int xy = x * y;
            const uint32_t mask = 0xFF00FF;

            int scale = 256 - 16*y - 16*x + xy;
            uint32_t lo = (a00 & mask) * scale;
            uint32_t hi = ((a00 >> 8) & mask) * scale;

            scale = 16*x - xy;
            lo += (a01 & mask) * scale;
            hi += ((a01 >> 8) & mask) * scale;

            scale = 16*y - xy;
            lo += (a10 & mask) * scale;
            hi += ((a10 >> 8) & mask) * scale;

            lo += (a11 & mask) * xy;
            hi += ((a11 >> 8) & mask) * xy;

            if (alphaScale < 256) {
                lo = ((lo >> 8) & mask) * alphaScale;
                hi = ((hi >> 8) & mask) * alphaScale;
            }

            *dstColor = ((lo >> 8) & mask) | (hi & ~mask);
        }
    #endif


    /*not static*/ inline
    void S32_alpha_D32_filter_DX(const SkBitmapProcState& s,
                                 const uint32_t* xy, int count, SkPMColor* colors) {
        SkASSERT(count > 0 && colors != nullptr);
        SkASSERT(s.fFilterQuality != kNone_SkFilterQuality);
        SkASSERT(4 == s.fPixmap.info().bytesPerPixel());
        SkASSERT(s.fAlphaScale <= 256);

        int y0, y1, wy;
        decode_packed_coordinates_and_weight(*xy++, &y0, &y1, &wy);

        auto row0 = (const uint32_t*)( (const char*)s.fPixmap.addr() + y0 * s.fPixmap.rowBytes() ),
             row1 = (const uint32_t*)( (const char*)s.fPixmap.addr() + y1 * s.fPixmap.rowBytes() );

        while (count --> 0) {
            int x0, x1, wx;
            decode_packed_coordinates_and_weight(*xy++, &x0, &x1, &wx);

            filter_and_scale_by_alpha(wx, wy,
                                      row0[x0], row0[x1],
                                      row1[x0], row1[x1],
                                      colors++,
                                      s.fAlphaScale);
        }
    }

#endif

}  // namespace SK_OPTS_NS

#endif
