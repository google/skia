/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitRow_opts_arm_neon.h"

#include "SkBlitMask.h"
#include "SkBlitRow.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkMathPriv.h"
#include "SkUtils.h"

#include "SkColor_opts_neon.h"
#include <arm_neon.h>

#ifdef SK_CPU_ARM64
static inline uint8x8x4_t sk_vld4_u8_arm64_3(const SkPMColor* SK_RESTRICT & src) {
    uint8x8x4_t vsrc;
    uint8x8_t vsrc_0, vsrc_1, vsrc_2;

    asm (
        "ld4    {v0.8b - v3.8b}, [%[src]], #32 \t\n"
        "mov    %[vsrc0].8b, v0.8b             \t\n"
        "mov    %[vsrc1].8b, v1.8b             \t\n"
        "mov    %[vsrc2].8b, v2.8b             \t\n"
        : [vsrc0] "=w" (vsrc_0), [vsrc1] "=w" (vsrc_1),
          [vsrc2] "=w" (vsrc_2), [src] "+&r" (src)
        : : "v0", "v1", "v2", "v3"
    );

    vsrc.val[0] = vsrc_0;
    vsrc.val[1] = vsrc_1;
    vsrc.val[2] = vsrc_2;

    return vsrc;
}

static inline uint8x8x4_t sk_vld4_u8_arm64_4(const SkPMColor* SK_RESTRICT & src) {
    uint8x8x4_t vsrc;
    uint8x8_t vsrc_0, vsrc_1, vsrc_2, vsrc_3;

    asm (
        "ld4    {v0.8b - v3.8b}, [%[src]], #32 \t\n"
        "mov    %[vsrc0].8b, v0.8b             \t\n"
        "mov    %[vsrc1].8b, v1.8b             \t\n"
        "mov    %[vsrc2].8b, v2.8b             \t\n"
        "mov    %[vsrc3].8b, v3.8b             \t\n"
        : [vsrc0] "=w" (vsrc_0), [vsrc1] "=w" (vsrc_1),
          [vsrc2] "=w" (vsrc_2), [vsrc3] "=w" (vsrc_3),
          [src] "+&r" (src)
        : : "v0", "v1", "v2", "v3"
    );

    vsrc.val[0] = vsrc_0;
    vsrc.val[1] = vsrc_1;
    vsrc.val[2] = vsrc_2;
    vsrc.val[3] = vsrc_3;

    return vsrc;
}
#endif

void S32_D565_Opaque_neon(uint16_t* SK_RESTRICT dst,
                           const SkPMColor* SK_RESTRICT src, int count,
                           U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 == alpha);

    while (count >= 8) {
        uint8x8x4_t vsrc;
        uint16x8_t vdst;

        // Load
#ifdef SK_CPU_ARM64
        vsrc = sk_vld4_u8_arm64_3(src);
#else
        vsrc = vld4_u8((uint8_t*)src);
        src += 8;
#endif

        // Convert src to 565
        vdst = SkPixel32ToPixel16_neon8(vsrc);

        // Store
        vst1q_u16(dst, vdst);

        // Prepare next iteration
        dst += 8;
        count -= 8;
    };

    // Leftovers
    while (count > 0) {
        SkPMColor c = *src++;
        SkPMColorAssert(c);
        *dst = SkPixel32ToPixel16_ToU16(c);
        dst++;
        count--;
    };
}

void S32_D565_Blend_neon(uint16_t* SK_RESTRICT dst,
                          const SkPMColor* SK_RESTRICT src, int count,
                          U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 > alpha);

    uint16x8_t vmask_blue, vscale;

    // prepare constants
    vscale = vdupq_n_u16(SkAlpha255To256(alpha));
    vmask_blue = vmovq_n_u16(0x1F);

    while (count >= 8) {
        uint8x8x4_t vsrc;
        uint16x8_t vdst, vdst_r, vdst_g, vdst_b;
        uint16x8_t vres_r, vres_g, vres_b;

        // Load src
#ifdef SK_CPU_ARM64
        vsrc = sk_vld4_u8_arm64_3(src);
#else
        {
        register uint8x8_t d0 asm("d0");
        register uint8x8_t d1 asm("d1");
        register uint8x8_t d2 asm("d2");
        register uint8x8_t d3 asm("d3");

        asm (
            "vld4.8    {d0-d3},[%[src]]!"
            : "=w" (d0), "=w" (d1), "=w" (d2), "=w" (d3), [src] "+&r" (src)
            :
        );
        vsrc.val[0] = d0;
        vsrc.val[1] = d1;
        vsrc.val[2] = d2;
        }
#endif

        // Load and unpack dst
        vdst = vld1q_u16(dst);
        vdst_g = vshlq_n_u16(vdst, 5);        // shift green to top of lanes
        vdst_b = vandq_u16(vdst, vmask_blue); // extract blue
        vdst_r = vshrq_n_u16(vdst, 6+5);      // extract red
        vdst_g = vshrq_n_u16(vdst_g, 5+5);    // extract green

        // Shift src to 565 range
        vsrc.val[NEON_R] = vshr_n_u8(vsrc.val[NEON_R], 3);
        vsrc.val[NEON_G] = vshr_n_u8(vsrc.val[NEON_G], 2);
        vsrc.val[NEON_B] = vshr_n_u8(vsrc.val[NEON_B], 3);

        // Scale src - dst
        vres_r = vmovl_u8(vsrc.val[NEON_R]) - vdst_r;
        vres_g = vmovl_u8(vsrc.val[NEON_G]) - vdst_g;
        vres_b = vmovl_u8(vsrc.val[NEON_B]) - vdst_b;

        vres_r = vshrq_n_u16(vres_r * vscale, 8);
        vres_g = vshrq_n_u16(vres_g * vscale, 8);
        vres_b = vshrq_n_u16(vres_b * vscale, 8);

        vres_r += vdst_r;
        vres_g += vdst_g;
        vres_b += vdst_b;

        // Combine
        vres_b = vsliq_n_u16(vres_b, vres_g, 5);    // insert green into blue
        vres_b = vsliq_n_u16(vres_b, vres_r, 6+5);  // insert red into green/blue

        // Store
        vst1q_u16(dst, vres_b);
        dst += 8;
        count -= 8;
    }
    if (count > 0) {
        int scale = SkAlpha255To256(alpha);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            uint16_t d = *dst;
            *dst++ = SkPackRGB16(
                    SkAlphaBlend(SkPacked32ToR16(c), SkGetPackedR16(d), scale),
                    SkAlphaBlend(SkPacked32ToG16(c), SkGetPackedG16(d), scale),
                    SkAlphaBlend(SkPacked32ToB16(c), SkGetPackedB16(d), scale));
        } while (--count != 0);
    }
}

