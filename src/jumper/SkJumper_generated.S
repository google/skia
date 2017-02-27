# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is generated semi-automatically with this command:
#   $ src/jumper/build_stages.py

.text
#if defined(__aarch64__)
.balign 4

.globl _sk_start_pipeline_aarch64
_sk_start_pipeline_aarch64:
  .long  0xa9bd57f6                          // stp           x22, x21, [sp,#-48]!
  .long  0xa9014ff4                          // stp           x20, x19, [sp,#16]
  .long  0xa9027bfd                          // stp           x29, x30, [sp,#32]
  .long  0x910083fd                          // add           x29, sp, #0x20
  .long  0xd10043ff                          // sub           sp, sp, #0x10
  .long  0xaa0103f3                          // mov           x19, x1
  .long  0xf8408664                          // ldr           x4, [x19],#8
  .long  0xaa0003f6                          // mov           x22, x0
  .long  0xaa0303f5                          // mov           x21, x3
  .long  0xaa0203f4                          // mov           x20, x2
  .long  0x910012c8                          // add           x8, x22, #0x4
  .long  0xf90007e4                          // str           x4, [sp,#8]
  .long  0xeb15011f                          // cmp           x8, x21
  .long  0x54000069                          // b.ls          40 <sk_start_pipeline_aarch64+0x40>
  .long  0xaa1603e0                          // mov           x0, x22
  .long  0x14000015                          // b             90 <sk_start_pipeline_aarch64+0x90>
  .long  0xf94007e4                          // ldr           x4, [sp,#8]
  .long  0xaa1f03e3                          // mov           x3, xzr
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
  .long  0xaa1403e2                          // mov           x2, x20
  .long  0xd63f0080                          // blr           x4
  .long  0xf94007e4                          // ldr           x4, [sp,#8]
  .long  0x910022c8                          // add           x8, x22, #0x8
  .long  0x910012c0                          // add           x0, x22, #0x4
  .long  0xeb15011f                          // cmp           x8, x21
  .long  0xaa0003f6                          // mov           x22, x0
  .long  0x54fffda9                          // b.ls          40 <sk_start_pipeline_aarch64+0x40>
  .long  0xeb15001f                          // cmp           x0, x21
  .long  0x540000c1                          // b.ne          ac <sk_start_pipeline_aarch64+0xac>
  .long  0xd10083bf                          // sub           sp, x29, #0x20
  .long  0xa9427bfd                          // ldp           x29, x30, [sp,#32]
  .long  0xa9414ff4                          // ldp           x20, x19, [sp,#16]
  .long  0xa8c357f6                          // ldp           x22, x21, [sp],#48
  .long  0xd65f03c0                          // ret
  .long  0xcb0002a3                          // sub           x3, x21, x0
  .long  0x6f00e400                          // movi          v0.2d, #0x0
  .long  0x6f00e401                          // movi          v1.2d, #0x0
  .long  0x6f00e402                          // movi          v2.2d, #0x0
  .long  0x6f00e403                          // movi          v3.2d, #0x0
  .long  0x6f00e404                          // movi          v4.2d, #0x0
  .long  0x6f00e405                          // movi          v5.2d, #0x0
  .long  0x6f00e406                          // movi          v6.2d, #0x0
  .long  0x6f00e407                          // movi          v7.2d, #0x0
  .long  0xaa1303e1                          // mov           x1, x19
  .long  0xaa1403e2                          // mov           x2, x20
  .long  0xd10083bf                          // sub           sp, x29, #0x20
  .long  0xa9427bfd                          // ldp           x29, x30, [sp,#32]
  .long  0xa9414ff4                          // ldp           x20, x19, [sp,#16]
  .long  0xa8c357f6                          // ldp           x22, x21, [sp],#48
  .long  0xd61f0080                          // br            x4

.globl _sk_just_return_aarch64
_sk_just_return_aarch64:
  .long  0xd65f03c0                          // ret

.globl _sk_seed_shader_aarch64
_sk_seed_shader_aarch64:
  .long  0xaa0203e9                          // mov           x9, x2
  .long  0xa8c11028                          // ldp           x8, x4, [x1],#16
  .long  0x4ddfc922                          // ld1r          {v2.4s}, [x9], #4
  .long  0x3cc14041                          // ldur          q1, [x2,#20]
  .long  0x4e040c00                          // dup           v0.4s, w0
  .long  0x4d40c903                          // ld1r          {v3.4s}, [x8]
  .long  0x4d40c924                          // ld1r          {v4.4s}, [x9]
  .long  0x4e21d800                          // scvtf         v0.4s, v0.4s
  .long  0x6f00e405                          // movi          v5.2d, #0x0
  .long  0x4e21d863                          // scvtf         v3.4s, v3.4s
  .long  0x4e24d400                          // fadd          v0.4s, v0.4s, v4.4s
  .long  0x4e20d420                          // fadd          v0.4s, v1.4s, v0.4s
  .long  0x4e24d461                          // fadd          v1.4s, v3.4s, v4.4s
  .long  0x6f00e403                          // movi          v3.2d, #0x0
  .long  0x6f00e404                          // movi          v4.2d, #0x0
  .long  0x6f00e406                          // movi          v6.2d, #0x0
  .long  0x6f00e407                          // movi          v7.2d, #0x0
  .long  0xd61f0080                          // br            x4

.globl _sk_constant_color_aarch64
_sk_constant_color_aarch64:
  .long  0xa8c11028                          // ldp           x8, x4, [x1],#16
  .long  0x3dc00103                          // ldr           q3, [x8]
  .long  0x4e040460                          // dup           v0.4s, v3.s[0]
  .long  0x4e0c0461                          // dup           v1.4s, v3.s[1]
  .long  0x4e140462                          // dup           v2.4s, v3.s[2]
  .long  0x4e1c0463                          // dup           v3.4s, v3.s[3]
  .long  0xd61f0080                          // br            x4

.globl _sk_clear_aarch64
_sk_clear_aarch64:
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x91004028                          // add           x8, x1, #0x10
  .long  0x6f00e400                          // movi          v0.2d, #0x0
  .long  0x6f00e401                          // movi          v1.2d, #0x0
  .long  0x6f00e402                          // movi          v2.2d, #0x0
  .long  0x6f00e403                          // movi          v3.2d, #0x0
  .long  0xaa0803e1                          // mov           x1, x8
  .long  0xd61f0080                          // br            x4

.globl _sk_plus__aarch64
_sk_plus__aarch64:
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x4e24d400                          // fadd          v0.4s, v0.4s, v4.4s
  .long  0x4e25d421                          // fadd          v1.4s, v1.4s, v5.4s
  .long  0x4e26d442                          // fadd          v2.4s, v2.4s, v6.4s
  .long  0x4e27d463                          // fadd          v3.4s, v3.4s, v7.4s
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0080                          // br            x4

.globl _sk_srcover_aarch64
_sk_srcover_aarch64:
  .long  0x4d40c850                          // ld1r          {v16.4s}, [x2]
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x4ea3d610                          // fsub          v16.4s, v16.4s, v3.4s
  .long  0x4e24ce00                          // fmla          v0.4s, v16.4s, v4.4s
  .long  0x4e25ce01                          // fmla          v1.4s, v16.4s, v5.4s
  .long  0x4e26ce02                          // fmla          v2.4s, v16.4s, v6.4s
  .long  0x4e27ce03                          // fmla          v3.4s, v16.4s, v7.4s
  .long  0xd61f0080                          // br            x4

.globl _sk_dstover_aarch64
_sk_dstover_aarch64:
  .long  0x4d40c851                          // ld1r          {v17.4s}, [x2]
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x4ea41c90                          // mov           v16.16b, v4.16b
  .long  0x4ea61cd2                          // mov           v18.16b, v6.16b
  .long  0x4ea7d634                          // fsub          v20.4s, v17.4s, v7.4s
  .long  0x4ea51cb1                          // mov           v17.16b, v5.16b
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
  .long  0xd61f0080                          // br            x4

.globl _sk_clamp_0_aarch64
_sk_clamp_0_aarch64:
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x6f00e410                          // movi          v16.2d, #0x0
  .long  0x4e30f400                          // fmax          v0.4s, v0.4s, v16.4s
  .long  0x4e30f421                          // fmax          v1.4s, v1.4s, v16.4s
  .long  0x4e30f442                          // fmax          v2.4s, v2.4s, v16.4s
  .long  0x4e30f463                          // fmax          v3.4s, v3.4s, v16.4s
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0080                          // br            x4

.globl _sk_clamp_1_aarch64
_sk_clamp_1_aarch64:
  .long  0x4d40c850                          // ld1r          {v16.4s}, [x2]
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x4eb0f400                          // fmin          v0.4s, v0.4s, v16.4s
  .long  0x4eb0f421                          // fmin          v1.4s, v1.4s, v16.4s
  .long  0x4eb0f442                          // fmin          v2.4s, v2.4s, v16.4s
  .long  0x4eb0f463                          // fmin          v3.4s, v3.4s, v16.4s
  .long  0xd61f0080                          // br            x4

.globl _sk_clamp_a_aarch64
_sk_clamp_a_aarch64:
  .long  0x4d40c850                          // ld1r          {v16.4s}, [x2]
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x4eb0f463                          // fmin          v3.4s, v3.4s, v16.4s
  .long  0x4ea3f400                          // fmin          v0.4s, v0.4s, v3.4s
  .long  0x4ea3f421                          // fmin          v1.4s, v1.4s, v3.4s
  .long  0x4ea3f442                          // fmin          v2.4s, v2.4s, v3.4s
  .long  0xd61f0080                          // br            x4

.globl _sk_set_rgb_aarch64
_sk_set_rgb_aarch64:
  .long  0xa8c11028                          // ldp           x8, x4, [x1],#16
  .long  0x91002109                          // add           x9, x8, #0x8
  .long  0x4ddfc900                          // ld1r          {v0.4s}, [x8], #4
  .long  0x4d40c922                          // ld1r          {v2.4s}, [x9]
  .long  0x4d40c901                          // ld1r          {v1.4s}, [x8]
  .long  0xd61f0080                          // br            x4

.globl _sk_swap_rb_aarch64
_sk_swap_rb_aarch64:
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x4ea01c10                          // mov           v16.16b, v0.16b
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x4ea21c40                          // mov           v0.16b, v2.16b
  .long  0x4eb01e02                          // mov           v2.16b, v16.16b
  .long  0xd61f0080                          // br            x4

.globl _sk_swap_aarch64
_sk_swap_aarch64:
  .long  0xf9400424                          // ldr           x4, [x1,#8]
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
  .long  0xd61f0080                          // br            x4

.globl _sk_move_src_dst_aarch64
_sk_move_src_dst_aarch64:
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x4ea01c04                          // mov           v4.16b, v0.16b
  .long  0x4ea11c25                          // mov           v5.16b, v1.16b
  .long  0x4ea21c46                          // mov           v6.16b, v2.16b
  .long  0x4ea31c67                          // mov           v7.16b, v3.16b
  .long  0xd61f0080                          // br            x4

.globl _sk_move_dst_src_aarch64
_sk_move_dst_src_aarch64:
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x4ea41c80                          // mov           v0.16b, v4.16b
  .long  0x4ea51ca1                          // mov           v1.16b, v5.16b
  .long  0x4ea61cc2                          // mov           v2.16b, v6.16b
  .long  0x4ea71ce3                          // mov           v3.16b, v7.16b
  .long  0xd61f0080                          // br            x4

.globl _sk_premul_aarch64
_sk_premul_aarch64:
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x6e23dc00                          // fmul          v0.4s, v0.4s, v3.4s
  .long  0x6e23dc21                          // fmul          v1.4s, v1.4s, v3.4s
  .long  0x6e23dc42                          // fmul          v2.4s, v2.4s, v3.4s
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0080                          // br            x4

.globl _sk_unpremul_aarch64
_sk_unpremul_aarch64:
  .long  0x4d40c850                          // ld1r          {v16.4s}, [x2]
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x4ea0d871                          // fcmeq         v17.4s, v3.4s, #0.0
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x6e23fe10                          // fdiv          v16.4s, v16.4s, v3.4s
  .long  0x4e711e10                          // bic           v16.16b, v16.16b, v17.16b
  .long  0x6e20de00                          // fmul          v0.4s, v16.4s, v0.4s
  .long  0x6e21de01                          // fmul          v1.4s, v16.4s, v1.4s
  .long  0x6e22de02                          // fmul          v2.4s, v16.4s, v2.4s
  .long  0xd61f0080                          // br            x4

.globl _sk_from_srgb_aarch64
_sk_from_srgb_aarch64:
  .long  0x9100e048                          // add           x8, x2, #0x38
  .long  0x4d40c910                          // ld1r          {v16.4s}, [x8]
  .long  0x9100d048                          // add           x8, x2, #0x34
  .long  0x4d40c911                          // ld1r          {v17.4s}, [x8]
  .long  0x2d47cc52                          // ldp           s18, s19, [x2,#60]
  .long  0x6e22dc54                          // fmul          v20.4s, v2.4s, v2.4s
  .long  0x4eb01e15                          // mov           v21.16b, v16.16b
  .long  0x4eb01e17                          // mov           v23.16b, v16.16b
  .long  0x4f921050                          // fmla          v16.4s, v2.4s, v18.s[0]
  .long  0x4eb11e36                          // mov           v22.16b, v17.16b
  .long  0x4eb11e38                          // mov           v24.16b, v17.16b
  .long  0x4e34ce11                          // fmla          v17.4s, v16.4s, v20.4s
  .long  0x6e20dc10                          // fmul          v16.4s, v0.4s, v0.4s
  .long  0x91011048                          // add           x8, x2, #0x44
  .long  0x4f921015                          // fmla          v21.4s, v0.4s, v18.s[0]
  .long  0x4e30ceb6                          // fmla          v22.4s, v21.4s, v16.4s
  .long  0x4d40c910                          // ld1r          {v16.4s}, [x8]
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x6e21dc34                          // fmul          v20.4s, v1.4s, v1.4s
  .long  0x4f921037                          // fmla          v23.4s, v1.4s, v18.s[0]
  .long  0x4f939015                          // fmul          v21.4s, v0.4s, v19.s[0]
  .long  0x4f939032                          // fmul          v18.4s, v1.4s, v19.s[0]
  .long  0x4f939053                          // fmul          v19.4s, v2.4s, v19.s[0]
  .long  0x6ea0e600                          // fcmgt         v0.4s, v16.4s, v0.4s
  .long  0x6ea1e601                          // fcmgt         v1.4s, v16.4s, v1.4s
  .long  0x6ea2e602                          // fcmgt         v2.4s, v16.4s, v2.4s
  .long  0x4e34cef8                          // fmla          v24.4s, v23.4s, v20.4s
  .long  0x6e711e62                          // bsl           v2.16b, v19.16b, v17.16b
  .long  0x6e761ea0                          // bsl           v0.16b, v21.16b, v22.16b
  .long  0x6e781e41                          // bsl           v1.16b, v18.16b, v24.16b
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0080                          // br            x4

.globl _sk_to_srgb_aarch64
_sk_to_srgb_aarch64:
  .long  0x6ea1d811                          // frsqrte       v17.4s, v0.4s
  .long  0x6ea1d835                          // frsqrte       v21.4s, v1.4s
  .long  0x6e31de37                          // fmul          v23.4s, v17.4s, v17.4s
  .long  0x6ea1d856                          // frsqrte       v22.4s, v2.4s
  .long  0x6e35deb9                          // fmul          v25.4s, v21.4s, v21.4s
  .long  0x4eb7fc17                          // frsqrts       v23.4s, v0.4s, v23.4s
  .long  0x91015048                          // add           x8, x2, #0x54
  .long  0x6e36deda                          // fmul          v26.4s, v22.4s, v22.4s
  .long  0x4eb9fc39                          // frsqrts       v25.4s, v1.4s, v25.4s
  .long  0x6e37de31                          // fmul          v17.4s, v17.4s, v23.4s
  .long  0x2d494052                          // ldp           s18, s16, [x2,#72]
  .long  0x4d40c914                          // ld1r          {v20.4s}, [x8]
  .long  0x4ebafc5a                          // frsqrts       v26.4s, v2.4s, v26.4s
  .long  0x6e39deb5                          // fmul          v21.4s, v21.4s, v25.4s
  .long  0x4ea1da37                          // frecpe        v23.4s, v17.4s
  .long  0xbd405053                          // ldr           s19, [x2,#80]
  .long  0x91016048                          // add           x8, x2, #0x58
  .long  0x6e3aded6                          // fmul          v22.4s, v22.4s, v26.4s
  .long  0x4ea1dabb                          // frecpe        v27.4s, v21.4s
  .long  0x4e37fe3d                          // frecps        v29.4s, v17.4s, v23.4s
  .long  0x4d40c918                          // ld1r          {v24.4s}, [x8]
  .long  0x4ea1dadc                          // frecpe        v28.4s, v22.4s
  .long  0x6e3ddef7                          // fmul          v23.4s, v23.4s, v29.4s
  .long  0x4e3bfebd                          // frecps        v29.4s, v21.4s, v27.4s
  .long  0x6e3ddf7b                          // fmul          v27.4s, v27.4s, v29.4s
  .long  0x4e3cfedd                          // frecps        v29.4s, v22.4s, v28.4s
  .long  0x6e3ddf9c                          // fmul          v28.4s, v28.4s, v29.4s
  .long  0x4eb41e9d                          // mov           v29.16b, v20.16b
  .long  0x6ea1da39                          // frsqrte       v25.4s, v17.4s
  .long  0x4f9312fd                          // fmla          v29.4s, v23.4s, v19.s[0]
  .long  0x4eb41e97                          // mov           v23.16b, v20.16b
  .long  0x4f92901a                          // fmul          v26.4s, v0.4s, v18.s[0]
  .long  0x4f931377                          // fmla          v23.4s, v27.4s, v19.s[0]
  .long  0x4f931394                          // fmla          v20.4s, v28.4s, v19.s[0]
  .long  0x4f929033                          // fmul          v19.4s, v1.4s, v18.s[0]
  .long  0x4f929052                          // fmul          v18.4s, v2.4s, v18.s[0]
  .long  0x6ea0e700                          // fcmgt         v0.4s, v24.4s, v0.4s
  .long  0x6ea1e701                          // fcmgt         v1.4s, v24.4s, v1.4s
  .long  0x6ea2e702                          // fcmgt         v2.4s, v24.4s, v2.4s
  .long  0x6e39df38                          // fmul          v24.4s, v25.4s, v25.4s
  .long  0x6ea1dabb                          // frsqrte       v27.4s, v21.4s
  .long  0x4eb8fe31                          // frsqrts       v17.4s, v17.4s, v24.4s
  .long  0x6ea1dadc                          // frsqrte       v28.4s, v22.4s
  .long  0x6e3bdf78                          // fmul          v24.4s, v27.4s, v27.4s
  .long  0x6e31df31                          // fmul          v17.4s, v25.4s, v17.4s
  .long  0x4eb8feb5                          // frsqrts       v21.4s, v21.4s, v24.4s
  .long  0x6e3cdf98                          // fmul          v24.4s, v28.4s, v28.4s
  .long  0x4f90123d                          // fmla          v29.4s, v17.4s, v16.s[0]
  .long  0x4d40c851                          // ld1r          {v17.4s}, [x2]
  .long  0x4eb8fed6                          // frsqrts       v22.4s, v22.4s, v24.4s
  .long  0x6e35df75                          // fmul          v21.4s, v27.4s, v21.4s
  .long  0x6e36df96                          // fmul          v22.4s, v28.4s, v22.4s
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x4f9012b7                          // fmla          v23.4s, v21.4s, v16.s[0]
  .long  0x4f9012d4                          // fmla          v20.4s, v22.4s, v16.s[0]
  .long  0x4ebdf630                          // fmin          v16.4s, v17.4s, v29.4s
  .long  0x4eb7f635                          // fmin          v21.4s, v17.4s, v23.4s
  .long  0x4eb4f631                          // fmin          v17.4s, v17.4s, v20.4s
  .long  0x6e701f40                          // bsl           v0.16b, v26.16b, v16.16b
  .long  0x6e751e61                          // bsl           v1.16b, v19.16b, v21.16b
  .long  0x6e711e42                          // bsl           v2.16b, v18.16b, v17.16b
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0080                          // br            x4

.globl _sk_scale_1_float_aarch64
_sk_scale_1_float_aarch64:
  .long  0xa8c11028                          // ldp           x8, x4, [x1],#16
  .long  0xbd400110                          // ldr           s16, [x8]
  .long  0x4f909000                          // fmul          v0.4s, v0.4s, v16.s[0]
  .long  0x4f909021                          // fmul          v1.4s, v1.4s, v16.s[0]
  .long  0x4f909042                          // fmul          v2.4s, v2.4s, v16.s[0]
  .long  0x4f909063                          // fmul          v3.4s, v3.4s, v16.s[0]
  .long  0xd61f0080                          // br            x4

.globl _sk_scale_u8_aarch64
_sk_scale_u8_aarch64:
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0xf9400109                          // ldr           x9, [x8]
  .long  0xb50002c3                          // cbnz          x3, 534 <sk_scale_u8_aarch64+0x60>
  .long  0x8b000128                          // add           x8, x9, x0
  .long  0x39400109                          // ldrb          w9, [x8]
  .long  0x3940050a                          // ldrb          w10, [x8,#1]
  .long  0x3940090b                          // ldrb          w11, [x8,#2]
  .long  0x39400d08                          // ldrb          w8, [x8,#3]
  .long  0x4e021d30                          // mov           v16.h[0], w9
  .long  0x4e061d50                          // mov           v16.h[1], w10
  .long  0x4e0a1d70                          // mov           v16.h[2], w11
  .long  0x4e0e1d10                          // mov           v16.h[3], w8
  .long  0xbd400c51                          // ldr           s17, [x2,#12]
  .long  0x2f07b7f0                          // bic           v16.4h, #0xff, lsl #8
  .long  0x2f10a610                          // uxtl          v16.4s, v16.4h
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x6e21da10                          // ucvtf         v16.4s, v16.4s
  .long  0x4f919210                          // fmul          v16.4s, v16.4s, v17.s[0]
  .long  0x6e20de00                          // fmul          v0.4s, v16.4s, v0.4s
  .long  0x6e21de01                          // fmul          v1.4s, v16.4s, v1.4s
  .long  0x6e22de02                          // fmul          v2.4s, v16.4s, v2.4s
  .long  0x6e23de03                          // fmul          v3.4s, v16.4s, v3.4s
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0080                          // br            x4
  .long  0xd10043ff                          // sub           sp, sp, #0x10
  .long  0xaa1f03e8                          // mov           x8, xzr
  .long  0x8b000129                          // add           x9, x9, x0
  .long  0x910033ea                          // add           x10, sp, #0xc
  .long  0x39003fff                          // strb          wzr, [sp,#15]
  .long  0x39003bff                          // strb          wzr, [sp,#14]
  .long  0x390037ff                          // strb          wzr, [sp,#13]
  .long  0x390033ff                          // strb          wzr, [sp,#12]
  .long  0x3868692b                          // ldrb          w11, [x9,x8]
  .long  0x3828694b                          // strb          w11, [x10,x8]
  .long  0x91000508                          // add           x8, x8, #0x1
  .long  0xeb08007f                          // cmp           x3, x8
  .long  0x54ffff81                          // b.ne          554 <sk_scale_u8_aarch64+0x80>
  .long  0x394033e8                          // ldrb          w8, [sp,#12]
  .long  0x394037e9                          // ldrb          w9, [sp,#13]
  .long  0x39403bea                          // ldrb          w10, [sp,#14]
  .long  0x39403feb                          // ldrb          w11, [sp,#15]
  .long  0x4e021d10                          // mov           v16.h[0], w8
  .long  0x4e061d30                          // mov           v16.h[1], w9
  .long  0x4e0a1d50                          // mov           v16.h[2], w10
  .long  0x4e0e1d70                          // mov           v16.h[3], w11
  .long  0x910043ff                          // add           sp, sp, #0x10
  .long  0x17ffffde                          // b             504 <sk_scale_u8_aarch64+0x30>

.globl _sk_lerp_1_float_aarch64
_sk_lerp_1_float_aarch64:
  .long  0xa8c11028                          // ldp           x8, x4, [x1],#16
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
  .long  0xd61f0080                          // br            x4

.globl _sk_lerp_u8_aarch64
_sk_lerp_u8_aarch64:
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0xf9400109                          // ldr           x9, [x8]
  .long  0xb50003c3                          // cbnz          x3, 64c <sk_lerp_u8_aarch64+0x80>
  .long  0x8b000128                          // add           x8, x9, x0
  .long  0x39400109                          // ldrb          w9, [x8]
  .long  0x3940050a                          // ldrb          w10, [x8,#1]
  .long  0x3940090b                          // ldrb          w11, [x8,#2]
  .long  0x39400d08                          // ldrb          w8, [x8,#3]
  .long  0x4e021d30                          // mov           v16.h[0], w9
  .long  0x4e061d50                          // mov           v16.h[1], w10
  .long  0x4e0a1d70                          // mov           v16.h[2], w11
  .long  0x4e0e1d10                          // mov           v16.h[3], w8
  .long  0xbd400c51                          // ldr           s17, [x2,#12]
  .long  0x2f07b7f0                          // bic           v16.4h, #0xff, lsl #8
  .long  0x2f10a610                          // uxtl          v16.4s, v16.4h
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x6e21da10                          // ucvtf         v16.4s, v16.4s
  .long  0x4f919210                          // fmul          v16.4s, v16.4s, v17.s[0]
  .long  0x4ea4d411                          // fsub          v17.4s, v0.4s, v4.4s
  .long  0x4ea41c80                          // mov           v0.16b, v4.16b
  .long  0x4ea5d432                          // fsub          v18.4s, v1.4s, v5.4s
  .long  0x4ea6d453                          // fsub          v19.4s, v2.4s, v6.4s
  .long  0x4ea7d474                          // fsub          v20.4s, v3.4s, v7.4s
  .long  0x4ea51ca1                          // mov           v1.16b, v5.16b
  .long  0x4ea61cc2                          // mov           v2.16b, v6.16b
  .long  0x4ea71ce3                          // mov           v3.16b, v7.16b
  .long  0x4e31ce00                          // fmla          v0.4s, v16.4s, v17.4s
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x4e32ce01                          // fmla          v1.4s, v16.4s, v18.4s
  .long  0x4e33ce02                          // fmla          v2.4s, v16.4s, v19.4s
  .long  0x4e34ce03                          // fmla          v3.4s, v16.4s, v20.4s
  .long  0xd61f0080                          // br            x4
  .long  0xd10043ff                          // sub           sp, sp, #0x10
  .long  0xaa1f03e8                          // mov           x8, xzr
  .long  0x8b000129                          // add           x9, x9, x0
  .long  0x910033ea                          // add           x10, sp, #0xc
  .long  0x39003fff                          // strb          wzr, [sp,#15]
  .long  0x39003bff                          // strb          wzr, [sp,#14]
  .long  0x390037ff                          // strb          wzr, [sp,#13]
  .long  0x390033ff                          // strb          wzr, [sp,#12]
  .long  0x3868692b                          // ldrb          w11, [x9,x8]
  .long  0x3828694b                          // strb          w11, [x10,x8]
  .long  0x91000508                          // add           x8, x8, #0x1
  .long  0xeb08007f                          // cmp           x3, x8
  .long  0x54ffff81                          // b.ne          66c <sk_lerp_u8_aarch64+0xa0>
  .long  0x394033e8                          // ldrb          w8, [sp,#12]
  .long  0x394037e9                          // ldrb          w9, [sp,#13]
  .long  0x39403bea                          // ldrb          w10, [sp,#14]
  .long  0x39403feb                          // ldrb          w11, [sp,#15]
  .long  0x4e021d10                          // mov           v16.h[0], w8
  .long  0x4e061d30                          // mov           v16.h[1], w9
  .long  0x4e0a1d50                          // mov           v16.h[2], w10
  .long  0x4e0e1d70                          // mov           v16.h[3], w11
  .long  0x910043ff                          // add           sp, sp, #0x10
  .long  0x17ffffd6                          // b             5fc <sk_lerp_u8_aarch64+0x30>

.globl _sk_lerp_565_aarch64
_sk_lerp_565_aarch64:
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x8b000508                          // add           x8, x8, x0, lsl #1
  .long  0xb5000423                          // cbnz          x3, 738 <sk_lerp_565_aarch64+0x90>
  .long  0xfd400103                          // ldr           d3, [x8]
  .long  0x9101a048                          // add           x8, x2, #0x68
  .long  0x4d40c910                          // ld1r          {v16.4s}, [x8]
  .long  0x9101b048                          // add           x8, x2, #0x6c
  .long  0x4d40c911                          // ld1r          {v17.4s}, [x8]
  .long  0x9101c048                          // add           x8, x2, #0x70
  .long  0x4d40c912                          // ld1r          {v18.4s}, [x8]
  .long  0x2f10a463                          // uxtl          v3.4s, v3.4h
  .long  0x4e231e10                          // and           v16.16b, v16.16b, v3.16b
  .long  0x4e231e31                          // and           v17.16b, v17.16b, v3.16b
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x4e231e43                          // and           v3.16b, v18.16b, v3.16b
  .long  0x2d4ecc52                          // ldp           s18, s19, [x2,#116]
  .long  0x4e21da10                          // scvtf         v16.4s, v16.4s
  .long  0x4e21d874                          // scvtf         v20.4s, v3.4s
  .long  0x4d40c843                          // ld1r          {v3.4s}, [x2]
  .long  0x4f929210                          // fmul          v16.4s, v16.4s, v18.s[0]
  .long  0xbd407c52                          // ldr           s18, [x2,#124]
  .long  0x4e21da31                          // scvtf         v17.4s, v17.4s
  .long  0x4f939231                          // fmul          v17.4s, v17.4s, v19.s[0]
  .long  0x4ea4d413                          // fsub          v19.4s, v0.4s, v4.4s
  .long  0x4f929292                          // fmul          v18.4s, v20.4s, v18.s[0]
  .long  0x4ea41c80                          // mov           v0.16b, v4.16b
  .long  0x4ea5d434                          // fsub          v20.4s, v1.4s, v5.4s
  .long  0x4ea6d455                          // fsub          v21.4s, v2.4s, v6.4s
  .long  0x4ea51ca1                          // mov           v1.16b, v5.16b
  .long  0x4ea61cc2                          // mov           v2.16b, v6.16b
  .long  0x4e33ce00                          // fmla          v0.4s, v16.4s, v19.4s
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x4e34ce21                          // fmla          v1.4s, v17.4s, v20.4s
  .long  0x4e35ce42                          // fmla          v2.4s, v18.4s, v21.4s
  .long  0xd61f0080                          // br            x4
  .long  0xd10043ff                          // sub           sp, sp, #0x10
  .long  0xaa1f03e9                          // mov           x9, xzr
  .long  0x2f00e403                          // movi          d3, #0x0
  .long  0x910023ea                          // add           x10, sp, #0x8
  .long  0xfd0007e3                          // str           d3, [sp,#8]
  .long  0x7869790b                          // ldrh          w11, [x8,x9,lsl #1]
  .long  0x7829794b                          // strh          w11, [x10,x9,lsl #1]
  .long  0x91000529                          // add           x9, x9, #0x1
  .long  0xeb09007f                          // cmp           x3, x9
  .long  0x54ffff81                          // b.ne          74c <sk_lerp_565_aarch64+0xa4>
  .long  0xfd4007e3                          // ldr           d3, [sp,#8]
  .long  0x910043ff                          // add           sp, sp, #0x10
  .long  0x17ffffd5                          // b             6bc <sk_lerp_565_aarch64+0x14>

.globl _sk_load_tables_aarch64
_sk_load_tables_aarch64:
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0xf9400109                          // ldr           x9, [x8]
  .long  0x8b000929                          // add           x9, x9, x0, lsl #2
  .long  0xb50006c3                          // cbnz          x3, 850 <sk_load_tables_aarch64+0xe4>
  .long  0x3dc00121                          // ldr           q1, [x9]
  .long  0x9100404b                          // add           x11, x2, #0x10
  .long  0xa940a909                          // ldp           x9, x10, [x8,#8]
  .long  0x4d40c960                          // ld1r          {v0.4s}, [x11]
  .long  0x6f380422                          // ushr          v2.4s, v1.4s, #8
  .long  0x6f300423                          // ushr          v3.4s, v1.4s, #16
  .long  0xf9400d08                          // ldr           x8, [x8,#24]
  .long  0x4e211c10                          // and           v16.16b, v0.16b, v1.16b
  .long  0x0e143e0c                          // mov           w12, v16.s[2]
  .long  0xbc6c5931                          // ldr           s17, [x9,w12,uxtw #2]
  .long  0x0e1c3e0c                          // mov           w12, v16.s[3]
  .long  0x0e0c3e0b                          // mov           w11, v16.s[1]
  .long  0xbc6c5932                          // ldr           s18, [x9,w12,uxtw #2]
  .long  0x1e26020c                          // fmov          w12, s16
  .long  0x8b2b492b                          // add           x11, x9, w11, uxtw #2
  .long  0x8b2c4929                          // add           x9, x9, w12, uxtw #2
  .long  0x4e221c02                          // and           v2.16b, v0.16b, v2.16b
  .long  0x4e231c03                          // and           v3.16b, v0.16b, v3.16b
  .long  0x0d408120                          // ld1           {v0.s}[0], [x9]
  .long  0x1e26004d                          // fmov          w13, s2
  .long  0x6f280421                          // ushr          v1.4s, v1.4s, #24
  .long  0x8b2d494d                          // add           x13, x10, w13, uxtw #2
  .long  0x0d409160                          // ld1           {v0.s}[1], [x11]
  .long  0x0e143c49                          // mov           w9, v2.s[2]
  .long  0x4e21d830                          // scvtf         v16.4s, v1.4s
  .long  0x0d4081a1                          // ld1           {v1.s}[0], [x13]
  .long  0x6e140620                          // mov           v0.s[2], v17.s[0]
  .long  0xbc695951                          // ldr           s17, [x10,w9,uxtw #2]
  .long  0x0e143c69                          // mov           w9, v3.s[2]
  .long  0x0e0c3c4c                          // mov           w12, v2.s[1]
  .long  0x6e1c0640                          // mov           v0.s[3], v18.s[0]
  .long  0xbc695912                          // ldr           s18, [x8,w9,uxtw #2]
  .long  0x1e260069                          // fmov          w9, s3
  .long  0x8b2c494c                          // add           x12, x10, w12, uxtw #2
  .long  0x8b294909                          // add           x9, x8, w9, uxtw #2
  .long  0x0e1c3c4b                          // mov           w11, v2.s[3]
  .long  0x0d409181                          // ld1           {v1.s}[1], [x12]
  .long  0x0d408122                          // ld1           {v2.s}[0], [x9]
  .long  0x0e0c3c6d                          // mov           w13, v3.s[1]
  .long  0x0e1c3c6c                          // mov           w12, v3.s[3]
  .long  0xbc6b5943                          // ldr           s3, [x10,w11,uxtw #2]
  .long  0x8b2d490d                          // add           x13, x8, w13, uxtw #2
  .long  0x6e140621                          // mov           v1.s[2], v17.s[0]
  .long  0x0d4091a2                          // ld1           {v2.s}[1], [x13]
  .long  0xbc6c5911                          // ldr           s17, [x8,w12,uxtw #2]
  .long  0x6e1c0461                          // mov           v1.s[3], v3.s[0]
  .long  0xbd400c43                          // ldr           s3, [x2,#12]
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x6e140642                          // mov           v2.s[2], v18.s[0]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x6e1c0622                          // mov           v2.s[3], v17.s[0]
  .long  0x4f839203                          // fmul          v3.4s, v16.4s, v3.s[0]
  .long  0xd61f0080                          // br            x4
  .long  0xd10043ff                          // sub           sp, sp, #0x10
  .long  0xaa1f03ea                          // mov           x10, xzr
  .long  0x6f00e400                          // movi          v0.2d, #0x0
  .long  0x910003eb                          // mov           x11, sp
  .long  0x3d8003e0                          // str           q0, [sp]
  .long  0xb86a792c                          // ldr           w12, [x9,x10,lsl #2]
  .long  0xb82a796c                          // str           w12, [x11,x10,lsl #2]
  .long  0x9100054a                          // add           x10, x10, #0x1
  .long  0xeb0a007f                          // cmp           x3, x10
  .long  0x54ffff81                          // b.ne          864 <sk_load_tables_aarch64+0xf8>
  .long  0x3cc107e1                          // ldr           q1, [sp],#16
  .long  0x17ffffc1                          // b             780 <sk_load_tables_aarch64+0x14>

