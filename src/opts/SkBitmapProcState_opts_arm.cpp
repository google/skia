/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkBitmapProcState.h"

#if __ARM_ARCH__ >= 5 && !defined(SK_CPU_BENDIAN)
void S16_D16_nofilter_DX_arm(const SkBitmapProcState& s,
                             const uint32_t* SK_RESTRICT xy,
                             int count, uint16_t* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask));
    SkASSERT(s.fDoFilter == false);
    
    const uint16_t* SK_RESTRICT srcAddr = (const uint16_t*)s.fBitmap->getPixels();
    
    // buffer is y32, x16, x16, x16, x16, x16
    // bump srcAddr to the proper row, since we're told Y never changes
    SkASSERT((unsigned)xy[0] < (unsigned)s.fBitmap->height());
    srcAddr = (const uint16_t*)((const char*)srcAddr +
                                xy[0] * s.fBitmap->rowBytes());
    
    uint16_t src;
    
    if (1 == s.fBitmap->width()) {
        src = srcAddr[0];
        uint16_t dstValue = src;
        sk_memset16(colors, dstValue, count);
    } else {
        int i;
        const uint16_t* SK_RESTRICT xx = (const uint16_t*)(xy + 1);
        
        if((count >> 2) > 0) {
            asm volatile (
                          "mov        r8, %[count], lsr #2    \n\t"   // shift down count so we iterate in fours
                          "1:                                     \n\t"
                          "subs       r8, r8, #1              \n\t"   // decrement loop counter
                          "ldrh       r4, [%[xx]], #2         \n\t"   // load xx value, update ptr
                          "ldrh       r5, [%[xx]], #2         \n\t"   // load xx value, update ptr
                          "ldrh       r6, [%[xx]], #2         \n\t"   // load xx value, update ptr
                          "add        r4, r4, r4              \n\t"   // double offset for half word addressing
                          "ldrh       r7, [%[xx]], #2         \n\t"   // load xx value, update ptr
                          "add        r5, r5, r5              \n\t"   // double offset for half word addressing
                          "ldrh       r4, [%[srcAddr], r4]    \n\t"   // load value from srcAddr[*xx]
                          "add        r6, r6, r6              \n\t"   // double offset for half word addressing
                          "ldrh       r5, [%[srcAddr], r5]    \n\t"   // load value from srcAddr[*xx]
                          "add        r7, r7, r7              \n\t"   // double offset for half word addressing
                          "ldrh       r6, [%[srcAddr], r6]    \n\t"   // load value from srcAddr[*xx]
                          "ldrh       r7, [%[srcAddr], r7]    \n\t"   // load value from srcAddr[*xx]
                          "strh       r4, [%[colors]], #2     \n\t"   // store value to colors, update ptr
                          "strh       r5, [%[colors]], #2     \n\t"   // store value to colors, update ptr
                          "strh       r6, [%[colors]], #2     \n\t"   // store value to colors, update ptr
                          "strh       r7, [%[colors]], #2     \n\t"   // store value to colors, update ptr
                          "bgt        1b                      \n\t"   // branch if loop counter > 0
                          : [count] "+r" (count), [xx] "+r" (xx), [srcAddr] "+r" (srcAddr), [colors] "+r" (colors)
                          :
                          : "cc", "memory", "r4", "r5", "r6", "r7", "r8"
                          );
        }
        for (i = (count & 3); i > 0; --i) {
            SkASSERT(*xx < (unsigned)s.fBitmap->width());
            src = srcAddr[*xx++]; *colors++ = src;
        }
    }
}
#endif //__ARM_ARCH__ >= 5 && !defined(SK_CPU_BENDIAN)

