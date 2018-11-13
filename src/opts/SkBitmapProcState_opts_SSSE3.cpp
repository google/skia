/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcState_opts_SSSE3.h"
#include "SkColorData.h"
#include "SkPaint.h"
#include "SkUTF.h"

#include <tmmintrin.h>  // SSSE3

namespace {

// Prepare all necessary constants for a round of processing for two pixel
// pairs.
// @param xy is the location where the xy parameters for four pixels should be
//           read from. It is identical in concept with argument two of
//           S32_{opaque}_D32_filter_DX methods.
// @param mask_3FFF vector of 32 bit constants containing 3FFF,
//                  suitable to mask the bottom 14 bits of a XY value.
// @param mask_000F vector of 32 bit constants containing 000F,
//                  suitable to mask the bottom 4 bits of a XY value.
// @param sixteen_8bit vector of 8 bit components containing the value 16.
// @param mask_dist_select vector of 8 bit components containing the shuffling
//                         parameters to reorder x[0-3] parameters.
// @param all_x_result vector of 8 bit components that will contain the
//              (4x(x3), 4x(x2), 4x(x1), 4x(x0)) upon return.
// @param sixteen_minus_x vector of 8 bit components, containing
//              (4x(16 - x3), 4x(16 - x2), 4x(16 - x1), 4x(16 - x0))
inline void PrepareConstantsTwoPixelPairs(const uint32_t* xy,
                                          const __m128i& mask_3FFF,
                                          const __m128i& mask_000F,
                                          const __m128i& sixteen_8bit,
                                          const __m128i& mask_dist_select,
                                          __m128i* all_x_result,
                                          __m128i* sixteen_minus_x,
                                          int* x0,
                                          int* x1) {
    const __m128i xx = _mm_loadu_si128(reinterpret_cast<const __m128i *>(xy));

    // 4 delta X
    // (x03, x02, x01, x00)
    const __m128i x0_wide = _mm_srli_epi32(xx, 18);
    // (x13, x12, x11, x10)
    const __m128i x1_wide = _mm_and_si128(xx, mask_3FFF);

    _mm_storeu_si128(reinterpret_cast<__m128i *>(x0), x0_wide);
    _mm_storeu_si128(reinterpret_cast<__m128i *>(x1), x1_wide);

    __m128i all_x = _mm_and_si128(_mm_srli_epi32(xx, 14), mask_000F);

    // (4x(x3), 4x(x2), 4x(x1), 4x(x0))
    all_x = _mm_shuffle_epi8(all_x, mask_dist_select);

    *all_x_result = all_x;
    // (4x(16-x3), 4x(16-x2), 4x(16-x1), 4x(16-x0))
    *sixteen_minus_x = _mm_sub_epi8(sixteen_8bit, all_x);
}

// Helper function used when processing one pixel pair.
// @param pixel0..3 are the four input pixels
// @param scale_x vector of 8 bit components to multiply the pixel[0:3]. This
//                will contain (4x(x1, 16-x1), 4x(x0, 16-x0))
//                or (4x(x3, 16-x3), 4x(x2, 16-x2))
// @return a vector of 16 bit components containing:
// (Aa2 * (16 - x1) + Aa3 * x1, ... , Ra0 * (16 - x0) + Ra1 * x0)
inline __m128i ProcessPixelPairHelper(uint32_t pixel0,
                                      uint32_t pixel1,
                                      uint32_t pixel2,
                                      uint32_t pixel3,
                                      const __m128i& scale_x) {
    __m128i a0, a1, a2, a3;
    // Load 2 pairs of pixels
    a0 = _mm_cvtsi32_si128(pixel0);
    a1 = _mm_cvtsi32_si128(pixel1);

    // Interleave pixels.
    // (0, 0, 0, 0, 0, 0, 0, 0, Aa1, Aa0, Ba1, Ba0, Ga1, Ga0, Ra1, Ra0)
    a0 = _mm_unpacklo_epi8(a0, a1);

    a2 = _mm_cvtsi32_si128(pixel2);
    a3 = _mm_cvtsi32_si128(pixel3);
    // (0, 0, 0, 0, 0, 0, 0, 0, Aa3, Aa2, Ba3, Ba2, Ga3, Ga2, Ra3, Ra2)
    a2 = _mm_unpacklo_epi8(a2, a3);

    // two pairs of pixel pairs, interleaved.
    // (Aa3, Aa2, Ba3, Ba2, Ga3, Ga2, Ra3, Ra2,
    //  Aa1, Aa0, Ba1, Ba0, Ga1, Ga0, Ra1, Ra0)
    a0 = _mm_unpacklo_epi64(a0, a2);

    // multiply and sum to 16 bit components.
    // (Aa2 * (16 - x1) + Aa3 * x1, ... , Ra0 * (16 - x0) + Ra1 * x0)
    // At that point, we use up a bit less than 12 bits for each 16 bit
    // component:
    // All components are less than 255. So,
    // C0 * (16 - x) + C1 * x <= 255 * (16 - x) + 255 * x = 255 * 16.
    return _mm_maddubs_epi16(a0, scale_x);
}

// Scale back the results after multiplications to the [0:255] range, and scale by alpha.
inline __m128i ScaleFourPixels(__m128i* pixels, const __m128i& alpha) {
    // Divide each 16 bit component by 256.
    *pixels = _mm_srli_epi16(*pixels, 8);

    // Multiply by alpha.
    *pixels = _mm_mullo_epi16(*pixels, alpha);

    // Divide each 16 bit component by 256.
    *pixels = _mm_srli_epi16(*pixels, 8);

    return *pixels;
}


// Same as ProcessPixelPairHelper, except that the values are scaled by y.
// @param y vector of 16 bit components containing 'y' values. There are two
//        cases in practice, where y will contain the sub_y constant, or will
//        contain the 16 - sub_y constant.
// @return vector of 16 bit components containing:
// (y * (Aa2 * (16 - x1) + Aa3 * x1), ... , y * (Ra0 * (16 - x0) + Ra1 * x0))
inline __m128i ProcessPixelPair(uint32_t pixel0,
                                uint32_t pixel1,
                                uint32_t pixel2,
                                uint32_t pixel3,
                                const __m128i& scale_x,
                                const __m128i& y) {
    __m128i sum = ProcessPixelPairHelper(pixel0, pixel1, pixel2, pixel3,
                                         scale_x);

    // first row times 16-y or y depending on whether 'y' represents one or
    // the other.
    // Values will be up to 255 * 16 * 16 = 65280.
    // (y * (Aa2 * (16 - x1) + Aa3 * x1), ... ,
    //  y * (Ra0 * (16 - x0) + Ra1 * x0))
    sum = _mm_mullo_epi16(sum, y);

    return sum;
}

// Process two pixel pairs out of eight input pixels.
// In other methods, the distinct pixels are passed one by one, but in this
// case, the rows, and index offsets to the pixels into the row are passed
// to generate the 8 pixels.
// @param row0..1 top and bottom row where to find input pixels.
// @param x0..1 offsets into the row for all eight input pixels.
// @param all_y vector of 16 bit components containing the constant sub_y
// @param neg_y vector of 16 bit components containing the constant 16 - sub_y
// @param alpha vector of 16 bit components containing the alpha value to scale
//        the results by
// @return
// (alpha * ((16-y) * (Aa2  * (16-x1) + Aa3  * x1) +
//             y    * (Aa2' * (16-x1) + Aa3' * x1)),
// ...
//  alpha * ((16-y) * (Ra0  * (16-x0) + Ra1 * x0) +
//             y    * (Ra0' * (16-x0) + Ra1' * x0))
// The values are scaled back to 16 bit components, but with only the bottom
// 8 bits being set.
inline __m128i ProcessTwoPixelPairs(const uint32_t* row0,
                                    const uint32_t* row1,
                                    const int* x0,
                                    const int* x1,
                                    const __m128i& scale_x,
                                    const __m128i& all_y,
                                    const __m128i& neg_y,
                                    const __m128i& alpha) {
    __m128i sum0 = ProcessPixelPair(
        row0[x0[0]], row0[x1[0]], row0[x0[1]], row0[x1[1]],
        scale_x, neg_y);
    __m128i sum1 = ProcessPixelPair(
        row1[x0[0]], row1[x1[0]], row1[x0[1]], row1[x1[1]],
        scale_x, all_y);

    // 2 samples fully summed.
    // ((16-y) * (Aa2 * (16-x1) + Aa3 * x1) +
    //  y * (Aa2' * (16-x1) + Aa3' * x1),
    // ...
    //  (16-y) * (Ra0 * (16 - x0) + Ra1 * x0)) +
    //  y * (Ra0' * (16-x0) + Ra1' * x0))
    // Each component, again can be at most 256 * 255 = 65280, so no overflow.
    sum0 = _mm_add_epi16(sum0, sum1);

    return ScaleFourPixels(&sum0, alpha);
}

// Same as ProcessPixelPair, except that performing the math one output pixel
// at a time. This means that only the bottom four 16 bit components are set.
inline __m128i ProcessOnePixel(uint32_t pixel0, uint32_t pixel1,
                               const __m128i& scale_x, const __m128i& y) {
    __m128i a0 = _mm_cvtsi32_si128(pixel0);
    __m128i a1 = _mm_cvtsi32_si128(pixel1);

    // Interleave
    // (0, 0, 0, 0, 0, 0, 0, 0, Aa1, Aa0, Ba1, Ba0, Ga1, Ga0, Ra1, Ra0)
    a0 = _mm_unpacklo_epi8(a0, a1);

    // (a0 * (16-x) + a1 * x)
    a0 = _mm_maddubs_epi16(a0, scale_x);

    // scale row by y
    return _mm_mullo_epi16(a0, y);
}

}  // namespace

