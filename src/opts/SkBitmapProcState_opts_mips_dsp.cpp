/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBitmapProcState.h"
#include "SkBitmapScaler.h"
#include "SkColorPriv.h"
#include "SkPaint.h"
#include "SkUtils.h"

static void SI8_D16_nofilter_DX_mips_dsp(const SkBitmapProcState& s,
                                         const uint32_t* SK_RESTRICT xy,
                                         int count, uint16_t* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask));
    SkASSERT(SkPaint::kNone_FilterLevel == s.fFilterLevel);
    const uint16_t* SK_RESTRICT table = s.fBitmap->getColorTable()->lock16BitCache();
    const uint8_t* SK_RESTRICT srcAddr = (const uint8_t*)s.fBitmap->getPixels();
    SkASSERT((unsigned)xy[0] < (unsigned)s.fBitmap->height());
    srcAddr = (const uint8_t*)((const char*)srcAddr + xy[0] * s.fBitmap->rowBytes());
    uint8_t src;

    if (1 == s.fBitmap->width()) {
        src = srcAddr[0];
        uint16_t dstValue = table[src];
        sk_memset16(colors, dstValue, count);
    } else {
        int count8;
        const uint16_t* SK_RESTRICT xx = (const uint16_t*)(xy + 1);

        __asm__ volatile (
            ".set    push                                \n\t"
            ".set    noreorder                           \n\t"
            ".set    noat                                \n\t"
            "sra     %[count8], %[count],      3         \n\t"
            "beqz    %[count8], 3f                       \n\t"
            " addiu  %[count8], %[count8],     -1        \n\t"
        "1:                                              \n\t"
            "beqz    %[count8], 2f                       \n\t"
            " addiu  %[count8], %[count8],     -1        \n\t"
            "pref    0,         16(%[xx])                \n\t"
            "lhu     $t0,       0(%[xx])                 \n\t"
            "lhu     $t1,       2(%[xx])                 \n\t"
            "lhu     $t2,       4(%[xx])                 \n\t"
            "lhu     $t3,       6(%[xx])                 \n\t"
            "lhu     $t4,       8(%[xx])                 \n\t"
            "lhu     $t5,       10(%[xx])                \n\t"
            "lhu     $t6,       12(%[xx])                \n\t"
            "lhu     $t7,       14(%[xx])                \n\t"
            "pref    0,         8(%[srcAddr])            \n\t"
            "lbux    $t0,       $t0(%[srcAddr])          \n\t"
            "lbux    $t1,       $t1(%[srcAddr])          \n\t"
            "lbux    $t2,       $t2(%[srcAddr])          \n\t"
            "lbux    $t3,       $t3(%[srcAddr])          \n\t"
            "lbux    $t4,       $t4(%[srcAddr])          \n\t"
            "lbux    $t5,       $t5(%[srcAddr])          \n\t"
            "lbux    $t6,       $t6(%[srcAddr])          \n\t"
            "lbux    $t7,       $t7(%[srcAddr])          \n\t"
            "addu    $t0,       $t0,           $t0       \n\t"
            "addu    $t1,       $t1,           $t1       \n\t"
            "addu    $t2,       $t2,           $t2       \n\t"
            "addu    $t3,       $t3,           $t3       \n\t"
            "addu    $t4,       $t4,           $t4       \n\t"
            "addu    $t5,       $t5,           $t5       \n\t"
            "addu    $t6,       $t6,           $t6       \n\t"
            "addu    $t7,       $t7,           $t7       \n\t"
            "pref    0,         16(%[table])             \n\t"
            "lhx     $t0,       $t0(%[table])            \n\t"
            "lhx     $t1,       $t1(%[table])            \n\t"
            "lhx     $t2,       $t2(%[table])            \n\t"
            "lhx     $t3,       $t3(%[table])            \n\t"
            "lhx     $t4,       $t4(%[table])            \n\t"
            "lhx     $t5,       $t5(%[table])            \n\t"
            "lhx     $t6,       $t6(%[table])            \n\t"
            "lhx     $t7,       $t7(%[table])            \n\t"
            "sh      $t0,       0(%[colors])             \n\t"
            "sh      $t1,       2(%[colors])             \n\t"
            "sh      $t2,       4(%[colors])             \n\t"
            "sh      $t3,       6(%[colors])             \n\t"
            "sh      $t4,       8(%[colors])             \n\t"
            "sh      $t5,       10(%[colors])            \n\t"
            "sh      $t6,       12(%[colors])            \n\t"
            "sh      $t7,       14(%[colors])            \n\t"
            "addiu   %[xx],     %[xx],         16        \n\t"
            "bgtz    %[count8], 1b                       \n\t"
            " addiu  %[colors], %[colors],     16        \n\t"
        "2:                                              \n\t"
            "lhu     $t0,       0(%[xx])                 \n\t"
            "lhu     $t1,       2(%[xx])                 \n\t"
            "lhu     $t2,       4(%[xx])                 \n\t"
            "lhu     $t3,       6(%[xx])                 \n\t"
            "lhu     $t4,       8(%[xx])                 \n\t"
            "lhu     $t5,       10(%[xx])                \n\t"
            "lhu     $t6,       12(%[xx])                \n\t"
            "lhu     $t7,       14(%[xx])                \n\t"
            "lbux    $t0,       $t0(%[srcAddr])          \n\t"
            "lbux    $t1,       $t1(%[srcAddr])          \n\t"
            "lbux    $t2,       $t2(%[srcAddr])          \n\t"
            "lbux    $t3,       $t3(%[srcAddr])          \n\t"
            "lbux    $t4,       $t4(%[srcAddr])          \n\t"
            "lbux    $t5,       $t5(%[srcAddr])          \n\t"
            "lbux    $t6,       $t6(%[srcAddr])          \n\t"
            "lbux    $t7,       $t7(%[srcAddr])          \n\t"
            "addu    $t0,       $t0,           $t0       \n\t"
            "addu    $t1,       $t1,           $t1       \n\t"
            "addu    $t2,       $t2,           $t2       \n\t"
            "addu    $t3,       $t3,           $t3       \n\t"
            "addu    $t4,       $t4,           $t4       \n\t"
            "addu    $t5,       $t5,           $t5       \n\t"
            "addu    $t6,       $t6,           $t6       \n\t"
            "addu    $t7,       $t7,           $t7       \n\t"
            "lhx     $t0,       $t0(%[table])            \n\t"
            "lhx     $t1,       $t1(%[table])            \n\t"
            "lhx     $t2,       $t2(%[table])            \n\t"
            "lhx     $t3,       $t3(%[table])            \n\t"
            "lhx     $t4,       $t4(%[table])            \n\t"
            "lhx     $t5,       $t5(%[table])            \n\t"
            "lhx     $t6,       $t6(%[table])            \n\t"
            "lhx     $t7,       $t7(%[table])            \n\t"
            "sh      $t0,       0(%[colors])             \n\t"
            "sh      $t1,       2(%[colors])             \n\t"
            "sh      $t2,       4(%[colors])             \n\t"
            "sh      $t3,       6(%[colors])             \n\t"
            "sh      $t4,       8(%[colors])             \n\t"
            "sh      $t5,       10(%[colors])            \n\t"
            "sh      $t6,       12(%[colors])            \n\t"
            "sh      $t7,       14(%[colors])            \n\t"
            "addiu   %[xx],     %[xx],         16        \n\t"
            "addiu   %[colors], %[colors],     16        \n\t"
        "3:                                              \n\t"
            ".set    pop                                 \n\t"
            : [xx]"+r"(xx), [count8]"=&r"(count8), [colors]"+r"(colors)
            : [table]"r"(table), [srcAddr]"r"(srcAddr), [count]"r"(count)
            : "memory","t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7"
        );

        for (int i = (count & 7); i > 0; --i) {
            src = srcAddr[*xx++]; *colors++ = table[src];
        }
    }
    s.fBitmap->getColorTable()->unlock16BitCache();
}

