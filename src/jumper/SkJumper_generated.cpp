/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This file is generated semi-automatically with this command:
//   $ src/jumper/build_stages.py

#include <stdint.h>

#if defined(_MSC_VER)
    #pragma section("code", read,execute)
    #define CODE extern "C" __declspec(allocate("code"))
#elif defined(__MACH__)
    #define CODE extern "C" __attribute__((section("__TEXT,__text")))
#else
    #define CODE extern "C" __attribute__((section(".text")))
#endif

#if defined(__aarch64__)

CODE const uint32_t sk_start_pipeline_aarch64[] = {
  0xa9bd5bf7,                             //stp           x23, x22, [sp, #-48]!
  0xa90153f5,                             //stp           x21, x20, [sp, #16]
  0xa9027bf3,                             //stp           x19, x30, [sp, #32]
  0xaa0103f5,                             //mov           x21, x1
  0xf84086b7,                             //ldr           x23, [x21], #8
  0xaa0003f6,                             //mov           x22, x0
  0xaa0303f3,                             //mov           x19, x3
  0xaa0203f4,                             //mov           x20, x2
  0x910012c8,                             //add           x8, x22, #0x4
  0xeb13011f,                             //cmp           x8, x19
  0x54000069,                             //b.ls          34 <sk_start_pipeline_aarch64+0x34>  // b.plast
  0xaa1603e0,                             //mov           x0, x22
  0x14000012,                             //b             78 <sk_start_pipeline_aarch64+0x78>
  0x6f00e400,                             //movi          v0.2d, #0x0
  0x6f00e401,                             //movi          v1.2d, #0x0
  0x6f00e402,                             //movi          v2.2d, #0x0
  0x6f00e403,                             //movi          v3.2d, #0x0
  0x6f00e404,                             //movi          v4.2d, #0x0
  0x6f00e405,                             //movi          v5.2d, #0x0
  0x6f00e406,                             //movi          v6.2d, #0x0
  0x6f00e407,                             //movi          v7.2d, #0x0
  0xaa1603e0,                             //mov           x0, x22
  0xaa1503e1,                             //mov           x1, x21
  0xaa1403e2,                             //mov           x2, x20
  0xd63f02e0,                             //blr           x23
  0x910022c8,                             //add           x8, x22, #0x8
  0x910012c0,                             //add           x0, x22, #0x4
  0xeb13011f,                             //cmp           x8, x19
  0xaa0003f6,                             //mov           x22, x0
  0x54fffe09,                             //b.ls          34 <sk_start_pipeline_aarch64+0x34>  // b.plast
  0xa9427bf3,                             //ldp           x19, x30, [sp, #32]
  0xa94153f5,                             //ldp           x21, x20, [sp, #16]
  0xa8c35bf7,                             //ldp           x23, x22, [sp], #48
  0xd65f03c0,                             //ret
};

CODE const uint32_t sk_just_return_aarch64[] = {
  0xd65f03c0,                             //ret
};

CODE const uint32_t sk_seed_shader_aarch64[] = {
  0xaa0203e9,                             //mov           x9, x2
  0xa9400c28,                             //ldp           x8, x3, [x1]
  0x4ddfc922,                             //ld1r          {v2.4s}, [x9], #4
  0x3cc14047,                             //ldur          q7, [x2, #20]
  0x4e040c00,                             //dup           v0.4s, w0
  0x4d40c901,                             //ld1r          {v1.4s}, [x8]
  0x4d40c926,                             //ld1r          {v6.4s}, [x9]
  0x4e21d800,                             //scvtf         v0.4s, v0.4s
  0x91004028,                             //add           x8, x1, #0x10
  0x4e21d821,                             //scvtf         v1.4s, v1.4s
  0x4e26d400,                             //fadd          v0.4s, v0.4s, v6.4s
  0x6f00e403,                             //movi          v3.2d, #0x0
  0x6f00e404,                             //movi          v4.2d, #0x0
  0x6f00e405,                             //movi          v5.2d, #0x0
  0x4e26d421,                             //fadd          v1.4s, v1.4s, v6.4s
  0x6f00e406,                             //movi          v6.2d, #0x0
  0x4e20d4e0,                             //fadd          v0.4s, v7.4s, v0.4s
  0x6f00e407,                             //movi          v7.2d, #0x0
  0xaa0803e1,                             //mov           x1, x8
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_constant_color_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0x3dc00103,                             //ldr           q3, [x8]
  0x4e040460,                             //dup           v0.4s, v3.s[0]
  0x4e0c0461,                             //dup           v1.4s, v3.s[1]
  0x4e140462,                             //dup           v2.4s, v3.s[2]
  0x4e1c0463,                             //dup           v3.4s, v3.s[3]
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_clear_aarch64[] = {
  0xf8408423,                             //ldr           x3, [x1], #8
  0x6f00e400,                             //movi          v0.2d, #0x0
  0x6f00e401,                             //movi          v1.2d, #0x0
  0x6f00e402,                             //movi          v2.2d, #0x0
  0x6f00e403,                             //movi          v3.2d, #0x0
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_plus__aarch64[] = {
  0xf8408423,                             //ldr           x3, [x1], #8
  0x4e24d400,                             //fadd          v0.4s, v0.4s, v4.4s
  0x4e25d421,                             //fadd          v1.4s, v1.4s, v5.4s
  0x4e26d442,                             //fadd          v2.4s, v2.4s, v6.4s
  0x4e27d463,                             //fadd          v3.4s, v3.4s, v7.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_srcover_aarch64[] = {
  0x4d40c850,                             //ld1r          {v16.4s}, [x2]
  0xf8408423,                             //ldr           x3, [x1], #8
  0x4ea3d610,                             //fsub          v16.4s, v16.4s, v3.4s
  0x4e24ce00,                             //fmla          v0.4s, v16.4s, v4.4s
  0x4e25ce01,                             //fmla          v1.4s, v16.4s, v5.4s
  0x4e26ce02,                             //fmla          v2.4s, v16.4s, v6.4s
  0x4e27ce03,                             //fmla          v3.4s, v16.4s, v7.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_dstover_aarch64[] = {
  0x4d40c851,                             //ld1r          {v17.4s}, [x2]
  0xf8408423,                             //ldr           x3, [x1], #8
  0x4ea41c90,                             //mov           v16.16b, v4.16b
  0x4ea61cd2,                             //mov           v18.16b, v6.16b
  0x4ea7d634,                             //fsub          v20.4s, v17.4s, v7.4s
  0x4ea51cb1,                             //mov           v17.16b, v5.16b
  0x4ea71cf3,                             //mov           v19.16b, v7.16b
  0x4e20ce90,                             //fmla          v16.4s, v20.4s, v0.4s
  0x4e21ce91,                             //fmla          v17.4s, v20.4s, v1.4s
  0x4e22ce92,                             //fmla          v18.4s, v20.4s, v2.4s
  0x4e23ce93,                             //fmla          v19.4s, v20.4s, v3.4s
  0x4eb01e00,                             //mov           v0.16b, v16.16b
  0x4eb11e21,                             //mov           v1.16b, v17.16b
  0x4eb21e42,                             //mov           v2.16b, v18.16b
  0x4eb31e63,                             //mov           v3.16b, v19.16b
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_clamp_0_aarch64[] = {
  0xf8408423,                             //ldr           x3, [x1], #8
  0x6f00e410,                             //movi          v16.2d, #0x0
  0x4e30f400,                             //fmax          v0.4s, v0.4s, v16.4s
  0x4e30f421,                             //fmax          v1.4s, v1.4s, v16.4s
  0x4e30f442,                             //fmax          v2.4s, v2.4s, v16.4s
  0x4e30f463,                             //fmax          v3.4s, v3.4s, v16.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_clamp_1_aarch64[] = {
  0x4d40c850,                             //ld1r          {v16.4s}, [x2]
  0xf8408423,                             //ldr           x3, [x1], #8
  0x4eb0f400,                             //fmin          v0.4s, v0.4s, v16.4s
  0x4eb0f421,                             //fmin          v1.4s, v1.4s, v16.4s
  0x4eb0f442,                             //fmin          v2.4s, v2.4s, v16.4s
  0x4eb0f463,                             //fmin          v3.4s, v3.4s, v16.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_clamp_a_aarch64[] = {
  0x4d40c850,                             //ld1r          {v16.4s}, [x2]
  0xf8408423,                             //ldr           x3, [x1], #8
  0x4eb0f463,                             //fmin          v3.4s, v3.4s, v16.4s
  0x4ea3f400,                             //fmin          v0.4s, v0.4s, v3.4s
  0x4ea3f421,                             //fmin          v1.4s, v1.4s, v3.4s
  0x4ea3f442,                             //fmin          v2.4s, v2.4s, v3.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_set_rgb_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0xaa0803e9,                             //mov           x9, x8
  0x4ddfc920,                             //ld1r          {v0.4s}, [x9], #4
  0x91002108,                             //add           x8, x8, #0x8
  0x4d40c902,                             //ld1r          {v2.4s}, [x8]
  0x4d40c921,                             //ld1r          {v1.4s}, [x9]
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_swap_rb_aarch64[] = {
  0xf8408423,                             //ldr           x3, [x1], #8
  0x4ea01c10,                             //mov           v16.16b, v0.16b
  0x4ea21c40,                             //mov           v0.16b, v2.16b
  0x4eb01e02,                             //mov           v2.16b, v16.16b
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_swap_aarch64[] = {
  0xf8408423,                             //ldr           x3, [x1], #8
  0x4ea31c70,                             //mov           v16.16b, v3.16b
  0x4ea21c51,                             //mov           v17.16b, v2.16b
  0x4ea11c32,                             //mov           v18.16b, v1.16b
  0x4ea01c13,                             //mov           v19.16b, v0.16b
  0x4ea41c80,                             //mov           v0.16b, v4.16b
  0x4ea51ca1,                             //mov           v1.16b, v5.16b
  0x4ea61cc2,                             //mov           v2.16b, v6.16b
  0x4ea71ce3,                             //mov           v3.16b, v7.16b
  0x4eb31e64,                             //mov           v4.16b, v19.16b
  0x4eb21e45,                             //mov           v5.16b, v18.16b
  0x4eb11e26,                             //mov           v6.16b, v17.16b
  0x4eb01e07,                             //mov           v7.16b, v16.16b
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_move_src_dst_aarch64[] = {
  0xf8408423,                             //ldr           x3, [x1], #8
  0x4ea01c04,                             //mov           v4.16b, v0.16b
  0x4ea11c25,                             //mov           v5.16b, v1.16b
  0x4ea21c46,                             //mov           v6.16b, v2.16b
  0x4ea31c67,                             //mov           v7.16b, v3.16b
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_move_dst_src_aarch64[] = {
  0xf8408423,                             //ldr           x3, [x1], #8
  0x4ea41c80,                             //mov           v0.16b, v4.16b
  0x4ea51ca1,                             //mov           v1.16b, v5.16b
  0x4ea61cc2,                             //mov           v2.16b, v6.16b
  0x4ea71ce3,                             //mov           v3.16b, v7.16b
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_premul_aarch64[] = {
  0xf8408423,                             //ldr           x3, [x1], #8
  0x6e23dc00,                             //fmul          v0.4s, v0.4s, v3.4s
  0x6e23dc21,                             //fmul          v1.4s, v1.4s, v3.4s
  0x6e23dc42,                             //fmul          v2.4s, v2.4s, v3.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_unpremul_aarch64[] = {
  0x4d40c850,                             //ld1r          {v16.4s}, [x2]
  0xf8408423,                             //ldr           x3, [x1], #8
  0x4ea0d871,                             //fcmeq         v17.4s, v3.4s, #0.0
  0x6e23fe10,                             //fdiv          v16.4s, v16.4s, v3.4s
  0x4e711e10,                             //bic           v16.16b, v16.16b, v17.16b
  0x6e20de00,                             //fmul          v0.4s, v16.4s, v0.4s
  0x6e21de01,                             //fmul          v1.4s, v16.4s, v1.4s
  0x6e22de02,                             //fmul          v2.4s, v16.4s, v2.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_from_srgb_aarch64[] = {
  0x9100e048,                             //add           x8, x2, #0x38
  0x4d40c910,                             //ld1r          {v16.4s}, [x8]
  0x9100d048,                             //add           x8, x2, #0x34
  0x2d47cc52,                             //ldp           s18, s19, [x2, #60]
  0x4d40c911,                             //ld1r          {v17.4s}, [x8]
  0x6e22dc54,                             //fmul          v20.4s, v2.4s, v2.4s
  0x4eb01e15,                             //mov           v21.16b, v16.16b
  0x4eb01e17,                             //mov           v23.16b, v16.16b
  0x4f921050,                             //fmla          v16.4s, v2.4s, v18.s[0]
  0x4eb11e36,                             //mov           v22.16b, v17.16b
  0x4eb11e38,                             //mov           v24.16b, v17.16b
  0x4e34ce11,                             //fmla          v17.4s, v16.4s, v20.4s
  0x6e20dc10,                             //fmul          v16.4s, v0.4s, v0.4s
  0x91011048,                             //add           x8, x2, #0x44
  0x4f921015,                             //fmla          v21.4s, v0.4s, v18.s[0]
  0x4e30ceb6,                             //fmla          v22.4s, v21.4s, v16.4s
  0x4d40c910,                             //ld1r          {v16.4s}, [x8]
  0xf8408423,                             //ldr           x3, [x1], #8
  0x6e21dc34,                             //fmul          v20.4s, v1.4s, v1.4s
  0x4f921037,                             //fmla          v23.4s, v1.4s, v18.s[0]
  0x4f939015,                             //fmul          v21.4s, v0.4s, v19.s[0]
  0x4f939032,                             //fmul          v18.4s, v1.4s, v19.s[0]
  0x4f939053,                             //fmul          v19.4s, v2.4s, v19.s[0]
  0x6ea0e600,                             //fcmgt         v0.4s, v16.4s, v0.4s
  0x6ea1e601,                             //fcmgt         v1.4s, v16.4s, v1.4s
  0x6ea2e602,                             //fcmgt         v2.4s, v16.4s, v2.4s
  0x4e34cef8,                             //fmla          v24.4s, v23.4s, v20.4s
  0x6e761ea0,                             //bsl           v0.16b, v21.16b, v22.16b
  0x6e781e41,                             //bsl           v1.16b, v18.16b, v24.16b
  0x6e711e62,                             //bsl           v2.16b, v19.16b, v17.16b
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_to_srgb_aarch64[] = {
  0x6ea1d811,                             //frsqrte       v17.4s, v0.4s
  0x6ea1d835,                             //frsqrte       v21.4s, v1.4s
  0x6e31de37,                             //fmul          v23.4s, v17.4s, v17.4s
  0x6ea1d856,                             //frsqrte       v22.4s, v2.4s
  0x6e35deb9,                             //fmul          v25.4s, v21.4s, v21.4s
  0x4eb7fc17,                             //frsqrts       v23.4s, v0.4s, v23.4s
  0x91015048,                             //add           x8, x2, #0x54
  0x6e36deda,                             //fmul          v26.4s, v22.4s, v22.4s
  0x4eb9fc39,                             //frsqrts       v25.4s, v1.4s, v25.4s
  0x6e37de31,                             //fmul          v17.4s, v17.4s, v23.4s
  0x4d40c914,                             //ld1r          {v20.4s}, [x8]
  0x4ebafc5a,                             //frsqrts       v26.4s, v2.4s, v26.4s
  0x6e39deb5,                             //fmul          v21.4s, v21.4s, v25.4s
  0x4ea1da37,                             //frecpe        v23.4s, v17.4s
  0xbd405053,                             //ldr           s19, [x2, #80]
  0x91016048,                             //add           x8, x2, #0x58
  0x6e3aded6,                             //fmul          v22.4s, v22.4s, v26.4s
  0x4ea1dabb,                             //frecpe        v27.4s, v21.4s
  0x4e37fe3d,                             //frecps        v29.4s, v17.4s, v23.4s
  0x2d494052,                             //ldp           s18, s16, [x2, #72]
  0x4d40c918,                             //ld1r          {v24.4s}, [x8]
  0x4ea1dadc,                             //frecpe        v28.4s, v22.4s
  0x6e3ddef7,                             //fmul          v23.4s, v23.4s, v29.4s
  0x4e3bfebd,                             //frecps        v29.4s, v21.4s, v27.4s
  0x6e3ddf7b,                             //fmul          v27.4s, v27.4s, v29.4s
  0x4e3cfedd,                             //frecps        v29.4s, v22.4s, v28.4s
  0x6e3ddf9c,                             //fmul          v28.4s, v28.4s, v29.4s
  0x4eb41e9d,                             //mov           v29.16b, v20.16b
  0x6ea1da39,                             //frsqrte       v25.4s, v17.4s
  0x4f9312fd,                             //fmla          v29.4s, v23.4s, v19.s[0]
  0x4eb41e97,                             //mov           v23.16b, v20.16b
  0x4f92901a,                             //fmul          v26.4s, v0.4s, v18.s[0]
  0x4f931377,                             //fmla          v23.4s, v27.4s, v19.s[0]
  0x4f931394,                             //fmla          v20.4s, v28.4s, v19.s[0]
  0x4f929033,                             //fmul          v19.4s, v1.4s, v18.s[0]
  0x4f929052,                             //fmul          v18.4s, v2.4s, v18.s[0]
  0x6ea0e700,                             //fcmgt         v0.4s, v24.4s, v0.4s
  0x6ea1e701,                             //fcmgt         v1.4s, v24.4s, v1.4s
  0x6ea2e702,                             //fcmgt         v2.4s, v24.4s, v2.4s
  0x6e39df38,                             //fmul          v24.4s, v25.4s, v25.4s
  0x6ea1dabb,                             //frsqrte       v27.4s, v21.4s
  0x4eb8fe31,                             //frsqrts       v17.4s, v17.4s, v24.4s
  0x6ea1dadc,                             //frsqrte       v28.4s, v22.4s
  0x6e3bdf78,                             //fmul          v24.4s, v27.4s, v27.4s
  0x6e31df31,                             //fmul          v17.4s, v25.4s, v17.4s
  0x4eb8feb5,                             //frsqrts       v21.4s, v21.4s, v24.4s
  0x6e3cdf98,                             //fmul          v24.4s, v28.4s, v28.4s
  0x4f90123d,                             //fmla          v29.4s, v17.4s, v16.s[0]
  0x4d40c851,                             //ld1r          {v17.4s}, [x2]
  0x4eb8fed6,                             //frsqrts       v22.4s, v22.4s, v24.4s
  0x6e35df75,                             //fmul          v21.4s, v27.4s, v21.4s
  0x6e36df96,                             //fmul          v22.4s, v28.4s, v22.4s
  0xf8408423,                             //ldr           x3, [x1], #8
  0x4f9012b7,                             //fmla          v23.4s, v21.4s, v16.s[0]
  0x4f9012d4,                             //fmla          v20.4s, v22.4s, v16.s[0]
  0x4ebdf630,                             //fmin          v16.4s, v17.4s, v29.4s
  0x4eb7f635,                             //fmin          v21.4s, v17.4s, v23.4s
  0x4eb4f631,                             //fmin          v17.4s, v17.4s, v20.4s
  0x6e701f40,                             //bsl           v0.16b, v26.16b, v16.16b
  0x6e751e61,                             //bsl           v1.16b, v19.16b, v21.16b
  0x6e711e42,                             //bsl           v2.16b, v18.16b, v17.16b
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_scale_1_float_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0xbd400110,                             //ldr           s16, [x8]
  0x4f909000,                             //fmul          v0.4s, v0.4s, v16.s[0]
  0x4f909021,                             //fmul          v1.4s, v1.4s, v16.s[0]
  0x4f909042,                             //fmul          v2.4s, v2.4s, v16.s[0]
  0x4f909063,                             //fmul          v3.4s, v3.4s, v16.s[0]
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_scale_u8_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0xbd400c51,                             //ldr           s17, [x2, #12]
  0xf9400108,                             //ldr           x8, [x8]
  0x8b000108,                             //add           x8, x8, x0
  0x39400109,                             //ldrb          w9, [x8]
  0x3940050a,                             //ldrb          w10, [x8, #1]
  0x3940090b,                             //ldrb          w11, [x8, #2]
  0x39400d08,                             //ldrb          w8, [x8, #3]
  0x4e021d30,                             //mov           v16.h[0], w9
  0x4e061d50,                             //mov           v16.h[1], w10
  0x4e0a1d70,                             //mov           v16.h[2], w11
  0x4e0e1d10,                             //mov           v16.h[3], w8
  0x2f07b7f0,                             //bic           v16.4h, #0xff, lsl #8
  0x2f10a610,                             //uxtl          v16.4s, v16.4h
  0x6e21da10,                             //ucvtf         v16.4s, v16.4s
  0x4f919210,                             //fmul          v16.4s, v16.4s, v17.s[0]
  0x6e20de00,                             //fmul          v0.4s, v16.4s, v0.4s
  0x6e21de01,                             //fmul          v1.4s, v16.4s, v1.4s
  0x6e22de02,                             //fmul          v2.4s, v16.4s, v2.4s
  0x6e23de03,                             //fmul          v3.4s, v16.4s, v3.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_lerp_1_float_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0x4ea4d411,                             //fsub          v17.4s, v0.4s, v4.4s
  0x4ea41c80,                             //mov           v0.16b, v4.16b
  0x4ea5d432,                             //fsub          v18.4s, v1.4s, v5.4s
  0xbd400110,                             //ldr           s16, [x8]
  0x4ea51ca1,                             //mov           v1.16b, v5.16b
  0x4f901220,                             //fmla          v0.4s, v17.4s, v16.s[0]
  0x4ea6d451,                             //fsub          v17.4s, v2.4s, v6.4s
  0x4f901241,                             //fmla          v1.4s, v18.4s, v16.s[0]
  0x4ea61cc2,                             //mov           v2.16b, v6.16b
  0x4ea7d472,                             //fsub          v18.4s, v3.4s, v7.4s
  0x4ea71ce3,                             //mov           v3.16b, v7.16b
  0x4f901222,                             //fmla          v2.4s, v17.4s, v16.s[0]
  0x4f901243,                             //fmla          v3.4s, v18.4s, v16.s[0]
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_lerp_u8_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0xbd400c51,                             //ldr           s17, [x2, #12]
  0x4ea4d412,                             //fsub          v18.4s, v0.4s, v4.4s
  0xf9400108,                             //ldr           x8, [x8]
  0x8b000108,                             //add           x8, x8, x0
  0x39400109,                             //ldrb          w9, [x8]
  0x3940050a,                             //ldrb          w10, [x8, #1]
  0x3940090b,                             //ldrb          w11, [x8, #2]
  0x39400d08,                             //ldrb          w8, [x8, #3]
  0x4e021d30,                             //mov           v16.h[0], w9
  0x4e061d50,                             //mov           v16.h[1], w10
  0x4e0a1d70,                             //mov           v16.h[2], w11
  0x4e0e1d10,                             //mov           v16.h[3], w8
  0x2f07b7f0,                             //bic           v16.4h, #0xff, lsl #8
  0x2f10a600,                             //uxtl          v0.4s, v16.4h
  0x6e21d800,                             //ucvtf         v0.4s, v0.4s
  0x4f919010,                             //fmul          v16.4s, v0.4s, v17.s[0]
  0x4ea41c80,                             //mov           v0.16b, v4.16b
  0x4ea5d431,                             //fsub          v17.4s, v1.4s, v5.4s
  0x4ea51ca1,                             //mov           v1.16b, v5.16b
  0x4e32ce00,                             //fmla          v0.4s, v16.4s, v18.4s
  0x4ea6d452,                             //fsub          v18.4s, v2.4s, v6.4s
  0x4e31ce01,                             //fmla          v1.4s, v16.4s, v17.4s
  0x4ea61cc2,                             //mov           v2.16b, v6.16b
  0x4ea7d471,                             //fsub          v17.4s, v3.4s, v7.4s
  0x4ea71ce3,                             //mov           v3.16b, v7.16b
  0x4e32ce02,                             //fmla          v2.4s, v16.4s, v18.4s
  0x4e31ce03,                             //fmla          v3.4s, v16.4s, v17.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_lerp_565_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0xd37ff809,                             //lsl           x9, x0, #1
  0x2d4ec851,                             //ldp           s17, s18, [x2, #116]
  0x4ea4d413,                             //fsub          v19.4s, v0.4s, v4.4s
  0xf9400108,                             //ldr           x8, [x8]
  0x4ea41c80,                             //mov           v0.16b, v4.16b
  0xfc696903,                             //ldr           d3, [x8, x9]
  0x9101a048,                             //add           x8, x2, #0x68
  0x4d40c910,                             //ld1r          {v16.4s}, [x8]
  0x9101b048,                             //add           x8, x2, #0x6c
  0x2f10a463,                             //uxtl          v3.4s, v3.4h
  0x4e231e10,                             //and           v16.16b, v16.16b, v3.16b
  0x4e21da10,                             //scvtf         v16.4s, v16.4s
  0x4f919210,                             //fmul          v16.4s, v16.4s, v17.s[0]
  0x4d40c911,                             //ld1r          {v17.4s}, [x8]
  0x9101c048,                             //add           x8, x2, #0x70
  0x4e33ce00,                             //fmla          v0.4s, v16.4s, v19.4s
  0x4ea5d430,                             //fsub          v16.4s, v1.4s, v5.4s
  0x4e231e31,                             //and           v17.16b, v17.16b, v3.16b
  0x4e21da31,                             //scvtf         v17.4s, v17.4s
  0x4f929231,                             //fmul          v17.4s, v17.4s, v18.s[0]
  0x4d40c912,                             //ld1r          {v18.4s}, [x8]
  0x4ea51ca1,                             //mov           v1.16b, v5.16b
  0x4e30ce21,                             //fmla          v1.4s, v17.4s, v16.4s
  0xbd407c50,                             //ldr           s16, [x2, #124]
  0x4e231e52,                             //and           v18.16b, v18.16b, v3.16b
  0x4d40c843,                             //ld1r          {v3.4s}, [x2]
  0x4e21da52,                             //scvtf         v18.4s, v18.4s
  0x4ea6d451,                             //fsub          v17.4s, v2.4s, v6.4s
  0x4ea61cc2,                             //mov           v2.16b, v6.16b
  0x4f909250,                             //fmul          v16.4s, v18.4s, v16.s[0]
  0x4e31ce02,                             //fmla          v2.4s, v16.4s, v17.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_load_tables_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0x9100404b,                             //add           x11, x2, #0x10
  0x4d40c960,                             //ld1r          {v0.4s}, [x11]
  0xd37ef409,                             //lsl           x9, x0, #2
  0xa9402d0a,                             //ldp           x10, x11, [x8]
  0x3ce96942,                             //ldr           q2, [x10, x9]
  0xa9412109,                             //ldp           x9, x8, [x8, #16]
  0x4e221c01,                             //and           v1.16b, v0.16b, v2.16b
  0x0e143c2c,                             //mov           w12, v1.s[2]
  0xbc6c5971,                             //ldr           s17, [x11, w12, uxtw #2]
  0x1e26002c,                             //fmov          w12, s1
  0x6f380443,                             //ushr          v3.4s, v2.4s, #8
  0x6f300450,                             //ushr          v16.4s, v2.4s, #16
  0x8b2c496c,                             //add           x12, x11, w12, uxtw #2
  0x0e0c3c2a,                             //mov           w10, v1.s[1]
  0x0e1c3c2d,                             //mov           w13, v1.s[3]
  0x4e231c01,                             //and           v1.16b, v0.16b, v3.16b
  0x4e301c03,                             //and           v3.16b, v0.16b, v16.16b
  0x0d408180,                             //ld1           {v0.s}[0], [x12]
  0x0e143c2c,                             //mov           w12, v1.s[2]
  0xbc6c5932,                             //ldr           s18, [x9, w12, uxtw #2]
  0x1e26002c,                             //fmov          w12, s1
  0x8b2a496a,                             //add           x10, x11, w10, uxtw #2
  0xbc6d5970,                             //ldr           s16, [x11, w13, uxtw #2]
  0x0e0c3c2b,                             //mov           w11, v1.s[1]
  0x0e1c3c2d,                             //mov           w13, v1.s[3]
  0x8b2c492c,                             //add           x12, x9, w12, uxtw #2
  0xbc6d5933,                             //ldr           s19, [x9, w13, uxtw #2]
  0x0e0c3c6d,                             //mov           w13, v3.s[1]
  0x8b2b4929,                             //add           x9, x9, w11, uxtw #2
  0x0e143c6b,                             //mov           w11, v3.s[2]
  0x0d408181,                             //ld1           {v1.s}[0], [x12]
  0x0e1c3c6c,                             //mov           w12, v3.s[3]
  0x0d409140,                             //ld1           {v0.s}[1], [x10]
  0x1e26006a,                             //fmov          w10, s3
  0xbd400c43,                             //ldr           s3, [x2, #12]
  0x6f280442,                             //ushr          v2.4s, v2.4s, #24
  0x4e21d842,                             //scvtf         v2.4s, v2.4s
  0x8b2a490a,                             //add           x10, x8, w10, uxtw #2
  0x4f839043,                             //fmul          v3.4s, v2.4s, v3.s[0]
  0x0d408142,                             //ld1           {v2.s}[0], [x10]
  0x8b2d490a,                             //add           x10, x8, w13, uxtw #2
  0x6e140620,                             //mov           v0.s[2], v17.s[0]
  0xbc6b5911,                             //ldr           s17, [x8, w11, uxtw #2]
  0x0d409121,                             //ld1           {v1.s}[1], [x9]
  0x0d409142,                             //ld1           {v2.s}[1], [x10]
  0x6e1c0600,                             //mov           v0.s[3], v16.s[0]
  0xbc6c5910,                             //ldr           s16, [x8, w12, uxtw #2]
  0x6e140641,                             //mov           v1.s[2], v18.s[0]
  0x6e140622,                             //mov           v2.s[2], v17.s[0]
  0x6e1c0661,                             //mov           v1.s[3], v19.s[0]
  0x6e1c0602,                             //mov           v2.s[3], v16.s[0]
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_load_a8_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0xbd400c43,                             //ldr           s3, [x2, #12]
  0x6f00e400,                             //movi          v0.2d, #0x0
  0x6f00e401,                             //movi          v1.2d, #0x0
  0xf9400108,                             //ldr           x8, [x8]
  0x8b000108,                             //add           x8, x8, x0
  0x39400109,                             //ldrb          w9, [x8]
  0x3940050a,                             //ldrb          w10, [x8, #1]
  0x3940090b,                             //ldrb          w11, [x8, #2]
  0x39400d08,                             //ldrb          w8, [x8, #3]
  0x4e021d22,                             //mov           v2.h[0], w9
  0x4e061d42,                             //mov           v2.h[1], w10
  0x4e0a1d62,                             //mov           v2.h[2], w11
  0x4e0e1d02,                             //mov           v2.h[3], w8
  0x2f07b7e2,                             //bic           v2.4h, #0xff, lsl #8
  0x2f10a442,                             //uxtl          v2.4s, v2.4h
  0x6e21d842,                             //ucvtf         v2.4s, v2.4s
  0x4f839043,                             //fmul          v3.4s, v2.4s, v3.s[0]
  0x6f00e402,                             //movi          v2.2d, #0x0
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_store_a8_aarch64[] = {
  0xf9400028,                             //ldr           x8, [x1]
  0xbd400850,                             //ldr           s16, [x2, #8]
  0xf9400108,                             //ldr           x8, [x8]
  0x4f909070,                             //fmul          v16.4s, v3.4s, v16.s[0]
  0x6e21aa10,                             //fcvtnu        v16.4s, v16.4s
  0x0e612a10,                             //xtn           v16.4h, v16.4s
  0x0e0e3e09,                             //umov          w9, v16.h[3]
  0x8b000108,                             //add           x8, x8, x0
  0x39000d09,                             //strb          w9, [x8, #3]
  0x0e0a3e09,                             //umov          w9, v16.h[2]
  0x39000909,                             //strb          w9, [x8, #2]
  0x0e063e09,                             //umov          w9, v16.h[1]
  0x39000509,                             //strb          w9, [x8, #1]
  0x0e023e09,                             //umov          w9, v16.h[0]
  0x39000109,                             //strb          w9, [x8]
  0xf9400423,                             //ldr           x3, [x1, #8]
  0x91004021,                             //add           x1, x1, #0x10
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_load_565_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0xd37ff809,                             //lsl           x9, x0, #1
  0xf9400108,                             //ldr           x8, [x8]
  0xfc696900,                             //ldr           d0, [x8, x9]
  0x9101a048,                             //add           x8, x2, #0x68
  0x4d40c901,                             //ld1r          {v1.4s}, [x8]
  0x9101b048,                             //add           x8, x2, #0x6c
  0x4d40c902,                             //ld1r          {v2.4s}, [x8]
  0x9101c048,                             //add           x8, x2, #0x70
  0x4d40c903,                             //ld1r          {v3.4s}, [x8]
  0x2f10a400,                             //uxtl          v0.4s, v0.4h
  0x4e201c21,                             //and           v1.16b, v1.16b, v0.16b
  0x4e201c42,                             //and           v2.16b, v2.16b, v0.16b
  0x4e201c71,                             //and           v17.16b, v3.16b, v0.16b
  0x2d4e8c50,                             //ldp           s16, s3, [x2, #116]
  0x4e21d820,                             //scvtf         v0.4s, v1.4s
  0x4e21d841,                             //scvtf         v1.4s, v2.4s
  0x4e21da22,                             //scvtf         v2.4s, v17.4s
  0x4f909000,                             //fmul          v0.4s, v0.4s, v16.s[0]
  0xbd407c50,                             //ldr           s16, [x2, #124]
  0x4f839021,                             //fmul          v1.4s, v1.4s, v3.s[0]
  0x4d40c843,                             //ld1r          {v3.4s}, [x2]
  0x4f909042,                             //fmul          v2.4s, v2.4s, v16.s[0]
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_store_565_aarch64[] = {
  0x2d504450,                             //ldp           s16, s17, [x2, #128]
  0xf9400028,                             //ldr           x8, [x1]
  0xd37ff809,                             //lsl           x9, x0, #1
  0x4f909012,                             //fmul          v18.4s, v0.4s, v16.s[0]
  0x4f919031,                             //fmul          v17.4s, v1.4s, v17.s[0]
  0x6e21aa52,                             //fcvtnu        v18.4s, v18.4s
  0x6e21aa31,                             //fcvtnu        v17.4s, v17.4s
  0xf9400108,                             //ldr           x8, [x8]
  0x4f909050,                             //fmul          v16.4s, v2.4s, v16.s[0]
  0x4f2b5652,                             //shl           v18.4s, v18.4s, #11
  0x4f255631,                             //shl           v17.4s, v17.4s, #5
  0x4eb21e31,                             //orr           v17.16b, v17.16b, v18.16b
  0x6e21aa10,                             //fcvtnu        v16.4s, v16.4s
  0x4eb01e30,                             //orr           v16.16b, v17.16b, v16.16b
  0x0e612a10,                             //xtn           v16.4h, v16.4s
  0xfc296910,                             //str           d16, [x8, x9]
  0xf9400423,                             //ldr           x3, [x1, #8]
  0x91004021,                             //add           x1, x1, #0x10
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_load_8888_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0xd37ef409,                             //lsl           x9, x0, #2
  0xbd400c42,                             //ldr           s2, [x2, #12]
  0xf9400108,                             //ldr           x8, [x8]
  0x3ce96900,                             //ldr           q0, [x8, x9]
  0x91004048,                             //add           x8, x2, #0x10
  0x4d40c901,                             //ld1r          {v1.4s}, [x8]
  0x6f380410,                             //ushr          v16.4s, v0.4s, #8
  0x6f300411,                             //ushr          v17.4s, v0.4s, #16
  0x4e201c23,                             //and           v3.16b, v1.16b, v0.16b
  0x6f280400,                             //ushr          v0.4s, v0.4s, #24
  0x4e301c30,                             //and           v16.16b, v1.16b, v16.16b
  0x4e311c21,                             //and           v1.16b, v1.16b, v17.16b
  0x4e21d863,                             //scvtf         v3.4s, v3.4s
  0x4e21d811,                             //scvtf         v17.4s, v0.4s
  0x4e21da10,                             //scvtf         v16.4s, v16.4s
  0x4e21d832,                             //scvtf         v18.4s, v1.4s
  0x4f829060,                             //fmul          v0.4s, v3.4s, v2.s[0]
  0x4f829223,                             //fmul          v3.4s, v17.4s, v2.s[0]
  0x4f829201,                             //fmul          v1.4s, v16.4s, v2.s[0]
  0x4f829242,                             //fmul          v2.4s, v18.4s, v2.s[0]
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_store_8888_aarch64[] = {
  0xbd400850,                             //ldr           s16, [x2, #8]
  0xf9400028,                             //ldr           x8, [x1]
  0xd37ef409,                             //lsl           x9, x0, #2
  0x4f909032,                             //fmul          v18.4s, v1.4s, v16.s[0]
  0x4f909011,                             //fmul          v17.4s, v0.4s, v16.s[0]
  0x6e21aa52,                             //fcvtnu        v18.4s, v18.4s
  0x6e21aa31,                             //fcvtnu        v17.4s, v17.4s
  0x4f285652,                             //shl           v18.4s, v18.4s, #8
  0x4eb11e51,                             //orr           v17.16b, v18.16b, v17.16b
  0x4f909052,                             //fmul          v18.4s, v2.4s, v16.s[0]
  0xf9400108,                             //ldr           x8, [x8]
  0x4f909070,                             //fmul          v16.4s, v3.4s, v16.s[0]
  0x6e21aa52,                             //fcvtnu        v18.4s, v18.4s
  0x6e21aa10,                             //fcvtnu        v16.4s, v16.4s
  0x4f305652,                             //shl           v18.4s, v18.4s, #16
  0x4eb21e31,                             //orr           v17.16b, v17.16b, v18.16b
  0x4f385610,                             //shl           v16.4s, v16.4s, #24
  0x4eb01e30,                             //orr           v16.16b, v17.16b, v16.16b
  0x3ca96910,                             //str           q16, [x8, x9]
  0xf9400423,                             //ldr           x3, [x1, #8]
  0x91004021,                             //add           x1, x1, #0x10
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_load_f16_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0xf9400108,                             //ldr           x8, [x8]
  0x8b000d08,                             //add           x8, x8, x0, lsl #3
  0x0c400510,                             //ld4           {v16.4h-v19.4h}, [x8]
  0x0e217a00,                             //fcvtl         v0.4s, v16.4h
  0x0e217a21,                             //fcvtl         v1.4s, v17.4h
  0x0e217a42,                             //fcvtl         v2.4s, v18.4h
  0x0e217a63,                             //fcvtl         v3.4s, v19.4h
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_store_f16_aarch64[] = {
  0xf9400028,                             //ldr           x8, [x1]
  0x0e216810,                             //fcvtn         v16.4h, v0.4s
  0x0e216831,                             //fcvtn         v17.4h, v1.4s
  0x0e216852,                             //fcvtn         v18.4h, v2.4s
  0xf9400108,                             //ldr           x8, [x8]
  0x0e216873,                             //fcvtn         v19.4h, v3.4s
  0x8b000d08,                             //add           x8, x8, x0, lsl #3
  0x0c000510,                             //st4           {v16.4h-v19.4h}, [x8]
  0xf9400423,                             //ldr           x3, [x1, #8]
  0x91004021,                             //add           x1, x1, #0x10
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_store_f32_aarch64[] = {
  0xf9400028,                             //ldr           x8, [x1]
  0xf9400108,                             //ldr           x8, [x8]
  0x8b001108,                             //add           x8, x8, x0, lsl #4
  0x4c000900,                             //st4           {v0.4s-v3.4s}, [x8]
  0xf9400423,                             //ldr           x3, [x1, #8]
  0x91004021,                             //add           x1, x1, #0x10
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_clamp_x_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0x6f00e411,                             //movi          v17.2d, #0x0
  0x4e20f620,                             //fmax          v0.4s, v17.4s, v0.4s
  0x6f07e7f1,                             //movi          v17.2d, #0xffffffffffffffff
  0x4d40c910,                             //ld1r          {v16.4s}, [x8]
  0x4eb18610,                             //add           v16.4s, v16.4s, v17.4s
  0x4eb0f400,                             //fmin          v0.4s, v0.4s, v16.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_clamp_y_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0x6f00e411,                             //movi          v17.2d, #0x0
  0x4e21f621,                             //fmax          v1.4s, v17.4s, v1.4s
  0x6f07e7f1,                             //movi          v17.2d, #0xffffffffffffffff
  0x4d40c910,                             //ld1r          {v16.4s}, [x8]
  0x4eb18610,                             //add           v16.4s, v16.4s, v17.4s
  0x4eb0f421,                             //fmin          v1.4s, v1.4s, v16.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_repeat_x_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0x6f07e7f1,                             //movi          v17.2d, #0xffffffffffffffff
  0xbd400110,                             //ldr           s16, [x8]
  0x4e040612,                             //dup           v18.4s, v16.s[0]
  0x4eb18651,                             //add           v17.4s, v18.4s, v17.4s
  0x6e32fc12,                             //fdiv          v18.4s, v0.4s, v18.4s
  0x4e219a52,                             //frintm        v18.4s, v18.4s
  0x4f905240,                             //fmls          v0.4s, v18.4s, v16.s[0]
  0x4eb1f400,                             //fmin          v0.4s, v0.4s, v17.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_repeat_y_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0x6f07e7f1,                             //movi          v17.2d, #0xffffffffffffffff
  0xbd400110,                             //ldr           s16, [x8]
  0x4e040612,                             //dup           v18.4s, v16.s[0]
  0x4eb18651,                             //add           v17.4s, v18.4s, v17.4s
  0x6e32fc32,                             //fdiv          v18.4s, v1.4s, v18.4s
  0x4e219a52,                             //frintm        v18.4s, v18.4s
  0x4f905241,                             //fmls          v1.4s, v18.4s, v16.s[0]
  0x4eb1f421,                             //fmin          v1.4s, v1.4s, v17.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_mirror_x_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0xbd400110,                             //ldr           s16, [x8]
  0x4e040611,                             //dup           v17.4s, v16.s[0]
  0x1e302a10,                             //fadd          s16, s16, s16
  0x4eb1d400,                             //fsub          v0.4s, v0.4s, v17.4s
  0x4e040612,                             //dup           v18.4s, v16.s[0]
  0x6e32fc12,                             //fdiv          v18.4s, v0.4s, v18.4s
  0x4e219a52,                             //frintm        v18.4s, v18.4s
  0x4f905240,                             //fmls          v0.4s, v18.4s, v16.s[0]
  0x6f07e7f0,                             //movi          v16.2d, #0xffffffffffffffff
  0x4eb1d400,                             //fsub          v0.4s, v0.4s, v17.4s
  0x4eb08630,                             //add           v16.4s, v17.4s, v16.4s
  0x4ea0f800,                             //fabs          v0.4s, v0.4s
  0x4eb0f400,                             //fmin          v0.4s, v0.4s, v16.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_mirror_y_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0xbd400110,                             //ldr           s16, [x8]
  0x4e040611,                             //dup           v17.4s, v16.s[0]
  0x1e302a10,                             //fadd          s16, s16, s16
  0x4eb1d421,                             //fsub          v1.4s, v1.4s, v17.4s
  0x4e040612,                             //dup           v18.4s, v16.s[0]
  0x6e32fc32,                             //fdiv          v18.4s, v1.4s, v18.4s
  0x4e219a52,                             //frintm        v18.4s, v18.4s
  0x4f905241,                             //fmls          v1.4s, v18.4s, v16.s[0]
  0x6f07e7f0,                             //movi          v16.2d, #0xffffffffffffffff
  0x4eb1d421,                             //fsub          v1.4s, v1.4s, v17.4s
  0x4eb08630,                             //add           v16.4s, v17.4s, v16.4s
  0x4ea0f821,                             //fabs          v1.4s, v1.4s
  0x4eb0f421,                             //fmin          v1.4s, v1.4s, v16.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_matrix_2x3_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0xaa0803e9,                             //mov           x9, x8
  0x9100410a,                             //add           x10, x8, #0x10
  0x4ddfc932,                             //ld1r          {v18.4s}, [x9], #4
  0x4d40c950,                             //ld1r          {v16.4s}, [x10]
  0x2d415113,                             //ldp           s19, s20, [x8, #8]
  0x9100510a,                             //add           x10, x8, #0x14
  0x4d40c951,                             //ld1r          {v17.4s}, [x10]
  0x4f931030,                             //fmla          v16.4s, v1.4s, v19.s[0]
  0xbd400133,                             //ldr           s19, [x9]
  0x4f941031,                             //fmla          v17.4s, v1.4s, v20.s[0]
  0x4e20ce50,                             //fmla          v16.4s, v18.4s, v0.4s
  0x4f931011,                             //fmla          v17.4s, v0.4s, v19.s[0]
  0x4eb01e00,                             //mov           v0.16b, v16.16b
  0x4eb11e21,                             //mov           v1.16b, v17.16b
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_matrix_3x4_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0xaa0803e9,                             //mov           x9, x8
  0x9100910a,                             //add           x10, x8, #0x24
  0x4ddfc933,                             //ld1r          {v19.4s}, [x9], #4
  0x4d40c950,                             //ld1r          {v16.4s}, [x10]
  0x9100a10a,                             //add           x10, x8, #0x28
  0x4d40c951,                             //ld1r          {v17.4s}, [x10]
  0x9100b10a,                             //add           x10, x8, #0x2c
  0x2d435514,                             //ldp           s20, s21, [x8, #24]
  0xbd402116,                             //ldr           s22, [x8, #32]
  0x4d40c952,                             //ld1r          {v18.4s}, [x10]
  0x4f941050,                             //fmla          v16.4s, v2.4s, v20.s[0]
  0x4f951051,                             //fmla          v17.4s, v2.4s, v21.s[0]
  0x4f961052,                             //fmla          v18.4s, v2.4s, v22.s[0]
  0x2d425502,                             //ldp           s2, s21, [x8, #16]
  0x2d415d14,                             //ldp           s20, s23, [x8, #8]
  0x4f821031,                             //fmla          v17.4s, v1.4s, v2.s[0]
  0xbd400122,                             //ldr           s2, [x9]
  0x4f971030,                             //fmla          v16.4s, v1.4s, v23.s[0]
  0x4f951032,                             //fmla          v18.4s, v1.4s, v21.s[0]
  0x4e20ce70,                             //fmla          v16.4s, v19.4s, v0.4s
  0x4f941012,                             //fmla          v18.4s, v0.4s, v20.s[0]
  0x4f821011,                             //fmla          v17.4s, v0.4s, v2.s[0]
  0x4eb01e00,                             //mov           v0.16b, v16.16b
  0x4eb11e21,                             //mov           v1.16b, v17.16b
  0x4eb21e42,                             //mov           v2.16b, v18.16b
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_matrix_perspective_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0xaa0803e9,                             //mov           x9, x8
  0x9100510a,                             //add           x10, x8, #0x14
  0x4ddfc930,                             //ld1r          {v16.4s}, [x9], #4
  0x4d40c951,                             //ld1r          {v17.4s}, [x10]
  0x9100810a,                             //add           x10, x8, #0x20
  0x4d40c952,                             //ld1r          {v18.4s}, [x10]
  0x2d41d113,                             //ldp           s19, s20, [x8, #12]
  0x2d435915,                             //ldp           s21, s22, [x8, #24]
  0x91002108,                             //add           x8, x8, #0x8
  0x4f941031,                             //fmla          v17.4s, v1.4s, v20.s[0]
  0x4d40c914,                             //ld1r          {v20.4s}, [x8]
  0x4f961032,                             //fmla          v18.4s, v1.4s, v22.s[0]
  0xbd400136,                             //ldr           s22, [x9]
  0x4f951012,                             //fmla          v18.4s, v0.4s, v21.s[0]
  0x4f931011,                             //fmla          v17.4s, v0.4s, v19.s[0]
  0x4f961034,                             //fmla          v20.4s, v1.4s, v22.s[0]
  0x4ea1da41,                             //frecpe        v1.4s, v18.4s
  0x4e21fe52,                             //frecps        v18.4s, v18.4s, v1.4s
  0x6e32dc32,                             //fmul          v18.4s, v1.4s, v18.4s
  0x4e20ce14,                             //fmla          v20.4s, v16.4s, v0.4s
  0x6e32de21,                             //fmul          v1.4s, v17.4s, v18.4s
  0x6e32de80,                             //fmul          v0.4s, v20.4s, v18.4s
  0xd61f0060,                             //br            x3
};

CODE const uint32_t sk_linear_gradient_2stops_aarch64[] = {
  0xa8c10c28,                             //ldp           x8, x3, [x1], #16
  0xad404503,                             //ldp           q3, q17, [x8]
  0x4e040470,                             //dup           v16.4s, v3.s[0]
  0x4e0c0461,                             //dup           v1.4s, v3.s[1]
  0x4e140462,                             //dup           v2.4s, v3.s[2]
  0x4e1c0463,                             //dup           v3.4s, v3.s[3]
  0x4f911010,                             //fmla          v16.4s, v0.4s, v17.s[0]
  0x4fb11001,                             //fmla          v1.4s, v0.4s, v17.s[1]
  0x4f911802,                             //fmla          v2.4s, v0.4s, v17.s[2]
  0x4fb11803,                             //fmla          v3.4s, v0.4s, v17.s[3]
  0x4eb01e00,                             //mov           v0.16b, v16.16b
  0xd61f0060,                             //br            x3
};
#elif defined(__arm__)

CODE const uint32_t sk_start_pipeline_vfp4[] = {
  0xe92d41f0,                             //push          {r4, r5, r6, r7, r8, lr}
  0xe1a07001,                             //mov           r7, r1
  0xe1a04000,                             //mov           r4, r0
  0xe1a05003,                             //mov           r5, r3
  0xe1a08002,                             //mov           r8, r2
  0xe4976004,                             //ldr           r6, [r7], #4
  0xe2840002,                             //add           r0, r4, #2
  0xea00000d,                             //b             58 <sk_start_pipeline_vfp4+0x58>
  0xf2800010,                             //vmov.i32      d0, #0
  0xe1a00004,                             //mov           r0, r4
  0xf2801010,                             //vmov.i32      d1, #0
  0xe1a01007,                             //mov           r1, r7
  0xf2802010,                             //vmov.i32      d2, #0
  0xe1a02008,                             //mov           r2, r8
  0xf2803010,                             //vmov.i32      d3, #0
  0xf2804010,                             //vmov.i32      d4, #0
  0xf2805010,                             //vmov.i32      d5, #0
  0xf2806010,                             //vmov.i32      d6, #0
  0xf2807010,                             //vmov.i32      d7, #0
  0xe12fff36,                             //blx           r6
  0xe2840004,                             //add           r0, r4, #4
  0xe2844002,                             //add           r4, r4, #2
  0xe1500005,                             //cmp           r0, r5
  0x9affffef,                             //bls           20 <sk_start_pipeline_vfp4+0x20>
  0xe1a00004,                             //mov           r0, r4
  0xe8bd81f0,                             //pop           {r4, r5, r6, r7, r8, pc}
};

CODE const uint32_t sk_just_return_vfp4[] = {
  0xe12fff1e,                             //bx            lr
};

CODE const uint32_t sk_seed_shader_vfp4[] = {
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xee800b90,                             //vdup.32       d16, r0
  0xf3fb0620,                             //vcvt.f32.s32  d16, d16
  0xedd23b05,                             //vldr          d19, [r2, #20]
  0xf2803010,                             //vmov.i32      d3, #0
  0xf4e31c9f,                             //vld1.32       {d17[]}, [r3 :32]
  0xe2823004,                             //add           r3, r2, #4
  0xf3fb1621,                             //vcvt.f32.s32  d17, d17
  0xe2811008,                             //add           r1, r1, #8
  0xf4e32c9f,                             //vld1.32       {d18[]}, [r3 :32]
  0xf2804010,                             //vmov.i32      d4, #0
  0xf2400da2,                             //vadd.f32      d16, d16, d18
  0xf2805010,                             //vmov.i32      d5, #0
  0xf4a22c9f,                             //vld1.32       {d2[]}, [r2 :32]
  0xf2011da2,                             //vadd.f32      d1, d17, d18
  0xf2806010,                             //vmov.i32      d6, #0
  0xf2030da0,                             //vadd.f32      d0, d19, d16
  0xf2807010,                             //vmov.i32      d7, #0
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_constant_color_vfp4[] = {
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xe2811008,                             //add           r1, r1, #8
  0xf4630a0f,                             //vld1.8        {d16-d17}, [r3]
  0xf3b40c20,                             //vdup.32       d0, d16[0]
  0xf3bc1c20,                             //vdup.32       d1, d16[1]
  0xf3b42c21,                             //vdup.32       d2, d17[0]
  0xf3bc3c21,                             //vdup.32       d3, d17[1]
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_clear_vfp4[] = {
  0xe4913004,                             //ldr           r3, [r1], #4
  0xf2800010,                             //vmov.i32      d0, #0
  0xf2801010,                             //vmov.i32      d1, #0
  0xf2802010,                             //vmov.i32      d2, #0
  0xf2803010,                             //vmov.i32      d3, #0
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_plus__vfp4[] = {
  0xf2000d04,                             //vadd.f32      d0, d0, d4
  0xe4913004,                             //ldr           r3, [r1], #4
  0xf2011d05,                             //vadd.f32      d1, d1, d5
  0xf2022d06,                             //vadd.f32      d2, d2, d6
  0xf2033d07,                             //vadd.f32      d3, d3, d7
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_srcover_vfp4[] = {
  0xf4e20c9f,                             //vld1.32       {d16[]}, [r2 :32]
  0xe4913004,                             //ldr           r3, [r1], #4
  0xf2600d83,                             //vsub.f32      d16, d16, d3
  0xf2040c30,                             //vfma.f32      d0, d4, d16
  0xf2051c30,                             //vfma.f32      d1, d5, d16
  0xf2062c30,                             //vfma.f32      d2, d6, d16
  0xf2073c30,                             //vfma.f32      d3, d7, d16
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_dstover_vfp4[] = {
  0xf4e20c9f,                             //vld1.32       {d16[]}, [r2 :32]
  0xf2651115,                             //vorr          d17, d5, d5
  0xf2604d87,                             //vsub.f32      d20, d16, d7
  0xf2640114,                             //vorr          d16, d4, d4
  0xf2662116,                             //vorr          d18, d6, d6
  0xe4913004,                             //ldr           r3, [r1], #4
  0xf2673117,                             //vorr          d19, d7, d7
  0xf2400c34,                             //vfma.f32      d16, d0, d20
  0xf2411c34,                             //vfma.f32      d17, d1, d20
  0xf2422c34,                             //vfma.f32      d18, d2, d20
  0xf2433c34,                             //vfma.f32      d19, d3, d20
  0xf22001b0,                             //vorr          d0, d16, d16
  0xf22111b1,                             //vorr          d1, d17, d17
  0xf22221b2,                             //vorr          d2, d18, d18
  0xf22331b3,                             //vorr          d3, d19, d19
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_clamp_0_vfp4[] = {
  0xf2c00010,                             //vmov.i32      d16, #0
  0xe4913004,                             //ldr           r3, [r1], #4
  0xf2000f20,                             //vmax.f32      d0, d0, d16
  0xf2011f20,                             //vmax.f32      d1, d1, d16
  0xf2022f20,                             //vmax.f32      d2, d2, d16
  0xf2033f20,                             //vmax.f32      d3, d3, d16
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_clamp_1_vfp4[] = {
  0xf4e20c9f,                             //vld1.32       {d16[]}, [r2 :32]
  0xe4913004,                             //ldr           r3, [r1], #4
  0xf2200f20,                             //vmin.f32      d0, d0, d16
  0xf2211f20,                             //vmin.f32      d1, d1, d16
  0xf2222f20,                             //vmin.f32      d2, d2, d16
  0xf2233f20,                             //vmin.f32      d3, d3, d16
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_clamp_a_vfp4[] = {
  0xf4e20c9f,                             //vld1.32       {d16[]}, [r2 :32]
  0xe4913004,                             //ldr           r3, [r1], #4
  0xf2233f20,                             //vmin.f32      d3, d3, d16
  0xf2200f03,                             //vmin.f32      d0, d0, d3
  0xf2211f03,                             //vmin.f32      d1, d1, d3
  0xf2222f03,                             //vmin.f32      d2, d2, d3
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_set_rgb_vfp4[] = {
  0xe92d4800,                             //push          {fp, lr}
  0xe591e000,                             //ldr           lr, [r1]
  0xe591c004,                             //ldr           ip, [r1, #4]
  0xe2811008,                             //add           r1, r1, #8
  0xe28e3008,                             //add           r3, lr, #8
  0xf4ae0c9f,                             //vld1.32       {d0[]}, [lr :32]
  0xf4a32c9f,                             //vld1.32       {d2[]}, [r3 :32]
  0xe28e3004,                             //add           r3, lr, #4
  0xf4a31c9f,                             //vld1.32       {d1[]}, [r3 :32]
  0xe8bd4800,                             //pop           {fp, lr}
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_swap_rb_vfp4[] = {
  0xeef00b40,                             //vmov.f64      d16, d0
  0xe4913004,                             //ldr           r3, [r1], #4
  0xeeb00b42,                             //vmov.f64      d0, d2
  0xeeb02b60,                             //vmov.f64      d2, d16
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_swap_vfp4[] = {
  0xeef00b43,                             //vmov.f64      d16, d3
  0xe4913004,                             //ldr           r3, [r1], #4
  0xeef01b42,                             //vmov.f64      d17, d2
  0xeef02b41,                             //vmov.f64      d18, d1
  0xeef03b40,                             //vmov.f64      d19, d0
  0xeeb00b44,                             //vmov.f64      d0, d4
  0xeeb01b45,                             //vmov.f64      d1, d5
  0xeeb02b46,                             //vmov.f64      d2, d6
  0xeeb03b47,                             //vmov.f64      d3, d7
  0xeeb04b63,                             //vmov.f64      d4, d19
  0xeeb05b62,                             //vmov.f64      d5, d18
  0xeeb06b61,                             //vmov.f64      d6, d17
  0xeeb07b60,                             //vmov.f64      d7, d16
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_move_src_dst_vfp4[] = {
  0xeeb04b40,                             //vmov.f64      d4, d0
  0xe4913004,                             //ldr           r3, [r1], #4
  0xeeb05b41,                             //vmov.f64      d5, d1
  0xeeb06b42,                             //vmov.f64      d6, d2
  0xeeb07b43,                             //vmov.f64      d7, d3
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_move_dst_src_vfp4[] = {
  0xeeb00b44,                             //vmov.f64      d0, d4
  0xe4913004,                             //ldr           r3, [r1], #4
  0xeeb01b45,                             //vmov.f64      d1, d5
  0xeeb02b46,                             //vmov.f64      d2, d6
  0xeeb03b47,                             //vmov.f64      d3, d7
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_premul_vfp4[] = {
  0xf3000d13,                             //vmul.f32      d0, d0, d3
  0xe4913004,                             //ldr           r3, [r1], #4
  0xf3011d13,                             //vmul.f32      d1, d1, d3
  0xf3022d13,                             //vmul.f32      d2, d2, d3
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_unpremul_vfp4[] = {
  0xed2d8b04,                             //vpush         {d8-d9}
  0xed928a00,                             //vldr          s16, [r2]
  0xf2c00010,                             //vmov.i32      d16, #0
  0xf3f91503,                             //vceq.f32      d17, d3, #0
  0xe4913004,                             //ldr           r3, [r1], #4
  0xeec89a23,                             //vdiv.f32      s19, s16, s7
  0xee889a03,                             //vdiv.f32      s18, s16, s6
  0xf3501199,                             //vbsl          d17, d16, d9
  0xf3010d90,                             //vmul.f32      d0, d17, d0
  0xf3011d91,                             //vmul.f32      d1, d17, d1
  0xf3012d92,                             //vmul.f32      d2, d17, d2
  0xecbd8b04,                             //vpop          {d8-d9}
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_from_srgb_vfp4[] = {
  0xed2d8b02,                             //vpush         {d8}
  0xe282303c,                             //add           r3, r2, #60
  0xed928a10,                             //vldr          s16, [r2, #64]
  0xf3402d10,                             //vmul.f32      d18, d0, d0
  0xf4e30c9f,                             //vld1.32       {d16[]}, [r3 :32]
  0xe2823038,                             //add           r3, r2, #56
  0xf3413d11,                             //vmul.f32      d19, d1, d1
  0xf4e31c9f,                             //vld1.32       {d17[]}, [r3 :32]
  0xe2823044,                             //add           r3, r2, #68
  0xf26141b1,                             //vorr          d20, d17, d17
  0xf26171b1,                             //vorr          d23, d17, d17
  0xf4e38c9f,                             //vld1.32       {d24[]}, [r3 :32]
  0xf2404c30,                             //vfma.f32      d20, d0, d16
  0xe2823034,                             //add           r3, r2, #52
  0xf2417c30,                             //vfma.f32      d23, d1, d16
  0xf2421c30,                             //vfma.f32      d17, d2, d16
  0xf3425d12,                             //vmul.f32      d21, d2, d2
  0xf2e16948,                             //vmul.f32      d22, d1, d8[0]
  0xf2e00948,                             //vmul.f32      d16, d0, d8[0]
  0xf2e29948,                             //vmul.f32      d25, d2, d8[0]
  0xf3282e82,                             //vcgt.f32      d2, d24, d2
  0xf3281e81,                             //vcgt.f32      d1, d24, d1
  0xf3280e80,                             //vcgt.f32      d0, d24, d0
  0xf4e38c9f,                             //vld1.32       {d24[]}, [r3 :32]
  0xf268a1b8,                             //vorr          d26, d24, d24
  0xf242acb4,                             //vfma.f32      d26, d18, d20
  0xf26821b8,                             //vorr          d18, d24, d24
  0xe4913004,                             //ldr           r3, [r1], #4
  0xf2432cb7,                             //vfma.f32      d18, d19, d23
  0xf2458cb1,                             //vfma.f32      d24, d21, d17
  0xf31001ba,                             //vbsl          d0, d16, d26
  0xf31611b2,                             //vbsl          d1, d22, d18
  0xf31921b8,                             //vbsl          d2, d25, d24
  0xecbd8b02,                             //vpop          {d8}
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_to_srgb_vfp4[] = {
  0xed2d8b02,                             //vpush         {d8}
  0xf3fb0580,                             //vrsqrte.f32   d16, d0
  0xe2823050,                             //add           r3, r2, #80
  0xf3fb1581,                             //vrsqrte.f32   d17, d1
  0xed928a12,                             //vldr          s16, [r2, #72]
  0xf3fb2582,                             //vrsqrte.f32   d18, d2
  0xf3403db0,                             //vmul.f32      d19, d16, d16
  0xf3414db1,                             //vmul.f32      d20, d17, d17
  0xf3425db2,                             //vmul.f32      d21, d18, d18
  0xf2603f33,                             //vrsqrts.f32   d19, d0, d19
  0xf2614f34,                             //vrsqrts.f32   d20, d1, d20
  0xf2625f35,                             //vrsqrts.f32   d21, d2, d21
  0xf3400db3,                             //vmul.f32      d16, d16, d19
  0xf3411db4,                             //vmul.f32      d17, d17, d20
  0xf3422db5,                             //vmul.f32      d18, d18, d21
  0xf3fb3520,                             //vrecpe.f32    d19, d16
  0xf3fb4521,                             //vrecpe.f32    d20, d17
  0xf3fb6522,                             //vrecpe.f32    d22, d18
  0xf3fb55a2,                             //vrsqrte.f32   d21, d18
  0xf3fb75a0,                             //vrsqrte.f32   d23, d16
  0xf3fb85a1,                             //vrsqrte.f32   d24, d17
  0xf2409fb3,                             //vrecps.f32    d25, d16, d19
  0xf241afb4,                             //vrecps.f32    d26, d17, d20
  0xf242bfb6,                             //vrecps.f32    d27, d18, d22
  0xf345cdb5,                             //vmul.f32      d28, d21, d21
  0xf347ddb7,                             //vmul.f32      d29, d23, d23
  0xf348edb8,                             //vmul.f32      d30, d24, d24
  0xf2622fbc,                             //vrsqrts.f32   d18, d18, d28
  0xf2600fbd,                             //vrsqrts.f32   d16, d16, d29
  0xf2611fbe,                             //vrsqrts.f32   d17, d17, d30
  0xf3433db9,                             //vmul.f32      d19, d19, d25
  0xf4e39c9f,                             //vld1.32       {d25[]}, [r3 :32]
  0xe2823054,                             //add           r3, r2, #84
  0xf3444dba,                             //vmul.f32      d20, d20, d26
  0xf3466dbb,                             //vmul.f32      d22, d22, d27
  0xf4e3ac9f,                             //vld1.32       {d26[]}, [r3 :32]
  0xe282304c,                             //add           r3, r2, #76
  0xf26ab1ba,                             //vorr          d27, d26, d26
  0xf249bcb3,                             //vfma.f32      d27, d25, d19
  0xf26a31ba,                             //vorr          d19, d26, d26
  0xf2493cb4,                             //vfma.f32      d19, d25, d20
  0xf4e34c9f,                             //vld1.32       {d20[]}, [r3 :32]
  0xf249acb6,                             //vfma.f32      d26, d25, d22
  0xe2823058,                             //add           r3, r2, #88
  0xf3452db2,                             //vmul.f32      d18, d21, d18
  0xf3470db0,                             //vmul.f32      d16, d23, d16
  0xf3481db1,                             //vmul.f32      d17, d24, d17
  0xf2e05948,                             //vmul.f32      d21, d0, d8[0]
  0xf244bcb0,                             //vfma.f32      d27, d20, d16
  0xf4e30c9f,                             //vld1.32       {d16[]}, [r3 :32]
  0xf2443cb1,                             //vfma.f32      d19, d20, d17
  0xf244acb2,                             //vfma.f32      d26, d20, d18
  0xf4e24c9f,                             //vld1.32       {d20[]}, [r2 :32]
  0xf2e11948,                             //vmul.f32      d17, d1, d8[0]
  0xf2e22948,                             //vmul.f32      d18, d2, d8[0]
  0xf3201e81,                             //vcgt.f32      d1, d16, d1
  0xe4913004,                             //ldr           r3, [r1], #4
  0xf3200e80,                             //vcgt.f32      d0, d16, d0
  0xf3202e82,                             //vcgt.f32      d2, d16, d2
  0xf2640fab,                             //vmin.f32      d16, d20, d27
  0xf2643fa3,                             //vmin.f32      d19, d20, d19
  0xf2644faa,                             //vmin.f32      d20, d20, d26
  0xf31501b0,                             //vbsl          d0, d21, d16
  0xf31111b3,                             //vbsl          d1, d17, d19
  0xf31221b4,                             //vbsl          d2, d18, d20
  0xecbd8b02,                             //vpop          {d8}
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_scale_1_float_vfp4[] = {
  0xed2d8b02,                             //vpush         {d8}
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xe2811008,                             //add           r1, r1, #8
  0xed938a00,                             //vldr          s16, [r3]
  0xf2a00948,                             //vmul.f32      d0, d0, d8[0]
  0xf2a11948,                             //vmul.f32      d1, d1, d8[0]
  0xf2a22948,                             //vmul.f32      d2, d2, d8[0]
  0xf2a33948,                             //vmul.f32      d3, d3, d8[0]
  0xecbd8b02,                             //vpop          {d8}
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_scale_u8_vfp4[] = {
  0xed2d8b02,                             //vpush         {d8}
  0xe24dd008,                             //sub           sp, sp, #8
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xe2811008,                             //add           r1, r1, #8
  0xe5933000,                             //ldr           r3, [r3]
  0xe0833000,                             //add           r3, r3, r0
  0xe1d330b0,                             //ldrh          r3, [r3]
  0xe1cd30b4,                             //strh          r3, [sp, #4]
  0xe28d3004,                             //add           r3, sp, #4
  0xed928a03,                             //vldr          s16, [r2, #12]
  0xf4e3041f,                             //vld1.16       {d16[0]}, [r3 :16]
  0xf3c80a30,                             //vmovl.u8      q8, d16
  0xf3d00a30,                             //vmovl.u16     q8, d16
  0xf3fb06a0,                             //vcvt.f32.u32  d16, d16
  0xf2e009c8,                             //vmul.f32      d16, d16, d8[0]
  0xf3000d90,                             //vmul.f32      d0, d16, d0
  0xf3001d91,                             //vmul.f32      d1, d16, d1
  0xf3002d92,                             //vmul.f32      d2, d16, d2
  0xf3003d93,                             //vmul.f32      d3, d16, d3
  0xe28dd008,                             //add           sp, sp, #8
  0xecbd8b02,                             //vpop          {d8}
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_lerp_1_float_vfp4[] = {
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xf2600d04,                             //vsub.f32      d16, d0, d4
  0xf2611d05,                             //vsub.f32      d17, d1, d5
  0xf2622d06,                             //vsub.f32      d18, d2, d6
  0xe2811008,                             //add           r1, r1, #8
  0xf2633d07,                             //vsub.f32      d19, d3, d7
  0xf4e34c9f,                             //vld1.32       {d20[]}, [r3 :32]
  0xf2240114,                             //vorr          d0, d4, d4
  0xf2251115,                             //vorr          d1, d5, d5
  0xf2262116,                             //vorr          d2, d6, d6
  0xf2273117,                             //vorr          d3, d7, d7
  0xf2000cb4,                             //vfma.f32      d0, d16, d20
  0xf2011cb4,                             //vfma.f32      d1, d17, d20
  0xf2022cb4,                             //vfma.f32      d2, d18, d20
  0xf2033cb4,                             //vfma.f32      d3, d19, d20
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_lerp_u8_vfp4[] = {
  0xed2d8b02,                             //vpush         {d8}
  0xe24dd008,                             //sub           sp, sp, #8
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xf2612d05,                             //vsub.f32      d18, d1, d5
  0xf2623d06,                             //vsub.f32      d19, d2, d6
  0xf2634d07,                             //vsub.f32      d20, d3, d7
  0xe2811008,                             //add           r1, r1, #8
  0xe5933000,                             //ldr           r3, [r3]
  0xf2251115,                             //vorr          d1, d5, d5
  0xf2262116,                             //vorr          d2, d6, d6
  0xe0833000,                             //add           r3, r3, r0
  0xf2273117,                             //vorr          d3, d7, d7
  0xe1d330b0,                             //ldrh          r3, [r3]
  0xe1cd30b4,                             //strh          r3, [sp, #4]
  0xe28d3004,                             //add           r3, sp, #4
  0xed928a03,                             //vldr          s16, [r2, #12]
  0xf4e3041f,                             //vld1.16       {d16[0]}, [r3 :16]
  0xf3c80a30,                             //vmovl.u8      q8, d16
  0xf3d00a30,                             //vmovl.u16     q8, d16
  0xf3fb06a0,                             //vcvt.f32.u32  d16, d16
  0xf2601d04,                             //vsub.f32      d17, d0, d4
  0xf2240114,                             //vorr          d0, d4, d4
  0xf2e009c8,                             //vmul.f32      d16, d16, d8[0]
  0xf2010cb0,                             //vfma.f32      d0, d17, d16
  0xf2021cb0,                             //vfma.f32      d1, d18, d16
  0xf2032cb0,                             //vfma.f32      d2, d19, d16
  0xf2043cb0,                             //vfma.f32      d3, d20, d16
  0xe28dd008,                             //add           sp, sp, #8
  0xecbd8b02,                             //vpop          {d8}
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_lerp_565_vfp4[] = {
  0xed2d8b04,                             //vpush         {d8-d9}
  0xe24dd008,                             //sub           sp, sp, #8
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xf2603d04,                             //vsub.f32      d19, d0, d4
  0xf2240114,                             //vorr          d0, d4, d4
  0xe2811008,                             //add           r1, r1, #8
  0xe5933000,                             //ldr           r3, [r3]
  0xe7933080,                             //ldr           r3, [r3, r0, lsl #1]
  0xe58d3004,                             //str           r3, [sp, #4]
  0xe28d3004,                             //add           r3, sp, #4
  0xed923a1d,                             //vldr          s6, [r2, #116]
  0xf4e3083f,                             //vld1.32       {d16[0]}, [r3 :32]
  0xe282306c,                             //add           r3, r2, #108
  0xf4e31c9f,                             //vld1.32       {d17[]}, [r3 :32]
  0xe2823068,                             //add           r3, r2, #104
  0xf3d04a30,                             //vmovl.u16     q10, d16
  0xf4e32c9f,                             //vld1.32       {d18[]}, [r3 :32]
  0xe2823070,                             //add           r3, r2, #112
  0xf24201b4,                             //vand          d16, d18, d20
  0xf4e32c9f,                             //vld1.32       {d18[]}, [r3 :32]
  0xf24221b4,                             //vand          d18, d18, d20
  0xf24111b4,                             //vand          d17, d17, d20
  0xf3fb0620,                             //vcvt.f32.s32  d16, d16
  0xed928a1e,                             //vldr          s16, [r2, #120]
  0xf3fb1621,                             //vcvt.f32.s32  d17, d17
  0xed929a1f,                             //vldr          s18, [r2, #124]
  0xf3fb2622,                             //vcvt.f32.s32  d18, d18
  0xf2614d05,                             //vsub.f32      d20, d1, d5
  0xf2e009c3,                             //vmul.f32      d16, d16, d3[0]
  0xf4a23c9f,                             //vld1.32       {d3[]}, [r2 :32]
  0xf2625d06,                             //vsub.f32      d21, d2, d6
  0xf2e119c8,                             //vmul.f32      d17, d17, d8[0]
  0xf2e229c9,                             //vmul.f32      d18, d18, d9[0]
  0xf2251115,                             //vorr          d1, d5, d5
  0xf2262116,                             //vorr          d2, d6, d6
  0xf2030cb0,                             //vfma.f32      d0, d19, d16
  0xf2041cb1,                             //vfma.f32      d1, d20, d17
  0xf2052cb2,                             //vfma.f32      d2, d21, d18
  0xe28dd008,                             //add           sp, sp, #8
  0xecbd8b04,                             //vpop          {d8-d9}
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_load_tables_vfp4[] = {
  0xe92d48f0,                             //push          {r4, r5, r6, r7, fp, lr}
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xe2826010,                             //add           r6, r2, #16
  0xe2811008,                             //add           r1, r1, #8
  0xe593e000,                             //ldr           lr, [r3]
  0xe99300b0,                             //ldmib         r3, {r4, r5, r7}
  0xf4e60c9f,                             //vld1.32       {d16[]}, [r6 :32]
  0xe08e6100,                             //add           r6, lr, r0, lsl #2
  0xedd61b00,                             //vldr          d17, [r6]
  0xf24021b1,                             //vand          d18, d16, d17
  0xed922a03,                             //vldr          s4, [r2, #12]
  0xf3f03031,                             //vshr.u32      d19, d17, #16
  0xee326b90,                             //vmov.32       r6, d18[1]
  0xe0846106,                             //add           r6, r4, r6, lsl #2
  0xedd60a00,                             //vldr          s1, [r6]
  0xee126b90,                             //vmov.32       r6, d18[0]
  0xf3f82031,                             //vshr.u32      d18, d17, #8
  0xf24021b2,                             //vand          d18, d16, d18
  0xf24001b3,                             //vand          d16, d16, d19
  0xee103b90,                             //vmov.32       r3, d16[0]
  0xe0846106,                             //add           r6, r4, r6, lsl #2
  0xee304b90,                             //vmov.32       r4, d16[1]
  0xf3e80031,                             //vshr.u32      d16, d17, #24
  0xed960a00,                             //vldr          s0, [r6]
  0xee326b90,                             //vmov.32       r6, d18[1]
  0xf3fb0620,                             //vcvt.f32.s32  d16, d16
  0xe0873103,                             //add           r3, r7, r3, lsl #2
  0xf2a039c2,                             //vmul.f32      d3, d16, d2[0]
  0xe0874104,                             //add           r4, r7, r4, lsl #2
  0xedd42a00,                             //vldr          s5, [r4]
  0xe0856106,                             //add           r6, r5, r6, lsl #2
  0xed932a00,                             //vldr          s4, [r3]
  0xedd61a00,                             //vldr          s3, [r6]
  0xee126b90,                             //vmov.32       r6, d18[0]
  0xe0856106,                             //add           r6, r5, r6, lsl #2
  0xed961a00,                             //vldr          s2, [r6]
  0xe8bd48f0,                             //pop           {r4, r5, r6, r7, fp, lr}
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_load_a8_vfp4[] = {
  0xe24dd004,                             //sub           sp, sp, #4
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xe2811008,                             //add           r1, r1, #8
  0xf2801010,                             //vmov.i32      d1, #0
  0xf2802010,                             //vmov.i32      d2, #0
  0xe5933000,                             //ldr           r3, [r3]
  0xe0833000,                             //add           r3, r3, r0
  0xe1d330b0,                             //ldrh          r3, [r3]
  0xe1cd30b0,                             //strh          r3, [sp]
  0xe1a0300d,                             //mov           r3, sp
  0xf4e3041f,                             //vld1.16       {d16[0]}, [r3 :16]
  0xed920a03,                             //vldr          s0, [r2, #12]
  0xf3c80a30,                             //vmovl.u8      q8, d16
  0xf3d00a30,                             //vmovl.u16     q8, d16
  0xf3fb06a0,                             //vcvt.f32.u32  d16, d16
  0xf2a039c0,                             //vmul.f32      d3, d16, d0[0]
  0xf2800010,                             //vmov.i32      d0, #0
  0xe28dd004,                             //add           sp, sp, #4
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_store_a8_vfp4[] = {
  0xe92d4800,                             //push          {fp, lr}
  0xe2823008,                             //add           r3, r2, #8
  0xf2c3061f,                             //vmov.i32      d16, #1056964608
  0xf4e31c9f,                             //vld1.32       {d17[]}, [r3 :32]
  0xe5913000,                             //ldr           r3, [r1]
  0xf2430c31,                             //vfma.f32      d16, d3, d17
  0xe5933000,                             //ldr           r3, [r3]
  0xf3fb07a0,                             //vcvt.u32.f32  d16, d16
  0xee10eb90,                             //vmov.32       lr, d16[0]
  0xee30cb90,                             //vmov.32       ip, d16[1]
  0xe7e3e000,                             //strb          lr, [r3, r0]!
  0xe5c3c001,                             //strb          ip, [r3, #1]
  0xe5913004,                             //ldr           r3, [r1, #4]
  0xe2811008,                             //add           r1, r1, #8
  0xe8bd4800,                             //pop           {fp, lr}
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_load_565_vfp4[] = {
  0xe24dd004,                             //sub           sp, sp, #4
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xe2811008,                             //add           r1, r1, #8
  0xe5933000,                             //ldr           r3, [r3]
  0xe7933080,                             //ldr           r3, [r3, r0, lsl #1]
  0xe58d3000,                             //str           r3, [sp]
  0xe1a0300d,                             //mov           r3, sp
  0xf4e3083f,                             //vld1.32       {d16[0]}, [r3 :32]
  0xe282306c,                             //add           r3, r2, #108
  0xf4e31c9f,                             //vld1.32       {d17[]}, [r3 :32]
  0xe2823068,                             //add           r3, r2, #104
  0xf3d04a30,                             //vmovl.u16     q10, d16
  0xf4e32c9f,                             //vld1.32       {d18[]}, [r3 :32]
  0xe2823070,                             //add           r3, r2, #112
  0xf24201b4,                             //vand          d16, d18, d20
  0xf4e32c9f,                             //vld1.32       {d18[]}, [r3 :32]
  0xf24111b4,                             //vand          d17, d17, d20
  0xf24221b4,                             //vand          d18, d18, d20
  0xf4a23c9f,                             //vld1.32       {d3[]}, [r2 :32]
  0xf3fb0620,                             //vcvt.f32.s32  d16, d16
  0xf3fb1621,                             //vcvt.f32.s32  d17, d17
  0xf3fb2622,                             //vcvt.f32.s32  d18, d18
  0xed920a1d,                             //vldr          s0, [r2, #116]
  0xed921a1e,                             //vldr          s2, [r2, #120]
  0xed922a1f,                             //vldr          s4, [r2, #124]
  0xf2a009c0,                             //vmul.f32      d0, d16, d0[0]
  0xf2a119c1,                             //vmul.f32      d1, d17, d1[0]
  0xf2a229c2,                             //vmul.f32      d2, d18, d2[0]
  0xe28dd004,                             //add           sp, sp, #4
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_store_565_vfp4[] = {
  0xe2823080,                             //add           r3, r2, #128
  0xf2c3361f,                             //vmov.i32      d19, #1056964608
  0xf2c3461f,                             //vmov.i32      d20, #1056964608
  0xf4e31c9f,                             //vld1.32       {d17[]}, [r3 :32]
  0xe2823084,                             //add           r3, r2, #132
  0xf2403c31,                             //vfma.f32      d19, d0, d17
  0xf4e32c9f,                             //vld1.32       {d18[]}, [r3 :32]
  0xf2c3061f,                             //vmov.i32      d16, #1056964608
  0xf2414c32,                             //vfma.f32      d20, d1, d18
  0xf2420c31,                             //vfma.f32      d16, d2, d17
  0xe5913000,                             //ldr           r3, [r1]
  0xe5933000,                             //ldr           r3, [r3]
  0xf3fb17a3,                             //vcvt.u32.f32  d17, d19
  0xe0833080,                             //add           r3, r3, r0, lsl #1
  0xf3fb27a4,                             //vcvt.u32.f32  d18, d20
  0xf3fb07a0,                             //vcvt.u32.f32  d16, d16
  0xf2eb1531,                             //vshl.s32      d17, d17, #11
  0xf2e52532,                             //vshl.s32      d18, d18, #5
  0xf26101b0,                             //vorr          d16, d17, d16
  0xf26001b2,                             //vorr          d16, d16, d18
  0xf3f60121,                             //vuzp.16       d16, d17
  0xf4c3080f,                             //vst1.32       {d16[0]}, [r3]
  0xe5913004,                             //ldr           r3, [r1, #4]
  0xe2811008,                             //add           r1, r1, #8
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_load_8888_vfp4[] = {
  0xe92d4800,                             //push          {fp, lr}
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xe2811008,                             //add           r1, r1, #8
  0xed922a03,                             //vldr          s4, [r2, #12]
  0xe593e000,                             //ldr           lr, [r3]
  0xe2823010,                             //add           r3, r2, #16
  0xf4e30c9f,                             //vld1.32       {d16[]}, [r3 :32]
  0xe08e3100,                             //add           r3, lr, r0, lsl #2
  0xedd31b00,                             //vldr          d17, [r3]
  0xf24021b1,                             //vand          d18, d16, d17
  0xf3f83031,                             //vshr.u32      d19, d17, #8
  0xf3e84031,                             //vshr.u32      d20, d17, #24
  0xf3f01031,                             //vshr.u32      d17, d17, #16
  0xf24031b3,                             //vand          d19, d16, d19
  0xf24001b1,                             //vand          d16, d16, d17
  0xf3fb2622,                             //vcvt.f32.s32  d18, d18
  0xf3fb4624,                             //vcvt.f32.s32  d20, d20
  0xf3fb1623,                             //vcvt.f32.s32  d17, d19
  0xf3fb0620,                             //vcvt.f32.s32  d16, d16
  0xf2a209c2,                             //vmul.f32      d0, d18, d2[0]
  0xf2a439c2,                             //vmul.f32      d3, d20, d2[0]
  0xf2a119c2,                             //vmul.f32      d1, d17, d2[0]
  0xf2a029c2,                             //vmul.f32      d2, d16, d2[0]
  0xe8bd4800,                             //pop           {fp, lr}
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_store_8888_vfp4[] = {
  0xe2823008,                             //add           r3, r2, #8
  0xf2c3261f,                             //vmov.i32      d18, #1056964608
  0xf2c3361f,                             //vmov.i32      d19, #1056964608
  0xf4e31c9f,                             //vld1.32       {d17[]}, [r3 :32]
  0xf2c3061f,                             //vmov.i32      d16, #1056964608
  0xf2412c31,                             //vfma.f32      d18, d1, d17
  0xf2423c31,                             //vfma.f32      d19, d2, d17
  0xf2c3461f,                             //vmov.i32      d20, #1056964608
  0xe5913000,                             //ldr           r3, [r1]
  0xf2400c31,                             //vfma.f32      d16, d0, d17
  0xf2434c31,                             //vfma.f32      d20, d3, d17
  0xe5933000,                             //ldr           r3, [r3]
  0xe0833100,                             //add           r3, r3, r0, lsl #2
  0xf3fb17a2,                             //vcvt.u32.f32  d17, d18
  0xf3fb27a3,                             //vcvt.u32.f32  d18, d19
  0xf3fb07a0,                             //vcvt.u32.f32  d16, d16
  0xf3fb37a4,                             //vcvt.u32.f32  d19, d20
  0xf2e81531,                             //vshl.s32      d17, d17, #8
  0xf2f02532,                             //vshl.s32      d18, d18, #16
  0xf26101b0,                             //vorr          d16, d17, d16
  0xf2f81533,                             //vshl.s32      d17, d19, #24
  0xf26001b2,                             //vorr          d16, d16, d18
  0xf26001b1,                             //vorr          d16, d16, d17
  0xedc30b00,                             //vstr          d16, [r3]
  0xe5913004,                             //ldr           r3, [r1, #4]
  0xe2811008,                             //add           r1, r1, #8
  0xe12fff13,                             //bx            r3
};

CODE const uint32_t sk_load_f16_vfp4[] = {
  0xed2d8b04,                             //vpush         {d8-d9}
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xe2811008,                             //add           r1, r1, #8
  0xe5933000,                             //ldr           r3, [r3]
  0xe0833180,                             //add           r3, r3, r0, lsl #3
  0xf463084f,                             //vld2.16       {d16-d17}, [r3]
  0xf3b62720,                             //vcvt.f32.f16  q1, d16
  0xf3b68721,                             //vcvt.f32.f16  q4, d17
  0xf2220112,                             //vorr          d0, d2, d2
  0xeef00a43,                             //vmov.f32      s1, s6
  0xf2281118,                             //vorr          d1, d8, d8
  0xeeb03a62,                             //vmov.f32      s6, s5
  0xeef01a49,                             //vmov.f32      s3, s18
  0xeeb09a68,                             //vmov.f32      s18, s17
  0xeeb02b43,                             //vmov.f64      d2, d3
  0xeeb03b49,                             //vmov.f64      d3, d9
  0xecbd8b04,                             //vpop          {d8-d9}
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_store_f16_vfp4[] = {
  0xeef00b41,                             //vmov.f64      d16, d1
  0xeef03b42,                             //vmov.f64      d19, d2
  0xf2631113,                             //vorr          d17, d3, d3
  0xf2602110,                             //vorr          d18, d0, d0
  0xf3fa00a1,                             //vtrn.32       d16, d17
  0xf3f61620,                             //vcvt.f16.f32  d17, q8
  0xf3fa20a3,                             //vtrn.32       d18, d19
  0xe5913000,                             //ldr           r3, [r1]
  0xf3f60622,                             //vcvt.f16.f32  d16, q9
  0xe5933000,                             //ldr           r3, [r3]
  0xe0833180,                             //add           r3, r3, r0, lsl #3
  0xf443084f,                             //vst2.16       {d16-d17}, [r3]
  0xe2813008,                             //add           r3, r1, #8
  0xe591c004,                             //ldr           ip, [r1, #4]
  0xe1a01003,                             //mov           r1, r3
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_store_f32_vfp4[] = {
  0xe5913000,                             //ldr           r3, [r1]
  0xe5933000,                             //ldr           r3, [r3]
  0xe0833200,                             //add           r3, r3, r0, lsl #4
  0xf403008f,                             //vst4.32       {d0-d3}, [r3]
  0xe2813008,                             //add           r3, r1, #8
  0xe591c004,                             //ldr           ip, [r1, #4]
  0xe1a01003,                             //mov           r1, r3
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_clamp_x_vfp4[] = {
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xf2c00010,                             //vmov.i32      d16, #0
  0xf3c71e1f,                             //vmov.i8       d17, #255
  0xf2400f80,                             //vmax.f32      d16, d16, d0
  0xe2811008,                             //add           r1, r1, #8
  0xf4e32c9f,                             //vld1.32       {d18[]}, [r3 :32]
  0xf26218a1,                             //vadd.i32      d17, d18, d17
  0xf2200fa1,                             //vmin.f32      d0, d16, d17
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_clamp_y_vfp4[] = {
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xf2c00010,                             //vmov.i32      d16, #0
  0xf3c71e1f,                             //vmov.i8       d17, #255
  0xf2400f81,                             //vmax.f32      d16, d16, d1
  0xe2811008,                             //add           r1, r1, #8
  0xf4e32c9f,                             //vld1.32       {d18[]}, [r3 :32]
  0xf26218a1,                             //vadd.i32      d17, d18, d17
  0xf2201fa1,                             //vmin.f32      d1, d16, d17
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_repeat_x_vfp4[] = {
  0xed2d8b04,                             //vpush         {d8-d9}
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xf2c02010,                             //vmov.i32      d18, #0
  0xf4e23c9f,                             //vld1.32       {d19[]}, [r2 :32]
  0xe2811008,                             //add           r1, r1, #8
  0xed938a00,                             //vldr          s16, [r3]
  0xeec09a88,                             //vdiv.f32      s19, s1, s16
  0xee809a08,                             //vdiv.f32      s18, s0, s16
  0xf3fb0709,                             //vcvt.s32.f32  d16, d9
  0xf3fb0620,                             //vcvt.f32.s32  d16, d16
  0xf3601e89,                             //vcgt.f32      d17, d16, d9
  0xf35311b2,                             //vbsl          d17, d19, d18
  0xf3f42c08,                             //vdup.32       d18, d8[0]
  0xf2600da1,                             //vsub.f32      d16, d16, d17
  0xf3c71e1f,                             //vmov.i8       d17, #255
  0xf26218a1,                             //vadd.i32      d17, d18, d17
  0xf2e009c8,                             //vmul.f32      d16, d16, d8[0]
  0xf2600d20,                             //vsub.f32      d16, d0, d16
  0xf2200fa1,                             //vmin.f32      d0, d16, d17
  0xecbd8b04,                             //vpop          {d8-d9}
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_repeat_y_vfp4[] = {
  0xed2d8b04,                             //vpush         {d8-d9}
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xf2c02010,                             //vmov.i32      d18, #0
  0xf4e23c9f,                             //vld1.32       {d19[]}, [r2 :32]
  0xe2811008,                             //add           r1, r1, #8
  0xed938a00,                             //vldr          s16, [r3]
  0xeec19a88,                             //vdiv.f32      s19, s3, s16
  0xee819a08,                             //vdiv.f32      s18, s2, s16
  0xf3fb0709,                             //vcvt.s32.f32  d16, d9
  0xf3fb0620,                             //vcvt.f32.s32  d16, d16
  0xf3601e89,                             //vcgt.f32      d17, d16, d9
  0xf35311b2,                             //vbsl          d17, d19, d18
  0xf3f42c08,                             //vdup.32       d18, d8[0]
  0xf2600da1,                             //vsub.f32      d16, d16, d17
  0xf3c71e1f,                             //vmov.i8       d17, #255
  0xf26218a1,                             //vadd.i32      d17, d18, d17
  0xf2e009c8,                             //vmul.f32      d16, d16, d8[0]
  0xf2610d20,                             //vsub.f32      d16, d1, d16
  0xf2201fa1,                             //vmin.f32      d1, d16, d17
  0xecbd8b04,                             //vpop          {d8-d9}
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_mirror_x_vfp4[] = {
  0xed2d8b04,                             //vpush         {d8-d9}
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xf2c03010,                             //vmov.i32      d19, #0
  0xf4e24c9f,                             //vld1.32       {d20[]}, [r2 :32]
  0xe2811008,                             //add           r1, r1, #8
  0xed938a00,                             //vldr          s16, [r3]
  0xee389a08,                             //vadd.f32      s18, s16, s16
  0xf3f40c08,                             //vdup.32       d16, d8[0]
  0xf2200d20,                             //vsub.f32      d0, d0, d16
  0xeec08a89,                             //vdiv.f32      s17, s1, s18
  0xee808a09,                             //vdiv.f32      s16, s0, s18
  0xf3fb1708,                             //vcvt.s32.f32  d17, d8
  0xf3fb1621,                             //vcvt.f32.s32  d17, d17
  0xf3612e88,                             //vcgt.f32      d18, d17, d8
  0xf35421b3,                             //vbsl          d18, d20, d19
  0xf2611da2,                             //vsub.f32      d17, d17, d18
  0xf3c72e1f,                             //vmov.i8       d18, #255
  0xf2e119c9,                             //vmul.f32      d17, d17, d9[0]
  0xf2601d21,                             //vsub.f32      d17, d0, d17
  0xf2611da0,                             //vsub.f32      d17, d17, d16
  0xf26008a2,                             //vadd.i32      d16, d16, d18
  0xf3f91721,                             //vabs.f32      d17, d17
  0xf2210fa0,                             //vmin.f32      d0, d17, d16
  0xecbd8b04,                             //vpop          {d8-d9}
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_mirror_y_vfp4[] = {
  0xed2d8b04,                             //vpush         {d8-d9}
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xf2c03010,                             //vmov.i32      d19, #0
  0xf4e24c9f,                             //vld1.32       {d20[]}, [r2 :32]
  0xe2811008,                             //add           r1, r1, #8
  0xed938a00,                             //vldr          s16, [r3]
  0xee389a08,                             //vadd.f32      s18, s16, s16
  0xf3f40c08,                             //vdup.32       d16, d8[0]
  0xf2211d20,                             //vsub.f32      d1, d1, d16
  0xeec18a89,                             //vdiv.f32      s17, s3, s18
  0xee818a09,                             //vdiv.f32      s16, s2, s18
  0xf3fb1708,                             //vcvt.s32.f32  d17, d8
  0xf3fb1621,                             //vcvt.f32.s32  d17, d17
  0xf3612e88,                             //vcgt.f32      d18, d17, d8
  0xf35421b3,                             //vbsl          d18, d20, d19
  0xf2611da2,                             //vsub.f32      d17, d17, d18
  0xf3c72e1f,                             //vmov.i8       d18, #255
  0xf2e119c9,                             //vmul.f32      d17, d17, d9[0]
  0xf2611d21,                             //vsub.f32      d17, d1, d17
  0xf2611da0,                             //vsub.f32      d17, d17, d16
  0xf26008a2,                             //vadd.i32      d16, d16, d18
  0xf3f91721,                             //vabs.f32      d17, d17
  0xf2211fa0,                             //vmin.f32      d1, d17, d16
  0xecbd8b04,                             //vpop          {d8-d9}
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_matrix_2x3_vfp4[] = {
  0xe92d4800,                             //push          {fp, lr}
  0xe591e000,                             //ldr           lr, [r1]
  0xe591c004,                             //ldr           ip, [r1, #4]
  0xe2811008,                             //add           r1, r1, #8
  0xe28e300c,                             //add           r3, lr, #12
  0xf4e32c9f,                             //vld1.32       {d18[]}, [r3 :32]
  0xe28e3008,                             //add           r3, lr, #8
  0xf4e31c9f,                             //vld1.32       {d17[]}, [r3 :32]
  0xe28e3010,                             //add           r3, lr, #16
  0xf4e30c9f,                             //vld1.32       {d16[]}, [r3 :32]
  0xe28e3014,                             //add           r3, lr, #20
  0xf2410c31,                             //vfma.f32      d16, d1, d17
  0xf4e31c9f,                             //vld1.32       {d17[]}, [r3 :32]
  0xe28e3004,                             //add           r3, lr, #4
  0xf2411c32,                             //vfma.f32      d17, d1, d18
  0xf4ee2c9f,                             //vld1.32       {d18[]}, [lr :32]
  0xf4e33c9f,                             //vld1.32       {d19[]}, [r3 :32]
  0xf2400c32,                             //vfma.f32      d16, d0, d18
  0xf2401c33,                             //vfma.f32      d17, d0, d19
  0xf22001b0,                             //vorr          d0, d16, d16
  0xf22111b1,                             //vorr          d1, d17, d17
  0xe8bd4800,                             //pop           {fp, lr}
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_matrix_3x4_vfp4[] = {
  0xe92d4800,                             //push          {fp, lr}
  0xe591e000,                             //ldr           lr, [r1]
  0xe591c004,                             //ldr           ip, [r1, #4]
  0xe2811008,                             //add           r1, r1, #8
  0xe28e3020,                             //add           r3, lr, #32
  0xf4e33c9f,                             //vld1.32       {d19[]}, [r3 :32]
  0xe28e302c,                             //add           r3, lr, #44
  0xf4e30c9f,                             //vld1.32       {d16[]}, [r3 :32]
  0xe28e301c,                             //add           r3, lr, #28
  0xf2420c33,                             //vfma.f32      d16, d2, d19
  0xf4e34c9f,                             //vld1.32       {d20[]}, [r3 :32]
  0xe28e3018,                             //add           r3, lr, #24
  0xf4e32c9f,                             //vld1.32       {d18[]}, [r3 :32]
  0xe28e3024,                             //add           r3, lr, #36
  0xf4e31c9f,                             //vld1.32       {d17[]}, [r3 :32]
  0xe28e3028,                             //add           r3, lr, #40
  0xf2421c32,                             //vfma.f32      d17, d2, d18
  0xf4e32c9f,                             //vld1.32       {d18[]}, [r3 :32]
  0xe28e3010,                             //add           r3, lr, #16
  0xf2422c34,                             //vfma.f32      d18, d2, d20
  0xf4e33c9f,                             //vld1.32       {d19[]}, [r3 :32]
  0xe28e300c,                             //add           r3, lr, #12
  0xf4e34c9f,                             //vld1.32       {d20[]}, [r3 :32]
  0xe28e3014,                             //add           r3, lr, #20
  0xf2411c34,                             //vfma.f32      d17, d1, d20
  0xf4e34c9f,                             //vld1.32       {d20[]}, [r3 :32]
  0xf2410c34,                             //vfma.f32      d16, d1, d20
  0xe28e3004,                             //add           r3, lr, #4
  0xf2412c33,                             //vfma.f32      d18, d1, d19
  0xf4ee3c9f,                             //vld1.32       {d19[]}, [lr :32]
  0xf4e34c9f,                             //vld1.32       {d20[]}, [r3 :32]
  0xe28e3008,                             //add           r3, lr, #8
  0xf2401c33,                             //vfma.f32      d17, d0, d19
  0xf4e33c9f,                             //vld1.32       {d19[]}, [r3 :32]
  0xf2400c33,                             //vfma.f32      d16, d0, d19
  0xf2402c34,                             //vfma.f32      d18, d0, d20
  0xf22101b1,                             //vorr          d0, d17, d17
  0xf22021b0,                             //vorr          d2, d16, d16
  0xf22211b2,                             //vorr          d1, d18, d18
  0xe8bd4800,                             //pop           {fp, lr}
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_matrix_perspective_vfp4[] = {
  0xe92d4800,                             //push          {fp, lr}
  0xe591e000,                             //ldr           lr, [r1]
  0xe591c004,                             //ldr           ip, [r1, #4]
  0xe2811008,                             //add           r1, r1, #8
  0xe28e301c,                             //add           r3, lr, #28
  0xf4e30c9f,                             //vld1.32       {d16[]}, [r3 :32]
  0xe28e3020,                             //add           r3, lr, #32
  0xf4e31c9f,                             //vld1.32       {d17[]}, [r3 :32]
  0xe28e3018,                             //add           r3, lr, #24
  0xf2411c30,                             //vfma.f32      d17, d1, d16
  0xf4e30c9f,                             //vld1.32       {d16[]}, [r3 :32]
  0xe28e3010,                             //add           r3, lr, #16
  0xf2401c30,                             //vfma.f32      d17, d0, d16
  0xf4e30c9f,                             //vld1.32       {d16[]}, [r3 :32]
  0xe28e3004,                             //add           r3, lr, #4
  0xf4e32c9f,                             //vld1.32       {d18[]}, [r3 :32]
  0xe28e3008,                             //add           r3, lr, #8
  0xf4e34c9f,                             //vld1.32       {d20[]}, [r3 :32]
  0xe28e3014,                             //add           r3, lr, #20
  0xf2414c32,                             //vfma.f32      d20, d1, d18
  0xf4e32c9f,                             //vld1.32       {d18[]}, [r3 :32]
  0xe28e300c,                             //add           r3, lr, #12
  0xf3fb3521,                             //vrecpe.f32    d19, d17
  0xf2412c30,                             //vfma.f32      d18, d1, d16
  0xf4e35c9f,                             //vld1.32       {d21[]}, [r3 :32]
  0xf2410fb3,                             //vrecps.f32    d16, d17, d19
  0xf4ee1c9f,                             //vld1.32       {d17[]}, [lr :32]
  0xf2404c31,                             //vfma.f32      d20, d0, d17
  0xf2402c35,                             //vfma.f32      d18, d0, d21
  0xf3430db0,                             //vmul.f32      d16, d19, d16
  0xf3040db0,                             //vmul.f32      d0, d20, d16
  0xf3021db0,                             //vmul.f32      d1, d18, d16
  0xe8bd4800,                             //pop           {fp, lr}
  0xe12fff1c,                             //bx            ip
};

CODE const uint32_t sk_linear_gradient_2stops_vfp4[] = {
  0xe8911008,                             //ldm           r1, {r3, ip}
  0xe2811008,                             //add           r1, r1, #8
  0xf4632a0d,                             //vld1.8        {d18-d19}, [r3]!
  0xf4634a0f,                             //vld1.8        {d20-d21}, [r3]
  0xf3f40c22,                             //vdup.32       d16, d18[0]
  0xf3f41c24,                             //vdup.32       d17, d20[0]
  0xf2400c31,                             //vfma.f32      d16, d0, d17
  0xf3fc6c24,                             //vdup.32       d22, d20[1]
  0xf3bc1c22,                             //vdup.32       d1, d18[1]
  0xf3b42c23,                             //vdup.32       d2, d19[0]
  0xf2001c36,                             //vfma.f32      d1, d0, d22
  0xf3f41c25,                             //vdup.32       d17, d21[0]
  0xf3fc4c25,                             //vdup.32       d20, d21[1]
  0xf2002c31,                             //vfma.f32      d2, d0, d17
  0xf3bc3c23,                             //vdup.32       d3, d19[1]
  0xf2003c34,                             //vfma.f32      d3, d0, d20
  0xf22001b0,                             //vorr          d0, d16, d16
  0xe12fff1c,                             //bx            ip
};
#elif defined(__x86_64__)

CODE const uint8_t sk_start_pipeline_hsw[] = {
  65,87,                                  //push          %r15
  65,86,                                  //push          %r14
  65,85,                                  //push          %r13
  65,84,                                  //push          %r12
  83,                                     //push          %rbx
  73,137,205,                             //mov           %rcx,%r13
  73,137,214,                             //mov           %rdx,%r14
  72,137,251,                             //mov           %rdi,%rbx
  72,173,                                 //lods          %ds:(%rsi),%rax
  73,137,199,                             //mov           %rax,%r15
  73,137,244,                             //mov           %rsi,%r12
  72,141,67,8,                            //lea           0x8(%rbx),%rax
  76,57,232,                              //cmp           %r13,%rax
  118,5,                                  //jbe           28 <_sk_start_pipeline_hsw+0x28>
  72,137,223,                             //mov           %rbx,%rdi
  235,65,                                 //jmp           69 <_sk_start_pipeline_hsw+0x69>
  185,0,0,0,0,                            //mov           $0x0,%ecx
  197,252,87,192,                         //vxorps        %ymm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  197,236,87,210,                         //vxorps        %ymm2,%ymm2,%ymm2
  197,228,87,219,                         //vxorps        %ymm3,%ymm3,%ymm3
  197,220,87,228,                         //vxorps        %ymm4,%ymm4,%ymm4
  197,212,87,237,                         //vxorps        %ymm5,%ymm5,%ymm5
  197,204,87,246,                         //vxorps        %ymm6,%ymm6,%ymm6
  197,196,87,255,                         //vxorps        %ymm7,%ymm7,%ymm7
  72,137,223,                             //mov           %rbx,%rdi
  76,137,230,                             //mov           %r12,%rsi
  76,137,242,                             //mov           %r14,%rdx
  65,255,215,                             //callq         *%r15
  72,141,123,8,                           //lea           0x8(%rbx),%rdi
  72,131,195,16,                          //add           $0x10,%rbx
  76,57,235,                              //cmp           %r13,%rbx
  72,137,251,                             //mov           %rdi,%rbx
  118,191,                                //jbe           28 <_sk_start_pipeline_hsw+0x28>
  76,137,233,                             //mov           %r13,%rcx
  72,41,249,                              //sub           %rdi,%rcx
  116,41,                                 //je            9a <_sk_start_pipeline_hsw+0x9a>
  197,252,87,192,                         //vxorps        %ymm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  197,236,87,210,                         //vxorps        %ymm2,%ymm2,%ymm2
  197,228,87,219,                         //vxorps        %ymm3,%ymm3,%ymm3
  197,220,87,228,                         //vxorps        %ymm4,%ymm4,%ymm4
  197,212,87,237,                         //vxorps        %ymm5,%ymm5,%ymm5
  197,204,87,246,                         //vxorps        %ymm6,%ymm6,%ymm6
  197,196,87,255,                         //vxorps        %ymm7,%ymm7,%ymm7
  76,137,230,                             //mov           %r12,%rsi
  76,137,242,                             //mov           %r14,%rdx
  65,255,215,                             //callq         *%r15
  76,137,232,                             //mov           %r13,%rax
  91,                                     //pop           %rbx
  65,92,                                  //pop           %r12
  65,93,                                  //pop           %r13
  65,94,                                  //pop           %r14
  65,95,                                  //pop           %r15
  197,248,119,                            //vzeroupper
  195,                                    //retq
};

CODE const uint8_t sk_just_return_hsw[] = {
  195,                                    //retq
};

CODE const uint8_t sk_seed_shader_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,249,110,199,                        //vmovd         %edi,%xmm0
  196,226,125,24,192,                     //vbroadcastss  %xmm0,%ymm0
  197,252,91,192,                         //vcvtdq2ps     %ymm0,%ymm0
  196,226,125,24,74,4,                    //vbroadcastss  0x4(%rdx),%ymm1
  197,252,88,193,                         //vaddps        %ymm1,%ymm0,%ymm0
  197,252,88,66,20,                       //vaddps        0x14(%rdx),%ymm0,%ymm0
  196,226,125,24,16,                      //vbroadcastss  (%rax),%ymm2
  197,252,91,210,                         //vcvtdq2ps     %ymm2,%ymm2
  197,236,88,201,                         //vaddps        %ymm1,%ymm2,%ymm1
  196,226,125,24,18,                      //vbroadcastss  (%rdx),%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,228,87,219,                         //vxorps        %ymm3,%ymm3,%ymm3
  197,220,87,228,                         //vxorps        %ymm4,%ymm4,%ymm4
  197,212,87,237,                         //vxorps        %ymm5,%ymm5,%ymm5
  197,204,87,246,                         //vxorps        %ymm6,%ymm6,%ymm6
  197,196,87,255,                         //vxorps        %ymm7,%ymm7,%ymm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_constant_color_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,226,125,24,0,                       //vbroadcastss  (%rax),%ymm0
  196,226,125,24,72,4,                    //vbroadcastss  0x4(%rax),%ymm1
  196,226,125,24,80,8,                    //vbroadcastss  0x8(%rax),%ymm2
  196,226,125,24,88,12,                   //vbroadcastss  0xc(%rax),%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clear_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,87,192,                         //vxorps        %ymm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  197,236,87,210,                         //vxorps        %ymm2,%ymm2,%ymm2
  197,228,87,219,                         //vxorps        %ymm3,%ymm3,%ymm3
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_plus__hsw[] = {
  197,252,88,196,                         //vaddps        %ymm4,%ymm0,%ymm0
  197,244,88,205,                         //vaddps        %ymm5,%ymm1,%ymm1
  197,236,88,214,                         //vaddps        %ymm6,%ymm2,%ymm2
  197,228,88,223,                         //vaddps        %ymm7,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_srcover_hsw[] = {
  196,98,125,24,2,                        //vbroadcastss  (%rdx),%ymm8
  197,60,92,195,                          //vsubps        %ymm3,%ymm8,%ymm8
  196,194,93,184,192,                     //vfmadd231ps   %ymm8,%ymm4,%ymm0
  196,194,85,184,200,                     //vfmadd231ps   %ymm8,%ymm5,%ymm1
  196,194,77,184,208,                     //vfmadd231ps   %ymm8,%ymm6,%ymm2
  196,194,69,184,216,                     //vfmadd231ps   %ymm8,%ymm7,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_dstover_hsw[] = {
  196,98,125,24,2,                        //vbroadcastss  (%rdx),%ymm8
  197,60,92,199,                          //vsubps        %ymm7,%ymm8,%ymm8
  196,226,61,168,196,                     //vfmadd213ps   %ymm4,%ymm8,%ymm0
  196,226,61,168,205,                     //vfmadd213ps   %ymm5,%ymm8,%ymm1
  196,226,61,168,214,                     //vfmadd213ps   %ymm6,%ymm8,%ymm2
  196,226,61,168,223,                     //vfmadd213ps   %ymm7,%ymm8,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_0_hsw[] = {
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  196,193,124,95,192,                     //vmaxps        %ymm8,%ymm0,%ymm0
  196,193,116,95,200,                     //vmaxps        %ymm8,%ymm1,%ymm1
  196,193,108,95,208,                     //vmaxps        %ymm8,%ymm2,%ymm2
  196,193,100,95,216,                     //vmaxps        %ymm8,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_1_hsw[] = {
  196,98,125,24,2,                        //vbroadcastss  (%rdx),%ymm8
  196,193,124,93,192,                     //vminps        %ymm8,%ymm0,%ymm0
  196,193,116,93,200,                     //vminps        %ymm8,%ymm1,%ymm1
  196,193,108,93,208,                     //vminps        %ymm8,%ymm2,%ymm2
  196,193,100,93,216,                     //vminps        %ymm8,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_a_hsw[] = {
  196,98,125,24,2,                        //vbroadcastss  (%rdx),%ymm8
  196,193,100,93,216,                     //vminps        %ymm8,%ymm3,%ymm3
  197,252,93,195,                         //vminps        %ymm3,%ymm0,%ymm0
  197,244,93,203,                         //vminps        %ymm3,%ymm1,%ymm1
  197,236,93,211,                         //vminps        %ymm3,%ymm2,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_set_rgb_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,226,125,24,0,                       //vbroadcastss  (%rax),%ymm0
  196,226,125,24,72,4,                    //vbroadcastss  0x4(%rax),%ymm1
  196,226,125,24,80,8,                    //vbroadcastss  0x8(%rax),%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_swap_rb_hsw[] = {
  197,124,40,192,                         //vmovaps       %ymm0,%ymm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,40,194,                         //vmovaps       %ymm2,%ymm0
  197,124,41,194,                         //vmovaps       %ymm8,%ymm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_swap_hsw[] = {
  197,124,40,195,                         //vmovaps       %ymm3,%ymm8
  197,124,40,202,                         //vmovaps       %ymm2,%ymm9
  197,124,40,209,                         //vmovaps       %ymm1,%ymm10
  197,124,40,216,                         //vmovaps       %ymm0,%ymm11
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,40,196,                         //vmovaps       %ymm4,%ymm0
  197,252,40,205,                         //vmovaps       %ymm5,%ymm1
  197,252,40,214,                         //vmovaps       %ymm6,%ymm2
  197,252,40,223,                         //vmovaps       %ymm7,%ymm3
  197,124,41,220,                         //vmovaps       %ymm11,%ymm4
  197,124,41,213,                         //vmovaps       %ymm10,%ymm5
  197,124,41,206,                         //vmovaps       %ymm9,%ymm6
  197,124,41,199,                         //vmovaps       %ymm8,%ymm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_move_src_dst_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,40,224,                         //vmovaps       %ymm0,%ymm4
  197,252,40,233,                         //vmovaps       %ymm1,%ymm5
  197,252,40,242,                         //vmovaps       %ymm2,%ymm6
  197,252,40,251,                         //vmovaps       %ymm3,%ymm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_move_dst_src_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,40,196,                         //vmovaps       %ymm4,%ymm0
  197,252,40,205,                         //vmovaps       %ymm5,%ymm1
  197,252,40,214,                         //vmovaps       %ymm6,%ymm2
  197,252,40,223,                         //vmovaps       %ymm7,%ymm3
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_premul_hsw[] = {
  197,252,89,195,                         //vmulps        %ymm3,%ymm0,%ymm0
  197,244,89,203,                         //vmulps        %ymm3,%ymm1,%ymm1
  197,236,89,211,                         //vmulps        %ymm3,%ymm2,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_unpremul_hsw[] = {
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  196,65,100,194,200,0,                   //vcmpeqps      %ymm8,%ymm3,%ymm9
  196,98,125,24,18,                       //vbroadcastss  (%rdx),%ymm10
  197,44,94,211,                          //vdivps        %ymm3,%ymm10,%ymm10
  196,67,45,74,192,144,                   //vblendvps     %ymm9,%ymm8,%ymm10,%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_from_srgb_hsw[] = {
  196,98,125,24,66,64,                    //vbroadcastss  0x40(%rdx),%ymm8
  197,60,89,200,                          //vmulps        %ymm0,%ymm8,%ymm9
  197,124,89,208,                         //vmulps        %ymm0,%ymm0,%ymm10
  196,98,125,24,90,60,                    //vbroadcastss  0x3c(%rdx),%ymm11
  196,98,125,24,98,56,                    //vbroadcastss  0x38(%rdx),%ymm12
  196,65,124,40,235,                      //vmovaps       %ymm11,%ymm13
  196,66,125,168,236,                     //vfmadd213ps   %ymm12,%ymm0,%ymm13
  196,98,125,24,114,52,                   //vbroadcastss  0x34(%rdx),%ymm14
  196,66,45,168,238,                      //vfmadd213ps   %ymm14,%ymm10,%ymm13
  196,98,125,24,82,68,                    //vbroadcastss  0x44(%rdx),%ymm10
  196,193,124,194,194,1,                  //vcmpltps      %ymm10,%ymm0,%ymm0
  196,195,21,74,193,0,                    //vblendvps     %ymm0,%ymm9,%ymm13,%ymm0
  197,60,89,201,                          //vmulps        %ymm1,%ymm8,%ymm9
  197,116,89,233,                         //vmulps        %ymm1,%ymm1,%ymm13
  196,65,124,40,251,                      //vmovaps       %ymm11,%ymm15
  196,66,117,168,252,                     //vfmadd213ps   %ymm12,%ymm1,%ymm15
  196,66,21,168,254,                      //vfmadd213ps   %ymm14,%ymm13,%ymm15
  196,193,116,194,202,1,                  //vcmpltps      %ymm10,%ymm1,%ymm1
  196,195,5,74,201,16,                    //vblendvps     %ymm1,%ymm9,%ymm15,%ymm1
  197,60,89,194,                          //vmulps        %ymm2,%ymm8,%ymm8
  197,108,89,202,                         //vmulps        %ymm2,%ymm2,%ymm9
  196,66,109,168,220,                     //vfmadd213ps   %ymm12,%ymm2,%ymm11
  196,66,53,168,222,                      //vfmadd213ps   %ymm14,%ymm9,%ymm11
  196,193,108,194,210,1,                  //vcmpltps      %ymm10,%ymm2,%ymm2
  196,195,37,74,208,32,                   //vblendvps     %ymm2,%ymm8,%ymm11,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_to_srgb_hsw[] = {
  197,124,82,192,                         //vrsqrtps      %ymm0,%ymm8
  196,65,124,83,200,                      //vrcpps        %ymm8,%ymm9
  196,65,124,82,208,                      //vrsqrtps      %ymm8,%ymm10
  196,98,125,24,66,72,                    //vbroadcastss  0x48(%rdx),%ymm8
  197,60,89,216,                          //vmulps        %ymm0,%ymm8,%ymm11
  196,98,125,24,34,                       //vbroadcastss  (%rdx),%ymm12
  196,98,125,24,106,76,                   //vbroadcastss  0x4c(%rdx),%ymm13
  196,98,125,24,114,80,                   //vbroadcastss  0x50(%rdx),%ymm14
  196,98,125,24,122,84,                   //vbroadcastss  0x54(%rdx),%ymm15
  196,66,13,168,207,                      //vfmadd213ps   %ymm15,%ymm14,%ymm9
  196,66,21,184,202,                      //vfmadd231ps   %ymm10,%ymm13,%ymm9
  196,65,28,93,201,                       //vminps        %ymm9,%ymm12,%ymm9
  196,98,125,24,82,88,                    //vbroadcastss  0x58(%rdx),%ymm10
  196,193,124,194,194,1,                  //vcmpltps      %ymm10,%ymm0,%ymm0
  196,195,53,74,195,0,                    //vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
  197,124,82,201,                         //vrsqrtps      %ymm1,%ymm9
  196,65,124,83,217,                      //vrcpps        %ymm9,%ymm11
  196,65,124,82,201,                      //vrsqrtps      %ymm9,%ymm9
  196,66,13,168,223,                      //vfmadd213ps   %ymm15,%ymm14,%ymm11
  196,66,21,184,217,                      //vfmadd231ps   %ymm9,%ymm13,%ymm11
  197,60,89,201,                          //vmulps        %ymm1,%ymm8,%ymm9
  196,65,28,93,219,                       //vminps        %ymm11,%ymm12,%ymm11
  196,193,116,194,202,1,                  //vcmpltps      %ymm10,%ymm1,%ymm1
  196,195,37,74,201,16,                   //vblendvps     %ymm1,%ymm9,%ymm11,%ymm1
  197,124,82,202,                         //vrsqrtps      %ymm2,%ymm9
  196,65,124,83,217,                      //vrcpps        %ymm9,%ymm11
  196,66,13,168,223,                      //vfmadd213ps   %ymm15,%ymm14,%ymm11
  196,65,124,82,201,                      //vrsqrtps      %ymm9,%ymm9
  196,66,21,184,217,                      //vfmadd231ps   %ymm9,%ymm13,%ymm11
  196,65,28,93,203,                       //vminps        %ymm11,%ymm12,%ymm9
  197,60,89,194,                          //vmulps        %ymm2,%ymm8,%ymm8
  196,193,108,194,210,1,                  //vcmpltps      %ymm10,%ymm2,%ymm2
  196,195,53,74,208,32,                   //vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_scale_1_float_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  197,188,89,219,                         //vmulps        %ymm3,%ymm8,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_scale_u8_hsw[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,1,248,                               //add           %rdi,%rax
  77,133,192,                             //test          %r8,%r8
  117,48,                                 //jne           41a <_sk_scale_u8_hsw+0x40>
  197,123,16,0,                           //vmovsd        (%rax),%xmm8
  196,66,125,49,192,                      //vpmovzxbd     %xmm8,%ymm8
  196,65,124,91,192,                      //vcvtdq2ps     %ymm8,%ymm8
  196,98,125,24,74,12,                    //vbroadcastss  0xc(%rdx),%ymm9
  196,65,60,89,193,                       //vmulps        %ymm9,%ymm8,%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  197,188,89,219,                         //vmulps        %ymm3,%ymm8,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  49,201,                                 //xor           %ecx,%ecx
  77,137,194,                             //mov           %r8,%r10
  69,49,201,                              //xor           %r9d,%r9d
  68,15,182,24,                           //movzbl        (%rax),%r11d
  72,255,192,                             //inc           %rax
  73,211,227,                             //shl           %cl,%r11
  77,9,217,                               //or            %r11,%r9
  72,131,193,8,                           //add           $0x8,%rcx
  73,255,202,                             //dec           %r10
  117,234,                                //jne           422 <_sk_scale_u8_hsw+0x48>
  196,65,249,110,193,                     //vmovq         %r9,%xmm8
  235,175,                                //jmp           3ee <_sk_scale_u8_hsw+0x14>
};

CODE const uint8_t sk_lerp_1_float_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  197,252,92,196,                         //vsubps        %ymm4,%ymm0,%ymm0
  196,226,61,168,196,                     //vfmadd213ps   %ymm4,%ymm8,%ymm0
  197,244,92,205,                         //vsubps        %ymm5,%ymm1,%ymm1
  196,226,61,168,205,                     //vfmadd213ps   %ymm5,%ymm8,%ymm1
  197,236,92,214,                         //vsubps        %ymm6,%ymm2,%ymm2
  196,226,61,168,214,                     //vfmadd213ps   %ymm6,%ymm8,%ymm2
  197,228,92,223,                         //vsubps        %ymm7,%ymm3,%ymm3
  196,226,61,168,223,                     //vfmadd213ps   %ymm7,%ymm8,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_lerp_u8_hsw[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,1,248,                               //add           %rdi,%rax
  77,133,192,                             //test          %r8,%r8
  117,68,                                 //jne           4c2 <_sk_lerp_u8_hsw+0x54>
  197,123,16,0,                           //vmovsd        (%rax),%xmm8
  196,66,125,49,192,                      //vpmovzxbd     %xmm8,%ymm8
  196,65,124,91,192,                      //vcvtdq2ps     %ymm8,%ymm8
  196,98,125,24,74,12,                    //vbroadcastss  0xc(%rdx),%ymm9
  196,65,60,89,193,                       //vmulps        %ymm9,%ymm8,%ymm8
  197,252,92,196,                         //vsubps        %ymm4,%ymm0,%ymm0
  196,226,61,168,196,                     //vfmadd213ps   %ymm4,%ymm8,%ymm0
  197,244,92,205,                         //vsubps        %ymm5,%ymm1,%ymm1
  196,226,61,168,205,                     //vfmadd213ps   %ymm5,%ymm8,%ymm1
  197,236,92,214,                         //vsubps        %ymm6,%ymm2,%ymm2
  196,226,61,168,214,                     //vfmadd213ps   %ymm6,%ymm8,%ymm2
  197,228,92,223,                         //vsubps        %ymm7,%ymm3,%ymm3
  196,226,61,168,223,                     //vfmadd213ps   %ymm7,%ymm8,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  49,201,                                 //xor           %ecx,%ecx
  77,137,194,                             //mov           %r8,%r10
  69,49,201,                              //xor           %r9d,%r9d
  68,15,182,24,                           //movzbl        (%rax),%r11d
  72,255,192,                             //inc           %rax
  73,211,227,                             //shl           %cl,%r11
  77,9,217,                               //or            %r11,%r9
  72,131,193,8,                           //add           $0x8,%rcx
  73,255,202,                             //dec           %r10
  117,234,                                //jne           4ca <_sk_lerp_u8_hsw+0x5c>
  196,65,249,110,193,                     //vmovq         %r9,%xmm8
  235,155,                                //jmp           482 <_sk_lerp_u8_hsw+0x14>
};

CODE const uint8_t sk_lerp_565_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,16,                              //mov           (%rax),%r10
  72,133,201,                             //test          %rcx,%rcx
  117,123,                                //jne           56c <_sk_lerp_565_hsw+0x85>
  196,193,122,111,28,122,                 //vmovdqu       (%r10,%rdi,2),%xmm3
  196,226,125,51,219,                     //vpmovzxwd     %xmm3,%ymm3
  196,98,125,88,66,104,                   //vpbroadcastd  0x68(%rdx),%ymm8
  197,61,219,195,                         //vpand         %ymm3,%ymm8,%ymm8
  196,65,124,91,192,                      //vcvtdq2ps     %ymm8,%ymm8
  196,98,125,24,74,116,                   //vbroadcastss  0x74(%rdx),%ymm9
  196,65,52,89,192,                       //vmulps        %ymm8,%ymm9,%ymm8
  196,98,125,88,74,108,                   //vpbroadcastd  0x6c(%rdx),%ymm9
  197,53,219,203,                         //vpand         %ymm3,%ymm9,%ymm9
  196,65,124,91,201,                      //vcvtdq2ps     %ymm9,%ymm9
  196,98,125,24,82,120,                   //vbroadcastss  0x78(%rdx),%ymm10
  196,65,44,89,201,                       //vmulps        %ymm9,%ymm10,%ymm9
  196,98,125,88,82,112,                   //vpbroadcastd  0x70(%rdx),%ymm10
  197,173,219,219,                        //vpand         %ymm3,%ymm10,%ymm3
  197,252,91,219,                         //vcvtdq2ps     %ymm3,%ymm3
  196,98,125,24,82,124,                   //vbroadcastss  0x7c(%rdx),%ymm10
  197,172,89,219,                         //vmulps        %ymm3,%ymm10,%ymm3
  197,252,92,196,                         //vsubps        %ymm4,%ymm0,%ymm0
  196,226,61,168,196,                     //vfmadd213ps   %ymm4,%ymm8,%ymm0
  197,244,92,205,                         //vsubps        %ymm5,%ymm1,%ymm1
  196,226,53,168,205,                     //vfmadd213ps   %ymm5,%ymm9,%ymm1
  197,236,92,214,                         //vsubps        %ymm6,%ymm2,%ymm2
  196,226,101,168,214,                    //vfmadd213ps   %ymm6,%ymm3,%ymm2
  196,226,125,24,26,                      //vbroadcastss  (%rdx),%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  65,137,200,                             //mov           %ecx,%r8d
  65,128,224,7,                           //and           $0x7,%r8b
  197,225,239,219,                        //vpxor         %xmm3,%xmm3,%xmm3
  65,254,200,                             //dec           %r8b
  69,15,182,192,                          //movzbl        %r8b,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  15,135,111,255,255,255,                 //ja            4f7 <_sk_lerp_565_hsw+0x10>
  76,141,13,73,0,0,0,                     //lea           0x49(%rip),%r9        # 5d8 <_sk_lerp_565_hsw+0xf1>
  75,99,4,129,                            //movslq        (%r9,%r8,4),%rax
  76,1,200,                               //add           %r9,%rax
  255,224,                                //jmpq          *%rax
  197,225,239,219,                        //vpxor         %xmm3,%xmm3,%xmm3
  196,193,97,196,92,122,12,6,             //vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm3,%xmm3
  196,193,97,196,92,122,10,5,             //vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm3,%xmm3
  196,193,97,196,92,122,8,4,              //vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm3,%xmm3
  196,193,97,196,92,122,6,3,              //vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm3,%xmm3
  196,193,97,196,92,122,4,2,              //vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm3,%xmm3
  196,193,97,196,92,122,2,1,              //vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm3,%xmm3
  196,193,97,196,28,122,0,                //vpinsrw       $0x0,(%r10,%rdi,2),%xmm3,%xmm3
  233,31,255,255,255,                     //jmpq          4f7 <_sk_lerp_565_hsw+0x10>
  244,                                    //hlt
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  236,                                    //in            (%dx),%al
  255,                                    //(bad)
  255,                                    //(bad)
  255,228,                                //jmpq          *%rsp
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  220,255,                                //fdivr         %st,%st(7)
  255,                                    //(bad)
  255,212,                                //callq         *%rsp
  255,                                    //(bad)
  255,                                    //(bad)
  255,204,                                //dec           %esp
  255,                                    //(bad)
  255,                                    //(bad)
  255,192,                                //inc           %eax
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_load_tables_hsw[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,141,12,189,0,0,0,0,                  //lea           0x0(,%rdi,4),%r9
  76,3,8,                                 //add           (%rax),%r9
  77,133,192,                             //test          %r8,%r8
  117,106,                                //jne           673 <_sk_load_tables_hsw+0x7f>
  196,193,126,111,25,                     //vmovdqu       (%r9),%ymm3
  196,226,125,88,82,16,                   //vpbroadcastd  0x10(%rdx),%ymm2
  197,237,219,203,                        //vpand         %ymm3,%ymm2,%ymm1
  196,65,61,118,192,                      //vpcmpeqd      %ymm8,%ymm8,%ymm8
  72,139,72,8,                            //mov           0x8(%rax),%rcx
  76,139,72,16,                           //mov           0x10(%rax),%r9
  196,65,53,118,201,                      //vpcmpeqd      %ymm9,%ymm9,%ymm9
  196,226,53,146,4,137,                   //vgatherdps    %ymm9,(%rcx,%ymm1,4),%ymm0
  197,245,114,211,8,                      //vpsrld        $0x8,%ymm3,%ymm1
  197,109,219,201,                        //vpand         %ymm1,%ymm2,%ymm9
  196,65,45,118,210,                      //vpcmpeqd      %ymm10,%ymm10,%ymm10
  196,130,45,146,12,137,                  //vgatherdps    %ymm10,(%r9,%ymm9,4),%ymm1
  72,139,64,24,                           //mov           0x18(%rax),%rax
  197,181,114,211,16,                     //vpsrld        $0x10,%ymm3,%ymm9
  196,65,109,219,201,                     //vpand         %ymm9,%ymm2,%ymm9
  196,162,61,146,20,136,                  //vgatherdps    %ymm8,(%rax,%ymm9,4),%ymm2
  197,229,114,211,24,                     //vpsrld        $0x18,%ymm3,%ymm3
  197,252,91,219,                         //vcvtdq2ps     %ymm3,%ymm3
  196,98,125,24,66,12,                    //vbroadcastss  0xc(%rdx),%ymm8
  196,193,100,89,216,                     //vmulps        %ymm8,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  185,8,0,0,0,                            //mov           $0x8,%ecx
  68,41,193,                              //sub           %r8d,%ecx
  192,225,3,                              //shl           $0x3,%cl
  73,199,194,255,255,255,255,             //mov           $0xffffffffffffffff,%r10
  73,211,234,                             //shr           %cl,%r10
  196,193,249,110,194,                    //vmovq         %r10,%xmm0
  196,226,125,33,192,                     //vpmovsxbd     %xmm0,%ymm0
  196,194,125,140,25,                     //vpmaskmovd    (%r9),%ymm0,%ymm3
  233,114,255,255,255,                    //jmpq          60e <_sk_load_tables_hsw+0x1a>
};

CODE const uint8_t sk_load_a8_hsw[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,1,248,                               //add           %rdi,%rax
  77,133,192,                             //test          %r8,%r8
  117,42,                                 //jne           6d6 <_sk_load_a8_hsw+0x3a>
  197,251,16,0,                           //vmovsd        (%rax),%xmm0
  196,226,125,49,192,                     //vpmovzxbd     %xmm0,%ymm0
  197,252,91,192,                         //vcvtdq2ps     %ymm0,%ymm0
  196,226,125,24,74,12,                   //vbroadcastss  0xc(%rdx),%ymm1
  197,252,89,217,                         //vmulps        %ymm1,%ymm0,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,87,192,                         //vxorps        %ymm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  197,236,87,210,                         //vxorps        %ymm2,%ymm2,%ymm2
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  49,201,                                 //xor           %ecx,%ecx
  77,137,194,                             //mov           %r8,%r10
  69,49,201,                              //xor           %r9d,%r9d
  68,15,182,24,                           //movzbl        (%rax),%r11d
  72,255,192,                             //inc           %rax
  73,211,227,                             //shl           %cl,%r11
  77,9,217,                               //or            %r11,%r9
  72,131,193,8,                           //add           $0x8,%rcx
  73,255,202,                             //dec           %r10
  117,234,                                //jne           6de <_sk_load_a8_hsw+0x42>
  196,193,249,110,193,                    //vmovq         %r9,%xmm0
  235,181,                                //jmp           6b0 <_sk_load_a8_hsw+0x14>
};

CODE const uint8_t sk_store_a8_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,8,                               //mov           (%rax),%r9
  196,98,125,24,66,8,                     //vbroadcastss  0x8(%rdx),%ymm8
  197,60,89,195,                          //vmulps        %ymm3,%ymm8,%ymm8
  196,65,125,91,192,                      //vcvtps2dq     %ymm8,%ymm8
  196,67,125,25,193,1,                    //vextractf128  $0x1,%ymm8,%xmm9
  196,66,57,43,193,                       //vpackusdw     %xmm9,%xmm8,%xmm8
  196,65,57,103,192,                      //vpackuswb     %xmm8,%xmm8,%xmm8
  72,133,201,                             //test          %rcx,%rcx
  117,10,                                 //jne           72e <_sk_store_a8_hsw+0x33>
  196,65,123,17,4,57,                     //vmovsd        %xmm8,(%r9,%rdi,1)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  137,200,                                //mov           %ecx,%eax
  36,7,                                   //and           $0x7,%al
  254,200,                                //dec           %al
  68,15,182,192,                          //movzbl        %al,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  119,236,                                //ja            72a <_sk_store_a8_hsw+0x2f>
  196,66,121,48,192,                      //vpmovzxbw     %xmm8,%xmm8
  76,141,21,66,0,0,0,                     //lea           0x42(%rip),%r10        # 78c <_sk_store_a8_hsw+0x91>
  75,99,4,130,                            //movslq        (%r10,%r8,4),%rax
  76,1,208,                               //add           %r10,%rax
  255,224,                                //jmpq          *%rax
  196,67,121,20,68,57,6,12,               //vpextrb       $0xc,%xmm8,0x6(%r9,%rdi,1)
  196,67,121,20,68,57,5,10,               //vpextrb       $0xa,%xmm8,0x5(%r9,%rdi,1)
  196,67,121,20,68,57,4,8,                //vpextrb       $0x8,%xmm8,0x4(%r9,%rdi,1)
  196,67,121,20,68,57,3,6,                //vpextrb       $0x6,%xmm8,0x3(%r9,%rdi,1)
  196,67,121,20,68,57,2,4,                //vpextrb       $0x4,%xmm8,0x2(%r9,%rdi,1)
  196,67,121,20,68,57,1,2,                //vpextrb       $0x2,%xmm8,0x1(%r9,%rdi,1)
  196,67,121,20,4,57,0,                   //vpextrb       $0x0,%xmm8,(%r9,%rdi,1)
  235,158,                                //jmp           72a <_sk_store_a8_hsw+0x2f>
  247,255,                                //idiv          %edi
  255,                                    //(bad)
  255,                                    //(bad)
  239,                                    //out           %eax,(%dx)
  255,                                    //(bad)
  255,                                    //(bad)
  255,231,                                //jmpq          *%rdi
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  223,255,                                //(bad)
  255,                                    //(bad)
  255,215,                                //callq         *%rdi
  255,                                    //(bad)
  255,                                    //(bad)
  255,207,                                //dec           %edi
  255,                                    //(bad)
  255,                                    //(bad)
  255,199,                                //inc           %edi
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_load_565_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,16,                              //mov           (%rax),%r10
  72,133,201,                             //test          %rcx,%rcx
  117,92,                                 //jne           80e <_sk_load_565_hsw+0x66>
  196,193,122,111,4,122,                  //vmovdqu       (%r10,%rdi,2),%xmm0
  196,226,125,51,208,                     //vpmovzxwd     %xmm0,%ymm2
  196,226,125,88,66,104,                  //vpbroadcastd  0x68(%rdx),%ymm0
  197,253,219,194,                        //vpand         %ymm2,%ymm0,%ymm0
  197,252,91,192,                         //vcvtdq2ps     %ymm0,%ymm0
  196,226,125,24,74,116,                  //vbroadcastss  0x74(%rdx),%ymm1
  197,244,89,192,                         //vmulps        %ymm0,%ymm1,%ymm0
  196,226,125,88,74,108,                  //vpbroadcastd  0x6c(%rdx),%ymm1
  197,245,219,202,                        //vpand         %ymm2,%ymm1,%ymm1
  197,252,91,201,                         //vcvtdq2ps     %ymm1,%ymm1
  196,226,125,24,90,120,                  //vbroadcastss  0x78(%rdx),%ymm3
  197,228,89,201,                         //vmulps        %ymm1,%ymm3,%ymm1
  196,226,125,88,90,112,                  //vpbroadcastd  0x70(%rdx),%ymm3
  197,229,219,210,                        //vpand         %ymm2,%ymm3,%ymm2
  197,252,91,210,                         //vcvtdq2ps     %ymm2,%ymm2
  196,226,125,24,90,124,                  //vbroadcastss  0x7c(%rdx),%ymm3
  197,228,89,210,                         //vmulps        %ymm2,%ymm3,%ymm2
  196,226,125,24,26,                      //vbroadcastss  (%rdx),%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  65,137,200,                             //mov           %ecx,%r8d
  65,128,224,7,                           //and           $0x7,%r8b
  197,249,239,192,                        //vpxor         %xmm0,%xmm0,%xmm0
  65,254,200,                             //dec           %r8b
  69,15,182,192,                          //movzbl        %r8b,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  119,146,                                //ja            7b8 <_sk_load_565_hsw+0x10>
  76,141,13,75,0,0,0,                     //lea           0x4b(%rip),%r9        # 878 <_sk_load_565_hsw+0xd0>
  75,99,4,129,                            //movslq        (%r9,%r8,4),%rax
  76,1,200,                               //add           %r9,%rax
  255,224,                                //jmpq          *%rax
  197,249,239,192,                        //vpxor         %xmm0,%xmm0,%xmm0
  196,193,121,196,68,122,12,6,            //vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,10,5,            //vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,8,4,             //vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,6,3,             //vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,4,2,             //vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,2,1,             //vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,4,122,0,                //vpinsrw       $0x0,(%r10,%rdi,2),%xmm0,%xmm0
  233,66,255,255,255,                     //jmpq          7b8 <_sk_load_565_hsw+0x10>
  102,144,                                //xchg          %ax,%ax
  242,255,                                //repnz         (bad)
  255,                                    //(bad)
  255,                                    //(bad)
  234,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  255,226,                                //jmpq          *%rdx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  218,255,                                //(bad)
  255,                                    //(bad)
  255,210,                                //callq         *%rdx
  255,                                    //(bad)
  255,                                    //(bad)
  255,202,                                //dec           %edx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  190,                                    //.byte         0xbe
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_store_565_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,8,                               //mov           (%rax),%r9
  196,98,125,24,130,128,0,0,0,            //vbroadcastss  0x80(%rdx),%ymm8
  197,60,89,200,                          //vmulps        %ymm0,%ymm8,%ymm9
  196,65,125,91,201,                      //vcvtps2dq     %ymm9,%ymm9
  196,193,53,114,241,11,                  //vpslld        $0xb,%ymm9,%ymm9
  196,98,125,24,146,132,0,0,0,            //vbroadcastss  0x84(%rdx),%ymm10
  197,44,89,209,                          //vmulps        %ymm1,%ymm10,%ymm10
  196,65,125,91,210,                      //vcvtps2dq     %ymm10,%ymm10
  196,193,45,114,242,5,                   //vpslld        $0x5,%ymm10,%ymm10
  196,65,45,235,201,                      //vpor          %ymm9,%ymm10,%ymm9
  197,60,89,194,                          //vmulps        %ymm2,%ymm8,%ymm8
  196,65,125,91,192,                      //vcvtps2dq     %ymm8,%ymm8
  196,65,53,235,192,                      //vpor          %ymm8,%ymm9,%ymm8
  196,67,125,57,193,1,                    //vextracti128  $0x1,%ymm8,%xmm9
  196,66,57,43,193,                       //vpackusdw     %xmm9,%xmm8,%xmm8
  72,133,201,                             //test          %rcx,%rcx
  117,10,                                 //jne           8f6 <_sk_store_565_hsw+0x62>
  196,65,122,127,4,121,                   //vmovdqu       %xmm8,(%r9,%rdi,2)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  137,200,                                //mov           %ecx,%eax
  36,7,                                   //and           $0x7,%al
  254,200,                                //dec           %al
  68,15,182,192,                          //movzbl        %al,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  119,236,                                //ja            8f2 <_sk_store_565_hsw+0x5e>
  76,141,21,71,0,0,0,                     //lea           0x47(%rip),%r10        # 954 <_sk_store_565_hsw+0xc0>
  75,99,4,130,                            //movslq        (%r10,%r8,4),%rax
  76,1,208,                               //add           %r10,%rax
  255,224,                                //jmpq          *%rax
  196,67,121,21,68,121,12,6,              //vpextrw       $0x6,%xmm8,0xc(%r9,%rdi,2)
  196,67,121,21,68,121,10,5,              //vpextrw       $0x5,%xmm8,0xa(%r9,%rdi,2)
  196,67,121,21,68,121,8,4,               //vpextrw       $0x4,%xmm8,0x8(%r9,%rdi,2)
  196,67,121,21,68,121,6,3,               //vpextrw       $0x3,%xmm8,0x6(%r9,%rdi,2)
  196,67,121,21,68,121,4,2,               //vpextrw       $0x2,%xmm8,0x4(%r9,%rdi,2)
  196,67,121,21,68,121,2,1,               //vpextrw       $0x1,%xmm8,0x2(%r9,%rdi,2)
  197,121,126,192,                        //vmovd         %xmm8,%eax
  102,65,137,4,121,                       //mov           %ax,(%r9,%rdi,2)
  235,161,                                //jmp           8f2 <_sk_store_565_hsw+0x5e>
  15,31,0,                                //nopl          (%rax)
  242,255,                                //repnz         (bad)
  255,                                    //(bad)
  255,                                    //(bad)
  234,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  255,226,                                //jmpq          *%rdx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  218,255,                                //(bad)
  255,                                    //(bad)
  255,210,                                //callq         *%rdx
  255,                                    //(bad)
  255,                                    //(bad)
  255,202,                                //dec           %edx
  255,                                    //(bad)
  255,                                    //(bad)
  255,194,                                //inc           %edx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_load_8888_hsw[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,141,12,189,0,0,0,0,                  //lea           0x0(,%rdi,4),%r9
  76,3,8,                                 //add           (%rax),%r9
  77,133,192,                             //test          %r8,%r8
  117,85,                                 //jne           9da <_sk_load_8888_hsw+0x6a>
  196,193,126,111,25,                     //vmovdqu       (%r9),%ymm3
  196,226,125,88,82,16,                   //vpbroadcastd  0x10(%rdx),%ymm2
  197,237,219,195,                        //vpand         %ymm3,%ymm2,%ymm0
  197,252,91,192,                         //vcvtdq2ps     %ymm0,%ymm0
  196,98,125,24,66,12,                    //vbroadcastss  0xc(%rdx),%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,245,114,211,8,                      //vpsrld        $0x8,%ymm3,%ymm1
  197,237,219,201,                        //vpand         %ymm1,%ymm2,%ymm1
  197,252,91,201,                         //vcvtdq2ps     %ymm1,%ymm1
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,181,114,211,16,                     //vpsrld        $0x10,%ymm3,%ymm9
  196,193,109,219,209,                    //vpand         %ymm9,%ymm2,%ymm2
  197,252,91,210,                         //vcvtdq2ps     %ymm2,%ymm2
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  197,229,114,211,24,                     //vpsrld        $0x18,%ymm3,%ymm3
  197,252,91,219,                         //vcvtdq2ps     %ymm3,%ymm3
  196,193,100,89,216,                     //vmulps        %ymm8,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  185,8,0,0,0,                            //mov           $0x8,%ecx
  68,41,193,                              //sub           %r8d,%ecx
  192,225,3,                              //shl           $0x3,%cl
  72,199,192,255,255,255,255,             //mov           $0xffffffffffffffff,%rax
  72,211,232,                             //shr           %cl,%rax
  196,225,249,110,192,                    //vmovq         %rax,%xmm0
  196,226,125,33,192,                     //vpmovsxbd     %xmm0,%ymm0
  196,194,125,140,25,                     //vpmaskmovd    (%r9),%ymm0,%ymm3
  235,138,                                //jmp           98a <_sk_load_8888_hsw+0x1a>
};

CODE const uint8_t sk_store_8888_hsw[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,141,12,189,0,0,0,0,                  //lea           0x0(,%rdi,4),%r9
  76,3,8,                                 //add           (%rax),%r9
  196,98,125,24,66,8,                     //vbroadcastss  0x8(%rdx),%ymm8
  197,60,89,200,                          //vmulps        %ymm0,%ymm8,%ymm9
  196,65,125,91,201,                      //vcvtps2dq     %ymm9,%ymm9
  197,60,89,209,                          //vmulps        %ymm1,%ymm8,%ymm10
  196,65,125,91,210,                      //vcvtps2dq     %ymm10,%ymm10
  196,193,45,114,242,8,                   //vpslld        $0x8,%ymm10,%ymm10
  196,65,45,235,201,                      //vpor          %ymm9,%ymm10,%ymm9
  197,60,89,210,                          //vmulps        %ymm2,%ymm8,%ymm10
  196,65,125,91,210,                      //vcvtps2dq     %ymm10,%ymm10
  196,193,45,114,242,16,                  //vpslld        $0x10,%ymm10,%ymm10
  197,60,89,195,                          //vmulps        %ymm3,%ymm8,%ymm8
  196,65,125,91,192,                      //vcvtps2dq     %ymm8,%ymm8
  196,193,61,114,240,24,                  //vpslld        $0x18,%ymm8,%ymm8
  196,65,45,235,192,                      //vpor          %ymm8,%ymm10,%ymm8
  196,65,53,235,192,                      //vpor          %ymm8,%ymm9,%ymm8
  77,133,192,                             //test          %r8,%r8
  117,12,                                 //jne           a6c <_sk_store_8888_hsw+0x6c>
  196,65,126,127,1,                       //vmovdqu       %ymm8,(%r9)
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  185,8,0,0,0,                            //mov           $0x8,%ecx
  68,41,193,                              //sub           %r8d,%ecx
  192,225,3,                              //shl           $0x3,%cl
  72,199,192,255,255,255,255,             //mov           $0xffffffffffffffff,%rax
  72,211,232,                             //shr           %cl,%rax
  196,97,249,110,200,                     //vmovq         %rax,%xmm9
  196,66,125,33,201,                      //vpmovsxbd     %xmm9,%ymm9
  196,66,53,142,1,                        //vpmaskmovd    %ymm8,%ymm9,(%r9)
  235,211,                                //jmp           a65 <_sk_store_8888_hsw+0x65>
};

CODE const uint8_t sk_load_f16_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,133,201,                             //test          %rcx,%rcx
  117,97,                                 //jne           afd <_sk_load_f16_hsw+0x6b>
  197,249,16,12,248,                      //vmovupd       (%rax,%rdi,8),%xmm1
  197,249,16,84,248,16,                   //vmovupd       0x10(%rax,%rdi,8),%xmm2
  197,249,16,92,248,32,                   //vmovupd       0x20(%rax,%rdi,8),%xmm3
  197,121,16,68,248,48,                   //vmovupd       0x30(%rax,%rdi,8),%xmm8
  197,241,97,194,                         //vpunpcklwd    %xmm2,%xmm1,%xmm0
  197,241,105,202,                        //vpunpckhwd    %xmm2,%xmm1,%xmm1
  196,193,97,97,208,                      //vpunpcklwd    %xmm8,%xmm3,%xmm2
  196,193,97,105,216,                     //vpunpckhwd    %xmm8,%xmm3,%xmm3
  197,121,97,193,                         //vpunpcklwd    %xmm1,%xmm0,%xmm8
  197,121,105,201,                        //vpunpckhwd    %xmm1,%xmm0,%xmm9
  197,233,97,203,                         //vpunpcklwd    %xmm3,%xmm2,%xmm1
  197,233,105,219,                        //vpunpckhwd    %xmm3,%xmm2,%xmm3
  197,185,108,193,                        //vpunpcklqdq   %xmm1,%xmm8,%xmm0
  196,226,125,19,192,                     //vcvtph2ps     %xmm0,%ymm0
  197,185,109,201,                        //vpunpckhqdq   %xmm1,%xmm8,%xmm1
  196,226,125,19,201,                     //vcvtph2ps     %xmm1,%ymm1
  197,177,108,211,                        //vpunpcklqdq   %xmm3,%xmm9,%xmm2
  196,226,125,19,210,                     //vcvtph2ps     %xmm2,%ymm2
  197,177,109,219,                        //vpunpckhqdq   %xmm3,%xmm9,%xmm3
  196,226,125,19,219,                     //vcvtph2ps     %xmm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  197,251,16,12,248,                      //vmovsd        (%rax,%rdi,8),%xmm1
  196,65,57,87,192,                       //vxorpd        %xmm8,%xmm8,%xmm8
  72,131,249,1,                           //cmp           $0x1,%rcx
  117,6,                                  //jne           b13 <_sk_load_f16_hsw+0x81>
  197,250,126,201,                        //vmovq         %xmm1,%xmm1
  235,30,                                 //jmp           b31 <_sk_load_f16_hsw+0x9f>
  197,241,22,76,248,8,                    //vmovhpd       0x8(%rax,%rdi,8),%xmm1,%xmm1
  72,131,249,3,                           //cmp           $0x3,%rcx
  114,18,                                 //jb            b31 <_sk_load_f16_hsw+0x9f>
  197,251,16,84,248,16,                   //vmovsd        0x10(%rax,%rdi,8),%xmm2
  72,131,249,3,                           //cmp           $0x3,%rcx
  117,19,                                 //jne           b3e <_sk_load_f16_hsw+0xac>
  197,250,126,210,                        //vmovq         %xmm2,%xmm2
  235,46,                                 //jmp           b5f <_sk_load_f16_hsw+0xcd>
  197,225,87,219,                         //vxorpd        %xmm3,%xmm3,%xmm3
  197,233,87,210,                         //vxorpd        %xmm2,%xmm2,%xmm2
  233,117,255,255,255,                    //jmpq          ab3 <_sk_load_f16_hsw+0x21>
  197,233,22,84,248,24,                   //vmovhpd       0x18(%rax,%rdi,8),%xmm2,%xmm2
  72,131,249,5,                           //cmp           $0x5,%rcx
  114,21,                                 //jb            b5f <_sk_load_f16_hsw+0xcd>
  197,251,16,92,248,32,                   //vmovsd        0x20(%rax,%rdi,8),%xmm3
  72,131,249,5,                           //cmp           $0x5,%rcx
  117,18,                                 //jne           b68 <_sk_load_f16_hsw+0xd6>
  197,250,126,219,                        //vmovq         %xmm3,%xmm3
  233,84,255,255,255,                     //jmpq          ab3 <_sk_load_f16_hsw+0x21>
  197,225,87,219,                         //vxorpd        %xmm3,%xmm3,%xmm3
  233,75,255,255,255,                     //jmpq          ab3 <_sk_load_f16_hsw+0x21>
  197,225,22,92,248,40,                   //vmovhpd       0x28(%rax,%rdi,8),%xmm3,%xmm3
  72,131,249,7,                           //cmp           $0x7,%rcx
  15,130,59,255,255,255,                  //jb            ab3 <_sk_load_f16_hsw+0x21>
  197,123,16,68,248,48,                   //vmovsd        0x30(%rax,%rdi,8),%xmm8
  233,48,255,255,255,                     //jmpq          ab3 <_sk_load_f16_hsw+0x21>
};

CODE const uint8_t sk_store_f16_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  196,195,125,29,192,4,                   //vcvtps2ph     $0x4,%ymm0,%xmm8
  196,195,125,29,201,4,                   //vcvtps2ph     $0x4,%ymm1,%xmm9
  196,195,125,29,210,4,                   //vcvtps2ph     $0x4,%ymm2,%xmm10
  196,195,125,29,219,4,                   //vcvtps2ph     $0x4,%ymm3,%xmm11
  196,65,57,97,225,                       //vpunpcklwd    %xmm9,%xmm8,%xmm12
  196,65,57,105,193,                      //vpunpckhwd    %xmm9,%xmm8,%xmm8
  196,65,41,97,203,                       //vpunpcklwd    %xmm11,%xmm10,%xmm9
  196,65,41,105,235,                      //vpunpckhwd    %xmm11,%xmm10,%xmm13
  196,65,25,98,217,                       //vpunpckldq    %xmm9,%xmm12,%xmm11
  196,65,25,106,209,                      //vpunpckhdq    %xmm9,%xmm12,%xmm10
  196,65,57,98,205,                       //vpunpckldq    %xmm13,%xmm8,%xmm9
  196,65,57,106,197,                      //vpunpckhdq    %xmm13,%xmm8,%xmm8
  72,133,201,                             //test          %rcx,%rcx
  117,27,                                 //jne           be8 <_sk_store_f16_hsw+0x65>
  197,120,17,28,248,                      //vmovups       %xmm11,(%rax,%rdi,8)
  197,120,17,84,248,16,                   //vmovups       %xmm10,0x10(%rax,%rdi,8)
  197,120,17,76,248,32,                   //vmovups       %xmm9,0x20(%rax,%rdi,8)
  197,122,127,68,248,48,                  //vmovdqu       %xmm8,0x30(%rax,%rdi,8)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  197,121,214,28,248,                     //vmovq         %xmm11,(%rax,%rdi,8)
  72,131,249,1,                           //cmp           $0x1,%rcx
  116,241,                                //je            be4 <_sk_store_f16_hsw+0x61>
  197,121,23,92,248,8,                    //vmovhpd       %xmm11,0x8(%rax,%rdi,8)
  72,131,249,3,                           //cmp           $0x3,%rcx
  114,229,                                //jb            be4 <_sk_store_f16_hsw+0x61>
  197,121,214,84,248,16,                  //vmovq         %xmm10,0x10(%rax,%rdi,8)
  116,221,                                //je            be4 <_sk_store_f16_hsw+0x61>
  197,121,23,84,248,24,                   //vmovhpd       %xmm10,0x18(%rax,%rdi,8)
  72,131,249,5,                           //cmp           $0x5,%rcx
  114,209,                                //jb            be4 <_sk_store_f16_hsw+0x61>
  197,121,214,76,248,32,                  //vmovq         %xmm9,0x20(%rax,%rdi,8)
  116,201,                                //je            be4 <_sk_store_f16_hsw+0x61>
  197,121,23,76,248,40,                   //vmovhpd       %xmm9,0x28(%rax,%rdi,8)
  72,131,249,7,                           //cmp           $0x7,%rcx
  114,189,                                //jb            be4 <_sk_store_f16_hsw+0x61>
  197,121,214,68,248,48,                  //vmovq         %xmm8,0x30(%rax,%rdi,8)
  235,181,                                //jmp           be4 <_sk_store_f16_hsw+0x61>
};

CODE const uint8_t sk_store_f32_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,0,                               //mov           (%rax),%r8
  72,141,4,189,0,0,0,0,                   //lea           0x0(,%rdi,4),%rax
  197,124,20,193,                         //vunpcklps     %ymm1,%ymm0,%ymm8
  197,124,21,217,                         //vunpckhps     %ymm1,%ymm0,%ymm11
  197,108,20,203,                         //vunpcklps     %ymm3,%ymm2,%ymm9
  197,108,21,227,                         //vunpckhps     %ymm3,%ymm2,%ymm12
  196,65,61,20,209,                       //vunpcklpd     %ymm9,%ymm8,%ymm10
  196,65,61,21,201,                       //vunpckhpd     %ymm9,%ymm8,%ymm9
  196,65,37,20,196,                       //vunpcklpd     %ymm12,%ymm11,%ymm8
  196,65,37,21,220,                       //vunpckhpd     %ymm12,%ymm11,%ymm11
  72,133,201,                             //test          %rcx,%rcx
  117,55,                                 //jne           c9c <_sk_store_f32_hsw+0x6d>
  196,67,45,24,225,1,                     //vinsertf128   $0x1,%xmm9,%ymm10,%ymm12
  196,67,61,24,235,1,                     //vinsertf128   $0x1,%xmm11,%ymm8,%ymm13
  196,67,45,6,201,49,                     //vperm2f128    $0x31,%ymm9,%ymm10,%ymm9
  196,67,61,6,195,49,                     //vperm2f128    $0x31,%ymm11,%ymm8,%ymm8
  196,65,125,17,36,128,                   //vmovupd       %ymm12,(%r8,%rax,4)
  196,65,125,17,108,128,32,               //vmovupd       %ymm13,0x20(%r8,%rax,4)
  196,65,125,17,76,128,64,                //vmovupd       %ymm9,0x40(%r8,%rax,4)
  196,65,125,17,68,128,96,                //vmovupd       %ymm8,0x60(%r8,%rax,4)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  196,65,121,17,20,128,                   //vmovupd       %xmm10,(%r8,%rax,4)
  72,131,249,1,                           //cmp           $0x1,%rcx
  116,240,                                //je            c98 <_sk_store_f32_hsw+0x69>
  196,65,121,17,76,128,16,                //vmovupd       %xmm9,0x10(%r8,%rax,4)
  72,131,249,3,                           //cmp           $0x3,%rcx
  114,227,                                //jb            c98 <_sk_store_f32_hsw+0x69>
  196,65,121,17,68,128,32,                //vmovupd       %xmm8,0x20(%r8,%rax,4)
  116,218,                                //je            c98 <_sk_store_f32_hsw+0x69>
  196,65,121,17,92,128,48,                //vmovupd       %xmm11,0x30(%r8,%rax,4)
  72,131,249,5,                           //cmp           $0x5,%rcx
  114,205,                                //jb            c98 <_sk_store_f32_hsw+0x69>
  196,67,125,25,84,128,64,1,              //vextractf128  $0x1,%ymm10,0x40(%r8,%rax,4)
  116,195,                                //je            c98 <_sk_store_f32_hsw+0x69>
  196,67,125,25,76,128,80,1,              //vextractf128  $0x1,%ymm9,0x50(%r8,%rax,4)
  72,131,249,7,                           //cmp           $0x7,%rcx
  114,181,                                //jb            c98 <_sk_store_f32_hsw+0x69>
  196,67,125,25,68,128,96,1,              //vextractf128  $0x1,%ymm8,0x60(%r8,%rax,4)
  235,171,                                //jmp           c98 <_sk_store_f32_hsw+0x69>
};

CODE const uint8_t sk_clamp_x_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  197,188,95,192,                         //vmaxps        %ymm0,%ymm8,%ymm0
  196,98,125,88,0,                        //vpbroadcastd  (%rax),%ymm8
  196,65,53,118,201,                      //vpcmpeqd      %ymm9,%ymm9,%ymm9
  196,65,61,254,193,                      //vpaddd        %ymm9,%ymm8,%ymm8
  196,193,124,93,192,                     //vminps        %ymm8,%ymm0,%ymm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_y_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  197,188,95,201,                         //vmaxps        %ymm1,%ymm8,%ymm1
  196,98,125,88,0,                        //vpbroadcastd  (%rax),%ymm8
  196,65,53,118,201,                      //vpcmpeqd      %ymm9,%ymm9,%ymm9
  196,65,61,254,193,                      //vpaddd        %ymm9,%ymm8,%ymm8
  196,193,116,93,200,                     //vminps        %ymm8,%ymm1,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_repeat_x_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,65,124,94,200,                      //vdivps        %ymm8,%ymm0,%ymm9
  196,67,125,8,201,1,                     //vroundps      $0x1,%ymm9,%ymm9
  196,98,61,172,200,                      //vfnmadd213ps  %ymm0,%ymm8,%ymm9
  197,253,118,192,                        //vpcmpeqd      %ymm0,%ymm0,%ymm0
  197,189,254,192,                        //vpaddd        %ymm0,%ymm8,%ymm0
  197,180,93,192,                         //vminps        %ymm0,%ymm9,%ymm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_repeat_y_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,65,116,94,200,                      //vdivps        %ymm8,%ymm1,%ymm9
  196,67,125,8,201,1,                     //vroundps      $0x1,%ymm9,%ymm9
  196,98,61,172,201,                      //vfnmadd213ps  %ymm1,%ymm8,%ymm9
  197,245,118,201,                        //vpcmpeqd      %ymm1,%ymm1,%ymm1
  197,189,254,201,                        //vpaddd        %ymm1,%ymm8,%ymm1
  197,180,93,201,                         //vminps        %ymm1,%ymm9,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_mirror_x_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,122,16,0,                           //vmovss        (%rax),%xmm8
  196,66,125,24,200,                      //vbroadcastss  %xmm8,%ymm9
  196,65,124,92,209,                      //vsubps        %ymm9,%ymm0,%ymm10
  196,193,58,88,192,                      //vaddss        %xmm8,%xmm8,%xmm0
  196,226,125,24,192,                     //vbroadcastss  %xmm0,%ymm0
  197,44,94,192,                          //vdivps        %ymm0,%ymm10,%ymm8
  196,67,125,8,192,1,                     //vroundps      $0x1,%ymm8,%ymm8
  196,66,125,172,194,                     //vfnmadd213ps  %ymm10,%ymm0,%ymm8
  196,193,60,92,193,                      //vsubps        %ymm9,%ymm8,%ymm0
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  197,60,92,192,                          //vsubps        %ymm0,%ymm8,%ymm8
  197,188,84,192,                         //vandps        %ymm0,%ymm8,%ymm0
  196,65,61,118,192,                      //vpcmpeqd      %ymm8,%ymm8,%ymm8
  196,65,53,254,192,                      //vpaddd        %ymm8,%ymm9,%ymm8
  196,193,124,93,192,                     //vminps        %ymm8,%ymm0,%ymm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_mirror_y_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,122,16,0,                           //vmovss        (%rax),%xmm8
  196,66,125,24,200,                      //vbroadcastss  %xmm8,%ymm9
  196,65,116,92,209,                      //vsubps        %ymm9,%ymm1,%ymm10
  196,193,58,88,200,                      //vaddss        %xmm8,%xmm8,%xmm1
  196,226,125,24,201,                     //vbroadcastss  %xmm1,%ymm1
  197,44,94,193,                          //vdivps        %ymm1,%ymm10,%ymm8
  196,67,125,8,192,1,                     //vroundps      $0x1,%ymm8,%ymm8
  196,66,117,172,194,                     //vfnmadd213ps  %ymm10,%ymm1,%ymm8
  196,193,60,92,201,                      //vsubps        %ymm9,%ymm8,%ymm1
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  197,60,92,193,                          //vsubps        %ymm1,%ymm8,%ymm8
  197,188,84,201,                         //vandps        %ymm1,%ymm8,%ymm1
  196,65,61,118,192,                      //vpcmpeqd      %ymm8,%ymm8,%ymm8
  196,65,53,254,192,                      //vpaddd        %ymm8,%ymm9,%ymm8
  196,193,116,93,200,                     //vminps        %ymm8,%ymm1,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_2x3_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,8,                        //vbroadcastss  (%rax),%ymm9
  196,98,125,24,80,8,                     //vbroadcastss  0x8(%rax),%ymm10
  196,98,125,24,64,16,                    //vbroadcastss  0x10(%rax),%ymm8
  196,66,117,184,194,                     //vfmadd231ps   %ymm10,%ymm1,%ymm8
  196,66,125,184,193,                     //vfmadd231ps   %ymm9,%ymm0,%ymm8
  196,98,125,24,80,4,                     //vbroadcastss  0x4(%rax),%ymm10
  196,98,125,24,88,12,                    //vbroadcastss  0xc(%rax),%ymm11
  196,98,125,24,72,20,                    //vbroadcastss  0x14(%rax),%ymm9
  196,66,117,184,203,                     //vfmadd231ps   %ymm11,%ymm1,%ymm9
  196,66,125,184,202,                     //vfmadd231ps   %ymm10,%ymm0,%ymm9
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,124,41,192,                         //vmovaps       %ymm8,%ymm0
  197,124,41,201,                         //vmovaps       %ymm9,%ymm1
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_3x4_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,8,                        //vbroadcastss  (%rax),%ymm9
  196,98,125,24,80,12,                    //vbroadcastss  0xc(%rax),%ymm10
  196,98,125,24,88,24,                    //vbroadcastss  0x18(%rax),%ymm11
  196,98,125,24,64,36,                    //vbroadcastss  0x24(%rax),%ymm8
  196,66,109,184,195,                     //vfmadd231ps   %ymm11,%ymm2,%ymm8
  196,66,117,184,194,                     //vfmadd231ps   %ymm10,%ymm1,%ymm8
  196,66,125,184,193,                     //vfmadd231ps   %ymm9,%ymm0,%ymm8
  196,98,125,24,80,4,                     //vbroadcastss  0x4(%rax),%ymm10
  196,98,125,24,88,16,                    //vbroadcastss  0x10(%rax),%ymm11
  196,98,125,24,96,28,                    //vbroadcastss  0x1c(%rax),%ymm12
  196,98,125,24,72,40,                    //vbroadcastss  0x28(%rax),%ymm9
  196,66,109,184,204,                     //vfmadd231ps   %ymm12,%ymm2,%ymm9
  196,66,117,184,203,                     //vfmadd231ps   %ymm11,%ymm1,%ymm9
  196,66,125,184,202,                     //vfmadd231ps   %ymm10,%ymm0,%ymm9
  196,98,125,24,88,8,                     //vbroadcastss  0x8(%rax),%ymm11
  196,98,125,24,96,20,                    //vbroadcastss  0x14(%rax),%ymm12
  196,98,125,24,104,32,                   //vbroadcastss  0x20(%rax),%ymm13
  196,98,125,24,80,44,                    //vbroadcastss  0x2c(%rax),%ymm10
  196,66,109,184,213,                     //vfmadd231ps   %ymm13,%ymm2,%ymm10
  196,66,117,184,212,                     //vfmadd231ps   %ymm12,%ymm1,%ymm10
  196,66,125,184,211,                     //vfmadd231ps   %ymm11,%ymm0,%ymm10
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,124,41,192,                         //vmovaps       %ymm8,%ymm0
  197,124,41,201,                         //vmovaps       %ymm9,%ymm1
  197,124,41,210,                         //vmovaps       %ymm10,%ymm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_perspective_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,98,125,24,72,4,                     //vbroadcastss  0x4(%rax),%ymm9
  196,98,125,24,80,8,                     //vbroadcastss  0x8(%rax),%ymm10
  196,66,117,184,209,                     //vfmadd231ps   %ymm9,%ymm1,%ymm10
  196,66,125,184,208,                     //vfmadd231ps   %ymm8,%ymm0,%ymm10
  196,98,125,24,64,12,                    //vbroadcastss  0xc(%rax),%ymm8
  196,98,125,24,72,16,                    //vbroadcastss  0x10(%rax),%ymm9
  196,98,125,24,88,20,                    //vbroadcastss  0x14(%rax),%ymm11
  196,66,117,184,217,                     //vfmadd231ps   %ymm9,%ymm1,%ymm11
  196,66,125,184,216,                     //vfmadd231ps   %ymm8,%ymm0,%ymm11
  196,98,125,24,64,24,                    //vbroadcastss  0x18(%rax),%ymm8
  196,98,125,24,72,28,                    //vbroadcastss  0x1c(%rax),%ymm9
  196,98,125,24,96,32,                    //vbroadcastss  0x20(%rax),%ymm12
  196,66,117,184,225,                     //vfmadd231ps   %ymm9,%ymm1,%ymm12
  196,66,125,184,224,                     //vfmadd231ps   %ymm8,%ymm0,%ymm12
  196,193,124,83,204,                     //vrcpps        %ymm12,%ymm1
  197,172,89,193,                         //vmulps        %ymm1,%ymm10,%ymm0
  197,164,89,201,                         //vmulps        %ymm1,%ymm11,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_linear_gradient_2stops_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,226,125,24,72,16,                   //vbroadcastss  0x10(%rax),%ymm1
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,98,125,184,193,                     //vfmadd231ps   %ymm1,%ymm0,%ymm8
  196,226,125,24,80,20,                   //vbroadcastss  0x14(%rax),%ymm2
  196,226,125,24,72,4,                    //vbroadcastss  0x4(%rax),%ymm1
  196,226,125,184,202,                    //vfmadd231ps   %ymm2,%ymm0,%ymm1
  196,226,125,24,88,24,                   //vbroadcastss  0x18(%rax),%ymm3
  196,226,125,24,80,8,                    //vbroadcastss  0x8(%rax),%ymm2
  196,226,125,184,211,                    //vfmadd231ps   %ymm3,%ymm0,%ymm2
  196,98,125,24,72,28,                    //vbroadcastss  0x1c(%rax),%ymm9
  196,226,125,24,88,12,                   //vbroadcastss  0xc(%rax),%ymm3
  196,194,125,184,217,                    //vfmadd231ps   %ymm9,%ymm0,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,124,41,192,                         //vmovaps       %ymm8,%ymm0
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_start_pipeline_avx[] = {
  65,87,                                  //push          %r15
  65,86,                                  //push          %r14
  65,85,                                  //push          %r13
  65,84,                                  //push          %r12
  83,                                     //push          %rbx
  73,137,205,                             //mov           %rcx,%r13
  73,137,214,                             //mov           %rdx,%r14
  72,137,251,                             //mov           %rdi,%rbx
  72,173,                                 //lods          %ds:(%rsi),%rax
  73,137,199,                             //mov           %rax,%r15
  73,137,244,                             //mov           %rsi,%r12
  72,141,67,8,                            //lea           0x8(%rbx),%rax
  76,57,232,                              //cmp           %r13,%rax
  118,5,                                  //jbe           28 <_sk_start_pipeline_avx+0x28>
  72,137,223,                             //mov           %rbx,%rdi
  235,65,                                 //jmp           69 <_sk_start_pipeline_avx+0x69>
  185,0,0,0,0,                            //mov           $0x0,%ecx
  197,252,87,192,                         //vxorps        %ymm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  197,236,87,210,                         //vxorps        %ymm2,%ymm2,%ymm2
  197,228,87,219,                         //vxorps        %ymm3,%ymm3,%ymm3
  197,220,87,228,                         //vxorps        %ymm4,%ymm4,%ymm4
  197,212,87,237,                         //vxorps        %ymm5,%ymm5,%ymm5
  197,204,87,246,                         //vxorps        %ymm6,%ymm6,%ymm6
  197,196,87,255,                         //vxorps        %ymm7,%ymm7,%ymm7
  72,137,223,                             //mov           %rbx,%rdi
  76,137,230,                             //mov           %r12,%rsi
  76,137,242,                             //mov           %r14,%rdx
  65,255,215,                             //callq         *%r15
  72,141,123,8,                           //lea           0x8(%rbx),%rdi
  72,131,195,16,                          //add           $0x10,%rbx
  76,57,235,                              //cmp           %r13,%rbx
  72,137,251,                             //mov           %rdi,%rbx
  118,191,                                //jbe           28 <_sk_start_pipeline_avx+0x28>
  76,137,233,                             //mov           %r13,%rcx
  72,41,249,                              //sub           %rdi,%rcx
  116,41,                                 //je            9a <_sk_start_pipeline_avx+0x9a>
  197,252,87,192,                         //vxorps        %ymm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  197,236,87,210,                         //vxorps        %ymm2,%ymm2,%ymm2
  197,228,87,219,                         //vxorps        %ymm3,%ymm3,%ymm3
  197,220,87,228,                         //vxorps        %ymm4,%ymm4,%ymm4
  197,212,87,237,                         //vxorps        %ymm5,%ymm5,%ymm5
  197,204,87,246,                         //vxorps        %ymm6,%ymm6,%ymm6
  197,196,87,255,                         //vxorps        %ymm7,%ymm7,%ymm7
  76,137,230,                             //mov           %r12,%rsi
  76,137,242,                             //mov           %r14,%rdx
  65,255,215,                             //callq         *%r15
  76,137,232,                             //mov           %r13,%rax
  91,                                     //pop           %rbx
  65,92,                                  //pop           %r12
  65,93,                                  //pop           %r13
  65,94,                                  //pop           %r14
  65,95,                                  //pop           %r15
  197,248,119,                            //vzeroupper
  195,                                    //retq
};

CODE const uint8_t sk_just_return_avx[] = {
  195,                                    //retq
};

CODE const uint8_t sk_seed_shader_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,249,110,199,                        //vmovd         %edi,%xmm0
  197,249,112,192,0,                      //vpshufd       $0x0,%xmm0,%xmm0
  196,227,125,24,192,1,                   //vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  197,252,91,192,                         //vcvtdq2ps     %ymm0,%ymm0
  196,226,125,24,74,4,                    //vbroadcastss  0x4(%rdx),%ymm1
  197,252,88,193,                         //vaddps        %ymm1,%ymm0,%ymm0
  197,252,88,66,20,                       //vaddps        0x14(%rdx),%ymm0,%ymm0
  196,226,125,24,16,                      //vbroadcastss  (%rax),%ymm2
  197,252,91,210,                         //vcvtdq2ps     %ymm2,%ymm2
  197,236,88,201,                         //vaddps        %ymm1,%ymm2,%ymm1
  196,226,125,24,18,                      //vbroadcastss  (%rdx),%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,228,87,219,                         //vxorps        %ymm3,%ymm3,%ymm3
  197,220,87,228,                         //vxorps        %ymm4,%ymm4,%ymm4
  197,212,87,237,                         //vxorps        %ymm5,%ymm5,%ymm5
  197,204,87,246,                         //vxorps        %ymm6,%ymm6,%ymm6
  197,196,87,255,                         //vxorps        %ymm7,%ymm7,%ymm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_constant_color_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,226,125,24,0,                       //vbroadcastss  (%rax),%ymm0
  196,226,125,24,72,4,                    //vbroadcastss  0x4(%rax),%ymm1
  196,226,125,24,80,8,                    //vbroadcastss  0x8(%rax),%ymm2
  196,226,125,24,88,12,                   //vbroadcastss  0xc(%rax),%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clear_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,87,192,                         //vxorps        %ymm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  197,236,87,210,                         //vxorps        %ymm2,%ymm2,%ymm2
  197,228,87,219,                         //vxorps        %ymm3,%ymm3,%ymm3
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_plus__avx[] = {
  197,252,88,196,                         //vaddps        %ymm4,%ymm0,%ymm0
  197,244,88,205,                         //vaddps        %ymm5,%ymm1,%ymm1
  197,236,88,214,                         //vaddps        %ymm6,%ymm2,%ymm2
  197,228,88,223,                         //vaddps        %ymm7,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_srcover_avx[] = {
  196,98,125,24,2,                        //vbroadcastss  (%rdx),%ymm8
  197,60,92,195,                          //vsubps        %ymm3,%ymm8,%ymm8
  197,60,89,204,                          //vmulps        %ymm4,%ymm8,%ymm9
  197,180,88,192,                         //vaddps        %ymm0,%ymm9,%ymm0
  197,60,89,205,                          //vmulps        %ymm5,%ymm8,%ymm9
  197,180,88,201,                         //vaddps        %ymm1,%ymm9,%ymm1
  197,60,89,206,                          //vmulps        %ymm6,%ymm8,%ymm9
  197,180,88,210,                         //vaddps        %ymm2,%ymm9,%ymm2
  197,60,89,199,                          //vmulps        %ymm7,%ymm8,%ymm8
  197,188,88,219,                         //vaddps        %ymm3,%ymm8,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_dstover_avx[] = {
  196,98,125,24,2,                        //vbroadcastss  (%rdx),%ymm8
  197,60,92,199,                          //vsubps        %ymm7,%ymm8,%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,252,88,196,                         //vaddps        %ymm4,%ymm0,%ymm0
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,244,88,205,                         //vaddps        %ymm5,%ymm1,%ymm1
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  197,236,88,214,                         //vaddps        %ymm6,%ymm2,%ymm2
  197,188,89,219,                         //vmulps        %ymm3,%ymm8,%ymm3
  197,228,88,223,                         //vaddps        %ymm7,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_0_avx[] = {
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  196,193,124,95,192,                     //vmaxps        %ymm8,%ymm0,%ymm0
  196,193,116,95,200,                     //vmaxps        %ymm8,%ymm1,%ymm1
  196,193,108,95,208,                     //vmaxps        %ymm8,%ymm2,%ymm2
  196,193,100,95,216,                     //vmaxps        %ymm8,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_1_avx[] = {
  196,98,125,24,2,                        //vbroadcastss  (%rdx),%ymm8
  196,193,124,93,192,                     //vminps        %ymm8,%ymm0,%ymm0
  196,193,116,93,200,                     //vminps        %ymm8,%ymm1,%ymm1
  196,193,108,93,208,                     //vminps        %ymm8,%ymm2,%ymm2
  196,193,100,93,216,                     //vminps        %ymm8,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_a_avx[] = {
  196,98,125,24,2,                        //vbroadcastss  (%rdx),%ymm8
  196,193,100,93,216,                     //vminps        %ymm8,%ymm3,%ymm3
  197,252,93,195,                         //vminps        %ymm3,%ymm0,%ymm0
  197,244,93,203,                         //vminps        %ymm3,%ymm1,%ymm1
  197,236,93,211,                         //vminps        %ymm3,%ymm2,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_set_rgb_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,226,125,24,0,                       //vbroadcastss  (%rax),%ymm0
  196,226,125,24,72,4,                    //vbroadcastss  0x4(%rax),%ymm1
  196,226,125,24,80,8,                    //vbroadcastss  0x8(%rax),%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_swap_rb_avx[] = {
  197,124,40,192,                         //vmovaps       %ymm0,%ymm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,40,194,                         //vmovaps       %ymm2,%ymm0
  197,124,41,194,                         //vmovaps       %ymm8,%ymm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_swap_avx[] = {
  197,124,40,195,                         //vmovaps       %ymm3,%ymm8
  197,124,40,202,                         //vmovaps       %ymm2,%ymm9
  197,124,40,209,                         //vmovaps       %ymm1,%ymm10
  197,124,40,216,                         //vmovaps       %ymm0,%ymm11
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,40,196,                         //vmovaps       %ymm4,%ymm0
  197,252,40,205,                         //vmovaps       %ymm5,%ymm1
  197,252,40,214,                         //vmovaps       %ymm6,%ymm2
  197,252,40,223,                         //vmovaps       %ymm7,%ymm3
  197,124,41,220,                         //vmovaps       %ymm11,%ymm4
  197,124,41,213,                         //vmovaps       %ymm10,%ymm5
  197,124,41,206,                         //vmovaps       %ymm9,%ymm6
  197,124,41,199,                         //vmovaps       %ymm8,%ymm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_move_src_dst_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,40,224,                         //vmovaps       %ymm0,%ymm4
  197,252,40,233,                         //vmovaps       %ymm1,%ymm5
  197,252,40,242,                         //vmovaps       %ymm2,%ymm6
  197,252,40,251,                         //vmovaps       %ymm3,%ymm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_move_dst_src_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,40,196,                         //vmovaps       %ymm4,%ymm0
  197,252,40,205,                         //vmovaps       %ymm5,%ymm1
  197,252,40,214,                         //vmovaps       %ymm6,%ymm2
  197,252,40,223,                         //vmovaps       %ymm7,%ymm3
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_premul_avx[] = {
  197,252,89,195,                         //vmulps        %ymm3,%ymm0,%ymm0
  197,244,89,203,                         //vmulps        %ymm3,%ymm1,%ymm1
  197,236,89,211,                         //vmulps        %ymm3,%ymm2,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_unpremul_avx[] = {
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  196,65,100,194,200,0,                   //vcmpeqps      %ymm8,%ymm3,%ymm9
  196,98,125,24,18,                       //vbroadcastss  (%rdx),%ymm10
  197,44,94,211,                          //vdivps        %ymm3,%ymm10,%ymm10
  196,67,45,74,192,144,                   //vblendvps     %ymm9,%ymm8,%ymm10,%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_from_srgb_avx[] = {
  196,98,125,24,66,64,                    //vbroadcastss  0x40(%rdx),%ymm8
  197,60,89,200,                          //vmulps        %ymm0,%ymm8,%ymm9
  197,124,89,208,                         //vmulps        %ymm0,%ymm0,%ymm10
  196,98,125,24,90,60,                    //vbroadcastss  0x3c(%rdx),%ymm11
  196,98,125,24,98,56,                    //vbroadcastss  0x38(%rdx),%ymm12
  197,36,89,232,                          //vmulps        %ymm0,%ymm11,%ymm13
  196,65,20,88,236,                       //vaddps        %ymm12,%ymm13,%ymm13
  196,98,125,24,114,52,                   //vbroadcastss  0x34(%rdx),%ymm14
  196,65,44,89,213,                       //vmulps        %ymm13,%ymm10,%ymm10
  196,65,12,88,210,                       //vaddps        %ymm10,%ymm14,%ymm10
  196,98,125,24,106,68,                   //vbroadcastss  0x44(%rdx),%ymm13
  196,193,124,194,197,1,                  //vcmpltps      %ymm13,%ymm0,%ymm0
  196,195,45,74,193,0,                    //vblendvps     %ymm0,%ymm9,%ymm10,%ymm0
  197,60,89,201,                          //vmulps        %ymm1,%ymm8,%ymm9
  197,116,89,209,                         //vmulps        %ymm1,%ymm1,%ymm10
  197,36,89,249,                          //vmulps        %ymm1,%ymm11,%ymm15
  196,65,4,88,252,                        //vaddps        %ymm12,%ymm15,%ymm15
  196,65,44,89,215,                       //vmulps        %ymm15,%ymm10,%ymm10
  196,65,12,88,210,                       //vaddps        %ymm10,%ymm14,%ymm10
  196,193,116,194,205,1,                  //vcmpltps      %ymm13,%ymm1,%ymm1
  196,195,45,74,201,16,                   //vblendvps     %ymm1,%ymm9,%ymm10,%ymm1
  197,60,89,194,                          //vmulps        %ymm2,%ymm8,%ymm8
  197,108,89,202,                         //vmulps        %ymm2,%ymm2,%ymm9
  197,36,89,210,                          //vmulps        %ymm2,%ymm11,%ymm10
  196,65,44,88,212,                       //vaddps        %ymm12,%ymm10,%ymm10
  196,65,52,89,202,                       //vmulps        %ymm10,%ymm9,%ymm9
  196,65,12,88,201,                       //vaddps        %ymm9,%ymm14,%ymm9
  196,193,108,194,213,1,                  //vcmpltps      %ymm13,%ymm2,%ymm2
  196,195,53,74,208,32,                   //vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_to_srgb_avx[] = {
  197,124,82,192,                         //vrsqrtps      %ymm0,%ymm8
  196,65,124,83,200,                      //vrcpps        %ymm8,%ymm9
  196,65,124,82,208,                      //vrsqrtps      %ymm8,%ymm10
  196,98,125,24,66,72,                    //vbroadcastss  0x48(%rdx),%ymm8
  197,60,89,216,                          //vmulps        %ymm0,%ymm8,%ymm11
  196,98,125,24,34,                       //vbroadcastss  (%rdx),%ymm12
  196,98,125,24,106,76,                   //vbroadcastss  0x4c(%rdx),%ymm13
  196,98,125,24,114,80,                   //vbroadcastss  0x50(%rdx),%ymm14
  196,98,125,24,122,84,                   //vbroadcastss  0x54(%rdx),%ymm15
  196,65,52,89,206,                       //vmulps        %ymm14,%ymm9,%ymm9
  196,65,52,88,207,                       //vaddps        %ymm15,%ymm9,%ymm9
  196,65,44,89,213,                       //vmulps        %ymm13,%ymm10,%ymm10
  196,65,44,88,201,                       //vaddps        %ymm9,%ymm10,%ymm9
  196,65,28,93,201,                       //vminps        %ymm9,%ymm12,%ymm9
  196,98,125,24,82,88,                    //vbroadcastss  0x58(%rdx),%ymm10
  196,193,124,194,194,1,                  //vcmpltps      %ymm10,%ymm0,%ymm0
  196,195,53,74,195,0,                    //vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
  197,124,82,201,                         //vrsqrtps      %ymm1,%ymm9
  196,65,124,83,217,                      //vrcpps        %ymm9,%ymm11
  196,65,124,82,201,                      //vrsqrtps      %ymm9,%ymm9
  196,65,12,89,219,                       //vmulps        %ymm11,%ymm14,%ymm11
  196,65,4,88,219,                        //vaddps        %ymm11,%ymm15,%ymm11
  196,65,20,89,201,                       //vmulps        %ymm9,%ymm13,%ymm9
  196,65,52,88,203,                       //vaddps        %ymm11,%ymm9,%ymm9
  197,60,89,217,                          //vmulps        %ymm1,%ymm8,%ymm11
  196,65,28,93,201,                       //vminps        %ymm9,%ymm12,%ymm9
  196,193,116,194,202,1,                  //vcmpltps      %ymm10,%ymm1,%ymm1
  196,195,53,74,203,16,                   //vblendvps     %ymm1,%ymm11,%ymm9,%ymm1
  197,124,82,202,                         //vrsqrtps      %ymm2,%ymm9
  196,65,124,83,217,                      //vrcpps        %ymm9,%ymm11
  196,65,12,89,219,                       //vmulps        %ymm11,%ymm14,%ymm11
  196,65,4,88,219,                        //vaddps        %ymm11,%ymm15,%ymm11
  196,65,124,82,201,                      //vrsqrtps      %ymm9,%ymm9
  196,65,20,89,201,                       //vmulps        %ymm9,%ymm13,%ymm9
  196,65,52,88,203,                       //vaddps        %ymm11,%ymm9,%ymm9
  196,65,28,93,201,                       //vminps        %ymm9,%ymm12,%ymm9
  197,60,89,194,                          //vmulps        %ymm2,%ymm8,%ymm8
  196,193,108,194,210,1,                  //vcmpltps      %ymm10,%ymm2,%ymm2
  196,195,53,74,208,32,                   //vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_scale_1_float_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  197,188,89,219,                         //vmulps        %ymm3,%ymm8,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_scale_u8_avx[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,1,248,                               //add           %rdi,%rax
  77,133,192,                             //test          %r8,%r8
  117,65,                                 //jne           478 <_sk_scale_u8_avx+0x51>
  197,123,16,0,                           //vmovsd        (%rax),%xmm8
  196,66,121,49,200,                      //vpmovzxbd     %xmm8,%xmm9
  196,67,121,4,192,229,                   //vpermilps     $0xe5,%xmm8,%xmm8
  196,66,121,49,192,                      //vpmovzxbd     %xmm8,%xmm8
  196,67,53,24,192,1,                     //vinsertf128   $0x1,%xmm8,%ymm9,%ymm8
  196,65,124,91,192,                      //vcvtdq2ps     %ymm8,%ymm8
  196,98,125,24,74,12,                    //vbroadcastss  0xc(%rdx),%ymm9
  196,65,60,89,193,                       //vmulps        %ymm9,%ymm8,%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  197,188,89,219,                         //vmulps        %ymm3,%ymm8,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  49,201,                                 //xor           %ecx,%ecx
  77,137,194,                             //mov           %r8,%r10
  69,49,201,                              //xor           %r9d,%r9d
  68,15,182,24,                           //movzbl        (%rax),%r11d
  72,255,192,                             //inc           %rax
  73,211,227,                             //shl           %cl,%r11
  77,9,217,                               //or            %r11,%r9
  72,131,193,8,                           //add           $0x8,%rcx
  73,255,202,                             //dec           %r10
  117,234,                                //jne           480 <_sk_scale_u8_avx+0x59>
  196,65,249,110,193,                     //vmovq         %r9,%xmm8
  235,158,                                //jmp           43b <_sk_scale_u8_avx+0x14>
};

CODE const uint8_t sk_lerp_1_float_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  197,252,92,196,                         //vsubps        %ymm4,%ymm0,%ymm0
  196,193,124,89,192,                     //vmulps        %ymm8,%ymm0,%ymm0
  197,252,88,196,                         //vaddps        %ymm4,%ymm0,%ymm0
  197,244,92,205,                         //vsubps        %ymm5,%ymm1,%ymm1
  196,193,116,89,200,                     //vmulps        %ymm8,%ymm1,%ymm1
  197,244,88,205,                         //vaddps        %ymm5,%ymm1,%ymm1
  197,236,92,214,                         //vsubps        %ymm6,%ymm2,%ymm2
  196,193,108,89,208,                     //vmulps        %ymm8,%ymm2,%ymm2
  197,236,88,214,                         //vaddps        %ymm6,%ymm2,%ymm2
  197,228,92,223,                         //vsubps        %ymm7,%ymm3,%ymm3
  196,193,100,89,216,                     //vmulps        %ymm8,%ymm3,%ymm3
  197,228,88,223,                         //vaddps        %ymm7,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_lerp_u8_avx[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,1,248,                               //add           %rdi,%rax
  77,133,192,                             //test          %r8,%r8
  117,101,                                //jne           551 <_sk_lerp_u8_avx+0x75>
  197,123,16,0,                           //vmovsd        (%rax),%xmm8
  196,66,121,49,200,                      //vpmovzxbd     %xmm8,%xmm9
  196,67,121,4,192,229,                   //vpermilps     $0xe5,%xmm8,%xmm8
  196,66,121,49,192,                      //vpmovzxbd     %xmm8,%xmm8
  196,67,53,24,192,1,                     //vinsertf128   $0x1,%xmm8,%ymm9,%ymm8
  196,65,124,91,192,                      //vcvtdq2ps     %ymm8,%ymm8
  196,98,125,24,74,12,                    //vbroadcastss  0xc(%rdx),%ymm9
  196,65,60,89,193,                       //vmulps        %ymm9,%ymm8,%ymm8
  197,252,92,196,                         //vsubps        %ymm4,%ymm0,%ymm0
  196,193,124,89,192,                     //vmulps        %ymm8,%ymm0,%ymm0
  197,252,88,196,                         //vaddps        %ymm4,%ymm0,%ymm0
  197,244,92,205,                         //vsubps        %ymm5,%ymm1,%ymm1
  196,193,116,89,200,                     //vmulps        %ymm8,%ymm1,%ymm1
  197,244,88,205,                         //vaddps        %ymm5,%ymm1,%ymm1
  197,236,92,214,                         //vsubps        %ymm6,%ymm2,%ymm2
  196,193,108,89,208,                     //vmulps        %ymm8,%ymm2,%ymm2
  197,236,88,214,                         //vaddps        %ymm6,%ymm2,%ymm2
  197,228,92,223,                         //vsubps        %ymm7,%ymm3,%ymm3
  196,193,100,89,216,                     //vmulps        %ymm8,%ymm3,%ymm3
  197,228,88,223,                         //vaddps        %ymm7,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  49,201,                                 //xor           %ecx,%ecx
  77,137,194,                             //mov           %r8,%r10
  69,49,201,                              //xor           %r9d,%r9d
  68,15,182,24,                           //movzbl        (%rax),%r11d
  72,255,192,                             //inc           %rax
  73,211,227,                             //shl           %cl,%r11
  77,9,217,                               //or            %r11,%r9
  72,131,193,8,                           //add           $0x8,%rcx
  73,255,202,                             //dec           %r10
  117,234,                                //jne           559 <_sk_lerp_u8_avx+0x7d>
  196,65,249,110,193,                     //vmovq         %r9,%xmm8
  233,119,255,255,255,                    //jmpq          4f0 <_sk_lerp_u8_avx+0x14>
};

CODE const uint8_t sk_lerp_565_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,16,                              //mov           (%rax),%r10
  72,133,201,                             //test          %rcx,%rcx
  15,133,148,0,0,0,                       //jne           61b <_sk_lerp_565_avx+0xa2>
  196,65,122,111,4,122,                   //vmovdqu       (%r10,%rdi,2),%xmm8
  197,225,239,219,                        //vpxor         %xmm3,%xmm3,%xmm3
  197,185,105,219,                        //vpunpckhwd    %xmm3,%xmm8,%xmm3
  196,66,121,51,192,                      //vpmovzxwd     %xmm8,%xmm8
  196,227,61,24,219,1,                    //vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  196,98,125,24,66,104,                   //vbroadcastss  0x68(%rdx),%ymm8
  197,60,84,195,                          //vandps        %ymm3,%ymm8,%ymm8
  196,65,124,91,192,                      //vcvtdq2ps     %ymm8,%ymm8
  196,98,125,24,74,116,                   //vbroadcastss  0x74(%rdx),%ymm9
  196,65,52,89,192,                       //vmulps        %ymm8,%ymm9,%ymm8
  196,98,125,24,74,108,                   //vbroadcastss  0x6c(%rdx),%ymm9
  197,52,84,203,                          //vandps        %ymm3,%ymm9,%ymm9
  196,65,124,91,201,                      //vcvtdq2ps     %ymm9,%ymm9
  196,98,125,24,82,120,                   //vbroadcastss  0x78(%rdx),%ymm10
  196,65,44,89,201,                       //vmulps        %ymm9,%ymm10,%ymm9
  196,98,125,24,82,112,                   //vbroadcastss  0x70(%rdx),%ymm10
  197,172,84,219,                         //vandps        %ymm3,%ymm10,%ymm3
  197,252,91,219,                         //vcvtdq2ps     %ymm3,%ymm3
  196,98,125,24,82,124,                   //vbroadcastss  0x7c(%rdx),%ymm10
  197,172,89,219,                         //vmulps        %ymm3,%ymm10,%ymm3
  197,252,92,196,                         //vsubps        %ymm4,%ymm0,%ymm0
  196,193,124,89,192,                     //vmulps        %ymm8,%ymm0,%ymm0
  197,252,88,196,                         //vaddps        %ymm4,%ymm0,%ymm0
  197,244,92,205,                         //vsubps        %ymm5,%ymm1,%ymm1
  196,193,116,89,201,                     //vmulps        %ymm9,%ymm1,%ymm1
  197,244,88,205,                         //vaddps        %ymm5,%ymm1,%ymm1
  197,236,92,214,                         //vsubps        %ymm6,%ymm2,%ymm2
  197,236,89,211,                         //vmulps        %ymm3,%ymm2,%ymm2
  197,236,88,214,                         //vaddps        %ymm6,%ymm2,%ymm2
  196,226,125,24,26,                      //vbroadcastss  (%rdx),%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  65,137,200,                             //mov           %ecx,%r8d
  65,128,224,7,                           //and           $0x7,%r8b
  196,65,57,239,192,                      //vpxor         %xmm8,%xmm8,%xmm8
  65,254,200,                             //dec           %r8b
  69,15,182,192,                          //movzbl        %r8b,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  15,135,85,255,255,255,                  //ja            58d <_sk_lerp_565_avx+0x14>
  76,141,13,73,0,0,0,                     //lea           0x49(%rip),%r9        # 688 <_sk_lerp_565_avx+0x10f>
  75,99,4,129,                            //movslq        (%r9,%r8,4),%rax
  76,1,200,                               //add           %r9,%rax
  255,224,                                //jmpq          *%rax
  197,225,239,219,                        //vpxor         %xmm3,%xmm3,%xmm3
  196,65,97,196,68,122,12,6,              //vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm3,%xmm8
  196,65,57,196,68,122,10,5,              //vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm8,%xmm8
  196,65,57,196,68,122,8,4,               //vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm8,%xmm8
  196,65,57,196,68,122,6,3,               //vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm8,%xmm8
  196,65,57,196,68,122,4,2,               //vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm8,%xmm8
  196,65,57,196,68,122,2,1,               //vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm8,%xmm8
  196,65,57,196,4,122,0,                  //vpinsrw       $0x0,(%r10,%rdi,2),%xmm8,%xmm8
  233,5,255,255,255,                      //jmpq          58d <_sk_lerp_565_avx+0x14>
  244,                                    //hlt
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  236,                                    //in            (%dx),%al
  255,                                    //(bad)
  255,                                    //(bad)
  255,228,                                //jmpq          *%rsp
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  220,255,                                //fdivr         %st,%st(7)
  255,                                    //(bad)
  255,212,                                //callq         *%rsp
  255,                                    //(bad)
  255,                                    //(bad)
  255,204,                                //dec           %esp
  255,                                    //(bad)
  255,                                    //(bad)
  255,192,                                //inc           %eax
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_load_tables_avx[] = {
  85,                                     //push          %rbp
  65,87,                                  //push          %r15
  65,86,                                  //push          %r14
  65,85,                                  //push          %r13
  65,84,                                  //push          %r12
  83,                                     //push          %rbx
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,0,                               //mov           (%rax),%r8
  72,133,201,                             //test          %rcx,%rcx
  15,133,18,2,0,0,                        //jne           8ce <_sk_load_tables_avx+0x22a>
  196,65,124,16,4,184,                    //vmovups       (%r8,%rdi,4),%ymm8
  196,98,125,24,74,16,                    //vbroadcastss  0x10(%rdx),%ymm9
  196,193,52,84,192,                      //vandps        %ymm8,%ymm9,%ymm0
  196,193,249,126,193,                    //vmovq         %xmm0,%r9
  69,137,203,                             //mov           %r9d,%r11d
  196,195,249,22,194,1,                   //vpextrq       $0x1,%xmm0,%r10
  69,137,214,                             //mov           %r10d,%r14d
  73,193,234,32,                          //shr           $0x20,%r10
  73,193,233,32,                          //shr           $0x20,%r9
  196,227,125,25,192,1,                   //vextractf128  $0x1,%ymm0,%xmm0
  196,193,249,126,196,                    //vmovq         %xmm0,%r12
  69,137,231,                             //mov           %r12d,%r15d
  196,227,249,22,195,1,                   //vpextrq       $0x1,%xmm0,%rbx
  65,137,221,                             //mov           %ebx,%r13d
  72,193,235,32,                          //shr           $0x20,%rbx
  73,193,236,32,                          //shr           $0x20,%r12
  72,139,104,8,                           //mov           0x8(%rax),%rbp
  76,139,64,16,                           //mov           0x10(%rax),%r8
  196,161,122,16,68,189,0,                //vmovss        0x0(%rbp,%r15,4),%xmm0
  196,163,121,33,68,165,0,16,             //vinsertps     $0x10,0x0(%rbp,%r12,4),%xmm0,%xmm0
  196,163,121,33,68,173,0,32,             //vinsertps     $0x20,0x0(%rbp,%r13,4),%xmm0,%xmm0
  197,250,16,76,157,0,                    //vmovss        0x0(%rbp,%rbx,4),%xmm1
  196,227,121,33,193,48,                  //vinsertps     $0x30,%xmm1,%xmm0,%xmm0
  196,161,122,16,76,157,0,                //vmovss        0x0(%rbp,%r11,4),%xmm1
  196,163,113,33,76,141,0,16,             //vinsertps     $0x10,0x0(%rbp,%r9,4),%xmm1,%xmm1
  196,163,113,33,76,181,0,32,             //vinsertps     $0x20,0x0(%rbp,%r14,4),%xmm1,%xmm1
  196,161,122,16,92,149,0,                //vmovss        0x0(%rbp,%r10,4),%xmm3
  196,227,113,33,203,48,                  //vinsertps     $0x30,%xmm3,%xmm1,%xmm1
  196,227,117,24,192,1,                   //vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  196,193,113,114,208,8,                  //vpsrld        $0x8,%xmm8,%xmm1
  196,67,125,25,194,1,                    //vextractf128  $0x1,%ymm8,%xmm10
  196,193,105,114,210,8,                  //vpsrld        $0x8,%xmm10,%xmm2
  196,227,117,24,202,1,                   //vinsertf128   $0x1,%xmm2,%ymm1,%ymm1
  197,180,84,201,                         //vandps        %ymm1,%ymm9,%ymm1
  196,193,249,126,201,                    //vmovq         %xmm1,%r9
  69,137,203,                             //mov           %r9d,%r11d
  196,195,249,22,202,1,                   //vpextrq       $0x1,%xmm1,%r10
  69,137,214,                             //mov           %r10d,%r14d
  73,193,234,32,                          //shr           $0x20,%r10
  73,193,233,32,                          //shr           $0x20,%r9
  196,227,125,25,201,1,                   //vextractf128  $0x1,%ymm1,%xmm1
  196,225,249,126,205,                    //vmovq         %xmm1,%rbp
  65,137,239,                             //mov           %ebp,%r15d
  196,227,249,22,203,1,                   //vpextrq       $0x1,%xmm1,%rbx
  65,137,220,                             //mov           %ebx,%r12d
  72,193,235,32,                          //shr           $0x20,%rbx
  72,193,237,32,                          //shr           $0x20,%rbp
  196,129,122,16,12,184,                  //vmovss        (%r8,%r15,4),%xmm1
  196,195,113,33,12,168,16,               //vinsertps     $0x10,(%r8,%rbp,4),%xmm1,%xmm1
  196,129,122,16,20,160,                  //vmovss        (%r8,%r12,4),%xmm2
  196,227,113,33,202,32,                  //vinsertps     $0x20,%xmm2,%xmm1,%xmm1
  196,193,122,16,20,152,                  //vmovss        (%r8,%rbx,4),%xmm2
  196,227,113,33,202,48,                  //vinsertps     $0x30,%xmm2,%xmm1,%xmm1
  196,129,122,16,20,152,                  //vmovss        (%r8,%r11,4),%xmm2
  196,131,105,33,20,136,16,               //vinsertps     $0x10,(%r8,%r9,4),%xmm2,%xmm2
  196,129,122,16,28,176,                  //vmovss        (%r8,%r14,4),%xmm3
  196,227,105,33,211,32,                  //vinsertps     $0x20,%xmm3,%xmm2,%xmm2
  196,129,122,16,28,144,                  //vmovss        (%r8,%r10,4),%xmm3
  196,227,105,33,211,48,                  //vinsertps     $0x30,%xmm3,%xmm2,%xmm2
  196,227,109,24,201,1,                   //vinsertf128   $0x1,%xmm1,%ymm2,%ymm1
  72,139,64,24,                           //mov           0x18(%rax),%rax
  196,193,105,114,208,16,                 //vpsrld        $0x10,%xmm8,%xmm2
  196,193,97,114,210,16,                  //vpsrld        $0x10,%xmm10,%xmm3
  196,227,109,24,211,1,                   //vinsertf128   $0x1,%xmm3,%ymm2,%ymm2
  197,180,84,210,                         //vandps        %ymm2,%ymm9,%ymm2
  196,193,249,126,208,                    //vmovq         %xmm2,%r8
  69,137,194,                             //mov           %r8d,%r10d
  196,195,249,22,209,1,                   //vpextrq       $0x1,%xmm2,%r9
  69,137,203,                             //mov           %r9d,%r11d
  73,193,233,32,                          //shr           $0x20,%r9
  73,193,232,32,                          //shr           $0x20,%r8
  196,227,125,25,210,1,                   //vextractf128  $0x1,%ymm2,%xmm2
  196,225,249,126,213,                    //vmovq         %xmm2,%rbp
  65,137,238,                             //mov           %ebp,%r14d
  196,227,249,22,211,1,                   //vpextrq       $0x1,%xmm2,%rbx
  65,137,223,                             //mov           %ebx,%r15d
  72,193,235,32,                          //shr           $0x20,%rbx
  72,193,237,32,                          //shr           $0x20,%rbp
  196,161,122,16,20,176,                  //vmovss        (%rax,%r14,4),%xmm2
  196,227,105,33,20,168,16,               //vinsertps     $0x10,(%rax,%rbp,4),%xmm2,%xmm2
  196,161,122,16,28,184,                  //vmovss        (%rax,%r15,4),%xmm3
  196,227,105,33,211,32,                  //vinsertps     $0x20,%xmm3,%xmm2,%xmm2
  197,250,16,28,152,                      //vmovss        (%rax,%rbx,4),%xmm3
  196,99,105,33,203,48,                   //vinsertps     $0x30,%xmm3,%xmm2,%xmm9
  196,161,122,16,28,144,                  //vmovss        (%rax,%r10,4),%xmm3
  196,163,97,33,28,128,16,                //vinsertps     $0x10,(%rax,%r8,4),%xmm3,%xmm3
  196,161,122,16,20,152,                  //vmovss        (%rax,%r11,4),%xmm2
  196,227,97,33,210,32,                   //vinsertps     $0x20,%xmm2,%xmm3,%xmm2
  196,161,122,16,28,136,                  //vmovss        (%rax,%r9,4),%xmm3
  196,227,105,33,211,48,                  //vinsertps     $0x30,%xmm3,%xmm2,%xmm2
  196,195,109,24,209,1,                   //vinsertf128   $0x1,%xmm9,%ymm2,%ymm2
  196,193,57,114,208,24,                  //vpsrld        $0x18,%xmm8,%xmm8
  196,193,97,114,210,24,                  //vpsrld        $0x18,%xmm10,%xmm3
  196,227,61,24,219,1,                    //vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  197,252,91,219,                         //vcvtdq2ps     %ymm3,%ymm3
  196,98,125,24,66,12,                    //vbroadcastss  0xc(%rdx),%ymm8
  196,193,100,89,216,                     //vmulps        %ymm8,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  91,                                     //pop           %rbx
  65,92,                                  //pop           %r12
  65,93,                                  //pop           %r13
  65,94,                                  //pop           %r14
  65,95,                                  //pop           %r15
  93,                                     //pop           %rbp
  255,224,                                //jmpq          *%rax
  65,137,201,                             //mov           %ecx,%r9d
  65,128,225,7,                           //and           $0x7,%r9b
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  65,254,201,                             //dec           %r9b
  69,15,182,201,                          //movzbl        %r9b,%r9d
  65,128,249,6,                           //cmp           $0x6,%r9b
  15,135,215,253,255,255,                 //ja            6c2 <_sk_load_tables_avx+0x1e>
  76,141,21,138,0,0,0,                    //lea           0x8a(%rip),%r10        # 97c <_sk_load_tables_avx+0x2d8>
  79,99,12,138,                           //movslq        (%r10,%r9,4),%r9
  77,1,209,                               //add           %r10,%r9
  65,255,225,                             //jmpq          *%r9
  196,193,121,110,68,184,24,              //vmovd         0x18(%r8,%rdi,4),%xmm0
  197,249,112,192,68,                     //vpshufd       $0x44,%xmm0,%xmm0
  196,227,125,24,192,1,                   //vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  196,99,117,12,192,64,                   //vblendps      $0x40,%ymm0,%ymm1,%ymm8
  196,99,125,25,192,1,                    //vextractf128  $0x1,%ymm8,%xmm0
  196,195,121,34,68,184,20,1,             //vpinsrd       $0x1,0x14(%r8,%rdi,4),%xmm0,%xmm0
  196,99,61,24,192,1,                     //vinsertf128   $0x1,%xmm0,%ymm8,%ymm8
  196,99,125,25,192,1,                    //vextractf128  $0x1,%ymm8,%xmm0
  196,195,121,34,68,184,16,0,             //vpinsrd       $0x0,0x10(%r8,%rdi,4),%xmm0,%xmm0
  196,99,61,24,192,1,                     //vinsertf128   $0x1,%xmm0,%ymm8,%ymm8
  196,195,57,34,68,184,12,3,              //vpinsrd       $0x3,0xc(%r8,%rdi,4),%xmm8,%xmm0
  196,99,61,12,192,15,                    //vblendps      $0xf,%ymm0,%ymm8,%ymm8
  196,195,57,34,68,184,8,2,               //vpinsrd       $0x2,0x8(%r8,%rdi,4),%xmm8,%xmm0
  196,99,61,12,192,15,                    //vblendps      $0xf,%ymm0,%ymm8,%ymm8
  196,195,57,34,68,184,4,1,               //vpinsrd       $0x1,0x4(%r8,%rdi,4),%xmm8,%xmm0
  196,99,61,12,192,15,                    //vblendps      $0xf,%ymm0,%ymm8,%ymm8
  196,195,57,34,4,184,0,                  //vpinsrd       $0x0,(%r8,%rdi,4),%xmm8,%xmm0
  196,99,61,12,192,15,                    //vblendps      $0xf,%ymm0,%ymm8,%ymm8
  233,70,253,255,255,                     //jmpq          6c2 <_sk_load_tables_avx+0x1e>
  238,                                    //out           %al,(%dx)
  255,                                    //(bad)
  255,                                    //(bad)
  255,224,                                //jmpq          *%rax
  255,                                    //(bad)
  255,                                    //(bad)
  255,210,                                //callq         *%rdx
  255,                                    //(bad)
  255,                                    //(bad)
  255,196,                                //inc           %esp
  255,                                    //(bad)
  255,                                    //(bad)
  255,176,255,255,255,156,                //pushq         -0x63000001(%rax)
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
  128,255,255,                            //cmp           $0xff,%bh
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_load_a8_avx[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,1,248,                               //add           %rdi,%rax
  77,133,192,                             //test          %r8,%r8
  117,59,                                 //jne           9e3 <_sk_load_a8_avx+0x4b>
  197,251,16,0,                           //vmovsd        (%rax),%xmm0
  196,226,121,49,200,                     //vpmovzxbd     %xmm0,%xmm1
  196,227,121,4,192,229,                  //vpermilps     $0xe5,%xmm0,%xmm0
  196,226,121,49,192,                     //vpmovzxbd     %xmm0,%xmm0
  196,227,117,24,192,1,                   //vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  197,252,91,192,                         //vcvtdq2ps     %ymm0,%ymm0
  196,226,125,24,74,12,                   //vbroadcastss  0xc(%rdx),%ymm1
  197,252,89,217,                         //vmulps        %ymm1,%ymm0,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,87,192,                         //vxorps        %ymm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  197,236,87,210,                         //vxorps        %ymm2,%ymm2,%ymm2
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  49,201,                                 //xor           %ecx,%ecx
  77,137,194,                             //mov           %r8,%r10
  69,49,201,                              //xor           %r9d,%r9d
  68,15,182,24,                           //movzbl        (%rax),%r11d
  72,255,192,                             //inc           %rax
  73,211,227,                             //shl           %cl,%r11
  77,9,217,                               //or            %r11,%r9
  72,131,193,8,                           //add           $0x8,%rcx
  73,255,202,                             //dec           %r10
  117,234,                                //jne           9eb <_sk_load_a8_avx+0x53>
  196,193,249,110,193,                    //vmovq         %r9,%xmm0
  235,164,                                //jmp           9ac <_sk_load_a8_avx+0x14>
};

CODE const uint8_t sk_store_a8_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,8,                               //mov           (%rax),%r9
  196,98,125,24,66,8,                     //vbroadcastss  0x8(%rdx),%ymm8
  197,60,89,195,                          //vmulps        %ymm3,%ymm8,%ymm8
  196,65,125,91,192,                      //vcvtps2dq     %ymm8,%ymm8
  196,67,125,25,193,1,                    //vextractf128  $0x1,%ymm8,%xmm9
  196,66,57,43,193,                       //vpackusdw     %xmm9,%xmm8,%xmm8
  196,65,57,103,192,                      //vpackuswb     %xmm8,%xmm8,%xmm8
  72,133,201,                             //test          %rcx,%rcx
  117,10,                                 //jne           a3b <_sk_store_a8_avx+0x33>
  196,65,123,17,4,57,                     //vmovsd        %xmm8,(%r9,%rdi,1)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  137,200,                                //mov           %ecx,%eax
  36,7,                                   //and           $0x7,%al
  254,200,                                //dec           %al
  68,15,182,192,                          //movzbl        %al,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  119,236,                                //ja            a37 <_sk_store_a8_avx+0x2f>
  196,66,121,48,192,                      //vpmovzxbw     %xmm8,%xmm8
  76,141,21,69,0,0,0,                     //lea           0x45(%rip),%r10        # a9c <_sk_store_a8_avx+0x94>
  75,99,4,130,                            //movslq        (%r10,%r8,4),%rax
  76,1,208,                               //add           %r10,%rax
  255,224,                                //jmpq          *%rax
  196,67,121,20,68,57,6,12,               //vpextrb       $0xc,%xmm8,0x6(%r9,%rdi,1)
  196,67,121,20,68,57,5,10,               //vpextrb       $0xa,%xmm8,0x5(%r9,%rdi,1)
  196,67,121,20,68,57,4,8,                //vpextrb       $0x8,%xmm8,0x4(%r9,%rdi,1)
  196,67,121,20,68,57,3,6,                //vpextrb       $0x6,%xmm8,0x3(%r9,%rdi,1)
  196,67,121,20,68,57,2,4,                //vpextrb       $0x4,%xmm8,0x2(%r9,%rdi,1)
  196,67,121,20,68,57,1,2,                //vpextrb       $0x2,%xmm8,0x1(%r9,%rdi,1)
  196,67,121,20,4,57,0,                   //vpextrb       $0x0,%xmm8,(%r9,%rdi,1)
  235,158,                                //jmp           a37 <_sk_store_a8_avx+0x2f>
  15,31,0,                                //nopl          (%rax)
  244,                                    //hlt
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  236,                                    //in            (%dx),%al
  255,                                    //(bad)
  255,                                    //(bad)
  255,228,                                //jmpq          *%rsp
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  220,255,                                //fdivr         %st,%st(7)
  255,                                    //(bad)
  255,212,                                //callq         *%rsp
  255,                                    //(bad)
  255,                                    //(bad)
  255,204,                                //dec           %esp
  255,                                    //(bad)
  255,                                    //(bad)
  255,196,                                //inc           %esp
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_load_565_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,16,                              //mov           (%rax),%r10
  72,133,201,                             //test          %rcx,%rcx
  117,106,                                //jne           b2c <_sk_load_565_avx+0x74>
  196,193,122,111,4,122,                  //vmovdqu       (%r10,%rdi,2),%xmm0
  197,241,239,201,                        //vpxor         %xmm1,%xmm1,%xmm1
  197,249,105,201,                        //vpunpckhwd    %xmm1,%xmm0,%xmm1
  196,226,121,51,192,                     //vpmovzxwd     %xmm0,%xmm0
  196,227,125,24,209,1,                   //vinsertf128   $0x1,%xmm1,%ymm0,%ymm2
  196,226,125,24,66,104,                  //vbroadcastss  0x68(%rdx),%ymm0
  197,252,84,194,                         //vandps        %ymm2,%ymm0,%ymm0
  197,252,91,192,                         //vcvtdq2ps     %ymm0,%ymm0
  196,226,125,24,74,116,                  //vbroadcastss  0x74(%rdx),%ymm1
  197,244,89,192,                         //vmulps        %ymm0,%ymm1,%ymm0
  196,226,125,24,74,108,                  //vbroadcastss  0x6c(%rdx),%ymm1
  197,244,84,202,                         //vandps        %ymm2,%ymm1,%ymm1
  197,252,91,201,                         //vcvtdq2ps     %ymm1,%ymm1
  196,226,125,24,90,120,                  //vbroadcastss  0x78(%rdx),%ymm3
  197,228,89,201,                         //vmulps        %ymm1,%ymm3,%ymm1
  196,226,125,24,90,112,                  //vbroadcastss  0x70(%rdx),%ymm3
  197,228,84,210,                         //vandps        %ymm2,%ymm3,%ymm2
  197,252,91,210,                         //vcvtdq2ps     %ymm2,%ymm2
  196,226,125,24,90,124,                  //vbroadcastss  0x7c(%rdx),%ymm3
  197,228,89,210,                         //vmulps        %ymm2,%ymm3,%ymm2
  196,226,125,24,26,                      //vbroadcastss  (%rdx),%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  65,137,200,                             //mov           %ecx,%r8d
  65,128,224,7,                           //and           $0x7,%r8b
  197,249,239,192,                        //vpxor         %xmm0,%xmm0,%xmm0
  65,254,200,                             //dec           %r8b
  69,15,182,192,                          //movzbl        %r8b,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  119,132,                                //ja            ac8 <_sk_load_565_avx+0x10>
  76,141,13,73,0,0,0,                     //lea           0x49(%rip),%r9        # b94 <_sk_load_565_avx+0xdc>
  75,99,4,129,                            //movslq        (%r9,%r8,4),%rax
  76,1,200,                               //add           %r9,%rax
  255,224,                                //jmpq          *%rax
  197,249,239,192,                        //vpxor         %xmm0,%xmm0,%xmm0
  196,193,121,196,68,122,12,6,            //vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,10,5,            //vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,8,4,             //vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,6,3,             //vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,4,2,             //vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,2,1,             //vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,4,122,0,                //vpinsrw       $0x0,(%r10,%rdi,2),%xmm0,%xmm0
  233,52,255,255,255,                     //jmpq          ac8 <_sk_load_565_avx+0x10>
  244,                                    //hlt
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  236,                                    //in            (%dx),%al
  255,                                    //(bad)
  255,                                    //(bad)
  255,228,                                //jmpq          *%rsp
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  220,255,                                //fdivr         %st,%st(7)
  255,                                    //(bad)
  255,212,                                //callq         *%rsp
  255,                                    //(bad)
  255,                                    //(bad)
  255,204,                                //dec           %esp
  255,                                    //(bad)
  255,                                    //(bad)
  255,192,                                //inc           %eax
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_store_565_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,8,                               //mov           (%rax),%r9
  196,98,125,24,130,128,0,0,0,            //vbroadcastss  0x80(%rdx),%ymm8
  197,60,89,200,                          //vmulps        %ymm0,%ymm8,%ymm9
  196,65,125,91,201,                      //vcvtps2dq     %ymm9,%ymm9
  196,193,41,114,241,11,                  //vpslld        $0xb,%xmm9,%xmm10
  196,67,125,25,201,1,                    //vextractf128  $0x1,%ymm9,%xmm9
  196,193,49,114,241,11,                  //vpslld        $0xb,%xmm9,%xmm9
  196,67,45,24,201,1,                     //vinsertf128   $0x1,%xmm9,%ymm10,%ymm9
  196,98,125,24,146,132,0,0,0,            //vbroadcastss  0x84(%rdx),%ymm10
  197,44,89,209,                          //vmulps        %ymm1,%ymm10,%ymm10
  196,65,125,91,210,                      //vcvtps2dq     %ymm10,%ymm10
  196,193,33,114,242,5,                   //vpslld        $0x5,%xmm10,%xmm11
  196,67,125,25,210,1,                    //vextractf128  $0x1,%ymm10,%xmm10
  196,193,41,114,242,5,                   //vpslld        $0x5,%xmm10,%xmm10
  196,67,37,24,210,1,                     //vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  196,65,45,86,201,                       //vorpd         %ymm9,%ymm10,%ymm9
  197,60,89,194,                          //vmulps        %ymm2,%ymm8,%ymm8
  196,65,125,91,192,                      //vcvtps2dq     %ymm8,%ymm8
  196,65,53,86,192,                       //vorpd         %ymm8,%ymm9,%ymm8
  196,67,125,25,193,1,                    //vextractf128  $0x1,%ymm8,%xmm9
  196,66,57,43,193,                       //vpackusdw     %xmm9,%xmm8,%xmm8
  72,133,201,                             //test          %rcx,%rcx
  117,10,                                 //jne           c36 <_sk_store_565_avx+0x86>
  196,65,122,127,4,121,                   //vmovdqu       %xmm8,(%r9,%rdi,2)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  137,200,                                //mov           %ecx,%eax
  36,7,                                   //and           $0x7,%al
  254,200,                                //dec           %al
  68,15,182,192,                          //movzbl        %al,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  119,236,                                //ja            c32 <_sk_store_565_avx+0x82>
  76,141,21,71,0,0,0,                     //lea           0x47(%rip),%r10        # c94 <_sk_store_565_avx+0xe4>
  75,99,4,130,                            //movslq        (%r10,%r8,4),%rax
  76,1,208,                               //add           %r10,%rax
  255,224,                                //jmpq          *%rax
  196,67,121,21,68,121,12,6,              //vpextrw       $0x6,%xmm8,0xc(%r9,%rdi,2)
  196,67,121,21,68,121,10,5,              //vpextrw       $0x5,%xmm8,0xa(%r9,%rdi,2)
  196,67,121,21,68,121,8,4,               //vpextrw       $0x4,%xmm8,0x8(%r9,%rdi,2)
  196,67,121,21,68,121,6,3,               //vpextrw       $0x3,%xmm8,0x6(%r9,%rdi,2)
  196,67,121,21,68,121,4,2,               //vpextrw       $0x2,%xmm8,0x4(%r9,%rdi,2)
  196,67,121,21,68,121,2,1,               //vpextrw       $0x1,%xmm8,0x2(%r9,%rdi,2)
  197,121,126,192,                        //vmovd         %xmm8,%eax
  102,65,137,4,121,                       //mov           %ax,(%r9,%rdi,2)
  235,161,                                //jmp           c32 <_sk_store_565_avx+0x82>
  15,31,0,                                //nopl          (%rax)
  242,255,                                //repnz         (bad)
  255,                                    //(bad)
  255,                                    //(bad)
  234,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  255,226,                                //jmpq          *%rdx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  218,255,                                //(bad)
  255,                                    //(bad)
  255,210,                                //callq         *%rdx
  255,                                    //(bad)
  255,                                    //(bad)
  255,202,                                //dec           %edx
  255,                                    //(bad)
  255,                                    //(bad)
  255,194,                                //inc           %edx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_load_8888_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,16,                              //mov           (%rax),%r10
  72,133,201,                             //test          %rcx,%rcx
  117,125,                                //jne           d37 <_sk_load_8888_avx+0x87>
  196,65,124,16,12,186,                   //vmovups       (%r10,%rdi,4),%ymm9
  196,98,125,24,90,16,                    //vbroadcastss  0x10(%rdx),%ymm11
  196,193,36,84,193,                      //vandps        %ymm9,%ymm11,%ymm0
  197,252,91,192,                         //vcvtdq2ps     %ymm0,%ymm0
  196,98,125,24,66,12,                    //vbroadcastss  0xc(%rdx),%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  196,193,41,114,209,8,                   //vpsrld        $0x8,%xmm9,%xmm10
  196,99,125,25,203,1,                    //vextractf128  $0x1,%ymm9,%xmm3
  197,241,114,211,8,                      //vpsrld        $0x8,%xmm3,%xmm1
  196,227,45,24,201,1,                    //vinsertf128   $0x1,%xmm1,%ymm10,%ymm1
  197,164,84,201,                         //vandps        %ymm1,%ymm11,%ymm1
  197,252,91,201,                         //vcvtdq2ps     %ymm1,%ymm1
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  196,193,41,114,209,16,                  //vpsrld        $0x10,%xmm9,%xmm10
  197,233,114,211,16,                     //vpsrld        $0x10,%xmm3,%xmm2
  196,227,45,24,210,1,                    //vinsertf128   $0x1,%xmm2,%ymm10,%ymm2
  197,164,84,210,                         //vandps        %ymm2,%ymm11,%ymm2
  197,252,91,210,                         //vcvtdq2ps     %ymm2,%ymm2
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  196,193,49,114,209,24,                  //vpsrld        $0x18,%xmm9,%xmm9
  197,225,114,211,24,                     //vpsrld        $0x18,%xmm3,%xmm3
  196,227,53,24,219,1,                    //vinsertf128   $0x1,%xmm3,%ymm9,%ymm3
  197,252,91,219,                         //vcvtdq2ps     %ymm3,%ymm3
  196,193,100,89,216,                     //vmulps        %ymm8,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  65,137,200,                             //mov           %ecx,%r8d
  65,128,224,7,                           //and           $0x7,%r8b
  196,65,52,87,201,                       //vxorps        %ymm9,%ymm9,%ymm9
  65,254,200,                             //dec           %r8b
  69,15,182,192,                          //movzbl        %r8b,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  15,135,108,255,255,255,                 //ja            cc0 <_sk_load_8888_avx+0x10>
  76,141,13,137,0,0,0,                    //lea           0x89(%rip),%r9        # de4 <_sk_load_8888_avx+0x134>
  75,99,4,129,                            //movslq        (%r9,%r8,4),%rax
  76,1,200,                               //add           %r9,%rax
  255,224,                                //jmpq          *%rax
  196,193,121,110,68,186,24,              //vmovd         0x18(%r10,%rdi,4),%xmm0
  197,249,112,192,68,                     //vpshufd       $0x44,%xmm0,%xmm0
  196,227,125,24,192,1,                   //vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  196,99,117,12,200,64,                   //vblendps      $0x40,%ymm0,%ymm1,%ymm9
  196,99,125,25,200,1,                    //vextractf128  $0x1,%ymm9,%xmm0
  196,195,121,34,68,186,20,1,             //vpinsrd       $0x1,0x14(%r10,%rdi,4),%xmm0,%xmm0
  196,99,53,24,200,1,                     //vinsertf128   $0x1,%xmm0,%ymm9,%ymm9
  196,99,125,25,200,1,                    //vextractf128  $0x1,%ymm9,%xmm0
  196,195,121,34,68,186,16,0,             //vpinsrd       $0x0,0x10(%r10,%rdi,4),%xmm0,%xmm0
  196,99,53,24,200,1,                     //vinsertf128   $0x1,%xmm0,%ymm9,%ymm9
  196,195,49,34,68,186,12,3,              //vpinsrd       $0x3,0xc(%r10,%rdi,4),%xmm9,%xmm0
  196,99,53,12,200,15,                    //vblendps      $0xf,%ymm0,%ymm9,%ymm9
  196,195,49,34,68,186,8,2,               //vpinsrd       $0x2,0x8(%r10,%rdi,4),%xmm9,%xmm0
  196,99,53,12,200,15,                    //vblendps      $0xf,%ymm0,%ymm9,%ymm9
  196,195,49,34,68,186,4,1,               //vpinsrd       $0x1,0x4(%r10,%rdi,4),%xmm9,%xmm0
  196,99,53,12,200,15,                    //vblendps      $0xf,%ymm0,%ymm9,%ymm9
  196,195,49,34,4,186,0,                  //vpinsrd       $0x0,(%r10,%rdi,4),%xmm9,%xmm0
  196,99,53,12,200,15,                    //vblendps      $0xf,%ymm0,%ymm9,%ymm9
  233,220,254,255,255,                    //jmpq          cc0 <_sk_load_8888_avx+0x10>
  238,                                    //out           %al,(%dx)
  255,                                    //(bad)
  255,                                    //(bad)
  255,224,                                //jmpq          *%rax
  255,                                    //(bad)
  255,                                    //(bad)
  255,210,                                //callq         *%rdx
  255,                                    //(bad)
  255,                                    //(bad)
  255,196,                                //inc           %esp
  255,                                    //(bad)
  255,                                    //(bad)
  255,176,255,255,255,156,                //pushq         -0x63000001(%rax)
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
  128,255,255,                            //cmp           $0xff,%bh
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_store_8888_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,8,                               //mov           (%rax),%r9
  196,98,125,24,66,8,                     //vbroadcastss  0x8(%rdx),%ymm8
  197,60,89,200,                          //vmulps        %ymm0,%ymm8,%ymm9
  196,65,125,91,201,                      //vcvtps2dq     %ymm9,%ymm9
  197,60,89,209,                          //vmulps        %ymm1,%ymm8,%ymm10
  196,65,125,91,210,                      //vcvtps2dq     %ymm10,%ymm10
  196,193,33,114,242,8,                   //vpslld        $0x8,%xmm10,%xmm11
  196,67,125,25,210,1,                    //vextractf128  $0x1,%ymm10,%xmm10
  196,193,41,114,242,8,                   //vpslld        $0x8,%xmm10,%xmm10
  196,67,37,24,210,1,                     //vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  196,65,45,86,201,                       //vorpd         %ymm9,%ymm10,%ymm9
  197,60,89,210,                          //vmulps        %ymm2,%ymm8,%ymm10
  196,65,125,91,210,                      //vcvtps2dq     %ymm10,%ymm10
  196,193,33,114,242,16,                  //vpslld        $0x10,%xmm10,%xmm11
  196,67,125,25,210,1,                    //vextractf128  $0x1,%ymm10,%xmm10
  196,193,41,114,242,16,                  //vpslld        $0x10,%xmm10,%xmm10
  196,67,37,24,210,1,                     //vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  197,60,89,195,                          //vmulps        %ymm3,%ymm8,%ymm8
  196,65,125,91,192,                      //vcvtps2dq     %ymm8,%ymm8
  196,193,33,114,240,24,                  //vpslld        $0x18,%xmm8,%xmm11
  196,67,125,25,192,1,                    //vextractf128  $0x1,%ymm8,%xmm8
  196,193,57,114,240,24,                  //vpslld        $0x18,%xmm8,%xmm8
  196,67,37,24,192,1,                     //vinsertf128   $0x1,%xmm8,%ymm11,%ymm8
  196,65,45,86,192,                       //vorpd         %ymm8,%ymm10,%ymm8
  196,65,53,86,192,                       //vorpd         %ymm8,%ymm9,%ymm8
  72,133,201,                             //test          %rcx,%rcx
  117,10,                                 //jne           e95 <_sk_store_8888_avx+0x95>
  196,65,124,17,4,185,                    //vmovups       %ymm8,(%r9,%rdi,4)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  137,200,                                //mov           %ecx,%eax
  36,7,                                   //and           $0x7,%al
  254,200,                                //dec           %al
  68,15,182,192,                          //movzbl        %al,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  119,236,                                //ja            e91 <_sk_store_8888_avx+0x91>
  76,141,21,84,0,0,0,                     //lea           0x54(%rip),%r10        # f00 <_sk_store_8888_avx+0x100>
  75,99,4,130,                            //movslq        (%r10,%r8,4),%rax
  76,1,208,                               //add           %r10,%rax
  255,224,                                //jmpq          *%rax
  196,67,125,25,193,1,                    //vextractf128  $0x1,%ymm8,%xmm9
  196,67,121,22,76,185,24,2,              //vpextrd       $0x2,%xmm9,0x18(%r9,%rdi,4)
  196,67,125,25,193,1,                    //vextractf128  $0x1,%ymm8,%xmm9
  196,67,121,22,76,185,20,1,              //vpextrd       $0x1,%xmm9,0x14(%r9,%rdi,4)
  196,67,125,25,193,1,                    //vextractf128  $0x1,%ymm8,%xmm9
  196,65,121,126,76,185,16,               //vmovd         %xmm9,0x10(%r9,%rdi,4)
  196,67,121,22,68,185,12,3,              //vpextrd       $0x3,%xmm8,0xc(%r9,%rdi,4)
  196,67,121,22,68,185,8,2,               //vpextrd       $0x2,%xmm8,0x8(%r9,%rdi,4)
  196,67,121,22,68,185,4,1,               //vpextrd       $0x1,%xmm8,0x4(%r9,%rdi,4)
  196,65,121,126,4,185,                   //vmovd         %xmm8,(%r9,%rdi,4)
  235,147,                                //jmp           e91 <_sk_store_8888_avx+0x91>
  102,144,                                //xchg          %ax,%ax
  246,255,                                //idiv          %bh
  255,                                    //(bad)
  255,                                    //(bad)
  238,                                    //out           %al,(%dx)
  255,                                    //(bad)
  255,                                    //(bad)
  255,230,                                //jmpq          *%rsi
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  222,255,                                //fdivrp        %st,%st(7)
  255,                                    //(bad)
  255,209,                                //callq         *%rcx
  255,                                    //(bad)
  255,                                    //(bad)
  255,195,                                //inc           %ebx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
  181,255,                                //mov           $0xff,%ch
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_load_f16_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,133,201,                             //test          %rcx,%rcx
  15,133,240,0,0,0,                       //jne           101a <_sk_load_f16_avx+0xfe>
  197,249,16,12,248,                      //vmovupd       (%rax,%rdi,8),%xmm1
  197,249,16,84,248,16,                   //vmovupd       0x10(%rax,%rdi,8),%xmm2
  197,249,16,92,248,32,                   //vmovupd       0x20(%rax,%rdi,8),%xmm3
  197,121,16,68,248,48,                   //vmovupd       0x30(%rax,%rdi,8),%xmm8
  197,241,97,194,                         //vpunpcklwd    %xmm2,%xmm1,%xmm0
  197,241,105,202,                        //vpunpckhwd    %xmm2,%xmm1,%xmm1
  196,193,97,97,208,                      //vpunpcklwd    %xmm8,%xmm3,%xmm2
  196,193,97,105,216,                     //vpunpckhwd    %xmm8,%xmm3,%xmm3
  197,121,97,193,                         //vpunpcklwd    %xmm1,%xmm0,%xmm8
  197,249,105,193,                        //vpunpckhwd    %xmm1,%xmm0,%xmm0
  197,233,97,203,                         //vpunpcklwd    %xmm3,%xmm2,%xmm1
  197,105,105,203,                        //vpunpckhwd    %xmm3,%xmm2,%xmm9
  197,249,110,90,100,                     //vmovd         0x64(%rdx),%xmm3
  197,249,112,219,0,                      //vpshufd       $0x0,%xmm3,%xmm3
  196,193,97,101,208,                     //vpcmpgtw      %xmm8,%xmm3,%xmm2
  196,65,105,223,192,                     //vpandn        %xmm8,%xmm2,%xmm8
  197,225,101,208,                        //vpcmpgtw      %xmm0,%xmm3,%xmm2
  197,233,223,192,                        //vpandn        %xmm0,%xmm2,%xmm0
  197,225,101,209,                        //vpcmpgtw      %xmm1,%xmm3,%xmm2
  197,233,223,201,                        //vpandn        %xmm1,%xmm2,%xmm1
  196,193,97,101,209,                     //vpcmpgtw      %xmm9,%xmm3,%xmm2
  196,193,105,223,209,                    //vpandn        %xmm9,%xmm2,%xmm2
  196,66,121,51,208,                      //vpmovzxwd     %xmm8,%xmm10
  196,98,121,51,201,                      //vpmovzxwd     %xmm1,%xmm9
  197,225,239,219,                        //vpxor         %xmm3,%xmm3,%xmm3
  197,57,105,195,                         //vpunpckhwd    %xmm3,%xmm8,%xmm8
  197,241,105,203,                        //vpunpckhwd    %xmm3,%xmm1,%xmm1
  196,98,121,51,216,                      //vpmovzxwd     %xmm0,%xmm11
  196,98,121,51,226,                      //vpmovzxwd     %xmm2,%xmm12
  197,121,105,235,                        //vpunpckhwd    %xmm3,%xmm0,%xmm13
  197,105,105,243,                        //vpunpckhwd    %xmm3,%xmm2,%xmm14
  196,193,121,114,242,13,                 //vpslld        $0xd,%xmm10,%xmm0
  196,193,105,114,241,13,                 //vpslld        $0xd,%xmm9,%xmm2
  196,227,125,24,194,1,                   //vinsertf128   $0x1,%xmm2,%ymm0,%ymm0
  196,98,125,24,74,92,                    //vbroadcastss  0x5c(%rdx),%ymm9
  197,180,89,192,                         //vmulps        %ymm0,%ymm9,%ymm0
  196,193,105,114,240,13,                 //vpslld        $0xd,%xmm8,%xmm2
  197,241,114,241,13,                     //vpslld        $0xd,%xmm1,%xmm1
  196,227,109,24,201,1,                   //vinsertf128   $0x1,%xmm1,%ymm2,%ymm1
  197,180,89,201,                         //vmulps        %ymm1,%ymm9,%ymm1
  196,193,105,114,243,13,                 //vpslld        $0xd,%xmm11,%xmm2
  196,193,97,114,244,13,                  //vpslld        $0xd,%xmm12,%xmm3
  196,227,109,24,211,1,                   //vinsertf128   $0x1,%xmm3,%ymm2,%ymm2
  197,180,89,210,                         //vmulps        %ymm2,%ymm9,%ymm2
  196,193,57,114,245,13,                  //vpslld        $0xd,%xmm13,%xmm8
  196,193,97,114,246,13,                  //vpslld        $0xd,%xmm14,%xmm3
  196,227,61,24,219,1,                    //vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  197,180,89,219,                         //vmulps        %ymm3,%ymm9,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  197,251,16,12,248,                      //vmovsd        (%rax,%rdi,8),%xmm1
  196,65,57,87,192,                       //vxorpd        %xmm8,%xmm8,%xmm8
  72,131,249,1,                           //cmp           $0x1,%rcx
  117,6,                                  //jne           1030 <_sk_load_f16_avx+0x114>
  197,250,126,201,                        //vmovq         %xmm1,%xmm1
  235,30,                                 //jmp           104e <_sk_load_f16_avx+0x132>
  197,241,22,76,248,8,                    //vmovhpd       0x8(%rax,%rdi,8),%xmm1,%xmm1
  72,131,249,3,                           //cmp           $0x3,%rcx
  114,18,                                 //jb            104e <_sk_load_f16_avx+0x132>
  197,251,16,84,248,16,                   //vmovsd        0x10(%rax,%rdi,8),%xmm2
  72,131,249,3,                           //cmp           $0x3,%rcx
  117,19,                                 //jne           105b <_sk_load_f16_avx+0x13f>
  197,250,126,210,                        //vmovq         %xmm2,%xmm2
  235,46,                                 //jmp           107c <_sk_load_f16_avx+0x160>
  197,225,87,219,                         //vxorpd        %xmm3,%xmm3,%xmm3
  197,233,87,210,                         //vxorpd        %xmm2,%xmm2,%xmm2
  233,230,254,255,255,                    //jmpq          f41 <_sk_load_f16_avx+0x25>
  197,233,22,84,248,24,                   //vmovhpd       0x18(%rax,%rdi,8),%xmm2,%xmm2
  72,131,249,5,                           //cmp           $0x5,%rcx
  114,21,                                 //jb            107c <_sk_load_f16_avx+0x160>
  197,251,16,92,248,32,                   //vmovsd        0x20(%rax,%rdi,8),%xmm3
  72,131,249,5,                           //cmp           $0x5,%rcx
  117,18,                                 //jne           1085 <_sk_load_f16_avx+0x169>
  197,250,126,219,                        //vmovq         %xmm3,%xmm3
  233,197,254,255,255,                    //jmpq          f41 <_sk_load_f16_avx+0x25>
  197,225,87,219,                         //vxorpd        %xmm3,%xmm3,%xmm3
  233,188,254,255,255,                    //jmpq          f41 <_sk_load_f16_avx+0x25>
  197,225,22,92,248,40,                   //vmovhpd       0x28(%rax,%rdi,8),%xmm3,%xmm3
  72,131,249,7,                           //cmp           $0x7,%rcx
  15,130,172,254,255,255,                 //jb            f41 <_sk_load_f16_avx+0x25>
  197,123,16,68,248,48,                   //vmovsd        0x30(%rax,%rdi,8),%xmm8
  233,161,254,255,255,                    //jmpq          f41 <_sk_load_f16_avx+0x25>
};

CODE const uint8_t sk_store_f16_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  196,98,125,24,66,96,                    //vbroadcastss  0x60(%rdx),%ymm8
  197,60,89,200,                          //vmulps        %ymm0,%ymm8,%ymm9
  196,67,125,25,202,1,                    //vextractf128  $0x1,%ymm9,%xmm10
  196,193,41,114,210,13,                  //vpsrld        $0xd,%xmm10,%xmm10
  196,193,49,114,209,13,                  //vpsrld        $0xd,%xmm9,%xmm9
  197,60,89,217,                          //vmulps        %ymm1,%ymm8,%ymm11
  196,67,125,25,220,1,                    //vextractf128  $0x1,%ymm11,%xmm12
  196,193,25,114,212,13,                  //vpsrld        $0xd,%xmm12,%xmm12
  196,193,33,114,211,13,                  //vpsrld        $0xd,%xmm11,%xmm11
  197,60,89,234,                          //vmulps        %ymm2,%ymm8,%ymm13
  196,67,125,25,238,1,                    //vextractf128  $0x1,%ymm13,%xmm14
  196,193,9,114,214,13,                   //vpsrld        $0xd,%xmm14,%xmm14
  196,193,17,114,213,13,                  //vpsrld        $0xd,%xmm13,%xmm13
  197,60,89,195,                          //vmulps        %ymm3,%ymm8,%ymm8
  196,67,125,25,199,1,                    //vextractf128  $0x1,%ymm8,%xmm15
  196,193,1,114,215,13,                   //vpsrld        $0xd,%xmm15,%xmm15
  196,193,57,114,208,13,                  //vpsrld        $0xd,%xmm8,%xmm8
  196,193,33,115,251,2,                   //vpslldq       $0x2,%xmm11,%xmm11
  196,65,33,235,201,                      //vpor          %xmm9,%xmm11,%xmm9
  196,193,33,115,252,2,                   //vpslldq       $0x2,%xmm12,%xmm11
  196,65,33,235,226,                      //vpor          %xmm10,%xmm11,%xmm12
  196,193,57,115,248,2,                   //vpslldq       $0x2,%xmm8,%xmm8
  196,65,57,235,197,                      //vpor          %xmm13,%xmm8,%xmm8
  196,193,41,115,255,2,                   //vpslldq       $0x2,%xmm15,%xmm10
  196,65,41,235,238,                      //vpor          %xmm14,%xmm10,%xmm13
  196,65,49,98,216,                       //vpunpckldq    %xmm8,%xmm9,%xmm11
  196,65,49,106,208,                      //vpunpckhdq    %xmm8,%xmm9,%xmm10
  196,65,25,98,205,                       //vpunpckldq    %xmm13,%xmm12,%xmm9
  196,65,25,106,197,                      //vpunpckhdq    %xmm13,%xmm12,%xmm8
  72,133,201,                             //test          %rcx,%rcx
  117,27,                                 //jne           1163 <_sk_store_f16_avx+0xc3>
  197,120,17,28,248,                      //vmovups       %xmm11,(%rax,%rdi,8)
  197,120,17,84,248,16,                   //vmovups       %xmm10,0x10(%rax,%rdi,8)
  197,120,17,76,248,32,                   //vmovups       %xmm9,0x20(%rax,%rdi,8)
  197,122,127,68,248,48,                  //vmovdqu       %xmm8,0x30(%rax,%rdi,8)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  197,121,214,28,248,                     //vmovq         %xmm11,(%rax,%rdi,8)
  72,131,249,1,                           //cmp           $0x1,%rcx
  116,241,                                //je            115f <_sk_store_f16_avx+0xbf>
  197,121,23,92,248,8,                    //vmovhpd       %xmm11,0x8(%rax,%rdi,8)
  72,131,249,3,                           //cmp           $0x3,%rcx
  114,229,                                //jb            115f <_sk_store_f16_avx+0xbf>
  197,121,214,84,248,16,                  //vmovq         %xmm10,0x10(%rax,%rdi,8)
  116,221,                                //je            115f <_sk_store_f16_avx+0xbf>
  197,121,23,84,248,24,                   //vmovhpd       %xmm10,0x18(%rax,%rdi,8)
  72,131,249,5,                           //cmp           $0x5,%rcx
  114,209,                                //jb            115f <_sk_store_f16_avx+0xbf>
  197,121,214,76,248,32,                  //vmovq         %xmm9,0x20(%rax,%rdi,8)
  116,201,                                //je            115f <_sk_store_f16_avx+0xbf>
  197,121,23,76,248,40,                   //vmovhpd       %xmm9,0x28(%rax,%rdi,8)
  72,131,249,7,                           //cmp           $0x7,%rcx
  114,189,                                //jb            115f <_sk_store_f16_avx+0xbf>
  197,121,214,68,248,48,                  //vmovq         %xmm8,0x30(%rax,%rdi,8)
  235,181,                                //jmp           115f <_sk_store_f16_avx+0xbf>
};

CODE const uint8_t sk_store_f32_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,0,                               //mov           (%rax),%r8
  72,141,4,189,0,0,0,0,                   //lea           0x0(,%rdi,4),%rax
  197,124,20,193,                         //vunpcklps     %ymm1,%ymm0,%ymm8
  197,124,21,217,                         //vunpckhps     %ymm1,%ymm0,%ymm11
  197,108,20,203,                         //vunpcklps     %ymm3,%ymm2,%ymm9
  197,108,21,227,                         //vunpckhps     %ymm3,%ymm2,%ymm12
  196,65,61,20,209,                       //vunpcklpd     %ymm9,%ymm8,%ymm10
  196,65,61,21,201,                       //vunpckhpd     %ymm9,%ymm8,%ymm9
  196,65,37,20,196,                       //vunpcklpd     %ymm12,%ymm11,%ymm8
  196,65,37,21,220,                       //vunpckhpd     %ymm12,%ymm11,%ymm11
  72,133,201,                             //test          %rcx,%rcx
  117,55,                                 //jne           1217 <_sk_store_f32_avx+0x6d>
  196,67,45,24,225,1,                     //vinsertf128   $0x1,%xmm9,%ymm10,%ymm12
  196,67,61,24,235,1,                     //vinsertf128   $0x1,%xmm11,%ymm8,%ymm13
  196,67,45,6,201,49,                     //vperm2f128    $0x31,%ymm9,%ymm10,%ymm9
  196,67,61,6,195,49,                     //vperm2f128    $0x31,%ymm11,%ymm8,%ymm8
  196,65,125,17,36,128,                   //vmovupd       %ymm12,(%r8,%rax,4)
  196,65,125,17,108,128,32,               //vmovupd       %ymm13,0x20(%r8,%rax,4)
  196,65,125,17,76,128,64,                //vmovupd       %ymm9,0x40(%r8,%rax,4)
  196,65,125,17,68,128,96,                //vmovupd       %ymm8,0x60(%r8,%rax,4)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  196,65,121,17,20,128,                   //vmovupd       %xmm10,(%r8,%rax,4)
  72,131,249,1,                           //cmp           $0x1,%rcx
  116,240,                                //je            1213 <_sk_store_f32_avx+0x69>
  196,65,121,17,76,128,16,                //vmovupd       %xmm9,0x10(%r8,%rax,4)
  72,131,249,3,                           //cmp           $0x3,%rcx
  114,227,                                //jb            1213 <_sk_store_f32_avx+0x69>
  196,65,121,17,68,128,32,                //vmovupd       %xmm8,0x20(%r8,%rax,4)
  116,218,                                //je            1213 <_sk_store_f32_avx+0x69>
  196,65,121,17,92,128,48,                //vmovupd       %xmm11,0x30(%r8,%rax,4)
  72,131,249,5,                           //cmp           $0x5,%rcx
  114,205,                                //jb            1213 <_sk_store_f32_avx+0x69>
  196,67,125,25,84,128,64,1,              //vextractf128  $0x1,%ymm10,0x40(%r8,%rax,4)
  116,195,                                //je            1213 <_sk_store_f32_avx+0x69>
  196,67,125,25,76,128,80,1,              //vextractf128  $0x1,%ymm9,0x50(%r8,%rax,4)
  72,131,249,7,                           //cmp           $0x7,%rcx
  114,181,                                //jb            1213 <_sk_store_f32_avx+0x69>
  196,67,125,25,68,128,96,1,              //vextractf128  $0x1,%ymm8,0x60(%r8,%rax,4)
  235,171,                                //jmp           1213 <_sk_store_f32_avx+0x69>
};

CODE const uint8_t sk_clamp_x_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  197,60,95,200,                          //vmaxps        %ymm0,%ymm8,%ymm9
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,99,125,25,192,1,                    //vextractf128  $0x1,%ymm8,%xmm0
  196,65,41,118,210,                      //vpcmpeqd      %xmm10,%xmm10,%xmm10
  196,193,121,254,194,                    //vpaddd        %xmm10,%xmm0,%xmm0
  196,65,57,254,194,                      //vpaddd        %xmm10,%xmm8,%xmm8
  196,227,61,24,192,1,                    //vinsertf128   $0x1,%xmm0,%ymm8,%ymm0
  197,180,93,192,                         //vminps        %ymm0,%ymm9,%ymm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_y_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  197,60,95,201,                          //vmaxps        %ymm1,%ymm8,%ymm9
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,99,125,25,193,1,                    //vextractf128  $0x1,%ymm8,%xmm1
  196,65,41,118,210,                      //vpcmpeqd      %xmm10,%xmm10,%xmm10
  196,193,113,254,202,                    //vpaddd        %xmm10,%xmm1,%xmm1
  196,65,57,254,194,                      //vpaddd        %xmm10,%xmm8,%xmm8
  196,227,61,24,201,1,                    //vinsertf128   $0x1,%xmm1,%ymm8,%ymm1
  197,180,93,201,                         //vminps        %ymm1,%ymm9,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_repeat_x_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,65,124,94,200,                      //vdivps        %ymm8,%ymm0,%ymm9
  196,67,125,8,201,1,                     //vroundps      $0x1,%ymm9,%ymm9
  196,65,52,89,200,                       //vmulps        %ymm8,%ymm9,%ymm9
  196,65,124,92,201,                      //vsubps        %ymm9,%ymm0,%ymm9
  196,99,125,25,192,1,                    //vextractf128  $0x1,%ymm8,%xmm0
  196,65,41,118,210,                      //vpcmpeqd      %xmm10,%xmm10,%xmm10
  196,193,121,254,194,                    //vpaddd        %xmm10,%xmm0,%xmm0
  196,65,57,254,194,                      //vpaddd        %xmm10,%xmm8,%xmm8
  196,227,61,24,192,1,                    //vinsertf128   $0x1,%xmm0,%ymm8,%ymm0
  197,180,93,192,                         //vminps        %ymm0,%ymm9,%ymm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_repeat_y_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,65,116,94,200,                      //vdivps        %ymm8,%ymm1,%ymm9
  196,67,125,8,201,1,                     //vroundps      $0x1,%ymm9,%ymm9
  196,65,52,89,200,                       //vmulps        %ymm8,%ymm9,%ymm9
  196,65,116,92,201,                      //vsubps        %ymm9,%ymm1,%ymm9
  196,99,125,25,193,1,                    //vextractf128  $0x1,%ymm8,%xmm1
  196,65,41,118,210,                      //vpcmpeqd      %xmm10,%xmm10,%xmm10
  196,193,113,254,202,                    //vpaddd        %xmm10,%xmm1,%xmm1
  196,65,57,254,194,                      //vpaddd        %xmm10,%xmm8,%xmm8
  196,227,61,24,201,1,                    //vinsertf128   $0x1,%xmm1,%ymm8,%ymm1
  197,180,93,201,                         //vminps        %ymm1,%ymm9,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_mirror_x_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,122,16,0,                           //vmovss        (%rax),%xmm8
  196,65,121,112,200,0,                   //vpshufd       $0x0,%xmm8,%xmm9
  196,67,53,24,201,1,                     //vinsertf128   $0x1,%xmm9,%ymm9,%ymm9
  196,65,124,92,209,                      //vsubps        %ymm9,%ymm0,%ymm10
  196,193,58,88,192,                      //vaddss        %xmm8,%xmm8,%xmm0
  196,227,121,4,192,0,                    //vpermilps     $0x0,%xmm0,%xmm0
  196,227,125,24,192,1,                   //vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  197,44,94,192,                          //vdivps        %ymm0,%ymm10,%ymm8
  196,67,125,8,192,1,                     //vroundps      $0x1,%ymm8,%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,172,92,192,                         //vsubps        %ymm0,%ymm10,%ymm0
  196,193,124,92,193,                     //vsubps        %ymm9,%ymm0,%ymm0
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  197,60,92,192,                          //vsubps        %ymm0,%ymm8,%ymm8
  197,60,84,192,                          //vandps        %ymm0,%ymm8,%ymm8
  196,99,125,25,200,1,                    //vextractf128  $0x1,%ymm9,%xmm0
  196,65,41,118,210,                      //vpcmpeqd      %xmm10,%xmm10,%xmm10
  196,193,121,254,194,                    //vpaddd        %xmm10,%xmm0,%xmm0
  196,65,49,254,202,                      //vpaddd        %xmm10,%xmm9,%xmm9
  196,227,53,24,192,1,                    //vinsertf128   $0x1,%xmm0,%ymm9,%ymm0
  197,188,93,192,                         //vminps        %ymm0,%ymm8,%ymm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_mirror_y_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,122,16,0,                           //vmovss        (%rax),%xmm8
  196,65,121,112,200,0,                   //vpshufd       $0x0,%xmm8,%xmm9
  196,67,53,24,201,1,                     //vinsertf128   $0x1,%xmm9,%ymm9,%ymm9
  196,65,116,92,209,                      //vsubps        %ymm9,%ymm1,%ymm10
  196,193,58,88,200,                      //vaddss        %xmm8,%xmm8,%xmm1
  196,227,121,4,201,0,                    //vpermilps     $0x0,%xmm1,%xmm1
  196,227,117,24,201,1,                   //vinsertf128   $0x1,%xmm1,%ymm1,%ymm1
  197,44,94,193,                          //vdivps        %ymm1,%ymm10,%ymm8
  196,67,125,8,192,1,                     //vroundps      $0x1,%ymm8,%ymm8
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,172,92,201,                         //vsubps        %ymm1,%ymm10,%ymm1
  196,193,116,92,201,                     //vsubps        %ymm9,%ymm1,%ymm1
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  197,60,92,193,                          //vsubps        %ymm1,%ymm8,%ymm8
  197,60,84,193,                          //vandps        %ymm1,%ymm8,%ymm8
  196,99,125,25,201,1,                    //vextractf128  $0x1,%ymm9,%xmm1
  196,65,41,118,210,                      //vpcmpeqd      %xmm10,%xmm10,%xmm10
  196,193,113,254,202,                    //vpaddd        %xmm10,%xmm1,%xmm1
  196,65,49,254,202,                      //vpaddd        %xmm10,%xmm9,%xmm9
  196,227,53,24,201,1,                    //vinsertf128   $0x1,%xmm1,%ymm9,%ymm1
  197,188,93,201,                         //vminps        %ymm1,%ymm8,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_2x3_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,98,125,24,72,8,                     //vbroadcastss  0x8(%rax),%ymm9
  196,98,125,24,80,16,                    //vbroadcastss  0x10(%rax),%ymm10
  197,52,89,201,                          //vmulps        %ymm1,%ymm9,%ymm9
  196,65,52,88,202,                       //vaddps        %ymm10,%ymm9,%ymm9
  197,60,89,192,                          //vmulps        %ymm0,%ymm8,%ymm8
  196,65,60,88,193,                       //vaddps        %ymm9,%ymm8,%ymm8
  196,98,125,24,72,4,                     //vbroadcastss  0x4(%rax),%ymm9
  196,98,125,24,80,12,                    //vbroadcastss  0xc(%rax),%ymm10
  196,98,125,24,88,20,                    //vbroadcastss  0x14(%rax),%ymm11
  197,172,89,201,                         //vmulps        %ymm1,%ymm10,%ymm1
  196,193,116,88,203,                     //vaddps        %ymm11,%ymm1,%ymm1
  197,180,89,192,                         //vmulps        %ymm0,%ymm9,%ymm0
  197,252,88,201,                         //vaddps        %ymm1,%ymm0,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,124,41,192,                         //vmovaps       %ymm8,%ymm0
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_3x4_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,98,125,24,72,12,                    //vbroadcastss  0xc(%rax),%ymm9
  196,98,125,24,80,24,                    //vbroadcastss  0x18(%rax),%ymm10
  196,98,125,24,88,36,                    //vbroadcastss  0x24(%rax),%ymm11
  197,44,89,210,                          //vmulps        %ymm2,%ymm10,%ymm10
  196,65,44,88,211,                       //vaddps        %ymm11,%ymm10,%ymm10
  197,52,89,201,                          //vmulps        %ymm1,%ymm9,%ymm9
  196,65,52,88,202,                       //vaddps        %ymm10,%ymm9,%ymm9
  197,60,89,192,                          //vmulps        %ymm0,%ymm8,%ymm8
  196,65,60,88,193,                       //vaddps        %ymm9,%ymm8,%ymm8
  196,98,125,24,72,4,                     //vbroadcastss  0x4(%rax),%ymm9
  196,98,125,24,80,16,                    //vbroadcastss  0x10(%rax),%ymm10
  196,98,125,24,88,28,                    //vbroadcastss  0x1c(%rax),%ymm11
  196,98,125,24,96,40,                    //vbroadcastss  0x28(%rax),%ymm12
  197,36,89,218,                          //vmulps        %ymm2,%ymm11,%ymm11
  196,65,36,88,220,                       //vaddps        %ymm12,%ymm11,%ymm11
  197,44,89,209,                          //vmulps        %ymm1,%ymm10,%ymm10
  196,65,44,88,211,                       //vaddps        %ymm11,%ymm10,%ymm10
  197,52,89,200,                          //vmulps        %ymm0,%ymm9,%ymm9
  196,65,52,88,202,                       //vaddps        %ymm10,%ymm9,%ymm9
  196,98,125,24,80,8,                     //vbroadcastss  0x8(%rax),%ymm10
  196,98,125,24,88,20,                    //vbroadcastss  0x14(%rax),%ymm11
  196,98,125,24,96,32,                    //vbroadcastss  0x20(%rax),%ymm12
  196,98,125,24,104,44,                   //vbroadcastss  0x2c(%rax),%ymm13
  197,156,89,210,                         //vmulps        %ymm2,%ymm12,%ymm2
  196,193,108,88,213,                     //vaddps        %ymm13,%ymm2,%ymm2
  197,164,89,201,                         //vmulps        %ymm1,%ymm11,%ymm1
  197,244,88,202,                         //vaddps        %ymm2,%ymm1,%ymm1
  197,172,89,192,                         //vmulps        %ymm0,%ymm10,%ymm0
  197,252,88,209,                         //vaddps        %ymm1,%ymm0,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,124,41,192,                         //vmovaps       %ymm8,%ymm0
  197,124,41,201,                         //vmovaps       %ymm9,%ymm1
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_perspective_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,98,125,24,72,4,                     //vbroadcastss  0x4(%rax),%ymm9
  196,98,125,24,80,8,                     //vbroadcastss  0x8(%rax),%ymm10
  197,52,89,201,                          //vmulps        %ymm1,%ymm9,%ymm9
  196,65,52,88,202,                       //vaddps        %ymm10,%ymm9,%ymm9
  197,60,89,192,                          //vmulps        %ymm0,%ymm8,%ymm8
  196,65,60,88,193,                       //vaddps        %ymm9,%ymm8,%ymm8
  196,98,125,24,72,12,                    //vbroadcastss  0xc(%rax),%ymm9
  196,98,125,24,80,16,                    //vbroadcastss  0x10(%rax),%ymm10
  196,98,125,24,88,20,                    //vbroadcastss  0x14(%rax),%ymm11
  197,44,89,209,                          //vmulps        %ymm1,%ymm10,%ymm10
  196,65,44,88,211,                       //vaddps        %ymm11,%ymm10,%ymm10
  197,52,89,200,                          //vmulps        %ymm0,%ymm9,%ymm9
  196,65,52,88,202,                       //vaddps        %ymm10,%ymm9,%ymm9
  196,98,125,24,80,24,                    //vbroadcastss  0x18(%rax),%ymm10
  196,98,125,24,88,28,                    //vbroadcastss  0x1c(%rax),%ymm11
  196,98,125,24,96,32,                    //vbroadcastss  0x20(%rax),%ymm12
  197,164,89,201,                         //vmulps        %ymm1,%ymm11,%ymm1
  196,193,116,88,204,                     //vaddps        %ymm12,%ymm1,%ymm1
  197,172,89,192,                         //vmulps        %ymm0,%ymm10,%ymm0
  197,252,88,193,                         //vaddps        %ymm1,%ymm0,%ymm0
  197,252,83,200,                         //vrcpps        %ymm0,%ymm1
  197,188,89,193,                         //vmulps        %ymm1,%ymm8,%ymm0
  197,180,89,201,                         //vmulps        %ymm1,%ymm9,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_linear_gradient_2stops_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,226,125,24,72,16,                   //vbroadcastss  0x10(%rax),%ymm1
  196,226,125,24,16,                      //vbroadcastss  (%rax),%ymm2
  197,244,89,200,                         //vmulps        %ymm0,%ymm1,%ymm1
  197,108,88,193,                         //vaddps        %ymm1,%ymm2,%ymm8
  196,226,125,24,72,20,                   //vbroadcastss  0x14(%rax),%ymm1
  196,226,125,24,80,4,                    //vbroadcastss  0x4(%rax),%ymm2
  197,244,89,200,                         //vmulps        %ymm0,%ymm1,%ymm1
  197,236,88,201,                         //vaddps        %ymm1,%ymm2,%ymm1
  196,226,125,24,80,24,                   //vbroadcastss  0x18(%rax),%ymm2
  196,226,125,24,88,8,                    //vbroadcastss  0x8(%rax),%ymm3
  197,236,89,208,                         //vmulps        %ymm0,%ymm2,%ymm2
  197,228,88,210,                         //vaddps        %ymm2,%ymm3,%ymm2
  196,226,125,24,88,28,                   //vbroadcastss  0x1c(%rax),%ymm3
  196,98,125,24,72,12,                    //vbroadcastss  0xc(%rax),%ymm9
  197,228,89,192,                         //vmulps        %ymm0,%ymm3,%ymm0
  197,180,88,216,                         //vaddps        %ymm0,%ymm9,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,124,41,192,                         //vmovaps       %ymm8,%ymm0
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_start_pipeline_sse41[] = {
  65,87,                                  //push          %r15
  65,86,                                  //push          %r14
  65,85,                                  //push          %r13
  65,84,                                  //push          %r12
  83,                                     //push          %rbx
  73,137,207,                             //mov           %rcx,%r15
  73,137,214,                             //mov           %rdx,%r14
  72,137,251,                             //mov           %rdi,%rbx
  72,173,                                 //lods          %ds:(%rsi),%rax
  73,137,196,                             //mov           %rax,%r12
  73,137,245,                             //mov           %rsi,%r13
  72,141,67,4,                            //lea           0x4(%rbx),%rax
  76,57,248,                              //cmp           %r15,%rax
  118,5,                                  //jbe           28 <_sk_start_pipeline_sse41+0x28>
  72,137,216,                             //mov           %rbx,%rax
  235,52,                                 //jmp           5c <_sk_start_pipeline_sse41+0x5c>
  15,87,192,                              //xorps         %xmm0,%xmm0
  15,87,201,                              //xorps         %xmm1,%xmm1
  15,87,210,                              //xorps         %xmm2,%xmm2
  15,87,219,                              //xorps         %xmm3,%xmm3
  15,87,228,                              //xorps         %xmm4,%xmm4
  15,87,237,                              //xorps         %xmm5,%xmm5
  15,87,246,                              //xorps         %xmm6,%xmm6
  15,87,255,                              //xorps         %xmm7,%xmm7
  72,137,223,                             //mov           %rbx,%rdi
  76,137,238,                             //mov           %r13,%rsi
  76,137,242,                             //mov           %r14,%rdx
  65,255,212,                             //callq         *%r12
  72,141,67,4,                            //lea           0x4(%rbx),%rax
  72,131,195,8,                           //add           $0x8,%rbx
  76,57,251,                              //cmp           %r15,%rbx
  72,137,195,                             //mov           %rax,%rbx
  118,204,                                //jbe           28 <_sk_start_pipeline_sse41+0x28>
  91,                                     //pop           %rbx
  65,92,                                  //pop           %r12
  65,93,                                  //pop           %r13
  65,94,                                  //pop           %r14
  65,95,                                  //pop           %r15
  195,                                    //retq
};

CODE const uint8_t sk_just_return_sse41[] = {
  195,                                    //retq
};

CODE const uint8_t sk_seed_shader_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  102,15,110,199,                         //movd          %edi,%xmm0
  102,15,112,192,0,                       //pshufd        $0x0,%xmm0,%xmm0
  15,91,200,                              //cvtdq2ps      %xmm0,%xmm1
  243,15,16,18,                           //movss         (%rdx),%xmm2
  243,15,16,90,4,                         //movss         0x4(%rdx),%xmm3
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  15,88,203,                              //addps         %xmm3,%xmm1
  15,16,66,20,                            //movups        0x14(%rdx),%xmm0
  15,88,193,                              //addps         %xmm1,%xmm0
  102,15,110,8,                           //movd          (%rax),%xmm1
  102,15,112,201,0,                       //pshufd        $0x0,%xmm1,%xmm1
  15,91,201,                              //cvtdq2ps      %xmm1,%xmm1
  15,88,203,                              //addps         %xmm3,%xmm1
  15,198,210,0,                           //shufps        $0x0,%xmm2,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,87,219,                              //xorps         %xmm3,%xmm3
  15,87,228,                              //xorps         %xmm4,%xmm4
  15,87,237,                              //xorps         %xmm5,%xmm5
  15,87,246,                              //xorps         %xmm6,%xmm6
  15,87,255,                              //xorps         %xmm7,%xmm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_constant_color_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,16,24,                               //movups        (%rax),%xmm3
  15,40,195,                              //movaps        %xmm3,%xmm0
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  15,40,203,                              //movaps        %xmm3,%xmm1
  15,198,201,85,                          //shufps        $0x55,%xmm1,%xmm1
  15,40,211,                              //movaps        %xmm3,%xmm2
  15,198,210,170,                         //shufps        $0xaa,%xmm2,%xmm2
  15,198,219,255,                         //shufps        $0xff,%xmm3,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clear_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,87,192,                              //xorps         %xmm0,%xmm0
  15,87,201,                              //xorps         %xmm1,%xmm1
  15,87,210,                              //xorps         %xmm2,%xmm2
  15,87,219,                              //xorps         %xmm3,%xmm3
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_plus__sse41[] = {
  15,88,196,                              //addps         %xmm4,%xmm0
  15,88,205,                              //addps         %xmm5,%xmm1
  15,88,214,                              //addps         %xmm6,%xmm2
  15,88,223,                              //addps         %xmm7,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_srcover_sse41[] = {
  243,68,15,16,2,                         //movss         (%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,92,195,                           //subps         %xmm3,%xmm8
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,204,                           //mulps         %xmm4,%xmm9
  65,15,88,193,                           //addps         %xmm9,%xmm0
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,205,                           //mulps         %xmm5,%xmm9
  65,15,88,201,                           //addps         %xmm9,%xmm1
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,206,                           //mulps         %xmm6,%xmm9
  65,15,88,209,                           //addps         %xmm9,%xmm2
  68,15,89,199,                           //mulps         %xmm7,%xmm8
  65,15,88,216,                           //addps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_dstover_sse41[] = {
  243,68,15,16,2,                         //movss         (%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,92,199,                           //subps         %xmm7,%xmm8
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  15,88,196,                              //addps         %xmm4,%xmm0
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  15,88,205,                              //addps         %xmm5,%xmm1
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  15,88,214,                              //addps         %xmm6,%xmm2
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  15,88,223,                              //addps         %xmm7,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_0_sse41[] = {
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  65,15,95,192,                           //maxps         %xmm8,%xmm0
  65,15,95,200,                           //maxps         %xmm8,%xmm1
  65,15,95,208,                           //maxps         %xmm8,%xmm2
  65,15,95,216,                           //maxps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_1_sse41[] = {
  243,68,15,16,2,                         //movss         (%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,93,192,                           //minps         %xmm8,%xmm0
  65,15,93,200,                           //minps         %xmm8,%xmm1
  65,15,93,208,                           //minps         %xmm8,%xmm2
  65,15,93,216,                           //minps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_a_sse41[] = {
  243,68,15,16,2,                         //movss         (%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,93,216,                           //minps         %xmm8,%xmm3
  15,93,195,                              //minps         %xmm3,%xmm0
  15,93,203,                              //minps         %xmm3,%xmm1
  15,93,211,                              //minps         %xmm3,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_set_rgb_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,15,16,0,                            //movss         (%rax),%xmm0
  243,15,16,72,4,                         //movss         0x4(%rax),%xmm1
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  243,15,16,80,8,                         //movss         0x8(%rax),%xmm2
  15,198,210,0,                           //shufps        $0x0,%xmm2,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_swap_rb_sse41[] = {
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,194,                              //movaps        %xmm2,%xmm0
  65,15,40,208,                           //movaps        %xmm8,%xmm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_swap_sse41[] = {
  68,15,40,195,                           //movaps        %xmm3,%xmm8
  68,15,40,202,                           //movaps        %xmm2,%xmm9
  68,15,40,209,                           //movaps        %xmm1,%xmm10
  68,15,40,216,                           //movaps        %xmm0,%xmm11
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,196,                              //movaps        %xmm4,%xmm0
  15,40,205,                              //movaps        %xmm5,%xmm1
  15,40,214,                              //movaps        %xmm6,%xmm2
  15,40,223,                              //movaps        %xmm7,%xmm3
  65,15,40,227,                           //movaps        %xmm11,%xmm4
  65,15,40,234,                           //movaps        %xmm10,%xmm5
  65,15,40,241,                           //movaps        %xmm9,%xmm6
  65,15,40,248,                           //movaps        %xmm8,%xmm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_move_src_dst_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,224,                              //movaps        %xmm0,%xmm4
  15,40,233,                              //movaps        %xmm1,%xmm5
  15,40,242,                              //movaps        %xmm2,%xmm6
  15,40,251,                              //movaps        %xmm3,%xmm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_move_dst_src_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,196,                              //movaps        %xmm4,%xmm0
  15,40,205,                              //movaps        %xmm5,%xmm1
  15,40,214,                              //movaps        %xmm6,%xmm2
  15,40,223,                              //movaps        %xmm7,%xmm3
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_premul_sse41[] = {
  15,89,195,                              //mulps         %xmm3,%xmm0
  15,89,203,                              //mulps         %xmm3,%xmm1
  15,89,211,                              //mulps         %xmm3,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_unpremul_sse41[] = {
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  69,15,87,201,                           //xorps         %xmm9,%xmm9
  243,68,15,16,18,                        //movss         (%rdx),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  68,15,94,211,                           //divps         %xmm3,%xmm10
  15,40,195,                              //movaps        %xmm3,%xmm0
  65,15,194,193,0,                        //cmpeqps       %xmm9,%xmm0
  102,69,15,56,20,209,                    //blendvps      %xmm0,%xmm9,%xmm10
  69,15,89,194,                           //mulps         %xmm10,%xmm8
  65,15,89,202,                           //mulps         %xmm10,%xmm1
  65,15,89,210,                           //mulps         %xmm10,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,192,                           //movaps        %xmm8,%xmm0
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_from_srgb_sse41[] = {
  68,15,40,194,                           //movaps        %xmm2,%xmm8
  243,68,15,16,90,64,                     //movss         0x40(%rdx),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,40,211,                           //movaps        %xmm11,%xmm10
  68,15,89,208,                           //mulps         %xmm0,%xmm10
  68,15,40,240,                           //movaps        %xmm0,%xmm14
  69,15,89,246,                           //mulps         %xmm14,%xmm14
  243,15,16,82,60,                        //movss         0x3c(%rdx),%xmm2
  15,198,210,0,                           //shufps        $0x0,%xmm2,%xmm2
  243,68,15,16,98,52,                     //movss         0x34(%rdx),%xmm12
  243,68,15,16,106,56,                    //movss         0x38(%rdx),%xmm13
  69,15,198,237,0,                        //shufps        $0x0,%xmm13,%xmm13
  68,15,40,202,                           //movaps        %xmm2,%xmm9
  68,15,89,200,                           //mulps         %xmm0,%xmm9
  69,15,88,205,                           //addps         %xmm13,%xmm9
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  69,15,89,206,                           //mulps         %xmm14,%xmm9
  69,15,88,204,                           //addps         %xmm12,%xmm9
  243,68,15,16,114,68,                    //movss         0x44(%rdx),%xmm14
  69,15,198,246,0,                        //shufps        $0x0,%xmm14,%xmm14
  65,15,194,198,1,                        //cmpltps       %xmm14,%xmm0
  102,69,15,56,20,202,                    //blendvps      %xmm0,%xmm10,%xmm9
  69,15,40,251,                           //movaps        %xmm11,%xmm15
  68,15,89,249,                           //mulps         %xmm1,%xmm15
  15,40,193,                              //movaps        %xmm1,%xmm0
  15,89,192,                              //mulps         %xmm0,%xmm0
  68,15,40,210,                           //movaps        %xmm2,%xmm10
  68,15,89,209,                           //mulps         %xmm1,%xmm10
  69,15,88,213,                           //addps         %xmm13,%xmm10
  68,15,89,208,                           //mulps         %xmm0,%xmm10
  69,15,88,212,                           //addps         %xmm12,%xmm10
  65,15,194,206,1,                        //cmpltps       %xmm14,%xmm1
  15,40,193,                              //movaps        %xmm1,%xmm0
  102,69,15,56,20,215,                    //blendvps      %xmm0,%xmm15,%xmm10
  69,15,89,216,                           //mulps         %xmm8,%xmm11
  65,15,40,192,                           //movaps        %xmm8,%xmm0
  15,89,192,                              //mulps         %xmm0,%xmm0
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  65,15,88,213,                           //addps         %xmm13,%xmm2
  15,89,208,                              //mulps         %xmm0,%xmm2
  65,15,88,212,                           //addps         %xmm12,%xmm2
  69,15,194,198,1,                        //cmpltps       %xmm14,%xmm8
  65,15,40,192,                           //movaps        %xmm8,%xmm0
  102,65,15,56,20,211,                    //blendvps      %xmm0,%xmm11,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,193,                           //movaps        %xmm9,%xmm0
  65,15,40,202,                           //movaps        %xmm10,%xmm1
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_to_srgb_sse41[] = {
  72,131,236,24,                          //sub           $0x18,%rsp
  15,41,60,36,                            //movaps        %xmm7,(%rsp)
  15,40,254,                              //movaps        %xmm6,%xmm7
  15,40,245,                              //movaps        %xmm5,%xmm6
  15,40,236,                              //movaps        %xmm4,%xmm5
  15,40,227,                              //movaps        %xmm3,%xmm4
  68,15,40,194,                           //movaps        %xmm2,%xmm8
  15,40,217,                              //movaps        %xmm1,%xmm3
  15,82,208,                              //rsqrtps       %xmm0,%xmm2
  68,15,83,202,                           //rcpps         %xmm2,%xmm9
  68,15,82,210,                           //rsqrtps       %xmm2,%xmm10
  243,15,16,18,                           //movss         (%rdx),%xmm2
  243,68,15,16,90,72,                     //movss         0x48(%rdx),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  65,15,40,203,                           //movaps        %xmm11,%xmm1
  15,89,200,                              //mulps         %xmm0,%xmm1
  15,198,210,0,                           //shufps        $0x0,%xmm2,%xmm2
  243,68,15,16,98,76,                     //movss         0x4c(%rdx),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  243,68,15,16,106,80,                    //movss         0x50(%rdx),%xmm13
  69,15,198,237,0,                        //shufps        $0x0,%xmm13,%xmm13
  243,68,15,16,114,84,                    //movss         0x54(%rdx),%xmm14
  69,15,198,246,0,                        //shufps        $0x0,%xmm14,%xmm14
  69,15,89,205,                           //mulps         %xmm13,%xmm9
  69,15,88,206,                           //addps         %xmm14,%xmm9
  69,15,89,212,                           //mulps         %xmm12,%xmm10
  69,15,88,209,                           //addps         %xmm9,%xmm10
  68,15,40,202,                           //movaps        %xmm2,%xmm9
  69,15,93,202,                           //minps         %xmm10,%xmm9
  243,68,15,16,122,88,                    //movss         0x58(%rdx),%xmm15
  69,15,198,255,0,                        //shufps        $0x0,%xmm15,%xmm15
  65,15,194,199,1,                        //cmpltps       %xmm15,%xmm0
  102,68,15,56,20,201,                    //blendvps      %xmm0,%xmm1,%xmm9
  15,82,195,                              //rsqrtps       %xmm3,%xmm0
  15,83,200,                              //rcpps         %xmm0,%xmm1
  15,82,192,                              //rsqrtps       %xmm0,%xmm0
  65,15,89,205,                           //mulps         %xmm13,%xmm1
  65,15,88,206,                           //addps         %xmm14,%xmm1
  65,15,89,196,                           //mulps         %xmm12,%xmm0
  15,88,193,                              //addps         %xmm1,%xmm0
  68,15,40,210,                           //movaps        %xmm2,%xmm10
  68,15,93,208,                           //minps         %xmm0,%xmm10
  65,15,40,203,                           //movaps        %xmm11,%xmm1
  15,89,203,                              //mulps         %xmm3,%xmm1
  65,15,194,223,1,                        //cmpltps       %xmm15,%xmm3
  15,40,195,                              //movaps        %xmm3,%xmm0
  102,68,15,56,20,209,                    //blendvps      %xmm0,%xmm1,%xmm10
  65,15,82,192,                           //rsqrtps       %xmm8,%xmm0
  15,83,200,                              //rcpps         %xmm0,%xmm1
  65,15,89,205,                           //mulps         %xmm13,%xmm1
  65,15,88,206,                           //addps         %xmm14,%xmm1
  15,82,192,                              //rsqrtps       %xmm0,%xmm0
  65,15,89,196,                           //mulps         %xmm12,%xmm0
  15,88,193,                              //addps         %xmm1,%xmm0
  15,93,208,                              //minps         %xmm0,%xmm2
  69,15,89,216,                           //mulps         %xmm8,%xmm11
  69,15,194,199,1,                        //cmpltps       %xmm15,%xmm8
  65,15,40,192,                           //movaps        %xmm8,%xmm0
  102,65,15,56,20,211,                    //blendvps      %xmm0,%xmm11,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,193,                           //movaps        %xmm9,%xmm0
  65,15,40,202,                           //movaps        %xmm10,%xmm1
  15,40,220,                              //movaps        %xmm4,%xmm3
  15,40,229,                              //movaps        %xmm5,%xmm4
  15,40,238,                              //movaps        %xmm6,%xmm5
  15,40,247,                              //movaps        %xmm7,%xmm6
  15,40,60,36,                            //movaps        (%rsp),%xmm7
  72,131,196,24,                          //add           $0x18,%rsp
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_scale_1_float_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_scale_u8_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,68,15,56,49,4,56,                   //pmovzxbd      (%rax,%rdi,1),%xmm8
  69,15,91,192,                           //cvtdq2ps      %xmm8,%xmm8
  243,68,15,16,74,12,                     //movss         0xc(%rdx),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  65,15,89,193,                           //mulps         %xmm9,%xmm0
  65,15,89,201,                           //mulps         %xmm9,%xmm1
  65,15,89,209,                           //mulps         %xmm9,%xmm2
  65,15,89,217,                           //mulps         %xmm9,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_lerp_1_float_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  15,92,196,                              //subps         %xmm4,%xmm0
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  15,88,196,                              //addps         %xmm4,%xmm0
  15,92,205,                              //subps         %xmm5,%xmm1
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  15,88,205,                              //addps         %xmm5,%xmm1
  15,92,214,                              //subps         %xmm6,%xmm2
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  15,88,214,                              //addps         %xmm6,%xmm2
  15,92,223,                              //subps         %xmm7,%xmm3
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  15,88,223,                              //addps         %xmm7,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_lerp_u8_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,68,15,56,49,4,56,                   //pmovzxbd      (%rax,%rdi,1),%xmm8
  69,15,91,192,                           //cvtdq2ps      %xmm8,%xmm8
  243,68,15,16,74,12,                     //movss         0xc(%rdx),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  15,92,196,                              //subps         %xmm4,%xmm0
  65,15,89,193,                           //mulps         %xmm9,%xmm0
  15,88,196,                              //addps         %xmm4,%xmm0
  15,92,205,                              //subps         %xmm5,%xmm1
  65,15,89,201,                           //mulps         %xmm9,%xmm1
  15,88,205,                              //addps         %xmm5,%xmm1
  15,92,214,                              //subps         %xmm6,%xmm2
  65,15,89,209,                           //mulps         %xmm9,%xmm2
  15,88,214,                              //addps         %xmm6,%xmm2
  15,92,223,                              //subps         %xmm7,%xmm3
  65,15,89,217,                           //mulps         %xmm9,%xmm3
  15,88,223,                              //addps         %xmm7,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_lerp_565_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,68,15,56,51,4,120,                  //pmovzxwd      (%rax,%rdi,2),%xmm8
  102,15,110,90,104,                      //movd          0x68(%rdx),%xmm3
  102,15,112,219,0,                       //pshufd        $0x0,%xmm3,%xmm3
  102,65,15,219,216,                      //pand          %xmm8,%xmm3
  68,15,91,203,                           //cvtdq2ps      %xmm3,%xmm9
  243,15,16,26,                           //movss         (%rdx),%xmm3
  243,68,15,16,82,116,                    //movss         0x74(%rdx),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  102,68,15,110,74,108,                   //movd          0x6c(%rdx),%xmm9
  102,69,15,112,201,0,                    //pshufd        $0x0,%xmm9,%xmm9
  102,69,15,219,200,                      //pand          %xmm8,%xmm9
  69,15,91,201,                           //cvtdq2ps      %xmm9,%xmm9
  243,68,15,16,90,120,                    //movss         0x78(%rdx),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,89,217,                           //mulps         %xmm9,%xmm11
  102,68,15,110,74,112,                   //movd          0x70(%rdx),%xmm9
  102,69,15,112,201,0,                    //pshufd        $0x0,%xmm9,%xmm9
  102,69,15,219,200,                      //pand          %xmm8,%xmm9
  69,15,91,193,                           //cvtdq2ps      %xmm9,%xmm8
  243,68,15,16,74,124,                    //movss         0x7c(%rdx),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  15,92,196,                              //subps         %xmm4,%xmm0
  65,15,89,194,                           //mulps         %xmm10,%xmm0
  15,88,196,                              //addps         %xmm4,%xmm0
  15,92,205,                              //subps         %xmm5,%xmm1
  65,15,89,203,                           //mulps         %xmm11,%xmm1
  15,88,205,                              //addps         %xmm5,%xmm1
  15,92,214,                              //subps         %xmm6,%xmm2
  65,15,89,209,                           //mulps         %xmm9,%xmm2
  15,88,214,                              //addps         %xmm6,%xmm2
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_tables_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,8,                               //mov           (%rax),%rcx
  76,139,64,8,                            //mov           0x8(%rax),%r8
  243,68,15,111,4,185,                    //movdqu        (%rcx,%rdi,4),%xmm8
  102,15,110,66,16,                       //movd          0x10(%rdx),%xmm0
  102,15,112,192,0,                       //pshufd        $0x0,%xmm0,%xmm0
  102,65,15,111,200,                      //movdqa        %xmm8,%xmm1
  102,15,114,209,8,                       //psrld         $0x8,%xmm1
  102,15,219,200,                         //pand          %xmm0,%xmm1
  102,65,15,111,208,                      //movdqa        %xmm8,%xmm2
  102,15,114,210,16,                      //psrld         $0x10,%xmm2
  102,15,219,208,                         //pand          %xmm0,%xmm2
  102,65,15,219,192,                      //pand          %xmm8,%xmm0
  102,72,15,58,22,193,1,                  //pextrq        $0x1,%xmm0,%rcx
  65,137,201,                             //mov           %ecx,%r9d
  72,193,233,32,                          //shr           $0x20,%rcx
  102,73,15,126,194,                      //movq          %xmm0,%r10
  69,137,211,                             //mov           %r10d,%r11d
  73,193,234,32,                          //shr           $0x20,%r10
  243,67,15,16,4,152,                     //movss         (%r8,%r11,4),%xmm0
  102,67,15,58,33,4,144,16,               //insertps      $0x10,(%r8,%r10,4),%xmm0
  102,67,15,58,33,4,136,32,               //insertps      $0x20,(%r8,%r9,4),%xmm0
  102,65,15,58,33,4,136,48,               //insertps      $0x30,(%r8,%rcx,4),%xmm0
  72,139,72,16,                           //mov           0x10(%rax),%rcx
  102,73,15,58,22,200,1,                  //pextrq        $0x1,%xmm1,%r8
  69,137,193,                             //mov           %r8d,%r9d
  73,193,232,32,                          //shr           $0x20,%r8
  102,73,15,126,202,                      //movq          %xmm1,%r10
  69,137,211,                             //mov           %r10d,%r11d
  73,193,234,32,                          //shr           $0x20,%r10
  243,66,15,16,12,153,                    //movss         (%rcx,%r11,4),%xmm1
  102,66,15,58,33,12,145,16,              //insertps      $0x10,(%rcx,%r10,4),%xmm1
  243,66,15,16,28,137,                    //movss         (%rcx,%r9,4),%xmm3
  102,15,58,33,203,32,                    //insertps      $0x20,%xmm3,%xmm1
  243,66,15,16,28,129,                    //movss         (%rcx,%r8,4),%xmm3
  102,15,58,33,203,48,                    //insertps      $0x30,%xmm3,%xmm1
  72,139,64,24,                           //mov           0x18(%rax),%rax
  102,72,15,58,22,209,1,                  //pextrq        $0x1,%xmm2,%rcx
  65,137,200,                             //mov           %ecx,%r8d
  72,193,233,32,                          //shr           $0x20,%rcx
  102,73,15,126,209,                      //movq          %xmm2,%r9
  69,137,202,                             //mov           %r9d,%r10d
  73,193,233,32,                          //shr           $0x20,%r9
  243,66,15,16,20,144,                    //movss         (%rax,%r10,4),%xmm2
  102,66,15,58,33,20,136,16,              //insertps      $0x10,(%rax,%r9,4),%xmm2
  243,66,15,16,28,128,                    //movss         (%rax,%r8,4),%xmm3
  102,15,58,33,211,32,                    //insertps      $0x20,%xmm3,%xmm2
  243,15,16,28,136,                       //movss         (%rax,%rcx,4),%xmm3
  102,15,58,33,211,48,                    //insertps      $0x30,%xmm3,%xmm2
  102,65,15,114,208,24,                   //psrld         $0x18,%xmm8
  69,15,91,192,                           //cvtdq2ps      %xmm8,%xmm8
  243,15,16,90,12,                        //movss         0xc(%rdx),%xmm3
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_a8_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,15,56,49,4,56,                      //pmovzxbd      (%rax,%rdi,1),%xmm0
  15,91,192,                              //cvtdq2ps      %xmm0,%xmm0
  243,15,16,90,12,                        //movss         0xc(%rdx),%xmm3
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  15,89,216,                              //mulps         %xmm0,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,87,192,                              //xorps         %xmm0,%xmm0
  15,87,201,                              //xorps         %xmm1,%xmm1
  15,87,210,                              //xorps         %xmm2,%xmm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_a8_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,68,15,16,66,8,                      //movss         0x8(%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,89,195,                           //mulps         %xmm3,%xmm8
  102,69,15,91,192,                       //cvtps2dq      %xmm8,%xmm8
  102,69,15,56,43,192,                    //packusdw      %xmm8,%xmm8
  102,69,15,103,192,                      //packuswb      %xmm8,%xmm8
  102,68,15,126,4,56,                     //movd          %xmm8,(%rax,%rdi,1)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_565_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,68,15,56,51,12,120,                 //pmovzxwd      (%rax,%rdi,2),%xmm9
  102,15,110,66,104,                      //movd          0x68(%rdx),%xmm0
  102,15,112,192,0,                       //pshufd        $0x0,%xmm0,%xmm0
  102,65,15,219,193,                      //pand          %xmm9,%xmm0
  15,91,200,                              //cvtdq2ps      %xmm0,%xmm1
  243,15,16,26,                           //movss         (%rdx),%xmm3
  243,15,16,66,116,                       //movss         0x74(%rdx),%xmm0
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  15,89,193,                              //mulps         %xmm1,%xmm0
  102,15,110,74,108,                      //movd          0x6c(%rdx),%xmm1
  102,15,112,201,0,                       //pshufd        $0x0,%xmm1,%xmm1
  102,65,15,219,201,                      //pand          %xmm9,%xmm1
  68,15,91,193,                           //cvtdq2ps      %xmm1,%xmm8
  243,15,16,74,120,                       //movss         0x78(%rdx),%xmm1
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  102,15,110,82,112,                      //movd          0x70(%rdx),%xmm2
  102,15,112,210,0,                       //pshufd        $0x0,%xmm2,%xmm2
  102,65,15,219,209,                      //pand          %xmm9,%xmm2
  68,15,91,194,                           //cvtdq2ps      %xmm2,%xmm8
  243,15,16,82,124,                       //movss         0x7c(%rdx),%xmm2
  15,198,210,0,                           //shufps        $0x0,%xmm2,%xmm2
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_565_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,68,15,16,130,128,0,0,0,             //movss         0x80(%rdx),%xmm8
  243,68,15,16,138,132,0,0,0,             //movss         0x84(%rdx),%xmm9
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  69,15,40,208,                           //movaps        %xmm8,%xmm10
  68,15,89,208,                           //mulps         %xmm0,%xmm10
  102,69,15,91,210,                       //cvtps2dq      %xmm10,%xmm10
  102,65,15,114,242,11,                   //pslld         $0xb,%xmm10
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  68,15,89,201,                           //mulps         %xmm1,%xmm9
  102,69,15,91,201,                       //cvtps2dq      %xmm9,%xmm9
  102,65,15,114,241,5,                    //pslld         $0x5,%xmm9
  102,69,15,235,202,                      //por           %xmm10,%xmm9
  68,15,89,194,                           //mulps         %xmm2,%xmm8
  102,69,15,91,192,                       //cvtps2dq      %xmm8,%xmm8
  102,69,15,86,193,                       //orpd          %xmm9,%xmm8
  102,69,15,56,43,192,                    //packusdw      %xmm8,%xmm8
  102,68,15,214,4,120,                    //movq          %xmm8,(%rax,%rdi,2)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_8888_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,15,111,28,184,                      //movdqu        (%rax,%rdi,4),%xmm3
  102,15,110,66,16,                       //movd          0x10(%rdx),%xmm0
  102,15,112,192,0,                       //pshufd        $0x0,%xmm0,%xmm0
  102,15,111,203,                         //movdqa        %xmm3,%xmm1
  102,15,114,209,8,                       //psrld         $0x8,%xmm1
  102,15,219,200,                         //pand          %xmm0,%xmm1
  102,15,111,211,                         //movdqa        %xmm3,%xmm2
  102,15,114,210,16,                      //psrld         $0x10,%xmm2
  102,15,219,208,                         //pand          %xmm0,%xmm2
  102,15,219,195,                         //pand          %xmm3,%xmm0
  15,91,192,                              //cvtdq2ps      %xmm0,%xmm0
  243,68,15,16,66,12,                     //movss         0xc(%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  15,91,201,                              //cvtdq2ps      %xmm1,%xmm1
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  15,91,210,                              //cvtdq2ps      %xmm2,%xmm2
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  102,15,114,211,24,                      //psrld         $0x18,%xmm3
  15,91,219,                              //cvtdq2ps      %xmm3,%xmm3
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_8888_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,68,15,16,66,8,                      //movss         0x8(%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,200,                           //mulps         %xmm0,%xmm9
  102,69,15,91,201,                       //cvtps2dq      %xmm9,%xmm9
  69,15,40,208,                           //movaps        %xmm8,%xmm10
  68,15,89,209,                           //mulps         %xmm1,%xmm10
  102,69,15,91,210,                       //cvtps2dq      %xmm10,%xmm10
  102,65,15,114,242,8,                    //pslld         $0x8,%xmm10
  102,69,15,235,209,                      //por           %xmm9,%xmm10
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,202,                           //mulps         %xmm2,%xmm9
  102,69,15,91,201,                       //cvtps2dq      %xmm9,%xmm9
  102,65,15,114,241,16,                   //pslld         $0x10,%xmm9
  68,15,89,195,                           //mulps         %xmm3,%xmm8
  102,69,15,91,192,                       //cvtps2dq      %xmm8,%xmm8
  102,65,15,114,240,24,                   //pslld         $0x18,%xmm8
  102,69,15,235,193,                      //por           %xmm9,%xmm8
  102,69,15,235,194,                      //por           %xmm10,%xmm8
  243,68,15,127,4,184,                    //movdqu        %xmm8,(%rax,%rdi,4)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_f16_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,15,111,4,248,                       //movdqu        (%rax,%rdi,8),%xmm0
  243,15,111,76,248,16,                   //movdqu        0x10(%rax,%rdi,8),%xmm1
  102,15,111,208,                         //movdqa        %xmm0,%xmm2
  102,15,97,209,                          //punpcklwd     %xmm1,%xmm2
  102,15,105,193,                         //punpckhwd     %xmm1,%xmm0
  102,68,15,111,194,                      //movdqa        %xmm2,%xmm8
  102,68,15,97,192,                       //punpcklwd     %xmm0,%xmm8
  102,15,105,208,                         //punpckhwd     %xmm0,%xmm2
  102,15,110,66,100,                      //movd          0x64(%rdx),%xmm0
  102,15,112,216,0,                       //pshufd        $0x0,%xmm0,%xmm3
  102,15,111,203,                         //movdqa        %xmm3,%xmm1
  102,65,15,101,200,                      //pcmpgtw       %xmm8,%xmm1
  102,65,15,223,200,                      //pandn         %xmm8,%xmm1
  102,15,101,218,                         //pcmpgtw       %xmm2,%xmm3
  102,15,223,218,                         //pandn         %xmm2,%xmm3
  102,15,56,51,193,                       //pmovzxwd      %xmm1,%xmm0
  102,15,114,240,13,                      //pslld         $0xd,%xmm0
  102,15,110,82,92,                       //movd          0x5c(%rdx),%xmm2
  102,68,15,112,194,0,                    //pshufd        $0x0,%xmm2,%xmm8
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  102,69,15,239,201,                      //pxor          %xmm9,%xmm9
  102,65,15,105,201,                      //punpckhwd     %xmm9,%xmm1
  102,15,114,241,13,                      //pslld         $0xd,%xmm1
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  102,15,56,51,211,                       //pmovzxwd      %xmm3,%xmm2
  102,15,114,242,13,                      //pslld         $0xd,%xmm2
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  102,65,15,105,217,                      //punpckhwd     %xmm9,%xmm3
  102,15,114,243,13,                      //pslld         $0xd,%xmm3
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_f16_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,68,15,110,66,96,                    //movd          0x60(%rdx),%xmm8
  102,69,15,112,192,0,                    //pshufd        $0x0,%xmm8,%xmm8
  102,69,15,111,200,                      //movdqa        %xmm8,%xmm9
  68,15,89,200,                           //mulps         %xmm0,%xmm9
  102,65,15,114,209,13,                   //psrld         $0xd,%xmm9
  102,69,15,111,208,                      //movdqa        %xmm8,%xmm10
  68,15,89,209,                           //mulps         %xmm1,%xmm10
  102,65,15,114,210,13,                   //psrld         $0xd,%xmm10
  102,69,15,111,216,                      //movdqa        %xmm8,%xmm11
  68,15,89,218,                           //mulps         %xmm2,%xmm11
  102,65,15,114,211,13,                   //psrld         $0xd,%xmm11
  68,15,89,195,                           //mulps         %xmm3,%xmm8
  102,65,15,114,208,13,                   //psrld         $0xd,%xmm8
  102,65,15,115,250,2,                    //pslldq        $0x2,%xmm10
  102,69,15,235,209,                      //por           %xmm9,%xmm10
  102,65,15,115,248,2,                    //pslldq        $0x2,%xmm8
  102,69,15,235,195,                      //por           %xmm11,%xmm8
  102,69,15,111,202,                      //movdqa        %xmm10,%xmm9
  102,69,15,98,200,                       //punpckldq     %xmm8,%xmm9
  243,68,15,127,12,248,                   //movdqu        %xmm9,(%rax,%rdi,8)
  102,69,15,106,208,                      //punpckhdq     %xmm8,%xmm10
  243,68,15,127,84,248,16,                //movdqu        %xmm10,0x10(%rax,%rdi,8)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_f32_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,137,249,                             //mov           %rdi,%rcx
  72,193,225,4,                           //shl           $0x4,%rcx
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  68,15,40,200,                           //movaps        %xmm0,%xmm9
  68,15,20,201,                           //unpcklps      %xmm1,%xmm9
  68,15,40,210,                           //movaps        %xmm2,%xmm10
  68,15,40,218,                           //movaps        %xmm2,%xmm11
  68,15,20,219,                           //unpcklps      %xmm3,%xmm11
  68,15,21,193,                           //unpckhps      %xmm1,%xmm8
  68,15,21,211,                           //unpckhps      %xmm3,%xmm10
  69,15,40,225,                           //movaps        %xmm9,%xmm12
  102,69,15,20,227,                       //unpcklpd      %xmm11,%xmm12
  102,69,15,21,203,                       //unpckhpd      %xmm11,%xmm9
  69,15,40,216,                           //movaps        %xmm8,%xmm11
  102,69,15,20,218,                       //unpcklpd      %xmm10,%xmm11
  102,69,15,21,194,                       //unpckhpd      %xmm10,%xmm8
  102,68,15,17,36,8,                      //movupd        %xmm12,(%rax,%rcx,1)
  102,68,15,17,76,8,16,                   //movupd        %xmm9,0x10(%rax,%rcx,1)
  102,68,15,17,92,8,32,                   //movupd        %xmm11,0x20(%rax,%rcx,1)
  102,68,15,17,68,8,48,                   //movupd        %xmm8,0x30(%rax,%rcx,1)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_x_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  68,15,95,192,                           //maxps         %xmm0,%xmm8
  243,68,15,16,8,                         //movss         (%rax),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  102,15,118,192,                         //pcmpeqd       %xmm0,%xmm0
  102,65,15,254,193,                      //paddd         %xmm9,%xmm0
  68,15,93,192,                           //minps         %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,192,                           //movaps        %xmm8,%xmm0
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_y_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  68,15,95,193,                           //maxps         %xmm1,%xmm8
  243,68,15,16,8,                         //movss         (%rax),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  102,15,118,201,                         //pcmpeqd       %xmm1,%xmm1
  102,65,15,254,201,                      //paddd         %xmm9,%xmm1
  68,15,93,193,                           //minps         %xmm1,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,200,                           //movaps        %xmm8,%xmm1
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_repeat_x_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,40,200,                           //movaps        %xmm0,%xmm9
  69,15,94,200,                           //divps         %xmm8,%xmm9
  102,69,15,58,8,201,1,                   //roundps       $0x1,%xmm9,%xmm9
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  65,15,92,193,                           //subps         %xmm9,%xmm0
  102,69,15,118,201,                      //pcmpeqd       %xmm9,%xmm9
  102,69,15,254,200,                      //paddd         %xmm8,%xmm9
  65,15,93,193,                           //minps         %xmm9,%xmm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_repeat_y_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,40,201,                           //movaps        %xmm1,%xmm9
  69,15,94,200,                           //divps         %xmm8,%xmm9
  102,69,15,58,8,201,1,                   //roundps       $0x1,%xmm9,%xmm9
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  65,15,92,201,                           //subps         %xmm9,%xmm1
  102,69,15,118,201,                      //pcmpeqd       %xmm9,%xmm9
  102,69,15,254,200,                      //paddd         %xmm8,%xmm9
  65,15,93,201,                           //minps         %xmm9,%xmm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_mirror_x_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  65,15,92,193,                           //subps         %xmm9,%xmm0
  243,69,15,88,192,                       //addss         %xmm8,%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,40,208,                           //movaps        %xmm0,%xmm10
  69,15,94,208,                           //divps         %xmm8,%xmm10
  102,69,15,58,8,210,1,                   //roundps       $0x1,%xmm10,%xmm10
  69,15,89,208,                           //mulps         %xmm8,%xmm10
  65,15,92,194,                           //subps         %xmm10,%xmm0
  65,15,92,193,                           //subps         %xmm9,%xmm0
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  68,15,92,192,                           //subps         %xmm0,%xmm8
  65,15,84,192,                           //andps         %xmm8,%xmm0
  102,69,15,118,192,                      //pcmpeqd       %xmm8,%xmm8
  102,69,15,254,193,                      //paddd         %xmm9,%xmm8
  65,15,93,192,                           //minps         %xmm8,%xmm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_mirror_y_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  65,15,92,201,                           //subps         %xmm9,%xmm1
  243,69,15,88,192,                       //addss         %xmm8,%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,40,209,                           //movaps        %xmm1,%xmm10
  69,15,94,208,                           //divps         %xmm8,%xmm10
  102,69,15,58,8,210,1,                   //roundps       $0x1,%xmm10,%xmm10
  69,15,89,208,                           //mulps         %xmm8,%xmm10
  65,15,92,202,                           //subps         %xmm10,%xmm1
  65,15,92,201,                           //subps         %xmm9,%xmm1
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  68,15,92,193,                           //subps         %xmm1,%xmm8
  65,15,84,200,                           //andps         %xmm8,%xmm1
  102,69,15,118,192,                      //pcmpeqd       %xmm8,%xmm8
  102,69,15,254,193,                      //paddd         %xmm9,%xmm8
  65,15,93,200,                           //minps         %xmm8,%xmm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_2x3_sse41[] = {
  68,15,40,201,                           //movaps        %xmm1,%xmm9
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,15,16,0,                            //movss         (%rax),%xmm0
  243,15,16,72,4,                         //movss         0x4(%rax),%xmm1
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  243,68,15,16,80,8,                      //movss         0x8(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,16,                     //movss         0x10(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,88,194,                           //addps         %xmm10,%xmm0
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  243,68,15,16,80,12,                     //movss         0xc(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,20,                     //movss         0x14(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  65,15,88,202,                           //addps         %xmm10,%xmm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_3x4_sse41[] = {
  68,15,40,201,                           //movaps        %xmm1,%xmm9
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,15,16,0,                            //movss         (%rax),%xmm0
  243,15,16,72,4,                         //movss         0x4(%rax),%xmm1
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  243,68,15,16,80,12,                     //movss         0xc(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,24,                     //movss         0x18(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,96,36,                     //movss         0x24(%rax),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  68,15,89,218,                           //mulps         %xmm2,%xmm11
  69,15,88,220,                           //addps         %xmm12,%xmm11
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,88,194,                           //addps         %xmm10,%xmm0
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  243,68,15,16,80,16,                     //movss         0x10(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,28,                     //movss         0x1c(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,96,40,                     //movss         0x28(%rax),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  68,15,89,218,                           //mulps         %xmm2,%xmm11
  69,15,88,220,                           //addps         %xmm12,%xmm11
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  65,15,88,202,                           //addps         %xmm10,%xmm1
  243,68,15,16,80,8,                      //movss         0x8(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,20,                     //movss         0x14(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,96,32,                     //movss         0x20(%rax),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  243,68,15,16,104,44,                    //movss         0x2c(%rax),%xmm13
  69,15,198,237,0,                        //shufps        $0x0,%xmm13,%xmm13
  68,15,89,226,                           //mulps         %xmm2,%xmm12
  69,15,88,229,                           //addps         %xmm13,%xmm12
  69,15,89,217,                           //mulps         %xmm9,%xmm11
  69,15,88,220,                           //addps         %xmm12,%xmm11
  69,15,89,208,                           //mulps         %xmm8,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,210,                           //movaps        %xmm10,%xmm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_perspective_sse41[] = {
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,15,16,0,                            //movss         (%rax),%xmm0
  243,68,15,16,72,4,                      //movss         0x4(%rax),%xmm9
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  243,68,15,16,80,8,                      //movss         0x8(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  68,15,89,201,                           //mulps         %xmm1,%xmm9
  69,15,88,202,                           //addps         %xmm10,%xmm9
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,88,193,                           //addps         %xmm9,%xmm0
  243,68,15,16,72,12,                     //movss         0xc(%rax),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  243,68,15,16,80,16,                     //movss         0x10(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,20,                     //movss         0x14(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  68,15,89,209,                           //mulps         %xmm1,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  69,15,88,202,                           //addps         %xmm10,%xmm9
  243,68,15,16,80,24,                     //movss         0x18(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,28,                     //movss         0x1c(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,96,32,                     //movss         0x20(%rax),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  68,15,89,217,                           //mulps         %xmm1,%xmm11
  69,15,88,220,                           //addps         %xmm12,%xmm11
  69,15,89,208,                           //mulps         %xmm8,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,83,202,                           //rcpps         %xmm10,%xmm1
  15,89,193,                              //mulps         %xmm1,%xmm0
  68,15,89,201,                           //mulps         %xmm1,%xmm9
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,201,                           //movaps        %xmm9,%xmm1
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_linear_gradient_2stops_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  68,15,16,8,                             //movups        (%rax),%xmm9
  15,16,88,16,                            //movups        0x10(%rax),%xmm3
  68,15,40,195,                           //movaps        %xmm3,%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,40,201,                           //movaps        %xmm9,%xmm1
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  68,15,89,192,                           //mulps         %xmm0,%xmm8
  68,15,88,193,                           //addps         %xmm1,%xmm8
  15,40,203,                              //movaps        %xmm3,%xmm1
  15,198,201,85,                          //shufps        $0x55,%xmm1,%xmm1
  65,15,40,209,                           //movaps        %xmm9,%xmm2
  15,198,210,85,                          //shufps        $0x55,%xmm2,%xmm2
  15,89,200,                              //mulps         %xmm0,%xmm1
  15,88,202,                              //addps         %xmm2,%xmm1
  15,40,211,                              //movaps        %xmm3,%xmm2
  15,198,210,170,                         //shufps        $0xaa,%xmm2,%xmm2
  69,15,40,209,                           //movaps        %xmm9,%xmm10
  69,15,198,210,170,                      //shufps        $0xaa,%xmm10,%xmm10
  15,89,208,                              //mulps         %xmm0,%xmm2
  65,15,88,210,                           //addps         %xmm10,%xmm2
  15,198,219,255,                         //shufps        $0xff,%xmm3,%xmm3
  69,15,198,201,255,                      //shufps        $0xff,%xmm9,%xmm9
  15,89,216,                              //mulps         %xmm0,%xmm3
  65,15,88,217,                           //addps         %xmm9,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,192,                           //movaps        %xmm8,%xmm0
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_start_pipeline_sse2[] = {
  65,87,                                  //push          %r15
  65,86,                                  //push          %r14
  65,85,                                  //push          %r13
  65,84,                                  //push          %r12
  83,                                     //push          %rbx
  73,137,207,                             //mov           %rcx,%r15
  73,137,214,                             //mov           %rdx,%r14
  72,137,251,                             //mov           %rdi,%rbx
  72,173,                                 //lods          %ds:(%rsi),%rax
  73,137,196,                             //mov           %rax,%r12
  73,137,245,                             //mov           %rsi,%r13
  72,141,67,4,                            //lea           0x4(%rbx),%rax
  76,57,248,                              //cmp           %r15,%rax
  118,5,                                  //jbe           28 <_sk_start_pipeline_sse2+0x28>
  72,137,216,                             //mov           %rbx,%rax
  235,52,                                 //jmp           5c <_sk_start_pipeline_sse2+0x5c>
  15,87,192,                              //xorps         %xmm0,%xmm0
  15,87,201,                              //xorps         %xmm1,%xmm1
  15,87,210,                              //xorps         %xmm2,%xmm2
  15,87,219,                              //xorps         %xmm3,%xmm3
  15,87,228,                              //xorps         %xmm4,%xmm4
  15,87,237,                              //xorps         %xmm5,%xmm5
  15,87,246,                              //xorps         %xmm6,%xmm6
  15,87,255,                              //xorps         %xmm7,%xmm7
  72,137,223,                             //mov           %rbx,%rdi
  76,137,238,                             //mov           %r13,%rsi
  76,137,242,                             //mov           %r14,%rdx
  65,255,212,                             //callq         *%r12
  72,141,67,4,                            //lea           0x4(%rbx),%rax
  72,131,195,8,                           //add           $0x8,%rbx
  76,57,251,                              //cmp           %r15,%rbx
  72,137,195,                             //mov           %rax,%rbx
  118,204,                                //jbe           28 <_sk_start_pipeline_sse2+0x28>
  91,                                     //pop           %rbx
  65,92,                                  //pop           %r12
  65,93,                                  //pop           %r13
  65,94,                                  //pop           %r14
  65,95,                                  //pop           %r15
  195,                                    //retq
};

CODE const uint8_t sk_just_return_sse2[] = {
  195,                                    //retq
};

CODE const uint8_t sk_seed_shader_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  102,15,110,199,                         //movd          %edi,%xmm0
  102,15,112,192,0,                       //pshufd        $0x0,%xmm0,%xmm0
  15,91,200,                              //cvtdq2ps      %xmm0,%xmm1
  243,15,16,18,                           //movss         (%rdx),%xmm2
  243,15,16,90,4,                         //movss         0x4(%rdx),%xmm3
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  15,88,203,                              //addps         %xmm3,%xmm1
  15,16,66,20,                            //movups        0x14(%rdx),%xmm0
  15,88,193,                              //addps         %xmm1,%xmm0
  102,15,110,8,                           //movd          (%rax),%xmm1
  102,15,112,201,0,                       //pshufd        $0x0,%xmm1,%xmm1
  15,91,201,                              //cvtdq2ps      %xmm1,%xmm1
  15,88,203,                              //addps         %xmm3,%xmm1
  15,198,210,0,                           //shufps        $0x0,%xmm2,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,87,219,                              //xorps         %xmm3,%xmm3
  15,87,228,                              //xorps         %xmm4,%xmm4
  15,87,237,                              //xorps         %xmm5,%xmm5
  15,87,246,                              //xorps         %xmm6,%xmm6
  15,87,255,                              //xorps         %xmm7,%xmm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_constant_color_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,16,24,                               //movups        (%rax),%xmm3
  15,40,195,                              //movaps        %xmm3,%xmm0
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  15,40,203,                              //movaps        %xmm3,%xmm1
  15,198,201,85,                          //shufps        $0x55,%xmm1,%xmm1
  15,40,211,                              //movaps        %xmm3,%xmm2
  15,198,210,170,                         //shufps        $0xaa,%xmm2,%xmm2
  15,198,219,255,                         //shufps        $0xff,%xmm3,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clear_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,87,192,                              //xorps         %xmm0,%xmm0
  15,87,201,                              //xorps         %xmm1,%xmm1
  15,87,210,                              //xorps         %xmm2,%xmm2
  15,87,219,                              //xorps         %xmm3,%xmm3
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_plus__sse2[] = {
  15,88,196,                              //addps         %xmm4,%xmm0
  15,88,205,                              //addps         %xmm5,%xmm1
  15,88,214,                              //addps         %xmm6,%xmm2
  15,88,223,                              //addps         %xmm7,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_srcover_sse2[] = {
  243,68,15,16,2,                         //movss         (%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,92,195,                           //subps         %xmm3,%xmm8
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,204,                           //mulps         %xmm4,%xmm9
  65,15,88,193,                           //addps         %xmm9,%xmm0
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,205,                           //mulps         %xmm5,%xmm9
  65,15,88,201,                           //addps         %xmm9,%xmm1
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,206,                           //mulps         %xmm6,%xmm9
  65,15,88,209,                           //addps         %xmm9,%xmm2
  68,15,89,199,                           //mulps         %xmm7,%xmm8
  65,15,88,216,                           //addps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_dstover_sse2[] = {
  243,68,15,16,2,                         //movss         (%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,92,199,                           //subps         %xmm7,%xmm8
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  15,88,196,                              //addps         %xmm4,%xmm0
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  15,88,205,                              //addps         %xmm5,%xmm1
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  15,88,214,                              //addps         %xmm6,%xmm2
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  15,88,223,                              //addps         %xmm7,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_0_sse2[] = {
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  65,15,95,192,                           //maxps         %xmm8,%xmm0
  65,15,95,200,                           //maxps         %xmm8,%xmm1
  65,15,95,208,                           //maxps         %xmm8,%xmm2
  65,15,95,216,                           //maxps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_1_sse2[] = {
  243,68,15,16,2,                         //movss         (%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,93,192,                           //minps         %xmm8,%xmm0
  65,15,93,200,                           //minps         %xmm8,%xmm1
  65,15,93,208,                           //minps         %xmm8,%xmm2
  65,15,93,216,                           //minps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_a_sse2[] = {
  243,68,15,16,2,                         //movss         (%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,93,216,                           //minps         %xmm8,%xmm3
  15,93,195,                              //minps         %xmm3,%xmm0
  15,93,203,                              //minps         %xmm3,%xmm1
  15,93,211,                              //minps         %xmm3,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_set_rgb_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,15,16,0,                            //movss         (%rax),%xmm0
  243,15,16,72,4,                         //movss         0x4(%rax),%xmm1
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  243,15,16,80,8,                         //movss         0x8(%rax),%xmm2
  15,198,210,0,                           //shufps        $0x0,%xmm2,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_swap_rb_sse2[] = {
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,194,                              //movaps        %xmm2,%xmm0
  65,15,40,208,                           //movaps        %xmm8,%xmm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_swap_sse2[] = {
  68,15,40,195,                           //movaps        %xmm3,%xmm8
  68,15,40,202,                           //movaps        %xmm2,%xmm9
  68,15,40,209,                           //movaps        %xmm1,%xmm10
  68,15,40,216,                           //movaps        %xmm0,%xmm11
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,196,                              //movaps        %xmm4,%xmm0
  15,40,205,                              //movaps        %xmm5,%xmm1
  15,40,214,                              //movaps        %xmm6,%xmm2
  15,40,223,                              //movaps        %xmm7,%xmm3
  65,15,40,227,                           //movaps        %xmm11,%xmm4
  65,15,40,234,                           //movaps        %xmm10,%xmm5
  65,15,40,241,                           //movaps        %xmm9,%xmm6
  65,15,40,248,                           //movaps        %xmm8,%xmm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_move_src_dst_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,224,                              //movaps        %xmm0,%xmm4
  15,40,233,                              //movaps        %xmm1,%xmm5
  15,40,242,                              //movaps        %xmm2,%xmm6
  15,40,251,                              //movaps        %xmm3,%xmm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_move_dst_src_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,196,                              //movaps        %xmm4,%xmm0
  15,40,205,                              //movaps        %xmm5,%xmm1
  15,40,214,                              //movaps        %xmm6,%xmm2
  15,40,223,                              //movaps        %xmm7,%xmm3
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_premul_sse2[] = {
  15,89,195,                              //mulps         %xmm3,%xmm0
  15,89,203,                              //mulps         %xmm3,%xmm1
  15,89,211,                              //mulps         %xmm3,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_unpremul_sse2[] = {
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  68,15,194,195,0,                        //cmpeqps       %xmm3,%xmm8
  243,68,15,16,10,                        //movss         (%rdx),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  68,15,94,203,                           //divps         %xmm3,%xmm9
  69,15,85,193,                           //andnps        %xmm9,%xmm8
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_from_srgb_sse2[] = {
  243,68,15,16,66,64,                     //movss         0x40(%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  69,15,40,232,                           //movaps        %xmm8,%xmm13
  68,15,89,232,                           //mulps         %xmm0,%xmm13
  68,15,40,224,                           //movaps        %xmm0,%xmm12
  69,15,89,228,                           //mulps         %xmm12,%xmm12
  243,68,15,16,74,60,                     //movss         0x3c(%rdx),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  243,68,15,16,82,52,                     //movss         0x34(%rdx),%xmm10
  243,68,15,16,90,56,                     //movss         0x38(%rdx),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,40,241,                           //movaps        %xmm9,%xmm14
  68,15,89,240,                           //mulps         %xmm0,%xmm14
  69,15,88,243,                           //addps         %xmm11,%xmm14
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  69,15,89,244,                           //mulps         %xmm12,%xmm14
  69,15,88,242,                           //addps         %xmm10,%xmm14
  243,68,15,16,98,68,                     //movss         0x44(%rdx),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  65,15,194,196,1,                        //cmpltps       %xmm12,%xmm0
  68,15,84,232,                           //andps         %xmm0,%xmm13
  65,15,85,198,                           //andnps        %xmm14,%xmm0
  65,15,86,197,                           //orps          %xmm13,%xmm0
  69,15,40,232,                           //movaps        %xmm8,%xmm13
  68,15,89,233,                           //mulps         %xmm1,%xmm13
  68,15,40,241,                           //movaps        %xmm1,%xmm14
  69,15,89,246,                           //mulps         %xmm14,%xmm14
  69,15,40,249,                           //movaps        %xmm9,%xmm15
  68,15,89,249,                           //mulps         %xmm1,%xmm15
  69,15,88,251,                           //addps         %xmm11,%xmm15
  69,15,89,254,                           //mulps         %xmm14,%xmm15
  69,15,88,250,                           //addps         %xmm10,%xmm15
  65,15,194,204,1,                        //cmpltps       %xmm12,%xmm1
  68,15,84,233,                           //andps         %xmm1,%xmm13
  65,15,85,207,                           //andnps        %xmm15,%xmm1
  65,15,86,205,                           //orps          %xmm13,%xmm1
  68,15,89,194,                           //mulps         %xmm2,%xmm8
  68,15,40,234,                           //movaps        %xmm2,%xmm13
  69,15,89,237,                           //mulps         %xmm13,%xmm13
  68,15,89,202,                           //mulps         %xmm2,%xmm9
  69,15,88,203,                           //addps         %xmm11,%xmm9
  69,15,89,205,                           //mulps         %xmm13,%xmm9
  69,15,88,202,                           //addps         %xmm10,%xmm9
  65,15,194,212,1,                        //cmpltps       %xmm12,%xmm2
  68,15,84,194,                           //andps         %xmm2,%xmm8
  65,15,85,209,                           //andnps        %xmm9,%xmm2
  65,15,86,208,                           //orps          %xmm8,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_to_srgb_sse2[] = {
  72,131,236,40,                          //sub           $0x28,%rsp
  15,41,124,36,16,                        //movaps        %xmm7,0x10(%rsp)
  15,41,52,36,                            //movaps        %xmm6,(%rsp)
  15,40,245,                              //movaps        %xmm5,%xmm6
  15,40,236,                              //movaps        %xmm4,%xmm5
  15,40,227,                              //movaps        %xmm3,%xmm4
  68,15,82,192,                           //rsqrtps       %xmm0,%xmm8
  69,15,83,232,                           //rcpps         %xmm8,%xmm13
  69,15,82,248,                           //rsqrtps       %xmm8,%xmm15
  243,15,16,26,                           //movss         (%rdx),%xmm3
  243,68,15,16,66,72,                     //movss         0x48(%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  69,15,40,240,                           //movaps        %xmm8,%xmm14
  68,15,89,240,                           //mulps         %xmm0,%xmm14
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  243,68,15,16,82,76,                     //movss         0x4c(%rdx),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,90,80,                     //movss         0x50(%rdx),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,98,84,                     //movss         0x54(%rdx),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  69,15,89,235,                           //mulps         %xmm11,%xmm13
  69,15,88,236,                           //addps         %xmm12,%xmm13
  69,15,89,250,                           //mulps         %xmm10,%xmm15
  69,15,88,253,                           //addps         %xmm13,%xmm15
  68,15,40,203,                           //movaps        %xmm3,%xmm9
  69,15,93,207,                           //minps         %xmm15,%xmm9
  243,68,15,16,106,88,                    //movss         0x58(%rdx),%xmm13
  69,15,198,237,0,                        //shufps        $0x0,%xmm13,%xmm13
  65,15,194,197,1,                        //cmpltps       %xmm13,%xmm0
  68,15,84,240,                           //andps         %xmm0,%xmm14
  65,15,85,193,                           //andnps        %xmm9,%xmm0
  65,15,86,198,                           //orps          %xmm14,%xmm0
  68,15,82,201,                           //rsqrtps       %xmm1,%xmm9
  69,15,83,241,                           //rcpps         %xmm9,%xmm14
  69,15,82,201,                           //rsqrtps       %xmm9,%xmm9
  69,15,89,243,                           //mulps         %xmm11,%xmm14
  69,15,88,244,                           //addps         %xmm12,%xmm14
  69,15,89,202,                           //mulps         %xmm10,%xmm9
  69,15,88,206,                           //addps         %xmm14,%xmm9
  68,15,40,243,                           //movaps        %xmm3,%xmm14
  69,15,93,241,                           //minps         %xmm9,%xmm14
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,201,                           //mulps         %xmm1,%xmm9
  65,15,194,205,1,                        //cmpltps       %xmm13,%xmm1
  68,15,84,201,                           //andps         %xmm1,%xmm9
  65,15,85,206,                           //andnps        %xmm14,%xmm1
  65,15,86,201,                           //orps          %xmm9,%xmm1
  68,15,82,202,                           //rsqrtps       %xmm2,%xmm9
  69,15,83,241,                           //rcpps         %xmm9,%xmm14
  69,15,89,243,                           //mulps         %xmm11,%xmm14
  69,15,88,244,                           //addps         %xmm12,%xmm14
  65,15,82,249,                           //rsqrtps       %xmm9,%xmm7
  65,15,89,250,                           //mulps         %xmm10,%xmm7
  65,15,88,254,                           //addps         %xmm14,%xmm7
  15,93,223,                              //minps         %xmm7,%xmm3
  68,15,89,194,                           //mulps         %xmm2,%xmm8
  65,15,194,213,1,                        //cmpltps       %xmm13,%xmm2
  68,15,84,194,                           //andps         %xmm2,%xmm8
  15,85,211,                              //andnps        %xmm3,%xmm2
  65,15,86,208,                           //orps          %xmm8,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,220,                              //movaps        %xmm4,%xmm3
  15,40,229,                              //movaps        %xmm5,%xmm4
  15,40,238,                              //movaps        %xmm6,%xmm5
  15,40,52,36,                            //movaps        (%rsp),%xmm6
  15,40,124,36,16,                        //movaps        0x10(%rsp),%xmm7
  72,131,196,40,                          //add           $0x28,%rsp
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_scale_1_float_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_scale_u8_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,68,15,110,4,56,                     //movd          (%rax,%rdi,1),%xmm8
  102,69,15,239,201,                      //pxor          %xmm9,%xmm9
  102,69,15,96,193,                       //punpcklbw     %xmm9,%xmm8
  102,69,15,97,193,                       //punpcklwd     %xmm9,%xmm8
  69,15,91,192,                           //cvtdq2ps      %xmm8,%xmm8
  243,68,15,16,74,12,                     //movss         0xc(%rdx),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  65,15,89,193,                           //mulps         %xmm9,%xmm0
  65,15,89,201,                           //mulps         %xmm9,%xmm1
  65,15,89,209,                           //mulps         %xmm9,%xmm2
  65,15,89,217,                           //mulps         %xmm9,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_lerp_1_float_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  15,92,196,                              //subps         %xmm4,%xmm0
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  15,88,196,                              //addps         %xmm4,%xmm0
  15,92,205,                              //subps         %xmm5,%xmm1
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  15,88,205,                              //addps         %xmm5,%xmm1
  15,92,214,                              //subps         %xmm6,%xmm2
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  15,88,214,                              //addps         %xmm6,%xmm2
  15,92,223,                              //subps         %xmm7,%xmm3
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  15,88,223,                              //addps         %xmm7,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_lerp_u8_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,68,15,110,4,56,                     //movd          (%rax,%rdi,1),%xmm8
  102,69,15,239,201,                      //pxor          %xmm9,%xmm9
  102,69,15,96,193,                       //punpcklbw     %xmm9,%xmm8
  102,69,15,97,193,                       //punpcklwd     %xmm9,%xmm8
  69,15,91,192,                           //cvtdq2ps      %xmm8,%xmm8
  243,68,15,16,74,12,                     //movss         0xc(%rdx),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  15,92,196,                              //subps         %xmm4,%xmm0
  65,15,89,193,                           //mulps         %xmm9,%xmm0
  15,88,196,                              //addps         %xmm4,%xmm0
  15,92,205,                              //subps         %xmm5,%xmm1
  65,15,89,201,                           //mulps         %xmm9,%xmm1
  15,88,205,                              //addps         %xmm5,%xmm1
  15,92,214,                              //subps         %xmm6,%xmm2
  65,15,89,209,                           //mulps         %xmm9,%xmm2
  15,88,214,                              //addps         %xmm6,%xmm2
  15,92,223,                              //subps         %xmm7,%xmm3
  65,15,89,217,                           //mulps         %xmm9,%xmm3
  15,88,223,                              //addps         %xmm7,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_lerp_565_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,68,15,126,4,120,                    //movq          (%rax,%rdi,2),%xmm8
  102,15,239,219,                         //pxor          %xmm3,%xmm3
  102,68,15,97,195,                       //punpcklwd     %xmm3,%xmm8
  102,15,110,90,104,                      //movd          0x68(%rdx),%xmm3
  102,15,112,219,0,                       //pshufd        $0x0,%xmm3,%xmm3
  102,65,15,219,216,                      //pand          %xmm8,%xmm3
  68,15,91,203,                           //cvtdq2ps      %xmm3,%xmm9
  243,15,16,26,                           //movss         (%rdx),%xmm3
  243,68,15,16,82,116,                    //movss         0x74(%rdx),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  102,68,15,110,74,108,                   //movd          0x6c(%rdx),%xmm9
  102,69,15,112,201,0,                    //pshufd        $0x0,%xmm9,%xmm9
  102,69,15,219,200,                      //pand          %xmm8,%xmm9
  69,15,91,201,                           //cvtdq2ps      %xmm9,%xmm9
  243,68,15,16,90,120,                    //movss         0x78(%rdx),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,89,217,                           //mulps         %xmm9,%xmm11
  102,68,15,110,74,112,                   //movd          0x70(%rdx),%xmm9
  102,69,15,112,201,0,                    //pshufd        $0x0,%xmm9,%xmm9
  102,69,15,219,200,                      //pand          %xmm8,%xmm9
  69,15,91,193,                           //cvtdq2ps      %xmm9,%xmm8
  243,68,15,16,74,124,                    //movss         0x7c(%rdx),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  15,92,196,                              //subps         %xmm4,%xmm0
  65,15,89,194,                           //mulps         %xmm10,%xmm0
  15,88,196,                              //addps         %xmm4,%xmm0
  15,92,205,                              //subps         %xmm5,%xmm1
  65,15,89,203,                           //mulps         %xmm11,%xmm1
  15,88,205,                              //addps         %xmm5,%xmm1
  15,92,214,                              //subps         %xmm6,%xmm2
  65,15,89,209,                           //mulps         %xmm9,%xmm2
  15,88,214,                              //addps         %xmm6,%xmm2
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_tables_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,8,                               //mov           (%rax),%rcx
  76,139,64,8,                            //mov           0x8(%rax),%r8
  243,68,15,111,4,185,                    //movdqu        (%rcx,%rdi,4),%xmm8
  102,15,110,66,16,                       //movd          0x10(%rdx),%xmm0
  102,15,112,192,0,                       //pshufd        $0x0,%xmm0,%xmm0
  102,69,15,111,200,                      //movdqa        %xmm8,%xmm9
  102,65,15,114,209,8,                    //psrld         $0x8,%xmm9
  102,68,15,219,200,                      //pand          %xmm0,%xmm9
  102,69,15,111,208,                      //movdqa        %xmm8,%xmm10
  102,65,15,114,210,16,                   //psrld         $0x10,%xmm10
  102,68,15,219,208,                      //pand          %xmm0,%xmm10
  102,65,15,219,192,                      //pand          %xmm8,%xmm0
  102,15,112,216,78,                      //pshufd        $0x4e,%xmm0,%xmm3
  102,72,15,126,217,                      //movq          %xmm3,%rcx
  65,137,201,                             //mov           %ecx,%r9d
  72,193,233,32,                          //shr           $0x20,%rcx
  102,73,15,126,194,                      //movq          %xmm0,%r10
  69,137,211,                             //mov           %r10d,%r11d
  73,193,234,32,                          //shr           $0x20,%r10
  243,67,15,16,28,144,                    //movss         (%r8,%r10,4),%xmm3
  243,65,15,16,4,136,                     //movss         (%r8,%rcx,4),%xmm0
  15,20,216,                              //unpcklps      %xmm0,%xmm3
  243,67,15,16,4,152,                     //movss         (%r8,%r11,4),%xmm0
  243,67,15,16,12,136,                    //movss         (%r8,%r9,4),%xmm1
  15,20,193,                              //unpcklps      %xmm1,%xmm0
  15,20,195,                              //unpcklps      %xmm3,%xmm0
  72,139,72,16,                           //mov           0x10(%rax),%rcx
  102,65,15,112,201,78,                   //pshufd        $0x4e,%xmm9,%xmm1
  102,73,15,126,200,                      //movq          %xmm1,%r8
  69,137,193,                             //mov           %r8d,%r9d
  73,193,232,32,                          //shr           $0x20,%r8
  102,77,15,126,202,                      //movq          %xmm9,%r10
  69,137,211,                             //mov           %r10d,%r11d
  73,193,234,32,                          //shr           $0x20,%r10
  243,66,15,16,28,145,                    //movss         (%rcx,%r10,4),%xmm3
  243,66,15,16,12,129,                    //movss         (%rcx,%r8,4),%xmm1
  15,20,217,                              //unpcklps      %xmm1,%xmm3
  243,66,15,16,12,153,                    //movss         (%rcx,%r11,4),%xmm1
  243,66,15,16,20,137,                    //movss         (%rcx,%r9,4),%xmm2
  15,20,202,                              //unpcklps      %xmm2,%xmm1
  15,20,203,                              //unpcklps      %xmm3,%xmm1
  72,139,64,24,                           //mov           0x18(%rax),%rax
  102,65,15,112,210,78,                   //pshufd        $0x4e,%xmm10,%xmm2
  102,72,15,126,209,                      //movq          %xmm2,%rcx
  65,137,200,                             //mov           %ecx,%r8d
  72,193,233,32,                          //shr           $0x20,%rcx
  102,77,15,126,209,                      //movq          %xmm10,%r9
  69,137,202,                             //mov           %r9d,%r10d
  73,193,233,32,                          //shr           $0x20,%r9
  243,70,15,16,12,136,                    //movss         (%rax,%r9,4),%xmm9
  243,15,16,20,136,                       //movss         (%rax,%rcx,4),%xmm2
  68,15,20,202,                           //unpcklps      %xmm2,%xmm9
  243,66,15,16,20,144,                    //movss         (%rax,%r10,4),%xmm2
  243,66,15,16,28,128,                    //movss         (%rax,%r8,4),%xmm3
  15,20,211,                              //unpcklps      %xmm3,%xmm2
  65,15,20,209,                           //unpcklps      %xmm9,%xmm2
  102,65,15,114,208,24,                   //psrld         $0x18,%xmm8
  69,15,91,192,                           //cvtdq2ps      %xmm8,%xmm8
  243,15,16,90,12,                        //movss         0xc(%rdx),%xmm3
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_a8_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,15,110,4,56,                        //movd          (%rax,%rdi,1),%xmm0
  102,15,239,201,                         //pxor          %xmm1,%xmm1
  102,15,96,193,                          //punpcklbw     %xmm1,%xmm0
  102,15,97,193,                          //punpcklwd     %xmm1,%xmm0
  15,91,192,                              //cvtdq2ps      %xmm0,%xmm0
  243,15,16,90,12,                        //movss         0xc(%rdx),%xmm3
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  15,89,216,                              //mulps         %xmm0,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,87,192,                              //xorps         %xmm0,%xmm0
  102,15,239,201,                         //pxor          %xmm1,%xmm1
  15,87,210,                              //xorps         %xmm2,%xmm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_a8_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,68,15,16,66,8,                      //movss         0x8(%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,89,195,                           //mulps         %xmm3,%xmm8
  102,69,15,91,192,                       //cvtps2dq      %xmm8,%xmm8
  102,65,15,114,240,16,                   //pslld         $0x10,%xmm8
  102,65,15,114,224,16,                   //psrad         $0x10,%xmm8
  102,69,15,107,192,                      //packssdw      %xmm8,%xmm8
  102,69,15,103,192,                      //packuswb      %xmm8,%xmm8
  102,68,15,126,4,56,                     //movd          %xmm8,(%rax,%rdi,1)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_565_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,68,15,126,12,120,                   //movq          (%rax,%rdi,2),%xmm9
  102,15,239,192,                         //pxor          %xmm0,%xmm0
  102,68,15,97,200,                       //punpcklwd     %xmm0,%xmm9
  102,15,110,66,104,                      //movd          0x68(%rdx),%xmm0
  102,15,112,192,0,                       //pshufd        $0x0,%xmm0,%xmm0
  102,65,15,219,193,                      //pand          %xmm9,%xmm0
  15,91,200,                              //cvtdq2ps      %xmm0,%xmm1
  243,15,16,26,                           //movss         (%rdx),%xmm3
  243,15,16,66,116,                       //movss         0x74(%rdx),%xmm0
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  15,89,193,                              //mulps         %xmm1,%xmm0
  102,15,110,74,108,                      //movd          0x6c(%rdx),%xmm1
  102,15,112,201,0,                       //pshufd        $0x0,%xmm1,%xmm1
  102,65,15,219,201,                      //pand          %xmm9,%xmm1
  68,15,91,193,                           //cvtdq2ps      %xmm1,%xmm8
  243,15,16,74,120,                       //movss         0x78(%rdx),%xmm1
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  102,15,110,82,112,                      //movd          0x70(%rdx),%xmm2
  102,15,112,210,0,                       //pshufd        $0x0,%xmm2,%xmm2
  102,65,15,219,209,                      //pand          %xmm9,%xmm2
  68,15,91,194,                           //cvtdq2ps      %xmm2,%xmm8
  243,15,16,82,124,                       //movss         0x7c(%rdx),%xmm2
  15,198,210,0,                           //shufps        $0x0,%xmm2,%xmm2
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_565_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,68,15,16,130,128,0,0,0,             //movss         0x80(%rdx),%xmm8
  243,68,15,16,138,132,0,0,0,             //movss         0x84(%rdx),%xmm9
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  69,15,40,208,                           //movaps        %xmm8,%xmm10
  68,15,89,208,                           //mulps         %xmm0,%xmm10
  102,69,15,91,210,                       //cvtps2dq      %xmm10,%xmm10
  102,65,15,114,242,11,                   //pslld         $0xb,%xmm10
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  68,15,89,201,                           //mulps         %xmm1,%xmm9
  102,69,15,91,201,                       //cvtps2dq      %xmm9,%xmm9
  102,65,15,114,241,5,                    //pslld         $0x5,%xmm9
  102,69,15,235,202,                      //por           %xmm10,%xmm9
  68,15,89,194,                           //mulps         %xmm2,%xmm8
  102,69,15,91,192,                       //cvtps2dq      %xmm8,%xmm8
  102,69,15,86,193,                       //orpd          %xmm9,%xmm8
  102,65,15,114,240,16,                   //pslld         $0x10,%xmm8
  102,65,15,114,224,16,                   //psrad         $0x10,%xmm8
  102,69,15,107,192,                      //packssdw      %xmm8,%xmm8
  102,68,15,214,4,120,                    //movq          %xmm8,(%rax,%rdi,2)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_8888_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,15,111,28,184,                      //movdqu        (%rax,%rdi,4),%xmm3
  102,15,110,66,16,                       //movd          0x10(%rdx),%xmm0
  102,15,112,192,0,                       //pshufd        $0x0,%xmm0,%xmm0
  102,15,111,203,                         //movdqa        %xmm3,%xmm1
  102,15,114,209,8,                       //psrld         $0x8,%xmm1
  102,15,219,200,                         //pand          %xmm0,%xmm1
  102,15,111,211,                         //movdqa        %xmm3,%xmm2
  102,15,114,210,16,                      //psrld         $0x10,%xmm2
  102,15,219,208,                         //pand          %xmm0,%xmm2
  102,15,219,195,                         //pand          %xmm3,%xmm0
  15,91,192,                              //cvtdq2ps      %xmm0,%xmm0
  243,68,15,16,66,12,                     //movss         0xc(%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  15,91,201,                              //cvtdq2ps      %xmm1,%xmm1
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  15,91,210,                              //cvtdq2ps      %xmm2,%xmm2
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  102,15,114,211,24,                      //psrld         $0x18,%xmm3
  15,91,219,                              //cvtdq2ps      %xmm3,%xmm3
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_8888_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,68,15,16,66,8,                      //movss         0x8(%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,200,                           //mulps         %xmm0,%xmm9
  102,69,15,91,201,                       //cvtps2dq      %xmm9,%xmm9
  69,15,40,208,                           //movaps        %xmm8,%xmm10
  68,15,89,209,                           //mulps         %xmm1,%xmm10
  102,69,15,91,210,                       //cvtps2dq      %xmm10,%xmm10
  102,65,15,114,242,8,                    //pslld         $0x8,%xmm10
  102,69,15,235,209,                      //por           %xmm9,%xmm10
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,202,                           //mulps         %xmm2,%xmm9
  102,69,15,91,201,                       //cvtps2dq      %xmm9,%xmm9
  102,65,15,114,241,16,                   //pslld         $0x10,%xmm9
  68,15,89,195,                           //mulps         %xmm3,%xmm8
  102,69,15,91,192,                       //cvtps2dq      %xmm8,%xmm8
  102,65,15,114,240,24,                   //pslld         $0x18,%xmm8
  102,69,15,235,193,                      //por           %xmm9,%xmm8
  102,69,15,235,194,                      //por           %xmm10,%xmm8
  243,68,15,127,4,184,                    //movdqu        %xmm8,(%rax,%rdi,4)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_f16_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,15,111,4,248,                       //movdqu        (%rax,%rdi,8),%xmm0
  243,15,111,76,248,16,                   //movdqu        0x10(%rax,%rdi,8),%xmm1
  102,15,111,208,                         //movdqa        %xmm0,%xmm2
  102,15,97,209,                          //punpcklwd     %xmm1,%xmm2
  102,15,105,193,                         //punpckhwd     %xmm1,%xmm0
  102,68,15,111,194,                      //movdqa        %xmm2,%xmm8
  102,68,15,97,192,                       //punpcklwd     %xmm0,%xmm8
  102,15,105,208,                         //punpckhwd     %xmm0,%xmm2
  102,15,110,66,100,                      //movd          0x64(%rdx),%xmm0
  102,15,112,216,0,                       //pshufd        $0x0,%xmm0,%xmm3
  102,15,111,203,                         //movdqa        %xmm3,%xmm1
  102,65,15,101,200,                      //pcmpgtw       %xmm8,%xmm1
  102,65,15,223,200,                      //pandn         %xmm8,%xmm1
  102,15,101,218,                         //pcmpgtw       %xmm2,%xmm3
  102,15,223,218,                         //pandn         %xmm2,%xmm3
  102,69,15,239,192,                      //pxor          %xmm8,%xmm8
  102,15,111,193,                         //movdqa        %xmm1,%xmm0
  102,65,15,97,192,                       //punpcklwd     %xmm8,%xmm0
  102,15,114,240,13,                      //pslld         $0xd,%xmm0
  102,15,110,82,92,                       //movd          0x5c(%rdx),%xmm2
  102,68,15,112,202,0,                    //pshufd        $0x0,%xmm2,%xmm9
  65,15,89,193,                           //mulps         %xmm9,%xmm0
  102,65,15,105,200,                      //punpckhwd     %xmm8,%xmm1
  102,15,114,241,13,                      //pslld         $0xd,%xmm1
  65,15,89,201,                           //mulps         %xmm9,%xmm1
  102,15,111,211,                         //movdqa        %xmm3,%xmm2
  102,65,15,97,208,                       //punpcklwd     %xmm8,%xmm2
  102,15,114,242,13,                      //pslld         $0xd,%xmm2
  65,15,89,209,                           //mulps         %xmm9,%xmm2
  102,65,15,105,216,                      //punpckhwd     %xmm8,%xmm3
  102,15,114,243,13,                      //pslld         $0xd,%xmm3
  65,15,89,217,                           //mulps         %xmm9,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_f16_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,68,15,110,66,96,                    //movd          0x60(%rdx),%xmm8
  102,69,15,112,192,0,                    //pshufd        $0x0,%xmm8,%xmm8
  102,69,15,111,200,                      //movdqa        %xmm8,%xmm9
  68,15,89,200,                           //mulps         %xmm0,%xmm9
  102,65,15,114,209,13,                   //psrld         $0xd,%xmm9
  102,69,15,111,208,                      //movdqa        %xmm8,%xmm10
  68,15,89,209,                           //mulps         %xmm1,%xmm10
  102,65,15,114,210,13,                   //psrld         $0xd,%xmm10
  102,69,15,111,216,                      //movdqa        %xmm8,%xmm11
  68,15,89,218,                           //mulps         %xmm2,%xmm11
  102,65,15,114,211,13,                   //psrld         $0xd,%xmm11
  68,15,89,195,                           //mulps         %xmm3,%xmm8
  102,65,15,114,208,13,                   //psrld         $0xd,%xmm8
  102,65,15,115,250,2,                    //pslldq        $0x2,%xmm10
  102,69,15,235,209,                      //por           %xmm9,%xmm10
  102,65,15,115,248,2,                    //pslldq        $0x2,%xmm8
  102,69,15,235,195,                      //por           %xmm11,%xmm8
  102,69,15,111,202,                      //movdqa        %xmm10,%xmm9
  102,69,15,98,200,                       //punpckldq     %xmm8,%xmm9
  243,68,15,127,12,248,                   //movdqu        %xmm9,(%rax,%rdi,8)
  102,69,15,106,208,                      //punpckhdq     %xmm8,%xmm10
  243,68,15,127,84,248,16,                //movdqu        %xmm10,0x10(%rax,%rdi,8)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_f32_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,137,249,                             //mov           %rdi,%rcx
  72,193,225,4,                           //shl           $0x4,%rcx
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  68,15,40,200,                           //movaps        %xmm0,%xmm9
  68,15,20,201,                           //unpcklps      %xmm1,%xmm9
  68,15,40,210,                           //movaps        %xmm2,%xmm10
  68,15,40,218,                           //movaps        %xmm2,%xmm11
  68,15,20,219,                           //unpcklps      %xmm3,%xmm11
  68,15,21,193,                           //unpckhps      %xmm1,%xmm8
  68,15,21,211,                           //unpckhps      %xmm3,%xmm10
  69,15,40,225,                           //movaps        %xmm9,%xmm12
  102,69,15,20,227,                       //unpcklpd      %xmm11,%xmm12
  102,69,15,21,203,                       //unpckhpd      %xmm11,%xmm9
  69,15,40,216,                           //movaps        %xmm8,%xmm11
  102,69,15,20,218,                       //unpcklpd      %xmm10,%xmm11
  102,69,15,21,194,                       //unpckhpd      %xmm10,%xmm8
  102,68,15,17,36,8,                      //movupd        %xmm12,(%rax,%rcx,1)
  102,68,15,17,76,8,16,                   //movupd        %xmm9,0x10(%rax,%rcx,1)
  102,68,15,17,92,8,32,                   //movupd        %xmm11,0x20(%rax,%rcx,1)
  102,68,15,17,68,8,48,                   //movupd        %xmm8,0x30(%rax,%rcx,1)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_x_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  68,15,95,192,                           //maxps         %xmm0,%xmm8
  243,68,15,16,8,                         //movss         (%rax),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  102,15,118,192,                         //pcmpeqd       %xmm0,%xmm0
  102,65,15,254,193,                      //paddd         %xmm9,%xmm0
  68,15,93,192,                           //minps         %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,192,                           //movaps        %xmm8,%xmm0
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_y_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  68,15,95,193,                           //maxps         %xmm1,%xmm8
  243,68,15,16,8,                         //movss         (%rax),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  102,15,118,201,                         //pcmpeqd       %xmm1,%xmm1
  102,65,15,254,201,                      //paddd         %xmm9,%xmm1
  68,15,93,193,                           //minps         %xmm1,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,200,                           //movaps        %xmm8,%xmm1
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_repeat_x_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,40,200,                           //movaps        %xmm0,%xmm9
  69,15,94,200,                           //divps         %xmm8,%xmm9
  243,69,15,91,209,                       //cvttps2dq     %xmm9,%xmm10
  69,15,91,210,                           //cvtdq2ps      %xmm10,%xmm10
  69,15,194,202,1,                        //cmpltps       %xmm10,%xmm9
  243,68,15,16,26,                        //movss         (%rdx),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,84,217,                           //andps         %xmm9,%xmm11
  69,15,92,211,                           //subps         %xmm11,%xmm10
  69,15,89,208,                           //mulps         %xmm8,%xmm10
  65,15,92,194,                           //subps         %xmm10,%xmm0
  102,69,15,118,201,                      //pcmpeqd       %xmm9,%xmm9
  102,69,15,254,200,                      //paddd         %xmm8,%xmm9
  65,15,93,193,                           //minps         %xmm9,%xmm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_repeat_y_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,40,201,                           //movaps        %xmm1,%xmm9
  69,15,94,200,                           //divps         %xmm8,%xmm9
  243,69,15,91,209,                       //cvttps2dq     %xmm9,%xmm10
  69,15,91,210,                           //cvtdq2ps      %xmm10,%xmm10
  69,15,194,202,1,                        //cmpltps       %xmm10,%xmm9
  243,68,15,16,26,                        //movss         (%rdx),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,84,217,                           //andps         %xmm9,%xmm11
  69,15,92,211,                           //subps         %xmm11,%xmm10
  69,15,89,208,                           //mulps         %xmm8,%xmm10
  65,15,92,202,                           //subps         %xmm10,%xmm1
  102,69,15,118,201,                      //pcmpeqd       %xmm9,%xmm9
  102,69,15,254,200,                      //paddd         %xmm8,%xmm9
  65,15,93,201,                           //minps         %xmm9,%xmm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_mirror_x_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,8,                         //movss         (%rax),%xmm9
  69,15,40,193,                           //movaps        %xmm9,%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,92,192,                           //subps         %xmm8,%xmm0
  243,69,15,88,201,                       //addss         %xmm9,%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  68,15,40,208,                           //movaps        %xmm0,%xmm10
  69,15,94,209,                           //divps         %xmm9,%xmm10
  243,69,15,91,218,                       //cvttps2dq     %xmm10,%xmm11
  69,15,91,219,                           //cvtdq2ps      %xmm11,%xmm11
  69,15,194,211,1,                        //cmpltps       %xmm11,%xmm10
  243,68,15,16,34,                        //movss         (%rdx),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  69,15,84,226,                           //andps         %xmm10,%xmm12
  69,15,87,210,                           //xorps         %xmm10,%xmm10
  69,15,92,220,                           //subps         %xmm12,%xmm11
  69,15,89,217,                           //mulps         %xmm9,%xmm11
  65,15,92,195,                           //subps         %xmm11,%xmm0
  65,15,92,192,                           //subps         %xmm8,%xmm0
  68,15,92,208,                           //subps         %xmm0,%xmm10
  65,15,84,194,                           //andps         %xmm10,%xmm0
  102,69,15,118,201,                      //pcmpeqd       %xmm9,%xmm9
  102,69,15,254,200,                      //paddd         %xmm8,%xmm9
  65,15,93,193,                           //minps         %xmm9,%xmm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_mirror_y_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,8,                         //movss         (%rax),%xmm9
  69,15,40,193,                           //movaps        %xmm9,%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,92,200,                           //subps         %xmm8,%xmm1
  243,69,15,88,201,                       //addss         %xmm9,%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  68,15,40,209,                           //movaps        %xmm1,%xmm10
  69,15,94,209,                           //divps         %xmm9,%xmm10
  243,69,15,91,218,                       //cvttps2dq     %xmm10,%xmm11
  69,15,91,219,                           //cvtdq2ps      %xmm11,%xmm11
  69,15,194,211,1,                        //cmpltps       %xmm11,%xmm10
  243,68,15,16,34,                        //movss         (%rdx),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  69,15,84,226,                           //andps         %xmm10,%xmm12
  69,15,87,210,                           //xorps         %xmm10,%xmm10
  69,15,92,220,                           //subps         %xmm12,%xmm11
  69,15,89,217,                           //mulps         %xmm9,%xmm11
  65,15,92,203,                           //subps         %xmm11,%xmm1
  65,15,92,200,                           //subps         %xmm8,%xmm1
  68,15,92,209,                           //subps         %xmm1,%xmm10
  65,15,84,202,                           //andps         %xmm10,%xmm1
  102,69,15,118,201,                      //pcmpeqd       %xmm9,%xmm9
  102,69,15,254,200,                      //paddd         %xmm8,%xmm9
  65,15,93,201,                           //minps         %xmm9,%xmm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_2x3_sse2[] = {
  68,15,40,201,                           //movaps        %xmm1,%xmm9
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,15,16,0,                            //movss         (%rax),%xmm0
  243,15,16,72,4,                         //movss         0x4(%rax),%xmm1
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  243,68,15,16,80,8,                      //movss         0x8(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,16,                     //movss         0x10(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,88,194,                           //addps         %xmm10,%xmm0
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  243,68,15,16,80,12,                     //movss         0xc(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,20,                     //movss         0x14(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  65,15,88,202,                           //addps         %xmm10,%xmm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_3x4_sse2[] = {
  68,15,40,201,                           //movaps        %xmm1,%xmm9
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,15,16,0,                            //movss         (%rax),%xmm0
  243,15,16,72,4,                         //movss         0x4(%rax),%xmm1
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  243,68,15,16,80,12,                     //movss         0xc(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,24,                     //movss         0x18(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,96,36,                     //movss         0x24(%rax),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  68,15,89,218,                           //mulps         %xmm2,%xmm11
  69,15,88,220,                           //addps         %xmm12,%xmm11
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,88,194,                           //addps         %xmm10,%xmm0
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  243,68,15,16,80,16,                     //movss         0x10(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,28,                     //movss         0x1c(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,96,40,                     //movss         0x28(%rax),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  68,15,89,218,                           //mulps         %xmm2,%xmm11
  69,15,88,220,                           //addps         %xmm12,%xmm11
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  65,15,88,202,                           //addps         %xmm10,%xmm1
  243,68,15,16,80,8,                      //movss         0x8(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,20,                     //movss         0x14(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,96,32,                     //movss         0x20(%rax),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  243,68,15,16,104,44,                    //movss         0x2c(%rax),%xmm13
  69,15,198,237,0,                        //shufps        $0x0,%xmm13,%xmm13
  68,15,89,226,                           //mulps         %xmm2,%xmm12
  69,15,88,229,                           //addps         %xmm13,%xmm12
  69,15,89,217,                           //mulps         %xmm9,%xmm11
  69,15,88,220,                           //addps         %xmm12,%xmm11
  69,15,89,208,                           //mulps         %xmm8,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,210,                           //movaps        %xmm10,%xmm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_perspective_sse2[] = {
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,15,16,0,                            //movss         (%rax),%xmm0
  243,68,15,16,72,4,                      //movss         0x4(%rax),%xmm9
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  243,68,15,16,80,8,                      //movss         0x8(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  68,15,89,201,                           //mulps         %xmm1,%xmm9
  69,15,88,202,                           //addps         %xmm10,%xmm9
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,88,193,                           //addps         %xmm9,%xmm0
  243,68,15,16,72,12,                     //movss         0xc(%rax),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  243,68,15,16,80,16,                     //movss         0x10(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,20,                     //movss         0x14(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  68,15,89,209,                           //mulps         %xmm1,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  69,15,88,202,                           //addps         %xmm10,%xmm9
  243,68,15,16,80,24,                     //movss         0x18(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,28,                     //movss         0x1c(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,96,32,                     //movss         0x20(%rax),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  68,15,89,217,                           //mulps         %xmm1,%xmm11
  69,15,88,220,                           //addps         %xmm12,%xmm11
  69,15,89,208,                           //mulps         %xmm8,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,83,202,                           //rcpps         %xmm10,%xmm1
  15,89,193,                              //mulps         %xmm1,%xmm0
  68,15,89,201,                           //mulps         %xmm1,%xmm9
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,201,                           //movaps        %xmm9,%xmm1
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_linear_gradient_2stops_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  68,15,16,8,                             //movups        (%rax),%xmm9
  15,16,88,16,                            //movups        0x10(%rax),%xmm3
  68,15,40,195,                           //movaps        %xmm3,%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,40,201,                           //movaps        %xmm9,%xmm1
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  68,15,89,192,                           //mulps         %xmm0,%xmm8
  68,15,88,193,                           //addps         %xmm1,%xmm8
  15,40,203,                              //movaps        %xmm3,%xmm1
  15,198,201,85,                          //shufps        $0x55,%xmm1,%xmm1
  65,15,40,209,                           //movaps        %xmm9,%xmm2
  15,198,210,85,                          //shufps        $0x55,%xmm2,%xmm2
  15,89,200,                              //mulps         %xmm0,%xmm1
  15,88,202,                              //addps         %xmm2,%xmm1
  15,40,211,                              //movaps        %xmm3,%xmm2
  15,198,210,170,                         //shufps        $0xaa,%xmm2,%xmm2
  69,15,40,209,                           //movaps        %xmm9,%xmm10
  69,15,198,210,170,                      //shufps        $0xaa,%xmm10,%xmm10
  15,89,208,                              //mulps         %xmm0,%xmm2
  65,15,88,210,                           //addps         %xmm10,%xmm2
  15,198,219,255,                         //shufps        $0xff,%xmm3,%xmm3
  69,15,198,201,255,                      //shufps        $0xff,%xmm9,%xmm9
  15,89,216,                              //mulps         %xmm0,%xmm3
  65,15,88,217,                           //addps         %xmm9,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,192,                           //movaps        %xmm8,%xmm0
  255,224,                                //jmpq          *%rax
};
#elif defined(_M_X64)

CODE const uint8_t sk_start_pipeline_hsw[] = {
  65,87,                                  //push          %r15
  65,86,                                  //push          %r14
  65,85,                                  //push          %r13
  65,84,                                  //push          %r12
  86,                                     //push          %rsi
  87,                                     //push          %rdi
  83,                                     //push          %rbx
  72,129,236,160,0,0,0,                   //sub           $0xa0,%rsp
  197,120,41,188,36,144,0,0,0,            //vmovaps       %xmm15,0x90(%rsp)
  197,120,41,180,36,128,0,0,0,            //vmovaps       %xmm14,0x80(%rsp)
  197,120,41,108,36,112,                  //vmovaps       %xmm13,0x70(%rsp)
  197,120,41,100,36,96,                   //vmovaps       %xmm12,0x60(%rsp)
  197,120,41,92,36,80,                    //vmovaps       %xmm11,0x50(%rsp)
  197,120,41,84,36,64,                    //vmovaps       %xmm10,0x40(%rsp)
  197,120,41,76,36,48,                    //vmovaps       %xmm9,0x30(%rsp)
  197,120,41,68,36,32,                    //vmovaps       %xmm8,0x20(%rsp)
  197,248,41,124,36,16,                   //vmovaps       %xmm7,0x10(%rsp)
  197,248,41,52,36,                       //vmovaps       %xmm6,(%rsp)
  77,137,205,                             //mov           %r9,%r13
  77,137,198,                             //mov           %r8,%r14
  72,137,203,                             //mov           %rcx,%rbx
  72,137,214,                             //mov           %rdx,%rsi
  72,173,                                 //lods          %ds:(%rsi),%rax
  73,137,199,                             //mov           %rax,%r15
  73,137,244,                             //mov           %rsi,%r12
  72,141,67,8,                            //lea           0x8(%rbx),%rax
  76,57,232,                              //cmp           %r13,%rax
  118,5,                                  //jbe           75 <_sk_start_pipeline_hsw+0x75>
  72,137,223,                             //mov           %rbx,%rdi
  235,65,                                 //jmp           b6 <_sk_start_pipeline_hsw+0xb6>
  185,0,0,0,0,                            //mov           $0x0,%ecx
  197,252,87,192,                         //vxorps        %ymm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  197,236,87,210,                         //vxorps        %ymm2,%ymm2,%ymm2
  197,228,87,219,                         //vxorps        %ymm3,%ymm3,%ymm3
  197,220,87,228,                         //vxorps        %ymm4,%ymm4,%ymm4
  197,212,87,237,                         //vxorps        %ymm5,%ymm5,%ymm5
  197,204,87,246,                         //vxorps        %ymm6,%ymm6,%ymm6
  197,196,87,255,                         //vxorps        %ymm7,%ymm7,%ymm7
  72,137,223,                             //mov           %rbx,%rdi
  76,137,230,                             //mov           %r12,%rsi
  76,137,242,                             //mov           %r14,%rdx
  65,255,215,                             //callq         *%r15
  72,141,123,8,                           //lea           0x8(%rbx),%rdi
  72,131,195,16,                          //add           $0x10,%rbx
  76,57,235,                              //cmp           %r13,%rbx
  72,137,251,                             //mov           %rdi,%rbx
  118,191,                                //jbe           75 <_sk_start_pipeline_hsw+0x75>
  76,137,233,                             //mov           %r13,%rcx
  72,41,249,                              //sub           %rdi,%rcx
  116,41,                                 //je            e7 <_sk_start_pipeline_hsw+0xe7>
  197,252,87,192,                         //vxorps        %ymm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  197,236,87,210,                         //vxorps        %ymm2,%ymm2,%ymm2
  197,228,87,219,                         //vxorps        %ymm3,%ymm3,%ymm3
  197,220,87,228,                         //vxorps        %ymm4,%ymm4,%ymm4
  197,212,87,237,                         //vxorps        %ymm5,%ymm5,%ymm5
  197,204,87,246,                         //vxorps        %ymm6,%ymm6,%ymm6
  197,196,87,255,                         //vxorps        %ymm7,%ymm7,%ymm7
  76,137,230,                             //mov           %r12,%rsi
  76,137,242,                             //mov           %r14,%rdx
  65,255,215,                             //callq         *%r15
  76,137,232,                             //mov           %r13,%rax
  197,248,40,52,36,                       //vmovaps       (%rsp),%xmm6
  197,248,40,124,36,16,                   //vmovaps       0x10(%rsp),%xmm7
  197,120,40,68,36,32,                    //vmovaps       0x20(%rsp),%xmm8
  197,120,40,76,36,48,                    //vmovaps       0x30(%rsp),%xmm9
  197,120,40,84,36,64,                    //vmovaps       0x40(%rsp),%xmm10
  197,120,40,92,36,80,                    //vmovaps       0x50(%rsp),%xmm11
  197,120,40,100,36,96,                   //vmovaps       0x60(%rsp),%xmm12
  197,120,40,108,36,112,                  //vmovaps       0x70(%rsp),%xmm13
  197,120,40,180,36,128,0,0,0,            //vmovaps       0x80(%rsp),%xmm14
  197,120,40,188,36,144,0,0,0,            //vmovaps       0x90(%rsp),%xmm15
  72,129,196,160,0,0,0,                   //add           $0xa0,%rsp
  91,                                     //pop           %rbx
  95,                                     //pop           %rdi
  94,                                     //pop           %rsi
  65,92,                                  //pop           %r12
  65,93,                                  //pop           %r13
  65,94,                                  //pop           %r14
  65,95,                                  //pop           %r15
  197,248,119,                            //vzeroupper
  195,                                    //retq
};

CODE const uint8_t sk_just_return_hsw[] = {
  195,                                    //retq
};

CODE const uint8_t sk_seed_shader_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,249,110,199,                        //vmovd         %edi,%xmm0
  196,226,125,24,192,                     //vbroadcastss  %xmm0,%ymm0
  197,252,91,192,                         //vcvtdq2ps     %ymm0,%ymm0
  196,226,125,24,74,4,                    //vbroadcastss  0x4(%rdx),%ymm1
  197,252,88,193,                         //vaddps        %ymm1,%ymm0,%ymm0
  197,252,88,66,20,                       //vaddps        0x14(%rdx),%ymm0,%ymm0
  196,226,125,24,16,                      //vbroadcastss  (%rax),%ymm2
  197,252,91,210,                         //vcvtdq2ps     %ymm2,%ymm2
  197,236,88,201,                         //vaddps        %ymm1,%ymm2,%ymm1
  196,226,125,24,18,                      //vbroadcastss  (%rdx),%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,228,87,219,                         //vxorps        %ymm3,%ymm3,%ymm3
  197,220,87,228,                         //vxorps        %ymm4,%ymm4,%ymm4
  197,212,87,237,                         //vxorps        %ymm5,%ymm5,%ymm5
  197,204,87,246,                         //vxorps        %ymm6,%ymm6,%ymm6
  197,196,87,255,                         //vxorps        %ymm7,%ymm7,%ymm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_constant_color_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,226,125,24,0,                       //vbroadcastss  (%rax),%ymm0
  196,226,125,24,72,4,                    //vbroadcastss  0x4(%rax),%ymm1
  196,226,125,24,80,8,                    //vbroadcastss  0x8(%rax),%ymm2
  196,226,125,24,88,12,                   //vbroadcastss  0xc(%rax),%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clear_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,87,192,                         //vxorps        %ymm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  197,236,87,210,                         //vxorps        %ymm2,%ymm2,%ymm2
  197,228,87,219,                         //vxorps        %ymm3,%ymm3,%ymm3
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_plus__hsw[] = {
  197,252,88,196,                         //vaddps        %ymm4,%ymm0,%ymm0
  197,244,88,205,                         //vaddps        %ymm5,%ymm1,%ymm1
  197,236,88,214,                         //vaddps        %ymm6,%ymm2,%ymm2
  197,228,88,223,                         //vaddps        %ymm7,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_srcover_hsw[] = {
  196,98,125,24,2,                        //vbroadcastss  (%rdx),%ymm8
  197,60,92,195,                          //vsubps        %ymm3,%ymm8,%ymm8
  196,194,93,184,192,                     //vfmadd231ps   %ymm8,%ymm4,%ymm0
  196,194,85,184,200,                     //vfmadd231ps   %ymm8,%ymm5,%ymm1
  196,194,77,184,208,                     //vfmadd231ps   %ymm8,%ymm6,%ymm2
  196,194,69,184,216,                     //vfmadd231ps   %ymm8,%ymm7,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_dstover_hsw[] = {
  196,98,125,24,2,                        //vbroadcastss  (%rdx),%ymm8
  197,60,92,199,                          //vsubps        %ymm7,%ymm8,%ymm8
  196,226,61,168,196,                     //vfmadd213ps   %ymm4,%ymm8,%ymm0
  196,226,61,168,205,                     //vfmadd213ps   %ymm5,%ymm8,%ymm1
  196,226,61,168,214,                     //vfmadd213ps   %ymm6,%ymm8,%ymm2
  196,226,61,168,223,                     //vfmadd213ps   %ymm7,%ymm8,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_0_hsw[] = {
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  196,193,124,95,192,                     //vmaxps        %ymm8,%ymm0,%ymm0
  196,193,116,95,200,                     //vmaxps        %ymm8,%ymm1,%ymm1
  196,193,108,95,208,                     //vmaxps        %ymm8,%ymm2,%ymm2
  196,193,100,95,216,                     //vmaxps        %ymm8,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_1_hsw[] = {
  196,98,125,24,2,                        //vbroadcastss  (%rdx),%ymm8
  196,193,124,93,192,                     //vminps        %ymm8,%ymm0,%ymm0
  196,193,116,93,200,                     //vminps        %ymm8,%ymm1,%ymm1
  196,193,108,93,208,                     //vminps        %ymm8,%ymm2,%ymm2
  196,193,100,93,216,                     //vminps        %ymm8,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_a_hsw[] = {
  196,98,125,24,2,                        //vbroadcastss  (%rdx),%ymm8
  196,193,100,93,216,                     //vminps        %ymm8,%ymm3,%ymm3
  197,252,93,195,                         //vminps        %ymm3,%ymm0,%ymm0
  197,244,93,203,                         //vminps        %ymm3,%ymm1,%ymm1
  197,236,93,211,                         //vminps        %ymm3,%ymm2,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_set_rgb_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,226,125,24,0,                       //vbroadcastss  (%rax),%ymm0
  196,226,125,24,72,4,                    //vbroadcastss  0x4(%rax),%ymm1
  196,226,125,24,80,8,                    //vbroadcastss  0x8(%rax),%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_swap_rb_hsw[] = {
  197,124,40,192,                         //vmovaps       %ymm0,%ymm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,40,194,                         //vmovaps       %ymm2,%ymm0
  197,124,41,194,                         //vmovaps       %ymm8,%ymm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_swap_hsw[] = {
  197,124,40,195,                         //vmovaps       %ymm3,%ymm8
  197,124,40,202,                         //vmovaps       %ymm2,%ymm9
  197,124,40,209,                         //vmovaps       %ymm1,%ymm10
  197,124,40,216,                         //vmovaps       %ymm0,%ymm11
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,40,196,                         //vmovaps       %ymm4,%ymm0
  197,252,40,205,                         //vmovaps       %ymm5,%ymm1
  197,252,40,214,                         //vmovaps       %ymm6,%ymm2
  197,252,40,223,                         //vmovaps       %ymm7,%ymm3
  197,124,41,220,                         //vmovaps       %ymm11,%ymm4
  197,124,41,213,                         //vmovaps       %ymm10,%ymm5
  197,124,41,206,                         //vmovaps       %ymm9,%ymm6
  197,124,41,199,                         //vmovaps       %ymm8,%ymm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_move_src_dst_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,40,224,                         //vmovaps       %ymm0,%ymm4
  197,252,40,233,                         //vmovaps       %ymm1,%ymm5
  197,252,40,242,                         //vmovaps       %ymm2,%ymm6
  197,252,40,251,                         //vmovaps       %ymm3,%ymm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_move_dst_src_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,40,196,                         //vmovaps       %ymm4,%ymm0
  197,252,40,205,                         //vmovaps       %ymm5,%ymm1
  197,252,40,214,                         //vmovaps       %ymm6,%ymm2
  197,252,40,223,                         //vmovaps       %ymm7,%ymm3
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_premul_hsw[] = {
  197,252,89,195,                         //vmulps        %ymm3,%ymm0,%ymm0
  197,244,89,203,                         //vmulps        %ymm3,%ymm1,%ymm1
  197,236,89,211,                         //vmulps        %ymm3,%ymm2,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_unpremul_hsw[] = {
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  196,65,100,194,200,0,                   //vcmpeqps      %ymm8,%ymm3,%ymm9
  196,98,125,24,18,                       //vbroadcastss  (%rdx),%ymm10
  197,44,94,211,                          //vdivps        %ymm3,%ymm10,%ymm10
  196,67,45,74,192,144,                   //vblendvps     %ymm9,%ymm8,%ymm10,%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_from_srgb_hsw[] = {
  196,98,125,24,66,64,                    //vbroadcastss  0x40(%rdx),%ymm8
  197,60,89,200,                          //vmulps        %ymm0,%ymm8,%ymm9
  197,124,89,208,                         //vmulps        %ymm0,%ymm0,%ymm10
  196,98,125,24,90,60,                    //vbroadcastss  0x3c(%rdx),%ymm11
  196,98,125,24,98,56,                    //vbroadcastss  0x38(%rdx),%ymm12
  196,65,124,40,235,                      //vmovaps       %ymm11,%ymm13
  196,66,125,168,236,                     //vfmadd213ps   %ymm12,%ymm0,%ymm13
  196,98,125,24,114,52,                   //vbroadcastss  0x34(%rdx),%ymm14
  196,66,45,168,238,                      //vfmadd213ps   %ymm14,%ymm10,%ymm13
  196,98,125,24,82,68,                    //vbroadcastss  0x44(%rdx),%ymm10
  196,193,124,194,194,1,                  //vcmpltps      %ymm10,%ymm0,%ymm0
  196,195,21,74,193,0,                    //vblendvps     %ymm0,%ymm9,%ymm13,%ymm0
  197,60,89,201,                          //vmulps        %ymm1,%ymm8,%ymm9
  197,116,89,233,                         //vmulps        %ymm1,%ymm1,%ymm13
  196,65,124,40,251,                      //vmovaps       %ymm11,%ymm15
  196,66,117,168,252,                     //vfmadd213ps   %ymm12,%ymm1,%ymm15
  196,66,21,168,254,                      //vfmadd213ps   %ymm14,%ymm13,%ymm15
  196,193,116,194,202,1,                  //vcmpltps      %ymm10,%ymm1,%ymm1
  196,195,5,74,201,16,                    //vblendvps     %ymm1,%ymm9,%ymm15,%ymm1
  197,60,89,194,                          //vmulps        %ymm2,%ymm8,%ymm8
  197,108,89,202,                         //vmulps        %ymm2,%ymm2,%ymm9
  196,66,109,168,220,                     //vfmadd213ps   %ymm12,%ymm2,%ymm11
  196,66,53,168,222,                      //vfmadd213ps   %ymm14,%ymm9,%ymm11
  196,193,108,194,210,1,                  //vcmpltps      %ymm10,%ymm2,%ymm2
  196,195,37,74,208,32,                   //vblendvps     %ymm2,%ymm8,%ymm11,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_to_srgb_hsw[] = {
  197,124,82,192,                         //vrsqrtps      %ymm0,%ymm8
  196,65,124,83,200,                      //vrcpps        %ymm8,%ymm9
  196,65,124,82,208,                      //vrsqrtps      %ymm8,%ymm10
  196,98,125,24,66,72,                    //vbroadcastss  0x48(%rdx),%ymm8
  197,60,89,216,                          //vmulps        %ymm0,%ymm8,%ymm11
  196,98,125,24,34,                       //vbroadcastss  (%rdx),%ymm12
  196,98,125,24,106,76,                   //vbroadcastss  0x4c(%rdx),%ymm13
  196,98,125,24,114,80,                   //vbroadcastss  0x50(%rdx),%ymm14
  196,98,125,24,122,84,                   //vbroadcastss  0x54(%rdx),%ymm15
  196,66,13,168,207,                      //vfmadd213ps   %ymm15,%ymm14,%ymm9
  196,66,21,184,202,                      //vfmadd231ps   %ymm10,%ymm13,%ymm9
  196,65,28,93,201,                       //vminps        %ymm9,%ymm12,%ymm9
  196,98,125,24,82,88,                    //vbroadcastss  0x58(%rdx),%ymm10
  196,193,124,194,194,1,                  //vcmpltps      %ymm10,%ymm0,%ymm0
  196,195,53,74,195,0,                    //vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
  197,124,82,201,                         //vrsqrtps      %ymm1,%ymm9
  196,65,124,83,217,                      //vrcpps        %ymm9,%ymm11
  196,65,124,82,201,                      //vrsqrtps      %ymm9,%ymm9
  196,66,13,168,223,                      //vfmadd213ps   %ymm15,%ymm14,%ymm11
  196,66,21,184,217,                      //vfmadd231ps   %ymm9,%ymm13,%ymm11
  197,60,89,201,                          //vmulps        %ymm1,%ymm8,%ymm9
  196,65,28,93,219,                       //vminps        %ymm11,%ymm12,%ymm11
  196,193,116,194,202,1,                  //vcmpltps      %ymm10,%ymm1,%ymm1
  196,195,37,74,201,16,                   //vblendvps     %ymm1,%ymm9,%ymm11,%ymm1
  197,124,82,202,                         //vrsqrtps      %ymm2,%ymm9
  196,65,124,83,217,                      //vrcpps        %ymm9,%ymm11
  196,66,13,168,223,                      //vfmadd213ps   %ymm15,%ymm14,%ymm11
  196,65,124,82,201,                      //vrsqrtps      %ymm9,%ymm9
  196,66,21,184,217,                      //vfmadd231ps   %ymm9,%ymm13,%ymm11
  196,65,28,93,203,                       //vminps        %ymm11,%ymm12,%ymm9
  197,60,89,194,                          //vmulps        %ymm2,%ymm8,%ymm8
  196,193,108,194,210,1,                  //vcmpltps      %ymm10,%ymm2,%ymm2
  196,195,53,74,208,32,                   //vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_scale_1_float_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  197,188,89,219,                         //vmulps        %ymm3,%ymm8,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_scale_u8_hsw[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,1,248,                               //add           %rdi,%rax
  77,133,192,                             //test          %r8,%r8
  117,48,                                 //jne           4b1 <_sk_scale_u8_hsw+0x40>
  197,123,16,0,                           //vmovsd        (%rax),%xmm8
  196,66,125,49,192,                      //vpmovzxbd     %xmm8,%ymm8
  196,65,124,91,192,                      //vcvtdq2ps     %ymm8,%ymm8
  196,98,125,24,74,12,                    //vbroadcastss  0xc(%rdx),%ymm9
  196,65,60,89,193,                       //vmulps        %ymm9,%ymm8,%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  197,188,89,219,                         //vmulps        %ymm3,%ymm8,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  49,201,                                 //xor           %ecx,%ecx
  77,137,194,                             //mov           %r8,%r10
  69,49,201,                              //xor           %r9d,%r9d
  68,15,182,24,                           //movzbl        (%rax),%r11d
  72,255,192,                             //inc           %rax
  73,211,227,                             //shl           %cl,%r11
  77,9,217,                               //or            %r11,%r9
  72,131,193,8,                           //add           $0x8,%rcx
  73,255,202,                             //dec           %r10
  117,234,                                //jne           4b9 <_sk_scale_u8_hsw+0x48>
  196,65,249,110,193,                     //vmovq         %r9,%xmm8
  235,175,                                //jmp           485 <_sk_scale_u8_hsw+0x14>
};

CODE const uint8_t sk_lerp_1_float_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  197,252,92,196,                         //vsubps        %ymm4,%ymm0,%ymm0
  196,226,61,168,196,                     //vfmadd213ps   %ymm4,%ymm8,%ymm0
  197,244,92,205,                         //vsubps        %ymm5,%ymm1,%ymm1
  196,226,61,168,205,                     //vfmadd213ps   %ymm5,%ymm8,%ymm1
  197,236,92,214,                         //vsubps        %ymm6,%ymm2,%ymm2
  196,226,61,168,214,                     //vfmadd213ps   %ymm6,%ymm8,%ymm2
  197,228,92,223,                         //vsubps        %ymm7,%ymm3,%ymm3
  196,226,61,168,223,                     //vfmadd213ps   %ymm7,%ymm8,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_lerp_u8_hsw[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,1,248,                               //add           %rdi,%rax
  77,133,192,                             //test          %r8,%r8
  117,68,                                 //jne           559 <_sk_lerp_u8_hsw+0x54>
  197,123,16,0,                           //vmovsd        (%rax),%xmm8
  196,66,125,49,192,                      //vpmovzxbd     %xmm8,%ymm8
  196,65,124,91,192,                      //vcvtdq2ps     %ymm8,%ymm8
  196,98,125,24,74,12,                    //vbroadcastss  0xc(%rdx),%ymm9
  196,65,60,89,193,                       //vmulps        %ymm9,%ymm8,%ymm8
  197,252,92,196,                         //vsubps        %ymm4,%ymm0,%ymm0
  196,226,61,168,196,                     //vfmadd213ps   %ymm4,%ymm8,%ymm0
  197,244,92,205,                         //vsubps        %ymm5,%ymm1,%ymm1
  196,226,61,168,205,                     //vfmadd213ps   %ymm5,%ymm8,%ymm1
  197,236,92,214,                         //vsubps        %ymm6,%ymm2,%ymm2
  196,226,61,168,214,                     //vfmadd213ps   %ymm6,%ymm8,%ymm2
  197,228,92,223,                         //vsubps        %ymm7,%ymm3,%ymm3
  196,226,61,168,223,                     //vfmadd213ps   %ymm7,%ymm8,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  49,201,                                 //xor           %ecx,%ecx
  77,137,194,                             //mov           %r8,%r10
  69,49,201,                              //xor           %r9d,%r9d
  68,15,182,24,                           //movzbl        (%rax),%r11d
  72,255,192,                             //inc           %rax
  73,211,227,                             //shl           %cl,%r11
  77,9,217,                               //or            %r11,%r9
  72,131,193,8,                           //add           $0x8,%rcx
  73,255,202,                             //dec           %r10
  117,234,                                //jne           561 <_sk_lerp_u8_hsw+0x5c>
  196,65,249,110,193,                     //vmovq         %r9,%xmm8
  235,155,                                //jmp           519 <_sk_lerp_u8_hsw+0x14>
};

CODE const uint8_t sk_lerp_565_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,16,                              //mov           (%rax),%r10
  72,133,201,                             //test          %rcx,%rcx
  117,123,                                //jne           603 <_sk_lerp_565_hsw+0x85>
  196,193,122,111,28,122,                 //vmovdqu       (%r10,%rdi,2),%xmm3
  196,226,125,51,219,                     //vpmovzxwd     %xmm3,%ymm3
  196,98,125,88,66,104,                   //vpbroadcastd  0x68(%rdx),%ymm8
  197,61,219,195,                         //vpand         %ymm3,%ymm8,%ymm8
  196,65,124,91,192,                      //vcvtdq2ps     %ymm8,%ymm8
  196,98,125,24,74,116,                   //vbroadcastss  0x74(%rdx),%ymm9
  196,65,52,89,192,                       //vmulps        %ymm8,%ymm9,%ymm8
  196,98,125,88,74,108,                   //vpbroadcastd  0x6c(%rdx),%ymm9
  197,53,219,203,                         //vpand         %ymm3,%ymm9,%ymm9
  196,65,124,91,201,                      //vcvtdq2ps     %ymm9,%ymm9
  196,98,125,24,82,120,                   //vbroadcastss  0x78(%rdx),%ymm10
  196,65,44,89,201,                       //vmulps        %ymm9,%ymm10,%ymm9
  196,98,125,88,82,112,                   //vpbroadcastd  0x70(%rdx),%ymm10
  197,173,219,219,                        //vpand         %ymm3,%ymm10,%ymm3
  197,252,91,219,                         //vcvtdq2ps     %ymm3,%ymm3
  196,98,125,24,82,124,                   //vbroadcastss  0x7c(%rdx),%ymm10
  197,172,89,219,                         //vmulps        %ymm3,%ymm10,%ymm3
  197,252,92,196,                         //vsubps        %ymm4,%ymm0,%ymm0
  196,226,61,168,196,                     //vfmadd213ps   %ymm4,%ymm8,%ymm0
  197,244,92,205,                         //vsubps        %ymm5,%ymm1,%ymm1
  196,226,53,168,205,                     //vfmadd213ps   %ymm5,%ymm9,%ymm1
  197,236,92,214,                         //vsubps        %ymm6,%ymm2,%ymm2
  196,226,101,168,214,                    //vfmadd213ps   %ymm6,%ymm3,%ymm2
  196,226,125,24,26,                      //vbroadcastss  (%rdx),%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  65,137,200,                             //mov           %ecx,%r8d
  65,128,224,7,                           //and           $0x7,%r8b
  197,225,239,219,                        //vpxor         %xmm3,%xmm3,%xmm3
  65,254,200,                             //dec           %r8b
  69,15,182,192,                          //movzbl        %r8b,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  15,135,111,255,255,255,                 //ja            58e <_sk_lerp_565_hsw+0x10>
  76,141,13,74,0,0,0,                     //lea           0x4a(%rip),%r9        # 670 <_sk_lerp_565_hsw+0xf2>
  75,99,4,129,                            //movslq        (%r9,%r8,4),%rax
  76,1,200,                               //add           %r9,%rax
  255,224,                                //jmpq          *%rax
  197,225,239,219,                        //vpxor         %xmm3,%xmm3,%xmm3
  196,193,97,196,92,122,12,6,             //vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm3,%xmm3
  196,193,97,196,92,122,10,5,             //vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm3,%xmm3
  196,193,97,196,92,122,8,4,              //vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm3,%xmm3
  196,193,97,196,92,122,6,3,              //vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm3,%xmm3
  196,193,97,196,92,122,4,2,              //vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm3,%xmm3
  196,193,97,196,92,122,2,1,              //vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm3,%xmm3
  196,193,97,196,28,122,0,                //vpinsrw       $0x0,(%r10,%rdi,2),%xmm3,%xmm3
  233,31,255,255,255,                     //jmpq          58e <_sk_lerp_565_hsw+0x10>
  144,                                    //nop
  243,255,                                //repz          (bad)
  255,                                    //(bad)
  255,                                    //(bad)
  235,255,                                //jmp           675 <_sk_lerp_565_hsw+0xf7>
  255,                                    //(bad)
  255,227,                                //jmpq          *%rbx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  219,255,                                //(bad)
  255,                                    //(bad)
  255,211,                                //callq         *%rbx
  255,                                    //(bad)
  255,                                    //(bad)
  255,203,                                //dec           %ebx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  191,                                    //.byte         0xbf
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_load_tables_hsw[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,141,12,189,0,0,0,0,                  //lea           0x0(,%rdi,4),%r9
  76,3,8,                                 //add           (%rax),%r9
  77,133,192,                             //test          %r8,%r8
  117,106,                                //jne           70b <_sk_load_tables_hsw+0x7f>
  196,193,126,111,25,                     //vmovdqu       (%r9),%ymm3
  196,226,125,88,82,16,                   //vpbroadcastd  0x10(%rdx),%ymm2
  197,237,219,203,                        //vpand         %ymm3,%ymm2,%ymm1
  196,65,61,118,192,                      //vpcmpeqd      %ymm8,%ymm8,%ymm8
  72,139,72,8,                            //mov           0x8(%rax),%rcx
  76,139,72,16,                           //mov           0x10(%rax),%r9
  196,65,53,118,201,                      //vpcmpeqd      %ymm9,%ymm9,%ymm9
  196,226,53,146,4,137,                   //vgatherdps    %ymm9,(%rcx,%ymm1,4),%ymm0
  197,245,114,211,8,                      //vpsrld        $0x8,%ymm3,%ymm1
  197,109,219,201,                        //vpand         %ymm1,%ymm2,%ymm9
  196,65,45,118,210,                      //vpcmpeqd      %ymm10,%ymm10,%ymm10
  196,130,45,146,12,137,                  //vgatherdps    %ymm10,(%r9,%ymm9,4),%ymm1
  72,139,64,24,                           //mov           0x18(%rax),%rax
  197,181,114,211,16,                     //vpsrld        $0x10,%ymm3,%ymm9
  196,65,109,219,201,                     //vpand         %ymm9,%ymm2,%ymm9
  196,162,61,146,20,136,                  //vgatherdps    %ymm8,(%rax,%ymm9,4),%ymm2
  197,229,114,211,24,                     //vpsrld        $0x18,%ymm3,%ymm3
  197,252,91,219,                         //vcvtdq2ps     %ymm3,%ymm3
  196,98,125,24,66,12,                    //vbroadcastss  0xc(%rdx),%ymm8
  196,193,100,89,216,                     //vmulps        %ymm8,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  185,8,0,0,0,                            //mov           $0x8,%ecx
  68,41,193,                              //sub           %r8d,%ecx
  192,225,3,                              //shl           $0x3,%cl
  73,199,194,255,255,255,255,             //mov           $0xffffffffffffffff,%r10
  73,211,234,                             //shr           %cl,%r10
  196,193,249,110,194,                    //vmovq         %r10,%xmm0
  196,226,125,33,192,                     //vpmovsxbd     %xmm0,%ymm0
  196,194,125,140,25,                     //vpmaskmovd    (%r9),%ymm0,%ymm3
  233,114,255,255,255,                    //jmpq          6a6 <_sk_load_tables_hsw+0x1a>
};

CODE const uint8_t sk_load_a8_hsw[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,1,248,                               //add           %rdi,%rax
  77,133,192,                             //test          %r8,%r8
  117,42,                                 //jne           76e <_sk_load_a8_hsw+0x3a>
  197,251,16,0,                           //vmovsd        (%rax),%xmm0
  196,226,125,49,192,                     //vpmovzxbd     %xmm0,%ymm0
  197,252,91,192,                         //vcvtdq2ps     %ymm0,%ymm0
  196,226,125,24,74,12,                   //vbroadcastss  0xc(%rdx),%ymm1
  197,252,89,217,                         //vmulps        %ymm1,%ymm0,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,87,192,                         //vxorps        %ymm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  197,236,87,210,                         //vxorps        %ymm2,%ymm2,%ymm2
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  49,201,                                 //xor           %ecx,%ecx
  77,137,194,                             //mov           %r8,%r10
  69,49,201,                              //xor           %r9d,%r9d
  68,15,182,24,                           //movzbl        (%rax),%r11d
  72,255,192,                             //inc           %rax
  73,211,227,                             //shl           %cl,%r11
  77,9,217,                               //or            %r11,%r9
  72,131,193,8,                           //add           $0x8,%rcx
  73,255,202,                             //dec           %r10
  117,234,                                //jne           776 <_sk_load_a8_hsw+0x42>
  196,193,249,110,193,                    //vmovq         %r9,%xmm0
  235,181,                                //jmp           748 <_sk_load_a8_hsw+0x14>
};

CODE const uint8_t sk_store_a8_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,8,                               //mov           (%rax),%r9
  196,98,125,24,66,8,                     //vbroadcastss  0x8(%rdx),%ymm8
  197,60,89,195,                          //vmulps        %ymm3,%ymm8,%ymm8
  196,65,125,91,192,                      //vcvtps2dq     %ymm8,%ymm8
  196,67,125,25,193,1,                    //vextractf128  $0x1,%ymm8,%xmm9
  196,66,57,43,193,                       //vpackusdw     %xmm9,%xmm8,%xmm8
  196,65,57,103,192,                      //vpackuswb     %xmm8,%xmm8,%xmm8
  72,133,201,                             //test          %rcx,%rcx
  117,10,                                 //jne           7c6 <_sk_store_a8_hsw+0x33>
  196,65,123,17,4,57,                     //vmovsd        %xmm8,(%r9,%rdi,1)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  137,200,                                //mov           %ecx,%eax
  36,7,                                   //and           $0x7,%al
  254,200,                                //dec           %al
  68,15,182,192,                          //movzbl        %al,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  119,236,                                //ja            7c2 <_sk_store_a8_hsw+0x2f>
  196,66,121,48,192,                      //vpmovzxbw     %xmm8,%xmm8
  76,141,21,66,0,0,0,                     //lea           0x42(%rip),%r10        # 824 <_sk_store_a8_hsw+0x91>
  75,99,4,130,                            //movslq        (%r10,%r8,4),%rax
  76,1,208,                               //add           %r10,%rax
  255,224,                                //jmpq          *%rax
  196,67,121,20,68,57,6,12,               //vpextrb       $0xc,%xmm8,0x6(%r9,%rdi,1)
  196,67,121,20,68,57,5,10,               //vpextrb       $0xa,%xmm8,0x5(%r9,%rdi,1)
  196,67,121,20,68,57,4,8,                //vpextrb       $0x8,%xmm8,0x4(%r9,%rdi,1)
  196,67,121,20,68,57,3,6,                //vpextrb       $0x6,%xmm8,0x3(%r9,%rdi,1)
  196,67,121,20,68,57,2,4,                //vpextrb       $0x4,%xmm8,0x2(%r9,%rdi,1)
  196,67,121,20,68,57,1,2,                //vpextrb       $0x2,%xmm8,0x1(%r9,%rdi,1)
  196,67,121,20,4,57,0,                   //vpextrb       $0x0,%xmm8,(%r9,%rdi,1)
  235,158,                                //jmp           7c2 <_sk_store_a8_hsw+0x2f>
  247,255,                                //idiv          %edi
  255,                                    //(bad)
  255,                                    //(bad)
  239,                                    //out           %eax,(%dx)
  255,                                    //(bad)
  255,                                    //(bad)
  255,231,                                //jmpq          *%rdi
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  223,255,                                //(bad)
  255,                                    //(bad)
  255,215,                                //callq         *%rdi
  255,                                    //(bad)
  255,                                    //(bad)
  255,207,                                //dec           %edi
  255,                                    //(bad)
  255,                                    //(bad)
  255,199,                                //inc           %edi
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_load_565_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,16,                              //mov           (%rax),%r10
  72,133,201,                             //test          %rcx,%rcx
  117,92,                                 //jne           8a6 <_sk_load_565_hsw+0x66>
  196,193,122,111,4,122,                  //vmovdqu       (%r10,%rdi,2),%xmm0
  196,226,125,51,208,                     //vpmovzxwd     %xmm0,%ymm2
  196,226,125,88,66,104,                  //vpbroadcastd  0x68(%rdx),%ymm0
  197,253,219,194,                        //vpand         %ymm2,%ymm0,%ymm0
  197,252,91,192,                         //vcvtdq2ps     %ymm0,%ymm0
  196,226,125,24,74,116,                  //vbroadcastss  0x74(%rdx),%ymm1
  197,244,89,192,                         //vmulps        %ymm0,%ymm1,%ymm0
  196,226,125,88,74,108,                  //vpbroadcastd  0x6c(%rdx),%ymm1
  197,245,219,202,                        //vpand         %ymm2,%ymm1,%ymm1
  197,252,91,201,                         //vcvtdq2ps     %ymm1,%ymm1
  196,226,125,24,90,120,                  //vbroadcastss  0x78(%rdx),%ymm3
  197,228,89,201,                         //vmulps        %ymm1,%ymm3,%ymm1
  196,226,125,88,90,112,                  //vpbroadcastd  0x70(%rdx),%ymm3
  197,229,219,210,                        //vpand         %ymm2,%ymm3,%ymm2
  197,252,91,210,                         //vcvtdq2ps     %ymm2,%ymm2
  196,226,125,24,90,124,                  //vbroadcastss  0x7c(%rdx),%ymm3
  197,228,89,210,                         //vmulps        %ymm2,%ymm3,%ymm2
  196,226,125,24,26,                      //vbroadcastss  (%rdx),%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  65,137,200,                             //mov           %ecx,%r8d
  65,128,224,7,                           //and           $0x7,%r8b
  197,249,239,192,                        //vpxor         %xmm0,%xmm0,%xmm0
  65,254,200,                             //dec           %r8b
  69,15,182,192,                          //movzbl        %r8b,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  119,146,                                //ja            850 <_sk_load_565_hsw+0x10>
  76,141,13,75,0,0,0,                     //lea           0x4b(%rip),%r9        # 910 <_sk_load_565_hsw+0xd0>
  75,99,4,129,                            //movslq        (%r9,%r8,4),%rax
  76,1,200,                               //add           %r9,%rax
  255,224,                                //jmpq          *%rax
  197,249,239,192,                        //vpxor         %xmm0,%xmm0,%xmm0
  196,193,121,196,68,122,12,6,            //vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,10,5,            //vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,8,4,             //vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,6,3,             //vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,4,2,             //vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,2,1,             //vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,4,122,0,                //vpinsrw       $0x0,(%r10,%rdi,2),%xmm0,%xmm0
  233,66,255,255,255,                     //jmpq          850 <_sk_load_565_hsw+0x10>
  102,144,                                //xchg          %ax,%ax
  242,255,                                //repnz         (bad)
  255,                                    //(bad)
  255,                                    //(bad)
  234,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  255,226,                                //jmpq          *%rdx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  218,255,                                //(bad)
  255,                                    //(bad)
  255,210,                                //callq         *%rdx
  255,                                    //(bad)
  255,                                    //(bad)
  255,202,                                //dec           %edx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  190,                                    //.byte         0xbe
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_store_565_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,8,                               //mov           (%rax),%r9
  196,98,125,24,130,128,0,0,0,            //vbroadcastss  0x80(%rdx),%ymm8
  197,60,89,200,                          //vmulps        %ymm0,%ymm8,%ymm9
  196,65,125,91,201,                      //vcvtps2dq     %ymm9,%ymm9
  196,193,53,114,241,11,                  //vpslld        $0xb,%ymm9,%ymm9
  196,98,125,24,146,132,0,0,0,            //vbroadcastss  0x84(%rdx),%ymm10
  197,44,89,209,                          //vmulps        %ymm1,%ymm10,%ymm10
  196,65,125,91,210,                      //vcvtps2dq     %ymm10,%ymm10
  196,193,45,114,242,5,                   //vpslld        $0x5,%ymm10,%ymm10
  196,65,45,235,201,                      //vpor          %ymm9,%ymm10,%ymm9
  197,60,89,194,                          //vmulps        %ymm2,%ymm8,%ymm8
  196,65,125,91,192,                      //vcvtps2dq     %ymm8,%ymm8
  196,65,53,235,192,                      //vpor          %ymm8,%ymm9,%ymm8
  196,67,125,57,193,1,                    //vextracti128  $0x1,%ymm8,%xmm9
  196,66,57,43,193,                       //vpackusdw     %xmm9,%xmm8,%xmm8
  72,133,201,                             //test          %rcx,%rcx
  117,10,                                 //jne           98e <_sk_store_565_hsw+0x62>
  196,65,122,127,4,121,                   //vmovdqu       %xmm8,(%r9,%rdi,2)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  137,200,                                //mov           %ecx,%eax
  36,7,                                   //and           $0x7,%al
  254,200,                                //dec           %al
  68,15,182,192,                          //movzbl        %al,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  119,236,                                //ja            98a <_sk_store_565_hsw+0x5e>
  76,141,21,71,0,0,0,                     //lea           0x47(%rip),%r10        # 9ec <_sk_store_565_hsw+0xc0>
  75,99,4,130,                            //movslq        (%r10,%r8,4),%rax
  76,1,208,                               //add           %r10,%rax
  255,224,                                //jmpq          *%rax
  196,67,121,21,68,121,12,6,              //vpextrw       $0x6,%xmm8,0xc(%r9,%rdi,2)
  196,67,121,21,68,121,10,5,              //vpextrw       $0x5,%xmm8,0xa(%r9,%rdi,2)
  196,67,121,21,68,121,8,4,               //vpextrw       $0x4,%xmm8,0x8(%r9,%rdi,2)
  196,67,121,21,68,121,6,3,               //vpextrw       $0x3,%xmm8,0x6(%r9,%rdi,2)
  196,67,121,21,68,121,4,2,               //vpextrw       $0x2,%xmm8,0x4(%r9,%rdi,2)
  196,67,121,21,68,121,2,1,               //vpextrw       $0x1,%xmm8,0x2(%r9,%rdi,2)
  197,121,126,192,                        //vmovd         %xmm8,%eax
  102,65,137,4,121,                       //mov           %ax,(%r9,%rdi,2)
  235,161,                                //jmp           98a <_sk_store_565_hsw+0x5e>
  15,31,0,                                //nopl          (%rax)
  242,255,                                //repnz         (bad)
  255,                                    //(bad)
  255,                                    //(bad)
  234,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  255,226,                                //jmpq          *%rdx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  218,255,                                //(bad)
  255,                                    //(bad)
  255,210,                                //callq         *%rdx
  255,                                    //(bad)
  255,                                    //(bad)
  255,202,                                //dec           %edx
  255,                                    //(bad)
  255,                                    //(bad)
  255,194,                                //inc           %edx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_load_8888_hsw[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,141,12,189,0,0,0,0,                  //lea           0x0(,%rdi,4),%r9
  76,3,8,                                 //add           (%rax),%r9
  77,133,192,                             //test          %r8,%r8
  117,85,                                 //jne           a72 <_sk_load_8888_hsw+0x6a>
  196,193,126,111,25,                     //vmovdqu       (%r9),%ymm3
  196,226,125,88,82,16,                   //vpbroadcastd  0x10(%rdx),%ymm2
  197,237,219,195,                        //vpand         %ymm3,%ymm2,%ymm0
  197,252,91,192,                         //vcvtdq2ps     %ymm0,%ymm0
  196,98,125,24,66,12,                    //vbroadcastss  0xc(%rdx),%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,245,114,211,8,                      //vpsrld        $0x8,%ymm3,%ymm1
  197,237,219,201,                        //vpand         %ymm1,%ymm2,%ymm1
  197,252,91,201,                         //vcvtdq2ps     %ymm1,%ymm1
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,181,114,211,16,                     //vpsrld        $0x10,%ymm3,%ymm9
  196,193,109,219,209,                    //vpand         %ymm9,%ymm2,%ymm2
  197,252,91,210,                         //vcvtdq2ps     %ymm2,%ymm2
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  197,229,114,211,24,                     //vpsrld        $0x18,%ymm3,%ymm3
  197,252,91,219,                         //vcvtdq2ps     %ymm3,%ymm3
  196,193,100,89,216,                     //vmulps        %ymm8,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  185,8,0,0,0,                            //mov           $0x8,%ecx
  68,41,193,                              //sub           %r8d,%ecx
  192,225,3,                              //shl           $0x3,%cl
  72,199,192,255,255,255,255,             //mov           $0xffffffffffffffff,%rax
  72,211,232,                             //shr           %cl,%rax
  196,225,249,110,192,                    //vmovq         %rax,%xmm0
  196,226,125,33,192,                     //vpmovsxbd     %xmm0,%ymm0
  196,194,125,140,25,                     //vpmaskmovd    (%r9),%ymm0,%ymm3
  235,138,                                //jmp           a22 <_sk_load_8888_hsw+0x1a>
};

CODE const uint8_t sk_store_8888_hsw[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,141,12,189,0,0,0,0,                  //lea           0x0(,%rdi,4),%r9
  76,3,8,                                 //add           (%rax),%r9
  196,98,125,24,66,8,                     //vbroadcastss  0x8(%rdx),%ymm8
  197,60,89,200,                          //vmulps        %ymm0,%ymm8,%ymm9
  196,65,125,91,201,                      //vcvtps2dq     %ymm9,%ymm9
  197,60,89,209,                          //vmulps        %ymm1,%ymm8,%ymm10
  196,65,125,91,210,                      //vcvtps2dq     %ymm10,%ymm10
  196,193,45,114,242,8,                   //vpslld        $0x8,%ymm10,%ymm10
  196,65,45,235,201,                      //vpor          %ymm9,%ymm10,%ymm9
  197,60,89,210,                          //vmulps        %ymm2,%ymm8,%ymm10
  196,65,125,91,210,                      //vcvtps2dq     %ymm10,%ymm10
  196,193,45,114,242,16,                  //vpslld        $0x10,%ymm10,%ymm10
  197,60,89,195,                          //vmulps        %ymm3,%ymm8,%ymm8
  196,65,125,91,192,                      //vcvtps2dq     %ymm8,%ymm8
  196,193,61,114,240,24,                  //vpslld        $0x18,%ymm8,%ymm8
  196,65,45,235,192,                      //vpor          %ymm8,%ymm10,%ymm8
  196,65,53,235,192,                      //vpor          %ymm8,%ymm9,%ymm8
  77,133,192,                             //test          %r8,%r8
  117,12,                                 //jne           b04 <_sk_store_8888_hsw+0x6c>
  196,65,126,127,1,                       //vmovdqu       %ymm8,(%r9)
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  185,8,0,0,0,                            //mov           $0x8,%ecx
  68,41,193,                              //sub           %r8d,%ecx
  192,225,3,                              //shl           $0x3,%cl
  72,199,192,255,255,255,255,             //mov           $0xffffffffffffffff,%rax
  72,211,232,                             //shr           %cl,%rax
  196,97,249,110,200,                     //vmovq         %rax,%xmm9
  196,66,125,33,201,                      //vpmovsxbd     %xmm9,%ymm9
  196,66,53,142,1,                        //vpmaskmovd    %ymm8,%ymm9,(%r9)
  235,211,                                //jmp           afd <_sk_store_8888_hsw+0x65>
};

CODE const uint8_t sk_load_f16_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,133,201,                             //test          %rcx,%rcx
  117,97,                                 //jne           b95 <_sk_load_f16_hsw+0x6b>
  197,249,16,12,248,                      //vmovupd       (%rax,%rdi,8),%xmm1
  197,249,16,84,248,16,                   //vmovupd       0x10(%rax,%rdi,8),%xmm2
  197,249,16,92,248,32,                   //vmovupd       0x20(%rax,%rdi,8),%xmm3
  197,121,16,68,248,48,                   //vmovupd       0x30(%rax,%rdi,8),%xmm8
  197,241,97,194,                         //vpunpcklwd    %xmm2,%xmm1,%xmm0
  197,241,105,202,                        //vpunpckhwd    %xmm2,%xmm1,%xmm1
  196,193,97,97,208,                      //vpunpcklwd    %xmm8,%xmm3,%xmm2
  196,193,97,105,216,                     //vpunpckhwd    %xmm8,%xmm3,%xmm3
  197,121,97,193,                         //vpunpcklwd    %xmm1,%xmm0,%xmm8
  197,121,105,201,                        //vpunpckhwd    %xmm1,%xmm0,%xmm9
  197,233,97,203,                         //vpunpcklwd    %xmm3,%xmm2,%xmm1
  197,233,105,219,                        //vpunpckhwd    %xmm3,%xmm2,%xmm3
  197,185,108,193,                        //vpunpcklqdq   %xmm1,%xmm8,%xmm0
  196,226,125,19,192,                     //vcvtph2ps     %xmm0,%ymm0
  197,185,109,201,                        //vpunpckhqdq   %xmm1,%xmm8,%xmm1
  196,226,125,19,201,                     //vcvtph2ps     %xmm1,%ymm1
  197,177,108,211,                        //vpunpcklqdq   %xmm3,%xmm9,%xmm2
  196,226,125,19,210,                     //vcvtph2ps     %xmm2,%ymm2
  197,177,109,219,                        //vpunpckhqdq   %xmm3,%xmm9,%xmm3
  196,226,125,19,219,                     //vcvtph2ps     %xmm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  197,251,16,12,248,                      //vmovsd        (%rax,%rdi,8),%xmm1
  196,65,57,87,192,                       //vxorpd        %xmm8,%xmm8,%xmm8
  72,131,249,1,                           //cmp           $0x1,%rcx
  117,6,                                  //jne           bab <_sk_load_f16_hsw+0x81>
  197,250,126,201,                        //vmovq         %xmm1,%xmm1
  235,30,                                 //jmp           bc9 <_sk_load_f16_hsw+0x9f>
  197,241,22,76,248,8,                    //vmovhpd       0x8(%rax,%rdi,8),%xmm1,%xmm1
  72,131,249,3,                           //cmp           $0x3,%rcx
  114,18,                                 //jb            bc9 <_sk_load_f16_hsw+0x9f>
  197,251,16,84,248,16,                   //vmovsd        0x10(%rax,%rdi,8),%xmm2
  72,131,249,3,                           //cmp           $0x3,%rcx
  117,19,                                 //jne           bd6 <_sk_load_f16_hsw+0xac>
  197,250,126,210,                        //vmovq         %xmm2,%xmm2
  235,46,                                 //jmp           bf7 <_sk_load_f16_hsw+0xcd>
  197,225,87,219,                         //vxorpd        %xmm3,%xmm3,%xmm3
  197,233,87,210,                         //vxorpd        %xmm2,%xmm2,%xmm2
  233,117,255,255,255,                    //jmpq          b4b <_sk_load_f16_hsw+0x21>
  197,233,22,84,248,24,                   //vmovhpd       0x18(%rax,%rdi,8),%xmm2,%xmm2
  72,131,249,5,                           //cmp           $0x5,%rcx
  114,21,                                 //jb            bf7 <_sk_load_f16_hsw+0xcd>
  197,251,16,92,248,32,                   //vmovsd        0x20(%rax,%rdi,8),%xmm3
  72,131,249,5,                           //cmp           $0x5,%rcx
  117,18,                                 //jne           c00 <_sk_load_f16_hsw+0xd6>
  197,250,126,219,                        //vmovq         %xmm3,%xmm3
  233,84,255,255,255,                     //jmpq          b4b <_sk_load_f16_hsw+0x21>
  197,225,87,219,                         //vxorpd        %xmm3,%xmm3,%xmm3
  233,75,255,255,255,                     //jmpq          b4b <_sk_load_f16_hsw+0x21>
  197,225,22,92,248,40,                   //vmovhpd       0x28(%rax,%rdi,8),%xmm3,%xmm3
  72,131,249,7,                           //cmp           $0x7,%rcx
  15,130,59,255,255,255,                  //jb            b4b <_sk_load_f16_hsw+0x21>
  197,123,16,68,248,48,                   //vmovsd        0x30(%rax,%rdi,8),%xmm8
  233,48,255,255,255,                     //jmpq          b4b <_sk_load_f16_hsw+0x21>
};

CODE const uint8_t sk_store_f16_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  196,195,125,29,192,4,                   //vcvtps2ph     $0x4,%ymm0,%xmm8
  196,195,125,29,201,4,                   //vcvtps2ph     $0x4,%ymm1,%xmm9
  196,195,125,29,210,4,                   //vcvtps2ph     $0x4,%ymm2,%xmm10
  196,195,125,29,219,4,                   //vcvtps2ph     $0x4,%ymm3,%xmm11
  196,65,57,97,225,                       //vpunpcklwd    %xmm9,%xmm8,%xmm12
  196,65,57,105,193,                      //vpunpckhwd    %xmm9,%xmm8,%xmm8
  196,65,41,97,203,                       //vpunpcklwd    %xmm11,%xmm10,%xmm9
  196,65,41,105,235,                      //vpunpckhwd    %xmm11,%xmm10,%xmm13
  196,65,25,98,217,                       //vpunpckldq    %xmm9,%xmm12,%xmm11
  196,65,25,106,209,                      //vpunpckhdq    %xmm9,%xmm12,%xmm10
  196,65,57,98,205,                       //vpunpckldq    %xmm13,%xmm8,%xmm9
  196,65,57,106,197,                      //vpunpckhdq    %xmm13,%xmm8,%xmm8
  72,133,201,                             //test          %rcx,%rcx
  117,27,                                 //jne           c80 <_sk_store_f16_hsw+0x65>
  197,120,17,28,248,                      //vmovups       %xmm11,(%rax,%rdi,8)
  197,120,17,84,248,16,                   //vmovups       %xmm10,0x10(%rax,%rdi,8)
  197,120,17,76,248,32,                   //vmovups       %xmm9,0x20(%rax,%rdi,8)
  197,122,127,68,248,48,                  //vmovdqu       %xmm8,0x30(%rax,%rdi,8)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  197,121,214,28,248,                     //vmovq         %xmm11,(%rax,%rdi,8)
  72,131,249,1,                           //cmp           $0x1,%rcx
  116,241,                                //je            c7c <_sk_store_f16_hsw+0x61>
  197,121,23,92,248,8,                    //vmovhpd       %xmm11,0x8(%rax,%rdi,8)
  72,131,249,3,                           //cmp           $0x3,%rcx
  114,229,                                //jb            c7c <_sk_store_f16_hsw+0x61>
  197,121,214,84,248,16,                  //vmovq         %xmm10,0x10(%rax,%rdi,8)
  116,221,                                //je            c7c <_sk_store_f16_hsw+0x61>
  197,121,23,84,248,24,                   //vmovhpd       %xmm10,0x18(%rax,%rdi,8)
  72,131,249,5,                           //cmp           $0x5,%rcx
  114,209,                                //jb            c7c <_sk_store_f16_hsw+0x61>
  197,121,214,76,248,32,                  //vmovq         %xmm9,0x20(%rax,%rdi,8)
  116,201,                                //je            c7c <_sk_store_f16_hsw+0x61>
  197,121,23,76,248,40,                   //vmovhpd       %xmm9,0x28(%rax,%rdi,8)
  72,131,249,7,                           //cmp           $0x7,%rcx
  114,189,                                //jb            c7c <_sk_store_f16_hsw+0x61>
  197,121,214,68,248,48,                  //vmovq         %xmm8,0x30(%rax,%rdi,8)
  235,181,                                //jmp           c7c <_sk_store_f16_hsw+0x61>
};

CODE const uint8_t sk_store_f32_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,0,                               //mov           (%rax),%r8
  72,141,4,189,0,0,0,0,                   //lea           0x0(,%rdi,4),%rax
  197,124,20,193,                         //vunpcklps     %ymm1,%ymm0,%ymm8
  197,124,21,217,                         //vunpckhps     %ymm1,%ymm0,%ymm11
  197,108,20,203,                         //vunpcklps     %ymm3,%ymm2,%ymm9
  197,108,21,227,                         //vunpckhps     %ymm3,%ymm2,%ymm12
  196,65,61,20,209,                       //vunpcklpd     %ymm9,%ymm8,%ymm10
  196,65,61,21,201,                       //vunpckhpd     %ymm9,%ymm8,%ymm9
  196,65,37,20,196,                       //vunpcklpd     %ymm12,%ymm11,%ymm8
  196,65,37,21,220,                       //vunpckhpd     %ymm12,%ymm11,%ymm11
  72,133,201,                             //test          %rcx,%rcx
  117,55,                                 //jne           d34 <_sk_store_f32_hsw+0x6d>
  196,67,45,24,225,1,                     //vinsertf128   $0x1,%xmm9,%ymm10,%ymm12
  196,67,61,24,235,1,                     //vinsertf128   $0x1,%xmm11,%ymm8,%ymm13
  196,67,45,6,201,49,                     //vperm2f128    $0x31,%ymm9,%ymm10,%ymm9
  196,67,61,6,195,49,                     //vperm2f128    $0x31,%ymm11,%ymm8,%ymm8
  196,65,125,17,36,128,                   //vmovupd       %ymm12,(%r8,%rax,4)
  196,65,125,17,108,128,32,               //vmovupd       %ymm13,0x20(%r8,%rax,4)
  196,65,125,17,76,128,64,                //vmovupd       %ymm9,0x40(%r8,%rax,4)
  196,65,125,17,68,128,96,                //vmovupd       %ymm8,0x60(%r8,%rax,4)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  196,65,121,17,20,128,                   //vmovupd       %xmm10,(%r8,%rax,4)
  72,131,249,1,                           //cmp           $0x1,%rcx
  116,240,                                //je            d30 <_sk_store_f32_hsw+0x69>
  196,65,121,17,76,128,16,                //vmovupd       %xmm9,0x10(%r8,%rax,4)
  72,131,249,3,                           //cmp           $0x3,%rcx
  114,227,                                //jb            d30 <_sk_store_f32_hsw+0x69>
  196,65,121,17,68,128,32,                //vmovupd       %xmm8,0x20(%r8,%rax,4)
  116,218,                                //je            d30 <_sk_store_f32_hsw+0x69>
  196,65,121,17,92,128,48,                //vmovupd       %xmm11,0x30(%r8,%rax,4)
  72,131,249,5,                           //cmp           $0x5,%rcx
  114,205,                                //jb            d30 <_sk_store_f32_hsw+0x69>
  196,67,125,25,84,128,64,1,              //vextractf128  $0x1,%ymm10,0x40(%r8,%rax,4)
  116,195,                                //je            d30 <_sk_store_f32_hsw+0x69>
  196,67,125,25,76,128,80,1,              //vextractf128  $0x1,%ymm9,0x50(%r8,%rax,4)
  72,131,249,7,                           //cmp           $0x7,%rcx
  114,181,                                //jb            d30 <_sk_store_f32_hsw+0x69>
  196,67,125,25,68,128,96,1,              //vextractf128  $0x1,%ymm8,0x60(%r8,%rax,4)
  235,171,                                //jmp           d30 <_sk_store_f32_hsw+0x69>
};

CODE const uint8_t sk_clamp_x_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  197,188,95,192,                         //vmaxps        %ymm0,%ymm8,%ymm0
  196,98,125,88,0,                        //vpbroadcastd  (%rax),%ymm8
  196,65,53,118,201,                      //vpcmpeqd      %ymm9,%ymm9,%ymm9
  196,65,61,254,193,                      //vpaddd        %ymm9,%ymm8,%ymm8
  196,193,124,93,192,                     //vminps        %ymm8,%ymm0,%ymm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_y_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  197,188,95,201,                         //vmaxps        %ymm1,%ymm8,%ymm1
  196,98,125,88,0,                        //vpbroadcastd  (%rax),%ymm8
  196,65,53,118,201,                      //vpcmpeqd      %ymm9,%ymm9,%ymm9
  196,65,61,254,193,                      //vpaddd        %ymm9,%ymm8,%ymm8
  196,193,116,93,200,                     //vminps        %ymm8,%ymm1,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_repeat_x_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,65,124,94,200,                      //vdivps        %ymm8,%ymm0,%ymm9
  196,67,125,8,201,1,                     //vroundps      $0x1,%ymm9,%ymm9
  196,98,61,172,200,                      //vfnmadd213ps  %ymm0,%ymm8,%ymm9
  197,253,118,192,                        //vpcmpeqd      %ymm0,%ymm0,%ymm0
  197,189,254,192,                        //vpaddd        %ymm0,%ymm8,%ymm0
  197,180,93,192,                         //vminps        %ymm0,%ymm9,%ymm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_repeat_y_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,65,116,94,200,                      //vdivps        %ymm8,%ymm1,%ymm9
  196,67,125,8,201,1,                     //vroundps      $0x1,%ymm9,%ymm9
  196,98,61,172,201,                      //vfnmadd213ps  %ymm1,%ymm8,%ymm9
  197,245,118,201,                        //vpcmpeqd      %ymm1,%ymm1,%ymm1
  197,189,254,201,                        //vpaddd        %ymm1,%ymm8,%ymm1
  197,180,93,201,                         //vminps        %ymm1,%ymm9,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_mirror_x_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,122,16,0,                           //vmovss        (%rax),%xmm8
  196,66,125,24,200,                      //vbroadcastss  %xmm8,%ymm9
  196,65,124,92,209,                      //vsubps        %ymm9,%ymm0,%ymm10
  196,193,58,88,192,                      //vaddss        %xmm8,%xmm8,%xmm0
  196,226,125,24,192,                     //vbroadcastss  %xmm0,%ymm0
  197,44,94,192,                          //vdivps        %ymm0,%ymm10,%ymm8
  196,67,125,8,192,1,                     //vroundps      $0x1,%ymm8,%ymm8
  196,66,125,172,194,                     //vfnmadd213ps  %ymm10,%ymm0,%ymm8
  196,193,60,92,193,                      //vsubps        %ymm9,%ymm8,%ymm0
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  197,60,92,192,                          //vsubps        %ymm0,%ymm8,%ymm8
  197,188,84,192,                         //vandps        %ymm0,%ymm8,%ymm0
  196,65,61,118,192,                      //vpcmpeqd      %ymm8,%ymm8,%ymm8
  196,65,53,254,192,                      //vpaddd        %ymm8,%ymm9,%ymm8
  196,193,124,93,192,                     //vminps        %ymm8,%ymm0,%ymm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_mirror_y_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,122,16,0,                           //vmovss        (%rax),%xmm8
  196,66,125,24,200,                      //vbroadcastss  %xmm8,%ymm9
  196,65,116,92,209,                      //vsubps        %ymm9,%ymm1,%ymm10
  196,193,58,88,200,                      //vaddss        %xmm8,%xmm8,%xmm1
  196,226,125,24,201,                     //vbroadcastss  %xmm1,%ymm1
  197,44,94,193,                          //vdivps        %ymm1,%ymm10,%ymm8
  196,67,125,8,192,1,                     //vroundps      $0x1,%ymm8,%ymm8
  196,66,117,172,194,                     //vfnmadd213ps  %ymm10,%ymm1,%ymm8
  196,193,60,92,201,                      //vsubps        %ymm9,%ymm8,%ymm1
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  197,60,92,193,                          //vsubps        %ymm1,%ymm8,%ymm8
  197,188,84,201,                         //vandps        %ymm1,%ymm8,%ymm1
  196,65,61,118,192,                      //vpcmpeqd      %ymm8,%ymm8,%ymm8
  196,65,53,254,192,                      //vpaddd        %ymm8,%ymm9,%ymm8
  196,193,116,93,200,                     //vminps        %ymm8,%ymm1,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_2x3_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,8,                        //vbroadcastss  (%rax),%ymm9
  196,98,125,24,80,8,                     //vbroadcastss  0x8(%rax),%ymm10
  196,98,125,24,64,16,                    //vbroadcastss  0x10(%rax),%ymm8
  196,66,117,184,194,                     //vfmadd231ps   %ymm10,%ymm1,%ymm8
  196,66,125,184,193,                     //vfmadd231ps   %ymm9,%ymm0,%ymm8
  196,98,125,24,80,4,                     //vbroadcastss  0x4(%rax),%ymm10
  196,98,125,24,88,12,                    //vbroadcastss  0xc(%rax),%ymm11
  196,98,125,24,72,20,                    //vbroadcastss  0x14(%rax),%ymm9
  196,66,117,184,203,                     //vfmadd231ps   %ymm11,%ymm1,%ymm9
  196,66,125,184,202,                     //vfmadd231ps   %ymm10,%ymm0,%ymm9
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,124,41,192,                         //vmovaps       %ymm8,%ymm0
  197,124,41,201,                         //vmovaps       %ymm9,%ymm1
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_3x4_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,8,                        //vbroadcastss  (%rax),%ymm9
  196,98,125,24,80,12,                    //vbroadcastss  0xc(%rax),%ymm10
  196,98,125,24,88,24,                    //vbroadcastss  0x18(%rax),%ymm11
  196,98,125,24,64,36,                    //vbroadcastss  0x24(%rax),%ymm8
  196,66,109,184,195,                     //vfmadd231ps   %ymm11,%ymm2,%ymm8
  196,66,117,184,194,                     //vfmadd231ps   %ymm10,%ymm1,%ymm8
  196,66,125,184,193,                     //vfmadd231ps   %ymm9,%ymm0,%ymm8
  196,98,125,24,80,4,                     //vbroadcastss  0x4(%rax),%ymm10
  196,98,125,24,88,16,                    //vbroadcastss  0x10(%rax),%ymm11
  196,98,125,24,96,28,                    //vbroadcastss  0x1c(%rax),%ymm12
  196,98,125,24,72,40,                    //vbroadcastss  0x28(%rax),%ymm9
  196,66,109,184,204,                     //vfmadd231ps   %ymm12,%ymm2,%ymm9
  196,66,117,184,203,                     //vfmadd231ps   %ymm11,%ymm1,%ymm9
  196,66,125,184,202,                     //vfmadd231ps   %ymm10,%ymm0,%ymm9
  196,98,125,24,88,8,                     //vbroadcastss  0x8(%rax),%ymm11
  196,98,125,24,96,20,                    //vbroadcastss  0x14(%rax),%ymm12
  196,98,125,24,104,32,                   //vbroadcastss  0x20(%rax),%ymm13
  196,98,125,24,80,44,                    //vbroadcastss  0x2c(%rax),%ymm10
  196,66,109,184,213,                     //vfmadd231ps   %ymm13,%ymm2,%ymm10
  196,66,117,184,212,                     //vfmadd231ps   %ymm12,%ymm1,%ymm10
  196,66,125,184,211,                     //vfmadd231ps   %ymm11,%ymm0,%ymm10
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,124,41,192,                         //vmovaps       %ymm8,%ymm0
  197,124,41,201,                         //vmovaps       %ymm9,%ymm1
  197,124,41,210,                         //vmovaps       %ymm10,%ymm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_perspective_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,98,125,24,72,4,                     //vbroadcastss  0x4(%rax),%ymm9
  196,98,125,24,80,8,                     //vbroadcastss  0x8(%rax),%ymm10
  196,66,117,184,209,                     //vfmadd231ps   %ymm9,%ymm1,%ymm10
  196,66,125,184,208,                     //vfmadd231ps   %ymm8,%ymm0,%ymm10
  196,98,125,24,64,12,                    //vbroadcastss  0xc(%rax),%ymm8
  196,98,125,24,72,16,                    //vbroadcastss  0x10(%rax),%ymm9
  196,98,125,24,88,20,                    //vbroadcastss  0x14(%rax),%ymm11
  196,66,117,184,217,                     //vfmadd231ps   %ymm9,%ymm1,%ymm11
  196,66,125,184,216,                     //vfmadd231ps   %ymm8,%ymm0,%ymm11
  196,98,125,24,64,24,                    //vbroadcastss  0x18(%rax),%ymm8
  196,98,125,24,72,28,                    //vbroadcastss  0x1c(%rax),%ymm9
  196,98,125,24,96,32,                    //vbroadcastss  0x20(%rax),%ymm12
  196,66,117,184,225,                     //vfmadd231ps   %ymm9,%ymm1,%ymm12
  196,66,125,184,224,                     //vfmadd231ps   %ymm8,%ymm0,%ymm12
  196,193,124,83,204,                     //vrcpps        %ymm12,%ymm1
  197,172,89,193,                         //vmulps        %ymm1,%ymm10,%ymm0
  197,164,89,201,                         //vmulps        %ymm1,%ymm11,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_linear_gradient_2stops_hsw[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,226,125,24,72,16,                   //vbroadcastss  0x10(%rax),%ymm1
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,98,125,184,193,                     //vfmadd231ps   %ymm1,%ymm0,%ymm8
  196,226,125,24,80,20,                   //vbroadcastss  0x14(%rax),%ymm2
  196,226,125,24,72,4,                    //vbroadcastss  0x4(%rax),%ymm1
  196,226,125,184,202,                    //vfmadd231ps   %ymm2,%ymm0,%ymm1
  196,226,125,24,88,24,                   //vbroadcastss  0x18(%rax),%ymm3
  196,226,125,24,80,8,                    //vbroadcastss  0x8(%rax),%ymm2
  196,226,125,184,211,                    //vfmadd231ps   %ymm3,%ymm0,%ymm2
  196,98,125,24,72,28,                    //vbroadcastss  0x1c(%rax),%ymm9
  196,226,125,24,88,12,                   //vbroadcastss  0xc(%rax),%ymm3
  196,194,125,184,217,                    //vfmadd231ps   %ymm9,%ymm0,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,124,41,192,                         //vmovaps       %ymm8,%ymm0
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_start_pipeline_avx[] = {
  65,87,                                  //push          %r15
  65,86,                                  //push          %r14
  65,85,                                  //push          %r13
  65,84,                                  //push          %r12
  86,                                     //push          %rsi
  87,                                     //push          %rdi
  83,                                     //push          %rbx
  72,129,236,160,0,0,0,                   //sub           $0xa0,%rsp
  197,120,41,188,36,144,0,0,0,            //vmovaps       %xmm15,0x90(%rsp)
  197,120,41,180,36,128,0,0,0,            //vmovaps       %xmm14,0x80(%rsp)
  197,120,41,108,36,112,                  //vmovaps       %xmm13,0x70(%rsp)
  197,120,41,100,36,96,                   //vmovaps       %xmm12,0x60(%rsp)
  197,120,41,92,36,80,                    //vmovaps       %xmm11,0x50(%rsp)
  197,120,41,84,36,64,                    //vmovaps       %xmm10,0x40(%rsp)
  197,120,41,76,36,48,                    //vmovaps       %xmm9,0x30(%rsp)
  197,120,41,68,36,32,                    //vmovaps       %xmm8,0x20(%rsp)
  197,248,41,124,36,16,                   //vmovaps       %xmm7,0x10(%rsp)
  197,248,41,52,36,                       //vmovaps       %xmm6,(%rsp)
  77,137,205,                             //mov           %r9,%r13
  77,137,198,                             //mov           %r8,%r14
  72,137,203,                             //mov           %rcx,%rbx
  72,137,214,                             //mov           %rdx,%rsi
  72,173,                                 //lods          %ds:(%rsi),%rax
  73,137,199,                             //mov           %rax,%r15
  73,137,244,                             //mov           %rsi,%r12
  72,141,67,8,                            //lea           0x8(%rbx),%rax
  76,57,232,                              //cmp           %r13,%rax
  118,5,                                  //jbe           75 <_sk_start_pipeline_avx+0x75>
  72,137,223,                             //mov           %rbx,%rdi
  235,65,                                 //jmp           b6 <_sk_start_pipeline_avx+0xb6>
  185,0,0,0,0,                            //mov           $0x0,%ecx
  197,252,87,192,                         //vxorps        %ymm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  197,236,87,210,                         //vxorps        %ymm2,%ymm2,%ymm2
  197,228,87,219,                         //vxorps        %ymm3,%ymm3,%ymm3
  197,220,87,228,                         //vxorps        %ymm4,%ymm4,%ymm4
  197,212,87,237,                         //vxorps        %ymm5,%ymm5,%ymm5
  197,204,87,246,                         //vxorps        %ymm6,%ymm6,%ymm6
  197,196,87,255,                         //vxorps        %ymm7,%ymm7,%ymm7
  72,137,223,                             //mov           %rbx,%rdi
  76,137,230,                             //mov           %r12,%rsi
  76,137,242,                             //mov           %r14,%rdx
  65,255,215,                             //callq         *%r15
  72,141,123,8,                           //lea           0x8(%rbx),%rdi
  72,131,195,16,                          //add           $0x10,%rbx
  76,57,235,                              //cmp           %r13,%rbx
  72,137,251,                             //mov           %rdi,%rbx
  118,191,                                //jbe           75 <_sk_start_pipeline_avx+0x75>
  76,137,233,                             //mov           %r13,%rcx
  72,41,249,                              //sub           %rdi,%rcx
  116,41,                                 //je            e7 <_sk_start_pipeline_avx+0xe7>
  197,252,87,192,                         //vxorps        %ymm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  197,236,87,210,                         //vxorps        %ymm2,%ymm2,%ymm2
  197,228,87,219,                         //vxorps        %ymm3,%ymm3,%ymm3
  197,220,87,228,                         //vxorps        %ymm4,%ymm4,%ymm4
  197,212,87,237,                         //vxorps        %ymm5,%ymm5,%ymm5
  197,204,87,246,                         //vxorps        %ymm6,%ymm6,%ymm6
  197,196,87,255,                         //vxorps        %ymm7,%ymm7,%ymm7
  76,137,230,                             //mov           %r12,%rsi
  76,137,242,                             //mov           %r14,%rdx
  65,255,215,                             //callq         *%r15
  76,137,232,                             //mov           %r13,%rax
  197,248,40,52,36,                       //vmovaps       (%rsp),%xmm6
  197,248,40,124,36,16,                   //vmovaps       0x10(%rsp),%xmm7
  197,120,40,68,36,32,                    //vmovaps       0x20(%rsp),%xmm8
  197,120,40,76,36,48,                    //vmovaps       0x30(%rsp),%xmm9
  197,120,40,84,36,64,                    //vmovaps       0x40(%rsp),%xmm10
  197,120,40,92,36,80,                    //vmovaps       0x50(%rsp),%xmm11
  197,120,40,100,36,96,                   //vmovaps       0x60(%rsp),%xmm12
  197,120,40,108,36,112,                  //vmovaps       0x70(%rsp),%xmm13
  197,120,40,180,36,128,0,0,0,            //vmovaps       0x80(%rsp),%xmm14
  197,120,40,188,36,144,0,0,0,            //vmovaps       0x90(%rsp),%xmm15
  72,129,196,160,0,0,0,                   //add           $0xa0,%rsp
  91,                                     //pop           %rbx
  95,                                     //pop           %rdi
  94,                                     //pop           %rsi
  65,92,                                  //pop           %r12
  65,93,                                  //pop           %r13
  65,94,                                  //pop           %r14
  65,95,                                  //pop           %r15
  197,248,119,                            //vzeroupper
  195,                                    //retq
};

CODE const uint8_t sk_just_return_avx[] = {
  195,                                    //retq
};

CODE const uint8_t sk_seed_shader_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,249,110,199,                        //vmovd         %edi,%xmm0
  197,249,112,192,0,                      //vpshufd       $0x0,%xmm0,%xmm0
  196,227,125,24,192,1,                   //vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  197,252,91,192,                         //vcvtdq2ps     %ymm0,%ymm0
  196,226,125,24,74,4,                    //vbroadcastss  0x4(%rdx),%ymm1
  197,252,88,193,                         //vaddps        %ymm1,%ymm0,%ymm0
  197,252,88,66,20,                       //vaddps        0x14(%rdx),%ymm0,%ymm0
  196,226,125,24,16,                      //vbroadcastss  (%rax),%ymm2
  197,252,91,210,                         //vcvtdq2ps     %ymm2,%ymm2
  197,236,88,201,                         //vaddps        %ymm1,%ymm2,%ymm1
  196,226,125,24,18,                      //vbroadcastss  (%rdx),%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,228,87,219,                         //vxorps        %ymm3,%ymm3,%ymm3
  197,220,87,228,                         //vxorps        %ymm4,%ymm4,%ymm4
  197,212,87,237,                         //vxorps        %ymm5,%ymm5,%ymm5
  197,204,87,246,                         //vxorps        %ymm6,%ymm6,%ymm6
  197,196,87,255,                         //vxorps        %ymm7,%ymm7,%ymm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_constant_color_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,226,125,24,0,                       //vbroadcastss  (%rax),%ymm0
  196,226,125,24,72,4,                    //vbroadcastss  0x4(%rax),%ymm1
  196,226,125,24,80,8,                    //vbroadcastss  0x8(%rax),%ymm2
  196,226,125,24,88,12,                   //vbroadcastss  0xc(%rax),%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clear_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,87,192,                         //vxorps        %ymm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  197,236,87,210,                         //vxorps        %ymm2,%ymm2,%ymm2
  197,228,87,219,                         //vxorps        %ymm3,%ymm3,%ymm3
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_plus__avx[] = {
  197,252,88,196,                         //vaddps        %ymm4,%ymm0,%ymm0
  197,244,88,205,                         //vaddps        %ymm5,%ymm1,%ymm1
  197,236,88,214,                         //vaddps        %ymm6,%ymm2,%ymm2
  197,228,88,223,                         //vaddps        %ymm7,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_srcover_avx[] = {
  196,98,125,24,2,                        //vbroadcastss  (%rdx),%ymm8
  197,60,92,195,                          //vsubps        %ymm3,%ymm8,%ymm8
  197,60,89,204,                          //vmulps        %ymm4,%ymm8,%ymm9
  197,180,88,192,                         //vaddps        %ymm0,%ymm9,%ymm0
  197,60,89,205,                          //vmulps        %ymm5,%ymm8,%ymm9
  197,180,88,201,                         //vaddps        %ymm1,%ymm9,%ymm1
  197,60,89,206,                          //vmulps        %ymm6,%ymm8,%ymm9
  197,180,88,210,                         //vaddps        %ymm2,%ymm9,%ymm2
  197,60,89,199,                          //vmulps        %ymm7,%ymm8,%ymm8
  197,188,88,219,                         //vaddps        %ymm3,%ymm8,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_dstover_avx[] = {
  196,98,125,24,2,                        //vbroadcastss  (%rdx),%ymm8
  197,60,92,199,                          //vsubps        %ymm7,%ymm8,%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,252,88,196,                         //vaddps        %ymm4,%ymm0,%ymm0
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,244,88,205,                         //vaddps        %ymm5,%ymm1,%ymm1
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  197,236,88,214,                         //vaddps        %ymm6,%ymm2,%ymm2
  197,188,89,219,                         //vmulps        %ymm3,%ymm8,%ymm3
  197,228,88,223,                         //vaddps        %ymm7,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_0_avx[] = {
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  196,193,124,95,192,                     //vmaxps        %ymm8,%ymm0,%ymm0
  196,193,116,95,200,                     //vmaxps        %ymm8,%ymm1,%ymm1
  196,193,108,95,208,                     //vmaxps        %ymm8,%ymm2,%ymm2
  196,193,100,95,216,                     //vmaxps        %ymm8,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_1_avx[] = {
  196,98,125,24,2,                        //vbroadcastss  (%rdx),%ymm8
  196,193,124,93,192,                     //vminps        %ymm8,%ymm0,%ymm0
  196,193,116,93,200,                     //vminps        %ymm8,%ymm1,%ymm1
  196,193,108,93,208,                     //vminps        %ymm8,%ymm2,%ymm2
  196,193,100,93,216,                     //vminps        %ymm8,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_a_avx[] = {
  196,98,125,24,2,                        //vbroadcastss  (%rdx),%ymm8
  196,193,100,93,216,                     //vminps        %ymm8,%ymm3,%ymm3
  197,252,93,195,                         //vminps        %ymm3,%ymm0,%ymm0
  197,244,93,203,                         //vminps        %ymm3,%ymm1,%ymm1
  197,236,93,211,                         //vminps        %ymm3,%ymm2,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_set_rgb_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,226,125,24,0,                       //vbroadcastss  (%rax),%ymm0
  196,226,125,24,72,4,                    //vbroadcastss  0x4(%rax),%ymm1
  196,226,125,24,80,8,                    //vbroadcastss  0x8(%rax),%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_swap_rb_avx[] = {
  197,124,40,192,                         //vmovaps       %ymm0,%ymm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,40,194,                         //vmovaps       %ymm2,%ymm0
  197,124,41,194,                         //vmovaps       %ymm8,%ymm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_swap_avx[] = {
  197,124,40,195,                         //vmovaps       %ymm3,%ymm8
  197,124,40,202,                         //vmovaps       %ymm2,%ymm9
  197,124,40,209,                         //vmovaps       %ymm1,%ymm10
  197,124,40,216,                         //vmovaps       %ymm0,%ymm11
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,40,196,                         //vmovaps       %ymm4,%ymm0
  197,252,40,205,                         //vmovaps       %ymm5,%ymm1
  197,252,40,214,                         //vmovaps       %ymm6,%ymm2
  197,252,40,223,                         //vmovaps       %ymm7,%ymm3
  197,124,41,220,                         //vmovaps       %ymm11,%ymm4
  197,124,41,213,                         //vmovaps       %ymm10,%ymm5
  197,124,41,206,                         //vmovaps       %ymm9,%ymm6
  197,124,41,199,                         //vmovaps       %ymm8,%ymm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_move_src_dst_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,40,224,                         //vmovaps       %ymm0,%ymm4
  197,252,40,233,                         //vmovaps       %ymm1,%ymm5
  197,252,40,242,                         //vmovaps       %ymm2,%ymm6
  197,252,40,251,                         //vmovaps       %ymm3,%ymm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_move_dst_src_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,40,196,                         //vmovaps       %ymm4,%ymm0
  197,252,40,205,                         //vmovaps       %ymm5,%ymm1
  197,252,40,214,                         //vmovaps       %ymm6,%ymm2
  197,252,40,223,                         //vmovaps       %ymm7,%ymm3
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_premul_avx[] = {
  197,252,89,195,                         //vmulps        %ymm3,%ymm0,%ymm0
  197,244,89,203,                         //vmulps        %ymm3,%ymm1,%ymm1
  197,236,89,211,                         //vmulps        %ymm3,%ymm2,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_unpremul_avx[] = {
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  196,65,100,194,200,0,                   //vcmpeqps      %ymm8,%ymm3,%ymm9
  196,98,125,24,18,                       //vbroadcastss  (%rdx),%ymm10
  197,44,94,211,                          //vdivps        %ymm3,%ymm10,%ymm10
  196,67,45,74,192,144,                   //vblendvps     %ymm9,%ymm8,%ymm10,%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_from_srgb_avx[] = {
  196,98,125,24,66,64,                    //vbroadcastss  0x40(%rdx),%ymm8
  197,60,89,200,                          //vmulps        %ymm0,%ymm8,%ymm9
  197,124,89,208,                         //vmulps        %ymm0,%ymm0,%ymm10
  196,98,125,24,90,60,                    //vbroadcastss  0x3c(%rdx),%ymm11
  196,98,125,24,98,56,                    //vbroadcastss  0x38(%rdx),%ymm12
  197,36,89,232,                          //vmulps        %ymm0,%ymm11,%ymm13
  196,65,20,88,236,                       //vaddps        %ymm12,%ymm13,%ymm13
  196,98,125,24,114,52,                   //vbroadcastss  0x34(%rdx),%ymm14
  196,65,44,89,213,                       //vmulps        %ymm13,%ymm10,%ymm10
  196,65,12,88,210,                       //vaddps        %ymm10,%ymm14,%ymm10
  196,98,125,24,106,68,                   //vbroadcastss  0x44(%rdx),%ymm13
  196,193,124,194,197,1,                  //vcmpltps      %ymm13,%ymm0,%ymm0
  196,195,45,74,193,0,                    //vblendvps     %ymm0,%ymm9,%ymm10,%ymm0
  197,60,89,201,                          //vmulps        %ymm1,%ymm8,%ymm9
  197,116,89,209,                         //vmulps        %ymm1,%ymm1,%ymm10
  197,36,89,249,                          //vmulps        %ymm1,%ymm11,%ymm15
  196,65,4,88,252,                        //vaddps        %ymm12,%ymm15,%ymm15
  196,65,44,89,215,                       //vmulps        %ymm15,%ymm10,%ymm10
  196,65,12,88,210,                       //vaddps        %ymm10,%ymm14,%ymm10
  196,193,116,194,205,1,                  //vcmpltps      %ymm13,%ymm1,%ymm1
  196,195,45,74,201,16,                   //vblendvps     %ymm1,%ymm9,%ymm10,%ymm1
  197,60,89,194,                          //vmulps        %ymm2,%ymm8,%ymm8
  197,108,89,202,                         //vmulps        %ymm2,%ymm2,%ymm9
  197,36,89,210,                          //vmulps        %ymm2,%ymm11,%ymm10
  196,65,44,88,212,                       //vaddps        %ymm12,%ymm10,%ymm10
  196,65,52,89,202,                       //vmulps        %ymm10,%ymm9,%ymm9
  196,65,12,88,201,                       //vaddps        %ymm9,%ymm14,%ymm9
  196,193,108,194,213,1,                  //vcmpltps      %ymm13,%ymm2,%ymm2
  196,195,53,74,208,32,                   //vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_to_srgb_avx[] = {
  197,124,82,192,                         //vrsqrtps      %ymm0,%ymm8
  196,65,124,83,200,                      //vrcpps        %ymm8,%ymm9
  196,65,124,82,208,                      //vrsqrtps      %ymm8,%ymm10
  196,98,125,24,66,72,                    //vbroadcastss  0x48(%rdx),%ymm8
  197,60,89,216,                          //vmulps        %ymm0,%ymm8,%ymm11
  196,98,125,24,34,                       //vbroadcastss  (%rdx),%ymm12
  196,98,125,24,106,76,                   //vbroadcastss  0x4c(%rdx),%ymm13
  196,98,125,24,114,80,                   //vbroadcastss  0x50(%rdx),%ymm14
  196,98,125,24,122,84,                   //vbroadcastss  0x54(%rdx),%ymm15
  196,65,52,89,206,                       //vmulps        %ymm14,%ymm9,%ymm9
  196,65,52,88,207,                       //vaddps        %ymm15,%ymm9,%ymm9
  196,65,44,89,213,                       //vmulps        %ymm13,%ymm10,%ymm10
  196,65,44,88,201,                       //vaddps        %ymm9,%ymm10,%ymm9
  196,65,28,93,201,                       //vminps        %ymm9,%ymm12,%ymm9
  196,98,125,24,82,88,                    //vbroadcastss  0x58(%rdx),%ymm10
  196,193,124,194,194,1,                  //vcmpltps      %ymm10,%ymm0,%ymm0
  196,195,53,74,195,0,                    //vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
  197,124,82,201,                         //vrsqrtps      %ymm1,%ymm9
  196,65,124,83,217,                      //vrcpps        %ymm9,%ymm11
  196,65,124,82,201,                      //vrsqrtps      %ymm9,%ymm9
  196,65,12,89,219,                       //vmulps        %ymm11,%ymm14,%ymm11
  196,65,4,88,219,                        //vaddps        %ymm11,%ymm15,%ymm11
  196,65,20,89,201,                       //vmulps        %ymm9,%ymm13,%ymm9
  196,65,52,88,203,                       //vaddps        %ymm11,%ymm9,%ymm9
  197,60,89,217,                          //vmulps        %ymm1,%ymm8,%ymm11
  196,65,28,93,201,                       //vminps        %ymm9,%ymm12,%ymm9
  196,193,116,194,202,1,                  //vcmpltps      %ymm10,%ymm1,%ymm1
  196,195,53,74,203,16,                   //vblendvps     %ymm1,%ymm11,%ymm9,%ymm1
  197,124,82,202,                         //vrsqrtps      %ymm2,%ymm9
  196,65,124,83,217,                      //vrcpps        %ymm9,%ymm11
  196,65,12,89,219,                       //vmulps        %ymm11,%ymm14,%ymm11
  196,65,4,88,219,                        //vaddps        %ymm11,%ymm15,%ymm11
  196,65,124,82,201,                      //vrsqrtps      %ymm9,%ymm9
  196,65,20,89,201,                       //vmulps        %ymm9,%ymm13,%ymm9
  196,65,52,88,203,                       //vaddps        %ymm11,%ymm9,%ymm9
  196,65,28,93,201,                       //vminps        %ymm9,%ymm12,%ymm9
  197,60,89,194,                          //vmulps        %ymm2,%ymm8,%ymm8
  196,193,108,194,210,1,                  //vcmpltps      %ymm10,%ymm2,%ymm2
  196,195,53,74,208,32,                   //vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_scale_1_float_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  197,188,89,219,                         //vmulps        %ymm3,%ymm8,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_scale_u8_avx[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,1,248,                               //add           %rdi,%rax
  77,133,192,                             //test          %r8,%r8
  117,65,                                 //jne           50f <_sk_scale_u8_avx+0x51>
  197,123,16,0,                           //vmovsd        (%rax),%xmm8
  196,66,121,49,200,                      //vpmovzxbd     %xmm8,%xmm9
  196,67,121,4,192,229,                   //vpermilps     $0xe5,%xmm8,%xmm8
  196,66,121,49,192,                      //vpmovzxbd     %xmm8,%xmm8
  196,67,53,24,192,1,                     //vinsertf128   $0x1,%xmm8,%ymm9,%ymm8
  196,65,124,91,192,                      //vcvtdq2ps     %ymm8,%ymm8
  196,98,125,24,74,12,                    //vbroadcastss  0xc(%rdx),%ymm9
  196,65,60,89,193,                       //vmulps        %ymm9,%ymm8,%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  197,188,89,219,                         //vmulps        %ymm3,%ymm8,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  49,201,                                 //xor           %ecx,%ecx
  77,137,194,                             //mov           %r8,%r10
  69,49,201,                              //xor           %r9d,%r9d
  68,15,182,24,                           //movzbl        (%rax),%r11d
  72,255,192,                             //inc           %rax
  73,211,227,                             //shl           %cl,%r11
  77,9,217,                               //or            %r11,%r9
  72,131,193,8,                           //add           $0x8,%rcx
  73,255,202,                             //dec           %r10
  117,234,                                //jne           517 <_sk_scale_u8_avx+0x59>
  196,65,249,110,193,                     //vmovq         %r9,%xmm8
  235,158,                                //jmp           4d2 <_sk_scale_u8_avx+0x14>
};

CODE const uint8_t sk_lerp_1_float_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  197,252,92,196,                         //vsubps        %ymm4,%ymm0,%ymm0
  196,193,124,89,192,                     //vmulps        %ymm8,%ymm0,%ymm0
  197,252,88,196,                         //vaddps        %ymm4,%ymm0,%ymm0
  197,244,92,205,                         //vsubps        %ymm5,%ymm1,%ymm1
  196,193,116,89,200,                     //vmulps        %ymm8,%ymm1,%ymm1
  197,244,88,205,                         //vaddps        %ymm5,%ymm1,%ymm1
  197,236,92,214,                         //vsubps        %ymm6,%ymm2,%ymm2
  196,193,108,89,208,                     //vmulps        %ymm8,%ymm2,%ymm2
  197,236,88,214,                         //vaddps        %ymm6,%ymm2,%ymm2
  197,228,92,223,                         //vsubps        %ymm7,%ymm3,%ymm3
  196,193,100,89,216,                     //vmulps        %ymm8,%ymm3,%ymm3
  197,228,88,223,                         //vaddps        %ymm7,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_lerp_u8_avx[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,1,248,                               //add           %rdi,%rax
  77,133,192,                             //test          %r8,%r8
  117,101,                                //jne           5e8 <_sk_lerp_u8_avx+0x75>
  197,123,16,0,                           //vmovsd        (%rax),%xmm8
  196,66,121,49,200,                      //vpmovzxbd     %xmm8,%xmm9
  196,67,121,4,192,229,                   //vpermilps     $0xe5,%xmm8,%xmm8
  196,66,121,49,192,                      //vpmovzxbd     %xmm8,%xmm8
  196,67,53,24,192,1,                     //vinsertf128   $0x1,%xmm8,%ymm9,%ymm8
  196,65,124,91,192,                      //vcvtdq2ps     %ymm8,%ymm8
  196,98,125,24,74,12,                    //vbroadcastss  0xc(%rdx),%ymm9
  196,65,60,89,193,                       //vmulps        %ymm9,%ymm8,%ymm8
  197,252,92,196,                         //vsubps        %ymm4,%ymm0,%ymm0
  196,193,124,89,192,                     //vmulps        %ymm8,%ymm0,%ymm0
  197,252,88,196,                         //vaddps        %ymm4,%ymm0,%ymm0
  197,244,92,205,                         //vsubps        %ymm5,%ymm1,%ymm1
  196,193,116,89,200,                     //vmulps        %ymm8,%ymm1,%ymm1
  197,244,88,205,                         //vaddps        %ymm5,%ymm1,%ymm1
  197,236,92,214,                         //vsubps        %ymm6,%ymm2,%ymm2
  196,193,108,89,208,                     //vmulps        %ymm8,%ymm2,%ymm2
  197,236,88,214,                         //vaddps        %ymm6,%ymm2,%ymm2
  197,228,92,223,                         //vsubps        %ymm7,%ymm3,%ymm3
  196,193,100,89,216,                     //vmulps        %ymm8,%ymm3,%ymm3
  197,228,88,223,                         //vaddps        %ymm7,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  49,201,                                 //xor           %ecx,%ecx
  77,137,194,                             //mov           %r8,%r10
  69,49,201,                              //xor           %r9d,%r9d
  68,15,182,24,                           //movzbl        (%rax),%r11d
  72,255,192,                             //inc           %rax
  73,211,227,                             //shl           %cl,%r11
  77,9,217,                               //or            %r11,%r9
  72,131,193,8,                           //add           $0x8,%rcx
  73,255,202,                             //dec           %r10
  117,234,                                //jne           5f0 <_sk_lerp_u8_avx+0x7d>
  196,65,249,110,193,                     //vmovq         %r9,%xmm8
  233,119,255,255,255,                    //jmpq          587 <_sk_lerp_u8_avx+0x14>
};

CODE const uint8_t sk_lerp_565_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,16,                              //mov           (%rax),%r10
  72,133,201,                             //test          %rcx,%rcx
  15,133,148,0,0,0,                       //jne           6b2 <_sk_lerp_565_avx+0xa2>
  196,65,122,111,4,122,                   //vmovdqu       (%r10,%rdi,2),%xmm8
  197,225,239,219,                        //vpxor         %xmm3,%xmm3,%xmm3
  197,185,105,219,                        //vpunpckhwd    %xmm3,%xmm8,%xmm3
  196,66,121,51,192,                      //vpmovzxwd     %xmm8,%xmm8
  196,227,61,24,219,1,                    //vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  196,98,125,24,66,104,                   //vbroadcastss  0x68(%rdx),%ymm8
  197,60,84,195,                          //vandps        %ymm3,%ymm8,%ymm8
  196,65,124,91,192,                      //vcvtdq2ps     %ymm8,%ymm8
  196,98,125,24,74,116,                   //vbroadcastss  0x74(%rdx),%ymm9
  196,65,52,89,192,                       //vmulps        %ymm8,%ymm9,%ymm8
  196,98,125,24,74,108,                   //vbroadcastss  0x6c(%rdx),%ymm9
  197,52,84,203,                          //vandps        %ymm3,%ymm9,%ymm9
  196,65,124,91,201,                      //vcvtdq2ps     %ymm9,%ymm9
  196,98,125,24,82,120,                   //vbroadcastss  0x78(%rdx),%ymm10
  196,65,44,89,201,                       //vmulps        %ymm9,%ymm10,%ymm9
  196,98,125,24,82,112,                   //vbroadcastss  0x70(%rdx),%ymm10
  197,172,84,219,                         //vandps        %ymm3,%ymm10,%ymm3
  197,252,91,219,                         //vcvtdq2ps     %ymm3,%ymm3
  196,98,125,24,82,124,                   //vbroadcastss  0x7c(%rdx),%ymm10
  197,172,89,219,                         //vmulps        %ymm3,%ymm10,%ymm3
  197,252,92,196,                         //vsubps        %ymm4,%ymm0,%ymm0
  196,193,124,89,192,                     //vmulps        %ymm8,%ymm0,%ymm0
  197,252,88,196,                         //vaddps        %ymm4,%ymm0,%ymm0
  197,244,92,205,                         //vsubps        %ymm5,%ymm1,%ymm1
  196,193,116,89,201,                     //vmulps        %ymm9,%ymm1,%ymm1
  197,244,88,205,                         //vaddps        %ymm5,%ymm1,%ymm1
  197,236,92,214,                         //vsubps        %ymm6,%ymm2,%ymm2
  197,236,89,211,                         //vmulps        %ymm3,%ymm2,%ymm2
  197,236,88,214,                         //vaddps        %ymm6,%ymm2,%ymm2
  196,226,125,24,26,                      //vbroadcastss  (%rdx),%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  65,137,200,                             //mov           %ecx,%r8d
  65,128,224,7,                           //and           $0x7,%r8b
  196,65,57,239,192,                      //vpxor         %xmm8,%xmm8,%xmm8
  65,254,200,                             //dec           %r8b
  69,15,182,192,                          //movzbl        %r8b,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  15,135,85,255,255,255,                  //ja            624 <_sk_lerp_565_avx+0x14>
  76,141,13,74,0,0,0,                     //lea           0x4a(%rip),%r9        # 720 <_sk_lerp_565_avx+0x110>
  75,99,4,129,                            //movslq        (%r9,%r8,4),%rax
  76,1,200,                               //add           %r9,%rax
  255,224,                                //jmpq          *%rax
  197,225,239,219,                        //vpxor         %xmm3,%xmm3,%xmm3
  196,65,97,196,68,122,12,6,              //vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm3,%xmm8
  196,65,57,196,68,122,10,5,              //vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm8,%xmm8
  196,65,57,196,68,122,8,4,               //vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm8,%xmm8
  196,65,57,196,68,122,6,3,               //vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm8,%xmm8
  196,65,57,196,68,122,4,2,               //vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm8,%xmm8
  196,65,57,196,68,122,2,1,               //vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm8,%xmm8
  196,65,57,196,4,122,0,                  //vpinsrw       $0x0,(%r10,%rdi,2),%xmm8,%xmm8
  233,5,255,255,255,                      //jmpq          624 <_sk_lerp_565_avx+0x14>
  144,                                    //nop
  243,255,                                //repz          (bad)
  255,                                    //(bad)
  255,                                    //(bad)
  235,255,                                //jmp           725 <_sk_lerp_565_avx+0x115>
  255,                                    //(bad)
  255,227,                                //jmpq          *%rbx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  219,255,                                //(bad)
  255,                                    //(bad)
  255,211,                                //callq         *%rbx
  255,                                    //(bad)
  255,                                    //(bad)
  255,203,                                //dec           %ebx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  191,                                    //.byte         0xbf
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_load_tables_avx[] = {
  85,                                     //push          %rbp
  65,87,                                  //push          %r15
  65,86,                                  //push          %r14
  65,85,                                  //push          %r13
  65,84,                                  //push          %r12
  83,                                     //push          %rbx
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,0,                               //mov           (%rax),%r8
  72,133,201,                             //test          %rcx,%rcx
  15,133,18,2,0,0,                        //jne           966 <_sk_load_tables_avx+0x22a>
  196,65,124,16,4,184,                    //vmovups       (%r8,%rdi,4),%ymm8
  196,98,125,24,74,16,                    //vbroadcastss  0x10(%rdx),%ymm9
  196,193,52,84,192,                      //vandps        %ymm8,%ymm9,%ymm0
  196,193,249,126,193,                    //vmovq         %xmm0,%r9
  69,137,203,                             //mov           %r9d,%r11d
  196,195,249,22,194,1,                   //vpextrq       $0x1,%xmm0,%r10
  69,137,214,                             //mov           %r10d,%r14d
  73,193,234,32,                          //shr           $0x20,%r10
  73,193,233,32,                          //shr           $0x20,%r9
  196,227,125,25,192,1,                   //vextractf128  $0x1,%ymm0,%xmm0
  196,193,249,126,196,                    //vmovq         %xmm0,%r12
  69,137,231,                             //mov           %r12d,%r15d
  196,227,249,22,195,1,                   //vpextrq       $0x1,%xmm0,%rbx
  65,137,221,                             //mov           %ebx,%r13d
  72,193,235,32,                          //shr           $0x20,%rbx
  73,193,236,32,                          //shr           $0x20,%r12
  72,139,104,8,                           //mov           0x8(%rax),%rbp
  76,139,64,16,                           //mov           0x10(%rax),%r8
  196,161,122,16,68,189,0,                //vmovss        0x0(%rbp,%r15,4),%xmm0
  196,163,121,33,68,165,0,16,             //vinsertps     $0x10,0x0(%rbp,%r12,4),%xmm0,%xmm0
  196,163,121,33,68,173,0,32,             //vinsertps     $0x20,0x0(%rbp,%r13,4),%xmm0,%xmm0
  197,250,16,76,157,0,                    //vmovss        0x0(%rbp,%rbx,4),%xmm1
  196,227,121,33,193,48,                  //vinsertps     $0x30,%xmm1,%xmm0,%xmm0
  196,161,122,16,76,157,0,                //vmovss        0x0(%rbp,%r11,4),%xmm1
  196,163,113,33,76,141,0,16,             //vinsertps     $0x10,0x0(%rbp,%r9,4),%xmm1,%xmm1
  196,163,113,33,76,181,0,32,             //vinsertps     $0x20,0x0(%rbp,%r14,4),%xmm1,%xmm1
  196,161,122,16,92,149,0,                //vmovss        0x0(%rbp,%r10,4),%xmm3
  196,227,113,33,203,48,                  //vinsertps     $0x30,%xmm3,%xmm1,%xmm1
  196,227,117,24,192,1,                   //vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  196,193,113,114,208,8,                  //vpsrld        $0x8,%xmm8,%xmm1
  196,67,125,25,194,1,                    //vextractf128  $0x1,%ymm8,%xmm10
  196,193,105,114,210,8,                  //vpsrld        $0x8,%xmm10,%xmm2
  196,227,117,24,202,1,                   //vinsertf128   $0x1,%xmm2,%ymm1,%ymm1
  197,180,84,201,                         //vandps        %ymm1,%ymm9,%ymm1
  196,193,249,126,201,                    //vmovq         %xmm1,%r9
  69,137,203,                             //mov           %r9d,%r11d
  196,195,249,22,202,1,                   //vpextrq       $0x1,%xmm1,%r10
  69,137,214,                             //mov           %r10d,%r14d
  73,193,234,32,                          //shr           $0x20,%r10
  73,193,233,32,                          //shr           $0x20,%r9
  196,227,125,25,201,1,                   //vextractf128  $0x1,%ymm1,%xmm1
  196,225,249,126,205,                    //vmovq         %xmm1,%rbp
  65,137,239,                             //mov           %ebp,%r15d
  196,227,249,22,203,1,                   //vpextrq       $0x1,%xmm1,%rbx
  65,137,220,                             //mov           %ebx,%r12d
  72,193,235,32,                          //shr           $0x20,%rbx
  72,193,237,32,                          //shr           $0x20,%rbp
  196,129,122,16,12,184,                  //vmovss        (%r8,%r15,4),%xmm1
  196,195,113,33,12,168,16,               //vinsertps     $0x10,(%r8,%rbp,4),%xmm1,%xmm1
  196,129,122,16,20,160,                  //vmovss        (%r8,%r12,4),%xmm2
  196,227,113,33,202,32,                  //vinsertps     $0x20,%xmm2,%xmm1,%xmm1
  196,193,122,16,20,152,                  //vmovss        (%r8,%rbx,4),%xmm2
  196,227,113,33,202,48,                  //vinsertps     $0x30,%xmm2,%xmm1,%xmm1
  196,129,122,16,20,152,                  //vmovss        (%r8,%r11,4),%xmm2
  196,131,105,33,20,136,16,               //vinsertps     $0x10,(%r8,%r9,4),%xmm2,%xmm2
  196,129,122,16,28,176,                  //vmovss        (%r8,%r14,4),%xmm3
  196,227,105,33,211,32,                  //vinsertps     $0x20,%xmm3,%xmm2,%xmm2
  196,129,122,16,28,144,                  //vmovss        (%r8,%r10,4),%xmm3
  196,227,105,33,211,48,                  //vinsertps     $0x30,%xmm3,%xmm2,%xmm2
  196,227,109,24,201,1,                   //vinsertf128   $0x1,%xmm1,%ymm2,%ymm1
  72,139,64,24,                           //mov           0x18(%rax),%rax
  196,193,105,114,208,16,                 //vpsrld        $0x10,%xmm8,%xmm2
  196,193,97,114,210,16,                  //vpsrld        $0x10,%xmm10,%xmm3
  196,227,109,24,211,1,                   //vinsertf128   $0x1,%xmm3,%ymm2,%ymm2
  197,180,84,210,                         //vandps        %ymm2,%ymm9,%ymm2
  196,193,249,126,208,                    //vmovq         %xmm2,%r8
  69,137,194,                             //mov           %r8d,%r10d
  196,195,249,22,209,1,                   //vpextrq       $0x1,%xmm2,%r9
  69,137,203,                             //mov           %r9d,%r11d
  73,193,233,32,                          //shr           $0x20,%r9
  73,193,232,32,                          //shr           $0x20,%r8
  196,227,125,25,210,1,                   //vextractf128  $0x1,%ymm2,%xmm2
  196,225,249,126,213,                    //vmovq         %xmm2,%rbp
  65,137,238,                             //mov           %ebp,%r14d
  196,227,249,22,211,1,                   //vpextrq       $0x1,%xmm2,%rbx
  65,137,223,                             //mov           %ebx,%r15d
  72,193,235,32,                          //shr           $0x20,%rbx
  72,193,237,32,                          //shr           $0x20,%rbp
  196,161,122,16,20,176,                  //vmovss        (%rax,%r14,4),%xmm2
  196,227,105,33,20,168,16,               //vinsertps     $0x10,(%rax,%rbp,4),%xmm2,%xmm2
  196,161,122,16,28,184,                  //vmovss        (%rax,%r15,4),%xmm3
  196,227,105,33,211,32,                  //vinsertps     $0x20,%xmm3,%xmm2,%xmm2
  197,250,16,28,152,                      //vmovss        (%rax,%rbx,4),%xmm3
  196,99,105,33,203,48,                   //vinsertps     $0x30,%xmm3,%xmm2,%xmm9
  196,161,122,16,28,144,                  //vmovss        (%rax,%r10,4),%xmm3
  196,163,97,33,28,128,16,                //vinsertps     $0x10,(%rax,%r8,4),%xmm3,%xmm3
  196,161,122,16,20,152,                  //vmovss        (%rax,%r11,4),%xmm2
  196,227,97,33,210,32,                   //vinsertps     $0x20,%xmm2,%xmm3,%xmm2
  196,161,122,16,28,136,                  //vmovss        (%rax,%r9,4),%xmm3
  196,227,105,33,211,48,                  //vinsertps     $0x30,%xmm3,%xmm2,%xmm2
  196,195,109,24,209,1,                   //vinsertf128   $0x1,%xmm9,%ymm2,%ymm2
  196,193,57,114,208,24,                  //vpsrld        $0x18,%xmm8,%xmm8
  196,193,97,114,210,24,                  //vpsrld        $0x18,%xmm10,%xmm3
  196,227,61,24,219,1,                    //vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  197,252,91,219,                         //vcvtdq2ps     %ymm3,%ymm3
  196,98,125,24,66,12,                    //vbroadcastss  0xc(%rdx),%ymm8
  196,193,100,89,216,                     //vmulps        %ymm8,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  91,                                     //pop           %rbx
  65,92,                                  //pop           %r12
  65,93,                                  //pop           %r13
  65,94,                                  //pop           %r14
  65,95,                                  //pop           %r15
  93,                                     //pop           %rbp
  255,224,                                //jmpq          *%rax
  65,137,201,                             //mov           %ecx,%r9d
  65,128,225,7,                           //and           $0x7,%r9b
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  65,254,201,                             //dec           %r9b
  69,15,182,201,                          //movzbl        %r9b,%r9d
  65,128,249,6,                           //cmp           $0x6,%r9b
  15,135,215,253,255,255,                 //ja            75a <_sk_load_tables_avx+0x1e>
  76,141,21,138,0,0,0,                    //lea           0x8a(%rip),%r10        # a14 <_sk_load_tables_avx+0x2d8>
  79,99,12,138,                           //movslq        (%r10,%r9,4),%r9
  77,1,209,                               //add           %r10,%r9
  65,255,225,                             //jmpq          *%r9
  196,193,121,110,68,184,24,              //vmovd         0x18(%r8,%rdi,4),%xmm0
  197,249,112,192,68,                     //vpshufd       $0x44,%xmm0,%xmm0
  196,227,125,24,192,1,                   //vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  196,99,117,12,192,64,                   //vblendps      $0x40,%ymm0,%ymm1,%ymm8
  196,99,125,25,192,1,                    //vextractf128  $0x1,%ymm8,%xmm0
  196,195,121,34,68,184,20,1,             //vpinsrd       $0x1,0x14(%r8,%rdi,4),%xmm0,%xmm0
  196,99,61,24,192,1,                     //vinsertf128   $0x1,%xmm0,%ymm8,%ymm8
  196,99,125,25,192,1,                    //vextractf128  $0x1,%ymm8,%xmm0
  196,195,121,34,68,184,16,0,             //vpinsrd       $0x0,0x10(%r8,%rdi,4),%xmm0,%xmm0
  196,99,61,24,192,1,                     //vinsertf128   $0x1,%xmm0,%ymm8,%ymm8
  196,195,57,34,68,184,12,3,              //vpinsrd       $0x3,0xc(%r8,%rdi,4),%xmm8,%xmm0
  196,99,61,12,192,15,                    //vblendps      $0xf,%ymm0,%ymm8,%ymm8
  196,195,57,34,68,184,8,2,               //vpinsrd       $0x2,0x8(%r8,%rdi,4),%xmm8,%xmm0
  196,99,61,12,192,15,                    //vblendps      $0xf,%ymm0,%ymm8,%ymm8
  196,195,57,34,68,184,4,1,               //vpinsrd       $0x1,0x4(%r8,%rdi,4),%xmm8,%xmm0
  196,99,61,12,192,15,                    //vblendps      $0xf,%ymm0,%ymm8,%ymm8
  196,195,57,34,4,184,0,                  //vpinsrd       $0x0,(%r8,%rdi,4),%xmm8,%xmm0
  196,99,61,12,192,15,                    //vblendps      $0xf,%ymm0,%ymm8,%ymm8
  233,70,253,255,255,                     //jmpq          75a <_sk_load_tables_avx+0x1e>
  238,                                    //out           %al,(%dx)
  255,                                    //(bad)
  255,                                    //(bad)
  255,224,                                //jmpq          *%rax
  255,                                    //(bad)
  255,                                    //(bad)
  255,210,                                //callq         *%rdx
  255,                                    //(bad)
  255,                                    //(bad)
  255,196,                                //inc           %esp
  255,                                    //(bad)
  255,                                    //(bad)
  255,176,255,255,255,156,                //pushq         -0x63000001(%rax)
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
  128,255,255,                            //cmp           $0xff,%bh
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_load_a8_avx[] = {
  73,137,200,                             //mov           %rcx,%r8
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,1,248,                               //add           %rdi,%rax
  77,133,192,                             //test          %r8,%r8
  117,59,                                 //jne           a7b <_sk_load_a8_avx+0x4b>
  197,251,16,0,                           //vmovsd        (%rax),%xmm0
  196,226,121,49,200,                     //vpmovzxbd     %xmm0,%xmm1
  196,227,121,4,192,229,                  //vpermilps     $0xe5,%xmm0,%xmm0
  196,226,121,49,192,                     //vpmovzxbd     %xmm0,%xmm0
  196,227,117,24,192,1,                   //vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  197,252,91,192,                         //vcvtdq2ps     %ymm0,%ymm0
  196,226,125,24,74,12,                   //vbroadcastss  0xc(%rdx),%ymm1
  197,252,89,217,                         //vmulps        %ymm1,%ymm0,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,252,87,192,                         //vxorps        %ymm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  197,236,87,210,                         //vxorps        %ymm2,%ymm2,%ymm2
  76,137,193,                             //mov           %r8,%rcx
  255,224,                                //jmpq          *%rax
  49,201,                                 //xor           %ecx,%ecx
  77,137,194,                             //mov           %r8,%r10
  69,49,201,                              //xor           %r9d,%r9d
  68,15,182,24,                           //movzbl        (%rax),%r11d
  72,255,192,                             //inc           %rax
  73,211,227,                             //shl           %cl,%r11
  77,9,217,                               //or            %r11,%r9
  72,131,193,8,                           //add           $0x8,%rcx
  73,255,202,                             //dec           %r10
  117,234,                                //jne           a83 <_sk_load_a8_avx+0x53>
  196,193,249,110,193,                    //vmovq         %r9,%xmm0
  235,164,                                //jmp           a44 <_sk_load_a8_avx+0x14>
};

CODE const uint8_t sk_store_a8_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,8,                               //mov           (%rax),%r9
  196,98,125,24,66,8,                     //vbroadcastss  0x8(%rdx),%ymm8
  197,60,89,195,                          //vmulps        %ymm3,%ymm8,%ymm8
  196,65,125,91,192,                      //vcvtps2dq     %ymm8,%ymm8
  196,67,125,25,193,1,                    //vextractf128  $0x1,%ymm8,%xmm9
  196,66,57,43,193,                       //vpackusdw     %xmm9,%xmm8,%xmm8
  196,65,57,103,192,                      //vpackuswb     %xmm8,%xmm8,%xmm8
  72,133,201,                             //test          %rcx,%rcx
  117,10,                                 //jne           ad3 <_sk_store_a8_avx+0x33>
  196,65,123,17,4,57,                     //vmovsd        %xmm8,(%r9,%rdi,1)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  137,200,                                //mov           %ecx,%eax
  36,7,                                   //and           $0x7,%al
  254,200,                                //dec           %al
  68,15,182,192,                          //movzbl        %al,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  119,236,                                //ja            acf <_sk_store_a8_avx+0x2f>
  196,66,121,48,192,                      //vpmovzxbw     %xmm8,%xmm8
  76,141,21,69,0,0,0,                     //lea           0x45(%rip),%r10        # b34 <_sk_store_a8_avx+0x94>
  75,99,4,130,                            //movslq        (%r10,%r8,4),%rax
  76,1,208,                               //add           %r10,%rax
  255,224,                                //jmpq          *%rax
  196,67,121,20,68,57,6,12,               //vpextrb       $0xc,%xmm8,0x6(%r9,%rdi,1)
  196,67,121,20,68,57,5,10,               //vpextrb       $0xa,%xmm8,0x5(%r9,%rdi,1)
  196,67,121,20,68,57,4,8,                //vpextrb       $0x8,%xmm8,0x4(%r9,%rdi,1)
  196,67,121,20,68,57,3,6,                //vpextrb       $0x6,%xmm8,0x3(%r9,%rdi,1)
  196,67,121,20,68,57,2,4,                //vpextrb       $0x4,%xmm8,0x2(%r9,%rdi,1)
  196,67,121,20,68,57,1,2,                //vpextrb       $0x2,%xmm8,0x1(%r9,%rdi,1)
  196,67,121,20,4,57,0,                   //vpextrb       $0x0,%xmm8,(%r9,%rdi,1)
  235,158,                                //jmp           acf <_sk_store_a8_avx+0x2f>
  15,31,0,                                //nopl          (%rax)
  244,                                    //hlt
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  236,                                    //in            (%dx),%al
  255,                                    //(bad)
  255,                                    //(bad)
  255,228,                                //jmpq          *%rsp
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  220,255,                                //fdivr         %st,%st(7)
  255,                                    //(bad)
  255,212,                                //callq         *%rsp
  255,                                    //(bad)
  255,                                    //(bad)
  255,204,                                //dec           %esp
  255,                                    //(bad)
  255,                                    //(bad)
  255,196,                                //inc           %esp
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_load_565_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,16,                              //mov           (%rax),%r10
  72,133,201,                             //test          %rcx,%rcx
  117,106,                                //jne           bc4 <_sk_load_565_avx+0x74>
  196,193,122,111,4,122,                  //vmovdqu       (%r10,%rdi,2),%xmm0
  197,241,239,201,                        //vpxor         %xmm1,%xmm1,%xmm1
  197,249,105,201,                        //vpunpckhwd    %xmm1,%xmm0,%xmm1
  196,226,121,51,192,                     //vpmovzxwd     %xmm0,%xmm0
  196,227,125,24,209,1,                   //vinsertf128   $0x1,%xmm1,%ymm0,%ymm2
  196,226,125,24,66,104,                  //vbroadcastss  0x68(%rdx),%ymm0
  197,252,84,194,                         //vandps        %ymm2,%ymm0,%ymm0
  197,252,91,192,                         //vcvtdq2ps     %ymm0,%ymm0
  196,226,125,24,74,116,                  //vbroadcastss  0x74(%rdx),%ymm1
  197,244,89,192,                         //vmulps        %ymm0,%ymm1,%ymm0
  196,226,125,24,74,108,                  //vbroadcastss  0x6c(%rdx),%ymm1
  197,244,84,202,                         //vandps        %ymm2,%ymm1,%ymm1
  197,252,91,201,                         //vcvtdq2ps     %ymm1,%ymm1
  196,226,125,24,90,120,                  //vbroadcastss  0x78(%rdx),%ymm3
  197,228,89,201,                         //vmulps        %ymm1,%ymm3,%ymm1
  196,226,125,24,90,112,                  //vbroadcastss  0x70(%rdx),%ymm3
  197,228,84,210,                         //vandps        %ymm2,%ymm3,%ymm2
  197,252,91,210,                         //vcvtdq2ps     %ymm2,%ymm2
  196,226,125,24,90,124,                  //vbroadcastss  0x7c(%rdx),%ymm3
  197,228,89,210,                         //vmulps        %ymm2,%ymm3,%ymm2
  196,226,125,24,26,                      //vbroadcastss  (%rdx),%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  65,137,200,                             //mov           %ecx,%r8d
  65,128,224,7,                           //and           $0x7,%r8b
  197,249,239,192,                        //vpxor         %xmm0,%xmm0,%xmm0
  65,254,200,                             //dec           %r8b
  69,15,182,192,                          //movzbl        %r8b,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  119,132,                                //ja            b60 <_sk_load_565_avx+0x10>
  76,141,13,73,0,0,0,                     //lea           0x49(%rip),%r9        # c2c <_sk_load_565_avx+0xdc>
  75,99,4,129,                            //movslq        (%r9,%r8,4),%rax
  76,1,200,                               //add           %r9,%rax
  255,224,                                //jmpq          *%rax
  197,249,239,192,                        //vpxor         %xmm0,%xmm0,%xmm0
  196,193,121,196,68,122,12,6,            //vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,10,5,            //vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,8,4,             //vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,6,3,             //vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,4,2,             //vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,68,122,2,1,             //vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm0,%xmm0
  196,193,121,196,4,122,0,                //vpinsrw       $0x0,(%r10,%rdi,2),%xmm0,%xmm0
  233,52,255,255,255,                     //jmpq          b60 <_sk_load_565_avx+0x10>
  244,                                    //hlt
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  236,                                    //in            (%dx),%al
  255,                                    //(bad)
  255,                                    //(bad)
  255,228,                                //jmpq          *%rsp
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  220,255,                                //fdivr         %st,%st(7)
  255,                                    //(bad)
  255,212,                                //callq         *%rsp
  255,                                    //(bad)
  255,                                    //(bad)
  255,204,                                //dec           %esp
  255,                                    //(bad)
  255,                                    //(bad)
  255,192,                                //inc           %eax
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_store_565_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,8,                               //mov           (%rax),%r9
  196,98,125,24,130,128,0,0,0,            //vbroadcastss  0x80(%rdx),%ymm8
  197,60,89,200,                          //vmulps        %ymm0,%ymm8,%ymm9
  196,65,125,91,201,                      //vcvtps2dq     %ymm9,%ymm9
  196,193,41,114,241,11,                  //vpslld        $0xb,%xmm9,%xmm10
  196,67,125,25,201,1,                    //vextractf128  $0x1,%ymm9,%xmm9
  196,193,49,114,241,11,                  //vpslld        $0xb,%xmm9,%xmm9
  196,67,45,24,201,1,                     //vinsertf128   $0x1,%xmm9,%ymm10,%ymm9
  196,98,125,24,146,132,0,0,0,            //vbroadcastss  0x84(%rdx),%ymm10
  197,44,89,209,                          //vmulps        %ymm1,%ymm10,%ymm10
  196,65,125,91,210,                      //vcvtps2dq     %ymm10,%ymm10
  196,193,33,114,242,5,                   //vpslld        $0x5,%xmm10,%xmm11
  196,67,125,25,210,1,                    //vextractf128  $0x1,%ymm10,%xmm10
  196,193,41,114,242,5,                   //vpslld        $0x5,%xmm10,%xmm10
  196,67,37,24,210,1,                     //vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  196,65,45,86,201,                       //vorpd         %ymm9,%ymm10,%ymm9
  197,60,89,194,                          //vmulps        %ymm2,%ymm8,%ymm8
  196,65,125,91,192,                      //vcvtps2dq     %ymm8,%ymm8
  196,65,53,86,192,                       //vorpd         %ymm8,%ymm9,%ymm8
  196,67,125,25,193,1,                    //vextractf128  $0x1,%ymm8,%xmm9
  196,66,57,43,193,                       //vpackusdw     %xmm9,%xmm8,%xmm8
  72,133,201,                             //test          %rcx,%rcx
  117,10,                                 //jne           cce <_sk_store_565_avx+0x86>
  196,65,122,127,4,121,                   //vmovdqu       %xmm8,(%r9,%rdi,2)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  137,200,                                //mov           %ecx,%eax
  36,7,                                   //and           $0x7,%al
  254,200,                                //dec           %al
  68,15,182,192,                          //movzbl        %al,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  119,236,                                //ja            cca <_sk_store_565_avx+0x82>
  76,141,21,71,0,0,0,                     //lea           0x47(%rip),%r10        # d2c <_sk_store_565_avx+0xe4>
  75,99,4,130,                            //movslq        (%r10,%r8,4),%rax
  76,1,208,                               //add           %r10,%rax
  255,224,                                //jmpq          *%rax
  196,67,121,21,68,121,12,6,              //vpextrw       $0x6,%xmm8,0xc(%r9,%rdi,2)
  196,67,121,21,68,121,10,5,              //vpextrw       $0x5,%xmm8,0xa(%r9,%rdi,2)
  196,67,121,21,68,121,8,4,               //vpextrw       $0x4,%xmm8,0x8(%r9,%rdi,2)
  196,67,121,21,68,121,6,3,               //vpextrw       $0x3,%xmm8,0x6(%r9,%rdi,2)
  196,67,121,21,68,121,4,2,               //vpextrw       $0x2,%xmm8,0x4(%r9,%rdi,2)
  196,67,121,21,68,121,2,1,               //vpextrw       $0x1,%xmm8,0x2(%r9,%rdi,2)
  197,121,126,192,                        //vmovd         %xmm8,%eax
  102,65,137,4,121,                       //mov           %ax,(%r9,%rdi,2)
  235,161,                                //jmp           cca <_sk_store_565_avx+0x82>
  15,31,0,                                //nopl          (%rax)
  242,255,                                //repnz         (bad)
  255,                                    //(bad)
  255,                                    //(bad)
  234,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  255,226,                                //jmpq          *%rdx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  218,255,                                //(bad)
  255,                                    //(bad)
  255,210,                                //callq         *%rdx
  255,                                    //(bad)
  255,                                    //(bad)
  255,202,                                //dec           %edx
  255,                                    //(bad)
  255,                                    //(bad)
  255,194,                                //inc           %edx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_load_8888_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,16,                              //mov           (%rax),%r10
  72,133,201,                             //test          %rcx,%rcx
  117,125,                                //jne           dcf <_sk_load_8888_avx+0x87>
  196,65,124,16,12,186,                   //vmovups       (%r10,%rdi,4),%ymm9
  196,98,125,24,90,16,                    //vbroadcastss  0x10(%rdx),%ymm11
  196,193,36,84,193,                      //vandps        %ymm9,%ymm11,%ymm0
  197,252,91,192,                         //vcvtdq2ps     %ymm0,%ymm0
  196,98,125,24,66,12,                    //vbroadcastss  0xc(%rdx),%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  196,193,41,114,209,8,                   //vpsrld        $0x8,%xmm9,%xmm10
  196,99,125,25,203,1,                    //vextractf128  $0x1,%ymm9,%xmm3
  197,241,114,211,8,                      //vpsrld        $0x8,%xmm3,%xmm1
  196,227,45,24,201,1,                    //vinsertf128   $0x1,%xmm1,%ymm10,%ymm1
  197,164,84,201,                         //vandps        %ymm1,%ymm11,%ymm1
  197,252,91,201,                         //vcvtdq2ps     %ymm1,%ymm1
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  196,193,41,114,209,16,                  //vpsrld        $0x10,%xmm9,%xmm10
  197,233,114,211,16,                     //vpsrld        $0x10,%xmm3,%xmm2
  196,227,45,24,210,1,                    //vinsertf128   $0x1,%xmm2,%ymm10,%ymm2
  197,164,84,210,                         //vandps        %ymm2,%ymm11,%ymm2
  197,252,91,210,                         //vcvtdq2ps     %ymm2,%ymm2
  197,188,89,210,                         //vmulps        %ymm2,%ymm8,%ymm2
  196,193,49,114,209,24,                  //vpsrld        $0x18,%xmm9,%xmm9
  197,225,114,211,24,                     //vpsrld        $0x18,%xmm3,%xmm3
  196,227,53,24,219,1,                    //vinsertf128   $0x1,%xmm3,%ymm9,%ymm3
  197,252,91,219,                         //vcvtdq2ps     %ymm3,%ymm3
  196,193,100,89,216,                     //vmulps        %ymm8,%ymm3,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  65,137,200,                             //mov           %ecx,%r8d
  65,128,224,7,                           //and           $0x7,%r8b
  196,65,52,87,201,                       //vxorps        %ymm9,%ymm9,%ymm9
  65,254,200,                             //dec           %r8b
  69,15,182,192,                          //movzbl        %r8b,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  15,135,108,255,255,255,                 //ja            d58 <_sk_load_8888_avx+0x10>
  76,141,13,137,0,0,0,                    //lea           0x89(%rip),%r9        # e7c <_sk_load_8888_avx+0x134>
  75,99,4,129,                            //movslq        (%r9,%r8,4),%rax
  76,1,200,                               //add           %r9,%rax
  255,224,                                //jmpq          *%rax
  196,193,121,110,68,186,24,              //vmovd         0x18(%r10,%rdi,4),%xmm0
  197,249,112,192,68,                     //vpshufd       $0x44,%xmm0,%xmm0
  196,227,125,24,192,1,                   //vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  197,244,87,201,                         //vxorps        %ymm1,%ymm1,%ymm1
  196,99,117,12,200,64,                   //vblendps      $0x40,%ymm0,%ymm1,%ymm9
  196,99,125,25,200,1,                    //vextractf128  $0x1,%ymm9,%xmm0
  196,195,121,34,68,186,20,1,             //vpinsrd       $0x1,0x14(%r10,%rdi,4),%xmm0,%xmm0
  196,99,53,24,200,1,                     //vinsertf128   $0x1,%xmm0,%ymm9,%ymm9
  196,99,125,25,200,1,                    //vextractf128  $0x1,%ymm9,%xmm0
  196,195,121,34,68,186,16,0,             //vpinsrd       $0x0,0x10(%r10,%rdi,4),%xmm0,%xmm0
  196,99,53,24,200,1,                     //vinsertf128   $0x1,%xmm0,%ymm9,%ymm9
  196,195,49,34,68,186,12,3,              //vpinsrd       $0x3,0xc(%r10,%rdi,4),%xmm9,%xmm0
  196,99,53,12,200,15,                    //vblendps      $0xf,%ymm0,%ymm9,%ymm9
  196,195,49,34,68,186,8,2,               //vpinsrd       $0x2,0x8(%r10,%rdi,4),%xmm9,%xmm0
  196,99,53,12,200,15,                    //vblendps      $0xf,%ymm0,%ymm9,%ymm9
  196,195,49,34,68,186,4,1,               //vpinsrd       $0x1,0x4(%r10,%rdi,4),%xmm9,%xmm0
  196,99,53,12,200,15,                    //vblendps      $0xf,%ymm0,%ymm9,%ymm9
  196,195,49,34,4,186,0,                  //vpinsrd       $0x0,(%r10,%rdi,4),%xmm9,%xmm0
  196,99,53,12,200,15,                    //vblendps      $0xf,%ymm0,%ymm9,%ymm9
  233,220,254,255,255,                    //jmpq          d58 <_sk_load_8888_avx+0x10>
  238,                                    //out           %al,(%dx)
  255,                                    //(bad)
  255,                                    //(bad)
  255,224,                                //jmpq          *%rax
  255,                                    //(bad)
  255,                                    //(bad)
  255,210,                                //callq         *%rdx
  255,                                    //(bad)
  255,                                    //(bad)
  255,196,                                //inc           %esp
  255,                                    //(bad)
  255,                                    //(bad)
  255,176,255,255,255,156,                //pushq         -0x63000001(%rax)
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
  128,255,255,                            //cmp           $0xff,%bh
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_store_8888_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,8,                               //mov           (%rax),%r9
  196,98,125,24,66,8,                     //vbroadcastss  0x8(%rdx),%ymm8
  197,60,89,200,                          //vmulps        %ymm0,%ymm8,%ymm9
  196,65,125,91,201,                      //vcvtps2dq     %ymm9,%ymm9
  197,60,89,209,                          //vmulps        %ymm1,%ymm8,%ymm10
  196,65,125,91,210,                      //vcvtps2dq     %ymm10,%ymm10
  196,193,33,114,242,8,                   //vpslld        $0x8,%xmm10,%xmm11
  196,67,125,25,210,1,                    //vextractf128  $0x1,%ymm10,%xmm10
  196,193,41,114,242,8,                   //vpslld        $0x8,%xmm10,%xmm10
  196,67,37,24,210,1,                     //vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  196,65,45,86,201,                       //vorpd         %ymm9,%ymm10,%ymm9
  197,60,89,210,                          //vmulps        %ymm2,%ymm8,%ymm10
  196,65,125,91,210,                      //vcvtps2dq     %ymm10,%ymm10
  196,193,33,114,242,16,                  //vpslld        $0x10,%xmm10,%xmm11
  196,67,125,25,210,1,                    //vextractf128  $0x1,%ymm10,%xmm10
  196,193,41,114,242,16,                  //vpslld        $0x10,%xmm10,%xmm10
  196,67,37,24,210,1,                     //vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  197,60,89,195,                          //vmulps        %ymm3,%ymm8,%ymm8
  196,65,125,91,192,                      //vcvtps2dq     %ymm8,%ymm8
  196,193,33,114,240,24,                  //vpslld        $0x18,%xmm8,%xmm11
  196,67,125,25,192,1,                    //vextractf128  $0x1,%ymm8,%xmm8
  196,193,57,114,240,24,                  //vpslld        $0x18,%xmm8,%xmm8
  196,67,37,24,192,1,                     //vinsertf128   $0x1,%xmm8,%ymm11,%ymm8
  196,65,45,86,192,                       //vorpd         %ymm8,%ymm10,%ymm8
  196,65,53,86,192,                       //vorpd         %ymm8,%ymm9,%ymm8
  72,133,201,                             //test          %rcx,%rcx
  117,10,                                 //jne           f2d <_sk_store_8888_avx+0x95>
  196,65,124,17,4,185,                    //vmovups       %ymm8,(%r9,%rdi,4)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  137,200,                                //mov           %ecx,%eax
  36,7,                                   //and           $0x7,%al
  254,200,                                //dec           %al
  68,15,182,192,                          //movzbl        %al,%r8d
  65,128,248,6,                           //cmp           $0x6,%r8b
  119,236,                                //ja            f29 <_sk_store_8888_avx+0x91>
  76,141,21,84,0,0,0,                     //lea           0x54(%rip),%r10        # f98 <_sk_store_8888_avx+0x100>
  75,99,4,130,                            //movslq        (%r10,%r8,4),%rax
  76,1,208,                               //add           %r10,%rax
  255,224,                                //jmpq          *%rax
  196,67,125,25,193,1,                    //vextractf128  $0x1,%ymm8,%xmm9
  196,67,121,22,76,185,24,2,              //vpextrd       $0x2,%xmm9,0x18(%r9,%rdi,4)
  196,67,125,25,193,1,                    //vextractf128  $0x1,%ymm8,%xmm9
  196,67,121,22,76,185,20,1,              //vpextrd       $0x1,%xmm9,0x14(%r9,%rdi,4)
  196,67,125,25,193,1,                    //vextractf128  $0x1,%ymm8,%xmm9
  196,65,121,126,76,185,16,               //vmovd         %xmm9,0x10(%r9,%rdi,4)
  196,67,121,22,68,185,12,3,              //vpextrd       $0x3,%xmm8,0xc(%r9,%rdi,4)
  196,67,121,22,68,185,8,2,               //vpextrd       $0x2,%xmm8,0x8(%r9,%rdi,4)
  196,67,121,22,68,185,4,1,               //vpextrd       $0x1,%xmm8,0x4(%r9,%rdi,4)
  196,65,121,126,4,185,                   //vmovd         %xmm8,(%r9,%rdi,4)
  235,147,                                //jmp           f29 <_sk_store_8888_avx+0x91>
  102,144,                                //xchg          %ax,%ax
  246,255,                                //idiv          %bh
  255,                                    //(bad)
  255,                                    //(bad)
  238,                                    //out           %al,(%dx)
  255,                                    //(bad)
  255,                                    //(bad)
  255,230,                                //jmpq          *%rsi
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //(bad)
  222,255,                                //fdivrp        %st,%st(7)
  255,                                    //(bad)
  255,209,                                //callq         *%rcx
  255,                                    //(bad)
  255,                                    //(bad)
  255,195,                                //inc           %ebx
  255,                                    //(bad)
  255,                                    //(bad)
  255,                                    //.byte         0xff
  181,255,                                //mov           $0xff,%ch
  255,                                    //(bad)
  255,                                    //.byte         0xff
};

CODE const uint8_t sk_load_f16_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,133,201,                             //test          %rcx,%rcx
  15,133,240,0,0,0,                       //jne           10b2 <_sk_load_f16_avx+0xfe>
  197,249,16,12,248,                      //vmovupd       (%rax,%rdi,8),%xmm1
  197,249,16,84,248,16,                   //vmovupd       0x10(%rax,%rdi,8),%xmm2
  197,249,16,92,248,32,                   //vmovupd       0x20(%rax,%rdi,8),%xmm3
  197,121,16,68,248,48,                   //vmovupd       0x30(%rax,%rdi,8),%xmm8
  197,241,97,194,                         //vpunpcklwd    %xmm2,%xmm1,%xmm0
  197,241,105,202,                        //vpunpckhwd    %xmm2,%xmm1,%xmm1
  196,193,97,97,208,                      //vpunpcklwd    %xmm8,%xmm3,%xmm2
  196,193,97,105,216,                     //vpunpckhwd    %xmm8,%xmm3,%xmm3
  197,121,97,193,                         //vpunpcklwd    %xmm1,%xmm0,%xmm8
  197,249,105,193,                        //vpunpckhwd    %xmm1,%xmm0,%xmm0
  197,233,97,203,                         //vpunpcklwd    %xmm3,%xmm2,%xmm1
  197,105,105,203,                        //vpunpckhwd    %xmm3,%xmm2,%xmm9
  197,249,110,90,100,                     //vmovd         0x64(%rdx),%xmm3
  197,249,112,219,0,                      //vpshufd       $0x0,%xmm3,%xmm3
  196,193,97,101,208,                     //vpcmpgtw      %xmm8,%xmm3,%xmm2
  196,65,105,223,192,                     //vpandn        %xmm8,%xmm2,%xmm8
  197,225,101,208,                        //vpcmpgtw      %xmm0,%xmm3,%xmm2
  197,233,223,192,                        //vpandn        %xmm0,%xmm2,%xmm0
  197,225,101,209,                        //vpcmpgtw      %xmm1,%xmm3,%xmm2
  197,233,223,201,                        //vpandn        %xmm1,%xmm2,%xmm1
  196,193,97,101,209,                     //vpcmpgtw      %xmm9,%xmm3,%xmm2
  196,193,105,223,209,                    //vpandn        %xmm9,%xmm2,%xmm2
  196,66,121,51,208,                      //vpmovzxwd     %xmm8,%xmm10
  196,98,121,51,201,                      //vpmovzxwd     %xmm1,%xmm9
  197,225,239,219,                        //vpxor         %xmm3,%xmm3,%xmm3
  197,57,105,195,                         //vpunpckhwd    %xmm3,%xmm8,%xmm8
  197,241,105,203,                        //vpunpckhwd    %xmm3,%xmm1,%xmm1
  196,98,121,51,216,                      //vpmovzxwd     %xmm0,%xmm11
  196,98,121,51,226,                      //vpmovzxwd     %xmm2,%xmm12
  197,121,105,235,                        //vpunpckhwd    %xmm3,%xmm0,%xmm13
  197,105,105,243,                        //vpunpckhwd    %xmm3,%xmm2,%xmm14
  196,193,121,114,242,13,                 //vpslld        $0xd,%xmm10,%xmm0
  196,193,105,114,241,13,                 //vpslld        $0xd,%xmm9,%xmm2
  196,227,125,24,194,1,                   //vinsertf128   $0x1,%xmm2,%ymm0,%ymm0
  196,98,125,24,74,92,                    //vbroadcastss  0x5c(%rdx),%ymm9
  197,180,89,192,                         //vmulps        %ymm0,%ymm9,%ymm0
  196,193,105,114,240,13,                 //vpslld        $0xd,%xmm8,%xmm2
  197,241,114,241,13,                     //vpslld        $0xd,%xmm1,%xmm1
  196,227,109,24,201,1,                   //vinsertf128   $0x1,%xmm1,%ymm2,%ymm1
  197,180,89,201,                         //vmulps        %ymm1,%ymm9,%ymm1
  196,193,105,114,243,13,                 //vpslld        $0xd,%xmm11,%xmm2
  196,193,97,114,244,13,                  //vpslld        $0xd,%xmm12,%xmm3
  196,227,109,24,211,1,                   //vinsertf128   $0x1,%xmm3,%ymm2,%ymm2
  197,180,89,210,                         //vmulps        %ymm2,%ymm9,%ymm2
  196,193,57,114,245,13,                  //vpslld        $0xd,%xmm13,%xmm8
  196,193,97,114,246,13,                  //vpslld        $0xd,%xmm14,%xmm3
  196,227,61,24,219,1,                    //vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  197,180,89,219,                         //vmulps        %ymm3,%ymm9,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  197,251,16,12,248,                      //vmovsd        (%rax,%rdi,8),%xmm1
  196,65,57,87,192,                       //vxorpd        %xmm8,%xmm8,%xmm8
  72,131,249,1,                           //cmp           $0x1,%rcx
  117,6,                                  //jne           10c8 <_sk_load_f16_avx+0x114>
  197,250,126,201,                        //vmovq         %xmm1,%xmm1
  235,30,                                 //jmp           10e6 <_sk_load_f16_avx+0x132>
  197,241,22,76,248,8,                    //vmovhpd       0x8(%rax,%rdi,8),%xmm1,%xmm1
  72,131,249,3,                           //cmp           $0x3,%rcx
  114,18,                                 //jb            10e6 <_sk_load_f16_avx+0x132>
  197,251,16,84,248,16,                   //vmovsd        0x10(%rax,%rdi,8),%xmm2
  72,131,249,3,                           //cmp           $0x3,%rcx
  117,19,                                 //jne           10f3 <_sk_load_f16_avx+0x13f>
  197,250,126,210,                        //vmovq         %xmm2,%xmm2
  235,46,                                 //jmp           1114 <_sk_load_f16_avx+0x160>
  197,225,87,219,                         //vxorpd        %xmm3,%xmm3,%xmm3
  197,233,87,210,                         //vxorpd        %xmm2,%xmm2,%xmm2
  233,230,254,255,255,                    //jmpq          fd9 <_sk_load_f16_avx+0x25>
  197,233,22,84,248,24,                   //vmovhpd       0x18(%rax,%rdi,8),%xmm2,%xmm2
  72,131,249,5,                           //cmp           $0x5,%rcx
  114,21,                                 //jb            1114 <_sk_load_f16_avx+0x160>
  197,251,16,92,248,32,                   //vmovsd        0x20(%rax,%rdi,8),%xmm3
  72,131,249,5,                           //cmp           $0x5,%rcx
  117,18,                                 //jne           111d <_sk_load_f16_avx+0x169>
  197,250,126,219,                        //vmovq         %xmm3,%xmm3
  233,197,254,255,255,                    //jmpq          fd9 <_sk_load_f16_avx+0x25>
  197,225,87,219,                         //vxorpd        %xmm3,%xmm3,%xmm3
  233,188,254,255,255,                    //jmpq          fd9 <_sk_load_f16_avx+0x25>
  197,225,22,92,248,40,                   //vmovhpd       0x28(%rax,%rdi,8),%xmm3,%xmm3
  72,131,249,7,                           //cmp           $0x7,%rcx
  15,130,172,254,255,255,                 //jb            fd9 <_sk_load_f16_avx+0x25>
  197,123,16,68,248,48,                   //vmovsd        0x30(%rax,%rdi,8),%xmm8
  233,161,254,255,255,                    //jmpq          fd9 <_sk_load_f16_avx+0x25>
};

CODE const uint8_t sk_store_f16_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  196,98,125,24,66,96,                    //vbroadcastss  0x60(%rdx),%ymm8
  197,60,89,200,                          //vmulps        %ymm0,%ymm8,%ymm9
  196,67,125,25,202,1,                    //vextractf128  $0x1,%ymm9,%xmm10
  196,193,41,114,210,13,                  //vpsrld        $0xd,%xmm10,%xmm10
  196,193,49,114,209,13,                  //vpsrld        $0xd,%xmm9,%xmm9
  197,60,89,217,                          //vmulps        %ymm1,%ymm8,%ymm11
  196,67,125,25,220,1,                    //vextractf128  $0x1,%ymm11,%xmm12
  196,193,25,114,212,13,                  //vpsrld        $0xd,%xmm12,%xmm12
  196,193,33,114,211,13,                  //vpsrld        $0xd,%xmm11,%xmm11
  197,60,89,234,                          //vmulps        %ymm2,%ymm8,%ymm13
  196,67,125,25,238,1,                    //vextractf128  $0x1,%ymm13,%xmm14
  196,193,9,114,214,13,                   //vpsrld        $0xd,%xmm14,%xmm14
  196,193,17,114,213,13,                  //vpsrld        $0xd,%xmm13,%xmm13
  197,60,89,195,                          //vmulps        %ymm3,%ymm8,%ymm8
  196,67,125,25,199,1,                    //vextractf128  $0x1,%ymm8,%xmm15
  196,193,1,114,215,13,                   //vpsrld        $0xd,%xmm15,%xmm15
  196,193,57,114,208,13,                  //vpsrld        $0xd,%xmm8,%xmm8
  196,193,33,115,251,2,                   //vpslldq       $0x2,%xmm11,%xmm11
  196,65,33,235,201,                      //vpor          %xmm9,%xmm11,%xmm9
  196,193,33,115,252,2,                   //vpslldq       $0x2,%xmm12,%xmm11
  196,65,33,235,226,                      //vpor          %xmm10,%xmm11,%xmm12
  196,193,57,115,248,2,                   //vpslldq       $0x2,%xmm8,%xmm8
  196,65,57,235,197,                      //vpor          %xmm13,%xmm8,%xmm8
  196,193,41,115,255,2,                   //vpslldq       $0x2,%xmm15,%xmm10
  196,65,41,235,238,                      //vpor          %xmm14,%xmm10,%xmm13
  196,65,49,98,216,                       //vpunpckldq    %xmm8,%xmm9,%xmm11
  196,65,49,106,208,                      //vpunpckhdq    %xmm8,%xmm9,%xmm10
  196,65,25,98,205,                       //vpunpckldq    %xmm13,%xmm12,%xmm9
  196,65,25,106,197,                      //vpunpckhdq    %xmm13,%xmm12,%xmm8
  72,133,201,                             //test          %rcx,%rcx
  117,27,                                 //jne           11fb <_sk_store_f16_avx+0xc3>
  197,120,17,28,248,                      //vmovups       %xmm11,(%rax,%rdi,8)
  197,120,17,84,248,16,                   //vmovups       %xmm10,0x10(%rax,%rdi,8)
  197,120,17,76,248,32,                   //vmovups       %xmm9,0x20(%rax,%rdi,8)
  197,122,127,68,248,48,                  //vmovdqu       %xmm8,0x30(%rax,%rdi,8)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  197,121,214,28,248,                     //vmovq         %xmm11,(%rax,%rdi,8)
  72,131,249,1,                           //cmp           $0x1,%rcx
  116,241,                                //je            11f7 <_sk_store_f16_avx+0xbf>
  197,121,23,92,248,8,                    //vmovhpd       %xmm11,0x8(%rax,%rdi,8)
  72,131,249,3,                           //cmp           $0x3,%rcx
  114,229,                                //jb            11f7 <_sk_store_f16_avx+0xbf>
  197,121,214,84,248,16,                  //vmovq         %xmm10,0x10(%rax,%rdi,8)
  116,221,                                //je            11f7 <_sk_store_f16_avx+0xbf>
  197,121,23,84,248,24,                   //vmovhpd       %xmm10,0x18(%rax,%rdi,8)
  72,131,249,5,                           //cmp           $0x5,%rcx
  114,209,                                //jb            11f7 <_sk_store_f16_avx+0xbf>
  197,121,214,76,248,32,                  //vmovq         %xmm9,0x20(%rax,%rdi,8)
  116,201,                                //je            11f7 <_sk_store_f16_avx+0xbf>
  197,121,23,76,248,40,                   //vmovhpd       %xmm9,0x28(%rax,%rdi,8)
  72,131,249,7,                           //cmp           $0x7,%rcx
  114,189,                                //jb            11f7 <_sk_store_f16_avx+0xbf>
  197,121,214,68,248,48,                  //vmovq         %xmm8,0x30(%rax,%rdi,8)
  235,181,                                //jmp           11f7 <_sk_store_f16_avx+0xbf>
};

CODE const uint8_t sk_store_f32_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  76,139,0,                               //mov           (%rax),%r8
  72,141,4,189,0,0,0,0,                   //lea           0x0(,%rdi,4),%rax
  197,124,20,193,                         //vunpcklps     %ymm1,%ymm0,%ymm8
  197,124,21,217,                         //vunpckhps     %ymm1,%ymm0,%ymm11
  197,108,20,203,                         //vunpcklps     %ymm3,%ymm2,%ymm9
  197,108,21,227,                         //vunpckhps     %ymm3,%ymm2,%ymm12
  196,65,61,20,209,                       //vunpcklpd     %ymm9,%ymm8,%ymm10
  196,65,61,21,201,                       //vunpckhpd     %ymm9,%ymm8,%ymm9
  196,65,37,20,196,                       //vunpcklpd     %ymm12,%ymm11,%ymm8
  196,65,37,21,220,                       //vunpckhpd     %ymm12,%ymm11,%ymm11
  72,133,201,                             //test          %rcx,%rcx
  117,55,                                 //jne           12af <_sk_store_f32_avx+0x6d>
  196,67,45,24,225,1,                     //vinsertf128   $0x1,%xmm9,%ymm10,%ymm12
  196,67,61,24,235,1,                     //vinsertf128   $0x1,%xmm11,%ymm8,%ymm13
  196,67,45,6,201,49,                     //vperm2f128    $0x31,%ymm9,%ymm10,%ymm9
  196,67,61,6,195,49,                     //vperm2f128    $0x31,%ymm11,%ymm8,%ymm8
  196,65,125,17,36,128,                   //vmovupd       %ymm12,(%r8,%rax,4)
  196,65,125,17,108,128,32,               //vmovupd       %ymm13,0x20(%r8,%rax,4)
  196,65,125,17,76,128,64,                //vmovupd       %ymm9,0x40(%r8,%rax,4)
  196,65,125,17,68,128,96,                //vmovupd       %ymm8,0x60(%r8,%rax,4)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
  196,65,121,17,20,128,                   //vmovupd       %xmm10,(%r8,%rax,4)
  72,131,249,1,                           //cmp           $0x1,%rcx
  116,240,                                //je            12ab <_sk_store_f32_avx+0x69>
  196,65,121,17,76,128,16,                //vmovupd       %xmm9,0x10(%r8,%rax,4)
  72,131,249,3,                           //cmp           $0x3,%rcx
  114,227,                                //jb            12ab <_sk_store_f32_avx+0x69>
  196,65,121,17,68,128,32,                //vmovupd       %xmm8,0x20(%r8,%rax,4)
  116,218,                                //je            12ab <_sk_store_f32_avx+0x69>
  196,65,121,17,92,128,48,                //vmovupd       %xmm11,0x30(%r8,%rax,4)
  72,131,249,5,                           //cmp           $0x5,%rcx
  114,205,                                //jb            12ab <_sk_store_f32_avx+0x69>
  196,67,125,25,84,128,64,1,              //vextractf128  $0x1,%ymm10,0x40(%r8,%rax,4)
  116,195,                                //je            12ab <_sk_store_f32_avx+0x69>
  196,67,125,25,76,128,80,1,              //vextractf128  $0x1,%ymm9,0x50(%r8,%rax,4)
  72,131,249,7,                           //cmp           $0x7,%rcx
  114,181,                                //jb            12ab <_sk_store_f32_avx+0x69>
  196,67,125,25,68,128,96,1,              //vextractf128  $0x1,%ymm8,0x60(%r8,%rax,4)
  235,171,                                //jmp           12ab <_sk_store_f32_avx+0x69>
};

CODE const uint8_t sk_clamp_x_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  197,60,95,200,                          //vmaxps        %ymm0,%ymm8,%ymm9
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,99,125,25,192,1,                    //vextractf128  $0x1,%ymm8,%xmm0
  196,65,41,118,210,                      //vpcmpeqd      %xmm10,%xmm10,%xmm10
  196,193,121,254,194,                    //vpaddd        %xmm10,%xmm0,%xmm0
  196,65,57,254,194,                      //vpaddd        %xmm10,%xmm8,%xmm8
  196,227,61,24,192,1,                    //vinsertf128   $0x1,%xmm0,%ymm8,%ymm0
  197,180,93,192,                         //vminps        %ymm0,%ymm9,%ymm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_y_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  197,60,95,201,                          //vmaxps        %ymm1,%ymm8,%ymm9
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,99,125,25,193,1,                    //vextractf128  $0x1,%ymm8,%xmm1
  196,65,41,118,210,                      //vpcmpeqd      %xmm10,%xmm10,%xmm10
  196,193,113,254,202,                    //vpaddd        %xmm10,%xmm1,%xmm1
  196,65,57,254,194,                      //vpaddd        %xmm10,%xmm8,%xmm8
  196,227,61,24,201,1,                    //vinsertf128   $0x1,%xmm1,%ymm8,%ymm1
  197,180,93,201,                         //vminps        %ymm1,%ymm9,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_repeat_x_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,65,124,94,200,                      //vdivps        %ymm8,%ymm0,%ymm9
  196,67,125,8,201,1,                     //vroundps      $0x1,%ymm9,%ymm9
  196,65,52,89,200,                       //vmulps        %ymm8,%ymm9,%ymm9
  196,65,124,92,201,                      //vsubps        %ymm9,%ymm0,%ymm9
  196,99,125,25,192,1,                    //vextractf128  $0x1,%ymm8,%xmm0
  196,65,41,118,210,                      //vpcmpeqd      %xmm10,%xmm10,%xmm10
  196,193,121,254,194,                    //vpaddd        %xmm10,%xmm0,%xmm0
  196,65,57,254,194,                      //vpaddd        %xmm10,%xmm8,%xmm8
  196,227,61,24,192,1,                    //vinsertf128   $0x1,%xmm0,%ymm8,%ymm0
  197,180,93,192,                         //vminps        %ymm0,%ymm9,%ymm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_repeat_y_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,65,116,94,200,                      //vdivps        %ymm8,%ymm1,%ymm9
  196,67,125,8,201,1,                     //vroundps      $0x1,%ymm9,%ymm9
  196,65,52,89,200,                       //vmulps        %ymm8,%ymm9,%ymm9
  196,65,116,92,201,                      //vsubps        %ymm9,%ymm1,%ymm9
  196,99,125,25,193,1,                    //vextractf128  $0x1,%ymm8,%xmm1
  196,65,41,118,210,                      //vpcmpeqd      %xmm10,%xmm10,%xmm10
  196,193,113,254,202,                    //vpaddd        %xmm10,%xmm1,%xmm1
  196,65,57,254,194,                      //vpaddd        %xmm10,%xmm8,%xmm8
  196,227,61,24,201,1,                    //vinsertf128   $0x1,%xmm1,%ymm8,%ymm1
  197,180,93,201,                         //vminps        %ymm1,%ymm9,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_mirror_x_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,122,16,0,                           //vmovss        (%rax),%xmm8
  196,65,121,112,200,0,                   //vpshufd       $0x0,%xmm8,%xmm9
  196,67,53,24,201,1,                     //vinsertf128   $0x1,%xmm9,%ymm9,%ymm9
  196,65,124,92,209,                      //vsubps        %ymm9,%ymm0,%ymm10
  196,193,58,88,192,                      //vaddss        %xmm8,%xmm8,%xmm0
  196,227,121,4,192,0,                    //vpermilps     $0x0,%xmm0,%xmm0
  196,227,125,24,192,1,                   //vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  197,44,94,192,                          //vdivps        %ymm0,%ymm10,%ymm8
  196,67,125,8,192,1,                     //vroundps      $0x1,%ymm8,%ymm8
  197,188,89,192,                         //vmulps        %ymm0,%ymm8,%ymm0
  197,172,92,192,                         //vsubps        %ymm0,%ymm10,%ymm0
  196,193,124,92,193,                     //vsubps        %ymm9,%ymm0,%ymm0
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  197,60,92,192,                          //vsubps        %ymm0,%ymm8,%ymm8
  197,60,84,192,                          //vandps        %ymm0,%ymm8,%ymm8
  196,99,125,25,200,1,                    //vextractf128  $0x1,%ymm9,%xmm0
  196,65,41,118,210,                      //vpcmpeqd      %xmm10,%xmm10,%xmm10
  196,193,121,254,194,                    //vpaddd        %xmm10,%xmm0,%xmm0
  196,65,49,254,202,                      //vpaddd        %xmm10,%xmm9,%xmm9
  196,227,53,24,192,1,                    //vinsertf128   $0x1,%xmm0,%ymm9,%ymm0
  197,188,93,192,                         //vminps        %ymm0,%ymm8,%ymm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_mirror_y_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,122,16,0,                           //vmovss        (%rax),%xmm8
  196,65,121,112,200,0,                   //vpshufd       $0x0,%xmm8,%xmm9
  196,67,53,24,201,1,                     //vinsertf128   $0x1,%xmm9,%ymm9,%ymm9
  196,65,116,92,209,                      //vsubps        %ymm9,%ymm1,%ymm10
  196,193,58,88,200,                      //vaddss        %xmm8,%xmm8,%xmm1
  196,227,121,4,201,0,                    //vpermilps     $0x0,%xmm1,%xmm1
  196,227,117,24,201,1,                   //vinsertf128   $0x1,%xmm1,%ymm1,%ymm1
  197,44,94,193,                          //vdivps        %ymm1,%ymm10,%ymm8
  196,67,125,8,192,1,                     //vroundps      $0x1,%ymm8,%ymm8
  197,188,89,201,                         //vmulps        %ymm1,%ymm8,%ymm1
  197,172,92,201,                         //vsubps        %ymm1,%ymm10,%ymm1
  196,193,116,92,201,                     //vsubps        %ymm9,%ymm1,%ymm1
  196,65,60,87,192,                       //vxorps        %ymm8,%ymm8,%ymm8
  197,60,92,193,                          //vsubps        %ymm1,%ymm8,%ymm8
  197,60,84,193,                          //vandps        %ymm1,%ymm8,%ymm8
  196,99,125,25,201,1,                    //vextractf128  $0x1,%ymm9,%xmm1
  196,65,41,118,210,                      //vpcmpeqd      %xmm10,%xmm10,%xmm10
  196,193,113,254,202,                    //vpaddd        %xmm10,%xmm1,%xmm1
  196,65,49,254,202,                      //vpaddd        %xmm10,%xmm9,%xmm9
  196,227,53,24,201,1,                    //vinsertf128   $0x1,%xmm1,%ymm9,%ymm1
  197,188,93,201,                         //vminps        %ymm1,%ymm8,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_2x3_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,98,125,24,72,8,                     //vbroadcastss  0x8(%rax),%ymm9
  196,98,125,24,80,16,                    //vbroadcastss  0x10(%rax),%ymm10
  197,52,89,201,                          //vmulps        %ymm1,%ymm9,%ymm9
  196,65,52,88,202,                       //vaddps        %ymm10,%ymm9,%ymm9
  197,60,89,192,                          //vmulps        %ymm0,%ymm8,%ymm8
  196,65,60,88,193,                       //vaddps        %ymm9,%ymm8,%ymm8
  196,98,125,24,72,4,                     //vbroadcastss  0x4(%rax),%ymm9
  196,98,125,24,80,12,                    //vbroadcastss  0xc(%rax),%ymm10
  196,98,125,24,88,20,                    //vbroadcastss  0x14(%rax),%ymm11
  197,172,89,201,                         //vmulps        %ymm1,%ymm10,%ymm1
  196,193,116,88,203,                     //vaddps        %ymm11,%ymm1,%ymm1
  197,180,89,192,                         //vmulps        %ymm0,%ymm9,%ymm0
  197,252,88,201,                         //vaddps        %ymm1,%ymm0,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,124,41,192,                         //vmovaps       %ymm8,%ymm0
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_3x4_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,98,125,24,72,12,                    //vbroadcastss  0xc(%rax),%ymm9
  196,98,125,24,80,24,                    //vbroadcastss  0x18(%rax),%ymm10
  196,98,125,24,88,36,                    //vbroadcastss  0x24(%rax),%ymm11
  197,44,89,210,                          //vmulps        %ymm2,%ymm10,%ymm10
  196,65,44,88,211,                       //vaddps        %ymm11,%ymm10,%ymm10
  197,52,89,201,                          //vmulps        %ymm1,%ymm9,%ymm9
  196,65,52,88,202,                       //vaddps        %ymm10,%ymm9,%ymm9
  197,60,89,192,                          //vmulps        %ymm0,%ymm8,%ymm8
  196,65,60,88,193,                       //vaddps        %ymm9,%ymm8,%ymm8
  196,98,125,24,72,4,                     //vbroadcastss  0x4(%rax),%ymm9
  196,98,125,24,80,16,                    //vbroadcastss  0x10(%rax),%ymm10
  196,98,125,24,88,28,                    //vbroadcastss  0x1c(%rax),%ymm11
  196,98,125,24,96,40,                    //vbroadcastss  0x28(%rax),%ymm12
  197,36,89,218,                          //vmulps        %ymm2,%ymm11,%ymm11
  196,65,36,88,220,                       //vaddps        %ymm12,%ymm11,%ymm11
  197,44,89,209,                          //vmulps        %ymm1,%ymm10,%ymm10
  196,65,44,88,211,                       //vaddps        %ymm11,%ymm10,%ymm10
  197,52,89,200,                          //vmulps        %ymm0,%ymm9,%ymm9
  196,65,52,88,202,                       //vaddps        %ymm10,%ymm9,%ymm9
  196,98,125,24,80,8,                     //vbroadcastss  0x8(%rax),%ymm10
  196,98,125,24,88,20,                    //vbroadcastss  0x14(%rax),%ymm11
  196,98,125,24,96,32,                    //vbroadcastss  0x20(%rax),%ymm12
  196,98,125,24,104,44,                   //vbroadcastss  0x2c(%rax),%ymm13
  197,156,89,210,                         //vmulps        %ymm2,%ymm12,%ymm2
  196,193,108,88,213,                     //vaddps        %ymm13,%ymm2,%ymm2
  197,164,89,201,                         //vmulps        %ymm1,%ymm11,%ymm1
  197,244,88,202,                         //vaddps        %ymm2,%ymm1,%ymm1
  197,172,89,192,                         //vmulps        %ymm0,%ymm10,%ymm0
  197,252,88,209,                         //vaddps        %ymm1,%ymm0,%ymm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,124,41,192,                         //vmovaps       %ymm8,%ymm0
  197,124,41,201,                         //vmovaps       %ymm9,%ymm1
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_perspective_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,98,125,24,0,                        //vbroadcastss  (%rax),%ymm8
  196,98,125,24,72,4,                     //vbroadcastss  0x4(%rax),%ymm9
  196,98,125,24,80,8,                     //vbroadcastss  0x8(%rax),%ymm10
  197,52,89,201,                          //vmulps        %ymm1,%ymm9,%ymm9
  196,65,52,88,202,                       //vaddps        %ymm10,%ymm9,%ymm9
  197,60,89,192,                          //vmulps        %ymm0,%ymm8,%ymm8
  196,65,60,88,193,                       //vaddps        %ymm9,%ymm8,%ymm8
  196,98,125,24,72,12,                    //vbroadcastss  0xc(%rax),%ymm9
  196,98,125,24,80,16,                    //vbroadcastss  0x10(%rax),%ymm10
  196,98,125,24,88,20,                    //vbroadcastss  0x14(%rax),%ymm11
  197,44,89,209,                          //vmulps        %ymm1,%ymm10,%ymm10
  196,65,44,88,211,                       //vaddps        %ymm11,%ymm10,%ymm10
  197,52,89,200,                          //vmulps        %ymm0,%ymm9,%ymm9
  196,65,52,88,202,                       //vaddps        %ymm10,%ymm9,%ymm9
  196,98,125,24,80,24,                    //vbroadcastss  0x18(%rax),%ymm10
  196,98,125,24,88,28,                    //vbroadcastss  0x1c(%rax),%ymm11
  196,98,125,24,96,32,                    //vbroadcastss  0x20(%rax),%ymm12
  197,164,89,201,                         //vmulps        %ymm1,%ymm11,%ymm1
  196,193,116,88,204,                     //vaddps        %ymm12,%ymm1,%ymm1
  197,172,89,192,                         //vmulps        %ymm0,%ymm10,%ymm0
  197,252,88,193,                         //vaddps        %ymm1,%ymm0,%ymm0
  197,252,83,200,                         //vrcpps        %ymm0,%ymm1
  197,188,89,193,                         //vmulps        %ymm1,%ymm8,%ymm0
  197,180,89,201,                         //vmulps        %ymm1,%ymm9,%ymm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_linear_gradient_2stops_avx[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  196,226,125,24,72,16,                   //vbroadcastss  0x10(%rax),%ymm1
  196,226,125,24,16,                      //vbroadcastss  (%rax),%ymm2
  197,244,89,200,                         //vmulps        %ymm0,%ymm1,%ymm1
  197,108,88,193,                         //vaddps        %ymm1,%ymm2,%ymm8
  196,226,125,24,72,20,                   //vbroadcastss  0x14(%rax),%ymm1
  196,226,125,24,80,4,                    //vbroadcastss  0x4(%rax),%ymm2
  197,244,89,200,                         //vmulps        %ymm0,%ymm1,%ymm1
  197,236,88,201,                         //vaddps        %ymm1,%ymm2,%ymm1
  196,226,125,24,80,24,                   //vbroadcastss  0x18(%rax),%ymm2
  196,226,125,24,88,8,                    //vbroadcastss  0x8(%rax),%ymm3
  197,236,89,208,                         //vmulps        %ymm0,%ymm2,%ymm2
  197,228,88,210,                         //vaddps        %ymm2,%ymm3,%ymm2
  196,226,125,24,88,28,                   //vbroadcastss  0x1c(%rax),%ymm3
  196,98,125,24,72,12,                    //vbroadcastss  0xc(%rax),%ymm9
  197,228,89,192,                         //vmulps        %ymm0,%ymm3,%ymm0
  197,180,88,216,                         //vaddps        %ymm0,%ymm9,%ymm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  197,124,41,192,                         //vmovaps       %ymm8,%ymm0
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_start_pipeline_sse41[] = {
  65,87,                                  //push          %r15
  65,86,                                  //push          %r14
  65,85,                                  //push          %r13
  65,84,                                  //push          %r12
  86,                                     //push          %rsi
  87,                                     //push          %rdi
  83,                                     //push          %rbx
  72,129,236,160,0,0,0,                   //sub           $0xa0,%rsp
  68,15,41,188,36,144,0,0,0,              //movaps        %xmm15,0x90(%rsp)
  68,15,41,180,36,128,0,0,0,              //movaps        %xmm14,0x80(%rsp)
  68,15,41,108,36,112,                    //movaps        %xmm13,0x70(%rsp)
  68,15,41,100,36,96,                     //movaps        %xmm12,0x60(%rsp)
  68,15,41,92,36,80,                      //movaps        %xmm11,0x50(%rsp)
  68,15,41,84,36,64,                      //movaps        %xmm10,0x40(%rsp)
  68,15,41,76,36,48,                      //movaps        %xmm9,0x30(%rsp)
  68,15,41,68,36,32,                      //movaps        %xmm8,0x20(%rsp)
  15,41,124,36,16,                        //movaps        %xmm7,0x10(%rsp)
  15,41,52,36,                            //movaps        %xmm6,(%rsp)
  77,137,207,                             //mov           %r9,%r15
  77,137,198,                             //mov           %r8,%r14
  72,137,203,                             //mov           %rcx,%rbx
  72,137,214,                             //mov           %rdx,%rsi
  72,173,                                 //lods          %ds:(%rsi),%rax
  73,137,196,                             //mov           %rax,%r12
  73,137,245,                             //mov           %rsi,%r13
  72,141,67,4,                            //lea           0x4(%rbx),%rax
  76,57,248,                              //cmp           %r15,%rax
  118,5,                                  //jbe           73 <_sk_start_pipeline_sse41+0x73>
  72,137,216,                             //mov           %rbx,%rax
  235,52,                                 //jmp           a7 <_sk_start_pipeline_sse41+0xa7>
  15,87,192,                              //xorps         %xmm0,%xmm0
  15,87,201,                              //xorps         %xmm1,%xmm1
  15,87,210,                              //xorps         %xmm2,%xmm2
  15,87,219,                              //xorps         %xmm3,%xmm3
  15,87,228,                              //xorps         %xmm4,%xmm4
  15,87,237,                              //xorps         %xmm5,%xmm5
  15,87,246,                              //xorps         %xmm6,%xmm6
  15,87,255,                              //xorps         %xmm7,%xmm7
  72,137,223,                             //mov           %rbx,%rdi
  76,137,238,                             //mov           %r13,%rsi
  76,137,242,                             //mov           %r14,%rdx
  65,255,212,                             //callq         *%r12
  72,141,67,4,                            //lea           0x4(%rbx),%rax
  72,131,195,8,                           //add           $0x8,%rbx
  76,57,251,                              //cmp           %r15,%rbx
  72,137,195,                             //mov           %rax,%rbx
  118,204,                                //jbe           73 <_sk_start_pipeline_sse41+0x73>
  15,40,52,36,                            //movaps        (%rsp),%xmm6
  15,40,124,36,16,                        //movaps        0x10(%rsp),%xmm7
  68,15,40,68,36,32,                      //movaps        0x20(%rsp),%xmm8
  68,15,40,76,36,48,                      //movaps        0x30(%rsp),%xmm9
  68,15,40,84,36,64,                      //movaps        0x40(%rsp),%xmm10
  68,15,40,92,36,80,                      //movaps        0x50(%rsp),%xmm11
  68,15,40,100,36,96,                     //movaps        0x60(%rsp),%xmm12
  68,15,40,108,36,112,                    //movaps        0x70(%rsp),%xmm13
  68,15,40,180,36,128,0,0,0,              //movaps        0x80(%rsp),%xmm14
  68,15,40,188,36,144,0,0,0,              //movaps        0x90(%rsp),%xmm15
  72,129,196,160,0,0,0,                   //add           $0xa0,%rsp
  91,                                     //pop           %rbx
  95,                                     //pop           %rdi
  94,                                     //pop           %rsi
  65,92,                                  //pop           %r12
  65,93,                                  //pop           %r13
  65,94,                                  //pop           %r14
  65,95,                                  //pop           %r15
  195,                                    //retq
};

CODE const uint8_t sk_just_return_sse41[] = {
  195,                                    //retq
};

CODE const uint8_t sk_seed_shader_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  102,15,110,199,                         //movd          %edi,%xmm0
  102,15,112,192,0,                       //pshufd        $0x0,%xmm0,%xmm0
  15,91,200,                              //cvtdq2ps      %xmm0,%xmm1
  243,15,16,18,                           //movss         (%rdx),%xmm2
  243,15,16,90,4,                         //movss         0x4(%rdx),%xmm3
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  15,88,203,                              //addps         %xmm3,%xmm1
  15,16,66,20,                            //movups        0x14(%rdx),%xmm0
  15,88,193,                              //addps         %xmm1,%xmm0
  102,15,110,8,                           //movd          (%rax),%xmm1
  102,15,112,201,0,                       //pshufd        $0x0,%xmm1,%xmm1
  15,91,201,                              //cvtdq2ps      %xmm1,%xmm1
  15,88,203,                              //addps         %xmm3,%xmm1
  15,198,210,0,                           //shufps        $0x0,%xmm2,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,87,219,                              //xorps         %xmm3,%xmm3
  15,87,228,                              //xorps         %xmm4,%xmm4
  15,87,237,                              //xorps         %xmm5,%xmm5
  15,87,246,                              //xorps         %xmm6,%xmm6
  15,87,255,                              //xorps         %xmm7,%xmm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_constant_color_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,16,24,                               //movups        (%rax),%xmm3
  15,40,195,                              //movaps        %xmm3,%xmm0
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  15,40,203,                              //movaps        %xmm3,%xmm1
  15,198,201,85,                          //shufps        $0x55,%xmm1,%xmm1
  15,40,211,                              //movaps        %xmm3,%xmm2
  15,198,210,170,                         //shufps        $0xaa,%xmm2,%xmm2
  15,198,219,255,                         //shufps        $0xff,%xmm3,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clear_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,87,192,                              //xorps         %xmm0,%xmm0
  15,87,201,                              //xorps         %xmm1,%xmm1
  15,87,210,                              //xorps         %xmm2,%xmm2
  15,87,219,                              //xorps         %xmm3,%xmm3
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_plus__sse41[] = {
  15,88,196,                              //addps         %xmm4,%xmm0
  15,88,205,                              //addps         %xmm5,%xmm1
  15,88,214,                              //addps         %xmm6,%xmm2
  15,88,223,                              //addps         %xmm7,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_srcover_sse41[] = {
  243,68,15,16,2,                         //movss         (%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,92,195,                           //subps         %xmm3,%xmm8
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,204,                           //mulps         %xmm4,%xmm9
  65,15,88,193,                           //addps         %xmm9,%xmm0
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,205,                           //mulps         %xmm5,%xmm9
  65,15,88,201,                           //addps         %xmm9,%xmm1
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,206,                           //mulps         %xmm6,%xmm9
  65,15,88,209,                           //addps         %xmm9,%xmm2
  68,15,89,199,                           //mulps         %xmm7,%xmm8
  65,15,88,216,                           //addps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_dstover_sse41[] = {
  243,68,15,16,2,                         //movss         (%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,92,199,                           //subps         %xmm7,%xmm8
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  15,88,196,                              //addps         %xmm4,%xmm0
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  15,88,205,                              //addps         %xmm5,%xmm1
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  15,88,214,                              //addps         %xmm6,%xmm2
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  15,88,223,                              //addps         %xmm7,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_0_sse41[] = {
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  65,15,95,192,                           //maxps         %xmm8,%xmm0
  65,15,95,200,                           //maxps         %xmm8,%xmm1
  65,15,95,208,                           //maxps         %xmm8,%xmm2
  65,15,95,216,                           //maxps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_1_sse41[] = {
  243,68,15,16,2,                         //movss         (%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,93,192,                           //minps         %xmm8,%xmm0
  65,15,93,200,                           //minps         %xmm8,%xmm1
  65,15,93,208,                           //minps         %xmm8,%xmm2
  65,15,93,216,                           //minps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_a_sse41[] = {
  243,68,15,16,2,                         //movss         (%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,93,216,                           //minps         %xmm8,%xmm3
  15,93,195,                              //minps         %xmm3,%xmm0
  15,93,203,                              //minps         %xmm3,%xmm1
  15,93,211,                              //minps         %xmm3,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_set_rgb_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,15,16,0,                            //movss         (%rax),%xmm0
  243,15,16,72,4,                         //movss         0x4(%rax),%xmm1
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  243,15,16,80,8,                         //movss         0x8(%rax),%xmm2
  15,198,210,0,                           //shufps        $0x0,%xmm2,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_swap_rb_sse41[] = {
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,194,                              //movaps        %xmm2,%xmm0
  65,15,40,208,                           //movaps        %xmm8,%xmm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_swap_sse41[] = {
  68,15,40,195,                           //movaps        %xmm3,%xmm8
  68,15,40,202,                           //movaps        %xmm2,%xmm9
  68,15,40,209,                           //movaps        %xmm1,%xmm10
  68,15,40,216,                           //movaps        %xmm0,%xmm11
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,196,                              //movaps        %xmm4,%xmm0
  15,40,205,                              //movaps        %xmm5,%xmm1
  15,40,214,                              //movaps        %xmm6,%xmm2
  15,40,223,                              //movaps        %xmm7,%xmm3
  65,15,40,227,                           //movaps        %xmm11,%xmm4
  65,15,40,234,                           //movaps        %xmm10,%xmm5
  65,15,40,241,                           //movaps        %xmm9,%xmm6
  65,15,40,248,                           //movaps        %xmm8,%xmm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_move_src_dst_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,224,                              //movaps        %xmm0,%xmm4
  15,40,233,                              //movaps        %xmm1,%xmm5
  15,40,242,                              //movaps        %xmm2,%xmm6
  15,40,251,                              //movaps        %xmm3,%xmm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_move_dst_src_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,196,                              //movaps        %xmm4,%xmm0
  15,40,205,                              //movaps        %xmm5,%xmm1
  15,40,214,                              //movaps        %xmm6,%xmm2
  15,40,223,                              //movaps        %xmm7,%xmm3
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_premul_sse41[] = {
  15,89,195,                              //mulps         %xmm3,%xmm0
  15,89,203,                              //mulps         %xmm3,%xmm1
  15,89,211,                              //mulps         %xmm3,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_unpremul_sse41[] = {
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  69,15,87,201,                           //xorps         %xmm9,%xmm9
  243,68,15,16,18,                        //movss         (%rdx),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  68,15,94,211,                           //divps         %xmm3,%xmm10
  15,40,195,                              //movaps        %xmm3,%xmm0
  65,15,194,193,0,                        //cmpeqps       %xmm9,%xmm0
  102,69,15,56,20,209,                    //blendvps      %xmm0,%xmm9,%xmm10
  69,15,89,194,                           //mulps         %xmm10,%xmm8
  65,15,89,202,                           //mulps         %xmm10,%xmm1
  65,15,89,210,                           //mulps         %xmm10,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,192,                           //movaps        %xmm8,%xmm0
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_from_srgb_sse41[] = {
  68,15,40,194,                           //movaps        %xmm2,%xmm8
  243,68,15,16,90,64,                     //movss         0x40(%rdx),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,40,211,                           //movaps        %xmm11,%xmm10
  68,15,89,208,                           //mulps         %xmm0,%xmm10
  68,15,40,240,                           //movaps        %xmm0,%xmm14
  69,15,89,246,                           //mulps         %xmm14,%xmm14
  243,15,16,82,60,                        //movss         0x3c(%rdx),%xmm2
  15,198,210,0,                           //shufps        $0x0,%xmm2,%xmm2
  243,68,15,16,98,52,                     //movss         0x34(%rdx),%xmm12
  243,68,15,16,106,56,                    //movss         0x38(%rdx),%xmm13
  69,15,198,237,0,                        //shufps        $0x0,%xmm13,%xmm13
  68,15,40,202,                           //movaps        %xmm2,%xmm9
  68,15,89,200,                           //mulps         %xmm0,%xmm9
  69,15,88,205,                           //addps         %xmm13,%xmm9
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  69,15,89,206,                           //mulps         %xmm14,%xmm9
  69,15,88,204,                           //addps         %xmm12,%xmm9
  243,68,15,16,114,68,                    //movss         0x44(%rdx),%xmm14
  69,15,198,246,0,                        //shufps        $0x0,%xmm14,%xmm14
  65,15,194,198,1,                        //cmpltps       %xmm14,%xmm0
  102,69,15,56,20,202,                    //blendvps      %xmm0,%xmm10,%xmm9
  69,15,40,251,                           //movaps        %xmm11,%xmm15
  68,15,89,249,                           //mulps         %xmm1,%xmm15
  15,40,193,                              //movaps        %xmm1,%xmm0
  15,89,192,                              //mulps         %xmm0,%xmm0
  68,15,40,210,                           //movaps        %xmm2,%xmm10
  68,15,89,209,                           //mulps         %xmm1,%xmm10
  69,15,88,213,                           //addps         %xmm13,%xmm10
  68,15,89,208,                           //mulps         %xmm0,%xmm10
  69,15,88,212,                           //addps         %xmm12,%xmm10
  65,15,194,206,1,                        //cmpltps       %xmm14,%xmm1
  15,40,193,                              //movaps        %xmm1,%xmm0
  102,69,15,56,20,215,                    //blendvps      %xmm0,%xmm15,%xmm10
  69,15,89,216,                           //mulps         %xmm8,%xmm11
  65,15,40,192,                           //movaps        %xmm8,%xmm0
  15,89,192,                              //mulps         %xmm0,%xmm0
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  65,15,88,213,                           //addps         %xmm13,%xmm2
  15,89,208,                              //mulps         %xmm0,%xmm2
  65,15,88,212,                           //addps         %xmm12,%xmm2
  69,15,194,198,1,                        //cmpltps       %xmm14,%xmm8
  65,15,40,192,                           //movaps        %xmm8,%xmm0
  102,65,15,56,20,211,                    //blendvps      %xmm0,%xmm11,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,193,                           //movaps        %xmm9,%xmm0
  65,15,40,202,                           //movaps        %xmm10,%xmm1
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_to_srgb_sse41[] = {
  72,131,236,24,                          //sub           $0x18,%rsp
  15,41,60,36,                            //movaps        %xmm7,(%rsp)
  15,40,254,                              //movaps        %xmm6,%xmm7
  15,40,245,                              //movaps        %xmm5,%xmm6
  15,40,236,                              //movaps        %xmm4,%xmm5
  15,40,227,                              //movaps        %xmm3,%xmm4
  68,15,40,194,                           //movaps        %xmm2,%xmm8
  15,40,217,                              //movaps        %xmm1,%xmm3
  15,82,208,                              //rsqrtps       %xmm0,%xmm2
  68,15,83,202,                           //rcpps         %xmm2,%xmm9
  68,15,82,210,                           //rsqrtps       %xmm2,%xmm10
  243,15,16,18,                           //movss         (%rdx),%xmm2
  243,68,15,16,90,72,                     //movss         0x48(%rdx),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  65,15,40,203,                           //movaps        %xmm11,%xmm1
  15,89,200,                              //mulps         %xmm0,%xmm1
  15,198,210,0,                           //shufps        $0x0,%xmm2,%xmm2
  243,68,15,16,98,76,                     //movss         0x4c(%rdx),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  243,68,15,16,106,80,                    //movss         0x50(%rdx),%xmm13
  69,15,198,237,0,                        //shufps        $0x0,%xmm13,%xmm13
  243,68,15,16,114,84,                    //movss         0x54(%rdx),%xmm14
  69,15,198,246,0,                        //shufps        $0x0,%xmm14,%xmm14
  69,15,89,205,                           //mulps         %xmm13,%xmm9
  69,15,88,206,                           //addps         %xmm14,%xmm9
  69,15,89,212,                           //mulps         %xmm12,%xmm10
  69,15,88,209,                           //addps         %xmm9,%xmm10
  68,15,40,202,                           //movaps        %xmm2,%xmm9
  69,15,93,202,                           //minps         %xmm10,%xmm9
  243,68,15,16,122,88,                    //movss         0x58(%rdx),%xmm15
  69,15,198,255,0,                        //shufps        $0x0,%xmm15,%xmm15
  65,15,194,199,1,                        //cmpltps       %xmm15,%xmm0
  102,68,15,56,20,201,                    //blendvps      %xmm0,%xmm1,%xmm9
  15,82,195,                              //rsqrtps       %xmm3,%xmm0
  15,83,200,                              //rcpps         %xmm0,%xmm1
  15,82,192,                              //rsqrtps       %xmm0,%xmm0
  65,15,89,205,                           //mulps         %xmm13,%xmm1
  65,15,88,206,                           //addps         %xmm14,%xmm1
  65,15,89,196,                           //mulps         %xmm12,%xmm0
  15,88,193,                              //addps         %xmm1,%xmm0
  68,15,40,210,                           //movaps        %xmm2,%xmm10
  68,15,93,208,                           //minps         %xmm0,%xmm10
  65,15,40,203,                           //movaps        %xmm11,%xmm1
  15,89,203,                              //mulps         %xmm3,%xmm1
  65,15,194,223,1,                        //cmpltps       %xmm15,%xmm3
  15,40,195,                              //movaps        %xmm3,%xmm0
  102,68,15,56,20,209,                    //blendvps      %xmm0,%xmm1,%xmm10
  65,15,82,192,                           //rsqrtps       %xmm8,%xmm0
  15,83,200,                              //rcpps         %xmm0,%xmm1
  65,15,89,205,                           //mulps         %xmm13,%xmm1
  65,15,88,206,                           //addps         %xmm14,%xmm1
  15,82,192,                              //rsqrtps       %xmm0,%xmm0
  65,15,89,196,                           //mulps         %xmm12,%xmm0
  15,88,193,                              //addps         %xmm1,%xmm0
  15,93,208,                              //minps         %xmm0,%xmm2
  69,15,89,216,                           //mulps         %xmm8,%xmm11
  69,15,194,199,1,                        //cmpltps       %xmm15,%xmm8
  65,15,40,192,                           //movaps        %xmm8,%xmm0
  102,65,15,56,20,211,                    //blendvps      %xmm0,%xmm11,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,193,                           //movaps        %xmm9,%xmm0
  65,15,40,202,                           //movaps        %xmm10,%xmm1
  15,40,220,                              //movaps        %xmm4,%xmm3
  15,40,229,                              //movaps        %xmm5,%xmm4
  15,40,238,                              //movaps        %xmm6,%xmm5
  15,40,247,                              //movaps        %xmm7,%xmm6
  15,40,60,36,                            //movaps        (%rsp),%xmm7
  72,131,196,24,                          //add           $0x18,%rsp
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_scale_1_float_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_scale_u8_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,68,15,56,49,4,56,                   //pmovzxbd      (%rax,%rdi,1),%xmm8
  69,15,91,192,                           //cvtdq2ps      %xmm8,%xmm8
  243,68,15,16,74,12,                     //movss         0xc(%rdx),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  65,15,89,193,                           //mulps         %xmm9,%xmm0
  65,15,89,201,                           //mulps         %xmm9,%xmm1
  65,15,89,209,                           //mulps         %xmm9,%xmm2
  65,15,89,217,                           //mulps         %xmm9,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_lerp_1_float_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  15,92,196,                              //subps         %xmm4,%xmm0
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  15,88,196,                              //addps         %xmm4,%xmm0
  15,92,205,                              //subps         %xmm5,%xmm1
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  15,88,205,                              //addps         %xmm5,%xmm1
  15,92,214,                              //subps         %xmm6,%xmm2
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  15,88,214,                              //addps         %xmm6,%xmm2
  15,92,223,                              //subps         %xmm7,%xmm3
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  15,88,223,                              //addps         %xmm7,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_lerp_u8_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,68,15,56,49,4,56,                   //pmovzxbd      (%rax,%rdi,1),%xmm8
  69,15,91,192,                           //cvtdq2ps      %xmm8,%xmm8
  243,68,15,16,74,12,                     //movss         0xc(%rdx),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  15,92,196,                              //subps         %xmm4,%xmm0
  65,15,89,193,                           //mulps         %xmm9,%xmm0
  15,88,196,                              //addps         %xmm4,%xmm0
  15,92,205,                              //subps         %xmm5,%xmm1
  65,15,89,201,                           //mulps         %xmm9,%xmm1
  15,88,205,                              //addps         %xmm5,%xmm1
  15,92,214,                              //subps         %xmm6,%xmm2
  65,15,89,209,                           //mulps         %xmm9,%xmm2
  15,88,214,                              //addps         %xmm6,%xmm2
  15,92,223,                              //subps         %xmm7,%xmm3
  65,15,89,217,                           //mulps         %xmm9,%xmm3
  15,88,223,                              //addps         %xmm7,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_lerp_565_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,68,15,56,51,4,120,                  //pmovzxwd      (%rax,%rdi,2),%xmm8
  102,15,110,90,104,                      //movd          0x68(%rdx),%xmm3
  102,15,112,219,0,                       //pshufd        $0x0,%xmm3,%xmm3
  102,65,15,219,216,                      //pand          %xmm8,%xmm3
  68,15,91,203,                           //cvtdq2ps      %xmm3,%xmm9
  243,15,16,26,                           //movss         (%rdx),%xmm3
  243,68,15,16,82,116,                    //movss         0x74(%rdx),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  102,68,15,110,74,108,                   //movd          0x6c(%rdx),%xmm9
  102,69,15,112,201,0,                    //pshufd        $0x0,%xmm9,%xmm9
  102,69,15,219,200,                      //pand          %xmm8,%xmm9
  69,15,91,201,                           //cvtdq2ps      %xmm9,%xmm9
  243,68,15,16,90,120,                    //movss         0x78(%rdx),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,89,217,                           //mulps         %xmm9,%xmm11
  102,68,15,110,74,112,                   //movd          0x70(%rdx),%xmm9
  102,69,15,112,201,0,                    //pshufd        $0x0,%xmm9,%xmm9
  102,69,15,219,200,                      //pand          %xmm8,%xmm9
  69,15,91,193,                           //cvtdq2ps      %xmm9,%xmm8
  243,68,15,16,74,124,                    //movss         0x7c(%rdx),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  15,92,196,                              //subps         %xmm4,%xmm0
  65,15,89,194,                           //mulps         %xmm10,%xmm0
  15,88,196,                              //addps         %xmm4,%xmm0
  15,92,205,                              //subps         %xmm5,%xmm1
  65,15,89,203,                           //mulps         %xmm11,%xmm1
  15,88,205,                              //addps         %xmm5,%xmm1
  15,92,214,                              //subps         %xmm6,%xmm2
  65,15,89,209,                           //mulps         %xmm9,%xmm2
  15,88,214,                              //addps         %xmm6,%xmm2
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_tables_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,8,                               //mov           (%rax),%rcx
  76,139,64,8,                            //mov           0x8(%rax),%r8
  243,68,15,111,4,185,                    //movdqu        (%rcx,%rdi,4),%xmm8
  102,15,110,66,16,                       //movd          0x10(%rdx),%xmm0
  102,15,112,192,0,                       //pshufd        $0x0,%xmm0,%xmm0
  102,65,15,111,200,                      //movdqa        %xmm8,%xmm1
  102,15,114,209,8,                       //psrld         $0x8,%xmm1
  102,15,219,200,                         //pand          %xmm0,%xmm1
  102,65,15,111,208,                      //movdqa        %xmm8,%xmm2
  102,15,114,210,16,                      //psrld         $0x10,%xmm2
  102,15,219,208,                         //pand          %xmm0,%xmm2
  102,65,15,219,192,                      //pand          %xmm8,%xmm0
  102,72,15,58,22,193,1,                  //pextrq        $0x1,%xmm0,%rcx
  65,137,201,                             //mov           %ecx,%r9d
  72,193,233,32,                          //shr           $0x20,%rcx
  102,73,15,126,194,                      //movq          %xmm0,%r10
  69,137,211,                             //mov           %r10d,%r11d
  73,193,234,32,                          //shr           $0x20,%r10
  243,67,15,16,4,152,                     //movss         (%r8,%r11,4),%xmm0
  102,67,15,58,33,4,144,16,               //insertps      $0x10,(%r8,%r10,4),%xmm0
  102,67,15,58,33,4,136,32,               //insertps      $0x20,(%r8,%r9,4),%xmm0
  102,65,15,58,33,4,136,48,               //insertps      $0x30,(%r8,%rcx,4),%xmm0
  72,139,72,16,                           //mov           0x10(%rax),%rcx
  102,73,15,58,22,200,1,                  //pextrq        $0x1,%xmm1,%r8
  69,137,193,                             //mov           %r8d,%r9d
  73,193,232,32,                          //shr           $0x20,%r8
  102,73,15,126,202,                      //movq          %xmm1,%r10
  69,137,211,                             //mov           %r10d,%r11d
  73,193,234,32,                          //shr           $0x20,%r10
  243,66,15,16,12,153,                    //movss         (%rcx,%r11,4),%xmm1
  102,66,15,58,33,12,145,16,              //insertps      $0x10,(%rcx,%r10,4),%xmm1
  243,66,15,16,28,137,                    //movss         (%rcx,%r9,4),%xmm3
  102,15,58,33,203,32,                    //insertps      $0x20,%xmm3,%xmm1
  243,66,15,16,28,129,                    //movss         (%rcx,%r8,4),%xmm3
  102,15,58,33,203,48,                    //insertps      $0x30,%xmm3,%xmm1
  72,139,64,24,                           //mov           0x18(%rax),%rax
  102,72,15,58,22,209,1,                  //pextrq        $0x1,%xmm2,%rcx
  65,137,200,                             //mov           %ecx,%r8d
  72,193,233,32,                          //shr           $0x20,%rcx
  102,73,15,126,209,                      //movq          %xmm2,%r9
  69,137,202,                             //mov           %r9d,%r10d
  73,193,233,32,                          //shr           $0x20,%r9
  243,66,15,16,20,144,                    //movss         (%rax,%r10,4),%xmm2
  102,66,15,58,33,20,136,16,              //insertps      $0x10,(%rax,%r9,4),%xmm2
  243,66,15,16,28,128,                    //movss         (%rax,%r8,4),%xmm3
  102,15,58,33,211,32,                    //insertps      $0x20,%xmm3,%xmm2
  243,15,16,28,136,                       //movss         (%rax,%rcx,4),%xmm3
  102,15,58,33,211,48,                    //insertps      $0x30,%xmm3,%xmm2
  102,65,15,114,208,24,                   //psrld         $0x18,%xmm8
  69,15,91,192,                           //cvtdq2ps      %xmm8,%xmm8
  243,15,16,90,12,                        //movss         0xc(%rdx),%xmm3
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_a8_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,15,56,49,4,56,                      //pmovzxbd      (%rax,%rdi,1),%xmm0
  15,91,192,                              //cvtdq2ps      %xmm0,%xmm0
  243,15,16,90,12,                        //movss         0xc(%rdx),%xmm3
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  15,89,216,                              //mulps         %xmm0,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,87,192,                              //xorps         %xmm0,%xmm0
  15,87,201,                              //xorps         %xmm1,%xmm1
  15,87,210,                              //xorps         %xmm2,%xmm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_a8_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,68,15,16,66,8,                      //movss         0x8(%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,89,195,                           //mulps         %xmm3,%xmm8
  102,69,15,91,192,                       //cvtps2dq      %xmm8,%xmm8
  102,69,15,56,43,192,                    //packusdw      %xmm8,%xmm8
  102,69,15,103,192,                      //packuswb      %xmm8,%xmm8
  102,68,15,126,4,56,                     //movd          %xmm8,(%rax,%rdi,1)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_565_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,68,15,56,51,12,120,                 //pmovzxwd      (%rax,%rdi,2),%xmm9
  102,15,110,66,104,                      //movd          0x68(%rdx),%xmm0
  102,15,112,192,0,                       //pshufd        $0x0,%xmm0,%xmm0
  102,65,15,219,193,                      //pand          %xmm9,%xmm0
  15,91,200,                              //cvtdq2ps      %xmm0,%xmm1
  243,15,16,26,                           //movss         (%rdx),%xmm3
  243,15,16,66,116,                       //movss         0x74(%rdx),%xmm0
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  15,89,193,                              //mulps         %xmm1,%xmm0
  102,15,110,74,108,                      //movd          0x6c(%rdx),%xmm1
  102,15,112,201,0,                       //pshufd        $0x0,%xmm1,%xmm1
  102,65,15,219,201,                      //pand          %xmm9,%xmm1
  68,15,91,193,                           //cvtdq2ps      %xmm1,%xmm8
  243,15,16,74,120,                       //movss         0x78(%rdx),%xmm1
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  102,15,110,82,112,                      //movd          0x70(%rdx),%xmm2
  102,15,112,210,0,                       //pshufd        $0x0,%xmm2,%xmm2
  102,65,15,219,209,                      //pand          %xmm9,%xmm2
  68,15,91,194,                           //cvtdq2ps      %xmm2,%xmm8
  243,15,16,82,124,                       //movss         0x7c(%rdx),%xmm2
  15,198,210,0,                           //shufps        $0x0,%xmm2,%xmm2
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_565_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,68,15,16,130,128,0,0,0,             //movss         0x80(%rdx),%xmm8
  243,68,15,16,138,132,0,0,0,             //movss         0x84(%rdx),%xmm9
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  69,15,40,208,                           //movaps        %xmm8,%xmm10
  68,15,89,208,                           //mulps         %xmm0,%xmm10
  102,69,15,91,210,                       //cvtps2dq      %xmm10,%xmm10
  102,65,15,114,242,11,                   //pslld         $0xb,%xmm10
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  68,15,89,201,                           //mulps         %xmm1,%xmm9
  102,69,15,91,201,                       //cvtps2dq      %xmm9,%xmm9
  102,65,15,114,241,5,                    //pslld         $0x5,%xmm9
  102,69,15,235,202,                      //por           %xmm10,%xmm9
  68,15,89,194,                           //mulps         %xmm2,%xmm8
  102,69,15,91,192,                       //cvtps2dq      %xmm8,%xmm8
  102,69,15,86,193,                       //orpd          %xmm9,%xmm8
  102,69,15,56,43,192,                    //packusdw      %xmm8,%xmm8
  102,68,15,214,4,120,                    //movq          %xmm8,(%rax,%rdi,2)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_8888_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,15,111,28,184,                      //movdqu        (%rax,%rdi,4),%xmm3
  102,15,110,66,16,                       //movd          0x10(%rdx),%xmm0
  102,15,112,192,0,                       //pshufd        $0x0,%xmm0,%xmm0
  102,15,111,203,                         //movdqa        %xmm3,%xmm1
  102,15,114,209,8,                       //psrld         $0x8,%xmm1
  102,15,219,200,                         //pand          %xmm0,%xmm1
  102,15,111,211,                         //movdqa        %xmm3,%xmm2
  102,15,114,210,16,                      //psrld         $0x10,%xmm2
  102,15,219,208,                         //pand          %xmm0,%xmm2
  102,15,219,195,                         //pand          %xmm3,%xmm0
  15,91,192,                              //cvtdq2ps      %xmm0,%xmm0
  243,68,15,16,66,12,                     //movss         0xc(%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  15,91,201,                              //cvtdq2ps      %xmm1,%xmm1
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  15,91,210,                              //cvtdq2ps      %xmm2,%xmm2
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  102,15,114,211,24,                      //psrld         $0x18,%xmm3
  15,91,219,                              //cvtdq2ps      %xmm3,%xmm3
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_8888_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,68,15,16,66,8,                      //movss         0x8(%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,200,                           //mulps         %xmm0,%xmm9
  102,69,15,91,201,                       //cvtps2dq      %xmm9,%xmm9
  69,15,40,208,                           //movaps        %xmm8,%xmm10
  68,15,89,209,                           //mulps         %xmm1,%xmm10
  102,69,15,91,210,                       //cvtps2dq      %xmm10,%xmm10
  102,65,15,114,242,8,                    //pslld         $0x8,%xmm10
  102,69,15,235,209,                      //por           %xmm9,%xmm10
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,202,                           //mulps         %xmm2,%xmm9
  102,69,15,91,201,                       //cvtps2dq      %xmm9,%xmm9
  102,65,15,114,241,16,                   //pslld         $0x10,%xmm9
  68,15,89,195,                           //mulps         %xmm3,%xmm8
  102,69,15,91,192,                       //cvtps2dq      %xmm8,%xmm8
  102,65,15,114,240,24,                   //pslld         $0x18,%xmm8
  102,69,15,235,193,                      //por           %xmm9,%xmm8
  102,69,15,235,194,                      //por           %xmm10,%xmm8
  243,68,15,127,4,184,                    //movdqu        %xmm8,(%rax,%rdi,4)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_f16_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,15,111,4,248,                       //movdqu        (%rax,%rdi,8),%xmm0
  243,15,111,76,248,16,                   //movdqu        0x10(%rax,%rdi,8),%xmm1
  102,15,111,208,                         //movdqa        %xmm0,%xmm2
  102,15,97,209,                          //punpcklwd     %xmm1,%xmm2
  102,15,105,193,                         //punpckhwd     %xmm1,%xmm0
  102,68,15,111,194,                      //movdqa        %xmm2,%xmm8
  102,68,15,97,192,                       //punpcklwd     %xmm0,%xmm8
  102,15,105,208,                         //punpckhwd     %xmm0,%xmm2
  102,15,110,66,100,                      //movd          0x64(%rdx),%xmm0
  102,15,112,216,0,                       //pshufd        $0x0,%xmm0,%xmm3
  102,15,111,203,                         //movdqa        %xmm3,%xmm1
  102,65,15,101,200,                      //pcmpgtw       %xmm8,%xmm1
  102,65,15,223,200,                      //pandn         %xmm8,%xmm1
  102,15,101,218,                         //pcmpgtw       %xmm2,%xmm3
  102,15,223,218,                         //pandn         %xmm2,%xmm3
  102,15,56,51,193,                       //pmovzxwd      %xmm1,%xmm0
  102,15,114,240,13,                      //pslld         $0xd,%xmm0
  102,15,110,82,92,                       //movd          0x5c(%rdx),%xmm2
  102,68,15,112,194,0,                    //pshufd        $0x0,%xmm2,%xmm8
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  102,69,15,239,201,                      //pxor          %xmm9,%xmm9
  102,65,15,105,201,                      //punpckhwd     %xmm9,%xmm1
  102,15,114,241,13,                      //pslld         $0xd,%xmm1
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  102,15,56,51,211,                       //pmovzxwd      %xmm3,%xmm2
  102,15,114,242,13,                      //pslld         $0xd,%xmm2
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  102,65,15,105,217,                      //punpckhwd     %xmm9,%xmm3
  102,15,114,243,13,                      //pslld         $0xd,%xmm3
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_f16_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,68,15,110,66,96,                    //movd          0x60(%rdx),%xmm8
  102,69,15,112,192,0,                    //pshufd        $0x0,%xmm8,%xmm8
  102,69,15,111,200,                      //movdqa        %xmm8,%xmm9
  68,15,89,200,                           //mulps         %xmm0,%xmm9
  102,65,15,114,209,13,                   //psrld         $0xd,%xmm9
  102,69,15,111,208,                      //movdqa        %xmm8,%xmm10
  68,15,89,209,                           //mulps         %xmm1,%xmm10
  102,65,15,114,210,13,                   //psrld         $0xd,%xmm10
  102,69,15,111,216,                      //movdqa        %xmm8,%xmm11
  68,15,89,218,                           //mulps         %xmm2,%xmm11
  102,65,15,114,211,13,                   //psrld         $0xd,%xmm11
  68,15,89,195,                           //mulps         %xmm3,%xmm8
  102,65,15,114,208,13,                   //psrld         $0xd,%xmm8
  102,65,15,115,250,2,                    //pslldq        $0x2,%xmm10
  102,69,15,235,209,                      //por           %xmm9,%xmm10
  102,65,15,115,248,2,                    //pslldq        $0x2,%xmm8
  102,69,15,235,195,                      //por           %xmm11,%xmm8
  102,69,15,111,202,                      //movdqa        %xmm10,%xmm9
  102,69,15,98,200,                       //punpckldq     %xmm8,%xmm9
  243,68,15,127,12,248,                   //movdqu        %xmm9,(%rax,%rdi,8)
  102,69,15,106,208,                      //punpckhdq     %xmm8,%xmm10
  243,68,15,127,84,248,16,                //movdqu        %xmm10,0x10(%rax,%rdi,8)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_f32_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,137,249,                             //mov           %rdi,%rcx
  72,193,225,4,                           //shl           $0x4,%rcx
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  68,15,40,200,                           //movaps        %xmm0,%xmm9
  68,15,20,201,                           //unpcklps      %xmm1,%xmm9
  68,15,40,210,                           //movaps        %xmm2,%xmm10
  68,15,40,218,                           //movaps        %xmm2,%xmm11
  68,15,20,219,                           //unpcklps      %xmm3,%xmm11
  68,15,21,193,                           //unpckhps      %xmm1,%xmm8
  68,15,21,211,                           //unpckhps      %xmm3,%xmm10
  69,15,40,225,                           //movaps        %xmm9,%xmm12
  102,69,15,20,227,                       //unpcklpd      %xmm11,%xmm12
  102,69,15,21,203,                       //unpckhpd      %xmm11,%xmm9
  69,15,40,216,                           //movaps        %xmm8,%xmm11
  102,69,15,20,218,                       //unpcklpd      %xmm10,%xmm11
  102,69,15,21,194,                       //unpckhpd      %xmm10,%xmm8
  102,68,15,17,36,8,                      //movupd        %xmm12,(%rax,%rcx,1)
  102,68,15,17,76,8,16,                   //movupd        %xmm9,0x10(%rax,%rcx,1)
  102,68,15,17,92,8,32,                   //movupd        %xmm11,0x20(%rax,%rcx,1)
  102,68,15,17,68,8,48,                   //movupd        %xmm8,0x30(%rax,%rcx,1)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_x_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  68,15,95,192,                           //maxps         %xmm0,%xmm8
  243,68,15,16,8,                         //movss         (%rax),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  102,15,118,192,                         //pcmpeqd       %xmm0,%xmm0
  102,65,15,254,193,                      //paddd         %xmm9,%xmm0
  68,15,93,192,                           //minps         %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,192,                           //movaps        %xmm8,%xmm0
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_y_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  68,15,95,193,                           //maxps         %xmm1,%xmm8
  243,68,15,16,8,                         //movss         (%rax),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  102,15,118,201,                         //pcmpeqd       %xmm1,%xmm1
  102,65,15,254,201,                      //paddd         %xmm9,%xmm1
  68,15,93,193,                           //minps         %xmm1,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,200,                           //movaps        %xmm8,%xmm1
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_repeat_x_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,40,200,                           //movaps        %xmm0,%xmm9
  69,15,94,200,                           //divps         %xmm8,%xmm9
  102,69,15,58,8,201,1,                   //roundps       $0x1,%xmm9,%xmm9
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  65,15,92,193,                           //subps         %xmm9,%xmm0
  102,69,15,118,201,                      //pcmpeqd       %xmm9,%xmm9
  102,69,15,254,200,                      //paddd         %xmm8,%xmm9
  65,15,93,193,                           //minps         %xmm9,%xmm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_repeat_y_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,40,201,                           //movaps        %xmm1,%xmm9
  69,15,94,200,                           //divps         %xmm8,%xmm9
  102,69,15,58,8,201,1,                   //roundps       $0x1,%xmm9,%xmm9
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  65,15,92,201,                           //subps         %xmm9,%xmm1
  102,69,15,118,201,                      //pcmpeqd       %xmm9,%xmm9
  102,69,15,254,200,                      //paddd         %xmm8,%xmm9
  65,15,93,201,                           //minps         %xmm9,%xmm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_mirror_x_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  65,15,92,193,                           //subps         %xmm9,%xmm0
  243,69,15,88,192,                       //addss         %xmm8,%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,40,208,                           //movaps        %xmm0,%xmm10
  69,15,94,208,                           //divps         %xmm8,%xmm10
  102,69,15,58,8,210,1,                   //roundps       $0x1,%xmm10,%xmm10
  69,15,89,208,                           //mulps         %xmm8,%xmm10
  65,15,92,194,                           //subps         %xmm10,%xmm0
  65,15,92,193,                           //subps         %xmm9,%xmm0
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  68,15,92,192,                           //subps         %xmm0,%xmm8
  65,15,84,192,                           //andps         %xmm8,%xmm0
  102,69,15,118,192,                      //pcmpeqd       %xmm8,%xmm8
  102,69,15,254,193,                      //paddd         %xmm9,%xmm8
  65,15,93,192,                           //minps         %xmm8,%xmm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_mirror_y_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  65,15,92,201,                           //subps         %xmm9,%xmm1
  243,69,15,88,192,                       //addss         %xmm8,%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,40,209,                           //movaps        %xmm1,%xmm10
  69,15,94,208,                           //divps         %xmm8,%xmm10
  102,69,15,58,8,210,1,                   //roundps       $0x1,%xmm10,%xmm10
  69,15,89,208,                           //mulps         %xmm8,%xmm10
  65,15,92,202,                           //subps         %xmm10,%xmm1
  65,15,92,201,                           //subps         %xmm9,%xmm1
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  68,15,92,193,                           //subps         %xmm1,%xmm8
  65,15,84,200,                           //andps         %xmm8,%xmm1
  102,69,15,118,192,                      //pcmpeqd       %xmm8,%xmm8
  102,69,15,254,193,                      //paddd         %xmm9,%xmm8
  65,15,93,200,                           //minps         %xmm8,%xmm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_2x3_sse41[] = {
  68,15,40,201,                           //movaps        %xmm1,%xmm9
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,15,16,0,                            //movss         (%rax),%xmm0
  243,15,16,72,4,                         //movss         0x4(%rax),%xmm1
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  243,68,15,16,80,8,                      //movss         0x8(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,16,                     //movss         0x10(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,88,194,                           //addps         %xmm10,%xmm0
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  243,68,15,16,80,12,                     //movss         0xc(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,20,                     //movss         0x14(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  65,15,88,202,                           //addps         %xmm10,%xmm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_3x4_sse41[] = {
  68,15,40,201,                           //movaps        %xmm1,%xmm9
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,15,16,0,                            //movss         (%rax),%xmm0
  243,15,16,72,4,                         //movss         0x4(%rax),%xmm1
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  243,68,15,16,80,12,                     //movss         0xc(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,24,                     //movss         0x18(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,96,36,                     //movss         0x24(%rax),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  68,15,89,218,                           //mulps         %xmm2,%xmm11
  69,15,88,220,                           //addps         %xmm12,%xmm11
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,88,194,                           //addps         %xmm10,%xmm0
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  243,68,15,16,80,16,                     //movss         0x10(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,28,                     //movss         0x1c(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,96,40,                     //movss         0x28(%rax),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  68,15,89,218,                           //mulps         %xmm2,%xmm11
  69,15,88,220,                           //addps         %xmm12,%xmm11
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  65,15,88,202,                           //addps         %xmm10,%xmm1
  243,68,15,16,80,8,                      //movss         0x8(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,20,                     //movss         0x14(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,96,32,                     //movss         0x20(%rax),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  243,68,15,16,104,44,                    //movss         0x2c(%rax),%xmm13
  69,15,198,237,0,                        //shufps        $0x0,%xmm13,%xmm13
  68,15,89,226,                           //mulps         %xmm2,%xmm12
  69,15,88,229,                           //addps         %xmm13,%xmm12
  69,15,89,217,                           //mulps         %xmm9,%xmm11
  69,15,88,220,                           //addps         %xmm12,%xmm11
  69,15,89,208,                           //mulps         %xmm8,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,210,                           //movaps        %xmm10,%xmm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_perspective_sse41[] = {
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,15,16,0,                            //movss         (%rax),%xmm0
  243,68,15,16,72,4,                      //movss         0x4(%rax),%xmm9
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  243,68,15,16,80,8,                      //movss         0x8(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  68,15,89,201,                           //mulps         %xmm1,%xmm9
  69,15,88,202,                           //addps         %xmm10,%xmm9
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,88,193,                           //addps         %xmm9,%xmm0
  243,68,15,16,72,12,                     //movss         0xc(%rax),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  243,68,15,16,80,16,                     //movss         0x10(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,20,                     //movss         0x14(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  68,15,89,209,                           //mulps         %xmm1,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  69,15,88,202,                           //addps         %xmm10,%xmm9
  243,68,15,16,80,24,                     //movss         0x18(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,28,                     //movss         0x1c(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,96,32,                     //movss         0x20(%rax),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  68,15,89,217,                           //mulps         %xmm1,%xmm11
  69,15,88,220,                           //addps         %xmm12,%xmm11
  69,15,89,208,                           //mulps         %xmm8,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,83,202,                           //rcpps         %xmm10,%xmm1
  15,89,193,                              //mulps         %xmm1,%xmm0
  68,15,89,201,                           //mulps         %xmm1,%xmm9
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,201,                           //movaps        %xmm9,%xmm1
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_linear_gradient_2stops_sse41[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  68,15,16,8,                             //movups        (%rax),%xmm9
  15,16,88,16,                            //movups        0x10(%rax),%xmm3
  68,15,40,195,                           //movaps        %xmm3,%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,40,201,                           //movaps        %xmm9,%xmm1
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  68,15,89,192,                           //mulps         %xmm0,%xmm8
  68,15,88,193,                           //addps         %xmm1,%xmm8
  15,40,203,                              //movaps        %xmm3,%xmm1
  15,198,201,85,                          //shufps        $0x55,%xmm1,%xmm1
  65,15,40,209,                           //movaps        %xmm9,%xmm2
  15,198,210,85,                          //shufps        $0x55,%xmm2,%xmm2
  15,89,200,                              //mulps         %xmm0,%xmm1
  15,88,202,                              //addps         %xmm2,%xmm1
  15,40,211,                              //movaps        %xmm3,%xmm2
  15,198,210,170,                         //shufps        $0xaa,%xmm2,%xmm2
  69,15,40,209,                           //movaps        %xmm9,%xmm10
  69,15,198,210,170,                      //shufps        $0xaa,%xmm10,%xmm10
  15,89,208,                              //mulps         %xmm0,%xmm2
  65,15,88,210,                           //addps         %xmm10,%xmm2
  15,198,219,255,                         //shufps        $0xff,%xmm3,%xmm3
  69,15,198,201,255,                      //shufps        $0xff,%xmm9,%xmm9
  15,89,216,                              //mulps         %xmm0,%xmm3
  65,15,88,217,                           //addps         %xmm9,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,192,                           //movaps        %xmm8,%xmm0
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_start_pipeline_sse2[] = {
  65,87,                                  //push          %r15
  65,86,                                  //push          %r14
  65,85,                                  //push          %r13
  65,84,                                  //push          %r12
  86,                                     //push          %rsi
  87,                                     //push          %rdi
  83,                                     //push          %rbx
  72,129,236,160,0,0,0,                   //sub           $0xa0,%rsp
  68,15,41,188,36,144,0,0,0,              //movaps        %xmm15,0x90(%rsp)
  68,15,41,180,36,128,0,0,0,              //movaps        %xmm14,0x80(%rsp)
  68,15,41,108,36,112,                    //movaps        %xmm13,0x70(%rsp)
  68,15,41,100,36,96,                     //movaps        %xmm12,0x60(%rsp)
  68,15,41,92,36,80,                      //movaps        %xmm11,0x50(%rsp)
  68,15,41,84,36,64,                      //movaps        %xmm10,0x40(%rsp)
  68,15,41,76,36,48,                      //movaps        %xmm9,0x30(%rsp)
  68,15,41,68,36,32,                      //movaps        %xmm8,0x20(%rsp)
  15,41,124,36,16,                        //movaps        %xmm7,0x10(%rsp)
  15,41,52,36,                            //movaps        %xmm6,(%rsp)
  77,137,207,                             //mov           %r9,%r15
  77,137,198,                             //mov           %r8,%r14
  72,137,203,                             //mov           %rcx,%rbx
  72,137,214,                             //mov           %rdx,%rsi
  72,173,                                 //lods          %ds:(%rsi),%rax
  73,137,196,                             //mov           %rax,%r12
  73,137,245,                             //mov           %rsi,%r13
  72,141,67,4,                            //lea           0x4(%rbx),%rax
  76,57,248,                              //cmp           %r15,%rax
  118,5,                                  //jbe           73 <_sk_start_pipeline_sse2+0x73>
  72,137,216,                             //mov           %rbx,%rax
  235,52,                                 //jmp           a7 <_sk_start_pipeline_sse2+0xa7>
  15,87,192,                              //xorps         %xmm0,%xmm0
  15,87,201,                              //xorps         %xmm1,%xmm1
  15,87,210,                              //xorps         %xmm2,%xmm2
  15,87,219,                              //xorps         %xmm3,%xmm3
  15,87,228,                              //xorps         %xmm4,%xmm4
  15,87,237,                              //xorps         %xmm5,%xmm5
  15,87,246,                              //xorps         %xmm6,%xmm6
  15,87,255,                              //xorps         %xmm7,%xmm7
  72,137,223,                             //mov           %rbx,%rdi
  76,137,238,                             //mov           %r13,%rsi
  76,137,242,                             //mov           %r14,%rdx
  65,255,212,                             //callq         *%r12
  72,141,67,4,                            //lea           0x4(%rbx),%rax
  72,131,195,8,                           //add           $0x8,%rbx
  76,57,251,                              //cmp           %r15,%rbx
  72,137,195,                             //mov           %rax,%rbx
  118,204,                                //jbe           73 <_sk_start_pipeline_sse2+0x73>
  15,40,52,36,                            //movaps        (%rsp),%xmm6
  15,40,124,36,16,                        //movaps        0x10(%rsp),%xmm7
  68,15,40,68,36,32,                      //movaps        0x20(%rsp),%xmm8
  68,15,40,76,36,48,                      //movaps        0x30(%rsp),%xmm9
  68,15,40,84,36,64,                      //movaps        0x40(%rsp),%xmm10
  68,15,40,92,36,80,                      //movaps        0x50(%rsp),%xmm11
  68,15,40,100,36,96,                     //movaps        0x60(%rsp),%xmm12
  68,15,40,108,36,112,                    //movaps        0x70(%rsp),%xmm13
  68,15,40,180,36,128,0,0,0,              //movaps        0x80(%rsp),%xmm14
  68,15,40,188,36,144,0,0,0,              //movaps        0x90(%rsp),%xmm15
  72,129,196,160,0,0,0,                   //add           $0xa0,%rsp
  91,                                     //pop           %rbx
  95,                                     //pop           %rdi
  94,                                     //pop           %rsi
  65,92,                                  //pop           %r12
  65,93,                                  //pop           %r13
  65,94,                                  //pop           %r14
  65,95,                                  //pop           %r15
  195,                                    //retq
};

CODE const uint8_t sk_just_return_sse2[] = {
  195,                                    //retq
};

CODE const uint8_t sk_seed_shader_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  102,15,110,199,                         //movd          %edi,%xmm0
  102,15,112,192,0,                       //pshufd        $0x0,%xmm0,%xmm0
  15,91,200,                              //cvtdq2ps      %xmm0,%xmm1
  243,15,16,18,                           //movss         (%rdx),%xmm2
  243,15,16,90,4,                         //movss         0x4(%rdx),%xmm3
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  15,88,203,                              //addps         %xmm3,%xmm1
  15,16,66,20,                            //movups        0x14(%rdx),%xmm0
  15,88,193,                              //addps         %xmm1,%xmm0
  102,15,110,8,                           //movd          (%rax),%xmm1
  102,15,112,201,0,                       //pshufd        $0x0,%xmm1,%xmm1
  15,91,201,                              //cvtdq2ps      %xmm1,%xmm1
  15,88,203,                              //addps         %xmm3,%xmm1
  15,198,210,0,                           //shufps        $0x0,%xmm2,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,87,219,                              //xorps         %xmm3,%xmm3
  15,87,228,                              //xorps         %xmm4,%xmm4
  15,87,237,                              //xorps         %xmm5,%xmm5
  15,87,246,                              //xorps         %xmm6,%xmm6
  15,87,255,                              //xorps         %xmm7,%xmm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_constant_color_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,16,24,                               //movups        (%rax),%xmm3
  15,40,195,                              //movaps        %xmm3,%xmm0
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  15,40,203,                              //movaps        %xmm3,%xmm1
  15,198,201,85,                          //shufps        $0x55,%xmm1,%xmm1
  15,40,211,                              //movaps        %xmm3,%xmm2
  15,198,210,170,                         //shufps        $0xaa,%xmm2,%xmm2
  15,198,219,255,                         //shufps        $0xff,%xmm3,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clear_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,87,192,                              //xorps         %xmm0,%xmm0
  15,87,201,                              //xorps         %xmm1,%xmm1
  15,87,210,                              //xorps         %xmm2,%xmm2
  15,87,219,                              //xorps         %xmm3,%xmm3
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_plus__sse2[] = {
  15,88,196,                              //addps         %xmm4,%xmm0
  15,88,205,                              //addps         %xmm5,%xmm1
  15,88,214,                              //addps         %xmm6,%xmm2
  15,88,223,                              //addps         %xmm7,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_srcover_sse2[] = {
  243,68,15,16,2,                         //movss         (%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,92,195,                           //subps         %xmm3,%xmm8
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,204,                           //mulps         %xmm4,%xmm9
  65,15,88,193,                           //addps         %xmm9,%xmm0
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,205,                           //mulps         %xmm5,%xmm9
  65,15,88,201,                           //addps         %xmm9,%xmm1
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,206,                           //mulps         %xmm6,%xmm9
  65,15,88,209,                           //addps         %xmm9,%xmm2
  68,15,89,199,                           //mulps         %xmm7,%xmm8
  65,15,88,216,                           //addps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_dstover_sse2[] = {
  243,68,15,16,2,                         //movss         (%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,92,199,                           //subps         %xmm7,%xmm8
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  15,88,196,                              //addps         %xmm4,%xmm0
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  15,88,205,                              //addps         %xmm5,%xmm1
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  15,88,214,                              //addps         %xmm6,%xmm2
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  15,88,223,                              //addps         %xmm7,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_0_sse2[] = {
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  65,15,95,192,                           //maxps         %xmm8,%xmm0
  65,15,95,200,                           //maxps         %xmm8,%xmm1
  65,15,95,208,                           //maxps         %xmm8,%xmm2
  65,15,95,216,                           //maxps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_1_sse2[] = {
  243,68,15,16,2,                         //movss         (%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,93,192,                           //minps         %xmm8,%xmm0
  65,15,93,200,                           //minps         %xmm8,%xmm1
  65,15,93,208,                           //minps         %xmm8,%xmm2
  65,15,93,216,                           //minps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_a_sse2[] = {
  243,68,15,16,2,                         //movss         (%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,93,216,                           //minps         %xmm8,%xmm3
  15,93,195,                              //minps         %xmm3,%xmm0
  15,93,203,                              //minps         %xmm3,%xmm1
  15,93,211,                              //minps         %xmm3,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_set_rgb_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,15,16,0,                            //movss         (%rax),%xmm0
  243,15,16,72,4,                         //movss         0x4(%rax),%xmm1
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  243,15,16,80,8,                         //movss         0x8(%rax),%xmm2
  15,198,210,0,                           //shufps        $0x0,%xmm2,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_swap_rb_sse2[] = {
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,194,                              //movaps        %xmm2,%xmm0
  65,15,40,208,                           //movaps        %xmm8,%xmm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_swap_sse2[] = {
  68,15,40,195,                           //movaps        %xmm3,%xmm8
  68,15,40,202,                           //movaps        %xmm2,%xmm9
  68,15,40,209,                           //movaps        %xmm1,%xmm10
  68,15,40,216,                           //movaps        %xmm0,%xmm11
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,196,                              //movaps        %xmm4,%xmm0
  15,40,205,                              //movaps        %xmm5,%xmm1
  15,40,214,                              //movaps        %xmm6,%xmm2
  15,40,223,                              //movaps        %xmm7,%xmm3
  65,15,40,227,                           //movaps        %xmm11,%xmm4
  65,15,40,234,                           //movaps        %xmm10,%xmm5
  65,15,40,241,                           //movaps        %xmm9,%xmm6
  65,15,40,248,                           //movaps        %xmm8,%xmm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_move_src_dst_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,224,                              //movaps        %xmm0,%xmm4
  15,40,233,                              //movaps        %xmm1,%xmm5
  15,40,242,                              //movaps        %xmm2,%xmm6
  15,40,251,                              //movaps        %xmm3,%xmm7
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_move_dst_src_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,196,                              //movaps        %xmm4,%xmm0
  15,40,205,                              //movaps        %xmm5,%xmm1
  15,40,214,                              //movaps        %xmm6,%xmm2
  15,40,223,                              //movaps        %xmm7,%xmm3
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_premul_sse2[] = {
  15,89,195,                              //mulps         %xmm3,%xmm0
  15,89,203,                              //mulps         %xmm3,%xmm1
  15,89,211,                              //mulps         %xmm3,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_unpremul_sse2[] = {
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  68,15,194,195,0,                        //cmpeqps       %xmm3,%xmm8
  243,68,15,16,10,                        //movss         (%rdx),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  68,15,94,203,                           //divps         %xmm3,%xmm9
  69,15,85,193,                           //andnps        %xmm9,%xmm8
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_from_srgb_sse2[] = {
  243,68,15,16,66,64,                     //movss         0x40(%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  69,15,40,232,                           //movaps        %xmm8,%xmm13
  68,15,89,232,                           //mulps         %xmm0,%xmm13
  68,15,40,224,                           //movaps        %xmm0,%xmm12
  69,15,89,228,                           //mulps         %xmm12,%xmm12
  243,68,15,16,74,60,                     //movss         0x3c(%rdx),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  243,68,15,16,82,52,                     //movss         0x34(%rdx),%xmm10
  243,68,15,16,90,56,                     //movss         0x38(%rdx),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,40,241,                           //movaps        %xmm9,%xmm14
  68,15,89,240,                           //mulps         %xmm0,%xmm14
  69,15,88,243,                           //addps         %xmm11,%xmm14
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  69,15,89,244,                           //mulps         %xmm12,%xmm14
  69,15,88,242,                           //addps         %xmm10,%xmm14
  243,68,15,16,98,68,                     //movss         0x44(%rdx),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  65,15,194,196,1,                        //cmpltps       %xmm12,%xmm0
  68,15,84,232,                           //andps         %xmm0,%xmm13
  65,15,85,198,                           //andnps        %xmm14,%xmm0
  65,15,86,197,                           //orps          %xmm13,%xmm0
  69,15,40,232,                           //movaps        %xmm8,%xmm13
  68,15,89,233,                           //mulps         %xmm1,%xmm13
  68,15,40,241,                           //movaps        %xmm1,%xmm14
  69,15,89,246,                           //mulps         %xmm14,%xmm14
  69,15,40,249,                           //movaps        %xmm9,%xmm15
  68,15,89,249,                           //mulps         %xmm1,%xmm15
  69,15,88,251,                           //addps         %xmm11,%xmm15
  69,15,89,254,                           //mulps         %xmm14,%xmm15
  69,15,88,250,                           //addps         %xmm10,%xmm15
  65,15,194,204,1,                        //cmpltps       %xmm12,%xmm1
  68,15,84,233,                           //andps         %xmm1,%xmm13
  65,15,85,207,                           //andnps        %xmm15,%xmm1
  65,15,86,205,                           //orps          %xmm13,%xmm1
  68,15,89,194,                           //mulps         %xmm2,%xmm8
  68,15,40,234,                           //movaps        %xmm2,%xmm13
  69,15,89,237,                           //mulps         %xmm13,%xmm13
  68,15,89,202,                           //mulps         %xmm2,%xmm9
  69,15,88,203,                           //addps         %xmm11,%xmm9
  69,15,89,205,                           //mulps         %xmm13,%xmm9
  69,15,88,202,                           //addps         %xmm10,%xmm9
  65,15,194,212,1,                        //cmpltps       %xmm12,%xmm2
  68,15,84,194,                           //andps         %xmm2,%xmm8
  65,15,85,209,                           //andnps        %xmm9,%xmm2
  65,15,86,208,                           //orps          %xmm8,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_to_srgb_sse2[] = {
  72,131,236,40,                          //sub           $0x28,%rsp
  15,41,124,36,16,                        //movaps        %xmm7,0x10(%rsp)
  15,41,52,36,                            //movaps        %xmm6,(%rsp)
  15,40,245,                              //movaps        %xmm5,%xmm6
  15,40,236,                              //movaps        %xmm4,%xmm5
  15,40,227,                              //movaps        %xmm3,%xmm4
  68,15,82,192,                           //rsqrtps       %xmm0,%xmm8
  69,15,83,232,                           //rcpps         %xmm8,%xmm13
  69,15,82,248,                           //rsqrtps       %xmm8,%xmm15
  243,15,16,26,                           //movss         (%rdx),%xmm3
  243,68,15,16,66,72,                     //movss         0x48(%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  69,15,40,240,                           //movaps        %xmm8,%xmm14
  68,15,89,240,                           //mulps         %xmm0,%xmm14
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  243,68,15,16,82,76,                     //movss         0x4c(%rdx),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,90,80,                     //movss         0x50(%rdx),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,98,84,                     //movss         0x54(%rdx),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  69,15,89,235,                           //mulps         %xmm11,%xmm13
  69,15,88,236,                           //addps         %xmm12,%xmm13
  69,15,89,250,                           //mulps         %xmm10,%xmm15
  69,15,88,253,                           //addps         %xmm13,%xmm15
  68,15,40,203,                           //movaps        %xmm3,%xmm9
  69,15,93,207,                           //minps         %xmm15,%xmm9
  243,68,15,16,106,88,                    //movss         0x58(%rdx),%xmm13
  69,15,198,237,0,                        //shufps        $0x0,%xmm13,%xmm13
  65,15,194,197,1,                        //cmpltps       %xmm13,%xmm0
  68,15,84,240,                           //andps         %xmm0,%xmm14
  65,15,85,193,                           //andnps        %xmm9,%xmm0
  65,15,86,198,                           //orps          %xmm14,%xmm0
  68,15,82,201,                           //rsqrtps       %xmm1,%xmm9
  69,15,83,241,                           //rcpps         %xmm9,%xmm14
  69,15,82,201,                           //rsqrtps       %xmm9,%xmm9
  69,15,89,243,                           //mulps         %xmm11,%xmm14
  69,15,88,244,                           //addps         %xmm12,%xmm14
  69,15,89,202,                           //mulps         %xmm10,%xmm9
  69,15,88,206,                           //addps         %xmm14,%xmm9
  68,15,40,243,                           //movaps        %xmm3,%xmm14
  69,15,93,241,                           //minps         %xmm9,%xmm14
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,201,                           //mulps         %xmm1,%xmm9
  65,15,194,205,1,                        //cmpltps       %xmm13,%xmm1
  68,15,84,201,                           //andps         %xmm1,%xmm9
  65,15,85,206,                           //andnps        %xmm14,%xmm1
  65,15,86,201,                           //orps          %xmm9,%xmm1
  68,15,82,202,                           //rsqrtps       %xmm2,%xmm9
  69,15,83,241,                           //rcpps         %xmm9,%xmm14
  69,15,89,243,                           //mulps         %xmm11,%xmm14
  69,15,88,244,                           //addps         %xmm12,%xmm14
  65,15,82,249,                           //rsqrtps       %xmm9,%xmm7
  65,15,89,250,                           //mulps         %xmm10,%xmm7
  65,15,88,254,                           //addps         %xmm14,%xmm7
  15,93,223,                              //minps         %xmm7,%xmm3
  68,15,89,194,                           //mulps         %xmm2,%xmm8
  65,15,194,213,1,                        //cmpltps       %xmm13,%xmm2
  68,15,84,194,                           //andps         %xmm2,%xmm8
  15,85,211,                              //andnps        %xmm3,%xmm2
  65,15,86,208,                           //orps          %xmm8,%xmm2
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,40,220,                              //movaps        %xmm4,%xmm3
  15,40,229,                              //movaps        %xmm5,%xmm4
  15,40,238,                              //movaps        %xmm6,%xmm5
  15,40,52,36,                            //movaps        (%rsp),%xmm6
  15,40,124,36,16,                        //movaps        0x10(%rsp),%xmm7
  72,131,196,40,                          //add           $0x28,%rsp
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_scale_1_float_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_scale_u8_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,68,15,110,4,56,                     //movd          (%rax,%rdi,1),%xmm8
  102,69,15,239,201,                      //pxor          %xmm9,%xmm9
  102,69,15,96,193,                       //punpcklbw     %xmm9,%xmm8
  102,69,15,97,193,                       //punpcklwd     %xmm9,%xmm8
  69,15,91,192,                           //cvtdq2ps      %xmm8,%xmm8
  243,68,15,16,74,12,                     //movss         0xc(%rdx),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  65,15,89,193,                           //mulps         %xmm9,%xmm0
  65,15,89,201,                           //mulps         %xmm9,%xmm1
  65,15,89,209,                           //mulps         %xmm9,%xmm2
  65,15,89,217,                           //mulps         %xmm9,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_lerp_1_float_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  15,92,196,                              //subps         %xmm4,%xmm0
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  15,88,196,                              //addps         %xmm4,%xmm0
  15,92,205,                              //subps         %xmm5,%xmm1
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  15,88,205,                              //addps         %xmm5,%xmm1
  15,92,214,                              //subps         %xmm6,%xmm2
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  15,88,214,                              //addps         %xmm6,%xmm2
  15,92,223,                              //subps         %xmm7,%xmm3
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  15,88,223,                              //addps         %xmm7,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_lerp_u8_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,68,15,110,4,56,                     //movd          (%rax,%rdi,1),%xmm8
  102,69,15,239,201,                      //pxor          %xmm9,%xmm9
  102,69,15,96,193,                       //punpcklbw     %xmm9,%xmm8
  102,69,15,97,193,                       //punpcklwd     %xmm9,%xmm8
  69,15,91,192,                           //cvtdq2ps      %xmm8,%xmm8
  243,68,15,16,74,12,                     //movss         0xc(%rdx),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  15,92,196,                              //subps         %xmm4,%xmm0
  65,15,89,193,                           //mulps         %xmm9,%xmm0
  15,88,196,                              //addps         %xmm4,%xmm0
  15,92,205,                              //subps         %xmm5,%xmm1
  65,15,89,201,                           //mulps         %xmm9,%xmm1
  15,88,205,                              //addps         %xmm5,%xmm1
  15,92,214,                              //subps         %xmm6,%xmm2
  65,15,89,209,                           //mulps         %xmm9,%xmm2
  15,88,214,                              //addps         %xmm6,%xmm2
  15,92,223,                              //subps         %xmm7,%xmm3
  65,15,89,217,                           //mulps         %xmm9,%xmm3
  15,88,223,                              //addps         %xmm7,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_lerp_565_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,68,15,126,4,120,                    //movq          (%rax,%rdi,2),%xmm8
  102,15,239,219,                         //pxor          %xmm3,%xmm3
  102,68,15,97,195,                       //punpcklwd     %xmm3,%xmm8
  102,15,110,90,104,                      //movd          0x68(%rdx),%xmm3
  102,15,112,219,0,                       //pshufd        $0x0,%xmm3,%xmm3
  102,65,15,219,216,                      //pand          %xmm8,%xmm3
  68,15,91,203,                           //cvtdq2ps      %xmm3,%xmm9
  243,15,16,26,                           //movss         (%rdx),%xmm3
  243,68,15,16,82,116,                    //movss         0x74(%rdx),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  102,68,15,110,74,108,                   //movd          0x6c(%rdx),%xmm9
  102,69,15,112,201,0,                    //pshufd        $0x0,%xmm9,%xmm9
  102,69,15,219,200,                      //pand          %xmm8,%xmm9
  69,15,91,201,                           //cvtdq2ps      %xmm9,%xmm9
  243,68,15,16,90,120,                    //movss         0x78(%rdx),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,89,217,                           //mulps         %xmm9,%xmm11
  102,68,15,110,74,112,                   //movd          0x70(%rdx),%xmm9
  102,69,15,112,201,0,                    //pshufd        $0x0,%xmm9,%xmm9
  102,69,15,219,200,                      //pand          %xmm8,%xmm9
  69,15,91,193,                           //cvtdq2ps      %xmm9,%xmm8
  243,68,15,16,74,124,                    //movss         0x7c(%rdx),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  15,92,196,                              //subps         %xmm4,%xmm0
  65,15,89,194,                           //mulps         %xmm10,%xmm0
  15,88,196,                              //addps         %xmm4,%xmm0
  15,92,205,                              //subps         %xmm5,%xmm1
  65,15,89,203,                           //mulps         %xmm11,%xmm1
  15,88,205,                              //addps         %xmm5,%xmm1
  15,92,214,                              //subps         %xmm6,%xmm2
  65,15,89,209,                           //mulps         %xmm9,%xmm2
  15,88,214,                              //addps         %xmm6,%xmm2
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_tables_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,8,                               //mov           (%rax),%rcx
  76,139,64,8,                            //mov           0x8(%rax),%r8
  243,68,15,111,4,185,                    //movdqu        (%rcx,%rdi,4),%xmm8
  102,15,110,66,16,                       //movd          0x10(%rdx),%xmm0
  102,15,112,192,0,                       //pshufd        $0x0,%xmm0,%xmm0
  102,69,15,111,200,                      //movdqa        %xmm8,%xmm9
  102,65,15,114,209,8,                    //psrld         $0x8,%xmm9
  102,68,15,219,200,                      //pand          %xmm0,%xmm9
  102,69,15,111,208,                      //movdqa        %xmm8,%xmm10
  102,65,15,114,210,16,                   //psrld         $0x10,%xmm10
  102,68,15,219,208,                      //pand          %xmm0,%xmm10
  102,65,15,219,192,                      //pand          %xmm8,%xmm0
  102,15,112,216,78,                      //pshufd        $0x4e,%xmm0,%xmm3
  102,72,15,126,217,                      //movq          %xmm3,%rcx
  65,137,201,                             //mov           %ecx,%r9d
  72,193,233,32,                          //shr           $0x20,%rcx
  102,73,15,126,194,                      //movq          %xmm0,%r10
  69,137,211,                             //mov           %r10d,%r11d
  73,193,234,32,                          //shr           $0x20,%r10
  243,67,15,16,28,144,                    //movss         (%r8,%r10,4),%xmm3
  243,65,15,16,4,136,                     //movss         (%r8,%rcx,4),%xmm0
  15,20,216,                              //unpcklps      %xmm0,%xmm3
  243,67,15,16,4,152,                     //movss         (%r8,%r11,4),%xmm0
  243,67,15,16,12,136,                    //movss         (%r8,%r9,4),%xmm1
  15,20,193,                              //unpcklps      %xmm1,%xmm0
  15,20,195,                              //unpcklps      %xmm3,%xmm0
  72,139,72,16,                           //mov           0x10(%rax),%rcx
  102,65,15,112,201,78,                   //pshufd        $0x4e,%xmm9,%xmm1
  102,73,15,126,200,                      //movq          %xmm1,%r8
  69,137,193,                             //mov           %r8d,%r9d
  73,193,232,32,                          //shr           $0x20,%r8
  102,77,15,126,202,                      //movq          %xmm9,%r10
  69,137,211,                             //mov           %r10d,%r11d
  73,193,234,32,                          //shr           $0x20,%r10
  243,66,15,16,28,145,                    //movss         (%rcx,%r10,4),%xmm3
  243,66,15,16,12,129,                    //movss         (%rcx,%r8,4),%xmm1
  15,20,217,                              //unpcklps      %xmm1,%xmm3
  243,66,15,16,12,153,                    //movss         (%rcx,%r11,4),%xmm1
  243,66,15,16,20,137,                    //movss         (%rcx,%r9,4),%xmm2
  15,20,202,                              //unpcklps      %xmm2,%xmm1
  15,20,203,                              //unpcklps      %xmm3,%xmm1
  72,139,64,24,                           //mov           0x18(%rax),%rax
  102,65,15,112,210,78,                   //pshufd        $0x4e,%xmm10,%xmm2
  102,72,15,126,209,                      //movq          %xmm2,%rcx
  65,137,200,                             //mov           %ecx,%r8d
  72,193,233,32,                          //shr           $0x20,%rcx
  102,77,15,126,209,                      //movq          %xmm10,%r9
  69,137,202,                             //mov           %r9d,%r10d
  73,193,233,32,                          //shr           $0x20,%r9
  243,70,15,16,12,136,                    //movss         (%rax,%r9,4),%xmm9
  243,15,16,20,136,                       //movss         (%rax,%rcx,4),%xmm2
  68,15,20,202,                           //unpcklps      %xmm2,%xmm9
  243,66,15,16,20,144,                    //movss         (%rax,%r10,4),%xmm2
  243,66,15,16,28,128,                    //movss         (%rax,%r8,4),%xmm3
  15,20,211,                              //unpcklps      %xmm3,%xmm2
  65,15,20,209,                           //unpcklps      %xmm9,%xmm2
  102,65,15,114,208,24,                   //psrld         $0x18,%xmm8
  69,15,91,192,                           //cvtdq2ps      %xmm8,%xmm8
  243,15,16,90,12,                        //movss         0xc(%rdx),%xmm3
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_a8_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,15,110,4,56,                        //movd          (%rax,%rdi,1),%xmm0
  102,15,239,201,                         //pxor          %xmm1,%xmm1
  102,15,96,193,                          //punpcklbw     %xmm1,%xmm0
  102,15,97,193,                          //punpcklwd     %xmm1,%xmm0
  15,91,192,                              //cvtdq2ps      %xmm0,%xmm0
  243,15,16,90,12,                        //movss         0xc(%rdx),%xmm3
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  15,89,216,                              //mulps         %xmm0,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  15,87,192,                              //xorps         %xmm0,%xmm0
  102,15,239,201,                         //pxor          %xmm1,%xmm1
  15,87,210,                              //xorps         %xmm2,%xmm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_a8_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,68,15,16,66,8,                      //movss         0x8(%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,89,195,                           //mulps         %xmm3,%xmm8
  102,69,15,91,192,                       //cvtps2dq      %xmm8,%xmm8
  102,65,15,114,240,16,                   //pslld         $0x10,%xmm8
  102,65,15,114,224,16,                   //psrad         $0x10,%xmm8
  102,69,15,107,192,                      //packssdw      %xmm8,%xmm8
  102,69,15,103,192,                      //packuswb      %xmm8,%xmm8
  102,68,15,126,4,56,                     //movd          %xmm8,(%rax,%rdi,1)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_565_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,68,15,126,12,120,                   //movq          (%rax,%rdi,2),%xmm9
  102,15,239,192,                         //pxor          %xmm0,%xmm0
  102,68,15,97,200,                       //punpcklwd     %xmm0,%xmm9
  102,15,110,66,104,                      //movd          0x68(%rdx),%xmm0
  102,15,112,192,0,                       //pshufd        $0x0,%xmm0,%xmm0
  102,65,15,219,193,                      //pand          %xmm9,%xmm0
  15,91,200,                              //cvtdq2ps      %xmm0,%xmm1
  243,15,16,26,                           //movss         (%rdx),%xmm3
  243,15,16,66,116,                       //movss         0x74(%rdx),%xmm0
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  15,89,193,                              //mulps         %xmm1,%xmm0
  102,15,110,74,108,                      //movd          0x6c(%rdx),%xmm1
  102,15,112,201,0,                       //pshufd        $0x0,%xmm1,%xmm1
  102,65,15,219,201,                      //pand          %xmm9,%xmm1
  68,15,91,193,                           //cvtdq2ps      %xmm1,%xmm8
  243,15,16,74,120,                       //movss         0x78(%rdx),%xmm1
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  102,15,110,82,112,                      //movd          0x70(%rdx),%xmm2
  102,15,112,210,0,                       //pshufd        $0x0,%xmm2,%xmm2
  102,65,15,219,209,                      //pand          %xmm9,%xmm2
  68,15,91,194,                           //cvtdq2ps      %xmm2,%xmm8
  243,15,16,82,124,                       //movss         0x7c(%rdx),%xmm2
  15,198,210,0,                           //shufps        $0x0,%xmm2,%xmm2
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  15,198,219,0,                           //shufps        $0x0,%xmm3,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_565_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,68,15,16,130,128,0,0,0,             //movss         0x80(%rdx),%xmm8
  243,68,15,16,138,132,0,0,0,             //movss         0x84(%rdx),%xmm9
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  69,15,40,208,                           //movaps        %xmm8,%xmm10
  68,15,89,208,                           //mulps         %xmm0,%xmm10
  102,69,15,91,210,                       //cvtps2dq      %xmm10,%xmm10
  102,65,15,114,242,11,                   //pslld         $0xb,%xmm10
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  68,15,89,201,                           //mulps         %xmm1,%xmm9
  102,69,15,91,201,                       //cvtps2dq      %xmm9,%xmm9
  102,65,15,114,241,5,                    //pslld         $0x5,%xmm9
  102,69,15,235,202,                      //por           %xmm10,%xmm9
  68,15,89,194,                           //mulps         %xmm2,%xmm8
  102,69,15,91,192,                       //cvtps2dq      %xmm8,%xmm8
  102,69,15,86,193,                       //orpd          %xmm9,%xmm8
  102,65,15,114,240,16,                   //pslld         $0x10,%xmm8
  102,65,15,114,224,16,                   //psrad         $0x10,%xmm8
  102,69,15,107,192,                      //packssdw      %xmm8,%xmm8
  102,68,15,214,4,120,                    //movq          %xmm8,(%rax,%rdi,2)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_8888_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,15,111,28,184,                      //movdqu        (%rax,%rdi,4),%xmm3
  102,15,110,66,16,                       //movd          0x10(%rdx),%xmm0
  102,15,112,192,0,                       //pshufd        $0x0,%xmm0,%xmm0
  102,15,111,203,                         //movdqa        %xmm3,%xmm1
  102,15,114,209,8,                       //psrld         $0x8,%xmm1
  102,15,219,200,                         //pand          %xmm0,%xmm1
  102,15,111,211,                         //movdqa        %xmm3,%xmm2
  102,15,114,210,16,                      //psrld         $0x10,%xmm2
  102,15,219,208,                         //pand          %xmm0,%xmm2
  102,15,219,195,                         //pand          %xmm3,%xmm0
  15,91,192,                              //cvtdq2ps      %xmm0,%xmm0
  243,68,15,16,66,12,                     //movss         0xc(%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  15,91,201,                              //cvtdq2ps      %xmm1,%xmm1
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  15,91,210,                              //cvtdq2ps      %xmm2,%xmm2
  65,15,89,208,                           //mulps         %xmm8,%xmm2
  102,15,114,211,24,                      //psrld         $0x18,%xmm3
  15,91,219,                              //cvtdq2ps      %xmm3,%xmm3
  65,15,89,216,                           //mulps         %xmm8,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_8888_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,68,15,16,66,8,                      //movss         0x8(%rdx),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,200,                           //mulps         %xmm0,%xmm9
  102,69,15,91,201,                       //cvtps2dq      %xmm9,%xmm9
  69,15,40,208,                           //movaps        %xmm8,%xmm10
  68,15,89,209,                           //mulps         %xmm1,%xmm10
  102,69,15,91,210,                       //cvtps2dq      %xmm10,%xmm10
  102,65,15,114,242,8,                    //pslld         $0x8,%xmm10
  102,69,15,235,209,                      //por           %xmm9,%xmm10
  69,15,40,200,                           //movaps        %xmm8,%xmm9
  68,15,89,202,                           //mulps         %xmm2,%xmm9
  102,69,15,91,201,                       //cvtps2dq      %xmm9,%xmm9
  102,65,15,114,241,16,                   //pslld         $0x10,%xmm9
  68,15,89,195,                           //mulps         %xmm3,%xmm8
  102,69,15,91,192,                       //cvtps2dq      %xmm8,%xmm8
  102,65,15,114,240,24,                   //pslld         $0x18,%xmm8
  102,69,15,235,193,                      //por           %xmm9,%xmm8
  102,69,15,235,194,                      //por           %xmm10,%xmm8
  243,68,15,127,4,184,                    //movdqu        %xmm8,(%rax,%rdi,4)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_load_f16_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  243,15,111,4,248,                       //movdqu        (%rax,%rdi,8),%xmm0
  243,15,111,76,248,16,                   //movdqu        0x10(%rax,%rdi,8),%xmm1
  102,15,111,208,                         //movdqa        %xmm0,%xmm2
  102,15,97,209,                          //punpcklwd     %xmm1,%xmm2
  102,15,105,193,                         //punpckhwd     %xmm1,%xmm0
  102,68,15,111,194,                      //movdqa        %xmm2,%xmm8
  102,68,15,97,192,                       //punpcklwd     %xmm0,%xmm8
  102,15,105,208,                         //punpckhwd     %xmm0,%xmm2
  102,15,110,66,100,                      //movd          0x64(%rdx),%xmm0
  102,15,112,216,0,                       //pshufd        $0x0,%xmm0,%xmm3
  102,15,111,203,                         //movdqa        %xmm3,%xmm1
  102,65,15,101,200,                      //pcmpgtw       %xmm8,%xmm1
  102,65,15,223,200,                      //pandn         %xmm8,%xmm1
  102,15,101,218,                         //pcmpgtw       %xmm2,%xmm3
  102,15,223,218,                         //pandn         %xmm2,%xmm3
  102,69,15,239,192,                      //pxor          %xmm8,%xmm8
  102,15,111,193,                         //movdqa        %xmm1,%xmm0
  102,65,15,97,192,                       //punpcklwd     %xmm8,%xmm0
  102,15,114,240,13,                      //pslld         $0xd,%xmm0
  102,15,110,82,92,                       //movd          0x5c(%rdx),%xmm2
  102,68,15,112,202,0,                    //pshufd        $0x0,%xmm2,%xmm9
  65,15,89,193,                           //mulps         %xmm9,%xmm0
  102,65,15,105,200,                      //punpckhwd     %xmm8,%xmm1
  102,15,114,241,13,                      //pslld         $0xd,%xmm1
  65,15,89,201,                           //mulps         %xmm9,%xmm1
  102,15,111,211,                         //movdqa        %xmm3,%xmm2
  102,65,15,97,208,                       //punpcklwd     %xmm8,%xmm2
  102,15,114,242,13,                      //pslld         $0xd,%xmm2
  65,15,89,209,                           //mulps         %xmm9,%xmm2
  102,65,15,105,216,                      //punpckhwd     %xmm8,%xmm3
  102,15,114,243,13,                      //pslld         $0xd,%xmm3
  65,15,89,217,                           //mulps         %xmm9,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_f16_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  102,68,15,110,66,96,                    //movd          0x60(%rdx),%xmm8
  102,69,15,112,192,0,                    //pshufd        $0x0,%xmm8,%xmm8
  102,69,15,111,200,                      //movdqa        %xmm8,%xmm9
  68,15,89,200,                           //mulps         %xmm0,%xmm9
  102,65,15,114,209,13,                   //psrld         $0xd,%xmm9
  102,69,15,111,208,                      //movdqa        %xmm8,%xmm10
  68,15,89,209,                           //mulps         %xmm1,%xmm10
  102,65,15,114,210,13,                   //psrld         $0xd,%xmm10
  102,69,15,111,216,                      //movdqa        %xmm8,%xmm11
  68,15,89,218,                           //mulps         %xmm2,%xmm11
  102,65,15,114,211,13,                   //psrld         $0xd,%xmm11
  68,15,89,195,                           //mulps         %xmm3,%xmm8
  102,65,15,114,208,13,                   //psrld         $0xd,%xmm8
  102,65,15,115,250,2,                    //pslldq        $0x2,%xmm10
  102,69,15,235,209,                      //por           %xmm9,%xmm10
  102,65,15,115,248,2,                    //pslldq        $0x2,%xmm8
  102,69,15,235,195,                      //por           %xmm11,%xmm8
  102,69,15,111,202,                      //movdqa        %xmm10,%xmm9
  102,69,15,98,200,                       //punpckldq     %xmm8,%xmm9
  243,68,15,127,12,248,                   //movdqu        %xmm9,(%rax,%rdi,8)
  102,69,15,106,208,                      //punpckhdq     %xmm8,%xmm10
  243,68,15,127,84,248,16,                //movdqu        %xmm10,0x10(%rax,%rdi,8)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_store_f32_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  72,139,0,                               //mov           (%rax),%rax
  72,137,249,                             //mov           %rdi,%rcx
  72,193,225,4,                           //shl           $0x4,%rcx
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  68,15,40,200,                           //movaps        %xmm0,%xmm9
  68,15,20,201,                           //unpcklps      %xmm1,%xmm9
  68,15,40,210,                           //movaps        %xmm2,%xmm10
  68,15,40,218,                           //movaps        %xmm2,%xmm11
  68,15,20,219,                           //unpcklps      %xmm3,%xmm11
  68,15,21,193,                           //unpckhps      %xmm1,%xmm8
  68,15,21,211,                           //unpckhps      %xmm3,%xmm10
  69,15,40,225,                           //movaps        %xmm9,%xmm12
  102,69,15,20,227,                       //unpcklpd      %xmm11,%xmm12
  102,69,15,21,203,                       //unpckhpd      %xmm11,%xmm9
  69,15,40,216,                           //movaps        %xmm8,%xmm11
  102,69,15,20,218,                       //unpcklpd      %xmm10,%xmm11
  102,69,15,21,194,                       //unpckhpd      %xmm10,%xmm8
  102,68,15,17,36,8,                      //movupd        %xmm12,(%rax,%rcx,1)
  102,68,15,17,76,8,16,                   //movupd        %xmm9,0x10(%rax,%rcx,1)
  102,68,15,17,92,8,32,                   //movupd        %xmm11,0x20(%rax,%rcx,1)
  102,68,15,17,68,8,48,                   //movupd        %xmm8,0x30(%rax,%rcx,1)
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_x_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  68,15,95,192,                           //maxps         %xmm0,%xmm8
  243,68,15,16,8,                         //movss         (%rax),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  102,15,118,192,                         //pcmpeqd       %xmm0,%xmm0
  102,65,15,254,193,                      //paddd         %xmm9,%xmm0
  68,15,93,192,                           //minps         %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,192,                           //movaps        %xmm8,%xmm0
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_clamp_y_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  69,15,87,192,                           //xorps         %xmm8,%xmm8
  68,15,95,193,                           //maxps         %xmm1,%xmm8
  243,68,15,16,8,                         //movss         (%rax),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  102,15,118,201,                         //pcmpeqd       %xmm1,%xmm1
  102,65,15,254,201,                      //paddd         %xmm9,%xmm1
  68,15,93,193,                           //minps         %xmm1,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,200,                           //movaps        %xmm8,%xmm1
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_repeat_x_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,40,200,                           //movaps        %xmm0,%xmm9
  69,15,94,200,                           //divps         %xmm8,%xmm9
  243,69,15,91,209,                       //cvttps2dq     %xmm9,%xmm10
  69,15,91,210,                           //cvtdq2ps      %xmm10,%xmm10
  69,15,194,202,1,                        //cmpltps       %xmm10,%xmm9
  243,68,15,16,26,                        //movss         (%rdx),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,84,217,                           //andps         %xmm9,%xmm11
  69,15,92,211,                           //subps         %xmm11,%xmm10
  69,15,89,208,                           //mulps         %xmm8,%xmm10
  65,15,92,194,                           //subps         %xmm10,%xmm0
  102,69,15,118,201,                      //pcmpeqd       %xmm9,%xmm9
  102,69,15,254,200,                      //paddd         %xmm8,%xmm9
  65,15,93,193,                           //minps         %xmm9,%xmm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_repeat_y_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,0,                         //movss         (%rax),%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  68,15,40,201,                           //movaps        %xmm1,%xmm9
  69,15,94,200,                           //divps         %xmm8,%xmm9
  243,69,15,91,209,                       //cvttps2dq     %xmm9,%xmm10
  69,15,91,210,                           //cvtdq2ps      %xmm10,%xmm10
  69,15,194,202,1,                        //cmpltps       %xmm10,%xmm9
  243,68,15,16,26,                        //movss         (%rdx),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,84,217,                           //andps         %xmm9,%xmm11
  69,15,92,211,                           //subps         %xmm11,%xmm10
  69,15,89,208,                           //mulps         %xmm8,%xmm10
  65,15,92,202,                           //subps         %xmm10,%xmm1
  102,69,15,118,201,                      //pcmpeqd       %xmm9,%xmm9
  102,69,15,254,200,                      //paddd         %xmm8,%xmm9
  65,15,93,201,                           //minps         %xmm9,%xmm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_mirror_x_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,8,                         //movss         (%rax),%xmm9
  69,15,40,193,                           //movaps        %xmm9,%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,92,192,                           //subps         %xmm8,%xmm0
  243,69,15,88,201,                       //addss         %xmm9,%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  68,15,40,208,                           //movaps        %xmm0,%xmm10
  69,15,94,209,                           //divps         %xmm9,%xmm10
  243,69,15,91,218,                       //cvttps2dq     %xmm10,%xmm11
  69,15,91,219,                           //cvtdq2ps      %xmm11,%xmm11
  69,15,194,211,1,                        //cmpltps       %xmm11,%xmm10
  243,68,15,16,34,                        //movss         (%rdx),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  69,15,84,226,                           //andps         %xmm10,%xmm12
  69,15,87,210,                           //xorps         %xmm10,%xmm10
  69,15,92,220,                           //subps         %xmm12,%xmm11
  69,15,89,217,                           //mulps         %xmm9,%xmm11
  65,15,92,195,                           //subps         %xmm11,%xmm0
  65,15,92,192,                           //subps         %xmm8,%xmm0
  68,15,92,208,                           //subps         %xmm0,%xmm10
  65,15,84,194,                           //andps         %xmm10,%xmm0
  102,69,15,118,201,                      //pcmpeqd       %xmm9,%xmm9
  102,69,15,254,200,                      //paddd         %xmm8,%xmm9
  65,15,93,193,                           //minps         %xmm9,%xmm0
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_mirror_y_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,68,15,16,8,                         //movss         (%rax),%xmm9
  69,15,40,193,                           //movaps        %xmm9,%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,92,200,                           //subps         %xmm8,%xmm1
  243,69,15,88,201,                       //addss         %xmm9,%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  68,15,40,209,                           //movaps        %xmm1,%xmm10
  69,15,94,209,                           //divps         %xmm9,%xmm10
  243,69,15,91,218,                       //cvttps2dq     %xmm10,%xmm11
  69,15,91,219,                           //cvtdq2ps      %xmm11,%xmm11
  69,15,194,211,1,                        //cmpltps       %xmm11,%xmm10
  243,68,15,16,34,                        //movss         (%rdx),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  69,15,84,226,                           //andps         %xmm10,%xmm12
  69,15,87,210,                           //xorps         %xmm10,%xmm10
  69,15,92,220,                           //subps         %xmm12,%xmm11
  69,15,89,217,                           //mulps         %xmm9,%xmm11
  65,15,92,203,                           //subps         %xmm11,%xmm1
  65,15,92,200,                           //subps         %xmm8,%xmm1
  68,15,92,209,                           //subps         %xmm1,%xmm10
  65,15,84,202,                           //andps         %xmm10,%xmm1
  102,69,15,118,201,                      //pcmpeqd       %xmm9,%xmm9
  102,69,15,254,200,                      //paddd         %xmm8,%xmm9
  65,15,93,201,                           //minps         %xmm9,%xmm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_2x3_sse2[] = {
  68,15,40,201,                           //movaps        %xmm1,%xmm9
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,15,16,0,                            //movss         (%rax),%xmm0
  243,15,16,72,4,                         //movss         0x4(%rax),%xmm1
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  243,68,15,16,80,8,                      //movss         0x8(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,16,                     //movss         0x10(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,88,194,                           //addps         %xmm10,%xmm0
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  243,68,15,16,80,12,                     //movss         0xc(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,20,                     //movss         0x14(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  65,15,88,202,                           //addps         %xmm10,%xmm1
  72,173,                                 //lods          %ds:(%rsi),%rax
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_3x4_sse2[] = {
  68,15,40,201,                           //movaps        %xmm1,%xmm9
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,15,16,0,                            //movss         (%rax),%xmm0
  243,15,16,72,4,                         //movss         0x4(%rax),%xmm1
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  243,68,15,16,80,12,                     //movss         0xc(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,24,                     //movss         0x18(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,96,36,                     //movss         0x24(%rax),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  68,15,89,218,                           //mulps         %xmm2,%xmm11
  69,15,88,220,                           //addps         %xmm12,%xmm11
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,88,194,                           //addps         %xmm10,%xmm0
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  243,68,15,16,80,16,                     //movss         0x10(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,28,                     //movss         0x1c(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,96,40,                     //movss         0x28(%rax),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  68,15,89,218,                           //mulps         %xmm2,%xmm11
  69,15,88,220,                           //addps         %xmm12,%xmm11
  69,15,89,209,                           //mulps         %xmm9,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,89,200,                           //mulps         %xmm8,%xmm1
  65,15,88,202,                           //addps         %xmm10,%xmm1
  243,68,15,16,80,8,                      //movss         0x8(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,20,                     //movss         0x14(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,96,32,                     //movss         0x20(%rax),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  243,68,15,16,104,44,                    //movss         0x2c(%rax),%xmm13
  69,15,198,237,0,                        //shufps        $0x0,%xmm13,%xmm13
  68,15,89,226,                           //mulps         %xmm2,%xmm12
  69,15,88,229,                           //addps         %xmm13,%xmm12
  69,15,89,217,                           //mulps         %xmm9,%xmm11
  69,15,88,220,                           //addps         %xmm12,%xmm11
  69,15,89,208,                           //mulps         %xmm8,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,210,                           //movaps        %xmm10,%xmm2
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_matrix_perspective_sse2[] = {
  68,15,40,192,                           //movaps        %xmm0,%xmm8
  72,173,                                 //lods          %ds:(%rsi),%rax
  243,15,16,0,                            //movss         (%rax),%xmm0
  243,68,15,16,72,4,                      //movss         0x4(%rax),%xmm9
  15,198,192,0,                           //shufps        $0x0,%xmm0,%xmm0
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  243,68,15,16,80,8,                      //movss         0x8(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  68,15,89,201,                           //mulps         %xmm1,%xmm9
  69,15,88,202,                           //addps         %xmm10,%xmm9
  65,15,89,192,                           //mulps         %xmm8,%xmm0
  65,15,88,193,                           //addps         %xmm9,%xmm0
  243,68,15,16,72,12,                     //movss         0xc(%rax),%xmm9
  69,15,198,201,0,                        //shufps        $0x0,%xmm9,%xmm9
  243,68,15,16,80,16,                     //movss         0x10(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,20,                     //movss         0x14(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  68,15,89,209,                           //mulps         %xmm1,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  69,15,89,200,                           //mulps         %xmm8,%xmm9
  69,15,88,202,                           //addps         %xmm10,%xmm9
  243,68,15,16,80,24,                     //movss         0x18(%rax),%xmm10
  69,15,198,210,0,                        //shufps        $0x0,%xmm10,%xmm10
  243,68,15,16,88,28,                     //movss         0x1c(%rax),%xmm11
  69,15,198,219,0,                        //shufps        $0x0,%xmm11,%xmm11
  243,68,15,16,96,32,                     //movss         0x20(%rax),%xmm12
  69,15,198,228,0,                        //shufps        $0x0,%xmm12,%xmm12
  68,15,89,217,                           //mulps         %xmm1,%xmm11
  69,15,88,220,                           //addps         %xmm12,%xmm11
  69,15,89,208,                           //mulps         %xmm8,%xmm10
  69,15,88,211,                           //addps         %xmm11,%xmm10
  65,15,83,202,                           //rcpps         %xmm10,%xmm1
  15,89,193,                              //mulps         %xmm1,%xmm0
  68,15,89,201,                           //mulps         %xmm1,%xmm9
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,201,                           //movaps        %xmm9,%xmm1
  255,224,                                //jmpq          *%rax
};

CODE const uint8_t sk_linear_gradient_2stops_sse2[] = {
  72,173,                                 //lods          %ds:(%rsi),%rax
  68,15,16,8,                             //movups        (%rax),%xmm9
  15,16,88,16,                            //movups        0x10(%rax),%xmm3
  68,15,40,195,                           //movaps        %xmm3,%xmm8
  69,15,198,192,0,                        //shufps        $0x0,%xmm8,%xmm8
  65,15,40,201,                           //movaps        %xmm9,%xmm1
  15,198,201,0,                           //shufps        $0x0,%xmm1,%xmm1
  68,15,89,192,                           //mulps         %xmm0,%xmm8
  68,15,88,193,                           //addps         %xmm1,%xmm8
  15,40,203,                              //movaps        %xmm3,%xmm1
  15,198,201,85,                          //shufps        $0x55,%xmm1,%xmm1
  65,15,40,209,                           //movaps        %xmm9,%xmm2
  15,198,210,85,                          //shufps        $0x55,%xmm2,%xmm2
  15,89,200,                              //mulps         %xmm0,%xmm1
  15,88,202,                              //addps         %xmm2,%xmm1
  15,40,211,                              //movaps        %xmm3,%xmm2
  15,198,210,170,                         //shufps        $0xaa,%xmm2,%xmm2
  69,15,40,209,                           //movaps        %xmm9,%xmm10
  69,15,198,210,170,                      //shufps        $0xaa,%xmm10,%xmm10
  15,89,208,                              //mulps         %xmm0,%xmm2
  65,15,88,210,                           //addps         %xmm10,%xmm2
  15,198,219,255,                         //shufps        $0xff,%xmm3,%xmm3
  69,15,198,201,255,                      //shufps        $0xff,%xmm9,%xmm9
  15,89,216,                              //mulps         %xmm0,%xmm3
  65,15,88,217,                           //addps         %xmm9,%xmm3
  72,173,                                 //lods          %ds:(%rsi),%rax
  65,15,40,192,                           //movaps        %xmm8,%xmm0
  255,224,                                //jmpq          *%rax
};
#endif