#if defined(__ARM_HAVE_NEON) && !defined(SK_CPU_BENDIAN)
void S16_D16_filter_DX_arm(const SkBitmapProcState& s, 
                           const uint32_t* SK_RESTRICT xy, 
                           int count, uint16_t* SK_RESTRICT colors) 
{
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fDoFilter);
    
    const char* SK_RESTRICT srcAddr = (const char*)s.fBitmap->getPixels();
    unsigned rb = s.fBitmap->rowBytes();
    unsigned subY;
    const uint16_t* SK_RESTRICT row0;
    const uint16_t* SK_RESTRICT row1;
    unsigned int rowgap;
    const uint32_t c7ffe = 0x7ffe;
    
    // setup row ptrs and update proc_table
    {
        uint32_t XY = *xy++;
        unsigned y0 = XY >> 14;
        row0 = (const uint16_t*)(srcAddr + (y0 >> 4) * rb);
        row1 = (const uint16_t*)(srcAddr + (XY & 0x3FFF) * rb);
        rowgap = (unsigned int)row1 - (unsigned int)row0;
        subY = y0 & 0xF;
    }
    
    unsigned int count4 = ((count >> 2) << 4) | subY;
    count &= 3;
    
    asm volatile (
                  "and            r4, %[count4], #0xF             \n\t"   // mask off subY
                  "vmov.u16       d2[0], r4                       \n\t"   // move subY to Neon
                  "rsb            r4, r4, #16                     \n\t"   // r4 = 16-subY
                  "vmov.u16       d2[1], r4                       \n\t"   // move 16-subY to Neon
                  "movs           %[count4], %[count4], lsr #4    \n\t"   // shift count down, lose subY
                  "vmov.u16       d3, #16                         \n\t"   // create constant
                  "vmov.u16       q2, #31                         \n\t"   // set up blue mask
                  "beq            2f                              \n\t"   // if count4 == 0, exit
                  
                  "1:                                                 \n\t"
                  "ldmia          %[xy]!, {r4, r5, r6, r7}        \n\t"   // load four xy values
                  // xy = [ x0:14 | subX:4 | x1:14 ]
                  // extract subX for iter 0-3
                  "vmov           d0, r4, r5                      \n\t"   // move xy to Neon, iter 0-1
                  "vmov           d1, r6, r7                      \n\t"   // move xy to Neon, iter 2-3
                  
                  // Load 16 pixels for four filter iterations from memory.
                  // Because the source pixels are potentially scattered, each lane
                  // of each vector is loaded separately. Also, the X sub pixel
                  // offset is extracted.
                  
                  // iter 0
                  "mov            r8, r4, lsr #18                 \n\t"   // extract x0
                  "and            r4, %[c7ffe], r4, lsl #1        \n\t"   // extract x1 and make byte offset
                  "add            r8, %[row0], r8, lsl #1         \n\t"   // calculate address of row0[x0]
                  "add            r4, %[row0], r4                 \n\t"   // calculate address of row0[x1]
                  "vld1.u16       {d16[0]}, [r8], %[rowgap]       \n\t"   // load row0[x0] and move ptr to row1
                  "vld1.u16       {d17[0]}, [r4], %[rowgap]       \n\t"   // load row0[x1] and move ptr to row1
                  "vld1.u16       {d18[0]}, [r8]                  \n\t"   // load row1[x0]
                  "vld1.u16       {d19[0]}, [r4]                  \n\t"   // load row1[x1]
                  
                  // iter 1
                  "mov            r8, r5, lsr #18                 \n\t"   // extract x0
                  "and            r5, %[c7ffe], r5, lsl #1        \n\t"   // extract x1 and make byte offset
                  "add            r8, %[row0], r8, lsl #1         \n\t"   // calculate address of row0[x0]
                  "add            r5, %[row0], r5                 \n\t"   // calculate address of row0[x1]
                  "vld1.u16       {d16[1]}, [r8], %[rowgap]       \n\t"   // load row0[x0] and move ptr to row1
                  "vld1.u16       {d17[1]}, [r5], %[rowgap]       \n\t"   // load row0[x1] and move ptr to row1
                  "vld1.u16       {d18[1]}, [r8]                  \n\t"   // load row1[x0]
                  "vld1.u16       {d19[1]}, [r5]                  \n\t"   // load row1[x1]
                  
                  "vshrn.u32      d0, q0, #2                      \n\t"   // shift right subX by 2 and narrow
                  // iter 2
                  "mov            r8, r6, lsr #18                 \n\t"   // extract x0
                  "and            r6, %[c7ffe], r6, lsl #1        \n\t"   // extract x1 and make byte offset
                  "add            r8, %[row0], r8, lsl #1         \n\t"   // calculate address of row0[x0]
                  "add            r6, %[row0], r6                 \n\t"   // calculate address of row0[x1]
                  "vld1.u16       {d16[2]}, [r8], %[rowgap]       \n\t"   // load row0[x0] and move ptr to row1
                  "vld1.u16       {d17[2]}, [r6], %[rowgap]       \n\t"   // load row0[x1] and move ptr to row1
                  "vld1.u16       {d18[2]}, [r8]                  \n\t"   // load row1[x0]
                  "vld1.u16       {d19[2]}, [r6]                  \n\t"   // load row1[x1]
                  
                  "vshr.u16       d0, d0, #12                     \n\t"   // shift right subX to bottom 4 bits
                  // iter 3
                  "mov            r8, r7, lsr #18                 \n\t"   // extract x0
                  "and            r7, %[c7ffe], r7, lsl #1        \n\t"   // extract x1 and make byte offset
                  "add            r8, %[row0], r8, lsl #1         \n\t"   // calculate address of row0[x0]
                  "add            r7, %[row0], r7                 \n\t"   // calculate address of row0[x1]
                  "vld1.u16       {d16[3]}, [r8], %[rowgap]       \n\t"   // load row0[x0] and move ptr to row1
                  "vld1.u16       {d17[3]}, [r7], %[rowgap]       \n\t"   // load row0[x1] and move ptr to row1
                  "vld1.u16       {d18[3]}, [r8]                  \n\t"   // load row1[x0]
                  "vld1.u16       {d19[3]}, [r7]                  \n\t"   // load row1[x1]
                  
                  // Registers d16-d19 now contain pixels a00-a11 for 4 iterations:
                  //   d16 = [ a00_3 | a00_2 | a00_1 | a00_0 ]
                  //   d17 = [ a01_3 | a01_2 | a01_1 | a01_0 ]
                  //   d18 = [ a10_3 | a10_2 | a10_1 | a10_0 ]
                  //   d19 = [ a11_3 | a11_2 | a11_1 | a11_0 ]
                  //
                  // Extract RGB channels from each 565 pixel.
                  
                  "vshl.i16       q11, q8, #5                     \n\t"   // shift greens to top of each lane
                  "vand           q12, q8, q2                     \n\t"   // mask blues
                  "vshr.u16       q10, q8, #11                    \n\t"   // shift reds to bottom of each lane
                  "vshr.u16       q11, q11, #10                   \n\t"   // shift greens to bottom of each lane
                  "vshl.i16       q14, q9, #5                     \n\t"   // shift greens to top of each lane
                  "vand           q15, q9, q2                     \n\t"   // mask blues
                  "vshr.u16       q13, q9, #11                    \n\t"   // shift reds to bottom of each lane
                  "vshr.u16       q14, q14, #10                   \n\t"   // shift greens to bottom of each lane
                  
                  // There are now six Q regs, containing
                  //   q10 = [ a01r3 | a01r2 | a01r1 | a01r0 | a00r3 | a00r2 | a00r1 | a00r0 ]
                  //   q11 = [ a01g3 | a01g2 | a01g1 | a01g0 | a00g3 | a00g2 | a00g1 | a00g0 ]
                  //   q12 = [ a01b3 | a01b2 | a01b1 | a01b0 | a00b3 | a00b2 | a00b1 | a00b0 ]
                  //   q13 = [ a11r3 | a11r2 | a11r1 | a11r0 | a01r3 | a01r2 | a01r1 | a01r0 ]
                  //   q14 = [ a11g3 | a11g2 | a11g1 | a11g0 | a01g3 | a01g2 | a01g1 | a01g0 ]
                  //   q15 = [ a11b3 | a11b2 | a11b1 | a11b0 | a01b3 | a01b2 | a01b1 | a01b0 ]
                  // where aXXyZ: XX = pixel position, y = colour channel, Z = iteration
                  //   d0 = subX, d1 = 16-subX
                  //   d2[0] = subY, d2[1] = 16-subY
                  //   d3 = 16, q2(d4d5) = 31
                  
                  // The filter:
                  //
                  //           |        |
                  //     ---- a00 ---- a01 ----> * (16-y)
                  //           |        |
                  //     -----a10 ---- a11 ----> * y
                  //           |        |
                  //           V        V
                  //        * (16-x)   * x
                  //
                  // result = (a00.(16-y).(16-x) + a01.(16-y).x + a10.(16-x).y + a11.x.y) >> 8
                  //
                  
                  "vsub.u16       d1, d3, d0                      \n\t"   // calculate 16-subX
                  // multiply top pixel pair by (16-y)
                  "vmul.i16       q10, q10, d2[1]                 \n\t"   // top reds multiplied by (16-y)
                  "vmul.i16       q11, q11, d2[1]                 \n\t"   // top greens multiplied by (16-y)
                  "vmul.i16       q12, q12, d2[1]                 \n\t"   // top blues multiplied by (16-y)
                  // multiply bottom pixel pair by y
                  "vmul.i16       q13, q13, d2[0]                 \n\t"   // bottom reds multiplied by y
                  "vmul.i16       q14, q14, d2[0]                 \n\t"   // bottom greens multiplied by y
                  "vmul.i16       q15, q15, d2[0]                 \n\t"   // bottom blues multiplied by y
                  // mul/acc left pixels by (16-x)
                  "vmul.i16       d16, d20, d1                    \n\t"   // resultr  = a00r * (16-x)
                  "vmul.i16       d17, d22, d1                    \n\t"   // resultg  = a00g * (16-x)
                  "vmul.i16       d18, d24, d1                    \n\t"   // resultb  = a00b * (16-x) 
                  "vmla.i16       d16, d26, d1                    \n\t"   // resultr += a00r * (16-x)
                  "vmla.i16       d17, d28, d1                    \n\t"   // resultg += a00g * (16-x)
                  "vmla.i16       d18, d30, d1                    \n\t"   // resultb += a00b * (16-x)
                  // mul/acc right pixels by x
                  "vmla.i16       d16, d21, d0                    \n\t"   // resultr += a01r * x
                  "vmla.i16       d17, d23, d0                    \n\t"   // resultg += a01g * x
                  "vmla.i16       d18, d25, d0                    \n\t"   // resultb += a01b * x
                  "vmla.i16       d16, d27, d0                    \n\t"   // resultr += a11r * x
                  "vmla.i16       d17, d29, d0                    \n\t"   // resultg += a11g * x
                  "vmla.i16       d18, d31, d0                    \n\t"   // resultb += a11b * x
                  "subs           %[count4], %[count4], #1        \n\t"   // decrement counter
                  // shift results down 8 bits
                  "vshr.u16       q8, q8, #8                      \n\t"   // resultr >>= 8, resultg >>=8
                  "vshr.u16       d18, d18, #8                    \n\t"   // resultb >>= 8
                  // put rgb into 565
                  "vsli.i16       d18, d17, #5                    \n\t"   // shift greens into blues
                  "vsli.i16       d18, d16, #11                   \n\t"   // shift reds into greens and blues
                  "vst1.i16       {d18}, [%[colors]]!             \n\t"   // store result
                  "bgt            1b                              \n\t"   // if counter > 0, loop
                  "2:                                                 \n\t"   // exit
                  : [xy] "+r" (xy), [count4] "+r" (count4), [colors] "+r" (colors)
                  : [row0] "r" (row0), [rowgap] "r" (rowgap), [c7ffe] "r" (c7ffe)
                  : "cc", "memory", "r4", "r5", "r6", "r7", "r8", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31"
                  );
    
    while(count != 0)
    {
        uint32_t XX = *xy++;    // x0:14 | subX:4 | x1:14
        unsigned x0 = XX >> 14;
        unsigned x1 = XX & 0x3FFF;
        unsigned subX = x0 & 0xF;
        x0 >>= 4;
        
        uint32_t a00 = SkExpand_rgb_16(row0[x0]);
        uint32_t a01 = SkExpand_rgb_16(row0[x1]);
        uint32_t a10 = SkExpand_rgb_16(row1[x0]);
        uint32_t a11 = SkExpand_rgb_16(row1[x1]);
        
        int xy = subX * subY >> 3;
        uint32_t c = a00 * (32 - 2*subY - 2*subX + xy) +
        a01 * (2*subX - xy) +
        a10 * (2*subY - xy) +
        a11 * xy;
        
        *colors++ = SkCompact_rgb_16(c>>5);
        count--;
    }
}
#endif //defined(__ARM_HAVE_NEON) && !defined(SK_CPU_BENDIAN)