.globl _sk_load_a8_aarch64
_sk_load_a8_aarch64:
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0xf9400109                          // ldr           x9, [x8]
  .long  0xb50002a3                          // cbnz          x3, 8dc <sk_load_a8_aarch64+0x5c>
  .long  0x8b000128                          // add           x8, x9, x0
  .long  0x39400109                          // ldrb          w9, [x8]
  .long  0x3940050a                          // ldrb          w10, [x8,#1]
  .long  0x3940090b                          // ldrb          w11, [x8,#2]
  .long  0x39400d08                          // ldrb          w8, [x8,#3]
  .long  0x4e021d20                          // mov           v0.h[0], w9
  .long  0x4e061d40                          // mov           v0.h[1], w10
  .long  0x4e0a1d60                          // mov           v0.h[2], w11
  .long  0x4e0e1d00                          // mov           v0.h[3], w8
  .long  0xbd400c41                          // ldr           s1, [x2,#12]
  .long  0x2f07b7e0                          // bic           v0.4h, #0xff, lsl #8
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x2f10a400                          // uxtl          v0.4s, v0.4h
  .long  0x6e21d800                          // ucvtf         v0.4s, v0.4s
  .long  0x4f819003                          // fmul          v3.4s, v0.4s, v1.s[0]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x6f00e400                          // movi          v0.2d, #0x0
  .long  0x6f00e401                          // movi          v1.2d, #0x0
  .long  0x6f00e402                          // movi          v2.2d, #0x0
  .long  0xd61f0080                          // br            x4
  .long  0xd10043ff                          // sub           sp, sp, #0x10
  .long  0xaa1f03e8                          // mov           x8, xzr
  .long  0x8b000129                          // add           x9, x9, x0
  .long  0x910033ea                          // add           x10, sp, #0xc
  .long  0x39003fff                          // strb          wzr, [sp,#15]
  .long  0x39003bff                          // strb          wzr, [sp,#14]
  .long  0x390037ff                          // strb          wzr, [sp,#13]
  .long  0x390033ff                          // strb          wzr, [sp,#12]
  .long  0x3868692b                          // ldrb          w11, [x9,x8]
  .long  0x3828694b                          // strb          w11, [x10,x8]
  .long  0x91000508                          // add           x8, x8, #0x1
  .long  0xeb08007f                          // cmp           x3, x8
  .long  0x54ffff81                          // b.ne          8fc <sk_load_a8_aarch64+0x7c>
  .long  0x394033e8                          // ldrb          w8, [sp,#12]
  .long  0x394037e9                          // ldrb          w9, [sp,#13]
  .long  0x39403bea                          // ldrb          w10, [sp,#14]
  .long  0x39403feb                          // ldrb          w11, [sp,#15]
  .long  0x4e021d00                          // mov           v0.h[0], w8
  .long  0x4e061d20                          // mov           v0.h[1], w9
  .long  0x4e0a1d40                          // mov           v0.h[2], w10
  .long  0x4e0e1d60                          // mov           v0.h[3], w11
  .long  0x910043ff                          // add           sp, sp, #0x10
  .long  0x17ffffdf                          // b             8b0 <sk_load_a8_aarch64+0x30>

.globl _sk_store_a8_aarch64
_sk_store_a8_aarch64:
  .long  0xd10043ff                          // sub           sp, sp, #0x10
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0xbd400850                          // ldr           s16, [x2,#8]
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x4f909070                          // fmul          v16.4s, v3.4s, v16.s[0]
  .long  0x6e21aa10                          // fcvtnu        v16.4s, v16.4s
  .long  0x0e612a10                          // xtn           v16.4h, v16.4s
  .long  0x0e0e3e09                          // umov          w9, v16.h[3]
  .long  0x0e0a3e0a                          // umov          w10, v16.h[2]
  .long  0x0e063e0b                          // umov          w11, v16.h[1]
  .long  0x0e023e0c                          // umov          w12, v16.h[0]
  .long  0x39003fe9                          // strb          w9, [sp,#15]
  .long  0x8b000109                          // add           x9, x8, x0
  .long  0x39003bea                          // strb          w10, [sp,#14]
  .long  0x390037eb                          // strb          w11, [sp,#13]
  .long  0x390033ec                          // strb          w12, [sp,#12]
  .long  0xb50001c3                          // cbnz          x3, 9b0 <sk_store_a8_aarch64+0x78>
  .long  0x0e0e3e08                          // umov          w8, v16.h[3]
  .long  0x0e0a3e0a                          // umov          w10, v16.h[2]
  .long  0x0e063e0b                          // umov          w11, v16.h[1]
  .long  0x0e023e0c                          // umov          w12, v16.h[0]
  .long  0x39000d28                          // strb          w8, [x9,#3]
  .long  0x3900092a                          // strb          w10, [x9,#2]
  .long  0x3900052b                          // strb          w11, [x9,#1]
  .long  0x3900012c                          // strb          w12, [x9]
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x91004028                          // add           x8, x1, #0x10
  .long  0xaa0803e1                          // mov           x1, x8
  .long  0x910043ff                          // add           sp, sp, #0x10
  .long  0xd61f0080                          // br            x4
  .long  0x0e023e0c                          // umov          w12, v16.h[0]
  .long  0x3900012c                          // strb          w12, [x9]
  .long  0xf100047f                          // cmp           x3, #0x1
  .long  0x54ffff00                          // b.eq          99c <sk_store_a8_aarch64+0x64>
  .long  0x3900052b                          // strb          w11, [x9,#1]
  .long  0xf100087f                          // cmp           x3, #0x2
  .long  0x54fffea0                          // b.eq          99c <sk_store_a8_aarch64+0x64>
  .long  0x3900092a                          // strb          w10, [x9,#2]
  .long  0xf1000c7f                          // cmp           x3, #0x3
  .long  0x54fffe40                          // b.eq          99c <sk_store_a8_aarch64+0x64>
  .long  0x910033e9                          // add           x9, sp, #0xc
  .long  0x320007ea                          // orr           w10, wzr, #0x3
  .long  0x8b08000b                          // add           x11, x0, x8
  .long  0xb2400528                          // orr           x8, x9, #0x3
  .long  0xcb030149                          // sub           x9, x10, x3
  .long  0x91000d6a                          // add           x10, x11, #0x3
  .long  0x3840150b                          // ldrb          w11, [x8],#1
  .long  0x91000529                          // add           x9, x9, #0x1
  .long  0x3800154b                          // strb          w11, [x10],#1
  .long  0xb5ffffa9                          // cbnz          x9, 9f0 <sk_store_a8_aarch64+0xb8>
  .long  0x17ffffe7                          // b             99c <sk_store_a8_aarch64+0x64>

.globl _sk_load_565_aarch64
_sk_load_565_aarch64:
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x8b000508                          // add           x8, x8, x0, lsl #1
  .long  0xb5000303                          // cbnz          x3, a70 <sk_load_565_aarch64+0x6c>
  .long  0xfd400100                          // ldr           d0, [x8]
  .long  0x9101a048                          // add           x8, x2, #0x68
  .long  0x4d40c901                          // ld1r          {v1.4s}, [x8]
  .long  0x9101b048                          // add           x8, x2, #0x6c
  .long  0x4d40c902                          // ld1r          {v2.4s}, [x8]
  .long  0x9101c048                          // add           x8, x2, #0x70
  .long  0x4d40c903                          // ld1r          {v3.4s}, [x8]
  .long  0x2f10a400                          // uxtl          v0.4s, v0.4h
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x4e201c21                          // and           v1.16b, v1.16b, v0.16b
  .long  0x4e201c42                          // and           v2.16b, v2.16b, v0.16b
  .long  0x4e201c60                          // and           v0.16b, v3.16b, v0.16b
  .long  0x2d4ec043                          // ldp           s3, s16, [x2,#116]
  .long  0x4e21d821                          // scvtf         v1.4s, v1.4s
  .long  0xbd407c52                          // ldr           s18, [x2,#124]
  .long  0x4e21d811                          // scvtf         v17.4s, v0.4s
  .long  0x4f839020                          // fmul          v0.4s, v1.4s, v3.s[0]
  .long  0x4d40c843                          // ld1r          {v3.4s}, [x2]
  .long  0x4e21d842                          // scvtf         v2.4s, v2.4s
  .long  0x4f909041                          // fmul          v1.4s, v2.4s, v16.s[0]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0x4f929222                          // fmul          v2.4s, v17.4s, v18.s[0]
  .long  0xd61f0080                          // br            x4
  .long  0xd10043ff                          // sub           sp, sp, #0x10
  .long  0xaa1f03e9                          // mov           x9, xzr
  .long  0x2f00e400                          // movi          d0, #0x0
  .long  0x910023ea                          // add           x10, sp, #0x8
  .long  0xfd0007e0                          // str           d0, [sp,#8]
  .long  0x7869790b                          // ldrh          w11, [x8,x9,lsl #1]
  .long  0x7829794b                          // strh          w11, [x10,x9,lsl #1]
  .long  0x91000529                          // add           x9, x9, #0x1
  .long  0xeb09007f                          // cmp           x3, x9
  .long  0x54ffff81                          // b.ne          a84 <sk_load_565_aarch64+0x80>
  .long  0xfd4007e0                          // ldr           d0, [sp,#8]
  .long  0x910043ff                          // add           sp, sp, #0x10
  .long  0x17ffffde                          // b             a18 <sk_load_565_aarch64+0x14>

.globl _sk_store_565_aarch64
_sk_store_565_aarch64:
  .long  0xd10043ff                          // sub           sp, sp, #0x10
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0x2d504450                          // ldp           s16, s17, [x2,#128]
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x4f909012                          // fmul          v18.4s, v0.4s, v16.s[0]
  .long  0x4f919031                          // fmul          v17.4s, v1.4s, v17.s[0]
  .long  0x6e21aa52                          // fcvtnu        v18.4s, v18.4s
  .long  0x6e21aa31                          // fcvtnu        v17.4s, v17.4s
  .long  0x4f909050                          // fmul          v16.4s, v2.4s, v16.s[0]
  .long  0x4f2b5652                          // shl           v18.4s, v18.4s, #11
  .long  0x4f255631                          // shl           v17.4s, v17.4s, #5
  .long  0x6e21aa10                          // fcvtnu        v16.4s, v16.4s
  .long  0x4eb21e31                          // orr           v17.16b, v17.16b, v18.16b
  .long  0x4eb01e30                          // orr           v16.16b, v17.16b, v16.16b
  .long  0x8b000509                          // add           x9, x8, x0, lsl #1
  .long  0x0e612a10                          // xtn           v16.4h, v16.4s
  .long  0xfd0007f0                          // str           d16, [sp,#8]
  .long  0xb50000e3                          // cbnz          x3, b04 <sk_store_565_aarch64+0x60>
  .long  0xfd000130                          // str           d16, [x9]
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x91004028                          // add           x8, x1, #0x10
  .long  0xaa0803e1                          // mov           x1, x8
  .long  0x910043ff                          // add           sp, sp, #0x10
  .long  0xd61f0080                          // br            x4
  .long  0x0d004130                          // st1           {v16.h}[0], [x9]
  .long  0xf100047f                          // cmp           x3, #0x1
  .long  0x54ffff20                          // b.eq          af0 <sk_store_565_aarch64+0x4c>
  .long  0x0e063e0a                          // umov          w10, v16.h[1]
  .long  0x7900052a                          // strh          w10, [x9,#2]
  .long  0xf100087f                          // cmp           x3, #0x2
  .long  0x54fffea0                          // b.eq          af0 <sk_store_565_aarch64+0x4c>
  .long  0x0e0a3e0a                          // umov          w10, v16.h[2]
  .long  0x7900092a                          // strh          w10, [x9,#4]
  .long  0xf1000c7f                          // cmp           x3, #0x3
  .long  0x54fffe20                          // b.eq          af0 <sk_store_565_aarch64+0x4c>
  .long  0x910023e9                          // add           x9, sp, #0x8
  .long  0x320007ea                          // orr           w10, wzr, #0x3
  .long  0x8b00050b                          // add           x11, x8, x0, lsl #1
  .long  0xb27f0528                          // orr           x8, x9, #0x6
  .long  0xcb030149                          // sub           x9, x10, x3
  .long  0x9100196a                          // add           x10, x11, #0x6
  .long  0x7840250b                          // ldrh          w11, [x8],#2
  .long  0x91000529                          // add           x9, x9, #0x1
  .long  0x7800254b                          // strh          w11, [x10],#2
  .long  0xb5ffffa9                          // cbnz          x9, b48 <sk_store_565_aarch64+0xa4>
  .long  0x17ffffe6                          // b             af0 <sk_store_565_aarch64+0x4c>

.globl _sk_load_8888_aarch64
_sk_load_8888_aarch64:
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x8b000908                          // add           x8, x8, x0, lsl #2
  .long  0xb50002c3                          // cbnz          x3, bc0 <sk_load_8888_aarch64+0x64>
  .long  0x3dc00100                          // ldr           q0, [x8]
  .long  0x91004048                          // add           x8, x2, #0x10
  .long  0x4d40c901                          // ld1r          {v1.4s}, [x8]
  .long  0xbd400c43                          // ldr           s3, [x2,#12]
  .long  0x6f380402                          // ushr          v2.4s, v0.4s, #8
  .long  0x6f300410                          // ushr          v16.4s, v0.4s, #16
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x4e201c31                          // and           v17.16b, v1.16b, v0.16b
  .long  0x4e221c22                          // and           v2.16b, v1.16b, v2.16b
  .long  0x4e301c21                          // and           v1.16b, v1.16b, v16.16b
  .long  0x6f280400                          // ushr          v0.4s, v0.4s, #24
  .long  0x4e21da30                          // scvtf         v16.4s, v17.4s
  .long  0x4e21d842                          // scvtf         v2.4s, v2.4s
  .long  0x4e21d831                          // scvtf         v17.4s, v1.4s
  .long  0x4e21d812                          // scvtf         v18.4s, v0.4s
  .long  0x4f839200                          // fmul          v0.4s, v16.4s, v3.s[0]
  .long  0x4f839041                          // fmul          v1.4s, v2.4s, v3.s[0]
  .long  0x4f839222                          // fmul          v2.4s, v17.4s, v3.s[0]
  .long  0x4f839243                          // fmul          v3.4s, v18.4s, v3.s[0]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0080                          // br            x4
  .long  0xd10043ff                          // sub           sp, sp, #0x10
  .long  0xaa1f03e9                          // mov           x9, xzr
  .long  0x6f00e400                          // movi          v0.2d, #0x0
  .long  0x910003ea                          // mov           x10, sp
  .long  0x3d8003e0                          // str           q0, [sp]
  .long  0xb869790b                          // ldr           w11, [x8,x9,lsl #2]
  .long  0xb829794b                          // str           w11, [x10,x9,lsl #2]
  .long  0x91000529                          // add           x9, x9, #0x1
  .long  0xeb09007f                          // cmp           x3, x9
  .long  0x54ffff81                          // b.ne          bd4 <sk_load_8888_aarch64+0x78>
  .long  0x3cc107e0                          // ldr           q0, [sp],#16
  .long  0x17ffffe1                          // b             b70 <sk_load_8888_aarch64+0x14>

.globl _sk_store_8888_aarch64
_sk_store_8888_aarch64:
  .long  0xd10043ff                          // sub           sp, sp, #0x10
  .long  0xbd400850                          // ldr           s16, [x2,#8]
  .long  0xf9400028                          // ldr           x8, [x1]
  .long  0x4f909032                          // fmul          v18.4s, v1.4s, v16.s[0]
  .long  0x4f909011                          // fmul          v17.4s, v0.4s, v16.s[0]
  .long  0x6e21aa52                          // fcvtnu        v18.4s, v18.4s
  .long  0x6e21aa31                          // fcvtnu        v17.4s, v17.4s
  .long  0x4f285652                          // shl           v18.4s, v18.4s, #8
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x4eb11e51                          // orr           v17.16b, v18.16b, v17.16b
  .long  0x4f909052                          // fmul          v18.4s, v2.4s, v16.s[0]
  .long  0x4f909070                          // fmul          v16.4s, v3.4s, v16.s[0]
  .long  0x6e21aa52                          // fcvtnu        v18.4s, v18.4s
  .long  0x6e21aa10                          // fcvtnu        v16.4s, v16.4s
  .long  0x4f305652                          // shl           v18.4s, v18.4s, #16
  .long  0x4eb21e31                          // orr           v17.16b, v17.16b, v18.16b
  .long  0x4f385610                          // shl           v16.4s, v16.4s, #24
  .long  0x8b000909                          // add           x9, x8, x0, lsl #2
  .long  0x4eb01e30                          // orr           v16.16b, v17.16b, v16.16b
  .long  0x3d8003f0                          // str           q16, [sp]
  .long  0xb5000103                          // cbnz          x3, c60 <sk_store_8888_aarch64+0x70>
  .long  0x3dc003f0                          // ldr           q16, [sp]
  .long  0x3d800130                          // str           q16, [x9]
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x91004028                          // add           x8, x1, #0x10
  .long  0xaa0803e1                          // mov           x1, x8
  .long  0x910043ff                          // add           sp, sp, #0x10
  .long  0xd61f0080                          // br            x4
  .long  0x0d008130                          // st1           {v16.s}[0], [x9]
  .long  0xf100047f                          // cmp           x3, #0x1
  .long  0x54ffff20                          // b.eq          c4c <sk_store_8888_aarch64+0x5c>
  .long  0x0e0c3e0a                          // mov           w10, v16.s[1]
  .long  0xb900052a                          // str           w10, [x9,#4]
  .long  0xf100087f                          // cmp           x3, #0x2
  .long  0x54fffea0                          // b.eq          c4c <sk_store_8888_aarch64+0x5c>
  .long  0x0e143e0a                          // mov           w10, v16.s[2]
  .long  0xb900092a                          // str           w10, [x9,#8]
  .long  0xf1000c7f                          // cmp           x3, #0x3
  .long  0x54fffe20                          // b.eq          c4c <sk_store_8888_aarch64+0x5c>
  .long  0x910003e9                          // mov           x9, sp
  .long  0x320007ea                          // orr           w10, wzr, #0x3
  .long  0x8b00090b                          // add           x11, x8, x0, lsl #2
  .long  0xb27e0528                          // orr           x8, x9, #0xc
  .long  0xcb030149                          // sub           x9, x10, x3
  .long  0x9100316a                          // add           x10, x11, #0xc
  .long  0xb840450b                          // ldr           w11, [x8],#4
  .long  0x91000529                          // add           x9, x9, #0x1
  .long  0xb800454b                          // str           w11, [x10],#4
  .long  0xb5ffffa9                          // cbnz          x9, ca4 <sk_store_8888_aarch64+0xb4>
  .long  0x17ffffe6                          // b             c4c <sk_store_8888_aarch64+0x5c>

.globl _sk_load_f16_aarch64
_sk_load_f16_aarch64:
  .long  0xa8c11028                          // ldp           x8, x4, [x1],#16
  .long  0xf9400108                          // ldr           x8, [x8]
  .long  0x8b000d08                          // add           x8, x8, x0, lsl #3
  .long  0x0c400510                          // ld4           {v16.4h-v19.4h}, [x8]
  .long  0x0e217a00                          // fcvtl         v0.4s, v16.4h
  .long  0x0e217a21                          // fcvtl         v1.4s, v17.4h
  .long  0x0e217a42                          // fcvtl         v2.4s, v18.4h
  .long  0x0e217a63                          // fcvtl         v3.4s, v19.4h
  .long  0xd61f0080                          // br            x4

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
  .long  0xf9400424                          // ldr           x4, [x1,#8]
  .long  0x91004021                          // add           x1, x1, #0x10
  .long  0xd61f0080                          // br            x4

.globl _sk_clamp_x_aarch64
_sk_clamp_x_aarch64:
  .long  0xa8c11028                          // ldp           x8, x4, [x1],#16
  .long  0x6f00e410                          // movi          v16.2d, #0x0
  .long  0x4e20f600                          // fmax          v0.4s, v16.4s, v0.4s
  .long  0x6f07e7f1                          // movi          v17.2d, #0xffffffffffffffff
  .long  0x4d40c910                          // ld1r          {v16.4s}, [x8]
  .long  0x4eb18610                          // add           v16.4s, v16.4s, v17.4s
  .long  0x4eb0f400                          // fmin          v0.4s, v0.4s, v16.4s
  .long  0xd61f0080                          // br            x4

.globl _sk_clamp_y_aarch64
_sk_clamp_y_aarch64:
  .long  0xa8c11028                          // ldp           x8, x4, [x1],#16
  .long  0x6f00e410                          // movi          v16.2d, #0x0
  .long  0x4e21f601                          // fmax          v1.4s, v16.4s, v1.4s
  .long  0x6f07e7f1                          // movi          v17.2d, #0xffffffffffffffff
  .long  0x4d40c910                          // ld1r          {v16.4s}, [x8]
  .long  0x4eb18610                          // add           v16.4s, v16.4s, v17.4s
  .long  0x4eb0f421                          // fmin          v1.4s, v1.4s, v16.4s
  .long  0xd61f0080                          // br            x4

.globl _sk_repeat_x_aarch64
_sk_repeat_x_aarch64:
  .long  0xa8c11028                          // ldp           x8, x4, [x1],#16
  .long  0x6f07e7f0                          // movi          v16.2d, #0xffffffffffffffff
  .long  0xbd400111                          // ldr           s17, [x8]
  .long  0x4e040632                          // dup           v18.4s, v17.s[0]
  .long  0x4eb08650                          // add           v16.4s, v18.4s, v16.4s
  .long  0x6e32fc12                          // fdiv          v18.4s, v0.4s, v18.4s
  .long  0x4e219a52                          // frintm        v18.4s, v18.4s
  .long  0x4f919251                          // fmul          v17.4s, v18.4s, v17.s[0]
  .long  0x4eb1d400                          // fsub          v0.4s, v0.4s, v17.4s
  .long  0x4eb0f400                          // fmin          v0.4s, v0.4s, v16.4s
  .long  0xd61f0080                          // br            x4

.globl _sk_repeat_y_aarch64
_sk_repeat_y_aarch64:
  .long  0xa8c11028                          // ldp           x8, x4, [x1],#16
  .long  0x6f07e7f0                          // movi          v16.2d, #0xffffffffffffffff
  .long  0xbd400111                          // ldr           s17, [x8]
  .long  0x4e040632                          // dup           v18.4s, v17.s[0]
  .long  0x4eb08650                          // add           v16.4s, v18.4s, v16.4s
  .long  0x6e32fc32                          // fdiv          v18.4s, v1.4s, v18.4s
  .long  0x4e219a52                          // frintm        v18.4s, v18.4s
  .long  0x4f919251                          // fmul          v17.4s, v18.4s, v17.s[0]
  .long  0x4eb1d421                          // fsub          v1.4s, v1.4s, v17.4s
  .long  0x4eb0f421                          // fmin          v1.4s, v1.4s, v16.4s
  .long  0xd61f0080                          // br            x4

.globl _sk_mirror_x_aarch64
_sk_mirror_x_aarch64:
  .long  0xa8c11028                          // ldp           x8, x4, [x1],#16
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
  .long  0xd61f0080                          // br            x4

.globl _sk_mirror_y_aarch64
_sk_mirror_y_aarch64:
  .long  0xa8c11028                          // ldp           x8, x4, [x1],#16
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
  .long  0xd61f0080                          // br            x4

.globl _sk_matrix_2x3_aarch64
_sk_matrix_2x3_aarch64:
  .long  0xa8c11028                          // ldp           x8, x4, [x1],#16
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
  .long  0xd61f0080                          // br            x4

.globl _sk_matrix_3x4_aarch64
_sk_matrix_3x4_aarch64:
  .long  0xa8c11028                          // ldp           x8, x4, [x1],#16
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
  .long  0xd61f0080                          // br            x4

.globl _sk_matrix_perspective_aarch64
_sk_matrix_perspective_aarch64:
  .long  0xa8c11028                          // ldp           x8, x4, [x1],#16
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
  .long  0xd61f0080                          // br            x4

.globl _sk_linear_gradient_2stops_aarch64
_sk_linear_gradient_2stops_aarch64:
  .long  0xa8c11028                          // ldp           x8, x4, [x1],#16
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
  .long  0xd61f0080                          // br            x4
#elif defined(__arm__)
.balign 4

.globl _sk_start_pipeline_vfp4
_sk_start_pipeline_vfp4:
  .long  0xe92d41f0                          // push          {r4, r5, r6, r7, r8, lr}
  .long  0xe1a05001                          // mov           r5, r1
  .long  0xe1a07000                          // mov           r7, r0
  .long  0xe1a06003                          // mov           r6, r3
  .long  0xe1a08002                          // mov           r8, r2
  .long  0xe495c004                          // ldr           ip, [r5], #4
  .long  0xe2870002                          // add           r0, r7, #2
  .long  0xea000010                          // b             64 <sk_start_pipeline_vfp4+0x64>
  .long  0xf2800010                          // vmov.i32      d0, #0
  .long  0xe1a00007                          // mov           r0, r7
  .long  0xf2801010                          // vmov.i32      d1, #0
  .long  0xe1a01005                          // mov           r1, r5
  .long  0xf2802010                          // vmov.i32      d2, #0
  .long  0xe1a02008                          // mov           r2, r8
  .long  0xf2803010                          // vmov.i32      d3, #0
  .long  0xe3a03000                          // mov           r3, #0
  .long  0xf2804010                          // vmov.i32      d4, #0
  .long  0xe1a0400c                          // mov           r4, ip
  .long  0xf2805010                          // vmov.i32      d5, #0
  .long  0xf2806010                          // vmov.i32      d6, #0
  .long  0xf2807010                          // vmov.i32      d7, #0
  .long  0xe12fff34                          // blx           r4
  .long  0xe2870004                          // add           r0, r7, #4
  .long  0xe2877002                          // add           r7, r7, #2
  .long  0xe1a0c004                          // mov           ip, r4
  .long  0xe1500006                          // cmp           r0, r6
  .long  0x9affffec                          // bls           20 <sk_start_pipeline_vfp4+0x20>
  .long  0xe1570006                          // cmp           r7, r6
  .long  0x08bd81f0                          // popeq         {r4, r5, r6, r7, r8, pc}
  .long  0xe0463007                          // sub           r3, r6, r7
  .long  0xe1a00007                          // mov           r0, r7
  .long  0xe1a01005                          // mov           r1, r5
  .long  0xe1a02008                          // mov           r2, r8
  .long  0xf2800010                          // vmov.i32      d0, #0
  .long  0xf2801010                          // vmov.i32      d1, #0
  .long  0xf2802010                          // vmov.i32      d2, #0
  .long  0xf2803010                          // vmov.i32      d3, #0
  .long  0xf2804010                          // vmov.i32      d4, #0
  .long  0xf2805010                          // vmov.i32      d5, #0
  .long  0xf2806010                          // vmov.i32      d6, #0
  .long  0xf2807010                          // vmov.i32      d7, #0
  .long  0xe8bd41f0                          // pop           {r4, r5, r6, r7, r8, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_just_return_vfp4
_sk_just_return_vfp4:
  .long  0xe12fff1e                          // bx            lr

.globl _sk_seed_shader_vfp4
_sk_seed_shader_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xee800b90                          // vdup.32       d16, r0
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xf4a22c9f                          // vld1.32       {d2[]}, [r2 :32]
  .long  0xf2803010                          // vmov.i32      d3, #0
  .long  0xf4ee1c9f                          // vld1.32       {d17[]}, [lr :32]
  .long  0xe282e004                          // add           lr, r2, #4
  .long  0xf3fb1621                          // vcvt.f32.s32  d17, d17
  .long  0xf4ee2c9f                          // vld1.32       {d18[]}, [lr :32]
  .long  0xf2804010                          // vmov.i32      d4, #0
  .long  0xf2400da2                          // vadd.f32      d16, d16, d18
  .long  0xf2805010                          // vmov.i32      d5, #0
  .long  0xedd23b05                          // vldr          d19, [r2, #20]
  .long  0xf2011da2                          // vadd.f32      d1, d17, d18
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2030da0                          // vadd.f32      d0, d19, d16
  .long  0xf2806010                          // vmov.i32      d6, #0
  .long  0xf2807010                          // vmov.i32      d7, #0
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_constant_color_vfp4
_sk_constant_color_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf46e0a0f                          // vld1.8        {d16-d17}, [lr]
  .long  0xf3b40c20                          // vdup.32       d0, d16[0]
  .long  0xf3bc1c20                          // vdup.32       d1, d16[1]
  .long  0xf3b42c21                          // vdup.32       d2, d17[0]
  .long  0xf3bc3c21                          // vdup.32       d3, d17[1]
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_clear_vfp4
_sk_clear_vfp4:
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2800010                          // vmov.i32      d0, #0
  .long  0xf2801010                          // vmov.i32      d1, #0
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2802010                          // vmov.i32      d2, #0
  .long  0xf2803010                          // vmov.i32      d3, #0
  .long  0xe12fff1c                          // bx            ip

.globl _sk_plus__vfp4
_sk_plus__vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xe591e004                          // ldr           lr, [r1, #4]
  .long  0xe281c008                          // add           ip, r1, #8
  .long  0xf2000d04                          // vadd.f32      d0, d0, d4
  .long  0xe1a0100c                          // mov           r1, ip
  .long  0xf2011d05                          // vadd.f32      d1, d1, d5
  .long  0xf2022d06                          // vadd.f32      d2, d2, d6
  .long  0xe1a0c00e                          // mov           ip, lr
  .long  0xf2033d07                          // vadd.f32      d3, d3, d7
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_srcover_vfp4
_sk_srcover_vfp4:
  .long  0xf4e20c9f                          // vld1.32       {d16[]}, [r2 :32]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2600d83                          // vsub.f32      d16, d16, d3
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2040c30                          // vfma.f32      d0, d4, d16
  .long  0xf2051c30                          // vfma.f32      d1, d5, d16
  .long  0xf2062c30                          // vfma.f32      d2, d6, d16
  .long  0xf2073c30                          // vfma.f32      d3, d7, d16
  .long  0xe12fff1c                          // bx            ip

.globl _sk_dstover_vfp4
_sk_dstover_vfp4:
  .long  0xf4e20c9f                          // vld1.32       {d16[]}, [r2 :32]
  .long  0xf2651115                          // vorr          d17, d5, d5
  .long  0xf2604d87                          // vsub.f32      d20, d16, d7
  .long  0xf2640114                          // vorr          d16, d4, d4
  .long  0xf2662116                          // vorr          d18, d6, d6
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2673117                          // vorr          d19, d7, d7
  .long  0xe2811008                          // add           r1, r1, #8
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
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2000f20                          // vmax.f32      d0, d0, d16
  .long  0xf2011f20                          // vmax.f32      d1, d1, d16
  .long  0xf2022f20                          // vmax.f32      d2, d2, d16
  .long  0xf2033f20                          // vmax.f32      d3, d3, d16
  .long  0xe12fff1c                          // bx            ip

.globl _sk_clamp_1_vfp4
_sk_clamp_1_vfp4:
  .long  0xf4e20c9f                          // vld1.32       {d16[]}, [r2 :32]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2200f20                          // vmin.f32      d0, d0, d16
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2211f20                          // vmin.f32      d1, d1, d16
  .long  0xf2222f20                          // vmin.f32      d2, d2, d16
  .long  0xf2233f20                          // vmin.f32      d3, d3, d16
  .long  0xe12fff1c                          // bx            ip

.globl _sk_clamp_a_vfp4
_sk_clamp_a_vfp4:
  .long  0xf4e20c9f                          // vld1.32       {d16[]}, [r2 :32]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2233f20                          // vmin.f32      d3, d3, d16
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2200f03                          // vmin.f32      d0, d0, d3
  .long  0xf2211f03                          // vmin.f32      d1, d1, d3
  .long  0xf2222f03                          // vmin.f32      d2, d2, d3
  .long  0xe12fff1c                          // bx            ip

.globl _sk_set_rgb_vfp4
_sk_set_rgb_vfp4:
  .long  0xe92d4010                          // push          {r4, lr}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe28e4008                          // add           r4, lr, #8
  .long  0xf4ae0c9f                          // vld1.32       {d0[]}, [lr :32]
  .long  0xf4a42c9f                          // vld1.32       {d2[]}, [r4 :32]
  .long  0xe28e4004                          // add           r4, lr, #4
  .long  0xf4a41c9f                          // vld1.32       {d1[]}, [r4 :32]
  .long  0xe8bd4010                          // pop           {r4, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_swap_rb_vfp4
_sk_swap_rb_vfp4:
  .long  0xeef00b40                          // vmov.f64      d16, d0
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xeeb00b42                          // vmov.f64      d0, d2
  .long  0xeeb02b60                          // vmov.f64      d2, d16
  .long  0xe12fff1c                          // bx            ip

.globl _sk_swap_vfp4
_sk_swap_vfp4:
  .long  0xeef00b43                          // vmov.f64      d16, d3
  .long  0xe591c004                          // ldr           ip, [r1, #4]
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
  .long  0xe12fff1c                          // bx            ip

