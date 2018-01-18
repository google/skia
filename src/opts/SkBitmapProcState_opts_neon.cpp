// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE
// file.

#include "SkBitmapProcState_opts_neon.h"
#include "SkBitmapProcState_utils.h"
#include "SkColorData.h"
#include "SkPaint.h"
#include "SkUtils.h"

#include <arm_neon.h>

// In this file, variations for alpha and non alpha versions are implemented with a template, as it
// makes the code more compact and a bit easier to maintain, while making the compiler generate the
// same exact code as with two functions that only differ by a few lines.

// Prepare all necessary constants for a round of processing for two pixel pairs.
// @param xy is the location where the xy parameters for four pixels should be read from. It is
//           identical in concept with argument two of S32_{opaque}_D32_filter_DX methods.
// @param all_x_result vector of 8 bit components that will contain the
//              (4x(x3), 4x(x2), 4x(x1), 4x(x0)) upon return.
// @param sixteen_minus_x vector of 8 bit components, containing
//              (4x(16 - x3), 4x(16 - x2), 4x(16 - x1), 4x(16 - x0))
inline void PrepareConstantsTwoPixelPairs(const uint32_t* xy,
                                          uint8x16_t* all_x_result,
                                          uint8x16_t* sixteen_minus_x,
                                          uint32_t* x0,
                                          uint32_t* x1) {
    const uint32x4_t mask_3FFF = vdupq_n_u32(0x3FFF);
    const uint32x4_t mask_000F = vdupq_n_u32(0x000F);
    const uint8x16_t sixteen_8bit = vdupq_n_u8(16);

    const uint32x4_t xx = vld1q_u32(xy);

    // 4 delta X
    // (x03, x02, x01, x00)
    const uint32x4_t x0_wide = vshrq_n_u32(xx, 18);
    // (x13, x12, x11, x10)
    const uint32x4_t x1_wide = vandq_u32(xx, mask_3FFF);

    vst1q_u32(x0, x0_wide);
    vst1q_u32(x1, x1_wide);

    uint32x4_t all_x = vandq_u32(vshrq_n_u32(xx, 14), mask_000F);

    // (4x(x3), 4x(x2), 4x(x1), 4x(x0))
    all_x = vmulq_n_u32(all_x, 0x01010101);

    *all_x_result = vreinterpretq_u8_u32(all_x);
    // (4x(16-x3), 4x(16-x2), 4x(16-x1), 4x(16-x0))
    *sixteen_minus_x = vsubq_u8(sixteen_8bit, vreinterpretq_u8_u32(all_x));
}

// Prepare all necessary constants for a round of processing for two pixel pairs.
// @param xy is the location where the xy parameters for four pixels should be read from. It is
//           identical in concept with argument two of S32_{opaque}_D32_filter_DXDY methods.
// @param all_xy_result vector of 8 bit components that will contain the
//              (4x(y1), 4x(y0), 4x(x1), 4x(x0)) upon return.
// @param sixteen_minus_x vector of 8 bit components, containing
//              (4x(16-y1), 4x(16-y0), 4x(16-x1), 4x(16-x0)).
inline void PrepareConstantsTwoPixelPairsDXDY(const uint32_t* xy,
                                              uint8x16_t* all_xy_result,
                                              uint8x16_t* sixteen_minus_xy,
                                              uint32_t* xy0,
                                              uint32_t* xy1) {
    const uint32x4_t mask_3FFF = vdupq_n_u32(0x3FFF);
    const uint32x4_t mask_000F = vdupq_n_u32(0x000F);
    const uint8x16_t sixteen_8bit = vdupq_n_u8(16);

    // Do the shuffling first with a clever load + combine.
    const uint32x2x2_t xy_wide_tmp = vld2_u32(xy);
    // (y10, y00, x10, x00)
    const uint32x4_t xy_wide = vcombine_u32(xy_wide_tmp.val[1], xy_wide_tmp.val[0]);

    // (y10, y00, x10, x00)
    uint32x4_t xy0_wide = vshrq_n_u32(xy_wide, 18);
    // (y11, y01, x11, x01)
    uint32x4_t xy1_wide = vandq_u32(xy_wide, mask_3FFF);

    vst1q_u32(xy0, xy0_wide);
    vst1q_u32(xy1, xy1_wide);

    // (y1, y0, x1, x0)
    uint32x4_t all_xy = vandq_u32(vshrq_n_u32(xy_wide, 14), mask_000F);
    // (4x(y1), 4x(y0), 4x(x1), 4x(x0))
    all_xy = vmulq_n_u32(all_xy, 0x01010101);

    *all_xy_result = vreinterpretq_u8_u32(all_xy);
    // (4x(16-y1), 4x(16-y0), 4x(16-x1), 4x(16-x0))
    *sixteen_minus_xy = vsubq_u8(sixteen_8bit, vreinterpretq_u8_u32(all_xy));
}

