/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitRow_opts_arm.h"

#include "SkBlitMask.h"
#include "SkBlitRow.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkMathPriv.h"
#include "SkUtils.h"

#include "SkCachePreload_arm.h"

#include <arm_neon.h>

void S32A_D565_Opaque_neon(uint16_t* SK_RESTRICT dst,
                           const SkPMColor* SK_RESTRICT src, int count,
                           U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 == alpha);

    if (count >= 8) {
        uint16_t* SK_RESTRICT keep_dst;

        asm volatile (
                      "ands       ip, %[count], #7            \n\t"
                      "vmov.u8    d31, #1<<7                  \n\t"
                      "vld1.16    {q12}, [%[dst]]             \n\t"
                      "vld4.8     {d0-d3}, [%[src]]           \n\t"
                      // Thumb does not support the standard ARM conditional
                      // instructions but instead requires the 'it' instruction
                      // to signal conditional execution
                      "it eq                                  \n\t"
                      "moveq      ip, #8                      \n\t"
                      "mov        %[keep_dst], %[dst]         \n\t"

                      "add        %[src], %[src], ip, LSL#2   \n\t"
                      "add        %[dst], %[dst], ip, LSL#1   \n\t"
                      "subs       %[count], %[count], ip      \n\t"
                      "b          9f                          \n\t"
                      // LOOP
                      "2:                                         \n\t"

                      "vld1.16    {q12}, [%[dst]]!            \n\t"
                      "vld4.8     {d0-d3}, [%[src]]!          \n\t"
                      "vst1.16    {q10}, [%[keep_dst]]        \n\t"
                      "sub        %[keep_dst], %[dst], #8*2   \n\t"
                      "subs       %[count], %[count], #8      \n\t"
                      "9:                                         \n\t"
                      "pld        [%[dst],#32]                \n\t"
                      // expand 0565 q12 to 8888 {d4-d7}
                      "vmovn.u16  d4, q12                     \n\t"
                      "vshr.u16   q11, q12, #5                \n\t"
                      "vshr.u16   q10, q12, #6+5              \n\t"
                      "vmovn.u16  d5, q11                     \n\t"
                      "vmovn.u16  d6, q10                     \n\t"
                      "vshl.u8    d4, d4, #3                  \n\t"
                      "vshl.u8    d5, d5, #2                  \n\t"
                      "vshl.u8    d6, d6, #3                  \n\t"

                      "vmovl.u8   q14, d31                    \n\t"
                      "vmovl.u8   q13, d31                    \n\t"
                      "vmovl.u8   q12, d31                    \n\t"

                      // duplicate in 4/2/1 & 8pix vsns
                      "vmvn.8     d30, d3                     \n\t"
                      "vmlal.u8   q14, d30, d6                \n\t"
                      "vmlal.u8   q13, d30, d5                \n\t"
                      "vmlal.u8   q12, d30, d4                \n\t"
                      "vshr.u16   q8, q14, #5                 \n\t"
                      "vshr.u16   q9, q13, #6                 \n\t"
                      "vaddhn.u16 d6, q14, q8                 \n\t"
                      "vshr.u16   q8, q12, #5                 \n\t"
                      "vaddhn.u16 d5, q13, q9                 \n\t"
                      "vqadd.u8   d6, d6, d0                  \n\t"  // moved up
                      "vaddhn.u16 d4, q12, q8                 \n\t"
                      // intentionally don't calculate alpha
                      // result in d4-d6

                      "vqadd.u8   d5, d5, d1                  \n\t"
                      "vqadd.u8   d4, d4, d2                  \n\t"

                      // pack 8888 {d4-d6} to 0565 q10
                      "vshll.u8   q10, d6, #8                 \n\t"
                      "vshll.u8   q3, d5, #8                  \n\t"
                      "vshll.u8   q2, d4, #8                  \n\t"
                      "vsri.u16   q10, q3, #5                 \n\t"
                      "vsri.u16   q10, q2, #11                \n\t"

                      "bne        2b                          \n\t"

                      "1:                                         \n\t"
                      "vst1.16      {q10}, [%[keep_dst]]      \n\t"
                      : [count] "+r" (count)
                      : [dst] "r" (dst), [keep_dst] "r" (keep_dst), [src] "r" (src)
                      : "ip", "cc", "memory", "d0","d1","d2","d3","d4","d5","d6","d7",
                      "d16","d17","d18","d19","d20","d21","d22","d23","d24","d25","d26","d27","d28","d29",
                      "d30","d31"
                      );
    }
    else
    {   // handle count < 8
        uint16_t* SK_RESTRICT keep_dst;

        asm volatile (
                      "vmov.u8    d31, #1<<7                  \n\t"
                      "mov        %[keep_dst], %[dst]         \n\t"

                      "tst        %[count], #4                \n\t"
                      "beq        14f                         \n\t"
                      "vld1.16    {d25}, [%[dst]]!            \n\t"
                      "vld1.32    {q1}, [%[src]]!             \n\t"

                      "14:                                        \n\t"
                      "tst        %[count], #2                \n\t"
                      "beq        12f                         \n\t"
                      "vld1.32    {d24[1]}, [%[dst]]!         \n\t"
                      "vld1.32    {d1}, [%[src]]!             \n\t"

                      "12:                                        \n\t"
                      "tst        %[count], #1                \n\t"
                      "beq        11f                         \n\t"
                      "vld1.16    {d24[1]}, [%[dst]]!         \n\t"
                      "vld1.32    {d0[1]}, [%[src]]!          \n\t"

                      "11:                                        \n\t"
                      // unzips achieve the same as a vld4 operation
                      "vuzpq.u16  q0, q1                      \n\t"
                      "vuzp.u8    d0, d1                      \n\t"
                      "vuzp.u8    d2, d3                      \n\t"
                      // expand 0565 q12 to 8888 {d4-d7}
                      "vmovn.u16  d4, q12                     \n\t"
                      "vshr.u16   q11, q12, #5                \n\t"
                      "vshr.u16   q10, q12, #6+5              \n\t"
                      "vmovn.u16  d5, q11                     \n\t"
                      "vmovn.u16  d6, q10                     \n\t"
                      "vshl.u8    d4, d4, #3                  \n\t"
                      "vshl.u8    d5, d5, #2                  \n\t"
                      "vshl.u8    d6, d6, #3                  \n\t"

                      "vmovl.u8   q14, d31                    \n\t"
                      "vmovl.u8   q13, d31                    \n\t"
                      "vmovl.u8   q12, d31                    \n\t"

                      // duplicate in 4/2/1 & 8pix vsns
                      "vmvn.8     d30, d3                     \n\t"
                      "vmlal.u8   q14, d30, d6                \n\t"
                      "vmlal.u8   q13, d30, d5                \n\t"
                      "vmlal.u8   q12, d30, d4                \n\t"
                      "vshr.u16   q8, q14, #5                 \n\t"
                      "vshr.u16   q9, q13, #6                 \n\t"
                      "vaddhn.u16 d6, q14, q8                 \n\t"
                      "vshr.u16   q8, q12, #5                 \n\t"
                      "vaddhn.u16 d5, q13, q9                 \n\t"
                      "vqadd.u8   d6, d6, d0                  \n\t"  // moved up
                      "vaddhn.u16 d4, q12, q8                 \n\t"
                      // intentionally don't calculate alpha
                      // result in d4-d6

                      "vqadd.u8   d5, d5, d1                  \n\t"
                      "vqadd.u8   d4, d4, d2                  \n\t"

                      // pack 8888 {d4-d6} to 0565 q10
                      "vshll.u8   q10, d6, #8                 \n\t"
                      "vshll.u8   q3, d5, #8                  \n\t"
                      "vshll.u8   q2, d4, #8                  \n\t"
                      "vsri.u16   q10, q3, #5                 \n\t"
                      "vsri.u16   q10, q2, #11                \n\t"

                      // store
                      "tst        %[count], #4                \n\t"
                      "beq        24f                         \n\t"
                      "vst1.16    {d21}, [%[keep_dst]]!       \n\t"

                      "24:                                        \n\t"
                      "tst        %[count], #2                \n\t"
                      "beq        22f                         \n\t"
                      "vst1.32    {d20[1]}, [%[keep_dst]]!    \n\t"

                      "22:                                        \n\t"
                      "tst        %[count], #1                \n\t"
                      "beq        21f                         \n\t"
                      "vst1.16    {d20[1]}, [%[keep_dst]]!    \n\t"

                      "21:                                        \n\t"
                      : [count] "+r" (count)
                      : [dst] "r" (dst), [keep_dst] "r" (keep_dst), [src] "r" (src)
                      : "ip", "cc", "memory", "d0","d1","d2","d3","d4","d5","d6","d7",
                      "d16","d17","d18","d19","d20","d21","d22","d23","d24","d25","d26","d27","d28","d29",
                      "d30","d31"
                      );
    }
}

