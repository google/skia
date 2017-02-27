# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is generated semi-automatically with this command:
#   $ src/jumper/build_stages.py

.text
#if defined(__aarch64__)
.balign 16

.globl _sk_start_pipeline_aarch64
_sk_start_pipeline_aarch64:
  .long  0xa9bc5ff8                          // stp           x24, x23, [sp,#-64]!
  .long  0xa90157f6                          // stp           x22, x21, [sp,#16]
  .long  0xa9024ff4                          // stp           x20, x19, [sp,#32]
  .long  0xa9037bfd                          // stp           x29, x30, [sp,#48]
  .long  0x9100c3fd                          // add           x29, sp, #0x30
  .long  0xaa0103f3                          // mov           x19, x1
  .long  0xf8408677                          // ldr           x23, [x19],#8
  .long  0xaa0003f6                          // mov           x22, x0
  .long  0xaa0303f4                          // mov           x20, x3
  .long  0xaa0203f5                          // mov           x21, x2
  .long  0x910012c8                          // add           x8, x22, #0x4
  .long  0xeb14011f                          // cmp           x8, x20
  .long  0x54000069                          // b.ls          3c <sk_start_pipeline_aarch64+0x3c>
  .long  0xaa1603e0                          // mov           x0, x22
  .long  0x14000012                          // b             80 <sk_start_pipeline_aarch64+0x80>
  .long  0x6f00e400                          // movi          v0.2d, #0x0
  .long  0x6f00e401                          // movi          v1.2d, #0x0
  .long  0x6f00e402                          // movi          v2.2d, #0x0
  .long  0x6f00e403                          // movi          v3.2d, #0x0
  .long  0x6f00e404                          // movi          v4.2d, #0x0
  .long  0x6f00e405                          // movi          v5.2d, #0x0
  .long  0x6f00e406                          // movi          v6.2d, #0x0
  .long  0x6f00e407                          // movi          v7.2d, #0x0
  .long  0xaa1603e0                          // mov           x0, x22
  .long  0xaa1303e1                          // mov           x1, x19
  .long  0xaa1503e2                          // mov           x2, x21
  .long  0xd63f02e0                          // blr           x23
  .long  0x910022c8                          // add           x8, x22, #0x8
  .long  0x910012c0                          // add           x0, x22, #0x4
  .long  0xeb14011f                          // cmp           x8, x20
  .long  0xaa0003f6                          // mov           x22, x0
  .long  0x54fffe09                          // b.ls          3c <sk_start_pipeline_aarch64+0x3c>
  .long  0xa9437bfd                          // ldp           x29, x30, [sp,#48]
  .long  0xa9424ff4                          // ldp           x20, x19, [sp,#32]
  .long  0xa94157f6                          // ldp           x22, x21, [sp,#16]
  .long  0xa8c45ff8                          // ldp           x24, x23, [sp],#64
  .long  0xd65f03c0                          // ret

.globl _sk_just_return_aarch64
_sk_just_return_aarch64:
  .long  0xd65f03c0                          // ret

.globl _sk_seed_shader_aarch64
_sk_seed_shader_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0x4e040c00                          // dup           v0.4s, w0
  .long  0x4e21d800                          // scvtf         v0.4s, v0.4s
  .long  0x4f0167e1                          // movi          v1.4s, #0x3f, lsl #24
  .long  0x4d40c907                          // ld1r          {v7.4s}, [x8]
  .long  0x90000008                          // adrp          x8, 0 <sk_start_pipeline_aarch64>
  .long  0x3dc00106                          // ldr           q6, [x8]
  .long  0x4e21d400                          // fadd          v0.4s, v0.4s, v1.4s
  .long  0x4e21d8e7                          // scvtf         v7.4s, v7.4s
  .long  0x4f03f602                          // fmov          v2.4s, #1.000000000000000000e+00
  .long  0x6f00e403                          // movi          v3.2d, #0x0
  .long  0x6f00e404                          // movi          v4.2d, #0x0
  .long  0x6f00e405                          // movi          v5.2d, #0x0
  .long  0x4e26d400                          // fadd          v0.4s, v0.4s, v6.4s
  .long  0x6f00e406                          // movi          v6.2d, #0x0
  .long  0x4e21d4e1                          // fadd          v1.4s, v7.4s, v1.4s
  .long  0x6f00e407                          // movi          v7.2d, #0x0
  .long  0xd61f0060                          // br            x3

.globl _sk_constant_color_aarch64
_sk_constant_color_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0x3dc00103                          // ldr           q3, [x8]
  .long  0x4e040460                          // dup           v0.4s, v3.s[0]
  .long  0x4e0c0461                          // dup           v1.4s, v3.s[1]
  .long  0x4e140462                          // dup           v2.4s, v3.s[2]
  .long  0x4e1c0463                          // dup           v3.4s, v3.s[3]
  .long  0xd61f0060                          // br            x3

.globl _sk_clear_aarch64
_sk_clear_aarch64:
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x91004028                          // add           x8, x1, #0x10
  .long  0x6f00e400                          // movi          v0.2d, #0x0
  .long  0x6f00e401                          // movi          v1.2d, #0x0
  .long  0x6f00e402                          // movi          v2.2d, #0x0
  .long  0x6f00e403                          // movi          v3.2d, #0x0
  .long  0xaa0803e1                          // mov           x1, x8
  .long  0xd61f0060                          // br            x3

.globl _sk_plus__aarch64
_sk_plus__aarch64:
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x4e24d400                          // fadd          v0.4s, v0.4s, v4.4s
  .long  0x4e25d421                          // fadd          v1.4s, v1.4s, v5.4s
  .long  0x4e26d442                          // fadd          v2.4s, v2.4s, v6.4s
  .long  0x4e27d463                          // fadd          v3.4s, v3.4s, v7.4s
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

.globl _sk_srcover_aarch64
_sk_srcover_aarch64:
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x4ea3d610                          // fsub          v16.4s, v16.4s, v3.4s
  .long  0x4e24ce00                          // fmla          v0.4s, v16.4s, v4.4s
  .long  0x4e25ce01                          // fmla          v1.4s, v16.4s, v5.4s
  .long  0x4e26ce02                          // fmla          v2.4s, v16.4s, v6.4s
  .long  0x4e27ce03                          // fmla          v3.4s, v16.4s, v7.4s
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

.globl _sk_dstover_aarch64
_sk_dstover_aarch64:
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x4ea7d614                          // fsub          v20.4s, v16.4s, v7.4s
  .long  0x4ea41c90                          // mov           v16.16b, v4.16b
  .long  0x4ea51cb1                          // mov           v17.16b, v5.16b
  .long  0x4ea61cd2                          // mov           v18.16b, v6.16b
  .long  0x4ea71cf3                          // mov           v19.16b, v7.16b
  .long  0x4e20ce90                          // fmla          v16.4s, v20.4s, v0.4s
  .long  0x4e21ce91                          // fmla          v17.4s, v20.4s, v1.4s
  .long  0x4e22ce92                          // fmla          v18.4s, v20.4s, v2.4s
  .long  0x4e23ce93                          // fmla          v19.4s, v20.4s, v3.4s
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x4eb01e00                          // mov           v0.16b, v16.16b
  .long  0x4eb11e21                          // mov           v1.16b, v17.16b
  .long  0x4eb21e42                          // mov           v2.16b, v18.16b
  .long  0x4eb31e63                          // mov           v3.16b, v19.16b
  .long  0xd61f0060                          // br            x3

.globl _sk_clamp_0_aarch64
_sk_clamp_0_aarch64:
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x6f00e410                          // movi          v16.2d, #0x0
  .long  0x4e30f400                          // fmax          v0.4s, v0.4s, v16.4s
  .long  0x4e30f421                          // fmax          v1.4s, v1.4s, v16.4s
  .long  0x4e30f442                          // fmax          v2.4s, v2.4s, v16.4s
  .long  0x4e30f463                          // fmax          v3.4s, v3.4s, v16.4s
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

.globl _sk_clamp_1_aarch64
_sk_clamp_1_aarch64:
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x4eb0f400                          // fmin          v0.4s, v0.4s, v16.4s
  .long  0x4eb0f421                          // fmin          v1.4s, v1.4s, v16.4s
  .long  0x4eb0f442                          // fmin          v2.4s, v2.4s, v16.4s
  .long  0x4eb0f463                          // fmin          v3.4s, v3.4s, v16.4s
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

.globl _sk_clamp_a_aarch64
_sk_clamp_a_aarch64:
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x4eb0f463                          // fmin          v3.4s, v3.4s, v16.4s
  .long  0x4ea3f400                          // fmin          v0.4s, v0.4s, v3.4s
  .long  0x4ea3f421                          // fmin          v1.4s, v1.4s, v3.4s
  .long  0x4ea3f442                          // fmin          v2.4s, v2.4s, v3.4s
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

.globl _sk_set_rgb_aarch64
_sk_set_rgb_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0x91002109                          // add           x9, x8, #0x8
  .long  0x4ddfc900                          // ld1r          {v0.4s}, [x8], #4
  .long  0x4d40c922                          // ld1r          {v2.4s}, [x9]
  .long  0x4d40c901                          // ld1r          {v1.4s}, [x8]
  .long  0xd61f0060                          // br            x3

.globl _sk_swap_rb_aarch64
_sk_swap_rb_aarch64:
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x4ea01c10                          // mov           v16.16b, v0.16b
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x4ea21c40                          // mov           v0.16b, v2.16b
  .long  0x4eb01e02                          // mov           v2.16b, v16.16b
  .long  0xd61f0060                          // br            x3

.globl _sk_swap_aarch64
_sk_swap_aarch64:
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x4ea31c70                          // mov           v16.16b, v3.16b
  .long  0x4ea21c51                          // mov           v17.16b, v2.16b
  .long  0x4ea11c32                          // mov           v18.16b, v1.16b
  .long  0x4ea01c13                          // mov           v19.16b, v0.16b
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x4ea41c80                          // mov           v0.16b, v4.16b
  .long  0x4ea51ca1                          // mov           v1.16b, v5.16b
  .long  0x4ea61cc2                          // mov           v2.16b, v6.16b
  .long  0x4ea71ce3                          // mov           v3.16b, v7.16b
  .long  0x4eb31e64                          // mov           v4.16b, v19.16b
  .long  0x4eb21e45                          // mov           v5.16b, v18.16b
  .long  0x4eb11e26                          // mov           v6.16b, v17.16b
  .long  0x4eb01e07                          // mov           v7.16b, v16.16b
  .long  0xd61f0060                          // br            x3

.globl _sk_move_src_dst_aarch64
_sk_move_src_dst_aarch64:
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x4ea01c04                          // mov           v4.16b, v0.16b
  .long  0x4ea11c25                          // mov           v5.16b, v1.16b
  .long  0x4ea21c46                          // mov           v6.16b, v2.16b
  .long  0x4ea31c67                          // mov           v7.16b, v3.16b
  .long  0xd61f0060                          // br            x3

.globl _sk_move_dst_src_aarch64
_sk_move_dst_src_aarch64:
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x4ea41c80                          // mov           v0.16b, v4.16b
  .long  0x4ea51ca1                          // mov           v1.16b, v5.16b
  .long  0x4ea61cc2                          // mov           v2.16b, v6.16b
  .long  0x4ea71ce3                          // mov           v3.16b, v7.16b
  .long  0xd61f0060                          // br            x3

.globl _sk_premul_aarch64
_sk_premul_aarch64:
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x6e23dc00                          // fmul          v0.4s, v0.4s, v3.4s
  .long  0x6e23dc21                          // fmul          v1.4s, v1.4s, v3.4s
  .long  0x6e23dc42                          // fmul          v2.4s, v2.4s, v3.4s
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

.globl _sk_unpremul_aarch64
_sk_unpremul_aarch64:
  .long  0x4f03f611                          // fmov          v17.4s, #1.000000000000000000e+00
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x4ea0d870                          // fcmeq         v16.4s, v3.4s, #0.0
  .long  0x6e23fe31                          // fdiv          v17.4s, v17.4s, v3.4s
  .long  0x4e701e30                          // bic           v16.16b, v17.16b, v16.16b
  .long  0x6e20de00                          // fmul          v0.4s, v16.4s, v0.4s
  .long  0x6e21de01                          // fmul          v1.4s, v16.4s, v1.4s
  .long  0x6e22de02                          // fmul          v2.4s, v16.4s, v2.4s
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

.globl _sk_from_srgb_aarch64
_sk_from_srgb_aarch64:
  .long  0x52a7b3c8                          // mov           w8, #0x3d9e0000
  .long  0x72907228                          // movk          w8, #0x8391
  .long  0x4e040d10                          // dup           v16.4s, w8
  .long  0x52a7e648                          // mov           w8, #0x3f320000
  .long  0x7291eb88                          // movk          w8, #0x8f5c
  .long  0x4e040d11                          // dup           v17.4s, w8
  .long  0x52a7d328                          // mov           w8, #0x3e990000
  .long  0x72933348                          // movk          w8, #0x999a
  .long  0x4e040d12                          // dup           v18.4s, w8
  .long  0x52a76468                          // mov           w8, #0x3b230000
  .long  0x729ae148                          // movk          w8, #0xd70a
  .long  0x4eb11e33                          // mov           v19.16b, v17.16b
  .long  0x4e040d14                          // dup           v20.4s, w8
  .long  0x52a7ac28                          // mov           w8, #0x3d610000
  .long  0x7288f5c8                          // movk          w8, #0x47ae
  .long  0x4eb11e35                          // mov           v21.16b, v17.16b
  .long  0x6e20dc16                          // fmul          v22.4s, v0.4s, v0.4s
  .long  0x4e20ce53                          // fmla          v19.4s, v18.4s, v0.4s
  .long  0x4eb41e97                          // mov           v23.16b, v20.16b
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x4e040d18                          // dup           v24.4s, w8
  .long  0x4e21ce55                          // fmla          v21.4s, v18.4s, v1.4s
  .long  0x4e22ce51                          // fmla          v17.4s, v18.4s, v2.4s
  .long  0x6e21dc32                          // fmul          v18.4s, v1.4s, v1.4s
  .long  0x4e36ce77                          // fmla          v23.4s, v19.4s, v22.4s
  .long  0x4eb41e93                          // mov           v19.16b, v20.16b
  .long  0x6e22dc56                          // fmul          v22.4s, v2.4s, v2.4s
  .long  0x4e32ceb3                          // fmla          v19.4s, v21.4s, v18.4s
  .long  0x6e30dc12                          // fmul          v18.4s, v0.4s, v16.4s
  .long  0x6ea0e700                          // fcmgt         v0.4s, v24.4s, v0.4s
  .long  0x6e30dc35                          // fmul          v21.4s, v1.4s, v16.4s
  .long  0x6ea1e701                          // fcmgt         v1.4s, v24.4s, v1.4s
  .long  0x6e30dc50                          // fmul          v16.4s, v2.4s, v16.4s
  .long  0x4e36ce34                          // fmla          v20.4s, v17.4s, v22.4s
  .long  0x6ea2e702                          // fcmgt         v2.4s, v24.4s, v2.4s
  .long  0x6e771e40                          // bsl           v0.16b, v18.16b, v23.16b
  .long  0x6e731ea1                          // bsl           v1.16b, v21.16b, v19.16b
  .long  0x6e741e02                          // bsl           v2.16b, v16.16b, v20.16b
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

.globl _sk_to_srgb_aarch64
_sk_to_srgb_aarch64:
  .long  0x6ea1d810                          // frsqrte       v16.4s, v0.4s
  .long  0x6e30de13                          // fmul          v19.4s, v16.4s, v16.4s
  .long  0x6ea1d831                          // frsqrte       v17.4s, v1.4s
  .long  0x4eb3fc13                          // frsqrts       v19.4s, v0.4s, v19.4s
  .long  0x6e31de34                          // fmul          v20.4s, v17.4s, v17.4s
  .long  0x6e33de10                          // fmul          v16.4s, v16.4s, v19.4s
  .long  0x6ea1d852                          // frsqrte       v18.4s, v2.4s
  .long  0x4eb4fc34                          // frsqrts       v20.4s, v1.4s, v20.4s
  .long  0x4ea1da13                          // frecpe        v19.4s, v16.4s
  .long  0x6e32de55                          // fmul          v21.4s, v18.4s, v18.4s
  .long  0x6e34de31                          // fmul          v17.4s, v17.4s, v20.4s
  .long  0x6ea1da14                          // frsqrte       v20.4s, v16.4s
  .long  0x4e33fe19                          // frecps        v25.4s, v16.4s, v19.4s
  .long  0x4eb5fc55                          // frsqrts       v21.4s, v2.4s, v21.4s
  .long  0x6e39de73                          // fmul          v19.4s, v19.4s, v25.4s
  .long  0x6e34de99                          // fmul          v25.4s, v20.4s, v20.4s
  .long  0x6e35de52                          // fmul          v18.4s, v18.4s, v21.4s
  .long  0x4ea1da35                          // frecpe        v21.4s, v17.4s
  .long  0x4eb9fe10                          // frsqrts       v16.4s, v16.4s, v25.4s
  .long  0x52b7b948                          // mov           w8, #0xbdca0000
  .long  0x6ea1da36                          // frsqrte       v22.4s, v17.4s
  .long  0x6e30de90                          // fmul          v16.4s, v20.4s, v16.4s
  .long  0x4e35fe34                          // frecps        v20.4s, v17.4s, v21.4s
  .long  0x728af508                          // movk          w8, #0x57a8
  .long  0x4ea1da57                          // frecpe        v23.4s, v18.4s
  .long  0x6e34deb4                          // fmul          v20.4s, v21.4s, v20.4s
  .long  0x6e36ded5                          // fmul          v21.4s, v22.4s, v22.4s
  .long  0x6ea1da58                          // frsqrte       v24.4s, v18.4s
  .long  0x4e040d19                          // dup           v25.4s, w8
  .long  0x52a7e608                          // mov           w8, #0x3f300000
  .long  0x4eb5fe31                          // frsqrts       v17.4s, v17.4s, v21.4s
  .long  0x4e37fe55                          // frecps        v21.4s, v18.4s, v23.4s
  .long  0x728df9c8                          // movk          w8, #0x6fce
  .long  0x6e35def5                          // fmul          v21.4s, v23.4s, v21.4s
  .long  0x6e38df17                          // fmul          v23.4s, v24.4s, v24.4s
  .long  0x4eb7fe52                          // frsqrts       v18.4s, v18.4s, v23.4s
  .long  0x4e040d17                          // dup           v23.4s, w8
  .long  0x52a7da48                          // mov           w8, #0x3ed20000
  .long  0x7290f848                          // movk          w8, #0x87c2
  .long  0x6e31ded1                          // fmul          v17.4s, v22.4s, v17.4s
  .long  0x4eb91f36                          // mov           v22.16b, v25.16b
  .long  0x4e33cef6                          // fmla          v22.4s, v23.4s, v19.4s
  .long  0x4e040d13                          // dup           v19.4s, w8
  .long  0x52a828e8                          // mov           w8, #0x41470000
  .long  0x6e32df12                          // fmul          v18.4s, v24.4s, v18.4s
  .long  0x4eb91f38                          // mov           v24.16b, v25.16b
  .long  0x728b8528                          // movk          w8, #0x5c29
  .long  0x4e34cef8                          // fmla          v24.4s, v23.4s, v20.4s
  .long  0x4e040d14                          // dup           v20.4s, w8
  .long  0x52a77188                          // mov           w8, #0x3b8c0000
  .long  0x4e35cef9                          // fmla          v25.4s, v23.4s, v21.4s
  .long  0x729ce088                          // movk          w8, #0xe704
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x4e30ce76                          // fmla          v22.4s, v19.4s, v16.4s
  .long  0x4f03f610                          // fmov          v16.4s, #1.000000000000000000e+00
  .long  0x4e040d15                          // dup           v21.4s, w8
  .long  0x4e31ce78                          // fmla          v24.4s, v19.4s, v17.4s
  .long  0x4e32ce79                          // fmla          v25.4s, v19.4s, v18.4s
  .long  0x6e34dc11                          // fmul          v17.4s, v0.4s, v20.4s
  .long  0x4eb6f612                          // fmin          v18.4s, v16.4s, v22.4s
  .long  0x6ea0e6a0                          // fcmgt         v0.4s, v21.4s, v0.4s
  .long  0x6e34dc33                          // fmul          v19.4s, v1.4s, v20.4s
  .long  0x4eb8f616                          // fmin          v22.4s, v16.4s, v24.4s
  .long  0x6ea1e6a1                          // fcmgt         v1.4s, v21.4s, v1.4s
  .long  0x6e34dc54                          // fmul          v20.4s, v2.4s, v20.4s
  .long  0x4eb9f610                          // fmin          v16.4s, v16.4s, v25.4s
  .long  0x6ea2e6a2                          // fcmgt         v2.4s, v21.4s, v2.4s
  .long  0x6e721e20                          // bsl           v0.16b, v17.16b, v18.16b
  .long  0x6e761e61                          // bsl           v1.16b, v19.16b, v22.16b
  .long  0x6e701e82                          // bsl           v2.16b, v20.16b, v16.16b
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

.globl _sk_scale_1_float_aarch64
_sk_scale_1_float_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0xbd400110                          // ldr           s16, [x8]
  .long  0x4f909000                          // fmul          v0.4s, v0.4s, v16.s[0]
  .long  0x4f909021                          // fmul          v1.4s, v1.4s, v16.s[0]
  .long  0x4f909042                          // fmul          v2.4s, v2.4s, v16.s[0]
  .long  0x4f909063                          // fmul          v3.4s, v3.4s, v16.s[0]
  .long  0xd61f0060                          // br            x3

.globl _sk_scale_u8_aarch64
_sk_scale_u8_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0x52a77009                          // mov           w9, #0x3b800000
  .long  0x72901029                          // movk          w9, #0x8081
  .long  0x4e040d30                          // dup           v16.4s, w9
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x8b000108                          // add           x8, x8, x0
  .long  0x39400109                          // ldrb          w9, [x8]
  .long  0x3940050a                          // ldrb          w10, [x8,#1]
  .long  0x3940090b                          // ldrb          w11, [x8,#2]
  .long  0x39400d08                          // ldrb          w8, [x8,#3]
  .long  0x4e021d31                          // mov           v17.h[0], w9
  .long  0x4e061d51                          // mov           v17.h[1], w10
  .long  0x4e0a1d71                          // mov           v17.h[2], w11
  .long  0x4e0e1d11                          // mov           v17.h[3], w8
  .long  0x2f07b7f1                          // bic           v17.4h, #0xff, lsl #8
  .long  0x2f10a631                          // uxtl          v17.4s, v17.4h
  .long  0x6e21da31                          // ucvtf         v17.4s, v17.4s
  .long  0x6e30de30                          // fmul          v16.4s, v17.4s, v16.4s
  .long  0x6e20de00                          // fmul          v0.4s, v16.4s, v0.4s
  .long  0x6e21de01                          // fmul          v1.4s, v16.4s, v1.4s
  .long  0x6e22de02                          // fmul          v2.4s, v16.4s, v2.4s
  .long  0x6e23de03                          // fmul          v3.4s, v16.4s, v3.4s
  .long  0xd61f0060                          // br            x3

.globl _sk_lerp_1_float_aarch64
_sk_lerp_1_float_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
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

.globl _sk_lerp_u8_aarch64
_sk_lerp_u8_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0x4ea4d410                          // fsub          v16.4s, v0.4s, v4.4s
  .long  0x4ea5d431                          // fsub          v17.4s, v1.4s, v5.4s
  .long  0x4ea6d452                          // fsub          v18.4s, v2.4s, v6.4s
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x4ea7d473                          // fsub          v19.4s, v3.4s, v7.4s
  .long  0x4ea61cc2                          // mov           v2.16b, v6.16b
  .long  0x4ea71ce3                          // mov           v3.16b, v7.16b
  .long  0x8b000108                          // add           x8, x8, x0
  .long  0x39400109                          // ldrb          w9, [x8]
  .long  0x3940050a                          // ldrb          w10, [x8,#1]
  .long  0x3940090b                          // ldrb          w11, [x8,#2]
  .long  0x39400d08                          // ldrb          w8, [x8,#3]
  .long  0x4e021d20                          // mov           v0.h[0], w9
  .long  0x4e061d40                          // mov           v0.h[1], w10
  .long  0x4e0a1d60                          // mov           v0.h[2], w11
  .long  0x4e0e1d00                          // mov           v0.h[3], w8
  .long  0x52a77009                          // mov           w9, #0x3b800000
  .long  0x2f07b7e0                          // bic           v0.4h, #0xff, lsl #8
  .long  0x72901029                          // movk          w9, #0x8081
  .long  0x2f10a400                          // uxtl          v0.4s, v0.4h
  .long  0x4e040d21                          // dup           v1.4s, w9
  .long  0x6e21d800                          // ucvtf         v0.4s, v0.4s
  .long  0x6e21dc14                          // fmul          v20.4s, v0.4s, v1.4s
  .long  0x4ea41c80                          // mov           v0.16b, v4.16b
  .long  0x4ea51ca1                          // mov           v1.16b, v5.16b
  .long  0x4e30ce80                          // fmla          v0.4s, v20.4s, v16.4s
  .long  0x4e31ce81                          // fmla          v1.4s, v20.4s, v17.4s
  .long  0x4e32ce82                          // fmla          v2.4s, v20.4s, v18.4s
  .long  0x4e33ce83                          // fmla          v3.4s, v20.4s, v19.4s
  .long  0xd61f0060                          // br            x3

.globl _sk_lerp_565_aarch64
_sk_lerp_565_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0xd37ff809                          // lsl           x9, x0, #1
  .long  0x4ea4d412                          // fsub          v18.4s, v0.4s, v4.4s
  .long  0x4f072710                          // movi          v16.4s, #0xf8, lsl #8
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x4f0007f1                          // movi          v17.4s, #0x1f
  .long  0x4ea5d433                          // fsub          v19.4s, v1.4s, v5.4s
  .long  0x52a7404a                          // mov           w10, #0x3a020000
  .long  0xfc696900                          // ldr           d0, [x8,x9]
  .long  0x321b17e8                          // orr           w8, wzr, #0x7e0
  .long  0x52a6f089                          // mov           w9, #0x37840000
  .long  0x72842109                          // movk          w9, #0x2108
  .long  0x2f10a400                          // uxtl          v0.4s, v0.4h
  .long  0x4e301c01                          // and           v1.16b, v0.16b, v16.16b
  .long  0x4e040d10                          // dup           v16.4s, w8
  .long  0x4ea6d454                          // fsub          v20.4s, v2.4s, v6.4s
  .long  0x7281042a                          // movk          w10, #0x821
  .long  0x4e311c02                          // and           v2.16b, v0.16b, v17.16b
  .long  0x4e21d821                          // scvtf         v1.4s, v1.4s
  .long  0x52a7a08b                          // mov           w11, #0x3d040000
  .long  0x4e301c00                          // and           v0.16b, v0.16b, v16.16b
  .long  0x4e040d30                          // dup           v16.4s, w9
  .long  0x7284210b                          // movk          w11, #0x2108
  .long  0x4e21d800                          // scvtf         v0.4s, v0.4s
  .long  0x6e30dc30                          // fmul          v16.4s, v1.4s, v16.4s
  .long  0x4e040d41                          // dup           v1.4s, w10
  .long  0x4e21d842                          // scvtf         v2.4s, v2.4s
  .long  0x6e21dc11                          // fmul          v17.4s, v0.4s, v1.4s
  .long  0x4e040d60                          // dup           v0.4s, w11
  .long  0x6e20dc55                          // fmul          v21.4s, v2.4s, v0.4s
  .long  0x4ea41c80                          // mov           v0.16b, v4.16b
  .long  0x4ea51ca1                          // mov           v1.16b, v5.16b
  .long  0x4ea61cc2                          // mov           v2.16b, v6.16b
  .long  0x4f03f603                          // fmov          v3.4s, #1.000000000000000000e+00
  .long  0x4e32ce00                          // fmla          v0.4s, v16.4s, v18.4s
  .long  0x4e33ce21                          // fmla          v1.4s, v17.4s, v19.4s
  .long  0x4e34cea2                          // fmla          v2.4s, v21.4s, v20.4s
  .long  0xd61f0060                          // br            x3

.globl _sk_load_tables_aarch64
_sk_load_tables_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0xd37ef409                          // lsl           x9, x0, #2
  .long  0x6f00e620                          // movi          v0.2d, #0xff000000ff
  .long  0xa9402d0a                          // ldp           x10, x11, [x8]
  .long  0xa941210c                          // ldp           x12, x8, [x8,#16]
  .long  0x3ce96941                          // ldr           q1, [x10,x9]
  .long  0x52a77009                          // mov           w9, #0x3b800000
  .long  0x72901029                          // movk          w9, #0x8081
  .long  0x4e040d22                          // dup           v2.4s, w9
  .long  0x4e201c31                          // and           v17.16b, v1.16b, v0.16b
  .long  0x0e143e2a                          // mov           w10, v17.s[2]
  .long  0x6f380423                          // ushr          v3.4s, v1.4s, #8
  .long  0x6f300430                          // ushr          v16.4s, v1.4s, #16
  .long  0xbc6a5973                          // ldr           s19, [x11,w10,uxtw #2]
  .long  0x0e1c3e2a                          // mov           w10, v17.s[3]
  .long  0x4e201c72                          // and           v18.16b, v3.16b, v0.16b
  .long  0x4e201e10                          // and           v16.16b, v16.16b, v0.16b
  .long  0x6f280420                          // ushr          v0.4s, v1.4s, #24
  .long  0xbc6a5974                          // ldr           s20, [x11,w10,uxtw #2]
  .long  0x1e26022a                          // fmov          w10, s17
  .long  0x4e21d800                          // scvtf         v0.4s, v0.4s
  .long  0x8b2a496a                          // add           x10, x11, w10, uxtw #2
  .long  0x6e22dc03                          // fmul          v3.4s, v0.4s, v2.4s
  .long  0x0d408140                          // ld1           {v0.s}[0], [x10]
  .long  0x0e143e4a                          // mov           w10, v18.s[2]
  .long  0x0e0c3e29                          // mov           w9, v17.s[1]
  .long  0xbc6a5991                          // ldr           s17, [x12,w10,uxtw #2]
  .long  0x1e26024a                          // fmov          w10, s18
  .long  0x8b2a498a                          // add           x10, x12, w10, uxtw #2
  .long  0x0d408141                          // ld1           {v1.s}[0], [x10]
  .long  0x1e26020a                          // fmov          w10, s16
  .long  0x8b2a490a                          // add           x10, x8, w10, uxtw #2
  .long  0x8b294969                          // add           x9, x11, w9, uxtw #2
  .long  0x0d408142                          // ld1           {v2.s}[0], [x10]
  .long  0x0e1c3e4d                          // mov           w13, v18.s[3]
  .long  0x0d409120                          // ld1           {v0.s}[1], [x9]
  .long  0x0e0c3e4b                          // mov           w11, v18.s[1]
  .long  0xbc6d5992                          // ldr           s18, [x12,w13,uxtw #2]
  .long  0x0e0c3e0d                          // mov           w13, v16.s[1]
  .long  0x8b2b498b                          // add           x11, x12, w11, uxtw #2
  .long  0x0e143e0c                          // mov           w12, v16.s[2]
  .long  0x8b2d490a                          // add           x10, x8, w13, uxtw #2
  .long  0x0e1c3e09                          // mov           w9, v16.s[3]
  .long  0xbc6c5910                          // ldr           s16, [x8,w12,uxtw #2]
  .long  0x0d409161                          // ld1           {v1.s}[1], [x11]
  .long  0x0d409142                          // ld1           {v2.s}[1], [x10]
  .long  0x6e140660                          // mov           v0.s[2], v19.s[0]
  .long  0xbc695913                          // ldr           s19, [x8,w9,uxtw #2]
  .long  0x6e140621                          // mov           v1.s[2], v17.s[0]
  .long  0x6e140602                          // mov           v2.s[2], v16.s[0]
  .long  0x6e1c0680                          // mov           v0.s[3], v20.s[0]
  .long  0x6e1c0641                          // mov           v1.s[3], v18.s[0]
  .long  0x6e1c0662                          // mov           v2.s[3], v19.s[0]
  .long  0xd61f0060                          // br            x3