// Notes about the various tricks that are used in this implementation:
// - calculating 4 output pixels at a time.
//  This allows loading the coefficients x0 and x1 and shuffling them to the
// optimum location only once per loop, instead of twice per loop.
// This also allows us to store the four pixels with a single store.
// - Use of 2 special SSSE3 instructions (comparatively to the SSE2 instruction
// version):
// _mm_shuffle_epi8 : this allows us to spread the coefficients x[0-3] loaded
// in 32 bit values to 8 bit values repeated four times.
// _mm_maddubs_epi16 : this allows us to perform multiplications and additions
// in one swoop of 8bit values storing the results in 16 bit values. This
// instruction is actually crucial for the speed of the implementation since
// as one can see in the SSE2 implementation, all inputs have to be used as
// 16 bits because the results are 16 bits. This basically allows us to process
// twice as many pixel components per iteration.
//
// As a result, this method behaves faster than the traditional SSE2. The actual
// boost varies greatly on the underlying architecture.
void S32_alpha_D32_filter_DX_SSSE3(const SkBitmapProcState& s,
                                   const uint32_t* xy,
                                   int count, uint32_t* colors) {
    SkASSERT(count > 0 && colors != nullptr);
    SkASSERT(s.fFilterQuality != kNone_SkFilterQuality);
    SkASSERT(kN32_SkColorType == s.fPixmap.colorType());

    const uint8_t* src_addr =
            static_cast<const uint8_t*>(s.fPixmap.addr());
    const size_t rb = s.fPixmap.rowBytes();
    const uint32_t XY = *xy++;
    const unsigned y0 = XY >> 14;
    const uint32_t* row0 =
            reinterpret_cast<const uint32_t*>(src_addr + (y0 >> 4) * rb);
    const uint32_t* row1 =
            reinterpret_cast<const uint32_t*>(src_addr + (XY & 0x3FFF) * rb);

    // vector constants
    const __m128i mask_dist_select = _mm_set_epi8(12, 12, 12, 12,
                                                  8,  8,  8,  8,
                                                  4,  4,  4,  4,
                                                  0,  0,  0,  0);
    const __m128i mask_3FFF = _mm_set1_epi32(0x3FFF);
    const __m128i mask_000F = _mm_set1_epi32(0x000F);
    const __m128i sixteen_8bit = _mm_set1_epi8(16);
    // (0, 0, 0, 0, 0, 0, 0, 0)
    const __m128i zero = _mm_setzero_si128();

    // 8x(alpha)
    const __m128i alpha = _mm_set1_epi16(s.fAlphaScale);

    // 8x(16)
    const __m128i sixteen_16bit = _mm_set1_epi16(16);

    // 8x (y)
    const __m128i all_y = _mm_set1_epi16(y0 & 0xF);

    // 8x (16-y)
    const __m128i neg_y = _mm_sub_epi16(sixteen_16bit, all_y);

    // Unroll 4x, interleave bytes, use pmaddubsw (all_x is small)
    while (count > 3) {
        count -= 4;

        int x0[4];
        int x1[4];
        __m128i all_x, sixteen_minus_x;
        PrepareConstantsTwoPixelPairs(xy, mask_3FFF, mask_000F,
                sixteen_8bit, mask_dist_select,
                &all_x, &sixteen_minus_x, x0, x1);
        xy += 4;

        // First pair of pixel pairs
        // (4x(x1, 16-x1), 4x(x0, 16-x0))
        __m128i scale_x;
        scale_x = _mm_unpacklo_epi8(sixteen_minus_x, all_x);

        __m128i sum0 = ProcessTwoPixelPairs(
                row0, row1, x0, x1,
                scale_x, all_y, neg_y, alpha);

        // second pair of pixel pairs
        // (4x (x3, 16-x3), 4x (16-x2, x2))
        scale_x = _mm_unpackhi_epi8(sixteen_minus_x, all_x);

        __m128i sum1 = ProcessTwoPixelPairs(
                row0, row1, x0 + 2, x1 + 2,
                scale_x, all_y, neg_y, alpha);

        // Do the final packing of the two results

        // Pack lower 4 16 bit values of sum into lower 4 bytes.
        sum0 = _mm_packus_epi16(sum0, sum1);

        // Extract low int and store.
        _mm_storeu_si128(reinterpret_cast<__m128i *>(colors), sum0);

        colors += 4;
    }

    // Left over.
    while (count-- > 0) {
        const uint32_t xx = *xy++;  // x0:14 | 4 | x1:14
        const unsigned x0 = xx >> 18;
        const unsigned x1 = xx & 0x3FFF;

        // 16x(x)
        const __m128i all_x = _mm_set1_epi8((xx >> 14) & 0x0F);

        // 16x (16-x)
        __m128i scale_x = _mm_sub_epi8(sixteen_8bit, all_x);

        // (8x (x, 16-x))
        scale_x = _mm_unpacklo_epi8(scale_x, all_x);

        // first row.
        __m128i sum0 = ProcessOnePixel(row0[x0], row0[x1], scale_x, neg_y);
        // second row.
        __m128i sum1 = ProcessOnePixel(row1[x0], row1[x1], scale_x, all_y);

        // Add both rows for full sample
        sum0 = _mm_add_epi16(sum0, sum1);

        sum0 = ScaleFourPixels(&sum0, alpha);

        // Pack lower 4 16 bit values of sum into lower 4 bytes.
        sum0 = _mm_packus_epi16(sum0, zero);

        // Extract low int and store.
        *colors++ = _mm_cvtsi128_si32(sum0);
    }
}
