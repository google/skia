/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapFilter_opts_DEFINED
#define SkBitmapFilter_opts_DEFINED

#include "SkConvolver.h"

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include <immintrin.h>
#elif defined(SK_ARM_HAS_NEON)
    #include <arm_neon.h>
#endif

namespace SK_OPTS_NS {

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2

    static SK_ALWAYS_INLINE void AccumRemainder(const unsigned char* pixelsLeft,
            const SkConvolutionFilter1D::ConvolutionFixed* filterValues, __m128i& accum, int r) {
        int remainder[4] = {0};
        for (int i = 0; i < r; i++) {
            SkConvolutionFilter1D::ConvolutionFixed coeff = filterValues[i];
            remainder[0] += coeff * pixelsLeft[i * 4 + 0];
            remainder[1] += coeff * pixelsLeft[i * 4 + 1];
            remainder[2] += coeff * pixelsLeft[i * 4 + 2];
            remainder[3] += coeff * pixelsLeft[i * 4 + 3];
        }
        __m128i t = _mm_setr_epi32(remainder[0], remainder[1], remainder[2], remainder[3]);
        accum = _mm_add_epi32(accum, t);
    }

    // Convolves horizontally along a single row. The row data is given in
    // |srcData| and continues for the numValues() of the filter.
    void convolve_horizontally(const unsigned char* srcData,
                               const SkConvolutionFilter1D& filter,
                               unsigned char* outRow,
                               bool /*hasAlpha*/) {
        // Output one pixel each iteration, calculating all channels (RGBA) together.
        int numValues = filter.numValues();
        for (int outX = 0; outX < numValues; outX++) {
            // Get the filter that determines the current output pixel.
            int filterOffset, filterLength;
            const SkConvolutionFilter1D::ConvolutionFixed* filterValues =
                filter.FilterForValue(outX, &filterOffset, &filterLength);

            // Compute the first pixel in this row that the filter affects. It will
            // touch |filterLength| pixels (4 bytes each) after this.
            const unsigned char* rowToFilter = &srcData[filterOffset * 4];

            __m128i zero = _mm_setzero_si128();
            __m128i accum = _mm_setzero_si128();

            // We will load and accumulate with four coefficients per iteration.
            for (int filterX = 0; filterX < filterLength >> 2; filterX++) {
                // Load 4 coefficients => duplicate 1st and 2nd of them for all channels.
                __m128i coeff, coeff16;
                // [16] xx xx xx xx c3 c2 c1 c0
                coeff = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(filterValues));
                // [16] xx xx xx xx c1 c1 c0 c0
                coeff16 = _mm_shufflelo_epi16(coeff, _MM_SHUFFLE(1, 1, 0, 0));
                // [16] c1 c1 c1 c1 c0 c0 c0 c0
                coeff16 = _mm_unpacklo_epi16(coeff16, coeff16);

                // Load four pixels => unpack the first two pixels to 16 bits =>
                // multiply with coefficients => accumulate the convolution result.
                // [8] a3 b3 g3 r3 a2 b2 g2 r2 a1 b1 g1 r1 a0 b0 g0 r0
                __m128i src8 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(rowToFilter));
                // [16] a1 b1 g1 r1 a0 b0 g0 r0
                __m128i src16 = _mm_unpacklo_epi8(src8, zero);
                __m128i mul_hi = _mm_mulhi_epi16(src16, coeff16);
                __m128i mul_lo = _mm_mullo_epi16(src16, coeff16);
                // [32]  a0*c0 b0*c0 g0*c0 r0*c0
                __m128i t = _mm_unpacklo_epi16(mul_lo, mul_hi);
                accum = _mm_add_epi32(accum, t);
                // [32]  a1*c1 b1*c1 g1*c1 r1*c1
                t = _mm_unpackhi_epi16(mul_lo, mul_hi);
                accum = _mm_add_epi32(accum, t);

                // Duplicate 3rd and 4th coefficients for all channels =>
                // unpack the 3rd and 4th pixels to 16 bits => multiply with coefficients
                // => accumulate the convolution results.
                // [16] xx xx xx xx c3 c3 c2 c2
                coeff16 = _mm_shufflelo_epi16(coeff, _MM_SHUFFLE(3, 3, 2, 2));
                // [16] c3 c3 c3 c3 c2 c2 c2 c2
                coeff16 = _mm_unpacklo_epi16(coeff16, coeff16);
                // [16] a3 g3 b3 r3 a2 g2 b2 r2
                src16 = _mm_unpackhi_epi8(src8, zero);
                mul_hi = _mm_mulhi_epi16(src16, coeff16);
                mul_lo = _mm_mullo_epi16(src16, coeff16);
                // [32]  a2*c2 b2*c2 g2*c2 r2*c2
                t = _mm_unpacklo_epi16(mul_lo, mul_hi);
                accum = _mm_add_epi32(accum, t);
                // [32]  a3*c3 b3*c3 g3*c3 r3*c3
                t = _mm_unpackhi_epi16(mul_lo, mul_hi);
                accum = _mm_add_epi32(accum, t);

                // Advance the pixel and coefficients pointers.
                rowToFilter += 16;
                filterValues += 4;
            }

