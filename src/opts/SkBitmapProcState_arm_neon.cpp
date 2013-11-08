
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBitmapProcState.h"
#include "SkBitmapProcState_filter.h"
#include "SkColorPriv.h"
#include "SkFilterProc.h"
#include "SkPaint.h"
#include "SkShader.h"   // for tilemodes
#include "SkUtilsArm.h"

// Required to ensure the table is part of the final binary.
extern const SkBitmapProcState::SampleProc32 gSkBitmapProcStateSample32_neon[];
extern const SkBitmapProcState::SampleProc16 gSkBitmapProcStateSample16_neon[];

#define   NAME_WRAP(x)  x ## _neon
#include "SkBitmapProcState_filter_neon.h"
#include "SkBitmapProcState_procs.h"

const SkBitmapProcState::SampleProc32 gSkBitmapProcStateSample32_neon[] = {
    S32_opaque_D32_nofilter_DXDY_neon,
    S32_alpha_D32_nofilter_DXDY_neon,
    S32_opaque_D32_nofilter_DX_neon,
    S32_alpha_D32_nofilter_DX_neon,
    S32_opaque_D32_filter_DXDY_neon,
    S32_alpha_D32_filter_DXDY_neon,
    S32_opaque_D32_filter_DX_neon,
    S32_alpha_D32_filter_DX_neon,

    S16_opaque_D32_nofilter_DXDY_neon,
    S16_alpha_D32_nofilter_DXDY_neon,
    S16_opaque_D32_nofilter_DX_neon,
    S16_alpha_D32_nofilter_DX_neon,
    S16_opaque_D32_filter_DXDY_neon,
    S16_alpha_D32_filter_DXDY_neon,
    S16_opaque_D32_filter_DX_neon,
    S16_alpha_D32_filter_DX_neon,

    SI8_opaque_D32_nofilter_DXDY_neon,
    SI8_alpha_D32_nofilter_DXDY_neon,
    SI8_opaque_D32_nofilter_DX_neon,
    SI8_alpha_D32_nofilter_DX_neon,
    SI8_opaque_D32_filter_DXDY_neon,
    SI8_alpha_D32_filter_DXDY_neon,
    SI8_opaque_D32_filter_DX_neon,
    SI8_alpha_D32_filter_DX_neon,

    S4444_opaque_D32_nofilter_DXDY_neon,
    S4444_alpha_D32_nofilter_DXDY_neon,
    S4444_opaque_D32_nofilter_DX_neon,
    S4444_alpha_D32_nofilter_DX_neon,
    S4444_opaque_D32_filter_DXDY_neon,
    S4444_alpha_D32_filter_DXDY_neon,
    S4444_opaque_D32_filter_DX_neon,
    S4444_alpha_D32_filter_DX_neon,

    // A8 treats alpha/opauqe the same (equally efficient)
    SA8_alpha_D32_nofilter_DXDY_neon,
    SA8_alpha_D32_nofilter_DXDY_neon,
    SA8_alpha_D32_nofilter_DX_neon,
    SA8_alpha_D32_nofilter_DX_neon,
    SA8_alpha_D32_filter_DXDY_neon,
    SA8_alpha_D32_filter_DXDY_neon,
    SA8_alpha_D32_filter_DX_neon,
    SA8_alpha_D32_filter_DX_neon
};

const SkBitmapProcState::SampleProc16 gSkBitmapProcStateSample16_neon[] = {
    S32_D16_nofilter_DXDY_neon,
    S32_D16_nofilter_DX_neon,
    S32_D16_filter_DXDY_neon,
    S32_D16_filter_DX_neon,

    S16_D16_nofilter_DXDY_neon,
    S16_D16_nofilter_DX_neon,
    S16_D16_filter_DXDY_neon,
    S16_D16_filter_DX_neon,

    SI8_D16_nofilter_DXDY_neon,
    SI8_D16_nofilter_DX_neon,
    SI8_D16_filter_DXDY_neon,
    SI8_D16_filter_DX_neon,

    // Don't support 4444 -> 565
    NULL, NULL, NULL, NULL,
    // Don't support A8 -> 565
    NULL, NULL, NULL, NULL
};