void S32A_D565_Blend_neon(uint16_t* SK_RESTRICT dst,
                          const SkPMColor* SK_RESTRICT src, int count,
                          U8CPU alpha, int /*x*/, int /*y*/) {

    U8CPU alpha_for_asm = alpha;

    asm volatile (
    /* This code implements a Neon version of S32A_D565_Blend. The output differs from
     * the original in two respects:
     *  1. The results have a few mismatches compared to the original code. These mismatches
     *     never exceed 1. It's possible to improve accuracy vs. a floating point
     *     implementation by introducing rounding right shifts (vrshr) for the final stage.
     *     Rounding is not present in the code below, because although results would be closer
     *     to a floating point implementation, the number of mismatches compared to the
     *     original code would be far greater.
     *  2. On certain inputs, the original code can overflow, causing colour channels to
     *     mix. Although the Neon code can also overflow, it doesn't allow one colour channel
     *     to affect another.
     */

#if 1
        /* reflects SkAlpha255To256()'s change from a+a>>7 to a+1 */
                  "add        %[alpha], %[alpha], #1         \n\t"   // adjust range of alpha 0-256
#else
                  "add        %[alpha], %[alpha], %[alpha], lsr #7    \n\t"   // adjust range of alpha 0-256
#endif
                  "vmov.u16   q3, #255                        \n\t"   // set up constant
                  "movs       r4, %[count], lsr #3            \n\t"   // calc. count>>3
                  "vmov.u16   d2[0], %[alpha]                 \n\t"   // move alpha to Neon
                  "beq        2f                              \n\t"   // if count8 == 0, exit
                  "vmov.u16   q15, #0x1f                      \n\t"   // set up blue mask

                  "1:                                             \n\t"
                  "vld1.u16   {d0, d1}, [%[dst]]              \n\t"   // load eight dst RGB565 pixels
                  "subs       r4, r4, #1                      \n\t"   // decrement loop counter
                  "vld4.u8    {d24, d25, d26, d27}, [%[src]]! \n\t"   // load eight src ABGR32 pixels
                  //  and deinterleave

                  "vshl.u16   q9, q0, #5                      \n\t"   // shift green to top of lanes
                  "vand       q10, q0, q15                    \n\t"   // extract blue
                  "vshr.u16   q8, q0, #11                     \n\t"   // extract red
                  "vshr.u16   q9, q9, #10                     \n\t"   // extract green
                  // dstrgb = {q8, q9, q10}

                  "vshr.u8    d24, d24, #3                    \n\t"   // shift red to 565 range
                  "vshr.u8    d25, d25, #2                    \n\t"   // shift green to 565 range
                  "vshr.u8    d26, d26, #3                    \n\t"   // shift blue to 565 range

                  "vmovl.u8   q11, d24                        \n\t"   // widen red to 16 bits
                  "vmovl.u8   q12, d25                        \n\t"   // widen green to 16 bits
                  "vmovl.u8   q14, d27                        \n\t"   // widen alpha to 16 bits
                  "vmovl.u8   q13, d26                        \n\t"   // widen blue to 16 bits
                  // srcrgba = {q11, q12, q13, q14}

                  "vmul.u16   q2, q14, d2[0]                  \n\t"   // sa * src_scale
                  "vmul.u16   q11, q11, d2[0]                 \n\t"   // red result = src_red * src_scale
                  "vmul.u16   q12, q12, d2[0]                 \n\t"   // grn result = src_grn * src_scale
                  "vmul.u16   q13, q13, d2[0]                 \n\t"   // blu result = src_blu * src_scale

                  "vshr.u16   q2, q2, #8                      \n\t"   // sa * src_scale >> 8
                  "vsub.u16   q2, q3, q2                      \n\t"   // 255 - (sa * src_scale >> 8)
                  // dst_scale = q2

                  "vmla.u16   q11, q8, q2                     \n\t"   // red result += dst_red * dst_scale
                  "vmla.u16   q12, q9, q2                     \n\t"   // grn result += dst_grn * dst_scale
                  "vmla.u16   q13, q10, q2                    \n\t"   // blu result += dst_blu * dst_scale

#if 1
    // trying for a better match with SkDiv255Round(a)
    // C alg is:  a+=128; (a+a>>8)>>8
    // we'll use just a rounding shift [q2 is available for scratch]
                  "vrshr.u16   q11, q11, #8                    \n\t"   // shift down red
                  "vrshr.u16   q12, q12, #8                    \n\t"   // shift down green
                  "vrshr.u16   q13, q13, #8                    \n\t"   // shift down blue
#else
    // arm's original "truncating divide by 256"
                  "vshr.u16   q11, q11, #8                    \n\t"   // shift down red
                  "vshr.u16   q12, q12, #8                    \n\t"   // shift down green
                  "vshr.u16   q13, q13, #8                    \n\t"   // shift down blue
#endif

                  "vsli.u16   q13, q12, #5                    \n\t"   // insert green into blue
                  "vsli.u16   q13, q11, #11                   \n\t"   // insert red into green/blue
                  "vst1.16    {d26, d27}, [%[dst]]!           \n\t"   // write pixel back to dst, update ptr

                  "bne        1b                              \n\t"   // if counter != 0, loop
                  "2:                                             \n\t"   // exit

                  : [src] "+r" (src), [dst] "+r" (dst), [count] "+r" (count), [alpha] "+r" (alpha_for_asm)
                  :
                  : "cc", "memory", "r4", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31"
                  );

    count &= 7;
    if (count > 0) {
        do {
            SkPMColor sc = *src++;
            if (sc) {
                uint16_t dc = *dst;
                unsigned dst_scale = 255 - SkMulDiv255Round(SkGetPackedA32(sc), alpha);
                unsigned dr = SkMulS16(SkPacked32ToR16(sc), alpha) + SkMulS16(SkGetPackedR16(dc), dst_scale);
                unsigned dg = SkMulS16(SkPacked32ToG16(sc), alpha) + SkMulS16(SkGetPackedG16(dc), dst_scale);
                unsigned db = SkMulS16(SkPacked32ToB16(sc), alpha) + SkMulS16(SkGetPackedB16(dc), dst_scale);
                *dst = SkPackRGB16(SkDiv255Round(dr), SkDiv255Round(dg), SkDiv255Round(db));
            }
            dst += 1;
        } while (--count != 0);
    }
}