.globl _sk_load_a8_aarch64
_sk_load_a8_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0x52a77009                          // mov           w9, #0x3b800000
  .long  0x72901029                          // movk          w9, #0x8081
  .long  0x4e040d23                          // dup           v3.4s, w9
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x6f00e400                          // movi          v0.2d, #0x0
  .long  0x6f00e401                          // movi          v1.2d, #0x0
  .long  0x6f00e402                          // movi          v2.2d, #0x0
  .long  0x8b000108                          // add           x8, x8, x0
  .long  0x39400109                          // ldrb          w9, [x8]
  .long  0x3940050a                          // ldrb          w10, [x8,#1]
  .long  0x3940090b                          // ldrb          w11, [x8,#2]
  .long  0x39400d08                          // ldrb          w8, [x8,#3]
  .long  0x4e021d30                          // mov           v16.h[0], w9
  .long  0x4e061d50                          // mov           v16.h[1], w10
  .long  0x4e0a1d70                          // mov           v16.h[2], w11
  .long  0x4e0e1d10                          // mov           v16.h[3], w8
  .long  0x2f07b7f0                          // bic           v16.4h, #0xff, lsl #8
  .long  0x2f10a610                          // uxtl          v16.4s, v16.4h
  .long  0x6e21da10                          // ucvtf         v16.4s, v16.4s
  .long  0x6e23de03                          // fmul          v3.4s, v16.4s, v3.4s
  .long  0xd61f0060                          // br            x3

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
  .long  0x39000d09                          // strb          w9, [x8,#3]
  .long  0x0e0a3e09                          // umov          w9, v16.h[2]
  .long  0x39000909                          // strb          w9, [x8,#2]
  .long  0x0e063e09                          // umov          w9, v16.h[1]
  .long  0x39000509                          // strb          w9, [x8,#1]
  .long  0x0e023e09                          // umov          w9, v16.h[0]
  .long  0x39000109                          // strb          w9, [x8]
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

.globl _sk_load_565_aarch64
_sk_load_565_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0xd37ff809                          // lsl           x9, x0, #1
  .long  0x4f0007e1                          // movi          v1.4s, #0x1f
  .long  0x4f072700                          // movi          v0.4s, #0xf8, lsl #8
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x52a7404a                          // mov           w10, #0x3a020000
  .long  0x7281042a                          // movk          w10, #0x821
  .long  0x52a7a08b                          // mov           w11, #0x3d040000
  .long  0xfc696902                          // ldr           d2, [x8,x9]
  .long  0x52a6f088                          // mov           w8, #0x37840000
  .long  0x321b17e9                          // orr           w9, wzr, #0x7e0
  .long  0x72842108                          // movk          w8, #0x2108
  .long  0x2f10a442                          // uxtl          v2.4s, v2.4h
  .long  0x4e211c41                          // and           v1.16b, v2.16b, v1.16b
  .long  0x4e201c40                          // and           v0.16b, v2.16b, v0.16b
  .long  0x4e21d830                          // scvtf         v16.4s, v1.4s
  .long  0x4e040d21                          // dup           v1.4s, w9
  .long  0x4e21d800                          // scvtf         v0.4s, v0.4s
  .long  0x4e211c41                          // and           v1.16b, v2.16b, v1.16b
  .long  0x4e040d02                          // dup           v2.4s, w8
  .long  0x7284210b                          // movk          w11, #0x2108
  .long  0x4e21d821                          // scvtf         v1.4s, v1.4s
  .long  0x6e22dc00                          // fmul          v0.4s, v0.4s, v2.4s
  .long  0x4e040d42                          // dup           v2.4s, w10
  .long  0x6e22dc21                          // fmul          v1.4s, v1.4s, v2.4s
  .long  0x4e040d62                          // dup           v2.4s, w11
  .long  0x4f03f603                          // fmov          v3.4s, #1.000000000000000000e+00
  .long  0x6e22de02                          // fmul          v2.4s, v16.4s, v2.4s
  .long  0xd61f0060                          // br            x3

.globl _sk_store_565_aarch64
_sk_store_565_aarch64:
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0x52a84f89                          // mov           w9, #0x427c0000
  .long  0x4f01f7f0                          // fmov          v16.4s, #3.100000000000000000e+01
  .long  0x4e040d31                          // dup           v17.4s, w9
  .long  0x6e30dc12                          // fmul          v18.4s, v0.4s, v16.4s
  .long  0x6e31dc31                          // fmul          v17.4s, v1.4s, v17.4s
  .long  0x6e21aa52                          // fcvtnu        v18.4s, v18.4s
  .long  0x6e21aa31                          // fcvtnu        v17.4s, v17.4s
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x4f2b5652                          // shl           v18.4s, v18.4s, #11
  .long  0x4f255631                          // shl           v17.4s, v17.4s, #5
  .long  0x6e30dc50                          // fmul          v16.4s, v2.4s, v16.4s
  .long  0x4eb21e31                          // orr           v17.16b, v17.16b, v18.16b
  .long  0x6e21aa10                          // fcvtnu        v16.4s, v16.4s
  .long  0x4eb01e30                          // orr           v16.16b, v17.16b, v16.16b
  .long  0xd37ff809                          // lsl           x9, x0, #1
  .long  0x0e612a10                          // xtn           v16.4h, v16.4s
  .long  0xfc296910                          // str           d16, [x8,x9]
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

.globl _sk_load_8888_aarch64
_sk_load_8888_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0xd37ef409                          // lsl           x9, x0, #2
  .long  0x6f00e620                          // movi          v0.2d, #0xff000000ff
  .long  0x52a7700a                          // mov           w10, #0x3b800000
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x7290102a                          // movk          w10, #0x8081
  .long  0x4e040d43                          // dup           v3.4s, w10
  .long  0x3ce96901                          // ldr           q1, [x8,x9]
  .long  0x4e201c22                          // and           v2.16b, v1.16b, v0.16b
  .long  0x6f380430                          // ushr          v16.4s, v1.4s, #8
  .long  0x6f300431                          // ushr          v17.4s, v1.4s, #16
  .long  0x6f280421                          // ushr          v1.4s, v1.4s, #24
  .long  0x4e21d842                          // scvtf         v2.4s, v2.4s
  .long  0x4e201e10                          // and           v16.16b, v16.16b, v0.16b
  .long  0x4e201e31                          // and           v17.16b, v17.16b, v0.16b
  .long  0x4e21d832                          // scvtf         v18.4s, v1.4s
  .long  0x6e23dc40                          // fmul          v0.4s, v2.4s, v3.4s
  .long  0x4e21da01                          // scvtf         v1.4s, v16.4s
  .long  0x4e21da22                          // scvtf         v2.4s, v17.4s
  .long  0x6e23dc21                          // fmul          v1.4s, v1.4s, v3.4s
  .long  0x6e23dc42                          // fmul          v2.4s, v2.4s, v3.4s
  .long  0x6e23de43                          // fmul          v3.4s, v18.4s, v3.4s
  .long  0xd61f0060                          // br            x3

.globl _sk_store_8888_aarch64
_sk_store_8888_aarch64:
  .long  0x52a86fe9                          // mov           w9, #0x437f0000
  .long  0x4e040d30                          // dup           v16.4s, w9
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0x6e30dc31                          // fmul          v17.4s, v1.4s, v16.4s
  .long  0x6e30dc12                          // fmul          v18.4s, v0.4s, v16.4s
  .long  0x6e21aa31                          // fcvtnu        v17.4s, v17.4s
  .long  0x6e21aa52                          // fcvtnu        v18.4s, v18.4s
  .long  0x4f285631                          // shl           v17.4s, v17.4s, #8
  .long  0x4eb21e31                          // orr           v17.16b, v17.16b, v18.16b
  .long  0x6e30dc52                          // fmul          v18.4s, v2.4s, v16.4s
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x6e21aa52                          // fcvtnu        v18.4s, v18.4s
  .long  0x6e30dc70                          // fmul          v16.4s, v3.4s, v16.4s
  .long  0x4f305652                          // shl           v18.4s, v18.4s, #16
  .long  0x6e21aa10                          // fcvtnu        v16.4s, v16.4s
  .long  0x4eb21e31                          // orr           v17.16b, v17.16b, v18.16b
  .long  0x4f385610                          // shl           v16.4s, v16.4s, #24
  .long  0xd37ef409                          // lsl           x9, x0, #2
  .long  0x4eb01e30                          // orr           v16.16b, v17.16b, v16.16b
  .long  0x3ca96910                          // str           q16, [x8,x9]
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

.globl _sk_load_f16_aarch64
_sk_load_f16_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x8b000d08                          // add           x8, x8, x0, lsl #3
  .long  0x0c400510                          // ld4           {v16.4h-v19.4h}, [x8]
  .long  0x0e217a00                          // fcvtl         v0.4s, v16.4h
  .long  0x0e217a21                          // fcvtl         v1.4s, v17.4h
  .long  0x0e217a42                          // fcvtl         v2.4s, v18.4h
  .long  0x0e217a63                          // fcvtl         v3.4s, v19.4h
  .long  0xd61f0060                          // br            x3

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
  .long  0xf9400423                          // ldr           x3, [x1,#8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0060                          // br            x3

.globl _sk_clamp_x_aarch64
_sk_clamp_x_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0x6f00e410                          // movi          v16.2d, #0x0
  .long  0x4e20f600                          // fmax          v0.4s, v16.4s, v0.4s
  .long  0x6f07e7f1                          // movi          v17.2d, #0xffffffffffffffff
  .long  0x4d40c910                          // ld1r          {v16.4s}, [x8]
  .long  0x4eb18610                          // add           v16.4s, v16.4s, v17.4s
  .long  0x4eb0f400                          // fmin          v0.4s, v0.4s, v16.4s
  .long  0xd61f0060                          // br            x3

.globl _sk_clamp_y_aarch64
_sk_clamp_y_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0x6f00e410                          // movi          v16.2d, #0x0
  .long  0x4e21f601                          // fmax          v1.4s, v16.4s, v1.4s
  .long  0x6f07e7f1                          // movi          v17.2d, #0xffffffffffffffff
  .long  0x4d40c910                          // ld1r          {v16.4s}, [x8]
  .long  0x4eb18610                          // add           v16.4s, v16.4s, v17.4s
  .long  0x4eb0f421                          // fmin          v1.4s, v1.4s, v16.4s
  .long  0xd61f0060                          // br            x3

.globl _sk_repeat_x_aarch64
_sk_repeat_x_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0x6f07e7f0                          // movi          v16.2d, #0xffffffffffffffff
  .long  0xbd400111                          // ldr           s17, [x8]
  .long  0x4e040632                          // dup           v18.4s, v17.s[0]
  .long  0x4eb08650                          // add           v16.4s, v18.4s, v16.4s
  .long  0x6e32fc12                          // fdiv          v18.4s, v0.4s, v18.4s
  .long  0x4e219a52                          // frintm        v18.4s, v18.4s
  .long  0x4f919251                          // fmul          v17.4s, v18.4s, v17.s[0]
  .long  0x4eb1d400                          // fsub          v0.4s, v0.4s, v17.4s
  .long  0x4eb0f400                          // fmin          v0.4s, v0.4s, v16.4s
  .long  0xd61f0060                          // br            x3

.globl _sk_repeat_y_aarch64
_sk_repeat_y_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0x6f07e7f0                          // movi          v16.2d, #0xffffffffffffffff
  .long  0xbd400111                          // ldr           s17, [x8]
  .long  0x4e040632                          // dup           v18.4s, v17.s[0]
  .long  0x4eb08650                          // add           v16.4s, v18.4s, v16.4s
  .long  0x6e32fc32                          // fdiv          v18.4s, v1.4s, v18.4s
  .long  0x4e219a52                          // frintm        v18.4s, v18.4s
  .long  0x4f919251                          // fmul          v17.4s, v18.4s, v17.s[0]
  .long  0x4eb1d421                          // fsub          v1.4s, v1.4s, v17.4s
  .long  0x4eb0f421                          // fmin          v1.4s, v1.4s, v16.4s
  .long  0xd61f0060                          // br            x3

.globl _sk_mirror_x_aarch64
_sk_mirror_x_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0x6f07e7f0                          // movi          v16.2d, #0xffffffffffffffff
  .long  0xbd400111                          // ldr           s17, [x8]
  .long  0x4e040632                          // dup           v18.4s, v17.s[0]
  .long  0x1e312a31                          // fadd          s17, s17, s17
  .long  0x4eb2d400                          // fsub          v0.4s, v0.4s, v18.4s
  .long  0x4e040633                          // dup           v19.4s, v17.s[0]
  .long  0x6e33fc13                          // fdiv          v19.4s, v0.4s, v19.4s
  .long  0x4e219a73                          // frintm        v19.4s, v19.4s
  .long  0x4f919271                          // fmul          v17.4s, v19.4s, v17.s[0]
  .long  0x4eb1d400                          // fsub          v0.4s, v0.4s, v17.4s
  .long  0x4eb2d400                          // fsub          v0.4s, v0.4s, v18.4s
  .long  0x4ea0f800                          // fabs          v0.4s, v0.4s
  .long  0x4eb08650                          // add           v16.4s, v18.4s, v16.4s
  .long  0x4eb0f400                          // fmin          v0.4s, v0.4s, v16.4s
  .long  0xd61f0060                          // br            x3

.globl _sk_mirror_y_aarch64
_sk_mirror_y_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0x6f07e7f0                          // movi          v16.2d, #0xffffffffffffffff
  .long  0xbd400111                          // ldr           s17, [x8]
  .long  0x4e040632                          // dup           v18.4s, v17.s[0]
  .long  0x1e312a31                          // fadd          s17, s17, s17
  .long  0x4eb2d421                          // fsub          v1.4s, v1.4s, v18.4s
  .long  0x4e040633                          // dup           v19.4s, v17.s[0]
  .long  0x6e33fc33                          // fdiv          v19.4s, v1.4s, v19.4s
  .long  0x4e219a73                          // frintm        v19.4s, v19.4s
  .long  0x4f919271                          // fmul          v17.4s, v19.4s, v17.s[0]
  .long  0x4eb1d421                          // fsub          v1.4s, v1.4s, v17.4s
  .long  0x4eb2d421                          // fsub          v1.4s, v1.4s, v18.4s
  .long  0x4ea0f821                          // fabs          v1.4s, v1.4s
  .long  0x4eb08650                          // add           v16.4s, v18.4s, v16.4s
  .long  0x4eb0f421                          // fmin          v1.4s, v1.4s, v16.4s
  .long  0xd61f0060                          // br            x3

.globl _sk_matrix_2x3_aarch64
_sk_matrix_2x3_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0xaa0803e9                          // mov           x9, x8
  .long  0x9100410a                          // add           x10, x8, #0x10
  .long  0x4ddfc932                          // ld1r          {v18.4s}, [x9], #4
  .long  0x4d40c950                          // ld1r          {v16.4s}, [x10]
  .long  0x2d415113                          // ldp           s19, s20, [x8,#8]
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

.globl _sk_matrix_3x4_aarch64
_sk_matrix_3x4_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0xaa0803e9                          // mov           x9, x8
  .long  0x9100910a                          // add           x10, x8, #0x24
  .long  0x4ddfc933                          // ld1r          {v19.4s}, [x9], #4
  .long  0x4d40c950                          // ld1r          {v16.4s}, [x10]
  .long  0x9100a10a                          // add           x10, x8, #0x28
  .long  0x4d40c951                          // ld1r          {v17.4s}, [x10]
  .long  0x9100b10a                          // add           x10, x8, #0x2c
  .long  0x2d435514                          // ldp           s20, s21, [x8,#24]
  .long  0xbd402116                          // ldr           s22, [x8,#32]
  .long  0x4d40c952                          // ld1r          {v18.4s}, [x10]
  .long  0x4f941050                          // fmla          v16.4s, v2.4s, v20.s[0]
  .long  0x2d415d14                          // ldp           s20, s23, [x8,#8]
  .long  0x4f951051                          // fmla          v17.4s, v2.4s, v21.s[0]
  .long  0x4f961052                          // fmla          v18.4s, v2.4s, v22.s[0]
  .long  0x2d425502                          // ldp           s2, s21, [x8,#16]
  .long  0x4f971030                          // fmla          v16.4s, v1.4s, v23.s[0]
  .long  0x4e20ce70                          // fmla          v16.4s, v19.4s, v0.4s
  .long  0x4f821031                          // fmla          v17.4s, v1.4s, v2.s[0]
  .long  0xbd400122                          // ldr           s2, [x9]
  .long  0x4f951032                          // fmla          v18.4s, v1.4s, v21.s[0]
  .long  0x4f941012                          // fmla          v18.4s, v0.4s, v20.s[0]
  .long  0x4f821011                          // fmla          v17.4s, v0.4s, v2.s[0]
  .long  0x4eb01e00                          // mov           v0.16b, v16.16b
  .long  0x4eb11e21                          // mov           v1.16b, v17.16b
  .long  0x4eb21e42                          // mov           v2.16b, v18.16b
  .long  0xd61f0060                          // br            x3

.globl _sk_matrix_perspective_aarch64
_sk_matrix_perspective_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
  .long  0xaa0803e9                          // mov           x9, x8
  .long  0x9100510a                          // add           x10, x8, #0x14
  .long  0x4ddfc930                          // ld1r          {v16.4s}, [x9], #4
  .long  0x4d40c951                          // ld1r          {v17.4s}, [x10]
  .long  0x9100810a                          // add           x10, x8, #0x20
  .long  0x4d40c952                          // ld1r          {v18.4s}, [x10]
  .long  0x2d41d113                          // ldp           s19, s20, [x8,#12]
  .long  0x2d435915                          // ldp           s21, s22, [x8,#24]
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

.globl _sk_linear_gradient_2stops_aarch64
_sk_linear_gradient_2stops_aarch64:
  .long  0xa8c10c28                          // ldp           x8, x3, [x1],#16
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
.balign 16

.globl _sk_start_pipeline_vfp4
_sk_start_pipeline_vfp4:
  .long  0xe92d41f0                          // push          {r4, r5, r6, r7, r8, lr}
  .long  0xe1a07001                          // mov           r7, r1
  .long  0xe1a04000                          // mov           r4, r0
  .long  0xe1a05003                          // mov           r5, r3
  .long  0xe1a08002                          // mov           r8, r2
  .long  0xe4976004                          // ldr           r6, [r7], #4
  .long  0xe2840002                          // add           r0, r4, #2
  .long  0xea00000d                          // b             58 <sk_start_pipeline_vfp4+0x58>
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
  .long  0x9affffef                          // bls           20 <sk_start_pipeline_vfp4+0x20>
  .long  0xe1a00004                          // mov           r0, r4
  .long  0xe8bd81f0                          // pop           {r4, r5, r6, r7, r8, pc}

.globl _sk_just_return_vfp4
_sk_just_return_vfp4:
  .long  0xe12fff1e                          // bx            lr
  .long  0xe320f000                          // nop           {0}

.globl _sk_seed_shader_vfp4
_sk_seed_shader_vfp4:
  .long  0xee800b90                          // vdup.32       d16, r0
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xeddf3b0f                          // vldr          d19, [pc, #60]
  .long  0xf2c3161f                          // vmov.i32      d17, #1056964608
  .long  0xe591c004                          // ldr           ip, [r1, #4]
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
  .long  0xe320f000                          // nop           {0}
  .long  0x00000000                          // andeq         r0, r0, r0
  .long  0x3f800000                          // svccc         0x00800000

.globl _sk_constant_color_vfp4
_sk_constant_color_vfp4:
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf4630a0f                          // vld1.8        {d16-d17}, [r3]
  .long  0xf3b40c20                          // vdup.32       d0, d16[0]
  .long  0xf3bc1c20                          // vdup.32       d1, d16[1]
  .long  0xf3b42c21                          // vdup.32       d2, d17[0]
  .long  0xf3bc3c21                          // vdup.32       d3, d17[1]
  .long  0xe12fff1c                          // bx            ip

.globl _sk_clear_vfp4
_sk_clear_vfp4:
  .long  0xe5913004                          // ldr           r3, [r1, #4]
  .long  0xf2800010                          // vmov.i32      d0, #0
  .long  0xf2801010                          // vmov.i32      d1, #0
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2802010                          // vmov.i32      d2, #0
  .long  0xf2803010                          // vmov.i32      d3, #0
  .long  0xe12fff13                          // bx            r3

.globl _sk_plus__vfp4
_sk_plus__vfp4:
  .long  0xf2000d04                          // vadd.f32      d0, d0, d4
  .long  0xe2813008                          // add           r3, r1, #8
  .long  0xf2011d05                          // vadd.f32      d1, d1, d5
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2022d06                          // vadd.f32      d2, d2, d6
  .long  0xe1a01003                          // mov           r1, r3
  .long  0xf2033d07                          // vadd.f32      d3, d3, d7
  .long  0xe12fff1c                          // bx            ip

.globl _sk_srcover_vfp4
_sk_srcover_vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe2813008                          // add           r3, r1, #8
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2600d83                          // vsub.f32      d16, d16, d3
  .long  0xe1a01003                          // mov           r1, r3
  .long  0xf2040c30                          // vfma.f32      d0, d4, d16
  .long  0xf2051c30                          // vfma.f32      d1, d5, d16
  .long  0xf2062c30                          // vfma.f32      d2, d6, d16
  .long  0xf2073c30                          // vfma.f32      d3, d7, d16
  .long  0xe12fff1c                          // bx            ip

.globl _sk_dstover_vfp4
_sk_dstover_vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe2813008                          // add           r3, r1, #8
  .long  0xf2651115                          // vorr          d17, d5, d5
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2604d87                          // vsub.f32      d20, d16, d7
  .long  0xe1a01003                          // mov           r1, r3
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
  .long  0xe12fff1c                          // bx            ip

.globl _sk_clamp_0_vfp4
_sk_clamp_0_vfp4:
  .long  0xf2c00010                          // vmov.i32      d16, #0
  .long  0xe5913004                          // ldr           r3, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2000f20                          // vmax.f32      d0, d0, d16
  .long  0xf2011f20                          // vmax.f32      d1, d1, d16
  .long  0xf2022f20                          // vmax.f32      d2, d2, d16
  .long  0xf2033f20                          // vmax.f32      d3, d3, d16
  .long  0xe12fff13                          // bx            r3

.globl _sk_clamp_1_vfp4
_sk_clamp_1_vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe5913004                          // ldr           r3, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2200f20                          // vmin.f32      d0, d0, d16
  .long  0xf2211f20                          // vmin.f32      d1, d1, d16
  .long  0xf2222f20                          // vmin.f32      d2, d2, d16
  .long  0xf2233f20                          // vmin.f32      d3, d3, d16
  .long  0xe12fff13                          // bx            r3

.globl _sk_clamp_a_vfp4
_sk_clamp_a_vfp4:
  .long  0xf2c70f10                          // vmov.f32      d16, #1
  .long  0xe5913004                          // ldr           r3, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2233f20                          // vmin.f32      d3, d3, d16
  .long  0xf2200f03                          // vmin.f32      d0, d0, d3
  .long  0xf2211f03                          // vmin.f32      d1, d1, d3
  .long  0xf2222f03                          // vmin.f32      d2, d2, d3
  .long  0xe12fff13                          // bx            r3

.globl _sk_set_rgb_vfp4
_sk_set_rgb_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe28e3008                          // add           r3, lr, #8
  .long  0xf4ae0c9f                          // vld1.32       {d0[]}, [lr :32]
  .long  0xf4a32c9f                          // vld1.32       {d2[]}, [r3 :32]
  .long  0xe28e3004                          // add           r3, lr, #4
  .long  0xf4a31c9f                          // vld1.32       {d1[]}, [r3 :32]
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_swap_rb_vfp4
_sk_swap_rb_vfp4:
  .long  0xeef00b40                          // vmov.f64      d16, d0
  .long  0xe5913004                          // ldr           r3, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xeeb00b42                          // vmov.f64      d0, d2
  .long  0xeeb02b60                          // vmov.f64      d2, d16
  .long  0xe12fff13                          // bx            r3

.globl _sk_swap_vfp4
_sk_swap_vfp4:
  .long  0xeef00b43                          // vmov.f64      d16, d3
  .long  0xe5913004                          // ldr           r3, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
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

.globl _sk_move_src_dst_vfp4
_sk_move_src_dst_vfp4:
  .long  0xeeb04b40                          // vmov.f64      d4, d0
  .long  0xe5913004                          // ldr           r3, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xeeb05b41                          // vmov.f64      d5, d1
  .long  0xeeb06b42                          // vmov.f64      d6, d2
  .long  0xeeb07b43                          // vmov.f64      d7, d3
  .long  0xe12fff13                          // bx            r3

.globl _sk_move_dst_src_vfp4
_sk_move_dst_src_vfp4:
  .long  0xeeb00b44                          // vmov.f64      d0, d4
  .long  0xe5913004                          // ldr           r3, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xeeb01b45                          // vmov.f64      d1, d5
  .long  0xeeb02b46                          // vmov.f64      d2, d6
  .long  0xeeb03b47                          // vmov.f64      d3, d7
  .long  0xe12fff13                          // bx            r3

.globl _sk_premul_vfp4
_sk_premul_vfp4:
  .long  0xf3000d13                          // vmul.f32      d0, d0, d3
  .long  0xe5913004                          // ldr           r3, [r1, #4]
  .long  0xf3011d13                          // vmul.f32      d1, d1, d3
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf3022d13                          // vmul.f32      d2, d2, d3
  .long  0xe12fff13                          // bx            r3

.globl _sk_unpremul_vfp4
_sk_unpremul_vfp4:
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xeeb78a00                          // vmov.f32      s16, #112
  .long  0xf3f91503                          // vceq.f32      d17, d3, #0
  .long  0xf2c00010                          // vmov.i32      d16, #0
  .long  0xe5913004                          // ldr           r3, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xeec89a23                          // vdiv.f32      s19, s16, s7
  .long  0xee889a03                          // vdiv.f32      s18, s16, s6
  .long  0xf3501199                          // vbsl          d17, d16, d9
  .long  0xf3010d90                          // vmul.f32      d0, d17, d0
  .long  0xf3011d91                          // vmul.f32      d1, d17, d1
  .long  0xf3012d92                          // vmul.f32      d2, d17, d2
  .long  0xecbd8b04                          // vpop          {d8-d9}
  .long  0xe12fff13                          // bx            r3

.globl _sk_from_srgb_vfp4
_sk_from_srgb_vfp4:
  .long  0xeddf3b22                          // vldr          d19, [pc, #136]
  .long  0xf3408d10                          // vmul.f32      d24, d0, d0
  .long  0xeddf0b1e                          // vldr          d16, [pc, #120]
  .long  0xf26341b3                          // vorr          d20, d19, d19
  .long  0xf26351b3                          // vorr          d21, d19, d19
  .long  0xeddf9b21                          // vldr          d25, [pc, #132]
  .long  0xf2404c30                          // vfma.f32      d20, d0, d16
  .long  0xeddf2b1d                          // vldr          d18, [pc, #116]
  .long  0xf2415c30                          // vfma.f32      d21, d1, d16
  .long  0xeddfcb1f                          // vldr          d28, [pc, #124]
  .long  0xf2423c30                          // vfma.f32      d19, d2, d16
  .long  0xe5913004                          // ldr           r3, [r1, #4]
  .long  0xf3426d12                          // vmul.f32      d22, d2, d2
  .long  0xe2811008                          // add           r1, r1, #8
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
  .long  0xe320f000                          // nop           {0}
  .long  0x3e99999a                          // 
  .long  0x3e99999a                          // 
  .long  0x3f328f5c                          // svccc         0x00328f5c
  .long  0x3f328f5c                          // svccc         0x00328f5c
  .long  0x3d6147ae                          // stclcc        7, cr4, [r1, #-696]!
  .long  0x3d6147ae                          // stclcc        7, cr4, [r1, #-696]!
  .long  0x3d9e8391                          // ldccc         3, cr8, [lr, #580]
  .long  0x3d9e8391                          // ldccc         3, cr8, [lr, #580]
  .long  0x3b23d70a                          // blcc          8f5fd8 <sk_linear_gradient_2stops_vfp4+0x8f51c8>
  .long  0x3b23d70a                          // blcc          8f5fdc <sk_linear_gradient_2stops_vfp4+0x8f51cc>

.globl _sk_to_srgb_vfp4
_sk_to_srgb_vfp4:
  .long  0xf3fb0582                          // vrsqrte.f32   d16, d2
  .long  0xe5913004                          // ldr           r3, [r1, #4]
  .long  0xf3fb1581                          // vrsqrte.f32   d17, d1
  .long  0xe2811008                          // add           r1, r1, #8
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
  .long  0xeddf9b22                          // vldr          d25, [pc, #136]
  .long  0xf3444dba                          // vmul.f32      d20, d20, d26
  .long  0xeddfab22                          // vldr          d26, [pc, #136]
  .long  0xf3466dbb                          // vmul.f32      d22, d22, d27
  .long  0xf26ab1ba                          // vorr          d27, d26, d26
  .long  0xf243bcb9                          // vfma.f32      d27, d19, d25
  .long  0xf26a31ba                          // vorr          d19, d26, d26
  .long  0xf2443cb9                          // vfma.f32      d19, d20, d25
  .long  0xeddf4b1e                          // vldr          d20, [pc, #120]
  .long  0xf246acb9                          // vfma.f32      d26, d22, d25
  .long  0xf3450db0                          // vmul.f32      d16, d21, d16
  .long  0xeddf5b1d                          // vldr          d21, [pc, #116]
  .long  0xf3471db1                          // vmul.f32      d17, d23, d17
  .long  0xf3482db2                          // vmul.f32      d18, d24, d18
  .long  0xf3406d35                          // vmul.f32      d22, d0, d21
  .long  0xf240bcb4                          // vfma.f32      d27, d16, d20
  .long  0xf2413cb4                          // vfma.f32      d19, d17, d20
  .long  0xf242acb4                          // vfma.f32      d26, d18, d20
  .long  0xeddf2b18                          // vldr          d18, [pc, #96]
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
  .long  0xe320f000                          // nop           {0}
  .long  0x3f306fce                          // svccc         0x00306fce
  .long  0x3f306fce                          // svccc         0x00306fce
  .long  0xbdca57a8                          // stcllt        7, cr5, [sl, #672]
  .long  0xbdca57a8                          // stcllt        7, cr5, [sl, #672]
  .long  0x3ed287c2                          // cdpcc         7, 13, cr8, cr2, cr2, {6}
  .long  0x3ed287c2                          // cdpcc         7, 13, cr8, cr2, cr2, {6}
  .long  0x41475c29                          // cmpmi         r7, r9, lsr #24
  .long  0x41475c29                          // cmpmi         r7, r9, lsr #24
  .long  0x3b8ce704                          // blcc          fe33a0f0 <sk_linear_gradient_2stops_vfp4+0xfe3392e0>
  .long  0x3b8ce704                          // blcc          fe33a0f4 <sk_linear_gradient_2stops_vfp4+0xfe3392e4>

.globl _sk_scale_1_float_vfp4
_sk_scale_1_float_vfp4:
  .long  0xed2d8b02                          // vpush         {d8}
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xed938a00                          // vldr          s16, [r3]
  .long  0xf2a00948                          // vmul.f32      d0, d0, d8[0]
  .long  0xf2a11948                          // vmul.f32      d1, d1, d8[0]
  .long  0xf2a22948                          // vmul.f32      d2, d2, d8[0]
  .long  0xf2a33948                          // vmul.f32      d3, d3, d8[0]
  .long  0xecbd8b02                          // vpop          {d8}
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}