#ifdef SK_CPU_ARM32
void S32A_D565_Opaque_neon(uint16_t* SK_RESTRICT dst,
                           const SkPMColor* SK_RESTRICT src, int count,
                           U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 == alpha);

    if (count >= 8) {
        uint16_t* SK_RESTRICT keep_dst = 0;

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
                      "vaddhn.u16 d4, q12, q8                 \n\t"
                      // intentionally don't calculate alpha
                      // result in d4-d6

            #ifdef SK_PMCOLOR_IS_RGBA
                      "vqadd.u8   d6, d6, d0                  \n\t"
                      "vqadd.u8   d5, d5, d1                  \n\t"
                      "vqadd.u8   d4, d4, d2                  \n\t"
            #else
                      "vqadd.u8   d6, d6, d2                  \n\t"
                      "vqadd.u8   d5, d5, d1                  \n\t"
                      "vqadd.u8   d4, d4, d0                  \n\t"
            #endif

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
        uint16_t* SK_RESTRICT keep_dst = 0;

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
                      "vuzp.u16   q0, q1                      \n\t"
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
                      "vaddhn.u16 d4, q12, q8                 \n\t"
                      // intentionally don't calculate alpha
                      // result in d4-d6

            #ifdef SK_PMCOLOR_IS_RGBA
                      "vqadd.u8   d6, d6, d0                  \n\t"
                      "vqadd.u8   d5, d5, d1                  \n\t"
                      "vqadd.u8   d4, d4, d2                  \n\t"
            #else
                      "vqadd.u8   d6, d6, d2                  \n\t"
                      "vqadd.u8   d5, d5, d1                  \n\t"
                      "vqadd.u8   d4, d4, d0                  \n\t"
            #endif

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

#else // #ifdef SK_CPU_ARM32

void S32A_D565_Opaque_neon(uint16_t* SK_RESTRICT dst,
                           const SkPMColor* SK_RESTRICT src, int count,
                           U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 == alpha);

    if (count >= 16) {
        asm (
            "movi    v4.8h, #0x80                   \t\n"

            "1:                                     \t\n"
            "sub     %w[count], %w[count], #16      \t\n"
            "ld1     {v16.8h-v17.8h}, [%[dst]]      \t\n"
            "ld4     {v0.16b-v3.16b}, [%[src]], #64 \t\n"
            "prfm    pldl1keep, [%[src],#512]       \t\n"
            "prfm    pldl1keep, [%[dst],#256]       \t\n"
            "ushr    v20.8h, v17.8h, #5             \t\n"
            "ushr    v31.8h, v16.8h, #5             \t\n"
            "xtn     v6.8b, v31.8h                  \t\n"
            "xtn2    v6.16b, v20.8h                 \t\n"
            "ushr    v20.8h, v17.8h, #11            \t\n"
            "shl     v19.16b, v6.16b, #2            \t\n"
            "ushr    v31.8h, v16.8h, #11            \t\n"
            "xtn     v22.8b, v31.8h                 \t\n"
            "xtn2    v22.16b, v20.8h                \t\n"
            "shl     v18.16b, v22.16b, #3           \t\n"
            "mvn     v3.16b, v3.16b                 \t\n"
            "xtn     v16.8b, v16.8h                 \t\n"
            "mov     v7.16b, v4.16b                 \t\n"
            "xtn2    v16.16b, v17.8h                \t\n"
            "umlal   v7.8h, v3.8b, v19.8b           \t\n"
            "shl     v16.16b, v16.16b, #3           \t\n"
            "mov     v22.16b, v4.16b                \t\n"
            "ushr    v24.8h, v7.8h, #6              \t\n"
            "umlal   v22.8h, v3.8b, v18.8b          \t\n"
            "ushr    v20.8h, v22.8h, #5             \t\n"
            "addhn   v20.8b, v22.8h, v20.8h         \t\n"
            "cmp     %w[count], #16                 \t\n"
            "mov     v6.16b, v4.16b                 \t\n"
            "mov     v5.16b, v4.16b                 \t\n"
            "umlal   v6.8h, v3.8b, v16.8b           \t\n"
            "umlal2  v5.8h, v3.16b, v19.16b         \t\n"
            "mov     v17.16b, v4.16b                \t\n"
            "ushr    v19.8h, v6.8h, #5              \t\n"
            "umlal2  v17.8h, v3.16b, v18.16b        \t\n"
            "addhn   v7.8b, v7.8h, v24.8h           \t\n"
            "ushr    v18.8h, v5.8h, #6              \t\n"
            "ushr    v21.8h, v17.8h, #5             \t\n"
            "addhn2  v7.16b, v5.8h, v18.8h          \t\n"
            "addhn2  v20.16b, v17.8h, v21.8h        \t\n"
            "mov     v22.16b, v4.16b                \t\n"
            "addhn   v6.8b, v6.8h, v19.8h           \t\n"
            "umlal2  v22.8h, v3.16b, v16.16b        \t\n"
            "ushr    v5.8h, v22.8h, #5              \t\n"
            "addhn2  v6.16b, v22.8h, v5.8h          \t\n"
            "uqadd   v7.16b, v1.16b, v7.16b         \t\n"
#if SK_PMCOLOR_BYTE_ORDER(B,G,R,A)
            "uqadd   v20.16b, v2.16b, v20.16b       \t\n"
            "uqadd   v6.16b, v0.16b, v6.16b         \t\n"
#elif SK_PMCOLOR_BYTE_ORDER(R,G,B,A)
            "uqadd   v20.16b, v0.16b, v20.16b       \t\n"
            "uqadd   v6.16b, v2.16b, v6.16b         \t\n"
#else
#error "This function only supports BGRA and RGBA."
#endif
            "shll    v22.8h, v20.8b, #8             \t\n"
            "shll    v5.8h, v7.8b, #8               \t\n"
            "sri     v22.8h, v5.8h, #5              \t\n"
            "shll    v17.8h, v6.8b, #8              \t\n"
            "shll2   v23.8h, v20.16b, #8            \t\n"
            "shll2   v7.8h, v7.16b, #8              \t\n"
            "sri     v22.8h, v17.8h, #11            \t\n"
            "sri     v23.8h, v7.8h, #5              \t\n"
            "shll2   v6.8h, v6.16b, #8              \t\n"
            "st1     {v22.8h}, [%[dst]], #16        \t\n"
            "sri     v23.8h, v6.8h, #11             \t\n"
            "st1     {v23.8h}, [%[dst]], #16        \t\n"
            "b.ge    1b                             \t\n"
            : [dst] "+&r" (dst), [src] "+&r" (src), [count] "+&r" (count)
            :: "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
               "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24",
               "v31"
        );
    }
        // Leftovers
    if (count > 0) {
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            if (c) {
                *dst = SkSrcOver32To16(c, *dst);
            }
            dst += 1;
        } while (--count != 0);
    }
}
#endif // #ifdef SK_CPU_ARM32

