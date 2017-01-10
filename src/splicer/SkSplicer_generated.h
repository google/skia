/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSplicer_generated_DEFINED
#define SkSplicer_generated_DEFINED

// This file is generated semi-automatically with this command:
//   $ src/splicer/build_stages.py > src/splicer/SkSplicer_generated.h

#if defined(__aarch64__)

static const unsigned int kSplice_clear[] = {
    0x6f00e400,                                 //  movi          v0.2d, #0x0
    0x6f00e401,                                 //  movi          v1.2d, #0x0
    0x6f00e402,                                 //  movi          v2.2d, #0x0
    0x6f00e403,                                 //  movi          v3.2d, #0x0
};
static const unsigned int kSplice_plus[] = {
    0x4e24d400,                                 //  fadd          v0.4s, v0.4s, v4.4s
    0x4e25d421,                                 //  fadd          v1.4s, v1.4s, v5.4s
    0x4e26d442,                                 //  fadd          v2.4s, v2.4s, v6.4s
    0x4e27d463,                                 //  fadd          v3.4s, v3.4s, v7.4s
};
static const unsigned int kSplice_srcover[] = {
    0x91001068,                                 //  add           x8, x3, #0x4
    0x4d40c910,                                 //  ld1r          {v16.4s}, [x8]
    0x4ea3d610,                                 //  fsub          v16.4s, v16.4s, v3.4s
    0x4e24ce00,                                 //  fmla          v0.4s, v16.4s, v4.4s
    0x4e25ce01,                                 //  fmla          v1.4s, v16.4s, v5.4s
    0x4e26ce02,                                 //  fmla          v2.4s, v16.4s, v6.4s
    0x4e26ce03,                                 //  fmla          v3.4s, v16.4s, v6.4s
};
static const unsigned int kSplice_dstover[] = {
    0x91001068,                                 //  add           x8, x3, #0x4
    0x4d40c910,                                 //  ld1r          {v16.4s}, [x8]
    0x4ea7d610,                                 //  fsub          v16.4s, v16.4s, v7.4s
    0x4e20ce04,                                 //  fmla          v4.4s, v16.4s, v0.4s
    0x4e21ce05,                                 //  fmla          v5.4s, v16.4s, v1.4s
    0x4e22ce06,                                 //  fmla          v6.4s, v16.4s, v2.4s
    0x4e22ce07,                                 //  fmla          v7.4s, v16.4s, v2.4s
};
static const unsigned int kSplice_clamp_0[] = {
    0x6f00e410,                                 //  movi          v16.2d, #0x0
    0x4e30f400,                                 //  fmax          v0.4s, v0.4s, v16.4s
    0x4e30f421,                                 //  fmax          v1.4s, v1.4s, v16.4s
    0x4e30f442,                                 //  fmax          v2.4s, v2.4s, v16.4s
    0x4e30f463,                                 //  fmax          v3.4s, v3.4s, v16.4s
};
static const unsigned int kSplice_clamp_1[] = {
    0x91001068,                                 //  add           x8, x3, #0x4
    0x4d40c910,                                 //  ld1r          {v16.4s}, [x8]
    0x4eb0f400,                                 //  fmin          v0.4s, v0.4s, v16.4s
    0x4eb0f421,                                 //  fmin          v1.4s, v1.4s, v16.4s
    0x4eb0f442,                                 //  fmin          v2.4s, v2.4s, v16.4s
    0x4eb0f463,                                 //  fmin          v3.4s, v3.4s, v16.4s
};
static const unsigned int kSplice_clamp_a[] = {
    0x91001068,                                 //  add           x8, x3, #0x4
    0x4d40c910,                                 //  ld1r          {v16.4s}, [x8]
    0x4eb0f463,                                 //  fmin          v3.4s, v3.4s, v16.4s
    0x4ea3f400,                                 //  fmin          v0.4s, v0.4s, v3.4s
    0x4ea3f421,                                 //  fmin          v1.4s, v1.4s, v3.4s
    0x4ea3f442,                                 //  fmin          v2.4s, v2.4s, v3.4s
};
static const unsigned int kSplice_swap[] = {
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
static const unsigned int kSplice_move_src_dst[] = {
    0x4ea01c04,                                 //  mov           v4.16b, v0.16b
    0x4ea11c25,                                 //  mov           v5.16b, v1.16b
    0x4ea21c46,                                 //  mov           v6.16b, v2.16b
    0x4ea31c67,                                 //  mov           v7.16b, v3.16b
};
static const unsigned int kSplice_move_dst_src[] = {
    0x4ea41c80,                                 //  mov           v0.16b, v4.16b
    0x4ea51ca1,                                 //  mov           v1.16b, v5.16b
    0x4ea61cc2,                                 //  mov           v2.16b, v6.16b
    0x4ea71ce3,                                 //  mov           v3.16b, v7.16b
};
static const unsigned int kSplice_premul[] = {
    0x6e23dc00,                                 //  fmul          v0.4s, v0.4s, v3.4s
    0x6e23dc21,                                 //  fmul          v1.4s, v1.4s, v3.4s
    0x6e23dc42,                                 //  fmul          v2.4s, v2.4s, v3.4s
};
static const unsigned int kSplice_unpremul[] = {
    0x91001068,                                 //  add           x8, x3, #0x4
    0x4d40c910,                                 //  ld1r          {v16.4s}, [x8]
    0x4ea0d871,                                 //  fcmeq         v17.4s, v3.4s, #0.0
    0x6e23fe10,                                 //  fdiv          v16.4s, v16.4s, v3.4s
    0x4e711e10,                                 //  bic           v16.16b, v16.16b, v17.16b
    0x6e20de00,                                 //  fmul          v0.4s, v16.4s, v0.4s
    0x6e21de01,                                 //  fmul          v1.4s, v16.4s, v1.4s
    0x6e22de02,                                 //  fmul          v2.4s, v16.4s, v2.4s
};
static const unsigned int kSplice_from_srgb[] = {
    0x91005068,                                 //  add           x8, x3, #0x14
    0x4d40c910,                                 //  ld1r          {v16.4s}, [x8]
    0x91004068,                                 //  add           x8, x3, #0x10
    0x4d40c911,                                 //  ld1r          {v17.4s}, [x8]
    0x2d434c72,                                 //  ldp           s18, s19, [x3,#24]
    0x6e22dc54,                                 //  fmul          v20.4s, v2.4s, v2.4s
    0x4eb01e15,                                 //  mov           v21.16b, v16.16b
    0x4eb01e17,                                 //  mov           v23.16b, v16.16b
    0x4f921050,                                 //  fmla          v16.4s, v2.4s, v18.s[0]
    0x4eb11e36,                                 //  mov           v22.16b, v17.16b
    0x4eb11e38,                                 //  mov           v24.16b, v17.16b
    0x4e34ce11,                                 //  fmla          v17.4s, v16.4s, v20.4s
    0x6e20dc10,                                 //  fmul          v16.4s, v0.4s, v0.4s
    0x91008068,                                 //  add           x8, x3, #0x20
    0x4f921015,                                 //  fmla          v21.4s, v0.4s, v18.s[0]
    0x4e30ceb6,                                 //  fmla          v22.4s, v21.4s, v16.4s
    0x4d40c910,                                 //  ld1r          {v16.4s}, [x8]
    0x6e21dc34,                                 //  fmul          v20.4s, v1.4s, v1.4s
    0x4f921037,                                 //  fmla          v23.4s, v1.4s, v18.s[0]
    0x4f939015,                                 //  fmul          v21.4s, v0.4s, v19.s[0]
    0x4f939032,                                 //  fmul          v18.4s, v1.4s, v19.s[0]
    0x4f939053,                                 //  fmul          v19.4s, v2.4s, v19.s[0]
    0x6ea0e600,                                 //  fcmgt         v0.4s, v16.4s, v0.4s
    0x6ea1e601,                                 //  fcmgt         v1.4s, v16.4s, v1.4s
    0x6ea2e602,                                 //  fcmgt         v2.4s, v16.4s, v2.4s
    0x4e34cef8,                                 //  fmla          v24.4s, v23.4s, v20.4s
    0x6e761ea0,                                 //  bsl           v0.16b, v21.16b, v22.16b
    0x6e781e41,                                 //  bsl           v1.16b, v18.16b, v24.16b
    0x6e711e62,                                 //  bsl           v2.16b, v19.16b, v17.16b
};
static const unsigned int kSplice_to_srgb[] = {
    0x6ea1d810,                                 //  frsqrte       v16.4s, v0.4s
    0x6ea1d835,                                 //  frsqrte       v21.4s, v1.4s
    0x6e30de17,                                 //  fmul          v23.4s, v16.4s, v16.4s
    0x6ea1d856,                                 //  frsqrte       v22.4s, v2.4s
    0x6e35deb9,                                 //  fmul          v25.4s, v21.4s, v21.4s
    0x4eb7fc17,                                 //  frsqrts       v23.4s, v0.4s, v23.4s
    0x9100c068,                                 //  add           x8, x3, #0x30
    0x6e36deda,                                 //  fmul          v26.4s, v22.4s, v22.4s
    0x4eb9fc39,                                 //  frsqrts       v25.4s, v1.4s, v25.4s
    0x6e37de10,                                 //  fmul          v16.4s, v16.4s, v23.4s
    0x2d44c871,                                 //  ldp           s17, s18, [x3,#36]
    0x4d40c914,                                 //  ld1r          {v20.4s}, [x8]
    0x4ebafc5a,                                 //  frsqrts       v26.4s, v2.4s, v26.4s
    0x6e39deb5,                                 //  fmul          v21.4s, v21.4s, v25.4s
    0x4ea1da17,                                 //  frecpe        v23.4s, v16.4s
    0xbd402c73,                                 //  ldr           s19, [x3,#44]
    0x9100d068,                                 //  add           x8, x3, #0x34
    0x6e3aded6,                                 //  fmul          v22.4s, v22.4s, v26.4s
    0x4ea1dabb,                                 //  frecpe        v27.4s, v21.4s
    0x4e37fe1d,                                 //  frecps        v29.4s, v16.4s, v23.4s
    0x4d40c918,                                 //  ld1r          {v24.4s}, [x8]
    0x4ea1dadc,                                 //  frecpe        v28.4s, v22.4s
    0x6e3ddef7,                                 //  fmul          v23.4s, v23.4s, v29.4s
    0x4e3bfebd,                                 //  frecps        v29.4s, v21.4s, v27.4s
    0x6e3ddf7b,                                 //  fmul          v27.4s, v27.4s, v29.4s
    0x4e3cfedd,                                 //  frecps        v29.4s, v22.4s, v28.4s
    0x6e3ddf9c,                                 //  fmul          v28.4s, v28.4s, v29.4s
    0x4eb41e9d,                                 //  mov           v29.16b, v20.16b
    0x6ea1da19,                                 //  frsqrte       v25.4s, v16.4s
    0x4f9312fd,                                 //  fmla          v29.4s, v23.4s, v19.s[0]
    0x4eb41e97,                                 //  mov           v23.16b, v20.16b
    0x4f91901a,                                 //  fmul          v26.4s, v0.4s, v17.s[0]
    0x4f931377,                                 //  fmla          v23.4s, v27.4s, v19.s[0]
    0x6ea1dabb,                                 //  frsqrte       v27.4s, v21.4s
    0x4f931394,                                 //  fmla          v20.4s, v28.4s, v19.s[0]
    0x4f919033,                                 //  fmul          v19.4s, v1.4s, v17.s[0]
    0x4f919051,                                 //  fmul          v17.4s, v2.4s, v17.s[0]
    0x6ea0e700,                                 //  fcmgt         v0.4s, v24.4s, v0.4s
    0x6ea1e701,                                 //  fcmgt         v1.4s, v24.4s, v1.4s
    0x6ea2e702,                                 //  fcmgt         v2.4s, v24.4s, v2.4s
    0x6e39df38,                                 //  fmul          v24.4s, v25.4s, v25.4s
    0x6ea1dadc,                                 //  frsqrte       v28.4s, v22.4s
    0x4eb8fe10,                                 //  frsqrts       v16.4s, v16.4s, v24.4s
    0x6e3bdf78,                                 //  fmul          v24.4s, v27.4s, v27.4s
    0x4eb8feb5,                                 //  frsqrts       v21.4s, v21.4s, v24.4s
    0x6e3cdf98,                                 //  fmul          v24.4s, v28.4s, v28.4s
    0x91001068,                                 //  add           x8, x3, #0x4
    0x4eb8fed6,                                 //  frsqrts       v22.4s, v22.4s, v24.4s
    0x4d40c918,                                 //  ld1r          {v24.4s}, [x8]
    0x6e30df30,                                 //  fmul          v16.4s, v25.4s, v16.4s
    0x6e35df75,                                 //  fmul          v21.4s, v27.4s, v21.4s
    0x6e36df96,                                 //  fmul          v22.4s, v28.4s, v22.4s
    0x4f92121d,                                 //  fmla          v29.4s, v16.4s, v18.s[0]
    0x4f9212b7,                                 //  fmla          v23.4s, v21.4s, v18.s[0]
    0x4f9212d4,                                 //  fmla          v20.4s, v22.4s, v18.s[0]
    0x4ebdf710,                                 //  fmin          v16.4s, v24.4s, v29.4s
    0x4eb7f712,                                 //  fmin          v18.4s, v24.4s, v23.4s
    0x4eb4f714,                                 //  fmin          v20.4s, v24.4s, v20.4s
    0x6e701f40,                                 //  bsl           v0.16b, v26.16b, v16.16b
    0x6e721e61,                                 //  bsl           v1.16b, v19.16b, v18.16b
    0x6e741e22,                                 //  bsl           v2.16b, v17.16b, v20.16b
};
static const unsigned int kSplice_scale_u8[] = {
    0xf9400048,                                 //  ldr           x8, [x2]
    0xbd400c71,                                 //  ldr           s17, [x3,#12]
    0x8b000108,                                 //  add           x8, x8, x0
    0x39400109,                                 //  ldrb          w9, [x8]
    0x3940050a,                                 //  ldrb          w10, [x8,#1]
    0x4e021d30,                                 //  mov           v16.h[0], w9
    0x39400909,                                 //  ldrb          w9, [x8,#2]
    0x39400d08,                                 //  ldrb          w8, [x8,#3]
    0x4e061d50,                                 //  mov           v16.h[1], w10
    0x4e0a1d30,                                 //  mov           v16.h[2], w9
    0x4e0e1d10,                                 //  mov           v16.h[3], w8
    0x2f07b7f0,                                 //  bic           v16.4h, #0xff, lsl #8
    0x2f10a610,                                 //  uxtl          v16.4s, v16.4h
    0x6e21da10,                                 //  ucvtf         v16.4s, v16.4s
    0x4f919210,                                 //  fmul          v16.4s, v16.4s, v17.s[0]
    0x6e20de00,                                 //  fmul          v0.4s, v16.4s, v0.4s
    0x6e21de01,                                 //  fmul          v1.4s, v16.4s, v1.4s
    0x6e22de02,                                 //  fmul          v2.4s, v16.4s, v2.4s
    0x6e23de03,                                 //  fmul          v3.4s, v16.4s, v3.4s
};
static const unsigned int kSplice_load_8888[] = {
    0xf9400048,                                 //  ldr           x8, [x2]
    0xd37ef409,                                 //  lsl           x9, x0, #2
    0x4d40c860,                                 //  ld1r          {v0.4s}, [x3]
    0xbd400c63,                                 //  ldr           s3, [x3,#12]
    0x3ce96901,                                 //  ldr           q1, [x8,x9]
    0x4e211c02,                                 //  and           v2.16b, v0.16b, v1.16b
    0x6f380430,                                 //  ushr          v16.4s, v1.4s, #8
    0x6f300431,                                 //  ushr          v17.4s, v1.4s, #16
    0x6f280421,                                 //  ushr          v1.4s, v1.4s, #24
    0x4e21d842,                                 //  scvtf         v2.4s, v2.4s
    0x4e301c10,                                 //  and           v16.16b, v0.16b, v16.16b
    0x4e311c11,                                 //  and           v17.16b, v0.16b, v17.16b
    0x4e21d832,                                 //  scvtf         v18.4s, v1.4s
    0x4f839040,                                 //  fmul          v0.4s, v2.4s, v3.s[0]
    0x4e21da01,                                 //  scvtf         v1.4s, v16.4s
    0x4e21da22,                                 //  scvtf         v2.4s, v17.4s
    0x4f839021,                                 //  fmul          v1.4s, v1.4s, v3.s[0]
    0x4f839042,                                 //  fmul          v2.4s, v2.4s, v3.s[0]
    0x4f839243,                                 //  fmul          v3.4s, v18.4s, v3.s[0]
};
static const unsigned int kSplice_store_8888[] = {
    0xbd400870,                                 //  ldr           s16, [x3,#8]
    0xf9400048,                                 //  ldr           x8, [x2]
    0xd37ef409,                                 //  lsl           x9, x0, #2
    0x4f909032,                                 //  fmul          v18.4s, v1.4s, v16.s[0]
    0x4f909011,                                 //  fmul          v17.4s, v0.4s, v16.s[0]
    0x6e21aa52,                                 //  fcvtnu        v18.4s, v18.4s
    0x6e21aa31,                                 //  fcvtnu        v17.4s, v17.4s
    0x4f285652,                                 //  shl           v18.4s, v18.4s, #8
    0x4eb11e51,                                 //  orr           v17.16b, v18.16b, v17.16b
    0x4f909052,                                 //  fmul          v18.4s, v2.4s, v16.s[0]
    0x4f909070,                                 //  fmul          v16.4s, v3.4s, v16.s[0]
    0x6e21aa52,                                 //  fcvtnu        v18.4s, v18.4s
    0x6e21aa10,                                 //  fcvtnu        v16.4s, v16.4s
    0x4f305652,                                 //  shl           v18.4s, v18.4s, #16
    0x4eb21e31,                                 //  orr           v17.16b, v17.16b, v18.16b
    0x4f385610,                                 //  shl           v16.4s, v16.4s, #24
    0x4eb01e30,                                 //  orr           v16.16b, v17.16b, v16.16b
    0x3ca96910,                                 //  str           q16, [x8,x9]
};
static const unsigned int kSplice_load_f16[] = {
    0xf9400048,                                 //  ldr           x8, [x2]
    0x8b000d08,                                 //  add           x8, x8, x0, lsl #3
    0x0c400510,                                 //  ld4           {v16.4h-v19.4h}, [x8]
    0x0e217a00,                                 //  fcvtl         v0.4s, v16.4h
    0x0e217a21,                                 //  fcvtl         v1.4s, v17.4h
    0x0e217a42,                                 //  fcvtl         v2.4s, v18.4h
    0x0e217a63,                                 //  fcvtl         v3.4s, v19.4h
};
static const unsigned int kSplice_store_f16[] = {
    0xf9400048,                                 //  ldr           x8, [x2]
    0x0e216810,                                 //  fcvtn         v16.4h, v0.4s
    0x0e216831,                                 //  fcvtn         v17.4h, v1.4s
    0x0e216852,                                 //  fcvtn         v18.4h, v2.4s
    0x8b000d08,                                 //  add           x8, x8, x0, lsl #3
    0x0e216873,                                 //  fcvtn         v19.4h, v3.4s
    0x0c000510,                                 //  st4           {v16.4h-v19.4h}, [x8]
};

#else

static const unsigned char kSplice_clear[] = {
    0xc5,0xfc,0x57,0xc0,                        //  vxorps        %ymm0,%ymm0,%ymm0
    0xc5,0xf4,0x57,0xc9,                        //  vxorps        %ymm1,%ymm1,%ymm1
    0xc5,0xec,0x57,0xd2,                        //  vxorps        %ymm2,%ymm2,%ymm2
    0xc5,0xe4,0x57,0xdb,                        //  vxorps        %ymm3,%ymm3,%ymm3
};
static const unsigned char kSplice_plus[] = {
    0xc5,0xfc,0x58,0xc4,                        //  vaddps        %ymm4,%ymm0,%ymm0
    0xc5,0xf4,0x58,0xcd,                        //  vaddps        %ymm5,%ymm1,%ymm1
    0xc5,0xec,0x58,0xd6,                        //  vaddps        %ymm6,%ymm2,%ymm2
    0xc5,0xe4,0x58,0xdf,                        //  vaddps        %ymm7,%ymm3,%ymm3
};
static const unsigned char kSplice_srcover[] = {
    0xc4,0x62,0x7d,0x18,0x41,0x04,              //  vbroadcastss  0x4(%rcx),%ymm8
    0xc5,0x3c,0x5c,0xc3,                        //  vsubps        %ymm3,%ymm8,%ymm8
    0xc4,0xc2,0x5d,0xb8,0xc0,                   //  vfmadd231ps   %ymm8,%ymm4,%ymm0
    0xc4,0xc2,0x55,0xb8,0xc8,                   //  vfmadd231ps   %ymm8,%ymm5,%ymm1
    0xc4,0xc2,0x4d,0xb8,0xd0,                   //  vfmadd231ps   %ymm8,%ymm6,%ymm2
    0xc4,0xc2,0x4d,0xb8,0xd8,                   //  vfmadd231ps   %ymm8,%ymm6,%ymm3
};
static const unsigned char kSplice_dstover[] = {
    0xc4,0x62,0x7d,0x18,0x41,0x04,              //  vbroadcastss  0x4(%rcx),%ymm8
    0xc5,0x3c,0x5c,0xc7,                        //  vsubps        %ymm7,%ymm8,%ymm8
    0xc4,0xc2,0x7d,0xb8,0xe0,                   //  vfmadd231ps   %ymm8,%ymm0,%ymm4
    0xc4,0xc2,0x75,0xb8,0xe8,                   //  vfmadd231ps   %ymm8,%ymm1,%ymm5
    0xc4,0xc2,0x6d,0xb8,0xf0,                   //  vfmadd231ps   %ymm8,%ymm2,%ymm6
    0xc4,0xc2,0x6d,0xb8,0xf8,                   //  vfmadd231ps   %ymm8,%ymm2,%ymm7
};
static const unsigned char kSplice_clamp_0[] = {
    0xc4,0x41,0x3c,0x57,0xc0,                   //  vxorps        %ymm8,%ymm8,%ymm8
    0xc4,0xc1,0x7c,0x5f,0xc0,                   //  vmaxps        %ymm8,%ymm0,%ymm0
    0xc4,0xc1,0x74,0x5f,0xc8,                   //  vmaxps        %ymm8,%ymm1,%ymm1
    0xc4,0xc1,0x6c,0x5f,0xd0,                   //  vmaxps        %ymm8,%ymm2,%ymm2
    0xc4,0xc1,0x64,0x5f,0xd8,                   //  vmaxps        %ymm8,%ymm3,%ymm3
};
static const unsigned char kSplice_clamp_1[] = {
    0xc4,0x62,0x7d,0x18,0x41,0x04,              //  vbroadcastss  0x4(%rcx),%ymm8
    0xc4,0xc1,0x7c,0x5d,0xc0,                   //  vminps        %ymm8,%ymm0,%ymm0
    0xc4,0xc1,0x74,0x5d,0xc8,                   //  vminps        %ymm8,%ymm1,%ymm1
    0xc4,0xc1,0x6c,0x5d,0xd0,                   //  vminps        %ymm8,%ymm2,%ymm2
    0xc4,0xc1,0x64,0x5d,0xd8,                   //  vminps        %ymm8,%ymm3,%ymm3
};
static const unsigned char kSplice_clamp_a[] = {
    0xc4,0x62,0x7d,0x18,0x41,0x04,              //  vbroadcastss  0x4(%rcx),%ymm8
    0xc4,0xc1,0x64,0x5d,0xd8,                   //  vminps        %ymm8,%ymm3,%ymm3
    0xc5,0xfc,0x5d,0xc3,                        //  vminps        %ymm3,%ymm0,%ymm0
    0xc5,0xf4,0x5d,0xcb,                        //  vminps        %ymm3,%ymm1,%ymm1
    0xc5,0xec,0x5d,0xd3,                        //  vminps        %ymm3,%ymm2,%ymm2
};
static const unsigned char kSplice_swap[] = {
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
static const unsigned char kSplice_move_src_dst[] = {
    0xc5,0xfc,0x28,0xe0,                        //  vmovaps       %ymm0,%ymm4
    0xc5,0xfc,0x28,0xe9,                        //  vmovaps       %ymm1,%ymm5
    0xc5,0xfc,0x28,0xf2,                        //  vmovaps       %ymm2,%ymm6
    0xc5,0xfc,0x28,0xfb,                        //  vmovaps       %ymm3,%ymm7
};
static const unsigned char kSplice_move_dst_src[] = {
    0xc5,0xfc,0x28,0xc4,                        //  vmovaps       %ymm4,%ymm0
    0xc5,0xfc,0x28,0xcd,                        //  vmovaps       %ymm5,%ymm1
    0xc5,0xfc,0x28,0xd6,                        //  vmovaps       %ymm6,%ymm2
    0xc5,0xfc,0x28,0xdf,                        //  vmovaps       %ymm7,%ymm3
};
static const unsigned char kSplice_premul[] = {
    0xc5,0xfc,0x59,0xc3,                        //  vmulps        %ymm3,%ymm0,%ymm0
    0xc5,0xf4,0x59,0xcb,                        //  vmulps        %ymm3,%ymm1,%ymm1
    0xc5,0xec,0x59,0xd3,                        //  vmulps        %ymm3,%ymm2,%ymm2
};
static const unsigned char kSplice_unpremul[] = {
    0xc4,0x41,0x3c,0x57,0xc0,                   //  vxorps        %ymm8,%ymm8,%ymm8
    0xc4,0x41,0x64,0xc2,0xc8,0x00,              //  vcmpeqps      %ymm8,%ymm3,%ymm9
    0xc4,0x62,0x7d,0x18,0x51,0x04,              //  vbroadcastss  0x4(%rcx),%ymm10
    0xc5,0x2c,0x5e,0xd3,                        //  vdivps        %ymm3,%ymm10,%ymm10
    0xc4,0x43,0x2d,0x4a,0xc0,0x90,              //  vblendvps     %ymm9,%ymm8,%ymm10,%ymm8
    0xc5,0xbc,0x59,0xc0,                        //  vmulps        %ymm0,%ymm8,%ymm0
    0xc5,0xbc,0x59,0xc9,                        //  vmulps        %ymm1,%ymm8,%ymm1
    0xc5,0xbc,0x59,0xd2,                        //  vmulps        %ymm2,%ymm8,%ymm2
};
static const unsigned char kSplice_from_srgb[] = {
    0xc4,0x62,0x7d,0x18,0x41,0x1c,              //  vbroadcastss  0x1c(%rcx),%ymm8
    0xc5,0x3c,0x59,0xc8,                        //  vmulps        %ymm0,%ymm8,%ymm9
    0xc5,0x7c,0x59,0xd0,                        //  vmulps        %ymm0,%ymm0,%ymm10
    0xc4,0x62,0x7d,0x18,0x59,0x18,              //  vbroadcastss  0x18(%rcx),%ymm11
    0xc4,0x62,0x7d,0x18,0x61,0x14,              //  vbroadcastss  0x14(%rcx),%ymm12
    0xc4,0x41,0x7c,0x28,0xeb,                   //  vmovaps       %ymm11,%ymm13
    0xc4,0x42,0x7d,0xa8,0xec,                   //  vfmadd213ps   %ymm12,%ymm0,%ymm13
    0xc4,0x62,0x7d,0x18,0x71,0x10,              //  vbroadcastss  0x10(%rcx),%ymm14
    0xc4,0x42,0x2d,0xa8,0xee,                   //  vfmadd213ps   %ymm14,%ymm10,%ymm13
    0xc4,0x62,0x7d,0x18,0x51,0x20,              //  vbroadcastss  0x20(%rcx),%ymm10
    0xc4,0xc1,0x7c,0xc2,0xc2,0x01,              //  vcmpltps      %ymm10,%ymm0,%ymm0
    0xc4,0xc3,0x15,0x4a,0xc1,0x00,              //  vblendvps     %ymm0,%ymm9,%ymm13,%ymm0
    0xc5,0x3c,0x59,0xc9,                        //  vmulps        %ymm1,%ymm8,%ymm9
    0xc5,0x74,0x59,0xe9,                        //  vmulps        %ymm1,%ymm1,%ymm13
    0xc4,0x41,0x7c,0x28,0xfb,                   //  vmovaps       %ymm11,%ymm15
    0xc4,0x42,0x75,0xa8,0xfc,                   //  vfmadd213ps   %ymm12,%ymm1,%ymm15
    0xc4,0x42,0x15,0xa8,0xfe,                   //  vfmadd213ps   %ymm14,%ymm13,%ymm15
    0xc4,0xc1,0x74,0xc2,0xca,0x01,              //  vcmpltps      %ymm10,%ymm1,%ymm1
    0xc4,0xc3,0x05,0x4a,0xc9,0x10,              //  vblendvps     %ymm1,%ymm9,%ymm15,%ymm1
    0xc5,0x3c,0x59,0xc2,                        //  vmulps        %ymm2,%ymm8,%ymm8
    0xc5,0x6c,0x59,0xca,                        //  vmulps        %ymm2,%ymm2,%ymm9
    0xc4,0x42,0x6d,0xa8,0xdc,                   //  vfmadd213ps   %ymm12,%ymm2,%ymm11
    0xc4,0x42,0x35,0xa8,0xde,                   //  vfmadd213ps   %ymm14,%ymm9,%ymm11
    0xc4,0xc1,0x6c,0xc2,0xd2,0x01,              //  vcmpltps      %ymm10,%ymm2,%ymm2
    0xc4,0xc3,0x25,0x4a,0xd0,0x20,              //  vblendvps     %ymm2,%ymm8,%ymm11,%ymm2
};
static const unsigned char kSplice_to_srgb[] = {
    0xc5,0x7c,0x52,0xc0,                        //  vrsqrtps      %ymm0,%ymm8
    0xc4,0x41,0x7c,0x53,0xc8,                   //  vrcpps        %ymm8,%ymm9
    0xc4,0x41,0x7c,0x52,0xd0,                   //  vrsqrtps      %ymm8,%ymm10
    0xc4,0x62,0x7d,0x18,0x41,0x24,              //  vbroadcastss  0x24(%rcx),%ymm8
    0xc5,0x3c,0x59,0xd8,                        //  vmulps        %ymm0,%ymm8,%ymm11
    0xc4,0x62,0x7d,0x18,0x61,0x04,              //  vbroadcastss  0x4(%rcx),%ymm12
    0xc4,0x62,0x7d,0x18,0x69,0x28,              //  vbroadcastss  0x28(%rcx),%ymm13
    0xc4,0x62,0x7d,0x18,0x71,0x2c,              //  vbroadcastss  0x2c(%rcx),%ymm14
    0xc4,0x62,0x7d,0x18,0x79,0x30,              //  vbroadcastss  0x30(%rcx),%ymm15
    0xc4,0x42,0x0d,0xa8,0xcf,                   //  vfmadd213ps   %ymm15,%ymm14,%ymm9
    0xc4,0x42,0x15,0xb8,0xca,                   //  vfmadd231ps   %ymm10,%ymm13,%ymm9
    0xc4,0x41,0x1c,0x5d,0xc9,                   //  vminps        %ymm9,%ymm12,%ymm9
    0xc4,0x62,0x7d,0x18,0x51,0x34,              //  vbroadcastss  0x34(%rcx),%ymm10
    0xc4,0xc1,0x7c,0xc2,0xc2,0x01,              //  vcmpltps      %ymm10,%ymm0,%ymm0
    0xc4,0xc3,0x35,0x4a,0xc3,0x00,              //  vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
    0xc5,0x7c,0x52,0xc9,                        //  vrsqrtps      %ymm1,%ymm9
    0xc4,0x41,0x7c,0x53,0xd9,                   //  vrcpps        %ymm9,%ymm11
    0xc4,0x41,0x7c,0x52,0xc9,                   //  vrsqrtps      %ymm9,%ymm9
    0xc4,0x42,0x0d,0xa8,0xdf,                   //  vfmadd213ps   %ymm15,%ymm14,%ymm11
    0xc4,0x42,0x15,0xb8,0xd9,                   //  vfmadd231ps   %ymm9,%ymm13,%ymm11
    0xc5,0x3c,0x59,0xc9,                        //  vmulps        %ymm1,%ymm8,%ymm9
    0xc4,0x41,0x1c,0x5d,0xdb,                   //  vminps        %ymm11,%ymm12,%ymm11
    0xc4,0xc1,0x74,0xc2,0xca,0x01,              //  vcmpltps      %ymm10,%ymm1,%ymm1
    0xc4,0xc3,0x25,0x4a,0xc9,0x10,              //  vblendvps     %ymm1,%ymm9,%ymm11,%ymm1
    0xc5,0x7c,0x52,0xca,                        //  vrsqrtps      %ymm2,%ymm9
    0xc4,0x41,0x7c,0x53,0xd9,                   //  vrcpps        %ymm9,%ymm11
    0xc4,0x42,0x0d,0xa8,0xdf,                   //  vfmadd213ps   %ymm15,%ymm14,%ymm11
    0xc4,0x41,0x7c,0x52,0xc9,                   //  vrsqrtps      %ymm9,%ymm9
    0xc4,0x42,0x15,0xb8,0xd9,                   //  vfmadd231ps   %ymm9,%ymm13,%ymm11
    0xc4,0x41,0x1c,0x5d,0xcb,                   //  vminps        %ymm11,%ymm12,%ymm9
    0xc5,0x3c,0x59,0xc2,                        //  vmulps        %ymm2,%ymm8,%ymm8
    0xc4,0xc1,0x6c,0xc2,0xd2,0x01,              //  vcmpltps      %ymm10,%ymm2,%ymm2
    0xc4,0xc3,0x35,0x4a,0xd0,0x20,              //  vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
};
static const unsigned char kSplice_scale_u8[] = {
    0x48,0x8b,0x02,                             //  mov           (%rdx),%rax
    0xc4,0x62,0x7d,0x31,0x04,0x38,              //  vpmovzxbd     (%rax,%rdi,1),%ymm8
    0xc4,0x41,0x7c,0x5b,0xc0,                   //  vcvtdq2ps     %ymm8,%ymm8
    0xc4,0x62,0x7d,0x18,0x49,0x0c,              //  vbroadcastss  0xc(%rcx),%ymm9
    0xc4,0x41,0x3c,0x59,0xc1,                   //  vmulps        %ymm9,%ymm8,%ymm8
    0xc5,0xbc,0x59,0xc0,                        //  vmulps        %ymm0,%ymm8,%ymm0
    0xc5,0xbc,0x59,0xc9,                        //  vmulps        %ymm1,%ymm8,%ymm1
    0xc5,0xbc,0x59,0xd2,                        //  vmulps        %ymm2,%ymm8,%ymm2
    0xc5,0xbc,0x59,0xdb,                        //  vmulps        %ymm3,%ymm8,%ymm3
};
static const unsigned char kSplice_load_8888[] = {
    0x48,0x8b,0x02,                             //  mov           (%rdx),%rax
    0xc5,0xfc,0x10,0x1c,0xb8,                   //  vmovups       (%rax,%rdi,4),%ymm3
    0xc4,0xe2,0x7d,0x18,0x11,                   //  vbroadcastss  (%rcx),%ymm2
    0xc5,0xec,0x54,0xc3,                        //  vandps        %ymm3,%ymm2,%ymm0
    0xc5,0xfc,0x5b,0xc0,                        //  vcvtdq2ps     %ymm0,%ymm0
    0xc4,0x62,0x7d,0x18,0x41,0x0c,              //  vbroadcastss  0xc(%rcx),%ymm8
    0xc5,0xbc,0x59,0xc0,                        //  vmulps        %ymm0,%ymm8,%ymm0
    0xc5,0xf5,0x72,0xd3,0x08,                   //  vpsrld        $0x8,%ymm3,%ymm1
    0xc5,0xec,0x54,0xc9,                        //  vandps        %ymm1,%ymm2,%ymm1
    0xc5,0xfc,0x5b,0xc9,                        //  vcvtdq2ps     %ymm1,%ymm1
    0xc5,0xbc,0x59,0xc9,                        //  vmulps        %ymm1,%ymm8,%ymm1
    0xc5,0xb5,0x72,0xd3,0x10,                   //  vpsrld        $0x10,%ymm3,%ymm9
    0xc4,0xc1,0x6c,0x54,0xd1,                   //  vandps        %ymm9,%ymm2,%ymm2
    0xc5,0xfc,0x5b,0xd2,                        //  vcvtdq2ps     %ymm2,%ymm2
    0xc5,0xbc,0x59,0xd2,                        //  vmulps        %ymm2,%ymm8,%ymm2
    0xc5,0xe5,0x72,0xd3,0x18,                   //  vpsrld        $0x18,%ymm3,%ymm3
    0xc5,0xfc,0x5b,0xdb,                        //  vcvtdq2ps     %ymm3,%ymm3
    0xc4,0xc1,0x64,0x59,0xd8,                   //  vmulps        %ymm8,%ymm3,%ymm3
};
static const unsigned char kSplice_store_8888[] = {
    0x48,0x8b,0x02,                             //  mov           (%rdx),%rax
    0xc4,0x62,0x7d,0x18,0x41,0x08,              //  vbroadcastss  0x8(%rcx),%ymm8
    0xc5,0x3c,0x59,0xc8,                        //  vmulps        %ymm0,%ymm8,%ymm9
    0xc4,0x41,0x7d,0x5b,0xc9,                   //  vcvtps2dq     %ymm9,%ymm9
    0xc5,0x3c,0x59,0xd1,                        //  vmulps        %ymm1,%ymm8,%ymm10
    0xc4,0x41,0x7d,0x5b,0xd2,                   //  vcvtps2dq     %ymm10,%ymm10
    0xc4,0xc1,0x2d,0x72,0xf2,0x08,              //  vpslld        $0x8,%ymm10,%ymm10
    0xc4,0x41,0x2d,0xeb,0xc9,                   //  vpor          %ymm9,%ymm10,%ymm9
    0xc5,0x3c,0x59,0xd2,                        //  vmulps        %ymm2,%ymm8,%ymm10
    0xc4,0x41,0x7d,0x5b,0xd2,                   //  vcvtps2dq     %ymm10,%ymm10
    0xc4,0xc1,0x2d,0x72,0xf2,0x10,              //  vpslld        $0x10,%ymm10,%ymm10
    0xc5,0x3c,0x59,0xc3,                        //  vmulps        %ymm3,%ymm8,%ymm8
    0xc4,0x41,0x7d,0x5b,0xc0,                   //  vcvtps2dq     %ymm8,%ymm8
    0xc4,0xc1,0x3d,0x72,0xf0,0x18,              //  vpslld        $0x18,%ymm8,%ymm8
    0xc4,0x41,0x2d,0xeb,0xc0,                   //  vpor          %ymm8,%ymm10,%ymm8
    0xc4,0x41,0x35,0xeb,0xc0,                   //  vpor          %ymm8,%ymm9,%ymm8
    0xc5,0x7e,0x7f,0x04,0xb8,                   //  vmovdqu       %ymm8,(%rax,%rdi,4)
};
static const unsigned char kSplice_load_f16[] = {
    0x48,0x8b,0x02,                             //  mov           (%rdx),%rax
    0xc5,0xfa,0x6f,0x04,0xf8,                   //  vmovdqu       (%rax,%rdi,8),%xmm0
    0xc5,0xfa,0x6f,0x4c,0xf8,0x10,              //  vmovdqu       0x10(%rax,%rdi,8),%xmm1
    0xc5,0xfa,0x6f,0x54,0xf8,0x20,              //  vmovdqu       0x20(%rax,%rdi,8),%xmm2
    0xc5,0xfa,0x6f,0x5c,0xf8,0x30,              //  vmovdqu       0x30(%rax,%rdi,8),%xmm3
    0xc5,0x79,0x61,0xc1,                        //  vpunpcklwd    %xmm1,%xmm0,%xmm8
    0xc5,0xf9,0x69,0xc1,                        //  vpunpckhwd    %xmm1,%xmm0,%xmm0
    0xc5,0xe9,0x61,0xcb,                        //  vpunpcklwd    %xmm3,%xmm2,%xmm1
    0xc5,0xe9,0x69,0xd3,                        //  vpunpckhwd    %xmm3,%xmm2,%xmm2
    0xc5,0x39,0x61,0xc8,                        //  vpunpcklwd    %xmm0,%xmm8,%xmm9
    0xc5,0x39,0x69,0xc0,                        //  vpunpckhwd    %xmm0,%xmm8,%xmm8
    0xc5,0xf1,0x61,0xda,                        //  vpunpcklwd    %xmm2,%xmm1,%xmm3
    0xc5,0x71,0x69,0xd2,                        //  vpunpckhwd    %xmm2,%xmm1,%xmm10
    0xc5,0xb1,0x6c,0xc3,                        //  vpunpcklqdq   %xmm3,%xmm9,%xmm0
    0xc4,0xe2,0x7d,0x13,0xc0,                   //  vcvtph2ps     %xmm0,%ymm0
    0xc5,0xb1,0x6d,0xcb,                        //  vpunpckhqdq   %xmm3,%xmm9,%xmm1
    0xc4,0xe2,0x7d,0x13,0xc9,                   //  vcvtph2ps     %xmm1,%ymm1
    0xc4,0xc1,0x39,0x6c,0xd2,                   //  vpunpcklqdq   %xmm10,%xmm8,%xmm2
    0xc4,0xe2,0x7d,0x13,0xd2,                   //  vcvtph2ps     %xmm2,%ymm2
    0xc4,0xc1,0x39,0x6d,0xda,                   //  vpunpckhqdq   %xmm10,%xmm8,%xmm3
    0xc4,0xe2,0x7d,0x13,0xdb,                   //  vcvtph2ps     %xmm3,%ymm3
};
static const unsigned char kSplice_store_f16[] = {
    0x48,0x8b,0x02,                             //  mov           (%rdx),%rax
    0xc4,0xc3,0x7d,0x1d,0xc0,0x04,              //  vcvtps2ph     $0x4,%ymm0,%xmm8
    0xc4,0xc3,0x7d,0x1d,0xc9,0x04,              //  vcvtps2ph     $0x4,%ymm1,%xmm9
    0xc4,0xc3,0x7d,0x1d,0xd2,0x04,              //  vcvtps2ph     $0x4,%ymm2,%xmm10
    0xc4,0xc3,0x7d,0x1d,0xdb,0x04,              //  vcvtps2ph     $0x4,%ymm3,%xmm11
    0xc4,0x41,0x39,0x61,0xe1,                   //  vpunpcklwd    %xmm9,%xmm8,%xmm12
    0xc4,0x41,0x39,0x69,0xc1,                   //  vpunpckhwd    %xmm9,%xmm8,%xmm8
    0xc4,0x41,0x29,0x61,0xcb,                   //  vpunpcklwd    %xmm11,%xmm10,%xmm9
    0xc4,0x41,0x29,0x69,0xd3,                   //  vpunpckhwd    %xmm11,%xmm10,%xmm10
    0xc4,0x41,0x19,0x62,0xd9,                   //  vpunpckldq    %xmm9,%xmm12,%xmm11
    0xc5,0x7a,0x7f,0x1c,0xf8,                   //  vmovdqu       %xmm11,(%rax,%rdi,8)
    0xc4,0x41,0x19,0x6a,0xc9,                   //  vpunpckhdq    %xmm9,%xmm12,%xmm9
    0xc5,0x7a,0x7f,0x4c,0xf8,0x10,              //  vmovdqu       %xmm9,0x10(%rax,%rdi,8)
    0xc4,0x41,0x39,0x62,0xca,                   //  vpunpckldq    %xmm10,%xmm8,%xmm9
    0xc5,0x7a,0x7f,0x4c,0xf8,0x20,              //  vmovdqu       %xmm9,0x20(%rax,%rdi,8)
    0xc4,0x41,0x39,0x6a,0xc2,                   //  vpunpckhdq    %xmm10,%xmm8,%xmm8
    0xc5,0x7a,0x7f,0x44,0xf8,0x30,              //  vmovdqu       %xmm8,0x30(%rax,%rdi,8)
};

#endif

#endif//SkSplicer_generated_DEFINED