///////////////////////////////////////////////////////////////////////////////

#include <arm_neon.h>
#include "SkConvolver.h"

// Convolves horizontally along a single row. The row data is given in
// |srcData| and continues for the numValues() of the filter.
void convolveHorizontally_neon(const unsigned char* srcData,
                               const SkConvolutionFilter1D& filter,
                               unsigned char* outRow,
                               bool hasAlpha) {
    // Loop over each pixel on this row in the output image.
    int numValues = filter.numValues();
    for (int outX = 0; outX < numValues; outX++) {
        uint8x8_t coeff_mask0 = vcreate_u8(0x0100010001000100);
        uint8x8_t coeff_mask1 = vcreate_u8(0x0302030203020302);
        uint8x8_t coeff_mask2 = vcreate_u8(0x0504050405040504);
        uint8x8_t coeff_mask3 = vcreate_u8(0x0706070607060706);
        // Get the filter that determines the current output pixel.
        int filterOffset, filterLength;
        const SkConvolutionFilter1D::ConvolutionFixed* filterValues =
            filter.FilterForValue(outX, &filterOffset, &filterLength);

        // Compute the first pixel in this row that the filter affects. It will
        // touch |filterLength| pixels (4 bytes each) after this.
        const unsigned char* rowToFilter = &srcData[filterOffset * 4];

        // Apply the filter to the row to get the destination pixel in |accum|.
        int32x4_t accum = vdupq_n_s32(0);
        for (int filterX = 0; filterX < filterLength >> 2; filterX++) {
            // Load 4 coefficients
            int16x4_t coeffs, coeff0, coeff1, coeff2, coeff3;
            coeffs = vld1_s16(filterValues);
            coeff0 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_s16(coeffs), coeff_mask0));
            coeff1 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_s16(coeffs), coeff_mask1));
            coeff2 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_s16(coeffs), coeff_mask2));
            coeff3 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_s16(coeffs), coeff_mask3));

            // Load pixels and calc
            uint8x16_t pixels = vld1q_u8(rowToFilter);
            int16x8_t p01_16 = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pixels)));
            int16x8_t p23_16 = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pixels)));

            int16x4_t p0_src = vget_low_s16(p01_16);
            int16x4_t p1_src = vget_high_s16(p01_16);
            int16x4_t p2_src = vget_low_s16(p23_16);
            int16x4_t p3_src = vget_high_s16(p23_16);

            int32x4_t p0 = vmull_s16(p0_src, coeff0);
            int32x4_t p1 = vmull_s16(p1_src, coeff1);
            int32x4_t p2 = vmull_s16(p2_src, coeff2);
            int32x4_t p3 = vmull_s16(p3_src, coeff3);

            accum += p0;
            accum += p1;
            accum += p2;
            accum += p3;

            // Advance the pointers
            rowToFilter += 16;
            filterValues += 4;
        }
        int r = filterLength & 3;
        if (r) {
            const uint16_t mask[4][4] = {
                {0, 0, 0, 0},
                {0xFFFF, 0, 0, 0},
                {0xFFFF, 0xFFFF, 0, 0},
                {0xFFFF, 0xFFFF, 0xFFFF, 0}
            };
            uint16x4_t coeffs;
            int16x4_t coeff0, coeff1, coeff2;
            coeffs = vld1_u16(reinterpret_cast<const uint16_t*>(filterValues));
            coeffs &= vld1_u16(&mask[r][0]);
            coeff0 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_u16(coeffs), coeff_mask0));
            coeff1 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_u16(coeffs), coeff_mask1));
            coeff2 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_u16(coeffs), coeff_mask2));

            // Load pixels and calc
            uint8x16_t pixels = vld1q_u8(rowToFilter);
            int16x8_t p01_16 = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pixels)));
            int16x8_t p23_16 = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pixels)));
            int32x4_t p0 = vmull_s16(vget_low_s16(p01_16), coeff0);
            int32x4_t p1 = vmull_s16(vget_high_s16(p01_16), coeff1);
            int32x4_t p2 = vmull_s16(vget_low_s16(p23_16), coeff2);

            accum += p0;
            accum += p1;
            accum += p2;
        }

        // Bring this value back in range. All of the filter scaling factors
        // are in fixed point with kShiftBits bits of fractional part.
        accum = vshrq_n_s32(accum, SkConvolutionFilter1D::kShiftBits);

        // Pack and store the new pixel.
        int16x4_t accum16 = vqmovn_s32(accum);
        uint8x8_t accum8 = vqmovun_s16(vcombine_s16(accum16, accum16));
        vst1_lane_u32(reinterpret_cast<uint32_t*>(outRow), vreinterpret_u32_u8(accum8), 0);
        outRow += 4;
    }
}

