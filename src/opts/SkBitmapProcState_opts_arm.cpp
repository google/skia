/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBitmapProcState.h"
#include "SkColorPriv.h"
#include "SkPaint.h"
#include "SkTypes.h"
#include "SkUtils.h"
#include "SkUtilsArm.h"

#include "SkConvolver.h"

#if SK_ARM_ARCH >= 6 && !defined(SK_CPU_BENDIAN)
void SI8_D16_nofilter_DX_arm(
    const SkBitmapProcState& s,
    const uint32_t* SK_RESTRICT xy,
    int count,
    uint16_t* SK_RESTRICT colors) SK_ATTRIBUTE_OPTIMIZE_O1;

void SI8_D16_nofilter_DX_arm(const SkBitmapProcState& s,
                             const uint32_t* SK_RESTRICT xy,
                             int count, uint16_t* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask));
    SkASSERT(SkPaint::kNone_FilterLevel == s.fFilterLevel);

    const uint16_t* SK_RESTRICT table = s.fBitmap->getColorTable()->lock16BitCache();
    const uint8_t* SK_RESTRICT srcAddr = (const uint8_t*)s.fBitmap->getPixels();

    // buffer is y32, x16, x16, x16, x16, x16
    // bump srcAddr to the proper row, since we're told Y never changes
    SkASSERT((unsigned)xy[0] < (unsigned)s.fBitmap->height());
    srcAddr = (const uint8_t*)((const char*)srcAddr +
                               xy[0] * s.fBitmap->rowBytes());

    uint8_t src;

    if (1 == s.fBitmap->width()) {
        src = srcAddr[0];
        uint16_t dstValue = table[src];
        sk_memset16(colors, dstValue, count);
    } else {
        int i;
        int count8 = count >> 3;
        const uint16_t* SK_RESTRICT xx = (const uint16_t*)(xy + 1);

        asm volatile (
                      "cmp        %[count8], #0                   \n\t"   // compare loop counter with 0
                      "beq        2f                              \n\t"   // if loop counter == 0, exit
                      "1:                                             \n\t"
                      "ldmia      %[xx]!, {r5, r7, r9, r11}       \n\t"   // load ptrs to pixels 0-7
                      "subs       %[count8], %[count8], #1        \n\t"   // decrement loop counter
                      "uxth       r4, r5                          \n\t"   // extract ptr 0
                      "mov        r5, r5, lsr #16                 \n\t"   // extract ptr 1
                      "uxth       r6, r7                          \n\t"   // extract ptr 2
                      "mov        r7, r7, lsr #16                 \n\t"   // extract ptr 3
                      "ldrb       r4, [%[srcAddr], r4]            \n\t"   // load pixel 0 from image
                      "uxth       r8, r9                          \n\t"   // extract ptr 4
                      "ldrb       r5, [%[srcAddr], r5]            \n\t"   // load pixel 1 from image
                      "mov        r9, r9, lsr #16                 \n\t"   // extract ptr 5
                      "ldrb       r6, [%[srcAddr], r6]            \n\t"   // load pixel 2 from image
                      "uxth       r10, r11                        \n\t"   // extract ptr 6
                      "ldrb       r7, [%[srcAddr], r7]            \n\t"   // load pixel 3 from image
                      "mov        r11, r11, lsr #16               \n\t"   // extract ptr 7
                      "ldrb       r8, [%[srcAddr], r8]            \n\t"   // load pixel 4 from image
                      "add        r4, r4, r4                      \n\t"   // double pixel 0 for RGB565 lookup
                      "ldrb       r9, [%[srcAddr], r9]            \n\t"   // load pixel 5 from image
                      "add        r5, r5, r5                      \n\t"   // double pixel 1 for RGB565 lookup
                      "ldrb       r10, [%[srcAddr], r10]          \n\t"   // load pixel 6 from image
                      "add        r6, r6, r6                      \n\t"   // double pixel 2 for RGB565 lookup
                      "ldrb       r11, [%[srcAddr], r11]          \n\t"   // load pixel 7 from image
                      "add        r7, r7, r7                      \n\t"   // double pixel 3 for RGB565 lookup
                      "ldrh       r4, [%[table], r4]              \n\t"   // load pixel 0 RGB565 from colmap
                      "add        r8, r8, r8                      \n\t"   // double pixel 4 for RGB565 lookup
                      "ldrh       r5, [%[table], r5]              \n\t"   // load pixel 1 RGB565 from colmap
                      "add        r9, r9, r9                      \n\t"   // double pixel 5 for RGB565 lookup
                      "ldrh       r6, [%[table], r6]              \n\t"   // load pixel 2 RGB565 from colmap
                      "add        r10, r10, r10                   \n\t"   // double pixel 6 for RGB565 lookup
                      "ldrh       r7, [%[table], r7]              \n\t"   // load pixel 3 RGB565 from colmap
                      "add        r11, r11, r11                   \n\t"   // double pixel 7 for RGB565 lookup
                      "ldrh       r8, [%[table], r8]              \n\t"   // load pixel 4 RGB565 from colmap
                      "ldrh       r9, [%[table], r9]              \n\t"   // load pixel 5 RGB565 from colmap
                      "ldrh       r10, [%[table], r10]            \n\t"   // load pixel 6 RGB565 from colmap
                      "ldrh       r11, [%[table], r11]            \n\t"   // load pixel 7 RGB565 from colmap
                      "pkhbt      r5, r4, r5, lsl #16             \n\t"   // pack pixels 0 and 1
                      "pkhbt      r6, r6, r7, lsl #16             \n\t"   // pack pixels 2 and 3
                      "pkhbt      r8, r8, r9, lsl #16             \n\t"   // pack pixels 4 and 5
                      "pkhbt      r10, r10, r11, lsl #16          \n\t"   // pack pixels 6 and 7
                      "stmia      %[colors]!, {r5, r6, r8, r10}   \n\t"   // store last 8 pixels
                      "bgt        1b                              \n\t"   // loop if counter > 0
                      "2:                                             \n\t"
                      : [xx] "+r" (xx), [count8] "+r" (count8), [colors] "+r" (colors)
                      : [table] "r" (table), [srcAddr] "r" (srcAddr)
                      : "memory", "cc", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11"
                      );

        for (i = (count & 7); i > 0; --i) {
            src = srcAddr[*xx++]; *colors++ = table[src];
        }
    }

    s.fBitmap->getColorTable()->unlock16BitCache();
}

