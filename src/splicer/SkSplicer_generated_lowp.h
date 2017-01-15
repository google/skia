/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSplicer_generated_DEFINED
#define SkSplicer_generated_DEFINED

// This file is generated semi-automatically with this command:
//   $ src/splicer/build_stages.py

#if defined(__aarch64__)

static const unsigned int kSplice_clear_lowp[] = {
    0x6f00e400,                                 //  movi          v0.2d, #0x0
    0x6f00e401,                                 //  movi          v1.2d, #0x0
    0x6f00e402,                                 //  movi          v2.2d, #0x0
    0x6f00e403,                                 //  movi          v3.2d, #0x0
};
static const unsigned int kSplice_plus_lowp[] = {
    0x6e640c00,                                 //  uqadd         v0.8h, v0.8h, v4.8h
    0x6e650c21,                                 //  uqadd         v1.8h, v1.8h, v5.8h
    0x6e660c42,                                 //  uqadd         v2.8h, v2.8h, v6.8h
    0x6e670c63,                                 //  uqadd         v3.8h, v3.8h, v7.8h
};
static const unsigned int kSplice_multiply_lowp[] = {
    0x6e64b410,                                 //  sqrdmulh      v16.8h, v0.8h, v4.8h
    0x4e201c91,                                 //  and           v17.16b, v4.16b, v0.16b
    0x4e60ba00,                                 //  abs           v0.8h, v16.8h
    0x6e65b430,                                 //  sqrdmulh      v16.8h, v1.8h, v5.8h
    0x6f111620,                                 //  usra          v0.8h, v17.8h, #15
    0x4e211cb1,                                 //  and           v17.16b, v5.16b, v1.16b
    0x4e60ba01,                                 //  abs           v1.8h, v16.8h
    0x6e66b450,                                 //  sqrdmulh      v16.8h, v2.8h, v6.8h
    0x6f111621,                                 //  usra          v1.8h, v17.8h, #15
    0x4e221cd1,                                 //  and           v17.16b, v6.16b, v2.16b
    0x4e60ba02,                                 //  abs           v2.8h, v16.8h
    0x6e67b470,                                 //  sqrdmulh      v16.8h, v3.8h, v7.8h
    0x6f111622,                                 //  usra          v2.8h, v17.8h, #15
    0x4e231cf1,                                 //  and           v17.16b, v7.16b, v3.16b
    0x4e60ba03,                                 //  abs           v3.8h, v16.8h
    0x6f111623,                                 //  usra          v3.8h, v17.8h, #15
};
static const unsigned int kSplice_srcover_lowp[] = {
    0x91000868,                                 //  add           x8, x3, #0x2
    0x4d40c510,                                 //  ld1r          {v16.8h}, [x8]
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
    0x91000868,                                 //  add           x8, x3, #0x2
    0x4d40c510,                                 //  ld1r          {v16.8h}, [x8]
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
static const unsigned int kSplice_clamp_0_lowp[] = {
    0x6f00e410,                                 //  movi          v16.2d, #0x0
    0x6e706400,                                 //  umax          v0.8h, v0.8h, v16.8h
    0x6e706421,                                 //  umax          v1.8h, v1.8h, v16.8h
    0x6e706442,                                 //  umax          v2.8h, v2.8h, v16.8h
    0x6e706463,                                 //  umax          v3.8h, v3.8h, v16.8h
};
static const unsigned int kSplice_clamp_1_lowp[] = {
    0x91000868,                                 //  add           x8, x3, #0x2
    0x4d40c510,                                 //  ld1r          {v16.8h}, [x8]
    0x6e706c00,                                 //  umin          v0.8h, v0.8h, v16.8h
    0x6e706c21,                                 //  umin          v1.8h, v1.8h, v16.8h
    0x6e706c42,                                 //  umin          v2.8h, v2.8h, v16.8h
    0x6e706c63,                                 //  umin          v3.8h, v3.8h, v16.8h
};
static const unsigned int kSplice_clamp_a_lowp[] = {
    0x91000868,                                 //  add           x8, x3, #0x2
    0x4d40c510,                                 //  ld1r          {v16.8h}, [x8]
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
static const unsigned int kSplice_load_8888_lowp[] = {
    0xf9400048,                                 //  ldr           x8, [x2]
    0x4f008420,                                 //  movi          v0.8h, #0x1
    0x8b000908,                                 //  add           x8, x8, x0, lsl #2
    0x0c400110,                                 //  ld4           {v16.8b-v19.8b}, [x8]
    0x2f08a601,                                 //  uxtl          v1.8h, v16.8b
    0x2f0fa602,                                 //  ushll         v2.8h, v16.8b, #7
    0x2f08a623,                                 //  uxtl          v3.8h, v17.8b
    0x2f0fa634,                                 //  ushll         v20.8h, v17.8b, #7
    0x2f0fa670,                                 //  ushll         v16.8h, v19.8b, #7
    0x6f1f0431,                                 //  ushr          v17.8h, v1.8h, #1
    0x2f08a655,                                 //  uxtl          v21.8h, v18.8b
    0x2f08a677,                                 //  uxtl          v23.8h, v19.8b
    0x6e710c42,                                 //  uqadd         v2.8h, v2.8h, v17.8h
    0x6f1f0471,                                 //  ushr          v17.8h, v3.8h, #1
    0x2f0fa656,                                 //  ushll         v22.8h, v18.8b, #7
    0x6e600c21,                                 //  uqadd         v1.8h, v1.8h, v0.8h
    0x6e600c63,                                 //  uqadd         v3.8h, v3.8h, v0.8h
    0x6e710e91,                                 //  uqadd         v17.8h, v20.8h, v17.8h
    0x6f1f06b2,                                 //  ushr          v18.8h, v21.8h, #1
    0x6e600eb3,                                 //  uqadd         v19.8h, v21.8h, v0.8h
    0x6e600ee0,                                 //  uqadd         v0.8h, v23.8h, v0.8h
    0x6f1f06f4,                                 //  ushr          v20.8h, v23.8h, #1
    0x6f180421,                                 //  ushr          v1.8h, v1.8h, #8
    0x6f180463,                                 //  ushr          v3.8h, v3.8h, #8
    0x6e720ed2,                                 //  uqadd         v18.8h, v22.8h, v18.8h
    0x6f180673,                                 //  ushr          v19.8h, v19.8h, #8
    0x6e740e10,                                 //  uqadd         v16.8h, v16.8h, v20.8h
    0x6f180414,                                 //  ushr          v20.8h, v0.8h, #8
    0x6e610c40,                                 //  uqadd         v0.8h, v2.8h, v1.8h
    0x6e630e21,                                 //  uqadd         v1.8h, v17.8h, v3.8h
    0x6e730e42,                                 //  uqadd         v2.8h, v18.8h, v19.8h
    0x6e740e03,                                 //  uqadd         v3.8h, v16.8h, v20.8h
};
static const unsigned int kSplice_store_8888_lowp[] = {
    0x6f180410,                                 //  ushr          v16.8h, v0.8h, #8
    0x6f180431,                                 //  ushr          v17.8h, v1.8h, #8
    0x6e702c10,                                 //  uqsub         v16.8h, v0.8h, v16.8h
    0xf9400048,                                 //  ldr           x8, [x2]
    0x6e712c31,                                 //  uqsub         v17.8h, v1.8h, v17.8h
    0x0f098612,                                 //  shrn          v18.8b, v16.8h, #7
    0x6f180450,                                 //  ushr          v16.8h, v2.8h, #8
    0x0f098633,                                 //  shrn          v19.8b, v17.8h, #7
    0x6e702c50,                                 //  uqsub         v16.8h, v2.8h, v16.8h
    0x0f098614,                                 //  shrn          v20.8b, v16.8h, #7
    0x6f180470,                                 //  ushr          v16.8h, v3.8h, #8
    0x6e702c70,                                 //  uqsub         v16.8h, v3.8h, v16.8h
    0x8b000908,                                 //  add           x8, x8, x0, lsl #2
    0x0f098615,                                 //  shrn          v21.8b, v16.8h, #7
    0x0c000112,                                 //  st4           {v18.8b-v21.8b}, [x8]
};

#elif defined(__ARM_NEON__)

static const unsigned int kSplice_clear_lowp[] = {
    0xe92d4bf0,                                 //  push          {r4, r5, r6, r7, r8, r9, fp, lr}
    0xe24dd040,                                 //  sub           sp, sp, #64
    0xe3a06000,                                 //  mov           r6, #0
    0xec547b17,                                 //  vmov          r7, r4, d7
    0xe58d601c,                                 //  str           r6, [sp, #28]
    0xec598b16,                                 //  vmov          r8, r9, d6
    0xe58d6018,                                 //  str           r6, [sp, #24]
    0xec5ecb15,                                 //  vmov          ip, lr, d5
    0xe58d6014,                                 //  str           r6, [sp, #20]
    0xe58d6010,                                 //  str           r6, [sp, #16]
    0xe58d600c,                                 //  str           r6, [sp, #12]
    0xe58d6008,                                 //  str           r6, [sp, #8]
    0xe58d6004,                                 //  str           r6, [sp, #4]
    0xe58d6000,                                 //  str           r6, [sp]
    0xec556b14,                                 //  vmov          r6, r5, d4
    0xe58d403c,                                 //  str           r4, [sp, #60]
    0xe28d4024,                                 //  add           r4, sp, #36
    0xe58d7038,                                 //  str           r7, [sp, #56]
    0xe58d9034,                                 //  str           r9, [sp, #52]
    0xe58d8030,                                 //  str           r8, [sp, #48]
    0xe8845020,                                 //  stm           r4, {r5, ip, lr}
    0xe58d6020,                                 //  str           r6, [sp, #32]
    0xebfffffe,                                 //  bl            0 <done>
    0xe28dd040,                                 //  add           sp, sp, #64
    0xe8bd8bf0,                                 //  pop           {r4, r5, r6, r7, r8, r9, fp, pc}
static const unsigned int kSplice_plus_lowp[] = {
    0xe92d4ff0,                                 //  push          {r4, r5, r6, r7, r8, r9, sl, fp, lr}
    0xe24dd044,                                 //  sub           sp, sp, #68
    0xec5ecb17,                                 //  vmov          ip, lr, d7
    0xec554b16,                                 //  vmov          r4, r5, d6
    0xec5bab15,                                 //  vmov          sl, fp, d5
    0xec598b14,                                 //  vmov          r8, r9, d4
    0xf3530017,                                 //  vqadd.u16     d16, d3, d7
    0xf3511015,                                 //  vqadd.u16     d17, d1, d5
    0xf3522016,                                 //  vqadd.u16     d18, d2, d6
    0xe58de03c,                                 //  str           lr, [sp, #60]
    0xe58dc038,                                 //  str           ip, [sp, #56]
    0xec5ecb30,                                 //  vmov          ip, lr, d16
    0xf3500014,                                 //  vqadd.u16     d16, d0, d4
    0xe58d5034,                                 //  str           r5, [sp, #52]
    0xe58d4030,                                 //  str           r4, [sp, #48]
    0xec545b32,                                 //  vmov          r5, r4, d18
    0xe58db02c,                                 //  str           fp, [sp, #44]
    0xe58da028,                                 //  str           sl, [sp, #40]
    0xec57ab31,                                 //  vmov          sl, r7, d17
    0xe58d9024,                                 //  str           r9, [sp, #36]
    0xe28d9014,                                 //  add           r9, sp, #20
    0xe58d8020,                                 //  str           r8, [sp, #32]
    0xec568b30,                                 //  vmov          r8, r6, d16
    0xe8895010,                                 //  stm           r9, {r4, ip, lr}
    0xe58d5010,                                 //  str           r5, [sp, #16]
    0xe58d700c,                                 //  str           r7, [sp, #12]
    0xe98d0440,                                 //  stmib         sp, {r6, sl}
    0xe58d8000,                                 //  str           r8, [sp]
    0xebfffffe,                                 //  bl            0 <done>
    0xe28dd044,                                 //  add           sp, sp, #68
    0xe8bd8ff0,                                 //  pop           {r4, r5, r6, r7, r8, r9, sl, fp, pc}
static const unsigned int kSplice_multiply_lowp[] = {
    0xe92d4ff0,                                 //  push          {r4, r5, r6, r7, r8, r9, sl, fp, lr}
    0xe24dd044,                                 //  sub           sp, sp, #68
    0xec5ecb17,                                 //  vmov          ip, lr, d7
    0xec554b16,                                 //  vmov          r4, r5, d6
    0xec5bab15,                                 //  vmov          sl, fp, d5
    0xf3530b07,                                 //  vqrdmulh.s16  d16, d3, d7
    0xf3521b06,                                 //  vqrdmulh.s16  d17, d2, d6
    0xf3504b04,                                 //  vqrdmulh.s16  d20, d0, d4
    0xf3513b05,                                 //  vqrdmulh.s16  d19, d1, d5
    0xf2472113,                                 //  vand          d18, d7, d3
    0xf3f50320,                                 //  vabs.s16      d16, d16
    0xf2465112,                                 //  vand          d21, d6, d2
    0xf2446110,                                 //  vand          d22, d4, d0
    0xf3f51321,                                 //  vabs.s16      d17, d17
    0xf3f54324,                                 //  vabs.s16      d20, d20
    0xf3d10132,                                 //  vsra.u16      d16, d18, #15
    0xf2452111,                                 //  vand          d18, d5, d1
    0xf3f53323,                                 //  vabs.s16      d19, d19
    0xf3d11135,                                 //  vsra.u16      d17, d21, #15
    0xf3d14136,                                 //  vsra.u16      d20, d22, #15
    0xf3d13132,                                 //  vsra.u16      d19, d18, #15
    0xe58de03c,                                 //  str           lr, [sp, #60]
    0xe58dc038,                                 //  str           ip, [sp, #56]
    0xec598b14,                                 //  vmov          r8, r9, d4
    0xe58d5034,                                 //  str           r5, [sp, #52]
    0xec57cb30,                                 //  vmov          ip, r7, d16
    0xe58d4030,                                 //  str           r4, [sp, #48]
    0xec54eb31,                                 //  vmov          lr, r4, d17
    0xe58db02c,                                 //  str           fp, [sp, #44]
    0xec556b34,                                 //  vmov          r6, r5, d20
    0xe58da028,                                 //  str           sl, [sp, #40]
    0xec5bab33,                                 //  vmov          sl, fp, d19
    0xe58d9024,                                 //  str           r9, [sp, #36]
    0xe58d8020,                                 //  str           r8, [sp, #32]
    0xe58d701c,                                 //  str           r7, [sp, #28]
    0xe58dc018,                                 //  str           ip, [sp, #24]
    0xe58d4014,                                 //  str           r4, [sp, #20]
    0xe98d4c20,                                 //  stmib         sp, {r5, sl, fp, lr}
    0xe58d6000,                                 //  str           r6, [sp]
    0xebfffffe,                                 //  bl            0 <done>
    0xe28dd044,                                 //  add           sp, sp, #68
    0xe8bd8ff0,                                 //  pop           {r4, r5, r6, r7, r8, r9, sl, fp, pc}
static const unsigned int kSplice_srcover_lowp[] = {
    0xe92d4ff0,                                 //  push          {r4, r5, r6, r7, r8, r9, sl, fp, lr}
    0xe24dd044,                                 //  sub           sp, sp, #68
    0xe2834002,                                 //  add           r4, r3, #2
    0xec5ecb17,                                 //  vmov          ip, lr, d7
    0xec558b16,                                 //  vmov          r8, r5, d6
    0xf4e40c5f,                                 //  vld1.16       {d16[]}, [r4 :16]
    0xec579b15,                                 //  vmov          r9, r7, d5
    0xf3500293,                                 //  vqsub.u16     d16, d16, d3
    0xf3571b20,                                 //  vqrdmulh.s16  d17, d7, d16
    0xf2402197,                                 //  vand          d18, d16, d7
    0xf3553b20,                                 //  vqrdmulh.s16  d19, d5, d16
    0xf2404196,                                 //  vand          d20, d16, d6
    0xf2405195,                                 //  vand          d21, d16, d5
    0xf3f51321,                                 //  vabs.s16      d17, d17
    0xf3d11132,                                 //  vsra.u16      d17, d18, #15
    0xf3562b20,                                 //  vqrdmulh.s16  d18, d6, d16
    0xf3f53323,                                 //  vabs.s16      d19, d19
    0xf3d13135,                                 //  vsra.u16      d19, d21, #15
    0xf3511093,                                 //  vqadd.u16     d17, d17, d3
    0xe58de03c,                                 //  str           lr, [sp, #60]
    0xec56eb14,                                 //  vmov          lr, r6, d4
    0xe58dc038,                                 //  str           ip, [sp, #56]
    0xf3f52322,                                 //  vabs.s16      d18, d18
    0xe58d5034,                                 //  str           r5, [sp, #52]
    0xe58d8030,                                 //  str           r8, [sp, #48]
    0xec5acb31,                                 //  vmov          ip, sl, d17
    0xe58d702c,                                 //  str           r7, [sp, #44]
    0xf3541b20,                                 //  vqrdmulh.s16  d17, d4, d16
    0xe58d9028,                                 //  str           r9, [sp, #40]
    0xf2400194,                                 //  vand          d16, d16, d4
    0xf3d12134,                                 //  vsra.u16      d18, d20, #15
    0xf3f51321,                                 //  vabs.s16      d17, d17
    0xf3d11130,                                 //  vsra.u16      d17, d16, #15
    0xf3520092,                                 //  vqadd.u16     d16, d18, d2
    0xf3532091,                                 //  vqadd.u16     d18, d19, d1
    0xf3511090,                                 //  vqadd.u16     d17, d17, d0
    0xec5b8b30,                                 //  vmov          r8, fp, d16
    0xe58d6024,                                 //  str           r6, [sp, #36]
    0xec546b32,                                 //  vmov          r6, r4, d18
    0xe58de020,                                 //  str           lr, [sp, #32]
    0xe28de00c,                                 //  add           lr, sp, #12
    0xec557b31,                                 //  vmov          r7, r5, d17
    0xe58da01c,                                 //  str           sl, [sp, #28]
    0xe88e1910,                                 //  stm           lr, {r4, r8, fp, ip}
    0xe98d0060,                                 //  stmib         sp, {r5, r6}
    0xe58d7000,                                 //  str           r7, [sp]
    0xebfffffe,                                 //  bl            0 <done>
    0xe28dd044,                                 //  add           sp, sp, #68
    0xe8bd8ff0,                                 //  pop           {r4, r5, r6, r7, r8, r9, sl, fp, pc}
static const unsigned int kSplice_dstover_lowp[] = {
    0xe92d4ff0,                                 //  push          {r4, r5, r6, r7, r8, r9, sl, fp, lr}
    0xe24dd044,                                 //  sub           sp, sp, #68
    0xe2834002,                                 //  add           r4, r3, #2
    0xec5ecb13,                                 //  vmov          ip, lr, d3
    0xec558b12,                                 //  vmov          r8, r5, d2
    0xf4e40c5f,                                 //  vld1.16       {d16[]}, [r4 :16]
    0xec579b11,                                 //  vmov          r9, r7, d1
    0xf3500297,                                 //  vqsub.u16     d16, d16, d7
    0xf3531b20,                                 //  vqrdmulh.s16  d17, d3, d16
    0xf2402193,                                 //  vand          d18, d16, d3
    0xf3513b20,                                 //  vqrdmulh.s16  d19, d1, d16
    0xf2404192,                                 //  vand          d20, d16, d2
    0xf2405191,                                 //  vand          d21, d16, d1
    0xf3f51321,                                 //  vabs.s16      d17, d17
    0xf3d11132,                                 //  vsra.u16      d17, d18, #15
    0xf3522b20,                                 //  vqrdmulh.s16  d18, d2, d16
    0xf3f53323,                                 //  vabs.s16      d19, d19
    0xf3d13135,                                 //  vsra.u16      d19, d21, #15
    0xf3511097,                                 //  vqadd.u16     d17, d17, d7
    0xe58de01c,                                 //  str           lr, [sp, #28]
    0xe58dc018,                                 //  str           ip, [sp, #24]
    0xf3f52322,                                 //  vabs.s16      d18, d18
    0xec56eb10,                                 //  vmov          lr, r6, d0
    0xe58d5014,                                 //  str           r5, [sp, #20]
    0xf3d12134,                                 //  vsra.u16      d18, d20, #15
    0xe58d8010,                                 //  str           r8, [sp, #16]
    0xec5acb31,                                 //  vmov          ip, sl, d17
    0xe58d700c,                                 //  str           r7, [sp, #12]
    0xf3501b20,                                 //  vqrdmulh.s16  d17, d0, d16
    0xe98d0240,                                 //  stmib         sp, {r6, r9}
    0xf2400190,                                 //  vand          d16, d16, d0
    0xf3f51321,                                 //  vabs.s16      d17, d17
    0xf3d11130,                                 //  vsra.u16      d17, d16, #15
    0xf3520096,                                 //  vqadd.u16     d16, d18, d6
    0xf3532095,                                 //  vqadd.u16     d18, d19, d5
    0xf3511094,                                 //  vqadd.u16     d17, d17, d4
    0xec546b32,                                 //  vmov          r6, r4, d18
    0xec5b8b30,                                 //  vmov          r8, fp, d16
    0xe58de000,                                 //  str           lr, [sp]
    0xe28de02c,                                 //  add           lr, sp, #44
    0xec557b31,                                 //  vmov          r7, r5, d17
    0xe58da03c,                                 //  str           sl, [sp, #60]
    0xe88e1910,                                 //  stm           lr, {r4, r8, fp, ip}
    0xe58d6028,                                 //  str           r6, [sp, #40]
    0xe58d5024,                                 //  str           r5, [sp, #36]
    0xe58d7020,                                 //  str           r7, [sp, #32]
    0xebfffffe,                                 //  bl            0 <done>
    0xe28dd044,                                 //  add           sp, sp, #68
    0xe8bd8ff0,                                 //  pop           {r4, r5, r6, r7, r8, r9, sl, fp, pc}
static const unsigned int kSplice_clamp_0_lowp[] = {
    0xe92d4ff0,                                 //  push          {r4, r5, r6, r7, r8, r9, sl, fp, lr}
    0xe24dd044,                                 //  sub           sp, sp, #68
    0xec5ecb17,                                 //  vmov          ip, lr, d7
    0xec554b16,                                 //  vmov          r4, r5, d6
    0xec5bab15,                                 //  vmov          sl, fp, d5
    0xec598b14,                                 //  vmov          r8, r9, d4
    0xf2c00010,                                 //  vmov.i32      d16, #0
    0xf3531620,                                 //  vmax.u16      d17, d3, d16
    0xf3522620,                                 //  vmax.u16      d18, d2, d16
    0xf3513620,                                 //  vmax.u16      d19, d1, d16
    0xf3500620,                                 //  vmax.u16      d16, d0, d16
    0xe58de03c,                                 //  str           lr, [sp, #60]
    0xe58dc038,                                 //  str           ip, [sp, #56]
    0xec5ecb31,                                 //  vmov          ip, lr, d17
    0xe58d5034,                                 //  str           r5, [sp, #52]
    0xe58d4030,                                 //  str           r4, [sp, #48]
    0xec545b32,                                 //  vmov          r5, r4, d18
    0xe58db02c,                                 //  str           fp, [sp, #44]
    0xe58da028,                                 //  str           sl, [sp, #40]
    0xec57ab33,                                 //  vmov          sl, r7, d19
    0xe58d9024,                                 //  str           r9, [sp, #36]
    0xe28d9014,                                 //  add           r9, sp, #20
    0xe58d8020,                                 //  str           r8, [sp, #32]
    0xec568b30,                                 //  vmov          r8, r6, d16
    0xe8895010,                                 //  stm           r9, {r4, ip, lr}
    0xe58d5010,                                 //  str           r5, [sp, #16]
    0xe58d700c,                                 //  str           r7, [sp, #12]
    0xe98d0440,                                 //  stmib         sp, {r6, sl}
    0xe58d8000,                                 //  str           r8, [sp]
    0xebfffffe,                                 //  bl            0 <done>
    0xe28dd044,                                 //  add           sp, sp, #68
    0xe8bd8ff0,                                 //  pop           {r4, r5, r6, r7, r8, r9, sl, fp, pc}
static const unsigned int kSplice_clamp_1_lowp[] = {
    0xe92d4ff0,                                 //  push          {r4, r5, r6, r7, r8, r9, sl, fp, lr}
    0xe24dd044,                                 //  sub           sp, sp, #68
    0xec5ecb17,                                 //  vmov          ip, lr, d7
    0xe2836002,                                 //  add           r6, r3, #2
    0xec558b16,                                 //  vmov          r8, r5, d6
    0xec579b15,                                 //  vmov          r9, r7, d5
    0xf4e60c5f,                                 //  vld1.16       {d16[]}, [r6 :16]
    0xf3531630,                                 //  vmin.u16      d17, d3, d16
    0xf3512630,                                 //  vmin.u16      d18, d1, d16
    0xe58de03c,                                 //  str           lr, [sp, #60]
    0xec56eb14,                                 //  vmov          lr, r6, d4
    0xe58dc038,                                 //  str           ip, [sp, #56]
    0xec5acb31,                                 //  vmov          ip, sl, d17
    0xf3521630,                                 //  vmin.u16      d17, d2, d16
    0xe58d5034,                                 //  str           r5, [sp, #52]
    0xf3500630,                                 //  vmin.u16      d16, d0, d16
    0xe58d8030,                                 //  str           r8, [sp, #48]
    0xe58d702c,                                 //  str           r7, [sp, #44]
    0xe58d9028,                                 //  str           r9, [sp, #40]
    0xec5b8b31,                                 //  vmov          r8, fp, d17
    0xec557b30,                                 //  vmov          r7, r5, d16
    0xe58d6024,                                 //  str           r6, [sp, #36]
    0xec546b32,                                 //  vmov          r6, r4, d18
    0xe58de020,                                 //  str           lr, [sp, #32]
    0xe28de00c,                                 //  add           lr, sp, #12
    0xe58da01c,                                 //  str           sl, [sp, #28]
    0xe88e1910,                                 //  stm           lr, {r4, r8, fp, ip}
    0xe98d0060,                                 //  stmib         sp, {r5, r6}
    0xe58d7000,                                 //  str           r7, [sp]
    0xebfffffe,                                 //  bl            0 <done>
    0xe28dd044,                                 //  add           sp, sp, #68
    0xe8bd8ff0,                                 //  pop           {r4, r5, r6, r7, r8, r9, sl, fp, pc}
static const unsigned int kSplice_clamp_a_lowp[] = {
    0xe92d4ff0,                                 //  push          {r4, r5, r6, r7, r8, r9, sl, fp, lr}
    0xe24dd044,                                 //  sub           sp, sp, #68
    0xec5ecb17,                                 //  vmov          ip, lr, d7
    0xe2836002,                                 //  add           r6, r3, #2
    0xec558b16,                                 //  vmov          r8, r5, d6
    0xec579b15,                                 //  vmov          r9, r7, d5
    0xf4e60c5f,                                 //  vld1.16       {d16[]}, [r6 :16]
    0xf3530630,                                 //  vmin.u16      d16, d3, d16
    0xf3521630,                                 //  vmin.u16      d17, d2, d16
    0xf3512630,                                 //  vmin.u16      d18, d1, d16
    0xe58de03c,                                 //  str           lr, [sp, #60]
    0xec56eb14,                                 //  vmov          lr, r6, d4
    0xe58dc038,                                 //  str           ip, [sp, #56]
    0xec5acb30,                                 //  vmov          ip, sl, d16
    0xf3500630,                                 //  vmin.u16      d16, d0, d16
    0xe58d5034,                                 //  str           r5, [sp, #52]
    0xe58d8030,                                 //  str           r8, [sp, #48]
    0xec5b8b31,                                 //  vmov          r8, fp, d17
    0xe58d702c,                                 //  str           r7, [sp, #44]
    0xe58d9028,                                 //  str           r9, [sp, #40]
    0xec557b30,                                 //  vmov          r7, r5, d16
    0xe58d6024,                                 //  str           r6, [sp, #36]
    0xec546b32,                                 //  vmov          r6, r4, d18
    0xe58de020,                                 //  str           lr, [sp, #32]
    0xe28de00c,                                 //  add           lr, sp, #12
    0xe58da01c,                                 //  str           sl, [sp, #28]
    0xe88e1910,                                 //  stm           lr, {r4, r8, fp, ip}
    0xe98d0060,                                 //  stmib         sp, {r5, r6}
    0xe58d7000,                                 //  str           r7, [sp]
    0xebfffffe,                                 //  bl            0 <done>
    0xe28dd044,                                 //  add           sp, sp, #68
    0xe8bd8ff0,                                 //  pop           {r4, r5, r6, r7, r8, r9, sl, fp, pc}
static const unsigned int kSplice_swap_lowp[] = {
    0xe92d4ff0,                                 //  push          {r4, r5, r6, r7, r8, r9, sl, fp, lr}
    0xe24dd044,                                 //  sub           sp, sp, #68
    0xec5ecb13,                                 //  vmov          ip, lr, d3
    0xec554b12,                                 //  vmov          r4, r5, d2
    0xec57ab11,                                 //  vmov          sl, r7, d1
    0xec598b10,                                 //  vmov          r8, r9, d0
    0xe58de03c,                                 //  str           lr, [sp, #60]
    0xe58dc038,                                 //  str           ip, [sp, #56]
    0xec5bcb17,                                 //  vmov          ip, fp, d7
    0xe58d5034,                                 //  str           r5, [sp, #52]
    0xe58d4030,                                 //  str           r4, [sp, #48]
    0xec54eb16,                                 //  vmov          lr, r4, d6
    0xe58d702c,                                 //  str           r7, [sp, #44]
    0xec576b14,                                 //  vmov          r6, r7, d4
    0xe58da028,                                 //  str           sl, [sp, #40]
    0xec55ab15,                                 //  vmov          sl, r5, d5
    0xe58d9024,                                 //  str           r9, [sp, #36]
    0xe58d8020,                                 //  str           r8, [sp, #32]
    0xe58db01c,                                 //  str           fp, [sp, #28]
    0xe58dc018,                                 //  str           ip, [sp, #24]
    0xe58d4014,                                 //  str           r4, [sp, #20]
    0xe58de010,                                 //  str           lr, [sp, #16]
    0xe58d500c,                                 //  str           r5, [sp, #12]
    0xe88d04c0,                                 //  stm           sp, {r6, r7, sl}
    0xebfffffe,                                 //  bl            0 <done>
    0xe28dd044,                                 //  add           sp, sp, #68
    0xe8bd8ff0,                                 //  pop           {r4, r5, r6, r7, r8, r9, sl, fp, pc}
static const unsigned int kSplice_move_src_dst_lowp[] = {
    0xe92d4bf0,                                 //  push          {r4, r5, r6, r7, r8, r9, fp, lr}
    0xe24dd040,                                 //  sub           sp, sp, #64
    0xec5ecb13,                                 //  vmov          ip, lr, d3
    0xe28db030,                                 //  add           fp, sp, #48
    0xec554b12,                                 //  vmov          r4, r5, d2
    0xec576b11,                                 //  vmov          r6, r7, d1
    0xec598b10,                                 //  vmov          r8, r9, d0
    0xe88b5030,                                 //  stm           fp, {r4, r5, ip, lr}
    0xe28db010,                                 //  add           fp, sp, #16
    0xe58d702c,                                 //  str           r7, [sp, #44]
    0xe58d6028,                                 //  str           r6, [sp, #40]
    0xe58d9024,                                 //  str           r9, [sp, #36]
    0xe58d8020,                                 //  str           r8, [sp, #32]
    0xe88b5030,                                 //  stm           fp, {r4, r5, ip, lr}
    0xe58d700c,                                 //  str           r7, [sp, #12]
    0xe58d6008,                                 //  str           r6, [sp, #8]
    0xe88d0300,                                 //  stm           sp, {r8, r9}
    0xebfffffe,                                 //  bl            0 <done>
    0xe28dd040,                                 //  add           sp, sp, #64
    0xe8bd8bf0,                                 //  pop           {r4, r5, r6, r7, r8, r9, fp, pc}
static const unsigned int kSplice_move_dst_src_lowp[] = {
    0xe92d4bf0,                                 //  push          {r4, r5, r6, r7, r8, r9, fp, lr}
    0xe24dd040,                                 //  sub           sp, sp, #64
    0xec5ecb17,                                 //  vmov          ip, lr, d7
    0xe28db030,                                 //  add           fp, sp, #48
    0xec554b16,                                 //  vmov          r4, r5, d6
    0xec576b15,                                 //  vmov          r6, r7, d5
    0xec598b14,                                 //  vmov          r8, r9, d4
    0xe88b5030,                                 //  stm           fp, {r4, r5, ip, lr}
    0xe28db010,                                 //  add           fp, sp, #16
    0xe58d702c,                                 //  str           r7, [sp, #44]
    0xe58d6028,                                 //  str           r6, [sp, #40]
    0xe58d9024,                                 //  str           r9, [sp, #36]
    0xe58d8020,                                 //  str           r8, [sp, #32]
    0xe88b5030,                                 //  stm           fp, {r4, r5, ip, lr}
    0xe58d700c,                                 //  str           r7, [sp, #12]
    0xe58d6008,                                 //  str           r6, [sp, #8]
    0xe88d0300,                                 //  stm           sp, {r8, r9}
    0xebfffffe,                                 //  bl            0 <done>
    0xe28dd040,                                 //  add           sp, sp, #64
    0xe8bd8bf0,                                 //  pop           {r4, r5, r6, r7, r8, r9, fp, pc}
static const unsigned int kSplice_premul_lowp[] = {
    0xe92d4ff0,                                 //  push          {r4, r5, r6, r7, r8, r9, sl, fp, lr}
    0xe24dd044,                                 //  sub           sp, sp, #68
    0xec5ecb17,                                 //  vmov          ip, lr, d7
    0xec554b16,                                 //  vmov          r4, r5, d6
    0xec5bab15,                                 //  vmov          sl, fp, d5
    0xec598b14,                                 //  vmov          r8, r9, d4
    0xf3520b03,                                 //  vqrdmulh.s16  d16, d2, d3
    0xf3511b03,                                 //  vqrdmulh.s16  d17, d1, d3
    0xf3502b03,                                 //  vqrdmulh.s16  d18, d0, d3
    0xf2433112,                                 //  vand          d19, d3, d2
    0xf2434111,                                 //  vand          d20, d3, d1
    0xf2435110,                                 //  vand          d21, d3, d0
    0xf3f50320,                                 //  vabs.s16      d16, d16
    0xf3f51321,                                 //  vabs.s16      d17, d17
    0xf3f52322,                                 //  vabs.s16      d18, d18
    0xf3d10133,                                 //  vsra.u16      d16, d19, #15
    0xf3d11134,                                 //  vsra.u16      d17, d20, #15
    0xf3d12135,                                 //  vsra.u16      d18, d21, #15
    0xe58de03c,                                 //  str           lr, [sp, #60]
    0xe58dc038,                                 //  str           ip, [sp, #56]
    0xec5ecb13,                                 //  vmov          ip, lr, d3
    0xe58d5034,                                 //  str           r5, [sp, #52]
    0xe58d4030,                                 //  str           r4, [sp, #48]
    0xec545b30,                                 //  vmov          r5, r4, d16
    0xe58db02c,                                 //  str           fp, [sp, #44]
    0xe58da028,                                 //  str           sl, [sp, #40]
    0xec57ab31,                                 //  vmov          sl, r7, d17
    0xe58d9024,                                 //  str           r9, [sp, #36]
    0xe28d9014,                                 //  add           r9, sp, #20
    0xe58d8020,                                 //  str           r8, [sp, #32]
    0xec568b32,                                 //  vmov          r8, r6, d18
    0xe8895010,                                 //  stm           r9, {r4, ip, lr}
    0xe58d5010,                                 //  str           r5, [sp, #16]
    0xe58d700c,                                 //  str           r7, [sp, #12]
    0xe98d0440,                                 //  stmib         sp, {r6, sl}
    0xe58d8000,                                 //  str           r8, [sp]
    0xebfffffe,                                 //  bl            0 <done>
    0xe28dd044,                                 //  add           sp, sp, #68
    0xe8bd8ff0,                                 //  pop           {r4, r5, r6, r7, r8, r9, sl, fp, pc}
static const unsigned int kSplice_load_8888_lowp[] = {
    0xe92d4ff0,                                 //  push          {r4, r5, r6, r7, r8, r9, sl, fp, lr}
    0xe24dd044,                                 //  sub           sp, sp, #68
    0xec5ecb17,                                 //  vmov          ip, lr, d7
    0xec554b16,                                 //  vmov          r4, r5, d6
    0xec57ab15,                                 //  vmov          sl, r7, d5
    0xec598b14,                                 //  vmov          r8, r9, d4
    0xe58de03c,                                 //  str           lr, [sp, #60]
    0xe58dc038,                                 //  str           ip, [sp, #56]
    0xec5bcb13,                                 //  vmov          ip, fp, d3
    0xe58d5034,                                 //  str           r5, [sp, #52]
    0xe58d4030,                                 //  str           r4, [sp, #48]
    0xec54eb12,                                 //  vmov          lr, r4, d2
    0xe58d702c,                                 //  str           r7, [sp, #44]
    0xec576b10,                                 //  vmov          r6, r7, d0
    0xe58da028,                                 //  str           sl, [sp, #40]
    0xec55ab11,                                 //  vmov          sl, r5, d1
    0xe58d9024,                                 //  str           r9, [sp, #36]
    0xe58d8020,                                 //  str           r8, [sp, #32]
    0xe58db01c,                                 //  str           fp, [sp, #28]
    0xe58dc018,                                 //  str           ip, [sp, #24]
    0xe58d4014,                                 //  str           r4, [sp, #20]
    0xe58de010,                                 //  str           lr, [sp, #16]
    0xe58d500c,                                 //  str           r5, [sp, #12]
    0xe88d04c0,                                 //  stm           sp, {r6, r7, sl}
    0xebfffffe,                                 //  bl            0 <done>
    0xe28dd044,                                 //  add           sp, sp, #68
    0xe8bd8ff0,                                 //  pop           {r4, r5, r6, r7, r8, r9, sl, fp, pc}
static const unsigned int kSplice_store_8888_lowp[] = {
    0xe92d4ff0,                                 //  push          {r4, r5, r6, r7, r8, r9, sl, fp, lr}
    0xe24dd044,                                 //  sub           sp, sp, #68
    0xec5ecb17,                                 //  vmov          ip, lr, d7
    0xec554b16,                                 //  vmov          r4, r5, d6
    0xec57ab15,                                 //  vmov          sl, r7, d5
    0xec598b14,                                 //  vmov          r8, r9, d4
    0xe58de03c,                                 //  str           lr, [sp, #60]
    0xe58dc038,                                 //  str           ip, [sp, #56]
    0xec5bcb13,                                 //  vmov          ip, fp, d3
    0xe58d5034,                                 //  str           r5, [sp, #52]
    0xe58d4030,                                 //  str           r4, [sp, #48]
    0xec54eb12,                                 //  vmov          lr, r4, d2
    0xe58d702c,                                 //  str           r7, [sp, #44]
    0xec576b10,                                 //  vmov          r6, r7, d0
    0xe58da028,                                 //  str           sl, [sp, #40]
    0xec55ab11,                                 //  vmov          sl, r5, d1
    0xe58d9024,                                 //  str           r9, [sp, #36]
    0xe58d8020,                                 //  str           r8, [sp, #32]
    0xe58db01c,                                 //  str           fp, [sp, #28]
    0xe58dc018,                                 //  str           ip, [sp, #24]
    0xe58d4014,                                 //  str           r4, [sp, #20]
    0xe58de010,                                 //  str           lr, [sp, #16]
    0xe58d500c,                                 //  str           r5, [sp, #12]
    0xe88d04c0,                                 //  stm           sp, {r6, r7, sl}
    0xebfffffe,                                 //  bl            0 <done>
    0xe28dd044,                                 //  add           sp, sp, #68
    0xe8bd8ff0,                                 //  pop           {r4, r5, r6, r7, r8, r9, sl, fp, pc}

#else

static const unsigned char kSplice_clear_lowp[] = {
    0xc5,0xfc,0x57,0xc0,                        //  vxorps        %ymm0,%ymm0,%ymm0
    0xc5,0xf4,0x57,0xc9,                        //  vxorps        %ymm1,%ymm1,%ymm1
    0xc5,0xec,0x57,0xd2,                        //  vxorps        %ymm2,%ymm2,%ymm2
    0xc5,0xe4,0x57,0xdb,                        //  vxorps        %ymm3,%ymm3,%ymm3
};
static const unsigned char kSplice_plus_lowp[] = {
    0xc5,0xfd,0xdd,0xc4,                        //  vpaddusw      %ymm4,%ymm0,%ymm0
    0xc5,0xf5,0xdd,0xcd,                        //  vpaddusw      %ymm5,%ymm1,%ymm1
    0xc5,0xed,0xdd,0xd6,                        //  vpaddusw      %ymm6,%ymm2,%ymm2
    0xc5,0xe5,0xdd,0xdf,                        //  vpaddusw      %ymm7,%ymm3,%ymm3
};
static const unsigned char kSplice_multiply_lowp[] = {
    0xc4,0xe2,0x7d,0x0b,0xc4,                   //  vpmulhrsw     %ymm4,%ymm0,%ymm0
    0xc4,0xe2,0x7d,0x1d,0xc0,                   //  vpabsw        %ymm0,%ymm0
    0xc4,0xe2,0x75,0x0b,0xcd,                   //  vpmulhrsw     %ymm5,%ymm1,%ymm1
    0xc4,0xe2,0x7d,0x1d,0xc9,                   //  vpabsw        %ymm1,%ymm1
    0xc4,0xe2,0x6d,0x0b,0xd6,                   //  vpmulhrsw     %ymm6,%ymm2,%ymm2
    0xc4,0xe2,0x7d,0x1d,0xd2,                   //  vpabsw        %ymm2,%ymm2
    0xc4,0xe2,0x65,0x0b,0xdf,                   //  vpmulhrsw     %ymm7,%ymm3,%ymm3
    0xc4,0xe2,0x7d,0x1d,0xdb,                   //  vpabsw        %ymm3,%ymm3
};
static const unsigned char kSplice_srcover_lowp[] = {
    0xc4,0x62,0x7d,0x79,0x41,0x02,              //  vpbroadcastw  0x2(%rcx),%ymm8
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
    0xc4,0x62,0x7d,0x79,0x41,0x02,              //  vpbroadcastw  0x2(%rcx),%ymm8
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
static const unsigned char kSplice_clamp_0_lowp[] = {
    0xc4,0x41,0x3d,0xef,0xc0,                   //  vpxor         %ymm8,%ymm8,%ymm8
    0xc4,0xc2,0x7d,0x3e,0xc0,                   //  vpmaxuw       %ymm8,%ymm0,%ymm0
    0xc4,0xc2,0x75,0x3e,0xc8,                   //  vpmaxuw       %ymm8,%ymm1,%ymm1
    0xc4,0xc2,0x6d,0x3e,0xd0,                   //  vpmaxuw       %ymm8,%ymm2,%ymm2
    0xc4,0xc2,0x65,0x3e,0xd8,                   //  vpmaxuw       %ymm8,%ymm3,%ymm3
};
static const unsigned char kSplice_clamp_1_lowp[] = {
    0xc4,0x62,0x7d,0x79,0x41,0x02,              //  vpbroadcastw  0x2(%rcx),%ymm8
    0xc4,0xc2,0x7d,0x3a,0xc0,                   //  vpminuw       %ymm8,%ymm0,%ymm0
    0xc4,0xc2,0x75,0x3a,0xc8,                   //  vpminuw       %ymm8,%ymm1,%ymm1
    0xc4,0xc2,0x6d,0x3a,0xd0,                   //  vpminuw       %ymm8,%ymm2,%ymm2
    0xc4,0xc2,0x65,0x3a,0xd8,                   //  vpminuw       %ymm8,%ymm3,%ymm3
};
static const unsigned char kSplice_clamp_a_lowp[] = {
    0xc4,0x62,0x7d,0x79,0x41,0x02,              //  vpbroadcastw  0x2(%rcx),%ymm8
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
    0xc5,0xf5,0x71,0xf0,0x07,                   //  vpsllw        $0x7,%ymm0,%ymm1
    0xc5,0xad,0x71,0xd0,0x01,                   //  vpsrlw        $0x1,%ymm0,%ymm10
    0xc4,0xc1,0x75,0xdd,0xca,                   //  vpaddusw      %ymm10,%ymm1,%ymm1
    0xc4,0x62,0x7d,0x79,0x11,                   //  vpbroadcastw  (%rcx),%ymm10
    0xc4,0xc1,0x7d,0xdd,0xc2,                   //  vpaddusw      %ymm10,%ymm0,%ymm0
    0xc5,0xfd,0x71,0xd0,0x08,                   //  vpsrlw        $0x8,%ymm0,%ymm0
    0xc5,0xf5,0xdd,0xc0,                        //  vpaddusw      %ymm0,%ymm1,%ymm0
    0xc5,0xe9,0x6d,0xcb,                        //  vpunpckhqdq   %xmm3,%xmm2,%xmm1
    0xc4,0xe2,0x7d,0x30,0xc9,                   //  vpmovzxbw     %xmm1,%ymm1
    0xc5,0xed,0x71,0xf1,0x07,                   //  vpsllw        $0x7,%ymm1,%ymm2
    0xc5,0xe5,0x71,0xd1,0x01,                   //  vpsrlw        $0x1,%ymm1,%ymm3
    0xc5,0xed,0xdd,0xd3,                        //  vpaddusw      %ymm3,%ymm2,%ymm2
    0xc4,0xc1,0x75,0xdd,0xca,                   //  vpaddusw      %ymm10,%ymm1,%ymm1
    0xc5,0xf5,0x71,0xd1,0x08,                   //  vpsrlw        $0x8,%ymm1,%ymm1
    0xc5,0xed,0xdd,0xc9,                        //  vpaddusw      %ymm1,%ymm2,%ymm1
    0xc4,0xc1,0x31,0x6c,0xd0,                   //  vpunpcklqdq   %xmm8,%xmm9,%xmm2
    0xc4,0xe2,0x7d,0x30,0xd2,                   //  vpmovzxbw     %xmm2,%ymm2
    0xc5,0xe5,0x71,0xf2,0x07,                   //  vpsllw        $0x7,%ymm2,%ymm3
    0xc5,0xa5,0x71,0xd2,0x01,                   //  vpsrlw        $0x1,%ymm2,%ymm11
    0xc4,0xc1,0x65,0xdd,0xdb,                   //  vpaddusw      %ymm11,%ymm3,%ymm3
    0xc4,0xc1,0x6d,0xdd,0xd2,                   //  vpaddusw      %ymm10,%ymm2,%ymm2
    0xc5,0xed,0x71,0xd2,0x08,                   //  vpsrlw        $0x8,%ymm2,%ymm2
    0xc5,0xe5,0xdd,0xd2,                        //  vpaddusw      %ymm2,%ymm3,%ymm2
    0xc4,0xc1,0x31,0x6d,0xd8,                   //  vpunpckhqdq   %xmm8,%xmm9,%xmm3
    0xc4,0xe2,0x7d,0x30,0xdb,                   //  vpmovzxbw     %xmm3,%ymm3
    0xc5,0xbd,0x71,0xf3,0x07,                   //  vpsllw        $0x7,%ymm3,%ymm8
    0xc5,0xb5,0x71,0xd3,0x01,                   //  vpsrlw        $0x1,%ymm3,%ymm9
    0xc4,0x41,0x3d,0xdd,0xc1,                   //  vpaddusw      %ymm9,%ymm8,%ymm8
    0xc4,0xc1,0x65,0xdd,0xda,                   //  vpaddusw      %ymm10,%ymm3,%ymm3
    0xc5,0xe5,0x71,0xd3,0x08,                   //  vpsrlw        $0x8,%ymm3,%ymm3
    0xc5,0xbd,0xdd,0xdb,                        //  vpaddusw      %ymm3,%ymm8,%ymm3
};
static const unsigned char kSplice_store_8888_lowp[] = {
    0x48,0x8b,0x02,                             //  mov           (%rdx),%rax
    0xc5,0xbd,0x71,0xd0,0x08,                   //  vpsrlw        $0x8,%ymm0,%ymm8
    0xc4,0x41,0x7d,0xd9,0xc0,                   //  vpsubusw      %ymm8,%ymm0,%ymm8
    0xc4,0xc1,0x3d,0x71,0xd0,0x07,              //  vpsrlw        $0x7,%ymm8,%ymm8
    0xc4,0x43,0x7d,0x39,0xc1,0x01,              //  vextracti128  $0x1,%ymm8,%xmm9
    0xc4,0x41,0x39,0x67,0xc1,                   //  vpackuswb     %xmm9,%xmm8,%xmm8
    0xc5,0xb5,0x71,0xd1,0x08,                   //  vpsrlw        $0x8,%ymm1,%ymm9
    0xc4,0x41,0x75,0xd9,0xc9,                   //  vpsubusw      %ymm9,%ymm1,%ymm9
    0xc4,0xc1,0x35,0x71,0xd1,0x07,              //  vpsrlw        $0x7,%ymm9,%ymm9
    0xc4,0x43,0x7d,0x39,0xca,0x01,              //  vextracti128  $0x1,%ymm9,%xmm10
    0xc4,0x41,0x31,0x67,0xca,                   //  vpackuswb     %xmm10,%xmm9,%xmm9
    0xc5,0xad,0x71,0xd2,0x08,                   //  vpsrlw        $0x8,%ymm2,%ymm10
    0xc4,0x41,0x6d,0xd9,0xd2,                   //  vpsubusw      %ymm10,%ymm2,%ymm10
    0xc4,0xc1,0x2d,0x71,0xd2,0x07,              //  vpsrlw        $0x7,%ymm10,%ymm10
    0xc4,0x43,0x7d,0x39,0xd3,0x01,              //  vextracti128  $0x1,%ymm10,%xmm11
    0xc4,0x41,0x29,0x67,0xd3,                   //  vpackuswb     %xmm11,%xmm10,%xmm10
    0xc5,0xa5,0x71,0xd3,0x08,                   //  vpsrlw        $0x8,%ymm3,%ymm11
    0xc4,0x41,0x65,0xd9,0xdb,                   //  vpsubusw      %ymm11,%ymm3,%ymm11
    0xc4,0xc1,0x25,0x71,0xd3,0x07,              //  vpsrlw        $0x7,%ymm11,%ymm11
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

#endif//SkSplicer_generated_DEFINED