// Does vertical convolution to produce one output row. The filter values and
// length are given in the first two parameters. These are applied to each
// of the rows pointed to in the |sourceDataRows| array, with each row
// being |pixelWidth| wide.
//
// The output must have room for |pixelWidth * 4| bytes.
template<bool hasAlpha>
void convolveVertically_neon(const SkConvolutionFilter1D::ConvolutionFixed* filterValues,
                             int filterLength,
                             unsigned char* const* sourceDataRows,
                             int pixelWidth,
                             unsigned char* outRow) {
    int width = pixelWidth & ~3;

    int32x4_t accum0, accum1, accum2, accum3;
    int16x4_t coeff16;

    // Output four pixels per iteration (16 bytes).
    for (int outX = 0; outX < width; outX += 4) {

        // Accumulated result for each pixel. 32 bits per RGBA channel.
        accum0 = accum1 = accum2 = accum3 = vdupq_n_s32(0);

        // Convolve with one filter coefficient per iteration.
        for (int filterY = 0; filterY < filterLength; filterY++) {

            // Duplicate the filter coefficient 4 times.
            // [16] cj cj cj cj
            coeff16 = vdup_n_s16(filterValues[filterY]);

            // Load four pixels (16 bytes) together.
            // [8] a3 b3 g3 r3 a2 b2 g2 r2 a1 b1 g1 r1 a0 b0 g0 r0
            uint8x16_t src8 = vld1q_u8(&sourceDataRows[filterY][outX << 2]);

            int16x8_t src16_01 = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(src8)));
            int16x8_t src16_23 = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(src8)));
            int16x4_t src16_0 = vget_low_s16(src16_01);
            int16x4_t src16_1 = vget_high_s16(src16_01);
            int16x4_t src16_2 = vget_low_s16(src16_23);
            int16x4_t src16_3 = vget_high_s16(src16_23);

            accum0 += vmull_s16(src16_0, coeff16);
            accum1 += vmull_s16(src16_1, coeff16);
            accum2 += vmull_s16(src16_2, coeff16);
            accum3 += vmull_s16(src16_3, coeff16);
        }

        // Shift right for fixed point implementation.
        accum0 = vshrq_n_s32(accum0, SkConvolutionFilter1D::kShiftBits);
        accum1 = vshrq_n_s32(accum1, SkConvolutionFilter1D::kShiftBits);
        accum2 = vshrq_n_s32(accum2, SkConvolutionFilter1D::kShiftBits);
        accum3 = vshrq_n_s32(accum3, SkConvolutionFilter1D::kShiftBits);

        // Packing 32 bits |accum| to 16 bits per channel (signed saturation).
        // [16] a1 b1 g1 r1 a0 b0 g0 r0
        int16x8_t accum16_0 = vcombine_s16(vqmovn_s32(accum0), vqmovn_s32(accum1));
        // [16] a3 b3 g3 r3 a2 b2 g2 r2
        int16x8_t accum16_1 = vcombine_s16(vqmovn_s32(accum2), vqmovn_s32(accum3));

        // Packing 16 bits |accum| to 8 bits per channel (unsigned saturation).
        // [8] a3 b3 g3 r3 a2 b2 g2 r2 a1 b1 g1 r1 a0 b0 g0 r0
        uint8x16_t accum8 = vcombine_u8(vqmovun_s16(accum16_0), vqmovun_s16(accum16_1));

        if (hasAlpha) {
            // Compute the max(ri, gi, bi) for each pixel.
            // [8] xx a3 b3 g3 xx a2 b2 g2 xx a1 b1 g1 xx a0 b0 g0
            uint8x16_t a = vreinterpretq_u8_u32(vshrq_n_u32(vreinterpretq_u32_u8(accum8), 8));
            // [8] xx xx xx max3 xx xx xx max2 xx xx xx max1 xx xx xx max0
            uint8x16_t b = vmaxq_u8(a, accum8); // Max of r and g
            // [8] xx xx a3 b3 xx xx a2 b2 xx xx a1 b1 xx xx a0 b0
            a = vreinterpretq_u8_u32(vshrq_n_u32(vreinterpretq_u32_u8(accum8), 16));
            // [8] xx xx xx max3 xx xx xx max2 xx xx xx max1 xx xx xx max0
            b = vmaxq_u8(a, b); // Max of r and g and b.
            // [8] max3 00 00 00 max2 00 00 00 max1 00 00 00 max0 00 00 00
            b = vreinterpretq_u8_u32(vshlq_n_u32(vreinterpretq_u32_u8(b), 24));

            // Make sure the value of alpha channel is always larger than maximum
            // value of color channels.
            accum8 = vmaxq_u8(b, accum8);
        } else {
            // Set value of alpha channels to 0xFF.
            accum8 = vreinterpretq_u8_u32(vreinterpretq_u32_u8(accum8) | vdupq_n_u32(0xFF000000));
        }

        // Store the convolution result (16 bytes) and advance the pixel pointers.
        vst1q_u8(outRow, accum8);
        outRow += 16;
    }

    // Process the leftovers when the width of the output is not divisible
    // by 4, that is at most 3 pixels.
    int r = pixelWidth & 3;
    if (r) {

        accum0 = accum1 = accum2 = vdupq_n_s32(0);

        for (int filterY = 0; filterY < filterLength; ++filterY) {
            coeff16 = vdup_n_s16(filterValues[filterY]);

            // [8] a3 b3 g3 r3 a2 b2 g2 r2 a1 b1 g1 r1 a0 b0 g0 r0
            uint8x16_t src8 = vld1q_u8(&sourceDataRows[filterY][width << 2]);

            int16x8_t src16_01 = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(src8)));
            int16x8_t src16_23 = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(src8)));
            int16x4_t src16_0 = vget_low_s16(src16_01);
            int16x4_t src16_1 = vget_high_s16(src16_01);
            int16x4_t src16_2 = vget_low_s16(src16_23);

            accum0 += vmull_s16(src16_0, coeff16);
            accum1 += vmull_s16(src16_1, coeff16);
            accum2 += vmull_s16(src16_2, coeff16);
        }

        accum0 = vshrq_n_s32(accum0, SkConvolutionFilter1D::kShiftBits);
        accum1 = vshrq_n_s32(accum1, SkConvolutionFilter1D::kShiftBits);
        accum2 = vshrq_n_s32(accum2, SkConvolutionFilter1D::kShiftBits);

        int16x8_t accum16_0 = vcombine_s16(vqmovn_s32(accum0), vqmovn_s32(accum1));
        int16x8_t accum16_1 = vcombine_s16(vqmovn_s32(accum2), vqmovn_s32(accum2));

        uint8x16_t accum8 = vcombine_u8(vqmovun_s16(accum16_0), vqmovun_s16(accum16_1));

        if (hasAlpha) {
            // Compute the max(ri, gi, bi) for each pixel.
            // [8] xx a3 b3 g3 xx a2 b2 g2 xx a1 b1 g1 xx a0 b0 g0
            uint8x16_t a = vreinterpretq_u8_u32(vshrq_n_u32(vreinterpretq_u32_u8(accum8), 8));
            // [8] xx xx xx max3 xx xx xx max2 xx xx xx max1 xx xx xx max0
            uint8x16_t b = vmaxq_u8(a, accum8); // Max of r and g
            // [8] xx xx a3 b3 xx xx a2 b2 xx xx a1 b1 xx xx a0 b0
            a = vreinterpretq_u8_u32(vshrq_n_u32(vreinterpretq_u32_u8(accum8), 16));
            // [8] xx xx xx max3 xx xx xx max2 xx xx xx max1 xx xx xx max0
            b = vmaxq_u8(a, b); // Max of r and g and b.
            // [8] max3 00 00 00 max2 00 00 00 max1 00 00 00 max0 00 00 00
            b = vreinterpretq_u8_u32(vshlq_n_u32(vreinterpretq_u32_u8(b), 24));

            // Make sure the value of alpha channel is always larger than maximum
            // value of color channels.
            accum8 = vmaxq_u8(b, accum8);
        } else {
            // Set value of alpha channels to 0xFF.
            accum8 = vreinterpretq_u8_u32(vreinterpretq_u32_u8(accum8) | vdupq_n_u32(0xFF000000));
        }

        switch(r) {
        case 1:
            vst1q_lane_u32(reinterpret_cast<uint32_t*>(outRow), vreinterpretq_u32_u8(accum8), 0);
            break;
        case 2:
            vst1_u32(reinterpret_cast<uint32_t*>(outRow),
                     vreinterpret_u32_u8(vget_low_u8(accum8)));
            break;
        case 3:
            vst1_u32(reinterpret_cast<uint32_t*>(outRow),
                     vreinterpret_u32_u8(vget_low_u8(accum8)));
            vst1q_lane_u32(reinterpret_cast<uint32_t*>(outRow+8), vreinterpretq_u32_u8(accum8), 2);
            break;
        }
    }
}