static uint32_t pmcolor_to_expand16(SkPMColor c) {
    unsigned r = SkGetPackedR32(c);
    unsigned g = SkGetPackedG32(c);
    unsigned b = SkGetPackedB32(c);
    return (g << 24) | (r << 13) | (b << 2);
}

void Color32A_D565_neon(uint16_t dst[], SkPMColor src, int count, int x, int y) {
    uint32_t src_expand;
    unsigned scale;
    uint16x8_t vmask_blue;

    if (count <= 0) return;
    SkASSERT(((size_t)dst & 0x01) == 0);

    /*
     * This preamble code is in order to make dst aligned to 8 bytes
     * in the next mutiple bytes read & write access.
     */
    src_expand = pmcolor_to_expand16(src);
    scale = SkAlpha255To256(0xFF - SkGetPackedA32(src)) >> 3;

#define DST_ALIGN 8

    /*
     * preamble_size is in byte, meantime, this blend32_16_row_neon updates 2 bytes at a time.
     */
    int preamble_size = (DST_ALIGN - (size_t)dst) & (DST_ALIGN - 1);

    for (int i = 0; i < preamble_size; i+=2, dst++) {
        uint32_t dst_expand = SkExpand_rgb_16(*dst) * scale;
        *dst = SkCompact_rgb_16((src_expand + dst_expand) >> 5);
        if (--count == 0)
            break;
    }

    int count16 = 0;
    count16 = count >> 4;
    vmask_blue = vmovq_n_u16(SK_B16_MASK);

    if (count16) {
        uint16x8_t wide_sr;
        uint16x8_t wide_sg;
        uint16x8_t wide_sb;
        uint16x8_t wide_256_sa;

        unsigned sr = SkGetPackedR32(src);
        unsigned sg = SkGetPackedG32(src);
        unsigned sb = SkGetPackedB32(src);
        unsigned sa = SkGetPackedA32(src);

        // Operation: dst_rgb = src_rgb + ((256 - src_a) >> 3) x dst_rgb
        // sr: 8-bit based, dr: 5-bit based, with dr x ((256-sa)>>3), 5-bit left shifted,
        //thus, for sr, do 2-bit left shift to match MSB : (8 + 2 = 5 + 5)
        wide_sr = vshlq_n_u16(vmovl_u8(vdup_n_u8(sr)), 2); // widen and src_red shift

        // sg: 8-bit based, dg: 6-bit based, with dg x ((256-sa)>>3), 5-bit left shifted,
        //thus, for sg, do 3-bit left shift to match MSB : (8 + 3 = 6 + 5)
        wide_sg = vshlq_n_u16(vmovl_u8(vdup_n_u8(sg)), 3); // widen and src_grn shift

        // sb: 8-bit based, db: 5-bit based, with db x ((256-sa)>>3), 5-bit left shifted,
        //thus, for sb, do 2-bit left shift to match MSB : (8 + 2 = 5 + 5)
        wide_sb = vshlq_n_u16(vmovl_u8(vdup_n_u8(sb)), 2); // widen and src blu shift

        wide_256_sa =
            vshrq_n_u16(vsubw_u8(vdupq_n_u16(256), vdup_n_u8(sa)), 3); // (256 - sa) >> 3

        while (count16-- > 0) {
            uint16x8_t vdst1, vdst1_r, vdst1_g, vdst1_b;
            uint16x8_t vdst2, vdst2_r, vdst2_g, vdst2_b;
            vdst1 = vld1q_u16(dst);
            dst += 8;
            vdst2 = vld1q_u16(dst);
            dst -= 8;    //to store dst again.

            vdst1_g = vshlq_n_u16(vdst1, SK_R16_BITS);                 // shift green to top of lanes
            vdst1_b = vdst1 & vmask_blue;                              // extract blue
            vdst1_r = vshrq_n_u16(vdst1, SK_R16_SHIFT);                // extract red
            vdst1_g = vshrq_n_u16(vdst1_g, SK_R16_BITS + SK_B16_BITS); // extract green

            vdst2_g = vshlq_n_u16(vdst2, SK_R16_BITS);                 // shift green to top of lanes
            vdst2_b = vdst2 & vmask_blue;                              // extract blue
            vdst2_r = vshrq_n_u16(vdst2, SK_R16_SHIFT);                // extract red
            vdst2_g = vshrq_n_u16(vdst2_g, SK_R16_BITS + SK_B16_BITS); // extract green

            vdst1_r = vmlaq_u16(wide_sr, wide_256_sa, vdst1_r);        // sr + (256-sa) x dr1
            vdst1_g = vmlaq_u16(wide_sg, wide_256_sa, vdst1_g);        // sg + (256-sa) x dg1
            vdst1_b = vmlaq_u16(wide_sb, wide_256_sa, vdst1_b);        // sb + (256-sa) x db1

            vdst2_r = vmlaq_u16(wide_sr, wide_256_sa, vdst2_r);        // sr + (256-sa) x dr2
            vdst2_g = vmlaq_u16(wide_sg, wide_256_sa, vdst2_g);        // sg + (256-sa) x dg2
            vdst2_b = vmlaq_u16(wide_sb, wide_256_sa, vdst2_b);        // sb + (256-sa) x db2

            vdst1_r = vshrq_n_u16(vdst1_r, 5);                         // 5-bit right shift for 5-bit red
            vdst1_g = vshrq_n_u16(vdst1_g, 5);                         // 5-bit right shift for 6-bit green
            vdst1_b = vshrq_n_u16(vdst1_b, 5);                         // 5-bit right shift for 5-bit blue

            vdst1 = vsliq_n_u16(vdst1_b, vdst1_g, SK_G16_SHIFT);       // insert green into blue
            vdst1 = vsliq_n_u16(vdst1, vdst1_r, SK_R16_SHIFT);         // insert red into green/blue

            vdst2_r = vshrq_n_u16(vdst2_r, 5);                         // 5-bit right shift for 5-bit red
            vdst2_g = vshrq_n_u16(vdst2_g, 5);                         // 5-bit right shift for 6-bit green
            vdst2_b = vshrq_n_u16(vdst2_b, 5);                         // 5-bit right shift for 5-bit blue

            vdst2 = vsliq_n_u16(vdst2_b, vdst2_g, SK_G16_SHIFT);       // insert green into blue
            vdst2 = vsliq_n_u16(vdst2, vdst2_r, SK_R16_SHIFT);         // insert red into green/blue

            vst1q_u16(dst, vdst1);
            dst += 8;
            vst1q_u16(dst, vdst2);
            dst += 8;
        }
    }

    count &= 0xF;
    if (count > 0) {
        do {
            uint32_t dst_expand = SkExpand_rgb_16(*dst) * scale;
            *dst = SkCompact_rgb_16((src_expand + dst_expand) >> 5);
            dst += 1;
        } while (--count != 0);
    }
}