// Helper function used when processing one pixel pair.
// @param pixel0..3 are the four input pixels
// @param scale_x vector of 8 bit components to multiply the pixel[0:3]. This will contain
//                (4x(x1, 16-x1), 4x(x0, 16-x0)) or (4x(x3, 16-x3), 4x(x2, 16-x2))
// @return a vector of 16 bit components containing:
//                (Aa2 * (16 - x1) + Aa3 * x1, ... , Ra0 * (16 - x0) + Ra1 * x0)
inline uint16x8_t ProcessPixelPairHelper(uint32_t pixel0,
                                         uint32_t pixel1,
                                         uint32_t pixel2,
                                         uint32_t pixel3,
                                         const uint8x16_t& scale_x) {
    uint32x2_t px0, px1;
    uint8x8x2_t a0, a1;

    // Load 2 pairs of pixels
    px0 = vmov_n_u32(pixel0);
    px1 = vmov_n_u32(pixel1);
    // Interleave pixels: (Aa1, Aa0, Ba1, Ba0, Ga1, Ga0, Ra1, Ra0)
    a0 = vzip_u8(vreinterpret_u8_u32(px0), vreinterpret_u8_u32(px1));

    // Repeat for pixel2 and pixel3.
    px0 = vmov_n_u32(pixel2);
    px1 = vmov_n_u32(pixel3);
    // Interleave pixels: (Aa3, Aa2, Ba3, Ba2, Ga3, Ga2, Ra3, Ra2)
    a1 = vzip_u8(vreinterpret_u8_u32(px0), vreinterpret_u8_u32(px1));

    // Multiply and sum to 16 bit components.
    // (Aa2 * (16 - x1) + Aa3 * x1, ... , Ra0 * (16 - x0) + Ra1 * x0)
    // At that point, we use up a bit less than 12 bits for each 16 bit component:
    // All components are less than 255. So,
    // C0 * (16 - x) + C1 * x <= 255 * (16 - x) + 255 * x = 255 * 16.
    uint16x8_t a0_scaled = vmull_u8(a0.val[0], vget_low_u8(scale_x));
    uint16x8_t a1_scaled = vmull_u8(a1.val[0], vget_high_u8(scale_x));
    uint16x4_t low = vpadd_u16(vget_low_u16(a0_scaled), vget_high_u16(a0_scaled));
    uint16x4_t high = vpadd_u16(vget_low_u16(a1_scaled), vget_high_u16(a1_scaled));

    return vcombine_u16(low, high);
}