void convolveVertically_neon(const SkConvolutionFilter1D::ConvolutionFixed* filterValues,
                             int filterLength,
                             unsigned char* const* sourceDataRows,
                             int pixelWidth,
                             unsigned char* outRow,
                             bool sourceHasAlpha) {
    if (sourceHasAlpha) {
        convolveVertically_neon<true>(filterValues, filterLength,
                                      sourceDataRows, pixelWidth,
                                      outRow);
    } else {
        convolveVertically_neon<false>(filterValues, filterLength,
                                       sourceDataRows, pixelWidth,
                                       outRow);
    }
}

// Convolves horizontally along four rows. The row data is given in
// |src_data| and continues for the num_values() of the filter.
// The algorithm is almost same as |ConvolveHorizontally_SSE2|. Please
// refer to that function for detailed comments.
void convolve4RowsHorizontally_neon(const unsigned char* srcData[4],
                                    const SkConvolutionFilter1D& filter,
                                    unsigned char* outRow[4]) {

    uint8x8_t coeff_mask0 = vcreate_u8(0x0100010001000100);
    uint8x8_t coeff_mask1 = vcreate_u8(0x0302030203020302);
    uint8x8_t coeff_mask2 = vcreate_u8(0x0504050405040504);
    uint8x8_t coeff_mask3 = vcreate_u8(0x0706070607060706);
    int num_values = filter.numValues();

    int filterOffset, filterLength;
    // |mask| will be used to decimate all extra filter coefficients that are
    // loaded by SIMD when |filter_length| is not divisible by 4.
    // mask[0] is not used in following algorithm.
    const uint16_t mask[4][4] = {
        {0, 0, 0, 0},
        {0xFFFF, 0, 0, 0},
        {0xFFFF, 0xFFFF, 0, 0},
        {0xFFFF, 0xFFFF, 0xFFFF, 0}
    };

    // Output one pixel each iteration, calculating all channels (RGBA) together.
    for (int outX = 0; outX < num_values; outX++) {

        const SkConvolutionFilter1D::ConvolutionFixed* filterValues =
        filter.FilterForValue(outX, &filterOffset, &filterLength);

        // four pixels in a column per iteration.
        int32x4_t accum0 = vdupq_n_s32(0);
        int32x4_t accum1 = vdupq_n_s32(0);
        int32x4_t accum2 = vdupq_n_s32(0);
        int32x4_t accum3 = vdupq_n_s32(0);

        int start = (filterOffset<<2);

        // We will load and accumulate with four coefficients per iteration.
        for (int filter_x = 0; filter_x < (filterLength >> 2); filter_x++) {
            int16x4_t coeffs, coeff0, coeff1, coeff2, coeff3;

            coeffs = vld1_s16(filterValues);
            coeff0 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_s16(coeffs), coeff_mask0));
            coeff1 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_s16(coeffs), coeff_mask1));
            coeff2 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_s16(coeffs), coeff_mask2));
            coeff3 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_s16(coeffs), coeff_mask3));

            uint8x16_t pixels;
            int16x8_t p01_16, p23_16;
            int32x4_t p0, p1, p2, p3;