static inline uint16x8_t SkDiv255Round_neon8(uint16x8_t prod) {
    prod += vdupq_n_u16(128);
    prod += vshrq_n_u16(prod, 8);
    return vshrq_n_u16(prod, 8);
}

void S32A_D565_Blend_neon(uint16_t* SK_RESTRICT dst,
                          const SkPMColor* SK_RESTRICT src, int count,
                          U8CPU alpha, int /*x*/, int /*y*/) {
   SkASSERT(255 > alpha);

    /* This code implements a Neon version of S32A_D565_Blend. The results have
     * a few mismatches compared to the original code. These mismatches never
     * exceed 1.
     */

    if (count >= 8) {
        uint16x8_t valpha_max, vmask_blue;
        uint8x8_t valpha;

        // prepare constants
        valpha_max = vmovq_n_u16(255);
        valpha = vdup_n_u8(alpha);
        vmask_blue = vmovq_n_u16(SK_B16_MASK);

        do {
            uint16x8_t vdst, vdst_r, vdst_g, vdst_b;
            uint16x8_t vres_a, vres_r, vres_g, vres_b;
            uint8x8x4_t vsrc;

            // load pixels
            vdst = vld1q_u16(dst);
#ifdef SK_CPU_ARM64
            vsrc = sk_vld4_u8_arm64_4(src);
#elif (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 6))
            asm (
                "vld4.u8 %h[vsrc], [%[src]]!"
                : [vsrc] "=w" (vsrc), [src] "+&r" (src)
                : :
            );
#else
            register uint8x8_t d0 asm("d0");
            register uint8x8_t d1 asm("d1");
            register uint8x8_t d2 asm("d2");
            register uint8x8_t d3 asm("d3");

            asm volatile (
                "vld4.u8    {d0-d3},[%[src]]!;"
                : "=w" (d0), "=w" (d1), "=w" (d2), "=w" (d3),
                  [src] "+&r" (src)
                : :
            );
            vsrc.val[0] = d0;
            vsrc.val[1] = d1;
            vsrc.val[2] = d2;
            vsrc.val[3] = d3;
#endif


            // deinterleave dst
            vdst_g = vshlq_n_u16(vdst, SK_R16_BITS);        // shift green to top of lanes
            vdst_b = vdst & vmask_blue;                     // extract blue
            vdst_r = vshrq_n_u16(vdst, SK_R16_SHIFT);       // extract red
            vdst_g = vshrq_n_u16(vdst_g, SK_R16_BITS + SK_B16_BITS); // extract green

            // shift src to 565
            vsrc.val[NEON_R] = vshr_n_u8(vsrc.val[NEON_R], 8 - SK_R16_BITS);
            vsrc.val[NEON_G] = vshr_n_u8(vsrc.val[NEON_G], 8 - SK_G16_BITS);
            vsrc.val[NEON_B] = vshr_n_u8(vsrc.val[NEON_B], 8 - SK_B16_BITS);

            // calc src * src_scale
            vres_a = vmull_u8(vsrc.val[NEON_A], valpha);
            vres_r = vmull_u8(vsrc.val[NEON_R], valpha);
            vres_g = vmull_u8(vsrc.val[NEON_G], valpha);
            vres_b = vmull_u8(vsrc.val[NEON_B], valpha);

            // prepare dst_scale
            vres_a = SkDiv255Round_neon8(vres_a);
            vres_a = valpha_max - vres_a; // 255 - (sa * src_scale) / 255

            // add dst * dst_scale to previous result
            vres_r = vmlaq_u16(vres_r, vdst_r, vres_a);
            vres_g = vmlaq_u16(vres_g, vdst_g, vres_a);
            vres_b = vmlaq_u16(vres_b, vdst_b, vres_a);

#ifdef S32A_D565_BLEND_EXACT
            // It is possible to get exact results with this but it is slow,
            // even slower than C code in some cases
            vres_r = SkDiv255Round_neon8(vres_r);
            vres_g = SkDiv255Round_neon8(vres_g);
            vres_b = SkDiv255Round_neon8(vres_b);
#else
            vres_r = vrshrq_n_u16(vres_r, 8);
            vres_g = vrshrq_n_u16(vres_g, 8);
            vres_b = vrshrq_n_u16(vres_b, 8);
#endif
            // pack result
            vres_b = vsliq_n_u16(vres_b, vres_g, SK_G16_SHIFT); // insert green into blue
            vres_b = vsliq_n_u16(vres_b, vres_r, SK_R16_SHIFT); // insert red into green/blue

            // store
            vst1q_u16(dst, vres_b);
            dst += 8;
            count -= 8;
        } while (count >= 8);
    }

    // leftovers
    while (count-- > 0) {
        SkPMColor sc = *src++;
        if (sc) {
            uint16_t dc = *dst;
            unsigned dst_scale = 255 - SkMulDiv255Round(SkGetPackedA32(sc), alpha);
            unsigned dr = (SkPacked32ToR16(sc) * alpha) + (SkGetPackedR16(dc) * dst_scale);
            unsigned dg = (SkPacked32ToG16(sc) * alpha) + (SkGetPackedG16(dc) * dst_scale);
            unsigned db = (SkPacked32ToB16(sc) * alpha) + (SkGetPackedB16(dc) * dst_scale);
            *dst = SkPackRGB16(SkDiv255Round(dr), SkDiv255Round(dg), SkDiv255Round(db));
        }
        dst += 1;
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

    SkASSERT(255 > alpha);

    // rescale alpha to range 1 - 256
    int scale = SkAlpha255To256(alpha);

    if (count >= 8) {
        /* select row and offset for dither array */
        const uint8_t *dstart = &gDitherMatrix_Neon[(y&3)*12 + (x&3)];

        uint8x8_t vdither = vld1_u8(dstart);         // load dither values
        uint8x8_t vdither_g = vshr_n_u8(vdither, 1); // calc. green dither values

        int16x8_t vscale = vdupq_n_s16(scale);        // duplicate scale into neon reg
        uint16x8_t vmask_b = vdupq_n_u16(0x1F);         // set up blue mask

        do {

            uint8x8x4_t vsrc;
            uint8x8_t vsrc_r, vsrc_g, vsrc_b;
            uint8x8_t vsrc565_r, vsrc565_g, vsrc565_b;
            uint16x8_t vsrc_dit_r, vsrc_dit_g, vsrc_dit_b;
            uint16x8_t vsrc_res_r, vsrc_res_g, vsrc_res_b;
            uint16x8_t vdst;
            uint16x8_t vdst_r, vdst_g, vdst_b;
            int16x8_t vres_r, vres_g, vres_b;
            int8x8_t vres8_r, vres8_g, vres8_b;

            // Load source and add dither
#ifdef SK_CPU_ARM64
            vsrc = sk_vld4_u8_arm64_3(src);
#else
            {
            register uint8x8_t d0 asm("d0");
            register uint8x8_t d1 asm("d1");
            register uint8x8_t d2 asm("d2");
            register uint8x8_t d3 asm("d3");

            asm (
                "vld4.8    {d0-d3},[%[src]]! "
                : "=w" (d0), "=w" (d1), "=w" (d2), "=w" (d3), [src] "+&r" (src)
                :
            );
            vsrc.val[0] = d0;
            vsrc.val[1] = d1;
            vsrc.val[2] = d2;
            }
#endif
            vsrc_r = vsrc.val[NEON_R];
            vsrc_g = vsrc.val[NEON_G];
            vsrc_b = vsrc.val[NEON_B];

            vsrc565_g = vshr_n_u8(vsrc_g, 6); // calc. green >> 6
            vsrc565_r = vshr_n_u8(vsrc_r, 5); // calc. red >> 5
            vsrc565_b = vshr_n_u8(vsrc_b, 5); // calc. blue >> 5

            vsrc_dit_g = vaddl_u8(vsrc_g, vdither_g); // add in dither to green and widen
            vsrc_dit_r = vaddl_u8(vsrc_r, vdither);   // add in dither to red and widen
            vsrc_dit_b = vaddl_u8(vsrc_b, vdither);   // add in dither to blue and widen

            vsrc_dit_r = vsubw_u8(vsrc_dit_r, vsrc565_r);  // sub shifted red from result
            vsrc_dit_g = vsubw_u8(vsrc_dit_g, vsrc565_g);  // sub shifted green from result
            vsrc_dit_b = vsubw_u8(vsrc_dit_b, vsrc565_b);  // sub shifted blue from result

            vsrc_res_r = vshrq_n_u16(vsrc_dit_r, 3);
            vsrc_res_g = vshrq_n_u16(vsrc_dit_g, 2);
            vsrc_res_b = vshrq_n_u16(vsrc_dit_b, 3);

            // Load dst and unpack
            vdst = vld1q_u16(dst);
            vdst_g = vshrq_n_u16(vdst, 5);                   // shift down to get green
            vdst_r = vshrq_n_u16(vshlq_n_u16(vdst, 5), 5+5); // double shift to extract red
            vdst_b = vandq_u16(vdst, vmask_b);               // mask to get blue

            // subtract dst from src and widen
            vres_r = vsubq_s16(vreinterpretq_s16_u16(vsrc_res_r), vreinterpretq_s16_u16(vdst_r));
            vres_g = vsubq_s16(vreinterpretq_s16_u16(vsrc_res_g), vreinterpretq_s16_u16(vdst_g));
            vres_b = vsubq_s16(vreinterpretq_s16_u16(vsrc_res_b), vreinterpretq_s16_u16(vdst_b));

            // multiply diffs by scale and shift
            vres_r = vmulq_s16(vres_r, vscale);
            vres_g = vmulq_s16(vres_g, vscale);
            vres_b = vmulq_s16(vres_b, vscale);

            vres8_r = vshrn_n_s16(vres_r, 8);
            vres8_g = vshrn_n_s16(vres_g, 8);
            vres8_b = vshrn_n_s16(vres_b, 8);

            // add dst to result
            vres_r = vaddw_s8(vreinterpretq_s16_u16(vdst_r), vres8_r);
            vres_g = vaddw_s8(vreinterpretq_s16_u16(vdst_g), vres8_g);
            vres_b = vaddw_s8(vreinterpretq_s16_u16(vdst_b), vres8_b);

            // put result into 565 format
            vres_b = vsliq_n_s16(vres_b, vres_g, 5);   // shift up green and insert into blue
            vres_b = vsliq_n_s16(vres_b, vres_r, 6+5); // shift up red and insert into blue

            // Store result
            vst1q_u16(dst, vreinterpretq_u16_s16(vres_b));

            // Next iteration
            dst += 8;
            count -= 8;

        } while (count >= 8);
    }

    // Leftovers
    if (count > 0) {
        int scale = SkAlpha255To256(alpha);
        DITHER_565_SCAN(y);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);

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
        } while (--count != 0);
    }
}

