/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitRow.h"
#include "SkBlitMask.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkMathPriv.h"

static void S32_D565_Blend_mips_dsp(uint16_t* SK_RESTRICT dst,
                                    const SkPMColor* SK_RESTRICT src, int count,
                                    U8CPU alpha, int /*x*/, int /*y*/) {
    register uint32_t t0, t1, t2, t3, t4, t5, t6;
    register uint32_t s0, s1, s2, s4, s5, s6;

    alpha += 1;
    if (count >= 2) {
        __asm__ volatile (
           ".set             push                          \n\t"
           ".set             noreorder                     \n\t"
            "sll             %[s4],    %[alpha], 8         \n\t"
            "or              %[s4],    %[s4],    %[alpha]  \n\t"
            "repl.ph         %[s5],    0x1f                \n\t"
            "repl.ph         %[s6],    0x3f                \n\t"
        "1:                                                \n\t"
            "lw              %[s2],    0(%[src])           \n\t"
            "lw              %[s1],    4(%[src])           \n\t"
            "lwr             %[s0],    0(%[dst])           \n\t"
            "lwl             %[s0],    3(%[dst])           \n\t"
            "and             %[t1],    %[s0],    %[s5]     \n\t"
            "shra.ph         %[t0],    %[s0],    5         \n\t"
            "and             %[t2],    %[t0],    %[s6]     \n\t"
#ifdef __mips_dspr2
            "shrl.ph         %[t3],    %[s0],    11        \n\t"
#else
            "shra.ph         %[t0],    %[s0],    11        \n\t"
            "and             %[t3],    %[t0],    %[s5]     \n\t"
#endif
            "precrq.ph.w     %[t0],    %[s1],    %[s2]     \n\t"
            "shrl.qb         %[t5],    %[t0],    3         \n\t"
            "and             %[t4],    %[t5],    %[s5]     \n\t"
            "ins             %[s2],    %[s1],    16, 16    \n\t"
            "preceu.ph.qbra  %[t0],    %[s2]               \n\t"
            "shrl.qb         %[t6],    %[t0],    3         \n\t"
#ifdef __mips_dspr2
            "shrl.ph         %[t5],    %[s2],    10        \n\t"
#else
            "shra.ph         %[t0],    %[s2],    10        \n\t"
            "and             %[t5],    %[t0],    %[s6]     \n\t"
#endif
            "subu.qb         %[t4],    %[t4],    %[t1]     \n\t"
            "subu.qb         %[t5],    %[t5],    %[t2]     \n\t"
            "subu.qb         %[t6],    %[t6],    %[t3]     \n\t"
            "muleu_s.ph.qbr  %[t4],    %[s4],    %[t4]     \n\t"
            "muleu_s.ph.qbr  %[t5],    %[s4],    %[t5]     \n\t"
            "muleu_s.ph.qbr  %[t6],    %[s4],    %[t6]     \n\t"
            "addiu           %[count], %[count], -2        \n\t"
            "addiu           %[src],   %[src],   8         \n\t"
            "shra.ph         %[t4],    %[t4],    8         \n\t"
            "shra.ph         %[t5],    %[t5],    8         \n\t"
            "shra.ph         %[t6],    %[t6],    8         \n\t"
            "addu.qb         %[t4],    %[t4],    %[t1]     \n\t"
            "addu.qb         %[t5],    %[t5],    %[t2]     \n\t"
            "addu.qb         %[t6],    %[t6],    %[t3]     \n\t"
            "andi            %[s0],    %[t4],    0xffff    \n\t"
            "andi            %[t0],    %[t5],    0xffff    \n\t"
            "sll             %[t0],    %[t0],    0x5       \n\t"
            "or              %[s0],    %[s0],    %[t0]     \n\t"
            "sll             %[t0],    %[t6],    0xb       \n\t"
            "or              %[t0],    %[t0],    %[s0]     \n\t"
            "sh              %[t0],    0(%[dst])           \n\t"
            "srl             %[s1],    %[t4],    16        \n\t"
            "srl             %[t0],    %[t5],    16        \n\t"
            "sll             %[t5],    %[t0],    5         \n\t"
            "or              %[t0],    %[t5],    %[s1]     \n\t"
            "srl             %[s0],    %[t6],    16        \n\t"
            "sll             %[s2],    %[s0],    0xb       \n\t"
            "or              %[s1],    %[s2],    %[t0]     \n\t"
            "sh              %[s1],    2(%[dst])           \n\t"
            "bge             %[count], 2,        1b        \n\t"
            " addiu          %[dst],   %[dst],   4         \n\t"
            ".set            pop                           \n\t"
            : [t0]"=&r"(t0), [t1]"=&r"(t1), [t2]"=&r"(t2), [t3]"=&r"(t3),
              [t4]"=&r"(t4), [t5]"=&r"(t5), [t6]"=&r"(t6), [s0]"=&r"(s0),
              [s1]"=&r"(s1), [s2]"=&r"(s2), [s4]"=&r"(s4), [s5]"=&r"(s5),
              [s6]"=&r"(s6), [count]"+r"(count), [dst]"+r"(dst),
              [src]"+r"(src)
            : [alpha]"r"(alpha)
            : "memory", "hi", "lo"
        );
    }

    if (count == 1) {
        SkPMColor c = *src++;
        SkPMColorAssert(c);
        SkASSERT(SkGetPackedA32(c) == 255);
        uint16_t d = *dst;
        *dst++ = SkPackRGB16(SkAlphaBlend(SkPacked32ToR16(c), SkGetPackedR16(d), alpha),
                             SkAlphaBlend(SkPacked32ToG16(c), SkGetPackedG16(d), alpha),
                             SkAlphaBlend(SkPacked32ToB16(c), SkGetPackedB16(d), alpha));
    }
}