.globl _sk_move_src_dst_vfp4
_sk_move_src_dst_vfp4:
  .long  0xeeb04b40                          // vmov.f64      d4, d0
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xeeb05b41                          // vmov.f64      d5, d1
  .long  0xeeb06b42                          // vmov.f64      d6, d2
  .long  0xeeb07b43                          // vmov.f64      d7, d3
  .long  0xe12fff1c                          // bx            ip

.globl _sk_move_dst_src_vfp4
_sk_move_dst_src_vfp4:
  .long  0xeeb00b44                          // vmov.f64      d0, d4
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xeeb01b45                          // vmov.f64      d1, d5
  .long  0xeeb02b46                          // vmov.f64      d2, d6
  .long  0xeeb03b47                          // vmov.f64      d3, d7
  .long  0xe12fff1c                          // bx            ip

.globl _sk_premul_vfp4
_sk_premul_vfp4:
  .long  0xf3000d13                          // vmul.f32      d0, d0, d3
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf3011d13                          // vmul.f32      d1, d1, d3
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf3022d13                          // vmul.f32      d2, d2, d3
  .long  0xe12fff1c                          // bx            ip

.globl _sk_unpremul_vfp4
_sk_unpremul_vfp4:
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xed928a00                          // vldr          s16, [r2]
  .long  0xf2c00010                          // vmov.i32      d16, #0
  .long  0xf3f91503                          // vceq.f32      d17, d3, #0
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xeec89a23                          // vdiv.f32      s19, s16, s7
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xee889a03                          // vdiv.f32      s18, s16, s6
  .long  0xf3501199                          // vbsl          d17, d16, d9
  .long  0xf3010d90                          // vmul.f32      d0, d17, d0
  .long  0xf3011d91                          // vmul.f32      d1, d17, d1
  .long  0xf3012d92                          // vmul.f32      d2, d17, d2
  .long  0xecbd8b04                          // vpop          {d8-d9}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_from_srgb_vfp4
_sk_from_srgb_vfp4:
  .long  0xed2d8b02                          // vpush         {d8}
  .long  0xe282c03c                          // add           ip, r2, #60
  .long  0xed928a10                          // vldr          s16, [r2, #64]
  .long  0xf3402d10                          // vmul.f32      d18, d0, d0
  .long  0xf4ec0c9f                          // vld1.32       {d16[]}, [ip :32]
  .long  0xe282c038                          // add           ip, r2, #56
  .long  0xf3413d11                          // vmul.f32      d19, d1, d1
  .long  0xf4ec1c9f                          // vld1.32       {d17[]}, [ip :32]
  .long  0xe282c044                          // add           ip, r2, #68
  .long  0xf26141b1                          // vorr          d20, d17, d17
  .long  0xf26171b1                          // vorr          d23, d17, d17
  .long  0xf4ec8c9f                          // vld1.32       {d24[]}, [ip :32]
  .long  0xf2404c30                          // vfma.f32      d20, d0, d16
  .long  0xe282c034                          // add           ip, r2, #52
  .long  0xf2417c30                          // vfma.f32      d23, d1, d16
  .long  0xf2421c30                          // vfma.f32      d17, d2, d16
  .long  0xf3425d12                          // vmul.f32      d21, d2, d2
  .long  0xf2e16948                          // vmul.f32      d22, d1, d8[0]
  .long  0xf2e00948                          // vmul.f32      d16, d0, d8[0]
  .long  0xf2e29948                          // vmul.f32      d25, d2, d8[0]
  .long  0xf3282e82                          // vcgt.f32      d2, d24, d2
  .long  0xf3281e81                          // vcgt.f32      d1, d24, d1
  .long  0xf3280e80                          // vcgt.f32      d0, d24, d0
  .long  0xf4ec8c9f                          // vld1.32       {d24[]}, [ip :32]
  .long  0xf268a1b8                          // vorr          d26, d24, d24
  .long  0xf242acb4                          // vfma.f32      d26, d18, d20
  .long  0xf26821b8                          // vorr          d18, d24, d24
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2432cb7                          // vfma.f32      d18, d19, d23
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2458cb1                          // vfma.f32      d24, d21, d17
  .long  0xf31001ba                          // vbsl          d0, d16, d26
  .long  0xf31611b2                          // vbsl          d1, d22, d18
  .long  0xf31921b8                          // vbsl          d2, d25, d24
  .long  0xecbd8b02                          // vpop          {d8}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_to_srgb_vfp4
_sk_to_srgb_vfp4:
  .long  0xed2d8b02                          // vpush         {d8}
  .long  0xf3fb0580                          // vrsqrte.f32   d16, d0
  .long  0xe282c050                          // add           ip, r2, #80
  .long  0xf3fb1581                          // vrsqrte.f32   d17, d1
  .long  0xed928a12                          // vldr          s16, [r2, #72]
  .long  0xf3fb2582                          // vrsqrte.f32   d18, d2
  .long  0xf3403db0                          // vmul.f32      d19, d16, d16
  .long  0xf3414db1                          // vmul.f32      d20, d17, d17
  .long  0xf3425db2                          // vmul.f32      d21, d18, d18
  .long  0xf2603f33                          // vrsqrts.f32   d19, d0, d19
  .long  0xf2614f34                          // vrsqrts.f32   d20, d1, d20
  .long  0xf2625f35                          // vrsqrts.f32   d21, d2, d21
  .long  0xf3400db3                          // vmul.f32      d16, d16, d19
  .long  0xf3411db4                          // vmul.f32      d17, d17, d20
  .long  0xf3422db5                          // vmul.f32      d18, d18, d21
  .long  0xf3fb3520                          // vrecpe.f32    d19, d16
  .long  0xf3fb4521                          // vrecpe.f32    d20, d17
  .long  0xf3fb6522                          // vrecpe.f32    d22, d18
  .long  0xf3fb55a2                          // vrsqrte.f32   d21, d18
  .long  0xf3fb75a0                          // vrsqrte.f32   d23, d16
  .long  0xf3fb85a1                          // vrsqrte.f32   d24, d17
  .long  0xf2409fb3                          // vrecps.f32    d25, d16, d19
  .long  0xf241afb4                          // vrecps.f32    d26, d17, d20
  .long  0xf242bfb6                          // vrecps.f32    d27, d18, d22
  .long  0xf345cdb5                          // vmul.f32      d28, d21, d21
  .long  0xf347ddb7                          // vmul.f32      d29, d23, d23
  .long  0xf348edb8                          // vmul.f32      d30, d24, d24
  .long  0xf2622fbc                          // vrsqrts.f32   d18, d18, d28
  .long  0xf2600fbd                          // vrsqrts.f32   d16, d16, d29
  .long  0xf2611fbe                          // vrsqrts.f32   d17, d17, d30
  .long  0xf3433db9                          // vmul.f32      d19, d19, d25
  .long  0xf4ec9c9f                          // vld1.32       {d25[]}, [ip :32]
  .long  0xe282c054                          // add           ip, r2, #84
  .long  0xf3444dba                          // vmul.f32      d20, d20, d26
  .long  0xf3466dbb                          // vmul.f32      d22, d22, d27
  .long  0xf4ecac9f                          // vld1.32       {d26[]}, [ip :32]
  .long  0xe282c04c                          // add           ip, r2, #76
  .long  0xf26ab1ba                          // vorr          d27, d26, d26
  .long  0xf249bcb3                          // vfma.f32      d27, d25, d19
  .long  0xf26a31ba                          // vorr          d19, d26, d26
  .long  0xf2493cb4                          // vfma.f32      d19, d25, d20
  .long  0xf4ec4c9f                          // vld1.32       {d20[]}, [ip :32]
  .long  0xf249acb6                          // vfma.f32      d26, d25, d22
  .long  0xe282c058                          // add           ip, r2, #88
  .long  0xf3452db2                          // vmul.f32      d18, d21, d18
  .long  0xf3470db0                          // vmul.f32      d16, d23, d16
  .long  0xf3481db1                          // vmul.f32      d17, d24, d17
  .long  0xf2e05948                          // vmul.f32      d21, d0, d8[0]
  .long  0xf244bcb0                          // vfma.f32      d27, d20, d16
  .long  0xf4ec0c9f                          // vld1.32       {d16[]}, [ip :32]
  .long  0xf2443cb1                          // vfma.f32      d19, d20, d17
  .long  0xf244acb2                          // vfma.f32      d26, d20, d18
  .long  0xf4e24c9f                          // vld1.32       {d20[]}, [r2 :32]
  .long  0xf2e11948                          // vmul.f32      d17, d1, d8[0]
  .long  0xf2e22948                          // vmul.f32      d18, d2, d8[0]
  .long  0xf3201e81                          // vcgt.f32      d1, d16, d1
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf3200e80                          // vcgt.f32      d0, d16, d0
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf3202e82                          // vcgt.f32      d2, d16, d2
  .long  0xf2640fab                          // vmin.f32      d16, d20, d27
  .long  0xf2643fa3                          // vmin.f32      d19, d20, d19
  .long  0xf2644faa                          // vmin.f32      d20, d20, d26
  .long  0xf31501b0                          // vbsl          d0, d21, d16
  .long  0xf31111b3                          // vbsl          d1, d17, d19
  .long  0xf31221b4                          // vbsl          d2, d18, d20
  .long  0xecbd8b02                          // vpop          {d8}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_scale_1_float_vfp4
_sk_scale_1_float_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xed2d8b02                          // vpush         {d8}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xed9e8a00                          // vldr          s16, [lr]
  .long  0xf2a00948                          // vmul.f32      d0, d0, d8[0]
  .long  0xf2a11948                          // vmul.f32      d1, d1, d8[0]
  .long  0xf2a22948                          // vmul.f32      d2, d2, d8[0]
  .long  0xf2a33948                          // vmul.f32      d3, d3, d8[0]
  .long  0xecbd8b02                          // vpop          {d8}
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_scale_u8_vfp4
_sk_scale_u8_vfp4:
  .long  0xe92d4830                          // push          {r4, r5, fp, lr}
  .long  0xed2d8b02                          // vpush         {d8}
  .long  0xe24dd008                          // sub           sp, sp, #8
  .long  0xe5914000                          // ldr           r4, [r1]
  .long  0xe3530000                          // cmp           r3, #0
  .long  0xe5944000                          // ldr           r4, [r4]
  .long  0xe084c000                          // add           ip, r4, r0
  .long  0x1a000014                          // bne           594 <sk_scale_u8_vfp4+0x74>
  .long  0xe1dc50b0                          // ldrh          r5, [ip]
  .long  0xe1cd50b0                          // strh          r5, [sp]
  .long  0xe1a0500d                          // mov           r5, sp
  .long  0xf4e5041f                          // vld1.16       {d16[0]}, [r5 :16]
  .long  0xf3c80a30                          // vmovl.u8      q8, d16
  .long  0xed928a03                          // vldr          s16, [r2, #12]
  .long  0xf3c7201f                          // vmov.i32      d18, #255
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf3d00a30                          // vmovl.u16     q8, d16
  .long  0xf24001b2                          // vand          d16, d16, d18
  .long  0xf3fb06a0                          // vcvt.f32.u32  d16, d16
  .long  0xf2e009c8                          // vmul.f32      d16, d16, d8[0]
  .long  0xf3000d90                          // vmul.f32      d0, d16, d0
  .long  0xf3001d91                          // vmul.f32      d1, d16, d1
  .long  0xf3002d92                          // vmul.f32      d2, d16, d2
  .long  0xf3003d93                          // vmul.f32      d3, d16, d3
  .long  0xe28dd008                          // add           sp, sp, #8
  .long  0xecbd8b02                          // vpop          {d8}
  .long  0xe8bd4830                          // pop           {r4, r5, fp, lr}
  .long  0xe12fff1c                          // bx            ip
  .long  0xe28d4006                          // add           r4, sp, #6
  .long  0xe3a0e000                          // mov           lr, #0
  .long  0xe5cde007                          // strb          lr, [sp, #7]
  .long  0xe5cde006                          // strb          lr, [sp, #6]
  .long  0xe7dc500e                          // ldrb          r5, [ip, lr]
  .long  0xe7c4500e                          // strb          r5, [r4, lr]
  .long  0xe28ee001                          // add           lr, lr, #1
  .long  0xe153000e                          // cmp           r3, lr
  .long  0x1afffffa                          // bne           5a4 <sk_scale_u8_vfp4+0x84>
  .long  0xf4e4041f                          // vld1.16       {d16[0]}, [r4 :16]
  .long  0xeaffffe3                          // b             550 <sk_scale_u8_vfp4+0x30>

.globl _sk_lerp_1_float_vfp4
_sk_lerp_1_float_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xf2600d04                          // vsub.f32      d16, d0, d4
  .long  0xf2611d05                          // vsub.f32      d17, d1, d5
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2622d06                          // vsub.f32      d18, d2, d6
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2633d07                          // vsub.f32      d19, d3, d7
  .long  0xf4ee4c9f                          // vld1.32       {d20[]}, [lr :32]
  .long  0xf2240114                          // vorr          d0, d4, d4
  .long  0xf2251115                          // vorr          d1, d5, d5
  .long  0xf2262116                          // vorr          d2, d6, d6
  .long  0xf2273117                          // vorr          d3, d7, d7
  .long  0xf2000cb4                          // vfma.f32      d0, d16, d20
  .long  0xf2011cb4                          // vfma.f32      d1, d17, d20
  .long  0xf2022cb4                          // vfma.f32      d2, d18, d20
  .long  0xf2033cb4                          // vfma.f32      d3, d19, d20
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_lerp_u8_vfp4
_sk_lerp_u8_vfp4:
  .long  0xe92d4830                          // push          {r4, r5, fp, lr}
  .long  0xed2d8b02                          // vpush         {d8}
  .long  0xe24dd008                          // sub           sp, sp, #8
  .long  0xe5914000                          // ldr           r4, [r1]
  .long  0xe3530000                          // cmp           r3, #0
  .long  0xe5944000                          // ldr           r4, [r4]
  .long  0xe084c000                          // add           ip, r4, r0
  .long  0x1a00001c                          // bne           6a0 <sk_lerp_u8_vfp4+0x94>
  .long  0xe1dc50b0                          // ldrh          r5, [ip]
  .long  0xe1cd50b0                          // strh          r5, [sp]
  .long  0xe1a0500d                          // mov           r5, sp
  .long  0xf4e5041f                          // vld1.16       {d16[0]}, [r5 :16]
  .long  0xf3c80a30                          // vmovl.u8      q8, d16
  .long  0xed928a03                          // vldr          s16, [r2, #12]
  .long  0xf3c7201f                          // vmov.i32      d18, #255
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2623d06                          // vsub.f32      d19, d2, d6
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2634d07                          // vsub.f32      d20, d3, d7
  .long  0xf3d00a30                          // vmovl.u16     q8, d16
  .long  0xf2262116                          // vorr          d2, d6, d6
  .long  0xf2273117                          // vorr          d3, d7, d7
  .long  0xf24001b2                          // vand          d16, d16, d18
  .long  0xf2601d04                          // vsub.f32      d17, d0, d4
  .long  0xf3fb06a0                          // vcvt.f32.u32  d16, d16
  .long  0xf2612d05                          // vsub.f32      d18, d1, d5
  .long  0xf2240114                          // vorr          d0, d4, d4
  .long  0xf2251115                          // vorr          d1, d5, d5
  .long  0xf2e009c8                          // vmul.f32      d16, d16, d8[0]
  .long  0xf2010cb0                          // vfma.f32      d0, d17, d16
  .long  0xf2021cb0                          // vfma.f32      d1, d18, d16
  .long  0xf2032cb0                          // vfma.f32      d2, d19, d16
  .long  0xf2043cb0                          // vfma.f32      d3, d20, d16
  .long  0xe28dd008                          // add           sp, sp, #8
  .long  0xecbd8b02                          // vpop          {d8}
  .long  0xe8bd4830                          // pop           {r4, r5, fp, lr}
  .long  0xe12fff1c                          // bx            ip
  .long  0xe28d4006                          // add           r4, sp, #6
  .long  0xe3a0e000                          // mov           lr, #0
  .long  0xe5cde007                          // strb          lr, [sp, #7]
  .long  0xe5cde006                          // strb          lr, [sp, #6]
  .long  0xe7dc500e                          // ldrb          r5, [ip, lr]
  .long  0xe7c4500e                          // strb          r5, [r4, lr]
  .long  0xe28ee001                          // add           lr, lr, #1
  .long  0xe153000e                          // cmp           r3, lr
  .long  0x1afffffa                          // bne           6b0 <sk_lerp_u8_vfp4+0xa4>
  .long  0xf4e4041f                          // vld1.16       {d16[0]}, [r4 :16]
  .long  0xeaffffdb                          // b             63c <sk_lerp_u8_vfp4+0x30>

.globl _sk_lerp_565_vfp4
_sk_lerp_565_vfp4:
  .long  0xe92d4070                          // push          {r4, r5, r6, lr}
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xe24dd008                          // sub           sp, sp, #8
  .long  0xe5915000                          // ldr           r5, [r1]
  .long  0xe3530000                          // cmp           r3, #0
  .long  0xe5955000                          // ldr           r5, [r5]
  .long  0xe085c080                          // add           ip, r5, r0, lsl #1
  .long  0x1a000028                          // bne           790 <sk_lerp_565_vfp4+0xc4>
  .long  0xe59c6000                          // ldr           r6, [ip]
  .long  0xe58d6000                          // str           r6, [sp]
  .long  0xe1a0600d                          // mov           r6, sp
  .long  0xf4e6083f                          // vld1.32       {d16[0]}, [r6 :32]
  .long  0xe282606c                          // add           r6, r2, #108
  .long  0xf3d00a30                          // vmovl.u16     q8, d16
  .long  0xf3c72c1f                          // vmov.i32      d18, #65535
  .long  0xed923a1d                          // vldr          s6, [r2, #116]
  .long  0xf4e63c9f                          // vld1.32       {d19[]}, [r6 :32]
  .long  0xe2826068                          // add           r6, r2, #104
  .long  0xf24001b2                          // vand          d16, d16, d18
  .long  0xf4e61c9f                          // vld1.32       {d17[]}, [r6 :32]
  .long  0xe2826070                          // add           r6, r2, #112
  .long  0xf24111b0                          // vand          d17, d17, d16
  .long  0xf4e62c9f                          // vld1.32       {d18[]}, [r6 :32]
  .long  0xf24331b0                          // vand          d19, d19, d16
  .long  0xf24201b0                          // vand          d16, d18, d16
  .long  0xf3fb1621                          // vcvt.f32.s32  d17, d17
  .long  0xf3fb2623                          // vcvt.f32.s32  d18, d19
  .long  0xed928a1e                          // vldr          s16, [r2, #120]
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xed929a1f                          // vldr          s18, [r2, #124]
  .long  0xf2603d04                          // vsub.f32      d19, d0, d4
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2e119c3                          // vmul.f32      d17, d17, d3[0]
  .long  0xf4a23c9f                          // vld1.32       {d3[]}, [r2 :32]
  .long  0xf2614d05                          // vsub.f32      d20, d1, d5
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2625d06                          // vsub.f32      d21, d2, d6
  .long  0xf2e229c8                          // vmul.f32      d18, d18, d8[0]
  .long  0xf2e009c9                          // vmul.f32      d16, d16, d9[0]
  .long  0xf2240114                          // vorr          d0, d4, d4
  .long  0xf2251115                          // vorr          d1, d5, d5
  .long  0xf2262116                          // vorr          d2, d6, d6
  .long  0xf2030cb1                          // vfma.f32      d0, d19, d17
  .long  0xf2041cb2                          // vfma.f32      d1, d20, d18
  .long  0xf2052cb0                          // vfma.f32      d2, d21, d16
  .long  0xe28dd008                          // add           sp, sp, #8
  .long  0xecbd8b04                          // vpop          {d8-d9}
  .long  0xe8bd4070                          // pop           {r4, r5, r6, lr}
  .long  0xe12fff1c                          // bx            ip
  .long  0xe28d4004                          // add           r4, sp, #4
  .long  0xe3a0e000                          // mov           lr, #0
  .long  0xe1a05003                          // mov           r5, r3
  .long  0xe58de004                          // str           lr, [sp, #4]
  .long  0xe11c60be                          // ldrh          r6, [ip, -lr]
  .long  0xe2555001                          // subs          r5, r5, #1
  .long  0xe10460be                          // strh          r6, [r4, -lr]
  .long  0xe24ee002                          // sub           lr, lr, #2
  .long  0x1afffffa                          // bne           7a0 <sk_lerp_565_vfp4+0xd4>
  .long  0xf4e4083f                          // vld1.32       {d16[0]}, [r4 :32]
  .long  0xeaffffcf                          // b             6fc <sk_lerp_565_vfp4+0x30>

.globl _sk_load_tables_vfp4
_sk_load_tables_vfp4:
  .long  0xe92d4bf0                          // push          {r4, r5, r6, r7, r8, r9, fp, lr}
  .long  0xe24dd008                          // sub           sp, sp, #8
  .long  0xe591c000                          // ldr           ip, [r1]
  .long  0xe3530000                          // cmp           r3, #0
  .long  0xe59c5000                          // ldr           r5, [ip]
  .long  0xe085e100                          // add           lr, r5, r0, lsl #2
  .long  0x1a000025                          // bne           870 <sk_load_tables_vfp4+0xb4>
  .long  0xedde0b00                          // vldr          d16, [lr]
  .long  0xe2826010                          // add           r6, r2, #16
  .long  0xf3f01030                          // vshr.u32      d17, d16, #16
  .long  0xf3f84030                          // vshr.u32      d20, d16, #8
  .long  0xe59c4004                          // ldr           r4, [ip, #4]
  .long  0xf4e62c9f                          // vld1.32       {d18[]}, [r6 :32]
  .long  0xe59c7008                          // ldr           r7, [ip, #8]
  .long  0xf24231b0                          // vand          d19, d18, d16
  .long  0xf24211b1                          // vand          d17, d18, d17
  .long  0xf24221b4                          // vand          d18, d18, d20
  .long  0xe59c600c                          // ldr           r6, [ip, #12]
  .long  0xee335b90                          // vmov.32       r5, d19[1]
  .long  0xf3e80030                          // vshr.u32      d16, d16, #24
  .long  0xee318b90                          // vmov.32       r8, d17[1]
  .long  0xee32eb90                          // vmov.32       lr, d18[1]
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xee129b90                          // vmov.32       r9, d18[0]
  .long  0xee13cb90                          // vmov.32       ip, d19[0]
  .long  0xed921a03                          // vldr          s2, [r2, #12]
  .long  0xf2a039c1                          // vmul.f32      d3, d16, d1[0]
  .long  0xe0845105                          // add           r5, r4, r5, lsl #2
  .long  0xe0868108                          // add           r8, r6, r8, lsl #2
  .long  0xedd50a00                          // vldr          s1, [r5]
  .long  0xe087510e                          // add           r5, r7, lr, lsl #2
  .long  0xee11eb90                          // vmov.32       lr, d17[0]
  .long  0xe0877109                          // add           r7, r7, r9, lsl #2
  .long  0xedd51a00                          // vldr          s3, [r5]
  .long  0xe084410c                          // add           r4, r4, ip, lsl #2
  .long  0xed971a00                          // vldr          s2, [r7]
  .long  0xedd82a00                          // vldr          s5, [r8]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xed940a00                          // vldr          s0, [r4]
  .long  0xe086710e                          // add           r7, r6, lr, lsl #2
  .long  0xed972a00                          // vldr          s4, [r7]
  .long  0xe28dd008                          // add           sp, sp, #8
  .long  0xe8bd4bf0                          // pop           {r4, r5, r6, r7, r8, r9, fp, lr}
  .long  0xe12fff1c                          // bx            ip
  .long  0xf2c00010                          // vmov.i32      d16, #0
  .long  0xe3a04000                          // mov           r4, #0
  .long  0xe1a0500d                          // mov           r5, sp
  .long  0xedcd0b00                          // vstr          d16, [sp]
  .long  0xe79e6104                          // ldr           r6, [lr, r4, lsl #2]
  .long  0xe7856104                          // str           r6, [r5, r4, lsl #2]
  .long  0xe2844001                          // add           r4, r4, #1
  .long  0xe1530004                          // cmp           r3, r4
  .long  0x1afffffa                          // bne           880 <sk_load_tables_vfp4+0xc4>
  .long  0xeddd0b00                          // vldr          d16, [sp]
  .long  0xeaffffcf                          // b             7dc <sk_load_tables_vfp4+0x20>

.globl _sk_load_a8_vfp4
_sk_load_a8_vfp4:
  .long  0xe92d4830                          // push          {r4, r5, fp, lr}
  .long  0xe24dd008                          // sub           sp, sp, #8
  .long  0xe5914000                          // ldr           r4, [r1]
  .long  0xe3530000                          // cmp           r3, #0
  .long  0xe5944000                          // ldr           r4, [r4]
  .long  0xe084c000                          // add           ip, r4, r0
  .long  0x1a000012                          // bne           904 <sk_load_a8_vfp4+0x68>
  .long  0xe1dc50b0                          // ldrh          r5, [ip]
  .long  0xe1cd50b0                          // strh          r5, [sp]
  .long  0xe1a0500d                          // mov           r5, sp
  .long  0xf4e5041f                          // vld1.16       {d16[0]}, [r5 :16]
  .long  0xf3c80a30                          // vmovl.u8      q8, d16
  .long  0xed920a03                          // vldr          s0, [r2, #12]
  .long  0xf3c7201f                          // vmov.i32      d18, #255
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2801010                          // vmov.i32      d1, #0
  .long  0xf2802010                          // vmov.i32      d2, #0
  .long  0xf3d00a30                          // vmovl.u16     q8, d16
  .long  0xf24001b2                          // vand          d16, d16, d18
  .long  0xf3fb06a0                          // vcvt.f32.u32  d16, d16
  .long  0xf2a039c0                          // vmul.f32      d3, d16, d0[0]
  .long  0xf2800010                          // vmov.i32      d0, #0
  .long  0xe28dd008                          // add           sp, sp, #8
  .long  0xe8bd4830                          // pop           {r4, r5, fp, lr}
  .long  0xe12fff1c                          // bx            ip
  .long  0xe28d4006                          // add           r4, sp, #6
  .long  0xe3a0e000                          // mov           lr, #0
  .long  0xe5cde007                          // strb          lr, [sp, #7]
  .long  0xe5cde006                          // strb          lr, [sp, #6]
  .long  0xe7dc500e                          // ldrb          r5, [ip, lr]
  .long  0xe7c4500e                          // strb          r5, [r4, lr]
  .long  0xe28ee001                          // add           lr, lr, #1
  .long  0xe153000e                          // cmp           r3, lr
  .long  0x1afffffa                          // bne           914 <sk_load_a8_vfp4+0x78>
  .long  0xf4e4041f                          // vld1.16       {d16[0]}, [r4 :16]
  .long  0xeaffffe5                          // b             8c8 <sk_load_a8_vfp4+0x2c>

.globl _sk_store_a8_vfp4
_sk_store_a8_vfp4:
  .long  0xe92d4010                          // push          {r4, lr}
  .long  0xe282c008                          // add           ip, r2, #8
  .long  0xf2c3061f                          // vmov.i32      d16, #1056964608
  .long  0xe3530000                          // cmp           r3, #0
  .long  0xf4ec1c9f                          // vld1.32       {d17[]}, [ip :32]
  .long  0xe591c000                          // ldr           ip, [r1]
  .long  0xf2430c31                          // vfma.f32      d16, d3, d17
  .long  0xe59cc000                          // ldr           ip, [ip]
  .long  0xe08cc000                          // add           ip, ip, r0
  .long  0xf3fb07a0                          // vcvt.u32.f32  d16, d16
  .long  0x1a000007                          // bne           97c <sk_store_a8_vfp4+0x4c>
  .long  0xee30eb90                          // vmov.32       lr, d16[1]
  .long  0xee104b90                          // vmov.32       r4, d16[0]
  .long  0xe5cce001                          // strb          lr, [ip, #1]
  .long  0xe5cc4000                          // strb          r4, [ip]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe8bd4010                          // pop           {r4, lr}
  .long  0xe12fff1c                          // bx            ip
  .long  0xee10eb90                          // vmov.32       lr, d16[0]
  .long  0xe5cce000                          // strb          lr, [ip]
  .long  0xeafffff8                          // b             96c <sk_store_a8_vfp4+0x3c>

.globl _sk_load_565_vfp4
_sk_load_565_vfp4:
  .long  0xe92d4070                          // push          {r4, r5, r6, lr}
  .long  0xe24dd008                          // sub           sp, sp, #8
  .long  0xe5915000                          // ldr           r5, [r1]
  .long  0xe3530000                          // cmp           r3, #0
  .long  0xe5955000                          // ldr           r5, [r5]
  .long  0xe085c080                          // add           ip, r5, r0, lsl #1
  .long  0x1a00001e                          // bne           a20 <sk_load_565_vfp4+0x98>
  .long  0xe59c6000                          // ldr           r6, [ip]
  .long  0xe58d6000                          // str           r6, [sp]
  .long  0xe1a0600d                          // mov           r6, sp
  .long  0xf4e6083f                          // vld1.32       {d16[0]}, [r6 :32]
  .long  0xe282606c                          // add           r6, r2, #108
  .long  0xf3d00a30                          // vmovl.u16     q8, d16
  .long  0xf3c72c1f                          // vmov.i32      d18, #65535
  .long  0xf4a23c9f                          // vld1.32       {d3[]}, [r2 :32]
  .long  0xf4e63c9f                          // vld1.32       {d19[]}, [r6 :32]
  .long  0xe2826068                          // add           r6, r2, #104
  .long  0xf24001b2                          // vand          d16, d16, d18
  .long  0xf4e61c9f                          // vld1.32       {d17[]}, [r6 :32]
  .long  0xe2826070                          // add           r6, r2, #112
  .long  0xf24111b0                          // vand          d17, d17, d16
  .long  0xf4e62c9f                          // vld1.32       {d18[]}, [r6 :32]
  .long  0xf24331b0                          // vand          d19, d19, d16
  .long  0xf24201b0                          // vand          d16, d18, d16
  .long  0xf3fb1621                          // vcvt.f32.s32  d17, d17
  .long  0xf3fb2623                          // vcvt.f32.s32  d18, d19
  .long  0xed920a1d                          // vldr          s0, [r2, #116]
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xed921a1e                          // vldr          s2, [r2, #120]
  .long  0xed922a1f                          // vldr          s4, [r2, #124]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2a109c0                          // vmul.f32      d0, d17, d0[0]
  .long  0xf2a219c1                          // vmul.f32      d1, d18, d1[0]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf2a029c2                          // vmul.f32      d2, d16, d2[0]
  .long  0xe28dd008                          // add           sp, sp, #8
  .long  0xe8bd4070                          // pop           {r4, r5, r6, lr}
  .long  0xe12fff1c                          // bx            ip
  .long  0xe28d4004                          // add           r4, sp, #4
  .long  0xe3a0e000                          // mov           lr, #0
  .long  0xe1a05003                          // mov           r5, r3
  .long  0xe58de004                          // str           lr, [sp, #4]
  .long  0xe11c60be                          // ldrh          r6, [ip, -lr]
  .long  0xe2555001                          // subs          r5, r5, #1
  .long  0xe10460be                          // strh          r6, [r4, -lr]
  .long  0xe24ee002                          // sub           lr, lr, #2
  .long  0x1afffffa                          // bne           a30 <sk_load_565_vfp4+0xa8>
  .long  0xf4e4083f                          // vld1.32       {d16[0]}, [r4 :32]
  .long  0xeaffffd9                          // b             9b4 <sk_load_565_vfp4+0x2c>

.globl _sk_store_565_vfp4
_sk_store_565_vfp4:
  .long  0xe282c080                          // add           ip, r2, #128
  .long  0xf2c3361f                          // vmov.i32      d19, #1056964608
  .long  0xf2c3461f                          // vmov.i32      d20, #1056964608
  .long  0xe3530000                          // cmp           r3, #0
  .long  0xf4ec1c9f                          // vld1.32       {d17[]}, [ip :32]
  .long  0xe282c084                          // add           ip, r2, #132
  .long  0xf2403c31                          // vfma.f32      d19, d0, d17
  .long  0xf4ec2c9f                          // vld1.32       {d18[]}, [ip :32]
  .long  0xf2c3061f                          // vmov.i32      d16, #1056964608
  .long  0xf2414c32                          // vfma.f32      d20, d1, d18
  .long  0xf2420c31                          // vfma.f32      d16, d2, d17
  .long  0xe591c000                          // ldr           ip, [r1]
  .long  0xe59cc000                          // ldr           ip, [ip]
  .long  0xf3fb17a3                          // vcvt.u32.f32  d17, d19
  .long  0xe08cc080                          // add           ip, ip, r0, lsl #1
  .long  0xf3fb27a4                          // vcvt.u32.f32  d18, d20
  .long  0xf3fb07a0                          // vcvt.u32.f32  d16, d16
  .long  0xf2eb1531                          // vshl.s32      d17, d17, #11
  .long  0xf2e52532                          // vshl.s32      d18, d18, #5
  .long  0xf26101b0                          // vorr          d16, d17, d16
  .long  0xf26001b2                          // vorr          d16, d16, d18
  .long  0x1a000004                          // bne           ab8 <sk_store_565_vfp4+0x6c>
  .long  0xf3f60121                          // vuzp.16       d16, d17
  .long  0xf4cc080f                          // vst1.32       {d16[0]}, [ip]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe12fff1c                          // bx            ip
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xee10eb90                          // vmov.32       lr, d16[0]
  .long  0xe1cce0b0                          // strh          lr, [ip]
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xeafffff7                          // b             aac <sk_store_565_vfp4+0x60>