/* Neon version of S32_Blend_BlitRow32()
 * portable version is in src/core/SkBlitRow_D32.cpp
 */
void S32_Blend_BlitRow32_neon(SkPMColor* SK_RESTRICT dst,
                              const SkPMColor* SK_RESTRICT src,
                              int count, U8CPU alpha) {
    SkASSERT(alpha <= 255);

    if (count <= 0) {
        return;
    }

    uint16_t src_scale = SkAlpha255To256(alpha);
    uint16_t dst_scale = 256 - src_scale;

    while (count >= 2) {
        uint8x8_t vsrc, vdst, vres;
        uint16x8_t vsrc_wide, vdst_wide;

        /* These commented prefetches are a big win for count
         * values > 64 on an A9 (Pandaboard) but hurt by 10% for count = 4.
         * They also hurt a little (<5%) on an A15
         */
        //__builtin_prefetch(src+32);
        //__builtin_prefetch(dst+32);

        // Load
        vsrc = vreinterpret_u8_u32(vld1_u32(src));
        vdst = vreinterpret_u8_u32(vld1_u32(dst));

        // Process src
        vsrc_wide = vmovl_u8(vsrc);
        vsrc_wide = vmulq_u16(vsrc_wide, vdupq_n_u16(src_scale));

        // Process dst
        vdst_wide = vmull_u8(vdst, vdup_n_u8(dst_scale));

        // Combine
#ifdef SK_SUPPORT_LEGACY_BROKEN_LERP
        vres = vshrn_n_u16(vdst_wide, 8) + vshrn_n_u16(vsrc_wide, 8);
#else
        vdst_wide += vsrc_wide;
        vres = vshrn_n_u16(vdst_wide, 8);
#endif

        // Store
        vst1_u32(dst, vreinterpret_u32_u8(vres));

        src += 2;
        dst += 2;
        count -= 2;
    }

    if (count == 1) {
        uint8x8_t vsrc = vdup_n_u8(0), vdst = vdup_n_u8(0), vres;
        uint16x8_t vsrc_wide, vdst_wide;

        // Load
        vsrc = vreinterpret_u8_u32(vld1_lane_u32(src, vreinterpret_u32_u8(vsrc), 0));
        vdst = vreinterpret_u8_u32(vld1_lane_u32(dst, vreinterpret_u32_u8(vdst), 0));

        // Process
        vsrc_wide = vmovl_u8(vsrc);
        vsrc_wide = vmulq_u16(vsrc_wide, vdupq_n_u16(src_scale));
        vdst_wide = vmull_u8(vdst, vdup_n_u8(dst_scale));
#ifdef SK_SUPPORT_LEGACY_BROKEN_LERP
        vres = vshrn_n_u16(vdst_wide, 8) + vshrn_n_u16(vsrc_wide, 8);
#else
        vdst_wide += vsrc_wide;
        vres = vshrn_n_u16(vdst_wide, 8);
#endif

        // Store
        vst1_lane_u32(dst, vreinterpret_u32_u8(vres), 0);
    }
}