#if __ARM_ARCH__ >= 6 && !defined(SK_CPU_BENDIAN)
void SI8_D16_nofilter_DX_arm(const SkBitmapProcState& s,
                             const uint32_t* SK_RESTRICT xy,
                             int count, uint16_t* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask));
    SkASSERT(s.fDoFilter == false);
    
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

void SI8_opaque_D32_nofilter_DX_arm(const SkBitmapProcState& s,
                                    const uint32_t* SK_RESTRICT xy,
                                    int count, SkPMColor* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask));
    SkASSERT(s.fDoFilter == false);
    
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
    
    s.fBitmap->getColorTable()->unlockColors(false);
}
#endif //__ARM_ARCH__ >= 6 && !defined(SK_CPU_BENDIAN)

#if defined(__ARM_HAVE_NEON) && !defined(SK_CPU_BENDIAN)
static inline void Filter_32_direct(unsigned x, unsigned y, 
                                    SkPMColor a00, SkPMColor a01,
                                    SkPMColor a10, SkPMColor a11,
                                    SkPMColor *dst) {
    asm volatile(
                 "vdup.8         d0, %[y]                \n\t"   // duplicate y into d0
                 "vmov.u8        d16, #16                \n\t"   // set up constant in d16
                 "vsub.u8        d1, d16, d0             \n\t"   // d1 = 16-y
                 
                 "vdup.32        d4, %[a00]              \n\t"   // duplicate a00 into d4
                 "vdup.32        d5, %[a10]              \n\t"   // duplicate a10 into d5
                 "vmov.32        d4[1], %[a01]           \n\t"   // set top of d4 to a01
                 "vmov.32        d5[1], %[a11]           \n\t"   // set top of d5 to a11
                 
                 "vmull.u8       q3, d4, d1              \n\t"   // q3 = [a01|a00] * (16-y)
                 "vmull.u8       q0, d5, d0              \n\t"   // q0 = [a11|a10] * y
                 
                 "vdup.16        d5, %[x]                \n\t"   // duplicate x into d5
                 "vmov.u16       d16, #16                \n\t"   // set up constant in d16
                 "vsub.u16       d3, d16, d5             \n\t"   // d3 = 16-x
                 
                 "vmul.i16       d4, d7, d5              \n\t"   // d4  = a01 * x
                 "vmla.i16       d4, d1, d5              \n\t"   // d4 += a11 * x
                 "vmla.i16       d4, d6, d3              \n\t"   // d4 += a00 * (16-x)
                 "vmla.i16       d4, d0, d3              \n\t"   // d4 += a10 * (16-x)
                 "vshrn.i16      d0, q2, #8              \n\t"   // shift down result by 8
                 "vst1.32        {d0[0]}, [%[dst]]       \n\t"   // store result
                 :
                 : [x] "r" (x), [y] "r" (y), [a00] "r" (a00), [a01] "r" (a01), [a10] "r" (a10), [a11] "r" (a11), [dst] "r" (dst)
                 : "cc", "memory", "r4", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d16"
                 );
}

