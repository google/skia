/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJumper_generated_DEFINED
#define SkJumper_generated_DEFINED

// This file is generated semi-automatically with this command:
//   $ src/jumper/build_stages.py

static const unsigned int aarch64_sk_start_pipeline[] = {
    0xf8408423,                                     //  ldr           x3, [x1],#8
    0x6f00e400,                                     //  movi          v0.2d, #0x0
    0x6f00e401,                                     //  movi          v1.2d, #0x0
    0x6f00e402,                                     //  movi          v2.2d, #0x0
    0x6f00e403,                                     //  movi          v3.2d, #0x0
    0x6f00e404,                                     //  movi          v4.2d, #0x0
    0x6f00e405,                                     //  movi          v5.2d, #0x0
    0x6f00e406,                                     //  movi          v6.2d, #0x0
    0x6f00e407,                                     //  movi          v7.2d, #0x0
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_just_return[] = {
    0xd65f03c0,                                     //  ret
};
static const unsigned int aarch64_sk_seed_shader[] = {
    0xaa0203e9,                                     //  mov           x9, x2
    0xa8c10c28,                                     //  ldp           x8, x3, [x1],#16
    0x4ddfc922,                                     //  ld1r          {v2.4s}, [x9], #4
    0x3cc14041,                                     //  ldur          q1, [x2,#20]
    0x4e040c00,                                     //  dup           v0.4s, w0
    0x4d40c903,                                     //  ld1r          {v3.4s}, [x8]
    0x4d40c924,                                     //  ld1r          {v4.4s}, [x9]
    0x4e21d800,                                     //  scvtf         v0.4s, v0.4s
    0x6f00e405,                                     //  movi          v5.2d, #0x0
    0x4e21d863,                                     //  scvtf         v3.4s, v3.4s
    0x4e24d400,                                     //  fadd          v0.4s, v0.4s, v4.4s
    0x4e20d420,                                     //  fadd          v0.4s, v1.4s, v0.4s
    0x4e24d461,                                     //  fadd          v1.4s, v3.4s, v4.4s
    0x6f00e403,                                     //  movi          v3.2d, #0x0
    0x6f00e404,                                     //  movi          v4.2d, #0x0
    0x6f00e406,                                     //  movi          v6.2d, #0x0
    0x6f00e407,                                     //  movi          v7.2d, #0x0
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_constant_color[] = {
    0xa8c10c28,                                     //  ldp           x8, x3, [x1],#16
    0x3dc00103,                                     //  ldr           q3, [x8]
    0x4e040460,                                     //  dup           v0.4s, v3.s[0]
    0x4e0c0461,                                     //  dup           v1.4s, v3.s[1]
    0x4e140462,                                     //  dup           v2.4s, v3.s[2]
    0x4e1c0463,                                     //  dup           v3.4s, v3.s[3]
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_clear[] = {
    0xf9400423,                                     //  ldr           x3, [x1,#8]
    0x91004028,                                     //  add           x8, x1, #0x10
    0x6f00e400,                                     //  movi          v0.2d, #0x0
    0x6f00e401,                                     //  movi          v1.2d, #0x0
    0x6f00e402,                                     //  movi          v2.2d, #0x0
    0x6f00e403,                                     //  movi          v3.2d, #0x0
    0xaa0803e1,                                     //  mov           x1, x8
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_plus_[] = {
    0xf9400423,                                     //  ldr           x3, [x1,#8]
    0x4e24d400,                                     //  fadd          v0.4s, v0.4s, v4.4s
    0x4e25d421,                                     //  fadd          v1.4s, v1.4s, v5.4s
    0x4e26d442,                                     //  fadd          v2.4s, v2.4s, v6.4s
    0x4e27d463,                                     //  fadd          v3.4s, v3.4s, v7.4s
    0x91004021,                                     //  add           x1, x1, #0x10
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_srcover[] = {
    0x4d40c850,                                     //  ld1r          {v16.4s}, [x2]
    0xf9400423,                                     //  ldr           x3, [x1,#8]
    0x91004021,                                     //  add           x1, x1, #0x10
    0x4ea3d610,                                     //  fsub          v16.4s, v16.4s, v3.4s
    0x4e24ce00,                                     //  fmla          v0.4s, v16.4s, v4.4s
    0x4e25ce01,                                     //  fmla          v1.4s, v16.4s, v5.4s
    0x4e26ce02,                                     //  fmla          v2.4s, v16.4s, v6.4s
    0x4e27ce03,                                     //  fmla          v3.4s, v16.4s, v7.4s
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_dstover[] = {
    0x4d40c851,                                     //  ld1r          {v17.4s}, [x2]
    0xf9400423,                                     //  ldr           x3, [x1,#8]
    0x4ea41c90,                                     //  mov           v16.16b, v4.16b
    0x4ea61cd2,                                     //  mov           v18.16b, v6.16b
    0x4ea7d634,                                     //  fsub          v20.4s, v17.4s, v7.4s
    0x4ea51cb1,                                     //  mov           v17.16b, v5.16b
    0x4ea71cf3,                                     //  mov           v19.16b, v7.16b
    0x4e20ce90,                                     //  fmla          v16.4s, v20.4s, v0.4s
    0x4e21ce91,                                     //  fmla          v17.4s, v20.4s, v1.4s
    0x4e22ce92,                                     //  fmla          v18.4s, v20.4s, v2.4s
    0x4e23ce93,                                     //  fmla          v19.4s, v20.4s, v3.4s
    0x91004021,                                     //  add           x1, x1, #0x10
    0x4eb01e00,                                     //  mov           v0.16b, v16.16b
    0x4eb11e21,                                     //  mov           v1.16b, v17.16b
    0x4eb21e42,                                     //  mov           v2.16b, v18.16b
    0x4eb31e63,                                     //  mov           v3.16b, v19.16b
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_clamp_0[] = {
    0xf9400423,                                     //  ldr           x3, [x1,#8]
    0x6f00e410,                                     //  movi          v16.2d, #0x0
    0x4e30f400,                                     //  fmax          v0.4s, v0.4s, v16.4s
    0x4e30f421,                                     //  fmax          v1.4s, v1.4s, v16.4s
    0x4e30f442,                                     //  fmax          v2.4s, v2.4s, v16.4s
    0x4e30f463,                                     //  fmax          v3.4s, v3.4s, v16.4s
    0x91004021,                                     //  add           x1, x1, #0x10
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_clamp_1[] = {
    0x4d40c850,                                     //  ld1r          {v16.4s}, [x2]
    0xf9400423,                                     //  ldr           x3, [x1,#8]
    0x91004021,                                     //  add           x1, x1, #0x10
    0x4eb0f400,                                     //  fmin          v0.4s, v0.4s, v16.4s
    0x4eb0f421,                                     //  fmin          v1.4s, v1.4s, v16.4s
    0x4eb0f442,                                     //  fmin          v2.4s, v2.4s, v16.4s
    0x4eb0f463,                                     //  fmin          v3.4s, v3.4s, v16.4s
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_clamp_a[] = {
    0x4d40c850,                                     //  ld1r          {v16.4s}, [x2]
    0xf9400423,                                     //  ldr           x3, [x1,#8]
    0x91004021,                                     //  add           x1, x1, #0x10
    0x4eb0f463,                                     //  fmin          v3.4s, v3.4s, v16.4s
    0x4ea3f400,                                     //  fmin          v0.4s, v0.4s, v3.4s
    0x4ea3f421,                                     //  fmin          v1.4s, v1.4s, v3.4s
    0x4ea3f442,                                     //  fmin          v2.4s, v2.4s, v3.4s
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_swap[] = {
    0xf9400423,                                     //  ldr           x3, [x1,#8]
    0x4ea31c70,                                     //  mov           v16.16b, v3.16b
    0x4ea21c51,                                     //  mov           v17.16b, v2.16b
    0x4ea11c32,                                     //  mov           v18.16b, v1.16b
    0x4ea01c13,                                     //  mov           v19.16b, v0.16b
    0x91004021,                                     //  add           x1, x1, #0x10
    0x4ea41c80,                                     //  mov           v0.16b, v4.16b
    0x4ea51ca1,                                     //  mov           v1.16b, v5.16b
    0x4ea61cc2,                                     //  mov           v2.16b, v6.16b
    0x4ea71ce3,                                     //  mov           v3.16b, v7.16b
    0x4eb31e64,                                     //  mov           v4.16b, v19.16b
    0x4eb21e45,                                     //  mov           v5.16b, v18.16b
    0x4eb11e26,                                     //  mov           v6.16b, v17.16b
    0x4eb01e07,                                     //  mov           v7.16b, v16.16b
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_move_src_dst[] = {
    0xf9400423,                                     //  ldr           x3, [x1,#8]
    0x91004021,                                     //  add           x1, x1, #0x10
    0x4ea01c04,                                     //  mov           v4.16b, v0.16b
    0x4ea11c25,                                     //  mov           v5.16b, v1.16b
    0x4ea21c46,                                     //  mov           v6.16b, v2.16b
    0x4ea31c67,                                     //  mov           v7.16b, v3.16b
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_move_dst_src[] = {
    0xf9400423,                                     //  ldr           x3, [x1,#8]
    0x91004021,                                     //  add           x1, x1, #0x10
    0x4ea41c80,                                     //  mov           v0.16b, v4.16b
    0x4ea51ca1,                                     //  mov           v1.16b, v5.16b
    0x4ea61cc2,                                     //  mov           v2.16b, v6.16b
    0x4ea71ce3,                                     //  mov           v3.16b, v7.16b
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_premul[] = {
    0xf9400423,                                     //  ldr           x3, [x1,#8]
    0x6e23dc00,                                     //  fmul          v0.4s, v0.4s, v3.4s
    0x6e23dc21,                                     //  fmul          v1.4s, v1.4s, v3.4s
    0x6e23dc42,                                     //  fmul          v2.4s, v2.4s, v3.4s
    0x91004021,                                     //  add           x1, x1, #0x10
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_unpremul[] = {
    0x4d40c850,                                     //  ld1r          {v16.4s}, [x2]
    0xf9400423,                                     //  ldr           x3, [x1,#8]
    0x4ea0d871,                                     //  fcmeq         v17.4s, v3.4s, #0.0
    0x91004021,                                     //  add           x1, x1, #0x10
    0x6e23fe10,                                     //  fdiv          v16.4s, v16.4s, v3.4s
    0x4e711e10,                                     //  bic           v16.16b, v16.16b, v17.16b
    0x6e20de00,                                     //  fmul          v0.4s, v16.4s, v0.4s
    0x6e21de01,                                     //  fmul          v1.4s, v16.4s, v1.4s
    0x6e22de02,                                     //  fmul          v2.4s, v16.4s, v2.4s
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_from_srgb[] = {
    0x9100e048,                                     //  add           x8, x2, #0x38
    0x4d40c910,                                     //  ld1r          {v16.4s}, [x8]
    0x9100d048,                                     //  add           x8, x2, #0x34
    0x4d40c911,                                     //  ld1r          {v17.4s}, [x8]
    0x2d47cc52,                                     //  ldp           s18, s19, [x2,#60]
    0x6e22dc54,                                     //  fmul          v20.4s, v2.4s, v2.4s
    0x4eb01e15,                                     //  mov           v21.16b, v16.16b
    0x4eb01e17,                                     //  mov           v23.16b, v16.16b
    0x4f921050,                                     //  fmla          v16.4s, v2.4s, v18.s[0]
    0x4eb11e36,                                     //  mov           v22.16b, v17.16b
    0x4eb11e38,                                     //  mov           v24.16b, v17.16b
    0x4e34ce11,                                     //  fmla          v17.4s, v16.4s, v20.4s
    0x6e20dc10,                                     //  fmul          v16.4s, v0.4s, v0.4s
    0x91011048,                                     //  add           x8, x2, #0x44
    0x4f921015,                                     //  fmla          v21.4s, v0.4s, v18.s[0]
    0x4e30ceb6,                                     //  fmla          v22.4s, v21.4s, v16.4s
    0x4d40c910,                                     //  ld1r          {v16.4s}, [x8]
    0xf9400423,                                     //  ldr           x3, [x1,#8]
    0x6e21dc34,                                     //  fmul          v20.4s, v1.4s, v1.4s
    0x4f921037,                                     //  fmla          v23.4s, v1.4s, v18.s[0]
    0x4f939015,                                     //  fmul          v21.4s, v0.4s, v19.s[0]
    0x4f939032,                                     //  fmul          v18.4s, v1.4s, v19.s[0]
    0x4f939053,                                     //  fmul          v19.4s, v2.4s, v19.s[0]
    0x6ea0e600,                                     //  fcmgt         v0.4s, v16.4s, v0.4s
    0x6ea1e601,                                     //  fcmgt         v1.4s, v16.4s, v1.4s
    0x6ea2e602,                                     //  fcmgt         v2.4s, v16.4s, v2.4s
    0x4e34cef8,                                     //  fmla          v24.4s, v23.4s, v20.4s
    0x6e711e62,                                     //  bsl           v2.16b, v19.16b, v17.16b
    0x6e761ea0,                                     //  bsl           v0.16b, v21.16b, v22.16b
    0x6e781e41,                                     //  bsl           v1.16b, v18.16b, v24.16b
    0x91004021,                                     //  add           x1, x1, #0x10
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_to_srgb[] = {
    0x6ea1d811,                                     //  frsqrte       v17.4s, v0.4s
    0x6ea1d835,                                     //  frsqrte       v21.4s, v1.4s
    0x6e31de37,                                     //  fmul          v23.4s, v17.4s, v17.4s
    0x6ea1d856,                                     //  frsqrte       v22.4s, v2.4s
    0x6e35deb9,                                     //  fmul          v25.4s, v21.4s, v21.4s
    0x4eb7fc17,                                     //  frsqrts       v23.4s, v0.4s, v23.4s
    0x91015048,                                     //  add           x8, x2, #0x54
    0x6e36deda,                                     //  fmul          v26.4s, v22.4s, v22.4s
    0x4eb9fc39,                                     //  frsqrts       v25.4s, v1.4s, v25.4s
    0x6e37de31,                                     //  fmul          v17.4s, v17.4s, v23.4s
    0x2d494052,                                     //  ldp           s18, s16, [x2,#72]
    0x4d40c914,                                     //  ld1r          {v20.4s}, [x8]
    0x4ebafc5a,                                     //  frsqrts       v26.4s, v2.4s, v26.4s
    0x6e39deb5,                                     //  fmul          v21.4s, v21.4s, v25.4s
    0x4ea1da37,                                     //  frecpe        v23.4s, v17.4s
    0xbd405053,                                     //  ldr           s19, [x2,#80]
    0x91016048,                                     //  add           x8, x2, #0x58
    0x6e3aded6,                                     //  fmul          v22.4s, v22.4s, v26.4s
    0x4ea1dabb,                                     //  frecpe        v27.4s, v21.4s
    0x4e37fe3d,                                     //  frecps        v29.4s, v17.4s, v23.4s
    0x4d40c918,                                     //  ld1r          {v24.4s}, [x8]
    0x4ea1dadc,                                     //  frecpe        v28.4s, v22.4s
    0x6e3ddef7,                                     //  fmul          v23.4s, v23.4s, v29.4s
    0x4e3bfebd,                                     //  frecps        v29.4s, v21.4s, v27.4s
    0x6e3ddf7b,                                     //  fmul          v27.4s, v27.4s, v29.4s
    0x4e3cfedd,                                     //  frecps        v29.4s, v22.4s, v28.4s
    0x6e3ddf9c,                                     //  fmul          v28.4s, v28.4s, v29.4s
    0x4eb41e9d,                                     //  mov           v29.16b, v20.16b
    0x6ea1da39,                                     //  frsqrte       v25.4s, v17.4s
    0x4f9312fd,                                     //  fmla          v29.4s, v23.4s, v19.s[0]
    0x4eb41e97,                                     //  mov           v23.16b, v20.16b
    0x4f92901a,                                     //  fmul          v26.4s, v0.4s, v18.s[0]
    0x4f931377,                                     //  fmla          v23.4s, v27.4s, v19.s[0]
    0x4f931394,                                     //  fmla          v20.4s, v28.4s, v19.s[0]
    0x4f929033,                                     //  fmul          v19.4s, v1.4s, v18.s[0]
    0x4f929052,                                     //  fmul          v18.4s, v2.4s, v18.s[0]
    0x6ea0e700,                                     //  fcmgt         v0.4s, v24.4s, v0.4s
    0x6ea1e701,                                     //  fcmgt         v1.4s, v24.4s, v1.4s
    0x6ea2e702,                                     //  fcmgt         v2.4s, v24.4s, v2.4s
    0x6e39df38,                                     //  fmul          v24.4s, v25.4s, v25.4s
    0x6ea1dabb,                                     //  frsqrte       v27.4s, v21.4s
    0x4eb8fe31,                                     //  frsqrts       v17.4s, v17.4s, v24.4s
    0x6ea1dadc,                                     //  frsqrte       v28.4s, v22.4s
    0x6e3bdf78,                                     //  fmul          v24.4s, v27.4s, v27.4s
    0x6e31df31,                                     //  fmul          v17.4s, v25.4s, v17.4s
    0x4eb8feb5,                                     //  frsqrts       v21.4s, v21.4s, v24.4s
    0x6e3cdf98,                                     //  fmul          v24.4s, v28.4s, v28.4s
    0x4f90123d,                                     //  fmla          v29.4s, v17.4s, v16.s[0]
    0x4d40c851,                                     //  ld1r          {v17.4s}, [x2]
    0x4eb8fed6,                                     //  frsqrts       v22.4s, v22.4s, v24.4s
    0x6e35df75,                                     //  fmul          v21.4s, v27.4s, v21.4s
    0x6e36df96,                                     //  fmul          v22.4s, v28.4s, v22.4s
    0xf9400423,                                     //  ldr           x3, [x1,#8]
    0x4f9012b7,                                     //  fmla          v23.4s, v21.4s, v16.s[0]
    0x4f9012d4,                                     //  fmla          v20.4s, v22.4s, v16.s[0]
    0x4ebdf630,                                     //  fmin          v16.4s, v17.4s, v29.4s
    0x4eb7f635,                                     //  fmin          v21.4s, v17.4s, v23.4s
    0x4eb4f631,                                     //  fmin          v17.4s, v17.4s, v20.4s
    0x6e701f40,                                     //  bsl           v0.16b, v26.16b, v16.16b
    0x6e751e61,                                     //  bsl           v1.16b, v19.16b, v21.16b
    0x6e711e42,                                     //  bsl           v2.16b, v18.16b, v17.16b
    0x91004021,                                     //  add           x1, x1, #0x10
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_scale_u8[] = {
    0xa8c10c28,                                     //  ldp           x8, x3, [x1],#16
    0xbd400c51,                                     //  ldr           s17, [x2,#12]
    0xf9400108,                                     //  ldr           x8, [x8]
    0x8b000108,                                     //  add           x8, x8, x0
    0x39400109,                                     //  ldrb          w9, [x8]
    0x3940050a,                                     //  ldrb          w10, [x8,#1]
    0x3940090b,                                     //  ldrb          w11, [x8,#2]
    0x39400d08,                                     //  ldrb          w8, [x8,#3]
    0x4e021d30,                                     //  mov           v16.h[0], w9
    0x4e061d50,                                     //  mov           v16.h[1], w10
    0x4e0a1d70,                                     //  mov           v16.h[2], w11
    0x4e0e1d10,                                     //  mov           v16.h[3], w8
    0x2f07b7f0,                                     //  bic           v16.4h, #0xff, lsl #8
    0x2f10a610,                                     //  uxtl          v16.4s, v16.4h
    0x6e21da10,                                     //  ucvtf         v16.4s, v16.4s
    0x4f919210,                                     //  fmul          v16.4s, v16.4s, v17.s[0]
    0x6e20de00,                                     //  fmul          v0.4s, v16.4s, v0.4s
    0x6e21de01,                                     //  fmul          v1.4s, v16.4s, v1.4s
    0x6e22de02,                                     //  fmul          v2.4s, v16.4s, v2.4s
    0x6e23de03,                                     //  fmul          v3.4s, v16.4s, v3.4s
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_load_tables[] = {
    0xa8c10c28,                                     //  ldp           x8, x3, [x1],#16
    0x9100404b,                                     //  add           x11, x2, #0x10
    0x4d40c960,                                     //  ld1r          {v0.4s}, [x11]
    0xd37ef409,                                     //  lsl           x9, x0, #2
    0xa9402d0a,                                     //  ldp           x10, x11, [x8]
    0x3ce96942,                                     //  ldr           q2, [x10,x9]
    0xa9412109,                                     //  ldp           x9, x8, [x8,#16]
    0x4e221c01,                                     //  and           v1.16b, v0.16b, v2.16b
    0x0e143c2c,                                     //  mov           w12, v1.s[2]
    0xbc6c5971,                                     //  ldr           s17, [x11,w12,uxtw #2]
    0x1e26002c,                                     //  fmov          w12, s1
    0x6f380443,                                     //  ushr          v3.4s, v2.4s, #8
    0x6f300450,                                     //  ushr          v16.4s, v2.4s, #16
    0x8b2c496c,                                     //  add           x12, x11, w12, uxtw #2
    0x0e0c3c2a,                                     //  mov           w10, v1.s[1]
    0x0e1c3c2d,                                     //  mov           w13, v1.s[3]
    0x4e231c01,                                     //  and           v1.16b, v0.16b, v3.16b
    0x4e301c03,                                     //  and           v3.16b, v0.16b, v16.16b
    0x0d408180,                                     //  ld1           {v0.s}[0], [x12]
    0x0e143c2c,                                     //  mov           w12, v1.s[2]
    0xbc6c5932,                                     //  ldr           s18, [x9,w12,uxtw #2]
    0x1e26002c,                                     //  fmov          w12, s1
    0x8b2a496a,                                     //  add           x10, x11, w10, uxtw #2
    0xbc6d5970,                                     //  ldr           s16, [x11,w13,uxtw #2]
    0x0e0c3c2b,                                     //  mov           w11, v1.s[1]
    0x0e1c3c2d,                                     //  mov           w13, v1.s[3]
    0x8b2c492c,                                     //  add           x12, x9, w12, uxtw #2
    0xbc6d5933,                                     //  ldr           s19, [x9,w13,uxtw #2]
    0x0e0c3c6d,                                     //  mov           w13, v3.s[1]
    0x8b2b4929,                                     //  add           x9, x9, w11, uxtw #2
    0x0e143c6b,                                     //  mov           w11, v3.s[2]
    0x0d408181,                                     //  ld1           {v1.s}[0], [x12]
    0x0e1c3c6c,                                     //  mov           w12, v3.s[3]
    0x0d409140,                                     //  ld1           {v0.s}[1], [x10]
    0x1e26006a,                                     //  fmov          w10, s3
    0xbd400c43,                                     //  ldr           s3, [x2,#12]
    0x6f280442,                                     //  ushr          v2.4s, v2.4s, #24
    0x4e21d842,                                     //  scvtf         v2.4s, v2.4s
    0x8b2a490a,                                     //  add           x10, x8, w10, uxtw #2
    0x4f839043,                                     //  fmul          v3.4s, v2.4s, v3.s[0]
    0x0d408142,                                     //  ld1           {v2.s}[0], [x10]
    0x8b2d490a,                                     //  add           x10, x8, w13, uxtw #2
    0x6e140620,                                     //  mov           v0.s[2], v17.s[0]
    0xbc6b5911,                                     //  ldr           s17, [x8,w11,uxtw #2]
    0x0d409121,                                     //  ld1           {v1.s}[1], [x9]
    0x0d409142,                                     //  ld1           {v2.s}[1], [x10]
    0x6e1c0600,                                     //  mov           v0.s[3], v16.s[0]
    0xbc6c5910,                                     //  ldr           s16, [x8,w12,uxtw #2]
    0x6e140641,                                     //  mov           v1.s[2], v18.s[0]
    0x6e140622,                                     //  mov           v2.s[2], v17.s[0]
    0x6e1c0661,                                     //  mov           v1.s[3], v19.s[0]
    0x6e1c0602,                                     //  mov           v2.s[3], v16.s[0]
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_load_8888[] = {
    0xa8c10c28,                                     //  ldp           x8, x3, [x1],#16
    0xd37ef409,                                     //  lsl           x9, x0, #2
    0xbd400c42,                                     //  ldr           s2, [x2,#12]
    0xf9400108,                                     //  ldr           x8, [x8]
    0x3ce96900,                                     //  ldr           q0, [x8,x9]
    0x91004048,                                     //  add           x8, x2, #0x10
    0x4d40c901,                                     //  ld1r          {v1.4s}, [x8]
    0x6f380410,                                     //  ushr          v16.4s, v0.4s, #8
    0x6f300411,                                     //  ushr          v17.4s, v0.4s, #16
    0x4e201c23,                                     //  and           v3.16b, v1.16b, v0.16b
    0x6f280400,                                     //  ushr          v0.4s, v0.4s, #24
    0x4e301c30,                                     //  and           v16.16b, v1.16b, v16.16b
    0x4e311c21,                                     //  and           v1.16b, v1.16b, v17.16b
    0x4e21d863,                                     //  scvtf         v3.4s, v3.4s
    0x4e21d811,                                     //  scvtf         v17.4s, v0.4s
    0x4e21da10,                                     //  scvtf         v16.4s, v16.4s
    0x4e21d832,                                     //  scvtf         v18.4s, v1.4s
    0x4f829060,                                     //  fmul          v0.4s, v3.4s, v2.s[0]
    0x4f829223,                                     //  fmul          v3.4s, v17.4s, v2.s[0]
    0x4f829201,                                     //  fmul          v1.4s, v16.4s, v2.s[0]
    0x4f829242,                                     //  fmul          v2.4s, v18.4s, v2.s[0]
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_store_8888[] = {
    0xbd400850,                                     //  ldr           s16, [x2,#8]
    0xf9400028,                                     //  ldr           x8, [x1]
    0xd37ef409,                                     //  lsl           x9, x0, #2
    0x4f909032,                                     //  fmul          v18.4s, v1.4s, v16.s[0]
    0x4f909011,                                     //  fmul          v17.4s, v0.4s, v16.s[0]
    0x6e21aa52,                                     //  fcvtnu        v18.4s, v18.4s
    0x6e21aa31,                                     //  fcvtnu        v17.4s, v17.4s
    0x4f285652,                                     //  shl           v18.4s, v18.4s, #8
    0x4eb11e51,                                     //  orr           v17.16b, v18.16b, v17.16b
    0x4f909052,                                     //  fmul          v18.4s, v2.4s, v16.s[0]
    0xf9400108,                                     //  ldr           x8, [x8]
    0x4f909070,                                     //  fmul          v16.4s, v3.4s, v16.s[0]
    0x6e21aa52,                                     //  fcvtnu        v18.4s, v18.4s
    0x6e21aa10,                                     //  fcvtnu        v16.4s, v16.4s
    0x4f305652,                                     //  shl           v18.4s, v18.4s, #16
    0x4eb21e31,                                     //  orr           v17.16b, v17.16b, v18.16b
    0x4f385610,                                     //  shl           v16.4s, v16.4s, #24
    0x4eb01e30,                                     //  orr           v16.16b, v17.16b, v16.16b
    0x3ca96910,                                     //  str           q16, [x8,x9]
    0xf9400423,                                     //  ldr           x3, [x1,#8]
    0x91004021,                                     //  add           x1, x1, #0x10
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_load_f16[] = {
    0xa8c10c28,                                     //  ldp           x8, x3, [x1],#16
    0xf9400108,                                     //  ldr           x8, [x8]
    0x8b000d08,                                     //  add           x8, x8, x0, lsl #3
    0x0c400510,                                     //  ld4           {v16.4h-v19.4h}, [x8]
    0x0e217a00,                                     //  fcvtl         v0.4s, v16.4h
    0x0e217a21,                                     //  fcvtl         v1.4s, v17.4h
    0x0e217a42,                                     //  fcvtl         v2.4s, v18.4h
    0x0e217a63,                                     //  fcvtl         v3.4s, v19.4h
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_store_f16[] = {
    0xf9400028,                                     //  ldr           x8, [x1]
    0x0e216810,                                     //  fcvtn         v16.4h, v0.4s
    0x0e216831,                                     //  fcvtn         v17.4h, v1.4s
    0x0e216852,                                     //  fcvtn         v18.4h, v2.4s
    0xf9400108,                                     //  ldr           x8, [x8]
    0x0e216873,                                     //  fcvtn         v19.4h, v3.4s
    0x8b000d08,                                     //  add           x8, x8, x0, lsl #3
    0x0c000510,                                     //  st4           {v16.4h-v19.4h}, [x8]
    0xf9400423,                                     //  ldr           x3, [x1,#8]
    0x91004021,                                     //  add           x1, x1, #0x10
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_clamp_x[] = {
    0xa8c10c28,                                     //  ldp           x8, x3, [x1],#16
    0x6f07e7f0,                                     //  movi          v16.2d, #0xffffffffffffffff
    0x6f00e411,                                     //  movi          v17.2d, #0x0
    0x4d40c912,                                     //  ld1r          {v18.4s}, [x8]
    0x4eb08650,                                     //  add           v16.4s, v18.4s, v16.4s
    0x4eb0f400,                                     //  fmin          v0.4s, v0.4s, v16.4s
    0x4e20f620,                                     //  fmax          v0.4s, v17.4s, v0.4s
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_clamp_y[] = {
    0xa8c10c28,                                     //  ldp           x8, x3, [x1],#16
    0x6f07e7f0,                                     //  movi          v16.2d, #0xffffffffffffffff
    0x6f00e411,                                     //  movi          v17.2d, #0x0
    0x4d40c912,                                     //  ld1r          {v18.4s}, [x8]
    0x4eb08650,                                     //  add           v16.4s, v18.4s, v16.4s
    0x4eb0f421,                                     //  fmin          v1.4s, v1.4s, v16.4s
    0x4e21f621,                                     //  fmax          v1.4s, v17.4s, v1.4s
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_matrix_2x3[] = {
    0xa8c10c28,                                     //  ldp           x8, x3, [x1],#16
    0xaa0803e9,                                     //  mov           x9, x8
    0x9100410a,                                     //  add           x10, x8, #0x10
    0x4ddfc932,                                     //  ld1r          {v18.4s}, [x9], #4
    0x4d40c950,                                     //  ld1r          {v16.4s}, [x10]
    0x2d415113,                                     //  ldp           s19, s20, [x8,#8]
    0x9100510a,                                     //  add           x10, x8, #0x14
    0x4d40c951,                                     //  ld1r          {v17.4s}, [x10]
    0x4f931030,                                     //  fmla          v16.4s, v1.4s, v19.s[0]
    0xbd400133,                                     //  ldr           s19, [x9]
    0x4f941031,                                     //  fmla          v17.4s, v1.4s, v20.s[0]
    0x4e20ce50,                                     //  fmla          v16.4s, v18.4s, v0.4s
    0x4f931011,                                     //  fmla          v17.4s, v0.4s, v19.s[0]
    0x4eb01e00,                                     //  mov           v0.16b, v16.16b
    0x4eb11e21,                                     //  mov           v1.16b, v17.16b
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_matrix_3x4[] = {
    0xa8c10c28,                                     //  ldp           x8, x3, [x1],#16
    0xaa0803e9,                                     //  mov           x9, x8
    0x9100910a,                                     //  add           x10, x8, #0x24
    0x4ddfc933,                                     //  ld1r          {v19.4s}, [x9], #4
    0x4d40c950,                                     //  ld1r          {v16.4s}, [x10]
    0x9100a10a,                                     //  add           x10, x8, #0x28
    0x4d40c951,                                     //  ld1r          {v17.4s}, [x10]
    0x9100b10a,                                     //  add           x10, x8, #0x2c
    0x2d435514,                                     //  ldp           s20, s21, [x8,#24]
    0xbd402116,                                     //  ldr           s22, [x8,#32]
    0x4d40c952,                                     //  ld1r          {v18.4s}, [x10]
    0x4f941050,                                     //  fmla          v16.4s, v2.4s, v20.s[0]
    0x2d415d14,                                     //  ldp           s20, s23, [x8,#8]
    0x4f951051,                                     //  fmla          v17.4s, v2.4s, v21.s[0]
    0x4f961052,                                     //  fmla          v18.4s, v2.4s, v22.s[0]
    0x2d425502,                                     //  ldp           s2, s21, [x8,#16]
    0x4f971030,                                     //  fmla          v16.4s, v1.4s, v23.s[0]
    0x4e20ce70,                                     //  fmla          v16.4s, v19.4s, v0.4s
    0x4f821031,                                     //  fmla          v17.4s, v1.4s, v2.s[0]
    0xbd400122,                                     //  ldr           s2, [x9]
    0x4f951032,                                     //  fmla          v18.4s, v1.4s, v21.s[0]
    0x4f941012,                                     //  fmla          v18.4s, v0.4s, v20.s[0]
    0x4f821011,                                     //  fmla          v17.4s, v0.4s, v2.s[0]
    0x4eb01e00,                                     //  mov           v0.16b, v16.16b
    0x4eb11e21,                                     //  mov           v1.16b, v17.16b
    0x4eb21e42,                                     //  mov           v2.16b, v18.16b
    0xd61f0060,                                     //  br            x3
};
static const unsigned int aarch64_sk_linear_gradient_2stops[] = {
    0xa8c10c28,                                     //  ldp           x8, x3, [x1],#16
    0xad404503,                                     //  ldp           q3, q17, [x8]
    0x4e040470,                                     //  dup           v16.4s, v3.s[0]
    0x4e0c0461,                                     //  dup           v1.4s, v3.s[1]
    0x4e140462,                                     //  dup           v2.4s, v3.s[2]
    0x4e1c0463,                                     //  dup           v3.4s, v3.s[3]
    0x4f911010,                                     //  fmla          v16.4s, v0.4s, v17.s[0]
    0x4fb11001,                                     //  fmla          v1.4s, v0.4s, v17.s[1]
    0x4f911802,                                     //  fmla          v2.4s, v0.4s, v17.s[2]
    0x4fb11803,                                     //  fmla          v3.4s, v0.4s, v17.s[3]
    0x4eb01e00,                                     //  mov           v0.16b, v16.16b
    0xd61f0060,                                     //  br            x3
};
static const unsigned int armv7_sk_start_pipeline[] = {
    0xe4913004,                                     //  ldr           r3, [r1], #4
    0xf2800010,                                     //  vmov.i32      d0, #0
    0xf2801010,                                     //  vmov.i32      d1, #0
    0xf2802010,                                     //  vmov.i32      d2, #0
    0xf2803010,                                     //  vmov.i32      d3, #0
    0xf2804010,                                     //  vmov.i32      d4, #0
    0xf2805010,                                     //  vmov.i32      d5, #0
    0xf2806010,                                     //  vmov.i32      d6, #0
    0xf2807010,                                     //  vmov.i32      d7, #0
    0xe12fff13,                                     //  bx            r3
};
static const unsigned int armv7_sk_just_return[] = {
    0xe12fff1e,                                     //  bx            lr
};
static const unsigned int armv7_sk_seed_shader[] = {
    0xe5913000,                                     //  ldr           r3, [r1]
    0xee800b90,                                     //  vdup.32       d16, r0
    0xf3fb0620,                                     //  vcvt.f32.s32  d16, d16
    0xedd23b05,                                     //  vldr          d19, [r2, #20]
    0xe591c004,                                     //  ldr           ip, [r1, #4]
    0xf2803010,                                     //  vmov.i32      d3, #0
    0xf4e31c9f,                                     //  vld1.32       {d17[]}, [r3 :32]
    0xe2823004,                                     //  add           r3, r2, #4
    0xf3fb1621,                                     //  vcvt.f32.s32  d17, d17
    0xe2811008,                                     //  add           r1, r1, #8
    0xf4e32c9f,                                     //  vld1.32       {d18[]}, [r3 :32]
    0xf2804010,                                     //  vmov.i32      d4, #0
    0xf2400da2,                                     //  vadd.f32      d16, d16, d18
    0xf2805010,                                     //  vmov.i32      d5, #0
    0xf4a22c9f,                                     //  vld1.32       {d2[]}, [r2 :32]
    0xf2011da2,                                     //  vadd.f32      d1, d17, d18
    0xf2806010,                                     //  vmov.i32      d6, #0
    0xf2030da0,                                     //  vadd.f32      d0, d19, d16
    0xf2807010,                                     //  vmov.i32      d7, #0
    0xe12fff1c,                                     //  bx            ip
};
static const unsigned int armv7_sk_constant_color[] = {
    0xe5913000,                                     //  ldr           r3, [r1]
    0xe591c004,                                     //  ldr           ip, [r1, #4]
    0xe2811008,                                     //  add           r1, r1, #8
    0xf4630a0f,                                     //  vld1.8        {d16-d17}, [r3]
    0xf3b40c20,                                     //  vdup.32       d0, d16[0]
    0xf3bc1c20,                                     //  vdup.32       d1, d16[1]
    0xf3b42c21,                                     //  vdup.32       d2, d17[0]
    0xf3bc3c21,                                     //  vdup.32       d3, d17[1]
    0xe12fff1c,                                     //  bx            ip
};
static const unsigned int armv7_sk_clear[] = {
    0xe5913004,                                     //  ldr           r3, [r1, #4]
    0xf2800010,                                     //  vmov.i32      d0, #0
    0xf2801010,                                     //  vmov.i32      d1, #0
    0xe2811008,                                     //  add           r1, r1, #8
    0xf2802010,                                     //  vmov.i32      d2, #0
    0xf2803010,                                     //  vmov.i32      d3, #0
    0xe12fff13,                                     //  bx            r3
};
static const unsigned int armv7_sk_plus_[] = {
    0xf2000d04,                                     //  vadd.f32      d0, d0, d4
    0xe2813008,                                     //  add           r3, r1, #8
    0xf2011d05,                                     //  vadd.f32      d1, d1, d5
    0xe591c004,                                     //  ldr           ip, [r1, #4]
    0xf2022d06,                                     //  vadd.f32      d2, d2, d6
    0xe1a01003,                                     //  mov           r1, r3
    0xf2033d07,                                     //  vadd.f32      d3, d3, d7
    0xe12fff1c,                                     //  bx            ip
};
static const unsigned int armv7_sk_srcover[] = {
    0xf4e20c9f,                                     //  vld1.32       {d16[]}, [r2 :32]
    0xe5913004,                                     //  ldr           r3, [r1, #4]
    0xf2600d83,                                     //  vsub.f32      d16, d16, d3
    0xe2811008,                                     //  add           r1, r1, #8
    0xf2040c30,                                     //  vfma.f32      d0, d4, d16
    0xf2051c30,                                     //  vfma.f32      d1, d5, d16
    0xf2062c30,                                     //  vfma.f32      d2, d6, d16
    0xf2073c30,                                     //  vfma.f32      d3, d7, d16
    0xe12fff13,                                     //  bx            r3
};
static const unsigned int armv7_sk_dstover[] = {
    0xf4e20c9f,                                     //  vld1.32       {d16[]}, [r2 :32]
    0xf2651115,                                     //  vorr          d17, d5, d5
    0xf2604d87,                                     //  vsub.f32      d20, d16, d7
    0xf2640114,                                     //  vorr          d16, d4, d4
    0xf2662116,                                     //  vorr          d18, d6, d6
    0xe5913004,                                     //  ldr           r3, [r1, #4]
    0xf2673117,                                     //  vorr          d19, d7, d7
    0xe2811008,                                     //  add           r1, r1, #8
    0xf2400c34,                                     //  vfma.f32      d16, d0, d20
    0xf2411c34,                                     //  vfma.f32      d17, d1, d20
    0xf2422c34,                                     //  vfma.f32      d18, d2, d20
    0xf2433c34,                                     //  vfma.f32      d19, d3, d20
    0xf22001b0,                                     //  vorr          d0, d16, d16
    0xf22111b1,                                     //  vorr          d1, d17, d17
    0xf22221b2,                                     //  vorr          d2, d18, d18
    0xf22331b3,                                     //  vorr          d3, d19, d19
    0xe12fff13,                                     //  bx            r3
};
static const unsigned int armv7_sk_clamp_0[] = {
    0xf2c00010,                                     //  vmov.i32      d16, #0
    0xe5913004,                                     //  ldr           r3, [r1, #4]
    0xe2811008,                                     //  add           r1, r1, #8
    0xf2000f20,                                     //  vmax.f32      d0, d0, d16
    0xf2011f20,                                     //  vmax.f32      d1, d1, d16
    0xf2022f20,                                     //  vmax.f32      d2, d2, d16
    0xf2033f20,                                     //  vmax.f32      d3, d3, d16
    0xe12fff13,                                     //  bx            r3
};
static const unsigned int armv7_sk_clamp_1[] = {
    0xf4e20c9f,                                     //  vld1.32       {d16[]}, [r2 :32]
    0xe5913004,                                     //  ldr           r3, [r1, #4]
    0xf2200f20,                                     //  vmin.f32      d0, d0, d16
    0xe2811008,                                     //  add           r1, r1, #8
    0xf2211f20,                                     //  vmin.f32      d1, d1, d16
    0xf2222f20,                                     //  vmin.f32      d2, d2, d16
    0xf2233f20,                                     //  vmin.f32      d3, d3, d16
    0xe12fff13,                                     //  bx            r3
};
static const unsigned int armv7_sk_clamp_a[] = {
    0xf4e20c9f,                                     //  vld1.32       {d16[]}, [r2 :32]
    0xe5913004,                                     //  ldr           r3, [r1, #4]
    0xf2233f20,                                     //  vmin.f32      d3, d3, d16
    0xe2811008,                                     //  add           r1, r1, #8
    0xf2200f03,                                     //  vmin.f32      d0, d0, d3
    0xf2211f03,                                     //  vmin.f32      d1, d1, d3
    0xf2222f03,                                     //  vmin.f32      d2, d2, d3
    0xe12fff13,                                     //  bx            r3
};
static const unsigned int armv7_sk_swap[] = {
    0xeef00b43,                                     //  vmov.f64      d16, d3
    0xe5913004,                                     //  ldr           r3, [r1, #4]
    0xe2811008,                                     //  add           r1, r1, #8
    0xeef01b42,                                     //  vmov.f64      d17, d2
    0xeef02b41,                                     //  vmov.f64      d18, d1
    0xeef03b40,                                     //  vmov.f64      d19, d0
    0xeeb00b44,                                     //  vmov.f64      d0, d4
    0xeeb01b45,                                     //  vmov.f64      d1, d5
    0xeeb02b46,                                     //  vmov.f64      d2, d6
    0xeeb03b47,                                     //  vmov.f64      d3, d7
    0xeeb04b63,                                     //  vmov.f64      d4, d19
    0xeeb05b62,                                     //  vmov.f64      d5, d18
    0xeeb06b61,                                     //  vmov.f64      d6, d17
    0xeeb07b60,                                     //  vmov.f64      d7, d16
    0xe12fff13,                                     //  bx            r3
};
static const unsigned int armv7_sk_move_src_dst[] = {
    0xeeb04b40,                                     //  vmov.f64      d4, d0
    0xe5913004,                                     //  ldr           r3, [r1, #4]
    0xe2811008,                                     //  add           r1, r1, #8
    0xeeb05b41,                                     //  vmov.f64      d5, d1
    0xeeb06b42,                                     //  vmov.f64      d6, d2
    0xeeb07b43,                                     //  vmov.f64      d7, d3
    0xe12fff13,                                     //  bx            r3
};
static const unsigned int armv7_sk_move_dst_src[] = {
    0xeeb00b44,                                     //  vmov.f64      d0, d4
    0xe5913004,                                     //  ldr           r3, [r1, #4]
    0xe2811008,                                     //  add           r1, r1, #8
    0xeeb01b45,                                     //  vmov.f64      d1, d5
    0xeeb02b46,                                     //  vmov.f64      d2, d6
    0xeeb03b47,                                     //  vmov.f64      d3, d7
    0xe12fff13,                                     //  bx            r3
};
static const unsigned int armv7_sk_premul[] = {
    0xf3000d13,                                     //  vmul.f32      d0, d0, d3
    0xe5913004,                                     //  ldr           r3, [r1, #4]
    0xf3011d13,                                     //  vmul.f32      d1, d1, d3
    0xe2811008,                                     //  add           r1, r1, #8
    0xf3022d13,                                     //  vmul.f32      d2, d2, d3
    0xe12fff13,                                     //  bx            r3
};
static const unsigned int armv7_sk_unpremul[] = {
    0xed2d8b04,                                     //  vpush         {d8-d9}
    0xed928a00,                                     //  vldr          s16, [r2]
    0xf2c00010,                                     //  vmov.i32      d16, #0
    0xf3f91503,                                     //  vceq.f32      d17, d3, #0
    0xe5913004,                                     //  ldr           r3, [r1, #4]
    0xeec89a23,                                     //  vdiv.f32      s19, s16, s7
    0xe2811008,                                     //  add           r1, r1, #8
    0xee889a03,                                     //  vdiv.f32      s18, s16, s6
    0xf3501199,                                     //  vbsl          d17, d16, d9
    0xf3010d90,                                     //  vmul.f32      d0, d17, d0
    0xf3011d91,                                     //  vmul.f32      d1, d17, d1
    0xf3012d92,                                     //  vmul.f32      d2, d17, d2
    0xecbd8b04,                                     //  vpop          {d8-d9}
    0xe12fff13,                                     //  bx            r3
};
static const unsigned int armv7_sk_from_srgb[] = {
    0xed2d8b02,                                     //  vpush         {d8}
    0xe282303c,                                     //  add           r3, r2, #60
    0xed928a10,                                     //  vldr          s16, [r2, #64]
    0xf3402d10,                                     //  vmul.f32      d18, d0, d0
    0xf4e30c9f,                                     //  vld1.32       {d16[]}, [r3 :32]
    0xe2823038,                                     //  add           r3, r2, #56
    0xf3413d11,                                     //  vmul.f32      d19, d1, d1
    0xf4e31c9f,                                     //  vld1.32       {d17[]}, [r3 :32]
    0xe2823044,                                     //  add           r3, r2, #68
    0xf26141b1,                                     //  vorr          d20, d17, d17
    0xf26171b1,                                     //  vorr          d23, d17, d17
    0xf4e38c9f,                                     //  vld1.32       {d24[]}, [r3 :32]
    0xf2404c30,                                     //  vfma.f32      d20, d0, d16
    0xe2823034,                                     //  add           r3, r2, #52
    0xf2417c30,                                     //  vfma.f32      d23, d1, d16
    0xf2421c30,                                     //  vfma.f32      d17, d2, d16
    0xf3425d12,                                     //  vmul.f32      d21, d2, d2
    0xf2e16948,                                     //  vmul.f32      d22, d1, d8[0]
    0xf2e00948,                                     //  vmul.f32      d16, d0, d8[0]
    0xf2e29948,                                     //  vmul.f32      d25, d2, d8[0]
    0xf3282e82,                                     //  vcgt.f32      d2, d24, d2
    0xf3281e81,                                     //  vcgt.f32      d1, d24, d1
    0xf3280e80,                                     //  vcgt.f32      d0, d24, d0
    0xf4e38c9f,                                     //  vld1.32       {d24[]}, [r3 :32]
    0xf268a1b8,                                     //  vorr          d26, d24, d24
    0xf242acb4,                                     //  vfma.f32      d26, d18, d20
    0xf26821b8,                                     //  vorr          d18, d24, d24
    0xe5913004,                                     //  ldr           r3, [r1, #4]
    0xf2432cb7,                                     //  vfma.f32      d18, d19, d23
    0xe2811008,                                     //  add           r1, r1, #8
    0xf2458cb1,                                     //  vfma.f32      d24, d21, d17
    0xf31001ba,                                     //  vbsl          d0, d16, d26
    0xf31611b2,                                     //  vbsl          d1, d22, d18
    0xf31921b8,                                     //  vbsl          d2, d25, d24
    0xecbd8b02,                                     //  vpop          {d8}
    0xe12fff13,                                     //  bx            r3
};
static const unsigned int armv7_sk_to_srgb[] = {
    0xed2d8b02,                                     //  vpush         {d8}
    0xf3fb0580,                                     //  vrsqrte.f32   d16, d0
    0xe2823050,                                     //  add           r3, r2, #80
    0xf3fb1581,                                     //  vrsqrte.f32   d17, d1
    0xed928a12,                                     //  vldr          s16, [r2, #72]
    0xf3fb2582,                                     //  vrsqrte.f32   d18, d2
    0xf3403db0,                                     //  vmul.f32      d19, d16, d16
    0xf3414db1,                                     //  vmul.f32      d20, d17, d17
    0xf3425db2,                                     //  vmul.f32      d21, d18, d18
    0xf2603f33,                                     //  vrsqrts.f32   d19, d0, d19
    0xf2614f34,                                     //  vrsqrts.f32   d20, d1, d20
    0xf2625f35,                                     //  vrsqrts.f32   d21, d2, d21
    0xf3400db3,                                     //  vmul.f32      d16, d16, d19
    0xf3411db4,                                     //  vmul.f32      d17, d17, d20
    0xf3422db5,                                     //  vmul.f32      d18, d18, d21
    0xf3fb3520,                                     //  vrecpe.f32    d19, d16
    0xf3fb4521,                                     //  vrecpe.f32    d20, d17
    0xf3fb6522,                                     //  vrecpe.f32    d22, d18
    0xf3fb55a2,                                     //  vrsqrte.f32   d21, d18
    0xf3fb75a0,                                     //  vrsqrte.f32   d23, d16
    0xf3fb85a1,                                     //  vrsqrte.f32   d24, d17
    0xf2409fb3,                                     //  vrecps.f32    d25, d16, d19
    0xf241afb4,                                     //  vrecps.f32    d26, d17, d20
    0xf242bfb6,                                     //  vrecps.f32    d27, d18, d22
    0xf345cdb5,                                     //  vmul.f32      d28, d21, d21
    0xf347ddb7,                                     //  vmul.f32      d29, d23, d23
    0xf348edb8,                                     //  vmul.f32      d30, d24, d24
    0xf2622fbc,                                     //  vrsqrts.f32   d18, d18, d28
    0xf2600fbd,                                     //  vrsqrts.f32   d16, d16, d29
    0xf2611fbe,                                     //  vrsqrts.f32   d17, d17, d30
    0xf3433db9,                                     //  vmul.f32      d19, d19, d25
    0xf4e39c9f,                                     //  vld1.32       {d25[]}, [r3 :32]
    0xe2823054,                                     //  add           r3, r2, #84
    0xf3444dba,                                     //  vmul.f32      d20, d20, d26
    0xf3466dbb,                                     //  vmul.f32      d22, d22, d27
    0xf4e3ac9f,                                     //  vld1.32       {d26[]}, [r3 :32]
    0xe282304c,                                     //  add           r3, r2, #76
    0xf26ab1ba,                                     //  vorr          d27, d26, d26
    0xf249bcb3,                                     //  vfma.f32      d27, d25, d19
    0xf26a31ba,                                     //  vorr          d19, d26, d26
    0xf2493cb4,                                     //  vfma.f32      d19, d25, d20
    0xf4e34c9f,                                     //  vld1.32       {d20[]}, [r3 :32]
    0xf249acb6,                                     //  vfma.f32      d26, d25, d22
    0xe2823058,                                     //  add           r3, r2, #88
    0xf3452db2,                                     //  vmul.f32      d18, d21, d18
    0xf3470db0,                                     //  vmul.f32      d16, d23, d16
    0xf3481db1,                                     //  vmul.f32      d17, d24, d17
    0xf2e05948,                                     //  vmul.f32      d21, d0, d8[0]
    0xf244bcb0,                                     //  vfma.f32      d27, d20, d16
    0xf4e30c9f,                                     //  vld1.32       {d16[]}, [r3 :32]
    0xf2443cb1,                                     //  vfma.f32      d19, d20, d17
    0xf244acb2,                                     //  vfma.f32      d26, d20, d18
    0xf4e24c9f,                                     //  vld1.32       {d20[]}, [r2 :32]
    0xf2e11948,                                     //  vmul.f32      d17, d1, d8[0]
    0xf2e22948,                                     //  vmul.f32      d18, d2, d8[0]
    0xf3201e81,                                     //  vcgt.f32      d1, d16, d1
    0xe5913004,                                     //  ldr           r3, [r1, #4]
    0xf3200e80,                                     //  vcgt.f32      d0, d16, d0
    0xe2811008,                                     //  add           r1, r1, #8
    0xf3202e82,                                     //  vcgt.f32      d2, d16, d2
    0xf2640fab,                                     //  vmin.f32      d16, d20, d27
    0xf2643fa3,                                     //  vmin.f32      d19, d20, d19
    0xf2644faa,                                     //  vmin.f32      d20, d20, d26
    0xf31501b0,                                     //  vbsl          d0, d21, d16
    0xf31111b3,                                     //  vbsl          d1, d17, d19
    0xf31221b4,                                     //  vbsl          d2, d18, d20
    0xecbd8b02,                                     //  vpop          {d8}
    0xe12fff13,                                     //  bx            r3
};
static const unsigned int armv7_sk_scale_u8[] = {
    0xed2d8b02,                                     //  vpush         {d8}
    0xe24dd008,                                     //  sub           sp, sp, #8
    0xe5913000,                                     //  ldr           r3, [r1]
    0xe591c004,                                     //  ldr           ip, [r1, #4]
    0xe2811008,                                     //  add           r1, r1, #8
    0xe5933000,                                     //  ldr           r3, [r3]
    0xe0833000,                                     //  add           r3, r3, r0
    0xe1d330b0,                                     //  ldrh          r3, [r3]
    0xe1cd30b4,                                     //  strh          r3, [sp, #4]
    0xe28d3004,                                     //  add           r3, sp, #4
    0xed928a03,                                     //  vldr          s16, [r2, #12]
    0xf4e3041f,                                     //  vld1.16       {d16[0]}, [r3 :16]
    0xf3c80a30,                                     //  vmovl.u8      q8, d16
    0xf3d00a30,                                     //  vmovl.u16     q8, d16
    0xf3fb06a0,                                     //  vcvt.f32.u32  d16, d16
    0xf2e009c8,                                     //  vmul.f32      d16, d16, d8[0]
    0xf3000d90,                                     //  vmul.f32      d0, d16, d0
    0xf3001d91,                                     //  vmul.f32      d1, d16, d1
    0xf3002d92,                                     //  vmul.f32      d2, d16, d2
    0xf3003d93,                                     //  vmul.f32      d3, d16, d3
    0xe28dd008,                                     //  add           sp, sp, #8
    0xecbd8b02,                                     //  vpop          {d8}
    0xe12fff1c,                                     //  bx            ip
};
static const unsigned int armv7_sk_load_tables[] = {
    0xe92d48f0,                                     //  push          {r4, r5, r6, r7, fp, lr}
    0xe5913000,                                     //  ldr           r3, [r1]
    0xe2826010,                                     //  add           r6, r2, #16
    0xed922a03,                                     //  vldr          s4, [r2, #12]
    0xf4e60c9f,                                     //  vld1.32       {d16[]}, [r6 :32]
    0xe591c004,                                     //  ldr           ip, [r1, #4]
    0xe2811008,                                     //  add           r1, r1, #8
    0xe593e000,                                     //  ldr           lr, [r3]
    0xe5934004,                                     //  ldr           r4, [r3, #4]
    0xe08e6100,                                     //  add           r6, lr, r0, lsl #2
    0xe5935008,                                     //  ldr           r5, [r3, #8]
    0xe593700c,                                     //  ldr           r7, [r3, #12]
    0xedd61b00,                                     //  vldr          d17, [r6]
    0xf24021b1,                                     //  vand          d18, d16, d17
    0xf3f03031,                                     //  vshr.u32      d19, d17, #16
    0xee326b90,                                     //  vmov.32       r6, d18[1]
    0xe0846106,                                     //  add           r6, r4, r6, lsl #2
    0xedd60a00,                                     //  vldr          s1, [r6]
    0xee126b90,                                     //  vmov.32       r6, d18[0]
    0xf3f82031,                                     //  vshr.u32      d18, d17, #8
    0xf24021b2,                                     //  vand          d18, d16, d18
    0xf24001b3,                                     //  vand          d16, d16, d19
    0xee103b90,                                     //  vmov.32       r3, d16[0]
    0xe0846106,                                     //  add           r6, r4, r6, lsl #2
    0xee304b90,                                     //  vmov.32       r4, d16[1]
    0xf3e80031,                                     //  vshr.u32      d16, d17, #24
    0xed960a00,                                     //  vldr          s0, [r6]
    0xee326b90,                                     //  vmov.32       r6, d18[1]
    0xf3fb0620,                                     //  vcvt.f32.s32  d16, d16
    0xe0873103,                                     //  add           r3, r7, r3, lsl #2
    0xf2a039c2,                                     //  vmul.f32      d3, d16, d2[0]
    0xe0874104,                                     //  add           r4, r7, r4, lsl #2
    0xedd42a00,                                     //  vldr          s5, [r4]
    0xe0856106,                                     //  add           r6, r5, r6, lsl #2
    0xed932a00,                                     //  vldr          s4, [r3]
    0xedd61a00,                                     //  vldr          s3, [r6]
    0xee126b90,                                     //  vmov.32       r6, d18[0]
    0xe0856106,                                     //  add           r6, r5, r6, lsl #2
    0xed961a00,                                     //  vldr          s2, [r6]
    0xe8bd48f0,                                     //  pop           {r4, r5, r6, r7, fp, lr}
    0xe12fff1c,                                     //  bx            ip
};
static const unsigned int armv7_sk_load_8888[] = {
    0xe92d4800,                                     //  push          {fp, lr}
    0xe5913000,                                     //  ldr           r3, [r1]
    0xed922a03,                                     //  vldr          s4, [r2, #12]
    0xe591c004,                                     //  ldr           ip, [r1, #4]
    0xe2811008,                                     //  add           r1, r1, #8
    0xe593e000,                                     //  ldr           lr, [r3]
    0xe2823010,                                     //  add           r3, r2, #16
    0xf4e30c9f,                                     //  vld1.32       {d16[]}, [r3 :32]
    0xe08e3100,                                     //  add           r3, lr, r0, lsl #2
    0xedd31b00,                                     //  vldr          d17, [r3]
    0xf24021b1,                                     //  vand          d18, d16, d17
    0xf3f83031,                                     //  vshr.u32      d19, d17, #8
    0xf3e84031,                                     //  vshr.u32      d20, d17, #24
    0xf3f01031,                                     //  vshr.u32      d17, d17, #16
    0xf24031b3,                                     //  vand          d19, d16, d19
    0xf24001b1,                                     //  vand          d16, d16, d17
    0xf3fb2622,                                     //  vcvt.f32.s32  d18, d18
    0xf3fb4624,                                     //  vcvt.f32.s32  d20, d20
    0xf3fb1623,                                     //  vcvt.f32.s32  d17, d19
    0xf3fb0620,                                     //  vcvt.f32.s32  d16, d16
    0xf2a209c2,                                     //  vmul.f32      d0, d18, d2[0]
    0xf2a439c2,                                     //  vmul.f32      d3, d20, d2[0]
    0xf2a119c2,                                     //  vmul.f32      d1, d17, d2[0]
    0xf2a029c2,                                     //  vmul.f32      d2, d16, d2[0]
    0xe8bd4800,                                     //  pop           {fp, lr}
    0xe12fff1c,                                     //  bx            ip
};
static const unsigned int armv7_sk_store_8888[] = {
    0xe2823008,                                     //  add           r3, r2, #8
    0xf2c3261f,                                     //  vmov.i32      d18, #1056964608
    0xf2c3361f,                                     //  vmov.i32      d19, #1056964608
    0xf4e31c9f,                                     //  vld1.32       {d17[]}, [r3 :32]
    0xf2c3061f,                                     //  vmov.i32      d16, #1056964608
    0xf2412c31,                                     //  vfma.f32      d18, d1, d17
    0xf2423c31,                                     //  vfma.f32      d19, d2, d17
    0xf2c3461f,                                     //  vmov.i32      d20, #1056964608
    0xe5913000,                                     //  ldr           r3, [r1]
    0xf2400c31,                                     //  vfma.f32      d16, d0, d17
    0xf2434c31,                                     //  vfma.f32      d20, d3, d17
    0xe5933000,                                     //  ldr           r3, [r3]
    0xe0833100,                                     //  add           r3, r3, r0, lsl #2
    0xf3fb17a2,                                     //  vcvt.u32.f32  d17, d18
    0xf3fb27a3,                                     //  vcvt.u32.f32  d18, d19
    0xf3fb07a0,                                     //  vcvt.u32.f32  d16, d16
    0xf3fb37a4,                                     //  vcvt.u32.f32  d19, d20
    0xf2e81531,                                     //  vshl.s32      d17, d17, #8
    0xf2f02532,                                     //  vshl.s32      d18, d18, #16
    0xf26101b0,                                     //  vorr          d16, d17, d16
    0xf2f81533,                                     //  vshl.s32      d17, d19, #24
    0xf26001b2,                                     //  vorr          d16, d16, d18
    0xf26001b1,                                     //  vorr          d16, d16, d17
    0xedc30b00,                                     //  vstr          d16, [r3]
    0xe5913004,                                     //  ldr           r3, [r1, #4]
    0xe2811008,                                     //  add           r1, r1, #8
    0xe12fff13,                                     //  bx            r3
};
static const unsigned int armv7_sk_load_f16[] = {
    0xed2d8b04,                                     //  vpush         {d8-d9}
    0xe5913000,                                     //  ldr           r3, [r1]
    0xe591c004,                                     //  ldr           ip, [r1, #4]
    0xe2811008,                                     //  add           r1, r1, #8
    0xe5933000,                                     //  ldr           r3, [r3]
    0xe0833180,                                     //  add           r3, r3, r0, lsl #3
    0xf463084f,                                     //  vld2.16       {d16-d17}, [r3]
    0xf3b62720,                                     //  vcvt.f32.f16  q1, d16
    0xf3b68721,                                     //  vcvt.f32.f16  q4, d17
    0xf2220112,                                     //  vorr          d0, d2, d2
    0xeef00a43,                                     //  vmov.f32      s1, s6
    0xf2281118,                                     //  vorr          d1, d8, d8
    0xeeb03a62,                                     //  vmov.f32      s6, s5
    0xeef01a49,                                     //  vmov.f32      s3, s18
    0xeeb09a68,                                     //  vmov.f32      s18, s17
    0xeeb02b43,                                     //  vmov.f64      d2, d3
    0xeeb03b49,                                     //  vmov.f64      d3, d9
    0xecbd8b04,                                     //  vpop          {d8-d9}
    0xe12fff1c,                                     //  bx            ip
};
static const unsigned int armv7_sk_store_f16[] = {
    0xeef00b41,                                     //  vmov.f64      d16, d1
    0xf2631113,                                     //  vorr          d17, d3, d3
    0xeef03b42,                                     //  vmov.f64      d19, d2
    0xf2602110,                                     //  vorr          d18, d0, d0
    0xf3fa00a1,                                     //  vtrn.32       d16, d17
    0xf3f61620,                                     //  vcvt.f16.f32  d17, q8
    0xf3fa20a3,                                     //  vtrn.32       d18, d19
    0xe5913000,                                     //  ldr           r3, [r1]
    0xf3f60622,                                     //  vcvt.f16.f32  d16, q9
    0xe5933000,                                     //  ldr           r3, [r3]
    0xe0833180,                                     //  add           r3, r3, r0, lsl #3
    0xf443084f,                                     //  vst2.16       {d16-d17}, [r3]
    0xe2813008,                                     //  add           r3, r1, #8
    0xe591c004,                                     //  ldr           ip, [r1, #4]
    0xe1a01003,                                     //  mov           r1, r3
    0xe12fff1c,                                     //  bx            ip
};
static const unsigned int armv7_sk_clamp_x[] = {
    0xe5913000,                                     //  ldr           r3, [r1]
    0xf3c70e1f,                                     //  vmov.i8       d16, #255
    0xe591c004,                                     //  ldr           ip, [r1, #4]
    0xe2811008,                                     //  add           r1, r1, #8
    0xf4e31c9f,                                     //  vld1.32       {d17[]}, [r3 :32]
    0xf26108a0,                                     //  vadd.i32      d16, d17, d16
    0xf2c01010,                                     //  vmov.i32      d17, #0
    0xf2600f20,                                     //  vmin.f32      d16, d0, d16
    0xf2010fa0,                                     //  vmax.f32      d0, d17, d16
    0xe12fff1c,                                     //  bx            ip
};
static const unsigned int armv7_sk_clamp_y[] = {
    0xe5913000,                                     //  ldr           r3, [r1]
    0xf3c70e1f,                                     //  vmov.i8       d16, #255
    0xe591c004,                                     //  ldr           ip, [r1, #4]
    0xe2811008,                                     //  add           r1, r1, #8
    0xf4e31c9f,                                     //  vld1.32       {d17[]}, [r3 :32]
    0xf26108a0,                                     //  vadd.i32      d16, d17, d16
    0xf2c01010,                                     //  vmov.i32      d17, #0
    0xf2610f20,                                     //  vmin.f32      d16, d1, d16
    0xf2011fa0,                                     //  vmax.f32      d1, d17, d16
    0xe12fff1c,                                     //  bx            ip
};
static const unsigned int armv7_sk_matrix_2x3[] = {
    0xe92d4800,                                     //  push          {fp, lr}
    0xe591e000,                                     //  ldr           lr, [r1]
    0xe591c004,                                     //  ldr           ip, [r1, #4]
    0xe2811008,                                     //  add           r1, r1, #8
    0xe28e300c,                                     //  add           r3, lr, #12
    0xf4e32c9f,                                     //  vld1.32       {d18[]}, [r3 :32]
    0xe28e3008,                                     //  add           r3, lr, #8
    0xf4e31c9f,                                     //  vld1.32       {d17[]}, [r3 :32]
    0xe28e3010,                                     //  add           r3, lr, #16
    0xf4e30c9f,                                     //  vld1.32       {d16[]}, [r3 :32]
    0xe28e3014,                                     //  add           r3, lr, #20
    0xf2410c31,                                     //  vfma.f32      d16, d1, d17
    0xf4e31c9f,                                     //  vld1.32       {d17[]}, [r3 :32]
    0xe28e3004,                                     //  add           r3, lr, #4
    0xf2411c32,                                     //  vfma.f32      d17, d1, d18
    0xf4ee2c9f,                                     //  vld1.32       {d18[]}, [lr :32]
    0xf4e33c9f,                                     //  vld1.32       {d19[]}, [r3 :32]
    0xf2400c32,                                     //  vfma.f32      d16, d0, d18
    0xf2401c33,                                     //  vfma.f32      d17, d0, d19
    0xf22001b0,                                     //  vorr          d0, d16, d16
    0xf22111b1,                                     //  vorr          d1, d17, d17
    0xe8bd4800,                                     //  pop           {fp, lr}
    0xe12fff1c,                                     //  bx            ip
};
static const unsigned int armv7_sk_matrix_3x4[] = {
    0xe92d4800,                                     //  push          {fp, lr}
    0xe591e000,                                     //  ldr           lr, [r1]
    0xe591c004,                                     //  ldr           ip, [r1, #4]
    0xe2811008,                                     //  add           r1, r1, #8
    0xe28e3020,                                     //  add           r3, lr, #32
    0xf4e33c9f,                                     //  vld1.32       {d19[]}, [r3 :32]
    0xe28e302c,                                     //  add           r3, lr, #44
    0xf4e30c9f,                                     //  vld1.32       {d16[]}, [r3 :32]
    0xe28e301c,                                     //  add           r3, lr, #28
    0xf2420c33,                                     //  vfma.f32      d16, d2, d19
    0xf4e34c9f,                                     //  vld1.32       {d20[]}, [r3 :32]
    0xe28e3018,                                     //  add           r3, lr, #24
    0xf4e32c9f,                                     //  vld1.32       {d18[]}, [r3 :32]
    0xe28e3024,                                     //  add           r3, lr, #36
    0xf4e31c9f,                                     //  vld1.32       {d17[]}, [r3 :32]
    0xe28e3028,                                     //  add           r3, lr, #40
    0xf2421c32,                                     //  vfma.f32      d17, d2, d18
    0xf4e32c9f,                                     //  vld1.32       {d18[]}, [r3 :32]
    0xe28e3010,                                     //  add           r3, lr, #16
    0xf2422c34,                                     //  vfma.f32      d18, d2, d20
    0xf4e33c9f,                                     //  vld1.32       {d19[]}, [r3 :32]
    0xe28e300c,                                     //  add           r3, lr, #12
    0xf4e34c9f,                                     //  vld1.32       {d20[]}, [r3 :32]
    0xe28e3014,                                     //  add           r3, lr, #20
    0xf2411c34,                                     //  vfma.f32      d17, d1, d20
    0xf4e34c9f,                                     //  vld1.32       {d20[]}, [r3 :32]
    0xf2410c34,                                     //  vfma.f32      d16, d1, d20
    0xe28e3004,                                     //  add           r3, lr, #4
    0xf2412c33,                                     //  vfma.f32      d18, d1, d19
    0xf4ee3c9f,                                     //  vld1.32       {d19[]}, [lr :32]
    0xf4e34c9f,                                     //  vld1.32       {d20[]}, [r3 :32]
    0xe28e3008,                                     //  add           r3, lr, #8
    0xf2401c33,                                     //  vfma.f32      d17, d0, d19
    0xf4e33c9f,                                     //  vld1.32       {d19[]}, [r3 :32]
    0xf2400c33,                                     //  vfma.f32      d16, d0, d19
    0xf2402c34,                                     //  vfma.f32      d18, d0, d20
    0xf22101b1,                                     //  vorr          d0, d17, d17
    0xf22021b0,                                     //  vorr          d2, d16, d16
    0xf22211b2,                                     //  vorr          d1, d18, d18
    0xe8bd4800,                                     //  pop           {fp, lr}
    0xe12fff1c,                                     //  bx            ip
};
static const unsigned int armv7_sk_linear_gradient_2stops[] = {
    0xe5913000,                                     //  ldr           r3, [r1]
    0xe591c004,                                     //  ldr           ip, [r1, #4]
    0xe2811008,                                     //  add           r1, r1, #8
    0xf4632a0d,                                     //  vld1.8        {d18-d19}, [r3]!
    0xf4634a0f,                                     //  vld1.8        {d20-d21}, [r3]
    0xf3f40c22,                                     //  vdup.32       d16, d18[0]
    0xf3f41c24,                                     //  vdup.32       d17, d20[0]
    0xf2400c31,                                     //  vfma.f32      d16, d0, d17
    0xf3fc6c24,                                     //  vdup.32       d22, d20[1]
    0xf3bc1c22,                                     //  vdup.32       d1, d18[1]
    0xf3b42c23,                                     //  vdup.32       d2, d19[0]
    0xf2001c36,                                     //  vfma.f32      d1, d0, d22
    0xf3f41c25,                                     //  vdup.32       d17, d21[0]
    0xf3fc4c25,                                     //  vdup.32       d20, d21[1]
    0xf2002c31,                                     //  vfma.f32      d2, d0, d17
    0xf3bc3c23,                                     //  vdup.32       d3, d19[1]
    0xf2003c34,                                     //  vfma.f32      d3, d0, d20
    0xf22001b0,                                     //  vorr          d0, d16, d16
    0xe12fff1c,                                     //  bx            ip
};
static const unsigned char hsw_sk_start_pipeline[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc5,0xfc,0x57,0xc0,                            //  vxorps        %ymm0,%ymm0,%ymm0
    0xc5,0xf4,0x57,0xc9,                            //  vxorps        %ymm1,%ymm1,%ymm1
    0xc5,0xec,0x57,0xd2,                            //  vxorps        %ymm2,%ymm2,%ymm2
    0xc5,0xe4,0x57,0xdb,                            //  vxorps        %ymm3,%ymm3,%ymm3
    0xc5,0xdc,0x57,0xe4,                            //  vxorps        %ymm4,%ymm4,%ymm4
    0xc5,0xd4,0x57,0xed,                            //  vxorps        %ymm5,%ymm5,%ymm5
    0xc5,0xcc,0x57,0xf6,                            //  vxorps        %ymm6,%ymm6,%ymm6
    0xc5,0xc4,0x57,0xff,                            //  vxorps        %ymm7,%ymm7,%ymm7
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_start_pipeline_ms[] = {
    0x56,                                           //  push          %rsi
    0x57,                                           //  push          %rdi
    0x48,0x81,0xec,0xa8,0x00,0x00,0x00,             //  sub           $0xa8,%rsp
    0xc5,0x78,0x29,0xbc,0x24,0x90,0x00,0x00,0x00,   //  vmovaps       %xmm15,0x90(%rsp)
    0xc5,0x78,0x29,0xb4,0x24,0x80,0x00,0x00,0x00,   //  vmovaps       %xmm14,0x80(%rsp)
    0xc5,0x78,0x29,0x6c,0x24,0x70,                  //  vmovaps       %xmm13,0x70(%rsp)
    0xc5,0x78,0x29,0x64,0x24,0x60,                  //  vmovaps       %xmm12,0x60(%rsp)
    0xc5,0x78,0x29,0x5c,0x24,0x50,                  //  vmovaps       %xmm11,0x50(%rsp)
    0xc5,0x78,0x29,0x54,0x24,0x40,                  //  vmovaps       %xmm10,0x40(%rsp)
    0xc5,0x78,0x29,0x4c,0x24,0x30,                  //  vmovaps       %xmm9,0x30(%rsp)
    0xc5,0x78,0x29,0x44,0x24,0x20,                  //  vmovaps       %xmm8,0x20(%rsp)
    0xc5,0xf8,0x29,0x7c,0x24,0x10,                  //  vmovaps       %xmm7,0x10(%rsp)
    0xc5,0xf8,0x29,0x34,0x24,                       //  vmovaps       %xmm6,(%rsp)
    0x48,0x89,0xd6,                                 //  mov           %rdx,%rsi
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc5,0xfc,0x57,0xc0,                            //  vxorps        %ymm0,%ymm0,%ymm0
    0xc5,0xf4,0x57,0xc9,                            //  vxorps        %ymm1,%ymm1,%ymm1
    0xc5,0xec,0x57,0xd2,                            //  vxorps        %ymm2,%ymm2,%ymm2
    0xc5,0xe4,0x57,0xdb,                            //  vxorps        %ymm3,%ymm3,%ymm3
    0xc5,0xdc,0x57,0xe4,                            //  vxorps        %ymm4,%ymm4,%ymm4
    0xc5,0xd4,0x57,0xed,                            //  vxorps        %ymm5,%ymm5,%ymm5
    0xc5,0xcc,0x57,0xf6,                            //  vxorps        %ymm6,%ymm6,%ymm6
    0xc5,0xc4,0x57,0xff,                            //  vxorps        %ymm7,%ymm7,%ymm7
    0x48,0x89,0xcf,                                 //  mov           %rcx,%rdi
    0x4c,0x89,0xc2,                                 //  mov           %r8,%rdx
    0xff,0xd0,                                      //  callq         *%rax
    0xc5,0xf8,0x28,0x34,0x24,                       //  vmovaps       (%rsp),%xmm6
    0xc5,0xf8,0x28,0x7c,0x24,0x10,                  //  vmovaps       0x10(%rsp),%xmm7
    0xc5,0x78,0x28,0x44,0x24,0x20,                  //  vmovaps       0x20(%rsp),%xmm8
    0xc5,0x78,0x28,0x4c,0x24,0x30,                  //  vmovaps       0x30(%rsp),%xmm9
    0xc5,0x78,0x28,0x54,0x24,0x40,                  //  vmovaps       0x40(%rsp),%xmm10
    0xc5,0x78,0x28,0x5c,0x24,0x50,                  //  vmovaps       0x50(%rsp),%xmm11
    0xc5,0x78,0x28,0x64,0x24,0x60,                  //  vmovaps       0x60(%rsp),%xmm12
    0xc5,0x78,0x28,0x6c,0x24,0x70,                  //  vmovaps       0x70(%rsp),%xmm13
    0xc5,0x78,0x28,0xb4,0x24,0x80,0x00,0x00,0x00,   //  vmovaps       0x80(%rsp),%xmm14
    0xc5,0x78,0x28,0xbc,0x24,0x90,0x00,0x00,0x00,   //  vmovaps       0x90(%rsp),%xmm15
    0x48,0x81,0xc4,0xa8,0x00,0x00,0x00,             //  add           $0xa8,%rsp
    0x5f,                                           //  pop           %rdi
    0x5e,                                           //  pop           %rsi
    0xc5,0xf8,0x77,                                 //  vzeroupper
    0xc3,                                           //  retq
};
static const unsigned char hsw_sk_just_return[] = {
    0xc3,                                           //  retq
};
static const unsigned char hsw_sk_seed_shader[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc5,0xf9,0x6e,0xc7,                            //  vmovd         %edi,%xmm0
    0xc4,0xe2,0x7d,0x18,0xc0,                       //  vbroadcastss  %xmm0,%ymm0
    0xc5,0xfc,0x5b,0xc0,                            //  vcvtdq2ps     %ymm0,%ymm0
    0xc4,0xe2,0x7d,0x18,0x4a,0x04,                  //  vbroadcastss  0x4(%rdx),%ymm1
    0xc5,0xfc,0x58,0xc1,                            //  vaddps        %ymm1,%ymm0,%ymm0
    0xc5,0xfc,0x58,0x42,0x14,                       //  vaddps        0x14(%rdx),%ymm0,%ymm0
    0xc4,0xe2,0x7d,0x18,0x10,                       //  vbroadcastss  (%rax),%ymm2
    0xc5,0xfc,0x5b,0xd2,                            //  vcvtdq2ps     %ymm2,%ymm2
    0xc5,0xec,0x58,0xc9,                            //  vaddps        %ymm1,%ymm2,%ymm1
    0xc4,0xe2,0x7d,0x18,0x12,                       //  vbroadcastss  (%rdx),%ymm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc5,0xe4,0x57,0xdb,                            //  vxorps        %ymm3,%ymm3,%ymm3
    0xc5,0xdc,0x57,0xe4,                            //  vxorps        %ymm4,%ymm4,%ymm4
    0xc5,0xd4,0x57,0xed,                            //  vxorps        %ymm5,%ymm5,%ymm5
    0xc5,0xcc,0x57,0xf6,                            //  vxorps        %ymm6,%ymm6,%ymm6
    0xc5,0xc4,0x57,0xff,                            //  vxorps        %ymm7,%ymm7,%ymm7
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_constant_color[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc4,0xe2,0x7d,0x18,0x00,                       //  vbroadcastss  (%rax),%ymm0
    0xc4,0xe2,0x7d,0x18,0x48,0x04,                  //  vbroadcastss  0x4(%rax),%ymm1
    0xc4,0xe2,0x7d,0x18,0x50,0x08,                  //  vbroadcastss  0x8(%rax),%ymm2
    0xc4,0xe2,0x7d,0x18,0x58,0x0c,                  //  vbroadcastss  0xc(%rax),%ymm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_clear[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc5,0xfc,0x57,0xc0,                            //  vxorps        %ymm0,%ymm0,%ymm0
    0xc5,0xf4,0x57,0xc9,                            //  vxorps        %ymm1,%ymm1,%ymm1
    0xc5,0xec,0x57,0xd2,                            //  vxorps        %ymm2,%ymm2,%ymm2
    0xc5,0xe4,0x57,0xdb,                            //  vxorps        %ymm3,%ymm3,%ymm3
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_plus_[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc5,0xfc,0x58,0xc4,                            //  vaddps        %ymm4,%ymm0,%ymm0
    0xc5,0xf4,0x58,0xcd,                            //  vaddps        %ymm5,%ymm1,%ymm1
    0xc5,0xec,0x58,0xd6,                            //  vaddps        %ymm6,%ymm2,%ymm2
    0xc5,0xe4,0x58,0xdf,                            //  vaddps        %ymm7,%ymm3,%ymm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_srcover[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc4,0x62,0x7d,0x18,0x02,                       //  vbroadcastss  (%rdx),%ymm8
    0xc5,0x3c,0x5c,0xc3,                            //  vsubps        %ymm3,%ymm8,%ymm8
    0xc4,0xc2,0x5d,0xb8,0xc0,                       //  vfmadd231ps   %ymm8,%ymm4,%ymm0
    0xc4,0xc2,0x55,0xb8,0xc8,                       //  vfmadd231ps   %ymm8,%ymm5,%ymm1
    0xc4,0xc2,0x4d,0xb8,0xd0,                       //  vfmadd231ps   %ymm8,%ymm6,%ymm2
    0xc4,0xc2,0x45,0xb8,0xd8,                       //  vfmadd231ps   %ymm8,%ymm7,%ymm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_dstover[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc4,0x62,0x7d,0x18,0x02,                       //  vbroadcastss  (%rdx),%ymm8
    0xc5,0x3c,0x5c,0xc7,                            //  vsubps        %ymm7,%ymm8,%ymm8
    0xc4,0xe2,0x3d,0xa8,0xc4,                       //  vfmadd213ps   %ymm4,%ymm8,%ymm0
    0xc4,0xe2,0x3d,0xa8,0xcd,                       //  vfmadd213ps   %ymm5,%ymm8,%ymm1
    0xc4,0xe2,0x3d,0xa8,0xd6,                       //  vfmadd213ps   %ymm6,%ymm8,%ymm2
    0xc4,0xe2,0x3d,0xa8,0xdf,                       //  vfmadd213ps   %ymm7,%ymm8,%ymm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_clamp_0[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc4,0x41,0x3c,0x57,0xc0,                       //  vxorps        %ymm8,%ymm8,%ymm8
    0xc4,0xc1,0x7c,0x5f,0xc0,                       //  vmaxps        %ymm8,%ymm0,%ymm0
    0xc4,0xc1,0x74,0x5f,0xc8,                       //  vmaxps        %ymm8,%ymm1,%ymm1
    0xc4,0xc1,0x6c,0x5f,0xd0,                       //  vmaxps        %ymm8,%ymm2,%ymm2
    0xc4,0xc1,0x64,0x5f,0xd8,                       //  vmaxps        %ymm8,%ymm3,%ymm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_clamp_1[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc4,0x62,0x7d,0x18,0x02,                       //  vbroadcastss  (%rdx),%ymm8
    0xc4,0xc1,0x7c,0x5d,0xc0,                       //  vminps        %ymm8,%ymm0,%ymm0
    0xc4,0xc1,0x74,0x5d,0xc8,                       //  vminps        %ymm8,%ymm1,%ymm1
    0xc4,0xc1,0x6c,0x5d,0xd0,                       //  vminps        %ymm8,%ymm2,%ymm2
    0xc4,0xc1,0x64,0x5d,0xd8,                       //  vminps        %ymm8,%ymm3,%ymm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_clamp_a[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc4,0x62,0x7d,0x18,0x02,                       //  vbroadcastss  (%rdx),%ymm8
    0xc4,0xc1,0x64,0x5d,0xd8,                       //  vminps        %ymm8,%ymm3,%ymm3
    0xc5,0xfc,0x5d,0xc3,                            //  vminps        %ymm3,%ymm0,%ymm0
    0xc5,0xf4,0x5d,0xcb,                            //  vminps        %ymm3,%ymm1,%ymm1
    0xc5,0xec,0x5d,0xd3,                            //  vminps        %ymm3,%ymm2,%ymm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_swap[] = {
    0xc5,0x7c,0x28,0xc3,                            //  vmovaps       %ymm3,%ymm8
    0xc5,0x7c,0x28,0xca,                            //  vmovaps       %ymm2,%ymm9
    0xc5,0x7c,0x28,0xd1,                            //  vmovaps       %ymm1,%ymm10
    0xc5,0x7c,0x28,0xd8,                            //  vmovaps       %ymm0,%ymm11
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc5,0xfc,0x28,0xc4,                            //  vmovaps       %ymm4,%ymm0
    0xc5,0xfc,0x28,0xcd,                            //  vmovaps       %ymm5,%ymm1
    0xc5,0xfc,0x28,0xd6,                            //  vmovaps       %ymm6,%ymm2
    0xc5,0xfc,0x28,0xdf,                            //  vmovaps       %ymm7,%ymm3
    0xc5,0x7c,0x29,0xdc,                            //  vmovaps       %ymm11,%ymm4
    0xc5,0x7c,0x29,0xd5,                            //  vmovaps       %ymm10,%ymm5
    0xc5,0x7c,0x29,0xce,                            //  vmovaps       %ymm9,%ymm6
    0xc5,0x7c,0x29,0xc7,                            //  vmovaps       %ymm8,%ymm7
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_move_src_dst[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc5,0xfc,0x28,0xe0,                            //  vmovaps       %ymm0,%ymm4
    0xc5,0xfc,0x28,0xe9,                            //  vmovaps       %ymm1,%ymm5
    0xc5,0xfc,0x28,0xf2,                            //  vmovaps       %ymm2,%ymm6
    0xc5,0xfc,0x28,0xfb,                            //  vmovaps       %ymm3,%ymm7
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_move_dst_src[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc5,0xfc,0x28,0xc4,                            //  vmovaps       %ymm4,%ymm0
    0xc5,0xfc,0x28,0xcd,                            //  vmovaps       %ymm5,%ymm1
    0xc5,0xfc,0x28,0xd6,                            //  vmovaps       %ymm6,%ymm2
    0xc5,0xfc,0x28,0xdf,                            //  vmovaps       %ymm7,%ymm3
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_premul[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc5,0xfc,0x59,0xc3,                            //  vmulps        %ymm3,%ymm0,%ymm0
    0xc5,0xf4,0x59,0xcb,                            //  vmulps        %ymm3,%ymm1,%ymm1
    0xc5,0xec,0x59,0xd3,                            //  vmulps        %ymm3,%ymm2,%ymm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_unpremul[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc4,0x41,0x3c,0x57,0xc0,                       //  vxorps        %ymm8,%ymm8,%ymm8
    0xc4,0x41,0x64,0xc2,0xc8,0x00,                  //  vcmpeqps      %ymm8,%ymm3,%ymm9
    0xc4,0x62,0x7d,0x18,0x12,                       //  vbroadcastss  (%rdx),%ymm10
    0xc5,0x2c,0x5e,0xd3,                            //  vdivps        %ymm3,%ymm10,%ymm10
    0xc4,0x43,0x2d,0x4a,0xc0,0x90,                  //  vblendvps     %ymm9,%ymm8,%ymm10,%ymm8
    0xc5,0xbc,0x59,0xc0,                            //  vmulps        %ymm0,%ymm8,%ymm0
    0xc5,0xbc,0x59,0xc9,                            //  vmulps        %ymm1,%ymm8,%ymm1
    0xc5,0xbc,0x59,0xd2,                            //  vmulps        %ymm2,%ymm8,%ymm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_from_srgb[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc4,0x62,0x7d,0x18,0x42,0x40,                  //  vbroadcastss  0x40(%rdx),%ymm8
    0xc5,0x3c,0x59,0xc8,                            //  vmulps        %ymm0,%ymm8,%ymm9
    0xc5,0x7c,0x59,0xd0,                            //  vmulps        %ymm0,%ymm0,%ymm10
    0xc4,0x62,0x7d,0x18,0x5a,0x3c,                  //  vbroadcastss  0x3c(%rdx),%ymm11
    0xc4,0x62,0x7d,0x18,0x62,0x38,                  //  vbroadcastss  0x38(%rdx),%ymm12
    0xc4,0x41,0x7c,0x28,0xeb,                       //  vmovaps       %ymm11,%ymm13
    0xc4,0x42,0x7d,0xa8,0xec,                       //  vfmadd213ps   %ymm12,%ymm0,%ymm13
    0xc4,0x62,0x7d,0x18,0x72,0x34,                  //  vbroadcastss  0x34(%rdx),%ymm14
    0xc4,0x42,0x2d,0xa8,0xee,                       //  vfmadd213ps   %ymm14,%ymm10,%ymm13
    0xc4,0x62,0x7d,0x18,0x52,0x44,                  //  vbroadcastss  0x44(%rdx),%ymm10
    0xc4,0xc1,0x7c,0xc2,0xc2,0x01,                  //  vcmpltps      %ymm10,%ymm0,%ymm0
    0xc4,0xc3,0x15,0x4a,0xc1,0x00,                  //  vblendvps     %ymm0,%ymm9,%ymm13,%ymm0
    0xc5,0x3c,0x59,0xc9,                            //  vmulps        %ymm1,%ymm8,%ymm9
    0xc5,0x74,0x59,0xe9,                            //  vmulps        %ymm1,%ymm1,%ymm13
    0xc4,0x41,0x7c,0x28,0xfb,                       //  vmovaps       %ymm11,%ymm15
    0xc4,0x42,0x75,0xa8,0xfc,                       //  vfmadd213ps   %ymm12,%ymm1,%ymm15
    0xc4,0x42,0x15,0xa8,0xfe,                       //  vfmadd213ps   %ymm14,%ymm13,%ymm15
    0xc4,0xc1,0x74,0xc2,0xca,0x01,                  //  vcmpltps      %ymm10,%ymm1,%ymm1
    0xc4,0xc3,0x05,0x4a,0xc9,0x10,                  //  vblendvps     %ymm1,%ymm9,%ymm15,%ymm1
    0xc5,0x3c,0x59,0xc2,                            //  vmulps        %ymm2,%ymm8,%ymm8
    0xc5,0x6c,0x59,0xca,                            //  vmulps        %ymm2,%ymm2,%ymm9
    0xc4,0x42,0x6d,0xa8,0xdc,                       //  vfmadd213ps   %ymm12,%ymm2,%ymm11
    0xc4,0x42,0x35,0xa8,0xde,                       //  vfmadd213ps   %ymm14,%ymm9,%ymm11
    0xc4,0xc1,0x6c,0xc2,0xd2,0x01,                  //  vcmpltps      %ymm10,%ymm2,%ymm2
    0xc4,0xc3,0x25,0x4a,0xd0,0x20,                  //  vblendvps     %ymm2,%ymm8,%ymm11,%ymm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_to_srgb[] = {
    0xc5,0x7c,0x52,0xc0,                            //  vrsqrtps      %ymm0,%ymm8
    0xc4,0x41,0x7c,0x53,0xc8,                       //  vrcpps        %ymm8,%ymm9
    0xc4,0x41,0x7c,0x52,0xd0,                       //  vrsqrtps      %ymm8,%ymm10
    0xc4,0x62,0x7d,0x18,0x42,0x48,                  //  vbroadcastss  0x48(%rdx),%ymm8
    0xc5,0x3c,0x59,0xd8,                            //  vmulps        %ymm0,%ymm8,%ymm11
    0xc4,0x62,0x7d,0x18,0x22,                       //  vbroadcastss  (%rdx),%ymm12
    0xc4,0x62,0x7d,0x18,0x6a,0x4c,                  //  vbroadcastss  0x4c(%rdx),%ymm13
    0xc4,0x62,0x7d,0x18,0x72,0x50,                  //  vbroadcastss  0x50(%rdx),%ymm14
    0xc4,0x62,0x7d,0x18,0x7a,0x54,                  //  vbroadcastss  0x54(%rdx),%ymm15
    0xc4,0x42,0x0d,0xa8,0xcf,                       //  vfmadd213ps   %ymm15,%ymm14,%ymm9
    0xc4,0x42,0x15,0xb8,0xca,                       //  vfmadd231ps   %ymm10,%ymm13,%ymm9
    0xc4,0x41,0x1c,0x5d,0xc9,                       //  vminps        %ymm9,%ymm12,%ymm9
    0xc4,0x62,0x7d,0x18,0x52,0x58,                  //  vbroadcastss  0x58(%rdx),%ymm10
    0xc4,0xc1,0x7c,0xc2,0xc2,0x01,                  //  vcmpltps      %ymm10,%ymm0,%ymm0
    0xc4,0xc3,0x35,0x4a,0xc3,0x00,                  //  vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
    0xc5,0x7c,0x52,0xc9,                            //  vrsqrtps      %ymm1,%ymm9
    0xc4,0x41,0x7c,0x53,0xd9,                       //  vrcpps        %ymm9,%ymm11
    0xc4,0x41,0x7c,0x52,0xc9,                       //  vrsqrtps      %ymm9,%ymm9
    0xc4,0x42,0x0d,0xa8,0xdf,                       //  vfmadd213ps   %ymm15,%ymm14,%ymm11
    0xc4,0x42,0x15,0xb8,0xd9,                       //  vfmadd231ps   %ymm9,%ymm13,%ymm11
    0xc5,0x3c,0x59,0xc9,                            //  vmulps        %ymm1,%ymm8,%ymm9
    0xc4,0x41,0x1c,0x5d,0xdb,                       //  vminps        %ymm11,%ymm12,%ymm11
    0xc4,0xc1,0x74,0xc2,0xca,0x01,                  //  vcmpltps      %ymm10,%ymm1,%ymm1
    0xc4,0xc3,0x25,0x4a,0xc9,0x10,                  //  vblendvps     %ymm1,%ymm9,%ymm11,%ymm1
    0xc5,0x7c,0x52,0xca,                            //  vrsqrtps      %ymm2,%ymm9
    0xc4,0x41,0x7c,0x53,0xd9,                       //  vrcpps        %ymm9,%ymm11
    0xc4,0x42,0x0d,0xa8,0xdf,                       //  vfmadd213ps   %ymm15,%ymm14,%ymm11
    0xc4,0x41,0x7c,0x52,0xc9,                       //  vrsqrtps      %ymm9,%ymm9
    0xc4,0x42,0x15,0xb8,0xd9,                       //  vfmadd231ps   %ymm9,%ymm13,%ymm11
    0xc4,0x41,0x1c,0x5d,0xcb,                       //  vminps        %ymm11,%ymm12,%ymm9
    0xc5,0x3c,0x59,0xc2,                            //  vmulps        %ymm2,%ymm8,%ymm8
    0xc4,0xc1,0x6c,0xc2,0xd2,0x01,                  //  vcmpltps      %ymm10,%ymm2,%ymm2
    0xc4,0xc3,0x35,0x4a,0xd0,0x20,                  //  vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_scale_u8[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x00,                                 //  mov           (%rax),%rax
    0xc4,0x62,0x7d,0x31,0x04,0x38,                  //  vpmovzxbd     (%rax,%rdi,1),%ymm8
    0xc4,0x41,0x7c,0x5b,0xc0,                       //  vcvtdq2ps     %ymm8,%ymm8
    0xc4,0x62,0x7d,0x18,0x4a,0x0c,                  //  vbroadcastss  0xc(%rdx),%ymm9
    0xc4,0x41,0x3c,0x59,0xc1,                       //  vmulps        %ymm9,%ymm8,%ymm8
    0xc5,0xbc,0x59,0xc0,                            //  vmulps        %ymm0,%ymm8,%ymm0
    0xc5,0xbc,0x59,0xc9,                            //  vmulps        %ymm1,%ymm8,%ymm1
    0xc5,0xbc,0x59,0xd2,                            //  vmulps        %ymm2,%ymm8,%ymm2
    0xc5,0xbc,0x59,0xdb,                            //  vmulps        %ymm3,%ymm8,%ymm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_load_tables[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x08,                                 //  mov           (%rax),%rcx
    0x4c,0x8b,0x40,0x08,                            //  mov           0x8(%rax),%r8
    0xc5,0xfc,0x10,0x1c,0xb9,                       //  vmovups       (%rcx,%rdi,4),%ymm3
    0xc4,0xe2,0x7d,0x18,0x52,0x10,                  //  vbroadcastss  0x10(%rdx),%ymm2
    0xc5,0xec,0x54,0xcb,                            //  vandps        %ymm3,%ymm2,%ymm1
    0xc5,0xfc,0x57,0xc0,                            //  vxorps        %ymm0,%ymm0,%ymm0
    0xc5,0x7c,0xc2,0xc0,0x00,                       //  vcmpeqps      %ymm0,%ymm0,%ymm8
    0xc4,0x41,0x7c,0x28,0xc8,                       //  vmovaps       %ymm8,%ymm9
    0xc4,0xc2,0x35,0x92,0x04,0x88,                  //  vgatherdps    %ymm9,(%r8,%ymm1,4),%ymm0
    0x48,0x8b,0x48,0x10,                            //  mov           0x10(%rax),%rcx
    0xc5,0xf5,0x72,0xd3,0x08,                       //  vpsrld        $0x8,%ymm3,%ymm1
    0xc5,0x6c,0x54,0xc9,                            //  vandps        %ymm1,%ymm2,%ymm9
    0xc4,0x41,0x7c,0x28,0xd0,                       //  vmovaps       %ymm8,%ymm10
    0xc4,0xa2,0x2d,0x92,0x0c,0x89,                  //  vgatherdps    %ymm10,(%rcx,%ymm9,4),%ymm1
    0x48,0x8b,0x40,0x18,                            //  mov           0x18(%rax),%rax
    0xc5,0xb5,0x72,0xd3,0x10,                       //  vpsrld        $0x10,%ymm3,%ymm9
    0xc4,0x41,0x6c,0x54,0xc9,                       //  vandps        %ymm9,%ymm2,%ymm9
    0xc4,0xa2,0x3d,0x92,0x14,0x88,                  //  vgatherdps    %ymm8,(%rax,%ymm9,4),%ymm2
    0xc5,0xe5,0x72,0xd3,0x18,                       //  vpsrld        $0x18,%ymm3,%ymm3
    0xc5,0xfc,0x5b,0xdb,                            //  vcvtdq2ps     %ymm3,%ymm3
    0xc4,0x62,0x7d,0x18,0x42,0x0c,                  //  vbroadcastss  0xc(%rdx),%ymm8
    0xc4,0xc1,0x64,0x59,0xd8,                       //  vmulps        %ymm8,%ymm3,%ymm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_load_8888[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x00,                                 //  mov           (%rax),%rax
    0xc5,0xfc,0x10,0x1c,0xb8,                       //  vmovups       (%rax,%rdi,4),%ymm3
    0xc4,0xe2,0x7d,0x18,0x52,0x10,                  //  vbroadcastss  0x10(%rdx),%ymm2
    0xc5,0xec,0x54,0xc3,                            //  vandps        %ymm3,%ymm2,%ymm0
    0xc5,0xfc,0x5b,0xc0,                            //  vcvtdq2ps     %ymm0,%ymm0
    0xc4,0x62,0x7d,0x18,0x42,0x0c,                  //  vbroadcastss  0xc(%rdx),%ymm8
    0xc5,0xbc,0x59,0xc0,                            //  vmulps        %ymm0,%ymm8,%ymm0
    0xc5,0xf5,0x72,0xd3,0x08,                       //  vpsrld        $0x8,%ymm3,%ymm1
    0xc5,0xec,0x54,0xc9,                            //  vandps        %ymm1,%ymm2,%ymm1
    0xc5,0xfc,0x5b,0xc9,                            //  vcvtdq2ps     %ymm1,%ymm1
    0xc5,0xbc,0x59,0xc9,                            //  vmulps        %ymm1,%ymm8,%ymm1
    0xc5,0xb5,0x72,0xd3,0x10,                       //  vpsrld        $0x10,%ymm3,%ymm9
    0xc4,0xc1,0x6c,0x54,0xd1,                       //  vandps        %ymm9,%ymm2,%ymm2
    0xc5,0xfc,0x5b,0xd2,                            //  vcvtdq2ps     %ymm2,%ymm2
    0xc5,0xbc,0x59,0xd2,                            //  vmulps        %ymm2,%ymm8,%ymm2
    0xc5,0xe5,0x72,0xd3,0x18,                       //  vpsrld        $0x18,%ymm3,%ymm3
    0xc5,0xfc,0x5b,0xdb,                            //  vcvtdq2ps     %ymm3,%ymm3
    0xc4,0xc1,0x64,0x59,0xd8,                       //  vmulps        %ymm8,%ymm3,%ymm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_store_8888[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x00,                                 //  mov           (%rax),%rax
    0xc4,0x62,0x7d,0x18,0x42,0x08,                  //  vbroadcastss  0x8(%rdx),%ymm8
    0xc5,0x3c,0x59,0xc8,                            //  vmulps        %ymm0,%ymm8,%ymm9
    0xc4,0x41,0x7d,0x5b,0xc9,                       //  vcvtps2dq     %ymm9,%ymm9
    0xc5,0x3c,0x59,0xd1,                            //  vmulps        %ymm1,%ymm8,%ymm10
    0xc4,0x41,0x7d,0x5b,0xd2,                       //  vcvtps2dq     %ymm10,%ymm10
    0xc4,0xc1,0x2d,0x72,0xf2,0x08,                  //  vpslld        $0x8,%ymm10,%ymm10
    0xc4,0x41,0x2d,0xeb,0xc9,                       //  vpor          %ymm9,%ymm10,%ymm9
    0xc5,0x3c,0x59,0xd2,                            //  vmulps        %ymm2,%ymm8,%ymm10
    0xc4,0x41,0x7d,0x5b,0xd2,                       //  vcvtps2dq     %ymm10,%ymm10
    0xc4,0xc1,0x2d,0x72,0xf2,0x10,                  //  vpslld        $0x10,%ymm10,%ymm10
    0xc5,0x3c,0x59,0xc3,                            //  vmulps        %ymm3,%ymm8,%ymm8
    0xc4,0x41,0x7d,0x5b,0xc0,                       //  vcvtps2dq     %ymm8,%ymm8
    0xc4,0xc1,0x3d,0x72,0xf0,0x18,                  //  vpslld        $0x18,%ymm8,%ymm8
    0xc4,0x41,0x2d,0xeb,0xc0,                       //  vpor          %ymm8,%ymm10,%ymm8
    0xc4,0x41,0x35,0xeb,0xc0,                       //  vpor          %ymm8,%ymm9,%ymm8
    0xc5,0x7e,0x7f,0x04,0xb8,                       //  vmovdqu       %ymm8,(%rax,%rdi,4)
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_load_f16[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x00,                                 //  mov           (%rax),%rax
    0xc5,0xfa,0x6f,0x04,0xf8,                       //  vmovdqu       (%rax,%rdi,8),%xmm0
    0xc5,0xfa,0x6f,0x4c,0xf8,0x10,                  //  vmovdqu       0x10(%rax,%rdi,8),%xmm1
    0xc5,0xfa,0x6f,0x54,0xf8,0x20,                  //  vmovdqu       0x20(%rax,%rdi,8),%xmm2
    0xc5,0xfa,0x6f,0x5c,0xf8,0x30,                  //  vmovdqu       0x30(%rax,%rdi,8),%xmm3
    0xc5,0x79,0x61,0xc1,                            //  vpunpcklwd    %xmm1,%xmm0,%xmm8
    0xc5,0xf9,0x69,0xc1,                            //  vpunpckhwd    %xmm1,%xmm0,%xmm0
    0xc5,0xe9,0x61,0xcb,                            //  vpunpcklwd    %xmm3,%xmm2,%xmm1
    0xc5,0xe9,0x69,0xd3,                            //  vpunpckhwd    %xmm3,%xmm2,%xmm2
    0xc5,0x39,0x61,0xc8,                            //  vpunpcklwd    %xmm0,%xmm8,%xmm9
    0xc5,0x39,0x69,0xc0,                            //  vpunpckhwd    %xmm0,%xmm8,%xmm8
    0xc5,0xf1,0x61,0xda,                            //  vpunpcklwd    %xmm2,%xmm1,%xmm3
    0xc5,0x71,0x69,0xd2,                            //  vpunpckhwd    %xmm2,%xmm1,%xmm10
    0xc5,0xb1,0x6c,0xc3,                            //  vpunpcklqdq   %xmm3,%xmm9,%xmm0
    0xc4,0xe2,0x7d,0x13,0xc0,                       //  vcvtph2ps     %xmm0,%ymm0
    0xc5,0xb1,0x6d,0xcb,                            //  vpunpckhqdq   %xmm3,%xmm9,%xmm1
    0xc4,0xe2,0x7d,0x13,0xc9,                       //  vcvtph2ps     %xmm1,%ymm1
    0xc4,0xc1,0x39,0x6c,0xd2,                       //  vpunpcklqdq   %xmm10,%xmm8,%xmm2
    0xc4,0xe2,0x7d,0x13,0xd2,                       //  vcvtph2ps     %xmm2,%ymm2
    0xc4,0xc1,0x39,0x6d,0xda,                       //  vpunpckhqdq   %xmm10,%xmm8,%xmm3
    0xc4,0xe2,0x7d,0x13,0xdb,                       //  vcvtph2ps     %xmm3,%ymm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_store_f16[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x00,                                 //  mov           (%rax),%rax
    0xc4,0xc3,0x7d,0x1d,0xc0,0x04,                  //  vcvtps2ph     $0x4,%ymm0,%xmm8
    0xc4,0xc3,0x7d,0x1d,0xc9,0x04,                  //  vcvtps2ph     $0x4,%ymm1,%xmm9
    0xc4,0xc3,0x7d,0x1d,0xd2,0x04,                  //  vcvtps2ph     $0x4,%ymm2,%xmm10
    0xc4,0xc3,0x7d,0x1d,0xdb,0x04,                  //  vcvtps2ph     $0x4,%ymm3,%xmm11
    0xc4,0x41,0x39,0x61,0xe1,                       //  vpunpcklwd    %xmm9,%xmm8,%xmm12
    0xc4,0x41,0x39,0x69,0xc1,                       //  vpunpckhwd    %xmm9,%xmm8,%xmm8
    0xc4,0x41,0x29,0x61,0xcb,                       //  vpunpcklwd    %xmm11,%xmm10,%xmm9
    0xc4,0x41,0x29,0x69,0xd3,                       //  vpunpckhwd    %xmm11,%xmm10,%xmm10
    0xc4,0x41,0x19,0x62,0xd9,                       //  vpunpckldq    %xmm9,%xmm12,%xmm11
    0xc5,0x7a,0x7f,0x1c,0xf8,                       //  vmovdqu       %xmm11,(%rax,%rdi,8)
    0xc4,0x41,0x19,0x6a,0xc9,                       //  vpunpckhdq    %xmm9,%xmm12,%xmm9
    0xc5,0x7a,0x7f,0x4c,0xf8,0x10,                  //  vmovdqu       %xmm9,0x10(%rax,%rdi,8)
    0xc4,0x41,0x39,0x62,0xca,                       //  vpunpckldq    %xmm10,%xmm8,%xmm9
    0xc5,0x7a,0x7f,0x4c,0xf8,0x20,                  //  vmovdqu       %xmm9,0x20(%rax,%rdi,8)
    0xc4,0x41,0x39,0x6a,0xc2,                       //  vpunpckhdq    %xmm10,%xmm8,%xmm8
    0xc5,0x7a,0x7f,0x44,0xf8,0x30,                  //  vmovdqu       %xmm8,0x30(%rax,%rdi,8)
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_clamp_x[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc4,0x62,0x7d,0x58,0x00,                       //  vpbroadcastd  (%rax),%ymm8
    0xc4,0x41,0x35,0x76,0xc9,                       //  vpcmpeqd      %ymm9,%ymm9,%ymm9
    0xc4,0x41,0x3d,0xfe,0xc1,                       //  vpaddd        %ymm9,%ymm8,%ymm8
    0xc4,0xc1,0x7c,0x5d,0xc0,                       //  vminps        %ymm8,%ymm0,%ymm0
    0xc4,0x41,0x3c,0x57,0xc0,                       //  vxorps        %ymm8,%ymm8,%ymm8
    0xc5,0xbc,0x5f,0xc0,                            //  vmaxps        %ymm0,%ymm8,%ymm0
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_clamp_y[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc4,0x62,0x7d,0x58,0x00,                       //  vpbroadcastd  (%rax),%ymm8
    0xc4,0x41,0x35,0x76,0xc9,                       //  vpcmpeqd      %ymm9,%ymm9,%ymm9
    0xc4,0x41,0x3d,0xfe,0xc1,                       //  vpaddd        %ymm9,%ymm8,%ymm8
    0xc4,0xc1,0x74,0x5d,0xc8,                       //  vminps        %ymm8,%ymm1,%ymm1
    0xc4,0x41,0x3c,0x57,0xc0,                       //  vxorps        %ymm8,%ymm8,%ymm8
    0xc5,0xbc,0x5f,0xc9,                            //  vmaxps        %ymm1,%ymm8,%ymm1
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_matrix_2x3[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc4,0x62,0x7d,0x18,0x08,                       //  vbroadcastss  (%rax),%ymm9
    0xc4,0x62,0x7d,0x18,0x50,0x08,                  //  vbroadcastss  0x8(%rax),%ymm10
    0xc4,0x62,0x7d,0x18,0x40,0x10,                  //  vbroadcastss  0x10(%rax),%ymm8
    0xc4,0x42,0x75,0xb8,0xc2,                       //  vfmadd231ps   %ymm10,%ymm1,%ymm8
    0xc4,0x42,0x7d,0xb8,0xc1,                       //  vfmadd231ps   %ymm9,%ymm0,%ymm8
    0xc4,0x62,0x7d,0x18,0x50,0x04,                  //  vbroadcastss  0x4(%rax),%ymm10
    0xc4,0x62,0x7d,0x18,0x58,0x0c,                  //  vbroadcastss  0xc(%rax),%ymm11
    0xc4,0x62,0x7d,0x18,0x48,0x14,                  //  vbroadcastss  0x14(%rax),%ymm9
    0xc4,0x42,0x75,0xb8,0xcb,                       //  vfmadd231ps   %ymm11,%ymm1,%ymm9
    0xc4,0x42,0x7d,0xb8,0xca,                       //  vfmadd231ps   %ymm10,%ymm0,%ymm9
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc5,0x7c,0x29,0xc0,                            //  vmovaps       %ymm8,%ymm0
    0xc5,0x7c,0x29,0xc9,                            //  vmovaps       %ymm9,%ymm1
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_matrix_3x4[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc4,0x62,0x7d,0x18,0x08,                       //  vbroadcastss  (%rax),%ymm9
    0xc4,0x62,0x7d,0x18,0x50,0x0c,                  //  vbroadcastss  0xc(%rax),%ymm10
    0xc4,0x62,0x7d,0x18,0x58,0x18,                  //  vbroadcastss  0x18(%rax),%ymm11
    0xc4,0x62,0x7d,0x18,0x40,0x24,                  //  vbroadcastss  0x24(%rax),%ymm8
    0xc4,0x42,0x6d,0xb8,0xc3,                       //  vfmadd231ps   %ymm11,%ymm2,%ymm8
    0xc4,0x42,0x75,0xb8,0xc2,                       //  vfmadd231ps   %ymm10,%ymm1,%ymm8
    0xc4,0x42,0x7d,0xb8,0xc1,                       //  vfmadd231ps   %ymm9,%ymm0,%ymm8
    0xc4,0x62,0x7d,0x18,0x50,0x04,                  //  vbroadcastss  0x4(%rax),%ymm10
    0xc4,0x62,0x7d,0x18,0x58,0x10,                  //  vbroadcastss  0x10(%rax),%ymm11
    0xc4,0x62,0x7d,0x18,0x60,0x1c,                  //  vbroadcastss  0x1c(%rax),%ymm12
    0xc4,0x62,0x7d,0x18,0x48,0x28,                  //  vbroadcastss  0x28(%rax),%ymm9
    0xc4,0x42,0x6d,0xb8,0xcc,                       //  vfmadd231ps   %ymm12,%ymm2,%ymm9
    0xc4,0x42,0x75,0xb8,0xcb,                       //  vfmadd231ps   %ymm11,%ymm1,%ymm9
    0xc4,0x42,0x7d,0xb8,0xca,                       //  vfmadd231ps   %ymm10,%ymm0,%ymm9
    0xc4,0x62,0x7d,0x18,0x58,0x08,                  //  vbroadcastss  0x8(%rax),%ymm11
    0xc4,0x62,0x7d,0x18,0x60,0x14,                  //  vbroadcastss  0x14(%rax),%ymm12
    0xc4,0x62,0x7d,0x18,0x68,0x20,                  //  vbroadcastss  0x20(%rax),%ymm13
    0xc4,0x62,0x7d,0x18,0x50,0x2c,                  //  vbroadcastss  0x2c(%rax),%ymm10
    0xc4,0x42,0x6d,0xb8,0xd5,                       //  vfmadd231ps   %ymm13,%ymm2,%ymm10
    0xc4,0x42,0x75,0xb8,0xd4,                       //  vfmadd231ps   %ymm12,%ymm1,%ymm10
    0xc4,0x42,0x7d,0xb8,0xd3,                       //  vfmadd231ps   %ymm11,%ymm0,%ymm10
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc5,0x7c,0x29,0xc0,                            //  vmovaps       %ymm8,%ymm0
    0xc5,0x7c,0x29,0xc9,                            //  vmovaps       %ymm9,%ymm1
    0xc5,0x7c,0x29,0xd2,                            //  vmovaps       %ymm10,%ymm2
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char hsw_sk_linear_gradient_2stops[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc4,0xe2,0x7d,0x18,0x48,0x10,                  //  vbroadcastss  0x10(%rax),%ymm1
    0xc4,0x62,0x7d,0x18,0x00,                       //  vbroadcastss  (%rax),%ymm8
    0xc4,0x62,0x7d,0xb8,0xc1,                       //  vfmadd231ps   %ymm1,%ymm0,%ymm8
    0xc4,0xe2,0x7d,0x18,0x50,0x14,                  //  vbroadcastss  0x14(%rax),%ymm2
    0xc4,0xe2,0x7d,0x18,0x48,0x04,                  //  vbroadcastss  0x4(%rax),%ymm1
    0xc4,0xe2,0x7d,0xb8,0xca,                       //  vfmadd231ps   %ymm2,%ymm0,%ymm1
    0xc4,0xe2,0x7d,0x18,0x58,0x18,                  //  vbroadcastss  0x18(%rax),%ymm3
    0xc4,0xe2,0x7d,0x18,0x50,0x08,                  //  vbroadcastss  0x8(%rax),%ymm2
    0xc4,0xe2,0x7d,0xb8,0xd3,                       //  vfmadd231ps   %ymm3,%ymm0,%ymm2
    0xc4,0x62,0x7d,0x18,0x48,0x1c,                  //  vbroadcastss  0x1c(%rax),%ymm9
    0xc4,0xe2,0x7d,0x18,0x58,0x0c,                  //  vbroadcastss  0xc(%rax),%ymm3
    0xc4,0xc2,0x7d,0xb8,0xd9,                       //  vfmadd231ps   %ymm9,%ymm0,%ymm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xc5,0x7c,0x29,0xc0,                            //  vmovaps       %ymm8,%ymm0
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_start_pipeline[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x57,0xc0,                                 //  xorps         %xmm0,%xmm0
    0x0f,0x57,0xc9,                                 //  xorps         %xmm1,%xmm1
    0x0f,0x57,0xd2,                                 //  xorps         %xmm2,%xmm2
    0x0f,0x57,0xdb,                                 //  xorps         %xmm3,%xmm3
    0x0f,0x57,0xe4,                                 //  xorps         %xmm4,%xmm4
    0x0f,0x57,0xed,                                 //  xorps         %xmm5,%xmm5
    0x0f,0x57,0xf6,                                 //  xorps         %xmm6,%xmm6
    0x0f,0x57,0xff,                                 //  xorps         %xmm7,%xmm7
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_start_pipeline_ms[] = {
    0x56,                                           //  push          %rsi
    0x57,                                           //  push          %rdi
    0x48,0x81,0xec,0xa8,0x00,0x00,0x00,             //  sub           $0xa8,%rsp
    0x44,0x0f,0x29,0xbc,0x24,0x90,0x00,0x00,0x00,   //  movaps        %xmm15,0x90(%rsp)
    0x44,0x0f,0x29,0xb4,0x24,0x80,0x00,0x00,0x00,   //  movaps        %xmm14,0x80(%rsp)
    0x44,0x0f,0x29,0x6c,0x24,0x70,                  //  movaps        %xmm13,0x70(%rsp)
    0x44,0x0f,0x29,0x64,0x24,0x60,                  //  movaps        %xmm12,0x60(%rsp)
    0x44,0x0f,0x29,0x5c,0x24,0x50,                  //  movaps        %xmm11,0x50(%rsp)
    0x44,0x0f,0x29,0x54,0x24,0x40,                  //  movaps        %xmm10,0x40(%rsp)
    0x44,0x0f,0x29,0x4c,0x24,0x30,                  //  movaps        %xmm9,0x30(%rsp)
    0x44,0x0f,0x29,0x44,0x24,0x20,                  //  movaps        %xmm8,0x20(%rsp)
    0x0f,0x29,0x7c,0x24,0x10,                       //  movaps        %xmm7,0x10(%rsp)
    0x0f,0x29,0x34,0x24,                            //  movaps        %xmm6,(%rsp)
    0x48,0x89,0xd6,                                 //  mov           %rdx,%rsi
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x57,0xc0,                                 //  xorps         %xmm0,%xmm0
    0x0f,0x57,0xc9,                                 //  xorps         %xmm1,%xmm1
    0x0f,0x57,0xd2,                                 //  xorps         %xmm2,%xmm2
    0x0f,0x57,0xdb,                                 //  xorps         %xmm3,%xmm3
    0x0f,0x57,0xe4,                                 //  xorps         %xmm4,%xmm4
    0x0f,0x57,0xed,                                 //  xorps         %xmm5,%xmm5
    0x0f,0x57,0xf6,                                 //  xorps         %xmm6,%xmm6
    0x0f,0x57,0xff,                                 //  xorps         %xmm7,%xmm7
    0x48,0x89,0xcf,                                 //  mov           %rcx,%rdi
    0x4c,0x89,0xc2,                                 //  mov           %r8,%rdx
    0xff,0xd0,                                      //  callq         *%rax
    0x0f,0x28,0x34,0x24,                            //  movaps        (%rsp),%xmm6
    0x0f,0x28,0x7c,0x24,0x10,                       //  movaps        0x10(%rsp),%xmm7
    0x44,0x0f,0x28,0x44,0x24,0x20,                  //  movaps        0x20(%rsp),%xmm8
    0x44,0x0f,0x28,0x4c,0x24,0x30,                  //  movaps        0x30(%rsp),%xmm9
    0x44,0x0f,0x28,0x54,0x24,0x40,                  //  movaps        0x40(%rsp),%xmm10
    0x44,0x0f,0x28,0x5c,0x24,0x50,                  //  movaps        0x50(%rsp),%xmm11
    0x44,0x0f,0x28,0x64,0x24,0x60,                  //  movaps        0x60(%rsp),%xmm12
    0x44,0x0f,0x28,0x6c,0x24,0x70,                  //  movaps        0x70(%rsp),%xmm13
    0x44,0x0f,0x28,0xb4,0x24,0x80,0x00,0x00,0x00,   //  movaps        0x80(%rsp),%xmm14
    0x44,0x0f,0x28,0xbc,0x24,0x90,0x00,0x00,0x00,   //  movaps        0x90(%rsp),%xmm15
    0x48,0x81,0xc4,0xa8,0x00,0x00,0x00,             //  add           $0xa8,%rsp
    0x5f,                                           //  pop           %rdi
    0x5e,                                           //  pop           %rsi
    0xc3,                                           //  retq
};
static const unsigned char sse41_sk_just_return[] = {
    0xc3,                                           //  retq
};
static const unsigned char sse41_sk_seed_shader[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x66,0x0f,0x6e,0xc7,                            //  movd          %edi,%xmm0
    0x66,0x0f,0x70,0xc0,0x00,                       //  pshufd        $0x0,%xmm0,%xmm0
    0x0f,0x5b,0xc8,                                 //  cvtdq2ps      %xmm0,%xmm1
    0xf3,0x0f,0x10,0x12,                            //  movss         (%rdx),%xmm2
    0xf3,0x0f,0x10,0x5a,0x04,                       //  movss         0x4(%rdx),%xmm3
    0x0f,0xc6,0xdb,0x00,                            //  shufps        $0x0,%xmm3,%xmm3
    0x0f,0x58,0xcb,                                 //  addps         %xmm3,%xmm1
    0x0f,0x10,0x42,0x14,                            //  movups        0x14(%rdx),%xmm0
    0x0f,0x58,0xc1,                                 //  addps         %xmm1,%xmm0
    0x66,0x0f,0x6e,0x08,                            //  movd          (%rax),%xmm1
    0x66,0x0f,0x70,0xc9,0x00,                       //  pshufd        $0x0,%xmm1,%xmm1
    0x0f,0x5b,0xc9,                                 //  cvtdq2ps      %xmm1,%xmm1
    0x0f,0x58,0xcb,                                 //  addps         %xmm3,%xmm1
    0x0f,0xc6,0xd2,0x00,                            //  shufps        $0x0,%xmm2,%xmm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x57,0xdb,                                 //  xorps         %xmm3,%xmm3
    0x0f,0x57,0xe4,                                 //  xorps         %xmm4,%xmm4
    0x0f,0x57,0xed,                                 //  xorps         %xmm5,%xmm5
    0x0f,0x57,0xf6,                                 //  xorps         %xmm6,%xmm6
    0x0f,0x57,0xff,                                 //  xorps         %xmm7,%xmm7
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_constant_color[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x10,0x18,                                 //  movups        (%rax),%xmm3
    0x0f,0x28,0xc3,                                 //  movaps        %xmm3,%xmm0
    0x0f,0xc6,0xc0,0x00,                            //  shufps        $0x0,%xmm0,%xmm0
    0x0f,0x28,0xcb,                                 //  movaps        %xmm3,%xmm1
    0x0f,0xc6,0xc9,0x55,                            //  shufps        $0x55,%xmm1,%xmm1
    0x0f,0x28,0xd3,                                 //  movaps        %xmm3,%xmm2
    0x0f,0xc6,0xd2,0xaa,                            //  shufps        $0xaa,%xmm2,%xmm2
    0x0f,0xc6,0xdb,0xff,                            //  shufps        $0xff,%xmm3,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_clear[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x57,0xc0,                                 //  xorps         %xmm0,%xmm0
    0x0f,0x57,0xc9,                                 //  xorps         %xmm1,%xmm1
    0x0f,0x57,0xd2,                                 //  xorps         %xmm2,%xmm2
    0x0f,0x57,0xdb,                                 //  xorps         %xmm3,%xmm3
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_plus_[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x58,0xc4,                                 //  addps         %xmm4,%xmm0
    0x0f,0x58,0xcd,                                 //  addps         %xmm5,%xmm1
    0x0f,0x58,0xd6,                                 //  addps         %xmm6,%xmm2
    0x0f,0x58,0xdf,                                 //  addps         %xmm7,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_srcover[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x44,0x0f,0x10,0x02,                       //  movss         (%rdx),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x44,0x0f,0x5c,0xc3,                            //  subps         %xmm3,%xmm8
    0x45,0x0f,0x28,0xc8,                            //  movaps        %xmm8,%xmm9
    0x44,0x0f,0x59,0xcc,                            //  mulps         %xmm4,%xmm9
    0x41,0x0f,0x58,0xc1,                            //  addps         %xmm9,%xmm0
    0x45,0x0f,0x28,0xc8,                            //  movaps        %xmm8,%xmm9
    0x44,0x0f,0x59,0xcd,                            //  mulps         %xmm5,%xmm9
    0x41,0x0f,0x58,0xc9,                            //  addps         %xmm9,%xmm1
    0x45,0x0f,0x28,0xc8,                            //  movaps        %xmm8,%xmm9
    0x44,0x0f,0x59,0xce,                            //  mulps         %xmm6,%xmm9
    0x41,0x0f,0x58,0xd1,                            //  addps         %xmm9,%xmm2
    0x44,0x0f,0x59,0xc7,                            //  mulps         %xmm7,%xmm8
    0x41,0x0f,0x58,0xd8,                            //  addps         %xmm8,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_dstover[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x44,0x0f,0x10,0x02,                       //  movss         (%rdx),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x44,0x0f,0x5c,0xc7,                            //  subps         %xmm7,%xmm8
    0x41,0x0f,0x59,0xc0,                            //  mulps         %xmm8,%xmm0
    0x0f,0x58,0xc4,                                 //  addps         %xmm4,%xmm0
    0x41,0x0f,0x59,0xc8,                            //  mulps         %xmm8,%xmm1
    0x0f,0x58,0xcd,                                 //  addps         %xmm5,%xmm1
    0x41,0x0f,0x59,0xd0,                            //  mulps         %xmm8,%xmm2
    0x0f,0x58,0xd6,                                 //  addps         %xmm6,%xmm2
    0x41,0x0f,0x59,0xd8,                            //  mulps         %xmm8,%xmm3
    0x0f,0x58,0xdf,                                 //  addps         %xmm7,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_clamp_0[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x45,0x0f,0x57,0xc0,                            //  xorps         %xmm8,%xmm8
    0x41,0x0f,0x5f,0xc0,                            //  maxps         %xmm8,%xmm0
    0x41,0x0f,0x5f,0xc8,                            //  maxps         %xmm8,%xmm1
    0x41,0x0f,0x5f,0xd0,                            //  maxps         %xmm8,%xmm2
    0x41,0x0f,0x5f,0xd8,                            //  maxps         %xmm8,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_clamp_1[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x44,0x0f,0x10,0x02,                       //  movss         (%rdx),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x41,0x0f,0x5d,0xc0,                            //  minps         %xmm8,%xmm0
    0x41,0x0f,0x5d,0xc8,                            //  minps         %xmm8,%xmm1
    0x41,0x0f,0x5d,0xd0,                            //  minps         %xmm8,%xmm2
    0x41,0x0f,0x5d,0xd8,                            //  minps         %xmm8,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_clamp_a[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x44,0x0f,0x10,0x02,                       //  movss         (%rdx),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x41,0x0f,0x5d,0xd8,                            //  minps         %xmm8,%xmm3
    0x0f,0x5d,0xc3,                                 //  minps         %xmm3,%xmm0
    0x0f,0x5d,0xcb,                                 //  minps         %xmm3,%xmm1
    0x0f,0x5d,0xd3,                                 //  minps         %xmm3,%xmm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_swap[] = {
    0x44,0x0f,0x28,0xc3,                            //  movaps        %xmm3,%xmm8
    0x44,0x0f,0x28,0xca,                            //  movaps        %xmm2,%xmm9
    0x44,0x0f,0x28,0xd1,                            //  movaps        %xmm1,%xmm10
    0x44,0x0f,0x28,0xd8,                            //  movaps        %xmm0,%xmm11
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x28,0xc4,                                 //  movaps        %xmm4,%xmm0
    0x0f,0x28,0xcd,                                 //  movaps        %xmm5,%xmm1
    0x0f,0x28,0xd6,                                 //  movaps        %xmm6,%xmm2
    0x0f,0x28,0xdf,                                 //  movaps        %xmm7,%xmm3
    0x41,0x0f,0x28,0xe3,                            //  movaps        %xmm11,%xmm4
    0x41,0x0f,0x28,0xea,                            //  movaps        %xmm10,%xmm5
    0x41,0x0f,0x28,0xf1,                            //  movaps        %xmm9,%xmm6
    0x41,0x0f,0x28,0xf8,                            //  movaps        %xmm8,%xmm7
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_move_src_dst[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x28,0xe0,                                 //  movaps        %xmm0,%xmm4
    0x0f,0x28,0xe9,                                 //  movaps        %xmm1,%xmm5
    0x0f,0x28,0xf2,                                 //  movaps        %xmm2,%xmm6
    0x0f,0x28,0xfb,                                 //  movaps        %xmm3,%xmm7
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_move_dst_src[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x28,0xc4,                                 //  movaps        %xmm4,%xmm0
    0x0f,0x28,0xcd,                                 //  movaps        %xmm5,%xmm1
    0x0f,0x28,0xd6,                                 //  movaps        %xmm6,%xmm2
    0x0f,0x28,0xdf,                                 //  movaps        %xmm7,%xmm3
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_premul[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x59,0xc3,                                 //  mulps         %xmm3,%xmm0
    0x0f,0x59,0xcb,                                 //  mulps         %xmm3,%xmm1
    0x0f,0x59,0xd3,                                 //  mulps         %xmm3,%xmm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_unpremul[] = {
    0x44,0x0f,0x28,0xc0,                            //  movaps        %xmm0,%xmm8
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x45,0x0f,0x57,0xc9,                            //  xorps         %xmm9,%xmm9
    0xf3,0x44,0x0f,0x10,0x12,                       //  movss         (%rdx),%xmm10
    0x45,0x0f,0xc6,0xd2,0x00,                       //  shufps        $0x0,%xmm10,%xmm10
    0x44,0x0f,0x5e,0xd3,                            //  divps         %xmm3,%xmm10
    0x0f,0x28,0xc3,                                 //  movaps        %xmm3,%xmm0
    0x41,0x0f,0xc2,0xc1,0x00,                       //  cmpeqps       %xmm9,%xmm0
    0x66,0x45,0x0f,0x38,0x14,0xd1,                  //  blendvps      %xmm0,%xmm9,%xmm10
    0x45,0x0f,0x59,0xc2,                            //  mulps         %xmm10,%xmm8
    0x41,0x0f,0x59,0xca,                            //  mulps         %xmm10,%xmm1
    0x41,0x0f,0x59,0xd2,                            //  mulps         %xmm10,%xmm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x41,0x0f,0x28,0xc0,                            //  movaps        %xmm8,%xmm0
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_from_srgb[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x44,0x0f,0x10,0x5a,0x40,                  //  movss         0x40(%rdx),%xmm11
    0x45,0x0f,0xc6,0xdb,0x00,                       //  shufps        $0x0,%xmm11,%xmm11
    0x45,0x0f,0x28,0xd3,                            //  movaps        %xmm11,%xmm10
    0x44,0x0f,0x59,0xd0,                            //  mulps         %xmm0,%xmm10
    0x44,0x0f,0x28,0xf0,                            //  movaps        %xmm0,%xmm14
    0x45,0x0f,0x59,0xf6,                            //  mulps         %xmm14,%xmm14
    0xf3,0x44,0x0f,0x10,0x42,0x3c,                  //  movss         0x3c(%rdx),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0xf3,0x44,0x0f,0x10,0x62,0x34,                  //  movss         0x34(%rdx),%xmm12
    0xf3,0x44,0x0f,0x10,0x6a,0x38,                  //  movss         0x38(%rdx),%xmm13
    0x45,0x0f,0xc6,0xed,0x00,                       //  shufps        $0x0,%xmm13,%xmm13
    0x45,0x0f,0x28,0xc8,                            //  movaps        %xmm8,%xmm9
    0x44,0x0f,0x59,0xc8,                            //  mulps         %xmm0,%xmm9
    0x45,0x0f,0x58,0xcd,                            //  addps         %xmm13,%xmm9
    0x45,0x0f,0xc6,0xe4,0x00,                       //  shufps        $0x0,%xmm12,%xmm12
    0x45,0x0f,0x59,0xce,                            //  mulps         %xmm14,%xmm9
    0x45,0x0f,0x58,0xcc,                            //  addps         %xmm12,%xmm9
    0xf3,0x44,0x0f,0x10,0x72,0x44,                  //  movss         0x44(%rdx),%xmm14
    0x45,0x0f,0xc6,0xf6,0x00,                       //  shufps        $0x0,%xmm14,%xmm14
    0x41,0x0f,0xc2,0xc6,0x01,                       //  cmpltps       %xmm14,%xmm0
    0x66,0x45,0x0f,0x38,0x14,0xca,                  //  blendvps      %xmm0,%xmm10,%xmm9
    0x45,0x0f,0x28,0xfb,                            //  movaps        %xmm11,%xmm15
    0x44,0x0f,0x59,0xf9,                            //  mulps         %xmm1,%xmm15
    0x0f,0x28,0xc1,                                 //  movaps        %xmm1,%xmm0
    0x0f,0x59,0xc0,                                 //  mulps         %xmm0,%xmm0
    0x45,0x0f,0x28,0xd0,                            //  movaps        %xmm8,%xmm10
    0x44,0x0f,0x59,0xd1,                            //  mulps         %xmm1,%xmm10
    0x45,0x0f,0x58,0xd5,                            //  addps         %xmm13,%xmm10
    0x44,0x0f,0x59,0xd0,                            //  mulps         %xmm0,%xmm10
    0x45,0x0f,0x58,0xd4,                            //  addps         %xmm12,%xmm10
    0x41,0x0f,0xc2,0xce,0x01,                       //  cmpltps       %xmm14,%xmm1
    0x0f,0x28,0xc1,                                 //  movaps        %xmm1,%xmm0
    0x66,0x45,0x0f,0x38,0x14,0xd7,                  //  blendvps      %xmm0,%xmm15,%xmm10
    0x44,0x0f,0x59,0xda,                            //  mulps         %xmm2,%xmm11
    0x0f,0x28,0xc2,                                 //  movaps        %xmm2,%xmm0
    0x0f,0x59,0xc0,                                 //  mulps         %xmm0,%xmm0
    0x44,0x0f,0x59,0xc2,                            //  mulps         %xmm2,%xmm8
    0x45,0x0f,0x58,0xc5,                            //  addps         %xmm13,%xmm8
    0x44,0x0f,0x59,0xc0,                            //  mulps         %xmm0,%xmm8
    0x45,0x0f,0x58,0xc4,                            //  addps         %xmm12,%xmm8
    0x41,0x0f,0xc2,0xd6,0x01,                       //  cmpltps       %xmm14,%xmm2
    0x0f,0x28,0xc2,                                 //  movaps        %xmm2,%xmm0
    0x66,0x45,0x0f,0x38,0x14,0xc3,                  //  blendvps      %xmm0,%xmm11,%xmm8
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x41,0x0f,0x28,0xc1,                            //  movaps        %xmm9,%xmm0
    0x41,0x0f,0x28,0xca,                            //  movaps        %xmm10,%xmm1
    0x41,0x0f,0x28,0xd0,                            //  movaps        %xmm8,%xmm2
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_to_srgb[] = {
    0x48,0x83,0xec,0x18,                            //  sub           $0x18,%rsp
    0x0f,0x29,0x3c,0x24,                            //  movaps        %xmm7,(%rsp)
    0x0f,0x28,0xfe,                                 //  movaps        %xmm6,%xmm7
    0x0f,0x28,0xf5,                                 //  movaps        %xmm5,%xmm6
    0x0f,0x28,0xec,                                 //  movaps        %xmm4,%xmm5
    0x0f,0x28,0xe3,                                 //  movaps        %xmm3,%xmm4
    0x44,0x0f,0x28,0xc2,                            //  movaps        %xmm2,%xmm8
    0x0f,0x28,0xd9,                                 //  movaps        %xmm1,%xmm3
    0x0f,0x52,0xd0,                                 //  rsqrtps       %xmm0,%xmm2
    0x44,0x0f,0x53,0xca,                            //  rcpps         %xmm2,%xmm9
    0x44,0x0f,0x52,0xd2,                            //  rsqrtps       %xmm2,%xmm10
    0xf3,0x0f,0x10,0x12,                            //  movss         (%rdx),%xmm2
    0xf3,0x44,0x0f,0x10,0x5a,0x48,                  //  movss         0x48(%rdx),%xmm11
    0x45,0x0f,0xc6,0xdb,0x00,                       //  shufps        $0x0,%xmm11,%xmm11
    0x41,0x0f,0x28,0xcb,                            //  movaps        %xmm11,%xmm1
    0x0f,0x59,0xc8,                                 //  mulps         %xmm0,%xmm1
    0x0f,0xc6,0xd2,0x00,                            //  shufps        $0x0,%xmm2,%xmm2
    0xf3,0x44,0x0f,0x10,0x62,0x4c,                  //  movss         0x4c(%rdx),%xmm12
    0x45,0x0f,0xc6,0xe4,0x00,                       //  shufps        $0x0,%xmm12,%xmm12
    0xf3,0x44,0x0f,0x10,0x6a,0x50,                  //  movss         0x50(%rdx),%xmm13
    0x45,0x0f,0xc6,0xed,0x00,                       //  shufps        $0x0,%xmm13,%xmm13
    0xf3,0x44,0x0f,0x10,0x72,0x54,                  //  movss         0x54(%rdx),%xmm14
    0x45,0x0f,0xc6,0xf6,0x00,                       //  shufps        $0x0,%xmm14,%xmm14
    0x45,0x0f,0x59,0xcd,                            //  mulps         %xmm13,%xmm9
    0x45,0x0f,0x58,0xce,                            //  addps         %xmm14,%xmm9
    0x45,0x0f,0x59,0xd4,                            //  mulps         %xmm12,%xmm10
    0x45,0x0f,0x58,0xd1,                            //  addps         %xmm9,%xmm10
    0x44,0x0f,0x28,0xca,                            //  movaps        %xmm2,%xmm9
    0x45,0x0f,0x5d,0xca,                            //  minps         %xmm10,%xmm9
    0xf3,0x44,0x0f,0x10,0x7a,0x58,                  //  movss         0x58(%rdx),%xmm15
    0x45,0x0f,0xc6,0xff,0x00,                       //  shufps        $0x0,%xmm15,%xmm15
    0x41,0x0f,0xc2,0xc7,0x01,                       //  cmpltps       %xmm15,%xmm0
    0x66,0x44,0x0f,0x38,0x14,0xc9,                  //  blendvps      %xmm0,%xmm1,%xmm9
    0x0f,0x52,0xc3,                                 //  rsqrtps       %xmm3,%xmm0
    0x0f,0x53,0xc8,                                 //  rcpps         %xmm0,%xmm1
    0x0f,0x52,0xc0,                                 //  rsqrtps       %xmm0,%xmm0
    0x41,0x0f,0x59,0xcd,                            //  mulps         %xmm13,%xmm1
    0x41,0x0f,0x58,0xce,                            //  addps         %xmm14,%xmm1
    0x41,0x0f,0x59,0xc4,                            //  mulps         %xmm12,%xmm0
    0x0f,0x58,0xc1,                                 //  addps         %xmm1,%xmm0
    0x44,0x0f,0x28,0xd2,                            //  movaps        %xmm2,%xmm10
    0x44,0x0f,0x5d,0xd0,                            //  minps         %xmm0,%xmm10
    0x41,0x0f,0x28,0xcb,                            //  movaps        %xmm11,%xmm1
    0x0f,0x59,0xcb,                                 //  mulps         %xmm3,%xmm1
    0x41,0x0f,0xc2,0xdf,0x01,                       //  cmpltps       %xmm15,%xmm3
    0x0f,0x28,0xc3,                                 //  movaps        %xmm3,%xmm0
    0x66,0x44,0x0f,0x38,0x14,0xd1,                  //  blendvps      %xmm0,%xmm1,%xmm10
    0x41,0x0f,0x52,0xc0,                            //  rsqrtps       %xmm8,%xmm0
    0x0f,0x53,0xc8,                                 //  rcpps         %xmm0,%xmm1
    0x41,0x0f,0x59,0xcd,                            //  mulps         %xmm13,%xmm1
    0x41,0x0f,0x58,0xce,                            //  addps         %xmm14,%xmm1
    0x0f,0x52,0xc0,                                 //  rsqrtps       %xmm0,%xmm0
    0x41,0x0f,0x59,0xc4,                            //  mulps         %xmm12,%xmm0
    0x0f,0x58,0xc1,                                 //  addps         %xmm1,%xmm0
    0x0f,0x5d,0xd0,                                 //  minps         %xmm0,%xmm2
    0x45,0x0f,0x59,0xd8,                            //  mulps         %xmm8,%xmm11
    0x45,0x0f,0xc2,0xc7,0x01,                       //  cmpltps       %xmm15,%xmm8
    0x41,0x0f,0x28,0xc0,                            //  movaps        %xmm8,%xmm0
    0x66,0x41,0x0f,0x38,0x14,0xd3,                  //  blendvps      %xmm0,%xmm11,%xmm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x41,0x0f,0x28,0xc1,                            //  movaps        %xmm9,%xmm0
    0x41,0x0f,0x28,0xca,                            //  movaps        %xmm10,%xmm1
    0x0f,0x28,0xdc,                                 //  movaps        %xmm4,%xmm3
    0x0f,0x28,0xe5,                                 //  movaps        %xmm5,%xmm4
    0x0f,0x28,0xee,                                 //  movaps        %xmm6,%xmm5
    0x0f,0x28,0xf7,                                 //  movaps        %xmm7,%xmm6
    0x0f,0x28,0x3c,0x24,                            //  movaps        (%rsp),%xmm7
    0x48,0x83,0xc4,0x18,                            //  add           $0x18,%rsp
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_scale_u8[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x00,                                 //  mov           (%rax),%rax
    0x66,0x44,0x0f,0x38,0x31,0x04,0x38,             //  pmovzxbd      (%rax,%rdi,1),%xmm8
    0x45,0x0f,0x5b,0xc0,                            //  cvtdq2ps      %xmm8,%xmm8
    0xf3,0x44,0x0f,0x10,0x4a,0x0c,                  //  movss         0xc(%rdx),%xmm9
    0x45,0x0f,0xc6,0xc9,0x00,                       //  shufps        $0x0,%xmm9,%xmm9
    0x45,0x0f,0x59,0xc8,                            //  mulps         %xmm8,%xmm9
    0x41,0x0f,0x59,0xc1,                            //  mulps         %xmm9,%xmm0
    0x41,0x0f,0x59,0xc9,                            //  mulps         %xmm9,%xmm1
    0x41,0x0f,0x59,0xd1,                            //  mulps         %xmm9,%xmm2
    0x41,0x0f,0x59,0xd9,                            //  mulps         %xmm9,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_load_tables[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x08,                                 //  mov           (%rax),%rcx
    0x4c,0x8b,0x40,0x08,                            //  mov           0x8(%rax),%r8
    0xf3,0x44,0x0f,0x6f,0x04,0xb9,                  //  movdqu        (%rcx,%rdi,4),%xmm8
    0x66,0x0f,0x6e,0x42,0x10,                       //  movd          0x10(%rdx),%xmm0
    0x66,0x0f,0x70,0xc0,0x00,                       //  pshufd        $0x0,%xmm0,%xmm0
    0x66,0x41,0x0f,0x6f,0xc8,                       //  movdqa        %xmm8,%xmm1
    0x66,0x0f,0x72,0xd1,0x08,                       //  psrld         $0x8,%xmm1
    0x66,0x0f,0xdb,0xc8,                            //  pand          %xmm0,%xmm1
    0x66,0x41,0x0f,0x6f,0xd0,                       //  movdqa        %xmm8,%xmm2
    0x66,0x0f,0x72,0xd2,0x10,                       //  psrld         $0x10,%xmm2
    0x66,0x0f,0xdb,0xd0,                            //  pand          %xmm0,%xmm2
    0x66,0x41,0x0f,0xdb,0xc0,                       //  pand          %xmm8,%xmm0
    0x66,0x48,0x0f,0x3a,0x16,0xc1,0x01,             //  pextrq        $0x1,%xmm0,%rcx
    0x41,0x89,0xc9,                                 //  mov           %ecx,%r9d
    0x48,0xc1,0xe9,0x20,                            //  shr           $0x20,%rcx
    0x66,0x49,0x0f,0x7e,0xc2,                       //  movq          %xmm0,%r10
    0x45,0x89,0xd3,                                 //  mov           %r10d,%r11d
    0x49,0xc1,0xea,0x20,                            //  shr           $0x20,%r10
    0xf3,0x43,0x0f,0x10,0x04,0x98,                  //  movss         (%r8,%r11,4),%xmm0
    0x66,0x43,0x0f,0x3a,0x21,0x04,0x90,0x10,        //  insertps      $0x10,(%r8,%r10,4),%xmm0
    0x66,0x43,0x0f,0x3a,0x21,0x04,0x88,0x20,        //  insertps      $0x20,(%r8,%r9,4),%xmm0
    0x66,0x41,0x0f,0x3a,0x21,0x04,0x88,0x30,        //  insertps      $0x30,(%r8,%rcx,4),%xmm0
    0x48,0x8b,0x48,0x10,                            //  mov           0x10(%rax),%rcx
    0x66,0x49,0x0f,0x3a,0x16,0xc8,0x01,             //  pextrq        $0x1,%xmm1,%r8
    0x45,0x89,0xc1,                                 //  mov           %r8d,%r9d
    0x49,0xc1,0xe8,0x20,                            //  shr           $0x20,%r8
    0x66,0x49,0x0f,0x7e,0xca,                       //  movq          %xmm1,%r10
    0x45,0x89,0xd3,                                 //  mov           %r10d,%r11d
    0x49,0xc1,0xea,0x20,                            //  shr           $0x20,%r10
    0xf3,0x42,0x0f,0x10,0x0c,0x99,                  //  movss         (%rcx,%r11,4),%xmm1
    0x66,0x42,0x0f,0x3a,0x21,0x0c,0x91,0x10,        //  insertps      $0x10,(%rcx,%r10,4),%xmm1
    0xf3,0x42,0x0f,0x10,0x1c,0x89,                  //  movss         (%rcx,%r9,4),%xmm3
    0x66,0x0f,0x3a,0x21,0xcb,0x20,                  //  insertps      $0x20,%xmm3,%xmm1
    0xf3,0x42,0x0f,0x10,0x1c,0x81,                  //  movss         (%rcx,%r8,4),%xmm3
    0x66,0x0f,0x3a,0x21,0xcb,0x30,                  //  insertps      $0x30,%xmm3,%xmm1
    0x48,0x8b,0x40,0x18,                            //  mov           0x18(%rax),%rax
    0x66,0x48,0x0f,0x3a,0x16,0xd1,0x01,             //  pextrq        $0x1,%xmm2,%rcx
    0x41,0x89,0xc8,                                 //  mov           %ecx,%r8d
    0x48,0xc1,0xe9,0x20,                            //  shr           $0x20,%rcx
    0x66,0x49,0x0f,0x7e,0xd1,                       //  movq          %xmm2,%r9
    0x45,0x89,0xca,                                 //  mov           %r9d,%r10d
    0x49,0xc1,0xe9,0x20,                            //  shr           $0x20,%r9
    0xf3,0x42,0x0f,0x10,0x14,0x90,                  //  movss         (%rax,%r10,4),%xmm2
    0x66,0x42,0x0f,0x3a,0x21,0x14,0x88,0x10,        //  insertps      $0x10,(%rax,%r9,4),%xmm2
    0xf3,0x42,0x0f,0x10,0x1c,0x80,                  //  movss         (%rax,%r8,4),%xmm3
    0x66,0x0f,0x3a,0x21,0xd3,0x20,                  //  insertps      $0x20,%xmm3,%xmm2
    0xf3,0x0f,0x10,0x1c,0x88,                       //  movss         (%rax,%rcx,4),%xmm3
    0x66,0x0f,0x3a,0x21,0xd3,0x30,                  //  insertps      $0x30,%xmm3,%xmm2
    0x66,0x41,0x0f,0x72,0xd0,0x18,                  //  psrld         $0x18,%xmm8
    0x45,0x0f,0x5b,0xc0,                            //  cvtdq2ps      %xmm8,%xmm8
    0xf3,0x0f,0x10,0x5a,0x0c,                       //  movss         0xc(%rdx),%xmm3
    0x0f,0xc6,0xdb,0x00,                            //  shufps        $0x0,%xmm3,%xmm3
    0x41,0x0f,0x59,0xd8,                            //  mulps         %xmm8,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_load_8888[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x00,                                 //  mov           (%rax),%rax
    0xf3,0x0f,0x6f,0x1c,0xb8,                       //  movdqu        (%rax,%rdi,4),%xmm3
    0x66,0x0f,0x6e,0x42,0x10,                       //  movd          0x10(%rdx),%xmm0
    0x66,0x0f,0x70,0xc0,0x00,                       //  pshufd        $0x0,%xmm0,%xmm0
    0x66,0x0f,0x6f,0xcb,                            //  movdqa        %xmm3,%xmm1
    0x66,0x0f,0x72,0xd1,0x08,                       //  psrld         $0x8,%xmm1
    0x66,0x0f,0xdb,0xc8,                            //  pand          %xmm0,%xmm1
    0x66,0x0f,0x6f,0xd3,                            //  movdqa        %xmm3,%xmm2
    0x66,0x0f,0x72,0xd2,0x10,                       //  psrld         $0x10,%xmm2
    0x66,0x0f,0xdb,0xd0,                            //  pand          %xmm0,%xmm2
    0x66,0x0f,0xdb,0xc3,                            //  pand          %xmm3,%xmm0
    0x0f,0x5b,0xc0,                                 //  cvtdq2ps      %xmm0,%xmm0
    0xf3,0x44,0x0f,0x10,0x42,0x0c,                  //  movss         0xc(%rdx),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x41,0x0f,0x59,0xc0,                            //  mulps         %xmm8,%xmm0
    0x0f,0x5b,0xc9,                                 //  cvtdq2ps      %xmm1,%xmm1
    0x41,0x0f,0x59,0xc8,                            //  mulps         %xmm8,%xmm1
    0x0f,0x5b,0xd2,                                 //  cvtdq2ps      %xmm2,%xmm2
    0x41,0x0f,0x59,0xd0,                            //  mulps         %xmm8,%xmm2
    0x66,0x0f,0x72,0xd3,0x18,                       //  psrld         $0x18,%xmm3
    0x0f,0x5b,0xdb,                                 //  cvtdq2ps      %xmm3,%xmm3
    0x41,0x0f,0x59,0xd8,                            //  mulps         %xmm8,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_store_8888[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x00,                                 //  mov           (%rax),%rax
    0xf3,0x44,0x0f,0x10,0x42,0x08,                  //  movss         0x8(%rdx),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x45,0x0f,0x28,0xc8,                            //  movaps        %xmm8,%xmm9
    0x44,0x0f,0x59,0xc8,                            //  mulps         %xmm0,%xmm9
    0x66,0x45,0x0f,0x5b,0xc9,                       //  cvtps2dq      %xmm9,%xmm9
    0x45,0x0f,0x28,0xd0,                            //  movaps        %xmm8,%xmm10
    0x44,0x0f,0x59,0xd1,                            //  mulps         %xmm1,%xmm10
    0x66,0x45,0x0f,0x5b,0xd2,                       //  cvtps2dq      %xmm10,%xmm10
    0x66,0x41,0x0f,0x72,0xf2,0x08,                  //  pslld         $0x8,%xmm10
    0x66,0x45,0x0f,0xeb,0xd1,                       //  por           %xmm9,%xmm10
    0x45,0x0f,0x28,0xc8,                            //  movaps        %xmm8,%xmm9
    0x44,0x0f,0x59,0xca,                            //  mulps         %xmm2,%xmm9
    0x66,0x45,0x0f,0x5b,0xc9,                       //  cvtps2dq      %xmm9,%xmm9
    0x66,0x41,0x0f,0x72,0xf1,0x10,                  //  pslld         $0x10,%xmm9
    0x44,0x0f,0x59,0xc3,                            //  mulps         %xmm3,%xmm8
    0x66,0x45,0x0f,0x5b,0xc0,                       //  cvtps2dq      %xmm8,%xmm8
    0x66,0x41,0x0f,0x72,0xf0,0x18,                  //  pslld         $0x18,%xmm8
    0x66,0x45,0x0f,0xeb,0xc1,                       //  por           %xmm9,%xmm8
    0x66,0x45,0x0f,0xeb,0xc2,                       //  por           %xmm10,%xmm8
    0xf3,0x44,0x0f,0x7f,0x04,0xb8,                  //  movdqu        %xmm8,(%rax,%rdi,4)
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_load_f16[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x00,                                 //  mov           (%rax),%rax
    0xf3,0x0f,0x6f,0x04,0xf8,                       //  movdqu        (%rax,%rdi,8),%xmm0
    0xf3,0x0f,0x6f,0x4c,0xf8,0x10,                  //  movdqu        0x10(%rax,%rdi,8),%xmm1
    0x66,0x0f,0x6f,0xd0,                            //  movdqa        %xmm0,%xmm2
    0x66,0x0f,0x61,0xd1,                            //  punpcklwd     %xmm1,%xmm2
    0x66,0x0f,0x69,0xc1,                            //  punpckhwd     %xmm1,%xmm0
    0x66,0x44,0x0f,0x6f,0xc2,                       //  movdqa        %xmm2,%xmm8
    0x66,0x44,0x0f,0x61,0xc0,                       //  punpcklwd     %xmm0,%xmm8
    0x66,0x0f,0x69,0xd0,                            //  punpckhwd     %xmm0,%xmm2
    0x66,0x0f,0x6e,0x42,0x64,                       //  movd          0x64(%rdx),%xmm0
    0x66,0x0f,0x70,0xd8,0x00,                       //  pshufd        $0x0,%xmm0,%xmm3
    0x66,0x0f,0x6f,0xcb,                            //  movdqa        %xmm3,%xmm1
    0x66,0x41,0x0f,0x65,0xc8,                       //  pcmpgtw       %xmm8,%xmm1
    0x66,0x41,0x0f,0xdf,0xc8,                       //  pandn         %xmm8,%xmm1
    0x66,0x0f,0x65,0xda,                            //  pcmpgtw       %xmm2,%xmm3
    0x66,0x0f,0xdf,0xda,                            //  pandn         %xmm2,%xmm3
    0x66,0x0f,0x38,0x33,0xc1,                       //  pmovzxwd      %xmm1,%xmm0
    0x66,0x0f,0x72,0xf0,0x0d,                       //  pslld         $0xd,%xmm0
    0x66,0x0f,0x6e,0x52,0x5c,                       //  movd          0x5c(%rdx),%xmm2
    0x66,0x44,0x0f,0x70,0xc2,0x00,                  //  pshufd        $0x0,%xmm2,%xmm8
    0x41,0x0f,0x59,0xc0,                            //  mulps         %xmm8,%xmm0
    0x66,0x45,0x0f,0xef,0xc9,                       //  pxor          %xmm9,%xmm9
    0x66,0x41,0x0f,0x69,0xc9,                       //  punpckhwd     %xmm9,%xmm1
    0x66,0x0f,0x72,0xf1,0x0d,                       //  pslld         $0xd,%xmm1
    0x41,0x0f,0x59,0xc8,                            //  mulps         %xmm8,%xmm1
    0x66,0x0f,0x38,0x33,0xd3,                       //  pmovzxwd      %xmm3,%xmm2
    0x66,0x0f,0x72,0xf2,0x0d,                       //  pslld         $0xd,%xmm2
    0x41,0x0f,0x59,0xd0,                            //  mulps         %xmm8,%xmm2
    0x66,0x41,0x0f,0x69,0xd9,                       //  punpckhwd     %xmm9,%xmm3
    0x66,0x0f,0x72,0xf3,0x0d,                       //  pslld         $0xd,%xmm3
    0x41,0x0f,0x59,0xd8,                            //  mulps         %xmm8,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_store_f16[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x00,                                 //  mov           (%rax),%rax
    0x66,0x44,0x0f,0x6e,0x42,0x60,                  //  movd          0x60(%rdx),%xmm8
    0x66,0x45,0x0f,0x70,0xc0,0x00,                  //  pshufd        $0x0,%xmm8,%xmm8
    0x66,0x45,0x0f,0x6f,0xc8,                       //  movdqa        %xmm8,%xmm9
    0x44,0x0f,0x59,0xc8,                            //  mulps         %xmm0,%xmm9
    0x66,0x41,0x0f,0x72,0xd1,0x0d,                  //  psrld         $0xd,%xmm9
    0x66,0x45,0x0f,0x6f,0xd0,                       //  movdqa        %xmm8,%xmm10
    0x44,0x0f,0x59,0xd1,                            //  mulps         %xmm1,%xmm10
    0x66,0x41,0x0f,0x72,0xd2,0x0d,                  //  psrld         $0xd,%xmm10
    0x66,0x45,0x0f,0x6f,0xd8,                       //  movdqa        %xmm8,%xmm11
    0x44,0x0f,0x59,0xda,                            //  mulps         %xmm2,%xmm11
    0x66,0x41,0x0f,0x72,0xd3,0x0d,                  //  psrld         $0xd,%xmm11
    0x44,0x0f,0x59,0xc3,                            //  mulps         %xmm3,%xmm8
    0x66,0x41,0x0f,0x72,0xd0,0x0d,                  //  psrld         $0xd,%xmm8
    0x66,0x41,0x0f,0x73,0xfa,0x02,                  //  pslldq        $0x2,%xmm10
    0x66,0x45,0x0f,0xeb,0xd1,                       //  por           %xmm9,%xmm10
    0x66,0x41,0x0f,0x73,0xf8,0x02,                  //  pslldq        $0x2,%xmm8
    0x66,0x45,0x0f,0xeb,0xc3,                       //  por           %xmm11,%xmm8
    0x66,0x45,0x0f,0x6f,0xca,                       //  movdqa        %xmm10,%xmm9
    0x66,0x45,0x0f,0x62,0xc8,                       //  punpckldq     %xmm8,%xmm9
    0xf3,0x44,0x0f,0x7f,0x0c,0xf8,                  //  movdqu        %xmm9,(%rax,%rdi,8)
    0x66,0x45,0x0f,0x6a,0xd0,                       //  punpckhdq     %xmm8,%xmm10
    0xf3,0x44,0x0f,0x7f,0x54,0xf8,0x10,             //  movdqu        %xmm10,0x10(%rax,%rdi,8)
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_clamp_x[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x44,0x0f,0x10,0x00,                       //  movss         (%rax),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x66,0x45,0x0f,0x76,0xc9,                       //  pcmpeqd       %xmm9,%xmm9
    0x66,0x45,0x0f,0xfe,0xc8,                       //  paddd         %xmm8,%xmm9
    0x41,0x0f,0x5d,0xc1,                            //  minps         %xmm9,%xmm0
    0x45,0x0f,0x57,0xc0,                            //  xorps         %xmm8,%xmm8
    0x44,0x0f,0x5f,0xc0,                            //  maxps         %xmm0,%xmm8
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x41,0x0f,0x28,0xc0,                            //  movaps        %xmm8,%xmm0
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_clamp_y[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x44,0x0f,0x10,0x00,                       //  movss         (%rax),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x66,0x45,0x0f,0x76,0xc9,                       //  pcmpeqd       %xmm9,%xmm9
    0x66,0x45,0x0f,0xfe,0xc8,                       //  paddd         %xmm8,%xmm9
    0x41,0x0f,0x5d,0xc9,                            //  minps         %xmm9,%xmm1
    0x45,0x0f,0x57,0xc0,                            //  xorps         %xmm8,%xmm8
    0x44,0x0f,0x5f,0xc1,                            //  maxps         %xmm1,%xmm8
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x41,0x0f,0x28,0xc8,                            //  movaps        %xmm8,%xmm1
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_matrix_2x3[] = {
    0x44,0x0f,0x28,0xc9,                            //  movaps        %xmm1,%xmm9
    0x44,0x0f,0x28,0xc0,                            //  movaps        %xmm0,%xmm8
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x0f,0x10,0x00,                            //  movss         (%rax),%xmm0
    0xf3,0x0f,0x10,0x48,0x04,                       //  movss         0x4(%rax),%xmm1
    0x0f,0xc6,0xc0,0x00,                            //  shufps        $0x0,%xmm0,%xmm0
    0xf3,0x44,0x0f,0x10,0x50,0x08,                  //  movss         0x8(%rax),%xmm10
    0x45,0x0f,0xc6,0xd2,0x00,                       //  shufps        $0x0,%xmm10,%xmm10
    0xf3,0x44,0x0f,0x10,0x58,0x10,                  //  movss         0x10(%rax),%xmm11
    0x45,0x0f,0xc6,0xdb,0x00,                       //  shufps        $0x0,%xmm11,%xmm11
    0x45,0x0f,0x59,0xd1,                            //  mulps         %xmm9,%xmm10
    0x45,0x0f,0x58,0xd3,                            //  addps         %xmm11,%xmm10
    0x41,0x0f,0x59,0xc0,                            //  mulps         %xmm8,%xmm0
    0x41,0x0f,0x58,0xc2,                            //  addps         %xmm10,%xmm0
    0x0f,0xc6,0xc9,0x00,                            //  shufps        $0x0,%xmm1,%xmm1
    0xf3,0x44,0x0f,0x10,0x50,0x0c,                  //  movss         0xc(%rax),%xmm10
    0x45,0x0f,0xc6,0xd2,0x00,                       //  shufps        $0x0,%xmm10,%xmm10
    0xf3,0x44,0x0f,0x10,0x58,0x14,                  //  movss         0x14(%rax),%xmm11
    0x45,0x0f,0xc6,0xdb,0x00,                       //  shufps        $0x0,%xmm11,%xmm11
    0x45,0x0f,0x59,0xd1,                            //  mulps         %xmm9,%xmm10
    0x45,0x0f,0x58,0xd3,                            //  addps         %xmm11,%xmm10
    0x41,0x0f,0x59,0xc8,                            //  mulps         %xmm8,%xmm1
    0x41,0x0f,0x58,0xca,                            //  addps         %xmm10,%xmm1
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_matrix_3x4[] = {
    0x44,0x0f,0x28,0xc9,                            //  movaps        %xmm1,%xmm9
    0x44,0x0f,0x28,0xc0,                            //  movaps        %xmm0,%xmm8
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x0f,0x10,0x00,                            //  movss         (%rax),%xmm0
    0xf3,0x0f,0x10,0x48,0x04,                       //  movss         0x4(%rax),%xmm1
    0x0f,0xc6,0xc0,0x00,                            //  shufps        $0x0,%xmm0,%xmm0
    0xf3,0x44,0x0f,0x10,0x50,0x0c,                  //  movss         0xc(%rax),%xmm10
    0x45,0x0f,0xc6,0xd2,0x00,                       //  shufps        $0x0,%xmm10,%xmm10
    0xf3,0x44,0x0f,0x10,0x58,0x18,                  //  movss         0x18(%rax),%xmm11
    0x45,0x0f,0xc6,0xdb,0x00,                       //  shufps        $0x0,%xmm11,%xmm11
    0xf3,0x44,0x0f,0x10,0x60,0x24,                  //  movss         0x24(%rax),%xmm12
    0x45,0x0f,0xc6,0xe4,0x00,                       //  shufps        $0x0,%xmm12,%xmm12
    0x44,0x0f,0x59,0xda,                            //  mulps         %xmm2,%xmm11
    0x45,0x0f,0x58,0xdc,                            //  addps         %xmm12,%xmm11
    0x45,0x0f,0x59,0xd1,                            //  mulps         %xmm9,%xmm10
    0x45,0x0f,0x58,0xd3,                            //  addps         %xmm11,%xmm10
    0x41,0x0f,0x59,0xc0,                            //  mulps         %xmm8,%xmm0
    0x41,0x0f,0x58,0xc2,                            //  addps         %xmm10,%xmm0
    0x0f,0xc6,0xc9,0x00,                            //  shufps        $0x0,%xmm1,%xmm1
    0xf3,0x44,0x0f,0x10,0x50,0x10,                  //  movss         0x10(%rax),%xmm10
    0x45,0x0f,0xc6,0xd2,0x00,                       //  shufps        $0x0,%xmm10,%xmm10
    0xf3,0x44,0x0f,0x10,0x58,0x1c,                  //  movss         0x1c(%rax),%xmm11
    0x45,0x0f,0xc6,0xdb,0x00,                       //  shufps        $0x0,%xmm11,%xmm11
    0xf3,0x44,0x0f,0x10,0x60,0x28,                  //  movss         0x28(%rax),%xmm12
    0x45,0x0f,0xc6,0xe4,0x00,                       //  shufps        $0x0,%xmm12,%xmm12
    0x44,0x0f,0x59,0xda,                            //  mulps         %xmm2,%xmm11
    0x45,0x0f,0x58,0xdc,                            //  addps         %xmm12,%xmm11
    0x45,0x0f,0x59,0xd1,                            //  mulps         %xmm9,%xmm10
    0x45,0x0f,0x58,0xd3,                            //  addps         %xmm11,%xmm10
    0x41,0x0f,0x59,0xc8,                            //  mulps         %xmm8,%xmm1
    0x41,0x0f,0x58,0xca,                            //  addps         %xmm10,%xmm1
    0xf3,0x44,0x0f,0x10,0x50,0x08,                  //  movss         0x8(%rax),%xmm10
    0x45,0x0f,0xc6,0xd2,0x00,                       //  shufps        $0x0,%xmm10,%xmm10
    0xf3,0x44,0x0f,0x10,0x58,0x14,                  //  movss         0x14(%rax),%xmm11
    0x45,0x0f,0xc6,0xdb,0x00,                       //  shufps        $0x0,%xmm11,%xmm11
    0xf3,0x44,0x0f,0x10,0x60,0x20,                  //  movss         0x20(%rax),%xmm12
    0x45,0x0f,0xc6,0xe4,0x00,                       //  shufps        $0x0,%xmm12,%xmm12
    0xf3,0x44,0x0f,0x10,0x68,0x2c,                  //  movss         0x2c(%rax),%xmm13
    0x45,0x0f,0xc6,0xed,0x00,                       //  shufps        $0x0,%xmm13,%xmm13
    0x44,0x0f,0x59,0xe2,                            //  mulps         %xmm2,%xmm12
    0x45,0x0f,0x58,0xe5,                            //  addps         %xmm13,%xmm12
    0x45,0x0f,0x59,0xd9,                            //  mulps         %xmm9,%xmm11
    0x45,0x0f,0x58,0xdc,                            //  addps         %xmm12,%xmm11
    0x45,0x0f,0x59,0xd0,                            //  mulps         %xmm8,%xmm10
    0x45,0x0f,0x58,0xd3,                            //  addps         %xmm11,%xmm10
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x41,0x0f,0x28,0xd2,                            //  movaps        %xmm10,%xmm2
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse41_sk_linear_gradient_2stops[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x44,0x0f,0x10,0x08,                            //  movups        (%rax),%xmm9
    0x0f,0x10,0x58,0x10,                            //  movups        0x10(%rax),%xmm3
    0x44,0x0f,0x28,0xc3,                            //  movaps        %xmm3,%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x41,0x0f,0x28,0xc9,                            //  movaps        %xmm9,%xmm1
    0x0f,0xc6,0xc9,0x00,                            //  shufps        $0x0,%xmm1,%xmm1
    0x44,0x0f,0x59,0xc0,                            //  mulps         %xmm0,%xmm8
    0x44,0x0f,0x58,0xc1,                            //  addps         %xmm1,%xmm8
    0x0f,0x28,0xcb,                                 //  movaps        %xmm3,%xmm1
    0x0f,0xc6,0xc9,0x55,                            //  shufps        $0x55,%xmm1,%xmm1
    0x41,0x0f,0x28,0xd1,                            //  movaps        %xmm9,%xmm2
    0x0f,0xc6,0xd2,0x55,                            //  shufps        $0x55,%xmm2,%xmm2
    0x0f,0x59,0xc8,                                 //  mulps         %xmm0,%xmm1
    0x0f,0x58,0xca,                                 //  addps         %xmm2,%xmm1
    0x0f,0x28,0xd3,                                 //  movaps        %xmm3,%xmm2
    0x0f,0xc6,0xd2,0xaa,                            //  shufps        $0xaa,%xmm2,%xmm2
    0x45,0x0f,0x28,0xd1,                            //  movaps        %xmm9,%xmm10
    0x45,0x0f,0xc6,0xd2,0xaa,                       //  shufps        $0xaa,%xmm10,%xmm10
    0x0f,0x59,0xd0,                                 //  mulps         %xmm0,%xmm2
    0x41,0x0f,0x58,0xd2,                            //  addps         %xmm10,%xmm2
    0x0f,0xc6,0xdb,0xff,                            //  shufps        $0xff,%xmm3,%xmm3
    0x45,0x0f,0xc6,0xc9,0xff,                       //  shufps        $0xff,%xmm9,%xmm9
    0x0f,0x59,0xd8,                                 //  mulps         %xmm0,%xmm3
    0x41,0x0f,0x58,0xd9,                            //  addps         %xmm9,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x41,0x0f,0x28,0xc0,                            //  movaps        %xmm8,%xmm0
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_start_pipeline[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x57,0xc0,                                 //  xorps         %xmm0,%xmm0
    0x0f,0x57,0xc9,                                 //  xorps         %xmm1,%xmm1
    0x0f,0x57,0xd2,                                 //  xorps         %xmm2,%xmm2
    0x0f,0x57,0xdb,                                 //  xorps         %xmm3,%xmm3
    0x0f,0x57,0xe4,                                 //  xorps         %xmm4,%xmm4
    0x0f,0x57,0xed,                                 //  xorps         %xmm5,%xmm5
    0x0f,0x57,0xf6,                                 //  xorps         %xmm6,%xmm6
    0x0f,0x57,0xff,                                 //  xorps         %xmm7,%xmm7
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_start_pipeline_ms[] = {
    0x56,                                           //  push          %rsi
    0x57,                                           //  push          %rdi
    0x48,0x81,0xec,0xa8,0x00,0x00,0x00,             //  sub           $0xa8,%rsp
    0x44,0x0f,0x29,0xbc,0x24,0x90,0x00,0x00,0x00,   //  movaps        %xmm15,0x90(%rsp)
    0x44,0x0f,0x29,0xb4,0x24,0x80,0x00,0x00,0x00,   //  movaps        %xmm14,0x80(%rsp)
    0x44,0x0f,0x29,0x6c,0x24,0x70,                  //  movaps        %xmm13,0x70(%rsp)
    0x44,0x0f,0x29,0x64,0x24,0x60,                  //  movaps        %xmm12,0x60(%rsp)
    0x44,0x0f,0x29,0x5c,0x24,0x50,                  //  movaps        %xmm11,0x50(%rsp)
    0x44,0x0f,0x29,0x54,0x24,0x40,                  //  movaps        %xmm10,0x40(%rsp)
    0x44,0x0f,0x29,0x4c,0x24,0x30,                  //  movaps        %xmm9,0x30(%rsp)
    0x44,0x0f,0x29,0x44,0x24,0x20,                  //  movaps        %xmm8,0x20(%rsp)
    0x0f,0x29,0x7c,0x24,0x10,                       //  movaps        %xmm7,0x10(%rsp)
    0x0f,0x29,0x34,0x24,                            //  movaps        %xmm6,(%rsp)
    0x48,0x89,0xd6,                                 //  mov           %rdx,%rsi
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x57,0xc0,                                 //  xorps         %xmm0,%xmm0
    0x0f,0x57,0xc9,                                 //  xorps         %xmm1,%xmm1
    0x0f,0x57,0xd2,                                 //  xorps         %xmm2,%xmm2
    0x0f,0x57,0xdb,                                 //  xorps         %xmm3,%xmm3
    0x0f,0x57,0xe4,                                 //  xorps         %xmm4,%xmm4
    0x0f,0x57,0xed,                                 //  xorps         %xmm5,%xmm5
    0x0f,0x57,0xf6,                                 //  xorps         %xmm6,%xmm6
    0x0f,0x57,0xff,                                 //  xorps         %xmm7,%xmm7
    0x48,0x89,0xcf,                                 //  mov           %rcx,%rdi
    0x4c,0x89,0xc2,                                 //  mov           %r8,%rdx
    0xff,0xd0,                                      //  callq         *%rax
    0x0f,0x28,0x34,0x24,                            //  movaps        (%rsp),%xmm6
    0x0f,0x28,0x7c,0x24,0x10,                       //  movaps        0x10(%rsp),%xmm7
    0x44,0x0f,0x28,0x44,0x24,0x20,                  //  movaps        0x20(%rsp),%xmm8
    0x44,0x0f,0x28,0x4c,0x24,0x30,                  //  movaps        0x30(%rsp),%xmm9
    0x44,0x0f,0x28,0x54,0x24,0x40,                  //  movaps        0x40(%rsp),%xmm10
    0x44,0x0f,0x28,0x5c,0x24,0x50,                  //  movaps        0x50(%rsp),%xmm11
    0x44,0x0f,0x28,0x64,0x24,0x60,                  //  movaps        0x60(%rsp),%xmm12
    0x44,0x0f,0x28,0x6c,0x24,0x70,                  //  movaps        0x70(%rsp),%xmm13
    0x44,0x0f,0x28,0xb4,0x24,0x80,0x00,0x00,0x00,   //  movaps        0x80(%rsp),%xmm14
    0x44,0x0f,0x28,0xbc,0x24,0x90,0x00,0x00,0x00,   //  movaps        0x90(%rsp),%xmm15
    0x48,0x81,0xc4,0xa8,0x00,0x00,0x00,             //  add           $0xa8,%rsp
    0x5f,                                           //  pop           %rdi
    0x5e,                                           //  pop           %rsi
    0xc3,                                           //  retq
};
static const unsigned char sse2_sk_just_return[] = {
    0xc3,                                           //  retq
};
static const unsigned char sse2_sk_seed_shader[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x66,0x0f,0x6e,0xc7,                            //  movd          %edi,%xmm0
    0x66,0x0f,0x70,0xc0,0x00,                       //  pshufd        $0x0,%xmm0,%xmm0
    0x0f,0x5b,0xc8,                                 //  cvtdq2ps      %xmm0,%xmm1
    0xf3,0x0f,0x10,0x12,                            //  movss         (%rdx),%xmm2
    0xf3,0x0f,0x10,0x5a,0x04,                       //  movss         0x4(%rdx),%xmm3
    0x0f,0xc6,0xdb,0x00,                            //  shufps        $0x0,%xmm3,%xmm3
    0x0f,0x58,0xcb,                                 //  addps         %xmm3,%xmm1
    0x0f,0x10,0x42,0x14,                            //  movups        0x14(%rdx),%xmm0
    0x0f,0x58,0xc1,                                 //  addps         %xmm1,%xmm0
    0x66,0x0f,0x6e,0x08,                            //  movd          (%rax),%xmm1
    0x66,0x0f,0x70,0xc9,0x00,                       //  pshufd        $0x0,%xmm1,%xmm1
    0x0f,0x5b,0xc9,                                 //  cvtdq2ps      %xmm1,%xmm1
    0x0f,0x58,0xcb,                                 //  addps         %xmm3,%xmm1
    0x0f,0xc6,0xd2,0x00,                            //  shufps        $0x0,%xmm2,%xmm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x57,0xdb,                                 //  xorps         %xmm3,%xmm3
    0x0f,0x57,0xe4,                                 //  xorps         %xmm4,%xmm4
    0x0f,0x57,0xed,                                 //  xorps         %xmm5,%xmm5
    0x0f,0x57,0xf6,                                 //  xorps         %xmm6,%xmm6
    0x0f,0x57,0xff,                                 //  xorps         %xmm7,%xmm7
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_constant_color[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x10,0x18,                                 //  movups        (%rax),%xmm3
    0x0f,0x28,0xc3,                                 //  movaps        %xmm3,%xmm0
    0x0f,0xc6,0xc0,0x00,                            //  shufps        $0x0,%xmm0,%xmm0
    0x0f,0x28,0xcb,                                 //  movaps        %xmm3,%xmm1
    0x0f,0xc6,0xc9,0x55,                            //  shufps        $0x55,%xmm1,%xmm1
    0x0f,0x28,0xd3,                                 //  movaps        %xmm3,%xmm2
    0x0f,0xc6,0xd2,0xaa,                            //  shufps        $0xaa,%xmm2,%xmm2
    0x0f,0xc6,0xdb,0xff,                            //  shufps        $0xff,%xmm3,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_clear[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x57,0xc0,                                 //  xorps         %xmm0,%xmm0
    0x0f,0x57,0xc9,                                 //  xorps         %xmm1,%xmm1
    0x0f,0x57,0xd2,                                 //  xorps         %xmm2,%xmm2
    0x0f,0x57,0xdb,                                 //  xorps         %xmm3,%xmm3
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_plus_[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x58,0xc4,                                 //  addps         %xmm4,%xmm0
    0x0f,0x58,0xcd,                                 //  addps         %xmm5,%xmm1
    0x0f,0x58,0xd6,                                 //  addps         %xmm6,%xmm2
    0x0f,0x58,0xdf,                                 //  addps         %xmm7,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_srcover[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x44,0x0f,0x10,0x02,                       //  movss         (%rdx),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x44,0x0f,0x5c,0xc3,                            //  subps         %xmm3,%xmm8
    0x45,0x0f,0x28,0xc8,                            //  movaps        %xmm8,%xmm9
    0x44,0x0f,0x59,0xcc,                            //  mulps         %xmm4,%xmm9
    0x41,0x0f,0x58,0xc1,                            //  addps         %xmm9,%xmm0
    0x45,0x0f,0x28,0xc8,                            //  movaps        %xmm8,%xmm9
    0x44,0x0f,0x59,0xcd,                            //  mulps         %xmm5,%xmm9
    0x41,0x0f,0x58,0xc9,                            //  addps         %xmm9,%xmm1
    0x45,0x0f,0x28,0xc8,                            //  movaps        %xmm8,%xmm9
    0x44,0x0f,0x59,0xce,                            //  mulps         %xmm6,%xmm9
    0x41,0x0f,0x58,0xd1,                            //  addps         %xmm9,%xmm2
    0x44,0x0f,0x59,0xc7,                            //  mulps         %xmm7,%xmm8
    0x41,0x0f,0x58,0xd8,                            //  addps         %xmm8,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_dstover[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x44,0x0f,0x10,0x02,                       //  movss         (%rdx),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x44,0x0f,0x5c,0xc7,                            //  subps         %xmm7,%xmm8
    0x41,0x0f,0x59,0xc0,                            //  mulps         %xmm8,%xmm0
    0x0f,0x58,0xc4,                                 //  addps         %xmm4,%xmm0
    0x41,0x0f,0x59,0xc8,                            //  mulps         %xmm8,%xmm1
    0x0f,0x58,0xcd,                                 //  addps         %xmm5,%xmm1
    0x41,0x0f,0x59,0xd0,                            //  mulps         %xmm8,%xmm2
    0x0f,0x58,0xd6,                                 //  addps         %xmm6,%xmm2
    0x41,0x0f,0x59,0xd8,                            //  mulps         %xmm8,%xmm3
    0x0f,0x58,0xdf,                                 //  addps         %xmm7,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_clamp_0[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x45,0x0f,0x57,0xc0,                            //  xorps         %xmm8,%xmm8
    0x41,0x0f,0x5f,0xc0,                            //  maxps         %xmm8,%xmm0
    0x41,0x0f,0x5f,0xc8,                            //  maxps         %xmm8,%xmm1
    0x41,0x0f,0x5f,0xd0,                            //  maxps         %xmm8,%xmm2
    0x41,0x0f,0x5f,0xd8,                            //  maxps         %xmm8,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_clamp_1[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x44,0x0f,0x10,0x02,                       //  movss         (%rdx),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x41,0x0f,0x5d,0xc0,                            //  minps         %xmm8,%xmm0
    0x41,0x0f,0x5d,0xc8,                            //  minps         %xmm8,%xmm1
    0x41,0x0f,0x5d,0xd0,                            //  minps         %xmm8,%xmm2
    0x41,0x0f,0x5d,0xd8,                            //  minps         %xmm8,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_clamp_a[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x44,0x0f,0x10,0x02,                       //  movss         (%rdx),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x41,0x0f,0x5d,0xd8,                            //  minps         %xmm8,%xmm3
    0x0f,0x5d,0xc3,                                 //  minps         %xmm3,%xmm0
    0x0f,0x5d,0xcb,                                 //  minps         %xmm3,%xmm1
    0x0f,0x5d,0xd3,                                 //  minps         %xmm3,%xmm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_swap[] = {
    0x44,0x0f,0x28,0xc3,                            //  movaps        %xmm3,%xmm8
    0x44,0x0f,0x28,0xca,                            //  movaps        %xmm2,%xmm9
    0x44,0x0f,0x28,0xd1,                            //  movaps        %xmm1,%xmm10
    0x44,0x0f,0x28,0xd8,                            //  movaps        %xmm0,%xmm11
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x28,0xc4,                                 //  movaps        %xmm4,%xmm0
    0x0f,0x28,0xcd,                                 //  movaps        %xmm5,%xmm1
    0x0f,0x28,0xd6,                                 //  movaps        %xmm6,%xmm2
    0x0f,0x28,0xdf,                                 //  movaps        %xmm7,%xmm3
    0x41,0x0f,0x28,0xe3,                            //  movaps        %xmm11,%xmm4
    0x41,0x0f,0x28,0xea,                            //  movaps        %xmm10,%xmm5
    0x41,0x0f,0x28,0xf1,                            //  movaps        %xmm9,%xmm6
    0x41,0x0f,0x28,0xf8,                            //  movaps        %xmm8,%xmm7
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_move_src_dst[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x28,0xe0,                                 //  movaps        %xmm0,%xmm4
    0x0f,0x28,0xe9,                                 //  movaps        %xmm1,%xmm5
    0x0f,0x28,0xf2,                                 //  movaps        %xmm2,%xmm6
    0x0f,0x28,0xfb,                                 //  movaps        %xmm3,%xmm7
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_move_dst_src[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x28,0xc4,                                 //  movaps        %xmm4,%xmm0
    0x0f,0x28,0xcd,                                 //  movaps        %xmm5,%xmm1
    0x0f,0x28,0xd6,                                 //  movaps        %xmm6,%xmm2
    0x0f,0x28,0xdf,                                 //  movaps        %xmm7,%xmm3
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_premul[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x59,0xc3,                                 //  mulps         %xmm3,%xmm0
    0x0f,0x59,0xcb,                                 //  mulps         %xmm3,%xmm1
    0x0f,0x59,0xd3,                                 //  mulps         %xmm3,%xmm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_unpremul[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x45,0x0f,0x57,0xc0,                            //  xorps         %xmm8,%xmm8
    0x44,0x0f,0xc2,0xc3,0x00,                       //  cmpeqps       %xmm3,%xmm8
    0xf3,0x44,0x0f,0x10,0x0a,                       //  movss         (%rdx),%xmm9
    0x45,0x0f,0xc6,0xc9,0x00,                       //  shufps        $0x0,%xmm9,%xmm9
    0x44,0x0f,0x5e,0xcb,                            //  divps         %xmm3,%xmm9
    0x45,0x0f,0x55,0xc1,                            //  andnps        %xmm9,%xmm8
    0x41,0x0f,0x59,0xc0,                            //  mulps         %xmm8,%xmm0
    0x41,0x0f,0x59,0xc8,                            //  mulps         %xmm8,%xmm1
    0x41,0x0f,0x59,0xd0,                            //  mulps         %xmm8,%xmm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_from_srgb[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x44,0x0f,0x10,0x42,0x40,                  //  movss         0x40(%rdx),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x45,0x0f,0x28,0xe8,                            //  movaps        %xmm8,%xmm13
    0x44,0x0f,0x59,0xe8,                            //  mulps         %xmm0,%xmm13
    0x44,0x0f,0x28,0xe0,                            //  movaps        %xmm0,%xmm12
    0x45,0x0f,0x59,0xe4,                            //  mulps         %xmm12,%xmm12
    0xf3,0x44,0x0f,0x10,0x4a,0x3c,                  //  movss         0x3c(%rdx),%xmm9
    0x45,0x0f,0xc6,0xc9,0x00,                       //  shufps        $0x0,%xmm9,%xmm9
    0xf3,0x44,0x0f,0x10,0x52,0x34,                  //  movss         0x34(%rdx),%xmm10
    0xf3,0x44,0x0f,0x10,0x5a,0x38,                  //  movss         0x38(%rdx),%xmm11
    0x45,0x0f,0xc6,0xdb,0x00,                       //  shufps        $0x0,%xmm11,%xmm11
    0x45,0x0f,0x28,0xf1,                            //  movaps        %xmm9,%xmm14
    0x44,0x0f,0x59,0xf0,                            //  mulps         %xmm0,%xmm14
    0x45,0x0f,0x58,0xf3,                            //  addps         %xmm11,%xmm14
    0x45,0x0f,0xc6,0xd2,0x00,                       //  shufps        $0x0,%xmm10,%xmm10
    0x45,0x0f,0x59,0xf4,                            //  mulps         %xmm12,%xmm14
    0x45,0x0f,0x58,0xf2,                            //  addps         %xmm10,%xmm14
    0xf3,0x44,0x0f,0x10,0x62,0x44,                  //  movss         0x44(%rdx),%xmm12
    0x45,0x0f,0xc6,0xe4,0x00,                       //  shufps        $0x0,%xmm12,%xmm12
    0x41,0x0f,0xc2,0xc4,0x01,                       //  cmpltps       %xmm12,%xmm0
    0x44,0x0f,0x54,0xe8,                            //  andps         %xmm0,%xmm13
    0x41,0x0f,0x55,0xc6,                            //  andnps        %xmm14,%xmm0
    0x41,0x0f,0x56,0xc5,                            //  orps          %xmm13,%xmm0
    0x45,0x0f,0x28,0xe8,                            //  movaps        %xmm8,%xmm13
    0x44,0x0f,0x59,0xe9,                            //  mulps         %xmm1,%xmm13
    0x44,0x0f,0x28,0xf1,                            //  movaps        %xmm1,%xmm14
    0x45,0x0f,0x59,0xf6,                            //  mulps         %xmm14,%xmm14
    0x45,0x0f,0x28,0xf9,                            //  movaps        %xmm9,%xmm15
    0x44,0x0f,0x59,0xf9,                            //  mulps         %xmm1,%xmm15
    0x45,0x0f,0x58,0xfb,                            //  addps         %xmm11,%xmm15
    0x45,0x0f,0x59,0xfe,                            //  mulps         %xmm14,%xmm15
    0x45,0x0f,0x58,0xfa,                            //  addps         %xmm10,%xmm15
    0x41,0x0f,0xc2,0xcc,0x01,                       //  cmpltps       %xmm12,%xmm1
    0x44,0x0f,0x54,0xe9,                            //  andps         %xmm1,%xmm13
    0x41,0x0f,0x55,0xcf,                            //  andnps        %xmm15,%xmm1
    0x41,0x0f,0x56,0xcd,                            //  orps          %xmm13,%xmm1
    0x44,0x0f,0x59,0xc2,                            //  mulps         %xmm2,%xmm8
    0x44,0x0f,0x28,0xea,                            //  movaps        %xmm2,%xmm13
    0x45,0x0f,0x59,0xed,                            //  mulps         %xmm13,%xmm13
    0x44,0x0f,0x59,0xca,                            //  mulps         %xmm2,%xmm9
    0x45,0x0f,0x58,0xcb,                            //  addps         %xmm11,%xmm9
    0x45,0x0f,0x59,0xcd,                            //  mulps         %xmm13,%xmm9
    0x45,0x0f,0x58,0xca,                            //  addps         %xmm10,%xmm9
    0x41,0x0f,0xc2,0xd4,0x01,                       //  cmpltps       %xmm12,%xmm2
    0x44,0x0f,0x54,0xc2,                            //  andps         %xmm2,%xmm8
    0x41,0x0f,0x55,0xd1,                            //  andnps        %xmm9,%xmm2
    0x41,0x0f,0x56,0xd0,                            //  orps          %xmm8,%xmm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_to_srgb[] = {
    0x48,0x83,0xec,0x28,                            //  sub           $0x28,%rsp
    0x0f,0x29,0x7c,0x24,0x10,                       //  movaps        %xmm7,0x10(%rsp)
    0x0f,0x29,0x34,0x24,                            //  movaps        %xmm6,(%rsp)
    0x0f,0x28,0xf5,                                 //  movaps        %xmm5,%xmm6
    0x0f,0x28,0xec,                                 //  movaps        %xmm4,%xmm5
    0x0f,0x28,0xe3,                                 //  movaps        %xmm3,%xmm4
    0x44,0x0f,0x52,0xc0,                            //  rsqrtps       %xmm0,%xmm8
    0x45,0x0f,0x53,0xe8,                            //  rcpps         %xmm8,%xmm13
    0x45,0x0f,0x52,0xf8,                            //  rsqrtps       %xmm8,%xmm15
    0xf3,0x0f,0x10,0x1a,                            //  movss         (%rdx),%xmm3
    0xf3,0x44,0x0f,0x10,0x42,0x48,                  //  movss         0x48(%rdx),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x45,0x0f,0x28,0xf0,                            //  movaps        %xmm8,%xmm14
    0x44,0x0f,0x59,0xf0,                            //  mulps         %xmm0,%xmm14
    0x0f,0xc6,0xdb,0x00,                            //  shufps        $0x0,%xmm3,%xmm3
    0xf3,0x44,0x0f,0x10,0x52,0x4c,                  //  movss         0x4c(%rdx),%xmm10
    0x45,0x0f,0xc6,0xd2,0x00,                       //  shufps        $0x0,%xmm10,%xmm10
    0xf3,0x44,0x0f,0x10,0x5a,0x50,                  //  movss         0x50(%rdx),%xmm11
    0x45,0x0f,0xc6,0xdb,0x00,                       //  shufps        $0x0,%xmm11,%xmm11
    0xf3,0x44,0x0f,0x10,0x62,0x54,                  //  movss         0x54(%rdx),%xmm12
    0x45,0x0f,0xc6,0xe4,0x00,                       //  shufps        $0x0,%xmm12,%xmm12
    0x45,0x0f,0x59,0xeb,                            //  mulps         %xmm11,%xmm13
    0x45,0x0f,0x58,0xec,                            //  addps         %xmm12,%xmm13
    0x45,0x0f,0x59,0xfa,                            //  mulps         %xmm10,%xmm15
    0x45,0x0f,0x58,0xfd,                            //  addps         %xmm13,%xmm15
    0x44,0x0f,0x28,0xcb,                            //  movaps        %xmm3,%xmm9
    0x45,0x0f,0x5d,0xcf,                            //  minps         %xmm15,%xmm9
    0xf3,0x44,0x0f,0x10,0x6a,0x58,                  //  movss         0x58(%rdx),%xmm13
    0x45,0x0f,0xc6,0xed,0x00,                       //  shufps        $0x0,%xmm13,%xmm13
    0x41,0x0f,0xc2,0xc5,0x01,                       //  cmpltps       %xmm13,%xmm0
    0x44,0x0f,0x54,0xf0,                            //  andps         %xmm0,%xmm14
    0x41,0x0f,0x55,0xc1,                            //  andnps        %xmm9,%xmm0
    0x41,0x0f,0x56,0xc6,                            //  orps          %xmm14,%xmm0
    0x44,0x0f,0x52,0xc9,                            //  rsqrtps       %xmm1,%xmm9
    0x45,0x0f,0x53,0xf1,                            //  rcpps         %xmm9,%xmm14
    0x45,0x0f,0x52,0xc9,                            //  rsqrtps       %xmm9,%xmm9
    0x45,0x0f,0x59,0xf3,                            //  mulps         %xmm11,%xmm14
    0x45,0x0f,0x58,0xf4,                            //  addps         %xmm12,%xmm14
    0x45,0x0f,0x59,0xca,                            //  mulps         %xmm10,%xmm9
    0x45,0x0f,0x58,0xce,                            //  addps         %xmm14,%xmm9
    0x44,0x0f,0x28,0xf3,                            //  movaps        %xmm3,%xmm14
    0x45,0x0f,0x5d,0xf1,                            //  minps         %xmm9,%xmm14
    0x45,0x0f,0x28,0xc8,                            //  movaps        %xmm8,%xmm9
    0x44,0x0f,0x59,0xc9,                            //  mulps         %xmm1,%xmm9
    0x41,0x0f,0xc2,0xcd,0x01,                       //  cmpltps       %xmm13,%xmm1
    0x44,0x0f,0x54,0xc9,                            //  andps         %xmm1,%xmm9
    0x41,0x0f,0x55,0xce,                            //  andnps        %xmm14,%xmm1
    0x41,0x0f,0x56,0xc9,                            //  orps          %xmm9,%xmm1
    0x44,0x0f,0x52,0xca,                            //  rsqrtps       %xmm2,%xmm9
    0x45,0x0f,0x53,0xf1,                            //  rcpps         %xmm9,%xmm14
    0x45,0x0f,0x59,0xf3,                            //  mulps         %xmm11,%xmm14
    0x45,0x0f,0x58,0xf4,                            //  addps         %xmm12,%xmm14
    0x41,0x0f,0x52,0xf9,                            //  rsqrtps       %xmm9,%xmm7
    0x41,0x0f,0x59,0xfa,                            //  mulps         %xmm10,%xmm7
    0x41,0x0f,0x58,0xfe,                            //  addps         %xmm14,%xmm7
    0x0f,0x5d,0xdf,                                 //  minps         %xmm7,%xmm3
    0x44,0x0f,0x59,0xc2,                            //  mulps         %xmm2,%xmm8
    0x41,0x0f,0xc2,0xd5,0x01,                       //  cmpltps       %xmm13,%xmm2
    0x44,0x0f,0x54,0xc2,                            //  andps         %xmm2,%xmm8
    0x0f,0x55,0xd3,                                 //  andnps        %xmm3,%xmm2
    0x41,0x0f,0x56,0xd0,                            //  orps          %xmm8,%xmm2
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x0f,0x28,0xdc,                                 //  movaps        %xmm4,%xmm3
    0x0f,0x28,0xe5,                                 //  movaps        %xmm5,%xmm4
    0x0f,0x28,0xee,                                 //  movaps        %xmm6,%xmm5
    0x0f,0x28,0x34,0x24,                            //  movaps        (%rsp),%xmm6
    0x0f,0x28,0x7c,0x24,0x10,                       //  movaps        0x10(%rsp),%xmm7
    0x48,0x83,0xc4,0x28,                            //  add           $0x28,%rsp
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_scale_u8[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x00,                                 //  mov           (%rax),%rax
    0x66,0x44,0x0f,0x6e,0x04,0x38,                  //  movd          (%rax,%rdi,1),%xmm8
    0x66,0x45,0x0f,0xef,0xc9,                       //  pxor          %xmm9,%xmm9
    0x66,0x45,0x0f,0x60,0xc1,                       //  punpcklbw     %xmm9,%xmm8
    0x66,0x45,0x0f,0x61,0xc1,                       //  punpcklwd     %xmm9,%xmm8
    0x45,0x0f,0x5b,0xc0,                            //  cvtdq2ps      %xmm8,%xmm8
    0xf3,0x44,0x0f,0x10,0x4a,0x0c,                  //  movss         0xc(%rdx),%xmm9
    0x45,0x0f,0xc6,0xc9,0x00,                       //  shufps        $0x0,%xmm9,%xmm9
    0x45,0x0f,0x59,0xc8,                            //  mulps         %xmm8,%xmm9
    0x41,0x0f,0x59,0xc1,                            //  mulps         %xmm9,%xmm0
    0x41,0x0f,0x59,0xc9,                            //  mulps         %xmm9,%xmm1
    0x41,0x0f,0x59,0xd1,                            //  mulps         %xmm9,%xmm2
    0x41,0x0f,0x59,0xd9,                            //  mulps         %xmm9,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_load_tables[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x08,                                 //  mov           (%rax),%rcx
    0x4c,0x8b,0x40,0x08,                            //  mov           0x8(%rax),%r8
    0xf3,0x44,0x0f,0x6f,0x04,0xb9,                  //  movdqu        (%rcx,%rdi,4),%xmm8
    0x66,0x0f,0x6e,0x42,0x10,                       //  movd          0x10(%rdx),%xmm0
    0x66,0x0f,0x70,0xc0,0x00,                       //  pshufd        $0x0,%xmm0,%xmm0
    0x66,0x45,0x0f,0x6f,0xc8,                       //  movdqa        %xmm8,%xmm9
    0x66,0x41,0x0f,0x72,0xd1,0x08,                  //  psrld         $0x8,%xmm9
    0x66,0x44,0x0f,0xdb,0xc8,                       //  pand          %xmm0,%xmm9
    0x66,0x45,0x0f,0x6f,0xd0,                       //  movdqa        %xmm8,%xmm10
    0x66,0x41,0x0f,0x72,0xd2,0x10,                  //  psrld         $0x10,%xmm10
    0x66,0x44,0x0f,0xdb,0xd0,                       //  pand          %xmm0,%xmm10
    0x66,0x41,0x0f,0xdb,0xc0,                       //  pand          %xmm8,%xmm0
    0x66,0x0f,0x70,0xd8,0x4e,                       //  pshufd        $0x4e,%xmm0,%xmm3
    0x66,0x48,0x0f,0x7e,0xd9,                       //  movq          %xmm3,%rcx
    0x41,0x89,0xc9,                                 //  mov           %ecx,%r9d
    0x48,0xc1,0xe9,0x20,                            //  shr           $0x20,%rcx
    0x66,0x49,0x0f,0x7e,0xc2,                       //  movq          %xmm0,%r10
    0x45,0x89,0xd3,                                 //  mov           %r10d,%r11d
    0x49,0xc1,0xea,0x20,                            //  shr           $0x20,%r10
    0xf3,0x43,0x0f,0x10,0x1c,0x90,                  //  movss         (%r8,%r10,4),%xmm3
    0xf3,0x41,0x0f,0x10,0x04,0x88,                  //  movss         (%r8,%rcx,4),%xmm0
    0x0f,0x14,0xd8,                                 //  unpcklps      %xmm0,%xmm3
    0xf3,0x43,0x0f,0x10,0x04,0x98,                  //  movss         (%r8,%r11,4),%xmm0
    0xf3,0x43,0x0f,0x10,0x0c,0x88,                  //  movss         (%r8,%r9,4),%xmm1
    0x0f,0x14,0xc1,                                 //  unpcklps      %xmm1,%xmm0
    0x0f,0x14,0xc3,                                 //  unpcklps      %xmm3,%xmm0
    0x48,0x8b,0x48,0x10,                            //  mov           0x10(%rax),%rcx
    0x66,0x41,0x0f,0x70,0xc9,0x4e,                  //  pshufd        $0x4e,%xmm9,%xmm1
    0x66,0x49,0x0f,0x7e,0xc8,                       //  movq          %xmm1,%r8
    0x45,0x89,0xc1,                                 //  mov           %r8d,%r9d
    0x49,0xc1,0xe8,0x20,                            //  shr           $0x20,%r8
    0x66,0x4d,0x0f,0x7e,0xca,                       //  movq          %xmm9,%r10
    0x45,0x89,0xd3,                                 //  mov           %r10d,%r11d
    0x49,0xc1,0xea,0x20,                            //  shr           $0x20,%r10
    0xf3,0x42,0x0f,0x10,0x1c,0x91,                  //  movss         (%rcx,%r10,4),%xmm3
    0xf3,0x42,0x0f,0x10,0x0c,0x81,                  //  movss         (%rcx,%r8,4),%xmm1
    0x0f,0x14,0xd9,                                 //  unpcklps      %xmm1,%xmm3
    0xf3,0x42,0x0f,0x10,0x0c,0x99,                  //  movss         (%rcx,%r11,4),%xmm1
    0xf3,0x42,0x0f,0x10,0x14,0x89,                  //  movss         (%rcx,%r9,4),%xmm2
    0x0f,0x14,0xca,                                 //  unpcklps      %xmm2,%xmm1
    0x0f,0x14,0xcb,                                 //  unpcklps      %xmm3,%xmm1
    0x48,0x8b,0x40,0x18,                            //  mov           0x18(%rax),%rax
    0x66,0x41,0x0f,0x70,0xd2,0x4e,                  //  pshufd        $0x4e,%xmm10,%xmm2
    0x66,0x48,0x0f,0x7e,0xd1,                       //  movq          %xmm2,%rcx
    0x41,0x89,0xc8,                                 //  mov           %ecx,%r8d
    0x48,0xc1,0xe9,0x20,                            //  shr           $0x20,%rcx
    0x66,0x4d,0x0f,0x7e,0xd1,                       //  movq          %xmm10,%r9
    0x45,0x89,0xca,                                 //  mov           %r9d,%r10d
    0x49,0xc1,0xe9,0x20,                            //  shr           $0x20,%r9
    0xf3,0x46,0x0f,0x10,0x0c,0x88,                  //  movss         (%rax,%r9,4),%xmm9
    0xf3,0x0f,0x10,0x14,0x88,                       //  movss         (%rax,%rcx,4),%xmm2
    0x44,0x0f,0x14,0xca,                            //  unpcklps      %xmm2,%xmm9
    0xf3,0x42,0x0f,0x10,0x14,0x90,                  //  movss         (%rax,%r10,4),%xmm2
    0xf3,0x42,0x0f,0x10,0x1c,0x80,                  //  movss         (%rax,%r8,4),%xmm3
    0x0f,0x14,0xd3,                                 //  unpcklps      %xmm3,%xmm2
    0x41,0x0f,0x14,0xd1,                            //  unpcklps      %xmm9,%xmm2
    0x66,0x41,0x0f,0x72,0xd0,0x18,                  //  psrld         $0x18,%xmm8
    0x45,0x0f,0x5b,0xc0,                            //  cvtdq2ps      %xmm8,%xmm8
    0xf3,0x0f,0x10,0x5a,0x0c,                       //  movss         0xc(%rdx),%xmm3
    0x0f,0xc6,0xdb,0x00,                            //  shufps        $0x0,%xmm3,%xmm3
    0x41,0x0f,0x59,0xd8,                            //  mulps         %xmm8,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_load_8888[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x00,                                 //  mov           (%rax),%rax
    0xf3,0x0f,0x6f,0x1c,0xb8,                       //  movdqu        (%rax,%rdi,4),%xmm3
    0x66,0x0f,0x6e,0x42,0x10,                       //  movd          0x10(%rdx),%xmm0
    0x66,0x0f,0x70,0xc0,0x00,                       //  pshufd        $0x0,%xmm0,%xmm0
    0x66,0x0f,0x6f,0xcb,                            //  movdqa        %xmm3,%xmm1
    0x66,0x0f,0x72,0xd1,0x08,                       //  psrld         $0x8,%xmm1
    0x66,0x0f,0xdb,0xc8,                            //  pand          %xmm0,%xmm1
    0x66,0x0f,0x6f,0xd3,                            //  movdqa        %xmm3,%xmm2
    0x66,0x0f,0x72,0xd2,0x10,                       //  psrld         $0x10,%xmm2
    0x66,0x0f,0xdb,0xd0,                            //  pand          %xmm0,%xmm2
    0x66,0x0f,0xdb,0xc3,                            //  pand          %xmm3,%xmm0
    0x0f,0x5b,0xc0,                                 //  cvtdq2ps      %xmm0,%xmm0
    0xf3,0x44,0x0f,0x10,0x42,0x0c,                  //  movss         0xc(%rdx),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x41,0x0f,0x59,0xc0,                            //  mulps         %xmm8,%xmm0
    0x0f,0x5b,0xc9,                                 //  cvtdq2ps      %xmm1,%xmm1
    0x41,0x0f,0x59,0xc8,                            //  mulps         %xmm8,%xmm1
    0x0f,0x5b,0xd2,                                 //  cvtdq2ps      %xmm2,%xmm2
    0x41,0x0f,0x59,0xd0,                            //  mulps         %xmm8,%xmm2
    0x66,0x0f,0x72,0xd3,0x18,                       //  psrld         $0x18,%xmm3
    0x0f,0x5b,0xdb,                                 //  cvtdq2ps      %xmm3,%xmm3
    0x41,0x0f,0x59,0xd8,                            //  mulps         %xmm8,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_store_8888[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x00,                                 //  mov           (%rax),%rax
    0xf3,0x44,0x0f,0x10,0x42,0x08,                  //  movss         0x8(%rdx),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x45,0x0f,0x28,0xc8,                            //  movaps        %xmm8,%xmm9
    0x44,0x0f,0x59,0xc8,                            //  mulps         %xmm0,%xmm9
    0x66,0x45,0x0f,0x5b,0xc9,                       //  cvtps2dq      %xmm9,%xmm9
    0x45,0x0f,0x28,0xd0,                            //  movaps        %xmm8,%xmm10
    0x44,0x0f,0x59,0xd1,                            //  mulps         %xmm1,%xmm10
    0x66,0x45,0x0f,0x5b,0xd2,                       //  cvtps2dq      %xmm10,%xmm10
    0x66,0x41,0x0f,0x72,0xf2,0x08,                  //  pslld         $0x8,%xmm10
    0x66,0x45,0x0f,0xeb,0xd1,                       //  por           %xmm9,%xmm10
    0x45,0x0f,0x28,0xc8,                            //  movaps        %xmm8,%xmm9
    0x44,0x0f,0x59,0xca,                            //  mulps         %xmm2,%xmm9
    0x66,0x45,0x0f,0x5b,0xc9,                       //  cvtps2dq      %xmm9,%xmm9
    0x66,0x41,0x0f,0x72,0xf1,0x10,                  //  pslld         $0x10,%xmm9
    0x44,0x0f,0x59,0xc3,                            //  mulps         %xmm3,%xmm8
    0x66,0x45,0x0f,0x5b,0xc0,                       //  cvtps2dq      %xmm8,%xmm8
    0x66,0x41,0x0f,0x72,0xf0,0x18,                  //  pslld         $0x18,%xmm8
    0x66,0x45,0x0f,0xeb,0xc1,                       //  por           %xmm9,%xmm8
    0x66,0x45,0x0f,0xeb,0xc2,                       //  por           %xmm10,%xmm8
    0xf3,0x44,0x0f,0x7f,0x04,0xb8,                  //  movdqu        %xmm8,(%rax,%rdi,4)
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_load_f16[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x00,                                 //  mov           (%rax),%rax
    0xf3,0x0f,0x6f,0x04,0xf8,                       //  movdqu        (%rax,%rdi,8),%xmm0
    0xf3,0x0f,0x6f,0x4c,0xf8,0x10,                  //  movdqu        0x10(%rax,%rdi,8),%xmm1
    0x66,0x0f,0x6f,0xd0,                            //  movdqa        %xmm0,%xmm2
    0x66,0x0f,0x61,0xd1,                            //  punpcklwd     %xmm1,%xmm2
    0x66,0x0f,0x69,0xc1,                            //  punpckhwd     %xmm1,%xmm0
    0x66,0x44,0x0f,0x6f,0xc2,                       //  movdqa        %xmm2,%xmm8
    0x66,0x44,0x0f,0x61,0xc0,                       //  punpcklwd     %xmm0,%xmm8
    0x66,0x0f,0x69,0xd0,                            //  punpckhwd     %xmm0,%xmm2
    0x66,0x0f,0x6e,0x42,0x64,                       //  movd          0x64(%rdx),%xmm0
    0x66,0x0f,0x70,0xd8,0x00,                       //  pshufd        $0x0,%xmm0,%xmm3
    0x66,0x0f,0x6f,0xcb,                            //  movdqa        %xmm3,%xmm1
    0x66,0x41,0x0f,0x65,0xc8,                       //  pcmpgtw       %xmm8,%xmm1
    0x66,0x41,0x0f,0xdf,0xc8,                       //  pandn         %xmm8,%xmm1
    0x66,0x0f,0x65,0xda,                            //  pcmpgtw       %xmm2,%xmm3
    0x66,0x0f,0xdf,0xda,                            //  pandn         %xmm2,%xmm3
    0x66,0x45,0x0f,0xef,0xc0,                       //  pxor          %xmm8,%xmm8
    0x66,0x0f,0x6f,0xc1,                            //  movdqa        %xmm1,%xmm0
    0x66,0x41,0x0f,0x61,0xc0,                       //  punpcklwd     %xmm8,%xmm0
    0x66,0x0f,0x72,0xf0,0x0d,                       //  pslld         $0xd,%xmm0
    0x66,0x0f,0x6e,0x52,0x5c,                       //  movd          0x5c(%rdx),%xmm2
    0x66,0x44,0x0f,0x70,0xca,0x00,                  //  pshufd        $0x0,%xmm2,%xmm9
    0x41,0x0f,0x59,0xc1,                            //  mulps         %xmm9,%xmm0
    0x66,0x41,0x0f,0x69,0xc8,                       //  punpckhwd     %xmm8,%xmm1
    0x66,0x0f,0x72,0xf1,0x0d,                       //  pslld         $0xd,%xmm1
    0x41,0x0f,0x59,0xc9,                            //  mulps         %xmm9,%xmm1
    0x66,0x0f,0x6f,0xd3,                            //  movdqa        %xmm3,%xmm2
    0x66,0x41,0x0f,0x61,0xd0,                       //  punpcklwd     %xmm8,%xmm2
    0x66,0x0f,0x72,0xf2,0x0d,                       //  pslld         $0xd,%xmm2
    0x41,0x0f,0x59,0xd1,                            //  mulps         %xmm9,%xmm2
    0x66,0x41,0x0f,0x69,0xd8,                       //  punpckhwd     %xmm8,%xmm3
    0x66,0x0f,0x72,0xf3,0x0d,                       //  pslld         $0xd,%xmm3
    0x41,0x0f,0x59,0xd9,                            //  mulps         %xmm9,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_store_f16[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x48,0x8b,0x00,                                 //  mov           (%rax),%rax
    0x66,0x44,0x0f,0x6e,0x42,0x60,                  //  movd          0x60(%rdx),%xmm8
    0x66,0x45,0x0f,0x70,0xc0,0x00,                  //  pshufd        $0x0,%xmm8,%xmm8
    0x66,0x45,0x0f,0x6f,0xc8,                       //  movdqa        %xmm8,%xmm9
    0x44,0x0f,0x59,0xc8,                            //  mulps         %xmm0,%xmm9
    0x66,0x41,0x0f,0x72,0xd1,0x0d,                  //  psrld         $0xd,%xmm9
    0x66,0x45,0x0f,0x6f,0xd0,                       //  movdqa        %xmm8,%xmm10
    0x44,0x0f,0x59,0xd1,                            //  mulps         %xmm1,%xmm10
    0x66,0x41,0x0f,0x72,0xd2,0x0d,                  //  psrld         $0xd,%xmm10
    0x66,0x45,0x0f,0x6f,0xd8,                       //  movdqa        %xmm8,%xmm11
    0x44,0x0f,0x59,0xda,                            //  mulps         %xmm2,%xmm11
    0x66,0x41,0x0f,0x72,0xd3,0x0d,                  //  psrld         $0xd,%xmm11
    0x44,0x0f,0x59,0xc3,                            //  mulps         %xmm3,%xmm8
    0x66,0x41,0x0f,0x72,0xd0,0x0d,                  //  psrld         $0xd,%xmm8
    0x66,0x41,0x0f,0x73,0xfa,0x02,                  //  pslldq        $0x2,%xmm10
    0x66,0x45,0x0f,0xeb,0xd1,                       //  por           %xmm9,%xmm10
    0x66,0x41,0x0f,0x73,0xf8,0x02,                  //  pslldq        $0x2,%xmm8
    0x66,0x45,0x0f,0xeb,0xc3,                       //  por           %xmm11,%xmm8
    0x66,0x45,0x0f,0x6f,0xca,                       //  movdqa        %xmm10,%xmm9
    0x66,0x45,0x0f,0x62,0xc8,                       //  punpckldq     %xmm8,%xmm9
    0xf3,0x44,0x0f,0x7f,0x0c,0xf8,                  //  movdqu        %xmm9,(%rax,%rdi,8)
    0x66,0x45,0x0f,0x6a,0xd0,                       //  punpckhdq     %xmm8,%xmm10
    0xf3,0x44,0x0f,0x7f,0x54,0xf8,0x10,             //  movdqu        %xmm10,0x10(%rax,%rdi,8)
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_clamp_x[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x44,0x0f,0x10,0x00,                       //  movss         (%rax),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x66,0x45,0x0f,0x76,0xc9,                       //  pcmpeqd       %xmm9,%xmm9
    0x66,0x45,0x0f,0xfe,0xc8,                       //  paddd         %xmm8,%xmm9
    0x41,0x0f,0x5d,0xc1,                            //  minps         %xmm9,%xmm0
    0x45,0x0f,0x57,0xc0,                            //  xorps         %xmm8,%xmm8
    0x44,0x0f,0x5f,0xc0,                            //  maxps         %xmm0,%xmm8
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x41,0x0f,0x28,0xc0,                            //  movaps        %xmm8,%xmm0
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_clamp_y[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x44,0x0f,0x10,0x00,                       //  movss         (%rax),%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x66,0x45,0x0f,0x76,0xc9,                       //  pcmpeqd       %xmm9,%xmm9
    0x66,0x45,0x0f,0xfe,0xc8,                       //  paddd         %xmm8,%xmm9
    0x41,0x0f,0x5d,0xc9,                            //  minps         %xmm9,%xmm1
    0x45,0x0f,0x57,0xc0,                            //  xorps         %xmm8,%xmm8
    0x44,0x0f,0x5f,0xc1,                            //  maxps         %xmm1,%xmm8
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x41,0x0f,0x28,0xc8,                            //  movaps        %xmm8,%xmm1
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_matrix_2x3[] = {
    0x44,0x0f,0x28,0xc9,                            //  movaps        %xmm1,%xmm9
    0x44,0x0f,0x28,0xc0,                            //  movaps        %xmm0,%xmm8
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x0f,0x10,0x00,                            //  movss         (%rax),%xmm0
    0xf3,0x0f,0x10,0x48,0x04,                       //  movss         0x4(%rax),%xmm1
    0x0f,0xc6,0xc0,0x00,                            //  shufps        $0x0,%xmm0,%xmm0
    0xf3,0x44,0x0f,0x10,0x50,0x08,                  //  movss         0x8(%rax),%xmm10
    0x45,0x0f,0xc6,0xd2,0x00,                       //  shufps        $0x0,%xmm10,%xmm10
    0xf3,0x44,0x0f,0x10,0x58,0x10,                  //  movss         0x10(%rax),%xmm11
    0x45,0x0f,0xc6,0xdb,0x00,                       //  shufps        $0x0,%xmm11,%xmm11
    0x45,0x0f,0x59,0xd1,                            //  mulps         %xmm9,%xmm10
    0x45,0x0f,0x58,0xd3,                            //  addps         %xmm11,%xmm10
    0x41,0x0f,0x59,0xc0,                            //  mulps         %xmm8,%xmm0
    0x41,0x0f,0x58,0xc2,                            //  addps         %xmm10,%xmm0
    0x0f,0xc6,0xc9,0x00,                            //  shufps        $0x0,%xmm1,%xmm1
    0xf3,0x44,0x0f,0x10,0x50,0x0c,                  //  movss         0xc(%rax),%xmm10
    0x45,0x0f,0xc6,0xd2,0x00,                       //  shufps        $0x0,%xmm10,%xmm10
    0xf3,0x44,0x0f,0x10,0x58,0x14,                  //  movss         0x14(%rax),%xmm11
    0x45,0x0f,0xc6,0xdb,0x00,                       //  shufps        $0x0,%xmm11,%xmm11
    0x45,0x0f,0x59,0xd1,                            //  mulps         %xmm9,%xmm10
    0x45,0x0f,0x58,0xd3,                            //  addps         %xmm11,%xmm10
    0x41,0x0f,0x59,0xc8,                            //  mulps         %xmm8,%xmm1
    0x41,0x0f,0x58,0xca,                            //  addps         %xmm10,%xmm1
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_matrix_3x4[] = {
    0x44,0x0f,0x28,0xc9,                            //  movaps        %xmm1,%xmm9
    0x44,0x0f,0x28,0xc0,                            //  movaps        %xmm0,%xmm8
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0xf3,0x0f,0x10,0x00,                            //  movss         (%rax),%xmm0
    0xf3,0x0f,0x10,0x48,0x04,                       //  movss         0x4(%rax),%xmm1
    0x0f,0xc6,0xc0,0x00,                            //  shufps        $0x0,%xmm0,%xmm0
    0xf3,0x44,0x0f,0x10,0x50,0x0c,                  //  movss         0xc(%rax),%xmm10
    0x45,0x0f,0xc6,0xd2,0x00,                       //  shufps        $0x0,%xmm10,%xmm10
    0xf3,0x44,0x0f,0x10,0x58,0x18,                  //  movss         0x18(%rax),%xmm11
    0x45,0x0f,0xc6,0xdb,0x00,                       //  shufps        $0x0,%xmm11,%xmm11
    0xf3,0x44,0x0f,0x10,0x60,0x24,                  //  movss         0x24(%rax),%xmm12
    0x45,0x0f,0xc6,0xe4,0x00,                       //  shufps        $0x0,%xmm12,%xmm12
    0x44,0x0f,0x59,0xda,                            //  mulps         %xmm2,%xmm11
    0x45,0x0f,0x58,0xdc,                            //  addps         %xmm12,%xmm11
    0x45,0x0f,0x59,0xd1,                            //  mulps         %xmm9,%xmm10
    0x45,0x0f,0x58,0xd3,                            //  addps         %xmm11,%xmm10
    0x41,0x0f,0x59,0xc0,                            //  mulps         %xmm8,%xmm0
    0x41,0x0f,0x58,0xc2,                            //  addps         %xmm10,%xmm0
    0x0f,0xc6,0xc9,0x00,                            //  shufps        $0x0,%xmm1,%xmm1
    0xf3,0x44,0x0f,0x10,0x50,0x10,                  //  movss         0x10(%rax),%xmm10
    0x45,0x0f,0xc6,0xd2,0x00,                       //  shufps        $0x0,%xmm10,%xmm10
    0xf3,0x44,0x0f,0x10,0x58,0x1c,                  //  movss         0x1c(%rax),%xmm11
    0x45,0x0f,0xc6,0xdb,0x00,                       //  shufps        $0x0,%xmm11,%xmm11
    0xf3,0x44,0x0f,0x10,0x60,0x28,                  //  movss         0x28(%rax),%xmm12
    0x45,0x0f,0xc6,0xe4,0x00,                       //  shufps        $0x0,%xmm12,%xmm12
    0x44,0x0f,0x59,0xda,                            //  mulps         %xmm2,%xmm11
    0x45,0x0f,0x58,0xdc,                            //  addps         %xmm12,%xmm11
    0x45,0x0f,0x59,0xd1,                            //  mulps         %xmm9,%xmm10
    0x45,0x0f,0x58,0xd3,                            //  addps         %xmm11,%xmm10
    0x41,0x0f,0x59,0xc8,                            //  mulps         %xmm8,%xmm1
    0x41,0x0f,0x58,0xca,                            //  addps         %xmm10,%xmm1
    0xf3,0x44,0x0f,0x10,0x50,0x08,                  //  movss         0x8(%rax),%xmm10
    0x45,0x0f,0xc6,0xd2,0x00,                       //  shufps        $0x0,%xmm10,%xmm10
    0xf3,0x44,0x0f,0x10,0x58,0x14,                  //  movss         0x14(%rax),%xmm11
    0x45,0x0f,0xc6,0xdb,0x00,                       //  shufps        $0x0,%xmm11,%xmm11
    0xf3,0x44,0x0f,0x10,0x60,0x20,                  //  movss         0x20(%rax),%xmm12
    0x45,0x0f,0xc6,0xe4,0x00,                       //  shufps        $0x0,%xmm12,%xmm12
    0xf3,0x44,0x0f,0x10,0x68,0x2c,                  //  movss         0x2c(%rax),%xmm13
    0x45,0x0f,0xc6,0xed,0x00,                       //  shufps        $0x0,%xmm13,%xmm13
    0x44,0x0f,0x59,0xe2,                            //  mulps         %xmm2,%xmm12
    0x45,0x0f,0x58,0xe5,                            //  addps         %xmm13,%xmm12
    0x45,0x0f,0x59,0xd9,                            //  mulps         %xmm9,%xmm11
    0x45,0x0f,0x58,0xdc,                            //  addps         %xmm12,%xmm11
    0x45,0x0f,0x59,0xd0,                            //  mulps         %xmm8,%xmm10
    0x45,0x0f,0x58,0xd3,                            //  addps         %xmm11,%xmm10
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x41,0x0f,0x28,0xd2,                            //  movaps        %xmm10,%xmm2
    0xff,0xe0,                                      //  jmpq          *%rax
};
static const unsigned char sse2_sk_linear_gradient_2stops[] = {
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x44,0x0f,0x10,0x08,                            //  movups        (%rax),%xmm9
    0x0f,0x10,0x58,0x10,                            //  movups        0x10(%rax),%xmm3
    0x44,0x0f,0x28,0xc3,                            //  movaps        %xmm3,%xmm8
    0x45,0x0f,0xc6,0xc0,0x00,                       //  shufps        $0x0,%xmm8,%xmm8
    0x41,0x0f,0x28,0xc9,                            //  movaps        %xmm9,%xmm1
    0x0f,0xc6,0xc9,0x00,                            //  shufps        $0x0,%xmm1,%xmm1
    0x44,0x0f,0x59,0xc0,                            //  mulps         %xmm0,%xmm8
    0x44,0x0f,0x58,0xc1,                            //  addps         %xmm1,%xmm8
    0x0f,0x28,0xcb,                                 //  movaps        %xmm3,%xmm1
    0x0f,0xc6,0xc9,0x55,                            //  shufps        $0x55,%xmm1,%xmm1
    0x41,0x0f,0x28,0xd1,                            //  movaps        %xmm9,%xmm2
    0x0f,0xc6,0xd2,0x55,                            //  shufps        $0x55,%xmm2,%xmm2
    0x0f,0x59,0xc8,                                 //  mulps         %xmm0,%xmm1
    0x0f,0x58,0xca,                                 //  addps         %xmm2,%xmm1
    0x0f,0x28,0xd3,                                 //  movaps        %xmm3,%xmm2
    0x0f,0xc6,0xd2,0xaa,                            //  shufps        $0xaa,%xmm2,%xmm2
    0x45,0x0f,0x28,0xd1,                            //  movaps        %xmm9,%xmm10
    0x45,0x0f,0xc6,0xd2,0xaa,                       //  shufps        $0xaa,%xmm10,%xmm10
    0x0f,0x59,0xd0,                                 //  mulps         %xmm0,%xmm2
    0x41,0x0f,0x58,0xd2,                            //  addps         %xmm10,%xmm2
    0x0f,0xc6,0xdb,0xff,                            //  shufps        $0xff,%xmm3,%xmm3
    0x45,0x0f,0xc6,0xc9,0xff,                       //  shufps        $0xff,%xmm9,%xmm9
    0x0f,0x59,0xd8,                                 //  mulps         %xmm0,%xmm3
    0x41,0x0f,0x58,0xd9,                            //  addps         %xmm9,%xmm3
    0x48,0xad,                                      //  lods          %ds:(%rsi),%rax
    0x41,0x0f,0x28,0xc0,                            //  movaps        %xmm8,%xmm0
    0xff,0xe0,                                      //  jmpq          *%rax
};
#endif//SkJumper_generated_DEFINED