#ifdef SK_CPU_ARM32
void S32A_Blend_BlitRow32_neon(SkPMColor* SK_RESTRICT dst,
                         const SkPMColor* SK_RESTRICT src,
                         int count, U8CPU alpha) {

    SkASSERT(255 > alpha);

    if (count <= 0) {
        return;
    }

    unsigned alpha256 = SkAlpha255To256(alpha);

    // First deal with odd counts
    if (count & 1) {
        uint8x8_t vsrc = vdup_n_u8(0), vdst = vdup_n_u8(0), vres;
        uint16x8_t vdst_wide, vsrc_wide;
        unsigned dst_scale;

        // Load
        vsrc = vreinterpret_u8_u32(vld1_lane_u32(src, vreinterpret_u32_u8(vsrc), 0));
        vdst = vreinterpret_u8_u32(vld1_lane_u32(dst, vreinterpret_u32_u8(vdst), 0));

        // Calc dst_scale
        dst_scale = vget_lane_u8(vsrc, 3);
        dst_scale = SkAlphaMulInv256(dst_scale, alpha256);

        // Process src
        vsrc_wide = vmovl_u8(vsrc);
        vsrc_wide = vmulq_n_u16(vsrc_wide, alpha256);

        // Process dst
        vdst_wide = vmovl_u8(vdst);
        vdst_wide = vmulq_n_u16(vdst_wide, dst_scale);

        // Combine
#ifdef SK_SUPPORT_LEGACY_BROKEN_LERP
        vres = vshrn_n_u16(vdst_wide, 8) + vshrn_n_u16(vsrc_wide, 8);
#else
        vdst_wide += vsrc_wide;
        vres = vshrn_n_u16(vdst_wide, 8);
#endif

        vst1_lane_u32(dst, vreinterpret_u32_u8(vres), 0);
        dst++;
        src++;
        count--;
    }

    if (count) {
        uint8x8_t alpha_mask;
        static const uint8_t alpha_mask_setup[] = {3,3,3,3,7,7,7,7};
        alpha_mask = vld1_u8(alpha_mask_setup);

        do {

            uint8x8_t vsrc, vdst, vres, vsrc_alphas;
            uint16x8_t vdst_wide, vsrc_wide, vsrc_scale, vdst_scale;

            __builtin_prefetch(src+32);
            __builtin_prefetch(dst+32);

            // Load
            vsrc = vreinterpret_u8_u32(vld1_u32(src));
            vdst = vreinterpret_u8_u32(vld1_u32(dst));

            // Prepare src_scale
            vsrc_scale = vdupq_n_u16(alpha256);

            // Calc dst_scale
            vsrc_alphas = vtbl1_u8(vsrc, alpha_mask);
            vdst_scale = vmovl_u8(vsrc_alphas);
#ifdef SK_SUPPORT_LEGACY_BROKEN_LERP
            vdst_scale *= vsrc_scale;
            vdst_scale = vshrq_n_u16(vdst_scale, 8);
            vdst_scale = vsubq_u16(vdupq_n_u16(256), vdst_scale);
#else
            // Calculate SkAlphaMulInv256(vdst_scale, vsrc_scale).
            // A 16-bit lane would overflow if we used 0xFFFF here,
            // so use an approximation with 0xFF00 that is off by 1,
            // and add back 1 after to get the correct value.
            // This is valid if alpha256 <= 255.
            vdst_scale = vmlsq_u16(vdupq_n_u16(0xFF00), vdst_scale, vsrc_scale);
            vdst_scale = vsraq_n_u16(vdst_scale, vdst_scale, 8);
            vdst_scale = vsraq_n_u16(vdupq_n_u16(1), vdst_scale, 8);
#endif

            // Process src
            vsrc_wide = vmovl_u8(vsrc);
            vsrc_wide *= vsrc_scale;

            // Process dst
            vdst_wide = vmovl_u8(vdst);
            vdst_wide *= vdst_scale;

            // Combine
#ifdef SK_SUPPORT_LEGACY_BROKEN_LERP
            vres = vshrn_n_u16(vdst_wide, 8) + vshrn_n_u16(vsrc_wide, 8);
#else
            vdst_wide += vsrc_wide;
            vres = vshrn_n_u16(vdst_wide, 8);
#endif

            vst1_u32(dst, vreinterpret_u32_u8(vres));

            src += 2;
            dst += 2;
            count -= 2;
        } while(count);
    }
}