/* dither matrix for Neon, derived from gDitherMatrix_3Bit_16.
 * each dither value is spaced out into byte lanes, and repeated
 * to allow an 8-byte load from offsets 0, 1, 2 or 3 from the
 * start of each row.
 */
static const uint8_t gDitherMatrix_Neon[48] = {
    0, 4, 1, 5, 0, 4, 1, 5, 0, 4, 1, 5,
    6, 2, 7, 3, 6, 2, 7, 3, 6, 2, 7, 3,
    1, 5, 0, 4, 1, 5, 0, 4, 1, 5, 0, 4,
    7, 3, 6, 2, 7, 3, 6, 2, 7, 3, 6, 2,

};

void S32_D565_Blend_Dither_neon(uint16_t *dst, const SkPMColor *src,
                                int count, U8CPU alpha, int x, int y)
{
    /* select row and offset for dither array */
    const uint8_t *dstart = &gDitherMatrix_Neon[(y&3)*12 + (x&3)];

    /* rescale alpha to range 0 - 256 */
    int scale = SkAlpha255To256(alpha);

    asm volatile (
                  "vld1.8         {d31}, [%[dstart]]              \n\t"   // load dither values
                  "vshr.u8        d30, d31, #1                    \n\t"   // calc. green dither values
                  "vdup.16        d6, %[scale]                    \n\t"   // duplicate scale into neon reg
                  "vmov.i8        d29, #0x3f                      \n\t"   // set up green mask
                  "vmov.i8        d28, #0x1f                      \n\t"   // set up blue mask
                  "1:                                                 \n\t"
                  "vld4.8         {d0, d1, d2, d3}, [%[src]]!     \n\t"   // load 8 pixels and split into argb
                  "vshr.u8        d22, d0, #5                     \n\t"   // calc. red >> 5
                  "vshr.u8        d23, d1, #6                     \n\t"   // calc. green >> 6
                  "vshr.u8        d24, d2, #5                     \n\t"   // calc. blue >> 5
                  "vaddl.u8       q8, d0, d31                     \n\t"   // add in dither to red and widen
                  "vaddl.u8       q9, d1, d30                     \n\t"   // add in dither to green and widen
                  "vaddl.u8       q10, d2, d31                    \n\t"   // add in dither to blue and widen
                  "vsubw.u8       q8, q8, d22                     \n\t"   // sub shifted red from result
                  "vsubw.u8       q9, q9, d23                     \n\t"   // sub shifted green from result
                  "vsubw.u8       q10, q10, d24                   \n\t"   // sub shifted blue from result
                  "vshrn.i16      d22, q8, #3                     \n\t"   // shift right and narrow to 5 bits
                  "vshrn.i16      d23, q9, #2                     \n\t"   // shift right and narrow to 6 bits
                  "vshrn.i16      d24, q10, #3                    \n\t"   // shift right and narrow to 5 bits
                  // load 8 pixels from dst, extract rgb
                  "vld1.16        {d0, d1}, [%[dst]]              \n\t"   // load 8 pixels
                  "vshrn.i16      d17, q0, #5                     \n\t"   // shift green down to bottom 6 bits
                  "vmovn.i16      d18, q0                         \n\t"   // narrow to get blue as bytes
                  "vshr.u16       q0, q0, #11                     \n\t"   // shift down to extract red
                  "vand           d17, d17, d29                   \n\t"   // and green with green mask
                  "vand           d18, d18, d28                   \n\t"   // and blue with blue mask
                  "vmovn.i16      d16, q0                         \n\t"   // narrow to get red as bytes
                  // src = {d22 (r), d23 (g), d24 (b)}
                  // dst = {d16 (r), d17 (g), d18 (b)}
                  // subtract dst from src and widen
                  "vsubl.s8       q0, d22, d16                    \n\t"   // subtract red src from dst
                  "vsubl.s8       q1, d23, d17                    \n\t"   // subtract green src from dst
                  "vsubl.s8       q2, d24, d18                    \n\t"   // subtract blue src from dst
                  // multiply diffs by scale and shift
                  "vmul.i16       q0, q0, d6[0]                   \n\t"   // multiply red by scale
                  "vmul.i16       q1, q1, d6[0]                   \n\t"   // multiply blue by scale
                  "vmul.i16       q2, q2, d6[0]                   \n\t"   // multiply green by scale
                  "subs           %[count], %[count], #8          \n\t"   // decrement loop counter
                  "vshrn.i16      d0, q0, #8                      \n\t"   // shift down red by 8 and narrow
                  "vshrn.i16      d2, q1, #8                      \n\t"   // shift down green by 8 and narrow
                  "vshrn.i16      d4, q2, #8                      \n\t"   // shift down blue by 8 and narrow
                  // add dst to result
                  "vaddl.s8       q0, d0, d16                     \n\t"   // add dst to red
                  "vaddl.s8       q1, d2, d17                     \n\t"   // add dst to green
                  "vaddl.s8       q2, d4, d18                     \n\t"   // add dst to blue
                  // put result into 565 format
                  "vsli.i16       q2, q1, #5                      \n\t"   // shift up green and insert into blue
                  "vsli.i16       q2, q0, #11                     \n\t"   // shift up red and insert into blue
                  "vst1.16        {d4, d5}, [%[dst]]!             \n\t"   // store result
                  "bgt            1b                              \n\t"   // loop if count > 0
                  : [src] "+r" (src), [dst] "+r" (dst), [count] "+r" (count)
                  : [dstart] "r" (dstart), [scale] "r" (scale)
                  : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
                  );

    DITHER_565_SCAN(y);

    while((count & 7) > 0)
    {
        SkPMColor c = *src++;

        int dither = DITHER_VALUE(x);
        int sr = SkGetPackedR32(c);
        int sg = SkGetPackedG32(c);
        int sb = SkGetPackedB32(c);
        sr = SkDITHER_R32To565(sr, dither);
        sg = SkDITHER_G32To565(sg, dither);
        sb = SkDITHER_B32To565(sb, dither);

        uint16_t d = *dst;
        *dst++ = SkPackRGB16(SkAlphaBlend(sr, SkGetPackedR16(d), scale),
                             SkAlphaBlend(sg, SkGetPackedG16(d), scale),
                             SkAlphaBlend(sb, SkGetPackedB16(d), scale));
        DITHER_INC_X(x);
        count--;
    }
}