.globl _sk_scale_u8_vfp4
_sk_scale_u8_vfp4:
  .long  0xe24dd004                          // sub           sp, sp, #4
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
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
  .long  0xeddf1b07                          // vldr          d17, [pc, #28]
  .long  0xf3400db1                          // vmul.f32      d16, d16, d17
  .long  0xf3000d90                          // vmul.f32      d0, d16, d0
  .long  0xf3001d91                          // vmul.f32      d1, d16, d1
  .long  0xf3002d92                          // vmul.f32      d2, d16, d2
  .long  0xf3003d93                          // vmul.f32      d3, d16, d3
  .long  0xe28dd004                          // add           sp, sp, #4
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}
  .long  0x3b808081                          // blcc          fe020774 <sk_linear_gradient_2stops_vfp4+0xfe01f964>
  .long  0x3b808081                          // blcc          fe020778 <sk_linear_gradient_2stops_vfp4+0xfe01f968>

.globl _sk_lerp_1_float_vfp4
_sk_lerp_1_float_vfp4:
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf2600d04                          // vsub.f32      d16, d0, d4
  .long  0xf2611d05                          // vsub.f32      d17, d1, d5
  .long  0xe591c004                          // ldr           ip, [r1, #4]
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
  .long  0xe320f000                          // nop           {0}

.globl _sk_lerp_u8_vfp4
_sk_lerp_u8_vfp4:
  .long  0xe24dd004                          // sub           sp, sp, #4
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf2602d04                          // vsub.f32      d18, d0, d4
  .long  0xe591c004                          // ldr           ip, [r1, #4]
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
  .long  0xeddf1b09                          // vldr          d17, [pc, #36]
  .long  0xf3400db1                          // vmul.f32      d16, d16, d17
  .long  0xf2611d05                          // vsub.f32      d17, d1, d5
  .long  0xf2251115                          // vorr          d1, d5, d5
  .long  0xf2020cb0                          // vfma.f32      d0, d18, d16
  .long  0xf2011cb0                          // vfma.f32      d1, d17, d16
  .long  0xf2032cb0                          // vfma.f32      d2, d19, d16
  .long  0xf2043cb0                          // vfma.f32      d3, d20, d16
  .long  0xe28dd004                          // add           sp, sp, #4
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}
  .long  0x3b808081                          // blcc          fe02083c <sk_linear_gradient_2stops_vfp4+0xfe01fa2c>
  .long  0x3b808081                          // blcc          fe020840 <sk_linear_gradient_2stops_vfp4+0xfe01fa30>

.globl _sk_lerp_565_vfp4
_sk_lerp_565_vfp4:
  .long  0xe24dd004                          // sub           sp, sp, #4
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf3c72218                          // vmov.i32      d18, #63488
  .long  0xe591c004                          // ldr           ip, [r1, #4]
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
  .long  0xeddf5b11                          // vldr          d21, [pc, #68]
  .long  0xf24401b0                          // vand          d16, d20, d16
  .long  0xeddf4b0d                          // vldr          d20, [pc, #52]
  .long  0xf3fb2622                          // vcvt.f32.s32  d18, d18
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xf3fb1621                          // vcvt.f32.s32  d17, d17
  .long  0xf3422db4                          // vmul.f32      d18, d18, d20
  .long  0xeddf4b0c                          // vldr          d20, [pc, #48]
  .long  0xf3400db5                          // vmul.f32      d16, d16, d21
  .long  0xf2625d06                          // vsub.f32      d21, d2, d6
  .long  0xf3411db4                          // vmul.f32      d17, d17, d20
  .long  0xf2262116                          // vorr          d2, d6, d6
  .long  0xf2030cb2                          // vfma.f32      d0, d19, d18
  .long  0xf2061cb0                          // vfma.f32      d1, d22, d16
  .long  0xf2052cb1                          // vfma.f32      d2, d21, d17
  .long  0xe28dd004                          // add           sp, sp, #4
  .long  0xe12fff1c                          // bx            ip
  .long  0x37842108                          // strcc         r2, [r4, r8, lsl #2]
  .long  0x37842108                          // strcc         r2, [r4, r8, lsl #2]
  .long  0x3a020821                          // bcc           82764 <sk_linear_gradient_2stops_vfp4+0x81954>
  .long  0x3a020821                          // bcc           82768 <sk_linear_gradient_2stops_vfp4+0x81958>
  .long  0x3d042108                          // stfccs        f2, [r4, #-32]
  .long  0x3d042108                          // stfccs        f2, [r4, #-32]

.globl _sk_load_tables_vfp4
_sk_load_tables_vfp4:
  .long  0xe92d48f0                          // push          {r4, r5, r6, r7, fp, lr}
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf3c7001f                          // vmov.i32      d16, #255
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe593e000                          // ldr           lr, [r3]
  .long  0xe5934004                          // ldr           r4, [r3, #4]
  .long  0xe5935008                          // ldr           r5, [r3, #8]
  .long  0xe593700c                          // ldr           r7, [r3, #12]
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
  .long  0xeddf1b0c                          // vldr          d17, [pc, #48]
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
  .long  0x3b808081                          // blcc          fe020994 <sk_linear_gradient_2stops_vfp4+0xfe01fb84>
  .long  0x3b808081                          // blcc          fe020998 <sk_linear_gradient_2stops_vfp4+0xfe01fb88>

.globl _sk_load_a8_vfp4
_sk_load_a8_vfp4:
  .long  0xe24dd004                          // sub           sp, sp, #4
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf2800010                          // vmov.i32      d0, #0
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
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
  .long  0xeddf1b02                          // vldr          d17, [pc, #8]
  .long  0xf3003db1                          // vmul.f32      d3, d16, d17
  .long  0xe28dd004                          // add           sp, sp, #4
  .long  0xe12fff1c                          // bx            ip
  .long  0x3b808081                          // blcc          fe0209ec <sk_linear_gradient_2stops_vfp4+0xfe01fbdc>
  .long  0x3b808081                          // blcc          fe0209f0 <sk_linear_gradient_2stops_vfp4+0xfe01fbe0>

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
  .long  0x437f0000                          // cmnmi         pc, #0
  .long  0x437f0000                          // cmnmi         pc, #0

.globl _sk_load_565_vfp4
_sk_load_565_vfp4:
  .long  0xe24dd004                          // sub           sp, sp, #4
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf2c1101f                          // vmov.i32      d17, #31
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf3c72218                          // vmov.i32      d18, #63488
  .long  0xeddf3b17                          // vldr          d19, [pc, #92]
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
  .long  0xeddf5b0f                          // vldr          d21, [pc, #60]
  .long  0xf24421b2                          // vand          d18, d20, d18
  .long  0xf24401b0                          // vand          d16, d20, d16
  .long  0xeddf4b0a                          // vldr          d20, [pc, #40]
  .long  0xf3fb2622                          // vcvt.f32.s32  d18, d18
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xf3fb1621                          // vcvt.f32.s32  d17, d17
  .long  0xf3020db3                          // vmul.f32      d0, d18, d19
  .long  0xf3001db4                          // vmul.f32      d1, d16, d20
  .long  0xf3012db5                          // vmul.f32      d2, d17, d21
  .long  0xe28dd004                          // add           sp, sp, #4
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}
  .long  0x37842108                          // strcc         r2, [r4, r8, lsl #2]
  .long  0x37842108                          // strcc         r2, [r4, r8, lsl #2]
  .long  0x3a020821                          // bcc           8293c <sk_linear_gradient_2stops_vfp4+0x81b2c>
  .long  0x3a020821                          // bcc           82940 <sk_linear_gradient_2stops_vfp4+0x81b30>
  .long  0x3d042108                          // stfccs        f2, [r4, #-32]
  .long  0x3d042108                          // stfccs        f2, [r4, #-32]

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
  .long  0x427c0000                          // rsbsmi        r0, ip, #0
  .long  0x427c0000                          // rsbsmi        r0, ip, #0

.globl _sk_load_8888_vfp4
_sk_load_8888_vfp4:
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf3c7001f                          // vmov.i32      d16, #255
  .long  0xe591c004                          // ldr           ip, [r1, #4]
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
  .long  0xeddf1b09                          // vldr          d17, [pc, #36]
  .long  0xf3fb2622                          // vcvt.f32.s32  d18, d18
  .long  0xf3fb4624                          // vcvt.f32.s32  d20, d20
  .long  0xf3fb3623                          // vcvt.f32.s32  d19, d19
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xf3020db1                          // vmul.f32      d0, d18, d17
  .long  0xf3043db1                          // vmul.f32      d3, d20, d17
  .long  0xf3031db1                          // vmul.f32      d1, d19, d17
  .long  0xf3002db1                          // vmul.f32      d2, d16, d17
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}
  .long  0x3b808081                          // blcc          fe020b94 <sk_linear_gradient_2stops_vfp4+0xfe01fd84>
  .long  0x3b808081                          // blcc          fe020b98 <sk_linear_gradient_2stops_vfp4+0xfe01fd88>

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
  .long  0x437f0000                          // cmnmi         pc, #0
  .long  0x437f0000                          // cmnmi         pc, #0

.globl _sk_load_f16_vfp4
_sk_load_f16_vfp4:
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe5933000                          // ldr           r3, [r3]
  .long  0xe0833180                          // add           r3, r3, r0, lsl #3
  .long  0xf463084f                          // vld2.16       {d16-d17}, [r3]
  .long  0xf3b62720                          // vcvt.f32.f16  q1, d16
  .long  0xf3b68721                          // vcvt.f32.f16  q4, d17
  .long  0xf2220112                          // vorr          d0, d2, d2
  .long  0xeef00a43                          // vmov.f32      s1, s6
  .long  0xf2281118                          // vorr          d1, d8, d8
  .long  0xeeb03a62                          // vmov.f32      s6, s5
  .long  0xeef01a49                          // vmov.f32      s3, s18
  .long  0xeeb09a68                          // vmov.f32      s18, s17
  .long  0xeeb02b43                          // vmov.f64      d2, d3
  .long  0xeeb03b49                          // vmov.f64      d3, d9
  .long  0xecbd8b04                          // vpop          {d8-d9}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_store_f16_vfp4
_sk_store_f16_vfp4:
  .long  0xeef00b41                          // vmov.f64      d16, d1
  .long  0xf2631113                          // vorr          d17, d3, d3
  .long  0xeef03b42                          // vmov.f64      d19, d2
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

.globl _sk_clamp_x_vfp4
_sk_clamp_x_vfp4:
  .long  0xf2c00010                          // vmov.i32      d16, #0
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf3c71e1f                          // vmov.i8       d17, #255
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2400f80                          // vmax.f32      d16, d16, d0
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf4e32c9f                          // vld1.32       {d18[]}, [r3 :32]
  .long  0xf26218a1                          // vadd.i32      d17, d18, d17
  .long  0xf2200fa1                          // vmin.f32      d0, d16, d17
  .long  0xe12fff1c                          // bx            ip

.globl _sk_clamp_y_vfp4
_sk_clamp_y_vfp4:
  .long  0xf2c00010                          // vmov.i32      d16, #0
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf3c71e1f                          // vmov.i8       d17, #255
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2400f81                          // vmax.f32      d16, d16, d1
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf4e32c9f                          // vld1.32       {d18[]}, [r3 :32]
  .long  0xf26218a1                          // vadd.i32      d17, d18, d17
  .long  0xf2201fa1                          // vmin.f32      d1, d16, d17
  .long  0xe12fff1c                          // bx            ip
  .long  0xe320f000                          // nop           {0}

.globl _sk_repeat_x_vfp4
_sk_repeat_x_vfp4:
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf2c02010                          // vmov.i32      d18, #0
  .long  0xeddf3b11                          // vldr          d19, [pc, #68]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
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
  .long  0x3f800000                          // svccc         0x00800000
  .long  0x3f800000                          // svccc         0x00800000

.globl _sk_repeat_y_vfp4
_sk_repeat_y_vfp4:
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf2c02010                          // vmov.i32      d18, #0
  .long  0xeddf3b11                          // vldr          d19, [pc, #68]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
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
  .long  0x3f800000                          // svccc         0x00800000
  .long  0x3f800000                          // svccc         0x00800000

.globl _sk_mirror_x_vfp4
_sk_mirror_x_vfp4:
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf2c03010                          // vmov.i32      d19, #0
  .long  0xeddf4b15                          // vldr          d20, [pc, #84]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
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
  .long  0x3f800000                          // svccc         0x00800000
  .long  0x3f800000                          // svccc         0x00800000

.globl _sk_mirror_y_vfp4
_sk_mirror_y_vfp4:
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xf2c03010                          // vmov.i32      d19, #0
  .long  0xeddf4b15                          // vldr          d20, [pc, #84]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
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
  .long  0x3f800000                          // svccc         0x00800000
  .long  0x3f800000                          // svccc         0x00800000

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
  .long  0xe28e3004                          // add           r3, lr, #4
  .long  0xf2411c32                          // vfma.f32      d17, d1, d18
  .long  0xf4ee2c9f                          // vld1.32       {d18[]}, [lr :32]
  .long  0xf4e33c9f                          // vld1.32       {d19[]}, [r3 :32]
  .long  0xf2400c32                          // vfma.f32      d16, d0, d18
  .long  0xf2401c33                          // vfma.f32      d17, d0, d19
  .long  0xf22001b0                          // vorr          d0, d16, d16
  .long  0xf22111b1                          // vorr          d1, d17, d17
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

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
  .long  0xe28e3004                          // add           r3, lr, #4
  .long  0xf2412c33                          // vfma.f32      d18, d1, d19
  .long  0xf4ee3c9f                          // vld1.32       {d19[]}, [lr :32]
  .long  0xf4e34c9f                          // vld1.32       {d20[]}, [r3 :32]
  .long  0xe28e3008                          // add           r3, lr, #8
  .long  0xf2401c33                          // vfma.f32      d17, d0, d19
  .long  0xf4e33c9f                          // vld1.32       {d19[]}, [r3 :32]
  .long  0xf2400c33                          // vfma.f32      d16, d0, d19
  .long  0xf2402c34                          // vfma.f32      d18, d0, d20
  .long  0xf22101b1                          // vorr          d0, d17, d17
  .long  0xf22021b0                          // vorr          d2, d16, d16
  .long  0xf22211b2                          // vorr          d1, d18, d18
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_matrix_perspective_vfp4
_sk_matrix_perspective_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe28e301c                          // add           r3, lr, #28
  .long  0xf4e30c9f                          // vld1.32       {d16[]}, [r3 :32]
  .long  0xe28e3020                          // add           r3, lr, #32
  .long  0xf4e31c9f                          // vld1.32       {d17[]}, [r3 :32]
  .long  0xe28e3018                          // add           r3, lr, #24
  .long  0xf2411c30                          // vfma.f32      d17, d1, d16
  .long  0xf4e30c9f                          // vld1.32       {d16[]}, [r3 :32]
  .long  0xe28e3010                          // add           r3, lr, #16
  .long  0xf2401c30                          // vfma.f32      d17, d0, d16
  .long  0xf4e30c9f                          // vld1.32       {d16[]}, [r3 :32]
  .long  0xe28e3004                          // add           r3, lr, #4
  .long  0xf4e32c9f                          // vld1.32       {d18[]}, [r3 :32]
  .long  0xe28e3008                          // add           r3, lr, #8
  .long  0xf4e34c9f                          // vld1.32       {d20[]}, [r3 :32]
  .long  0xe28e3014                          // add           r3, lr, #20
  .long  0xf2414c32                          // vfma.f32      d20, d1, d18
  .long  0xf4e32c9f                          // vld1.32       {d18[]}, [r3 :32]
  .long  0xe28e300c                          // add           r3, lr, #12
  .long  0xf3fb3521                          // vrecpe.f32    d19, d17
  .long  0xf2412c30                          // vfma.f32      d18, d1, d16
  .long  0xf4e35c9f                          // vld1.32       {d21[]}, [r3 :32]
  .long  0xf2410fb3                          // vrecps.f32    d16, d17, d19
  .long  0xf4ee1c9f                          // vld1.32       {d17[]}, [lr :32]
  .long  0xf2404c31                          // vfma.f32      d20, d0, d17
  .long  0xf2402c35                          // vfma.f32      d18, d0, d21
  .long  0xf3430db0                          // vmul.f32      d16, d19, d16
  .long  0xf3040db0                          // vmul.f32      d0, d20, d16
  .long  0xf3021db0                          // vmul.f32      d1, d18, d16
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_linear_gradient_2stops_vfp4
_sk_linear_gradient_2stops_vfp4:
  .long  0xe5913000                          // ldr           r3, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
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
.balign 16

.globl _sk_start_pipeline_hsw
_sk_start_pipeline_hsw:
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
  .byte  72,141,67,8                         // lea           0x8(%rbx),%rax
  .byte  76,57,248                           // cmp           %r15,%rax
  .byte  118,5                               // jbe           28 <_sk_start_pipeline_hsw+0x28>
  .byte  72,137,216                          // mov           %rbx,%rax
  .byte  235,60                              // jmp           64 <_sk_start_pipeline_hsw+0x64>
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  197,228,87,219                      // vxorps        %ymm3,%ymm3,%ymm3
  .byte  197,220,87,228                      // vxorps        %ymm4,%ymm4,%ymm4
  .byte  197,212,87,237                      // vxorps        %ymm5,%ymm5,%ymm5
  .byte  197,204,87,246                      // vxorps        %ymm6,%ymm6,%ymm6
  .byte  197,196,87,255                      // vxorps        %ymm7,%ymm7,%ymm7
  .byte  72,137,223                          // mov           %rbx,%rdi
  .byte  76,137,238                          // mov           %r13,%rsi
  .byte  76,137,242                          // mov           %r14,%rdx
  .byte  65,255,212                          // callq         *%r12
  .byte  72,141,67,8                         // lea           0x8(%rbx),%rax
  .byte  72,131,195,16                       // add           $0x10,%rbx
  .byte  76,57,251                           // cmp           %r15,%rbx
  .byte  72,137,195                          // mov           %rax,%rbx
  .byte  118,196                             // jbe           28 <_sk_start_pipeline_hsw+0x28>
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,93                               // pop           %r13
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  197,248,119                         // vzeroupper
  .byte  195                                 // retq

.globl _sk_just_return_hsw
_sk_just_return_hsw:
  .byte  195                                 // retq

.globl _sk_seed_shader_hsw
_sk_seed_shader_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,249,110,199                     // vmovd         %edi,%xmm0
  .byte  196,226,125,24,192                  // vbroadcastss  %xmm0,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  196,226,125,24,13,162,10,0,0        // vbroadcastss  0xaa2(%rip),%ymm1        # b2c <_sk_linear_gradient_2stops_hsw+0x4e>
  .byte  197,252,88,193                      // vaddps        %ymm1,%ymm0,%ymm0
  .byte  197,252,88,5,74,11,0,0              // vaddps        0xb4a(%rip),%ymm0,%ymm0        # be0 <_sk_linear_gradient_2stops_hsw+0x102>
  .byte  196,226,125,24,16                   // vbroadcastss  (%rax),%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  197,236,88,201                      // vaddps        %ymm1,%ymm2,%ymm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,226,125,24,21,130,10,0,0        // vbroadcastss  0xa82(%rip),%ymm2        # b30 <_sk_linear_gradient_2stops_hsw+0x52>
  .byte  197,228,87,219                      // vxorps        %ymm3,%ymm3,%ymm3
  .byte  197,220,87,228                      // vxorps        %ymm4,%ymm4,%ymm4
  .byte  197,212,87,237                      // vxorps        %ymm5,%ymm5,%ymm5
  .byte  197,204,87,246                      // vxorps        %ymm6,%ymm6,%ymm6
  .byte  197,196,87,255                      // vxorps        %ymm7,%ymm7,%ymm7
  .byte  255,224                             // jmpq          *%rax

.globl _sk_constant_color_hsw
_sk_constant_color_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,226,125,24,0                    // vbroadcastss  (%rax),%ymm0
  .byte  196,226,125,24,72,4                 // vbroadcastss  0x4(%rax),%ymm1
  .byte  196,226,125,24,80,8                 // vbroadcastss  0x8(%rax),%ymm2
  .byte  196,226,125,24,88,12                // vbroadcastss  0xc(%rax),%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_clear_hsw
_sk_clear_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  197,228,87,219                      // vxorps        %ymm3,%ymm3,%ymm3
  .byte  255,224                             // jmpq          *%rax

.globl _sk_plus__hsw
_sk_plus__hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,88,196                      // vaddps        %ymm4,%ymm0,%ymm0
  .byte  197,244,88,205                      // vaddps        %ymm5,%ymm1,%ymm1
  .byte  197,236,88,214                      // vaddps        %ymm6,%ymm2,%ymm2
  .byte  197,228,88,223                      // vaddps        %ymm7,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_srcover_hsw
_sk_srcover_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,5,28,10,0,0           // vbroadcastss  0xa1c(%rip),%ymm8        # b34 <_sk_linear_gradient_2stops_hsw+0x56>
  .byte  197,60,92,195                       // vsubps        %ymm3,%ymm8,%ymm8
  .byte  196,194,93,184,192                  // vfmadd231ps   %ymm8,%ymm4,%ymm0
  .byte  196,194,85,184,200                  // vfmadd231ps   %ymm8,%ymm5,%ymm1
  .byte  196,194,77,184,208                  // vfmadd231ps   %ymm8,%ymm6,%ymm2
  .byte  196,194,69,184,216                  // vfmadd231ps   %ymm8,%ymm7,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_dstover_hsw
_sk_dstover_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,5,249,9,0,0           // vbroadcastss  0x9f9(%rip),%ymm8        # b38 <_sk_linear_gradient_2stops_hsw+0x5a>
  .byte  197,60,92,199                       // vsubps        %ymm7,%ymm8,%ymm8
  .byte  196,226,61,168,196                  // vfmadd213ps   %ymm4,%ymm8,%ymm0
  .byte  196,226,61,168,205                  // vfmadd213ps   %ymm5,%ymm8,%ymm1
  .byte  196,226,61,168,214                  // vfmadd213ps   %ymm6,%ymm8,%ymm2
  .byte  196,226,61,168,223                  // vfmadd213ps   %ymm7,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_clamp_0_hsw
_sk_clamp_0_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  196,193,124,95,192                  // vmaxps        %ymm8,%ymm0,%ymm0
  .byte  196,193,116,95,200                  // vmaxps        %ymm8,%ymm1,%ymm1
  .byte  196,193,108,95,208                  // vmaxps        %ymm8,%ymm2,%ymm2
  .byte  196,193,100,95,216                  // vmaxps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_clamp_1_hsw
_sk_clamp_1_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,5,183,9,0,0           // vbroadcastss  0x9b7(%rip),%ymm8        # b3c <_sk_linear_gradient_2stops_hsw+0x5e>
  .byte  196,193,124,93,192                  // vminps        %ymm8,%ymm0,%ymm0
  .byte  196,193,116,93,200                  // vminps        %ymm8,%ymm1,%ymm1
  .byte  196,193,108,93,208                  // vminps        %ymm8,%ymm2,%ymm2
  .byte  196,193,100,93,216                  // vminps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_clamp_a_hsw
_sk_clamp_a_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,5,152,9,0,0           // vbroadcastss  0x998(%rip),%ymm8        # b40 <_sk_linear_gradient_2stops_hsw+0x62>
  .byte  196,193,100,93,216                  // vminps        %ymm8,%ymm3,%ymm3
  .byte  197,252,93,195                      // vminps        %ymm3,%ymm0,%ymm0
  .byte  197,244,93,203                      // vminps        %ymm3,%ymm1,%ymm1
  .byte  197,236,93,211                      // vminps        %ymm3,%ymm2,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_set_rgb_hsw
_sk_set_rgb_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,226,125,24,0                    // vbroadcastss  (%rax),%ymm0
  .byte  196,226,125,24,72,4                 // vbroadcastss  0x4(%rax),%ymm1
  .byte  196,226,125,24,80,8                 // vbroadcastss  0x8(%rax),%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_swap_rb_hsw
_sk_swap_rb_hsw:
  .byte  197,124,40,192                      // vmovaps       %ymm0,%ymm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,40,194                      // vmovaps       %ymm2,%ymm0
  .byte  197,124,41,194                      // vmovaps       %ymm8,%ymm2
  .byte  255,224                             // jmpq          *%rax

.globl _sk_swap_hsw
_sk_swap_hsw:
  .byte  197,124,40,195                      // vmovaps       %ymm3,%ymm8
  .byte  197,124,40,202                      // vmovaps       %ymm2,%ymm9
  .byte  197,124,40,209                      // vmovaps       %ymm1,%ymm10
  .byte  197,124,40,216                      // vmovaps       %ymm0,%ymm11
  .byte  72,173                              // lods          %ds:(%rsi),%rax
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

.globl _sk_move_src_dst_hsw
_sk_move_src_dst_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,40,224                      // vmovaps       %ymm0,%ymm4
  .byte  197,252,40,233                      // vmovaps       %ymm1,%ymm5
  .byte  197,252,40,242                      // vmovaps       %ymm2,%ymm6
  .byte  197,252,40,251                      // vmovaps       %ymm3,%ymm7
  .byte  255,224                             // jmpq          *%rax

.globl _sk_move_dst_src_hsw
_sk_move_dst_src_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,40,196                      // vmovaps       %ymm4,%ymm0
  .byte  197,252,40,205                      // vmovaps       %ymm5,%ymm1
  .byte  197,252,40,214                      // vmovaps       %ymm6,%ymm2
  .byte  197,252,40,223                      // vmovaps       %ymm7,%ymm3
  .byte  255,224                             // jmpq          *%rax

.globl _sk_premul_hsw
_sk_premul_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,89,195                      // vmulps        %ymm3,%ymm0,%ymm0
  .byte  197,244,89,203                      // vmulps        %ymm3,%ymm1,%ymm1
  .byte  197,236,89,211                      // vmulps        %ymm3,%ymm2,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_unpremul_hsw
_sk_unpremul_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  196,65,100,194,200,0                // vcmpeqps      %ymm8,%ymm3,%ymm9
  .byte  196,98,125,24,21,212,8,0,0          // vbroadcastss  0x8d4(%rip),%ymm10        # b44 <_sk_linear_gradient_2stops_hsw+0x66>
  .byte  197,44,94,211                       // vdivps        %ymm3,%ymm10,%ymm10
  .byte  196,67,45,74,192,144                // vblendvps     %ymm9,%ymm8,%ymm10,%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_from_srgb_hsw
_sk_from_srgb_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,5,179,8,0,0           // vbroadcastss  0x8b3(%rip),%ymm8        # b48 <_sk_linear_gradient_2stops_hsw+0x6a>
  .byte  196,65,124,89,200                   // vmulps        %ymm8,%ymm0,%ymm9
  .byte  197,124,89,208                      // vmulps        %ymm0,%ymm0,%ymm10
  .byte  196,98,125,24,29,165,8,0,0          // vbroadcastss  0x8a5(%rip),%ymm11        # b4c <_sk_linear_gradient_2stops_hsw+0x6e>
  .byte  196,98,125,24,37,160,8,0,0          // vbroadcastss  0x8a0(%rip),%ymm12        # b50 <_sk_linear_gradient_2stops_hsw+0x72>
  .byte  196,65,124,40,236                   // vmovaps       %ymm12,%ymm13
  .byte  196,66,125,168,235                  // vfmadd213ps   %ymm11,%ymm0,%ymm13
  .byte  196,98,125,24,53,145,8,0,0          // vbroadcastss  0x891(%rip),%ymm14        # b54 <_sk_linear_gradient_2stops_hsw+0x76>
  .byte  196,66,45,168,238                   // vfmadd213ps   %ymm14,%ymm10,%ymm13
  .byte  196,98,125,24,21,135,8,0,0          // vbroadcastss  0x887(%rip),%ymm10        # b58 <_sk_linear_gradient_2stops_hsw+0x7a>
  .byte  196,193,124,194,194,1               // vcmpltps      %ymm10,%ymm0,%ymm0
  .byte  196,195,21,74,193,0                 // vblendvps     %ymm0,%ymm9,%ymm13,%ymm0
  .byte  196,65,116,89,200                   // vmulps        %ymm8,%ymm1,%ymm9
  .byte  197,116,89,233                      // vmulps        %ymm1,%ymm1,%ymm13
  .byte  196,65,124,40,252                   // vmovaps       %ymm12,%ymm15
  .byte  196,66,117,168,251                  // vfmadd213ps   %ymm11,%ymm1,%ymm15
  .byte  196,66,21,168,254                   // vfmadd213ps   %ymm14,%ymm13,%ymm15
  .byte  196,193,116,194,202,1               // vcmpltps      %ymm10,%ymm1,%ymm1
  .byte  196,195,5,74,201,16                 // vblendvps     %ymm1,%ymm9,%ymm15,%ymm1
  .byte  196,65,108,89,192                   // vmulps        %ymm8,%ymm2,%ymm8
  .byte  197,108,89,202                      // vmulps        %ymm2,%ymm2,%ymm9
  .byte  196,66,109,168,227                  // vfmadd213ps   %ymm11,%ymm2,%ymm12
  .byte  196,66,53,168,230                   // vfmadd213ps   %ymm14,%ymm9,%ymm12
  .byte  196,193,108,194,210,1               // vcmpltps      %ymm10,%ymm2,%ymm2
  .byte  196,195,29,74,208,32                // vblendvps     %ymm2,%ymm8,%ymm12,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_to_srgb_hsw
_sk_to_srgb_hsw:
  .byte  197,124,82,192                      // vrsqrtps      %ymm0,%ymm8
  .byte  196,65,124,83,200                   // vrcpps        %ymm8,%ymm9
  .byte  196,65,124,82,208                   // vrsqrtps      %ymm8,%ymm10
  .byte  196,98,125,24,5,33,8,0,0            // vbroadcastss  0x821(%rip),%ymm8        # b5c <_sk_linear_gradient_2stops_hsw+0x7e>
  .byte  196,65,124,89,216                   // vmulps        %ymm8,%ymm0,%ymm11
  .byte  196,98,125,24,37,23,8,0,0           // vbroadcastss  0x817(%rip),%ymm12        # b60 <_sk_linear_gradient_2stops_hsw+0x82>
  .byte  196,98,125,24,45,18,8,0,0           // vbroadcastss  0x812(%rip),%ymm13        # b64 <_sk_linear_gradient_2stops_hsw+0x86>
  .byte  196,66,21,168,204                   // vfmadd213ps   %ymm12,%ymm13,%ymm9
  .byte  196,98,125,24,53,8,8,0,0            // vbroadcastss  0x808(%rip),%ymm14        # b68 <_sk_linear_gradient_2stops_hsw+0x8a>
  .byte  196,66,13,184,202                   // vfmadd231ps   %ymm10,%ymm14,%ymm9
  .byte  196,98,125,24,21,254,7,0,0          // vbroadcastss  0x7fe(%rip),%ymm10        # b6c <_sk_linear_gradient_2stops_hsw+0x8e>
  .byte  196,65,44,93,201                    // vminps        %ymm9,%ymm10,%ymm9
  .byte  196,98,125,24,61,244,7,0,0          // vbroadcastss  0x7f4(%rip),%ymm15        # b70 <_sk_linear_gradient_2stops_hsw+0x92>
  .byte  196,193,124,194,199,1               // vcmpltps      %ymm15,%ymm0,%ymm0
  .byte  196,195,53,74,195,0                 // vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
  .byte  197,124,82,201                      // vrsqrtps      %ymm1,%ymm9
  .byte  196,65,124,83,217                   // vrcpps        %ymm9,%ymm11
  .byte  196,65,124,82,201                   // vrsqrtps      %ymm9,%ymm9
  .byte  196,66,21,168,220                   // vfmadd213ps   %ymm12,%ymm13,%ymm11
  .byte  196,66,13,184,217                   // vfmadd231ps   %ymm9,%ymm14,%ymm11
  .byte  196,65,116,89,200                   // vmulps        %ymm8,%ymm1,%ymm9
  .byte  196,65,44,93,219                    // vminps        %ymm11,%ymm10,%ymm11
  .byte  196,193,116,194,207,1               // vcmpltps      %ymm15,%ymm1,%ymm1
  .byte  196,195,37,74,201,16                // vblendvps     %ymm1,%ymm9,%ymm11,%ymm1
  .byte  197,124,82,202                      // vrsqrtps      %ymm2,%ymm9
  .byte  196,65,124,83,217                   // vrcpps        %ymm9,%ymm11
  .byte  196,66,21,168,220                   // vfmadd213ps   %ymm12,%ymm13,%ymm11
  .byte  196,65,124,82,201                   // vrsqrtps      %ymm9,%ymm9
  .byte  196,66,13,184,217                   // vfmadd231ps   %ymm9,%ymm14,%ymm11
  .byte  196,65,44,93,203                    // vminps        %ymm11,%ymm10,%ymm9
  .byte  196,65,108,89,192                   // vmulps        %ymm8,%ymm2,%ymm8
  .byte  196,193,108,194,215,1               // vcmpltps      %ymm15,%ymm2,%ymm2
  .byte  196,195,53,74,208,32                // vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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