static void S32A_D565_Opaque_Dither_mips_dsp(uint16_t* __restrict__ dst,
                                             const SkPMColor* __restrict__ src,
                                             int count, U8CPU alpha, int x, int y) {
    __asm__ volatile (
        "pref  0,   0(%[src])     \n\t"
        "pref  1,   0(%[dst])     \n\t"
        "pref  0,   32(%[src])    \n\t"
        "pref  1,   32(%[dst])    \n\t"
        :
        : [src]"r"(src), [dst]"r"(dst)
        : "memory"
    );

    register int32_t t0, t1, t2, t3, t4, t5, t6;
    register int32_t t7, t8, t9, s0, s1, s2, s3;
    const uint16_t dither_scan = gDitherMatrix_3Bit_16[(y) & 3];

    if (count >= 2) {
        __asm__ volatile (
            ".set            push                                \n\t"
            ".set            noreorder                           \n\t"
            "li              %[s1],    0x01010101                \n\t"
            "li              %[s2],    -2017                     \n\t"
        "1:                                                      \n\t"
            "bnez            %[s3],    4f                        \n\t"
            " li             %[s3],    2                         \n\t"
            "pref            0,        64(%[src])                \n\t"
            "pref            1,        64(%[dst])                \n\t"
        "4:                                                      \n\t"
            "addiu           %[s3],    %[s3],    -1              \n\t"
            "lw              %[t1],    0(%[src])                 \n\t"
            "andi            %[t3],    %[x],     0x3             \n\t"
            "addiu           %[x],     %[x],     1               \n\t"
            "sll             %[t4],    %[t3],    2               \n\t"
            "srav            %[t5],    %[dither_scan], %[t4]     \n\t"
            "andi            %[t3],    %[t5],    0xf             \n\t"
            "lw              %[t2],    4(%[src])                 \n\t"
            "andi            %[t4],    %[x],     0x3             \n\t"
            "sll             %[t5],    %[t4],    2               \n\t"
            "srav            %[t6],    %[dither_scan], %[t5]     \n\t"
            "addiu           %[x],     %[x],     1               \n\t"
            "ins             %[t3],    %[t6],    8,    4         \n\t"
            "srl             %[t4],    %[t1],    24              \n\t"
            "addiu           %[t0],    %[t4],    1               \n\t"
            "srl             %[t4],    %[t2],    24              \n\t"
            "addiu           %[t5],    %[t4],    1               \n\t"
            "ins             %[t0],    %[t5],    16,   16        \n\t"
            "muleu_s.ph.qbr  %[t4],    %[t3],    %[t0]           \n\t"
            "preceu.ph.qbla  %[t3],    %[t4]                     \n\t"
            "andi            %[t4],    %[t1],    0xff            \n\t"
            "ins             %[t4],    %[t2],    16,   8         \n\t"
            "shrl.qb         %[t5],    %[t4],    5               \n\t"
            "subu.qb         %[t6],    %[t3],    %[t5]           \n\t"
            "addq.ph         %[t5],    %[t6],    %[t4]           \n\t"
            "ext             %[t4],    %[t1],    8,    8         \n\t"
            "srl             %[t6],    %[t2],    8               \n\t"
            "ins             %[t4],    %[t6],    16,   8         \n\t"
            "shrl.qb         %[t6],    %[t4],    6               \n\t"
            "shrl.qb         %[t7],    %[t3],    1               \n\t"
            "subu.qb         %[t8],    %[t7],    %[t6]           \n\t"
            "addq.ph         %[t6],    %[t8],    %[t4]           \n\t"
            "ext             %[t4],    %[t1],    16,   8         \n\t"
            "srl             %[t7],    %[t2],    16              \n\t"
            "ins             %[t4],    %[t7],    16,   8         \n\t"
            "shrl.qb         %[t7],    %[t4],    5               \n\t"
            "subu.qb         %[t8],    %[t3],    %[t7]           \n\t"
            "addq.ph         %[t7],    %[t8],    %[t4]           \n\t"
            "shll.ph         %[t4],    %[t7],    2               \n\t"
            "andi            %[t9],    %[t4],    0xffff          \n\t"
            "srl             %[s0],    %[t4],    16              \n\t"
            "andi            %[t3],    %[t6],    0xffff          \n\t"
            "srl             %[t4],    %[t6],    16              \n\t"
            "andi            %[t6],    %[t5],    0xffff          \n\t"
            "srl             %[t7],    %[t5],    16              \n\t"
            "subq.ph         %[t5],    %[s1],    %[t0]           \n\t"
            "srl             %[t0],    %[t5],    3               \n\t"
            "beqz            %[t1],    3f                        \n\t"
            " lhu            %[t5],    0(%[dst])                 \n\t"
            "sll             %[t1],    %[t6],    13              \n\t"
            "or              %[t8],    %[t9],    %[t1]           \n\t"
            "sll             %[t1],    %[t3],    24              \n\t"
            "or              %[t9],    %[t1],    %[t8]           \n\t"
            "andi            %[t3],    %[t5],    0x7e0           \n\t"
            "sll             %[t6],    %[t3],    0x10            \n\t"
            "and             %[t8],    %[s2],    %[t5]           \n\t"
            "or              %[t5],    %[t6],    %[t8]           \n\t"
            "andi            %[t6],    %[t0],    0xff            \n\t"
            "mul             %[t1],    %[t6],    %[t5]           \n\t"
            "addu            %[t5],    %[t1],    %[t9]           \n\t"
            "srl             %[t6],    %[t5],    5               \n\t"
            "and             %[t5],    %[s2],    %[t6]           \n\t"
            "srl             %[t8],    %[t6],    16              \n\t"
            "andi            %[t6],    %[t8],    0x7e0           \n\t"
            "or              %[t1],    %[t5],    %[t6]           \n\t"
            "sh              %[t1],    0(%[dst])                 \n\t"
        "3:                                                      \n\t"
            "beqz            %[t2],    2f                        \n\t"
            " lhu            %[t5],    2(%[dst])                 \n\t"
            "sll             %[t1],    %[t7],    13              \n\t"
            "or              %[t8],    %[s0],    %[t1]           \n\t"
            "sll             %[t1],    %[t4],    24              \n\t"
            "or              %[t9],    %[t1],    %[t8]           \n\t"
            "andi            %[t3],    %[t5],    0x7e0           \n\t"
            "sll             %[t6],    %[t3],    0x10            \n\t"
            "and             %[t8],    %[s2],    %[t5]           \n\t"
            "or              %[t5],    %[t6],    %[t8]           \n\t"
            "srl             %[t6],    %[t0],    16              \n\t"
            "mul             %[t1],    %[t6],    %[t5]           \n\t"
            "addu            %[t5],    %[t1],    %[t9]           \n\t"
            "srl             %[t6],    %[t5],    5               \n\t"
            "and             %[t5],    %[s2],    %[t6]           \n\t"
            "srl             %[t8],    %[t6],    16              \n\t"
            "andi            %[t6],    %[t8],    0x7e0           \n\t"
            "or              %[t1],    %[t5],    %[t6]           \n\t"
            "sh              %[t1],    2(%[dst])                 \n\t"
        "2:                                                      \n\t"
            "addiu           %[count], %[count], -2              \n\t"
            "addiu           %[src],   %[src],   8               \n\t"
            "addiu           %[t1],    %[count], -1              \n\t"
            "bgtz            %[t1],    1b                        \n\t"
            " addiu          %[dst],  %[dst],    4               \n\t"
            ".set            pop                                 \n\t"
            : [src]"+r"(src), [count]"+r"(count), [dst]"+r"(dst), [x]"+r"(x),
              [t0]"=&r"(t0), [t1]"=&r"(t1), [t2]"=&r"(t2), [t3]"=&r"(t3),
              [t4]"=&r"(t4), [t5]"=&r"(t5), [t6]"=&r"(t6), [t7]"=&r"(t7),
              [t8]"=&r"(t8),  [t9]"=&r"(t9), [s0]"=&r"(s0), [s1]"=&r"(s1),
              [s2]"=&r"(s2), [s3]"=&r"(s3)
            : [dither_scan]"r"(dither_scan)
            : "memory", "hi", "lo"
        );
    }

    if (count == 1) {
        SkPMColor c = *src++;
        SkPMColorAssert(c);
        if (c) {
            unsigned a = SkGetPackedA32(c);
            int d = SkAlphaMul(DITHER_VALUE(x), SkAlpha255To256(a));

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
    }
}

static void S32_D565_Opaque_Dither_mips_dsp(uint16_t* __restrict__ dst,
                                            const SkPMColor* __restrict__ src,
                                            int count, U8CPU alpha, int x, int y) {
    uint16_t dither_scan = gDitherMatrix_3Bit_16[(y) & 3];
    register uint32_t t0, t1, t2, t3, t4, t5;
    register uint32_t t6, t7, t8, t9, s0;
    int dither[4];
    int i;

    for (i = 0; i < 4; i++, x++) {
        dither[i] = (dither_scan >> ((x & 3) << 2)) & 0xF;
    }

    __asm__ volatile (
        ".set            push                          \n\t"
        ".set            noreorder                     \n\t"
        "li              %[s0],    1                   \n\t"
    "2:                                                \n\t"
        "beqz            %[count], 1f                  \n\t"
        " nop                                          \n\t"
        "addiu           %[t0],    %[count], -1        \n\t"
        "beqz            %[t0],    1f                  \n\t"
        " nop                                          \n\t"
        "beqz            %[s0],    3f                  \n\t"
        " nop                                          \n\t"
        "lw              %[t0],    0(%[dither])        \n\t"
        "lw              %[t1],    4(%[dither])        \n\t"
        "li              %[s0],    0                   \n\t"
        "b               4f                            \n\t"
        " nop                                          \n\t"
    "3:                                                \n\t"
        "lw              %[t0],    8(%[dither])        \n\t"
        "lw              %[t1],    12(%[dither])       \n\t"
        "li              %[s0],    1                   \n\t"
    "4:                                                \n\t"
        "sll             %[t2],    %[t0],    16        \n\t"
        "or              %[t1],    %[t2],    %[t1]     \n\t"
        "lw              %[t0],    0(%[src])           \n\t"
        "lw              %[t2],    4(%[src])           \n\t"
        "precrq.ph.w     %[t3],    %[t0],    %[t2]     \n\t"
        "preceu.ph.qbra  %[t9],    %[t3]               \n\t"
#ifdef __mips_dspr2
        "append          %[t0],    %[t2],    16        \n\t"
        "preceu.ph.qbra  %[t4],    %[t0]               \n\t"
        "preceu.ph.qbla  %[t5],    %[t0]               \n\t"
#else
        "sll             %[t6],    %[t0],    16        \n\t"
        "sll             %[t7],    %[t2],    16        \n\t"
        "precrq.ph.w     %[t8],    %[t6],    %[t7]     \n\t"
        "preceu.ph.qbra  %[t4],    %[t8]               \n\t"
        "preceu.ph.qbla  %[t5],    %[t8]               \n\t"
#endif
        "addu.qb         %[t0],    %[t4],    %[t1]     \n\t"
        "shra.ph         %[t2],    %[t4],    5         \n\t"
        "subu.qb         %[t3],    %[t0],    %[t2]     \n\t"
        "shra.ph         %[t6],    %[t3],    3         \n\t"
        "addu.qb         %[t0],    %[t9],    %[t1]     \n\t"
        "shra.ph         %[t2],    %[t9],    5         \n\t"
        "subu.qb         %[t3],    %[t0],    %[t2]     \n\t"
        "shra.ph         %[t7],    %[t3],    3         \n\t"
        "shra.ph         %[t0],    %[t1],    1         \n\t"
        "shra.ph         %[t2],    %[t5],    6         \n\t"
        "addu.qb         %[t3],    %[t5],    %[t0]     \n\t"
        "subu.qb         %[t4],    %[t3],    %[t2]     \n\t"
        "shra.ph         %[t8],    %[t4],    2         \n\t"
        "precrq.ph.w     %[t0],    %[t6],    %[t7]     \n\t"
#ifdef __mips_dspr2
        "append          %[t6],    %[t7],    16        \n\t"
#else
        "sll             %[t6],    %[t6],    16        \n\t"
        "sll             %[t2],    %[t7],    16        \n\t"
        "precrq.ph.w     %[t6],    %[t6],    %[t2]     \n\t"
#endif
        "sra             %[t4],    %[t8],    16        \n\t"
        "andi            %[t5],    %[t8],    0xFF      \n\t"
        "sll             %[t7],    %[t4],    5         \n\t"
        "sra             %[t8],    %[t0],    5         \n\t"
        "or              %[t9],    %[t7],    %[t8]     \n\t"
        "or              %[t3],    %[t9],    %[t0]     \n\t"
        "andi            %[t4],    %[t3],    0xFFFF    \n\t"
        "sll             %[t7],    %[t5],    5         \n\t"
        "sra             %[t8],    %[t6],    5         \n\t"
        "or              %[t9],    %[t7],    %[t8]     \n\t"
        "or              %[t3],    %[t9],    %[t6]     \n\t"
        "and             %[t7],    %[t3],    0xFFFF    \n\t"
        "sh              %[t4],    0(%[dst])           \n\t"
        "sh              %[t7],    2(%[dst])           \n\t"
        "addiu           %[count], %[count], -2        \n\t"
        "addiu           %[src],   %[src],   8         \n\t"
        "b               2b                            \n\t"
        " addiu          %[dst],   %[dst],   4         \n\t"
    "1:                                                \n\t"
        ".set            pop                           \n\t"
        : [dst]"+r"(dst), [src]"+r"(src), [count]"+r"(count),
          [x]"+r"(x), [t0]"=&r"(t0), [t1]"=&r"(t1), [t2]"=&r"(t2),
          [t3]"=&r"(t3), [t4]"=&r"(t4), [t5]"=&r"(t5), [t6]"=&r"(t6),
          [t7]"=&r"(t7), [t8]"=&r"(t8), [t9]"=&r"(t9), [s0]"=&r"(s0)
        : [dither] "r" (dither)
        : "memory"
    );

    if (count == 1) {
        SkPMColor c = *src++;
        SkPMColorAssert(c); // only if DEBUG is turned on
        SkASSERT(SkGetPackedA32(c) == 255);
        unsigned dither = DITHER_VALUE(x);
        *dst++ = SkDitherRGB32To565(c, dither);
    }
}

static void S32_D565_Blend_Dither_mips_dsp(uint16_t* dst,
                                           const SkPMColor* src,
                                           int count, U8CPU alpha, int x, int y) {
    register int32_t t0, t1, t2, t3, t4, t5, t6;
    register int32_t s0, s1, s2, s3;
    register int x1 = 0;
    register uint32_t sc_mul;
    register uint32_t sc_add;
#ifdef ENABLE_DITHER_MATRIX_4X4
    const uint8_t* dither_scan = gDitherMatrix_3Bit_4X4[(y) & 3];
#else // ENABLE_DITHER_MATRIX_4X4
    const uint16_t dither_scan = gDitherMatrix_3Bit_16[(y) & 3];
#endif // ENABLE_DITHER_MATRIX_4X4
    int dither[4];

    for (int i = 0; i < 4; i++) {
        dither[i] = (dither_scan >> ((x & 3) << 2)) & 0xF;
        x += 1;
    }
    alpha += 1;
    __asm__ volatile (
        ".set            push                              \n\t"
        ".set            noreorder                         \n\t"
        "li              %[t0],     0x100                  \n\t"
        "subu            %[t0],     %[t0],     %[alpha]    \n\t"
        "replv.ph        %[sc_mul], %[alpha]               \n\t"
        "beqz            %[alpha],  1f                     \n\t"
        " nop                                              \n\t"
        "replv.qb        %[sc_add], %[t0]                  \n\t"
        "b               2f                                \n\t"
        " nop                                              \n\t"
    "1:                                                    \n\t"
        "replv.qb        %[sc_add], %[alpha]               \n\t"
    "2:                                                    \n\t"
        "addiu           %[t2],     %[count],  -1          \n\t"
        "blez            %[t2],     3f                     \n\t"
        " nop                                              \n\t"
        "lw              %[s0],     0(%[src])              \n\t"
        "lw              %[s1],     4(%[src])              \n\t"
        "bnez            %[x1],     4f                     \n\t"
        " nop                                              \n\t"
        "lw              %[t0],     0(%[dither])           \n\t"
        "lw              %[t1],     4(%[dither])           \n\t"
        "li              %[x1],     1                      \n\t"
        "b               5f                                \n\t"
        " nop                                              \n\t"
    "4:                                                    \n\t"
        "lw              %[t0],     8(%[dither])           \n\t"
        "lw              %[t1],     12(%[dither])          \n\t"
        "li              %[x1],     0                      \n\t"
    "5:                                                    \n\t"
        "sll             %[t3],     %[t0],     7           \n\t"
        "sll             %[t4],     %[t1],     7           \n\t"
#ifdef __mips_dspr2
        "append          %[t0],     %[t1],     16          \n\t"
#else
        "sll             %[t0],     %[t0],     8           \n\t"
        "sll             %[t2],     %[t1],     8           \n\t"
        "precrq.qb.ph    %[t0],     %[t0],     %[t2]       \n\t"
#endif
        "precrq.qb.ph    %[t1],     %[t3],     %[t4]       \n\t"
        "sll             %[t5],     %[s0],     8           \n\t"
        "sll             %[t6],     %[s1],     8           \n\t"
        "precrq.qb.ph    %[t4],     %[t5],     %[t6]       \n\t"
        "precrq.qb.ph    %[t6],     %[s0],     %[s1]       \n\t"
        "preceu.ph.qbla  %[t5],     %[t4]                  \n\t"
        "preceu.ph.qbra  %[t4],     %[t4]                  \n\t"
        "preceu.ph.qbra  %[t6],     %[t6]                  \n\t"
        "lh              %[t2],     0(%[dst])              \n\t"
        "lh              %[s1],     2(%[dst])              \n\t"
#ifdef __mips_dspr2
        "append          %[t2],     %[s1],     16          \n\t"
#else
        "sll             %[s1],     %[s1],     16          \n\t"
        "packrl.ph       %[t2],     %[t2],     %[s1]       \n\t"
#endif
        "shra.ph         %[s1],     %[t2],     11          \n\t"
        "and             %[s1],     %[s1],     0x1F001F    \n\t"
        "shra.ph         %[s2],     %[t2],     5           \n\t"
        "and             %[s2],     %[s2],     0x3F003F    \n\t"
        "and             %[s3],     %[t2],     0x1F001F    \n\t"
        "shrl.qb         %[t3],     %[t4],     5           \n\t"
        "addu.qb         %[t4],     %[t4],     %[t0]       \n\t"
        "subu.qb         %[t4],     %[t4],     %[t3]       \n\t"
        "shrl.qb         %[t4],     %[t4],     3           \n\t"
        "shrl.qb         %[t3],     %[t5],     5           \n\t"
        "addu.qb         %[t5],     %[t5],     %[t0]       \n\t"
        "subu.qb         %[t5],     %[t5],     %[t3]       \n\t"
        "shrl.qb         %[t5],     %[t5],     3           \n\t"
        "shrl.qb         %[t3],     %[t6],     6           \n\t"
        "addu.qb         %[t6],     %[t6],     %[t1]       \n\t"
        "subu.qb         %[t6],     %[t6],     %[t3]       \n\t"
        "shrl.qb         %[t6],     %[t6],     2           \n\t"
        "cmpu.lt.qb      %[t4],     %[s1]                  \n\t"
        "pick.qb         %[s0],     %[sc_add], $0          \n\t"
        "addu.qb         %[s0],     %[s0],     %[s1]       \n\t"
        "subu.qb         %[t4],     %[t4],     %[s1]       \n\t"
        "muleu_s.ph.qbl  %[t0],     %[t4],     %[sc_mul]   \n\t"
        "muleu_s.ph.qbr  %[t1],     %[t4],     %[sc_mul]   \n\t"
        "precrq.qb.ph    %[t4],     %[t0],     %[t1]       \n\t"
        "addu.qb         %[t4],     %[t4],     %[s0]       \n\t"
        "cmpu.lt.qb      %[t5],     %[s3]                  \n\t"
        "pick.qb         %[s0],     %[sc_add], $0          \n\t"
        "addu.qb         %[s0],     %[s0],     %[s3]       \n\t"
        "subu.qb         %[t5],     %[t5],     %[s3]       \n\t"
        "muleu_s.ph.qbl  %[t0],     %[t5],     %[sc_mul]   \n\t"
        "muleu_s.ph.qbr  %[t1],     %[t5],     %[sc_mul]   \n\t"
        "precrq.qb.ph    %[t5],     %[t0],     %[t1]       \n\t"
        "addu.qb         %[t5],     %[t5],     %[s0]       \n\t"
        "cmpu.lt.qb      %[t6],     %[s2]                  \n\t"
        "pick.qb         %[s0],     %[sc_add], $0          \n\t"
        "addu.qb         %[s0],     %[s0],     %[s2]       \n\t"
        "subu.qb         %[t6],     %[t6],     %[s2]       \n\t"
        "muleu_s.ph.qbl  %[t0],     %[t6],     %[sc_mul]   \n\t"
        "muleu_s.ph.qbr  %[t1],     %[t6],     %[sc_mul]   \n\t"
        "precrq.qb.ph    %[t6],     %[t0],     %[t1]       \n\t"
        "addu.qb         %[t6],     %[t6],     %[s0]       \n\t"
        "shll.ph         %[s1],     %[t4],     11          \n\t"
        "shll.ph         %[t0],     %[t6],     5           \n\t"
        "or              %[s0],     %[s1],     %[t0]       \n\t"
        "or              %[s1],     %[s0],     %[t5]       \n\t"
        "srl             %[t2],     %[s1],     16          \n\t"
        "and             %[t3],     %[s1],     0xFFFF      \n\t"
        "sh              %[t2],     0(%[dst])              \n\t"
        "sh              %[t3],     2(%[dst])              \n\t"
        "addiu           %[src],    %[src],    8           \n\t"
        "addi            %[count],  %[count],  -2          \n\t"
        "b               2b                                \n\t"
        " addu           %[dst],    %[dst],    4           \n\t"
    "3:                                                    \n\t"
        ".set            pop                               \n\t"
        : [src]"+r"(src), [dst]"+r"(dst), [count]"+r"(count),
          [x1]"+r"(x1), [sc_mul]"=&r"(sc_mul), [sc_add]"=&r"(sc_add),
          [t0]"=&r"(t0), [t1]"=&r"(t1), [t2]"=&r"(t2), [t3]"=&r"(t3),
          [t4]"=&r"(t4), [t5]"=&r"(t5), [t6]"=&r"(t6), [s0]"=&r"(s0),
          [s1]"=&r"(s1), [s2]"=&r"(s2), [s3]"=&r"(s3)
        : [dither]"r"(dither), [alpha]"r"(alpha)
        : "memory", "hi", "lo"
    );

    if(count == 1) {
        SkPMColor c = *src++;
        SkPMColorAssert(c);
        SkASSERT(SkGetPackedA32(c) == 255);
        DITHER_565_SCAN(y);
        int dither = DITHER_VALUE(x);
        int sr = SkGetPackedR32(c);
        int sg = SkGetPackedG32(c);
        int sb = SkGetPackedB32(c);
        sr = SkDITHER_R32To565(sr, dither);
        sg = SkDITHER_G32To565(sg, dither);
        sb = SkDITHER_B32To565(sb, dither);

        uint16_t d = *dst;
        *dst++ = SkPackRGB16(SkAlphaBlend(sr, SkGetPackedR16(d), alpha),
                             SkAlphaBlend(sg, SkGetPackedG16(d), alpha),
                             SkAlphaBlend(sb, SkGetPackedB16(d), alpha));
        DITHER_INC_X(x);
    }
}

static void S32A_D565_Opaque_mips_dsp(uint16_t* __restrict__ dst,
                                      const SkPMColor* __restrict__ src,
                                      int count, U8CPU alpha, int x, int y) {

    __asm__ volatile (
        "pref  0,  0(%[src])     \n\t"
        "pref  1,  0(%[dst])     \n\t"
        "pref  0,  32(%[src])    \n\t"
        "pref  1,  32(%[dst])    \n\t"
        :
        : [src]"r"(src), [dst]"r"(dst)
        : "memory"
    );

    register uint32_t t0, t1, t2, t3, t4, t5, t6, t7, t8;
    register uint32_t t16;
    register uint32_t add_x10 = 0x100010;
    register uint32_t add_x20 = 0x200020;
    register uint32_t sa = 0xff00ff;

    __asm__ volatile (
        ".set           push                            \n\t"
        ".set           noreorder                       \n\t"
        "blez           %[count], 1f                    \n\t"
        " nop                                           \n\t"
    "2:                                                 \n\t"
        "beqz           %[count], 1f                    \n\t"
        " nop                                           \n\t"
        "addiu          %[t0],    %[count], -1          \n\t"
        "beqz           %[t0],    1f                    \n\t"
        " nop                                           \n\t"
        "bnez           %[t16],   3f                    \n\t"
        " nop                                           \n\t"
        "li             %[t16],   2                     \n\t"
        "pref           0,        64(%[src])            \n\t"
        "pref           1,        64(%[dst])            \n\t"
    "3:                                                 \n\t"
        "addiu          %[t16],   %[t16],   -1          \n\t"
        "lw             %[t0],    0(%[src])             \n\t"
        "lw             %[t1],    4(%[src])             \n\t"
        "precrq.ph.w    %[t2],    %[t0],    %[t1]       \n\t"
        "preceu.ph.qbra %[t8],    %[t2]                 \n\t"
#ifdef __mips_dspr2
        "append         %[t0],    %[t1],    16          \n\t"
#else
        "sll            %[t0],    %[t0],    16          \n\t"
        "sll            %[t6],    %[t1],    16          \n\t"
        "precrq.ph.w    %[t0],    %[t0],    %[t6]       \n\t"
#endif
        "preceu.ph.qbra %[t3],    %[t0]                 \n\t"
        "preceu.ph.qbla %[t4],    %[t0]                 \n\t"
        "preceu.ph.qbla %[t0],    %[t2]                 \n\t"
        "subq.ph        %[t1],    %[sa],    %[t0]       \n\t"
        "sra            %[t2],    %[t1],    8           \n\t"
        "or             %[t5],    %[t2],    %[t1]       \n\t"
        "replv.ph       %[t2],    %[t5]                 \n\t"
        "lh             %[t0],    0(%[dst])             \n\t"
        "lh             %[t1],    2(%[dst])             \n\t"
        "and            %[t1],    %[t1],    0xffff      \n\t"
#ifdef __mips_dspr2
        "append         %[t0],    %[t1],    16          \n\t"
#else
        "sll            %[t5],    %[t0],    16          \n\t"
        "or             %[t0],    %[t5],    %[t1]       \n\t"
#endif
        "and            %[t1],    %[t0],    0x1f001f    \n\t"
        "shra.ph        %[t6],    %[t0],    11          \n\t"
        "and            %[t6],    %[t6],    0x1f001f    \n\t"
        "and            %[t7],    %[t0],    0x7e007e0   \n\t"
        "shra.ph        %[t5],    %[t7],    5           \n\t"
        "muleu_s.ph.qbl %[t0],    %[t2],    %[t6]       \n\t"
        "addq.ph        %[t7],    %[t0],    %[add_x10]  \n\t"
        "shra.ph        %[t6],    %[t7],    5           \n\t"
        "addq.ph        %[t6],    %[t7],    %[t6]       \n\t"
        "shra.ph        %[t0],    %[t6],    5           \n\t"
        "addq.ph        %[t7],    %[t0],    %[t3]       \n\t"
        "shra.ph        %[t6],    %[t7],    3           \n\t"
        "muleu_s.ph.qbl %[t0],    %[t2],    %[t1]       \n\t"
        "addq.ph        %[t7],    %[t0],    %[add_x10]  \n\t"
        "shra.ph        %[t0],    %[t7],    5           \n\t"
        "addq.ph        %[t7],    %[t7],    %[t0]       \n\t"
        "shra.ph        %[t0],    %[t7],    5           \n\t"
        "addq.ph        %[t7],    %[t0],    %[t8]       \n\t"
        "shra.ph        %[t3],    %[t7],    3           \n\t"
        "muleu_s.ph.qbl %[t0],    %[t2],    %[t5]       \n\t"
        "addq.ph        %[t7],    %[t0],    %[add_x20]  \n\t"
        "shra.ph        %[t0],    %[t7],    6           \n\t"
        "addq.ph        %[t8],    %[t7],    %[t0]       \n\t"
        "shra.ph        %[t0],    %[t8],    6           \n\t"
        "addq.ph        %[t7],    %[t0],    %[t4]       \n\t"
        "shra.ph        %[t8],    %[t7],    2           \n\t"
        "shll.ph        %[t0],    %[t8],    5           \n\t"
        "shll.ph        %[t1],    %[t6],    11          \n\t"
        "or             %[t2],    %[t0],    %[t1]       \n\t"
        "or             %[t3],    %[t2],    %[t3]       \n\t"
        "sra            %[t4],    %[t3],    16          \n\t"
        "sh             %[t4],    0(%[dst])             \n\t"
        "sh             %[t3],    2(%[dst])             \n\t"
        "addiu          %[count], %[count], -2          \n\t"
        "addiu          %[src],   %[src],   8           \n\t"
        "b              2b                              \n\t"
        " addiu         %[dst],   %[dst],   4           \n\t"
    "1:                                                 \n\t"
        ".set           pop                             \n\t"
        : [dst]"+r"(dst), [src]"+r"(src), [count]"+r"(count),
          [t16]"=&r"(t16), [t0]"=&r"(t0), [t1]"=&r"(t1), [t2]"=&r"(t2),
          [t3]"=&r"(t3), [t4]"=&r"(t4), [t5]"=&r"(t5), [t6]"=&r"(t6),
          [t7]"=&r"(t7), [t8]"=&r"(t8)
        : [add_x10]"r"(add_x10), [add_x20]"r"(add_x20), [sa]"r"(sa)
        : "memory", "hi", "lo"
    );

    if (count == 1) {
        SkPMColor c = *src++;
        SkPMColorAssert(c);
        if (c) {
            *dst = SkSrcOver32To16(c, *dst);
        }
        dst += 1;
    }
}

static void S32A_D565_Blend_mips_dsp(uint16_t* SK_RESTRICT dst,
                                     const SkPMColor* SK_RESTRICT src, int count,
                                     U8CPU alpha, int /*x*/, int /*y*/) {
    register uint32_t t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
    register uint32_t  s0, s1, s2, s3;
    register unsigned dst_scale = 0;

    __asm__ volatile (
        ".set            push                                       \n\t"
        ".set            noreorder                                  \n\t"
        "replv.qb        %[t0],        %[alpha]                     \n\t"
        "repl.ph         %[t6],        0x80                         \n\t"
        "repl.ph         %[t7],        0xFF                         \n\t"
    "1:                                                             \n\t"
        "addiu           %[t8],        %[count],     -1             \n\t"
        "blez            %[t8],        2f                           \n\t"
        " nop                                                       \n\t"
        "lw              %[t8],        0(%[src])                    \n\t"
        "lw              %[t9],        4(%[src])                    \n\t"
        "lh              %[t4],        0(%[dst])                    \n\t"
        "lh              %[t5],        2(%[dst])                    \n\t"
        "sll             %[t5],        %[t5],        16             \n\t"
        "sll             %[t2],        %[t8],        8              \n\t"
        "sll             %[t3],        %[t9],        8              \n\t"
        "precrq.qb.ph    %[t1],        %[t2],        %[t3]          \n\t"
        "precrq.qb.ph    %[t3],        %[t8],        %[t9]          \n\t"
        "preceu.ph.qbla  %[t8],        %[t3]                        \n\t"
        "muleu_s.ph.qbr  %[s3],        %[t0],        %[t8]          \n\t"
        "preceu.ph.qbla  %[t2],        %[t1]                        \n\t"
        "preceu.ph.qbra  %[t1],        %[t1]                        \n\t"
        "preceu.ph.qbra  %[t3],        %[t3]                        \n\t"
        "packrl.ph       %[t9],        %[t4],        %[t5]          \n\t"
        "shra.ph         %[s0],        %[t9],        11             \n\t"
        "and             %[s0],        %[s0],        0x1F001F       \n\t"
        "shra.ph         %[s1],        %[t9],        5              \n\t"
        "and             %[s1],        %[s1],        0x3F003F       \n\t"
        "and             %[s2],        %[t9],        0x1F001F       \n\t"
        "addq.ph         %[s3],        %[s3],        %[t6]          \n\t"
        "shra.ph         %[t5],        %[s3],        8              \n\t"
        "and             %[t5],        %[t5],        0xFF00FF       \n\t"
        "addq.ph         %[dst_scale], %[s3],        %[t5]          \n\t"
        "shra.ph         %[dst_scale], %[dst_scale], 8              \n\t"
        "subq_s.ph       %[dst_scale], %[t7],        %[dst_scale]   \n\t"
        "sll             %[dst_scale], %[dst_scale], 8              \n\t"
        "precrq.qb.ph    %[dst_scale], %[dst_scale], %[dst_scale]   \n\t"
        "shrl.qb         %[t1],        %[t1],        3              \n\t"
        "shrl.qb         %[t2],        %[t2],        3              \n\t"
        "shrl.qb         %[t3],        %[t3],        2              \n\t"
        "muleu_s.ph.qbl  %[t1],        %[t0],        %[t1]          \n\t"
        "muleu_s.ph.qbl  %[t2],        %[t0],        %[t2]          \n\t"
        "muleu_s.ph.qbl  %[t3],        %[t0],        %[t3]          \n\t"
        "muleu_s.ph.qbl  %[t8],        %[dst_scale], %[s0]          \n\t"
        "muleu_s.ph.qbl  %[t9],        %[dst_scale], %[s2]          \n\t"
        "muleu_s.ph.qbl  %[t4],        %[dst_scale], %[s1]          \n\t"
        "addq.ph         %[t1],        %[t1],        %[t8]          \n\t"
        "addq.ph         %[t2],        %[t2],        %[t9]          \n\t"
        "addq.ph         %[t3],        %[t3],        %[t4]          \n\t"
        "addq.ph         %[t8],        %[t1],        %[t6]          \n\t"
        "addq.ph         %[t9],        %[t2],        %[t6]          \n\t"
        "addq.ph         %[t4],        %[t3],        %[t6]          \n\t"
        "shra.ph         %[t1],        %[t8],        8              \n\t"
        "addq.ph         %[t1],        %[t1],        %[t8]          \n\t"
        "preceu.ph.qbla  %[t1],        %[t1]                        \n\t"
        "shra.ph         %[t2],        %[t9],        8              \n\t"
        "addq.ph         %[t2],        %[t2],        %[t9]          \n\t"
        "preceu.ph.qbla  %[t2],        %[t2]                        \n\t"
        "shra.ph         %[t3],        %[t4],        8              \n\t"
        "addq.ph         %[t3],        %[t3],        %[t4]          \n\t"
        "preceu.ph.qbla  %[t3],        %[t3]                        \n\t"
        "shll.ph         %[t8],        %[t1],        11             \n\t"
        "shll.ph         %[t9],        %[t3],        5              \n\t"
        "or              %[t8],        %[t8],        %[t9]          \n\t"
        "or              %[s0],        %[t8],        %[t2]          \n\t"
        "srl             %[t8],        %[s0],        16             \n\t"
        "and             %[t9],        %[s0],        0xFFFF         \n\t"
        "sh              %[t8],        0(%[dst])                    \n\t"
        "sh              %[t9],        2(%[dst])                    \n\t"
        "addiu           %[src],       %[src],       8              \n\t"
        "addiu           %[count],     %[count],     -2             \n\t"
        "b               1b                                         \n\t"
        " addiu          %[dst],       %[dst],       4              \n\t"
    "2:                                                             \n\t"
        ".set            pop                                        \n\t"
        : [src]"+r"(src), [dst]"+r"(dst), [count]"+r"(count),
          [dst_scale]"+r"(dst_scale), [s0]"=&r"(s0), [s1]"=&r"(s1),
          [s2]"=&r"(s2), [s3]"=&r"(s3), [t0]"=&r"(t0), [t1]"=&r"(t1),
          [t2]"=&r"(t2), [t3]"=&r"(t3), [t4]"=&r"(t4), [t5]"=&r"(t5),
          [t6]"=&r"(t6), [t7]"=&r"(t7), [t8]"=&r"(t8), [t9]"=&r"(t9)
        : [alpha]"r"(alpha)
        : "memory", "hi", "lo"
    );

    if (count == 1) {
        SkPMColor sc = *src++;
        SkPMColorAssert(sc);
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

static void S32_Blend_BlitRow32_mips_dsp(SkPMColor* SK_RESTRICT dst,
                                         const SkPMColor* SK_RESTRICT src,
                                         int count, U8CPU alpha) {
    register int32_t t0, t1, t2, t3, t4, t5, t6, t7;

    __asm__ volatile (
        ".set            push                         \n\t"
        ".set            noreorder                    \n\t"
        "li              %[t2],    0x100              \n\t"
        "addiu           %[t0],    %[alpha], 1        \n\t"
        "subu            %[t1],    %[t2],    %[t0]    \n\t"
        "replv.qb        %[t7],    %[t0]              \n\t"
        "replv.qb        %[t6],    %[t1]              \n\t"
    "1:                                               \n\t"
        "blez            %[count], 2f                 \n\t"
        "lw              %[t0],    0(%[src])          \n\t"
        "lw              %[t1],    0(%[dst])          \n\t"
        "preceu.ph.qbr   %[t2],    %[t0]              \n\t"
        "preceu.ph.qbl   %[t3],    %[t0]              \n\t"
        "preceu.ph.qbr   %[t4],    %[t1]              \n\t"
        "preceu.ph.qbl   %[t5],    %[t1]              \n\t"
        "muleu_s.ph.qbr  %[t2],    %[t7],    %[t2]    \n\t"
        "muleu_s.ph.qbr  %[t3],    %[t7],    %[t3]    \n\t"
        "muleu_s.ph.qbr  %[t4],    %[t6],    %[t4]    \n\t"
        "muleu_s.ph.qbr  %[t5],    %[t6],    %[t5]    \n\t"
        "addiu           %[src],   %[src],   4        \n\t"
        "addiu           %[count], %[count], -1       \n\t"
        "precrq.qb.ph    %[t0],    %[t3],    %[t2]    \n\t"
        "precrq.qb.ph    %[t2],    %[t5],    %[t4]    \n\t"
        "addu            %[t1],    %[t0],    %[t2]    \n\t"
        "sw              %[t1],    0(%[dst])          \n\t"
        "b               1b                           \n\t"
        " addi           %[dst],   %[dst],   4        \n\t"
    "2:                                               \n\t"
        ".set            pop                          \n\t"
        : [src]"+r"(src), [dst]"+r"(dst), [count]"+r"(count),
          [t0]"=&r"(t0), [t1]"=&r"(t1), [t2]"=&r"(t2), [t3]"=&r"(t3),
          [t4]"=&r"(t4), [t5]"=&r"(t5), [t6]"=&r"(t6), [t7]"=&r"(t7)
        : [alpha]"r"(alpha)
        : "memory", "hi", "lo"
    );
}

void blitmask_d565_opaque_mips(int width, int height, uint16_t* device,
                               unsigned deviceRB, const uint8_t* alpha,
                               uint32_t expanded32, unsigned maskRB) {
    register uint32_t s0, s1, s2, s3;

    __asm__ volatile (
        ".set            push                                    \n\t"
        ".set            noreorder                               \n\t"
        ".set            noat                                    \n\t"
        "li              $t9,       0x7E0F81F                    \n\t"
    "1:                                                          \n\t"
        "move            $t8,       %[width]                     \n\t"
        "addiu           %[height], %[height],     -1            \n\t"
    "2:                                                          \n\t"
        "beqz            $t8,       4f                           \n\t"
        " addiu          $t0,       $t8,           -4            \n\t"
        "bltz            $t0,       3f                           \n\t"
        " nop                                                    \n\t"
        "addiu           $t8,       $t8,           -4            \n\t"
        "lhu             $t0,       0(%[device])                 \n\t"
        "lhu             $t1,       2(%[device])                 \n\t"
        "lhu             $t2,       4(%[device])                 \n\t"
        "lhu             $t3,       6(%[device])                 \n\t"
        "lbu             $t4,       0(%[alpha])                  \n\t"
        "lbu             $t5,       1(%[alpha])                  \n\t"
        "lbu             $t6,       2(%[alpha])                  \n\t"
        "lbu             $t7,       3(%[alpha])                  \n\t"
        "replv.ph        $t0,       $t0                          \n\t"
        "replv.ph        $t1,       $t1                          \n\t"
        "replv.ph        $t2,       $t2                          \n\t"
        "replv.ph        $t3,       $t3                          \n\t"
        "addiu           %[s0],     $t4,           1             \n\t"
        "addiu           %[s1],     $t5,           1             \n\t"
        "addiu           %[s2],     $t6,           1             \n\t"
        "addiu           %[s3],     $t7,           1             \n\t"
        "srl             %[s0],     %[s0],         3             \n\t"
        "srl             %[s1],     %[s1],         3             \n\t"
        "srl             %[s2],     %[s2],         3             \n\t"
        "srl             %[s3],     %[s3],         3             \n\t"
        "and             $t0,       $t0,           $t9           \n\t"
        "and             $t1,       $t1,           $t9           \n\t"
        "and             $t2,       $t2,           $t9           \n\t"
        "and             $t3,       $t3,           $t9           \n\t"
        "subu            $t4,       %[expanded32], $t0           \n\t"
        "subu            $t5,       %[expanded32], $t1           \n\t"
        "subu            $t6,       %[expanded32], $t2           \n\t"
        "subu            $t7,       %[expanded32], $t3           \n\t"
        "mul             $t4,       $t4,           %[s0]         \n\t"
        "mul             $t5,       $t5,           %[s1]         \n\t"
        "mul             $t6,       $t6,           %[s2]         \n\t"
        "mul             $t7,       $t7,           %[s3]         \n\t"
        "addiu           %[alpha],  %[alpha],      4             \n\t"
        "srl             $t4,       $t4,           5             \n\t"
        "srl             $t5,       $t5,           5             \n\t"
        "srl             $t6,       $t6,           5             \n\t"
        "srl             $t7,       $t7,           5             \n\t"
        "addu            $t4,       $t0,           $t4           \n\t"
        "addu            $t5,       $t1,           $t5           \n\t"
        "addu            $t6,       $t2,           $t6           \n\t"
        "addu            $t7,       $t3,           $t7           \n\t"
        "and             $t4,       $t4,           $t9           \n\t"
        "and             $t5,       $t5,           $t9           \n\t"
        "and             $t6,       $t6,           $t9           \n\t"
        "and             $t7,       $t7,           $t9           \n\t"
        "srl             $t0,       $t4,           16            \n\t"
        "srl             $t1,       $t5,           16            \n\t"
        "srl             $t2,       $t6,           16            \n\t"
        "srl             $t3,       $t7,           16            \n\t"
        "or              %[s0],     $t0,           $t4           \n\t"
        "or              %[s1],     $t1,           $t5           \n\t"
        "or              %[s2],     $t2,           $t6           \n\t"
        "or              %[s3],     $t3,           $t7           \n\t"
        "sh              %[s0],     0(%[device])                 \n\t"
        "sh              %[s1],     2(%[device])                 \n\t"
        "sh              %[s2],     4(%[device])                 \n\t"
        "sh              %[s3],     6(%[device])                 \n\t"
        "b               2b                                      \n\t"
        " addiu          %[device], %[device],     8             \n\t"
    "3:                                                          \n\t"
        "lhu             $t0,       0(%[device])                 \n\t"
        "lbu             $t1,       0(%[alpha])                  \n\t"
        "addiu           $t8,       $t8,           -1            \n\t"
        "replv.ph        $t2,       $t0                          \n\t"
        "and             $t2,       $t2,           $t9           \n\t"
        "addiu           $t0,       $t1,           1             \n\t"
        "srl             $t0,       $t0,           3             \n\t"
        "subu            $t3,       %[expanded32], $t2           \n\t"
        "mul             $t3,       $t3,           $t0           \n\t"
        "addiu           %[alpha],  %[alpha],      1             \n\t"
        "srl             $t3,       $t3,           5             \n\t"
        "addu            $t3,       $t2,           $t3           \n\t"
        "and             $t3,       $t3,           $t9           \n\t"
        "srl             $t4,       $t3,           16            \n\t"
        "or              %[s0],     $t4,           $t3           \n\t"
        "sh              %[s0],     0(%[device])                 \n\t"
        "bnez            $t8,       3b                           \n\t"
         "addiu          %[device], %[device],     2             \n\t"
    "4:                                                          \n\t"
        "addu            %[device], %[device],     %[deviceRB]   \n\t"
        "bgtz            %[height], 1b                           \n\t"
        " addu           %[alpha],  %[alpha],      %[maskRB]     \n\t"
        ".set            pop                                     \n\t"
        : [height]"+r"(height), [alpha]"+r"(alpha), [device]"+r"(device),
          [deviceRB]"+r"(deviceRB), [maskRB]"+r"(maskRB), [s0]"=&r"(s0),
          [s1]"=&r"(s1), [s2]"=&r"(s2), [s3]"=&r"(s3)
        : [expanded32] "r" (expanded32), [width] "r" (width)
        : "memory", "hi", "lo", "t0", "t1", "t2", "t3",
          "t4", "t5", "t6", "t7", "t8", "t9"
    );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const SkBlitRow::Proc16 platform_565_procs_mips_dsp[] = {
    // no dither
    nullptr,
    S32_D565_Blend_mips_dsp,
    S32A_D565_Opaque_mips_dsp,
    S32A_D565_Blend_mips_dsp,

    // dither
    S32_D565_Opaque_Dither_mips_dsp,
    S32_D565_Blend_Dither_mips_dsp,
    S32A_D565_Opaque_Dither_mips_dsp,
    nullptr,
};

static const SkBlitRow::Proc32 platform_32_procs_mips_dsp[] = {
    nullptr,   // S32_Opaque,
    S32_Blend_BlitRow32_mips_dsp,   // S32_Blend,
    nullptr,   // S32A_Opaque,
    nullptr,   // S32A_Blend,
};

SkBlitRow::Proc16 SkBlitRow::PlatformFactory565(unsigned flags) {
    return platform_565_procs_mips_dsp[flags];
}

SkBlitRow::ColorProc16 SkBlitRow::PlatformColorFactory565(unsigned flags) {
    return nullptr;
}

SkBlitRow::Proc32 SkBlitRow::PlatformProcs32(unsigned flags) {
    return platform_32_procs_mips_dsp[flags];
}