void S32A_Opaque_BlitRow32_neon(SkPMColor* SK_RESTRICT dst,
                                const SkPMColor* SK_RESTRICT src,
                                int count, U8CPU alpha) {

    SkASSERT(255 == alpha);
    if (count > 0) {


    uint8x8_t alpha_mask;

    static const uint8_t alpha_mask_setup[] = {3,3,3,3,7,7,7,7};
    alpha_mask = vld1_u8(alpha_mask_setup);

    /* do the NEON unrolled code */
#define    UNROLL    4
    while (count >= UNROLL) {
        uint8x8_t src_raw, dst_raw, dst_final;
        uint8x8_t src_raw_2, dst_raw_2, dst_final_2;

        /* get the source */
        src_raw = vreinterpret_u8_u32(vld1_u32(src));
#if    UNROLL > 2
        src_raw_2 = vreinterpret_u8_u32(vld1_u32(src+2));
#endif

        /* get and hold the dst too */
        dst_raw = vreinterpret_u8_u32(vld1_u32(dst));
#if    UNROLL > 2
        dst_raw_2 = vreinterpret_u8_u32(vld1_u32(dst+2));
#endif

    /* 1st and 2nd bits of the unrolling */
    {
        uint8x8_t dst_cooked;
        uint16x8_t dst_wide;
        uint8x8_t alpha_narrow;
        uint16x8_t alpha_wide;

        /* get the alphas spread out properly */
        alpha_narrow = vtbl1_u8(src_raw, alpha_mask);
#if 1
        /* reflect SkAlpha255To256() semantics a+1 vs a+a>>7 */
        /* we collapsed (255-a)+1 ... */
        alpha_wide = vsubw_u8(vdupq_n_u16(256), alpha_narrow);
#else
        alpha_wide = vsubw_u8(vdupq_n_u16(255), alpha_narrow);
        alpha_wide = vaddq_u16(alpha_wide, vshrq_n_u16(alpha_wide,7));
#endif

        /* spread the dest */
        dst_wide = vmovl_u8(dst_raw);

        /* alpha mul the dest */
        dst_wide = vmulq_u16 (dst_wide, alpha_wide);
        dst_cooked = vshrn_n_u16(dst_wide, 8);

        /* sum -- ignoring any byte lane overflows */
        dst_final = vadd_u8(src_raw, dst_cooked);
    }

#if    UNROLL > 2
    /* the 3rd and 4th bits of our unrolling */
    {
        uint8x8_t dst_cooked;
        uint16x8_t dst_wide;
        uint8x8_t alpha_narrow;
        uint16x8_t alpha_wide;

        alpha_narrow = vtbl1_u8(src_raw_2, alpha_mask);
#if 1
        /* reflect SkAlpha255To256() semantics a+1 vs a+a>>7 */
        /* we collapsed (255-a)+1 ... */
        alpha_wide = vsubw_u8(vdupq_n_u16(256), alpha_narrow);
#else
        alpha_wide = vsubw_u8(vdupq_n_u16(255), alpha_narrow);
        alpha_wide = vaddq_u16(alpha_wide, vshrq_n_u16(alpha_wide,7));
#endif

        /* spread the dest */
        dst_wide = vmovl_u8(dst_raw_2);

        /* alpha mul the dest */
        dst_wide = vmulq_u16 (dst_wide, alpha_wide);
        dst_cooked = vshrn_n_u16(dst_wide, 8);

        /* sum -- ignoring any byte lane overflows */
        dst_final_2 = vadd_u8(src_raw_2, dst_cooked);
    }
#endif

        vst1_u32(dst, vreinterpret_u32_u8(dst_final));
#if    UNROLL > 2
        vst1_u32(dst+2, vreinterpret_u32_u8(dst_final_2));
#endif

        src += UNROLL;
        dst += UNROLL;
        count -= UNROLL;
    }
#undef    UNROLL

    /* do any residual iterations */
        while (--count >= 0) {
            *dst = SkPMSrcOver(*src, *dst);
            src += 1;
            dst += 1;
        }
    }
}