.globl _sk_scale_u8_hsw
_sk_scale_u8_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,98,125,49,4,56                  // vpmovzxbd     (%rax,%rdi,1),%ymm8
  .byte  196,65,124,91,192                   // vcvtdq2ps     %ymm8,%ymm8
  .byte  196,98,125,24,13,86,7,0,0           // vbroadcastss  0x756(%rip),%ymm9        # b74 <_sk_linear_gradient_2stops_hsw+0x96>
  .byte  196,65,60,89,193                    // vmulps        %ymm9,%ymm8,%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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

.globl _sk_lerp_u8_hsw
_sk_lerp_u8_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,98,125,49,4,56                  // vpmovzxbd     (%rax,%rdi,1),%ymm8
  .byte  196,65,124,91,192                   // vcvtdq2ps     %ymm8,%ymm8
  .byte  196,98,125,24,13,249,6,0,0          // vbroadcastss  0x6f9(%rip),%ymm9        # b78 <_sk_linear_gradient_2stops_hsw+0x9a>
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
  .byte  255,224                             // jmpq          *%rax

.globl _sk_lerp_565_hsw
_sk_lerp_565_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,226,125,51,28,120               // vpmovzxwd     (%rax,%rdi,2),%ymm3
  .byte  196,98,125,88,5,188,6,0,0           // vpbroadcastd  0x6bc(%rip),%ymm8        # b7c <_sk_linear_gradient_2stops_hsw+0x9e>
  .byte  196,65,101,219,192                  // vpand         %ymm8,%ymm3,%ymm8
  .byte  196,65,124,91,192                   // vcvtdq2ps     %ymm8,%ymm8
  .byte  196,98,125,24,13,173,6,0,0          // vbroadcastss  0x6ad(%rip),%ymm9        # b80 <_sk_linear_gradient_2stops_hsw+0xa2>
  .byte  196,65,60,89,193                    // vmulps        %ymm9,%ymm8,%ymm8
  .byte  196,98,125,88,13,163,6,0,0          // vpbroadcastd  0x6a3(%rip),%ymm9        # b84 <_sk_linear_gradient_2stops_hsw+0xa6>
  .byte  196,65,101,219,201                  // vpand         %ymm9,%ymm3,%ymm9
  .byte  196,65,124,91,201                   // vcvtdq2ps     %ymm9,%ymm9
  .byte  196,98,125,24,21,148,6,0,0          // vbroadcastss  0x694(%rip),%ymm10        # b88 <_sk_linear_gradient_2stops_hsw+0xaa>
  .byte  196,65,52,89,202                    // vmulps        %ymm10,%ymm9,%ymm9
  .byte  196,98,125,88,21,138,6,0,0          // vpbroadcastd  0x68a(%rip),%ymm10        # b8c <_sk_linear_gradient_2stops_hsw+0xae>
  .byte  196,193,101,219,218                 // vpand         %ymm10,%ymm3,%ymm3
  .byte  197,252,91,219                      // vcvtdq2ps     %ymm3,%ymm3
  .byte  196,98,125,24,21,124,6,0,0          // vbroadcastss  0x67c(%rip),%ymm10        # b90 <_sk_linear_gradient_2stops_hsw+0xb2>
  .byte  196,193,100,89,218                  // vmulps        %ymm10,%ymm3,%ymm3
  .byte  197,252,92,196                      // vsubps        %ymm4,%ymm0,%ymm0
  .byte  196,226,61,168,196                  // vfmadd213ps   %ymm4,%ymm8,%ymm0
  .byte  197,244,92,205                      // vsubps        %ymm5,%ymm1,%ymm1
  .byte  196,226,53,168,205                  // vfmadd213ps   %ymm5,%ymm9,%ymm1
  .byte  197,236,92,214                      // vsubps        %ymm6,%ymm2,%ymm2
  .byte  196,226,101,168,214                 // vfmadd213ps   %ymm6,%ymm3,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,226,125,24,29,85,6,0,0          // vbroadcastss  0x655(%rip),%ymm3        # b94 <_sk_linear_gradient_2stops_hsw+0xb6>
  .byte  255,224                             // jmpq          *%rax

.globl _sk_load_tables_hsw
_sk_load_tables_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,8                            // mov           (%rax),%rcx
  .byte  76,139,64,8                         // mov           0x8(%rax),%r8
  .byte  197,252,16,28,185                   // vmovups       (%rcx,%rdi,4),%ymm3
  .byte  197,252,40,21,169,6,0,0             // vmovaps       0x6a9(%rip),%ymm2        # c00 <_sk_linear_gradient_2stops_hsw+0x122>
  .byte  197,228,84,202                      // vandps        %ymm2,%ymm3,%ymm1
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,124,194,192,0                   // vcmpeqps      %ymm0,%ymm0,%ymm8
  .byte  196,65,124,40,200                   // vmovaps       %ymm8,%ymm9
  .byte  196,194,53,146,4,136                // vgatherdps    %ymm9,(%r8,%ymm1,4),%ymm0
  .byte  72,139,72,16                        // mov           0x10(%rax),%rcx
  .byte  197,245,114,211,8                   // vpsrld        $0x8,%ymm3,%ymm1
  .byte  197,117,219,202                     // vpand         %ymm2,%ymm1,%ymm9
  .byte  196,65,124,40,208                   // vmovaps       %ymm8,%ymm10
  .byte  196,162,45,146,12,137               // vgatherdps    %ymm10,(%rcx,%ymm9,4),%ymm1
  .byte  72,139,64,24                        // mov           0x18(%rax),%rax
  .byte  197,181,114,211,16                  // vpsrld        $0x10,%ymm3,%ymm9
  .byte  197,53,219,202                      // vpand         %ymm2,%ymm9,%ymm9
  .byte  196,162,61,146,20,136               // vgatherdps    %ymm8,(%rax,%ymm9,4),%ymm2
  .byte  197,229,114,211,24                  // vpsrld        $0x18,%ymm3,%ymm3
  .byte  197,252,91,219                      // vcvtdq2ps     %ymm3,%ymm3
  .byte  196,98,125,24,5,236,5,0,0           // vbroadcastss  0x5ec(%rip),%ymm8        # b98 <_sk_linear_gradient_2stops_hsw+0xba>
  .byte  196,193,100,89,216                  // vmulps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_load_a8_hsw
_sk_load_a8_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,226,125,49,4,56                 // vpmovzxbd     (%rax,%rdi,1),%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  196,226,125,24,13,207,5,0,0         // vbroadcastss  0x5cf(%rip),%ymm1        # b9c <_sk_linear_gradient_2stops_hsw+0xbe>
  .byte  197,252,89,217                      // vmulps        %ymm1,%ymm0,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  255,224                             // jmpq          *%rax

.globl _sk_store_a8_hsw
_sk_store_a8_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,98,125,24,5,177,5,0,0           // vbroadcastss  0x5b1(%rip),%ymm8        # ba0 <_sk_linear_gradient_2stops_hsw+0xc2>
  .byte  196,65,100,89,192                   // vmulps        %ymm8,%ymm3,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,67,125,25,193,1                 // vextractf128  $0x1,%ymm8,%xmm9
  .byte  196,66,57,43,193                    // vpackusdw     %xmm9,%xmm8,%xmm8
  .byte  196,65,57,103,192                   // vpackuswb     %xmm8,%xmm8,%xmm8
  .byte  197,121,214,4,56                    // vmovq         %xmm8,(%rax,%rdi,1)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_load_565_hsw
_sk_load_565_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,226,125,51,20,120               // vpmovzxwd     (%rax,%rdi,2),%ymm2
  .byte  196,226,125,88,5,126,5,0,0          // vpbroadcastd  0x57e(%rip),%ymm0        # ba4 <_sk_linear_gradient_2stops_hsw+0xc6>
  .byte  197,237,219,192                     // vpand         %ymm0,%ymm2,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  196,226,125,24,13,113,5,0,0         // vbroadcastss  0x571(%rip),%ymm1        # ba8 <_sk_linear_gradient_2stops_hsw+0xca>
  .byte  197,252,89,193                      // vmulps        %ymm1,%ymm0,%ymm0
  .byte  196,226,125,88,13,104,5,0,0         // vpbroadcastd  0x568(%rip),%ymm1        # bac <_sk_linear_gradient_2stops_hsw+0xce>
  .byte  197,237,219,201                     // vpand         %ymm1,%ymm2,%ymm1
  .byte  197,252,91,201                      // vcvtdq2ps     %ymm1,%ymm1
  .byte  196,226,125,24,29,91,5,0,0          // vbroadcastss  0x55b(%rip),%ymm3        # bb0 <_sk_linear_gradient_2stops_hsw+0xd2>
  .byte  197,244,89,203                      // vmulps        %ymm3,%ymm1,%ymm1
  .byte  196,226,125,88,29,82,5,0,0          // vpbroadcastd  0x552(%rip),%ymm3        # bb4 <_sk_linear_gradient_2stops_hsw+0xd6>
  .byte  197,237,219,211                     // vpand         %ymm3,%ymm2,%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  196,226,125,24,29,69,5,0,0          // vbroadcastss  0x545(%rip),%ymm3        # bb8 <_sk_linear_gradient_2stops_hsw+0xda>
  .byte  197,236,89,211                      // vmulps        %ymm3,%ymm2,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,226,125,24,29,58,5,0,0          // vbroadcastss  0x53a(%rip),%ymm3        # bbc <_sk_linear_gradient_2stops_hsw+0xde>
  .byte  255,224                             // jmpq          *%rax

.globl _sk_store_565_hsw
_sk_store_565_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,98,125,24,5,46,5,0,0            // vbroadcastss  0x52e(%rip),%ymm8        # bc0 <_sk_linear_gradient_2stops_hsw+0xe2>
  .byte  196,65,124,89,200                   // vmulps        %ymm8,%ymm0,%ymm9
  .byte  196,65,125,91,201                   // vcvtps2dq     %ymm9,%ymm9
  .byte  196,193,53,114,241,11               // vpslld        $0xb,%ymm9,%ymm9
  .byte  196,98,125,24,21,25,5,0,0           // vbroadcastss  0x519(%rip),%ymm10        # bc4 <_sk_linear_gradient_2stops_hsw+0xe6>
  .byte  196,65,116,89,210                   // vmulps        %ymm10,%ymm1,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,45,114,242,5                // vpslld        $0x5,%ymm10,%ymm10
  .byte  196,65,45,235,201                   // vpor          %ymm9,%ymm10,%ymm9
  .byte  196,65,108,89,192                   // vmulps        %ymm8,%ymm2,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,65,53,235,192                   // vpor          %ymm8,%ymm9,%ymm8
  .byte  196,67,125,57,193,1                 // vextracti128  $0x1,%ymm8,%xmm9
  .byte  196,66,57,43,193                    // vpackusdw     %xmm9,%xmm8,%xmm8
  .byte  197,122,127,4,120                   // vmovdqu       %xmm8,(%rax,%rdi,2)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_load_8888_hsw
_sk_load_8888_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  197,252,16,28,184                   // vmovups       (%rax,%rdi,4),%ymm3
  .byte  197,252,40,21,43,5,0,0              // vmovaps       0x52b(%rip),%ymm2        # c20 <_sk_linear_gradient_2stops_hsw+0x142>
  .byte  197,228,84,194                      // vandps        %ymm2,%ymm3,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  196,98,125,24,5,194,4,0,0           // vbroadcastss  0x4c2(%rip),%ymm8        # bc8 <_sk_linear_gradient_2stops_hsw+0xea>
  .byte  196,193,124,89,192                  // vmulps        %ymm8,%ymm0,%ymm0
  .byte  197,245,114,211,8                   // vpsrld        $0x8,%ymm3,%ymm1
  .byte  197,245,219,202                     // vpand         %ymm2,%ymm1,%ymm1
  .byte  197,252,91,201                      // vcvtdq2ps     %ymm1,%ymm1
  .byte  196,193,116,89,200                  // vmulps        %ymm8,%ymm1,%ymm1
  .byte  197,181,114,211,16                  // vpsrld        $0x10,%ymm3,%ymm9
  .byte  197,181,219,210                     // vpand         %ymm2,%ymm9,%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  196,193,108,89,208                  // vmulps        %ymm8,%ymm2,%ymm2
  .byte  197,229,114,211,24                  // vpsrld        $0x18,%ymm3,%ymm3
  .byte  197,252,91,219                      // vcvtdq2ps     %ymm3,%ymm3
  .byte  196,193,100,89,216                  // vmulps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_store_8888_hsw
_sk_store_8888_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,98,125,24,5,125,4,0,0           // vbroadcastss  0x47d(%rip),%ymm8        # bcc <_sk_linear_gradient_2stops_hsw+0xee>
  .byte  196,65,124,89,200                   // vmulps        %ymm8,%ymm0,%ymm9
  .byte  196,65,125,91,201                   // vcvtps2dq     %ymm9,%ymm9
  .byte  196,65,116,89,208                   // vmulps        %ymm8,%ymm1,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,45,114,242,8                // vpslld        $0x8,%ymm10,%ymm10
  .byte  196,65,45,235,201                   // vpor          %ymm9,%ymm10,%ymm9
  .byte  196,65,108,89,208                   // vmulps        %ymm8,%ymm2,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,45,114,242,16               // vpslld        $0x10,%ymm10,%ymm10
  .byte  196,65,100,89,192                   // vmulps        %ymm8,%ymm3,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,193,61,114,240,24               // vpslld        $0x18,%ymm8,%ymm8
  .byte  196,65,45,235,192                   // vpor          %ymm8,%ymm10,%ymm8
  .byte  196,65,53,235,192                   // vpor          %ymm8,%ymm9,%ymm8
  .byte  197,126,127,4,184                   // vmovdqu       %ymm8,(%rax,%rdi,4)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_load_f16_hsw
_sk_load_f16_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  197,250,111,4,248                   // vmovdqu       (%rax,%rdi,8),%xmm0
  .byte  197,250,111,76,248,16               // vmovdqu       0x10(%rax,%rdi,8),%xmm1
  .byte  197,250,111,84,248,32               // vmovdqu       0x20(%rax,%rdi,8),%xmm2
  .byte  197,250,111,92,248,48               // vmovdqu       0x30(%rax,%rdi,8),%xmm3
  .byte  197,121,97,193                      // vpunpcklwd    %xmm1,%xmm0,%xmm8
  .byte  197,249,105,193                     // vpunpckhwd    %xmm1,%xmm0,%xmm0
  .byte  197,233,97,203                      // vpunpcklwd    %xmm3,%xmm2,%xmm1
  .byte  197,233,105,211                     // vpunpckhwd    %xmm3,%xmm2,%xmm2
  .byte  197,57,97,200                       // vpunpcklwd    %xmm0,%xmm8,%xmm9
  .byte  197,57,105,192                      // vpunpckhwd    %xmm0,%xmm8,%xmm8
  .byte  197,241,97,218                      // vpunpcklwd    %xmm2,%xmm1,%xmm3
  .byte  197,113,105,210                     // vpunpckhwd    %xmm2,%xmm1,%xmm10
  .byte  197,177,108,195                     // vpunpcklqdq   %xmm3,%xmm9,%xmm0
  .byte  196,226,125,19,192                  // vcvtph2ps     %xmm0,%ymm0
  .byte  197,177,109,203                     // vpunpckhqdq   %xmm3,%xmm9,%xmm1
  .byte  196,226,125,19,201                  // vcvtph2ps     %xmm1,%ymm1
  .byte  196,193,57,108,210                  // vpunpcklqdq   %xmm10,%xmm8,%xmm2
  .byte  196,226,125,19,210                  // vcvtph2ps     %xmm2,%ymm2
  .byte  196,193,57,109,218                  // vpunpckhqdq   %xmm10,%xmm8,%xmm3
  .byte  196,226,125,19,219                  // vcvtph2ps     %xmm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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
  .byte  196,65,41,105,211                   // vpunpckhwd    %xmm11,%xmm10,%xmm10
  .byte  196,65,25,98,217                    // vpunpckldq    %xmm9,%xmm12,%xmm11
  .byte  197,122,127,28,248                  // vmovdqu       %xmm11,(%rax,%rdi,8)
  .byte  196,65,25,106,201                   // vpunpckhdq    %xmm9,%xmm12,%xmm9
  .byte  197,122,127,76,248,16               // vmovdqu       %xmm9,0x10(%rax,%rdi,8)
  .byte  196,65,57,98,202                    // vpunpckldq    %xmm10,%xmm8,%xmm9
  .byte  197,122,127,76,248,32               // vmovdqu       %xmm9,0x20(%rax,%rdi,8)
  .byte  196,65,57,106,194                   // vpunpckhdq    %xmm10,%xmm8,%xmm8
  .byte  197,122,127,68,248,48               // vmovdqu       %xmm8,0x30(%rax,%rdi,8)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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

.globl _sk_repeat_x_hsw
_sk_repeat_x_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  196,65,124,94,200                   // vdivps        %ymm8,%ymm0,%ymm9
  .byte  196,67,125,8,201,1                  // vroundps      $0x1,%ymm9,%ymm9
  .byte  196,65,52,89,200                    // vmulps        %ymm8,%ymm9,%ymm9
  .byte  196,193,124,92,193                  // vsubps        %ymm9,%ymm0,%ymm0
  .byte  196,65,53,118,201                   // vpcmpeqd      %ymm9,%ymm9,%ymm9
  .byte  196,65,61,254,193                   // vpaddd        %ymm9,%ymm8,%ymm8
  .byte  196,193,124,93,192                  // vminps        %ymm8,%ymm0,%ymm0
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_repeat_y_hsw
_sk_repeat_y_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,0                     // vbroadcastss  (%rax),%ymm8
  .byte  196,65,116,94,200                   // vdivps        %ymm8,%ymm1,%ymm9
  .byte  196,67,125,8,201,1                  // vroundps      $0x1,%ymm9,%ymm9
  .byte  196,65,52,89,200                    // vmulps        %ymm8,%ymm9,%ymm9
  .byte  196,193,116,92,201                  // vsubps        %ymm9,%ymm1,%ymm1
  .byte  196,65,53,118,201                   // vpcmpeqd      %ymm9,%ymm9,%ymm9
  .byte  196,65,61,254,193                   // vpaddd        %ymm9,%ymm8,%ymm8
  .byte  196,193,116,93,200                  // vminps        %ymm8,%ymm1,%ymm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,172,92,192                      // vsubps        %ymm0,%ymm10,%ymm0
  .byte  196,193,124,92,193                  // vsubps        %ymm9,%ymm0,%ymm0
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  197,60,92,192                       // vsubps        %ymm0,%ymm8,%ymm8
  .byte  197,188,84,192                      // vandps        %ymm0,%ymm8,%ymm0
  .byte  196,65,61,118,192                   // vpcmpeqd      %ymm8,%ymm8,%ymm8
  .byte  196,65,53,254,192                   // vpaddd        %ymm8,%ymm9,%ymm8
  .byte  196,193,124,93,192                  // vminps        %ymm8,%ymm0,%ymm0
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,172,92,201                      // vsubps        %ymm1,%ymm10,%ymm1
  .byte  196,193,116,92,201                  // vsubps        %ymm9,%ymm1,%ymm1
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  197,60,92,193                       // vsubps        %ymm1,%ymm8,%ymm8
  .byte  197,188,84,201                      // vandps        %ymm1,%ymm8,%ymm1
  .byte  196,65,61,118,192                   // vpcmpeqd      %ymm8,%ymm8,%ymm8
  .byte  196,65,53,254,192                   // vpaddd        %ymm8,%ymm9,%ymm8
  .byte  196,193,116,93,200                  // vminps        %ymm8,%ymm1,%ymm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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

.balign 4
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,63                                // add           %bh,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  145                                 // xchg          %eax,%ecx
  .byte  131,158,61,92,143,50,63             // sbbl          $0x3f,0x328f5c3d(%rsi)
  .byte  154                                 // (bad)
  .byte  153                                 // cltd
  .byte  153                                 // cltd
  .byte  62,10,215                           // ds            or  %bh,%dl
  .byte  35,59                               // and           (%rbx),%edi
  .byte  174                                 // scas          %es:(%rdi),%al
  .byte  71,97                               // rex.RXB       (bad)
  .byte  61,41,92,71,65                      // cmp           $0x41475c29,%eax
  .byte  168,87                              // test          $0x57,%al
  .byte  202,189,206                         // lret          $0xcebd
  .byte  111                                 // outsl         %ds:(%rsi),(%dx)
  .byte  48,63                               // xor           %bh,(%rdi)
  .byte  194,135,210                         // retq          $0xd287
  .byte  62,0,0                              // add           %al,%ds:(%rax)
  .byte  128,63,4                            // cmpb          $0x4,(%rdi)
  .byte  231,140                             // out           %eax,$0x8c
  .byte  59,129,128,128,59,129               // cmp           -0x7ec47f80(%rcx),%eax
  .byte  128,128,59,0,248,0,0                // addb          $0x0,0xf8003b(%rax)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  224,7                               // loopne        b8d <.literal4+0x61>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  31                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,8                                 // add           %cl,(%rax)
  .byte  33,4,61,0,0,128,63                  // and           %eax,0x3f800000(,%rdi,1)
  .byte  129,128,128,59,129,128,128,59,0,0   // addl          $0x3b80,-0x7f7ec480(%rax)
  .byte  127,67                              // jg            be7 <_sk_linear_gradient_2stops_hsw+0x109>
  .byte  0,248                               // add           %bh,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  224,7                               // loopne        bb5 <.literal4+0x89>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  31                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,8                                 // add           %cl,(%rax)
  .byte  33,4,61,0,0,128,63                  // and           %eax,0x3f800000(,%rdi,1)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  248                                 // clc
  .byte  65,0,0                              // add           %al,(%r8)
  .byte  124,66                              // jl            c0a <_sk_linear_gradient_2stops_hsw+0x12c>
  .byte  129                                 // .byte         0x81
  .byte  128,128,59,0,0,127,67               // addb          $0x43,0x7f00003b(%rax)

.balign 16
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  64,0,0                              // add           %al,(%rax)
  .byte  64                                  // rex
  .byte  64,0,0                              // add           %al,(%rax)
  .byte  128,64,0,0                          // addb          $0x0,0x0(%rax)
  .byte  160,64,0,0,192,64,0,0,224           // movabs        0xe0000040c0000040,%al
  .byte  64,255,0                            // rex           incl (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
.balign 16

.globl _sk_start_pipeline_avx
_sk_start_pipeline_avx:
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
  .byte  72,141,67,8                         // lea           0x8(%rbx),%rax
  .byte  76,57,248                           // cmp           %r15,%rax
  .byte  118,5                               // jbe           28 <_sk_start_pipeline_avx+0x28>
  .byte  72,137,216                          // mov           %rbx,%rax
  .byte  235,60                              // jmp           64 <_sk_start_pipeline_avx+0x64>
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  197,228,87,219                      // vxorps        %ymm3,%ymm3,%ymm3
  .byte  197,220,87,228                      // vxorps        %ymm4,%ymm4,%ymm4
  .byte  197,212,87,237                      // vxorps        %ymm5,%ymm5,%ymm5
  .byte  197,204,87,246                      // vxorps        %ymm6,%ymm6,%ymm6
  .byte  197,196,87,255                      // vxorps        %ymm7,%ymm7,%ymm7
  .byte  72,137,223                          // mov           %rbx,%rdi
  .byte  76,137,238                          // mov           %r13,%rsi
  .byte  76,137,242                          // mov           %r14,%rdx
  .byte  65,255,212                          // callq         *%r12
  .byte  72,141,67,8                         // lea           0x8(%rbx),%rax
  .byte  72,131,195,16                       // add           $0x10,%rbx
  .byte  76,57,251                           // cmp           %r15,%rbx
  .byte  72,137,195                          // mov           %rax,%rbx
  .byte  118,196                             // jbe           28 <_sk_start_pipeline_avx+0x28>
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,93                               // pop           %r13
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  197,248,119                         // vzeroupper
  .byte  195                                 // retq

.globl _sk_just_return_avx
_sk_just_return_avx:
  .byte  195                                 // retq

.globl _sk_seed_shader_avx
_sk_seed_shader_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,249,110,199                     // vmovd         %edi,%xmm0
  .byte  196,227,121,4,192,0                 // vpermilps     $0x0,%xmm0,%xmm0
  .byte  196,227,125,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  196,226,125,24,13,71,15,0,0         // vbroadcastss  0xf47(%rip),%ymm1        # fd8 <_sk_linear_gradient_2stops_avx+0x5a>
  .byte  197,252,88,193                      // vaddps        %ymm1,%ymm0,%ymm0
  .byte  197,252,88,5,3,16,0,0               // vaddps        0x1003(%rip),%ymm0,%ymm0        # 10a0 <_sk_linear_gradient_2stops_avx+0x122>
  .byte  197,249,110,16                      // vmovd         (%rax),%xmm2
  .byte  196,227,121,4,210,0                 // vpermilps     $0x0,%xmm2,%xmm2
  .byte  196,227,109,24,210,1                // vinsertf128   $0x1,%xmm2,%ymm2,%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  197,236,88,201                      // vaddps        %ymm1,%ymm2,%ymm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,226,125,24,21,28,15,0,0         // vbroadcastss  0xf1c(%rip),%ymm2        # fdc <_sk_linear_gradient_2stops_avx+0x5e>
  .byte  197,228,87,219                      // vxorps        %ymm3,%ymm3,%ymm3
  .byte  197,220,87,228                      // vxorps        %ymm4,%ymm4,%ymm4
  .byte  197,212,87,237                      // vxorps        %ymm5,%ymm5,%ymm5
  .byte  197,204,87,246                      // vxorps        %ymm6,%ymm6,%ymm6
  .byte  197,196,87,255                      // vxorps        %ymm7,%ymm7,%ymm7
  .byte  255,224                             // jmpq          *%rax

.globl _sk_constant_color_avx
_sk_constant_color_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,226,125,24,0                    // vbroadcastss  (%rax),%ymm0
  .byte  196,226,125,24,72,4                 // vbroadcastss  0x4(%rax),%ymm1
  .byte  196,226,125,24,80,8                 // vbroadcastss  0x8(%rax),%ymm2
  .byte  196,226,125,24,88,12                // vbroadcastss  0xc(%rax),%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_clear_avx
_sk_clear_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  197,228,87,219                      // vxorps        %ymm3,%ymm3,%ymm3
  .byte  255,224                             // jmpq          *%rax

.globl _sk_plus__avx
_sk_plus__avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,88,196                      // vaddps        %ymm4,%ymm0,%ymm0
  .byte  197,244,88,205                      // vaddps        %ymm5,%ymm1,%ymm1
  .byte  197,236,88,214                      // vaddps        %ymm6,%ymm2,%ymm2
  .byte  197,228,88,223                      // vaddps        %ymm7,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_srcover_avx
_sk_srcover_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,5,182,14,0,0          // vbroadcastss  0xeb6(%rip),%ymm8        # fe0 <_sk_linear_gradient_2stops_avx+0x62>
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

.globl _sk_dstover_avx
_sk_dstover_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,5,135,14,0,0          // vbroadcastss  0xe87(%rip),%ymm8        # fe4 <_sk_linear_gradient_2stops_avx+0x66>
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

.globl _sk_clamp_0_avx
_sk_clamp_0_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  196,193,124,95,192                  // vmaxps        %ymm8,%ymm0,%ymm0
  .byte  196,193,116,95,200                  // vmaxps        %ymm8,%ymm1,%ymm1
  .byte  196,193,108,95,208                  // vmaxps        %ymm8,%ymm2,%ymm2
  .byte  196,193,100,95,216                  // vmaxps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_clamp_1_avx
_sk_clamp_1_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,5,57,14,0,0           // vbroadcastss  0xe39(%rip),%ymm8        # fe8 <_sk_linear_gradient_2stops_avx+0x6a>
  .byte  196,193,124,93,192                  // vminps        %ymm8,%ymm0,%ymm0
  .byte  196,193,116,93,200                  // vminps        %ymm8,%ymm1,%ymm1
  .byte  196,193,108,93,208                  // vminps        %ymm8,%ymm2,%ymm2
  .byte  196,193,100,93,216                  // vminps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_clamp_a_avx
_sk_clamp_a_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,5,26,14,0,0           // vbroadcastss  0xe1a(%rip),%ymm8        # fec <_sk_linear_gradient_2stops_avx+0x6e>
  .byte  196,193,100,93,216                  // vminps        %ymm8,%ymm3,%ymm3
  .byte  197,252,93,195                      // vminps        %ymm3,%ymm0,%ymm0
  .byte  197,244,93,203                      // vminps        %ymm3,%ymm1,%ymm1
  .byte  197,236,93,211                      // vminps        %ymm3,%ymm2,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_set_rgb_avx
_sk_set_rgb_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,226,125,24,0                    // vbroadcastss  (%rax),%ymm0
  .byte  196,226,125,24,72,4                 // vbroadcastss  0x4(%rax),%ymm1
  .byte  196,226,125,24,80,8                 // vbroadcastss  0x8(%rax),%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_swap_rb_avx
_sk_swap_rb_avx:
  .byte  197,124,40,192                      // vmovaps       %ymm0,%ymm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,40,194                      // vmovaps       %ymm2,%ymm0
  .byte  197,124,41,194                      // vmovaps       %ymm8,%ymm2
  .byte  255,224                             // jmpq          *%rax

.globl _sk_swap_avx
_sk_swap_avx:
  .byte  197,124,40,195                      // vmovaps       %ymm3,%ymm8
  .byte  197,124,40,202                      // vmovaps       %ymm2,%ymm9
  .byte  197,124,40,209                      // vmovaps       %ymm1,%ymm10
  .byte  197,124,40,216                      // vmovaps       %ymm0,%ymm11
  .byte  72,173                              // lods          %ds:(%rsi),%rax
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

.globl _sk_move_src_dst_avx
_sk_move_src_dst_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,40,224                      // vmovaps       %ymm0,%ymm4
  .byte  197,252,40,233                      // vmovaps       %ymm1,%ymm5
  .byte  197,252,40,242                      // vmovaps       %ymm2,%ymm6
  .byte  197,252,40,251                      // vmovaps       %ymm3,%ymm7
  .byte  255,224                             // jmpq          *%rax

.globl _sk_move_dst_src_avx
_sk_move_dst_src_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,40,196                      // vmovaps       %ymm4,%ymm0
  .byte  197,252,40,205                      // vmovaps       %ymm5,%ymm1
  .byte  197,252,40,214                      // vmovaps       %ymm6,%ymm2
  .byte  197,252,40,223                      // vmovaps       %ymm7,%ymm3
  .byte  255,224                             // jmpq          *%rax

.globl _sk_premul_avx
_sk_premul_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,89,195                      // vmulps        %ymm3,%ymm0,%ymm0
  .byte  197,244,89,203                      // vmulps        %ymm3,%ymm1,%ymm1
  .byte  197,236,89,211                      // vmulps        %ymm3,%ymm2,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_unpremul_avx
_sk_unpremul_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,65,60,87,192                    // vxorps        %ymm8,%ymm8,%ymm8
  .byte  196,65,100,194,200,0                // vcmpeqps      %ymm8,%ymm3,%ymm9
  .byte  196,98,125,24,21,86,13,0,0          // vbroadcastss  0xd56(%rip),%ymm10        # ff0 <_sk_linear_gradient_2stops_avx+0x72>
  .byte  197,44,94,211                       // vdivps        %ymm3,%ymm10,%ymm10
  .byte  196,67,45,74,192,144                // vblendvps     %ymm9,%ymm8,%ymm10,%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_from_srgb_avx