///////////////////////////////////////////////////////////////////////////////

#endif // #ifdef SK_CPU_ARM32

void S32A_D565_Opaque_Dither_neon (uint16_t * SK_RESTRICT dst,
                                   const SkPMColor* SK_RESTRICT src,
                                   int count, U8CPU alpha, int x, int y) {
    SkASSERT(255 == alpha);

#define    UNROLL    8

    if (count >= UNROLL) {

    uint8x8_t dbase;
    const uint8_t *dstart = &gDitherMatrix_Neon[(y&3)*12 + (x&3)];
    dbase = vld1_u8(dstart);

        do {
        uint8x8x4_t vsrc;
        uint8x8_t sr, sg, sb, sa, d;
        uint16x8_t dst8, scale8, alpha8;
        uint16x8_t dst_r, dst_g, dst_b;

#ifdef SK_CPU_ARM64
        vsrc = sk_vld4_u8_arm64_4(src);
#else
        {
        register uint8x8_t d0 asm("d0");
        register uint8x8_t d1 asm("d1");
        register uint8x8_t d2 asm("d2");
        register uint8x8_t d3 asm("d3");

        asm ("vld4.8    {d0-d3},[%[src]]! "
            : "=w" (d0), "=w" (d1), "=w" (d2), "=w" (d3), [src] "+r" (src)
            :
        );
        vsrc.val[0] = d0;
        vsrc.val[1] = d1;
        vsrc.val[2] = d2;
        vsrc.val[3] = d3;
        }
#endif
        sa = vsrc.val[NEON_A];
        sr = vsrc.val[NEON_R];
        sg = vsrc.val[NEON_G];
        sb = vsrc.val[NEON_B];

        /* calculate 'd', which will be 0..7
         * dbase[] is 0..7; alpha is 0..256; 16 bits suffice
         */
        alpha8 = vmovl_u8(dbase);
        alpha8 = vmlal_u8(alpha8, sa, dbase);
        d = vshrn_n_u16(alpha8, 8);    // narrowing too

        // sr = sr - (sr>>5) + d
        /* watching for 8-bit overflow.  d is 0..7; risky range of
         * sr is >248; and then (sr>>5) is 7 so it offsets 'd';
         * safe  as long as we do ((sr-sr>>5) + d)
         */
        sr = vsub_u8(sr, vshr_n_u8(sr, 5));
        sr = vadd_u8(sr, d);

        // sb = sb - (sb>>5) + d
        sb = vsub_u8(sb, vshr_n_u8(sb, 5));
        sb = vadd_u8(sb, d);

        // sg = sg - (sg>>6) + d>>1; similar logic for overflows
        sg = vsub_u8(sg, vshr_n_u8(sg, 6));
        sg = vadd_u8(sg, vshr_n_u8(d,1));

        // need to pick up 8 dst's -- at 16 bits each, 128 bits
        dst8 = vld1q_u16(dst);
        dst_b = vandq_u16(dst8, vdupq_n_u16(SK_B16_MASK));
        dst_g = vshrq_n_u16(vshlq_n_u16(dst8, SK_R16_BITS), SK_R16_BITS + SK_B16_BITS);
        dst_r = vshrq_n_u16(dst8, SK_R16_SHIFT);    // clearing hi bits

        // blend
        scale8 = vsubw_u8(vdupq_n_u16(256), sa);

        // combine the addq and mul, save 3 insns
        scale8 = vshrq_n_u16(scale8, 3);
        dst_b = vmlaq_u16(vshll_n_u8(sb,2), dst_b, scale8);
        dst_g = vmlaq_u16(vshll_n_u8(sg,3), dst_g, scale8);
        dst_r = vmlaq_u16(vshll_n_u8(sr,2), dst_r, scale8);

        // repack to store
        dst8 = vshrq_n_u16(dst_b, 5);
        dst8 = vsliq_n_u16(dst8, vshrq_n_u16(dst_g, 5), 5);
        dst8 = vsliq_n_u16(dst8, vshrq_n_u16(dst_r,5), 11);

        vst1q_u16(dst, dst8);

        dst += UNROLL;
        count -= UNROLL;
        // skip x += UNROLL, since it's unchanged mod-4
        } while (count >= UNROLL);
    }
#undef    UNROLL

    // residuals
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
        uint8x8_t sr, sg, sb;
        uint16x8_t dr, dg, db;
        uint16x8_t dst8;
        uint8x8x4_t vsrc;

#ifdef SK_CPU_ARM64
        vsrc = sk_vld4_u8_arm64_3(src);
#else
        {
        register uint8x8_t d0 asm("d0");
        register uint8x8_t d1 asm("d1");
        register uint8x8_t d2 asm("d2");
        register uint8x8_t d3 asm("d3");

        asm (
            "vld4.8    {d0-d3},[%[src]]! "
            : "=w" (d0), "=w" (d1), "=w" (d2), "=w" (d3), [src] "+&r" (src)
            :
        );
        vsrc.val[0] = d0;
        vsrc.val[1] = d1;
        vsrc.val[2] = d2;
        }
#endif
        sr = vsrc.val[NEON_R];
        sg = vsrc.val[NEON_G];
        sb = vsrc.val[NEON_B];

        /* XXX: if we want to prefetch, hide it in the above asm()
         * using the gcc __builtin_prefetch(), the prefetch will
         * fall to the bottom of the loop -- it won't stick up
         * at the top of the loop, just after the vld4.
         */

        // sr = sr - (sr>>5) + d
        sr = vsub_u8(sr, vshr_n_u8(sr, 5));
        dr = vaddl_u8(sr, d);

        // sb = sb - (sb>>5) + d
        sb = vsub_u8(sb, vshr_n_u8(sb, 5));
        db = vaddl_u8(sb, d);

        // sg = sg - (sg>>6) + d>>1; similar logic for overflows
        sg = vsub_u8(sg, vshr_n_u8(sg, 6));
        dg = vaddl_u8(sg, vshr_n_u8(d, 1));

        // pack high bits of each into 565 format  (rgb, b is lsb)
        dst8 = vshrq_n_u16(db, 3);
        dst8 = vsliq_n_u16(dst8, vshrq_n_u16(dg, 2), 5);
        dst8 = vsliq_n_u16(dst8, vshrq_n_u16(dr, 3), 11);

        // store it
        vst1q_u16(dst, dst8);

        dst += UNROLL;
        // we don't need to increment src as the asm above has already done it
        count -= UNROLL;
        x += UNROLL;        // probably superfluous
    }
    }