/* Neon version of S32_Blend_BlitRow32()
 * portable version is in src/core/SkBlitRow_D32.cpp
 */
void S32_Blend_BlitRow32_neon(SkPMColor* SK_RESTRICT dst,
                              const SkPMColor* SK_RESTRICT src,
                              int count, U8CPU alpha) {
    SkASSERT(alpha <= 255);
    if (count > 0) {
        uint16_t src_scale = SkAlpha255To256(alpha);
        uint16_t dst_scale = 256 - src_scale;

    /* run them N at a time through the NEON unit */
    /* note that each 1 is 4 bytes, each treated exactly the same,
     * so we can work under that guise. We *do* know that the src&dst
     * will be 32-bit aligned quantities, so we can specify that on
     * the load/store ops and do a neon 'reinterpret' to get us to
     * byte-sized (pun intended) pieces that we widen/multiply/shift
     * we're limited at 128 bits in the wide ops, which is 8x16bits
     * or a pair of 32 bit src/dsts.
     */
    /* we *could* manually unroll this loop so that we load 128 bits
     * (as a pair of 64s) from each of src and dst, processing them
     * in pieces. This might give us a little better management of
     * the memory latency, but my initial attempts here did not
     * produce an instruction stream that looked all that nice.
     */
#define    UNROLL    2
    while (count >= UNROLL) {
        uint8x8_t  src_raw, dst_raw, dst_final;
        uint16x8_t  src_wide, dst_wide;

        /* get 64 bits of src, widen it, multiply by src_scale */
        src_raw = vreinterpret_u8_u32(vld1_u32(src));
        src_wide = vmovl_u8(src_raw);
        /* gcc hoists vdupq_n_u16(), better than using vmulq_n_u16() */
        src_wide = vmulq_u16 (src_wide, vdupq_n_u16(src_scale));

        /* ditto with dst */
        dst_raw = vreinterpret_u8_u32(vld1_u32(dst));
        dst_wide = vmovl_u8(dst_raw);

        /* combine add with dst multiply into mul-accumulate */
        dst_wide = vmlaq_u16(src_wide, dst_wide, vdupq_n_u16(dst_scale));

        dst_final = vshrn_n_u16(dst_wide, 8);
        vst1_u32(dst, vreinterpret_u32_u8(dst_final));

        src += UNROLL;
        dst += UNROLL;
        count -= UNROLL;
    }
    /* RBE: well, i don't like how gcc manages src/dst across the above
     * loop it's constantly calculating src+bias, dst+bias and it only
     * adjusts the real ones when we leave the loop. Not sure why
     * it's "hoisting down" (hoisting implies above in my lexicon ;))
     * the adjustments to src/dst/count, but it does...
     * (might be SSA-style internal logic...
     */

#if    UNROLL == 2
    if (count == 1) {
            *dst = SkAlphaMulQ(*src, src_scale) + SkAlphaMulQ(*dst, dst_scale);
    }
#else
    if (count > 0) {
            do {
                *dst = SkAlphaMulQ(*src, src_scale) + SkAlphaMulQ(*dst, dst_scale);
                src += 1;
                dst += 1;
            } while (--count > 0);
    }
#endif

#undef    UNROLL
    }
}

///////////////////////////////////////////////////////////////////////////////

#undef    DEBUG_OPAQUE_DITHER

#if    defined(DEBUG_OPAQUE_DITHER)
static void showme8(char *str, void *p, int len)
{
    static char buf[256];
    char tbuf[32];
    int i;
    char *pc = (char*) p;
    sprintf(buf,"%8s:", str);
    for(i=0;i<len;i++) {
        sprintf(tbuf, "   %02x", pc[i]);
        strcat(buf, tbuf);
    }
    SkDebugf("%s\n", buf);
}
static void showme16(char *str, void *p, int len)
{
    static char buf[256];
    char tbuf[32];
    int i;
    uint16_t *pc = (uint16_t*) p;
    sprintf(buf,"%8s:", str);
    len = (len / sizeof(uint16_t));    /* passed as bytes */
    for(i=0;i<len;i++) {
        sprintf(tbuf, " %04x", pc[i]);
        strcat(buf, tbuf);
    }
    SkDebugf("%s\n", buf);
}
#endif