            // When |filterLength| is not divisible by 4, we accumulate the last 1 - 3
            // coefficients one at a time.
            int r = filterLength & 3;
            if (r) {
                int remainderOffset = (filterOffset + filterLength - r) * 4;
                AccumRemainder(srcData + remainderOffset, filterValues, accum, r);
            }

            // Shift right for fixed point implementation.
            accum = _mm_srai_epi32(accum, SkConvolutionFilter1D::kShiftBits);

            // Packing 32 bits |accum| to 16 bits per channel (signed saturation).
            accum = _mm_packs_epi32(accum, zero);
            // Packing 16 bits |accum| to 8 bits per channel (unsigned saturation).
            accum = _mm_packus_epi16(accum, zero);

            // Store the pixel value of 32 bits.
            *(reinterpret_cast<int*>(outRow)) = _mm_cvtsi128_si32(accum);
            outRow += 4;
        }
    }

    // Convolves horizontally along four rows. The row data is given in
    // |srcData| and continues for the numValues() of the filter.
    // The algorithm is almost same as |convolve_horizontally|. Please
    // refer to that function for detailed comments.
    void convolve_4_rows_horizontally(const unsigned char* srcData[4],
                                      const SkConvolutionFilter1D& filter,
                                      unsigned char* outRow[4],
                                      size_t outRowBytes) {
        SkDEBUGCODE(const unsigned char* out_row_0_start = outRow[0];)

        // Output one pixel each iteration, calculating all channels (RGBA) together.
        int numValues = filter.numValues();
        for (int outX = 0; outX < numValues; outX++) {
            int filterOffset, filterLength;
            const SkConvolutionFilter1D::ConvolutionFixed* filterValues =
                filter.FilterForValue(outX, &filterOffset, &filterLength);

            __m128i zero = _mm_setzero_si128();

            // four pixels in a column per iteration.
            __m128i accum0 = _mm_setzero_si128();
            __m128i accum1 = _mm_setzero_si128();
            __m128i accum2 = _mm_setzero_si128();
            __m128i accum3 = _mm_setzero_si128();

            int start = filterOffset * 4;
            // We will load and accumulate with four coefficients per iteration.
            for (int filterX = 0; filterX < (filterLength >> 2); filterX++) {
                __m128i coeff, coeff16lo, coeff16hi;
                // [16] xx xx xx xx c3 c2 c1 c0
                coeff = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(filterValues));
                // [16] xx xx xx xx c1 c1 c0 c0
                coeff16lo = _mm_shufflelo_epi16(coeff, _MM_SHUFFLE(1, 1, 0, 0));
                // [16] c1 c1 c1 c1 c0 c0 c0 c0
                coeff16lo = _mm_unpacklo_epi16(coeff16lo, coeff16lo);
                // [16] xx xx xx xx c3 c3 c2 c2
                coeff16hi = _mm_shufflelo_epi16(coeff, _MM_SHUFFLE(3, 3, 2, 2));
                // [16] c3 c3 c3 c3 c2 c2 c2 c2
                coeff16hi = _mm_unpacklo_epi16(coeff16hi, coeff16hi);

                __m128i src8, src16, mul_hi, mul_lo, t;

#define ITERATION(src, accum)                                                    \
                src8 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));   \
                src16 = _mm_unpacklo_epi8(src8, zero);                           \
                mul_hi = _mm_mulhi_epi16(src16, coeff16lo);                      \
                mul_lo = _mm_mullo_epi16(src16, coeff16lo);                      \
                t = _mm_unpacklo_epi16(mul_lo, mul_hi);                          \
                accum = _mm_add_epi32(accum, t);                                 \
                t = _mm_unpackhi_epi16(mul_lo, mul_hi);                          \
                accum = _mm_add_epi32(accum, t);                                 \
                src16 = _mm_unpackhi_epi8(src8, zero);                           \
                mul_hi = _mm_mulhi_epi16(src16, coeff16hi);                      \
                mul_lo = _mm_mullo_epi16(src16, coeff16hi);                      \
                t = _mm_unpacklo_epi16(mul_lo, mul_hi);                          \
                accum = _mm_add_epi32(accum, t);                                 \
                t = _mm_unpackhi_epi16(mul_lo, mul_hi);                          \
                accum = _mm_add_epi32(accum, t)

                ITERATION(srcData[0] + start, accum0);
                ITERATION(srcData[1] + start, accum1);
                ITERATION(srcData[2] + start, accum2);
                ITERATION(srcData[3] + start, accum3);

                start += 16;
                filterValues += 4;
            }

            int r = filterLength & 3;
            if (r) {
                int remainderOffset = (filterOffset + filterLength - r) * 4;
                AccumRemainder(srcData[0] + remainderOffset, filterValues, accum0, r);
                AccumRemainder(srcData[1] + remainderOffset, filterValues, accum1, r);
                AccumRemainder(srcData[2] + remainderOffset, filterValues, accum2, r);
                AccumRemainder(srcData[3] + remainderOffset, filterValues, accum3, r);
            }

            accum0 = _mm_srai_epi32(accum0, SkConvolutionFilter1D::kShiftBits);
            accum0 = _mm_packs_epi32(accum0, zero);
            accum0 = _mm_packus_epi16(accum0, zero);
            accum1 = _mm_srai_epi32(accum1, SkConvolutionFilter1D::kShiftBits);
            accum1 = _mm_packs_epi32(accum1, zero);
            accum1 = _mm_packus_epi16(accum1, zero);
            accum2 = _mm_srai_epi32(accum2, SkConvolutionFilter1D::kShiftBits);
            accum2 = _mm_packs_epi32(accum2, zero);
            accum2 = _mm_packus_epi16(accum2, zero);
            accum3 = _mm_srai_epi32(accum3, SkConvolutionFilter1D::kShiftBits);
            accum3 = _mm_packs_epi32(accum3, zero);
            accum3 = _mm_packus_epi16(accum3, zero);

            // We seem to be running off the edge here (chromium:491660).
            SkASSERT(((size_t)outRow[0] - (size_t)out_row_0_start) < outRowBytes);

            *(reinterpret_cast<int*>(outRow[0])) = _mm_cvtsi128_si32(accum0);
            *(reinterpret_cast<int*>(outRow[1])) = _mm_cvtsi128_si32(accum1);
            *(reinterpret_cast<int*>(outRow[2])) = _mm_cvtsi128_si32(accum2);
            *(reinterpret_cast<int*>(outRow[3])) = _mm_cvtsi128_si32(accum3);

            outRow[0] += 4;
            outRow[1] += 4;
            outRow[2] += 4;
            outRow[3] += 4;
        }
    }

    // Does vertical convolution to produce one output row. The filter values and
    // length are given in the first two parameters. These are applied to each
    // of the rows pointed to in the |sourceDataRows| array, with each row
    // being |pixelWidth| wide.
    //
    // The output must have room for |pixelWidth * 4| bytes.
    template<bool hasAlpha>
    void ConvolveVertically(const SkConvolutionFilter1D::ConvolutionFixed* filterValues,
                            int filterLength,
                            unsigned char* const* sourceDataRows,
                            int pixelWidth,
                            unsigned char* outRow) {
        // Output four pixels per iteration (16 bytes).
        int width = pixelWidth & ~3;
        __m128i zero = _mm_setzero_si128();
        for (int outX = 0; outX < width; outX += 4) {
            // Accumulated result for each pixel. 32 bits per RGBA channel.
            __m128i accum0 = _mm_setzero_si128();
            __m128i accum1 = _mm_setzero_si128();
            __m128i accum2 = _mm_setzero_si128();
            __m128i accum3 = _mm_setzero_si128();

            // Convolve with one filter coefficient per iteration.
            for (int filterY = 0; filterY < filterLength; filterY++) {

                // Duplicate the filter coefficient 8 times.
                // [16] cj cj cj cj cj cj cj cj
                __m128i coeff16 = _mm_set1_epi16(filterValues[filterY]);

                // Load four pixels (16 bytes) together.
                // [8] a3 b3 g3 r3 a2 b2 g2 r2 a1 b1 g1 r1 a0 b0 g0 r0
                const __m128i* src = reinterpret_cast<const __m128i*>(
                    &sourceDataRows[filterY][outX << 2]);
                __m128i src8 = _mm_loadu_si128(src);

                // Unpack 1st and 2nd pixels from 8 bits to 16 bits for each channels =>
                // multiply with current coefficient => accumulate the result.
                // [16] a1 b1 g1 r1 a0 b0 g0 r0
                __m128i src16 = _mm_unpacklo_epi8(src8, zero);
                __m128i mul_hi = _mm_mulhi_epi16(src16, coeff16);
                __m128i mul_lo = _mm_mullo_epi16(src16, coeff16);
                // [32] a0 b0 g0 r0
                __m128i t = _mm_unpacklo_epi16(mul_lo, mul_hi);
                accum0 = _mm_add_epi32(accum0, t);
                // [32] a1 b1 g1 r1
                t = _mm_unpackhi_epi16(mul_lo, mul_hi);
                accum1 = _mm_add_epi32(accum1, t);

                // Unpack 3rd and 4th pixels from 8 bits to 16 bits for each channels =>
                // multiply with current coefficient => accumulate the result.
                // [16] a3 b3 g3 r3 a2 b2 g2 r2
                src16 = _mm_unpackhi_epi8(src8, zero);
                mul_hi = _mm_mulhi_epi16(src16, coeff16);
                mul_lo = _mm_mullo_epi16(src16, coeff16);
                // [32] a2 b2 g2 r2
                t = _mm_unpacklo_epi16(mul_lo, mul_hi);
                accum2 = _mm_add_epi32(accum2, t);
                // [32] a3 b3 g3 r3
                t = _mm_unpackhi_epi16(mul_lo, mul_hi);
                accum3 = _mm_add_epi32(accum3, t);
            }

            // Shift right for fixed point implementation.
            accum0 = _mm_srai_epi32(accum0, SkConvolutionFilter1D::kShiftBits);
            accum1 = _mm_srai_epi32(accum1, SkConvolutionFilter1D::kShiftBits);
            accum2 = _mm_srai_epi32(accum2, SkConvolutionFilter1D::kShiftBits);
            accum3 = _mm_srai_epi32(accum3, SkConvolutionFilter1D::kShiftBits);

            // Packing 32 bits |accum| to 16 bits per channel (signed saturation).
            // [16] a1 b1 g1 r1 a0 b0 g0 r0
            accum0 = _mm_packs_epi32(accum0, accum1);
            // [16] a3 b3 g3 r3 a2 b2 g2 r2
            accum2 = _mm_packs_epi32(accum2, accum3);

            // Packing 16 bits |accum| to 8 bits per channel (unsigned saturation).
            // [8] a3 b3 g3 r3 a2 b2 g2 r2 a1 b1 g1 r1 a0 b0 g0 r0
            accum0 = _mm_packus_epi16(accum0, accum2);

            if (hasAlpha) {
                // Compute the max(ri, gi, bi) for each pixel.
                // [8] xx a3 b3 g3 xx a2 b2 g2 xx a1 b1 g1 xx a0 b0 g0
                __m128i a = _mm_srli_epi32(accum0, 8);
                // [8] xx xx xx max3 xx xx xx max2 xx xx xx max1 xx xx xx max0
                __m128i b = _mm_max_epu8(a, accum0);  // Max of r and g.
                // [8] xx xx a3 b3 xx xx a2 b2 xx xx a1 b1 xx xx a0 b0
                a = _mm_srli_epi32(accum0, 16);
                // [8] xx xx xx max3 xx xx xx max2 xx xx xx max1 xx xx xx max0
                b = _mm_max_epu8(a, b);  // Max of r and g and b.
                // [8] max3 00 00 00 max2 00 00 00 max1 00 00 00 max0 00 00 00
                b = _mm_slli_epi32(b, 24);

                // Make sure the value of alpha channel is always larger than maximum
                // value of color channels.
                accum0 = _mm_max_epu8(b, accum0);
            } else {
                // Set value of alpha channels to 0xFF.
                __m128i mask = _mm_set1_epi32(0xff000000);
                accum0 = _mm_or_si128(accum0, mask);
            }

            // Store the convolution result (16 bytes) and advance the pixel pointers.
            _mm_storeu_si128(reinterpret_cast<__m128i*>(outRow), accum0);
            outRow += 16;
        }

        // When the width of the output is not divisible by 4, We need to save one
        // pixel (4 bytes) each time. And also the fourth pixel is always absent.
        int r = pixelWidth & 3;
        if (r) {
            __m128i accum0 = _mm_setzero_si128();
            __m128i accum1 = _mm_setzero_si128();
            __m128i accum2 = _mm_setzero_si128();
            for (int filterY = 0; filterY < filterLength; ++filterY) {
                __m128i coeff16 = _mm_set1_epi16(filterValues[filterY]);
                // [8] a3 b3 g3 r3 a2 b2 g2 r2 a1 b1 g1 r1 a0 b0 g0 r0
                const __m128i* src = reinterpret_cast<const __m128i*>(
                    &sourceDataRows[filterY][width << 2]);
                __m128i src8 = _mm_loadu_si128(src);
                // [16] a1 b1 g1 r1 a0 b0 g0 r0
                __m128i src16 = _mm_unpacklo_epi8(src8, zero);
                __m128i mul_hi = _mm_mulhi_epi16(src16, coeff16);
                __m128i mul_lo = _mm_mullo_epi16(src16, coeff16);
                // [32] a0 b0 g0 r0
                __m128i t = _mm_unpacklo_epi16(mul_lo, mul_hi);
                accum0 = _mm_add_epi32(accum0, t);
                // [32] a1 b1 g1 r1
                t = _mm_unpackhi_epi16(mul_lo, mul_hi);
                accum1 = _mm_add_epi32(accum1, t);
                // [16] a3 b3 g3 r3 a2 b2 g2 r2
                src16 = _mm_unpackhi_epi8(src8, zero);
                mul_hi = _mm_mulhi_epi16(src16, coeff16);
                mul_lo = _mm_mullo_epi16(src16, coeff16);
                // [32] a2 b2 g2 r2
                t = _mm_unpacklo_epi16(mul_lo, mul_hi);
                accum2 = _mm_add_epi32(accum2, t);
            }

            accum0 = _mm_srai_epi32(accum0, SkConvolutionFilter1D::kShiftBits);
            accum1 = _mm_srai_epi32(accum1, SkConvolutionFilter1D::kShiftBits);
            accum2 = _mm_srai_epi32(accum2, SkConvolutionFilter1D::kShiftBits);
            // [16] a1 b1 g1 r1 a0 b0 g0 r0
            accum0 = _mm_packs_epi32(accum0, accum1);
            // [16] a3 b3 g3 r3 a2 b2 g2 r2
            accum2 = _mm_packs_epi32(accum2, zero);
            // [8] a3 b3 g3 r3 a2 b2 g2 r2 a1 b1 g1 r1 a0 b0 g0 r0
            accum0 = _mm_packus_epi16(accum0, accum2);
            if (hasAlpha) {
                // [8] xx a3 b3 g3 xx a2 b2 g2 xx a1 b1 g1 xx a0 b0 g0
                __m128i a = _mm_srli_epi32(accum0, 8);
                // [8] xx xx xx max3 xx xx xx max2 xx xx xx max1 xx xx xx max0
                __m128i b = _mm_max_epu8(a, accum0);  // Max of r and g.
                // [8] xx xx a3 b3 xx xx a2 b2 xx xx a1 b1 xx xx a0 b0
                a = _mm_srli_epi32(accum0, 16);
                // [8] xx xx xx max3 xx xx xx max2 xx xx xx max1 xx xx xx max0
                b = _mm_max_epu8(a, b);  // Max of r and g and b.
                // [8] max3 00 00 00 max2 00 00 00 max1 00 00 00 max0 00 00 00
                b = _mm_slli_epi32(b, 24);
                accum0 = _mm_max_epu8(b, accum0);
            } else {
                __m128i mask = _mm_set1_epi32(0xff000000);
                accum0 = _mm_or_si128(accum0, mask);
            }

            for (int i = 0; i < r; i++) {
                *(reinterpret_cast<int*>(outRow)) = _mm_cvtsi128_si32(accum0);
                accum0 = _mm_srli_si128(accum0, 4);
                outRow += 4;
            }
        }
    }