void SI8_opaque_D32_nofilter_DX_arm(
    const SkBitmapProcState& s,
    const uint32_t* SK_RESTRICT xy,
    int count,
    SkPMColor* SK_RESTRICT colors) SK_ATTRIBUTE_OPTIMIZE_O1;

void SI8_opaque_D32_nofilter_DX_arm(const SkBitmapProcState& s,
                                    const uint32_t* SK_RESTRICT xy,
                                    int count, SkPMColor* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask));
    SkASSERT(SkPaint::kNone_FilterLevel == s.fFilterLevel);

    const SkPMColor* SK_RESTRICT table = s.fBitmap->getColorTable()->lockColors();
    const uint8_t* SK_RESTRICT srcAddr = (const uint8_t*)s.fBitmap->getPixels();

    // buffer is y32, x16, x16, x16, x16, x16
    // bump srcAddr to the proper row, since we're told Y never changes
    SkASSERT((unsigned)xy[0] < (unsigned)s.fBitmap->height());
    srcAddr = (const uint8_t*)((const char*)srcAddr + xy[0] * s.fBitmap->rowBytes());

    if (1 == s.fBitmap->width()) {
        uint8_t src = srcAddr[0];
        SkPMColor dstValue = table[src];
        sk_memset32(colors, dstValue, count);
    } else {
        const uint16_t* xx = (const uint16_t*)(xy + 1);

        asm volatile (
                      "subs       %[count], %[count], #8          \n\t"   // decrement count by 8, set flags
                      "blt        2f                              \n\t"   // if count < 0, branch to singles
                      "1:                                             \n\t"   // eights loop
                      "ldmia      %[xx]!, {r5, r7, r9, r11}       \n\t"   // load ptrs to pixels 0-7
                      "uxth       r4, r5                          \n\t"   // extract ptr 0
                      "mov        r5, r5, lsr #16                 \n\t"   // extract ptr 1
                      "uxth       r6, r7                          \n\t"   // extract ptr 2
                      "mov        r7, r7, lsr #16                 \n\t"   // extract ptr 3
                      "ldrb       r4, [%[srcAddr], r4]            \n\t"   // load pixel 0 from image
                      "uxth       r8, r9                          \n\t"   // extract ptr 4
                      "ldrb       r5, [%[srcAddr], r5]            \n\t"   // load pixel 1 from image
                      "mov        r9, r9, lsr #16                 \n\t"   // extract ptr 5
                      "ldrb       r6, [%[srcAddr], r6]            \n\t"   // load pixel 2 from image
                      "uxth       r10, r11                        \n\t"   // extract ptr 6
                      "ldrb       r7, [%[srcAddr], r7]            \n\t"   // load pixel 3 from image
                      "mov        r11, r11, lsr #16               \n\t"   // extract ptr 7
                      "ldrb       r8, [%[srcAddr], r8]            \n\t"   // load pixel 4 from image
                      "ldrb       r9, [%[srcAddr], r9]            \n\t"   // load pixel 5 from image
                      "ldrb       r10, [%[srcAddr], r10]          \n\t"   // load pixel 6 from image
                      "ldrb       r11, [%[srcAddr], r11]          \n\t"   // load pixel 7 from image
                      "ldr        r4, [%[table], r4, lsl #2]      \n\t"   // load pixel 0 SkPMColor from colmap
                      "ldr        r5, [%[table], r5, lsl #2]      \n\t"   // load pixel 1 SkPMColor from colmap
                      "ldr        r6, [%[table], r6, lsl #2]      \n\t"   // load pixel 2 SkPMColor from colmap
                      "ldr        r7, [%[table], r7, lsl #2]      \n\t"   // load pixel 3 SkPMColor from colmap
                      "ldr        r8, [%[table], r8, lsl #2]      \n\t"   // load pixel 4 SkPMColor from colmap
                      "ldr        r9, [%[table], r9, lsl #2]      \n\t"   // load pixel 5 SkPMColor from colmap
                      "ldr        r10, [%[table], r10, lsl #2]    \n\t"   // load pixel 6 SkPMColor from colmap
                      "ldr        r11, [%[table], r11, lsl #2]    \n\t"   // load pixel 7 SkPMColor from colmap
                      "subs       %[count], %[count], #8          \n\t"   // decrement loop counter
                      "stmia      %[colors]!, {r4-r11}            \n\t"   // store 8 pixels
                      "bge        1b                              \n\t"   // loop if counter >= 0
                      "2:                                             \n\t"
                      "adds       %[count], %[count], #8          \n\t"   // fix up counter, set flags
                      "beq        4f                              \n\t"   // if count == 0, branch to exit
                      "3:                                             \n\t"   // singles loop
                      "ldrh       r4, [%[xx]], #2                 \n\t"   // load pixel ptr
                      "subs       %[count], %[count], #1          \n\t"   // decrement loop counter
                      "ldrb       r5, [%[srcAddr], r4]            \n\t"   // load pixel from image
                      "ldr        r6, [%[table], r5, lsl #2]      \n\t"   // load SkPMColor from colmap
                      "str        r6, [%[colors]], #4             \n\t"   // store pixel, update ptr
                      "bne        3b                              \n\t"   // loop if counter != 0
                      "4:                                             \n\t"   // exit
                      : [xx] "+r" (xx), [count] "+r" (count), [colors] "+r" (colors)
                      : [table] "r" (table), [srcAddr] "r" (srcAddr)
                      : "memory", "cc", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11"
                      );
    }

    s.fBitmap->getColorTable()->unlockColors();
}
#endif // SK_ARM_ARCH >= 6 && !defined(SK_CPU_BENDIAN)