void S32A_D565_Opaque_Dither_neon (uint16_t * SK_RESTRICT dst,
                                   const SkPMColor* SK_RESTRICT src,
                                   int count, U8CPU alpha, int x, int y) {
    SkASSERT(255 == alpha);

#define    UNROLL    8

    if (count >= UNROLL) {
    uint8x8_t dbase;

#if    defined(DEBUG_OPAQUE_DITHER)
    uint16_t tmpbuf[UNROLL];
    int td[UNROLL];
    int tdv[UNROLL];
    int ta[UNROLL];
    int tap[UNROLL];
    uint16_t in_dst[UNROLL];
    int offset = 0;
    int noisy = 0;
#endif

    const uint8_t *dstart = &gDitherMatrix_Neon[(y&3)*12 + (x&3)];
    dbase = vld1_u8(dstart);

        do {
        uint8x8_t sr, sg, sb, sa, d;
        uint16x8_t dst8, scale8, alpha8;
        uint16x8_t dst_r, dst_g, dst_b;

#if    defined(DEBUG_OPAQUE_DITHER)
    /* calculate 8 elements worth into a temp buffer */
    {
      int my_y = y;
      int my_x = x;
      SkPMColor* my_src = (SkPMColor*)src;
      uint16_t* my_dst = dst;
      int i;

          DITHER_565_SCAN(my_y);
          for(i=0;i<UNROLL;i++) {
            SkPMColor c = *my_src++;
            SkPMColorAssert(c);
            if (c) {
                unsigned a = SkGetPackedA32(c);

                int d = SkAlphaMul(DITHER_VALUE(my_x), SkAlpha255To256(a));
        tdv[i] = DITHER_VALUE(my_x);
        ta[i] = a;
        tap[i] = SkAlpha255To256(a);
        td[i] = d;

                unsigned sr = SkGetPackedR32(c);
                unsigned sg = SkGetPackedG32(c);
                unsigned sb = SkGetPackedB32(c);
                sr = SkDITHER_R32_FOR_565(sr, d);
                sg = SkDITHER_G32_FOR_565(sg, d);
                sb = SkDITHER_B32_FOR_565(sb, d);

                uint32_t src_expanded = (sg << 24) | (sr << 13) | (sb << 2);
                uint32_t dst_expanded = SkExpand_rgb_16(*my_dst);
                dst_expanded = dst_expanded * (SkAlpha255To256(255 - a) >> 3);
                // now src and dst expanded are in g:11 r:10 x:1 b:10
                tmpbuf[i] = SkCompact_rgb_16((src_expanded + dst_expanded) >> 5);
        td[i] = d;

            } else {
        tmpbuf[i] = *my_dst;
        ta[i] = tdv[i] = td[i] = 0xbeef;
        }
        in_dst[i] = *my_dst;
            my_dst += 1;
            DITHER_INC_X(my_x);
          }
    }
#endif

        /* source is in ABGR */
        {
        register uint8x8_t d0 asm("d0");
        register uint8x8_t d1 asm("d1");
        register uint8x8_t d2 asm("d2");
        register uint8x8_t d3 asm("d3");

        asm ("vld4.8    {d0-d3},[%4]  /* r=%P0 g=%P1 b=%P2 a=%P3 */"
            : "=w" (d0), "=w" (d1), "=w" (d2), "=w" (d3)
            : "r" (src)
                    );
            sr = d0; sg = d1; sb = d2; sa = d3;
        }

        /* calculate 'd', which will be 0..7 */
        /* dbase[] is 0..7; alpha is 0..256; 16 bits suffice */
#if defined(SK_BUILD_FOR_ANDROID)
        /* SkAlpha255To256() semantic a+1 vs a+a>>7 */
        alpha8 = vaddw_u8(vmovl_u8(sa), vdup_n_u8(1));
#else
        alpha8 = vaddw_u8(vmovl_u8(sa), vshr_n_u8(sa, 7));
#endif
        alpha8 = vmulq_u16(alpha8, vmovl_u8(dbase));
        d = vshrn_n_u16(alpha8, 8);    /* narrowing too */

        /* sr = sr - (sr>>5) + d */
        /* watching for 8-bit overflow.  d is 0..7; risky range of
         * sr is >248; and then (sr>>5) is 7 so it offsets 'd';
         * safe  as long as we do ((sr-sr>>5) + d) */
        sr = vsub_u8(sr, vshr_n_u8(sr, 5));
        sr = vadd_u8(sr, d);

        /* sb = sb - (sb>>5) + d */
        sb = vsub_u8(sb, vshr_n_u8(sb, 5));
        sb = vadd_u8(sb, d);

        /* sg = sg - (sg>>6) + d>>1; similar logic for overflows */
        sg = vsub_u8(sg, vshr_n_u8(sg, 6));
        sg = vadd_u8(sg, vshr_n_u8(d,1));

        /* need to pick up 8 dst's -- at 16 bits each, 128 bits */
        dst8 = vld1q_u16(dst);
        dst_b = vandq_u16(dst8, vdupq_n_u16(0x001F));
        dst_g = vandq_u16(vshrq_n_u16(dst8,5), vdupq_n_u16(0x003F));
        dst_r = vshrq_n_u16(dst8,11);    /* clearing hi bits */

        /* blend */
#if 1
        /* SkAlpha255To256() semantic a+1 vs a+a>>7 */
        /* originally 255-sa + 1 */
        scale8 = vsubw_u8(vdupq_n_u16(256), sa);
#else
        scale8 = vsubw_u8(vdupq_n_u16(255), sa);
        scale8 = vaddq_u16(scale8, vshrq_n_u16(scale8, 7));
#endif

#if 1
        /* combine the addq and mul, save 3 insns */
        scale8 = vshrq_n_u16(scale8, 3);
        dst_b = vmlaq_u16(vshll_n_u8(sb,2), dst_b, scale8);
        dst_g = vmlaq_u16(vshll_n_u8(sg,3), dst_g, scale8);
        dst_r = vmlaq_u16(vshll_n_u8(sr,2), dst_r, scale8);
#else
        /* known correct, but +3 insns over above */
        scale8 = vshrq_n_u16(scale8, 3);
        dst_b = vmulq_u16(dst_b, scale8);
        dst_g = vmulq_u16(dst_g, scale8);
        dst_r = vmulq_u16(dst_r, scale8);

        /* combine */
        /* NB: vshll widens, need to preserve those bits */
        dst_b = vaddq_u16(dst_b, vshll_n_u8(sb,2));
        dst_g = vaddq_u16(dst_g, vshll_n_u8(sg,3));
        dst_r = vaddq_u16(dst_r, vshll_n_u8(sr,2));
#endif

        /* repack to store */
        dst8 = vandq_u16(vshrq_n_u16(dst_b, 5), vdupq_n_u16(0x001F));
        dst8 = vsliq_n_u16(dst8, vshrq_n_u16(dst_g, 5), 5);
        dst8 = vsliq_n_u16(dst8, vshrq_n_u16(dst_r,5), 11);

        vst1q_u16(dst, dst8);

#if    defined(DEBUG_OPAQUE_DITHER)
        /* verify my 8 elements match the temp buffer */
    {
       int i, bad=0;
       static int invocation;

       for (i=0;i<UNROLL;i++)
        if (tmpbuf[i] != dst[i]) bad=1;
       if (bad) {
        SkDebugf("BAD S32A_D565_Opaque_Dither_neon(); invocation %d offset %d\n",
            invocation, offset);
        SkDebugf("  alpha 0x%x\n", alpha);
        for (i=0;i<UNROLL;i++)
            SkDebugf("%2d: %s %04x w %04x id %04x s %08x d %04x %04x %04x %04x\n",
            i, ((tmpbuf[i] != dst[i])?"BAD":"got"),
            dst[i], tmpbuf[i], in_dst[i], src[i], td[i], tdv[i], tap[i], ta[i]);

        showme16("alpha8", &alpha8, sizeof(alpha8));
        showme16("scale8", &scale8, sizeof(scale8));
        showme8("d", &d, sizeof(d));
        showme16("dst8", &dst8, sizeof(dst8));
        showme16("dst_b", &dst_b, sizeof(dst_b));
        showme16("dst_g", &dst_g, sizeof(dst_g));
        showme16("dst_r", &dst_r, sizeof(dst_r));
        showme8("sb", &sb, sizeof(sb));
        showme8("sg", &sg, sizeof(sg));
        showme8("sr", &sr, sizeof(sr));

        /* cop out */
        return;
       }
       offset += UNROLL;
       invocation++;
    }
#endif

            dst += UNROLL;
        src += UNROLL;
        count -= UNROLL;
        /* skip x += UNROLL, since it's unchanged mod-4 */
        } while (count >= UNROLL);
    }
#undef    UNROLL

    /* residuals */
    if (count > 0) {
        DITHER_565_SCAN(y);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            if (c) {
                unsigned a = SkGetPackedA32(c);

                // dither and alpha are just temporary variables to work-around
                // an ICE in debug.
                unsigned dither = DITHER_VALUE(x);
                unsigned alpha = SkAlpha255To256(a);
                int d = SkAlphaMul(dither, alpha);

                unsigned sr = SkGetPackedR32(c);
                unsigned sg = SkGetPackedG32(c);
                unsigned sb = SkGetPackedB32(c);
                sr = SkDITHER_R32_FOR_565(sr, d);
                sg = SkDITHER_G32_FOR_565(sg, d);
                sb = SkDITHER_B32_FOR_565(sb, d);

                uint32_t src_expanded = (sg << 24) | (sr << 13) | (sb << 2);
                uint32_t dst_expanded = SkExpand_rgb_16(*dst);
                dst_expanded = dst_expanded * (SkAlpha255To256(255 - a) >> 3);
                // now src and dst expanded are in g:11 r:10 x:1 b:10
                *dst = SkCompact_rgb_16((src_expanded + dst_expanded) >> 5);
            }
            dst += 1;
            DITHER_INC_X(x);
        } while (--count != 0);
    }
}

