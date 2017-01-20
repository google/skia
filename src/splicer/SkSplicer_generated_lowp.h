/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSplicer_generated_lowp_DEFINED
#define SkSplicer_generated_lowp_DEFINED

// This file is generated semi-automatically with this command:
//   $ src/splicer/build_stages.py

#if defined(__aarch64__)

static const unsigned int kSplice_inc_x_lowp[] = {
    0x91002000,                                 //  add           x0, x0, #0x8
};
static const unsigned int kSplice_clear_lowp[] = {
    0x6f00e400,                                 //  movi          v0.2d, #0x0
    0x6f00e401,                                 //  movi          v1.2d, #0x0
    0x6f00e402,                                 //  movi          v2.2d, #0x0
    0x6f00e403,                                 //  movi          v3.2d, #0x0
};
static const unsigned int kSplice_plus__lowp[] = {
    0x6e640c00,                                 //  uqadd         v0.8h, v0.8h, v4.8h
    0x6e650c21,                                 //  uqadd         v1.8h, v1.8h, v5.8h
    0x6e660c42,                                 //  uqadd         v2.8h, v2.8h, v6.8h
    0x6e670c63,                                 //  uqadd         v3.8h, v3.8h, v7.8h
};
static const unsigned int kSplice_srcover_lowp[] = {
    0x4d40c470,                                 //  ld1r          {v16.8h}, [x3]
    0x6e632e10,                                 //  uqsub         v16.8h, v16.8h, v3.8h
    0x6e70b491,                                 //  sqrdmulh      v17.8h, v4.8h, v16.8h
    0x4e241e12,                                 //  and           v18.16b, v16.16b, v4.16b
    0x6e70b4b3,                                 //  sqrdmulh      v19.8h, v5.8h, v16.8h
    0x4e60ba31,                                 //  abs           v17.8h, v17.8h
    0x4e251e14,                                 //  and           v20.16b, v16.16b, v5.16b
    0x6f111651,                                 //  usra          v17.8h, v18.8h, #15
    0x6e70b4d2,                                 //  sqrdmulh      v18.8h, v6.8h, v16.8h
    0x4e60ba73,                                 //  abs           v19.8h, v19.8h
    0x6f111693,                                 //  usra          v19.8h, v20.8h, #15
    0x4e261e14,                                 //  and           v20.16b, v16.16b, v6.16b
    0x4e60ba52,                                 //  abs           v18.8h, v18.8h
    0x6f111692,                                 //  usra          v18.8h, v20.8h, #15
    0x6e70b4f4,                                 //  sqrdmulh      v20.8h, v7.8h, v16.8h
    0x4e271e10,                                 //  and           v16.16b, v16.16b, v7.16b
    0x4e60ba94,                                 //  abs           v20.8h, v20.8h
    0x6f111614,                                 //  usra          v20.8h, v16.8h, #15
    0x6e600e20,                                 //  uqadd         v0.8h, v17.8h, v0.8h
    0x6e610e61,                                 //  uqadd         v1.8h, v19.8h, v1.8h
    0x6e620e42,                                 //  uqadd         v2.8h, v18.8h, v2.8h
    0x6e630e83,                                 //  uqadd         v3.8h, v20.8h, v3.8h
};
static const unsigned int kSplice_dstover_lowp[] = {
    0x4d40c470,                                 //  ld1r          {v16.8h}, [x3]
    0x6e672e10,                                 //  uqsub         v16.8h, v16.8h, v7.8h
    0x6e70b411,                                 //  sqrdmulh      v17.8h, v0.8h, v16.8h
    0x4e201e12,                                 //  and           v18.16b, v16.16b, v0.16b
    0x6e70b433,                                 //  sqrdmulh      v19.8h, v1.8h, v16.8h
    0x4e60ba31,                                 //  abs           v17.8h, v17.8h
    0x4e211e14,                                 //  and           v20.16b, v16.16b, v1.16b
    0x6f111651,                                 //  usra          v17.8h, v18.8h, #15
    0x6e70b452,                                 //  sqrdmulh      v18.8h, v2.8h, v16.8h
    0x4e60ba73,                                 //  abs           v19.8h, v19.8h
    0x6f111693,                                 //  usra          v19.8h, v20.8h, #15
    0x4e221e14,                                 //  and           v20.16b, v16.16b, v2.16b
    0x4e60ba52,                                 //  abs           v18.8h, v18.8h
    0x6f111692,                                 //  usra          v18.8h, v20.8h, #15
    0x6e70b474,                                 //  sqrdmulh      v20.8h, v3.8h, v16.8h
    0x4e231e10,                                 //  and           v16.16b, v16.16b, v3.16b
    0x4e60ba94,                                 //  abs           v20.8h, v20.8h
    0x6f111614,                                 //  usra          v20.8h, v16.8h, #15
    0x6e640e24,                                 //  uqadd         v4.8h, v17.8h, v4.8h
    0x6e650e65,                                 //  uqadd         v5.8h, v19.8h, v5.8h
    0x6e660e46,                                 //  uqadd         v6.8h, v18.8h, v6.8h
    0x6e670e87,                                 //  uqadd         v7.8h, v20.8h, v7.8h
};
static const unsigned int kSplice_clamp_1_lowp[] = {
    0x4d40c470,                                 //  ld1r          {v16.8h}, [x3]
    0x6e706c00,                                 //  umin          v0.8h, v0.8h, v16.8h
    0x6e706c21,                                 //  umin          v1.8h, v1.8h, v16.8h
    0x6e706c42,                                 //  umin          v2.8h, v2.8h, v16.8h
    0x6e706c63,                                 //  umin          v3.8h, v3.8h, v16.8h
};
static const unsigned int kSplice_clamp_a_lowp[] = {
    0x4d40c470,                                 //  ld1r          {v16.8h}, [x3]
    0x6e706c63,                                 //  umin          v3.8h, v3.8h, v16.8h
    0x6e636c00,                                 //  umin          v0.8h, v0.8h, v3.8h
    0x6e636c21,                                 //  umin          v1.8h, v1.8h, v3.8h
    0x6e636c42,                                 //  umin          v2.8h, v2.8h, v3.8h
};
static const unsigned int kSplice_swap_lowp[] = {
    0x4ea31c70,                                 //  mov           v16.16b, v3.16b
    0x4ea21c51,                                 //  mov           v17.16b, v2.16b
    0x4ea11c32,                                 //  mov           v18.16b, v1.16b
    0x4ea01c13,                                 //  mov           v19.16b, v0.16b
    0x4ea41c80,                                 //  mov           v0.16b, v4.16b
    0x4ea51ca1,                                 //  mov           v1.16b, v5.16b
    0x4ea61cc2,                                 //  mov           v2.16b, v6.16b
    0x4ea71ce3,                                 //  mov           v3.16b, v7.16b
    0x4eb31e64,                                 //  mov           v4.16b, v19.16b
    0x4eb21e45,                                 //  mov           v5.16b, v18.16b
    0x4eb11e26,                                 //  mov           v6.16b, v17.16b
    0x4eb01e07,                                 //  mov           v7.16b, v16.16b
};
static const unsigned int kSplice_move_src_dst_lowp[] = {
    0x4ea01c04,                                 //  mov           v4.16b, v0.16b
    0x4ea11c25,                                 //  mov           v5.16b, v1.16b
    0x4ea21c46,                                 //  mov           v6.16b, v2.16b
    0x4ea31c67,                                 //  mov           v7.16b, v3.16b
};
static const unsigned int kSplice_move_dst_src_lowp[] = {
    0x4ea41c80,                                 //  mov           v0.16b, v4.16b
    0x4ea51ca1,                                 //  mov           v1.16b, v5.16b
    0x4ea61cc2,                                 //  mov           v2.16b, v6.16b
    0x4ea71ce3,                                 //  mov           v3.16b, v7.16b
};
static const unsigned int kSplice_premul_lowp[] = {
    0x6e63b410,                                 //  sqrdmulh      v16.8h, v0.8h, v3.8h
    0x4e201c71,                                 //  and           v17.16b, v3.16b, v0.16b
    0x4e60ba00,                                 //  abs           v0.8h, v16.8h
    0x6e63b430,                                 //  sqrdmulh      v16.8h, v1.8h, v3.8h
    0x6f111620,                                 //  usra          v0.8h, v17.8h, #15
    0x4e211c71,                                 //  and           v17.16b, v3.16b, v1.16b
    0x4e60ba01,                                 //  abs           v1.8h, v16.8h
    0x6e63b450,                                 //  sqrdmulh      v16.8h, v2.8h, v3.8h
    0x6f111621,                                 //  usra          v1.8h, v17.8h, #15
    0x4e221c71,                                 //  and           v17.16b, v3.16b, v2.16b
    0x4e60ba02,                                 //  abs           v2.8h, v16.8h
    0x6f111622,                                 //  usra          v2.8h, v17.8h, #15
};
static const unsigned int kSplice_scale_u8_lowp[] = {
    0xf9400048,                                 //  ldr           x8, [x2]
    0xfc606910,                                 //  ldr           d16, [x8,x0]
    0x2f0fa610,                                 //  ushll         v16.8h, v16.8b, #7
    0x6f183610,                                 //  ursra         v16.8h, v16.8h, #8
    0x6e70b411,                                 //  sqrdmulh      v17.8h, v0.8h, v16.8h
    0x6e70b433,                                 //  sqrdmulh      v19.8h, v1.8h, v16.8h
    0x6e70b455,                                 //  sqrdmulh      v21.8h, v2.8h, v16.8h
    0x6e70b477,                                 //  sqrdmulh      v23.8h, v3.8h, v16.8h
    0x4e201e12,                                 //  and           v18.16b, v16.16b, v0.16b
    0x4e211e14,                                 //  and           v20.16b, v16.16b, v1.16b
    0x4e221e16,                                 //  and           v22.16b, v16.16b, v2.16b
    0x4e231e10,                                 //  and           v16.16b, v16.16b, v3.16b
    0x4e60ba20,                                 //  abs           v0.8h, v17.8h
    0x4e60ba61,                                 //  abs           v1.8h, v19.8h
    0x4e60baa2,                                 //  abs           v2.8h, v21.8h
    0x4e60bae3,                                 //  abs           v3.8h, v23.8h
    0x6f111640,                                 //  usra          v0.8h, v18.8h, #15
    0x6f111681,                                 //  usra          v1.8h, v20.8h, #15
    0x6f1116c2,                                 //  usra          v2.8h, v22.8h, #15
    0x6f111603,                                 //  usra          v3.8h, v16.8h, #15
};
static const unsigned int kSplice_load_8888_lowp[] = {
    0xf9400048,                                 //  ldr           x8, [x2]
    0x8b000908,                                 //  add           x8, x8, x0, lsl #2
    0x0c400110,                                 //  ld4           {v16.8b-v19.8b}, [x8]
    0x2f0fa600,                                 //  ushll         v0.8h, v16.8b, #7
    0x2f0fa621,                                 //  ushll         v1.8h, v17.8b, #7
    0x2f0fa642,                                 //  ushll         v2.8h, v18.8b, #7
    0x2f0fa663,                                 //  ushll         v3.8h, v19.8b, #7
    0x6f183400,                                 //  ursra         v0.8h, v0.8h, #8
    0x6f183421,                                 //  ursra         v1.8h, v1.8h, #8
    0x6f183442,                                 //  ursra         v2.8h, v2.8h, #8
    0x6f183463,                                 //  ursra         v3.8h, v3.8h, #8
};
static const unsigned int kSplice_store_8888_lowp[] = {
    0xf9400048,                                 //  ldr           x8, [x2]
    0x2f099410,                                 //  uqshrn        v16.8b, v0.8h, #7
    0x2f099431,                                 //  uqshrn        v17.8b, v1.8h, #7
    0x2f099452,                                 //  uqshrn        v18.8b, v2.8h, #7
    0x8b000908,                                 //  add           x8, x8, x0, lsl #2
    0x2f099473,                                 //  uqshrn        v19.8b, v3.8h, #7
    0x0c000110,                                 //  st4           {v16.8b-v19.8b}, [x8]
};