_sk_from_srgb_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,5,53,13,0,0           // vbroadcastss  0xd35(%rip),%ymm8        # ff4 <_sk_linear_gradient_2stops_avx+0x76>
  .byte  196,65,124,89,200                   // vmulps        %ymm8,%ymm0,%ymm9
  .byte  197,124,89,208                      // vmulps        %ymm0,%ymm0,%ymm10
  .byte  196,98,125,24,29,39,13,0,0          // vbroadcastss  0xd27(%rip),%ymm11        # ff8 <_sk_linear_gradient_2stops_avx+0x7a>
  .byte  196,65,124,89,227                   // vmulps        %ymm11,%ymm0,%ymm12
  .byte  196,98,125,24,45,29,13,0,0          // vbroadcastss  0xd1d(%rip),%ymm13        # ffc <_sk_linear_gradient_2stops_avx+0x7e>
  .byte  196,65,28,88,229                    // vaddps        %ymm13,%ymm12,%ymm12
  .byte  196,65,44,89,212                    // vmulps        %ymm12,%ymm10,%ymm10
  .byte  196,98,125,24,37,14,13,0,0          // vbroadcastss  0xd0e(%rip),%ymm12        # 1000 <_sk_linear_gradient_2stops_avx+0x82>
  .byte  196,65,44,88,212                    // vaddps        %ymm12,%ymm10,%ymm10
  .byte  196,98,125,24,53,4,13,0,0           // vbroadcastss  0xd04(%rip),%ymm14        # 1004 <_sk_linear_gradient_2stops_avx+0x86>
  .byte  196,193,124,194,198,1               // vcmpltps      %ymm14,%ymm0,%ymm0
  .byte  196,195,45,74,193,0                 // vblendvps     %ymm0,%ymm9,%ymm10,%ymm0
  .byte  196,65,116,89,200                   // vmulps        %ymm8,%ymm1,%ymm9
  .byte  197,116,89,209                      // vmulps        %ymm1,%ymm1,%ymm10
  .byte  196,65,116,89,251                   // vmulps        %ymm11,%ymm1,%ymm15
  .byte  196,65,4,88,253                     // vaddps        %ymm13,%ymm15,%ymm15
  .byte  196,65,44,89,215                    // vmulps        %ymm15,%ymm10,%ymm10
  .byte  196,65,44,88,212                    // vaddps        %ymm12,%ymm10,%ymm10
  .byte  196,193,116,194,206,1               // vcmpltps      %ymm14,%ymm1,%ymm1
  .byte  196,195,45,74,201,16                // vblendvps     %ymm1,%ymm9,%ymm10,%ymm1
  .byte  196,65,108,89,192                   // vmulps        %ymm8,%ymm2,%ymm8
  .byte  197,108,89,202                      // vmulps        %ymm2,%ymm2,%ymm9
  .byte  196,65,108,89,211                   // vmulps        %ymm11,%ymm2,%ymm10
  .byte  196,65,44,88,213                    // vaddps        %ymm13,%ymm10,%ymm10
  .byte  196,65,52,89,202                    // vmulps        %ymm10,%ymm9,%ymm9
  .byte  196,65,52,88,204                    // vaddps        %ymm12,%ymm9,%ymm9
  .byte  196,193,108,194,214,1               // vcmpltps      %ymm14,%ymm2,%ymm2
  .byte  196,195,53,74,208,32                // vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_to_srgb_avx
_sk_to_srgb_avx:
  .byte  197,124,82,192                      // vrsqrtps      %ymm0,%ymm8
  .byte  196,65,124,83,200                   // vrcpps        %ymm8,%ymm9
  .byte  196,65,124,82,208                   // vrsqrtps      %ymm8,%ymm10
  .byte  196,98,125,24,5,143,12,0,0          // vbroadcastss  0xc8f(%rip),%ymm8        # 1008 <_sk_linear_gradient_2stops_avx+0x8a>
  .byte  196,65,124,89,216                   // vmulps        %ymm8,%ymm0,%ymm11
  .byte  196,98,125,24,37,133,12,0,0         // vbroadcastss  0xc85(%rip),%ymm12        # 100c <_sk_linear_gradient_2stops_avx+0x8e>
  .byte  196,65,52,89,204                    // vmulps        %ymm12,%ymm9,%ymm9
  .byte  196,98,125,24,45,123,12,0,0         // vbroadcastss  0xc7b(%rip),%ymm13        # 1010 <_sk_linear_gradient_2stops_avx+0x92>
  .byte  196,65,52,88,205                    // vaddps        %ymm13,%ymm9,%ymm9
  .byte  196,98,125,24,53,113,12,0,0         // vbroadcastss  0xc71(%rip),%ymm14        # 1014 <_sk_linear_gradient_2stops_avx+0x96>
  .byte  196,65,44,89,214                    // vmulps        %ymm14,%ymm10,%ymm10
  .byte  196,65,44,88,201                    // vaddps        %ymm9,%ymm10,%ymm9
  .byte  196,98,125,24,21,98,12,0,0          // vbroadcastss  0xc62(%rip),%ymm10        # 1018 <_sk_linear_gradient_2stops_avx+0x9a>
  .byte  196,65,44,93,201                    // vminps        %ymm9,%ymm10,%ymm9
  .byte  196,98,125,24,61,88,12,0,0          // vbroadcastss  0xc58(%rip),%ymm15        # 101c <_sk_linear_gradient_2stops_avx+0x9e>
  .byte  196,193,124,194,199,1               // vcmpltps      %ymm15,%ymm0,%ymm0
  .byte  196,195,53,74,195,0                 // vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
  .byte  197,124,82,201                      // vrsqrtps      %ymm1,%ymm9
  .byte  196,65,124,83,217                   // vrcpps        %ymm9,%ymm11
  .byte  196,65,124,82,201                   // vrsqrtps      %ymm9,%ymm9
  .byte  196,65,36,89,220                    // vmulps        %ymm12,%ymm11,%ymm11
  .byte  196,65,36,88,221                    // vaddps        %ymm13,%ymm11,%ymm11
  .byte  196,65,52,89,206                    // vmulps        %ymm14,%ymm9,%ymm9
  .byte  196,65,52,88,203                    // vaddps        %ymm11,%ymm9,%ymm9
  .byte  196,65,116,89,216                   // vmulps        %ymm8,%ymm1,%ymm11
  .byte  196,65,44,93,201                    // vminps        %ymm9,%ymm10,%ymm9
  .byte  196,193,116,194,207,1               // vcmpltps      %ymm15,%ymm1,%ymm1
  .byte  196,195,53,74,203,16                // vblendvps     %ymm1,%ymm11,%ymm9,%ymm1
  .byte  197,124,82,202                      // vrsqrtps      %ymm2,%ymm9
  .byte  196,65,124,83,217                   // vrcpps        %ymm9,%ymm11
  .byte  196,65,36,89,220                    // vmulps        %ymm12,%ymm11,%ymm11
  .byte  196,65,36,88,221                    // vaddps        %ymm13,%ymm11,%ymm11
  .byte  196,65,124,82,201                   // vrsqrtps      %ymm9,%ymm9
  .byte  196,65,52,89,206                    // vmulps        %ymm14,%ymm9,%ymm9
  .byte  196,65,52,88,203                    // vaddps        %ymm11,%ymm9,%ymm9
  .byte  196,65,44,93,201                    // vminps        %ymm9,%ymm10,%ymm9
  .byte  196,65,108,89,192                   // vmulps        %ymm8,%ymm2,%ymm8
  .byte  196,193,108,194,215,1               // vcmpltps      %ymm15,%ymm2,%ymm2
  .byte  196,195,53,74,208,32                // vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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

.globl _sk_scale_u8_avx
_sk_scale_u8_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,98,121,49,68,56,4               // vpmovzxbd     0x4(%rax,%rdi,1),%xmm8
  .byte  196,98,121,49,12,56                 // vpmovzxbd     (%rax,%rdi,1),%xmm9
  .byte  196,67,53,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm9,%ymm8
  .byte  196,65,124,91,192                   // vcvtdq2ps     %ymm8,%ymm8
  .byte  196,98,125,24,13,153,11,0,0         // vbroadcastss  0xb99(%rip),%ymm9        # 1020 <_sk_linear_gradient_2stops_avx+0xa2>
  .byte  196,65,60,89,193                    // vmulps        %ymm9,%ymm8,%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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

.globl _sk_lerp_u8_avx
_sk_lerp_u8_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,98,121,49,68,56,4               // vpmovzxbd     0x4(%rax,%rdi,1),%xmm8
  .byte  196,98,121,49,12,56                 // vpmovzxbd     (%rax,%rdi,1),%xmm9
  .byte  196,67,53,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm9,%ymm8
  .byte  196,65,124,91,192                   // vcvtdq2ps     %ymm8,%ymm8
  .byte  196,98,125,24,13,31,11,0,0          // vbroadcastss  0xb1f(%rip),%ymm9        # 1024 <_sk_linear_gradient_2stops_avx+0xa6>
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
  .byte  255,224                             // jmpq          *%rax

.globl _sk_lerp_565_avx
_sk_lerp_565_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,226,121,51,92,120,8             // vpmovzxwd     0x8(%rax,%rdi,2),%xmm3
  .byte  196,98,121,51,4,120                 // vpmovzxwd     (%rax,%rdi,2),%xmm8
  .byte  196,227,61,24,219,1                 // vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  .byte  196,98,125,24,5,197,10,0,0          // vbroadcastss  0xac5(%rip),%ymm8        # 1028 <_sk_linear_gradient_2stops_avx+0xaa>
  .byte  196,65,100,84,192                   // vandps        %ymm8,%ymm3,%ymm8
  .byte  196,65,124,91,192                   // vcvtdq2ps     %ymm8,%ymm8
  .byte  196,98,125,24,13,182,10,0,0         // vbroadcastss  0xab6(%rip),%ymm9        # 102c <_sk_linear_gradient_2stops_avx+0xae>
  .byte  196,65,60,89,193                    // vmulps        %ymm9,%ymm8,%ymm8
  .byte  196,98,125,24,13,172,10,0,0         // vbroadcastss  0xaac(%rip),%ymm9        # 1030 <_sk_linear_gradient_2stops_avx+0xb2>
  .byte  196,65,100,84,201                   // vandps        %ymm9,%ymm3,%ymm9
  .byte  196,65,124,91,201                   // vcvtdq2ps     %ymm9,%ymm9
  .byte  196,98,125,24,21,157,10,0,0         // vbroadcastss  0xa9d(%rip),%ymm10        # 1034 <_sk_linear_gradient_2stops_avx+0xb6>
  .byte  196,65,52,89,202                    // vmulps        %ymm10,%ymm9,%ymm9
  .byte  196,98,125,24,21,147,10,0,0         // vbroadcastss  0xa93(%rip),%ymm10        # 1038 <_sk_linear_gradient_2stops_avx+0xba>
  .byte  196,193,100,84,218                  // vandps        %ymm10,%ymm3,%ymm3
  .byte  197,252,91,219                      // vcvtdq2ps     %ymm3,%ymm3
  .byte  196,98,125,24,21,133,10,0,0         // vbroadcastss  0xa85(%rip),%ymm10        # 103c <_sk_linear_gradient_2stops_avx+0xbe>
  .byte  196,193,100,89,218                  // vmulps        %ymm10,%ymm3,%ymm3
  .byte  197,252,92,196                      // vsubps        %ymm4,%ymm0,%ymm0
  .byte  196,193,124,89,192                  // vmulps        %ymm8,%ymm0,%ymm0
  .byte  197,252,88,196                      // vaddps        %ymm4,%ymm0,%ymm0
  .byte  197,244,92,205                      // vsubps        %ymm5,%ymm1,%ymm1
  .byte  196,193,116,89,201                  // vmulps        %ymm9,%ymm1,%ymm1
  .byte  197,244,88,205                      // vaddps        %ymm5,%ymm1,%ymm1
  .byte  197,236,92,214                      // vsubps        %ymm6,%ymm2,%ymm2
  .byte  197,236,89,211                      // vmulps        %ymm3,%ymm2,%ymm2
  .byte  197,236,88,214                      // vaddps        %ymm6,%ymm2,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,226,125,24,29,83,10,0,0         // vbroadcastss  0xa53(%rip),%ymm3        # 1040 <_sk_linear_gradient_2stops_avx+0xc2>
  .byte  255,224                             // jmpq          *%rax

.globl _sk_load_tables_avx
_sk_load_tables_avx:
  .byte  65,87                               // push          %r15
  .byte  65,86                               // push          %r14
  .byte  65,84                               // push          %r12
  .byte  83                                  // push          %rbx
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  72,139,72,8                         // mov           0x8(%rax),%rcx
  .byte  196,65,124,16,4,184                 // vmovups       (%r8,%rdi,4),%ymm8
  .byte  196,67,125,25,194,1                 // vextractf128  $0x1,%ymm8,%xmm10
  .byte  197,120,40,13,173,10,0,0            // vmovaps       0xaad(%rip),%xmm9        # 10c0 <_sk_linear_gradient_2stops_avx+0x142>
  .byte  196,193,40,84,193                   // vandps        %xmm9,%xmm10,%xmm0
  .byte  196,193,56,84,201                   // vandps        %xmm9,%xmm8,%xmm1
  .byte  196,193,249,126,200                 // vmovq         %xmm1,%r8
  .byte  69,137,193                          // mov           %r8d,%r9d
  .byte  196,195,249,22,202,1                // vpextrq       $0x1,%xmm1,%r10
  .byte  69,137,211                          // mov           %r10d,%r11d
  .byte  73,193,234,32                       // shr           $0x20,%r10
  .byte  73,193,232,32                       // shr           $0x20,%r8
  .byte  196,193,249,126,199                 // vmovq         %xmm0,%r15
  .byte  69,137,254                          // mov           %r15d,%r14d
  .byte  196,227,249,22,195,1                // vpextrq       $0x1,%xmm0,%rbx
  .byte  65,137,220                          // mov           %ebx,%r12d
  .byte  72,193,235,32                       // shr           $0x20,%rbx
  .byte  73,193,239,32                       // shr           $0x20,%r15
  .byte  196,161,122,16,4,177                // vmovss        (%rcx,%r14,4),%xmm0
  .byte  196,163,121,33,4,185,16             // vinsertps     $0x10,(%rcx,%r15,4),%xmm0,%xmm0
  .byte  196,163,121,33,4,161,32             // vinsertps     $0x20,(%rcx,%r12,4),%xmm0,%xmm0
  .byte  196,227,121,33,4,153,48             // vinsertps     $0x30,(%rcx,%rbx,4),%xmm0,%xmm0
  .byte  196,161,122,16,12,137               // vmovss        (%rcx,%r9,4),%xmm1
  .byte  196,163,113,33,12,129,16            // vinsertps     $0x10,(%rcx,%r8,4),%xmm1,%xmm1
  .byte  196,163,113,33,12,153,32            // vinsertps     $0x20,(%rcx,%r11,4),%xmm1,%xmm1
  .byte  196,163,113,33,12,145,48            // vinsertps     $0x30,(%rcx,%r10,4),%xmm1,%xmm1
  .byte  196,227,117,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  .byte  72,139,72,16                        // mov           0x10(%rax),%rcx
  .byte  196,193,113,114,208,8               // vpsrld        $0x8,%xmm8,%xmm1
  .byte  196,193,105,114,210,8               // vpsrld        $0x8,%xmm10,%xmm2
  .byte  196,193,105,219,209                 // vpand         %xmm9,%xmm2,%xmm2
  .byte  196,193,113,219,201                 // vpand         %xmm9,%xmm1,%xmm1
  .byte  196,193,249,126,200                 // vmovq         %xmm1,%r8
  .byte  69,137,194                          // mov           %r8d,%r10d
  .byte  196,195,249,22,201,1                // vpextrq       $0x1,%xmm1,%r9
  .byte  69,137,203                          // mov           %r9d,%r11d
  .byte  73,193,233,32                       // shr           $0x20,%r9
  .byte  73,193,232,32                       // shr           $0x20,%r8
  .byte  196,193,249,126,215                 // vmovq         %xmm2,%r15
  .byte  69,137,254                          // mov           %r15d,%r14d
  .byte  196,227,249,22,211,1                // vpextrq       $0x1,%xmm2,%rbx
  .byte  65,137,220                          // mov           %ebx,%r12d
  .byte  72,193,235,32                       // shr           $0x20,%rbx
  .byte  73,193,239,32                       // shr           $0x20,%r15
  .byte  196,161,122,16,12,177               // vmovss        (%rcx,%r14,4),%xmm1
  .byte  196,163,113,33,12,185,16            // vinsertps     $0x10,(%rcx,%r15,4),%xmm1,%xmm1
  .byte  196,161,122,16,20,161               // vmovss        (%rcx,%r12,4),%xmm2
  .byte  196,227,113,33,202,32               // vinsertps     $0x20,%xmm2,%xmm1,%xmm1
  .byte  197,250,16,20,153                   // vmovss        (%rcx,%rbx,4),%xmm2
  .byte  196,227,113,33,202,48               // vinsertps     $0x30,%xmm2,%xmm1,%xmm1
  .byte  196,161,122,16,20,145               // vmovss        (%rcx,%r10,4),%xmm2
  .byte  196,163,105,33,20,129,16            // vinsertps     $0x10,(%rcx,%r8,4),%xmm2,%xmm2
  .byte  196,161,122,16,28,153               // vmovss        (%rcx,%r11,4),%xmm3
  .byte  196,227,105,33,211,32               // vinsertps     $0x20,%xmm3,%xmm2,%xmm2
  .byte  196,161,122,16,28,137               // vmovss        (%rcx,%r9,4),%xmm3
  .byte  196,227,105,33,211,48               // vinsertps     $0x30,%xmm3,%xmm2,%xmm2
  .byte  196,227,109,24,201,1                // vinsertf128   $0x1,%xmm1,%ymm2,%ymm1
  .byte  72,139,64,24                        // mov           0x18(%rax),%rax
  .byte  196,193,105,114,208,16              // vpsrld        $0x10,%xmm8,%xmm2
  .byte  196,193,97,114,210,16               // vpsrld        $0x10,%xmm10,%xmm3
  .byte  196,193,97,219,217                  // vpand         %xmm9,%xmm3,%xmm3
  .byte  196,193,105,219,209                 // vpand         %xmm9,%xmm2,%xmm2
  .byte  196,193,249,126,208                 // vmovq         %xmm2,%r8
  .byte  69,137,193                          // mov           %r8d,%r9d
  .byte  196,195,249,22,214,1                // vpextrq       $0x1,%xmm2,%r14
  .byte  69,137,242                          // mov           %r14d,%r10d
  .byte  73,193,238,32                       // shr           $0x20,%r14
  .byte  73,193,232,32                       // shr           $0x20,%r8
  .byte  196,225,249,126,219                 // vmovq         %xmm3,%rbx
  .byte  65,137,219                          // mov           %ebx,%r11d
  .byte  196,227,249,22,217,1                // vpextrq       $0x1,%xmm3,%rcx
  .byte  65,137,207                          // mov           %ecx,%r15d
  .byte  72,193,233,32                       // shr           $0x20,%rcx
  .byte  72,193,235,32                       // shr           $0x20,%rbx
  .byte  196,161,122,16,20,152               // vmovss        (%rax,%r11,4),%xmm2
  .byte  196,227,105,33,20,152,16            // vinsertps     $0x10,(%rax,%rbx,4),%xmm2,%xmm2
  .byte  196,161,122,16,28,184               // vmovss        (%rax,%r15,4),%xmm3
  .byte  196,227,105,33,211,32               // vinsertps     $0x20,%xmm3,%xmm2,%xmm2
  .byte  197,250,16,28,136                   // vmovss        (%rax,%rcx,4),%xmm3
  .byte  196,99,105,33,203,48                // vinsertps     $0x30,%xmm3,%xmm2,%xmm9
  .byte  196,161,122,16,28,136               // vmovss        (%rax,%r9,4),%xmm3
  .byte  196,163,97,33,28,128,16             // vinsertps     $0x10,(%rax,%r8,4),%xmm3,%xmm3
  .byte  196,161,122,16,20,144               // vmovss        (%rax,%r10,4),%xmm2
  .byte  196,227,97,33,210,32                // vinsertps     $0x20,%xmm2,%xmm3,%xmm2
  .byte  196,161,122,16,28,176               // vmovss        (%rax,%r14,4),%xmm3
  .byte  196,227,105,33,211,48               // vinsertps     $0x30,%xmm3,%xmm2,%xmm2
  .byte  196,195,109,24,209,1                // vinsertf128   $0x1,%xmm9,%ymm2,%ymm2
  .byte  196,193,57,114,208,24               // vpsrld        $0x18,%xmm8,%xmm8
  .byte  196,193,97,114,210,24               // vpsrld        $0x18,%xmm10,%xmm3
  .byte  196,227,61,24,219,1                 // vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  .byte  197,252,91,219                      // vcvtdq2ps     %ymm3,%ymm3
  .byte  196,98,125,24,5,100,8,0,0           // vbroadcastss  0x864(%rip),%ymm8        # 1044 <_sk_linear_gradient_2stops_avx+0xc6>
  .byte  196,193,100,89,216                  // vmulps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  255,224                             // jmpq          *%rax

.globl _sk_load_a8_avx
_sk_load_a8_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,226,121,49,68,56,4              // vpmovzxbd     0x4(%rax,%rdi,1),%xmm0
  .byte  196,226,121,49,12,56                // vpmovzxbd     (%rax,%rdi,1),%xmm1
  .byte  196,227,117,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  196,226,125,24,13,51,8,0,0          // vbroadcastss  0x833(%rip),%ymm1        # 1048 <_sk_linear_gradient_2stops_avx+0xca>
  .byte  197,252,89,217                      // vmulps        %ymm1,%ymm0,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  255,224                             // jmpq          *%rax

.globl _sk_store_a8_avx
_sk_store_a8_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,98,125,24,5,21,8,0,0            // vbroadcastss  0x815(%rip),%ymm8        # 104c <_sk_linear_gradient_2stops_avx+0xce>
  .byte  196,65,100,89,192                   // vmulps        %ymm8,%ymm3,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,67,125,25,193,1                 // vextractf128  $0x1,%ymm8,%xmm9
  .byte  196,66,57,43,193                    // vpackusdw     %xmm9,%xmm8,%xmm8
  .byte  196,65,57,103,192                   // vpackuswb     %xmm8,%xmm8,%xmm8
  .byte  197,121,214,4,56                    // vmovq         %xmm8,(%rax,%rdi,1)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_load_565_avx
_sk_load_565_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,226,121,51,68,120,8             // vpmovzxwd     0x8(%rax,%rdi,2),%xmm0
  .byte  196,226,121,51,12,120               // vpmovzxwd     (%rax,%rdi,2),%xmm1
  .byte  196,227,117,24,208,1                // vinsertf128   $0x1,%xmm0,%ymm1,%ymm2
  .byte  196,226,125,24,5,213,7,0,0          // vbroadcastss  0x7d5(%rip),%ymm0        # 1050 <_sk_linear_gradient_2stops_avx+0xd2>
  .byte  197,236,84,192                      // vandps        %ymm0,%ymm2,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  196,226,125,24,13,200,7,0,0         // vbroadcastss  0x7c8(%rip),%ymm1        # 1054 <_sk_linear_gradient_2stops_avx+0xd6>
  .byte  197,252,89,193                      // vmulps        %ymm1,%ymm0,%ymm0
  .byte  196,226,125,24,13,191,7,0,0         // vbroadcastss  0x7bf(%rip),%ymm1        # 1058 <_sk_linear_gradient_2stops_avx+0xda>
  .byte  197,236,84,201                      // vandps        %ymm1,%ymm2,%ymm1
  .byte  197,252,91,201                      // vcvtdq2ps     %ymm1,%ymm1
  .byte  196,226,125,24,29,178,7,0,0         // vbroadcastss  0x7b2(%rip),%ymm3        # 105c <_sk_linear_gradient_2stops_avx+0xde>
  .byte  197,244,89,203                      // vmulps        %ymm3,%ymm1,%ymm1
  .byte  196,226,125,24,29,169,7,0,0         // vbroadcastss  0x7a9(%rip),%ymm3        # 1060 <_sk_linear_gradient_2stops_avx+0xe2>
  .byte  197,236,84,211                      // vandps        %ymm3,%ymm2,%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  196,226,125,24,29,156,7,0,0         // vbroadcastss  0x79c(%rip),%ymm3        # 1064 <_sk_linear_gradient_2stops_avx+0xe6>
  .byte  197,236,89,211                      // vmulps        %ymm3,%ymm2,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,226,125,24,29,145,7,0,0         // vbroadcastss  0x791(%rip),%ymm3        # 1068 <_sk_linear_gradient_2stops_avx+0xea>
  .byte  255,224                             // jmpq          *%rax

.globl _sk_store_565_avx
_sk_store_565_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,98,125,24,5,133,7,0,0           // vbroadcastss  0x785(%rip),%ymm8        # 106c <_sk_linear_gradient_2stops_avx+0xee>
  .byte  196,65,124,89,200                   // vmulps        %ymm8,%ymm0,%ymm9
  .byte  196,65,125,91,201                   // vcvtps2dq     %ymm9,%ymm9
  .byte  196,193,41,114,241,11               // vpslld        $0xb,%xmm9,%xmm10
  .byte  196,67,125,25,201,1                 // vextractf128  $0x1,%ymm9,%xmm9
  .byte  196,193,49,114,241,11               // vpslld        $0xb,%xmm9,%xmm9
  .byte  196,67,45,24,201,1                  // vinsertf128   $0x1,%xmm9,%ymm10,%ymm9
  .byte  196,98,125,24,21,94,7,0,0           // vbroadcastss  0x75e(%rip),%ymm10        # 1070 <_sk_linear_gradient_2stops_avx+0xf2>
  .byte  196,65,116,89,210                   // vmulps        %ymm10,%ymm1,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,33,114,242,5                // vpslld        $0x5,%xmm10,%xmm11
  .byte  196,67,125,25,210,1                 // vextractf128  $0x1,%ymm10,%xmm10
  .byte  196,193,41,114,242,5                // vpslld        $0x5,%xmm10,%xmm10
  .byte  196,67,37,24,210,1                  // vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  .byte  196,65,45,86,201                    // vorpd         %ymm9,%ymm10,%ymm9
  .byte  196,65,108,89,192                   // vmulps        %ymm8,%ymm2,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,65,53,86,192                    // vorpd         %ymm8,%ymm9,%ymm8
  .byte  196,67,125,25,193,1                 // vextractf128  $0x1,%ymm8,%xmm9
  .byte  196,66,57,43,193                    // vpackusdw     %xmm9,%xmm8,%xmm8
  .byte  197,122,127,4,120                   // vmovdqu       %xmm8,(%rax,%rdi,2)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_load_8888_avx
_sk_load_8888_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  197,252,16,28,184                   // vmovups       (%rax,%rdi,4),%ymm3
  .byte  196,195,125,25,216,1                // vextractf128  $0x1,%ymm3,%xmm8
  .byte  197,120,40,29,92,7,0,0              // vmovaps       0x75c(%rip),%xmm11        # 10d0 <_sk_linear_gradient_2stops_avx+0x152>
  .byte  196,193,56,84,195                   // vandps        %xmm11,%xmm8,%xmm0
  .byte  196,193,96,84,203                   // vandps        %xmm11,%xmm3,%xmm1
  .byte  196,227,117,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  196,98,125,24,13,227,6,0,0          // vbroadcastss  0x6e3(%rip),%ymm9        # 1074 <_sk_linear_gradient_2stops_avx+0xf6>
  .byte  196,193,124,89,193                  // vmulps        %ymm9,%ymm0,%ymm0
  .byte  197,169,114,211,8                   // vpsrld        $0x8,%xmm3,%xmm10
  .byte  196,193,113,114,208,8               // vpsrld        $0x8,%xmm8,%xmm1
  .byte  196,193,113,219,203                 // vpand         %xmm11,%xmm1,%xmm1
  .byte  196,65,41,219,211                   // vpand         %xmm11,%xmm10,%xmm10
  .byte  196,227,45,24,201,1                 // vinsertf128   $0x1,%xmm1,%ymm10,%ymm1
  .byte  197,252,91,201                      // vcvtdq2ps     %ymm1,%ymm1
  .byte  196,193,116,89,201                  // vmulps        %ymm9,%ymm1,%ymm1
  .byte  197,169,114,211,16                  // vpsrld        $0x10,%xmm3,%xmm10
  .byte  196,193,105,114,208,16              // vpsrld        $0x10,%xmm8,%xmm2
  .byte  196,193,105,219,211                 // vpand         %xmm11,%xmm2,%xmm2
  .byte  196,65,41,219,211                   // vpand         %xmm11,%xmm10,%xmm10
  .byte  196,227,45,24,210,1                 // vinsertf128   $0x1,%xmm2,%ymm10,%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  196,193,108,89,209                  // vmulps        %ymm9,%ymm2,%ymm2
  .byte  197,169,114,211,24                  // vpsrld        $0x18,%xmm3,%xmm10
  .byte  196,193,97,114,208,24               // vpsrld        $0x18,%xmm8,%xmm3
  .byte  196,227,45,24,219,1                 // vinsertf128   $0x1,%xmm3,%ymm10,%ymm3
  .byte  197,252,91,219                      // vcvtdq2ps     %ymm3,%ymm3
  .byte  196,193,100,89,217                  // vmulps        %ymm9,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_store_8888_avx
_sk_store_8888_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,98,125,24,5,110,6,0,0           // vbroadcastss  0x66e(%rip),%ymm8        # 1078 <_sk_linear_gradient_2stops_avx+0xfa>
  .byte  196,65,124,89,200                   // vmulps        %ymm8,%ymm0,%ymm9
  .byte  196,65,125,91,201                   // vcvtps2dq     %ymm9,%ymm9
  .byte  196,65,116,89,208                   // vmulps        %ymm8,%ymm1,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,33,114,242,8                // vpslld        $0x8,%xmm10,%xmm11
  .byte  196,67,125,25,210,1                 // vextractf128  $0x1,%ymm10,%xmm10
  .byte  196,193,41,114,242,8                // vpslld        $0x8,%xmm10,%xmm10
  .byte  196,67,37,24,210,1                  // vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  .byte  196,65,45,86,201                    // vorpd         %ymm9,%ymm10,%ymm9
  .byte  196,65,108,89,208                   // vmulps        %ymm8,%ymm2,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,33,114,242,16               // vpslld        $0x10,%xmm10,%xmm11
  .byte  196,67,125,25,210,1                 // vextractf128  $0x1,%ymm10,%xmm10
  .byte  196,193,41,114,242,16               // vpslld        $0x10,%xmm10,%xmm10
  .byte  196,67,37,24,210,1                  // vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  .byte  196,65,53,86,202                    // vorpd         %ymm10,%ymm9,%ymm9
  .byte  196,65,100,89,192                   // vmulps        %ymm8,%ymm3,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,193,41,114,240,24               // vpslld        $0x18,%xmm8,%xmm10
  .byte  196,67,125,25,192,1                 // vextractf128  $0x1,%ymm8,%xmm8
  .byte  196,193,57,114,240,24               // vpslld        $0x18,%xmm8,%xmm8
  .byte  196,67,45,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm10,%ymm8
  .byte  196,65,53,86,192                    // vorpd         %ymm8,%ymm9,%ymm8
  .byte  197,125,17,4,184                    // vmovupd       %ymm8,(%rax,%rdi,4)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_load_f16_avx