static inline void Filter_32_direct_alpha(unsigned x, unsigned y,
                                          SkPMColor a00, SkPMColor a01,
                                          SkPMColor a10, SkPMColor a11,
                                          SkPMColor *dst, uint16_t scale) {
    asm volatile(
                 "vdup.8         d0, %[y]                \n\t"   // duplicate y into d0
                 "vmov.u8        d16, #16                \n\t"   // set up constant in d16
                 "vsub.u8        d1, d16, d0             \n\t"   // d1 = 16-y
                 
                 "vdup.32        d4, %[a00]              \n\t"   // duplicate a00 into d4
                 "vdup.32        d5, %[a10]              \n\t"   // duplicate a10 into d5
                 "vmov.32        d4[1], %[a01]           \n\t"   // set top of d4 to a01
                 "vmov.32        d5[1], %[a11]           \n\t"   // set top of d5 to a11
                 
                 "vmull.u8       q3, d4, d1              \n\t"   // q3 = [a01|a00] * (16-y)
                 "vmull.u8       q0, d5, d0              \n\t"   // q0 = [a11|a10] * y
                 
                 "vdup.16        d5, %[x]                \n\t"   // duplicate x into d5
                 "vmov.u16       d16, #16                \n\t"   // set up constant in d16
                 "vsub.u16       d3, d16, d5             \n\t"   // d3 = 16-x
                 
                 "vmul.i16       d4, d7, d5              \n\t"   // d4  = a01 * x
                 "vmla.i16       d4, d1, d5              \n\t"   // d4 += a11 * x
                 "vmla.i16       d4, d6, d3              \n\t"   // d4 += a00 * (16-x)
                 "vmla.i16       d4, d0, d3              \n\t"   // d4 += a10 * (16-x)
                 "vdup.16        d3, %[scale]            \n\t"   // duplicate scale into d3
                 "vshr.u16       d4, d4, #8              \n\t"   // shift down result by 8
                 "vmul.i16       d4, d4, d3              \n\t"   // multiply result by scale
                 "vshrn.i16      d0, q2, #8              \n\t"   // shift down result by 8
                 "vst1.32        {d0[0]}, [%[dst]]       \n\t"   // store result
                 :
                 : [x] "r" (x), [y] "r" (y), [a00] "r" (a00), [a01] "r" (a01), [a10] "r" (a10), [a11] "r" (a11), [dst] "r" (dst), [scale] "r" (scale)
                 : "cc", "memory", "r4", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d16"
                 );
}