// Scale back the results after multiplications to the [0:255] range, and scale by alpha when
// has_alpha is true. Depending on whether one set or two sets of multiplications had been applied,
// the results have to be shifted by four places (dividing by 16), or shifted by eight places
// (dividing by 256), since each multiplication is by a quantity in the range [0:16].
template <bool has_alpha, int scale>
inline uint16x8_t ScaleFourPixels(uint16x8_t* pixels, const uint16x8_t& alpha) {
    // Divide each 16 bit component by 16 (or 256 depending on scale).
    *pixels = vshrq_n_u16(*pixels, scale);

    if (has_alpha) {
        // Multiply by alpha.
        *pixels = vmulq_u16(*pixels, alpha);

        // Divide each 16 bit component by 256.
        *pixels = vshrq_n_u16(*pixels, 8);
    }
    return *pixels;
}

// Wrapper to calculate two output pixels from four input pixels. The arguments are the same as
// ProcessPixelPairHelper. Technically, there are eight input pixels, but since sub_y == 0, the
// factors applied to half of the pixels is zero (sub_y), and are therefore omitted here to save on
// some processing.
// @param alpha when has_alpha is true, scale all resulting components by this value.
// @return a vector of 16 bit components containing:
// ((Aa2 * (16 - x1) + Aa3 * x1) * alpha, ..., (Ra0 * (16 - x0) + Ra1 * x0) * alpha) when has_alpha
// is true. Otherwise:
// (Aa2 * (16 - x1) + Aa3 * x1, ... , Ra0 * (16 - x0) + Ra1 * x0)
// In both cases, the results are renormalized (divided by 16) to match the expected formats when
// storing back the results into memory.
template <bool has_alpha>
inline uint16x8_t ProcessPixelPairZeroSubY(uint32_t pixel0,
                                           uint32_t pixel1,
                                           uint32_t pixel2,
                                           uint32_t pixel3,
                                           const uint8x16_t& scale_x,
                                           const uint16x8_t& alpha) {
    uint16x8_t sum = ProcessPixelPairHelper(pixel0, pixel1, pixel2, pixel3, scale_x);
    return ScaleFourPixels<has_alpha, 4>(&sum, alpha);
}

// Same as ProcessPixelPairZeroSubY, expect processing one output pixel at a time instead of two.
// As in the above function, only two pixels are needed to generate a single pixel since
// sub_y == 0.
// @return same as ProcessPixelPairZeroSubY, except that only the bottom 4 16 bit components are
// set.
template <bool has_alpha>
inline uint16x8_t ProcessOnePixelZeroSubY(uint32_t pixel0,
                                          uint32_t pixel1,
                                          uint8x16_t scale_x,
                                          uint16x8_t alpha) {
    // Load pair of pixels.
    uint32x2_t px0 = vmov_n_u32(pixel0);
    uint32x2_t px1 = vmov_n_u32(pixel1);
    // Interleave pixels: (Aa1, Aa0, Ba1, Ba0, Ga1, Ga0, Ra1, Ra0)
    uint8x8x2_t a0 = vzip_u8(vreinterpret_u8_u32(px0), vreinterpret_u8_u32(px1));

    // (a0 * (16-x) + a1 * x)
    uint16x8_t a0_scaled = vmull_u8(a0.val[0], vget_low_u8(scale_x));
    uint16x4_t sum_low = vpadd_u16(vget_low_u16(a0_scaled), vget_high_u16(a0_scaled));
    // This is a hack to make the result vector the correct width.
    uint16x8_t sum = vcombine_u16(sum_low, vcreate_u64(0x0));
    return ScaleFourPixels<has_alpha, 4>(&sum, alpha);
}

// Methods when sub_y != 0.