#undef    UNROLL

    // residuals
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

///////////////////////////////////////////////////////////////////////////////

const SkBlitRow::Proc16 sk_blitrow_platform_565_procs_arm_neon[] = {
    // no dither
    S32_D565_Opaque_neon,
    S32_D565_Blend_neon,
    S32A_D565_Opaque_neon,
#if 0
    S32A_D565_Blend_neon,
#else
    nullptr,   // https://code.google.com/p/skia/issues/detail?id=2797
#endif

    // dither
    S32_D565_Opaque_Dither_neon,
    S32_D565_Blend_Dither_neon,
    S32A_D565_Opaque_Dither_neon,
    nullptr,   // S32A_D565_Blend_Dither
};

const SkBlitRow::ColorProc16 sk_blitrow_platform_565_colorprocs_arm_neon[] = {
    Color32A_D565_neon,    // Color32_D565,
    Color32A_D565_neon,    // Color32A_D565,
    Color32A_D565_neon,    // Color32_D565_Dither,
    Color32A_D565_neon,    // Color32A_D565_Dither
};

const SkBlitRow::Proc32 sk_blitrow_platform_32_procs_arm_neon[] = {
    nullptr,   // S32_Opaque,
    S32_Blend_BlitRow32_neon,        // S32_Blend,
    nullptr,  // Ported to SkOpts
#ifdef SK_CPU_ARM32
    S32A_Blend_BlitRow32_neon        // S32A_Blend
#else
    nullptr
#endif
};
