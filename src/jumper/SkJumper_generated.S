# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is generated semi-automatically with this command:
#   $ src/jumper/build_stages.py

#if defined(__MACH__)
    #define HIDDEN .private_extern
#else
    #define HIDDEN .hidden
    .section .note.GNU-stack,"",%progbits
#endif
.text
#if defined(__aarch64__)
.balign 4

HIDDEN _sk_start_pipeline_aarch64
.globl _sk_start_pipeline_aarch64
_sk_start_pipeline_aarch64:
  .long  0xa9bd5bf7                          // stp           x23, x22, [sp, #-48]!
  .long  0xa90153f5                          // stp           x21, x20, [sp, #16]
  .long  0xa9027bf3                          // stp           x19, x30, [sp, #32]
  .long  0xaa0103f4                          // mov           x20, x1
  .long  0xf8408697                          // ldr           x23, [x20], #8
  .long  0xaa0003f5                          // mov           x21, x0
  .long  0xaa0303f3                          // mov           x19, x3
  .long  0x910012a8                          // add           x8, x21, #0x4
  .long  0xeb13011f                          // cmp           x8, x19
  .long  0xaa0203f6                          // mov           x22, x2
  .long  0x54000069                          // b.ls          34 <sk_start_pipeline_aarch64+0x34>  // b.plast
  .long  0xaa1503e0                          // mov           x0, x21
  .long  0x14000012                          // b             78 <sk_start_pipeline_aarch64+0x78>
  .long  0x6f00e400                          // movi          v0.2d, #0x0
  .long  0x6f00e401                          // movi          v1.2d, #0x0
  .long  0x6f00e402                          // movi          v2.2d, #0x0
  .long  0x6f00e403                          // movi          v3.2d, #0x0
  .long  0x6f00e404                          // movi          v4.2d, #0x0
  .long  0x6f00e405                          // movi          v5.2d, #0x0
  .long  0x6f00e406                          // movi          v6.2d, #0x0
  .long  0x6f00e407                          // movi          v7.2d, #0x0
  .long  0xaa1503e0                          // mov           x0, x21
  .long  0xaa1403e1                          // mov           x1, x20
  .long  0xaa1603e2                          // mov           x2, x22
  .long  0xd63f02e0                          // blr           x23
  .long  0x910012a0                          // add           x0, x21, #0x4
  .long  0x910022a8                          // add           x8, x21, #0x8
  .long  0xeb13011f                          // cmp           x8, x19
  .long  0xaa0003f5                          // mov           x21, x0
  .long  0x54fffe09                          // b.ls          34 <sk_start_pipeline_aarch64+0x34>  // b.plast
  .long  0xa9427bf3                          // ldp           x19, x30, [sp, #32]
  .long  0xa94153f5                          // ldp           x21, x20, [sp, #16]
  .long  0xa8c35bf7                          // ldp           x23, x22, [sp], #48
  .long  0xd65f03c0                          // ret

HIDDEN _sk_just_return_aarch64
.globl _sk_just_return_aarch64
_sk_just_return_aarch64:
  .long  0xd65f03c0                          // ret

HIDDEN _sk_seed_shader_aarch64
.globl _sk_seed_shader_aarch64
_sk_seed_shader_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0x3dc00046                          // ldr           q6, [x2]
  .long  0x4e040c00                          // dup           v0.4s, w0
  .long  0x4f0167e7                          // movi          v7.4s, #0x3f, lsl #24
  .long  0x4d40c901                          // ld1r          {v1.4s}, [x8]
  .long  0x4e21d800                          // scvtf         v0.4s, v0.4s
  .long  0x4e27d400                          // fadd          v0.4s, v0.4s, v7.4s
  .long  0x4f03f602                          // fmov          v2.4s, #1.000000000000000000e+00
  .long  0x4e21d821                          // scvtf         v1.4s, v1.4s
  .long  0x6f00e403                          // movi          v3.2d, #0x0
  .long  0x6f00e404                          // movi          v4.2d, #0x0
  .long  0x6f00e405                          // movi          v5.2d, #0x0
  .long  0x4e26d400                          // fadd          v0.4s, v0.4s, v6.4s
  .long  0x6f00e406                          // movi          v6.2d, #0x0
  .long  0x4e27d421                          // fadd          v1.4s, v1.4s, v7.4s
  .long  0x6f00e407                          // movi          v7.2d, #0x0
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_constant_color_aarch64
.globl _sk_constant_color_aarch64
_sk_constant_color_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0x3dc00103                          // ldr           q3, [x8]
  .long  0x4e040460                          // dup           v0.4s, v3.s[0]
  .long  0x4e0c0461                          // dup           v1.4s, v3.s[1]
  .long  0x4e140462                          // dup           v2.4s, v3.s[2]
  .long  0x4e1c0463                          // dup           v3.4s, v3.s[3]
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_clear_aarch64
.globl _sk_clear_aarch64
_sk_clear_aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x6f00e400                          // movi          v0.2d, #0x0
  .long  0x6f00e401                          // movi          v1.2d, #0x0
  .long  0x6f00e402                          // movi          v2.2d, #0x0
  .long  0x6f00e403                          // movi          v3.2d, #0x0
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_srcatop_aarch64
.globl _sk_srcatop_aarch64
_sk_srcatop_aarch64:
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x6e27dc00                          // fmul          v0.4s, v0.4s, v7.4s
  .long  0x6e27dc21                          // fmul          v1.4s, v1.4s, v7.4s
  .long  0x6e27dc42                          // fmul          v2.4s, v2.4s, v7.4s
  .long  0x4ea3d610                          // fsub          v16.4s, v16.4s, v3.4s
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4e30cc80                          // fmla          v0.4s, v4.4s, v16.4s
  .long  0x4e30cca1                          // fmla          v1.4s, v5.4s, v16.4s
  .long  0x4e30ccc2                          // fmla          v2.4s, v6.4s, v16.4s
  .long  0x6e27de10                          // fmul          v16.4s, v16.4s, v7.4s
  .long  0x4e23ccf0                          // fmla          v16.4s, v7.4s, v3.4s
  .long  0x4eb01e03                          // mov           v3.16b, v16.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_dstatop_aarch64
.globl _sk_dstatop_aarch64
_sk_dstatop_aarch64:
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x4ea7d610                          // fsub          v16.4s, v16.4s, v7.4s
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x6e20de00                          // fmul          v0.4s, v16.4s, v0.4s
  .long  0x6e21de01                          // fmul          v1.4s, v16.4s, v1.4s
  .long  0x6e22de02                          // fmul          v2.4s, v16.4s, v2.4s
  .long  0x6e23de10                          // fmul          v16.4s, v16.4s, v3.4s
  .long  0x4e23ccf0                          // fmla          v16.4s, v7.4s, v3.4s
  .long  0x4e23cc80                          // fmla          v0.4s, v4.4s, v3.4s
  .long  0x4e23cca1                          // fmla          v1.4s, v5.4s, v3.4s
  .long  0x4e23ccc2                          // fmla          v2.4s, v6.4s, v3.4s
  .long  0x4eb01e03                          // mov           v3.16b, v16.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_srcin_aarch64
.globl _sk_srcin_aarch64
_sk_srcin_aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x6e27dc00                          // fmul          v0.4s, v0.4s, v7.4s
  .long  0x6e27dc21                          // fmul          v1.4s, v1.4s, v7.4s
  .long  0x6e27dc42                          // fmul          v2.4s, v2.4s, v7.4s
  .long  0x6e27dc63                          // fmul          v3.4s, v3.4s, v7.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_dstin_aarch64
.globl _sk_dstin_aarch64
_sk_dstin_aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x6e24dc60                          // fmul          v0.4s, v3.4s, v4.4s
  .long  0x6e25dc61                          // fmul          v1.4s, v3.4s, v5.4s
  .long  0x6e26dc62                          // fmul          v2.4s, v3.4s, v6.4s
  .long  0x6e27dc63                          // fmul          v3.4s, v3.4s, v7.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_srcout_aarch64
.globl _sk_srcout_aarch64
_sk_srcout_aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x4ea7d610                          // fsub          v16.4s, v16.4s, v7.4s
  .long  0x6e20de00                          // fmul          v0.4s, v16.4s, v0.4s
  .long  0x6e21de01                          // fmul          v1.4s, v16.4s, v1.4s
  .long  0x6e22de02                          // fmul          v2.4s, v16.4s, v2.4s
  .long  0x6e23de03                          // fmul          v3.4s, v16.4s, v3.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_dstout_aarch64
.globl _sk_dstout_aarch64
_sk_dstout_aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4f03f600                          // fmov          v0.4s, #1.000000000000000000e+00
  .long  0x4ea3d403                          // fsub          v3.4s, v0.4s, v3.4s
  .long  0x6e24dc60                          // fmul          v0.4s, v3.4s, v4.4s
  .long  0x6e25dc61                          // fmul          v1.4s, v3.4s, v5.4s
  .long  0x6e26dc62                          // fmul          v2.4s, v3.4s, v6.4s
  .long  0x6e27dc63                          // fmul          v3.4s, v3.4s, v7.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_srcover_aarch64
.globl _sk_srcover_aarch64
_sk_srcover_aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x4ea3d610                          // fsub          v16.4s, v16.4s, v3.4s
  .long  0x4e24ce00                          // fmla          v0.4s, v16.4s, v4.4s
  .long  0x4e25ce01                          // fmla          v1.4s, v16.4s, v5.4s
  .long  0x4e26ce02                          // fmla          v2.4s, v16.4s, v6.4s
  .long  0x4e27ce03                          // fmla          v3.4s, v16.4s, v7.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_dstover_aarch64
.globl _sk_dstover_aarch64
_sk_dstover_aarch64:
  .long  0x4f03f611                          // fmov          v17.4s, #1.000000000000000000e+00
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4ea41c90                          // mov           v16.16b, v4.16b
  .long  0x4ea7d634                          // fsub          v20.4s, v17.4s, v7.4s
  .long  0x4ea51cb1                          // mov           v17.16b, v5.16b
  .long  0x4ea61cd2                          // mov           v18.16b, v6.16b
  .long  0x4ea71cf3                          // mov           v19.16b, v7.16b
  .long  0x4e20ce90                          // fmla          v16.4s, v20.4s, v0.4s
  .long  0x4e21ce91                          // fmla          v17.4s, v20.4s, v1.4s
  .long  0x4e22ce92                          // fmla          v18.4s, v20.4s, v2.4s
  .long  0x4e23ce93                          // fmla          v19.4s, v20.4s, v3.4s
  .long  0x4eb01e00                          // mov           v0.16b, v16.16b
  .long  0x4eb11e21                          // mov           v1.16b, v17.16b
  .long  0x4eb21e42                          // mov           v2.16b, v18.16b
  .long  0x4eb31e63                          // mov           v3.16b, v19.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_modulate_aarch64
.globl _sk_modulate_aarch64
_sk_modulate_aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x6e24dc00                          // fmul          v0.4s, v0.4s, v4.4s
  .long  0x6e25dc21                          // fmul          v1.4s, v1.4s, v5.4s
  .long  0x6e26dc42                          // fmul          v2.4s, v2.4s, v6.4s
  .long  0x6e27dc63                          // fmul          v3.4s, v3.4s, v7.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_multiply_aarch64
.globl _sk_multiply_aarch64
_sk_multiply_aarch64:
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x4ea7d613                          // fsub          v19.4s, v16.4s, v7.4s
  .long  0x4ea3d614                          // fsub          v20.4s, v16.4s, v3.4s
  .long  0x6e20de70                          // fmul          v16.4s, v19.4s, v0.4s
  .long  0x6e21de71                          // fmul          v17.4s, v19.4s, v1.4s
  .long  0x6e22de72                          // fmul          v18.4s, v19.4s, v2.4s
  .long  0x6e23de73                          // fmul          v19.4s, v19.4s, v3.4s
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4e34cc90                          // fmla          v16.4s, v4.4s, v20.4s
  .long  0x4e34ccb1                          // fmla          v17.4s, v5.4s, v20.4s
  .long  0x4e34ccd2                          // fmla          v18.4s, v6.4s, v20.4s
  .long  0x4e34ccf3                          // fmla          v19.4s, v7.4s, v20.4s
  .long  0x4e20cc90                          // fmla          v16.4s, v4.4s, v0.4s
  .long  0x4e21ccb1                          // fmla          v17.4s, v5.4s, v1.4s
  .long  0x4e22ccd2                          // fmla          v18.4s, v6.4s, v2.4s
  .long  0x4e23ccf3                          // fmla          v19.4s, v7.4s, v3.4s
  .long  0x4eb01e00                          // mov           v0.16b, v16.16b
  .long  0x4eb11e21                          // mov           v1.16b, v17.16b
  .long  0x4eb21e42                          // mov           v2.16b, v18.16b
  .long  0x4eb31e63                          // mov           v3.16b, v19.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_plus__aarch64
.globl _sk_plus__aarch64
_sk_plus__aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4e24d400                          // fadd          v0.4s, v0.4s, v4.4s
  .long  0x4e25d421                          // fadd          v1.4s, v1.4s, v5.4s
  .long  0x4e26d442                          // fadd          v2.4s, v2.4s, v6.4s
  .long  0x4e27d463                          // fadd          v3.4s, v3.4s, v7.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_screen_aarch64
.globl _sk_screen_aarch64
_sk_screen_aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4e24d410                          // fadd          v16.4s, v0.4s, v4.4s
  .long  0x4e25d431                          // fadd          v17.4s, v1.4s, v5.4s
  .long  0x4e26d452                          // fadd          v18.4s, v2.4s, v6.4s
  .long  0x4e27d473                          // fadd          v19.4s, v3.4s, v7.4s
  .long  0x4ea4cc10                          // fmls          v16.4s, v0.4s, v4.4s
  .long  0x4ea5cc31                          // fmls          v17.4s, v1.4s, v5.4s
  .long  0x4ea6cc52                          // fmls          v18.4s, v2.4s, v6.4s
  .long  0x4ea7cc73                          // fmls          v19.4s, v3.4s, v7.4s
  .long  0x4eb01e00                          // mov           v0.16b, v16.16b
  .long  0x4eb11e21                          // mov           v1.16b, v17.16b
  .long  0x4eb21e42                          // mov           v2.16b, v18.16b
  .long  0x4eb31e63                          // mov           v3.16b, v19.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_xor__aarch64
.globl _sk_xor__aarch64
_sk_xor__aarch64:
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4ea7d611                          // fsub          v17.4s, v16.4s, v7.4s
  .long  0x4ea3d610                          // fsub          v16.4s, v16.4s, v3.4s
  .long  0x6e20de20                          // fmul          v0.4s, v17.4s, v0.4s
  .long  0x6e21de21                          // fmul          v1.4s, v17.4s, v1.4s
  .long  0x6e22de22                          // fmul          v2.4s, v17.4s, v2.4s
  .long  0x6e23de23                          // fmul          v3.4s, v17.4s, v3.4s
  .long  0x4e30cc80                          // fmla          v0.4s, v4.4s, v16.4s
  .long  0x4e30cca1                          // fmla          v1.4s, v5.4s, v16.4s
  .long  0x4e30ccc2                          // fmla          v2.4s, v6.4s, v16.4s
  .long  0x4e30cce3                          // fmla          v3.4s, v7.4s, v16.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_darken_aarch64
.globl _sk_darken_aarch64
_sk_darken_aarch64:
  .long  0x6e27dc10                          // fmul          v16.4s, v0.4s, v7.4s
  .long  0x6e24dc71                          // fmul          v17.4s, v3.4s, v4.4s
  .long  0x6e27dc32                          // fmul          v18.4s, v1.4s, v7.4s
  .long  0x6e25dc73                          // fmul          v19.4s, v3.4s, v5.4s
  .long  0x4e31f610                          // fmax          v16.4s, v16.4s, v17.4s
  .long  0x4e24d400                          // fadd          v0.4s, v0.4s, v4.4s
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x6e27dc51                          // fmul          v17.4s, v2.4s, v7.4s
  .long  0x4e33f652                          // fmax          v18.4s, v18.4s, v19.4s
  .long  0x6e26dc73                          // fmul          v19.4s, v3.4s, v6.4s
  .long  0x4eb0d400                          // fsub          v0.4s, v0.4s, v16.4s
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x4e33f631                          // fmax          v17.4s, v17.4s, v19.4s
  .long  0x4e25d421                          // fadd          v1.4s, v1.4s, v5.4s
  .long  0x4e26d442                          // fadd          v2.4s, v2.4s, v6.4s
  .long  0x4ea3d610                          // fsub          v16.4s, v16.4s, v3.4s
  .long  0x4eb2d421                          // fsub          v1.4s, v1.4s, v18.4s
  .long  0x4eb1d442                          // fsub          v2.4s, v2.4s, v17.4s
  .long  0x4e27ce03                          // fmla          v3.4s, v16.4s, v7.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_lighten_aarch64
.globl _sk_lighten_aarch64
_sk_lighten_aarch64:
  .long  0x6e27dc10                          // fmul          v16.4s, v0.4s, v7.4s
  .long  0x6e24dc71                          // fmul          v17.4s, v3.4s, v4.4s
  .long  0x6e27dc32                          // fmul          v18.4s, v1.4s, v7.4s
  .long  0x6e25dc73                          // fmul          v19.4s, v3.4s, v5.4s
  .long  0x4eb1f610                          // fmin          v16.4s, v16.4s, v17.4s
  .long  0x4e24d400                          // fadd          v0.4s, v0.4s, v4.4s
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x6e27dc51                          // fmul          v17.4s, v2.4s, v7.4s
  .long  0x4eb3f652                          // fmin          v18.4s, v18.4s, v19.4s
  .long  0x6e26dc73                          // fmul          v19.4s, v3.4s, v6.4s
  .long  0x4eb0d400                          // fsub          v0.4s, v0.4s, v16.4s
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x4eb3f631                          // fmin          v17.4s, v17.4s, v19.4s
  .long  0x4e25d421                          // fadd          v1.4s, v1.4s, v5.4s
  .long  0x4e26d442                          // fadd          v2.4s, v2.4s, v6.4s
  .long  0x4ea3d610                          // fsub          v16.4s, v16.4s, v3.4s
  .long  0x4eb2d421                          // fsub          v1.4s, v1.4s, v18.4s
  .long  0x4eb1d442                          // fsub          v2.4s, v2.4s, v17.4s
  .long  0x4e27ce03                          // fmla          v3.4s, v16.4s, v7.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_difference_aarch64
.globl _sk_difference_aarch64
_sk_difference_aarch64:
  .long  0x6e27dc10                          // fmul          v16.4s, v0.4s, v7.4s
  .long  0x6e24dc71                          // fmul          v17.4s, v3.4s, v4.4s
  .long  0x6e27dc32                          // fmul          v18.4s, v1.4s, v7.4s
  .long  0x6e25dc73                          // fmul          v19.4s, v3.4s, v5.4s
  .long  0x4eb1f610                          // fmin          v16.4s, v16.4s, v17.4s
  .long  0x4eb3f652                          // fmin          v18.4s, v18.4s, v19.4s
  .long  0x4e24d400                          // fadd          v0.4s, v0.4s, v4.4s
  .long  0x4e30d610                          // fadd          v16.4s, v16.4s, v16.4s
  .long  0x6e27dc51                          // fmul          v17.4s, v2.4s, v7.4s
  .long  0x6e26dc73                          // fmul          v19.4s, v3.4s, v6.4s
  .long  0x4eb0d400                          // fsub          v0.4s, v0.4s, v16.4s
  .long  0x4e25d421                          // fadd          v1.4s, v1.4s, v5.4s
  .long  0x4e32d650                          // fadd          v16.4s, v18.4s, v18.4s
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4eb3f631                          // fmin          v17.4s, v17.4s, v19.4s
  .long  0x4eb0d421                          // fsub          v1.4s, v1.4s, v16.4s
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x4e26d442                          // fadd          v2.4s, v2.4s, v6.4s
  .long  0x4e31d631                          // fadd          v17.4s, v17.4s, v17.4s
  .long  0x4ea3d610                          // fsub          v16.4s, v16.4s, v3.4s
  .long  0x4eb1d442                          // fsub          v2.4s, v2.4s, v17.4s
  .long  0x4e27ce03                          // fmla          v3.4s, v16.4s, v7.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_exclusion_aarch64
.globl _sk_exclusion_aarch64
_sk_exclusion_aarch64:
  .long  0x4e24d410                          // fadd          v16.4s, v0.4s, v4.4s
  .long  0x6e24dc00                          // fmul          v0.4s, v0.4s, v4.4s
  .long  0x4e20d400                          // fadd          v0.4s, v0.4s, v0.4s
  .long  0x4ea0d600                          // fsub          v0.4s, v16.4s, v0.4s
  .long  0x4e25d430                          // fadd          v16.4s, v1.4s, v5.4s
  .long  0x6e25dc21                          // fmul          v1.4s, v1.4s, v5.4s
  .long  0x4e21d421                          // fadd          v1.4s, v1.4s, v1.4s
  .long  0x4ea1d601                          // fsub          v1.4s, v16.4s, v1.4s
  .long  0x4e26d450                          // fadd          v16.4s, v2.4s, v6.4s
  .long  0x6e26dc42                          // fmul          v2.4s, v2.4s, v6.4s
  .long  0x4e22d442                          // fadd          v2.4s, v2.4s, v2.4s
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4ea2d602                          // fsub          v2.4s, v16.4s, v2.4s
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x4ea3d610                          // fsub          v16.4s, v16.4s, v3.4s
  .long  0x4e27ce03                          // fmla          v3.4s, v16.4s, v7.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_colorburn_aarch64
.globl _sk_colorburn_aarch64
_sk_colorburn_aarch64:
  .long  0x4ea4d4f3                          // fsub          v19.4s, v7.4s, v4.4s
  .long  0x6e23de73                          // fmul          v19.4s, v19.4s, v3.4s
  .long  0x4f03f611                          // fmov          v17.4s, #1.000000000000000000e+00
  .long  0x6e20fe73                          // fdiv          v19.4s, v19.4s, v0.4s
  .long  0x4ea7d634                          // fsub          v20.4s, v17.4s, v7.4s
  .long  0x4eb3f4f3                          // fmin          v19.4s, v7.4s, v19.4s
  .long  0x6e20de95                          // fmul          v21.4s, v20.4s, v0.4s
  .long  0x4eb3d4f3                          // fsub          v19.4s, v7.4s, v19.4s
  .long  0x4e24d6b6                          // fadd          v22.4s, v21.4s, v4.4s
  .long  0x4e33cc75                          // fmla          v21.4s, v3.4s, v19.4s
  .long  0x4ea5d4f3                          // fsub          v19.4s, v7.4s, v5.4s
  .long  0x6e23de73                          // fmul          v19.4s, v19.4s, v3.4s
  .long  0x6e21fe73                          // fdiv          v19.4s, v19.4s, v1.4s
  .long  0x4ea0d812                          // fcmeq         v18.4s, v0.4s, #0.0
  .long  0x4eb3f4f3                          // fmin          v19.4s, v7.4s, v19.4s
  .long  0x6e751c12                          // bsl           v18.16b, v0.16b, v21.16b
  .long  0x6e21de80                          // fmul          v0.4s, v20.4s, v1.4s
  .long  0x4eb3d4f3                          // fsub          v19.4s, v7.4s, v19.4s
  .long  0x4e25d415                          // fadd          v21.4s, v0.4s, v5.4s
  .long  0x4e33cc60                          // fmla          v0.4s, v3.4s, v19.4s
  .long  0x4ea0d833                          // fcmeq         v19.4s, v1.4s, #0.0
  .long  0x6e601c33                          // bsl           v19.16b, v1.16b, v0.16b
  .long  0x4ea6d4e0                          // fsub          v0.4s, v7.4s, v6.4s
  .long  0x6e23dc00                          // fmul          v0.4s, v0.4s, v3.4s
  .long  0x6e22fc00                          // fdiv          v0.4s, v0.4s, v2.4s
  .long  0x4ea0f4e0                          // fmin          v0.4s, v7.4s, v0.4s
  .long  0x6e22de81                          // fmul          v1.4s, v20.4s, v2.4s
  .long  0x4ea0d4e0                          // fsub          v0.4s, v7.4s, v0.4s
  .long  0x4e26d434                          // fadd          v20.4s, v1.4s, v6.4s
  .long  0x4e20cc61                          // fmla          v1.4s, v3.4s, v0.4s
  .long  0x4ea0d840                          // fcmeq         v0.4s, v2.4s, #0.0
  .long  0x4ea3d631                          // fsub          v17.4s, v17.4s, v3.4s
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4e27e490                          // fcmeq         v16.4s, v4.4s, v7.4s
  .long  0x6e611c40                          // bsl           v0.16b, v2.16b, v1.16b
  .long  0x4e31cc92                          // fmla          v18.4s, v4.4s, v17.4s
  .long  0x4e27e4a1                          // fcmeq         v1.4s, v5.4s, v7.4s
  .long  0x4e27e4c2                          // fcmeq         v2.4s, v6.4s, v7.4s
  .long  0x4e31ccb3                          // fmla          v19.4s, v5.4s, v17.4s
  .long  0x4e31ccc0                          // fmla          v0.4s, v6.4s, v17.4s
  .long  0x6e721ed0                          // bsl           v16.16b, v22.16b, v18.16b
  .long  0x6e731ea1                          // bsl           v1.16b, v21.16b, v19.16b
  .long  0x6e601e82                          // bsl           v2.16b, v20.16b, v0.16b
  .long  0x4e27ce23                          // fmla          v3.4s, v17.4s, v7.4s
  .long  0x4eb01e00                          // mov           v0.16b, v16.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_colordodge_aarch64
.globl _sk_colordodge_aarch64
_sk_colordodge_aarch64:
  .long  0x4f03f612                          // fmov          v18.4s, #1.000000000000000000e+00
  .long  0x6e24dc71                          // fmul          v17.4s, v3.4s, v4.4s
  .long  0x4ea0d474                          // fsub          v20.4s, v3.4s, v0.4s
  .long  0x6e25dc75                          // fmul          v21.4s, v3.4s, v5.4s
  .long  0x4ea1d476                          // fsub          v22.4s, v3.4s, v1.4s
  .long  0x4ea7d657                          // fsub          v23.4s, v18.4s, v7.4s
  .long  0x6e34fe31                          // fdiv          v17.4s, v17.4s, v20.4s
  .long  0x6e36feb4                          // fdiv          v20.4s, v21.4s, v22.4s
  .long  0x6e20def5                          // fmul          v21.4s, v23.4s, v0.4s
  .long  0x4eb1f4f1                          // fmin          v17.4s, v7.4s, v17.4s
  .long  0x4e23e413                          // fcmeq         v19.4s, v0.4s, v3.4s
  .long  0x4e24d6b6                          // fadd          v22.4s, v21.4s, v4.4s
  .long  0x4e31cc75                          // fmla          v21.4s, v3.4s, v17.4s
  .long  0x6e751c13                          // bsl           v19.16b, v0.16b, v21.16b
  .long  0x6e21dee0                          // fmul          v0.4s, v23.4s, v1.4s
  .long  0x4eb4f4f4                          // fmin          v20.4s, v7.4s, v20.4s
  .long  0x4e25d415                          // fadd          v21.4s, v0.4s, v5.4s
  .long  0x4e34cc60                          // fmla          v0.4s, v3.4s, v20.4s
  .long  0x4e23e434                          // fcmeq         v20.4s, v1.4s, v3.4s
  .long  0x6e601c34                          // bsl           v20.16b, v1.16b, v0.16b
  .long  0x6e26dc60                          // fmul          v0.4s, v3.4s, v6.4s
  .long  0x4ea2d461                          // fsub          v1.4s, v3.4s, v2.4s
  .long  0x6e21fc00                          // fdiv          v0.4s, v0.4s, v1.4s
  .long  0x6e22dee1                          // fmul          v1.4s, v23.4s, v2.4s
  .long  0x4ea0f4e0                          // fmin          v0.4s, v7.4s, v0.4s
  .long  0x4e26d437                          // fadd          v23.4s, v1.4s, v6.4s
  .long  0x4e20cc61                          // fmla          v1.4s, v3.4s, v0.4s
  .long  0x4e23e440                          // fcmeq         v0.4s, v2.4s, v3.4s
  .long  0x6e611c40                          // bsl           v0.16b, v2.16b, v1.16b
  .long  0x4ea3d641                          // fsub          v1.4s, v18.4s, v3.4s
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4ea0d890                          // fcmeq         v16.4s, v4.4s, #0.0
  .long  0x4ea0d8b1                          // fcmeq         v17.4s, v5.4s, #0.0
  .long  0x4e21cc93                          // fmla          v19.4s, v4.4s, v1.4s
  .long  0x4e21ccb4                          // fmla          v20.4s, v5.4s, v1.4s
  .long  0x4ea0d8c2                          // fcmeq         v2.4s, v6.4s, #0.0
  .long  0x4e21ccc0                          // fmla          v0.4s, v6.4s, v1.4s
  .long  0x6e731ed0                          // bsl           v16.16b, v22.16b, v19.16b
  .long  0x6e741eb1                          // bsl           v17.16b, v21.16b, v20.16b
  .long  0x6e601ee2                          // bsl           v2.16b, v23.16b, v0.16b
  .long  0x4e27cc23                          // fmla          v3.4s, v1.4s, v7.4s
  .long  0x4eb01e00                          // mov           v0.16b, v16.16b
  .long  0x4eb11e21                          // mov           v1.16b, v17.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_hardlight_aarch64
.globl _sk_hardlight_aarch64
_sk_hardlight_aarch64:
  .long  0x4ea4d4f4                          // fsub          v20.4s, v7.4s, v4.4s
  .long  0x4ea0d475                          // fsub          v21.4s, v3.4s, v0.4s
  .long  0x6e34deb4                          // fmul          v20.4s, v21.4s, v20.4s
  .long  0x4e20d411                          // fadd          v17.4s, v0.4s, v0.4s
  .long  0x6e24dc12                          // fmul          v18.4s, v0.4s, v4.4s
  .long  0x6e27dc73                          // fmul          v19.4s, v3.4s, v7.4s
  .long  0x4e34d694                          // fadd          v20.4s, v20.4s, v20.4s
  .long  0x6e31e471                          // fcmge         v17.4s, v3.4s, v17.4s
  .long  0x4e32d652                          // fadd          v18.4s, v18.4s, v18.4s
  .long  0x4eb4d674                          // fsub          v20.4s, v19.4s, v20.4s
  .long  0x6e741e51                          // bsl           v17.16b, v18.16b, v20.16b
  .long  0x4ea5d4f2                          // fsub          v18.4s, v7.4s, v5.4s
  .long  0x4ea1d474                          // fsub          v20.4s, v3.4s, v1.4s
  .long  0x6e32de92                          // fmul          v18.4s, v20.4s, v18.4s
  .long  0x4e21d436                          // fadd          v22.4s, v1.4s, v1.4s
  .long  0x6e25dc35                          // fmul          v21.4s, v1.4s, v5.4s
  .long  0x4e32d652                          // fadd          v18.4s, v18.4s, v18.4s
  .long  0x6e36e476                          // fcmge         v22.4s, v3.4s, v22.4s
  .long  0x4e35d6b5                          // fadd          v21.4s, v21.4s, v21.4s
  .long  0x4eb2d672                          // fsub          v18.4s, v19.4s, v18.4s
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x6e721eb6                          // bsl           v22.16b, v21.16b, v18.16b
  .long  0x4ea6d4f2                          // fsub          v18.4s, v7.4s, v6.4s
  .long  0x4ea2d475                          // fsub          v21.4s, v3.4s, v2.4s
  .long  0x6e32deb2                          // fmul          v18.4s, v21.4s, v18.4s
  .long  0x4ea7d615                          // fsub          v21.4s, v16.4s, v7.4s
  .long  0x4e22d454                          // fadd          v20.4s, v2.4s, v2.4s
  .long  0x6e20dea0                          // fmul          v0.4s, v21.4s, v0.4s
  .long  0x6e21dea1                          // fmul          v1.4s, v21.4s, v1.4s
  .long  0x6e22deb5                          // fmul          v21.4s, v21.4s, v2.4s
  .long  0x6e26dc42                          // fmul          v2.4s, v2.4s, v6.4s
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4e32d652                          // fadd          v18.4s, v18.4s, v18.4s
  .long  0x4ea3d610                          // fsub          v16.4s, v16.4s, v3.4s
  .long  0x6e34e474                          // fcmge         v20.4s, v3.4s, v20.4s
  .long  0x4e22d442                          // fadd          v2.4s, v2.4s, v2.4s
  .long  0x4eb2d672                          // fsub          v18.4s, v19.4s, v18.4s
  .long  0x4e30cc80                          // fmla          v0.4s, v4.4s, v16.4s
  .long  0x4e30cca1                          // fmla          v1.4s, v5.4s, v16.4s
  .long  0x4e30ccd5                          // fmla          v21.4s, v6.4s, v16.4s
  .long  0x6e721c54                          // bsl           v20.16b, v2.16b, v18.16b
  .long  0x4e31d400                          // fadd          v0.4s, v0.4s, v17.4s
  .long  0x4e36d421                          // fadd          v1.4s, v1.4s, v22.4s
  .long  0x4e34d6a2                          // fadd          v2.4s, v21.4s, v20.4s
  .long  0x4e27ce03                          // fmla          v3.4s, v16.4s, v7.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_overlay_aarch64
.globl _sk_overlay_aarch64
_sk_overlay_aarch64:
  .long  0x4ea4d4f4                          // fsub          v20.4s, v7.4s, v4.4s
  .long  0x4ea0d475                          // fsub          v21.4s, v3.4s, v0.4s
  .long  0x6e34deb4                          // fmul          v20.4s, v21.4s, v20.4s
  .long  0x4e24d491                          // fadd          v17.4s, v4.4s, v4.4s
  .long  0x6e24dc12                          // fmul          v18.4s, v0.4s, v4.4s
  .long  0x6e27dc73                          // fmul          v19.4s, v3.4s, v7.4s
  .long  0x4e34d694                          // fadd          v20.4s, v20.4s, v20.4s
  .long  0x6e31e4f1                          // fcmge         v17.4s, v7.4s, v17.4s
  .long  0x4e32d652                          // fadd          v18.4s, v18.4s, v18.4s
  .long  0x4eb4d674                          // fsub          v20.4s, v19.4s, v20.4s
  .long  0x6e741e51                          // bsl           v17.16b, v18.16b, v20.16b
  .long  0x4ea5d4f2                          // fsub          v18.4s, v7.4s, v5.4s
  .long  0x4ea1d474                          // fsub          v20.4s, v3.4s, v1.4s
  .long  0x6e32de92                          // fmul          v18.4s, v20.4s, v18.4s
  .long  0x4e25d4b6                          // fadd          v22.4s, v5.4s, v5.4s
  .long  0x6e25dc35                          // fmul          v21.4s, v1.4s, v5.4s
  .long  0x4e32d652                          // fadd          v18.4s, v18.4s, v18.4s
  .long  0x6e36e4f6                          // fcmge         v22.4s, v7.4s, v22.4s
  .long  0x4e35d6b5                          // fadd          v21.4s, v21.4s, v21.4s
  .long  0x4eb2d672                          // fsub          v18.4s, v19.4s, v18.4s
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x6e721eb6                          // bsl           v22.16b, v21.16b, v18.16b
  .long  0x4ea6d4f2                          // fsub          v18.4s, v7.4s, v6.4s
  .long  0x4ea2d475                          // fsub          v21.4s, v3.4s, v2.4s
  .long  0x6e32deb2                          // fmul          v18.4s, v21.4s, v18.4s
  .long  0x4ea7d615                          // fsub          v21.4s, v16.4s, v7.4s
  .long  0x4e26d4d4                          // fadd          v20.4s, v6.4s, v6.4s
  .long  0x6e20dea0                          // fmul          v0.4s, v21.4s, v0.4s
  .long  0x6e21dea1                          // fmul          v1.4s, v21.4s, v1.4s
  .long  0x6e22deb5                          // fmul          v21.4s, v21.4s, v2.4s
  .long  0x6e26dc42                          // fmul          v2.4s, v2.4s, v6.4s
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4e32d652                          // fadd          v18.4s, v18.4s, v18.4s
  .long  0x4ea3d610                          // fsub          v16.4s, v16.4s, v3.4s
  .long  0x6e34e4f4                          // fcmge         v20.4s, v7.4s, v20.4s
  .long  0x4e22d442                          // fadd          v2.4s, v2.4s, v2.4s
  .long  0x4eb2d672                          // fsub          v18.4s, v19.4s, v18.4s
  .long  0x4e30cc80                          // fmla          v0.4s, v4.4s, v16.4s
  .long  0x4e30cca1                          // fmla          v1.4s, v5.4s, v16.4s
  .long  0x4e30ccd5                          // fmla          v21.4s, v6.4s, v16.4s
  .long  0x6e721c54                          // bsl           v20.16b, v2.16b, v18.16b
  .long  0x4e31d400                          // fadd          v0.4s, v0.4s, v17.4s
  .long  0x4e36d421                          // fadd          v1.4s, v1.4s, v22.4s
  .long  0x4e34d6a2                          // fadd          v2.4s, v21.4s, v20.4s
  .long  0x4e27ce03                          // fmla          v3.4s, v16.4s, v7.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_softlight_aarch64
.globl _sk_softlight_aarch64
_sk_softlight_aarch64:
  .long  0x4ea0c8f5                          // fcmgt         v21.4s, v7.4s, #0.0
  .long  0x6e27fc96                          // fdiv          v22.4s, v4.4s, v7.4s
  .long  0x6e27fcb8                          // fdiv          v24.4s, v5.4s, v7.4s
  .long  0x6e27fcd9                          // fdiv          v25.4s, v6.4s, v7.4s
  .long  0x4e351ed6                          // and           v22.16b, v22.16b, v21.16b
  .long  0x4e351f18                          // and           v24.16b, v24.16b, v21.16b
  .long  0x4e351f35                          // and           v21.16b, v25.16b, v21.16b
  .long  0x6ea1dad9                          // frsqrte       v25.4s, v22.4s
  .long  0x6e39df3d                          // fmul          v29.4s, v25.4s, v25.4s
  .long  0x4ebdfedd                          // frsqrts       v29.4s, v22.4s, v29.4s
  .long  0x6e3ddf39                          // fmul          v25.4s, v25.4s, v29.4s
  .long  0x4ea1db3d                          // frecpe        v29.4s, v25.4s
  .long  0x6ea0fada                          // fneg          v26.4s, v22.4s
  .long  0x6ea1db1b                          // frsqrte       v27.4s, v24.4s
  .long  0x4e3dff39                          // frecps        v25.4s, v25.4s, v29.4s
  .long  0x4e3dcf3a                          // fmla          v26.4s, v25.4s, v29.4s
  .long  0x6e3bdf7d                          // fmul          v29.4s, v27.4s, v27.4s
  .long  0x4ebdff1d                          // frsqrts       v29.4s, v24.4s, v29.4s
  .long  0x6e3ddf7b                          // fmul          v27.4s, v27.4s, v29.4s
  .long  0x4ea1db7d                          // frecpe        v29.4s, v27.4s
  .long  0x6ea0fb1c                          // fneg          v28.4s, v24.4s
  .long  0x6ea1dab9                          // frsqrte       v25.4s, v21.4s
  .long  0x4e3dff7b                          // frecps        v27.4s, v27.4s, v29.4s
  .long  0x4e3dcf7c                          // fmla          v28.4s, v27.4s, v29.4s
  .long  0x6e39df3d                          // fmul          v29.4s, v25.4s, v25.4s
  .long  0x4ebdfebd                          // frsqrts       v29.4s, v21.4s, v29.4s
  .long  0x6e3ddf39                          // fmul          v25.4s, v25.4s, v29.4s
  .long  0x4ea1db3d                          // frecpe        v29.4s, v25.4s
  .long  0x6ea0fabb                          // fneg          v27.4s, v21.4s
  .long  0x4e3dff39                          // frecps        v25.4s, v25.4s, v29.4s
  .long  0x4e3dcf3b                          // fmla          v27.4s, v25.4s, v29.4s
  .long  0x4e36d6d9                          // fadd          v25.4s, v22.4s, v22.4s
  .long  0x4f07f613                          // fmov          v19.4s, #-1.000000000000000000e+00
  .long  0x4e39d739                          // fadd          v25.4s, v25.4s, v25.4s
  .long  0x4e24d497                          // fadd          v23.4s, v4.4s, v4.4s
  .long  0x4e33d6dd                          // fadd          v29.4s, v22.4s, v19.4s
  .long  0x4e39cf39                          // fmla          v25.4s, v25.4s, v25.4s
  .long  0x4f00f794                          // fmov          v20.4s, #7.000000000000000000e+00
  .long  0x6e39dfb9                          // fmul          v25.4s, v29.4s, v25.4s
  .long  0x4e37d6f7                          // fadd          v23.4s, v23.4s, v23.4s
  .long  0x6e37e4f7                          // fcmge         v23.4s, v7.4s, v23.4s
  .long  0x4e36ce99                          // fmla          v25.4s, v20.4s, v22.4s
  .long  0x6e7a1f37                          // bsl           v23.16b, v25.16b, v26.16b
  .long  0x4e38d719                          // fadd          v25.4s, v24.4s, v24.4s
  .long  0x4e39d739                          // fadd          v25.4s, v25.4s, v25.4s
  .long  0x4e33d71a                          // fadd          v26.4s, v24.4s, v19.4s
  .long  0x4e39cf39                          // fmla          v25.4s, v25.4s, v25.4s
  .long  0x6e39df59                          // fmul          v25.4s, v26.4s, v25.4s
  .long  0x4e25d4ba                          // fadd          v26.4s, v5.4s, v5.4s
  .long  0x4e3ad75a                          // fadd          v26.4s, v26.4s, v26.4s
  .long  0x6e3ae4fa                          // fcmge         v26.4s, v7.4s, v26.4s
  .long  0x4e38ce99                          // fmla          v25.4s, v20.4s, v24.4s
  .long  0x6e7c1f3a                          // bsl           v26.16b, v25.16b, v28.16b
  .long  0x4e35d6bc                          // fadd          v28.4s, v21.4s, v21.4s
  .long  0x4e3cd79c                          // fadd          v28.4s, v28.4s, v28.4s
  .long  0x4e33d6b3                          // fadd          v19.4s, v21.4s, v19.4s
  .long  0x4e3ccf9c                          // fmla          v28.4s, v28.4s, v28.4s
  .long  0x6e3cde73                          // fmul          v19.4s, v19.4s, v28.4s
  .long  0x4e35ce93                          // fmla          v19.4s, v20.4s, v21.4s
  .long  0x4e26d4d4                          // fadd          v20.4s, v6.4s, v6.4s
  .long  0x4e34d694                          // fadd          v20.4s, v20.4s, v20.4s
  .long  0x4f03f612                          // fmov          v18.4s, #1.000000000000000000e+00
  .long  0x6e34e4f4                          // fcmge         v20.4s, v7.4s, v20.4s
  .long  0x4e20d411                          // fadd          v17.4s, v0.4s, v0.4s
  .long  0x6e7b1e74                          // bsl           v20.16b, v19.16b, v27.16b
  .long  0x4ea7d65b                          // fsub          v27.4s, v18.4s, v7.4s
  .long  0x4ea31c70                          // mov           v16.16b, v3.16b
  .long  0x4e21d43d                          // fadd          v29.4s, v1.4s, v1.4s
  .long  0x4e22d45c                          // fadd          v28.4s, v2.4s, v2.4s
  .long  0x6e20df60                          // fmul          v0.4s, v27.4s, v0.4s
  .long  0x6e21df61                          // fmul          v1.4s, v27.4s, v1.4s
  .long  0x6e22df62                          // fmul          v2.4s, v27.4s, v2.4s
  .long  0x4ea3d63b                          // fsub          v27.4s, v17.4s, v3.4s
  .long  0x4eb6d656                          // fsub          v22.4s, v18.4s, v22.4s
  .long  0x4ea31c79                          // mov           v25.16b, v3.16b
  .long  0x4e3bced0                          // fmla          v16.4s, v22.4s, v27.4s
  .long  0x4ea3d7b6                          // fsub          v22.4s, v29.4s, v3.4s
  .long  0x4eb8d658                          // fsub          v24.4s, v18.4s, v24.4s
  .long  0x4ea31c73                          // mov           v19.16b, v3.16b
  .long  0x4e36cf19                          // fmla          v25.4s, v24.4s, v22.4s
  .long  0x4ea3d798                          // fsub          v24.4s, v28.4s, v3.4s
  .long  0x4eb5d655                          // fsub          v21.4s, v18.4s, v21.4s
  .long  0x4e38ceb3                          // fmla          v19.4s, v21.4s, v24.4s
  .long  0x6e27df7b                          // fmul          v27.4s, v27.4s, v7.4s
  .long  0x6e27ded6                          // fmul          v22.4s, v22.4s, v7.4s
  .long  0x6e27df18                          // fmul          v24.4s, v24.4s, v7.4s
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x6e37df77                          // fmul          v23.4s, v27.4s, v23.4s
  .long  0x6e3aded6                          // fmul          v22.4s, v22.4s, v26.4s
  .long  0x6e34df14                          // fmul          v20.4s, v24.4s, v20.4s
  .long  0x4ea3d652                          // fsub          v18.4s, v18.4s, v3.4s
  .long  0x6e31e471                          // fcmge         v17.4s, v3.4s, v17.4s
  .long  0x6e3de475                          // fcmge         v21.4s, v3.4s, v29.4s
  .long  0x6e3ce47c                          // fcmge         v28.4s, v3.4s, v28.4s
  .long  0x6e24de10                          // fmul          v16.4s, v16.4s, v4.4s
  .long  0x6e25df39                          // fmul          v25.4s, v25.4s, v5.4s
  .long  0x6e26de73                          // fmul          v19.4s, v19.4s, v6.4s
  .long  0x4e23cc97                          // fmla          v23.4s, v4.4s, v3.4s
  .long  0x4e23ccb6                          // fmla          v22.4s, v5.4s, v3.4s
  .long  0x4e23ccd4                          // fmla          v20.4s, v6.4s, v3.4s
  .long  0x4e32cc80                          // fmla          v0.4s, v4.4s, v18.4s
  .long  0x4e32cca1                          // fmla          v1.4s, v5.4s, v18.4s
  .long  0x4e32ccc2                          // fmla          v2.4s, v6.4s, v18.4s
  .long  0x6e771e11                          // bsl           v17.16b, v16.16b, v23.16b
  .long  0x6e761f35                          // bsl           v21.16b, v25.16b, v22.16b
  .long  0x6e741e7c                          // bsl           v28.16b, v19.16b, v20.16b
  .long  0x4e31d400                          // fadd          v0.4s, v0.4s, v17.4s
  .long  0x4e35d421                          // fadd          v1.4s, v1.4s, v21.4s
  .long  0x4e3cd442                          // fadd          v2.4s, v2.4s, v28.4s
  .long  0x4e27ce43                          // fmla          v3.4s, v18.4s, v7.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_clamp_0_aarch64
.globl _sk_clamp_0_aarch64
_sk_clamp_0_aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x6f00e410                          // movi          v16.2d, #0x0
  .long  0x4e30f400                          // fmax          v0.4s, v0.4s, v16.4s
  .long  0x4e30f421                          // fmax          v1.4s, v1.4s, v16.4s
  .long  0x4e30f442                          // fmax          v2.4s, v2.4s, v16.4s
  .long  0x4e30f463                          // fmax          v3.4s, v3.4s, v16.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_clamp_1_aarch64
.globl _sk_clamp_1_aarch64
_sk_clamp_1_aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x4eb0f400                          // fmin          v0.4s, v0.4s, v16.4s
  .long  0x4eb0f421                          // fmin          v1.4s, v1.4s, v16.4s
  .long  0x4eb0f442                          // fmin          v2.4s, v2.4s, v16.4s
  .long  0x4eb0f463                          // fmin          v3.4s, v3.4s, v16.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_clamp_a_aarch64
.globl _sk_clamp_a_aarch64
_sk_clamp_a_aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x4eb0f463                          // fmin          v3.4s, v3.4s, v16.4s
  .long  0x4ea3f400                          // fmin          v0.4s, v0.4s, v3.4s
  .long  0x4ea3f421                          // fmin          v1.4s, v1.4s, v3.4s
  .long  0x4ea3f442                          // fmin          v2.4s, v2.4s, v3.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_set_rgb_aarch64
.globl _sk_set_rgb_aarch64
_sk_set_rgb_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0xaa0803e9                          // mov           x9, x8
  .long  0x4ddfc920                          // ld1r          {v0.4s}, [x9], #4
  .long  0x91002108                          // add           x8, x8, #0x8
  .long  0x4d40c902                          // ld1r          {v2.4s}, [x8]
  .long  0x4d40c921                          // ld1r          {v1.4s}, [x9]
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_swap_rb_aarch64
.globl _sk_swap_rb_aarch64
_sk_swap_rb_aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4ea01c10                          // mov           v16.16b, v0.16b
  .long  0x4ea21c40                          // mov           v0.16b, v2.16b
  .long  0x4eb01e02                          // mov           v2.16b, v16.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_swap_aarch64
.globl _sk_swap_aarch64
_sk_swap_aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4ea31c70                          // mov           v16.16b, v3.16b
  .long  0x4ea21c51                          // mov           v17.16b, v2.16b
  .long  0x4ea11c32                          // mov           v18.16b, v1.16b
  .long  0x4ea01c13                          // mov           v19.16b, v0.16b
  .long  0x4ea41c80                          // mov           v0.16b, v4.16b
  .long  0x4ea51ca1                          // mov           v1.16b, v5.16b
  .long  0x4ea61cc2                          // mov           v2.16b, v6.16b
  .long  0x4ea71ce3                          // mov           v3.16b, v7.16b
  .long  0x4eb31e64                          // mov           v4.16b, v19.16b
  .long  0x4eb21e45                          // mov           v5.16b, v18.16b
  .long  0x4eb11e26                          // mov           v6.16b, v17.16b
  .long  0x4eb01e07                          // mov           v7.16b, v16.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_move_src_dst_aarch64
.globl _sk_move_src_dst_aarch64
_sk_move_src_dst_aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4ea01c04                          // mov           v4.16b, v0.16b
  .long  0x4ea11c25                          // mov           v5.16b, v1.16b
  .long  0x4ea21c46                          // mov           v6.16b, v2.16b
  .long  0x4ea31c67                          // mov           v7.16b, v3.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_move_dst_src_aarch64
.globl _sk_move_dst_src_aarch64
_sk_move_dst_src_aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4ea41c80                          // mov           v0.16b, v4.16b
  .long  0x4ea51ca1                          // mov           v1.16b, v5.16b
  .long  0x4ea61cc2                          // mov           v2.16b, v6.16b
  .long  0x4ea71ce3                          // mov           v3.16b, v7.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_premul_aarch64
.globl _sk_premul_aarch64
_sk_premul_aarch64:
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x6e23dc00                          // fmul          v0.4s, v0.4s, v3.4s
  .long  0x6e23dc21                          // fmul          v1.4s, v1.4s, v3.4s
  .long  0x6e23dc42                          // fmul          v2.4s, v2.4s, v3.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_unpremul_aarch64
.globl _sk_unpremul_aarch64
_sk_unpremul_aarch64:
  .long  0x4f03f611                          // fmov          v17.4s, #1.000000000000000000e+00
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4ea0d870                          // fcmeq         v16.4s, v3.4s, #0.0
  .long  0x6e23fe31                          // fdiv          v17.4s, v17.4s, v3.4s
  .long  0x4e701e30                          // bic           v16.16b, v17.16b, v16.16b
  .long  0x6e20de00                          // fmul          v0.4s, v16.4s, v0.4s
  .long  0x6e21de01                          // fmul          v1.4s, v16.4s, v1.4s
  .long  0x6e22de02                          // fmul          v2.4s, v16.4s, v2.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_from_srgb_aarch64
.globl _sk_from_srgb_aarch64
_sk_from_srgb_aarch64:
  .long  0x52a7d328                          // mov           w8, #0x3e990000
  .long  0x72933348                          // movk          w8, #0x999a
  .long  0x4e040d10                          // dup           v16.4s, w8
  .long  0x52a7e648                          // mov           w8, #0x3f320000
  .long  0x7291eb88                          // movk          w8, #0x8f5c
  .long  0x4e040d11                          // dup           v17.4s, w8
  .long  0x52a76468                          // mov           w8, #0x3b230000
  .long  0x729ae148                          // movk          w8, #0xd70a
  .long  0x4e040d12                          // dup           v18.4s, w8
  .long  0x52a7b3c8                          // mov           w8, #0x3d9e0000
  .long  0x72907228                          // movk          w8, #0x8391
  .long  0x6e22dc54                          // fmul          v20.4s, v2.4s, v2.4s
  .long  0x4eb11e35                          // mov           v21.16b, v17.16b
  .long  0x4eb11e37                          // mov           v23.16b, v17.16b
  .long  0x4e22ce11                          // fmla          v17.4s, v16.4s, v2.4s
  .long  0x4eb21e56                          // mov           v22.16b, v18.16b
  .long  0x4eb21e58                          // mov           v24.16b, v18.16b
  .long  0x4e34ce32                          // fmla          v18.4s, v17.4s, v20.4s
  .long  0x4e040d11                          // dup           v17.4s, w8
  .long  0x52a7ac28                          // mov           w8, #0x3d610000
  .long  0x6e20dc13                          // fmul          v19.4s, v0.4s, v0.4s
  .long  0x7288f5c8                          // movk          w8, #0x47ae
  .long  0x4e20ce15                          // fmla          v21.4s, v16.4s, v0.4s
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x6e21dc34                          // fmul          v20.4s, v1.4s, v1.4s
  .long  0x4e33ceb6                          // fmla          v22.4s, v21.4s, v19.4s
  .long  0x4e040d13                          // dup           v19.4s, w8
  .long  0x4e21ce17                          // fmla          v23.4s, v16.4s, v1.4s
  .long  0x6e31dc15                          // fmul          v21.4s, v0.4s, v17.4s
  .long  0x6ea0e660                          // fcmgt         v0.4s, v19.4s, v0.4s
  .long  0x6e31dc30                          // fmul          v16.4s, v1.4s, v17.4s
  .long  0x6ea1e661                          // fcmgt         v1.4s, v19.4s, v1.4s
  .long  0x6e31dc51                          // fmul          v17.4s, v2.4s, v17.4s
  .long  0x6ea2e662                          // fcmgt         v2.4s, v19.4s, v2.4s
  .long  0x4e34cef8                          // fmla          v24.4s, v23.4s, v20.4s
  .long  0x6e761ea0                          // bsl           v0.16b, v21.16b, v22.16b
  .long  0x6e781e01                          // bsl           v1.16b, v16.16b, v24.16b
  .long  0x6e721e22                          // bsl           v2.16b, v17.16b, v18.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_to_srgb_aarch64
.globl _sk_to_srgb_aarch64
_sk_to_srgb_aarch64:
  .long  0x52a828e8                          // mov           w8, #0x41470000
  .long  0x728b8528                          // movk          w8, #0x5c29
  .long  0x4e040d12                          // dup           v18.4s, w8
  .long  0x52a7e608                          // mov           w8, #0x3f300000
  .long  0x728df9c8                          // movk          w8, #0x6fce
  .long  0x6ea1d811                          // frsqrte       v17.4s, v0.4s
  .long  0x4e040d13                          // dup           v19.4s, w8
  .long  0x52b7b948                          // mov           w8, #0xbdca0000
  .long  0x728af508                          // movk          w8, #0x57a8
  .long  0x6ea1d834                          // frsqrte       v20.4s, v1.4s
  .long  0x6e31de36                          // fmul          v22.4s, v17.4s, v17.4s
  .long  0x4e040d10                          // dup           v16.4s, w8
  .long  0x52a77188                          // mov           w8, #0x3b8c0000
  .long  0x6ea1d855                          // frsqrte       v21.4s, v2.4s
  .long  0x6e34de98                          // fmul          v24.4s, v20.4s, v20.4s
  .long  0x4eb6fc16                          // frsqrts       v22.4s, v0.4s, v22.4s
  .long  0x729ce088                          // movk          w8, #0xe704
  .long  0x6e35deb9                          // fmul          v25.4s, v21.4s, v21.4s
  .long  0x4eb8fc38                          // frsqrts       v24.4s, v1.4s, v24.4s
  .long  0x6e36de31                          // fmul          v17.4s, v17.4s, v22.4s
  .long  0x4e040d17                          // dup           v23.4s, w8
  .long  0x4eb9fc59                          // frsqrts       v25.4s, v2.4s, v25.4s
  .long  0x6e38de94                          // fmul          v20.4s, v20.4s, v24.4s
  .long  0x4ea1da36                          // frecpe        v22.4s, v17.4s
  .long  0x6e32dc1a                          // fmul          v26.4s, v0.4s, v18.4s
  .long  0x6ea0e6e0                          // fcmgt         v0.4s, v23.4s, v0.4s
  .long  0x6e32dc3c                          // fmul          v28.4s, v1.4s, v18.4s
  .long  0x6ea1e6e1                          // fcmgt         v1.4s, v23.4s, v1.4s
  .long  0x6e32dc52                          // fmul          v18.4s, v2.4s, v18.4s
  .long  0x6ea2e6e2                          // fcmgt         v2.4s, v23.4s, v2.4s
  .long  0x6e39deb5                          // fmul          v21.4s, v21.4s, v25.4s
  .long  0x4ea1da97                          // frecpe        v23.4s, v20.4s
  .long  0x4e36fe39                          // frecps        v25.4s, v17.4s, v22.4s
  .long  0x4ea1dab8                          // frecpe        v24.4s, v21.4s
  .long  0x6e39ded6                          // fmul          v22.4s, v22.4s, v25.4s
  .long  0x4e37fe99                          // frecps        v25.4s, v20.4s, v23.4s
  .long  0x4eb01e1b                          // mov           v27.16b, v16.16b
  .long  0x6e39def7                          // fmul          v23.4s, v23.4s, v25.4s
  .long  0x4e38feb9                          // frecps        v25.4s, v21.4s, v24.4s
  .long  0x6e39df18                          // fmul          v24.4s, v24.4s, v25.4s
  .long  0x4eb01e19                          // mov           v25.16b, v16.16b
  .long  0x4e36ce7b                          // fmla          v27.4s, v19.4s, v22.4s
  .long  0x6ea1da36                          // frsqrte       v22.4s, v17.4s
  .long  0x4e37ce79                          // fmla          v25.4s, v19.4s, v23.4s
  .long  0x6ea1da97                          // frsqrte       v23.4s, v20.4s
  .long  0x4e38ce70                          // fmla          v16.4s, v19.4s, v24.4s
  .long  0x6e36ded8                          // fmul          v24.4s, v22.4s, v22.4s
  .long  0x6ea1dab3                          // frsqrte       v19.4s, v21.4s
  .long  0x4eb8fe31                          // frsqrts       v17.4s, v17.4s, v24.4s
  .long  0x6e37def8                          // fmul          v24.4s, v23.4s, v23.4s
  .long  0x4eb8fe94                          // frsqrts       v20.4s, v20.4s, v24.4s
  .long  0x6e33de78                          // fmul          v24.4s, v19.4s, v19.4s
  .long  0x52a7da48                          // mov           w8, #0x3ed20000
  .long  0x4eb8feb5                          // frsqrts       v21.4s, v21.4s, v24.4s
  .long  0x7290f848                          // movk          w8, #0x87c2
  .long  0x6e31ded1                          // fmul          v17.4s, v22.4s, v17.4s
  .long  0x6e34def4                          // fmul          v20.4s, v23.4s, v20.4s
  .long  0x6e35de73                          // fmul          v19.4s, v19.4s, v21.4s
  .long  0x4e040d15                          // dup           v21.4s, w8
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x4e31cebb                          // fmla          v27.4s, v21.4s, v17.4s
  .long  0x4f03f611                          // fmov          v17.4s, #1.000000000000000000e+00
  .long  0x4e34ceb9                          // fmla          v25.4s, v21.4s, v20.4s
  .long  0x4e33ceb0                          // fmla          v16.4s, v21.4s, v19.4s
  .long  0x4ebbf633                          // fmin          v19.4s, v17.4s, v27.4s
  .long  0x4eb9f634                          // fmin          v20.4s, v17.4s, v25.4s
  .long  0x4eb0f630                          // fmin          v16.4s, v17.4s, v16.4s
  .long  0x6e731f40                          // bsl           v0.16b, v26.16b, v19.16b
  .long  0x6e741f81                          // bsl           v1.16b, v28.16b, v20.16b
  .long  0x6e701e42                          // bsl           v2.16b, v18.16b, v16.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_scale_1_float_aarch64
.globl _sk_scale_1_float_aarch64
_sk_scale_1_float_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0xbd400110                          // ldr           s16, [x8]
  .long  0x4f909000                          // fmul          v0.4s, v0.4s, v16.s[0]
  .long  0x4f909021                          // fmul          v1.4s, v1.4s, v16.s[0]
  .long  0x4f909042                          // fmul          v2.4s, v2.4s, v16.s[0]
  .long  0x4f909063                          // fmul          v3.4s, v3.4s, v16.s[0]
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_scale_u8_aarch64
.globl _sk_scale_u8_aarch64
_sk_scale_u8_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0x52a77009                          // mov           w9, #0x3b800000
  .long  0x72901029                          // movk          w9, #0x8081
  .long  0x4e040d30                          // dup           v16.4s, w9
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x8b000108                          // add           x8, x8, x0
  .long  0x39400109                          // ldrb          w9, [x8]
  .long  0x3940050a                          // ldrb          w10, [x8, #1]
  .long  0x3940090b                          // ldrb          w11, [x8, #2]
  .long  0x39400d08                          // ldrb          w8, [x8, #3]
  .long  0x4e021d31                          // mov           v17.h[0], w9
  .long  0x4e061d51                          // mov           v17.h[1], w10
  .long  0x4e0a1d71                          // mov           v17.h[2], w11
  .long  0x4e0e1d11                          // mov           v17.h[3], w8
  .long  0x2f10a631                          // uxtl          v17.4s, v17.4h
  .long  0x6e21da31                          // ucvtf         v17.4s, v17.4s
  .long  0x6e30de30                          // fmul          v16.4s, v17.4s, v16.4s
  .long  0x6e20de00                          // fmul          v0.4s, v16.4s, v0.4s
  .long  0x6e21de01                          // fmul          v1.4s, v16.4s, v1.4s
  .long  0x6e22de02                          // fmul          v2.4s, v16.4s, v2.4s
  .long  0x6e23de03                          // fmul          v3.4s, v16.4s, v3.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_lerp_1_float_aarch64
.globl _sk_lerp_1_float_aarch64
_sk_lerp_1_float_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0x4ea4d411                          // fsub          v17.4s, v0.4s, v4.4s
  .long  0x4ea41c80                          // mov           v0.16b, v4.16b
  .long  0x4ea5d432                          // fsub          v18.4s, v1.4s, v5.4s
  .long  0xbd400110                          // ldr           s16, [x8]
  .long  0x4ea51ca1                          // mov           v1.16b, v5.16b
  .long  0x4f901220                          // fmla          v0.4s, v17.4s, v16.s[0]
  .long  0x4ea6d451                          // fsub          v17.4s, v2.4s, v6.4s
  .long  0x4f901241                          // fmla          v1.4s, v18.4s, v16.s[0]
  .long  0x4ea61cc2                          // mov           v2.16b, v6.16b
  .long  0x4ea7d472                          // fsub          v18.4s, v3.4s, v7.4s
  .long  0x4ea71ce3                          // mov           v3.16b, v7.16b
  .long  0x4f901222                          // fmla          v2.4s, v17.4s, v16.s[0]
  .long  0x4f901243                          // fmla          v3.4s, v18.4s, v16.s[0]
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_lerp_u8_aarch64
.globl _sk_lerp_u8_aarch64
_sk_lerp_u8_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0x52a77009                          // mov           w9, #0x3b800000
  .long  0x72901029                          // movk          w9, #0x8081
  .long  0x4e040d30                          // dup           v16.4s, w9
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x4ea4d412                          // fsub          v18.4s, v0.4s, v4.4s
  .long  0x8b000108                          // add           x8, x8, x0
  .long  0x3940010a                          // ldrb          w10, [x8]
  .long  0x39400509                          // ldrb          w9, [x8, #1]
  .long  0x3940090b                          // ldrb          w11, [x8, #2]
  .long  0x39400d08                          // ldrb          w8, [x8, #3]
  .long  0x4e021d51                          // mov           v17.h[0], w10
  .long  0x4e061d31                          // mov           v17.h[1], w9
  .long  0x4e0a1d71                          // mov           v17.h[2], w11
  .long  0x4e0e1d11                          // mov           v17.h[3], w8
  .long  0x2f10a620                          // uxtl          v0.4s, v17.4h
  .long  0x6e21d800                          // ucvtf         v0.4s, v0.4s
  .long  0x6e30dc10                          // fmul          v16.4s, v0.4s, v16.4s
  .long  0x4ea41c80                          // mov           v0.16b, v4.16b
  .long  0x4ea5d431                          // fsub          v17.4s, v1.4s, v5.4s
  .long  0x4ea51ca1                          // mov           v1.16b, v5.16b
  .long  0x4e32ce00                          // fmla          v0.4s, v16.4s, v18.4s
  .long  0x4ea6d452                          // fsub          v18.4s, v2.4s, v6.4s
  .long  0x4e31ce01                          // fmla          v1.4s, v16.4s, v17.4s
  .long  0x4ea61cc2                          // mov           v2.16b, v6.16b
  .long  0x4ea7d471                          // fsub          v17.4s, v3.4s, v7.4s
  .long  0x4ea71ce3                          // mov           v3.16b, v7.16b
  .long  0x4e32ce02                          // fmla          v2.4s, v16.4s, v18.4s
  .long  0x4e31ce03                          // fmla          v3.4s, v16.4s, v17.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_lerp_565_aarch64
.globl _sk_lerp_565_aarch64
_sk_lerp_565_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0xd37ff809                          // lsl           x9, x0, #1
  .long  0x4f072710                          // movi          v16.4s, #0xf8, lsl #8
  .long  0x4ea4d413                          // fsub          v19.4s, v0.4s, v4.4s
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0xfc696903                          // ldr           d3, [x8, x9]
  .long  0x52a6f088                          // mov           w8, #0x37840000
  .long  0x72842108                          // movk          w8, #0x2108
  .long  0x4e040d11                          // dup           v17.4s, w8
  .long  0x2f10a463                          // uxtl          v3.4s, v3.4h
  .long  0x321b17e8                          // orr           w8, wzr, #0x7e0
  .long  0x4e301c60                          // and           v0.16b, v3.16b, v16.16b
  .long  0x4e040d12                          // dup           v18.4s, w8
  .long  0x52a74048                          // mov           w8, #0x3a020000
  .long  0x4e21d800                          // scvtf         v0.4s, v0.4s
  .long  0x72810428                          // movk          w8, #0x821
  .long  0x6e31dc10                          // fmul          v16.4s, v0.4s, v17.4s
  .long  0x4ea41c80                          // mov           v0.16b, v4.16b
  .long  0x4e33ce00                          // fmla          v0.4s, v16.4s, v19.4s
  .long  0x4f0007f0                          // movi          v16.4s, #0x1f
  .long  0x4e040d11                          // dup           v17.4s, w8
  .long  0x52a7a088                          // mov           w8, #0x3d040000
  .long  0x4e321c72                          // and           v18.16b, v3.16b, v18.16b
  .long  0x72842108                          // movk          w8, #0x2108
  .long  0x4e301c63                          // and           v3.16b, v3.16b, v16.16b
  .long  0x4ea6d450                          // fsub          v16.4s, v2.4s, v6.4s
  .long  0x4e21da42                          // scvtf         v2.4s, v18.4s
  .long  0x6e31dc51                          // fmul          v17.4s, v2.4s, v17.4s
  .long  0x4e040d02                          // dup           v2.4s, w8
  .long  0x4e21d863                          // scvtf         v3.4s, v3.4s
  .long  0x4ea5d433                          // fsub          v19.4s, v1.4s, v5.4s
  .long  0x4ea51ca1                          // mov           v1.16b, v5.16b
  .long  0x6e22dc63                          // fmul          v3.4s, v3.4s, v2.4s
  .long  0x4ea61cc2                          // mov           v2.16b, v6.16b
  .long  0x4e33ce21                          // fmla          v1.4s, v17.4s, v19.4s
  .long  0x4e30cc62                          // fmla          v2.4s, v3.4s, v16.4s
  .long  0x4f03f603                          // fmov          v3.4s, #1.000000000000000000e+00
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_load_tables_aarch64
.globl _sk_load_tables_aarch64
_sk_load_tables_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0xd37ef409                          // lsl           x9, x0, #2
  .long  0x6f00e620                          // movi          v0.2d, #0xff000000ff
  .long  0x52a7700b                          // mov           w11, #0x3b800000
  .long  0xa940310a                          // ldp           x10, x12, [x8]
  .long  0x7290102b                          // movk          w11, #0x8081
  .long  0x4e040d63                          // dup           v3.4s, w11
  .long  0x3ce96942                          // ldr           q2, [x10, x9]
  .long  0xa9412109                          // ldp           x9, x8, [x8, #16]
  .long  0x4e201c41                          // and           v1.16b, v2.16b, v0.16b
  .long  0x1e26002e                          // fmov          w14, s1
  .long  0x6f380450                          // ushr          v16.4s, v2.4s, #8
  .long  0x6f300451                          // ushr          v17.4s, v2.4s, #16
  .long  0x8b2e498e                          // add           x14, x12, w14, uxtw #2
  .long  0x0e0c3c2a                          // mov           w10, v1.s[1]
  .long  0x0e143c2b                          // mov           w11, v1.s[2]
  .long  0x0e1c3c2d                          // mov           w13, v1.s[3]
  .long  0x4e201e01                          // and           v1.16b, v16.16b, v0.16b
  .long  0x4e201e30                          // and           v16.16b, v17.16b, v0.16b
  .long  0x0d4081c0                          // ld1           {v0.s}[0], [x14]
  .long  0x8b2a498a                          // add           x10, x12, w10, uxtw #2
  .long  0xbc6b5991                          // ldr           s17, [x12, w11, uxtw #2]
  .long  0xbc6d5992                          // ldr           s18, [x12, w13, uxtw #2]
  .long  0x0e0c3c2b                          // mov           w11, v1.s[1]
  .long  0x0e143c2c                          // mov           w12, v1.s[2]
  .long  0x0e1c3c2d                          // mov           w13, v1.s[3]
  .long  0x1e26002e                          // fmov          w14, s1
  .long  0x8b2e492e                          // add           x14, x9, w14, uxtw #2
  .long  0xbc6c5933                          // ldr           s19, [x9, w12, uxtw #2]
  .long  0xbc6d5934                          // ldr           s20, [x9, w13, uxtw #2]
  .long  0x8b2b4929                          // add           x9, x9, w11, uxtw #2
  .long  0x1e26020b                          // fmov          w11, s16
  .long  0x6f280442                          // ushr          v2.4s, v2.4s, #24
  .long  0x0d409140                          // ld1           {v0.s}[1], [x10]
  .long  0x4e21d842                          // scvtf         v2.4s, v2.4s
  .long  0x8b2b490a                          // add           x10, x8, w11, uxtw #2
  .long  0x0d4081c1                          // ld1           {v1.s}[0], [x14]
  .long  0x6e23dc43                          // fmul          v3.4s, v2.4s, v3.4s
  .long  0x0d408142                          // ld1           {v2.s}[0], [x10]
  .long  0x0e0c3e0f                          // mov           w15, v16.s[1]
  .long  0x0e143e0c                          // mov           w12, v16.s[2]
  .long  0x8b2f490a                          // add           x10, x8, w15, uxtw #2
  .long  0x0e1c3e0d                          // mov           w13, v16.s[3]
  .long  0xbc6c5910                          // ldr           s16, [x8, w12, uxtw #2]
  .long  0x0d409121                          // ld1           {v1.s}[1], [x9]
  .long  0x0d409142                          // ld1           {v2.s}[1], [x10]
  .long  0x6e140620                          // mov           v0.s[2], v17.s[0]
  .long  0xbc6d5911                          // ldr           s17, [x8, w13, uxtw #2]
  .long  0x6e140661                          // mov           v1.s[2], v19.s[0]
  .long  0x6e140602                          // mov           v2.s[2], v16.s[0]
  .long  0x6e1c0640                          // mov           v0.s[3], v18.s[0]
  .long  0x6e1c0681                          // mov           v1.s[3], v20.s[0]
  .long  0x6e1c0622                          // mov           v2.s[3], v17.s[0]
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_load_a8_aarch64
.globl _sk_load_a8_aarch64
_sk_load_a8_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0x52a77009                          // mov           w9, #0x3b800000
  .long  0x72901029                          // movk          w9, #0x8081
  .long  0x4e040d22                          // dup           v2.4s, w9
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x6f00e400                          // movi          v0.2d, #0x0
  .long  0x6f00e401                          // movi          v1.2d, #0x0
  .long  0x8b000108                          // add           x8, x8, x0
  .long  0x3940010a                          // ldrb          w10, [x8]
  .long  0x3940050b                          // ldrb          w11, [x8, #1]
  .long  0x3940090c                          // ldrb          w12, [x8, #2]
  .long  0x39400d08                          // ldrb          w8, [x8, #3]
  .long  0x4e021d43                          // mov           v3.h[0], w10
  .long  0x4e061d63                          // mov           v3.h[1], w11
  .long  0x4e0a1d83                          // mov           v3.h[2], w12
  .long  0x4e0e1d03                          // mov           v3.h[3], w8
  .long  0x2f10a463                          // uxtl          v3.4s, v3.4h
  .long  0x6e21d863                          // ucvtf         v3.4s, v3.4s
  .long  0x6e22dc63                          // fmul          v3.4s, v3.4s, v2.4s
  .long  0x6f00e402                          // movi          v2.2d, #0x0
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_store_a8_aarch64
.globl _sk_store_a8_aarch64
_sk_store_a8_aarch64:
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0x52a86fe9                          // mov           w9, #0x437f0000
  .long  0x4e040d30                          // dup           v16.4s, w9
  .long  0x6e30dc70                          // fmul          v16.4s, v3.4s, v16.4s
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x6e21aa10                          // fcvtnu        v16.4s, v16.4s
  .long  0x0e612a10                          // xtn           v16.4h, v16.4s
  .long  0x0e0e3e09                          // umov          w9, v16.h[3]
  .long  0x8b000108                          // add           x8, x8, x0
  .long  0x39000d09                          // strb          w9, [x8, #3]
  .long  0x0e0a3e09                          // umov          w9, v16.h[2]
  .long  0x39000909                          // strb          w9, [x8, #2]
  .long  0x0e063e09                          // umov          w9, v16.h[1]
  .long  0x39000509                          // strb          w9, [x8, #1]
  .long  0x0e023e09                          // umov          w9, v16.h[0]
  .long  0x39000109                          // strb          w9, [x8]
  .long  0xf9400423                          // ldr           x3, [x1, #8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_load_g8_aarch64
.globl _sk_load_g8_aarch64
_sk_load_g8_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0x52a77009                          // mov           w9, #0x3b800000
  .long  0x72901029                          // movk          w9, #0x8081
  .long  0x4e040d20                          // dup           v0.4s, w9
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x4f03f603                          // fmov          v3.4s, #1.000000000000000000e+00
  .long  0x8b000108                          // add           x8, x8, x0
  .long  0x3940010a                          // ldrb          w10, [x8]
  .long  0x39400509                          // ldrb          w9, [x8, #1]
  .long  0x3940090b                          // ldrb          w11, [x8, #2]
  .long  0x39400d08                          // ldrb          w8, [x8, #3]
  .long  0x4e021d41                          // mov           v1.h[0], w10
  .long  0x4e061d21                          // mov           v1.h[1], w9
  .long  0x4e0a1d61                          // mov           v1.h[2], w11
  .long  0x4e0e1d01                          // mov           v1.h[3], w8
  .long  0x2f10a421                          // uxtl          v1.4s, v1.4h
  .long  0x6e21d821                          // ucvtf         v1.4s, v1.4s
  .long  0x6e20dc20                          // fmul          v0.4s, v1.4s, v0.4s
  .long  0x4ea01c01                          // mov           v1.16b, v0.16b
  .long  0x4ea01c02                          // mov           v2.16b, v0.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_load_565_aarch64
.globl _sk_load_565_aarch64
_sk_load_565_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0xd37ff809                          // lsl           x9, x0, #1
  .long  0x4f072701                          // movi          v1.4s, #0xf8, lsl #8
  .long  0x4f0007e3                          // movi          v3.4s, #0x1f
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0xfc696900                          // ldr           d0, [x8, x9]
  .long  0x321b17e8                          // orr           w8, wzr, #0x7e0
  .long  0x4e040d02                          // dup           v2.4s, w8
  .long  0x52a6f088                          // mov           w8, #0x37840000
  .long  0x72842108                          // movk          w8, #0x2108
  .long  0x2f10a400                          // uxtl          v0.4s, v0.4h
  .long  0x4e211c01                          // and           v1.16b, v0.16b, v1.16b
  .long  0x4e221c02                          // and           v2.16b, v0.16b, v2.16b
  .long  0x4e231c03                          // and           v3.16b, v0.16b, v3.16b
  .long  0x4e040d00                          // dup           v0.4s, w8
  .long  0x52a74048                          // mov           w8, #0x3a020000
  .long  0x72810428                          // movk          w8, #0x821
  .long  0x4e21d821                          // scvtf         v1.4s, v1.4s
  .long  0x6e20dc20                          // fmul          v0.4s, v1.4s, v0.4s
  .long  0x4e040d01                          // dup           v1.4s, w8
  .long  0x52a7a088                          // mov           w8, #0x3d040000
  .long  0x72842108                          // movk          w8, #0x2108
  .long  0x4e21d842                          // scvtf         v2.4s, v2.4s
  .long  0x6e21dc41                          // fmul          v1.4s, v2.4s, v1.4s
  .long  0x4e040d02                          // dup           v2.4s, w8
  .long  0x4e21d863                          // scvtf         v3.4s, v3.4s
  .long  0x6e22dc62                          // fmul          v2.4s, v3.4s, v2.4s
  .long  0x4f03f603                          // fmov          v3.4s, #1.000000000000000000e+00
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_store_565_aarch64
.globl _sk_store_565_aarch64
_sk_store_565_aarch64:
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0x52a84f8a                          // mov           w10, #0x427c0000
  .long  0x4f01f7f0                          // fmov          v16.4s, #3.100000000000000000e+01
  .long  0x4e040d52                          // dup           v18.4s, w10
  .long  0x6e30dc11                          // fmul          v17.4s, v0.4s, v16.4s
  .long  0x6e32dc32                          // fmul          v18.4s, v1.4s, v18.4s
  .long  0x6e21aa31                          // fcvtnu        v17.4s, v17.4s
  .long  0x6e21aa52                          // fcvtnu        v18.4s, v18.4s
  .long  0x6e30dc50                          // fmul          v16.4s, v2.4s, v16.4s
  .long  0x4f2b5631                          // shl           v17.4s, v17.4s, #11
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x4f255652                          // shl           v18.4s, v18.4s, #5
  .long  0x4eb11e51                          // orr           v17.16b, v18.16b, v17.16b
  .long  0x6e21aa10                          // fcvtnu        v16.4s, v16.4s
  .long  0x4eb01e30                          // orr           v16.16b, v17.16b, v16.16b
  .long  0xd37ff809                          // lsl           x9, x0, #1
  .long  0x0e612a10                          // xtn           v16.4h, v16.4s
  .long  0xfc296910                          // str           d16, [x8, x9]
  .long  0xf9400423                          // ldr           x3, [x1, #8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_load_4444_aarch64
.globl _sk_load_4444_aarch64
_sk_load_4444_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0xd37ff809                          // lsl           x9, x0, #1
  .long  0x4f072601                          // movi          v1.4s, #0xf0, lsl #8
  .long  0x4f0025e2                          // movi          v2.4s, #0xf, lsl #8
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x4f070603                          // movi          v3.4s, #0xf0
  .long  0x4f0005f0                          // movi          v16.4s, #0xf
  .long  0xfc696900                          // ldr           d0, [x8, x9]
  .long  0x52a6f108                          // mov           w8, #0x37880000
  .long  0x72911128                          // movk          w8, #0x8889
  .long  0x2f10a400                          // uxtl          v0.4s, v0.4h
  .long  0x4e211c01                          // and           v1.16b, v0.16b, v1.16b
  .long  0x4e221c02                          // and           v2.16b, v0.16b, v2.16b
  .long  0x4e231c03                          // and           v3.16b, v0.16b, v3.16b
  .long  0x4e301c10                          // and           v16.16b, v0.16b, v16.16b
  .long  0x4e040d00                          // dup           v0.4s, w8
  .long  0x52a73108                          // mov           w8, #0x39880000
  .long  0x72911128                          // movk          w8, #0x8889
  .long  0x4e21d821                          // scvtf         v1.4s, v1.4s
  .long  0x6e20dc20                          // fmul          v0.4s, v1.4s, v0.4s
  .long  0x4e040d01                          // dup           v1.4s, w8
  .long  0x52a77108                          // mov           w8, #0x3b880000
  .long  0x72911128                          // movk          w8, #0x8889
  .long  0x4e21d842                          // scvtf         v2.4s, v2.4s
  .long  0x6e21dc41                          // fmul          v1.4s, v2.4s, v1.4s
  .long  0x4e040d02                          // dup           v2.4s, w8
  .long  0x52a7b108                          // mov           w8, #0x3d880000
  .long  0x72911128                          // movk          w8, #0x8889
  .long  0x4e21d863                          // scvtf         v3.4s, v3.4s
  .long  0x6e22dc62                          // fmul          v2.4s, v3.4s, v2.4s
  .long  0x4e040d03                          // dup           v3.4s, w8
  .long  0x4e21da10                          // scvtf         v16.4s, v16.4s
  .long  0x6e23de03                          // fmul          v3.4s, v16.4s, v3.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_store_4444_aarch64
.globl _sk_store_4444_aarch64
_sk_store_4444_aarch64:
  .long  0x4f01f5d0                          // fmov          v16.4s, #1.500000000000000000e+01
  .long  0x6e30dc11                          // fmul          v17.4s, v0.4s, v16.4s
  .long  0x6e30dc32                          // fmul          v18.4s, v1.4s, v16.4s
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0x6e21aa31                          // fcvtnu        v17.4s, v17.4s
  .long  0x6e21aa52                          // fcvtnu        v18.4s, v18.4s
  .long  0x4f2c5631                          // shl           v17.4s, v17.4s, #12
  .long  0x4f285652                          // shl           v18.4s, v18.4s, #8
  .long  0x4eb11e51                          // orr           v17.16b, v18.16b, v17.16b
  .long  0x6e30dc52                          // fmul          v18.4s, v2.4s, v16.4s
  .long  0x6e21aa52                          // fcvtnu        v18.4s, v18.4s
  .long  0x6e30dc70                          // fmul          v16.4s, v3.4s, v16.4s
  .long  0x4f245652                          // shl           v18.4s, v18.4s, #4
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x4eb21e31                          // orr           v17.16b, v17.16b, v18.16b
  .long  0x6e21aa10                          // fcvtnu        v16.4s, v16.4s
  .long  0x4eb01e30                          // orr           v16.16b, v17.16b, v16.16b
  .long  0xd37ff809                          // lsl           x9, x0, #1
  .long  0x0e612a10                          // xtn           v16.4h, v16.4s
  .long  0xfc296910                          // str           d16, [x8, x9]
  .long  0xf9400423                          // ldr           x3, [x1, #8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_load_8888_aarch64
.globl _sk_load_8888_aarch64
_sk_load_8888_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0xd37ef409                          // lsl           x9, x0, #2
  .long  0x6f00e621                          // movi          v1.2d, #0xff000000ff
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x3ce96900                          // ldr           q0, [x8, x9]
  .long  0x52a77008                          // mov           w8, #0x3b800000
  .long  0x72901028                          // movk          w8, #0x8081
  .long  0x4e040d02                          // dup           v2.4s, w8
  .long  0x6f380410                          // ushr          v16.4s, v0.4s, #8
  .long  0x6f300411                          // ushr          v17.4s, v0.4s, #16
  .long  0x4e211c03                          // and           v3.16b, v0.16b, v1.16b
  .long  0x6f280400                          // ushr          v0.4s, v0.4s, #24
  .long  0x4e211e10                          // and           v16.16b, v16.16b, v1.16b
  .long  0x4e211e21                          // and           v1.16b, v17.16b, v1.16b
  .long  0x4e21d863                          // scvtf         v3.4s, v3.4s
  .long  0x4e21d811                          // scvtf         v17.4s, v0.4s
  .long  0x4e21da10                          // scvtf         v16.4s, v16.4s
  .long  0x4e21d832                          // scvtf         v18.4s, v1.4s
  .long  0x6e22dc60                          // fmul          v0.4s, v3.4s, v2.4s
  .long  0x6e22de23                          // fmul          v3.4s, v17.4s, v2.4s
  .long  0x6e22de01                          // fmul          v1.4s, v16.4s, v2.4s
  .long  0x6e22de42                          // fmul          v2.4s, v18.4s, v2.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_store_8888_aarch64
.globl _sk_store_8888_aarch64
_sk_store_8888_aarch64:
  .long  0x52a86fea                          // mov           w10, #0x437f0000
  .long  0x4e040d50                          // dup           v16.4s, w10
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0x6e30dc32                          // fmul          v18.4s, v1.4s, v16.4s
  .long  0x6e30dc11                          // fmul          v17.4s, v0.4s, v16.4s
  .long  0x6e21aa52                          // fcvtnu        v18.4s, v18.4s
  .long  0x6e21aa31                          // fcvtnu        v17.4s, v17.4s
  .long  0x4f285652                          // shl           v18.4s, v18.4s, #8
  .long  0x4eb11e51                          // orr           v17.16b, v18.16b, v17.16b
  .long  0x6e30dc52                          // fmul          v18.4s, v2.4s, v16.4s
  .long  0x6e30dc70                          // fmul          v16.4s, v3.4s, v16.4s
  .long  0x6e21aa52                          // fcvtnu        v18.4s, v18.4s
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x6e21aa10                          // fcvtnu        v16.4s, v16.4s
  .long  0x4f305652                          // shl           v18.4s, v18.4s, #16
  .long  0x4eb21e31                          // orr           v17.16b, v17.16b, v18.16b
  .long  0x4f385610                          // shl           v16.4s, v16.4s, #24
  .long  0xd37ef409                          // lsl           x9, x0, #2
  .long  0x4eb01e30                          // orr           v16.16b, v17.16b, v16.16b
  .long  0x3ca96910                          // str           q16, [x8, x9]
  .long  0xf9400423                          // ldr           x3, [x1, #8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_load_f16_aarch64
.globl _sk_load_f16_aarch64
_sk_load_f16_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x8b000d08                          // add           x8, x8, x0, lsl #3
  .long  0x0c400510                          // ld4           {v16.4h-v19.4h}, [x8]
  .long  0x0e217a00                          // fcvtl         v0.4s, v16.4h
  .long  0x0e217a21                          // fcvtl         v1.4s, v17.4h
  .long  0x0e217a42                          // fcvtl         v2.4s, v18.4h
  .long  0x0e217a63                          // fcvtl         v3.4s, v19.4h
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_store_f16_aarch64
.globl _sk_store_f16_aarch64
_sk_store_f16_aarch64:
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0x0e216810                          // fcvtn         v16.4h, v0.4s
  .long  0x0e216831                          // fcvtn         v17.4h, v1.4s
  .long  0x0e216852                          // fcvtn         v18.4h, v2.4s
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x0e216873                          // fcvtn         v19.4h, v3.4s
  .long  0x8b000d08                          // add           x8, x8, x0, lsl #3
  .long  0x0c000510                          // st4           {v16.4h-v19.4h}, [x8]
  .long  0xf9400423                          // ldr           x3, [x1, #8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_store_f32_aarch64
.globl _sk_store_f32_aarch64
_sk_store_f32_aarch64:
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x8b001108                          // add           x8, x8, x0, lsl #4
  .long  0x4c000900                          // st4           {v0.4s-v3.4s}, [x8]
  .long  0xf9400423                          // ldr           x3, [x1, #8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_clamp_x_aarch64
.globl _sk_clamp_x_aarch64
_sk_clamp_x_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0x6f00e411                          // movi          v17.2d, #0x0
  .long  0x4e20f620                          // fmax          v0.4s, v17.4s, v0.4s
  .long  0x6f07e7f1                          // movi          v17.2d, #0xffffffffffffffff
  .long  0x4d40c910                          // ld1r          {v16.4s}, [x8]
  .long  0x4eb18610                          // add           v16.4s, v16.4s, v17.4s
  .long  0x4eb0f400                          // fmin          v0.4s, v0.4s, v16.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_clamp_y_aarch64
.globl _sk_clamp_y_aarch64
_sk_clamp_y_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0x6f00e411                          // movi          v17.2d, #0x0
  .long  0x4e21f621                          // fmax          v1.4s, v17.4s, v1.4s
  .long  0x6f07e7f1                          // movi          v17.2d, #0xffffffffffffffff
  .long  0x4d40c910                          // ld1r          {v16.4s}, [x8]
  .long  0x4eb18610                          // add           v16.4s, v16.4s, v17.4s
  .long  0x4eb0f421                          // fmin          v1.4s, v1.4s, v16.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_repeat_x_aarch64
.globl _sk_repeat_x_aarch64
_sk_repeat_x_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0x6f07e7f1                          // movi          v17.2d, #0xffffffffffffffff
  .long  0xbd400110                          // ldr           s16, [x8]
  .long  0x4e040612                          // dup           v18.4s, v16.s[0]
  .long  0x4eb18651                          // add           v17.4s, v18.4s, v17.4s
  .long  0x6e32fc12                          // fdiv          v18.4s, v0.4s, v18.4s
  .long  0x4e219a52                          // frintm        v18.4s, v18.4s
  .long  0x4f905240                          // fmls          v0.4s, v18.4s, v16.s[0]
  .long  0x4eb1f400                          // fmin          v0.4s, v0.4s, v17.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_repeat_y_aarch64
.globl _sk_repeat_y_aarch64
_sk_repeat_y_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0x6f07e7f1                          // movi          v17.2d, #0xffffffffffffffff
  .long  0xbd400110                          // ldr           s16, [x8]
  .long  0x4e040612                          // dup           v18.4s, v16.s[0]
  .long  0x4eb18651                          // add           v17.4s, v18.4s, v17.4s
  .long  0x6e32fc32                          // fdiv          v18.4s, v1.4s, v18.4s
  .long  0x4e219a52                          // frintm        v18.4s, v18.4s
  .long  0x4f905241                          // fmls          v1.4s, v18.4s, v16.s[0]
  .long  0x4eb1f421                          // fmin          v1.4s, v1.4s, v17.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_mirror_x_aarch64
.globl _sk_mirror_x_aarch64
_sk_mirror_x_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0xbd400110                          // ldr           s16, [x8]
  .long  0x4e040611                          // dup           v17.4s, v16.s[0]
  .long  0x1e302a10                          // fadd          s16, s16, s16
  .long  0x4eb1d400                          // fsub          v0.4s, v0.4s, v17.4s
  .long  0x4e040612                          // dup           v18.4s, v16.s[0]
  .long  0x6e32fc12                          // fdiv          v18.4s, v0.4s, v18.4s
  .long  0x4e219a52                          // frintm        v18.4s, v18.4s
  .long  0x4f905240                          // fmls          v0.4s, v18.4s, v16.s[0]
  .long  0x6f07e7f0                          // movi          v16.2d, #0xffffffffffffffff
  .long  0x4eb1d400                          // fsub          v0.4s, v0.4s, v17.4s
  .long  0x4eb08630                          // add           v16.4s, v17.4s, v16.4s
  .long  0x4ea0f800                          // fabs          v0.4s, v0.4s
  .long  0x4eb0f400                          // fmin          v0.4s, v0.4s, v16.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_mirror_y_aarch64
.globl _sk_mirror_y_aarch64
_sk_mirror_y_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0xbd400110                          // ldr           s16, [x8]
  .long  0x4e040611                          // dup           v17.4s, v16.s[0]
  .long  0x1e302a10                          // fadd          s16, s16, s16
  .long  0x4eb1d421                          // fsub          v1.4s, v1.4s, v17.4s
  .long  0x4e040612                          // dup           v18.4s, v16.s[0]
  .long  0x6e32fc32                          // fdiv          v18.4s, v1.4s, v18.4s
  .long  0x4e219a52                          // frintm        v18.4s, v18.4s
  .long  0x4f905241                          // fmls          v1.4s, v18.4s, v16.s[0]
  .long  0x6f07e7f0                          // movi          v16.2d, #0xffffffffffffffff
  .long  0x4eb1d421                          // fsub          v1.4s, v1.4s, v17.4s
  .long  0x4eb08630                          // add           v16.4s, v17.4s, v16.4s
  .long  0x4ea0f821                          // fabs          v1.4s, v1.4s
  .long  0x4eb0f421                          // fmin          v1.4s, v1.4s, v16.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_luminance_to_alpha_aarch64
.globl _sk_luminance_to_alpha_aarch64
_sk_luminance_to_alpha_aarch64:
  .long  0x52a7cb28                          // mov           w8, #0x3e590000
  .long  0x72967a08                          // movk          w8, #0xb3d0
  .long  0x4e040d11                          // dup           v17.4s, w8
  .long  0x52a7e6e8                          // mov           w8, #0x3f370000
  .long  0x7282eb28                          // movk          w8, #0x1759
  .long  0x4ea01c10                          // mov           v16.16b, v0.16b
  .long  0x4e040d00                          // dup           v0.4s, w8
  .long  0x52a7b268                          // mov           w8, #0x3d930000
  .long  0xf8408423                          // ldr           x3, [x1], #8
  .long  0x729bb308                          // movk          w8, #0xdd98
  .long  0x6e20dc23                          // fmul          v3.4s, v1.4s, v0.4s
  .long  0x4e30ce23                          // fmla          v3.4s, v17.4s, v16.4s
  .long  0x4e040d10                          // dup           v16.4s, w8
  .long  0x6f00e400                          // movi          v0.2d, #0x0
  .long  0x6f00e401                          // movi          v1.2d, #0x0
  .long  0x4e22ce03                          // fmla          v3.4s, v16.4s, v2.4s
  .long  0x6f00e402                          // movi          v2.2d, #0x0
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_matrix_2x3_aarch64
.globl _sk_matrix_2x3_aarch64
_sk_matrix_2x3_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0xaa0803e9                          // mov           x9, x8
  .long  0x9100410a                          // add           x10, x8, #0x10
  .long  0x4ddfc932                          // ld1r          {v18.4s}, [x9], #4
  .long  0x4d40c950                          // ld1r          {v16.4s}, [x10]
  .long  0x2d415113                          // ldp           s19, s20, [x8, #8]
  .long  0x9100510a                          // add           x10, x8, #0x14
  .long  0x4d40c951                          // ld1r          {v17.4s}, [x10]
  .long  0x4f931030                          // fmla          v16.4s, v1.4s, v19.s[0]
  .long  0xbd400133                          // ldr           s19, [x9]
  .long  0x4f941031                          // fmla          v17.4s, v1.4s, v20.s[0]
  .long  0x4e20ce50                          // fmla          v16.4s, v18.4s, v0.4s
  .long  0x4f931011                          // fmla          v17.4s, v0.4s, v19.s[0]
  .long  0x4eb01e00                          // mov           v0.16b, v16.16b
  .long  0x4eb11e21                          // mov           v1.16b, v17.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_matrix_3x4_aarch64
.globl _sk_matrix_3x4_aarch64
_sk_matrix_3x4_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0xaa0803e9                          // mov           x9, x8
  .long  0x9100910a                          // add           x10, x8, #0x24
  .long  0x4ddfc933                          // ld1r          {v19.4s}, [x9], #4
  .long  0x4d40c950                          // ld1r          {v16.4s}, [x10]
  .long  0x9100a10a                          // add           x10, x8, #0x28
  .long  0x4d40c951                          // ld1r          {v17.4s}, [x10]
  .long  0x9100b10a                          // add           x10, x8, #0x2c
  .long  0x2d435514                          // ldp           s20, s21, [x8, #24]
  .long  0xbd402116                          // ldr           s22, [x8, #32]
  .long  0x4d40c952                          // ld1r          {v18.4s}, [x10]
  .long  0x4f941050                          // fmla          v16.4s, v2.4s, v20.s[0]
  .long  0x4f951051                          // fmla          v17.4s, v2.4s, v21.s[0]
  .long  0x4f961052                          // fmla          v18.4s, v2.4s, v22.s[0]
  .long  0x2d425502                          // ldp           s2, s21, [x8, #16]
  .long  0x2d415d14                          // ldp           s20, s23, [x8, #8]
  .long  0x4f821031                          // fmla          v17.4s, v1.4s, v2.s[0]
  .long  0xbd400122                          // ldr           s2, [x9]
  .long  0x4f971030                          // fmla          v16.4s, v1.4s, v23.s[0]
  .long  0x4f951032                          // fmla          v18.4s, v1.4s, v21.s[0]
  .long  0x4e20ce70                          // fmla          v16.4s, v19.4s, v0.4s
  .long  0x4f941012                          // fmla          v18.4s, v0.4s, v20.s[0]
  .long  0x4f821011                          // fmla          v17.4s, v0.4s, v2.s[0]
  .long  0x4eb01e00                          // mov           v0.16b, v16.16b
  .long  0x4eb11e21                          // mov           v1.16b, v17.16b
  .long  0x4eb21e42                          // mov           v2.16b, v18.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_matrix_4x5_aarch64
.globl _sk_matrix_4x5_aarch64
_sk_matrix_4x5_aarch64:
  .long  0xf9400029                          // ldr           x9, [x1]
  .long  0xaa0903e8                          // mov           x8, x9
  .long  0x9101012a                          // add           x10, x9, #0x40
  .long  0x4ddfc914                          // ld1r          {v20.4s}, [x8], #4
  .long  0x4d40c950                          // ld1r          {v16.4s}, [x10]
  .long  0x9101112a                          // add           x10, x9, #0x44
  .long  0x4d40c951                          // ld1r          {v17.4s}, [x10]
  .long  0x9101212a                          // add           x10, x9, #0x48
  .long  0x4d40c952                          // ld1r          {v18.4s}, [x10]
  .long  0x2d465533                          // ldp           s19, s21, [x9, #48]
  .long  0x2d475d36                          // ldp           s22, s23, [x9, #56]
  .long  0x9101312a                          // add           x10, x9, #0x4c
  .long  0xf9400423                          // ldr           x3, [x1, #8]
  .long  0x4f931070                          // fmla          v16.4s, v3.4s, v19.s[0]
  .long  0x4d40c953                          // ld1r          {v19.4s}, [x10]
  .long  0x4f951071                          // fmla          v17.4s, v3.4s, v21.s[0]
  .long  0x4f961072                          // fmla          v18.4s, v3.4s, v22.s[0]
  .long  0x2d445935                          // ldp           s21, s22, [x9, #32]
  .long  0x4f971073                          // fmla          v19.4s, v3.4s, v23.s[0]
  .long  0x2d455d23                          // ldp           s3, s23, [x9, #40]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x4f951050                          // fmla          v16.4s, v2.4s, v21.s[0]
  .long  0x4f961051                          // fmla          v17.4s, v2.4s, v22.s[0]
  .long  0x2d425935                          // ldp           s21, s22, [x9, #16]
  .long  0x4f971053                          // fmla          v19.4s, v2.4s, v23.s[0]
  .long  0x4f831052                          // fmla          v18.4s, v2.4s, v3.s[0]
  .long  0x2d410d22                          // ldp           s2, s3, [x9, #8]
  .long  0x4f951030                          // fmla          v16.4s, v1.4s, v21.s[0]
  .long  0x2d435d35                          // ldp           s21, s23, [x9, #24]
  .long  0x4f961031                          // fmla          v17.4s, v1.4s, v22.s[0]
  .long  0xbd400116                          // ldr           s22, [x8]
  .long  0x4e20ce90                          // fmla          v16.4s, v20.4s, v0.4s
  .long  0x4f951032                          // fmla          v18.4s, v1.4s, v21.s[0]
  .long  0x4f971033                          // fmla          v19.4s, v1.4s, v23.s[0]
  .long  0x4f821012                          // fmla          v18.4s, v0.4s, v2.s[0]
  .long  0x4f831013                          // fmla          v19.4s, v0.4s, v3.s[0]
  .long  0x4f961011                          // fmla          v17.4s, v0.4s, v22.s[0]
  .long  0x4eb01e00                          // mov           v0.16b, v16.16b
  .long  0x4eb11e21                          // mov           v1.16b, v17.16b
  .long  0x4eb21e42                          // mov           v2.16b, v18.16b
  .long  0x4eb31e63                          // mov           v3.16b, v19.16b
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_matrix_perspective_aarch64
.globl _sk_matrix_perspective_aarch64
_sk_matrix_perspective_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0xaa0803e9                          // mov           x9, x8
  .long  0x9100510a                          // add           x10, x8, #0x14
  .long  0x4ddfc930                          // ld1r          {v16.4s}, [x9], #4
  .long  0x4d40c951                          // ld1r          {v17.4s}, [x10]
  .long  0x9100810a                          // add           x10, x8, #0x20
  .long  0x4d40c952                          // ld1r          {v18.4s}, [x10]
  .long  0x2d41d113                          // ldp           s19, s20, [x8, #12]
  .long  0x2d435915                          // ldp           s21, s22, [x8, #24]
  .long  0x91002108                          // add           x8, x8, #0x8
  .long  0x4f941031                          // fmla          v17.4s, v1.4s, v20.s[0]
  .long  0x4d40c914                          // ld1r          {v20.4s}, [x8]
  .long  0x4f961032                          // fmla          v18.4s, v1.4s, v22.s[0]
  .long  0xbd400136                          // ldr           s22, [x9]
  .long  0x4f951012                          // fmla          v18.4s, v0.4s, v21.s[0]
  .long  0x4f931011                          // fmla          v17.4s, v0.4s, v19.s[0]
  .long  0x4f961034                          // fmla          v20.4s, v1.4s, v22.s[0]
  .long  0x4ea1da41                          // frecpe        v1.4s, v18.4s
  .long  0x4e21fe52                          // frecps        v18.4s, v18.4s, v1.4s
  .long  0x6e32dc32                          // fmul          v18.4s, v1.4s, v18.4s
  .long  0x4e20ce14                          // fmla          v20.4s, v16.4s, v0.4s
  .long  0x6e32de21                          // fmul          v1.4s, v17.4s, v18.4s
  .long  0x6e32de80                          // fmul          v0.4s, v20.4s, v18.4s
  .long  0xd61f0060                          // br            x3

HIDDEN _sk_linear_gradient_2stops_aarch64
.globl _sk_linear_gradient_2stops_aarch64
_sk_linear_gradient_2stops_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1], #16
  .long  0xad404503                          // ldp           q3, q17, [x8]
  .long  0x4e040470                          // dup           v16.4s, v3.s[0]
  .long  0x4e0c0461                          // dup           v1.4s, v3.s[1]
  .long  0x4e140462                          // dup           v2.4s, v3.s[2]
  .long  0x4e1c0463                          // dup           v3.4s, v3.s[3]
  .long  0x4f911010                          // fmla          v16.4s, v0.4s, v17.s[0]
  .long  0x4fb11001                          // fmla          v1.4s, v0.4s, v17.s[1]
  .long  0x4f911802                          // fmla          v2.4s, v0.4s, v17.s[2]
  .long  0x4fb11803                          // fmla          v3.4s, v0.4s, v17.s[3]
  .long  0x4eb01e00                          // mov           v0.16b, v16.16b
  .long  0xd61f0060                          // br            x3
#elif defined(__arm__)
.balign 4

HIDDEN _sk_start_pipeline_vfp4
.globl _sk_start_pipeline_vfp4
_sk_start_pipeline_vfp4:
  .long  0xe92d41f0                          // push          {r4, r5, r6, r7, r8, lr}
  .long  0xe1a04000                          // mov           r4, r0
  .long  0xe2840002                          // add           r0, r4, #2
  .long  0xe1a05003                          // mov           r5, r3
  .long  0xe1a08002                          // mov           r8, r2
  .long  0xe1a07001                          // mov           r7, r1
  .long  0xe1500005                          // cmp           r0, r5
  .long  0x8a000010                          // bhi           64 <sk_start_pipeline_vfp4+0x64>
  .long  0xe4976004                          // ldr           r6, [r7], #4
  .long  0xf2800010                          // vmov.i32      d0, #0
  .long  0xe1a00004                          // mov           r0, r4
  .long  0xf2801010                          // vmov.i32      d1, #0
  .long  0xe1a01007                          // mov           r1, r7
  .long  0xf2802010                          // vmov.i32      d2, #0
  .long  0xe1a02008                          // mov           r2, r8
  .long  0xf2803010                          // vmov.i32      d3, #0
  .long  0xf2804010                          // vmov.i32      d4, #0
  .long  0xf2805010                          // vmov.i32      d5, #0
  .long  0xf2806010                          // vmov.i32      d6, #0
  .long  0xf2807010                          // vmov.i32      d7, #0
  .long  0xe12fff36                          // blx           r6
  .long  0xe2840004                          // add           r0, r4, #4
  .long  0xe2844002                          // add           r4, r4, #2
  .long  0xe1500005                          // cmp           r0, r5
  .long  0x9affffef                          // bls           24 <sk_start_pipeline_vfp4+0x24>
  .long  0xe1a00004                          // mov           r0, r4
  .long  0xe8bd81f0                          // pop           {r4, r5, r6, r7, r8, pc}

HIDDEN _sk_just_return_vfp4
.globl _sk_just_return_vfp4
_sk_just_return_vfp4:
  .long  0xe12fff1e                          // bx            lr

HIDDEN _sk_seed_shader_vfp4
.globl _sk_seed_shader_vfp4
_sk_seed_shader_vfp4:
  .long  0xee800b90                          // vdup.32       d16, r0
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xf2c3161f                          // vmov.i32      d17, #1056964608
  .long  0xedd23b00                          // vldr          d19, [r2]
  .long  0xf4e32c9f                          // vld1.32       {d18[]}, [r3 :32]
  .long  0xf2872f10                          // vmov.f32      d2, #1
  .long  0xf3fb2622                          // vcvt.f32.s32  d18, d18
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2400da1                          // vadd.f32      d16, d16, d17
  .long  0xf2803010                          // vmov.i32      d3, #0
  .long  0xf2804010                          // vmov.i32      d4, #0
  .long  0xf2021da1                          // vadd.f32      d1, d18, d17
  .long  0xf2000da3                          // vadd.f32      d0, d16, d19
  .long  0xf2805010                          // vmov.i32      d5, #0
  .long  0xf2806010                          // vmov.i32      d6, #0
  .long  0xf2807010                          // vmov.i32      d7, #0
  .long  0xe12fff1c                          // bx            ip

HIDDEN _sk_constant_color_vfp4
.globl _sk_constant_color_vfp4
_sk_constant_color_vfp4:
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf4630a0f                          // vld1.8        {d16-d17}, [r3]
  .long  0xf3b40c20                          // vdup.32       d0, d16[0]
  .long  0xf3bc1c20                          // vdup.32       d1, d16[1]
  .long  0xf3b42c21                          // vdup.32       d2, d17[0]
  .long  0xf3bc3c21                          // vdup.32       d3, d17[1]
  .long  0xe12fff1c                          // bx            ip

HIDDEN _sk_clear_vfp4
.globl _sk_clear_vfp4
_sk_clear_vfp4:
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf2800010                          // vmov.i32      d0, #0
  .long  0xf2801010                          // vmov.i32      d1, #0
  .long  0xf2802010                          // vmov.i32      d2, #0
  .long  0xf2803010                          // vmov.i32      d3, #0
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_srcatop_vfp4
.globl _sk_srcatop_vfp4
_sk_srcatop_vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf2603d83                          // vsub.f32      d19, d16, d3
  .long  0xf3033d17                          // vmul.f32      d3, d3, d7
  .long  0xf3430d94                          // vmul.f32      d16, d19, d4
  .long  0xf3431d95                          // vmul.f32      d17, d19, d5
  .long  0xf3432d96                          // vmul.f32      d18, d19, d6
  .long  0xf2400c17                          // vfma.f32      d16, d0, d7
  .long  0xf2411c17                          // vfma.f32      d17, d1, d7
  .long  0xf2422c17                          // vfma.f32      d18, d2, d7
  .long  0xf2033c97                          // vfma.f32      d3, d19, d7
  .long  0xf22001b0                          // vorr          d0, d16, d16
  .long  0xf22111b1                          // vorr          d1, d17, d17
  .long  0xf22221b2                          // vorr          d2, d18, d18
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_dstatop_vfp4
.globl _sk_dstatop_vfp4
_sk_dstatop_vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf3431d15                          // vmul.f32      d17, d3, d5
  .long  0xf2604d87                          // vsub.f32      d20, d16, d7
  .long  0xf3430d14                          // vmul.f32      d16, d3, d4
  .long  0xf3432d16                          // vmul.f32      d18, d3, d6
  .long  0xf3433d17                          // vmul.f32      d19, d3, d7
  .long  0xf2440c90                          // vfma.f32      d16, d20, d0
  .long  0xf2441c91                          // vfma.f32      d17, d20, d1
  .long  0xf2442c92                          // vfma.f32      d18, d20, d2
  .long  0xf2443c93                          // vfma.f32      d19, d20, d3
  .long  0xf22001b0                          // vorr          d0, d16, d16
  .long  0xf22111b1                          // vorr          d1, d17, d17
  .long  0xf22221b2                          // vorr          d2, d18, d18
  .long  0xf22331b3                          // vorr          d3, d19, d19
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_srcin_vfp4
.globl _sk_srcin_vfp4
_sk_srcin_vfp4:
  .long  0xf3000d17                          // vmul.f32      d0, d0, d7
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf3011d17                          // vmul.f32      d1, d1, d7
  .long  0xf3022d17                          // vmul.f32      d2, d2, d7
  .long  0xf3033d17                          // vmul.f32      d3, d3, d7
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_dstin_vfp4
.globl _sk_dstin_vfp4
_sk_dstin_vfp4:
  .long  0xf3030d14                          // vmul.f32      d0, d3, d4
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf3031d15                          // vmul.f32      d1, d3, d5
  .long  0xf3032d16                          // vmul.f32      d2, d3, d6
  .long  0xf3033d17                          // vmul.f32      d3, d3, d7
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_srcout_vfp4
.globl _sk_srcout_vfp4
_sk_srcout_vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf2600d87                          // vsub.f32      d16, d16, d7
  .long  0xf3000d90                          // vmul.f32      d0, d16, d0
  .long  0xf3001d91                          // vmul.f32      d1, d16, d1
  .long  0xf3002d92                          // vmul.f32      d2, d16, d2
  .long  0xf3003d93                          // vmul.f32      d3, d16, d3
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_dstout_vfp4
.globl _sk_dstout_vfp4
_sk_dstout_vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf2600d83                          // vsub.f32      d16, d16, d3
  .long  0xf3000d94                          // vmul.f32      d0, d16, d4
  .long  0xf3001d95                          // vmul.f32      d1, d16, d5
  .long  0xf3002d96                          // vmul.f32      d2, d16, d6
  .long  0xf3003d97                          // vmul.f32      d3, d16, d7
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_srcover_vfp4
.globl _sk_srcover_vfp4
_sk_srcover_vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf2600d83                          // vsub.f32      d16, d16, d3
  .long  0xf2040c30                          // vfma.f32      d0, d4, d16
  .long  0xf2051c30                          // vfma.f32      d1, d5, d16
  .long  0xf2062c30                          // vfma.f32      d2, d6, d16
  .long  0xf2073c30                          // vfma.f32      d3, d7, d16
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_dstover_vfp4
.globl _sk_dstover_vfp4
_sk_dstover_vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf2651115                          // vorr          d17, d5, d5
  .long  0xf2604d87                          // vsub.f32      d20, d16, d7
  .long  0xf2640114                          // vorr          d16, d4, d4
  .long  0xf2662116                          // vorr          d18, d6, d6
  .long  0xf2673117                          // vorr          d19, d7, d7
  .long  0xf2400c34                          // vfma.f32      d16, d0, d20
  .long  0xf2411c34                          // vfma.f32      d17, d1, d20
  .long  0xf2422c34                          // vfma.f32      d18, d2, d20
  .long  0xf2433c34                          // vfma.f32      d19, d3, d20
  .long  0xf22001b0                          // vorr          d0, d16, d16
  .long  0xf22111b1                          // vorr          d1, d17, d17
  .long  0xf22221b2                          // vorr          d2, d18, d18
  .long  0xf22331b3                          // vorr          d3, d19, d19
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_modulate_vfp4
.globl _sk_modulate_vfp4
_sk_modulate_vfp4:
  .long  0xf3000d14                          // vmul.f32      d0, d0, d4
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf3011d15                          // vmul.f32      d1, d1, d5
  .long  0xf3022d16                          // vmul.f32      d2, d2, d6
  .long  0xf3033d17                          // vmul.f32      d3, d3, d7
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_multiply_vfp4
.globl _sk_multiply_vfp4
_sk_multiply_vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf2601d87                          // vsub.f32      d17, d16, d7
  .long  0xf2600d83                          // vsub.f32      d16, d16, d3
  .long  0xf3412d90                          // vmul.f32      d18, d17, d0
  .long  0xf3403d94                          // vmul.f32      d19, d16, d4
  .long  0xf3414d91                          // vmul.f32      d20, d17, d1
  .long  0xf3405d95                          // vmul.f32      d21, d16, d5
  .long  0xf3416d92                          // vmul.f32      d22, d17, d2
  .long  0xf3418d93                          // vmul.f32      d24, d17, d3
  .long  0xf3407d96                          // vmul.f32      d23, d16, d6
  .long  0xf3409d97                          // vmul.f32      d25, d16, d7
  .long  0xf2430da2                          // vadd.f32      d16, d19, d18
  .long  0xf2451da4                          // vadd.f32      d17, d21, d20
  .long  0xf2472da6                          // vadd.f32      d18, d23, d22
  .long  0xf2493da8                          // vadd.f32      d19, d25, d24
  .long  0xf2400c14                          // vfma.f32      d16, d0, d4
  .long  0xf2411c15                          // vfma.f32      d17, d1, d5
  .long  0xf2422c16                          // vfma.f32      d18, d2, d6
  .long  0xf2433c17                          // vfma.f32      d19, d3, d7
  .long  0xf22001b0                          // vorr          d0, d16, d16
  .long  0xf22111b1                          // vorr          d1, d17, d17
  .long  0xf22221b2                          // vorr          d2, d18, d18
  .long  0xf22331b3                          // vorr          d3, d19, d19
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_plus__vfp4
.globl _sk_plus__vfp4
_sk_plus__vfp4:
  .long  0xf2000d04                          // vadd.f32      d0, d0, d4
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf2011d05                          // vadd.f32      d1, d1, d5
  .long  0xf2022d06                          // vadd.f32      d2, d2, d6
  .long  0xf2033d07                          // vadd.f32      d3, d3, d7
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_screen_vfp4
.globl _sk_screen_vfp4
_sk_screen_vfp4:
  .long  0xf2400d04                          // vadd.f32      d16, d0, d4
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf2411d05                          // vadd.f32      d17, d1, d5
  .long  0xf2422d06                          // vadd.f32      d18, d2, d6
  .long  0xf2433d07                          // vadd.f32      d19, d3, d7
  .long  0xf2600c14                          // vfms.f32      d16, d0, d4
  .long  0xf2611c15                          // vfms.f32      d17, d1, d5
  .long  0xf2622c16                          // vfms.f32      d18, d2, d6
  .long  0xf2633c17                          // vfms.f32      d19, d3, d7
  .long  0xf22001b0                          // vorr          d0, d16, d16
  .long  0xf22111b1                          // vorr          d1, d17, d17
  .long  0xf22221b2                          // vorr          d2, d18, d18
  .long  0xf22331b3                          // vorr          d3, d19, d19
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_xor__vfp4
.globl _sk_xor__vfp4
_sk_xor__vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf2603d83                          // vsub.f32      d19, d16, d3
  .long  0xf2604d87                          // vsub.f32      d20, d16, d7
  .long  0xf3430d94                          // vmul.f32      d16, d19, d4
  .long  0xf3431d95                          // vmul.f32      d17, d19, d5
  .long  0xf3432d96                          // vmul.f32      d18, d19, d6
  .long  0xf3433d97                          // vmul.f32      d19, d19, d7
  .long  0xf2440c90                          // vfma.f32      d16, d20, d0
  .long  0xf2441c91                          // vfma.f32      d17, d20, d1
  .long  0xf2442c92                          // vfma.f32      d18, d20, d2
  .long  0xf2443c93                          // vfma.f32      d19, d20, d3
  .long  0xf22001b0                          // vorr          d0, d16, d16
  .long  0xf22111b1                          // vorr          d1, d17, d17
  .long  0xf22221b2                          // vorr          d2, d18, d18
  .long  0xf22331b3                          // vorr          d3, d19, d19
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_darken_vfp4
.globl _sk_darken_vfp4
_sk_darken_vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf3431d14                          // vmul.f32      d17, d3, d4
  .long  0xf3402d17                          // vmul.f32      d18, d0, d7
  .long  0xf3433d15                          // vmul.f32      d19, d3, d5
  .long  0xf3414d17                          // vmul.f32      d20, d1, d7
  .long  0xf3435d16                          // vmul.f32      d21, d3, d6
  .long  0xf2600d83                          // vsub.f32      d16, d16, d3
  .long  0xf3426d17                          // vmul.f32      d22, d2, d7
  .long  0xf2421fa1                          // vmax.f32      d17, d18, d17
  .long  0xf2407d04                          // vadd.f32      d23, d0, d4
  .long  0xf2443fa3                          // vmax.f32      d19, d20, d19
  .long  0xf2412d05                          // vadd.f32      d18, d1, d5
  .long  0xf2424d06                          // vadd.f32      d20, d2, d6
  .long  0xf2465fa5                          // vmax.f32      d21, d22, d21
  .long  0xf2073c30                          // vfma.f32      d3, d7, d16
  .long  0xf2270da1                          // vsub.f32      d0, d23, d17
  .long  0xf2221da3                          // vsub.f32      d1, d18, d19
  .long  0xf2242da5                          // vsub.f32      d2, d20, d21
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_lighten_vfp4
.globl _sk_lighten_vfp4
_sk_lighten_vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf3431d14                          // vmul.f32      d17, d3, d4
  .long  0xf3402d17                          // vmul.f32      d18, d0, d7
  .long  0xf3433d15                          // vmul.f32      d19, d3, d5
  .long  0xf3414d17                          // vmul.f32      d20, d1, d7
  .long  0xf3435d16                          // vmul.f32      d21, d3, d6
  .long  0xf2600d83                          // vsub.f32      d16, d16, d3
  .long  0xf3426d17                          // vmul.f32      d22, d2, d7
  .long  0xf2621fa1                          // vmin.f32      d17, d18, d17
  .long  0xf2407d04                          // vadd.f32      d23, d0, d4
  .long  0xf2643fa3                          // vmin.f32      d19, d20, d19
  .long  0xf2412d05                          // vadd.f32      d18, d1, d5
  .long  0xf2424d06                          // vadd.f32      d20, d2, d6
  .long  0xf2665fa5                          // vmin.f32      d21, d22, d21
  .long  0xf2073c30                          // vfma.f32      d3, d7, d16
  .long  0xf2270da1                          // vsub.f32      d0, d23, d17
  .long  0xf2221da3                          // vsub.f32      d1, d18, d19
  .long  0xf2242da5                          // vsub.f32      d2, d20, d21
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_difference_vfp4
.globl _sk_difference_vfp4
_sk_difference_vfp4:
  .long  0xf3430d14                          // vmul.f32      d16, d3, d4
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf3401d17                          // vmul.f32      d17, d0, d7
  .long  0xf3432d15                          // vmul.f32      d18, d3, d5
  .long  0xf3413d17                          // vmul.f32      d19, d1, d7
  .long  0xf3434d16                          // vmul.f32      d20, d3, d6
  .long  0xf3425d17                          // vmul.f32      d21, d2, d7
  .long  0xf2c76f10                          // vmov.f32      d22, #1
  .long  0xf2610fa0                          // vmin.f32      d16, d17, d16
  .long  0xf2631fa2                          // vmin.f32      d17, d19, d18
  .long  0xf2662d83                          // vsub.f32      d18, d22, d3
  .long  0xf2653fa4                          // vmin.f32      d19, d21, d20
  .long  0xf2404d04                          // vadd.f32      d20, d0, d4
  .long  0xf2400da0                          // vadd.f32      d16, d16, d16
  .long  0xf2073c32                          // vfma.f32      d3, d7, d18
  .long  0xf2415d05                          // vadd.f32      d21, d1, d5
  .long  0xf2411da1                          // vadd.f32      d17, d17, d17
  .long  0xf2426d06                          // vadd.f32      d22, d2, d6
  .long  0xf2432da3                          // vadd.f32      d18, d19, d19
  .long  0xf2240da0                          // vsub.f32      d0, d20, d16
  .long  0xf2251da1                          // vsub.f32      d1, d21, d17
  .long  0xf2262da2                          // vsub.f32      d2, d22, d18
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_exclusion_vfp4
.globl _sk_exclusion_vfp4
_sk_exclusion_vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf3401d14                          // vmul.f32      d17, d0, d4
  .long  0xf3412d15                          // vmul.f32      d18, d1, d5
  .long  0xf3423d16                          // vmul.f32      d19, d2, d6
  .long  0xf2600d83                          // vsub.f32      d16, d16, d3
  .long  0xf2404d04                          // vadd.f32      d20, d0, d4
  .long  0xf2411da1                          // vadd.f32      d17, d17, d17
  .long  0xf2415d05                          // vadd.f32      d21, d1, d5
  .long  0xf2422da2                          // vadd.f32      d18, d18, d18
  .long  0xf2426d06                          // vadd.f32      d22, d2, d6
  .long  0xf2433da3                          // vadd.f32      d19, d19, d19
  .long  0xf2073c30                          // vfma.f32      d3, d7, d16
  .long  0xf2240da1                          // vsub.f32      d0, d20, d17
  .long  0xf2251da2                          // vsub.f32      d1, d21, d18
  .long  0xf2262da3                          // vsub.f32      d2, d22, d19
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_colorburn_vfp4
.globl _sk_colorburn_vfp4
_sk_colorburn_vfp4:
  .long  0xed2d8b08                          // vpush         {d8-d11}
  .long  0xf2670d04                          // vsub.f32      d16, d7, d4
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf2671d06                          // vsub.f32      d17, d7, d6
  .long  0xf2672d05                          // vsub.f32      d18, d7, d5
  .long  0xf3008d93                          // vmul.f32      d8, d16, d3
  .long  0xf3019d93                          // vmul.f32      d9, d17, d3
  .long  0xf302ad93                          // vmul.f32      d10, d18, d3
  .long  0xf2c71f10                          // vmov.f32      d17, #1
  .long  0xeec8baa0                          // vdiv.f32      s23, s17, s1
  .long  0xee88ba00                          // vdiv.f32      s22, s16, s0
  .long  0xeec98aa2                          // vdiv.f32      s17, s19, s5
  .long  0xee898a02                          // vdiv.f32      s16, s18, s4
  .long  0xeeca9aa1                          // vdiv.f32      s19, s21, s3
  .long  0xee8a9a01                          // vdiv.f32      s18, s20, s2
  .long  0xf2672f08                          // vmin.f32      d18, d7, d8
  .long  0xf2673f09                          // vmin.f32      d19, d7, d9
  .long  0xf2670f0b                          // vmin.f32      d16, d7, d11
  .long  0xf2614d87                          // vsub.f32      d20, d17, d7
  .long  0xf2672d22                          // vsub.f32      d18, d7, d18
  .long  0xf2673d23                          // vsub.f32      d19, d7, d19
  .long  0xf2611d83                          // vsub.f32      d17, d17, d3
  .long  0xf2670d20                          // vsub.f32      d16, d7, d16
  .long  0xf3445d90                          // vmul.f32      d21, d20, d0
  .long  0xf3446d92                          // vmul.f32      d22, d20, d2
  .long  0xf3422d93                          // vmul.f32      d18, d18, d3
  .long  0xf3444d91                          // vmul.f32      d20, d20, d1
  .long  0xf3433d93                          // vmul.f32      d19, d19, d3
  .long  0xf3400d93                          // vmul.f32      d16, d16, d3
  .long  0xf3417d95                          // vmul.f32      d23, d17, d5
  .long  0xf3418d94                          // vmul.f32      d24, d17, d4
  .long  0xf3419d96                          // vmul.f32      d25, d17, d6
  .long  0xf2443da3                          // vadd.f32      d19, d20, d19
  .long  0xf2462da2                          // vadd.f32      d18, d22, d18
  .long  0xf245ada0                          // vadd.f32      d26, d21, d16
  .long  0xf247bd81                          // vadd.f32      d27, d23, d1
  .long  0xf248cd80                          // vadd.f32      d28, d24, d0
  .long  0xf249dd82                          // vadd.f32      d29, d25, d2
  .long  0xf2073c31                          // vfma.f32      d3, d7, d17
  .long  0xf2499da2                          // vadd.f32      d25, d25, d18
  .long  0xf2473da3                          // vadd.f32      d19, d23, d19
  .long  0xf3f97501                          // vceq.f32      d23, d1, #0
  .long  0xf2455d84                          // vadd.f32      d21, d21, d4
  .long  0xf2444d85                          // vadd.f32      d20, d20, d5
  .long  0xf2440e07                          // vceq.f32      d16, d4, d7
  .long  0xf2466d86                          // vadd.f32      d22, d22, d6
  .long  0xf2451e07                          // vceq.f32      d17, d5, d7
  .long  0xf2462e07                          // vceq.f32      d18, d6, d7
  .long  0xf35b71b3                          // vbsl          d23, d27, d19
  .long  0xf3f93500                          // vceq.f32      d19, d0, #0
  .long  0xf2488daa                          // vadd.f32      d24, d24, d26
  .long  0xf35c31b8                          // vbsl          d19, d28, d24
  .long  0xf3f98502                          // vceq.f32      d24, d2, #0
  .long  0xf35d81b9                          // vbsl          d24, d29, d25
  .long  0xf35501b3                          // vbsl          d16, d21, d19
  .long  0xf35411b7                          // vbsl          d17, d20, d23
  .long  0xf35621b8                          // vbsl          d18, d22, d24
  .long  0xf22001b0                          // vorr          d0, d16, d16
  .long  0xf22111b1                          // vorr          d1, d17, d17
  .long  0xf22221b2                          // vorr          d2, d18, d18
  .long  0xecbd8b08                          // vpop          {d8-d11}
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_colordodge_vfp4
.globl _sk_colordodge_vfp4
_sk_colordodge_vfp4:
  .long  0xed2d8b0e                          // vpush         {d8-d14}
  .long  0xf2238d02                          // vsub.f32      d8, d3, d2
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf3039d16                          // vmul.f32      d9, d3, d6
  .long  0xf223ad01                          // vsub.f32      d10, d3, d1
  .long  0xf303bd15                          // vmul.f32      d11, d3, d5
  .long  0xf223cd00                          // vsub.f32      d12, d3, d0
  .long  0xf303dd14                          // vmul.f32      d13, d3, d4
  .long  0xeec9eaa8                          // vdiv.f32      s29, s19, s17
  .long  0xee89ea08                          // vdiv.f32      s28, s18, s16
  .long  0xeecb8aaa                          // vdiv.f32      s17, s23, s21
  .long  0xeecd9aac                          // vdiv.f32      s19, s27, s25
  .long  0xee8b8a0a                          // vdiv.f32      s16, s22, s20
  .long  0xee8d9a0c                          // vdiv.f32      s18, s26, s24
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xf2672f0e                          // vmin.f32      d18, d7, d14
  .long  0xf2601d87                          // vsub.f32      d17, d16, d7
  .long  0xf2673f08                          // vmin.f32      d19, d7, d8
  .long  0xf2674f09                          // vmin.f32      d20, d7, d9
  .long  0xf2600d83                          // vsub.f32      d16, d16, d3
  .long  0xf3415d92                          // vmul.f32      d21, d17, d2
  .long  0xf3422d93                          // vmul.f32      d18, d18, d3
  .long  0xf3416d91                          // vmul.f32      d22, d17, d1
  .long  0xf3433d93                          // vmul.f32      d19, d19, d3
  .long  0xf3411d90                          // vmul.f32      d17, d17, d0
  .long  0xf3444d93                          // vmul.f32      d20, d20, d3
  .long  0xf3407d95                          // vmul.f32      d23, d16, d5
  .long  0xf3408d94                          // vmul.f32      d24, d16, d4
  .long  0xf3409d96                          // vmul.f32      d25, d16, d6
  .long  0xf2452da2                          // vadd.f32      d18, d21, d18
  .long  0xf2463da3                          // vadd.f32      d19, d22, d19
  .long  0xf2414da4                          // vadd.f32      d20, d17, d20
  .long  0xf241ae03                          // vceq.f32      d26, d1, d3
  .long  0xf247bd81                          // vadd.f32      d27, d23, d1
  .long  0xf3b91505                          // vceq.f32      d1, d5, #0
  .long  0xf240ce03                          // vceq.f32      d28, d0, d3
  .long  0xf248dd80                          // vadd.f32      d29, d24, d0
  .long  0xf3b90504                          // vceq.f32      d0, d4, #0
  .long  0xf242ee03                          // vceq.f32      d30, d2, d3
  .long  0xf249fd82                          // vadd.f32      d31, d25, d2
  .long  0xf3b92506                          // vceq.f32      d2, d6, #0
  .long  0xf2073c30                          // vfma.f32      d3, d7, d16
  .long  0xf2410d84                          // vadd.f32      d16, d17, d4
  .long  0xf2491da2                          // vadd.f32      d17, d25, d18
  .long  0xf2462d85                          // vadd.f32      d18, d22, d5
  .long  0xf2455d86                          // vadd.f32      d21, d21, d6
  .long  0xf2473da3                          // vadd.f32      d19, d23, d19
  .long  0xf2484da4                          // vadd.f32      d20, d24, d20
  .long  0xf35fe1b1                          // vbsl          d30, d31, d17
  .long  0xf35ba1b3                          // vbsl          d26, d27, d19
  .long  0xf35dc1b4                          // vbsl          d28, d29, d20
  .long  0xf31001bc                          // vbsl          d0, d16, d28
  .long  0xf31211ba                          // vbsl          d1, d18, d26
  .long  0xf31521be                          // vbsl          d2, d21, d30
  .long  0xecbd8b0e                          // vpop          {d8-d14}
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_hardlight_vfp4
.globl _sk_hardlight_vfp4
_sk_hardlight_vfp4:
  .long  0xf2c71f10                          // vmov.f32      d17, #1
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf2670d04                          // vsub.f32      d16, d7, d4
  .long  0xf2617d87                          // vsub.f32      d23, d17, d7
  .long  0xf2611d83                          // vsub.f32      d17, d17, d3
  .long  0xf2672d05                          // vsub.f32      d18, d7, d5
  .long  0xf2674d06                          // vsub.f32      d20, d7, d6
  .long  0xf2633d00                          // vsub.f32      d19, d3, d0
  .long  0xf2635d01                          // vsub.f32      d21, d3, d1
  .long  0xf2636d02                          // vsub.f32      d22, d3, d2
  .long  0xf347bd90                          // vmul.f32      d27, d23, d0
  .long  0xf341cd94                          // vmul.f32      d28, d17, d4
  .long  0xf3430db0                          // vmul.f32      d16, d19, d16
  .long  0xf3463db4                          // vmul.f32      d19, d22, d20
  .long  0xf3452db2                          // vmul.f32      d18, d21, d18
  .long  0xf2404d00                          // vadd.f32      d20, d0, d0
  .long  0xf3405d14                          // vmul.f32      d21, d0, d4
  .long  0xf2416d01                          // vadd.f32      d22, d1, d1
  .long  0xf3418d15                          // vmul.f32      d24, d1, d5
  .long  0xf2429d02                          // vadd.f32      d25, d2, d2
  .long  0xf342ad16                          // vmul.f32      d26, d2, d6
  .long  0xf347dd91                          // vmul.f32      d29, d23, d1
  .long  0xf341fd95                          // vmul.f32      d31, d17, d5
  .long  0xf24cbdab                          // vadd.f32      d27, d28, d27
  .long  0xf3477d92                          // vmul.f32      d23, d23, d2
  .long  0xf341cd96                          // vmul.f32      d28, d17, d6
  .long  0xf2400da0                          // vadd.f32      d16, d16, d16
  .long  0xf343ed17                          // vmul.f32      d30, d3, d7
  .long  0xf2422da2                          // vadd.f32      d18, d18, d18
  .long  0xf2433da3                          // vadd.f32      d19, d19, d19
  .long  0xf3434e24                          // vcge.f32      d20, d3, d20
  .long  0xf2455da5                          // vadd.f32      d21, d21, d21
  .long  0xf3436e26                          // vcge.f32      d22, d3, d22
  .long  0xf3439e29                          // vcge.f32      d25, d3, d25
  .long  0xf2488da8                          // vadd.f32      d24, d24, d24
  .long  0xf24aadaa                          // vadd.f32      d26, d26, d26
  .long  0xf2073c31                          // vfma.f32      d3, d7, d17
  .long  0xf24fddad                          // vadd.f32      d29, d31, d29
  .long  0xf24c1da7                          // vadd.f32      d17, d28, d23
  .long  0xf26e0da0                          // vsub.f32      d16, d30, d16
  .long  0xf26e2da2                          // vsub.f32      d18, d30, d18
  .long  0xf26e3da3                          // vsub.f32      d19, d30, d19
  .long  0xf35541b0                          // vbsl          d20, d21, d16
  .long  0xf35861b2                          // vbsl          d22, d24, d18
  .long  0xf35a91b3                          // vbsl          d25, d26, d19
  .long  0xf20b0da4                          // vadd.f32      d0, d27, d20
  .long  0xf20d1da6                          // vadd.f32      d1, d29, d22
  .long  0xf2012da9                          // vadd.f32      d2, d17, d25
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_overlay_vfp4
.globl _sk_overlay_vfp4
_sk_overlay_vfp4:
  .long  0xf2c71f10                          // vmov.f32      d17, #1
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf2670d04                          // vsub.f32      d16, d7, d4
  .long  0xf2617d87                          // vsub.f32      d23, d17, d7
  .long  0xf2611d83                          // vsub.f32      d17, d17, d3
  .long  0xf2672d05                          // vsub.f32      d18, d7, d5
  .long  0xf2674d06                          // vsub.f32      d20, d7, d6
  .long  0xf2633d00                          // vsub.f32      d19, d3, d0
  .long  0xf2635d01                          // vsub.f32      d21, d3, d1
  .long  0xf2636d02                          // vsub.f32      d22, d3, d2
  .long  0xf347bd90                          // vmul.f32      d27, d23, d0
  .long  0xf341cd94                          // vmul.f32      d28, d17, d4
  .long  0xf3430db0                          // vmul.f32      d16, d19, d16
  .long  0xf3463db4                          // vmul.f32      d19, d22, d20
  .long  0xf3452db2                          // vmul.f32      d18, d21, d18
  .long  0xf2444d04                          // vadd.f32      d20, d4, d4
  .long  0xf3405d14                          // vmul.f32      d21, d0, d4
  .long  0xf2456d05                          // vadd.f32      d22, d5, d5
  .long  0xf3418d15                          // vmul.f32      d24, d1, d5
  .long  0xf2469d06                          // vadd.f32      d25, d6, d6
  .long  0xf342ad16                          // vmul.f32      d26, d2, d6
  .long  0xf347dd91                          // vmul.f32      d29, d23, d1
  .long  0xf341fd95                          // vmul.f32      d31, d17, d5
  .long  0xf24cbdab                          // vadd.f32      d27, d28, d27
  .long  0xf3477d92                          // vmul.f32      d23, d23, d2
  .long  0xf341cd96                          // vmul.f32      d28, d17, d6
  .long  0xf343ed17                          // vmul.f32      d30, d3, d7
  .long  0xf2400da0                          // vadd.f32      d16, d16, d16
  .long  0xf2422da2                          // vadd.f32      d18, d18, d18
  .long  0xf2433da3                          // vadd.f32      d19, d19, d19
  .long  0xf3474e24                          // vcge.f32      d20, d7, d20
  .long  0xf2455da5                          // vadd.f32      d21, d21, d21
  .long  0xf3476e26                          // vcge.f32      d22, d7, d22
  .long  0xf2488da8                          // vadd.f32      d24, d24, d24
  .long  0xf3479e29                          // vcge.f32      d25, d7, d25
  .long  0xf24aadaa                          // vadd.f32      d26, d26, d26
  .long  0xf2073c31                          // vfma.f32      d3, d7, d17
  .long  0xf24fddad                          // vadd.f32      d29, d31, d29
  .long  0xf24c1da7                          // vadd.f32      d17, d28, d23
  .long  0xf26e0da0                          // vsub.f32      d16, d30, d16
  .long  0xf26e2da2                          // vsub.f32      d18, d30, d18
  .long  0xf26e3da3                          // vsub.f32      d19, d30, d19
  .long  0xf35541b0                          // vbsl          d20, d21, d16
  .long  0xf35861b2                          // vbsl          d22, d24, d18
  .long  0xf35a91b3                          // vbsl          d25, d26, d19
  .long  0xf20b0da4                          // vadd.f32      d0, d27, d20
  .long  0xf20d1da6                          // vadd.f32      d1, d29, d22
  .long  0xf2012da9                          // vadd.f32      d2, d17, d25
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_softlight_vfp4
.globl _sk_softlight_vfp4
_sk_softlight_vfp4:
  .long  0xed2d8b06                          // vpush         {d8-d10}
  .long  0xeec58aa7                          // vdiv.f32      s17, s11, s15
  .long  0xf3f90407                          // vcgt.f32      d16, d7, #0
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xeec49aa7                          // vdiv.f32      s19, s9, s15
  .long  0xeec6aaa7                          // vdiv.f32      s21, s13, s15
  .long  0xee858a07                          // vdiv.f32      s16, s10, s14
  .long  0xee849a07                          // vdiv.f32      s18, s8, s14
  .long  0xee86aa07                          // vdiv.f32      s20, s12, s14
  .long  0xf26021b0                          // vorr          d18, d16, d16
  .long  0xf2c01010                          // vmov.i32      d17, #0
  .long  0xf3582131                          // vbsl          d18, d8, d17
  .long  0xf26031b0                          // vorr          d19, d16, d16
  .long  0xf3fb45a2                          // vrsqrte.f32   d20, d18
  .long  0xf3593131                          // vbsl          d19, d9, d17
  .long  0xf35a0131                          // vbsl          d16, d10, d17
  .long  0xf3fb15a3                          // vrsqrte.f32   d17, d19
  .long  0xf3fb55a0                          // vrsqrte.f32   d21, d16
  .long  0xf3446db4                          // vmul.f32      d22, d20, d20
  .long  0xf243ada3                          // vadd.f32      d26, d19, d19
  .long  0xf240bda0                          // vadd.f32      d27, d16, d16
  .long  0xf3417db1                          // vmul.f32      d23, d17, d17
  .long  0xf3458db5                          // vmul.f32      d24, d21, d21
  .long  0xf2626fb6                          // vrsqrts.f32   d22, d18, d22
  .long  0xf2429da2                          // vadd.f32      d25, d18, d18
  .long  0xf2637fb7                          // vrsqrts.f32   d23, d19, d23
  .long  0xf2608fb8                          // vrsqrts.f32   d24, d16, d24
  .long  0xf2818f1c                          // vmov.f32      d8, #7
  .long  0xf2499da9                          // vadd.f32      d25, d25, d25
  .long  0xf3444db6                          // vmul.f32      d20, d20, d22
  .long  0xf24a6daa                          // vadd.f32      d22, d26, d26
  .long  0xf24badab                          // vadd.f32      d26, d27, d27
  .long  0xf3411db7                          // vmul.f32      d17, d17, d23
  .long  0xf3455db8                          // vmul.f32      d21, d21, d24
  .long  0xf3fb7524                          // vrecpe.f32    d23, d20
  .long  0xf3498db9                          // vmul.f32      d24, d25, d25
  .long  0xf3fbd521                          // vrecpe.f32    d29, d17
  .long  0xf34aedba                          // vmul.f32      d30, d26, d26
  .long  0xf3fbf525                          // vrecpe.f32    d31, d21
  .long  0xf2444fb7                          // vrecps.f32    d20, d20, d23
  .long  0xf346cdb6                          // vmul.f32      d28, d22, d22
  .long  0xf2411fbd                          // vrecps.f32    d17, d17, d29
  .long  0xf3c7bf10                          // vmov.f32      d27, #-1
  .long  0xf2455fbf                          // vrecps.f32    d21, d21, d31
  .long  0xf24aadae                          // vadd.f32      d26, d26, d30
  .long  0xf2498da8                          // vadd.f32      d24, d25, d24
  .long  0xf2429dab                          // vadd.f32      d25, d18, d27
  .long  0xf2466dac                          // vadd.f32      d22, d22, d28
  .long  0xf243cdab                          // vadd.f32      d28, d19, d27
  .long  0xf240bdab                          // vadd.f32      d27, d16, d27
  .long  0xf3474db4                          // vmul.f32      d20, d23, d20
  .long  0xf2c7ef10                          // vmov.f32      d30, #1
  .long  0xf34d1db1                          // vmul.f32      d17, d29, d17
  .long  0xf34badba                          // vmul.f32      d26, d27, d26
  .long  0xf242bd02                          // vadd.f32      d27, d2, d2
  .long  0xf26edda0                          // vsub.f32      d29, d30, d16
  .long  0xf3498db8                          // vmul.f32      d24, d25, d24
  .long  0xf3429d98                          // vmul.f32      d25, d18, d8
  .long  0xf34f5db5                          // vmul.f32      d21, d31, d21
  .long  0xf26efda2                          // vsub.f32      d31, d30, d18
  .long  0xf2642da2                          // vsub.f32      d18, d20, d18
  .long  0xf26b4d83                          // vsub.f32      d20, d27, d3
  .long  0xf2498da8                          // vadd.f32      d24, d25, d24
  .long  0xf34c6db6                          // vmul.f32      d22, d28, d22
  .long  0xf3437d98                          // vmul.f32      d23, d19, d8
  .long  0xf3449dbd                          // vmul.f32      d25, d20, d29
  .long  0xf245dd05                          // vadd.f32      d29, d5, d5
  .long  0xf340cd98                          // vmul.f32      d28, d16, d8
  .long  0xf2476da6                          // vadd.f32      d22, d23, d22
  .long  0xf2611da3                          // vsub.f32      d17, d17, d19
  .long  0xf24dddad                          // vadd.f32      d29, d29, d29
  .long  0xf24c7daa                          // vadd.f32      d23, d28, d26
  .long  0xf2650da0                          // vsub.f32      d16, d21, d16
  .long  0xf26e3da3                          // vsub.f32      d19, d30, d19
  .long  0xf347de2d                          // vcge.f32      d29, d7, d29
  .long  0xf241ad01                          // vadd.f32      d26, d1, d1
  .long  0xf3444d97                          // vmul.f32      d20, d20, d7
  .long  0xf358d1b2                          // vbsl          d29, d24, d18
  .long  0xf2448d04                          // vadd.f32      d24, d4, d4
  .long  0xf2462d06                          // vadd.f32      d18, d6, d6
  .long  0xf26a5d83                          // vsub.f32      d21, d26, d3
  .long  0xf2488da8                          // vadd.f32      d24, d24, d24
  .long  0xf2422da2                          // vadd.f32      d18, d18, d18
  .long  0xf345cdbf                          // vmul.f32      d28, d21, d31
  .long  0xf3455d97                          // vmul.f32      d21, d21, d7
  .long  0xf3478e28                          // vcge.f32      d24, d7, d24
  .long  0xf3472e22                          // vcge.f32      d18, d7, d18
  .long  0xf343fd14                          // vmul.f32      d31, d3, d4
  .long  0xf3455dbd                          // vmul.f32      d21, d21, d29
  .long  0xf35681b1                          // vbsl          d24, d22, d17
  .long  0xf2401d00                          // vadd.f32      d17, d0, d0
  .long  0xf35721b0                          // vbsl          d18, d23, d16
  .long  0xf24c0d83                          // vadd.f32      d16, d28, d3
  .long  0xf2496d83                          // vadd.f32      d22, d25, d3
  .long  0xf2617d83                          // vsub.f32      d23, d17, d3
  .long  0xf3442db2                          // vmul.f32      d18, d20, d18
  .long  0xf3434e2a                          // vcge.f32      d20, d3, d26
  .long  0xf343ae2b                          // vcge.f32      d26, d3, d27
  .long  0xf3473db3                          // vmul.f32      d19, d23, d19
  .long  0xf3477d97                          // vmul.f32      d23, d23, d7
  .long  0xf3431e21                          // vcge.f32      d17, d3, d17
  .long  0xf3400d95                          // vmul.f32      d16, d16, d5
  .long  0xf2433d83                          // vadd.f32      d19, d19, d3
  .long  0xf3477db8                          // vmul.f32      d23, d23, d24
  .long  0xf26e8d87                          // vsub.f32      d24, d30, d7
  .long  0xf26eed83                          // vsub.f32      d30, d30, d3
  .long  0xf3433d94                          // vmul.f32      d19, d19, d4
  .long  0xf24f7da7                          // vadd.f32      d23, d31, d23
  .long  0xf3489d91                          // vmul.f32      d25, d24, d1
  .long  0xf348cd90                          // vmul.f32      d28, d24, d0
  .long  0xf34edd94                          // vmul.f32      d29, d30, d4
  .long  0xf34ebd95                          // vmul.f32      d27, d30, d5
  .long  0xf3488d92                          // vmul.f32      d24, d24, d2
  .long  0xf34efd96                          // vmul.f32      d31, d30, d6
  .long  0xf24dcdac                          // vadd.f32      d28, d29, d28
  .long  0xf343dd15                          // vmul.f32      d29, d3, d5
  .long  0xf24b9da9                          // vadd.f32      d25, d27, d25
  .long  0xf343bd16                          // vmul.f32      d27, d3, d6
  .long  0xf3466d96                          // vmul.f32      d22, d22, d6
  .long  0xf24f8da8                          // vadd.f32      d24, d31, d24
  .long  0xf24d5da5                          // vadd.f32      d21, d29, d21
  .long  0xf24b2da2                          // vadd.f32      d18, d27, d18
  .long  0xf35311b7                          // vbsl          d17, d19, d23
  .long  0xf35041b5                          // vbsl          d20, d16, d21
  .long  0xf356a1b2                          // vbsl          d26, d22, d18
  .long  0xf2073c3e                          // vfma.f32      d3, d7, d30
  .long  0xf20c0da1                          // vadd.f32      d0, d28, d17
  .long  0xf2091da4                          // vadd.f32      d1, d25, d20
  .long  0xf2082daa                          // vadd.f32      d2, d24, d26
  .long  0xecbd8b06                          // vpop          {d8-d10}
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_clamp_0_vfp4
.globl _sk_clamp_0_vfp4
_sk_clamp_0_vfp4:
  .long  0xf2c00010                          // vmov.i32      d16, #0
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf2000f20                          // vmax.f32      d0, d0, d16
  .long  0xf2011f20                          // vmax.f32      d1, d1, d16
  .long  0xf2022f20                          // vmax.f32      d2, d2, d16
  .long  0xf2033f20                          // vmax.f32      d3, d3, d16
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_clamp_1_vfp4
.globl _sk_clamp_1_vfp4
_sk_clamp_1_vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf2200f20                          // vmin.f32      d0, d0, d16
  .long  0xf2211f20                          // vmin.f32      d1, d1, d16
  .long  0xf2222f20                          // vmin.f32      d2, d2, d16
  .long  0xf2233f20                          // vmin.f32      d3, d3, d16
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_clamp_a_vfp4
.globl _sk_clamp_a_vfp4
_sk_clamp_a_vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf2233f20                          // vmin.f32      d3, d3, d16
  .long  0xf2200f03                          // vmin.f32      d0, d0, d3
  .long  0xf2211f03                          // vmin.f32      d1, d1, d3
  .long  0xf2222f03                          // vmin.f32      d2, d2, d3
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_set_rgb_vfp4
.globl _sk_set_rgb_vfp4
_sk_set_rgb_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe283e008                          // add           lr, r3, #8
  .long  0xf4a30c9d                          // vld1.32       {d0[]}, [r3 :32]!
  .long  0xf4ae2c9f                          // vld1.32       {d2[]}, [lr :32]
  .long  0xf4a31c9f                          // vld1.32       {d1[]}, [r3 :32]
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

HIDDEN _sk_swap_rb_vfp4
.globl _sk_swap_rb_vfp4
_sk_swap_rb_vfp4:
  .long  0xeef00b40                          // vmov.f64      d16, d0
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xeeb00b42                          // vmov.f64      d0, d2
  .long  0xeeb02b60                          // vmov.f64      d2, d16
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_swap_vfp4
.globl _sk_swap_vfp4
_sk_swap_vfp4:
  .long  0xeef00b43                          // vmov.f64      d16, d3
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xeef01b42                          // vmov.f64      d17, d2
  .long  0xeef02b41                          // vmov.f64      d18, d1
  .long  0xeef03b40                          // vmov.f64      d19, d0
  .long  0xeeb00b44                          // vmov.f64      d0, d4
  .long  0xeeb01b45                          // vmov.f64      d1, d5
  .long  0xeeb02b46                          // vmov.f64      d2, d6
  .long  0xeeb03b47                          // vmov.f64      d3, d7
  .long  0xeeb04b63                          // vmov.f64      d4, d19
  .long  0xeeb05b62                          // vmov.f64      d5, d18
  .long  0xeeb06b61                          // vmov.f64      d6, d17
  .long  0xeeb07b60                          // vmov.f64      d7, d16
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_move_src_dst_vfp4
.globl _sk_move_src_dst_vfp4
_sk_move_src_dst_vfp4:
  .long  0xeeb04b40                          // vmov.f64      d4, d0
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xeeb05b41                          // vmov.f64      d5, d1
  .long  0xeeb06b42                          // vmov.f64      d6, d2
  .long  0xeeb07b43                          // vmov.f64      d7, d3
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_move_dst_src_vfp4
.globl _sk_move_dst_src_vfp4
_sk_move_dst_src_vfp4:
  .long  0xeeb00b44                          // vmov.f64      d0, d4
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xeeb01b45                          // vmov.f64      d1, d5
  .long  0xeeb02b46                          // vmov.f64      d2, d6
  .long  0xeeb03b47                          // vmov.f64      d3, d7
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_premul_vfp4
.globl _sk_premul_vfp4
_sk_premul_vfp4:
  .long  0xf3000d13                          // vmul.f32      d0, d0, d3
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf3011d13                          // vmul.f32      d1, d1, d3
  .long  0xf3022d13                          // vmul.f32      d2, d2, d3
  .long  0xe12fff13                          // bx            r3

HIDDEN _sk_unpremul_vfp4
.globl _sk_unpremul_vfp4
_sk_unpremul_vfp4:
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xeeb78a00                          // vmov.f32      s16, #112
  .long  0xf3f91503                          // vceq.f32      d17, d3, #0
  .long  0xf2c00010                          // vmov.i32      d16, #0
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xeec89a23                          // vdiv.f32      s19, s16, s7
  .long  0xee889a03                          // vdiv.f32      s18, s16, s6
  .long  0xf3501199                          // vbsl          d17, d16, d9
  .long  0xf3010d90                          // vmul.f32      d0, d17, d0
  .long  0xf3011d91                          // vmul.f32      d1, d17, d1
  .long  0xf3012d92                          // vmul.f32      d2, d17, d2
  .long  0xecbd8b04                          // vpop          {d8-d9}
  .long  0xe12fff13                          // bx            r3
  .long  0xe320f000                          // nop           {0}

HIDDEN _sk_from_srgb_vfp4
.globl _sk_from_srgb_vfp4
_sk_from_srgb_vfp4:
  .long  0xeddf3b20                          // vldr          d19, [pc, #128]
  .long  0xf3408d10                          // vmul.f32      d24, d0, d0
  .long  0xeddf0b1c                          // vldr          d16, [pc, #112]
  .long  0xf26341b3                          // vorr          d20, d19, d19
  .long  0xf26351b3                          // vorr          d21, d19, d19
  .long  0xeddf9b1f                          // vldr          d25, [pc, #124]
  .long  0xf2404c30                          // vfma.f32      d20, d0, d16
  .long  0xeddf2b1b                          // vldr          d18, [pc, #108]
  .long  0xf2415c30                          // vfma.f32      d21, d1, d16
  .long  0xeddfcb1d                          // vldr          d28, [pc, #116]
  .long  0xf2423c30                          // vfma.f32      d19, d2, d16
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf3426d12                          // vmul.f32      d22, d2, d2
  .long  0xf3417d11                          // vmul.f32      d23, d1, d1
  .long  0xf3620e80                          // vcgt.f32      d16, d18, d0
  .long  0xf3621e81                          // vcgt.f32      d17, d18, d1
  .long  0xf341ad39                          // vmul.f32      d26, d1, d25
  .long  0xf342bd39                          // vmul.f32      d27, d2, d25
  .long  0xf3622e82                          // vcgt.f32      d18, d18, d2
  .long  0xf3409d39                          // vmul.f32      d25, d0, d25
  .long  0xf26cd1bc                          // vorr          d29, d28, d28
  .long  0xf248dcb4                          // vfma.f32      d29, d24, d20
  .long  0xf26c41bc                          // vorr          d20, d28, d28
  .long  0xf2474cb5                          // vfma.f32      d20, d23, d21
  .long  0xf246ccb3                          // vfma.f32      d28, d22, d19
  .long  0xf35901bd                          // vbsl          d16, d25, d29
  .long  0xf35a11b4                          // vbsl          d17, d26, d20
  .long  0xf35b21bc                          // vbsl          d18, d27, d28
  .long  0xf22001b0                          // vorr          d0, d16, d16
  .long  0xf22111b1                          // vorr          d1, d17, d17
  .long  0xf22221b2                          // vorr          d2, d18, d18
  .long  0xe12fff13                          // bx            r3
  .long  0x3e99999a                          // .word         0x3e99999a
  .long  0x3e99999a                          // .word         0x3e99999a
  .long  0x3f328f5c                          // .word         0x3f328f5c
  .long  0x3f328f5c                          // .word         0x3f328f5c
  .long  0x3d6147ae                          // .word         0x3d6147ae
  .long  0x3d6147ae                          // .word         0x3d6147ae
  .long  0x3d9e8391                          // .word         0x3d9e8391
  .long  0x3d9e8391                          // .word         0x3d9e8391
  .long  0x3b23d70a                          // .word         0x3b23d70a
  .long  0x3b23d70a                          // .word         0x3b23d70a

HIDDEN _sk_to_srgb_vfp4
.globl _sk_to_srgb_vfp4
_sk_to_srgb_vfp4:
  .long  0xf3fb0582                          // vrsqrte.f32   d16, d2
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf3fb1581                          // vrsqrte.f32   d17, d1
  .long  0xf3fb2580                          // vrsqrte.f32   d18, d0
  .long  0xf3403db0                          // vmul.f32      d19, d16, d16
  .long  0xf3414db1                          // vmul.f32      d20, d17, d17
  .long  0xf3425db2                          // vmul.f32      d21, d18, d18
  .long  0xf2623f33                          // vrsqrts.f32   d19, d2, d19
  .long  0xf2614f34                          // vrsqrts.f32   d20, d1, d20
  .long  0xf2605f35                          // vrsqrts.f32   d21, d0, d21
  .long  0xf3400db3                          // vmul.f32      d16, d16, d19
  .long  0xf3411db4                          // vmul.f32      d17, d17, d20
  .long  0xf3422db5                          // vmul.f32      d18, d18, d21
  .long  0xf3fb3520                          // vrecpe.f32    d19, d16
  .long  0xf3fb4521                          // vrecpe.f32    d20, d17
  .long  0xf3fb6522                          // vrecpe.f32    d22, d18
  .long  0xf3fb55a0                          // vrsqrte.f32   d21, d16
  .long  0xf3fb75a1                          // vrsqrte.f32   d23, d17
  .long  0xf3fb85a2                          // vrsqrte.f32   d24, d18
  .long  0xf2409fb3                          // vrecps.f32    d25, d16, d19
  .long  0xf241afb4                          // vrecps.f32    d26, d17, d20
  .long  0xf242bfb6                          // vrecps.f32    d27, d18, d22
  .long  0xf345cdb5                          // vmul.f32      d28, d21, d21
  .long  0xf347ddb7                          // vmul.f32      d29, d23, d23
  .long  0xf348edb8                          // vmul.f32      d30, d24, d24
  .long  0xf2600fbc                          // vrsqrts.f32   d16, d16, d28
  .long  0xf2611fbd                          // vrsqrts.f32   d17, d17, d29
  .long  0xf2622fbe                          // vrsqrts.f32   d18, d18, d30
  .long  0xf3433db9                          // vmul.f32      d19, d19, d25
  .long  0xeddf9b21                          // vldr          d25, [pc, #132]
  .long  0xf3444dba                          // vmul.f32      d20, d20, d26
  .long  0xeddfab21                          // vldr          d26, [pc, #132]
  .long  0xf3466dbb                          // vmul.f32      d22, d22, d27
  .long  0xf26ab1ba                          // vorr          d27, d26, d26
  .long  0xf243bcb9                          // vfma.f32      d27, d19, d25
  .long  0xf26a31ba                          // vorr          d19, d26, d26
  .long  0xf2443cb9                          // vfma.f32      d19, d20, d25
  .long  0xeddf4b1d                          // vldr          d20, [pc, #116]
  .long  0xf246acb9                          // vfma.f32      d26, d22, d25
  .long  0xf3450db0                          // vmul.f32      d16, d21, d16
  .long  0xeddf5b1c                          // vldr          d21, [pc, #112]
  .long  0xf3471db1                          // vmul.f32      d17, d23, d17
  .long  0xf3482db2                          // vmul.f32      d18, d24, d18
  .long  0xf3406d35                          // vmul.f32      d22, d0, d21
  .long  0xf240bcb4                          // vfma.f32      d27, d16, d20
  .long  0xf2413cb4                          // vfma.f32      d19, d17, d20
  .long  0xf242acb4                          // vfma.f32      d26, d18, d20
  .long  0xeddf2b17                          // vldr          d18, [pc, #92]
  .long  0xf3417d35                          // vmul.f32      d23, d1, d21
  .long  0xf3620e80                          // vcgt.f32      d16, d18, d0
  .long  0xf3621e81                          // vcgt.f32      d17, d18, d1
  .long  0xf3622e82                          // vcgt.f32      d18, d18, d2
  .long  0xf3425d35                          // vmul.f32      d21, d2, d21
  .long  0xf2c74f10                          // vmov.f32      d20, #1
  .long  0xf2648faa                          // vmin.f32      d24, d20, d26
  .long  0xf2643fa3                          // vmin.f32      d19, d20, d19
  .long  0xf2644fab                          // vmin.f32      d20, d20, d27
  .long  0xf35601b8                          // vbsl          d16, d22, d24
  .long  0xf35711b3                          // vbsl          d17, d23, d19
  .long  0xf35521b4                          // vbsl          d18, d21, d20
  .long  0xf22001b0                          // vorr          d0, d16, d16
  .long  0xf22111b1                          // vorr          d1, d17, d17
  .long  0xf22221b2                          // vorr          d2, d18, d18
  .long  0xe12fff13                          // bx            r3
  .long  0x3f306fce                          // .word         0x3f306fce
  .long  0x3f306fce                          // .word         0x3f306fce
  .long  0xbdca57a8                          // .word         0xbdca57a8
  .long  0xbdca57a8                          // .word         0xbdca57a8
  .long  0x3ed287c2                          // .word         0x3ed287c2
  .long  0x3ed287c2                          // .word         0x3ed287c2
  .long  0x41475c29                          // .word         0x41475c29
  .long  0x41475c29                          // .word         0x41475c29
  .long  0x3b8ce704                          // .word         0x3b8ce704
  .long  0x3b8ce704                          // .word         0x3b8ce704

HIDDEN _sk_scale_1_float_vfp4
.globl _sk_scale_1_float_vfp4
_sk_scale_1_float_vfp4:
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf4e30c9f                          // vld1.32       {d16[]}, [r3 :32]
  .long  0xf3000d90                          // vmul.f32      d0, d16, d0
  .long  0xf3001d91                          // vmul.f32      d1, d16, d1
  .long  0xf3002d92                          // vmul.f32      d2, d16, d2
  .long  0xf3003d93                          // vmul.f32      d3, d16, d3
  .long  0xe12fff1c                          // bx            ip

HIDDEN _sk_scale_u8_vfp4
.globl _sk_scale_u8_vfp4
_sk_scale_u8_vfp4:
  .long  0xe24dd004                          // sub           sp, sp, #4
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe5933000                          // ldr           r3, [r3]
  .long  0xe0833000                          // add           r3, r3, r0
  .long  0xe1d330b0                          // ldrh          r3, [r3]
  .long  0xe1cd30b0                          // strh          r3, [sp]
  .long  0xe1a0300d                          // mov           r3, sp
  .long  0xf4e3041f                          // vld1.16       {d16[0]}, [r3 :16]
  .long  0xf3c80a30                          // vmovl.u8      q8, d16
  .long  0xf3d00a30                          // vmovl.u16     q8, d16
  .long  0xf3fb06a0                          // vcvt.f32.u32  d16, d16
  .long  0xeddf1b06                          // vldr          d17, [pc, #24]
  .long  0xf3400db1                          // vmul.f32      d16, d16, d17
  .long  0xf3000d90                          // vmul.f32      d0, d16, d0
  .long  0xf3001d91                          // vmul.f32      d1, d16, d1
  .long  0xf3002d92                          // vmul.f32      d2, d16, d2
  .long  0xf3003d93                          // vmul.f32      d3, d16, d3
  .long  0xe28dd004                          // add           sp, sp, #4
  .long  0xe12fff1c                          // bx            ip
  .long  0x3b808081                          // .word         0x3b808081
  .long  0x3b808081                          // .word         0x3b808081

HIDDEN _sk_lerp_1_float_vfp4
.globl _sk_lerp_1_float_vfp4
_sk_lerp_1_float_vfp4:
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xf2600d04                          // vsub.f32      d16, d0, d4
  .long  0xf2611d05                          // vsub.f32      d17, d1, d5
  .long  0xf2622d06                          // vsub.f32      d18, d2, d6
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2633d07                          // vsub.f32      d19, d3, d7
  .long  0xf4e34c9f                          // vld1.32       {d20[]}, [r3 :32]
  .long  0xf2240114                          // vorr          d0, d4, d4
  .long  0xf2251115                          // vorr          d1, d5, d5
  .long  0xf2262116                          // vorr          d2, d6, d6
  .long  0xf2273117                          // vorr          d3, d7, d7
  .long  0xf2000cb4                          // vfma.f32      d0, d16, d20
  .long  0xf2011cb4                          // vfma.f32      d1, d17, d20
  .long  0xf2022cb4                          // vfma.f32      d2, d18, d20
  .long  0xf2033cb4                          // vfma.f32      d3, d19, d20
  .long  0xe12fff1c                          // bx            ip

HIDDEN _sk_lerp_u8_vfp4
.globl _sk_lerp_u8_vfp4
_sk_lerp_u8_vfp4:
  .long  0xe24dd004                          // sub           sp, sp, #4
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xf2602d04                          // vsub.f32      d18, d0, d4
  .long  0xf2623d06                          // vsub.f32      d19, d2, d6
  .long  0xf2634d07                          // vsub.f32      d20, d3, d7
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe5933000                          // ldr           r3, [r3]
  .long  0xf2240114                          // vorr          d0, d4, d4
  .long  0xf2262116                          // vorr          d2, d6, d6
  .long  0xe0833000                          // add           r3, r3, r0
  .long  0xf2273117                          // vorr          d3, d7, d7
  .long  0xe1d330b0                          // ldrh          r3, [r3]
  .long  0xe1cd30b0                          // strh          r3, [sp]
  .long  0xe1a0300d                          // mov           r3, sp
  .long  0xf4e3041f                          // vld1.16       {d16[0]}, [r3 :16]
  .long  0xf3c80a30                          // vmovl.u8      q8, d16
  .long  0xf3d00a30                          // vmovl.u16     q8, d16
  .long  0xf3fb06a0                          // vcvt.f32.u32  d16, d16
  .long  0xeddf1b08                          // vldr          d17, [pc, #32]
  .long  0xf3400db1                          // vmul.f32      d16, d16, d17
  .long  0xf2611d05                          // vsub.f32      d17, d1, d5
  .long  0xf2251115                          // vorr          d1, d5, d5
  .long  0xf2020cb0                          // vfma.f32      d0, d18, d16
  .long  0xf2011cb0                          // vfma.f32      d1, d17, d16
  .long  0xf2032cb0                          // vfma.f32      d2, d19, d16
  .long  0xf2043cb0                          // vfma.f32      d3, d20, d16
  .long  0xe28dd004                          // add           sp, sp, #4
  .long  0xe12fff1c                          // bx            ip
  .long  0x3b808081                          // .word         0x3b808081
  .long  0x3b808081                          // .word         0x3b808081

HIDDEN _sk_lerp_565_vfp4
.globl _sk_lerp_565_vfp4
_sk_lerp_565_vfp4:
  .long  0xe24dd004                          // sub           sp, sp, #4
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xf3c72218                          // vmov.i32      d18, #63488
  .long  0xf2c1101f                          // vmov.i32      d17, #31
  .long  0xf2603d04                          // vsub.f32      d19, d0, d4
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe5933000                          // ldr           r3, [r3]
  .long  0xf2616d05                          // vsub.f32      d22, d1, d5
  .long  0xf2240114                          // vorr          d0, d4, d4
  .long  0xf2251115                          // vorr          d1, d5, d5
  .long  0xe7933080                          // ldr           r3, [r3, r0, lsl #1]
  .long  0xf2873f10                          // vmov.f32      d3, #1
  .long  0xe58d3000                          // str           r3, [sp]
  .long  0xe1a0300d                          // mov           r3, sp
  .long  0xf4e3083f                          // vld1.32       {d16[0]}, [r3 :32]
  .long  0xe3a03e7e                          // mov           r3, #2016
  .long  0xf3d04a30                          // vmovl.u16     q10, d16
  .long  0xee803b90                          // vdup.32       d16, r3
  .long  0xf24421b2                          // vand          d18, d20, d18
  .long  0xf24411b1                          // vand          d17, d20, d17
  .long  0xeddf5b12                          // vldr          d21, [pc, #72]
  .long  0xf24401b0                          // vand          d16, d20, d16
  .long  0xeddf4b0e                          // vldr          d20, [pc, #56]
  .long  0xf3fb2622                          // vcvt.f32.s32  d18, d18
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xf3fb1621                          // vcvt.f32.s32  d17, d17
  .long  0xf3422db4                          // vmul.f32      d18, d18, d20
  .long  0xeddf4b0d                          // vldr          d20, [pc, #52]
  .long  0xf3400db5                          // vmul.f32      d16, d16, d21
  .long  0xf2625d06                          // vsub.f32      d21, d2, d6
  .long  0xf3411db4                          // vmul.f32      d17, d17, d20
  .long  0xf2262116                          // vorr          d2, d6, d6
  .long  0xf2030cb2                          // vfma.f32      d0, d19, d18
  .long  0xf2061cb0                          // vfma.f32      d1, d22, d16
  .long  0xf2052cb1                          // vfma.f32      d2, d21, d17
  .long  0xe28dd004                          // add           sp, sp, #4
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}
  .long  0x37842108                          // .word         0x37842108
  .long  0x37842108                          // .word         0x37842108
  .long  0x3a020821                          // .word         0x3a020821
  .long  0x3a020821                          // .word         0x3a020821
  .long  0x3d042108                          // .word         0x3d042108
  .long  0x3d042108                          // .word         0x3d042108

HIDDEN _sk_load_tables_vfp4
.globl _sk_load_tables_vfp4
_sk_load_tables_vfp4:
  .long  0xe92d48f0                          // push          {r4, r5, r6, r7, fp, lr}
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xf3c7001f                          // vmov.i32      d16, #255
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe593e000                          // ldr           lr, [r3]
  .long  0xe99300b0                          // ldmib         r3, {r4, r5, r7}
  .long  0xe08e3100                          // add           r3, lr, r0, lsl #2
  .long  0xedd31b00                          // vldr          d17, [r3]
  .long  0xf24121b0                          // vand          d18, d17, d16
  .long  0xf3f83031                          // vshr.u32      d19, d17, #8
  .long  0xee323b90                          // vmov.32       r3, d18[1]
  .long  0xee126b90                          // vmov.32       r6, d18[0]
  .long  0xf3f02031                          // vshr.u32      d18, d17, #16
  .long  0xf24221b0                          // vand          d18, d18, d16
  .long  0xf24301b0                          // vand          d16, d19, d16
  .long  0xe0843103                          // add           r3, r4, r3, lsl #2
  .long  0xedd30a00                          // vldr          s1, [r3]
  .long  0xe0843106                          // add           r3, r4, r6, lsl #2
  .long  0xee326b90                          // vmov.32       r6, d18[1]
  .long  0xed930a00                          // vldr          s0, [r3]
  .long  0xee303b90                          // vmov.32       r3, d16[1]
  .long  0xee104b90                          // vmov.32       r4, d16[0]
  .long  0xf3e80031                          // vshr.u32      d16, d17, #24
  .long  0xeddf1b0d                          // vldr          d17, [pc, #52]
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xf3003db1                          // vmul.f32      d3, d16, d17
  .long  0xe087e106                          // add           lr, r7, r6, lsl #2
  .long  0xee126b90                          // vmov.32       r6, d18[0]
  .long  0xe0853103                          // add           r3, r5, r3, lsl #2
  .long  0xedde2a00                          // vldr          s5, [lr]
  .long  0xedd31a00                          // vldr          s3, [r3]
  .long  0xe0853104                          // add           r3, r5, r4, lsl #2
  .long  0xed931a00                          // vldr          s2, [r3]
  .long  0xe0873106                          // add           r3, r7, r6, lsl #2
  .long  0xed932a00                          // vldr          s4, [r3]
  .long  0xe8bd48f0                          // pop           {r4, r5, r6, r7, fp, lr}
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}
  .long  0x3b808081                          // .word         0x3b808081
  .long  0x3b808081                          // .word         0x3b808081

HIDDEN _sk_load_a8_vfp4
.globl _sk_load_a8_vfp4
_sk_load_a8_vfp4:
  .long  0xe24dd004                          // sub           sp, sp, #4
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2800010                          // vmov.i32      d0, #0
  .long  0xf2801010                          // vmov.i32      d1, #0
  .long  0xe5933000                          // ldr           r3, [r3]
  .long  0xf2802010                          // vmov.i32      d2, #0
  .long  0xe0833000                          // add           r3, r3, r0
  .long  0xe1d330b0                          // ldrh          r3, [r3]
  .long  0xe1cd30b0                          // strh          r3, [sp]
  .long  0xe1a0300d                          // mov           r3, sp
  .long  0xf4e3041f                          // vld1.16       {d16[0]}, [r3 :16]
  .long  0xf3c80a30                          // vmovl.u8      q8, d16
  .long  0xf3d00a30                          // vmovl.u16     q8, d16
  .long  0xf3fb06a0                          // vcvt.f32.u32  d16, d16
  .long  0xeddf1b03                          // vldr          d17, [pc, #12]
  .long  0xf3003db1                          // vmul.f32      d3, d16, d17
  .long  0xe28dd004                          // add           sp, sp, #4
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}
  .long  0x3b808081                          // .word         0x3b808081
  .long  0x3b808081                          // .word         0x3b808081

HIDDEN _sk_store_a8_vfp4
.globl _sk_store_a8_vfp4
_sk_store_a8_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xeddf0b0d                          // vldr          d16, [pc, #52]
  .long  0xf2c3161f                          // vmov.i32      d17, #1056964608
  .long  0xf2431c30                          // vfma.f32      d17, d3, d16
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xe5933000                          // ldr           r3, [r3]
  .long  0xf3fb07a1                          // vcvt.u32.f32  d16, d17
  .long  0xee10eb90                          // vmov.32       lr, d16[0]
  .long  0xee30cb90                          // vmov.32       ip, d16[1]
  .long  0xe7e3e000                          // strb          lr, [r3, r0]!
  .long  0xe5c3c001                          // strb          ip, [r3, #1]
  .long  0xe2813008                          // add           r3, r1, #8
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe1a01003                          // mov           r1, r3
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip
  .long  0x437f0000                          // .word         0x437f0000
  .long  0x437f0000                          // .word         0x437f0000

HIDDEN _sk_load_g8_vfp4
.globl _sk_load_g8_vfp4
_sk_load_g8_vfp4:
  .long  0xe24dd004                          // sub           sp, sp, #4
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2873f10                          // vmov.f32      d3, #1
  .long  0xe5933000                          // ldr           r3, [r3]
  .long  0xe0833000                          // add           r3, r3, r0
  .long  0xe1d330b0                          // ldrh          r3, [r3]
  .long  0xe1cd30b0                          // strh          r3, [sp]
  .long  0xe1a0300d                          // mov           r3, sp
  .long  0xf4e3041f                          // vld1.16       {d16[0]}, [r3 :16]
  .long  0xf3c80a30                          // vmovl.u8      q8, d16
  .long  0xf3d00a30                          // vmovl.u16     q8, d16
  .long  0xf3fb06a0                          // vcvt.f32.u32  d16, d16
  .long  0xeddf1b05                          // vldr          d17, [pc, #20]
  .long  0xf3000db1                          // vmul.f32      d0, d16, d17
  .long  0xf2201110                          // vorr          d1, d0, d0
  .long  0xf2202110                          // vorr          d2, d0, d0
  .long  0xe28dd004                          // add           sp, sp, #4
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}
  .long  0x3b808081                          // .word         0x3b808081
  .long  0x3b808081                          // .word         0x3b808081

HIDDEN _sk_load_565_vfp4
.globl _sk_load_565_vfp4
_sk_load_565_vfp4:
  .long  0xe24dd004                          // sub           sp, sp, #4
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xf2c1101f                          // vmov.i32      d17, #31
  .long  0xf3c72218                          // vmov.i32      d18, #63488
  .long  0xeddf3b16                          // vldr          d19, [pc, #88]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe5933000                          // ldr           r3, [r3]
  .long  0xf2873f10                          // vmov.f32      d3, #1
  .long  0xe7933080                          // ldr           r3, [r3, r0, lsl #1]
  .long  0xe58d3000                          // str           r3, [sp]
  .long  0xe1a0300d                          // mov           r3, sp
  .long  0xf4e3083f                          // vld1.32       {d16[0]}, [r3 :32]
  .long  0xe3a03e7e                          // mov           r3, #2016
  .long  0xf3d04a30                          // vmovl.u16     q10, d16
  .long  0xee803b90                          // vdup.32       d16, r3
  .long  0xf24411b1                          // vand          d17, d20, d17
  .long  0xeddf5b0e                          // vldr          d21, [pc, #56]
  .long  0xf24421b2                          // vand          d18, d20, d18
  .long  0xf24401b0                          // vand          d16, d20, d16
  .long  0xeddf4b09                          // vldr          d20, [pc, #36]
  .long  0xf3fb2622                          // vcvt.f32.s32  d18, d18
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xf3fb1621                          // vcvt.f32.s32  d17, d17
  .long  0xf3020db3                          // vmul.f32      d0, d18, d19
  .long  0xf3001db4                          // vmul.f32      d1, d16, d20
  .long  0xf3012db5                          // vmul.f32      d2, d17, d21
  .long  0xe28dd004                          // add           sp, sp, #4
  .long  0xe12fff1c                          // bx            ip
  .long  0x37842108                          // .word         0x37842108
  .long  0x37842108                          // .word         0x37842108
  .long  0x3a020821                          // .word         0x3a020821
  .long  0x3a020821                          // .word         0x3a020821
  .long  0x3d042108                          // .word         0x3d042108
  .long  0x3d042108                          // .word         0x3d042108

HIDDEN _sk_store_565_vfp4
.globl _sk_store_565_vfp4
_sk_store_565_vfp4:
  .long  0xf2c30f1f                          // vmov.f32      d16, #31
  .long  0xeddf1b15                          // vldr          d17, [pc, #84]
  .long  0xf2c3361f                          // vmov.i32      d19, #1056964608
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf2413c31                          // vfma.f32      d19, d1, d17
  .long  0xf2c3161f                          // vmov.i32      d17, #1056964608
  .long  0xf2401c30                          // vfma.f32      d17, d0, d16
  .long  0xe5933000                          // ldr           r3, [r3]
  .long  0xf2c3261f                          // vmov.i32      d18, #1056964608
  .long  0xf2422c30                          // vfma.f32      d18, d2, d16
  .long  0xe0833080                          // add           r3, r3, r0, lsl #1
  .long  0xf3fb07a3                          // vcvt.u32.f32  d16, d19
  .long  0xf3fb17a1                          // vcvt.u32.f32  d17, d17
  .long  0xf3fb27a2                          // vcvt.u32.f32  d18, d18
  .long  0xf2e50530                          // vshl.s32      d16, d16, #5
  .long  0xf2eb1531                          // vshl.s32      d17, d17, #11
  .long  0xf26001b1                          // vorr          d16, d16, d17
  .long  0xf26001b2                          // vorr          d16, d16, d18
  .long  0xf3f60121                          // vuzp.16       d16, d17
  .long  0xf4c3080f                          // vst1.32       {d16[0]}, [r3]
  .long  0xe2813008                          // add           r3, r1, #8
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe1a01003                          // mov           r1, r3
  .long  0xe12fff1c                          // bx            ip
  .long  0x427c0000                          // .word         0x427c0000
  .long  0x427c0000                          // .word         0x427c0000

HIDDEN _sk_load_4444_vfp4
.globl _sk_load_4444_vfp4
_sk_load_4444_vfp4:
  .long  0xe24dd004                          // sub           sp, sp, #4
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xf3c71210                          // vmov.i32      d17, #61440
  .long  0xf3c74010                          // vmov.i32      d20, #240
  .long  0xf2c0501f                          // vmov.i32      d21, #15
  .long  0xeddf6b1d                          // vldr          d22, [pc, #116]
  .long  0xe5933000                          // ldr           r3, [r3]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe7933080                          // ldr           r3, [r3, r0, lsl #1]
  .long  0xe58d3000                          // str           r3, [sp]
  .long  0xe1a0300d                          // mov           r3, sp
  .long  0xf4e3083f                          // vld1.32       {d16[0]}, [r3 :32]
  .long  0xf3d02a30                          // vmovl.u16     q9, d16
  .long  0xf2c0021f                          // vmov.i32      d16, #3840
  .long  0xf24211b1                          // vand          d17, d18, d17
  .long  0xf24201b0                          // vand          d16, d18, d16
  .long  0xf24241b4                          // vand          d20, d18, d20
  .long  0xf24221b5                          // vand          d18, d18, d21
  .long  0xeddf3b0c                          // vldr          d19, [pc, #48]
  .long  0xf3fb1621                          // vcvt.f32.s32  d17, d17
  .long  0xeddf5b0c                          // vldr          d21, [pc, #48]
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xf3fb4624                          // vcvt.f32.s32  d20, d20
  .long  0xf3fb2622                          // vcvt.f32.s32  d18, d18
  .long  0xf3010db3                          // vmul.f32      d0, d17, d19
  .long  0xeddf1b0b                          // vldr          d17, [pc, #44]
  .long  0xf3001db5                          // vmul.f32      d1, d16, d21
  .long  0xf3042db6                          // vmul.f32      d2, d20, d22
  .long  0xf3023db1                          // vmul.f32      d3, d18, d17
  .long  0xe28dd004                          // add           sp, sp, #4
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}
  .long  0x37888889                          // .word         0x37888889
  .long  0x37888889                          // .word         0x37888889
  .long  0x39888889                          // .word         0x39888889
  .long  0x39888889                          // .word         0x39888889
  .long  0x3b888889                          // .word         0x3b888889
  .long  0x3b888889                          // .word         0x3b888889
  .long  0x3d888889                          // .word         0x3d888889
  .long  0x3d888889                          // .word         0x3d888889

HIDDEN _sk_store_4444_vfp4
.globl _sk_store_4444_vfp4
_sk_store_4444_vfp4:
  .long  0xf2c20f1e                          // vmov.f32      d16, #15
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf2c3261f                          // vmov.i32      d18, #1056964608
  .long  0xf2c3361f                          // vmov.i32      d19, #1056964608
  .long  0xf2402c30                          // vfma.f32      d18, d0, d16
  .long  0xe5933000                          // ldr           r3, [r3]
  .long  0xf2413c30                          // vfma.f32      d19, d1, d16
  .long  0xf2c3461f                          // vmov.i32      d20, #1056964608
  .long  0xe0833080                          // add           r3, r3, r0, lsl #1
  .long  0xf2424c30                          // vfma.f32      d20, d2, d16
  .long  0xf2c3161f                          // vmov.i32      d17, #1056964608
  .long  0xf2431c30                          // vfma.f32      d17, d3, d16
  .long  0xf3fb07a2                          // vcvt.u32.f32  d16, d18
  .long  0xf3fb27a3                          // vcvt.u32.f32  d18, d19
  .long  0xf3fb37a4                          // vcvt.u32.f32  d19, d20
  .long  0xf2ec0530                          // vshl.s32      d16, d16, #12
  .long  0xf2e82532                          // vshl.s32      d18, d18, #8
  .long  0xf3fb17a1                          // vcvt.u32.f32  d17, d17
  .long  0xf2e43533                          // vshl.s32      d19, d19, #4
  .long  0xf26201b0                          // vorr          d16, d18, d16
  .long  0xf26001b3                          // vorr          d16, d16, d19
  .long  0xf26001b1                          // vorr          d16, d16, d17
  .long  0xf3f60121                          // vuzp.16       d16, d17
  .long  0xf4c3080f                          // vst1.32       {d16[0]}, [r3]
  .long  0xe2813008                          // add           r3, r1, #8
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe1a01003                          // mov           r1, r3
  .long  0xe12fff1c                          // bx            ip

HIDDEN _sk_load_8888_vfp4
.globl _sk_load_8888_vfp4
_sk_load_8888_vfp4:
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xf3c7001f                          // vmov.i32      d16, #255
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe5933000                          // ldr           r3, [r3]
  .long  0xe0833100                          // add           r3, r3, r0, lsl #2
  .long  0xedd31b00                          // vldr          d17, [r3]
  .long  0xf24121b0                          // vand          d18, d17, d16
  .long  0xf3f83031                          // vshr.u32      d19, d17, #8
  .long  0xf3e84031                          // vshr.u32      d20, d17, #24
  .long  0xf3f01031                          // vshr.u32      d17, d17, #16
  .long  0xf24331b0                          // vand          d19, d19, d16
  .long  0xf24101b0                          // vand          d16, d17, d16
  .long  0xeddf1b08                          // vldr          d17, [pc, #32]
  .long  0xf3fb2622                          // vcvt.f32.s32  d18, d18
  .long  0xf3fb4624                          // vcvt.f32.s32  d20, d20
  .long  0xf3fb3623                          // vcvt.f32.s32  d19, d19
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xf3020db1                          // vmul.f32      d0, d18, d17
  .long  0xf3043db1                          // vmul.f32      d3, d20, d17
  .long  0xf3031db1                          // vmul.f32      d1, d19, d17
  .long  0xf3002db1                          // vmul.f32      d2, d16, d17
  .long  0xe12fff1c                          // bx            ip
  .long  0x3b808081                          // .word         0x3b808081
  .long  0x3b808081                          // .word         0x3b808081

HIDDEN _sk_store_8888_vfp4
.globl _sk_store_8888_vfp4
_sk_store_8888_vfp4:
  .long  0xeddf0b1a                          // vldr          d16, [pc, #104]
  .long  0xf2c3261f                          // vmov.i32      d18, #1056964608
  .long  0xf2412c30                          // vfma.f32      d18, d1, d16
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf2c3361f                          // vmov.i32      d19, #1056964608
  .long  0xf2c3161f                          // vmov.i32      d17, #1056964608
  .long  0xf2423c30                          // vfma.f32      d19, d2, d16
  .long  0xe5933000                          // ldr           r3, [r3]
  .long  0xf2c3461f                          // vmov.i32      d20, #1056964608
  .long  0xf2401c30                          // vfma.f32      d17, d0, d16
  .long  0xe0833100                          // add           r3, r3, r0, lsl #2
  .long  0xf2434c30                          // vfma.f32      d20, d3, d16
  .long  0xf3fb07a2                          // vcvt.u32.f32  d16, d18
  .long  0xf3fb27a3                          // vcvt.u32.f32  d18, d19
  .long  0xf3fb17a1                          // vcvt.u32.f32  d17, d17
  .long  0xf3fb37a4                          // vcvt.u32.f32  d19, d20
  .long  0xf2e80530                          // vshl.s32      d16, d16, #8
  .long  0xf2f02532                          // vshl.s32      d18, d18, #16
  .long  0xf26001b1                          // vorr          d16, d16, d17
  .long  0xf2f81533                          // vshl.s32      d17, d19, #24
  .long  0xf26001b2                          // vorr          d16, d16, d18
  .long  0xf26001b1                          // vorr          d16, d16, d17
  .long  0xedc30b00                          // vstr          d16, [r3]
  .long  0xe2813008                          // add           r3, r1, #8
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe1a01003                          // mov           r1, r3
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}
  .long  0x437f0000                          // .word         0x437f0000
  .long  0x437f0000                          // .word         0x437f0000

HIDDEN _sk_load_f16_vfp4
.globl _sk_load_f16_vfp4
_sk_load_f16_vfp4:
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe5933000                          // ldr           r3, [r3]
  .long  0xe0833180                          // add           r3, r3, r0, lsl #3
  .long  0xf4e3070d                          // vld4.16       {d16[0],d17[0],d18[0],d19[0]}, [r3]!
  .long  0xf4e3074f                          // vld4.16       {d16[1],d17[1],d18[1],d19[1]}, [r3]
  .long  0xf3b60720                          // vcvt.f32.f16  q0, d16
  .long  0xf3b62722                          // vcvt.f32.f16  q1, d18
  .long  0xf3f64721                          // vcvt.f32.f16  q10, d17
  .long  0xf3f60723                          // vcvt.f32.f16  q8, d19
  .long  0xf22411b4                          // vorr          d1, d20, d20
  .long  0xf22031b0                          // vorr          d3, d16, d16
  .long  0xe12fff1c                          // bx            ip

HIDDEN _sk_store_f16_vfp4
.globl _sk_store_f16_vfp4
_sk_store_f16_vfp4:
  .long  0xeef00b41                          // vmov.f64      d16, d1
  .long  0xeef03b42                          // vmov.f64      d19, d2
  .long  0xf2631113                          // vorr          d17, d3, d3
  .long  0xf2602110                          // vorr          d18, d0, d0
  .long  0xf3fa00a1                          // vtrn.32       d16, d17
  .long  0xf3f61620                          // vcvt.f16.f32  d17, q8
  .long  0xf3fa20a3                          // vtrn.32       d18, d19
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf3f60622                          // vcvt.f16.f32  d16, q9
  .long  0xe5933000                          // ldr           r3, [r3]
  .long  0xe0833180                          // add           r3, r3, r0, lsl #3
  .long  0xf443084f                          // vst2.16       {d16-d17}, [r3]
  .long  0xe2813008                          // add           r3, r1, #8
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe1a01003                          // mov           r1, r3
  .long  0xe12fff1c                          // bx            ip

HIDDEN _sk_store_f32_vfp4
.globl _sk_store_f32_vfp4
_sk_store_f32_vfp4:
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xe5933000                          // ldr           r3, [r3]
  .long  0xe0833200                          // add           r3, r3, r0, lsl #4
  .long  0xf403008f                          // vst4.32       {d0-d3}, [r3]
  .long  0xe2813008                          // add           r3, r1, #8
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe1a01003                          // mov           r1, r3
  .long  0xe12fff1c                          // bx            ip

HIDDEN _sk_clamp_x_vfp4
.globl _sk_clamp_x_vfp4
_sk_clamp_x_vfp4:
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xf2c00010                          // vmov.i32      d16, #0
  .long  0xf3c71e1f                          // vmov.i8       d17, #255
  .long  0xf2400f80                          // vmax.f32      d16, d16, d0
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf4e32c9f                          // vld1.32       {d18[]}, [r3 :32]
  .long  0xf26218a1                          // vadd.i32      d17, d18, d17
  .long  0xf2200fa1                          // vmin.f32      d0, d16, d17
  .long  0xe12fff1c                          // bx            ip

HIDDEN _sk_clamp_y_vfp4
.globl _sk_clamp_y_vfp4
_sk_clamp_y_vfp4:
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xf2c00010                          // vmov.i32      d16, #0
  .long  0xf3c71e1f                          // vmov.i8       d17, #255
  .long  0xf2400f81                          // vmax.f32      d16, d16, d1
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf4e32c9f                          // vld1.32       {d18[]}, [r3 :32]
  .long  0xf26218a1                          // vadd.i32      d17, d18, d17
  .long  0xf2201fa1                          // vmin.f32      d1, d16, d17
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}

HIDDEN _sk_repeat_x_vfp4
.globl _sk_repeat_x_vfp4
_sk_repeat_x_vfp4:
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xf2c02010                          // vmov.i32      d18, #0
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xeddf3b10                          // vldr          d19, [pc, #64]
  .long  0xed938a00                          // vldr          s16, [r3]
  .long  0xeec09a88                          // vdiv.f32      s19, s1, s16
  .long  0xee809a08                          // vdiv.f32      s18, s0, s16
  .long  0xf3fb0709                          // vcvt.s32.f32  d16, d9
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xf3601e89                          // vcgt.f32      d17, d16, d9
  .long  0xf35311b2                          // vbsl          d17, d19, d18
  .long  0xf3f42c08                          // vdup.32       d18, d8[0]
  .long  0xf2600da1                          // vsub.f32      d16, d16, d17
  .long  0xf3c71e1f                          // vmov.i8       d17, #255
  .long  0xf26218a1                          // vadd.i32      d17, d18, d17
  .long  0xf2e009c8                          // vmul.f32      d16, d16, d8[0]
  .long  0xf2600d20                          // vsub.f32      d16, d0, d16
  .long  0xf2200fa1                          // vmin.f32      d0, d16, d17
  .long  0xecbd8b04                          // vpop          {d8-d9}
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}
  .long  0x3f800000                          // .word         0x3f800000
  .long  0x3f800000                          // .word         0x3f800000

HIDDEN _sk_repeat_y_vfp4
.globl _sk_repeat_y_vfp4
_sk_repeat_y_vfp4:
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xf2c02010                          // vmov.i32      d18, #0
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xeddf3b10                          // vldr          d19, [pc, #64]
  .long  0xed938a00                          // vldr          s16, [r3]
  .long  0xeec19a88                          // vdiv.f32      s19, s3, s16
  .long  0xee819a08                          // vdiv.f32      s18, s2, s16
  .long  0xf3fb0709                          // vcvt.s32.f32  d16, d9
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xf3601e89                          // vcgt.f32      d17, d16, d9
  .long  0xf35311b2                          // vbsl          d17, d19, d18
  .long  0xf3f42c08                          // vdup.32       d18, d8[0]
  .long  0xf2600da1                          // vsub.f32      d16, d16, d17
  .long  0xf3c71e1f                          // vmov.i8       d17, #255
  .long  0xf26218a1                          // vadd.i32      d17, d18, d17
  .long  0xf2e009c8                          // vmul.f32      d16, d16, d8[0]
  .long  0xf2610d20                          // vsub.f32      d16, d1, d16
  .long  0xf2201fa1                          // vmin.f32      d1, d16, d17
  .long  0xecbd8b04                          // vpop          {d8-d9}
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}
  .long  0x3f800000                          // .word         0x3f800000
  .long  0x3f800000                          // .word         0x3f800000

HIDDEN _sk_mirror_x_vfp4
.globl _sk_mirror_x_vfp4
_sk_mirror_x_vfp4:
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xf2c03010                          // vmov.i32      d19, #0
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xeddf4b14                          // vldr          d20, [pc, #80]
  .long  0xed938a00                          // vldr          s16, [r3]
  .long  0xee389a08                          // vadd.f32      s18, s16, s16
  .long  0xf3f40c08                          // vdup.32       d16, d8[0]
  .long  0xf2200d20                          // vsub.f32      d0, d0, d16
  .long  0xeec08a89                          // vdiv.f32      s17, s1, s18
  .long  0xee808a09                          // vdiv.f32      s16, s0, s18
  .long  0xf3fb1708                          // vcvt.s32.f32  d17, d8
  .long  0xf3fb1621                          // vcvt.f32.s32  d17, d17
  .long  0xf3612e88                          // vcgt.f32      d18, d17, d8
  .long  0xf35421b3                          // vbsl          d18, d20, d19
  .long  0xf2611da2                          // vsub.f32      d17, d17, d18
  .long  0xf3c72e1f                          // vmov.i8       d18, #255
  .long  0xf2e119c9                          // vmul.f32      d17, d17, d9[0]
  .long  0xf2601d21                          // vsub.f32      d17, d0, d17
  .long  0xf2611da0                          // vsub.f32      d17, d17, d16
  .long  0xf26008a2                          // vadd.i32      d16, d16, d18
  .long  0xf3f91721                          // vabs.f32      d17, d17
  .long  0xf2210fa0                          // vmin.f32      d0, d17, d16
  .long  0xecbd8b04                          // vpop          {d8-d9}
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}
  .long  0x3f800000                          // .word         0x3f800000
  .long  0x3f800000                          // .word         0x3f800000

HIDDEN _sk_mirror_y_vfp4
.globl _sk_mirror_y_vfp4
_sk_mirror_y_vfp4:
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xf2c03010                          // vmov.i32      d19, #0
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xeddf4b14                          // vldr          d20, [pc, #80]
  .long  0xed938a00                          // vldr          s16, [r3]
  .long  0xee389a08                          // vadd.f32      s18, s16, s16
  .long  0xf3f40c08                          // vdup.32       d16, d8[0]
  .long  0xf2211d20                          // vsub.f32      d1, d1, d16
  .long  0xeec18a89                          // vdiv.f32      s17, s3, s18
  .long  0xee818a09                          // vdiv.f32      s16, s2, s18
  .long  0xf3fb1708                          // vcvt.s32.f32  d17, d8
  .long  0xf3fb1621                          // vcvt.f32.s32  d17, d17
  .long  0xf3612e88                          // vcgt.f32      d18, d17, d8
  .long  0xf35421b3                          // vbsl          d18, d20, d19
  .long  0xf2611da2                          // vsub.f32      d17, d17, d18
  .long  0xf3c72e1f                          // vmov.i8       d18, #255
  .long  0xf2e119c9                          // vmul.f32      d17, d17, d9[0]
  .long  0xf2611d21                          // vsub.f32      d17, d1, d17
  .long  0xf2611da0                          // vsub.f32      d17, d17, d16
  .long  0xf26008a2                          // vadd.i32      d16, d16, d18
  .long  0xf3f91721                          // vabs.f32      d17, d17
  .long  0xf2211fa0                          // vmin.f32      d1, d17, d16
  .long  0xecbd8b04                          // vpop          {d8-d9}
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}
  .long  0x3f800000                          // .word         0x3f800000
  .long  0x3f800000                          // .word         0x3f800000

HIDDEN _sk_luminance_to_alpha_vfp4
.globl _sk_luminance_to_alpha_vfp4
_sk_luminance_to_alpha_vfp4:
  .long  0xeddf0b0a                          // vldr          d16, [pc, #40]
  .long  0xeddf1b0b                          // vldr          d17, [pc, #44]
  .long  0xf3410d30                          // vmul.f32      d16, d1, d16
  .long  0xe4913004                          // ldr           r3, [r1], #4
  .long  0xf3401d31                          // vmul.f32      d17, d0, d17
  .long  0xf2800010                          // vmov.i32      d0, #0
  .long  0xf2801010                          // vmov.i32      d1, #0
  .long  0xf2013da0                          // vadd.f32      d3, d17, d16
  .long  0xeddf0b06                          // vldr          d16, [pc, #24]
  .long  0xf2023c30                          // vfma.f32      d3, d2, d16
  .long  0xf2802010                          // vmov.i32      d2, #0
  .long  0xe12fff13                          // bx            r3
  .long  0x3f371759                          // .word         0x3f371759
  .long  0x3f371759                          // .word         0x3f371759
  .long  0x3e59b3d0                          // .word         0x3e59b3d0
  .long  0x3e59b3d0                          // .word         0x3e59b3d0
  .long  0x3d93dd98                          // .word         0x3d93dd98
  .long  0x3d93dd98                          // .word         0x3d93dd98

HIDDEN _sk_matrix_2x3_vfp4
.globl _sk_matrix_2x3_vfp4
_sk_matrix_2x3_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe28e300c                          // add           r3, lr, #12
  .long  0xf4e32c9f                          // vld1.32       {d18[]}, [r3 :32]
  .long  0xe28e3008                          // add           r3, lr, #8
  .long  0xf4e31c9f                          // vld1.32       {d17[]}, [r3 :32]
  .long  0xe28e3010                          // add           r3, lr, #16
  .long  0xf4e30c9f                          // vld1.32       {d16[]}, [r3 :32]
  .long  0xe28e3014                          // add           r3, lr, #20
  .long  0xf2410c31                          // vfma.f32      d16, d1, d17
  .long  0xf4e31c9f                          // vld1.32       {d17[]}, [r3 :32]
  .long  0xf2411c32                          // vfma.f32      d17, d1, d18
  .long  0xf4ee2c9d                          // vld1.32       {d18[]}, [lr :32]!
  .long  0xf4ee3c9f                          // vld1.32       {d19[]}, [lr :32]
  .long  0xf2400c32                          // vfma.f32      d16, d0, d18
  .long  0xf2401c33                          // vfma.f32      d17, d0, d19
  .long  0xf22001b0                          // vorr          d0, d16, d16
  .long  0xf22111b1                          // vorr          d1, d17, d17
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

HIDDEN _sk_matrix_3x4_vfp4
.globl _sk_matrix_3x4_vfp4
_sk_matrix_3x4_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe28e3020                          // add           r3, lr, #32
  .long  0xf4e33c9f                          // vld1.32       {d19[]}, [r3 :32]
  .long  0xe28e302c                          // add           r3, lr, #44
  .long  0xf4e30c9f                          // vld1.32       {d16[]}, [r3 :32]
  .long  0xe28e301c                          // add           r3, lr, #28
  .long  0xf2420c33                          // vfma.f32      d16, d2, d19
  .long  0xf4e34c9f                          // vld1.32       {d20[]}, [r3 :32]
  .long  0xe28e3018                          // add           r3, lr, #24
  .long  0xf4e32c9f                          // vld1.32       {d18[]}, [r3 :32]
  .long  0xe28e3024                          // add           r3, lr, #36
  .long  0xf4e31c9f                          // vld1.32       {d17[]}, [r3 :32]
  .long  0xe28e3028                          // add           r3, lr, #40
  .long  0xf2421c32                          // vfma.f32      d17, d2, d18
  .long  0xf4e32c9f                          // vld1.32       {d18[]}, [r3 :32]
  .long  0xe28e3010                          // add           r3, lr, #16
  .long  0xf2422c34                          // vfma.f32      d18, d2, d20
  .long  0xf4e33c9f                          // vld1.32       {d19[]}, [r3 :32]
  .long  0xe28e300c                          // add           r3, lr, #12
  .long  0xf4e34c9f                          // vld1.32       {d20[]}, [r3 :32]
  .long  0xe28e3014                          // add           r3, lr, #20
  .long  0xf2411c34                          // vfma.f32      d17, d1, d20
  .long  0xf4e34c9f                          // vld1.32       {d20[]}, [r3 :32]
  .long  0xf2410c34                          // vfma.f32      d16, d1, d20
  .long  0xe28e3008                          // add           r3, lr, #8
  .long  0xf2412c33                          // vfma.f32      d18, d1, d19
  .long  0xf4ee3c9d                          // vld1.32       {d19[]}, [lr :32]!
  .long  0xf4ee4c9f                          // vld1.32       {d20[]}, [lr :32]
  .long  0xf2401c33                          // vfma.f32      d17, d0, d19
  .long  0xf4e33c9f                          // vld1.32       {d19[]}, [r3 :32]
  .long  0xf2400c33                          // vfma.f32      d16, d0, d19
  .long  0xf2402c34                          // vfma.f32      d18, d0, d20
  .long  0xf22101b1                          // vorr          d0, d17, d17
  .long  0xf22021b0                          // vorr          d2, d16, d16
  .long  0xf22211b2                          // vorr          d1, d18, d18
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

HIDDEN _sk_matrix_4x5_vfp4
.globl _sk_matrix_4x5_vfp4
_sk_matrix_4x5_vfp4:
  .long  0xe92d4010                          // push          {r4, lr}
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xf2620112                          // vorr          d16, d2, d2
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe2834014                          // add           r4, r3, #20
  .long  0xe1a0e003                          // mov           lr, r3
  .long  0xf4e45c9f                          // vld1.32       {d21[]}, [r4 :32]
  .long  0xe2834028                          // add           r4, r3, #40
  .long  0xf4e46c9f                          // vld1.32       {d22[]}, [r4 :32]
  .long  0xe2834038                          // add           r4, r3, #56
  .long  0xf4e47c9f                          // vld1.32       {d23[]}, [r4 :32]
  .long  0xe2834048                          // add           r4, r3, #72
  .long  0xf4a42c9f                          // vld1.32       {d2[]}, [r4 :32]
  .long  0xe2834034                          // add           r4, r3, #52
  .long  0xf2032c37                          // vfma.f32      d2, d3, d23
  .long  0xf4e48c9f                          // vld1.32       {d24[]}, [r4 :32]
  .long  0xe2834044                          // add           r4, r3, #68
  .long  0xf4e41c9f                          // vld1.32       {d17[]}, [r4 :32]
  .long  0xe2834030                          // add           r4, r3, #48
  .long  0xf2431c38                          // vfma.f32      d17, d3, d24
  .long  0xf4e49c9f                          // vld1.32       {d25[]}, [r4 :32]
  .long  0xe283403c                          // add           r4, r3, #60
  .long  0xf4e43c9f                          // vld1.32       {d19[]}, [r4 :32]
  .long  0xe283404c                          // add           r4, r3, #76
  .long  0xf2002cb6                          // vfma.f32      d2, d16, d22
  .long  0xf4e42c9f                          // vld1.32       {d18[]}, [r4 :32]
  .long  0xe2834040                          // add           r4, r3, #64
  .long  0xf2432c33                          // vfma.f32      d18, d3, d19
  .long  0xf4e43c9f                          // vld1.32       {d19[]}, [r4 :32]
  .long  0xe2834020                          // add           r4, r3, #32
  .long  0xf2433c39                          // vfma.f32      d19, d3, d25
  .long  0xf4e47c9f                          // vld1.32       {d23[]}, [r4 :32]
  .long  0xe283402c                          // add           r4, r3, #44
  .long  0xf4e48c9f                          // vld1.32       {d24[]}, [r4 :32]
  .long  0xe2834024                          // add           r4, r3, #36
  .long  0xf2402cb8                          // vfma.f32      d18, d16, d24
  .long  0xf4e48c9f                          // vld1.32       {d24[]}, [r4 :32]
  .long  0xf2401cb8                          // vfma.f32      d17, d16, d24
  .long  0xe2834010                          // add           r4, r3, #16
  .long  0xf2403cb7                          // vfma.f32      d19, d16, d23
  .long  0xf4ee4c9d                          // vld1.32       {d20[]}, [lr :32]!
  .long  0xf4e40c9f                          // vld1.32       {d16[]}, [r4 :32]
  .long  0xe283401c                          // add           r4, r3, #28
  .long  0xf4e46c9f                          // vld1.32       {d22[]}, [r4 :32]
  .long  0xe2834018                          // add           r4, r3, #24
  .long  0xf2412c36                          // vfma.f32      d18, d1, d22
  .long  0xf2411c35                          // vfma.f32      d17, d1, d21
  .long  0xf4ee5c9f                          // vld1.32       {d21[]}, [lr :32]
  .long  0xf2413c30                          // vfma.f32      d19, d1, d16
  .long  0xf4e40c9f                          // vld1.32       {d16[]}, [r4 :32]
  .long  0xe2834008                          // add           r4, r3, #8
  .long  0xe283300c                          // add           r3, r3, #12
  .long  0xf2012c30                          // vfma.f32      d2, d1, d16
  .long  0xf4e40c9f                          // vld1.32       {d16[]}, [r4 :32]
  .long  0xf2401c35                          // vfma.f32      d17, d0, d21
  .long  0xf2403c34                          // vfma.f32      d19, d0, d20
  .long  0xf4e34c9f                          // vld1.32       {d20[]}, [r3 :32]
  .long  0xf2402c34                          // vfma.f32      d18, d0, d20
  .long  0xf2002c30                          // vfma.f32      d2, d0, d16
  .long  0xf22111b1                          // vorr          d1, d17, d17
  .long  0xf22301b3                          // vorr          d0, d19, d19
  .long  0xf22231b2                          // vorr          d3, d18, d18
  .long  0xe8bd4010                          // pop           {r4, lr}
  .long  0xe12fff1c                          // bx            ip

HIDDEN _sk_matrix_perspective_vfp4
.globl _sk_matrix_perspective_vfp4
_sk_matrix_perspective_vfp4:
  .long  0xe92d4010                          // push          {r4, lr}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe28e301c                          // add           r3, lr, #28
  .long  0xe28e4010                          // add           r4, lr, #16
  .long  0xf4e30c9f                          // vld1.32       {d16[]}, [r3 :32]
  .long  0xe28e3020                          // add           r3, lr, #32
  .long  0xf4e31c9f                          // vld1.32       {d17[]}, [r3 :32]
  .long  0xe28e3018                          // add           r3, lr, #24
  .long  0xf2411c30                          // vfma.f32      d17, d1, d16
  .long  0xf4e30c9f                          // vld1.32       {d16[]}, [r3 :32]
  .long  0xe1a0300e                          // mov           r3, lr
  .long  0xf4e42c9f                          // vld1.32       {d18[]}, [r4 :32]
  .long  0xe28e4008                          // add           r4, lr, #8
  .long  0xf4e43c9f                          // vld1.32       {d19[]}, [r4 :32]
  .long  0xf2401c30                          // vfma.f32      d17, d0, d16
  .long  0xf4e30c9d                          // vld1.32       {d16[]}, [r3 :32]!
  .long  0xf4e35c9f                          // vld1.32       {d21[]}, [r3 :32]
  .long  0xe28e3014                          // add           r3, lr, #20
  .long  0xf2413c35                          // vfma.f32      d19, d1, d21
  .long  0xf4e35c9f                          // vld1.32       {d21[]}, [r3 :32]
  .long  0xe28e300c                          // add           r3, lr, #12
  .long  0xf2415c32                          // vfma.f32      d21, d1, d18
  .long  0xf4e32c9f                          // vld1.32       {d18[]}, [r3 :32]
  .long  0xf3fb4521                          // vrecpe.f32    d20, d17
  .long  0xf2403c30                          // vfma.f32      d19, d0, d16
  .long  0xf2411fb4                          // vrecps.f32    d17, d17, d20
  .long  0xf2405c32                          // vfma.f32      d21, d0, d18
  .long  0xf3440db1                          // vmul.f32      d16, d20, d17
  .long  0xf3030db0                          // vmul.f32      d0, d19, d16
  .long  0xf3051db0                          // vmul.f32      d1, d21, d16
  .long  0xe8bd4010                          // pop           {r4, lr}
  .long  0xe12fff1c                          // bx            ip

HIDDEN _sk_linear_gradient_2stops_vfp4
.globl _sk_linear_gradient_2stops_vfp4
_sk_linear_gradient_2stops_vfp4:
  .long  0xe8911008                          // ldm           r1, {r3, ip}
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf4632a0d                          // vld1.8        {d18-d19}, [r3]!
  .long  0xf4634a0f                          // vld1.8        {d20-d21}, [r3]
  .long  0xf3f40c22                          // vdup.32       d16, d18[0]
  .long  0xf3f41c24                          // vdup.32       d17, d20[0]
  .long  0xf2400c31                          // vfma.f32      d16, d0, d17
  .long  0xf3fc6c24                          // vdup.32       d22, d20[1]
  .long  0xf3bc1c22                          // vdup.32       d1, d18[1]
  .long  0xf3b42c23                          // vdup.32       d2, d19[0]
  .long  0xf2001c36                          // vfma.f32      d1, d0, d22
  .long  0xf3f41c25                          // vdup.32       d17, d21[0]
  .long  0xf3fc4c25                          // vdup.32       d20, d21[1]
  .long  0xf2002c31                          // vfma.f32      d2, d0, d17
  .long  0xf3bc3c23                          // vdup.32       d3, d19[1]
  .long  0xf2003c34                          // vfma.f32      d3, d0, d20
  .long  0xf22001b0                          // vorr          d0, d16, d16
  .long  0xe12fff1c                          // bx            ip
#elif defined(__x86_64__)

HIDDEN _sk_start_pipeline_hsw
.globl _sk_start_pipeline_hsw
_sk_start_pipeline_hsw:
  .byte  65,87                               // push          %r15
  .byte  65,86                               // push          %r14
  .byte  65,85                               // push          %r13
  .byte  65,84                               // push          %r12
  .byte  83                                  // push          %rbx
  .byte  73,137,205                          // mov           %rcx,%r13
  .byte  73,137,214                          // mov           %rdx,%r14
  .byte  72,137,251                          // mov           %rdi,%rbx
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  73,137,199                          // mov           %rax,%r15
  .byte  73,137,244                          // mov           %rsi,%r12
  .byte  72,141,67,8                         // lea           0x8(%rbx),%rax
  .byte  76,57,232                           // cmp           %r13,%rax
  .byte  118,5                               // jbe           28 <_sk_start_pipeline_hsw+0x28>
  .byte  72,137,223                          // mov           %rbx,%rdi
  .byte  235,65                              // jmp           69 <_sk_start_pipeline_hsw+0x69>
  .byte  185,0,0,0,0                         // mov           $0x0,%ecx
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  197,228,87,219                      // vxorps        %ymm3,%ymm3,%ymm3
  .byte  197,220,87,228                      // vxorps        %ymm4,%ymm4,%ymm4
  .byte  197,212,87,237                      // vxorps        %ymm5,%ymm5,%ymm5
  .byte  197,204,87,246                      // vxorps        %ymm6,%ymm6,%ymm6
  .byte  197,196,87,255                      // vxorps        %ymm7,%ymm7,%ymm7
  .byte  72,137,223                          // mov           %rbx,%rdi
  .byte  76,137,230                          // mov           %r12,%rsi
  .byte  76,137,242                          // mov           %r14,%rdx
  .byte  65,255,215                          // callq         *%r15
  .byte  72,141,123,8                        // lea           0x8(%rbx),%rdi
  .byte  72,131,195,16                       // add           $0x10,%rbx
  .byte  76,57,235                           // cmp           %r13,%rbx
  .byte  72,137,251                          // mov           %rdi,%rbx
  .byte  118,191                             // jbe           28 <_sk_start_pipeline_hsw+0x28>
  .byte  76,137,233                          // mov           %r13,%rcx
  .byte  72,41,249                           // sub           %rdi,%rcx
  .byte  116,41                              // je            9a <_sk_start_pipeline_hsw+0x9a>
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  197,228,87,219                      // vxorps        %ymm3,%ymm3,%ymm3
  .byte  197,220,87,228                      // vxorps        %ymm4,%ymm4,%ymm4
  .byte  197,212,87,237                      // vxorps        %ymm5,%ymm5,%ymm5
  .byte  197,204,87,246                      // vxorps        %ymm6,%ymm6,%ymm6
  .byte  197,196,87,255                      // vxorps        %ymm7,%ymm7,%ymm7
  .byte  76,137,230                          // mov           %r12,%rsi
  .byte  76,137,242                          // mov           %r14,%rdx
  .byte  65,255,215                          // callq         *%r15
  .byte  76,137,232                          // mov           %r13,%rax
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,93                               // pop           %r13
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  197,248,119                         // vzeroupper
  .byte  195                                 // retq

HIDDEN _sk_just_return_hsw
.globl _sk_just_return_hsw
_sk_just_return_hsw:
  .byte  195                                 // retq

HIDDEN _sk_seed_shader_hsw
.globl _sk_seed_shader_hsw
_sk_seed_shader_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,249,110,199                     // vmovd         %edi,%xmm0
  .byte  196,226,125,88,192                  // vpbroadcastd  %xmm0,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  65,184,0,0,0,63                     // mov           $0x3f000000,%r8d
  .byte  196,193,121,110,200                 // vmovd         %r8d,%xmm1
  .byte  196,226,125,88,201                  // vpbroadcastd  %xmm1,%ymm1
  .byte  197,252,88,193                      // vaddps        %ymm1,%ymm0,%ymm0
  .byte  197,252,88,2                        // vaddps        (%rdx),%ymm0,%ymm0
  .byte  196,226,125,24,16                   // vbroadcastss  (%rax),%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  197,236,88,201                      // vaddps        %ymm1,%ymm2,%ymm1
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,249,110,208                     // vmovd         %eax,%xmm2
  .byte  196,226,125,88,210                  // vpbroadcastd  %xmm2,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,228,87,219                      // vxorps        %ymm3,%ymm3,%ymm3
  .byte  197,220,87,228                      // vxorps        %ymm4,%ymm4,%ymm4
  .byte  197,212,87,237                      // vxorps        %ymm5,%ymm5,%ymm5
  .byte  197,204,87,246                      // vxorps        %ymm6,%ymm6,%ymm6
  .byte  197,196,87,255                      // vxorps        %ymm7,%ymm7,%ymm7
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_constant_color_hsw
.globl _sk_constant_color_hsw
_sk_constant_color_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,226,125,24,0                    // vbroadcastss  (%rax),%ymm0
  .byte  196,226,125,24,72,4                 // vbroadcastss  0x4(%rax),%ymm1
  .byte  196,226,125,24,80,8                 // vbroadcastss  0x8(%rax),%ymm2
  .byte  196,226,125,24,88,12                // vbroadcastss  0xc(%rax),%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clear_hsw
.globl _sk_clear_hsw
_sk_clear_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  197,228,87,219                      // vxorps        %ymm3,%ymm3,%ymm3
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_srcatop_hsw
.globl _sk_srcatop_hsw
_sk_srcatop_hsw:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,204                       // vmulps        %ymm4,%ymm8,%ymm9
  .byte  196,194,69,168,193                  // vfmadd213ps   %ymm9,%ymm7,%ymm0
  .byte  197,60,89,205                       // vmulps        %ymm5,%ymm8,%ymm9
  .byte  196,194,69,168,201                  // vfmadd213ps   %ymm9,%ymm7,%ymm1
  .byte  197,60,89,206                       // vmulps        %ymm6,%ymm8,%ymm9
  .byte  196,194,69,168,209                  // vfmadd213ps   %ymm9,%ymm7,%ymm2
  .byte  197,60,89,199                       // vmulps        %ymm7,%ymm8,%ymm8
  .byte  196,194,69,168,216                  // vfmadd213ps   %ymm8,%ymm7,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_dstatop_hsw
.globl _sk_dstatop_hsw
_sk_dstatop_hsw:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,92,199                       // vsubps        %ymm7,%ymm8,%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  196,226,101,184,196                 // vfmadd231ps   %ymm4,%ymm3,%ymm0
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  196,226,101,184,205                 // vfmadd231ps   %ymm5,%ymm3,%ymm1
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  196,226,101,184,214                 // vfmadd231ps   %ymm6,%ymm3,%ymm2
  .byte  197,60,89,195                       // vmulps        %ymm3,%ymm8,%ymm8
  .byte  196,194,69,168,216                  // vfmadd213ps   %ymm8,%ymm7,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_srcin_hsw
.globl _sk_srcin_hsw
_sk_srcin_hsw:
  .byte  197,252,89,199                      // vmulps        %ymm7,%ymm0,%ymm0
  .byte  197,244,89,207                      // vmulps        %ymm7,%ymm1,%ymm1
  .byte  197,236,89,215                      // vmulps        %ymm7,%ymm2,%ymm2
  .byte  197,228,89,223                      // vmulps        %ymm7,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_dstin_hsw
.globl _sk_dstin_hsw
_sk_dstin_hsw:
  .byte  197,228,89,196                      // vmulps        %ymm4,%ymm3,%ymm0
  .byte  197,228,89,205                      // vmulps        %ymm5,%ymm3,%ymm1
  .byte  197,228,89,214                      // vmulps        %ymm6,%ymm3,%ymm2
  .byte  197,228,89,223                      // vmulps        %ymm7,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_srcout_hsw
.globl _sk_srcout_hsw
_sk_srcout_hsw:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,92,199                       // vsubps        %ymm7,%ymm8,%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_dstout_hsw
.globl _sk_dstout_hsw
_sk_dstout_hsw:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,249,110,192                     // vmovd         %eax,%xmm0
  .byte  196,226,125,88,192                  // vpbroadcastd  %xmm0,%ymm0
  .byte  197,252,92,219                      // vsubps        %ymm3,%ymm0,%ymm3
  .byte  197,228,89,196                      // vmulps        %ymm4,%ymm3,%ymm0
  .byte  197,228,89,205                      // vmulps        %ymm5,%ymm3,%ymm1
  .byte  197,228,89,214                      // vmulps        %ymm6,%ymm3,%ymm2
  .byte  197,228,89,223                      // vmulps        %ymm7,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_srcover_hsw
.globl _sk_srcover_hsw
_sk_srcover_hsw:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  196,194,93,184,192                  // vfmadd231ps   %ymm8,%ymm4,%ymm0
  .byte  196,194,85,184,200                  // vfmadd231ps   %ymm8,%ymm5,%ymm1
  .byte  196,194,77,184,208                  // vfmadd231ps   %ymm8,%ymm6,%ymm2
  .byte  196,194,69,184,216                  // vfmadd231ps   %ymm8,%ymm7,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_dstover_hsw
.globl _sk_dstover_hsw
_sk_dstover_hsw:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,92,199                       // vsubps        %ymm7,%ymm8,%ymm8
  .byte  196,226,61,168,196                  // vfmadd213ps   %ymm4,%ymm8,%ymm0
  .byte  196,226,61,168,205                  // vfmadd213ps   %ymm5,%ymm8,%ymm1
  .byte  196,226,61,168,214                  // vfmadd213ps   %ymm6,%ymm8,%ymm2
  .byte  196,226,61,168,223                  // vfmadd213ps   %ymm7,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_modulate_hsw
.globl _sk_modulate_hsw
_sk_modulate_hsw:
  .byte  197,252,89,196                      // vmulps        %ymm4,%ymm0,%ymm0
  .byte  197,244,89,205                      // vmulps        %ymm5,%ymm1,%ymm1
  .byte  197,236,89,214                      // vmulps        %ymm6,%ymm2,%ymm2
  .byte  197,228,89,223                      // vmulps        %ymm7,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_multiply_hsw
.globl _sk_multiply_hsw
_sk_multiply_hsw:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,92,207                       // vsubps        %ymm7,%ymm8,%ymm9
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,212                       // vmulps        %ymm4,%ymm8,%ymm10
  .byte  196,98,53,184,208                   // vfmadd231ps   %ymm0,%ymm9,%ymm10
  .byte  196,194,93,168,194                  // vfmadd213ps   %ymm10,%ymm4,%ymm0
  .byte  197,52,89,209                       // vmulps        %ymm1,%ymm9,%ymm10
  .byte  196,98,61,184,213                   // vfmadd231ps   %ymm5,%ymm8,%ymm10
  .byte  196,194,85,168,202                  // vfmadd213ps   %ymm10,%ymm5,%ymm1
  .byte  197,52,89,210                       // vmulps        %ymm2,%ymm9,%ymm10
  .byte  196,98,61,184,214                   // vfmadd231ps   %ymm6,%ymm8,%ymm10
  .byte  196,194,77,168,210                  // vfmadd213ps   %ymm10,%ymm6,%ymm2
  .byte  197,52,89,203                       // vmulps        %ymm3,%ymm9,%ymm9
  .byte  196,66,69,168,193                   // vfmadd213ps   %ymm9,%ymm7,%ymm8
  .byte  196,194,69,168,216                  // vfmadd213ps   %ymm8,%ymm7,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_plus__hsw
.globl _sk_plus__hsw
_sk_plus__hsw:
  .byte  197,252,88,196                      // vaddps        %ymm4,%ymm0,%ymm0
  .byte  197,244,88,205                      // vaddps        %ymm5,%ymm1,%ymm1
  .byte  197,236,88,214                      // vaddps        %ymm6,%ymm2,%ymm2
  .byte  197,228,88,223                      // vaddps        %ymm7,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_screen_hsw
.globl _sk_screen_hsw
_sk_screen_hsw:
  .byte  197,124,88,196                      // vaddps        %ymm4,%ymm0,%ymm8
  .byte  196,194,93,172,192                  // vfnmadd213ps  %ymm8,%ymm4,%ymm0
  .byte  197,116,88,197                      // vaddps        %ymm5,%ymm1,%ymm8
  .byte  196,194,85,172,200                  // vfnmadd213ps  %ymm8,%ymm5,%ymm1
  .byte  197,108,88,198                      // vaddps        %ymm6,%ymm2,%ymm8
  .byte  196,194,77,172,208                  // vfnmadd213ps  %ymm8,%ymm6,%ymm2
  .byte  197,100,88,199                      // vaddps        %ymm7,%ymm3,%ymm8
  .byte  196,194,69,172,216                  // vfnmadd213ps  %ymm8,%ymm7,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_xor__hsw
.globl _sk_xor__hsw
_sk_xor__hsw:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,92,207                       // vsubps        %ymm7,%ymm8,%ymm9
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,212                       // vmulps        %ymm4,%ymm8,%ymm10
  .byte  196,194,53,168,194                  // vfmadd213ps   %ymm10,%ymm9,%ymm0
  .byte  197,180,89,201                      // vmulps        %ymm1,%ymm9,%ymm1
  .byte  196,226,61,184,205                  // vfmadd231ps   %ymm5,%ymm8,%ymm1
  .byte  197,180,89,210                      // vmulps        %ymm2,%ymm9,%ymm2
  .byte  196,226,61,184,214                  // vfmadd231ps   %ymm6,%ymm8,%ymm2
  .byte  197,180,89,219                      // vmulps        %ymm3,%ymm9,%ymm3
  .byte  196,98,69,168,195                   // vfmadd213ps   %ymm3,%ymm7,%ymm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,124,41,195                      // vmovaps       %ymm8,%ymm3
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_darken_hsw
.globl _sk_darken_hsw
_sk_darken_hsw:
  .byte  197,124,88,196                      // vaddps        %ymm4,%ymm0,%ymm8
  .byte  197,252,89,199                      // vmulps        %ymm7,%ymm0,%ymm0
  .byte  197,100,89,204                      // vmulps        %ymm4,%ymm3,%ymm9
  .byte  196,193,124,95,193                  // vmaxps        %ymm9,%ymm0,%ymm0
  .byte  197,188,92,192                      // vsubps        %ymm0,%ymm8,%ymm0
  .byte  197,116,88,197                      // vaddps        %ymm5,%ymm1,%ymm8
  .byte  197,244,89,207                      // vmulps        %ymm7,%ymm1,%ymm1
  .byte  197,100,89,205                      // vmulps        %ymm5,%ymm3,%ymm9
  .byte  196,193,116,95,201                  // vmaxps        %ymm9,%ymm1,%ymm1
  .byte  197,188,92,201                      // vsubps        %ymm1,%ymm8,%ymm1
  .byte  197,108,88,198                      // vaddps        %ymm6,%ymm2,%ymm8
  .byte  197,236,89,215                      // vmulps        %ymm7,%ymm2,%ymm2
  .byte  197,100,89,206                      // vmulps        %ymm6,%ymm3,%ymm9
  .byte  196,193,108,95,209                  // vmaxps        %ymm9,%ymm2,%ymm2
  .byte  197,188,92,210                      // vsubps        %ymm2,%ymm8,%ymm2
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  196,194,69,184,216                  // vfmadd231ps   %ymm8,%ymm7,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_lighten_hsw
.globl _sk_lighten_hsw
_sk_lighten_hsw:
  .byte  197,124,88,196                      // vaddps        %ymm4,%ymm0,%ymm8
  .byte  197,252,89,199                      // vmulps        %ymm7,%ymm0,%ymm0
  .byte  197,100,89,204                      // vmulps        %ymm4,%ymm3,%ymm9
  .byte  196,193,124,93,193                  // vminps        %ymm9,%ymm0,%ymm0
  .byte  197,188,92,192                      // vsubps        %ymm0,%ymm8,%ymm0
  .byte  197,116,88,197                      // vaddps        %ymm5,%ymm1,%ymm8
  .byte  197,244,89,207                      // vmulps        %ymm7,%ymm1,%ymm1
  .byte  197,100,89,205                      // vmulps        %ymm5,%ymm3,%ymm9
  .byte  196,193,116,93,201                  // vminps        %ymm9,%ymm1,%ymm1
  .byte  197,188,92,201                      // vsubps        %ymm1,%ymm8,%ymm1
  .byte  197,108,88,198                      // vaddps        %ymm6,%ymm2,%ymm8
  .byte  197,236,89,215                      // vmulps        %ymm7,%ymm2,%ymm2
  .byte  197,100,89,206                      // vmulps        %ymm6,%ymm3,%ymm9
  .byte  196,193,108,93,209                  // vminps        %ymm9,%ymm2,%ymm2
  .byte  197,188,92,210                      // vsubps        %ymm2,%ymm8,%ymm2
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  196,194,69,184,216                  // vfmadd231ps   %ymm8,%ymm7,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_difference_hsw
.globl _sk_difference_hsw
_sk_difference_hsw:
  .byte  197,124,88,196                      // vaddps        %ymm4,%ymm0,%ymm8
  .byte  197,252,89,199                      // vmulps        %ymm7,%ymm0,%ymm0
  .byte  197,100,89,204                      // vmulps        %ymm4,%ymm3,%ymm9
  .byte  196,193,124,93,193                  // vminps        %ymm9,%ymm0,%ymm0
  .byte  197,252,88,192                      // vaddps        %ymm0,%ymm0,%ymm0
  .byte  197,188,92,192                      // vsubps        %ymm0,%ymm8,%ymm0
  .byte  197,116,88,197                      // vaddps        %ymm5,%ymm1,%ymm8
  .byte  197,244,89,207                      // vmulps        %ymm7,%ymm1,%ymm1
  .byte  197,100,89,205                      // vmulps        %ymm5,%ymm3,%ymm9
  .byte  196,193,116,93,201                  // vminps        %ymm9,%ymm1,%ymm1
  .byte  197,244,88,201                      // vaddps        %ymm1,%ymm1,%ymm1
  .byte  197,188,92,201                      // vsubps        %ymm1,%ymm8,%ymm1
  .byte  197,108,88,198                      // vaddps        %ymm6,%ymm2,%ymm8
  .byte  197,236,89,215                      // vmulps        %ymm7,%ymm2,%ymm2
  .byte  197,100,89,206                      // vmulps        %ymm6,%ymm3,%ymm9
  .byte  196,193,108,93,209                  // vminps        %ymm9,%ymm2,%ymm2
  .byte  197,236,88,210                      // vaddps        %ymm2,%ymm2,%ymm2
  .byte  197,188,92,210                      // vsubps        %ymm2,%ymm8,%ymm2
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  196,194,69,184,216                  // vfmadd231ps   %ymm8,%ymm7,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_exclusion_hsw
.globl _sk_exclusion_hsw
_sk_exclusion_hsw:
  .byte  197,124,88,196                      // vaddps        %ymm4,%ymm0,%ymm8
  .byte  197,252,89,196                      // vmulps        %ymm4,%ymm0,%ymm0
  .byte  197,252,88,192                      // vaddps        %ymm0,%ymm0,%ymm0
  .byte  197,188,92,192                      // vsubps        %ymm0,%ymm8,%ymm0
  .byte  197,116,88,197                      // vaddps        %ymm5,%ymm1,%ymm8
  .byte  197,244,89,205                      // vmulps        %ymm5,%ymm1,%ymm1
  .byte  197,244,88,201                      // vaddps        %ymm1,%ymm1,%ymm1
  .byte  197,188,92,201                      // vsubps        %ymm1,%ymm8,%ymm1
  .byte  197,108,88,198                      // vaddps        %ymm6,%ymm2,%ymm8
  .byte  197,236,89,214                      // vmulps        %ymm6,%ymm2,%ymm2
  .byte  197,236,88,210                      // vaddps        %ymm2,%ymm2,%ymm2
  .byte  197,188,92,210                      // vsubps        %ymm2,%ymm8,%ymm2
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  196,194,69,184,216                  // vfmadd231ps   %ymm8,%ymm7,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_colorburn_hsw
.globl _sk_colorburn_hsw
_sk_colorburn_hsw:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,92,207                       // vsubps        %ymm7,%ymm8,%ymm9
  .byte  197,52,89,216                       // vmulps        %ymm0,%ymm9,%ymm11
  .byte  196,65,44,87,210                    // vxorps        %ymm10,%ymm10,%ymm10
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,228                       // vmulps        %ymm4,%ymm8,%ymm12
  .byte  197,68,92,236                       // vsubps        %ymm4,%ymm7,%ymm13
  .byte  197,20,89,235                       // vmulps        %ymm3,%ymm13,%ymm13
  .byte  197,20,94,232                       // vdivps        %ymm0,%ymm13,%ymm13
  .byte  196,65,68,93,237                    // vminps        %ymm13,%ymm7,%ymm13
  .byte  196,65,68,92,237                    // vsubps        %ymm13,%ymm7,%ymm13
  .byte  196,66,101,168,235                  // vfmadd213ps   %ymm11,%ymm3,%ymm13
  .byte  196,65,28,88,237                    // vaddps        %ymm13,%ymm12,%ymm13
  .byte  197,28,88,224                       // vaddps        %ymm0,%ymm12,%ymm12
  .byte  196,193,124,194,194,0               // vcmpeqps      %ymm10,%ymm0,%ymm0
  .byte  196,195,21,74,196,0                 // vblendvps     %ymm0,%ymm12,%ymm13,%ymm0
  .byte  197,92,194,231,0                    // vcmpeqps      %ymm7,%ymm4,%ymm12
  .byte  197,36,88,220                       // vaddps        %ymm4,%ymm11,%ymm11
  .byte  196,195,125,74,195,192              // vblendvps     %ymm12,%ymm11,%ymm0,%ymm0
  .byte  197,52,89,217                       // vmulps        %ymm1,%ymm9,%ymm11
  .byte  197,60,89,229                       // vmulps        %ymm5,%ymm8,%ymm12
  .byte  197,68,92,237                       // vsubps        %ymm5,%ymm7,%ymm13
  .byte  197,20,89,235                       // vmulps        %ymm3,%ymm13,%ymm13
  .byte  197,20,94,233                       // vdivps        %ymm1,%ymm13,%ymm13
  .byte  196,65,68,93,237                    // vminps        %ymm13,%ymm7,%ymm13
  .byte  196,65,68,92,237                    // vsubps        %ymm13,%ymm7,%ymm13
  .byte  196,66,101,168,235                  // vfmadd213ps   %ymm11,%ymm3,%ymm13
  .byte  196,65,28,88,237                    // vaddps        %ymm13,%ymm12,%ymm13
  .byte  197,28,88,225                       // vaddps        %ymm1,%ymm12,%ymm12
  .byte  196,193,116,194,202,0               // vcmpeqps      %ymm10,%ymm1,%ymm1
  .byte  196,195,21,74,204,16                // vblendvps     %ymm1,%ymm12,%ymm13,%ymm1
  .byte  197,84,194,231,0                    // vcmpeqps      %ymm7,%ymm5,%ymm12
  .byte  197,36,88,221                       // vaddps        %ymm5,%ymm11,%ymm11
  .byte  196,195,117,74,203,192              // vblendvps     %ymm12,%ymm11,%ymm1,%ymm1
  .byte  197,52,89,202                       // vmulps        %ymm2,%ymm9,%ymm9
  .byte  196,65,108,194,210,0                // vcmpeqps      %ymm10,%ymm2,%ymm10
  .byte  197,60,89,222                       // vmulps        %ymm6,%ymm8,%ymm11
  .byte  197,68,92,230                       // vsubps        %ymm6,%ymm7,%ymm12
  .byte  197,28,89,227                       // vmulps        %ymm3,%ymm12,%ymm12
  .byte  197,28,94,226                       // vdivps        %ymm2,%ymm12,%ymm12
  .byte  197,164,88,210                      // vaddps        %ymm2,%ymm11,%ymm2
  .byte  196,65,68,93,228                    // vminps        %ymm12,%ymm7,%ymm12
  .byte  196,65,68,92,228                    // vsubps        %ymm12,%ymm7,%ymm12
  .byte  196,66,101,168,225                  // vfmadd213ps   %ymm9,%ymm3,%ymm12
  .byte  196,65,36,88,220                    // vaddps        %ymm12,%ymm11,%ymm11
  .byte  196,227,37,74,210,160               // vblendvps     %ymm10,%ymm2,%ymm11,%ymm2
  .byte  197,76,194,215,0                    // vcmpeqps      %ymm7,%ymm6,%ymm10
  .byte  197,52,88,206                       // vaddps        %ymm6,%ymm9,%ymm9
  .byte  196,195,109,74,209,160              // vblendvps     %ymm10,%ymm9,%ymm2,%ymm2
  .byte  196,194,69,184,216                  // vfmadd231ps   %ymm8,%ymm7,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_colordodge_hsw
.globl _sk_colordodge_hsw
_sk_colordodge_hsw:
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,200                     // vmovd         %eax,%xmm9
  .byte  196,66,125,88,201                   // vpbroadcastd  %xmm9,%ymm9
  .byte  197,52,92,215                       // vsubps        %ymm7,%ymm9,%ymm10
  .byte  197,44,89,216                       // vmulps        %ymm0,%ymm10,%ymm11
  .byte  197,52,92,203                       // vsubps        %ymm3,%ymm9,%ymm9
  .byte  197,100,89,228                      // vmulps        %ymm4,%ymm3,%ymm12
  .byte  197,100,92,232                      // vsubps        %ymm0,%ymm3,%ymm13
  .byte  196,65,28,94,229                    // vdivps        %ymm13,%ymm12,%ymm12
  .byte  197,52,89,236                       // vmulps        %ymm4,%ymm9,%ymm13
  .byte  196,65,68,93,228                    // vminps        %ymm12,%ymm7,%ymm12
  .byte  196,66,101,168,227                  // vfmadd213ps   %ymm11,%ymm3,%ymm12
  .byte  196,65,20,88,228                    // vaddps        %ymm12,%ymm13,%ymm12
  .byte  197,20,88,232                       // vaddps        %ymm0,%ymm13,%ymm13
  .byte  197,252,194,195,0                   // vcmpeqps      %ymm3,%ymm0,%ymm0
  .byte  196,195,29,74,197,0                 // vblendvps     %ymm0,%ymm13,%ymm12,%ymm0
  .byte  196,65,92,194,224,0                 // vcmpeqps      %ymm8,%ymm4,%ymm12
  .byte  197,36,88,220                       // vaddps        %ymm4,%ymm11,%ymm11
  .byte  196,195,125,74,195,192              // vblendvps     %ymm12,%ymm11,%ymm0,%ymm0
  .byte  197,44,89,217                       // vmulps        %ymm1,%ymm10,%ymm11
  .byte  197,100,89,229                      // vmulps        %ymm5,%ymm3,%ymm12
  .byte  197,100,92,233                      // vsubps        %ymm1,%ymm3,%ymm13
  .byte  196,65,28,94,229                    // vdivps        %ymm13,%ymm12,%ymm12
  .byte  197,52,89,237                       // vmulps        %ymm5,%ymm9,%ymm13
  .byte  196,65,68,93,228                    // vminps        %ymm12,%ymm7,%ymm12
  .byte  196,66,101,168,227                  // vfmadd213ps   %ymm11,%ymm3,%ymm12
  .byte  196,65,20,88,228                    // vaddps        %ymm12,%ymm13,%ymm12
  .byte  197,20,88,233                       // vaddps        %ymm1,%ymm13,%ymm13
  .byte  197,244,194,203,0                   // vcmpeqps      %ymm3,%ymm1,%ymm1
  .byte  196,195,29,74,205,16                // vblendvps     %ymm1,%ymm13,%ymm12,%ymm1
  .byte  196,65,84,194,224,0                 // vcmpeqps      %ymm8,%ymm5,%ymm12
  .byte  197,36,88,221                       // vaddps        %ymm5,%ymm11,%ymm11
  .byte  196,195,117,74,203,192              // vblendvps     %ymm12,%ymm11,%ymm1,%ymm1
  .byte  197,44,89,210                       // vmulps        %ymm2,%ymm10,%ymm10
  .byte  197,100,89,222                      // vmulps        %ymm6,%ymm3,%ymm11
  .byte  197,100,92,226                      // vsubps        %ymm2,%ymm3,%ymm12
  .byte  196,65,36,94,220                    // vdivps        %ymm12,%ymm11,%ymm11
  .byte  197,52,89,230                       // vmulps        %ymm6,%ymm9,%ymm12
  .byte  196,65,68,93,219                    // vminps        %ymm11,%ymm7,%ymm11
  .byte  196,66,101,168,218                  // vfmadd213ps   %ymm10,%ymm3,%ymm11
  .byte  196,65,28,88,219                    // vaddps        %ymm11,%ymm12,%ymm11
  .byte  197,28,88,226                       // vaddps        %ymm2,%ymm12,%ymm12
  .byte  197,236,194,211,0                   // vcmpeqps      %ymm3,%ymm2,%ymm2
  .byte  196,195,37,74,212,32                // vblendvps     %ymm2,%ymm12,%ymm11,%ymm2
  .byte  196,65,76,194,192,0                 // vcmpeqps      %ymm8,%ymm6,%ymm8
  .byte  197,44,88,214                       // vaddps        %ymm6,%ymm10,%ymm10
  .byte  196,195,109,74,210,128              // vblendvps     %ymm8,%ymm10,%ymm2,%ymm2
  .byte  196,194,69,184,217                  // vfmadd231ps   %ymm9,%ymm7,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_hardlight_hsw
.globl _sk_hardlight_hsw
_sk_hardlight_hsw:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,92,215                       // vsubps        %ymm7,%ymm8,%ymm10
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,220                       // vmulps        %ymm4,%ymm8,%ymm11
  .byte  196,98,45,184,216                   // vfmadd231ps   %ymm0,%ymm10,%ymm11
  .byte  197,124,88,200                      // vaddps        %ymm0,%ymm0,%ymm9
  .byte  197,52,194,227,2                    // vcmpleps      %ymm3,%ymm9,%ymm12
  .byte  197,124,89,204                      // vmulps        %ymm4,%ymm0,%ymm9
  .byte  196,65,52,88,233                    // vaddps        %ymm9,%ymm9,%ymm13
  .byte  197,100,89,207                      // vmulps        %ymm7,%ymm3,%ymm9
  .byte  197,68,92,244                       // vsubps        %ymm4,%ymm7,%ymm14
  .byte  197,228,92,192                      // vsubps        %ymm0,%ymm3,%ymm0
  .byte  196,193,124,89,198                  // vmulps        %ymm14,%ymm0,%ymm0
  .byte  197,252,88,192                      // vaddps        %ymm0,%ymm0,%ymm0
  .byte  197,180,92,192                      // vsubps        %ymm0,%ymm9,%ymm0
  .byte  196,195,125,74,197,192              // vblendvps     %ymm12,%ymm13,%ymm0,%ymm0
  .byte  196,193,124,88,195                  // vaddps        %ymm11,%ymm0,%ymm0
  .byte  197,44,89,217                       // vmulps        %ymm1,%ymm10,%ymm11
  .byte  196,98,61,184,221                   // vfmadd231ps   %ymm5,%ymm8,%ymm11
  .byte  197,116,88,225                      // vaddps        %ymm1,%ymm1,%ymm12
  .byte  197,28,194,227,2                    // vcmpleps      %ymm3,%ymm12,%ymm12
  .byte  197,116,89,237                      // vmulps        %ymm5,%ymm1,%ymm13
  .byte  196,65,20,88,237                    // vaddps        %ymm13,%ymm13,%ymm13
  .byte  197,68,92,245                       // vsubps        %ymm5,%ymm7,%ymm14
  .byte  197,228,92,201                      // vsubps        %ymm1,%ymm3,%ymm1
  .byte  196,193,116,89,206                  // vmulps        %ymm14,%ymm1,%ymm1
  .byte  197,244,88,201                      // vaddps        %ymm1,%ymm1,%ymm1
  .byte  197,180,92,201                      // vsubps        %ymm1,%ymm9,%ymm1
  .byte  196,195,117,74,205,192              // vblendvps     %ymm12,%ymm13,%ymm1,%ymm1
  .byte  196,193,116,88,203                  // vaddps        %ymm11,%ymm1,%ymm1
  .byte  197,44,89,210                       // vmulps        %ymm2,%ymm10,%ymm10
  .byte  196,98,61,184,214                   // vfmadd231ps   %ymm6,%ymm8,%ymm10
  .byte  197,108,88,218                      // vaddps        %ymm2,%ymm2,%ymm11
  .byte  197,36,194,219,2                    // vcmpleps      %ymm3,%ymm11,%ymm11
  .byte  197,108,89,230                      // vmulps        %ymm6,%ymm2,%ymm12
  .byte  196,65,28,88,228                    // vaddps        %ymm12,%ymm12,%ymm12
  .byte  197,68,92,238                       // vsubps        %ymm6,%ymm7,%ymm13
  .byte  197,228,92,210                      // vsubps        %ymm2,%ymm3,%ymm2
  .byte  196,193,108,89,213                  // vmulps        %ymm13,%ymm2,%ymm2
  .byte  197,236,88,210                      // vaddps        %ymm2,%ymm2,%ymm2
  .byte  197,180,92,210                      // vsubps        %ymm2,%ymm9,%ymm2
  .byte  196,195,109,74,212,176              // vblendvps     %ymm11,%ymm12,%ymm2,%ymm2
  .byte  196,193,108,88,210                  // vaddps        %ymm10,%ymm2,%ymm2
  .byte  196,194,69,184,216                  // vfmadd231ps   %ymm8,%ymm7,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_overlay_hsw
.globl _sk_overlay_hsw
_sk_overlay_hsw:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,92,215                       // vsubps        %ymm7,%ymm8,%ymm10
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,220                       // vmulps        %ymm4,%ymm8,%ymm11
  .byte  196,98,45,184,216                   // vfmadd231ps   %ymm0,%ymm10,%ymm11
  .byte  197,92,88,204                       // vaddps        %ymm4,%ymm4,%ymm9
  .byte  197,52,194,231,2                    // vcmpleps      %ymm7,%ymm9,%ymm12
  .byte  197,124,89,204                      // vmulps        %ymm4,%ymm0,%ymm9
  .byte  196,65,52,88,233                    // vaddps        %ymm9,%ymm9,%ymm13
  .byte  197,100,89,207                      // vmulps        %ymm7,%ymm3,%ymm9
  .byte  197,68,92,244                       // vsubps        %ymm4,%ymm7,%ymm14
  .byte  197,228,92,192                      // vsubps        %ymm0,%ymm3,%ymm0
  .byte  196,193,124,89,198                  // vmulps        %ymm14,%ymm0,%ymm0
  .byte  197,252,88,192                      // vaddps        %ymm0,%ymm0,%ymm0
  .byte  197,180,92,192                      // vsubps        %ymm0,%ymm9,%ymm0
  .byte  196,195,125,74,197,192              // vblendvps     %ymm12,%ymm13,%ymm0,%ymm0
  .byte  196,193,124,88,195                  // vaddps        %ymm11,%ymm0,%ymm0
  .byte  197,44,89,217                       // vmulps        %ymm1,%ymm10,%ymm11
  .byte  196,98,61,184,221                   // vfmadd231ps   %ymm5,%ymm8,%ymm11
  .byte  197,84,88,229                       // vaddps        %ymm5,%ymm5,%ymm12
  .byte  197,28,194,231,2                    // vcmpleps      %ymm7,%ymm12,%ymm12
  .byte  197,116,89,237                      // vmulps        %ymm5,%ymm1,%ymm13
  .byte  196,65,20,88,237                    // vaddps        %ymm13,%ymm13,%ymm13
  .byte  197,68,92,245                       // vsubps        %ymm5,%ymm7,%ymm14
  .byte  197,228,92,201                      // vsubps        %ymm1,%ymm3,%ymm1
  .byte  196,193,116,89,206                  // vmulps        %ymm14,%ymm1,%ymm1
  .byte  197,244,88,201                      // vaddps        %ymm1,%ymm1,%ymm1
  .byte  197,180,92,201                      // vsubps        %ymm1,%ymm9,%ymm1
  .byte  196,195,117,74,205,192              // vblendvps     %ymm12,%ymm13,%ymm1,%ymm1
  .byte  196,193,116,88,203                  // vaddps        %ymm11,%ymm1,%ymm1
  .byte  197,44,89,210                       // vmulps        %ymm2,%ymm10,%ymm10
  .byte  196,98,61,184,214                   // vfmadd231ps   %ymm6,%ymm8,%ymm10
  .byte  197,76,88,222                       // vaddps        %ymm6,%ymm6,%ymm11
  .byte  197,36,194,223,2                    // vcmpleps      %ymm7,%ymm11,%ymm11
  .byte  197,108,89,230                      // vmulps        %ymm6,%ymm2,%ymm12
  .byte  196,65,28,88,228                    // vaddps        %ymm12,%ymm12,%ymm12
  .byte  197,68,92,238                       // vsubps        %ymm6,%ymm7,%ymm13
  .byte  197,228,92,210                      // vsubps        %ymm2,%ymm3,%ymm2
  .byte  196,193,108,89,213                  // vmulps        %ymm13,%ymm2,%ymm2
  .byte  197,236,88,210                      // vaddps        %ymm2,%ymm2,%ymm2
  .byte  197,180,92,210                      // vsubps        %ymm2,%ymm9,%ymm2
  .byte  196,195,109,74,212,176              // vblendvps     %ymm11,%ymm12,%ymm2,%ymm2
  .byte  196,193,108,88,210                  // vaddps        %ymm10,%ymm2,%ymm2
  .byte  196,194,69,184,216                  // vfmadd231ps   %ymm8,%ymm7,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_softlight_hsw
.globl _sk_softlight_hsw
_sk_softlight_hsw:
  .byte  197,252,17,84,36,200                // vmovups       %ymm2,-0x38(%rsp)
  .byte  196,65,44,87,210                    // vxorps        %ymm10,%ymm10,%ymm10
  .byte  197,44,194,223,1                    // vcmpltps      %ymm7,%ymm10,%ymm11
  .byte  197,92,94,199                       // vdivps        %ymm7,%ymm4,%ymm8
  .byte  196,67,45,74,224,176                // vblendvps     %ymm11,%ymm8,%ymm10,%ymm12
  .byte  196,65,28,88,196                    // vaddps        %ymm12,%ymm12,%ymm8
  .byte  196,65,60,88,232                    // vaddps        %ymm8,%ymm8,%ymm13
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  196,66,21,168,237                   // vfmadd213ps   %ymm13,%ymm13,%ymm13
  .byte  196,65,28,92,240                    // vsubps        %ymm8,%ymm12,%ymm14
  .byte  184,0,0,224,64                      // mov           $0x40e00000,%eax
  .byte  197,121,110,200                     // vmovd         %eax,%xmm9
  .byte  196,66,125,88,201                   // vpbroadcastd  %xmm9,%ymm9
  .byte  196,65,28,89,249                    // vmulps        %ymm9,%ymm12,%ymm15
  .byte  196,66,21,184,254                   // vfmadd231ps   %ymm14,%ymm13,%ymm15
  .byte  196,65,124,82,236                   // vrsqrtps      %ymm12,%ymm13
  .byte  196,65,124,83,237                   // vrcpps        %ymm13,%ymm13
  .byte  196,65,20,92,236                    // vsubps        %ymm12,%ymm13,%ymm13
  .byte  197,92,88,244                       // vaddps        %ymm4,%ymm4,%ymm14
  .byte  196,65,12,88,246                    // vaddps        %ymm14,%ymm14,%ymm14
  .byte  197,12,194,247,2                    // vcmpleps      %ymm7,%ymm14,%ymm14
  .byte  196,67,21,74,239,224                // vblendvps     %ymm14,%ymm15,%ymm13,%ymm13
  .byte  197,124,88,240                      // vaddps        %ymm0,%ymm0,%ymm14
  .byte  197,12,92,251                       // vsubps        %ymm3,%ymm14,%ymm15
  .byte  196,65,60,92,228                    // vsubps        %ymm12,%ymm8,%ymm12
  .byte  196,98,5,168,227                    // vfmadd213ps   %ymm3,%ymm15,%ymm12
  .byte  197,28,89,228                       // vmulps        %ymm4,%ymm12,%ymm12
  .byte  197,4,89,255                        // vmulps        %ymm7,%ymm15,%ymm15
  .byte  196,65,4,89,237                     // vmulps        %ymm13,%ymm15,%ymm13
  .byte  196,98,101,184,236                  // vfmadd231ps   %ymm4,%ymm3,%ymm13
  .byte  197,12,194,243,2                    // vcmpleps      %ymm3,%ymm14,%ymm14
  .byte  196,195,21,74,212,224               // vblendvps     %ymm14,%ymm12,%ymm13,%ymm2
  .byte  197,84,94,239                       // vdivps        %ymm7,%ymm5,%ymm13
  .byte  196,67,45,74,237,176                // vblendvps     %ymm11,%ymm13,%ymm10,%ymm13
  .byte  196,65,20,88,245                    // vaddps        %ymm13,%ymm13,%ymm14
  .byte  196,65,12,88,246                    // vaddps        %ymm14,%ymm14,%ymm14
  .byte  196,66,13,168,246                   // vfmadd213ps   %ymm14,%ymm14,%ymm14
  .byte  196,65,20,92,248                    // vsubps        %ymm8,%ymm13,%ymm15
  .byte  196,65,4,89,246                     // vmulps        %ymm14,%ymm15,%ymm14
  .byte  196,66,53,184,245                   // vfmadd231ps   %ymm13,%ymm9,%ymm14
  .byte  196,65,124,82,253                   // vrsqrtps      %ymm13,%ymm15
  .byte  196,65,124,83,255                   // vrcpps        %ymm15,%ymm15
  .byte  196,65,4,92,253                     // vsubps        %ymm13,%ymm15,%ymm15
  .byte  197,84,88,229                       // vaddps        %ymm5,%ymm5,%ymm12
  .byte  196,65,28,88,228                    // vaddps        %ymm12,%ymm12,%ymm12
  .byte  197,28,194,231,2                    // vcmpleps      %ymm7,%ymm12,%ymm12
  .byte  196,67,5,74,230,192                 // vblendvps     %ymm12,%ymm14,%ymm15,%ymm12
  .byte  197,116,88,241                      // vaddps        %ymm1,%ymm1,%ymm14
  .byte  196,65,60,92,237                    // vsubps        %ymm13,%ymm8,%ymm13
  .byte  197,12,92,251                       // vsubps        %ymm3,%ymm14,%ymm15
  .byte  196,98,5,168,235                    // vfmadd213ps   %ymm3,%ymm15,%ymm13
  .byte  197,4,89,255                        // vmulps        %ymm7,%ymm15,%ymm15
  .byte  196,65,4,89,228                     // vmulps        %ymm12,%ymm15,%ymm12
  .byte  197,20,89,237                       // vmulps        %ymm5,%ymm13,%ymm13
  .byte  196,98,101,184,229                  // vfmadd231ps   %ymm5,%ymm3,%ymm12
  .byte  197,12,194,243,2                    // vcmpleps      %ymm3,%ymm14,%ymm14
  .byte  196,67,29,74,237,224                // vblendvps     %ymm14,%ymm13,%ymm12,%ymm13
  .byte  197,76,94,231                       // vdivps        %ymm7,%ymm6,%ymm12
  .byte  196,67,45,74,212,176                // vblendvps     %ymm11,%ymm12,%ymm10,%ymm10
  .byte  196,65,44,88,218                    // vaddps        %ymm10,%ymm10,%ymm11
  .byte  196,65,36,88,219                    // vaddps        %ymm11,%ymm11,%ymm11
  .byte  196,66,37,168,219                   // vfmadd213ps   %ymm11,%ymm11,%ymm11
  .byte  196,65,44,92,224                    // vsubps        %ymm8,%ymm10,%ymm12
  .byte  196,65,28,89,219                    // vmulps        %ymm11,%ymm12,%ymm11
  .byte  196,66,45,168,203                   // vfmadd213ps   %ymm11,%ymm10,%ymm9
  .byte  196,65,124,82,218                   // vrsqrtps      %ymm10,%ymm11
  .byte  196,65,124,83,219                   // vrcpps        %ymm11,%ymm11
  .byte  196,65,36,92,218                    // vsubps        %ymm10,%ymm11,%ymm11
  .byte  197,76,88,230                       // vaddps        %ymm6,%ymm6,%ymm12
  .byte  196,65,28,88,228                    // vaddps        %ymm12,%ymm12,%ymm12
  .byte  197,28,194,231,2                    // vcmpleps      %ymm7,%ymm12,%ymm12
  .byte  196,67,37,74,201,192                // vblendvps     %ymm12,%ymm9,%ymm11,%ymm9
  .byte  197,124,16,116,36,200               // vmovups       -0x38(%rsp),%ymm14
  .byte  196,65,12,88,222                    // vaddps        %ymm14,%ymm14,%ymm11
  .byte  197,36,92,227                       // vsubps        %ymm3,%ymm11,%ymm12
  .byte  196,65,60,92,210                    // vsubps        %ymm10,%ymm8,%ymm10
  .byte  196,98,29,168,211                   // vfmadd213ps   %ymm3,%ymm12,%ymm10
  .byte  197,28,89,231                       // vmulps        %ymm7,%ymm12,%ymm12
  .byte  196,65,28,89,201                    // vmulps        %ymm9,%ymm12,%ymm9
  .byte  197,44,89,214                       // vmulps        %ymm6,%ymm10,%ymm10
  .byte  196,98,101,184,206                  // vfmadd231ps   %ymm6,%ymm3,%ymm9
  .byte  197,36,194,219,2                    // vcmpleps      %ymm3,%ymm11,%ymm11
  .byte  196,67,53,74,202,176                // vblendvps     %ymm11,%ymm10,%ymm9,%ymm9
  .byte  197,60,92,215                       // vsubps        %ymm7,%ymm8,%ymm10
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,220                       // vmulps        %ymm4,%ymm8,%ymm11
  .byte  196,98,45,184,216                   // vfmadd231ps   %ymm0,%ymm10,%ymm11
  .byte  196,193,108,88,195                  // vaddps        %ymm11,%ymm2,%ymm0
  .byte  197,172,89,201                      // vmulps        %ymm1,%ymm10,%ymm1
  .byte  196,226,61,184,205                  // vfmadd231ps   %ymm5,%ymm8,%ymm1
  .byte  196,193,116,88,205                  // vaddps        %ymm13,%ymm1,%ymm1
  .byte  196,193,44,89,214                   // vmulps        %ymm14,%ymm10,%ymm2
  .byte  196,226,61,184,214                  // vfmadd231ps   %ymm6,%ymm8,%ymm2
  .byte  196,193,108,88,209                  // vaddps        %ymm9,%ymm2,%ymm2
  .byte  196,194,69,184,216                  // vfmadd231ps   %ymm8,%ymm7,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_0_hsw
.globl _sk_clamp_0_hsw
_sk_clamp_0_hsw:
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  196,193,124,95,192                  // vmaxps        %ymm8,%ymm0,%ymm0
  .byte  196,193,116,95,200                  // vmaxps        %ymm8,%ymm1,%ymm1
  .byte  196,193,108,95,208                  // vmaxps        %ymm8,%ymm2,%ymm2
  .byte  196,193,100,95,216                  // vmaxps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_1_hsw
.globl _sk_clamp_1_hsw
_sk_clamp_1_hsw:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  196,193,124,93,192                  // vminps        %ymm8,%ymm0,%ymm0
  .byte  196,193,116,93,200                  // vminps        %ymm8,%ymm1,%ymm1
  .byte  196,193,108,93,208                  // vminps        %ymm8,%ymm2,%ymm2
  .byte  196,193,100,93,216                  // vminps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_a_hsw
.globl _sk_clamp_a_hsw
_sk_clamp_a_hsw:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  196,193,100,93,216                  // vminps        %ymm8,%ymm3,%ymm3
  .byte  197,252,93,195                      // vminps        %ymm3,%ymm0,%ymm0
  .byte  197,244,93,203                      // vminps        %ymm3,%ymm1,%ymm1
  .byte  197,236,93,211                      // vminps        %ymm3,%ymm2,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_set_rgb_hsw
.globl _sk_set_rgb_hsw
_sk_set_rgb_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,226,125,24,0                    // vbroadcastss  (%rax),%ymm0
  .byte  196,226,125,24,72,4                 // vbroadcastss  0x4(%rax),%ymm1
  .byte  196,226,125,24,80,8                 // vbroadcastss  0x8(%rax),%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_swap_rb_hsw
.globl _sk_swap_rb_hsw
_sk_swap_rb_hsw:
  .byte  197,124,40,192                      // vmovaps       %ymm0,%ymm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,40,194                      // vmovaps       %ymm2,%ymm0
  .byte  197,124,41,194                      // vmovaps       %ymm8,%ymm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_swap_hsw
.globl _sk_swap_hsw
_sk_swap_hsw:
  .byte  197,124,40,195                      // vmovaps       %ymm3,%ymm8
  .byte  197,124,40,202                      // vmovaps       %ymm2,%ymm9
  .byte  197,124,40,209                      // vmovaps       %ymm1,%ymm10
  .byte  197,124,40,216                      // vmovaps       %ymm0,%ymm11
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,40,196                      // vmovaps       %ymm4,%ymm0
  .byte  197,252,40,205                      // vmovaps       %ymm5,%ymm1
  .byte  197,252,40,214                      // vmovaps       %ymm6,%ymm2
  .byte  197,252,40,223                      // vmovaps       %ymm7,%ymm3
  .byte  197,124,41,220                      // vmovaps       %ymm11,%ymm4
  .byte  197,124,41,213                      // vmovaps       %ymm10,%ymm5
  .byte  197,124,41,206                      // vmovaps       %ymm9,%ymm6
  .byte  197,124,41,199                      // vmovaps       %ymm8,%ymm7
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_move_src_dst_hsw
.globl _sk_move_src_dst_hsw
_sk_move_src_dst_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,40,224                      // vmovaps       %ymm0,%ymm4
  .byte  197,252,40,233                      // vmovaps       %ymm1,%ymm5
  .byte  197,252,40,242                      // vmovaps       %ymm2,%ymm6
  .byte  197,252,40,251                      // vmovaps       %ymm3,%ymm7
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_move_dst_src_hsw
.globl _sk_move_dst_src_hsw
_sk_move_dst_src_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,40,196                      // vmovaps       %ymm4,%ymm0
  .byte  197,252,40,205                      // vmovaps       %ymm5,%ymm1
  .byte  197,252,40,214                      // vmovaps       %ymm6,%ymm2
  .byte  197,252,40,223                      // vmovaps       %ymm7,%ymm3
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_premul_hsw
.globl _sk_premul_hsw
_sk_premul_hsw:
  .byte  197,252,89,195                      // vmulps        %ymm3,%ymm0,%ymm0
  .byte  197,244,89,203                      // vmulps        %ymm3,%ymm1,%ymm1
  .byte  197,236,89,211                      // vmulps        %ymm3,%ymm2,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_unpremul_hsw
.globl _sk_unpremul_hsw
_sk_unpremul_hsw:
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  196,65,100,194,200,0                // vcmpeqps      %ymm8,%ymm3,%ymm9
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,208                     // vmovd         %eax,%xmm10
  .byte  196,66,125,88,210                   // vpbroadcastd  %xmm10,%ymm10
  .byte  197,44,94,211                       // vdivps        %ymm3,%ymm10,%ymm10
  .byte  196,67,45,74,192,144                // vblendvps     %ymm9,%ymm8,%ymm10,%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_from_srgb_hsw
.globl _sk_from_srgb_hsw
_sk_from_srgb_hsw:
  .byte  184,145,131,158,61                  // mov           $0x3d9e8391,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,89,200                       // vmulps        %ymm0,%ymm8,%ymm9
  .byte  197,124,89,208                      // vmulps        %ymm0,%ymm0,%ymm10
  .byte  184,154,153,153,62                  // mov           $0x3e99999a,%eax
  .byte  197,121,110,216                     // vmovd         %eax,%xmm11
  .byte  196,66,125,88,219                   // vpbroadcastd  %xmm11,%ymm11
  .byte  184,92,143,50,63                    // mov           $0x3f328f5c,%eax
  .byte  197,121,110,224                     // vmovd         %eax,%xmm12
  .byte  196,66,125,88,228                   // vpbroadcastd  %xmm12,%ymm12
  .byte  196,65,125,111,235                  // vmovdqa       %ymm11,%ymm13
  .byte  196,66,125,168,236                  // vfmadd213ps   %ymm12,%ymm0,%ymm13
  .byte  184,10,215,35,59                    // mov           $0x3b23d70a,%eax
  .byte  197,121,110,240                     // vmovd         %eax,%xmm14
  .byte  196,66,125,88,246                   // vpbroadcastd  %xmm14,%ymm14
  .byte  196,66,45,168,238                   // vfmadd213ps   %ymm14,%ymm10,%ymm13
  .byte  184,174,71,97,61                    // mov           $0x3d6147ae,%eax
  .byte  197,121,110,208                     // vmovd         %eax,%xmm10
  .byte  196,66,125,88,210                   // vpbroadcastd  %xmm10,%ymm10
  .byte  196,193,124,194,194,1               // vcmpltps      %ymm10,%ymm0,%ymm0
  .byte  196,195,21,74,193,0                 // vblendvps     %ymm0,%ymm9,%ymm13,%ymm0
  .byte  197,60,89,201                       // vmulps        %ymm1,%ymm8,%ymm9
  .byte  197,116,89,233                      // vmulps        %ymm1,%ymm1,%ymm13
  .byte  196,65,125,111,251                  // vmovdqa       %ymm11,%ymm15
  .byte  196,66,117,168,252                  // vfmadd213ps   %ymm12,%ymm1,%ymm15
  .byte  196,66,21,168,254                   // vfmadd213ps   %ymm14,%ymm13,%ymm15
  .byte  196,193,116,194,202,1               // vcmpltps      %ymm10,%ymm1,%ymm1
  .byte  196,195,5,74,201,16                 // vblendvps     %ymm1,%ymm9,%ymm15,%ymm1
  .byte  197,60,89,194                       // vmulps        %ymm2,%ymm8,%ymm8
  .byte  197,108,89,202                      // vmulps        %ymm2,%ymm2,%ymm9
  .byte  196,66,109,168,220                  // vfmadd213ps   %ymm12,%ymm2,%ymm11
  .byte  196,66,53,168,222                   // vfmadd213ps   %ymm14,%ymm9,%ymm11
  .byte  196,193,108,194,210,1               // vcmpltps      %ymm10,%ymm2,%ymm2
  .byte  196,195,37,74,208,32                // vblendvps     %ymm2,%ymm8,%ymm11,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_to_srgb_hsw
.globl _sk_to_srgb_hsw
_sk_to_srgb_hsw:
  .byte  197,124,82,192                      // vrsqrtps      %ymm0,%ymm8
  .byte  196,65,124,83,216                   // vrcpps        %ymm8,%ymm11
  .byte  196,65,124,82,224                   // vrsqrtps      %ymm8,%ymm12
  .byte  184,41,92,71,65                     // mov           $0x41475c29,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,89,232                       // vmulps        %ymm0,%ymm8,%ymm13
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,200                     // vmovd         %eax,%xmm9
  .byte  196,66,125,88,201                   // vpbroadcastd  %xmm9,%ymm9
  .byte  184,194,135,210,62                  // mov           $0x3ed287c2,%eax
  .byte  197,121,110,208                     // vmovd         %eax,%xmm10
  .byte  196,66,125,88,210                   // vpbroadcastd  %xmm10,%ymm10
  .byte  184,206,111,48,63                   // mov           $0x3f306fce,%eax
  .byte  197,121,110,240                     // vmovd         %eax,%xmm14
  .byte  196,66,125,88,246                   // vpbroadcastd  %xmm14,%ymm14
  .byte  184,168,87,202,61                   // mov           $0x3dca57a8,%eax
  .byte  53,0,0,0,128                        // xor           $0x80000000,%eax
  .byte  197,121,110,248                     // vmovd         %eax,%xmm15
  .byte  196,66,125,88,255                   // vpbroadcastd  %xmm15,%ymm15
  .byte  196,66,13,168,223                   // vfmadd213ps   %ymm15,%ymm14,%ymm11
  .byte  196,66,45,184,220                   // vfmadd231ps   %ymm12,%ymm10,%ymm11
  .byte  196,65,52,93,219                    // vminps        %ymm11,%ymm9,%ymm11
  .byte  184,4,231,140,59                    // mov           $0x3b8ce704,%eax
  .byte  197,121,110,224                     // vmovd         %eax,%xmm12
  .byte  196,66,125,88,228                   // vpbroadcastd  %xmm12,%ymm12
  .byte  196,193,124,194,196,1               // vcmpltps      %ymm12,%ymm0,%ymm0
  .byte  196,195,37,74,197,0                 // vblendvps     %ymm0,%ymm13,%ymm11,%ymm0
  .byte  197,124,82,217                      // vrsqrtps      %ymm1,%ymm11
  .byte  196,65,124,83,235                   // vrcpps        %ymm11,%ymm13
  .byte  196,65,124,82,219                   // vrsqrtps      %ymm11,%ymm11
  .byte  196,66,13,168,239                   // vfmadd213ps   %ymm15,%ymm14,%ymm13
  .byte  196,66,45,184,235                   // vfmadd231ps   %ymm11,%ymm10,%ymm13
  .byte  197,60,89,217                       // vmulps        %ymm1,%ymm8,%ymm11
  .byte  196,65,52,93,237                    // vminps        %ymm13,%ymm9,%ymm13
  .byte  196,193,116,194,204,1               // vcmpltps      %ymm12,%ymm1,%ymm1
  .byte  196,195,21,74,203,16                // vblendvps     %ymm1,%ymm11,%ymm13,%ymm1
  .byte  197,124,82,218                      // vrsqrtps      %ymm2,%ymm11
  .byte  196,65,124,83,235                   // vrcpps        %ymm11,%ymm13
  .byte  196,66,13,168,239                   // vfmadd213ps   %ymm15,%ymm14,%ymm13
  .byte  196,65,124,82,219                   // vrsqrtps      %ymm11,%ymm11
  .byte  196,66,45,184,235                   // vfmadd231ps   %ymm11,%ymm10,%ymm13
  .byte  196,65,52,93,205                    // vminps        %ymm13,%ymm9,%ymm9
  .byte  197,60,89,194                       // vmulps        %ymm2,%ymm8,%ymm8
  .byte  196,193,108,194,212,1               // vcmpltps      %ymm12,%ymm2,%ymm2
  .byte  196,195,53,74,208,32                // vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_scale_1_float_hsw
.globl _sk_scale_1_float_hsw
_sk_scale_1_float_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_scale_u8_hsw
.globl _sk_scale_u8_hsw
_sk_scale_u8_hsw:
  .byte  73,137,200                          // mov           %rcx,%r8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,1,248                            // add           %rdi,%rax
  .byte  77,133,192                          // test          %r8,%r8
  .byte  117,56                              // jne           d47 <_sk_scale_u8_hsw+0x48>
  .byte  197,122,126,0                       // vmovq         (%rax),%xmm8
  .byte  196,66,125,49,192                   // vpmovzxbd     %xmm8,%ymm8
  .byte  196,65,124,91,192                   // vcvtdq2ps     %ymm8,%ymm8
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  197,121,110,200                     // vmovd         %eax,%xmm9
  .byte  196,66,125,88,201                   // vpbroadcastd  %xmm9,%ymm9
  .byte  196,65,60,89,193                    // vmulps        %ymm9,%ymm8,%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,137,193                          // mov           %r8,%rcx
  .byte  255,224                             // jmpq          *%rax
  .byte  49,201                              // xor           %ecx,%ecx
  .byte  77,137,194                          // mov           %r8,%r10
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  68,15,182,24                        // movzbl        (%rax),%r11d
  .byte  72,255,192                          // inc           %rax
  .byte  73,211,227                          // shl           %cl,%r11
  .byte  77,9,217                            // or            %r11,%r9
  .byte  72,131,193,8                        // add           $0x8,%rcx
  .byte  73,255,202                          // dec           %r10
  .byte  117,234                             // jne           d4f <_sk_scale_u8_hsw+0x50>
  .byte  196,65,249,110,193                  // vmovq         %r9,%xmm8
  .byte  235,167                             // jmp           d13 <_sk_scale_u8_hsw+0x14>

HIDDEN _sk_lerp_1_float_hsw
.globl _sk_lerp_1_float_hsw
_sk_lerp_1_float_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  197,252,92,196                      // vsubps        %ymm4,%ymm0,%ymm0
  .byte  196,226,61,168,196                  // vfmadd213ps   %ymm4,%ymm8,%ymm0
  .byte  197,244,92,205                      // vsubps        %ymm5,%ymm1,%ymm1
  .byte  196,226,61,168,205                  // vfmadd213ps   %ymm5,%ymm8,%ymm1
  .byte  197,236,92,214                      // vsubps        %ymm6,%ymm2,%ymm2
  .byte  196,226,61,168,214                  // vfmadd213ps   %ymm6,%ymm8,%ymm2
  .byte  197,228,92,223                      // vsubps        %ymm7,%ymm3,%ymm3
  .byte  196,226,61,168,223                  // vfmadd213ps   %ymm7,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_lerp_u8_hsw
.globl _sk_lerp_u8_hsw
_sk_lerp_u8_hsw:
  .byte  73,137,200                          // mov           %rcx,%r8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,1,248                            // add           %rdi,%rax
  .byte  77,133,192                          // test          %r8,%r8
  .byte  117,76                              // jne           df7 <_sk_lerp_u8_hsw+0x5c>
  .byte  197,122,126,0                       // vmovq         (%rax),%xmm8
  .byte  196,66,125,49,192                   // vpmovzxbd     %xmm8,%ymm8
  .byte  196,65,124,91,192                   // vcvtdq2ps     %ymm8,%ymm8
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  197,121,110,200                     // vmovd         %eax,%xmm9
  .byte  196,66,125,88,201                   // vpbroadcastd  %xmm9,%ymm9
  .byte  196,65,60,89,193                    // vmulps        %ymm9,%ymm8,%ymm8
  .byte  197,252,92,196                      // vsubps        %ymm4,%ymm0,%ymm0
  .byte  196,226,61,168,196                  // vfmadd213ps   %ymm4,%ymm8,%ymm0
  .byte  197,244,92,205                      // vsubps        %ymm5,%ymm1,%ymm1
  .byte  196,226,61,168,205                  // vfmadd213ps   %ymm5,%ymm8,%ymm1
  .byte  197,236,92,214                      // vsubps        %ymm6,%ymm2,%ymm2
  .byte  196,226,61,168,214                  // vfmadd213ps   %ymm6,%ymm8,%ymm2
  .byte  197,228,92,223                      // vsubps        %ymm7,%ymm3,%ymm3
  .byte  196,226,61,168,223                  // vfmadd213ps   %ymm7,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,137,193                          // mov           %r8,%rcx
  .byte  255,224                             // jmpq          *%rax
  .byte  49,201                              // xor           %ecx,%ecx
  .byte  77,137,194                          // mov           %r8,%r10
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  68,15,182,24                        // movzbl        (%rax),%r11d
  .byte  72,255,192                          // inc           %rax
  .byte  73,211,227                          // shl           %cl,%r11
  .byte  77,9,217                            // or            %r11,%r9
  .byte  72,131,193,8                        // add           $0x8,%rcx
  .byte  73,255,202                          // dec           %r10
  .byte  117,234                             // jne           dff <_sk_lerp_u8_hsw+0x64>
  .byte  196,65,249,110,193                  // vmovq         %r9,%xmm8
  .byte  235,147                             // jmp           daf <_sk_lerp_u8_hsw+0x14>

HIDDEN _sk_lerp_565_hsw
.globl _sk_lerp_565_hsw
_sk_lerp_565_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,16                           // mov           (%rax),%r10
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,179,0,0,0                    // jne           edd <_sk_lerp_565_hsw+0xc1>
  .byte  196,193,122,111,28,122              // vmovdqu       (%r10,%rdi,2),%xmm3
  .byte  196,98,125,51,195                   // vpmovzxwd     %xmm3,%ymm8
  .byte  184,0,248,0,0                       // mov           $0xf800,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,226,125,88,219                  // vpbroadcastd  %xmm3,%ymm3
  .byte  196,193,101,219,216                 // vpand         %ymm8,%ymm3,%ymm3
  .byte  197,124,91,203                      // vcvtdq2ps     %ymm3,%ymm9
  .byte  184,8,33,132,55                     // mov           $0x37842108,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,226,125,88,219                  // vpbroadcastd  %xmm3,%ymm3
  .byte  197,52,89,203                       // vmulps        %ymm3,%ymm9,%ymm9
  .byte  184,224,7,0,0                       // mov           $0x7e0,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,226,125,88,219                  // vpbroadcastd  %xmm3,%ymm3
  .byte  196,193,101,219,216                 // vpand         %ymm8,%ymm3,%ymm3
  .byte  197,124,91,211                      // vcvtdq2ps     %ymm3,%ymm10
  .byte  184,33,8,2,58                       // mov           $0x3a020821,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,226,125,88,219                  // vpbroadcastd  %xmm3,%ymm3
  .byte  197,44,89,211                       // vmulps        %ymm3,%ymm10,%ymm10
  .byte  184,31,0,0,0                        // mov           $0x1f,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,226,125,88,219                  // vpbroadcastd  %xmm3,%ymm3
  .byte  196,193,101,219,216                 // vpand         %ymm8,%ymm3,%ymm3
  .byte  197,124,91,195                      // vcvtdq2ps     %ymm3,%ymm8
  .byte  184,8,33,4,61                       // mov           $0x3d042108,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,226,125,88,219                  // vpbroadcastd  %xmm3,%ymm3
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  197,252,92,196                      // vsubps        %ymm4,%ymm0,%ymm0
  .byte  196,226,53,168,196                  // vfmadd213ps   %ymm4,%ymm9,%ymm0
  .byte  197,244,92,205                      // vsubps        %ymm5,%ymm1,%ymm1
  .byte  196,226,45,168,205                  // vfmadd213ps   %ymm5,%ymm10,%ymm1
  .byte  197,236,92,214                      // vsubps        %ymm6,%ymm2,%ymm2
  .byte  196,226,101,168,214                 // vfmadd213ps   %ymm6,%ymm3,%ymm2
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,226,125,88,219                  // vpbroadcastd  %xmm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  65,137,200                          // mov           %ecx,%r8d
  .byte  65,128,224,7                        // and           $0x7,%r8b
  .byte  197,225,239,219                     // vpxor         %xmm3,%xmm3,%xmm3
  .byte  65,254,200                          // dec           %r8b
  .byte  65,128,248,6                        // cmp           $0x6,%r8b
  .byte  15,135,59,255,255,255               // ja            e30 <_sk_lerp_565_hsw+0x14>
  .byte  69,15,182,192                       // movzbl        %r8b,%r8d
  .byte  76,141,13,76,0,0,0                  // lea           0x4c(%rip),%r9        # f4c <_sk_lerp_565_hsw+0x130>
  .byte  75,99,4,129                         // movslq        (%r9,%r8,4),%rax
  .byte  76,1,200                            // add           %r9,%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  197,225,239,219                     // vpxor         %xmm3,%xmm3,%xmm3
  .byte  196,193,97,196,92,122,12,6          // vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm3,%xmm3
  .byte  196,193,97,196,92,122,10,5          // vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm3,%xmm3
  .byte  196,193,97,196,92,122,8,4           // vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm3,%xmm3
  .byte  196,193,97,196,92,122,6,3           // vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm3,%xmm3
  .byte  196,193,97,196,92,122,4,2           // vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm3,%xmm3
  .byte  196,193,97,196,92,122,2,1           // vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm3,%xmm3
  .byte  196,193,97,196,28,122,0             // vpinsrw       $0x0,(%r10,%rdi,2),%xmm3,%xmm3
  .byte  233,231,254,255,255                 // jmpq          e30 <_sk_lerp_565_hsw+0x14>
  .byte  15,31,0                             // nopl          (%rax)
  .byte  241                                 // icebp
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  233,255,255,255,225                 // jmpq          ffffffffe2000f54 <_sk_linear_gradient_2stops_hsw+0xffffffffe1fff248>
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  217,255                             // fcos
  .byte  255                                 // (bad)
  .byte  255,209                             // callq         *%rcx
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,201                             // dec           %ecx
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  189                                 // .byte         0xbd
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // .byte         0xff

HIDDEN _sk_load_tables_hsw
.globl _sk_load_tables_hsw
_sk_load_tables_hsw:
  .byte  73,137,200                          // mov           %rcx,%r8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,141,12,189,0,0,0,0               // lea           0x0(,%rdi,4),%r9
  .byte  76,3,8                              // add           (%rax),%r9
  .byte  77,133,192                          // test          %r8,%r8
  .byte  117,121                             // jne           ff6 <_sk_load_tables_hsw+0x8e>
  .byte  196,193,126,111,25                  // vmovdqu       (%r9),%ymm3
  .byte  185,255,0,0,0                       // mov           $0xff,%ecx
  .byte  197,249,110,193                     // vmovd         %ecx,%xmm0
  .byte  196,226,125,88,208                  // vpbroadcastd  %xmm0,%ymm2
  .byte  197,237,219,203                     // vpand         %ymm3,%ymm2,%ymm1
  .byte  196,65,61,118,192                   // vpcmpeqd      %ymm8,%ymm8,%ymm8
  .byte  72,139,72,8                         // mov           0x8(%rax),%rcx
  .byte  76,139,72,16                        // mov           0x10(%rax),%r9
  .byte  196,65,53,118,201                   // vpcmpeqd      %ymm9,%ymm9,%ymm9
  .byte  196,226,53,146,4,137                // vgatherdps    %ymm9,(%rcx,%ymm1,4),%ymm0
  .byte  197,245,114,211,8                   // vpsrld        $0x8,%ymm3,%ymm1
  .byte  197,109,219,201                     // vpand         %ymm1,%ymm2,%ymm9
  .byte  196,65,45,118,210                   // vpcmpeqd      %ymm10,%ymm10,%ymm10
  .byte  196,130,45,146,12,137               // vgatherdps    %ymm10,(%r9,%ymm9,4),%ymm1
  .byte  72,139,64,24                        // mov           0x18(%rax),%rax
  .byte  197,181,114,211,16                  // vpsrld        $0x10,%ymm3,%ymm9
  .byte  196,65,109,219,201                  // vpand         %ymm9,%ymm2,%ymm9
  .byte  196,162,61,146,20,136               // vgatherdps    %ymm8,(%rax,%ymm9,4),%ymm2
  .byte  197,229,114,211,24                  // vpsrld        $0x18,%ymm3,%ymm3
  .byte  197,124,91,195                      // vcvtdq2ps     %ymm3,%ymm8
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,226,125,88,219                  // vpbroadcastd  %xmm3,%ymm3
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,137,193                          // mov           %r8,%rcx
  .byte  255,224                             // jmpq          *%rax
  .byte  185,8,0,0,0                         // mov           $0x8,%ecx
  .byte  68,41,193                           // sub           %r8d,%ecx
  .byte  192,225,3                           // shl           $0x3,%cl
  .byte  73,199,194,255,255,255,255          // mov           $0xffffffffffffffff,%r10
  .byte  73,211,234                          // shr           %cl,%r10
  .byte  196,193,249,110,194                 // vmovq         %r10,%xmm0
  .byte  196,226,125,33,192                  // vpmovsxbd     %xmm0,%ymm0
  .byte  196,194,125,140,25                  // vpmaskmovd    (%r9),%ymm0,%ymm3
  .byte  233,99,255,255,255                  // jmpq          f82 <_sk_load_tables_hsw+0x1a>

HIDDEN _sk_load_a8_hsw
.globl _sk_load_a8_hsw
_sk_load_a8_hsw:
  .byte  73,137,200                          // mov           %rcx,%r8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,1,248                            // add           %rdi,%rax
  .byte  77,133,192                          // test          %r8,%r8
  .byte  117,50                              // jne           1061 <_sk_load_a8_hsw+0x42>
  .byte  197,250,126,0                       // vmovq         (%rax),%xmm0
  .byte  196,226,125,49,192                  // vpmovzxbd     %xmm0,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  196,226,125,88,201                  // vpbroadcastd  %xmm1,%ymm1
  .byte  197,252,89,217                      // vmulps        %ymm1,%ymm0,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  76,137,193                          // mov           %r8,%rcx
  .byte  255,224                             // jmpq          *%rax
  .byte  49,201                              // xor           %ecx,%ecx
  .byte  77,137,194                          // mov           %r8,%r10
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  68,15,182,24                        // movzbl        (%rax),%r11d
  .byte  72,255,192                          // inc           %rax
  .byte  73,211,227                          // shl           %cl,%r11
  .byte  77,9,217                            // or            %r11,%r9
  .byte  72,131,193,8                        // add           $0x8,%rcx
  .byte  73,255,202                          // dec           %r10
  .byte  117,234                             // jne           1069 <_sk_load_a8_hsw+0x4a>
  .byte  196,193,249,110,193                 // vmovq         %r9,%xmm0
  .byte  235,173                             // jmp           1033 <_sk_load_a8_hsw+0x14>

HIDDEN _sk_store_a8_hsw
.globl _sk_store_a8_hsw
_sk_store_a8_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,8                            // mov           (%rax),%r9
  .byte  184,0,0,127,67                      // mov           $0x437f0000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,89,195                       // vmulps        %ymm3,%ymm8,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,67,125,25,193,1                 // vextractf128  $0x1,%ymm8,%xmm9
  .byte  196,66,57,43,193                    // vpackusdw     %xmm9,%xmm8,%xmm8
  .byte  196,65,57,103,192                   // vpackuswb     %xmm8,%xmm8,%xmm8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,10                              // jne           10c1 <_sk_store_a8_hsw+0x3b>
  .byte  196,65,123,17,4,57                  // vmovsd        %xmm8,(%r9,%rdi,1)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  65,137,200                          // mov           %ecx,%r8d
  .byte  65,128,224,7                        // and           $0x7,%r8b
  .byte  65,254,200                          // dec           %r8b
  .byte  65,128,248,6                        // cmp           $0x6,%r8b
  .byte  119,236                             // ja            10bd <_sk_store_a8_hsw+0x37>
  .byte  196,66,121,48,192                   // vpmovzxbw     %xmm8,%xmm8
  .byte  65,15,182,192                       // movzbl        %r8b,%eax
  .byte  76,141,5,67,0,0,0                   // lea           0x43(%rip),%r8        # 1124 <_sk_store_a8_hsw+0x9e>
  .byte  73,99,4,128                         // movslq        (%r8,%rax,4),%rax
  .byte  76,1,192                            // add           %r8,%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  196,67,121,20,68,57,6,12            // vpextrb       $0xc,%xmm8,0x6(%r9,%rdi,1)
  .byte  196,67,121,20,68,57,5,10            // vpextrb       $0xa,%xmm8,0x5(%r9,%rdi,1)
  .byte  196,67,121,20,68,57,4,8             // vpextrb       $0x8,%xmm8,0x4(%r9,%rdi,1)
  .byte  196,67,121,20,68,57,3,6             // vpextrb       $0x6,%xmm8,0x3(%r9,%rdi,1)
  .byte  196,67,121,20,68,57,2,4             // vpextrb       $0x4,%xmm8,0x2(%r9,%rdi,1)
  .byte  196,67,121,20,68,57,1,2             // vpextrb       $0x2,%xmm8,0x1(%r9,%rdi,1)
  .byte  196,67,121,20,4,57,0                // vpextrb       $0x0,%xmm8,(%r9,%rdi,1)
  .byte  235,154                             // jmp           10bd <_sk_store_a8_hsw+0x37>
  .byte  144                                 // nop
  .byte  246,255                             // idiv          %bh
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  238                                 // out           %al,(%dx)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,230                             // jmpq          *%rsi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  222,255                             // fdivrp        %st,%st(7)
  .byte  255                                 // (bad)
  .byte  255,214                             // callq         *%rsi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,206                             // dec           %esi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,198                             // inc           %esi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // .byte         0xff

HIDDEN _sk_load_g8_hsw
.globl _sk_load_g8_hsw
_sk_load_g8_hsw:
  .byte  73,137,200                          // mov           %rcx,%r8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,1,248                            // add           %rdi,%rax
  .byte  77,133,192                          // test          %r8,%r8
  .byte  117,60                              // jne           118c <_sk_load_g8_hsw+0x4c>
  .byte  197,250,126,0                       // vmovq         (%rax),%xmm0
  .byte  196,226,125,49,192                  // vpmovzxbd     %xmm0,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  196,226,125,88,201                  // vpbroadcastd  %xmm1,%ymm1
  .byte  197,252,89,193                      // vmulps        %ymm1,%ymm0,%ymm0
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  196,226,125,88,217                  // vpbroadcastd  %xmm1,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,137,193                          // mov           %r8,%rcx
  .byte  197,252,40,200                      // vmovaps       %ymm0,%ymm1
  .byte  197,252,40,208                      // vmovaps       %ymm0,%ymm2
  .byte  255,224                             // jmpq          *%rax
  .byte  49,201                              // xor           %ecx,%ecx
  .byte  77,137,194                          // mov           %r8,%r10
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  68,15,182,24                        // movzbl        (%rax),%r11d
  .byte  72,255,192                          // inc           %rax
  .byte  73,211,227                          // shl           %cl,%r11
  .byte  77,9,217                            // or            %r11,%r9
  .byte  72,131,193,8                        // add           $0x8,%rcx
  .byte  73,255,202                          // dec           %r10
  .byte  117,234                             // jne           1194 <_sk_load_g8_hsw+0x54>
  .byte  196,193,249,110,193                 // vmovq         %r9,%xmm0
  .byte  235,163                             // jmp           1154 <_sk_load_g8_hsw+0x14>

HIDDEN _sk_load_565_hsw
.globl _sk_load_565_hsw
_sk_load_565_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,16                           // mov           (%rax),%r10
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,149,0,0,0                    // jne           1254 <_sk_load_565_hsw+0xa3>
  .byte  196,193,122,111,4,122               // vmovdqu       (%r10,%rdi,2),%xmm0
  .byte  196,226,125,51,208                  // vpmovzxwd     %xmm0,%ymm2
  .byte  184,0,248,0,0                       // mov           $0xf800,%eax
  .byte  197,249,110,192                     // vmovd         %eax,%xmm0
  .byte  196,226,125,88,192                  // vpbroadcastd  %xmm0,%ymm0
  .byte  197,253,219,194                     // vpand         %ymm2,%ymm0,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  184,8,33,132,55                     // mov           $0x37842108,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  196,226,125,88,201                  // vpbroadcastd  %xmm1,%ymm1
  .byte  197,252,89,193                      // vmulps        %ymm1,%ymm0,%ymm0
  .byte  184,224,7,0,0                       // mov           $0x7e0,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  196,226,125,88,201                  // vpbroadcastd  %xmm1,%ymm1
  .byte  197,245,219,202                     // vpand         %ymm2,%ymm1,%ymm1
  .byte  197,252,91,201                      // vcvtdq2ps     %ymm1,%ymm1
  .byte  184,33,8,2,58                       // mov           $0x3a020821,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,226,125,88,219                  // vpbroadcastd  %xmm3,%ymm3
  .byte  197,244,89,203                      // vmulps        %ymm3,%ymm1,%ymm1
  .byte  184,31,0,0,0                        // mov           $0x1f,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,226,125,88,219                  // vpbroadcastd  %xmm3,%ymm3
  .byte  197,229,219,210                     // vpand         %ymm2,%ymm3,%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  184,8,33,4,61                       // mov           $0x3d042108,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,226,125,88,219                  // vpbroadcastd  %xmm3,%ymm3
  .byte  197,236,89,211                      // vmulps        %ymm3,%ymm2,%ymm2
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,226,125,88,219                  // vpbroadcastd  %xmm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  65,137,200                          // mov           %ecx,%r8d
  .byte  65,128,224,7                        // and           $0x7,%r8b
  .byte  197,249,239,192                     // vpxor         %xmm0,%xmm0,%xmm0
  .byte  65,254,200                          // dec           %r8b
  .byte  65,128,248,6                        // cmp           $0x6,%r8b
  .byte  15,135,89,255,255,255               // ja            11c5 <_sk_load_565_hsw+0x14>
  .byte  69,15,182,192                       // movzbl        %r8b,%r8d
  .byte  76,141,13,73,0,0,0                  // lea           0x49(%rip),%r9        # 12c0 <_sk_load_565_hsw+0x10f>
  .byte  75,99,4,129                         // movslq        (%r9,%r8,4),%rax
  .byte  76,1,200                            // add           %r9,%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  197,249,239,192                     // vpxor         %xmm0,%xmm0,%xmm0
  .byte  196,193,121,196,68,122,12,6         // vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,10,5         // vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,8,4          // vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,6,3          // vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,4,2          // vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,2,1          // vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,4,122,0             // vpinsrw       $0x0,(%r10,%rdi,2),%xmm0,%xmm0
  .byte  233,5,255,255,255                   // jmpq          11c5 <_sk_load_565_hsw+0x14>
  .byte  244                                 // hlt
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  236                                 // in            (%dx),%al
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,228                             // jmpq          *%rsp
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  220,255                             // fdivr         %st,%st(7)
  .byte  255                                 // (bad)
  .byte  255,212                             // callq         *%rsp
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,204                             // dec           %esp
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,192                             // inc           %eax
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // .byte         0xff

HIDDEN _sk_store_565_hsw
.globl _sk_store_565_hsw
_sk_store_565_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,8                            // mov           (%rax),%r9
  .byte  184,0,0,248,65                      // mov           $0x41f80000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,89,200                       // vmulps        %ymm0,%ymm8,%ymm9
  .byte  196,65,125,91,201                   // vcvtps2dq     %ymm9,%ymm9
  .byte  196,193,53,114,241,11               // vpslld        $0xb,%ymm9,%ymm9
  .byte  184,0,0,124,66                      // mov           $0x427c0000,%eax
  .byte  197,121,110,208                     // vmovd         %eax,%xmm10
  .byte  196,66,125,88,210                   // vpbroadcastd  %xmm10,%ymm10
  .byte  197,44,89,209                       // vmulps        %ymm1,%ymm10,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,45,114,242,5                // vpslld        $0x5,%ymm10,%ymm10
  .byte  196,65,45,235,201                   // vpor          %ymm9,%ymm10,%ymm9
  .byte  197,60,89,194                       // vmulps        %ymm2,%ymm8,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,65,53,235,192                   // vpor          %ymm8,%ymm9,%ymm8
  .byte  196,67,125,57,193,1                 // vextracti128  $0x1,%ymm8,%xmm9
  .byte  196,66,57,43,193                    // vpackusdw     %xmm9,%xmm8,%xmm8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,10                              // jne           1348 <_sk_store_565_hsw+0x6c>
  .byte  196,65,122,127,4,121                // vmovdqu       %xmm8,(%r9,%rdi,2)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  65,137,200                          // mov           %ecx,%r8d
  .byte  65,128,224,7                        // and           $0x7,%r8b
  .byte  65,254,200                          // dec           %r8b
  .byte  65,128,248,6                        // cmp           $0x6,%r8b
  .byte  119,236                             // ja            1344 <_sk_store_565_hsw+0x68>
  .byte  65,15,182,192                       // movzbl        %r8b,%eax
  .byte  76,141,5,69,0,0,0                   // lea           0x45(%rip),%r8        # 13a8 <_sk_store_565_hsw+0xcc>
  .byte  73,99,4,128                         // movslq        (%r8,%rax,4),%rax
  .byte  76,1,192                            // add           %r8,%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  196,67,121,21,68,121,12,6           // vpextrw       $0x6,%xmm8,0xc(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,10,5           // vpextrw       $0x5,%xmm8,0xa(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,8,4            // vpextrw       $0x4,%xmm8,0x8(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,6,3            // vpextrw       $0x3,%xmm8,0x6(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,4,2            // vpextrw       $0x2,%xmm8,0x4(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,2,1            // vpextrw       $0x1,%xmm8,0x2(%r9,%rdi,2)
  .byte  196,67,121,21,4,121,0               // vpextrw       $0x0,%xmm8,(%r9,%rdi,2)
  .byte  235,159                             // jmp           1344 <_sk_store_565_hsw+0x68>
  .byte  15,31,0                             // nopl          (%rax)
  .byte  244                                 // hlt
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  236                                 // in            (%dx),%al
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,228                             // jmpq          *%rsp
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  220,255                             // fdivr         %st,%st(7)
  .byte  255                                 // (bad)
  .byte  255,212                             // callq         *%rsp
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,204                             // dec           %esp
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,196                             // inc           %esp
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // .byte         0xff

HIDDEN _sk_load_4444_hsw
.globl _sk_load_4444_hsw
_sk_load_4444_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,16                           // mov           (%rax),%r10
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,179,0,0,0                    // jne           1485 <_sk_load_4444_hsw+0xc1>
  .byte  196,193,122,111,4,122               // vmovdqu       (%r10,%rdi,2),%xmm0
  .byte  196,98,125,51,200                   // vpmovzxwd     %xmm0,%ymm9
  .byte  184,0,240,0,0                       // mov           $0xf000,%eax
  .byte  197,249,110,192                     // vmovd         %eax,%xmm0
  .byte  196,226,125,88,192                  // vpbroadcastd  %xmm0,%ymm0
  .byte  196,193,125,219,193                 // vpand         %ymm9,%ymm0,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  184,137,136,136,55                  // mov           $0x37888889,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  196,226,125,88,201                  // vpbroadcastd  %xmm1,%ymm1
  .byte  197,252,89,193                      // vmulps        %ymm1,%ymm0,%ymm0
  .byte  184,0,15,0,0                        // mov           $0xf00,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  196,226,125,88,201                  // vpbroadcastd  %xmm1,%ymm1
  .byte  196,193,117,219,201                 // vpand         %ymm9,%ymm1,%ymm1
  .byte  197,252,91,201                      // vcvtdq2ps     %ymm1,%ymm1
  .byte  184,137,136,136,57                  // mov           $0x39888889,%eax
  .byte  197,249,110,208                     // vmovd         %eax,%xmm2
  .byte  196,226,125,88,210                  // vpbroadcastd  %xmm2,%ymm2
  .byte  197,244,89,202                      // vmulps        %ymm2,%ymm1,%ymm1
  .byte  184,240,0,0,0                       // mov           $0xf0,%eax
  .byte  197,249,110,208                     // vmovd         %eax,%xmm2
  .byte  196,226,125,88,210                  // vpbroadcastd  %xmm2,%ymm2
  .byte  196,193,109,219,209                 // vpand         %ymm9,%ymm2,%ymm2
  .byte  197,124,91,194                      // vcvtdq2ps     %ymm2,%ymm8
  .byte  184,137,136,136,59                  // mov           $0x3b888889,%eax
  .byte  197,249,110,208                     // vmovd         %eax,%xmm2
  .byte  196,226,125,88,210                  // vpbroadcastd  %xmm2,%ymm2
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  184,15,0,0,0                        // mov           $0xf,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,226,125,88,219                  // vpbroadcastd  %xmm3,%ymm3
  .byte  196,193,101,219,217                 // vpand         %ymm9,%ymm3,%ymm3
  .byte  197,124,91,195                      // vcvtdq2ps     %ymm3,%ymm8
  .byte  184,137,136,136,61                  // mov           $0x3d888889,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,226,125,88,219                  // vpbroadcastd  %xmm3,%ymm3
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  65,137,200                          // mov           %ecx,%r8d
  .byte  65,128,224,7                        // and           $0x7,%r8b
  .byte  197,249,239,192                     // vpxor         %xmm0,%xmm0,%xmm0
  .byte  65,254,200                          // dec           %r8b
  .byte  65,128,248,6                        // cmp           $0x6,%r8b
  .byte  15,135,59,255,255,255               // ja            13d8 <_sk_load_4444_hsw+0x14>
  .byte  69,15,182,192                       // movzbl        %r8b,%r8d
  .byte  76,141,13,76,0,0,0                  // lea           0x4c(%rip),%r9        # 14f4 <_sk_load_4444_hsw+0x130>
  .byte  75,99,4,129                         // movslq        (%r9,%r8,4),%rax
  .byte  76,1,200                            // add           %r9,%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  197,249,239,192                     // vpxor         %xmm0,%xmm0,%xmm0
  .byte  196,193,121,196,68,122,12,6         // vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,10,5         // vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,8,4          // vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,6,3          // vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,4,2          // vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,2,1          // vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,4,122,0             // vpinsrw       $0x0,(%r10,%rdi,2),%xmm0,%xmm0
  .byte  233,231,254,255,255                 // jmpq          13d8 <_sk_load_4444_hsw+0x14>
  .byte  15,31,0                             // nopl          (%rax)
  .byte  241                                 // icebp
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  233,255,255,255,225                 // jmpq          ffffffffe20014fc <_sk_linear_gradient_2stops_hsw+0xffffffffe1fff7f0>
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  217,255                             // fcos
  .byte  255                                 // (bad)
  .byte  255,209                             // callq         *%rcx
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,201                             // dec           %ecx
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  189                                 // .byte         0xbd
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // .byte         0xff

HIDDEN _sk_store_4444_hsw
.globl _sk_store_4444_hsw
_sk_store_4444_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,8                            // mov           (%rax),%r9
  .byte  184,0,0,112,65                      // mov           $0x41700000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,89,200                       // vmulps        %ymm0,%ymm8,%ymm9
  .byte  196,65,125,91,201                   // vcvtps2dq     %ymm9,%ymm9
  .byte  196,193,53,114,241,12               // vpslld        $0xc,%ymm9,%ymm9
  .byte  197,60,89,209                       // vmulps        %ymm1,%ymm8,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,45,114,242,8                // vpslld        $0x8,%ymm10,%ymm10
  .byte  196,65,45,235,201                   // vpor          %ymm9,%ymm10,%ymm9
  .byte  197,60,89,210                       // vmulps        %ymm2,%ymm8,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,45,114,242,4                // vpslld        $0x4,%ymm10,%ymm10
  .byte  197,60,89,195                       // vmulps        %ymm3,%ymm8,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,65,45,235,192                   // vpor          %ymm8,%ymm10,%ymm8
  .byte  196,65,53,235,192                   // vpor          %ymm8,%ymm9,%ymm8
  .byte  196,67,125,57,193,1                 // vextracti128  $0x1,%ymm8,%xmm9
  .byte  196,66,57,43,193                    // vpackusdw     %xmm9,%xmm8,%xmm8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,10                              // jne           1582 <_sk_store_4444_hsw+0x72>
  .byte  196,65,122,127,4,121                // vmovdqu       %xmm8,(%r9,%rdi,2)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  65,137,200                          // mov           %ecx,%r8d
  .byte  65,128,224,7                        // and           $0x7,%r8b
  .byte  65,254,200                          // dec           %r8b
  .byte  65,128,248,6                        // cmp           $0x6,%r8b
  .byte  119,236                             // ja            157e <_sk_store_4444_hsw+0x6e>
  .byte  65,15,182,192                       // movzbl        %r8b,%eax
  .byte  76,141,5,67,0,0,0                   // lea           0x43(%rip),%r8        # 15e0 <_sk_store_4444_hsw+0xd0>
  .byte  73,99,4,128                         // movslq        (%r8,%rax,4),%rax
  .byte  76,1,192                            // add           %r8,%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  196,67,121,21,68,121,12,6           // vpextrw       $0x6,%xmm8,0xc(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,10,5           // vpextrw       $0x5,%xmm8,0xa(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,8,4            // vpextrw       $0x4,%xmm8,0x8(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,6,3            // vpextrw       $0x3,%xmm8,0x6(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,4,2            // vpextrw       $0x2,%xmm8,0x4(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,2,1            // vpextrw       $0x1,%xmm8,0x2(%r9,%rdi,2)
  .byte  196,67,121,21,4,121,0               // vpextrw       $0x0,%xmm8,(%r9,%rdi,2)
  .byte  235,159                             // jmp           157e <_sk_store_4444_hsw+0x6e>
  .byte  144                                 // nop
  .byte  246,255                             // idiv          %bh
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  238                                 // out           %al,(%dx)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,230                             // jmpq          *%rsi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  222,255                             // fdivrp        %st,%st(7)
  .byte  255                                 // (bad)
  .byte  255,214                             // callq         *%rsi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,206                             // dec           %esi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,198                             // inc           %esi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // .byte         0xff

HIDDEN _sk_load_8888_hsw
.globl _sk_load_8888_hsw
_sk_load_8888_hsw:
  .byte  73,137,200                          // mov           %rcx,%r8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,141,12,189,0,0,0,0               // lea           0x0(,%rdi,4),%r9
  .byte  76,3,8                              // add           (%rax),%r9
  .byte  77,133,192                          // test          %r8,%r8
  .byte  117,104                             // jne           1679 <_sk_load_8888_hsw+0x7d>
  .byte  196,193,126,111,25                  // vmovdqu       (%r9),%ymm3
  .byte  184,255,0,0,0                       // mov           $0xff,%eax
  .byte  197,249,110,192                     // vmovd         %eax,%xmm0
  .byte  196,226,125,88,208                  // vpbroadcastd  %xmm0,%ymm2
  .byte  197,237,219,195                     // vpand         %ymm3,%ymm2,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  196,98,125,88,193                   // vpbroadcastd  %xmm1,%ymm8
  .byte  196,193,124,89,192                  // vmulps        %ymm8,%ymm0,%ymm0
  .byte  197,245,114,211,8                   // vpsrld        $0x8,%ymm3,%ymm1
  .byte  197,237,219,201                     // vpand         %ymm1,%ymm2,%ymm1
  .byte  197,252,91,201                      // vcvtdq2ps     %ymm1,%ymm1
  .byte  196,193,116,89,200                  // vmulps        %ymm8,%ymm1,%ymm1
  .byte  197,181,114,211,16                  // vpsrld        $0x10,%ymm3,%ymm9
  .byte  196,193,109,219,209                 // vpand         %ymm9,%ymm2,%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  196,193,108,89,208                  // vmulps        %ymm8,%ymm2,%ymm2
  .byte  197,229,114,211,24                  // vpsrld        $0x18,%ymm3,%ymm3
  .byte  197,252,91,219                      // vcvtdq2ps     %ymm3,%ymm3
  .byte  196,193,100,89,216                  // vmulps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,137,193                          // mov           %r8,%rcx
  .byte  255,224                             // jmpq          *%rax
  .byte  185,8,0,0,0                         // mov           $0x8,%ecx
  .byte  68,41,193                           // sub           %r8d,%ecx
  .byte  192,225,3                           // shl           $0x3,%cl
  .byte  72,199,192,255,255,255,255          // mov           $0xffffffffffffffff,%rax
  .byte  72,211,232                          // shr           %cl,%rax
  .byte  196,225,249,110,192                 // vmovq         %rax,%xmm0
  .byte  196,226,125,33,192                  // vpmovsxbd     %xmm0,%ymm0
  .byte  196,194,125,140,25                  // vpmaskmovd    (%r9),%ymm0,%ymm3
  .byte  233,116,255,255,255                 // jmpq          1616 <_sk_load_8888_hsw+0x1a>

HIDDEN _sk_store_8888_hsw
.globl _sk_store_8888_hsw
_sk_store_8888_hsw:
  .byte  73,137,200                          // mov           %rcx,%r8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,141,12,189,0,0,0,0               // lea           0x0(,%rdi,4),%r9
  .byte  76,3,8                              // add           (%rax),%r9
  .byte  184,0,0,127,67                      // mov           $0x437f0000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,66,125,88,192                   // vpbroadcastd  %xmm8,%ymm8
  .byte  197,60,89,200                       // vmulps        %ymm0,%ymm8,%ymm9
  .byte  196,65,125,91,201                   // vcvtps2dq     %ymm9,%ymm9
  .byte  197,60,89,209                       // vmulps        %ymm1,%ymm8,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,45,114,242,8                // vpslld        $0x8,%ymm10,%ymm10
  .byte  196,65,45,235,201                   // vpor          %ymm9,%ymm10,%ymm9
  .byte  197,60,89,210                       // vmulps        %ymm2,%ymm8,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,45,114,242,16               // vpslld        $0x10,%ymm10,%ymm10
  .byte  197,60,89,195                       // vmulps        %ymm3,%ymm8,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,193,61,114,240,24               // vpslld        $0x18,%ymm8,%ymm8
  .byte  196,65,45,235,192                   // vpor          %ymm8,%ymm10,%ymm8
  .byte  196,65,53,235,192                   // vpor          %ymm8,%ymm9,%ymm8
  .byte  77,133,192                          // test          %r8,%r8
  .byte  117,12                              // jne           1716 <_sk_store_8888_hsw+0x74>
  .byte  196,65,126,127,1                    // vmovdqu       %ymm8,(%r9)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,137,193                          // mov           %r8,%rcx
  .byte  255,224                             // jmpq          *%rax
  .byte  185,8,0,0,0                         // mov           $0x8,%ecx
  .byte  68,41,193                           // sub           %r8d,%ecx
  .byte  192,225,3                           // shl           $0x3,%cl
  .byte  72,199,192,255,255,255,255          // mov           $0xffffffffffffffff,%rax
  .byte  72,211,232                          // shr           %cl,%rax
  .byte  196,97,249,110,200                  // vmovq         %rax,%xmm9
  .byte  196,66,125,33,201                   // vpmovsxbd     %xmm9,%ymm9
  .byte  196,66,53,142,1                     // vpmaskmovd    %ymm8,%ymm9,(%r9)
  .byte  235,211                             // jmp           170f <_sk_store_8888_hsw+0x6d>

HIDDEN _sk_load_f16_hsw
.globl _sk_load_f16_hsw
_sk_load_f16_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,97                              // jne           17a7 <_sk_load_f16_hsw+0x6b>
  .byte  197,121,16,4,248                    // vmovupd       (%rax,%rdi,8),%xmm8
  .byte  197,249,16,84,248,16                // vmovupd       0x10(%rax,%rdi,8),%xmm2
  .byte  197,249,16,92,248,32                // vmovupd       0x20(%rax,%rdi,8),%xmm3
  .byte  197,122,111,76,248,48               // vmovdqu       0x30(%rax,%rdi,8),%xmm9
  .byte  197,185,97,194                      // vpunpcklwd    %xmm2,%xmm8,%xmm0
  .byte  197,185,105,210                     // vpunpckhwd    %xmm2,%xmm8,%xmm2
  .byte  196,193,97,97,201                   // vpunpcklwd    %xmm9,%xmm3,%xmm1
  .byte  196,193,97,105,217                  // vpunpckhwd    %xmm9,%xmm3,%xmm3
  .byte  197,121,97,194                      // vpunpcklwd    %xmm2,%xmm0,%xmm8
  .byte  197,121,105,202                     // vpunpckhwd    %xmm2,%xmm0,%xmm9
  .byte  197,241,97,211                      // vpunpcklwd    %xmm3,%xmm1,%xmm2
  .byte  197,241,105,219                     // vpunpckhwd    %xmm3,%xmm1,%xmm3
  .byte  197,185,108,194                     // vpunpcklqdq   %xmm2,%xmm8,%xmm0
  .byte  196,226,125,19,192                  // vcvtph2ps     %xmm0,%ymm0
  .byte  197,185,109,202                     // vpunpckhqdq   %xmm2,%xmm8,%xmm1
  .byte  196,226,125,19,201                  // vcvtph2ps     %xmm1,%ymm1
  .byte  197,177,108,211                     // vpunpcklqdq   %xmm3,%xmm9,%xmm2
  .byte  196,226,125,19,210                  // vcvtph2ps     %xmm2,%ymm2
  .byte  197,177,109,219                     // vpunpckhqdq   %xmm3,%xmm9,%xmm3
  .byte  196,226,125,19,219                  // vcvtph2ps     %xmm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  197,123,16,4,248                    // vmovsd        (%rax,%rdi,8),%xmm8
  .byte  196,65,49,239,201                   // vpxor         %xmm9,%xmm9,%xmm9
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,79                              // je            1806 <_sk_load_f16_hsw+0xca>
  .byte  197,57,22,68,248,8                  // vmovhpd       0x8(%rax,%rdi,8),%xmm8,%xmm8
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  114,67                              // jb            1806 <_sk_load_f16_hsw+0xca>
  .byte  197,251,16,84,248,16                // vmovsd        0x10(%rax,%rdi,8),%xmm2
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  116,68                              // je            1813 <_sk_load_f16_hsw+0xd7>
  .byte  197,233,22,84,248,24                // vmovhpd       0x18(%rax,%rdi,8),%xmm2,%xmm2
  .byte  72,131,249,5                        // cmp           $0x5,%rcx
  .byte  114,56                              // jb            1813 <_sk_load_f16_hsw+0xd7>
  .byte  197,251,16,92,248,32                // vmovsd        0x20(%rax,%rdi,8),%xmm3
  .byte  72,131,249,5                        // cmp           $0x5,%rcx
  .byte  15,132,114,255,255,255              // je            175d <_sk_load_f16_hsw+0x21>
  .byte  197,225,22,92,248,40                // vmovhpd       0x28(%rax,%rdi,8),%xmm3,%xmm3
  .byte  72,131,249,7                        // cmp           $0x7,%rcx
  .byte  15,130,98,255,255,255               // jb            175d <_sk_load_f16_hsw+0x21>
  .byte  197,122,126,76,248,48               // vmovq         0x30(%rax,%rdi,8),%xmm9
  .byte  233,87,255,255,255                  // jmpq          175d <_sk_load_f16_hsw+0x21>
  .byte  197,225,87,219                      // vxorpd        %xmm3,%xmm3,%xmm3
  .byte  197,233,87,210                      // vxorpd        %xmm2,%xmm2,%xmm2
  .byte  233,74,255,255,255                  // jmpq          175d <_sk_load_f16_hsw+0x21>
  .byte  197,225,87,219                      // vxorpd        %xmm3,%xmm3,%xmm3
  .byte  233,65,255,255,255                  // jmpq          175d <_sk_load_f16_hsw+0x21>

HIDDEN _sk_store_f16_hsw
.globl _sk_store_f16_hsw
_sk_store_f16_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,195,125,29,192,4                // vcvtps2ph     $0x4,%ymm0,%xmm8
  .byte  196,195,125,29,201,4                // vcvtps2ph     $0x4,%ymm1,%xmm9
  .byte  196,195,125,29,210,4                // vcvtps2ph     $0x4,%ymm2,%xmm10
  .byte  196,195,125,29,219,4                // vcvtps2ph     $0x4,%ymm3,%xmm11
  .byte  196,65,57,97,225                    // vpunpcklwd    %xmm9,%xmm8,%xmm12
  .byte  196,65,57,105,193                   // vpunpckhwd    %xmm9,%xmm8,%xmm8
  .byte  196,65,41,97,203                    // vpunpcklwd    %xmm11,%xmm10,%xmm9
  .byte  196,65,41,105,235                   // vpunpckhwd    %xmm11,%xmm10,%xmm13
  .byte  196,65,25,98,217                    // vpunpckldq    %xmm9,%xmm12,%xmm11
  .byte  196,65,25,106,209                   // vpunpckhdq    %xmm9,%xmm12,%xmm10
  .byte  196,65,57,98,205                    // vpunpckldq    %xmm13,%xmm8,%xmm9
  .byte  196,65,57,106,197                   // vpunpckhdq    %xmm13,%xmm8,%xmm8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,27                              // jne           1881 <_sk_store_f16_hsw+0x65>
  .byte  197,120,17,28,248                   // vmovups       %xmm11,(%rax,%rdi,8)
  .byte  197,120,17,84,248,16                // vmovups       %xmm10,0x10(%rax,%rdi,8)
  .byte  197,120,17,76,248,32                // vmovups       %xmm9,0x20(%rax,%rdi,8)
  .byte  197,122,127,68,248,48               // vmovdqu       %xmm8,0x30(%rax,%rdi,8)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  197,121,214,28,248                  // vmovq         %xmm11,(%rax,%rdi,8)
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,241                             // je            187d <_sk_store_f16_hsw+0x61>
  .byte  197,121,23,92,248,8                 // vmovhpd       %xmm11,0x8(%rax,%rdi,8)
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  114,229                             // jb            187d <_sk_store_f16_hsw+0x61>
  .byte  197,121,214,84,248,16               // vmovq         %xmm10,0x10(%rax,%rdi,8)
  .byte  116,221                             // je            187d <_sk_store_f16_hsw+0x61>
  .byte  197,121,23,84,248,24                // vmovhpd       %xmm10,0x18(%rax,%rdi,8)
  .byte  72,131,249,5                        // cmp           $0x5,%rcx
  .byte  114,209                             // jb            187d <_sk_store_f16_hsw+0x61>
  .byte  197,121,214,76,248,32               // vmovq         %xmm9,0x20(%rax,%rdi,8)
  .byte  116,201                             // je            187d <_sk_store_f16_hsw+0x61>
  .byte  197,121,23,76,248,40                // vmovhpd       %xmm9,0x28(%rax,%rdi,8)
  .byte  72,131,249,7                        // cmp           $0x7,%rcx
  .byte  114,189                             // jb            187d <_sk_store_f16_hsw+0x61>
  .byte  197,121,214,68,248,48               // vmovq         %xmm8,0x30(%rax,%rdi,8)
  .byte  235,181                             // jmp           187d <_sk_store_f16_hsw+0x61>

HIDDEN _sk_store_f32_hsw
.globl _sk_store_f32_hsw
_sk_store_f32_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  72,141,4,189,0,0,0,0                // lea           0x0(,%rdi,4),%rax
  .byte  197,124,20,193                      // vunpcklps     %ymm1,%ymm0,%ymm8
  .byte  197,124,21,217                      // vunpckhps     %ymm1,%ymm0,%ymm11
  .byte  197,108,20,203                      // vunpcklps     %ymm3,%ymm2,%ymm9
  .byte  197,108,21,227                      // vunpckhps     %ymm3,%ymm2,%ymm12
  .byte  196,65,61,20,209                    // vunpcklpd     %ymm9,%ymm8,%ymm10
  .byte  196,65,61,21,201                    // vunpckhpd     %ymm9,%ymm8,%ymm9
  .byte  196,65,37,20,196                    // vunpcklpd     %ymm12,%ymm11,%ymm8
  .byte  196,65,37,21,220                    // vunpckhpd     %ymm12,%ymm11,%ymm11
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,55                              // jne           1935 <_sk_store_f32_hsw+0x6d>
  .byte  196,67,45,24,225,1                  // vinsertf128   $0x1,%xmm9,%ymm10,%ymm12
  .byte  196,67,61,24,235,1                  // vinsertf128   $0x1,%xmm11,%ymm8,%ymm13
  .byte  196,67,45,6,201,49                  // vperm2f128    $0x31,%ymm9,%ymm10,%ymm9
  .byte  196,67,61,6,195,49                  // vperm2f128    $0x31,%ymm11,%ymm8,%ymm8
  .byte  196,65,125,17,36,128                // vmovupd       %ymm12,(%r8,%rax,4)
  .byte  196,65,125,17,108,128,32            // vmovupd       %ymm13,0x20(%r8,%rax,4)
  .byte  196,65,125,17,76,128,64             // vmovupd       %ymm9,0x40(%r8,%rax,4)
  .byte  196,65,125,17,68,128,96             // vmovupd       %ymm8,0x60(%r8,%rax,4)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  196,65,121,17,20,128                // vmovupd       %xmm10,(%r8,%rax,4)
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,240                             // je            1931 <_sk_store_f32_hsw+0x69>
  .byte  196,65,121,17,76,128,16             // vmovupd       %xmm9,0x10(%r8,%rax,4)
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  114,227                             // jb            1931 <_sk_store_f32_hsw+0x69>
  .byte  196,65,121,17,68,128,32             // vmovupd       %xmm8,0x20(%r8,%rax,4)
  .byte  116,218                             // je            1931 <_sk_store_f32_hsw+0x69>
  .byte  196,65,121,17,92,128,48             // vmovupd       %xmm11,0x30(%r8,%rax,4)
  .byte  72,131,249,5                        // cmp           $0x5,%rcx
  .byte  114,205                             // jb            1931 <_sk_store_f32_hsw+0x69>
  .byte  196,67,125,25,84,128,64,1           // vextractf128  $0x1,%ymm10,0x40(%r8,%rax,4)
  .byte  116,195                             // je            1931 <_sk_store_f32_hsw+0x69>
  .byte  196,67,125,25,76,128,80,1           // vextractf128  $0x1,%ymm9,0x50(%r8,%rax,4)
  .byte  72,131,249,7                        // cmp           $0x7,%rcx
  .byte  114,181                             // jb            1931 <_sk_store_f32_hsw+0x69>
  .byte  196,67,125,25,68,128,96,1           // vextractf128  $0x1,%ymm8,0x60(%r8,%rax,4)
  .byte  235,171                             // jmp           1931 <_sk_store_f32_hsw+0x69>

HIDDEN _sk_clamp_x_hsw
.globl _sk_clamp_x_hsw
_sk_clamp_x_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  197,188,95,192                      // vmaxps        %ymm0,%ymm8,%ymm0
  .byte  196,98,125,88,0                     // vpbroadcastd  (%rax),%ymm8
  .byte  196,65,53,118,201                   // vpcmpeqd      %ymm9,%ymm9,%ymm9
  .byte  196,65,61,254,193                   // vpaddd        %ymm9,%ymm8,%ymm8
  .byte  196,193,124,93,192                  // vminps        %ymm8,%ymm0,%ymm0
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_y_hsw
.globl _sk_clamp_y_hsw
_sk_clamp_y_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  197,188,95,201                      // vmaxps        %ymm1,%ymm8,%ymm1
  .byte  196,98,125,88,0                     // vpbroadcastd  (%rax),%ymm8
  .byte  196,65,53,118,201                   // vpcmpeqd      %ymm9,%ymm9,%ymm9
  .byte  196,65,61,254,193                   // vpaddd        %ymm9,%ymm8,%ymm8
  .byte  196,193,116,93,200                  // vminps        %ymm8,%ymm1,%ymm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_repeat_x_hsw
.globl _sk_repeat_x_hsw
_sk_repeat_x_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  196,65,124,94,200                   // vdivps        %ymm8,%ymm0,%ymm9
  .byte  196,67,125,8,201,1                  // vroundps      $0x1,%ymm9,%ymm9
  .byte  196,98,61,172,200                   // vfnmadd213ps  %ymm0,%ymm8,%ymm9
  .byte  197,253,118,192                     // vpcmpeqd      %ymm0,%ymm0,%ymm0
  .byte  197,189,254,192                     // vpaddd        %ymm0,%ymm8,%ymm0
  .byte  197,180,93,192                      // vminps        %ymm0,%ymm9,%ymm0
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_repeat_y_hsw
.globl _sk_repeat_y_hsw
_sk_repeat_y_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  196,65,116,94,200                   // vdivps        %ymm8,%ymm1,%ymm9
  .byte  196,67,125,8,201,1                  // vroundps      $0x1,%ymm9,%ymm9
  .byte  196,98,61,172,201                   // vfnmadd213ps  %ymm1,%ymm8,%ymm9
  .byte  197,245,118,201                     // vpcmpeqd      %ymm1,%ymm1,%ymm1
  .byte  197,189,254,201                     // vpaddd        %ymm1,%ymm8,%ymm1
  .byte  197,180,93,201                      // vminps        %ymm1,%ymm9,%ymm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_mirror_x_hsw
.globl _sk_mirror_x_hsw
_sk_mirror_x_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,122,16,0                        // vmovss        (%rax),%xmm8
  .byte  196,66,125,24,200                   // vbroadcastss  %xmm8,%ymm9
  .byte  196,65,124,92,209                   // vsubps        %ymm9,%ymm0,%ymm10
  .byte  196,193,58,88,192                   // vaddss        %xmm8,%xmm8,%xmm0
  .byte  196,226,125,24,192                  // vbroadcastss  %xmm0,%ymm0
  .byte  197,44,94,192                       // vdivps        %ymm0,%ymm10,%ymm8
  .byte  196,67,125,8,192,1                  // vroundps      $0x1,%ymm8,%ymm8
  .byte  196,66,125,172,194                  // vfnmadd213ps  %ymm10,%ymm0,%ymm8
  .byte  196,193,60,92,193                   // vsubps        %ymm9,%ymm8,%ymm0
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  197,60,92,192                       // vsubps        %ymm0,%ymm8,%ymm8
  .byte  197,188,84,192                      // vandps        %ymm0,%ymm8,%ymm0
  .byte  196,65,61,118,192                   // vpcmpeqd      %ymm8,%ymm8,%ymm8
  .byte  196,65,53,254,192                   // vpaddd        %ymm8,%ymm9,%ymm8
  .byte  196,193,124,93,192                  // vminps        %ymm8,%ymm0,%ymm0
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_mirror_y_hsw
.globl _sk_mirror_y_hsw
_sk_mirror_y_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,122,16,0                        // vmovss        (%rax),%xmm8
  .byte  196,66,125,24,200                   // vbroadcastss  %xmm8,%ymm9
  .byte  196,65,116,92,209                   // vsubps        %ymm9,%ymm1,%ymm10
  .byte  196,193,58,88,200                   // vaddss        %xmm8,%xmm8,%xmm1
  .byte  196,226,125,24,201                  // vbroadcastss  %xmm1,%ymm1
  .byte  197,44,94,193                       // vdivps        %ymm1,%ymm10,%ymm8
  .byte  196,67,125,8,192,1                  // vroundps      $0x1,%ymm8,%ymm8
  .byte  196,66,117,172,194                  // vfnmadd213ps  %ymm10,%ymm1,%ymm8
  .byte  196,193,60,92,201                   // vsubps        %ymm9,%ymm8,%ymm1
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  197,60,92,193                       // vsubps        %ymm1,%ymm8,%ymm8
  .byte  197,188,84,201                      // vandps        %ymm1,%ymm8,%ymm1
  .byte  196,65,61,118,192                   // vpcmpeqd      %ymm8,%ymm8,%ymm8
  .byte  196,65,53,254,192                   // vpaddd        %ymm8,%ymm9,%ymm8
  .byte  196,193,116,93,200                  // vminps        %ymm8,%ymm1,%ymm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_luminance_to_alpha_hsw
.globl _sk_luminance_to_alpha_hsw
_sk_luminance_to_alpha_hsw:
  .byte  184,208,179,89,62                   // mov           $0x3e59b3d0,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,98,125,88,195                   // vpbroadcastd  %xmm3,%ymm8
  .byte  184,89,23,55,63                     // mov           $0x3f371759,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,226,125,88,219                  // vpbroadcastd  %xmm3,%ymm3
  .byte  197,228,89,201                      // vmulps        %ymm1,%ymm3,%ymm1
  .byte  196,98,125,168,193                  // vfmadd213ps   %ymm1,%ymm0,%ymm8
  .byte  184,152,221,147,61                  // mov           $0x3d93dd98,%eax
  .byte  197,249,110,192                     // vmovd         %eax,%xmm0
  .byte  196,226,125,88,216                  // vpbroadcastd  %xmm0,%ymm3
  .byte  196,194,109,168,216                 // vfmadd213ps   %ymm8,%ymm2,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,253,239,192                     // vpxor         %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_matrix_2x3_hsw
.globl _sk_matrix_2x3_hsw
_sk_matrix_2x3_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,8                     // vbroadcastss  (%rax),%ymm9
  .byte  196,98,125,24,80,8                  // vbroadcastss  0x8(%rax),%ymm10
  .byte  196,98,125,24,64,16                 // vbroadcastss  0x10(%rax),%ymm8
  .byte  196,66,117,184,194                  // vfmadd231ps   %ymm10,%ymm1,%ymm8
  .byte  196,66,125,184,193                  // vfmadd231ps   %ymm9,%ymm0,%ymm8
  .byte  196,98,125,24,80,4                  // vbroadcastss  0x4(%rax),%ymm10
  .byte  196,98,125,24,88,12                 // vbroadcastss  0xc(%rax),%ymm11
  .byte  196,98,125,24,72,20                 // vbroadcastss  0x14(%rax),%ymm9
  .byte  196,66,117,184,203                  // vfmadd231ps   %ymm11,%ymm1,%ymm9
  .byte  196,66,125,184,202                  // vfmadd231ps   %ymm10,%ymm0,%ymm9
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,124,41,192                      // vmovaps       %ymm8,%ymm0
  .byte  197,124,41,201                      // vmovaps       %ymm9,%ymm1
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_matrix_3x4_hsw
.globl _sk_matrix_3x4_hsw
_sk_matrix_3x4_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,8                     // vbroadcastss  (%rax),%ymm9
  .byte  196,98,125,24,80,12                 // vbroadcastss  0xc(%rax),%ymm10
  .byte  196,98,125,24,88,24                 // vbroadcastss  0x18(%rax),%ymm11
  .byte  196,98,125,24,64,36                 // vbroadcastss  0x24(%rax),%ymm8
  .byte  196,66,109,184,195                  // vfmadd231ps   %ymm11,%ymm2,%ymm8
  .byte  196,66,117,184,194                  // vfmadd231ps   %ymm10,%ymm1,%ymm8
  .byte  196,66,125,184,193                  // vfmadd231ps   %ymm9,%ymm0,%ymm8
  .byte  196,98,125,24,80,4                  // vbroadcastss  0x4(%rax),%ymm10
  .byte  196,98,125,24,88,16                 // vbroadcastss  0x10(%rax),%ymm11
  .byte  196,98,125,24,96,28                 // vbroadcastss  0x1c(%rax),%ymm12
  .byte  196,98,125,24,72,40                 // vbroadcastss  0x28(%rax),%ymm9
  .byte  196,66,109,184,204                  // vfmadd231ps   %ymm12,%ymm2,%ymm9
  .byte  196,66,117,184,203                  // vfmadd231ps   %ymm11,%ymm1,%ymm9
  .byte  196,66,125,184,202                  // vfmadd231ps   %ymm10,%ymm0,%ymm9
  .byte  196,98,125,24,88,8                  // vbroadcastss  0x8(%rax),%ymm11
  .byte  196,98,125,24,96,20                 // vbroadcastss  0x14(%rax),%ymm12
  .byte  196,98,125,24,104,32                // vbroadcastss  0x20(%rax),%ymm13
  .byte  196,98,125,24,80,44                 // vbroadcastss  0x2c(%rax),%ymm10
  .byte  196,66,109,184,213                  // vfmadd231ps   %ymm13,%ymm2,%ymm10
  .byte  196,66,117,184,212                  // vfmadd231ps   %ymm12,%ymm1,%ymm10
  .byte  196,66,125,184,211                  // vfmadd231ps   %ymm11,%ymm0,%ymm10
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,124,41,192                      // vmovaps       %ymm8,%ymm0
  .byte  197,124,41,201                      // vmovaps       %ymm9,%ymm1
  .byte  197,124,41,210                      // vmovaps       %ymm10,%ymm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_matrix_4x5_hsw
.globl _sk_matrix_4x5_hsw
_sk_matrix_4x5_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,8                     // vbroadcastss  (%rax),%ymm9
  .byte  196,98,125,24,80,16                 // vbroadcastss  0x10(%rax),%ymm10
  .byte  196,98,125,24,88,32                 // vbroadcastss  0x20(%rax),%ymm11
  .byte  196,98,125,24,96,48                 // vbroadcastss  0x30(%rax),%ymm12
  .byte  196,98,125,24,64,64                 // vbroadcastss  0x40(%rax),%ymm8
  .byte  196,66,101,184,196                  // vfmadd231ps   %ymm12,%ymm3,%ymm8
  .byte  196,66,109,184,195                  // vfmadd231ps   %ymm11,%ymm2,%ymm8
  .byte  196,66,117,184,194                  // vfmadd231ps   %ymm10,%ymm1,%ymm8
  .byte  196,66,125,184,193                  // vfmadd231ps   %ymm9,%ymm0,%ymm8
  .byte  196,98,125,24,80,4                  // vbroadcastss  0x4(%rax),%ymm10
  .byte  196,98,125,24,88,20                 // vbroadcastss  0x14(%rax),%ymm11
  .byte  196,98,125,24,96,36                 // vbroadcastss  0x24(%rax),%ymm12
  .byte  196,98,125,24,104,52                // vbroadcastss  0x34(%rax),%ymm13
  .byte  196,98,125,24,72,68                 // vbroadcastss  0x44(%rax),%ymm9
  .byte  196,66,101,184,205                  // vfmadd231ps   %ymm13,%ymm3,%ymm9
  .byte  196,66,109,184,204                  // vfmadd231ps   %ymm12,%ymm2,%ymm9
  .byte  196,66,117,184,203                  // vfmadd231ps   %ymm11,%ymm1,%ymm9
  .byte  196,66,125,184,202                  // vfmadd231ps   %ymm10,%ymm0,%ymm9
  .byte  196,98,125,24,88,8                  // vbroadcastss  0x8(%rax),%ymm11
  .byte  196,98,125,24,96,24                 // vbroadcastss  0x18(%rax),%ymm12
  .byte  196,98,125,24,104,40                // vbroadcastss  0x28(%rax),%ymm13
  .byte  196,98,125,24,112,56                // vbroadcastss  0x38(%rax),%ymm14
  .byte  196,98,125,24,80,72                 // vbroadcastss  0x48(%rax),%ymm10
  .byte  196,66,101,184,214                  // vfmadd231ps   %ymm14,%ymm3,%ymm10
  .byte  196,66,109,184,213                  // vfmadd231ps   %ymm13,%ymm2,%ymm10
  .byte  196,66,117,184,212                  // vfmadd231ps   %ymm12,%ymm1,%ymm10
  .byte  196,66,125,184,211                  // vfmadd231ps   %ymm11,%ymm0,%ymm10
  .byte  196,98,125,24,96,12                 // vbroadcastss  0xc(%rax),%ymm12
  .byte  196,98,125,24,104,28                // vbroadcastss  0x1c(%rax),%ymm13
  .byte  196,98,125,24,112,44                // vbroadcastss  0x2c(%rax),%ymm14
  .byte  196,98,125,24,120,60                // vbroadcastss  0x3c(%rax),%ymm15
  .byte  196,98,125,24,88,76                 // vbroadcastss  0x4c(%rax),%ymm11
  .byte  196,66,101,184,223                  // vfmadd231ps   %ymm15,%ymm3,%ymm11
  .byte  196,66,109,184,222                  // vfmadd231ps   %ymm14,%ymm2,%ymm11
  .byte  196,66,117,184,221                  // vfmadd231ps   %ymm13,%ymm1,%ymm11
  .byte  196,66,125,184,220                  // vfmadd231ps   %ymm12,%ymm0,%ymm11
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,124,41,192                      // vmovaps       %ymm8,%ymm0
  .byte  197,124,41,201                      // vmovaps       %ymm9,%ymm1
  .byte  197,124,41,210                      // vmovaps       %ymm10,%ymm2
  .byte  197,124,41,219                      // vmovaps       %ymm11,%ymm3
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_matrix_perspective_hsw
.globl _sk_matrix_perspective_hsw
_sk_matrix_perspective_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  196,98,125,24,72,4                  // vbroadcastss  0x4(%rax),%ymm9
  .byte  196,98,125,24,80,8                  // vbroadcastss  0x8(%rax),%ymm10
  .byte  196,66,117,184,209                  // vfmadd231ps   %ymm9,%ymm1,%ymm10
  .byte  196,66,125,184,208                  // vfmadd231ps   %ymm8,%ymm0,%ymm10
  .byte  196,98,125,24,64,12                 // vbroadcastss  0xc(%rax),%ymm8
  .byte  196,98,125,24,72,16                 // vbroadcastss  0x10(%rax),%ymm9
  .byte  196,98,125,24,88,20                 // vbroadcastss  0x14(%rax),%ymm11
  .byte  196,66,117,184,217                  // vfmadd231ps   %ymm9,%ymm1,%ymm11
  .byte  196,66,125,184,216                  // vfmadd231ps   %ymm8,%ymm0,%ymm11
  .byte  196,98,125,24,64,24                 // vbroadcastss  0x18(%rax),%ymm8
  .byte  196,98,125,24,72,28                 // vbroadcastss  0x1c(%rax),%ymm9
  .byte  196,98,125,24,96,32                 // vbroadcastss  0x20(%rax),%ymm12
  .byte  196,66,117,184,225                  // vfmadd231ps   %ymm9,%ymm1,%ymm12
  .byte  196,66,125,184,224                  // vfmadd231ps   %ymm8,%ymm0,%ymm12
  .byte  196,193,124,83,204                  // vrcpps        %ymm12,%ymm1
  .byte  197,172,89,193                      // vmulps        %ymm1,%ymm10,%ymm0
  .byte  197,164,89,201                      // vmulps        %ymm1,%ymm11,%ymm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_linear_gradient_2stops_hsw
.globl _sk_linear_gradient_2stops_hsw
_sk_linear_gradient_2stops_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,226,125,24,72,16                // vbroadcastss  0x10(%rax),%ymm1
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  196,98,125,184,193                  // vfmadd231ps   %ymm1,%ymm0,%ymm8
  .byte  196,226,125,24,80,20                // vbroadcastss  0x14(%rax),%ymm2
  .byte  196,226,125,24,72,4                 // vbroadcastss  0x4(%rax),%ymm1
  .byte  196,226,125,184,202                 // vfmadd231ps   %ymm2,%ymm0,%ymm1
  .byte  196,226,125,24,88,24                // vbroadcastss  0x18(%rax),%ymm3
  .byte  196,226,125,24,80,8                 // vbroadcastss  0x8(%rax),%ymm2
  .byte  196,226,125,184,211                 // vfmadd231ps   %ymm3,%ymm0,%ymm2
  .byte  196,98,125,24,72,28                 // vbroadcastss  0x1c(%rax),%ymm9
  .byte  196,226,125,24,88,12                // vbroadcastss  0xc(%rax),%ymm3
  .byte  196,194,125,184,217                 // vfmadd231ps   %ymm9,%ymm0,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,124,41,192                      // vmovaps       %ymm8,%ymm0
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_start_pipeline_avx
.globl _sk_start_pipeline_avx
_sk_start_pipeline_avx:
  .byte  65,87                               // push          %r15
  .byte  65,86                               // push          %r14
  .byte  65,85                               // push          %r13
  .byte  65,84                               // push          %r12
  .byte  83                                  // push          %rbx
  .byte  73,137,205                          // mov           %rcx,%r13
  .byte  73,137,214                          // mov           %rdx,%r14
  .byte  72,137,251                          // mov           %rdi,%rbx
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  73,137,199                          // mov           %rax,%r15
  .byte  73,137,244                          // mov           %rsi,%r12
  .byte  72,141,67,8                         // lea           0x8(%rbx),%rax
  .byte  76,57,232                           // cmp           %r13,%rax
  .byte  118,5                               // jbe           28 <_sk_start_pipeline_avx+0x28>
  .byte  72,137,223                          // mov           %rbx,%rdi
  .byte  235,65                              // jmp           69 <_sk_start_pipeline_avx+0x69>
  .byte  185,0,0,0,0                         // mov           $0x0,%ecx
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  197,228,87,219                      // vxorps        %ymm3,%ymm3,%ymm3
  .byte  197,220,87,228                      // vxorps        %ymm4,%ymm4,%ymm4
  .byte  197,212,87,237                      // vxorps        %ymm5,%ymm5,%ymm5
  .byte  197,204,87,246                      // vxorps        %ymm6,%ymm6,%ymm6
  .byte  197,196,87,255                      // vxorps        %ymm7,%ymm7,%ymm7
  .byte  72,137,223                          // mov           %rbx,%rdi
  .byte  76,137,230                          // mov           %r12,%rsi
  .byte  76,137,242                          // mov           %r14,%rdx
  .byte  65,255,215                          // callq         *%r15
  .byte  72,141,123,8                        // lea           0x8(%rbx),%rdi
  .byte  72,131,195,16                       // add           $0x10,%rbx
  .byte  76,57,235                           // cmp           %r13,%rbx
  .byte  72,137,251                          // mov           %rdi,%rbx
  .byte  118,191                             // jbe           28 <_sk_start_pipeline_avx+0x28>
  .byte  76,137,233                          // mov           %r13,%rcx
  .byte  72,41,249                           // sub           %rdi,%rcx
  .byte  116,41                              // je            9a <_sk_start_pipeline_avx+0x9a>
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  197,228,87,219                      // vxorps        %ymm3,%ymm3,%ymm3
  .byte  197,220,87,228                      // vxorps        %ymm4,%ymm4,%ymm4
  .byte  197,212,87,237                      // vxorps        %ymm5,%ymm5,%ymm5
  .byte  197,204,87,246                      // vxorps        %ymm6,%ymm6,%ymm6
  .byte  197,196,87,255                      // vxorps        %ymm7,%ymm7,%ymm7
  .byte  76,137,230                          // mov           %r12,%rsi
  .byte  76,137,242                          // mov           %r14,%rdx
  .byte  65,255,215                          // callq         *%r15
  .byte  76,137,232                          // mov           %r13,%rax
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,93                               // pop           %r13
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  197,248,119                         // vzeroupper
  .byte  195                                 // retq

HIDDEN _sk_just_return_avx
.globl _sk_just_return_avx
_sk_just_return_avx:
  .byte  195                                 // retq

HIDDEN _sk_seed_shader_avx
.globl _sk_seed_shader_avx
_sk_seed_shader_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,249,110,199                     // vmovd         %edi,%xmm0
  .byte  197,249,112,192,0                   // vpshufd       $0x0,%xmm0,%xmm0
  .byte  196,227,125,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  65,184,0,0,0,63                     // mov           $0x3f000000,%r8d
  .byte  196,193,121,110,200                 // vmovd         %r8d,%xmm1
  .byte  196,227,121,4,201,0                 // vpermilps     $0x0,%xmm1,%xmm1
  .byte  196,227,117,24,201,1                // vinsertf128   $0x1,%xmm1,%ymm1,%ymm1
  .byte  197,252,88,193                      // vaddps        %ymm1,%ymm0,%ymm0
  .byte  197,252,88,2                        // vaddps        (%rdx),%ymm0,%ymm0
  .byte  196,226,125,24,16                   // vbroadcastss  (%rax),%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  197,236,88,201                      // vaddps        %ymm1,%ymm2,%ymm1
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,249,110,208                     // vmovd         %eax,%xmm2
  .byte  196,227,121,4,210,0                 // vpermilps     $0x0,%xmm2,%xmm2
  .byte  196,227,109,24,210,1                // vinsertf128   $0x1,%xmm2,%ymm2,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,228,87,219                      // vxorps        %ymm3,%ymm3,%ymm3
  .byte  197,220,87,228                      // vxorps        %ymm4,%ymm4,%ymm4
  .byte  197,212,87,237                      // vxorps        %ymm5,%ymm5,%ymm5
  .byte  197,204,87,246                      // vxorps        %ymm6,%ymm6,%ymm6
  .byte  197,196,87,255                      // vxorps        %ymm7,%ymm7,%ymm7
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_constant_color_avx
.globl _sk_constant_color_avx
_sk_constant_color_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,226,125,24,0                    // vbroadcastss  (%rax),%ymm0
  .byte  196,226,125,24,72,4                 // vbroadcastss  0x4(%rax),%ymm1
  .byte  196,226,125,24,80,8                 // vbroadcastss  0x8(%rax),%ymm2
  .byte  196,226,125,24,88,12                // vbroadcastss  0xc(%rax),%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clear_avx
.globl _sk_clear_avx
_sk_clear_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  197,228,87,219                      // vxorps        %ymm3,%ymm3,%ymm3
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_srcatop_avx
.globl _sk_srcatop_avx
_sk_srcatop_avx:
  .byte  197,124,89,199                      // vmulps        %ymm7,%ymm0,%ymm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,249,110,192                     // vmovd         %eax,%xmm0
  .byte  196,227,121,4,192,0                 // vpermilps     $0x0,%xmm0,%xmm0
  .byte  196,227,125,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  .byte  197,124,92,203                      // vsubps        %ymm3,%ymm0,%ymm9
  .byte  197,180,89,196                      // vmulps        %ymm4,%ymm9,%ymm0
  .byte  197,188,88,192                      // vaddps        %ymm0,%ymm8,%ymm0
  .byte  197,244,89,207                      // vmulps        %ymm7,%ymm1,%ymm1
  .byte  197,52,89,197                       // vmulps        %ymm5,%ymm9,%ymm8
  .byte  196,193,116,88,200                  // vaddps        %ymm8,%ymm1,%ymm1
  .byte  197,236,89,215                      // vmulps        %ymm7,%ymm2,%ymm2
  .byte  197,52,89,198                       // vmulps        %ymm6,%ymm9,%ymm8
  .byte  196,193,108,88,208                  // vaddps        %ymm8,%ymm2,%ymm2
  .byte  197,228,89,223                      // vmulps        %ymm7,%ymm3,%ymm3
  .byte  197,52,89,199                       // vmulps        %ymm7,%ymm9,%ymm8
  .byte  196,193,100,88,216                  // vaddps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_dstatop_avx
.globl _sk_dstatop_avx
_sk_dstatop_avx:
  .byte  197,100,89,196                      // vmulps        %ymm4,%ymm3,%ymm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,200                     // vmovd         %eax,%xmm9
  .byte  196,67,121,4,201,0                  // vpermilps     $0x0,%xmm9,%xmm9
  .byte  196,67,53,24,201,1                  // vinsertf128   $0x1,%xmm9,%ymm9,%ymm9
  .byte  197,52,92,207                       // vsubps        %ymm7,%ymm9,%ymm9
  .byte  197,180,89,192                      // vmulps        %ymm0,%ymm9,%ymm0
  .byte  197,188,88,192                      // vaddps        %ymm0,%ymm8,%ymm0
  .byte  197,100,89,197                      // vmulps        %ymm5,%ymm3,%ymm8
  .byte  197,180,89,201                      // vmulps        %ymm1,%ymm9,%ymm1
  .byte  197,188,88,201                      // vaddps        %ymm1,%ymm8,%ymm1
  .byte  197,100,89,198                      // vmulps        %ymm6,%ymm3,%ymm8
  .byte  197,180,89,210                      // vmulps        %ymm2,%ymm9,%ymm2
  .byte  197,188,88,210                      // vaddps        %ymm2,%ymm8,%ymm2
  .byte  197,100,89,199                      // vmulps        %ymm7,%ymm3,%ymm8
  .byte  197,180,89,219                      // vmulps        %ymm3,%ymm9,%ymm3
  .byte  197,188,88,219                      // vaddps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_srcin_avx
.globl _sk_srcin_avx
_sk_srcin_avx:
  .byte  197,252,89,199                      // vmulps        %ymm7,%ymm0,%ymm0
  .byte  197,244,89,207                      // vmulps        %ymm7,%ymm1,%ymm1
  .byte  197,236,89,215                      // vmulps        %ymm7,%ymm2,%ymm2
  .byte  197,228,89,223                      // vmulps        %ymm7,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_dstin_avx
.globl _sk_dstin_avx
_sk_dstin_avx:
  .byte  197,228,89,196                      // vmulps        %ymm4,%ymm3,%ymm0
  .byte  197,228,89,205                      // vmulps        %ymm5,%ymm3,%ymm1
  .byte  197,228,89,214                      // vmulps        %ymm6,%ymm3,%ymm2
  .byte  197,228,89,223                      // vmulps        %ymm7,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_srcout_avx
.globl _sk_srcout_avx
_sk_srcout_avx:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,92,199                       // vsubps        %ymm7,%ymm8,%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_dstout_avx
.globl _sk_dstout_avx
_sk_dstout_avx:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,249,110,192                     // vmovd         %eax,%xmm0
  .byte  196,227,121,4,192,0                 // vpermilps     $0x0,%xmm0,%xmm0
  .byte  196,227,125,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  .byte  197,252,92,219                      // vsubps        %ymm3,%ymm0,%ymm3
  .byte  197,228,89,196                      // vmulps        %ymm4,%ymm3,%ymm0
  .byte  197,228,89,205                      // vmulps        %ymm5,%ymm3,%ymm1
  .byte  197,228,89,214                      // vmulps        %ymm6,%ymm3,%ymm2
  .byte  197,228,89,223                      // vmulps        %ymm7,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_srcover_avx
.globl _sk_srcover_avx
_sk_srcover_avx:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,204                       // vmulps        %ymm4,%ymm8,%ymm9
  .byte  197,180,88,192                      // vaddps        %ymm0,%ymm9,%ymm0
  .byte  197,60,89,205                       // vmulps        %ymm5,%ymm8,%ymm9
  .byte  197,180,88,201                      // vaddps        %ymm1,%ymm9,%ymm1
  .byte  197,60,89,206                       // vmulps        %ymm6,%ymm8,%ymm9
  .byte  197,180,88,210                      // vaddps        %ymm2,%ymm9,%ymm2
  .byte  197,60,89,199                       // vmulps        %ymm7,%ymm8,%ymm8
  .byte  197,188,88,219                      // vaddps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_dstover_avx
.globl _sk_dstover_avx
_sk_dstover_avx:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,92,199                       // vsubps        %ymm7,%ymm8,%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,252,88,196                      // vaddps        %ymm4,%ymm0,%ymm0
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,244,88,205                      // vaddps        %ymm5,%ymm1,%ymm1
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  197,236,88,214                      // vaddps        %ymm6,%ymm2,%ymm2
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  197,228,88,223                      // vaddps        %ymm7,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_modulate_avx
.globl _sk_modulate_avx
_sk_modulate_avx:
  .byte  197,252,89,196                      // vmulps        %ymm4,%ymm0,%ymm0
  .byte  197,244,89,205                      // vmulps        %ymm5,%ymm1,%ymm1
  .byte  197,236,89,214                      // vmulps        %ymm6,%ymm2,%ymm2
  .byte  197,228,89,223                      // vmulps        %ymm7,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_multiply_avx
.globl _sk_multiply_avx
_sk_multiply_avx:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,92,207                       // vsubps        %ymm7,%ymm8,%ymm9
  .byte  197,52,89,208                       // vmulps        %ymm0,%ymm9,%ymm10
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,220                       // vmulps        %ymm4,%ymm8,%ymm11
  .byte  196,65,44,88,211                    // vaddps        %ymm11,%ymm10,%ymm10
  .byte  197,252,89,196                      // vmulps        %ymm4,%ymm0,%ymm0
  .byte  196,193,124,88,194                  // vaddps        %ymm10,%ymm0,%ymm0
  .byte  197,52,89,209                       // vmulps        %ymm1,%ymm9,%ymm10
  .byte  197,60,89,221                       // vmulps        %ymm5,%ymm8,%ymm11
  .byte  196,65,36,88,210                    // vaddps        %ymm10,%ymm11,%ymm10
  .byte  197,244,89,205                      // vmulps        %ymm5,%ymm1,%ymm1
  .byte  196,193,116,88,202                  // vaddps        %ymm10,%ymm1,%ymm1
  .byte  197,52,89,210                       // vmulps        %ymm2,%ymm9,%ymm10
  .byte  197,60,89,222                       // vmulps        %ymm6,%ymm8,%ymm11
  .byte  196,65,36,88,210                    // vaddps        %ymm10,%ymm11,%ymm10
  .byte  197,236,89,214                      // vmulps        %ymm6,%ymm2,%ymm2
  .byte  196,193,108,88,210                  // vaddps        %ymm10,%ymm2,%ymm2
  .byte  197,52,89,203                       // vmulps        %ymm3,%ymm9,%ymm9
  .byte  197,60,89,199                       // vmulps        %ymm7,%ymm8,%ymm8
  .byte  196,65,60,88,193                    // vaddps        %ymm9,%ymm8,%ymm8
  .byte  197,228,89,223                      // vmulps        %ymm7,%ymm3,%ymm3
  .byte  196,193,100,88,216                  // vaddps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_plus__avx
.globl _sk_plus__avx
_sk_plus__avx:
  .byte  197,252,88,196                      // vaddps        %ymm4,%ymm0,%ymm0
  .byte  197,244,88,205                      // vaddps        %ymm5,%ymm1,%ymm1
  .byte  197,236,88,214                      // vaddps        %ymm6,%ymm2,%ymm2
  .byte  197,228,88,223                      // vaddps        %ymm7,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_screen_avx
.globl _sk_screen_avx
_sk_screen_avx:
  .byte  197,124,88,196                      // vaddps        %ymm4,%ymm0,%ymm8
  .byte  197,252,89,196                      // vmulps        %ymm4,%ymm0,%ymm0
  .byte  197,188,92,192                      // vsubps        %ymm0,%ymm8,%ymm0
  .byte  197,116,88,197                      // vaddps        %ymm5,%ymm1,%ymm8
  .byte  197,244,89,205                      // vmulps        %ymm5,%ymm1,%ymm1
  .byte  197,188,92,201                      // vsubps        %ymm1,%ymm8,%ymm1
  .byte  197,108,88,198                      // vaddps        %ymm6,%ymm2,%ymm8
  .byte  197,236,89,214                      // vmulps        %ymm6,%ymm2,%ymm2
  .byte  197,188,92,210                      // vsubps        %ymm2,%ymm8,%ymm2
  .byte  197,100,88,199                      // vaddps        %ymm7,%ymm3,%ymm8
  .byte  197,228,89,223                      // vmulps        %ymm7,%ymm3,%ymm3
  .byte  197,188,92,219                      // vsubps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_xor__avx
.globl _sk_xor__avx
_sk_xor__avx:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,92,207                       // vsubps        %ymm7,%ymm8,%ymm9
  .byte  197,180,89,192                      // vmulps        %ymm0,%ymm9,%ymm0
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,212                       // vmulps        %ymm4,%ymm8,%ymm10
  .byte  196,193,124,88,194                  // vaddps        %ymm10,%ymm0,%ymm0
  .byte  197,180,89,201                      // vmulps        %ymm1,%ymm9,%ymm1
  .byte  197,60,89,213                       // vmulps        %ymm5,%ymm8,%ymm10
  .byte  197,172,88,201                      // vaddps        %ymm1,%ymm10,%ymm1
  .byte  197,180,89,210                      // vmulps        %ymm2,%ymm9,%ymm2
  .byte  197,60,89,214                       // vmulps        %ymm6,%ymm8,%ymm10
  .byte  197,172,88,210                      // vaddps        %ymm2,%ymm10,%ymm2
  .byte  197,180,89,219                      // vmulps        %ymm3,%ymm9,%ymm3
  .byte  197,60,89,199                       // vmulps        %ymm7,%ymm8,%ymm8
  .byte  197,188,88,219                      // vaddps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_darken_avx
.globl _sk_darken_avx
_sk_darken_avx:
  .byte  197,124,88,196                      // vaddps        %ymm4,%ymm0,%ymm8
  .byte  197,252,89,199                      // vmulps        %ymm7,%ymm0,%ymm0
  .byte  197,100,89,204                      // vmulps        %ymm4,%ymm3,%ymm9
  .byte  196,193,124,95,193                  // vmaxps        %ymm9,%ymm0,%ymm0
  .byte  197,188,92,192                      // vsubps        %ymm0,%ymm8,%ymm0
  .byte  197,116,88,197                      // vaddps        %ymm5,%ymm1,%ymm8
  .byte  197,244,89,207                      // vmulps        %ymm7,%ymm1,%ymm1
  .byte  197,100,89,205                      // vmulps        %ymm5,%ymm3,%ymm9
  .byte  196,193,116,95,201                  // vmaxps        %ymm9,%ymm1,%ymm1
  .byte  197,188,92,201                      // vsubps        %ymm1,%ymm8,%ymm1
  .byte  197,108,88,198                      // vaddps        %ymm6,%ymm2,%ymm8
  .byte  197,236,89,215                      // vmulps        %ymm7,%ymm2,%ymm2
  .byte  197,100,89,206                      // vmulps        %ymm6,%ymm3,%ymm9
  .byte  196,193,108,95,209                  // vmaxps        %ymm9,%ymm2,%ymm2
  .byte  197,188,92,210                      // vsubps        %ymm2,%ymm8,%ymm2
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,199                       // vmulps        %ymm7,%ymm8,%ymm8
  .byte  197,188,88,219                      // vaddps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_lighten_avx
.globl _sk_lighten_avx
_sk_lighten_avx:
  .byte  197,124,88,196                      // vaddps        %ymm4,%ymm0,%ymm8
  .byte  197,252,89,199                      // vmulps        %ymm7,%ymm0,%ymm0
  .byte  197,100,89,204                      // vmulps        %ymm4,%ymm3,%ymm9
  .byte  196,193,124,93,193                  // vminps        %ymm9,%ymm0,%ymm0
  .byte  197,188,92,192                      // vsubps        %ymm0,%ymm8,%ymm0
  .byte  197,116,88,197                      // vaddps        %ymm5,%ymm1,%ymm8
  .byte  197,244,89,207                      // vmulps        %ymm7,%ymm1,%ymm1
  .byte  197,100,89,205                      // vmulps        %ymm5,%ymm3,%ymm9
  .byte  196,193,116,93,201                  // vminps        %ymm9,%ymm1,%ymm1
  .byte  197,188,92,201                      // vsubps        %ymm1,%ymm8,%ymm1
  .byte  197,108,88,198                      // vaddps        %ymm6,%ymm2,%ymm8
  .byte  197,236,89,215                      // vmulps        %ymm7,%ymm2,%ymm2
  .byte  197,100,89,206                      // vmulps        %ymm6,%ymm3,%ymm9
  .byte  196,193,108,93,209                  // vminps        %ymm9,%ymm2,%ymm2
  .byte  197,188,92,210                      // vsubps        %ymm2,%ymm8,%ymm2
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,199                       // vmulps        %ymm7,%ymm8,%ymm8
  .byte  197,188,88,219                      // vaddps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_difference_avx
.globl _sk_difference_avx
_sk_difference_avx:
  .byte  197,124,88,196                      // vaddps        %ymm4,%ymm0,%ymm8
  .byte  197,252,89,199                      // vmulps        %ymm7,%ymm0,%ymm0
  .byte  197,100,89,204                      // vmulps        %ymm4,%ymm3,%ymm9
  .byte  196,193,124,93,193                  // vminps        %ymm9,%ymm0,%ymm0
  .byte  197,252,88,192                      // vaddps        %ymm0,%ymm0,%ymm0
  .byte  197,188,92,192                      // vsubps        %ymm0,%ymm8,%ymm0
  .byte  197,116,88,197                      // vaddps        %ymm5,%ymm1,%ymm8
  .byte  197,244,89,207                      // vmulps        %ymm7,%ymm1,%ymm1
  .byte  197,100,89,205                      // vmulps        %ymm5,%ymm3,%ymm9
  .byte  196,193,116,93,201                  // vminps        %ymm9,%ymm1,%ymm1
  .byte  197,244,88,201                      // vaddps        %ymm1,%ymm1,%ymm1
  .byte  197,188,92,201                      // vsubps        %ymm1,%ymm8,%ymm1
  .byte  197,108,88,198                      // vaddps        %ymm6,%ymm2,%ymm8
  .byte  197,236,89,215                      // vmulps        %ymm7,%ymm2,%ymm2
  .byte  197,100,89,206                      // vmulps        %ymm6,%ymm3,%ymm9
  .byte  196,193,108,93,209                  // vminps        %ymm9,%ymm2,%ymm2
  .byte  197,236,88,210                      // vaddps        %ymm2,%ymm2,%ymm2
  .byte  197,188,92,210                      // vsubps        %ymm2,%ymm8,%ymm2
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,199                       // vmulps        %ymm7,%ymm8,%ymm8
  .byte  197,188,88,219                      // vaddps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_exclusion_avx
.globl _sk_exclusion_avx
_sk_exclusion_avx:
  .byte  197,124,88,196                      // vaddps        %ymm4,%ymm0,%ymm8
  .byte  197,252,89,196                      // vmulps        %ymm4,%ymm0,%ymm0
  .byte  197,252,88,192                      // vaddps        %ymm0,%ymm0,%ymm0
  .byte  197,188,92,192                      // vsubps        %ymm0,%ymm8,%ymm0
  .byte  197,116,88,197                      // vaddps        %ymm5,%ymm1,%ymm8
  .byte  197,244,89,205                      // vmulps        %ymm5,%ymm1,%ymm1
  .byte  197,244,88,201                      // vaddps        %ymm1,%ymm1,%ymm1
  .byte  197,188,92,201                      // vsubps        %ymm1,%ymm8,%ymm1
  .byte  197,108,88,198                      // vaddps        %ymm6,%ymm2,%ymm8
  .byte  197,236,89,214                      // vmulps        %ymm6,%ymm2,%ymm2
  .byte  197,236,88,210                      // vaddps        %ymm2,%ymm2,%ymm2
  .byte  197,188,92,210                      // vsubps        %ymm2,%ymm8,%ymm2
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,199                       // vmulps        %ymm7,%ymm8,%ymm8
  .byte  197,188,88,219                      // vaddps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_colorburn_avx
.globl _sk_colorburn_avx
_sk_colorburn_avx:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,92,207                       // vsubps        %ymm7,%ymm8,%ymm9
  .byte  197,52,89,216                       // vmulps        %ymm0,%ymm9,%ymm11
  .byte  196,65,44,87,210                    // vxorps        %ymm10,%ymm10,%ymm10
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,228                       // vmulps        %ymm4,%ymm8,%ymm12
  .byte  197,68,92,236                       // vsubps        %ymm4,%ymm7,%ymm13
  .byte  197,20,89,235                       // vmulps        %ymm3,%ymm13,%ymm13
  .byte  197,20,94,232                       // vdivps        %ymm0,%ymm13,%ymm13
  .byte  196,65,68,93,237                    // vminps        %ymm13,%ymm7,%ymm13
  .byte  196,65,68,92,237                    // vsubps        %ymm13,%ymm7,%ymm13
  .byte  197,20,89,235                       // vmulps        %ymm3,%ymm13,%ymm13
  .byte  196,65,20,88,235                    // vaddps        %ymm11,%ymm13,%ymm13
  .byte  196,65,28,88,237                    // vaddps        %ymm13,%ymm12,%ymm13
  .byte  197,28,88,224                       // vaddps        %ymm0,%ymm12,%ymm12
  .byte  196,193,124,194,194,0               // vcmpeqps      %ymm10,%ymm0,%ymm0
  .byte  196,195,21,74,196,0                 // vblendvps     %ymm0,%ymm12,%ymm13,%ymm0
  .byte  197,92,194,231,0                    // vcmpeqps      %ymm7,%ymm4,%ymm12
  .byte  197,36,88,220                       // vaddps        %ymm4,%ymm11,%ymm11
  .byte  196,195,125,74,195,192              // vblendvps     %ymm12,%ymm11,%ymm0,%ymm0
  .byte  197,52,89,217                       // vmulps        %ymm1,%ymm9,%ymm11
  .byte  197,60,89,229                       // vmulps        %ymm5,%ymm8,%ymm12
  .byte  197,68,92,237                       // vsubps        %ymm5,%ymm7,%ymm13
  .byte  197,20,89,235                       // vmulps        %ymm3,%ymm13,%ymm13
  .byte  197,20,94,233                       // vdivps        %ymm1,%ymm13,%ymm13
  .byte  196,65,68,93,237                    // vminps        %ymm13,%ymm7,%ymm13
  .byte  196,65,68,92,237                    // vsubps        %ymm13,%ymm7,%ymm13
  .byte  197,20,89,235                       // vmulps        %ymm3,%ymm13,%ymm13
  .byte  196,65,36,88,237                    // vaddps        %ymm13,%ymm11,%ymm13
  .byte  196,65,28,88,237                    // vaddps        %ymm13,%ymm12,%ymm13
  .byte  197,28,88,225                       // vaddps        %ymm1,%ymm12,%ymm12
  .byte  196,193,116,194,202,0               // vcmpeqps      %ymm10,%ymm1,%ymm1
  .byte  196,195,21,74,204,16                // vblendvps     %ymm1,%ymm12,%ymm13,%ymm1
  .byte  197,84,194,231,0                    // vcmpeqps      %ymm7,%ymm5,%ymm12
  .byte  197,36,88,221                       // vaddps        %ymm5,%ymm11,%ymm11
  .byte  196,195,117,74,203,192              // vblendvps     %ymm12,%ymm11,%ymm1,%ymm1
  .byte  197,52,89,202                       // vmulps        %ymm2,%ymm9,%ymm9
  .byte  196,65,108,194,210,0                // vcmpeqps      %ymm10,%ymm2,%ymm10
  .byte  197,60,89,222                       // vmulps        %ymm6,%ymm8,%ymm11
  .byte  197,68,92,230                       // vsubps        %ymm6,%ymm7,%ymm12
  .byte  197,28,89,227                       // vmulps        %ymm3,%ymm12,%ymm12
  .byte  197,28,94,226                       // vdivps        %ymm2,%ymm12,%ymm12
  .byte  197,164,88,210                      // vaddps        %ymm2,%ymm11,%ymm2
  .byte  196,65,68,93,228                    // vminps        %ymm12,%ymm7,%ymm12
  .byte  196,65,68,92,228                    // vsubps        %ymm12,%ymm7,%ymm12
  .byte  197,28,89,227                       // vmulps        %ymm3,%ymm12,%ymm12
  .byte  196,65,52,88,228                    // vaddps        %ymm12,%ymm9,%ymm12
  .byte  196,65,36,88,220                    // vaddps        %ymm12,%ymm11,%ymm11
  .byte  196,227,37,74,210,160               // vblendvps     %ymm10,%ymm2,%ymm11,%ymm2
  .byte  197,76,194,215,0                    // vcmpeqps      %ymm7,%ymm6,%ymm10
  .byte  197,52,88,206                       // vaddps        %ymm6,%ymm9,%ymm9
  .byte  196,195,109,74,209,160              // vblendvps     %ymm10,%ymm9,%ymm2,%ymm2
  .byte  197,60,89,199                       // vmulps        %ymm7,%ymm8,%ymm8
  .byte  197,188,88,219                      // vaddps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_colordodge_avx
.globl _sk_colordodge_avx
_sk_colordodge_avx:
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,200                     // vmovd         %eax,%xmm9
  .byte  196,67,121,4,201,0                  // vpermilps     $0x0,%xmm9,%xmm9
  .byte  196,67,53,24,201,1                  // vinsertf128   $0x1,%xmm9,%ymm9,%ymm9
  .byte  197,52,92,215                       // vsubps        %ymm7,%ymm9,%ymm10
  .byte  197,44,89,216                       // vmulps        %ymm0,%ymm10,%ymm11
  .byte  197,52,92,203                       // vsubps        %ymm3,%ymm9,%ymm9
  .byte  197,100,89,228                      // vmulps        %ymm4,%ymm3,%ymm12
  .byte  197,100,92,232                      // vsubps        %ymm0,%ymm3,%ymm13
  .byte  196,65,28,94,229                    // vdivps        %ymm13,%ymm12,%ymm12
  .byte  197,52,89,236                       // vmulps        %ymm4,%ymm9,%ymm13
  .byte  196,65,68,93,228                    // vminps        %ymm12,%ymm7,%ymm12
  .byte  197,28,89,227                       // vmulps        %ymm3,%ymm12,%ymm12
  .byte  196,65,28,88,227                    // vaddps        %ymm11,%ymm12,%ymm12
  .byte  196,65,20,88,228                    // vaddps        %ymm12,%ymm13,%ymm12
  .byte  197,20,88,232                       // vaddps        %ymm0,%ymm13,%ymm13
  .byte  197,252,194,195,0                   // vcmpeqps      %ymm3,%ymm0,%ymm0
  .byte  196,195,29,74,197,0                 // vblendvps     %ymm0,%ymm13,%ymm12,%ymm0
  .byte  196,65,92,194,224,0                 // vcmpeqps      %ymm8,%ymm4,%ymm12
  .byte  197,36,88,220                       // vaddps        %ymm4,%ymm11,%ymm11
  .byte  196,195,125,74,195,192              // vblendvps     %ymm12,%ymm11,%ymm0,%ymm0
  .byte  197,44,89,217                       // vmulps        %ymm1,%ymm10,%ymm11
  .byte  197,100,89,229                      // vmulps        %ymm5,%ymm3,%ymm12
  .byte  197,100,92,233                      // vsubps        %ymm1,%ymm3,%ymm13
  .byte  196,65,28,94,229                    // vdivps        %ymm13,%ymm12,%ymm12
  .byte  197,52,89,237                       // vmulps        %ymm5,%ymm9,%ymm13
  .byte  196,65,68,93,228                    // vminps        %ymm12,%ymm7,%ymm12
  .byte  197,28,89,227                       // vmulps        %ymm3,%ymm12,%ymm12
  .byte  196,65,28,88,227                    // vaddps        %ymm11,%ymm12,%ymm12
  .byte  196,65,20,88,228                    // vaddps        %ymm12,%ymm13,%ymm12
  .byte  197,20,88,233                       // vaddps        %ymm1,%ymm13,%ymm13
  .byte  197,244,194,203,0                   // vcmpeqps      %ymm3,%ymm1,%ymm1
  .byte  196,195,29,74,205,16                // vblendvps     %ymm1,%ymm13,%ymm12,%ymm1
  .byte  196,65,84,194,224,0                 // vcmpeqps      %ymm8,%ymm5,%ymm12
  .byte  197,36,88,221                       // vaddps        %ymm5,%ymm11,%ymm11
  .byte  196,195,117,74,203,192              // vblendvps     %ymm12,%ymm11,%ymm1,%ymm1
  .byte  197,44,89,210                       // vmulps        %ymm2,%ymm10,%ymm10
  .byte  197,100,89,222                      // vmulps        %ymm6,%ymm3,%ymm11
  .byte  197,100,92,226                      // vsubps        %ymm2,%ymm3,%ymm12
  .byte  196,65,36,94,220                    // vdivps        %ymm12,%ymm11,%ymm11
  .byte  197,52,89,230                       // vmulps        %ymm6,%ymm9,%ymm12
  .byte  196,65,68,93,219                    // vminps        %ymm11,%ymm7,%ymm11
  .byte  197,36,89,219                       // vmulps        %ymm3,%ymm11,%ymm11
  .byte  196,65,44,88,219                    // vaddps        %ymm11,%ymm10,%ymm11
  .byte  196,65,28,88,219                    // vaddps        %ymm11,%ymm12,%ymm11
  .byte  197,28,88,226                       // vaddps        %ymm2,%ymm12,%ymm12
  .byte  197,236,194,211,0                   // vcmpeqps      %ymm3,%ymm2,%ymm2
  .byte  196,195,37,74,212,32                // vblendvps     %ymm2,%ymm12,%ymm11,%ymm2
  .byte  196,65,76,194,192,0                 // vcmpeqps      %ymm8,%ymm6,%ymm8
  .byte  197,44,88,214                       // vaddps        %ymm6,%ymm10,%ymm10
  .byte  196,195,109,74,210,128              // vblendvps     %ymm8,%ymm10,%ymm2,%ymm2
  .byte  197,52,89,199                       // vmulps        %ymm7,%ymm9,%ymm8
  .byte  197,188,88,219                      // vaddps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_hardlight_avx
.globl _sk_hardlight_avx
_sk_hardlight_avx:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,92,215                       // vsubps        %ymm7,%ymm8,%ymm10
  .byte  197,44,89,200                       // vmulps        %ymm0,%ymm10,%ymm9
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,220                       // vmulps        %ymm4,%ymm8,%ymm11
  .byte  196,65,52,88,219                    // vaddps        %ymm11,%ymm9,%ymm11
  .byte  197,124,88,200                      // vaddps        %ymm0,%ymm0,%ymm9
  .byte  197,52,194,227,2                    // vcmpleps      %ymm3,%ymm9,%ymm12
  .byte  197,124,89,204                      // vmulps        %ymm4,%ymm0,%ymm9
  .byte  196,65,52,88,233                    // vaddps        %ymm9,%ymm9,%ymm13
  .byte  197,100,89,207                      // vmulps        %ymm7,%ymm3,%ymm9
  .byte  197,68,92,244                       // vsubps        %ymm4,%ymm7,%ymm14
  .byte  197,228,92,192                      // vsubps        %ymm0,%ymm3,%ymm0
  .byte  196,193,124,89,198                  // vmulps        %ymm14,%ymm0,%ymm0
  .byte  197,252,88,192                      // vaddps        %ymm0,%ymm0,%ymm0
  .byte  197,180,92,192                      // vsubps        %ymm0,%ymm9,%ymm0
  .byte  196,195,125,74,197,192              // vblendvps     %ymm12,%ymm13,%ymm0,%ymm0
  .byte  196,193,124,88,195                  // vaddps        %ymm11,%ymm0,%ymm0
  .byte  197,44,89,217                       // vmulps        %ymm1,%ymm10,%ymm11
  .byte  197,60,89,229                       // vmulps        %ymm5,%ymm8,%ymm12
  .byte  196,65,28,88,219                    // vaddps        %ymm11,%ymm12,%ymm11
  .byte  197,116,88,225                      // vaddps        %ymm1,%ymm1,%ymm12
  .byte  197,28,194,227,2                    // vcmpleps      %ymm3,%ymm12,%ymm12
  .byte  197,116,89,237                      // vmulps        %ymm5,%ymm1,%ymm13
  .byte  196,65,20,88,237                    // vaddps        %ymm13,%ymm13,%ymm13
  .byte  197,68,92,245                       // vsubps        %ymm5,%ymm7,%ymm14
  .byte  197,228,92,201                      // vsubps        %ymm1,%ymm3,%ymm1
  .byte  196,193,116,89,206                  // vmulps        %ymm14,%ymm1,%ymm1
  .byte  197,244,88,201                      // vaddps        %ymm1,%ymm1,%ymm1
  .byte  197,180,92,201                      // vsubps        %ymm1,%ymm9,%ymm1
  .byte  196,195,117,74,205,192              // vblendvps     %ymm12,%ymm13,%ymm1,%ymm1
  .byte  196,193,116,88,203                  // vaddps        %ymm11,%ymm1,%ymm1
  .byte  197,44,89,210                       // vmulps        %ymm2,%ymm10,%ymm10
  .byte  197,60,89,222                       // vmulps        %ymm6,%ymm8,%ymm11
  .byte  196,65,36,88,210                    // vaddps        %ymm10,%ymm11,%ymm10
  .byte  197,108,88,218                      // vaddps        %ymm2,%ymm2,%ymm11
  .byte  197,36,194,219,2                    // vcmpleps      %ymm3,%ymm11,%ymm11
  .byte  197,108,89,230                      // vmulps        %ymm6,%ymm2,%ymm12
  .byte  196,65,28,88,228                    // vaddps        %ymm12,%ymm12,%ymm12
  .byte  197,68,92,238                       // vsubps        %ymm6,%ymm7,%ymm13
  .byte  197,228,92,210                      // vsubps        %ymm2,%ymm3,%ymm2
  .byte  196,193,108,89,213                  // vmulps        %ymm13,%ymm2,%ymm2
  .byte  197,236,88,210                      // vaddps        %ymm2,%ymm2,%ymm2
  .byte  197,180,92,210                      // vsubps        %ymm2,%ymm9,%ymm2
  .byte  196,195,109,74,212,176              // vblendvps     %ymm11,%ymm12,%ymm2,%ymm2
  .byte  196,193,108,88,210                  // vaddps        %ymm10,%ymm2,%ymm2
  .byte  197,60,89,199                       // vmulps        %ymm7,%ymm8,%ymm8
  .byte  197,188,88,219                      // vaddps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_overlay_avx
.globl _sk_overlay_avx
_sk_overlay_avx:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,92,215                       // vsubps        %ymm7,%ymm8,%ymm10
  .byte  197,44,89,200                       // vmulps        %ymm0,%ymm10,%ymm9
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,220                       // vmulps        %ymm4,%ymm8,%ymm11
  .byte  196,65,52,88,219                    // vaddps        %ymm11,%ymm9,%ymm11
  .byte  197,92,88,204                       // vaddps        %ymm4,%ymm4,%ymm9
  .byte  197,52,194,231,2                    // vcmpleps      %ymm7,%ymm9,%ymm12
  .byte  197,124,89,204                      // vmulps        %ymm4,%ymm0,%ymm9
  .byte  196,65,52,88,233                    // vaddps        %ymm9,%ymm9,%ymm13
  .byte  197,100,89,207                      // vmulps        %ymm7,%ymm3,%ymm9
  .byte  197,68,92,244                       // vsubps        %ymm4,%ymm7,%ymm14
  .byte  197,228,92,192                      // vsubps        %ymm0,%ymm3,%ymm0
  .byte  196,193,124,89,198                  // vmulps        %ymm14,%ymm0,%ymm0
  .byte  197,252,88,192                      // vaddps        %ymm0,%ymm0,%ymm0
  .byte  197,180,92,192                      // vsubps        %ymm0,%ymm9,%ymm0
  .byte  196,195,125,74,197,192              // vblendvps     %ymm12,%ymm13,%ymm0,%ymm0
  .byte  196,193,124,88,195                  // vaddps        %ymm11,%ymm0,%ymm0
  .byte  197,44,89,217                       // vmulps        %ymm1,%ymm10,%ymm11
  .byte  197,60,89,229                       // vmulps        %ymm5,%ymm8,%ymm12
  .byte  196,65,28,88,219                    // vaddps        %ymm11,%ymm12,%ymm11
  .byte  197,84,88,229                       // vaddps        %ymm5,%ymm5,%ymm12
  .byte  197,28,194,231,2                    // vcmpleps      %ymm7,%ymm12,%ymm12
  .byte  197,116,89,237                      // vmulps        %ymm5,%ymm1,%ymm13
  .byte  196,65,20,88,237                    // vaddps        %ymm13,%ymm13,%ymm13
  .byte  197,68,92,245                       // vsubps        %ymm5,%ymm7,%ymm14
  .byte  197,228,92,201                      // vsubps        %ymm1,%ymm3,%ymm1
  .byte  196,193,116,89,206                  // vmulps        %ymm14,%ymm1,%ymm1
  .byte  197,244,88,201                      // vaddps        %ymm1,%ymm1,%ymm1
  .byte  197,180,92,201                      // vsubps        %ymm1,%ymm9,%ymm1
  .byte  196,195,117,74,205,192              // vblendvps     %ymm12,%ymm13,%ymm1,%ymm1
  .byte  196,193,116,88,203                  // vaddps        %ymm11,%ymm1,%ymm1
  .byte  197,44,89,210                       // vmulps        %ymm2,%ymm10,%ymm10
  .byte  197,60,89,222                       // vmulps        %ymm6,%ymm8,%ymm11
  .byte  196,65,36,88,210                    // vaddps        %ymm10,%ymm11,%ymm10
  .byte  197,76,88,222                       // vaddps        %ymm6,%ymm6,%ymm11
  .byte  197,36,194,223,2                    // vcmpleps      %ymm7,%ymm11,%ymm11
  .byte  197,108,89,230                      // vmulps        %ymm6,%ymm2,%ymm12
  .byte  196,65,28,88,228                    // vaddps        %ymm12,%ymm12,%ymm12
  .byte  197,68,92,238                       // vsubps        %ymm6,%ymm7,%ymm13
  .byte  197,228,92,210                      // vsubps        %ymm2,%ymm3,%ymm2
  .byte  196,193,108,89,213                  // vmulps        %ymm13,%ymm2,%ymm2
  .byte  197,236,88,210                      // vaddps        %ymm2,%ymm2,%ymm2
  .byte  197,180,92,210                      // vsubps        %ymm2,%ymm9,%ymm2
  .byte  196,195,109,74,212,176              // vblendvps     %ymm11,%ymm12,%ymm2,%ymm2
  .byte  196,193,108,88,210                  // vaddps        %ymm10,%ymm2,%ymm2
  .byte  197,60,89,199                       // vmulps        %ymm7,%ymm8,%ymm8
  .byte  197,188,88,219                      // vaddps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_softlight_avx
.globl _sk_softlight_avx
_sk_softlight_avx:
  .byte  197,252,17,84,36,200                // vmovups       %ymm2,-0x38(%rsp)
  .byte  197,252,40,209                      // vmovaps       %ymm1,%ymm2
  .byte  196,65,52,87,201                    // vxorps        %ymm9,%ymm9,%ymm9
  .byte  197,52,194,215,1                    // vcmpltps      %ymm7,%ymm9,%ymm10
  .byte  197,92,94,199                       // vdivps        %ymm7,%ymm4,%ymm8
  .byte  196,67,53,74,216,160                // vblendvps     %ymm10,%ymm8,%ymm9,%ymm11
  .byte  196,65,36,88,195                    // vaddps        %ymm11,%ymm11,%ymm8
  .byte  196,65,60,88,224                    // vaddps        %ymm8,%ymm8,%ymm12
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  196,65,28,89,236                    // vmulps        %ymm12,%ymm12,%ymm13
  .byte  196,65,28,88,229                    // vaddps        %ymm13,%ymm12,%ymm12
  .byte  196,65,36,92,232                    // vsubps        %ymm8,%ymm11,%ymm13
  .byte  196,65,28,89,237                    // vmulps        %ymm13,%ymm12,%ymm13
  .byte  184,0,0,224,64                      // mov           $0x40e00000,%eax
  .byte  197,121,110,224                     // vmovd         %eax,%xmm12
  .byte  196,67,121,4,228,0                  // vpermilps     $0x0,%xmm12,%xmm12
  .byte  196,67,29,24,228,1                  // vinsertf128   $0x1,%xmm12,%ymm12,%ymm12
  .byte  196,65,36,89,244                    // vmulps        %ymm12,%ymm11,%ymm14
  .byte  196,65,20,88,238                    // vaddps        %ymm14,%ymm13,%ymm13
  .byte  196,65,124,82,243                   // vrsqrtps      %ymm11,%ymm14
  .byte  196,65,124,83,246                   // vrcpps        %ymm14,%ymm14
  .byte  196,65,12,92,243                    // vsubps        %ymm11,%ymm14,%ymm14
  .byte  197,92,88,252                       // vaddps        %ymm4,%ymm4,%ymm15
  .byte  196,65,4,88,255                     // vaddps        %ymm15,%ymm15,%ymm15
  .byte  197,4,194,255,2                     // vcmpleps      %ymm7,%ymm15,%ymm15
  .byte  196,67,13,74,237,240                // vblendvps     %ymm15,%ymm13,%ymm14,%ymm13
  .byte  197,124,88,240                      // vaddps        %ymm0,%ymm0,%ymm14
  .byte  197,12,92,251                       // vsubps        %ymm3,%ymm14,%ymm15
  .byte  196,65,60,92,219                    // vsubps        %ymm11,%ymm8,%ymm11
  .byte  196,65,4,89,219                     // vmulps        %ymm11,%ymm15,%ymm11
  .byte  197,36,88,219                       // vaddps        %ymm3,%ymm11,%ymm11
  .byte  197,36,89,220                       // vmulps        %ymm4,%ymm11,%ymm11
  .byte  197,4,89,255                        // vmulps        %ymm7,%ymm15,%ymm15
  .byte  196,65,4,89,237                     // vmulps        %ymm13,%ymm15,%ymm13
  .byte  197,100,89,252                      // vmulps        %ymm4,%ymm3,%ymm15
  .byte  196,65,4,88,237                     // vaddps        %ymm13,%ymm15,%ymm13
  .byte  197,12,194,243,2                    // vcmpleps      %ymm3,%ymm14,%ymm14
  .byte  196,195,21,74,203,224               // vblendvps     %ymm14,%ymm11,%ymm13,%ymm1
  .byte  197,84,94,239                       // vdivps        %ymm7,%ymm5,%ymm13
  .byte  196,67,53,74,237,160                // vblendvps     %ymm10,%ymm13,%ymm9,%ymm13
  .byte  196,65,20,88,245                    // vaddps        %ymm13,%ymm13,%ymm14
  .byte  196,65,12,88,246                    // vaddps        %ymm14,%ymm14,%ymm14
  .byte  196,65,12,89,254                    // vmulps        %ymm14,%ymm14,%ymm15
  .byte  196,65,12,88,247                    // vaddps        %ymm15,%ymm14,%ymm14
  .byte  196,65,20,92,248                    // vsubps        %ymm8,%ymm13,%ymm15
  .byte  196,65,4,89,246                     // vmulps        %ymm14,%ymm15,%ymm14
  .byte  196,65,28,89,253                    // vmulps        %ymm13,%ymm12,%ymm15
  .byte  196,65,4,88,246                     // vaddps        %ymm14,%ymm15,%ymm14
  .byte  196,65,124,82,253                   // vrsqrtps      %ymm13,%ymm15
  .byte  196,65,124,83,255                   // vrcpps        %ymm15,%ymm15
  .byte  196,65,4,92,253                     // vsubps        %ymm13,%ymm15,%ymm15
  .byte  197,84,88,221                       // vaddps        %ymm5,%ymm5,%ymm11
  .byte  196,65,36,88,219                    // vaddps        %ymm11,%ymm11,%ymm11
  .byte  197,36,194,223,2                    // vcmpleps      %ymm7,%ymm11,%ymm11
  .byte  196,67,5,74,222,176                 // vblendvps     %ymm11,%ymm14,%ymm15,%ymm11
  .byte  197,108,88,242                      // vaddps        %ymm2,%ymm2,%ymm14
  .byte  196,65,60,92,237                    // vsubps        %ymm13,%ymm8,%ymm13
  .byte  197,12,92,251                       // vsubps        %ymm3,%ymm14,%ymm15
  .byte  196,65,4,89,237                     // vmulps        %ymm13,%ymm15,%ymm13
  .byte  197,4,89,255                        // vmulps        %ymm7,%ymm15,%ymm15
  .byte  196,65,4,89,219                     // vmulps        %ymm11,%ymm15,%ymm11
  .byte  197,100,89,253                      // vmulps        %ymm5,%ymm3,%ymm15
  .byte  196,65,4,88,219                     // vaddps        %ymm11,%ymm15,%ymm11
  .byte  197,20,88,235                       // vaddps        %ymm3,%ymm13,%ymm13
  .byte  197,20,89,237                       // vmulps        %ymm5,%ymm13,%ymm13
  .byte  197,12,194,243,2                    // vcmpleps      %ymm3,%ymm14,%ymm14
  .byte  196,67,37,74,237,224                // vblendvps     %ymm14,%ymm13,%ymm11,%ymm13
  .byte  197,76,94,223                       // vdivps        %ymm7,%ymm6,%ymm11
  .byte  196,67,53,74,203,160                // vblendvps     %ymm10,%ymm11,%ymm9,%ymm9
  .byte  196,65,52,88,209                    // vaddps        %ymm9,%ymm9,%ymm10
  .byte  196,65,44,88,210                    // vaddps        %ymm10,%ymm10,%ymm10
  .byte  196,65,44,89,218                    // vmulps        %ymm10,%ymm10,%ymm11
  .byte  196,65,44,88,211                    // vaddps        %ymm11,%ymm10,%ymm10
  .byte  196,65,52,92,216                    // vsubps        %ymm8,%ymm9,%ymm11
  .byte  196,65,36,89,210                    // vmulps        %ymm10,%ymm11,%ymm10
  .byte  196,65,28,89,217                    // vmulps        %ymm9,%ymm12,%ymm11
  .byte  196,65,36,88,210                    // vaddps        %ymm10,%ymm11,%ymm10
  .byte  196,65,124,82,217                   // vrsqrtps      %ymm9,%ymm11
  .byte  196,65,124,83,219                   // vrcpps        %ymm11,%ymm11
  .byte  196,65,36,92,217                    // vsubps        %ymm9,%ymm11,%ymm11
  .byte  197,76,88,230                       // vaddps        %ymm6,%ymm6,%ymm12
  .byte  196,65,28,88,228                    // vaddps        %ymm12,%ymm12,%ymm12
  .byte  197,28,194,231,2                    // vcmpleps      %ymm7,%ymm12,%ymm12
  .byte  196,67,37,74,210,192                // vblendvps     %ymm12,%ymm10,%ymm11,%ymm10
  .byte  197,124,16,116,36,200               // vmovups       -0x38(%rsp),%ymm14
  .byte  196,65,12,88,222                    // vaddps        %ymm14,%ymm14,%ymm11
  .byte  197,36,92,227                       // vsubps        %ymm3,%ymm11,%ymm12
  .byte  196,65,60,92,201                    // vsubps        %ymm9,%ymm8,%ymm9
  .byte  196,65,28,89,201                    // vmulps        %ymm9,%ymm12,%ymm9
  .byte  197,28,89,231                       // vmulps        %ymm7,%ymm12,%ymm12
  .byte  196,65,28,89,210                    // vmulps        %ymm10,%ymm12,%ymm10
  .byte  197,100,89,230                      // vmulps        %ymm6,%ymm3,%ymm12
  .byte  196,65,28,88,210                    // vaddps        %ymm10,%ymm12,%ymm10
  .byte  197,52,88,203                       // vaddps        %ymm3,%ymm9,%ymm9
  .byte  197,52,89,206                       // vmulps        %ymm6,%ymm9,%ymm9
  .byte  197,36,194,219,2                    // vcmpleps      %ymm3,%ymm11,%ymm11
  .byte  196,67,45,74,201,176                // vblendvps     %ymm11,%ymm9,%ymm10,%ymm9
  .byte  197,60,92,215                       // vsubps        %ymm7,%ymm8,%ymm10
  .byte  197,172,89,192                      // vmulps        %ymm0,%ymm10,%ymm0
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  197,60,89,220                       // vmulps        %ymm4,%ymm8,%ymm11
  .byte  196,193,124,88,195                  // vaddps        %ymm11,%ymm0,%ymm0
  .byte  197,244,88,192                      // vaddps        %ymm0,%ymm1,%ymm0
  .byte  197,172,89,202                      // vmulps        %ymm2,%ymm10,%ymm1
  .byte  197,188,89,213                      // vmulps        %ymm5,%ymm8,%ymm2
  .byte  197,236,88,201                      // vaddps        %ymm1,%ymm2,%ymm1
  .byte  196,193,116,88,205                  // vaddps        %ymm13,%ymm1,%ymm1
  .byte  196,193,44,89,214                   // vmulps        %ymm14,%ymm10,%ymm2
  .byte  197,60,89,214                       // vmulps        %ymm6,%ymm8,%ymm10
  .byte  197,172,88,210                      // vaddps        %ymm2,%ymm10,%ymm2
  .byte  196,193,108,88,209                  // vaddps        %ymm9,%ymm2,%ymm2
  .byte  197,60,89,199                       // vmulps        %ymm7,%ymm8,%ymm8
  .byte  197,188,88,219                      // vaddps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_0_avx
.globl _sk_clamp_0_avx
_sk_clamp_0_avx:
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  196,193,124,95,192                  // vmaxps        %ymm8,%ymm0,%ymm0
  .byte  196,193,116,95,200                  // vmaxps        %ymm8,%ymm1,%ymm1
  .byte  196,193,108,95,208                  // vmaxps        %ymm8,%ymm2,%ymm2
  .byte  196,193,100,95,216                  // vmaxps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_1_avx
.globl _sk_clamp_1_avx
_sk_clamp_1_avx:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  196,193,124,93,192                  // vminps        %ymm8,%ymm0,%ymm0
  .byte  196,193,116,93,200                  // vminps        %ymm8,%ymm1,%ymm1
  .byte  196,193,108,93,208                  // vminps        %ymm8,%ymm2,%ymm2
  .byte  196,193,100,93,216                  // vminps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_a_avx
.globl _sk_clamp_a_avx
_sk_clamp_a_avx:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  196,193,100,93,216                  // vminps        %ymm8,%ymm3,%ymm3
  .byte  197,252,93,195                      // vminps        %ymm3,%ymm0,%ymm0
  .byte  197,244,93,203                      // vminps        %ymm3,%ymm1,%ymm1
  .byte  197,236,93,211                      // vminps        %ymm3,%ymm2,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_set_rgb_avx
.globl _sk_set_rgb_avx
_sk_set_rgb_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,226,125,24,0                    // vbroadcastss  (%rax),%ymm0
  .byte  196,226,125,24,72,4                 // vbroadcastss  0x4(%rax),%ymm1
  .byte  196,226,125,24,80,8                 // vbroadcastss  0x8(%rax),%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_swap_rb_avx
.globl _sk_swap_rb_avx
_sk_swap_rb_avx:
  .byte  197,124,40,192                      // vmovaps       %ymm0,%ymm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,40,194                      // vmovaps       %ymm2,%ymm0
  .byte  197,124,41,194                      // vmovaps       %ymm8,%ymm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_swap_avx
.globl _sk_swap_avx
_sk_swap_avx:
  .byte  197,124,40,195                      // vmovaps       %ymm3,%ymm8
  .byte  197,124,40,202                      // vmovaps       %ymm2,%ymm9
  .byte  197,124,40,209                      // vmovaps       %ymm1,%ymm10
  .byte  197,124,40,216                      // vmovaps       %ymm0,%ymm11
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,40,196                      // vmovaps       %ymm4,%ymm0
  .byte  197,252,40,205                      // vmovaps       %ymm5,%ymm1
  .byte  197,252,40,214                      // vmovaps       %ymm6,%ymm2
  .byte  197,252,40,223                      // vmovaps       %ymm7,%ymm3
  .byte  197,124,41,220                      // vmovaps       %ymm11,%ymm4
  .byte  197,124,41,213                      // vmovaps       %ymm10,%ymm5
  .byte  197,124,41,206                      // vmovaps       %ymm9,%ymm6
  .byte  197,124,41,199                      // vmovaps       %ymm8,%ymm7
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_move_src_dst_avx
.globl _sk_move_src_dst_avx
_sk_move_src_dst_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,40,224                      // vmovaps       %ymm0,%ymm4
  .byte  197,252,40,233                      // vmovaps       %ymm1,%ymm5
  .byte  197,252,40,242                      // vmovaps       %ymm2,%ymm6
  .byte  197,252,40,251                      // vmovaps       %ymm3,%ymm7
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_move_dst_src_avx
.globl _sk_move_dst_src_avx
_sk_move_dst_src_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,40,196                      // vmovaps       %ymm4,%ymm0
  .byte  197,252,40,205                      // vmovaps       %ymm5,%ymm1
  .byte  197,252,40,214                      // vmovaps       %ymm6,%ymm2
  .byte  197,252,40,223                      // vmovaps       %ymm7,%ymm3
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_premul_avx
.globl _sk_premul_avx
_sk_premul_avx:
  .byte  197,252,89,195                      // vmulps        %ymm3,%ymm0,%ymm0
  .byte  197,244,89,203                      // vmulps        %ymm3,%ymm1,%ymm1
  .byte  197,236,89,211                      // vmulps        %ymm3,%ymm2,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_unpremul_avx
.globl _sk_unpremul_avx
_sk_unpremul_avx:
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  196,65,100,194,200,0                // vcmpeqps      %ymm8,%ymm3,%ymm9
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,208                     // vmovd         %eax,%xmm10
  .byte  196,67,121,4,210,0                  // vpermilps     $0x0,%xmm10,%xmm10
  .byte  196,67,45,24,210,1                  // vinsertf128   $0x1,%xmm10,%ymm10,%ymm10
  .byte  197,44,94,211                       // vdivps        %ymm3,%ymm10,%ymm10
  .byte  196,67,45,74,192,144                // vblendvps     %ymm9,%ymm8,%ymm10,%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_from_srgb_avx
.globl _sk_from_srgb_avx
_sk_from_srgb_avx:
  .byte  184,145,131,158,61                  // mov           $0x3d9e8391,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,89,200                       // vmulps        %ymm0,%ymm8,%ymm9
  .byte  197,124,89,208                      // vmulps        %ymm0,%ymm0,%ymm10
  .byte  184,154,153,153,62                  // mov           $0x3e99999a,%eax
  .byte  197,121,110,216                     // vmovd         %eax,%xmm11
  .byte  196,67,121,4,219,0                  // vpermilps     $0x0,%xmm11,%xmm11
  .byte  196,67,37,24,219,1                  // vinsertf128   $0x1,%xmm11,%ymm11,%ymm11
  .byte  184,92,143,50,63                    // mov           $0x3f328f5c,%eax
  .byte  197,121,110,224                     // vmovd         %eax,%xmm12
  .byte  196,67,121,4,228,0                  // vpermilps     $0x0,%xmm12,%xmm12
  .byte  196,67,29,24,228,1                  // vinsertf128   $0x1,%xmm12,%ymm12,%ymm12
  .byte  197,36,89,232                       // vmulps        %ymm0,%ymm11,%ymm13
  .byte  196,65,20,88,236                    // vaddps        %ymm12,%ymm13,%ymm13
  .byte  184,10,215,35,59                    // mov           $0x3b23d70a,%eax
  .byte  197,121,110,240                     // vmovd         %eax,%xmm14
  .byte  196,67,121,4,246,0                  // vpermilps     $0x0,%xmm14,%xmm14
  .byte  196,67,13,24,246,1                  // vinsertf128   $0x1,%xmm14,%ymm14,%ymm14
  .byte  196,65,44,89,213                    // vmulps        %ymm13,%ymm10,%ymm10
  .byte  196,65,12,88,210                    // vaddps        %ymm10,%ymm14,%ymm10
  .byte  184,174,71,97,61                    // mov           $0x3d6147ae,%eax
  .byte  197,121,110,232                     // vmovd         %eax,%xmm13
  .byte  196,67,121,4,237,0                  // vpermilps     $0x0,%xmm13,%xmm13
  .byte  196,67,21,24,237,1                  // vinsertf128   $0x1,%xmm13,%ymm13,%ymm13
  .byte  196,193,124,194,197,1               // vcmpltps      %ymm13,%ymm0,%ymm0
  .byte  196,195,45,74,193,0                 // vblendvps     %ymm0,%ymm9,%ymm10,%ymm0
  .byte  197,60,89,201                       // vmulps        %ymm1,%ymm8,%ymm9
  .byte  197,116,89,209                      // vmulps        %ymm1,%ymm1,%ymm10
  .byte  197,36,89,249                       // vmulps        %ymm1,%ymm11,%ymm15
  .byte  196,65,28,88,255                    // vaddps        %ymm15,%ymm12,%ymm15
  .byte  196,65,44,89,215                    // vmulps        %ymm15,%ymm10,%ymm10
  .byte  196,65,12,88,210                    // vaddps        %ymm10,%ymm14,%ymm10
  .byte  196,193,116,194,205,1               // vcmpltps      %ymm13,%ymm1,%ymm1
  .byte  196,195,45,74,201,16                // vblendvps     %ymm1,%ymm9,%ymm10,%ymm1
  .byte  197,60,89,194                       // vmulps        %ymm2,%ymm8,%ymm8
  .byte  197,108,89,202                      // vmulps        %ymm2,%ymm2,%ymm9
  .byte  197,36,89,210                       // vmulps        %ymm2,%ymm11,%ymm10
  .byte  196,65,28,88,210                    // vaddps        %ymm10,%ymm12,%ymm10
  .byte  196,65,52,89,202                    // vmulps        %ymm10,%ymm9,%ymm9
  .byte  196,65,12,88,201                    // vaddps        %ymm9,%ymm14,%ymm9
  .byte  196,193,108,194,213,1               // vcmpltps      %ymm13,%ymm2,%ymm2
  .byte  196,195,53,74,208,32                // vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_to_srgb_avx
.globl _sk_to_srgb_avx
_sk_to_srgb_avx:
  .byte  197,124,82,192                      // vrsqrtps      %ymm0,%ymm8
  .byte  196,65,124,83,232                   // vrcpps        %ymm8,%ymm13
  .byte  196,65,124,82,240                   // vrsqrtps      %ymm8,%ymm14
  .byte  184,41,92,71,65                     // mov           $0x41475c29,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,89,224                       // vmulps        %ymm0,%ymm8,%ymm12
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,121,110,200                     // vmovd         %eax,%xmm9
  .byte  196,67,121,4,201,0                  // vpermilps     $0x0,%xmm9,%xmm9
  .byte  196,67,53,24,201,1                  // vinsertf128   $0x1,%xmm9,%ymm9,%ymm9
  .byte  184,194,135,210,62                  // mov           $0x3ed287c2,%eax
  .byte  197,121,110,208                     // vmovd         %eax,%xmm10
  .byte  196,67,121,4,210,0                  // vpermilps     $0x0,%xmm10,%xmm10
  .byte  196,67,45,24,210,1                  // vinsertf128   $0x1,%xmm10,%ymm10,%ymm10
  .byte  184,206,111,48,63                   // mov           $0x3f306fce,%eax
  .byte  197,121,110,216                     // vmovd         %eax,%xmm11
  .byte  196,67,121,4,219,0                  // vpermilps     $0x0,%xmm11,%xmm11
  .byte  196,67,37,24,219,1                  // vinsertf128   $0x1,%xmm11,%ymm11,%ymm11
  .byte  184,168,87,202,61                   // mov           $0x3dca57a8,%eax
  .byte  53,0,0,0,128                        // xor           $0x80000000,%eax
  .byte  197,121,110,248                     // vmovd         %eax,%xmm15
  .byte  196,67,121,4,255,0                  // vpermilps     $0x0,%xmm15,%xmm15
  .byte  196,67,5,24,255,1                   // vinsertf128   $0x1,%xmm15,%ymm15,%ymm15
  .byte  196,65,20,89,235                    // vmulps        %ymm11,%ymm13,%ymm13
  .byte  196,65,20,88,239                    // vaddps        %ymm15,%ymm13,%ymm13
  .byte  196,65,12,89,242                    // vmulps        %ymm10,%ymm14,%ymm14
  .byte  196,65,12,88,237                    // vaddps        %ymm13,%ymm14,%ymm13
  .byte  196,65,52,93,237                    // vminps        %ymm13,%ymm9,%ymm13
  .byte  184,4,231,140,59                    // mov           $0x3b8ce704,%eax
  .byte  197,121,110,240                     // vmovd         %eax,%xmm14
  .byte  196,67,121,4,246,0                  // vpermilps     $0x0,%xmm14,%xmm14
  .byte  196,67,13,24,246,1                  // vinsertf128   $0x1,%xmm14,%ymm14,%ymm14
  .byte  196,193,124,194,198,1               // vcmpltps      %ymm14,%ymm0,%ymm0
  .byte  196,195,21,74,196,0                 // vblendvps     %ymm0,%ymm12,%ymm13,%ymm0
  .byte  197,124,82,225                      // vrsqrtps      %ymm1,%ymm12
  .byte  196,65,124,83,236                   // vrcpps        %ymm12,%ymm13
  .byte  196,65,124,82,228                   // vrsqrtps      %ymm12,%ymm12
  .byte  196,65,36,89,237                    // vmulps        %ymm13,%ymm11,%ymm13
  .byte  196,65,4,88,237                     // vaddps        %ymm13,%ymm15,%ymm13
  .byte  196,65,44,89,228                    // vmulps        %ymm12,%ymm10,%ymm12
  .byte  196,65,28,88,229                    // vaddps        %ymm13,%ymm12,%ymm12
  .byte  197,60,89,233                       // vmulps        %ymm1,%ymm8,%ymm13
  .byte  196,65,52,93,228                    // vminps        %ymm12,%ymm9,%ymm12
  .byte  196,193,116,194,206,1               // vcmpltps      %ymm14,%ymm1,%ymm1
  .byte  196,195,29,74,205,16                // vblendvps     %ymm1,%ymm13,%ymm12,%ymm1
  .byte  197,124,82,226                      // vrsqrtps      %ymm2,%ymm12
  .byte  196,65,124,83,236                   // vrcpps        %ymm12,%ymm13
  .byte  196,65,36,89,221                    // vmulps        %ymm13,%ymm11,%ymm11
  .byte  196,65,4,88,219                     // vaddps        %ymm11,%ymm15,%ymm11
  .byte  196,65,124,82,228                   // vrsqrtps      %ymm12,%ymm12
  .byte  196,65,44,89,212                    // vmulps        %ymm12,%ymm10,%ymm10
  .byte  196,65,44,88,211                    // vaddps        %ymm11,%ymm10,%ymm10
  .byte  196,65,52,93,202                    // vminps        %ymm10,%ymm9,%ymm9
  .byte  197,60,89,194                       // vmulps        %ymm2,%ymm8,%ymm8
  .byte  196,193,108,194,214,1               // vcmpltps      %ymm14,%ymm2,%ymm2
  .byte  196,195,53,74,208,32                // vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_scale_1_float_avx
.globl _sk_scale_1_float_avx
_sk_scale_1_float_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_scale_u8_avx
.globl _sk_scale_u8_avx
_sk_scale_u8_avx:
  .byte  73,137,200                          // mov           %rcx,%r8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,1,248                            // add           %rdi,%rax
  .byte  77,133,192                          // test          %r8,%r8
  .byte  117,80                              // jne           f78 <_sk_scale_u8_avx+0x60>
  .byte  197,122,126,0                       // vmovq         (%rax),%xmm8
  .byte  196,66,121,49,200                   // vpmovzxbd     %xmm8,%xmm9
  .byte  196,67,121,4,192,229                // vpermilps     $0xe5,%xmm8,%xmm8
  .byte  196,66,121,49,192                   // vpmovzxbd     %xmm8,%xmm8
  .byte  196,67,53,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm9,%ymm8
  .byte  196,65,124,91,192                   // vcvtdq2ps     %ymm8,%ymm8
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  197,121,110,200                     // vmovd         %eax,%xmm9
  .byte  196,67,121,4,201,0                  // vpermilps     $0x0,%xmm9,%xmm9
  .byte  196,67,53,24,201,1                  // vinsertf128   $0x1,%xmm9,%ymm9,%ymm9
  .byte  196,65,60,89,193                    // vmulps        %ymm9,%ymm8,%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,137,193                          // mov           %r8,%rcx
  .byte  255,224                             // jmpq          *%rax
  .byte  49,201                              // xor           %ecx,%ecx
  .byte  77,137,194                          // mov           %r8,%r10
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  68,15,182,24                        // movzbl        (%rax),%r11d
  .byte  72,255,192                          // inc           %rax
  .byte  73,211,227                          // shl           %cl,%r11
  .byte  77,9,217                            // or            %r11,%r9
  .byte  72,131,193,8                        // add           $0x8,%rcx
  .byte  73,255,202                          // dec           %r10
  .byte  117,234                             // jne           f80 <_sk_scale_u8_avx+0x68>
  .byte  196,65,249,110,193                  // vmovq         %r9,%xmm8
  .byte  235,143                             // jmp           f2c <_sk_scale_u8_avx+0x14>

HIDDEN _sk_lerp_1_float_avx
.globl _sk_lerp_1_float_avx
_sk_lerp_1_float_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  197,252,92,196                      // vsubps        %ymm4,%ymm0,%ymm0
  .byte  196,193,124,89,192                  // vmulps        %ymm8,%ymm0,%ymm0
  .byte  197,252,88,196                      // vaddps        %ymm4,%ymm0,%ymm0
  .byte  197,244,92,205                      // vsubps        %ymm5,%ymm1,%ymm1
  .byte  196,193,116,89,200                  // vmulps        %ymm8,%ymm1,%ymm1
  .byte  197,244,88,205                      // vaddps        %ymm5,%ymm1,%ymm1
  .byte  197,236,92,214                      // vsubps        %ymm6,%ymm2,%ymm2
  .byte  196,193,108,89,208                  // vmulps        %ymm8,%ymm2,%ymm2
  .byte  197,236,88,214                      // vaddps        %ymm6,%ymm2,%ymm2
  .byte  197,228,92,223                      // vsubps        %ymm7,%ymm3,%ymm3
  .byte  196,193,100,89,216                  // vmulps        %ymm8,%ymm3,%ymm3
  .byte  197,228,88,223                      // vaddps        %ymm7,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_lerp_u8_avx
.globl _sk_lerp_u8_avx
_sk_lerp_u8_avx:
  .byte  73,137,200                          // mov           %rcx,%r8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,1,248                            // add           %rdi,%rax
  .byte  77,133,192                          // test          %r8,%r8
  .byte  117,116                             // jne           1060 <_sk_lerp_u8_avx+0x84>
  .byte  197,122,126,0                       // vmovq         (%rax),%xmm8
  .byte  196,66,121,49,200                   // vpmovzxbd     %xmm8,%xmm9
  .byte  196,67,121,4,192,229                // vpermilps     $0xe5,%xmm8,%xmm8
  .byte  196,66,121,49,192                   // vpmovzxbd     %xmm8,%xmm8
  .byte  196,67,53,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm9,%ymm8
  .byte  196,65,124,91,192                   // vcvtdq2ps     %ymm8,%ymm8
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  197,121,110,200                     // vmovd         %eax,%xmm9
  .byte  196,67,121,4,201,0                  // vpermilps     $0x0,%xmm9,%xmm9
  .byte  196,67,53,24,201,1                  // vinsertf128   $0x1,%xmm9,%ymm9,%ymm9
  .byte  196,65,60,89,193                    // vmulps        %ymm9,%ymm8,%ymm8
  .byte  197,252,92,196                      // vsubps        %ymm4,%ymm0,%ymm0
  .byte  196,193,124,89,192                  // vmulps        %ymm8,%ymm0,%ymm0
  .byte  197,252,88,196                      // vaddps        %ymm4,%ymm0,%ymm0
  .byte  197,244,92,205                      // vsubps        %ymm5,%ymm1,%ymm1
  .byte  196,193,116,89,200                  // vmulps        %ymm8,%ymm1,%ymm1
  .byte  197,244,88,205                      // vaddps        %ymm5,%ymm1,%ymm1
  .byte  197,236,92,214                      // vsubps        %ymm6,%ymm2,%ymm2
  .byte  196,193,108,89,208                  // vmulps        %ymm8,%ymm2,%ymm2
  .byte  197,236,88,214                      // vaddps        %ymm6,%ymm2,%ymm2
  .byte  197,228,92,223                      // vsubps        %ymm7,%ymm3,%ymm3
  .byte  196,193,100,89,216                  // vmulps        %ymm8,%ymm3,%ymm3
  .byte  197,228,88,223                      // vaddps        %ymm7,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,137,193                          // mov           %r8,%rcx
  .byte  255,224                             // jmpq          *%rax
  .byte  49,201                              // xor           %ecx,%ecx
  .byte  77,137,194                          // mov           %r8,%r10
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  68,15,182,24                        // movzbl        (%rax),%r11d
  .byte  72,255,192                          // inc           %rax
  .byte  73,211,227                          // shl           %cl,%r11
  .byte  77,9,217                            // or            %r11,%r9
  .byte  72,131,193,8                        // add           $0x8,%rcx
  .byte  73,255,202                          // dec           %r10
  .byte  117,234                             // jne           1068 <_sk_lerp_u8_avx+0x8c>
  .byte  196,65,249,110,193                  // vmovq         %r9,%xmm8
  .byte  233,104,255,255,255                 // jmpq          ff0 <_sk_lerp_u8_avx+0x14>

HIDDEN _sk_lerp_565_avx
.globl _sk_lerp_565_avx
_sk_lerp_565_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,16                           // mov           (%rax),%r10
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,250,0,0,0                    // jne           1190 <_sk_lerp_565_avx+0x108>
  .byte  196,65,122,111,4,122                // vmovdqu       (%r10,%rdi,2),%xmm8
  .byte  197,225,239,219                     // vpxor         %xmm3,%xmm3,%xmm3
  .byte  197,185,105,219                     // vpunpckhwd    %xmm3,%xmm8,%xmm3
  .byte  196,66,121,51,192                   // vpmovzxwd     %xmm8,%xmm8
  .byte  196,99,61,24,195,1                  // vinsertf128   $0x1,%xmm3,%ymm8,%ymm8
  .byte  184,0,248,0,0                       // mov           $0xf800,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  197,249,112,219,0                   // vpshufd       $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  196,193,100,84,216                  // vandps        %ymm8,%ymm3,%ymm3
  .byte  197,124,91,203                      // vcvtdq2ps     %ymm3,%ymm9
  .byte  184,8,33,132,55                     // mov           $0x37842108,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,227,121,4,219,0                 // vpermilps     $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  197,52,89,203                       // vmulps        %ymm3,%ymm9,%ymm9
  .byte  184,224,7,0,0                       // mov           $0x7e0,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  197,249,112,219,0                   // vpshufd       $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  196,193,100,84,216                  // vandps        %ymm8,%ymm3,%ymm3
  .byte  197,124,91,211                      // vcvtdq2ps     %ymm3,%ymm10
  .byte  184,33,8,2,58                       // mov           $0x3a020821,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,227,121,4,219,0                 // vpermilps     $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  197,44,89,211                       // vmulps        %ymm3,%ymm10,%ymm10
  .byte  184,31,0,0,0                        // mov           $0x1f,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  197,249,112,219,0                   // vpshufd       $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  196,193,100,84,216                  // vandps        %ymm8,%ymm3,%ymm3
  .byte  197,124,91,195                      // vcvtdq2ps     %ymm3,%ymm8
  .byte  184,8,33,4,61                       // mov           $0x3d042108,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,227,121,4,219,0                 // vpermilps     $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  197,252,92,196                      // vsubps        %ymm4,%ymm0,%ymm0
  .byte  196,193,124,89,193                  // vmulps        %ymm9,%ymm0,%ymm0
  .byte  197,252,88,196                      // vaddps        %ymm4,%ymm0,%ymm0
  .byte  197,244,92,205                      // vsubps        %ymm5,%ymm1,%ymm1
  .byte  196,193,116,89,202                  // vmulps        %ymm10,%ymm1,%ymm1
  .byte  197,244,88,205                      // vaddps        %ymm5,%ymm1,%ymm1
  .byte  197,236,92,214                      // vsubps        %ymm6,%ymm2,%ymm2
  .byte  197,236,89,211                      // vmulps        %ymm3,%ymm2,%ymm2
  .byte  197,236,88,214                      // vaddps        %ymm6,%ymm2,%ymm2
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,227,121,4,219,0                 // vpermilps     $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  65,137,200                          // mov           %ecx,%r8d
  .byte  65,128,224,7                        // and           $0x7,%r8b
  .byte  196,65,57,239,192                   // vpxor         %xmm8,%xmm8,%xmm8
  .byte  65,254,200                          // dec           %r8b
  .byte  65,128,248,6                        // cmp           $0x6,%r8b
  .byte  15,135,243,254,255,255              // ja            109c <_sk_lerp_565_avx+0x14>
  .byte  69,15,182,192                       // movzbl        %r8b,%r8d
  .byte  76,141,13,76,0,0,0                  // lea           0x4c(%rip),%r9        # 1200 <_sk_lerp_565_avx+0x178>
  .byte  75,99,4,129                         // movslq        (%r9,%r8,4),%rax
  .byte  76,1,200                            // add           %r9,%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  197,225,239,219                     // vpxor         %xmm3,%xmm3,%xmm3
  .byte  196,65,97,196,68,122,12,6           // vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm3,%xmm8
  .byte  196,65,57,196,68,122,10,5           // vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm8,%xmm8
  .byte  196,65,57,196,68,122,8,4            // vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm8,%xmm8
  .byte  196,65,57,196,68,122,6,3            // vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm8,%xmm8
  .byte  196,65,57,196,68,122,4,2            // vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm8,%xmm8
  .byte  196,65,57,196,68,122,2,1            // vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm8,%xmm8
  .byte  196,65,57,196,4,122,0               // vpinsrw       $0x0,(%r10,%rdi,2),%xmm8,%xmm8
  .byte  233,159,254,255,255                 // jmpq          109c <_sk_lerp_565_avx+0x14>
  .byte  15,31,0                             // nopl          (%rax)
  .byte  241                                 // icebp
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  233,255,255,255,225                 // jmpq          ffffffffe2001208 <_sk_linear_gradient_2stops_avx+0xffffffffe1ffeb0f>
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  217,255                             // fcos
  .byte  255                                 // (bad)
  .byte  255,209                             // callq         *%rcx
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,201                             // dec           %ecx
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  189                                 // .byte         0xbd
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // .byte         0xff

HIDDEN _sk_load_tables_avx
.globl _sk_load_tables_avx
_sk_load_tables_avx:
  .byte  85                                  // push          %rbp
  .byte  65,87                               // push          %r15
  .byte  65,86                               // push          %r14
  .byte  65,85                               // push          %r13
  .byte  65,84                               // push          %r12
  .byte  83                                  // push          %rbx
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,56,2,0,0                     // jne           146c <_sk_load_tables_avx+0x250>
  .byte  196,65,124,16,4,184                 // vmovups       (%r8,%rdi,4),%ymm8
  .byte  187,255,0,0,0                       // mov           $0xff,%ebx
  .byte  197,249,110,195                     // vmovd         %ebx,%xmm0
  .byte  197,249,112,192,0                   // vpshufd       $0x0,%xmm0,%xmm0
  .byte  196,99,125,24,200,1                 // vinsertf128   $0x1,%xmm0,%ymm0,%ymm9
  .byte  196,193,52,84,192                   // vandps        %ymm8,%ymm9,%ymm0
  .byte  196,193,249,126,193                 // vmovq         %xmm0,%r9
  .byte  69,137,203                          // mov           %r9d,%r11d
  .byte  196,195,249,22,194,1                // vpextrq       $0x1,%xmm0,%r10
  .byte  69,137,214                          // mov           %r10d,%r14d
  .byte  73,193,234,32                       // shr           $0x20,%r10
  .byte  73,193,233,32                       // shr           $0x20,%r9
  .byte  196,227,125,25,192,1                // vextractf128  $0x1,%ymm0,%xmm0
  .byte  196,193,249,126,196                 // vmovq         %xmm0,%r12
  .byte  69,137,231                          // mov           %r12d,%r15d
  .byte  196,227,249,22,195,1                // vpextrq       $0x1,%xmm0,%rbx
  .byte  65,137,221                          // mov           %ebx,%r13d
  .byte  72,193,235,32                       // shr           $0x20,%rbx
  .byte  73,193,236,32                       // shr           $0x20,%r12
  .byte  72,139,104,8                        // mov           0x8(%rax),%rbp
  .byte  76,139,64,16                        // mov           0x10(%rax),%r8
  .byte  196,161,122,16,68,189,0             // vmovss        0x0(%rbp,%r15,4),%xmm0
  .byte  196,163,121,33,68,165,0,16          // vinsertps     $0x10,0x0(%rbp,%r12,4),%xmm0,%xmm0
  .byte  196,161,122,16,76,173,0             // vmovss        0x0(%rbp,%r13,4),%xmm1
  .byte  196,227,121,33,193,32               // vinsertps     $0x20,%xmm1,%xmm0,%xmm0
  .byte  197,250,16,76,157,0                 // vmovss        0x0(%rbp,%rbx,4),%xmm1
  .byte  196,227,121,33,193,48               // vinsertps     $0x30,%xmm1,%xmm0,%xmm0
  .byte  196,161,122,16,76,157,0             // vmovss        0x0(%rbp,%r11,4),%xmm1
  .byte  196,163,113,33,76,141,0,16          // vinsertps     $0x10,0x0(%rbp,%r9,4),%xmm1,%xmm1
  .byte  196,161,122,16,92,181,0             // vmovss        0x0(%rbp,%r14,4),%xmm3
  .byte  196,227,113,33,203,32               // vinsertps     $0x20,%xmm3,%xmm1,%xmm1
  .byte  196,161,122,16,92,149,0             // vmovss        0x0(%rbp,%r10,4),%xmm3
  .byte  196,227,113,33,203,48               // vinsertps     $0x30,%xmm3,%xmm1,%xmm1
  .byte  196,227,117,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  .byte  196,193,113,114,208,8               // vpsrld        $0x8,%xmm8,%xmm1
  .byte  196,67,125,25,194,1                 // vextractf128  $0x1,%ymm8,%xmm10
  .byte  196,193,105,114,210,8               // vpsrld        $0x8,%xmm10,%xmm2
  .byte  196,227,117,24,202,1                // vinsertf128   $0x1,%xmm2,%ymm1,%ymm1
  .byte  197,180,84,201                      // vandps        %ymm1,%ymm9,%ymm1
  .byte  196,193,249,126,201                 // vmovq         %xmm1,%r9
  .byte  69,137,203                          // mov           %r9d,%r11d
  .byte  196,195,249,22,202,1                // vpextrq       $0x1,%xmm1,%r10
  .byte  69,137,214                          // mov           %r10d,%r14d
  .byte  73,193,234,32                       // shr           $0x20,%r10
  .byte  73,193,233,32                       // shr           $0x20,%r9
  .byte  196,227,125,25,201,1                // vextractf128  $0x1,%ymm1,%xmm1
  .byte  196,225,249,126,205                 // vmovq         %xmm1,%rbp
  .byte  65,137,239                          // mov           %ebp,%r15d
  .byte  196,227,249,22,203,1                // vpextrq       $0x1,%xmm1,%rbx
  .byte  65,137,220                          // mov           %ebx,%r12d
  .byte  72,193,235,32                       // shr           $0x20,%rbx
  .byte  72,193,237,32                       // shr           $0x20,%rbp
  .byte  196,129,122,16,12,184               // vmovss        (%r8,%r15,4),%xmm1
  .byte  196,195,113,33,12,168,16            // vinsertps     $0x10,(%r8,%rbp,4),%xmm1,%xmm1
  .byte  196,129,122,16,20,160               // vmovss        (%r8,%r12,4),%xmm2
  .byte  196,227,113,33,202,32               // vinsertps     $0x20,%xmm2,%xmm1,%xmm1
  .byte  196,193,122,16,20,152               // vmovss        (%r8,%rbx,4),%xmm2
  .byte  196,227,113,33,202,48               // vinsertps     $0x30,%xmm2,%xmm1,%xmm1
  .byte  196,129,122,16,20,152               // vmovss        (%r8,%r11,4),%xmm2
  .byte  196,131,105,33,20,136,16            // vinsertps     $0x10,(%r8,%r9,4),%xmm2,%xmm2
  .byte  196,129,122,16,28,176               // vmovss        (%r8,%r14,4),%xmm3
  .byte  196,227,105,33,211,32               // vinsertps     $0x20,%xmm3,%xmm2,%xmm2
  .byte  196,129,122,16,28,144               // vmovss        (%r8,%r10,4),%xmm3
  .byte  196,227,105,33,211,48               // vinsertps     $0x30,%xmm3,%xmm2,%xmm2
  .byte  196,227,109,24,201,1                // vinsertf128   $0x1,%xmm1,%ymm2,%ymm1
  .byte  72,139,64,24                        // mov           0x18(%rax),%rax
  .byte  196,193,105,114,208,16              // vpsrld        $0x10,%xmm8,%xmm2
  .byte  196,193,97,114,210,16               // vpsrld        $0x10,%xmm10,%xmm3
  .byte  196,227,109,24,211,1                // vinsertf128   $0x1,%xmm3,%ymm2,%ymm2
  .byte  197,180,84,210                      // vandps        %ymm2,%ymm9,%ymm2
  .byte  196,193,249,126,208                 // vmovq         %xmm2,%r8
  .byte  69,137,194                          // mov           %r8d,%r10d
  .byte  196,195,249,22,209,1                // vpextrq       $0x1,%xmm2,%r9
  .byte  69,137,203                          // mov           %r9d,%r11d
  .byte  73,193,233,32                       // shr           $0x20,%r9
  .byte  73,193,232,32                       // shr           $0x20,%r8
  .byte  196,227,125,25,210,1                // vextractf128  $0x1,%ymm2,%xmm2
  .byte  196,225,249,126,213                 // vmovq         %xmm2,%rbp
  .byte  65,137,238                          // mov           %ebp,%r14d
  .byte  196,227,249,22,211,1                // vpextrq       $0x1,%xmm2,%rbx
  .byte  65,137,223                          // mov           %ebx,%r15d
  .byte  72,193,235,32                       // shr           $0x20,%rbx
  .byte  72,193,237,32                       // shr           $0x20,%rbp
  .byte  196,161,122,16,20,176               // vmovss        (%rax,%r14,4),%xmm2
  .byte  196,227,105,33,20,168,16            // vinsertps     $0x10,(%rax,%rbp,4),%xmm2,%xmm2
  .byte  196,161,122,16,28,184               // vmovss        (%rax,%r15,4),%xmm3
  .byte  196,227,105,33,211,32               // vinsertps     $0x20,%xmm3,%xmm2,%xmm2
  .byte  197,250,16,28,152                   // vmovss        (%rax,%rbx,4),%xmm3
  .byte  196,99,105,33,203,48                // vinsertps     $0x30,%xmm3,%xmm2,%xmm9
  .byte  196,161,122,16,28,144               // vmovss        (%rax,%r10,4),%xmm3
  .byte  196,163,97,33,28,128,16             // vinsertps     $0x10,(%rax,%r8,4),%xmm3,%xmm3
  .byte  196,161,122,16,20,152               // vmovss        (%rax,%r11,4),%xmm2
  .byte  196,227,97,33,210,32                // vinsertps     $0x20,%xmm2,%xmm3,%xmm2
  .byte  196,161,122,16,28,136               // vmovss        (%rax,%r9,4),%xmm3
  .byte  196,227,105,33,211,48               // vinsertps     $0x30,%xmm3,%xmm2,%xmm2
  .byte  196,195,109,24,209,1                // vinsertf128   $0x1,%xmm9,%ymm2,%ymm2
  .byte  196,193,57,114,208,24               // vpsrld        $0x18,%xmm8,%xmm8
  .byte  196,193,97,114,210,24               // vpsrld        $0x18,%xmm10,%xmm3
  .byte  196,227,61,24,219,1                 // vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  .byte  197,124,91,195                      // vcvtdq2ps     %ymm3,%ymm8
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,227,121,4,219,0                 // vpermilps     $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,93                               // pop           %r13
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  93                                  // pop           %rbp
  .byte  255,224                             // jmpq          *%rax
  .byte  137,203                             // mov           %ecx,%ebx
  .byte  128,227,7                           // and           $0x7,%bl
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  254,203                             // dec           %bl
  .byte  128,251,6                           // cmp           $0x6,%bl
  .byte  15,135,185,253,255,255              // ja            123a <_sk_load_tables_avx+0x1e>
  .byte  15,182,219                          // movzbl        %bl,%ebx
  .byte  76,141,13,137,0,0,0                 // lea           0x89(%rip),%r9        # 1514 <_sk_load_tables_avx+0x2f8>
  .byte  73,99,28,153                        // movslq        (%r9,%rbx,4),%rbx
  .byte  76,1,203                            // add           %r9,%rbx
  .byte  255,227                             // jmpq          *%rbx
  .byte  196,193,121,110,68,184,24           // vmovd         0x18(%r8,%rdi,4),%xmm0
  .byte  197,249,112,192,68                  // vpshufd       $0x44,%xmm0,%xmm0
  .byte  196,227,125,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  196,99,117,12,192,64                // vblendps      $0x40,%ymm0,%ymm1,%ymm8
  .byte  196,99,125,25,192,1                 // vextractf128  $0x1,%ymm8,%xmm0
  .byte  196,195,121,34,68,184,20,1          // vpinsrd       $0x1,0x14(%r8,%rdi,4),%xmm0,%xmm0
  .byte  196,99,61,24,192,1                  // vinsertf128   $0x1,%xmm0,%ymm8,%ymm8
  .byte  196,99,125,25,192,1                 // vextractf128  $0x1,%ymm8,%xmm0
  .byte  196,195,121,34,68,184,16,0          // vpinsrd       $0x0,0x10(%r8,%rdi,4),%xmm0,%xmm0
  .byte  196,99,61,24,192,1                  // vinsertf128   $0x1,%xmm0,%ymm8,%ymm8
  .byte  196,195,57,34,68,184,12,3           // vpinsrd       $0x3,0xc(%r8,%rdi,4),%xmm8,%xmm0
  .byte  196,99,61,12,192,15                 // vblendps      $0xf,%ymm0,%ymm8,%ymm8
  .byte  196,195,57,34,68,184,8,2            // vpinsrd       $0x2,0x8(%r8,%rdi,4),%xmm8,%xmm0
  .byte  196,99,61,12,192,15                 // vblendps      $0xf,%ymm0,%ymm8,%ymm8
  .byte  196,195,57,34,68,184,4,1            // vpinsrd       $0x1,0x4(%r8,%rdi,4),%xmm8,%xmm0
  .byte  196,99,61,12,192,15                 // vblendps      $0xf,%ymm0,%ymm8,%ymm8
  .byte  196,195,57,34,4,184,0               // vpinsrd       $0x0,(%r8,%rdi,4),%xmm8,%xmm0
  .byte  196,99,61,12,192,15                 // vblendps      $0xf,%ymm0,%ymm8,%ymm8
  .byte  233,38,253,255,255                  // jmpq          123a <_sk_load_tables_avx+0x1e>
  .byte  238                                 // out           %al,(%dx)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,224                             // jmpq          *%rax
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,210                             // callq         *%rdx
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,196                             // inc           %esp
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,176,255,255,255,156             // pushq         -0x63000001(%rax)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // .byte         0xff
  .byte  128,255,255                         // cmp           $0xff,%bh
  .byte  255                                 // .byte         0xff

HIDDEN _sk_load_a8_avx
.globl _sk_load_a8_avx
_sk_load_a8_avx:
  .byte  73,137,200                          // mov           %rcx,%r8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,1,248                            // add           %rdi,%rax
  .byte  77,133,192                          // test          %r8,%r8
  .byte  117,74                              // jne           158a <_sk_load_a8_avx+0x5a>
  .byte  197,250,126,0                       // vmovq         (%rax),%xmm0
  .byte  196,226,121,49,200                  // vpmovzxbd     %xmm0,%xmm1
  .byte  196,227,121,4,192,229               // vpermilps     $0xe5,%xmm0,%xmm0
  .byte  196,226,121,49,192                  // vpmovzxbd     %xmm0,%xmm0
  .byte  196,227,117,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  196,227,121,4,201,0                 // vpermilps     $0x0,%xmm1,%xmm1
  .byte  196,227,117,24,201,1                // vinsertf128   $0x1,%xmm1,%ymm1,%ymm1
  .byte  197,252,89,217                      // vmulps        %ymm1,%ymm0,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  76,137,193                          // mov           %r8,%rcx
  .byte  255,224                             // jmpq          *%rax
  .byte  49,201                              // xor           %ecx,%ecx
  .byte  77,137,194                          // mov           %r8,%r10
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  68,15,182,24                        // movzbl        (%rax),%r11d
  .byte  72,255,192                          // inc           %rax
  .byte  73,211,227                          // shl           %cl,%r11
  .byte  77,9,217                            // or            %r11,%r9
  .byte  72,131,193,8                        // add           $0x8,%rcx
  .byte  73,255,202                          // dec           %r10
  .byte  117,234                             // jne           1592 <_sk_load_a8_avx+0x62>
  .byte  196,193,249,110,193                 // vmovq         %r9,%xmm0
  .byte  235,149                             // jmp           1544 <_sk_load_a8_avx+0x14>

HIDDEN _sk_store_a8_avx
.globl _sk_store_a8_avx
_sk_store_a8_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,8                            // mov           (%rax),%r9
  .byte  184,0,0,127,67                      // mov           $0x437f0000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,89,195                       // vmulps        %ymm3,%ymm8,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,67,125,25,193,1                 // vextractf128  $0x1,%ymm8,%xmm9
  .byte  196,66,57,43,193                    // vpackusdw     %xmm9,%xmm8,%xmm8
  .byte  196,65,57,103,192                   // vpackuswb     %xmm8,%xmm8,%xmm8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,10                              // jne           15f1 <_sk_store_a8_avx+0x42>
  .byte  196,65,123,17,4,57                  // vmovsd        %xmm8,(%r9,%rdi,1)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  65,137,200                          // mov           %ecx,%r8d
  .byte  65,128,224,7                        // and           $0x7,%r8b
  .byte  65,254,200                          // dec           %r8b
  .byte  65,128,248,6                        // cmp           $0x6,%r8b
  .byte  119,236                             // ja            15ed <_sk_store_a8_avx+0x3e>
  .byte  196,66,121,48,192                   // vpmovzxbw     %xmm8,%xmm8
  .byte  65,15,182,192                       // movzbl        %r8b,%eax
  .byte  76,141,5,67,0,0,0                   // lea           0x43(%rip),%r8        # 1654 <_sk_store_a8_avx+0xa5>
  .byte  73,99,4,128                         // movslq        (%r8,%rax,4),%rax
  .byte  76,1,192                            // add           %r8,%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  196,67,121,20,68,57,6,12            // vpextrb       $0xc,%xmm8,0x6(%r9,%rdi,1)
  .byte  196,67,121,20,68,57,5,10            // vpextrb       $0xa,%xmm8,0x5(%r9,%rdi,1)
  .byte  196,67,121,20,68,57,4,8             // vpextrb       $0x8,%xmm8,0x4(%r9,%rdi,1)
  .byte  196,67,121,20,68,57,3,6             // vpextrb       $0x6,%xmm8,0x3(%r9,%rdi,1)
  .byte  196,67,121,20,68,57,2,4             // vpextrb       $0x4,%xmm8,0x2(%r9,%rdi,1)
  .byte  196,67,121,20,68,57,1,2             // vpextrb       $0x2,%xmm8,0x1(%r9,%rdi,1)
  .byte  196,67,121,20,4,57,0                // vpextrb       $0x0,%xmm8,(%r9,%rdi,1)
  .byte  235,154                             // jmp           15ed <_sk_store_a8_avx+0x3e>
  .byte  144                                 // nop
  .byte  246,255                             // idiv          %bh
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  238                                 // out           %al,(%dx)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,230                             // jmpq          *%rsi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  222,255                             // fdivrp        %st,%st(7)
  .byte  255                                 // (bad)
  .byte  255,214                             // callq         *%rsi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,206                             // dec           %esi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,198                             // inc           %esi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // .byte         0xff

HIDDEN _sk_load_g8_avx
.globl _sk_load_g8_avx
_sk_load_g8_avx:
  .byte  73,137,200                          // mov           %rcx,%r8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,1,248                            // add           %rdi,%rax
  .byte  77,133,192                          // test          %r8,%r8
  .byte  117,91                              // jne           16db <_sk_load_g8_avx+0x6b>
  .byte  197,250,126,0                       // vmovq         (%rax),%xmm0
  .byte  196,226,121,49,200                  // vpmovzxbd     %xmm0,%xmm1
  .byte  196,227,121,4,192,229               // vpermilps     $0xe5,%xmm0,%xmm0
  .byte  196,226,121,49,192                  // vpmovzxbd     %xmm0,%xmm0
  .byte  196,227,117,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  196,227,121,4,201,0                 // vpermilps     $0x0,%xmm1,%xmm1
  .byte  196,227,117,24,201,1                // vinsertf128   $0x1,%xmm1,%ymm1,%ymm1
  .byte  197,252,89,193                      // vmulps        %ymm1,%ymm0,%ymm0
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  196,227,121,4,201,0                 // vpermilps     $0x0,%xmm1,%xmm1
  .byte  196,227,117,24,217,1                // vinsertf128   $0x1,%xmm1,%ymm1,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,137,193                          // mov           %r8,%rcx
  .byte  197,252,40,200                      // vmovaps       %ymm0,%ymm1
  .byte  197,252,40,208                      // vmovaps       %ymm0,%ymm2
  .byte  255,224                             // jmpq          *%rax
  .byte  49,201                              // xor           %ecx,%ecx
  .byte  77,137,194                          // mov           %r8,%r10
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  68,15,182,24                        // movzbl        (%rax),%r11d
  .byte  72,255,192                          // inc           %rax
  .byte  73,211,227                          // shl           %cl,%r11
  .byte  77,9,217                            // or            %r11,%r9
  .byte  72,131,193,8                        // add           $0x8,%rcx
  .byte  73,255,202                          // dec           %r10
  .byte  117,234                             // jne           16e3 <_sk_load_g8_avx+0x73>
  .byte  196,193,249,110,193                 // vmovq         %r9,%xmm0
  .byte  235,132                             // jmp           1684 <_sk_load_g8_avx+0x14>

HIDDEN _sk_load_565_avx
.globl _sk_load_565_avx
_sk_load_565_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,16                           // mov           (%rax),%r10
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,209,0,0,0                    // jne           17df <_sk_load_565_avx+0xdf>
  .byte  196,193,122,111,4,122               // vmovdqu       (%r10,%rdi,2),%xmm0
  .byte  197,241,239,201                     // vpxor         %xmm1,%xmm1,%xmm1
  .byte  197,249,105,201                     // vpunpckhwd    %xmm1,%xmm0,%xmm1
  .byte  196,226,121,51,192                  // vpmovzxwd     %xmm0,%xmm0
  .byte  196,227,125,24,209,1                // vinsertf128   $0x1,%xmm1,%ymm0,%ymm2
  .byte  184,0,248,0,0                       // mov           $0xf800,%eax
  .byte  197,249,110,192                     // vmovd         %eax,%xmm0
  .byte  197,249,112,192,0                   // vpshufd       $0x0,%xmm0,%xmm0
  .byte  196,227,125,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  .byte  197,252,84,194                      // vandps        %ymm2,%ymm0,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  184,8,33,132,55                     // mov           $0x37842108,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  196,227,121,4,201,0                 // vpermilps     $0x0,%xmm1,%xmm1
  .byte  196,227,117,24,201,1                // vinsertf128   $0x1,%xmm1,%ymm1,%ymm1
  .byte  197,252,89,193                      // vmulps        %ymm1,%ymm0,%ymm0
  .byte  184,224,7,0,0                       // mov           $0x7e0,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  197,249,112,201,0                   // vpshufd       $0x0,%xmm1,%xmm1
  .byte  196,227,117,24,201,1                // vinsertf128   $0x1,%xmm1,%ymm1,%ymm1
  .byte  197,244,84,202                      // vandps        %ymm2,%ymm1,%ymm1
  .byte  197,252,91,201                      // vcvtdq2ps     %ymm1,%ymm1
  .byte  184,33,8,2,58                       // mov           $0x3a020821,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,227,121,4,219,0                 // vpermilps     $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  197,244,89,203                      // vmulps        %ymm3,%ymm1,%ymm1
  .byte  184,31,0,0,0                        // mov           $0x1f,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  197,249,112,219,0                   // vpshufd       $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  197,228,84,210                      // vandps        %ymm2,%ymm3,%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  184,8,33,4,61                       // mov           $0x3d042108,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,227,121,4,219,0                 // vpermilps     $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  197,236,89,211                      // vmulps        %ymm3,%ymm2,%ymm2
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,227,121,4,219,0                 // vpermilps     $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  65,137,200                          // mov           %ecx,%r8d
  .byte  65,128,224,7                        // and           $0x7,%r8b
  .byte  197,249,239,192                     // vpxor         %xmm0,%xmm0,%xmm0
  .byte  65,254,200                          // dec           %r8b
  .byte  65,128,248,6                        // cmp           $0x6,%r8b
  .byte  15,135,29,255,255,255               // ja            1714 <_sk_load_565_avx+0x14>
  .byte  69,15,182,192                       // movzbl        %r8b,%r8d
  .byte  76,141,13,74,0,0,0                  // lea           0x4a(%rip),%r9        # 184c <_sk_load_565_avx+0x14c>
  .byte  75,99,4,129                         // movslq        (%r9,%r8,4),%rax
  .byte  76,1,200                            // add           %r9,%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  197,249,239,192                     // vpxor         %xmm0,%xmm0,%xmm0
  .byte  196,193,121,196,68,122,12,6         // vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,10,5         // vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,8,4          // vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,6,3          // vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,4,2          // vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,2,1          // vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,4,122,0             // vpinsrw       $0x0,(%r10,%rdi,2),%xmm0,%xmm0
  .byte  233,201,254,255,255                 // jmpq          1714 <_sk_load_565_avx+0x14>
  .byte  144                                 // nop
  .byte  243,255                             // repz          (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  235,255                             // jmp           1851 <_sk_load_565_avx+0x151>
  .byte  255                                 // (bad)
  .byte  255,227                             // jmpq          *%rbx
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  219,255                             // (bad)
  .byte  255                                 // (bad)
  .byte  255,211                             // callq         *%rbx
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,203                             // dec           %ebx
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  191                                 // .byte         0xbf
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // .byte         0xff

HIDDEN _sk_store_565_avx
.globl _sk_store_565_avx
_sk_store_565_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,8                            // mov           (%rax),%r9
  .byte  184,0,0,248,65                      // mov           $0x41f80000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,89,200                       // vmulps        %ymm0,%ymm8,%ymm9
  .byte  196,65,125,91,201                   // vcvtps2dq     %ymm9,%ymm9
  .byte  196,193,41,114,241,11               // vpslld        $0xb,%xmm9,%xmm10
  .byte  196,67,125,25,201,1                 // vextractf128  $0x1,%ymm9,%xmm9
  .byte  196,193,49,114,241,11               // vpslld        $0xb,%xmm9,%xmm9
  .byte  196,67,45,24,201,1                  // vinsertf128   $0x1,%xmm9,%ymm10,%ymm9
  .byte  184,0,0,124,66                      // mov           $0x427c0000,%eax
  .byte  197,121,110,208                     // vmovd         %eax,%xmm10
  .byte  196,67,121,4,210,0                  // vpermilps     $0x0,%xmm10,%xmm10
  .byte  196,67,45,24,210,1                  // vinsertf128   $0x1,%xmm10,%ymm10,%ymm10
  .byte  197,44,89,209                       // vmulps        %ymm1,%ymm10,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,33,114,242,5                // vpslld        $0x5,%xmm10,%xmm11
  .byte  196,67,125,25,210,1                 // vextractf128  $0x1,%ymm10,%xmm10
  .byte  196,193,41,114,242,5                // vpslld        $0x5,%xmm10,%xmm10
  .byte  196,67,37,24,210,1                  // vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  .byte  196,65,45,86,201                    // vorpd         %ymm9,%ymm10,%ymm9
  .byte  197,60,89,194                       // vmulps        %ymm2,%ymm8,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,65,53,86,192                    // vorpd         %ymm8,%ymm9,%ymm8
  .byte  196,67,125,25,193,1                 // vextractf128  $0x1,%ymm8,%xmm9
  .byte  196,66,57,43,193                    // vpackusdw     %xmm9,%xmm8,%xmm8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,10                              // jne           1906 <_sk_store_565_avx+0x9e>
  .byte  196,65,122,127,4,121                // vmovdqu       %xmm8,(%r9,%rdi,2)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  65,137,200                          // mov           %ecx,%r8d
  .byte  65,128,224,7                        // and           $0x7,%r8b
  .byte  65,254,200                          // dec           %r8b
  .byte  65,128,248,6                        // cmp           $0x6,%r8b
  .byte  119,236                             // ja            1902 <_sk_store_565_avx+0x9a>
  .byte  65,15,182,192                       // movzbl        %r8b,%eax
  .byte  76,141,5,67,0,0,0                   // lea           0x43(%rip),%r8        # 1964 <_sk_store_565_avx+0xfc>
  .byte  73,99,4,128                         // movslq        (%r8,%rax,4),%rax
  .byte  76,1,192                            // add           %r8,%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  196,67,121,21,68,121,12,6           // vpextrw       $0x6,%xmm8,0xc(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,10,5           // vpextrw       $0x5,%xmm8,0xa(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,8,4            // vpextrw       $0x4,%xmm8,0x8(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,6,3            // vpextrw       $0x3,%xmm8,0x6(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,4,2            // vpextrw       $0x2,%xmm8,0x4(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,2,1            // vpextrw       $0x1,%xmm8,0x2(%r9,%rdi,2)
  .byte  196,67,121,21,4,121,0               // vpextrw       $0x0,%xmm8,(%r9,%rdi,2)
  .byte  235,159                             // jmp           1902 <_sk_store_565_avx+0x9a>
  .byte  144                                 // nop
  .byte  246,255                             // idiv          %bh
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  238                                 // out           %al,(%dx)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,230                             // jmpq          *%rsi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  222,255                             // fdivrp        %st,%st(7)
  .byte  255                                 // (bad)
  .byte  255,214                             // callq         *%rsi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,206                             // dec           %esi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,198                             // inc           %esi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // .byte         0xff

HIDDEN _sk_load_4444_avx
.globl _sk_load_4444_avx
_sk_load_4444_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,16                           // mov           (%rax),%r10
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,245,0,0,0                    // jne           1a83 <_sk_load_4444_avx+0x103>
  .byte  196,193,122,111,4,122               // vmovdqu       (%r10,%rdi,2),%xmm0
  .byte  197,241,239,201                     // vpxor         %xmm1,%xmm1,%xmm1
  .byte  197,249,105,201                     // vpunpckhwd    %xmm1,%xmm0,%xmm1
  .byte  196,226,121,51,192                  // vpmovzxwd     %xmm0,%xmm0
  .byte  196,99,125,24,201,1                 // vinsertf128   $0x1,%xmm1,%ymm0,%ymm9
  .byte  184,0,240,0,0                       // mov           $0xf000,%eax
  .byte  197,249,110,192                     // vmovd         %eax,%xmm0
  .byte  197,249,112,192,0                   // vpshufd       $0x0,%xmm0,%xmm0
  .byte  196,227,125,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  .byte  196,193,124,84,193                  // vandps        %ymm9,%ymm0,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  184,137,136,136,55                  // mov           $0x37888889,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  196,227,121,4,201,0                 // vpermilps     $0x0,%xmm1,%xmm1
  .byte  196,227,117,24,201,1                // vinsertf128   $0x1,%xmm1,%ymm1,%ymm1
  .byte  197,252,89,193                      // vmulps        %ymm1,%ymm0,%ymm0
  .byte  184,0,15,0,0                        // mov           $0xf00,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  197,249,112,201,0                   // vpshufd       $0x0,%xmm1,%xmm1
  .byte  196,227,117,24,201,1                // vinsertf128   $0x1,%xmm1,%ymm1,%ymm1
  .byte  196,193,116,84,201                  // vandps        %ymm9,%ymm1,%ymm1
  .byte  197,252,91,201                      // vcvtdq2ps     %ymm1,%ymm1
  .byte  184,137,136,136,57                  // mov           $0x39888889,%eax
  .byte  197,249,110,208                     // vmovd         %eax,%xmm2
  .byte  196,227,121,4,210,0                 // vpermilps     $0x0,%xmm2,%xmm2
  .byte  196,227,109,24,210,1                // vinsertf128   $0x1,%xmm2,%ymm2,%ymm2
  .byte  197,244,89,202                      // vmulps        %ymm2,%ymm1,%ymm1
  .byte  184,240,0,0,0                       // mov           $0xf0,%eax
  .byte  197,249,110,208                     // vmovd         %eax,%xmm2
  .byte  197,249,112,210,0                   // vpshufd       $0x0,%xmm2,%xmm2
  .byte  196,227,109,24,210,1                // vinsertf128   $0x1,%xmm2,%ymm2,%ymm2
  .byte  196,193,108,84,209                  // vandps        %ymm9,%ymm2,%ymm2
  .byte  197,124,91,194                      // vcvtdq2ps     %ymm2,%ymm8
  .byte  184,137,136,136,59                  // mov           $0x3b888889,%eax
  .byte  197,249,110,208                     // vmovd         %eax,%xmm2
  .byte  196,227,121,4,210,0                 // vpermilps     $0x0,%xmm2,%xmm2
  .byte  196,227,109,24,210,1                // vinsertf128   $0x1,%xmm2,%ymm2,%ymm2
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  184,15,0,0,0                        // mov           $0xf,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  197,249,112,219,0                   // vpshufd       $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  196,193,100,84,217                  // vandps        %ymm9,%ymm3,%ymm3
  .byte  197,124,91,195                      // vcvtdq2ps     %ymm3,%ymm8
  .byte  184,137,136,136,61                  // mov           $0x3d888889,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,227,121,4,219,0                 // vpermilps     $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  65,137,200                          // mov           %ecx,%r8d
  .byte  65,128,224,7                        // and           $0x7,%r8b
  .byte  197,249,239,192                     // vpxor         %xmm0,%xmm0,%xmm0
  .byte  65,254,200                          // dec           %r8b
  .byte  65,128,248,6                        // cmp           $0x6,%r8b
  .byte  15,135,249,254,255,255              // ja            1994 <_sk_load_4444_avx+0x14>
  .byte  69,15,182,192                       // movzbl        %r8b,%r8d
  .byte  76,141,13,74,0,0,0                  // lea           0x4a(%rip),%r9        # 1af0 <_sk_load_4444_avx+0x170>
  .byte  75,99,4,129                         // movslq        (%r9,%r8,4),%rax
  .byte  76,1,200                            // add           %r9,%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  197,249,239,192                     // vpxor         %xmm0,%xmm0,%xmm0
  .byte  196,193,121,196,68,122,12,6         // vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,10,5         // vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,8,4          // vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,6,3          // vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,4,2          // vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,68,122,2,1          // vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm0,%xmm0
  .byte  196,193,121,196,4,122,0             // vpinsrw       $0x0,(%r10,%rdi,2),%xmm0,%xmm0
  .byte  233,165,254,255,255                 // jmpq          1994 <_sk_load_4444_avx+0x14>
  .byte  144                                 // nop
  .byte  243,255                             // repz          (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  235,255                             // jmp           1af5 <_sk_load_4444_avx+0x175>
  .byte  255                                 // (bad)
  .byte  255,227                             // jmpq          *%rbx
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  219,255                             // (bad)
  .byte  255                                 // (bad)
  .byte  255,211                             // callq         *%rbx
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,203                             // dec           %ebx
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  191                                 // .byte         0xbf
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // .byte         0xff

HIDDEN _sk_store_4444_avx
.globl _sk_store_4444_avx
_sk_store_4444_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,8                            // mov           (%rax),%r9
  .byte  184,0,0,112,65                      // mov           $0x41700000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,89,200                       // vmulps        %ymm0,%ymm8,%ymm9
  .byte  196,65,125,91,201                   // vcvtps2dq     %ymm9,%ymm9
  .byte  196,193,41,114,241,12               // vpslld        $0xc,%xmm9,%xmm10
  .byte  196,67,125,25,201,1                 // vextractf128  $0x1,%ymm9,%xmm9
  .byte  196,193,49,114,241,12               // vpslld        $0xc,%xmm9,%xmm9
  .byte  196,67,45,24,201,1                  // vinsertf128   $0x1,%xmm9,%ymm10,%ymm9
  .byte  197,60,89,209                       // vmulps        %ymm1,%ymm8,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,33,114,242,8                // vpslld        $0x8,%xmm10,%xmm11
  .byte  196,67,125,25,210,1                 // vextractf128  $0x1,%ymm10,%xmm10
  .byte  196,193,41,114,242,8                // vpslld        $0x8,%xmm10,%xmm10
  .byte  196,67,37,24,210,1                  // vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  .byte  196,65,45,86,201                    // vorpd         %ymm9,%ymm10,%ymm9
  .byte  197,60,89,210                       // vmulps        %ymm2,%ymm8,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,33,114,242,4                // vpslld        $0x4,%xmm10,%xmm11
  .byte  196,67,125,25,210,1                 // vextractf128  $0x1,%ymm10,%xmm10
  .byte  196,193,41,114,242,4                // vpslld        $0x4,%xmm10,%xmm10
  .byte  196,67,37,24,210,1                  // vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  .byte  197,60,89,195                       // vmulps        %ymm3,%ymm8,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,65,45,86,192                    // vorpd         %ymm8,%ymm10,%ymm8
  .byte  196,65,53,86,192                    // vorpd         %ymm8,%ymm9,%ymm8
  .byte  196,67,125,25,193,1                 // vextractf128  $0x1,%ymm8,%xmm9
  .byte  196,66,57,43,193                    // vpackusdw     %xmm9,%xmm8,%xmm8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,10                              // jne           1bbb <_sk_store_4444_avx+0xaf>
  .byte  196,65,122,127,4,121                // vmovdqu       %xmm8,(%r9,%rdi,2)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  65,137,200                          // mov           %ecx,%r8d
  .byte  65,128,224,7                        // and           $0x7,%r8b
  .byte  65,254,200                          // dec           %r8b
  .byte  65,128,248,6                        // cmp           $0x6,%r8b
  .byte  119,236                             // ja            1bb7 <_sk_store_4444_avx+0xab>
  .byte  65,15,182,192                       // movzbl        %r8b,%eax
  .byte  76,141,5,66,0,0,0                   // lea           0x42(%rip),%r8        # 1c18 <_sk_store_4444_avx+0x10c>
  .byte  73,99,4,128                         // movslq        (%r8,%rax,4),%rax
  .byte  76,1,192                            // add           %r8,%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  196,67,121,21,68,121,12,6           // vpextrw       $0x6,%xmm8,0xc(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,10,5           // vpextrw       $0x5,%xmm8,0xa(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,8,4            // vpextrw       $0x4,%xmm8,0x8(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,6,3            // vpextrw       $0x3,%xmm8,0x6(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,4,2            // vpextrw       $0x2,%xmm8,0x4(%r9,%rdi,2)
  .byte  196,67,121,21,68,121,2,1            // vpextrw       $0x1,%xmm8,0x2(%r9,%rdi,2)
  .byte  196,67,121,21,4,121,0               // vpextrw       $0x0,%xmm8,(%r9,%rdi,2)
  .byte  235,159                             // jmp           1bb7 <_sk_store_4444_avx+0xab>
  .byte  247,255                             // idiv          %edi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  239                                 // out           %eax,(%dx)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,231                             // jmpq          *%rdi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  223,255                             // (bad)
  .byte  255                                 // (bad)
  .byte  255,215                             // callq         *%rdi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,207                             // dec           %edi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,199                             // inc           %edi
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // .byte         0xff

HIDDEN _sk_load_8888_avx
.globl _sk_load_8888_avx
_sk_load_8888_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,16                           // mov           (%rax),%r10
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,157,0,0,0                    // jne           1cdf <_sk_load_8888_avx+0xab>
  .byte  196,65,124,16,12,186                // vmovups       (%r10,%rdi,4),%ymm9
  .byte  184,255,0,0,0                       // mov           $0xff,%eax
  .byte  197,249,110,192                     // vmovd         %eax,%xmm0
  .byte  197,249,112,192,0                   // vpshufd       $0x0,%xmm0,%xmm0
  .byte  196,99,125,24,216,1                 // vinsertf128   $0x1,%xmm0,%ymm0,%ymm11
  .byte  196,193,36,84,193                   // vandps        %ymm9,%ymm11,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  196,227,121,4,201,0                 // vpermilps     $0x0,%xmm1,%xmm1
  .byte  196,99,117,24,193,1                 // vinsertf128   $0x1,%xmm1,%ymm1,%ymm8
  .byte  196,193,124,89,192                  // vmulps        %ymm8,%ymm0,%ymm0
  .byte  196,193,41,114,209,8                // vpsrld        $0x8,%xmm9,%xmm10
  .byte  196,99,125,25,203,1                 // vextractf128  $0x1,%ymm9,%xmm3
  .byte  197,241,114,211,8                   // vpsrld        $0x8,%xmm3,%xmm1
  .byte  196,227,45,24,201,1                 // vinsertf128   $0x1,%xmm1,%ymm10,%ymm1
  .byte  197,164,84,201                      // vandps        %ymm1,%ymm11,%ymm1
  .byte  197,252,91,201                      // vcvtdq2ps     %ymm1,%ymm1
  .byte  196,193,116,89,200                  // vmulps        %ymm8,%ymm1,%ymm1
  .byte  196,193,41,114,209,16               // vpsrld        $0x10,%xmm9,%xmm10
  .byte  197,233,114,211,16                  // vpsrld        $0x10,%xmm3,%xmm2
  .byte  196,227,45,24,210,1                 // vinsertf128   $0x1,%xmm2,%ymm10,%ymm2
  .byte  197,164,84,210                      // vandps        %ymm2,%ymm11,%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  196,193,108,89,208                  // vmulps        %ymm8,%ymm2,%ymm2
  .byte  196,193,49,114,209,24               // vpsrld        $0x18,%xmm9,%xmm9
  .byte  197,225,114,211,24                  // vpsrld        $0x18,%xmm3,%xmm3
  .byte  196,227,53,24,219,1                 // vinsertf128   $0x1,%xmm3,%ymm9,%ymm3
  .byte  197,252,91,219                      // vcvtdq2ps     %ymm3,%ymm3
  .byte  196,193,100,89,216                  // vmulps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  65,137,200                          // mov           %ecx,%r8d
  .byte  65,128,224,7                        // and           $0x7,%r8b
  .byte  196,65,52,87,201                    // vxorps        %ymm9,%ymm9,%ymm9
  .byte  65,254,200                          // dec           %r8b
  .byte  65,128,248,6                        // cmp           $0x6,%r8b
  .byte  15,135,80,255,255,255               // ja            1c48 <_sk_load_8888_avx+0x14>
  .byte  69,15,182,192                       // movzbl        %r8b,%r8d
  .byte  76,141,13,137,0,0,0                 // lea           0x89(%rip),%r9        # 1d8c <_sk_load_8888_avx+0x158>
  .byte  75,99,4,129                         // movslq        (%r9,%r8,4),%rax
  .byte  76,1,200                            // add           %r9,%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  196,193,121,110,68,186,24           // vmovd         0x18(%r10,%rdi,4),%xmm0
  .byte  197,249,112,192,68                  // vpshufd       $0x44,%xmm0,%xmm0
  .byte  196,227,125,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  196,99,117,12,200,64                // vblendps      $0x40,%ymm0,%ymm1,%ymm9
  .byte  196,99,125,25,200,1                 // vextractf128  $0x1,%ymm9,%xmm0
  .byte  196,195,121,34,68,186,20,1          // vpinsrd       $0x1,0x14(%r10,%rdi,4),%xmm0,%xmm0
  .byte  196,99,53,24,200,1                  // vinsertf128   $0x1,%xmm0,%ymm9,%ymm9
  .byte  196,99,125,25,200,1                 // vextractf128  $0x1,%ymm9,%xmm0
  .byte  196,195,121,34,68,186,16,0          // vpinsrd       $0x0,0x10(%r10,%rdi,4),%xmm0,%xmm0
  .byte  196,99,53,24,200,1                  // vinsertf128   $0x1,%xmm0,%ymm9,%ymm9
  .byte  196,195,49,34,68,186,12,3           // vpinsrd       $0x3,0xc(%r10,%rdi,4),%xmm9,%xmm0
  .byte  196,99,53,12,200,15                 // vblendps      $0xf,%ymm0,%ymm9,%ymm9
  .byte  196,195,49,34,68,186,8,2            // vpinsrd       $0x2,0x8(%r10,%rdi,4),%xmm9,%xmm0
  .byte  196,99,53,12,200,15                 // vblendps      $0xf,%ymm0,%ymm9,%ymm9
  .byte  196,195,49,34,68,186,4,1            // vpinsrd       $0x1,0x4(%r10,%rdi,4),%xmm9,%xmm0
  .byte  196,99,53,12,200,15                 // vblendps      $0xf,%ymm0,%ymm9,%ymm9
  .byte  196,195,49,34,4,186,0               // vpinsrd       $0x0,(%r10,%rdi,4),%xmm9,%xmm0
  .byte  196,99,53,12,200,15                 // vblendps      $0xf,%ymm0,%ymm9,%ymm9
  .byte  233,188,254,255,255                 // jmpq          1c48 <_sk_load_8888_avx+0x14>
  .byte  238                                 // out           %al,(%dx)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,224                             // jmpq          *%rax
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,210                             // callq         *%rdx
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,196                             // inc           %esp
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,176,255,255,255,156             // pushq         -0x63000001(%rax)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // .byte         0xff
  .byte  128,255,255                         // cmp           $0xff,%bh
  .byte  255                                 // .byte         0xff

HIDDEN _sk_store_8888_avx
.globl _sk_store_8888_avx
_sk_store_8888_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,8                            // mov           (%rax),%r9
  .byte  184,0,0,127,67                      // mov           $0x437f0000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,89,200                       // vmulps        %ymm0,%ymm8,%ymm9
  .byte  196,65,125,91,201                   // vcvtps2dq     %ymm9,%ymm9
  .byte  197,60,89,209                       // vmulps        %ymm1,%ymm8,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,33,114,242,8                // vpslld        $0x8,%xmm10,%xmm11
  .byte  196,67,125,25,210,1                 // vextractf128  $0x1,%ymm10,%xmm10
  .byte  196,193,41,114,242,8                // vpslld        $0x8,%xmm10,%xmm10
  .byte  196,67,37,24,210,1                  // vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  .byte  196,65,45,86,201                    // vorpd         %ymm9,%ymm10,%ymm9
  .byte  197,60,89,210                       // vmulps        %ymm2,%ymm8,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,33,114,242,16               // vpslld        $0x10,%xmm10,%xmm11
  .byte  196,67,125,25,210,1                 // vextractf128  $0x1,%ymm10,%xmm10
  .byte  196,193,41,114,242,16               // vpslld        $0x10,%xmm10,%xmm10
  .byte  196,67,37,24,210,1                  // vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  .byte  197,60,89,195                       // vmulps        %ymm3,%ymm8,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,193,33,114,240,24               // vpslld        $0x18,%xmm8,%xmm11
  .byte  196,67,125,25,192,1                 // vextractf128  $0x1,%ymm8,%xmm8
  .byte  196,193,57,114,240,24               // vpslld        $0x18,%xmm8,%xmm8
  .byte  196,67,37,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm11,%ymm8
  .byte  196,65,45,86,192                    // vorpd         %ymm8,%ymm10,%ymm8
  .byte  196,65,53,86,192                    // vorpd         %ymm8,%ymm9,%ymm8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,10                              // jne           1e4c <_sk_store_8888_avx+0xa4>
  .byte  196,65,124,17,4,185                 // vmovups       %ymm8,(%r9,%rdi,4)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  65,137,200                          // mov           %ecx,%r8d
  .byte  65,128,224,7                        // and           $0x7,%r8b
  .byte  65,254,200                          // dec           %r8b
  .byte  65,128,248,6                        // cmp           $0x6,%r8b
  .byte  119,236                             // ja            1e48 <_sk_store_8888_avx+0xa0>
  .byte  65,15,182,192                       // movzbl        %r8b,%eax
  .byte  76,141,5,85,0,0,0                   // lea           0x55(%rip),%r8        # 1ebc <_sk_store_8888_avx+0x114>
  .byte  73,99,4,128                         // movslq        (%r8,%rax,4),%rax
  .byte  76,1,192                            // add           %r8,%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  196,67,125,25,193,1                 // vextractf128  $0x1,%ymm8,%xmm9
  .byte  196,67,121,22,76,185,24,2           // vpextrd       $0x2,%xmm9,0x18(%r9,%rdi,4)
  .byte  196,67,125,25,193,1                 // vextractf128  $0x1,%ymm8,%xmm9
  .byte  196,67,121,22,76,185,20,1           // vpextrd       $0x1,%xmm9,0x14(%r9,%rdi,4)
  .byte  196,67,125,25,193,1                 // vextractf128  $0x1,%ymm8,%xmm9
  .byte  196,65,122,17,76,185,16             // vmovss        %xmm9,0x10(%r9,%rdi,4)
  .byte  196,67,121,22,68,185,12,3           // vpextrd       $0x3,%xmm8,0xc(%r9,%rdi,4)
  .byte  196,67,121,22,68,185,8,2            // vpextrd       $0x2,%xmm8,0x8(%r9,%rdi,4)
  .byte  196,67,121,22,68,185,4,1            // vpextrd       $0x1,%xmm8,0x4(%r9,%rdi,4)
  .byte  196,65,121,126,4,185                // vmovd         %xmm8,(%r9,%rdi,4)
  .byte  235,143                             // jmp           1e48 <_sk_store_8888_avx+0xa0>
  .byte  15,31,0                             // nopl          (%rax)
  .byte  245                                 // cmc
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  237                                 // in            (%dx),%eax
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,229                             // jmpq          *%rbp
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  221,255                             // (bad)
  .byte  255                                 // (bad)
  .byte  255,208                             // callq         *%rax
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255,194                             // inc           %edx
  .byte  255                                 // (bad)
  .byte  255                                 // (bad)
  .byte  255                                 // .byte         0xff
  .byte  180,255                             // mov           $0xff,%ah
  .byte  255                                 // (bad)
  .byte  255                                 // .byte         0xff

HIDDEN _sk_load_f16_avx
.globl _sk_load_f16_avx
_sk_load_f16_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,17,1,0,0                     // jne           1ff7 <_sk_load_f16_avx+0x11f>
  .byte  197,121,16,4,248                    // vmovupd       (%rax,%rdi,8),%xmm8
  .byte  197,249,16,84,248,16                // vmovupd       0x10(%rax,%rdi,8),%xmm2
  .byte  197,249,16,92,248,32                // vmovupd       0x20(%rax,%rdi,8),%xmm3
  .byte  197,122,111,76,248,48               // vmovdqu       0x30(%rax,%rdi,8),%xmm9
  .byte  197,185,97,194                      // vpunpcklwd    %xmm2,%xmm8,%xmm0
  .byte  197,185,105,210                     // vpunpckhwd    %xmm2,%xmm8,%xmm2
  .byte  196,193,97,97,201                   // vpunpcklwd    %xmm9,%xmm3,%xmm1
  .byte  196,193,97,105,217                  // vpunpckhwd    %xmm9,%xmm3,%xmm3
  .byte  197,121,97,218                      // vpunpcklwd    %xmm2,%xmm0,%xmm11
  .byte  197,121,105,194                     // vpunpckhwd    %xmm2,%xmm0,%xmm8
  .byte  197,241,97,211                      // vpunpcklwd    %xmm3,%xmm1,%xmm2
  .byte  197,113,105,203                     // vpunpckhwd    %xmm3,%xmm1,%xmm9
  .byte  197,161,108,194                     // vpunpcklqdq   %xmm2,%xmm11,%xmm0
  .byte  184,0,4,0,4                         // mov           $0x4000400,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  197,121,112,233,0                   // vpshufd       $0x0,%xmm1,%xmm13
  .byte  197,145,101,200                     // vpcmpgtw      %xmm0,%xmm13,%xmm1
  .byte  197,241,223,192                     // vpandn        %xmm0,%xmm1,%xmm0
  .byte  196,226,121,51,200                  // vpmovzxwd     %xmm0,%xmm1
  .byte  196,65,41,239,210                   // vpxor         %xmm10,%xmm10,%xmm10
  .byte  196,193,121,105,194                 // vpunpckhwd    %xmm10,%xmm0,%xmm0
  .byte  197,241,114,241,13                  // vpslld        $0xd,%xmm1,%xmm1
  .byte  197,249,114,240,13                  // vpslld        $0xd,%xmm0,%xmm0
  .byte  196,227,117,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  .byte  184,0,0,128,119                     // mov           $0x77800000,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  197,249,112,201,0                   // vpshufd       $0x0,%xmm1,%xmm1
  .byte  196,99,117,24,225,1                 // vinsertf128   $0x1,%xmm1,%ymm1,%ymm12
  .byte  197,156,89,192                      // vmulps        %ymm0,%ymm12,%ymm0
  .byte  197,161,109,202                     // vpunpckhqdq   %xmm2,%xmm11,%xmm1
  .byte  197,145,101,209                     // vpcmpgtw      %xmm1,%xmm13,%xmm2
  .byte  197,233,223,201                     // vpandn        %xmm1,%xmm2,%xmm1
  .byte  196,226,121,51,209                  // vpmovzxwd     %xmm1,%xmm2
  .byte  196,193,113,105,202                 // vpunpckhwd    %xmm10,%xmm1,%xmm1
  .byte  197,233,114,242,13                  // vpslld        $0xd,%xmm2,%xmm2
  .byte  197,241,114,241,13                  // vpslld        $0xd,%xmm1,%xmm1
  .byte  196,227,109,24,201,1                // vinsertf128   $0x1,%xmm1,%ymm2,%ymm1
  .byte  197,156,89,201                      // vmulps        %ymm1,%ymm12,%ymm1
  .byte  196,193,57,108,209                  // vpunpcklqdq   %xmm9,%xmm8,%xmm2
  .byte  197,145,101,218                     // vpcmpgtw      %xmm2,%xmm13,%xmm3
  .byte  197,225,223,210                     // vpandn        %xmm2,%xmm3,%xmm2
  .byte  196,226,121,51,218                  // vpmovzxwd     %xmm2,%xmm3
  .byte  196,193,105,105,210                 // vpunpckhwd    %xmm10,%xmm2,%xmm2
  .byte  197,225,114,243,13                  // vpslld        $0xd,%xmm3,%xmm3
  .byte  197,233,114,242,13                  // vpslld        $0xd,%xmm2,%xmm2
  .byte  196,227,101,24,210,1                // vinsertf128   $0x1,%xmm2,%ymm3,%ymm2
  .byte  197,156,89,210                      // vmulps        %ymm2,%ymm12,%ymm2
  .byte  196,65,57,109,193                   // vpunpckhqdq   %xmm9,%xmm8,%xmm8
  .byte  196,193,17,101,216                  // vpcmpgtw      %xmm8,%xmm13,%xmm3
  .byte  196,193,97,223,216                  // vpandn        %xmm8,%xmm3,%xmm3
  .byte  196,98,121,51,195                   // vpmovzxwd     %xmm3,%xmm8
  .byte  196,193,97,105,218                  // vpunpckhwd    %xmm10,%xmm3,%xmm3
  .byte  196,193,57,114,240,13               // vpslld        $0xd,%xmm8,%xmm8
  .byte  197,225,114,243,13                  // vpslld        $0xd,%xmm3,%xmm3
  .byte  196,227,61,24,219,1                 // vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  .byte  197,156,89,219                      // vmulps        %ymm3,%ymm12,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  197,123,16,4,248                    // vmovsd        (%rax,%rdi,8),%xmm8
  .byte  196,65,49,239,201                   // vpxor         %xmm9,%xmm9,%xmm9
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,79                              // je            2056 <_sk_load_f16_avx+0x17e>
  .byte  197,57,22,68,248,8                  // vmovhpd       0x8(%rax,%rdi,8),%xmm8,%xmm8
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  114,67                              // jb            2056 <_sk_load_f16_avx+0x17e>
  .byte  197,251,16,84,248,16                // vmovsd        0x10(%rax,%rdi,8),%xmm2
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  116,68                              // je            2063 <_sk_load_f16_avx+0x18b>
  .byte  197,233,22,84,248,24                // vmovhpd       0x18(%rax,%rdi,8),%xmm2,%xmm2
  .byte  72,131,249,5                        // cmp           $0x5,%rcx
  .byte  114,56                              // jb            2063 <_sk_load_f16_avx+0x18b>
  .byte  197,251,16,92,248,32                // vmovsd        0x20(%rax,%rdi,8),%xmm3
  .byte  72,131,249,5                        // cmp           $0x5,%rcx
  .byte  15,132,194,254,255,255              // je            1efd <_sk_load_f16_avx+0x25>
  .byte  197,225,22,92,248,40                // vmovhpd       0x28(%rax,%rdi,8),%xmm3,%xmm3
  .byte  72,131,249,7                        // cmp           $0x7,%rcx
  .byte  15,130,178,254,255,255              // jb            1efd <_sk_load_f16_avx+0x25>
  .byte  197,122,126,76,248,48               // vmovq         0x30(%rax,%rdi,8),%xmm9
  .byte  233,167,254,255,255                 // jmpq          1efd <_sk_load_f16_avx+0x25>
  .byte  197,225,87,219                      // vxorpd        %xmm3,%xmm3,%xmm3
  .byte  197,233,87,210                      // vxorpd        %xmm2,%xmm2,%xmm2
  .byte  233,154,254,255,255                 // jmpq          1efd <_sk_load_f16_avx+0x25>
  .byte  197,225,87,219                      // vxorpd        %xmm3,%xmm3,%xmm3
  .byte  233,145,254,255,255                 // jmpq          1efd <_sk_load_f16_avx+0x25>

HIDDEN _sk_store_f16_avx
.globl _sk_store_f16_avx
_sk_store_f16_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  184,0,0,128,7                       // mov           $0x7800000,%eax
  .byte  197,121,110,192                     // vmovd         %eax,%xmm8
  .byte  196,65,121,112,192,0                // vpshufd       $0x0,%xmm8,%xmm8
  .byte  196,67,61,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm8,%ymm8
  .byte  197,60,89,200                       // vmulps        %ymm0,%ymm8,%ymm9
  .byte  196,67,125,25,202,1                 // vextractf128  $0x1,%ymm9,%xmm10
  .byte  196,193,41,114,210,13               // vpsrld        $0xd,%xmm10,%xmm10
  .byte  196,193,49,114,209,13               // vpsrld        $0xd,%xmm9,%xmm9
  .byte  197,60,89,217                       // vmulps        %ymm1,%ymm8,%ymm11
  .byte  196,67,125,25,220,1                 // vextractf128  $0x1,%ymm11,%xmm12
  .byte  196,193,25,114,212,13               // vpsrld        $0xd,%xmm12,%xmm12
  .byte  196,193,33,114,211,13               // vpsrld        $0xd,%xmm11,%xmm11
  .byte  197,60,89,234                       // vmulps        %ymm2,%ymm8,%ymm13
  .byte  196,67,125,25,238,1                 // vextractf128  $0x1,%ymm13,%xmm14
  .byte  196,193,9,114,214,13                // vpsrld        $0xd,%xmm14,%xmm14
  .byte  196,193,17,114,213,13               // vpsrld        $0xd,%xmm13,%xmm13
  .byte  197,60,89,195                       // vmulps        %ymm3,%ymm8,%ymm8
  .byte  196,67,125,25,199,1                 // vextractf128  $0x1,%ymm8,%xmm15
  .byte  196,193,1,114,215,13                // vpsrld        $0xd,%xmm15,%xmm15
  .byte  196,193,57,114,208,13               // vpsrld        $0xd,%xmm8,%xmm8
  .byte  196,193,33,115,251,2                // vpslldq       $0x2,%xmm11,%xmm11
  .byte  196,65,33,235,201                   // vpor          %xmm9,%xmm11,%xmm9
  .byte  196,193,33,115,252,2                // vpslldq       $0x2,%xmm12,%xmm11
  .byte  196,65,33,235,226                   // vpor          %xmm10,%xmm11,%xmm12
  .byte  196,193,57,115,248,2                // vpslldq       $0x2,%xmm8,%xmm8
  .byte  196,65,57,235,197                   // vpor          %xmm13,%xmm8,%xmm8
  .byte  196,193,41,115,255,2                // vpslldq       $0x2,%xmm15,%xmm10
  .byte  196,65,41,235,238                   // vpor          %xmm14,%xmm10,%xmm13
  .byte  196,65,49,98,216                    // vpunpckldq    %xmm8,%xmm9,%xmm11
  .byte  196,65,49,106,208                   // vpunpckhdq    %xmm8,%xmm9,%xmm10
  .byte  196,65,25,98,205                    // vpunpckldq    %xmm13,%xmm12,%xmm9
  .byte  196,65,25,106,197                   // vpunpckhdq    %xmm13,%xmm12,%xmm8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,31                              // jne           2142 <_sk_store_f16_avx+0xd6>
  .byte  196,65,120,17,28,248                // vmovups       %xmm11,(%r8,%rdi,8)
  .byte  196,65,120,17,84,248,16             // vmovups       %xmm10,0x10(%r8,%rdi,8)
  .byte  196,65,120,17,76,248,32             // vmovups       %xmm9,0x20(%r8,%rdi,8)
  .byte  196,65,122,127,68,248,48            // vmovdqu       %xmm8,0x30(%r8,%rdi,8)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  196,65,121,214,28,248               // vmovq         %xmm11,(%r8,%rdi,8)
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,240                             // je            213e <_sk_store_f16_avx+0xd2>
  .byte  196,65,121,23,92,248,8              // vmovhpd       %xmm11,0x8(%r8,%rdi,8)
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  114,227                             // jb            213e <_sk_store_f16_avx+0xd2>
  .byte  196,65,121,214,84,248,16            // vmovq         %xmm10,0x10(%r8,%rdi,8)
  .byte  116,218                             // je            213e <_sk_store_f16_avx+0xd2>
  .byte  196,65,121,23,84,248,24             // vmovhpd       %xmm10,0x18(%r8,%rdi,8)
  .byte  72,131,249,5                        // cmp           $0x5,%rcx
  .byte  114,205                             // jb            213e <_sk_store_f16_avx+0xd2>
  .byte  196,65,121,214,76,248,32            // vmovq         %xmm9,0x20(%r8,%rdi,8)
  .byte  116,196                             // je            213e <_sk_store_f16_avx+0xd2>
  .byte  196,65,121,23,76,248,40             // vmovhpd       %xmm9,0x28(%r8,%rdi,8)
  .byte  72,131,249,7                        // cmp           $0x7,%rcx
  .byte  114,183                             // jb            213e <_sk_store_f16_avx+0xd2>
  .byte  196,65,121,214,68,248,48            // vmovq         %xmm8,0x30(%r8,%rdi,8)
  .byte  235,174                             // jmp           213e <_sk_store_f16_avx+0xd2>

HIDDEN _sk_store_f32_avx
.globl _sk_store_f32_avx
_sk_store_f32_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  72,141,4,189,0,0,0,0                // lea           0x0(,%rdi,4),%rax
  .byte  197,124,20,193                      // vunpcklps     %ymm1,%ymm0,%ymm8
  .byte  197,124,21,217                      // vunpckhps     %ymm1,%ymm0,%ymm11
  .byte  197,108,20,203                      // vunpcklps     %ymm3,%ymm2,%ymm9
  .byte  197,108,21,227                      // vunpckhps     %ymm3,%ymm2,%ymm12
  .byte  196,65,61,20,209                    // vunpcklpd     %ymm9,%ymm8,%ymm10
  .byte  196,65,61,21,201                    // vunpckhpd     %ymm9,%ymm8,%ymm9
  .byte  196,65,37,20,196                    // vunpcklpd     %ymm12,%ymm11,%ymm8
  .byte  196,65,37,21,220                    // vunpckhpd     %ymm12,%ymm11,%ymm11
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,55                              // jne           21fd <_sk_store_f32_avx+0x6d>
  .byte  196,67,45,24,225,1                  // vinsertf128   $0x1,%xmm9,%ymm10,%ymm12
  .byte  196,67,61,24,235,1                  // vinsertf128   $0x1,%xmm11,%ymm8,%ymm13
  .byte  196,67,45,6,201,49                  // vperm2f128    $0x31,%ymm9,%ymm10,%ymm9
  .byte  196,67,61,6,195,49                  // vperm2f128    $0x31,%ymm11,%ymm8,%ymm8
  .byte  196,65,125,17,36,128                // vmovupd       %ymm12,(%r8,%rax,4)
  .byte  196,65,125,17,108,128,32            // vmovupd       %ymm13,0x20(%r8,%rax,4)
  .byte  196,65,125,17,76,128,64             // vmovupd       %ymm9,0x40(%r8,%rax,4)
  .byte  196,65,125,17,68,128,96             // vmovupd       %ymm8,0x60(%r8,%rax,4)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax
  .byte  196,65,121,17,20,128                // vmovupd       %xmm10,(%r8,%rax,4)
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,240                             // je            21f9 <_sk_store_f32_avx+0x69>
  .byte  196,65,121,17,76,128,16             // vmovupd       %xmm9,0x10(%r8,%rax,4)
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  114,227                             // jb            21f9 <_sk_store_f32_avx+0x69>
  .byte  196,65,121,17,68,128,32             // vmovupd       %xmm8,0x20(%r8,%rax,4)
  .byte  116,218                             // je            21f9 <_sk_store_f32_avx+0x69>
  .byte  196,65,121,17,92,128,48             // vmovupd       %xmm11,0x30(%r8,%rax,4)
  .byte  72,131,249,5                        // cmp           $0x5,%rcx
  .byte  114,205                             // jb            21f9 <_sk_store_f32_avx+0x69>
  .byte  196,67,125,25,84,128,64,1           // vextractf128  $0x1,%ymm10,0x40(%r8,%rax,4)
  .byte  116,195                             // je            21f9 <_sk_store_f32_avx+0x69>
  .byte  196,67,125,25,76,128,80,1           // vextractf128  $0x1,%ymm9,0x50(%r8,%rax,4)
  .byte  72,131,249,7                        // cmp           $0x7,%rcx
  .byte  114,181                             // jb            21f9 <_sk_store_f32_avx+0x69>
  .byte  196,67,125,25,68,128,96,1           // vextractf128  $0x1,%ymm8,0x60(%r8,%rax,4)
  .byte  235,171                             // jmp           21f9 <_sk_store_f32_avx+0x69>

HIDDEN _sk_clamp_x_avx
.globl _sk_clamp_x_avx
_sk_clamp_x_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  197,60,95,200                       // vmaxps        %ymm0,%ymm8,%ymm9
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  196,99,125,25,192,1                 // vextractf128  $0x1,%ymm8,%xmm0
  .byte  196,65,41,118,210                   // vpcmpeqd      %xmm10,%xmm10,%xmm10
  .byte  196,193,121,254,194                 // vpaddd        %xmm10,%xmm0,%xmm0
  .byte  196,65,57,254,194                   // vpaddd        %xmm10,%xmm8,%xmm8
  .byte  196,227,61,24,192,1                 // vinsertf128   $0x1,%xmm0,%ymm8,%ymm0
  .byte  197,180,93,192                      // vminps        %ymm0,%ymm9,%ymm0
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_y_avx
.globl _sk_clamp_y_avx
_sk_clamp_y_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  197,60,95,201                       // vmaxps        %ymm1,%ymm8,%ymm9
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  196,99,125,25,193,1                 // vextractf128  $0x1,%ymm8,%xmm1
  .byte  196,65,41,118,210                   // vpcmpeqd      %xmm10,%xmm10,%xmm10
  .byte  196,193,113,254,202                 // vpaddd        %xmm10,%xmm1,%xmm1
  .byte  196,65,57,254,194                   // vpaddd        %xmm10,%xmm8,%xmm8
  .byte  196,227,61,24,201,1                 // vinsertf128   $0x1,%xmm1,%ymm8,%ymm1
  .byte  197,180,93,201                      // vminps        %ymm1,%ymm9,%ymm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_repeat_x_avx
.globl _sk_repeat_x_avx
_sk_repeat_x_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  196,65,124,94,200                   // vdivps        %ymm8,%ymm0,%ymm9
  .byte  196,67,125,8,201,1                  // vroundps      $0x1,%ymm9,%ymm9
  .byte  196,65,52,89,200                    // vmulps        %ymm8,%ymm9,%ymm9
  .byte  196,65,124,92,201                   // vsubps        %ymm9,%ymm0,%ymm9
  .byte  196,99,125,25,192,1                 // vextractf128  $0x1,%ymm8,%xmm0
  .byte  196,65,41,118,210                   // vpcmpeqd      %xmm10,%xmm10,%xmm10
  .byte  196,193,121,254,194                 // vpaddd        %xmm10,%xmm0,%xmm0
  .byte  196,65,57,254,194                   // vpaddd        %xmm10,%xmm8,%xmm8
  .byte  196,227,61,24,192,1                 // vinsertf128   $0x1,%xmm0,%ymm8,%ymm0
  .byte  197,180,93,192                      // vminps        %ymm0,%ymm9,%ymm0
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_repeat_y_avx
.globl _sk_repeat_y_avx
_sk_repeat_y_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  196,65,116,94,200                   // vdivps        %ymm8,%ymm1,%ymm9
  .byte  196,67,125,8,201,1                  // vroundps      $0x1,%ymm9,%ymm9
  .byte  196,65,52,89,200                    // vmulps        %ymm8,%ymm9,%ymm9
  .byte  196,65,116,92,201                   // vsubps        %ymm9,%ymm1,%ymm9
  .byte  196,99,125,25,193,1                 // vextractf128  $0x1,%ymm8,%xmm1
  .byte  196,65,41,118,210                   // vpcmpeqd      %xmm10,%xmm10,%xmm10
  .byte  196,193,113,254,202                 // vpaddd        %xmm10,%xmm1,%xmm1
  .byte  196,65,57,254,194                   // vpaddd        %xmm10,%xmm8,%xmm8
  .byte  196,227,61,24,201,1                 // vinsertf128   $0x1,%xmm1,%ymm8,%ymm1
  .byte  197,180,93,201                      // vminps        %ymm1,%ymm9,%ymm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_mirror_x_avx
.globl _sk_mirror_x_avx
_sk_mirror_x_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,121,110,0                       // vmovd         (%rax),%xmm8
  .byte  196,65,121,112,200,0                // vpshufd       $0x0,%xmm8,%xmm9
  .byte  196,67,53,24,201,1                  // vinsertf128   $0x1,%xmm9,%ymm9,%ymm9
  .byte  196,65,124,92,209                   // vsubps        %ymm9,%ymm0,%ymm10
  .byte  196,193,58,88,192                   // vaddss        %xmm8,%xmm8,%xmm0
  .byte  196,227,121,4,192,0                 // vpermilps     $0x0,%xmm0,%xmm0
  .byte  196,227,125,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  .byte  197,44,94,192                       // vdivps        %ymm0,%ymm10,%ymm8
  .byte  196,67,125,8,192,1                  // vroundps      $0x1,%ymm8,%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,172,92,192                      // vsubps        %ymm0,%ymm10,%ymm0
  .byte  196,193,124,92,193                  // vsubps        %ymm9,%ymm0,%ymm0
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  197,60,92,192                       // vsubps        %ymm0,%ymm8,%ymm8
  .byte  197,60,84,192                       // vandps        %ymm0,%ymm8,%ymm8
  .byte  196,99,125,25,200,1                 // vextractf128  $0x1,%ymm9,%xmm0
  .byte  196,65,41,118,210                   // vpcmpeqd      %xmm10,%xmm10,%xmm10
  .byte  196,193,121,254,194                 // vpaddd        %xmm10,%xmm0,%xmm0
  .byte  196,65,49,254,202                   // vpaddd        %xmm10,%xmm9,%xmm9
  .byte  196,227,53,24,192,1                 // vinsertf128   $0x1,%xmm0,%ymm9,%ymm0
  .byte  197,188,93,192                      // vminps        %ymm0,%ymm8,%ymm0
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_mirror_y_avx
.globl _sk_mirror_y_avx
_sk_mirror_y_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,121,110,0                       // vmovd         (%rax),%xmm8
  .byte  196,65,121,112,200,0                // vpshufd       $0x0,%xmm8,%xmm9
  .byte  196,67,53,24,201,1                  // vinsertf128   $0x1,%xmm9,%ymm9,%ymm9
  .byte  196,65,116,92,209                   // vsubps        %ymm9,%ymm1,%ymm10
  .byte  196,193,58,88,200                   // vaddss        %xmm8,%xmm8,%xmm1
  .byte  196,227,121,4,201,0                 // vpermilps     $0x0,%xmm1,%xmm1
  .byte  196,227,117,24,201,1                // vinsertf128   $0x1,%xmm1,%ymm1,%ymm1
  .byte  197,44,94,193                       // vdivps        %ymm1,%ymm10,%ymm8
  .byte  196,67,125,8,192,1                  // vroundps      $0x1,%ymm8,%ymm8
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,172,92,201                      // vsubps        %ymm1,%ymm10,%ymm1
  .byte  196,193,116,92,201                  // vsubps        %ymm9,%ymm1,%ymm1
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  197,60,92,193                       // vsubps        %ymm1,%ymm8,%ymm8
  .byte  197,60,84,193                       // vandps        %ymm1,%ymm8,%ymm8
  .byte  196,99,125,25,201,1                 // vextractf128  $0x1,%ymm9,%xmm1
  .byte  196,65,41,118,210                   // vpcmpeqd      %xmm10,%xmm10,%xmm10
  .byte  196,193,113,254,202                 // vpaddd        %xmm10,%xmm1,%xmm1
  .byte  196,65,49,254,202                   // vpaddd        %xmm10,%xmm9,%xmm9
  .byte  196,227,53,24,201,1                 // vinsertf128   $0x1,%xmm1,%ymm9,%ymm1
  .byte  197,188,93,201                      // vminps        %ymm1,%ymm8,%ymm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_luminance_to_alpha_avx
.globl _sk_luminance_to_alpha_avx
_sk_luminance_to_alpha_avx:
  .byte  184,208,179,89,62                   // mov           $0x3e59b3d0,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,227,121,4,219,0                 // vpermilps     $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  197,228,89,192                      // vmulps        %ymm0,%ymm3,%ymm0
  .byte  184,89,23,55,63                     // mov           $0x3f371759,%eax
  .byte  197,249,110,216                     // vmovd         %eax,%xmm3
  .byte  196,227,121,4,219,0                 // vpermilps     $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  197,228,89,201                      // vmulps        %ymm1,%ymm3,%ymm1
  .byte  197,252,88,193                      // vaddps        %ymm1,%ymm0,%ymm0
  .byte  184,152,221,147,61                  // mov           $0x3d93dd98,%eax
  .byte  197,249,110,200                     // vmovd         %eax,%xmm1
  .byte  196,227,121,4,201,0                 // vpermilps     $0x0,%xmm1,%xmm1
  .byte  196,227,117,24,201,1                // vinsertf128   $0x1,%xmm1,%ymm1,%ymm1
  .byte  197,244,89,202                      // vmulps        %ymm2,%ymm1,%ymm1
  .byte  197,252,88,217                      // vaddps        %ymm1,%ymm0,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_matrix_2x3_avx
.globl _sk_matrix_2x3_avx
_sk_matrix_2x3_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  196,98,125,24,72,8                  // vbroadcastss  0x8(%rax),%ymm9
  .byte  196,98,125,24,80,16                 // vbroadcastss  0x10(%rax),%ymm10
  .byte  197,52,89,201                       // vmulps        %ymm1,%ymm9,%ymm9
  .byte  196,65,52,88,202                    // vaddps        %ymm10,%ymm9,%ymm9
  .byte  197,60,89,192                       // vmulps        %ymm0,%ymm8,%ymm8
  .byte  196,65,60,88,193                    // vaddps        %ymm9,%ymm8,%ymm8
  .byte  196,98,125,24,72,4                  // vbroadcastss  0x4(%rax),%ymm9
  .byte  196,98,125,24,80,12                 // vbroadcastss  0xc(%rax),%ymm10
  .byte  196,98,125,24,88,20                 // vbroadcastss  0x14(%rax),%ymm11
  .byte  197,172,89,201                      // vmulps        %ymm1,%ymm10,%ymm1
  .byte  196,193,116,88,203                  // vaddps        %ymm11,%ymm1,%ymm1
  .byte  197,180,89,192                      // vmulps        %ymm0,%ymm9,%ymm0
  .byte  197,252,88,201                      // vaddps        %ymm1,%ymm0,%ymm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,124,41,192                      // vmovaps       %ymm8,%ymm0
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_matrix_3x4_avx
.globl _sk_matrix_3x4_avx
_sk_matrix_3x4_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  196,98,125,24,72,12                 // vbroadcastss  0xc(%rax),%ymm9
  .byte  196,98,125,24,80,24                 // vbroadcastss  0x18(%rax),%ymm10
  .byte  196,98,125,24,88,36                 // vbroadcastss  0x24(%rax),%ymm11
  .byte  197,44,89,210                       // vmulps        %ymm2,%ymm10,%ymm10
  .byte  196,65,44,88,211                    // vaddps        %ymm11,%ymm10,%ymm10
  .byte  197,52,89,201                       // vmulps        %ymm1,%ymm9,%ymm9
  .byte  196,65,52,88,202                    // vaddps        %ymm10,%ymm9,%ymm9
  .byte  197,60,89,192                       // vmulps        %ymm0,%ymm8,%ymm8
  .byte  196,65,60,88,193                    // vaddps        %ymm9,%ymm8,%ymm8
  .byte  196,98,125,24,72,4                  // vbroadcastss  0x4(%rax),%ymm9
  .byte  196,98,125,24,80,16                 // vbroadcastss  0x10(%rax),%ymm10
  .byte  196,98,125,24,88,28                 // vbroadcastss  0x1c(%rax),%ymm11
  .byte  196,98,125,24,96,40                 // vbroadcastss  0x28(%rax),%ymm12
  .byte  197,36,89,218                       // vmulps        %ymm2,%ymm11,%ymm11
  .byte  196,65,36,88,220                    // vaddps        %ymm12,%ymm11,%ymm11
  .byte  197,44,89,209                       // vmulps        %ymm1,%ymm10,%ymm10
  .byte  196,65,44,88,211                    // vaddps        %ymm11,%ymm10,%ymm10
  .byte  197,52,89,200                       // vmulps        %ymm0,%ymm9,%ymm9
  .byte  196,65,52,88,202                    // vaddps        %ymm10,%ymm9,%ymm9
  .byte  196,98,125,24,80,8                  // vbroadcastss  0x8(%rax),%ymm10
  .byte  196,98,125,24,88,20                 // vbroadcastss  0x14(%rax),%ymm11
  .byte  196,98,125,24,96,32                 // vbroadcastss  0x20(%rax),%ymm12
  .byte  196,98,125,24,104,44                // vbroadcastss  0x2c(%rax),%ymm13
  .byte  197,156,89,210                      // vmulps        %ymm2,%ymm12,%ymm2
  .byte  196,193,108,88,213                  // vaddps        %ymm13,%ymm2,%ymm2
  .byte  197,164,89,201                      // vmulps        %ymm1,%ymm11,%ymm1
  .byte  197,244,88,202                      // vaddps        %ymm2,%ymm1,%ymm1
  .byte  197,172,89,192                      // vmulps        %ymm0,%ymm10,%ymm0
  .byte  197,252,88,209                      // vaddps        %ymm1,%ymm0,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,124,41,192                      // vmovaps       %ymm8,%ymm0
  .byte  197,124,41,201                      // vmovaps       %ymm9,%ymm1
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_matrix_4x5_avx
.globl _sk_matrix_4x5_avx
_sk_matrix_4x5_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  196,98,125,24,72,16                 // vbroadcastss  0x10(%rax),%ymm9
  .byte  196,98,125,24,80,32                 // vbroadcastss  0x20(%rax),%ymm10
  .byte  196,98,125,24,88,48                 // vbroadcastss  0x30(%rax),%ymm11
  .byte  196,98,125,24,96,64                 // vbroadcastss  0x40(%rax),%ymm12
  .byte  197,36,89,219                       // vmulps        %ymm3,%ymm11,%ymm11
  .byte  196,65,36,88,220                    // vaddps        %ymm12,%ymm11,%ymm11
  .byte  197,44,89,210                       // vmulps        %ymm2,%ymm10,%ymm10
  .byte  196,65,44,88,211                    // vaddps        %ymm11,%ymm10,%ymm10
  .byte  197,52,89,201                       // vmulps        %ymm1,%ymm9,%ymm9
  .byte  196,65,52,88,202                    // vaddps        %ymm10,%ymm9,%ymm9
  .byte  197,60,89,192                       // vmulps        %ymm0,%ymm8,%ymm8
  .byte  196,65,60,88,193                    // vaddps        %ymm9,%ymm8,%ymm8
  .byte  196,98,125,24,72,4                  // vbroadcastss  0x4(%rax),%ymm9
  .byte  196,98,125,24,80,20                 // vbroadcastss  0x14(%rax),%ymm10
  .byte  196,98,125,24,88,36                 // vbroadcastss  0x24(%rax),%ymm11
  .byte  196,98,125,24,96,52                 // vbroadcastss  0x34(%rax),%ymm12
  .byte  196,98,125,24,104,68                // vbroadcastss  0x44(%rax),%ymm13
  .byte  197,28,89,227                       // vmulps        %ymm3,%ymm12,%ymm12
  .byte  196,65,28,88,229                    // vaddps        %ymm13,%ymm12,%ymm12
  .byte  197,36,89,218                       // vmulps        %ymm2,%ymm11,%ymm11
  .byte  196,65,36,88,220                    // vaddps        %ymm12,%ymm11,%ymm11
  .byte  197,44,89,209                       // vmulps        %ymm1,%ymm10,%ymm10
  .byte  196,65,44,88,211                    // vaddps        %ymm11,%ymm10,%ymm10
  .byte  197,52,89,200                       // vmulps        %ymm0,%ymm9,%ymm9
  .byte  196,65,52,88,202                    // vaddps        %ymm10,%ymm9,%ymm9
  .byte  196,98,125,24,80,8                  // vbroadcastss  0x8(%rax),%ymm10
  .byte  196,98,125,24,88,24                 // vbroadcastss  0x18(%rax),%ymm11
  .byte  196,98,125,24,96,40                 // vbroadcastss  0x28(%rax),%ymm12
  .byte  196,98,125,24,104,56                // vbroadcastss  0x38(%rax),%ymm13
  .byte  196,98,125,24,112,72                // vbroadcastss  0x48(%rax),%ymm14
  .byte  197,20,89,235                       // vmulps        %ymm3,%ymm13,%ymm13
  .byte  196,65,20,88,238                    // vaddps        %ymm14,%ymm13,%ymm13
  .byte  197,28,89,226                       // vmulps        %ymm2,%ymm12,%ymm12
  .byte  196,65,28,88,229                    // vaddps        %ymm13,%ymm12,%ymm12
  .byte  197,36,89,217                       // vmulps        %ymm1,%ymm11,%ymm11
  .byte  196,65,36,88,220                    // vaddps        %ymm12,%ymm11,%ymm11
  .byte  197,44,89,208                       // vmulps        %ymm0,%ymm10,%ymm10
  .byte  196,65,44,88,211                    // vaddps        %ymm11,%ymm10,%ymm10
  .byte  196,98,125,24,88,12                 // vbroadcastss  0xc(%rax),%ymm11
  .byte  196,98,125,24,96,28                 // vbroadcastss  0x1c(%rax),%ymm12
  .byte  196,98,125,24,104,44                // vbroadcastss  0x2c(%rax),%ymm13
  .byte  196,98,125,24,112,60                // vbroadcastss  0x3c(%rax),%ymm14
  .byte  196,98,125,24,120,76                // vbroadcastss  0x4c(%rax),%ymm15
  .byte  197,140,89,219                      // vmulps        %ymm3,%ymm14,%ymm3
  .byte  196,193,100,88,223                  // vaddps        %ymm15,%ymm3,%ymm3
  .byte  197,148,89,210                      // vmulps        %ymm2,%ymm13,%ymm2
  .byte  197,236,88,211                      // vaddps        %ymm3,%ymm2,%ymm2
  .byte  197,156,89,201                      // vmulps        %ymm1,%ymm12,%ymm1
  .byte  197,244,88,202                      // vaddps        %ymm2,%ymm1,%ymm1
  .byte  197,164,89,192                      // vmulps        %ymm0,%ymm11,%ymm0
  .byte  197,252,88,217                      // vaddps        %ymm1,%ymm0,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,124,41,192                      // vmovaps       %ymm8,%ymm0
  .byte  197,124,41,201                      // vmovaps       %ymm9,%ymm1
  .byte  197,124,41,210                      // vmovaps       %ymm10,%ymm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_matrix_perspective_avx
.globl _sk_matrix_perspective_avx
_sk_matrix_perspective_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  196,98,125,24,72,4                  // vbroadcastss  0x4(%rax),%ymm9
  .byte  196,98,125,24,80,8                  // vbroadcastss  0x8(%rax),%ymm10
  .byte  197,52,89,201                       // vmulps        %ymm1,%ymm9,%ymm9
  .byte  196,65,52,88,202                    // vaddps        %ymm10,%ymm9,%ymm9
  .byte  197,60,89,192                       // vmulps        %ymm0,%ymm8,%ymm8
  .byte  196,65,60,88,193                    // vaddps        %ymm9,%ymm8,%ymm8
  .byte  196,98,125,24,72,12                 // vbroadcastss  0xc(%rax),%ymm9
  .byte  196,98,125,24,80,16                 // vbroadcastss  0x10(%rax),%ymm10
  .byte  196,98,125,24,88,20                 // vbroadcastss  0x14(%rax),%ymm11
  .byte  197,44,89,209                       // vmulps        %ymm1,%ymm10,%ymm10
  .byte  196,65,44,88,211                    // vaddps        %ymm11,%ymm10,%ymm10
  .byte  197,52,89,200                       // vmulps        %ymm0,%ymm9,%ymm9
  .byte  196,65,52,88,202                    // vaddps        %ymm10,%ymm9,%ymm9
  .byte  196,98,125,24,80,24                 // vbroadcastss  0x18(%rax),%ymm10
  .byte  196,98,125,24,88,28                 // vbroadcastss  0x1c(%rax),%ymm11
  .byte  196,98,125,24,96,32                 // vbroadcastss  0x20(%rax),%ymm12
  .byte  197,164,89,201                      // vmulps        %ymm1,%ymm11,%ymm1
  .byte  196,193,116,88,204                  // vaddps        %ymm12,%ymm1,%ymm1
  .byte  197,172,89,192                      // vmulps        %ymm0,%ymm10,%ymm0
  .byte  197,252,88,193                      // vaddps        %ymm1,%ymm0,%ymm0
  .byte  197,252,83,200                      // vrcpps        %ymm0,%ymm1
  .byte  197,188,89,193                      // vmulps        %ymm1,%ymm8,%ymm0
  .byte  197,180,89,201                      // vmulps        %ymm1,%ymm9,%ymm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_linear_gradient_2stops_avx
.globl _sk_linear_gradient_2stops_avx
_sk_linear_gradient_2stops_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,226,125,24,72,16                // vbroadcastss  0x10(%rax),%ymm1
  .byte  196,226,125,24,16                   // vbroadcastss  (%rax),%ymm2
  .byte  197,244,89,200                      // vmulps        %ymm0,%ymm1,%ymm1
  .byte  197,108,88,193                      // vaddps        %ymm1,%ymm2,%ymm8
  .byte  196,226,125,24,72,20                // vbroadcastss  0x14(%rax),%ymm1
  .byte  196,226,125,24,80,4                 // vbroadcastss  0x4(%rax),%ymm2
  .byte  197,244,89,200                      // vmulps        %ymm0,%ymm1,%ymm1
  .byte  197,236,88,201                      // vaddps        %ymm1,%ymm2,%ymm1
  .byte  196,226,125,24,80,24                // vbroadcastss  0x18(%rax),%ymm2
  .byte  196,226,125,24,88,8                 // vbroadcastss  0x8(%rax),%ymm3
  .byte  197,236,89,208                      // vmulps        %ymm0,%ymm2,%ymm2
  .byte  197,228,88,210                      // vaddps        %ymm2,%ymm3,%ymm2
  .byte  196,226,125,24,88,28                // vbroadcastss  0x1c(%rax),%ymm3
  .byte  196,98,125,24,72,12                 // vbroadcastss  0xc(%rax),%ymm9
  .byte  197,228,89,192                      // vmulps        %ymm0,%ymm3,%ymm0
  .byte  197,180,88,216                      // vaddps        %ymm0,%ymm9,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,124,41,192                      // vmovaps       %ymm8,%ymm0
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_start_pipeline_sse41
.globl _sk_start_pipeline_sse41
_sk_start_pipeline_sse41:
  .byte  65,87                               // push          %r15
  .byte  65,86                               // push          %r14
  .byte  65,85                               // push          %r13
  .byte  65,84                               // push          %r12
  .byte  83                                  // push          %rbx
  .byte  73,137,207                          // mov           %rcx,%r15
  .byte  73,137,214                          // mov           %rdx,%r14
  .byte  72,137,251                          // mov           %rdi,%rbx
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  73,137,196                          // mov           %rax,%r12
  .byte  73,137,245                          // mov           %rsi,%r13
  .byte  72,141,67,4                         // lea           0x4(%rbx),%rax
  .byte  76,57,248                           // cmp           %r15,%rax
  .byte  118,5                               // jbe           28 <_sk_start_pipeline_sse41+0x28>
  .byte  72,137,216                          // mov           %rbx,%rax
  .byte  235,52                              // jmp           5c <_sk_start_pipeline_sse41+0x5c>
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,87,201                           // xorps         %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  15,87,219                           // xorps         %xmm3,%xmm3
  .byte  15,87,228                           // xorps         %xmm4,%xmm4
  .byte  15,87,237                           // xorps         %xmm5,%xmm5
  .byte  15,87,246                           // xorps         %xmm6,%xmm6
  .byte  15,87,255                           // xorps         %xmm7,%xmm7
  .byte  72,137,223                          // mov           %rbx,%rdi
  .byte  76,137,238                          // mov           %r13,%rsi
  .byte  76,137,242                          // mov           %r14,%rdx
  .byte  65,255,212                          // callq         *%r12
  .byte  72,141,67,4                         // lea           0x4(%rbx),%rax
  .byte  72,131,195,8                        // add           $0x8,%rbx
  .byte  76,57,251                           // cmp           %r15,%rbx
  .byte  72,137,195                          // mov           %rax,%rbx
  .byte  118,204                             // jbe           28 <_sk_start_pipeline_sse41+0x28>
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,93                               // pop           %r13
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  195                                 // retq

HIDDEN _sk_just_return_sse41
.globl _sk_just_return_sse41
_sk_just_return_sse41:
  .byte  195                                 // retq

HIDDEN _sk_seed_shader_sse41
.globl _sk_seed_shader_sse41
_sk_seed_shader_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  102,15,110,199                      // movd          %edi,%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  15,91,200                           // cvtdq2ps      %xmm0,%xmm1
  .byte  185,0,0,0,63                        // mov           $0x3f000000,%ecx
  .byte  102,15,110,209                      // movd          %ecx,%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  15,88,202                           // addps         %xmm2,%xmm1
  .byte  15,16,2                             // movups        (%rdx),%xmm0
  .byte  15,88,193                           // addps         %xmm1,%xmm0
  .byte  102,15,110,8                        // movd          (%rax),%xmm1
  .byte  102,15,112,201,0                    // pshufd        $0x0,%xmm1,%xmm1
  .byte  15,91,201                           // cvtdq2ps      %xmm1,%xmm1
  .byte  15,88,202                           // addps         %xmm2,%xmm1
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,87,219                           // xorps         %xmm3,%xmm3
  .byte  15,87,228                           // xorps         %xmm4,%xmm4
  .byte  15,87,237                           // xorps         %xmm5,%xmm5
  .byte  15,87,246                           // xorps         %xmm6,%xmm6
  .byte  15,87,255                           // xorps         %xmm7,%xmm7
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_constant_color_sse41
.globl _sk_constant_color_sse41
_sk_constant_color_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,16,24                            // movups        (%rax),%xmm3
  .byte  15,40,195                           // movaps        %xmm3,%xmm0
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  15,40,203                           // movaps        %xmm3,%xmm1
  .byte  15,198,201,85                       // shufps        $0x55,%xmm1,%xmm1
  .byte  15,40,211                           // movaps        %xmm3,%xmm2
  .byte  15,198,210,170                      // shufps        $0xaa,%xmm2,%xmm2
  .byte  15,198,219,255                      // shufps        $0xff,%xmm3,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clear_sse41
.globl _sk_clear_sse41
_sk_clear_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,87,201                           // xorps         %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  15,87,219                           // xorps         %xmm3,%xmm3
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_srcatop_sse41
.globl _sk_srcatop_sse41
_sk_srcatop_sse41:
  .byte  15,89,199                           // mulps         %xmm7,%xmm0
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,92,195                        // subps         %xmm3,%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,204                        // mulps         %xmm4,%xmm9
  .byte  65,15,88,193                        // addps         %xmm9,%xmm0
  .byte  15,89,207                           // mulps         %xmm7,%xmm1
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,205                        // mulps         %xmm5,%xmm9
  .byte  65,15,88,201                        // addps         %xmm9,%xmm1
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,206                        // mulps         %xmm6,%xmm9
  .byte  65,15,88,209                        // addps         %xmm9,%xmm2
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  68,15,89,199                        // mulps         %xmm7,%xmm8
  .byte  65,15,88,216                        // addps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_dstatop_sse41
.globl _sk_dstatop_sse41
_sk_dstatop_sse41:
  .byte  68,15,40,195                        // movaps        %xmm3,%xmm8
  .byte  68,15,89,196                        // mulps         %xmm4,%xmm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,200                   // movd          %eax,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  68,15,92,207                        // subps         %xmm7,%xmm9
  .byte  65,15,89,193                        // mulps         %xmm9,%xmm0
  .byte  65,15,88,192                        // addps         %xmm8,%xmm0
  .byte  68,15,40,195                        // movaps        %xmm3,%xmm8
  .byte  68,15,89,197                        // mulps         %xmm5,%xmm8
  .byte  65,15,89,201                        // mulps         %xmm9,%xmm1
  .byte  65,15,88,200                        // addps         %xmm8,%xmm1
  .byte  68,15,40,195                        // movaps        %xmm3,%xmm8
  .byte  68,15,89,198                        // mulps         %xmm6,%xmm8
  .byte  65,15,89,209                        // mulps         %xmm9,%xmm2
  .byte  65,15,88,208                        // addps         %xmm8,%xmm2
  .byte  68,15,89,203                        // mulps         %xmm3,%xmm9
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  65,15,88,217                        // addps         %xmm9,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_srcin_sse41
.globl _sk_srcin_sse41
_sk_srcin_sse41:
  .byte  15,89,199                           // mulps         %xmm7,%xmm0
  .byte  15,89,207                           // mulps         %xmm7,%xmm1
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_dstin_sse41
.globl _sk_dstin_sse41
_sk_dstin_sse41:
  .byte  15,40,195                           // movaps        %xmm3,%xmm0
  .byte  15,89,196                           // mulps         %xmm4,%xmm0
  .byte  15,40,203                           // movaps        %xmm3,%xmm1
  .byte  15,89,205                           // mulps         %xmm5,%xmm1
  .byte  15,40,211                           // movaps        %xmm3,%xmm2
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_srcout_sse41
.globl _sk_srcout_sse41
_sk_srcout_sse41:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,92,199                        // subps         %xmm7,%xmm8
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_dstout_sse41
.globl _sk_dstout_sse41
_sk_dstout_sse41:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,92,195                        // subps         %xmm3,%xmm8
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  15,89,196                           // mulps         %xmm4,%xmm0
  .byte  65,15,40,200                        // movaps        %xmm8,%xmm1
  .byte  15,89,205                           // mulps         %xmm5,%xmm1
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  68,15,89,199                        // mulps         %xmm7,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,216                        // movaps        %xmm8,%xmm3
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_srcover_sse41
.globl _sk_srcover_sse41
_sk_srcover_sse41:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,92,195                        // subps         %xmm3,%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,204                        // mulps         %xmm4,%xmm9
  .byte  65,15,88,193                        // addps         %xmm9,%xmm0
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,205                        // mulps         %xmm5,%xmm9
  .byte  65,15,88,201                        // addps         %xmm9,%xmm1
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,206                        // mulps         %xmm6,%xmm9
  .byte  65,15,88,209                        // addps         %xmm9,%xmm2
  .byte  68,15,89,199                        // mulps         %xmm7,%xmm8
  .byte  65,15,88,216                        // addps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_dstover_sse41
.globl _sk_dstover_sse41
_sk_dstover_sse41:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,92,199                        // subps         %xmm7,%xmm8
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  15,88,214                           // addps         %xmm6,%xmm2
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  15,88,223                           // addps         %xmm7,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_modulate_sse41
.globl _sk_modulate_sse41
_sk_modulate_sse41:
  .byte  15,89,196                           // mulps         %xmm4,%xmm0
  .byte  15,89,205                           // mulps         %xmm5,%xmm1
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_multiply_sse41
.globl _sk_multiply_sse41
_sk_multiply_sse41:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,92,207                        // subps         %xmm7,%xmm9
  .byte  69,15,40,209                        // movaps        %xmm9,%xmm10
  .byte  68,15,89,208                        // mulps         %xmm0,%xmm10
  .byte  68,15,92,195                        // subps         %xmm3,%xmm8
  .byte  69,15,40,216                        // movaps        %xmm8,%xmm11
  .byte  68,15,89,220                        // mulps         %xmm4,%xmm11
  .byte  69,15,88,218                        // addps         %xmm10,%xmm11
  .byte  15,89,196                           // mulps         %xmm4,%xmm0
  .byte  65,15,88,195                        // addps         %xmm11,%xmm0
  .byte  69,15,40,209                        // movaps        %xmm9,%xmm10
  .byte  68,15,89,209                        // mulps         %xmm1,%xmm10
  .byte  69,15,40,216                        // movaps        %xmm8,%xmm11
  .byte  68,15,89,221                        // mulps         %xmm5,%xmm11
  .byte  69,15,88,218                        // addps         %xmm10,%xmm11
  .byte  15,89,205                           // mulps         %xmm5,%xmm1
  .byte  65,15,88,203                        // addps         %xmm11,%xmm1
  .byte  69,15,40,209                        // movaps        %xmm9,%xmm10
  .byte  68,15,89,210                        // mulps         %xmm2,%xmm10
  .byte  69,15,40,216                        // movaps        %xmm8,%xmm11
  .byte  68,15,89,222                        // mulps         %xmm6,%xmm11
  .byte  69,15,88,218                        // addps         %xmm10,%xmm11
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  65,15,88,211                        // addps         %xmm11,%xmm2
  .byte  68,15,89,203                        // mulps         %xmm3,%xmm9
  .byte  68,15,89,199                        // mulps         %xmm7,%xmm8
  .byte  69,15,88,193                        // addps         %xmm9,%xmm8
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  65,15,88,216                        // addps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_plus__sse41
.globl _sk_plus__sse41
_sk_plus__sse41:
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  15,88,214                           // addps         %xmm6,%xmm2
  .byte  15,88,223                           // addps         %xmm7,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_screen_sse41
.globl _sk_screen_sse41
_sk_screen_sse41:
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  68,15,88,196                        // addps         %xmm4,%xmm8
  .byte  15,89,196                           // mulps         %xmm4,%xmm0
  .byte  68,15,92,192                        // subps         %xmm0,%xmm8
  .byte  68,15,40,201                        // movaps        %xmm1,%xmm9
  .byte  68,15,88,205                        // addps         %xmm5,%xmm9
  .byte  15,89,205                           // mulps         %xmm5,%xmm1
  .byte  68,15,92,201                        // subps         %xmm1,%xmm9
  .byte  68,15,40,210                        // movaps        %xmm2,%xmm10
  .byte  68,15,88,214                        // addps         %xmm6,%xmm10
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  68,15,92,210                        // subps         %xmm2,%xmm10
  .byte  68,15,40,219                        // movaps        %xmm3,%xmm11
  .byte  68,15,88,223                        // addps         %xmm7,%xmm11
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  68,15,92,219                        // subps         %xmm3,%xmm11
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  65,15,40,201                        // movaps        %xmm9,%xmm1
  .byte  65,15,40,210                        // movaps        %xmm10,%xmm2
  .byte  65,15,40,219                        // movaps        %xmm11,%xmm3
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_xor__sse41
.globl _sk_xor__sse41
_sk_xor__sse41:
  .byte  68,15,40,195                        // movaps        %xmm3,%xmm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,92,207                        // subps         %xmm7,%xmm9
  .byte  65,15,89,193                        // mulps         %xmm9,%xmm0
  .byte  65,15,92,216                        // subps         %xmm8,%xmm3
  .byte  68,15,40,211                        // movaps        %xmm3,%xmm10
  .byte  68,15,89,212                        // mulps         %xmm4,%xmm10
  .byte  65,15,88,194                        // addps         %xmm10,%xmm0
  .byte  65,15,89,201                        // mulps         %xmm9,%xmm1
  .byte  68,15,40,211                        // movaps        %xmm3,%xmm10
  .byte  68,15,89,213                        // mulps         %xmm5,%xmm10
  .byte  65,15,88,202                        // addps         %xmm10,%xmm1
  .byte  65,15,89,209                        // mulps         %xmm9,%xmm2
  .byte  68,15,40,211                        // movaps        %xmm3,%xmm10
  .byte  68,15,89,214                        // mulps         %xmm6,%xmm10
  .byte  65,15,88,210                        // addps         %xmm10,%xmm2
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  65,15,88,217                        // addps         %xmm9,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_darken_sse41
.globl _sk_darken_sse41
_sk_darken_sse41:
  .byte  68,15,40,193                        // movaps        %xmm1,%xmm8
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  68,15,89,207                        // mulps         %xmm7,%xmm9
  .byte  15,40,203                           // movaps        %xmm3,%xmm1
  .byte  15,89,204                           // mulps         %xmm4,%xmm1
  .byte  68,15,95,201                        // maxps         %xmm1,%xmm9
  .byte  65,15,92,193                        // subps         %xmm9,%xmm0
  .byte  65,15,40,200                        // movaps        %xmm8,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  68,15,89,199                        // mulps         %xmm7,%xmm8
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,89,205                        // mulps         %xmm5,%xmm9
  .byte  69,15,95,193                        // maxps         %xmm9,%xmm8
  .byte  65,15,92,200                        // subps         %xmm8,%xmm1
  .byte  68,15,40,194                        // movaps        %xmm2,%xmm8
  .byte  68,15,88,198                        // addps         %xmm6,%xmm8
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,89,206                        // mulps         %xmm6,%xmm9
  .byte  65,15,95,209                        // maxps         %xmm9,%xmm2
  .byte  68,15,92,194                        // subps         %xmm2,%xmm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  15,92,211                           // subps         %xmm3,%xmm2
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  15,88,218                           // addps         %xmm2,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_lighten_sse41
.globl _sk_lighten_sse41
_sk_lighten_sse41:
  .byte  68,15,40,193                        // movaps        %xmm1,%xmm8
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  68,15,89,207                        // mulps         %xmm7,%xmm9
  .byte  15,40,203                           // movaps        %xmm3,%xmm1
  .byte  15,89,204                           // mulps         %xmm4,%xmm1
  .byte  68,15,93,201                        // minps         %xmm1,%xmm9
  .byte  65,15,92,193                        // subps         %xmm9,%xmm0
  .byte  65,15,40,200                        // movaps        %xmm8,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  68,15,89,199                        // mulps         %xmm7,%xmm8
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,89,205                        // mulps         %xmm5,%xmm9
  .byte  69,15,93,193                        // minps         %xmm9,%xmm8
  .byte  65,15,92,200                        // subps         %xmm8,%xmm1
  .byte  68,15,40,194                        // movaps        %xmm2,%xmm8
  .byte  68,15,88,198                        // addps         %xmm6,%xmm8
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,89,206                        // mulps         %xmm6,%xmm9
  .byte  65,15,93,209                        // minps         %xmm9,%xmm2
  .byte  68,15,92,194                        // subps         %xmm2,%xmm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  15,92,211                           // subps         %xmm3,%xmm2
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  15,88,218                           // addps         %xmm2,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_difference_sse41
.globl _sk_difference_sse41
_sk_difference_sse41:
  .byte  68,15,40,193                        // movaps        %xmm1,%xmm8
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  68,15,89,207                        // mulps         %xmm7,%xmm9
  .byte  15,40,203                           // movaps        %xmm3,%xmm1
  .byte  15,89,204                           // mulps         %xmm4,%xmm1
  .byte  68,15,93,201                        // minps         %xmm1,%xmm9
  .byte  69,15,88,201                        // addps         %xmm9,%xmm9
  .byte  65,15,92,193                        // subps         %xmm9,%xmm0
  .byte  65,15,40,200                        // movaps        %xmm8,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  68,15,89,199                        // mulps         %xmm7,%xmm8
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,89,205                        // mulps         %xmm5,%xmm9
  .byte  69,15,93,193                        // minps         %xmm9,%xmm8
  .byte  69,15,88,192                        // addps         %xmm8,%xmm8
  .byte  65,15,92,200                        // subps         %xmm8,%xmm1
  .byte  68,15,40,194                        // movaps        %xmm2,%xmm8
  .byte  68,15,88,198                        // addps         %xmm6,%xmm8
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,89,206                        // mulps         %xmm6,%xmm9
  .byte  65,15,93,209                        // minps         %xmm9,%xmm2
  .byte  15,88,210                           // addps         %xmm2,%xmm2
  .byte  68,15,92,194                        // subps         %xmm2,%xmm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  15,92,211                           // subps         %xmm3,%xmm2
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  15,88,218                           // addps         %xmm2,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_exclusion_sse41
.globl _sk_exclusion_sse41
_sk_exclusion_sse41:
  .byte  68,15,40,193                        // movaps        %xmm1,%xmm8
  .byte  15,40,200                           // movaps        %xmm0,%xmm1
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  15,89,204                           // mulps         %xmm4,%xmm1
  .byte  15,88,201                           // addps         %xmm1,%xmm1
  .byte  15,92,193                           // subps         %xmm1,%xmm0
  .byte  65,15,40,200                        // movaps        %xmm8,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  68,15,89,197                        // mulps         %xmm5,%xmm8
  .byte  69,15,88,192                        // addps         %xmm8,%xmm8
  .byte  65,15,92,200                        // subps         %xmm8,%xmm1
  .byte  68,15,40,194                        // movaps        %xmm2,%xmm8
  .byte  68,15,88,198                        // addps         %xmm6,%xmm8
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  15,88,210                           // addps         %xmm2,%xmm2
  .byte  68,15,92,194                        // subps         %xmm2,%xmm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  15,92,211                           // subps         %xmm3,%xmm2
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  15,88,218                           // addps         %xmm2,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_colorburn_sse41
.globl _sk_colorburn_sse41
_sk_colorburn_sse41:
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,208                   // movd          %eax,%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  69,15,40,218                        // movaps        %xmm10,%xmm11
  .byte  68,15,92,223                        // subps         %xmm7,%xmm11
  .byte  69,15,40,227                        // movaps        %xmm11,%xmm12
  .byte  69,15,89,225                        // mulps         %xmm9,%xmm12
  .byte  69,15,87,192                        // xorps         %xmm8,%xmm8
  .byte  15,40,199                           // movaps        %xmm7,%xmm0
  .byte  15,92,196                           // subps         %xmm4,%xmm0
  .byte  15,89,195                           // mulps         %xmm3,%xmm0
  .byte  65,15,94,193                        // divps         %xmm9,%xmm0
  .byte  68,15,40,239                        // movaps        %xmm7,%xmm13
  .byte  68,15,93,232                        // minps         %xmm0,%xmm13
  .byte  68,15,40,247                        // movaps        %xmm7,%xmm14
  .byte  69,15,92,245                        // subps         %xmm13,%xmm14
  .byte  65,15,40,193                        // movaps        %xmm9,%xmm0
  .byte  65,15,194,192,0                     // cmpeqps       %xmm8,%xmm0
  .byte  68,15,92,211                        // subps         %xmm3,%xmm10
  .byte  68,15,89,243                        // mulps         %xmm3,%xmm14
  .byte  69,15,88,244                        // addps         %xmm12,%xmm14
  .byte  102,69,15,56,20,241                 // blendvps      %xmm0,%xmm9,%xmm14
  .byte  69,15,40,202                        // movaps        %xmm10,%xmm9
  .byte  68,15,89,204                        // mulps         %xmm4,%xmm9
  .byte  68,15,88,228                        // addps         %xmm4,%xmm12
  .byte  15,40,196                           // movaps        %xmm4,%xmm0
  .byte  15,194,199,0                        // cmpeqps       %xmm7,%xmm0
  .byte  69,15,88,206                        // addps         %xmm14,%xmm9
  .byte  102,69,15,56,20,204                 // blendvps      %xmm0,%xmm12,%xmm9
  .byte  69,15,40,227                        // movaps        %xmm11,%xmm12
  .byte  68,15,89,225                        // mulps         %xmm1,%xmm12
  .byte  15,40,199                           // movaps        %xmm7,%xmm0
  .byte  15,92,197                           // subps         %xmm5,%xmm0
  .byte  15,89,195                           // mulps         %xmm3,%xmm0
  .byte  15,94,193                           // divps         %xmm1,%xmm0
  .byte  68,15,40,239                        // movaps        %xmm7,%xmm13
  .byte  68,15,93,232                        // minps         %xmm0,%xmm13
  .byte  68,15,40,247                        // movaps        %xmm7,%xmm14
  .byte  69,15,92,245                        // subps         %xmm13,%xmm14
  .byte  15,40,193                           // movaps        %xmm1,%xmm0
  .byte  65,15,194,192,0                     // cmpeqps       %xmm8,%xmm0
  .byte  68,15,89,243                        // mulps         %xmm3,%xmm14
  .byte  69,15,88,244                        // addps         %xmm12,%xmm14
  .byte  102,68,15,56,20,241                 // blendvps      %xmm0,%xmm1,%xmm14
  .byte  65,15,40,202                        // movaps        %xmm10,%xmm1
  .byte  15,89,205                           // mulps         %xmm5,%xmm1
  .byte  68,15,88,229                        // addps         %xmm5,%xmm12
  .byte  65,15,88,206                        // addps         %xmm14,%xmm1
  .byte  15,40,197                           // movaps        %xmm5,%xmm0
  .byte  15,194,199,0                        // cmpeqps       %xmm7,%xmm0
  .byte  102,65,15,56,20,204                 // blendvps      %xmm0,%xmm12,%xmm1
  .byte  15,40,199                           // movaps        %xmm7,%xmm0
  .byte  15,92,198                           // subps         %xmm6,%xmm0
  .byte  15,89,195                           // mulps         %xmm3,%xmm0
  .byte  15,94,194                           // divps         %xmm2,%xmm0
  .byte  68,15,40,231                        // movaps        %xmm7,%xmm12
  .byte  68,15,93,224                        // minps         %xmm0,%xmm12
  .byte  68,15,40,239                        // movaps        %xmm7,%xmm13
  .byte  69,15,92,236                        // subps         %xmm12,%xmm13
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  68,15,194,194,0                     // cmpeqps       %xmm2,%xmm8
  .byte  68,15,89,235                        // mulps         %xmm3,%xmm13
  .byte  69,15,88,235                        // addps         %xmm11,%xmm13
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  102,68,15,56,20,234                 // blendvps      %xmm0,%xmm2,%xmm13
  .byte  68,15,88,222                        // addps         %xmm6,%xmm11
  .byte  65,15,40,210                        // movaps        %xmm10,%xmm2
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  65,15,88,213                        // addps         %xmm13,%xmm2
  .byte  15,40,198                           // movaps        %xmm6,%xmm0
  .byte  15,194,199,0                        // cmpeqps       %xmm7,%xmm0
  .byte  102,65,15,56,20,211                 // blendvps      %xmm0,%xmm11,%xmm2
  .byte  68,15,89,215                        // mulps         %xmm7,%xmm10
  .byte  65,15,88,218                        // addps         %xmm10,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,193                        // movaps        %xmm9,%xmm0
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_colordodge_sse41
.globl _sk_colordodge_sse41
_sk_colordodge_sse41:
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,208                   // movd          %eax,%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  69,15,40,218                        // movaps        %xmm10,%xmm11
  .byte  68,15,92,223                        // subps         %xmm7,%xmm11
  .byte  69,15,40,227                        // movaps        %xmm11,%xmm12
  .byte  69,15,89,224                        // mulps         %xmm8,%xmm12
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,89,204                        // mulps         %xmm4,%xmm9
  .byte  15,40,195                           // movaps        %xmm3,%xmm0
  .byte  65,15,92,192                        // subps         %xmm8,%xmm0
  .byte  68,15,94,200                        // divps         %xmm0,%xmm9
  .byte  68,15,40,239                        // movaps        %xmm7,%xmm13
  .byte  68,15,40,247                        // movaps        %xmm7,%xmm14
  .byte  69,15,93,241                        // minps         %xmm9,%xmm14
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  15,194,195,0                        // cmpeqps       %xmm3,%xmm0
  .byte  68,15,89,243                        // mulps         %xmm3,%xmm14
  .byte  69,15,88,244                        // addps         %xmm12,%xmm14
  .byte  102,69,15,56,20,240                 // blendvps      %xmm0,%xmm8,%xmm14
  .byte  69,15,87,201                        // xorps         %xmm9,%xmm9
  .byte  68,15,92,211                        // subps         %xmm3,%xmm10
  .byte  69,15,40,194                        // movaps        %xmm10,%xmm8
  .byte  68,15,89,196                        // mulps         %xmm4,%xmm8
  .byte  68,15,88,228                        // addps         %xmm4,%xmm12
  .byte  69,15,88,198                        // addps         %xmm14,%xmm8
  .byte  15,40,196                           // movaps        %xmm4,%xmm0
  .byte  65,15,194,193,0                     // cmpeqps       %xmm9,%xmm0
  .byte  102,69,15,56,20,196                 // blendvps      %xmm0,%xmm12,%xmm8
  .byte  68,15,40,227                        // movaps        %xmm3,%xmm12
  .byte  68,15,89,229                        // mulps         %xmm5,%xmm12
  .byte  15,40,195                           // movaps        %xmm3,%xmm0
  .byte  15,92,193                           // subps         %xmm1,%xmm0
  .byte  68,15,94,224                        // divps         %xmm0,%xmm12
  .byte  69,15,40,243                        // movaps        %xmm11,%xmm14
  .byte  68,15,89,241                        // mulps         %xmm1,%xmm14
  .byte  69,15,93,236                        // minps         %xmm12,%xmm13
  .byte  15,40,193                           // movaps        %xmm1,%xmm0
  .byte  15,194,195,0                        // cmpeqps       %xmm3,%xmm0
  .byte  68,15,89,235                        // mulps         %xmm3,%xmm13
  .byte  69,15,88,238                        // addps         %xmm14,%xmm13
  .byte  102,68,15,56,20,233                 // blendvps      %xmm0,%xmm1,%xmm13
  .byte  65,15,40,202                        // movaps        %xmm10,%xmm1
  .byte  15,89,205                           // mulps         %xmm5,%xmm1
  .byte  68,15,88,245                        // addps         %xmm5,%xmm14
  .byte  65,15,88,205                        // addps         %xmm13,%xmm1
  .byte  15,40,197                           // movaps        %xmm5,%xmm0
  .byte  65,15,194,193,0                     // cmpeqps       %xmm9,%xmm0
  .byte  102,65,15,56,20,206                 // blendvps      %xmm0,%xmm14,%xmm1
  .byte  68,15,40,227                        // movaps        %xmm3,%xmm12
  .byte  68,15,89,230                        // mulps         %xmm6,%xmm12
  .byte  15,40,195                           // movaps        %xmm3,%xmm0
  .byte  15,92,194                           // subps         %xmm2,%xmm0
  .byte  68,15,94,224                        // divps         %xmm0,%xmm12
  .byte  68,15,40,239                        // movaps        %xmm7,%xmm13
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  69,15,93,236                        // minps         %xmm12,%xmm13
  .byte  15,40,194                           // movaps        %xmm2,%xmm0
  .byte  15,194,195,0                        // cmpeqps       %xmm3,%xmm0
  .byte  68,15,89,235                        // mulps         %xmm3,%xmm13
  .byte  69,15,88,235                        // addps         %xmm11,%xmm13
  .byte  102,68,15,56,20,234                 // blendvps      %xmm0,%xmm2,%xmm13
  .byte  65,15,40,210                        // movaps        %xmm10,%xmm2
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  65,15,88,213                        // addps         %xmm13,%xmm2
  .byte  68,15,194,206,0                     // cmpeqps       %xmm6,%xmm9
  .byte  68,15,88,222                        // addps         %xmm6,%xmm11
  .byte  65,15,40,193                        // movaps        %xmm9,%xmm0
  .byte  102,65,15,56,20,211                 // blendvps      %xmm0,%xmm11,%xmm2
  .byte  68,15,89,215                        // mulps         %xmm7,%xmm10
  .byte  65,15,88,218                        // addps         %xmm10,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_hardlight_sse41
.globl _sk_hardlight_sse41
_sk_hardlight_sse41:
  .byte  15,41,116,36,232                    // movaps        %xmm6,-0x18(%rsp)
  .byte  68,15,40,229                        // movaps        %xmm5,%xmm12
  .byte  15,40,244                           // movaps        %xmm4,%xmm6
  .byte  15,40,227                           // movaps        %xmm3,%xmm4
  .byte  15,40,234                           // movaps        %xmm2,%xmm5
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,208                   // movd          %eax,%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  65,15,40,210                        // movaps        %xmm10,%xmm2
  .byte  15,92,215                           // subps         %xmm7,%xmm2
  .byte  15,40,194                           // movaps        %xmm2,%xmm0
  .byte  65,15,89,193                        // mulps         %xmm9,%xmm0
  .byte  68,15,92,212                        // subps         %xmm4,%xmm10
  .byte  69,15,40,194                        // movaps        %xmm10,%xmm8
  .byte  68,15,89,198                        // mulps         %xmm6,%xmm8
  .byte  68,15,88,192                        // addps         %xmm0,%xmm8
  .byte  68,15,40,252                        // movaps        %xmm4,%xmm15
  .byte  69,15,92,249                        // subps         %xmm9,%xmm15
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  68,15,40,239                        // movaps        %xmm7,%xmm13
  .byte  68,15,40,247                        // movaps        %xmm7,%xmm14
  .byte  15,40,199                           // movaps        %xmm7,%xmm0
  .byte  15,92,198                           // subps         %xmm6,%xmm0
  .byte  65,15,89,199                        // mulps         %xmm15,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  68,15,40,251                        // movaps        %xmm3,%xmm15
  .byte  68,15,92,248                        // subps         %xmm0,%xmm15
  .byte  65,15,40,193                        // movaps        %xmm9,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,194,196,2                        // cmpleps       %xmm4,%xmm0
  .byte  68,15,89,206                        // mulps         %xmm6,%xmm9
  .byte  69,15,88,201                        // addps         %xmm9,%xmm9
  .byte  102,69,15,56,20,249                 // blendvps      %xmm0,%xmm9,%xmm15
  .byte  68,15,40,218                        // movaps        %xmm2,%xmm11
  .byte  68,15,89,217                        // mulps         %xmm1,%xmm11
  .byte  15,40,196                           // movaps        %xmm4,%xmm0
  .byte  15,92,193                           // subps         %xmm1,%xmm0
  .byte  69,15,40,204                        // movaps        %xmm12,%xmm9
  .byte  69,15,92,233                        // subps         %xmm9,%xmm13
  .byte  68,15,89,232                        // mulps         %xmm0,%xmm13
  .byte  69,15,88,237                        // addps         %xmm13,%xmm13
  .byte  68,15,40,227                        // movaps        %xmm3,%xmm12
  .byte  69,15,92,229                        // subps         %xmm13,%xmm12
  .byte  15,40,193                           // movaps        %xmm1,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,194,196,2                        // cmpleps       %xmm4,%xmm0
  .byte  65,15,89,201                        // mulps         %xmm9,%xmm1
  .byte  69,15,40,233                        // movaps        %xmm9,%xmm13
  .byte  15,88,201                           // addps         %xmm1,%xmm1
  .byte  102,68,15,56,20,225                 // blendvps      %xmm0,%xmm1,%xmm12
  .byte  65,15,40,202                        // movaps        %xmm10,%xmm1
  .byte  69,15,40,202                        // movaps        %xmm10,%xmm9
  .byte  68,15,89,215                        // mulps         %xmm7,%xmm10
  .byte  69,15,88,199                        // addps         %xmm15,%xmm8
  .byte  65,15,89,205                        // mulps         %xmm13,%xmm1
  .byte  65,15,88,203                        // addps         %xmm11,%xmm1
  .byte  65,15,88,204                        // addps         %xmm12,%xmm1
  .byte  15,89,213                           // mulps         %xmm5,%xmm2
  .byte  68,15,40,92,36,232                  // movaps        -0x18(%rsp),%xmm11
  .byte  69,15,89,203                        // mulps         %xmm11,%xmm9
  .byte  68,15,88,202                        // addps         %xmm2,%xmm9
  .byte  15,40,197                           // movaps        %xmm5,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,194,196,2                        // cmpleps       %xmm4,%xmm0
  .byte  15,40,212                           // movaps        %xmm4,%xmm2
  .byte  15,92,213                           // subps         %xmm5,%xmm2
  .byte  65,15,89,235                        // mulps         %xmm11,%xmm5
  .byte  15,88,237                           // addps         %xmm5,%xmm5
  .byte  69,15,92,243                        // subps         %xmm11,%xmm14
  .byte  68,15,89,242                        // mulps         %xmm2,%xmm14
  .byte  69,15,88,246                        // addps         %xmm14,%xmm14
  .byte  65,15,92,222                        // subps         %xmm14,%xmm3
  .byte  102,15,56,20,221                    // blendvps      %xmm0,%xmm5,%xmm3
  .byte  68,15,88,203                        // addps         %xmm3,%xmm9
  .byte  65,15,88,226                        // addps         %xmm10,%xmm4
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  65,15,40,209                        // movaps        %xmm9,%xmm2
  .byte  15,40,220                           // movaps        %xmm4,%xmm3
  .byte  15,40,230                           // movaps        %xmm6,%xmm4
  .byte  65,15,40,237                        // movaps        %xmm13,%xmm5
  .byte  65,15,40,243                        // movaps        %xmm11,%xmm6
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_overlay_sse41
.globl _sk_overlay_sse41
_sk_overlay_sse41:
  .byte  68,15,40,201                        // movaps        %xmm1,%xmm9
  .byte  68,15,40,240                        // movaps        %xmm0,%xmm14
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,208                   // movd          %eax,%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  69,15,40,218                        // movaps        %xmm10,%xmm11
  .byte  68,15,92,223                        // subps         %xmm7,%xmm11
  .byte  65,15,40,195                        // movaps        %xmm11,%xmm0
  .byte  65,15,89,198                        // mulps         %xmm14,%xmm0
  .byte  68,15,92,211                        // subps         %xmm3,%xmm10
  .byte  69,15,40,194                        // movaps        %xmm10,%xmm8
  .byte  68,15,89,196                        // mulps         %xmm4,%xmm8
  .byte  68,15,88,192                        // addps         %xmm0,%xmm8
  .byte  68,15,40,235                        // movaps        %xmm3,%xmm13
  .byte  69,15,92,238                        // subps         %xmm14,%xmm13
  .byte  68,15,89,244                        // mulps         %xmm4,%xmm14
  .byte  15,40,207                           // movaps        %xmm7,%xmm1
  .byte  15,92,204                           // subps         %xmm4,%xmm1
  .byte  15,40,196                           // movaps        %xmm4,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,194,199,2                        // cmpleps       %xmm7,%xmm0
  .byte  69,15,88,246                        // addps         %xmm14,%xmm14
  .byte  68,15,40,227                        // movaps        %xmm3,%xmm12
  .byte  68,15,89,231                        // mulps         %xmm7,%xmm12
  .byte  65,15,89,205                        // mulps         %xmm13,%xmm1
  .byte  15,88,201                           // addps         %xmm1,%xmm1
  .byte  69,15,40,236                        // movaps        %xmm12,%xmm13
  .byte  68,15,92,233                        // subps         %xmm1,%xmm13
  .byte  102,69,15,56,20,238                 // blendvps      %xmm0,%xmm14,%xmm13
  .byte  69,15,88,197                        // addps         %xmm13,%xmm8
  .byte  65,15,40,195                        // movaps        %xmm11,%xmm0
  .byte  65,15,89,193                        // mulps         %xmm9,%xmm0
  .byte  65,15,40,202                        // movaps        %xmm10,%xmm1
  .byte  15,89,205                           // mulps         %xmm5,%xmm1
  .byte  15,88,200                           // addps         %xmm0,%xmm1
  .byte  68,15,40,235                        // movaps        %xmm3,%xmm13
  .byte  69,15,92,233                        // subps         %xmm9,%xmm13
  .byte  68,15,89,205                        // mulps         %xmm5,%xmm9
  .byte  68,15,40,247                        // movaps        %xmm7,%xmm14
  .byte  68,15,92,245                        // subps         %xmm5,%xmm14
  .byte  15,40,197                           // movaps        %xmm5,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,194,199,2                        // cmpleps       %xmm7,%xmm0
  .byte  69,15,88,201                        // addps         %xmm9,%xmm9
  .byte  69,15,89,245                        // mulps         %xmm13,%xmm14
  .byte  69,15,88,246                        // addps         %xmm14,%xmm14
  .byte  69,15,40,236                        // movaps        %xmm12,%xmm13
  .byte  69,15,92,238                        // subps         %xmm14,%xmm13
  .byte  102,69,15,56,20,233                 // blendvps      %xmm0,%xmm9,%xmm13
  .byte  65,15,88,205                        // addps         %xmm13,%xmm1
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  69,15,40,202                        // movaps        %xmm10,%xmm9
  .byte  68,15,89,206                        // mulps         %xmm6,%xmm9
  .byte  69,15,88,203                        // addps         %xmm11,%xmm9
  .byte  68,15,40,219                        // movaps        %xmm3,%xmm11
  .byte  68,15,92,218                        // subps         %xmm2,%xmm11
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  68,15,40,239                        // movaps        %xmm7,%xmm13
  .byte  68,15,92,238                        // subps         %xmm6,%xmm13
  .byte  15,40,198                           // movaps        %xmm6,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,194,199,2                        // cmpleps       %xmm7,%xmm0
  .byte  15,88,210                           // addps         %xmm2,%xmm2
  .byte  69,15,89,235                        // mulps         %xmm11,%xmm13
  .byte  69,15,88,237                        // addps         %xmm13,%xmm13
  .byte  69,15,92,229                        // subps         %xmm13,%xmm12
  .byte  102,68,15,56,20,226                 // blendvps      %xmm0,%xmm2,%xmm12
  .byte  69,15,88,204                        // addps         %xmm12,%xmm9
  .byte  68,15,89,215                        // mulps         %xmm7,%xmm10
  .byte  65,15,88,218                        // addps         %xmm10,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  65,15,40,209                        // movaps        %xmm9,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_softlight_sse41
.globl _sk_softlight_sse41
_sk_softlight_sse41:
  .byte  15,41,116,36,216                    // movaps        %xmm6,-0x28(%rsp)
  .byte  15,40,244                           // movaps        %xmm4,%xmm6
  .byte  15,41,84,36,232                     // movaps        %xmm2,-0x18(%rsp)
  .byte  68,15,40,225                        // movaps        %xmm1,%xmm12
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  15,87,228                           // xorps         %xmm4,%xmm4
  .byte  15,194,231,1                        // cmpltps       %xmm7,%xmm4
  .byte  15,40,198                           // movaps        %xmm6,%xmm0
  .byte  15,94,199                           // divps         %xmm7,%xmm0
  .byte  15,84,196                           // andps         %xmm4,%xmm0
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,200                      // movd          %eax,%xmm1
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  68,15,40,209                        // movaps        %xmm1,%xmm10
  .byte  68,15,92,208                        // subps         %xmm0,%xmm10
  .byte  68,15,40,240                        // movaps        %xmm0,%xmm14
  .byte  68,15,40,248                        // movaps        %xmm0,%xmm15
  .byte  15,82,208                           // rsqrtps       %xmm0,%xmm2
  .byte  68,15,83,218                        // rcpps         %xmm2,%xmm11
  .byte  68,15,92,216                        // subps         %xmm0,%xmm11
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,40,208                           // movaps        %xmm0,%xmm2
  .byte  15,89,210                           // mulps         %xmm2,%xmm2
  .byte  15,88,208                           // addps         %xmm0,%xmm2
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  69,15,88,201                        // addps         %xmm9,%xmm9
  .byte  68,15,92,241                        // subps         %xmm1,%xmm14
  .byte  68,15,89,242                        // mulps         %xmm2,%xmm14
  .byte  184,0,0,224,64                      // mov           $0x40e00000,%eax
  .byte  102,68,15,110,232                   // movd          %eax,%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  69,15,89,253                        // mulps         %xmm13,%xmm15
  .byte  69,15,88,254                        // addps         %xmm14,%xmm15
  .byte  15,40,198                           // movaps        %xmm6,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,194,199,2                        // cmpleps       %xmm7,%xmm0
  .byte  102,69,15,56,20,223                 // blendvps      %xmm0,%xmm15,%xmm11
  .byte  65,15,40,193                        // movaps        %xmm9,%xmm0
  .byte  15,92,195                           // subps         %xmm3,%xmm0
  .byte  68,15,89,208                        // mulps         %xmm0,%xmm10
  .byte  68,15,88,211                        // addps         %xmm3,%xmm10
  .byte  68,15,89,214                        // mulps         %xmm6,%xmm10
  .byte  15,40,211                           // movaps        %xmm3,%xmm2
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  15,89,199                           // mulps         %xmm7,%xmm0
  .byte  68,15,89,216                        // mulps         %xmm0,%xmm11
  .byte  68,15,88,218                        // addps         %xmm2,%xmm11
  .byte  68,15,194,203,2                     // cmpleps       %xmm3,%xmm9
  .byte  65,15,40,193                        // movaps        %xmm9,%xmm0
  .byte  102,69,15,56,20,218                 // blendvps      %xmm0,%xmm10,%xmm11
  .byte  68,15,40,213                        // movaps        %xmm5,%xmm10
  .byte  68,15,94,215                        // divps         %xmm7,%xmm10
  .byte  68,15,84,212                        // andps         %xmm4,%xmm10
  .byte  68,15,40,201                        // movaps        %xmm1,%xmm9
  .byte  69,15,92,202                        // subps         %xmm10,%xmm9
  .byte  65,15,40,210                        // movaps        %xmm10,%xmm2
  .byte  15,88,210                           // addps         %xmm2,%xmm2
  .byte  15,88,210                           // addps         %xmm2,%xmm2
  .byte  15,40,194                           // movaps        %xmm2,%xmm0
  .byte  15,89,192                           // mulps         %xmm0,%xmm0
  .byte  15,88,194                           // addps         %xmm2,%xmm0
  .byte  65,15,40,210                        // movaps        %xmm10,%xmm2
  .byte  15,92,209                           // subps         %xmm1,%xmm2
  .byte  15,89,208                           // mulps         %xmm0,%xmm2
  .byte  65,15,82,194                        // rsqrtps       %xmm10,%xmm0
  .byte  68,15,83,240                        // rcpps         %xmm0,%xmm14
  .byte  69,15,92,242                        // subps         %xmm10,%xmm14
  .byte  69,15,89,213                        // mulps         %xmm13,%xmm10
  .byte  68,15,88,210                        // addps         %xmm2,%xmm10
  .byte  15,40,197                           // movaps        %xmm5,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,194,199,2                        // cmpleps       %xmm7,%xmm0
  .byte  102,69,15,56,20,242                 // blendvps      %xmm0,%xmm10,%xmm14
  .byte  65,15,40,196                        // movaps        %xmm12,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,40,208                           // movaps        %xmm0,%xmm2
  .byte  15,92,211                           // subps         %xmm3,%xmm2
  .byte  68,15,89,202                        // mulps         %xmm2,%xmm9
  .byte  68,15,88,203                        // addps         %xmm3,%xmm9
  .byte  15,41,108,36,200                    // movaps        %xmm5,-0x38(%rsp)
  .byte  68,15,89,205                        // mulps         %xmm5,%xmm9
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  68,15,89,242                        // mulps         %xmm2,%xmm14
  .byte  15,40,211                           // movaps        %xmm3,%xmm2
  .byte  15,89,213                           // mulps         %xmm5,%xmm2
  .byte  68,15,88,242                        // addps         %xmm2,%xmm14
  .byte  68,15,40,249                        // movaps        %xmm1,%xmm15
  .byte  15,194,195,2                        // cmpleps       %xmm3,%xmm0
  .byte  102,69,15,56,20,241                 // blendvps      %xmm0,%xmm9,%xmm14
  .byte  68,15,40,209                        // movaps        %xmm1,%xmm10
  .byte  15,40,108,36,216                    // movaps        -0x28(%rsp),%xmm5
  .byte  15,40,197                           // movaps        %xmm5,%xmm0
  .byte  15,94,199                           // divps         %xmm7,%xmm0
  .byte  15,84,196                           // andps         %xmm4,%xmm0
  .byte  15,40,208                           // movaps        %xmm0,%xmm2
  .byte  15,92,209                           // subps         %xmm1,%xmm2
  .byte  15,92,200                           // subps         %xmm0,%xmm1
  .byte  68,15,89,232                        // mulps         %xmm0,%xmm13
  .byte  15,82,224                           // rsqrtps       %xmm0,%xmm4
  .byte  68,15,83,204                        // rcpps         %xmm4,%xmm9
  .byte  68,15,92,200                        // subps         %xmm0,%xmm9
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,40,224                           // movaps        %xmm0,%xmm4
  .byte  15,89,228                           // mulps         %xmm4,%xmm4
  .byte  15,88,224                           // addps         %xmm0,%xmm4
  .byte  15,89,226                           // mulps         %xmm2,%xmm4
  .byte  68,15,88,236                        // addps         %xmm4,%xmm13
  .byte  15,40,197                           // movaps        %xmm5,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,194,199,2                        // cmpleps       %xmm7,%xmm0
  .byte  102,69,15,56,20,205                 // blendvps      %xmm0,%xmm13,%xmm9
  .byte  68,15,40,108,36,232                 // movaps        -0x18(%rsp),%xmm13
  .byte  65,15,40,197                        // movaps        %xmm13,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  15,40,208                           // movaps        %xmm0,%xmm2
  .byte  15,92,211                           // subps         %xmm3,%xmm2
  .byte  15,89,202                           // mulps         %xmm2,%xmm1
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  68,15,89,202                        // mulps         %xmm2,%xmm9
  .byte  15,40,211                           // movaps        %xmm3,%xmm2
  .byte  15,89,213                           // mulps         %xmm5,%xmm2
  .byte  68,15,88,202                        // addps         %xmm2,%xmm9
  .byte  15,88,203                           // addps         %xmm3,%xmm1
  .byte  15,89,205                           // mulps         %xmm5,%xmm1
  .byte  15,40,213                           // movaps        %xmm5,%xmm2
  .byte  15,194,195,2                        // cmpleps       %xmm3,%xmm0
  .byte  102,68,15,56,20,201                 // blendvps      %xmm0,%xmm1,%xmm9
  .byte  68,15,92,255                        // subps         %xmm7,%xmm15
  .byte  69,15,89,199                        // mulps         %xmm15,%xmm8
  .byte  69,15,89,231                        // mulps         %xmm15,%xmm12
  .byte  69,15,89,253                        // mulps         %xmm13,%xmm15
  .byte  68,15,92,211                        // subps         %xmm3,%xmm10
  .byte  65,15,40,194                        // movaps        %xmm10,%xmm0
  .byte  15,89,198                           // mulps         %xmm6,%xmm0
  .byte  68,15,88,192                        // addps         %xmm0,%xmm8
  .byte  69,15,88,195                        // addps         %xmm11,%xmm8
  .byte  65,15,40,194                        // movaps        %xmm10,%xmm0
  .byte  15,40,108,36,200                    // movaps        -0x38(%rsp),%xmm5
  .byte  15,89,197                           // mulps         %xmm5,%xmm0
  .byte  68,15,88,224                        // addps         %xmm0,%xmm12
  .byte  69,15,88,230                        // addps         %xmm14,%xmm12
  .byte  65,15,40,194                        // movaps        %xmm10,%xmm0
  .byte  15,89,194                           // mulps         %xmm2,%xmm0
  .byte  65,15,88,199                        // addps         %xmm15,%xmm0
  .byte  68,15,88,200                        // addps         %xmm0,%xmm9
  .byte  68,15,89,215                        // mulps         %xmm7,%xmm10
  .byte  65,15,88,218                        // addps         %xmm10,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,230                           // movaps        %xmm6,%xmm4
  .byte  15,40,242                           // movaps        %xmm2,%xmm6
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  65,15,40,204                        // movaps        %xmm12,%xmm1
  .byte  65,15,40,209                        // movaps        %xmm9,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_0_sse41
.globl _sk_clamp_0_sse41
_sk_clamp_0_sse41:
  .byte  69,15,87,192                        // xorps         %xmm8,%xmm8
  .byte  65,15,95,192                        // maxps         %xmm8,%xmm0
  .byte  65,15,95,200                        // maxps         %xmm8,%xmm1
  .byte  65,15,95,208                        // maxps         %xmm8,%xmm2
  .byte  65,15,95,216                        // maxps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_1_sse41
.globl _sk_clamp_1_sse41
_sk_clamp_1_sse41:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  65,15,93,192                        // minps         %xmm8,%xmm0
  .byte  65,15,93,200                        // minps         %xmm8,%xmm1
  .byte  65,15,93,208                        // minps         %xmm8,%xmm2
  .byte  65,15,93,216                        // minps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_a_sse41
.globl _sk_clamp_a_sse41
_sk_clamp_a_sse41:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  65,15,93,216                        // minps         %xmm8,%xmm3
  .byte  15,93,195                           // minps         %xmm3,%xmm0
  .byte  15,93,203                           // minps         %xmm3,%xmm1
  .byte  15,93,211                           // minps         %xmm3,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_set_rgb_sse41
.globl _sk_set_rgb_sse41
_sk_set_rgb_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,15,16,0                         // movss         (%rax),%xmm0
  .byte  243,15,16,72,4                      // movss         0x4(%rax),%xmm1
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  243,15,16,80,8                      // movss         0x8(%rax),%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_swap_rb_sse41
.globl _sk_swap_rb_sse41
_sk_swap_rb_sse41:
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,194                           // movaps        %xmm2,%xmm0
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_swap_sse41
.globl _sk_swap_sse41
_sk_swap_sse41:
  .byte  68,15,40,195                        // movaps        %xmm3,%xmm8
  .byte  68,15,40,202                        // movaps        %xmm2,%xmm9
  .byte  68,15,40,209                        // movaps        %xmm1,%xmm10
  .byte  68,15,40,216                        // movaps        %xmm0,%xmm11
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,196                           // movaps        %xmm4,%xmm0
  .byte  15,40,205                           // movaps        %xmm5,%xmm1
  .byte  15,40,214                           // movaps        %xmm6,%xmm2
  .byte  15,40,223                           // movaps        %xmm7,%xmm3
  .byte  65,15,40,227                        // movaps        %xmm11,%xmm4
  .byte  65,15,40,234                        // movaps        %xmm10,%xmm5
  .byte  65,15,40,241                        // movaps        %xmm9,%xmm6
  .byte  65,15,40,248                        // movaps        %xmm8,%xmm7
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_move_src_dst_sse41
.globl _sk_move_src_dst_sse41
_sk_move_src_dst_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,224                           // movaps        %xmm0,%xmm4
  .byte  15,40,233                           // movaps        %xmm1,%xmm5
  .byte  15,40,242                           // movaps        %xmm2,%xmm6
  .byte  15,40,251                           // movaps        %xmm3,%xmm7
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_move_dst_src_sse41
.globl _sk_move_dst_src_sse41
_sk_move_dst_src_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,196                           // movaps        %xmm4,%xmm0
  .byte  15,40,205                           // movaps        %xmm5,%xmm1
  .byte  15,40,214                           // movaps        %xmm6,%xmm2
  .byte  15,40,223                           // movaps        %xmm7,%xmm3
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_premul_sse41
.globl _sk_premul_sse41
_sk_premul_sse41:
  .byte  15,89,195                           // mulps         %xmm3,%xmm0
  .byte  15,89,203                           // mulps         %xmm3,%xmm1
  .byte  15,89,211                           // mulps         %xmm3,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_unpremul_sse41
.globl _sk_unpremul_sse41
_sk_unpremul_sse41:
  .byte  69,15,87,192                        // xorps         %xmm8,%xmm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,200                   // movd          %eax,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  68,15,94,203                        // divps         %xmm3,%xmm9
  .byte  68,15,194,195,4                     // cmpneqps      %xmm3,%xmm8
  .byte  69,15,84,193                        // andps         %xmm9,%xmm8
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_from_srgb_sse41
.globl _sk_from_srgb_sse41
_sk_from_srgb_sse41:
  .byte  184,145,131,158,61                  // mov           $0x3d9e8391,%eax
  .byte  102,68,15,110,216                   // movd          %eax,%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  69,15,40,211                        // movaps        %xmm11,%xmm10
  .byte  68,15,89,208                        // mulps         %xmm0,%xmm10
  .byte  68,15,40,240                        // movaps        %xmm0,%xmm14
  .byte  69,15,89,246                        // mulps         %xmm14,%xmm14
  .byte  184,154,153,153,62                  // mov           $0x3e99999a,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  184,92,143,50,63                    // mov           $0x3f328f5c,%eax
  .byte  102,68,15,110,224                   // movd          %eax,%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,200                        // mulps         %xmm0,%xmm9
  .byte  69,15,88,204                        // addps         %xmm12,%xmm9
  .byte  184,10,215,35,59                    // mov           $0x3b23d70a,%eax
  .byte  102,68,15,110,232                   // movd          %eax,%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  69,15,89,206                        // mulps         %xmm14,%xmm9
  .byte  69,15,88,205                        // addps         %xmm13,%xmm9
  .byte  184,174,71,97,61                    // mov           $0x3d6147ae,%eax
  .byte  102,68,15,110,240                   // movd          %eax,%xmm14
  .byte  69,15,198,246,0                     // shufps        $0x0,%xmm14,%xmm14
  .byte  65,15,194,198,1                     // cmpltps       %xmm14,%xmm0
  .byte  102,69,15,56,20,202                 // blendvps      %xmm0,%xmm10,%xmm9
  .byte  69,15,40,251                        // movaps        %xmm11,%xmm15
  .byte  68,15,89,249                        // mulps         %xmm1,%xmm15
  .byte  15,40,193                           // movaps        %xmm1,%xmm0
  .byte  15,89,192                           // mulps         %xmm0,%xmm0
  .byte  69,15,40,208                        // movaps        %xmm8,%xmm10
  .byte  68,15,89,209                        // mulps         %xmm1,%xmm10
  .byte  69,15,88,212                        // addps         %xmm12,%xmm10
  .byte  68,15,89,208                        // mulps         %xmm0,%xmm10
  .byte  69,15,88,213                        // addps         %xmm13,%xmm10
  .byte  65,15,194,206,1                     // cmpltps       %xmm14,%xmm1
  .byte  15,40,193                           // movaps        %xmm1,%xmm0
  .byte  102,69,15,56,20,215                 // blendvps      %xmm0,%xmm15,%xmm10
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  15,40,194                           // movaps        %xmm2,%xmm0
  .byte  15,89,192                           // mulps         %xmm0,%xmm0
  .byte  68,15,89,194                        // mulps         %xmm2,%xmm8
  .byte  69,15,88,196                        // addps         %xmm12,%xmm8
  .byte  68,15,89,192                        // mulps         %xmm0,%xmm8
  .byte  69,15,88,197                        // addps         %xmm13,%xmm8
  .byte  65,15,194,214,1                     // cmpltps       %xmm14,%xmm2
  .byte  15,40,194                           // movaps        %xmm2,%xmm0
  .byte  102,69,15,56,20,195                 // blendvps      %xmm0,%xmm11,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,193                        // movaps        %xmm9,%xmm0
  .byte  65,15,40,202                        // movaps        %xmm10,%xmm1
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_to_srgb_sse41
.globl _sk_to_srgb_sse41
_sk_to_srgb_sse41:
  .byte  15,41,124,36,232                    // movaps        %xmm7,-0x18(%rsp)
  .byte  15,40,254                           // movaps        %xmm6,%xmm7
  .byte  15,40,245                           // movaps        %xmm5,%xmm6
  .byte  15,40,236                           // movaps        %xmm4,%xmm5
  .byte  15,40,227                           // movaps        %xmm3,%xmm4
  .byte  15,40,218                           // movaps        %xmm2,%xmm3
  .byte  15,40,209                           // movaps        %xmm1,%xmm2
  .byte  68,15,82,192                        // rsqrtps       %xmm0,%xmm8
  .byte  69,15,83,200                        // rcpps         %xmm8,%xmm9
  .byte  69,15,82,248                        // rsqrtps       %xmm8,%xmm15
  .byte  184,41,92,71,65                     // mov           $0x41475c29,%eax
  .byte  102,68,15,110,216                   // movd          %eax,%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  69,15,40,211                        // movaps        %xmm11,%xmm10
  .byte  68,15,89,208                        // mulps         %xmm0,%xmm10
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  184,194,135,210,62                  // mov           $0x3ed287c2,%eax
  .byte  102,68,15,110,224                   // movd          %eax,%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  184,206,111,48,63                   // mov           $0x3f306fce,%eax
  .byte  102,68,15,110,232                   // movd          %eax,%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  184,168,87,202,61                   // mov           $0x3dca57a8,%eax
  .byte  53,0,0,0,128                        // xor           $0x80000000,%eax
  .byte  102,68,15,110,240                   // movd          %eax,%xmm14
  .byte  69,15,198,246,0                     // shufps        $0x0,%xmm14,%xmm14
  .byte  69,15,89,205                        // mulps         %xmm13,%xmm9
  .byte  69,15,88,206                        // addps         %xmm14,%xmm9
  .byte  69,15,89,252                        // mulps         %xmm12,%xmm15
  .byte  69,15,88,249                        // addps         %xmm9,%xmm15
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  69,15,93,207                        // minps         %xmm15,%xmm9
  .byte  184,4,231,140,59                    // mov           $0x3b8ce704,%eax
  .byte  102,68,15,110,248                   // movd          %eax,%xmm15
  .byte  69,15,198,255,0                     // shufps        $0x0,%xmm15,%xmm15
  .byte  65,15,194,199,1                     // cmpltps       %xmm15,%xmm0
  .byte  102,69,15,56,20,202                 // blendvps      %xmm0,%xmm10,%xmm9
  .byte  68,15,82,210                        // rsqrtps       %xmm2,%xmm10
  .byte  65,15,83,194                        // rcpps         %xmm10,%xmm0
  .byte  69,15,82,210                        // rsqrtps       %xmm10,%xmm10
  .byte  65,15,89,197                        // mulps         %xmm13,%xmm0
  .byte  65,15,88,198                        // addps         %xmm14,%xmm0
  .byte  69,15,89,212                        // mulps         %xmm12,%xmm10
  .byte  68,15,88,208                        // addps         %xmm0,%xmm10
  .byte  65,15,40,200                        // movaps        %xmm8,%xmm1
  .byte  65,15,93,202                        // minps         %xmm10,%xmm1
  .byte  69,15,40,211                        // movaps        %xmm11,%xmm10
  .byte  68,15,89,210                        // mulps         %xmm2,%xmm10
  .byte  65,15,194,215,1                     // cmpltps       %xmm15,%xmm2
  .byte  15,40,194                           // movaps        %xmm2,%xmm0
  .byte  102,65,15,56,20,202                 // blendvps      %xmm0,%xmm10,%xmm1
  .byte  15,82,195                           // rsqrtps       %xmm3,%xmm0
  .byte  15,83,208                           // rcpps         %xmm0,%xmm2
  .byte  65,15,89,213                        // mulps         %xmm13,%xmm2
  .byte  65,15,88,214                        // addps         %xmm14,%xmm2
  .byte  15,82,192                           // rsqrtps       %xmm0,%xmm0
  .byte  65,15,89,196                        // mulps         %xmm12,%xmm0
  .byte  15,88,194                           // addps         %xmm2,%xmm0
  .byte  68,15,93,192                        // minps         %xmm0,%xmm8
  .byte  68,15,89,219                        // mulps         %xmm3,%xmm11
  .byte  65,15,194,223,1                     // cmpltps       %xmm15,%xmm3
  .byte  15,40,195                           // movaps        %xmm3,%xmm0
  .byte  102,69,15,56,20,195                 // blendvps      %xmm0,%xmm11,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,193                        // movaps        %xmm9,%xmm0
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  15,40,220                           // movaps        %xmm4,%xmm3
  .byte  15,40,229                           // movaps        %xmm5,%xmm4
  .byte  15,40,238                           // movaps        %xmm6,%xmm5
  .byte  15,40,247                           // movaps        %xmm7,%xmm6
  .byte  15,40,124,36,232                    // movaps        -0x18(%rsp),%xmm7
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_scale_1_float_sse41
.globl _sk_scale_1_float_sse41
_sk_scale_1_float_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,0                      // movss         (%rax),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_scale_u8_sse41
.globl _sk_scale_u8_sse41
_sk_scale_u8_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,68,15,56,49,4,56                // pmovzxbd      (%rax,%rdi,1),%xmm8
  .byte  69,15,91,192                        // cvtdq2ps      %xmm8,%xmm8
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  102,68,15,110,200                   // movd          %eax,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  65,15,89,193                        // mulps         %xmm9,%xmm0
  .byte  65,15,89,201                        // mulps         %xmm9,%xmm1
  .byte  65,15,89,209                        // mulps         %xmm9,%xmm2
  .byte  65,15,89,217                        // mulps         %xmm9,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_lerp_1_float_sse41
.globl _sk_lerp_1_float_sse41
_sk_lerp_1_float_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,0                      // movss         (%rax),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  15,92,196                           // subps         %xmm4,%xmm0
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  15,92,205                           // subps         %xmm5,%xmm1
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  15,92,214                           // subps         %xmm6,%xmm2
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  15,88,214                           // addps         %xmm6,%xmm2
  .byte  15,92,223                           // subps         %xmm7,%xmm3
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  15,88,223                           // addps         %xmm7,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_lerp_u8_sse41
.globl _sk_lerp_u8_sse41
_sk_lerp_u8_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,68,15,56,49,4,56                // pmovzxbd      (%rax,%rdi,1),%xmm8
  .byte  69,15,91,192                        // cvtdq2ps      %xmm8,%xmm8
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  102,68,15,110,200                   // movd          %eax,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  15,92,196                           // subps         %xmm4,%xmm0
  .byte  65,15,89,193                        // mulps         %xmm9,%xmm0
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  15,92,205                           // subps         %xmm5,%xmm1
  .byte  65,15,89,201                        // mulps         %xmm9,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  15,92,214                           // subps         %xmm6,%xmm2
  .byte  65,15,89,209                        // mulps         %xmm9,%xmm2
  .byte  15,88,214                           // addps         %xmm6,%xmm2
  .byte  15,92,223                           // subps         %xmm7,%xmm3
  .byte  65,15,89,217                        // mulps         %xmm9,%xmm3
  .byte  15,88,223                           // addps         %xmm7,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_lerp_565_sse41
.globl _sk_lerp_565_sse41
_sk_lerp_565_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,68,15,56,51,4,120               // pmovzxwd      (%rax,%rdi,2),%xmm8
  .byte  184,0,248,0,0                       // mov           $0xf800,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  102,15,112,219,0                    // pshufd        $0x0,%xmm3,%xmm3
  .byte  102,65,15,219,216                   // pand          %xmm8,%xmm3
  .byte  68,15,91,203                        // cvtdq2ps      %xmm3,%xmm9
  .byte  184,8,33,132,55                     // mov           $0x37842108,%eax
  .byte  102,68,15,110,208                   // movd          %eax,%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  69,15,89,209                        // mulps         %xmm9,%xmm10
  .byte  184,224,7,0,0                       // mov           $0x7e0,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  102,15,112,219,0                    // pshufd        $0x0,%xmm3,%xmm3
  .byte  102,65,15,219,216                   // pand          %xmm8,%xmm3
  .byte  68,15,91,203                        // cvtdq2ps      %xmm3,%xmm9
  .byte  184,33,8,2,58                       // mov           $0x3a020821,%eax
  .byte  102,68,15,110,216                   // movd          %eax,%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  69,15,89,217                        // mulps         %xmm9,%xmm11
  .byte  184,31,0,0,0                        // mov           $0x1f,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  102,15,112,219,0                    // pshufd        $0x0,%xmm3,%xmm3
  .byte  102,65,15,219,216                   // pand          %xmm8,%xmm3
  .byte  68,15,91,195                        // cvtdq2ps      %xmm3,%xmm8
  .byte  184,8,33,4,61                       // mov           $0x3d042108,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  15,92,196                           // subps         %xmm4,%xmm0
  .byte  65,15,89,194                        // mulps         %xmm10,%xmm0
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  15,92,205                           // subps         %xmm5,%xmm1
  .byte  65,15,89,203                        // mulps         %xmm11,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  15,92,214                           // subps         %xmm6,%xmm2
  .byte  15,89,211                           // mulps         %xmm3,%xmm2
  .byte  15,88,214                           // addps         %xmm6,%xmm2
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_load_tables_sse41
.globl _sk_load_tables_sse41
_sk_load_tables_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,8                            // mov           (%rax),%rcx
  .byte  76,139,64,8                         // mov           0x8(%rax),%r8
  .byte  243,68,15,111,4,185                 // movdqu        (%rcx,%rdi,4),%xmm8
  .byte  185,255,0,0,0                       // mov           $0xff,%ecx
  .byte  102,15,110,193                      // movd          %ecx,%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  102,65,15,111,200                   // movdqa        %xmm8,%xmm1
  .byte  102,15,114,209,8                    // psrld         $0x8,%xmm1
  .byte  102,15,219,200                      // pand          %xmm0,%xmm1
  .byte  102,65,15,111,208                   // movdqa        %xmm8,%xmm2
  .byte  102,15,114,210,16                   // psrld         $0x10,%xmm2
  .byte  102,15,219,208                      // pand          %xmm0,%xmm2
  .byte  102,65,15,219,192                   // pand          %xmm8,%xmm0
  .byte  102,72,15,58,22,193,1               // pextrq        $0x1,%xmm0,%rcx
  .byte  65,137,201                          // mov           %ecx,%r9d
  .byte  72,193,233,32                       // shr           $0x20,%rcx
  .byte  102,73,15,126,194                   // movq          %xmm0,%r10
  .byte  69,137,211                          // mov           %r10d,%r11d
  .byte  73,193,234,32                       // shr           $0x20,%r10
  .byte  243,67,15,16,4,152                  // movss         (%r8,%r11,4),%xmm0
  .byte  102,67,15,58,33,4,144,16            // insertps      $0x10,(%r8,%r10,4),%xmm0
  .byte  102,67,15,58,33,4,136,32            // insertps      $0x20,(%r8,%r9,4),%xmm0
  .byte  102,65,15,58,33,4,136,48            // insertps      $0x30,(%r8,%rcx,4),%xmm0
  .byte  76,139,64,16                        // mov           0x10(%rax),%r8
  .byte  102,73,15,58,22,202,1               // pextrq        $0x1,%xmm1,%r10
  .byte  77,137,209                          // mov           %r10,%r9
  .byte  73,193,233,32                       // shr           $0x20,%r9
  .byte  102,72,15,126,201                   // movq          %xmm1,%rcx
  .byte  65,137,203                          // mov           %ecx,%r11d
  .byte  65,129,227,255,255,255,0            // and           $0xffffff,%r11d
  .byte  72,193,233,30                       // shr           $0x1e,%rcx
  .byte  65,129,226,255,255,255,0            // and           $0xffffff,%r10d
  .byte  243,67,15,16,12,152                 // movss         (%r8,%r11,4),%xmm1
  .byte  102,65,15,58,33,12,8,16             // insertps      $0x10,(%r8,%rcx,1),%xmm1
  .byte  243,67,15,16,28,144                 // movss         (%r8,%r10,4),%xmm3
  .byte  102,15,58,33,203,32                 // insertps      $0x20,%xmm3,%xmm1
  .byte  243,67,15,16,28,136                 // movss         (%r8,%r9,4),%xmm3
  .byte  102,15,58,33,203,48                 // insertps      $0x30,%xmm3,%xmm1
  .byte  76,139,72,24                        // mov           0x18(%rax),%r9
  .byte  102,72,15,58,22,209,1               // pextrq        $0x1,%xmm2,%rcx
  .byte  68,15,183,193                       // movzwl        %cx,%r8d
  .byte  72,193,233,32                       // shr           $0x20,%rcx
  .byte  102,72,15,126,208                   // movq          %xmm2,%rax
  .byte  68,15,183,208                       // movzwl        %ax,%r10d
  .byte  72,193,232,30                       // shr           $0x1e,%rax
  .byte  243,67,15,16,20,145                 // movss         (%r9,%r10,4),%xmm2
  .byte  102,65,15,58,33,20,1,16             // insertps      $0x10,(%r9,%rax,1),%xmm2
  .byte  243,67,15,16,28,129                 // movss         (%r9,%r8,4),%xmm3
  .byte  102,15,58,33,211,32                 // insertps      $0x20,%xmm3,%xmm2
  .byte  243,65,15,16,28,137                 // movss         (%r9,%rcx,4),%xmm3
  .byte  102,15,58,33,211,48                 // insertps      $0x30,%xmm3,%xmm2
  .byte  102,65,15,114,208,24                // psrld         $0x18,%xmm8
  .byte  69,15,91,192                        // cvtdq2ps      %xmm8,%xmm8
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_load_a8_sse41
.globl _sk_load_a8_sse41
_sk_load_a8_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,15,56,49,4,56                   // pmovzxbd      (%rax,%rdi,1),%xmm0
  .byte  15,91,192                           // cvtdq2ps      %xmm0,%xmm0
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  15,89,216                           // mulps         %xmm0,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,87,201                           // xorps         %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_store_a8_sse41
.globl _sk_store_a8_sse41
_sk_store_a8_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  185,0,0,127,67                      // mov           $0x437f0000,%ecx
  .byte  102,68,15,110,193                   // movd          %ecx,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,89,195                        // mulps         %xmm3,%xmm8
  .byte  102,69,15,91,192                    // cvtps2dq      %xmm8,%xmm8
  .byte  102,69,15,56,43,192                 // packusdw      %xmm8,%xmm8
  .byte  102,69,15,103,192                   // packuswb      %xmm8,%xmm8
  .byte  102,68,15,126,4,56                  // movd          %xmm8,(%rax,%rdi,1)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_load_g8_sse41
.globl _sk_load_g8_sse41
_sk_load_g8_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,15,56,49,4,56                   // pmovzxbd      (%rax,%rdi,1),%xmm0
  .byte  15,91,200                           // cvtdq2ps      %xmm0,%xmm1
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  15,89,193                           // mulps         %xmm1,%xmm0
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,200                           // movaps        %xmm0,%xmm1
  .byte  15,40,208                           // movaps        %xmm0,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_load_565_sse41
.globl _sk_load_565_sse41
_sk_load_565_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,15,56,51,20,120                 // pmovzxwd      (%rax,%rdi,2),%xmm2
  .byte  184,0,248,0,0                       // mov           $0xf800,%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  102,15,219,194                      // pand          %xmm2,%xmm0
  .byte  15,91,200                           // cvtdq2ps      %xmm0,%xmm1
  .byte  184,8,33,132,55                     // mov           $0x37842108,%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  15,89,193                           // mulps         %xmm1,%xmm0
  .byte  184,224,7,0,0                       // mov           $0x7e0,%eax
  .byte  102,15,110,200                      // movd          %eax,%xmm1
  .byte  102,15,112,201,0                    // pshufd        $0x0,%xmm1,%xmm1
  .byte  102,15,219,202                      // pand          %xmm2,%xmm1
  .byte  15,91,217                           // cvtdq2ps      %xmm1,%xmm3
  .byte  184,33,8,2,58                       // mov           $0x3a020821,%eax
  .byte  102,15,110,200                      // movd          %eax,%xmm1
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  15,89,203                           // mulps         %xmm3,%xmm1
  .byte  184,31,0,0,0                        // mov           $0x1f,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  102,15,112,219,0                    // pshufd        $0x0,%xmm3,%xmm3
  .byte  102,15,219,218                      // pand          %xmm2,%xmm3
  .byte  15,91,219                           // cvtdq2ps      %xmm3,%xmm3
  .byte  184,8,33,4,61                       // mov           $0x3d042108,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  15,89,211                           // mulps         %xmm3,%xmm2
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_store_565_sse41
.globl _sk_store_565_sse41
_sk_store_565_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  185,0,0,248,65                      // mov           $0x41f80000,%ecx
  .byte  102,68,15,110,193                   // movd          %ecx,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,200                        // mulps         %xmm0,%xmm9
  .byte  102,69,15,91,201                    // cvtps2dq      %xmm9,%xmm9
  .byte  102,65,15,114,241,11                // pslld         $0xb,%xmm9
  .byte  185,0,0,124,66                      // mov           $0x427c0000,%ecx
  .byte  102,68,15,110,209                   // movd          %ecx,%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  68,15,89,209                        // mulps         %xmm1,%xmm10
  .byte  102,69,15,91,210                    // cvtps2dq      %xmm10,%xmm10
  .byte  102,65,15,114,242,5                 // pslld         $0x5,%xmm10
  .byte  102,69,15,235,209                   // por           %xmm9,%xmm10
  .byte  68,15,89,194                        // mulps         %xmm2,%xmm8
  .byte  102,69,15,91,192                    // cvtps2dq      %xmm8,%xmm8
  .byte  102,69,15,86,194                    // orpd          %xmm10,%xmm8
  .byte  102,69,15,56,43,192                 // packusdw      %xmm8,%xmm8
  .byte  102,68,15,214,4,120                 // movq          %xmm8,(%rax,%rdi,2)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_load_4444_sse41
.globl _sk_load_4444_sse41
_sk_load_4444_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,68,15,56,51,12,120              // pmovzxwd      (%rax,%rdi,2),%xmm9
  .byte  184,0,240,0,0                       // mov           $0xf000,%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  102,65,15,219,193                   // pand          %xmm9,%xmm0
  .byte  15,91,200                           // cvtdq2ps      %xmm0,%xmm1
  .byte  184,137,136,136,55                  // mov           $0x37888889,%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  15,89,193                           // mulps         %xmm1,%xmm0
  .byte  184,0,15,0,0                        // mov           $0xf00,%eax
  .byte  102,15,110,200                      // movd          %eax,%xmm1
  .byte  102,15,112,201,0                    // pshufd        $0x0,%xmm1,%xmm1
  .byte  102,65,15,219,201                   // pand          %xmm9,%xmm1
  .byte  15,91,209                           // cvtdq2ps      %xmm1,%xmm2
  .byte  184,137,136,136,57                  // mov           $0x39888889,%eax
  .byte  102,15,110,200                      // movd          %eax,%xmm1
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  15,89,202                           // mulps         %xmm2,%xmm1
  .byte  184,240,0,0,0                       // mov           $0xf0,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  102,15,112,210,0                    // pshufd        $0x0,%xmm2,%xmm2
  .byte  102,65,15,219,209                   // pand          %xmm9,%xmm2
  .byte  68,15,91,194                        // cvtdq2ps      %xmm2,%xmm8
  .byte  184,137,136,136,59                  // mov           $0x3b888889,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  184,15,0,0,0                        // mov           $0xf,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  102,15,112,219,0                    // pshufd        $0x0,%xmm3,%xmm3
  .byte  102,65,15,219,217                   // pand          %xmm9,%xmm3
  .byte  68,15,91,195                        // cvtdq2ps      %xmm3,%xmm8
  .byte  184,137,136,136,61                  // mov           $0x3d888889,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_store_4444_sse41
.globl _sk_store_4444_sse41
_sk_store_4444_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  185,0,0,112,65                      // mov           $0x41700000,%ecx
  .byte  102,68,15,110,193                   // movd          %ecx,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,200                        // mulps         %xmm0,%xmm9
  .byte  102,69,15,91,201                    // cvtps2dq      %xmm9,%xmm9
  .byte  102,65,15,114,241,12                // pslld         $0xc,%xmm9
  .byte  69,15,40,208                        // movaps        %xmm8,%xmm10
  .byte  68,15,89,209                        // mulps         %xmm1,%xmm10
  .byte  102,69,15,91,210                    // cvtps2dq      %xmm10,%xmm10
  .byte  102,65,15,114,242,8                 // pslld         $0x8,%xmm10
  .byte  102,69,15,235,209                   // por           %xmm9,%xmm10
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,202                        // mulps         %xmm2,%xmm9
  .byte  102,69,15,91,201                    // cvtps2dq      %xmm9,%xmm9
  .byte  102,65,15,114,241,4                 // pslld         $0x4,%xmm9
  .byte  68,15,89,195                        // mulps         %xmm3,%xmm8
  .byte  102,69,15,91,192                    // cvtps2dq      %xmm8,%xmm8
  .byte  102,69,15,86,193                    // orpd          %xmm9,%xmm8
  .byte  102,69,15,86,194                    // orpd          %xmm10,%xmm8
  .byte  102,69,15,56,43,192                 // packusdw      %xmm8,%xmm8
  .byte  102,68,15,214,4,120                 // movq          %xmm8,(%rax,%rdi,2)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_load_8888_sse41
.globl _sk_load_8888_sse41
_sk_load_8888_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  243,15,111,28,184                   // movdqu        (%rax,%rdi,4),%xmm3
  .byte  184,255,0,0,0                       // mov           $0xff,%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  102,15,111,203                      // movdqa        %xmm3,%xmm1
  .byte  102,15,114,209,8                    // psrld         $0x8,%xmm1
  .byte  102,15,219,200                      // pand          %xmm0,%xmm1
  .byte  102,15,111,211                      // movdqa        %xmm3,%xmm2
  .byte  102,15,114,210,16                   // psrld         $0x10,%xmm2
  .byte  102,15,219,208                      // pand          %xmm0,%xmm2
  .byte  102,15,219,195                      // pand          %xmm3,%xmm0
  .byte  15,91,192                           // cvtdq2ps      %xmm0,%xmm0
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  15,91,201                           // cvtdq2ps      %xmm1,%xmm1
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  15,91,210                           // cvtdq2ps      %xmm2,%xmm2
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  102,15,114,211,24                   // psrld         $0x18,%xmm3
  .byte  15,91,219                           // cvtdq2ps      %xmm3,%xmm3
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_store_8888_sse41
.globl _sk_store_8888_sse41
_sk_store_8888_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  185,0,0,127,67                      // mov           $0x437f0000,%ecx
  .byte  102,68,15,110,193                   // movd          %ecx,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,200                        // mulps         %xmm0,%xmm9
  .byte  102,69,15,91,201                    // cvtps2dq      %xmm9,%xmm9
  .byte  69,15,40,208                        // movaps        %xmm8,%xmm10
  .byte  68,15,89,209                        // mulps         %xmm1,%xmm10
  .byte  102,69,15,91,210                    // cvtps2dq      %xmm10,%xmm10
  .byte  102,65,15,114,242,8                 // pslld         $0x8,%xmm10
  .byte  102,69,15,235,209                   // por           %xmm9,%xmm10
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,202                        // mulps         %xmm2,%xmm9
  .byte  102,69,15,91,201                    // cvtps2dq      %xmm9,%xmm9
  .byte  102,65,15,114,241,16                // pslld         $0x10,%xmm9
  .byte  68,15,89,195                        // mulps         %xmm3,%xmm8
  .byte  102,69,15,91,192                    // cvtps2dq      %xmm8,%xmm8
  .byte  102,65,15,114,240,24                // pslld         $0x18,%xmm8
  .byte  102,69,15,235,193                   // por           %xmm9,%xmm8
  .byte  102,69,15,235,194                   // por           %xmm10,%xmm8
  .byte  243,68,15,127,4,184                 // movdqu        %xmm8,(%rax,%rdi,4)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_load_f16_sse41
.globl _sk_load_f16_sse41
_sk_load_f16_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  243,15,111,4,248                    // movdqu        (%rax,%rdi,8),%xmm0
  .byte  243,15,111,76,248,16                // movdqu        0x10(%rax,%rdi,8),%xmm1
  .byte  102,68,15,111,192                   // movdqa        %xmm0,%xmm8
  .byte  102,68,15,97,193                    // punpcklwd     %xmm1,%xmm8
  .byte  102,15,105,193                      // punpckhwd     %xmm1,%xmm0
  .byte  102,65,15,111,200                   // movdqa        %xmm8,%xmm1
  .byte  102,15,97,200                       // punpcklwd     %xmm0,%xmm1
  .byte  102,68,15,105,192                   // punpckhwd     %xmm0,%xmm8
  .byte  184,0,4,0,4                         // mov           $0x4000400,%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  102,15,112,216,0                    // pshufd        $0x0,%xmm0,%xmm3
  .byte  102,15,111,195                      // movdqa        %xmm3,%xmm0
  .byte  102,15,101,193                      // pcmpgtw       %xmm1,%xmm0
  .byte  102,15,223,193                      // pandn         %xmm1,%xmm0
  .byte  102,15,56,51,192                    // pmovzxwd      %xmm0,%xmm0
  .byte  102,15,114,240,13                   // pslld         $0xd,%xmm0
  .byte  184,0,0,128,119                     // mov           $0x77800000,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  102,68,15,112,202,0                 // pshufd        $0x0,%xmm2,%xmm9
  .byte  65,15,89,193                        // mulps         %xmm9,%xmm0
  .byte  102,15,112,201,78                   // pshufd        $0x4e,%xmm1,%xmm1
  .byte  102,15,111,211                      // movdqa        %xmm3,%xmm2
  .byte  102,15,101,209                      // pcmpgtw       %xmm1,%xmm2
  .byte  102,15,223,209                      // pandn         %xmm1,%xmm2
  .byte  102,15,56,51,202                    // pmovzxwd      %xmm2,%xmm1
  .byte  102,15,114,241,13                   // pslld         $0xd,%xmm1
  .byte  65,15,89,201                        // mulps         %xmm9,%xmm1
  .byte  102,15,111,211                      // movdqa        %xmm3,%xmm2
  .byte  102,65,15,101,208                   // pcmpgtw       %xmm8,%xmm2
  .byte  102,65,15,223,208                   // pandn         %xmm8,%xmm2
  .byte  102,15,56,51,210                    // pmovzxwd      %xmm2,%xmm2
  .byte  102,15,114,242,13                   // pslld         $0xd,%xmm2
  .byte  65,15,89,209                        // mulps         %xmm9,%xmm2
  .byte  102,69,15,112,192,78                // pshufd        $0x4e,%xmm8,%xmm8
  .byte  102,65,15,101,216                   // pcmpgtw       %xmm8,%xmm3
  .byte  102,65,15,223,216                   // pandn         %xmm8,%xmm3
  .byte  102,15,56,51,219                    // pmovzxwd      %xmm3,%xmm3
  .byte  102,15,114,243,13                   // pslld         $0xd,%xmm3
  .byte  65,15,89,217                        // mulps         %xmm9,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_store_f16_sse41
.globl _sk_store_f16_sse41
_sk_store_f16_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  185,0,0,128,7                       // mov           $0x7800000,%ecx
  .byte  102,68,15,110,193                   // movd          %ecx,%xmm8
  .byte  102,69,15,112,192,0                 // pshufd        $0x0,%xmm8,%xmm8
  .byte  102,69,15,111,200                   // movdqa        %xmm8,%xmm9
  .byte  68,15,89,200                        // mulps         %xmm0,%xmm9
  .byte  102,65,15,114,209,13                // psrld         $0xd,%xmm9
  .byte  102,69,15,111,208                   // movdqa        %xmm8,%xmm10
  .byte  68,15,89,209                        // mulps         %xmm1,%xmm10
  .byte  102,65,15,114,210,13                // psrld         $0xd,%xmm10
  .byte  102,69,15,111,216                   // movdqa        %xmm8,%xmm11
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  102,65,15,114,211,13                // psrld         $0xd,%xmm11
  .byte  68,15,89,195                        // mulps         %xmm3,%xmm8
  .byte  102,65,15,114,208,13                // psrld         $0xd,%xmm8
  .byte  102,65,15,115,250,2                 // pslldq        $0x2,%xmm10
  .byte  102,69,15,235,209                   // por           %xmm9,%xmm10
  .byte  102,65,15,115,248,2                 // pslldq        $0x2,%xmm8
  .byte  102,69,15,235,195                   // por           %xmm11,%xmm8
  .byte  102,69,15,111,202                   // movdqa        %xmm10,%xmm9
  .byte  102,69,15,98,200                    // punpckldq     %xmm8,%xmm9
  .byte  243,68,15,127,12,248                // movdqu        %xmm9,(%rax,%rdi,8)
  .byte  102,69,15,106,208                   // punpckhdq     %xmm8,%xmm10
  .byte  243,68,15,127,84,248,16             // movdqu        %xmm10,0x10(%rax,%rdi,8)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_store_f32_sse41
.globl _sk_store_f32_sse41
_sk_store_f32_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,137,249                          // mov           %rdi,%rcx
  .byte  72,193,225,4                        // shl           $0x4,%rcx
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  68,15,20,201                        // unpcklps      %xmm1,%xmm9
  .byte  68,15,40,210                        // movaps        %xmm2,%xmm10
  .byte  68,15,40,218                        // movaps        %xmm2,%xmm11
  .byte  68,15,20,219                        // unpcklps      %xmm3,%xmm11
  .byte  68,15,21,193                        // unpckhps      %xmm1,%xmm8
  .byte  68,15,21,211                        // unpckhps      %xmm3,%xmm10
  .byte  69,15,40,225                        // movaps        %xmm9,%xmm12
  .byte  102,69,15,20,227                    // unpcklpd      %xmm11,%xmm12
  .byte  69,15,18,217                        // movhlps       %xmm9,%xmm11
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  102,69,15,20,202                    // unpcklpd      %xmm10,%xmm9
  .byte  69,15,18,208                        // movhlps       %xmm8,%xmm10
  .byte  102,68,15,17,36,8                   // movupd        %xmm12,(%rax,%rcx,1)
  .byte  68,15,17,92,8,16                    // movups        %xmm11,0x10(%rax,%rcx,1)
  .byte  102,68,15,17,76,8,32                // movupd        %xmm9,0x20(%rax,%rcx,1)
  .byte  68,15,17,84,8,48                    // movups        %xmm10,0x30(%rax,%rcx,1)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_x_sse41
.globl _sk_clamp_x_sse41
_sk_clamp_x_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  69,15,87,192                        // xorps         %xmm8,%xmm8
  .byte  68,15,95,192                        // maxps         %xmm0,%xmm8
  .byte  243,68,15,16,8                      // movss         (%rax),%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  102,15,118,192                      // pcmpeqd       %xmm0,%xmm0
  .byte  102,65,15,254,193                   // paddd         %xmm9,%xmm0
  .byte  68,15,93,192                        // minps         %xmm0,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_y_sse41
.globl _sk_clamp_y_sse41
_sk_clamp_y_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  69,15,87,192                        // xorps         %xmm8,%xmm8
  .byte  68,15,95,193                        // maxps         %xmm1,%xmm8
  .byte  243,68,15,16,8                      // movss         (%rax),%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  102,15,118,201                      // pcmpeqd       %xmm1,%xmm1
  .byte  102,65,15,254,201                   // paddd         %xmm9,%xmm1
  .byte  68,15,93,193                        // minps         %xmm1,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,200                        // movaps        %xmm8,%xmm1
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_repeat_x_sse41
.globl _sk_repeat_x_sse41
_sk_repeat_x_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,0                      // movss         (%rax),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  69,15,94,200                        // divps         %xmm8,%xmm9
  .byte  102,69,15,58,8,201,1                // roundps       $0x1,%xmm9,%xmm9
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  65,15,92,193                        // subps         %xmm9,%xmm0
  .byte  102,69,15,118,201                   // pcmpeqd       %xmm9,%xmm9
  .byte  102,69,15,254,200                   // paddd         %xmm8,%xmm9
  .byte  65,15,93,193                        // minps         %xmm9,%xmm0
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_repeat_y_sse41
.globl _sk_repeat_y_sse41
_sk_repeat_y_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,0                      // movss         (%rax),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,40,201                        // movaps        %xmm1,%xmm9
  .byte  69,15,94,200                        // divps         %xmm8,%xmm9
  .byte  102,69,15,58,8,201,1                // roundps       $0x1,%xmm9,%xmm9
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  65,15,92,201                        // subps         %xmm9,%xmm1
  .byte  102,69,15,118,201                   // pcmpeqd       %xmm9,%xmm9
  .byte  102,69,15,254,200                   // paddd         %xmm8,%xmm9
  .byte  65,15,93,201                        // minps         %xmm9,%xmm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_mirror_x_sse41
.globl _sk_mirror_x_sse41
_sk_mirror_x_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,0                      // movss         (%rax),%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  65,15,92,193                        // subps         %xmm9,%xmm0
  .byte  243,69,15,88,192                    // addss         %xmm8,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,40,208                        // movaps        %xmm0,%xmm10
  .byte  69,15,94,208                        // divps         %xmm8,%xmm10
  .byte  102,69,15,58,8,210,1                // roundps       $0x1,%xmm10,%xmm10
  .byte  69,15,89,208                        // mulps         %xmm8,%xmm10
  .byte  65,15,92,194                        // subps         %xmm10,%xmm0
  .byte  65,15,92,193                        // subps         %xmm9,%xmm0
  .byte  69,15,87,192                        // xorps         %xmm8,%xmm8
  .byte  68,15,92,192                        // subps         %xmm0,%xmm8
  .byte  65,15,84,192                        // andps         %xmm8,%xmm0
  .byte  102,69,15,118,192                   // pcmpeqd       %xmm8,%xmm8
  .byte  102,69,15,254,193                   // paddd         %xmm9,%xmm8
  .byte  65,15,93,192                        // minps         %xmm8,%xmm0
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_mirror_y_sse41
.globl _sk_mirror_y_sse41
_sk_mirror_y_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,0                      // movss         (%rax),%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  65,15,92,201                        // subps         %xmm9,%xmm1
  .byte  243,69,15,88,192                    // addss         %xmm8,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,40,209                        // movaps        %xmm1,%xmm10
  .byte  69,15,94,208                        // divps         %xmm8,%xmm10
  .byte  102,69,15,58,8,210,1                // roundps       $0x1,%xmm10,%xmm10
  .byte  69,15,89,208                        // mulps         %xmm8,%xmm10
  .byte  65,15,92,202                        // subps         %xmm10,%xmm1
  .byte  65,15,92,201                        // subps         %xmm9,%xmm1
  .byte  69,15,87,192                        // xorps         %xmm8,%xmm8
  .byte  68,15,92,193                        // subps         %xmm1,%xmm8
  .byte  65,15,84,200                        // andps         %xmm8,%xmm1
  .byte  102,69,15,118,192                   // pcmpeqd       %xmm8,%xmm8
  .byte  102,69,15,254,193                   // paddd         %xmm9,%xmm8
  .byte  65,15,93,200                        // minps         %xmm8,%xmm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_luminance_to_alpha_sse41
.globl _sk_luminance_to_alpha_sse41
_sk_luminance_to_alpha_sse41:
  .byte  184,208,179,89,62                   // mov           $0x3e59b3d0,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  15,89,216                           // mulps         %xmm0,%xmm3
  .byte  184,89,23,55,63                     // mov           $0x3f371759,%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  15,89,193                           // mulps         %xmm1,%xmm0
  .byte  15,88,195                           // addps         %xmm3,%xmm0
  .byte  184,152,221,147,61                  // mov           $0x3d93dd98,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  15,89,218                           // mulps         %xmm2,%xmm3
  .byte  15,88,216                           // addps         %xmm0,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,87,201                           // xorps         %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_matrix_2x3_sse41
.globl _sk_matrix_2x3_sse41
_sk_matrix_2x3_sse41:
  .byte  68,15,40,201                        // movaps        %xmm1,%xmm9
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,15,16,0                         // movss         (%rax),%xmm0
  .byte  243,15,16,72,4                      // movss         0x4(%rax),%xmm1
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  243,68,15,16,80,8                   // movss         0x8(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,16                  // movss         0x10(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  69,15,89,209                        // mulps         %xmm9,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,88,194                        // addps         %xmm10,%xmm0
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  243,68,15,16,80,12                  // movss         0xc(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,20                  // movss         0x14(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  69,15,89,209                        // mulps         %xmm9,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,88,202                        // addps         %xmm10,%xmm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_matrix_3x4_sse41
.globl _sk_matrix_3x4_sse41
_sk_matrix_3x4_sse41:
  .byte  68,15,40,201                        // movaps        %xmm1,%xmm9
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,15,16,0                         // movss         (%rax),%xmm0
  .byte  243,15,16,72,4                      // movss         0x4(%rax),%xmm1
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  243,68,15,16,80,12                  // movss         0xc(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,24                  // movss         0x18(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,96,36                  // movss         0x24(%rax),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  69,15,88,220                        // addps         %xmm12,%xmm11
  .byte  69,15,89,209                        // mulps         %xmm9,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,88,194                        // addps         %xmm10,%xmm0
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  243,68,15,16,80,16                  // movss         0x10(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,28                  // movss         0x1c(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,96,40                  // movss         0x28(%rax),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  69,15,88,220                        // addps         %xmm12,%xmm11
  .byte  69,15,89,209                        // mulps         %xmm9,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,88,202                        // addps         %xmm10,%xmm1
  .byte  243,68,15,16,80,8                   // movss         0x8(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,20                  // movss         0x14(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,96,32                  // movss         0x20(%rax),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  243,68,15,16,104,44                 // movss         0x2c(%rax),%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  68,15,89,226                        // mulps         %xmm2,%xmm12
  .byte  69,15,88,229                        // addps         %xmm13,%xmm12
  .byte  69,15,89,217                        // mulps         %xmm9,%xmm11
  .byte  69,15,88,220                        // addps         %xmm12,%xmm11
  .byte  69,15,89,208                        // mulps         %xmm8,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,210                        // movaps        %xmm10,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_matrix_4x5_sse41
.globl _sk_matrix_4x5_sse41
_sk_matrix_4x5_sse41:
  .byte  68,15,40,201                        // movaps        %xmm1,%xmm9
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,15,16,0                         // movss         (%rax),%xmm0
  .byte  243,15,16,72,4                      // movss         0x4(%rax),%xmm1
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  243,68,15,16,80,16                  // movss         0x10(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,32                  // movss         0x20(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,96,48                  // movss         0x30(%rax),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  243,68,15,16,104,64                 // movss         0x40(%rax),%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  68,15,89,227                        // mulps         %xmm3,%xmm12
  .byte  69,15,88,229                        // addps         %xmm13,%xmm12
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  69,15,88,220                        // addps         %xmm12,%xmm11
  .byte  69,15,89,209                        // mulps         %xmm9,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,88,194                        // addps         %xmm10,%xmm0
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  243,68,15,16,80,20                  // movss         0x14(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,36                  // movss         0x24(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,96,52                  // movss         0x34(%rax),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  243,68,15,16,104,68                 // movss         0x44(%rax),%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  68,15,89,227                        // mulps         %xmm3,%xmm12
  .byte  69,15,88,229                        // addps         %xmm13,%xmm12
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  69,15,88,220                        // addps         %xmm12,%xmm11
  .byte  69,15,89,209                        // mulps         %xmm9,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,88,202                        // addps         %xmm10,%xmm1
  .byte  243,68,15,16,80,8                   // movss         0x8(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,24                  // movss         0x18(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,96,40                  // movss         0x28(%rax),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  243,68,15,16,104,56                 // movss         0x38(%rax),%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  243,68,15,16,112,72                 // movss         0x48(%rax),%xmm14
  .byte  69,15,198,246,0                     // shufps        $0x0,%xmm14,%xmm14
  .byte  68,15,89,235                        // mulps         %xmm3,%xmm13
  .byte  69,15,88,238                        // addps         %xmm14,%xmm13
  .byte  68,15,89,226                        // mulps         %xmm2,%xmm12
  .byte  69,15,88,229                        // addps         %xmm13,%xmm12
  .byte  69,15,89,217                        // mulps         %xmm9,%xmm11
  .byte  69,15,88,220                        // addps         %xmm12,%xmm11
  .byte  69,15,89,208                        // mulps         %xmm8,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  243,68,15,16,88,12                  // movss         0xc(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,96,28                  // movss         0x1c(%rax),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  243,68,15,16,104,44                 // movss         0x2c(%rax),%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  243,68,15,16,112,60                 // movss         0x3c(%rax),%xmm14
  .byte  69,15,198,246,0                     // shufps        $0x0,%xmm14,%xmm14
  .byte  243,68,15,16,120,76                 // movss         0x4c(%rax),%xmm15
  .byte  69,15,198,255,0                     // shufps        $0x0,%xmm15,%xmm15
  .byte  68,15,89,243                        // mulps         %xmm3,%xmm14
  .byte  69,15,88,247                        // addps         %xmm15,%xmm14
  .byte  68,15,89,234                        // mulps         %xmm2,%xmm13
  .byte  69,15,88,238                        // addps         %xmm14,%xmm13
  .byte  69,15,89,225                        // mulps         %xmm9,%xmm12
  .byte  69,15,88,229                        // addps         %xmm13,%xmm12
  .byte  69,15,89,216                        // mulps         %xmm8,%xmm11
  .byte  69,15,88,220                        // addps         %xmm12,%xmm11
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,210                        // movaps        %xmm10,%xmm2
  .byte  65,15,40,219                        // movaps        %xmm11,%xmm3
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_matrix_perspective_sse41
.globl _sk_matrix_perspective_sse41
_sk_matrix_perspective_sse41:
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,15,16,0                         // movss         (%rax),%xmm0
  .byte  243,68,15,16,72,4                   // movss         0x4(%rax),%xmm9
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  243,68,15,16,80,8                   // movss         0x8(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  68,15,89,201                        // mulps         %xmm1,%xmm9
  .byte  69,15,88,202                        // addps         %xmm10,%xmm9
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,88,193                        // addps         %xmm9,%xmm0
  .byte  243,68,15,16,72,12                  // movss         0xc(%rax),%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  243,68,15,16,80,16                  // movss         0x10(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,20                  // movss         0x14(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  68,15,89,209                        // mulps         %xmm1,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  69,15,88,202                        // addps         %xmm10,%xmm9
  .byte  243,68,15,16,80,24                  // movss         0x18(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,28                  // movss         0x1c(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,96,32                  // movss         0x20(%rax),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  68,15,89,217                        // mulps         %xmm1,%xmm11
  .byte  69,15,88,220                        // addps         %xmm12,%xmm11
  .byte  69,15,89,208                        // mulps         %xmm8,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  65,15,83,202                        // rcpps         %xmm10,%xmm1
  .byte  15,89,193                           // mulps         %xmm1,%xmm0
  .byte  68,15,89,201                        // mulps         %xmm1,%xmm9
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,201                        // movaps        %xmm9,%xmm1
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_linear_gradient_2stops_sse41
.globl _sk_linear_gradient_2stops_sse41
_sk_linear_gradient_2stops_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  68,15,16,8                          // movups        (%rax),%xmm9
  .byte  15,16,88,16                         // movups        0x10(%rax),%xmm3
  .byte  68,15,40,195                        // movaps        %xmm3,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  65,15,40,201                        // movaps        %xmm9,%xmm1
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  68,15,89,192                        // mulps         %xmm0,%xmm8
  .byte  68,15,88,193                        // addps         %xmm1,%xmm8
  .byte  15,40,203                           // movaps        %xmm3,%xmm1
  .byte  15,198,201,85                       // shufps        $0x55,%xmm1,%xmm1
  .byte  65,15,40,209                        // movaps        %xmm9,%xmm2
  .byte  15,198,210,85                       // shufps        $0x55,%xmm2,%xmm2
  .byte  15,89,200                           // mulps         %xmm0,%xmm1
  .byte  15,88,202                           // addps         %xmm2,%xmm1
  .byte  15,40,211                           // movaps        %xmm3,%xmm2
  .byte  15,198,210,170                      // shufps        $0xaa,%xmm2,%xmm2
  .byte  69,15,40,209                        // movaps        %xmm9,%xmm10
  .byte  69,15,198,210,170                   // shufps        $0xaa,%xmm10,%xmm10
  .byte  15,89,208                           // mulps         %xmm0,%xmm2
  .byte  65,15,88,210                        // addps         %xmm10,%xmm2
  .byte  15,198,219,255                      // shufps        $0xff,%xmm3,%xmm3
  .byte  69,15,198,201,255                   // shufps        $0xff,%xmm9,%xmm9
  .byte  15,89,216                           // mulps         %xmm0,%xmm3
  .byte  65,15,88,217                        // addps         %xmm9,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_start_pipeline_sse2
.globl _sk_start_pipeline_sse2
_sk_start_pipeline_sse2:
  .byte  65,87                               // push          %r15
  .byte  65,86                               // push          %r14
  .byte  65,85                               // push          %r13
  .byte  65,84                               // push          %r12
  .byte  83                                  // push          %rbx
  .byte  73,137,207                          // mov           %rcx,%r15
  .byte  73,137,214                          // mov           %rdx,%r14
  .byte  72,137,251                          // mov           %rdi,%rbx
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  73,137,196                          // mov           %rax,%r12
  .byte  73,137,245                          // mov           %rsi,%r13
  .byte  72,141,67,4                         // lea           0x4(%rbx),%rax
  .byte  76,57,248                           // cmp           %r15,%rax
  .byte  118,5                               // jbe           28 <_sk_start_pipeline_sse2+0x28>
  .byte  72,137,216                          // mov           %rbx,%rax
  .byte  235,52                              // jmp           5c <_sk_start_pipeline_sse2+0x5c>
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,87,201                           // xorps         %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  15,87,219                           // xorps         %xmm3,%xmm3
  .byte  15,87,228                           // xorps         %xmm4,%xmm4
  .byte  15,87,237                           // xorps         %xmm5,%xmm5
  .byte  15,87,246                           // xorps         %xmm6,%xmm6
  .byte  15,87,255                           // xorps         %xmm7,%xmm7
  .byte  72,137,223                          // mov           %rbx,%rdi
  .byte  76,137,238                          // mov           %r13,%rsi
  .byte  76,137,242                          // mov           %r14,%rdx
  .byte  65,255,212                          // callq         *%r12
  .byte  72,141,67,4                         // lea           0x4(%rbx),%rax
  .byte  72,131,195,8                        // add           $0x8,%rbx
  .byte  76,57,251                           // cmp           %r15,%rbx
  .byte  72,137,195                          // mov           %rax,%rbx
  .byte  118,204                             // jbe           28 <_sk_start_pipeline_sse2+0x28>
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,93                               // pop           %r13
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  195                                 // retq

HIDDEN _sk_just_return_sse2
.globl _sk_just_return_sse2
_sk_just_return_sse2:
  .byte  195                                 // retq

HIDDEN _sk_seed_shader_sse2
.globl _sk_seed_shader_sse2
_sk_seed_shader_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  102,15,110,199                      // movd          %edi,%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  15,91,200                           // cvtdq2ps      %xmm0,%xmm1
  .byte  185,0,0,0,63                        // mov           $0x3f000000,%ecx
  .byte  102,15,110,209                      // movd          %ecx,%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  15,88,202                           // addps         %xmm2,%xmm1
  .byte  15,16,2                             // movups        (%rdx),%xmm0
  .byte  15,88,193                           // addps         %xmm1,%xmm0
  .byte  102,15,110,8                        // movd          (%rax),%xmm1
  .byte  102,15,112,201,0                    // pshufd        $0x0,%xmm1,%xmm1
  .byte  15,91,201                           // cvtdq2ps      %xmm1,%xmm1
  .byte  15,88,202                           // addps         %xmm2,%xmm1
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,87,219                           // xorps         %xmm3,%xmm3
  .byte  15,87,228                           // xorps         %xmm4,%xmm4
  .byte  15,87,237                           // xorps         %xmm5,%xmm5
  .byte  15,87,246                           // xorps         %xmm6,%xmm6
  .byte  15,87,255                           // xorps         %xmm7,%xmm7
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_constant_color_sse2
.globl _sk_constant_color_sse2
_sk_constant_color_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,16,24                            // movups        (%rax),%xmm3
  .byte  15,40,195                           // movaps        %xmm3,%xmm0
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  15,40,203                           // movaps        %xmm3,%xmm1
  .byte  15,198,201,85                       // shufps        $0x55,%xmm1,%xmm1
  .byte  15,40,211                           // movaps        %xmm3,%xmm2
  .byte  15,198,210,170                      // shufps        $0xaa,%xmm2,%xmm2
  .byte  15,198,219,255                      // shufps        $0xff,%xmm3,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clear_sse2
.globl _sk_clear_sse2
_sk_clear_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,87,201                           // xorps         %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  15,87,219                           // xorps         %xmm3,%xmm3
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_srcatop_sse2
.globl _sk_srcatop_sse2
_sk_srcatop_sse2:
  .byte  15,89,199                           // mulps         %xmm7,%xmm0
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,92,195                        // subps         %xmm3,%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,204                        // mulps         %xmm4,%xmm9
  .byte  65,15,88,193                        // addps         %xmm9,%xmm0
  .byte  15,89,207                           // mulps         %xmm7,%xmm1
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,205                        // mulps         %xmm5,%xmm9
  .byte  65,15,88,201                        // addps         %xmm9,%xmm1
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,206                        // mulps         %xmm6,%xmm9
  .byte  65,15,88,209                        // addps         %xmm9,%xmm2
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  68,15,89,199                        // mulps         %xmm7,%xmm8
  .byte  65,15,88,216                        // addps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_dstatop_sse2
.globl _sk_dstatop_sse2
_sk_dstatop_sse2:
  .byte  68,15,40,195                        // movaps        %xmm3,%xmm8
  .byte  68,15,89,196                        // mulps         %xmm4,%xmm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,200                   // movd          %eax,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  68,15,92,207                        // subps         %xmm7,%xmm9
  .byte  65,15,89,193                        // mulps         %xmm9,%xmm0
  .byte  65,15,88,192                        // addps         %xmm8,%xmm0
  .byte  68,15,40,195                        // movaps        %xmm3,%xmm8
  .byte  68,15,89,197                        // mulps         %xmm5,%xmm8
  .byte  65,15,89,201                        // mulps         %xmm9,%xmm1
  .byte  65,15,88,200                        // addps         %xmm8,%xmm1
  .byte  68,15,40,195                        // movaps        %xmm3,%xmm8
  .byte  68,15,89,198                        // mulps         %xmm6,%xmm8
  .byte  65,15,89,209                        // mulps         %xmm9,%xmm2
  .byte  65,15,88,208                        // addps         %xmm8,%xmm2
  .byte  68,15,89,203                        // mulps         %xmm3,%xmm9
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  65,15,88,217                        // addps         %xmm9,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_srcin_sse2
.globl _sk_srcin_sse2
_sk_srcin_sse2:
  .byte  15,89,199                           // mulps         %xmm7,%xmm0
  .byte  15,89,207                           // mulps         %xmm7,%xmm1
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_dstin_sse2
.globl _sk_dstin_sse2
_sk_dstin_sse2:
  .byte  15,40,195                           // movaps        %xmm3,%xmm0
  .byte  15,89,196                           // mulps         %xmm4,%xmm0
  .byte  15,40,203                           // movaps        %xmm3,%xmm1
  .byte  15,89,205                           // mulps         %xmm5,%xmm1
  .byte  15,40,211                           // movaps        %xmm3,%xmm2
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_srcout_sse2
.globl _sk_srcout_sse2
_sk_srcout_sse2:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,92,199                        // subps         %xmm7,%xmm8
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_dstout_sse2
.globl _sk_dstout_sse2
_sk_dstout_sse2:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,92,195                        // subps         %xmm3,%xmm8
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  15,89,196                           // mulps         %xmm4,%xmm0
  .byte  65,15,40,200                        // movaps        %xmm8,%xmm1
  .byte  15,89,205                           // mulps         %xmm5,%xmm1
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  68,15,89,199                        // mulps         %xmm7,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,216                        // movaps        %xmm8,%xmm3
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_srcover_sse2
.globl _sk_srcover_sse2
_sk_srcover_sse2:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,92,195                        // subps         %xmm3,%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,204                        // mulps         %xmm4,%xmm9
  .byte  65,15,88,193                        // addps         %xmm9,%xmm0
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,205                        // mulps         %xmm5,%xmm9
  .byte  65,15,88,201                        // addps         %xmm9,%xmm1
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,206                        // mulps         %xmm6,%xmm9
  .byte  65,15,88,209                        // addps         %xmm9,%xmm2
  .byte  68,15,89,199                        // mulps         %xmm7,%xmm8
  .byte  65,15,88,216                        // addps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_dstover_sse2
.globl _sk_dstover_sse2
_sk_dstover_sse2:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,92,199                        // subps         %xmm7,%xmm8
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  15,88,214                           // addps         %xmm6,%xmm2
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  15,88,223                           // addps         %xmm7,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_modulate_sse2
.globl _sk_modulate_sse2
_sk_modulate_sse2:
  .byte  15,89,196                           // mulps         %xmm4,%xmm0
  .byte  15,89,205                           // mulps         %xmm5,%xmm1
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_multiply_sse2
.globl _sk_multiply_sse2
_sk_multiply_sse2:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,92,207                        // subps         %xmm7,%xmm9
  .byte  69,15,40,209                        // movaps        %xmm9,%xmm10
  .byte  68,15,89,208                        // mulps         %xmm0,%xmm10
  .byte  68,15,92,195                        // subps         %xmm3,%xmm8
  .byte  69,15,40,216                        // movaps        %xmm8,%xmm11
  .byte  68,15,89,220                        // mulps         %xmm4,%xmm11
  .byte  69,15,88,218                        // addps         %xmm10,%xmm11
  .byte  15,89,196                           // mulps         %xmm4,%xmm0
  .byte  65,15,88,195                        // addps         %xmm11,%xmm0
  .byte  69,15,40,209                        // movaps        %xmm9,%xmm10
  .byte  68,15,89,209                        // mulps         %xmm1,%xmm10
  .byte  69,15,40,216                        // movaps        %xmm8,%xmm11
  .byte  68,15,89,221                        // mulps         %xmm5,%xmm11
  .byte  69,15,88,218                        // addps         %xmm10,%xmm11
  .byte  15,89,205                           // mulps         %xmm5,%xmm1
  .byte  65,15,88,203                        // addps         %xmm11,%xmm1
  .byte  69,15,40,209                        // movaps        %xmm9,%xmm10
  .byte  68,15,89,210                        // mulps         %xmm2,%xmm10
  .byte  69,15,40,216                        // movaps        %xmm8,%xmm11
  .byte  68,15,89,222                        // mulps         %xmm6,%xmm11
  .byte  69,15,88,218                        // addps         %xmm10,%xmm11
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  65,15,88,211                        // addps         %xmm11,%xmm2
  .byte  68,15,89,203                        // mulps         %xmm3,%xmm9
  .byte  68,15,89,199                        // mulps         %xmm7,%xmm8
  .byte  69,15,88,193                        // addps         %xmm9,%xmm8
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  65,15,88,216                        // addps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_plus__sse2
.globl _sk_plus__sse2
_sk_plus__sse2:
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  15,88,214                           // addps         %xmm6,%xmm2
  .byte  15,88,223                           // addps         %xmm7,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_screen_sse2
.globl _sk_screen_sse2
_sk_screen_sse2:
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  68,15,88,196                        // addps         %xmm4,%xmm8
  .byte  15,89,196                           // mulps         %xmm4,%xmm0
  .byte  68,15,92,192                        // subps         %xmm0,%xmm8
  .byte  68,15,40,201                        // movaps        %xmm1,%xmm9
  .byte  68,15,88,205                        // addps         %xmm5,%xmm9
  .byte  15,89,205                           // mulps         %xmm5,%xmm1
  .byte  68,15,92,201                        // subps         %xmm1,%xmm9
  .byte  68,15,40,210                        // movaps        %xmm2,%xmm10
  .byte  68,15,88,214                        // addps         %xmm6,%xmm10
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  68,15,92,210                        // subps         %xmm2,%xmm10
  .byte  68,15,40,219                        // movaps        %xmm3,%xmm11
  .byte  68,15,88,223                        // addps         %xmm7,%xmm11
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  68,15,92,219                        // subps         %xmm3,%xmm11
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  65,15,40,201                        // movaps        %xmm9,%xmm1
  .byte  65,15,40,210                        // movaps        %xmm10,%xmm2
  .byte  65,15,40,219                        // movaps        %xmm11,%xmm3
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_xor__sse2
.globl _sk_xor__sse2
_sk_xor__sse2:
  .byte  68,15,40,195                        // movaps        %xmm3,%xmm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,92,207                        // subps         %xmm7,%xmm9
  .byte  65,15,89,193                        // mulps         %xmm9,%xmm0
  .byte  65,15,92,216                        // subps         %xmm8,%xmm3
  .byte  68,15,40,211                        // movaps        %xmm3,%xmm10
  .byte  68,15,89,212                        // mulps         %xmm4,%xmm10
  .byte  65,15,88,194                        // addps         %xmm10,%xmm0
  .byte  65,15,89,201                        // mulps         %xmm9,%xmm1
  .byte  68,15,40,211                        // movaps        %xmm3,%xmm10
  .byte  68,15,89,213                        // mulps         %xmm5,%xmm10
  .byte  65,15,88,202                        // addps         %xmm10,%xmm1
  .byte  65,15,89,209                        // mulps         %xmm9,%xmm2
  .byte  68,15,40,211                        // movaps        %xmm3,%xmm10
  .byte  68,15,89,214                        // mulps         %xmm6,%xmm10
  .byte  65,15,88,210                        // addps         %xmm10,%xmm2
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  15,89,223                           // mulps         %xmm7,%xmm3
  .byte  65,15,88,217                        // addps         %xmm9,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_darken_sse2
.globl _sk_darken_sse2
_sk_darken_sse2:
  .byte  68,15,40,193                        // movaps        %xmm1,%xmm8
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  68,15,89,207                        // mulps         %xmm7,%xmm9
  .byte  15,40,203                           // movaps        %xmm3,%xmm1
  .byte  15,89,204                           // mulps         %xmm4,%xmm1
  .byte  68,15,95,201                        // maxps         %xmm1,%xmm9
  .byte  65,15,92,193                        // subps         %xmm9,%xmm0
  .byte  65,15,40,200                        // movaps        %xmm8,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  68,15,89,199                        // mulps         %xmm7,%xmm8
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,89,205                        // mulps         %xmm5,%xmm9
  .byte  69,15,95,193                        // maxps         %xmm9,%xmm8
  .byte  65,15,92,200                        // subps         %xmm8,%xmm1
  .byte  68,15,40,194                        // movaps        %xmm2,%xmm8
  .byte  68,15,88,198                        // addps         %xmm6,%xmm8
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,89,206                        // mulps         %xmm6,%xmm9
  .byte  65,15,95,209                        // maxps         %xmm9,%xmm2
  .byte  68,15,92,194                        // subps         %xmm2,%xmm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  15,92,211                           // subps         %xmm3,%xmm2
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  15,88,218                           // addps         %xmm2,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_lighten_sse2
.globl _sk_lighten_sse2
_sk_lighten_sse2:
  .byte  68,15,40,193                        // movaps        %xmm1,%xmm8
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  68,15,89,207                        // mulps         %xmm7,%xmm9
  .byte  15,40,203                           // movaps        %xmm3,%xmm1
  .byte  15,89,204                           // mulps         %xmm4,%xmm1
  .byte  68,15,93,201                        // minps         %xmm1,%xmm9
  .byte  65,15,92,193                        // subps         %xmm9,%xmm0
  .byte  65,15,40,200                        // movaps        %xmm8,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  68,15,89,199                        // mulps         %xmm7,%xmm8
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,89,205                        // mulps         %xmm5,%xmm9
  .byte  69,15,93,193                        // minps         %xmm9,%xmm8
  .byte  65,15,92,200                        // subps         %xmm8,%xmm1
  .byte  68,15,40,194                        // movaps        %xmm2,%xmm8
  .byte  68,15,88,198                        // addps         %xmm6,%xmm8
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,89,206                        // mulps         %xmm6,%xmm9
  .byte  65,15,93,209                        // minps         %xmm9,%xmm2
  .byte  68,15,92,194                        // subps         %xmm2,%xmm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  15,92,211                           // subps         %xmm3,%xmm2
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  15,88,218                           // addps         %xmm2,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_difference_sse2
.globl _sk_difference_sse2
_sk_difference_sse2:
  .byte  68,15,40,193                        // movaps        %xmm1,%xmm8
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  68,15,89,207                        // mulps         %xmm7,%xmm9
  .byte  15,40,203                           // movaps        %xmm3,%xmm1
  .byte  15,89,204                           // mulps         %xmm4,%xmm1
  .byte  68,15,93,201                        // minps         %xmm1,%xmm9
  .byte  69,15,88,201                        // addps         %xmm9,%xmm9
  .byte  65,15,92,193                        // subps         %xmm9,%xmm0
  .byte  65,15,40,200                        // movaps        %xmm8,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  68,15,89,199                        // mulps         %xmm7,%xmm8
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,89,205                        // mulps         %xmm5,%xmm9
  .byte  69,15,93,193                        // minps         %xmm9,%xmm8
  .byte  69,15,88,192                        // addps         %xmm8,%xmm8
  .byte  65,15,92,200                        // subps         %xmm8,%xmm1
  .byte  68,15,40,194                        // movaps        %xmm2,%xmm8
  .byte  68,15,88,198                        // addps         %xmm6,%xmm8
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,89,206                        // mulps         %xmm6,%xmm9
  .byte  65,15,93,209                        // minps         %xmm9,%xmm2
  .byte  15,88,210                           // addps         %xmm2,%xmm2
  .byte  68,15,92,194                        // subps         %xmm2,%xmm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  15,92,211                           // subps         %xmm3,%xmm2
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  15,88,218                           // addps         %xmm2,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_exclusion_sse2
.globl _sk_exclusion_sse2
_sk_exclusion_sse2:
  .byte  68,15,40,193                        // movaps        %xmm1,%xmm8
  .byte  15,40,200                           // movaps        %xmm0,%xmm1
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  15,89,204                           // mulps         %xmm4,%xmm1
  .byte  15,88,201                           // addps         %xmm1,%xmm1
  .byte  15,92,193                           // subps         %xmm1,%xmm0
  .byte  65,15,40,200                        // movaps        %xmm8,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  68,15,89,197                        // mulps         %xmm5,%xmm8
  .byte  69,15,88,192                        // addps         %xmm8,%xmm8
  .byte  65,15,92,200                        // subps         %xmm8,%xmm1
  .byte  68,15,40,194                        // movaps        %xmm2,%xmm8
  .byte  68,15,88,198                        // addps         %xmm6,%xmm8
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  15,88,210                           // addps         %xmm2,%xmm2
  .byte  68,15,92,194                        // subps         %xmm2,%xmm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  15,92,211                           // subps         %xmm3,%xmm2
  .byte  15,89,215                           // mulps         %xmm7,%xmm2
  .byte  15,88,218                           // addps         %xmm2,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_colorburn_sse2
.globl _sk_colorburn_sse2
_sk_colorburn_sse2:
  .byte  68,15,40,193                        // movaps        %xmm1,%xmm8
  .byte  68,15,40,224                        // movaps        %xmm0,%xmm12
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,200                   // movd          %eax,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  69,15,40,217                        // movaps        %xmm9,%xmm11
  .byte  68,15,92,223                        // subps         %xmm7,%xmm11
  .byte  65,15,40,195                        // movaps        %xmm11,%xmm0
  .byte  65,15,89,196                        // mulps         %xmm12,%xmm0
  .byte  69,15,87,210                        // xorps         %xmm10,%xmm10
  .byte  15,40,207                           // movaps        %xmm7,%xmm1
  .byte  15,92,204                           // subps         %xmm4,%xmm1
  .byte  15,89,203                           // mulps         %xmm3,%xmm1
  .byte  65,15,94,204                        // divps         %xmm12,%xmm1
  .byte  68,15,40,239                        // movaps        %xmm7,%xmm13
  .byte  68,15,93,233                        // minps         %xmm1,%xmm13
  .byte  68,15,40,247                        // movaps        %xmm7,%xmm14
  .byte  69,15,92,245                        // subps         %xmm13,%xmm14
  .byte  65,15,40,204                        // movaps        %xmm12,%xmm1
  .byte  65,15,194,202,0                     // cmpeqps       %xmm10,%xmm1
  .byte  68,15,92,203                        // subps         %xmm3,%xmm9
  .byte  68,15,89,243                        // mulps         %xmm3,%xmm14
  .byte  68,15,88,240                        // addps         %xmm0,%xmm14
  .byte  68,15,84,225                        // andps         %xmm1,%xmm12
  .byte  65,15,85,206                        // andnps        %xmm14,%xmm1
  .byte  69,15,40,233                        // movaps        %xmm9,%xmm13
  .byte  68,15,89,236                        // mulps         %xmm4,%xmm13
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  65,15,86,204                        // orps          %xmm12,%xmm1
  .byte  68,15,40,228                        // movaps        %xmm4,%xmm12
  .byte  68,15,194,231,0                     // cmpeqps       %xmm7,%xmm12
  .byte  65,15,88,205                        // addps         %xmm13,%xmm1
  .byte  65,15,84,196                        // andps         %xmm12,%xmm0
  .byte  68,15,85,225                        // andnps        %xmm1,%xmm12
  .byte  65,15,86,196                        // orps          %xmm12,%xmm0
  .byte  65,15,40,203                        // movaps        %xmm11,%xmm1
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  68,15,40,231                        // movaps        %xmm7,%xmm12
  .byte  68,15,92,229                        // subps         %xmm5,%xmm12
  .byte  68,15,89,227                        // mulps         %xmm3,%xmm12
  .byte  69,15,94,224                        // divps         %xmm8,%xmm12
  .byte  68,15,40,239                        // movaps        %xmm7,%xmm13
  .byte  69,15,93,236                        // minps         %xmm12,%xmm13
  .byte  68,15,40,231                        // movaps        %xmm7,%xmm12
  .byte  69,15,92,229                        // subps         %xmm13,%xmm12
  .byte  69,15,40,232                        // movaps        %xmm8,%xmm13
  .byte  69,15,194,234,0                     // cmpeqps       %xmm10,%xmm13
  .byte  68,15,89,227                        // mulps         %xmm3,%xmm12
  .byte  68,15,88,225                        // addps         %xmm1,%xmm12
  .byte  69,15,84,197                        // andps         %xmm13,%xmm8
  .byte  69,15,85,236                        // andnps        %xmm12,%xmm13
  .byte  69,15,86,232                        // orps          %xmm8,%xmm13
  .byte  69,15,40,193                        // movaps        %xmm9,%xmm8
  .byte  68,15,89,197                        // mulps         %xmm5,%xmm8
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  69,15,88,232                        // addps         %xmm8,%xmm13
  .byte  68,15,40,197                        // movaps        %xmm5,%xmm8
  .byte  68,15,194,199,0                     // cmpeqps       %xmm7,%xmm8
  .byte  65,15,84,200                        // andps         %xmm8,%xmm1
  .byte  69,15,85,197                        // andnps        %xmm13,%xmm8
  .byte  65,15,86,200                        // orps          %xmm8,%xmm1
  .byte  68,15,40,199                        // movaps        %xmm7,%xmm8
  .byte  68,15,92,198                        // subps         %xmm6,%xmm8
  .byte  68,15,89,195                        // mulps         %xmm3,%xmm8
  .byte  68,15,94,194                        // divps         %xmm2,%xmm8
  .byte  68,15,40,231                        // movaps        %xmm7,%xmm12
  .byte  69,15,93,224                        // minps         %xmm8,%xmm12
  .byte  68,15,40,199                        // movaps        %xmm7,%xmm8
  .byte  69,15,92,196                        // subps         %xmm12,%xmm8
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  68,15,194,210,0                     // cmpeqps       %xmm2,%xmm10
  .byte  68,15,89,195                        // mulps         %xmm3,%xmm8
  .byte  69,15,88,195                        // addps         %xmm11,%xmm8
  .byte  65,15,84,210                        // andps         %xmm10,%xmm2
  .byte  69,15,85,208                        // andnps        %xmm8,%xmm10
  .byte  69,15,40,195                        // movaps        %xmm11,%xmm8
  .byte  68,15,88,198                        // addps         %xmm6,%xmm8
  .byte  68,15,86,210                        // orps          %xmm2,%xmm10
  .byte  65,15,40,209                        // movaps        %xmm9,%xmm2
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  68,15,88,210                        // addps         %xmm2,%xmm10
  .byte  15,40,214                           // movaps        %xmm6,%xmm2
  .byte  15,194,215,0                        // cmpeqps       %xmm7,%xmm2
  .byte  68,15,84,194                        // andps         %xmm2,%xmm8
  .byte  65,15,85,210                        // andnps        %xmm10,%xmm2
  .byte  68,15,86,194                        // orps          %xmm2,%xmm8
  .byte  68,15,89,207                        // mulps         %xmm7,%xmm9
  .byte  65,15,88,217                        // addps         %xmm9,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_colordodge_sse2
.globl _sk_colordodge_sse2
_sk_colordodge_sse2:
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,208                   // movd          %eax,%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  69,15,40,218                        // movaps        %xmm10,%xmm11
  .byte  68,15,92,223                        // subps         %xmm7,%xmm11
  .byte  65,15,40,195                        // movaps        %xmm11,%xmm0
  .byte  65,15,89,193                        // mulps         %xmm9,%xmm0
  .byte  68,15,40,195                        // movaps        %xmm3,%xmm8
  .byte  68,15,89,196                        // mulps         %xmm4,%xmm8
  .byte  68,15,40,227                        // movaps        %xmm3,%xmm12
  .byte  69,15,92,225                        // subps         %xmm9,%xmm12
  .byte  69,15,94,196                        // divps         %xmm12,%xmm8
  .byte  68,15,40,231                        // movaps        %xmm7,%xmm12
  .byte  68,15,40,239                        // movaps        %xmm7,%xmm13
  .byte  69,15,93,232                        // minps         %xmm8,%xmm13
  .byte  69,15,40,241                        // movaps        %xmm9,%xmm14
  .byte  68,15,194,243,0                     // cmpeqps       %xmm3,%xmm14
  .byte  68,15,89,235                        // mulps         %xmm3,%xmm13
  .byte  68,15,88,232                        // addps         %xmm0,%xmm13
  .byte  69,15,84,206                        // andps         %xmm14,%xmm9
  .byte  69,15,85,245                        // andnps        %xmm13,%xmm14
  .byte  69,15,87,192                        // xorps         %xmm8,%xmm8
  .byte  68,15,92,211                        // subps         %xmm3,%xmm10
  .byte  69,15,86,241                        // orps          %xmm9,%xmm14
  .byte  69,15,40,202                        // movaps        %xmm10,%xmm9
  .byte  68,15,89,204                        // mulps         %xmm4,%xmm9
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  69,15,88,241                        // addps         %xmm9,%xmm14
  .byte  68,15,40,204                        // movaps        %xmm4,%xmm9
  .byte  69,15,194,200,0                     // cmpeqps       %xmm8,%xmm9
  .byte  65,15,84,193                        // andps         %xmm9,%xmm0
  .byte  69,15,85,206                        // andnps        %xmm14,%xmm9
  .byte  65,15,86,193                        // orps          %xmm9,%xmm0
  .byte  68,15,40,235                        // movaps        %xmm3,%xmm13
  .byte  68,15,89,237                        // mulps         %xmm5,%xmm13
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,92,201                        // subps         %xmm1,%xmm9
  .byte  69,15,94,233                        // divps         %xmm9,%xmm13
  .byte  69,15,40,203                        // movaps        %xmm11,%xmm9
  .byte  68,15,89,201                        // mulps         %xmm1,%xmm9
  .byte  69,15,93,229                        // minps         %xmm13,%xmm12
  .byte  68,15,40,233                        // movaps        %xmm1,%xmm13
  .byte  68,15,194,235,0                     // cmpeqps       %xmm3,%xmm13
  .byte  68,15,89,227                        // mulps         %xmm3,%xmm12
  .byte  69,15,88,225                        // addps         %xmm9,%xmm12
  .byte  65,15,84,205                        // andps         %xmm13,%xmm1
  .byte  69,15,85,236                        // andnps        %xmm12,%xmm13
  .byte  68,15,86,233                        // orps          %xmm1,%xmm13
  .byte  65,15,40,202                        // movaps        %xmm10,%xmm1
  .byte  15,89,205                           // mulps         %xmm5,%xmm1
  .byte  68,15,88,205                        // addps         %xmm5,%xmm9
  .byte  68,15,88,233                        // addps         %xmm1,%xmm13
  .byte  15,40,205                           // movaps        %xmm5,%xmm1
  .byte  65,15,194,200,0                     // cmpeqps       %xmm8,%xmm1
  .byte  68,15,84,201                        // andps         %xmm1,%xmm9
  .byte  65,15,85,205                        // andnps        %xmm13,%xmm1
  .byte  68,15,86,201                        // orps          %xmm1,%xmm9
  .byte  68,15,40,227                        // movaps        %xmm3,%xmm12
  .byte  68,15,89,230                        // mulps         %xmm6,%xmm12
  .byte  15,40,203                           // movaps        %xmm3,%xmm1
  .byte  15,92,202                           // subps         %xmm2,%xmm1
  .byte  68,15,94,225                        // divps         %xmm1,%xmm12
  .byte  68,15,40,239                        // movaps        %xmm7,%xmm13
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  69,15,93,236                        // minps         %xmm12,%xmm13
  .byte  15,40,202                           // movaps        %xmm2,%xmm1
  .byte  15,194,203,0                        // cmpeqps       %xmm3,%xmm1
  .byte  68,15,89,235                        // mulps         %xmm3,%xmm13
  .byte  69,15,88,235                        // addps         %xmm11,%xmm13
  .byte  15,84,209                           // andps         %xmm1,%xmm2
  .byte  65,15,85,205                        // andnps        %xmm13,%xmm1
  .byte  15,86,202                           // orps          %xmm2,%xmm1
  .byte  65,15,40,210                        // movaps        %xmm10,%xmm2
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  15,88,202                           // addps         %xmm2,%xmm1
  .byte  68,15,194,198,0                     // cmpeqps       %xmm6,%xmm8
  .byte  68,15,88,222                        // addps         %xmm6,%xmm11
  .byte  69,15,84,216                        // andps         %xmm8,%xmm11
  .byte  68,15,85,193                        // andnps        %xmm1,%xmm8
  .byte  69,15,86,195                        // orps          %xmm11,%xmm8
  .byte  68,15,89,215                        // mulps         %xmm7,%xmm10
  .byte  65,15,88,218                        // addps         %xmm10,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,201                        // movaps        %xmm9,%xmm1
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_hardlight_sse2
.globl _sk_hardlight_sse2
_sk_hardlight_sse2:
  .byte  15,41,116,36,232                    // movaps        %xmm6,-0x18(%rsp)
  .byte  15,40,245                           // movaps        %xmm5,%xmm6
  .byte  15,40,236                           // movaps        %xmm4,%xmm5
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,216                   // movd          %eax,%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  69,15,40,211                        // movaps        %xmm11,%xmm10
  .byte  68,15,92,215                        // subps         %xmm7,%xmm10
  .byte  69,15,40,194                        // movaps        %xmm10,%xmm8
  .byte  68,15,89,192                        // mulps         %xmm0,%xmm8
  .byte  68,15,92,219                        // subps         %xmm3,%xmm11
  .byte  69,15,40,203                        // movaps        %xmm11,%xmm9
  .byte  68,15,89,205                        // mulps         %xmm5,%xmm9
  .byte  69,15,88,200                        // addps         %xmm8,%xmm9
  .byte  68,15,40,195                        // movaps        %xmm3,%xmm8
  .byte  68,15,92,192                        // subps         %xmm0,%xmm8
  .byte  15,40,227                           // movaps        %xmm3,%xmm4
  .byte  15,89,231                           // mulps         %xmm7,%xmm4
  .byte  68,15,40,239                        // movaps        %xmm7,%xmm13
  .byte  68,15,40,247                        // movaps        %xmm7,%xmm14
  .byte  68,15,40,255                        // movaps        %xmm7,%xmm15
  .byte  68,15,92,253                        // subps         %xmm5,%xmm15
  .byte  69,15,89,248                        // mulps         %xmm8,%xmm15
  .byte  69,15,88,255                        // addps         %xmm15,%xmm15
  .byte  68,15,40,228                        // movaps        %xmm4,%xmm12
  .byte  69,15,92,231                        // subps         %xmm15,%xmm12
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  69,15,88,192                        // addps         %xmm8,%xmm8
  .byte  68,15,194,195,2                     // cmpleps       %xmm3,%xmm8
  .byte  15,89,197                           // mulps         %xmm5,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  65,15,84,192                        // andps         %xmm8,%xmm0
  .byte  69,15,85,196                        // andnps        %xmm12,%xmm8
  .byte  68,15,86,192                        // orps          %xmm0,%xmm8
  .byte  69,15,40,251                        // movaps        %xmm11,%xmm15
  .byte  69,15,40,227                        // movaps        %xmm11,%xmm12
  .byte  68,15,89,223                        // mulps         %xmm7,%xmm11
  .byte  69,15,88,193                        // addps         %xmm9,%xmm8
  .byte  65,15,40,194                        // movaps        %xmm10,%xmm0
  .byte  15,89,193                           // mulps         %xmm1,%xmm0
  .byte  68,15,89,254                        // mulps         %xmm6,%xmm15
  .byte  68,15,88,248                        // addps         %xmm0,%xmm15
  .byte  15,40,195                           // movaps        %xmm3,%xmm0
  .byte  15,92,193                           // subps         %xmm1,%xmm0
  .byte  68,15,92,238                        // subps         %xmm6,%xmm13
  .byte  68,15,89,232                        // mulps         %xmm0,%xmm13
  .byte  69,15,88,237                        // addps         %xmm13,%xmm13
  .byte  15,40,196                           // movaps        %xmm4,%xmm0
  .byte  65,15,92,197                        // subps         %xmm13,%xmm0
  .byte  68,15,40,201                        // movaps        %xmm1,%xmm9
  .byte  69,15,88,201                        // addps         %xmm9,%xmm9
  .byte  68,15,194,203,2                     // cmpleps       %xmm3,%xmm9
  .byte  15,89,206                           // mulps         %xmm6,%xmm1
  .byte  15,88,201                           // addps         %xmm1,%xmm1
  .byte  65,15,84,201                        // andps         %xmm9,%xmm1
  .byte  68,15,85,200                        // andnps        %xmm0,%xmm9
  .byte  68,15,86,201                        // orps          %xmm1,%xmm9
  .byte  69,15,88,207                        // addps         %xmm15,%xmm9
  .byte  68,15,89,210                        // mulps         %xmm2,%xmm10
  .byte  68,15,40,108,36,232                 // movaps        -0x18(%rsp),%xmm13
  .byte  69,15,89,229                        // mulps         %xmm13,%xmm12
  .byte  69,15,88,226                        // addps         %xmm10,%xmm12
  .byte  68,15,40,210                        // movaps        %xmm2,%xmm10
  .byte  69,15,88,210                        // addps         %xmm10,%xmm10
  .byte  68,15,194,211,2                     // cmpleps       %xmm3,%xmm10
  .byte  15,40,195                           // movaps        %xmm3,%xmm0
  .byte  15,92,194                           // subps         %xmm2,%xmm0
  .byte  65,15,89,213                        // mulps         %xmm13,%xmm2
  .byte  15,88,210                           // addps         %xmm2,%xmm2
  .byte  69,15,92,245                        // subps         %xmm13,%xmm14
  .byte  68,15,89,240                        // mulps         %xmm0,%xmm14
  .byte  69,15,88,246                        // addps         %xmm14,%xmm14
  .byte  65,15,92,230                        // subps         %xmm14,%xmm4
  .byte  65,15,84,210                        // andps         %xmm10,%xmm2
  .byte  68,15,85,212                        // andnps        %xmm4,%xmm10
  .byte  68,15,86,210                        // orps          %xmm2,%xmm10
  .byte  69,15,88,212                        // addps         %xmm12,%xmm10
  .byte  65,15,88,219                        // addps         %xmm11,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  65,15,40,201                        // movaps        %xmm9,%xmm1
  .byte  65,15,40,210                        // movaps        %xmm10,%xmm2
  .byte  15,40,229                           // movaps        %xmm5,%xmm4
  .byte  15,40,238                           // movaps        %xmm6,%xmm5
  .byte  65,15,40,245                        // movaps        %xmm13,%xmm6
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_overlay_sse2
.globl _sk_overlay_sse2
_sk_overlay_sse2:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,92,207                        // subps         %xmm7,%xmm9
  .byte  69,15,40,209                        // movaps        %xmm9,%xmm10
  .byte  68,15,89,208                        // mulps         %xmm0,%xmm10
  .byte  68,15,92,195                        // subps         %xmm3,%xmm8
  .byte  69,15,40,216                        // movaps        %xmm8,%xmm11
  .byte  68,15,89,220                        // mulps         %xmm4,%xmm11
  .byte  69,15,88,218                        // addps         %xmm10,%xmm11
  .byte  68,15,40,227                        // movaps        %xmm3,%xmm12
  .byte  68,15,92,224                        // subps         %xmm0,%xmm12
  .byte  15,89,196                           // mulps         %xmm4,%xmm0
  .byte  68,15,40,239                        // movaps        %xmm7,%xmm13
  .byte  68,15,92,236                        // subps         %xmm4,%xmm13
  .byte  68,15,40,244                        // movaps        %xmm4,%xmm14
  .byte  69,15,88,246                        // addps         %xmm14,%xmm14
  .byte  68,15,194,247,2                     // cmpleps       %xmm7,%xmm14
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  68,15,40,211                        // movaps        %xmm3,%xmm10
  .byte  68,15,89,215                        // mulps         %xmm7,%xmm10
  .byte  69,15,89,236                        // mulps         %xmm12,%xmm13
  .byte  69,15,88,237                        // addps         %xmm13,%xmm13
  .byte  69,15,40,226                        // movaps        %xmm10,%xmm12
  .byte  69,15,92,229                        // subps         %xmm13,%xmm12
  .byte  65,15,84,198                        // andps         %xmm14,%xmm0
  .byte  69,15,85,244                        // andnps        %xmm12,%xmm14
  .byte  65,15,86,198                        // orps          %xmm14,%xmm0
  .byte  65,15,88,195                        // addps         %xmm11,%xmm0
  .byte  69,15,40,217                        // movaps        %xmm9,%xmm11
  .byte  68,15,89,217                        // mulps         %xmm1,%xmm11
  .byte  69,15,40,224                        // movaps        %xmm8,%xmm12
  .byte  68,15,89,229                        // mulps         %xmm5,%xmm12
  .byte  69,15,88,227                        // addps         %xmm11,%xmm12
  .byte  68,15,40,219                        // movaps        %xmm3,%xmm11
  .byte  68,15,92,217                        // subps         %xmm1,%xmm11
  .byte  15,89,205                           // mulps         %xmm5,%xmm1
  .byte  68,15,40,239                        // movaps        %xmm7,%xmm13
  .byte  68,15,92,237                        // subps         %xmm5,%xmm13
  .byte  68,15,40,245                        // movaps        %xmm5,%xmm14
  .byte  69,15,88,246                        // addps         %xmm14,%xmm14
  .byte  68,15,194,247,2                     // cmpleps       %xmm7,%xmm14
  .byte  15,88,201                           // addps         %xmm1,%xmm1
  .byte  69,15,89,235                        // mulps         %xmm11,%xmm13
  .byte  69,15,88,237                        // addps         %xmm13,%xmm13
  .byte  69,15,40,218                        // movaps        %xmm10,%xmm11
  .byte  69,15,92,221                        // subps         %xmm13,%xmm11
  .byte  65,15,84,206                        // andps         %xmm14,%xmm1
  .byte  69,15,85,243                        // andnps        %xmm11,%xmm14
  .byte  65,15,86,206                        // orps          %xmm14,%xmm1
  .byte  65,15,88,204                        // addps         %xmm12,%xmm1
  .byte  68,15,89,202                        // mulps         %xmm2,%xmm9
  .byte  69,15,40,216                        // movaps        %xmm8,%xmm11
  .byte  68,15,89,222                        // mulps         %xmm6,%xmm11
  .byte  69,15,88,217                        // addps         %xmm9,%xmm11
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  68,15,92,202                        // subps         %xmm2,%xmm9
  .byte  15,89,214                           // mulps         %xmm6,%xmm2
  .byte  68,15,40,231                        // movaps        %xmm7,%xmm12
  .byte  68,15,92,230                        // subps         %xmm6,%xmm12
  .byte  68,15,40,238                        // movaps        %xmm6,%xmm13
  .byte  69,15,88,237                        // addps         %xmm13,%xmm13
  .byte  68,15,194,239,2                     // cmpleps       %xmm7,%xmm13
  .byte  15,88,210                           // addps         %xmm2,%xmm2
  .byte  69,15,89,225                        // mulps         %xmm9,%xmm12
  .byte  69,15,88,228                        // addps         %xmm12,%xmm12
  .byte  69,15,92,212                        // subps         %xmm12,%xmm10
  .byte  65,15,84,213                        // andps         %xmm13,%xmm2
  .byte  69,15,85,234                        // andnps        %xmm10,%xmm13
  .byte  65,15,86,213                        // orps          %xmm13,%xmm2
  .byte  65,15,88,211                        // addps         %xmm11,%xmm2
  .byte  68,15,89,199                        // mulps         %xmm7,%xmm8
  .byte  65,15,88,216                        // addps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_softlight_sse2
.globl _sk_softlight_sse2
_sk_softlight_sse2:
  .byte  15,41,84,36,232                     // movaps        %xmm2,-0x18(%rsp)
  .byte  15,40,209                           // movaps        %xmm1,%xmm2
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  69,15,87,228                        // xorps         %xmm12,%xmm12
  .byte  68,15,194,231,1                     // cmpltps       %xmm7,%xmm12
  .byte  68,15,40,212                        // movaps        %xmm4,%xmm10
  .byte  68,15,94,215                        // divps         %xmm7,%xmm10
  .byte  69,15,84,212                        // andps         %xmm12,%xmm10
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,200                   // movd          %eax,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  69,15,40,241                        // movaps        %xmm9,%xmm14
  .byte  69,15,92,242                        // subps         %xmm10,%xmm14
  .byte  69,15,40,218                        // movaps        %xmm10,%xmm11
  .byte  69,15,40,234                        // movaps        %xmm10,%xmm13
  .byte  65,15,82,194                        // rsqrtps       %xmm10,%xmm0
  .byte  68,15,83,248                        // rcpps         %xmm0,%xmm15
  .byte  69,15,92,250                        // subps         %xmm10,%xmm15
  .byte  69,15,88,210                        // addps         %xmm10,%xmm10
  .byte  69,15,88,210                        // addps         %xmm10,%xmm10
  .byte  65,15,40,194                        // movaps        %xmm10,%xmm0
  .byte  15,89,192                           // mulps         %xmm0,%xmm0
  .byte  65,15,88,194                        // addps         %xmm10,%xmm0
  .byte  69,15,92,217                        // subps         %xmm9,%xmm11
  .byte  68,15,89,216                        // mulps         %xmm0,%xmm11
  .byte  184,0,0,224,64                      // mov           $0x40e00000,%eax
  .byte  102,68,15,110,208                   // movd          %eax,%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  69,15,89,234                        // mulps         %xmm10,%xmm13
  .byte  69,15,88,235                        // addps         %xmm11,%xmm13
  .byte  68,15,40,219                        // movaps        %xmm3,%xmm11
  .byte  15,40,204                           // movaps        %xmm4,%xmm1
  .byte  68,15,89,217                        // mulps         %xmm1,%xmm11
  .byte  15,88,228                           // addps         %xmm4,%xmm4
  .byte  15,88,228                           // addps         %xmm4,%xmm4
  .byte  15,194,231,2                        // cmpleps       %xmm7,%xmm4
  .byte  68,15,84,236                        // andps         %xmm4,%xmm13
  .byte  65,15,85,231                        // andnps        %xmm15,%xmm4
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  15,88,192                           // addps         %xmm0,%xmm0
  .byte  65,15,86,229                        // orps          %xmm13,%xmm4
  .byte  68,15,40,232                        // movaps        %xmm0,%xmm13
  .byte  68,15,92,235                        // subps         %xmm3,%xmm13
  .byte  69,15,89,245                        // mulps         %xmm13,%xmm14
  .byte  68,15,89,239                        // mulps         %xmm7,%xmm13
  .byte  65,15,89,229                        // mulps         %xmm13,%xmm4
  .byte  65,15,88,227                        // addps         %xmm11,%xmm4
  .byte  69,15,40,217                        // movaps        %xmm9,%xmm11
  .byte  68,15,92,219                        // subps         %xmm3,%xmm11
  .byte  69,15,40,251                        // movaps        %xmm11,%xmm15
  .byte  15,41,76,36,216                     // movaps        %xmm1,-0x28(%rsp)
  .byte  68,15,89,249                        // mulps         %xmm1,%xmm15
  .byte  69,15,40,233                        // movaps        %xmm9,%xmm13
  .byte  68,15,92,239                        // subps         %xmm7,%xmm13
  .byte  69,15,89,197                        // mulps         %xmm13,%xmm8
  .byte  69,15,88,199                        // addps         %xmm15,%xmm8
  .byte  68,15,88,243                        // addps         %xmm3,%xmm14
  .byte  68,15,89,241                        // mulps         %xmm1,%xmm14
  .byte  15,194,195,2                        // cmpleps       %xmm3,%xmm0
  .byte  68,15,84,240                        // andps         %xmm0,%xmm14
  .byte  15,85,196                           // andnps        %xmm4,%xmm0
  .byte  65,15,86,198                        // orps          %xmm14,%xmm0
  .byte  65,15,88,192                        // addps         %xmm8,%xmm0
  .byte  15,41,108,36,200                    // movaps        %xmm5,-0x38(%rsp)
  .byte  68,15,40,197                        // movaps        %xmm5,%xmm8
  .byte  68,15,94,199                        // divps         %xmm7,%xmm8
  .byte  69,15,84,196                        // andps         %xmm12,%xmm8
  .byte  69,15,40,240                        // movaps        %xmm8,%xmm14
  .byte  69,15,88,246                        // addps         %xmm14,%xmm14
  .byte  69,15,88,246                        // addps         %xmm14,%xmm14
  .byte  65,15,40,230                        // movaps        %xmm14,%xmm4
  .byte  15,89,228                           // mulps         %xmm4,%xmm4
  .byte  65,15,88,230                        // addps         %xmm14,%xmm4
  .byte  69,15,40,248                        // movaps        %xmm8,%xmm15
  .byte  69,15,92,249                        // subps         %xmm9,%xmm15
  .byte  68,15,89,252                        // mulps         %xmm4,%xmm15
  .byte  69,15,40,241                        // movaps        %xmm9,%xmm14
  .byte  69,15,92,240                        // subps         %xmm8,%xmm14
  .byte  65,15,82,224                        // rsqrtps       %xmm8,%xmm4
  .byte  15,83,228                           // rcpps         %xmm4,%xmm4
  .byte  65,15,92,224                        // subps         %xmm8,%xmm4
  .byte  69,15,89,194                        // mulps         %xmm10,%xmm8
  .byte  69,15,88,199                        // addps         %xmm15,%xmm8
  .byte  68,15,40,253                        // movaps        %xmm5,%xmm15
  .byte  69,15,88,255                        // addps         %xmm15,%xmm15
  .byte  69,15,88,255                        // addps         %xmm15,%xmm15
  .byte  68,15,194,255,2                     // cmpleps       %xmm7,%xmm15
  .byte  69,15,84,199                        // andps         %xmm15,%xmm8
  .byte  68,15,85,252                        // andnps        %xmm4,%xmm15
  .byte  69,15,86,248                        // orps          %xmm8,%xmm15
  .byte  68,15,40,194                        // movaps        %xmm2,%xmm8
  .byte  69,15,88,192                        // addps         %xmm8,%xmm8
  .byte  65,15,40,224                        // movaps        %xmm8,%xmm4
  .byte  15,92,227                           // subps         %xmm3,%xmm4
  .byte  68,15,89,244                        // mulps         %xmm4,%xmm14
  .byte  15,89,231                           // mulps         %xmm7,%xmm4
  .byte  68,15,89,252                        // mulps         %xmm4,%xmm15
  .byte  15,40,227                           // movaps        %xmm3,%xmm4
  .byte  15,89,229                           // mulps         %xmm5,%xmm4
  .byte  68,15,88,252                        // addps         %xmm4,%xmm15
  .byte  65,15,40,227                        // movaps        %xmm11,%xmm4
  .byte  15,89,229                           // mulps         %xmm5,%xmm4
  .byte  65,15,89,213                        // mulps         %xmm13,%xmm2
  .byte  15,88,212                           // addps         %xmm4,%xmm2
  .byte  68,15,88,243                        // addps         %xmm3,%xmm14
  .byte  68,15,89,245                        // mulps         %xmm5,%xmm14
  .byte  68,15,194,195,2                     // cmpleps       %xmm3,%xmm8
  .byte  69,15,84,240                        // andps         %xmm8,%xmm14
  .byte  69,15,85,199                        // andnps        %xmm15,%xmm8
  .byte  69,15,86,198                        // orps          %xmm14,%xmm8
  .byte  68,15,88,194                        // addps         %xmm2,%xmm8
  .byte  68,15,40,246                        // movaps        %xmm6,%xmm14
  .byte  65,15,40,206                        // movaps        %xmm14,%xmm1
  .byte  15,94,207                           // divps         %xmm7,%xmm1
  .byte  65,15,84,204                        // andps         %xmm12,%xmm1
  .byte  15,40,225                           // movaps        %xmm1,%xmm4
  .byte  65,15,92,225                        // subps         %xmm9,%xmm4
  .byte  68,15,92,201                        // subps         %xmm1,%xmm9
  .byte  68,15,89,209                        // mulps         %xmm1,%xmm10
  .byte  15,82,241                           // rsqrtps       %xmm1,%xmm6
  .byte  15,83,246                           // rcpps         %xmm6,%xmm6
  .byte  15,92,241                           // subps         %xmm1,%xmm6
  .byte  15,88,201                           // addps         %xmm1,%xmm1
  .byte  15,88,201                           // addps         %xmm1,%xmm1
  .byte  15,40,233                           // movaps        %xmm1,%xmm5
  .byte  15,89,237                           // mulps         %xmm5,%xmm5
  .byte  15,88,233                           // addps         %xmm1,%xmm5
  .byte  15,89,236                           // mulps         %xmm4,%xmm5
  .byte  68,15,88,213                        // addps         %xmm5,%xmm10
  .byte  65,15,40,238                        // movaps        %xmm14,%xmm5
  .byte  15,40,205                           // movaps        %xmm5,%xmm1
  .byte  15,88,201                           // addps         %xmm1,%xmm1
  .byte  15,88,201                           // addps         %xmm1,%xmm1
  .byte  15,194,207,2                        // cmpleps       %xmm7,%xmm1
  .byte  68,15,84,209                        // andps         %xmm1,%xmm10
  .byte  15,85,206                           // andnps        %xmm6,%xmm1
  .byte  15,40,84,36,232                     // movaps        -0x18(%rsp),%xmm2
  .byte  68,15,89,234                        // mulps         %xmm2,%xmm13
  .byte  15,88,210                           // addps         %xmm2,%xmm2
  .byte  65,15,86,202                        // orps          %xmm10,%xmm1
  .byte  15,40,226                           // movaps        %xmm2,%xmm4
  .byte  15,92,227                           // subps         %xmm3,%xmm4
  .byte  68,15,89,204                        // mulps         %xmm4,%xmm9
  .byte  15,89,231                           // mulps         %xmm7,%xmm4
  .byte  15,89,204                           // mulps         %xmm4,%xmm1
  .byte  15,40,227                           // movaps        %xmm3,%xmm4
  .byte  15,89,229                           // mulps         %xmm5,%xmm4
  .byte  15,88,204                           // addps         %xmm4,%xmm1
  .byte  65,15,40,227                        // movaps        %xmm11,%xmm4
  .byte  15,89,229                           // mulps         %xmm5,%xmm4
  .byte  65,15,88,229                        // addps         %xmm13,%xmm4
  .byte  68,15,88,203                        // addps         %xmm3,%xmm9
  .byte  68,15,89,205                        // mulps         %xmm5,%xmm9
  .byte  15,40,245                           // movaps        %xmm5,%xmm6
  .byte  15,194,211,2                        // cmpleps       %xmm3,%xmm2
  .byte  68,15,84,202                        // andps         %xmm2,%xmm9
  .byte  15,85,209                           // andnps        %xmm1,%xmm2
  .byte  65,15,86,209                        // orps          %xmm9,%xmm2
  .byte  15,88,212                           // addps         %xmm4,%xmm2
  .byte  68,15,89,223                        // mulps         %xmm7,%xmm11
  .byte  65,15,88,219                        // addps         %xmm11,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,100,36,216                    // movaps        -0x28(%rsp),%xmm4
  .byte  15,40,108,36,200                    // movaps        -0x38(%rsp),%xmm5
  .byte  65,15,40,200                        // movaps        %xmm8,%xmm1
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_0_sse2
.globl _sk_clamp_0_sse2
_sk_clamp_0_sse2:
  .byte  69,15,87,192                        // xorps         %xmm8,%xmm8
  .byte  65,15,95,192                        // maxps         %xmm8,%xmm0
  .byte  65,15,95,200                        // maxps         %xmm8,%xmm1
  .byte  65,15,95,208                        // maxps         %xmm8,%xmm2
  .byte  65,15,95,216                        // maxps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_1_sse2
.globl _sk_clamp_1_sse2
_sk_clamp_1_sse2:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  65,15,93,192                        // minps         %xmm8,%xmm0
  .byte  65,15,93,200                        // minps         %xmm8,%xmm1
  .byte  65,15,93,208                        // minps         %xmm8,%xmm2
  .byte  65,15,93,216                        // minps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_a_sse2
.globl _sk_clamp_a_sse2
_sk_clamp_a_sse2:
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  65,15,93,216                        // minps         %xmm8,%xmm3
  .byte  15,93,195                           // minps         %xmm3,%xmm0
  .byte  15,93,203                           // minps         %xmm3,%xmm1
  .byte  15,93,211                           // minps         %xmm3,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_set_rgb_sse2
.globl _sk_set_rgb_sse2
_sk_set_rgb_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,15,16,0                         // movss         (%rax),%xmm0
  .byte  243,15,16,72,4                      // movss         0x4(%rax),%xmm1
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  243,15,16,80,8                      // movss         0x8(%rax),%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_swap_rb_sse2
.globl _sk_swap_rb_sse2
_sk_swap_rb_sse2:
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,194                           // movaps        %xmm2,%xmm0
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_swap_sse2
.globl _sk_swap_sse2
_sk_swap_sse2:
  .byte  68,15,40,195                        // movaps        %xmm3,%xmm8
  .byte  68,15,40,202                        // movaps        %xmm2,%xmm9
  .byte  68,15,40,209                        // movaps        %xmm1,%xmm10
  .byte  68,15,40,216                        // movaps        %xmm0,%xmm11
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,196                           // movaps        %xmm4,%xmm0
  .byte  15,40,205                           // movaps        %xmm5,%xmm1
  .byte  15,40,214                           // movaps        %xmm6,%xmm2
  .byte  15,40,223                           // movaps        %xmm7,%xmm3
  .byte  65,15,40,227                        // movaps        %xmm11,%xmm4
  .byte  65,15,40,234                        // movaps        %xmm10,%xmm5
  .byte  65,15,40,241                        // movaps        %xmm9,%xmm6
  .byte  65,15,40,248                        // movaps        %xmm8,%xmm7
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_move_src_dst_sse2
.globl _sk_move_src_dst_sse2
_sk_move_src_dst_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,224                           // movaps        %xmm0,%xmm4
  .byte  15,40,233                           // movaps        %xmm1,%xmm5
  .byte  15,40,242                           // movaps        %xmm2,%xmm6
  .byte  15,40,251                           // movaps        %xmm3,%xmm7
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_move_dst_src_sse2
.globl _sk_move_dst_src_sse2
_sk_move_dst_src_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,196                           // movaps        %xmm4,%xmm0
  .byte  15,40,205                           // movaps        %xmm5,%xmm1
  .byte  15,40,214                           // movaps        %xmm6,%xmm2
  .byte  15,40,223                           // movaps        %xmm7,%xmm3
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_premul_sse2
.globl _sk_premul_sse2
_sk_premul_sse2:
  .byte  15,89,195                           // mulps         %xmm3,%xmm0
  .byte  15,89,203                           // mulps         %xmm3,%xmm1
  .byte  15,89,211                           // mulps         %xmm3,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_unpremul_sse2
.globl _sk_unpremul_sse2
_sk_unpremul_sse2:
  .byte  69,15,87,192                        // xorps         %xmm8,%xmm8
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,200                   // movd          %eax,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  68,15,94,203                        // divps         %xmm3,%xmm9
  .byte  68,15,194,195,4                     // cmpneqps      %xmm3,%xmm8
  .byte  69,15,84,193                        // andps         %xmm9,%xmm8
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_from_srgb_sse2
.globl _sk_from_srgb_sse2
_sk_from_srgb_sse2:
  .byte  184,145,131,158,61                  // mov           $0x3d9e8391,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,40,232                        // movaps        %xmm8,%xmm13
  .byte  68,15,89,232                        // mulps         %xmm0,%xmm13
  .byte  68,15,40,224                        // movaps        %xmm0,%xmm12
  .byte  69,15,89,228                        // mulps         %xmm12,%xmm12
  .byte  184,154,153,153,62                  // mov           $0x3e99999a,%eax
  .byte  102,68,15,110,200                   // movd          %eax,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  184,92,143,50,63                    // mov           $0x3f328f5c,%eax
  .byte  102,68,15,110,208                   // movd          %eax,%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  69,15,40,241                        // movaps        %xmm9,%xmm14
  .byte  68,15,89,240                        // mulps         %xmm0,%xmm14
  .byte  69,15,88,242                        // addps         %xmm10,%xmm14
  .byte  184,10,215,35,59                    // mov           $0x3b23d70a,%eax
  .byte  102,68,15,110,216                   // movd          %eax,%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  69,15,89,244                        // mulps         %xmm12,%xmm14
  .byte  69,15,88,243                        // addps         %xmm11,%xmm14
  .byte  184,174,71,97,61                    // mov           $0x3d6147ae,%eax
  .byte  102,68,15,110,224                   // movd          %eax,%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  65,15,194,196,1                     // cmpltps       %xmm12,%xmm0
  .byte  68,15,84,232                        // andps         %xmm0,%xmm13
  .byte  65,15,85,198                        // andnps        %xmm14,%xmm0
  .byte  65,15,86,197                        // orps          %xmm13,%xmm0
  .byte  69,15,40,232                        // movaps        %xmm8,%xmm13
  .byte  68,15,89,233                        // mulps         %xmm1,%xmm13
  .byte  68,15,40,241                        // movaps        %xmm1,%xmm14
  .byte  69,15,89,246                        // mulps         %xmm14,%xmm14
  .byte  69,15,40,249                        // movaps        %xmm9,%xmm15
  .byte  68,15,89,249                        // mulps         %xmm1,%xmm15
  .byte  69,15,88,250                        // addps         %xmm10,%xmm15
  .byte  69,15,89,254                        // mulps         %xmm14,%xmm15
  .byte  69,15,88,251                        // addps         %xmm11,%xmm15
  .byte  65,15,194,204,1                     // cmpltps       %xmm12,%xmm1
  .byte  68,15,84,233                        // andps         %xmm1,%xmm13
  .byte  65,15,85,207                        // andnps        %xmm15,%xmm1
  .byte  65,15,86,205                        // orps          %xmm13,%xmm1
  .byte  68,15,89,194                        // mulps         %xmm2,%xmm8
  .byte  68,15,40,234                        // movaps        %xmm2,%xmm13
  .byte  69,15,89,237                        // mulps         %xmm13,%xmm13
  .byte  68,15,89,202                        // mulps         %xmm2,%xmm9
  .byte  69,15,88,202                        // addps         %xmm10,%xmm9
  .byte  69,15,89,205                        // mulps         %xmm13,%xmm9
  .byte  69,15,88,203                        // addps         %xmm11,%xmm9
  .byte  65,15,194,212,1                     // cmpltps       %xmm12,%xmm2
  .byte  68,15,84,194                        // andps         %xmm2,%xmm8
  .byte  65,15,85,209                        // andnps        %xmm9,%xmm2
  .byte  65,15,86,208                        // orps          %xmm8,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_to_srgb_sse2
.globl _sk_to_srgb_sse2
_sk_to_srgb_sse2:
  .byte  68,15,82,192                        // rsqrtps       %xmm0,%xmm8
  .byte  69,15,83,248                        // rcpps         %xmm8,%xmm15
  .byte  69,15,82,232                        // rsqrtps       %xmm8,%xmm13
  .byte  184,41,92,71,65                     // mov           $0x41475c29,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,40,240                        // movaps        %xmm8,%xmm14
  .byte  68,15,89,240                        // mulps         %xmm0,%xmm14
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,200                   // movd          %eax,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  184,194,135,210,62                  // mov           $0x3ed287c2,%eax
  .byte  102,68,15,110,208                   // movd          %eax,%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  184,206,111,48,63                   // mov           $0x3f306fce,%eax
  .byte  102,68,15,110,216                   // movd          %eax,%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  184,168,87,202,61                   // mov           $0x3dca57a8,%eax
  .byte  53,0,0,0,128                        // xor           $0x80000000,%eax
  .byte  102,68,15,110,224                   // movd          %eax,%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  69,15,89,251                        // mulps         %xmm11,%xmm15
  .byte  69,15,88,252                        // addps         %xmm12,%xmm15
  .byte  69,15,89,234                        // mulps         %xmm10,%xmm13
  .byte  69,15,88,239                        // addps         %xmm15,%xmm13
  .byte  69,15,40,249                        // movaps        %xmm9,%xmm15
  .byte  69,15,93,253                        // minps         %xmm13,%xmm15
  .byte  184,4,231,140,59                    // mov           $0x3b8ce704,%eax
  .byte  102,68,15,110,232                   // movd          %eax,%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  65,15,194,197,1                     // cmpltps       %xmm13,%xmm0
  .byte  68,15,84,240                        // andps         %xmm0,%xmm14
  .byte  65,15,85,199                        // andnps        %xmm15,%xmm0
  .byte  65,15,86,198                        // orps          %xmm14,%xmm0
  .byte  68,15,82,241                        // rsqrtps       %xmm1,%xmm14
  .byte  69,15,83,254                        // rcpps         %xmm14,%xmm15
  .byte  69,15,82,246                        // rsqrtps       %xmm14,%xmm14
  .byte  69,15,89,251                        // mulps         %xmm11,%xmm15
  .byte  69,15,88,252                        // addps         %xmm12,%xmm15
  .byte  69,15,89,242                        // mulps         %xmm10,%xmm14
  .byte  69,15,88,247                        // addps         %xmm15,%xmm14
  .byte  69,15,40,249                        // movaps        %xmm9,%xmm15
  .byte  69,15,93,254                        // minps         %xmm14,%xmm15
  .byte  69,15,40,240                        // movaps        %xmm8,%xmm14
  .byte  68,15,89,241                        // mulps         %xmm1,%xmm14
  .byte  65,15,194,205,1                     // cmpltps       %xmm13,%xmm1
  .byte  68,15,84,241                        // andps         %xmm1,%xmm14
  .byte  65,15,85,207                        // andnps        %xmm15,%xmm1
  .byte  65,15,86,206                        // orps          %xmm14,%xmm1
  .byte  68,15,82,242                        // rsqrtps       %xmm2,%xmm14
  .byte  69,15,83,254                        // rcpps         %xmm14,%xmm15
  .byte  69,15,89,251                        // mulps         %xmm11,%xmm15
  .byte  69,15,88,252                        // addps         %xmm12,%xmm15
  .byte  69,15,82,222                        // rsqrtps       %xmm14,%xmm11
  .byte  69,15,89,218                        // mulps         %xmm10,%xmm11
  .byte  69,15,88,223                        // addps         %xmm15,%xmm11
  .byte  69,15,93,203                        // minps         %xmm11,%xmm9
  .byte  68,15,89,194                        // mulps         %xmm2,%xmm8
  .byte  65,15,194,213,1                     // cmpltps       %xmm13,%xmm2
  .byte  68,15,84,194                        // andps         %xmm2,%xmm8
  .byte  65,15,85,209                        // andnps        %xmm9,%xmm2
  .byte  65,15,86,208                        // orps          %xmm8,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_scale_1_float_sse2
.globl _sk_scale_1_float_sse2
_sk_scale_1_float_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,0                      // movss         (%rax),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_scale_u8_sse2
.globl _sk_scale_u8_sse2
_sk_scale_u8_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,68,15,110,4,56                  // movd          (%rax,%rdi,1),%xmm8
  .byte  102,69,15,239,201                   // pxor          %xmm9,%xmm9
  .byte  102,69,15,96,193                    // punpcklbw     %xmm9,%xmm8
  .byte  102,69,15,97,193                    // punpcklwd     %xmm9,%xmm8
  .byte  69,15,91,192                        // cvtdq2ps      %xmm8,%xmm8
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  102,68,15,110,200                   // movd          %eax,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  65,15,89,193                        // mulps         %xmm9,%xmm0
  .byte  65,15,89,201                        // mulps         %xmm9,%xmm1
  .byte  65,15,89,209                        // mulps         %xmm9,%xmm2
  .byte  65,15,89,217                        // mulps         %xmm9,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_lerp_1_float_sse2
.globl _sk_lerp_1_float_sse2
_sk_lerp_1_float_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,0                      // movss         (%rax),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  15,92,196                           // subps         %xmm4,%xmm0
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  15,92,205                           // subps         %xmm5,%xmm1
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  15,92,214                           // subps         %xmm6,%xmm2
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  15,88,214                           // addps         %xmm6,%xmm2
  .byte  15,92,223                           // subps         %xmm7,%xmm3
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  15,88,223                           // addps         %xmm7,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_lerp_u8_sse2
.globl _sk_lerp_u8_sse2
_sk_lerp_u8_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,68,15,110,4,56                  // movd          (%rax,%rdi,1),%xmm8
  .byte  102,69,15,239,201                   // pxor          %xmm9,%xmm9
  .byte  102,69,15,96,193                    // punpcklbw     %xmm9,%xmm8
  .byte  102,69,15,97,193                    // punpcklwd     %xmm9,%xmm8
  .byte  69,15,91,192                        // cvtdq2ps      %xmm8,%xmm8
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  102,68,15,110,200                   // movd          %eax,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  15,92,196                           // subps         %xmm4,%xmm0
  .byte  65,15,89,193                        // mulps         %xmm9,%xmm0
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  15,92,205                           // subps         %xmm5,%xmm1
  .byte  65,15,89,201                        // mulps         %xmm9,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  15,92,214                           // subps         %xmm6,%xmm2
  .byte  65,15,89,209                        // mulps         %xmm9,%xmm2
  .byte  15,88,214                           // addps         %xmm6,%xmm2
  .byte  15,92,223                           // subps         %xmm7,%xmm3
  .byte  65,15,89,217                        // mulps         %xmm9,%xmm3
  .byte  15,88,223                           // addps         %xmm7,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_lerp_565_sse2
.globl _sk_lerp_565_sse2
_sk_lerp_565_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  243,68,15,126,4,120                 // movq          (%rax,%rdi,2),%xmm8
  .byte  102,15,239,219                      // pxor          %xmm3,%xmm3
  .byte  102,68,15,97,195                    // punpcklwd     %xmm3,%xmm8
  .byte  184,0,248,0,0                       // mov           $0xf800,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  102,15,112,219,0                    // pshufd        $0x0,%xmm3,%xmm3
  .byte  102,65,15,219,216                   // pand          %xmm8,%xmm3
  .byte  68,15,91,203                        // cvtdq2ps      %xmm3,%xmm9
  .byte  184,8,33,132,55                     // mov           $0x37842108,%eax
  .byte  102,68,15,110,208                   // movd          %eax,%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  69,15,89,209                        // mulps         %xmm9,%xmm10
  .byte  184,224,7,0,0                       // mov           $0x7e0,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  102,15,112,219,0                    // pshufd        $0x0,%xmm3,%xmm3
  .byte  102,65,15,219,216                   // pand          %xmm8,%xmm3
  .byte  68,15,91,203                        // cvtdq2ps      %xmm3,%xmm9
  .byte  184,33,8,2,58                       // mov           $0x3a020821,%eax
  .byte  102,68,15,110,216                   // movd          %eax,%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  69,15,89,217                        // mulps         %xmm9,%xmm11
  .byte  184,31,0,0,0                        // mov           $0x1f,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  102,15,112,219,0                    // pshufd        $0x0,%xmm3,%xmm3
  .byte  102,65,15,219,216                   // pand          %xmm8,%xmm3
  .byte  68,15,91,195                        // cvtdq2ps      %xmm3,%xmm8
  .byte  184,8,33,4,61                       // mov           $0x3d042108,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  15,92,196                           // subps         %xmm4,%xmm0
  .byte  65,15,89,194                        // mulps         %xmm10,%xmm0
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  15,92,205                           // subps         %xmm5,%xmm1
  .byte  65,15,89,203                        // mulps         %xmm11,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  15,92,214                           // subps         %xmm6,%xmm2
  .byte  15,89,211                           // mulps         %xmm3,%xmm2
  .byte  15,88,214                           // addps         %xmm6,%xmm2
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_load_tables_sse2
.globl _sk_load_tables_sse2
_sk_load_tables_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,8                            // mov           (%rax),%rcx
  .byte  76,139,64,8                         // mov           0x8(%rax),%r8
  .byte  243,68,15,111,4,185                 // movdqu        (%rcx,%rdi,4),%xmm8
  .byte  185,255,0,0,0                       // mov           $0xff,%ecx
  .byte  102,15,110,193                      // movd          %ecx,%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  102,69,15,111,200                   // movdqa        %xmm8,%xmm9
  .byte  102,65,15,114,209,8                 // psrld         $0x8,%xmm9
  .byte  102,68,15,219,200                   // pand          %xmm0,%xmm9
  .byte  102,69,15,111,208                   // movdqa        %xmm8,%xmm10
  .byte  102,65,15,114,210,16                // psrld         $0x10,%xmm10
  .byte  102,68,15,219,208                   // pand          %xmm0,%xmm10
  .byte  102,65,15,219,192                   // pand          %xmm8,%xmm0
  .byte  102,15,112,216,78                   // pshufd        $0x4e,%xmm0,%xmm3
  .byte  102,72,15,126,217                   // movq          %xmm3,%rcx
  .byte  65,137,201                          // mov           %ecx,%r9d
  .byte  72,193,233,32                       // shr           $0x20,%rcx
  .byte  102,73,15,126,194                   // movq          %xmm0,%r10
  .byte  69,137,211                          // mov           %r10d,%r11d
  .byte  73,193,234,32                       // shr           $0x20,%r10
  .byte  243,67,15,16,28,144                 // movss         (%r8,%r10,4),%xmm3
  .byte  243,65,15,16,4,136                  // movss         (%r8,%rcx,4),%xmm0
  .byte  15,20,216                           // unpcklps      %xmm0,%xmm3
  .byte  243,67,15,16,4,152                  // movss         (%r8,%r11,4),%xmm0
  .byte  243,67,15,16,12,136                 // movss         (%r8,%r9,4),%xmm1
  .byte  15,20,193                           // unpcklps      %xmm1,%xmm0
  .byte  15,20,195                           // unpcklps      %xmm3,%xmm0
  .byte  76,139,64,16                        // mov           0x10(%rax),%r8
  .byte  102,65,15,112,201,78                // pshufd        $0x4e,%xmm9,%xmm1
  .byte  102,73,15,126,202                   // movq          %xmm1,%r10
  .byte  77,137,209                          // mov           %r10,%r9
  .byte  73,193,233,32                       // shr           $0x20,%r9
  .byte  102,76,15,126,201                   // movq          %xmm9,%rcx
  .byte  65,137,203                          // mov           %ecx,%r11d
  .byte  65,129,227,255,255,255,0            // and           $0xffffff,%r11d
  .byte  72,193,233,30                       // shr           $0x1e,%rcx
  .byte  65,129,226,255,255,255,0            // and           $0xffffff,%r10d
  .byte  243,65,15,16,28,8                   // movss         (%r8,%rcx,1),%xmm3
  .byte  243,67,15,16,12,136                 // movss         (%r8,%r9,4),%xmm1
  .byte  15,20,217                           // unpcklps      %xmm1,%xmm3
  .byte  243,67,15,16,12,152                 // movss         (%r8,%r11,4),%xmm1
  .byte  243,67,15,16,20,144                 // movss         (%r8,%r10,4),%xmm2
  .byte  15,20,202                           // unpcklps      %xmm2,%xmm1
  .byte  15,20,203                           // unpcklps      %xmm3,%xmm1
  .byte  76,139,72,24                        // mov           0x18(%rax),%r9
  .byte  102,65,15,112,210,78                // pshufd        $0x4e,%xmm10,%xmm2
  .byte  102,72,15,126,209                   // movq          %xmm2,%rcx
  .byte  68,15,183,193                       // movzwl        %cx,%r8d
  .byte  72,193,233,32                       // shr           $0x20,%rcx
  .byte  102,76,15,126,208                   // movq          %xmm10,%rax
  .byte  68,15,183,208                       // movzwl        %ax,%r10d
  .byte  72,193,232,30                       // shr           $0x1e,%rax
  .byte  243,69,15,16,12,1                   // movss         (%r9,%rax,1),%xmm9
  .byte  243,65,15,16,20,137                 // movss         (%r9,%rcx,4),%xmm2
  .byte  68,15,20,202                        // unpcklps      %xmm2,%xmm9
  .byte  243,67,15,16,20,145                 // movss         (%r9,%r10,4),%xmm2
  .byte  243,67,15,16,28,129                 // movss         (%r9,%r8,4),%xmm3
  .byte  15,20,211                           // unpcklps      %xmm3,%xmm2
  .byte  65,15,20,209                        // unpcklps      %xmm9,%xmm2
  .byte  102,65,15,114,208,24                // psrld         $0x18,%xmm8
  .byte  69,15,91,192                        // cvtdq2ps      %xmm8,%xmm8
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_load_a8_sse2
.globl _sk_load_a8_sse2
_sk_load_a8_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,15,110,4,56                     // movd          (%rax,%rdi,1),%xmm0
  .byte  102,15,239,201                      // pxor          %xmm1,%xmm1
  .byte  102,15,96,193                       // punpcklbw     %xmm1,%xmm0
  .byte  102,15,97,193                       // punpcklwd     %xmm1,%xmm0
  .byte  15,91,192                           // cvtdq2ps      %xmm0,%xmm0
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  15,89,216                           // mulps         %xmm0,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  102,15,239,201                      // pxor          %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_store_a8_sse2
.globl _sk_store_a8_sse2
_sk_store_a8_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  185,0,0,127,67                      // mov           $0x437f0000,%ecx
  .byte  102,68,15,110,193                   // movd          %ecx,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,89,195                        // mulps         %xmm3,%xmm8
  .byte  102,69,15,91,192                    // cvtps2dq      %xmm8,%xmm8
  .byte  102,65,15,114,240,16                // pslld         $0x10,%xmm8
  .byte  102,65,15,114,224,16                // psrad         $0x10,%xmm8
  .byte  102,69,15,107,192                   // packssdw      %xmm8,%xmm8
  .byte  102,69,15,103,192                   // packuswb      %xmm8,%xmm8
  .byte  102,68,15,126,4,56                  // movd          %xmm8,(%rax,%rdi,1)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_load_g8_sse2
.globl _sk_load_g8_sse2
_sk_load_g8_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,15,110,4,56                     // movd          (%rax,%rdi,1),%xmm0
  .byte  102,15,239,201                      // pxor          %xmm1,%xmm1
  .byte  102,15,96,193                       // punpcklbw     %xmm1,%xmm0
  .byte  102,15,97,193                       // punpcklwd     %xmm1,%xmm0
  .byte  15,91,200                           // cvtdq2ps      %xmm0,%xmm1
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  15,89,193                           // mulps         %xmm1,%xmm0
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,200                           // movaps        %xmm0,%xmm1
  .byte  15,40,208                           // movaps        %xmm0,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_load_565_sse2
.globl _sk_load_565_sse2
_sk_load_565_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  243,15,126,20,120                   // movq          (%rax,%rdi,2),%xmm2
  .byte  102,15,239,192                      // pxor          %xmm0,%xmm0
  .byte  102,15,97,208                       // punpcklwd     %xmm0,%xmm2
  .byte  184,0,248,0,0                       // mov           $0xf800,%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  102,15,219,194                      // pand          %xmm2,%xmm0
  .byte  15,91,200                           // cvtdq2ps      %xmm0,%xmm1
  .byte  184,8,33,132,55                     // mov           $0x37842108,%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  15,89,193                           // mulps         %xmm1,%xmm0
  .byte  184,224,7,0,0                       // mov           $0x7e0,%eax
  .byte  102,15,110,200                      // movd          %eax,%xmm1
  .byte  102,15,112,201,0                    // pshufd        $0x0,%xmm1,%xmm1
  .byte  102,15,219,202                      // pand          %xmm2,%xmm1
  .byte  15,91,217                           // cvtdq2ps      %xmm1,%xmm3
  .byte  184,33,8,2,58                       // mov           $0x3a020821,%eax
  .byte  102,15,110,200                      // movd          %eax,%xmm1
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  15,89,203                           // mulps         %xmm3,%xmm1
  .byte  184,31,0,0,0                        // mov           $0x1f,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  102,15,112,219,0                    // pshufd        $0x0,%xmm3,%xmm3
  .byte  102,15,219,218                      // pand          %xmm2,%xmm3
  .byte  15,91,219                           // cvtdq2ps      %xmm3,%xmm3
  .byte  184,8,33,4,61                       // mov           $0x3d042108,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  15,89,211                           // mulps         %xmm3,%xmm2
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_store_565_sse2
.globl _sk_store_565_sse2
_sk_store_565_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  185,0,0,248,65                      // mov           $0x41f80000,%ecx
  .byte  102,68,15,110,193                   // movd          %ecx,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,200                        // mulps         %xmm0,%xmm9
  .byte  102,69,15,91,201                    // cvtps2dq      %xmm9,%xmm9
  .byte  102,65,15,114,241,11                // pslld         $0xb,%xmm9
  .byte  185,0,0,124,66                      // mov           $0x427c0000,%ecx
  .byte  102,68,15,110,209                   // movd          %ecx,%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  68,15,89,209                        // mulps         %xmm1,%xmm10
  .byte  102,69,15,91,210                    // cvtps2dq      %xmm10,%xmm10
  .byte  102,65,15,114,242,5                 // pslld         $0x5,%xmm10
  .byte  102,69,15,235,209                   // por           %xmm9,%xmm10
  .byte  68,15,89,194                        // mulps         %xmm2,%xmm8
  .byte  102,69,15,91,192                    // cvtps2dq      %xmm8,%xmm8
  .byte  102,69,15,86,194                    // orpd          %xmm10,%xmm8
  .byte  102,65,15,114,240,16                // pslld         $0x10,%xmm8
  .byte  102,65,15,114,224,16                // psrad         $0x10,%xmm8
  .byte  102,69,15,107,192                   // packssdw      %xmm8,%xmm8
  .byte  102,68,15,214,4,120                 // movq          %xmm8,(%rax,%rdi,2)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_load_4444_sse2
.globl _sk_load_4444_sse2
_sk_load_4444_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  243,68,15,126,12,120                // movq          (%rax,%rdi,2),%xmm9
  .byte  102,15,239,192                      // pxor          %xmm0,%xmm0
  .byte  102,68,15,97,200                    // punpcklwd     %xmm0,%xmm9
  .byte  184,0,240,0,0                       // mov           $0xf000,%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  102,65,15,219,193                   // pand          %xmm9,%xmm0
  .byte  15,91,200                           // cvtdq2ps      %xmm0,%xmm1
  .byte  184,137,136,136,55                  // mov           $0x37888889,%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  15,89,193                           // mulps         %xmm1,%xmm0
  .byte  184,0,15,0,0                        // mov           $0xf00,%eax
  .byte  102,15,110,200                      // movd          %eax,%xmm1
  .byte  102,15,112,201,0                    // pshufd        $0x0,%xmm1,%xmm1
  .byte  102,65,15,219,201                   // pand          %xmm9,%xmm1
  .byte  15,91,209                           // cvtdq2ps      %xmm1,%xmm2
  .byte  184,137,136,136,57                  // mov           $0x39888889,%eax
  .byte  102,15,110,200                      // movd          %eax,%xmm1
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  15,89,202                           // mulps         %xmm2,%xmm1
  .byte  184,240,0,0,0                       // mov           $0xf0,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  102,15,112,210,0                    // pshufd        $0x0,%xmm2,%xmm2
  .byte  102,65,15,219,209                   // pand          %xmm9,%xmm2
  .byte  68,15,91,194                        // cvtdq2ps      %xmm2,%xmm8
  .byte  184,137,136,136,59                  // mov           $0x3b888889,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  184,15,0,0,0                        // mov           $0xf,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  102,15,112,219,0                    // pshufd        $0x0,%xmm3,%xmm3
  .byte  102,65,15,219,217                   // pand          %xmm9,%xmm3
  .byte  68,15,91,195                        // cvtdq2ps      %xmm3,%xmm8
  .byte  184,137,136,136,61                  // mov           $0x3d888889,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_store_4444_sse2
.globl _sk_store_4444_sse2
_sk_store_4444_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  185,0,0,112,65                      // mov           $0x41700000,%ecx
  .byte  102,68,15,110,193                   // movd          %ecx,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,200                        // mulps         %xmm0,%xmm9
  .byte  102,69,15,91,201                    // cvtps2dq      %xmm9,%xmm9
  .byte  102,65,15,114,241,12                // pslld         $0xc,%xmm9
  .byte  69,15,40,208                        // movaps        %xmm8,%xmm10
  .byte  68,15,89,209                        // mulps         %xmm1,%xmm10
  .byte  102,69,15,91,210                    // cvtps2dq      %xmm10,%xmm10
  .byte  102,65,15,114,242,8                 // pslld         $0x8,%xmm10
  .byte  102,69,15,235,209                   // por           %xmm9,%xmm10
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,202                        // mulps         %xmm2,%xmm9
  .byte  102,69,15,91,201                    // cvtps2dq      %xmm9,%xmm9
  .byte  102,65,15,114,241,4                 // pslld         $0x4,%xmm9
  .byte  68,15,89,195                        // mulps         %xmm3,%xmm8
  .byte  102,69,15,91,192                    // cvtps2dq      %xmm8,%xmm8
  .byte  102,69,15,86,193                    // orpd          %xmm9,%xmm8
  .byte  102,69,15,86,194                    // orpd          %xmm10,%xmm8
  .byte  102,65,15,114,240,16                // pslld         $0x10,%xmm8
  .byte  102,65,15,114,224,16                // psrad         $0x10,%xmm8
  .byte  102,69,15,107,192                   // packssdw      %xmm8,%xmm8
  .byte  102,68,15,214,4,120                 // movq          %xmm8,(%rax,%rdi,2)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_load_8888_sse2
.globl _sk_load_8888_sse2
_sk_load_8888_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  243,15,111,28,184                   // movdqu        (%rax,%rdi,4),%xmm3
  .byte  184,255,0,0,0                       // mov           $0xff,%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  102,15,111,203                      // movdqa        %xmm3,%xmm1
  .byte  102,15,114,209,8                    // psrld         $0x8,%xmm1
  .byte  102,15,219,200                      // pand          %xmm0,%xmm1
  .byte  102,15,111,211                      // movdqa        %xmm3,%xmm2
  .byte  102,15,114,210,16                   // psrld         $0x10,%xmm2
  .byte  102,15,219,208                      // pand          %xmm0,%xmm2
  .byte  102,15,219,195                      // pand          %xmm3,%xmm0
  .byte  15,91,192                           // cvtdq2ps      %xmm0,%xmm0
  .byte  184,129,128,128,59                  // mov           $0x3b808081,%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  15,91,201                           // cvtdq2ps      %xmm1,%xmm1
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  15,91,210                           // cvtdq2ps      %xmm2,%xmm2
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  102,15,114,211,24                   // psrld         $0x18,%xmm3
  .byte  15,91,219                           // cvtdq2ps      %xmm3,%xmm3
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_store_8888_sse2
.globl _sk_store_8888_sse2
_sk_store_8888_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  185,0,0,127,67                      // mov           $0x437f0000,%ecx
  .byte  102,68,15,110,193                   // movd          %ecx,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,200                        // mulps         %xmm0,%xmm9
  .byte  102,69,15,91,201                    // cvtps2dq      %xmm9,%xmm9
  .byte  69,15,40,208                        // movaps        %xmm8,%xmm10
  .byte  68,15,89,209                        // mulps         %xmm1,%xmm10
  .byte  102,69,15,91,210                    // cvtps2dq      %xmm10,%xmm10
  .byte  102,65,15,114,242,8                 // pslld         $0x8,%xmm10
  .byte  102,69,15,235,209                   // por           %xmm9,%xmm10
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,202                        // mulps         %xmm2,%xmm9
  .byte  102,69,15,91,201                    // cvtps2dq      %xmm9,%xmm9
  .byte  102,65,15,114,241,16                // pslld         $0x10,%xmm9
  .byte  68,15,89,195                        // mulps         %xmm3,%xmm8
  .byte  102,69,15,91,192                    // cvtps2dq      %xmm8,%xmm8
  .byte  102,65,15,114,240,24                // pslld         $0x18,%xmm8
  .byte  102,69,15,235,193                   // por           %xmm9,%xmm8
  .byte  102,69,15,235,194                   // por           %xmm10,%xmm8
  .byte  243,68,15,127,4,184                 // movdqu        %xmm8,(%rax,%rdi,4)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_load_f16_sse2
.globl _sk_load_f16_sse2
_sk_load_f16_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  243,15,111,4,248                    // movdqu        (%rax,%rdi,8),%xmm0
  .byte  243,15,111,76,248,16                // movdqu        0x10(%rax,%rdi,8),%xmm1
  .byte  102,68,15,111,192                   // movdqa        %xmm0,%xmm8
  .byte  102,68,15,97,193                    // punpcklwd     %xmm1,%xmm8
  .byte  102,15,105,193                      // punpckhwd     %xmm1,%xmm0
  .byte  102,65,15,111,200                   // movdqa        %xmm8,%xmm1
  .byte  102,15,97,200                       // punpcklwd     %xmm0,%xmm1
  .byte  102,68,15,105,192                   // punpckhwd     %xmm0,%xmm8
  .byte  184,0,4,0,4                         // mov           $0x4000400,%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  102,15,112,216,0                    // pshufd        $0x0,%xmm0,%xmm3
  .byte  102,15,111,195                      // movdqa        %xmm3,%xmm0
  .byte  102,15,101,193                      // pcmpgtw       %xmm1,%xmm0
  .byte  102,15,223,193                      // pandn         %xmm1,%xmm0
  .byte  102,69,15,239,201                   // pxor          %xmm9,%xmm9
  .byte  102,65,15,97,193                    // punpcklwd     %xmm9,%xmm0
  .byte  102,15,114,240,13                   // pslld         $0xd,%xmm0
  .byte  184,0,0,128,119                     // mov           $0x77800000,%eax
  .byte  102,15,110,208                      // movd          %eax,%xmm2
  .byte  102,68,15,112,210,0                 // pshufd        $0x0,%xmm2,%xmm10
  .byte  65,15,89,194                        // mulps         %xmm10,%xmm0
  .byte  102,15,112,209,78                   // pshufd        $0x4e,%xmm1,%xmm2
  .byte  102,15,111,203                      // movdqa        %xmm3,%xmm1
  .byte  102,15,101,202                      // pcmpgtw       %xmm2,%xmm1
  .byte  102,15,223,202                      // pandn         %xmm2,%xmm1
  .byte  102,65,15,97,201                    // punpcklwd     %xmm9,%xmm1
  .byte  102,15,114,241,13                   // pslld         $0xd,%xmm1
  .byte  65,15,89,202                        // mulps         %xmm10,%xmm1
  .byte  102,15,111,211                      // movdqa        %xmm3,%xmm2
  .byte  102,65,15,101,208                   // pcmpgtw       %xmm8,%xmm2
  .byte  102,65,15,223,208                   // pandn         %xmm8,%xmm2
  .byte  102,65,15,97,209                    // punpcklwd     %xmm9,%xmm2
  .byte  102,15,114,242,13                   // pslld         $0xd,%xmm2
  .byte  65,15,89,210                        // mulps         %xmm10,%xmm2
  .byte  102,69,15,112,192,78                // pshufd        $0x4e,%xmm8,%xmm8
  .byte  102,65,15,101,216                   // pcmpgtw       %xmm8,%xmm3
  .byte  102,65,15,223,216                   // pandn         %xmm8,%xmm3
  .byte  102,65,15,97,217                    // punpcklwd     %xmm9,%xmm3
  .byte  102,15,114,243,13                   // pslld         $0xd,%xmm3
  .byte  65,15,89,218                        // mulps         %xmm10,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_store_f16_sse2
.globl _sk_store_f16_sse2
_sk_store_f16_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  185,0,0,128,7                       // mov           $0x7800000,%ecx
  .byte  102,68,15,110,193                   // movd          %ecx,%xmm8
  .byte  102,69,15,112,192,0                 // pshufd        $0x0,%xmm8,%xmm8
  .byte  102,69,15,111,200                   // movdqa        %xmm8,%xmm9
  .byte  68,15,89,200                        // mulps         %xmm0,%xmm9
  .byte  102,65,15,114,209,13                // psrld         $0xd,%xmm9
  .byte  102,69,15,111,208                   // movdqa        %xmm8,%xmm10
  .byte  68,15,89,209                        // mulps         %xmm1,%xmm10
  .byte  102,65,15,114,210,13                // psrld         $0xd,%xmm10
  .byte  102,69,15,111,216                   // movdqa        %xmm8,%xmm11
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  102,65,15,114,211,13                // psrld         $0xd,%xmm11
  .byte  68,15,89,195                        // mulps         %xmm3,%xmm8
  .byte  102,65,15,114,208,13                // psrld         $0xd,%xmm8
  .byte  102,65,15,115,250,2                 // pslldq        $0x2,%xmm10
  .byte  102,69,15,235,209                   // por           %xmm9,%xmm10
  .byte  102,65,15,115,248,2                 // pslldq        $0x2,%xmm8
  .byte  102,69,15,235,195                   // por           %xmm11,%xmm8
  .byte  102,69,15,111,202                   // movdqa        %xmm10,%xmm9
  .byte  102,69,15,98,200                    // punpckldq     %xmm8,%xmm9
  .byte  243,68,15,127,12,248                // movdqu        %xmm9,(%rax,%rdi,8)
  .byte  102,69,15,106,208                   // punpckhdq     %xmm8,%xmm10
  .byte  243,68,15,127,84,248,16             // movdqu        %xmm10,0x10(%rax,%rdi,8)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_store_f32_sse2
.globl _sk_store_f32_sse2
_sk_store_f32_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,137,249                          // mov           %rdi,%rcx
  .byte  72,193,225,4                        // shl           $0x4,%rcx
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  68,15,20,201                        // unpcklps      %xmm1,%xmm9
  .byte  68,15,40,210                        // movaps        %xmm2,%xmm10
  .byte  68,15,40,218                        // movaps        %xmm2,%xmm11
  .byte  68,15,20,219                        // unpcklps      %xmm3,%xmm11
  .byte  68,15,21,193                        // unpckhps      %xmm1,%xmm8
  .byte  68,15,21,211                        // unpckhps      %xmm3,%xmm10
  .byte  69,15,40,225                        // movaps        %xmm9,%xmm12
  .byte  102,69,15,20,227                    // unpcklpd      %xmm11,%xmm12
  .byte  69,15,18,217                        // movhlps       %xmm9,%xmm11
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  102,69,15,20,202                    // unpcklpd      %xmm10,%xmm9
  .byte  69,15,18,208                        // movhlps       %xmm8,%xmm10
  .byte  102,68,15,17,36,8                   // movupd        %xmm12,(%rax,%rcx,1)
  .byte  68,15,17,92,8,16                    // movups        %xmm11,0x10(%rax,%rcx,1)
  .byte  102,68,15,17,76,8,32                // movupd        %xmm9,0x20(%rax,%rcx,1)
  .byte  68,15,17,84,8,48                    // movups        %xmm10,0x30(%rax,%rcx,1)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_x_sse2
.globl _sk_clamp_x_sse2
_sk_clamp_x_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  69,15,87,192                        // xorps         %xmm8,%xmm8
  .byte  68,15,95,192                        // maxps         %xmm0,%xmm8
  .byte  243,68,15,16,8                      // movss         (%rax),%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  102,15,118,192                      // pcmpeqd       %xmm0,%xmm0
  .byte  102,65,15,254,193                   // paddd         %xmm9,%xmm0
  .byte  68,15,93,192                        // minps         %xmm0,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_clamp_y_sse2
.globl _sk_clamp_y_sse2
_sk_clamp_y_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  69,15,87,192                        // xorps         %xmm8,%xmm8
  .byte  68,15,95,193                        // maxps         %xmm1,%xmm8
  .byte  243,68,15,16,8                      // movss         (%rax),%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  102,15,118,201                      // pcmpeqd       %xmm1,%xmm1
  .byte  102,65,15,254,201                   // paddd         %xmm9,%xmm1
  .byte  68,15,93,193                        // minps         %xmm1,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,200                        // movaps        %xmm8,%xmm1
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_repeat_x_sse2
.globl _sk_repeat_x_sse2
_sk_repeat_x_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,0                      // movss         (%rax),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  69,15,94,200                        // divps         %xmm8,%xmm9
  .byte  243,69,15,91,209                    // cvttps2dq     %xmm9,%xmm10
  .byte  69,15,91,210                        // cvtdq2ps      %xmm10,%xmm10
  .byte  69,15,194,202,1                     // cmpltps       %xmm10,%xmm9
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,216                   // movd          %eax,%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  69,15,84,217                        // andps         %xmm9,%xmm11
  .byte  69,15,92,211                        // subps         %xmm11,%xmm10
  .byte  69,15,89,208                        // mulps         %xmm8,%xmm10
  .byte  65,15,92,194                        // subps         %xmm10,%xmm0
  .byte  102,69,15,118,201                   // pcmpeqd       %xmm9,%xmm9
  .byte  102,69,15,254,200                   // paddd         %xmm8,%xmm9
  .byte  65,15,93,193                        // minps         %xmm9,%xmm0
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_repeat_y_sse2
.globl _sk_repeat_y_sse2
_sk_repeat_y_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,0                      // movss         (%rax),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,40,201                        // movaps        %xmm1,%xmm9
  .byte  69,15,94,200                        // divps         %xmm8,%xmm9
  .byte  243,69,15,91,209                    // cvttps2dq     %xmm9,%xmm10
  .byte  69,15,91,210                        // cvtdq2ps      %xmm10,%xmm10
  .byte  69,15,194,202,1                     // cmpltps       %xmm10,%xmm9
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,216                   // movd          %eax,%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  69,15,84,217                        // andps         %xmm9,%xmm11
  .byte  69,15,92,211                        // subps         %xmm11,%xmm10
  .byte  69,15,89,208                        // mulps         %xmm8,%xmm10
  .byte  65,15,92,202                        // subps         %xmm10,%xmm1
  .byte  102,69,15,118,201                   // pcmpeqd       %xmm9,%xmm9
  .byte  102,69,15,254,200                   // paddd         %xmm8,%xmm9
  .byte  65,15,93,201                        // minps         %xmm9,%xmm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_mirror_x_sse2
.globl _sk_mirror_x_sse2
_sk_mirror_x_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,8                      // movss         (%rax),%xmm9
  .byte  69,15,40,193                        // movaps        %xmm9,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  65,15,92,192                        // subps         %xmm8,%xmm0
  .byte  243,69,15,88,201                    // addss         %xmm9,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  68,15,40,208                        // movaps        %xmm0,%xmm10
  .byte  69,15,94,209                        // divps         %xmm9,%xmm10
  .byte  243,69,15,91,218                    // cvttps2dq     %xmm10,%xmm11
  .byte  69,15,91,219                        // cvtdq2ps      %xmm11,%xmm11
  .byte  69,15,194,211,1                     // cmpltps       %xmm11,%xmm10
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,224                   // movd          %eax,%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  69,15,84,226                        // andps         %xmm10,%xmm12
  .byte  69,15,87,210                        // xorps         %xmm10,%xmm10
  .byte  69,15,92,220                        // subps         %xmm12,%xmm11
  .byte  69,15,89,217                        // mulps         %xmm9,%xmm11
  .byte  65,15,92,195                        // subps         %xmm11,%xmm0
  .byte  65,15,92,192                        // subps         %xmm8,%xmm0
  .byte  68,15,92,208                        // subps         %xmm0,%xmm10
  .byte  65,15,84,194                        // andps         %xmm10,%xmm0
  .byte  102,69,15,118,201                   // pcmpeqd       %xmm9,%xmm9
  .byte  102,69,15,254,200                   // paddd         %xmm8,%xmm9
  .byte  65,15,93,193                        // minps         %xmm9,%xmm0
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_mirror_y_sse2
.globl _sk_mirror_y_sse2
_sk_mirror_y_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,8                      // movss         (%rax),%xmm9
  .byte  69,15,40,193                        // movaps        %xmm9,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  65,15,92,200                        // subps         %xmm8,%xmm1
  .byte  243,69,15,88,201                    // addss         %xmm9,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  68,15,40,209                        // movaps        %xmm1,%xmm10
  .byte  69,15,94,209                        // divps         %xmm9,%xmm10
  .byte  243,69,15,91,218                    // cvttps2dq     %xmm10,%xmm11
  .byte  69,15,91,219                        // cvtdq2ps      %xmm11,%xmm11
  .byte  69,15,194,211,1                     // cmpltps       %xmm11,%xmm10
  .byte  184,0,0,128,63                      // mov           $0x3f800000,%eax
  .byte  102,68,15,110,224                   // movd          %eax,%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  69,15,84,226                        // andps         %xmm10,%xmm12
  .byte  69,15,87,210                        // xorps         %xmm10,%xmm10
  .byte  69,15,92,220                        // subps         %xmm12,%xmm11
  .byte  69,15,89,217                        // mulps         %xmm9,%xmm11
  .byte  65,15,92,203                        // subps         %xmm11,%xmm1
  .byte  65,15,92,200                        // subps         %xmm8,%xmm1
  .byte  68,15,92,209                        // subps         %xmm1,%xmm10
  .byte  65,15,84,202                        // andps         %xmm10,%xmm1
  .byte  102,69,15,118,201                   // pcmpeqd       %xmm9,%xmm9
  .byte  102,69,15,254,200                   // paddd         %xmm8,%xmm9
  .byte  65,15,93,201                        // minps         %xmm9,%xmm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_luminance_to_alpha_sse2
.globl _sk_luminance_to_alpha_sse2
_sk_luminance_to_alpha_sse2:
  .byte  184,208,179,89,62                   // mov           $0x3e59b3d0,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  15,89,216                           // mulps         %xmm0,%xmm3
  .byte  184,89,23,55,63                     // mov           $0x3f371759,%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  15,89,193                           // mulps         %xmm1,%xmm0
  .byte  15,88,195                           // addps         %xmm3,%xmm0
  .byte  184,152,221,147,61                  // mov           $0x3d93dd98,%eax
  .byte  102,15,110,216                      // movd          %eax,%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  15,89,218                           // mulps         %xmm2,%xmm3
  .byte  15,88,216                           // addps         %xmm0,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,87,201                           // xorps         %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_matrix_2x3_sse2
.globl _sk_matrix_2x3_sse2
_sk_matrix_2x3_sse2:
  .byte  68,15,40,201                        // movaps        %xmm1,%xmm9
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,15,16,0                         // movss         (%rax),%xmm0
  .byte  243,15,16,72,4                      // movss         0x4(%rax),%xmm1
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  243,68,15,16,80,8                   // movss         0x8(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,16                  // movss         0x10(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  69,15,89,209                        // mulps         %xmm9,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,88,194                        // addps         %xmm10,%xmm0
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  243,68,15,16,80,12                  // movss         0xc(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,20                  // movss         0x14(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  69,15,89,209                        // mulps         %xmm9,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,88,202                        // addps         %xmm10,%xmm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_matrix_3x4_sse2
.globl _sk_matrix_3x4_sse2
_sk_matrix_3x4_sse2:
  .byte  68,15,40,201                        // movaps        %xmm1,%xmm9
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,15,16,0                         // movss         (%rax),%xmm0
  .byte  243,15,16,72,4                      // movss         0x4(%rax),%xmm1
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  243,68,15,16,80,12                  // movss         0xc(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,24                  // movss         0x18(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,96,36                  // movss         0x24(%rax),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  69,15,88,220                        // addps         %xmm12,%xmm11
  .byte  69,15,89,209                        // mulps         %xmm9,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,88,194                        // addps         %xmm10,%xmm0
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  243,68,15,16,80,16                  // movss         0x10(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,28                  // movss         0x1c(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,96,40                  // movss         0x28(%rax),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  69,15,88,220                        // addps         %xmm12,%xmm11
  .byte  69,15,89,209                        // mulps         %xmm9,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,88,202                        // addps         %xmm10,%xmm1
  .byte  243,68,15,16,80,8                   // movss         0x8(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,20                  // movss         0x14(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,96,32                  // movss         0x20(%rax),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  243,68,15,16,104,44                 // movss         0x2c(%rax),%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  68,15,89,226                        // mulps         %xmm2,%xmm12
  .byte  69,15,88,229                        // addps         %xmm13,%xmm12
  .byte  69,15,89,217                        // mulps         %xmm9,%xmm11
  .byte  69,15,88,220                        // addps         %xmm12,%xmm11
  .byte  69,15,89,208                        // mulps         %xmm8,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,210                        // movaps        %xmm10,%xmm2
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_matrix_4x5_sse2
.globl _sk_matrix_4x5_sse2
_sk_matrix_4x5_sse2:
  .byte  68,15,40,201                        // movaps        %xmm1,%xmm9
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,15,16,0                         // movss         (%rax),%xmm0
  .byte  243,15,16,72,4                      // movss         0x4(%rax),%xmm1
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  243,68,15,16,80,16                  // movss         0x10(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,32                  // movss         0x20(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,96,48                  // movss         0x30(%rax),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  243,68,15,16,104,64                 // movss         0x40(%rax),%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  68,15,89,227                        // mulps         %xmm3,%xmm12
  .byte  69,15,88,229                        // addps         %xmm13,%xmm12
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  69,15,88,220                        // addps         %xmm12,%xmm11
  .byte  69,15,89,209                        // mulps         %xmm9,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,88,194                        // addps         %xmm10,%xmm0
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  243,68,15,16,80,20                  // movss         0x14(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,36                  // movss         0x24(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,96,52                  // movss         0x34(%rax),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  243,68,15,16,104,68                 // movss         0x44(%rax),%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  68,15,89,227                        // mulps         %xmm3,%xmm12
  .byte  69,15,88,229                        // addps         %xmm13,%xmm12
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  69,15,88,220                        // addps         %xmm12,%xmm11
  .byte  69,15,89,209                        // mulps         %xmm9,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,88,202                        // addps         %xmm10,%xmm1
  .byte  243,68,15,16,80,8                   // movss         0x8(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,24                  // movss         0x18(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,96,40                  // movss         0x28(%rax),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  243,68,15,16,104,56                 // movss         0x38(%rax),%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  243,68,15,16,112,72                 // movss         0x48(%rax),%xmm14
  .byte  69,15,198,246,0                     // shufps        $0x0,%xmm14,%xmm14
  .byte  68,15,89,235                        // mulps         %xmm3,%xmm13
  .byte  69,15,88,238                        // addps         %xmm14,%xmm13
  .byte  68,15,89,226                        // mulps         %xmm2,%xmm12
  .byte  69,15,88,229                        // addps         %xmm13,%xmm12
  .byte  69,15,89,217                        // mulps         %xmm9,%xmm11
  .byte  69,15,88,220                        // addps         %xmm12,%xmm11
  .byte  69,15,89,208                        // mulps         %xmm8,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  243,68,15,16,88,12                  // movss         0xc(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,96,28                  // movss         0x1c(%rax),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  243,68,15,16,104,44                 // movss         0x2c(%rax),%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  243,68,15,16,112,60                 // movss         0x3c(%rax),%xmm14
  .byte  69,15,198,246,0                     // shufps        $0x0,%xmm14,%xmm14
  .byte  243,68,15,16,120,76                 // movss         0x4c(%rax),%xmm15
  .byte  69,15,198,255,0                     // shufps        $0x0,%xmm15,%xmm15
  .byte  68,15,89,243                        // mulps         %xmm3,%xmm14
  .byte  69,15,88,247                        // addps         %xmm15,%xmm14
  .byte  68,15,89,234                        // mulps         %xmm2,%xmm13
  .byte  69,15,88,238                        // addps         %xmm14,%xmm13
  .byte  69,15,89,225                        // mulps         %xmm9,%xmm12
  .byte  69,15,88,229                        // addps         %xmm13,%xmm12
  .byte  69,15,89,216                        // mulps         %xmm8,%xmm11
  .byte  69,15,88,220                        // addps         %xmm12,%xmm11
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,210                        // movaps        %xmm10,%xmm2
  .byte  65,15,40,219                        // movaps        %xmm11,%xmm3
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_matrix_perspective_sse2
.globl _sk_matrix_perspective_sse2
_sk_matrix_perspective_sse2:
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,15,16,0                         // movss         (%rax),%xmm0
  .byte  243,68,15,16,72,4                   // movss         0x4(%rax),%xmm9
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  243,68,15,16,80,8                   // movss         0x8(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  68,15,89,201                        // mulps         %xmm1,%xmm9
  .byte  69,15,88,202                        // addps         %xmm10,%xmm9
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,88,193                        // addps         %xmm9,%xmm0
  .byte  243,68,15,16,72,12                  // movss         0xc(%rax),%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  243,68,15,16,80,16                  // movss         0x10(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,20                  // movss         0x14(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  68,15,89,209                        // mulps         %xmm1,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  69,15,88,202                        // addps         %xmm10,%xmm9
  .byte  243,68,15,16,80,24                  // movss         0x18(%rax),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,88,28                  // movss         0x1c(%rax),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,96,32                  // movss         0x20(%rax),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  68,15,89,217                        // mulps         %xmm1,%xmm11
  .byte  69,15,88,220                        // addps         %xmm12,%xmm11
  .byte  69,15,89,208                        // mulps         %xmm8,%xmm10
  .byte  69,15,88,211                        // addps         %xmm11,%xmm10
  .byte  65,15,83,202                        // rcpps         %xmm10,%xmm1
  .byte  15,89,193                           // mulps         %xmm1,%xmm0
  .byte  68,15,89,201                        // mulps         %xmm1,%xmm9
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,201                        // movaps        %xmm9,%xmm1
  .byte  255,224                             // jmpq          *%rax

HIDDEN _sk_linear_gradient_2stops_sse2
.globl _sk_linear_gradient_2stops_sse2
_sk_linear_gradient_2stops_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  68,15,16,8                          // movups        (%rax),%xmm9
  .byte  15,16,88,16                         // movups        0x10(%rax),%xmm3
  .byte  68,15,40,195                        // movaps        %xmm3,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  65,15,40,201                        // movaps        %xmm9,%xmm1
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  68,15,89,192                        // mulps         %xmm0,%xmm8
  .byte  68,15,88,193                        // addps         %xmm1,%xmm8
  .byte  15,40,203                           // movaps        %xmm3,%xmm1
  .byte  15,198,201,85                       // shufps        $0x55,%xmm1,%xmm1
  .byte  65,15,40,209                        // movaps        %xmm9,%xmm2
  .byte  15,198,210,85                       // shufps        $0x55,%xmm2,%xmm2
  .byte  15,89,200                           // mulps         %xmm0,%xmm1
  .byte  15,88,202                           // addps         %xmm2,%xmm1
  .byte  15,40,211                           // movaps        %xmm3,%xmm2
  .byte  15,198,210,170                      // shufps        $0xaa,%xmm2,%xmm2
  .byte  69,15,40,209                        // movaps        %xmm9,%xmm10
  .byte  69,15,198,210,170                   // shufps        $0xaa,%xmm10,%xmm10
  .byte  15,89,208                           // mulps         %xmm0,%xmm2
  .byte  65,15,88,210                        // addps         %xmm10,%xmm2
  .byte  15,198,219,255                      // shufps        $0xff,%xmm3,%xmm3
  .byte  69,15,198,201,255                   // shufps        $0xff,%xmm9,%xmm9
  .byte  15,89,216                           // mulps         %xmm0,%xmm3
  .byte  65,15,88,217                        // addps         %xmm9,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  255,224                             // jmpq          *%rax
#endif