#elif defined(__ARM_NEON__)

static const unsigned int kSplice_inc_x_lowp[] = {
    0xe2800004,                                 //  add           r0, r0, #4
};
static const unsigned int kSplice_clear_lowp[] = {
    0xf2800010,                                 //  vmov.i32      d0, #0
    0xf2801010,                                 //  vmov.i32      d1, #0
    0xf2802010,                                 //  vmov.i32      d2, #0
    0xf2803010,                                 //  vmov.i32      d3, #0
};
static const unsigned int kSplice_plus__lowp[] = {
    0xf3100014,                                 //  vqadd.u16     d0, d0, d4
    0xf3111015,                                 //  vqadd.u16     d1, d1, d5
    0xf3122016,                                 //  vqadd.u16     d2, d2, d6
    0xf3133017,                                 //  vqadd.u16     d3, d3, d7
};
static const unsigned int kSplice_srcover_lowp[] = {
    0xf4e30c5f,                                 //  vld1.16       {d16[]}, [r3 :16]
    0xf3500293,                                 //  vqsub.u16     d16, d16, d3
    0xf3541b20,                                 //  vqrdmulh.s16  d17, d4, d16
    0xf3552b20,                                 //  vqrdmulh.s16  d18, d5, d16
    0xf3563b20,                                 //  vqrdmulh.s16  d19, d6, d16
    0xf3574b20,                                 //  vqrdmulh.s16  d20, d7, d16
    0xf2405194,                                 //  vand          d21, d16, d4
    0xf2406195,                                 //  vand          d22, d16, d5
    0xf2407196,                                 //  vand          d23, d16, d6
    0xf2400197,                                 //  vand          d16, d16, d7
    0xf3f51321,                                 //  vabs.s16      d17, d17
    0xf3f52322,                                 //  vabs.s16      d18, d18
    0xf3f53323,                                 //  vabs.s16      d19, d19
    0xf3f54324,                                 //  vabs.s16      d20, d20
    0xf3d11135,                                 //  vsra.u16      d17, d21, #15
    0xf3d12136,                                 //  vsra.u16      d18, d22, #15
    0xf3d13137,                                 //  vsra.u16      d19, d23, #15
    0xf3d14130,                                 //  vsra.u16      d20, d16, #15
    0xf3110090,                                 //  vqadd.u16     d0, d17, d0
    0xf3121091,                                 //  vqadd.u16     d1, d18, d1
    0xf3132092,                                 //  vqadd.u16     d2, d19, d2
    0xf3143093,                                 //  vqadd.u16     d3, d20, d3
};
static const unsigned int kSplice_dstover_lowp[] = {
    0xf4e30c5f,                                 //  vld1.16       {d16[]}, [r3 :16]
    0xf3500297,                                 //  vqsub.u16     d16, d16, d7
    0xf3501b20,                                 //  vqrdmulh.s16  d17, d0, d16
    0xf3512b20,                                 //  vqrdmulh.s16  d18, d1, d16
    0xf3523b20,                                 //  vqrdmulh.s16  d19, d2, d16
    0xf3534b20,                                 //  vqrdmulh.s16  d20, d3, d16
    0xf2405190,                                 //  vand          d21, d16, d0
    0xf2406191,                                 //  vand          d22, d16, d1
    0xf2407192,                                 //  vand          d23, d16, d2
    0xf2400193,                                 //  vand          d16, d16, d3
    0xf3f51321,                                 //  vabs.s16      d17, d17
    0xf3f52322,                                 //  vabs.s16      d18, d18
    0xf3f53323,                                 //  vabs.s16      d19, d19
    0xf3f54324,                                 //  vabs.s16      d20, d20
    0xf3d11135,                                 //  vsra.u16      d17, d21, #15
    0xf3d12136,                                 //  vsra.u16      d18, d22, #15
    0xf3d13137,                                 //  vsra.u16      d19, d23, #15
    0xf3d14130,                                 //  vsra.u16      d20, d16, #15
    0xf3114094,                                 //  vqadd.u16     d4, d17, d4
    0xf3125095,                                 //  vqadd.u16     d5, d18, d5
    0xf3136096,                                 //  vqadd.u16     d6, d19, d6
    0xf3147097,                                 //  vqadd.u16     d7, d20, d7
};
static const unsigned int kSplice_clamp_1_lowp[] = {
    0xf4e30c5f,                                 //  vld1.16       {d16[]}, [r3 :16]
    0xf3100630,                                 //  vmin.u16      d0, d0, d16
    0xf3111630,                                 //  vmin.u16      d1, d1, d16
    0xf3122630,                                 //  vmin.u16      d2, d2, d16
    0xf3133630,                                 //  vmin.u16      d3, d3, d16
};
static const unsigned int kSplice_clamp_a_lowp[] = {
    0xf4e30c5f,                                 //  vld1.16       {d16[]}, [r3 :16]
    0xf3133630,                                 //  vmin.u16      d3, d3, d16
    0xf3100613,                                 //  vmin.u16      d0, d0, d3
    0xf3111613,                                 //  vmin.u16      d1, d1, d3
    0xf3122613,                                 //  vmin.u16      d2, d2, d3
};
static const unsigned int kSplice_swap_lowp[] = {
    0xeef00b43,                                 //  vmov.f64      d16, d3
    0xeef01b42,                                 //  vmov.f64      d17, d2
    0xeef02b41,                                 //  vmov.f64      d18, d1
    0xeef03b40,                                 //  vmov.f64      d19, d0
    0xeeb00b44,                                 //  vmov.f64      d0, d4
    0xeeb01b45,                                 //  vmov.f64      d1, d5
    0xeeb02b46,                                 //  vmov.f64      d2, d6
    0xeeb03b47,                                 //  vmov.f64      d3, d7
    0xeeb04b63,                                 //  vmov.f64      d4, d19
    0xeeb05b62,                                 //  vmov.f64      d5, d18
    0xeeb06b61,                                 //  vmov.f64      d6, d17
    0xeeb07b60,                                 //  vmov.f64      d7, d16
};
static const unsigned int kSplice_move_src_dst_lowp[] = {
    0xeeb04b40,                                 //  vmov.f64      d4, d0
    0xeeb05b41,                                 //  vmov.f64      d5, d1
    0xeeb06b42,                                 //  vmov.f64      d6, d2
    0xeeb07b43,                                 //  vmov.f64      d7, d3
};
static const unsigned int kSplice_move_dst_src_lowp[] = {
    0xeeb00b44,                                 //  vmov.f64      d0, d4
    0xeeb01b45,                                 //  vmov.f64      d1, d5
    0xeeb02b46,                                 //  vmov.f64      d2, d6
    0xeeb03b47,                                 //  vmov.f64      d3, d7
};
static const unsigned int kSplice_premul_lowp[] = {
    0xf3500b03,                                 //  vqrdmulh.s16  d16, d0, d3
    0xf3511b03,                                 //  vqrdmulh.s16  d17, d1, d3
    0xf3522b03,                                 //  vqrdmulh.s16  d18, d2, d3
    0xf2433110,                                 //  vand          d19, d3, d0
    0xf2434111,                                 //  vand          d20, d3, d1
    0xf3b50320,                                 //  vabs.s16      d0, d16
    0xf2430112,                                 //  vand          d16, d3, d2
    0xf3b51321,                                 //  vabs.s16      d1, d17
    0xf3b52322,                                 //  vabs.s16      d2, d18
    0xf3910133,                                 //  vsra.u16      d0, d19, #15
    0xf3911134,                                 //  vsra.u16      d1, d20, #15
    0xf3912130,                                 //  vsra.u16      d2, d16, #15
};
static const unsigned int kSplice_scale_u8_lowp[] = {
    0xe592c000,                                 //  ldr           ip, [r2]
    0xe08cc000,                                 //  add           ip, ip, r0
    0xf4ec0c8f,                                 //  vld1.32       {d16[]}, [ip]
    0xf3cf0a30,                                 //  vshll.u8      q8, d16, #7
    0xf3d80370,                                 //  vrsra.u16     q8, q8, #8
    0xf3502b20,                                 //  vqrdmulh.s16  d18, d0, d16
    0xf3513b20,                                 //  vqrdmulh.s16  d19, d1, d16
    0xf3524b20,                                 //  vqrdmulh.s16  d20, d2, d16
    0xf3535b20,                                 //  vqrdmulh.s16  d21, d3, d16
    0xf2406190,                                 //  vand          d22, d16, d0
    0xf3b50322,                                 //  vabs.s16      d0, d18
    0xf2407191,                                 //  vand          d23, d16, d1
    0xf2402192,                                 //  vand          d18, d16, d2
    0xf2400193,                                 //  vand          d16, d16, d3
    0xf3b51323,                                 //  vabs.s16      d1, d19
    0xf3b52324,                                 //  vabs.s16      d2, d20
    0xf3b53325,                                 //  vabs.s16      d3, d21
    0xf3910136,                                 //  vsra.u16      d0, d22, #15
    0xf3911137,                                 //  vsra.u16      d1, d23, #15
    0xf3912132,                                 //  vsra.u16      d2, d18, #15
    0xf3913130,                                 //  vsra.u16      d3, d16, #15
};
static const unsigned int kSplice_load_8888_lowp[] = {
    0xe592c000,                                 //  ldr           ip, [r2]
    0xe08cc100,                                 //  add           ip, ip, r0, lsl #2
    0xf4ec030d,                                 //  vld4.8        {d16[0],d17[0],d18[0],d19[0]}, [ip]!
    0xf4ec032d,                                 //  vld4.8        {d16[1],d17[1],d18[1],d19[1]}, [ip]!
    0xf4ec034d,                                 //  vld4.8        {d16[2],d17[2],d18[2],d19[2]}, [ip]!
    0xf4ec036d,                                 //  vld4.8        {d16[3],d17[3],d18[3],d19[3]}, [ip]!
    0xf38f0a30,                                 //  vshll.u8      q0, d16, #7
    0xf38f2a32,                                 //  vshll.u8      q1, d18, #7
    0xf3cf0a31,                                 //  vshll.u8      q8, d17, #7
    0xf3cf2a33,                                 //  vshll.u8      q9, d19, #7
    0xf3980350,                                 //  vrsra.u16     q0, q0, #8
    0xf3d80370,                                 //  vrsra.u16     q8, q8, #8
    0xf3d82372,                                 //  vrsra.u16     q9, q9, #8
    0xf3982352,                                 //  vrsra.u16     q1, q1, #8
    0xf22011b0,                                 //  vorr          d1, d16, d16
    0xf22231b2,                                 //  vorr          d3, d18, d18
};
static const unsigned int kSplice_store_8888_lowp[] = {
    0xf2630113,                                 //  vorr          d16, d3, d3
    0xe592c000,                                 //  ldr           ip, [r2]
    0xf2612111,                                 //  vorr          d18, d1, d1
    0xf3c94910,                                 //  vqshrn.u16    d20, q0, #7
    0xe08cc100,                                 //  add           ip, ip, r0, lsl #2
    0xf3c96912,                                 //  vqshrn.u16    d22, q1, #7
    0xf3c95932,                                 //  vqshrn.u16    d21, q9, #7
    0xf3c97930,                                 //  vqshrn.u16    d23, q8, #7
    0xf4cc430d,                                 //  vst4.8        {d20[0],d21[0],d22[0],d23[0]}, [ip]!
    0xf4cc432d,                                 //  vst4.8        {d20[1],d21[1],d22[1],d23[1]}, [ip]!
    0xf4cc434d,                                 //  vst4.8        {d20[2],d21[2],d22[2],d23[2]}, [ip]!
    0xf4cc436d,                                 //  vst4.8        {d20[3],d21[3],d22[3],d23[3]}, [ip]!
};

