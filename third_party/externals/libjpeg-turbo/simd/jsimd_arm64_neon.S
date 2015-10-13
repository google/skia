/*
 * ARMv8 NEON optimizations for libjpeg-turbo
 *
 * Copyright (C) 2009-2011 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Author: Siarhei Siamashka <siarhei.siamashka@nokia.com>
 * Copyright (C) 2013-2014, Linaro Limited
 * Author: Ragesh Radhakrishnan <ragesh.r@linaro.org>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits /* mark stack as non-executable */
#endif

.text
.arch armv8-a+fp+simd


#define RESPECT_STRICT_ALIGNMENT 1


/*****************************************************************************/

/* Supplementary macro for setting function attributes */
.macro asm_function fname
#ifdef __APPLE__
    .globl _\fname
_\fname:
#else
    .global \fname
#ifdef __ELF__
    .hidden \fname
    .type \fname, %function
#endif
\fname:
#endif
.endm

/* Transpose elements of single 128 bit registers */
.macro transpose_single x0,x1,xi,xilen,literal
    ins  \xi\xilen[0],  \x0\xilen[0]
    ins  \x1\xilen[0],  \x0\xilen[1]
    trn1 \x0\literal,   \x0\literal, \x1\literal
    trn2 \x1\literal,   \xi\literal, \x1\literal
.endm

/* Transpose elements of 2 differnet registers */
.macro transpose x0,x1,xi,xilen,literal
    mov  \xi\xilen,     \x0\xilen
    trn1 \x0\literal,   \x0\literal, \x1\literal
    trn2 \x1\literal,   \xi\literal, \x1\literal
.endm

/* Transpose a block of 4x4 coefficients in four 64-bit registers */
.macro transpose_4x4_32 x0,x0len x1,x1len x2,x2len x3,x3len,xi,xilen
    mov  \xi\xilen, \x0\xilen
    trn1 \x0\x0len, \x0\x0len, \x2\x2len
    trn2 \x2\x2len, \xi\x0len, \x2\x2len
    mov  \xi\xilen, \x1\xilen
    trn1 \x1\x1len, \x1\x1len, \x3\x3len
    trn2 \x3\x3len, \xi\x1len, \x3\x3len
.endm

.macro transpose_4x4_16 x0,x0len x1,x1len, x2,x2len, x3,x3len,xi,xilen
    mov  \xi\xilen, \x0\xilen
    trn1 \x0\x0len, \x0\x0len, \x1\x1len
    trn2 \x1\x2len, \xi\x0len, \x1\x2len
    mov  \xi\xilen, \x2\xilen
    trn1 \x2\x2len, \x2\x2len, \x3\x3len
    trn2 \x3\x2len, \xi\x1len, \x3\x3len
.endm

.macro transpose_4x4 x0, x1, x2, x3,x5
    transpose_4x4_16 \x0,.4h, \x1,.4h, \x2,.4h,\x3,.4h,\x5,.16b
    transpose_4x4_32 \x0,.2s, \x1,.2s, \x2,.2s,\x3,.2s,\x5,.16b
.endm


#define CENTERJSAMPLE 128

/*****************************************************************************/

/*
 * Perform dequantization and inverse DCT on one block of coefficients.
 *
 * GLOBAL(void)
 * jsimd_idct_islow_neon (void * dct_table, JCOEFPTR coef_block,
 *                        JSAMPARRAY output_buf, JDIMENSION output_col)
 */

#define FIX_0_298631336  (2446)
#define FIX_0_390180644  (3196)
#define FIX_0_541196100  (4433)
#define FIX_0_765366865  (6270)
#define FIX_0_899976223  (7373)
#define FIX_1_175875602  (9633)
#define FIX_1_501321110  (12299)
#define FIX_1_847759065  (15137)
#define FIX_1_961570560  (16069)
#define FIX_2_053119869  (16819)
#define FIX_2_562915447  (20995)
#define FIX_3_072711026  (25172)

#define FIX_1_175875602_MINUS_1_961570560 (FIX_1_175875602 - FIX_1_961570560)
#define FIX_1_175875602_MINUS_0_390180644 (FIX_1_175875602 - FIX_0_390180644)
#define FIX_0_541196100_MINUS_1_847759065 (FIX_0_541196100 - FIX_1_847759065)
#define FIX_3_072711026_MINUS_2_562915447 (FIX_3_072711026 - FIX_2_562915447)
#define FIX_0_298631336_MINUS_0_899976223 (FIX_0_298631336 - FIX_0_899976223)
#define FIX_1_501321110_MINUS_0_899976223 (FIX_1_501321110 - FIX_0_899976223)
#define FIX_2_053119869_MINUS_2_562915447 (FIX_2_053119869 - FIX_2_562915447)
#define FIX_0_541196100_PLUS_0_765366865  (FIX_0_541196100 + FIX_0_765366865)

/*
 * Reference SIMD-friendly 1-D ISLOW iDCT C implementation.
 * Uses some ideas from the comments in 'simd/jiss2int-64.asm'
 */
#define REF_1D_IDCT(xrow0, xrow1, xrow2, xrow3, xrow4, xrow5, xrow6, xrow7)   \
{                                                                             \
    DCTELEM row0, row1, row2, row3, row4, row5, row6, row7;                   \
    INT32   q1, q2, q3, q4, q5, q6, q7;                                       \
    INT32   tmp11_plus_tmp2, tmp11_minus_tmp2;                                \
                                                                              \
    /* 1-D iDCT input data */                                                 \
    row0 = xrow0;                                                             \
    row1 = xrow1;                                                             \
    row2 = xrow2;                                                             \
    row3 = xrow3;                                                             \
    row4 = xrow4;                                                             \
    row5 = xrow5;                                                             \
    row6 = xrow6;                                                             \
    row7 = xrow7;                                                             \
                                                                              \
    q5 = row7 + row3;                                                         \
    q4 = row5 + row1;                                                         \
    q6 = MULTIPLY(q5, FIX_1_175875602_MINUS_1_961570560) +                    \
         MULTIPLY(q4, FIX_1_175875602);                                       \
    q7 = MULTIPLY(q5, FIX_1_175875602) +                                      \
         MULTIPLY(q4, FIX_1_175875602_MINUS_0_390180644);                     \
    q2 = MULTIPLY(row2, FIX_0_541196100) +                                    \
         MULTIPLY(row6, FIX_0_541196100_MINUS_1_847759065);                   \
    q4 = q6;                                                                  \
    q3 = ((INT32) row0 - (INT32) row4) << 13;                                 \
    q6 += MULTIPLY(row5, -FIX_2_562915447) +                                  \
          MULTIPLY(row3, FIX_3_072711026_MINUS_2_562915447);                  \
    /* now we can use q1 (reloadable constants have been used up) */          \
    q1 = q3 + q2;                                                             \
    q4 += MULTIPLY(row7, FIX_0_298631336_MINUS_0_899976223) +                 \
          MULTIPLY(row1, -FIX_0_899976223);                                   \
    q5 = q7;                                                                  \
    q1 = q1 + q6;                                                             \
    q7 += MULTIPLY(row7, -FIX_0_899976223) +                                  \
          MULTIPLY(row1, FIX_1_501321110_MINUS_0_899976223);                  \
                                                                              \
    /* (tmp11 + tmp2) has been calculated (out_row1 before descale) */        \
    tmp11_plus_tmp2 = q1;                                                     \
    row1 = 0;                                                                 \
                                                                              \
    q1 = q1 - q6;                                                             \
    q5 += MULTIPLY(row5, FIX_2_053119869_MINUS_2_562915447) +                 \
          MULTIPLY(row3, -FIX_2_562915447);                                   \
    q1 = q1 - q6;                                                             \
    q6 = MULTIPLY(row2, FIX_0_541196100_PLUS_0_765366865) +                   \
         MULTIPLY(row6, FIX_0_541196100);                                     \
    q3 = q3 - q2;                                                             \
                                                                              \
    /* (tmp11 - tmp2) has been calculated (out_row6 before descale) */        \
    tmp11_minus_tmp2 = q1;                                                    \
                                                                              \
    q1 = ((INT32) row0 + (INT32) row4) << 13;                                 \
    q2 = q1 + q6;                                                             \
    q1 = q1 - q6;                                                             \
                                                                              \
    /* pick up the results */                                                 \
    tmp0  = q4;                                                               \
    tmp1  = q5;                                                               \
    tmp2  = (tmp11_plus_tmp2 - tmp11_minus_tmp2) / 2;                         \
    tmp3  = q7;                                                               \
    tmp10 = q2;                                                               \
    tmp11 = (tmp11_plus_tmp2 + tmp11_minus_tmp2) / 2;                         \
    tmp12 = q3;                                                               \
    tmp13 = q1;                                                               \
}

#define XFIX_0_899976223                    v0.4h[0]
#define XFIX_0_541196100                    v0.4h[1]
#define XFIX_2_562915447                    v0.4h[2]
#define XFIX_0_298631336_MINUS_0_899976223  v0.4h[3]
#define XFIX_1_501321110_MINUS_0_899976223  v1.4h[0]
#define XFIX_2_053119869_MINUS_2_562915447  v1.4h[1]
#define XFIX_0_541196100_PLUS_0_765366865   v1.4h[2]
#define XFIX_1_175875602                    v1.4h[3]
#define XFIX_1_175875602_MINUS_0_390180644  v2.4h[0]
#define XFIX_0_541196100_MINUS_1_847759065  v2.4h[1]
#define XFIX_3_072711026_MINUS_2_562915447  v2.4h[2]
#define XFIX_1_175875602_MINUS_1_961570560  v2.4h[3]

.balign 16
jsimd_idct_islow_neon_consts:
    .short FIX_0_899976223                    /* d0[0] */
    .short FIX_0_541196100                    /* d0[1] */
    .short FIX_2_562915447                    /* d0[2] */
    .short FIX_0_298631336_MINUS_0_899976223  /* d0[3] */
    .short FIX_1_501321110_MINUS_0_899976223  /* d1[0] */
    .short FIX_2_053119869_MINUS_2_562915447  /* d1[1] */
    .short FIX_0_541196100_PLUS_0_765366865   /* d1[2] */
    .short FIX_1_175875602                    /* d1[3] */
    /* reloadable constants */
    .short FIX_1_175875602_MINUS_0_390180644  /* d2[0] */
    .short FIX_0_541196100_MINUS_1_847759065  /* d2[1] */
    .short FIX_3_072711026_MINUS_2_562915447  /* d2[2] */
    .short FIX_1_175875602_MINUS_1_961570560  /* d2[3] */