///////////////////////////////////////////////////////////////////////////////

/* 2009/10/27: RBE says "a work in progress"; debugging says ok;
 * speedup untested, but ARM version is 26 insns/iteration and
 * this NEON version is 21 insns/iteration-of-8 (2.62insns/element)
 * which is 10x the native version; that's pure instruction counts,
 * not accounting for any instruction or memory latencies.
 */

#undef    DEBUG_S32_OPAQUE_DITHER

void S32_D565_Opaque_Dither_neon(uint16_t* SK_RESTRICT dst,
                                 const SkPMColor* SK_RESTRICT src,
                                 int count, U8CPU alpha, int x, int y) {
    SkASSERT(255 == alpha);

#define    UNROLL    8
    if (count >= UNROLL) {
    uint8x8_t d;
    const uint8_t *dstart = &gDitherMatrix_Neon[(y&3)*12 + (x&3)];
    d = vld1_u8(dstart);

    while (count >= UNROLL) {
        uint8x8_t sr, sg, sb, sa;
        uint16x8_t dr, dg, db, da;
        uint16x8_t dst8;

        /* source is in ABGR ordering (R == lsb) */
        {
        register uint8x8_t d0 asm("d0");
        register uint8x8_t d1 asm("d1");
        register uint8x8_t d2 asm("d2");
        register uint8x8_t d3 asm("d3");

        asm ("vld4.8    {d0-d3},[%4]  /* r=%P0 g=%P1 b=%P2 a=%P3 */"
            : "=w" (d0), "=w" (d1), "=w" (d2), "=w" (d3)
            : "r" (src)
                    );
            sr = d0; sg = d1; sb = d2; sa = d3;
        }
        /* XXX: if we want to prefetch, hide it in the above asm()
         * using the gcc __builtin_prefetch(), the prefetch will
         * fall to the bottom of the loop -- it won't stick up
         * at the top of the loop, just after the vld4.
         */

        /* sr = sr - (sr>>5) + d */
        sr = vsub_u8(sr, vshr_n_u8(sr, 5));
        dr = vaddl_u8(sr, d);

        /* sb = sb - (sb>>5) + d */
        sb = vsub_u8(sb, vshr_n_u8(sb, 5));
        db = vaddl_u8(sb, d);

        /* sg = sg - (sg>>6) + d>>1; similar logic for overflows */
        sg = vsub_u8(sg, vshr_n_u8(sg, 6));
        dg = vaddl_u8(sg, vshr_n_u8(d,1));
        /* XXX: check that the "d>>1" here is hoisted */

        /* pack high bits of each into 565 format  (rgb, b is lsb) */
        dst8 = vshrq_n_u16(db, 3);
        dst8 = vsliq_n_u16(dst8, vshrq_n_u16(dg, 2), 5);
        dst8 = vsliq_n_u16(dst8, vshrq_n_u16(dr,3), 11);

        /* store it */
        vst1q_u16(dst, dst8);

#if    defined(DEBUG_S32_OPAQUE_DITHER)
        /* always good to know if we generated good results */
        {
        int i, myx = x, myy = y;
        DITHER_565_SCAN(myy);
        for (i=0;i<UNROLL;i++) {
            SkPMColor c = src[i];
            unsigned dither = DITHER_VALUE(myx);
            uint16_t val = SkDitherRGB32To565(c, dither);
            if (val != dst[i]) {
            SkDebugf("RBE: src %08x dither %02x, want %04x got %04x dbas[i] %02x\n",
                c, dither, val, dst[i], dstart[i]);
            }
            DITHER_INC_X(myx);
        }
        }
#endif

        dst += UNROLL;
        src += UNROLL;
        count -= UNROLL;
        x += UNROLL;        /* probably superfluous */
    }
    }
#undef    UNROLL

    /* residuals */
    if (count > 0) {
        DITHER_565_SCAN(y);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            SkASSERT(SkGetPackedA32(c) == 255);

            unsigned dither = DITHER_VALUE(x);
            *dst++ = SkDitherRGB32To565(c, dither);
            DITHER_INC_X(x);
        } while (--count != 0);
    }
}