// Same as ProcessPixelPairHelper, except that the values are scaled by y.
// @param y vector of 16 bit components containing 'y' values. There are two cases in practice,
//        where y will contain the sub_y constant, or will contain the 16 - sub_y constant.
// @return vector of 16 bit components containing:
// (y * (Aa2 * (16 - x1) + Aa3 * x1), ... , y * (Ra0 * (16 - x0) + Ra1 * x0))
inline uint16x8_t ProcessPixelPair(uint32_t pixel0,
                                   uint32_t pixel1,
                                   uint32_t pixel2,
                                   uint32_t pixel3,
                                   const uint8x16_t& scale_x,
                                   const uint16x8_t& y) {
    uint16x8_t sum = ProcessPixelPairHelper(pixel0, pixel1, pixel2, pixel3, scale_x);

    // First row times 16-y or y depending on whether 'y' represents one or the other.
    // Values will be up to 255 * 16 * 16 = 65280.
    // (y * (Aa2 * (16 - x1) + Aa3 * x1), ... , y * (Ra0 * (16 - x0) + Ra1 * x0))
    sum = vmulq_u16(sum, y);
    return sum;
}

// Process two pixel pairs out of eight input pixels.
// In other methods, the distinct pixels are passed one by one, but in this case, the rows, and
// index offsets to the pixels into the row are passed to generate the 8 pixels.
// @param row0..1 top and bottom row where to find input pixels.
// @param x0..1 offsets into the row for all eight input pixels.
// @param all_y vector of 16 bit components containing the constant sub_y
// @param neg_y vector of 16 bit components containing the constant 16 - sub_y
// @param alpha vector of 16 bit components containing the alpha value to scale the results by,
//        when has_alpha is true.
// @return
// (alpha * ((16-y) * (Aa2  * (16-x1) + Aa3  * x1) +
//             y    * (Aa2' * (16-x1) + Aa3' * x1)),
// ...
//  alpha * ((16-y) * (Ra0  * (16-x0) + Ra1 * x0) +
//             y    * (Ra0' * (16-x0) + Ra1' * x0))
// With the factor alpha removed when has_alpha is false.
// The values are scaled back to 16 bit components, but with only the bottom 8 bits being set.
template <bool has_alpha>
inline uint16x8_t ProcessTwoPixelPairs(const uint32_t* row0,
                                       const uint32_t* row1,
                                       const unsigned* x0,
                                       const unsigned* x1,
                                       const uint8x16_t& scale_x,
                                       const uint16x8_t& all_y,
                                       const uint16x8_t& neg_y,
                                       const uint16x8_t& alpha) {
    uint16x8_t sum0 =
            ProcessPixelPair(row0[x0[0]], row0[x1[0]], row0[x0[1]], row0[x1[1]], scale_x, neg_y);
    uint16x8_t sum1 =
            ProcessPixelPair(row1[x0[0]], row1[x1[0]], row1[x0[1]], row1[x1[1]], scale_x, all_y);

    // 2 samples fully summed.
    // ((16-y) * (Aa2 * (16-x1) + Aa3 * x1)    + y * (Aa2' * (16-x1) + Aa3' * x1),
    // ...
    //  (16-y) * (Ra0 * (16 - x0) + Ra1 * x0)) + y * (Ra0' * (16-x0) + Ra1' * x0))
    // Each component, again can be at most 256 * 255 = 65280, so no overflow.
    sum0 = vaddq_u16(sum0, sum1);

    return ScaleFourPixels<has_alpha, 8>(&sum0, alpha);
}

// Similar to ProcessTwoPixelPairs except the pixel indexes.
template <bool has_alpha>
inline uint16x8_t ProcessTwoPixelPairsDXDY(const uint32_t* row00,
                                           const uint32_t* row01,
                                           const uint32_t* row10,
                                           const uint32_t* row11,
                                           const unsigned* xy0,
                                           const unsigned* xy1,
                                           const uint8x16_t& scale_x,
                                           const uint16x8_t& all_y,
                                           const uint16x8_t& neg_y,
                                           const uint16x8_t& alpha) {
    // First row.
    uint16x8_t sum0 = ProcessPixelPair(row00[xy0[0]], row00[xy1[0]], row10[xy0[1]], row10[xy1[1]],
                                       scale_x, neg_y);
    // Second row.
    uint16x8_t sum1 = ProcessPixelPair(row01[xy0[0]], row01[xy1[0]], row11[xy0[1]], row11[xy1[1]],
                                       scale_x, all_y);

    // 2 samples fully summed.
    // ((16-y1) * (Aa2 * (16-x1) + Aa3 * x1)    + y0 * (Aa2' * (16-x1) + Aa3' * x1),
    // ...
    //  (16-y0) * (Ra0 * (16 - x0) + Ra1 * x0)) + y0 * (Ra0' * (16-x0) + Ra1' * x0))
    // Each component, again can be at most 256 * 255 = 65280, so no overflow.
    sum0 = vaddq_u16(sum0, sum1);

    return ScaleFourPixels<has_alpha, 8>(&sum0, alpha);
}