_sk_load_f16_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  197,250,111,4,248                   // vmovdqu       (%rax,%rdi,8),%xmm0
  .byte  197,250,111,76,248,16               // vmovdqu       0x10(%rax,%rdi,8),%xmm1
  .byte  197,250,111,84,248,32               // vmovdqu       0x20(%rax,%rdi,8),%xmm2
  .byte  197,250,111,92,248,48               // vmovdqu       0x30(%rax,%rdi,8),%xmm3
  .byte  197,121,97,193                      // vpunpcklwd    %xmm1,%xmm0,%xmm8
  .byte  197,249,105,193                     // vpunpckhwd    %xmm1,%xmm0,%xmm0
  .byte  197,233,97,203                      // vpunpcklwd    %xmm3,%xmm2,%xmm1
  .byte  197,233,105,211                     // vpunpckhwd    %xmm3,%xmm2,%xmm2
  .byte  197,185,97,216                      // vpunpcklwd    %xmm0,%xmm8,%xmm3
  .byte  197,185,105,192                     // vpunpckhwd    %xmm0,%xmm8,%xmm0
  .byte  197,113,97,194                      // vpunpcklwd    %xmm2,%xmm1,%xmm8
  .byte  197,241,105,202                     // vpunpckhwd    %xmm2,%xmm1,%xmm1
  .byte  197,121,111,13,10,6,0,0             // vmovdqa       0x60a(%rip),%xmm9        # 10e0 <_sk_linear_gradient_2stops_avx+0x162>
  .byte  197,177,101,211                     // vpcmpgtw      %xmm3,%xmm9,%xmm2
  .byte  197,233,223,211                     // vpandn        %xmm3,%xmm2,%xmm2
  .byte  197,177,101,216                     // vpcmpgtw      %xmm0,%xmm9,%xmm3
  .byte  197,97,223,208                      // vpandn        %xmm0,%xmm3,%xmm10
  .byte  196,193,49,101,216                  // vpcmpgtw      %xmm8,%xmm9,%xmm3
  .byte  196,193,97,223,216                  // vpandn        %xmm8,%xmm3,%xmm3
  .byte  197,177,101,193                     // vpcmpgtw      %xmm1,%xmm9,%xmm0
  .byte  197,249,223,193                     // vpandn        %xmm1,%xmm0,%xmm0
  .byte  196,98,121,51,202                   // vpmovzxwd     %xmm2,%xmm9
  .byte  196,98,121,51,195                   // vpmovzxwd     %xmm3,%xmm8
  .byte  197,241,239,201                     // vpxor         %xmm1,%xmm1,%xmm1
  .byte  197,233,105,209                     // vpunpckhwd    %xmm1,%xmm2,%xmm2
  .byte  197,225,105,217                     // vpunpckhwd    %xmm1,%xmm3,%xmm3
  .byte  196,66,121,51,218                   // vpmovzxwd     %xmm10,%xmm11
  .byte  196,98,121,51,224                   // vpmovzxwd     %xmm0,%xmm12
  .byte  197,41,105,209                      // vpunpckhwd    %xmm1,%xmm10,%xmm10
  .byte  197,121,105,233                     // vpunpckhwd    %xmm1,%xmm0,%xmm13
  .byte  196,193,121,114,241,13              // vpslld        $0xd,%xmm9,%xmm0
  .byte  196,193,113,114,240,13              // vpslld        $0xd,%xmm8,%xmm1
  .byte  196,227,125,24,193,1                // vinsertf128   $0x1,%xmm1,%ymm0,%ymm0
  .byte  196,98,125,24,5,65,5,0,0            // vbroadcastss  0x541(%rip),%ymm8        # 107c <_sk_linear_gradient_2stops_avx+0xfe>
  .byte  196,193,124,89,192                  // vmulps        %ymm8,%ymm0,%ymm0
  .byte  197,241,114,242,13                  // vpslld        $0xd,%xmm2,%xmm1
  .byte  197,233,114,243,13                  // vpslld        $0xd,%xmm3,%xmm2
  .byte  196,227,117,24,202,1                // vinsertf128   $0x1,%xmm2,%ymm1,%ymm1
  .byte  196,193,116,89,200                  // vmulps        %ymm8,%ymm1,%ymm1
  .byte  196,193,105,114,243,13              // vpslld        $0xd,%xmm11,%xmm2
  .byte  196,193,97,114,244,13               // vpslld        $0xd,%xmm12,%xmm3
  .byte  196,227,109,24,211,1                // vinsertf128   $0x1,%xmm3,%ymm2,%ymm2
  .byte  196,193,108,89,208                  // vmulps        %ymm8,%ymm2,%ymm2
  .byte  196,193,49,114,242,13               // vpslld        $0xd,%xmm10,%xmm9
  .byte  196,193,97,114,245,13               // vpslld        $0xd,%xmm13,%xmm3
  .byte  196,227,53,24,219,1                 // vinsertf128   $0x1,%xmm3,%ymm9,%ymm3
  .byte  196,193,100,89,216                  // vmulps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_store_f16_avx
_sk_store_f16_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  196,98,125,24,5,235,4,0,0           // vbroadcastss  0x4eb(%rip),%ymm8        # 1080 <_sk_linear_gradient_2stops_avx+0x102>
  .byte  196,65,124,89,200                   // vmulps        %ymm8,%ymm0,%ymm9
  .byte  196,67,125,25,202,1                 // vextractf128  $0x1,%ymm9,%xmm10
  .byte  196,193,41,114,210,13               // vpsrld        $0xd,%xmm10,%xmm10
  .byte  196,193,49,114,209,13               // vpsrld        $0xd,%xmm9,%xmm9
  .byte  196,65,116,89,216                   // vmulps        %ymm8,%ymm1,%ymm11
  .byte  196,67,125,25,220,1                 // vextractf128  $0x1,%ymm11,%xmm12
  .byte  196,193,25,114,212,13               // vpsrld        $0xd,%xmm12,%xmm12
  .byte  196,193,33,114,211,13               // vpsrld        $0xd,%xmm11,%xmm11
  .byte  196,65,108,89,232                   // vmulps        %ymm8,%ymm2,%ymm13
  .byte  196,67,125,25,238,1                 // vextractf128  $0x1,%ymm13,%xmm14
  .byte  196,193,9,114,214,13                // vpsrld        $0xd,%xmm14,%xmm14
  .byte  196,193,17,114,213,13               // vpsrld        $0xd,%xmm13,%xmm13
  .byte  196,65,100,89,192                   // vmulps        %ymm8,%ymm3,%ymm8
  .byte  196,67,125,25,199,1                 // vextractf128  $0x1,%ymm8,%xmm15
  .byte  196,193,1,114,215,13                // vpsrld        $0xd,%xmm15,%xmm15
  .byte  196,193,57,114,208,13               // vpsrld        $0xd,%xmm8,%xmm8
  .byte  196,193,33,115,251,2                // vpslldq       $0x2,%xmm11,%xmm11
  .byte  196,65,33,235,201                   // vpor          %xmm9,%xmm11,%xmm9
  .byte  196,193,33,115,252,2                // vpslldq       $0x2,%xmm12,%xmm11
  .byte  196,65,33,235,210                   // vpor          %xmm10,%xmm11,%xmm10
  .byte  196,193,57,115,248,2                // vpslldq       $0x2,%xmm8,%xmm8
  .byte  196,65,57,235,197                   // vpor          %xmm13,%xmm8,%xmm8
  .byte  196,193,33,115,255,2                // vpslldq       $0x2,%xmm15,%xmm11
  .byte  196,65,33,235,222                   // vpor          %xmm14,%xmm11,%xmm11
  .byte  196,65,49,98,224                    // vpunpckldq    %xmm8,%xmm9,%xmm12
  .byte  197,122,127,36,248                  // vmovdqu       %xmm12,(%rax,%rdi,8)
  .byte  196,65,49,106,192                   // vpunpckhdq    %xmm8,%xmm9,%xmm8
  .byte  197,122,127,68,248,16               // vmovdqu       %xmm8,0x10(%rax,%rdi,8)
  .byte  196,65,41,98,195                    // vpunpckldq    %xmm11,%xmm10,%xmm8
  .byte  197,122,127,68,248,32               // vmovdqu       %xmm8,0x20(%rax,%rdi,8)
  .byte  196,65,41,106,195                   // vpunpckhdq    %xmm11,%xmm10,%xmm8
  .byte  197,122,127,68,248,48               // vmovdqu       %xmm8,0x30(%rax,%rdi,8)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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

.globl _sk_mirror_x_avx
_sk_mirror_x_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,122,16,0                        // vmovss        (%rax),%xmm8
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

.globl _sk_mirror_y_avx
_sk_mirror_y_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,122,16,0                        // vmovss        (%rax),%xmm8
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

.balign 4
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,63                                // add           %bh,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  145                                 // xchg          %eax,%ecx
  .byte  131,158,61,154,153,153,62           // sbbl          $0x3e,-0x666665c3(%rsi)
  .byte  92                                  // pop           %rsp
  .byte  143                                 // (bad)
  .byte  50,63                               // xor           (%rdi),%bh
  .byte  10,215                              // or            %bh,%dl
  .byte  35,59                               // and           (%rbx),%edi
  .byte  174                                 // scas          %es:(%rdi),%al
  .byte  71,97                               // rex.RXB       (bad)
  .byte  61,41,92,71,65                      // cmp           $0x41475c29,%eax
  .byte  206                                 // (bad)
  .byte  111                                 // outsl         %ds:(%rsi),(%dx)
  .byte  48,63                               // xor           %bh,(%rdi)
  .byte  168,87                              // test          $0x57,%al
  .byte  202,189,194                         // lret          $0xc2bd
  .byte  135,210                             // xchg          %edx,%edx
  .byte  62,0,0                              // add           %al,%ds:(%rax)
  .byte  128,63,4                            // cmpb          $0x4,(%rdi)
  .byte  231,140                             // out           %eax,$0x8c
  .byte  59,129,128,128,59,129               // cmp           -0x7ec47f80(%rcx),%eax
  .byte  128,128,59,0,248,0,0                // addb          $0x0,0xf8003b(%rax)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  224,7                               // loopne        1039 <.literal4+0x61>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  31                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,8                                 // add           %cl,(%rax)
  .byte  33,4,61,0,0,128,63                  // and           %eax,0x3f800000(,%rdi,1)
  .byte  129,128,128,59,129,128,128,59,0,0   // addl          $0x3b80,-0x7f7ec480(%rax)
  .byte  127,67                              // jg            1093 <_sk_linear_gradient_2stops_avx+0x115>
  .byte  0,248                               // add           %bh,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  224,7                               // loopne        1061 <.literal4+0x89>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  31                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,8                                 // add           %cl,(%rax)
  .byte  33,4,61,0,0,128,63                  // and           %eax,0x3f800000(,%rdi,1)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  248                                 // clc
  .byte  65,0,0                              // add           %al,(%r8)
  .byte  124,66                              // jl            10b6 <_sk_linear_gradient_2stops_avx+0x138>
  .byte  129,128,128,59,0,0,127,67,0,0       // addl          $0x437f,0x3b80(%rax)
  .byte  128,119,0,0                         // xorb          $0x0,0x0(%rdi)
  .byte  128                                 // .byte         0x80
  .byte  7                                   // (bad)

.balign 16
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  64,0,0                              // add           %al,(%rax)
  .byte  64                                  // rex
  .byte  64,0,0                              // add           %al,(%rax)
  .byte  128,64,0,0                          // addb          $0x0,0x0(%rax)
  .byte  160,64,0,0,192,64,0,0,224           // movabs        0xe0000040c0000040,%al
  .byte  64                                  // rex

.balign 16
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,4,0                               // add           %al,(%rax,%rax,1)
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  4                                   // .byte         0x4
.balign 16

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

.globl _sk_just_return_sse41
_sk_just_return_sse41:
  .byte  195                                 // retq

.globl _sk_seed_shader_sse41
_sk_seed_shader_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  102,15,110,199                      // movd          %edi,%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  15,91,192                           // cvtdq2ps      %xmm0,%xmm0
  .byte  15,40,21,180,12,0,0                 // movaps        0xcb4(%rip),%xmm2        # d30 <_sk_linear_gradient_2stops_sse41+0x6e>
  .byte  15,88,194                           // addps         %xmm2,%xmm0
  .byte  15,88,5,186,12,0,0                  // addps         0xcba(%rip),%xmm0        # d40 <_sk_linear_gradient_2stops_sse41+0x7e>
  .byte  102,15,110,8                        // movd          (%rax),%xmm1
  .byte  102,15,112,201,0                    // pshufd        $0x0,%xmm1,%xmm1
  .byte  15,91,201                           // cvtdq2ps      %xmm1,%xmm1
  .byte  15,88,202                           // addps         %xmm2,%xmm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,21,178,12,0,0                 // movaps        0xcb2(%rip),%xmm2        # d50 <_sk_linear_gradient_2stops_sse41+0x8e>
  .byte  15,87,219                           // xorps         %xmm3,%xmm3
  .byte  15,87,228                           // xorps         %xmm4,%xmm4
  .byte  15,87,237                           // xorps         %xmm5,%xmm5
  .byte  15,87,246                           // xorps         %xmm6,%xmm6
  .byte  15,87,255                           // xorps         %xmm7,%xmm7
  .byte  255,224                             // jmpq          *%rax

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

.globl _sk_clear_sse41
_sk_clear_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,87,201                           // xorps         %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  15,87,219                           // xorps         %xmm3,%xmm3
  .byte  255,224                             // jmpq          *%rax

.globl _sk_plus__sse41
_sk_plus__sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  15,88,214                           // addps         %xmm6,%xmm2
  .byte  15,88,223                           // addps         %xmm7,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_srcover_sse41
_sk_srcover_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  68,15,40,5,97,12,0,0                // movaps        0xc61(%rip),%xmm8        # d60 <_sk_linear_gradient_2stops_sse41+0x9e>
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

.globl _sk_dstover_sse41
_sk_dstover_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  68,15,40,5,51,12,0,0                // movaps        0xc33(%rip),%xmm8        # d70 <_sk_linear_gradient_2stops_sse41+0xae>
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

.globl _sk_clamp_0_sse41
_sk_clamp_0_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  69,15,87,192                        // xorps         %xmm8,%xmm8
  .byte  65,15,95,192                        // maxps         %xmm8,%xmm0
  .byte  65,15,95,200                        // maxps         %xmm8,%xmm1
  .byte  65,15,95,208                        // maxps         %xmm8,%xmm2
  .byte  65,15,95,216                        // maxps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_clamp_1_sse41
_sk_clamp_1_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  68,15,40,5,251,11,0,0               // movaps        0xbfb(%rip),%xmm8        # d80 <_sk_linear_gradient_2stops_sse41+0xbe>
  .byte  65,15,93,192                        // minps         %xmm8,%xmm0
  .byte  65,15,93,200                        // minps         %xmm8,%xmm1
  .byte  65,15,93,208                        // minps         %xmm8,%xmm2
  .byte  65,15,93,216                        // minps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_clamp_a_sse41
_sk_clamp_a_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,93,29,238,11,0,0                 // minps         0xbee(%rip),%xmm3        # d90 <_sk_linear_gradient_2stops_sse41+0xce>
  .byte  15,93,195                           // minps         %xmm3,%xmm0
  .byte  15,93,203                           // minps         %xmm3,%xmm1
  .byte  15,93,211                           // minps         %xmm3,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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

.globl _sk_swap_rb_sse41
_sk_swap_rb_sse41:
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,194                           // movaps        %xmm2,%xmm0
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  255,224                             // jmpq          *%rax

.globl _sk_swap_sse41
_sk_swap_sse41:
  .byte  68,15,40,195                        // movaps        %xmm3,%xmm8
  .byte  68,15,40,202                        // movaps        %xmm2,%xmm9
  .byte  68,15,40,209                        // movaps        %xmm1,%xmm10
  .byte  68,15,40,216                        // movaps        %xmm0,%xmm11
  .byte  72,173                              // lods          %ds:(%rsi),%rax
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

.globl _sk_move_src_dst_sse41
_sk_move_src_dst_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,224                           // movaps        %xmm0,%xmm4
  .byte  15,40,233                           // movaps        %xmm1,%xmm5
  .byte  15,40,242                           // movaps        %xmm2,%xmm6
  .byte  15,40,251                           // movaps        %xmm3,%xmm7
  .byte  255,224                             // jmpq          *%rax

.globl _sk_move_dst_src_sse41
_sk_move_dst_src_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,196                           // movaps        %xmm4,%xmm0
  .byte  15,40,205                           // movaps        %xmm5,%xmm1
  .byte  15,40,214                           // movaps        %xmm6,%xmm2
  .byte  15,40,223                           // movaps        %xmm7,%xmm3
  .byte  255,224                             // jmpq          *%rax

.globl _sk_premul_sse41
_sk_premul_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,89,195                           // mulps         %xmm3,%xmm0
  .byte  15,89,203                           // mulps         %xmm3,%xmm1
  .byte  15,89,211                           // mulps         %xmm3,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_unpremul_sse41
_sk_unpremul_sse41:
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  69,15,87,201                        // xorps         %xmm9,%xmm9
  .byte  68,15,40,21,73,11,0,0               // movaps        0xb49(%rip),%xmm10        # da0 <_sk_linear_gradient_2stops_sse41+0xde>
  .byte  68,15,94,211                        // divps         %xmm3,%xmm10
  .byte  15,40,195                           // movaps        %xmm3,%xmm0
  .byte  65,15,194,193,0                     // cmpeqps       %xmm9,%xmm0
  .byte  102,69,15,56,20,209                 // blendvps      %xmm0,%xmm9,%xmm10
  .byte  69,15,89,194                        // mulps         %xmm10,%xmm8
  .byte  65,15,89,202                        // mulps         %xmm10,%xmm1
  .byte  65,15,89,210                        // mulps         %xmm10,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  255,224                             // jmpq          *%rax

.globl _sk_from_srgb_sse41
_sk_from_srgb_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  68,15,40,29,41,11,0,0               // movaps        0xb29(%rip),%xmm11        # db0 <_sk_linear_gradient_2stops_sse41+0xee>
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  69,15,89,203                        // mulps         %xmm11,%xmm9
  .byte  68,15,40,208                        // movaps        %xmm0,%xmm10
  .byte  69,15,89,210                        // mulps         %xmm10,%xmm10
  .byte  68,15,40,37,33,11,0,0               // movaps        0xb21(%rip),%xmm12        # dc0 <_sk_linear_gradient_2stops_sse41+0xfe>
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  69,15,89,196                        // mulps         %xmm12,%xmm8
  .byte  68,15,40,45,33,11,0,0               // movaps        0xb21(%rip),%xmm13        # dd0 <_sk_linear_gradient_2stops_sse41+0x10e>
  .byte  69,15,88,197                        // addps         %xmm13,%xmm8
  .byte  69,15,89,194                        // mulps         %xmm10,%xmm8
  .byte  68,15,40,53,33,11,0,0               // movaps        0xb21(%rip),%xmm14        # de0 <_sk_linear_gradient_2stops_sse41+0x11e>
  .byte  69,15,88,198                        // addps         %xmm14,%xmm8
  .byte  68,15,40,61,37,11,0,0               // movaps        0xb25(%rip),%xmm15        # df0 <_sk_linear_gradient_2stops_sse41+0x12e>
  .byte  65,15,194,199,1                     // cmpltps       %xmm15,%xmm0
  .byte  102,69,15,56,20,193                 // blendvps      %xmm0,%xmm9,%xmm8
  .byte  68,15,40,209                        // movaps        %xmm1,%xmm10
  .byte  69,15,89,211                        // mulps         %xmm11,%xmm10
  .byte  15,40,193                           // movaps        %xmm1,%xmm0
  .byte  15,89,192                           // mulps         %xmm0,%xmm0
  .byte  68,15,40,201                        // movaps        %xmm1,%xmm9
  .byte  69,15,89,204                        // mulps         %xmm12,%xmm9
  .byte  69,15,88,205                        // addps         %xmm13,%xmm9
  .byte  68,15,89,200                        // mulps         %xmm0,%xmm9
  .byte  69,15,88,206                        // addps         %xmm14,%xmm9
  .byte  65,15,194,207,1                     // cmpltps       %xmm15,%xmm1
  .byte  15,40,193                           // movaps        %xmm1,%xmm0
  .byte  102,69,15,56,20,202                 // blendvps      %xmm0,%xmm10,%xmm9
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  68,15,40,210                        // movaps        %xmm2,%xmm10
  .byte  69,15,89,210                        // mulps         %xmm10,%xmm10
  .byte  68,15,89,226                        // mulps         %xmm2,%xmm12
  .byte  69,15,88,229                        // addps         %xmm13,%xmm12
  .byte  69,15,89,212                        // mulps         %xmm12,%xmm10
  .byte  69,15,88,214                        // addps         %xmm14,%xmm10
  .byte  65,15,194,215,1                     // cmpltps       %xmm15,%xmm2
  .byte  15,40,194                           // movaps        %xmm2,%xmm0
  .byte  102,69,15,56,20,211                 // blendvps      %xmm0,%xmm11,%xmm10
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  65,15,40,201                        // movaps        %xmm9,%xmm1
  .byte  65,15,40,210                        // movaps        %xmm10,%xmm2
  .byte  255,224                             // jmpq          *%rax

.globl _sk_to_srgb_sse41
_sk_to_srgb_sse41:
  .byte  72,131,236,24                       // sub           $0x18,%rsp
  .byte  15,41,60,36                         // movaps        %xmm7,(%rsp)
  .byte  15,40,254                           // movaps        %xmm6,%xmm7
  .byte  15,40,245                           // movaps        %xmm5,%xmm6
  .byte  15,40,236                           // movaps        %xmm4,%xmm5
  .byte  15,40,227                           // movaps        %xmm3,%xmm4
  .byte  15,40,218                           // movaps        %xmm2,%xmm3
  .byte  15,40,209                           // movaps        %xmm1,%xmm2
  .byte  68,15,82,192                        // rsqrtps       %xmm0,%xmm8
  .byte  69,15,83,200                        // rcpps         %xmm8,%xmm9
  .byte  69,15,82,208                        // rsqrtps       %xmm8,%xmm10
  .byte  68,15,40,29,146,10,0,0              // movaps        0xa92(%rip),%xmm11        # e00 <_sk_linear_gradient_2stops_sse41+0x13e>
  .byte  15,40,200                           // movaps        %xmm0,%xmm1
  .byte  65,15,89,203                        // mulps         %xmm11,%xmm1
  .byte  68,15,40,37,147,10,0,0              // movaps        0xa93(%rip),%xmm12        # e10 <_sk_linear_gradient_2stops_sse41+0x14e>
  .byte  69,15,89,204                        // mulps         %xmm12,%xmm9
  .byte  68,15,40,45,151,10,0,0              // movaps        0xa97(%rip),%xmm13        # e20 <_sk_linear_gradient_2stops_sse41+0x15e>
  .byte  69,15,88,205                        // addps         %xmm13,%xmm9
  .byte  68,15,40,53,155,10,0,0              // movaps        0xa9b(%rip),%xmm14        # e30 <_sk_linear_gradient_2stops_sse41+0x16e>
  .byte  69,15,89,214                        // mulps         %xmm14,%xmm10
  .byte  69,15,88,209                        // addps         %xmm9,%xmm10
  .byte  68,15,40,5,155,10,0,0               // movaps        0xa9b(%rip),%xmm8        # e40 <_sk_linear_gradient_2stops_sse41+0x17e>
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  69,15,93,202                        // minps         %xmm10,%xmm9
  .byte  68,15,40,61,155,10,0,0              // movaps        0xa9b(%rip),%xmm15        # e50 <_sk_linear_gradient_2stops_sse41+0x18e>
  .byte  65,15,194,199,1                     // cmpltps       %xmm15,%xmm0
  .byte  102,68,15,56,20,201                 // blendvps      %xmm0,%xmm1,%xmm9
  .byte  15,82,194                           // rsqrtps       %xmm2,%xmm0
  .byte  15,83,200                           // rcpps         %xmm0,%xmm1
  .byte  15,82,192                           // rsqrtps       %xmm0,%xmm0
  .byte  65,15,89,204                        // mulps         %xmm12,%xmm1
  .byte  65,15,88,205                        // addps         %xmm13,%xmm1
  .byte  65,15,89,198                        // mulps         %xmm14,%xmm0
  .byte  15,88,193                           // addps         %xmm1,%xmm0
  .byte  69,15,40,208                        // movaps        %xmm8,%xmm10
  .byte  68,15,93,208                        // minps         %xmm0,%xmm10
  .byte  15,40,202                           // movaps        %xmm2,%xmm1
  .byte  65,15,89,203                        // mulps         %xmm11,%xmm1
  .byte  65,15,194,215,1                     // cmpltps       %xmm15,%xmm2
  .byte  15,40,194                           // movaps        %xmm2,%xmm0
  .byte  102,68,15,56,20,209                 // blendvps      %xmm0,%xmm1,%xmm10
  .byte  15,82,195                           // rsqrtps       %xmm3,%xmm0
  .byte  15,83,200                           // rcpps         %xmm0,%xmm1
  .byte  65,15,89,204                        // mulps         %xmm12,%xmm1
  .byte  65,15,88,205                        // addps         %xmm13,%xmm1
  .byte  15,82,192                           // rsqrtps       %xmm0,%xmm0
  .byte  65,15,89,198                        // mulps         %xmm14,%xmm0
  .byte  15,88,193                           // addps         %xmm1,%xmm0
  .byte  68,15,93,192                        // minps         %xmm0,%xmm8
  .byte  68,15,89,219                        // mulps         %xmm3,%xmm11
  .byte  65,15,194,223,1                     // cmpltps       %xmm15,%xmm3
  .byte  15,40,195                           // movaps        %xmm3,%xmm0
  .byte  102,69,15,56,20,195                 // blendvps      %xmm0,%xmm11,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,193                        // movaps        %xmm9,%xmm0
  .byte  65,15,40,202                        // movaps        %xmm10,%xmm1
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  15,40,220                           // movaps        %xmm4,%xmm3
  .byte  15,40,229                           // movaps        %xmm5,%xmm4
  .byte  15,40,238                           // movaps        %xmm6,%xmm5
  .byte  15,40,247                           // movaps        %xmm7,%xmm6
  .byte  15,40,60,36                         // movaps        (%rsp),%xmm7
  .byte  72,131,196,24                       // add           $0x18,%rsp
  .byte  255,224                             // jmpq          *%rax

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

.globl _sk_scale_u8_sse41
_sk_scale_u8_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,68,15,56,49,4,56                // pmovzxbd      (%rax,%rdi,1),%xmm8
  .byte  69,15,91,192                        // cvtdq2ps      %xmm8,%xmm8
  .byte  68,15,89,5,223,9,0,0                // mulps         0x9df(%rip),%xmm8        # e60 <_sk_linear_gradient_2stops_sse41+0x19e>
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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

.globl _sk_lerp_u8_sse41
_sk_lerp_u8_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,68,15,56,49,4,56                // pmovzxbd      (%rax,%rdi,1),%xmm8
  .byte  69,15,91,192                        // cvtdq2ps      %xmm8,%xmm8
  .byte  68,15,89,5,139,9,0,0                // mulps         0x98b(%rip),%xmm8        # e70 <_sk_linear_gradient_2stops_sse41+0x1ae>
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

.globl _sk_lerp_565_sse41
_sk_lerp_565_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,68,15,56,51,4,120               // pmovzxwd      (%rax,%rdi,2),%xmm8
  .byte  102,15,111,29,91,9,0,0              // movdqa        0x95b(%rip),%xmm3        # e80 <_sk_linear_gradient_2stops_sse41+0x1be>
  .byte  102,65,15,219,216                   // pand          %xmm8,%xmm3
  .byte  68,15,91,203                        // cvtdq2ps      %xmm3,%xmm9
  .byte  68,15,89,13,90,9,0,0                // mulps         0x95a(%rip),%xmm9        # e90 <_sk_linear_gradient_2stops_sse41+0x1ce>
  .byte  102,15,111,29,98,9,0,0              // movdqa        0x962(%rip),%xmm3        # ea0 <_sk_linear_gradient_2stops_sse41+0x1de>
  .byte  102,65,15,219,216                   // pand          %xmm8,%xmm3
  .byte  15,91,219                           // cvtdq2ps      %xmm3,%xmm3
  .byte  15,89,29,99,9,0,0                   // mulps         0x963(%rip),%xmm3        # eb0 <_sk_linear_gradient_2stops_sse41+0x1ee>
  .byte  102,68,15,219,5,106,9,0,0           // pand          0x96a(%rip),%xmm8        # ec0 <_sk_linear_gradient_2stops_sse41+0x1fe>
  .byte  69,15,91,192                        // cvtdq2ps      %xmm8,%xmm8
  .byte  68,15,89,5,110,9,0,0                // mulps         0x96e(%rip),%xmm8        # ed0 <_sk_linear_gradient_2stops_sse41+0x20e>
  .byte  15,92,196                           // subps         %xmm4,%xmm0
  .byte  65,15,89,193                        // mulps         %xmm9,%xmm0
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  15,92,205                           // subps         %xmm5,%xmm1
  .byte  15,89,203                           // mulps         %xmm3,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  15,92,214                           // subps         %xmm6,%xmm2
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  15,88,214                           // addps         %xmm6,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,29,88,9,0,0                   // movaps        0x958(%rip),%xmm3        # ee0 <_sk_linear_gradient_2stops_sse41+0x21e>
  .byte  255,224                             // jmpq          *%rax

.globl _sk_load_tables_sse41
_sk_load_tables_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  76,139,80,8                         // mov           0x8(%rax),%r10
  .byte  243,69,15,111,12,184                // movdqu        (%r8,%rdi,4),%xmm9
  .byte  102,68,15,111,5,78,9,0,0            // movdqa        0x94e(%rip),%xmm8        # ef0 <_sk_linear_gradient_2stops_sse41+0x22e>
  .byte  102,65,15,111,193                   // movdqa        %xmm9,%xmm0
  .byte  102,65,15,219,192                   // pand          %xmm8,%xmm0
  .byte  102,73,15,58,22,192,1               // pextrq        $0x1,%xmm0,%r8
  .byte  69,137,193                          // mov           %r8d,%r9d
  .byte  73,193,232,32                       // shr           $0x20,%r8
  .byte  102,72,15,126,193                   // movq          %xmm0,%rcx
  .byte  65,137,203                          // mov           %ecx,%r11d
  .byte  72,193,233,32                       // shr           $0x20,%rcx
  .byte  243,67,15,16,4,154                  // movss         (%r10,%r11,4),%xmm0
  .byte  102,65,15,58,33,4,138,16            // insertps      $0x10,(%r10,%rcx,4),%xmm0
  .byte  102,67,15,58,33,4,138,32            // insertps      $0x20,(%r10,%r9,4),%xmm0
  .byte  102,67,15,58,33,4,130,48            // insertps      $0x30,(%r10,%r8,4),%xmm0
  .byte  76,139,80,16                        // mov           0x10(%rax),%r10
  .byte  102,65,15,111,201                   // movdqa        %xmm9,%xmm1
  .byte  102,15,114,209,8                    // psrld         $0x8,%xmm1
  .byte  102,65,15,219,200                   // pand          %xmm8,%xmm1
  .byte  102,73,15,58,22,200,1               // pextrq        $0x1,%xmm1,%r8
  .byte  69,137,193                          // mov           %r8d,%r9d
  .byte  73,193,232,32                       // shr           $0x20,%r8
  .byte  102,72,15,126,201                   // movq          %xmm1,%rcx
  .byte  65,137,203                          // mov           %ecx,%r11d
  .byte  72,193,233,32                       // shr           $0x20,%rcx
  .byte  243,67,15,16,12,154                 // movss         (%r10,%r11,4),%xmm1
  .byte  102,65,15,58,33,12,138,16           // insertps      $0x10,(%r10,%rcx,4),%xmm1
  .byte  243,67,15,16,20,138                 // movss         (%r10,%r9,4),%xmm2
  .byte  102,15,58,33,202,32                 // insertps      $0x20,%xmm2,%xmm1
  .byte  243,67,15,16,20,130                 // movss         (%r10,%r8,4),%xmm2
  .byte  102,15,58,33,202,48                 // insertps      $0x30,%xmm2,%xmm1
  .byte  76,139,72,24                        // mov           0x18(%rax),%r9
  .byte  102,65,15,111,209                   // movdqa        %xmm9,%xmm2
  .byte  102,15,114,210,16                   // psrld         $0x10,%xmm2
  .byte  102,65,15,219,208                   // pand          %xmm8,%xmm2
  .byte  102,72,15,58,22,209,1               // pextrq        $0x1,%xmm2,%rcx
  .byte  65,137,200                          // mov           %ecx,%r8d
  .byte  72,193,233,32                       // shr           $0x20,%rcx
  .byte  102,72,15,126,208                   // movq          %xmm2,%rax
  .byte  65,137,194                          // mov           %eax,%r10d
  .byte  72,193,232,32                       // shr           $0x20,%rax
  .byte  243,67,15,16,20,145                 // movss         (%r9,%r10,4),%xmm2
  .byte  102,65,15,58,33,20,129,16           // insertps      $0x10,(%r9,%rax,4),%xmm2
  .byte  243,67,15,16,28,129                 // movss         (%r9,%r8,4),%xmm3
  .byte  102,15,58,33,211,32                 // insertps      $0x20,%xmm3,%xmm2
  .byte  243,65,15,16,28,137                 // movss         (%r9,%rcx,4),%xmm3
  .byte  102,15,58,33,211,48                 // insertps      $0x30,%xmm3,%xmm2
  .byte  102,65,15,114,209,24                // psrld         $0x18,%xmm9
  .byte  65,15,91,217                        // cvtdq2ps      %xmm9,%xmm3
  .byte  15,89,29,101,8,0,0                  // mulps         0x865(%rip),%xmm3        # f00 <_sk_linear_gradient_2stops_sse41+0x23e>
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_load_a8_sse41
_sk_load_a8_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,15,56,49,4,56                   // pmovzxbd      (%rax,%rdi,1),%xmm0
  .byte  15,91,216                           // cvtdq2ps      %xmm0,%xmm3
  .byte  15,89,29,92,8,0,0                   // mulps         0x85c(%rip),%xmm3        # f10 <_sk_linear_gradient_2stops_sse41+0x24e>
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,87,201                           // xorps         %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  255,224                             // jmpq          *%rax