.globl _sk_load_8888_vfp4
_sk_load_8888_vfp4:
  .long  0xe92d4830                          // push          {r4, r5, fp, lr}
  .long  0xe24dd008                          // sub           sp, sp, #8
  .long  0xe5914000                          // ldr           r4, [r1]
  .long  0xe3530000                          // cmp           r3, #0
  .long  0xe5944000                          // ldr           r4, [r4]
  .long  0xe084c100                          // add           ip, r4, r0, lsl #2
  .long  0x1a000016                          // bne           b44 <sk_load_8888_vfp4+0x78>
  .long  0xeddc0b00                          // vldr          d16, [ip]
  .long  0xe2825010                          // add           r5, r2, #16
  .long  0xf3f81030                          // vshr.u32      d17, d16, #8
  .long  0xf3f04030                          // vshr.u32      d20, d16, #16
  .long  0xed922a03                          // vldr          s4, [r2, #12]
  .long  0xf4e53c9f                          // vld1.32       {d19[]}, [r5 :32]
  .long  0xf3e82030                          // vshr.u32      d18, d16, #24
  .long  0xf24301b0                          // vand          d16, d19, d16
  .long  0xf24311b1                          // vand          d17, d19, d17
  .long  0xf24331b4                          // vand          d19, d19, d20
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf3fb2622                          // vcvt.f32.s32  d18, d18
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf3fb0620                          // vcvt.f32.s32  d16, d16
  .long  0xf3fb1621                          // vcvt.f32.s32  d17, d17
  .long  0xf3fb3623                          // vcvt.f32.s32  d19, d19
  .long  0xf2a239c2                          // vmul.f32      d3, d18, d2[0]
  .long  0xf2a009c2                          // vmul.f32      d0, d16, d2[0]
  .long  0xf2a119c2                          // vmul.f32      d1, d17, d2[0]
  .long  0xf2a329c2                          // vmul.f32      d2, d19, d2[0]
  .long  0xe28dd008                          // add           sp, sp, #8
  .long  0xe8bd4830                          // pop           {r4, r5, fp, lr}
  .long  0xe12fff1c                          // bx            ip
  .long  0xf2c00010                          // vmov.i32      d16, #0
  .long  0xe3a0e000                          // mov           lr, #0
  .long  0xe1a0400d                          // mov           r4, sp
  .long  0xedcd0b00                          // vstr          d16, [sp]
  .long  0xe79c510e                          // ldr           r5, [ip, lr, lsl #2]
  .long  0xe784510e                          // str           r5, [r4, lr, lsl #2]
  .long  0xe28ee001                          // add           lr, lr, #1
  .long  0xe153000e                          // cmp           r3, lr
  .long  0x1afffffa                          // bne           b54 <sk_load_8888_vfp4+0x88>
  .long  0xeddd0b00                          // vldr          d16, [sp]
  .long  0xeaffffde                          // b             aec <sk_load_8888_vfp4+0x20>

.globl _sk_store_8888_vfp4
_sk_store_8888_vfp4:
  .long  0xe282c008                          // add           ip, r2, #8
  .long  0xf2c3261f                          // vmov.i32      d18, #1056964608
  .long  0xf2c3361f                          // vmov.i32      d19, #1056964608
  .long  0xe3530000                          // cmp           r3, #0
  .long  0xf4ec1c9f                          // vld1.32       {d17[]}, [ip :32]
  .long  0xf2c3061f                          // vmov.i32      d16, #1056964608
  .long  0xf2412c31                          // vfma.f32      d18, d1, d17
  .long  0xf2423c31                          // vfma.f32      d19, d2, d17
  .long  0xf2c3461f                          // vmov.i32      d20, #1056964608
  .long  0xe591c000                          // ldr           ip, [r1]
  .long  0xf2400c31                          // vfma.f32      d16, d0, d17
  .long  0xf2434c31                          // vfma.f32      d20, d3, d17
  .long  0xe59cc000                          // ldr           ip, [ip]
  .long  0xe08cc100                          // add           ip, ip, r0, lsl #2
  .long  0xf3fb17a2                          // vcvt.u32.f32  d17, d18
  .long  0xf3fb27a3                          // vcvt.u32.f32  d18, d19
  .long  0xf3fb07a0                          // vcvt.u32.f32  d16, d16
  .long  0xf3fb37a4                          // vcvt.u32.f32  d19, d20
  .long  0xf2e81531                          // vshl.s32      d17, d17, #8
  .long  0xf2f02532                          // vshl.s32      d18, d18, #16
  .long  0xf26101b0                          // vorr          d16, d17, d16
  .long  0xf2f81533                          // vshl.s32      d17, d19, #24
  .long  0xf26001b2                          // vorr          d16, d16, d18
  .long  0xf26001b1                          // vorr          d16, d16, d17
  .long  0x1a000003                          // bne           be4 <sk_store_8888_vfp4+0x74>
  .long  0xedcc0b00                          // vstr          d16, [ip]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe12fff1c                          // bx            ip
  .long  0xf4cc083f                          // vst1.32       {d16[0]}, [ip :32]
  .long  0xeafffffa                          // b             bd8 <sk_store_8888_vfp4+0x68>

.globl _sk_load_f16_vfp4
_sk_load_f16_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe59ee000                          // ldr           lr, [lr]
  .long  0xe08ee180                          // add           lr, lr, r0, lsl #3
  .long  0xf46e084f                          // vld2.16       {d16-d17}, [lr]
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
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_store_f16_vfp4
_sk_store_f16_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xeef00b41                          // vmov.f64      d16, d1
  .long  0xf2631113                          // vorr          d17, d3, d3
  .long  0xeef02b40                          // vmov.f64      d18, d0
  .long  0xf2623112                          // vorr          d19, d2, d2
  .long  0xf3fa00a1                          // vtrn.32       d16, d17
  .long  0xf3f61620                          // vcvt.f16.f32  d17, q8
  .long  0xf3fa20a3                          // vtrn.32       d18, d19
  .long  0xe591c000                          // ldr           ip, [r1]
  .long  0xf3f60622                          // vcvt.f16.f32  d16, q9
  .long  0xe59cc000                          // ldr           ip, [ip]
  .long  0xe08cc180                          // add           ip, ip, r0, lsl #3
  .long  0xf44c084f                          // vst2.16       {d16-d17}, [ip]
  .long  0xe281c008                          // add           ip, r1, #8
  .long  0xe591e004                          // ldr           lr, [r1, #4]
  .long  0xe1a0100c                          // mov           r1, ip
  .long  0xe1a0c00e                          // mov           ip, lr
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_clamp_x_vfp4
_sk_clamp_x_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xf2c00010                          // vmov.i32      d16, #0
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xf3c71e1f                          // vmov.i8       d17, #255
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2400f80                          // vmax.f32      d16, d16, d0
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf4ee2c9f                          // vld1.32       {d18[]}, [lr :32]
  .long  0xf26218a1                          // vadd.i32      d17, d18, d17
  .long  0xf2200fa1                          // vmin.f32      d0, d16, d17
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_clamp_y_vfp4
_sk_clamp_y_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xf2c00010                          // vmov.i32      d16, #0
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xf3c71e1f                          // vmov.i8       d17, #255
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xf2400f81                          // vmax.f32      d16, d16, d1
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf4ee2c9f                          // vld1.32       {d18[]}, [lr :32]
  .long  0xf26218a1                          // vadd.i32      d17, d18, d17
  .long  0xf2201fa1                          // vmin.f32      d1, d16, d17
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_repeat_x_vfp4
_sk_repeat_x_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xf2c02010                          // vmov.i32      d18, #0
  .long  0xf4e23c9f                          // vld1.32       {d19[]}, [r2 :32]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xed9e8a00                          // vldr          s16, [lr]
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
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_repeat_y_vfp4
_sk_repeat_y_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xf2c02010                          // vmov.i32      d18, #0
  .long  0xf4e23c9f                          // vld1.32       {d19[]}, [r2 :32]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xed9e8a00                          // vldr          s16, [lr]
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
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_mirror_x_vfp4
_sk_mirror_x_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xf2c03010                          // vmov.i32      d19, #0
  .long  0xf4e24c9f                          // vld1.32       {d20[]}, [r2 :32]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xed9e8a00                          // vldr          s16, [lr]
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
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_mirror_y_vfp4
_sk_mirror_y_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xed2d8b04                          // vpush         {d8-d9}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xf2c03010                          // vmov.i32      d19, #0
  .long  0xf4e24c9f                          // vld1.32       {d20[]}, [r2 :32]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xed9e8a00                          // vldr          s16, [lr]
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
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_matrix_2x3_vfp4
_sk_matrix_2x3_vfp4:
  .long  0xe92d4010                          // push          {r4, lr}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe28e400c                          // add           r4, lr, #12
  .long  0xf4e42c9f                          // vld1.32       {d18[]}, [r4 :32]
  .long  0xe28e4008                          // add           r4, lr, #8
  .long  0xf4e41c9f                          // vld1.32       {d17[]}, [r4 :32]
  .long  0xe28e4010                          // add           r4, lr, #16
  .long  0xf4e40c9f                          // vld1.32       {d16[]}, [r4 :32]
  .long  0xe28e4014                          // add           r4, lr, #20
  .long  0xf2410c31                          // vfma.f32      d16, d1, d17
  .long  0xf4e41c9f                          // vld1.32       {d17[]}, [r4 :32]
  .long  0xe28e4004                          // add           r4, lr, #4
  .long  0xf2411c32                          // vfma.f32      d17, d1, d18
  .long  0xf4ee2c9f                          // vld1.32       {d18[]}, [lr :32]
  .long  0xf4e43c9f                          // vld1.32       {d19[]}, [r4 :32]
  .long  0xf2400c32                          // vfma.f32      d16, d0, d18
  .long  0xf2401c33                          // vfma.f32      d17, d0, d19
  .long  0xf22001b0                          // vorr          d0, d16, d16
  .long  0xf22111b1                          // vorr          d1, d17, d17
  .long  0xe8bd4010                          // pop           {r4, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_matrix_3x4_vfp4
_sk_matrix_3x4_vfp4:
  .long  0xe92d4010                          // push          {r4, lr}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe28e4020                          // add           r4, lr, #32
  .long  0xf4e43c9f                          // vld1.32       {d19[]}, [r4 :32]
  .long  0xe28e402c                          // add           r4, lr, #44
  .long  0xf4e40c9f                          // vld1.32       {d16[]}, [r4 :32]
  .long  0xe28e401c                          // add           r4, lr, #28
  .long  0xf2420c33                          // vfma.f32      d16, d2, d19
  .long  0xf4e44c9f                          // vld1.32       {d20[]}, [r4 :32]
  .long  0xe28e4018                          // add           r4, lr, #24
  .long  0xf4e42c9f                          // vld1.32       {d18[]}, [r4 :32]
  .long  0xe28e4024                          // add           r4, lr, #36
  .long  0xf4e41c9f                          // vld1.32       {d17[]}, [r4 :32]
  .long  0xe28e4028                          // add           r4, lr, #40
  .long  0xf2421c32                          // vfma.f32      d17, d2, d18
  .long  0xf4e42c9f                          // vld1.32       {d18[]}, [r4 :32]
  .long  0xe28e4010                          // add           r4, lr, #16
  .long  0xf2422c34                          // vfma.f32      d18, d2, d20
  .long  0xf4e43c9f                          // vld1.32       {d19[]}, [r4 :32]
  .long  0xe28e400c                          // add           r4, lr, #12
  .long  0xf4e44c9f                          // vld1.32       {d20[]}, [r4 :32]
  .long  0xe28e4014                          // add           r4, lr, #20
  .long  0xf2411c34                          // vfma.f32      d17, d1, d20
  .long  0xf4e44c9f                          // vld1.32       {d20[]}, [r4 :32]
  .long  0xf2410c34                          // vfma.f32      d16, d1, d20
  .long  0xe28e4004                          // add           r4, lr, #4
  .long  0xf2412c33                          // vfma.f32      d18, d1, d19
  .long  0xf4ee3c9f                          // vld1.32       {d19[]}, [lr :32]
  .long  0xf4e44c9f                          // vld1.32       {d20[]}, [r4 :32]
  .long  0xe28e4008                          // add           r4, lr, #8
  .long  0xf2401c33                          // vfma.f32      d17, d0, d19
  .long  0xf4e43c9f                          // vld1.32       {d19[]}, [r4 :32]
  .long  0xf2400c33                          // vfma.f32      d16, d0, d19
  .long  0xf2402c34                          // vfma.f32      d18, d0, d20
  .long  0xf22101b1                          // vorr          d0, d17, d17
  .long  0xf22021b0                          // vorr          d2, d16, d16
  .long  0xf22211b2                          // vorr          d1, d18, d18
  .long  0xe8bd4010                          // pop           {r4, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_matrix_perspective_vfp4
_sk_matrix_perspective_vfp4:
  .long  0xe92d4010                          // push          {r4, lr}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xe28e401c                          // add           r4, lr, #28
  .long  0xf4e40c9f                          // vld1.32       {d16[]}, [r4 :32]
  .long  0xe28e4020                          // add           r4, lr, #32
  .long  0xf4e41c9f                          // vld1.32       {d17[]}, [r4 :32]
  .long  0xe28e4018                          // add           r4, lr, #24
  .long  0xf2411c30                          // vfma.f32      d17, d1, d16
  .long  0xf4e40c9f                          // vld1.32       {d16[]}, [r4 :32]
  .long  0xe28e4010                          // add           r4, lr, #16
  .long  0xf2401c30                          // vfma.f32      d17, d0, d16
  .long  0xf4e40c9f                          // vld1.32       {d16[]}, [r4 :32]
  .long  0xe28e4004                          // add           r4, lr, #4
  .long  0xf4e42c9f                          // vld1.32       {d18[]}, [r4 :32]
  .long  0xe28e4008                          // add           r4, lr, #8
  .long  0xf4e44c9f                          // vld1.32       {d20[]}, [r4 :32]
  .long  0xe28e4014                          // add           r4, lr, #20
  .long  0xf2414c32                          // vfma.f32      d20, d1, d18
  .long  0xf4e42c9f                          // vld1.32       {d18[]}, [r4 :32]
  .long  0xe28e400c                          // add           r4, lr, #12
  .long  0xf3fb3521                          // vrecpe.f32    d19, d17
  .long  0xf2412c30                          // vfma.f32      d18, d1, d16
  .long  0xf4e45c9f                          // vld1.32       {d21[]}, [r4 :32]
  .long  0xf2410fb3                          // vrecps.f32    d16, d17, d19
  .long  0xf4ee1c9f                          // vld1.32       {d17[]}, [lr :32]
  .long  0xf2404c31                          // vfma.f32      d20, d0, d17
  .long  0xf2402c35                          // vfma.f32      d18, d0, d21
  .long  0xf3430db0                          // vmul.f32      d16, d19, d16
  .long  0xf3040db0                          // vmul.f32      d0, d20, d16
  .long  0xf3021db0                          // vmul.f32      d1, d18, d16
  .long  0xe8bd4010                          // pop           {r4, lr}
  .long  0xe12fff1c                          // bx            ip

.globl _sk_linear_gradient_2stops_vfp4
_sk_linear_gradient_2stops_vfp4:
  .long  0xe92d4800                          // push          {fp, lr}
  .long  0xe591e000                          // ldr           lr, [r1]
  .long  0xe591c004                          // ldr           ip, [r1, #4]
  .long  0xe2811008                          // add           r1, r1, #8
  .long  0xf46e2a0d                          // vld1.8        {d18-d19}, [lr]!
  .long  0xf46e4a0f                          // vld1.8        {d20-d21}, [lr]
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
  .long  0xe8bd4800                          // pop           {fp, lr}
  .long  0xe12fff1c                          // bx            ip
#elif defined(__x86_64__)

.globl _sk_start_pipeline_hsw
_sk_start_pipeline_hsw:
  .byte  65,87                               // push          %r15
  .byte  65,86                               // push          %r14
  .byte  65,85                               // push          %r13
  .byte  65,84                               // push          %r12
  .byte  83                                  // push          %rbx
  .byte  73,137,204                          // mov           %rcx,%r12
  .byte  73,137,214                          // mov           %rdx,%r14
  .byte  72,137,251                          // mov           %rdi,%rbx
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  73,137,247                          // mov           %rsi,%r15
  .byte  72,141,75,8                         // lea           0x8(%rbx),%rcx
  .byte  76,57,225                           // cmp           %r12,%rcx
  .byte  118,5                               // jbe           25 <_sk_start_pipeline_hsw+0x25>
  .byte  72,137,223                          // mov           %rbx,%rdi
  .byte  235,71                              // jmp           6c <_sk_start_pipeline_hsw+0x6c>
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
  .byte  76,137,254                          // mov           %r15,%rsi
  .byte  76,137,242                          // mov           %r14,%rdx
  .byte  73,137,197                          // mov           %rax,%r13
  .byte  65,255,213                          // callq         *%r13
  .byte  76,137,232                          // mov           %r13,%rax
  .byte  72,141,123,8                        // lea           0x8(%rbx),%rdi
  .byte  72,131,195,16                       // add           $0x10,%rbx
  .byte  76,57,227                           // cmp           %r12,%rbx
  .byte  72,137,251                          // mov           %rdi,%rbx
  .byte  118,185                             // jbe           25 <_sk_start_pipeline_hsw+0x25>
  .byte  76,57,231                           // cmp           %r12,%rdi
  .byte  117,13                              // jne           7e <_sk_start_pipeline_hsw+0x7e>
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,93                               // pop           %r13
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  197,248,119                         // vzeroupper
  .byte  195                                 // retq
  .byte  73,41,252                           // sub           %rdi,%r12
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  197,228,87,219                      // vxorps        %ymm3,%ymm3,%ymm3
  .byte  197,220,87,228                      // vxorps        %ymm4,%ymm4,%ymm4
  .byte  197,212,87,237                      // vxorps        %ymm5,%ymm5,%ymm5
  .byte  197,204,87,246                      // vxorps        %ymm6,%ymm6,%ymm6
  .byte  197,196,87,255                      // vxorps        %ymm7,%ymm7,%ymm7
  .byte  76,137,254                          // mov           %r15,%rsi
  .byte  76,137,242                          // mov           %r14,%rdx
  .byte  76,137,225                          // mov           %r12,%rcx
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,93                               // pop           %r13
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  255,224                             // jmpq          *%rax

.globl _sk_just_return_hsw
_sk_just_return_hsw:
  .byte  195                                 // retq

.globl _sk_seed_shader_hsw
_sk_seed_shader_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,249,110,199                     // vmovd         %edi,%xmm0
  .byte  196,226,125,24,192                  // vbroadcastss  %xmm0,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  196,226,125,24,74,4                 // vbroadcastss  0x4(%rdx),%ymm1
  .byte  197,252,88,193                      // vaddps        %ymm1,%ymm0,%ymm0
  .byte  197,252,88,66,20                    // vaddps        0x14(%rdx),%ymm0,%ymm0
  .byte  196,226,125,24,16                   // vbroadcastss  (%rax),%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  197,236,88,201                      // vaddps        %ymm1,%ymm2,%ymm1
  .byte  196,226,125,24,18                   // vbroadcastss  (%rdx),%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
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
  .byte  196,98,125,24,2                     // vbroadcastss  (%rdx),%ymm8
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
  .byte  196,98,125,24,2                     // vbroadcastss  (%rdx),%ymm8
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
  .byte  196,98,125,24,2                     // vbroadcastss  (%rdx),%ymm8
  .byte  196,193,124,93,192                  // vminps        %ymm8,%ymm0,%ymm0
  .byte  196,193,116,93,200                  // vminps        %ymm8,%ymm1,%ymm1
  .byte  196,193,108,93,208                  // vminps        %ymm8,%ymm2,%ymm2
  .byte  196,193,100,93,216                  // vminps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_clamp_a_hsw
_sk_clamp_a_hsw:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,2                     // vbroadcastss  (%rdx),%ymm8
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
  .byte  196,98,125,24,18                    // vbroadcastss  (%rdx),%ymm10
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
  .byte  196,98,125,24,66,64                 // vbroadcastss  0x40(%rdx),%ymm8
  .byte  197,60,89,200                       // vmulps        %ymm0,%ymm8,%ymm9
  .byte  197,124,89,208                      // vmulps        %ymm0,%ymm0,%ymm10
  .byte  196,98,125,24,90,60                 // vbroadcastss  0x3c(%rdx),%ymm11
  .byte  196,98,125,24,98,56                 // vbroadcastss  0x38(%rdx),%ymm12
  .byte  196,65,124,40,235                   // vmovaps       %ymm11,%ymm13
  .byte  196,66,125,168,236                  // vfmadd213ps   %ymm12,%ymm0,%ymm13
  .byte  196,98,125,24,114,52                // vbroadcastss  0x34(%rdx),%ymm14
  .byte  196,66,45,168,238                   // vfmadd213ps   %ymm14,%ymm10,%ymm13
  .byte  196,98,125,24,82,68                 // vbroadcastss  0x44(%rdx),%ymm10
  .byte  196,193,124,194,194,1               // vcmpltps      %ymm10,%ymm0,%ymm0
  .byte  196,195,21,74,193,0                 // vblendvps     %ymm0,%ymm9,%ymm13,%ymm0
  .byte  197,60,89,201                       // vmulps        %ymm1,%ymm8,%ymm9
  .byte  197,116,89,233                      // vmulps        %ymm1,%ymm1,%ymm13
  .byte  196,65,124,40,251                   // vmovaps       %ymm11,%ymm15
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

.globl _sk_to_srgb_hsw
_sk_to_srgb_hsw:
  .byte  197,124,82,192                      // vrsqrtps      %ymm0,%ymm8
  .byte  196,65,124,83,200                   // vrcpps        %ymm8,%ymm9
  .byte  196,65,124,82,208                   // vrsqrtps      %ymm8,%ymm10
  .byte  196,98,125,24,66,72                 // vbroadcastss  0x48(%rdx),%ymm8
  .byte  197,60,89,216                       // vmulps        %ymm0,%ymm8,%ymm11
  .byte  196,98,125,24,34                    // vbroadcastss  (%rdx),%ymm12
  .byte  196,98,125,24,106,76                // vbroadcastss  0x4c(%rdx),%ymm13
  .byte  196,98,125,24,114,80                // vbroadcastss  0x50(%rdx),%ymm14
  .byte  196,98,125,24,122,84                // vbroadcastss  0x54(%rdx),%ymm15
  .byte  196,66,13,168,207                   // vfmadd213ps   %ymm15,%ymm14,%ymm9
  .byte  196,66,21,184,202                   // vfmadd231ps   %ymm10,%ymm13,%ymm9
  .byte  196,65,28,93,201                    // vminps        %ymm9,%ymm12,%ymm9
  .byte  196,98,125,24,82,88                 // vbroadcastss  0x58(%rdx),%ymm10
  .byte  196,193,124,194,194,1               // vcmpltps      %ymm10,%ymm0,%ymm0
  .byte  196,195,53,74,195,0                 // vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
  .byte  197,124,82,201                      // vrsqrtps      %ymm1,%ymm9
  .byte  196,65,124,83,217                   // vrcpps        %ymm9,%ymm11
  .byte  196,65,124,82,201                   // vrsqrtps      %ymm9,%ymm9
  .byte  196,66,13,168,223                   // vfmadd213ps   %ymm15,%ymm14,%ymm11
  .byte  196,66,21,184,217                   // vfmadd231ps   %ymm9,%ymm13,%ymm11
  .byte  197,60,89,201                       // vmulps        %ymm1,%ymm8,%ymm9
  .byte  196,65,28,93,219                    // vminps        %ymm11,%ymm12,%ymm11
  .byte  196,193,116,194,202,1               // vcmpltps      %ymm10,%ymm1,%ymm1
  .byte  196,195,37,74,201,16                // vblendvps     %ymm1,%ymm9,%ymm11,%ymm1
  .byte  197,124,82,202                      // vrsqrtps      %ymm2,%ymm9
  .byte  196,65,124,83,217                   // vrcpps        %ymm9,%ymm11
  .byte  196,66,13,168,223                   // vfmadd213ps   %ymm15,%ymm14,%ymm11
  .byte  196,65,124,82,201                   // vrsqrtps      %ymm9,%ymm9
  .byte  196,66,21,184,217                   // vfmadd231ps   %ymm9,%ymm13,%ymm11
  .byte  196,65,28,93,203                    // vminps        %ymm11,%ymm12,%ymm9
  .byte  197,60,89,194                       // vmulps        %ymm2,%ymm8,%ymm8
  .byte  196,193,108,194,210,1               // vcmpltps      %ymm10,%ymm2,%ymm2
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
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,57                              // jne           447 <_sk_scale_u8_hsw+0x44>
  .byte  196,66,121,48,4,56                  // vpmovzxbw     (%r8,%rdi,1),%xmm8
  .byte  197,57,219,5,212,9,0,0              // vpand         0x9d4(%rip),%xmm8,%xmm8        # df0 <_sk_linear_gradient_2stops_hsw+0x53>
  .byte  196,66,125,51,192                   // vpmovzxwd     %xmm8,%ymm8
  .byte  196,65,124,91,192                   // vcvtdq2ps     %ymm8,%ymm8
  .byte  196,98,125,24,74,12                 // vbroadcastss  0xc(%rdx),%ymm9
  .byte  196,65,60,89,193                    // vmulps        %ymm9,%ymm8,%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  196,65,56,87,192                    // vxorps        %xmm8,%xmm8,%xmm8
  .byte  197,120,19,4,36                     // vmovlps       %xmm8,(%rsp)
  .byte  73,1,248                            // add           %rdi,%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  67,138,4,8                          // mov           (%r8,%r9,1),%al
  .byte  66,136,4,12                         // mov           %al,(%rsp,%r9,1)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,240                             // jne           457 <_sk_scale_u8_hsw+0x54>
  .byte  197,122,126,4,36                    // vmovq         (%rsp),%xmm8
  .byte  196,66,121,48,192                   // vpmovzxbw     %xmm8,%xmm8
  .byte  235,161                             // jmp           414 <_sk_scale_u8_hsw+0x11>

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
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,77                              // jne           4fa <_sk_lerp_u8_hsw+0x58>
  .byte  196,66,121,48,4,56                  // vpmovzxbw     (%r8,%rdi,1),%xmm8
  .byte  197,57,219,5,69,9,0,0               // vpand         0x945(%rip),%xmm8,%xmm8        # e00 <_sk_linear_gradient_2stops_hsw+0x63>
  .byte  196,66,125,51,192                   // vpmovzxwd     %xmm8,%ymm8
  .byte  196,65,124,91,192                   // vcvtdq2ps     %ymm8,%ymm8
  .byte  196,98,125,24,74,12                 // vbroadcastss  0xc(%rdx),%ymm9
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
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  196,65,56,87,192                    // vxorps        %xmm8,%xmm8,%xmm8
  .byte  197,120,19,4,36                     // vmovlps       %xmm8,(%rsp)
  .byte  73,1,248                            // add           %rdi,%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  67,138,4,8                          // mov           (%r8,%r9,1),%al
  .byte  66,136,4,12                         // mov           %al,(%rsp,%r9,1)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,240                             // jne           50a <_sk_lerp_u8_hsw+0x68>
  .byte  197,122,126,4,36                    // vmovq         (%rsp),%xmm8
  .byte  196,66,121,48,192                   // vpmovzxbw     %xmm8,%xmm8
  .byte  235,141                             // jmp           4b3 <_sk_lerp_u8_hsw+0x11>

.globl _sk_lerp_565_hsw
_sk_lerp_565_hsw:
  .byte  72,131,236,24                       // sub           $0x18,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,126                             // jne           5b2 <_sk_lerp_565_hsw+0x8c>
  .byte  197,250,111,28,120                  // vmovdqu       (%rax,%rdi,2),%xmm3
  .byte  196,226,125,51,219                  // vpmovzxwd     %xmm3,%ymm3
  .byte  196,98,125,88,66,104                // vpbroadcastd  0x68(%rdx),%ymm8
  .byte  197,61,219,195                      // vpand         %ymm3,%ymm8,%ymm8
  .byte  196,65,124,91,192                   // vcvtdq2ps     %ymm8,%ymm8
  .byte  196,98,125,24,74,116                // vbroadcastss  0x74(%rdx),%ymm9
  .byte  196,65,52,89,192                    // vmulps        %ymm8,%ymm9,%ymm8
  .byte  196,98,125,88,74,108                // vpbroadcastd  0x6c(%rdx),%ymm9
  .byte  197,53,219,203                      // vpand         %ymm3,%ymm9,%ymm9
  .byte  196,65,124,91,201                   // vcvtdq2ps     %ymm9,%ymm9
  .byte  196,98,125,24,82,120                // vbroadcastss  0x78(%rdx),%ymm10
  .byte  196,65,44,89,201                    // vmulps        %ymm9,%ymm10,%ymm9
  .byte  196,98,125,88,82,112                // vpbroadcastd  0x70(%rdx),%ymm10
  .byte  197,173,219,219                     // vpand         %ymm3,%ymm10,%ymm3
  .byte  197,252,91,219                      // vcvtdq2ps     %ymm3,%ymm3
  .byte  196,98,125,24,82,124                // vbroadcastss  0x7c(%rdx),%ymm10
  .byte  197,172,89,219                      // vmulps        %ymm3,%ymm10,%ymm3
  .byte  197,252,92,196                      // vsubps        %ymm4,%ymm0,%ymm0
  .byte  196,226,61,168,196                  // vfmadd213ps   %ymm4,%ymm8,%ymm0
  .byte  197,244,92,205                      // vsubps        %ymm5,%ymm1,%ymm1
  .byte  196,226,53,168,205                  // vfmadd213ps   %ymm5,%ymm9,%ymm1
  .byte  197,236,92,214                      // vsubps        %ymm6,%ymm2,%ymm2
  .byte  196,226,101,168,214                 // vfmadd213ps   %ymm6,%ymm3,%ymm2
  .byte  196,226,125,24,26                   // vbroadcastss  (%rdx),%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,131,196,24                       // add           $0x18,%rsp
  .byte  255,224                             // jmpq          *%rax
  .byte  197,224,87,219                      // vxorps        %xmm3,%xmm3,%xmm3
  .byte  197,248,41,28,36                    // vmovaps       %xmm3,(%rsp)
  .byte  76,141,4,120                        // lea           (%rax,%rdi,2),%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  102,67,139,4,72                     // mov           (%r8,%r9,2),%ax
  .byte  102,66,137,4,76                     // mov           %ax,(%rsp,%r9,2)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,238                             // jne           5c2 <_sk_lerp_565_hsw+0x9c>
  .byte  197,249,111,28,36                   // vmovdqa       (%rsp),%xmm3
  .byte  233,91,255,255,255                  // jmpq          539 <_sk_lerp_565_hsw+0x13>

.globl _sk_load_tables_hsw
_sk_load_tables_hsw:
  .byte  85                                  // push          %rbp
  .byte  72,137,229                          // mov           %rsp,%rbp
  .byte  72,131,228,224                      // and           $0xffffffffffffffe0,%rsp
  .byte  72,131,236,64                       // sub           $0x40,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,116                             // jne           668 <_sk_load_tables_hsw+0x8a>
  .byte  196,193,124,16,28,184               // vmovups       (%r8,%rdi,4),%ymm3
  .byte  196,226,125,24,82,16                // vbroadcastss  0x10(%rdx),%ymm2
  .byte  197,236,84,203                      // vandps        %ymm3,%ymm2,%ymm1
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,124,194,192,0                   // vcmpeqps      %ymm0,%ymm0,%ymm8
  .byte  76,139,64,8                         // mov           0x8(%rax),%r8
  .byte  76,139,72,16                        // mov           0x10(%rax),%r9
  .byte  196,65,124,40,200                   // vmovaps       %ymm8,%ymm9
  .byte  196,194,53,146,4,136                // vgatherdps    %ymm9,(%r8,%ymm1,4),%ymm0
  .byte  197,245,114,211,8                   // vpsrld        $0x8,%ymm3,%ymm1
  .byte  197,108,84,201                      // vandps        %ymm1,%ymm2,%ymm9
  .byte  196,65,124,40,208                   // vmovaps       %ymm8,%ymm10
  .byte  196,130,45,146,12,137               // vgatherdps    %ymm10,(%r9,%ymm9,4),%ymm1
  .byte  72,139,64,24                        // mov           0x18(%rax),%rax
  .byte  197,181,114,211,16                  // vpsrld        $0x10,%ymm3,%ymm9
  .byte  196,65,108,84,201                   // vandps        %ymm9,%ymm2,%ymm9
  .byte  196,162,61,146,20,136               // vgatherdps    %ymm8,(%rax,%ymm9,4),%ymm2
  .byte  197,229,114,211,24                  // vpsrld        $0x18,%ymm3,%ymm3
  .byte  197,252,91,219                      // vcvtdq2ps     %ymm3,%ymm3
  .byte  196,98,125,24,66,12                 // vbroadcastss  0xc(%rdx),%ymm8
  .byte  196,193,100,89,216                  // vmulps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,208                             // callq         *%rax
  .byte  72,137,236                          // mov           %rbp,%rsp
  .byte  93                                  // pop           %rbp
  .byte  197,248,119                         // vzeroupper
  .byte  195                                 // retq
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,252,41,4,36                     // vmovaps       %ymm0,(%rsp)
  .byte  77,141,4,184                        // lea           (%r8,%rdi,4),%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  71,139,20,136                       // mov           (%r8,%r9,4),%r10d
  .byte  70,137,20,140                       // mov           %r10d,(%rsp,%r9,4)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,240                             // jne           678 <_sk_load_tables_hsw+0x9a>
  .byte  197,252,40,28,36                    // vmovaps       (%rsp),%ymm3
  .byte  233,104,255,255,255                 // jmpq          5fa <_sk_load_tables_hsw+0x1c>

.globl _sk_load_a8_hsw
_sk_load_a8_hsw:
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,51                              // jne           6d0 <_sk_load_a8_hsw+0x3e>
  .byte  196,194,121,48,4,56                 // vpmovzxbw     (%r8,%rdi,1),%xmm0
  .byte  197,249,219,5,101,7,0,0             // vpand         0x765(%rip),%xmm0,%xmm0        # e10 <_sk_linear_gradient_2stops_hsw+0x73>
  .byte  196,226,125,51,192                  // vpmovzxwd     %xmm0,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  196,226,125,24,74,12                // vbroadcastss  0xc(%rdx),%ymm1
  .byte  197,252,89,217                      // vmulps        %ymm1,%ymm0,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  197,248,87,192                      // vxorps        %xmm0,%xmm0,%xmm0
  .byte  197,248,19,4,36                     // vmovlps       %xmm0,(%rsp)
  .byte  73,1,248                            // add           %rdi,%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  67,138,4,8                          // mov           (%r8,%r9,1),%al
  .byte  66,136,4,12                         // mov           %al,(%rsp,%r9,1)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,240                             // jne           6df <_sk_load_a8_hsw+0x4d>
  .byte  197,250,126,4,36                    // vmovq         (%rsp),%xmm0
  .byte  196,226,121,48,192                  // vpmovzxbw     %xmm0,%xmm0
  .byte  235,168                             // jmp           6a3 <_sk_load_a8_hsw+0x11>