// Same as ProcessPixelPair, except that performing the math one output pixel at a time. This means
// that only the bottom four 16 bit components are set.
inline uint16x8_t ProcessOnePixel(uint32_t pixel0,
                                  uint32_t pixel1,
                                  const uint8x16_t& scale_x,
                                  const uint16x8_t& y) {
    // Load pair of pixels
    uint32x2_t px0 = vmov_n_u32(pixel0);
    uint32x2_t px1 = vmov_n_u32(pixel1);

    // Interleave: (Aa1, Aa0, Ba1, Ba0, Ga1, Ga0, Ra1, Ra0)
    uint8x8x2_t a0 = vzip_u8(vreinterpret_u8_u32(px0), vreinterpret_u8_u32(px1));

    // (a0 * (16-x) + a1 * x)
    uint16x8_t a0_scaled = vmull_u8(a0.val[0], vget_low_u8(scale_x));
    uint16x4_t sum_low = vpadd_u16(vget_low_u16(a0_scaled), vget_high_u16(a0_scaled));
    // This is a hack to make the result vector the correct width.
    uint16x8_t sum = vcombine_u16(sum_low, vcreate_u64(0x0));

    // Scale row by y.
    return vmulq_u16(sum, y);
}

// Notes about the various tricks that are used in this implementation:
// -> Specialization for sub_y == 0.
// Statistically, 1/16th of the samples will have sub_y == 0. When this happens, the math goes
// from:
// (16 - x)*(16 - y)*a00 + x*(16 - y)*a01 + (16 - x)*y*a10 + x*y*a11
// to:
// (16 - x)*a00 + 16*x*a01
// i.e. much simpler. The simplification makes for an easy boost in performance.
// -> Calculating 4 output pixels at a time.
// This allows loading the coefficients x0 and x1 and shuffling them to the optimum location only
// once per loop, instead of twice per loop. This also allows us to store the four pixels with a
// single store.
template <bool has_alpha>
void S32_generic_D32_filter_DX_neon(const SkBitmapProcState& s,
                                    const unsigned* xy,
                                    int count,
                                    unsigned* colors) {
    SkASSERT(count > 0 && colors != nullptr);
    SkASSERT(s.fFilterQuality != kNone_SkFilterQuality);
    SkASSERT(kN32_SkColorType == s.fPixmap.colorType());
    if (has_alpha) {
        SkASSERT(s.fAlphaScale < 256);
    } else {
        SkASSERT(s.fAlphaScale == 256);
    }

    const uint8_t* src_addr = static_cast<const uint8_t*>(s.fPixmap.addr());
    const size_t rb = s.fPixmap.rowBytes();
    const uint32_t XY = *xy++;
    const unsigned y0 = XY >> 14;
    const uint32_t* row0 = reinterpret_cast<const uint32_t*>(src_addr + (y0 >> 4) * rb);
    const uint32_t* row1 = reinterpret_cast<const uint32_t*>(src_addr + (XY & 0x3FFF) * rb);
    const uint16_t sub_y = y0 & 0xF;

    // Vector constants.
    const uint8x16_t sixteen_8bit = vdupq_n_u8(16);

    uint16x8_t alpha = vdupq_n_u16(0);
    if (has_alpha) {
        // 8x(alpha)
        alpha = vdupq_n_u16(s.fAlphaScale);
    }

    if (sub_y == 0) {
        // Unroll 4x, interleave bytes.
        while (count > 3) {
            count -= 4;

            unsigned x0[4];
            unsigned x1[4];
            uint8x16_t all_x, sixteen_minus_x;
            PrepareConstantsTwoPixelPairs(xy, &all_x, &sixteen_minus_x, x0, x1);
            xy += 4;

            // Both pairs of pixel pairs.
            // (4x (x3, 16-x3), 4x (16-x2, x2)) (4x(x1, 16-x1), 4x(x0, 16-x0))
            uint8x16x2_t scale_x = vzipq_u8(sixteen_minus_x, all_x);

            uint16x8_t sum0 = ProcessPixelPairZeroSubY<has_alpha>(
                    row0[x0[0]], row0[x1[0]], row0[x0[1]], row0[x1[1]], scale_x.val[0], alpha);

            uint16x8_t sum1 = ProcessPixelPairZeroSubY<has_alpha>(
                    row0[x0[2]], row0[x1[2]], row0[x0[3]], row0[x1[3]], scale_x.val[1], alpha);

            // Narrow (and saturate) values of sum from 16 to 8 bits.
            uint8x16_t sum = vcombine_u8(vqmovn_u16(sum0), vqmovn_u16(sum1));

            // Store 4 pixels to the output pointer: colors.
            vst1q_u32(colors, vreinterpretq_u32_u8(sum));

            colors += 4;
        }

        // Handle remainder.
        while (count-- > 0) {
            const uint32_t xx = *xy++;  // x0:14 | 4 | x1:14
            const unsigned x0 = xx >> 18;
            const unsigned x1 = xx & 0x3FFF;

            // 16x(x)
            const uint8x16_t all_x = vdupq_n_u8((xx >> 14) & 0x0F);

            // (16x(16-x))
            uint8x16x2_t scale_x;
            scale_x.val[0] = vsubq_u8(sixteen_8bit, all_x);

            scale_x = vzipq_u8(scale_x.val[0], all_x);

            // Only lower half of interest here.
            uint16x8_t sum =
                    ProcessOnePixelZeroSubY<has_alpha>(row0[x0], row0[x1], scale_x.val[0], alpha);

            // Narrow (and saturate) value of sum from 16 to 8 bits.
            uint8x8_t pixel = vqmovn_u16(sum);

            // Extract low int and store.
            *colors++ = vget_lane_u32(vreinterpret_u32_u8(pixel), 0);
        }
    } else {  // More general case: y != 0.
        // 8x (y)
        const uint16x8_t all_y = vdupq_n_u16(sub_y);
        // 8x (16-y)
        const uint16x8_t neg_y = vsubq_u16(vdupq_n_u16(16), all_y);

        // Unroll 4x, interleave bytes.
        while (count > 3) {
            count -= 4;

            unsigned x0[4];
            unsigned x1[4];
            uint8x16_t all_x, sixteen_minus_x;
            PrepareConstantsTwoPixelPairs(xy, &all_x, &sixteen_minus_x, x0, x1);
            xy += 4;

            // Both pair of pixel pairs.
            // (4x (x3, 16-x3), 4x (16-x2, x2)) (4x(x1, 16-x1), 4x(x0, 16-x0))
            uint8x16x2_t scale_x = vzipq_u8(sixteen_minus_x, all_x);

            uint16x8_t sum0 = ProcessTwoPixelPairs<has_alpha>(row0, row1, x0, x1, scale_x.val[0],
                                                              all_y, neg_y, alpha);

            uint16x8_t sum1 = ProcessTwoPixelPairs<has_alpha>(row0, row1, x0 + 2, x1 + 2,
                                                              scale_x.val[1], all_y, neg_y, alpha);

            // Narrow (and saturate) values of sum from 16 to 8 bits.
            uint8x16_t sum = vcombine_u8(vqmovn_u16(sum0), vqmovn_u16(sum1));

            // Store 4 pixels to the output pointer: colors.
            vst1q_u32(colors, vreinterpretq_u32_u8(sum));

            colors += 4;
        }

        // Handle remainder.
        while (count-- > 0) {
            const uint32_t xx = *xy++;  // x0:14 | 4 | x1:14
            const unsigned x0 = xx >> 18;
            const unsigned x1 = xx & 0x3FFF;

            // 16x(x)
            const uint8x16_t all_x = vdupq_n_u8((xx >> 14) & 0x0F);
            // 16x (16-x)
            uint8x16x2_t scale_x;
            scale_x.val[0] = vsubq_u8(sixteen_8bit, all_x);
            // (8x (x, 16-x))
            scale_x = vzipq_u8(scale_x.val[0], all_x);

            // First row.
            uint16x8_t sum0 = ProcessOnePixel(row0[x0], row0[x1], scale_x.val[0], neg_y);
            // Second row.
            uint16x8_t sum1 = ProcessOnePixel(row1[x0], row1[x1], scale_x.val[0], all_y);

            // Add both rows for full sample.
            sum0 = vaddq_u16(sum0, sum1);

            sum0 = ScaleFourPixels<has_alpha, 8>(&sum0, alpha);

            // Narrow (and saturate) value of sum from 16 to 8 bits.
            uint8x8_t pixel = vqmovn_u16(sum0);

            // Extract low int and store.
            *colors++ = vget_lane_u32(vreinterpret_u32_u8(pixel), 0);
        }
    }
}