void SI8_opaque_D32_filter_DX_arm(const SkBitmapProcState& s,
                                  const uint32_t* SK_RESTRICT xy,
                                  int count, SkPMColor* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fDoFilter);
    
    const SkPMColor* SK_RESTRICT table = s.fBitmap->getColorTable()->lockColors();
    const char* SK_RESTRICT srcAddr = (const char*)s.fBitmap->getPixels();
    unsigned rb = s.fBitmap->rowBytes();
    unsigned subY;
    const uint8_t* SK_RESTRICT row0;
    const uint8_t* SK_RESTRICT row1;
    
    // setup row ptrs and update proc_table
    {
        uint32_t XY = *xy++;
        unsigned y0 = XY >> 14;
        row0 = (const uint8_t*)(srcAddr + (y0 >> 4) * rb);
        row1 = (const uint8_t*)(srcAddr + (XY & 0x3FFF) * rb);
        subY = y0 & 0xF;
    }
    
    do {
        uint32_t XX = *xy++;    // x0:14 | 4 | x1:14
        unsigned x0 = XX >> 14;
        unsigned x1 = XX & 0x3FFF;
        unsigned subX = x0 & 0xF;        
        x0 >>= 4;
        
        Filter_32_direct(subX, subY, table[row0[x0]], 
                         table[row0[x1]], 
                         table[row1[x0]], 
                         table[row1[x1]], colors);
        colors++;
    } while (--count != 0);
    
    s.fBitmap->getColorTable()->unlockColors(false);    
}