asm_function jsimd_idct_islow_neon

    DCT_TABLE       .req x0
    COEF_BLOCK      .req x1
    OUTPUT_BUF      .req x2
    OUTPUT_COL      .req x3
    TMP1            .req x0
    TMP2            .req x1
    TMP3            .req x2
    TMP4            .req x15

    ROW0L           .req v16
    ROW0R           .req v17
    ROW1L           .req v18
    ROW1R           .req v19
    ROW2L           .req v20
    ROW2R           .req v21
    ROW3L           .req v22
    ROW3R           .req v23
    ROW4L           .req v24
    ROW4R           .req v25
    ROW5L           .req v26
    ROW5R           .req v27
    ROW6L           .req v28
    ROW6R           .req v29
    ROW7L           .req v30
    ROW7R           .req v31
    /* Save all NEON registers and x15 (32 NEON registers * 8 bytes + 16) */
    sub             sp, sp, 272
    str             x15, [sp], 16
    adr             x15, jsimd_idct_islow_neon_consts
    st1             {v0.8b - v3.8b}, [sp], 32
    st1             {v4.8b - v7.8b}, [sp], 32
    st1             {v8.8b - v11.8b}, [sp], 32
    st1             {v12.8b - v15.8b}, [sp], 32
    st1             {v16.8b - v19.8b}, [sp], 32
    st1             {v20.8b - v23.8b}, [sp], 32
    st1             {v24.8b - v27.8b}, [sp], 32
    st1             {v28.8b - v31.8b}, [sp], 32
    ld1             {v16.4h, v17.4h, v18.4h, v19.4h}, [COEF_BLOCK], 32
    ld1             {v0.4h, v1.4h, v2.4h, v3.4h}, [DCT_TABLE], 32
    ld1             {v20.4h, v21.4h, v22.4h, v23.4h}, [COEF_BLOCK], 32
    mul             v16.4h, v16.4h, v0.4h
    mul             v17.4h, v17.4h, v1.4h
    ins             v16.2d[1], v17.2d[0]  /* 128 bit q8 */
    ld1             {v4.4h, v5.4h, v6.4h, v7.4h}, [DCT_TABLE], 32
    mul             v18.4h, v18.4h, v2.4h
    mul             v19.4h, v19.4h, v3.4h
    ins             v18.2d[1], v19.2d[0]  /* 128 bit q9 */
    ld1             {v24.4h, v25.4h, v26.4h, v27.4h}, [COEF_BLOCK], 32
    mul             v20.4h, v20.4h, v4.4h
    mul             v21.4h, v21.4h, v5.4h
    ins             v20.2d[1], v21.2d[0]  /* 128 bit q10 */
    ld1             {v0.4h, v1.4h, v2.4h, v3.4h}, [DCT_TABLE], 32
    mul             v22.4h, v22.4h, v6.4h
    mul             v23.4h, v23.4h, v7.4h
    ins             v22.2d[1], v23.2d[0]  /* 128 bit q11 */
    ld1             {v28.4h, v29.4h, v30.4h, v31.4h}, [COEF_BLOCK]
    mul             v24.4h, v24.4h, v0.4h
    mul             v25.4h, v25.4h, v1.4h
    ins             v24.2d[1], v25.2d[0]  /* 128 bit q12 */
    ld1             {v4.4h, v5.4h, v6.4h, v7.4h}, [DCT_TABLE], 32
    mul             v28.4h, v28.4h, v4.4h
    mul             v29.4h, v29.4h, v5.4h
    ins             v28.2d[1], v29.2d[0]  /* 128 bit q14 */
    mul             v26.4h, v26.4h, v2.4h
    mul             v27.4h, v27.4h, v3.4h
    ins             v26.2d[1], v27.2d[0]  /* 128 bit q13 */
    ld1             {v0.4h, v1.4h, v2.4h, v3.4h}, [x15]  /* load constants */
    add             x15, x15, #16
    mul             v30.4h, v30.4h, v6.4h
    mul             v31.4h, v31.4h, v7.4h
    ins             v30.2d[1], v31.2d[0]  /* 128 bit q15 */
    /* Go to the bottom of the stack */
    sub             sp, sp, 352
    stp             x4, x5, [sp], 16
    st1             {v8.4h - v11.4h}, [sp], 32  /* save NEON registers */
    st1             {v12.4h - v15.4h}, [sp], 32
    /* 1-D IDCT, pass 1, left 4x8 half */
    add             v4.4h,    ROW7L.4h, ROW3L.4h
    add             v5.4h,    ROW5L.4h, ROW1L.4h
    smull           v12.4s,   v4.4h,    XFIX_1_175875602_MINUS_1_961570560
    smlal           v12.4s,   v5.4h,    XFIX_1_175875602
    smull           v14.4s,   v4.4h,    XFIX_1_175875602
    /* Check for the zero coefficients in the right 4x8 half */
    smlal           v14.4s,   v5.4h,    XFIX_1_175875602_MINUS_0_390180644
    ssubl           v6.4s,    ROW0L.4h, ROW4L.4h
      ldp           w4,       w5,       [COEF_BLOCK, #(-96 + 2 * (4 + 1 * 8))]
    smull           v4.4s,    ROW2L.4h, XFIX_0_541196100
    smlal           v4.4s,    ROW6L.4h, XFIX_0_541196100_MINUS_1_847759065
      orr           x0,       x4,       x5
    mov             v8.16b,   v12.16b
    smlsl           v12.4s,   ROW5L.4h, XFIX_2_562915447
      ldp           w4,       w5,       [COEF_BLOCK, #(-96 + 2 * (4 + 2 * 8))]
    smlal           v12.4s,   ROW3L.4h, XFIX_3_072711026_MINUS_2_562915447
    shl             v6.4s,    v6.4s,    #13
      orr           x0,       x0,       x4
    smlsl           v8.4s,    ROW1L.4h, XFIX_0_899976223
      orr           x0,       x0 ,      x5
    add             v2.4s,    v6.4s,    v4.4s
      ldp           w4,       w5,       [COEF_BLOCK, #(-96 + 2 * (4 + 3 * 8))]
    mov             v10.16b,  v14.16b
    add             v2.4s,    v2.4s,    v12.4s
      orr           x0,       x0,       x4
    smlsl           v14.4s,   ROW7L.4h, XFIX_0_899976223
      orr           x0,       x0,       x5
    smlal           v14.4s,   ROW1L.4h, XFIX_1_501321110_MINUS_0_899976223
    rshrn           ROW1L.4h, v2.4s,    #11
      ldp           w4,       w5,       [COEF_BLOCK, #(-96 + 2 * (4 + 4 * 8))]
    sub             v2.4s,    v2.4s,    v12.4s
    smlal           v10.4s,   ROW5L.4h, XFIX_2_053119869_MINUS_2_562915447
      orr           x0,       x0,       x4
    smlsl           v10.4s,   ROW3L.4h, XFIX_2_562915447
      orr           x0,       x0,       x5
    sub             v2.4s,    v2.4s,    v12.4s
    smull           v12.4s,   ROW2L.4h, XFIX_0_541196100_PLUS_0_765366865
      ldp           w4,       w5,       [COEF_BLOCK, #(-96 + 2 * (4 + 5 * 8))]
    smlal           v12.4s,   ROW6L.4h, XFIX_0_541196100
    sub             v6.4s,    v6.4s,    v4.4s
      orr           x0,       x0,       x4
    rshrn           ROW6L.4h, v2.4s,    #11
      orr           x0,       x0,       x5
    add             v2.4s,    v6.4s,    v10.4s
      ldp           w4,       w5,       [COEF_BLOCK, #(-96 + 2 * (4 + 6 * 8))]
    sub             v6.4s,    v6.4s,    v10.4s
    saddl           v10.4s,   ROW0L.4h, ROW4L.4h
      orr           x0,       x0,       x4
    rshrn           ROW2L.4h, v2.4s,    #11
      orr           x0,       x0,       x5
    rshrn           ROW5L.4h, v6.4s,    #11
      ldp           w4,       w5,       [COEF_BLOCK, #(-96 + 2 * (4 + 7 * 8))]
    shl             v10.4s,   v10.4s,   #13
    smlal           v8.4s,    ROW7L.4h, XFIX_0_298631336_MINUS_0_899976223
      orr           x0,       x0,       x4
    add             v4.4s,    v10.4s,   v12.4s
      orr           x0,       x0,       x5
    cmp             x0, #0 /* orrs instruction removed */
    sub             v2.4s,    v10.4s,   v12.4s
    add             v12.4s,   v4.4s,    v14.4s
      ldp           w4,       w5,       [COEF_BLOCK, #(-96 + 2 * (4 + 0 * 8))]
    sub             v4.4s,    v4.4s,    v14.4s
    add             v10.4s,   v2.4s,    v8.4s
      orr           x0,       x4,       x5
    sub             v6.4s,    v2.4s,    v8.4s
      /* pop             {x4, x5} */
      sub           sp, sp, 80
      ldp           x4, x5, [sp], 16
    rshrn           ROW7L.4h, v4.4s,    #11
    rshrn           ROW3L.4h, v10.4s,   #11
    rshrn           ROW0L.4h, v12.4s,   #11
    rshrn           ROW4L.4h, v6.4s,    #11

      beq             3f /* Go to do some special handling for the sparse right 4x8 half */

    /* 1-D IDCT, pass 1, right 4x8 half */
    ld1             {v2.4h},  [x15]    /* reload constants */
    add             v10.4h,   ROW7R.4h, ROW3R.4h
    add             v8.4h,    ROW5R.4h, ROW1R.4h
    /* Transpose ROW6L <-> ROW7L   (v3 available free register) */
    transpose       ROW6L, ROW7L, v3, .16b, .4h
    smull           v12.4s,   v10.4h,   XFIX_1_175875602_MINUS_1_961570560
    smlal           v12.4s,   v8.4h,    XFIX_1_175875602
    /* Transpose ROW2L <-> ROW3L   (v3 available free register) */
    transpose       ROW2L, ROW3L, v3, .16b, .4h
    smull           v14.4s,   v10.4h,   XFIX_1_175875602
    smlal           v14.4s,   v8.4h,    XFIX_1_175875602_MINUS_0_390180644
    /* Transpose ROW0L <-> ROW1L   (v3 available free register) */
    transpose       ROW0L, ROW1L, v3, .16b, .4h
    ssubl           v6.4s,    ROW0R.4h, ROW4R.4h
    smull           v4.4s,    ROW2R.4h, XFIX_0_541196100
    smlal           v4.4s,    ROW6R.4h, XFIX_0_541196100_MINUS_1_847759065
    /* Transpose ROW4L <-> ROW5L   (v3 available free register) */
    transpose       ROW4L, ROW5L, v3, .16b, .4h
    mov             v8.16b,   v12.16b
    smlsl           v12.4s,   ROW5R.4h, XFIX_2_562915447
    smlal           v12.4s,   ROW3R.4h, XFIX_3_072711026_MINUS_2_562915447
    /* Transpose ROW1L <-> ROW3L   (v3 available free register) */
    transpose       ROW1L, ROW3L, v3, .16b, .2s
    shl             v6.4s,    v6.4s,    #13
    smlsl           v8.4s,    ROW1R.4h, XFIX_0_899976223
    /* Transpose ROW4L <-> ROW6L   (v3 available free register) */
    transpose       ROW4L, ROW6L, v3, .16b, .2s
    add             v2.4s,    v6.4s,    v4.4s
    mov             v10.16b,  v14.16b
    add             v2.4s,    v2.4s,    v12.4s
    /* Transpose ROW0L <-> ROW2L   (v3 available free register) */
    transpose       ROW0L, ROW2L, v3, .16b, .2s
    smlsl           v14.4s,   ROW7R.4h, XFIX_0_899976223
    smlal           v14.4s,   ROW1R.4h, XFIX_1_501321110_MINUS_0_899976223
    rshrn           ROW1R.4h, v2.4s,    #11
    /* Transpose ROW5L <-> ROW7L   (v3 available free register) */
    transpose       ROW5L, ROW7L, v3, .16b, .2s
    sub             v2.4s,    v2.4s,    v12.4s
    smlal           v10.4s,   ROW5R.4h, XFIX_2_053119869_MINUS_2_562915447
    smlsl           v10.4s,   ROW3R.4h, XFIX_2_562915447
    sub             v2.4s,    v2.4s,    v12.4s
    smull           v12.4s,   ROW2R.4h, XFIX_0_541196100_PLUS_0_765366865
    smlal           v12.4s,   ROW6R.4h, XFIX_0_541196100
    sub             v6.4s,    v6.4s,    v4.4s
    rshrn           ROW6R.4h, v2.4s,    #11
    add             v2.4s,    v6.4s,    v10.4s
    sub             v6.4s,    v6.4s,    v10.4s
    saddl           v10.4s,   ROW0R.4h, ROW4R.4h
    rshrn           ROW2R.4h, v2.4s,    #11
    rshrn           ROW5R.4h, v6.4s,    #11
    shl             v10.4s,   v10.4s,   #13
    smlal           v8.4s,    ROW7R.4h, XFIX_0_298631336_MINUS_0_899976223
    add             v4.4s,    v10.4s,   v12.4s
    sub             v2.4s,    v10.4s,   v12.4s
    add             v12.4s,   v4.4s,    v14.4s
    sub             v4.4s,    v4.4s,    v14.4s
    add             v10.4s,   v2.4s,    v8.4s
    sub             v6.4s,    v2.4s,    v8.4s
    rshrn           ROW7R.4h, v4.4s,    #11
    rshrn           ROW3R.4h, v10.4s,   #11
    rshrn           ROW0R.4h, v12.4s,   #11
    rshrn           ROW4R.4h, v6.4s,    #11
    /* Transpose right 4x8 half */
    transpose       ROW6R, ROW7R, v3, .16b, .4h
    transpose       ROW2R, ROW3R, v3, .16b, .4h
    transpose       ROW0R, ROW1R, v3, .16b, .4h
    transpose       ROW4R, ROW5R, v3, .16b, .4h
    transpose       ROW1R, ROW3R, v3, .16b, .2s
    transpose       ROW4R, ROW6R, v3, .16b, .2s
    transpose       ROW0R, ROW2R, v3, .16b, .2s
    transpose       ROW5R, ROW7R, v3, .16b, .2s

1:  /* 1-D IDCT, pass 2 (normal variant), left 4x8 half */
    ld1             {v2.4h},  [x15]    /* reload constants */
    smull           v12.4S,   ROW1R.4h, XFIX_1_175875602 /* ROW5L.4h <-> ROW1R.4h */
    smlal           v12.4s,   ROW1L.4h, XFIX_1_175875602
    smlal           v12.4s,   ROW3R.4h, XFIX_1_175875602_MINUS_1_961570560 /* ROW7L.4h <-> ROW3R.4h */
    smlal           v12.4s,   ROW3L.4h, XFIX_1_175875602_MINUS_1_961570560
    smull           v14.4s,   ROW3R.4h, XFIX_1_175875602 /* ROW7L.4h <-> ROW3R.4h */
    smlal           v14.4s,   ROW3L.4h, XFIX_1_175875602
    smlal           v14.4s,   ROW1R.4h, XFIX_1_175875602_MINUS_0_390180644 /* ROW5L.4h <-> ROW1R.4h */
    smlal           v14.4s,   ROW1L.4h, XFIX_1_175875602_MINUS_0_390180644
    ssubl           v6.4s,    ROW0L.4h, ROW0R.4h /* ROW4L.4h <-> ROW0R.4h */
    smull           v4.4s,    ROW2L.4h, XFIX_0_541196100
    smlal           v4.4s,    ROW2R.4h, XFIX_0_541196100_MINUS_1_847759065 /* ROW6L.4h <-> ROW2R.4h */
    mov             v8.16b,   v12.16b
    smlsl           v12.4s,   ROW1R.4h, XFIX_2_562915447 /* ROW5L.4h <-> ROW1R.4h */
    smlal           v12.4s,   ROW3L.4h, XFIX_3_072711026_MINUS_2_562915447
    shl             v6.4s,    v6.4s,    #13
    smlsl           v8.4s,    ROW1L.4h, XFIX_0_899976223
    add             v2.4s,    v6.4s,    v4.4s
    mov             v10.16b,  v14.16b
    add             v2.4s,    v2.4s,    v12.4s
    smlsl           v14.4s,   ROW3R.4h, XFIX_0_899976223 /* ROW7L.4h <-> ROW3R.4h */
    smlal           v14.4s,   ROW1L.4h, XFIX_1_501321110_MINUS_0_899976223
    shrn            ROW1L.4h, v2.4s,    #16
    sub             v2.4s,    v2.4s,    v12.4s
    smlal           v10.4s,   ROW1R.4h, XFIX_2_053119869_MINUS_2_562915447 /* ROW5L.4h <-> ROW1R.4h */
    smlsl           v10.4s,   ROW3L.4h, XFIX_2_562915447
    sub             v2.4s,    v2.4s,    v12.4s
    smull           v12.4s,   ROW2L.4h, XFIX_0_541196100_PLUS_0_765366865
    smlal           v12.4s,   ROW2R.4h, XFIX_0_541196100 /* ROW6L.4h <-> ROW2R.4h */
    sub             v6.4s,    v6.4s,    v4.4s
    shrn            ROW2R.4h, v2.4s,    #16 /* ROW6L.4h <-> ROW2R.4h */
    add             v2.4s,    v6.4s,    v10.4s
    sub             v6.4s,    v6.4s,    v10.4s
    saddl           v10.4s,   ROW0L.4h, ROW0R.4h /* ROW4L.4h <-> ROW0R.4h */
    shrn            ROW2L.4h, v2.4s,    #16
    shrn            ROW1R.4h, v6.4s,    #16 /* ROW5L.4h <-> ROW1R.4h */
    shl             v10.4s,   v10.4s,   #13
    smlal           v8.4s,    ROW3R.4h, XFIX_0_298631336_MINUS_0_899976223 /* ROW7L.4h <-> ROW3R.4h */
    add             v4.4s,    v10.4s,   v12.4s
    sub             v2.4s,    v10.4s,   v12.4s
    add             v12.4s,   v4.4s,    v14.4s
    sub             v4.4s,    v4.4s,    v14.4s
    add             v10.4s,   v2.4s,    v8.4s
    sub             v6.4s,    v2.4s,    v8.4s
    shrn            ROW3R.4h, v4.4s,    #16 /* ROW7L.4h <-> ROW3R.4h */
    shrn            ROW3L.4h, v10.4s,   #16
    shrn            ROW0L.4h, v12.4s,   #16
    shrn            ROW0R.4h, v6.4s,    #16 /* ROW4L.4h <-> ROW0R.4h */
    /* 1-D IDCT, pass 2, right 4x8 half */
    ld1             {v2.4h},  [x15]    /* reload constants */
    smull           v12.4s,   ROW5R.4h, XFIX_1_175875602
    smlal           v12.4s,   ROW5L.4h, XFIX_1_175875602 /* ROW5L.4h <-> ROW1R.4h */
    smlal           v12.4s,   ROW7R.4h, XFIX_1_175875602_MINUS_1_961570560
    smlal           v12.4s,   ROW7L.4h, XFIX_1_175875602_MINUS_1_961570560 /* ROW7L.4h <-> ROW3R.4h */
    smull           v14.4s,   ROW7R.4h, XFIX_1_175875602
    smlal           v14.4s,   ROW7L.4h, XFIX_1_175875602 /* ROW7L.4h <-> ROW3R.4h */
    smlal           v14.4s,   ROW5R.4h, XFIX_1_175875602_MINUS_0_390180644
    smlal           v14.4s,   ROW5L.4h, XFIX_1_175875602_MINUS_0_390180644 /* ROW5L.4h <-> ROW1R.4h */
    ssubl           v6.4s,    ROW4L.4h, ROW4R.4h /* ROW4L.4h <-> ROW0R.4h */
    smull           v4.4s,    ROW6L.4h, XFIX_0_541196100 /* ROW6L.4h <-> ROW2R.4h */
    smlal           v4.4s,    ROW6R.4h, XFIX_0_541196100_MINUS_1_847759065
    mov             v8.16b,   v12.16b
    smlsl           v12.4s,   ROW5R.4h, XFIX_2_562915447
    smlal           v12.4s,   ROW7L.4h, XFIX_3_072711026_MINUS_2_562915447 /* ROW7L.4h <-> ROW3R.4h */
    shl             v6.4s,    v6.4s,    #13
    smlsl           v8.4s,    ROW5L.4h, XFIX_0_899976223 /* ROW5L.4h <-> ROW1R.4h */
    add             v2.4s,    v6.4s,    v4.4s
    mov             v10.16b,  v14.16b
    add             v2.4s,    v2.4s,    v12.4s
    smlsl           v14.4s,   ROW7R.4h, XFIX_0_899976223
    smlal           v14.4s,   ROW5L.4h, XFIX_1_501321110_MINUS_0_899976223 /* ROW5L.4h <-> ROW1R.4h */
    shrn            ROW5L.4h, v2.4s,    #16 /* ROW5L.4h <-> ROW1R.4h */
    sub             v2.4s,    v2.4s,    v12.4s
    smlal           v10.4s,   ROW5R.4h, XFIX_2_053119869_MINUS_2_562915447
    smlsl           v10.4s,   ROW7L.4h, XFIX_2_562915447 /* ROW7L.4h <-> ROW3R.4h */
    sub             v2.4s,    v2.4s,    v12.4s
    smull           v12.4s,   ROW6L.4h, XFIX_0_541196100_PLUS_0_765366865 /* ROW6L.4h <-> ROW2R.4h */
    smlal           v12.4s,   ROW6R.4h, XFIX_0_541196100
    sub             v6.4s,    v6.4s,    v4.4s
    shrn            ROW6R.4h, v2.4s,    #16
    add             v2.4s,    v6.4s,    v10.4s
    sub             v6.4s,    v6.4s,    v10.4s
    saddl           v10.4s,   ROW4L.4h, ROW4R.4h /* ROW4L.4h <-> ROW0R.4h */
    shrn            ROW6L.4h, v2.4s,    #16 /* ROW6L.4h <-> ROW2R.4h */
    shrn            ROW5R.4h, v6.4s,    #16
    shl             v10.4s,   v10.4s,   #13
    smlal           v8.4s,    ROW7R.4h, XFIX_0_298631336_MINUS_0_899976223
    add             v4.4s,    v10.4s,   v12.4s
    sub             v2.4s,    v10.4s,   v12.4s
    add             v12.4s,   v4.4s,    v14.4s
    sub             v4.4s,    v4.4s,    v14.4s
    add             v10.4s,   v2.4s,    v8.4s
    sub             v6.4s,    v2.4s,    v8.4s
    shrn            ROW7R.4h, v4.4s,    #16
    shrn            ROW7L.4h, v10.4s,   #16 /* ROW7L.4h <-> ROW3R.4h */
    shrn            ROW4L.4h, v12.4s,   #16 /* ROW4L.4h <-> ROW0R.4h */
    shrn            ROW4R.4h, v6.4s,    #16

2:  /* Descale to 8-bit and range limit */
    ins             v16.2d[1], v17.2d[0]
    ins             v18.2d[1], v19.2d[0]
    ins             v20.2d[1], v21.2d[0]
    ins             v22.2d[1], v23.2d[0]
    sqrshrn         v16.8b,   v16.8h,   #2
    sqrshrn2        v16.16b,  v18.8h,   #2
    sqrshrn         v18.8b,   v20.8h,   #2
    sqrshrn2        v18.16b,  v22.8h,   #2

    /* vpop            {v8.4h - d15.4h} */ /* restore NEON registers */
    ld1             {v8.4h - v11.4h}, [sp], 32
    ld1             {v12.4h - v15.4h}, [sp], 32
    ins             v24.2d[1], v25.2d[0]

    sqrshrn         v20.8b,   v24.8h,   #2
      /* Transpose the final 8-bit samples and do signed->unsigned conversion */
    /* trn1            v16.8h,    v16.8h,  v18.8h */
    transpose       v16, v18, v3, .16b, .8h
    ins             v26.2d[1], v27.2d[0]
    ins             v28.2d[1], v29.2d[0]
    ins             v30.2d[1], v31.2d[0]
    sqrshrn2        v20.16b,  v26.8h,   #2
    sqrshrn         v22.8b,   v28.8h,   #2
    movi            v0.16b,   #(CENTERJSAMPLE)
    sqrshrn2        v22.16b,  v30.8h,   #2
    transpose_single v16, v17, v3, .2d, .8b
    transpose_single v18, v19, v3, .2d, .8b
    add             v16.8b,   v16.8b,   v0.8b
    add             v17.8b,   v17.8b,   v0.8b
    add             v18.8b,   v18.8b,   v0.8b
    add             v19.8b,   v19.8b,   v0.8b
    transpose       v20, v22, v3, .16b, .8h
    /* Store results to the output buffer */
    ldp             TMP1,     TMP2,     [OUTPUT_BUF], 16
    add             TMP1,     TMP1,     OUTPUT_COL
    add             TMP2,     TMP2,     OUTPUT_COL
    st1             {v16.8b}, [TMP1]
    transpose_single v20, v21, v3, .2d, .8b
    st1             {v17.8b}, [TMP2]
    ldp             TMP1,     TMP2,     [OUTPUT_BUF], 16
    add             TMP1,     TMP1,     OUTPUT_COL
    add             TMP2,     TMP2,     OUTPUT_COL
    st1             {v18.8b}, [TMP1]
    add             v20.8b,   v20.8b,   v0.8b
    add             v21.8b,   v21.8b,   v0.8b
    st1             {v19.8b}, [TMP2]
    ldp             TMP1,     TMP2,     [OUTPUT_BUF], 16
    ldp             TMP3,     TMP4,     [OUTPUT_BUF]
    add             TMP1,     TMP1,     OUTPUT_COL
    add             TMP2,     TMP2,     OUTPUT_COL
    add             TMP3,     TMP3,     OUTPUT_COL
    add             TMP4,     TMP4,     OUTPUT_COL
    transpose_single v22, v23, v3, .2d, .8b
    st1             {v20.8b}, [TMP1]
    add             v22.8b,   v22.8b,   v0.8b
    add             v23.8b,   v23.8b,   v0.8b
    st1             {v21.8b}, [TMP2]
    st1             {v22.8b}, [TMP3]
    st1             {v23.8b}, [TMP4]
    ldr             x15, [sp], 16
    ld1             {v0.8b - v3.8b}, [sp], 32
    ld1             {v4.8b - v7.8b}, [sp], 32
    ld1             {v8.8b - v11.8b}, [sp], 32
    ld1             {v12.8b - v15.8b}, [sp], 32
    ld1             {v16.8b - v19.8b}, [sp], 32
    ld1             {v20.8b - v23.8b}, [sp], 32
    ld1             {v24.8b - v27.8b}, [sp], 32
    ld1             {v28.8b - v31.8b}, [sp], 32
    blr             x30

3:  /* Left 4x8 half is done, right 4x8 half contains mostly zeros */

    /* Transpose left 4x8 half */
    transpose       ROW6L, ROW7L, v3, .16b, .4h
    transpose       ROW2L, ROW3L, v3, .16b, .4h
    transpose       ROW0L, ROW1L, v3, .16b, .4h
    transpose       ROW4L, ROW5L, v3, .16b, .4h
    shl             ROW0R.4h, ROW0R.4h, #2 /* PASS1_BITS */
    transpose       ROW1L, ROW3L, v3, .16b, .2s
    transpose       ROW4L, ROW6L, v3, .16b, .2s
    transpose       ROW0L, ROW2L, v3, .16b, .2s
    transpose       ROW5L, ROW7L, v3, .16b, .2s
    cmp             x0, #0
    beq             4f /* Right 4x8 half has all zeros, go to 'sparse' second pass */

    /* Only row 0 is non-zero for the right 4x8 half  */
    dup             ROW1R.4h, ROW0R.4h[1]
    dup             ROW2R.4h, ROW0R.4h[2]
    dup             ROW3R.4h, ROW0R.4h[3]
    dup             ROW4R.4h, ROW0R.4h[0]
    dup             ROW5R.4h, ROW0R.4h[1]
    dup             ROW6R.4h, ROW0R.4h[2]
    dup             ROW7R.4h, ROW0R.4h[3]
    dup             ROW0R.4h, ROW0R.4h[0]
    b               1b /* Go to 'normal' second pass */

4:  /* 1-D IDCT, pass 2 (sparse variant with zero rows 4-7), left 4x8 half */
    ld1             {v2.4h},  [x15]    /* reload constants */
    smull           v12.4s,   ROW1L.4h, XFIX_1_175875602
    smlal           v12.4s,   ROW3L.4h, XFIX_1_175875602_MINUS_1_961570560
    smull           v14.4s,   ROW3L.4h, XFIX_1_175875602
    smlal           v14.4s,   ROW1L.4h, XFIX_1_175875602_MINUS_0_390180644
    smull           v4.4s,    ROW2L.4h, XFIX_0_541196100
    sshll           v6.4s,    ROW0L.4h, #13
    mov             v8.16b,   v12.16b
    smlal           v12.4s,   ROW3L.4h, XFIX_3_072711026_MINUS_2_562915447
    smlsl           v8.4s,    ROW1L.4h, XFIX_0_899976223
    add             v2.4s,    v6.4s,    v4.4s
    mov             v10.16b,  v14.16b
    smlal           v14.4s,   ROW1L.4h, XFIX_1_501321110_MINUS_0_899976223
    add             v2.4s,    v2.4s,    v12.4s
    add             v12.4s,   v12.4s,   v12.4s
    smlsl           v10.4s,   ROW3L.4h, XFIX_2_562915447
    shrn            ROW1L.4h, v2.4s,    #16
    sub             v2.4s,    v2.4s,    v12.4s
    smull           v12.4s,   ROW2L.4h, XFIX_0_541196100_PLUS_0_765366865
    sub             v6.4s,    v6.4s,    v4.4s
    shrn            ROW2R.4h, v2.4s,    #16 /* ROW6L.4h <-> ROW2R.4h */
    add             v2.4s,    v6.4s,    v10.4s
    sub             v6.4s,    v6.4s,    v10.4s
    sshll           v10.4s,   ROW0L.4h, #13
    shrn            ROW2L.4h, v2.4s,    #16
    shrn            ROW1R.4h, v6.4s,    #16 /* ROW5L.4h <-> ROW1R.4h */
    add             v4.4s,    v10.4s,   v12.4s
    sub             v2.4s,    v10.4s,   v12.4s
    add             v12.4s,   v4.4s,    v14.4s
    sub             v4.4s,    v4.4s,    v14.4s
    add             v10.4s,   v2.4s,    v8.4s
    sub             v6.4s,    v2.4s,    v8.4s
    shrn            ROW3R.4h, v4.4s,    #16 /* ROW7L.4h <-> ROW3R.4h */
    shrn            ROW3L.4h, v10.4s,   #16
    shrn            ROW0L.4h, v12.4s,   #16
    shrn            ROW0R.4h, v6.4s,    #16 /* ROW4L.4h <-> ROW0R.4h */
    /* 1-D IDCT, pass 2 (sparse variant with zero rows 4-7), right 4x8 half */
    ld1             {v2.4h},  [x15]    /* reload constants */
    smull           v12.4s,   ROW5L.4h, XFIX_1_175875602
    smlal           v12.4s,   ROW7L.4h, XFIX_1_175875602_MINUS_1_961570560
    smull           v14.4s,   ROW7L.4h, XFIX_1_175875602
    smlal           v14.4s,   ROW5L.4h, XFIX_1_175875602_MINUS_0_390180644
    smull           v4.4s,    ROW6L.4h, XFIX_0_541196100
    sshll           v6.4s,    ROW4L.4h, #13
    mov             v8.16b,   v12.16b
    smlal           v12.4s,   ROW7L.4h, XFIX_3_072711026_MINUS_2_562915447
    smlsl           v8.4s,    ROW5L.4h, XFIX_0_899976223
    add             v2.4s,    v6.4s,    v4.4s
    mov             v10.16b,  v14.16b
    smlal           v14.4s,   ROW5L.4h, XFIX_1_501321110_MINUS_0_899976223
    add             v2.4s,    v2.4s,    v12.4s
    add             v12.4s,   v12.4s,   v12.4s
    smlsl           v10.4s,   ROW7L.4h, XFIX_2_562915447
    shrn            ROW5L.4h, v2.4s,    #16 /* ROW5L.4h <-> ROW1R.4h */
    sub             v2.4s,    v2.4s,    v12.4s
    smull           v12.4s,   ROW6L.4h, XFIX_0_541196100_PLUS_0_765366865
    sub             v6.4s,    v6.4s,    v4.4s
    shrn            ROW6R.4h, v2.4s,    #16
    add             v2.4s,    v6.4s,    v10.4s
    sub             v6.4s,    v6.4s,    v10.4s
    sshll           v10.4s,   ROW4L.4h, #13
    shrn            ROW6L.4h, v2.4s,    #16 /* ROW6L.4h <-> ROW2R.4h */
    shrn            ROW5R.4h, v6.4s,    #16
    add             v4.4s,    v10.4s,   v12.4s
    sub             v2.4s,    v10.4s,   v12.4s
    add             v12.4s,   v4.4s,    v14.4s
    sub             v4.4s,    v4.4s,    v14.4s
    add             v10.4s,   v2.4s,    v8.4s
    sub             v6.4s,    v2.4s,    v8.4s
    shrn            ROW7R.4h, v4.4s,    #16
    shrn            ROW7L.4h, v10.4s,   #16 /* ROW7L.4h <-> ROW3R.4h */
    shrn            ROW4L.4h, v12.4s,   #16 /* ROW4L.4h <-> ROW0R.4h */
    shrn            ROW4R.4h, v6.4s,    #16
    b               2b /* Go to epilogue */

    .unreq          DCT_TABLE
    .unreq          COEF_BLOCK
    .unreq          OUTPUT_BUF
    .unreq          OUTPUT_COL
    .unreq          TMP1
    .unreq          TMP2
    .unreq          TMP3
    .unreq          TMP4

    .unreq          ROW0L
    .unreq          ROW0R
    .unreq          ROW1L
    .unreq          ROW1R
    .unreq          ROW2L
    .unreq          ROW2R
    .unreq          ROW3L
    .unreq          ROW3R
    .unreq          ROW4L
    .unreq          ROW4R
    .unreq          ROW5L
    .unreq          ROW5R
    .unreq          ROW6L
    .unreq          ROW6R
    .unreq          ROW7L
    .unreq          ROW7R


/*****************************************************************************/

/*
 * jsimd_idct_ifast_neon
 *
 * This function contains a fast, not so accurate integer implementation of
 * the inverse DCT (Discrete Cosine Transform). It uses the same calculations
 * and produces exactly the same output as IJG's original 'jpeg_idct_ifast'
 * function from jidctfst.c
 *
 * Normally 1-D AAN DCT needs 5 multiplications and 29 additions.
 * But in ARM NEON case some extra additions are required because VQDMULH
 * instruction can't handle the constants larger than 1. So the expressions
 * like "x * 1.082392200" have to be converted to "x * 0.082392200 + x",
 * which introduces an extra addition. Overall, there are 6 extra additions
 * per 1-D IDCT pass, totalling to 5 VQDMULH and 35 VADD/VSUB instructions.
 */

#define XFIX_1_082392200 v0.4h[0]
#define XFIX_1_414213562 v0.4h[1]
#define XFIX_1_847759065 v0.4h[2]
#define XFIX_2_613125930 v0.4h[3]

.balign 16
jsimd_idct_ifast_neon_consts:
    .short (277 * 128 - 256 * 128) /* XFIX_1_082392200 */
    .short (362 * 128 - 256 * 128) /* XFIX_1_414213562 */
    .short (473 * 128 - 256 * 128) /* XFIX_1_847759065 */
    .short (669 * 128 - 512 * 128) /* XFIX_2_613125930 */

asm_function jsimd_idct_ifast_neon

    DCT_TABLE       .req x0
    COEF_BLOCK      .req x1
    OUTPUT_BUF      .req x2
    OUTPUT_COL      .req x3
    TMP1            .req x0
    TMP2            .req x1
    TMP3            .req x2
    TMP4            .req x22
    TMP5            .req x23

    /* Load and dequantize coefficients into NEON registers
     * with the following allocation:
     *       0 1 2 3 | 4 5 6 7
     *      ---------+--------
     *   0 | d16     | d17     ( v8.8h  )
     *   1 | d18     | d19     ( v9.8h  )
     *   2 | d20     | d21     ( v10.8h )
     *   3 | d22     | d23     ( v11.8h )
     *   4 | d24     | d25     ( v12.8h )
     *   5 | d26     | d27     ( v13.8h )
     *   6 | d28     | d29     ( v14.8h )
     *   7 | d30     | d31     ( v15.8h )
     */
    /* Save NEON registers used in fast IDCT */
    sub             sp, sp, #176
    stp             x22, x23, [sp], 16
    adr             x23, jsimd_idct_ifast_neon_consts
    st1             {v0.8b - v3.8b}, [sp], 32
    st1             {v4.8b - v7.8b}, [sp], 32
    st1             {v8.8b - v11.8b}, [sp], 32
    st1             {v12.8b - v15.8b}, [sp], 32
    st1             {v16.8b - v19.8b}, [sp], 32
    ld1             {v8.8h, v9.8h}, [COEF_BLOCK], 32
    ld1             {v0.8h, v1.8h}, [DCT_TABLE], 32
    ld1             {v10.8h, v11.8h}, [COEF_BLOCK], 32
    mul             v8.8h,  v8.8h,  v0.8h
    ld1             {v2.8h, v3.8h}, [DCT_TABLE], 32
    mul             v9.8h,  v9.8h,  v1.8h
    ld1             {v12.8h, v13.8h}, [COEF_BLOCK], 32
    mul             v10.8h, v10.8h, v2.8h
    ld1             {v0.8h, v1.8h}, [DCT_TABLE], 32
    mul             v11.8h, v11.8h, v3.8h
    ld1             {v14.8h, v15.8h}, [COEF_BLOCK], 32
    mul             v12.8h, v12.8h, v0.8h
    ld1             {v2.8h, v3.8h}, [DCT_TABLE], 32
    mul             v14.8h, v14.8h, v2.8h
    mul             v13.8h, v13.8h, v1.8h
    ld1             {v0.4h}, [x23]      /* load constants */
    mul             v15.8h, v15.8h, v3.8h

    /* 1-D IDCT, pass 1 */
    sub             v2.8h,    v10.8h,   v14.8h
    add             v14.8h,   v10.8h,   v14.8h
    sub             v1.8h,    v11.8h,   v13.8h
    add             v13.8h,   v11.8h,   v13.8h
    sub             v5.8h,    v9.8h,    v15.8h
    add             v15.8h,   v9.8h,    v15.8h
    sqdmulh         v4.8h,    v2.8h,    XFIX_1_414213562
    sqdmulh         v6.8h,    v1.8h,    XFIX_2_613125930
    add             v3.8h,    v1.8h,    v1.8h
    sub             v1.8h,    v5.8h,    v1.8h
    add             v10.8h,   v2.8h,    v4.8h
    sqdmulh         v4.8h,    v1.8h,    XFIX_1_847759065
    sub             v2.8h,    v15.8h,   v13.8h
    add             v3.8h,    v3.8h,    v6.8h
    sqdmulh         v6.8h,    v2.8h,    XFIX_1_414213562
    add             v1.8h,    v1.8h,    v4.8h
    sqdmulh         v4.8h,    v5.8h,    XFIX_1_082392200
    sub             v10.8h,   v10.8h,   v14.8h
    add             v2.8h,    v2.8h,    v6.8h
    sub             v6.8h,    v8.8h,    v12.8h
    add             v12.8h,   v8.8h,    v12.8h
    add             v9.8h,    v5.8h,    v4.8h
    add             v5.8h,    v6.8h,    v10.8h
    sub             v10.8h,   v6.8h,    v10.8h
    add             v6.8h,    v15.8h,   v13.8h
    add             v8.8h,    v12.8h,   v14.8h
    sub             v3.8h,    v6.8h,    v3.8h
    sub             v12.8h,   v12.8h,   v14.8h
    sub             v3.8h,    v3.8h,    v1.8h
    sub             v1.8h,    v9.8h,    v1.8h
    add             v2.8h,    v3.8h,    v2.8h
    sub             v15.8h,   v8.8h,    v6.8h
    add             v1.8h,    v1.8h,    v2.8h
    add             v8.8h,    v8.8h,    v6.8h
    add             v14.8h,   v5.8h,    v3.8h
    sub             v9.8h,    v5.8h,    v3.8h
    sub             v13.8h,   v10.8h,   v2.8h
    add             v10.8h,   v10.8h,   v2.8h
    /* Transpose  q8-q9 */
    mov             v18.16b,  v8.16b
    trn1            v8.8h,    v8.8h,    v9.8h
    trn2            v9.8h,    v18.8h,   v9.8h
    sub             v11.8h,   v12.8h,   v1.8h
    /* Transpose  q14-q15 */
    mov             v18.16b,  v14.16b
    trn1            v14.8h,   v14.8h,   v15.8h
    trn2            v15.8h,   v18.8h,   v15.8h
    add             v12.8h,   v12.8h,   v1.8h
    /* Transpose  q10-q11 */
    mov             v18.16b,  v10.16b
    trn1            v10.8h,   v10.8h,   v11.8h
    trn2            v11.8h,   v18.8h,   v11.8h
    /* Transpose  q12-q13 */
    mov             v18.16b,  v12.16b
    trn1            v12.8h,   v12.8h,   v13.8h
    trn2            v13.8h,   v18.8h,   v13.8h
    /* Transpose  q9-q11 */
    mov             v18.16b,  v9.16b
    trn1            v9.4s,    v9.4s,    v11.4s
    trn2            v11.4s,   v18.4s,   v11.4s
    /* Transpose  q12-q14 */
    mov             v18.16b,  v12.16b
    trn1            v12.4s,   v12.4s,   v14.4s
    trn2            v14.4s,   v18.4s,   v14.4s
    /* Transpose  q8-q10 */
    mov             v18.16b,  v8.16b
    trn1            v8.4s,    v8.4s,    v10.4s
    trn2            v10.4s,   v18.4s,   v10.4s
    /* Transpose  q13-q15 */
    mov             v18.16b,  v13.16b
    trn1            v13.4s,   v13.4s,   v15.4s
    trn2            v15.4s,   v18.4s,   v15.4s
    /* vswp            v14.4h,   v10-MSB.4h */
    umov            x22, v14.d[0]
    ins             v14.2d[0], v10.2d[1]
    ins             v10.2d[1], x22
    /* vswp            v13.4h,   v9MSB.4h */

    umov            x22, v13.d[0]
    ins             v13.2d[0], v9.2d[1]
    ins             v9.2d[1], x22
    /* 1-D IDCT, pass 2 */
    sub             v2.8h,    v10.8h,   v14.8h
    /* vswp            v15.4h,   v11MSB.4h */
    umov            x22, v15.d[0]
    ins             v15.2d[0], v11.2d[1]
    ins             v11.2d[1], x22
    add             v14.8h,   v10.8h,   v14.8h
    /* vswp            v12.4h,   v8-MSB.4h */
    umov            x22, v12.d[0]
    ins             v12.2d[0], v8.2d[1]
    ins             v8.2d[1], x22
    sub             v1.8h,    v11.8h,   v13.8h
    add             v13.8h,   v11.8h,   v13.8h
    sub             v5.8h,    v9.8h,    v15.8h
    add             v15.8h,   v9.8h,    v15.8h
    sqdmulh         v4.8h,    v2.8h,    XFIX_1_414213562
    sqdmulh         v6.8h,    v1.8h,    XFIX_2_613125930
    add             v3.8h,    v1.8h,    v1.8h
    sub             v1.8h,    v5.8h,    v1.8h
    add             v10.8h,   v2.8h,    v4.8h
    sqdmulh         v4.8h,    v1.8h,    XFIX_1_847759065
    sub             v2.8h,    v15.8h,   v13.8h
    add             v3.8h,    v3.8h,    v6.8h
    sqdmulh         v6.8h,    v2.8h,    XFIX_1_414213562
    add             v1.8h,    v1.8h,    v4.8h
    sqdmulh         v4.8h,    v5.8h,    XFIX_1_082392200
    sub             v10.8h,   v10.8h,   v14.8h
    add             v2.8h,    v2.8h,    v6.8h
    sub             v6.8h,    v8.8h,    v12.8h
    add             v12.8h,   v8.8h,    v12.8h
    add             v9.8h,    v5.8h,    v4.8h
    add             v5.8h,    v6.8h,    v10.8h
    sub             v10.8h,   v6.8h,    v10.8h
    add             v6.8h,    v15.8h,   v13.8h
    add             v8.8h,    v12.8h,   v14.8h
    sub             v3.8h,    v6.8h,    v3.8h
    sub             v12.8h,   v12.8h,   v14.8h
    sub             v3.8h,    v3.8h,    v1.8h
    sub             v1.8h,    v9.8h,    v1.8h
    add             v2.8h,    v3.8h,    v2.8h
    sub             v15.8h,   v8.8h,    v6.8h
    add             v1.8h,    v1.8h,    v2.8h
    add             v8.8h,    v8.8h,    v6.8h
    add             v14.8h,   v5.8h,    v3.8h
    sub             v9.8h,    v5.8h,    v3.8h
    sub             v13.8h,   v10.8h,   v2.8h
    add             v10.8h,   v10.8h,   v2.8h
    sub             v11.8h,   v12.8h,   v1.8h
    add             v12.8h,   v12.8h,   v1.8h
    /* Descale to 8-bit and range limit */
    movi            v0.16b,   #0x80
    sqshrn          v8.8b,    v8.8h,    #5
    sqshrn2         v8.16b,   v9.8h,    #5
    sqshrn          v9.8b,    v10.8h,   #5
    sqshrn2         v9.16b,   v11.8h,   #5
    sqshrn          v10.8b,   v12.8h,   #5
    sqshrn2         v10.16b,  v13.8h,   #5
    sqshrn          v11.8b,   v14.8h,   #5
    sqshrn2         v11.16b,  v15.8h,   #5
    add             v8.16b,   v8.16b,   v0.16b
    add             v9.16b,   v9.16b,   v0.16b
    add             v10.16b,  v10.16b,  v0.16b
    add             v11.16b,  v11.16b,  v0.16b
    /* Transpose the final 8-bit samples */
    /* Transpose  q8-q9 */
    mov             v18.16b,  v8.16b
    trn1            v8.8h,    v8.8h,    v9.8h
    trn2            v9.8h,    v18.8h,   v9.8h
    /* Transpose  q10-q11 */
    mov             v18.16b,  v10.16b
    trn1            v10.8h,   v10.8h,   v11.8h
    trn2            v11.8h,   v18.8h,   v11.8h
    /* Transpose  q8-q10 */
    mov             v18.16b,  v8.16b
    trn1            v8.4s,    v8.4s,    v10.4s
    trn2            v10.4s,   v18.4s,   v10.4s
    /* Transpose  q9-q11 */
    mov             v18.16b,  v9.16b
    trn1            v9.4s,    v9.4s,    v11.4s
    trn2            v11.4s,   v18.4s,   v11.4s
    /* make copy */
    ins             v17.2d[0], v8.2d[1]
    /* Transpose  d16-d17-msb */
    mov             v18.16b,  v8.16b
    trn1            v8.8b,    v8.8b,    v17.8b
    trn2            v17.8b,   v18.8b,   v17.8b
    /* make copy */
    ins             v19.2d[0], v9.2d[1]
    mov             v18.16b,  v9.16b
    trn1            v9.8b,    v9.8b,    v19.8b
    trn2            v19.8b,   v18.8b,   v19.8b
    /* Store results to the output buffer */
    ldp             TMP1,     TMP2,     [OUTPUT_BUF], 16
    add             TMP1,     TMP1,     OUTPUT_COL
    add             TMP2,     TMP2,     OUTPUT_COL
    st1             {v8.8b},  [TMP1]
    st1             {v17.8b}, [TMP2]
    ldp             TMP1,     TMP2,     [OUTPUT_BUF], 16
    add             TMP1,     TMP1,     OUTPUT_COL
    add             TMP2,     TMP2,     OUTPUT_COL
    st1             {v9.8b},  [TMP1]
    /* make copy */
    ins             v7.2d[0], v10.2d[1]
    mov             v18.16b,  v10.16b
    trn1            v10.8b,   v10.8b,   v7.8b
    trn2            v7.8b,    v18.8b,   v7.8b
    st1             {v19.8b}, [TMP2]
    ldp             TMP1,     TMP2,     [OUTPUT_BUF], 16
    ldp             TMP4,     TMP5,     [OUTPUT_BUF], 16
    add             TMP1,     TMP1,     OUTPUT_COL
    add             TMP2,     TMP2,     OUTPUT_COL
    add             TMP4,     TMP4,     OUTPUT_COL
    add             TMP5,     TMP5,     OUTPUT_COL
    st1             {v10.8b}, [TMP1]
    /* make copy */
    ins             v16.2d[0], v11.2d[1]
    mov             v18.16b,  v11.16b
    trn1            v11.8b,   v11.8b,   v16.8b
    trn2            v16.8b,   v18.8b,   v16.8b
    st1             {v7.8b},  [TMP2]
    st1             {v11.8b}, [TMP4]
    st1             {v16.8b}, [TMP5]
    sub             sp, sp, #176
    ldp             x22, x23, [sp], 16
    ld1             {v0.8b - v3.8b}, [sp], 32
    ld1             {v4.8b - v7.8b}, [sp], 32
    ld1             {v8.8b - v11.8b}, [sp], 32
    ld1             {v12.8b - v15.8b}, [sp], 32
    ld1             {v16.8b - v19.8b}, [sp], 32
    blr             x30

    .unreq          DCT_TABLE
    .unreq          COEF_BLOCK
    .unreq          OUTPUT_BUF
    .unreq          OUTPUT_COL
    .unreq          TMP1
    .unreq          TMP2
    .unreq          TMP3
    .unreq          TMP4


/*****************************************************************************/

/*
 * jsimd_idct_4x4_neon
 *
 * This function contains inverse-DCT code for getting reduced-size
 * 4x4 pixels output from an 8x8 DCT block. It uses the same  calculations
 * and produces exactly the same output as IJG's original 'jpeg_idct_4x4'
 * function from jpeg-6b (jidctred.c).
 *
 * NOTE: jpeg-8 has an improved implementation of 4x4 inverse-DCT, which
 *       requires much less arithmetic operations and hence should be faster.
 *       The primary purpose of this particular NEON optimized function is
 *       bit exact compatibility with jpeg-6b.
 *
 * TODO: a bit better instructions scheduling can be achieved by expanding
 *       idct_helper/transpose_4x4 macros and reordering instructions,
 *       but readability will suffer somewhat.
 */

#define CONST_BITS  13

#define FIX_0_211164243  (1730)  /* FIX(0.211164243) */
#define FIX_0_509795579  (4176)  /* FIX(0.509795579) */
#define FIX_0_601344887  (4926)  /* FIX(0.601344887) */
#define FIX_0_720959822  (5906)  /* FIX(0.720959822) */
#define FIX_0_765366865  (6270)  /* FIX(0.765366865) */
#define FIX_0_850430095  (6967)  /* FIX(0.850430095) */
#define FIX_0_899976223  (7373)  /* FIX(0.899976223) */
#define FIX_1_061594337  (8697)  /* FIX(1.061594337) */
#define FIX_1_272758580  (10426) /* FIX(1.272758580) */
#define FIX_1_451774981  (11893) /* FIX(1.451774981) */
#define FIX_1_847759065  (15137) /* FIX(1.847759065) */
#define FIX_2_172734803  (17799) /* FIX(2.172734803) */
#define FIX_2_562915447  (20995) /* FIX(2.562915447) */
#define FIX_3_624509785  (29692) /* FIX(3.624509785) */

.balign 16
jsimd_idct_4x4_neon_consts:
    .short     FIX_1_847759065     /* v0.4h[0] */
    .short     -FIX_0_765366865    /* v0.4h[1] */
    .short     -FIX_0_211164243    /* v0.4h[2] */
    .short     FIX_1_451774981     /* v0.4h[3] */
    .short     -FIX_2_172734803    /* d1[0] */
    .short     FIX_1_061594337     /* d1[1] */
    .short     -FIX_0_509795579    /* d1[2] */
    .short     -FIX_0_601344887    /* d1[3] */
    .short     FIX_0_899976223     /* v2.4h[0] */
    .short     FIX_2_562915447     /* v2.4h[1] */
    .short     1 << (CONST_BITS+1) /* v2.4h[2] */
    .short     0                   /* v2.4h[3] */

.macro idct_helper x4, x6, x8, x10, x12, x14, x16, shift, y26, y27, y28, y29
    smull           v28.4s, \x4,    v2.4h[2]
    smlal           v28.4s, \x8,    v0.4h[0]
    smlal           v28.4s, \x14,   v0.4h[1]

    smull           v26.4s, \x16,   v1.4h[2]
    smlal           v26.4s, \x12,   v1.4h[3]
    smlal           v26.4s, \x10,   v2.4h[0]
    smlal           v26.4s, \x6,    v2.4h[1]

    smull           v30.4s, \x4,    v2.4h[2]
    smlsl           v30.4s, \x8,    v0.4h[0]
    smlsl           v30.4s, \x14,   v0.4h[1]

    smull           v24.4s, \x16,   v0.4h[2]
    smlal           v24.4s, \x12,   v0.4h[3]
    smlal           v24.4s, \x10,   v1.4h[0]
    smlal           v24.4s, \x6,    v1.4h[1]

    add             v20.4s, v28.4s, v26.4s
    sub             v28.4s, v28.4s, v26.4s

.if \shift > 16
    srshr           v20.4s, v20.4s, #\shift
    srshr           v28.4s, v28.4s, #\shift
    xtn             \y26,   v20.4s
    xtn             \y29,   v28.4s
.else
    rshrn           \y26,   v20.4s, #\shift
    rshrn           \y29,   v28.4s, #\shift
.endif

    add             v20.4s, v30.4s, v24.4s
    sub             v30.4s, v30.4s, v24.4s

.if \shift > 16
    srshr           v20.4s, v20.4s, #\shift
    srshr           v30.4s, v30.4s, #\shift
    xtn             \y27,   v20.4s
    xtn             \y28,   v30.4s
.else
    rshrn           \y27,   v20.4s, #\shift
    rshrn           \y28,   v30.4s, #\shift
.endif

.endm

asm_function jsimd_idct_4x4_neon

    DCT_TABLE       .req x0
    COEF_BLOCK      .req x1
    OUTPUT_BUF      .req x2
    OUTPUT_COL      .req x3
    TMP1            .req x0
    TMP2            .req x1
    TMP3            .req x2
    TMP4            .req x15

    /* Save all used NEON registers */
    sub             sp, sp, 272
    str             x15, [sp], 16
    /* Load constants (v3.4h is just used for padding) */
    adr             TMP4, jsimd_idct_4x4_neon_consts
    st1             {v0.8b - v3.8b}, [sp], 32
    st1             {v4.8b - v7.8b}, [sp], 32
    st1             {v8.8b - v11.8b}, [sp], 32
    st1             {v12.8b - v15.8b}, [sp], 32
    st1             {v16.8b - v19.8b}, [sp], 32
    st1             {v20.8b - v23.8b}, [sp], 32
    st1             {v24.8b - v27.8b}, [sp], 32
    st1             {v28.8b - v31.8b}, [sp], 32
    ld1             {v0.4h, v1.4h, v2.4h, v3.4h}, [TMP4]

    /* Load all COEF_BLOCK into NEON registers with the following allocation:
     *       0 1 2 3 | 4 5 6 7
     *      ---------+--------
     *   0 | v4.4h   | v5.4h
     *   1 | v6.4h   | v7.4h
     *   2 | v8.4h   | v9.4h
     *   3 | v10.4h  | v11.4h
     *   4 | -       | -
     *   5 | v12.4h  | v13.4h
     *   6 | v14.4h  | v15.4h
     *   7 | v16.4h  | v17.4h
     */
    ld1             {v4.4h, v5.4h, v6.4h, v7.4h}, [COEF_BLOCK], 32
    ld1             {v8.4h, v9.4h, v10.4h, v11.4h}, [COEF_BLOCK], 32
    add             COEF_BLOCK, COEF_BLOCK, #16
    ld1             {v12.4h, v13.4h, v14.4h, v15.4h}, [COEF_BLOCK], 32
    ld1             {v16.4h, v17.4h}, [COEF_BLOCK], 16
    /* dequantize */
    ld1             {v18.4h, v19.4h, v20.4h, v21.4h}, [DCT_TABLE], 32
    mul             v4.4h, v4.4h, v18.4h
    mul             v5.4h, v5.4h, v19.4h
    ins             v4.2d[1], v5.2d[0]    /* 128 bit q4 */
    ld1             {v22.4h, v23.4h, v24.4h, v25.4h}, [DCT_TABLE], 32
    mul             v6.4h, v6.4h, v20.4h
    mul             v7.4h, v7.4h, v21.4h
    ins             v6.2d[1], v7.2d[0]    /* 128 bit q6 */
    mul             v8.4h, v8.4h, v22.4h
    mul             v9.4h, v9.4h, v23.4h
    ins             v8.2d[1], v9.2d[0]    /* 128 bit q8 */
    add             DCT_TABLE, DCT_TABLE, #16
    ld1             {v26.4h, v27.4h, v28.4h, v29.4h}, [DCT_TABLE], 32
    mul             v10.4h, v10.4h, v24.4h
    mul             v11.4h, v11.4h, v25.4h
    ins             v10.2d[1], v11.2d[0]  /* 128 bit q10 */
    mul             v12.4h, v12.4h, v26.4h
    mul             v13.4h, v13.4h, v27.4h
    ins             v12.2d[1], v13.2d[0]  /* 128 bit q12 */
    ld1             {v30.4h, v31.4h}, [DCT_TABLE], 16
    mul             v14.4h, v14.4h, v28.4h
    mul             v15.4h, v15.4h, v29.4h
    ins             v14.2d[1], v15.2d[0]  /* 128 bit q14 */
    mul             v16.4h, v16.4h, v30.4h
    mul             v17.4h, v17.4h, v31.4h
    ins             v16.2d[1], v17.2d[0]  /* 128 bit q16 */

    /* Pass 1 */
    idct_helper     v4.4h, v6.4h, v8.4h, v10.4h, v12.4h, v14.4h, v16.4h, 12, v4.4h, v6.4h, v8.4h, v10.4h
    transpose_4x4   v4, v6, v8, v10, v3
    ins             v10.2d[1], v11.2d[0]
    idct_helper     v5.4h, v7.4h, v9.4h, v11.4h, v13.4h, v15.4h, v17.4h, 12, v5.4h, v7.4h, v9.4h, v11.4h
    transpose_4x4   v5, v7, v9, v11, v3
    ins             v10.2d[1], v11.2d[0]
    /* Pass 2 */
    idct_helper     v4.4h, v6.4h, v8.4h, v10.4h, v7.4h, v9.4h, v11.4h, 19, v26.4h, v27.4h, v28.4h, v29.4h
    transpose_4x4   v26, v27, v28, v29, v3

    /* Range limit */
    movi            v30.8h, #0x80
    ins             v26.2d[1], v27.2d[0]
    ins             v28.2d[1], v29.2d[0]
    add             v26.8h, v26.8h, v30.8h
    add             v28.8h, v28.8h, v30.8h
    sqxtun          v26.8b, v26.8h
    sqxtun          v27.8b, v28.8h

    /* Store results to the output buffer */
    ldp             TMP1, TMP2, [OUTPUT_BUF], 16
    ldp             TMP3, TMP4, [OUTPUT_BUF]
    add             TMP1, TMP1, OUTPUT_COL
    add             TMP2, TMP2, OUTPUT_COL
    add             TMP3, TMP3, OUTPUT_COL
    add             TMP4, TMP4, OUTPUT_COL

#if defined(__ARMEL__) && !RESPECT_STRICT_ALIGNMENT
    /* We can use much less instructions on little endian systems if the
     * OS kernel is not configured to trap unaligned memory accesses
     */
    st1             {v26.s}[0], [TMP1], 4
    st1             {v27.s}[0], [TMP3], 4
    st1             {v26.s}[1], [TMP2], 4
    st1             {v27.s}[1], [TMP4], 4
#else
    st1             {v26.b}[0], [TMP1], 1
    st1             {v27.b}[0], [TMP3], 1
    st1             {v26.b}[1], [TMP1], 1
    st1             {v27.b}[1], [TMP3], 1
    st1             {v26.b}[2], [TMP1], 1
    st1             {v27.b}[2], [TMP3], 1
    st1             {v26.b}[3], [TMP1], 1
    st1             {v27.b}[3], [TMP3], 1

    st1             {v26.b}[4], [TMP2], 1
    st1             {v27.b}[4], [TMP4], 1
    st1             {v26.b}[5], [TMP2], 1
    st1             {v27.b}[5], [TMP4], 1
    st1             {v26.b}[6], [TMP2], 1
    st1             {v27.b}[6], [TMP4], 1
    st1             {v26.b}[7], [TMP2], 1
    st1             {v27.b}[7], [TMP4], 1
#endif

    /* vpop            {v8.4h - v15.4h}    ;not available */
    sub             sp, sp, #272
    ldr             x15, [sp], 16
    ld1             {v0.8b - v3.8b}, [sp], 32
    ld1             {v4.8b - v7.8b}, [sp], 32
    ld1             {v8.8b - v11.8b}, [sp], 32
    ld1             {v12.8b - v15.8b}, [sp], 32
    ld1             {v16.8b - v19.8b}, [sp], 32
    ld1             {v20.8b - v23.8b}, [sp], 32
    ld1             {v24.8b - v27.8b}, [sp], 32
    ld1             {v28.8b - v31.8b}, [sp], 32
    blr             x30

    .unreq          DCT_TABLE
    .unreq          COEF_BLOCK
    .unreq          OUTPUT_BUF
    .unreq          OUTPUT_COL
    .unreq          TMP1
    .unreq          TMP2
    .unreq          TMP3
    .unreq          TMP4

.purgem idct_helper


/*****************************************************************************/

/*
 * jsimd_idct_2x2_neon
 *
 * This function contains inverse-DCT code for getting reduced-size
 * 2x2 pixels output from an 8x8 DCT block. It uses the same  calculations
 * and produces exactly the same output as IJG's original 'jpeg_idct_2x2'
 * function from jpeg-6b (jidctred.c).
 *
 * NOTE: jpeg-8 has an improved implementation of 2x2 inverse-DCT, which
 *       requires much less arithmetic operations and hence should be faster.
 *       The primary purpose of this particular NEON optimized function is
 *       bit exact compatibility with jpeg-6b.
 */

.balign 8
jsimd_idct_2x2_neon_consts:
    .short     -FIX_0_720959822    /* v14[0] */
    .short     FIX_0_850430095     /* v14[1] */
    .short     -FIX_1_272758580    /* v14[2] */
    .short     FIX_3_624509785     /* v14[3] */

.macro idct_helper x4, x6, x10, x12, x16, shift, y26, y27
    sshll      v15.4s, \x4,    #15
    smull      v26.4s, \x6,    v14.4h[3]
    smlal      v26.4s, \x10,   v14.4h[2]
    smlal      v26.4s, \x12,   v14.4h[1]
    smlal      v26.4s, \x16,   v14.4h[0]

    add        v20.4s, v15.4s, v26.4s
    sub        v15.4s, v15.4s, v26.4s

.if \shift > 16
    srshr      v20.4s, v20.4s, #\shift
    srshr      v15.4s, v15.4s, #\shift
    xtn        \y26,   v20.4s
    xtn        \y27,   v15.4s
.else
    rshrn      \y26,   v20.4s, #\shift
    rshrn      \y27,   v15.4s, #\shift
.endif

.endm

asm_function jsimd_idct_2x2_neon

    DCT_TABLE       .req x0
    COEF_BLOCK      .req x1
    OUTPUT_BUF      .req x2
    OUTPUT_COL      .req x3
    TMP1            .req x0
    TMP2            .req x15

    /* vpush           {v8.4h - v15.4h}            ; not available */
    sub             sp, sp, 208
    str             x15, [sp], 16

    /* Load constants */
    adr             TMP2, jsimd_idct_2x2_neon_consts
    st1             {v4.8b - v7.8b}, [sp], 32
    st1             {v8.8b - v11.8b}, [sp], 32
    st1             {v12.8b - v15.8b}, [sp], 32
    st1             {v16.8b - v19.8b}, [sp], 32
    st1             {v21.8b - v22.8b}, [sp], 16
    st1             {v24.8b - v27.8b}, [sp], 32
    st1             {v30.8b - v31.8b}, [sp], 16
    ld1             {v14.4h}, [TMP2]

    /* Load all COEF_BLOCK into NEON registers with the following allocation:
     *       0 1 2 3 | 4 5 6 7
     *      ---------+--------
     *   0 | v4.4h   | v5.4h
     *   1 | v6.4h   | v7.4h
     *   2 | -       | -
     *   3 | v10.4h  | v11.4h
     *   4 | -       | -
     *   5 | v12.4h  | v13.4h
     *   6 | -       | -
     *   7 | v16.4h  | v17.4h
     */
    ld1             {v4.4h, v5.4h, v6.4h, v7.4h}, [COEF_BLOCK], 32
    add             COEF_BLOCK, COEF_BLOCK, #16
    ld1             {v10.4h, v11.4h}, [COEF_BLOCK], 16
    add             COEF_BLOCK, COEF_BLOCK, #16
    ld1             {v12.4h, v13.4h}, [COEF_BLOCK], 16
    add             COEF_BLOCK, COEF_BLOCK, #16
    ld1             {v16.4h, v17.4h}, [COEF_BLOCK], 16
    /* Dequantize */
    ld1             {v18.4h, v19.4h, v20.4h, v21.4h}, [DCT_TABLE], 32
    mul             v4.4h, v4.4h, v18.4h
    mul             v5.4h, v5.4h, v19.4h
    ins             v4.2d[1], v5.2d[0]
    mul             v6.4h, v6.4h, v20.4h
    mul             v7.4h, v7.4h, v21.4h
    ins             v6.2d[1], v7.2d[0]
    add             DCT_TABLE, DCT_TABLE, #16
    ld1             {v24.4h, v25.4h}, [DCT_TABLE], 16
    mul             v10.4h, v10.4h, v24.4h
    mul             v11.4h, v11.4h, v25.4h
    ins             v10.2d[1], v11.2d[0]
    add             DCT_TABLE, DCT_TABLE, #16
    ld1             {v26.4h, v27.4h}, [DCT_TABLE], 16
    mul             v12.4h, v12.4h, v26.4h
    mul             v13.4h, v13.4h, v27.4h
    ins             v12.2d[1], v13.2d[0]
    add             DCT_TABLE, DCT_TABLE, #16
    ld1             {v30.4h, v31.4h}, [DCT_TABLE], 16
    mul             v16.4h, v16.4h, v30.4h
    mul             v17.4h, v17.4h, v31.4h
    ins             v16.2d[1], v17.2d[0]

    /* Pass 1 */
#if 0
    idct_helper     v4.4h, v6.4h, v10.4h, v12.4h, v16.4h, 13, v4.4h, v6.4h
    transpose_4x4   v4.4h, v6.4h, v8.4h,  v10.4h
    idct_helper     v5.4h, v7.4h, v11.4h, v13.4h, v17.4h, 13, v5.4h, v7.4h
    transpose_4x4   v5.4h, v7.4h, v9.4h,  v11.4h
#else
    smull           v26.4s, v6.4h,  v14.4h[3]
    smlal           v26.4s, v10.4h, v14.4h[2]
    smlal           v26.4s, v12.4h, v14.4h[1]
    smlal           v26.4s, v16.4h, v14.4h[0]
    smull           v24.4s, v7.4h,  v14.4h[3]
    smlal           v24.4s, v11.4h, v14.4h[2]
    smlal           v24.4s, v13.4h, v14.4h[1]
    smlal           v24.4s, v17.4h, v14.4h[0]
    sshll           v15.4s, v4.4h,  #15
    sshll           v30.4s, v5.4h,  #15
    add             v20.4s, v15.4s, v26.4s
    sub             v15.4s, v15.4s, v26.4s
    rshrn           v4.4h,  v20.4s, #13
    rshrn           v6.4h,  v15.4s, #13
    add             v20.4s, v30.4s, v24.4s
    sub             v15.4s, v30.4s, v24.4s
    rshrn           v5.4h,  v20.4s, #13
    rshrn           v7.4h,  v15.4s, #13
    ins             v4.2d[1], v5.2d[0]
    ins             v6.2d[1], v7.2d[0]
    transpose       v4, v6, v3, .16b, .8h
    transpose       v6, v10, v3, .16b, .4s
    ins             v11.2d[0], v10.2d[1]
    ins             v7.2d[0], v6.2d[1]
#endif

    /* Pass 2 */
    idct_helper     v4.4h, v6.4h, v10.4h, v7.4h, v11.4h, 20, v26.4h, v27.4h

    /* Range limit */
    movi            v30.8h, #0x80
    ins             v26.2d[1], v27.2d[0]
    add             v26.8h, v26.8h, v30.8h
    sqxtun          v30.8b, v26.8h
    ins             v26.2d[0], v30.2d[0]
    sqxtun          v27.8b, v26.8h

    /* Store results to the output buffer */
    ldp             TMP1, TMP2, [OUTPUT_BUF]
    add             TMP1, TMP1, OUTPUT_COL
    add             TMP2, TMP2, OUTPUT_COL

    st1             {v26.b}[0], [TMP1], 1
    st1             {v27.b}[4], [TMP1], 1
    st1             {v26.b}[1], [TMP2], 1
    st1             {v27.b}[5], [TMP2], 1

    sub             sp, sp, #208
    ldr             x15, [sp], 16
    ld1             {v4.8b - v7.8b}, [sp], 32
    ld1             {v8.8b - v11.8b}, [sp], 32
    ld1             {v12.8b - v15.8b}, [sp], 32
    ld1             {v16.8b - v19.8b}, [sp], 32
    ld1             {v21.8b - v22.8b}, [sp], 16
    ld1             {v24.8b - v27.8b}, [sp], 32
    ld1             {v30.8b - v31.8b}, [sp], 16
    blr             x30

    .unreq          DCT_TABLE
    .unreq          COEF_BLOCK
    .unreq          OUTPUT_BUF
    .unreq          OUTPUT_COL
    .unreq          TMP1
    .unreq          TMP2

.purgem idct_helper


/*****************************************************************************/

/*
 * jsimd_ycc_extrgb_convert_neon
 * jsimd_ycc_extbgr_convert_neon
 * jsimd_ycc_extrgbx_convert_neon
 * jsimd_ycc_extbgrx_convert_neon
 * jsimd_ycc_extxbgr_convert_neon
 * jsimd_ycc_extxrgb_convert_neon
 *
 * Colorspace conversion YCbCr -> RGB
 */


.macro do_load size
    .if \size == 8
        ld1  {v4.8b}, [U], 8
        ld1  {v5.8b}, [V], 8
        ld1  {v0.8b}, [Y], 8
        prfm PLDL1KEEP, [U, #64]
        prfm PLDL1KEEP, [V, #64]
        prfm PLDL1KEEP, [Y, #64]
    .elseif \size == 4
        ld1  {v4.b}[0], [U], 1
        ld1  {v4.b}[1], [U], 1
        ld1  {v4.b}[2], [U], 1
        ld1  {v4.b}[3], [U], 1
        ld1  {v5.b}[0], [V], 1
        ld1  {v5.b}[1], [V], 1
        ld1  {v5.b}[2], [V], 1
        ld1  {v5.b}[3], [V], 1
        ld1  {v0.b}[0], [Y], 1
        ld1  {v0.b}[1], [Y], 1
        ld1  {v0.b}[2], [Y], 1
        ld1  {v0.b}[3], [Y], 1
    .elseif \size == 2
        ld1  {v4.b}[4], [U], 1
        ld1  {v4.b}[5], [U], 1
        ld1  {v5.b}[4], [V], 1
        ld1  {v5.b}[5], [V], 1
        ld1  {v0.b}[4], [Y], 1
        ld1  {v0.b}[5], [Y], 1
    .elseif \size == 1
        ld1  {v4.b}[6], [U], 1
        ld1  {v5.b}[6], [V], 1
        ld1  {v0.b}[6], [Y], 1
    .else
        .error unsupported macroblock size
    .endif
.endm

.macro do_store bpp, size
    .if \bpp == 24
        .if \size == 8
            st3  {v10.8b, v11.8b, v12.8b}, [RGB], 24
        .elseif \size == 4
            st3  {v10.b, v11.b, v12.b}[0], [RGB], 3
            st3  {v10.b, v11.b, v12.b}[1], [RGB], 3
            st3  {v10.b, v11.b, v12.b}[2], [RGB], 3
            st3  {v10.b, v11.b, v12.b}[3], [RGB], 3
        .elseif \size == 2
            st3  {v10.b, v11.b, v12.b}[4], [RGB], 3
            st3  {v10.b, v11.b, v12.b}[5], [RGB], 3
        .elseif \size == 1
            st3  {v10.b, v11.b, v12.b}[6], [RGB], 3
        .else
            .error unsupported macroblock size
        .endif
    .elseif \bpp == 32
        .if \size == 8
            st4  {v10.8b, v11.8b, v12.8b, v13.8b}, [RGB], 32
        .elseif \size == 4
            st4  {v10.b, v11.b, v12.b, v13.b}[0], [RGB], 4
            st4  {v10.b, v11.b, v12.b, v13.b}[1], [RGB], 4
            st4  {v10.b, v11.b, v12.b, v13.b}[2], [RGB], 4
            st4  {v10.b, v11.b, v12.b, v13.b}[3], [RGB], 4
        .elseif \size == 2
            st4  {v10.b, v11.b, v12.b, v13.b}[4], [RGB], 4
            st4  {v10.b, v11.b, v12.b, v13.b}[5], [RGB], 4
        .elseif \size == 1
            st4  {v10.b, v11.b, v12.b, v13.b}[6], [RGB], 4
        .else
            .error unsupported macroblock size
        .endif
    .elseif \bpp==16
        .if \size == 8
            st1  {v25.8h}, [RGB],16
        .elseif \size == 4
            st1  {v25.4h}, [RGB],8
        .elseif \size == 2
            st1  {v25.h}[4], [RGB],2
            st1  {v25.h}[5], [RGB],2
        .elseif \size == 1
            st1  {v25.h}[6], [RGB],2
        .else
            .error unsupported macroblock size
        .endif
     .else
        .error unsupported bpp
    .endif
.endm

.macro generate_jsimd_ycc_rgb_convert_neon colorid, bpp, r_offs, rsize, g_offs, gsize, b_offs, bsize, defsize

/*
 * 2-stage pipelined YCbCr->RGB conversion
 */

.macro do_yuv_to_rgb_stage1
    uaddw        v6.8h, v2.8h, v4.8b     /* q3 = u - 128 */
    uaddw        v8.8h, v2.8h, v5.8b     /* q2 = v - 128 */
    smull        v20.4s, v6.4h, v1.4h[1] /* multiply by -11277 */
    smlal        v20.4s, v8.4h, v1.4h[2] /* multiply by -23401 */
    smull2       v22.4s, v6.8h, v1.4h[1] /* multiply by -11277 */
    smlal2       v22.4s, v8.8h, v1.4h[2] /* multiply by -23401 */
    smull        v24.4s, v8.4h, v1.4h[0] /* multiply by 22971 */
    smull2       v26.4s, v8.8h, v1.4h[0] /* multiply by 22971 */
    smull        v28.4s, v6.4h, v1.4h[3] /* multiply by 29033 */
    smull2       v30.4s, v6.8h, v1.4h[3] /* multiply by 29033 */
.endm

.macro do_yuv_to_rgb_stage2
    rshrn        v20.4h, v20.4s, #15
    rshrn2       v20.8h, v22.4s, #15
    rshrn        v24.4h, v24.4s, #14
    rshrn2       v24.8h, v26.4s, #14
    rshrn        v28.4h, v28.4s, #14
    rshrn2       v28.8h, v30.4s, #14
    uaddw        v20.8h, v20.8h, v0.8b
    uaddw        v24.8h, v24.8h, v0.8b
    uaddw        v28.8h, v28.8h, v0.8b
.if \bpp != 16
    sqxtun       v1\g_offs\defsize, v20.8h
    sqxtun       v1\r_offs\defsize, v24.8h
    sqxtun       v1\b_offs\defsize, v28.8h
.else
    sqshlu       v21.8h, v20.8h, #8
    sqshlu       v25.8h, v24.8h, #8
    sqshlu       v29.8h, v28.8h, #8
    sri          v25.8h, v21.8h, #5
    sri          v25.8h, v29.8h, #11
.endif

.endm

.macro do_yuv_to_rgb_stage2_store_load_stage1
    rshrn        v20.4h, v20.4s, #15
    rshrn        v24.4h, v24.4s, #14
    rshrn        v28.4h, v28.4s, #14
    ld1          {v4.8b}, [U], 8
    rshrn2       v20.8h, v22.4s, #15
    rshrn2       v24.8h, v26.4s, #14
    rshrn2       v28.8h, v30.4s, #14
    ld1          {v5.8b}, [V], 8
    uaddw        v20.8h, v20.8h, v0.8b
    uaddw        v24.8h, v24.8h, v0.8b
    uaddw        v28.8h, v28.8h, v0.8b
.if \bpp != 16 /**************** rgb24/rgb32 *********************************/
    sqxtun       v1\g_offs\defsize, v20.8h
    ld1          {v0.8b}, [Y], 8
    sqxtun       v1\r_offs\defsize, v24.8h
    prfm         PLDL1KEEP, [U, #64]
    prfm         PLDL1KEEP, [V, #64]
    prfm         PLDL1KEEP, [Y, #64]
    sqxtun       v1\b_offs\defsize, v28.8h
    uaddw        v6.8h, v2.8h, v4.8b     /* v6.16b = u - 128 */
    uaddw        v8.8h, v2.8h, v5.8b     /* q2 = v - 128 */
    smull        v20.4s, v6.4h, v1.4h[1] /* multiply by -11277 */
    smlal        v20.4s, v8.4h, v1.4h[2] /* multiply by -23401 */
    smull2       v22.4s, v6.8h, v1.4h[1] /* multiply by -11277 */
    smlal2       v22.4s, v8.8h, v1.4h[2] /* multiply by -23401 */
    smull        v24.4s, v8.4h, v1.4h[0] /* multiply by 22971 */
    smull2       v26.4s, v8.8h, v1.4h[0] /* multiply by 22971 */
.else /**************************** rgb565 ***********************************/
    sqshlu       v21.8h, v20.8h, #8
    sqshlu       v25.8h, v24.8h, #8
    sqshlu       v29.8h, v28.8h, #8
    uaddw        v6.8h, v2.8h, v4.8b     /* v6.16b = u - 128 */
    uaddw        v8.8h, v2.8h, v5.8b     /* q2 = v - 128 */
    ld1          {v0.8b}, [Y], 8
    smull        v20.4s, v6.4h, v1.4h[1] /* multiply by -11277 */
    smlal        v20.4s, v8.4h, v1.4h[2] /* multiply by -23401 */
    smull2       v22.4s, v6.8h, v1.4h[1] /* multiply by -11277 */
    smlal2       v22.4s, v8.8h, v1.4h[2] /* multiply by -23401 */
    sri          v25.8h, v21.8h, #5
    smull        v24.4s, v8.4h, v1.4h[0] /* multiply by 22971 */
    smull2       v26.4s, v8.8h, v1.4h[0] /* multiply by 22971 */
    prfm         PLDL1KEEP, [U, #64]
    prfm         PLDL1KEEP, [V, #64]
    prfm         PLDL1KEEP, [Y, #64]
    sri          v25.8h, v29.8h, #11
.endif
    do_store     \bpp, 8
    smull        v28.4s, v6.4h, v1.4h[3] /* multiply by 29033 */
    smull2       v30.4s, v6.8h, v1.4h[3] /* multiply by 29033 */
.endm

.macro do_yuv_to_rgb
    do_yuv_to_rgb_stage1
    do_yuv_to_rgb_stage2
.endm

/* Apple gas crashes on adrl, work around that by using adr.
 * But this requires a copy of these constants for each function.
 */

.balign 16
jsimd_ycc_\colorid\()_neon_consts:
    .short          0,      0,     0,      0
    .short          22971, -11277, -23401, 29033
    .short          -128,  -128,   -128,   -128
    .short          -128,  -128,   -128,   -128

asm_function jsimd_ycc_\colorid\()_convert_neon
    OUTPUT_WIDTH    .req x0
    INPUT_BUF       .req x1
    INPUT_ROW       .req x2
    OUTPUT_BUF      .req x3
    NUM_ROWS        .req x4

    INPUT_BUF0      .req x5
    INPUT_BUF1      .req x6
    INPUT_BUF2      .req INPUT_BUF

    RGB             .req x7
    Y               .req x8
    U               .req x9
    V               .req x10
    N               .req x15

    sub             sp, sp, 336
    str             x15, [sp], 16
    /* Load constants to d1, d2, d3 (v0.4h is just used for padding) */
    adr             x15, jsimd_ycc_\colorid\()_neon_consts
    /* Save NEON registers */
    st1             {v0.8b - v3.8b}, [sp], 32
    st1             {v4.8b - v7.8b}, [sp], 32
    st1             {v8.8b - v11.8b}, [sp], 32
    st1             {v12.8b - v15.8b}, [sp], 32
    st1             {v16.8b - v19.8b}, [sp], 32
    st1             {v20.8b - v23.8b}, [sp], 32
    st1             {v24.8b - v27.8b}, [sp], 32
    st1             {v28.8b - v31.8b}, [sp], 32
    ld1             {v0.4h, v1.4h}, [x15], 16
    ld1             {v2.8h}, [x15]

    /* Save ARM registers and handle input arguments */
    /* push            {x4, x5, x6, x7, x8, x9, x10, x30} */
    stp             x4, x5, [sp], 16
    stp             x6, x7, [sp], 16
    stp             x8, x9, [sp], 16
    stp             x10, x30, [sp], 16
    ldr             INPUT_BUF0, [INPUT_BUF]
    ldr             INPUT_BUF1, [INPUT_BUF, 8]
    ldr             INPUT_BUF2, [INPUT_BUF, 16]
    .unreq          INPUT_BUF

    /* Initially set v10, v11.4h, v12.8b, d13 to 0xFF */
    movi            v10.16b, #255
    movi            v13.16b, #255

    /* Outer loop over scanlines */
    cmp             NUM_ROWS, #1
    blt             9f
0:
    lsl             x16, INPUT_ROW, #3
    ldr             Y, [INPUT_BUF0, x16]
    ldr             U, [INPUT_BUF1, x16]
    mov             N, OUTPUT_WIDTH
    ldr             V, [INPUT_BUF2, x16]
    add             INPUT_ROW, INPUT_ROW, #1
    ldr             RGB, [OUTPUT_BUF], #8

    /* Inner loop over pixels */
    subs            N, N, #8
    blt             3f
    do_load         8
    do_yuv_to_rgb_stage1
    subs            N, N, #8
    blt             2f
1:
    do_yuv_to_rgb_stage2_store_load_stage1
    subs            N, N, #8
    bge             1b
2:
    do_yuv_to_rgb_stage2
    do_store        \bpp, 8
    tst             N, #7
    beq             8f
3:
    tst             N, #4
    beq             3f
    do_load         4
3:
    tst             N, #2
    beq             4f
    do_load         2
4:
    tst             N, #1
    beq             5f
    do_load         1
5:
    do_yuv_to_rgb
    tst             N, #4
    beq             6f
    do_store        \bpp, 4
6:
    tst             N, #2
    beq             7f
    do_store        \bpp, 2
7:
    tst             N, #1
    beq             8f
    do_store        \bpp, 1
8:
    subs            NUM_ROWS, NUM_ROWS, #1
    bgt             0b
9:
    /* Restore all registers and return */
    sub             sp, sp, #336
    ldr             x15, [sp], 16
    ld1             {v0.8b - v3.8b}, [sp], 32
    ld1             {v4.8b - v7.8b}, [sp], 32
    ld1             {v8.8b - v11.8b}, [sp], 32
    ld1             {v12.8b - v15.8b}, [sp], 32
    ld1             {v16.8b - v19.8b}, [sp], 32
    ld1             {v20.8b - v23.8b}, [sp], 32
    ld1             {v24.8b - v27.8b}, [sp], 32
    ld1             {v28.8b - v31.8b}, [sp], 32
    /* pop             {r4, r5, r6, r7, r8, r9, r10, pc} */
    ldp             x4, x5, [sp], 16
    ldp             x6, x7, [sp], 16
    ldp             x8, x9, [sp], 16
    ldp             x10, x30, [sp], 16
    br              x30
    .unreq          OUTPUT_WIDTH
    .unreq          INPUT_ROW
    .unreq          OUTPUT_BUF
    .unreq          NUM_ROWS
    .unreq          INPUT_BUF0
    .unreq          INPUT_BUF1
    .unreq          INPUT_BUF2
    .unreq          RGB
    .unreq          Y
    .unreq          U
    .unreq          V
    .unreq          N

.purgem do_yuv_to_rgb
.purgem do_yuv_to_rgb_stage1
.purgem do_yuv_to_rgb_stage2
.purgem do_yuv_to_rgb_stage2_store_load_stage1
.endm

/*--------------------------------- id ----- bpp R  rsize  G  gsize  B  bsize  defsize   */
generate_jsimd_ycc_rgb_convert_neon extrgb,  24, 0, .4h,   1, .4h,   2, .4h,   .8b
generate_jsimd_ycc_rgb_convert_neon extbgr,  24, 2, .4h,   1, .4h,   0, .4h,   .8b
generate_jsimd_ycc_rgb_convert_neon extrgbx, 32, 0, .4h,   1, .4h,   2, .4h,   .8b
generate_jsimd_ycc_rgb_convert_neon extbgrx, 32, 2, .4h,   1, .4h,   0, .4h,   .8b
generate_jsimd_ycc_rgb_convert_neon extxbgr, 32, 3, .4h,   2, .4h,   1, .4h,   .8b
generate_jsimd_ycc_rgb_convert_neon extxrgb, 32, 1, .4h,   2, .4h,   3, .4h,   .8b
generate_jsimd_ycc_rgb_convert_neon rgb565,  16, 0, .4h,   0, .4h,   0, .4h,   .8b
.purgem do_load
.purgem do_store