#else

static const unsigned char kSplice_inc_x_lowp[] = {
    0x48,0x83,0xc7,0x10,                        //  add           $0x10,%rdi
};
static const unsigned char kSplice_clear_lowp[] = {
    0xc5,0xfc,0x57,0xc0,                        //  vxorps        %ymm0,%ymm0,%ymm0
    0xc5,0xf4,0x57,0xc9,                        //  vxorps        %ymm1,%ymm1,%ymm1
    0xc5,0xec,0x57,0xd2,                        //  vxorps        %ymm2,%ymm2,%ymm2
    0xc5,0xe4,0x57,0xdb,                        //  vxorps        %ymm3,%ymm3,%ymm3
};
static const unsigned char kSplice_plus__lowp[] = {
    0xc5,0xfd,0xdd,0xc4,                        //  vpaddusw      %ymm4,%ymm0,%ymm0
    0xc5,0xf5,0xdd,0xcd,                        //  vpaddusw      %ymm5,%ymm1,%ymm1
    0xc5,0xed,0xdd,0xd6,                        //  vpaddusw      %ymm6,%ymm2,%ymm2
    0xc5,0xe5,0xdd,0xdf,                        //  vpaddusw      %ymm7,%ymm3,%ymm3
};
static const unsigned char kSplice_srcover_lowp[] = {
    0xc4,0x62,0x7d,0x79,0x01,                   //  vpbroadcastw  (%rcx),%ymm8
    0xc5,0x3d,0xd9,0xc3,                        //  vpsubusw      %ymm3,%ymm8,%ymm8
    0xc4,0x42,0x5d,0x0b,0xc8,                   //  vpmulhrsw     %ymm8,%ymm4,%ymm9
    0xc4,0x42,0x7d,0x1d,0xc9,                   //  vpabsw        %ymm9,%ymm9
    0xc5,0xb5,0xdd,0xc0,                        //  vpaddusw      %ymm0,%ymm9,%ymm0
    0xc4,0x42,0x55,0x0b,0xc8,                   //  vpmulhrsw     %ymm8,%ymm5,%ymm9
    0xc4,0x42,0x7d,0x1d,0xc9,                   //  vpabsw        %ymm9,%ymm9
    0xc5,0xb5,0xdd,0xc9,                        //  vpaddusw      %ymm1,%ymm9,%ymm1
    0xc4,0x42,0x4d,0x0b,0xc8,                   //  vpmulhrsw     %ymm8,%ymm6,%ymm9
    0xc4,0x42,0x7d,0x1d,0xc9,                   //  vpabsw        %ymm9,%ymm9
    0xc5,0xb5,0xdd,0xd2,                        //  vpaddusw      %ymm2,%ymm9,%ymm2
    0xc4,0x42,0x45,0x0b,0xc0,                   //  vpmulhrsw     %ymm8,%ymm7,%ymm8
    0xc4,0x42,0x7d,0x1d,0xc0,                   //  vpabsw        %ymm8,%ymm8
    0xc5,0xbd,0xdd,0xdb,                        //  vpaddusw      %ymm3,%ymm8,%ymm3
};
static const unsigned char kSplice_dstover_lowp[] = {
    0xc4,0x62,0x7d,0x79,0x01,                   //  vpbroadcastw  (%rcx),%ymm8
    0xc5,0x3d,0xd9,0xc7,                        //  vpsubusw      %ymm7,%ymm8,%ymm8
    0xc4,0x42,0x7d,0x0b,0xc8,                   //  vpmulhrsw     %ymm8,%ymm0,%ymm9
    0xc4,0x42,0x7d,0x1d,0xc9,                   //  vpabsw        %ymm9,%ymm9
    0xc5,0xb5,0xdd,0xe4,                        //  vpaddusw      %ymm4,%ymm9,%ymm4
    0xc4,0x42,0x75,0x0b,0xc8,                   //  vpmulhrsw     %ymm8,%ymm1,%ymm9
    0xc4,0x42,0x7d,0x1d,0xc9,                   //  vpabsw        %ymm9,%ymm9
    0xc5,0xb5,0xdd,0xed,                        //  vpaddusw      %ymm5,%ymm9,%ymm5
    0xc4,0x42,0x6d,0x0b,0xc8,                   //  vpmulhrsw     %ymm8,%ymm2,%ymm9
    0xc4,0x42,0x7d,0x1d,0xc9,                   //  vpabsw        %ymm9,%ymm9
    0xc5,0xb5,0xdd,0xf6,                        //  vpaddusw      %ymm6,%ymm9,%ymm6
    0xc4,0x42,0x65,0x0b,0xc0,                   //  vpmulhrsw     %ymm8,%ymm3,%ymm8
    0xc4,0x42,0x7d,0x1d,0xc0,                   //  vpabsw        %ymm8,%ymm8
    0xc5,0xbd,0xdd,0xff,                        //  vpaddusw      %ymm7,%ymm8,%ymm7
};
static const unsigned char kSplice_clamp_1_lowp[] = {
    0xc4,0x62,0x7d,0x79,0x01,                   //  vpbroadcastw  (%rcx),%ymm8
    0xc4,0xc2,0x7d,0x3a,0xc0,                   //  vpminuw       %ymm8,%ymm0,%ymm0
    0xc4,0xc2,0x75,0x3a,0xc8,                   //  vpminuw       %ymm8,%ymm1,%ymm1
    0xc4,0xc2,0x6d,0x3a,0xd0,                   //  vpminuw       %ymm8,%ymm2,%ymm2
    0xc4,0xc2,0x65,0x3a,0xd8,                   //  vpminuw       %ymm8,%ymm3,%ymm3
};
static const unsigned char kSplice_clamp_a_lowp[] = {
    0xc4,0x62,0x7d,0x79,0x01,                   //  vpbroadcastw  (%rcx),%ymm8
    0xc4,0xc2,0x65,0x3a,0xd8,                   //  vpminuw       %ymm8,%ymm3,%ymm3
    0xc4,0xe2,0x7d,0x3a,0xc3,                   //  vpminuw       %ymm3,%ymm0,%ymm0
    0xc4,0xe2,0x75,0x3a,0xcb,                   //  vpminuw       %ymm3,%ymm1,%ymm1
    0xc4,0xe2,0x6d,0x3a,0xd3,                   //  vpminuw       %ymm3,%ymm2,%ymm2
};
static const unsigned char kSplice_swap_lowp[] = {
    0xc5,0x7c,0x28,0xc3,                        //  vmovaps       %ymm3,%ymm8
    0xc5,0x7c,0x28,0xca,                        //  vmovaps       %ymm2,%ymm9
    0xc5,0x7c,0x28,0xd1,                        //  vmovaps       %ymm1,%ymm10
    0xc5,0x7c,0x28,0xd8,                        //  vmovaps       %ymm0,%ymm11
    0xc5,0xfc,0x28,0xc4,                        //  vmovaps       %ymm4,%ymm0
    0xc5,0xfc,0x28,0xcd,                        //  vmovaps       %ymm5,%ymm1
    0xc5,0xfc,0x28,0xd6,                        //  vmovaps       %ymm6,%ymm2
    0xc5,0xfc,0x28,0xdf,                        //  vmovaps       %ymm7,%ymm3
    0xc5,0x7c,0x29,0xdc,                        //  vmovaps       %ymm11,%ymm4
    0xc5,0x7c,0x29,0xd5,                        //  vmovaps       %ymm10,%ymm5
    0xc5,0x7c,0x29,0xce,                        //  vmovaps       %ymm9,%ymm6
    0xc5,0x7c,0x29,0xc7,                        //  vmovaps       %ymm8,%ymm7
};
static const unsigned char kSplice_move_src_dst_lowp[] = {
    0xc5,0xfc,0x28,0xe0,                        //  vmovaps       %ymm0,%ymm4
    0xc5,0xfc,0x28,0xe9,                        //  vmovaps       %ymm1,%ymm5
    0xc5,0xfc,0x28,0xf2,                        //  vmovaps       %ymm2,%ymm6
    0xc5,0xfc,0x28,0xfb,                        //  vmovaps       %ymm3,%ymm7
};
static const unsigned char kSplice_move_dst_src_lowp[] = {
    0xc5,0xfc,0x28,0xc4,                        //  vmovaps       %ymm4,%ymm0
    0xc5,0xfc,0x28,0xcd,                        //  vmovaps       %ymm5,%ymm1
    0xc5,0xfc,0x28,0xd6,                        //  vmovaps       %ymm6,%ymm2
    0xc5,0xfc,0x28,0xdf,                        //  vmovaps       %ymm7,%ymm3
};
static const unsigned char kSplice_premul_lowp[] = {
    0xc4,0xe2,0x7d,0x0b,0xc3,                   //  vpmulhrsw     %ymm3,%ymm0,%ymm0
    0xc4,0xe2,0x7d,0x1d,0xc0,                   //  vpabsw        %ymm0,%ymm0
    0xc4,0xe2,0x75,0x0b,0xcb,                   //  vpmulhrsw     %ymm3,%ymm1,%ymm1
    0xc4,0xe2,0x7d,0x1d,0xc9,                   //  vpabsw        %ymm1,%ymm1
    0xc4,0xe2,0x6d,0x0b,0xd3,                   //  vpmulhrsw     %ymm3,%ymm2,%ymm2
    0xc4,0xe2,0x7d,0x1d,0xd2,                   //  vpabsw        %ymm2,%ymm2
};
static const unsigned char kSplice_scale_u8_lowp[] = {
    0x48,0x8b,0x02,                             //  mov           (%rdx),%rax
    0xc4,0x62,0x7d,0x30,0x04,0x38,              //  vpmovzxbw     (%rax,%rdi,1),%ymm8
    0xc4,0xc1,0x3d,0x71,0xf0,0x08,              //  vpsllw        $0x8,%ymm8,%ymm8
    0xc4,0x62,0x7d,0x79,0x49,0x02,              //  vpbroadcastw  0x2(%rcx),%ymm9
    0xc4,0x41,0x3d,0xe4,0xc1,                   //  vpmulhuw      %ymm9,%ymm8,%ymm8
    0xc4,0xc2,0x7d,0x0b,0xc0,                   //  vpmulhrsw     %ymm8,%ymm0,%ymm0
    0xc4,0xe2,0x7d,0x1d,0xc0,                   //  vpabsw        %ymm0,%ymm0
    0xc4,0xc2,0x75,0x0b,0xc8,                   //  vpmulhrsw     %ymm8,%ymm1,%ymm1
    0xc4,0xe2,0x7d,0x1d,0xc9,                   //  vpabsw        %ymm1,%ymm1
    0xc4,0xc2,0x6d,0x0b,0xd0,                   //  vpmulhrsw     %ymm8,%ymm2,%ymm2
    0xc4,0xe2,0x7d,0x1d,0xd2,                   //  vpabsw        %ymm2,%ymm2
    0xc4,0xc2,0x65,0x0b,0xd8,                   //  vpmulhrsw     %ymm8,%ymm3,%ymm3
    0xc4,0xe2,0x7d,0x1d,0xdb,                   //  vpabsw        %ymm3,%ymm3
};
static const unsigned char kSplice_load_8888_lowp[] = {
    0x48,0x8b,0x02,                             //  mov           (%rdx),%rax
    0xc5,0xfa,0x6f,0x04,0xb8,                   //  vmovdqu       (%rax,%rdi,4),%xmm0
    0xc5,0xfa,0x6f,0x4c,0xb8,0x10,              //  vmovdqu       0x10(%rax,%rdi,4),%xmm1
    0xc5,0xfa,0x6f,0x54,0xb8,0x20,              //  vmovdqu       0x20(%rax,%rdi,4),%xmm2
    0xc5,0xfa,0x6f,0x5c,0xb8,0x30,              //  vmovdqu       0x30(%rax,%rdi,4),%xmm3
    0xc5,0x79,0x60,0xc1,                        //  vpunpcklbw    %xmm1,%xmm0,%xmm8
    0xc5,0xf9,0x68,0xc1,                        //  vpunpckhbw    %xmm1,%xmm0,%xmm0
    0xc5,0xe9,0x60,0xcb,                        //  vpunpcklbw    %xmm3,%xmm2,%xmm1
    0xc5,0xe9,0x68,0xd3,                        //  vpunpckhbw    %xmm3,%xmm2,%xmm2
    0xc5,0xb9,0x60,0xd8,                        //  vpunpcklbw    %xmm0,%xmm8,%xmm3
    0xc5,0xb9,0x68,0xc0,                        //  vpunpckhbw    %xmm0,%xmm8,%xmm0
    0xc5,0x71,0x60,0xc2,                        //  vpunpcklbw    %xmm2,%xmm1,%xmm8
    0xc5,0xf1,0x68,0xca,                        //  vpunpckhbw    %xmm2,%xmm1,%xmm1
    0xc5,0xe1,0x60,0xd0,                        //  vpunpcklbw    %xmm0,%xmm3,%xmm2
    0xc5,0x61,0x68,0xc8,                        //  vpunpckhbw    %xmm0,%xmm3,%xmm9
    0xc5,0xb9,0x60,0xd9,                        //  vpunpcklbw    %xmm1,%xmm8,%xmm3
    0xc5,0x39,0x68,0xc1,                        //  vpunpckhbw    %xmm1,%xmm8,%xmm8
    0xc5,0xe9,0x6c,0xc3,                        //  vpunpcklqdq   %xmm3,%xmm2,%xmm0
    0xc4,0xe2,0x7d,0x30,0xc0,                   //  vpmovzxbw     %xmm0,%ymm0
    0xc5,0xfd,0x71,0xf0,0x08,                   //  vpsllw        $0x8,%ymm0,%ymm0
    0xc4,0x62,0x7d,0x79,0x51,0x02,              //  vpbroadcastw  0x2(%rcx),%ymm10
    0xc4,0xc1,0x7d,0xe4,0xc2,                   //  vpmulhuw      %ymm10,%ymm0,%ymm0
    0xc5,0xe9,0x6d,0xcb,                        //  vpunpckhqdq   %xmm3,%xmm2,%xmm1
    0xc4,0xe2,0x7d,0x30,0xc9,                   //  vpmovzxbw     %xmm1,%ymm1
    0xc5,0xf5,0x71,0xf1,0x08,                   //  vpsllw        $0x8,%ymm1,%ymm1
    0xc4,0xc1,0x75,0xe4,0xca,                   //  vpmulhuw      %ymm10,%ymm1,%ymm1
    0xc4,0xc1,0x31,0x6c,0xd0,                   //  vpunpcklqdq   %xmm8,%xmm9,%xmm2
    0xc4,0xe2,0x7d,0x30,0xd2,                   //  vpmovzxbw     %xmm2,%ymm2
    0xc5,0xed,0x71,0xf2,0x08,                   //  vpsllw        $0x8,%ymm2,%ymm2
    0xc4,0xc1,0x6d,0xe4,0xd2,                   //  vpmulhuw      %ymm10,%ymm2,%ymm2
    0xc4,0xc1,0x31,0x6d,0xd8,                   //  vpunpckhqdq   %xmm8,%xmm9,%xmm3
    0xc4,0xe2,0x7d,0x30,0xdb,                   //  vpmovzxbw     %xmm3,%ymm3
    0xc5,0xe5,0x71,0xf3,0x08,                   //  vpsllw        $0x8,%ymm3,%ymm3
    0xc4,0xc1,0x65,0xe4,0xda,                   //  vpmulhuw      %ymm10,%ymm3,%ymm3
};
static const unsigned char kSplice_store_8888_lowp[] = {
    0x48,0x8b,0x02,                             //  mov           (%rdx),%rax
    0xc5,0x7d,0xdd,0xc0,                        //  vpaddusw      %ymm0,%ymm0,%ymm8
    0xc4,0xc1,0x3d,0x71,0xd0,0x08,              //  vpsrlw        $0x8,%ymm8,%ymm8
    0xc4,0x43,0x7d,0x39,0xc1,0x01,              //  vextracti128  $0x1,%ymm8,%xmm9
    0xc4,0x41,0x39,0x67,0xc1,                   //  vpackuswb     %xmm9,%xmm8,%xmm8
    0xc5,0x75,0xdd,0xc9,                        //  vpaddusw      %ymm1,%ymm1,%ymm9
    0xc4,0xc1,0x35,0x71,0xd1,0x08,              //  vpsrlw        $0x8,%ymm9,%ymm9
    0xc4,0x43,0x7d,0x39,0xca,0x01,              //  vextracti128  $0x1,%ymm9,%xmm10
    0xc4,0x41,0x31,0x67,0xca,                   //  vpackuswb     %xmm10,%xmm9,%xmm9
    0xc5,0x6d,0xdd,0xd2,                        //  vpaddusw      %ymm2,%ymm2,%ymm10
    0xc4,0xc1,0x2d,0x71,0xd2,0x08,              //  vpsrlw        $0x8,%ymm10,%ymm10
    0xc4,0x43,0x7d,0x39,0xd3,0x01,              //  vextracti128  $0x1,%ymm10,%xmm11
    0xc4,0x41,0x29,0x67,0xd3,                   //  vpackuswb     %xmm11,%xmm10,%xmm10
    0xc5,0x65,0xdd,0xdb,                        //  vpaddusw      %ymm3,%ymm3,%ymm11
    0xc4,0xc1,0x25,0x71,0xd3,0x08,              //  vpsrlw        $0x8,%ymm11,%ymm11
    0xc4,0x43,0x7d,0x39,0xdc,0x01,              //  vextracti128  $0x1,%ymm11,%xmm12
    0xc4,0x41,0x21,0x67,0xdc,                   //  vpackuswb     %xmm12,%xmm11,%xmm11
    0xc4,0x41,0x39,0x60,0xe1,                   //  vpunpcklbw    %xmm9,%xmm8,%xmm12
    0xc4,0x41,0x39,0x68,0xc1,                   //  vpunpckhbw    %xmm9,%xmm8,%xmm8
    0xc4,0x41,0x29,0x60,0xcb,                   //  vpunpcklbw    %xmm11,%xmm10,%xmm9
    0xc4,0x41,0x29,0x68,0xd3,                   //  vpunpckhbw    %xmm11,%xmm10,%xmm10
    0xc4,0x41,0x19,0x61,0xd9,                   //  vpunpcklwd    %xmm9,%xmm12,%xmm11
    0xc5,0x7a,0x7f,0x1c,0xb8,                   //  vmovdqu       %xmm11,(%rax,%rdi,4)
    0xc4,0x41,0x19,0x69,0xc9,                   //  vpunpckhwd    %xmm9,%xmm12,%xmm9
    0xc5,0x7a,0x7f,0x4c,0xb8,0x10,              //  vmovdqu       %xmm9,0x10(%rax,%rdi,4)
    0xc4,0x41,0x39,0x61,0xca,                   //  vpunpcklwd    %xmm10,%xmm8,%xmm9
    0xc5,0x7a,0x7f,0x4c,0xb8,0x20,              //  vmovdqu       %xmm9,0x20(%rax,%rdi,4)
    0xc4,0x41,0x39,0x69,0xc2,                   //  vpunpckhwd    %xmm10,%xmm8,%xmm8
    0xc5,0x7a,0x7f,0x44,0xb8,0x30,              //  vmovdqu       %xmm8,0x30(%rax,%rdi,4)
};

#endif

#endif//SkSplicer_generated_lowp_DEFINED