.globl _sk_store_a8_hsw
_sk_store_a8_hsw:
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  196,98,125,24,66,8                  // vbroadcastss  0x8(%rdx),%ymm8
  .byte  197,60,89,195                       // vmulps        %ymm3,%ymm8,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,67,125,25,193,1                 // vextractf128  $0x1,%ymm8,%xmm9
  .byte  196,66,57,43,193                    // vpackusdw     %xmm9,%xmm8,%xmm8
  .byte  196,65,57,103,192                   // vpackuswb     %xmm8,%xmm8,%xmm8
  .byte  197,121,214,4,36                    // vmovq         %xmm8,(%rsp)
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,12                              // jne           736 <_sk_store_a8_hsw+0x3b>
  .byte  196,65,123,17,4,56                  // vmovsd        %xmm8,(%r8,%rdi,1)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  196,65,249,126,193                  // vmovq         %xmm8,%r9
  .byte  69,136,12,56                        // mov           %r9b,(%r8,%rdi,1)
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,235                             // je            730 <_sk_store_a8_hsw+0x35>
  .byte  76,137,200                          // mov           %r9,%rax
  .byte  72,193,232,8                        // shr           $0x8,%rax
  .byte  65,136,68,56,1                      // mov           %al,0x1(%r8,%rdi,1)
  .byte  72,131,249,2                        // cmp           $0x2,%rcx
  .byte  116,217                             // je            730 <_sk_store_a8_hsw+0x35>
  .byte  73,193,233,16                       // shr           $0x10,%r9
  .byte  69,136,76,56,2                      // mov           %r9b,0x2(%r8,%rdi,1)
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  116,202                             // je            730 <_sk_store_a8_hsw+0x35>
  .byte  76,141,84,36,3                      // lea           0x3(%rsp),%r10
  .byte  65,185,3,0,0,0                      // mov           $0x3,%r9d
  .byte  73,41,201                           // sub           %rcx,%r9
  .byte  74,141,68,7,3                       // lea           0x3(%rdi,%r8,1),%rax
  .byte  69,138,2                            // mov           (%r10),%r8b
  .byte  73,255,194                          // inc           %r10
  .byte  68,136,0                            // mov           %r8b,(%rax)
  .byte  72,255,192                          // inc           %rax
  .byte  73,255,193                          // inc           %r9
  .byte  117,239                             // jne           779 <_sk_store_a8_hsw+0x7e>
  .byte  235,164                             // jmp           730 <_sk_store_a8_hsw+0x35>

.globl _sk_load_565_hsw
_sk_load_565_hsw:
  .byte  72,131,236,24                       // sub           $0x18,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,95                              // jne           7f9 <_sk_load_565_hsw+0x6d>
  .byte  197,250,111,4,120                   // vmovdqu       (%rax,%rdi,2),%xmm0
  .byte  196,226,125,51,208                  // vpmovzxwd     %xmm0,%ymm2
  .byte  196,226,125,88,66,104               // vpbroadcastd  0x68(%rdx),%ymm0
  .byte  197,253,219,194                     // vpand         %ymm2,%ymm0,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  196,226,125,24,74,116               // vbroadcastss  0x74(%rdx),%ymm1
  .byte  197,244,89,192                      // vmulps        %ymm0,%ymm1,%ymm0
  .byte  196,226,125,88,74,108               // vpbroadcastd  0x6c(%rdx),%ymm1
  .byte  197,245,219,202                     // vpand         %ymm2,%ymm1,%ymm1
  .byte  197,252,91,201                      // vcvtdq2ps     %ymm1,%ymm1
  .byte  196,226,125,24,90,120               // vbroadcastss  0x78(%rdx),%ymm3
  .byte  197,228,89,201                      // vmulps        %ymm1,%ymm3,%ymm1
  .byte  196,226,125,88,90,112               // vpbroadcastd  0x70(%rdx),%ymm3
  .byte  197,229,219,210                     // vpand         %ymm2,%ymm3,%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  196,226,125,24,90,124               // vbroadcastss  0x7c(%rdx),%ymm3
  .byte  197,228,89,210                      // vmulps        %ymm2,%ymm3,%ymm2
  .byte  196,226,125,24,26                   // vbroadcastss  (%rdx),%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,131,196,24                       // add           $0x18,%rsp
  .byte  255,224                             // jmpq          *%rax
  .byte  197,248,87,192                      // vxorps        %xmm0,%xmm0,%xmm0
  .byte  197,248,41,4,36                     // vmovaps       %xmm0,(%rsp)
  .byte  76,141,4,120                        // lea           (%rax,%rdi,2),%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  102,67,139,4,72                     // mov           (%r8,%r9,2),%ax
  .byte  102,66,137,4,76                     // mov           %ax,(%rsp,%r9,2)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,238                             // jne           809 <_sk_load_565_hsw+0x7d>
  .byte  197,249,111,4,36                    // vmovdqa       (%rsp),%xmm0
  .byte  233,122,255,255,255                 // jmpq          79f <_sk_load_565_hsw+0x13>

.globl _sk_store_565_hsw
_sk_store_565_hsw:
  .byte  72,131,236,24                       // sub           $0x18,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  77,141,12,120                       // lea           (%r8,%rdi,2),%r9
  .byte  196,98,125,24,130,128,0,0,0         // vbroadcastss  0x80(%rdx),%ymm8
  .byte  197,60,89,200                       // vmulps        %ymm0,%ymm8,%ymm9
  .byte  196,65,125,91,201                   // vcvtps2dq     %ymm9,%ymm9
  .byte  196,193,53,114,241,11               // vpslld        $0xb,%ymm9,%ymm9
  .byte  196,98,125,24,146,132,0,0,0         // vbroadcastss  0x84(%rdx),%ymm10
  .byte  197,44,89,209                       // vmulps        %ymm1,%ymm10,%ymm10
  .byte  196,65,125,91,210                   // vcvtps2dq     %ymm10,%ymm10
  .byte  196,193,45,114,242,5                // vpslld        $0x5,%ymm10,%ymm10
  .byte  196,65,45,235,201                   // vpor          %ymm9,%ymm10,%ymm9
  .byte  197,60,89,194                       // vmulps        %ymm2,%ymm8,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,65,53,235,192                   // vpor          %ymm8,%ymm9,%ymm8
  .byte  196,67,125,57,193,1                 // vextracti128  $0x1,%ymm8,%xmm9
  .byte  196,66,57,43,193                    // vpackusdw     %xmm9,%xmm8,%xmm8
  .byte  197,121,127,4,36                    // vmovdqa       %xmm8,(%rsp)
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,18                              // jne           89c <_sk_store_565_hsw+0x77>
  .byte  197,121,111,4,36                    // vmovdqa       (%rsp),%xmm8
  .byte  196,65,122,127,1                    // vmovdqu       %xmm8,(%r9)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,131,196,24                       // add           $0x18,%rsp
  .byte  255,224                             // jmpq          *%rax
  .byte  197,121,126,192                     // vmovd         %xmm8,%eax
  .byte  102,65,137,1                        // mov           %ax,(%r9)
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,234                             // je            894 <_sk_store_565_hsw+0x6f>
  .byte  196,193,121,197,192,1               // vpextrw       $0x1,%xmm8,%eax
  .byte  102,65,137,65,2                     // mov           %ax,0x2(%r9)
  .byte  72,131,249,2                        // cmp           $0x2,%rcx
  .byte  116,217                             // je            894 <_sk_store_565_hsw+0x6f>
  .byte  196,193,121,197,192,2               // vpextrw       $0x2,%xmm8,%eax
  .byte  102,65,137,65,4                     // mov           %ax,0x4(%r9)
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  116,200                             // je            894 <_sk_store_565_hsw+0x6f>
  .byte  76,141,84,36,6                      // lea           0x6(%rsp),%r10
  .byte  65,185,3,0,0,0                      // mov           $0x3,%r9d
  .byte  73,41,201                           // sub           %rcx,%r9
  .byte  73,141,68,120,6                     // lea           0x6(%r8,%rdi,2),%rax
  .byte  102,69,139,2                        // mov           (%r10),%r8w
  .byte  73,131,194,2                        // add           $0x2,%r10
  .byte  102,68,137,0                        // mov           %r8w,(%rax)
  .byte  72,131,192,2                        // add           $0x2,%rax
  .byte  73,255,193                          // inc           %r9
  .byte  117,235                             // jne           8df <_sk_store_565_hsw+0xba>
  .byte  235,158                             // jmp           894 <_sk_store_565_hsw+0x6f>

.globl _sk_load_8888_hsw
_sk_load_8888_hsw:
  .byte  85                                  // push          %rbp
  .byte  72,137,229                          // mov           %rsp,%rbp
  .byte  72,131,228,224                      // and           $0xffffffffffffffe0,%rsp
  .byte  72,131,236,64                       // sub           $0x40,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,90                              // jne           966 <_sk_load_8888_hsw+0x70>
  .byte  197,252,16,28,184                   // vmovups       (%rax,%rdi,4),%ymm3
  .byte  196,226,125,24,82,16                // vbroadcastss  0x10(%rdx),%ymm2
  .byte  197,236,84,195                      // vandps        %ymm3,%ymm2,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  196,98,125,24,66,12                 // vbroadcastss  0xc(%rdx),%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,245,114,211,8                   // vpsrld        $0x8,%ymm3,%ymm1
  .byte  197,236,84,201                      // vandps        %ymm1,%ymm2,%ymm1
  .byte  197,252,91,201                      // vcvtdq2ps     %ymm1,%ymm1
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,181,114,211,16                  // vpsrld        $0x10,%ymm3,%ymm9
  .byte  196,193,108,84,209                  // vandps        %ymm9,%ymm2,%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  197,229,114,211,24                  // vpsrld        $0x18,%ymm3,%ymm3
  .byte  197,252,91,219                      // vcvtdq2ps     %ymm3,%ymm3
  .byte  196,193,100,89,216                  // vmulps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,208                             // callq         *%rax
  .byte  72,137,236                          // mov           %rbp,%rsp
  .byte  93                                  // pop           %rbp
  .byte  197,248,119                         // vzeroupper
  .byte  195                                 // retq
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,252,41,4,36                     // vmovaps       %ymm0,(%rsp)
  .byte  76,141,4,184                        // lea           (%rax,%rdi,4),%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  67,139,4,136                        // mov           (%r8,%r9,4),%eax
  .byte  66,137,4,140                        // mov           %eax,(%rsp,%r9,4)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,240                             // jne           976 <_sk_load_8888_hsw+0x80>
  .byte  197,252,40,28,36                    // vmovaps       (%rsp),%ymm3
  .byte  235,132                             // jmp           911 <_sk_load_8888_hsw+0x1b>

.globl _sk_store_8888_hsw
_sk_store_8888_hsw:
  .byte  85                                  // push          %rbp
  .byte  72,137,229                          // mov           %rsp,%rbp
  .byte  72,131,228,224                      // and           $0xffffffffffffffe0,%rsp
  .byte  72,131,236,64                       // sub           $0x40,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  77,141,12,184                       // lea           (%r8,%rdi,4),%r9
  .byte  196,98,125,24,66,8                  // vbroadcastss  0x8(%rdx),%ymm8
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
  .byte  197,125,127,4,36                    // vmovdqa       %ymm8,(%rsp)
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,22                              // jne           a0d <_sk_store_8888_hsw+0x80>
  .byte  197,125,111,4,36                    // vmovdqa       (%rsp),%ymm8
  .byte  196,65,126,127,1                    // vmovdqu       %ymm8,(%r9)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,208                             // callq         *%rax
  .byte  72,137,236                          // mov           %rbp,%rsp
  .byte  93                                  // pop           %rbp
  .byte  197,248,119                         // vzeroupper
  .byte  195                                 // retq
  .byte  196,65,121,126,1                    // vmovd         %xmm8,(%r9)
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,233                             // je            a01 <_sk_store_8888_hsw+0x74>
  .byte  196,99,121,22,192,1                 // vpextrd       $0x1,%xmm8,%eax
  .byte  65,137,65,4                         // mov           %eax,0x4(%r9)
  .byte  72,131,249,2                        // cmp           $0x2,%rcx
  .byte  116,217                             // je            a01 <_sk_store_8888_hsw+0x74>
  .byte  196,99,121,22,192,2                 // vpextrd       $0x2,%xmm8,%eax
  .byte  65,137,65,8                         // mov           %eax,0x8(%r9)
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  116,201                             // je            a01 <_sk_store_8888_hsw+0x74>
  .byte  76,141,84,36,12                     // lea           0xc(%rsp),%r10
  .byte  65,185,3,0,0,0                      // mov           $0x3,%r9d
  .byte  73,41,201                           // sub           %rcx,%r9
  .byte  73,141,68,184,12                    // lea           0xc(%r8,%rdi,4),%rax
  .byte  69,139,2                            // mov           (%r10),%r8d
  .byte  73,131,194,4                        // add           $0x4,%r10
  .byte  68,137,0                            // mov           %r8d,(%rax)
  .byte  72,131,192,4                        // add           $0x4,%rax
  .byte  73,255,193                          // inc           %r9
  .byte  117,237                             // jne           a4b <_sk_store_8888_hsw+0xbe>
  .byte  235,161                             // jmp           a01 <_sk_store_8888_hsw+0x74>

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

.globl _sk_start_pipeline_avx
_sk_start_pipeline_avx:
  .byte  65,87                               // push          %r15
  .byte  65,86                               // push          %r14
  .byte  65,85                               // push          %r13
  .byte  65,84                               // push          %r12
  .byte  83                                  // push          %rbx
  .byte  73,137,204                          // mov           %rcx,%r12
  .byte  73,137,214                          // mov           %rdx,%r14
  .byte  72,137,251                          // mov           %rdi,%rbx
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  73,137,247                          // mov           %rsi,%r15
  .byte  72,141,75,8                         // lea           0x8(%rbx),%rcx
  .byte  76,57,225                           // cmp           %r12,%rcx
  .byte  118,5                               // jbe           25 <_sk_start_pipeline_avx+0x25>
  .byte  72,137,223                          // mov           %rbx,%rdi
  .byte  235,71                              // jmp           6c <_sk_start_pipeline_avx+0x6c>
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
  .byte  76,137,254                          // mov           %r15,%rsi
  .byte  76,137,242                          // mov           %r14,%rdx
  .byte  73,137,197                          // mov           %rax,%r13
  .byte  65,255,213                          // callq         *%r13
  .byte  76,137,232                          // mov           %r13,%rax
  .byte  72,141,123,8                        // lea           0x8(%rbx),%rdi
  .byte  72,131,195,16                       // add           $0x10,%rbx
  .byte  76,57,227                           // cmp           %r12,%rbx
  .byte  72,137,251                          // mov           %rdi,%rbx
  .byte  118,185                             // jbe           25 <_sk_start_pipeline_avx+0x25>
  .byte  76,57,231                           // cmp           %r12,%rdi
  .byte  117,13                              // jne           7e <_sk_start_pipeline_avx+0x7e>
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,93                               // pop           %r13
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  197,248,119                         // vzeroupper
  .byte  195                                 // retq
  .byte  73,41,252                           // sub           %rdi,%r12
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  197,228,87,219                      // vxorps        %ymm3,%ymm3,%ymm3
  .byte  197,220,87,228                      // vxorps        %ymm4,%ymm4,%ymm4
  .byte  197,212,87,237                      // vxorps        %ymm5,%ymm5,%ymm5
  .byte  197,204,87,246                      // vxorps        %ymm6,%ymm6,%ymm6
  .byte  197,196,87,255                      // vxorps        %ymm7,%ymm7,%ymm7
  .byte  76,137,254                          // mov           %r15,%rsi
  .byte  76,137,242                          // mov           %r14,%rdx
  .byte  76,137,225                          // mov           %r12,%rcx
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,93                               // pop           %r13
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  255,224                             // jmpq          *%rax

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
  .byte  196,226,125,24,74,4                 // vbroadcastss  0x4(%rdx),%ymm1
  .byte  197,252,88,193                      // vaddps        %ymm1,%ymm0,%ymm0
  .byte  197,252,88,66,20                    // vaddps        0x14(%rdx),%ymm0,%ymm0
  .byte  197,249,110,16                      // vmovd         (%rax),%xmm2
  .byte  196,227,121,4,210,0                 // vpermilps     $0x0,%xmm2,%xmm2
  .byte  196,227,109,24,210,1                // vinsertf128   $0x1,%xmm2,%ymm2,%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  197,236,88,201                      // vaddps        %ymm1,%ymm2,%ymm1
  .byte  196,226,125,24,18                   // vbroadcastss  (%rdx),%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
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
  .byte  196,98,125,24,2                     // vbroadcastss  (%rdx),%ymm8
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
  .byte  196,98,125,24,2                     // vbroadcastss  (%rdx),%ymm8
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
  .byte  196,98,125,24,2                     // vbroadcastss  (%rdx),%ymm8
  .byte  196,193,124,93,192                  // vminps        %ymm8,%ymm0,%ymm0
  .byte  196,193,116,93,200                  // vminps        %ymm8,%ymm1,%ymm1
  .byte  196,193,108,93,208                  // vminps        %ymm8,%ymm2,%ymm2
  .byte  196,193,100,93,216                  // vminps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_clamp_a_avx
_sk_clamp_a_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  196,98,125,24,2                     // vbroadcastss  (%rdx),%ymm8
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
  .byte  196,98,125,24,18                    // vbroadcastss  (%rdx),%ymm10
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
  .byte  196,98,125,24,66,64                 // vbroadcastss  0x40(%rdx),%ymm8
  .byte  197,60,89,200                       // vmulps        %ymm0,%ymm8,%ymm9
  .byte  197,124,89,208                      // vmulps        %ymm0,%ymm0,%ymm10
  .byte  196,98,125,24,90,60                 // vbroadcastss  0x3c(%rdx),%ymm11
  .byte  196,98,125,24,98,56                 // vbroadcastss  0x38(%rdx),%ymm12
  .byte  197,36,89,232                       // vmulps        %ymm0,%ymm11,%ymm13
  .byte  196,65,20,88,236                    // vaddps        %ymm12,%ymm13,%ymm13
  .byte  196,98,125,24,114,52                // vbroadcastss  0x34(%rdx),%ymm14
  .byte  196,65,44,89,213                    // vmulps        %ymm13,%ymm10,%ymm10
  .byte  196,65,12,88,210                    // vaddps        %ymm10,%ymm14,%ymm10
  .byte  196,98,125,24,106,68                // vbroadcastss  0x44(%rdx),%ymm13
  .byte  196,193,124,194,197,1               // vcmpltps      %ymm13,%ymm0,%ymm0
  .byte  196,195,45,74,193,0                 // vblendvps     %ymm0,%ymm9,%ymm10,%ymm0
  .byte  197,60,89,201                       // vmulps        %ymm1,%ymm8,%ymm9
  .byte  197,116,89,209                      // vmulps        %ymm1,%ymm1,%ymm10
  .byte  197,36,89,249                       // vmulps        %ymm1,%ymm11,%ymm15
  .byte  196,65,4,88,252                     // vaddps        %ymm12,%ymm15,%ymm15
  .byte  196,65,44,89,215                    // vmulps        %ymm15,%ymm10,%ymm10
  .byte  196,65,12,88,210                    // vaddps        %ymm10,%ymm14,%ymm10
  .byte  196,193,116,194,205,1               // vcmpltps      %ymm13,%ymm1,%ymm1
  .byte  196,195,45,74,201,16                // vblendvps     %ymm1,%ymm9,%ymm10,%ymm1
  .byte  197,60,89,194                       // vmulps        %ymm2,%ymm8,%ymm8
  .byte  197,108,89,202                      // vmulps        %ymm2,%ymm2,%ymm9
  .byte  197,36,89,210                       // vmulps        %ymm2,%ymm11,%ymm10
  .byte  196,65,44,88,212                    // vaddps        %ymm12,%ymm10,%ymm10
  .byte  196,65,52,89,202                    // vmulps        %ymm10,%ymm9,%ymm9
  .byte  196,65,12,88,201                    // vaddps        %ymm9,%ymm14,%ymm9
  .byte  196,193,108,194,213,1               // vcmpltps      %ymm13,%ymm2,%ymm2
  .byte  196,195,53,74,208,32                // vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_to_srgb_avx
_sk_to_srgb_avx:
  .byte  197,124,82,192                      // vrsqrtps      %ymm0,%ymm8
  .byte  196,65,124,83,200                   // vrcpps        %ymm8,%ymm9
  .byte  196,65,124,82,208                   // vrsqrtps      %ymm8,%ymm10
  .byte  196,98,125,24,66,72                 // vbroadcastss  0x48(%rdx),%ymm8
  .byte  197,60,89,216                       // vmulps        %ymm0,%ymm8,%ymm11
  .byte  196,98,125,24,34                    // vbroadcastss  (%rdx),%ymm12
  .byte  196,98,125,24,106,76                // vbroadcastss  0x4c(%rdx),%ymm13
  .byte  196,98,125,24,114,80                // vbroadcastss  0x50(%rdx),%ymm14
  .byte  196,98,125,24,122,84                // vbroadcastss  0x54(%rdx),%ymm15
  .byte  196,65,52,89,206                    // vmulps        %ymm14,%ymm9,%ymm9
  .byte  196,65,52,88,207                    // vaddps        %ymm15,%ymm9,%ymm9
  .byte  196,65,44,89,213                    // vmulps        %ymm13,%ymm10,%ymm10
  .byte  196,65,44,88,201                    // vaddps        %ymm9,%ymm10,%ymm9
  .byte  196,65,28,93,201                    // vminps        %ymm9,%ymm12,%ymm9
  .byte  196,98,125,24,82,88                 // vbroadcastss  0x58(%rdx),%ymm10
  .byte  196,193,124,194,194,1               // vcmpltps      %ymm10,%ymm0,%ymm0
  .byte  196,195,53,74,195,0                 // vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
  .byte  197,124,82,201                      // vrsqrtps      %ymm1,%ymm9
  .byte  196,65,124,83,217                   // vrcpps        %ymm9,%ymm11
  .byte  196,65,124,82,201                   // vrsqrtps      %ymm9,%ymm9
  .byte  196,65,12,89,219                    // vmulps        %ymm11,%ymm14,%ymm11
  .byte  196,65,4,88,219                     // vaddps        %ymm11,%ymm15,%ymm11
  .byte  196,65,20,89,201                    // vmulps        %ymm9,%ymm13,%ymm9
  .byte  196,65,52,88,203                    // vaddps        %ymm11,%ymm9,%ymm9
  .byte  197,60,89,217                       // vmulps        %ymm1,%ymm8,%ymm11
  .byte  196,65,28,93,201                    // vminps        %ymm9,%ymm12,%ymm9
  .byte  196,193,116,194,202,1               // vcmpltps      %ymm10,%ymm1,%ymm1
  .byte  196,195,53,74,203,16                // vblendvps     %ymm1,%ymm11,%ymm9,%ymm1
  .byte  197,124,82,202                      // vrsqrtps      %ymm2,%ymm9
  .byte  196,65,124,83,217                   // vrcpps        %ymm9,%ymm11
  .byte  196,65,12,89,219                    // vmulps        %ymm11,%ymm14,%ymm11
  .byte  196,65,4,88,219                     // vaddps        %ymm11,%ymm15,%ymm11
  .byte  196,65,124,82,201                   // vrsqrtps      %ymm9,%ymm9
  .byte  196,65,20,89,201                    // vmulps        %ymm9,%ymm13,%ymm9
  .byte  196,65,52,88,203                    // vaddps        %ymm11,%ymm9,%ymm9
  .byte  196,65,28,93,201                    // vminps        %ymm9,%ymm12,%ymm9
  .byte  197,60,89,194                       // vmulps        %ymm2,%ymm8,%ymm8
  .byte  196,193,108,194,210,1               // vcmpltps      %ymm10,%ymm2,%ymm2
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
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,73                              // jne           4b0 <_sk_scale_u8_avx+0x54>
  .byte  196,66,121,48,4,56                  // vpmovzxbw     (%r8,%rdi,1),%xmm8
  .byte  197,57,219,5,203,14,0,0             // vpand         0xecb(%rip),%xmm8,%xmm8        # 1340 <_sk_linear_gradient_2stops_avx+0x68>
  .byte  196,65,49,239,201                   // vpxor         %xmm9,%xmm9,%xmm9
  .byte  196,65,57,105,201                   // vpunpckhwd    %xmm9,%xmm8,%xmm9
  .byte  196,66,121,51,192                   // vpmovzxwd     %xmm8,%xmm8
  .byte  196,67,61,24,193,1                  // vinsertf128   $0x1,%xmm9,%ymm8,%ymm8
  .byte  196,65,124,91,192                   // vcvtdq2ps     %ymm8,%ymm8
  .byte  196,98,125,24,74,12                 // vbroadcastss  0xc(%rdx),%ymm9
  .byte  196,65,60,89,193                    // vmulps        %ymm9,%ymm8,%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  196,65,56,87,192                    // vxorps        %xmm8,%xmm8,%xmm8
  .byte  197,120,19,4,36                     // vmovlps       %xmm8,(%rsp)
  .byte  73,1,248                            // add           %rdi,%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  67,138,4,8                          // mov           (%r8,%r9,1),%al
  .byte  66,136,4,12                         // mov           %al,(%rsp,%r9,1)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,240                             // jne           4c0 <_sk_scale_u8_avx+0x64>
  .byte  197,122,126,4,36                    // vmovq         (%rsp),%xmm8
  .byte  196,66,121,48,192                   // vpmovzxbw     %xmm8,%xmm8
  .byte  235,145                             // jmp           46d <_sk_scale_u8_avx+0x11>

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
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,109                             // jne           593 <_sk_lerp_u8_avx+0x78>
  .byte  196,66,121,48,4,56                  // vpmovzxbw     (%r8,%rdi,1),%xmm8
  .byte  197,57,219,5,28,14,0,0              // vpand         0xe1c(%rip),%xmm8,%xmm8        # 1350 <_sk_linear_gradient_2stops_avx+0x78>
  .byte  196,65,49,239,201                   // vpxor         %xmm9,%xmm9,%xmm9
  .byte  196,65,57,105,201                   // vpunpckhwd    %xmm9,%xmm8,%xmm9
  .byte  196,66,121,51,192                   // vpmovzxwd     %xmm8,%xmm8
  .byte  196,67,61,24,193,1                  // vinsertf128   $0x1,%xmm9,%ymm8,%ymm8
  .byte  196,65,124,91,192                   // vcvtdq2ps     %ymm8,%ymm8
  .byte  196,98,125,24,74,12                 // vbroadcastss  0xc(%rdx),%ymm9
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
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  196,65,56,87,192                    // vxorps        %xmm8,%xmm8,%xmm8
  .byte  197,120,19,4,36                     // vmovlps       %xmm8,(%rsp)
  .byte  73,1,248                            // add           %rdi,%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  67,138,4,8                          // mov           (%r8,%r9,1),%al
  .byte  66,136,4,12                         // mov           %al,(%rsp,%r9,1)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,240                             // jne           5a3 <_sk_lerp_u8_avx+0x88>
  .byte  197,122,126,4,36                    // vmovq         (%rsp),%xmm8
  .byte  196,66,121,48,192                   // vpmovzxbw     %xmm8,%xmm8
  .byte  233,106,255,255,255                 // jmpq          52c <_sk_lerp_u8_avx+0x11>

.globl _sk_lerp_565_avx
_sk_lerp_565_avx:
  .byte  72,131,236,24                       // sub           $0x18,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,183,0,0,0                    // jne           68b <_sk_lerp_565_avx+0xc9>
  .byte  197,122,111,4,120                   // vmovdqu       (%rax,%rdi,2),%xmm8
  .byte  197,225,239,219                     // vpxor         %xmm3,%xmm3,%xmm3
  .byte  197,185,105,219                     // vpunpckhwd    %xmm3,%xmm8,%xmm3
  .byte  196,66,121,51,192                   // vpmovzxwd     %xmm8,%xmm8
  .byte  196,99,61,24,195,1                  // vinsertf128   $0x1,%xmm3,%ymm8,%ymm8
  .byte  197,249,110,90,104                  // vmovd         0x68(%rdx),%xmm3
  .byte  196,227,121,4,219,0                 // vpermilps     $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  196,193,100,84,216                  // vandps        %ymm8,%ymm3,%ymm3
  .byte  197,252,91,219                      // vcvtdq2ps     %ymm3,%ymm3
  .byte  196,98,125,24,74,116                // vbroadcastss  0x74(%rdx),%ymm9
  .byte  197,52,89,203                       // vmulps        %ymm3,%ymm9,%ymm9
  .byte  197,249,110,90,108                  // vmovd         0x6c(%rdx),%xmm3
  .byte  196,227,121,4,219,0                 // vpermilps     $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  196,193,100,84,216                  // vandps        %ymm8,%ymm3,%ymm3
  .byte  197,252,91,219                      // vcvtdq2ps     %ymm3,%ymm3
  .byte  196,98,125,24,82,120                // vbroadcastss  0x78(%rdx),%ymm10
  .byte  197,44,89,211                       // vmulps        %ymm3,%ymm10,%ymm10
  .byte  197,249,110,90,112                  // vmovd         0x70(%rdx),%xmm3
  .byte  196,227,121,4,219,0                 // vpermilps     $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  196,193,100,84,216                  // vandps        %ymm8,%ymm3,%ymm3
  .byte  197,252,91,219                      // vcvtdq2ps     %ymm3,%ymm3
  .byte  196,98,125,24,66,124                // vbroadcastss  0x7c(%rdx),%ymm8
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
  .byte  196,226,125,24,26                   // vbroadcastss  (%rdx),%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,131,196,24                       // add           $0x18,%rsp
  .byte  255,224                             // jmpq          *%rax
  .byte  197,225,239,219                     // vpxor         %xmm3,%xmm3,%xmm3
  .byte  197,249,127,28,36                   // vmovdqa       %xmm3,(%rsp)
  .byte  76,141,4,120                        // lea           (%rax,%rdi,2),%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  102,67,139,4,72                     // mov           (%r8,%r9,2),%ax
  .byte  102,66,137,4,76                     // mov           %ax,(%rsp,%r9,2)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,238                             // jne           69b <_sk_lerp_565_avx+0xd9>
  .byte  197,121,111,4,36                    // vmovdqa       (%rsp),%xmm8
  .byte  233,34,255,255,255                  // jmpq          5d9 <_sk_lerp_565_avx+0x17>

.globl _sk_load_tables_avx
_sk_load_tables_avx:
  .byte  85                                  // push          %rbp
  .byte  72,137,229                          // mov           %rsp,%rbp
  .byte  65,87                               // push          %r15
  .byte  65,86                               // push          %r14
  .byte  65,85                               // push          %r13
  .byte  65,84                               // push          %r12
  .byte  83                                  // push          %rbx
  .byte  72,131,228,224                      // and           $0xffffffffffffffe0,%rsp
  .byte  72,131,236,96                       // sub           $0x60,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,137,116,36,24                    // mov           %rsi,0x18(%rsp)
  .byte  72,139,24                           // mov           (%rax),%rbx
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,43,2,0,0                     // jne           90a <_sk_load_tables_avx+0x253>
  .byte  197,124,16,20,187                   // vmovups       (%rbx,%rdi,4),%ymm10
  .byte  197,249,110,66,16                   // vmovd         0x10(%rdx),%xmm0
  .byte  196,227,121,4,192,0                 // vpermilps     $0x0,%xmm0,%xmm0
  .byte  196,99,125,24,200,1                 // vinsertf128   $0x1,%xmm0,%ymm0,%ymm9
  .byte  196,193,52,84,194                   // vandps        %ymm10,%ymm9,%ymm0
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
  .byte  72,139,112,8                        // mov           0x8(%rax),%rsi
  .byte  76,139,64,16                        // mov           0x10(%rax),%r8
  .byte  196,161,122,16,4,190                // vmovss        (%rsi,%r15,4),%xmm0
  .byte  196,163,121,33,4,166,16             // vinsertps     $0x10,(%rsi,%r12,4),%xmm0,%xmm0
  .byte  196,161,122,16,12,174               // vmovss        (%rsi,%r13,4),%xmm1
  .byte  196,227,121,33,193,32               // vinsertps     $0x20,%xmm1,%xmm0,%xmm0
  .byte  197,250,16,12,158                   // vmovss        (%rsi,%rbx,4),%xmm1
  .byte  196,227,121,33,193,48               // vinsertps     $0x30,%xmm1,%xmm0,%xmm0
  .byte  196,161,122,16,12,158               // vmovss        (%rsi,%r11,4),%xmm1
  .byte  196,163,113,33,12,142,16            // vinsertps     $0x10,(%rsi,%r9,4),%xmm1,%xmm1
  .byte  196,161,122,16,20,182               // vmovss        (%rsi,%r14,4),%xmm2
  .byte  196,227,113,33,202,32               // vinsertps     $0x20,%xmm2,%xmm1,%xmm1
  .byte  196,161,122,16,20,150               // vmovss        (%rsi,%r10,4),%xmm2
  .byte  196,227,113,33,202,48               // vinsertps     $0x30,%xmm2,%xmm1,%xmm1
  .byte  196,227,117,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  .byte  196,193,113,114,210,8               // vpsrld        $0x8,%xmm10,%xmm1
  .byte  196,67,125,25,208,1                 // vextractf128  $0x1,%ymm10,%xmm8
  .byte  196,193,105,114,208,8               // vpsrld        $0x8,%xmm8,%xmm2
  .byte  196,227,117,24,202,1                // vinsertf128   $0x1,%xmm2,%ymm1,%ymm1
  .byte  197,180,84,201                      // vandps        %ymm1,%ymm9,%ymm1
  .byte  196,193,249,126,201                 // vmovq         %xmm1,%r9
  .byte  69,137,203                          // mov           %r9d,%r11d
  .byte  196,195,249,22,202,1                // vpextrq       $0x1,%xmm1,%r10
  .byte  69,137,214                          // mov           %r10d,%r14d
  .byte  73,193,234,32                       // shr           $0x20,%r10
  .byte  73,193,233,32                       // shr           $0x20,%r9
  .byte  196,227,125,25,201,1                // vextractf128  $0x1,%ymm1,%xmm1
  .byte  196,225,249,126,206                 // vmovq         %xmm1,%rsi
  .byte  65,137,247                          // mov           %esi,%r15d
  .byte  196,227,249,22,203,1                // vpextrq       $0x1,%xmm1,%rbx
  .byte  65,137,220                          // mov           %ebx,%r12d
  .byte  72,193,235,32                       // shr           $0x20,%rbx
  .byte  72,193,238,32                       // shr           $0x20,%rsi
  .byte  196,129,122,16,12,184               // vmovss        (%r8,%r15,4),%xmm1
  .byte  196,195,113,33,12,176,16            // vinsertps     $0x10,(%r8,%rsi,4),%xmm1,%xmm1
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
  .byte  196,193,105,114,210,16              // vpsrld        $0x10,%xmm10,%xmm2
  .byte  196,193,97,114,208,16               // vpsrld        $0x10,%xmm8,%xmm3
  .byte  196,227,109,24,211,1                // vinsertf128   $0x1,%xmm3,%ymm2,%ymm2
  .byte  197,180,84,210                      // vandps        %ymm2,%ymm9,%ymm2
  .byte  196,193,249,126,208                 // vmovq         %xmm2,%r8
  .byte  69,137,194                          // mov           %r8d,%r10d
  .byte  196,195,249,22,209,1                // vpextrq       $0x1,%xmm2,%r9
  .byte  69,137,203                          // mov           %r9d,%r11d
  .byte  73,193,233,32                       // shr           $0x20,%r9
  .byte  73,193,232,32                       // shr           $0x20,%r8
  .byte  196,227,125,25,210,1                // vextractf128  $0x1,%ymm2,%xmm2
  .byte  196,225,249,126,214                 // vmovq         %xmm2,%rsi
  .byte  65,137,246                          // mov           %esi,%r14d
  .byte  196,227,249,22,211,1                // vpextrq       $0x1,%xmm2,%rbx
  .byte  65,137,223                          // mov           %ebx,%r15d
  .byte  72,193,235,32                       // shr           $0x20,%rbx
  .byte  72,193,238,32                       // shr           $0x20,%rsi
  .byte  196,161,122,16,20,176               // vmovss        (%rax,%r14,4),%xmm2
  .byte  196,227,105,33,20,176,16            // vinsertps     $0x10,(%rax,%rsi,4),%xmm2,%xmm2
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
  .byte  196,193,49,114,210,24               // vpsrld        $0x18,%xmm10,%xmm9
  .byte  196,193,97,114,208,24               // vpsrld        $0x18,%xmm8,%xmm3
  .byte  196,227,53,24,219,1                 // vinsertf128   $0x1,%xmm3,%ymm9,%ymm3
  .byte  197,252,91,219                      // vcvtdq2ps     %ymm3,%ymm3
  .byte  196,98,125,24,66,12                 // vbroadcastss  0xc(%rdx),%ymm8
  .byte  196,193,100,89,216                  // vmulps        %ymm8,%ymm3,%ymm3
  .byte  72,139,116,36,24                    // mov           0x18(%rsp),%rsi
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,208                             // callq         *%rax
  .byte  72,141,101,216                      // lea           -0x28(%rbp),%rsp
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,93                               // pop           %r13
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  93                                  // pop           %rbp
  .byte  197,248,119                         // vzeroupper
  .byte  195                                 // retq
  .byte  197,248,87,192                      // vxorps        %xmm0,%xmm0,%xmm0
  .byte  197,252,41,68,36,32                 // vmovaps       %ymm0,0x20(%rsp)
  .byte  76,141,4,187                        // lea           (%rbx,%rdi,4),%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  67,139,28,136                       // mov           (%r8,%r9,4),%ebx
  .byte  66,137,92,140,32                    // mov           %ebx,0x20(%rsp,%r9,4)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,239                             // jne           91b <_sk_load_tables_avx+0x264>
  .byte  197,124,40,84,36,32                 // vmovaps       0x20(%rsp),%ymm10
  .byte  233,173,253,255,255                 // jmpq          6e4 <_sk_load_tables_avx+0x2d>