#define ITERATION(src, accum)                                       \
    pixels = vld1q_u8(src);                                         \
    p01_16 = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pixels)));  \
    p23_16 = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pixels))); \
    p0 = vmull_s16(vget_low_s16(p01_16), coeff0);                   \
    p1 = vmull_s16(vget_high_s16(p01_16), coeff1);                  \
    p2 = vmull_s16(vget_low_s16(p23_16), coeff2);                   \
    p3 = vmull_s16(vget_high_s16(p23_16), coeff3);                  \
    accum += p0;                                                    \
    accum += p1;                                                    \
    accum += p2;                                                    \
    accum += p3

            ITERATION(srcData[0] + start, accum0);
            ITERATION(srcData[1] + start, accum1);
            ITERATION(srcData[2] + start, accum2);
            ITERATION(srcData[3] + start, accum3);

            start += 16;
            filterValues += 4;
        }

        int r = filterLength & 3;
        if (r) {
            int16x4_t coeffs, coeff0, coeff1, coeff2, coeff3;
            coeffs = vld1_s16(filterValues);
            coeffs &= vreinterpret_s16_u16(vld1_u16(&mask[r][0]));
            coeff0 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_s16(coeffs), coeff_mask0));
            coeff1 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_s16(coeffs), coeff_mask1));
            coeff2 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_s16(coeffs), coeff_mask2));
            coeff3 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_s16(coeffs), coeff_mask3));

            uint8x16_t pixels;
            int16x8_t p01_16, p23_16;
            int32x4_t p0, p1, p2, p3;

            ITERATION(srcData[0] + start, accum0);
            ITERATION(srcData[1] + start, accum1);
            ITERATION(srcData[2] + start, accum2);
            ITERATION(srcData[3] + start, accum3);
        }

        int16x4_t accum16;
        uint8x8_t res0, res1, res2, res3;