///////////////////////////////////////////////////////////////////////////////

/*  If we replace a sampleproc, then we null-out the associated shaderproc,
    otherwise the shader won't even look at the matrix/sampler
 */
void SkBitmapProcState::platformProcs() {
    bool isOpaque = 256 == fAlphaScale;
    bool justDx = false;

    if (fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask)) {
        justDx = true;
    }

    switch (fBitmap->config()) {
        case SkBitmap::kIndex8_Config:
#if SK_ARM_ARCH >= 6 && !defined(SK_CPU_BENDIAN)
            if (justDx && SkPaint::kNone_FilterLevel == fFilterLevel) {
#if 0   /* crashing on android device */
                fSampleProc16 = SI8_D16_nofilter_DX_arm;
                fShaderProc16 = NULL;
#endif
                if (isOpaque) {
                    // this one is only very slighty faster than the C version
                    fSampleProc32 = SI8_opaque_D32_nofilter_DX_arm;
                    fShaderProc32 = NULL;
                }
            }
#endif
            break;
        default:
            break;
    }
}

/////////////////////////////////////

/* FUNCTIONS BELOW ARE SCALAR STUBS INTENDED FOR ARM DEVELOPERS TO REPLACE */

/////////////////////////////////////


static inline unsigned char ClampTo8(int a) {
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
void convolveHorizontally_arm(const unsigned char* srcData,
                              const SkConvolutionFilter1D& filter,
                              unsigned char* outRow,
                              bool hasAlpha) {
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
    void convolveVertically_arm(const SkConvolutionFilter1D::ConvolutionFixed* filterValues,
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

void convolveVertically_arm(const SkConvolutionFilter1D::ConvolutionFixed* filterValues,
                            int filterLength,
                            unsigned char* const* sourceDataRows,
                            int pixelWidth,
                            unsigned char* outRow,
                            bool sourceHasAlpha) {
    if (sourceHasAlpha) {
        convolveVertically_arm<true>(filterValues, filterLength,
                                     sourceDataRows, pixelWidth,
                                     outRow);
    } else {
        convolveVertically_arm<false>(filterValues, filterLength,
                                      sourceDataRows, pixelWidth,
                                      outRow);
    }
}

// Convolves horizontally along four rows. The row data is given in
// |src_data| and continues for the num_values() of the filter.
// The algorithm is almost same as |ConvolveHorizontally_SSE2|. Please
// refer to that function for detailed comments.
void convolve4RowsHorizontally_arm(const unsigned char* src_data[4],
                                   const SkConvolutionFilter1D& filter,
                                   unsigned char* out_row[4]) {
}

///////////////////////////

/* STOP REWRITING FUNCTIONS HERE, BUT DON'T FORGET TO EDIT THE
   PLATFORM CONVOLUTION PROCS BELOW */

///////////////////////////

void applySIMDPadding_arm(SkConvolutionFilter1D *filter) {
    // Padding |paddingCount| of more dummy coefficients after the coefficients
    // of last filter to prevent SIMD instructions which load 8 or 16 bytes
    // together to access invalid memory areas. We are not trying to align the
    // coefficients right now due to the opaqueness of <vector> implementation.
    // This has to be done after all |AddFilter| calls.
    for (int i = 0; i < 8; ++i) {
        filter->addFilterValue(static_cast<SkConvolutionFilter1D::ConvolutionFixed>(0));
    }
}

void SkBitmapProcState::platformConvolutionProcs(SkConvolutionProcs* procs) {
    if (sk_cpu_arm_has_neon()) {
        procs->fExtraHorizontalReads = 3;
        procs->fConvolveVertically = &convolveVertically_arm;

        // next line is commented out because the four-row convolution function above is
        // just a no-op.  Please see the comment above its definition, and the SSE implementation
        // in SkBitmapProcState_opts_SSE2.cpp for guidance on its semantics.
        // leaving it as NULL will just cause the convolution system to not attempt
        // to operate on four rows at once, which is correct but not performance-optimal.

        // procs->fConvolve4RowsHorizontally = &convolve4RowsHorizontally_arm;

        procs->fConvolve4RowsHorizontally = NULL;

        procs->fConvolveHorizontally = &convolveHorizontally_arm;
        procs->fApplySIMDPadding = &applySIMDPadding_arm;
    }
}