void SI8_opaque_D32_filter_DXDY_arm(const SkBitmapProcState& s,
                                    const uint32_t* SK_RESTRICT xy,
                                    int count, SkPMColor* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fDoFilter);
    
    const SkPMColor* SK_RESTRICT table = s.fBitmap->getColorTable()->lockColors();        
    const char* SK_RESTRICT srcAddr = (const char*)s.fBitmap->getPixels();
    int rb = s.fBitmap->rowBytes();
    
    do {
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
        
        const uint8_t* SK_RESTRICT row0 = (const uint8_t*)(srcAddr + y0 * rb);
        const uint8_t* SK_RESTRICT row1 = (const uint8_t*)(srcAddr + y1 * rb);
        
        Filter_32_direct(subX, subY, table[row0[x0]],
                         table[row0[x1]],
                         table[row1[x0]],
                         table[row1[x1]], colors);
        colors++;
    } while (--count != 0);
    
    s.fBitmap->getColorTable()->unlockColors(false);    
}

void SI8_alpha_D32_filter_DX_arm(const SkBitmapProcState& s,
                                 const uint32_t* SK_RESTRICT xy,
                                 int count, SkPMColor* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fDoFilter);
    
    unsigned scale = s.fAlphaScale;
    const SkPMColor* SK_RESTRICT table = s.fBitmap->getColorTable()->lockColors();
    const char* SK_RESTRICT srcAddr = (const char*)s.fBitmap->getPixels();
    unsigned rb = s.fBitmap->rowBytes();
    unsigned subY;
    const uint8_t* SK_RESTRICT row0;
    const uint8_t* SK_RESTRICT row1;
    
    // setup row ptrs and update proc_table
    {
        uint32_t XY = *xy++;
        unsigned y0 = XY >> 14;
        row0 = (const uint8_t*)(srcAddr + (y0 >> 4) * rb);
        row1 = (const uint8_t*)(srcAddr + (XY & 0x3FFF) * rb);
        subY = y0 & 0xF;
    }
    
    do {
        uint32_t XX = *xy++;    // x0:14 | 4 | x1:14
        unsigned x0 = XX >> 14;
        unsigned x1 = XX & 0x3FFF;
        unsigned subX = x0 & 0xF;        
        x0 >>= 4;
        
        Filter_32_direct_alpha(subX, subY, table[row0[x0]],
                               table[row0[x1]],
                               table[row1[x0]],
                               table[row1[x1]], colors, scale);
        colors++;
    } while (--count != 0);
    
    s.fBitmap->getColorTable()->unlockColors(false);    
}

