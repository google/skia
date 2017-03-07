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
  0xa9bd5bf7,                                 //stp           x23, x22, [sp, #-48]!
  0xa90153f5,                                 //stp           x21, x20, [sp, #16]
  0xa9027bf3,                                 //stp           x19, x30, [sp, #32]
  0xaa0103f5,                                 //mov           x21, x1
  0xf84086b7,                                 //ldr           x23, [x21], #8
  0xaa0003f6,                                 //mov           x22, x0
  0xaa0303f3,                                 //mov           x19, x3
  0xaa0203f4,                                 //mov           x20, x2
  0x910012c8,                                 //add           x8, x22, #0x4
  0xeb13011f,                                 //cmp           x8, x19
  0x54000069,                                 //b.ls          34 <sk_start_pipeline_aarch64+0x34>  // b.plast
  0xaa1603e0,                                 //mov           x0, x22
  0x14000012,                                 //b             78 <sk_start_pipeline_aarch64+0x78>
  0x6f00e400,                                 //movi          v0.2d, #0x0
  0x6f00e401,                                 //movi          v1.2d, #0x0
  0x6f00e402,                                 //movi          v2.2d, #0x0
  0x6f00e403,                                 //movi          v3.2d, #0x0
  0x6f00e404,                                 //movi          v4.2d, #0x0
  0x6f00e405,                                 //movi          v5.2d, #0x0
  0x6f00e406,                                 //movi          v6.2d, #0x0
  0x6f00e407,                                 //movi          v7.2d, #0x0
  0xaa1603e0,                                 //mov           x0, x22
  0xaa1503e1,                                 //mov           x1, x21
  0xaa1403e2,                                 //mov           x2, x20
  0xd63f02e0,                                 //blr           x23
  0x910022c8,                                 //add           x8, x22, #0x8
  0x910012c0,                                 //add           x0, x22, #0x4
  0xeb13011f,                                 //cmp           x8, x19
  0xaa0003f6,                                 //mov           x22, x0
  0x54fffe09,                                 //b.ls          34 <sk_start_pipeline_aarch64+0x34>  // b.plast
  0xa9427bf3,                                 //ldp           x19, x30, [sp, #32]
  0xa94153f5,                                 //ldp           x21, x20, [sp, #16]
  0xa8c35bf7,                                 //ldp           x23, x22, [sp], #48
  0xd65f03c0,                                 //ret
};

CODE const uint32_t sk_just_return_aarch64[] = {
  0xd65f03c0,                                 //ret
};

CODE const uint32_t sk_seed_shader_aarch64[] = {
  0xaa0203e9,                                 //mov           x9, x2
  0xa9400c28,                                 //ldp           x8, x3, [x1]
  0x4ddfc922,                                 //ld1r          {v2.4s}, [x9], #4
  0x3cc14047,                                 //ldur          q7, [x2, #20]
  0x4e040c00,                                 //dup           v0.4s, w0
  0x4d40c901,                                 //ld1r          {v1.4s}, [x8]
  0x4d40c926,                                 //ld1r          {v6.4s}, [x9]
  0x4e21d800,                                 //scvtf         v0.4s, v0.4s
  0x91004028,                                 //add           x8, x1, #0x10
  0x4e21d821,                                 //scvtf         v1.4s, v1.4s
  0x4e26d400,                                 //fadd          v0.4s, v0.4s, v6.4s
  0x6f00e403,                                 //movi          v3.2d, #0x0
  0x6f00e404,                                 //movi          v4.2d, #0x0
  0x6f00e405,                                 //movi          v5.2d, #0x0
  0x4e26d421,                                 //fadd          v1.4s, v1.4s, v6.4s
  0x6f00e406,                                 //movi          v6.2d, #0x0
  0x4e20d4e0,                                 //fadd          v0.4s, v7.4s, v0.4s
  0x6f00e407,                                 //movi          v7.2d, #0x0
  0xaa0803e1,                                 //mov           x1, x8
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_constant_color_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0x3dc00103,                                 //ldr           q3, [x8]
  0x4e040460,                                 //dup           v0.4s, v3.s[0]
  0x4e0c0461,                                 //dup           v1.4s, v3.s[1]
  0x4e140462,                                 //dup           v2.4s, v3.s[2]
  0x4e1c0463,                                 //dup           v3.4s, v3.s[3]
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_clear_aarch64[] = {
  0xf8408423,                                 //ldr           x3, [x1], #8
  0x6f00e400,                                 //movi          v0.2d, #0x0
  0x6f00e401,                                 //movi          v1.2d, #0x0
  0x6f00e402,                                 //movi          v2.2d, #0x0
  0x6f00e403,                                 //movi          v3.2d, #0x0
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_plus__aarch64[] = {
  0xf8408423,                                 //ldr           x3, [x1], #8
  0x4e24d400,                                 //fadd          v0.4s, v0.4s, v4.4s
  0x4e25d421,                                 //fadd          v1.4s, v1.4s, v5.4s
  0x4e26d442,                                 //fadd          v2.4s, v2.4s, v6.4s
  0x4e27d463,                                 //fadd          v3.4s, v3.4s, v7.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_srcover_aarch64[] = {
  0x4d40c850,                                 //ld1r          {v16.4s}, [x2]
  0xf8408423,                                 //ldr           x3, [x1], #8
  0x4ea3d610,                                 //fsub          v16.4s, v16.4s, v3.4s
  0x4e24ce00,                                 //fmla          v0.4s, v16.4s, v4.4s
  0x4e25ce01,                                 //fmla          v1.4s, v16.4s, v5.4s
  0x4e26ce02,                                 //fmla          v2.4s, v16.4s, v6.4s
  0x4e27ce03,                                 //fmla          v3.4s, v16.4s, v7.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_dstover_aarch64[] = {
  0x4d40c851,                                 //ld1r          {v17.4s}, [x2]
  0xf8408423,                                 //ldr           x3, [x1], #8
  0x4ea41c90,                                 //mov           v16.16b, v4.16b
  0x4ea61cd2,                                 //mov           v18.16b, v6.16b
  0x4ea7d634,                                 //fsub          v20.4s, v17.4s, v7.4s
  0x4ea51cb1,                                 //mov           v17.16b, v5.16b
  0x4ea71cf3,                                 //mov           v19.16b, v7.16b
  0x4e20ce90,                                 //fmla          v16.4s, v20.4s, v0.4s
  0x4e21ce91,                                 //fmla          v17.4s, v20.4s, v1.4s
  0x4e22ce92,                                 //fmla          v18.4s, v20.4s, v2.4s
  0x4e23ce93,                                 //fmla          v19.4s, v20.4s, v3.4s
  0x4eb01e00,                                 //mov           v0.16b, v16.16b
  0x4eb11e21,                                 //mov           v1.16b, v17.16b
  0x4eb21e42,                                 //mov           v2.16b, v18.16b
  0x4eb31e63,                                 //mov           v3.16b, v19.16b
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_clamp_0_aarch64[] = {
  0xf8408423,                                 //ldr           x3, [x1], #8
  0x6f00e410,                                 //movi          v16.2d, #0x0
  0x4e30f400,                                 //fmax          v0.4s, v0.4s, v16.4s
  0x4e30f421,                                 //fmax          v1.4s, v1.4s, v16.4s
  0x4e30f442,                                 //fmax          v2.4s, v2.4s, v16.4s
  0x4e30f463,                                 //fmax          v3.4s, v3.4s, v16.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_clamp_1_aarch64[] = {
  0x4d40c850,                                 //ld1r          {v16.4s}, [x2]
  0xf8408423,                                 //ldr           x3, [x1], #8
  0x4eb0f400,                                 //fmin          v0.4s, v0.4s, v16.4s
  0x4eb0f421,                                 //fmin          v1.4s, v1.4s, v16.4s
  0x4eb0f442,                                 //fmin          v2.4s, v2.4s, v16.4s
  0x4eb0f463,                                 //fmin          v3.4s, v3.4s, v16.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_clamp_a_aarch64[] = {
  0x4d40c850,                                 //ld1r          {v16.4s}, [x2]
  0xf8408423,                                 //ldr           x3, [x1], #8
  0x4eb0f463,                                 //fmin          v3.4s, v3.4s, v16.4s
  0x4ea3f400,                                 //fmin          v0.4s, v0.4s, v3.4s
  0x4ea3f421,                                 //fmin          v1.4s, v1.4s, v3.4s
  0x4ea3f442,                                 //fmin          v2.4s, v2.4s, v3.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_set_rgb_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0xaa0803e9,                                 //mov           x9, x8
  0x4ddfc920,                                 //ld1r          {v0.4s}, [x9], #4
  0x91002108,                                 //add           x8, x8, #0x8
  0x4d40c902,                                 //ld1r          {v2.4s}, [x8]
  0x4d40c921,                                 //ld1r          {v1.4s}, [x9]
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_swap_rb_aarch64[] = {
  0xf8408423,                                 //ldr           x3, [x1], #8
  0x4ea01c10,                                 //mov           v16.16b, v0.16b
  0x4ea21c40,                                 //mov           v0.16b, v2.16b
  0x4eb01e02,                                 //mov           v2.16b, v16.16b
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_swap_aarch64[] = {
  0xf8408423,                                 //ldr           x3, [x1], #8
  0x4ea31c70,                                 //mov           v16.16b, v3.16b
  0x4ea21c51,                                 //mov           v17.16b, v2.16b
  0x4ea11c32,                                 //mov           v18.16b, v1.16b
  0x4ea01c13,                                 //mov           v19.16b, v0.16b
  0x4ea41c80,                                 //mov           v0.16b, v4.16b
  0x4ea51ca1,                                 //mov           v1.16b, v5.16b
  0x4ea61cc2,                                 //mov           v2.16b, v6.16b
  0x4ea71ce3,                                 //mov           v3.16b, v7.16b
  0x4eb31e64,                                 //mov           v4.16b, v19.16b
  0x4eb21e45,                                 //mov           v5.16b, v18.16b
  0x4eb11e26,                                 //mov           v6.16b, v17.16b
  0x4eb01e07,                                 //mov           v7.16b, v16.16b
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_move_src_dst_aarch64[] = {
  0xf8408423,                                 //ldr           x3, [x1], #8
  0x4ea01c04,                                 //mov           v4.16b, v0.16b
  0x4ea11c25,                                 //mov           v5.16b, v1.16b
  0x4ea21c46,                                 //mov           v6.16b, v2.16b
  0x4ea31c67,                                 //mov           v7.16b, v3.16b
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_move_dst_src_aarch64[] = {
  0xf8408423,                                 //ldr           x3, [x1], #8
  0x4ea41c80,                                 //mov           v0.16b, v4.16b
  0x4ea51ca1,                                 //mov           v1.16b, v5.16b
  0x4ea61cc2,                                 //mov           v2.16b, v6.16b
  0x4ea71ce3,                                 //mov           v3.16b, v7.16b
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_premul_aarch64[] = {
  0xf8408423,                                 //ldr           x3, [x1], #8
  0x6e23dc00,                                 //fmul          v0.4s, v0.4s, v3.4s
  0x6e23dc21,                                 //fmul          v1.4s, v1.4s, v3.4s
  0x6e23dc42,                                 //fmul          v2.4s, v2.4s, v3.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_unpremul_aarch64[] = {
  0x4d40c850,                                 //ld1r          {v16.4s}, [x2]
  0xf8408423,                                 //ldr           x3, [x1], #8
  0x4ea0d871,                                 //fcmeq         v17.4s, v3.4s, #0.0
  0x6e23fe10,                                 //fdiv          v16.4s, v16.4s, v3.4s
  0x4e711e10,                                 //bic           v16.16b, v16.16b, v17.16b
  0x6e20de00,                                 //fmul          v0.4s, v16.4s, v0.4s
  0x6e21de01,                                 //fmul          v1.4s, v16.4s, v1.4s
  0x6e22de02,                                 //fmul          v2.4s, v16.4s, v2.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_from_srgb_aarch64[] = {
  0x9100e048,                                 //add           x8, x2, #0x38
  0x4d40c910,                                 //ld1r          {v16.4s}, [x8]
  0x9100d048,                                 //add           x8, x2, #0x34
  0x2d47cc52,                                 //ldp           s18, s19, [x2, #60]
  0x4d40c911,                                 //ld1r          {v17.4s}, [x8]
  0x6e22dc54,                                 //fmul          v20.4s, v2.4s, v2.4s
  0x4eb01e15,                                 //mov           v21.16b, v16.16b
  0x4eb01e17,                                 //mov           v23.16b, v16.16b
  0x4f921050,                                 //fmla          v16.4s, v2.4s, v18.s[0]
  0x4eb11e36,                                 //mov           v22.16b, v17.16b
  0x4eb11e38,                                 //mov           v24.16b, v17.16b
  0x4e34ce11,                                 //fmla          v17.4s, v16.4s, v20.4s
  0x6e20dc10,                                 //fmul          v16.4s, v0.4s, v0.4s
  0x91011048,                                 //add           x8, x2, #0x44
  0x4f921015,                                 //fmla          v21.4s, v0.4s, v18.s[0]
  0x4e30ceb6,                                 //fmla          v22.4s, v21.4s, v16.4s
  0x4d40c910,                                 //ld1r          {v16.4s}, [x8]
  0xf8408423,                                 //ldr           x3, [x1], #8
  0x6e21dc34,                                 //fmul          v20.4s, v1.4s, v1.4s
  0x4f921037,                                 //fmla          v23.4s, v1.4s, v18.s[0]
  0x4f939015,                                 //fmul          v21.4s, v0.4s, v19.s[0]
  0x4f939032,                                 //fmul          v18.4s, v1.4s, v19.s[0]
  0x4f939053,                                 //fmul          v19.4s, v2.4s, v19.s[0]
  0x6ea0e600,                                 //fcmgt         v0.4s, v16.4s, v0.4s
  0x6ea1e601,                                 //fcmgt         v1.4s, v16.4s, v1.4s
  0x6ea2e602,                                 //fcmgt         v2.4s, v16.4s, v2.4s
  0x4e34cef8,                                 //fmla          v24.4s, v23.4s, v20.4s
  0x6e761ea0,                                 //bsl           v0.16b, v21.16b, v22.16b
  0x6e781e41,                                 //bsl           v1.16b, v18.16b, v24.16b
  0x6e711e62,                                 //bsl           v2.16b, v19.16b, v17.16b
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_to_srgb_aarch64[] = {
  0x6ea1d811,                                 //frsqrte       v17.4s, v0.4s
  0x6ea1d835,                                 //frsqrte       v21.4s, v1.4s
  0x6e31de37,                                 //fmul          v23.4s, v17.4s, v17.4s
  0x6ea1d856,                                 //frsqrte       v22.4s, v2.4s
  0x6e35deb9,                                 //fmul          v25.4s, v21.4s, v21.4s
  0x4eb7fc17,                                 //frsqrts       v23.4s, v0.4s, v23.4s
  0x91015048,                                 //add           x8, x2, #0x54
  0x6e36deda,                                 //fmul          v26.4s, v22.4s, v22.4s
  0x4eb9fc39,                                 //frsqrts       v25.4s, v1.4s, v25.4s
  0x6e37de31,                                 //fmul          v17.4s, v17.4s, v23.4s
  0x4d40c914,                                 //ld1r          {v20.4s}, [x8]
  0x4ebafc5a,                                 //frsqrts       v26.4s, v2.4s, v26.4s
  0x6e39deb5,                                 //fmul          v21.4s, v21.4s, v25.4s
  0x4ea1da37,                                 //frecpe        v23.4s, v17.4s
  0xbd405053,                                 //ldr           s19, [x2, #80]
  0x91016048,                                 //add           x8, x2, #0x58
  0x6e3aded6,                                 //fmul          v22.4s, v22.4s, v26.4s
  0x4ea1dabb,                                 //frecpe        v27.4s, v21.4s
  0x4e37fe3d,                                 //frecps        v29.4s, v17.4s, v23.4s
  0x2d494052,                                 //ldp           s18, s16, [x2, #72]
  0x4d40c918,                                 //ld1r          {v24.4s}, [x8]
  0x4ea1dadc,                                 //frecpe        v28.4s, v22.4s
  0x6e3ddef7,                                 //fmul          v23.4s, v23.4s, v29.4s
  0x4e3bfebd,                                 //frecps        v29.4s, v21.4s, v27.4s
  0x6e3ddf7b,                                 //fmul          v27.4s, v27.4s, v29.4s
  0x4e3cfedd,                                 //frecps        v29.4s, v22.4s, v28.4s
  0x6e3ddf9c,                                 //fmul          v28.4s, v28.4s, v29.4s
  0x4eb41e9d,                                 //mov           v29.16b, v20.16b
  0x6ea1da39,                                 //frsqrte       v25.4s, v17.4s
  0x4f9312fd,                                 //fmla          v29.4s, v23.4s, v19.s[0]
  0x4eb41e97,                                 //mov           v23.16b, v20.16b
  0x4f92901a,                                 //fmul          v26.4s, v0.4s, v18.s[0]
  0x4f931377,                                 //fmla          v23.4s, v27.4s, v19.s[0]
  0x4f931394,                                 //fmla          v20.4s, v28.4s, v19.s[0]
  0x4f929033,                                 //fmul          v19.4s, v1.4s, v18.s[0]
  0x4f929052,                                 //fmul          v18.4s, v2.4s, v18.s[0]
  0x6ea0e700,                                 //fcmgt         v0.4s, v24.4s, v0.4s
  0x6ea1e701,                                 //fcmgt         v1.4s, v24.4s, v1.4s
  0x6ea2e702,                                 //fcmgt         v2.4s, v24.4s, v2.4s
  0x6e39df38,                                 //fmul          v24.4s, v25.4s, v25.4s
  0x6ea1dabb,                                 //frsqrte       v27.4s, v21.4s
  0x4eb8fe31,                                 //frsqrts       v17.4s, v17.4s, v24.4s
  0x6ea1dadc,                                 //frsqrte       v28.4s, v22.4s
  0x6e3bdf78,                                 //fmul          v24.4s, v27.4s, v27.4s
  0x6e31df31,                                 //fmul          v17.4s, v25.4s, v17.4s
  0x4eb8feb5,                                 //frsqrts       v21.4s, v21.4s, v24.4s
  0x6e3cdf98,                                 //fmul          v24.4s, v28.4s, v28.4s
  0x4f90123d,                                 //fmla          v29.4s, v17.4s, v16.s[0]
  0x4d40c851,                                 //ld1r          {v17.4s}, [x2]
  0x4eb8fed6,                                 //frsqrts       v22.4s, v22.4s, v24.4s
  0x6e35df75,                                 //fmul          v21.4s, v27.4s, v21.4s
  0x6e36df96,                                 //fmul          v22.4s, v28.4s, v22.4s
  0xf8408423,                                 //ldr           x3, [x1], #8
  0x4f9012b7,                                 //fmla          v23.4s, v21.4s, v16.s[0]
  0x4f9012d4,                                 //fmla          v20.4s, v22.4s, v16.s[0]
  0x4ebdf630,                                 //fmin          v16.4s, v17.4s, v29.4s
  0x4eb7f635,                                 //fmin          v21.4s, v17.4s, v23.4s
  0x4eb4f631,                                 //fmin          v17.4s, v17.4s, v20.4s
  0x6e701f40,                                 //bsl           v0.16b, v26.16b, v16.16b
  0x6e751e61,                                 //bsl           v1.16b, v19.16b, v21.16b
  0x6e711e42,                                 //bsl           v2.16b, v18.16b, v17.16b
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_scale_1_float_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0xbd400110,                                 //ldr           s16, [x8]
  0x4f909000,                                 //fmul          v0.4s, v0.4s, v16.s[0]
  0x4f909021,                                 //fmul          v1.4s, v1.4s, v16.s[0]
  0x4f909042,                                 //fmul          v2.4s, v2.4s, v16.s[0]
  0x4f909063,                                 //fmul          v3.4s, v3.4s, v16.s[0]
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_scale_u8_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0xbd400c51,                                 //ldr           s17, [x2, #12]
  0xf9400108,                                 //ldr           x8, [x8]
  0x8b000108,                                 //add           x8, x8, x0
  0x39400109,                                 //ldrb          w9, [x8]
  0x3940050a,                                 //ldrb          w10, [x8, #1]
  0x3940090b,                                 //ldrb          w11, [x8, #2]
  0x39400d08,                                 //ldrb          w8, [x8, #3]
  0x4e021d30,                                 //mov           v16.h[0], w9
  0x4e061d50,                                 //mov           v16.h[1], w10
  0x4e0a1d70,                                 //mov           v16.h[2], w11
  0x4e0e1d10,                                 //mov           v16.h[3], w8
  0x2f07b7f0,                                 //bic           v16.4h, #0xff, lsl #8
  0x2f10a610,                                 //uxtl          v16.4s, v16.4h
  0x6e21da10,                                 //ucvtf         v16.4s, v16.4s
  0x4f919210,                                 //fmul          v16.4s, v16.4s, v17.s[0]
  0x6e20de00,                                 //fmul          v0.4s, v16.4s, v0.4s
  0x6e21de01,                                 //fmul          v1.4s, v16.4s, v1.4s
  0x6e22de02,                                 //fmul          v2.4s, v16.4s, v2.4s
  0x6e23de03,                                 //fmul          v3.4s, v16.4s, v3.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_lerp_1_float_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0x4ea4d411,                                 //fsub          v17.4s, v0.4s, v4.4s
  0x4ea41c80,                                 //mov           v0.16b, v4.16b
  0x4ea5d432,                                 //fsub          v18.4s, v1.4s, v5.4s
  0xbd400110,                                 //ldr           s16, [x8]
  0x4ea51ca1,                                 //mov           v1.16b, v5.16b
  0x4f901220,                                 //fmla          v0.4s, v17.4s, v16.s[0]
  0x4ea6d451,                                 //fsub          v17.4s, v2.4s, v6.4s
  0x4f901241,                                 //fmla          v1.4s, v18.4s, v16.s[0]
  0x4ea61cc2,                                 //mov           v2.16b, v6.16b
  0x4ea7d472,                                 //fsub          v18.4s, v3.4s, v7.4s
  0x4ea71ce3,                                 //mov           v3.16b, v7.16b
  0x4f901222,                                 //fmla          v2.4s, v17.4s, v16.s[0]
  0x4f901243,                                 //fmla          v3.4s, v18.4s, v16.s[0]
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_lerp_u8_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0xbd400c51,                                 //ldr           s17, [x2, #12]
  0x4ea4d412,                                 //fsub          v18.4s, v0.4s, v4.4s
  0xf9400108,                                 //ldr           x8, [x8]
  0x8b000108,                                 //add           x8, x8, x0
  0x39400109,                                 //ldrb          w9, [x8]
  0x3940050a,                                 //ldrb          w10, [x8, #1]
  0x3940090b,                                 //ldrb          w11, [x8, #2]
  0x39400d08,                                 //ldrb          w8, [x8, #3]
  0x4e021d30,                                 //mov           v16.h[0], w9
  0x4e061d50,                                 //mov           v16.h[1], w10
  0x4e0a1d70,                                 //mov           v16.h[2], w11
  0x4e0e1d10,                                 //mov           v16.h[3], w8
  0x2f07b7f0,                                 //bic           v16.4h, #0xff, lsl #8
  0x2f10a600,                                 //uxtl          v0.4s, v16.4h
  0x6e21d800,                                 //ucvtf         v0.4s, v0.4s
  0x4f919010,                                 //fmul          v16.4s, v0.4s, v17.s[0]
  0x4ea41c80,                                 //mov           v0.16b, v4.16b
  0x4ea5d431,                                 //fsub          v17.4s, v1.4s, v5.4s
  0x4ea51ca1,                                 //mov           v1.16b, v5.16b
  0x4e32ce00,                                 //fmla          v0.4s, v16.4s, v18.4s
  0x4ea6d452,                                 //fsub          v18.4s, v2.4s, v6.4s
  0x4e31ce01,                                 //fmla          v1.4s, v16.4s, v17.4s
  0x4ea61cc2,                                 //mov           v2.16b, v6.16b
  0x4ea7d471,                                 //fsub          v17.4s, v3.4s, v7.4s
  0x4ea71ce3,                                 //mov           v3.16b, v7.16b
  0x4e32ce02,                                 //fmla          v2.4s, v16.4s, v18.4s
  0x4e31ce03,                                 //fmla          v3.4s, v16.4s, v17.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_lerp_565_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0xd37ff809,                                 //lsl           x9, x0, #1
  0x2d4ec851,                                 //ldp           s17, s18, [x2, #116]
  0x4ea4d413,                                 //fsub          v19.4s, v0.4s, v4.4s
  0xf9400108,                                 //ldr           x8, [x8]
  0x4ea41c80,                                 //mov           v0.16b, v4.16b
  0xfc696903,                                 //ldr           d3, [x8, x9]
  0x9101a048,                                 //add           x8, x2, #0x68
  0x4d40c910,                                 //ld1r          {v16.4s}, [x8]
  0x9101b048,                                 //add           x8, x2, #0x6c
  0x2f10a463,                                 //uxtl          v3.4s, v3.4h
  0x4e231e10,                                 //and           v16.16b, v16.16b, v3.16b
  0x4e21da10,                                 //scvtf         v16.4s, v16.4s
  0x4f919210,                                 //fmul          v16.4s, v16.4s, v17.s[0]
  0x4d40c911,                                 //ld1r          {v17.4s}, [x8]
  0x9101c048,                                 //add           x8, x2, #0x70
  0x4e33ce00,                                 //fmla          v0.4s, v16.4s, v19.4s
  0x4ea5d430,                                 //fsub          v16.4s, v1.4s, v5.4s
  0x4e231e31,                                 //and           v17.16b, v17.16b, v3.16b
  0x4e21da31,                                 //scvtf         v17.4s, v17.4s
  0x4f929231,                                 //fmul          v17.4s, v17.4s, v18.s[0]
  0x4d40c912,                                 //ld1r          {v18.4s}, [x8]
  0x4ea51ca1,                                 //mov           v1.16b, v5.16b
  0x4e30ce21,                                 //fmla          v1.4s, v17.4s, v16.4s
  0xbd407c50,                                 //ldr           s16, [x2, #124]
  0x4e231e52,                                 //and           v18.16b, v18.16b, v3.16b
  0x4d40c843,                                 //ld1r          {v3.4s}, [x2]
  0x4e21da52,                                 //scvtf         v18.4s, v18.4s
  0x4ea6d451,                                 //fsub          v17.4s, v2.4s, v6.4s
  0x4ea61cc2,                                 //mov           v2.16b, v6.16b
  0x4f909250,                                 //fmul          v16.4s, v18.4s, v16.s[0]
  0x4e31ce02,                                 //fmla          v2.4s, v16.4s, v17.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_load_tables_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0x9100404b,                                 //add           x11, x2, #0x10
  0x4d40c960,                                 //ld1r          {v0.4s}, [x11]
  0xd37ef409,                                 //lsl           x9, x0, #2
  0xa9402d0a,                                 //ldp           x10, x11, [x8]
  0x3ce96942,                                 //ldr           q2, [x10, x9]
  0xa9412109,                                 //ldp           x9, x8, [x8, #16]
  0x4e221c01,                                 //and           v1.16b, v0.16b, v2.16b
  0x0e143c2c,                                 //mov           w12, v1.s[2]
  0xbc6c5971,                                 //ldr           s17, [x11, w12, uxtw #2]
  0x1e26002c,                                 //fmov          w12, s1
  0x6f380443,                                 //ushr          v3.4s, v2.4s, #8
  0x6f300450,                                 //ushr          v16.4s, v2.4s, #16
  0x8b2c496c,                                 //add           x12, x11, w12, uxtw #2
  0x0e0c3c2a,                                 //mov           w10, v1.s[1]
  0x0e1c3c2d,                                 //mov           w13, v1.s[3]
  0x4e231c01,                                 //and           v1.16b, v0.16b, v3.16b
  0x4e301c03,                                 //and           v3.16b, v0.16b, v16.16b
  0x0d408180,                                 //ld1           {v0.s}[0], [x12]
  0x0e143c2c,                                 //mov           w12, v1.s[2]
  0xbc6c5932,                                 //ldr           s18, [x9, w12, uxtw #2]
  0x1e26002c,                                 //fmov          w12, s1
  0x8b2a496a,                                 //add           x10, x11, w10, uxtw #2
  0xbc6d5970,                                 //ldr           s16, [x11, w13, uxtw #2]
  0x0e0c3c2b,                                 //mov           w11, v1.s[1]
  0x0e1c3c2d,                                 //mov           w13, v1.s[3]
  0x8b2c492c,                                 //add           x12, x9, w12, uxtw #2
  0xbc6d5933,                                 //ldr           s19, [x9, w13, uxtw #2]
  0x0e0c3c6d,                                 //mov           w13, v3.s[1]
  0x8b2b4929,                                 //add           x9, x9, w11, uxtw #2
  0x0e143c6b,                                 //mov           w11, v3.s[2]
  0x0d408181,                                 //ld1           {v1.s}[0], [x12]
  0x0e1c3c6c,                                 //mov           w12, v3.s[3]
  0x0d409140,                                 //ld1           {v0.s}[1], [x10]
  0x1e26006a,                                 //fmov          w10, s3
  0xbd400c43,                                 //ldr           s3, [x2, #12]
  0x6f280442,                                 //ushr          v2.4s, v2.4s, #24
  0x4e21d842,                                 //scvtf         v2.4s, v2.4s
  0x8b2a490a,                                 //add           x10, x8, w10, uxtw #2
  0x4f839043,                                 //fmul          v3.4s, v2.4s, v3.s[0]
  0x0d408142,                                 //ld1           {v2.s}[0], [x10]
  0x8b2d490a,                                 //add           x10, x8, w13, uxtw #2
  0x6e140620,                                 //mov           v0.s[2], v17.s[0]
  0xbc6b5911,                                 //ldr           s17, [x8, w11, uxtw #2]
  0x0d409121,                                 //ld1           {v1.s}[1], [x9]
  0x0d409142,                                 //ld1           {v2.s}[1], [x10]
  0x6e1c0600,                                 //mov           v0.s[3], v16.s[0]
  0xbc6c5910,                                 //ldr           s16, [x8, w12, uxtw #2]
  0x6e140641,                                 //mov           v1.s[2], v18.s[0]
  0x6e140622,                                 //mov           v2.s[2], v17.s[0]
  0x6e1c0661,                                 //mov           v1.s[3], v19.s[0]
  0x6e1c0602,                                 //mov           v2.s[3], v16.s[0]
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_load_a8_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0xbd400c43,                                 //ldr           s3, [x2, #12]
  0x6f00e400,                                 //movi          v0.2d, #0x0
  0x6f00e401,                                 //movi          v1.2d, #0x0
  0xf9400108,                                 //ldr           x8, [x8]
  0x8b000108,                                 //add           x8, x8, x0
  0x39400109,                                 //ldrb          w9, [x8]
  0x3940050a,                                 //ldrb          w10, [x8, #1]
  0x3940090b,                                 //ldrb          w11, [x8, #2]
  0x39400d08,                                 //ldrb          w8, [x8, #3]
  0x4e021d22,                                 //mov           v2.h[0], w9
  0x4e061d42,                                 //mov           v2.h[1], w10
  0x4e0a1d62,                                 //mov           v2.h[2], w11
  0x4e0e1d02,                                 //mov           v2.h[3], w8
  0x2f07b7e2,                                 //bic           v2.4h, #0xff, lsl #8
  0x2f10a442,                                 //uxtl          v2.4s, v2.4h
  0x6e21d842,                                 //ucvtf         v2.4s, v2.4s
  0x4f839043,                                 //fmul          v3.4s, v2.4s, v3.s[0]
  0x6f00e402,                                 //movi          v2.2d, #0x0
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_store_a8_aarch64[] = {
  0xf9400028,                                 //ldr           x8, [x1]
  0xbd400850,                                 //ldr           s16, [x2, #8]
  0xf9400108,                                 //ldr           x8, [x8]
  0x4f909070,                                 //fmul          v16.4s, v3.4s, v16.s[0]
  0x6e21aa10,                                 //fcvtnu        v16.4s, v16.4s
  0x0e612a10,                                 //xtn           v16.4h, v16.4s
  0x0e0e3e09,                                 //umov          w9, v16.h[3]
  0x8b000108,                                 //add           x8, x8, x0
  0x39000d09,                                 //strb          w9, [x8, #3]
  0x0e0a3e09,                                 //umov          w9, v16.h[2]
  0x39000909,                                 //strb          w9, [x8, #2]
  0x0e063e09,                                 //umov          w9, v16.h[1]
  0x39000509,                                 //strb          w9, [x8, #1]
  0x0e023e09,                                 //umov          w9, v16.h[0]
  0x39000109,                                 //strb          w9, [x8]
  0xf9400423,                                 //ldr           x3, [x1, #8]
  0x91004021,                                 //add           x1, x1, #0x10
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_load_565_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0xd37ff809,                                 //lsl           x9, x0, #1
  0xf9400108,                                 //ldr           x8, [x8]
  0xfc696900,                                 //ldr           d0, [x8, x9]
  0x9101a048,                                 //add           x8, x2, #0x68
  0x4d40c901,                                 //ld1r          {v1.4s}, [x8]
  0x9101b048,                                 //add           x8, x2, #0x6c
  0x4d40c902,                                 //ld1r          {v2.4s}, [x8]
  0x9101c048,                                 //add           x8, x2, #0x70
  0x4d40c903,                                 //ld1r          {v3.4s}, [x8]
  0x2f10a400,                                 //uxtl          v0.4s, v0.4h
  0x4e201c21,                                 //and           v1.16b, v1.16b, v0.16b
  0x4e201c42,                                 //and           v2.16b, v2.16b, v0.16b
  0x4e201c71,                                 //and           v17.16b, v3.16b, v0.16b
  0x2d4e8c50,                                 //ldp           s16, s3, [x2, #116]
  0x4e21d820,                                 //scvtf         v0.4s, v1.4s
  0x4e21d841,                                 //scvtf         v1.4s, v2.4s
  0x4e21da22,                                 //scvtf         v2.4s, v17.4s
  0x4f909000,                                 //fmul          v0.4s, v0.4s, v16.s[0]
  0xbd407c50,                                 //ldr           s16, [x2, #124]
  0x4f839021,                                 //fmul          v1.4s, v1.4s, v3.s[0]
  0x4d40c843,                                 //ld1r          {v3.4s}, [x2]
  0x4f909042,                                 //fmul          v2.4s, v2.4s, v16.s[0]
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_store_565_aarch64[] = {
  0x2d504450,                                 //ldp           s16, s17, [x2, #128]
  0xf9400028,                                 //ldr           x8, [x1]
  0xd37ff809,                                 //lsl           x9, x0, #1
  0x4f909012,                                 //fmul          v18.4s, v0.4s, v16.s[0]
  0x4f919031,                                 //fmul          v17.4s, v1.4s, v17.s[0]
  0x6e21aa52,                                 //fcvtnu        v18.4s, v18.4s
  0x6e21aa31,                                 //fcvtnu        v17.4s, v17.4s
  0xf9400108,                                 //ldr           x8, [x8]
  0x4f909050,                                 //fmul          v16.4s, v2.4s, v16.s[0]
  0x4f2b5652,                                 //shl           v18.4s, v18.4s, #11
  0x4f255631,                                 //shl           v17.4s, v17.4s, #5
  0x4eb21e31,                                 //orr           v17.16b, v17.16b, v18.16b
  0x6e21aa10,                                 //fcvtnu        v16.4s, v16.4s
  0x4eb01e30,                                 //orr           v16.16b, v17.16b, v16.16b
  0x0e612a10,                                 //xtn           v16.4h, v16.4s
  0xfc296910,                                 //str           d16, [x8, x9]
  0xf9400423,                                 //ldr           x3, [x1, #8]
  0x91004021,                                 //add           x1, x1, #0x10
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_load_8888_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0xd37ef409,                                 //lsl           x9, x0, #2
  0xbd400c42,                                 //ldr           s2, [x2, #12]
  0xf9400108,                                 //ldr           x8, [x8]
  0x3ce96900,                                 //ldr           q0, [x8, x9]
  0x91004048,                                 //add           x8, x2, #0x10
  0x4d40c901,                                 //ld1r          {v1.4s}, [x8]
  0x6f380410,                                 //ushr          v16.4s, v0.4s, #8
  0x6f300411,                                 //ushr          v17.4s, v0.4s, #16
  0x4e201c23,                                 //and           v3.16b, v1.16b, v0.16b
  0x6f280400,                                 //ushr          v0.4s, v0.4s, #24
  0x4e301c30,                                 //and           v16.16b, v1.16b, v16.16b
  0x4e311c21,                                 //and           v1.16b, v1.16b, v17.16b
  0x4e21d863,                                 //scvtf         v3.4s, v3.4s
  0x4e21d811,                                 //scvtf         v17.4s, v0.4s
  0x4e21da10,                                 //scvtf         v16.4s, v16.4s
  0x4e21d832,                                 //scvtf         v18.4s, v1.4s
  0x4f829060,                                 //fmul          v0.4s, v3.4s, v2.s[0]
  0x4f829223,                                 //fmul          v3.4s, v17.4s, v2.s[0]
  0x4f829201,                                 //fmul          v1.4s, v16.4s, v2.s[0]
  0x4f829242,                                 //fmul          v2.4s, v18.4s, v2.s[0]
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_store_8888_aarch64[] = {
  0xbd400850,                                 //ldr           s16, [x2, #8]
  0xf9400028,                                 //ldr           x8, [x1]
  0xd37ef409,                                 //lsl           x9, x0, #2
  0x4f909032,                                 //fmul          v18.4s, v1.4s, v16.s[0]
  0x4f909011,                                 //fmul          v17.4s, v0.4s, v16.s[0]
  0x6e21aa52,                                 //fcvtnu        v18.4s, v18.4s
  0x6e21aa31,                                 //fcvtnu        v17.4s, v17.4s
  0x4f285652,                                 //shl           v18.4s, v18.4s, #8
  0x4eb11e51,                                 //orr           v17.16b, v18.16b, v17.16b
  0x4f909052,                                 //fmul          v18.4s, v2.4s, v16.s[0]
  0xf9400108,                                 //ldr           x8, [x8]
  0x4f909070,                                 //fmul          v16.4s, v3.4s, v16.s[0]
  0x6e21aa52,                                 //fcvtnu        v18.4s, v18.4s
  0x6e21aa10,                                 //fcvtnu        v16.4s, v16.4s
  0x4f305652,                                 //shl           v18.4s, v18.4s, #16
  0x4eb21e31,                                 //orr           v17.16b, v17.16b, v18.16b
  0x4f385610,                                 //shl           v16.4s, v16.4s, #24
  0x4eb01e30,                                 //orr           v16.16b, v17.16b, v16.16b
  0x3ca96910,                                 //str           q16, [x8, x9]
  0xf9400423,                                 //ldr           x3, [x1, #8]
  0x91004021,                                 //add           x1, x1, #0x10
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_load_f16_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0xf9400108,                                 //ldr           x8, [x8]
  0x8b000d08,                                 //add           x8, x8, x0, lsl #3
  0x0c400510,                                 //ld4           {v16.4h-v19.4h}, [x8]
  0x0e217a00,                                 //fcvtl         v0.4s, v16.4h
  0x0e217a21,                                 //fcvtl         v1.4s, v17.4h
  0x0e217a42,                                 //fcvtl         v2.4s, v18.4h
  0x0e217a63,                                 //fcvtl         v3.4s, v19.4h
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_store_f16_aarch64[] = {
  0xf9400028,                                 //ldr           x8, [x1]
  0x0e216810,                                 //fcvtn         v16.4h, v0.4s
  0x0e216831,                                 //fcvtn         v17.4h, v1.4s
  0x0e216852,                                 //fcvtn         v18.4h, v2.4s
  0xf9400108,                                 //ldr           x8, [x8]
  0x0e216873,                                 //fcvtn         v19.4h, v3.4s
  0x8b000d08,                                 //add           x8, x8, x0, lsl #3
  0x0c000510,                                 //st4           {v16.4h-v19.4h}, [x8]
  0xf9400423,                                 //ldr           x3, [x1, #8]
  0x91004021,                                 //add           x1, x1, #0x10
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_store_f32_aarch64[] = {
  0xf9400028,                                 //ldr           x8, [x1]
  0xf9400108,                                 //ldr           x8, [x8]
  0x8b001108,                                 //add           x8, x8, x0, lsl #4
  0x4c000900,                                 //st4           {v0.4s-v3.4s}, [x8]
  0xf9400423,                                 //ldr           x3, [x1, #8]
  0x91004021,                                 //add           x1, x1, #0x10
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_clamp_x_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0x6f00e411,                                 //movi          v17.2d, #0x0
  0x4e20f620,                                 //fmax          v0.4s, v17.4s, v0.4s
  0x6f07e7f1,                                 //movi          v17.2d, #0xffffffffffffffff
  0x4d40c910,                                 //ld1r          {v16.4s}, [x8]
  0x4eb18610,                                 //add           v16.4s, v16.4s, v17.4s
  0x4eb0f400,                                 //fmin          v0.4s, v0.4s, v16.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_clamp_y_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0x6f00e411,                                 //movi          v17.2d, #0x0
  0x4e21f621,                                 //fmax          v1.4s, v17.4s, v1.4s
  0x6f07e7f1,                                 //movi          v17.2d, #0xffffffffffffffff
  0x4d40c910,                                 //ld1r          {v16.4s}, [x8]
  0x4eb18610,                                 //add           v16.4s, v16.4s, v17.4s
  0x4eb0f421,                                 //fmin          v1.4s, v1.4s, v16.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_repeat_x_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0x6f07e7f1,                                 //movi          v17.2d, #0xffffffffffffffff
  0xbd400110,                                 //ldr           s16, [x8]
  0x4e040612,                                 //dup           v18.4s, v16.s[0]
  0x4eb18651,                                 //add           v17.4s, v18.4s, v17.4s
  0x6e32fc12,                                 //fdiv          v18.4s, v0.4s, v18.4s
  0x4e219a52,                                 //frintm        v18.4s, v18.4s
  0x4f905240,                                 //fmls          v0.4s, v18.4s, v16.s[0]
  0x4eb1f400,                                 //fmin          v0.4s, v0.4s, v17.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_repeat_y_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0x6f07e7f1,                                 //movi          v17.2d, #0xffffffffffffffff
  0xbd400110,                                 //ldr           s16, [x8]
  0x4e040612,                                 //dup           v18.4s, v16.s[0]
  0x4eb18651,                                 //add           v17.4s, v18.4s, v17.4s
  0x6e32fc32,                                 //fdiv          v18.4s, v1.4s, v18.4s
  0x4e219a52,                                 //frintm        v18.4s, v18.4s
  0x4f905241,                                 //fmls          v1.4s, v18.4s, v16.s[0]
  0x4eb1f421,                                 //fmin          v1.4s, v1.4s, v17.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_mirror_x_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0xbd400110,                                 //ldr           s16, [x8]
  0x4e040611,                                 //dup           v17.4s, v16.s[0]
  0x1e302a10,                                 //fadd          s16, s16, s16
  0x4eb1d400,                                 //fsub          v0.4s, v0.4s, v17.4s
  0x4e040612,                                 //dup           v18.4s, v16.s[0]
  0x6e32fc12,                                 //fdiv          v18.4s, v0.4s, v18.4s
  0x4e219a52,                                 //frintm        v18.4s, v18.4s
  0x4f905240,                                 //fmls          v0.4s, v18.4s, v16.s[0]
  0x6f07e7f0,                                 //movi          v16.2d, #0xffffffffffffffff
  0x4eb1d400,                                 //fsub          v0.4s, v0.4s, v17.4s
  0x4eb08630,                                 //add           v16.4s, v17.4s, v16.4s
  0x4ea0f800,                                 //fabs          v0.4s, v0.4s
  0x4eb0f400,                                 //fmin          v0.4s, v0.4s, v16.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_mirror_y_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0xbd400110,                                 //ldr           s16, [x8]
  0x4e040611,                                 //dup           v17.4s, v16.s[0]
  0x1e302a10,                                 //fadd          s16, s16, s16
  0x4eb1d421,                                 //fsub          v1.4s, v1.4s, v17.4s
  0x4e040612,                                 //dup           v18.4s, v16.s[0]
  0x6e32fc32,                                 //fdiv          v18.4s, v1.4s, v18.4s
  0x4e219a52,                                 //frintm        v18.4s, v18.4s
  0x4f905241,                                 //fmls          v1.4s, v18.4s, v16.s[0]
  0x6f07e7f0,                                 //movi          v16.2d, #0xffffffffffffffff
  0x4eb1d421,                                 //fsub          v1.4s, v1.4s, v17.4s
  0x4eb08630,                                 //add           v16.4s, v17.4s, v16.4s
  0x4ea0f821,                                 //fabs          v1.4s, v1.4s
  0x4eb0f421,                                 //fmin          v1.4s, v1.4s, v16.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_matrix_2x3_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0xaa0803e9,                                 //mov           x9, x8
  0x9100410a,                                 //add           x10, x8, #0x10
  0x4ddfc932,                                 //ld1r          {v18.4s}, [x9], #4
  0x4d40c950,                                 //ld1r          {v16.4s}, [x10]
  0x2d415113,                                 //ldp           s19, s20, [x8, #8]
  0x9100510a,                                 //add           x10, x8, #0x14
  0x4d40c951,                                 //ld1r          {v17.4s}, [x10]
  0x4f931030,                                 //fmla          v16.4s, v1.4s, v19.s[0]
  0xbd400133,                                 //ldr           s19, [x9]
  0x4f941031,                                 //fmla          v17.4s, v1.4s, v20.s[0]
  0x4e20ce50,                                 //fmla          v16.4s, v18.4s, v0.4s
  0x4f931011,                                 //fmla          v17.4s, v0.4s, v19.s[0]
  0x4eb01e00,                                 //mov           v0.16b, v16.16b
  0x4eb11e21,                                 //mov           v1.16b, v17.16b
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_matrix_3x4_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0xaa0803e9,                                 //mov           x9, x8
  0x9100910a,                                 //add           x10, x8, #0x24
  0x4ddfc933,                                 //ld1r          {v19.4s}, [x9], #4
  0x4d40c950,                                 //ld1r          {v16.4s}, [x10]
  0x9100a10a,                                 //add           x10, x8, #0x28
  0x4d40c951,                                 //ld1r          {v17.4s}, [x10]
  0x9100b10a,                                 //add           x10, x8, #0x2c
  0x2d435514,                                 //ldp           s20, s21, [x8, #24]
  0xbd402116,                                 //ldr           s22, [x8, #32]
  0x4d40c952,                                 //ld1r          {v18.4s}, [x10]
  0x4f941050,                                 //fmla          v16.4s, v2.4s, v20.s[0]
  0x4f951051,                                 //fmla          v17.4s, v2.4s, v21.s[0]
  0x4f961052,                                 //fmla          v18.4s, v2.4s, v22.s[0]
  0x2d425502,                                 //ldp           s2, s21, [x8, #16]
  0x2d415d14,                                 //ldp           s20, s23, [x8, #8]
  0x4f821031,                                 //fmla          v17.4s, v1.4s, v2.s[0]
  0xbd400122,                                 //ldr           s2, [x9]
  0x4f971030,                                 //fmla          v16.4s, v1.4s, v23.s[0]
  0x4f951032,                                 //fmla          v18.4s, v1.4s, v21.s[0]
  0x4e20ce70,                                 //fmla          v16.4s, v19.4s, v0.4s
  0x4f941012,                                 //fmla          v18.4s, v0.4s, v20.s[0]
  0x4f821011,                                 //fmla          v17.4s, v0.4s, v2.s[0]
  0x4eb01e00,                                 //mov           v0.16b, v16.16b
  0x4eb11e21,                                 //mov           v1.16b, v17.16b
  0x4eb21e42,                                 //mov           v2.16b, v18.16b
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_matrix_perspective_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0xaa0803e9,                                 //mov           x9, x8
  0x9100510a,                                 //add           x10, x8, #0x14
  0x4ddfc930,                                 //ld1r          {v16.4s}, [x9], #4
  0x4d40c951,                                 //ld1r          {v17.4s}, [x10]
  0x9100810a,                                 //add           x10, x8, #0x20
  0x4d40c952,                                 //ld1r          {v18.4s}, [x10]
  0x2d41d113,                                 //ldp           s19, s20, [x8, #12]
  0x2d435915,                                 //ldp           s21, s22, [x8, #24]
  0x91002108,                                 //add           x8, x8, #0x8
  0x4f941031,                                 //fmla          v17.4s, v1.4s, v20.s[0]
  0x4d40c914,                                 //ld1r          {v20.4s}, [x8]
  0x4f961032,                                 //fmla          v18.4s, v1.4s, v22.s[0]
  0xbd400136,                                 //ldr           s22, [x9]
  0x4f951012,                                 //fmla          v18.4s, v0.4s, v21.s[0]
  0x4f931011,                                 //fmla          v17.4s, v0.4s, v19.s[0]
  0x4f961034,                                 //fmla          v20.4s, v1.4s, v22.s[0]
  0x4ea1da41,                                 //frecpe        v1.4s, v18.4s
  0x4e21fe52,                                 //frecps        v18.4s, v18.4s, v1.4s
  0x6e32dc32,                                 //fmul          v18.4s, v1.4s, v18.4s
  0x4e20ce14,                                 //fmla          v20.4s, v16.4s, v0.4s
  0x6e32de21,                                 //fmul          v1.4s, v17.4s, v18.4s
  0x6e32de80,                                 //fmul          v0.4s, v20.4s, v18.4s
  0xd61f0060,                                 //br            x3
};

CODE const uint32_t sk_linear_gradient_2stops_aarch64[] = {
  0xa8c10c28,                                 //ldp           x8, x3, [x1], #16
  0xad404503,                                 //ldp           q3, q17, [x8]
  0x4e040470,                                 //dup           v16.4s, v3.s[0]
  0x4e0c0461,                                 //dup           v1.4s, v3.s[1]
  0x4e140462,                                 //dup           v2.4s, v3.s[2]
  0x4e1c0463,                                 //dup           v3.4s, v3.s[3]
  0x4f911010,                                 //fmla          v16.4s, v0.4s, v17.s[0]
  0x4fb11001,                                 //fmla          v1.4s, v0.4s, v17.s[1]
  0x4f911802,                                 //fmla          v2.4s, v0.4s, v17.s[2]
  0x4fb11803,                                 //fmla          v3.4s, v0.4s, v17.s[3]
  0x4eb01e00,                                 //mov           v0.16b, v16.16b
  0xd61f0060,                                 //br            x3
};
#elif defined(__arm__)

CODE const uint32_t sk_start_pipeline_vfp4[] = {
  0xe92d41f0,                                 //push          {r4, r5, r6, r7, r8, lr}
  0xe1a07001,                                 //mov           r7, r1
  0xe1a04000,                                 //mov           r4, r0
  0xe1a05003,                                 //mov           r5, r3
  0xe1a08002,                                 //mov           r8, r2
  0xe4976004,                                 //ldr           r6, [r7], #4
  0xe2840002,                                 //add           r0, r4, #2
  0xea00000d,                                 //b             58 <sk_start_pipeline_vfp4+0x58>
  0xf2800010,                                 //vmov.i32      d0, #0
  0xe1a00004,                                 //mov           r0, r4
  0xf2801010,                                 //vmov.i32      d1, #0
  0xe1a01007,                                 //mov           r1, r7
  0xf2802010,                                 //vmov.i32      d2, #0
  0xe1a02008,                                 //mov           r2, r8
  0xf2803010,                                 //vmov.i32      d3, #0
  0xf2804010,                                 //vmov.i32      d4, #0
  0xf2805010,                                 //vmov.i32      d5, #0
  0xf2806010,                                 //vmov.i32      d6, #0
  0xf2807010,                                 //vmov.i32      d7, #0
  0xe12fff36,                                 //blx           r6
  0xe2840004,                                 //add           r0, r4, #4
  0xe2844002,                                 //add           r4, r4, #2
  0xe1500005,                                 //cmp           r0, r5
  0x9affffef,                                 //bls           20 <sk_start_pipeline_vfp4+0x20>
  0xe1a00004,                                 //mov           r0, r4
  0xe8bd81f0,                                 //pop           {r4, r5, r6, r7, r8, pc}
};

CODE const uint32_t sk_just_return_vfp4[] = {
  0xe12fff1e,                                 //bx            lr
};

CODE const uint32_t sk_seed_shader_vfp4[] = {
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xee800b90,                                 //vdup.32       d16, r0
  0xf3fb0620,                                 //vcvt.f32.s32  d16, d16
  0xedd23b05,                                 //vldr          d19, [r2, #20]
  0xf2803010,                                 //vmov.i32      d3, #0
  0xf4e31c9f,                                 //vld1.32       {d17[]}, [r3 :32]
  0xe2823004,                                 //add           r3, r2, #4
  0xf3fb1621,                                 //vcvt.f32.s32  d17, d17
  0xe2811008,                                 //add           r1, r1, #8
  0xf4e32c9f,                                 //vld1.32       {d18[]}, [r3 :32]
  0xf2804010,                                 //vmov.i32      d4, #0
  0xf2400da2,                                 //vadd.f32      d16, d16, d18
  0xf2805010,                                 //vmov.i32      d5, #0
  0xf4a22c9f,                                 //vld1.32       {d2[]}, [r2 :32]
  0xf2011da2,                                 //vadd.f32      d1, d17, d18
  0xf2806010,                                 //vmov.i32      d6, #0
  0xf2030da0,                                 //vadd.f32      d0, d19, d16
  0xf2807010,                                 //vmov.i32      d7, #0
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_constant_color_vfp4[] = {
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xe2811008,                                 //add           r1, r1, #8
  0xf4630a0f,                                 //vld1.8        {d16-d17}, [r3]
  0xf3b40c20,                                 //vdup.32       d0, d16[0]
  0xf3bc1c20,                                 //vdup.32       d1, d16[1]
  0xf3b42c21,                                 //vdup.32       d2, d17[0]
  0xf3bc3c21,                                 //vdup.32       d3, d17[1]
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_clear_vfp4[] = {
  0xe4913004,                                 //ldr           r3, [r1], #4
  0xf2800010,                                 //vmov.i32      d0, #0
  0xf2801010,                                 //vmov.i32      d1, #0
  0xf2802010,                                 //vmov.i32      d2, #0
  0xf2803010,                                 //vmov.i32      d3, #0
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_plus__vfp4[] = {
  0xf2000d04,                                 //vadd.f32      d0, d0, d4
  0xe4913004,                                 //ldr           r3, [r1], #4
  0xf2011d05,                                 //vadd.f32      d1, d1, d5
  0xf2022d06,                                 //vadd.f32      d2, d2, d6
  0xf2033d07,                                 //vadd.f32      d3, d3, d7
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_srcover_vfp4[] = {
  0xf4e20c9f,                                 //vld1.32       {d16[]}, [r2 :32]
  0xe4913004,                                 //ldr           r3, [r1], #4
  0xf2600d83,                                 //vsub.f32      d16, d16, d3
  0xf2040c30,                                 //vfma.f32      d0, d4, d16
  0xf2051c30,                                 //vfma.f32      d1, d5, d16
  0xf2062c30,                                 //vfma.f32      d2, d6, d16
  0xf2073c30,                                 //vfma.f32      d3, d7, d16
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_dstover_vfp4[] = {
  0xf4e20c9f,                                 //vld1.32       {d16[]}, [r2 :32]
  0xf2651115,                                 //vorr          d17, d5, d5
  0xf2604d87,                                 //vsub.f32      d20, d16, d7
  0xf2640114,                                 //vorr          d16, d4, d4
  0xf2662116,                                 //vorr          d18, d6, d6
  0xe4913004,                                 //ldr           r3, [r1], #4
  0xf2673117,                                 //vorr          d19, d7, d7
  0xf2400c34,                                 //vfma.f32      d16, d0, d20
  0xf2411c34,                                 //vfma.f32      d17, d1, d20
  0xf2422c34,                                 //vfma.f32      d18, d2, d20
  0xf2433c34,                                 //vfma.f32      d19, d3, d20
  0xf22001b0,                                 //vorr          d0, d16, d16
  0xf22111b1,                                 //vorr          d1, d17, d17
  0xf22221b2,                                 //vorr          d2, d18, d18
  0xf22331b3,                                 //vorr          d3, d19, d19
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_clamp_0_vfp4[] = {
  0xf2c00010,                                 //vmov.i32      d16, #0
  0xe4913004,                                 //ldr           r3, [r1], #4
  0xf2000f20,                                 //vmax.f32      d0, d0, d16
  0xf2011f20,                                 //vmax.f32      d1, d1, d16
  0xf2022f20,                                 //vmax.f32      d2, d2, d16
  0xf2033f20,                                 //vmax.f32      d3, d3, d16
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_clamp_1_vfp4[] = {
  0xf4e20c9f,                                 //vld1.32       {d16[]}, [r2 :32]
  0xe4913004,                                 //ldr           r3, [r1], #4
  0xf2200f20,                                 //vmin.f32      d0, d0, d16
  0xf2211f20,                                 //vmin.f32      d1, d1, d16
  0xf2222f20,                                 //vmin.f32      d2, d2, d16
  0xf2233f20,                                 //vmin.f32      d3, d3, d16
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_clamp_a_vfp4[] = {
  0xf4e20c9f,                                 //vld1.32       {d16[]}, [r2 :32]
  0xe4913004,                                 //ldr           r3, [r1], #4
  0xf2233f20,                                 //vmin.f32      d3, d3, d16
  0xf2200f03,                                 //vmin.f32      d0, d0, d3
  0xf2211f03,                                 //vmin.f32      d1, d1, d3
  0xf2222f03,                                 //vmin.f32      d2, d2, d3
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_set_rgb_vfp4[] = {
  0xe92d4800,                                 //push          {fp, lr}
  0xe591e000,                                 //ldr           lr, [r1]
  0xe591c004,                                 //ldr           ip, [r1, #4]
  0xe2811008,                                 //add           r1, r1, #8
  0xe28e3008,                                 //add           r3, lr, #8
  0xf4ae0c9f,                                 //vld1.32       {d0[]}, [lr :32]
  0xf4a32c9f,                                 //vld1.32       {d2[]}, [r3 :32]
  0xe28e3004,                                 //add           r3, lr, #4
  0xf4a31c9f,                                 //vld1.32       {d1[]}, [r3 :32]
  0xe8bd4800,                                 //pop           {fp, lr}
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_swap_rb_vfp4[] = {
  0xeef00b40,                                 //vmov.f64      d16, d0
  0xe4913004,                                 //ldr           r3, [r1], #4
  0xeeb00b42,                                 //vmov.f64      d0, d2
  0xeeb02b60,                                 //vmov.f64      d2, d16
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_swap_vfp4[] = {
  0xeef00b43,                                 //vmov.f64      d16, d3
  0xe4913004,                                 //ldr           r3, [r1], #4
  0xeef01b42,                                 //vmov.f64      d17, d2
  0xeef02b41,                                 //vmov.f64      d18, d1
  0xeef03b40,                                 //vmov.f64      d19, d0
  0xeeb00b44,                                 //vmov.f64      d0, d4
  0xeeb01b45,                                 //vmov.f64      d1, d5
  0xeeb02b46,                                 //vmov.f64      d2, d6
  0xeeb03b47,                                 //vmov.f64      d3, d7
  0xeeb04b63,                                 //vmov.f64      d4, d19
  0xeeb05b62,                                 //vmov.f64      d5, d18
  0xeeb06b61,                                 //vmov.f64      d6, d17
  0xeeb07b60,                                 //vmov.f64      d7, d16
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_move_src_dst_vfp4[] = {
  0xeeb04b40,                                 //vmov.f64      d4, d0
  0xe4913004,                                 //ldr           r3, [r1], #4
  0xeeb05b41,                                 //vmov.f64      d5, d1
  0xeeb06b42,                                 //vmov.f64      d6, d2
  0xeeb07b43,                                 //vmov.f64      d7, d3
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_move_dst_src_vfp4[] = {
  0xeeb00b44,                                 //vmov.f64      d0, d4
  0xe4913004,                                 //ldr           r3, [r1], #4
  0xeeb01b45,                                 //vmov.f64      d1, d5
  0xeeb02b46,                                 //vmov.f64      d2, d6
  0xeeb03b47,                                 //vmov.f64      d3, d7
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_premul_vfp4[] = {
  0xf3000d13,                                 //vmul.f32      d0, d0, d3
  0xe4913004,                                 //ldr           r3, [r1], #4
  0xf3011d13,                                 //vmul.f32      d1, d1, d3
  0xf3022d13,                                 //vmul.f32      d2, d2, d3
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_unpremul_vfp4[] = {
  0xed2d8b04,                                 //vpush         {d8-d9}
  0xed928a00,                                 //vldr          s16, [r2]
  0xf2c00010,                                 //vmov.i32      d16, #0
  0xf3f91503,                                 //vceq.f32      d17, d3, #0
  0xe4913004,                                 //ldr           r3, [r1], #4
  0xeec89a23,                                 //vdiv.f32      s19, s16, s7
  0xee889a03,                                 //vdiv.f32      s18, s16, s6
  0xf3501199,                                 //vbsl          d17, d16, d9
  0xf3010d90,                                 //vmul.f32      d0, d17, d0
  0xf3011d91,                                 //vmul.f32      d1, d17, d1
  0xf3012d92,                                 //vmul.f32      d2, d17, d2
  0xecbd8b04,                                 //vpop          {d8-d9}
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_from_srgb_vfp4[] = {
  0xed2d8b02,                                 //vpush         {d8}
  0xe282303c,                                 //add           r3, r2, #60
  0xed928a10,                                 //vldr          s16, [r2, #64]
  0xf3402d10,                                 //vmul.f32      d18, d0, d0
  0xf4e30c9f,                                 //vld1.32       {d16[]}, [r3 :32]
  0xe2823038,                                 //add           r3, r2, #56
  0xf3413d11,                                 //vmul.f32      d19, d1, d1
  0xf4e31c9f,                                 //vld1.32       {d17[]}, [r3 :32]
  0xe2823044,                                 //add           r3, r2, #68
  0xf26141b1,                                 //vorr          d20, d17, d17
  0xf26171b1,                                 //vorr          d23, d17, d17
  0xf4e38c9f,                                 //vld1.32       {d24[]}, [r3 :32]
  0xf2404c30,                                 //vfma.f32      d20, d0, d16
  0xe2823034,                                 //add           r3, r2, #52
  0xf2417c30,                                 //vfma.f32      d23, d1, d16
  0xf2421c30,                                 //vfma.f32      d17, d2, d16
  0xf3425d12,                                 //vmul.f32      d21, d2, d2
  0xf2e16948,                                 //vmul.f32      d22, d1, d8[0]
  0xf2e00948,                                 //vmul.f32      d16, d0, d8[0]
  0xf2e29948,                                 //vmul.f32      d25, d2, d8[0]
  0xf3282e82,                                 //vcgt.f32      d2, d24, d2
  0xf3281e81,                                 //vcgt.f32      d1, d24, d1
  0xf3280e80,                                 //vcgt.f32      d0, d24, d0
  0xf4e38c9f,                                 //vld1.32       {d24[]}, [r3 :32]
  0xf268a1b8,                                 //vorr          d26, d24, d24
  0xf242acb4,                                 //vfma.f32      d26, d18, d20
  0xf26821b8,                                 //vorr          d18, d24, d24
  0xe4913004,                                 //ldr           r3, [r1], #4
  0xf2432cb7,                                 //vfma.f32      d18, d19, d23
  0xf2458cb1,                                 //vfma.f32      d24, d21, d17
  0xf31001ba,                                 //vbsl          d0, d16, d26
  0xf31611b2,                                 //vbsl          d1, d22, d18
  0xf31921b8,                                 //vbsl          d2, d25, d24
  0xecbd8b02,                                 //vpop          {d8}
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_to_srgb_vfp4[] = {
  0xed2d8b02,                                 //vpush         {d8}
  0xf3fb0580,                                 //vrsqrte.f32   d16, d0
  0xe2823050,                                 //add           r3, r2, #80
  0xf3fb1581,                                 //vrsqrte.f32   d17, d1
  0xed928a12,                                 //vldr          s16, [r2, #72]
  0xf3fb2582,                                 //vrsqrte.f32   d18, d2
  0xf3403db0,                                 //vmul.f32      d19, d16, d16
  0xf3414db1,                                 //vmul.f32      d20, d17, d17
  0xf3425db2,                                 //vmul.f32      d21, d18, d18
  0xf2603f33,                                 //vrsqrts.f32   d19, d0, d19
  0xf2614f34,                                 //vrsqrts.f32   d20, d1, d20
  0xf2625f35,                                 //vrsqrts.f32   d21, d2, d21
  0xf3400db3,                                 //vmul.f32      d16, d16, d19
  0xf3411db4,                                 //vmul.f32      d17, d17, d20
  0xf3422db5,                                 //vmul.f32      d18, d18, d21
  0xf3fb3520,                                 //vrecpe.f32    d19, d16
  0xf3fb4521,                                 //vrecpe.f32    d20, d17
  0xf3fb6522,                                 //vrecpe.f32    d22, d18
  0xf3fb55a2,                                 //vrsqrte.f32   d21, d18
  0xf3fb75a0,                                 //vrsqrte.f32   d23, d16
  0xf3fb85a1,                                 //vrsqrte.f32   d24, d17
  0xf2409fb3,                                 //vrecps.f32    d25, d16, d19
  0xf241afb4,                                 //vrecps.f32    d26, d17, d20
  0xf242bfb6,                                 //vrecps.f32    d27, d18, d22
  0xf345cdb5,                                 //vmul.f32      d28, d21, d21
  0xf347ddb7,                                 //vmul.f32      d29, d23, d23
  0xf348edb8,                                 //vmul.f32      d30, d24, d24
  0xf2622fbc,                                 //vrsqrts.f32   d18, d18, d28
  0xf2600fbd,                                 //vrsqrts.f32   d16, d16, d29
  0xf2611fbe,                                 //vrsqrts.f32   d17, d17, d30
  0xf3433db9,                                 //vmul.f32      d19, d19, d25
  0xf4e39c9f,                                 //vld1.32       {d25[]}, [r3 :32]
  0xe2823054,                                 //add           r3, r2, #84
  0xf3444dba,                                 //vmul.f32      d20, d20, d26
  0xf3466dbb,                                 //vmul.f32      d22, d22, d27
  0xf4e3ac9f,                                 //vld1.32       {d26[]}, [r3 :32]
  0xe282304c,                                 //add           r3, r2, #76
  0xf26ab1ba,                                 //vorr          d27, d26, d26
  0xf249bcb3,                                 //vfma.f32      d27, d25, d19
  0xf26a31ba,                                 //vorr          d19, d26, d26
  0xf2493cb4,                                 //vfma.f32      d19, d25, d20
  0xf4e34c9f,                                 //vld1.32       {d20[]}, [r3 :32]
  0xf249acb6,                                 //vfma.f32      d26, d25, d22
  0xe2823058,                                 //add           r3, r2, #88
  0xf3452db2,                                 //vmul.f32      d18, d21, d18
  0xf3470db0,                                 //vmul.f32      d16, d23, d16
  0xf3481db1,                                 //vmul.f32      d17, d24, d17
  0xf2e05948,                                 //vmul.f32      d21, d0, d8[0]
  0xf244bcb0,                                 //vfma.f32      d27, d20, d16
  0xf4e30c9f,                                 //vld1.32       {d16[]}, [r3 :32]
  0xf2443cb1,                                 //vfma.f32      d19, d20, d17
  0xf244acb2,                                 //vfma.f32      d26, d20, d18
  0xf4e24c9f,                                 //vld1.32       {d20[]}, [r2 :32]
  0xf2e11948,                                 //vmul.f32      d17, d1, d8[0]
  0xf2e22948,                                 //vmul.f32      d18, d2, d8[0]
  0xf3201e81,                                 //vcgt.f32      d1, d16, d1
  0xe4913004,                                 //ldr           r3, [r1], #4
  0xf3200e80,                                 //vcgt.f32      d0, d16, d0
  0xf3202e82,                                 //vcgt.f32      d2, d16, d2
  0xf2640fab,                                 //vmin.f32      d16, d20, d27
  0xf2643fa3,                                 //vmin.f32      d19, d20, d19
  0xf2644faa,                                 //vmin.f32      d20, d20, d26
  0xf31501b0,                                 //vbsl          d0, d21, d16
  0xf31111b3,                                 //vbsl          d1, d17, d19
  0xf31221b4,                                 //vbsl          d2, d18, d20
  0xecbd8b02,                                 //vpop          {d8}
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_scale_1_float_vfp4[] = {
  0xed2d8b02,                                 //vpush         {d8}
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xe2811008,                                 //add           r1, r1, #8
  0xed938a00,                                 //vldr          s16, [r3]
  0xf2a00948,                                 //vmul.f32      d0, d0, d8[0]
  0xf2a11948,                                 //vmul.f32      d1, d1, d8[0]
  0xf2a22948,                                 //vmul.f32      d2, d2, d8[0]
  0xf2a33948,                                 //vmul.f32      d3, d3, d8[0]
  0xecbd8b02,                                 //vpop          {d8}
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_scale_u8_vfp4[] = {
  0xed2d8b02,                                 //vpush         {d8}
  0xe24dd008,                                 //sub           sp, sp, #8
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xe2811008,                                 //add           r1, r1, #8
  0xe5933000,                                 //ldr           r3, [r3]
  0xe0833000,                                 //add           r3, r3, r0
  0xe1d330b0,                                 //ldrh          r3, [r3]
  0xe1cd30b4,                                 //strh          r3, [sp, #4]
  0xe28d3004,                                 //add           r3, sp, #4
  0xed928a03,                                 //vldr          s16, [r2, #12]
  0xf4e3041f,                                 //vld1.16       {d16[0]}, [r3 :16]
  0xf3c80a30,                                 //vmovl.u8      q8, d16
  0xf3d00a30,                                 //vmovl.u16     q8, d16
  0xf3fb06a0,                                 //vcvt.f32.u32  d16, d16
  0xf2e009c8,                                 //vmul.f32      d16, d16, d8[0]
  0xf3000d90,                                 //vmul.f32      d0, d16, d0
  0xf3001d91,                                 //vmul.f32      d1, d16, d1
  0xf3002d92,                                 //vmul.f32      d2, d16, d2
  0xf3003d93,                                 //vmul.f32      d3, d16, d3
  0xe28dd008,                                 //add           sp, sp, #8
  0xecbd8b02,                                 //vpop          {d8}
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_lerp_1_float_vfp4[] = {
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xf2600d04,                                 //vsub.f32      d16, d0, d4
  0xf2611d05,                                 //vsub.f32      d17, d1, d5
  0xf2622d06,                                 //vsub.f32      d18, d2, d6
  0xe2811008,                                 //add           r1, r1, #8
  0xf2633d07,                                 //vsub.f32      d19, d3, d7
  0xf4e34c9f,                                 //vld1.32       {d20[]}, [r3 :32]
  0xf2240114,                                 //vorr          d0, d4, d4
  0xf2251115,                                 //vorr          d1, d5, d5
  0xf2262116,                                 //vorr          d2, d6, d6
  0xf2273117,                                 //vorr          d3, d7, d7
  0xf2000cb4,                                 //vfma.f32      d0, d16, d20
  0xf2011cb4,                                 //vfma.f32      d1, d17, d20
  0xf2022cb4,                                 //vfma.f32      d2, d18, d20
  0xf2033cb4,                                 //vfma.f32      d3, d19, d20
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_lerp_u8_vfp4[] = {
  0xed2d8b02,                                 //vpush         {d8}
  0xe24dd008,                                 //sub           sp, sp, #8
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xf2612d05,                                 //vsub.f32      d18, d1, d5
  0xf2623d06,                                 //vsub.f32      d19, d2, d6
  0xf2634d07,                                 //vsub.f32      d20, d3, d7
  0xe2811008,                                 //add           r1, r1, #8
  0xe5933000,                                 //ldr           r3, [r3]
  0xf2251115,                                 //vorr          d1, d5, d5
  0xf2262116,                                 //vorr          d2, d6, d6
  0xe0833000,                                 //add           r3, r3, r0
  0xf2273117,                                 //vorr          d3, d7, d7
  0xe1d330b0,                                 //ldrh          r3, [r3]
  0xe1cd30b4,                                 //strh          r3, [sp, #4]
  0xe28d3004,                                 //add           r3, sp, #4
  0xed928a03,                                 //vldr          s16, [r2, #12]
  0xf4e3041f,                                 //vld1.16       {d16[0]}, [r3 :16]
  0xf3c80a30,                                 //vmovl.u8      q8, d16
  0xf3d00a30,                                 //vmovl.u16     q8, d16
  0xf3fb06a0,                                 //vcvt.f32.u32  d16, d16
  0xf2601d04,                                 //vsub.f32      d17, d0, d4
  0xf2240114,                                 //vorr          d0, d4, d4
  0xf2e009c8,                                 //vmul.f32      d16, d16, d8[0]
  0xf2010cb0,                                 //vfma.f32      d0, d17, d16
  0xf2021cb0,                                 //vfma.f32      d1, d18, d16
  0xf2032cb0,                                 //vfma.f32      d2, d19, d16
  0xf2043cb0,                                 //vfma.f32      d3, d20, d16
  0xe28dd008,                                 //add           sp, sp, #8
  0xecbd8b02,                                 //vpop          {d8}
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_lerp_565_vfp4[] = {
  0xed2d8b04,                                 //vpush         {d8-d9}
  0xe24dd008,                                 //sub           sp, sp, #8
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xf2603d04,                                 //vsub.f32      d19, d0, d4
  0xf2240114,                                 //vorr          d0, d4, d4
  0xe2811008,                                 //add           r1, r1, #8
  0xe5933000,                                 //ldr           r3, [r3]
  0xe7933080,                                 //ldr           r3, [r3, r0, lsl #1]
  0xe58d3004,                                 //str           r3, [sp, #4]
  0xe28d3004,                                 //add           r3, sp, #4
  0xed923a1d,                                 //vldr          s6, [r2, #116]
  0xf4e3083f,                                 //vld1.32       {d16[0]}, [r3 :32]
  0xe282306c,                                 //add           r3, r2, #108
  0xf4e31c9f,                                 //vld1.32       {d17[]}, [r3 :32]
  0xe2823068,                                 //add           r3, r2, #104
  0xf3d04a30,                                 //vmovl.u16     q10, d16
  0xf4e32c9f,                                 //vld1.32       {d18[]}, [r3 :32]
  0xe2823070,                                 //add           r3, r2, #112
  0xf24201b4,                                 //vand          d16, d18, d20
  0xf4e32c9f,                                 //vld1.32       {d18[]}, [r3 :32]
  0xf24221b4,                                 //vand          d18, d18, d20
  0xf24111b4,                                 //vand          d17, d17, d20
  0xf3fb0620,                                 //vcvt.f32.s32  d16, d16
  0xed928a1e,                                 //vldr          s16, [r2, #120]
  0xf3fb1621,                                 //vcvt.f32.s32  d17, d17
  0xed929a1f,                                 //vldr          s18, [r2, #124]
  0xf3fb2622,                                 //vcvt.f32.s32  d18, d18
  0xf2614d05,                                 //vsub.f32      d20, d1, d5
  0xf2e009c3,                                 //vmul.f32      d16, d16, d3[0]
  0xf4a23c9f,                                 //vld1.32       {d3[]}, [r2 :32]
  0xf2625d06,                                 //vsub.f32      d21, d2, d6
  0xf2e119c8,                                 //vmul.f32      d17, d17, d8[0]
  0xf2e229c9,                                 //vmul.f32      d18, d18, d9[0]
  0xf2251115,                                 //vorr          d1, d5, d5
  0xf2262116,                                 //vorr          d2, d6, d6
  0xf2030cb0,                                 //vfma.f32      d0, d19, d16
  0xf2041cb1,                                 //vfma.f32      d1, d20, d17
  0xf2052cb2,                                 //vfma.f32      d2, d21, d18
  0xe28dd008,                                 //add           sp, sp, #8
  0xecbd8b04,                                 //vpop          {d8-d9}
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_load_tables_vfp4[] = {
  0xe92d48f0,                                 //push          {r4, r5, r6, r7, fp, lr}
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xe2826010,                                 //add           r6, r2, #16
  0xe2811008,                                 //add           r1, r1, #8
  0xe593e000,                                 //ldr           lr, [r3]
  0xe99300b0,                                 //ldmib         r3, {r4, r5, r7}
  0xf4e60c9f,                                 //vld1.32       {d16[]}, [r6 :32]
  0xe08e6100,                                 //add           r6, lr, r0, lsl #2
  0xedd61b00,                                 //vldr          d17, [r6]
  0xf24021b1,                                 //vand          d18, d16, d17
  0xed922a03,                                 //vldr          s4, [r2, #12]
  0xf3f03031,                                 //vshr.u32      d19, d17, #16
  0xee326b90,                                 //vmov.32       r6, d18[1]
  0xe0846106,                                 //add           r6, r4, r6, lsl #2
  0xedd60a00,                                 //vldr          s1, [r6]
  0xee126b90,                                 //vmov.32       r6, d18[0]
  0xf3f82031,                                 //vshr.u32      d18, d17, #8
  0xf24021b2,                                 //vand          d18, d16, d18
  0xf24001b3,                                 //vand          d16, d16, d19
  0xee103b90,                                 //vmov.32       r3, d16[0]
  0xe0846106,                                 //add           r6, r4, r6, lsl #2
  0xee304b90,                                 //vmov.32       r4, d16[1]
  0xf3e80031,                                 //vshr.u32      d16, d17, #24
  0xed960a00,                                 //vldr          s0, [r6]
  0xee326b90,                                 //vmov.32       r6, d18[1]
  0xf3fb0620,                                 //vcvt.f32.s32  d16, d16
  0xe0873103,                                 //add           r3, r7, r3, lsl #2
  0xf2a039c2,                                 //vmul.f32      d3, d16, d2[0]
  0xe0874104,                                 //add           r4, r7, r4, lsl #2
  0xedd42a00,                                 //vldr          s5, [r4]
  0xe0856106,                                 //add           r6, r5, r6, lsl #2
  0xed932a00,                                 //vldr          s4, [r3]
  0xedd61a00,                                 //vldr          s3, [r6]
  0xee126b90,                                 //vmov.32       r6, d18[0]
  0xe0856106,                                 //add           r6, r5, r6, lsl #2
  0xed961a00,                                 //vldr          s2, [r6]
  0xe8bd48f0,                                 //pop           {r4, r5, r6, r7, fp, lr}
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_load_a8_vfp4[] = {
  0xe24dd004,                                 //sub           sp, sp, #4
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xe2811008,                                 //add           r1, r1, #8
  0xf2801010,                                 //vmov.i32      d1, #0
  0xf2802010,                                 //vmov.i32      d2, #0
  0xe5933000,                                 //ldr           r3, [r3]
  0xe0833000,                                 //add           r3, r3, r0
  0xe1d330b0,                                 //ldrh          r3, [r3]
  0xe1cd30b0,                                 //strh          r3, [sp]
  0xe1a0300d,                                 //mov           r3, sp
  0xf4e3041f,                                 //vld1.16       {d16[0]}, [r3 :16]
  0xed920a03,                                 //vldr          s0, [r2, #12]
  0xf3c80a30,                                 //vmovl.u8      q8, d16
  0xf3d00a30,                                 //vmovl.u16     q8, d16
  0xf3fb06a0,                                 //vcvt.f32.u32  d16, d16
  0xf2a039c0,                                 //vmul.f32      d3, d16, d0[0]
  0xf2800010,                                 //vmov.i32      d0, #0
  0xe28dd004,                                 //add           sp, sp, #4
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_store_a8_vfp4[] = {
  0xe92d4800,                                 //push          {fp, lr}
  0xe2823008,                                 //add           r3, r2, #8
  0xf2c3061f,                                 //vmov.i32      d16, #1056964608
  0xf4e31c9f,                                 //vld1.32       {d17[]}, [r3 :32]
  0xe5913000,                                 //ldr           r3, [r1]
  0xf2430c31,                                 //vfma.f32      d16, d3, d17
  0xe5933000,                                 //ldr           r3, [r3]
  0xf3fb07a0,                                 //vcvt.u32.f32  d16, d16
  0xee10eb90,                                 //vmov.32       lr, d16[0]
  0xee30cb90,                                 //vmov.32       ip, d16[1]
  0xe7e3e000,                                 //strb          lr, [r3, r0]!
  0xe5c3c001,                                 //strb          ip, [r3, #1]
  0xe5913004,                                 //ldr           r3, [r1, #4]
  0xe2811008,                                 //add           r1, r1, #8
  0xe8bd4800,                                 //pop           {fp, lr}
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_load_565_vfp4[] = {
  0xe24dd004,                                 //sub           sp, sp, #4
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xe2811008,                                 //add           r1, r1, #8
  0xe5933000,                                 //ldr           r3, [r3]
  0xe7933080,                                 //ldr           r3, [r3, r0, lsl #1]
  0xe58d3000,                                 //str           r3, [sp]
  0xe1a0300d,                                 //mov           r3, sp
  0xf4e3083f,                                 //vld1.32       {d16[0]}, [r3 :32]
  0xe282306c,                                 //add           r3, r2, #108
  0xf4e31c9f,                                 //vld1.32       {d17[]}, [r3 :32]
  0xe2823068,                                 //add           r3, r2, #104
  0xf3d04a30,                                 //vmovl.u16     q10, d16
  0xf4e32c9f,                                 //vld1.32       {d18[]}, [r3 :32]
  0xe2823070,                                 //add           r3, r2, #112
  0xf24201b4,                                 //vand          d16, d18, d20
  0xf4e32c9f,                                 //vld1.32       {d18[]}, [r3 :32]
  0xf24111b4,                                 //vand          d17, d17, d20
  0xf24221b4,                                 //vand          d18, d18, d20
  0xf4a23c9f,                                 //vld1.32       {d3[]}, [r2 :32]
  0xf3fb0620,                                 //vcvt.f32.s32  d16, d16
  0xf3fb1621,                                 //vcvt.f32.s32  d17, d17
  0xf3fb2622,                                 //vcvt.f32.s32  d18, d18
  0xed920a1d,                                 //vldr          s0, [r2, #116]
  0xed921a1e,                                 //vldr          s2, [r2, #120]
  0xed922a1f,                                 //vldr          s4, [r2, #124]
  0xf2a009c0,                                 //vmul.f32      d0, d16, d0[0]
  0xf2a119c1,                                 //vmul.f32      d1, d17, d1[0]
  0xf2a229c2,                                 //vmul.f32      d2, d18, d2[0]
  0xe28dd004,                                 //add           sp, sp, #4
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_store_565_vfp4[] = {
  0xe2823080,                                 //add           r3, r2, #128
  0xf2c3361f,                                 //vmov.i32      d19, #1056964608
  0xf2c3461f,                                 //vmov.i32      d20, #1056964608
  0xf4e31c9f,                                 //vld1.32       {d17[]}, [r3 :32]
  0xe2823084,                                 //add           r3, r2, #132
  0xf2403c31,                                 //vfma.f32      d19, d0, d17
  0xf4e32c9f,                                 //vld1.32       {d18[]}, [r3 :32]
  0xf2c3061f,                                 //vmov.i32      d16, #1056964608
  0xf2414c32,                                 //vfma.f32      d20, d1, d18
  0xf2420c31,                                 //vfma.f32      d16, d2, d17
  0xe5913000,                                 //ldr           r3, [r1]
  0xe5933000,                                 //ldr           r3, [r3]
  0xf3fb17a3,                                 //vcvt.u32.f32  d17, d19
  0xe0833080,                                 //add           r3, r3, r0, lsl #1
  0xf3fb27a4,                                 //vcvt.u32.f32  d18, d20
  0xf3fb07a0,                                 //vcvt.u32.f32  d16, d16
  0xf2eb1531,                                 //vshl.s32      d17, d17, #11
  0xf2e52532,                                 //vshl.s32      d18, d18, #5
  0xf26101b0,                                 //vorr          d16, d17, d16
  0xf26001b2,                                 //vorr          d16, d16, d18
  0xf3f60121,                                 //vuzp.16       d16, d17
  0xf4c3080f,                                 //vst1.32       {d16[0]}, [r3]
  0xe5913004,                                 //ldr           r3, [r1, #4]
  0xe2811008,                                 //add           r1, r1, #8
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_load_8888_vfp4[] = {
  0xe92d4800,                                 //push          {fp, lr}
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xe2811008,                                 //add           r1, r1, #8
  0xed922a03,                                 //vldr          s4, [r2, #12]
  0xe593e000,                                 //ldr           lr, [r3]
  0xe2823010,                                 //add           r3, r2, #16
  0xf4e30c9f,                                 //vld1.32       {d16[]}, [r3 :32]
  0xe08e3100,                                 //add           r3, lr, r0, lsl #2
  0xedd31b00,                                 //vldr          d17, [r3]
  0xf24021b1,                                 //vand          d18, d16, d17
  0xf3f83031,                                 //vshr.u32      d19, d17, #8
  0xf3e84031,                                 //vshr.u32      d20, d17, #24
  0xf3f01031,                                 //vshr.u32      d17, d17, #16
  0xf24031b3,                                 //vand          d19, d16, d19
  0xf24001b1,                                 //vand          d16, d16, d17
  0xf3fb2622,                                 //vcvt.f32.s32  d18, d18
  0xf3fb4624,                                 //vcvt.f32.s32  d20, d20
  0xf3fb1623,                                 //vcvt.f32.s32  d17, d19
  0xf3fb0620,                                 //vcvt.f32.s32  d16, d16
  0xf2a209c2,                                 //vmul.f32      d0, d18, d2[0]
  0xf2a439c2,                                 //vmul.f32      d3, d20, d2[0]
  0xf2a119c2,                                 //vmul.f32      d1, d17, d2[0]
  0xf2a029c2,                                 //vmul.f32      d2, d16, d2[0]
  0xe8bd4800,                                 //pop           {fp, lr}
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_store_8888_vfp4[] = {
  0xe2823008,                                 //add           r3, r2, #8
  0xf2c3261f,                                 //vmov.i32      d18, #1056964608
  0xf2c3361f,                                 //vmov.i32      d19, #1056964608
  0xf4e31c9f,                                 //vld1.32       {d17[]}, [r3 :32]
  0xf2c3061f,                                 //vmov.i32      d16, #1056964608
  0xf2412c31,                                 //vfma.f32      d18, d1, d17
  0xf2423c31,                                 //vfma.f32      d19, d2, d17
  0xf2c3461f,                                 //vmov.i32      d20, #1056964608
  0xe5913000,                                 //ldr           r3, [r1]
  0xf2400c31,                                 //vfma.f32      d16, d0, d17
  0xf2434c31,                                 //vfma.f32      d20, d3, d17
  0xe5933000,                                 //ldr           r3, [r3]
  0xe0833100,                                 //add           r3, r3, r0, lsl #2
  0xf3fb17a2,                                 //vcvt.u32.f32  d17, d18
  0xf3fb27a3,                                 //vcvt.u32.f32  d18, d19
  0xf3fb07a0,                                 //vcvt.u32.f32  d16, d16
  0xf3fb37a4,                                 //vcvt.u32.f32  d19, d20
  0xf2e81531,                                 //vshl.s32      d17, d17, #8
  0xf2f02532,                                 //vshl.s32      d18, d18, #16
  0xf26101b0,                                 //vorr          d16, d17, d16
  0xf2f81533,                                 //vshl.s32      d17, d19, #24
  0xf26001b2,                                 //vorr          d16, d16, d18
  0xf26001b1,                                 //vorr          d16, d16, d17
  0xedc30b00,                                 //vstr          d16, [r3]
  0xe5913004,                                 //ldr           r3, [r1, #4]
  0xe2811008,                                 //add           r1, r1, #8
  0xe12fff13,                                 //bx            r3
};

CODE const uint32_t sk_load_f16_vfp4[] = {
  0xed2d8b04,                                 //vpush         {d8-d9}
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xe2811008,                                 //add           r1, r1, #8
  0xe5933000,                                 //ldr           r3, [r3]
  0xe0833180,                                 //add           r3, r3, r0, lsl #3
  0xf463084f,                                 //vld2.16       {d16-d17}, [r3]
  0xf3b62720,                                 //vcvt.f32.f16  q1, d16
  0xf3b68721,                                 //vcvt.f32.f16  q4, d17
  0xf2220112,                                 //vorr          d0, d2, d2
  0xeef00a43,                                 //vmov.f32      s1, s6
  0xf2281118,                                 //vorr          d1, d8, d8
  0xeeb03a62,                                 //vmov.f32      s6, s5
  0xeef01a49,                                 //vmov.f32      s3, s18
  0xeeb09a68,                                 //vmov.f32      s18, s17
  0xeeb02b43,                                 //vmov.f64      d2, d3
  0xeeb03b49,                                 //vmov.f64      d3, d9
  0xecbd8b04,                                 //vpop          {d8-d9}
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_store_f16_vfp4[] = {
  0xeef00b41,                                 //vmov.f64      d16, d1
  0xeef03b42,                                 //vmov.f64      d19, d2
  0xf2631113,                                 //vorr          d17, d3, d3
  0xf2602110,                                 //vorr          d18, d0, d0
  0xf3fa00a1,                                 //vtrn.32       d16, d17
  0xf3f61620,                                 //vcvt.f16.f32  d17, q8
  0xf3fa20a3,                                 //vtrn.32       d18, d19
  0xe5913000,                                 //ldr           r3, [r1]
  0xf3f60622,                                 //vcvt.f16.f32  d16, q9
  0xe5933000,                                 //ldr           r3, [r3]
  0xe0833180,                                 //add           r3, r3, r0, lsl #3
  0xf443084f,                                 //vst2.16       {d16-d17}, [r3]
  0xe2813008,                                 //add           r3, r1, #8
  0xe591c004,                                 //ldr           ip, [r1, #4]
  0xe1a01003,                                 //mov           r1, r3
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_store_f32_vfp4[] = {
  0xe5913000,                                 //ldr           r3, [r1]
  0xe5933000,                                 //ldr           r3, [r3]
  0xe0833200,                                 //add           r3, r3, r0, lsl #4
  0xf403008f,                                 //vst4.32       {d0-d3}, [r3]
  0xe2813008,                                 //add           r3, r1, #8
  0xe591c004,                                 //ldr           ip, [r1, #4]
  0xe1a01003,                                 //mov           r1, r3
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_clamp_x_vfp4[] = {
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xf2c00010,                                 //vmov.i32      d16, #0
  0xf3c71e1f,                                 //vmov.i8       d17, #255
  0xf2400f80,                                 //vmax.f32      d16, d16, d0
  0xe2811008,                                 //add           r1, r1, #8
  0xf4e32c9f,                                 //vld1.32       {d18[]}, [r3 :32]
  0xf26218a1,                                 //vadd.i32      d17, d18, d17
  0xf2200fa1,                                 //vmin.f32      d0, d16, d17
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_clamp_y_vfp4[] = {
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xf2c00010,                                 //vmov.i32      d16, #0
  0xf3c71e1f,                                 //vmov.i8       d17, #255
  0xf2400f81,                                 //vmax.f32      d16, d16, d1
  0xe2811008,                                 //add           r1, r1, #8
  0xf4e32c9f,                                 //vld1.32       {d18[]}, [r3 :32]
  0xf26218a1,                                 //vadd.i32      d17, d18, d17
  0xf2201fa1,                                 //vmin.f32      d1, d16, d17
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_repeat_x_vfp4[] = {
  0xed2d8b04,                                 //vpush         {d8-d9}
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xf2c02010,                                 //vmov.i32      d18, #0
  0xf4e23c9f,                                 //vld1.32       {d19[]}, [r2 :32]
  0xe2811008,                                 //add           r1, r1, #8
  0xed938a00,                                 //vldr          s16, [r3]
  0xeec09a88,                                 //vdiv.f32      s19, s1, s16
  0xee809a08,                                 //vdiv.f32      s18, s0, s16
  0xf3fb0709,                                 //vcvt.s32.f32  d16, d9
  0xf3fb0620,                                 //vcvt.f32.s32  d16, d16
  0xf3601e89,                                 //vcgt.f32      d17, d16, d9
  0xf35311b2,                                 //vbsl          d17, d19, d18
  0xf3f42c08,                                 //vdup.32       d18, d8[0]
  0xf2600da1,                                 //vsub.f32      d16, d16, d17
  0xf3c71e1f,                                 //vmov.i8       d17, #255
  0xf26218a1,                                 //vadd.i32      d17, d18, d17
  0xf2e009c8,                                 //vmul.f32      d16, d16, d8[0]
  0xf2600d20,                                 //vsub.f32      d16, d0, d16
  0xf2200fa1,                                 //vmin.f32      d0, d16, d17
  0xecbd8b04,                                 //vpop          {d8-d9}
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_repeat_y_vfp4[] = {
  0xed2d8b04,                                 //vpush         {d8-d9}
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xf2c02010,                                 //vmov.i32      d18, #0
  0xf4e23c9f,                                 //vld1.32       {d19[]}, [r2 :32]
  0xe2811008,                                 //add           r1, r1, #8
  0xed938a00,                                 //vldr          s16, [r3]
  0xeec19a88,                                 //vdiv.f32      s19, s3, s16
  0xee819a08,                                 //vdiv.f32      s18, s2, s16
  0xf3fb0709,                                 //vcvt.s32.f32  d16, d9
  0xf3fb0620,                                 //vcvt.f32.s32  d16, d16
  0xf3601e89,                                 //vcgt.f32      d17, d16, d9
  0xf35311b2,                                 //vbsl          d17, d19, d18
  0xf3f42c08,                                 //vdup.32       d18, d8[0]
  0xf2600da1,                                 //vsub.f32      d16, d16, d17
  0xf3c71e1f,                                 //vmov.i8       d17, #255
  0xf26218a1,                                 //vadd.i32      d17, d18, d17
  0xf2e009c8,                                 //vmul.f32      d16, d16, d8[0]
  0xf2610d20,                                 //vsub.f32      d16, d1, d16
  0xf2201fa1,                                 //vmin.f32      d1, d16, d17
  0xecbd8b04,                                 //vpop          {d8-d9}
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_mirror_x_vfp4[] = {
  0xed2d8b04,                                 //vpush         {d8-d9}
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xf2c03010,                                 //vmov.i32      d19, #0
  0xf4e24c9f,                                 //vld1.32       {d20[]}, [r2 :32]
  0xe2811008,                                 //add           r1, r1, #8
  0xed938a00,                                 //vldr          s16, [r3]
  0xee389a08,                                 //vadd.f32      s18, s16, s16
  0xf3f40c08,                                 //vdup.32       d16, d8[0]
  0xf2200d20,                                 //vsub.f32      d0, d0, d16
  0xeec08a89,                                 //vdiv.f32      s17, s1, s18
  0xee808a09,                                 //vdiv.f32      s16, s0, s18
  0xf3fb1708,                                 //vcvt.s32.f32  d17, d8
  0xf3fb1621,                                 //vcvt.f32.s32  d17, d17
  0xf3612e88,                                 //vcgt.f32      d18, d17, d8
  0xf35421b3,                                 //vbsl          d18, d20, d19
  0xf2611da2,                                 //vsub.f32      d17, d17, d18
  0xf3c72e1f,                                 //vmov.i8       d18, #255
  0xf2e119c9,                                 //vmul.f32      d17, d17, d9[0]
  0xf2601d21,                                 //vsub.f32      d17, d0, d17
  0xf2611da0,                                 //vsub.f32      d17, d17, d16
  0xf26008a2,                                 //vadd.i32      d16, d16, d18
  0xf3f91721,                                 //vabs.f32      d17, d17
  0xf2210fa0,                                 //vmin.f32      d0, d17, d16
  0xecbd8b04,                                 //vpop          {d8-d9}
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_mirror_y_vfp4[] = {
  0xed2d8b04,                                 //vpush         {d8-d9}
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xf2c03010,                                 //vmov.i32      d19, #0
  0xf4e24c9f,                                 //vld1.32       {d20[]}, [r2 :32]
  0xe2811008,                                 //add           r1, r1, #8
  0xed938a00,                                 //vldr          s16, [r3]
  0xee389a08,                                 //vadd.f32      s18, s16, s16
  0xf3f40c08,                                 //vdup.32       d16, d8[0]
  0xf2211d20,                                 //vsub.f32      d1, d1, d16
  0xeec18a89,                                 //vdiv.f32      s17, s3, s18
  0xee818a09,                                 //vdiv.f32      s16, s2, s18
  0xf3fb1708,                                 //vcvt.s32.f32  d17, d8
  0xf3fb1621,                                 //vcvt.f32.s32  d17, d17
  0xf3612e88,                                 //vcgt.f32      d18, d17, d8
  0xf35421b3,                                 //vbsl          d18, d20, d19
  0xf2611da2,                                 //vsub.f32      d17, d17, d18
  0xf3c72e1f,                                 //vmov.i8       d18, #255
  0xf2e119c9,                                 //vmul.f32      d17, d17, d9[0]
  0xf2611d21,                                 //vsub.f32      d17, d1, d17
  0xf2611da0,                                 //vsub.f32      d17, d17, d16
  0xf26008a2,                                 //vadd.i32      d16, d16, d18
  0xf3f91721,                                 //vabs.f32      d17, d17
  0xf2211fa0,                                 //vmin.f32      d1, d17, d16
  0xecbd8b04,                                 //vpop          {d8-d9}
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_matrix_2x3_vfp4[] = {
  0xe92d4800,                                 //push          {fp, lr}
  0xe591e000,                                 //ldr           lr, [r1]
  0xe591c004,                                 //ldr           ip, [r1, #4]
  0xe2811008,                                 //add           r1, r1, #8
  0xe28e300c,                                 //add           r3, lr, #12
  0xf4e32c9f,                                 //vld1.32       {d18[]}, [r3 :32]
  0xe28e3008,                                 //add           r3, lr, #8
  0xf4e31c9f,                                 //vld1.32       {d17[]}, [r3 :32]
  0xe28e3010,                                 //add           r3, lr, #16
  0xf4e30c9f,                                 //vld1.32       {d16[]}, [r3 :32]
  0xe28e3014,                                 //add           r3, lr, #20
  0xf2410c31,                                 //vfma.f32      d16, d1, d17
  0xf4e31c9f,                                 //vld1.32       {d17[]}, [r3 :32]
  0xe28e3004,                                 //add           r3, lr, #4
  0xf2411c32,                                 //vfma.f32      d17, d1, d18
  0xf4ee2c9f,                                 //vld1.32       {d18[]}, [lr :32]
  0xf4e33c9f,                                 //vld1.32       {d19[]}, [r3 :32]
  0xf2400c32,                                 //vfma.f32      d16, d0, d18
  0xf2401c33,                                 //vfma.f32      d17, d0, d19
  0xf22001b0,                                 //vorr          d0, d16, d16
  0xf22111b1,                                 //vorr          d1, d17, d17
  0xe8bd4800,                                 //pop           {fp, lr}
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_matrix_3x4_vfp4[] = {
  0xe92d4800,                                 //push          {fp, lr}
  0xe591e000,                                 //ldr           lr, [r1]
  0xe591c004,                                 //ldr           ip, [r1, #4]
  0xe2811008,                                 //add           r1, r1, #8
  0xe28e3020,                                 //add           r3, lr, #32
  0xf4e33c9f,                                 //vld1.32       {d19[]}, [r3 :32]
  0xe28e302c,                                 //add           r3, lr, #44
  0xf4e30c9f,                                 //vld1.32       {d16[]}, [r3 :32]
  0xe28e301c,                                 //add           r3, lr, #28
  0xf2420c33,                                 //vfma.f32      d16, d2, d19
  0xf4e34c9f,                                 //vld1.32       {d20[]}, [r3 :32]
  0xe28e3018,                                 //add           r3, lr, #24
  0xf4e32c9f,                                 //vld1.32       {d18[]}, [r3 :32]
  0xe28e3024,                                 //add           r3, lr, #36
  0xf4e31c9f,                                 //vld1.32       {d17[]}, [r3 :32]
  0xe28e3028,                                 //add           r3, lr, #40
  0xf2421c32,                                 //vfma.f32      d17, d2, d18
  0xf4e32c9f,                                 //vld1.32       {d18[]}, [r3 :32]
  0xe28e3010,                                 //add           r3, lr, #16
  0xf2422c34,                                 //vfma.f32      d18, d2, d20
  0xf4e33c9f,                                 //vld1.32       {d19[]}, [r3 :32]
  0xe28e300c,                                 //add           r3, lr, #12
  0xf4e34c9f,                                 //vld1.32       {d20[]}, [r3 :32]
  0xe28e3014,                                 //add           r3, lr, #20
  0xf2411c34,                                 //vfma.f32      d17, d1, d20
  0xf4e34c9f,                                 //vld1.32       {d20[]}, [r3 :32]
  0xf2410c34,                                 //vfma.f32      d16, d1, d20
  0xe28e3004,                                 //add           r3, lr, #4
  0xf2412c33,                                 //vfma.f32      d18, d1, d19
  0xf4ee3c9f,                                 //vld1.32       {d19[]}, [lr :32]
  0xf4e34c9f,                                 //vld1.32       {d20[]}, [r3 :32]
  0xe28e3008,                                 //add           r3, lr, #8
  0xf2401c33,                                 //vfma.f32      d17, d0, d19
  0xf4e33c9f,                                 //vld1.32       {d19[]}, [r3 :32]
  0xf2400c33,                                 //vfma.f32      d16, d0, d19
  0xf2402c34,                                 //vfma.f32      d18, d0, d20
  0xf22101b1,                                 //vorr          d0, d17, d17
  0xf22021b0,                                 //vorr          d2, d16, d16
  0xf22211b2,                                 //vorr          d1, d18, d18
  0xe8bd4800,                                 //pop           {fp, lr}
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_matrix_perspective_vfp4[] = {
  0xe92d4800,                                 //push          {fp, lr}
  0xe591e000,                                 //ldr           lr, [r1]
  0xe591c004,                                 //ldr           ip, [r1, #4]
  0xe2811008,                                 //add           r1, r1, #8
  0xe28e301c,                                 //add           r3, lr, #28
  0xf4e30c9f,                                 //vld1.32       {d16[]}, [r3 :32]
  0xe28e3020,                                 //add           r3, lr, #32
  0xf4e31c9f,                                 //vld1.32       {d17[]}, [r3 :32]
  0xe28e3018,                                 //add           r3, lr, #24
  0xf2411c30,                                 //vfma.f32      d17, d1, d16
  0xf4e30c9f,                                 //vld1.32       {d16[]}, [r3 :32]
  0xe28e3010,                                 //add           r3, lr, #16
  0xf2401c30,                                 //vfma.f32      d17, d0, d16
  0xf4e30c9f,                                 //vld1.32       {d16[]}, [r3 :32]
  0xe28e3004,                                 //add           r3, lr, #4
  0xf4e32c9f,                                 //vld1.32       {d18[]}, [r3 :32]
  0xe28e3008,                                 //add           r3, lr, #8
  0xf4e34c9f,                                 //vld1.32       {d20[]}, [r3 :32]
  0xe28e3014,                                 //add           r3, lr, #20
  0xf2414c32,                                 //vfma.f32      d20, d1, d18
  0xf4e32c9f,                                 //vld1.32       {d18[]}, [r3 :32]
  0xe28e300c,                                 //add           r3, lr, #12
  0xf3fb3521,                                 //vrecpe.f32    d19, d17
  0xf2412c30,                                 //vfma.f32      d18, d1, d16
  0xf4e35c9f,                                 //vld1.32       {d21[]}, [r3 :32]
  0xf2410fb3,                                 //vrecps.f32    d16, d17, d19
  0xf4ee1c9f,                                 //vld1.32       {d17[]}, [lr :32]
  0xf2404c31,                                 //vfma.f32      d20, d0, d17
  0xf2402c35,                                 //vfma.f32      d18, d0, d21
  0xf3430db0,                                 //vmul.f32      d16, d19, d16
  0xf3040db0,                                 //vmul.f32      d0, d20, d16
  0xf3021db0,                                 //vmul.f32      d1, d18, d16
  0xe8bd4800,                                 //pop           {fp, lr}
  0xe12fff1c,                                 //bx            ip
};

CODE const uint32_t sk_linear_gradient_2stops_vfp4[] = {
  0xe8911008,                                 //ldm           r1, {r3, ip}
  0xe2811008,                                 //add           r1, r1, #8
  0xf4632a0d,                                 //vld1.8        {d18-d19}, [r3]!
  0xf4634a0f,                                 //vld1.8        {d20-d21}, [r3]
  0xf3f40c22,                                 //vdup.32       d16, d18[0]
  0xf3f41c24,                                 //vdup.32       d17, d20[0]
  0xf2400c31,                                 //vfma.f32      d16, d0, d17
  0xf3fc6c24,                                 //vdup.32       d22, d20[1]
  0xf3bc1c22,                                 //vdup.32       d1, d18[1]
  0xf3b42c23,                                 //vdup.32       d2, d19[0]
  0xf2001c36,                                 //vfma.f32      d1, d0, d22
  0xf3f41c25,                                 //vdup.32       d17, d21[0]
  0xf3fc4c25,                                 //vdup.32       d20, d21[1]
  0xf2002c31,                                 //vfma.f32      d2, d0, d17
  0xf3bc3c23,                                 //vdup.32       d3, d19[1]
  0xf2003c34,                                 //vfma.f32      d3, d0, d20
  0xf22001b0,                                 //vorr          d0, d16, d16
  0xe12fff1c,                                 //bx            ip
};
#elif defined(__x86_64__) || defined(_M_X64)

CODE const uint8_t sk_start_pipeline_hsw[] = {
  0x41,0x57,                                  //push          %r15
  0x41,0x56,                                  //push          %r14
  0x41,0x55,                                  //push          %r13
  0x41,0x54,                                  //push          %r12
  0x53,                                       //push          %rbx
  0x49,0x89,0xcd,                             //mov           %rcx,%r13
  0x49,0x89,0xd6,                             //mov           %rdx,%r14
  0x48,0x89,0xfb,                             //mov           %rdi,%rbx
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x49,0x89,0xc7,                             //mov           %rax,%r15
  0x49,0x89,0xf4,                             //mov           %rsi,%r12
  0x48,0x8d,0x43,0x08,                        //lea           0x8(%rbx),%rax
  0x4c,0x39,0xe8,                             //cmp           %r13,%rax
  0x76,0x05,                                  //jbe           28 <_sk_start_pipeline_hsw+0x28>
  0x48,0x89,0xdf,                             //mov           %rbx,%rdi
  0xeb,0x41,                                  //jmp           69 <_sk_start_pipeline_hsw+0x69>
  0xb9,0x00,0x00,0x00,0x00,                   //mov           $0x0,%ecx
  0xc5,0xfc,0x57,0xc0,                        //vxorps        %ymm0,%ymm0,%ymm0
  0xc5,0xf4,0x57,0xc9,                        //vxorps        %ymm1,%ymm1,%ymm1
  0xc5,0xec,0x57,0xd2,                        //vxorps        %ymm2,%ymm2,%ymm2
  0xc5,0xe4,0x57,0xdb,                        //vxorps        %ymm3,%ymm3,%ymm3
  0xc5,0xdc,0x57,0xe4,                        //vxorps        %ymm4,%ymm4,%ymm4
  0xc5,0xd4,0x57,0xed,                        //vxorps        %ymm5,%ymm5,%ymm5
  0xc5,0xcc,0x57,0xf6,                        //vxorps        %ymm6,%ymm6,%ymm6
  0xc5,0xc4,0x57,0xff,                        //vxorps        %ymm7,%ymm7,%ymm7
  0x48,0x89,0xdf,                             //mov           %rbx,%rdi
  0x4c,0x89,0xe6,                             //mov           %r12,%rsi
  0x4c,0x89,0xf2,                             //mov           %r14,%rdx
  0x41,0xff,0xd7,                             //callq         *%r15
  0x48,0x8d,0x7b,0x08,                        //lea           0x8(%rbx),%rdi
  0x48,0x83,0xc3,0x10,                        //add           $0x10,%rbx
  0x4c,0x39,0xeb,                             //cmp           %r13,%rbx
  0x48,0x89,0xfb,                             //mov           %rdi,%rbx
  0x76,0xbf,                                  //jbe           28 <_sk_start_pipeline_hsw+0x28>
  0x4c,0x89,0xe9,                             //mov           %r13,%rcx
  0x48,0x29,0xf9,                             //sub           %rdi,%rcx
  0x74,0x29,                                  //je            9a <_sk_start_pipeline_hsw+0x9a>
  0xc5,0xfc,0x57,0xc0,                        //vxorps        %ymm0,%ymm0,%ymm0
  0xc5,0xf4,0x57,0xc9,                        //vxorps        %ymm1,%ymm1,%ymm1
  0xc5,0xec,0x57,0xd2,                        //vxorps        %ymm2,%ymm2,%ymm2
  0xc5,0xe4,0x57,0xdb,                        //vxorps        %ymm3,%ymm3,%ymm3
  0xc5,0xdc,0x57,0xe4,                        //vxorps        %ymm4,%ymm4,%ymm4
  0xc5,0xd4,0x57,0xed,                        //vxorps        %ymm5,%ymm5,%ymm5
  0xc5,0xcc,0x57,0xf6,                        //vxorps        %ymm6,%ymm6,%ymm6
  0xc5,0xc4,0x57,0xff,                        //vxorps        %ymm7,%ymm7,%ymm7
  0x4c,0x89,0xe6,                             //mov           %r12,%rsi
  0x4c,0x89,0xf2,                             //mov           %r14,%rdx
  0x41,0xff,0xd7,                             //callq         *%r15
  0x4c,0x89,0xe8,                             //mov           %r13,%rax
  0x5b,                                       //pop           %rbx
  0x41,0x5c,                                  //pop           %r12
  0x41,0x5d,                                  //pop           %r13
  0x41,0x5e,                                  //pop           %r14
  0x41,0x5f,                                  //pop           %r15
  0xc5,0xf8,0x77,                             //vzeroupper
  0xc3,                                       //retq
};

CODE const uint8_t sk_start_pipeline_ms_hsw[] = {
  0x56,                                       //push          %rsi
  0x57,                                       //push          %rdi
  0x48,0x81,0xec,0xa8,0x00,0x00,0x00,         //sub           $0xa8,%rsp
  0xc5,0x78,0x29,0xbc,0x24,0x90,0x00,0x00,0x00,//vmovaps       %xmm15,0x90(%rsp)
  0xc5,0x78,0x29,0xb4,0x24,0x80,0x00,0x00,0x00,//vmovaps       %xmm14,0x80(%rsp)
  0xc5,0x78,0x29,0x6c,0x24,0x70,              //vmovaps       %xmm13,0x70(%rsp)
  0xc5,0x78,0x29,0x64,0x24,0x60,              //vmovaps       %xmm12,0x60(%rsp)
  0xc5,0x78,0x29,0x5c,0x24,0x50,              //vmovaps       %xmm11,0x50(%rsp)
  0xc5,0x78,0x29,0x54,0x24,0x40,              //vmovaps       %xmm10,0x40(%rsp)
  0xc5,0x78,0x29,0x4c,0x24,0x30,              //vmovaps       %xmm9,0x30(%rsp)
  0xc5,0x78,0x29,0x44,0x24,0x20,              //vmovaps       %xmm8,0x20(%rsp)
  0xc5,0xf8,0x29,0x7c,0x24,0x10,              //vmovaps       %xmm7,0x10(%rsp)
  0xc5,0xf8,0x29,0x34,0x24,                   //vmovaps       %xmm6,(%rsp)
  0x48,0x89,0xcf,                             //mov           %rcx,%rdi
  0x48,0x89,0xd6,                             //mov           %rdx,%rsi
  0x4c,0x89,0xc2,                             //mov           %r8,%rdx
  0x4c,0x89,0xc9,                             //mov           %r9,%rcx
  0xe8,0x00,0x00,0x00,0x00,                   //callq         105 <_sk_start_pipeline_ms_hsw+0x5b>
  0xc5,0xf8,0x28,0x34,0x24,                   //vmovaps       (%rsp),%xmm6
  0xc5,0xf8,0x28,0x7c,0x24,0x10,              //vmovaps       0x10(%rsp),%xmm7
  0xc5,0x78,0x28,0x44,0x24,0x20,              //vmovaps       0x20(%rsp),%xmm8
  0xc5,0x78,0x28,0x4c,0x24,0x30,              //vmovaps       0x30(%rsp),%xmm9
  0xc5,0x78,0x28,0x54,0x24,0x40,              //vmovaps       0x40(%rsp),%xmm10
  0xc5,0x78,0x28,0x5c,0x24,0x50,              //vmovaps       0x50(%rsp),%xmm11
  0xc5,0x78,0x28,0x64,0x24,0x60,              //vmovaps       0x60(%rsp),%xmm12
  0xc5,0x78,0x28,0x6c,0x24,0x70,              //vmovaps       0x70(%rsp),%xmm13
  0xc5,0x78,0x28,0xb4,0x24,0x80,0x00,0x00,0x00,//vmovaps       0x80(%rsp),%xmm14
  0xc5,0x78,0x28,0xbc,0x24,0x90,0x00,0x00,0x00,//vmovaps       0x90(%rsp),%xmm15
  0x48,0x81,0xc4,0xa8,0x00,0x00,0x00,         //add           $0xa8,%rsp
  0x5f,                                       //pop           %rdi
  0x5e,                                       //pop           %rsi
  0xc3,                                       //retq
};

CODE const uint8_t sk_just_return_hsw[] = {
  0xc3,                                       //retq
};

CODE const uint8_t sk_seed_shader_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0xf9,0x6e,0xc7,                        //vmovd         %edi,%xmm0
  0xc4,0xe2,0x7d,0x18,0xc0,                   //vbroadcastss  %xmm0,%ymm0
  0xc5,0xfc,0x5b,0xc0,                        //vcvtdq2ps     %ymm0,%ymm0
  0xc4,0xe2,0x7d,0x18,0x4a,0x04,              //vbroadcastss  0x4(%rdx),%ymm1
  0xc5,0xfc,0x58,0xc1,                        //vaddps        %ymm1,%ymm0,%ymm0
  0xc5,0xfc,0x58,0x42,0x14,                   //vaddps        0x14(%rdx),%ymm0,%ymm0
  0xc4,0xe2,0x7d,0x18,0x10,                   //vbroadcastss  (%rax),%ymm2
  0xc5,0xfc,0x5b,0xd2,                        //vcvtdq2ps     %ymm2,%ymm2
  0xc5,0xec,0x58,0xc9,                        //vaddps        %ymm1,%ymm2,%ymm1
  0xc4,0xe2,0x7d,0x18,0x12,                   //vbroadcastss  (%rdx),%ymm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0xe4,0x57,0xdb,                        //vxorps        %ymm3,%ymm3,%ymm3
  0xc5,0xdc,0x57,0xe4,                        //vxorps        %ymm4,%ymm4,%ymm4
  0xc5,0xd4,0x57,0xed,                        //vxorps        %ymm5,%ymm5,%ymm5
  0xc5,0xcc,0x57,0xf6,                        //vxorps        %ymm6,%ymm6,%ymm6
  0xc5,0xc4,0x57,0xff,                        //vxorps        %ymm7,%ymm7,%ymm7
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_constant_color_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0xe2,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm0
  0xc4,0xe2,0x7d,0x18,0x48,0x04,              //vbroadcastss  0x4(%rax),%ymm1
  0xc4,0xe2,0x7d,0x18,0x50,0x08,              //vbroadcastss  0x8(%rax),%ymm2
  0xc4,0xe2,0x7d,0x18,0x58,0x0c,              //vbroadcastss  0xc(%rax),%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clear_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0xfc,0x57,0xc0,                        //vxorps        %ymm0,%ymm0,%ymm0
  0xc5,0xf4,0x57,0xc9,                        //vxorps        %ymm1,%ymm1,%ymm1
  0xc5,0xec,0x57,0xd2,                        //vxorps        %ymm2,%ymm2,%ymm2
  0xc5,0xe4,0x57,0xdb,                        //vxorps        %ymm3,%ymm3,%ymm3
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_plus__hsw[] = {
  0xc5,0xfc,0x58,0xc4,                        //vaddps        %ymm4,%ymm0,%ymm0
  0xc5,0xf4,0x58,0xcd,                        //vaddps        %ymm5,%ymm1,%ymm1
  0xc5,0xec,0x58,0xd6,                        //vaddps        %ymm6,%ymm2,%ymm2
  0xc5,0xe4,0x58,0xdf,                        //vaddps        %ymm7,%ymm3,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_srcover_hsw[] = {
  0xc4,0x62,0x7d,0x18,0x02,                   //vbroadcastss  (%rdx),%ymm8
  0xc5,0x3c,0x5c,0xc3,                        //vsubps        %ymm3,%ymm8,%ymm8
  0xc4,0xc2,0x5d,0xb8,0xc0,                   //vfmadd231ps   %ymm8,%ymm4,%ymm0
  0xc4,0xc2,0x55,0xb8,0xc8,                   //vfmadd231ps   %ymm8,%ymm5,%ymm1
  0xc4,0xc2,0x4d,0xb8,0xd0,                   //vfmadd231ps   %ymm8,%ymm6,%ymm2
  0xc4,0xc2,0x45,0xb8,0xd8,                   //vfmadd231ps   %ymm8,%ymm7,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_dstover_hsw[] = {
  0xc4,0x62,0x7d,0x18,0x02,                   //vbroadcastss  (%rdx),%ymm8
  0xc5,0x3c,0x5c,0xc7,                        //vsubps        %ymm7,%ymm8,%ymm8
  0xc4,0xe2,0x3d,0xa8,0xc4,                   //vfmadd213ps   %ymm4,%ymm8,%ymm0
  0xc4,0xe2,0x3d,0xa8,0xcd,                   //vfmadd213ps   %ymm5,%ymm8,%ymm1
  0xc4,0xe2,0x3d,0xa8,0xd6,                   //vfmadd213ps   %ymm6,%ymm8,%ymm2
  0xc4,0xe2,0x3d,0xa8,0xdf,                   //vfmadd213ps   %ymm7,%ymm8,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_0_hsw[] = {
  0xc4,0x41,0x3c,0x57,0xc0,                   //vxorps        %ymm8,%ymm8,%ymm8
  0xc4,0xc1,0x7c,0x5f,0xc0,                   //vmaxps        %ymm8,%ymm0,%ymm0
  0xc4,0xc1,0x74,0x5f,0xc8,                   //vmaxps        %ymm8,%ymm1,%ymm1
  0xc4,0xc1,0x6c,0x5f,0xd0,                   //vmaxps        %ymm8,%ymm2,%ymm2
  0xc4,0xc1,0x64,0x5f,0xd8,                   //vmaxps        %ymm8,%ymm3,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_1_hsw[] = {
  0xc4,0x62,0x7d,0x18,0x02,                   //vbroadcastss  (%rdx),%ymm8
  0xc4,0xc1,0x7c,0x5d,0xc0,                   //vminps        %ymm8,%ymm0,%ymm0
  0xc4,0xc1,0x74,0x5d,0xc8,                   //vminps        %ymm8,%ymm1,%ymm1
  0xc4,0xc1,0x6c,0x5d,0xd0,                   //vminps        %ymm8,%ymm2,%ymm2
  0xc4,0xc1,0x64,0x5d,0xd8,                   //vminps        %ymm8,%ymm3,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_a_hsw[] = {
  0xc4,0x62,0x7d,0x18,0x02,                   //vbroadcastss  (%rdx),%ymm8
  0xc4,0xc1,0x64,0x5d,0xd8,                   //vminps        %ymm8,%ymm3,%ymm3
  0xc5,0xfc,0x5d,0xc3,                        //vminps        %ymm3,%ymm0,%ymm0
  0xc5,0xf4,0x5d,0xcb,                        //vminps        %ymm3,%ymm1,%ymm1
  0xc5,0xec,0x5d,0xd3,                        //vminps        %ymm3,%ymm2,%ymm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_set_rgb_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0xe2,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm0
  0xc4,0xe2,0x7d,0x18,0x48,0x04,              //vbroadcastss  0x4(%rax),%ymm1
  0xc4,0xe2,0x7d,0x18,0x50,0x08,              //vbroadcastss  0x8(%rax),%ymm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_swap_rb_hsw[] = {
  0xc5,0x7c,0x28,0xc0,                        //vmovaps       %ymm0,%ymm8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0xfc,0x28,0xc2,                        //vmovaps       %ymm2,%ymm0
  0xc5,0x7c,0x29,0xc2,                        //vmovaps       %ymm8,%ymm2
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_swap_hsw[] = {
  0xc5,0x7c,0x28,0xc3,                        //vmovaps       %ymm3,%ymm8
  0xc5,0x7c,0x28,0xca,                        //vmovaps       %ymm2,%ymm9
  0xc5,0x7c,0x28,0xd1,                        //vmovaps       %ymm1,%ymm10
  0xc5,0x7c,0x28,0xd8,                        //vmovaps       %ymm0,%ymm11
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0xfc,0x28,0xc4,                        //vmovaps       %ymm4,%ymm0
  0xc5,0xfc,0x28,0xcd,                        //vmovaps       %ymm5,%ymm1
  0xc5,0xfc,0x28,0xd6,                        //vmovaps       %ymm6,%ymm2
  0xc5,0xfc,0x28,0xdf,                        //vmovaps       %ymm7,%ymm3
  0xc5,0x7c,0x29,0xdc,                        //vmovaps       %ymm11,%ymm4
  0xc5,0x7c,0x29,0xd5,                        //vmovaps       %ymm10,%ymm5
  0xc5,0x7c,0x29,0xce,                        //vmovaps       %ymm9,%ymm6
  0xc5,0x7c,0x29,0xc7,                        //vmovaps       %ymm8,%ymm7
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_move_src_dst_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0xfc,0x28,0xe0,                        //vmovaps       %ymm0,%ymm4
  0xc5,0xfc,0x28,0xe9,                        //vmovaps       %ymm1,%ymm5
  0xc5,0xfc,0x28,0xf2,                        //vmovaps       %ymm2,%ymm6
  0xc5,0xfc,0x28,0xfb,                        //vmovaps       %ymm3,%ymm7
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_move_dst_src_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0xfc,0x28,0xc4,                        //vmovaps       %ymm4,%ymm0
  0xc5,0xfc,0x28,0xcd,                        //vmovaps       %ymm5,%ymm1
  0xc5,0xfc,0x28,0xd6,                        //vmovaps       %ymm6,%ymm2
  0xc5,0xfc,0x28,0xdf,                        //vmovaps       %ymm7,%ymm3
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_premul_hsw[] = {
  0xc5,0xfc,0x59,0xc3,                        //vmulps        %ymm3,%ymm0,%ymm0
  0xc5,0xf4,0x59,0xcb,                        //vmulps        %ymm3,%ymm1,%ymm1
  0xc5,0xec,0x59,0xd3,                        //vmulps        %ymm3,%ymm2,%ymm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_unpremul_hsw[] = {
  0xc4,0x41,0x3c,0x57,0xc0,                   //vxorps        %ymm8,%ymm8,%ymm8
  0xc4,0x41,0x64,0xc2,0xc8,0x00,              //vcmpeqps      %ymm8,%ymm3,%ymm9
  0xc4,0x62,0x7d,0x18,0x12,                   //vbroadcastss  (%rdx),%ymm10
  0xc5,0x2c,0x5e,0xd3,                        //vdivps        %ymm3,%ymm10,%ymm10
  0xc4,0x43,0x2d,0x4a,0xc0,0x90,              //vblendvps     %ymm9,%ymm8,%ymm10,%ymm8
  0xc5,0xbc,0x59,0xc0,                        //vmulps        %ymm0,%ymm8,%ymm0
  0xc5,0xbc,0x59,0xc9,                        //vmulps        %ymm1,%ymm8,%ymm1
  0xc5,0xbc,0x59,0xd2,                        //vmulps        %ymm2,%ymm8,%ymm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_from_srgb_hsw[] = {
  0xc4,0x62,0x7d,0x18,0x42,0x40,              //vbroadcastss  0x40(%rdx),%ymm8
  0xc5,0x3c,0x59,0xc8,                        //vmulps        %ymm0,%ymm8,%ymm9
  0xc5,0x7c,0x59,0xd0,                        //vmulps        %ymm0,%ymm0,%ymm10
  0xc4,0x62,0x7d,0x18,0x5a,0x3c,              //vbroadcastss  0x3c(%rdx),%ymm11
  0xc4,0x62,0x7d,0x18,0x62,0x38,              //vbroadcastss  0x38(%rdx),%ymm12
  0xc4,0x41,0x7c,0x28,0xeb,                   //vmovaps       %ymm11,%ymm13
  0xc4,0x42,0x7d,0xa8,0xec,                   //vfmadd213ps   %ymm12,%ymm0,%ymm13
  0xc4,0x62,0x7d,0x18,0x72,0x34,              //vbroadcastss  0x34(%rdx),%ymm14
  0xc4,0x42,0x2d,0xa8,0xee,                   //vfmadd213ps   %ymm14,%ymm10,%ymm13
  0xc4,0x62,0x7d,0x18,0x52,0x44,              //vbroadcastss  0x44(%rdx),%ymm10
  0xc4,0xc1,0x7c,0xc2,0xc2,0x01,              //vcmpltps      %ymm10,%ymm0,%ymm0
  0xc4,0xc3,0x15,0x4a,0xc1,0x00,              //vblendvps     %ymm0,%ymm9,%ymm13,%ymm0
  0xc5,0x3c,0x59,0xc9,                        //vmulps        %ymm1,%ymm8,%ymm9
  0xc5,0x74,0x59,0xe9,                        //vmulps        %ymm1,%ymm1,%ymm13
  0xc4,0x41,0x7c,0x28,0xfb,                   //vmovaps       %ymm11,%ymm15
  0xc4,0x42,0x75,0xa8,0xfc,                   //vfmadd213ps   %ymm12,%ymm1,%ymm15
  0xc4,0x42,0x15,0xa8,0xfe,                   //vfmadd213ps   %ymm14,%ymm13,%ymm15
  0xc4,0xc1,0x74,0xc2,0xca,0x01,              //vcmpltps      %ymm10,%ymm1,%ymm1
  0xc4,0xc3,0x05,0x4a,0xc9,0x10,              //vblendvps     %ymm1,%ymm9,%ymm15,%ymm1
  0xc5,0x3c,0x59,0xc2,                        //vmulps        %ymm2,%ymm8,%ymm8
  0xc5,0x6c,0x59,0xca,                        //vmulps        %ymm2,%ymm2,%ymm9
  0xc4,0x42,0x6d,0xa8,0xdc,                   //vfmadd213ps   %ymm12,%ymm2,%ymm11
  0xc4,0x42,0x35,0xa8,0xde,                   //vfmadd213ps   %ymm14,%ymm9,%ymm11
  0xc4,0xc1,0x6c,0xc2,0xd2,0x01,              //vcmpltps      %ymm10,%ymm2,%ymm2
  0xc4,0xc3,0x25,0x4a,0xd0,0x20,              //vblendvps     %ymm2,%ymm8,%ymm11,%ymm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_to_srgb_hsw[] = {
  0xc5,0x7c,0x52,0xc0,                        //vrsqrtps      %ymm0,%ymm8
  0xc4,0x41,0x7c,0x53,0xc8,                   //vrcpps        %ymm8,%ymm9
  0xc4,0x41,0x7c,0x52,0xd0,                   //vrsqrtps      %ymm8,%ymm10
  0xc4,0x62,0x7d,0x18,0x42,0x48,              //vbroadcastss  0x48(%rdx),%ymm8
  0xc5,0x3c,0x59,0xd8,                        //vmulps        %ymm0,%ymm8,%ymm11
  0xc4,0x62,0x7d,0x18,0x22,                   //vbroadcastss  (%rdx),%ymm12
  0xc4,0x62,0x7d,0x18,0x6a,0x4c,              //vbroadcastss  0x4c(%rdx),%ymm13
  0xc4,0x62,0x7d,0x18,0x72,0x50,              //vbroadcastss  0x50(%rdx),%ymm14
  0xc4,0x62,0x7d,0x18,0x7a,0x54,              //vbroadcastss  0x54(%rdx),%ymm15
  0xc4,0x42,0x0d,0xa8,0xcf,                   //vfmadd213ps   %ymm15,%ymm14,%ymm9
  0xc4,0x42,0x15,0xb8,0xca,                   //vfmadd231ps   %ymm10,%ymm13,%ymm9
  0xc4,0x41,0x1c,0x5d,0xc9,                   //vminps        %ymm9,%ymm12,%ymm9
  0xc4,0x62,0x7d,0x18,0x52,0x58,              //vbroadcastss  0x58(%rdx),%ymm10
  0xc4,0xc1,0x7c,0xc2,0xc2,0x01,              //vcmpltps      %ymm10,%ymm0,%ymm0
  0xc4,0xc3,0x35,0x4a,0xc3,0x00,              //vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
  0xc5,0x7c,0x52,0xc9,                        //vrsqrtps      %ymm1,%ymm9
  0xc4,0x41,0x7c,0x53,0xd9,                   //vrcpps        %ymm9,%ymm11
  0xc4,0x41,0x7c,0x52,0xc9,                   //vrsqrtps      %ymm9,%ymm9
  0xc4,0x42,0x0d,0xa8,0xdf,                   //vfmadd213ps   %ymm15,%ymm14,%ymm11
  0xc4,0x42,0x15,0xb8,0xd9,                   //vfmadd231ps   %ymm9,%ymm13,%ymm11
  0xc5,0x3c,0x59,0xc9,                        //vmulps        %ymm1,%ymm8,%ymm9
  0xc4,0x41,0x1c,0x5d,0xdb,                   //vminps        %ymm11,%ymm12,%ymm11
  0xc4,0xc1,0x74,0xc2,0xca,0x01,              //vcmpltps      %ymm10,%ymm1,%ymm1
  0xc4,0xc3,0x25,0x4a,0xc9,0x10,              //vblendvps     %ymm1,%ymm9,%ymm11,%ymm1
  0xc5,0x7c,0x52,0xca,                        //vrsqrtps      %ymm2,%ymm9
  0xc4,0x41,0x7c,0x53,0xd9,                   //vrcpps        %ymm9,%ymm11
  0xc4,0x42,0x0d,0xa8,0xdf,                   //vfmadd213ps   %ymm15,%ymm14,%ymm11
  0xc4,0x41,0x7c,0x52,0xc9,                   //vrsqrtps      %ymm9,%ymm9
  0xc4,0x42,0x15,0xb8,0xd9,                   //vfmadd231ps   %ymm9,%ymm13,%ymm11
  0xc4,0x41,0x1c,0x5d,0xcb,                   //vminps        %ymm11,%ymm12,%ymm9
  0xc5,0x3c,0x59,0xc2,                        //vmulps        %ymm2,%ymm8,%ymm8
  0xc4,0xc1,0x6c,0xc2,0xd2,0x01,              //vcmpltps      %ymm10,%ymm2,%ymm2
  0xc4,0xc3,0x35,0x4a,0xd0,0x20,              //vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_scale_1_float_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x62,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm8
  0xc5,0xbc,0x59,0xc0,                        //vmulps        %ymm0,%ymm8,%ymm0
  0xc5,0xbc,0x59,0xc9,                        //vmulps        %ymm1,%ymm8,%ymm1
  0xc5,0xbc,0x59,0xd2,                        //vmulps        %ymm2,%ymm8,%ymm2
  0xc5,0xbc,0x59,0xdb,                        //vmulps        %ymm3,%ymm8,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_scale_u8_hsw[] = {
  0x49,0x89,0xc8,                             //mov           %rcx,%r8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x48,0x01,0xf8,                             //add           %rdi,%rax
  0x4d,0x85,0xc0,                             //test          %r8,%r8
  0x75,0x30,                                  //jne           4c0 <_sk_scale_u8_hsw+0x40>
  0xc5,0x7b,0x10,0x00,                        //vmovsd        (%rax),%xmm8
  0xc4,0x42,0x7d,0x31,0xc0,                   //vpmovzxbd     %xmm8,%ymm8
  0xc4,0x41,0x7c,0x5b,0xc0,                   //vcvtdq2ps     %ymm8,%ymm8
  0xc4,0x62,0x7d,0x18,0x4a,0x0c,              //vbroadcastss  0xc(%rdx),%ymm9
  0xc4,0x41,0x3c,0x59,0xc1,                   //vmulps        %ymm9,%ymm8,%ymm8
  0xc5,0xbc,0x59,0xc0,                        //vmulps        %ymm0,%ymm8,%ymm0
  0xc5,0xbc,0x59,0xc9,                        //vmulps        %ymm1,%ymm8,%ymm1
  0xc5,0xbc,0x59,0xd2,                        //vmulps        %ymm2,%ymm8,%ymm2
  0xc5,0xbc,0x59,0xdb,                        //vmulps        %ymm3,%ymm8,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x89,0xc1,                             //mov           %r8,%rcx
  0xff,0xe0,                                  //jmpq          *%rax
  0x31,0xc9,                                  //xor           %ecx,%ecx
  0x4d,0x89,0xc2,                             //mov           %r8,%r10
  0x45,0x31,0xc9,                             //xor           %r9d,%r9d
  0x44,0x0f,0xb6,0x18,                        //movzbl        (%rax),%r11d
  0x48,0xff,0xc0,                             //inc           %rax
  0x49,0xd3,0xe3,                             //shl           %cl,%r11
  0x4d,0x09,0xd9,                             //or            %r11,%r9
  0x48,0x83,0xc1,0x08,                        //add           $0x8,%rcx
  0x49,0xff,0xca,                             //dec           %r10
  0x75,0xea,                                  //jne           4c8 <_sk_scale_u8_hsw+0x48>
  0xc4,0x41,0xf9,0x6e,0xc1,                   //vmovq         %r9,%xmm8
  0xeb,0xaf,                                  //jmp           494 <_sk_scale_u8_hsw+0x14>
};

CODE const uint8_t sk_lerp_1_float_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x62,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm8
  0xc5,0xfc,0x5c,0xc4,                        //vsubps        %ymm4,%ymm0,%ymm0
  0xc4,0xe2,0x3d,0xa8,0xc4,                   //vfmadd213ps   %ymm4,%ymm8,%ymm0
  0xc5,0xf4,0x5c,0xcd,                        //vsubps        %ymm5,%ymm1,%ymm1
  0xc4,0xe2,0x3d,0xa8,0xcd,                   //vfmadd213ps   %ymm5,%ymm8,%ymm1
  0xc5,0xec,0x5c,0xd6,                        //vsubps        %ymm6,%ymm2,%ymm2
  0xc4,0xe2,0x3d,0xa8,0xd6,                   //vfmadd213ps   %ymm6,%ymm8,%ymm2
  0xc5,0xe4,0x5c,0xdf,                        //vsubps        %ymm7,%ymm3,%ymm3
  0xc4,0xe2,0x3d,0xa8,0xdf,                   //vfmadd213ps   %ymm7,%ymm8,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_lerp_u8_hsw[] = {
  0x49,0x89,0xc8,                             //mov           %rcx,%r8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x48,0x01,0xf8,                             //add           %rdi,%rax
  0x4d,0x85,0xc0,                             //test          %r8,%r8
  0x75,0x44,                                  //jne           568 <_sk_lerp_u8_hsw+0x54>
  0xc5,0x7b,0x10,0x00,                        //vmovsd        (%rax),%xmm8
  0xc4,0x42,0x7d,0x31,0xc0,                   //vpmovzxbd     %xmm8,%ymm8
  0xc4,0x41,0x7c,0x5b,0xc0,                   //vcvtdq2ps     %ymm8,%ymm8
  0xc4,0x62,0x7d,0x18,0x4a,0x0c,              //vbroadcastss  0xc(%rdx),%ymm9
  0xc4,0x41,0x3c,0x59,0xc1,                   //vmulps        %ymm9,%ymm8,%ymm8
  0xc5,0xfc,0x5c,0xc4,                        //vsubps        %ymm4,%ymm0,%ymm0
  0xc4,0xe2,0x3d,0xa8,0xc4,                   //vfmadd213ps   %ymm4,%ymm8,%ymm0
  0xc5,0xf4,0x5c,0xcd,                        //vsubps        %ymm5,%ymm1,%ymm1
  0xc4,0xe2,0x3d,0xa8,0xcd,                   //vfmadd213ps   %ymm5,%ymm8,%ymm1
  0xc5,0xec,0x5c,0xd6,                        //vsubps        %ymm6,%ymm2,%ymm2
  0xc4,0xe2,0x3d,0xa8,0xd6,                   //vfmadd213ps   %ymm6,%ymm8,%ymm2
  0xc5,0xe4,0x5c,0xdf,                        //vsubps        %ymm7,%ymm3,%ymm3
  0xc4,0xe2,0x3d,0xa8,0xdf,                   //vfmadd213ps   %ymm7,%ymm8,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x89,0xc1,                             //mov           %r8,%rcx
  0xff,0xe0,                                  //jmpq          *%rax
  0x31,0xc9,                                  //xor           %ecx,%ecx
  0x4d,0x89,0xc2,                             //mov           %r8,%r10
  0x45,0x31,0xc9,                             //xor           %r9d,%r9d
  0x44,0x0f,0xb6,0x18,                        //movzbl        (%rax),%r11d
  0x48,0xff,0xc0,                             //inc           %rax
  0x49,0xd3,0xe3,                             //shl           %cl,%r11
  0x4d,0x09,0xd9,                             //or            %r11,%r9
  0x48,0x83,0xc1,0x08,                        //add           $0x8,%rcx
  0x49,0xff,0xca,                             //dec           %r10
  0x75,0xea,                                  //jne           570 <_sk_lerp_u8_hsw+0x5c>
  0xc4,0x41,0xf9,0x6e,0xc1,                   //vmovq         %r9,%xmm8
  0xeb,0x9b,                                  //jmp           528 <_sk_lerp_u8_hsw+0x14>
};

CODE const uint8_t sk_lerp_565_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x8b,0x10,                             //mov           (%rax),%r10
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x75,0x7b,                                  //jne           612 <_sk_lerp_565_hsw+0x85>
  0xc4,0xc1,0x7a,0x6f,0x1c,0x7a,              //vmovdqu       (%r10,%rdi,2),%xmm3
  0xc4,0xe2,0x7d,0x33,0xdb,                   //vpmovzxwd     %xmm3,%ymm3
  0xc4,0x62,0x7d,0x58,0x42,0x68,              //vpbroadcastd  0x68(%rdx),%ymm8
  0xc5,0x3d,0xdb,0xc3,                        //vpand         %ymm3,%ymm8,%ymm8
  0xc4,0x41,0x7c,0x5b,0xc0,                   //vcvtdq2ps     %ymm8,%ymm8
  0xc4,0x62,0x7d,0x18,0x4a,0x74,              //vbroadcastss  0x74(%rdx),%ymm9
  0xc4,0x41,0x34,0x59,0xc0,                   //vmulps        %ymm8,%ymm9,%ymm8
  0xc4,0x62,0x7d,0x58,0x4a,0x6c,              //vpbroadcastd  0x6c(%rdx),%ymm9
  0xc5,0x35,0xdb,0xcb,                        //vpand         %ymm3,%ymm9,%ymm9
  0xc4,0x41,0x7c,0x5b,0xc9,                   //vcvtdq2ps     %ymm9,%ymm9
  0xc4,0x62,0x7d,0x18,0x52,0x78,              //vbroadcastss  0x78(%rdx),%ymm10
  0xc4,0x41,0x2c,0x59,0xc9,                   //vmulps        %ymm9,%ymm10,%ymm9
  0xc4,0x62,0x7d,0x58,0x52,0x70,              //vpbroadcastd  0x70(%rdx),%ymm10
  0xc5,0xad,0xdb,0xdb,                        //vpand         %ymm3,%ymm10,%ymm3
  0xc5,0xfc,0x5b,0xdb,                        //vcvtdq2ps     %ymm3,%ymm3
  0xc4,0x62,0x7d,0x18,0x52,0x7c,              //vbroadcastss  0x7c(%rdx),%ymm10
  0xc5,0xac,0x59,0xdb,                        //vmulps        %ymm3,%ymm10,%ymm3
  0xc5,0xfc,0x5c,0xc4,                        //vsubps        %ymm4,%ymm0,%ymm0
  0xc4,0xe2,0x3d,0xa8,0xc4,                   //vfmadd213ps   %ymm4,%ymm8,%ymm0
  0xc5,0xf4,0x5c,0xcd,                        //vsubps        %ymm5,%ymm1,%ymm1
  0xc4,0xe2,0x35,0xa8,0xcd,                   //vfmadd213ps   %ymm5,%ymm9,%ymm1
  0xc5,0xec,0x5c,0xd6,                        //vsubps        %ymm6,%ymm2,%ymm2
  0xc4,0xe2,0x65,0xa8,0xd6,                   //vfmadd213ps   %ymm6,%ymm3,%ymm2
  0xc4,0xe2,0x7d,0x18,0x1a,                   //vbroadcastss  (%rdx),%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0x41,0x89,0xc8,                             //mov           %ecx,%r8d
  0x41,0x80,0xe0,0x07,                        //and           $0x7,%r8b
  0xc5,0xe1,0xef,0xdb,                        //vpxor         %xmm3,%xmm3,%xmm3
  0x41,0xfe,0xc8,                             //dec           %r8b
  0x45,0x0f,0xb6,0xc0,                        //movzbl        %r8b,%r8d
  0x41,0x80,0xf8,0x06,                        //cmp           $0x6,%r8b
  0x0f,0x87,0x6f,0xff,0xff,0xff,              //ja            59d <_sk_lerp_565_hsw+0x10>
  0x4c,0x8d,0x0d,0x4b,0x00,0x00,0x00,         //lea           0x4b(%rip),%r9        # 680 <_sk_lerp_565_hsw+0xf3>
  0x4b,0x63,0x04,0x81,                        //movslq        (%r9,%r8,4),%rax
  0x4c,0x01,0xc8,                             //add           %r9,%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0xc5,0xe1,0xef,0xdb,                        //vpxor         %xmm3,%xmm3,%xmm3
  0xc4,0xc1,0x61,0xc4,0x5c,0x7a,0x0c,0x06,    //vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm3,%xmm3
  0xc4,0xc1,0x61,0xc4,0x5c,0x7a,0x0a,0x05,    //vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm3,%xmm3
  0xc4,0xc1,0x61,0xc4,0x5c,0x7a,0x08,0x04,    //vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm3,%xmm3
  0xc4,0xc1,0x61,0xc4,0x5c,0x7a,0x06,0x03,    //vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm3,%xmm3
  0xc4,0xc1,0x61,0xc4,0x5c,0x7a,0x04,0x02,    //vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm3,%xmm3
  0xc4,0xc1,0x61,0xc4,0x5c,0x7a,0x02,0x01,    //vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm3,%xmm3
  0xc4,0xc1,0x61,0xc4,0x1c,0x7a,0x00,         //vpinsrw       $0x0,(%r10,%rdi,2),%xmm3,%xmm3
  0xe9,0x1f,0xff,0xff,0xff,                   //jmpq          59d <_sk_lerp_565_hsw+0x10>
  0x66,0x90,                                  //xchg          %ax,%ax
  0xf2,0xff,                                  //repnz         (bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xea,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xe2,                                  //jmpq          *%rdx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xda,0xff,                                  //(bad)
  0xff,                                       //(bad)
  0xff,0xd2,                                  //callq         *%rdx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xca,                                  //dec           %edx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xbe,                                       //.byte         0xbe
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //.byte         0xff
};

CODE const uint8_t sk_load_tables_hsw[] = {
  0x49,0x89,0xc8,                             //mov           %rcx,%r8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x8d,0x0c,0xbd,0x00,0x00,0x00,0x00,    //lea           0x0(,%rdi,4),%r9
  0x4c,0x03,0x08,                             //add           (%rax),%r9
  0x4d,0x85,0xc0,                             //test          %r8,%r8
  0x75,0x6a,                                  //jne           71b <_sk_load_tables_hsw+0x7f>
  0xc4,0xc1,0x7e,0x6f,0x19,                   //vmovdqu       (%r9),%ymm3
  0xc4,0xe2,0x7d,0x58,0x52,0x10,              //vpbroadcastd  0x10(%rdx),%ymm2
  0xc5,0xed,0xdb,0xcb,                        //vpand         %ymm3,%ymm2,%ymm1
  0xc4,0x41,0x3d,0x76,0xc0,                   //vpcmpeqd      %ymm8,%ymm8,%ymm8
  0x48,0x8b,0x48,0x08,                        //mov           0x8(%rax),%rcx
  0x4c,0x8b,0x48,0x10,                        //mov           0x10(%rax),%r9
  0xc4,0x41,0x35,0x76,0xc9,                   //vpcmpeqd      %ymm9,%ymm9,%ymm9
  0xc4,0xe2,0x35,0x92,0x04,0x89,              //vgatherdps    %ymm9,(%rcx,%ymm1,4),%ymm0
  0xc5,0xf5,0x72,0xd3,0x08,                   //vpsrld        $0x8,%ymm3,%ymm1
  0xc5,0x6d,0xdb,0xc9,                        //vpand         %ymm1,%ymm2,%ymm9
  0xc4,0x41,0x2d,0x76,0xd2,                   //vpcmpeqd      %ymm10,%ymm10,%ymm10
  0xc4,0x82,0x2d,0x92,0x0c,0x89,              //vgatherdps    %ymm10,(%r9,%ymm9,4),%ymm1
  0x48,0x8b,0x40,0x18,                        //mov           0x18(%rax),%rax
  0xc5,0xb5,0x72,0xd3,0x10,                   //vpsrld        $0x10,%ymm3,%ymm9
  0xc4,0x41,0x6d,0xdb,0xc9,                   //vpand         %ymm9,%ymm2,%ymm9
  0xc4,0xa2,0x3d,0x92,0x14,0x88,              //vgatherdps    %ymm8,(%rax,%ymm9,4),%ymm2
  0xc5,0xe5,0x72,0xd3,0x18,                   //vpsrld        $0x18,%ymm3,%ymm3
  0xc5,0xfc,0x5b,0xdb,                        //vcvtdq2ps     %ymm3,%ymm3
  0xc4,0x62,0x7d,0x18,0x42,0x0c,              //vbroadcastss  0xc(%rdx),%ymm8
  0xc4,0xc1,0x64,0x59,0xd8,                   //vmulps        %ymm8,%ymm3,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x89,0xc1,                             //mov           %r8,%rcx
  0xff,0xe0,                                  //jmpq          *%rax
  0xb9,0x08,0x00,0x00,0x00,                   //mov           $0x8,%ecx
  0x44,0x29,0xc1,                             //sub           %r8d,%ecx
  0xc0,0xe1,0x03,                             //shl           $0x3,%cl
  0x49,0xc7,0xc2,0xff,0xff,0xff,0xff,         //mov           $0xffffffffffffffff,%r10
  0x49,0xd3,0xea,                             //shr           %cl,%r10
  0xc4,0xc1,0xf9,0x6e,0xc2,                   //vmovq         %r10,%xmm0
  0xc4,0xe2,0x7d,0x21,0xc0,                   //vpmovsxbd     %xmm0,%ymm0
  0xc4,0xc2,0x7d,0x8c,0x19,                   //vpmaskmovd    (%r9),%ymm0,%ymm3
  0xe9,0x72,0xff,0xff,0xff,                   //jmpq          6b6 <_sk_load_tables_hsw+0x1a>
};

CODE const uint8_t sk_load_a8_hsw[] = {
  0x49,0x89,0xc8,                             //mov           %rcx,%r8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x48,0x01,0xf8,                             //add           %rdi,%rax
  0x4d,0x85,0xc0,                             //test          %r8,%r8
  0x75,0x2a,                                  //jne           77e <_sk_load_a8_hsw+0x3a>
  0xc5,0xfb,0x10,0x00,                        //vmovsd        (%rax),%xmm0
  0xc4,0xe2,0x7d,0x31,0xc0,                   //vpmovzxbd     %xmm0,%ymm0
  0xc5,0xfc,0x5b,0xc0,                        //vcvtdq2ps     %ymm0,%ymm0
  0xc4,0xe2,0x7d,0x18,0x4a,0x0c,              //vbroadcastss  0xc(%rdx),%ymm1
  0xc5,0xfc,0x59,0xd9,                        //vmulps        %ymm1,%ymm0,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0xfc,0x57,0xc0,                        //vxorps        %ymm0,%ymm0,%ymm0
  0xc5,0xf4,0x57,0xc9,                        //vxorps        %ymm1,%ymm1,%ymm1
  0xc5,0xec,0x57,0xd2,                        //vxorps        %ymm2,%ymm2,%ymm2
  0x4c,0x89,0xc1,                             //mov           %r8,%rcx
  0xff,0xe0,                                  //jmpq          *%rax
  0x31,0xc9,                                  //xor           %ecx,%ecx
  0x4d,0x89,0xc2,                             //mov           %r8,%r10
  0x45,0x31,0xc9,                             //xor           %r9d,%r9d
  0x44,0x0f,0xb6,0x18,                        //movzbl        (%rax),%r11d
  0x48,0xff,0xc0,                             //inc           %rax
  0x49,0xd3,0xe3,                             //shl           %cl,%r11
  0x4d,0x09,0xd9,                             //or            %r11,%r9
  0x48,0x83,0xc1,0x08,                        //add           $0x8,%rcx
  0x49,0xff,0xca,                             //dec           %r10
  0x75,0xea,                                  //jne           786 <_sk_load_a8_hsw+0x42>
  0xc4,0xc1,0xf9,0x6e,0xc1,                   //vmovq         %r9,%xmm0
  0xeb,0xb5,                                  //jmp           758 <_sk_load_a8_hsw+0x14>
};

CODE const uint8_t sk_store_a8_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x8b,0x08,                             //mov           (%rax),%r9
  0xc4,0x62,0x7d,0x18,0x42,0x08,              //vbroadcastss  0x8(%rdx),%ymm8
  0xc5,0x3c,0x59,0xc3,                        //vmulps        %ymm3,%ymm8,%ymm8
  0xc4,0x41,0x7d,0x5b,0xc0,                   //vcvtps2dq     %ymm8,%ymm8
  0xc4,0x43,0x7d,0x19,0xc1,0x01,              //vextractf128  $0x1,%ymm8,%xmm9
  0xc4,0x42,0x39,0x2b,0xc1,                   //vpackusdw     %xmm9,%xmm8,%xmm8
  0xc4,0x41,0x39,0x67,0xc0,                   //vpackuswb     %xmm8,%xmm8,%xmm8
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x75,0x0a,                                  //jne           7d6 <_sk_store_a8_hsw+0x33>
  0xc4,0x41,0x7b,0x11,0x04,0x39,              //vmovsd        %xmm8,(%r9,%rdi,1)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0x89,0xc8,                                  //mov           %ecx,%eax
  0x24,0x07,                                  //and           $0x7,%al
  0xfe,0xc8,                                  //dec           %al
  0x44,0x0f,0xb6,0xc0,                        //movzbl        %al,%r8d
  0x41,0x80,0xf8,0x06,                        //cmp           $0x6,%r8b
  0x77,0xec,                                  //ja            7d2 <_sk_store_a8_hsw+0x2f>
  0xc4,0x42,0x79,0x30,0xc0,                   //vpmovzxbw     %xmm8,%xmm8
  0x4c,0x8d,0x15,0x42,0x00,0x00,0x00,         //lea           0x42(%rip),%r10        # 834 <_sk_store_a8_hsw+0x91>
  0x4b,0x63,0x04,0x82,                        //movslq        (%r10,%r8,4),%rax
  0x4c,0x01,0xd0,                             //add           %r10,%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0xc4,0x43,0x79,0x14,0x44,0x39,0x06,0x0c,    //vpextrb       $0xc,%xmm8,0x6(%r9,%rdi,1)
  0xc4,0x43,0x79,0x14,0x44,0x39,0x05,0x0a,    //vpextrb       $0xa,%xmm8,0x5(%r9,%rdi,1)
  0xc4,0x43,0x79,0x14,0x44,0x39,0x04,0x08,    //vpextrb       $0x8,%xmm8,0x4(%r9,%rdi,1)
  0xc4,0x43,0x79,0x14,0x44,0x39,0x03,0x06,    //vpextrb       $0x6,%xmm8,0x3(%r9,%rdi,1)
  0xc4,0x43,0x79,0x14,0x44,0x39,0x02,0x04,    //vpextrb       $0x4,%xmm8,0x2(%r9,%rdi,1)
  0xc4,0x43,0x79,0x14,0x44,0x39,0x01,0x02,    //vpextrb       $0x2,%xmm8,0x1(%r9,%rdi,1)
  0xc4,0x43,0x79,0x14,0x04,0x39,0x00,         //vpextrb       $0x0,%xmm8,(%r9,%rdi,1)
  0xeb,0x9e,                                  //jmp           7d2 <_sk_store_a8_hsw+0x2f>
  0xf7,0xff,                                  //idiv          %edi
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xef,                                       //out           %eax,(%dx)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xe7,                                  //jmpq          *%rdi
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xdf,0xff,                                  //(bad)
  0xff,                                       //(bad)
  0xff,0xd7,                                  //callq         *%rdi
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xcf,                                  //dec           %edi
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xc7,                                  //inc           %edi
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //.byte         0xff
};

CODE const uint8_t sk_load_565_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x8b,0x10,                             //mov           (%rax),%r10
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x75,0x5c,                                  //jne           8b6 <_sk_load_565_hsw+0x66>
  0xc4,0xc1,0x7a,0x6f,0x04,0x7a,              //vmovdqu       (%r10,%rdi,2),%xmm0
  0xc4,0xe2,0x7d,0x33,0xd0,                   //vpmovzxwd     %xmm0,%ymm2
  0xc4,0xe2,0x7d,0x58,0x42,0x68,              //vpbroadcastd  0x68(%rdx),%ymm0
  0xc5,0xfd,0xdb,0xc2,                        //vpand         %ymm2,%ymm0,%ymm0
  0xc5,0xfc,0x5b,0xc0,                        //vcvtdq2ps     %ymm0,%ymm0
  0xc4,0xe2,0x7d,0x18,0x4a,0x74,              //vbroadcastss  0x74(%rdx),%ymm1
  0xc5,0xf4,0x59,0xc0,                        //vmulps        %ymm0,%ymm1,%ymm0
  0xc4,0xe2,0x7d,0x58,0x4a,0x6c,              //vpbroadcastd  0x6c(%rdx),%ymm1
  0xc5,0xf5,0xdb,0xca,                        //vpand         %ymm2,%ymm1,%ymm1
  0xc5,0xfc,0x5b,0xc9,                        //vcvtdq2ps     %ymm1,%ymm1
  0xc4,0xe2,0x7d,0x18,0x5a,0x78,              //vbroadcastss  0x78(%rdx),%ymm3
  0xc5,0xe4,0x59,0xc9,                        //vmulps        %ymm1,%ymm3,%ymm1
  0xc4,0xe2,0x7d,0x58,0x5a,0x70,              //vpbroadcastd  0x70(%rdx),%ymm3
  0xc5,0xe5,0xdb,0xd2,                        //vpand         %ymm2,%ymm3,%ymm2
  0xc5,0xfc,0x5b,0xd2,                        //vcvtdq2ps     %ymm2,%ymm2
  0xc4,0xe2,0x7d,0x18,0x5a,0x7c,              //vbroadcastss  0x7c(%rdx),%ymm3
  0xc5,0xe4,0x59,0xd2,                        //vmulps        %ymm2,%ymm3,%ymm2
  0xc4,0xe2,0x7d,0x18,0x1a,                   //vbroadcastss  (%rdx),%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0x41,0x89,0xc8,                             //mov           %ecx,%r8d
  0x41,0x80,0xe0,0x07,                        //and           $0x7,%r8b
  0xc5,0xf9,0xef,0xc0,                        //vpxor         %xmm0,%xmm0,%xmm0
  0x41,0xfe,0xc8,                             //dec           %r8b
  0x45,0x0f,0xb6,0xc0,                        //movzbl        %r8b,%r8d
  0x41,0x80,0xf8,0x06,                        //cmp           $0x6,%r8b
  0x77,0x92,                                  //ja            860 <_sk_load_565_hsw+0x10>
  0x4c,0x8d,0x0d,0x4b,0x00,0x00,0x00,         //lea           0x4b(%rip),%r9        # 920 <_sk_load_565_hsw+0xd0>
  0x4b,0x63,0x04,0x81,                        //movslq        (%r9,%r8,4),%rax
  0x4c,0x01,0xc8,                             //add           %r9,%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0xc5,0xf9,0xef,0xc0,                        //vpxor         %xmm0,%xmm0,%xmm0
  0xc4,0xc1,0x79,0xc4,0x44,0x7a,0x0c,0x06,    //vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm0,%xmm0
  0xc4,0xc1,0x79,0xc4,0x44,0x7a,0x0a,0x05,    //vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm0,%xmm0
  0xc4,0xc1,0x79,0xc4,0x44,0x7a,0x08,0x04,    //vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm0,%xmm0
  0xc4,0xc1,0x79,0xc4,0x44,0x7a,0x06,0x03,    //vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm0,%xmm0
  0xc4,0xc1,0x79,0xc4,0x44,0x7a,0x04,0x02,    //vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm0,%xmm0
  0xc4,0xc1,0x79,0xc4,0x44,0x7a,0x02,0x01,    //vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm0,%xmm0
  0xc4,0xc1,0x79,0xc4,0x04,0x7a,0x00,         //vpinsrw       $0x0,(%r10,%rdi,2),%xmm0,%xmm0
  0xe9,0x42,0xff,0xff,0xff,                   //jmpq          860 <_sk_load_565_hsw+0x10>
  0x66,0x90,                                  //xchg          %ax,%ax
  0xf2,0xff,                                  //repnz         (bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xea,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xe2,                                  //jmpq          *%rdx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xda,0xff,                                  //(bad)
  0xff,                                       //(bad)
  0xff,0xd2,                                  //callq         *%rdx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xca,                                  //dec           %edx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xbe,                                       //.byte         0xbe
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //.byte         0xff
};

CODE const uint8_t sk_store_565_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x8b,0x08,                             //mov           (%rax),%r9
  0xc4,0x62,0x7d,0x18,0x82,0x80,0x00,0x00,0x00,//vbroadcastss  0x80(%rdx),%ymm8
  0xc5,0x3c,0x59,0xc8,                        //vmulps        %ymm0,%ymm8,%ymm9
  0xc4,0x41,0x7d,0x5b,0xc9,                   //vcvtps2dq     %ymm9,%ymm9
  0xc4,0xc1,0x35,0x72,0xf1,0x0b,              //vpslld        $0xb,%ymm9,%ymm9
  0xc4,0x62,0x7d,0x18,0x92,0x84,0x00,0x00,0x00,//vbroadcastss  0x84(%rdx),%ymm10
  0xc5,0x2c,0x59,0xd1,                        //vmulps        %ymm1,%ymm10,%ymm10
  0xc4,0x41,0x7d,0x5b,0xd2,                   //vcvtps2dq     %ymm10,%ymm10
  0xc4,0xc1,0x2d,0x72,0xf2,0x05,              //vpslld        $0x5,%ymm10,%ymm10
  0xc4,0x41,0x2d,0xeb,0xc9,                   //vpor          %ymm9,%ymm10,%ymm9
  0xc5,0x3c,0x59,0xc2,                        //vmulps        %ymm2,%ymm8,%ymm8
  0xc4,0x41,0x7d,0x5b,0xc0,                   //vcvtps2dq     %ymm8,%ymm8
  0xc4,0x41,0x35,0xeb,0xc0,                   //vpor          %ymm8,%ymm9,%ymm8
  0xc4,0x43,0x7d,0x39,0xc1,0x01,              //vextracti128  $0x1,%ymm8,%xmm9
  0xc4,0x42,0x39,0x2b,0xc1,                   //vpackusdw     %xmm9,%xmm8,%xmm8
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x75,0x0a,                                  //jne           99e <_sk_store_565_hsw+0x62>
  0xc4,0x41,0x7a,0x7f,0x04,0x79,              //vmovdqu       %xmm8,(%r9,%rdi,2)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0x89,0xc8,                                  //mov           %ecx,%eax
  0x24,0x07,                                  //and           $0x7,%al
  0xfe,0xc8,                                  //dec           %al
  0x44,0x0f,0xb6,0xc0,                        //movzbl        %al,%r8d
  0x41,0x80,0xf8,0x06,                        //cmp           $0x6,%r8b
  0x77,0xec,                                  //ja            99a <_sk_store_565_hsw+0x5e>
  0x4c,0x8d,0x15,0x47,0x00,0x00,0x00,         //lea           0x47(%rip),%r10        # 9fc <_sk_store_565_hsw+0xc0>
  0x4b,0x63,0x04,0x82,                        //movslq        (%r10,%r8,4),%rax
  0x4c,0x01,0xd0,                             //add           %r10,%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0xc4,0x43,0x79,0x15,0x44,0x79,0x0c,0x06,    //vpextrw       $0x6,%xmm8,0xc(%r9,%rdi,2)
  0xc4,0x43,0x79,0x15,0x44,0x79,0x0a,0x05,    //vpextrw       $0x5,%xmm8,0xa(%r9,%rdi,2)
  0xc4,0x43,0x79,0x15,0x44,0x79,0x08,0x04,    //vpextrw       $0x4,%xmm8,0x8(%r9,%rdi,2)
  0xc4,0x43,0x79,0x15,0x44,0x79,0x06,0x03,    //vpextrw       $0x3,%xmm8,0x6(%r9,%rdi,2)
  0xc4,0x43,0x79,0x15,0x44,0x79,0x04,0x02,    //vpextrw       $0x2,%xmm8,0x4(%r9,%rdi,2)
  0xc4,0x43,0x79,0x15,0x44,0x79,0x02,0x01,    //vpextrw       $0x1,%xmm8,0x2(%r9,%rdi,2)
  0xc5,0x79,0x7e,0xc0,                        //vmovd         %xmm8,%eax
  0x66,0x41,0x89,0x04,0x79,                   //mov           %ax,(%r9,%rdi,2)
  0xeb,0xa1,                                  //jmp           99a <_sk_store_565_hsw+0x5e>
  0x0f,0x1f,0x00,                             //nopl          (%rax)
  0xf2,0xff,                                  //repnz         (bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xea,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xe2,                                  //jmpq          *%rdx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xda,0xff,                                  //(bad)
  0xff,                                       //(bad)
  0xff,0xd2,                                  //callq         *%rdx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xca,                                  //dec           %edx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xc2,                                  //inc           %edx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //.byte         0xff
};

CODE const uint8_t sk_load_8888_hsw[] = {
  0x49,0x89,0xc8,                             //mov           %rcx,%r8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x8d,0x0c,0xbd,0x00,0x00,0x00,0x00,    //lea           0x0(,%rdi,4),%r9
  0x4c,0x03,0x08,                             //add           (%rax),%r9
  0x4d,0x85,0xc0,                             //test          %r8,%r8
  0x75,0x55,                                  //jne           a82 <_sk_load_8888_hsw+0x6a>
  0xc4,0xc1,0x7e,0x6f,0x19,                   //vmovdqu       (%r9),%ymm3
  0xc4,0xe2,0x7d,0x58,0x52,0x10,              //vpbroadcastd  0x10(%rdx),%ymm2
  0xc5,0xed,0xdb,0xc3,                        //vpand         %ymm3,%ymm2,%ymm0
  0xc5,0xfc,0x5b,0xc0,                        //vcvtdq2ps     %ymm0,%ymm0
  0xc4,0x62,0x7d,0x18,0x42,0x0c,              //vbroadcastss  0xc(%rdx),%ymm8
  0xc5,0xbc,0x59,0xc0,                        //vmulps        %ymm0,%ymm8,%ymm0
  0xc5,0xf5,0x72,0xd3,0x08,                   //vpsrld        $0x8,%ymm3,%ymm1
  0xc5,0xed,0xdb,0xc9,                        //vpand         %ymm1,%ymm2,%ymm1
  0xc5,0xfc,0x5b,0xc9,                        //vcvtdq2ps     %ymm1,%ymm1
  0xc5,0xbc,0x59,0xc9,                        //vmulps        %ymm1,%ymm8,%ymm1
  0xc5,0xb5,0x72,0xd3,0x10,                   //vpsrld        $0x10,%ymm3,%ymm9
  0xc4,0xc1,0x6d,0xdb,0xd1,                   //vpand         %ymm9,%ymm2,%ymm2
  0xc5,0xfc,0x5b,0xd2,                        //vcvtdq2ps     %ymm2,%ymm2
  0xc5,0xbc,0x59,0xd2,                        //vmulps        %ymm2,%ymm8,%ymm2
  0xc5,0xe5,0x72,0xd3,0x18,                   //vpsrld        $0x18,%ymm3,%ymm3
  0xc5,0xfc,0x5b,0xdb,                        //vcvtdq2ps     %ymm3,%ymm3
  0xc4,0xc1,0x64,0x59,0xd8,                   //vmulps        %ymm8,%ymm3,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x89,0xc1,                             //mov           %r8,%rcx
  0xff,0xe0,                                  //jmpq          *%rax
  0xb9,0x08,0x00,0x00,0x00,                   //mov           $0x8,%ecx
  0x44,0x29,0xc1,                             //sub           %r8d,%ecx
  0xc0,0xe1,0x03,                             //shl           $0x3,%cl
  0x48,0xc7,0xc0,0xff,0xff,0xff,0xff,         //mov           $0xffffffffffffffff,%rax
  0x48,0xd3,0xe8,                             //shr           %cl,%rax
  0xc4,0xe1,0xf9,0x6e,0xc0,                   //vmovq         %rax,%xmm0
  0xc4,0xe2,0x7d,0x21,0xc0,                   //vpmovsxbd     %xmm0,%ymm0
  0xc4,0xc2,0x7d,0x8c,0x19,                   //vpmaskmovd    (%r9),%ymm0,%ymm3
  0xeb,0x8a,                                  //jmp           a32 <_sk_load_8888_hsw+0x1a>
};

CODE const uint8_t sk_store_8888_hsw[] = {
  0x49,0x89,0xc8,                             //mov           %rcx,%r8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x8d,0x0c,0xbd,0x00,0x00,0x00,0x00,    //lea           0x0(,%rdi,4),%r9
  0x4c,0x03,0x08,                             //add           (%rax),%r9
  0xc4,0x62,0x7d,0x18,0x42,0x08,              //vbroadcastss  0x8(%rdx),%ymm8
  0xc5,0x3c,0x59,0xc8,                        //vmulps        %ymm0,%ymm8,%ymm9
  0xc4,0x41,0x7d,0x5b,0xc9,                   //vcvtps2dq     %ymm9,%ymm9
  0xc5,0x3c,0x59,0xd1,                        //vmulps        %ymm1,%ymm8,%ymm10
  0xc4,0x41,0x7d,0x5b,0xd2,                   //vcvtps2dq     %ymm10,%ymm10
  0xc4,0xc1,0x2d,0x72,0xf2,0x08,              //vpslld        $0x8,%ymm10,%ymm10
  0xc4,0x41,0x2d,0xeb,0xc9,                   //vpor          %ymm9,%ymm10,%ymm9
  0xc5,0x3c,0x59,0xd2,                        //vmulps        %ymm2,%ymm8,%ymm10
  0xc4,0x41,0x7d,0x5b,0xd2,                   //vcvtps2dq     %ymm10,%ymm10
  0xc4,0xc1,0x2d,0x72,0xf2,0x10,              //vpslld        $0x10,%ymm10,%ymm10
  0xc5,0x3c,0x59,0xc3,                        //vmulps        %ymm3,%ymm8,%ymm8
  0xc4,0x41,0x7d,0x5b,0xc0,                   //vcvtps2dq     %ymm8,%ymm8
  0xc4,0xc1,0x3d,0x72,0xf0,0x18,              //vpslld        $0x18,%ymm8,%ymm8
  0xc4,0x41,0x2d,0xeb,0xc0,                   //vpor          %ymm8,%ymm10,%ymm8
  0xc4,0x41,0x35,0xeb,0xc0,                   //vpor          %ymm8,%ymm9,%ymm8
  0x4d,0x85,0xc0,                             //test          %r8,%r8
  0x75,0x0c,                                  //jne           b14 <_sk_store_8888_hsw+0x6c>
  0xc4,0x41,0x7e,0x7f,0x01,                   //vmovdqu       %ymm8,(%r9)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x89,0xc1,                             //mov           %r8,%rcx
  0xff,0xe0,                                  //jmpq          *%rax
  0xb9,0x08,0x00,0x00,0x00,                   //mov           $0x8,%ecx
  0x44,0x29,0xc1,                             //sub           %r8d,%ecx
  0xc0,0xe1,0x03,                             //shl           $0x3,%cl
  0x48,0xc7,0xc0,0xff,0xff,0xff,0xff,         //mov           $0xffffffffffffffff,%rax
  0x48,0xd3,0xe8,                             //shr           %cl,%rax
  0xc4,0x61,0xf9,0x6e,0xc8,                   //vmovq         %rax,%xmm9
  0xc4,0x42,0x7d,0x21,0xc9,                   //vpmovsxbd     %xmm9,%ymm9
  0xc4,0x42,0x35,0x8e,0x01,                   //vpmaskmovd    %ymm8,%ymm9,(%r9)
  0xeb,0xd3,                                  //jmp           b0d <_sk_store_8888_hsw+0x65>
};

CODE const uint8_t sk_load_f16_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x75,0x61,                                  //jne           ba5 <_sk_load_f16_hsw+0x6b>
  0xc5,0xf9,0x10,0x0c,0xf8,                   //vmovupd       (%rax,%rdi,8),%xmm1
  0xc5,0xf9,0x10,0x54,0xf8,0x10,              //vmovupd       0x10(%rax,%rdi,8),%xmm2
  0xc5,0xf9,0x10,0x5c,0xf8,0x20,              //vmovupd       0x20(%rax,%rdi,8),%xmm3
  0xc5,0x79,0x10,0x44,0xf8,0x30,              //vmovupd       0x30(%rax,%rdi,8),%xmm8
  0xc5,0xf1,0x61,0xc2,                        //vpunpcklwd    %xmm2,%xmm1,%xmm0
  0xc5,0xf1,0x69,0xca,                        //vpunpckhwd    %xmm2,%xmm1,%xmm1
  0xc4,0xc1,0x61,0x61,0xd0,                   //vpunpcklwd    %xmm8,%xmm3,%xmm2
  0xc4,0xc1,0x61,0x69,0xd8,                   //vpunpckhwd    %xmm8,%xmm3,%xmm3
  0xc5,0x79,0x61,0xc1,                        //vpunpcklwd    %xmm1,%xmm0,%xmm8
  0xc5,0x79,0x69,0xc9,                        //vpunpckhwd    %xmm1,%xmm0,%xmm9
  0xc5,0xe9,0x61,0xcb,                        //vpunpcklwd    %xmm3,%xmm2,%xmm1
  0xc5,0xe9,0x69,0xdb,                        //vpunpckhwd    %xmm3,%xmm2,%xmm3
  0xc5,0xb9,0x6c,0xc1,                        //vpunpcklqdq   %xmm1,%xmm8,%xmm0
  0xc4,0xe2,0x7d,0x13,0xc0,                   //vcvtph2ps     %xmm0,%ymm0
  0xc5,0xb9,0x6d,0xc9,                        //vpunpckhqdq   %xmm1,%xmm8,%xmm1
  0xc4,0xe2,0x7d,0x13,0xc9,                   //vcvtph2ps     %xmm1,%ymm1
  0xc5,0xb1,0x6c,0xd3,                        //vpunpcklqdq   %xmm3,%xmm9,%xmm2
  0xc4,0xe2,0x7d,0x13,0xd2,                   //vcvtph2ps     %xmm2,%ymm2
  0xc5,0xb1,0x6d,0xdb,                        //vpunpckhqdq   %xmm3,%xmm9,%xmm3
  0xc4,0xe2,0x7d,0x13,0xdb,                   //vcvtph2ps     %xmm3,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0xc5,0xfb,0x10,0x0c,0xf8,                   //vmovsd        (%rax,%rdi,8),%xmm1
  0xc4,0x41,0x39,0x57,0xc0,                   //vxorpd        %xmm8,%xmm8,%xmm8
  0x48,0x83,0xf9,0x01,                        //cmp           $0x1,%rcx
  0x75,0x06,                                  //jne           bbb <_sk_load_f16_hsw+0x81>
  0xc5,0xfa,0x7e,0xc9,                        //vmovq         %xmm1,%xmm1
  0xeb,0x1e,                                  //jmp           bd9 <_sk_load_f16_hsw+0x9f>
  0xc5,0xf1,0x16,0x4c,0xf8,0x08,              //vmovhpd       0x8(%rax,%rdi,8),%xmm1,%xmm1
  0x48,0x83,0xf9,0x03,                        //cmp           $0x3,%rcx
  0x72,0x12,                                  //jb            bd9 <_sk_load_f16_hsw+0x9f>
  0xc5,0xfb,0x10,0x54,0xf8,0x10,              //vmovsd        0x10(%rax,%rdi,8),%xmm2
  0x48,0x83,0xf9,0x03,                        //cmp           $0x3,%rcx
  0x75,0x13,                                  //jne           be6 <_sk_load_f16_hsw+0xac>
  0xc5,0xfa,0x7e,0xd2,                        //vmovq         %xmm2,%xmm2
  0xeb,0x2e,                                  //jmp           c07 <_sk_load_f16_hsw+0xcd>
  0xc5,0xe1,0x57,0xdb,                        //vxorpd        %xmm3,%xmm3,%xmm3
  0xc5,0xe9,0x57,0xd2,                        //vxorpd        %xmm2,%xmm2,%xmm2
  0xe9,0x75,0xff,0xff,0xff,                   //jmpq          b5b <_sk_load_f16_hsw+0x21>
  0xc5,0xe9,0x16,0x54,0xf8,0x18,              //vmovhpd       0x18(%rax,%rdi,8),%xmm2,%xmm2
  0x48,0x83,0xf9,0x05,                        //cmp           $0x5,%rcx
  0x72,0x15,                                  //jb            c07 <_sk_load_f16_hsw+0xcd>
  0xc5,0xfb,0x10,0x5c,0xf8,0x20,              //vmovsd        0x20(%rax,%rdi,8),%xmm3
  0x48,0x83,0xf9,0x05,                        //cmp           $0x5,%rcx
  0x75,0x12,                                  //jne           c10 <_sk_load_f16_hsw+0xd6>
  0xc5,0xfa,0x7e,0xdb,                        //vmovq         %xmm3,%xmm3
  0xe9,0x54,0xff,0xff,0xff,                   //jmpq          b5b <_sk_load_f16_hsw+0x21>
  0xc5,0xe1,0x57,0xdb,                        //vxorpd        %xmm3,%xmm3,%xmm3
  0xe9,0x4b,0xff,0xff,0xff,                   //jmpq          b5b <_sk_load_f16_hsw+0x21>
  0xc5,0xe1,0x16,0x5c,0xf8,0x28,              //vmovhpd       0x28(%rax,%rdi,8),%xmm3,%xmm3
  0x48,0x83,0xf9,0x07,                        //cmp           $0x7,%rcx
  0x0f,0x82,0x3b,0xff,0xff,0xff,              //jb            b5b <_sk_load_f16_hsw+0x21>
  0xc5,0x7b,0x10,0x44,0xf8,0x30,              //vmovsd        0x30(%rax,%rdi,8),%xmm8
  0xe9,0x30,0xff,0xff,0xff,                   //jmpq          b5b <_sk_load_f16_hsw+0x21>
};

CODE const uint8_t sk_store_f16_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0xc4,0xc3,0x7d,0x1d,0xc0,0x04,              //vcvtps2ph     $0x4,%ymm0,%xmm8
  0xc4,0xc3,0x7d,0x1d,0xc9,0x04,              //vcvtps2ph     $0x4,%ymm1,%xmm9
  0xc4,0xc3,0x7d,0x1d,0xd2,0x04,              //vcvtps2ph     $0x4,%ymm2,%xmm10
  0xc4,0xc3,0x7d,0x1d,0xdb,0x04,              //vcvtps2ph     $0x4,%ymm3,%xmm11
  0xc4,0x41,0x39,0x61,0xe1,                   //vpunpcklwd    %xmm9,%xmm8,%xmm12
  0xc4,0x41,0x39,0x69,0xc1,                   //vpunpckhwd    %xmm9,%xmm8,%xmm8
  0xc4,0x41,0x29,0x61,0xcb,                   //vpunpcklwd    %xmm11,%xmm10,%xmm9
  0xc4,0x41,0x29,0x69,0xeb,                   //vpunpckhwd    %xmm11,%xmm10,%xmm13
  0xc4,0x41,0x19,0x62,0xd9,                   //vpunpckldq    %xmm9,%xmm12,%xmm11
  0xc4,0x41,0x19,0x6a,0xd1,                   //vpunpckhdq    %xmm9,%xmm12,%xmm10
  0xc4,0x41,0x39,0x62,0xcd,                   //vpunpckldq    %xmm13,%xmm8,%xmm9
  0xc4,0x41,0x39,0x6a,0xc5,                   //vpunpckhdq    %xmm13,%xmm8,%xmm8
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x75,0x1b,                                  //jne           c90 <_sk_store_f16_hsw+0x65>
  0xc5,0x78,0x11,0x1c,0xf8,                   //vmovups       %xmm11,(%rax,%rdi,8)
  0xc5,0x78,0x11,0x54,0xf8,0x10,              //vmovups       %xmm10,0x10(%rax,%rdi,8)
  0xc5,0x78,0x11,0x4c,0xf8,0x20,              //vmovups       %xmm9,0x20(%rax,%rdi,8)
  0xc5,0x7a,0x7f,0x44,0xf8,0x30,              //vmovdqu       %xmm8,0x30(%rax,%rdi,8)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0xc5,0x79,0xd6,0x1c,0xf8,                   //vmovq         %xmm11,(%rax,%rdi,8)
  0x48,0x83,0xf9,0x01,                        //cmp           $0x1,%rcx
  0x74,0xf1,                                  //je            c8c <_sk_store_f16_hsw+0x61>
  0xc5,0x79,0x17,0x5c,0xf8,0x08,              //vmovhpd       %xmm11,0x8(%rax,%rdi,8)
  0x48,0x83,0xf9,0x03,                        //cmp           $0x3,%rcx
  0x72,0xe5,                                  //jb            c8c <_sk_store_f16_hsw+0x61>
  0xc5,0x79,0xd6,0x54,0xf8,0x10,              //vmovq         %xmm10,0x10(%rax,%rdi,8)
  0x74,0xdd,                                  //je            c8c <_sk_store_f16_hsw+0x61>
  0xc5,0x79,0x17,0x54,0xf8,0x18,              //vmovhpd       %xmm10,0x18(%rax,%rdi,8)
  0x48,0x83,0xf9,0x05,                        //cmp           $0x5,%rcx
  0x72,0xd1,                                  //jb            c8c <_sk_store_f16_hsw+0x61>
  0xc5,0x79,0xd6,0x4c,0xf8,0x20,              //vmovq         %xmm9,0x20(%rax,%rdi,8)
  0x74,0xc9,                                  //je            c8c <_sk_store_f16_hsw+0x61>
  0xc5,0x79,0x17,0x4c,0xf8,0x28,              //vmovhpd       %xmm9,0x28(%rax,%rdi,8)
  0x48,0x83,0xf9,0x07,                        //cmp           $0x7,%rcx
  0x72,0xbd,                                  //jb            c8c <_sk_store_f16_hsw+0x61>
  0xc5,0x79,0xd6,0x44,0xf8,0x30,              //vmovq         %xmm8,0x30(%rax,%rdi,8)
  0xeb,0xb5,                                  //jmp           c8c <_sk_store_f16_hsw+0x61>
};

CODE const uint8_t sk_store_f32_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x8b,0x00,                             //mov           (%rax),%r8
  0x48,0x8d,0x04,0xbd,0x00,0x00,0x00,0x00,    //lea           0x0(,%rdi,4),%rax
  0xc5,0x7c,0x14,0xc1,                        //vunpcklps     %ymm1,%ymm0,%ymm8
  0xc5,0x7c,0x15,0xd9,                        //vunpckhps     %ymm1,%ymm0,%ymm11
  0xc5,0x6c,0x14,0xcb,                        //vunpcklps     %ymm3,%ymm2,%ymm9
  0xc5,0x6c,0x15,0xe3,                        //vunpckhps     %ymm3,%ymm2,%ymm12
  0xc4,0x41,0x3d,0x14,0xd1,                   //vunpcklpd     %ymm9,%ymm8,%ymm10
  0xc4,0x41,0x3d,0x15,0xc9,                   //vunpckhpd     %ymm9,%ymm8,%ymm9
  0xc4,0x41,0x25,0x14,0xc4,                   //vunpcklpd     %ymm12,%ymm11,%ymm8
  0xc4,0x41,0x25,0x15,0xdc,                   //vunpckhpd     %ymm12,%ymm11,%ymm11
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x75,0x37,                                  //jne           d44 <_sk_store_f32_hsw+0x6d>
  0xc4,0x43,0x2d,0x18,0xe1,0x01,              //vinsertf128   $0x1,%xmm9,%ymm10,%ymm12
  0xc4,0x43,0x3d,0x18,0xeb,0x01,              //vinsertf128   $0x1,%xmm11,%ymm8,%ymm13
  0xc4,0x43,0x2d,0x06,0xc9,0x31,              //vperm2f128    $0x31,%ymm9,%ymm10,%ymm9
  0xc4,0x43,0x3d,0x06,0xc3,0x31,              //vperm2f128    $0x31,%ymm11,%ymm8,%ymm8
  0xc4,0x41,0x7d,0x11,0x24,0x80,              //vmovupd       %ymm12,(%r8,%rax,4)
  0xc4,0x41,0x7d,0x11,0x6c,0x80,0x20,         //vmovupd       %ymm13,0x20(%r8,%rax,4)
  0xc4,0x41,0x7d,0x11,0x4c,0x80,0x40,         //vmovupd       %ymm9,0x40(%r8,%rax,4)
  0xc4,0x41,0x7d,0x11,0x44,0x80,0x60,         //vmovupd       %ymm8,0x60(%r8,%rax,4)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0xc4,0x41,0x79,0x11,0x14,0x80,              //vmovupd       %xmm10,(%r8,%rax,4)
  0x48,0x83,0xf9,0x01,                        //cmp           $0x1,%rcx
  0x74,0xf0,                                  //je            d40 <_sk_store_f32_hsw+0x69>
  0xc4,0x41,0x79,0x11,0x4c,0x80,0x10,         //vmovupd       %xmm9,0x10(%r8,%rax,4)
  0x48,0x83,0xf9,0x03,                        //cmp           $0x3,%rcx
  0x72,0xe3,                                  //jb            d40 <_sk_store_f32_hsw+0x69>
  0xc4,0x41,0x79,0x11,0x44,0x80,0x20,         //vmovupd       %xmm8,0x20(%r8,%rax,4)
  0x74,0xda,                                  //je            d40 <_sk_store_f32_hsw+0x69>
  0xc4,0x41,0x79,0x11,0x5c,0x80,0x30,         //vmovupd       %xmm11,0x30(%r8,%rax,4)
  0x48,0x83,0xf9,0x05,                        //cmp           $0x5,%rcx
  0x72,0xcd,                                  //jb            d40 <_sk_store_f32_hsw+0x69>
  0xc4,0x43,0x7d,0x19,0x54,0x80,0x40,0x01,    //vextractf128  $0x1,%ymm10,0x40(%r8,%rax,4)
  0x74,0xc3,                                  //je            d40 <_sk_store_f32_hsw+0x69>
  0xc4,0x43,0x7d,0x19,0x4c,0x80,0x50,0x01,    //vextractf128  $0x1,%ymm9,0x50(%r8,%rax,4)
  0x48,0x83,0xf9,0x07,                        //cmp           $0x7,%rcx
  0x72,0xb5,                                  //jb            d40 <_sk_store_f32_hsw+0x69>
  0xc4,0x43,0x7d,0x19,0x44,0x80,0x60,0x01,    //vextractf128  $0x1,%ymm8,0x60(%r8,%rax,4)
  0xeb,0xab,                                  //jmp           d40 <_sk_store_f32_hsw+0x69>
};

CODE const uint8_t sk_clamp_x_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x41,0x3c,0x57,0xc0,                   //vxorps        %ymm8,%ymm8,%ymm8
  0xc5,0xbc,0x5f,0xc0,                        //vmaxps        %ymm0,%ymm8,%ymm0
  0xc4,0x62,0x7d,0x58,0x00,                   //vpbroadcastd  (%rax),%ymm8
  0xc4,0x41,0x35,0x76,0xc9,                   //vpcmpeqd      %ymm9,%ymm9,%ymm9
  0xc4,0x41,0x3d,0xfe,0xc1,                   //vpaddd        %ymm9,%ymm8,%ymm8
  0xc4,0xc1,0x7c,0x5d,0xc0,                   //vminps        %ymm8,%ymm0,%ymm0
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_y_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x41,0x3c,0x57,0xc0,                   //vxorps        %ymm8,%ymm8,%ymm8
  0xc5,0xbc,0x5f,0xc9,                        //vmaxps        %ymm1,%ymm8,%ymm1
  0xc4,0x62,0x7d,0x58,0x00,                   //vpbroadcastd  (%rax),%ymm8
  0xc4,0x41,0x35,0x76,0xc9,                   //vpcmpeqd      %ymm9,%ymm9,%ymm9
  0xc4,0x41,0x3d,0xfe,0xc1,                   //vpaddd        %ymm9,%ymm8,%ymm8
  0xc4,0xc1,0x74,0x5d,0xc8,                   //vminps        %ymm8,%ymm1,%ymm1
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_repeat_x_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x62,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm8
  0xc4,0x41,0x7c,0x5e,0xc8,                   //vdivps        %ymm8,%ymm0,%ymm9
  0xc4,0x43,0x7d,0x08,0xc9,0x01,              //vroundps      $0x1,%ymm9,%ymm9
  0xc4,0x62,0x3d,0xac,0xc8,                   //vfnmadd213ps  %ymm0,%ymm8,%ymm9
  0xc5,0xfd,0x76,0xc0,                        //vpcmpeqd      %ymm0,%ymm0,%ymm0
  0xc5,0xbd,0xfe,0xc0,                        //vpaddd        %ymm0,%ymm8,%ymm0
  0xc5,0xb4,0x5d,0xc0,                        //vminps        %ymm0,%ymm9,%ymm0
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_repeat_y_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x62,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm8
  0xc4,0x41,0x74,0x5e,0xc8,                   //vdivps        %ymm8,%ymm1,%ymm9
  0xc4,0x43,0x7d,0x08,0xc9,0x01,              //vroundps      $0x1,%ymm9,%ymm9
  0xc4,0x62,0x3d,0xac,0xc9,                   //vfnmadd213ps  %ymm1,%ymm8,%ymm9
  0xc5,0xf5,0x76,0xc9,                        //vpcmpeqd      %ymm1,%ymm1,%ymm1
  0xc5,0xbd,0xfe,0xc9,                        //vpaddd        %ymm1,%ymm8,%ymm1
  0xc5,0xb4,0x5d,0xc9,                        //vminps        %ymm1,%ymm9,%ymm1
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_mirror_x_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0x7a,0x10,0x00,                        //vmovss        (%rax),%xmm8
  0xc4,0x42,0x7d,0x18,0xc8,                   //vbroadcastss  %xmm8,%ymm9
  0xc4,0x41,0x7c,0x5c,0xd1,                   //vsubps        %ymm9,%ymm0,%ymm10
  0xc4,0xc1,0x3a,0x58,0xc0,                   //vaddss        %xmm8,%xmm8,%xmm0
  0xc4,0xe2,0x7d,0x18,0xc0,                   //vbroadcastss  %xmm0,%ymm0
  0xc5,0x2c,0x5e,0xc0,                        //vdivps        %ymm0,%ymm10,%ymm8
  0xc4,0x43,0x7d,0x08,0xc0,0x01,              //vroundps      $0x1,%ymm8,%ymm8
  0xc4,0x42,0x7d,0xac,0xc2,                   //vfnmadd213ps  %ymm10,%ymm0,%ymm8
  0xc4,0xc1,0x3c,0x5c,0xc1,                   //vsubps        %ymm9,%ymm8,%ymm0
  0xc4,0x41,0x3c,0x57,0xc0,                   //vxorps        %ymm8,%ymm8,%ymm8
  0xc5,0x3c,0x5c,0xc0,                        //vsubps        %ymm0,%ymm8,%ymm8
  0xc5,0xbc,0x54,0xc0,                        //vandps        %ymm0,%ymm8,%ymm0
  0xc4,0x41,0x3d,0x76,0xc0,                   //vpcmpeqd      %ymm8,%ymm8,%ymm8
  0xc4,0x41,0x35,0xfe,0xc0,                   //vpaddd        %ymm8,%ymm9,%ymm8
  0xc4,0xc1,0x7c,0x5d,0xc0,                   //vminps        %ymm8,%ymm0,%ymm0
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_mirror_y_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0x7a,0x10,0x00,                        //vmovss        (%rax),%xmm8
  0xc4,0x42,0x7d,0x18,0xc8,                   //vbroadcastss  %xmm8,%ymm9
  0xc4,0x41,0x74,0x5c,0xd1,                   //vsubps        %ymm9,%ymm1,%ymm10
  0xc4,0xc1,0x3a,0x58,0xc8,                   //vaddss        %xmm8,%xmm8,%xmm1
  0xc4,0xe2,0x7d,0x18,0xc9,                   //vbroadcastss  %xmm1,%ymm1
  0xc5,0x2c,0x5e,0xc1,                        //vdivps        %ymm1,%ymm10,%ymm8
  0xc4,0x43,0x7d,0x08,0xc0,0x01,              //vroundps      $0x1,%ymm8,%ymm8
  0xc4,0x42,0x75,0xac,0xc2,                   //vfnmadd213ps  %ymm10,%ymm1,%ymm8
  0xc4,0xc1,0x3c,0x5c,0xc9,                   //vsubps        %ymm9,%ymm8,%ymm1
  0xc4,0x41,0x3c,0x57,0xc0,                   //vxorps        %ymm8,%ymm8,%ymm8
  0xc5,0x3c,0x5c,0xc1,                        //vsubps        %ymm1,%ymm8,%ymm8
  0xc5,0xbc,0x54,0xc9,                        //vandps        %ymm1,%ymm8,%ymm1
  0xc4,0x41,0x3d,0x76,0xc0,                   //vpcmpeqd      %ymm8,%ymm8,%ymm8
  0xc4,0x41,0x35,0xfe,0xc0,                   //vpaddd        %ymm8,%ymm9,%ymm8
  0xc4,0xc1,0x74,0x5d,0xc8,                   //vminps        %ymm8,%ymm1,%ymm1
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_matrix_2x3_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x62,0x7d,0x18,0x08,                   //vbroadcastss  (%rax),%ymm9
  0xc4,0x62,0x7d,0x18,0x50,0x08,              //vbroadcastss  0x8(%rax),%ymm10
  0xc4,0x62,0x7d,0x18,0x40,0x10,              //vbroadcastss  0x10(%rax),%ymm8
  0xc4,0x42,0x75,0xb8,0xc2,                   //vfmadd231ps   %ymm10,%ymm1,%ymm8
  0xc4,0x42,0x7d,0xb8,0xc1,                   //vfmadd231ps   %ymm9,%ymm0,%ymm8
  0xc4,0x62,0x7d,0x18,0x50,0x04,              //vbroadcastss  0x4(%rax),%ymm10
  0xc4,0x62,0x7d,0x18,0x58,0x0c,              //vbroadcastss  0xc(%rax),%ymm11
  0xc4,0x62,0x7d,0x18,0x48,0x14,              //vbroadcastss  0x14(%rax),%ymm9
  0xc4,0x42,0x75,0xb8,0xcb,                   //vfmadd231ps   %ymm11,%ymm1,%ymm9
  0xc4,0x42,0x7d,0xb8,0xca,                   //vfmadd231ps   %ymm10,%ymm0,%ymm9
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0x7c,0x29,0xc0,                        //vmovaps       %ymm8,%ymm0
  0xc5,0x7c,0x29,0xc9,                        //vmovaps       %ymm9,%ymm1
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_matrix_3x4_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x62,0x7d,0x18,0x08,                   //vbroadcastss  (%rax),%ymm9
  0xc4,0x62,0x7d,0x18,0x50,0x0c,              //vbroadcastss  0xc(%rax),%ymm10
  0xc4,0x62,0x7d,0x18,0x58,0x18,              //vbroadcastss  0x18(%rax),%ymm11
  0xc4,0x62,0x7d,0x18,0x40,0x24,              //vbroadcastss  0x24(%rax),%ymm8
  0xc4,0x42,0x6d,0xb8,0xc3,                   //vfmadd231ps   %ymm11,%ymm2,%ymm8
  0xc4,0x42,0x75,0xb8,0xc2,                   //vfmadd231ps   %ymm10,%ymm1,%ymm8
  0xc4,0x42,0x7d,0xb8,0xc1,                   //vfmadd231ps   %ymm9,%ymm0,%ymm8
  0xc4,0x62,0x7d,0x18,0x50,0x04,              //vbroadcastss  0x4(%rax),%ymm10
  0xc4,0x62,0x7d,0x18,0x58,0x10,              //vbroadcastss  0x10(%rax),%ymm11
  0xc4,0x62,0x7d,0x18,0x60,0x1c,              //vbroadcastss  0x1c(%rax),%ymm12
  0xc4,0x62,0x7d,0x18,0x48,0x28,              //vbroadcastss  0x28(%rax),%ymm9
  0xc4,0x42,0x6d,0xb8,0xcc,                   //vfmadd231ps   %ymm12,%ymm2,%ymm9
  0xc4,0x42,0x75,0xb8,0xcb,                   //vfmadd231ps   %ymm11,%ymm1,%ymm9
  0xc4,0x42,0x7d,0xb8,0xca,                   //vfmadd231ps   %ymm10,%ymm0,%ymm9
  0xc4,0x62,0x7d,0x18,0x58,0x08,              //vbroadcastss  0x8(%rax),%ymm11
  0xc4,0x62,0x7d,0x18,0x60,0x14,              //vbroadcastss  0x14(%rax),%ymm12
  0xc4,0x62,0x7d,0x18,0x68,0x20,              //vbroadcastss  0x20(%rax),%ymm13
  0xc4,0x62,0x7d,0x18,0x50,0x2c,              //vbroadcastss  0x2c(%rax),%ymm10
  0xc4,0x42,0x6d,0xb8,0xd5,                   //vfmadd231ps   %ymm13,%ymm2,%ymm10
  0xc4,0x42,0x75,0xb8,0xd4,                   //vfmadd231ps   %ymm12,%ymm1,%ymm10
  0xc4,0x42,0x7d,0xb8,0xd3,                   //vfmadd231ps   %ymm11,%ymm0,%ymm10
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0x7c,0x29,0xc0,                        //vmovaps       %ymm8,%ymm0
  0xc5,0x7c,0x29,0xc9,                        //vmovaps       %ymm9,%ymm1
  0xc5,0x7c,0x29,0xd2,                        //vmovaps       %ymm10,%ymm2
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_matrix_perspective_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x62,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm8
  0xc4,0x62,0x7d,0x18,0x48,0x04,              //vbroadcastss  0x4(%rax),%ymm9
  0xc4,0x62,0x7d,0x18,0x50,0x08,              //vbroadcastss  0x8(%rax),%ymm10
  0xc4,0x42,0x75,0xb8,0xd1,                   //vfmadd231ps   %ymm9,%ymm1,%ymm10
  0xc4,0x42,0x7d,0xb8,0xd0,                   //vfmadd231ps   %ymm8,%ymm0,%ymm10
  0xc4,0x62,0x7d,0x18,0x40,0x0c,              //vbroadcastss  0xc(%rax),%ymm8
  0xc4,0x62,0x7d,0x18,0x48,0x10,              //vbroadcastss  0x10(%rax),%ymm9
  0xc4,0x62,0x7d,0x18,0x58,0x14,              //vbroadcastss  0x14(%rax),%ymm11
  0xc4,0x42,0x75,0xb8,0xd9,                   //vfmadd231ps   %ymm9,%ymm1,%ymm11
  0xc4,0x42,0x7d,0xb8,0xd8,                   //vfmadd231ps   %ymm8,%ymm0,%ymm11
  0xc4,0x62,0x7d,0x18,0x40,0x18,              //vbroadcastss  0x18(%rax),%ymm8
  0xc4,0x62,0x7d,0x18,0x48,0x1c,              //vbroadcastss  0x1c(%rax),%ymm9
  0xc4,0x62,0x7d,0x18,0x60,0x20,              //vbroadcastss  0x20(%rax),%ymm12
  0xc4,0x42,0x75,0xb8,0xe1,                   //vfmadd231ps   %ymm9,%ymm1,%ymm12
  0xc4,0x42,0x7d,0xb8,0xe0,                   //vfmadd231ps   %ymm8,%ymm0,%ymm12
  0xc4,0xc1,0x7c,0x53,0xcc,                   //vrcpps        %ymm12,%ymm1
  0xc5,0xac,0x59,0xc1,                        //vmulps        %ymm1,%ymm10,%ymm0
  0xc5,0xa4,0x59,0xc9,                        //vmulps        %ymm1,%ymm11,%ymm1
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_linear_gradient_2stops_hsw[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0xe2,0x7d,0x18,0x48,0x10,              //vbroadcastss  0x10(%rax),%ymm1
  0xc4,0x62,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm8
  0xc4,0x62,0x7d,0xb8,0xc1,                   //vfmadd231ps   %ymm1,%ymm0,%ymm8
  0xc4,0xe2,0x7d,0x18,0x50,0x14,              //vbroadcastss  0x14(%rax),%ymm2
  0xc4,0xe2,0x7d,0x18,0x48,0x04,              //vbroadcastss  0x4(%rax),%ymm1
  0xc4,0xe2,0x7d,0xb8,0xca,                   //vfmadd231ps   %ymm2,%ymm0,%ymm1
  0xc4,0xe2,0x7d,0x18,0x58,0x18,              //vbroadcastss  0x18(%rax),%ymm3
  0xc4,0xe2,0x7d,0x18,0x50,0x08,              //vbroadcastss  0x8(%rax),%ymm2
  0xc4,0xe2,0x7d,0xb8,0xd3,                   //vfmadd231ps   %ymm3,%ymm0,%ymm2
  0xc4,0x62,0x7d,0x18,0x48,0x1c,              //vbroadcastss  0x1c(%rax),%ymm9
  0xc4,0xe2,0x7d,0x18,0x58,0x0c,              //vbroadcastss  0xc(%rax),%ymm3
  0xc4,0xc2,0x7d,0xb8,0xd9,                   //vfmadd231ps   %ymm9,%ymm0,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0x7c,0x29,0xc0,                        //vmovaps       %ymm8,%ymm0
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_start_pipeline_avx[] = {
  0x41,0x57,                                  //push          %r15
  0x41,0x56,                                  //push          %r14
  0x41,0x55,                                  //push          %r13
  0x41,0x54,                                  //push          %r12
  0x53,                                       //push          %rbx
  0x49,0x89,0xcd,                             //mov           %rcx,%r13
  0x49,0x89,0xd6,                             //mov           %rdx,%r14
  0x48,0x89,0xfb,                             //mov           %rdi,%rbx
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x49,0x89,0xc7,                             //mov           %rax,%r15
  0x49,0x89,0xf4,                             //mov           %rsi,%r12
  0x48,0x8d,0x43,0x08,                        //lea           0x8(%rbx),%rax
  0x4c,0x39,0xe8,                             //cmp           %r13,%rax
  0x76,0x05,                                  //jbe           28 <_sk_start_pipeline_avx+0x28>
  0x48,0x89,0xdf,                             //mov           %rbx,%rdi
  0xeb,0x41,                                  //jmp           69 <_sk_start_pipeline_avx+0x69>
  0xb9,0x00,0x00,0x00,0x00,                   //mov           $0x0,%ecx
  0xc5,0xfc,0x57,0xc0,                        //vxorps        %ymm0,%ymm0,%ymm0
  0xc5,0xf4,0x57,0xc9,                        //vxorps        %ymm1,%ymm1,%ymm1
  0xc5,0xec,0x57,0xd2,                        //vxorps        %ymm2,%ymm2,%ymm2
  0xc5,0xe4,0x57,0xdb,                        //vxorps        %ymm3,%ymm3,%ymm3
  0xc5,0xdc,0x57,0xe4,                        //vxorps        %ymm4,%ymm4,%ymm4
  0xc5,0xd4,0x57,0xed,                        //vxorps        %ymm5,%ymm5,%ymm5
  0xc5,0xcc,0x57,0xf6,                        //vxorps        %ymm6,%ymm6,%ymm6
  0xc5,0xc4,0x57,0xff,                        //vxorps        %ymm7,%ymm7,%ymm7
  0x48,0x89,0xdf,                             //mov           %rbx,%rdi
  0x4c,0x89,0xe6,                             //mov           %r12,%rsi
  0x4c,0x89,0xf2,                             //mov           %r14,%rdx
  0x41,0xff,0xd7,                             //callq         *%r15
  0x48,0x8d,0x7b,0x08,                        //lea           0x8(%rbx),%rdi
  0x48,0x83,0xc3,0x10,                        //add           $0x10,%rbx
  0x4c,0x39,0xeb,                             //cmp           %r13,%rbx
  0x48,0x89,0xfb,                             //mov           %rdi,%rbx
  0x76,0xbf,                                  //jbe           28 <_sk_start_pipeline_avx+0x28>
  0x4c,0x89,0xe9,                             //mov           %r13,%rcx
  0x48,0x29,0xf9,                             //sub           %rdi,%rcx
  0x74,0x29,                                  //je            9a <_sk_start_pipeline_avx+0x9a>
  0xc5,0xfc,0x57,0xc0,                        //vxorps        %ymm0,%ymm0,%ymm0
  0xc5,0xf4,0x57,0xc9,                        //vxorps        %ymm1,%ymm1,%ymm1
  0xc5,0xec,0x57,0xd2,                        //vxorps        %ymm2,%ymm2,%ymm2
  0xc5,0xe4,0x57,0xdb,                        //vxorps        %ymm3,%ymm3,%ymm3
  0xc5,0xdc,0x57,0xe4,                        //vxorps        %ymm4,%ymm4,%ymm4
  0xc5,0xd4,0x57,0xed,                        //vxorps        %ymm5,%ymm5,%ymm5
  0xc5,0xcc,0x57,0xf6,                        //vxorps        %ymm6,%ymm6,%ymm6
  0xc5,0xc4,0x57,0xff,                        //vxorps        %ymm7,%ymm7,%ymm7
  0x4c,0x89,0xe6,                             //mov           %r12,%rsi
  0x4c,0x89,0xf2,                             //mov           %r14,%rdx
  0x41,0xff,0xd7,                             //callq         *%r15
  0x4c,0x89,0xe8,                             //mov           %r13,%rax
  0x5b,                                       //pop           %rbx
  0x41,0x5c,                                  //pop           %r12
  0x41,0x5d,                                  //pop           %r13
  0x41,0x5e,                                  //pop           %r14
  0x41,0x5f,                                  //pop           %r15
  0xc5,0xf8,0x77,                             //vzeroupper
  0xc3,                                       //retq
};

CODE const uint8_t sk_start_pipeline_ms_avx[] = {
  0x56,                                       //push          %rsi
  0x57,                                       //push          %rdi
  0x48,0x81,0xec,0xa8,0x00,0x00,0x00,         //sub           $0xa8,%rsp
  0xc5,0x78,0x29,0xbc,0x24,0x90,0x00,0x00,0x00,//vmovaps       %xmm15,0x90(%rsp)
  0xc5,0x78,0x29,0xb4,0x24,0x80,0x00,0x00,0x00,//vmovaps       %xmm14,0x80(%rsp)
  0xc5,0x78,0x29,0x6c,0x24,0x70,              //vmovaps       %xmm13,0x70(%rsp)
  0xc5,0x78,0x29,0x64,0x24,0x60,              //vmovaps       %xmm12,0x60(%rsp)
  0xc5,0x78,0x29,0x5c,0x24,0x50,              //vmovaps       %xmm11,0x50(%rsp)
  0xc5,0x78,0x29,0x54,0x24,0x40,              //vmovaps       %xmm10,0x40(%rsp)
  0xc5,0x78,0x29,0x4c,0x24,0x30,              //vmovaps       %xmm9,0x30(%rsp)
  0xc5,0x78,0x29,0x44,0x24,0x20,              //vmovaps       %xmm8,0x20(%rsp)
  0xc5,0xf8,0x29,0x7c,0x24,0x10,              //vmovaps       %xmm7,0x10(%rsp)
  0xc5,0xf8,0x29,0x34,0x24,                   //vmovaps       %xmm6,(%rsp)
  0x48,0x89,0xcf,                             //mov           %rcx,%rdi
  0x48,0x89,0xd6,                             //mov           %rdx,%rsi
  0x4c,0x89,0xc2,                             //mov           %r8,%rdx
  0x4c,0x89,0xc9,                             //mov           %r9,%rcx
  0xe8,0x00,0x00,0x00,0x00,                   //callq         105 <_sk_start_pipeline_ms_avx+0x5b>
  0xc5,0xf8,0x28,0x34,0x24,                   //vmovaps       (%rsp),%xmm6
  0xc5,0xf8,0x28,0x7c,0x24,0x10,              //vmovaps       0x10(%rsp),%xmm7
  0xc5,0x78,0x28,0x44,0x24,0x20,              //vmovaps       0x20(%rsp),%xmm8
  0xc5,0x78,0x28,0x4c,0x24,0x30,              //vmovaps       0x30(%rsp),%xmm9
  0xc5,0x78,0x28,0x54,0x24,0x40,              //vmovaps       0x40(%rsp),%xmm10
  0xc5,0x78,0x28,0x5c,0x24,0x50,              //vmovaps       0x50(%rsp),%xmm11
  0xc5,0x78,0x28,0x64,0x24,0x60,              //vmovaps       0x60(%rsp),%xmm12
  0xc5,0x78,0x28,0x6c,0x24,0x70,              //vmovaps       0x70(%rsp),%xmm13
  0xc5,0x78,0x28,0xb4,0x24,0x80,0x00,0x00,0x00,//vmovaps       0x80(%rsp),%xmm14
  0xc5,0x78,0x28,0xbc,0x24,0x90,0x00,0x00,0x00,//vmovaps       0x90(%rsp),%xmm15
  0x48,0x81,0xc4,0xa8,0x00,0x00,0x00,         //add           $0xa8,%rsp
  0x5f,                                       //pop           %rdi
  0x5e,                                       //pop           %rsi
  0xc3,                                       //retq
};

CODE const uint8_t sk_just_return_avx[] = {
  0xc3,                                       //retq
};

CODE const uint8_t sk_seed_shader_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0xf9,0x6e,0xc7,                        //vmovd         %edi,%xmm0
  0xc5,0xf9,0x70,0xc0,0x00,                   //vpshufd       $0x0,%xmm0,%xmm0
  0xc4,0xe3,0x7d,0x18,0xc0,0x01,              //vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  0xc5,0xfc,0x5b,0xc0,                        //vcvtdq2ps     %ymm0,%ymm0
  0xc4,0xe2,0x7d,0x18,0x4a,0x04,              //vbroadcastss  0x4(%rdx),%ymm1
  0xc5,0xfc,0x58,0xc1,                        //vaddps        %ymm1,%ymm0,%ymm0
  0xc5,0xfc,0x58,0x42,0x14,                   //vaddps        0x14(%rdx),%ymm0,%ymm0
  0xc4,0xe2,0x7d,0x18,0x10,                   //vbroadcastss  (%rax),%ymm2
  0xc5,0xfc,0x5b,0xd2,                        //vcvtdq2ps     %ymm2,%ymm2
  0xc5,0xec,0x58,0xc9,                        //vaddps        %ymm1,%ymm2,%ymm1
  0xc4,0xe2,0x7d,0x18,0x12,                   //vbroadcastss  (%rdx),%ymm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0xe4,0x57,0xdb,                        //vxorps        %ymm3,%ymm3,%ymm3
  0xc5,0xdc,0x57,0xe4,                        //vxorps        %ymm4,%ymm4,%ymm4
  0xc5,0xd4,0x57,0xed,                        //vxorps        %ymm5,%ymm5,%ymm5
  0xc5,0xcc,0x57,0xf6,                        //vxorps        %ymm6,%ymm6,%ymm6
  0xc5,0xc4,0x57,0xff,                        //vxorps        %ymm7,%ymm7,%ymm7
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_constant_color_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0xe2,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm0
  0xc4,0xe2,0x7d,0x18,0x48,0x04,              //vbroadcastss  0x4(%rax),%ymm1
  0xc4,0xe2,0x7d,0x18,0x50,0x08,              //vbroadcastss  0x8(%rax),%ymm2
  0xc4,0xe2,0x7d,0x18,0x58,0x0c,              //vbroadcastss  0xc(%rax),%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clear_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0xfc,0x57,0xc0,                        //vxorps        %ymm0,%ymm0,%ymm0
  0xc5,0xf4,0x57,0xc9,                        //vxorps        %ymm1,%ymm1,%ymm1
  0xc5,0xec,0x57,0xd2,                        //vxorps        %ymm2,%ymm2,%ymm2
  0xc5,0xe4,0x57,0xdb,                        //vxorps        %ymm3,%ymm3,%ymm3
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_plus__avx[] = {
  0xc5,0xfc,0x58,0xc4,                        //vaddps        %ymm4,%ymm0,%ymm0
  0xc5,0xf4,0x58,0xcd,                        //vaddps        %ymm5,%ymm1,%ymm1
  0xc5,0xec,0x58,0xd6,                        //vaddps        %ymm6,%ymm2,%ymm2
  0xc5,0xe4,0x58,0xdf,                        //vaddps        %ymm7,%ymm3,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_srcover_avx[] = {
  0xc4,0x62,0x7d,0x18,0x02,                   //vbroadcastss  (%rdx),%ymm8
  0xc5,0x3c,0x5c,0xc3,                        //vsubps        %ymm3,%ymm8,%ymm8
  0xc5,0x3c,0x59,0xcc,                        //vmulps        %ymm4,%ymm8,%ymm9
  0xc5,0xb4,0x58,0xc0,                        //vaddps        %ymm0,%ymm9,%ymm0
  0xc5,0x3c,0x59,0xcd,                        //vmulps        %ymm5,%ymm8,%ymm9
  0xc5,0xb4,0x58,0xc9,                        //vaddps        %ymm1,%ymm9,%ymm1
  0xc5,0x3c,0x59,0xce,                        //vmulps        %ymm6,%ymm8,%ymm9
  0xc5,0xb4,0x58,0xd2,                        //vaddps        %ymm2,%ymm9,%ymm2
  0xc5,0x3c,0x59,0xc7,                        //vmulps        %ymm7,%ymm8,%ymm8
  0xc5,0xbc,0x58,0xdb,                        //vaddps        %ymm3,%ymm8,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_dstover_avx[] = {
  0xc4,0x62,0x7d,0x18,0x02,                   //vbroadcastss  (%rdx),%ymm8
  0xc5,0x3c,0x5c,0xc7,                        //vsubps        %ymm7,%ymm8,%ymm8
  0xc5,0xbc,0x59,0xc0,                        //vmulps        %ymm0,%ymm8,%ymm0
  0xc5,0xfc,0x58,0xc4,                        //vaddps        %ymm4,%ymm0,%ymm0
  0xc5,0xbc,0x59,0xc9,                        //vmulps        %ymm1,%ymm8,%ymm1
  0xc5,0xf4,0x58,0xcd,                        //vaddps        %ymm5,%ymm1,%ymm1
  0xc5,0xbc,0x59,0xd2,                        //vmulps        %ymm2,%ymm8,%ymm2
  0xc5,0xec,0x58,0xd6,                        //vaddps        %ymm6,%ymm2,%ymm2
  0xc5,0xbc,0x59,0xdb,                        //vmulps        %ymm3,%ymm8,%ymm3
  0xc5,0xe4,0x58,0xdf,                        //vaddps        %ymm7,%ymm3,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_0_avx[] = {
  0xc4,0x41,0x3c,0x57,0xc0,                   //vxorps        %ymm8,%ymm8,%ymm8
  0xc4,0xc1,0x7c,0x5f,0xc0,                   //vmaxps        %ymm8,%ymm0,%ymm0
  0xc4,0xc1,0x74,0x5f,0xc8,                   //vmaxps        %ymm8,%ymm1,%ymm1
  0xc4,0xc1,0x6c,0x5f,0xd0,                   //vmaxps        %ymm8,%ymm2,%ymm2
  0xc4,0xc1,0x64,0x5f,0xd8,                   //vmaxps        %ymm8,%ymm3,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_1_avx[] = {
  0xc4,0x62,0x7d,0x18,0x02,                   //vbroadcastss  (%rdx),%ymm8
  0xc4,0xc1,0x7c,0x5d,0xc0,                   //vminps        %ymm8,%ymm0,%ymm0
  0xc4,0xc1,0x74,0x5d,0xc8,                   //vminps        %ymm8,%ymm1,%ymm1
  0xc4,0xc1,0x6c,0x5d,0xd0,                   //vminps        %ymm8,%ymm2,%ymm2
  0xc4,0xc1,0x64,0x5d,0xd8,                   //vminps        %ymm8,%ymm3,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_a_avx[] = {
  0xc4,0x62,0x7d,0x18,0x02,                   //vbroadcastss  (%rdx),%ymm8
  0xc4,0xc1,0x64,0x5d,0xd8,                   //vminps        %ymm8,%ymm3,%ymm3
  0xc5,0xfc,0x5d,0xc3,                        //vminps        %ymm3,%ymm0,%ymm0
  0xc5,0xf4,0x5d,0xcb,                        //vminps        %ymm3,%ymm1,%ymm1
  0xc5,0xec,0x5d,0xd3,                        //vminps        %ymm3,%ymm2,%ymm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_set_rgb_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0xe2,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm0
  0xc4,0xe2,0x7d,0x18,0x48,0x04,              //vbroadcastss  0x4(%rax),%ymm1
  0xc4,0xe2,0x7d,0x18,0x50,0x08,              //vbroadcastss  0x8(%rax),%ymm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_swap_rb_avx[] = {
  0xc5,0x7c,0x28,0xc0,                        //vmovaps       %ymm0,%ymm8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0xfc,0x28,0xc2,                        //vmovaps       %ymm2,%ymm0
  0xc5,0x7c,0x29,0xc2,                        //vmovaps       %ymm8,%ymm2
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_swap_avx[] = {
  0xc5,0x7c,0x28,0xc3,                        //vmovaps       %ymm3,%ymm8
  0xc5,0x7c,0x28,0xca,                        //vmovaps       %ymm2,%ymm9
  0xc5,0x7c,0x28,0xd1,                        //vmovaps       %ymm1,%ymm10
  0xc5,0x7c,0x28,0xd8,                        //vmovaps       %ymm0,%ymm11
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0xfc,0x28,0xc4,                        //vmovaps       %ymm4,%ymm0
  0xc5,0xfc,0x28,0xcd,                        //vmovaps       %ymm5,%ymm1
  0xc5,0xfc,0x28,0xd6,                        //vmovaps       %ymm6,%ymm2
  0xc5,0xfc,0x28,0xdf,                        //vmovaps       %ymm7,%ymm3
  0xc5,0x7c,0x29,0xdc,                        //vmovaps       %ymm11,%ymm4
  0xc5,0x7c,0x29,0xd5,                        //vmovaps       %ymm10,%ymm5
  0xc5,0x7c,0x29,0xce,                        //vmovaps       %ymm9,%ymm6
  0xc5,0x7c,0x29,0xc7,                        //vmovaps       %ymm8,%ymm7
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_move_src_dst_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0xfc,0x28,0xe0,                        //vmovaps       %ymm0,%ymm4
  0xc5,0xfc,0x28,0xe9,                        //vmovaps       %ymm1,%ymm5
  0xc5,0xfc,0x28,0xf2,                        //vmovaps       %ymm2,%ymm6
  0xc5,0xfc,0x28,0xfb,                        //vmovaps       %ymm3,%ymm7
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_move_dst_src_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0xfc,0x28,0xc4,                        //vmovaps       %ymm4,%ymm0
  0xc5,0xfc,0x28,0xcd,                        //vmovaps       %ymm5,%ymm1
  0xc5,0xfc,0x28,0xd6,                        //vmovaps       %ymm6,%ymm2
  0xc5,0xfc,0x28,0xdf,                        //vmovaps       %ymm7,%ymm3
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_premul_avx[] = {
  0xc5,0xfc,0x59,0xc3,                        //vmulps        %ymm3,%ymm0,%ymm0
  0xc5,0xf4,0x59,0xcb,                        //vmulps        %ymm3,%ymm1,%ymm1
  0xc5,0xec,0x59,0xd3,                        //vmulps        %ymm3,%ymm2,%ymm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_unpremul_avx[] = {
  0xc4,0x41,0x3c,0x57,0xc0,                   //vxorps        %ymm8,%ymm8,%ymm8
  0xc4,0x41,0x64,0xc2,0xc8,0x00,              //vcmpeqps      %ymm8,%ymm3,%ymm9
  0xc4,0x62,0x7d,0x18,0x12,                   //vbroadcastss  (%rdx),%ymm10
  0xc5,0x2c,0x5e,0xd3,                        //vdivps        %ymm3,%ymm10,%ymm10
  0xc4,0x43,0x2d,0x4a,0xc0,0x90,              //vblendvps     %ymm9,%ymm8,%ymm10,%ymm8
  0xc5,0xbc,0x59,0xc0,                        //vmulps        %ymm0,%ymm8,%ymm0
  0xc5,0xbc,0x59,0xc9,                        //vmulps        %ymm1,%ymm8,%ymm1
  0xc5,0xbc,0x59,0xd2,                        //vmulps        %ymm2,%ymm8,%ymm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_from_srgb_avx[] = {
  0xc4,0x62,0x7d,0x18,0x42,0x40,              //vbroadcastss  0x40(%rdx),%ymm8
  0xc5,0x3c,0x59,0xc8,                        //vmulps        %ymm0,%ymm8,%ymm9
  0xc5,0x7c,0x59,0xd0,                        //vmulps        %ymm0,%ymm0,%ymm10
  0xc4,0x62,0x7d,0x18,0x5a,0x3c,              //vbroadcastss  0x3c(%rdx),%ymm11
  0xc4,0x62,0x7d,0x18,0x62,0x38,              //vbroadcastss  0x38(%rdx),%ymm12
  0xc5,0x24,0x59,0xe8,                        //vmulps        %ymm0,%ymm11,%ymm13
  0xc4,0x41,0x14,0x58,0xec,                   //vaddps        %ymm12,%ymm13,%ymm13
  0xc4,0x62,0x7d,0x18,0x72,0x34,              //vbroadcastss  0x34(%rdx),%ymm14
  0xc4,0x41,0x2c,0x59,0xd5,                   //vmulps        %ymm13,%ymm10,%ymm10
  0xc4,0x41,0x0c,0x58,0xd2,                   //vaddps        %ymm10,%ymm14,%ymm10
  0xc4,0x62,0x7d,0x18,0x6a,0x44,              //vbroadcastss  0x44(%rdx),%ymm13
  0xc4,0xc1,0x7c,0xc2,0xc5,0x01,              //vcmpltps      %ymm13,%ymm0,%ymm0
  0xc4,0xc3,0x2d,0x4a,0xc1,0x00,              //vblendvps     %ymm0,%ymm9,%ymm10,%ymm0
  0xc5,0x3c,0x59,0xc9,                        //vmulps        %ymm1,%ymm8,%ymm9
  0xc5,0x74,0x59,0xd1,                        //vmulps        %ymm1,%ymm1,%ymm10
  0xc5,0x24,0x59,0xf9,                        //vmulps        %ymm1,%ymm11,%ymm15
  0xc4,0x41,0x04,0x58,0xfc,                   //vaddps        %ymm12,%ymm15,%ymm15
  0xc4,0x41,0x2c,0x59,0xd7,                   //vmulps        %ymm15,%ymm10,%ymm10
  0xc4,0x41,0x0c,0x58,0xd2,                   //vaddps        %ymm10,%ymm14,%ymm10
  0xc4,0xc1,0x74,0xc2,0xcd,0x01,              //vcmpltps      %ymm13,%ymm1,%ymm1
  0xc4,0xc3,0x2d,0x4a,0xc9,0x10,              //vblendvps     %ymm1,%ymm9,%ymm10,%ymm1
  0xc5,0x3c,0x59,0xc2,                        //vmulps        %ymm2,%ymm8,%ymm8
  0xc5,0x6c,0x59,0xca,                        //vmulps        %ymm2,%ymm2,%ymm9
  0xc5,0x24,0x59,0xd2,                        //vmulps        %ymm2,%ymm11,%ymm10
  0xc4,0x41,0x2c,0x58,0xd4,                   //vaddps        %ymm12,%ymm10,%ymm10
  0xc4,0x41,0x34,0x59,0xca,                   //vmulps        %ymm10,%ymm9,%ymm9
  0xc4,0x41,0x0c,0x58,0xc9,                   //vaddps        %ymm9,%ymm14,%ymm9
  0xc4,0xc1,0x6c,0xc2,0xd5,0x01,              //vcmpltps      %ymm13,%ymm2,%ymm2
  0xc4,0xc3,0x35,0x4a,0xd0,0x20,              //vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_to_srgb_avx[] = {
  0xc5,0x7c,0x52,0xc0,                        //vrsqrtps      %ymm0,%ymm8
  0xc4,0x41,0x7c,0x53,0xc8,                   //vrcpps        %ymm8,%ymm9
  0xc4,0x41,0x7c,0x52,0xd0,                   //vrsqrtps      %ymm8,%ymm10
  0xc4,0x62,0x7d,0x18,0x42,0x48,              //vbroadcastss  0x48(%rdx),%ymm8
  0xc5,0x3c,0x59,0xd8,                        //vmulps        %ymm0,%ymm8,%ymm11
  0xc4,0x62,0x7d,0x18,0x22,                   //vbroadcastss  (%rdx),%ymm12
  0xc4,0x62,0x7d,0x18,0x6a,0x4c,              //vbroadcastss  0x4c(%rdx),%ymm13
  0xc4,0x62,0x7d,0x18,0x72,0x50,              //vbroadcastss  0x50(%rdx),%ymm14
  0xc4,0x62,0x7d,0x18,0x7a,0x54,              //vbroadcastss  0x54(%rdx),%ymm15
  0xc4,0x41,0x34,0x59,0xce,                   //vmulps        %ymm14,%ymm9,%ymm9
  0xc4,0x41,0x34,0x58,0xcf,                   //vaddps        %ymm15,%ymm9,%ymm9
  0xc4,0x41,0x2c,0x59,0xd5,                   //vmulps        %ymm13,%ymm10,%ymm10
  0xc4,0x41,0x2c,0x58,0xc9,                   //vaddps        %ymm9,%ymm10,%ymm9
  0xc4,0x41,0x1c,0x5d,0xc9,                   //vminps        %ymm9,%ymm12,%ymm9
  0xc4,0x62,0x7d,0x18,0x52,0x58,              //vbroadcastss  0x58(%rdx),%ymm10
  0xc4,0xc1,0x7c,0xc2,0xc2,0x01,              //vcmpltps      %ymm10,%ymm0,%ymm0
  0xc4,0xc3,0x35,0x4a,0xc3,0x00,              //vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
  0xc5,0x7c,0x52,0xc9,                        //vrsqrtps      %ymm1,%ymm9
  0xc4,0x41,0x7c,0x53,0xd9,                   //vrcpps        %ymm9,%ymm11
  0xc4,0x41,0x7c,0x52,0xc9,                   //vrsqrtps      %ymm9,%ymm9
  0xc4,0x41,0x0c,0x59,0xdb,                   //vmulps        %ymm11,%ymm14,%ymm11
  0xc4,0x41,0x04,0x58,0xdb,                   //vaddps        %ymm11,%ymm15,%ymm11
  0xc4,0x41,0x14,0x59,0xc9,                   //vmulps        %ymm9,%ymm13,%ymm9
  0xc4,0x41,0x34,0x58,0xcb,                   //vaddps        %ymm11,%ymm9,%ymm9
  0xc5,0x3c,0x59,0xd9,                        //vmulps        %ymm1,%ymm8,%ymm11
  0xc4,0x41,0x1c,0x5d,0xc9,                   //vminps        %ymm9,%ymm12,%ymm9
  0xc4,0xc1,0x74,0xc2,0xca,0x01,              //vcmpltps      %ymm10,%ymm1,%ymm1
  0xc4,0xc3,0x35,0x4a,0xcb,0x10,              //vblendvps     %ymm1,%ymm11,%ymm9,%ymm1
  0xc5,0x7c,0x52,0xca,                        //vrsqrtps      %ymm2,%ymm9
  0xc4,0x41,0x7c,0x53,0xd9,                   //vrcpps        %ymm9,%ymm11
  0xc4,0x41,0x0c,0x59,0xdb,                   //vmulps        %ymm11,%ymm14,%ymm11
  0xc4,0x41,0x04,0x58,0xdb,                   //vaddps        %ymm11,%ymm15,%ymm11
  0xc4,0x41,0x7c,0x52,0xc9,                   //vrsqrtps      %ymm9,%ymm9
  0xc4,0x41,0x14,0x59,0xc9,                   //vmulps        %ymm9,%ymm13,%ymm9
  0xc4,0x41,0x34,0x58,0xcb,                   //vaddps        %ymm11,%ymm9,%ymm9
  0xc4,0x41,0x1c,0x5d,0xc9,                   //vminps        %ymm9,%ymm12,%ymm9
  0xc5,0x3c,0x59,0xc2,                        //vmulps        %ymm2,%ymm8,%ymm8
  0xc4,0xc1,0x6c,0xc2,0xd2,0x01,              //vcmpltps      %ymm10,%ymm2,%ymm2
  0xc4,0xc3,0x35,0x4a,0xd0,0x20,              //vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_scale_1_float_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x62,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm8
  0xc5,0xbc,0x59,0xc0,                        //vmulps        %ymm0,%ymm8,%ymm0
  0xc5,0xbc,0x59,0xc9,                        //vmulps        %ymm1,%ymm8,%ymm1
  0xc5,0xbc,0x59,0xd2,                        //vmulps        %ymm2,%ymm8,%ymm2
  0xc5,0xbc,0x59,0xdb,                        //vmulps        %ymm3,%ymm8,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_scale_u8_avx[] = {
  0x49,0x89,0xc8,                             //mov           %rcx,%r8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x48,0x01,0xf8,                             //add           %rdi,%rax
  0x4d,0x85,0xc0,                             //test          %r8,%r8
  0x75,0x41,                                  //jne           51e <_sk_scale_u8_avx+0x51>
  0xc5,0x7b,0x10,0x00,                        //vmovsd        (%rax),%xmm8
  0xc4,0x42,0x79,0x31,0xc8,                   //vpmovzxbd     %xmm8,%xmm9
  0xc4,0x43,0x79,0x04,0xc0,0xe5,              //vpermilps     $0xe5,%xmm8,%xmm8
  0xc4,0x42,0x79,0x31,0xc0,                   //vpmovzxbd     %xmm8,%xmm8
  0xc4,0x43,0x35,0x18,0xc0,0x01,              //vinsertf128   $0x1,%xmm8,%ymm9,%ymm8
  0xc4,0x41,0x7c,0x5b,0xc0,                   //vcvtdq2ps     %ymm8,%ymm8
  0xc4,0x62,0x7d,0x18,0x4a,0x0c,              //vbroadcastss  0xc(%rdx),%ymm9
  0xc4,0x41,0x3c,0x59,0xc1,                   //vmulps        %ymm9,%ymm8,%ymm8
  0xc5,0xbc,0x59,0xc0,                        //vmulps        %ymm0,%ymm8,%ymm0
  0xc5,0xbc,0x59,0xc9,                        //vmulps        %ymm1,%ymm8,%ymm1
  0xc5,0xbc,0x59,0xd2,                        //vmulps        %ymm2,%ymm8,%ymm2
  0xc5,0xbc,0x59,0xdb,                        //vmulps        %ymm3,%ymm8,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x89,0xc1,                             //mov           %r8,%rcx
  0xff,0xe0,                                  //jmpq          *%rax
  0x31,0xc9,                                  //xor           %ecx,%ecx
  0x4d,0x89,0xc2,                             //mov           %r8,%r10
  0x45,0x31,0xc9,                             //xor           %r9d,%r9d
  0x44,0x0f,0xb6,0x18,                        //movzbl        (%rax),%r11d
  0x48,0xff,0xc0,                             //inc           %rax
  0x49,0xd3,0xe3,                             //shl           %cl,%r11
  0x4d,0x09,0xd9,                             //or            %r11,%r9
  0x48,0x83,0xc1,0x08,                        //add           $0x8,%rcx
  0x49,0xff,0xca,                             //dec           %r10
  0x75,0xea,                                  //jne           526 <_sk_scale_u8_avx+0x59>
  0xc4,0x41,0xf9,0x6e,0xc1,                   //vmovq         %r9,%xmm8
  0xeb,0x9e,                                  //jmp           4e1 <_sk_scale_u8_avx+0x14>
};

CODE const uint8_t sk_lerp_1_float_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x62,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm8
  0xc5,0xfc,0x5c,0xc4,                        //vsubps        %ymm4,%ymm0,%ymm0
  0xc4,0xc1,0x7c,0x59,0xc0,                   //vmulps        %ymm8,%ymm0,%ymm0
  0xc5,0xfc,0x58,0xc4,                        //vaddps        %ymm4,%ymm0,%ymm0
  0xc5,0xf4,0x5c,0xcd,                        //vsubps        %ymm5,%ymm1,%ymm1
  0xc4,0xc1,0x74,0x59,0xc8,                   //vmulps        %ymm8,%ymm1,%ymm1
  0xc5,0xf4,0x58,0xcd,                        //vaddps        %ymm5,%ymm1,%ymm1
  0xc5,0xec,0x5c,0xd6,                        //vsubps        %ymm6,%ymm2,%ymm2
  0xc4,0xc1,0x6c,0x59,0xd0,                   //vmulps        %ymm8,%ymm2,%ymm2
  0xc5,0xec,0x58,0xd6,                        //vaddps        %ymm6,%ymm2,%ymm2
  0xc5,0xe4,0x5c,0xdf,                        //vsubps        %ymm7,%ymm3,%ymm3
  0xc4,0xc1,0x64,0x59,0xd8,                   //vmulps        %ymm8,%ymm3,%ymm3
  0xc5,0xe4,0x58,0xdf,                        //vaddps        %ymm7,%ymm3,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_lerp_u8_avx[] = {
  0x49,0x89,0xc8,                             //mov           %rcx,%r8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x48,0x01,0xf8,                             //add           %rdi,%rax
  0x4d,0x85,0xc0,                             //test          %r8,%r8
  0x75,0x65,                                  //jne           5f7 <_sk_lerp_u8_avx+0x75>
  0xc5,0x7b,0x10,0x00,                        //vmovsd        (%rax),%xmm8
  0xc4,0x42,0x79,0x31,0xc8,                   //vpmovzxbd     %xmm8,%xmm9
  0xc4,0x43,0x79,0x04,0xc0,0xe5,              //vpermilps     $0xe5,%xmm8,%xmm8
  0xc4,0x42,0x79,0x31,0xc0,                   //vpmovzxbd     %xmm8,%xmm8
  0xc4,0x43,0x35,0x18,0xc0,0x01,              //vinsertf128   $0x1,%xmm8,%ymm9,%ymm8
  0xc4,0x41,0x7c,0x5b,0xc0,                   //vcvtdq2ps     %ymm8,%ymm8
  0xc4,0x62,0x7d,0x18,0x4a,0x0c,              //vbroadcastss  0xc(%rdx),%ymm9
  0xc4,0x41,0x3c,0x59,0xc1,                   //vmulps        %ymm9,%ymm8,%ymm8
  0xc5,0xfc,0x5c,0xc4,                        //vsubps        %ymm4,%ymm0,%ymm0
  0xc4,0xc1,0x7c,0x59,0xc0,                   //vmulps        %ymm8,%ymm0,%ymm0
  0xc5,0xfc,0x58,0xc4,                        //vaddps        %ymm4,%ymm0,%ymm0
  0xc5,0xf4,0x5c,0xcd,                        //vsubps        %ymm5,%ymm1,%ymm1
  0xc4,0xc1,0x74,0x59,0xc8,                   //vmulps        %ymm8,%ymm1,%ymm1
  0xc5,0xf4,0x58,0xcd,                        //vaddps        %ymm5,%ymm1,%ymm1
  0xc5,0xec,0x5c,0xd6,                        //vsubps        %ymm6,%ymm2,%ymm2
  0xc4,0xc1,0x6c,0x59,0xd0,                   //vmulps        %ymm8,%ymm2,%ymm2
  0xc5,0xec,0x58,0xd6,                        //vaddps        %ymm6,%ymm2,%ymm2
  0xc5,0xe4,0x5c,0xdf,                        //vsubps        %ymm7,%ymm3,%ymm3
  0xc4,0xc1,0x64,0x59,0xd8,                   //vmulps        %ymm8,%ymm3,%ymm3
  0xc5,0xe4,0x58,0xdf,                        //vaddps        %ymm7,%ymm3,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x89,0xc1,                             //mov           %r8,%rcx
  0xff,0xe0,                                  //jmpq          *%rax
  0x31,0xc9,                                  //xor           %ecx,%ecx
  0x4d,0x89,0xc2,                             //mov           %r8,%r10
  0x45,0x31,0xc9,                             //xor           %r9d,%r9d
  0x44,0x0f,0xb6,0x18,                        //movzbl        (%rax),%r11d
  0x48,0xff,0xc0,                             //inc           %rax
  0x49,0xd3,0xe3,                             //shl           %cl,%r11
  0x4d,0x09,0xd9,                             //or            %r11,%r9
  0x48,0x83,0xc1,0x08,                        //add           $0x8,%rcx
  0x49,0xff,0xca,                             //dec           %r10
  0x75,0xea,                                  //jne           5ff <_sk_lerp_u8_avx+0x7d>
  0xc4,0x41,0xf9,0x6e,0xc1,                   //vmovq         %r9,%xmm8
  0xe9,0x77,0xff,0xff,0xff,                   //jmpq          596 <_sk_lerp_u8_avx+0x14>
};

CODE const uint8_t sk_lerp_565_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x8b,0x10,                             //mov           (%rax),%r10
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x0f,0x85,0x94,0x00,0x00,0x00,              //jne           6c1 <_sk_lerp_565_avx+0xa2>
  0xc4,0x41,0x7a,0x6f,0x04,0x7a,              //vmovdqu       (%r10,%rdi,2),%xmm8
  0xc5,0xe1,0xef,0xdb,                        //vpxor         %xmm3,%xmm3,%xmm3
  0xc5,0xb9,0x69,0xdb,                        //vpunpckhwd    %xmm3,%xmm8,%xmm3
  0xc4,0x42,0x79,0x33,0xc0,                   //vpmovzxwd     %xmm8,%xmm8
  0xc4,0xe3,0x3d,0x18,0xdb,0x01,              //vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  0xc4,0x62,0x7d,0x18,0x42,0x68,              //vbroadcastss  0x68(%rdx),%ymm8
  0xc5,0x3c,0x54,0xc3,                        //vandps        %ymm3,%ymm8,%ymm8
  0xc4,0x41,0x7c,0x5b,0xc0,                   //vcvtdq2ps     %ymm8,%ymm8
  0xc4,0x62,0x7d,0x18,0x4a,0x74,              //vbroadcastss  0x74(%rdx),%ymm9
  0xc4,0x41,0x34,0x59,0xc0,                   //vmulps        %ymm8,%ymm9,%ymm8
  0xc4,0x62,0x7d,0x18,0x4a,0x6c,              //vbroadcastss  0x6c(%rdx),%ymm9
  0xc5,0x34,0x54,0xcb,                        //vandps        %ymm3,%ymm9,%ymm9
  0xc4,0x41,0x7c,0x5b,0xc9,                   //vcvtdq2ps     %ymm9,%ymm9
  0xc4,0x62,0x7d,0x18,0x52,0x78,              //vbroadcastss  0x78(%rdx),%ymm10
  0xc4,0x41,0x2c,0x59,0xc9,                   //vmulps        %ymm9,%ymm10,%ymm9
  0xc4,0x62,0x7d,0x18,0x52,0x70,              //vbroadcastss  0x70(%rdx),%ymm10
  0xc5,0xac,0x54,0xdb,                        //vandps        %ymm3,%ymm10,%ymm3
  0xc5,0xfc,0x5b,0xdb,                        //vcvtdq2ps     %ymm3,%ymm3
  0xc4,0x62,0x7d,0x18,0x52,0x7c,              //vbroadcastss  0x7c(%rdx),%ymm10
  0xc5,0xac,0x59,0xdb,                        //vmulps        %ymm3,%ymm10,%ymm3
  0xc5,0xfc,0x5c,0xc4,                        //vsubps        %ymm4,%ymm0,%ymm0
  0xc4,0xc1,0x7c,0x59,0xc0,                   //vmulps        %ymm8,%ymm0,%ymm0
  0xc5,0xfc,0x58,0xc4,                        //vaddps        %ymm4,%ymm0,%ymm0
  0xc5,0xf4,0x5c,0xcd,                        //vsubps        %ymm5,%ymm1,%ymm1
  0xc4,0xc1,0x74,0x59,0xc9,                   //vmulps        %ymm9,%ymm1,%ymm1
  0xc5,0xf4,0x58,0xcd,                        //vaddps        %ymm5,%ymm1,%ymm1
  0xc5,0xec,0x5c,0xd6,                        //vsubps        %ymm6,%ymm2,%ymm2
  0xc5,0xec,0x59,0xd3,                        //vmulps        %ymm3,%ymm2,%ymm2
  0xc5,0xec,0x58,0xd6,                        //vaddps        %ymm6,%ymm2,%ymm2
  0xc4,0xe2,0x7d,0x18,0x1a,                   //vbroadcastss  (%rdx),%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0x41,0x89,0xc8,                             //mov           %ecx,%r8d
  0x41,0x80,0xe0,0x07,                        //and           $0x7,%r8b
  0xc4,0x41,0x39,0xef,0xc0,                   //vpxor         %xmm8,%xmm8,%xmm8
  0x41,0xfe,0xc8,                             //dec           %r8b
  0x45,0x0f,0xb6,0xc0,                        //movzbl        %r8b,%r8d
  0x41,0x80,0xf8,0x06,                        //cmp           $0x6,%r8b
  0x0f,0x87,0x55,0xff,0xff,0xff,              //ja            633 <_sk_lerp_565_avx+0x14>
  0x4c,0x8d,0x0d,0x4b,0x00,0x00,0x00,         //lea           0x4b(%rip),%r9        # 730 <_sk_lerp_565_avx+0x111>
  0x4b,0x63,0x04,0x81,                        //movslq        (%r9,%r8,4),%rax
  0x4c,0x01,0xc8,                             //add           %r9,%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0xc5,0xe1,0xef,0xdb,                        //vpxor         %xmm3,%xmm3,%xmm3
  0xc4,0x41,0x61,0xc4,0x44,0x7a,0x0c,0x06,    //vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm3,%xmm8
  0xc4,0x41,0x39,0xc4,0x44,0x7a,0x0a,0x05,    //vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm8,%xmm8
  0xc4,0x41,0x39,0xc4,0x44,0x7a,0x08,0x04,    //vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm8,%xmm8
  0xc4,0x41,0x39,0xc4,0x44,0x7a,0x06,0x03,    //vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm8,%xmm8
  0xc4,0x41,0x39,0xc4,0x44,0x7a,0x04,0x02,    //vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm8,%xmm8
  0xc4,0x41,0x39,0xc4,0x44,0x7a,0x02,0x01,    //vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm8,%xmm8
  0xc4,0x41,0x39,0xc4,0x04,0x7a,0x00,         //vpinsrw       $0x0,(%r10,%rdi,2),%xmm8,%xmm8
  0xe9,0x05,0xff,0xff,0xff,                   //jmpq          633 <_sk_lerp_565_avx+0x14>
  0x66,0x90,                                  //xchg          %ax,%ax
  0xf2,0xff,                                  //repnz         (bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xea,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xe2,                                  //jmpq          *%rdx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xda,0xff,                                  //(bad)
  0xff,                                       //(bad)
  0xff,0xd2,                                  //callq         *%rdx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xca,                                  //dec           %edx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xbe,                                       //.byte         0xbe
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //.byte         0xff
};

CODE const uint8_t sk_load_tables_avx[] = {
  0x55,                                       //push          %rbp
  0x41,0x57,                                  //push          %r15
  0x41,0x56,                                  //push          %r14
  0x41,0x55,                                  //push          %r13
  0x41,0x54,                                  //push          %r12
  0x53,                                       //push          %rbx
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x8b,0x00,                             //mov           (%rax),%r8
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x0f,0x85,0x12,0x02,0x00,0x00,              //jne           976 <_sk_load_tables_avx+0x22a>
  0xc4,0x41,0x7c,0x10,0x04,0xb8,              //vmovups       (%r8,%rdi,4),%ymm8
  0xc4,0x62,0x7d,0x18,0x4a,0x10,              //vbroadcastss  0x10(%rdx),%ymm9
  0xc4,0xc1,0x34,0x54,0xc0,                   //vandps        %ymm8,%ymm9,%ymm0
  0xc4,0xc1,0xf9,0x7e,0xc1,                   //vmovq         %xmm0,%r9
  0x45,0x89,0xcb,                             //mov           %r9d,%r11d
  0xc4,0xc3,0xf9,0x16,0xc2,0x01,              //vpextrq       $0x1,%xmm0,%r10
  0x45,0x89,0xd6,                             //mov           %r10d,%r14d
  0x49,0xc1,0xea,0x20,                        //shr           $0x20,%r10
  0x49,0xc1,0xe9,0x20,                        //shr           $0x20,%r9
  0xc4,0xe3,0x7d,0x19,0xc0,0x01,              //vextractf128  $0x1,%ymm0,%xmm0
  0xc4,0xc1,0xf9,0x7e,0xc4,                   //vmovq         %xmm0,%r12
  0x45,0x89,0xe7,                             //mov           %r12d,%r15d
  0xc4,0xe3,0xf9,0x16,0xc3,0x01,              //vpextrq       $0x1,%xmm0,%rbx
  0x41,0x89,0xdd,                             //mov           %ebx,%r13d
  0x48,0xc1,0xeb,0x20,                        //shr           $0x20,%rbx
  0x49,0xc1,0xec,0x20,                        //shr           $0x20,%r12
  0x48,0x8b,0x68,0x08,                        //mov           0x8(%rax),%rbp
  0x4c,0x8b,0x40,0x10,                        //mov           0x10(%rax),%r8
  0xc4,0xa1,0x7a,0x10,0x44,0xbd,0x00,         //vmovss        0x0(%rbp,%r15,4),%xmm0
  0xc4,0xa3,0x79,0x21,0x44,0xa5,0x00,0x10,    //vinsertps     $0x10,0x0(%rbp,%r12,4),%xmm0,%xmm0
  0xc4,0xa3,0x79,0x21,0x44,0xad,0x00,0x20,    //vinsertps     $0x20,0x0(%rbp,%r13,4),%xmm0,%xmm0
  0xc5,0xfa,0x10,0x4c,0x9d,0x00,              //vmovss        0x0(%rbp,%rbx,4),%xmm1
  0xc4,0xe3,0x79,0x21,0xc1,0x30,              //vinsertps     $0x30,%xmm1,%xmm0,%xmm0
  0xc4,0xa1,0x7a,0x10,0x4c,0x9d,0x00,         //vmovss        0x0(%rbp,%r11,4),%xmm1
  0xc4,0xa3,0x71,0x21,0x4c,0x8d,0x00,0x10,    //vinsertps     $0x10,0x0(%rbp,%r9,4),%xmm1,%xmm1
  0xc4,0xa3,0x71,0x21,0x4c,0xb5,0x00,0x20,    //vinsertps     $0x20,0x0(%rbp,%r14,4),%xmm1,%xmm1
  0xc4,0xa1,0x7a,0x10,0x5c,0x95,0x00,         //vmovss        0x0(%rbp,%r10,4),%xmm3
  0xc4,0xe3,0x71,0x21,0xcb,0x30,              //vinsertps     $0x30,%xmm3,%xmm1,%xmm1
  0xc4,0xe3,0x75,0x18,0xc0,0x01,              //vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  0xc4,0xc1,0x71,0x72,0xd0,0x08,              //vpsrld        $0x8,%xmm8,%xmm1
  0xc4,0x43,0x7d,0x19,0xc2,0x01,              //vextractf128  $0x1,%ymm8,%xmm10
  0xc4,0xc1,0x69,0x72,0xd2,0x08,              //vpsrld        $0x8,%xmm10,%xmm2
  0xc4,0xe3,0x75,0x18,0xca,0x01,              //vinsertf128   $0x1,%xmm2,%ymm1,%ymm1
  0xc5,0xb4,0x54,0xc9,                        //vandps        %ymm1,%ymm9,%ymm1
  0xc4,0xc1,0xf9,0x7e,0xc9,                   //vmovq         %xmm1,%r9
  0x45,0x89,0xcb,                             //mov           %r9d,%r11d
  0xc4,0xc3,0xf9,0x16,0xca,0x01,              //vpextrq       $0x1,%xmm1,%r10
  0x45,0x89,0xd6,                             //mov           %r10d,%r14d
  0x49,0xc1,0xea,0x20,                        //shr           $0x20,%r10
  0x49,0xc1,0xe9,0x20,                        //shr           $0x20,%r9
  0xc4,0xe3,0x7d,0x19,0xc9,0x01,              //vextractf128  $0x1,%ymm1,%xmm1
  0xc4,0xe1,0xf9,0x7e,0xcd,                   //vmovq         %xmm1,%rbp
  0x41,0x89,0xef,                             //mov           %ebp,%r15d
  0xc4,0xe3,0xf9,0x16,0xcb,0x01,              //vpextrq       $0x1,%xmm1,%rbx
  0x41,0x89,0xdc,                             //mov           %ebx,%r12d
  0x48,0xc1,0xeb,0x20,                        //shr           $0x20,%rbx
  0x48,0xc1,0xed,0x20,                        //shr           $0x20,%rbp
  0xc4,0x81,0x7a,0x10,0x0c,0xb8,              //vmovss        (%r8,%r15,4),%xmm1
  0xc4,0xc3,0x71,0x21,0x0c,0xa8,0x10,         //vinsertps     $0x10,(%r8,%rbp,4),%xmm1,%xmm1
  0xc4,0x81,0x7a,0x10,0x14,0xa0,              //vmovss        (%r8,%r12,4),%xmm2
  0xc4,0xe3,0x71,0x21,0xca,0x20,              //vinsertps     $0x20,%xmm2,%xmm1,%xmm1
  0xc4,0xc1,0x7a,0x10,0x14,0x98,              //vmovss        (%r8,%rbx,4),%xmm2
  0xc4,0xe3,0x71,0x21,0xca,0x30,              //vinsertps     $0x30,%xmm2,%xmm1,%xmm1
  0xc4,0x81,0x7a,0x10,0x14,0x98,              //vmovss        (%r8,%r11,4),%xmm2
  0xc4,0x83,0x69,0x21,0x14,0x88,0x10,         //vinsertps     $0x10,(%r8,%r9,4),%xmm2,%xmm2
  0xc4,0x81,0x7a,0x10,0x1c,0xb0,              //vmovss        (%r8,%r14,4),%xmm3
  0xc4,0xe3,0x69,0x21,0xd3,0x20,              //vinsertps     $0x20,%xmm3,%xmm2,%xmm2
  0xc4,0x81,0x7a,0x10,0x1c,0x90,              //vmovss        (%r8,%r10,4),%xmm3
  0xc4,0xe3,0x69,0x21,0xd3,0x30,              //vinsertps     $0x30,%xmm3,%xmm2,%xmm2
  0xc4,0xe3,0x6d,0x18,0xc9,0x01,              //vinsertf128   $0x1,%xmm1,%ymm2,%ymm1
  0x48,0x8b,0x40,0x18,                        //mov           0x18(%rax),%rax
  0xc4,0xc1,0x69,0x72,0xd0,0x10,              //vpsrld        $0x10,%xmm8,%xmm2
  0xc4,0xc1,0x61,0x72,0xd2,0x10,              //vpsrld        $0x10,%xmm10,%xmm3
  0xc4,0xe3,0x6d,0x18,0xd3,0x01,              //vinsertf128   $0x1,%xmm3,%ymm2,%ymm2
  0xc5,0xb4,0x54,0xd2,                        //vandps        %ymm2,%ymm9,%ymm2
  0xc4,0xc1,0xf9,0x7e,0xd0,                   //vmovq         %xmm2,%r8
  0x45,0x89,0xc2,                             //mov           %r8d,%r10d
  0xc4,0xc3,0xf9,0x16,0xd1,0x01,              //vpextrq       $0x1,%xmm2,%r9
  0x45,0x89,0xcb,                             //mov           %r9d,%r11d
  0x49,0xc1,0xe9,0x20,                        //shr           $0x20,%r9
  0x49,0xc1,0xe8,0x20,                        //shr           $0x20,%r8
  0xc4,0xe3,0x7d,0x19,0xd2,0x01,              //vextractf128  $0x1,%ymm2,%xmm2
  0xc4,0xe1,0xf9,0x7e,0xd5,                   //vmovq         %xmm2,%rbp
  0x41,0x89,0xee,                             //mov           %ebp,%r14d
  0xc4,0xe3,0xf9,0x16,0xd3,0x01,              //vpextrq       $0x1,%xmm2,%rbx
  0x41,0x89,0xdf,                             //mov           %ebx,%r15d
  0x48,0xc1,0xeb,0x20,                        //shr           $0x20,%rbx
  0x48,0xc1,0xed,0x20,                        //shr           $0x20,%rbp
  0xc4,0xa1,0x7a,0x10,0x14,0xb0,              //vmovss        (%rax,%r14,4),%xmm2
  0xc4,0xe3,0x69,0x21,0x14,0xa8,0x10,         //vinsertps     $0x10,(%rax,%rbp,4),%xmm2,%xmm2
  0xc4,0xa1,0x7a,0x10,0x1c,0xb8,              //vmovss        (%rax,%r15,4),%xmm3
  0xc4,0xe3,0x69,0x21,0xd3,0x20,              //vinsertps     $0x20,%xmm3,%xmm2,%xmm2
  0xc5,0xfa,0x10,0x1c,0x98,                   //vmovss        (%rax,%rbx,4),%xmm3
  0xc4,0x63,0x69,0x21,0xcb,0x30,              //vinsertps     $0x30,%xmm3,%xmm2,%xmm9
  0xc4,0xa1,0x7a,0x10,0x1c,0x90,              //vmovss        (%rax,%r10,4),%xmm3
  0xc4,0xa3,0x61,0x21,0x1c,0x80,0x10,         //vinsertps     $0x10,(%rax,%r8,4),%xmm3,%xmm3
  0xc4,0xa1,0x7a,0x10,0x14,0x98,              //vmovss        (%rax,%r11,4),%xmm2
  0xc4,0xe3,0x61,0x21,0xd2,0x20,              //vinsertps     $0x20,%xmm2,%xmm3,%xmm2
  0xc4,0xa1,0x7a,0x10,0x1c,0x88,              //vmovss        (%rax,%r9,4),%xmm3
  0xc4,0xe3,0x69,0x21,0xd3,0x30,              //vinsertps     $0x30,%xmm3,%xmm2,%xmm2
  0xc4,0xc3,0x6d,0x18,0xd1,0x01,              //vinsertf128   $0x1,%xmm9,%ymm2,%ymm2
  0xc4,0xc1,0x39,0x72,0xd0,0x18,              //vpsrld        $0x18,%xmm8,%xmm8
  0xc4,0xc1,0x61,0x72,0xd2,0x18,              //vpsrld        $0x18,%xmm10,%xmm3
  0xc4,0xe3,0x3d,0x18,0xdb,0x01,              //vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  0xc5,0xfc,0x5b,0xdb,                        //vcvtdq2ps     %ymm3,%ymm3
  0xc4,0x62,0x7d,0x18,0x42,0x0c,              //vbroadcastss  0xc(%rdx),%ymm8
  0xc4,0xc1,0x64,0x59,0xd8,                   //vmulps        %ymm8,%ymm3,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x5b,                                       //pop           %rbx
  0x41,0x5c,                                  //pop           %r12
  0x41,0x5d,                                  //pop           %r13
  0x41,0x5e,                                  //pop           %r14
  0x41,0x5f,                                  //pop           %r15
  0x5d,                                       //pop           %rbp
  0xff,0xe0,                                  //jmpq          *%rax
  0x41,0x89,0xc9,                             //mov           %ecx,%r9d
  0x41,0x80,0xe1,0x07,                        //and           $0x7,%r9b
  0xc4,0x41,0x3c,0x57,0xc0,                   //vxorps        %ymm8,%ymm8,%ymm8
  0x41,0xfe,0xc9,                             //dec           %r9b
  0x45,0x0f,0xb6,0xc9,                        //movzbl        %r9b,%r9d
  0x41,0x80,0xf9,0x06,                        //cmp           $0x6,%r9b
  0x0f,0x87,0xd7,0xfd,0xff,0xff,              //ja            76a <_sk_load_tables_avx+0x1e>
  0x4c,0x8d,0x15,0x8a,0x00,0x00,0x00,         //lea           0x8a(%rip),%r10        # a24 <_sk_load_tables_avx+0x2d8>
  0x4f,0x63,0x0c,0x8a,                        //movslq        (%r10,%r9,4),%r9
  0x4d,0x01,0xd1,                             //add           %r10,%r9
  0x41,0xff,0xe1,                             //jmpq          *%r9
  0xc4,0xc1,0x79,0x6e,0x44,0xb8,0x18,         //vmovd         0x18(%r8,%rdi,4),%xmm0
  0xc5,0xf9,0x70,0xc0,0x44,                   //vpshufd       $0x44,%xmm0,%xmm0
  0xc4,0xe3,0x7d,0x18,0xc0,0x01,              //vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  0xc5,0xf4,0x57,0xc9,                        //vxorps        %ymm1,%ymm1,%ymm1
  0xc4,0x63,0x75,0x0c,0xc0,0x40,              //vblendps      $0x40,%ymm0,%ymm1,%ymm8
  0xc4,0x63,0x7d,0x19,0xc0,0x01,              //vextractf128  $0x1,%ymm8,%xmm0
  0xc4,0xc3,0x79,0x22,0x44,0xb8,0x14,0x01,    //vpinsrd       $0x1,0x14(%r8,%rdi,4),%xmm0,%xmm0
  0xc4,0x63,0x3d,0x18,0xc0,0x01,              //vinsertf128   $0x1,%xmm0,%ymm8,%ymm8
  0xc4,0x63,0x7d,0x19,0xc0,0x01,              //vextractf128  $0x1,%ymm8,%xmm0
  0xc4,0xc3,0x79,0x22,0x44,0xb8,0x10,0x00,    //vpinsrd       $0x0,0x10(%r8,%rdi,4),%xmm0,%xmm0
  0xc4,0x63,0x3d,0x18,0xc0,0x01,              //vinsertf128   $0x1,%xmm0,%ymm8,%ymm8
  0xc4,0xc3,0x39,0x22,0x44,0xb8,0x0c,0x03,    //vpinsrd       $0x3,0xc(%r8,%rdi,4),%xmm8,%xmm0
  0xc4,0x63,0x3d,0x0c,0xc0,0x0f,              //vblendps      $0xf,%ymm0,%ymm8,%ymm8
  0xc4,0xc3,0x39,0x22,0x44,0xb8,0x08,0x02,    //vpinsrd       $0x2,0x8(%r8,%rdi,4),%xmm8,%xmm0
  0xc4,0x63,0x3d,0x0c,0xc0,0x0f,              //vblendps      $0xf,%ymm0,%ymm8,%ymm8
  0xc4,0xc3,0x39,0x22,0x44,0xb8,0x04,0x01,    //vpinsrd       $0x1,0x4(%r8,%rdi,4),%xmm8,%xmm0
  0xc4,0x63,0x3d,0x0c,0xc0,0x0f,              //vblendps      $0xf,%ymm0,%ymm8,%ymm8
  0xc4,0xc3,0x39,0x22,0x04,0xb8,0x00,         //vpinsrd       $0x0,(%r8,%rdi,4),%xmm8,%xmm0
  0xc4,0x63,0x3d,0x0c,0xc0,0x0f,              //vblendps      $0xf,%ymm0,%ymm8,%ymm8
  0xe9,0x46,0xfd,0xff,0xff,                   //jmpq          76a <_sk_load_tables_avx+0x1e>
  0xee,                                       //out           %al,(%dx)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xe0,                                  //jmpq          *%rax
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xd2,                                  //callq         *%rdx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xc4,                                  //inc           %esp
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xb0,0xff,0xff,0xff,0x9c,              //pushq         -0x63000001(%rax)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //.byte         0xff
  0x80,0xff,0xff,                             //cmp           $0xff,%bh
  0xff,                                       //.byte         0xff
};

CODE const uint8_t sk_load_a8_avx[] = {
  0x49,0x89,0xc8,                             //mov           %rcx,%r8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x48,0x01,0xf8,                             //add           %rdi,%rax
  0x4d,0x85,0xc0,                             //test          %r8,%r8
  0x75,0x3b,                                  //jne           a8b <_sk_load_a8_avx+0x4b>
  0xc5,0xfb,0x10,0x00,                        //vmovsd        (%rax),%xmm0
  0xc4,0xe2,0x79,0x31,0xc8,                   //vpmovzxbd     %xmm0,%xmm1
  0xc4,0xe3,0x79,0x04,0xc0,0xe5,              //vpermilps     $0xe5,%xmm0,%xmm0
  0xc4,0xe2,0x79,0x31,0xc0,                   //vpmovzxbd     %xmm0,%xmm0
  0xc4,0xe3,0x75,0x18,0xc0,0x01,              //vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  0xc5,0xfc,0x5b,0xc0,                        //vcvtdq2ps     %ymm0,%ymm0
  0xc4,0xe2,0x7d,0x18,0x4a,0x0c,              //vbroadcastss  0xc(%rdx),%ymm1
  0xc5,0xfc,0x59,0xd9,                        //vmulps        %ymm1,%ymm0,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0xfc,0x57,0xc0,                        //vxorps        %ymm0,%ymm0,%ymm0
  0xc5,0xf4,0x57,0xc9,                        //vxorps        %ymm1,%ymm1,%ymm1
  0xc5,0xec,0x57,0xd2,                        //vxorps        %ymm2,%ymm2,%ymm2
  0x4c,0x89,0xc1,                             //mov           %r8,%rcx
  0xff,0xe0,                                  //jmpq          *%rax
  0x31,0xc9,                                  //xor           %ecx,%ecx
  0x4d,0x89,0xc2,                             //mov           %r8,%r10
  0x45,0x31,0xc9,                             //xor           %r9d,%r9d
  0x44,0x0f,0xb6,0x18,                        //movzbl        (%rax),%r11d
  0x48,0xff,0xc0,                             //inc           %rax
  0x49,0xd3,0xe3,                             //shl           %cl,%r11
  0x4d,0x09,0xd9,                             //or            %r11,%r9
  0x48,0x83,0xc1,0x08,                        //add           $0x8,%rcx
  0x49,0xff,0xca,                             //dec           %r10
  0x75,0xea,                                  //jne           a93 <_sk_load_a8_avx+0x53>
  0xc4,0xc1,0xf9,0x6e,0xc1,                   //vmovq         %r9,%xmm0
  0xeb,0xa4,                                  //jmp           a54 <_sk_load_a8_avx+0x14>
};

CODE const uint8_t sk_store_a8_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x8b,0x08,                             //mov           (%rax),%r9
  0xc4,0x62,0x7d,0x18,0x42,0x08,              //vbroadcastss  0x8(%rdx),%ymm8
  0xc5,0x3c,0x59,0xc3,                        //vmulps        %ymm3,%ymm8,%ymm8
  0xc4,0x41,0x7d,0x5b,0xc0,                   //vcvtps2dq     %ymm8,%ymm8
  0xc4,0x43,0x7d,0x19,0xc1,0x01,              //vextractf128  $0x1,%ymm8,%xmm9
  0xc4,0x42,0x39,0x2b,0xc1,                   //vpackusdw     %xmm9,%xmm8,%xmm8
  0xc4,0x41,0x39,0x67,0xc0,                   //vpackuswb     %xmm8,%xmm8,%xmm8
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x75,0x0a,                                  //jne           ae3 <_sk_store_a8_avx+0x33>
  0xc4,0x41,0x7b,0x11,0x04,0x39,              //vmovsd        %xmm8,(%r9,%rdi,1)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0x89,0xc8,                                  //mov           %ecx,%eax
  0x24,0x07,                                  //and           $0x7,%al
  0xfe,0xc8,                                  //dec           %al
  0x44,0x0f,0xb6,0xc0,                        //movzbl        %al,%r8d
  0x41,0x80,0xf8,0x06,                        //cmp           $0x6,%r8b
  0x77,0xec,                                  //ja            adf <_sk_store_a8_avx+0x2f>
  0xc4,0x42,0x79,0x30,0xc0,                   //vpmovzxbw     %xmm8,%xmm8
  0x4c,0x8d,0x15,0x45,0x00,0x00,0x00,         //lea           0x45(%rip),%r10        # b44 <_sk_store_a8_avx+0x94>
  0x4b,0x63,0x04,0x82,                        //movslq        (%r10,%r8,4),%rax
  0x4c,0x01,0xd0,                             //add           %r10,%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0xc4,0x43,0x79,0x14,0x44,0x39,0x06,0x0c,    //vpextrb       $0xc,%xmm8,0x6(%r9,%rdi,1)
  0xc4,0x43,0x79,0x14,0x44,0x39,0x05,0x0a,    //vpextrb       $0xa,%xmm8,0x5(%r9,%rdi,1)
  0xc4,0x43,0x79,0x14,0x44,0x39,0x04,0x08,    //vpextrb       $0x8,%xmm8,0x4(%r9,%rdi,1)
  0xc4,0x43,0x79,0x14,0x44,0x39,0x03,0x06,    //vpextrb       $0x6,%xmm8,0x3(%r9,%rdi,1)
  0xc4,0x43,0x79,0x14,0x44,0x39,0x02,0x04,    //vpextrb       $0x4,%xmm8,0x2(%r9,%rdi,1)
  0xc4,0x43,0x79,0x14,0x44,0x39,0x01,0x02,    //vpextrb       $0x2,%xmm8,0x1(%r9,%rdi,1)
  0xc4,0x43,0x79,0x14,0x04,0x39,0x00,         //vpextrb       $0x0,%xmm8,(%r9,%rdi,1)
  0xeb,0x9e,                                  //jmp           adf <_sk_store_a8_avx+0x2f>
  0x0f,0x1f,0x00,                             //nopl          (%rax)
  0xf4,                                       //hlt
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xec,                                       //in            (%dx),%al
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xe4,                                  //jmpq          *%rsp
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xdc,0xff,                                  //fdivr         %st,%st(7)
  0xff,                                       //(bad)
  0xff,0xd4,                                  //callq         *%rsp
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xcc,                                  //dec           %esp
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xc4,                                  //inc           %esp
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //.byte         0xff
};

CODE const uint8_t sk_load_565_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x8b,0x10,                             //mov           (%rax),%r10
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x75,0x6a,                                  //jne           bd4 <_sk_load_565_avx+0x74>
  0xc4,0xc1,0x7a,0x6f,0x04,0x7a,              //vmovdqu       (%r10,%rdi,2),%xmm0
  0xc5,0xf1,0xef,0xc9,                        //vpxor         %xmm1,%xmm1,%xmm1
  0xc5,0xf9,0x69,0xc9,                        //vpunpckhwd    %xmm1,%xmm0,%xmm1
  0xc4,0xe2,0x79,0x33,0xc0,                   //vpmovzxwd     %xmm0,%xmm0
  0xc4,0xe3,0x7d,0x18,0xd1,0x01,              //vinsertf128   $0x1,%xmm1,%ymm0,%ymm2
  0xc4,0xe2,0x7d,0x18,0x42,0x68,              //vbroadcastss  0x68(%rdx),%ymm0
  0xc5,0xfc,0x54,0xc2,                        //vandps        %ymm2,%ymm0,%ymm0
  0xc5,0xfc,0x5b,0xc0,                        //vcvtdq2ps     %ymm0,%ymm0
  0xc4,0xe2,0x7d,0x18,0x4a,0x74,              //vbroadcastss  0x74(%rdx),%ymm1
  0xc5,0xf4,0x59,0xc0,                        //vmulps        %ymm0,%ymm1,%ymm0
  0xc4,0xe2,0x7d,0x18,0x4a,0x6c,              //vbroadcastss  0x6c(%rdx),%ymm1
  0xc5,0xf4,0x54,0xca,                        //vandps        %ymm2,%ymm1,%ymm1
  0xc5,0xfc,0x5b,0xc9,                        //vcvtdq2ps     %ymm1,%ymm1
  0xc4,0xe2,0x7d,0x18,0x5a,0x78,              //vbroadcastss  0x78(%rdx),%ymm3
  0xc5,0xe4,0x59,0xc9,                        //vmulps        %ymm1,%ymm3,%ymm1
  0xc4,0xe2,0x7d,0x18,0x5a,0x70,              //vbroadcastss  0x70(%rdx),%ymm3
  0xc5,0xe4,0x54,0xd2,                        //vandps        %ymm2,%ymm3,%ymm2
  0xc5,0xfc,0x5b,0xd2,                        //vcvtdq2ps     %ymm2,%ymm2
  0xc4,0xe2,0x7d,0x18,0x5a,0x7c,              //vbroadcastss  0x7c(%rdx),%ymm3
  0xc5,0xe4,0x59,0xd2,                        //vmulps        %ymm2,%ymm3,%ymm2
  0xc4,0xe2,0x7d,0x18,0x1a,                   //vbroadcastss  (%rdx),%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0x41,0x89,0xc8,                             //mov           %ecx,%r8d
  0x41,0x80,0xe0,0x07,                        //and           $0x7,%r8b
  0xc5,0xf9,0xef,0xc0,                        //vpxor         %xmm0,%xmm0,%xmm0
  0x41,0xfe,0xc8,                             //dec           %r8b
  0x45,0x0f,0xb6,0xc0,                        //movzbl        %r8b,%r8d
  0x41,0x80,0xf8,0x06,                        //cmp           $0x6,%r8b
  0x77,0x84,                                  //ja            b70 <_sk_load_565_avx+0x10>
  0x4c,0x8d,0x0d,0x49,0x00,0x00,0x00,         //lea           0x49(%rip),%r9        # c3c <_sk_load_565_avx+0xdc>
  0x4b,0x63,0x04,0x81,                        //movslq        (%r9,%r8,4),%rax
  0x4c,0x01,0xc8,                             //add           %r9,%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0xc5,0xf9,0xef,0xc0,                        //vpxor         %xmm0,%xmm0,%xmm0
  0xc4,0xc1,0x79,0xc4,0x44,0x7a,0x0c,0x06,    //vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm0,%xmm0
  0xc4,0xc1,0x79,0xc4,0x44,0x7a,0x0a,0x05,    //vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm0,%xmm0
  0xc4,0xc1,0x79,0xc4,0x44,0x7a,0x08,0x04,    //vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm0,%xmm0
  0xc4,0xc1,0x79,0xc4,0x44,0x7a,0x06,0x03,    //vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm0,%xmm0
  0xc4,0xc1,0x79,0xc4,0x44,0x7a,0x04,0x02,    //vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm0,%xmm0
  0xc4,0xc1,0x79,0xc4,0x44,0x7a,0x02,0x01,    //vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm0,%xmm0
  0xc4,0xc1,0x79,0xc4,0x04,0x7a,0x00,         //vpinsrw       $0x0,(%r10,%rdi,2),%xmm0,%xmm0
  0xe9,0x34,0xff,0xff,0xff,                   //jmpq          b70 <_sk_load_565_avx+0x10>
  0xf4,                                       //hlt
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xec,                                       //in            (%dx),%al
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xe4,                                  //jmpq          *%rsp
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xdc,0xff,                                  //fdivr         %st,%st(7)
  0xff,                                       //(bad)
  0xff,0xd4,                                  //callq         *%rsp
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xcc,                                  //dec           %esp
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xc0,                                  //inc           %eax
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //.byte         0xff
};

CODE const uint8_t sk_store_565_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x8b,0x08,                             //mov           (%rax),%r9
  0xc4,0x62,0x7d,0x18,0x82,0x80,0x00,0x00,0x00,//vbroadcastss  0x80(%rdx),%ymm8
  0xc5,0x3c,0x59,0xc8,                        //vmulps        %ymm0,%ymm8,%ymm9
  0xc4,0x41,0x7d,0x5b,0xc9,                   //vcvtps2dq     %ymm9,%ymm9
  0xc4,0xc1,0x29,0x72,0xf1,0x0b,              //vpslld        $0xb,%xmm9,%xmm10
  0xc4,0x43,0x7d,0x19,0xc9,0x01,              //vextractf128  $0x1,%ymm9,%xmm9
  0xc4,0xc1,0x31,0x72,0xf1,0x0b,              //vpslld        $0xb,%xmm9,%xmm9
  0xc4,0x43,0x2d,0x18,0xc9,0x01,              //vinsertf128   $0x1,%xmm9,%ymm10,%ymm9
  0xc4,0x62,0x7d,0x18,0x92,0x84,0x00,0x00,0x00,//vbroadcastss  0x84(%rdx),%ymm10
  0xc5,0x2c,0x59,0xd1,                        //vmulps        %ymm1,%ymm10,%ymm10
  0xc4,0x41,0x7d,0x5b,0xd2,                   //vcvtps2dq     %ymm10,%ymm10
  0xc4,0xc1,0x21,0x72,0xf2,0x05,              //vpslld        $0x5,%xmm10,%xmm11
  0xc4,0x43,0x7d,0x19,0xd2,0x01,              //vextractf128  $0x1,%ymm10,%xmm10
  0xc4,0xc1,0x29,0x72,0xf2,0x05,              //vpslld        $0x5,%xmm10,%xmm10
  0xc4,0x43,0x25,0x18,0xd2,0x01,              //vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  0xc4,0x41,0x2d,0x56,0xc9,                   //vorpd         %ymm9,%ymm10,%ymm9
  0xc5,0x3c,0x59,0xc2,                        //vmulps        %ymm2,%ymm8,%ymm8
  0xc4,0x41,0x7d,0x5b,0xc0,                   //vcvtps2dq     %ymm8,%ymm8
  0xc4,0x41,0x35,0x56,0xc0,                   //vorpd         %ymm8,%ymm9,%ymm8
  0xc4,0x43,0x7d,0x19,0xc1,0x01,              //vextractf128  $0x1,%ymm8,%xmm9
  0xc4,0x42,0x39,0x2b,0xc1,                   //vpackusdw     %xmm9,%xmm8,%xmm8
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x75,0x0a,                                  //jne           cde <_sk_store_565_avx+0x86>
  0xc4,0x41,0x7a,0x7f,0x04,0x79,              //vmovdqu       %xmm8,(%r9,%rdi,2)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0x89,0xc8,                                  //mov           %ecx,%eax
  0x24,0x07,                                  //and           $0x7,%al
  0xfe,0xc8,                                  //dec           %al
  0x44,0x0f,0xb6,0xc0,                        //movzbl        %al,%r8d
  0x41,0x80,0xf8,0x06,                        //cmp           $0x6,%r8b
  0x77,0xec,                                  //ja            cda <_sk_store_565_avx+0x82>
  0x4c,0x8d,0x15,0x47,0x00,0x00,0x00,         //lea           0x47(%rip),%r10        # d3c <_sk_store_565_avx+0xe4>
  0x4b,0x63,0x04,0x82,                        //movslq        (%r10,%r8,4),%rax
  0x4c,0x01,0xd0,                             //add           %r10,%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0xc4,0x43,0x79,0x15,0x44,0x79,0x0c,0x06,    //vpextrw       $0x6,%xmm8,0xc(%r9,%rdi,2)
  0xc4,0x43,0x79,0x15,0x44,0x79,0x0a,0x05,    //vpextrw       $0x5,%xmm8,0xa(%r9,%rdi,2)
  0xc4,0x43,0x79,0x15,0x44,0x79,0x08,0x04,    //vpextrw       $0x4,%xmm8,0x8(%r9,%rdi,2)
  0xc4,0x43,0x79,0x15,0x44,0x79,0x06,0x03,    //vpextrw       $0x3,%xmm8,0x6(%r9,%rdi,2)
  0xc4,0x43,0x79,0x15,0x44,0x79,0x04,0x02,    //vpextrw       $0x2,%xmm8,0x4(%r9,%rdi,2)
  0xc4,0x43,0x79,0x15,0x44,0x79,0x02,0x01,    //vpextrw       $0x1,%xmm8,0x2(%r9,%rdi,2)
  0xc5,0x79,0x7e,0xc0,                        //vmovd         %xmm8,%eax
  0x66,0x41,0x89,0x04,0x79,                   //mov           %ax,(%r9,%rdi,2)
  0xeb,0xa1,                                  //jmp           cda <_sk_store_565_avx+0x82>
  0x0f,0x1f,0x00,                             //nopl          (%rax)
  0xf2,0xff,                                  //repnz         (bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xea,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xe2,                                  //jmpq          *%rdx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xda,0xff,                                  //(bad)
  0xff,                                       //(bad)
  0xff,0xd2,                                  //callq         *%rdx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xca,                                  //dec           %edx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xc2,                                  //inc           %edx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //.byte         0xff
};

CODE const uint8_t sk_load_8888_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x8b,0x10,                             //mov           (%rax),%r10
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x75,0x7d,                                  //jne           ddf <_sk_load_8888_avx+0x87>
  0xc4,0x41,0x7c,0x10,0x0c,0xba,              //vmovups       (%r10,%rdi,4),%ymm9
  0xc4,0x62,0x7d,0x18,0x5a,0x10,              //vbroadcastss  0x10(%rdx),%ymm11
  0xc4,0xc1,0x24,0x54,0xc1,                   //vandps        %ymm9,%ymm11,%ymm0
  0xc5,0xfc,0x5b,0xc0,                        //vcvtdq2ps     %ymm0,%ymm0
  0xc4,0x62,0x7d,0x18,0x42,0x0c,              //vbroadcastss  0xc(%rdx),%ymm8
  0xc5,0xbc,0x59,0xc0,                        //vmulps        %ymm0,%ymm8,%ymm0
  0xc4,0xc1,0x29,0x72,0xd1,0x08,              //vpsrld        $0x8,%xmm9,%xmm10
  0xc4,0x63,0x7d,0x19,0xcb,0x01,              //vextractf128  $0x1,%ymm9,%xmm3
  0xc5,0xf1,0x72,0xd3,0x08,                   //vpsrld        $0x8,%xmm3,%xmm1
  0xc4,0xe3,0x2d,0x18,0xc9,0x01,              //vinsertf128   $0x1,%xmm1,%ymm10,%ymm1
  0xc5,0xa4,0x54,0xc9,                        //vandps        %ymm1,%ymm11,%ymm1
  0xc5,0xfc,0x5b,0xc9,                        //vcvtdq2ps     %ymm1,%ymm1
  0xc5,0xbc,0x59,0xc9,                        //vmulps        %ymm1,%ymm8,%ymm1
  0xc4,0xc1,0x29,0x72,0xd1,0x10,              //vpsrld        $0x10,%xmm9,%xmm10
  0xc5,0xe9,0x72,0xd3,0x10,                   //vpsrld        $0x10,%xmm3,%xmm2
  0xc4,0xe3,0x2d,0x18,0xd2,0x01,              //vinsertf128   $0x1,%xmm2,%ymm10,%ymm2
  0xc5,0xa4,0x54,0xd2,                        //vandps        %ymm2,%ymm11,%ymm2
  0xc5,0xfc,0x5b,0xd2,                        //vcvtdq2ps     %ymm2,%ymm2
  0xc5,0xbc,0x59,0xd2,                        //vmulps        %ymm2,%ymm8,%ymm2
  0xc4,0xc1,0x31,0x72,0xd1,0x18,              //vpsrld        $0x18,%xmm9,%xmm9
  0xc5,0xe1,0x72,0xd3,0x18,                   //vpsrld        $0x18,%xmm3,%xmm3
  0xc4,0xe3,0x35,0x18,0xdb,0x01,              //vinsertf128   $0x1,%xmm3,%ymm9,%ymm3
  0xc5,0xfc,0x5b,0xdb,                        //vcvtdq2ps     %ymm3,%ymm3
  0xc4,0xc1,0x64,0x59,0xd8,                   //vmulps        %ymm8,%ymm3,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0x41,0x89,0xc8,                             //mov           %ecx,%r8d
  0x41,0x80,0xe0,0x07,                        //and           $0x7,%r8b
  0xc4,0x41,0x34,0x57,0xc9,                   //vxorps        %ymm9,%ymm9,%ymm9
  0x41,0xfe,0xc8,                             //dec           %r8b
  0x45,0x0f,0xb6,0xc0,                        //movzbl        %r8b,%r8d
  0x41,0x80,0xf8,0x06,                        //cmp           $0x6,%r8b
  0x0f,0x87,0x6c,0xff,0xff,0xff,              //ja            d68 <_sk_load_8888_avx+0x10>
  0x4c,0x8d,0x0d,0x89,0x00,0x00,0x00,         //lea           0x89(%rip),%r9        # e8c <_sk_load_8888_avx+0x134>
  0x4b,0x63,0x04,0x81,                        //movslq        (%r9,%r8,4),%rax
  0x4c,0x01,0xc8,                             //add           %r9,%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0xc4,0xc1,0x79,0x6e,0x44,0xba,0x18,         //vmovd         0x18(%r10,%rdi,4),%xmm0
  0xc5,0xf9,0x70,0xc0,0x44,                   //vpshufd       $0x44,%xmm0,%xmm0
  0xc4,0xe3,0x7d,0x18,0xc0,0x01,              //vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  0xc5,0xf4,0x57,0xc9,                        //vxorps        %ymm1,%ymm1,%ymm1
  0xc4,0x63,0x75,0x0c,0xc8,0x40,              //vblendps      $0x40,%ymm0,%ymm1,%ymm9
  0xc4,0x63,0x7d,0x19,0xc8,0x01,              //vextractf128  $0x1,%ymm9,%xmm0
  0xc4,0xc3,0x79,0x22,0x44,0xba,0x14,0x01,    //vpinsrd       $0x1,0x14(%r10,%rdi,4),%xmm0,%xmm0
  0xc4,0x63,0x35,0x18,0xc8,0x01,              //vinsertf128   $0x1,%xmm0,%ymm9,%ymm9
  0xc4,0x63,0x7d,0x19,0xc8,0x01,              //vextractf128  $0x1,%ymm9,%xmm0
  0xc4,0xc3,0x79,0x22,0x44,0xba,0x10,0x00,    //vpinsrd       $0x0,0x10(%r10,%rdi,4),%xmm0,%xmm0
  0xc4,0x63,0x35,0x18,0xc8,0x01,              //vinsertf128   $0x1,%xmm0,%ymm9,%ymm9
  0xc4,0xc3,0x31,0x22,0x44,0xba,0x0c,0x03,    //vpinsrd       $0x3,0xc(%r10,%rdi,4),%xmm9,%xmm0
  0xc4,0x63,0x35,0x0c,0xc8,0x0f,              //vblendps      $0xf,%ymm0,%ymm9,%ymm9
  0xc4,0xc3,0x31,0x22,0x44,0xba,0x08,0x02,    //vpinsrd       $0x2,0x8(%r10,%rdi,4),%xmm9,%xmm0
  0xc4,0x63,0x35,0x0c,0xc8,0x0f,              //vblendps      $0xf,%ymm0,%ymm9,%ymm9
  0xc4,0xc3,0x31,0x22,0x44,0xba,0x04,0x01,    //vpinsrd       $0x1,0x4(%r10,%rdi,4),%xmm9,%xmm0
  0xc4,0x63,0x35,0x0c,0xc8,0x0f,              //vblendps      $0xf,%ymm0,%ymm9,%ymm9
  0xc4,0xc3,0x31,0x22,0x04,0xba,0x00,         //vpinsrd       $0x0,(%r10,%rdi,4),%xmm9,%xmm0
  0xc4,0x63,0x35,0x0c,0xc8,0x0f,              //vblendps      $0xf,%ymm0,%ymm9,%ymm9
  0xe9,0xdc,0xfe,0xff,0xff,                   //jmpq          d68 <_sk_load_8888_avx+0x10>
  0xee,                                       //out           %al,(%dx)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xe0,                                  //jmpq          *%rax
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xd2,                                  //callq         *%rdx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xc4,                                  //inc           %esp
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xb0,0xff,0xff,0xff,0x9c,              //pushq         -0x63000001(%rax)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //.byte         0xff
  0x80,0xff,0xff,                             //cmp           $0xff,%bh
  0xff,                                       //.byte         0xff
};

CODE const uint8_t sk_store_8888_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x8b,0x08,                             //mov           (%rax),%r9
  0xc4,0x62,0x7d,0x18,0x42,0x08,              //vbroadcastss  0x8(%rdx),%ymm8
  0xc5,0x3c,0x59,0xc8,                        //vmulps        %ymm0,%ymm8,%ymm9
  0xc4,0x41,0x7d,0x5b,0xc9,                   //vcvtps2dq     %ymm9,%ymm9
  0xc5,0x3c,0x59,0xd1,                        //vmulps        %ymm1,%ymm8,%ymm10
  0xc4,0x41,0x7d,0x5b,0xd2,                   //vcvtps2dq     %ymm10,%ymm10
  0xc4,0xc1,0x21,0x72,0xf2,0x08,              //vpslld        $0x8,%xmm10,%xmm11
  0xc4,0x43,0x7d,0x19,0xd2,0x01,              //vextractf128  $0x1,%ymm10,%xmm10
  0xc4,0xc1,0x29,0x72,0xf2,0x08,              //vpslld        $0x8,%xmm10,%xmm10
  0xc4,0x43,0x25,0x18,0xd2,0x01,              //vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  0xc4,0x41,0x2d,0x56,0xc9,                   //vorpd         %ymm9,%ymm10,%ymm9
  0xc5,0x3c,0x59,0xd2,                        //vmulps        %ymm2,%ymm8,%ymm10
  0xc4,0x41,0x7d,0x5b,0xd2,                   //vcvtps2dq     %ymm10,%ymm10
  0xc4,0xc1,0x21,0x72,0xf2,0x10,              //vpslld        $0x10,%xmm10,%xmm11
  0xc4,0x43,0x7d,0x19,0xd2,0x01,              //vextractf128  $0x1,%ymm10,%xmm10
  0xc4,0xc1,0x29,0x72,0xf2,0x10,              //vpslld        $0x10,%xmm10,%xmm10
  0xc4,0x43,0x25,0x18,0xd2,0x01,              //vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  0xc5,0x3c,0x59,0xc3,                        //vmulps        %ymm3,%ymm8,%ymm8
  0xc4,0x41,0x7d,0x5b,0xc0,                   //vcvtps2dq     %ymm8,%ymm8
  0xc4,0xc1,0x21,0x72,0xf0,0x18,              //vpslld        $0x18,%xmm8,%xmm11
  0xc4,0x43,0x7d,0x19,0xc0,0x01,              //vextractf128  $0x1,%ymm8,%xmm8
  0xc4,0xc1,0x39,0x72,0xf0,0x18,              //vpslld        $0x18,%xmm8,%xmm8
  0xc4,0x43,0x25,0x18,0xc0,0x01,              //vinsertf128   $0x1,%xmm8,%ymm11,%ymm8
  0xc4,0x41,0x2d,0x56,0xc0,                   //vorpd         %ymm8,%ymm10,%ymm8
  0xc4,0x41,0x35,0x56,0xc0,                   //vorpd         %ymm8,%ymm9,%ymm8
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x75,0x0a,                                  //jne           f3d <_sk_store_8888_avx+0x95>
  0xc4,0x41,0x7c,0x11,0x04,0xb9,              //vmovups       %ymm8,(%r9,%rdi,4)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0x89,0xc8,                                  //mov           %ecx,%eax
  0x24,0x07,                                  //and           $0x7,%al
  0xfe,0xc8,                                  //dec           %al
  0x44,0x0f,0xb6,0xc0,                        //movzbl        %al,%r8d
  0x41,0x80,0xf8,0x06,                        //cmp           $0x6,%r8b
  0x77,0xec,                                  //ja            f39 <_sk_store_8888_avx+0x91>
  0x4c,0x8d,0x15,0x54,0x00,0x00,0x00,         //lea           0x54(%rip),%r10        # fa8 <_sk_store_8888_avx+0x100>
  0x4b,0x63,0x04,0x82,                        //movslq        (%r10,%r8,4),%rax
  0x4c,0x01,0xd0,                             //add           %r10,%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0xc4,0x43,0x7d,0x19,0xc1,0x01,              //vextractf128  $0x1,%ymm8,%xmm9
  0xc4,0x43,0x79,0x16,0x4c,0xb9,0x18,0x02,    //vpextrd       $0x2,%xmm9,0x18(%r9,%rdi,4)
  0xc4,0x43,0x7d,0x19,0xc1,0x01,              //vextractf128  $0x1,%ymm8,%xmm9
  0xc4,0x43,0x79,0x16,0x4c,0xb9,0x14,0x01,    //vpextrd       $0x1,%xmm9,0x14(%r9,%rdi,4)
  0xc4,0x43,0x7d,0x19,0xc1,0x01,              //vextractf128  $0x1,%ymm8,%xmm9
  0xc4,0x41,0x79,0x7e,0x4c,0xb9,0x10,         //vmovd         %xmm9,0x10(%r9,%rdi,4)
  0xc4,0x43,0x79,0x16,0x44,0xb9,0x0c,0x03,    //vpextrd       $0x3,%xmm8,0xc(%r9,%rdi,4)
  0xc4,0x43,0x79,0x16,0x44,0xb9,0x08,0x02,    //vpextrd       $0x2,%xmm8,0x8(%r9,%rdi,4)
  0xc4,0x43,0x79,0x16,0x44,0xb9,0x04,0x01,    //vpextrd       $0x1,%xmm8,0x4(%r9,%rdi,4)
  0xc4,0x41,0x79,0x7e,0x04,0xb9,              //vmovd         %xmm8,(%r9,%rdi,4)
  0xeb,0x93,                                  //jmp           f39 <_sk_store_8888_avx+0x91>
  0x66,0x90,                                  //xchg          %ax,%ax
  0xf6,0xff,                                  //idiv          %bh
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xee,                                       //out           %al,(%dx)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xe6,                                  //jmpq          *%rsi
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xde,0xff,                                  //fdivrp        %st,%st(7)
  0xff,                                       //(bad)
  0xff,0xd1,                                  //callq         *%rcx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,0xc3,                                  //inc           %ebx
  0xff,                                       //(bad)
  0xff,                                       //(bad)
  0xff,                                       //.byte         0xff
  0xb5,0xff,                                  //mov           $0xff,%ch
  0xff,                                       //(bad)
  0xff,                                       //.byte         0xff
};

CODE const uint8_t sk_load_f16_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x0f,0x85,0xf0,0x00,0x00,0x00,              //jne           10c2 <_sk_load_f16_avx+0xfe>
  0xc5,0xf9,0x10,0x0c,0xf8,                   //vmovupd       (%rax,%rdi,8),%xmm1
  0xc5,0xf9,0x10,0x54,0xf8,0x10,              //vmovupd       0x10(%rax,%rdi,8),%xmm2
  0xc5,0xf9,0x10,0x5c,0xf8,0x20,              //vmovupd       0x20(%rax,%rdi,8),%xmm3
  0xc5,0x79,0x10,0x44,0xf8,0x30,              //vmovupd       0x30(%rax,%rdi,8),%xmm8
  0xc5,0xf1,0x61,0xc2,                        //vpunpcklwd    %xmm2,%xmm1,%xmm0
  0xc5,0xf1,0x69,0xca,                        //vpunpckhwd    %xmm2,%xmm1,%xmm1
  0xc4,0xc1,0x61,0x61,0xd0,                   //vpunpcklwd    %xmm8,%xmm3,%xmm2
  0xc4,0xc1,0x61,0x69,0xd8,                   //vpunpckhwd    %xmm8,%xmm3,%xmm3
  0xc5,0x79,0x61,0xc1,                        //vpunpcklwd    %xmm1,%xmm0,%xmm8
  0xc5,0xf9,0x69,0xc1,                        //vpunpckhwd    %xmm1,%xmm0,%xmm0
  0xc5,0xe9,0x61,0xcb,                        //vpunpcklwd    %xmm3,%xmm2,%xmm1
  0xc5,0x69,0x69,0xcb,                        //vpunpckhwd    %xmm3,%xmm2,%xmm9
  0xc5,0xf9,0x6e,0x5a,0x64,                   //vmovd         0x64(%rdx),%xmm3
  0xc5,0xf9,0x70,0xdb,0x00,                   //vpshufd       $0x0,%xmm3,%xmm3
  0xc4,0xc1,0x61,0x65,0xd0,                   //vpcmpgtw      %xmm8,%xmm3,%xmm2
  0xc4,0x41,0x69,0xdf,0xc0,                   //vpandn        %xmm8,%xmm2,%xmm8
  0xc5,0xe1,0x65,0xd0,                        //vpcmpgtw      %xmm0,%xmm3,%xmm2
  0xc5,0xe9,0xdf,0xc0,                        //vpandn        %xmm0,%xmm2,%xmm0
  0xc5,0xe1,0x65,0xd1,                        //vpcmpgtw      %xmm1,%xmm3,%xmm2
  0xc5,0xe9,0xdf,0xc9,                        //vpandn        %xmm1,%xmm2,%xmm1
  0xc4,0xc1,0x61,0x65,0xd1,                   //vpcmpgtw      %xmm9,%xmm3,%xmm2
  0xc4,0xc1,0x69,0xdf,0xd1,                   //vpandn        %xmm9,%xmm2,%xmm2
  0xc4,0x42,0x79,0x33,0xd0,                   //vpmovzxwd     %xmm8,%xmm10
  0xc4,0x62,0x79,0x33,0xc9,                   //vpmovzxwd     %xmm1,%xmm9
  0xc5,0xe1,0xef,0xdb,                        //vpxor         %xmm3,%xmm3,%xmm3
  0xc5,0x39,0x69,0xc3,                        //vpunpckhwd    %xmm3,%xmm8,%xmm8
  0xc5,0xf1,0x69,0xcb,                        //vpunpckhwd    %xmm3,%xmm1,%xmm1
  0xc4,0x62,0x79,0x33,0xd8,                   //vpmovzxwd     %xmm0,%xmm11
  0xc4,0x62,0x79,0x33,0xe2,                   //vpmovzxwd     %xmm2,%xmm12
  0xc5,0x79,0x69,0xeb,                        //vpunpckhwd    %xmm3,%xmm0,%xmm13
  0xc5,0x69,0x69,0xf3,                        //vpunpckhwd    %xmm3,%xmm2,%xmm14
  0xc4,0xc1,0x79,0x72,0xf2,0x0d,              //vpslld        $0xd,%xmm10,%xmm0
  0xc4,0xc1,0x69,0x72,0xf1,0x0d,              //vpslld        $0xd,%xmm9,%xmm2
  0xc4,0xe3,0x7d,0x18,0xc2,0x01,              //vinsertf128   $0x1,%xmm2,%ymm0,%ymm0
  0xc4,0x62,0x7d,0x18,0x4a,0x5c,              //vbroadcastss  0x5c(%rdx),%ymm9
  0xc5,0xb4,0x59,0xc0,                        //vmulps        %ymm0,%ymm9,%ymm0
  0xc4,0xc1,0x69,0x72,0xf0,0x0d,              //vpslld        $0xd,%xmm8,%xmm2
  0xc5,0xf1,0x72,0xf1,0x0d,                   //vpslld        $0xd,%xmm1,%xmm1
  0xc4,0xe3,0x6d,0x18,0xc9,0x01,              //vinsertf128   $0x1,%xmm1,%ymm2,%ymm1
  0xc5,0xb4,0x59,0xc9,                        //vmulps        %ymm1,%ymm9,%ymm1
  0xc4,0xc1,0x69,0x72,0xf3,0x0d,              //vpslld        $0xd,%xmm11,%xmm2
  0xc4,0xc1,0x61,0x72,0xf4,0x0d,              //vpslld        $0xd,%xmm12,%xmm3
  0xc4,0xe3,0x6d,0x18,0xd3,0x01,              //vinsertf128   $0x1,%xmm3,%ymm2,%ymm2
  0xc5,0xb4,0x59,0xd2,                        //vmulps        %ymm2,%ymm9,%ymm2
  0xc4,0xc1,0x39,0x72,0xf5,0x0d,              //vpslld        $0xd,%xmm13,%xmm8
  0xc4,0xc1,0x61,0x72,0xf6,0x0d,              //vpslld        $0xd,%xmm14,%xmm3
  0xc4,0xe3,0x3d,0x18,0xdb,0x01,              //vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  0xc5,0xb4,0x59,0xdb,                        //vmulps        %ymm3,%ymm9,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0xc5,0xfb,0x10,0x0c,0xf8,                   //vmovsd        (%rax,%rdi,8),%xmm1
  0xc4,0x41,0x39,0x57,0xc0,                   //vxorpd        %xmm8,%xmm8,%xmm8
  0x48,0x83,0xf9,0x01,                        //cmp           $0x1,%rcx
  0x75,0x06,                                  //jne           10d8 <_sk_load_f16_avx+0x114>
  0xc5,0xfa,0x7e,0xc9,                        //vmovq         %xmm1,%xmm1
  0xeb,0x1e,                                  //jmp           10f6 <_sk_load_f16_avx+0x132>
  0xc5,0xf1,0x16,0x4c,0xf8,0x08,              //vmovhpd       0x8(%rax,%rdi,8),%xmm1,%xmm1
  0x48,0x83,0xf9,0x03,                        //cmp           $0x3,%rcx
  0x72,0x12,                                  //jb            10f6 <_sk_load_f16_avx+0x132>
  0xc5,0xfb,0x10,0x54,0xf8,0x10,              //vmovsd        0x10(%rax,%rdi,8),%xmm2
  0x48,0x83,0xf9,0x03,                        //cmp           $0x3,%rcx
  0x75,0x13,                                  //jne           1103 <_sk_load_f16_avx+0x13f>
  0xc5,0xfa,0x7e,0xd2,                        //vmovq         %xmm2,%xmm2
  0xeb,0x2e,                                  //jmp           1124 <_sk_load_f16_avx+0x160>
  0xc5,0xe1,0x57,0xdb,                        //vxorpd        %xmm3,%xmm3,%xmm3
  0xc5,0xe9,0x57,0xd2,                        //vxorpd        %xmm2,%xmm2,%xmm2
  0xe9,0xe6,0xfe,0xff,0xff,                   //jmpq          fe9 <_sk_load_f16_avx+0x25>
  0xc5,0xe9,0x16,0x54,0xf8,0x18,              //vmovhpd       0x18(%rax,%rdi,8),%xmm2,%xmm2
  0x48,0x83,0xf9,0x05,                        //cmp           $0x5,%rcx
  0x72,0x15,                                  //jb            1124 <_sk_load_f16_avx+0x160>
  0xc5,0xfb,0x10,0x5c,0xf8,0x20,              //vmovsd        0x20(%rax,%rdi,8),%xmm3
  0x48,0x83,0xf9,0x05,                        //cmp           $0x5,%rcx
  0x75,0x12,                                  //jne           112d <_sk_load_f16_avx+0x169>
  0xc5,0xfa,0x7e,0xdb,                        //vmovq         %xmm3,%xmm3
  0xe9,0xc5,0xfe,0xff,0xff,                   //jmpq          fe9 <_sk_load_f16_avx+0x25>
  0xc5,0xe1,0x57,0xdb,                        //vxorpd        %xmm3,%xmm3,%xmm3
  0xe9,0xbc,0xfe,0xff,0xff,                   //jmpq          fe9 <_sk_load_f16_avx+0x25>
  0xc5,0xe1,0x16,0x5c,0xf8,0x28,              //vmovhpd       0x28(%rax,%rdi,8),%xmm3,%xmm3
  0x48,0x83,0xf9,0x07,                        //cmp           $0x7,%rcx
  0x0f,0x82,0xac,0xfe,0xff,0xff,              //jb            fe9 <_sk_load_f16_avx+0x25>
  0xc5,0x7b,0x10,0x44,0xf8,0x30,              //vmovsd        0x30(%rax,%rdi,8),%xmm8
  0xe9,0xa1,0xfe,0xff,0xff,                   //jmpq          fe9 <_sk_load_f16_avx+0x25>
};

CODE const uint8_t sk_store_f16_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0xc4,0x62,0x7d,0x18,0x42,0x60,              //vbroadcastss  0x60(%rdx),%ymm8
  0xc5,0x3c,0x59,0xc8,                        //vmulps        %ymm0,%ymm8,%ymm9
  0xc4,0x43,0x7d,0x19,0xca,0x01,              //vextractf128  $0x1,%ymm9,%xmm10
  0xc4,0xc1,0x29,0x72,0xd2,0x0d,              //vpsrld        $0xd,%xmm10,%xmm10
  0xc4,0xc1,0x31,0x72,0xd1,0x0d,              //vpsrld        $0xd,%xmm9,%xmm9
  0xc5,0x3c,0x59,0xd9,                        //vmulps        %ymm1,%ymm8,%ymm11
  0xc4,0x43,0x7d,0x19,0xdc,0x01,              //vextractf128  $0x1,%ymm11,%xmm12
  0xc4,0xc1,0x19,0x72,0xd4,0x0d,              //vpsrld        $0xd,%xmm12,%xmm12
  0xc4,0xc1,0x21,0x72,0xd3,0x0d,              //vpsrld        $0xd,%xmm11,%xmm11
  0xc5,0x3c,0x59,0xea,                        //vmulps        %ymm2,%ymm8,%ymm13
  0xc4,0x43,0x7d,0x19,0xee,0x01,              //vextractf128  $0x1,%ymm13,%xmm14
  0xc4,0xc1,0x09,0x72,0xd6,0x0d,              //vpsrld        $0xd,%xmm14,%xmm14
  0xc4,0xc1,0x11,0x72,0xd5,0x0d,              //vpsrld        $0xd,%xmm13,%xmm13
  0xc5,0x3c,0x59,0xc3,                        //vmulps        %ymm3,%ymm8,%ymm8
  0xc4,0x43,0x7d,0x19,0xc7,0x01,              //vextractf128  $0x1,%ymm8,%xmm15
  0xc4,0xc1,0x01,0x72,0xd7,0x0d,              //vpsrld        $0xd,%xmm15,%xmm15
  0xc4,0xc1,0x39,0x72,0xd0,0x0d,              //vpsrld        $0xd,%xmm8,%xmm8
  0xc4,0xc1,0x21,0x73,0xfb,0x02,              //vpslldq       $0x2,%xmm11,%xmm11
  0xc4,0x41,0x21,0xeb,0xc9,                   //vpor          %xmm9,%xmm11,%xmm9
  0xc4,0xc1,0x21,0x73,0xfc,0x02,              //vpslldq       $0x2,%xmm12,%xmm11
  0xc4,0x41,0x21,0xeb,0xe2,                   //vpor          %xmm10,%xmm11,%xmm12
  0xc4,0xc1,0x39,0x73,0xf8,0x02,              //vpslldq       $0x2,%xmm8,%xmm8
  0xc4,0x41,0x39,0xeb,0xc5,                   //vpor          %xmm13,%xmm8,%xmm8
  0xc4,0xc1,0x29,0x73,0xff,0x02,              //vpslldq       $0x2,%xmm15,%xmm10
  0xc4,0x41,0x29,0xeb,0xee,                   //vpor          %xmm14,%xmm10,%xmm13
  0xc4,0x41,0x31,0x62,0xd8,                   //vpunpckldq    %xmm8,%xmm9,%xmm11
  0xc4,0x41,0x31,0x6a,0xd0,                   //vpunpckhdq    %xmm8,%xmm9,%xmm10
  0xc4,0x41,0x19,0x62,0xcd,                   //vpunpckldq    %xmm13,%xmm12,%xmm9
  0xc4,0x41,0x19,0x6a,0xc5,                   //vpunpckhdq    %xmm13,%xmm12,%xmm8
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x75,0x1b,                                  //jne           120b <_sk_store_f16_avx+0xc3>
  0xc5,0x78,0x11,0x1c,0xf8,                   //vmovups       %xmm11,(%rax,%rdi,8)
  0xc5,0x78,0x11,0x54,0xf8,0x10,              //vmovups       %xmm10,0x10(%rax,%rdi,8)
  0xc5,0x78,0x11,0x4c,0xf8,0x20,              //vmovups       %xmm9,0x20(%rax,%rdi,8)
  0xc5,0x7a,0x7f,0x44,0xf8,0x30,              //vmovdqu       %xmm8,0x30(%rax,%rdi,8)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0xc5,0x79,0xd6,0x1c,0xf8,                   //vmovq         %xmm11,(%rax,%rdi,8)
  0x48,0x83,0xf9,0x01,                        //cmp           $0x1,%rcx
  0x74,0xf1,                                  //je            1207 <_sk_store_f16_avx+0xbf>
  0xc5,0x79,0x17,0x5c,0xf8,0x08,              //vmovhpd       %xmm11,0x8(%rax,%rdi,8)
  0x48,0x83,0xf9,0x03,                        //cmp           $0x3,%rcx
  0x72,0xe5,                                  //jb            1207 <_sk_store_f16_avx+0xbf>
  0xc5,0x79,0xd6,0x54,0xf8,0x10,              //vmovq         %xmm10,0x10(%rax,%rdi,8)
  0x74,0xdd,                                  //je            1207 <_sk_store_f16_avx+0xbf>
  0xc5,0x79,0x17,0x54,0xf8,0x18,              //vmovhpd       %xmm10,0x18(%rax,%rdi,8)
  0x48,0x83,0xf9,0x05,                        //cmp           $0x5,%rcx
  0x72,0xd1,                                  //jb            1207 <_sk_store_f16_avx+0xbf>
  0xc5,0x79,0xd6,0x4c,0xf8,0x20,              //vmovq         %xmm9,0x20(%rax,%rdi,8)
  0x74,0xc9,                                  //je            1207 <_sk_store_f16_avx+0xbf>
  0xc5,0x79,0x17,0x4c,0xf8,0x28,              //vmovhpd       %xmm9,0x28(%rax,%rdi,8)
  0x48,0x83,0xf9,0x07,                        //cmp           $0x7,%rcx
  0x72,0xbd,                                  //jb            1207 <_sk_store_f16_avx+0xbf>
  0xc5,0x79,0xd6,0x44,0xf8,0x30,              //vmovq         %xmm8,0x30(%rax,%rdi,8)
  0xeb,0xb5,                                  //jmp           1207 <_sk_store_f16_avx+0xbf>
};

CODE const uint8_t sk_store_f32_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x4c,0x8b,0x00,                             //mov           (%rax),%r8
  0x48,0x8d,0x04,0xbd,0x00,0x00,0x00,0x00,    //lea           0x0(,%rdi,4),%rax
  0xc5,0x7c,0x14,0xc1,                        //vunpcklps     %ymm1,%ymm0,%ymm8
  0xc5,0x7c,0x15,0xd9,                        //vunpckhps     %ymm1,%ymm0,%ymm11
  0xc5,0x6c,0x14,0xcb,                        //vunpcklps     %ymm3,%ymm2,%ymm9
  0xc5,0x6c,0x15,0xe3,                        //vunpckhps     %ymm3,%ymm2,%ymm12
  0xc4,0x41,0x3d,0x14,0xd1,                   //vunpcklpd     %ymm9,%ymm8,%ymm10
  0xc4,0x41,0x3d,0x15,0xc9,                   //vunpckhpd     %ymm9,%ymm8,%ymm9
  0xc4,0x41,0x25,0x14,0xc4,                   //vunpcklpd     %ymm12,%ymm11,%ymm8
  0xc4,0x41,0x25,0x15,0xdc,                   //vunpckhpd     %ymm12,%ymm11,%ymm11
  0x48,0x85,0xc9,                             //test          %rcx,%rcx
  0x75,0x37,                                  //jne           12bf <_sk_store_f32_avx+0x6d>
  0xc4,0x43,0x2d,0x18,0xe1,0x01,              //vinsertf128   $0x1,%xmm9,%ymm10,%ymm12
  0xc4,0x43,0x3d,0x18,0xeb,0x01,              //vinsertf128   $0x1,%xmm11,%ymm8,%ymm13
  0xc4,0x43,0x2d,0x06,0xc9,0x31,              //vperm2f128    $0x31,%ymm9,%ymm10,%ymm9
  0xc4,0x43,0x3d,0x06,0xc3,0x31,              //vperm2f128    $0x31,%ymm11,%ymm8,%ymm8
  0xc4,0x41,0x7d,0x11,0x24,0x80,              //vmovupd       %ymm12,(%r8,%rax,4)
  0xc4,0x41,0x7d,0x11,0x6c,0x80,0x20,         //vmovupd       %ymm13,0x20(%r8,%rax,4)
  0xc4,0x41,0x7d,0x11,0x4c,0x80,0x40,         //vmovupd       %ymm9,0x40(%r8,%rax,4)
  0xc4,0x41,0x7d,0x11,0x44,0x80,0x60,         //vmovupd       %ymm8,0x60(%r8,%rax,4)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
  0xc4,0x41,0x79,0x11,0x14,0x80,              //vmovupd       %xmm10,(%r8,%rax,4)
  0x48,0x83,0xf9,0x01,                        //cmp           $0x1,%rcx
  0x74,0xf0,                                  //je            12bb <_sk_store_f32_avx+0x69>
  0xc4,0x41,0x79,0x11,0x4c,0x80,0x10,         //vmovupd       %xmm9,0x10(%r8,%rax,4)
  0x48,0x83,0xf9,0x03,                        //cmp           $0x3,%rcx
  0x72,0xe3,                                  //jb            12bb <_sk_store_f32_avx+0x69>
  0xc4,0x41,0x79,0x11,0x44,0x80,0x20,         //vmovupd       %xmm8,0x20(%r8,%rax,4)
  0x74,0xda,                                  //je            12bb <_sk_store_f32_avx+0x69>
  0xc4,0x41,0x79,0x11,0x5c,0x80,0x30,         //vmovupd       %xmm11,0x30(%r8,%rax,4)
  0x48,0x83,0xf9,0x05,                        //cmp           $0x5,%rcx
  0x72,0xcd,                                  //jb            12bb <_sk_store_f32_avx+0x69>
  0xc4,0x43,0x7d,0x19,0x54,0x80,0x40,0x01,    //vextractf128  $0x1,%ymm10,0x40(%r8,%rax,4)
  0x74,0xc3,                                  //je            12bb <_sk_store_f32_avx+0x69>
  0xc4,0x43,0x7d,0x19,0x4c,0x80,0x50,0x01,    //vextractf128  $0x1,%ymm9,0x50(%r8,%rax,4)
  0x48,0x83,0xf9,0x07,                        //cmp           $0x7,%rcx
  0x72,0xb5,                                  //jb            12bb <_sk_store_f32_avx+0x69>
  0xc4,0x43,0x7d,0x19,0x44,0x80,0x60,0x01,    //vextractf128  $0x1,%ymm8,0x60(%r8,%rax,4)
  0xeb,0xab,                                  //jmp           12bb <_sk_store_f32_avx+0x69>
};

CODE const uint8_t sk_clamp_x_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x41,0x3c,0x57,0xc0,                   //vxorps        %ymm8,%ymm8,%ymm8
  0xc5,0x3c,0x5f,0xc8,                        //vmaxps        %ymm0,%ymm8,%ymm9
  0xc4,0x62,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm8
  0xc4,0x63,0x7d,0x19,0xc0,0x01,              //vextractf128  $0x1,%ymm8,%xmm0
  0xc4,0x41,0x29,0x76,0xd2,                   //vpcmpeqd      %xmm10,%xmm10,%xmm10
  0xc4,0xc1,0x79,0xfe,0xc2,                   //vpaddd        %xmm10,%xmm0,%xmm0
  0xc4,0x41,0x39,0xfe,0xc2,                   //vpaddd        %xmm10,%xmm8,%xmm8
  0xc4,0xe3,0x3d,0x18,0xc0,0x01,              //vinsertf128   $0x1,%xmm0,%ymm8,%ymm0
  0xc5,0xb4,0x5d,0xc0,                        //vminps        %ymm0,%ymm9,%ymm0
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_y_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x41,0x3c,0x57,0xc0,                   //vxorps        %ymm8,%ymm8,%ymm8
  0xc5,0x3c,0x5f,0xc9,                        //vmaxps        %ymm1,%ymm8,%ymm9
  0xc4,0x62,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm8
  0xc4,0x63,0x7d,0x19,0xc1,0x01,              //vextractf128  $0x1,%ymm8,%xmm1
  0xc4,0x41,0x29,0x76,0xd2,                   //vpcmpeqd      %xmm10,%xmm10,%xmm10
  0xc4,0xc1,0x71,0xfe,0xca,                   //vpaddd        %xmm10,%xmm1,%xmm1
  0xc4,0x41,0x39,0xfe,0xc2,                   //vpaddd        %xmm10,%xmm8,%xmm8
  0xc4,0xe3,0x3d,0x18,0xc9,0x01,              //vinsertf128   $0x1,%xmm1,%ymm8,%ymm1
  0xc5,0xb4,0x5d,0xc9,                        //vminps        %ymm1,%ymm9,%ymm1
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_repeat_x_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x62,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm8
  0xc4,0x41,0x7c,0x5e,0xc8,                   //vdivps        %ymm8,%ymm0,%ymm9
  0xc4,0x43,0x7d,0x08,0xc9,0x01,              //vroundps      $0x1,%ymm9,%ymm9
  0xc4,0x41,0x34,0x59,0xc8,                   //vmulps        %ymm8,%ymm9,%ymm9
  0xc4,0x41,0x7c,0x5c,0xc9,                   //vsubps        %ymm9,%ymm0,%ymm9
  0xc4,0x63,0x7d,0x19,0xc0,0x01,              //vextractf128  $0x1,%ymm8,%xmm0
  0xc4,0x41,0x29,0x76,0xd2,                   //vpcmpeqd      %xmm10,%xmm10,%xmm10
  0xc4,0xc1,0x79,0xfe,0xc2,                   //vpaddd        %xmm10,%xmm0,%xmm0
  0xc4,0x41,0x39,0xfe,0xc2,                   //vpaddd        %xmm10,%xmm8,%xmm8
  0xc4,0xe3,0x3d,0x18,0xc0,0x01,              //vinsertf128   $0x1,%xmm0,%ymm8,%ymm0
  0xc5,0xb4,0x5d,0xc0,                        //vminps        %ymm0,%ymm9,%ymm0
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_repeat_y_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x62,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm8
  0xc4,0x41,0x74,0x5e,0xc8,                   //vdivps        %ymm8,%ymm1,%ymm9
  0xc4,0x43,0x7d,0x08,0xc9,0x01,              //vroundps      $0x1,%ymm9,%ymm9
  0xc4,0x41,0x34,0x59,0xc8,                   //vmulps        %ymm8,%ymm9,%ymm9
  0xc4,0x41,0x74,0x5c,0xc9,                   //vsubps        %ymm9,%ymm1,%ymm9
  0xc4,0x63,0x7d,0x19,0xc1,0x01,              //vextractf128  $0x1,%ymm8,%xmm1
  0xc4,0x41,0x29,0x76,0xd2,                   //vpcmpeqd      %xmm10,%xmm10,%xmm10
  0xc4,0xc1,0x71,0xfe,0xca,                   //vpaddd        %xmm10,%xmm1,%xmm1
  0xc4,0x41,0x39,0xfe,0xc2,                   //vpaddd        %xmm10,%xmm8,%xmm8
  0xc4,0xe3,0x3d,0x18,0xc9,0x01,              //vinsertf128   $0x1,%xmm1,%ymm8,%ymm1
  0xc5,0xb4,0x5d,0xc9,                        //vminps        %ymm1,%ymm9,%ymm1
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_mirror_x_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0x7a,0x10,0x00,                        //vmovss        (%rax),%xmm8
  0xc4,0x41,0x79,0x70,0xc8,0x00,              //vpshufd       $0x0,%xmm8,%xmm9
  0xc4,0x43,0x35,0x18,0xc9,0x01,              //vinsertf128   $0x1,%xmm9,%ymm9,%ymm9
  0xc4,0x41,0x7c,0x5c,0xd1,                   //vsubps        %ymm9,%ymm0,%ymm10
  0xc4,0xc1,0x3a,0x58,0xc0,                   //vaddss        %xmm8,%xmm8,%xmm0
  0xc4,0xe3,0x79,0x04,0xc0,0x00,              //vpermilps     $0x0,%xmm0,%xmm0
  0xc4,0xe3,0x7d,0x18,0xc0,0x01,              //vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  0xc5,0x2c,0x5e,0xc0,                        //vdivps        %ymm0,%ymm10,%ymm8
  0xc4,0x43,0x7d,0x08,0xc0,0x01,              //vroundps      $0x1,%ymm8,%ymm8
  0xc5,0xbc,0x59,0xc0,                        //vmulps        %ymm0,%ymm8,%ymm0
  0xc5,0xac,0x5c,0xc0,                        //vsubps        %ymm0,%ymm10,%ymm0
  0xc4,0xc1,0x7c,0x5c,0xc1,                   //vsubps        %ymm9,%ymm0,%ymm0
  0xc4,0x41,0x3c,0x57,0xc0,                   //vxorps        %ymm8,%ymm8,%ymm8
  0xc5,0x3c,0x5c,0xc0,                        //vsubps        %ymm0,%ymm8,%ymm8
  0xc5,0x3c,0x54,0xc0,                        //vandps        %ymm0,%ymm8,%ymm8
  0xc4,0x63,0x7d,0x19,0xc8,0x01,              //vextractf128  $0x1,%ymm9,%xmm0
  0xc4,0x41,0x29,0x76,0xd2,                   //vpcmpeqd      %xmm10,%xmm10,%xmm10
  0xc4,0xc1,0x79,0xfe,0xc2,                   //vpaddd        %xmm10,%xmm0,%xmm0
  0xc4,0x41,0x31,0xfe,0xca,                   //vpaddd        %xmm10,%xmm9,%xmm9
  0xc4,0xe3,0x35,0x18,0xc0,0x01,              //vinsertf128   $0x1,%xmm0,%ymm9,%ymm0
  0xc5,0xbc,0x5d,0xc0,                        //vminps        %ymm0,%ymm8,%ymm0
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_mirror_y_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0x7a,0x10,0x00,                        //vmovss        (%rax),%xmm8
  0xc4,0x41,0x79,0x70,0xc8,0x00,              //vpshufd       $0x0,%xmm8,%xmm9
  0xc4,0x43,0x35,0x18,0xc9,0x01,              //vinsertf128   $0x1,%xmm9,%ymm9,%ymm9
  0xc4,0x41,0x74,0x5c,0xd1,                   //vsubps        %ymm9,%ymm1,%ymm10
  0xc4,0xc1,0x3a,0x58,0xc8,                   //vaddss        %xmm8,%xmm8,%xmm1
  0xc4,0xe3,0x79,0x04,0xc9,0x00,              //vpermilps     $0x0,%xmm1,%xmm1
  0xc4,0xe3,0x75,0x18,0xc9,0x01,              //vinsertf128   $0x1,%xmm1,%ymm1,%ymm1
  0xc5,0x2c,0x5e,0xc1,                        //vdivps        %ymm1,%ymm10,%ymm8
  0xc4,0x43,0x7d,0x08,0xc0,0x01,              //vroundps      $0x1,%ymm8,%ymm8
  0xc5,0xbc,0x59,0xc9,                        //vmulps        %ymm1,%ymm8,%ymm1
  0xc5,0xac,0x5c,0xc9,                        //vsubps        %ymm1,%ymm10,%ymm1
  0xc4,0xc1,0x74,0x5c,0xc9,                   //vsubps        %ymm9,%ymm1,%ymm1
  0xc4,0x41,0x3c,0x57,0xc0,                   //vxorps        %ymm8,%ymm8,%ymm8
  0xc5,0x3c,0x5c,0xc1,                        //vsubps        %ymm1,%ymm8,%ymm8
  0xc5,0x3c,0x54,0xc1,                        //vandps        %ymm1,%ymm8,%ymm8
  0xc4,0x63,0x7d,0x19,0xc9,0x01,              //vextractf128  $0x1,%ymm9,%xmm1
  0xc4,0x41,0x29,0x76,0xd2,                   //vpcmpeqd      %xmm10,%xmm10,%xmm10
  0xc4,0xc1,0x71,0xfe,0xca,                   //vpaddd        %xmm10,%xmm1,%xmm1
  0xc4,0x41,0x31,0xfe,0xca,                   //vpaddd        %xmm10,%xmm9,%xmm9
  0xc4,0xe3,0x35,0x18,0xc9,0x01,              //vinsertf128   $0x1,%xmm1,%ymm9,%ymm1
  0xc5,0xbc,0x5d,0xc9,                        //vminps        %ymm1,%ymm8,%ymm1
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_matrix_2x3_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x62,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm8
  0xc4,0x62,0x7d,0x18,0x48,0x08,              //vbroadcastss  0x8(%rax),%ymm9
  0xc4,0x62,0x7d,0x18,0x50,0x10,              //vbroadcastss  0x10(%rax),%ymm10
  0xc5,0x34,0x59,0xc9,                        //vmulps        %ymm1,%ymm9,%ymm9
  0xc4,0x41,0x34,0x58,0xca,                   //vaddps        %ymm10,%ymm9,%ymm9
  0xc5,0x3c,0x59,0xc0,                        //vmulps        %ymm0,%ymm8,%ymm8
  0xc4,0x41,0x3c,0x58,0xc1,                   //vaddps        %ymm9,%ymm8,%ymm8
  0xc4,0x62,0x7d,0x18,0x48,0x04,              //vbroadcastss  0x4(%rax),%ymm9
  0xc4,0x62,0x7d,0x18,0x50,0x0c,              //vbroadcastss  0xc(%rax),%ymm10
  0xc4,0x62,0x7d,0x18,0x58,0x14,              //vbroadcastss  0x14(%rax),%ymm11
  0xc5,0xac,0x59,0xc9,                        //vmulps        %ymm1,%ymm10,%ymm1
  0xc4,0xc1,0x74,0x58,0xcb,                   //vaddps        %ymm11,%ymm1,%ymm1
  0xc5,0xb4,0x59,0xc0,                        //vmulps        %ymm0,%ymm9,%ymm0
  0xc5,0xfc,0x58,0xc9,                        //vaddps        %ymm1,%ymm0,%ymm1
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0x7c,0x29,0xc0,                        //vmovaps       %ymm8,%ymm0
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_matrix_3x4_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x62,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm8
  0xc4,0x62,0x7d,0x18,0x48,0x0c,              //vbroadcastss  0xc(%rax),%ymm9
  0xc4,0x62,0x7d,0x18,0x50,0x18,              //vbroadcastss  0x18(%rax),%ymm10
  0xc4,0x62,0x7d,0x18,0x58,0x24,              //vbroadcastss  0x24(%rax),%ymm11
  0xc5,0x2c,0x59,0xd2,                        //vmulps        %ymm2,%ymm10,%ymm10
  0xc4,0x41,0x2c,0x58,0xd3,                   //vaddps        %ymm11,%ymm10,%ymm10
  0xc5,0x34,0x59,0xc9,                        //vmulps        %ymm1,%ymm9,%ymm9
  0xc4,0x41,0x34,0x58,0xca,                   //vaddps        %ymm10,%ymm9,%ymm9
  0xc5,0x3c,0x59,0xc0,                        //vmulps        %ymm0,%ymm8,%ymm8
  0xc4,0x41,0x3c,0x58,0xc1,                   //vaddps        %ymm9,%ymm8,%ymm8
  0xc4,0x62,0x7d,0x18,0x48,0x04,              //vbroadcastss  0x4(%rax),%ymm9
  0xc4,0x62,0x7d,0x18,0x50,0x10,              //vbroadcastss  0x10(%rax),%ymm10
  0xc4,0x62,0x7d,0x18,0x58,0x1c,              //vbroadcastss  0x1c(%rax),%ymm11
  0xc4,0x62,0x7d,0x18,0x60,0x28,              //vbroadcastss  0x28(%rax),%ymm12
  0xc5,0x24,0x59,0xda,                        //vmulps        %ymm2,%ymm11,%ymm11
  0xc4,0x41,0x24,0x58,0xdc,                   //vaddps        %ymm12,%ymm11,%ymm11
  0xc5,0x2c,0x59,0xd1,                        //vmulps        %ymm1,%ymm10,%ymm10
  0xc4,0x41,0x2c,0x58,0xd3,                   //vaddps        %ymm11,%ymm10,%ymm10
  0xc5,0x34,0x59,0xc8,                        //vmulps        %ymm0,%ymm9,%ymm9
  0xc4,0x41,0x34,0x58,0xca,                   //vaddps        %ymm10,%ymm9,%ymm9
  0xc4,0x62,0x7d,0x18,0x50,0x08,              //vbroadcastss  0x8(%rax),%ymm10
  0xc4,0x62,0x7d,0x18,0x58,0x14,              //vbroadcastss  0x14(%rax),%ymm11
  0xc4,0x62,0x7d,0x18,0x60,0x20,              //vbroadcastss  0x20(%rax),%ymm12
  0xc4,0x62,0x7d,0x18,0x68,0x2c,              //vbroadcastss  0x2c(%rax),%ymm13
  0xc5,0x9c,0x59,0xd2,                        //vmulps        %ymm2,%ymm12,%ymm2
  0xc4,0xc1,0x6c,0x58,0xd5,                   //vaddps        %ymm13,%ymm2,%ymm2
  0xc5,0xa4,0x59,0xc9,                        //vmulps        %ymm1,%ymm11,%ymm1
  0xc5,0xf4,0x58,0xca,                        //vaddps        %ymm2,%ymm1,%ymm1
  0xc5,0xac,0x59,0xc0,                        //vmulps        %ymm0,%ymm10,%ymm0
  0xc5,0xfc,0x58,0xd1,                        //vaddps        %ymm1,%ymm0,%ymm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0x7c,0x29,0xc0,                        //vmovaps       %ymm8,%ymm0
  0xc5,0x7c,0x29,0xc9,                        //vmovaps       %ymm9,%ymm1
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_matrix_perspective_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0x62,0x7d,0x18,0x00,                   //vbroadcastss  (%rax),%ymm8
  0xc4,0x62,0x7d,0x18,0x48,0x04,              //vbroadcastss  0x4(%rax),%ymm9
  0xc4,0x62,0x7d,0x18,0x50,0x08,              //vbroadcastss  0x8(%rax),%ymm10
  0xc5,0x34,0x59,0xc9,                        //vmulps        %ymm1,%ymm9,%ymm9
  0xc4,0x41,0x34,0x58,0xca,                   //vaddps        %ymm10,%ymm9,%ymm9
  0xc5,0x3c,0x59,0xc0,                        //vmulps        %ymm0,%ymm8,%ymm8
  0xc4,0x41,0x3c,0x58,0xc1,                   //vaddps        %ymm9,%ymm8,%ymm8
  0xc4,0x62,0x7d,0x18,0x48,0x0c,              //vbroadcastss  0xc(%rax),%ymm9
  0xc4,0x62,0x7d,0x18,0x50,0x10,              //vbroadcastss  0x10(%rax),%ymm10
  0xc4,0x62,0x7d,0x18,0x58,0x14,              //vbroadcastss  0x14(%rax),%ymm11
  0xc5,0x2c,0x59,0xd1,                        //vmulps        %ymm1,%ymm10,%ymm10
  0xc4,0x41,0x2c,0x58,0xd3,                   //vaddps        %ymm11,%ymm10,%ymm10
  0xc5,0x34,0x59,0xc8,                        //vmulps        %ymm0,%ymm9,%ymm9
  0xc4,0x41,0x34,0x58,0xca,                   //vaddps        %ymm10,%ymm9,%ymm9
  0xc4,0x62,0x7d,0x18,0x50,0x18,              //vbroadcastss  0x18(%rax),%ymm10
  0xc4,0x62,0x7d,0x18,0x58,0x1c,              //vbroadcastss  0x1c(%rax),%ymm11
  0xc4,0x62,0x7d,0x18,0x60,0x20,              //vbroadcastss  0x20(%rax),%ymm12
  0xc5,0xa4,0x59,0xc9,                        //vmulps        %ymm1,%ymm11,%ymm1
  0xc4,0xc1,0x74,0x58,0xcc,                   //vaddps        %ymm12,%ymm1,%ymm1
  0xc5,0xac,0x59,0xc0,                        //vmulps        %ymm0,%ymm10,%ymm0
  0xc5,0xfc,0x58,0xc1,                        //vaddps        %ymm1,%ymm0,%ymm0
  0xc5,0xfc,0x53,0xc8,                        //vrcpps        %ymm0,%ymm1
  0xc5,0xbc,0x59,0xc1,                        //vmulps        %ymm1,%ymm8,%ymm0
  0xc5,0xb4,0x59,0xc9,                        //vmulps        %ymm1,%ymm9,%ymm1
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_linear_gradient_2stops_avx[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc4,0xe2,0x7d,0x18,0x48,0x10,              //vbroadcastss  0x10(%rax),%ymm1
  0xc4,0xe2,0x7d,0x18,0x10,                   //vbroadcastss  (%rax),%ymm2
  0xc5,0xf4,0x59,0xc8,                        //vmulps        %ymm0,%ymm1,%ymm1
  0xc5,0x6c,0x58,0xc1,                        //vaddps        %ymm1,%ymm2,%ymm8
  0xc4,0xe2,0x7d,0x18,0x48,0x14,              //vbroadcastss  0x14(%rax),%ymm1
  0xc4,0xe2,0x7d,0x18,0x50,0x04,              //vbroadcastss  0x4(%rax),%ymm2
  0xc5,0xf4,0x59,0xc8,                        //vmulps        %ymm0,%ymm1,%ymm1
  0xc5,0xec,0x58,0xc9,                        //vaddps        %ymm1,%ymm2,%ymm1
  0xc4,0xe2,0x7d,0x18,0x50,0x18,              //vbroadcastss  0x18(%rax),%ymm2
  0xc4,0xe2,0x7d,0x18,0x58,0x08,              //vbroadcastss  0x8(%rax),%ymm3
  0xc5,0xec,0x59,0xd0,                        //vmulps        %ymm0,%ymm2,%ymm2
  0xc5,0xe4,0x58,0xd2,                        //vaddps        %ymm2,%ymm3,%ymm2
  0xc4,0xe2,0x7d,0x18,0x58,0x1c,              //vbroadcastss  0x1c(%rax),%ymm3
  0xc4,0x62,0x7d,0x18,0x48,0x0c,              //vbroadcastss  0xc(%rax),%ymm9
  0xc5,0xe4,0x59,0xc0,                        //vmulps        %ymm0,%ymm3,%ymm0
  0xc5,0xb4,0x58,0xd8,                        //vaddps        %ymm0,%ymm9,%ymm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xc5,0x7c,0x29,0xc0,                        //vmovaps       %ymm8,%ymm0
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_start_pipeline_sse41[] = {
  0x41,0x57,                                  //push          %r15
  0x41,0x56,                                  //push          %r14
  0x41,0x55,                                  //push          %r13
  0x41,0x54,                                  //push          %r12
  0x53,                                       //push          %rbx
  0x49,0x89,0xcf,                             //mov           %rcx,%r15
  0x49,0x89,0xd6,                             //mov           %rdx,%r14
  0x48,0x89,0xfb,                             //mov           %rdi,%rbx
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x49,0x89,0xc4,                             //mov           %rax,%r12
  0x49,0x89,0xf5,                             //mov           %rsi,%r13
  0x48,0x8d,0x43,0x04,                        //lea           0x4(%rbx),%rax
  0x4c,0x39,0xf8,                             //cmp           %r15,%rax
  0x76,0x05,                                  //jbe           28 <_sk_start_pipeline_sse41+0x28>
  0x48,0x89,0xd8,                             //mov           %rbx,%rax
  0xeb,0x34,                                  //jmp           5c <_sk_start_pipeline_sse41+0x5c>
  0x0f,0x57,0xc0,                             //xorps         %xmm0,%xmm0
  0x0f,0x57,0xc9,                             //xorps         %xmm1,%xmm1
  0x0f,0x57,0xd2,                             //xorps         %xmm2,%xmm2
  0x0f,0x57,0xdb,                             //xorps         %xmm3,%xmm3
  0x0f,0x57,0xe4,                             //xorps         %xmm4,%xmm4
  0x0f,0x57,0xed,                             //xorps         %xmm5,%xmm5
  0x0f,0x57,0xf6,                             //xorps         %xmm6,%xmm6
  0x0f,0x57,0xff,                             //xorps         %xmm7,%xmm7
  0x48,0x89,0xdf,                             //mov           %rbx,%rdi
  0x4c,0x89,0xee,                             //mov           %r13,%rsi
  0x4c,0x89,0xf2,                             //mov           %r14,%rdx
  0x41,0xff,0xd4,                             //callq         *%r12
  0x48,0x8d,0x43,0x04,                        //lea           0x4(%rbx),%rax
  0x48,0x83,0xc3,0x08,                        //add           $0x8,%rbx
  0x4c,0x39,0xfb,                             //cmp           %r15,%rbx
  0x48,0x89,0xc3,                             //mov           %rax,%rbx
  0x76,0xcc,                                  //jbe           28 <_sk_start_pipeline_sse41+0x28>
  0x5b,                                       //pop           %rbx
  0x41,0x5c,                                  //pop           %r12
  0x41,0x5d,                                  //pop           %r13
  0x41,0x5e,                                  //pop           %r14
  0x41,0x5f,                                  //pop           %r15
  0xc3,                                       //retq
};

CODE const uint8_t sk_start_pipeline_ms_sse41[] = {
  0x56,                                       //push          %rsi
  0x57,                                       //push          %rdi
  0x48,0x81,0xec,0xa8,0x00,0x00,0x00,         //sub           $0xa8,%rsp
  0x44,0x0f,0x29,0xbc,0x24,0x90,0x00,0x00,0x00,//movaps        %xmm15,0x90(%rsp)
  0x44,0x0f,0x29,0xb4,0x24,0x80,0x00,0x00,0x00,//movaps        %xmm14,0x80(%rsp)
  0x44,0x0f,0x29,0x6c,0x24,0x70,              //movaps        %xmm13,0x70(%rsp)
  0x44,0x0f,0x29,0x64,0x24,0x60,              //movaps        %xmm12,0x60(%rsp)
  0x44,0x0f,0x29,0x5c,0x24,0x50,              //movaps        %xmm11,0x50(%rsp)
  0x44,0x0f,0x29,0x54,0x24,0x40,              //movaps        %xmm10,0x40(%rsp)
  0x44,0x0f,0x29,0x4c,0x24,0x30,              //movaps        %xmm9,0x30(%rsp)
  0x44,0x0f,0x29,0x44,0x24,0x20,              //movaps        %xmm8,0x20(%rsp)
  0x0f,0x29,0x7c,0x24,0x10,                   //movaps        %xmm7,0x10(%rsp)
  0x0f,0x29,0x34,0x24,                        //movaps        %xmm6,(%rsp)
  0x48,0x89,0xcf,                             //mov           %rcx,%rdi
  0x48,0x89,0xd6,                             //mov           %rdx,%rsi
  0x4c,0x89,0xc2,                             //mov           %r8,%rdx
  0x4c,0x89,0xc9,                             //mov           %r9,%rcx
  0xe8,0x00,0x00,0x00,0x00,                   //callq         bf <_sk_start_pipeline_ms_sse41+0x59>
  0x0f,0x28,0x34,0x24,                        //movaps        (%rsp),%xmm6
  0x0f,0x28,0x7c,0x24,0x10,                   //movaps        0x10(%rsp),%xmm7
  0x44,0x0f,0x28,0x44,0x24,0x20,              //movaps        0x20(%rsp),%xmm8
  0x44,0x0f,0x28,0x4c,0x24,0x30,              //movaps        0x30(%rsp),%xmm9
  0x44,0x0f,0x28,0x54,0x24,0x40,              //movaps        0x40(%rsp),%xmm10
  0x44,0x0f,0x28,0x5c,0x24,0x50,              //movaps        0x50(%rsp),%xmm11
  0x44,0x0f,0x28,0x64,0x24,0x60,              //movaps        0x60(%rsp),%xmm12
  0x44,0x0f,0x28,0x6c,0x24,0x70,              //movaps        0x70(%rsp),%xmm13
  0x44,0x0f,0x28,0xb4,0x24,0x80,0x00,0x00,0x00,//movaps        0x80(%rsp),%xmm14
  0x44,0x0f,0x28,0xbc,0x24,0x90,0x00,0x00,0x00,//movaps        0x90(%rsp),%xmm15
  0x48,0x81,0xc4,0xa8,0x00,0x00,0x00,         //add           $0xa8,%rsp
  0x5f,                                       //pop           %rdi
  0x5e,                                       //pop           %rsi
  0xc3,                                       //retq
};

CODE const uint8_t sk_just_return_sse41[] = {
  0xc3,                                       //retq
};

CODE const uint8_t sk_seed_shader_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x66,0x0f,0x6e,0xc7,                        //movd          %edi,%xmm0
  0x66,0x0f,0x70,0xc0,0x00,                   //pshufd        $0x0,%xmm0,%xmm0
  0x0f,0x5b,0xc8,                             //cvtdq2ps      %xmm0,%xmm1
  0xf3,0x0f,0x10,0x12,                        //movss         (%rdx),%xmm2
  0xf3,0x0f,0x10,0x5a,0x04,                   //movss         0x4(%rdx),%xmm3
  0x0f,0xc6,0xdb,0x00,                        //shufps        $0x0,%xmm3,%xmm3
  0x0f,0x58,0xcb,                             //addps         %xmm3,%xmm1
  0x0f,0x10,0x42,0x14,                        //movups        0x14(%rdx),%xmm0
  0x0f,0x58,0xc1,                             //addps         %xmm1,%xmm0
  0x66,0x0f,0x6e,0x08,                        //movd          (%rax),%xmm1
  0x66,0x0f,0x70,0xc9,0x00,                   //pshufd        $0x0,%xmm1,%xmm1
  0x0f,0x5b,0xc9,                             //cvtdq2ps      %xmm1,%xmm1
  0x0f,0x58,0xcb,                             //addps         %xmm3,%xmm1
  0x0f,0xc6,0xd2,0x00,                        //shufps        $0x0,%xmm2,%xmm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x57,0xdb,                             //xorps         %xmm3,%xmm3
  0x0f,0x57,0xe4,                             //xorps         %xmm4,%xmm4
  0x0f,0x57,0xed,                             //xorps         %xmm5,%xmm5
  0x0f,0x57,0xf6,                             //xorps         %xmm6,%xmm6
  0x0f,0x57,0xff,                             //xorps         %xmm7,%xmm7
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_constant_color_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x10,0x18,                             //movups        (%rax),%xmm3
  0x0f,0x28,0xc3,                             //movaps        %xmm3,%xmm0
  0x0f,0xc6,0xc0,0x00,                        //shufps        $0x0,%xmm0,%xmm0
  0x0f,0x28,0xcb,                             //movaps        %xmm3,%xmm1
  0x0f,0xc6,0xc9,0x55,                        //shufps        $0x55,%xmm1,%xmm1
  0x0f,0x28,0xd3,                             //movaps        %xmm3,%xmm2
  0x0f,0xc6,0xd2,0xaa,                        //shufps        $0xaa,%xmm2,%xmm2
  0x0f,0xc6,0xdb,0xff,                        //shufps        $0xff,%xmm3,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clear_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x57,0xc0,                             //xorps         %xmm0,%xmm0
  0x0f,0x57,0xc9,                             //xorps         %xmm1,%xmm1
  0x0f,0x57,0xd2,                             //xorps         %xmm2,%xmm2
  0x0f,0x57,0xdb,                             //xorps         %xmm3,%xmm3
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_plus__sse41[] = {
  0x0f,0x58,0xc4,                             //addps         %xmm4,%xmm0
  0x0f,0x58,0xcd,                             //addps         %xmm5,%xmm1
  0x0f,0x58,0xd6,                             //addps         %xmm6,%xmm2
  0x0f,0x58,0xdf,                             //addps         %xmm7,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_srcover_sse41[] = {
  0xf3,0x44,0x0f,0x10,0x02,                   //movss         (%rdx),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x44,0x0f,0x5c,0xc3,                        //subps         %xmm3,%xmm8
  0x45,0x0f,0x28,0xc8,                        //movaps        %xmm8,%xmm9
  0x44,0x0f,0x59,0xcc,                        //mulps         %xmm4,%xmm9
  0x41,0x0f,0x58,0xc1,                        //addps         %xmm9,%xmm0
  0x45,0x0f,0x28,0xc8,                        //movaps        %xmm8,%xmm9
  0x44,0x0f,0x59,0xcd,                        //mulps         %xmm5,%xmm9
  0x41,0x0f,0x58,0xc9,                        //addps         %xmm9,%xmm1
  0x45,0x0f,0x28,0xc8,                        //movaps        %xmm8,%xmm9
  0x44,0x0f,0x59,0xce,                        //mulps         %xmm6,%xmm9
  0x41,0x0f,0x58,0xd1,                        //addps         %xmm9,%xmm2
  0x44,0x0f,0x59,0xc7,                        //mulps         %xmm7,%xmm8
  0x41,0x0f,0x58,0xd8,                        //addps         %xmm8,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_dstover_sse41[] = {
  0xf3,0x44,0x0f,0x10,0x02,                   //movss         (%rdx),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x44,0x0f,0x5c,0xc7,                        //subps         %xmm7,%xmm8
  0x41,0x0f,0x59,0xc0,                        //mulps         %xmm8,%xmm0
  0x0f,0x58,0xc4,                             //addps         %xmm4,%xmm0
  0x41,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm1
  0x0f,0x58,0xcd,                             //addps         %xmm5,%xmm1
  0x41,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm2
  0x0f,0x58,0xd6,                             //addps         %xmm6,%xmm2
  0x41,0x0f,0x59,0xd8,                        //mulps         %xmm8,%xmm3
  0x0f,0x58,0xdf,                             //addps         %xmm7,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_0_sse41[] = {
  0x45,0x0f,0x57,0xc0,                        //xorps         %xmm8,%xmm8
  0x41,0x0f,0x5f,0xc0,                        //maxps         %xmm8,%xmm0
  0x41,0x0f,0x5f,0xc8,                        //maxps         %xmm8,%xmm1
  0x41,0x0f,0x5f,0xd0,                        //maxps         %xmm8,%xmm2
  0x41,0x0f,0x5f,0xd8,                        //maxps         %xmm8,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_1_sse41[] = {
  0xf3,0x44,0x0f,0x10,0x02,                   //movss         (%rdx),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x41,0x0f,0x5d,0xc0,                        //minps         %xmm8,%xmm0
  0x41,0x0f,0x5d,0xc8,                        //minps         %xmm8,%xmm1
  0x41,0x0f,0x5d,0xd0,                        //minps         %xmm8,%xmm2
  0x41,0x0f,0x5d,0xd8,                        //minps         %xmm8,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_a_sse41[] = {
  0xf3,0x44,0x0f,0x10,0x02,                   //movss         (%rdx),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x41,0x0f,0x5d,0xd8,                        //minps         %xmm8,%xmm3
  0x0f,0x5d,0xc3,                             //minps         %xmm3,%xmm0
  0x0f,0x5d,0xcb,                             //minps         %xmm3,%xmm1
  0x0f,0x5d,0xd3,                             //minps         %xmm3,%xmm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_set_rgb_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x0f,0x10,0x00,                        //movss         (%rax),%xmm0
  0xf3,0x0f,0x10,0x48,0x04,                   //movss         0x4(%rax),%xmm1
  0x0f,0xc6,0xc0,0x00,                        //shufps        $0x0,%xmm0,%xmm0
  0x0f,0xc6,0xc9,0x00,                        //shufps        $0x0,%xmm1,%xmm1
  0xf3,0x0f,0x10,0x50,0x08,                   //movss         0x8(%rax),%xmm2
  0x0f,0xc6,0xd2,0x00,                        //shufps        $0x0,%xmm2,%xmm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_swap_rb_sse41[] = {
  0x44,0x0f,0x28,0xc0,                        //movaps        %xmm0,%xmm8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x28,0xc2,                             //movaps        %xmm2,%xmm0
  0x41,0x0f,0x28,0xd0,                        //movaps        %xmm8,%xmm2
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_swap_sse41[] = {
  0x44,0x0f,0x28,0xc3,                        //movaps        %xmm3,%xmm8
  0x44,0x0f,0x28,0xca,                        //movaps        %xmm2,%xmm9
  0x44,0x0f,0x28,0xd1,                        //movaps        %xmm1,%xmm10
  0x44,0x0f,0x28,0xd8,                        //movaps        %xmm0,%xmm11
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x28,0xc4,                             //movaps        %xmm4,%xmm0
  0x0f,0x28,0xcd,                             //movaps        %xmm5,%xmm1
  0x0f,0x28,0xd6,                             //movaps        %xmm6,%xmm2
  0x0f,0x28,0xdf,                             //movaps        %xmm7,%xmm3
  0x41,0x0f,0x28,0xe3,                        //movaps        %xmm11,%xmm4
  0x41,0x0f,0x28,0xea,                        //movaps        %xmm10,%xmm5
  0x41,0x0f,0x28,0xf1,                        //movaps        %xmm9,%xmm6
  0x41,0x0f,0x28,0xf8,                        //movaps        %xmm8,%xmm7
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_move_src_dst_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x28,0xe0,                             //movaps        %xmm0,%xmm4
  0x0f,0x28,0xe9,                             //movaps        %xmm1,%xmm5
  0x0f,0x28,0xf2,                             //movaps        %xmm2,%xmm6
  0x0f,0x28,0xfb,                             //movaps        %xmm3,%xmm7
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_move_dst_src_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x28,0xc4,                             //movaps        %xmm4,%xmm0
  0x0f,0x28,0xcd,                             //movaps        %xmm5,%xmm1
  0x0f,0x28,0xd6,                             //movaps        %xmm6,%xmm2
  0x0f,0x28,0xdf,                             //movaps        %xmm7,%xmm3
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_premul_sse41[] = {
  0x0f,0x59,0xc3,                             //mulps         %xmm3,%xmm0
  0x0f,0x59,0xcb,                             //mulps         %xmm3,%xmm1
  0x0f,0x59,0xd3,                             //mulps         %xmm3,%xmm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_unpremul_sse41[] = {
  0x44,0x0f,0x28,0xc0,                        //movaps        %xmm0,%xmm8
  0x45,0x0f,0x57,0xc9,                        //xorps         %xmm9,%xmm9
  0xf3,0x44,0x0f,0x10,0x12,                   //movss         (%rdx),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0x44,0x0f,0x5e,0xd3,                        //divps         %xmm3,%xmm10
  0x0f,0x28,0xc3,                             //movaps        %xmm3,%xmm0
  0x41,0x0f,0xc2,0xc1,0x00,                   //cmpeqps       %xmm9,%xmm0
  0x66,0x45,0x0f,0x38,0x14,0xd1,              //blendvps      %xmm0,%xmm9,%xmm10
  0x45,0x0f,0x59,0xc2,                        //mulps         %xmm10,%xmm8
  0x41,0x0f,0x59,0xca,                        //mulps         %xmm10,%xmm1
  0x41,0x0f,0x59,0xd2,                        //mulps         %xmm10,%xmm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x41,0x0f,0x28,0xc0,                        //movaps        %xmm8,%xmm0
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_from_srgb_sse41[] = {
  0x44,0x0f,0x28,0xc2,                        //movaps        %xmm2,%xmm8
  0xf3,0x44,0x0f,0x10,0x5a,0x40,              //movss         0x40(%rdx),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0x45,0x0f,0x28,0xd3,                        //movaps        %xmm11,%xmm10
  0x44,0x0f,0x59,0xd0,                        //mulps         %xmm0,%xmm10
  0x44,0x0f,0x28,0xf0,                        //movaps        %xmm0,%xmm14
  0x45,0x0f,0x59,0xf6,                        //mulps         %xmm14,%xmm14
  0xf3,0x0f,0x10,0x52,0x3c,                   //movss         0x3c(%rdx),%xmm2
  0x0f,0xc6,0xd2,0x00,                        //shufps        $0x0,%xmm2,%xmm2
  0xf3,0x44,0x0f,0x10,0x62,0x34,              //movss         0x34(%rdx),%xmm12
  0xf3,0x44,0x0f,0x10,0x6a,0x38,              //movss         0x38(%rdx),%xmm13
  0x45,0x0f,0xc6,0xed,0x00,                   //shufps        $0x0,%xmm13,%xmm13
  0x44,0x0f,0x28,0xca,                        //movaps        %xmm2,%xmm9
  0x44,0x0f,0x59,0xc8,                        //mulps         %xmm0,%xmm9
  0x45,0x0f,0x58,0xcd,                        //addps         %xmm13,%xmm9
  0x45,0x0f,0xc6,0xe4,0x00,                   //shufps        $0x0,%xmm12,%xmm12
  0x45,0x0f,0x59,0xce,                        //mulps         %xmm14,%xmm9
  0x45,0x0f,0x58,0xcc,                        //addps         %xmm12,%xmm9
  0xf3,0x44,0x0f,0x10,0x72,0x44,              //movss         0x44(%rdx),%xmm14
  0x45,0x0f,0xc6,0xf6,0x00,                   //shufps        $0x0,%xmm14,%xmm14
  0x41,0x0f,0xc2,0xc6,0x01,                   //cmpltps       %xmm14,%xmm0
  0x66,0x45,0x0f,0x38,0x14,0xca,              //blendvps      %xmm0,%xmm10,%xmm9
  0x45,0x0f,0x28,0xfb,                        //movaps        %xmm11,%xmm15
  0x44,0x0f,0x59,0xf9,                        //mulps         %xmm1,%xmm15
  0x0f,0x28,0xc1,                             //movaps        %xmm1,%xmm0
  0x0f,0x59,0xc0,                             //mulps         %xmm0,%xmm0
  0x44,0x0f,0x28,0xd2,                        //movaps        %xmm2,%xmm10
  0x44,0x0f,0x59,0xd1,                        //mulps         %xmm1,%xmm10
  0x45,0x0f,0x58,0xd5,                        //addps         %xmm13,%xmm10
  0x44,0x0f,0x59,0xd0,                        //mulps         %xmm0,%xmm10
  0x45,0x0f,0x58,0xd4,                        //addps         %xmm12,%xmm10
  0x41,0x0f,0xc2,0xce,0x01,                   //cmpltps       %xmm14,%xmm1
  0x0f,0x28,0xc1,                             //movaps        %xmm1,%xmm0
  0x66,0x45,0x0f,0x38,0x14,0xd7,              //blendvps      %xmm0,%xmm15,%xmm10
  0x45,0x0f,0x59,0xd8,                        //mulps         %xmm8,%xmm11
  0x41,0x0f,0x28,0xc0,                        //movaps        %xmm8,%xmm0
  0x0f,0x59,0xc0,                             //mulps         %xmm0,%xmm0
  0x41,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm2
  0x41,0x0f,0x58,0xd5,                        //addps         %xmm13,%xmm2
  0x0f,0x59,0xd0,                             //mulps         %xmm0,%xmm2
  0x41,0x0f,0x58,0xd4,                        //addps         %xmm12,%xmm2
  0x45,0x0f,0xc2,0xc6,0x01,                   //cmpltps       %xmm14,%xmm8
  0x41,0x0f,0x28,0xc0,                        //movaps        %xmm8,%xmm0
  0x66,0x41,0x0f,0x38,0x14,0xd3,              //blendvps      %xmm0,%xmm11,%xmm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x41,0x0f,0x28,0xc1,                        //movaps        %xmm9,%xmm0
  0x41,0x0f,0x28,0xca,                        //movaps        %xmm10,%xmm1
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_to_srgb_sse41[] = {
  0x48,0x83,0xec,0x18,                        //sub           $0x18,%rsp
  0x0f,0x29,0x3c,0x24,                        //movaps        %xmm7,(%rsp)
  0x0f,0x28,0xfe,                             //movaps        %xmm6,%xmm7
  0x0f,0x28,0xf5,                             //movaps        %xmm5,%xmm6
  0x0f,0x28,0xec,                             //movaps        %xmm4,%xmm5
  0x0f,0x28,0xe3,                             //movaps        %xmm3,%xmm4
  0x44,0x0f,0x28,0xc2,                        //movaps        %xmm2,%xmm8
  0x0f,0x28,0xd9,                             //movaps        %xmm1,%xmm3
  0x0f,0x52,0xd0,                             //rsqrtps       %xmm0,%xmm2
  0x44,0x0f,0x53,0xca,                        //rcpps         %xmm2,%xmm9
  0x44,0x0f,0x52,0xd2,                        //rsqrtps       %xmm2,%xmm10
  0xf3,0x0f,0x10,0x12,                        //movss         (%rdx),%xmm2
  0xf3,0x44,0x0f,0x10,0x5a,0x48,              //movss         0x48(%rdx),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0x41,0x0f,0x28,0xcb,                        //movaps        %xmm11,%xmm1
  0x0f,0x59,0xc8,                             //mulps         %xmm0,%xmm1
  0x0f,0xc6,0xd2,0x00,                        //shufps        $0x0,%xmm2,%xmm2
  0xf3,0x44,0x0f,0x10,0x62,0x4c,              //movss         0x4c(%rdx),%xmm12
  0x45,0x0f,0xc6,0xe4,0x00,                   //shufps        $0x0,%xmm12,%xmm12
  0xf3,0x44,0x0f,0x10,0x6a,0x50,              //movss         0x50(%rdx),%xmm13
  0x45,0x0f,0xc6,0xed,0x00,                   //shufps        $0x0,%xmm13,%xmm13
  0xf3,0x44,0x0f,0x10,0x72,0x54,              //movss         0x54(%rdx),%xmm14
  0x45,0x0f,0xc6,0xf6,0x00,                   //shufps        $0x0,%xmm14,%xmm14
  0x45,0x0f,0x59,0xcd,                        //mulps         %xmm13,%xmm9
  0x45,0x0f,0x58,0xce,                        //addps         %xmm14,%xmm9
  0x45,0x0f,0x59,0xd4,                        //mulps         %xmm12,%xmm10
  0x45,0x0f,0x58,0xd1,                        //addps         %xmm9,%xmm10
  0x44,0x0f,0x28,0xca,                        //movaps        %xmm2,%xmm9
  0x45,0x0f,0x5d,0xca,                        //minps         %xmm10,%xmm9
  0xf3,0x44,0x0f,0x10,0x7a,0x58,              //movss         0x58(%rdx),%xmm15
  0x45,0x0f,0xc6,0xff,0x00,                   //shufps        $0x0,%xmm15,%xmm15
  0x41,0x0f,0xc2,0xc7,0x01,                   //cmpltps       %xmm15,%xmm0
  0x66,0x44,0x0f,0x38,0x14,0xc9,              //blendvps      %xmm0,%xmm1,%xmm9
  0x0f,0x52,0xc3,                             //rsqrtps       %xmm3,%xmm0
  0x0f,0x53,0xc8,                             //rcpps         %xmm0,%xmm1
  0x0f,0x52,0xc0,                             //rsqrtps       %xmm0,%xmm0
  0x41,0x0f,0x59,0xcd,                        //mulps         %xmm13,%xmm1
  0x41,0x0f,0x58,0xce,                        //addps         %xmm14,%xmm1
  0x41,0x0f,0x59,0xc4,                        //mulps         %xmm12,%xmm0
  0x0f,0x58,0xc1,                             //addps         %xmm1,%xmm0
  0x44,0x0f,0x28,0xd2,                        //movaps        %xmm2,%xmm10
  0x44,0x0f,0x5d,0xd0,                        //minps         %xmm0,%xmm10
  0x41,0x0f,0x28,0xcb,                        //movaps        %xmm11,%xmm1
  0x0f,0x59,0xcb,                             //mulps         %xmm3,%xmm1
  0x41,0x0f,0xc2,0xdf,0x01,                   //cmpltps       %xmm15,%xmm3
  0x0f,0x28,0xc3,                             //movaps        %xmm3,%xmm0
  0x66,0x44,0x0f,0x38,0x14,0xd1,              //blendvps      %xmm0,%xmm1,%xmm10
  0x41,0x0f,0x52,0xc0,                        //rsqrtps       %xmm8,%xmm0
  0x0f,0x53,0xc8,                             //rcpps         %xmm0,%xmm1
  0x41,0x0f,0x59,0xcd,                        //mulps         %xmm13,%xmm1
  0x41,0x0f,0x58,0xce,                        //addps         %xmm14,%xmm1
  0x0f,0x52,0xc0,                             //rsqrtps       %xmm0,%xmm0
  0x41,0x0f,0x59,0xc4,                        //mulps         %xmm12,%xmm0
  0x0f,0x58,0xc1,                             //addps         %xmm1,%xmm0
  0x0f,0x5d,0xd0,                             //minps         %xmm0,%xmm2
  0x45,0x0f,0x59,0xd8,                        //mulps         %xmm8,%xmm11
  0x45,0x0f,0xc2,0xc7,0x01,                   //cmpltps       %xmm15,%xmm8
  0x41,0x0f,0x28,0xc0,                        //movaps        %xmm8,%xmm0
  0x66,0x41,0x0f,0x38,0x14,0xd3,              //blendvps      %xmm0,%xmm11,%xmm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x41,0x0f,0x28,0xc1,                        //movaps        %xmm9,%xmm0
  0x41,0x0f,0x28,0xca,                        //movaps        %xmm10,%xmm1
  0x0f,0x28,0xdc,                             //movaps        %xmm4,%xmm3
  0x0f,0x28,0xe5,                             //movaps        %xmm5,%xmm4
  0x0f,0x28,0xee,                             //movaps        %xmm6,%xmm5
  0x0f,0x28,0xf7,                             //movaps        %xmm7,%xmm6
  0x0f,0x28,0x3c,0x24,                        //movaps        (%rsp),%xmm7
  0x48,0x83,0xc4,0x18,                        //add           $0x18,%rsp
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_scale_1_float_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x44,0x0f,0x10,0x00,                   //movss         (%rax),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x41,0x0f,0x59,0xc0,                        //mulps         %xmm8,%xmm0
  0x41,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm1
  0x41,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm2
  0x41,0x0f,0x59,0xd8,                        //mulps         %xmm8,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_scale_u8_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x66,0x44,0x0f,0x38,0x31,0x04,0x38,         //pmovzxbd      (%rax,%rdi,1),%xmm8
  0x45,0x0f,0x5b,0xc0,                        //cvtdq2ps      %xmm8,%xmm8
  0xf3,0x44,0x0f,0x10,0x4a,0x0c,              //movss         0xc(%rdx),%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x45,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm9
  0x41,0x0f,0x59,0xc1,                        //mulps         %xmm9,%xmm0
  0x41,0x0f,0x59,0xc9,                        //mulps         %xmm9,%xmm1
  0x41,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm2
  0x41,0x0f,0x59,0xd9,                        //mulps         %xmm9,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_lerp_1_float_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x44,0x0f,0x10,0x00,                   //movss         (%rax),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x0f,0x5c,0xc4,                             //subps         %xmm4,%xmm0
  0x41,0x0f,0x59,0xc0,                        //mulps         %xmm8,%xmm0
  0x0f,0x58,0xc4,                             //addps         %xmm4,%xmm0
  0x0f,0x5c,0xcd,                             //subps         %xmm5,%xmm1
  0x41,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm1
  0x0f,0x58,0xcd,                             //addps         %xmm5,%xmm1
  0x0f,0x5c,0xd6,                             //subps         %xmm6,%xmm2
  0x41,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm2
  0x0f,0x58,0xd6,                             //addps         %xmm6,%xmm2
  0x0f,0x5c,0xdf,                             //subps         %xmm7,%xmm3
  0x41,0x0f,0x59,0xd8,                        //mulps         %xmm8,%xmm3
  0x0f,0x58,0xdf,                             //addps         %xmm7,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_lerp_u8_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x66,0x44,0x0f,0x38,0x31,0x04,0x38,         //pmovzxbd      (%rax,%rdi,1),%xmm8
  0x45,0x0f,0x5b,0xc0,                        //cvtdq2ps      %xmm8,%xmm8
  0xf3,0x44,0x0f,0x10,0x4a,0x0c,              //movss         0xc(%rdx),%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x45,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm9
  0x0f,0x5c,0xc4,                             //subps         %xmm4,%xmm0
  0x41,0x0f,0x59,0xc1,                        //mulps         %xmm9,%xmm0
  0x0f,0x58,0xc4,                             //addps         %xmm4,%xmm0
  0x0f,0x5c,0xcd,                             //subps         %xmm5,%xmm1
  0x41,0x0f,0x59,0xc9,                        //mulps         %xmm9,%xmm1
  0x0f,0x58,0xcd,                             //addps         %xmm5,%xmm1
  0x0f,0x5c,0xd6,                             //subps         %xmm6,%xmm2
  0x41,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm2
  0x0f,0x58,0xd6,                             //addps         %xmm6,%xmm2
  0x0f,0x5c,0xdf,                             //subps         %xmm7,%xmm3
  0x41,0x0f,0x59,0xd9,                        //mulps         %xmm9,%xmm3
  0x0f,0x58,0xdf,                             //addps         %xmm7,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_lerp_565_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x66,0x44,0x0f,0x38,0x33,0x04,0x78,         //pmovzxwd      (%rax,%rdi,2),%xmm8
  0x66,0x0f,0x6e,0x5a,0x68,                   //movd          0x68(%rdx),%xmm3
  0x66,0x0f,0x70,0xdb,0x00,                   //pshufd        $0x0,%xmm3,%xmm3
  0x66,0x41,0x0f,0xdb,0xd8,                   //pand          %xmm8,%xmm3
  0x44,0x0f,0x5b,0xcb,                        //cvtdq2ps      %xmm3,%xmm9
  0xf3,0x0f,0x10,0x1a,                        //movss         (%rdx),%xmm3
  0xf3,0x44,0x0f,0x10,0x52,0x74,              //movss         0x74(%rdx),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0x45,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm10
  0x66,0x44,0x0f,0x6e,0x4a,0x6c,              //movd          0x6c(%rdx),%xmm9
  0x66,0x45,0x0f,0x70,0xc9,0x00,              //pshufd        $0x0,%xmm9,%xmm9
  0x66,0x45,0x0f,0xdb,0xc8,                   //pand          %xmm8,%xmm9
  0x45,0x0f,0x5b,0xc9,                        //cvtdq2ps      %xmm9,%xmm9
  0xf3,0x44,0x0f,0x10,0x5a,0x78,              //movss         0x78(%rdx),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0x45,0x0f,0x59,0xd9,                        //mulps         %xmm9,%xmm11
  0x66,0x44,0x0f,0x6e,0x4a,0x70,              //movd          0x70(%rdx),%xmm9
  0x66,0x45,0x0f,0x70,0xc9,0x00,              //pshufd        $0x0,%xmm9,%xmm9
  0x66,0x45,0x0f,0xdb,0xc8,                   //pand          %xmm8,%xmm9
  0x45,0x0f,0x5b,0xc1,                        //cvtdq2ps      %xmm9,%xmm8
  0xf3,0x44,0x0f,0x10,0x4a,0x7c,              //movss         0x7c(%rdx),%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x45,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm9
  0x0f,0x5c,0xc4,                             //subps         %xmm4,%xmm0
  0x41,0x0f,0x59,0xc2,                        //mulps         %xmm10,%xmm0
  0x0f,0x58,0xc4,                             //addps         %xmm4,%xmm0
  0x0f,0x5c,0xcd,                             //subps         %xmm5,%xmm1
  0x41,0x0f,0x59,0xcb,                        //mulps         %xmm11,%xmm1
  0x0f,0x58,0xcd,                             //addps         %xmm5,%xmm1
  0x0f,0x5c,0xd6,                             //subps         %xmm6,%xmm2
  0x41,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm2
  0x0f,0x58,0xd6,                             //addps         %xmm6,%xmm2
  0x0f,0xc6,0xdb,0x00,                        //shufps        $0x0,%xmm3,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_load_tables_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x08,                             //mov           (%rax),%rcx
  0x4c,0x8b,0x40,0x08,                        //mov           0x8(%rax),%r8
  0xf3,0x44,0x0f,0x6f,0x04,0xb9,              //movdqu        (%rcx,%rdi,4),%xmm8
  0x66,0x0f,0x6e,0x42,0x10,                   //movd          0x10(%rdx),%xmm0
  0x66,0x0f,0x70,0xc0,0x00,                   //pshufd        $0x0,%xmm0,%xmm0
  0x66,0x41,0x0f,0x6f,0xc8,                   //movdqa        %xmm8,%xmm1
  0x66,0x0f,0x72,0xd1,0x08,                   //psrld         $0x8,%xmm1
  0x66,0x0f,0xdb,0xc8,                        //pand          %xmm0,%xmm1
  0x66,0x41,0x0f,0x6f,0xd0,                   //movdqa        %xmm8,%xmm2
  0x66,0x0f,0x72,0xd2,0x10,                   //psrld         $0x10,%xmm2
  0x66,0x0f,0xdb,0xd0,                        //pand          %xmm0,%xmm2
  0x66,0x41,0x0f,0xdb,0xc0,                   //pand          %xmm8,%xmm0
  0x66,0x48,0x0f,0x3a,0x16,0xc1,0x01,         //pextrq        $0x1,%xmm0,%rcx
  0x41,0x89,0xc9,                             //mov           %ecx,%r9d
  0x48,0xc1,0xe9,0x20,                        //shr           $0x20,%rcx
  0x66,0x49,0x0f,0x7e,0xc2,                   //movq          %xmm0,%r10
  0x45,0x89,0xd3,                             //mov           %r10d,%r11d
  0x49,0xc1,0xea,0x20,                        //shr           $0x20,%r10
  0xf3,0x43,0x0f,0x10,0x04,0x98,              //movss         (%r8,%r11,4),%xmm0
  0x66,0x43,0x0f,0x3a,0x21,0x04,0x90,0x10,    //insertps      $0x10,(%r8,%r10,4),%xmm0
  0x66,0x43,0x0f,0x3a,0x21,0x04,0x88,0x20,    //insertps      $0x20,(%r8,%r9,4),%xmm0
  0x66,0x41,0x0f,0x3a,0x21,0x04,0x88,0x30,    //insertps      $0x30,(%r8,%rcx,4),%xmm0
  0x48,0x8b,0x48,0x10,                        //mov           0x10(%rax),%rcx
  0x66,0x49,0x0f,0x3a,0x16,0xc8,0x01,         //pextrq        $0x1,%xmm1,%r8
  0x45,0x89,0xc1,                             //mov           %r8d,%r9d
  0x49,0xc1,0xe8,0x20,                        //shr           $0x20,%r8
  0x66,0x49,0x0f,0x7e,0xca,                   //movq          %xmm1,%r10
  0x45,0x89,0xd3,                             //mov           %r10d,%r11d
  0x49,0xc1,0xea,0x20,                        //shr           $0x20,%r10
  0xf3,0x42,0x0f,0x10,0x0c,0x99,              //movss         (%rcx,%r11,4),%xmm1
  0x66,0x42,0x0f,0x3a,0x21,0x0c,0x91,0x10,    //insertps      $0x10,(%rcx,%r10,4),%xmm1
  0xf3,0x42,0x0f,0x10,0x1c,0x89,              //movss         (%rcx,%r9,4),%xmm3
  0x66,0x0f,0x3a,0x21,0xcb,0x20,              //insertps      $0x20,%xmm3,%xmm1
  0xf3,0x42,0x0f,0x10,0x1c,0x81,              //movss         (%rcx,%r8,4),%xmm3
  0x66,0x0f,0x3a,0x21,0xcb,0x30,              //insertps      $0x30,%xmm3,%xmm1
  0x48,0x8b,0x40,0x18,                        //mov           0x18(%rax),%rax
  0x66,0x48,0x0f,0x3a,0x16,0xd1,0x01,         //pextrq        $0x1,%xmm2,%rcx
  0x41,0x89,0xc8,                             //mov           %ecx,%r8d
  0x48,0xc1,0xe9,0x20,                        //shr           $0x20,%rcx
  0x66,0x49,0x0f,0x7e,0xd1,                   //movq          %xmm2,%r9
  0x45,0x89,0xca,                             //mov           %r9d,%r10d
  0x49,0xc1,0xe9,0x20,                        //shr           $0x20,%r9
  0xf3,0x42,0x0f,0x10,0x14,0x90,              //movss         (%rax,%r10,4),%xmm2
  0x66,0x42,0x0f,0x3a,0x21,0x14,0x88,0x10,    //insertps      $0x10,(%rax,%r9,4),%xmm2
  0xf3,0x42,0x0f,0x10,0x1c,0x80,              //movss         (%rax,%r8,4),%xmm3
  0x66,0x0f,0x3a,0x21,0xd3,0x20,              //insertps      $0x20,%xmm3,%xmm2
  0xf3,0x0f,0x10,0x1c,0x88,                   //movss         (%rax,%rcx,4),%xmm3
  0x66,0x0f,0x3a,0x21,0xd3,0x30,              //insertps      $0x30,%xmm3,%xmm2
  0x66,0x41,0x0f,0x72,0xd0,0x18,              //psrld         $0x18,%xmm8
  0x45,0x0f,0x5b,0xc0,                        //cvtdq2ps      %xmm8,%xmm8
  0xf3,0x0f,0x10,0x5a,0x0c,                   //movss         0xc(%rdx),%xmm3
  0x0f,0xc6,0xdb,0x00,                        //shufps        $0x0,%xmm3,%xmm3
  0x41,0x0f,0x59,0xd8,                        //mulps         %xmm8,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_load_a8_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x66,0x0f,0x38,0x31,0x04,0x38,              //pmovzxbd      (%rax,%rdi,1),%xmm0
  0x0f,0x5b,0xc0,                             //cvtdq2ps      %xmm0,%xmm0
  0xf3,0x0f,0x10,0x5a,0x0c,                   //movss         0xc(%rdx),%xmm3
  0x0f,0xc6,0xdb,0x00,                        //shufps        $0x0,%xmm3,%xmm3
  0x0f,0x59,0xd8,                             //mulps         %xmm0,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x57,0xc0,                             //xorps         %xmm0,%xmm0
  0x0f,0x57,0xc9,                             //xorps         %xmm1,%xmm1
  0x0f,0x57,0xd2,                             //xorps         %xmm2,%xmm2
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_store_a8_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0xf3,0x44,0x0f,0x10,0x42,0x08,              //movss         0x8(%rdx),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x44,0x0f,0x59,0xc3,                        //mulps         %xmm3,%xmm8
  0x66,0x45,0x0f,0x5b,0xc0,                   //cvtps2dq      %xmm8,%xmm8
  0x66,0x45,0x0f,0x38,0x2b,0xc0,              //packusdw      %xmm8,%xmm8
  0x66,0x45,0x0f,0x67,0xc0,                   //packuswb      %xmm8,%xmm8
  0x66,0x44,0x0f,0x7e,0x04,0x38,              //movd          %xmm8,(%rax,%rdi,1)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_load_565_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x66,0x44,0x0f,0x38,0x33,0x0c,0x78,         //pmovzxwd      (%rax,%rdi,2),%xmm9
  0x66,0x0f,0x6e,0x42,0x68,                   //movd          0x68(%rdx),%xmm0
  0x66,0x0f,0x70,0xc0,0x00,                   //pshufd        $0x0,%xmm0,%xmm0
  0x66,0x41,0x0f,0xdb,0xc1,                   //pand          %xmm9,%xmm0
  0x0f,0x5b,0xc8,                             //cvtdq2ps      %xmm0,%xmm1
  0xf3,0x0f,0x10,0x1a,                        //movss         (%rdx),%xmm3
  0xf3,0x0f,0x10,0x42,0x74,                   //movss         0x74(%rdx),%xmm0
  0x0f,0xc6,0xc0,0x00,                        //shufps        $0x0,%xmm0,%xmm0
  0x0f,0x59,0xc1,                             //mulps         %xmm1,%xmm0
  0x66,0x0f,0x6e,0x4a,0x6c,                   //movd          0x6c(%rdx),%xmm1
  0x66,0x0f,0x70,0xc9,0x00,                   //pshufd        $0x0,%xmm1,%xmm1
  0x66,0x41,0x0f,0xdb,0xc9,                   //pand          %xmm9,%xmm1
  0x44,0x0f,0x5b,0xc1,                        //cvtdq2ps      %xmm1,%xmm8
  0xf3,0x0f,0x10,0x4a,0x78,                   //movss         0x78(%rdx),%xmm1
  0x0f,0xc6,0xc9,0x00,                        //shufps        $0x0,%xmm1,%xmm1
  0x41,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm1
  0x66,0x0f,0x6e,0x52,0x70,                   //movd          0x70(%rdx),%xmm2
  0x66,0x0f,0x70,0xd2,0x00,                   //pshufd        $0x0,%xmm2,%xmm2
  0x66,0x41,0x0f,0xdb,0xd1,                   //pand          %xmm9,%xmm2
  0x44,0x0f,0x5b,0xc2,                        //cvtdq2ps      %xmm2,%xmm8
  0xf3,0x0f,0x10,0x52,0x7c,                   //movss         0x7c(%rdx),%xmm2
  0x0f,0xc6,0xd2,0x00,                        //shufps        $0x0,%xmm2,%xmm2
  0x41,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm2
  0x0f,0xc6,0xdb,0x00,                        //shufps        $0x0,%xmm3,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_store_565_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0xf3,0x44,0x0f,0x10,0x82,0x80,0x00,0x00,0x00,//movss         0x80(%rdx),%xmm8
  0xf3,0x44,0x0f,0x10,0x8a,0x84,0x00,0x00,0x00,//movss         0x84(%rdx),%xmm9
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x45,0x0f,0x28,0xd0,                        //movaps        %xmm8,%xmm10
  0x44,0x0f,0x59,0xd0,                        //mulps         %xmm0,%xmm10
  0x66,0x45,0x0f,0x5b,0xd2,                   //cvtps2dq      %xmm10,%xmm10
  0x66,0x41,0x0f,0x72,0xf2,0x0b,              //pslld         $0xb,%xmm10
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x44,0x0f,0x59,0xc9,                        //mulps         %xmm1,%xmm9
  0x66,0x45,0x0f,0x5b,0xc9,                   //cvtps2dq      %xmm9,%xmm9
  0x66,0x41,0x0f,0x72,0xf1,0x05,              //pslld         $0x5,%xmm9
  0x66,0x45,0x0f,0xeb,0xca,                   //por           %xmm10,%xmm9
  0x44,0x0f,0x59,0xc2,                        //mulps         %xmm2,%xmm8
  0x66,0x45,0x0f,0x5b,0xc0,                   //cvtps2dq      %xmm8,%xmm8
  0x66,0x45,0x0f,0x56,0xc1,                   //orpd          %xmm9,%xmm8
  0x66,0x45,0x0f,0x38,0x2b,0xc0,              //packusdw      %xmm8,%xmm8
  0x66,0x44,0x0f,0xd6,0x04,0x78,              //movq          %xmm8,(%rax,%rdi,2)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_load_8888_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0xf3,0x0f,0x6f,0x1c,0xb8,                   //movdqu        (%rax,%rdi,4),%xmm3
  0x66,0x0f,0x6e,0x42,0x10,                   //movd          0x10(%rdx),%xmm0
  0x66,0x0f,0x70,0xc0,0x00,                   //pshufd        $0x0,%xmm0,%xmm0
  0x66,0x0f,0x6f,0xcb,                        //movdqa        %xmm3,%xmm1
  0x66,0x0f,0x72,0xd1,0x08,                   //psrld         $0x8,%xmm1
  0x66,0x0f,0xdb,0xc8,                        //pand          %xmm0,%xmm1
  0x66,0x0f,0x6f,0xd3,                        //movdqa        %xmm3,%xmm2
  0x66,0x0f,0x72,0xd2,0x10,                   //psrld         $0x10,%xmm2
  0x66,0x0f,0xdb,0xd0,                        //pand          %xmm0,%xmm2
  0x66,0x0f,0xdb,0xc3,                        //pand          %xmm3,%xmm0
  0x0f,0x5b,0xc0,                             //cvtdq2ps      %xmm0,%xmm0
  0xf3,0x44,0x0f,0x10,0x42,0x0c,              //movss         0xc(%rdx),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x41,0x0f,0x59,0xc0,                        //mulps         %xmm8,%xmm0
  0x0f,0x5b,0xc9,                             //cvtdq2ps      %xmm1,%xmm1
  0x41,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm1
  0x0f,0x5b,0xd2,                             //cvtdq2ps      %xmm2,%xmm2
  0x41,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm2
  0x66,0x0f,0x72,0xd3,0x18,                   //psrld         $0x18,%xmm3
  0x0f,0x5b,0xdb,                             //cvtdq2ps      %xmm3,%xmm3
  0x41,0x0f,0x59,0xd8,                        //mulps         %xmm8,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_store_8888_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0xf3,0x44,0x0f,0x10,0x42,0x08,              //movss         0x8(%rdx),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x45,0x0f,0x28,0xc8,                        //movaps        %xmm8,%xmm9
  0x44,0x0f,0x59,0xc8,                        //mulps         %xmm0,%xmm9
  0x66,0x45,0x0f,0x5b,0xc9,                   //cvtps2dq      %xmm9,%xmm9
  0x45,0x0f,0x28,0xd0,                        //movaps        %xmm8,%xmm10
  0x44,0x0f,0x59,0xd1,                        //mulps         %xmm1,%xmm10
  0x66,0x45,0x0f,0x5b,0xd2,                   //cvtps2dq      %xmm10,%xmm10
  0x66,0x41,0x0f,0x72,0xf2,0x08,              //pslld         $0x8,%xmm10
  0x66,0x45,0x0f,0xeb,0xd1,                   //por           %xmm9,%xmm10
  0x45,0x0f,0x28,0xc8,                        //movaps        %xmm8,%xmm9
  0x44,0x0f,0x59,0xca,                        //mulps         %xmm2,%xmm9
  0x66,0x45,0x0f,0x5b,0xc9,                   //cvtps2dq      %xmm9,%xmm9
  0x66,0x41,0x0f,0x72,0xf1,0x10,              //pslld         $0x10,%xmm9
  0x44,0x0f,0x59,0xc3,                        //mulps         %xmm3,%xmm8
  0x66,0x45,0x0f,0x5b,0xc0,                   //cvtps2dq      %xmm8,%xmm8
  0x66,0x41,0x0f,0x72,0xf0,0x18,              //pslld         $0x18,%xmm8
  0x66,0x45,0x0f,0xeb,0xc1,                   //por           %xmm9,%xmm8
  0x66,0x45,0x0f,0xeb,0xc2,                   //por           %xmm10,%xmm8
  0xf3,0x44,0x0f,0x7f,0x04,0xb8,              //movdqu        %xmm8,(%rax,%rdi,4)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_load_f16_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0xf3,0x0f,0x6f,0x04,0xf8,                   //movdqu        (%rax,%rdi,8),%xmm0
  0xf3,0x0f,0x6f,0x4c,0xf8,0x10,              //movdqu        0x10(%rax,%rdi,8),%xmm1
  0x66,0x0f,0x6f,0xd0,                        //movdqa        %xmm0,%xmm2
  0x66,0x0f,0x61,0xd1,                        //punpcklwd     %xmm1,%xmm2
  0x66,0x0f,0x69,0xc1,                        //punpckhwd     %xmm1,%xmm0
  0x66,0x44,0x0f,0x6f,0xc2,                   //movdqa        %xmm2,%xmm8
  0x66,0x44,0x0f,0x61,0xc0,                   //punpcklwd     %xmm0,%xmm8
  0x66,0x0f,0x69,0xd0,                        //punpckhwd     %xmm0,%xmm2
  0x66,0x0f,0x6e,0x42,0x64,                   //movd          0x64(%rdx),%xmm0
  0x66,0x0f,0x70,0xd8,0x00,                   //pshufd        $0x0,%xmm0,%xmm3
  0x66,0x0f,0x6f,0xcb,                        //movdqa        %xmm3,%xmm1
  0x66,0x41,0x0f,0x65,0xc8,                   //pcmpgtw       %xmm8,%xmm1
  0x66,0x41,0x0f,0xdf,0xc8,                   //pandn         %xmm8,%xmm1
  0x66,0x0f,0x65,0xda,                        //pcmpgtw       %xmm2,%xmm3
  0x66,0x0f,0xdf,0xda,                        //pandn         %xmm2,%xmm3
  0x66,0x0f,0x38,0x33,0xc1,                   //pmovzxwd      %xmm1,%xmm0
  0x66,0x0f,0x72,0xf0,0x0d,                   //pslld         $0xd,%xmm0
  0x66,0x0f,0x6e,0x52,0x5c,                   //movd          0x5c(%rdx),%xmm2
  0x66,0x44,0x0f,0x70,0xc2,0x00,              //pshufd        $0x0,%xmm2,%xmm8
  0x41,0x0f,0x59,0xc0,                        //mulps         %xmm8,%xmm0
  0x66,0x45,0x0f,0xef,0xc9,                   //pxor          %xmm9,%xmm9
  0x66,0x41,0x0f,0x69,0xc9,                   //punpckhwd     %xmm9,%xmm1
  0x66,0x0f,0x72,0xf1,0x0d,                   //pslld         $0xd,%xmm1
  0x41,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm1
  0x66,0x0f,0x38,0x33,0xd3,                   //pmovzxwd      %xmm3,%xmm2
  0x66,0x0f,0x72,0xf2,0x0d,                   //pslld         $0xd,%xmm2
  0x41,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm2
  0x66,0x41,0x0f,0x69,0xd9,                   //punpckhwd     %xmm9,%xmm3
  0x66,0x0f,0x72,0xf3,0x0d,                   //pslld         $0xd,%xmm3
  0x41,0x0f,0x59,0xd8,                        //mulps         %xmm8,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_store_f16_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x66,0x44,0x0f,0x6e,0x42,0x60,              //movd          0x60(%rdx),%xmm8
  0x66,0x45,0x0f,0x70,0xc0,0x00,              //pshufd        $0x0,%xmm8,%xmm8
  0x66,0x45,0x0f,0x6f,0xc8,                   //movdqa        %xmm8,%xmm9
  0x44,0x0f,0x59,0xc8,                        //mulps         %xmm0,%xmm9
  0x66,0x41,0x0f,0x72,0xd1,0x0d,              //psrld         $0xd,%xmm9
  0x66,0x45,0x0f,0x6f,0xd0,                   //movdqa        %xmm8,%xmm10
  0x44,0x0f,0x59,0xd1,                        //mulps         %xmm1,%xmm10
  0x66,0x41,0x0f,0x72,0xd2,0x0d,              //psrld         $0xd,%xmm10
  0x66,0x45,0x0f,0x6f,0xd8,                   //movdqa        %xmm8,%xmm11
  0x44,0x0f,0x59,0xda,                        //mulps         %xmm2,%xmm11
  0x66,0x41,0x0f,0x72,0xd3,0x0d,              //psrld         $0xd,%xmm11
  0x44,0x0f,0x59,0xc3,                        //mulps         %xmm3,%xmm8
  0x66,0x41,0x0f,0x72,0xd0,0x0d,              //psrld         $0xd,%xmm8
  0x66,0x41,0x0f,0x73,0xfa,0x02,              //pslldq        $0x2,%xmm10
  0x66,0x45,0x0f,0xeb,0xd1,                   //por           %xmm9,%xmm10
  0x66,0x41,0x0f,0x73,0xf8,0x02,              //pslldq        $0x2,%xmm8
  0x66,0x45,0x0f,0xeb,0xc3,                   //por           %xmm11,%xmm8
  0x66,0x45,0x0f,0x6f,0xca,                   //movdqa        %xmm10,%xmm9
  0x66,0x45,0x0f,0x62,0xc8,                   //punpckldq     %xmm8,%xmm9
  0xf3,0x44,0x0f,0x7f,0x0c,0xf8,              //movdqu        %xmm9,(%rax,%rdi,8)
  0x66,0x45,0x0f,0x6a,0xd0,                   //punpckhdq     %xmm8,%xmm10
  0xf3,0x44,0x0f,0x7f,0x54,0xf8,0x10,         //movdqu        %xmm10,0x10(%rax,%rdi,8)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_store_f32_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x48,0x89,0xf9,                             //mov           %rdi,%rcx
  0x48,0xc1,0xe1,0x04,                        //shl           $0x4,%rcx
  0x44,0x0f,0x28,0xc0,                        //movaps        %xmm0,%xmm8
  0x44,0x0f,0x28,0xc8,                        //movaps        %xmm0,%xmm9
  0x44,0x0f,0x14,0xc9,                        //unpcklps      %xmm1,%xmm9
  0x44,0x0f,0x28,0xd2,                        //movaps        %xmm2,%xmm10
  0x44,0x0f,0x28,0xda,                        //movaps        %xmm2,%xmm11
  0x44,0x0f,0x14,0xdb,                        //unpcklps      %xmm3,%xmm11
  0x44,0x0f,0x15,0xc1,                        //unpckhps      %xmm1,%xmm8
  0x44,0x0f,0x15,0xd3,                        //unpckhps      %xmm3,%xmm10
  0x45,0x0f,0x28,0xe1,                        //movaps        %xmm9,%xmm12
  0x66,0x45,0x0f,0x14,0xe3,                   //unpcklpd      %xmm11,%xmm12
  0x66,0x45,0x0f,0x15,0xcb,                   //unpckhpd      %xmm11,%xmm9
  0x45,0x0f,0x28,0xd8,                        //movaps        %xmm8,%xmm11
  0x66,0x45,0x0f,0x14,0xda,                   //unpcklpd      %xmm10,%xmm11
  0x66,0x45,0x0f,0x15,0xc2,                   //unpckhpd      %xmm10,%xmm8
  0x66,0x44,0x0f,0x11,0x24,0x08,              //movupd        %xmm12,(%rax,%rcx,1)
  0x66,0x44,0x0f,0x11,0x4c,0x08,0x10,         //movupd        %xmm9,0x10(%rax,%rcx,1)
  0x66,0x44,0x0f,0x11,0x5c,0x08,0x20,         //movupd        %xmm11,0x20(%rax,%rcx,1)
  0x66,0x44,0x0f,0x11,0x44,0x08,0x30,         //movupd        %xmm8,0x30(%rax,%rcx,1)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_x_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x45,0x0f,0x57,0xc0,                        //xorps         %xmm8,%xmm8
  0x44,0x0f,0x5f,0xc0,                        //maxps         %xmm0,%xmm8
  0xf3,0x44,0x0f,0x10,0x08,                   //movss         (%rax),%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x66,0x0f,0x76,0xc0,                        //pcmpeqd       %xmm0,%xmm0
  0x66,0x41,0x0f,0xfe,0xc1,                   //paddd         %xmm9,%xmm0
  0x44,0x0f,0x5d,0xc0,                        //minps         %xmm0,%xmm8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x41,0x0f,0x28,0xc0,                        //movaps        %xmm8,%xmm0
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_y_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x45,0x0f,0x57,0xc0,                        //xorps         %xmm8,%xmm8
  0x44,0x0f,0x5f,0xc1,                        //maxps         %xmm1,%xmm8
  0xf3,0x44,0x0f,0x10,0x08,                   //movss         (%rax),%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x66,0x0f,0x76,0xc9,                        //pcmpeqd       %xmm1,%xmm1
  0x66,0x41,0x0f,0xfe,0xc9,                   //paddd         %xmm9,%xmm1
  0x44,0x0f,0x5d,0xc1,                        //minps         %xmm1,%xmm8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x41,0x0f,0x28,0xc8,                        //movaps        %xmm8,%xmm1
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_repeat_x_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x44,0x0f,0x10,0x00,                   //movss         (%rax),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x44,0x0f,0x28,0xc8,                        //movaps        %xmm0,%xmm9
  0x45,0x0f,0x5e,0xc8,                        //divps         %xmm8,%xmm9
  0x66,0x45,0x0f,0x3a,0x08,0xc9,0x01,         //roundps       $0x1,%xmm9,%xmm9
  0x45,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm9
  0x41,0x0f,0x5c,0xc1,                        //subps         %xmm9,%xmm0
  0x66,0x45,0x0f,0x76,0xc9,                   //pcmpeqd       %xmm9,%xmm9
  0x66,0x45,0x0f,0xfe,0xc8,                   //paddd         %xmm8,%xmm9
  0x41,0x0f,0x5d,0xc1,                        //minps         %xmm9,%xmm0
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_repeat_y_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x44,0x0f,0x10,0x00,                   //movss         (%rax),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x44,0x0f,0x28,0xc9,                        //movaps        %xmm1,%xmm9
  0x45,0x0f,0x5e,0xc8,                        //divps         %xmm8,%xmm9
  0x66,0x45,0x0f,0x3a,0x08,0xc9,0x01,         //roundps       $0x1,%xmm9,%xmm9
  0x45,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm9
  0x41,0x0f,0x5c,0xc9,                        //subps         %xmm9,%xmm1
  0x66,0x45,0x0f,0x76,0xc9,                   //pcmpeqd       %xmm9,%xmm9
  0x66,0x45,0x0f,0xfe,0xc8,                   //paddd         %xmm8,%xmm9
  0x41,0x0f,0x5d,0xc9,                        //minps         %xmm9,%xmm1
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_mirror_x_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x44,0x0f,0x10,0x00,                   //movss         (%rax),%xmm8
  0x45,0x0f,0x28,0xc8,                        //movaps        %xmm8,%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x41,0x0f,0x5c,0xc1,                        //subps         %xmm9,%xmm0
  0xf3,0x45,0x0f,0x58,0xc0,                   //addss         %xmm8,%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x44,0x0f,0x28,0xd0,                        //movaps        %xmm0,%xmm10
  0x45,0x0f,0x5e,0xd0,                        //divps         %xmm8,%xmm10
  0x66,0x45,0x0f,0x3a,0x08,0xd2,0x01,         //roundps       $0x1,%xmm10,%xmm10
  0x45,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm10
  0x41,0x0f,0x5c,0xc2,                        //subps         %xmm10,%xmm0
  0x41,0x0f,0x5c,0xc1,                        //subps         %xmm9,%xmm0
  0x45,0x0f,0x57,0xc0,                        //xorps         %xmm8,%xmm8
  0x44,0x0f,0x5c,0xc0,                        //subps         %xmm0,%xmm8
  0x41,0x0f,0x54,0xc0,                        //andps         %xmm8,%xmm0
  0x66,0x45,0x0f,0x76,0xc0,                   //pcmpeqd       %xmm8,%xmm8
  0x66,0x45,0x0f,0xfe,0xc1,                   //paddd         %xmm9,%xmm8
  0x41,0x0f,0x5d,0xc0,                        //minps         %xmm8,%xmm0
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_mirror_y_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x44,0x0f,0x10,0x00,                   //movss         (%rax),%xmm8
  0x45,0x0f,0x28,0xc8,                        //movaps        %xmm8,%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x41,0x0f,0x5c,0xc9,                        //subps         %xmm9,%xmm1
  0xf3,0x45,0x0f,0x58,0xc0,                   //addss         %xmm8,%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x44,0x0f,0x28,0xd1,                        //movaps        %xmm1,%xmm10
  0x45,0x0f,0x5e,0xd0,                        //divps         %xmm8,%xmm10
  0x66,0x45,0x0f,0x3a,0x08,0xd2,0x01,         //roundps       $0x1,%xmm10,%xmm10
  0x45,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm10
  0x41,0x0f,0x5c,0xca,                        //subps         %xmm10,%xmm1
  0x41,0x0f,0x5c,0xc9,                        //subps         %xmm9,%xmm1
  0x45,0x0f,0x57,0xc0,                        //xorps         %xmm8,%xmm8
  0x44,0x0f,0x5c,0xc1,                        //subps         %xmm1,%xmm8
  0x41,0x0f,0x54,0xc8,                        //andps         %xmm8,%xmm1
  0x66,0x45,0x0f,0x76,0xc0,                   //pcmpeqd       %xmm8,%xmm8
  0x66,0x45,0x0f,0xfe,0xc1,                   //paddd         %xmm9,%xmm8
  0x41,0x0f,0x5d,0xc8,                        //minps         %xmm8,%xmm1
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_matrix_2x3_sse41[] = {
  0x44,0x0f,0x28,0xc9,                        //movaps        %xmm1,%xmm9
  0x44,0x0f,0x28,0xc0,                        //movaps        %xmm0,%xmm8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x0f,0x10,0x00,                        //movss         (%rax),%xmm0
  0xf3,0x0f,0x10,0x48,0x04,                   //movss         0x4(%rax),%xmm1
  0x0f,0xc6,0xc0,0x00,                        //shufps        $0x0,%xmm0,%xmm0
  0xf3,0x44,0x0f,0x10,0x50,0x08,              //movss         0x8(%rax),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0xf3,0x44,0x0f,0x10,0x58,0x10,              //movss         0x10(%rax),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0x45,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm10
  0x45,0x0f,0x58,0xd3,                        //addps         %xmm11,%xmm10
  0x41,0x0f,0x59,0xc0,                        //mulps         %xmm8,%xmm0
  0x41,0x0f,0x58,0xc2,                        //addps         %xmm10,%xmm0
  0x0f,0xc6,0xc9,0x00,                        //shufps        $0x0,%xmm1,%xmm1
  0xf3,0x44,0x0f,0x10,0x50,0x0c,              //movss         0xc(%rax),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0xf3,0x44,0x0f,0x10,0x58,0x14,              //movss         0x14(%rax),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0x45,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm10
  0x45,0x0f,0x58,0xd3,                        //addps         %xmm11,%xmm10
  0x41,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm1
  0x41,0x0f,0x58,0xca,                        //addps         %xmm10,%xmm1
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_matrix_3x4_sse41[] = {
  0x44,0x0f,0x28,0xc9,                        //movaps        %xmm1,%xmm9
  0x44,0x0f,0x28,0xc0,                        //movaps        %xmm0,%xmm8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x0f,0x10,0x00,                        //movss         (%rax),%xmm0
  0xf3,0x0f,0x10,0x48,0x04,                   //movss         0x4(%rax),%xmm1
  0x0f,0xc6,0xc0,0x00,                        //shufps        $0x0,%xmm0,%xmm0
  0xf3,0x44,0x0f,0x10,0x50,0x0c,              //movss         0xc(%rax),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0xf3,0x44,0x0f,0x10,0x58,0x18,              //movss         0x18(%rax),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0xf3,0x44,0x0f,0x10,0x60,0x24,              //movss         0x24(%rax),%xmm12
  0x45,0x0f,0xc6,0xe4,0x00,                   //shufps        $0x0,%xmm12,%xmm12
  0x44,0x0f,0x59,0xda,                        //mulps         %xmm2,%xmm11
  0x45,0x0f,0x58,0xdc,                        //addps         %xmm12,%xmm11
  0x45,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm10
  0x45,0x0f,0x58,0xd3,                        //addps         %xmm11,%xmm10
  0x41,0x0f,0x59,0xc0,                        //mulps         %xmm8,%xmm0
  0x41,0x0f,0x58,0xc2,                        //addps         %xmm10,%xmm0
  0x0f,0xc6,0xc9,0x00,                        //shufps        $0x0,%xmm1,%xmm1
  0xf3,0x44,0x0f,0x10,0x50,0x10,              //movss         0x10(%rax),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0xf3,0x44,0x0f,0x10,0x58,0x1c,              //movss         0x1c(%rax),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0xf3,0x44,0x0f,0x10,0x60,0x28,              //movss         0x28(%rax),%xmm12
  0x45,0x0f,0xc6,0xe4,0x00,                   //shufps        $0x0,%xmm12,%xmm12
  0x44,0x0f,0x59,0xda,                        //mulps         %xmm2,%xmm11
  0x45,0x0f,0x58,0xdc,                        //addps         %xmm12,%xmm11
  0x45,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm10
  0x45,0x0f,0x58,0xd3,                        //addps         %xmm11,%xmm10
  0x41,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm1
  0x41,0x0f,0x58,0xca,                        //addps         %xmm10,%xmm1
  0xf3,0x44,0x0f,0x10,0x50,0x08,              //movss         0x8(%rax),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0xf3,0x44,0x0f,0x10,0x58,0x14,              //movss         0x14(%rax),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0xf3,0x44,0x0f,0x10,0x60,0x20,              //movss         0x20(%rax),%xmm12
  0x45,0x0f,0xc6,0xe4,0x00,                   //shufps        $0x0,%xmm12,%xmm12
  0xf3,0x44,0x0f,0x10,0x68,0x2c,              //movss         0x2c(%rax),%xmm13
  0x45,0x0f,0xc6,0xed,0x00,                   //shufps        $0x0,%xmm13,%xmm13
  0x44,0x0f,0x59,0xe2,                        //mulps         %xmm2,%xmm12
  0x45,0x0f,0x58,0xe5,                        //addps         %xmm13,%xmm12
  0x45,0x0f,0x59,0xd9,                        //mulps         %xmm9,%xmm11
  0x45,0x0f,0x58,0xdc,                        //addps         %xmm12,%xmm11
  0x45,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm10
  0x45,0x0f,0x58,0xd3,                        //addps         %xmm11,%xmm10
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x41,0x0f,0x28,0xd2,                        //movaps        %xmm10,%xmm2
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_matrix_perspective_sse41[] = {
  0x44,0x0f,0x28,0xc0,                        //movaps        %xmm0,%xmm8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x0f,0x10,0x00,                        //movss         (%rax),%xmm0
  0xf3,0x44,0x0f,0x10,0x48,0x04,              //movss         0x4(%rax),%xmm9
  0x0f,0xc6,0xc0,0x00,                        //shufps        $0x0,%xmm0,%xmm0
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0xf3,0x44,0x0f,0x10,0x50,0x08,              //movss         0x8(%rax),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0x44,0x0f,0x59,0xc9,                        //mulps         %xmm1,%xmm9
  0x45,0x0f,0x58,0xca,                        //addps         %xmm10,%xmm9
  0x41,0x0f,0x59,0xc0,                        //mulps         %xmm8,%xmm0
  0x41,0x0f,0x58,0xc1,                        //addps         %xmm9,%xmm0
  0xf3,0x44,0x0f,0x10,0x48,0x0c,              //movss         0xc(%rax),%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0xf3,0x44,0x0f,0x10,0x50,0x10,              //movss         0x10(%rax),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0xf3,0x44,0x0f,0x10,0x58,0x14,              //movss         0x14(%rax),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0x44,0x0f,0x59,0xd1,                        //mulps         %xmm1,%xmm10
  0x45,0x0f,0x58,0xd3,                        //addps         %xmm11,%xmm10
  0x45,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm9
  0x45,0x0f,0x58,0xca,                        //addps         %xmm10,%xmm9
  0xf3,0x44,0x0f,0x10,0x50,0x18,              //movss         0x18(%rax),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0xf3,0x44,0x0f,0x10,0x58,0x1c,              //movss         0x1c(%rax),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0xf3,0x44,0x0f,0x10,0x60,0x20,              //movss         0x20(%rax),%xmm12
  0x45,0x0f,0xc6,0xe4,0x00,                   //shufps        $0x0,%xmm12,%xmm12
  0x44,0x0f,0x59,0xd9,                        //mulps         %xmm1,%xmm11
  0x45,0x0f,0x58,0xdc,                        //addps         %xmm12,%xmm11
  0x45,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm10
  0x45,0x0f,0x58,0xd3,                        //addps         %xmm11,%xmm10
  0x41,0x0f,0x53,0xca,                        //rcpps         %xmm10,%xmm1
  0x0f,0x59,0xc1,                             //mulps         %xmm1,%xmm0
  0x44,0x0f,0x59,0xc9,                        //mulps         %xmm1,%xmm9
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x41,0x0f,0x28,0xc9,                        //movaps        %xmm9,%xmm1
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_linear_gradient_2stops_sse41[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x44,0x0f,0x10,0x08,                        //movups        (%rax),%xmm9
  0x0f,0x10,0x58,0x10,                        //movups        0x10(%rax),%xmm3
  0x44,0x0f,0x28,0xc3,                        //movaps        %xmm3,%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x41,0x0f,0x28,0xc9,                        //movaps        %xmm9,%xmm1
  0x0f,0xc6,0xc9,0x00,                        //shufps        $0x0,%xmm1,%xmm1
  0x44,0x0f,0x59,0xc0,                        //mulps         %xmm0,%xmm8
  0x44,0x0f,0x58,0xc1,                        //addps         %xmm1,%xmm8
  0x0f,0x28,0xcb,                             //movaps        %xmm3,%xmm1
  0x0f,0xc6,0xc9,0x55,                        //shufps        $0x55,%xmm1,%xmm1
  0x41,0x0f,0x28,0xd1,                        //movaps        %xmm9,%xmm2
  0x0f,0xc6,0xd2,0x55,                        //shufps        $0x55,%xmm2,%xmm2
  0x0f,0x59,0xc8,                             //mulps         %xmm0,%xmm1
  0x0f,0x58,0xca,                             //addps         %xmm2,%xmm1
  0x0f,0x28,0xd3,                             //movaps        %xmm3,%xmm2
  0x0f,0xc6,0xd2,0xaa,                        //shufps        $0xaa,%xmm2,%xmm2
  0x45,0x0f,0x28,0xd1,                        //movaps        %xmm9,%xmm10
  0x45,0x0f,0xc6,0xd2,0xaa,                   //shufps        $0xaa,%xmm10,%xmm10
  0x0f,0x59,0xd0,                             //mulps         %xmm0,%xmm2
  0x41,0x0f,0x58,0xd2,                        //addps         %xmm10,%xmm2
  0x0f,0xc6,0xdb,0xff,                        //shufps        $0xff,%xmm3,%xmm3
  0x45,0x0f,0xc6,0xc9,0xff,                   //shufps        $0xff,%xmm9,%xmm9
  0x0f,0x59,0xd8,                             //mulps         %xmm0,%xmm3
  0x41,0x0f,0x58,0xd9,                        //addps         %xmm9,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x41,0x0f,0x28,0xc0,                        //movaps        %xmm8,%xmm0
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_start_pipeline_sse2[] = {
  0x41,0x57,                                  //push          %r15
  0x41,0x56,                                  //push          %r14
  0x41,0x55,                                  //push          %r13
  0x41,0x54,                                  //push          %r12
  0x53,                                       //push          %rbx
  0x49,0x89,0xcf,                             //mov           %rcx,%r15
  0x49,0x89,0xd6,                             //mov           %rdx,%r14
  0x48,0x89,0xfb,                             //mov           %rdi,%rbx
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x49,0x89,0xc4,                             //mov           %rax,%r12
  0x49,0x89,0xf5,                             //mov           %rsi,%r13
  0x48,0x8d,0x43,0x04,                        //lea           0x4(%rbx),%rax
  0x4c,0x39,0xf8,                             //cmp           %r15,%rax
  0x76,0x05,                                  //jbe           28 <_sk_start_pipeline_sse2+0x28>
  0x48,0x89,0xd8,                             //mov           %rbx,%rax
  0xeb,0x34,                                  //jmp           5c <_sk_start_pipeline_sse2+0x5c>
  0x0f,0x57,0xc0,                             //xorps         %xmm0,%xmm0
  0x0f,0x57,0xc9,                             //xorps         %xmm1,%xmm1
  0x0f,0x57,0xd2,                             //xorps         %xmm2,%xmm2
  0x0f,0x57,0xdb,                             //xorps         %xmm3,%xmm3
  0x0f,0x57,0xe4,                             //xorps         %xmm4,%xmm4
  0x0f,0x57,0xed,                             //xorps         %xmm5,%xmm5
  0x0f,0x57,0xf6,                             //xorps         %xmm6,%xmm6
  0x0f,0x57,0xff,                             //xorps         %xmm7,%xmm7
  0x48,0x89,0xdf,                             //mov           %rbx,%rdi
  0x4c,0x89,0xee,                             //mov           %r13,%rsi
  0x4c,0x89,0xf2,                             //mov           %r14,%rdx
  0x41,0xff,0xd4,                             //callq         *%r12
  0x48,0x8d,0x43,0x04,                        //lea           0x4(%rbx),%rax
  0x48,0x83,0xc3,0x08,                        //add           $0x8,%rbx
  0x4c,0x39,0xfb,                             //cmp           %r15,%rbx
  0x48,0x89,0xc3,                             //mov           %rax,%rbx
  0x76,0xcc,                                  //jbe           28 <_sk_start_pipeline_sse2+0x28>
  0x5b,                                       //pop           %rbx
  0x41,0x5c,                                  //pop           %r12
  0x41,0x5d,                                  //pop           %r13
  0x41,0x5e,                                  //pop           %r14
  0x41,0x5f,                                  //pop           %r15
  0xc3,                                       //retq
};

CODE const uint8_t sk_start_pipeline_ms_sse2[] = {
  0x56,                                       //push          %rsi
  0x57,                                       //push          %rdi
  0x48,0x81,0xec,0xa8,0x00,0x00,0x00,         //sub           $0xa8,%rsp
  0x44,0x0f,0x29,0xbc,0x24,0x90,0x00,0x00,0x00,//movaps        %xmm15,0x90(%rsp)
  0x44,0x0f,0x29,0xb4,0x24,0x80,0x00,0x00,0x00,//movaps        %xmm14,0x80(%rsp)
  0x44,0x0f,0x29,0x6c,0x24,0x70,              //movaps        %xmm13,0x70(%rsp)
  0x44,0x0f,0x29,0x64,0x24,0x60,              //movaps        %xmm12,0x60(%rsp)
  0x44,0x0f,0x29,0x5c,0x24,0x50,              //movaps        %xmm11,0x50(%rsp)
  0x44,0x0f,0x29,0x54,0x24,0x40,              //movaps        %xmm10,0x40(%rsp)
  0x44,0x0f,0x29,0x4c,0x24,0x30,              //movaps        %xmm9,0x30(%rsp)
  0x44,0x0f,0x29,0x44,0x24,0x20,              //movaps        %xmm8,0x20(%rsp)
  0x0f,0x29,0x7c,0x24,0x10,                   //movaps        %xmm7,0x10(%rsp)
  0x0f,0x29,0x34,0x24,                        //movaps        %xmm6,(%rsp)
  0x48,0x89,0xcf,                             //mov           %rcx,%rdi
  0x48,0x89,0xd6,                             //mov           %rdx,%rsi
  0x4c,0x89,0xc2,                             //mov           %r8,%rdx
  0x4c,0x89,0xc9,                             //mov           %r9,%rcx
  0xe8,0x00,0x00,0x00,0x00,                   //callq         bf <_sk_start_pipeline_ms_sse2+0x59>
  0x0f,0x28,0x34,0x24,                        //movaps        (%rsp),%xmm6
  0x0f,0x28,0x7c,0x24,0x10,                   //movaps        0x10(%rsp),%xmm7
  0x44,0x0f,0x28,0x44,0x24,0x20,              //movaps        0x20(%rsp),%xmm8
  0x44,0x0f,0x28,0x4c,0x24,0x30,              //movaps        0x30(%rsp),%xmm9
  0x44,0x0f,0x28,0x54,0x24,0x40,              //movaps        0x40(%rsp),%xmm10
  0x44,0x0f,0x28,0x5c,0x24,0x50,              //movaps        0x50(%rsp),%xmm11
  0x44,0x0f,0x28,0x64,0x24,0x60,              //movaps        0x60(%rsp),%xmm12
  0x44,0x0f,0x28,0x6c,0x24,0x70,              //movaps        0x70(%rsp),%xmm13
  0x44,0x0f,0x28,0xb4,0x24,0x80,0x00,0x00,0x00,//movaps        0x80(%rsp),%xmm14
  0x44,0x0f,0x28,0xbc,0x24,0x90,0x00,0x00,0x00,//movaps        0x90(%rsp),%xmm15
  0x48,0x81,0xc4,0xa8,0x00,0x00,0x00,         //add           $0xa8,%rsp
  0x5f,                                       //pop           %rdi
  0x5e,                                       //pop           %rsi
  0xc3,                                       //retq
};

CODE const uint8_t sk_just_return_sse2[] = {
  0xc3,                                       //retq
};

CODE const uint8_t sk_seed_shader_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x66,0x0f,0x6e,0xc7,                        //movd          %edi,%xmm0
  0x66,0x0f,0x70,0xc0,0x00,                   //pshufd        $0x0,%xmm0,%xmm0
  0x0f,0x5b,0xc8,                             //cvtdq2ps      %xmm0,%xmm1
  0xf3,0x0f,0x10,0x12,                        //movss         (%rdx),%xmm2
  0xf3,0x0f,0x10,0x5a,0x04,                   //movss         0x4(%rdx),%xmm3
  0x0f,0xc6,0xdb,0x00,                        //shufps        $0x0,%xmm3,%xmm3
  0x0f,0x58,0xcb,                             //addps         %xmm3,%xmm1
  0x0f,0x10,0x42,0x14,                        //movups        0x14(%rdx),%xmm0
  0x0f,0x58,0xc1,                             //addps         %xmm1,%xmm0
  0x66,0x0f,0x6e,0x08,                        //movd          (%rax),%xmm1
  0x66,0x0f,0x70,0xc9,0x00,                   //pshufd        $0x0,%xmm1,%xmm1
  0x0f,0x5b,0xc9,                             //cvtdq2ps      %xmm1,%xmm1
  0x0f,0x58,0xcb,                             //addps         %xmm3,%xmm1
  0x0f,0xc6,0xd2,0x00,                        //shufps        $0x0,%xmm2,%xmm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x57,0xdb,                             //xorps         %xmm3,%xmm3
  0x0f,0x57,0xe4,                             //xorps         %xmm4,%xmm4
  0x0f,0x57,0xed,                             //xorps         %xmm5,%xmm5
  0x0f,0x57,0xf6,                             //xorps         %xmm6,%xmm6
  0x0f,0x57,0xff,                             //xorps         %xmm7,%xmm7
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_constant_color_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x10,0x18,                             //movups        (%rax),%xmm3
  0x0f,0x28,0xc3,                             //movaps        %xmm3,%xmm0
  0x0f,0xc6,0xc0,0x00,                        //shufps        $0x0,%xmm0,%xmm0
  0x0f,0x28,0xcb,                             //movaps        %xmm3,%xmm1
  0x0f,0xc6,0xc9,0x55,                        //shufps        $0x55,%xmm1,%xmm1
  0x0f,0x28,0xd3,                             //movaps        %xmm3,%xmm2
  0x0f,0xc6,0xd2,0xaa,                        //shufps        $0xaa,%xmm2,%xmm2
  0x0f,0xc6,0xdb,0xff,                        //shufps        $0xff,%xmm3,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clear_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x57,0xc0,                             //xorps         %xmm0,%xmm0
  0x0f,0x57,0xc9,                             //xorps         %xmm1,%xmm1
  0x0f,0x57,0xd2,                             //xorps         %xmm2,%xmm2
  0x0f,0x57,0xdb,                             //xorps         %xmm3,%xmm3
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_plus__sse2[] = {
  0x0f,0x58,0xc4,                             //addps         %xmm4,%xmm0
  0x0f,0x58,0xcd,                             //addps         %xmm5,%xmm1
  0x0f,0x58,0xd6,                             //addps         %xmm6,%xmm2
  0x0f,0x58,0xdf,                             //addps         %xmm7,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_srcover_sse2[] = {
  0xf3,0x44,0x0f,0x10,0x02,                   //movss         (%rdx),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x44,0x0f,0x5c,0xc3,                        //subps         %xmm3,%xmm8
  0x45,0x0f,0x28,0xc8,                        //movaps        %xmm8,%xmm9
  0x44,0x0f,0x59,0xcc,                        //mulps         %xmm4,%xmm9
  0x41,0x0f,0x58,0xc1,                        //addps         %xmm9,%xmm0
  0x45,0x0f,0x28,0xc8,                        //movaps        %xmm8,%xmm9
  0x44,0x0f,0x59,0xcd,                        //mulps         %xmm5,%xmm9
  0x41,0x0f,0x58,0xc9,                        //addps         %xmm9,%xmm1
  0x45,0x0f,0x28,0xc8,                        //movaps        %xmm8,%xmm9
  0x44,0x0f,0x59,0xce,                        //mulps         %xmm6,%xmm9
  0x41,0x0f,0x58,0xd1,                        //addps         %xmm9,%xmm2
  0x44,0x0f,0x59,0xc7,                        //mulps         %xmm7,%xmm8
  0x41,0x0f,0x58,0xd8,                        //addps         %xmm8,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_dstover_sse2[] = {
  0xf3,0x44,0x0f,0x10,0x02,                   //movss         (%rdx),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x44,0x0f,0x5c,0xc7,                        //subps         %xmm7,%xmm8
  0x41,0x0f,0x59,0xc0,                        //mulps         %xmm8,%xmm0
  0x0f,0x58,0xc4,                             //addps         %xmm4,%xmm0
  0x41,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm1
  0x0f,0x58,0xcd,                             //addps         %xmm5,%xmm1
  0x41,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm2
  0x0f,0x58,0xd6,                             //addps         %xmm6,%xmm2
  0x41,0x0f,0x59,0xd8,                        //mulps         %xmm8,%xmm3
  0x0f,0x58,0xdf,                             //addps         %xmm7,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_0_sse2[] = {
  0x45,0x0f,0x57,0xc0,                        //xorps         %xmm8,%xmm8
  0x41,0x0f,0x5f,0xc0,                        //maxps         %xmm8,%xmm0
  0x41,0x0f,0x5f,0xc8,                        //maxps         %xmm8,%xmm1
  0x41,0x0f,0x5f,0xd0,                        //maxps         %xmm8,%xmm2
  0x41,0x0f,0x5f,0xd8,                        //maxps         %xmm8,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_1_sse2[] = {
  0xf3,0x44,0x0f,0x10,0x02,                   //movss         (%rdx),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x41,0x0f,0x5d,0xc0,                        //minps         %xmm8,%xmm0
  0x41,0x0f,0x5d,0xc8,                        //minps         %xmm8,%xmm1
  0x41,0x0f,0x5d,0xd0,                        //minps         %xmm8,%xmm2
  0x41,0x0f,0x5d,0xd8,                        //minps         %xmm8,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_a_sse2[] = {
  0xf3,0x44,0x0f,0x10,0x02,                   //movss         (%rdx),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x41,0x0f,0x5d,0xd8,                        //minps         %xmm8,%xmm3
  0x0f,0x5d,0xc3,                             //minps         %xmm3,%xmm0
  0x0f,0x5d,0xcb,                             //minps         %xmm3,%xmm1
  0x0f,0x5d,0xd3,                             //minps         %xmm3,%xmm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_set_rgb_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x0f,0x10,0x00,                        //movss         (%rax),%xmm0
  0xf3,0x0f,0x10,0x48,0x04,                   //movss         0x4(%rax),%xmm1
  0x0f,0xc6,0xc0,0x00,                        //shufps        $0x0,%xmm0,%xmm0
  0x0f,0xc6,0xc9,0x00,                        //shufps        $0x0,%xmm1,%xmm1
  0xf3,0x0f,0x10,0x50,0x08,                   //movss         0x8(%rax),%xmm2
  0x0f,0xc6,0xd2,0x00,                        //shufps        $0x0,%xmm2,%xmm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_swap_rb_sse2[] = {
  0x44,0x0f,0x28,0xc0,                        //movaps        %xmm0,%xmm8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x28,0xc2,                             //movaps        %xmm2,%xmm0
  0x41,0x0f,0x28,0xd0,                        //movaps        %xmm8,%xmm2
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_swap_sse2[] = {
  0x44,0x0f,0x28,0xc3,                        //movaps        %xmm3,%xmm8
  0x44,0x0f,0x28,0xca,                        //movaps        %xmm2,%xmm9
  0x44,0x0f,0x28,0xd1,                        //movaps        %xmm1,%xmm10
  0x44,0x0f,0x28,0xd8,                        //movaps        %xmm0,%xmm11
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x28,0xc4,                             //movaps        %xmm4,%xmm0
  0x0f,0x28,0xcd,                             //movaps        %xmm5,%xmm1
  0x0f,0x28,0xd6,                             //movaps        %xmm6,%xmm2
  0x0f,0x28,0xdf,                             //movaps        %xmm7,%xmm3
  0x41,0x0f,0x28,0xe3,                        //movaps        %xmm11,%xmm4
  0x41,0x0f,0x28,0xea,                        //movaps        %xmm10,%xmm5
  0x41,0x0f,0x28,0xf1,                        //movaps        %xmm9,%xmm6
  0x41,0x0f,0x28,0xf8,                        //movaps        %xmm8,%xmm7
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_move_src_dst_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x28,0xe0,                             //movaps        %xmm0,%xmm4
  0x0f,0x28,0xe9,                             //movaps        %xmm1,%xmm5
  0x0f,0x28,0xf2,                             //movaps        %xmm2,%xmm6
  0x0f,0x28,0xfb,                             //movaps        %xmm3,%xmm7
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_move_dst_src_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x28,0xc4,                             //movaps        %xmm4,%xmm0
  0x0f,0x28,0xcd,                             //movaps        %xmm5,%xmm1
  0x0f,0x28,0xd6,                             //movaps        %xmm6,%xmm2
  0x0f,0x28,0xdf,                             //movaps        %xmm7,%xmm3
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_premul_sse2[] = {
  0x0f,0x59,0xc3,                             //mulps         %xmm3,%xmm0
  0x0f,0x59,0xcb,                             //mulps         %xmm3,%xmm1
  0x0f,0x59,0xd3,                             //mulps         %xmm3,%xmm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_unpremul_sse2[] = {
  0x45,0x0f,0x57,0xc0,                        //xorps         %xmm8,%xmm8
  0x44,0x0f,0xc2,0xc3,0x00,                   //cmpeqps       %xmm3,%xmm8
  0xf3,0x44,0x0f,0x10,0x0a,                   //movss         (%rdx),%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x44,0x0f,0x5e,0xcb,                        //divps         %xmm3,%xmm9
  0x45,0x0f,0x55,0xc1,                        //andnps        %xmm9,%xmm8
  0x41,0x0f,0x59,0xc0,                        //mulps         %xmm8,%xmm0
  0x41,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm1
  0x41,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_from_srgb_sse2[] = {
  0xf3,0x44,0x0f,0x10,0x42,0x40,              //movss         0x40(%rdx),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x45,0x0f,0x28,0xe8,                        //movaps        %xmm8,%xmm13
  0x44,0x0f,0x59,0xe8,                        //mulps         %xmm0,%xmm13
  0x44,0x0f,0x28,0xe0,                        //movaps        %xmm0,%xmm12
  0x45,0x0f,0x59,0xe4,                        //mulps         %xmm12,%xmm12
  0xf3,0x44,0x0f,0x10,0x4a,0x3c,              //movss         0x3c(%rdx),%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0xf3,0x44,0x0f,0x10,0x52,0x34,              //movss         0x34(%rdx),%xmm10
  0xf3,0x44,0x0f,0x10,0x5a,0x38,              //movss         0x38(%rdx),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0x45,0x0f,0x28,0xf1,                        //movaps        %xmm9,%xmm14
  0x44,0x0f,0x59,0xf0,                        //mulps         %xmm0,%xmm14
  0x45,0x0f,0x58,0xf3,                        //addps         %xmm11,%xmm14
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0x45,0x0f,0x59,0xf4,                        //mulps         %xmm12,%xmm14
  0x45,0x0f,0x58,0xf2,                        //addps         %xmm10,%xmm14
  0xf3,0x44,0x0f,0x10,0x62,0x44,              //movss         0x44(%rdx),%xmm12
  0x45,0x0f,0xc6,0xe4,0x00,                   //shufps        $0x0,%xmm12,%xmm12
  0x41,0x0f,0xc2,0xc4,0x01,                   //cmpltps       %xmm12,%xmm0
  0x44,0x0f,0x54,0xe8,                        //andps         %xmm0,%xmm13
  0x41,0x0f,0x55,0xc6,                        //andnps        %xmm14,%xmm0
  0x41,0x0f,0x56,0xc5,                        //orps          %xmm13,%xmm0
  0x45,0x0f,0x28,0xe8,                        //movaps        %xmm8,%xmm13
  0x44,0x0f,0x59,0xe9,                        //mulps         %xmm1,%xmm13
  0x44,0x0f,0x28,0xf1,                        //movaps        %xmm1,%xmm14
  0x45,0x0f,0x59,0xf6,                        //mulps         %xmm14,%xmm14
  0x45,0x0f,0x28,0xf9,                        //movaps        %xmm9,%xmm15
  0x44,0x0f,0x59,0xf9,                        //mulps         %xmm1,%xmm15
  0x45,0x0f,0x58,0xfb,                        //addps         %xmm11,%xmm15
  0x45,0x0f,0x59,0xfe,                        //mulps         %xmm14,%xmm15
  0x45,0x0f,0x58,0xfa,                        //addps         %xmm10,%xmm15
  0x41,0x0f,0xc2,0xcc,0x01,                   //cmpltps       %xmm12,%xmm1
  0x44,0x0f,0x54,0xe9,                        //andps         %xmm1,%xmm13
  0x41,0x0f,0x55,0xcf,                        //andnps        %xmm15,%xmm1
  0x41,0x0f,0x56,0xcd,                        //orps          %xmm13,%xmm1
  0x44,0x0f,0x59,0xc2,                        //mulps         %xmm2,%xmm8
  0x44,0x0f,0x28,0xea,                        //movaps        %xmm2,%xmm13
  0x45,0x0f,0x59,0xed,                        //mulps         %xmm13,%xmm13
  0x44,0x0f,0x59,0xca,                        //mulps         %xmm2,%xmm9
  0x45,0x0f,0x58,0xcb,                        //addps         %xmm11,%xmm9
  0x45,0x0f,0x59,0xcd,                        //mulps         %xmm13,%xmm9
  0x45,0x0f,0x58,0xca,                        //addps         %xmm10,%xmm9
  0x41,0x0f,0xc2,0xd4,0x01,                   //cmpltps       %xmm12,%xmm2
  0x44,0x0f,0x54,0xc2,                        //andps         %xmm2,%xmm8
  0x41,0x0f,0x55,0xd1,                        //andnps        %xmm9,%xmm2
  0x41,0x0f,0x56,0xd0,                        //orps          %xmm8,%xmm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_to_srgb_sse2[] = {
  0x48,0x83,0xec,0x28,                        //sub           $0x28,%rsp
  0x0f,0x29,0x7c,0x24,0x10,                   //movaps        %xmm7,0x10(%rsp)
  0x0f,0x29,0x34,0x24,                        //movaps        %xmm6,(%rsp)
  0x0f,0x28,0xf5,                             //movaps        %xmm5,%xmm6
  0x0f,0x28,0xec,                             //movaps        %xmm4,%xmm5
  0x0f,0x28,0xe3,                             //movaps        %xmm3,%xmm4
  0x44,0x0f,0x52,0xc0,                        //rsqrtps       %xmm0,%xmm8
  0x45,0x0f,0x53,0xe8,                        //rcpps         %xmm8,%xmm13
  0x45,0x0f,0x52,0xf8,                        //rsqrtps       %xmm8,%xmm15
  0xf3,0x0f,0x10,0x1a,                        //movss         (%rdx),%xmm3
  0xf3,0x44,0x0f,0x10,0x42,0x48,              //movss         0x48(%rdx),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x45,0x0f,0x28,0xf0,                        //movaps        %xmm8,%xmm14
  0x44,0x0f,0x59,0xf0,                        //mulps         %xmm0,%xmm14
  0x0f,0xc6,0xdb,0x00,                        //shufps        $0x0,%xmm3,%xmm3
  0xf3,0x44,0x0f,0x10,0x52,0x4c,              //movss         0x4c(%rdx),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0xf3,0x44,0x0f,0x10,0x5a,0x50,              //movss         0x50(%rdx),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0xf3,0x44,0x0f,0x10,0x62,0x54,              //movss         0x54(%rdx),%xmm12
  0x45,0x0f,0xc6,0xe4,0x00,                   //shufps        $0x0,%xmm12,%xmm12
  0x45,0x0f,0x59,0xeb,                        //mulps         %xmm11,%xmm13
  0x45,0x0f,0x58,0xec,                        //addps         %xmm12,%xmm13
  0x45,0x0f,0x59,0xfa,                        //mulps         %xmm10,%xmm15
  0x45,0x0f,0x58,0xfd,                        //addps         %xmm13,%xmm15
  0x44,0x0f,0x28,0xcb,                        //movaps        %xmm3,%xmm9
  0x45,0x0f,0x5d,0xcf,                        //minps         %xmm15,%xmm9
  0xf3,0x44,0x0f,0x10,0x6a,0x58,              //movss         0x58(%rdx),%xmm13
  0x45,0x0f,0xc6,0xed,0x00,                   //shufps        $0x0,%xmm13,%xmm13
  0x41,0x0f,0xc2,0xc5,0x01,                   //cmpltps       %xmm13,%xmm0
  0x44,0x0f,0x54,0xf0,                        //andps         %xmm0,%xmm14
  0x41,0x0f,0x55,0xc1,                        //andnps        %xmm9,%xmm0
  0x41,0x0f,0x56,0xc6,                        //orps          %xmm14,%xmm0
  0x44,0x0f,0x52,0xc9,                        //rsqrtps       %xmm1,%xmm9
  0x45,0x0f,0x53,0xf1,                        //rcpps         %xmm9,%xmm14
  0x45,0x0f,0x52,0xc9,                        //rsqrtps       %xmm9,%xmm9
  0x45,0x0f,0x59,0xf3,                        //mulps         %xmm11,%xmm14
  0x45,0x0f,0x58,0xf4,                        //addps         %xmm12,%xmm14
  0x45,0x0f,0x59,0xca,                        //mulps         %xmm10,%xmm9
  0x45,0x0f,0x58,0xce,                        //addps         %xmm14,%xmm9
  0x44,0x0f,0x28,0xf3,                        //movaps        %xmm3,%xmm14
  0x45,0x0f,0x5d,0xf1,                        //minps         %xmm9,%xmm14
  0x45,0x0f,0x28,0xc8,                        //movaps        %xmm8,%xmm9
  0x44,0x0f,0x59,0xc9,                        //mulps         %xmm1,%xmm9
  0x41,0x0f,0xc2,0xcd,0x01,                   //cmpltps       %xmm13,%xmm1
  0x44,0x0f,0x54,0xc9,                        //andps         %xmm1,%xmm9
  0x41,0x0f,0x55,0xce,                        //andnps        %xmm14,%xmm1
  0x41,0x0f,0x56,0xc9,                        //orps          %xmm9,%xmm1
  0x44,0x0f,0x52,0xca,                        //rsqrtps       %xmm2,%xmm9
  0x45,0x0f,0x53,0xf1,                        //rcpps         %xmm9,%xmm14
  0x45,0x0f,0x59,0xf3,                        //mulps         %xmm11,%xmm14
  0x45,0x0f,0x58,0xf4,                        //addps         %xmm12,%xmm14
  0x41,0x0f,0x52,0xf9,                        //rsqrtps       %xmm9,%xmm7
  0x41,0x0f,0x59,0xfa,                        //mulps         %xmm10,%xmm7
  0x41,0x0f,0x58,0xfe,                        //addps         %xmm14,%xmm7
  0x0f,0x5d,0xdf,                             //minps         %xmm7,%xmm3
  0x44,0x0f,0x59,0xc2,                        //mulps         %xmm2,%xmm8
  0x41,0x0f,0xc2,0xd5,0x01,                   //cmpltps       %xmm13,%xmm2
  0x44,0x0f,0x54,0xc2,                        //andps         %xmm2,%xmm8
  0x0f,0x55,0xd3,                             //andnps        %xmm3,%xmm2
  0x41,0x0f,0x56,0xd0,                        //orps          %xmm8,%xmm2
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x28,0xdc,                             //movaps        %xmm4,%xmm3
  0x0f,0x28,0xe5,                             //movaps        %xmm5,%xmm4
  0x0f,0x28,0xee,                             //movaps        %xmm6,%xmm5
  0x0f,0x28,0x34,0x24,                        //movaps        (%rsp),%xmm6
  0x0f,0x28,0x7c,0x24,0x10,                   //movaps        0x10(%rsp),%xmm7
  0x48,0x83,0xc4,0x28,                        //add           $0x28,%rsp
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_scale_1_float_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x44,0x0f,0x10,0x00,                   //movss         (%rax),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x41,0x0f,0x59,0xc0,                        //mulps         %xmm8,%xmm0
  0x41,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm1
  0x41,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm2
  0x41,0x0f,0x59,0xd8,                        //mulps         %xmm8,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_scale_u8_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x66,0x44,0x0f,0x6e,0x04,0x38,              //movd          (%rax,%rdi,1),%xmm8
  0x66,0x45,0x0f,0xef,0xc9,                   //pxor          %xmm9,%xmm9
  0x66,0x45,0x0f,0x60,0xc1,                   //punpcklbw     %xmm9,%xmm8
  0x66,0x45,0x0f,0x61,0xc1,                   //punpcklwd     %xmm9,%xmm8
  0x45,0x0f,0x5b,0xc0,                        //cvtdq2ps      %xmm8,%xmm8
  0xf3,0x44,0x0f,0x10,0x4a,0x0c,              //movss         0xc(%rdx),%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x45,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm9
  0x41,0x0f,0x59,0xc1,                        //mulps         %xmm9,%xmm0
  0x41,0x0f,0x59,0xc9,                        //mulps         %xmm9,%xmm1
  0x41,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm2
  0x41,0x0f,0x59,0xd9,                        //mulps         %xmm9,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_lerp_1_float_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x44,0x0f,0x10,0x00,                   //movss         (%rax),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x0f,0x5c,0xc4,                             //subps         %xmm4,%xmm0
  0x41,0x0f,0x59,0xc0,                        //mulps         %xmm8,%xmm0
  0x0f,0x58,0xc4,                             //addps         %xmm4,%xmm0
  0x0f,0x5c,0xcd,                             //subps         %xmm5,%xmm1
  0x41,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm1
  0x0f,0x58,0xcd,                             //addps         %xmm5,%xmm1
  0x0f,0x5c,0xd6,                             //subps         %xmm6,%xmm2
  0x41,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm2
  0x0f,0x58,0xd6,                             //addps         %xmm6,%xmm2
  0x0f,0x5c,0xdf,                             //subps         %xmm7,%xmm3
  0x41,0x0f,0x59,0xd8,                        //mulps         %xmm8,%xmm3
  0x0f,0x58,0xdf,                             //addps         %xmm7,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_lerp_u8_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x66,0x44,0x0f,0x6e,0x04,0x38,              //movd          (%rax,%rdi,1),%xmm8
  0x66,0x45,0x0f,0xef,0xc9,                   //pxor          %xmm9,%xmm9
  0x66,0x45,0x0f,0x60,0xc1,                   //punpcklbw     %xmm9,%xmm8
  0x66,0x45,0x0f,0x61,0xc1,                   //punpcklwd     %xmm9,%xmm8
  0x45,0x0f,0x5b,0xc0,                        //cvtdq2ps      %xmm8,%xmm8
  0xf3,0x44,0x0f,0x10,0x4a,0x0c,              //movss         0xc(%rdx),%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x45,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm9
  0x0f,0x5c,0xc4,                             //subps         %xmm4,%xmm0
  0x41,0x0f,0x59,0xc1,                        //mulps         %xmm9,%xmm0
  0x0f,0x58,0xc4,                             //addps         %xmm4,%xmm0
  0x0f,0x5c,0xcd,                             //subps         %xmm5,%xmm1
  0x41,0x0f,0x59,0xc9,                        //mulps         %xmm9,%xmm1
  0x0f,0x58,0xcd,                             //addps         %xmm5,%xmm1
  0x0f,0x5c,0xd6,                             //subps         %xmm6,%xmm2
  0x41,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm2
  0x0f,0x58,0xd6,                             //addps         %xmm6,%xmm2
  0x0f,0x5c,0xdf,                             //subps         %xmm7,%xmm3
  0x41,0x0f,0x59,0xd9,                        //mulps         %xmm9,%xmm3
  0x0f,0x58,0xdf,                             //addps         %xmm7,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_lerp_565_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0xf3,0x44,0x0f,0x7e,0x04,0x78,              //movq          (%rax,%rdi,2),%xmm8
  0x66,0x0f,0xef,0xdb,                        //pxor          %xmm3,%xmm3
  0x66,0x44,0x0f,0x61,0xc3,                   //punpcklwd     %xmm3,%xmm8
  0x66,0x0f,0x6e,0x5a,0x68,                   //movd          0x68(%rdx),%xmm3
  0x66,0x0f,0x70,0xdb,0x00,                   //pshufd        $0x0,%xmm3,%xmm3
  0x66,0x41,0x0f,0xdb,0xd8,                   //pand          %xmm8,%xmm3
  0x44,0x0f,0x5b,0xcb,                        //cvtdq2ps      %xmm3,%xmm9
  0xf3,0x0f,0x10,0x1a,                        //movss         (%rdx),%xmm3
  0xf3,0x44,0x0f,0x10,0x52,0x74,              //movss         0x74(%rdx),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0x45,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm10
  0x66,0x44,0x0f,0x6e,0x4a,0x6c,              //movd          0x6c(%rdx),%xmm9
  0x66,0x45,0x0f,0x70,0xc9,0x00,              //pshufd        $0x0,%xmm9,%xmm9
  0x66,0x45,0x0f,0xdb,0xc8,                   //pand          %xmm8,%xmm9
  0x45,0x0f,0x5b,0xc9,                        //cvtdq2ps      %xmm9,%xmm9
  0xf3,0x44,0x0f,0x10,0x5a,0x78,              //movss         0x78(%rdx),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0x45,0x0f,0x59,0xd9,                        //mulps         %xmm9,%xmm11
  0x66,0x44,0x0f,0x6e,0x4a,0x70,              //movd          0x70(%rdx),%xmm9
  0x66,0x45,0x0f,0x70,0xc9,0x00,              //pshufd        $0x0,%xmm9,%xmm9
  0x66,0x45,0x0f,0xdb,0xc8,                   //pand          %xmm8,%xmm9
  0x45,0x0f,0x5b,0xc1,                        //cvtdq2ps      %xmm9,%xmm8
  0xf3,0x44,0x0f,0x10,0x4a,0x7c,              //movss         0x7c(%rdx),%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x45,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm9
  0x0f,0x5c,0xc4,                             //subps         %xmm4,%xmm0
  0x41,0x0f,0x59,0xc2,                        //mulps         %xmm10,%xmm0
  0x0f,0x58,0xc4,                             //addps         %xmm4,%xmm0
  0x0f,0x5c,0xcd,                             //subps         %xmm5,%xmm1
  0x41,0x0f,0x59,0xcb,                        //mulps         %xmm11,%xmm1
  0x0f,0x58,0xcd,                             //addps         %xmm5,%xmm1
  0x0f,0x5c,0xd6,                             //subps         %xmm6,%xmm2
  0x41,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm2
  0x0f,0x58,0xd6,                             //addps         %xmm6,%xmm2
  0x0f,0xc6,0xdb,0x00,                        //shufps        $0x0,%xmm3,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_load_tables_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x08,                             //mov           (%rax),%rcx
  0x4c,0x8b,0x40,0x08,                        //mov           0x8(%rax),%r8
  0xf3,0x44,0x0f,0x6f,0x04,0xb9,              //movdqu        (%rcx,%rdi,4),%xmm8
  0x66,0x0f,0x6e,0x42,0x10,                   //movd          0x10(%rdx),%xmm0
  0x66,0x0f,0x70,0xc0,0x00,                   //pshufd        $0x0,%xmm0,%xmm0
  0x66,0x45,0x0f,0x6f,0xc8,                   //movdqa        %xmm8,%xmm9
  0x66,0x41,0x0f,0x72,0xd1,0x08,              //psrld         $0x8,%xmm9
  0x66,0x44,0x0f,0xdb,0xc8,                   //pand          %xmm0,%xmm9
  0x66,0x45,0x0f,0x6f,0xd0,                   //movdqa        %xmm8,%xmm10
  0x66,0x41,0x0f,0x72,0xd2,0x10,              //psrld         $0x10,%xmm10
  0x66,0x44,0x0f,0xdb,0xd0,                   //pand          %xmm0,%xmm10
  0x66,0x41,0x0f,0xdb,0xc0,                   //pand          %xmm8,%xmm0
  0x66,0x0f,0x70,0xd8,0x4e,                   //pshufd        $0x4e,%xmm0,%xmm3
  0x66,0x48,0x0f,0x7e,0xd9,                   //movq          %xmm3,%rcx
  0x41,0x89,0xc9,                             //mov           %ecx,%r9d
  0x48,0xc1,0xe9,0x20,                        //shr           $0x20,%rcx
  0x66,0x49,0x0f,0x7e,0xc2,                   //movq          %xmm0,%r10
  0x45,0x89,0xd3,                             //mov           %r10d,%r11d
  0x49,0xc1,0xea,0x20,                        //shr           $0x20,%r10
  0xf3,0x43,0x0f,0x10,0x1c,0x90,              //movss         (%r8,%r10,4),%xmm3
  0xf3,0x41,0x0f,0x10,0x04,0x88,              //movss         (%r8,%rcx,4),%xmm0
  0x0f,0x14,0xd8,                             //unpcklps      %xmm0,%xmm3
  0xf3,0x43,0x0f,0x10,0x04,0x98,              //movss         (%r8,%r11,4),%xmm0
  0xf3,0x43,0x0f,0x10,0x0c,0x88,              //movss         (%r8,%r9,4),%xmm1
  0x0f,0x14,0xc1,                             //unpcklps      %xmm1,%xmm0
  0x0f,0x14,0xc3,                             //unpcklps      %xmm3,%xmm0
  0x48,0x8b,0x48,0x10,                        //mov           0x10(%rax),%rcx
  0x66,0x41,0x0f,0x70,0xc9,0x4e,              //pshufd        $0x4e,%xmm9,%xmm1
  0x66,0x49,0x0f,0x7e,0xc8,                   //movq          %xmm1,%r8
  0x45,0x89,0xc1,                             //mov           %r8d,%r9d
  0x49,0xc1,0xe8,0x20,                        //shr           $0x20,%r8
  0x66,0x4d,0x0f,0x7e,0xca,                   //movq          %xmm9,%r10
  0x45,0x89,0xd3,                             //mov           %r10d,%r11d
  0x49,0xc1,0xea,0x20,                        //shr           $0x20,%r10
  0xf3,0x42,0x0f,0x10,0x1c,0x91,              //movss         (%rcx,%r10,4),%xmm3
  0xf3,0x42,0x0f,0x10,0x0c,0x81,              //movss         (%rcx,%r8,4),%xmm1
  0x0f,0x14,0xd9,                             //unpcklps      %xmm1,%xmm3
  0xf3,0x42,0x0f,0x10,0x0c,0x99,              //movss         (%rcx,%r11,4),%xmm1
  0xf3,0x42,0x0f,0x10,0x14,0x89,              //movss         (%rcx,%r9,4),%xmm2
  0x0f,0x14,0xca,                             //unpcklps      %xmm2,%xmm1
  0x0f,0x14,0xcb,                             //unpcklps      %xmm3,%xmm1
  0x48,0x8b,0x40,0x18,                        //mov           0x18(%rax),%rax
  0x66,0x41,0x0f,0x70,0xd2,0x4e,              //pshufd        $0x4e,%xmm10,%xmm2
  0x66,0x48,0x0f,0x7e,0xd1,                   //movq          %xmm2,%rcx
  0x41,0x89,0xc8,                             //mov           %ecx,%r8d
  0x48,0xc1,0xe9,0x20,                        //shr           $0x20,%rcx
  0x66,0x4d,0x0f,0x7e,0xd1,                   //movq          %xmm10,%r9
  0x45,0x89,0xca,                             //mov           %r9d,%r10d
  0x49,0xc1,0xe9,0x20,                        //shr           $0x20,%r9
  0xf3,0x46,0x0f,0x10,0x0c,0x88,              //movss         (%rax,%r9,4),%xmm9
  0xf3,0x0f,0x10,0x14,0x88,                   //movss         (%rax,%rcx,4),%xmm2
  0x44,0x0f,0x14,0xca,                        //unpcklps      %xmm2,%xmm9
  0xf3,0x42,0x0f,0x10,0x14,0x90,              //movss         (%rax,%r10,4),%xmm2
  0xf3,0x42,0x0f,0x10,0x1c,0x80,              //movss         (%rax,%r8,4),%xmm3
  0x0f,0x14,0xd3,                             //unpcklps      %xmm3,%xmm2
  0x41,0x0f,0x14,0xd1,                        //unpcklps      %xmm9,%xmm2
  0x66,0x41,0x0f,0x72,0xd0,0x18,              //psrld         $0x18,%xmm8
  0x45,0x0f,0x5b,0xc0,                        //cvtdq2ps      %xmm8,%xmm8
  0xf3,0x0f,0x10,0x5a,0x0c,                   //movss         0xc(%rdx),%xmm3
  0x0f,0xc6,0xdb,0x00,                        //shufps        $0x0,%xmm3,%xmm3
  0x41,0x0f,0x59,0xd8,                        //mulps         %xmm8,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_load_a8_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x66,0x0f,0x6e,0x04,0x38,                   //movd          (%rax,%rdi,1),%xmm0
  0x66,0x0f,0xef,0xc9,                        //pxor          %xmm1,%xmm1
  0x66,0x0f,0x60,0xc1,                        //punpcklbw     %xmm1,%xmm0
  0x66,0x0f,0x61,0xc1,                        //punpcklwd     %xmm1,%xmm0
  0x0f,0x5b,0xc0,                             //cvtdq2ps      %xmm0,%xmm0
  0xf3,0x0f,0x10,0x5a,0x0c,                   //movss         0xc(%rdx),%xmm3
  0x0f,0xc6,0xdb,0x00,                        //shufps        $0x0,%xmm3,%xmm3
  0x0f,0x59,0xd8,                             //mulps         %xmm0,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x0f,0x57,0xc0,                             //xorps         %xmm0,%xmm0
  0x66,0x0f,0xef,0xc9,                        //pxor          %xmm1,%xmm1
  0x0f,0x57,0xd2,                             //xorps         %xmm2,%xmm2
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_store_a8_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0xf3,0x44,0x0f,0x10,0x42,0x08,              //movss         0x8(%rdx),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x44,0x0f,0x59,0xc3,                        //mulps         %xmm3,%xmm8
  0x66,0x45,0x0f,0x5b,0xc0,                   //cvtps2dq      %xmm8,%xmm8
  0x66,0x41,0x0f,0x72,0xf0,0x10,              //pslld         $0x10,%xmm8
  0x66,0x41,0x0f,0x72,0xe0,0x10,              //psrad         $0x10,%xmm8
  0x66,0x45,0x0f,0x6b,0xc0,                   //packssdw      %xmm8,%xmm8
  0x66,0x45,0x0f,0x67,0xc0,                   //packuswb      %xmm8,%xmm8
  0x66,0x44,0x0f,0x7e,0x04,0x38,              //movd          %xmm8,(%rax,%rdi,1)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_load_565_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0xf3,0x44,0x0f,0x7e,0x0c,0x78,              //movq          (%rax,%rdi,2),%xmm9
  0x66,0x0f,0xef,0xc0,                        //pxor          %xmm0,%xmm0
  0x66,0x44,0x0f,0x61,0xc8,                   //punpcklwd     %xmm0,%xmm9
  0x66,0x0f,0x6e,0x42,0x68,                   //movd          0x68(%rdx),%xmm0
  0x66,0x0f,0x70,0xc0,0x00,                   //pshufd        $0x0,%xmm0,%xmm0
  0x66,0x41,0x0f,0xdb,0xc1,                   //pand          %xmm9,%xmm0
  0x0f,0x5b,0xc8,                             //cvtdq2ps      %xmm0,%xmm1
  0xf3,0x0f,0x10,0x1a,                        //movss         (%rdx),%xmm3
  0xf3,0x0f,0x10,0x42,0x74,                   //movss         0x74(%rdx),%xmm0
  0x0f,0xc6,0xc0,0x00,                        //shufps        $0x0,%xmm0,%xmm0
  0x0f,0x59,0xc1,                             //mulps         %xmm1,%xmm0
  0x66,0x0f,0x6e,0x4a,0x6c,                   //movd          0x6c(%rdx),%xmm1
  0x66,0x0f,0x70,0xc9,0x00,                   //pshufd        $0x0,%xmm1,%xmm1
  0x66,0x41,0x0f,0xdb,0xc9,                   //pand          %xmm9,%xmm1
  0x44,0x0f,0x5b,0xc1,                        //cvtdq2ps      %xmm1,%xmm8
  0xf3,0x0f,0x10,0x4a,0x78,                   //movss         0x78(%rdx),%xmm1
  0x0f,0xc6,0xc9,0x00,                        //shufps        $0x0,%xmm1,%xmm1
  0x41,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm1
  0x66,0x0f,0x6e,0x52,0x70,                   //movd          0x70(%rdx),%xmm2
  0x66,0x0f,0x70,0xd2,0x00,                   //pshufd        $0x0,%xmm2,%xmm2
  0x66,0x41,0x0f,0xdb,0xd1,                   //pand          %xmm9,%xmm2
  0x44,0x0f,0x5b,0xc2,                        //cvtdq2ps      %xmm2,%xmm8
  0xf3,0x0f,0x10,0x52,0x7c,                   //movss         0x7c(%rdx),%xmm2
  0x0f,0xc6,0xd2,0x00,                        //shufps        $0x0,%xmm2,%xmm2
  0x41,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm2
  0x0f,0xc6,0xdb,0x00,                        //shufps        $0x0,%xmm3,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_store_565_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0xf3,0x44,0x0f,0x10,0x82,0x80,0x00,0x00,0x00,//movss         0x80(%rdx),%xmm8
  0xf3,0x44,0x0f,0x10,0x8a,0x84,0x00,0x00,0x00,//movss         0x84(%rdx),%xmm9
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x45,0x0f,0x28,0xd0,                        //movaps        %xmm8,%xmm10
  0x44,0x0f,0x59,0xd0,                        //mulps         %xmm0,%xmm10
  0x66,0x45,0x0f,0x5b,0xd2,                   //cvtps2dq      %xmm10,%xmm10
  0x66,0x41,0x0f,0x72,0xf2,0x0b,              //pslld         $0xb,%xmm10
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x44,0x0f,0x59,0xc9,                        //mulps         %xmm1,%xmm9
  0x66,0x45,0x0f,0x5b,0xc9,                   //cvtps2dq      %xmm9,%xmm9
  0x66,0x41,0x0f,0x72,0xf1,0x05,              //pslld         $0x5,%xmm9
  0x66,0x45,0x0f,0xeb,0xca,                   //por           %xmm10,%xmm9
  0x44,0x0f,0x59,0xc2,                        //mulps         %xmm2,%xmm8
  0x66,0x45,0x0f,0x5b,0xc0,                   //cvtps2dq      %xmm8,%xmm8
  0x66,0x45,0x0f,0x56,0xc1,                   //orpd          %xmm9,%xmm8
  0x66,0x41,0x0f,0x72,0xf0,0x10,              //pslld         $0x10,%xmm8
  0x66,0x41,0x0f,0x72,0xe0,0x10,              //psrad         $0x10,%xmm8
  0x66,0x45,0x0f,0x6b,0xc0,                   //packssdw      %xmm8,%xmm8
  0x66,0x44,0x0f,0xd6,0x04,0x78,              //movq          %xmm8,(%rax,%rdi,2)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_load_8888_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0xf3,0x0f,0x6f,0x1c,0xb8,                   //movdqu        (%rax,%rdi,4),%xmm3
  0x66,0x0f,0x6e,0x42,0x10,                   //movd          0x10(%rdx),%xmm0
  0x66,0x0f,0x70,0xc0,0x00,                   //pshufd        $0x0,%xmm0,%xmm0
  0x66,0x0f,0x6f,0xcb,                        //movdqa        %xmm3,%xmm1
  0x66,0x0f,0x72,0xd1,0x08,                   //psrld         $0x8,%xmm1
  0x66,0x0f,0xdb,0xc8,                        //pand          %xmm0,%xmm1
  0x66,0x0f,0x6f,0xd3,                        //movdqa        %xmm3,%xmm2
  0x66,0x0f,0x72,0xd2,0x10,                   //psrld         $0x10,%xmm2
  0x66,0x0f,0xdb,0xd0,                        //pand          %xmm0,%xmm2
  0x66,0x0f,0xdb,0xc3,                        //pand          %xmm3,%xmm0
  0x0f,0x5b,0xc0,                             //cvtdq2ps      %xmm0,%xmm0
  0xf3,0x44,0x0f,0x10,0x42,0x0c,              //movss         0xc(%rdx),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x41,0x0f,0x59,0xc0,                        //mulps         %xmm8,%xmm0
  0x0f,0x5b,0xc9,                             //cvtdq2ps      %xmm1,%xmm1
  0x41,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm1
  0x0f,0x5b,0xd2,                             //cvtdq2ps      %xmm2,%xmm2
  0x41,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm2
  0x66,0x0f,0x72,0xd3,0x18,                   //psrld         $0x18,%xmm3
  0x0f,0x5b,0xdb,                             //cvtdq2ps      %xmm3,%xmm3
  0x41,0x0f,0x59,0xd8,                        //mulps         %xmm8,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_store_8888_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0xf3,0x44,0x0f,0x10,0x42,0x08,              //movss         0x8(%rdx),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x45,0x0f,0x28,0xc8,                        //movaps        %xmm8,%xmm9
  0x44,0x0f,0x59,0xc8,                        //mulps         %xmm0,%xmm9
  0x66,0x45,0x0f,0x5b,0xc9,                   //cvtps2dq      %xmm9,%xmm9
  0x45,0x0f,0x28,0xd0,                        //movaps        %xmm8,%xmm10
  0x44,0x0f,0x59,0xd1,                        //mulps         %xmm1,%xmm10
  0x66,0x45,0x0f,0x5b,0xd2,                   //cvtps2dq      %xmm10,%xmm10
  0x66,0x41,0x0f,0x72,0xf2,0x08,              //pslld         $0x8,%xmm10
  0x66,0x45,0x0f,0xeb,0xd1,                   //por           %xmm9,%xmm10
  0x45,0x0f,0x28,0xc8,                        //movaps        %xmm8,%xmm9
  0x44,0x0f,0x59,0xca,                        //mulps         %xmm2,%xmm9
  0x66,0x45,0x0f,0x5b,0xc9,                   //cvtps2dq      %xmm9,%xmm9
  0x66,0x41,0x0f,0x72,0xf1,0x10,              //pslld         $0x10,%xmm9
  0x44,0x0f,0x59,0xc3,                        //mulps         %xmm3,%xmm8
  0x66,0x45,0x0f,0x5b,0xc0,                   //cvtps2dq      %xmm8,%xmm8
  0x66,0x41,0x0f,0x72,0xf0,0x18,              //pslld         $0x18,%xmm8
  0x66,0x45,0x0f,0xeb,0xc1,                   //por           %xmm9,%xmm8
  0x66,0x45,0x0f,0xeb,0xc2,                   //por           %xmm10,%xmm8
  0xf3,0x44,0x0f,0x7f,0x04,0xb8,              //movdqu        %xmm8,(%rax,%rdi,4)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_load_f16_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0xf3,0x0f,0x6f,0x04,0xf8,                   //movdqu        (%rax,%rdi,8),%xmm0
  0xf3,0x0f,0x6f,0x4c,0xf8,0x10,              //movdqu        0x10(%rax,%rdi,8),%xmm1
  0x66,0x0f,0x6f,0xd0,                        //movdqa        %xmm0,%xmm2
  0x66,0x0f,0x61,0xd1,                        //punpcklwd     %xmm1,%xmm2
  0x66,0x0f,0x69,0xc1,                        //punpckhwd     %xmm1,%xmm0
  0x66,0x44,0x0f,0x6f,0xc2,                   //movdqa        %xmm2,%xmm8
  0x66,0x44,0x0f,0x61,0xc0,                   //punpcklwd     %xmm0,%xmm8
  0x66,0x0f,0x69,0xd0,                        //punpckhwd     %xmm0,%xmm2
  0x66,0x0f,0x6e,0x42,0x64,                   //movd          0x64(%rdx),%xmm0
  0x66,0x0f,0x70,0xd8,0x00,                   //pshufd        $0x0,%xmm0,%xmm3
  0x66,0x0f,0x6f,0xcb,                        //movdqa        %xmm3,%xmm1
  0x66,0x41,0x0f,0x65,0xc8,                   //pcmpgtw       %xmm8,%xmm1
  0x66,0x41,0x0f,0xdf,0xc8,                   //pandn         %xmm8,%xmm1
  0x66,0x0f,0x65,0xda,                        //pcmpgtw       %xmm2,%xmm3
  0x66,0x0f,0xdf,0xda,                        //pandn         %xmm2,%xmm3
  0x66,0x45,0x0f,0xef,0xc0,                   //pxor          %xmm8,%xmm8
  0x66,0x0f,0x6f,0xc1,                        //movdqa        %xmm1,%xmm0
  0x66,0x41,0x0f,0x61,0xc0,                   //punpcklwd     %xmm8,%xmm0
  0x66,0x0f,0x72,0xf0,0x0d,                   //pslld         $0xd,%xmm0
  0x66,0x0f,0x6e,0x52,0x5c,                   //movd          0x5c(%rdx),%xmm2
  0x66,0x44,0x0f,0x70,0xca,0x00,              //pshufd        $0x0,%xmm2,%xmm9
  0x41,0x0f,0x59,0xc1,                        //mulps         %xmm9,%xmm0
  0x66,0x41,0x0f,0x69,0xc8,                   //punpckhwd     %xmm8,%xmm1
  0x66,0x0f,0x72,0xf1,0x0d,                   //pslld         $0xd,%xmm1
  0x41,0x0f,0x59,0xc9,                        //mulps         %xmm9,%xmm1
  0x66,0x0f,0x6f,0xd3,                        //movdqa        %xmm3,%xmm2
  0x66,0x41,0x0f,0x61,0xd0,                   //punpcklwd     %xmm8,%xmm2
  0x66,0x0f,0x72,0xf2,0x0d,                   //pslld         $0xd,%xmm2
  0x41,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm2
  0x66,0x41,0x0f,0x69,0xd8,                   //punpckhwd     %xmm8,%xmm3
  0x66,0x0f,0x72,0xf3,0x0d,                   //pslld         $0xd,%xmm3
  0x41,0x0f,0x59,0xd9,                        //mulps         %xmm9,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_store_f16_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x66,0x44,0x0f,0x6e,0x42,0x60,              //movd          0x60(%rdx),%xmm8
  0x66,0x45,0x0f,0x70,0xc0,0x00,              //pshufd        $0x0,%xmm8,%xmm8
  0x66,0x45,0x0f,0x6f,0xc8,                   //movdqa        %xmm8,%xmm9
  0x44,0x0f,0x59,0xc8,                        //mulps         %xmm0,%xmm9
  0x66,0x41,0x0f,0x72,0xd1,0x0d,              //psrld         $0xd,%xmm9
  0x66,0x45,0x0f,0x6f,0xd0,                   //movdqa        %xmm8,%xmm10
  0x44,0x0f,0x59,0xd1,                        //mulps         %xmm1,%xmm10
  0x66,0x41,0x0f,0x72,0xd2,0x0d,              //psrld         $0xd,%xmm10
  0x66,0x45,0x0f,0x6f,0xd8,                   //movdqa        %xmm8,%xmm11
  0x44,0x0f,0x59,0xda,                        //mulps         %xmm2,%xmm11
  0x66,0x41,0x0f,0x72,0xd3,0x0d,              //psrld         $0xd,%xmm11
  0x44,0x0f,0x59,0xc3,                        //mulps         %xmm3,%xmm8
  0x66,0x41,0x0f,0x72,0xd0,0x0d,              //psrld         $0xd,%xmm8
  0x66,0x41,0x0f,0x73,0xfa,0x02,              //pslldq        $0x2,%xmm10
  0x66,0x45,0x0f,0xeb,0xd1,                   //por           %xmm9,%xmm10
  0x66,0x41,0x0f,0x73,0xf8,0x02,              //pslldq        $0x2,%xmm8
  0x66,0x45,0x0f,0xeb,0xc3,                   //por           %xmm11,%xmm8
  0x66,0x45,0x0f,0x6f,0xca,                   //movdqa        %xmm10,%xmm9
  0x66,0x45,0x0f,0x62,0xc8,                   //punpckldq     %xmm8,%xmm9
  0xf3,0x44,0x0f,0x7f,0x0c,0xf8,              //movdqu        %xmm9,(%rax,%rdi,8)
  0x66,0x45,0x0f,0x6a,0xd0,                   //punpckhdq     %xmm8,%xmm10
  0xf3,0x44,0x0f,0x7f,0x54,0xf8,0x10,         //movdqu        %xmm10,0x10(%rax,%rdi,8)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_store_f32_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x48,0x8b,0x00,                             //mov           (%rax),%rax
  0x48,0x89,0xf9,                             //mov           %rdi,%rcx
  0x48,0xc1,0xe1,0x04,                        //shl           $0x4,%rcx
  0x44,0x0f,0x28,0xc0,                        //movaps        %xmm0,%xmm8
  0x44,0x0f,0x28,0xc8,                        //movaps        %xmm0,%xmm9
  0x44,0x0f,0x14,0xc9,                        //unpcklps      %xmm1,%xmm9
  0x44,0x0f,0x28,0xd2,                        //movaps        %xmm2,%xmm10
  0x44,0x0f,0x28,0xda,                        //movaps        %xmm2,%xmm11
  0x44,0x0f,0x14,0xdb,                        //unpcklps      %xmm3,%xmm11
  0x44,0x0f,0x15,0xc1,                        //unpckhps      %xmm1,%xmm8
  0x44,0x0f,0x15,0xd3,                        //unpckhps      %xmm3,%xmm10
  0x45,0x0f,0x28,0xe1,                        //movaps        %xmm9,%xmm12
  0x66,0x45,0x0f,0x14,0xe3,                   //unpcklpd      %xmm11,%xmm12
  0x66,0x45,0x0f,0x15,0xcb,                   //unpckhpd      %xmm11,%xmm9
  0x45,0x0f,0x28,0xd8,                        //movaps        %xmm8,%xmm11
  0x66,0x45,0x0f,0x14,0xda,                   //unpcklpd      %xmm10,%xmm11
  0x66,0x45,0x0f,0x15,0xc2,                   //unpckhpd      %xmm10,%xmm8
  0x66,0x44,0x0f,0x11,0x24,0x08,              //movupd        %xmm12,(%rax,%rcx,1)
  0x66,0x44,0x0f,0x11,0x4c,0x08,0x10,         //movupd        %xmm9,0x10(%rax,%rcx,1)
  0x66,0x44,0x0f,0x11,0x5c,0x08,0x20,         //movupd        %xmm11,0x20(%rax,%rcx,1)
  0x66,0x44,0x0f,0x11,0x44,0x08,0x30,         //movupd        %xmm8,0x30(%rax,%rcx,1)
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_x_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x45,0x0f,0x57,0xc0,                        //xorps         %xmm8,%xmm8
  0x44,0x0f,0x5f,0xc0,                        //maxps         %xmm0,%xmm8
  0xf3,0x44,0x0f,0x10,0x08,                   //movss         (%rax),%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x66,0x0f,0x76,0xc0,                        //pcmpeqd       %xmm0,%xmm0
  0x66,0x41,0x0f,0xfe,0xc1,                   //paddd         %xmm9,%xmm0
  0x44,0x0f,0x5d,0xc0,                        //minps         %xmm0,%xmm8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x41,0x0f,0x28,0xc0,                        //movaps        %xmm8,%xmm0
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_clamp_y_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x45,0x0f,0x57,0xc0,                        //xorps         %xmm8,%xmm8
  0x44,0x0f,0x5f,0xc1,                        //maxps         %xmm1,%xmm8
  0xf3,0x44,0x0f,0x10,0x08,                   //movss         (%rax),%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x66,0x0f,0x76,0xc9,                        //pcmpeqd       %xmm1,%xmm1
  0x66,0x41,0x0f,0xfe,0xc9,                   //paddd         %xmm9,%xmm1
  0x44,0x0f,0x5d,0xc1,                        //minps         %xmm1,%xmm8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x41,0x0f,0x28,0xc8,                        //movaps        %xmm8,%xmm1
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_repeat_x_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x44,0x0f,0x10,0x00,                   //movss         (%rax),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x44,0x0f,0x28,0xc8,                        //movaps        %xmm0,%xmm9
  0x45,0x0f,0x5e,0xc8,                        //divps         %xmm8,%xmm9
  0xf3,0x45,0x0f,0x5b,0xd1,                   //cvttps2dq     %xmm9,%xmm10
  0x45,0x0f,0x5b,0xd2,                        //cvtdq2ps      %xmm10,%xmm10
  0x45,0x0f,0xc2,0xca,0x01,                   //cmpltps       %xmm10,%xmm9
  0xf3,0x44,0x0f,0x10,0x1a,                   //movss         (%rdx),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0x45,0x0f,0x54,0xd9,                        //andps         %xmm9,%xmm11
  0x45,0x0f,0x5c,0xd3,                        //subps         %xmm11,%xmm10
  0x45,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm10
  0x41,0x0f,0x5c,0xc2,                        //subps         %xmm10,%xmm0
  0x66,0x45,0x0f,0x76,0xc9,                   //pcmpeqd       %xmm9,%xmm9
  0x66,0x45,0x0f,0xfe,0xc8,                   //paddd         %xmm8,%xmm9
  0x41,0x0f,0x5d,0xc1,                        //minps         %xmm9,%xmm0
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_repeat_y_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x44,0x0f,0x10,0x00,                   //movss         (%rax),%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x44,0x0f,0x28,0xc9,                        //movaps        %xmm1,%xmm9
  0x45,0x0f,0x5e,0xc8,                        //divps         %xmm8,%xmm9
  0xf3,0x45,0x0f,0x5b,0xd1,                   //cvttps2dq     %xmm9,%xmm10
  0x45,0x0f,0x5b,0xd2,                        //cvtdq2ps      %xmm10,%xmm10
  0x45,0x0f,0xc2,0xca,0x01,                   //cmpltps       %xmm10,%xmm9
  0xf3,0x44,0x0f,0x10,0x1a,                   //movss         (%rdx),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0x45,0x0f,0x54,0xd9,                        //andps         %xmm9,%xmm11
  0x45,0x0f,0x5c,0xd3,                        //subps         %xmm11,%xmm10
  0x45,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm10
  0x41,0x0f,0x5c,0xca,                        //subps         %xmm10,%xmm1
  0x66,0x45,0x0f,0x76,0xc9,                   //pcmpeqd       %xmm9,%xmm9
  0x66,0x45,0x0f,0xfe,0xc8,                   //paddd         %xmm8,%xmm9
  0x41,0x0f,0x5d,0xc9,                        //minps         %xmm9,%xmm1
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_mirror_x_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x44,0x0f,0x10,0x08,                   //movss         (%rax),%xmm9
  0x45,0x0f,0x28,0xc1,                        //movaps        %xmm9,%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x41,0x0f,0x5c,0xc0,                        //subps         %xmm8,%xmm0
  0xf3,0x45,0x0f,0x58,0xc9,                   //addss         %xmm9,%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x44,0x0f,0x28,0xd0,                        //movaps        %xmm0,%xmm10
  0x45,0x0f,0x5e,0xd1,                        //divps         %xmm9,%xmm10
  0xf3,0x45,0x0f,0x5b,0xda,                   //cvttps2dq     %xmm10,%xmm11
  0x45,0x0f,0x5b,0xdb,                        //cvtdq2ps      %xmm11,%xmm11
  0x45,0x0f,0xc2,0xd3,0x01,                   //cmpltps       %xmm11,%xmm10
  0xf3,0x44,0x0f,0x10,0x22,                   //movss         (%rdx),%xmm12
  0x45,0x0f,0xc6,0xe4,0x00,                   //shufps        $0x0,%xmm12,%xmm12
  0x45,0x0f,0x54,0xe2,                        //andps         %xmm10,%xmm12
  0x45,0x0f,0x57,0xd2,                        //xorps         %xmm10,%xmm10
  0x45,0x0f,0x5c,0xdc,                        //subps         %xmm12,%xmm11
  0x45,0x0f,0x59,0xd9,                        //mulps         %xmm9,%xmm11
  0x41,0x0f,0x5c,0xc3,                        //subps         %xmm11,%xmm0
  0x41,0x0f,0x5c,0xc0,                        //subps         %xmm8,%xmm0
  0x44,0x0f,0x5c,0xd0,                        //subps         %xmm0,%xmm10
  0x41,0x0f,0x54,0xc2,                        //andps         %xmm10,%xmm0
  0x66,0x45,0x0f,0x76,0xc9,                   //pcmpeqd       %xmm9,%xmm9
  0x66,0x45,0x0f,0xfe,0xc8,                   //paddd         %xmm8,%xmm9
  0x41,0x0f,0x5d,0xc1,                        //minps         %xmm9,%xmm0
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_mirror_y_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x44,0x0f,0x10,0x08,                   //movss         (%rax),%xmm9
  0x45,0x0f,0x28,0xc1,                        //movaps        %xmm9,%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x41,0x0f,0x5c,0xc8,                        //subps         %xmm8,%xmm1
  0xf3,0x45,0x0f,0x58,0xc9,                   //addss         %xmm9,%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0x44,0x0f,0x28,0xd1,                        //movaps        %xmm1,%xmm10
  0x45,0x0f,0x5e,0xd1,                        //divps         %xmm9,%xmm10
  0xf3,0x45,0x0f,0x5b,0xda,                   //cvttps2dq     %xmm10,%xmm11
  0x45,0x0f,0x5b,0xdb,                        //cvtdq2ps      %xmm11,%xmm11
  0x45,0x0f,0xc2,0xd3,0x01,                   //cmpltps       %xmm11,%xmm10
  0xf3,0x44,0x0f,0x10,0x22,                   //movss         (%rdx),%xmm12
  0x45,0x0f,0xc6,0xe4,0x00,                   //shufps        $0x0,%xmm12,%xmm12
  0x45,0x0f,0x54,0xe2,                        //andps         %xmm10,%xmm12
  0x45,0x0f,0x57,0xd2,                        //xorps         %xmm10,%xmm10
  0x45,0x0f,0x5c,0xdc,                        //subps         %xmm12,%xmm11
  0x45,0x0f,0x59,0xd9,                        //mulps         %xmm9,%xmm11
  0x41,0x0f,0x5c,0xcb,                        //subps         %xmm11,%xmm1
  0x41,0x0f,0x5c,0xc8,                        //subps         %xmm8,%xmm1
  0x44,0x0f,0x5c,0xd1,                        //subps         %xmm1,%xmm10
  0x41,0x0f,0x54,0xca,                        //andps         %xmm10,%xmm1
  0x66,0x45,0x0f,0x76,0xc9,                   //pcmpeqd       %xmm9,%xmm9
  0x66,0x45,0x0f,0xfe,0xc8,                   //paddd         %xmm8,%xmm9
  0x41,0x0f,0x5d,0xc9,                        //minps         %xmm9,%xmm1
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_matrix_2x3_sse2[] = {
  0x44,0x0f,0x28,0xc9,                        //movaps        %xmm1,%xmm9
  0x44,0x0f,0x28,0xc0,                        //movaps        %xmm0,%xmm8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x0f,0x10,0x00,                        //movss         (%rax),%xmm0
  0xf3,0x0f,0x10,0x48,0x04,                   //movss         0x4(%rax),%xmm1
  0x0f,0xc6,0xc0,0x00,                        //shufps        $0x0,%xmm0,%xmm0
  0xf3,0x44,0x0f,0x10,0x50,0x08,              //movss         0x8(%rax),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0xf3,0x44,0x0f,0x10,0x58,0x10,              //movss         0x10(%rax),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0x45,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm10
  0x45,0x0f,0x58,0xd3,                        //addps         %xmm11,%xmm10
  0x41,0x0f,0x59,0xc0,                        //mulps         %xmm8,%xmm0
  0x41,0x0f,0x58,0xc2,                        //addps         %xmm10,%xmm0
  0x0f,0xc6,0xc9,0x00,                        //shufps        $0x0,%xmm1,%xmm1
  0xf3,0x44,0x0f,0x10,0x50,0x0c,              //movss         0xc(%rax),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0xf3,0x44,0x0f,0x10,0x58,0x14,              //movss         0x14(%rax),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0x45,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm10
  0x45,0x0f,0x58,0xd3,                        //addps         %xmm11,%xmm10
  0x41,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm1
  0x41,0x0f,0x58,0xca,                        //addps         %xmm10,%xmm1
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_matrix_3x4_sse2[] = {
  0x44,0x0f,0x28,0xc9,                        //movaps        %xmm1,%xmm9
  0x44,0x0f,0x28,0xc0,                        //movaps        %xmm0,%xmm8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x0f,0x10,0x00,                        //movss         (%rax),%xmm0
  0xf3,0x0f,0x10,0x48,0x04,                   //movss         0x4(%rax),%xmm1
  0x0f,0xc6,0xc0,0x00,                        //shufps        $0x0,%xmm0,%xmm0
  0xf3,0x44,0x0f,0x10,0x50,0x0c,              //movss         0xc(%rax),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0xf3,0x44,0x0f,0x10,0x58,0x18,              //movss         0x18(%rax),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0xf3,0x44,0x0f,0x10,0x60,0x24,              //movss         0x24(%rax),%xmm12
  0x45,0x0f,0xc6,0xe4,0x00,                   //shufps        $0x0,%xmm12,%xmm12
  0x44,0x0f,0x59,0xda,                        //mulps         %xmm2,%xmm11
  0x45,0x0f,0x58,0xdc,                        //addps         %xmm12,%xmm11
  0x45,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm10
  0x45,0x0f,0x58,0xd3,                        //addps         %xmm11,%xmm10
  0x41,0x0f,0x59,0xc0,                        //mulps         %xmm8,%xmm0
  0x41,0x0f,0x58,0xc2,                        //addps         %xmm10,%xmm0
  0x0f,0xc6,0xc9,0x00,                        //shufps        $0x0,%xmm1,%xmm1
  0xf3,0x44,0x0f,0x10,0x50,0x10,              //movss         0x10(%rax),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0xf3,0x44,0x0f,0x10,0x58,0x1c,              //movss         0x1c(%rax),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0xf3,0x44,0x0f,0x10,0x60,0x28,              //movss         0x28(%rax),%xmm12
  0x45,0x0f,0xc6,0xe4,0x00,                   //shufps        $0x0,%xmm12,%xmm12
  0x44,0x0f,0x59,0xda,                        //mulps         %xmm2,%xmm11
  0x45,0x0f,0x58,0xdc,                        //addps         %xmm12,%xmm11
  0x45,0x0f,0x59,0xd1,                        //mulps         %xmm9,%xmm10
  0x45,0x0f,0x58,0xd3,                        //addps         %xmm11,%xmm10
  0x41,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm1
  0x41,0x0f,0x58,0xca,                        //addps         %xmm10,%xmm1
  0xf3,0x44,0x0f,0x10,0x50,0x08,              //movss         0x8(%rax),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0xf3,0x44,0x0f,0x10,0x58,0x14,              //movss         0x14(%rax),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0xf3,0x44,0x0f,0x10,0x60,0x20,              //movss         0x20(%rax),%xmm12
  0x45,0x0f,0xc6,0xe4,0x00,                   //shufps        $0x0,%xmm12,%xmm12
  0xf3,0x44,0x0f,0x10,0x68,0x2c,              //movss         0x2c(%rax),%xmm13
  0x45,0x0f,0xc6,0xed,0x00,                   //shufps        $0x0,%xmm13,%xmm13
  0x44,0x0f,0x59,0xe2,                        //mulps         %xmm2,%xmm12
  0x45,0x0f,0x58,0xe5,                        //addps         %xmm13,%xmm12
  0x45,0x0f,0x59,0xd9,                        //mulps         %xmm9,%xmm11
  0x45,0x0f,0x58,0xdc,                        //addps         %xmm12,%xmm11
  0x45,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm10
  0x45,0x0f,0x58,0xd3,                        //addps         %xmm11,%xmm10
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x41,0x0f,0x28,0xd2,                        //movaps        %xmm10,%xmm2
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_matrix_perspective_sse2[] = {
  0x44,0x0f,0x28,0xc0,                        //movaps        %xmm0,%xmm8
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0xf3,0x0f,0x10,0x00,                        //movss         (%rax),%xmm0
  0xf3,0x44,0x0f,0x10,0x48,0x04,              //movss         0x4(%rax),%xmm9
  0x0f,0xc6,0xc0,0x00,                        //shufps        $0x0,%xmm0,%xmm0
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0xf3,0x44,0x0f,0x10,0x50,0x08,              //movss         0x8(%rax),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0x44,0x0f,0x59,0xc9,                        //mulps         %xmm1,%xmm9
  0x45,0x0f,0x58,0xca,                        //addps         %xmm10,%xmm9
  0x41,0x0f,0x59,0xc0,                        //mulps         %xmm8,%xmm0
  0x41,0x0f,0x58,0xc1,                        //addps         %xmm9,%xmm0
  0xf3,0x44,0x0f,0x10,0x48,0x0c,              //movss         0xc(%rax),%xmm9
  0x45,0x0f,0xc6,0xc9,0x00,                   //shufps        $0x0,%xmm9,%xmm9
  0xf3,0x44,0x0f,0x10,0x50,0x10,              //movss         0x10(%rax),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0xf3,0x44,0x0f,0x10,0x58,0x14,              //movss         0x14(%rax),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0x44,0x0f,0x59,0xd1,                        //mulps         %xmm1,%xmm10
  0x45,0x0f,0x58,0xd3,                        //addps         %xmm11,%xmm10
  0x45,0x0f,0x59,0xc8,                        //mulps         %xmm8,%xmm9
  0x45,0x0f,0x58,0xca,                        //addps         %xmm10,%xmm9
  0xf3,0x44,0x0f,0x10,0x50,0x18,              //movss         0x18(%rax),%xmm10
  0x45,0x0f,0xc6,0xd2,0x00,                   //shufps        $0x0,%xmm10,%xmm10
  0xf3,0x44,0x0f,0x10,0x58,0x1c,              //movss         0x1c(%rax),%xmm11
  0x45,0x0f,0xc6,0xdb,0x00,                   //shufps        $0x0,%xmm11,%xmm11
  0xf3,0x44,0x0f,0x10,0x60,0x20,              //movss         0x20(%rax),%xmm12
  0x45,0x0f,0xc6,0xe4,0x00,                   //shufps        $0x0,%xmm12,%xmm12
  0x44,0x0f,0x59,0xd9,                        //mulps         %xmm1,%xmm11
  0x45,0x0f,0x58,0xdc,                        //addps         %xmm12,%xmm11
  0x45,0x0f,0x59,0xd0,                        //mulps         %xmm8,%xmm10
  0x45,0x0f,0x58,0xd3,                        //addps         %xmm11,%xmm10
  0x41,0x0f,0x53,0xca,                        //rcpps         %xmm10,%xmm1
  0x0f,0x59,0xc1,                             //mulps         %xmm1,%xmm0
  0x44,0x0f,0x59,0xc9,                        //mulps         %xmm1,%xmm9
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x41,0x0f,0x28,0xc9,                        //movaps        %xmm9,%xmm1
  0xff,0xe0,                                  //jmpq          *%rax
};

CODE const uint8_t sk_linear_gradient_2stops_sse2[] = {
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x44,0x0f,0x10,0x08,                        //movups        (%rax),%xmm9
  0x0f,0x10,0x58,0x10,                        //movups        0x10(%rax),%xmm3
  0x44,0x0f,0x28,0xc3,                        //movaps        %xmm3,%xmm8
  0x45,0x0f,0xc6,0xc0,0x00,                   //shufps        $0x0,%xmm8,%xmm8
  0x41,0x0f,0x28,0xc9,                        //movaps        %xmm9,%xmm1
  0x0f,0xc6,0xc9,0x00,                        //shufps        $0x0,%xmm1,%xmm1
  0x44,0x0f,0x59,0xc0,                        //mulps         %xmm0,%xmm8
  0x44,0x0f,0x58,0xc1,                        //addps         %xmm1,%xmm8
  0x0f,0x28,0xcb,                             //movaps        %xmm3,%xmm1
  0x0f,0xc6,0xc9,0x55,                        //shufps        $0x55,%xmm1,%xmm1
  0x41,0x0f,0x28,0xd1,                        //movaps        %xmm9,%xmm2
  0x0f,0xc6,0xd2,0x55,                        //shufps        $0x55,%xmm2,%xmm2
  0x0f,0x59,0xc8,                             //mulps         %xmm0,%xmm1
  0x0f,0x58,0xca,                             //addps         %xmm2,%xmm1
  0x0f,0x28,0xd3,                             //movaps        %xmm3,%xmm2
  0x0f,0xc6,0xd2,0xaa,                        //shufps        $0xaa,%xmm2,%xmm2
  0x45,0x0f,0x28,0xd1,                        //movaps        %xmm9,%xmm10
  0x45,0x0f,0xc6,0xd2,0xaa,                   //shufps        $0xaa,%xmm10,%xmm10
  0x0f,0x59,0xd0,                             //mulps         %xmm0,%xmm2
  0x41,0x0f,0x58,0xd2,                        //addps         %xmm10,%xmm2
  0x0f,0xc6,0xdb,0xff,                        //shufps        $0xff,%xmm3,%xmm3
  0x45,0x0f,0xc6,0xc9,0xff,                   //shufps        $0xff,%xmm9,%xmm9
  0x0f,0x59,0xd8,                             //mulps         %xmm0,%xmm3
  0x41,0x0f,0x58,0xd9,                        //addps         %xmm9,%xmm3
  0x48,0xad,                                  //lods          %ds:(%rsi),%rax
  0x41,0x0f,0x28,0xc0,                        //movaps        %xmm8,%xmm0
  0xff,0xe0,                                  //jmpq          *%rax
};
#endif