.globl _sk_load_a8_avx
_sk_load_a8_avx:
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,65                              // jne           983 <_sk_load_a8_avx+0x4c>
  .byte  196,194,121,48,4,56                 // vpmovzxbw     (%r8,%rdi,1),%xmm0
  .byte  197,249,219,5,16,10,0,0             // vpand         0xa10(%rip),%xmm0,%xmm0        # 1360 <_sk_linear_gradient_2stops_avx+0x88>
  .byte  197,241,239,201                     // vpxor         %xmm1,%xmm1,%xmm1
  .byte  197,249,105,201                     // vpunpckhwd    %xmm1,%xmm0,%xmm1
  .byte  196,226,121,51,192                  // vpmovzxwd     %xmm0,%xmm0
  .byte  196,227,125,24,193,1                // vinsertf128   $0x1,%xmm1,%ymm0,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  196,226,125,24,74,12                // vbroadcastss  0xc(%rdx),%ymm1
  .byte  197,252,89,217                      // vmulps        %ymm1,%ymm0,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  197,252,87,192                      // vxorps        %ymm0,%ymm0,%ymm0
  .byte  197,244,87,201                      // vxorps        %ymm1,%ymm1,%ymm1
  .byte  197,236,87,210                      // vxorps        %ymm2,%ymm2,%ymm2
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  197,248,87,192                      // vxorps        %xmm0,%xmm0,%xmm0
  .byte  197,248,19,4,36                     // vmovlps       %xmm0,(%rsp)
  .byte  73,1,248                            // add           %rdi,%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  67,138,4,8                          // mov           (%r8,%r9,1),%al
  .byte  66,136,4,12                         // mov           %al,(%rsp,%r9,1)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,240                             // jne           992 <_sk_load_a8_avx+0x5b>
  .byte  197,250,126,4,36                    // vmovq         (%rsp),%xmm0
  .byte  196,226,121,48,192                  // vpmovzxbw     %xmm0,%xmm0
  .byte  235,154                             // jmp           948 <_sk_load_a8_avx+0x11>

.globl _sk_store_a8_avx
_sk_store_a8_avx:
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  196,98,125,24,66,8                  // vbroadcastss  0x8(%rdx),%ymm8
  .byte  197,60,89,195                       // vmulps        %ymm3,%ymm8,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,67,125,25,193,1                 // vextractf128  $0x1,%ymm8,%xmm9
  .byte  196,66,57,43,193                    // vpackusdw     %xmm9,%xmm8,%xmm8
  .byte  196,65,57,103,192                   // vpackuswb     %xmm8,%xmm8,%xmm8
  .byte  197,121,214,4,36                    // vmovq         %xmm8,(%rsp)
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,12                              // jne           9e9 <_sk_store_a8_avx+0x3b>
  .byte  196,65,123,17,4,56                  // vmovsd        %xmm8,(%r8,%rdi,1)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  196,65,249,126,193                  // vmovq         %xmm8,%r9
  .byte  69,136,12,56                        // mov           %r9b,(%r8,%rdi,1)
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,235                             // je            9e3 <_sk_store_a8_avx+0x35>
  .byte  76,137,200                          // mov           %r9,%rax
  .byte  72,193,232,8                        // shr           $0x8,%rax
  .byte  65,136,68,56,1                      // mov           %al,0x1(%r8,%rdi,1)
  .byte  72,131,249,2                        // cmp           $0x2,%rcx
  .byte  116,217                             // je            9e3 <_sk_store_a8_avx+0x35>
  .byte  73,193,233,16                       // shr           $0x10,%r9
  .byte  69,136,76,56,2                      // mov           %r9b,0x2(%r8,%rdi,1)
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  116,202                             // je            9e3 <_sk_store_a8_avx+0x35>
  .byte  76,141,84,36,3                      // lea           0x3(%rsp),%r10
  .byte  65,185,3,0,0,0                      // mov           $0x3,%r9d
  .byte  73,41,201                           // sub           %rcx,%r9
  .byte  74,141,68,7,3                       // lea           0x3(%rdi,%r8,1),%rax
  .byte  69,138,2                            // mov           (%r10),%r8b
  .byte  73,255,194                          // inc           %r10
  .byte  68,136,0                            // mov           %r8b,(%rax)
  .byte  72,255,192                          // inc           %rax
  .byte  73,255,193                          // inc           %r9
  .byte  117,239                             // jne           a2c <_sk_store_a8_avx+0x7e>
  .byte  235,164                             // jmp           9e3 <_sk_store_a8_avx+0x35>

.globl _sk_load_565_avx
_sk_load_565_avx:
  .byte  72,131,236,24                       // sub           $0x18,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,142,0,0,0                    // jne           adf <_sk_load_565_avx+0xa0>
  .byte  197,250,111,4,120                   // vmovdqu       (%rax,%rdi,2),%xmm0
  .byte  197,241,239,201                     // vpxor         %xmm1,%xmm1,%xmm1
  .byte  197,249,105,201                     // vpunpckhwd    %xmm1,%xmm0,%xmm1
  .byte  196,226,121,51,192                  // vpmovzxwd     %xmm0,%xmm0
  .byte  196,227,125,24,209,1                // vinsertf128   $0x1,%xmm1,%ymm0,%ymm2
  .byte  197,249,110,66,104                  // vmovd         0x68(%rdx),%xmm0
  .byte  196,227,121,4,192,0                 // vpermilps     $0x0,%xmm0,%xmm0
  .byte  196,227,125,24,192,1                // vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  .byte  197,252,84,194                      // vandps        %ymm2,%ymm0,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  196,226,125,24,74,116               // vbroadcastss  0x74(%rdx),%ymm1
  .byte  197,244,89,192                      // vmulps        %ymm0,%ymm1,%ymm0
  .byte  197,249,110,74,108                  // vmovd         0x6c(%rdx),%xmm1
  .byte  196,227,121,4,201,0                 // vpermilps     $0x0,%xmm1,%xmm1
  .byte  196,227,117,24,201,1                // vinsertf128   $0x1,%xmm1,%ymm1,%ymm1
  .byte  197,244,84,202                      // vandps        %ymm2,%ymm1,%ymm1
  .byte  197,252,91,201                      // vcvtdq2ps     %ymm1,%ymm1
  .byte  196,226,125,24,90,120               // vbroadcastss  0x78(%rdx),%ymm3
  .byte  197,228,89,201                      // vmulps        %ymm1,%ymm3,%ymm1
  .byte  197,249,110,90,112                  // vmovd         0x70(%rdx),%xmm3
  .byte  196,227,121,4,219,0                 // vpermilps     $0x0,%xmm3,%xmm3
  .byte  196,227,101,24,219,1                // vinsertf128   $0x1,%xmm3,%ymm3,%ymm3
  .byte  197,228,84,210                      // vandps        %ymm2,%ymm3,%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  196,226,125,24,90,124               // vbroadcastss  0x7c(%rdx),%ymm3
  .byte  197,228,89,210                      // vmulps        %ymm2,%ymm3,%ymm2
  .byte  196,226,125,24,26                   // vbroadcastss  (%rdx),%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,131,196,24                       // add           $0x18,%rsp
  .byte  255,224                             // jmpq          *%rax
  .byte  197,248,87,192                      // vxorps        %xmm0,%xmm0,%xmm0
  .byte  197,248,41,4,36                     // vmovaps       %xmm0,(%rsp)
  .byte  76,141,4,120                        // lea           (%rax,%rdi,2),%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  102,67,139,4,72                     // mov           (%r8,%r9,2),%ax
  .byte  102,66,137,4,76                     // mov           %ax,(%rsp,%r9,2)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,238                             // jne           aef <_sk_load_565_avx+0xb0>
  .byte  197,249,111,4,36                    // vmovdqa       (%rsp),%xmm0
  .byte  233,75,255,255,255                  // jmpq          a56 <_sk_load_565_avx+0x17>

.globl _sk_store_565_avx
_sk_store_565_avx:
  .byte  72,131,236,24                       // sub           $0x18,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  77,141,12,120                       // lea           (%r8,%rdi,2),%r9
  .byte  196,98,125,24,130,128,0,0,0         // vbroadcastss  0x80(%rdx),%ymm8
  .byte  197,60,89,200                       // vmulps        %ymm0,%ymm8,%ymm9
  .byte  196,65,125,91,201                   // vcvtps2dq     %ymm9,%ymm9
  .byte  196,193,41,114,241,11               // vpslld        $0xb,%xmm9,%xmm10
  .byte  196,67,125,25,201,1                 // vextractf128  $0x1,%ymm9,%xmm9
  .byte  196,193,49,114,241,11               // vpslld        $0xb,%xmm9,%xmm9
  .byte  196,67,45,24,201,1                  // vinsertf128   $0x1,%xmm9,%ymm10,%ymm9
  .byte  196,98,125,24,146,132,0,0,0         // vbroadcastss  0x84(%rdx),%ymm10
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
  .byte  197,121,127,4,36                    // vmovdqa       %xmm8,(%rsp)
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,18                              // jne           ba6 <_sk_store_565_avx+0x9b>
  .byte  197,121,111,4,36                    // vmovdqa       (%rsp),%xmm8
  .byte  196,65,122,127,1                    // vmovdqu       %xmm8,(%r9)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,131,196,24                       // add           $0x18,%rsp
  .byte  255,224                             // jmpq          *%rax
  .byte  197,121,126,192                     // vmovd         %xmm8,%eax
  .byte  102,65,137,1                        // mov           %ax,(%r9)
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,234                             // je            b9e <_sk_store_565_avx+0x93>
  .byte  196,193,121,197,192,1               // vpextrw       $0x1,%xmm8,%eax
  .byte  102,65,137,65,2                     // mov           %ax,0x2(%r9)
  .byte  72,131,249,2                        // cmp           $0x2,%rcx
  .byte  116,217                             // je            b9e <_sk_store_565_avx+0x93>
  .byte  196,193,121,197,192,2               // vpextrw       $0x2,%xmm8,%eax
  .byte  102,65,137,65,4                     // mov           %ax,0x4(%r9)
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  116,200                             // je            b9e <_sk_store_565_avx+0x93>
  .byte  76,141,84,36,6                      // lea           0x6(%rsp),%r10
  .byte  65,185,3,0,0,0                      // mov           $0x3,%r9d
  .byte  73,41,201                           // sub           %rcx,%r9
  .byte  73,141,68,120,6                     // lea           0x6(%r8,%rdi,2),%rax
  .byte  102,69,139,2                        // mov           (%r10),%r8w
  .byte  73,131,194,2                        // add           $0x2,%r10
  .byte  102,68,137,0                        // mov           %r8w,(%rax)
  .byte  72,131,192,2                        // add           $0x2,%rax
  .byte  73,255,193                          // inc           %r9
  .byte  117,235                             // jne           be9 <_sk_store_565_avx+0xde>
  .byte  235,158                             // jmp           b9e <_sk_store_565_avx+0x93>

.globl _sk_load_8888_avx
_sk_load_8888_avx:
  .byte  85                                  // push          %rbp
  .byte  72,137,229                          // mov           %rsp,%rbp
  .byte  72,131,228,224                      // and           $0xffffffffffffffe0,%rsp
  .byte  72,131,236,64                       // sub           $0x40,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,142,0,0,0                    // jne           ca8 <_sk_load_8888_avx+0xa8>
  .byte  197,252,16,28,184                   // vmovups       (%rax,%rdi,4),%ymm3
  .byte  197,249,110,66,16                   // vmovd         0x10(%rdx),%xmm0
  .byte  196,227,121,4,192,0                 // vpermilps     $0x0,%xmm0,%xmm0
  .byte  196,99,125,24,216,1                 // vinsertf128   $0x1,%xmm0,%ymm0,%ymm11
  .byte  197,164,84,195                      // vandps        %ymm3,%ymm11,%ymm0
  .byte  197,252,91,192                      // vcvtdq2ps     %ymm0,%ymm0
  .byte  196,98,125,24,66,12                 // vbroadcastss  0xc(%rdx),%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,169,114,211,8                   // vpsrld        $0x8,%xmm3,%xmm10
  .byte  196,195,125,25,217,1                // vextractf128  $0x1,%ymm3,%xmm9
  .byte  196,193,113,114,209,8               // vpsrld        $0x8,%xmm9,%xmm1
  .byte  196,227,45,24,201,1                 // vinsertf128   $0x1,%xmm1,%ymm10,%ymm1
  .byte  197,164,84,201                      // vandps        %ymm1,%ymm11,%ymm1
  .byte  197,252,91,201                      // vcvtdq2ps     %ymm1,%ymm1
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  197,169,114,211,16                  // vpsrld        $0x10,%xmm3,%xmm10
  .byte  196,193,105,114,209,16              // vpsrld        $0x10,%xmm9,%xmm2
  .byte  196,227,45,24,210,1                 // vinsertf128   $0x1,%xmm2,%ymm10,%ymm2
  .byte  197,164,84,210                      // vandps        %ymm2,%ymm11,%ymm2
  .byte  197,252,91,210                      // vcvtdq2ps     %ymm2,%ymm2
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  197,169,114,211,24                  // vpsrld        $0x18,%xmm3,%xmm10
  .byte  196,193,97,114,209,24               // vpsrld        $0x18,%xmm9,%xmm3
  .byte  196,227,45,24,219,1                 // vinsertf128   $0x1,%xmm3,%ymm10,%ymm3
  .byte  197,252,91,219                      // vcvtdq2ps     %ymm3,%ymm3
  .byte  196,193,100,89,216                  // vmulps        %ymm8,%ymm3,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,208                             // callq         *%rax
  .byte  72,137,236                          // mov           %rbp,%rsp
  .byte  93                                  // pop           %rbp
  .byte  197,248,119                         // vzeroupper
  .byte  195                                 // retq
  .byte  197,248,87,192                      // vxorps        %xmm0,%xmm0,%xmm0
  .byte  197,252,41,4,36                     // vmovaps       %ymm0,(%rsp)
  .byte  76,141,4,184                        // lea           (%rax,%rdi,4),%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  67,139,4,136                        // mov           (%r8,%r9,4),%eax
  .byte  66,137,4,140                        // mov           %eax,(%rsp,%r9,4)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,240                             // jne           cb8 <_sk_load_8888_avx+0xb8>
  .byte  197,252,40,28,36                    // vmovaps       (%rsp),%ymm3
  .byte  233,77,255,255,255                  // jmpq          c1f <_sk_load_8888_avx+0x1f>

.globl _sk_store_8888_avx
_sk_store_8888_avx:
  .byte  85                                  // push          %rbp
  .byte  72,137,229                          // mov           %rsp,%rbp
  .byte  72,131,228,224                      // and           $0xffffffffffffffe0,%rsp
  .byte  72,131,236,64                       // sub           $0x40,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  77,141,12,184                       // lea           (%r8,%rdi,4),%r9
  .byte  196,98,125,24,66,8                  // vbroadcastss  0x8(%rdx),%ymm8
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
  .byte  196,65,53,86,202                    // vorpd         %ymm10,%ymm9,%ymm9
  .byte  197,60,89,195                       // vmulps        %ymm3,%ymm8,%ymm8
  .byte  196,65,125,91,192                   // vcvtps2dq     %ymm8,%ymm8
  .byte  196,193,41,114,240,24               // vpslld        $0x18,%xmm8,%xmm10
  .byte  196,67,125,25,192,1                 // vextractf128  $0x1,%ymm8,%xmm8
  .byte  196,193,57,114,240,24               // vpslld        $0x18,%xmm8,%xmm8
  .byte  196,67,45,24,192,1                  // vinsertf128   $0x1,%xmm8,%ymm10,%ymm8
  .byte  196,65,53,86,192                    // vorpd         %ymm8,%ymm9,%ymm8
  .byte  197,125,41,4,36                     // vmovapd       %ymm8,(%rsp)
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,22                              // jne           d88 <_sk_store_8888_avx+0xb6>
  .byte  197,125,40,4,36                     // vmovapd       (%rsp),%ymm8
  .byte  196,65,125,17,1                     // vmovupd       %ymm8,(%r9)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,208                             // callq         *%rax
  .byte  72,137,236                          // mov           %rbp,%rsp
  .byte  93                                  // pop           %rbp
  .byte  197,248,119                         // vzeroupper
  .byte  195                                 // retq
  .byte  196,65,121,126,1                    // vmovd         %xmm8,(%r9)
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,233                             // je            d7c <_sk_store_8888_avx+0xaa>
  .byte  196,99,121,22,192,1                 // vpextrd       $0x1,%xmm8,%eax
  .byte  65,137,65,4                         // mov           %eax,0x4(%r9)
  .byte  72,131,249,2                        // cmp           $0x2,%rcx
  .byte  116,217                             // je            d7c <_sk_store_8888_avx+0xaa>
  .byte  196,99,121,22,192,2                 // vpextrd       $0x2,%xmm8,%eax
  .byte  65,137,65,8                         // mov           %eax,0x8(%r9)
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  116,201                             // je            d7c <_sk_store_8888_avx+0xaa>
  .byte  76,141,84,36,12                     // lea           0xc(%rsp),%r10
  .byte  65,185,3,0,0,0                      // mov           $0x3,%r9d
  .byte  73,41,201                           // sub           %rcx,%r9
  .byte  73,141,68,184,12                    // lea           0xc(%r8,%rdi,4),%rax
  .byte  69,139,2                            // mov           (%r10),%r8d
  .byte  73,131,194,4                        // add           $0x4,%r10
  .byte  68,137,0                            // mov           %r8d,(%rax)
  .byte  72,131,192,4                        // add           $0x4,%rax
  .byte  73,255,193                          // inc           %r9
  .byte  117,237                             // jne           dc6 <_sk_store_8888_avx+0xf4>
  .byte  235,161                             // jmp           d7c <_sk_store_8888_avx+0xaa>

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
  .byte  197,113,105,202                     // vpunpckhwd    %xmm2,%xmm1,%xmm9
  .byte  197,249,110,82,100                  // vmovd         0x64(%rdx),%xmm2
  .byte  197,249,112,210,0                   // vpshufd       $0x0,%xmm2,%xmm2
  .byte  197,233,101,203                     // vpcmpgtw      %xmm3,%xmm2,%xmm1
  .byte  197,241,223,203                     // vpandn        %xmm3,%xmm1,%xmm1
  .byte  197,233,101,216                     // vpcmpgtw      %xmm0,%xmm2,%xmm3
  .byte  197,225,223,192                     // vpandn        %xmm0,%xmm3,%xmm0
  .byte  196,193,105,101,216                 // vpcmpgtw      %xmm8,%xmm2,%xmm3
  .byte  196,193,97,223,216                  // vpandn        %xmm8,%xmm3,%xmm3
  .byte  196,193,105,101,209                 // vpcmpgtw      %xmm9,%xmm2,%xmm2
  .byte  196,193,105,223,209                 // vpandn        %xmm9,%xmm2,%xmm2
  .byte  196,98,121,51,193                   // vpmovzxwd     %xmm1,%xmm8
  .byte  196,98,121,51,203                   // vpmovzxwd     %xmm3,%xmm9
  .byte  196,65,41,239,210                   // vpxor         %xmm10,%xmm10,%xmm10
  .byte  196,193,113,105,202                 // vpunpckhwd    %xmm10,%xmm1,%xmm1
  .byte  196,193,97,105,218                  // vpunpckhwd    %xmm10,%xmm3,%xmm3
  .byte  196,98,121,51,216                   // vpmovzxwd     %xmm0,%xmm11
  .byte  196,98,121,51,226                   // vpmovzxwd     %xmm2,%xmm12
  .byte  196,65,121,105,234                  // vpunpckhwd    %xmm10,%xmm0,%xmm13
  .byte  196,65,105,105,210                  // vpunpckhwd    %xmm10,%xmm2,%xmm10
  .byte  196,193,121,114,240,13              // vpslld        $0xd,%xmm8,%xmm0
  .byte  196,193,105,114,241,13              // vpslld        $0xd,%xmm9,%xmm2
  .byte  196,227,125,24,194,1                // vinsertf128   $0x1,%xmm2,%ymm0,%ymm0
  .byte  197,249,110,82,92                   // vmovd         0x5c(%rdx),%xmm2
  .byte  196,227,121,4,210,0                 // vpermilps     $0x0,%xmm2,%xmm2
  .byte  196,99,109,24,194,1                 // vinsertf128   $0x1,%xmm2,%ymm2,%ymm8
  .byte  197,188,89,192                      // vmulps        %ymm0,%ymm8,%ymm0
  .byte  197,241,114,241,13                  // vpslld        $0xd,%xmm1,%xmm1
  .byte  197,233,114,243,13                  // vpslld        $0xd,%xmm3,%xmm2
  .byte  196,227,117,24,202,1                // vinsertf128   $0x1,%xmm2,%ymm1,%ymm1
  .byte  197,188,89,201                      // vmulps        %ymm1,%ymm8,%ymm1
  .byte  196,193,105,114,243,13              // vpslld        $0xd,%xmm11,%xmm2
  .byte  196,193,97,114,244,13               // vpslld        $0xd,%xmm12,%xmm3
  .byte  196,227,109,24,211,1                // vinsertf128   $0x1,%xmm3,%ymm2,%ymm2
  .byte  197,188,89,210                      // vmulps        %ymm2,%ymm8,%ymm2
  .byte  196,193,49,114,245,13               // vpslld        $0xd,%xmm13,%xmm9
  .byte  196,193,97,114,242,13               // vpslld        $0xd,%xmm10,%xmm3
  .byte  196,227,53,24,219,1                 // vinsertf128   $0x1,%xmm3,%ymm9,%ymm3
  .byte  197,188,89,219                      // vmulps        %ymm3,%ymm8,%ymm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_store_f16_avx
_sk_store_f16_avx:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  197,121,110,66,96                   // vmovd         0x60(%rdx),%xmm8
  .byte  196,67,121,4,192,0                  // vpermilps     $0x0,%xmm8,%xmm8
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

.globl _sk_start_pipeline_sse41
_sk_start_pipeline_sse41:
  .byte  65,87                               // push          %r15
  .byte  65,86                               // push          %r14
  .byte  65,85                               // push          %r13
  .byte  65,84                               // push          %r12
  .byte  83                                  // push          %rbx
  .byte  73,137,204                          // mov           %rcx,%r12
  .byte  73,137,214                          // mov           %rdx,%r14
  .byte  72,137,251                          // mov           %rdi,%rbx
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  73,137,247                          // mov           %rsi,%r15
  .byte  72,141,75,4                         // lea           0x4(%rbx),%rcx
  .byte  76,57,225                           // cmp           %r12,%rcx
  .byte  118,5                               // jbe           25 <_sk_start_pipeline_sse41+0x25>
  .byte  72,137,223                          // mov           %rbx,%rdi
  .byte  235,63                              // jmp           64 <_sk_start_pipeline_sse41+0x64>
  .byte  185,0,0,0,0                         // mov           $0x0,%ecx
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,87,201                           // xorps         %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  15,87,219                           // xorps         %xmm3,%xmm3
  .byte  15,87,228                           // xorps         %xmm4,%xmm4
  .byte  15,87,237                           // xorps         %xmm5,%xmm5
  .byte  15,87,246                           // xorps         %xmm6,%xmm6
  .byte  15,87,255                           // xorps         %xmm7,%xmm7
  .byte  72,137,223                          // mov           %rbx,%rdi
  .byte  76,137,254                          // mov           %r15,%rsi
  .byte  76,137,242                          // mov           %r14,%rdx
  .byte  73,137,197                          // mov           %rax,%r13
  .byte  65,255,213                          // callq         *%r13
  .byte  76,137,232                          // mov           %r13,%rax
  .byte  72,141,123,4                        // lea           0x4(%rbx),%rdi
  .byte  72,131,195,8                        // add           $0x8,%rbx
  .byte  76,57,227                           // cmp           %r12,%rbx
  .byte  72,137,251                          // mov           %rdi,%rbx
  .byte  118,193                             // jbe           25 <_sk_start_pipeline_sse41+0x25>
  .byte  76,57,231                           // cmp           %r12,%rdi
  .byte  117,10                              // jne           73 <_sk_start_pipeline_sse41+0x73>
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,93                               // pop           %r13
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  195                                 // retq
  .byte  73,41,252                           // sub           %rdi,%r12
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,87,201                           // xorps         %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  15,87,219                           // xorps         %xmm3,%xmm3
  .byte  15,87,228                           // xorps         %xmm4,%xmm4
  .byte  15,87,237                           // xorps         %xmm5,%xmm5
  .byte  15,87,246                           // xorps         %xmm6,%xmm6
  .byte  15,87,255                           // xorps         %xmm7,%xmm7
  .byte  76,137,254                          // mov           %r15,%rsi
  .byte  76,137,242                          // mov           %r14,%rdx
  .byte  76,137,225                          // mov           %r12,%rcx
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,93                               // pop           %r13
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  255,224                             // jmpq          *%rax

.globl _sk_just_return_sse41
_sk_just_return_sse41:
  .byte  195                                 // retq

.globl _sk_seed_shader_sse41
_sk_seed_shader_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  102,15,110,199                      // movd          %edi,%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  15,91,200                           // cvtdq2ps      %xmm0,%xmm1
  .byte  243,15,16,18                        // movss         (%rdx),%xmm2
  .byte  243,15,16,90,4                      // movss         0x4(%rdx),%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  15,88,203                           // addps         %xmm3,%xmm1
  .byte  15,16,66,20                         // movups        0x14(%rdx),%xmm0
  .byte  15,88,193                           // addps         %xmm1,%xmm0
  .byte  102,15,110,8                        // movd          (%rax),%xmm1
  .byte  102,15,112,201,0                    // pshufd        $0x0,%xmm1,%xmm1
  .byte  15,91,201                           // cvtdq2ps      %xmm1,%xmm1
  .byte  15,88,203                           // addps         %xmm3,%xmm1
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
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
  .byte  243,68,15,16,2                      // movss         (%rdx),%xmm8
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

.globl _sk_dstover_sse41
_sk_dstover_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,2                      // movss         (%rdx),%xmm8
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
  .byte  243,68,15,16,2                      // movss         (%rdx),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  65,15,93,192                        // minps         %xmm8,%xmm0
  .byte  65,15,93,200                        // minps         %xmm8,%xmm1
  .byte  65,15,93,208                        // minps         %xmm8,%xmm2
  .byte  65,15,93,216                        // minps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_clamp_a_sse41
_sk_clamp_a_sse41:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,2                      // movss         (%rdx),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  65,15,93,216                        // minps         %xmm8,%xmm3
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
  .byte  243,68,15,16,18                     // movss         (%rdx),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
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
  .byte  243,68,15,16,90,64                  // movss         0x40(%rdx),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  69,15,40,211                        // movaps        %xmm11,%xmm10
  .byte  68,15,89,208                        // mulps         %xmm0,%xmm10
  .byte  68,15,40,240                        // movaps        %xmm0,%xmm14
  .byte  69,15,89,246                        // mulps         %xmm14,%xmm14
  .byte  243,68,15,16,66,60                  // movss         0x3c(%rdx),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  243,68,15,16,98,52                  // movss         0x34(%rdx),%xmm12
  .byte  243,68,15,16,106,56                 // movss         0x38(%rdx),%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,200                        // mulps         %xmm0,%xmm9
  .byte  69,15,88,205                        // addps         %xmm13,%xmm9
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  69,15,89,206                        // mulps         %xmm14,%xmm9
  .byte  69,15,88,204                        // addps         %xmm12,%xmm9
  .byte  243,68,15,16,114,68                 // movss         0x44(%rdx),%xmm14
  .byte  69,15,198,246,0                     // shufps        $0x0,%xmm14,%xmm14
  .byte  65,15,194,198,1                     // cmpltps       %xmm14,%xmm0
  .byte  102,69,15,56,20,202                 // blendvps      %xmm0,%xmm10,%xmm9
  .byte  69,15,40,251                        // movaps        %xmm11,%xmm15
  .byte  68,15,89,249                        // mulps         %xmm1,%xmm15
  .byte  15,40,193                           // movaps        %xmm1,%xmm0
  .byte  15,89,192                           // mulps         %xmm0,%xmm0
  .byte  69,15,40,208                        // movaps        %xmm8,%xmm10
  .byte  68,15,89,209                        // mulps         %xmm1,%xmm10
  .byte  69,15,88,213                        // addps         %xmm13,%xmm10
  .byte  68,15,89,208                        // mulps         %xmm0,%xmm10
  .byte  69,15,88,212                        // addps         %xmm12,%xmm10
  .byte  65,15,194,206,1                     // cmpltps       %xmm14,%xmm1
  .byte  15,40,193                           // movaps        %xmm1,%xmm0
  .byte  102,69,15,56,20,215                 // blendvps      %xmm0,%xmm15,%xmm10
  .byte  68,15,89,218                        // mulps         %xmm2,%xmm11
  .byte  15,40,194                           // movaps        %xmm2,%xmm0
  .byte  15,89,192                           // mulps         %xmm0,%xmm0
  .byte  68,15,89,194                        // mulps         %xmm2,%xmm8
  .byte  69,15,88,197                        // addps         %xmm13,%xmm8
  .byte  68,15,89,192                        // mulps         %xmm0,%xmm8
  .byte  69,15,88,196                        // addps         %xmm12,%xmm8
  .byte  65,15,194,214,1                     // cmpltps       %xmm14,%xmm2
  .byte  15,40,194                           // movaps        %xmm2,%xmm0
  .byte  102,69,15,56,20,195                 // blendvps      %xmm0,%xmm11,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,193                        // movaps        %xmm9,%xmm0
  .byte  65,15,40,202                        // movaps        %xmm10,%xmm1
  .byte  65,15,40,208                        // movaps        %xmm8,%xmm2
  .byte  255,224                             // jmpq          *%rax