static void SI8_opaque_D32_nofilter_DX_mips_dsp(const SkBitmapProcState& s,
                                                const uint32_t* SK_RESTRICT xy,
                                                int count, SkPMColor* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask));
    SkASSERT(SkPaint::kNone_FilterLevel == s.fFilterLevel);
    const SkPMColor* SK_RESTRICT table = s.fBitmap->getColorTable()->lockColors();
    const uint8_t* SK_RESTRICT srcAddr = (const uint8_t*)s.fBitmap->getPixels();
    srcAddr = (const uint8_t*)((const char*)srcAddr + xy[0] * s.fBitmap->rowBytes());

    if (1 == s.fBitmap->width()) {
        uint8_t src = srcAddr[0];
        SkPMColor dstValue = table[src];
        sk_memset32(colors, dstValue, count);
    } else {
        const uint16_t* xx = (const uint16_t*)(xy + 1);
        int s0, s1, s2, s3, s4, s5, s6, s7;
        __asm__ volatile (
            ".set    push                                \n\t"
            ".set    noreorder                           \n\t"
            ".set    noat                                \n\t"
            "srl     $t8,       %[count],    4           \n\t"
            "beqz    $t8,       3f                       \n\t"
            " nop                                        \n\t"
        "1:                                              \n\t"
            "addiu   $t8,       $t8,         -1          \n\t"
            "beqz    $t8,       2f                       \n\t"
            " addiu  %[count],  %[count],    -16         \n\t"
            "pref    0,         32(%[xx])                \n\t"
            "lhu     $t0,       0(%[xx])                 \n\t"
            "lhu     $t1,       2(%[xx])                 \n\t"
            "lhu     $t2,       4(%[xx])                 \n\t"
            "lhu     $t3,       6(%[xx])                 \n\t"
            "lhu     $t4,       8(%[xx])                 \n\t"
            "lhu     $t5,       10(%[xx])                \n\t"
            "lhu     $t6,       12(%[xx])                \n\t"
            "lhu     $t7,       14(%[xx])                \n\t"
            "lhu     %[s0],     16(%[xx])                \n\t"
            "lhu     %[s1],     18(%[xx])                \n\t"
            "lhu     %[s2],     20(%[xx])                \n\t"
            "lhu     %[s3],     22(%[xx])                \n\t"
            "lhu     %[s4],     24(%[xx])                \n\t"
            "lhu     %[s5],     26(%[xx])                \n\t"
            "lhu     %[s6],     28(%[xx])                \n\t"
            "lhu     %[s7],     30(%[xx])                \n\t"
            "lbux    $t0,       $t0(%[srcAddr])          \n\t"
            "lbux    $t1,       $t1(%[srcAddr])          \n\t"
            "lbux    $t2,       $t2(%[srcAddr])          \n\t"
            "lbux    $t3,       $t3(%[srcAddr])          \n\t"
            "lbux    $t4,       $t4(%[srcAddr])          \n\t"
            "lbux    $t5,       $t5(%[srcAddr])          \n\t"
            "lbux    $t6,       $t6(%[srcAddr])          \n\t"
            "lbux    $t7,       $t7(%[srcAddr])          \n\t"
            "lbux    %[s0],     %[s0](%[srcAddr])        \n\t"
            "lbux    %[s1],     %[s1](%[srcAddr])        \n\t"
            "lbux    %[s2],     %[s2](%[srcAddr])        \n\t"
            "lbux    %[s3],     %[s3](%[srcAddr])        \n\t"
            "lbux    %[s4],     %[s4](%[srcAddr])        \n\t"
            "lbux    %[s5],     %[s5](%[srcAddr])        \n\t"
            "lbux    %[s6],     %[s6](%[srcAddr])        \n\t"
            "lbux    %[s7],     %[s7](%[srcAddr])        \n\t"
            "sll     $t0,       $t0,         2           \n\t"
            "sll     $t1,       $t1,         2           \n\t"
            "sll     $t2,       $t2,         2           \n\t"
            "sll     $t3,       $t3,         2           \n\t"
            "sll     $t4,       $t4,         2           \n\t"
            "sll     $t5,       $t5,         2           \n\t"
            "sll     $t6,       $t6,         2           \n\t"
            "sll     $t7,       $t7,         2           \n\t"
            "sll     %[s0],     %[s0],       2           \n\t"
            "sll     %[s1],     %[s1],       2           \n\t"
            "sll     %[s2],     %[s2],       2           \n\t"
            "sll     %[s3],     %[s3],       2           \n\t"
            "sll     %[s4],     %[s4],       2           \n\t"
            "sll     %[s5],     %[s5],       2           \n\t"
            "sll     %[s6],     %[s6],       2           \n\t"
            "sll     %[s7],     %[s7],       2           \n\t"
            "pref    0,         64(%[table])             \n\t"
            "lwx     $t0,       $t0(%[table])            \n\t"
            "lwx     $t1,       $t1(%[table])            \n\t"
            "lwx     $t2,       $t2(%[table])            \n\t"
            "lwx     $t3,       $t3(%[table])            \n\t"
            "lwx     $t4,       $t4(%[table])            \n\t"
            "lwx     $t5,       $t5(%[table])            \n\t"
            "lwx     $t6,       $t6(%[table])            \n\t"
            "lwx     $t7,       $t7(%[table])            \n\t"
            "lwx     %[s0],     %[s0](%[table])          \n\t"
            "lwx     %[s1],     %[s1](%[table])          \n\t"
            "lwx     %[s2],     %[s2](%[table])          \n\t"
            "lwx     %[s3],     %[s3](%[table])          \n\t"
            "lwx     %[s4],     %[s4](%[table])          \n\t"
            "lwx     %[s5],     %[s5](%[table])          \n\t"
            "lwx     %[s6],     %[s6](%[table])          \n\t"
            "lwx     %[s7],     %[s7](%[table])          \n\t"
            "pref    30,        64(%[colors])            \n\t"
            "sw      $t0,       0(%[colors])             \n\t"
            "sw      $t1,       4(%[colors])             \n\t"
            "sw      $t2,       8(%[colors])             \n\t"
            "sw      $t3,       12(%[colors])            \n\t"
            "sw      $t4,       16(%[colors])            \n\t"
            "sw      $t5,       20(%[colors])            \n\t"
            "sw      $t6,       24(%[colors])            \n\t"
            "sw      $t7,       28(%[colors])            \n\t"
            "sw      %[s0],     32(%[colors])            \n\t"
            "sw      %[s1],     36(%[colors])            \n\t"
            "sw      %[s2],     40(%[colors])            \n\t"
            "sw      %[s3],     44(%[colors])            \n\t"
            "sw      %[s4],     48(%[colors])            \n\t"
            "sw      %[s5],     52(%[colors])            \n\t"
            "sw      %[s6],     56(%[colors])            \n\t"
            "sw      %[s7],     60(%[colors])            \n\t"
            "addiu   %[xx],     %[xx],       32          \n\t"
            "b       1b                                  \n\t"
            " addiu  %[colors], %[colors],   64          \n\t"
        "2:                                              \n\t"
            "lhu     $t0,       0(%[xx])                 \n\t"
            "lhu     $t1,       2(%[xx])                 \n\t"
            "lhu     $t2,       4(%[xx])                 \n\t"
            "lhu     $t3,       6(%[xx])                 \n\t"
            "lhu     $t4,       8(%[xx])                 \n\t"
            "lhu     $t5,       10(%[xx])                \n\t"
            "lhu     $t6,       12(%[xx])                \n\t"
            "lhu     $t7,       14(%[xx])                \n\t"
            "lhu     %[s0],     16(%[xx])                \n\t"
            "lhu     %[s1],     18(%[xx])                \n\t"
            "lhu     %[s2],     20(%[xx])                \n\t"
            "lhu     %[s3],     22(%[xx])                \n\t"
            "lhu     %[s4],     24(%[xx])                \n\t"
            "lhu     %[s5],     26(%[xx])                \n\t"
            "lhu     %[s6],     28(%[xx])                \n\t"
            "lhu     %[s7],     30(%[xx])                \n\t"
            "lbux    $t0,       $t0(%[srcAddr])          \n\t"
            "lbux    $t1,       $t1(%[srcAddr])          \n\t"
            "lbux    $t2,       $t2(%[srcAddr])          \n\t"
            "lbux    $t3,       $t3(%[srcAddr])          \n\t"
            "lbux    $t4,       $t4(%[srcAddr])          \n\t"
            "lbux    $t5,       $t5(%[srcAddr])          \n\t"
            "lbux    $t6,       $t6(%[srcAddr])          \n\t"
            "lbux    $t7,       $t7(%[srcAddr])          \n\t"
            "lbux    %[s0],     %[s0](%[srcAddr])        \n\t"
            "lbux    %[s1],     %[s1](%[srcAddr])        \n\t"
            "lbux    %[s2],     %[s2](%[srcAddr])        \n\t"
            "lbux    %[s3],     %[s3](%[srcAddr])        \n\t"
            "lbux    %[s4],     %[s4](%[srcAddr])        \n\t"
            "lbux    %[s5],     %[s5](%[srcAddr])        \n\t"
            "lbux    %[s6],     %[s6](%[srcAddr])        \n\t"
            "lbux    %[s7],     %[s7](%[srcAddr])        \n\t"
            "sll     $t0,       $t0,         2           \n\t"
            "sll     $t1,       $t1,         2           \n\t"
            "sll     $t2,       $t2,         2           \n\t"
            "sll     $t3,       $t3,         2           \n\t"
            "sll     $t4,       $t4,         2           \n\t"
            "sll     $t5,       $t5,         2           \n\t"
            "sll     $t6,       $t6,         2           \n\t"
            "sll     $t7,       $t7,         2           \n\t"
            "sll     %[s0],     %[s0],       2           \n\t"
            "sll     %[s1],     %[s1],       2           \n\t"
            "sll     %[s2],     %[s2],       2           \n\t"
            "sll     %[s3],     %[s3],       2           \n\t"
            "sll     %[s4],     %[s4],       2           \n\t"
            "sll     %[s5],     %[s5],       2           \n\t"
            "sll     %[s6],     %[s6],       2           \n\t"
            "sll     %[s7],     %[s7],       2           \n\t"
            "lwx     $t0,       $t0(%[table])            \n\t"
            "lwx     $t1,       $t1(%[table])            \n\t"
            "lwx     $t2,       $t2(%[table])            \n\t"
            "lwx     $t3,       $t3(%[table])            \n\t"
            "lwx     $t4,       $t4(%[table])            \n\t"
            "lwx     $t5,       $t5(%[table])            \n\t"
            "lwx     $t6,       $t6(%[table])            \n\t"
            "lwx     $t7,       $t7(%[table])            \n\t"
            "lwx     %[s0],     %[s0](%[table])          \n\t"
            "lwx     %[s1],     %[s1](%[table])          \n\t"
            "lwx     %[s2],     %[s2](%[table])          \n\t"
            "lwx     %[s3],     %[s3](%[table])          \n\t"
            "lwx     %[s4],     %[s4](%[table])          \n\t"
            "lwx     %[s5],     %[s5](%[table])          \n\t"
            "lwx     %[s6],     %[s6](%[table])          \n\t"
            "lwx     %[s7],     %[s7](%[table])          \n\t"
            "sw      $t0,       0(%[colors])             \n\t"
            "sw      $t1,       4(%[colors])             \n\t"
            "sw      $t2,       8(%[colors])             \n\t"
            "sw      $t3,       12(%[colors])            \n\t"
            "sw      $t4,       16(%[colors])            \n\t"
            "sw      $t5,       20(%[colors])            \n\t"
            "sw      $t6,       24(%[colors])            \n\t"
            "sw      $t7,       28(%[colors])            \n\t"
            "sw      %[s0],     32(%[colors])            \n\t"
            "sw      %[s1],     36(%[colors])            \n\t"
            "sw      %[s2],     40(%[colors])            \n\t"
            "sw      %[s3],     44(%[colors])            \n\t"
            "sw      %[s4],     48(%[colors])            \n\t"
            "sw      %[s5],     52(%[colors])            \n\t"
            "sw      %[s6],     56(%[colors])            \n\t"
            "sw      %[s7],     60(%[colors])            \n\t"
            "addiu   %[xx],     %[xx],       32          \n\t"
            "beqz    %[count],  4f                       \n\t"
            " addiu  %[colors], %[colors],   64          \n\t"
        "3:                                              \n\t"
            "addiu   %[count],  %[count],    -1          \n\t"
            "lhu     $t0,       0(%[xx])                 \n\t"
            "lbux    $t1,       $t0(%[srcAddr])          \n\t"
            "sll     $t1,       $t1,         2           \n\t"
            "lwx     $t2,       $t1(%[table])            \n\t"
            "sw      $t2,       0(%[colors])             \n\t"
            "addiu   %[xx],     %[xx],       2           \n\t"
            "bnez    %[count],  3b                       \n\t"
            " addiu  %[colors], %[colors],   4           \n\t"
        "4:                                              \n\t"
            ".set    pop                                 \n\t"
            : [xx]"+r"(xx), [count]"+r"(count), [colors]"+r"(colors),
              [s0]"=&r"(s0), [s1]"=&r"(s1), [s2]"=&r"(s2), [s3]"=&r"(s3),
              [s4]"=&r"(s4), [s5]"=&r"(s5), [s6]"=&r"(s6), [s7]"=&r"(s7)
            : [table]"r"(table), [srcAddr]"r"(srcAddr)
            : "memory", "t0", "t1", "t2", "t3",
              "t4", "t5", "t6", "t7", "t8"
        );
    }
    s.fBitmap->getColorTable()->unlockColors();
}

/*  If we replace a sampleproc, then we null-out the associated shaderproc,
    otherwise the shader won't even look at the matrix/sampler
 */

void SkBitmapProcState::platformProcs() {
    bool isOpaque = 256 == fAlphaScale;
    bool justDx = false;

    if (fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask)) {
        justDx = true;
    }

    switch (fBitmap->colorType()) {
        case kIndex_8_SkColorType:
            if (justDx && SkPaint::kNone_FilterLevel == fFilterLevel) {
                fSampleProc16 = SI8_D16_nofilter_DX_mips_dsp;
                fShaderProc16 = NULL;
                if (isOpaque) {
                    fSampleProc32 = SI8_opaque_D32_nofilter_DX_mips_dsp;
                    fShaderProc32 = NULL;
                }
            }
            break;
        default:
            break;
    }
}

void SkBitmapScaler::PlatformConvolutionProcs(SkConvolutionProcs*) {}