void SI8_alpha_D32_filter_DXDY_arm(const SkBitmapProcState& s,
                                   const uint32_t* SK_RESTRICT xy,
                                   int count, SkPMColor* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fDoFilter);
    
    unsigned scale = s.fAlphaScale;
    const SkPMColor* SK_RESTRICT table = s.fBitmap->getColorTable()->lockColors();
    const char* SK_RESTRICT srcAddr = (const char*)s.fBitmap->getPixels();
    int rb = s.fBitmap->rowBytes();
    
    do {
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
        
        const uint8_t* SK_RESTRICT row0 = (const uint8_t*)(srcAddr + y0 * rb);
        const uint8_t* SK_RESTRICT row1 = (const uint8_t*)(srcAddr + y1 * rb);
        
        Filter_32_direct_alpha(subX, subY, table[row0[x0]],
                               table[row0[x1]],
                               table[row1[x0]],
                               table[row1[x1]], colors, scale);        
        colors++;
    } while (--count != 0);
    
    s.fBitmap->getColorTable()->unlockColors(false);    
}
#endif //defined(__ARM_HAVE_NEON) && !defined(SK_CPU_BENDIAN)

///////////////////////////////////////////////////////////////////////////////

void SkBitmapProcState::platformProcs() {
    bool doFilter = fDoFilter;
    bool isOpaque = 256 == fAlphaScale;
    bool justDx = false;

    if (fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask)) {
        justDx = true;
    }

    switch (fBitmap->config()) {
        case SkBitmap::kRGB_565_Config:
#if defined(__ARM_HAVE_NEON) && !defined(SK_CPU_BENDIAN)
            if (justDx && doFilter) {
                fSampleProc16 = S16_D16_filter_DX_arm;
            }
#endif
#if __ARM_ARCH__ >= 5 && !defined(SK_CPU_BENDIAN)
            if (justDx && !doFilter) {
                fSampleProc16 = S16_D16_nofilter_DX_arm;
            }
#endif
            break;  // k565
        case SkBitmap::kIndex8_Config:
#if __ARM_ARCH__ >= 6 && !defined(SK_CPU_BENDIAN)
            if (justDx && !doFilter) {
                fSampleProc16 = SI8_D16_nofilter_DX_arm;
                if (isOpaque) {
                    fSampleProc32 = SI8_opaque_D32_nofilter_DX_arm;
                }
            }
#endif
#if defined(__ARM_HAVE_NEON) && !defined(SK_CPU_BENDIAN)
            if (doFilter) {
                if (isOpaque) {
                    if (justDx) {
                        fSampleProc32 = SI8_opaque_D32_filter_DX_arm;
                    } else {
                        fSampleProc32 = SI8_opaque_D32_filter_DXDY_arm;
                    }
                } else {    // !isOpaque
                    if (justDx) {
                        fSampleProc32 = SI8_alpha_D32_filter_DX_arm;
                    } else {
                        fSampleProc32 = SI8_alpha_D32_filter_DXDY_arm;
                    }
                }
            }
#endif
            break;  // kIndex8
        default:
            break;
    }
}