.globl _sk_to_srgb_sse41
_sk_to_srgb_sse41:
  .byte  72,131,236,24                       // sub           $0x18,%rsp
  .byte  15,41,60,36                         // movaps        %xmm7,(%rsp)
  .byte  15,40,254                           // movaps        %xmm6,%xmm7
  .byte  15,40,245                           // movaps        %xmm5,%xmm6
  .byte  15,40,236                           // movaps        %xmm4,%xmm5
  .byte  15,40,227                           // movaps        %xmm3,%xmm4
  .byte  68,15,40,194                        // movaps        %xmm2,%xmm8
  .byte  15,40,217                           // movaps        %xmm1,%xmm3
  .byte  15,82,208                           // rsqrtps       %xmm0,%xmm2
  .byte  68,15,83,202                        // rcpps         %xmm2,%xmm9
  .byte  68,15,82,210                        // rsqrtps       %xmm2,%xmm10
  .byte  243,15,16,18                        // movss         (%rdx),%xmm2
  .byte  243,68,15,16,90,72                  // movss         0x48(%rdx),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  65,15,40,203                        // movaps        %xmm11,%xmm1
  .byte  15,89,200                           // mulps         %xmm0,%xmm1
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  243,68,15,16,98,76                  // movss         0x4c(%rdx),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  243,68,15,16,106,80                 // movss         0x50(%rdx),%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  243,68,15,16,114,84                 // movss         0x54(%rdx),%xmm14
  .byte  69,15,198,246,0                     // shufps        $0x0,%xmm14,%xmm14
  .byte  69,15,89,205                        // mulps         %xmm13,%xmm9
  .byte  69,15,88,206                        // addps         %xmm14,%xmm9
  .byte  69,15,89,212                        // mulps         %xmm12,%xmm10
  .byte  69,15,88,209                        // addps         %xmm9,%xmm10
  .byte  68,15,40,202                        // movaps        %xmm2,%xmm9
  .byte  69,15,93,202                        // minps         %xmm10,%xmm9
  .byte  243,68,15,16,122,88                 // movss         0x58(%rdx),%xmm15
  .byte  69,15,198,255,0                     // shufps        $0x0,%xmm15,%xmm15
  .byte  65,15,194,199,1                     // cmpltps       %xmm15,%xmm0
  .byte  102,68,15,56,20,201                 // blendvps      %xmm0,%xmm1,%xmm9
  .byte  15,82,195                           // rsqrtps       %xmm3,%xmm0
  .byte  15,83,200                           // rcpps         %xmm0,%xmm1
  .byte  15,82,192                           // rsqrtps       %xmm0,%xmm0
  .byte  65,15,89,205                        // mulps         %xmm13,%xmm1
  .byte  65,15,88,206                        // addps         %xmm14,%xmm1
  .byte  65,15,89,196                        // mulps         %xmm12,%xmm0
  .byte  15,88,193                           // addps         %xmm1,%xmm0
  .byte  68,15,40,210                        // movaps        %xmm2,%xmm10
  .byte  68,15,93,208                        // minps         %xmm0,%xmm10
  .byte  65,15,40,203                        // movaps        %xmm11,%xmm1
  .byte  15,89,203                           // mulps         %xmm3,%xmm1
  .byte  65,15,194,223,1                     // cmpltps       %xmm15,%xmm3
  .byte  15,40,195                           // movaps        %xmm3,%xmm0
  .byte  102,68,15,56,20,209                 // blendvps      %xmm0,%xmm1,%xmm10
  .byte  65,15,82,192                        // rsqrtps       %xmm8,%xmm0
  .byte  15,83,200                           // rcpps         %xmm0,%xmm1
  .byte  65,15,89,205                        // mulps         %xmm13,%xmm1
  .byte  65,15,88,206                        // addps         %xmm14,%xmm1
  .byte  15,82,192                           // rsqrtps       %xmm0,%xmm0
  .byte  65,15,89,196                        // mulps         %xmm12,%xmm0
  .byte  15,88,193                           // addps         %xmm1,%xmm0
  .byte  15,93,208                           // minps         %xmm0,%xmm2
  .byte  69,15,89,216                        // mulps         %xmm8,%xmm11
  .byte  69,15,194,199,1                     // cmpltps       %xmm15,%xmm8
  .byte  65,15,40,192                        // movaps        %xmm8,%xmm0
  .byte  102,65,15,56,20,211                 // blendvps      %xmm0,%xmm11,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,193                        // movaps        %xmm9,%xmm0
  .byte  65,15,40,202                        // movaps        %xmm10,%xmm1
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
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,1,248                            // add           %rdi,%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,58                              // jne           518 <_sk_scale_u8_sse41+0x48>
  .byte  139,0                               // mov           (%rax),%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  102,69,15,56,49,192                 // pmovzxbd      %xmm8,%xmm8
  .byte  69,15,91,200                        // cvtdq2ps      %xmm8,%xmm9
  .byte  243,68,15,16,66,12                  // movss         0xc(%rdx),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,89,193                        // mulps         %xmm9,%xmm8
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  68,15,89,195                        // mulps         %xmm3,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,216                        // movaps        %xmm8,%xmm3
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  76,141,68,36,4                      // lea           0x4(%rsp),%r8
  .byte  199,68,36,4,0,0,0,0                 // movl          $0x0,0x4(%rsp)
  .byte  73,137,201                          // mov           %rcx,%r9
  .byte  73,255,201                          // dec           %r9
  .byte  68,138,16                           // mov           (%rax),%r10b
  .byte  72,141,64,1                         // lea           0x1(%rax),%rax
  .byte  69,136,16                           // mov           %r10b,(%r8)
  .byte  77,141,64,1                         // lea           0x1(%r8),%r8
  .byte  117,237                             // jne           528 <_sk_scale_u8_sse41+0x58>
  .byte  139,68,36,4                         // mov           0x4(%rsp),%eax
  .byte  235,159                             // jmp           4e0 <_sk_scale_u8_sse41+0x10>

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
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,1,248                            // add           %rdi,%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,78                              // jne           5d5 <_sk_lerp_u8_sse41+0x5c>
  .byte  139,0                               // mov           (%rax),%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  102,69,15,56,49,192                 // pmovzxbd      %xmm8,%xmm8
  .byte  69,15,91,192                        // cvtdq2ps      %xmm8,%xmm8
  .byte  243,68,15,16,74,12                  // movss         0xc(%rdx),%xmm9
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
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  76,141,68,36,4                      // lea           0x4(%rsp),%r8
  .byte  199,68,36,4,0,0,0,0                 // movl          $0x0,0x4(%rsp)
  .byte  73,137,201                          // mov           %rcx,%r9
  .byte  73,255,201                          // dec           %r9
  .byte  68,138,16                           // mov           (%rax),%r10b
  .byte  72,141,64,1                         // lea           0x1(%rax),%rax
  .byte  69,136,16                           // mov           %r10b,(%r8)
  .byte  77,141,64,1                         // lea           0x1(%r8),%r8
  .byte  117,237                             // jne           5e5 <_sk_lerp_u8_sse41+0x6c>
  .byte  139,68,36,4                         // mov           0x4(%rsp),%eax
  .byte  235,139                             // jmp           589 <_sk_lerp_u8_sse41+0x10>

.globl _sk_lerp_565_sse41
_sk_lerp_565_sse41:
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,168,0,0,0                    // jne           6b5 <_sk_lerp_565_sse41+0xb7>
  .byte  102,15,56,51,28,120                 // pmovzxwd      (%rax,%rdi,2),%xmm3
  .byte  102,69,15,239,192                   // pxor          %xmm8,%xmm8
  .byte  102,68,15,58,14,195,85              // pblendw       $0x55,%xmm3,%xmm8
  .byte  102,15,110,90,104                   // movd          0x68(%rdx),%xmm3
  .byte  102,15,112,219,0                    // pshufd        $0x0,%xmm3,%xmm3
  .byte  102,65,15,219,216                   // pand          %xmm8,%xmm3
  .byte  68,15,91,203                        // cvtdq2ps      %xmm3,%xmm9
  .byte  243,15,16,26                        // movss         (%rdx),%xmm3
  .byte  243,68,15,16,82,116                 // movss         0x74(%rdx),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  69,15,89,209                        // mulps         %xmm9,%xmm10
  .byte  102,68,15,110,74,108                // movd          0x6c(%rdx),%xmm9
  .byte  102,69,15,112,201,0                 // pshufd        $0x0,%xmm9,%xmm9
  .byte  102,69,15,219,200                   // pand          %xmm8,%xmm9
  .byte  69,15,91,201                        // cvtdq2ps      %xmm9,%xmm9
  .byte  243,68,15,16,90,120                 // movss         0x78(%rdx),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  69,15,89,217                        // mulps         %xmm9,%xmm11
  .byte  102,68,15,110,74,112                // movd          0x70(%rdx),%xmm9
  .byte  102,69,15,112,201,0                 // pshufd        $0x0,%xmm9,%xmm9
  .byte  102,69,15,219,200                   // pand          %xmm8,%xmm9
  .byte  69,15,91,193                        // cvtdq2ps      %xmm9,%xmm8
  .byte  243,68,15,16,74,124                 // movss         0x7c(%rdx),%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  15,92,196                           // subps         %xmm4,%xmm0
  .byte  65,15,89,194                        // mulps         %xmm10,%xmm0
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  15,92,205                           // subps         %xmm5,%xmm1
  .byte  65,15,89,203                        // mulps         %xmm11,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  15,92,214                           // subps         %xmm6,%xmm2
  .byte  65,15,89,209                        // mulps         %xmm9,%xmm2
  .byte  15,88,214                           // addps         %xmm6,%xmm2
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  15,87,219                           // xorps         %xmm3,%xmm3
  .byte  15,19,28,36                         // movlps        %xmm3,(%rsp)
  .byte  76,141,4,120                        // lea           (%rax,%rdi,2),%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  102,67,139,4,72                     // mov           (%r8,%r9,2),%ax
  .byte  102,66,137,4,76                     // mov           %ax,(%rsp,%r9,2)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,238                             // jne           6c3 <_sk_lerp_565_sse41+0xc5>
  .byte  243,15,126,28,36                    // movq          (%rsp),%xmm3
  .byte  102,15,56,51,219                    // pmovzxwd      %xmm3,%xmm3
  .byte  233,47,255,255,255                  // jmpq          613 <_sk_lerp_565_sse41+0x15>

.globl _sk_load_tables_sse41
_sk_load_tables_sse41:
  .byte  65,86                               // push          %r14
  .byte  83                                  // push          %rbx
  .byte  72,131,236,24                       // sub           $0x18,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,30,1,0,0                     // jne           817 <_sk_load_tables_sse41+0x133>
  .byte  243,69,15,111,4,184                 // movdqu        (%r8,%rdi,4),%xmm8
  .byte  102,15,110,66,16                    // movd          0x10(%rdx),%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  102,65,15,111,200                   // movdqa        %xmm8,%xmm1
  .byte  102,15,114,209,8                    // psrld         $0x8,%xmm1
  .byte  102,15,219,200                      // pand          %xmm0,%xmm1
  .byte  102,65,15,111,208                   // movdqa        %xmm8,%xmm2
  .byte  102,15,114,210,16                   // psrld         $0x10,%xmm2
  .byte  102,15,219,208                      // pand          %xmm0,%xmm2
  .byte  102,65,15,219,192                   // pand          %xmm8,%xmm0
  .byte  102,73,15,58,22,192,1               // pextrq        $0x1,%xmm0,%r8
  .byte  69,137,193                          // mov           %r8d,%r9d
  .byte  73,193,232,32                       // shr           $0x20,%r8
  .byte  102,73,15,126,194                   // movq          %xmm0,%r10
  .byte  69,137,211                          // mov           %r10d,%r11d
  .byte  73,193,234,32                       // shr           $0x20,%r10
  .byte  72,139,88,8                         // mov           0x8(%rax),%rbx
  .byte  76,139,112,16                       // mov           0x10(%rax),%r14
  .byte  243,66,15,16,4,155                  // movss         (%rbx,%r11,4),%xmm0
  .byte  102,66,15,58,33,4,147,16            // insertps      $0x10,(%rbx,%r10,4),%xmm0
  .byte  243,66,15,16,28,139                 // movss         (%rbx,%r9,4),%xmm3
  .byte  102,15,58,33,195,32                 // insertps      $0x20,%xmm3,%xmm0
  .byte  243,66,15,16,28,131                 // movss         (%rbx,%r8,4),%xmm3
  .byte  102,15,58,33,195,48                 // insertps      $0x30,%xmm3,%xmm0
  .byte  102,72,15,58,22,203,1               // pextrq        $0x1,%xmm1,%rbx
  .byte  65,137,216                          // mov           %ebx,%r8d
  .byte  72,193,235,32                       // shr           $0x20,%rbx
  .byte  102,73,15,126,201                   // movq          %xmm1,%r9
  .byte  69,137,202                          // mov           %r9d,%r10d
  .byte  73,193,233,32                       // shr           $0x20,%r9
  .byte  243,67,15,16,12,150                 // movss         (%r14,%r10,4),%xmm1
  .byte  102,67,15,58,33,12,142,16           // insertps      $0x10,(%r14,%r9,4),%xmm1
  .byte  243,67,15,16,28,134                 // movss         (%r14,%r8,4),%xmm3
  .byte  102,15,58,33,203,32                 // insertps      $0x20,%xmm3,%xmm1
  .byte  243,65,15,16,28,158                 // movss         (%r14,%rbx,4),%xmm3
  .byte  102,15,58,33,203,48                 // insertps      $0x30,%xmm3,%xmm1
  .byte  72,139,64,24                        // mov           0x18(%rax),%rax
  .byte  102,72,15,58,22,211,1               // pextrq        $0x1,%xmm2,%rbx
  .byte  65,137,216                          // mov           %ebx,%r8d
  .byte  72,193,235,32                       // shr           $0x20,%rbx
  .byte  102,73,15,126,209                   // movq          %xmm2,%r9
  .byte  69,137,202                          // mov           %r9d,%r10d
  .byte  73,193,233,32                       // shr           $0x20,%r9
  .byte  243,66,15,16,20,144                 // movss         (%rax,%r10,4),%xmm2
  .byte  102,66,15,58,33,20,136,16           // insertps      $0x10,(%rax,%r9,4),%xmm2
  .byte  243,66,15,16,28,128                 // movss         (%rax,%r8,4),%xmm3
  .byte  102,15,58,33,211,32                 // insertps      $0x20,%xmm3,%xmm2
  .byte  243,15,16,28,152                    // movss         (%rax,%rbx,4),%xmm3
  .byte  102,15,58,33,211,48                 // insertps      $0x30,%xmm3,%xmm2
  .byte  102,65,15,114,208,24                // psrld         $0x18,%xmm8
  .byte  69,15,91,192                        // cvtdq2ps      %xmm8,%xmm8
  .byte  243,15,16,90,12                     // movss         0xc(%rdx),%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,131,196,24                       // add           $0x18,%rsp
  .byte  91                                  // pop           %rbx
  .byte  65,94                               // pop           %r14
  .byte  255,224                             // jmpq          *%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,41,4,36                          // movaps        %xmm0,(%rsp)
  .byte  77,141,4,184                        // lea           (%r8,%rdi,4),%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  71,139,20,136                       // mov           (%r8,%r9,4),%r10d
  .byte  70,137,20,140                       // mov           %r10d,(%rsp,%r9,4)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,240                             // jne           825 <_sk_load_tables_sse41+0x141>
  .byte  102,68,15,111,4,36                  // movdqa        (%rsp),%xmm8
  .byte  233,191,254,255,255                 // jmpq          6ff <_sk_load_tables_sse41+0x1b>

.globl _sk_load_a8_sse41
_sk_load_a8_sse41:
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,1,248                            // add           %rdi,%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,41                              // jne           877 <_sk_load_a8_sse41+0x37>
  .byte  139,0                               // mov           (%rax),%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  102,15,56,49,192                    // pmovzxbd      %xmm0,%xmm0
  .byte  15,91,192                           // cvtdq2ps      %xmm0,%xmm0
  .byte  243,15,16,90,12                     // movss         0xc(%rdx),%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  15,89,216                           // mulps         %xmm0,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,87,201                           // xorps         %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  76,141,68,36,4                      // lea           0x4(%rsp),%r8
  .byte  199,68,36,4,0,0,0,0                 // movl          $0x0,0x4(%rsp)
  .byte  73,137,201                          // mov           %rcx,%r9
  .byte  73,255,201                          // dec           %r9
  .byte  68,138,16                           // mov           (%rax),%r10b
  .byte  72,141,64,1                         // lea           0x1(%rax),%rax
  .byte  69,136,16                           // mov           %r10b,(%r8)
  .byte  77,141,64,1                         // lea           0x1(%r8),%r8
  .byte  117,237                             // jne           887 <_sk_load_a8_sse41+0x47>
  .byte  139,68,36,4                         // mov           0x4(%rsp),%eax
  .byte  235,176                             // jmp           850 <_sk_load_a8_sse41+0x10>

.globl _sk_store_a8_sse41
_sk_store_a8_sse41:
  .byte  72,131,236,4                        // sub           $0x4,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  243,68,15,16,66,8                   // movss         0x8(%rdx),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,89,195                        // mulps         %xmm3,%xmm8
  .byte  102,69,15,91,192                    // cvtps2dq      %xmm8,%xmm8
  .byte  102,69,15,56,43,192                 // packusdw      %xmm8,%xmm8
  .byte  102,69,15,103,192                   // packuswb      %xmm8,%xmm8
  .byte  102,69,15,126,193                   // movd          %xmm8,%r9d
  .byte  102,68,15,126,4,36                  // movd          %xmm8,(%rsp)
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,12                              // jne           8e4 <_sk_store_a8_sse41+0x44>
  .byte  69,137,12,56                        // mov           %r9d,(%r8,%rdi,1)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,131,196,4                        // add           $0x4,%rsp
  .byte  255,224                             // jmpq          *%rax
  .byte  69,136,12,56                        // mov           %r9b,(%r8,%rdi,1)
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,238                             // je            8dc <_sk_store_a8_sse41+0x3c>
  .byte  68,137,200                          // mov           %r9d,%eax
  .byte  193,232,8                           // shr           $0x8,%eax
  .byte  65,136,68,56,1                      // mov           %al,0x1(%r8,%rdi,1)
  .byte  72,131,249,2                        // cmp           $0x2,%rcx
  .byte  116,221                             // je            8dc <_sk_store_a8_sse41+0x3c>
  .byte  65,193,233,16                       // shr           $0x10,%r9d
  .byte  69,136,76,56,2                      // mov           %r9b,0x2(%r8,%rdi,1)
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  116,206                             // je            8dc <_sk_store_a8_sse41+0x3c>
  .byte  76,141,84,36,3                      // lea           0x3(%rsp),%r10
  .byte  65,185,3,0,0,0                      // mov           $0x3,%r9d
  .byte  73,41,201                           // sub           %rcx,%r9
  .byte  74,141,68,7,3                       // lea           0x3(%rdi,%r8,1),%rax
  .byte  69,138,2                            // mov           (%r10),%r8b
  .byte  73,255,194                          // inc           %r10
  .byte  68,136,0                            // mov           %r8b,(%rax)
  .byte  72,255,192                          // inc           %rax
  .byte  73,255,193                          // inc           %r9
  .byte  117,239                             // jne           921 <_sk_store_a8_sse41+0x81>
  .byte  235,168                             // jmp           8dc <_sk_store_a8_sse41+0x3c>

.globl _sk_load_565_sse41
_sk_load_565_sse41:
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,126                             // jne           9bd <_sk_load_565_sse41+0x89>
  .byte  102,15,56,51,4,120                  // pmovzxwd      (%rax,%rdi,2),%xmm0
  .byte  102,69,15,239,201                   // pxor          %xmm9,%xmm9
  .byte  102,68,15,58,14,200,85              // pblendw       $0x55,%xmm0,%xmm9
  .byte  102,15,110,66,104                   // movd          0x68(%rdx),%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  102,65,15,219,193                   // pand          %xmm9,%xmm0
  .byte  15,91,200                           // cvtdq2ps      %xmm0,%xmm1
  .byte  243,15,16,26                        // movss         (%rdx),%xmm3
  .byte  243,15,16,66,116                    // movss         0x74(%rdx),%xmm0
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  15,89,193                           // mulps         %xmm1,%xmm0
  .byte  102,15,110,74,108                   // movd          0x6c(%rdx),%xmm1
  .byte  102,15,112,201,0                    // pshufd        $0x0,%xmm1,%xmm1
  .byte  102,65,15,219,201                   // pand          %xmm9,%xmm1
  .byte  68,15,91,193                        // cvtdq2ps      %xmm1,%xmm8
  .byte  243,15,16,74,120                    // movss         0x78(%rdx),%xmm1
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  102,15,110,82,112                   // movd          0x70(%rdx),%xmm2
  .byte  102,15,112,210,0                    // pshufd        $0x0,%xmm2,%xmm2
  .byte  102,65,15,219,209                   // pand          %xmm9,%xmm2
  .byte  68,15,91,194                        // cvtdq2ps      %xmm2,%xmm8
  .byte  243,15,16,82,124                    // movss         0x7c(%rdx),%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,19,4,36                          // movlps        %xmm0,(%rsp)
  .byte  76,141,4,120                        // lea           (%rax,%rdi,2),%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  102,67,139,4,72                     // mov           (%r8,%r9,2),%ax
  .byte  102,66,137,4,76                     // mov           %ax,(%rsp,%r9,2)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,238                             // jne           9cb <_sk_load_565_sse41+0x97>
  .byte  243,15,126,4,36                     // movq          (%rsp),%xmm0
  .byte  102,15,56,51,192                    // pmovzxwd      %xmm0,%xmm0
  .byte  233,89,255,255,255                  // jmpq          945 <_sk_load_565_sse41+0x11>

.globl _sk_store_565_sse41
_sk_store_565_sse41:
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  243,68,15,16,130,128,0,0,0          // movss         0x80(%rdx),%xmm8
  .byte  243,68,15,16,138,132,0,0,0          // movss         0x84(%rdx),%xmm9
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,40,208                        // movaps        %xmm8,%xmm10
  .byte  68,15,89,208                        // mulps         %xmm0,%xmm10
  .byte  102,69,15,91,210                    // cvtps2dq      %xmm10,%xmm10
  .byte  102,65,15,114,242,11                // pslld         $0xb,%xmm10
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  68,15,89,201                        // mulps         %xmm1,%xmm9
  .byte  102,69,15,91,201                    // cvtps2dq      %xmm9,%xmm9
  .byte  102,65,15,114,241,5                 // pslld         $0x5,%xmm9
  .byte  102,69,15,235,202                   // por           %xmm10,%xmm9
  .byte  68,15,89,194                        // mulps         %xmm2,%xmm8
  .byte  102,69,15,91,192                    // cvtps2dq      %xmm8,%xmm8
  .byte  102,69,15,86,193                    // orpd          %xmm9,%xmm8
  .byte  102,69,15,56,43,192                 // packusdw      %xmm8,%xmm8
  .byte  102,68,15,214,4,36                  // movq          %xmm8,(%rsp)
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,12                              // jne           a60 <_sk_store_565_sse41+0x74>
  .byte  242,69,15,17,4,120                  // movsd         %xmm8,(%r8,%rdi,2)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  102,77,15,126,193                   // movq          %xmm8,%r9
  .byte  102,69,137,12,120                   // mov           %r9w,(%r8,%rdi,2)
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,234                             // je            a5a <_sk_store_565_sse41+0x6e>
  .byte  76,137,200                          // mov           %r9,%rax
  .byte  72,193,232,16                       // shr           $0x10,%rax
  .byte  102,65,137,68,120,2                 // mov           %ax,0x2(%r8,%rdi,2)
  .byte  72,131,249,2                        // cmp           $0x2,%rcx
  .byte  116,215                             // je            a5a <_sk_store_565_sse41+0x6e>
  .byte  73,193,233,32                       // shr           $0x20,%r9
  .byte  102,69,137,76,120,4                 // mov           %r9w,0x4(%r8,%rdi,2)
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  116,199                             // je            a5a <_sk_store_565_sse41+0x6e>
  .byte  76,141,84,36,6                      // lea           0x6(%rsp),%r10
  .byte  65,185,3,0,0,0                      // mov           $0x3,%r9d
  .byte  73,41,201                           // sub           %rcx,%r9
  .byte  73,141,68,120,6                     // lea           0x6(%r8,%rdi,2),%rax
  .byte  102,69,139,2                        // mov           (%r10),%r8w
  .byte  73,131,194,2                        // add           $0x2,%r10
  .byte  102,68,137,0                        // mov           %r8w,(%rax)
  .byte  72,131,192,2                        // add           $0x2,%rax
  .byte  73,255,193                          // inc           %r9
  .byte  117,235                             // jne           aa6 <_sk_store_565_sse41+0xba>
  .byte  235,157                             // jmp           a5a <_sk_store_565_sse41+0x6e>

.globl _sk_load_8888_sse41
_sk_load_8888_sse41:
  .byte  72,131,236,24                       // sub           $0x18,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,97                              // jne           b2c <_sk_load_8888_sse41+0x6f>
  .byte  243,15,111,28,184                   // movdqu        (%rax,%rdi,4),%xmm3
  .byte  102,15,110,66,16                    // movd          0x10(%rdx),%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  102,15,111,203                      // movdqa        %xmm3,%xmm1
  .byte  102,15,114,209,8                    // psrld         $0x8,%xmm1
  .byte  102,15,219,200                      // pand          %xmm0,%xmm1
  .byte  102,15,111,211                      // movdqa        %xmm3,%xmm2
  .byte  102,15,114,210,16                   // psrld         $0x10,%xmm2
  .byte  102,15,219,208                      // pand          %xmm0,%xmm2
  .byte  102,15,219,195                      // pand          %xmm3,%xmm0
  .byte  15,91,192                           // cvtdq2ps      %xmm0,%xmm0
  .byte  243,68,15,16,66,12                  // movss         0xc(%rdx),%xmm8
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
  .byte  72,131,196,24                       // add           $0x18,%rsp
  .byte  255,224                             // jmpq          *%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,41,4,36                          // movaps        %xmm0,(%rsp)
  .byte  76,141,4,184                        // lea           (%rax,%rdi,4),%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  67,139,4,136                        // mov           (%r8,%r9,4),%eax
  .byte  66,137,4,140                        // mov           %eax,(%rsp,%r9,4)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,240                             // jne           b3a <_sk_load_8888_sse41+0x7d>
  .byte  102,15,111,28,36                    // movdqa        (%rsp),%xmm3
  .byte  233,124,255,255,255                 // jmpq          ad0 <_sk_load_8888_sse41+0x13>

.globl _sk_store_8888_sse41
_sk_store_8888_sse41:
  .byte  72,131,236,24                       // sub           $0x18,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  77,141,12,184                       // lea           (%r8,%rdi,4),%r9
  .byte  243,68,15,16,66,8                   // movss         0x8(%rdx),%xmm8
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
  .byte  102,68,15,127,4,36                  // movdqa        %xmm8,(%rsp)
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,24                              // jne           be0 <_sk_store_8888_sse41+0x8c>
  .byte  76,139,4,36                         // mov           (%rsp),%r8
  .byte  72,139,68,36,8                      // mov           0x8(%rsp),%rax
  .byte  73,137,65,8                         // mov           %rax,0x8(%r9)
  .byte  77,137,1                            // mov           %r8,(%r9)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,131,196,24                       // add           $0x18,%rsp
  .byte  255,224                             // jmpq          *%rax
  .byte  102,69,15,126,1                     // movd          %xmm8,(%r9)
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,237                             // je            bd8 <_sk_store_8888_sse41+0x84>
  .byte  102,68,15,58,22,192,1               // pextrd        $0x1,%xmm8,%eax
  .byte  65,137,65,4                         // mov           %eax,0x4(%r9)
  .byte  72,131,249,2                        // cmp           $0x2,%rcx
  .byte  116,220                             // je            bd8 <_sk_store_8888_sse41+0x84>
  .byte  102,68,15,58,22,192,2               // pextrd        $0x2,%xmm8,%eax
  .byte  65,137,65,8                         // mov           %eax,0x8(%r9)
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  116,203                             // je            bd8 <_sk_store_8888_sse41+0x84>
  .byte  76,141,84,36,12                     // lea           0xc(%rsp),%r10
  .byte  65,185,3,0,0,0                      // mov           $0x3,%r9d
  .byte  73,41,201                           // sub           %rcx,%r9
  .byte  73,141,68,184,12                    // lea           0xc(%r8,%rdi,4),%rax
  .byte  69,139,2                            // mov           (%r10),%r8d
  .byte  73,131,194,4                        // add           $0x4,%r10
  .byte  68,137,0                            // mov           %r8d,(%rax)
  .byte  72,131,192,4                        // add           $0x4,%rax
  .byte  73,255,193                          // inc           %r9
  .byte  117,237                             // jne           c20 <_sk_store_8888_sse41+0xcc>
  .byte  235,163                             // jmp           bd8 <_sk_store_8888_sse41+0x84>

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
  .byte  102,15,110,66,100                   // movd          0x64(%rdx),%xmm0
  .byte  102,15,112,216,0                    // pshufd        $0x0,%xmm0,%xmm3
  .byte  102,15,111,203                      // movdqa        %xmm3,%xmm1
  .byte  102,65,15,101,200                   // pcmpgtw       %xmm8,%xmm1
  .byte  102,65,15,223,200                   // pandn         %xmm8,%xmm1
  .byte  102,15,101,218                      // pcmpgtw       %xmm2,%xmm3
  .byte  102,15,223,218                      // pandn         %xmm2,%xmm3
  .byte  102,15,56,51,193                    // pmovzxwd      %xmm1,%xmm0
  .byte  102,15,114,240,13                   // pslld         $0xd,%xmm0
  .byte  102,15,110,82,92                    // movd          0x5c(%rdx),%xmm2
  .byte  102,68,15,112,194,0                 // pshufd        $0x0,%xmm2,%xmm8
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
  .byte  102,68,15,110,66,96                 // movd          0x60(%rdx),%xmm8
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

.globl _sk_start_pipeline_sse2
_sk_start_pipeline_sse2:
  .byte  65,87                               // push          %r15
  .byte  65,86                               // push          %r14
  .byte  65,85                               // push          %r13
  .byte  65,84                               // push          %r12
  .byte  83                                  // push          %rbx
  .byte  73,137,204                          // mov           %rcx,%r12
  .byte  73,137,214                          // mov           %rdx,%r14
  .byte  72,137,251                          // mov           %rdi,%rbx
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  73,137,247                          // mov           %rsi,%r15
  .byte  72,141,75,4                         // lea           0x4(%rbx),%rcx
  .byte  76,57,225                           // cmp           %r12,%rcx
  .byte  118,5                               // jbe           25 <_sk_start_pipeline_sse2+0x25>
  .byte  72,137,223                          // mov           %rbx,%rdi
  .byte  235,63                              // jmp           64 <_sk_start_pipeline_sse2+0x64>
  .byte  185,0,0,0,0                         // mov           $0x0,%ecx
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,87,201                           // xorps         %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  15,87,219                           // xorps         %xmm3,%xmm3
  .byte  15,87,228                           // xorps         %xmm4,%xmm4
  .byte  15,87,237                           // xorps         %xmm5,%xmm5
  .byte  15,87,246                           // xorps         %xmm6,%xmm6
  .byte  15,87,255                           // xorps         %xmm7,%xmm7
  .byte  72,137,223                          // mov           %rbx,%rdi
  .byte  76,137,254                          // mov           %r15,%rsi
  .byte  76,137,242                          // mov           %r14,%rdx
  .byte  73,137,197                          // mov           %rax,%r13
  .byte  65,255,213                          // callq         *%r13
  .byte  76,137,232                          // mov           %r13,%rax
  .byte  72,141,123,4                        // lea           0x4(%rbx),%rdi
  .byte  72,131,195,8                        // add           $0x8,%rbx
  .byte  76,57,227                           // cmp           %r12,%rbx
  .byte  72,137,251                          // mov           %rdi,%rbx
  .byte  118,193                             // jbe           25 <_sk_start_pipeline_sse2+0x25>
  .byte  76,57,231                           // cmp           %r12,%rdi
  .byte  117,10                              // jne           73 <_sk_start_pipeline_sse2+0x73>
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,93                               // pop           %r13
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  195                                 // retq
  .byte  73,41,252                           // sub           %rdi,%r12
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,87,201                           // xorps         %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  15,87,219                           // xorps         %xmm3,%xmm3
  .byte  15,87,228                           // xorps         %xmm4,%xmm4
  .byte  15,87,237                           // xorps         %xmm5,%xmm5
  .byte  15,87,246                           // xorps         %xmm6,%xmm6
  .byte  15,87,255                           // xorps         %xmm7,%xmm7
  .byte  76,137,254                          // mov           %r15,%rsi
  .byte  76,137,242                          // mov           %r14,%rdx
  .byte  76,137,225                          // mov           %r12,%rcx
  .byte  91                                  // pop           %rbx
  .byte  65,92                               // pop           %r12
  .byte  65,93                               // pop           %r13
  .byte  65,94                               // pop           %r14
  .byte  65,95                               // pop           %r15
  .byte  255,224                             // jmpq          *%rax

.globl _sk_just_return_sse2
_sk_just_return_sse2:
  .byte  195                                 // retq

.globl _sk_seed_shader_sse2
_sk_seed_shader_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  102,15,110,199                      // movd          %edi,%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  15,91,200                           // cvtdq2ps      %xmm0,%xmm1
  .byte  243,15,16,18                        // movss         (%rdx),%xmm2
  .byte  243,15,16,90,4                      // movss         0x4(%rdx),%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  15,88,203                           // addps         %xmm3,%xmm1
  .byte  15,16,66,20                         // movups        0x14(%rdx),%xmm0
  .byte  15,88,193                           // addps         %xmm1,%xmm0
  .byte  102,15,110,8                        // movd          (%rax),%xmm1
  .byte  102,15,112,201,0                    // pshufd        $0x0,%xmm1,%xmm1
  .byte  15,91,201                           // cvtdq2ps      %xmm1,%xmm1
  .byte  15,88,203                           // addps         %xmm3,%xmm1
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
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
  .byte  243,68,15,16,2                      // movss         (%rdx),%xmm8
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

.globl _sk_dstover_sse2
_sk_dstover_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,2                      // movss         (%rdx),%xmm8
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
  .byte  243,68,15,16,2                      // movss         (%rdx),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  65,15,93,192                        // minps         %xmm8,%xmm0
  .byte  65,15,93,200                        // minps         %xmm8,%xmm1
  .byte  65,15,93,208                        // minps         %xmm8,%xmm2
  .byte  65,15,93,216                        // minps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_clamp_a_sse2
_sk_clamp_a_sse2:
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  243,68,15,16,2                      // movss         (%rdx),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  65,15,93,216                        // minps         %xmm8,%xmm3
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
  .byte  243,68,15,16,10                     // movss         (%rdx),%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
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
  .byte  243,68,15,16,66,64                  // movss         0x40(%rdx),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,40,232                        // movaps        %xmm8,%xmm13
  .byte  68,15,89,232                        // mulps         %xmm0,%xmm13
  .byte  68,15,40,224                        // movaps        %xmm0,%xmm12
  .byte  69,15,89,228                        // mulps         %xmm12,%xmm12
  .byte  243,68,15,16,74,60                  // movss         0x3c(%rdx),%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  243,68,15,16,82,52                  // movss         0x34(%rdx),%xmm10
  .byte  243,68,15,16,90,56                  // movss         0x38(%rdx),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  69,15,40,241                        // movaps        %xmm9,%xmm14
  .byte  68,15,89,240                        // mulps         %xmm0,%xmm14
  .byte  69,15,88,243                        // addps         %xmm11,%xmm14
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  69,15,89,244                        // mulps         %xmm12,%xmm14
  .byte  69,15,88,242                        // addps         %xmm10,%xmm14
  .byte  243,68,15,16,98,68                  // movss         0x44(%rdx),%xmm12
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
  .byte  69,15,88,251                        // addps         %xmm11,%xmm15
  .byte  69,15,89,254                        // mulps         %xmm14,%xmm15
  .byte  69,15,88,250                        // addps         %xmm10,%xmm15
  .byte  65,15,194,204,1                     // cmpltps       %xmm12,%xmm1
  .byte  68,15,84,233                        // andps         %xmm1,%xmm13
  .byte  65,15,85,207                        // andnps        %xmm15,%xmm1
  .byte  65,15,86,205                        // orps          %xmm13,%xmm1
  .byte  68,15,89,194                        // mulps         %xmm2,%xmm8
  .byte  68,15,40,234                        // movaps        %xmm2,%xmm13
  .byte  69,15,89,237                        // mulps         %xmm13,%xmm13
  .byte  68,15,89,202                        // mulps         %xmm2,%xmm9
  .byte  69,15,88,203                        // addps         %xmm11,%xmm9
  .byte  69,15,89,205                        // mulps         %xmm13,%xmm9
  .byte  69,15,88,202                        // addps         %xmm10,%xmm9
  .byte  65,15,194,212,1                     // cmpltps       %xmm12,%xmm2
  .byte  68,15,84,194                        // andps         %xmm2,%xmm8
  .byte  65,15,85,209                        // andnps        %xmm9,%xmm2
  .byte  65,15,86,208                        // orps          %xmm8,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  255,224                             // jmpq          *%rax