#elif defined(SK_ARM_HAS_NEON)

    static SK_ALWAYS_INLINE void AccumRemainder(const unsigned char* pixelsLeft,
            const SkConvolutionFilter1D::ConvolutionFixed* filterValues, int32x4_t& accum, int r) {
        int remainder[4] = {0};
        for (int i = 0; i < r; i++) {
            SkConvolutionFilter1D::ConvolutionFixed coeff = filterValues[i];
            remainder[0] += coeff * pixelsLeft[i * 4 + 0];
            remainder[1] += coeff * pixelsLeft[i * 4 + 1];
            remainder[2] += coeff * pixelsLeft[i * 4 + 2];
            remainder[3] += coeff * pixelsLeft[i * 4 + 3];
        }
        int32x4_t t = {remainder[0], remainder[1], remainder[2], remainder[3]};
        accum += t;
    }

    // Convolves horizontally along a single row. The row data is given in
    // |srcData| and continues for the numValues() of the filter.
    void convolve_horizontally(const unsigned char* srcData,
                               const SkConvolutionFilter1D& filter,
                               unsigned char* outRow,
                               bool /*hasAlpha*/) {
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
                int remainder_offset = (filterOffset + filterLength - r) * 4;
                AccumRemainder(srcData + remainder_offset, filterValues, accum, r);
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

    // Convolves horizontally along four rows. The row data is given in
    // |srcData| and continues for the numValues() of the filter.
    // The algorithm is almost same as |convolve_horizontally|. Please
    // refer to that function for detailed comments.
    void convolve_4_rows_horizontally(const unsigned char* srcData[4],
                                      const SkConvolutionFilter1D& filter,
                                      unsigned char* outRow[4],
                                      size_t outRowBytes) {
        // Output one pixel each iteration, calculating all channels (RGBA) together.
        int numValues = filter.numValues();
        for (int outX = 0; outX < numValues; outX++) {

            int filterOffset, filterLength;
            const SkConvolutionFilter1D::ConvolutionFixed* filterValues =
            filter.FilterForValue(outX, &filterOffset, &filterLength);

            // four pixels in a column per iteration.
            int32x4_t accum0 = vdupq_n_s32(0);
            int32x4_t accum1 = vdupq_n_s32(0);
            int32x4_t accum2 = vdupq_n_s32(0);
            int32x4_t accum3 = vdupq_n_s32(0);

            uint8x8_t coeff_mask0 = vcreate_u8(0x0100010001000100);
            uint8x8_t coeff_mask1 = vcreate_u8(0x0302030203020302);
            uint8x8_t coeff_mask2 = vcreate_u8(0x0504050405040504);
            uint8x8_t coeff_mask3 = vcreate_u8(0x0706070607060706);

            int start = filterOffset * 4;

            // We will load and accumulate with four coefficients per iteration.
            for (int filterX = 0; filterX < (filterLength >> 2); filterX++) {
                int16x4_t coeffs, coeff0, coeff1, coeff2, coeff3;

                coeffs = vld1_s16(filterValues);
                coeff0 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_s16(coeffs), coeff_mask0));
                coeff1 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_s16(coeffs), coeff_mask1));
                coeff2 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_s16(coeffs), coeff_mask2));
                coeff3 = vreinterpret_s16_u8(vtbl1_u8(vreinterpret_u8_s16(coeffs), coeff_mask3));

                uint8x16_t pixels;
                int16x8_t p01_16, p23_16;
                int32x4_t p0, p1, p2, p3;