.globl _sk_store_a8_sse41
_sk_store_a8_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  68,15,40,5,82,8,0,0                 // movaps        0x852(%rip),%xmm8        # f20 <_sk_linear_gradient_2stops_sse41+0x25e>
  .byte  68,15,89,195                        // mulps         %xmm3,%xmm8
  .byte  102,69,15,91,192                    // cvtps2dq      %xmm8,%xmm8
  .byte  102,69,15,56,43,192                 // packusdw      %xmm8,%xmm8
  .byte  102,69,15,103,192                   // packuswb      %xmm8,%xmm8
  .byte  102,68,15,126,4,56                  // movd          %xmm8,(%rax,%rdi,1)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_load_565_sse41
_sk_load_565_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,15,56,51,20,120                 // pmovzxwd      (%rax,%rdi,2),%xmm2
  .byte  102,15,111,5,49,8,0,0               // movdqa        0x831(%rip),%xmm0        # f30 <_sk_linear_gradient_2stops_sse41+0x26e>
  .byte  102,15,219,194                      // pand          %xmm2,%xmm0
  .byte  15,91,192                           // cvtdq2ps      %xmm0,%xmm0
  .byte  15,89,5,51,8,0,0                    // mulps         0x833(%rip),%xmm0        # f40 <_sk_linear_gradient_2stops_sse41+0x27e>
  .byte  102,15,111,13,59,8,0,0              // movdqa        0x83b(%rip),%xmm1        # f50 <_sk_linear_gradient_2stops_sse41+0x28e>
  .byte  102,15,219,202                      // pand          %xmm2,%xmm1
  .byte  15,91,201                           // cvtdq2ps      %xmm1,%xmm1
  .byte  15,89,13,61,8,0,0                   // mulps         0x83d(%rip),%xmm1        # f60 <_sk_linear_gradient_2stops_sse41+0x29e>
  .byte  102,15,219,21,69,8,0,0              // pand          0x845(%rip),%xmm2        # f70 <_sk_linear_gradient_2stops_sse41+0x2ae>
  .byte  15,91,210                           // cvtdq2ps      %xmm2,%xmm2
  .byte  15,89,21,75,8,0,0                   // mulps         0x84b(%rip),%xmm2        # f80 <_sk_linear_gradient_2stops_sse41+0x2be>
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,29,82,8,0,0                   // movaps        0x852(%rip),%xmm3        # f90 <_sk_linear_gradient_2stops_sse41+0x2ce>
  .byte  255,224                             // jmpq          *%rax

.globl _sk_store_565_sse41
_sk_store_565_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  68,15,40,5,83,8,0,0                 // movaps        0x853(%rip),%xmm8        # fa0 <_sk_linear_gradient_2stops_sse41+0x2de>
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  102,69,15,91,201                    // cvtps2dq      %xmm9,%xmm9
  .byte  102,65,15,114,241,11                // pslld         $0xb,%xmm9
  .byte  68,15,40,21,72,8,0,0                // movaps        0x848(%rip),%xmm10        # fb0 <_sk_linear_gradient_2stops_sse41+0x2ee>
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

.globl _sk_load_8888_sse41
_sk_load_8888_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  68,15,16,12,184                     // movups        (%rax,%rdi,4),%xmm9
  .byte  15,40,21,21,8,0,0                   // movaps        0x815(%rip),%xmm2        # fc0 <_sk_linear_gradient_2stops_sse41+0x2fe>
  .byte  65,15,40,193                        // movaps        %xmm9,%xmm0
  .byte  15,84,194                           // andps         %xmm2,%xmm0
  .byte  15,91,192                           // cvtdq2ps      %xmm0,%xmm0
  .byte  68,15,40,5,19,8,0,0                 // movaps        0x813(%rip),%xmm8        # fd0 <_sk_linear_gradient_2stops_sse41+0x30e>
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,40,201                        // movaps        %xmm9,%xmm1
  .byte  102,15,114,209,8                    // psrld         $0x8,%xmm1
  .byte  102,15,219,202                      // pand          %xmm2,%xmm1
  .byte  15,91,201                           // cvtdq2ps      %xmm1,%xmm1
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,40,217                        // movaps        %xmm9,%xmm3
  .byte  102,15,114,211,16                   // psrld         $0x10,%xmm3
  .byte  102,15,219,218                      // pand          %xmm2,%xmm3
  .byte  15,91,211                           // cvtdq2ps      %xmm3,%xmm2
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  102,65,15,114,209,24                // psrld         $0x18,%xmm9
  .byte  65,15,91,217                        // cvtdq2ps      %xmm9,%xmm3
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_store_8888_sse41
_sk_store_8888_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  68,15,40,5,216,7,0,0                // movaps        0x7d8(%rip),%xmm8        # fe0 <_sk_linear_gradient_2stops_sse41+0x31e>
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  102,69,15,91,201                    // cvtps2dq      %xmm9,%xmm9
  .byte  68,15,40,209                        // movaps        %xmm1,%xmm10
  .byte  69,15,89,208                        // mulps         %xmm8,%xmm10
  .byte  102,69,15,91,210                    // cvtps2dq      %xmm10,%xmm10
  .byte  102,65,15,114,242,8                 // pslld         $0x8,%xmm10
  .byte  102,69,15,235,209                   // por           %xmm9,%xmm10
  .byte  68,15,40,202                        // movaps        %xmm2,%xmm9
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
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

.globl _sk_load_f16_sse41
_sk_load_f16_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  243,15,111,4,248                    // movdqu        (%rax,%rdi,8),%xmm0
  .byte  243,15,111,76,248,16                // movdqu        0x10(%rax,%rdi,8),%xmm1
  .byte  102,15,111,208                      // movdqa        %xmm0,%xmm2
  .byte  102,15,97,209                       // punpcklwd     %xmm1,%xmm2
  .byte  102,15,105,193                      // punpckhwd     %xmm1,%xmm0
  .byte  102,68,15,111,194                   // movdqa        %xmm2,%xmm8
  .byte  102,68,15,97,192                    // punpcklwd     %xmm0,%xmm8
  .byte  102,15,105,208                      // punpckhwd     %xmm0,%xmm2
  .byte  102,15,111,29,91,7,0,0              // movdqa        0x75b(%rip),%xmm3        # ff0 <_sk_linear_gradient_2stops_sse41+0x32e>
  .byte  102,15,111,203                      // movdqa        %xmm3,%xmm1
  .byte  102,65,15,101,200                   // pcmpgtw       %xmm8,%xmm1
  .byte  102,65,15,223,200                   // pandn         %xmm8,%xmm1
  .byte  102,15,101,218                      // pcmpgtw       %xmm2,%xmm3
  .byte  102,15,223,218                      // pandn         %xmm2,%xmm3
  .byte  102,15,56,51,193                    // pmovzxwd      %xmm1,%xmm0
  .byte  102,15,114,240,13                   // pslld         $0xd,%xmm0
  .byte  68,15,40,5,67,7,0,0                 // movaps        0x743(%rip),%xmm8        # 1000 <_sk_linear_gradient_2stops_sse41+0x33e>
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  102,69,15,239,201                   // pxor          %xmm9,%xmm9
  .byte  102,65,15,105,201                   // punpckhwd     %xmm9,%xmm1
  .byte  102,15,114,241,13                   // pslld         $0xd,%xmm1
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  102,15,56,51,211                    // pmovzxwd      %xmm3,%xmm2
  .byte  102,15,114,242,13                   // pslld         $0xd,%xmm2
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  102,65,15,105,217                   // punpckhwd     %xmm9,%xmm3
  .byte  102,15,114,243,13                   // pslld         $0xd,%xmm3
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_store_f16_sse41
_sk_store_f16_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  68,15,40,5,15,7,0,0                 // movaps        0x70f(%rip),%xmm8        # 1010 <_sk_linear_gradient_2stops_sse41+0x34e>
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  102,65,15,114,209,13                // psrld         $0xd,%xmm9
  .byte  68,15,40,209                        // movaps        %xmm1,%xmm10
  .byte  69,15,89,208                        // mulps         %xmm8,%xmm10
  .byte  102,65,15,114,210,13                // psrld         $0xd,%xmm10
  .byte  68,15,40,218                        // movaps        %xmm2,%xmm11
  .byte  69,15,89,216                        // mulps         %xmm8,%xmm11
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

.balign 16
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,63                                // add           %bh,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,63                                // add           %bh,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,63                                // add           %bh,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,63                                // add           %bh,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  64,0,0                              // add           %al,(%rax)
  .byte  64                                  // rex
  .byte  64,0,0                              // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  145                                 // xchg          %eax,%ecx
  .byte  131,158,61,145,131,158,61           // sbbl          $0x3d,-0x617c6ec3(%rsi)
  .byte  145                                 // xchg          %eax,%ecx
  .byte  131,158,61,145,131,158,61           // sbbl          $0x3d,-0x617c6ec3(%rsi)
  .byte  154                                 // (bad)
  .byte  153                                 // cltd
  .byte  153                                 // cltd
  .byte  62,154                              // ds            (bad)
  .byte  153                                 // cltd
  .byte  153                                 // cltd
  .byte  62,154                              // ds            (bad)
  .byte  153                                 // cltd
  .byte  153                                 // cltd
  .byte  62,154                              // ds            (bad)
  .byte  153                                 // cltd
  .byte  153                                 // cltd
  .byte  62,92                               // ds            pop %rsp
  .byte  143                                 // (bad)
  .byte  50,63                               // xor           (%rdi),%bh
  .byte  92                                  // pop           %rsp
  .byte  143                                 // (bad)
  .byte  50,63                               // xor           (%rdi),%bh
  .byte  92                                  // pop           %rsp
  .byte  143                                 // (bad)
  .byte  50,63                               // xor           (%rdi),%bh
  .byte  92                                  // pop           %rsp
  .byte  143                                 // (bad)
  .byte  50,63                               // xor           (%rdi),%bh
  .byte  10,215                              // or            %bh,%dl
  .byte  35,59                               // and           (%rbx),%edi
  .byte  10,215                              // or            %bh,%dl
  .byte  35,59                               // and           (%rbx),%edi
  .byte  10,215                              // or            %bh,%dl
  .byte  35,59                               // and           (%rbx),%edi
  .byte  10,215                              // or            %bh,%dl
  .byte  35,59                               // and           (%rbx),%edi
  .byte  174                                 // scas          %es:(%rdi),%al
  .byte  71,97                               // rex.RXB       (bad)
  .byte  61,174,71,97,61                     // cmp           $0x3d6147ae,%eax
  .byte  174                                 // scas          %es:(%rdi),%al
  .byte  71,97                               // rex.RXB       (bad)
  .byte  61,174,71,97,61                     // cmp           $0x3d6147ae,%eax
  .byte  41,92,71,65                         // sub           %ebx,0x41(%rdi,%rax,2)
  .byte  41,92,71,65                         // sub           %ebx,0x41(%rdi,%rax,2)
  .byte  41,92,71,65                         // sub           %ebx,0x41(%rdi,%rax,2)
  .byte  41,92,71,65                         // sub           %ebx,0x41(%rdi,%rax,2)
  .byte  206                                 // (bad)
  .byte  111                                 // outsl         %ds:(%rsi),(%dx)
  .byte  48,63                               // xor           %bh,(%rdi)
  .byte  206                                 // (bad)
  .byte  111                                 // outsl         %ds:(%rsi),(%dx)
  .byte  48,63                               // xor           %bh,(%rdi)
  .byte  206                                 // (bad)
  .byte  111                                 // outsl         %ds:(%rsi),(%dx)
  .byte  48,63                               // xor           %bh,(%rdi)
  .byte  206                                 // (bad)
  .byte  111                                 // outsl         %ds:(%rsi),(%dx)
  .byte  48,63                               // xor           %bh,(%rdi)
  .byte  168,87                              // test          $0x57,%al
  .byte  202,189,168                         // lret          $0xa8bd
  .byte  87                                  // push          %rdi
  .byte  202,189,168                         // lret          $0xa8bd
  .byte  87                                  // push          %rdi
  .byte  202,189,168                         // lret          $0xa8bd
  .byte  87                                  // push          %rdi
  .byte  202,189,194                         // lret          $0xc2bd
  .byte  135,210                             // xchg          %edx,%edx
  .byte  62,194,135,210                      // ds            retq $0xd287
  .byte  62,194,135,210                      // ds            retq $0xd287
  .byte  62,194,135,210                      // ds            retq $0xd287
  .byte  62,0,0                              // add           %al,%ds:(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,4                            // cmpb          $0x4,(%rdi)
  .byte  231,140                             // out           %eax,$0x8c
  .byte  59,4,231                            // cmp           (%rdi,%riz,8),%eax
  .byte  140,59                              // mov           %?,(%rbx)
  .byte  4,231                               // add           $0xe7,%al
  .byte  140,59                              // mov           %?,(%rbx)
  .byte  4,231                               // add           $0xe7,%al
  .byte  140,59                              // mov           %?,(%rbx)
  .byte  129,128,128,59,129,128,128,59,129,128// addl          $0x80813b80,-0x7f7ec480(%rax)
  .byte  128,59,129                          // cmpb          $0x81,(%rbx)
  .byte  128,128,59,129,128,128,59           // addb          $0x3b,-0x7f7f7ec5(%rax)
  .byte  129,128,128,59,129,128,128,59,129,128// addl          $0x80813b80,-0x7f7ec480(%rax)
  .byte  128,59,0                            // cmpb          $0x0,(%rbx)
  .byte  248                                 // clc
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,248                               // add           %bh,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,248                               // add           %bh,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,248                               // add           %bh,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  224,7                               // loopne        ea9 <.literal16+0x179>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  224,7                               // loopne        ead <.literal16+0x17d>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  224,7                               // loopne        eb1 <.literal16+0x181>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  224,7                               // loopne        eb5 <.literal16+0x185>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  31                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,31                                // add           %bl,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,31                                // add           %bl,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,31                                // add           %bl,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,8                                 // add           %cl,(%rax)
  .byte  33,4,61,8,33,4,61                   // and           %eax,0x3d042108(,%rdi,1)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  4,61                                // add           $0x3d,%al
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  4,61                                // add           $0x3d,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,255                          // cmpb          $0xff,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,255                               // add           %bh,%bh
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,255                               // add           %bh,%bh
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,255                               // add           %bh,%bh
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,129,128,128,59,129                // add           %al,-0x7ec47f80(%rcx)
  .byte  128,128,59,129,128,128,59           // addb          $0x3b,-0x7f7f7ec5(%rax)
  .byte  129,128,128,59,129,128,128,59,129,128// addl          $0x80813b80,-0x7f7ec480(%rax)
  .byte  128,59,129                          // cmpb          $0x81,(%rbx)
  .byte  128,128,59,129,128,128,59           // addb          $0x3b,-0x7f7f7ec5(%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  127,67                              // jg            f67 <.literal16+0x237>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  127,67                              // jg            f6b <.literal16+0x23b>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  127,67                              // jg            f6f <.literal16+0x23f>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  127,67                              // jg            f73 <.literal16+0x243>
  .byte  0,248                               // add           %bh,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,248                               // add           %bh,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,248                               // add           %bh,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,248                               // add           %bh,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  224,7                               // loopne        f59 <.literal16+0x229>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  224,7                               // loopne        f5d <.literal16+0x22d>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  224,7                               // loopne        f61 <.literal16+0x231>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  224,7                               // loopne        f65 <.literal16+0x235>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  31                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,31                                // add           %bl,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,31                                // add           %bl,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,31                                // add           %bl,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,8                                 // add           %cl,(%rax)
  .byte  33,4,61,8,33,4,61                   // and           %eax,0x3d042108(,%rdi,1)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  4,61                                // add           $0x3d,%al
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  4,61                                // add           $0x3d,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,248                               // add           %bh,%al
  .byte  65,0,0                              // add           %al,(%r8)
  .byte  248                                 // clc
  .byte  65,0,0                              // add           %al,(%r8)
  .byte  248                                 // clc
  .byte  65,0,0                              // add           %al,(%r8)
  .byte  248                                 // clc
  .byte  65,0,0                              // add           %al,(%r8)
  .byte  124,66                              // jl            ff6 <.literal16+0x2c6>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  124,66                              // jl            ffa <.literal16+0x2ca>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  124,66                              // jl            ffe <.literal16+0x2ce>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  124,66                              // jl            1002 <.literal16+0x2d2>
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  129,128,128,59,129,128,128,59,129,128// addl          $0x80813b80,-0x7f7ec480(%rax)
  .byte  128,59,129                          // cmpb          $0x81,(%rbx)
  .byte  128,128,59,0,0,127,67               // addb          $0x43,0x7f00003b(%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  127,67                              // jg            102b <_sk_linear_gradient_2stops_sse41+0x369>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  127,67                              // jg            102f <_sk_linear_gradient_2stops_sse41+0x36d>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  127,67                              // jg            1033 <_sk_linear_gradient_2stops_sse41+0x371>
  .byte  0,4,0                               // add           %al,(%rax,%rax,1)
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  0,128,119,0,0,128                   // add           %al,-0x7fffff89(%rax)
  .byte  119,0                               // ja            1009 <.literal16+0x2d9>
  .byte  0,128,119,0,0,128                   // add           %al,-0x7fffff89(%rax)
  .byte  119,0                               // ja            1011 <.literal16+0x2e1>
  .byte  0,128,7,0,0,128                     // add           %al,-0x7ffffff9(%rax)
  .byte  7                                   // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,7,0                             // addb          $0x0,(%rdi)
  .byte  0                                   // .byte         0x0
  .byte  128                                 // .byte         0x80
  .byte  7                                   // (bad)
.balign 16

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

.globl _sk_just_return_sse2
_sk_just_return_sse2:
  .byte  195                                 // retq

.globl _sk_seed_shader_sse2
_sk_seed_shader_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  102,15,110,199                      // movd          %edi,%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  15,91,192                           // cvtdq2ps      %xmm0,%xmm0
  .byte  15,40,21,36,13,0,0                  // movaps        0xd24(%rip),%xmm2        # da0 <_sk_linear_gradient_2stops_sse2+0x68>
  .byte  15,88,194                           // addps         %xmm2,%xmm0
  .byte  15,88,5,42,13,0,0                   // addps         0xd2a(%rip),%xmm0        # db0 <_sk_linear_gradient_2stops_sse2+0x78>
  .byte  102,15,110,8                        // movd          (%rax),%xmm1
  .byte  102,15,112,201,0                    // pshufd        $0x0,%xmm1,%xmm1
  .byte  15,91,201                           // cvtdq2ps      %xmm1,%xmm1
  .byte  15,88,202                           // addps         %xmm2,%xmm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,21,34,13,0,0                  // movaps        0xd22(%rip),%xmm2        # dc0 <_sk_linear_gradient_2stops_sse2+0x88>
  .byte  15,87,219                           // xorps         %xmm3,%xmm3
  .byte  15,87,228                           // xorps         %xmm4,%xmm4
  .byte  15,87,237                           // xorps         %xmm5,%xmm5
  .byte  15,87,246                           // xorps         %xmm6,%xmm6
  .byte  15,87,255                           // xorps         %xmm7,%xmm7
  .byte  255,224                             // jmpq          *%rax

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

.globl _sk_clear_sse2
_sk_clear_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,87,201                           // xorps         %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  15,87,219                           // xorps         %xmm3,%xmm3
  .byte  255,224                             // jmpq          *%rax

.globl _sk_plus__sse2
_sk_plus__sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  15,88,214                           // addps         %xmm6,%xmm2
  .byte  15,88,223                           // addps         %xmm7,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_srcover_sse2
_sk_srcover_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  68,15,40,5,209,12,0,0               // movaps        0xcd1(%rip),%xmm8        # dd0 <_sk_linear_gradient_2stops_sse2+0x98>
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

.globl _sk_dstover_sse2
_sk_dstover_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  68,15,40,5,163,12,0,0               // movaps        0xca3(%rip),%xmm8        # de0 <_sk_linear_gradient_2stops_sse2+0xa8>
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

.globl _sk_clamp_0_sse2
_sk_clamp_0_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  69,15,87,192                        // xorps         %xmm8,%xmm8
  .byte  65,15,95,192                        // maxps         %xmm8,%xmm0
  .byte  65,15,95,200                        // maxps         %xmm8,%xmm1
  .byte  65,15,95,208                        // maxps         %xmm8,%xmm2
  .byte  65,15,95,216                        // maxps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_clamp_1_sse2
_sk_clamp_1_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  68,15,40,5,107,12,0,0               // movaps        0xc6b(%rip),%xmm8        # df0 <_sk_linear_gradient_2stops_sse2+0xb8>
  .byte  65,15,93,192                        // minps         %xmm8,%xmm0
  .byte  65,15,93,200                        // minps         %xmm8,%xmm1
  .byte  65,15,93,208                        // minps         %xmm8,%xmm2
  .byte  65,15,93,216                        // minps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_clamp_a_sse2
_sk_clamp_a_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,93,29,94,12,0,0                  // minps         0xc5e(%rip),%xmm3        # e00 <_sk_linear_gradient_2stops_sse2+0xc8>
  .byte  15,93,195                           // minps         %xmm3,%xmm0
  .byte  15,93,203                           // minps         %xmm3,%xmm1
  .byte  15,93,211                           // minps         %xmm3,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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

.globl _sk_swap_rb_sse2
_sk_swap_rb_sse2:
  .byte  68,15,40,192                        // movaps        %xmm0,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,194                           // movaps        %xmm2,%xmm0
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  255,224                             // jmpq          *%rax

.globl _sk_swap_sse2
_sk_swap_sse2:
  .byte  68,15,40,195                        // movaps        %xmm3,%xmm8
  .byte  68,15,40,202                        // movaps        %xmm2,%xmm9
  .byte  68,15,40,209                        // movaps        %xmm1,%xmm10
  .byte  68,15,40,216                        // movaps        %xmm0,%xmm11
  .byte  72,173                              // lods          %ds:(%rsi),%rax
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

.globl _sk_move_src_dst_sse2
_sk_move_src_dst_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,224                           // movaps        %xmm0,%xmm4
  .byte  15,40,233                           // movaps        %xmm1,%xmm5
  .byte  15,40,242                           // movaps        %xmm2,%xmm6
  .byte  15,40,251                           // movaps        %xmm3,%xmm7
  .byte  255,224                             // jmpq          *%rax

.globl _sk_move_dst_src_sse2
_sk_move_dst_src_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,196                           // movaps        %xmm4,%xmm0
  .byte  15,40,205                           // movaps        %xmm5,%xmm1
  .byte  15,40,214                           // movaps        %xmm6,%xmm2
  .byte  15,40,223                           // movaps        %xmm7,%xmm3
  .byte  255,224                             // jmpq          *%rax

.globl _sk_premul_sse2
_sk_premul_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,89,195                           // mulps         %xmm3,%xmm0
  .byte  15,89,203                           // mulps         %xmm3,%xmm1
  .byte  15,89,211                           // mulps         %xmm3,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_unpremul_sse2
_sk_unpremul_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  69,15,87,192                        // xorps         %xmm8,%xmm8
  .byte  68,15,194,195,0                     // cmpeqps       %xmm3,%xmm8
  .byte  68,15,40,13,184,11,0,0              // movaps        0xbb8(%rip),%xmm9        # e10 <_sk_linear_gradient_2stops_sse2+0xd8>
  .byte  68,15,94,203                        // divps         %xmm3,%xmm9
  .byte  69,15,85,193                        // andnps        %xmm9,%xmm8
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_from_srgb_sse2
_sk_from_srgb_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  68,15,40,5,166,11,0,0               // movaps        0xba6(%rip),%xmm8        # e20 <_sk_linear_gradient_2stops_sse2+0xe8>
  .byte  68,15,40,232                        // movaps        %xmm0,%xmm13
  .byte  69,15,89,232                        // mulps         %xmm8,%xmm13
  .byte  68,15,40,216                        // movaps        %xmm0,%xmm11
  .byte  69,15,89,219                        // mulps         %xmm11,%xmm11
  .byte  68,15,40,13,158,11,0,0              // movaps        0xb9e(%rip),%xmm9        # e30 <_sk_linear_gradient_2stops_sse2+0xf8>
  .byte  68,15,40,240                        // movaps        %xmm0,%xmm14
  .byte  69,15,89,241                        // mulps         %xmm9,%xmm14
  .byte  68,15,40,21,158,11,0,0              // movaps        0xb9e(%rip),%xmm10        # e40 <_sk_linear_gradient_2stops_sse2+0x108>
  .byte  69,15,88,242                        // addps         %xmm10,%xmm14
  .byte  69,15,89,243                        // mulps         %xmm11,%xmm14
  .byte  68,15,40,29,158,11,0,0              // movaps        0xb9e(%rip),%xmm11        # e50 <_sk_linear_gradient_2stops_sse2+0x118>
  .byte  69,15,88,243                        // addps         %xmm11,%xmm14
  .byte  68,15,40,37,162,11,0,0              // movaps        0xba2(%rip),%xmm12        # e60 <_sk_linear_gradient_2stops_sse2+0x128>
  .byte  65,15,194,196,1                     // cmpltps       %xmm12,%xmm0
  .byte  68,15,84,232                        // andps         %xmm0,%xmm13
  .byte  65,15,85,198                        // andnps        %xmm14,%xmm0
  .byte  65,15,86,197                        // orps          %xmm13,%xmm0
  .byte  68,15,40,233                        // movaps        %xmm1,%xmm13
  .byte  69,15,89,232                        // mulps         %xmm8,%xmm13
  .byte  68,15,40,241                        // movaps        %xmm1,%xmm14
  .byte  69,15,89,246                        // mulps         %xmm14,%xmm14
  .byte  68,15,40,249                        // movaps        %xmm1,%xmm15
  .byte  69,15,89,249                        // mulps         %xmm9,%xmm15
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

.globl _sk_to_srgb_sse2
_sk_to_srgb_sse2:
  .byte  68,15,82,192                        // rsqrtps       %xmm0,%xmm8
  .byte  69,15,83,200                        // rcpps         %xmm8,%xmm9
  .byte  69,15,82,232                        // rsqrtps       %xmm8,%xmm13
  .byte  68,15,40,5,39,11,0,0                // movaps        0xb27(%rip),%xmm8        # e70 <_sk_linear_gradient_2stops_sse2+0x138>
  .byte  68,15,40,240                        // movaps        %xmm0,%xmm14
  .byte  69,15,89,240                        // mulps         %xmm8,%xmm14
  .byte  68,15,40,21,39,11,0,0               // movaps        0xb27(%rip),%xmm10        # e80 <_sk_linear_gradient_2stops_sse2+0x148>
  .byte  69,15,89,202                        // mulps         %xmm10,%xmm9
  .byte  68,15,40,29,43,11,0,0               // movaps        0xb2b(%rip),%xmm11        # e90 <_sk_linear_gradient_2stops_sse2+0x158>
  .byte  69,15,88,203                        // addps         %xmm11,%xmm9
  .byte  68,15,40,37,47,11,0,0               // movaps        0xb2f(%rip),%xmm12        # ea0 <_sk_linear_gradient_2stops_sse2+0x168>
  .byte  69,15,89,236                        // mulps         %xmm12,%xmm13
  .byte  69,15,88,233                        // addps         %xmm9,%xmm13
  .byte  68,15,40,13,47,11,0,0               // movaps        0xb2f(%rip),%xmm9        # eb0 <_sk_linear_gradient_2stops_sse2+0x178>
  .byte  69,15,40,249                        // movaps        %xmm9,%xmm15
  .byte  69,15,93,253                        // minps         %xmm13,%xmm15
  .byte  68,15,40,45,47,11,0,0               // movaps        0xb2f(%rip),%xmm13        # ec0 <_sk_linear_gradient_2stops_sse2+0x188>
  .byte  65,15,194,197,1                     // cmpltps       %xmm13,%xmm0
  .byte  68,15,84,240                        // andps         %xmm0,%xmm14
  .byte  65,15,85,199                        // andnps        %xmm15,%xmm0
  .byte  65,15,86,198                        // orps          %xmm14,%xmm0
  .byte  68,15,82,241                        // rsqrtps       %xmm1,%xmm14
  .byte  69,15,83,254                        // rcpps         %xmm14,%xmm15
  .byte  69,15,82,246                        // rsqrtps       %xmm14,%xmm14
  .byte  69,15,89,250                        // mulps         %xmm10,%xmm15
  .byte  69,15,88,251                        // addps         %xmm11,%xmm15
  .byte  69,15,89,244                        // mulps         %xmm12,%xmm14
  .byte  69,15,88,247                        // addps         %xmm15,%xmm14
  .byte  69,15,40,249                        // movaps        %xmm9,%xmm15
  .byte  69,15,93,254                        // minps         %xmm14,%xmm15
  .byte  68,15,40,241                        // movaps        %xmm1,%xmm14
  .byte  69,15,89,240                        // mulps         %xmm8,%xmm14
  .byte  65,15,194,205,1                     // cmpltps       %xmm13,%xmm1
  .byte  68,15,84,241                        // andps         %xmm1,%xmm14
  .byte  65,15,85,207                        // andnps        %xmm15,%xmm1
  .byte  65,15,86,206                        // orps          %xmm14,%xmm1
  .byte  68,15,82,242                        // rsqrtps       %xmm2,%xmm14
  .byte  69,15,83,254                        // rcpps         %xmm14,%xmm15
  .byte  69,15,89,250                        // mulps         %xmm10,%xmm15
  .byte  69,15,88,251                        // addps         %xmm11,%xmm15
  .byte  69,15,82,214                        // rsqrtps       %xmm14,%xmm10
  .byte  69,15,89,212                        // mulps         %xmm12,%xmm10
  .byte  69,15,88,215                        // addps         %xmm15,%xmm10
  .byte  69,15,93,202                        // minps         %xmm10,%xmm9
  .byte  68,15,89,194                        // mulps         %xmm2,%xmm8
  .byte  65,15,194,213,1                     // cmpltps       %xmm13,%xmm2
  .byte  68,15,84,194                        // andps         %xmm2,%xmm8
  .byte  65,15,85,209                        // andnps        %xmm9,%xmm2
  .byte  65,15,86,208                        // orps          %xmm8,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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

.globl _sk_scale_u8_sse2
_sk_scale_u8_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,68,15,110,4,56                  // movd          (%rax,%rdi,1),%xmm8
  .byte  102,69,15,239,201                   // pxor          %xmm9,%xmm9
  .byte  102,69,15,96,193                    // punpcklbw     %xmm9,%xmm8
  .byte  102,69,15,97,193                    // punpcklwd     %xmm9,%xmm8
  .byte  69,15,91,192                        // cvtdq2ps      %xmm8,%xmm8
  .byte  68,15,89,5,112,10,0,0               // mulps         0xa70(%rip),%xmm8        # ed0 <_sk_linear_gradient_2stops_sse2+0x198>
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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