.globl _sk_to_srgb_sse2
_sk_to_srgb_sse2:
  .byte  72,131,236,40                       // sub           $0x28,%rsp
  .byte  15,41,124,36,16                     // movaps        %xmm7,0x10(%rsp)
  .byte  15,41,52,36                         // movaps        %xmm6,(%rsp)
  .byte  15,40,245                           // movaps        %xmm5,%xmm6
  .byte  15,40,236                           // movaps        %xmm4,%xmm5
  .byte  15,40,227                           // movaps        %xmm3,%xmm4
  .byte  68,15,82,192                        // rsqrtps       %xmm0,%xmm8
  .byte  69,15,83,232                        // rcpps         %xmm8,%xmm13
  .byte  69,15,82,248                        // rsqrtps       %xmm8,%xmm15
  .byte  243,15,16,26                        // movss         (%rdx),%xmm3
  .byte  243,68,15,16,66,72                  // movss         0x48(%rdx),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,40,240                        // movaps        %xmm8,%xmm14
  .byte  68,15,89,240                        // mulps         %xmm0,%xmm14
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  243,68,15,16,82,76                  // movss         0x4c(%rdx),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  243,68,15,16,90,80                  // movss         0x50(%rdx),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  243,68,15,16,98,84                  // movss         0x54(%rdx),%xmm12
  .byte  69,15,198,228,0                     // shufps        $0x0,%xmm12,%xmm12
  .byte  69,15,89,235                        // mulps         %xmm11,%xmm13
  .byte  69,15,88,236                        // addps         %xmm12,%xmm13
  .byte  69,15,89,250                        // mulps         %xmm10,%xmm15
  .byte  69,15,88,253                        // addps         %xmm13,%xmm15
  .byte  68,15,40,203                        // movaps        %xmm3,%xmm9
  .byte  69,15,93,207                        // minps         %xmm15,%xmm9
  .byte  243,68,15,16,106,88                 // movss         0x58(%rdx),%xmm13
  .byte  69,15,198,237,0                     // shufps        $0x0,%xmm13,%xmm13
  .byte  65,15,194,197,1                     // cmpltps       %xmm13,%xmm0
  .byte  68,15,84,240                        // andps         %xmm0,%xmm14
  .byte  65,15,85,193                        // andnps        %xmm9,%xmm0
  .byte  65,15,86,198                        // orps          %xmm14,%xmm0
  .byte  68,15,82,201                        // rsqrtps       %xmm1,%xmm9
  .byte  69,15,83,241                        // rcpps         %xmm9,%xmm14
  .byte  69,15,82,201                        // rsqrtps       %xmm9,%xmm9
  .byte  69,15,89,243                        // mulps         %xmm11,%xmm14
  .byte  69,15,88,244                        // addps         %xmm12,%xmm14
  .byte  69,15,89,202                        // mulps         %xmm10,%xmm9
  .byte  69,15,88,206                        // addps         %xmm14,%xmm9
  .byte  68,15,40,243                        // movaps        %xmm3,%xmm14
  .byte  69,15,93,241                        // minps         %xmm9,%xmm14
  .byte  69,15,40,200                        // movaps        %xmm8,%xmm9
  .byte  68,15,89,201                        // mulps         %xmm1,%xmm9
  .byte  65,15,194,205,1                     // cmpltps       %xmm13,%xmm1
  .byte  68,15,84,201                        // andps         %xmm1,%xmm9
  .byte  65,15,85,206                        // andnps        %xmm14,%xmm1
  .byte  65,15,86,201                        // orps          %xmm9,%xmm1
  .byte  68,15,82,202                        // rsqrtps       %xmm2,%xmm9
  .byte  69,15,83,241                        // rcpps         %xmm9,%xmm14
  .byte  69,15,89,243                        // mulps         %xmm11,%xmm14
  .byte  69,15,88,244                        // addps         %xmm12,%xmm14
  .byte  65,15,82,249                        // rsqrtps       %xmm9,%xmm7
  .byte  65,15,89,250                        // mulps         %xmm10,%xmm7
  .byte  65,15,88,254                        // addps         %xmm14,%xmm7
  .byte  15,93,223                           // minps         %xmm7,%xmm3
  .byte  68,15,89,194                        // mulps         %xmm2,%xmm8
  .byte  65,15,194,213,1                     // cmpltps       %xmm13,%xmm2
  .byte  68,15,84,194                        // andps         %xmm2,%xmm8
  .byte  15,85,211                           // andnps        %xmm3,%xmm2
  .byte  65,15,86,208                        // orps          %xmm8,%xmm2
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,40,220                           // movaps        %xmm4,%xmm3
  .byte  15,40,229                           // movaps        %xmm5,%xmm4
  .byte  15,40,238                           // movaps        %xmm6,%xmm5
  .byte  15,40,52,36                         // movaps        (%rsp),%xmm6
  .byte  15,40,124,36,16                     // movaps        0x10(%rsp),%xmm7
  .byte  72,131,196,40                       // add           $0x28,%rsp
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
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,1,248                            // add           %rdi,%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,67                              // jne           521 <_sk_scale_u8_sse2+0x51>
  .byte  139,0                               // mov           (%rax),%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  102,69,15,239,201                   // pxor          %xmm9,%xmm9
  .byte  102,69,15,96,193                    // punpcklbw     %xmm9,%xmm8
  .byte  102,69,15,97,193                    // punpcklwd     %xmm9,%xmm8
  .byte  69,15,91,200                        // cvtdq2ps      %xmm8,%xmm9
  .byte  243,68,15,16,66,12                  // movss         0xc(%rdx),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,89,193                        // mulps         %xmm9,%xmm8
  .byte  65,15,89,192                        // mulps         %xmm8,%xmm0
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  68,15,89,195                        // mulps         %xmm3,%xmm8
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,15,40,216                        // movaps        %xmm8,%xmm3
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  76,141,68,36,4                      // lea           0x4(%rsp),%r8
  .byte  199,68,36,4,0,0,0,0                 // movl          $0x0,0x4(%rsp)
  .byte  73,137,201                          // mov           %rcx,%r9
  .byte  73,255,201                          // dec           %r9
  .byte  68,138,16                           // mov           (%rax),%r10b
  .byte  72,141,64,1                         // lea           0x1(%rax),%rax
  .byte  69,136,16                           // mov           %r10b,(%r8)
  .byte  77,141,64,1                         // lea           0x1(%r8),%r8
  .byte  117,237                             // jne           531 <_sk_scale_u8_sse2+0x61>
  .byte  139,68,36,4                         // mov           0x4(%rsp),%eax
  .byte  235,150                             // jmp           4e0 <_sk_scale_u8_sse2+0x10>

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
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,1,248                            // add           %rdi,%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,87                              // jne           5e7 <_sk_lerp_u8_sse2+0x65>
  .byte  139,0                               // mov           (%rax),%eax
  .byte  102,68,15,110,192                   // movd          %eax,%xmm8
  .byte  102,69,15,239,201                   // pxor          %xmm9,%xmm9
  .byte  102,69,15,96,193                    // punpcklbw     %xmm9,%xmm8
  .byte  102,69,15,97,193                    // punpcklwd     %xmm9,%xmm8
  .byte  69,15,91,192                        // cvtdq2ps      %xmm8,%xmm8
  .byte  243,68,15,16,74,12                  // movss         0xc(%rdx),%xmm9
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
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  76,141,68,36,4                      // lea           0x4(%rsp),%r8
  .byte  199,68,36,4,0,0,0,0                 // movl          $0x0,0x4(%rsp)
  .byte  73,137,201                          // mov           %rcx,%r9
  .byte  73,255,201                          // dec           %r9
  .byte  68,138,16                           // mov           (%rax),%r10b
  .byte  72,141,64,1                         // lea           0x1(%rax),%rax
  .byte  69,136,16                           // mov           %r10b,(%r8)
  .byte  77,141,64,1                         // lea           0x1(%r8),%r8
  .byte  117,237                             // jne           5f7 <_sk_lerp_u8_sse2+0x75>
  .byte  139,68,36,4                         // mov           0x4(%rsp),%eax
  .byte  235,130                             // jmp           592 <_sk_lerp_u8_sse2+0x10>

.globl _sk_lerp_565_sse2
_sk_lerp_565_sse2:
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,170,0,0,0                    // jne           6c9 <_sk_lerp_565_sse2+0xb9>
  .byte  243,68,15,126,4,120                 // movq          (%rax,%rdi,2),%xmm8
  .byte  102,68,15,97,192                    // punpcklwd     %xmm0,%xmm8
  .byte  102,68,15,219,5,125,11,0,0          // pand          0xb7d(%rip),%xmm8        # 11b0 <_sk_linear_gradient_2stops_sse2+0x68>
  .byte  102,15,110,90,104                   // movd          0x68(%rdx),%xmm3
  .byte  102,15,112,219,0                    // pshufd        $0x0,%xmm3,%xmm3
  .byte  102,65,15,219,216                   // pand          %xmm8,%xmm3
  .byte  68,15,91,203                        // cvtdq2ps      %xmm3,%xmm9
  .byte  243,15,16,26                        // movss         (%rdx),%xmm3
  .byte  243,68,15,16,82,116                 // movss         0x74(%rdx),%xmm10
  .byte  69,15,198,210,0                     // shufps        $0x0,%xmm10,%xmm10
  .byte  69,15,89,209                        // mulps         %xmm9,%xmm10
  .byte  102,68,15,110,74,108                // movd          0x6c(%rdx),%xmm9
  .byte  102,69,15,112,201,0                 // pshufd        $0x0,%xmm9,%xmm9
  .byte  102,69,15,219,200                   // pand          %xmm8,%xmm9
  .byte  69,15,91,201                        // cvtdq2ps      %xmm9,%xmm9
  .byte  243,68,15,16,90,120                 // movss         0x78(%rdx),%xmm11
  .byte  69,15,198,219,0                     // shufps        $0x0,%xmm11,%xmm11
  .byte  69,15,89,217                        // mulps         %xmm9,%xmm11
  .byte  102,68,15,110,74,112                // movd          0x70(%rdx),%xmm9
  .byte  102,69,15,112,201,0                 // pshufd        $0x0,%xmm9,%xmm9
  .byte  102,69,15,219,200                   // pand          %xmm8,%xmm9
  .byte  69,15,91,193                        // cvtdq2ps      %xmm9,%xmm8
  .byte  243,68,15,16,74,124                 // movss         0x7c(%rdx),%xmm9
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  69,15,89,200                        // mulps         %xmm8,%xmm9
  .byte  15,92,196                           // subps         %xmm4,%xmm0
  .byte  65,15,89,194                        // mulps         %xmm10,%xmm0
  .byte  15,88,196                           // addps         %xmm4,%xmm0
  .byte  15,92,205                           // subps         %xmm5,%xmm1
  .byte  65,15,89,203                        // mulps         %xmm11,%xmm1
  .byte  15,88,205                           // addps         %xmm5,%xmm1
  .byte  15,92,214                           // subps         %xmm6,%xmm2
  .byte  65,15,89,209                        // mulps         %xmm9,%xmm2
  .byte  15,88,214                           // addps         %xmm6,%xmm2
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  15,87,219                           // xorps         %xmm3,%xmm3
  .byte  15,19,28,36                         // movlps        %xmm3,(%rsp)
  .byte  76,141,4,120                        // lea           (%rax,%rdi,2),%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  102,67,139,4,72                     // mov           (%r8,%r9,2),%ax
  .byte  102,66,137,4,76                     // mov           %ax,(%rsp,%r9,2)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,238                             // jne           6d7 <_sk_lerp_565_sse2+0xc7>
  .byte  243,68,15,126,4,36                  // movq          (%rsp),%xmm8
  .byte  233,49,255,255,255                  // jmpq          625 <_sk_lerp_565_sse2+0x15>

.globl _sk_load_tables_sse2
_sk_load_tables_sse2:
  .byte  65,86                               // push          %r14
  .byte  83                                  // push          %rbx
  .byte  72,131,236,24                       // sub           $0x18,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,32,1,0,0                     // jne           829 <_sk_load_tables_sse2+0x135>
  .byte  243,69,15,111,4,184                 // movdqu        (%r8,%rdi,4),%xmm8
  .byte  102,15,110,66,16                    // movd          0x10(%rdx),%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  102,69,15,111,200                   // movdqa        %xmm8,%xmm9
  .byte  102,65,15,114,209,8                 // psrld         $0x8,%xmm9
  .byte  102,68,15,219,200                   // pand          %xmm0,%xmm9
  .byte  102,69,15,111,208                   // movdqa        %xmm8,%xmm10
  .byte  102,65,15,114,210,16                // psrld         $0x10,%xmm10
  .byte  102,68,15,219,208                   // pand          %xmm0,%xmm10
  .byte  102,65,15,219,192                   // pand          %xmm8,%xmm0
  .byte  102,15,112,216,78                   // pshufd        $0x4e,%xmm0,%xmm3
  .byte  102,73,15,126,216                   // movq          %xmm3,%r8
  .byte  69,137,193                          // mov           %r8d,%r9d
  .byte  73,193,232,32                       // shr           $0x20,%r8
  .byte  102,73,15,126,194                   // movq          %xmm0,%r10
  .byte  69,137,211                          // mov           %r10d,%r11d
  .byte  73,193,234,32                       // shr           $0x20,%r10
  .byte  72,139,88,8                         // mov           0x8(%rax),%rbx
  .byte  76,139,112,16                       // mov           0x10(%rax),%r14
  .byte  243,66,15,16,28,147                 // movss         (%rbx,%r10,4),%xmm3
  .byte  243,66,15,16,4,131                  // movss         (%rbx,%r8,4),%xmm0
  .byte  15,20,216                           // unpcklps      %xmm0,%xmm3
  .byte  243,66,15,16,4,155                  // movss         (%rbx,%r11,4),%xmm0
  .byte  243,66,15,16,12,139                 // movss         (%rbx,%r9,4),%xmm1
  .byte  15,20,193                           // unpcklps      %xmm1,%xmm0
  .byte  15,20,195                           // unpcklps      %xmm3,%xmm0
  .byte  102,65,15,112,201,78                // pshufd        $0x4e,%xmm9,%xmm1
  .byte  102,72,15,126,203                   // movq          %xmm1,%rbx
  .byte  65,137,216                          // mov           %ebx,%r8d
  .byte  72,193,235,32                       // shr           $0x20,%rbx
  .byte  102,77,15,126,201                   // movq          %xmm9,%r9
  .byte  69,137,202                          // mov           %r9d,%r10d
  .byte  73,193,233,32                       // shr           $0x20,%r9
  .byte  243,67,15,16,28,142                 // movss         (%r14,%r9,4),%xmm3
  .byte  243,65,15,16,12,158                 // movss         (%r14,%rbx,4),%xmm1
  .byte  15,20,217                           // unpcklps      %xmm1,%xmm3
  .byte  243,67,15,16,12,150                 // movss         (%r14,%r10,4),%xmm1
  .byte  243,67,15,16,20,134                 // movss         (%r14,%r8,4),%xmm2
  .byte  15,20,202                           // unpcklps      %xmm2,%xmm1
  .byte  15,20,203                           // unpcklps      %xmm3,%xmm1
  .byte  72,139,64,24                        // mov           0x18(%rax),%rax
  .byte  102,65,15,112,210,78                // pshufd        $0x4e,%xmm10,%xmm2
  .byte  102,72,15,126,211                   // movq          %xmm2,%rbx
  .byte  65,137,216                          // mov           %ebx,%r8d
  .byte  72,193,235,32                       // shr           $0x20,%rbx
  .byte  102,77,15,126,209                   // movq          %xmm10,%r9
  .byte  69,137,202                          // mov           %r9d,%r10d
  .byte  73,193,233,32                       // shr           $0x20,%r9
  .byte  243,70,15,16,12,136                 // movss         (%rax,%r9,4),%xmm9
  .byte  243,15,16,20,152                    // movss         (%rax,%rbx,4),%xmm2
  .byte  68,15,20,202                        // unpcklps      %xmm2,%xmm9
  .byte  243,66,15,16,20,144                 // movss         (%rax,%r10,4),%xmm2
  .byte  243,66,15,16,28,128                 // movss         (%rax,%r8,4),%xmm3
  .byte  15,20,211                           // unpcklps      %xmm3,%xmm2
  .byte  65,15,20,209                        // unpcklps      %xmm9,%xmm2
  .byte  102,65,15,114,208,24                // psrld         $0x18,%xmm8
  .byte  69,15,91,192                        // cvtdq2ps      %xmm8,%xmm8
  .byte  243,15,16,90,12                     // movss         0xc(%rdx),%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  65,15,89,216                        // mulps         %xmm8,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,131,196,24                       // add           $0x18,%rsp
  .byte  91                                  // pop           %rbx
  .byte  65,94                               // pop           %r14
  .byte  255,224                             // jmpq          *%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,41,4,36                          // movaps        %xmm0,(%rsp)
  .byte  77,141,4,184                        // lea           (%r8,%rdi,4),%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  71,139,20,136                       // mov           (%r8,%r9,4),%r10d
  .byte  70,137,20,140                       // mov           %r10d,(%rsp,%r9,4)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,240                             // jne           837 <_sk_load_tables_sse2+0x143>
  .byte  102,68,15,111,4,36                  // movdqa        (%rsp),%xmm8
  .byte  233,189,254,255,255                 // jmpq          70f <_sk_load_tables_sse2+0x1b>

.globl _sk_load_a8_sse2
_sk_load_a8_sse2:
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,1,248                            // add           %rdi,%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,49                              // jne           891 <_sk_load_a8_sse2+0x3f>
  .byte  139,0                               // mov           (%rax),%eax
  .byte  102,15,110,192                      // movd          %eax,%xmm0
  .byte  102,15,239,201                      // pxor          %xmm1,%xmm1
  .byte  102,15,96,193                       // punpcklbw     %xmm1,%xmm0
  .byte  102,15,97,193                       // punpcklwd     %xmm1,%xmm0
  .byte  15,91,192                           // cvtdq2ps      %xmm0,%xmm0
  .byte  243,15,16,90,12                     // movss         0xc(%rdx),%xmm3
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  15,89,216                           // mulps         %xmm0,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  102,15,239,201                      // pxor          %xmm1,%xmm1
  .byte  15,87,210                           // xorps         %xmm2,%xmm2
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  76,141,68,36,4                      // lea           0x4(%rsp),%r8
  .byte  199,68,36,4,0,0,0,0                 // movl          $0x0,0x4(%rsp)
  .byte  73,137,201                          // mov           %rcx,%r9
  .byte  73,255,201                          // dec           %r9
  .byte  68,138,16                           // mov           (%rax),%r10b
  .byte  72,141,64,1                         // lea           0x1(%rax),%rax
  .byte  69,136,16                           // mov           %r10b,(%r8)
  .byte  77,141,64,1                         // lea           0x1(%r8),%r8
  .byte  117,237                             // jne           8a1 <_sk_load_a8_sse2+0x4f>
  .byte  139,68,36,4                         // mov           0x4(%rsp),%eax
  .byte  235,168                             // jmp           862 <_sk_load_a8_sse2+0x10>

.globl _sk_store_a8_sse2
_sk_store_a8_sse2:
  .byte  72,131,236,4                        // sub           $0x4,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  243,68,15,16,66,8                   // movss         0x8(%rdx),%xmm8
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  68,15,89,195                        // mulps         %xmm3,%xmm8
  .byte  102,69,15,91,192                    // cvtps2dq      %xmm8,%xmm8
  .byte  102,65,15,114,240,16                // pslld         $0x10,%xmm8
  .byte  102,65,15,114,224,16                // psrad         $0x10,%xmm8
  .byte  102,69,15,107,192                   // packssdw      %xmm8,%xmm8
  .byte  102,69,15,103,192                   // packuswb      %xmm8,%xmm8
  .byte  102,69,15,126,193                   // movd          %xmm8,%r9d
  .byte  102,68,15,126,4,36                  // movd          %xmm8,(%rsp)
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,12                              // jne           909 <_sk_store_a8_sse2+0x4f>
  .byte  69,137,12,56                        // mov           %r9d,(%r8,%rdi,1)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,131,196,4                        // add           $0x4,%rsp
  .byte  255,224                             // jmpq          *%rax
  .byte  69,136,12,56                        // mov           %r9b,(%r8,%rdi,1)
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,238                             // je            901 <_sk_store_a8_sse2+0x47>
  .byte  68,137,200                          // mov           %r9d,%eax
  .byte  193,232,8                           // shr           $0x8,%eax
  .byte  65,136,68,56,1                      // mov           %al,0x1(%r8,%rdi,1)
  .byte  72,131,249,2                        // cmp           $0x2,%rcx
  .byte  116,221                             // je            901 <_sk_store_a8_sse2+0x47>
  .byte  65,193,233,16                       // shr           $0x10,%r9d
  .byte  69,136,76,56,2                      // mov           %r9b,0x2(%r8,%rdi,1)
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  116,206                             // je            901 <_sk_store_a8_sse2+0x47>
  .byte  76,141,84,36,3                      // lea           0x3(%rsp),%r10
  .byte  65,185,3,0,0,0                      // mov           $0x3,%r9d
  .byte  73,41,201                           // sub           %rcx,%r9
  .byte  74,141,68,7,3                       // lea           0x3(%rdi,%r8,1),%rax
  .byte  69,138,2                            // mov           (%r10),%r8b
  .byte  73,255,194                          // inc           %r10
  .byte  68,136,0                            // mov           %r8b,(%rax)
  .byte  72,255,192                          // inc           %rax
  .byte  73,255,193                          // inc           %r9
  .byte  117,239                             // jne           946 <_sk_store_a8_sse2+0x8c>
  .byte  235,168                             // jmp           901 <_sk_store_a8_sse2+0x47>

.globl _sk_load_565_sse2
_sk_load_565_sse2:
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  15,133,128,0,0,0                    // jne           9e8 <_sk_load_565_sse2+0x8f>
  .byte  243,68,15,126,12,120                // movq          (%rax,%rdi,2),%xmm9
  .byte  102,68,15,97,200                    // punpcklwd     %xmm0,%xmm9
  .byte  102,68,15,219,13,68,8,0,0           // pand          0x844(%rip),%xmm9        # 11c0 <_sk_linear_gradient_2stops_sse2+0x78>
  .byte  102,15,110,66,104                   // movd          0x68(%rdx),%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  102,65,15,219,193                   // pand          %xmm9,%xmm0
  .byte  15,91,200                           // cvtdq2ps      %xmm0,%xmm1
  .byte  243,15,16,26                        // movss         (%rdx),%xmm3
  .byte  243,15,16,66,116                    // movss         0x74(%rdx),%xmm0
  .byte  15,198,192,0                        // shufps        $0x0,%xmm0,%xmm0
  .byte  15,89,193                           // mulps         %xmm1,%xmm0
  .byte  102,15,110,74,108                   // movd          0x6c(%rdx),%xmm1
  .byte  102,15,112,201,0                    // pshufd        $0x0,%xmm1,%xmm1
  .byte  102,65,15,219,201                   // pand          %xmm9,%xmm1
  .byte  68,15,91,193                        // cvtdq2ps      %xmm1,%xmm8
  .byte  243,15,16,74,120                    // movss         0x78(%rdx),%xmm1
  .byte  15,198,201,0                        // shufps        $0x0,%xmm1,%xmm1
  .byte  65,15,89,200                        // mulps         %xmm8,%xmm1
  .byte  102,15,110,82,112                   // movd          0x70(%rdx),%xmm2
  .byte  102,15,112,210,0                    // pshufd        $0x0,%xmm2,%xmm2
  .byte  102,65,15,219,209                   // pand          %xmm9,%xmm2
  .byte  68,15,91,194                        // cvtdq2ps      %xmm2,%xmm8
  .byte  243,15,16,82,124                    // movss         0x7c(%rdx),%xmm2
  .byte  15,198,210,0                        // shufps        $0x0,%xmm2,%xmm2
  .byte  65,15,89,208                        // mulps         %xmm8,%xmm2
  .byte  15,198,219,0                        // shufps        $0x0,%xmm3,%xmm3
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  102,15,239,192                      // pxor          %xmm0,%xmm0
  .byte  102,15,214,4,36                     // movq          %xmm0,(%rsp)
  .byte  76,141,4,120                        // lea           (%rax,%rdi,2),%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  102,67,139,4,72                     // mov           (%r8,%r9,2),%ax
  .byte  102,66,137,4,76                     // mov           %ax,(%rsp,%r9,2)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,238                             // jne           9f8 <_sk_load_565_sse2+0x9f>
  .byte  243,68,15,126,12,36                 // movq          (%rsp),%xmm9
  .byte  233,89,255,255,255                  // jmpq          96e <_sk_load_565_sse2+0x15>

.globl _sk_store_565_sse2
_sk_store_565_sse2:
  .byte  80                                  // push          %rax
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  243,68,15,16,130,128,0,0,0          // movss         0x80(%rdx),%xmm8
  .byte  243,68,15,16,138,132,0,0,0          // movss         0x84(%rdx),%xmm9
  .byte  69,15,198,192,0                     // shufps        $0x0,%xmm8,%xmm8
  .byte  69,15,40,208                        // movaps        %xmm8,%xmm10
  .byte  68,15,89,208                        // mulps         %xmm0,%xmm10
  .byte  102,69,15,91,210                    // cvtps2dq      %xmm10,%xmm10
  .byte  102,65,15,114,242,11                // pslld         $0xb,%xmm10
  .byte  69,15,198,201,0                     // shufps        $0x0,%xmm9,%xmm9
  .byte  68,15,89,201                        // mulps         %xmm1,%xmm9
  .byte  102,69,15,91,201                    // cvtps2dq      %xmm9,%xmm9
  .byte  102,65,15,114,241,5                 // pslld         $0x5,%xmm9
  .byte  102,69,15,235,202                   // por           %xmm10,%xmm9
  .byte  68,15,89,194                        // mulps         %xmm2,%xmm8
  .byte  102,69,15,91,192                    // cvtps2dq      %xmm8,%xmm8
  .byte  102,69,15,86,193                    // orpd          %xmm9,%xmm8
  .byte  102,65,15,114,240,16                // pslld         $0x10,%xmm8
  .byte  102,65,15,114,224,16                // psrad         $0x10,%xmm8
  .byte  102,69,15,107,192                   // packssdw      %xmm8,%xmm8
  .byte  102,68,15,214,4,36                  // movq          %xmm8,(%rsp)
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,12                              // jne           a94 <_sk_store_565_sse2+0x7f>
  .byte  242,69,15,17,4,120                  // movsd         %xmm8,(%r8,%rdi,2)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  65,88                               // pop           %r8
  .byte  255,224                             // jmpq          *%rax
  .byte  102,77,15,126,193                   // movq          %xmm8,%r9
  .byte  102,69,137,12,120                   // mov           %r9w,(%r8,%rdi,2)
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,234                             // je            a8e <_sk_store_565_sse2+0x79>
  .byte  76,137,200                          // mov           %r9,%rax
  .byte  72,193,232,16                       // shr           $0x10,%rax
  .byte  102,65,137,68,120,2                 // mov           %ax,0x2(%r8,%rdi,2)
  .byte  72,131,249,2                        // cmp           $0x2,%rcx
  .byte  116,215                             // je            a8e <_sk_store_565_sse2+0x79>
  .byte  73,193,233,32                       // shr           $0x20,%r9
  .byte  102,69,137,76,120,4                 // mov           %r9w,0x4(%r8,%rdi,2)
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  116,199                             // je            a8e <_sk_store_565_sse2+0x79>
  .byte  76,141,84,36,6                      // lea           0x6(%rsp),%r10
  .byte  65,185,3,0,0,0                      // mov           $0x3,%r9d
  .byte  73,41,201                           // sub           %rcx,%r9
  .byte  73,141,68,120,6                     // lea           0x6(%r8,%rdi,2),%rax
  .byte  102,69,139,2                        // mov           (%r10),%r8w
  .byte  73,131,194,2                        // add           $0x2,%r10
  .byte  102,68,137,0                        // mov           %r8w,(%rax)
  .byte  72,131,192,2                        // add           $0x2,%rax
  .byte  73,255,193                          // inc           %r9
  .byte  117,235                             // jne           ada <_sk_store_565_sse2+0xc5>
  .byte  235,157                             // jmp           a8e <_sk_store_565_sse2+0x79>

.globl _sk_load_8888_sse2
_sk_load_8888_sse2:
  .byte  72,131,236,24                       // sub           $0x18,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,139,0                            // mov           (%rax),%rax
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,97                              // jne           b60 <_sk_load_8888_sse2+0x6f>
  .byte  243,15,111,28,184                   // movdqu        (%rax,%rdi,4),%xmm3
  .byte  102,15,110,66,16                    // movd          0x10(%rdx),%xmm0
  .byte  102,15,112,192,0                    // pshufd        $0x0,%xmm0,%xmm0
  .byte  102,15,111,203                      // movdqa        %xmm3,%xmm1
  .byte  102,15,114,209,8                    // psrld         $0x8,%xmm1
  .byte  102,15,219,200                      // pand          %xmm0,%xmm1
  .byte  102,15,111,211                      // movdqa        %xmm3,%xmm2
  .byte  102,15,114,210,16                   // psrld         $0x10,%xmm2
  .byte  102,15,219,208                      // pand          %xmm0,%xmm2
  .byte  102,15,219,195                      // pand          %xmm3,%xmm0
  .byte  15,91,192                           // cvtdq2ps      %xmm0,%xmm0
  .byte  243,68,15,16,66,12                  // movss         0xc(%rdx),%xmm8
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
  .byte  72,131,196,24                       // add           $0x18,%rsp
  .byte  255,224                             // jmpq          *%rax
  .byte  15,87,192                           // xorps         %xmm0,%xmm0
  .byte  15,41,4,36                          // movaps        %xmm0,(%rsp)
  .byte  76,141,4,184                        // lea           (%rax,%rdi,4),%r8
  .byte  69,49,201                           // xor           %r9d,%r9d
  .byte  67,139,4,136                        // mov           (%r8,%r9,4),%eax
  .byte  66,137,4,140                        // mov           %eax,(%rsp,%r9,4)
  .byte  73,255,193                          // inc           %r9
  .byte  76,57,201                           // cmp           %r9,%rcx
  .byte  117,240                             // jne           b6e <_sk_load_8888_sse2+0x7d>
  .byte  102,15,111,28,36                    // movdqa        (%rsp),%xmm3
  .byte  233,124,255,255,255                 // jmpq          b04 <_sk_load_8888_sse2+0x13>

.globl _sk_store_8888_sse2
_sk_store_8888_sse2:
  .byte  72,131,236,24                       // sub           $0x18,%rsp
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  76,139,0                            // mov           (%rax),%r8
  .byte  77,141,12,184                       // lea           (%r8,%rdi,4),%r9
  .byte  243,68,15,16,66,8                   // movss         0x8(%rdx),%xmm8
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
  .byte  102,68,15,127,4,36                  // movdqa        %xmm8,(%rsp)
  .byte  72,133,201                          // test          %rcx,%rcx
  .byte  117,24                              // jne           c14 <_sk_store_8888_sse2+0x8c>
  .byte  76,139,4,36                         // mov           (%rsp),%r8
  .byte  72,139,68,36,8                      // mov           0x8(%rsp),%rax
  .byte  73,137,65,8                         // mov           %rax,0x8(%r9)
  .byte  77,137,1                            // mov           %r8,(%r9)
  .byte  72,173                              // lods          %ds:(%rsi),%rax
  .byte  72,131,196,24                       // add           $0x18,%rsp
  .byte  255,224                             // jmpq          *%rax
  .byte  102,69,15,126,1                     // movd          %xmm8,(%r9)
  .byte  72,131,249,1                        // cmp           $0x1,%rcx
  .byte  116,237                             // je            c0c <_sk_store_8888_sse2+0x84>
  .byte  102,69,15,112,200,229               // pshufd        $0xe5,%xmm8,%xmm9
  .byte  102,68,15,126,200                   // movd          %xmm9,%eax
  .byte  65,137,65,4                         // mov           %eax,0x4(%r9)
  .byte  72,131,249,2                        // cmp           $0x2,%rcx
  .byte  116,216                             // je            c0c <_sk_store_8888_sse2+0x84>
  .byte  102,69,15,112,192,78                // pshufd        $0x4e,%xmm8,%xmm8
  .byte  102,68,15,126,192                   // movd          %xmm8,%eax
  .byte  65,137,65,8                         // mov           %eax,0x8(%r9)
  .byte  72,131,249,3                        // cmp           $0x3,%rcx
  .byte  116,195                             // je            c0c <_sk_store_8888_sse2+0x84>
  .byte  76,141,84,36,12                     // lea           0xc(%rsp),%r10
  .byte  65,185,3,0,0,0                      // mov           $0x3,%r9d
  .byte  73,41,201                           // sub           %rcx,%r9
  .byte  73,141,68,184,12                    // lea           0xc(%r8,%rdi,4),%rax
  .byte  69,139,2                            // mov           (%r10),%r8d
  .byte  73,131,194,4                        // add           $0x4,%r10
  .byte  68,137,0                            // mov           %r8d,(%rax)
  .byte  72,131,192,4                        // add           $0x4,%rax
  .byte  73,255,193                          // inc           %r9
  .byte  117,237                             // jne           c5c <_sk_store_8888_sse2+0xd4>
  .byte  235,155                             // jmp           c0c <_sk_store_8888_sse2+0x84>

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
  .byte  102,15,110,66,100                   // movd          0x64(%rdx),%xmm0
  .byte  102,15,112,216,0                    // pshufd        $0x0,%xmm0,%xmm3
  .byte  102,15,111,203                      // movdqa        %xmm3,%xmm1
  .byte  102,65,15,101,200                   // pcmpgtw       %xmm8,%xmm1
  .byte  102,65,15,223,200                   // pandn         %xmm8,%xmm1
  .byte  102,15,101,218                      // pcmpgtw       %xmm2,%xmm3
  .byte  102,15,223,218                      // pandn         %xmm2,%xmm3
  .byte  102,69,15,239,192                   // pxor          %xmm8,%xmm8
  .byte  102,15,111,193                      // movdqa        %xmm1,%xmm0
  .byte  102,65,15,97,192                    // punpcklwd     %xmm8,%xmm0
  .byte  102,15,114,240,13                   // pslld         $0xd,%xmm0
  .byte  102,15,110,82,92                    // movd          0x5c(%rdx),%xmm2
  .byte  102,68,15,112,202,0                 // pshufd        $0x0,%xmm2,%xmm9
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
  .byte  102,68,15,110,66,96                 // movd          0x60(%rdx),%xmm8
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
  .byte  243,68,15,16,26                     // movss         (%rdx),%xmm11
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
  .byte  243,68,15,16,26                     // movss         (%rdx),%xmm11
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
  .byte  243,68,15,16,34                     // movss         (%rdx),%xmm12
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
  .byte  243,68,15,16,34                     // movss         (%rdx),%xmm12
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
#endif