#define ITERATION(src, accum)                                                   \
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
                int remainder_offset = (filterOffset + filterLength - r) * 4;
                AccumRemainder(srcData[0] + remainder_offset, filterValues, accum0, r);
                AccumRemainder(srcData[1] + remainder_offset, filterValues, accum1, r);
                AccumRemainder(srcData[2] + remainder_offset, filterValues, accum2, r);
                AccumRemainder(srcData[3] + remainder_offset, filterValues, accum3, r);
            }

            int16x4_t accum16;
            uint8x8_t res0, res1, res2, res3;

#define PACK_RESULT(accum, res)                                             \
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


    // Does vertical convolution to produce one output row. The filter values and
    // length are given in the first two parameters. These are applied to each
    // of the rows pointed to in the |sourceDataRows| array, with each row
    // being |pixelWidth| wide.
    //
    // The output must have room for |pixelWidth * 4| bytes.
    template<bool hasAlpha>
    void ConvolveVertically(const SkConvolutionFilter1D::ConvolutionFixed* filterValues,
                            int filterLength,
                            unsigned char* const* sourceDataRows,
                            int pixelWidth,
                            unsigned char* outRow) {
        int width = pixelWidth & ~3;

        // Output four pixels per iteration (16 bytes).
        for (int outX = 0; outX < width; outX += 4) {

            // Accumulated result for each pixel. 32 bits per RGBA channel.
            int32x4_t accum0 = vdupq_n_s32(0);
            int32x4_t accum1 = vdupq_n_s32(0);
            int32x4_t accum2 = vdupq_n_s32(0);
            int32x4_t accum3 = vdupq_n_s32(0);

            // Convolve with one filter coefficient per iteration.
            for (int filterY = 0; filterY < filterLength; filterY++) {

                // Duplicate the filter coefficient 4 times.
                // [16] cj cj cj cj
                int16x4_t coeff16 = vdup_n_s16(filterValues[filterY]);

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

            int32x4_t accum0 = vdupq_n_s32(0);
            int32x4_t accum1 = vdupq_n_s32(0);
            int32x4_t accum2 = vdupq_n_s32(0);

            for (int filterY = 0; filterY < filterLength; ++filterY) {
                int16x4_t coeff16 = vdup_n_s16(filterValues[filterY]);

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

#else

    // Converts the argument to an 8-bit unsigned value by clamping to the range
    // 0-255.
    inline unsigned char ClampTo8(int a) {
        if (static_cast<unsigned>(a) < 256) {
            return a;  // Avoid the extra check in the common case.
        }
        if (a < 0) {
            return 0;
        }
        return 255;
    }

    // Convolves horizontally along a single row. The row data is given in
    // |srcData| and continues for the numValues() of the filter.
    template<bool hasAlpha>
    void ConvolveHorizontally(const unsigned char* srcData,
                              const SkConvolutionFilter1D& filter,
                              unsigned char* outRow) {
        // Loop over each pixel on this row in the output image.
        int numValues = filter.numValues();
        for (int outX = 0; outX < numValues; outX++) {
            // Get the filter that determines the current output pixel.
            int filterOffset, filterLength;
            const SkConvolutionFilter1D::ConvolutionFixed* filterValues =
                filter.FilterForValue(outX, &filterOffset, &filterLength);

            // Compute the first pixel in this row that the filter affects. It will
            // touch |filterLength| pixels (4 bytes each) after this.
            const unsigned char* rowToFilter = &srcData[filterOffset * 4];

            // Apply the filter to the row to get the destination pixel in |accum|.
            int accum[4] = {0};
            for (int filterX = 0; filterX < filterLength; filterX++) {
                SkConvolutionFilter1D::ConvolutionFixed curFilter = filterValues[filterX];
                accum[0] += curFilter * rowToFilter[filterX * 4 + 0];
                accum[1] += curFilter * rowToFilter[filterX * 4 + 1];
                accum[2] += curFilter * rowToFilter[filterX * 4 + 2];
                if (hasAlpha) {
                    accum[3] += curFilter * rowToFilter[filterX * 4 + 3];
                }
            }

            // Bring this value back in range. All of the filter scaling factors
            // are in fixed point with kShiftBits bits of fractional part.
            accum[0] >>= SkConvolutionFilter1D::kShiftBits;
            accum[1] >>= SkConvolutionFilter1D::kShiftBits;
            accum[2] >>= SkConvolutionFilter1D::kShiftBits;
            if (hasAlpha) {
                accum[3] >>= SkConvolutionFilter1D::kShiftBits;
            }

            // Store the new pixel.
            outRow[outX * 4 + 0] = ClampTo8(accum[0]);
            outRow[outX * 4 + 1] = ClampTo8(accum[1]);
            outRow[outX * 4 + 2] = ClampTo8(accum[2]);
            if (hasAlpha) {
                outRow[outX * 4 + 3] = ClampTo8(accum[3]);
            }
        }
    }

    // Does vertical convolution to produce one output row. The filter values and
    // length are given in the first two parameters. These are applied to each
    // of the rows pointed to in the |sourceDataRows| array, with each row
    // being |pixelWidth| wide.
    //
    // The output must have room for |pixelWidth * 4| bytes.
    template<bool hasAlpha>
    void ConvolveVertically(const SkConvolutionFilter1D::ConvolutionFixed* filterValues,
                            int filterLength,
                            unsigned char* const* sourceDataRows,
                            int pixelWidth,
                            unsigned char* outRow) {
        // We go through each column in the output and do a vertical convolution,
        // generating one output pixel each time.
        for (int outX = 0; outX < pixelWidth; outX++) {
            // Compute the number of bytes over in each row that the current column
            // we're convolving starts at. The pixel will cover the next 4 bytes.
            int byteOffset = outX * 4;

            // Apply the filter to one column of pixels.
            int accum[4] = {0};
            for (int filterY = 0; filterY < filterLength; filterY++) {
                SkConvolutionFilter1D::ConvolutionFixed curFilter = filterValues[filterY];
                accum[0] += curFilter * sourceDataRows[filterY][byteOffset + 0];
                accum[1] += curFilter * sourceDataRows[filterY][byteOffset + 1];
                accum[2] += curFilter * sourceDataRows[filterY][byteOffset + 2];
                if (hasAlpha) {
                    accum[3] += curFilter * sourceDataRows[filterY][byteOffset + 3];
                }
            }

            // Bring this value back in range. All of the filter scaling factors
            // are in fixed point with kShiftBits bits of precision.
            accum[0] >>= SkConvolutionFilter1D::kShiftBits;
            accum[1] >>= SkConvolutionFilter1D::kShiftBits;
            accum[2] >>= SkConvolutionFilter1D::kShiftBits;
            if (hasAlpha) {
                accum[3] >>= SkConvolutionFilter1D::kShiftBits;
            }

            // Store the new pixel.
            outRow[byteOffset + 0] = ClampTo8(accum[0]);
            outRow[byteOffset + 1] = ClampTo8(accum[1]);
            outRow[byteOffset + 2] = ClampTo8(accum[2]);
            if (hasAlpha) {
                unsigned char alpha = ClampTo8(accum[3]);

                // Make sure the alpha channel doesn't come out smaller than any of the
                // color channels. We use premultipled alpha channels, so this should
                // never happen, but rounding errors will cause this from time to time.
                // These "impossible" colors will cause overflows (and hence random pixel
                // values) when the resulting bitmap is drawn to the screen.
                //
                // We only need to do this when generating the final output row (here).
                int maxColorChannel = SkTMax(outRow[byteOffset + 0],
                                               SkTMax(outRow[byteOffset + 1],
                                                      outRow[byteOffset + 2]));
                if (alpha < maxColorChannel) {
                    outRow[byteOffset + 3] = maxColorChannel;
                } else {
                    outRow[byteOffset + 3] = alpha;
                }
            } else {
                // No alpha channel, the image is opaque.
                outRow[byteOffset + 3] = 0xff;
            }
        }
    }

    // There's a bug somewhere here with GCC autovectorization (-ftree-vectorize).  We originally
    // thought this was 32 bit only, but subsequent tests show that some 64 bit gcc compiles
    // suffer here too.
    //
    // Dropping to -O2 disables -ftree-vectorize.  GCC 4.6 needs noinline.  https://bug.skia.org/2575
#if SK_HAS_ATTRIBUTE(optimize) && defined(SK_RELEASE)
        #define SK_MAYBE_DISABLE_VECTORIZATION __attribute__((optimize("O2"), noinline))
#else
        #define SK_MAYBE_DISABLE_VECTORIZATION
#endif

    SK_MAYBE_DISABLE_VECTORIZATION
    void convolve_horizontally(const unsigned char* srcData,
                               const SkConvolutionFilter1D& filter,
                               unsigned char* outRow,
                               bool hasAlpha) {
        if (hasAlpha) {
            ConvolveHorizontally<true>(srcData, filter, outRow);
        } else {
            ConvolveHorizontally<false>(srcData, filter, outRow);
        }
    }
#undef SK_MAYBE_DISABLE_VECTORIZATION

    void (*convolve_4_rows_horizontally)(const unsigned char* srcData[4],
                                         const SkConvolutionFilter1D& filter,
                                         unsigned char* outRow[4],
                                         size_t outRowBytes)
        = nullptr;


#endif

    void convolve_vertically(const SkConvolutionFilter1D::ConvolutionFixed* filterValues,
                             int filterLength,
                             unsigned char* const* sourceDataRows,
                             int pixelWidth,
                             unsigned char* outRow,
                             bool hasAlpha) {
        if (hasAlpha) {
            ConvolveVertically<true>(filterValues, filterLength, sourceDataRows,
                                     pixelWidth, outRow);
        } else {
            ConvolveVertically<false>(filterValues, filterLength, sourceDataRows,
                                      pixelWidth, outRow);
        }
    }

}  // namespace SK_OPTS_NS

#endif//SkBitmapFilter_opts_DEFINED