// Similar to S32_generic_D32_filter_DX_neon, we do not need to handle the special case suby == 0
// as suby is changing in every loop.
template <bool has_alpha>
void S32_generic_D32_filter_DXDY_neon(const SkBitmapProcState& s,
                                      const unsigned* xy,
                                      int count,
                                      unsigned* colors) {
    SkASSERT(count > 0 && colors != nullptr);
    SkASSERT(s.fFilterQuality != kNone_SkFilterQuality);
    SkASSERT(kN32_SkColorType == s.fPixmap.colorType());
    if (has_alpha) {
        SkASSERT(s.fAlphaScale < 256);
    } else {
        SkASSERT(s.fAlphaScale == 256);
    }

    const uint8_t* src_addr = static_cast<const uint8_t*>(s.fPixmap.addr());
    const size_t rb = s.fPixmap.rowBytes();

    // Vector constants.
    const uint8x16_t sixteen_8bit = vdupq_n_u8(16);

    uint16x8_t alpha;
    if (has_alpha) {
        // 8x(alpha)
        alpha = vdupq_n_u16(s.fAlphaScale);
    }

    // Unroll 2x, interleave bytes.
    while (count >= 2) {
        unsigned xy0[4];
        unsigned xy1[4];
        uint8x16_t all_xy, sixteen_minus_xy;
        PrepareConstantsTwoPixelPairsDXDY(xy, &all_xy, &sixteen_minus_xy, xy0, xy1);

        // (4x(x1, 16-x1), 4x(x0, 16-x0))
        uint8x16x2_t scale_x = vzipq_u8(sixteen_minus_xy, all_xy);
        // (4x(0, y1), 4x(0, y0))
        uint8x16x2_t all_y_wide = vzipq_u8(all_xy, vdupq_n_u8(0));
        uint16x8_t all_y = vreinterpretq_u16_u8(all_y_wide.val[1]);

        uint16x8_t neg_y = vsubq_u16(vdupq_n_u16(16), all_y);

        const uint32_t* row00 = reinterpret_cast<const uint32_t*>(src_addr + xy0[2] * rb);
        const uint32_t* row01 = reinterpret_cast<const uint32_t*>(src_addr + xy1[2] * rb);
        const uint32_t* row10 = reinterpret_cast<const uint32_t*>(src_addr + xy0[3] * rb);
        const uint32_t* row11 = reinterpret_cast<const uint32_t*>(src_addr + xy1[3] * rb);

        uint16x8_t sum0 = ProcessTwoPixelPairsDXDY<has_alpha>(row00, row01, row10, row11, xy0, xy1,
                                                              scale_x.val[0], all_y, neg_y, alpha);

        // Narrow (and saturate) values of sum0 from 16 to 8 bits.
        uint8x8_t sum = vqmovn_u16(sum0);

        // Store 2 pixels to the output pointer: colors.
        vst1_u32(colors, vreinterpret_u32_u8(sum));

        xy += 4;
        colors += 2;
        count -= 2;
    }

    // Handle remainder.
    while (count-- > 0) {
        uint32_t data = *xy++;
        unsigned y0 = data >> 14;
        unsigned y1 = data & 0x3FFF;
        unsigned subY = y0 & 0xF;
        y0 >>= 4;

        data = *xy++;
        unsigned x0 = data >> 14;
        unsigned x1 = data & 0x3FFF;
        unsigned subX = x0 & 0xF;
        x0 >>= 4;

        const uint32_t* row0 = reinterpret_cast<const uint32_t*>(src_addr + y0 * rb);
        const uint32_t* row1 = reinterpret_cast<const uint32_t*>(src_addr + y1 * rb);

        // 16x(x)
        const uint8x16_t all_x = vdupq_n_u8(subX);
        // 16x (16-x)
        uint8x16x2_t scale_x;
        scale_x.val[0] = vsubq_u8(sixteen_8bit, all_x);
        // (8x (x, 16-x))
        scale_x = vzipq_u8(scale_x.val[0], all_x);
        // 8x(16)
        const uint16x8_t sixteen_16bit = vdupq_n_u16(16);
        // 8x (y)
        const uint16x8_t all_y = vdupq_n_u16(subY);
        // 8x (16-y)
        const uint16x8_t neg_y = vsubq_u16(sixteen_16bit, all_y);

        // First row.
        uint16x8_t sum0 = ProcessOnePixel(row0[x0], row0[x1], scale_x.val[0], neg_y);
        // Second row.
        uint16x8_t sum1 = ProcessOnePixel(row1[x0], row1[x1], scale_x.val[0], all_y);

        // Add both rows for full sample.
        sum0 = vaddq_u16(sum0, sum1);

        sum0 = ScaleFourPixels<has_alpha, 8>(&sum0, alpha);

        // Narrow (and saturate) value of sum0 from 16 to 8 bits.
        uint8x8_t pixel = vqmovn_u16(sum0);

        // Extract low int and store.
        *colors++ = vget_lane_u32(vreinterpret_u32_u8(pixel), 0);
    }
}

void S32_opaque_D32_filter_DX_neon_4x(const SkBitmapProcState& s, const unsigned* xy, int count,
                                      unsigned* colors) {
    S32_generic_D32_filter_DX_neon<false>(s, xy, count, colors);
}

void S32_alpha_D32_filter_DX_neon_4x(const SkBitmapProcState& s, const unsigned* xy, int count,
                                     unsigned* colors) {
    S32_generic_D32_filter_DX_neon<true>(s, xy, count, colors);
}

void S32_opaque_D32_filter_DXDY_neon_4x(const SkBitmapProcState& s, const unsigned* xy, int count,
                                        unsigned* colors) {
    S32_generic_D32_filter_DXDY_neon<false>(s, xy, count, colors);
}

void S32_alpha_D32_filter_DXDY_neon_4x(const SkBitmapProcState& s, const unsigned* xy, int count,
                                       unsigned* colors) {
    S32_generic_D32_filter_DXDY_neon<true>(s, xy, count, colors);
}