#define PACK_RESULT(accum, res)                                         \
        accum = vshrq_n_s32(accum, SkConvolutionFilter1D::kShiftBits);  \
        accum16 = vqmovn_s32(accum);                                    \
        res = vqmovun_s16(vcombine_s16(accum16, accum16));

        PACK_RESULT(accum0, res0);
        PACK_RESULT(accum1, res1);
        PACK_RESULT(accum2, res2);
        PACK_RESULT(accum3, res3);

        vst1_lane_u32(reinterpret_cast<uint32_t*>(outRow[0]), vreinterpret_u32_u8(res0), 0);
        vst1_lane_u32(reinterpret_cast<uint32_t*>(outRow[1]), vreinterpret_u32_u8(res1), 0);
        vst1_lane_u32(reinterpret_cast<uint32_t*>(outRow[2]), vreinterpret_u32_u8(res2), 0);
        vst1_lane_u32(reinterpret_cast<uint32_t*>(outRow[3]), vreinterpret_u32_u8(res3), 0);
        outRow[0] += 4;
        outRow[1] += 4;
        outRow[2] += 4;
        outRow[3] += 4;
    }
}

void applySIMDPadding_neon(SkConvolutionFilter1D *filter) {
    // Padding |paddingCount| of more dummy coefficients after the coefficients
    // of last filter to prevent SIMD instructions which load 8 or 16 bytes
    // together to access invalid memory areas. We are not trying to align the
    // coefficients right now due to the opaqueness of <vector> implementation.
    // This has to be done after all |AddFilter| calls.
    for (int i = 0; i < 8; ++i) {
        filter->addFilterValue(static_cast<SkConvolutionFilter1D::ConvolutionFixed>(0));
    }
}

void platformConvolutionProcs_arm_neon(SkConvolutionProcs* procs) {
    procs->fExtraHorizontalReads = 3;
    procs->fConvolveVertically = &convolveVertically_neon;
    procs->fConvolve4RowsHorizontally = &convolve4RowsHorizontally_neon;
    procs->fConvolveHorizontally = &convolveHorizontally_neon;
    procs->fApplySIMDPadding = &applySIMDPadding_neon;
}