void Color32_arm_neon(SkPMColor* dst, const SkPMColor* src, int count,
                      SkPMColor color) {
    if (count <= 0) {
        return;
    }

    if (0 == color) {
        if (src != dst) {
            memcpy(dst, src, count * sizeof(SkPMColor));
        }
        return;
    }

    unsigned colorA = SkGetPackedA32(color);
    if (255 == colorA) {
        sk_memset32(dst, color, count);
    } else {
        unsigned scale = 256 - SkAlpha255To256(colorA);

        if (count >= 8) {
            // at the end of this assembly, count will have been decremented
            // to a negative value. That is, if count mod 8 = x, it will be
            // -8 +x coming out.
            asm volatile (
                PLD128(src, 0)

                "vdup.32    q0, %[color]                \n\t"

                PLD128(src, 128)

                // scale numerical interval [0-255], so load as 8 bits
                "vdup.8     d2, %[scale]                \n\t"

                PLD128(src, 256)

                "subs       %[count], %[count], #8      \n\t"

                PLD128(src, 384)

                "Loop_Color32:                          \n\t"

                // load src color, 8 pixels, 4 64 bit registers
                // (and increment src).
                "vld1.32    {d4-d7}, [%[src]]!          \n\t"

                PLD128(src, 384)

                // multiply long by scale, 64 bits at a time,
                // destination into a 128 bit register.
                "vmull.u8   q4, d4, d2                  \n\t"
                "vmull.u8   q5, d5, d2                  \n\t"
                "vmull.u8   q6, d6, d2                  \n\t"
                "vmull.u8   q7, d7, d2                  \n\t"

                // shift the 128 bit registers, containing the 16
                // bit scaled values back to 8 bits, narrowing the
                // results to 64 bit registers.
                "vshrn.i16  d8, q4, #8                  \n\t"
                "vshrn.i16  d9, q5, #8                  \n\t"
                "vshrn.i16  d10, q6, #8                 \n\t"
                "vshrn.i16  d11, q7, #8                 \n\t"

                // adding back the color, using 128 bit registers.
                "vadd.i8    q6, q4, q0                  \n\t"
                "vadd.i8    q7, q5, q0                  \n\t"

                // store back the 8 calculated pixels (2 128 bit
                // registers), and increment dst.
                "vst1.32    {d12-d15}, [%[dst]]!        \n\t"

                "subs       %[count], %[count], #8      \n\t"
                "bge        Loop_Color32                \n\t"
                : [src] "+r" (src), [dst] "+r" (dst), [count] "+r" (count)
                : [color] "r" (color), [scale] "r" (scale)
                : "cc", "memory",
                  "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
                  "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15"
                          );
            // At this point, if we went through the inline assembly, count is
            // a negative value:
            // if the value is -8, there is no pixel left to process.
            // if the value is -7, there is one pixel left to process
            // ...
            // And'ing it with 7 will give us the number of pixels
            // left to process.
            count = count & 0x7;
        }

        while (count > 0) {
            *dst = color + SkAlphaMulQ(*src, scale);
            src += 1;
            dst += 1;
            count--;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

const SkBlitRow::Proc sk_blitrow_platform_565_procs_arm_neon[] = {
    // no dither
    // NOTE: For the two functions below, we don't have a special version
    //       that assumes that each source pixel is opaque. But our S32A is
    //       still faster than the default, so use it.
    S32A_D565_Opaque_neon,  // really S32_D565_Opaque
    S32A_D565_Blend_neon,   // really S32_D565_Blend
    S32A_D565_Opaque_neon,
    S32A_D565_Blend_neon,

    // dither
    S32_D565_Opaque_Dither_neon,
    S32_D565_Blend_Dither_neon,
    S32A_D565_Opaque_Dither_neon,
    NULL,   // S32A_D565_Blend_Dither
};

const SkBlitRow::Proc sk_blitrow_platform_4444_procs_arm_neon[] = {
    // no dither
    NULL,   // S32_D4444_Opaque,
    NULL,   // S32_D4444_Blend,
    NULL,   // S32A_D4444_Opaque,
    NULL,   // S32A_D4444_Blend,

    // dither
    NULL,   // S32_D4444_Opaque_Dither,
    NULL,   // S32_D4444_Blend_Dither,
    NULL,   // S32A_D4444_Opaque_Dither,
    NULL,   // S32A_D4444_Blend_Dither
};

const SkBlitRow::Proc32 sk_blitrow_platform_32_procs_arm_neon[] = {
    NULL,   // S32_Opaque,
    S32_Blend_BlitRow32_neon,        // S32_Blend,
    S32A_Opaque_BlitRow32_neon,        // S32A_Opaque,
    S32A_Blend_BlitRow32_arm        // S32A_Blend
};
