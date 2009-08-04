/*
 **
 ** Copyright 2009, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License"); 
 ** you may not use this file except in compliance with the License. 
 ** You may obtain a copy of the License at 
 **
 **     http://www.apache.org/licenses/LICENSE-2.0 
 **
 ** Unless required by applicable law or agreed to in writing, software 
 ** distributed under the License is distributed on an "AS IS" BASIS, 
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 ** See the License for the specific language governing permissions and 
 ** limitations under the License.
 */

#include <machine/cpu-features.h>
#include "SkBlitRow.h"
#include "SkColorPriv.h"
#include "SkDither.h"

#if defined(__ARM_HAVE_NEON) && !defined(SK_CPU_BENDIAN)
static void S32A_D565_Opaque_neon(uint16_t* SK_RESTRICT dst,
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

static void S32A_D565_Blend_neon(uint16_t* SK_RESTRICT dst,
                                 const SkPMColor* SK_RESTRICT src, int count,
                                 U8CPU alpha, int /*x*/, int /*y*/) {
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
                  
                  "add        %[alpha], %[alpha], %[alpha], lsr #7    \n\t"   // adjust range of alpha 0-256
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
                  
                  "vshr.u16   q11, q11, #8                    \n\t"   // shift down red
                  "vshr.u16   q12, q12, #8                    \n\t"   // shift down green
                  "vshr.u16   q13, q13, #8                    \n\t"   // shift down blue
                  
                  "vsli.u16   q13, q12, #5                    \n\t"   // insert green into blue
                  "vsli.u16   q13, q11, #11                   \n\t"   // insert red into green/blue
                  "vst1.16    {d26, d27}, [%[dst]]!           \n\t"   // write pixel back to dst, update ptr
                  
                  "bne        1b                              \n\t"   // if counter != 0, loop
                  "2:                                             \n\t"   // exit
                  
                  : [src] "+r" (src), [dst] "+r" (dst), [count] "+r" (count), [alpha] "+r" (alpha)
                  :
                  : "cc", "memory", "r4", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31"
                  );
    
    count &= 7;
    if (count > 0) {
        do {
            SkPMColor sc = *src++;
            if (sc)
            {
                uint16_t dc = *dst;
                unsigned sa = SkGetPackedA32(sc);
                unsigned dr, dg, db;
                
                if (sa == 255) {
                    dr = SkAlphaBlend(SkPacked32ToR16(sc), SkGetPackedR16(dc), alpha);
                    dg = SkAlphaBlend(SkPacked32ToG16(sc), SkGetPackedG16(dc), alpha);
                    db = SkAlphaBlend(SkPacked32ToB16(sc), SkGetPackedB16(dc), alpha);
                } else {
                    unsigned dst_scale = 255 - SkAlphaMul(sa, alpha);
                    dr = (SkPacked32ToR16(sc) * alpha + SkGetPackedR16(dc) * dst_scale) >> 8;
                    dg = (SkPacked32ToG16(sc) * alpha + SkGetPackedG16(dc) * dst_scale) >> 8;
                    db = (SkPacked32ToB16(sc) * alpha + SkGetPackedB16(dc) * dst_scale) >> 8;
                }
                *dst = SkPackRGB16(dr, dg, db);
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

static void S32_D565_Blend_Dither_neon(uint16_t *dst, const SkPMColor *src,
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

#define S32A_D565_Opaque_PROC       S32A_D565_Opaque_neon
#define S32A_D565_Blend_PROC        S32A_D565_Blend_neon
#define S32_D565_Blend_Dither_PROC  S32_D565_Blend_Dither_neon
#else
#define S32A_D565_Opaque_PROC       NULL
#define S32A_D565_Blend_PROC        NULL
#define S32_D565_Blend_Dither_PROC  NULL
#endif

/* Don't have a special version that assumes each src is opaque, but our S32A
    is still faster than the default, so use it here
 */
#define S32_D565_Opaque_PROC    S32A_D565_Opaque_PROC
#define S32_D565_Blend_PROC     S32A_D565_Blend_PROC

///////////////////////////////////////////////////////////////////////////////

const SkBlitRow::Proc SkBlitRow::gPlatform_565_Procs[] = {
    // no dither
    S32_D565_Opaque_PROC,
    S32_D565_Blend_PROC,
    S32A_D565_Opaque_PROC,
    S32A_D565_Blend_PROC,
    
    // dither
    NULL,   // S32_D565_Opaque_Dither,
    S32_D565_Blend_Dither_PROC,
    NULL,   // S32A_D565_Opaque_Dither,
    NULL,   // S32A_D565_Blend_Dither
};

const SkBlitRow::Proc SkBlitRow::gPlatform_4444_Procs[] = {
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