.globl _sk_lerp_u8_sse2
_sk_lerp_u8_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,68,15,110,4,56                  // movd          (%rax,%rdi,1),%xmm8
  .byte  102,69,15,239,201                   // pxor          %xmm9,%xmm9
  .byte  102,69,15,96,193                    // punpcklbw     %xmm9,%xmm8
  .byte  102,69,15,97,193                    // punpcklwd     %xmm9,%xmm8
  .byte  69,15,91,192                        // cvtdq2ps      %xmm8,%xmm8
  .byte  68,15,89,5,14,10,0,0                // mulps         0xa0e(%rip),%xmm8        # ee0 <_sk_linear_gradient_2stops_sse2+0x1a8>
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

.globl _sk_lerp_565_sse2
_sk_lerp_565_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  243,68,15,126,4,120                 // movq          (%rax,%rdi,2),%xmm8
  .byte  102,15,239,219                      // pxor          %xmm3,%xmm3
  .byte  102,68,15,97,195                    // punpcklwd     %xmm3,%xmm8
  .byte  102,15,111,29,214,9,0,0             // movdqa        0x9d6(%rip),%xmm3        # ef0 <_sk_linear_gradient_2stops_sse2+0x1b8>
  .byte  102,65,15,219,216                   // pand          %xmm8,%xmm3
  .byte  68,15,91,203                        // cvtdq2ps      %xmm3,%xmm9
  .byte  68,15,89,13,213,9,0,0               // mulps         0x9d5(%rip),%xmm9        # f00 <_sk_linear_gradient_2stops_sse2+0x1c8>
  .byte  102,15,111,29,221,9,0,0             // movdqa        0x9dd(%rip),%xmm3        # f10 <_sk_linear_gradient_2stops_sse2+0x1d8>
  .byte  102,65,15,219,216                   // pand          %xmm8,%xmm3
  .byte  15,91,219                           // cvtdq2ps      %xmm3,%xmm3
  .byte  15,89,29,222,9,0,0                  // mulps         0x9de(%rip),%xmm3        # f20 <_sk_linear_gradient_2stops_sse2+0x1e8>
  .byte  102,68,15,219,5,229,9,0,0           // pand          0x9e5(%rip),%xmm8        # f30 <_sk_linear_gradient_2stops_sse2+0x1f8>
  .byte  69,15,91,192                        // cvtdq2ps      %xmm8,%xmm8
  .byte  68,15,89,5,233,9,0,0                // mulps         0x9e9(%rip),%xmm8        # f40 <_sk_linear_gradient_2stops_sse2+0x208>
  .byte  15,92,196                           // subps         %xmm4,%xmm0
  .byte  65,15,89,193                        // mulps         %xmm9,%xmm0
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  15,92,205                           // subps         %xmm5,%xmm1
  .byte  15,89,203                           // mulps         %xmm3,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  15,92,214                           // subps         %xmm6,%xmm2
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  15,88,214                           // addps         %xmm6,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,29,211,9,0,0                  // movaps        0x9d3(%rip),%xmm3        # f50 <_sk_linear_gradient_2stops_sse2+0x218>
  .byte  255,224                             // jmpq          *%rax

.globl _sk_load_tables_sse2
_sk_load_tables_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  76,139,80,8                         // mov           0x8(%rax),%r10
  .byte  243,69,15,111,12,184                // movdqu        (%r8,%rdi,4),%xmm9
  .byte  102,68,15,111,5,201,9,0,0           // movdqa        0x9c9(%rip),%xmm8        # f60 <_sk_linear_gradient_2stops_sse2+0x228>
  .byte  102,65,15,111,193                   // movdqa        %xmm9,%xmm0
  .byte  102,65,15,219,192                   // pand          %xmm8,%xmm0
  .byte  102,15,112,200,78                   // pshufd        $0x4e,%xmm0,%xmm1
  .byte  102,73,15,126,200                   // movq          %xmm1,%r8
  .byte  69,137,193                          // mov           %r8d,%r9d
  .byte  73,193,232,32                       // shr           $0x20,%r8
  .byte  102,72,15,126,193                   // movq          %xmm0,%rcx
  .byte  65,137,203                          // mov           %ecx,%r11d
  .byte  72,193,233,32                       // shr           $0x20,%rcx
  .byte  243,65,15,16,12,138                 // movss         (%r10,%rcx,4),%xmm1
  .byte  243,67,15,16,4,130                  // movss         (%r10,%r8,4),%xmm0
  .byte  15,20,200                           // unpcklps      %xmm0,%xmm1
  .byte  243,67,15,16,4,154                  // movss         (%r10,%r11,4),%xmm0
  .byte  243,67,15,16,20,138                 // movss         (%r10,%r9,4),%xmm2
  .byte  15,20,194                           // unpcklps      %xmm2,%xmm0
  .byte  15,20,193                           // unpcklps      %xmm1,%xmm0
  .byte  76,139,80,16                        // mov           0x10(%rax),%r10
  .byte  102,65,15,111,201                   // movdqa        %xmm9,%xmm1
  .byte  102,15,114,209,8                    // psrld         $0x8,%xmm1
  .byte  102,65,15,219,200                   // pand          %xmm8,%xmm1
  .byte  102,15,112,209,78                   // pshufd        $0x4e,%xmm1,%xmm2
  .byte  102,73,15,126,208                   // movq          %xmm2,%r8
  .byte  69,137,193                          // mov           %r8d,%r9d
  .byte  73,193,232,32                       // shr           $0x20,%r8
  .byte  102,72,15,126,201                   // movq          %xmm1,%rcx
  .byte  65,137,203                          // mov           %ecx,%r11d
  .byte  72,193,233,32                       // shr           $0x20,%rcx
  .byte  243,65,15,16,20,138                 // movss         (%r10,%rcx,4),%xmm2
  .byte  243,67,15,16,12,130                 // movss         (%r10,%r8,4),%xmm1
  .byte  15,20,209                           // unpcklps      %xmm1,%xmm2
  .byte  243,67,15,16,12,154                 // movss         (%r10,%r11,4),%xmm1
  .byte  243,67,15,16,28,138                 // movss         (%r10,%r9,4),%xmm3
  .byte  15,20,203                           // unpcklps      %xmm3,%xmm1
  .byte  15,20,202                           // unpcklps      %xmm2,%xmm1
  .byte  76,139,72,24                        // mov           0x18(%rax),%r9
  .byte  102,65,15,111,209                   // movdqa        %xmm9,%xmm2
  .byte  102,15,114,210,16                   // psrld         $0x10,%xmm2
  .byte  102,65,15,219,208                   // pand          %xmm8,%xmm2
  .byte  102,15,112,218,78                   // pshufd        $0x4e,%xmm2,%xmm3
  .byte  102,72,15,126,217                   // movq          %xmm3,%rcx
  .byte  65,137,200                          // mov           %ecx,%r8d
  .byte  72,193,233,32                       // shr           $0x20,%rcx
  .byte  102,72,15,126,208                   // movq          %xmm2,%rax
  .byte  65,137,194                          // mov           %eax,%r10d
  .byte  72,193,232,32                       // shr           $0x20,%rax
  .byte  243,69,15,16,4,129                  // movss         (%r9,%rax,4),%xmm8
  .byte  243,65,15,16,20,137                 // movss         (%r9,%rcx,4),%xmm2
  .byte  68,15,20,194                        // unpcklps      %xmm2,%xmm8
  .byte  243,67,15,16,20,145                 // movss         (%r9,%r10,4),%xmm2
  .byte  243,67,15,16,28,129                 // movss         (%r9,%r8,4),%xmm3
  .byte  15,20,211                           // unpcklps      %xmm3,%xmm2
  .byte  65,15,20,208                        // unpcklps      %xmm8,%xmm2
  .byte  102,65,15,114,209,24                // psrld         $0x18,%xmm9
  .byte  65,15,91,217                        // cvtdq2ps      %xmm9,%xmm3
  .byte  15,89,29,220,8,0,0                  // mulps         0x8dc(%rip),%xmm3        # f70 <_sk_linear_gradient_2stops_sse2+0x238>
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_load_a8_sse2
_sk_load_a8_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  102,15,110,4,56                     // movd          (%rax,%rdi,1),%xmm0
  .byte  102,15,239,201                      // pxor          %xmm1,%xmm1
  .byte  102,15,96,193                       // punpcklbw     %xmm1,%xmm0
  .byte  102,15,97,193                       // punpcklwd     %xmm1,%xmm0
  .byte  15,91,216                           // cvtdq2ps      %xmm0,%xmm3
  .byte  15,89,29,200,8,0,0                  // mulps         0x8c8(%rip),%xmm3        # f80 <_sk_linear_gradient_2stops_sse2+0x248>
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  102,15,239,201                      // pxor          %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  255,224                             // jmpq          *%rax

.globl _sk_store_a8_sse2
_sk_store_a8_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  68,15,40,5,189,8,0,0                // movaps        0x8bd(%rip),%xmm8        # f90 <_sk_linear_gradient_2stops_sse2+0x258>
  .byte  68,15,89,195                        // mulps         %xmm3,%xmm8
  .byte  102,69,15,91,192                    // cvtps2dq      %xmm8,%xmm8
  .byte  102,65,15,114,240,16                // pslld         $0x10,%xmm8
  .byte  102,65,15,114,224,16                // psrad         $0x10,%xmm8
  .byte  102,69,15,107,192                   // packssdw      %xmm8,%xmm8
  .byte  102,69,15,103,192                   // packuswb      %xmm8,%xmm8
  .byte  102,68,15,126,4,56                  // movd          %xmm8,(%rax,%rdi,1)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_load_565_sse2
_sk_load_565_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  243,15,126,20,120                   // movq          (%rax,%rdi,2),%xmm2
  .byte  102,15,239,192                      // pxor          %xmm0,%xmm0
  .byte  102,15,97,208                       // punpcklwd     %xmm0,%xmm2
  .byte  102,15,111,5,138,8,0,0              // movdqa        0x88a(%rip),%xmm0        # fa0 <_sk_linear_gradient_2stops_sse2+0x268>
  .byte  102,15,219,194                      // pand          %xmm2,%xmm0
  .byte  15,91,192                           // cvtdq2ps      %xmm0,%xmm0
  .byte  15,89,5,140,8,0,0                   // mulps         0x88c(%rip),%xmm0        # fb0 <_sk_linear_gradient_2stops_sse2+0x278>
  .byte  102,15,111,13,148,8,0,0             // movdqa        0x894(%rip),%xmm1        # fc0 <_sk_linear_gradient_2stops_sse2+0x288>
  .byte  102,15,219,202                      // pand          %xmm2,%xmm1
  .byte  15,91,201                           // cvtdq2ps      %xmm1,%xmm1
  .byte  15,89,13,150,8,0,0                  // mulps         0x896(%rip),%xmm1        # fd0 <_sk_linear_gradient_2stops_sse2+0x298>
  .byte  102,15,219,21,158,8,0,0             // pand          0x89e(%rip),%xmm2        # fe0 <_sk_linear_gradient_2stops_sse2+0x2a8>
  .byte  15,91,210                           // cvtdq2ps      %xmm2,%xmm2
  .byte  15,89,21,164,8,0,0                  // mulps         0x8a4(%rip),%xmm2        # ff0 <_sk_linear_gradient_2stops_sse2+0x2b8>
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,29,171,8,0,0                  // movaps        0x8ab(%rip),%xmm3        # 1000 <_sk_linear_gradient_2stops_sse2+0x2c8>
  .byte  255,224                             // jmpq          *%rax

.globl _sk_store_565_sse2
_sk_store_565_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  68,15,40,5,172,8,0,0                // movaps        0x8ac(%rip),%xmm8        # 1010 <_sk_linear_gradient_2stops_sse2+0x2d8>
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  102,69,15,91,201                    // cvtps2dq      %xmm9,%xmm9
  .byte  102,65,15,114,241,11                // pslld         $0xb,%xmm9
  .byte  68,15,40,21,161,8,0,0               // movaps        0x8a1(%rip),%xmm10        # 1020 <_sk_linear_gradient_2stops_sse2+0x2e8>
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

.globl _sk_load_8888_sse2
_sk_load_8888_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  68,15,16,12,184                     // movups        (%rax,%rdi,4),%xmm9
  .byte  15,40,21,99,8,0,0                   // movaps        0x863(%rip),%xmm2        # 1030 <_sk_linear_gradient_2stops_sse2+0x2f8>
  .byte  65,15,40,193                        // movaps        %xmm9,%xmm0
  .byte  15,84,194                           // andps         %xmm2,%xmm0
  .byte  15,91,192                           // cvtdq2ps      %xmm0,%xmm0
  .byte  68,15,40,5,97,8,0,0                 // movaps        0x861(%rip),%xmm8        # 1040 <_sk_linear_gradient_2stops_sse2+0x308>
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,40,201                        // movaps        %xmm9,%xmm1
  .byte  102,15,114,209,8                    // psrld         $0x8,%xmm1
  .byte  102,15,219,202                      // pand          %xmm2,%xmm1
  .byte  15,91,201                           // cvtdq2ps      %xmm1,%xmm1
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,40,217                        // movaps        %xmm9,%xmm3
  .byte  102,15,114,211,16                   // psrld         $0x10,%xmm3
  .byte  102,15,219,218                      // pand          %xmm2,%xmm3
  .byte  15,91,211                           // cvtdq2ps      %xmm3,%xmm2
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  102,65,15,114,209,24                // psrld         $0x18,%xmm9
  .byte  65,15,91,217                        // cvtdq2ps      %xmm9,%xmm3
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_store_8888_sse2
_sk_store_8888_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  68,15,40,5,38,8,0,0                 // movaps        0x826(%rip),%xmm8        # 1050 <_sk_linear_gradient_2stops_sse2+0x318>
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  102,69,15,91,201                    // cvtps2dq      %xmm9,%xmm9
  .byte  68,15,40,209                        // movaps        %xmm1,%xmm10
  .byte  69,15,89,208                        // mulps         %xmm8,%xmm10
  .byte  102,69,15,91,210                    // cvtps2dq      %xmm10,%xmm10
  .byte  102,65,15,114,242,8                 // pslld         $0x8,%xmm10
  .byte  102,69,15,235,209                   // por           %xmm9,%xmm10
  .byte  68,15,40,202                        // movaps        %xmm2,%xmm9
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
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

.globl _sk_load_f16_sse2
_sk_load_f16_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  243,15,111,4,248                    // movdqu        (%rax,%rdi,8),%xmm0
  .byte  243,15,111,76,248,16                // movdqu        0x10(%rax,%rdi,8),%xmm1
  .byte  102,15,111,208                      // movdqa        %xmm0,%xmm2
  .byte  102,15,97,209                       // punpcklwd     %xmm1,%xmm2
  .byte  102,15,105,193                      // punpckhwd     %xmm1,%xmm0
  .byte  102,68,15,111,194                   // movdqa        %xmm2,%xmm8
  .byte  102,68,15,97,192                    // punpcklwd     %xmm0,%xmm8
  .byte  102,15,105,208                      // punpckhwd     %xmm0,%xmm2
  .byte  102,15,111,29,169,7,0,0             // movdqa        0x7a9(%rip),%xmm3        # 1060 <_sk_linear_gradient_2stops_sse2+0x328>
  .byte  102,15,111,203                      // movdqa        %xmm3,%xmm1
  .byte  102,65,15,101,200                   // pcmpgtw       %xmm8,%xmm1
  .byte  102,65,15,223,200                   // pandn         %xmm8,%xmm1
  .byte  102,15,101,218                      // pcmpgtw       %xmm2,%xmm3
  .byte  102,15,223,218                      // pandn         %xmm2,%xmm3
  .byte  102,69,15,239,192                   // pxor          %xmm8,%xmm8
  .byte  102,15,111,193                      // movdqa        %xmm1,%xmm0
  .byte  102,65,15,97,192                    // punpcklwd     %xmm8,%xmm0
  .byte  102,15,114,240,13                   // pslld         $0xd,%xmm0
  .byte  68,15,40,13,136,7,0,0               // movaps        0x788(%rip),%xmm9        # 1070 <_sk_linear_gradient_2stops_sse2+0x338>
  .byte  65,15,89,193                        // mulps         %xmm9,%xmm0
  .byte  102,65,15,105,200                   // punpckhwd     %xmm8,%xmm1
  .byte  102,15,114,241,13                   // pslld         $0xd,%xmm1
  .byte  65,15,89,201                        // mulps         %xmm9,%xmm1
  .byte  102,15,111,211                      // movdqa        %xmm3,%xmm2
  .byte  102,65,15,97,208                    // punpcklwd     %xmm8,%xmm2
  .byte  102,15,114,242,13                   // pslld         $0xd,%xmm2
  .byte  65,15,89,209                        // mulps         %xmm9,%xmm2
  .byte  102,65,15,105,216                   // punpckhwd     %xmm8,%xmm3
  .byte  102,15,114,243,13                   // pslld         $0xd,%xmm3
  .byte  65,15,89,217                        // mulps         %xmm9,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_store_f16_sse2
_sk_store_f16_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  68,15,40,5,85,7,0,0                 // movaps        0x755(%rip),%xmm8        # 1080 <_sk_linear_gradient_2stops_sse2+0x348>
  .byte  68,15,40,200                        // movaps        %xmm0,%xmm9
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  102,65,15,114,209,13                // psrld         $0xd,%xmm9
  .byte  68,15,40,209                        // movaps        %xmm1,%xmm10
  .byte  69,15,89,208                        // mulps         %xmm8,%xmm10
  .byte  102,65,15,114,210,13                // psrld         $0xd,%xmm10
  .byte  68,15,40,218                        // movaps        %xmm2,%xmm11
  .byte  69,15,89,216                        // mulps         %xmm8,%xmm11
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
  .byte  68,15,84,13,127,6,0,0               // andps         0x67f(%rip),%xmm9        # 1090 <_sk_linear_gradient_2stops_sse2+0x358>
  .byte  69,15,92,209                        // subps         %xmm9,%xmm10
  .byte  69,15,89,208                        // mulps         %xmm8,%xmm10
  .byte  65,15,92,194                        // subps         %xmm10,%xmm0
  .byte  102,69,15,118,201                   // pcmpeqd       %xmm9,%xmm9
  .byte  102,69,15,254,200                   // paddd         %xmm8,%xmm9
  .byte  65,15,93,193                        // minps         %xmm9,%xmm0
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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
  .byte  68,15,84,13,71,6,0,0                // andps         0x647(%rip),%xmm9        # 10a0 <_sk_linear_gradient_2stops_sse2+0x368>
  .byte  69,15,92,209                        // subps         %xmm9,%xmm10
  .byte  69,15,89,208                        // mulps         %xmm8,%xmm10
  .byte  65,15,92,202                        // subps         %xmm10,%xmm1
  .byte  102,69,15,118,201                   // pcmpeqd       %xmm9,%xmm9
  .byte  102,69,15,254,200                   // paddd         %xmm8,%xmm9
  .byte  65,15,93,201                        // minps         %xmm9,%xmm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_mirror_x_sse2
_sk_mirror_x_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,0                      // movss         (%rax),%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  65,15,92,193                        // subps         %xmm9,%xmm0
  .byte  243,69,15,88,192                    // addss         %xmm8,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,40,208                        // movaps        %xmm0,%xmm10
  .byte  69,15,94,208                        // divps         %xmm8,%xmm10
  .byte  243,69,15,91,218                    // cvttps2dq     %xmm10,%xmm11
  .byte  69,15,91,219                        // cvtdq2ps      %xmm11,%xmm11
  .byte  69,15,194,211,1                     // cmpltps       %xmm11,%xmm10
  .byte  68,15,84,21,253,5,0,0               // andps         0x5fd(%rip),%xmm10        # 10b0 <_sk_linear_gradient_2stops_sse2+0x378>
  .byte  69,15,87,228                        // xorps         %xmm12,%xmm12
  .byte  69,15,92,218                        // subps         %xmm10,%xmm11
  .byte  69,15,89,216                        // mulps         %xmm8,%xmm11
  .byte  65,15,92,195                        // subps         %xmm11,%xmm0
  .byte  65,15,92,193                        // subps         %xmm9,%xmm0
  .byte  68,15,92,224                        // subps         %xmm0,%xmm12
  .byte  65,15,84,196                        // andps         %xmm12,%xmm0
  .byte  102,69,15,118,192                   // pcmpeqd       %xmm8,%xmm8
  .byte  102,69,15,254,193                   // paddd         %xmm9,%xmm8
  .byte  65,15,93,192                        // minps         %xmm8,%xmm0
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_mirror_y_sse2
_sk_mirror_y_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,0                      // movss         (%rax),%xmm8
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  65,15,92,201                        // subps         %xmm9,%xmm1
  .byte  243,69,15,88,192                    // addss         %xmm8,%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,40,209                        // movaps        %xmm1,%xmm10
  .byte  69,15,94,208                        // divps         %xmm8,%xmm10
  .byte  243,69,15,91,218                    // cvttps2dq     %xmm10,%xmm11
  .byte  69,15,91,219                        // cvtdq2ps      %xmm11,%xmm11
  .byte  69,15,194,211,1                     // cmpltps       %xmm11,%xmm10
  .byte  68,15,84,21,163,5,0,0               // andps         0x5a3(%rip),%xmm10        # 10c0 <_sk_linear_gradient_2stops_sse2+0x388>
  .byte  69,15,87,228                        // xorps         %xmm12,%xmm12
  .byte  69,15,92,218                        // subps         %xmm10,%xmm11
  .byte  69,15,89,216                        // mulps         %xmm8,%xmm11
  .byte  65,15,92,203                        // subps         %xmm11,%xmm1
  .byte  65,15,92,201                        // subps         %xmm9,%xmm1
  .byte  68,15,92,225                        // subps         %xmm1,%xmm12
  .byte  65,15,84,204                        // andps         %xmm12,%xmm1
  .byte  102,69,15,118,192                   // pcmpeqd       %xmm8,%xmm8
  .byte  102,69,15,254,193                   // paddd         %xmm9,%xmm8
  .byte  65,15,93,200                        // minps         %xmm8,%xmm1
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

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

.balign 16
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,63                                // add           %bh,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,63                                // add           %bh,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,63                                // add           %bh,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,63                                // add           %bh,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  64,0,0                              // add           %al,(%rax)
  .byte  64                                  // rex
  .byte  64,0,0                              // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  145                                 // xchg          %eax,%ecx
  .byte  131,158,61,145,131,158,61           // sbbl          $0x3d,-0x617c6ec3(%rsi)
  .byte  145                                 // xchg          %eax,%ecx
  .byte  131,158,61,145,131,158,61           // sbbl          $0x3d,-0x617c6ec3(%rsi)
  .byte  154                                 // (bad)
  .byte  153                                 // cltd
  .byte  153                                 // cltd
  .byte  62,154                              // ds            (bad)
  .byte  153                                 // cltd
  .byte  153                                 // cltd
  .byte  62,154                              // ds            (bad)
  .byte  153                                 // cltd
  .byte  153                                 // cltd
  .byte  62,154                              // ds            (bad)
  .byte  153                                 // cltd
  .byte  153                                 // cltd
  .byte  62,92                               // ds            pop %rsp
  .byte  143                                 // (bad)
  .byte  50,63                               // xor           (%rdi),%bh
  .byte  92                                  // pop           %rsp
  .byte  143                                 // (bad)
  .byte  50,63                               // xor           (%rdi),%bh
  .byte  92                                  // pop           %rsp
  .byte  143                                 // (bad)
  .byte  50,63                               // xor           (%rdi),%bh
  .byte  92                                  // pop           %rsp
  .byte  143                                 // (bad)
  .byte  50,63                               // xor           (%rdi),%bh
  .byte  10,215                              // or            %bh,%dl
  .byte  35,59                               // and           (%rbx),%edi
  .byte  10,215                              // or            %bh,%dl
  .byte  35,59                               // and           (%rbx),%edi
  .byte  10,215                              // or            %bh,%dl
  .byte  35,59                               // and           (%rbx),%edi
  .byte  10,215                              // or            %bh,%dl
  .byte  35,59                               // and           (%rbx),%edi
  .byte  174                                 // scas          %es:(%rdi),%al
  .byte  71,97                               // rex.RXB       (bad)
  .byte  61,174,71,97,61                     // cmp           $0x3d6147ae,%eax
  .byte  174                                 // scas          %es:(%rdi),%al
  .byte  71,97                               // rex.RXB       (bad)
  .byte  61,174,71,97,61                     // cmp           $0x3d6147ae,%eax
  .byte  41,92,71,65                         // sub           %ebx,0x41(%rdi,%rax,2)
  .byte  41,92,71,65                         // sub           %ebx,0x41(%rdi,%rax,2)
  .byte  41,92,71,65                         // sub           %ebx,0x41(%rdi,%rax,2)
  .byte  41,92,71,65                         // sub           %ebx,0x41(%rdi,%rax,2)
  .byte  206                                 // (bad)
  .byte  111                                 // outsl         %ds:(%rsi),(%dx)
  .byte  48,63                               // xor           %bh,(%rdi)
  .byte  206                                 // (bad)
  .byte  111                                 // outsl         %ds:(%rsi),(%dx)
  .byte  48,63                               // xor           %bh,(%rdi)
  .byte  206                                 // (bad)
  .byte  111                                 // outsl         %ds:(%rsi),(%dx)
  .byte  48,63                               // xor           %bh,(%rdi)
  .byte  206                                 // (bad)
  .byte  111                                 // outsl         %ds:(%rsi),(%dx)
  .byte  48,63                               // xor           %bh,(%rdi)
  .byte  168,87                              // test          $0x57,%al
  .byte  202,189,168                         // lret          $0xa8bd
  .byte  87                                  // push          %rdi
  .byte  202,189,168                         // lret          $0xa8bd
  .byte  87                                  // push          %rdi
  .byte  202,189,168                         // lret          $0xa8bd
  .byte  87                                  // push          %rdi
  .byte  202,189,194                         // lret          $0xc2bd
  .byte  135,210                             // xchg          %edx,%edx
  .byte  62,194,135,210                      // ds            retq $0xd287
  .byte  62,194,135,210                      // ds            retq $0xd287
  .byte  62,194,135,210                      // ds            retq $0xd287
  .byte  62,0,0                              // add           %al,%ds:(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,4                            // cmpb          $0x4,(%rdi)
  .byte  231,140                             // out           %eax,$0x8c
  .byte  59,4,231                            // cmp           (%rdi,%riz,8),%eax
  .byte  140,59                              // mov           %?,(%rbx)
  .byte  4,231                               // add           $0xe7,%al
  .byte  140,59                              // mov           %?,(%rbx)
  .byte  4,231                               // add           $0xe7,%al
  .byte  140,59                              // mov           %?,(%rbx)
  .byte  129,128,128,59,129,128,128,59,129,128// addl          $0x80813b80,-0x7f7ec480(%rax)
  .byte  128,59,129                          // cmpb          $0x81,(%rbx)
  .byte  128,128,59,129,128,128,59           // addb          $0x3b,-0x7f7f7ec5(%rax)
  .byte  129,128,128,59,129,128,128,59,129,128// addl          $0x80813b80,-0x7f7ec480(%rax)
  .byte  128,59,0                            // cmpb          $0x0,(%rbx)
  .byte  248                                 // clc
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,248                               // add           %bh,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,248                               // add           %bh,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,248                               // add           %bh,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  224,7                               // loopne        f19 <.literal16+0x179>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  224,7                               // loopne        f1d <.literal16+0x17d>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  224,7                               // loopne        f21 <.literal16+0x181>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  224,7                               // loopne        f25 <.literal16+0x185>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  31                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,31                                // add           %bl,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,31                                // add           %bl,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,31                                // add           %bl,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,8                                 // add           %cl,(%rax)
  .byte  33,4,61,8,33,4,61                   // and           %eax,0x3d042108(,%rdi,1)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  4,61                                // add           $0x3d,%al
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  4,61                                // add           $0x3d,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,255                          // cmpb          $0xff,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,255                               // add           %bh,%bh
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,255                               // add           %bh,%bh
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,255                               // add           %bh,%bh
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,129,128,128,59,129                // add           %al,-0x7ec47f80(%rcx)
  .byte  128,128,59,129,128,128,59           // addb          $0x3b,-0x7f7f7ec5(%rax)
  .byte  129,128,128,59,129,128,128,59,129,128// addl          $0x80813b80,-0x7f7ec480(%rax)
  .byte  128,59,129                          // cmpb          $0x81,(%rbx)
  .byte  128,128,59,129,128,128,59           // addb          $0x3b,-0x7f7f7ec5(%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  127,67                              // jg            fd7 <.literal16+0x237>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  127,67                              // jg            fdb <.literal16+0x23b>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  127,67                              // jg            fdf <.literal16+0x23f>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  127,67                              // jg            fe3 <.literal16+0x243>
  .byte  0,248                               // add           %bh,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,248                               // add           %bh,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,248                               // add           %bh,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,248                               // add           %bh,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  132,55                              // test          %dh,(%rdi)
  .byte  224,7                               // loopne        fc9 <.literal16+0x229>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  224,7                               // loopne        fcd <.literal16+0x22d>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  224,7                               // loopne        fd1 <.literal16+0x231>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  224,7                               // loopne        fd5 <.literal16+0x235>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  33,8                                // and           %ecx,(%rax)
  .byte  2,58                                // add           (%rdx),%bh
  .byte  31                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,31                                // add           %bl,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,31                                // add           %bl,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,31                                // add           %bl,(%rdi)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  0,8                                 // add           %cl,(%rax)
  .byte  33,4,61,8,33,4,61                   // and           %eax,0x3d042108(,%rdi,1)
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  4,61                                // add           $0x3d,%al
  .byte  8,33                                // or            %ah,(%rcx)
  .byte  4,61                                // add           $0x3d,%al
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,248                               // add           %bh,%al
  .byte  65,0,0                              // add           %al,(%r8)
  .byte  248                                 // clc
  .byte  65,0,0                              // add           %al,(%r8)
  .byte  248                                 // clc
  .byte  65,0,0                              // add           %al,(%r8)
  .byte  248                                 // clc
  .byte  65,0,0                              // add           %al,(%r8)
  .byte  124,66                              // jl            1066 <.literal16+0x2c6>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  124,66                              // jl            106a <.literal16+0x2ca>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  124,66                              // jl            106e <.literal16+0x2ce>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  124,66                              // jl            1072 <.literal16+0x2d2>
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  255,0                               // incl          (%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  129,128,128,59,129,128,128,59,129,128// addl          $0x80813b80,-0x7f7ec480(%rax)
  .byte  128,59,129                          // cmpb          $0x81,(%rbx)
  .byte  128,128,59,0,0,127,67               // addb          $0x43,0x7f00003b(%rax)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  127,67                              // jg            109b <.literal16+0x2fb>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  127,67                              // jg            109f <.literal16+0x2ff>
  .byte  0,0                                 // add           %al,(%rax)
  .byte  127,67                              // jg            10a3 <.literal16+0x303>
  .byte  0,4,0                               // add           %al,(%rax,%rax,1)
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  4,0                                 // add           $0x0,%al
  .byte  0,128,119,0,0,128                   // add           %al,-0x7fffff89(%rax)
  .byte  119,0                               // ja            1079 <.literal16+0x2d9>
  .byte  0,128,119,0,0,128                   // add           %al,-0x7fffff89(%rax)
  .byte  119,0                               // ja            1081 <.literal16+0x2e1>
  .byte  0,128,7,0,0,128                     // add           %al,-0x7ffffff9(%rax)
  .byte  7                                   // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,7,0                             // addb          $0x0,(%rdi)
  .byte  0,128,7,0,0,128                     // add           %al,-0x7ffffff9(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
  .byte  0,0                                 // add           %al,(%rax)
  .byte  128,63,0                            // cmpb          $0x0,(%rdi)
  .byte  0,128,63,0,0,128                    // add           %al,-0x7fffffc1(%rax)
  .byte  63                                  // (bad)
#endif
